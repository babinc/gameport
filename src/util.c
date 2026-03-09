#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "util.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── which ────────────────────────────────────────────────────── */

char *which_path(const char *bin) {
    return plat_which(bin);
}

int which(const char *bin) {
    char *p = plat_which(bin);
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

int has_runtime(const Toolchains *tc, AcquireMethod method) {
    switch (method) {
    case ACQUIRE_CARGO: return tc->cargo;
    case ACQUIRE_GIT:   return tc->git;
    }
    return 1;
}

const char *runtime_install_hint(AcquireMethod method) {
    switch (method) {
    case ACQUIRE_CARGO: return "Install Rust: https://rustup.rs";
    case ACQUIRE_GIT:   return "Install Git: https://git-scm.com";
    }
    return "Unknown runtime";
}

/* ── Path helpers ─────────────────────────────────────────────── */

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
    plat_mkdir_p(buf);
    return strdup(buf);
}

char *games_dir(void) { return sub_dir("games"); }
char *logs_dir(void)  { return sub_dir("logs"); }

int deps_check_satisfied(const PlatformDeps *deps) {
    if (!deps->check_cmd || !deps->check_cmd[0]) return 0;
    return plat_run_silent(deps->check_cmd, NULL);
}

int is_git_cloned_not_ready(const Game *g) {
    const Source *src = default_source(g);
    if (!src || src->method != ACQUIRE_GIT) return 0;
    if (!src->play_cmd || !src->play_cmd[0]) return 0;
    char *gdir = games_dir();
    char git_path[1024], bin_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/%s/.git", gdir, src->clone_dir);
    snprintf(bin_path, sizeof(bin_path), "%s/%s/%s", gdir, src->clone_dir, src->play_cmd[0]);
    free(gdir);
    return plat_file_exists(git_path) && !plat_file_exists(bin_path);
}

int is_installed(const Game *g) {
    const Source *src = default_source(g);
    if (!src) return 0;

    if (src->method == ACQUIRE_GIT) {
        if (!src->play_cmd || !src->play_cmd[0]) return 0;
        char *gdir = games_dir();
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s/%s", gdir, src->clone_dir, src->play_cmd[0]);
        free(gdir);
        return plat_file_exists(path);
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

/* ── Size formatting ──────────────────────────────────────────── */

void format_size(unsigned long bytes, char *buf, int buflen) {
    if (bytes >= 1048576)
        snprintf(buf, (size_t)buflen, "%.1f MB", (double)bytes / 1048576.0);
    else if (bytes >= 1024)
        snprintf(buf, (size_t)buflen, "%.0f KB", (double)bytes / 1024.0);
    else
        snprintf(buf, (size_t)buflen, "%lu B", bytes);
}
