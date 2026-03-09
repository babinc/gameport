#ifndef UI_H
#define UI_H

#include "term.h"
#include "catalog.h"
#include "install.h"
#include "util.h"

/* ── App modes ────────────────────────────────────────────────── */
typedef enum {
    MODE_NORMAL,
    MODE_INSTALLING,
    MODE_RUNNING,
    MODE_VIEWLOG,
    MODE_SEARCH,
    MODE_SOURCE_SELECT,
    MODE_CONTROLS,
} AppMode;

/* ── Constants ────────────────────────────────────────────────── */
#define MAX_CATEGORIES 16

/* ── App state ────────────────────────────────────────────────── */
typedef struct {
    int selected;
    int *installed;
    int *cloned;              /* cached is_git_cloned_not_ready */
    int *deps_satisfied;      /* cached deps_check_satisfied */
    char **install_methods;   /* cached load_install_method per game */
    int should_quit;
    unsigned long tick;
    AppMode mode;
    Toolchains toolchains;

    /* Status message */
    char message[256];
    int msg_ok;             /* 1 = green, 0 = red */
    int has_message;

    /* Child process (install/run) */
    ChildProc child;
    const char *panel_label;  /* "INSTALLING", "REMOVING", "RUNNING" */
    int panel_scroll;
    int active_game;          /* real GAMES[] index of game being installed/run */

    /* Command chain: next command to run after current child exits OK */
    char **next_cmd;          /* NULL-terminated, malloc'd (freed after use) */
    char *next_cwd;           /* malloc'd or NULL */

    /* Border flash (countdown ticks for install-complete flash) */
    int border_flash;

    /* Last log */
    char *last_log;           /* malloc'd, NULL if none */
    int log_scroll;

    /* Search */
    char search[64];
    int search_len;
    int *filtered;            /* indices into GAMES[] that match */
    int filter_count;         /* number of matches */

    /* Category filter */
    int cat_index;            /* 0 = ALL, 1..N = specific category */
    int cat_collapsed[MAX_CATEGORIES];
    int cat_counts[MAX_CATEGORIES]; /* cached game count per category */

    /* Source selection */
    int source_selected;      /* cursor in source picker */

    /* Platform filter */
    int plat_filter;          /* 0 = all, 1 = linux, 2 = macos, 3 = windows */
} App;

/* ── Filtered list encoding ──────────────────────────────────── */
#define IS_HEADER(x)    ((x) < 0)
#define HEADER_CAT(x)   (-(x) - 1)       /* filtered value → CATEGORIES index */
#define MAKE_HEADER(c)  (-(c) - 1)        /* CATEGORIES index → filtered value */

/* ── App lifecycle ────────────────────────────────────────────── */
void app_init(App *app);
void app_refresh(App *app);
void app_cleanup(App *app);
void app_next(App *app);
void app_prev(App *app);
void app_set_message(App *app, const char *msg, int ok);
void app_clear_message(App *app);
void app_rebuild_filter(App *app);

extern const int NUM_CATEGORIES;

/* ── Rendering ────────────────────────────────────────────────── */
void ui_draw(Screen *s, App *app);

#endif
