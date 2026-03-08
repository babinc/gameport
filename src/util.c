#define _POSIX_C_SOURCE 200809L
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

/* ── which ────────────────────────────────────────────────────── */

char *which_path(const char *bin) {
    const char *path = getenv("PATH");
    if (!path) return NULL;

    char *dup = strdup(path);
    char *saveptr = NULL;
    char *dir = strtok_r(dup, ":", &saveptr);
    while (dir) {
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", dir, bin);
        if (access(full, X_OK) == 0) {
            free(dup);
            return strdup(full);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }
    free(dup);
    return NULL;
}

int which(const char *bin) {
    char *p = which_path(bin);
    if (p) { free(p); return 1; }
    return 0;
}

/* ── Toolchains ───────────────────────────────────────────────── */

Toolchains toolchains_detect(void) {
    Toolchains tc;
    tc.cargo  = which("cargo");
    tc.python = which("python") || which("python3");
    tc.cmake  = which("cmake");
    tc.git    = which("git");
    return tc;
}

int has_runtime(const Toolchains *tc, MethodType method) {
    switch (method) {
    case METHOD_CARGO: return tc->cargo;
    case METHOD_GIT:   return tc->git;
    }
    return 1;
}

const char *runtime_install_hint(MethodType method) {
    switch (method) {
    case METHOD_CARGO: return "Install Rust: https://rustup.rs";
    case METHOD_GIT:   return "Install Git: https://git-scm.com";
    }
    return "Unknown runtime";
}

/* ── Path helpers ─────────────────────────────────────────────── */

static void mkdir_p(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

/* OS-aware base data directory for gameport */
static void data_base(char *buf, size_t buflen) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
#if defined(__APPLE__)
    snprintf(buf, buflen, "%s/Library/Application Support/gameport", home);
#elif defined(_WIN32)
    const char *appdata = getenv("APPDATA");
    if (appdata)
        snprintf(buf, buflen, "%s/gameport", appdata);
    else
        snprintf(buf, buflen, "%s/AppData/Roaming/gameport", home);
#else
    snprintf(buf, buflen, "%s/.local/share/gameport", home);
#endif
}

static char *sub_dir(const char *name) {
    char base[512];
    data_base(base, sizeof(base));
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/%s", base, name);
    mkdir_p(buf);
    return strdup(buf);
}

char *games_dir(void) { return sub_dir("games"); }
char *logs_dir(void)  { return sub_dir("logs"); }

int deps_check_satisfied(const PlatformDeps *deps) {
    if (!deps->check_cmd || !deps->check_cmd[0]) return 0;
    pid_t pid = fork();
    if (pid == 0) {
        /* Redirect to /dev/null */
        int fd = open("/dev/null", O_WRONLY);
        if (fd >= 0) { dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); close(fd); }
        /* Count args */
        int n = 0;
        while (deps->check_cmd[n]) n++;
        char **argv = malloc((size_t)(n + 1) * sizeof(char *));
        for (int i = 0; i < n; i++) argv[i] = (char *)deps->check_cmd[i];
        argv[n] = NULL;
        execvp(argv[0], argv);
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int is_git_cloned_not_ready(const Game *g) {
    const Source *src = default_source(g);
    if (!src || src->method != METHOD_GIT) return 0;
    if (!src->play_cmd || !src->play_cmd[0]) return 0;
    char *gdir = games_dir();
    char git_path[1024], bin_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/%s/.git", gdir, src->clone_dir);
    snprintf(bin_path, sizeof(bin_path), "%s/%s/%s", gdir, src->clone_dir, src->play_cmd[0]);
    free(gdir);
    struct stat st;
    return (stat(git_path, &st) == 0) && (stat(bin_path, &st) != 0);
}

int is_installed(const Game *g) {
    const Source *src = default_source(g);
    if (!src) return 0;

    if (src->method == METHOD_GIT) {
        if (!src->play_cmd || !src->play_cmd[0]) return 0;
        char *gdir = games_dir();
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s/%s", gdir, src->clone_dir, src->play_cmd[0]);
        free(gdir);
        struct stat st;
        return stat(path, &st) == 0;
    }
    /* cargo — check bin on PATH */
    return which(src->bin);
}

/* ── Install method tracking ──────────────────────────────────── */

static void meta_path(const char *game_name, char *buf, size_t buflen) {
    char *mdir = sub_dir("meta");
    char safe[128];
    sanitize_name(game_name, safe, sizeof(safe));
    snprintf(buf, buflen, "%s/%s.method", mdir, safe);
    free(mdir);
}

void save_install_method(const char *game_name, const char *label) {
    char path[1024];
    meta_path(game_name, path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s\n", label);
        fclose(f);
    }
}

char *load_install_method(const char *game_name) {
    char path[1024];
    meta_path(game_name, path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    char buf[256];
    if (fgets(buf, sizeof(buf), f)) {
        fclose(f);
        /* Strip trailing newline */
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        return strdup(buf);
    }
    fclose(f);
    return NULL;
}

void clear_install_method(const char *game_name) {
    char path[1024];
    meta_path(game_name, path, sizeof(path));
    remove(path);
}

/* ── Name sanitization ────────────────────────────────────────── */

void sanitize_name(const char *in, char *out, size_t outlen) {
    snprintf(out, outlen, "%s", in);
    for (char *p = out; *p; p++) {
        if (*p == ' ') *p = '_';
        *p = (char)tolower((unsigned char)*p);
    }
}

/* ── Shell helpers ────────────────────────────────────────────── */

void shell_quote(char *out, size_t outlen, const char *in) {
    size_t j = 0;
    if (j < outlen - 1) out[j++] = '\'';
    for (size_t i = 0; in[i] && j < outlen - 5; i++) {
        if (in[i] == '\'') {
            out[j++] = '\'';
            out[j++] = '\\';
            out[j++] = '\'';
            out[j++] = '\'';
        } else {
            out[j++] = in[i];
        }
    }
    if (j < outlen - 1) out[j++] = '\'';
    out[j] = '\0';
}

/* ── Size formatting ──────────────────────────────────────────── */

void format_size(unsigned long bytes, char *buf, int buflen) {
    if (bytes >= 1048576)
        snprintf(buf, (size_t)buflen, "%.1f MB", (double)bytes / 1048576.0);
    else if (bytes >= 1024)
        snprintf(buf, (size_t)buflen, "%.0f KB", (double)bytes / 1024.0);
    else
        snprintf(buf, (size_t)buflen, "%lu B", bytes);
}
