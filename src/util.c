#define _POSIX_C_SOURCE 200809L
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
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

int has_runtime(const Toolchains *tc, const char *method) {
    if (strcmp(method, "cargo") == 0) return tc->cargo;
    if (strcmp(method, "cmake") == 0) return tc->cmake;
    if (strcmp(method, "git") == 0)   return tc->git;
    return 1;
}

const char *runtime_install_hint(const char *method) {
    if (strcmp(method, "cargo") == 0) return "Install Rust: https://rustup.rs";
    if (strcmp(method, "cmake") == 0) return "Need: cmake, git, C compiler (gcc)";
    if (strcmp(method, "git") == 0)   return "Install Git: https://git-scm.com";
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

char *games_dir(void) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/.local/share/gameport/games", home);
    mkdir_p(buf);
    return strdup(buf);
}

char *cmake_game_exe(const Source *src) {
    char *gdir = games_dir();
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/%s/build/%s", gdir, src->clone_dir, src->bin);
    free(gdir);
    return strdup(buf);
}

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
    if (!src || strcmp(src->method, "git") != 0) return 0;
    char *gdir = games_dir();
    char git_path[1024], ready_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/%s/.git", gdir, src->clone_dir);
    snprintf(ready_path, sizeof(ready_path), "%s/%s/.arcade-ready", gdir, src->clone_dir);
    free(gdir);
    struct stat st;
    return (stat(git_path, &st) == 0) && (stat(ready_path, &st) != 0);
}

int is_installed(const Game *g) {
    const Source *src = default_source(g);
    if (!src) return 0;

    if (strcmp(src->method, "cmake") == 0) {
        char *gdir = games_dir();
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s/build", gdir, src->clone_dir);
        free(gdir);
        struct stat st;
        return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
    }
    if (strcmp(src->method, "git") == 0) {
        char *gdir = games_dir();
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s/.arcade-ready", gdir, src->clone_dir);
        free(gdir);
        struct stat st;
        return stat(path, &st) == 0;
    }
    /* cargo — check bin on PATH */
    return which(src->bin);
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
