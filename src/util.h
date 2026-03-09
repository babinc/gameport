#ifndef UTIL_H
#define UTIL_H

#include "catalog.h"
#include <stddef.h>

/* ── Toolchain detection ──────────────────────────────────────── */
typedef struct {
    int cargo;
    int cmake;
    int make;
    int git;
    int curl;
} Toolchains;

Toolchains toolchains_detect(void);
int        has_runtime(const Toolchains *tc, AcquireMethod method);
const char *runtime_install_hint(AcquireMethod method);

/* ── Path helpers ─────────────────────────────────────────────── */
int   which(const char *bin);                /* returns 1 if found on PATH */
char *which_path(const char *bin);           /* returns malloc'd full path or NULL */
char *games_dir(void);                       /* returns malloc'd path, creates dir */
char *logs_dir(void);                        /* returns malloc'd path, creates dir */
int   deps_check_satisfied(const PlatformDeps *deps);
int   is_git_cloned_not_ready(const Game *g);
int   is_installed(const Game *g);

/* ── Install method tracking ──────────────────────────────────── */
void  save_install_method(const char *game_name, const char *label);
char *load_install_method(const char *game_name);  /* malloc'd or NULL */
void  clear_install_method(const char *game_name);

/* ── Name sanitization ────────────────────────────────────────── */
void  sanitize_name(const char *in, char *out, size_t outlen);

/* ── Size formatting ──────────────────────────────────────────── */
void  format_size(unsigned long bytes, char *buf, int buflen);

#endif
