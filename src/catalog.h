#ifndef CATALOG_H
#define CATALOG_H

#include <stddef.h>   /* NULL */

/* ── Acquire method ──────────────────────────────────────────── */
typedef enum { ACQUIRE_CARGO, ACQUIRE_GIT, ACQUIRE_DOWNLOAD } AcquireMethod;
const char *acquire_str(AcquireMethod m);

/* ── Platform flags (bitfield) ───────────────────────────────── */
enum {
    PLAT_LINUX   = 1,
    PLAT_MACOS   = 2,
    PLAT_WINDOWS = 4,
    PLAT_ALL     = PLAT_LINUX | PLAT_MACOS | PLAT_WINDOWS,
};

/* ── Platform deps ────────────────────────────────────────────── */
typedef struct {
    const char *os;             /* "linux", "windows", "macos" */
    const char *label;          /* human-readable dep list */
    const char **install_cmd;   /* NULL-terminated */
    const char **check_cmd;     /* NULL-terminated */
    int needs_sudo;
} PlatformDeps;

/* ── Source (a way to acquire and run a game) ─────────────────── */
typedef struct {
    AcquireMethod method;
    const char *label;          /* shown in UI */
    int platforms;              /* PLAT_* bitfield */

    /* Acquire (git clone or download) */
    const char *url;            /* git URL or download URL */
    const char *dir;      /* local directory name */
    int shallow;                /* git: shallow clone */
    const char *archive_type;   /* download: NULL=raw binary, "tar.gz", "zip" */

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
    const char *website;        /* game homepage URL, NULL if none */
    const char *repo;
    int platforms;              /* PLAT_* bitfield */
    const PlatformDeps *platform_deps;
    int num_platform_deps;
    const Source *sources;
    int num_sources;
} Game;

/* ── Catalog ──────────────────────────────────────────────────── */
extern Game GAMES[];
extern int  NUM_GAMES;
void catalog_init(void);

/* ── Common arrays ───────────────────────────────────────────── */
extern const char *MAC_XCODE_INSTALL[];     /* {"xcode-select", "--install", NULL} */
extern const char *MAC_XCODE_CHECK[];       /* {"xcode-select", "-p", NULL} */

/* ── Helpers ──────────────────────────────────────────────────── */
const char     *current_platform(void);     /* string: "linux", "macos", "windows" */
int             current_platform_bit(void); /* PLAT_LINUX, PLAT_MACOS, or PLAT_WINDOWS */
int             game_supports_platform(const Game *g);
const PlatformDeps *platform_deps_for_current(const Game *g);
const Source   *default_source(const Game *g);
int             default_source_index(const Game *g);
int             count_platform_sources(const Game *g);

#endif
