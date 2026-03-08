#ifndef UTIL_H
#define UTIL_H

#include "catalog.h"

/* ── Toolchain detection ──────────────────────────────────────── */
typedef struct {
    int cargo;
    int python;
    int cmake;
    int git;
} Toolchains;

Toolchains toolchains_detect(void);
int        has_runtime(const Toolchains *tc, const char *method);
const char *runtime_install_hint(const char *method);

/* ── Path helpers ─────────────────────────────────────────────── */
int   which(const char *bin);                /* returns 1 if found on PATH */
char *which_path(const char *bin);           /* returns malloc'd full path or NULL */
char *games_dir(void);                       /* returns malloc'd path, creates dir */
char *cmake_game_exe(const Source *src);     /* returns malloc'd path */
int   deps_check_satisfied(const PlatformDeps *deps);
int   is_git_cloned_not_ready(const Game *g);
int   is_installed(const Game *g);

/* ── Size formatting ──────────────────────────────────────────── */
void  format_size(unsigned long bytes, char *buf, int buflen);

#endif
