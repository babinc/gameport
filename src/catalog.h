#ifndef CATALOG_H
#define CATALOG_H

#include <stddef.h>   /* NULL */

/* ── Acquire method ──────────────────────────────────────────── */
typedef enum { ACQUIRE_CARGO, ACQUIRE_GIT } AcquireMethod;
const char *acquire_str(AcquireMethod m);

/* ── Platform deps ────────────────────────────────────────────── */
typedef struct {
    const char *os;             /* "linux", "windows", "macos" */
    const char *deps;           /* human-readable dep list */
    const char **install_cmd;   /* NULL-terminated */
    const char **check_cmd;     /* NULL-terminated */
    int needs_sudo;
} PlatformDeps;

/* ── Source (a way to acquire and run a game) ─────────────────── */
typedef struct {
    AcquireMethod method;
    const char *label;          /* shown in UI */
    const char **platforms;     /* NULL-terminated, NULL = all */

    /* Acquire */
    const char *clone_url;
    const char *clone_dir;
    int shallow;

    /* Build */
    const char **build_cmd;     /* NULL-terminated */

    /* Run */
    const char **play_cmd;      /* NULL-terminated */
    const char *bin;

    /* Uninstall */
    const char **uninstall_cmd; /* NULL-terminated */
} Source;

/* ── Game ─────────────────────────────────────────────────────── */
typedef struct {
    const char *name;
    const char *icon;
    const char *desc;
    const char **keys;          /* NULL-terminated "Key|Action" pairs */
    const char *category;
    const char *engine;
    const char *repo;
    const char **platforms;     /* NULL-terminated, NULL = all */
    const PlatformDeps *platform_deps;
    int num_deps;
    const Source *sources;
    int num_sources;
} Game;

/* ── Catalog ──────────────────────────────────────────────────── */
extern Game GAMES[];
extern int  NUM_GAMES;
void catalog_init(void);

/* ── Helpers ──────────────────────────────────────────────────── */
const char     *current_platform(void);
int             game_supports_platform(const Game *g);
const PlatformDeps *platform_deps_for_current(const Game *g);
const Source   *default_source(const Game *g);
int             default_source_index(const Game *g);
int             count_platform_sources(const Game *g);

#endif
