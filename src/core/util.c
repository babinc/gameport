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
    tc.cmake  = which("cmake");
    tc.make   = which("make");
    tc.git    = which("git");
    tc.curl   = which("curl");
    return tc;
}

int has_runtime(const Toolchains *tc, AcquireMethod method) {
    switch (method) {
    case ACQUIRE_CARGO:    return tc->cargo;
    case ACQUIRE_GIT:      return tc->git;
    case ACQUIRE_DOWNLOAD: return tc->curl;
    default: return 1;
    }
}

const char *runtime_install_hint(AcquireMethod method) {
    switch (method) {
    case ACQUIRE_CARGO:    return "Install Rust: https://rustup.rs";
    case ACQUIRE_GIT:      return "Install Git: https://git-scm.com";
    case ACQUIRE_DOWNLOAD: return "Install curl: https://curl.se";
    default: return "Unknown runtime";
    }
}

/* ── Path helpers ─────────────────────────────────────────────── */

/* OS-aware base data directory for open-game-portal */
static void data_base(char *buf, size_t buflen) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
#if defined(__APPLE__)
    snprintf(buf, buflen, "%s/Library/Application Support/open-game-portal", home);
#elif defined(_WIN32)
    const char *appdata = getenv("APPDATA");
    if (appdata)
        snprintf(buf, buflen, "%s/open-game-portal", appdata);
    else
        snprintf(buf, buflen, "%s/AppData/Roaming/open-game-portal", home);
#else
    snprintf(buf, buflen, "%s/.local/share/open-game-portal", home);
#endif
}

static char *sub_dir(const char *name) {
    char base[512];
    data_base(base, sizeof(base));
    char buf[PATHBUF];
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

/* ── Install marker (.ogp_installed) ─────────────────────────── */

static void marker_path(const Source *src, char *buf, size_t buflen) {
    char *gdir = games_dir();
    snprintf(buf, buflen, "%s/%s/.ogp_installed", gdir, src->dir);
    free(gdir);
}

void mark_installed(const Source *src) {
    if (!src->dir) return;
    char path[PATHBUF];
    marker_path(src, path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (f) fclose(f);
}

void mark_uninstalled(const Source *src) {
    if (!src->dir) return;
    char path[PATHBUF];
    marker_path(src, path, sizeof(path));
    remove(path);
}

int is_git_cloned_not_ready(const Game *g) {
    const Source *src = default_source(g);
    if (!src || src->method != ACQUIRE_GIT) return 0;
    char *gdir = games_dir();
    char git_path[PATHBUF], marker[PATHBUF];
    snprintf(git_path, sizeof(git_path), "%s/%s/.git", gdir, src->dir);
    free(gdir);
    marker_path(src, marker, sizeof(marker));
    return plat_file_exists(git_path) && !plat_file_exists(marker);
}

int is_installed(const Game *g) {
    const Source *src = default_source(g);
    if (!src) return 0;

    if (src->method == ACQUIRE_GIT || src->method == ACQUIRE_DOWNLOAD) {
        if (!src->dir) return 0;
        char *gdir = games_dir();
        char marker[PATHBUF], dir_path[PATHBUF];
        snprintf(marker, sizeof(marker), "%s/%s/.ogp_installed", gdir, src->dir);
        snprintf(dir_path, sizeof(dir_path), "%s/%s", gdir, src->dir);
        free(gdir);

        if (plat_file_exists(marker)) return 1;

        /* Migration: if game dir exists but no marker, write one.
           Covers games installed before marker system was added. */
        if (plat_file_exists(dir_path)) {
            mark_installed(src);
            return 1;
        }
        return 0;
    }
    /* cargo — check bin on PATH */
    return which(src->bin);
}

/* ── Play binary resolution ──────────────────────────────────── */

/* Extract the basename from a path (e.g., "./bin/foo" → "foo") */
static const char *basename_of(const char *path) {
    const char *last_slash = strrchr(path, '/');
    const char *last_bslash = strrchr(path, '\\');
    if (last_bslash && (!last_slash || last_bslash > last_slash))
        last_slash = last_bslash;
    return last_slash ? last_slash + 1 : path;
}

char *resolve_play_binary(const Source *src, const char *cwd) {
    if (!src->play_cmd || !src->play_cmd[0]) return NULL;

    const char *cmd0 = src->play_cmd[0];

    /* Skip wrapper commands — can't resolve these */
    if (strcmp(cmd0, "bash") == 0 || strcmp(cmd0, "sh") == 0 ||
        strcmp(cmd0, "cmd") == 0 || strcmp(cmd0, "powershell") == 0)
        return NULL;

    /* Check if the binary exists at the specified path */
    if (cwd) {
        char full[PATHBUF];
        snprintf(full, sizeof(full), "%s/%s", cwd, cmd0);
        if (plat_file_exists(full)) return NULL; /* path is fine */
    }

    /* Binary not found — search the game directory */
    const char *name = basename_of(cmd0);
    if (!cwd || !name[0]) return NULL;
    return plat_find_executable(cwd, name);
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
    char path[PATHBUF];
    meta_path(game_name, path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s\n", label);
        fclose(f);
    }
}

char *load_install_method(const char *game_name) {
    char path[PATHBUF];
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
    char path[PATHBUF];
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

void format_size(long long bytes, char *buf, int buflen) {
    if (bytes >= 1024LL * 1024 * 1024)
        snprintf(buf, (size_t)buflen, "%.1f GB", (double)bytes / (1024.0 * 1024 * 1024));
    else if (bytes >= 1024LL * 1024)
        snprintf(buf, (size_t)buflen, "%.1f MB", (double)bytes / (1024.0 * 1024));
    else if (bytes >= 1024)
        snprintf(buf, (size_t)buflen, "%.0f KB", (double)bytes / 1024.0);
    else
        snprintf(buf, (size_t)buflen, "%lld B", bytes);
}
