#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/* ── App lifecycle ────────────────────────────────────────────── */

/* ── Categories ───────────────────────────────────────────────── */

static const char *CATEGORIES[] = {
    "ALL", "Action", "Puzzle", "Strategy", "Shooter",
    "Racing", "Simulation", "Platformer", "Stealth", "Roguelike", "Typing",
};
const int NUM_CATEGORIES = sizeof(CATEGORIES) / sizeof(CATEGORIES[0]);

/* Platform filter options */
static const char *PLAT_LABELS[] = {"All", "Linux", "macOS", "Windows"};
static const char *PLAT_IDS[]    = {NULL,  "linux", "macos", "windows"};
const int NUM_PLAT_FILTERS = sizeof(PLAT_LABELS) / sizeof(PLAT_LABELS[0]);

static int game_matches_plat_filter(const Game *g, int plat_filter) {
    if (plat_filter == 0) return 1;  /* "All" */
    const char *plat = PLAT_IDS[plat_filter];
    if (!g->platforms) return 1;     /* NULL = supports all */
    for (int i = 0; g->platforms[i]; i++) {
        if (strcmp(g->platforms[i], plat) == 0) return 1;
    }
    return 0;
}

static int search_matches(const char *name, const char *search, int search_len) {
    for (int n = 0; name[n]; n++) {
        int match = 1;
        for (int s = 0; s < search_len && match; s++) {
            if (!name[n + s] || tolower((unsigned char)name[n + s]) != tolower((unsigned char)search[s]))
                match = 0;
        }
        if (match) return 1;
    }
    return 0;
}

void app_rebuild_filter(App *app) {
    app->filter_count = 0;

    /* Compute per-category counts and header stats (respecting platform filter) */
    memset(app->cat_counts, 0, sizeof(app->cat_counts));
    app->plat_total = 0;
    app->plat_installed = 0;
    for (int i = 0; i < NUM_GAMES; i++) {
        if (!game_matches_plat_filter(&GAMES[i], app->plat_filter)) continue;
        app->plat_total++;
        if (app->installed[i]) app->plat_installed++;
        for (int c = 1; c < NUM_CATEGORIES; c++) {
            if (strcmp(GAMES[i].category, CATEGORIES[c]) == 0) {
                app->cat_counts[c]++;
                break;
            }
        }
    }

    if (app->cat_index == 0 && app->search_len == 0) {
        /* ALL view: group by category with headers */
        for (int c = 1; c < NUM_CATEGORIES; c++) {
            if (app->cat_counts[c] == 0) continue;

            app->filtered[app->filter_count++] = MAKE_HEADER(c);

            if (!app->cat_collapsed[c]) {
                for (int i = 0; i < NUM_GAMES; i++) {
                    if (!game_matches_plat_filter(&GAMES[i], app->plat_filter)) continue;
                    if (strcmp(GAMES[i].category, CATEGORIES[c]) == 0)
                        app->filtered[app->filter_count++] = i;
                }
            }
        }
    } else {
        /* Filtered view: flat list (single category or search active) */
        const char *cat = (app->cat_index == 0) ? NULL : CATEGORIES[app->cat_index];

        for (int i = 0; i < NUM_GAMES; i++) {
            if (!game_matches_plat_filter(&GAMES[i], app->plat_filter)) continue;
            if (cat && strcmp(GAMES[i].category, cat) != 0) continue;
            if (app->search_len > 0 && !search_matches(GAMES[i].name, app->search, app->search_len))
                continue;
            app->filtered[app->filter_count++] = i;
        }
    }

    /* Clamp selection */
    if (app->selected >= app->filter_count)
        app->selected = app->filter_count > 0 ? app->filter_count - 1 : 0;
}

void app_init(App *app) {
    memset(app, 0, sizeof(*app));
    app->installed = malloc((size_t)NUM_GAMES * sizeof(int));
    app->cloned = malloc((size_t)NUM_GAMES * sizeof(int));
    app->deps_satisfied = malloc((size_t)NUM_GAMES * sizeof(int));
    app->install_methods = calloc((size_t)NUM_GAMES, sizeof(char *));
    app->filtered = malloc((size_t)(NUM_GAMES + NUM_CATEGORIES) * sizeof(int));
    app->toolchains = toolchains_detect();
    memset(&app->child.proc, 0, sizeof(app->child.proc));
    app->mode = MODE_NORMAL;
    app->panel_label = "INSTALLING";
    app_refresh(app);
    app_rebuild_filter(app);

    if (!app->toolchains.cargo) {
        app_set_message(app, "cargo not found! Install: https://rustup.rs", 0);
    }
}

void app_refresh(App *app) {
    for (int i = 0; i < NUM_GAMES; i++) {
        app->installed[i] = is_installed(&GAMES[i]);
        app->cloned[i] = is_git_cloned_not_ready(&GAMES[i]);
        const PlatformDeps *deps = platform_deps_for_current(&GAMES[i]);
        app->deps_satisfied[i] = deps ? deps_check_satisfied(deps) : 0;
        free(app->install_methods[i]);
        app->install_methods[i] = load_install_method(GAMES[i].name);
    }
    app->toolchains = toolchains_detect();
}

void app_cleanup(App *app) {
    free(app->installed);
    free(app->cloned);
    free(app->deps_satisfied);
    for (int i = 0; i < NUM_GAMES; i++) free(app->install_methods[i]);
    free(app->install_methods);
    free(app->filtered);
    free(app->last_log);
    if (app->next_cmd) free_cmd(app->next_cmd);
    free(app->next_cwd);
    if (!app->child.done) child_kill(&app->child);
    child_cleanup(&app->child);
}

void app_next(App *app) {
    if (app->filter_count > 0)
        app->selected = (app->selected + 1) % app->filter_count;
    app_clear_message(app);
}

void app_prev(App *app) {
    if (app->filter_count > 0)
        app->selected = (app->selected - 1 + app->filter_count) % app->filter_count;
    app_clear_message(app);
}

void app_set_message(App *app, const char *msg, int ok) {
    snprintf(app->message, sizeof(app->message), "%s", msg);
    app->msg_ok = ok;
    app->has_message = 1;
}

void app_clear_message(App *app) {
    app->has_message = 0;
}

/* ── Header (row 0 = title + badges, row 1 = separator) ──────── */

static Color header_color(unsigned long tick, int col) {
    double hue = fmod((double)(tick * 3 + (unsigned long)col * 8), 360.0);
    return hsl_to_rgb(hue, 0.55, 0.65);
}

static void render_header(Screen *s, App *app) {
    /* Row 0: "GAME PORTAL" rainbow + toolchain badges + game count */
    const char *title = "GAME PORTAL";
    int tx = 2;
    for (int i = 0; title[i]; i++) {
        Color clr = header_color(app->tick, i);
        scr_put(s, tx + i, 0, (uint32_t)title[i], clr, CLR_BG, 1);
    }

    /* Right side: toolchain badges + game count */
    int rx = s->w - 1;

    /* Game count badge (cached in app_rebuild_filter) */
    char countbuf[16];
    snprintf(countbuf, sizeof(countbuf), "%d/%d", app->plat_installed, app->plat_total);
    int clen = (int)strlen(countbuf) + 2; /* +2 for padding */
    rx -= clen;
    scr_badge(s, rx, 0, countbuf, CLR_BLACK, CLR_CYAN, 1);
    rx -= 1;

    /* Toolchain badges */
    struct { const char *name; int found; } tools[] = {
        {"curl", app->toolchains.curl},
        {"python", app->toolchains.python},
        {"make", app->toolchains.make},
        {"cmake", app->toolchains.cmake},
        {"cargo", app->toolchains.cargo},
    };
    for (int t = 0; t < (int)(sizeof(tools)/sizeof(tools[0])); t++) {
        int tlen = (int)strlen(tools[t].name) + 2;
        rx -= tlen;
        Color fg = tools[t].found ? CLR_BLACK : CLR_DARKGRAY;
        Color bg = tools[t].found ? CLR_GREEN : (Color){30,30,40};
        scr_badge(s, rx, 0, tools[t].name, fg, bg, 0);
        rx -= 1;
    }

    /* Row 1: separator */
    scr_hline(s, 0, 1, s->w, 0x2500, CLR_BORDER, CLR_BG);
}

/* ── Game list panel ──────────────────────────────────────────── */

static Color category_color(const char *category) {
    if (!category) return CLR_ICON;
    if (strcmp(category, "Puzzle") == 0)    return (Color){140,100,200};
    if (strcmp(category, "Action") == 0)    return (Color){200,120,80};
    if (strcmp(category, "Strategy") == 0)  return (Color){80,180,160};
    if (strcmp(category, "Shooter") == 0)   return (Color){200,80,80};
    if (strcmp(category, "Racing") == 0)    return (Color){200,160,60};
    if (strcmp(category, "Simulation") == 0) return (Color){80,160,200};
    if (strcmp(category, "Platformer") == 0) return (Color){100,200,100};
    if (strcmp(category, "Stealth") == 0)    return (Color){120,120,180};
    if (strcmp(category, "Roguelike") == 0)  return (Color){180,200,80};
    if (strcmp(category, "Typing") == 0)    return (Color){100,180,220};
    return CLR_ICON;
}

static void render_game_list(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);
    scr_box(s, x, y, w, h, CLR_BORDER);

    /* Category tab in title area */
    const char *cat = CATEGORIES[app->cat_index];
    char title[32];
    snprintf(title, sizeof(title), "%s", cat);
    /* Uppercase for title */
    for (int i = 0; title[i]; i++) title[i] = (char)toupper((unsigned char)title[i]);
    scr_box_title(s, x, y, w, title, category_color(app->cat_index == 0 ? NULL : cat), CLR_BORDER);

    /* Category arrows */
    if (app->cat_index > 0)
        scr_put(s, x + 1, y, 0x25C0, CLR_HINT, CLR_BG, 0); /* ◀ */
    if (app->cat_index < NUM_CATEGORIES - 1)
        scr_put(s, x + w - 2, y, 0x25B6, CLR_HINT, CLR_BG, 0); /* ▶ */

    /* Platform filter bar (row below box title) */
    int plat_row = y + 1;
    {
        static const Color PLAT_COLORS[] = {
            {0,200,200},    /* All = cyan */
            {220,180,50},   /* Linux = gold */
            {180,180,200},  /* macOS = silver */
            {80,160,230},   /* Windows = blue */
        };
        Color bar_bg = (Color){22,22,35};
        scr_fill(s, x + 1, plat_row, w - 2, 1, bar_bg);
        int px = x + 2;
        for (int p = 0; p < NUM_PLAT_FILTERS; p++) {
            int active = (p == app->plat_filter);
            Color fg = active ? CLR_BLACK : (Color){100,100,120};
            Color bg = active ? PLAT_COLORS[p] : bar_bg;
            px += scr_str_n(s, px, plat_row, " ", 1, fg, bg, 0);
            px += scr_str_n(s, px, plat_row, PLAT_LABELS[p], w - (px - x) - 2, fg, bg, active);
            if (active)
                px += scr_str_n(s, px, plat_row, " ", 1, fg, bg, 0);
            px++; /* gap between options */
        }
    }

    /* Search bar (bottom of box, inside) */
    int search_row = y + h - 2;
    if (app->mode == MODE_SEARCH) {
        scr_fill(s, x + 1, search_row, w - 2, 1, (Color){30,30,50});
        int sx = x + 1;
        scr_put(s, sx++, search_row, '/', CLR_CYAN, (Color){30,30,50}, 1);
        scr_str_n(s, sx, search_row, app->search, w - 4, CLR_WHITE, (Color){30,30,50}, 0);
        sx += app->search_len;
        scr_put(s, sx, search_row, '_', CLR_CYAN, (Color){30,30,50}, 0); /* cursor */
    } else if (app->search_len > 0) {
        scr_fill(s, x + 1, search_row, w - 2, 1, CLR_BG);
        int sx = x + 1;
        scr_put(s, sx++, search_row, '/', CLR_HINT, CLR_BG, 0);
        scr_str_n(s, sx, search_row, app->search, w - 4, CLR_HINT, CLR_BG, 0);
    }

    int list_h = h - 3; /* -2 for box border, -1 for platform filter bar */
    /* Reserve a row for search bar when searching or have active search */
    if (app->mode == MODE_SEARCH || app->search_len > 0) list_h--;
    if (list_h < 1) return;

    int total = app->filter_count;

    /* Compute scroll offset to keep selected visible */
    int scroll = 0;
    if (total > list_h) {
        if (app->selected >= list_h) {
            scroll = app->selected - list_h + 1;
        }
        if (scroll > total - list_h) {
            scroll = total - list_h;
        }
    }

    /* Scroll indicators */
    if (scroll > 0) {
        int ax = x + w / 2;
        scr_put(s, ax, y, 0x25B2, CLR_HINT, CLR_BG, 0); /* ▲ */
    }
    if (scroll + list_h < total) {
        int ax = x + w / 2;
        int arrow_row = (app->mode == MODE_SEARCH || app->search_len > 0) ? y + h - 2 : y + h - 1;
        scr_put(s, ax, arrow_row, 0x25BC, CLR_HINT, CLR_BG, 0); /* ▼ */
    }

    for (int i = 0; i < list_h && scroll + i < total; i++) {
        int val = app->filtered[scroll + i];
        int row = y + 2 + i; /* +2: border + platform filter bar */
        int is_sel = (scroll + i == app->selected);

        if (IS_HEADER(val)) {
            /* ── Category header row ─────────────────────── */
            int ci = HEADER_CAT(val);
            int collapsed = app->cat_collapsed[ci];
            Color bg = is_sel ? CLR_SELBG : CLR_BG;
            scr_fill(s, x + 1, row, w - 2, 1, bg);

            int cx = x + 1;
            scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);
            /* ▾ expanded, ▸ collapsed */
            uint32_t tri = collapsed ? 0x25B8 : 0x25BE;
            Color cat_clr = category_color(CATEGORIES[ci]);
            scr_put(s, cx++, row, tri, cat_clr, bg, 1);
            scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);

            /* Category name */
            int name_max = w - 2 - (cx - x - 1) - 6;
            scr_str_n(s, cx, row, CATEGORIES[ci], name_max, cat_clr, bg, 1);

            /* Game count at right */
            int count = app->cat_counts[ci];
            char cbuf[8];
            snprintf(cbuf, sizeof(cbuf), "(%d)", count);
            int clen = (int)strlen(cbuf);
            scr_str_n(s, x + w - 2 - clen, row, cbuf, clen, CLR_HINT, bg, 0);
        } else {
            /* ── Game row ────────────────────────────────── */
            int gi = val;
            const Game *g = &GAMES[gi];
            int supported = game_supports_platform(g);
            int cloned = app->cloned[gi];

            Color bg = is_sel ? CLR_SELBG : CLR_BG;
            scr_fill(s, x + 1, row, w - 2, 1, bg);

            /* Status dot: ● ○ ◐ × */
            uint32_t dot;
            Color dot_color;
            if (!supported)              { dot = 0x00D7; dot_color = CLR_DARK; }     /* × */
            else if (app->installed[gi]) { dot = 0x25CF; dot_color = CLR_GREEN; }    /* ● */
            else if (cloned)             { dot = 0x25D0; dot_color = CLR_YELLOW; }   /* ◐ */
            else                         { dot = 0x25CB; dot_color = (Color){80,80,100}; } /* ○ */

            int cx = x + 1;
            /* Extra indent when in grouped (ALL) view */
            if (app->cat_index == 0 && app->search_len == 0) {
                scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);
                scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);
            }
            scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);
            scr_put(s, cx++, row, dot, dot_color, bg, 0);
            scr_put(s, cx++, row, ' ', CLR_WHITE, bg, 0);

            /* Name */
            Color name_color;
            if (!supported)              name_color = CLR_DARK;
            else if (app->installed[gi]) name_color = is_sel ? CLR_CYAN : CLR_WHITE;
            else if (cloned)             name_color = CLR_YELLOW;
            else                         name_color = (Color){140,140,150};

            int name_max = w - 2 - (cx - x - 1) - 2; /* leave room for arrow */
            scr_str_n(s, cx, row, g->name, name_max, name_color, bg, is_sel);

            /* Selection arrow at right edge */
            if (is_sel) {
                scr_put(s, x + w - 2, row, 0x25B8, CLR_CYAN, bg, 0); /* ▸ */
            }
        }
    }
}

/* ── Detail row helper ────────────────────────────────────────── */

static int detail_row(Screen *s, int x, int y, int w, const char *label, const char *value, Color val_color) {
    int cx = x;
    cx += scr_str_n(s, cx, y, label, w, CLR_LABEL, CLR_BG, 0);
    scr_str_n(s, cx, y, value, w - (cx - x), val_color, CLR_BG, 0);
    return 1;
}

/* ── Details panel ────────────────────────────────────────────── */

static void render_details(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);
    scr_box(s, x, y, w, h, CLR_BORDER);
    scr_box_title(s, x, y, w, "INFO", CLR_YELLOW, CLR_BORDER);

    if (app->filter_count == 0) return;
    int gi = app->filtered[app->selected];
    if (IS_HEADER(gi)) {
        /* Show category summary */
        int ci = HEADER_CAT(gi);
        int ix = x + 2, iw = w - 4, row = y + 2;
        Color cat_clr = category_color(CATEGORIES[ci]);
        scr_str_n(s, ix, row, CATEGORIES[ci], iw, cat_clr, CLR_BG, 1);
        row += 2;
        int count = app->cat_counts[ci];
        char info[64];
        snprintf(info, sizeof(info), "%d game%s", count, count == 1 ? "" : "s");
        scr_str_n(s, ix, row, info, iw, CLR_DIMWHITE, CLR_BG, 0);
        row += 2;
        scr_str_n(s, ix, row, "Enter", 5, CLR_CYAN, CLR_BG, 1);
        scr_str_n(s, ix + 6, row, app->cat_collapsed[ci] ? "to expand" : "to collapse",
                  iw - 6, CLR_HINT, CLR_BG, 0);
        return;
    }
    const Game *g = &GAMES[gi];
    int installed = app->installed[gi];
    int cloned = app->cloned[gi];
    int supported = game_supports_platform(g);
    int row = y + 1;
    int iw = w - 3; /* inner width */
    int ix = x + 2;

    /* Title */
    int tlen = scr_str_n(s, ix, row, g->name, iw, CLR_CYAN, CLR_BG, 1);

    /* Status badge */
    int badge_x = ix + tlen + 2;
    if (badge_x + 15 < ix + iw) {
        if (installed) {
            scr_badge(s, badge_x, row, "READY", CLR_BLACK, CLR_GREEN, 1);
        } else if (cloned) {
            scr_badge(s, badge_x, row, "CLONED", CLR_BLACK, CLR_YELLOW, 1);
        } else {
            scr_badge(s, badge_x, row, "NOT INSTALLED", CLR_WHITE, (Color){120,40,40}, 1);
        }
    }
    row += 2;

    /* Description — word wrap, max 3 lines */
    const char *desc = g->desc;
    int desc_lines = 0;
    while (*desc && row < y + h - 1 && desc_lines < 3) {
        int fit = 0;
        int last_space = -1;
        const char *p = desc;
        while (*p && fit < iw) {
            if (*p == ' ') last_space = fit;
            p++; fit++;
        }
        int take = fit;
        if (*p && last_space > 0) take = last_space + 1;
        scr_str_n(s, ix, row, desc, take, (Color){200,200,220}, CLR_BG, 0);
        desc += take;
        while (*desc == ' ') desc++;
        row++;
        desc_lines++;
    }
    if (*desc && desc_lines >= 3 && row > y + 1) {
        /* Truncate indicator on last desc line */
        scr_str(s, ix + iw - 3, row - 1, "...", CLR_HINT, CLR_BG, 0);
    }
    row++;

    /* Separator between description and metadata */
    if (row < y + h - 1) {
        scr_hline(s, ix, row, iw, 0x2500, CLR_SEPARATOR, CLR_BG);
        row++;
    }

    /* Metadata */
    if (row < y + h - 1) { detail_row(s, ix, row, iw, "Type      ", g->category, CLR_YELLOW); row++; }
    if (row < y + h - 1 && g->keys && g->keys[0]) {
        /* Show first 2 keys as preview */
        char preview[128] = "";
        for (int i = 0; g->keys[i] && i < 2; i++) {
            const char *pipe = strchr(g->keys[i], '|');
            if (pipe) {
                if (i > 0) strncat(preview, ", ", sizeof(preview) - strlen(preview) - 1);
                size_t klen = (size_t)(pipe - g->keys[i]);
                if (strlen(preview) + klen < sizeof(preview) - 1) {
                    strncat(preview, g->keys[i], klen);
                }
            }
        }
        strncat(preview, ", ...", sizeof(preview) - strlen(preview) - 1);
        detail_row(s, ix, row, iw, "Keys      ", preview, (Color){220,180,100});
        row++;
        /* Hint to open full controls view */
        if (row < y + h - 1) {
            int hx = ix + 10; /* align under value column */
            hx += scr_str_n(s, hx, row, "press ", iw - 10, CLR_HINT, CLR_BG, 0);
            hx += scr_str_n(s, hx, row, "c", iw - (hx - ix), CLR_CYAN, CLR_BG, 1);
            scr_str_n(s, hx, row, " for all controls", iw - (hx - ix), CLR_HINT, CLR_BG, 0);
            row++;
        }
    }
    row++;

    /* Separator between metadata and technicals */
    if (row < y + h - 1) {
        scr_hline(s, ix, row, iw, 0x2500, CLR_SEPARATOR, CLR_BG);
        row++;
    }

    /* Technical section */
    if (row < y + h - 1) { detail_row(s, ix, row, iw, "Engine    ", g->engine, (Color){200,160,100}); row++; }

    const Source *src = default_source(g);
    if (src && row < y + h - 1) {
        detail_row(s, ix, row, iw, "Method    ", src->label, (Color){180,140,220}); row++;
        if (src->bin[0] && row < y + h - 1) {
            detail_row(s, ix, row, iw, "Bin       ", src->bin, CLR_GREEN); row++;
        }
    }

    /* Runtime status */
    if (src && row < y + h - 1) {
        int avail = has_runtime(&app->toolchains, src->method);
        const char *mstr = acquire_str(src->method);
        int cx = ix;
        cx += scr_str_n(s, cx, row, "Runtime   ", iw, CLR_LABEL, CLR_BG, 0);
        cx += scr_str_n(s, cx, row, mstr, iw - (cx - ix),
                         avail ? CLR_GREEN : CLR_RED, CLR_BG, 0);
        scr_str_n(s, cx, row, avail ? " (found)" : " (missing!)", iw - (cx - ix),
                  avail ? CLR_DARKGRAY : CLR_RED, CLR_BG, 0);
        row++;
    }

    /* Platform */
    if (row < y + h - 1) {
        int cx = ix;
        cx += scr_str_n(s, cx, row, "Platform  ", iw, CLR_LABEL, CLR_BG, 0);
        if (!g->platforms) {
            scr_str_n(s, cx, row, "all", iw - (cx - ix), CLR_GREEN, CLR_BG, 0);
        } else {
            char platbuf[128] = "";
            for (int i = 0; g->platforms[i]; i++) {
                if (i > 0) strncat(platbuf, ", ", sizeof(platbuf) - strlen(platbuf) - 1);
                strncat(platbuf, g->platforms[i], sizeof(platbuf) - strlen(platbuf) - 1);
            }
            int pcx = cx;
            pcx += scr_str_n(s, pcx, row, platbuf, iw - (pcx - ix),
                             supported ? CLR_GREEN : (Color){140,140,150}, CLR_BG, 0);
            if (!supported) {
                char notbuf[64];
                snprintf(notbuf, sizeof(notbuf), " (not %s)", current_platform());
                scr_str_n(s, pcx, row, notbuf, iw - (pcx - ix), CLR_RED, CLR_BG, 0);
            }
        }
        row++;
    }

    /* Deps */
    const PlatformDeps *deps = platform_deps_for_current(g);
    if (deps && row < y + h - 1) {
        int sat = app->deps_satisfied[gi];
        int cx = ix;
        cx += scr_str_n(s, cx, row, "Deps      ", iw, CLR_LABEL, CLR_BG, 0);
        cx += scr_str_n(s, cx, row, deps->deps, iw - (cx - ix),
                         sat ? CLR_GREEN : CLR_YELLOW, CLR_BG, 0);
        scr_str_n(s, cx, row, sat ? " (installed)" : " (needed)", iw - (cx - ix),
                  sat ? CLR_DARKGRAY : CLR_YELLOW, CLR_BG, 0);
        row++;
    }

    /* Installed-via line */
    if (installed && row < y + h - 2 && app->install_methods[gi]) {
        detail_row(s, ix, row, iw, "Installed ", app->install_methods[gi], CLR_GREEN);
        row++;
    }

    /* Affordance line at bottom of panel */
    int afford_row = y + h - 2;
    if (afford_row > row) {
        if (!supported) {
            char nbuf[128];
            snprintf(nbuf, sizeof(nbuf), "-- Not available on %s", current_platform());
            scr_str_n(s, ix, afford_row, nbuf, iw, CLR_DARK, CLR_BG, 0);
        } else if (installed) {
            scr_str_n(s, ix, afford_row, ">> Enter to play  |  d to remove", iw, CLR_GREEN, CLR_BG, 1);
        } else if (cloned) {
            scr_str_n(s, ix, afford_row, ">> i to build", iw, CLR_YELLOW, CLR_BG, 1);
        } else {
            scr_str_n(s, ix, afford_row, ">> i to install", iw, CLR_YELLOW, CLR_BG, 1);
        }
    }
}

/* ── Install/running output panel ─────────────────────────────── */

static void render_output_panel(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);

    /* Determine title and border color */
    const char *title = app->panel_label;
    Color title_color = CLR_YELLOW;
    Color border_color = CLR_BORDER;

    if (app->child.done) {
        title = app->child.ok ? "DONE" : "FAILED";
        title_color = app->child.ok ? CLR_GREEN : CLR_RED;
    }

    /* Border flash on completion */
    if (app->border_flash > 0) {
        border_color = app->child.ok ? CLR_GREEN : CLR_RED;
    }

    scr_box(s, x, y, w, h, border_color);
    scr_box_title(s, x, y, w, title, title_color, border_color);

    int inner_w = w - 2;
    LineBuf *lb = &app->child.output;

    /* Reserve bottom row for status message when done */
    int inner_h = h - 2;
    if (app->child.done) inner_h--;

    /* Auto-scroll while running; clamp when done */
    int max_scroll = lb->count - inner_h;
    if (max_scroll < 0) max_scroll = 0;
    if (!app->child.done) {
        app->panel_scroll = max_scroll;
    } else if (app->panel_scroll > max_scroll) {
        app->panel_scroll = max_scroll;
    }

    int start = app->panel_scroll;
    for (int i = 0; i < inner_h && start + i < lb->count; i++) {
        const char *line = lb->lines[start + i];
        /* Colorize output lines */
        Color clr = CLR_DIMWHITE;
        if (strncmp(line, "   Compiling", 12) == 0)      clr = CLR_CYAN;
        else if (strncmp(line, " Downloading", 12) == 0)  clr = (Color){120,120,180};
        else if (strncmp(line, "  Downloaded", 12) == 0)  clr = (Color){120,120,180};
        else if (strncmp(line, "    Finished", 12) == 0)  clr = CLR_GREEN;
        else if (strncmp(line, "  Installing", 12) == 0)  clr = CLR_GREEN;
        else if (strncmp(line, "   Installed", 12) == 0)  clr = CLR_GREEN;
        else if (strncmp(line, "error", 5) == 0)          clr = CLR_RED;
        else if (strncmp(line, "Error", 5) == 0)          clr = CLR_RED;
        else if (strncmp(line, "warning", 7) == 0)        clr = CLR_YELLOW;
        else if (line[0] == '$')                          clr = (Color){180,180,220};

        scr_str_n(s, x + 1, y + 1 + i, line, inner_w, clr, CLR_BG, 0);
    }

    /* Status line at bottom — only when done */
    if (app->child.done) {
        int status_row = y + h - 2;
        int is_running = (app->mode == MODE_RUNNING);
        const char *msg = app->child.ok
            ? (is_running ? "Exited. Press any key to close." : "Done! Press any key to close.")
            : (is_running ? "Crashed! Press any key to close." : "Failed! Press any key to close.");
        scr_str_n(s, x + 2, status_row, msg, inner_w - 2,
                  app->child.ok ? CLR_GREEN : CLR_RED, CLR_BG, 1);
    }
}

/* ── Log view ─────────────────────────────────────────────────── */

static void render_log_view(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);
    scr_box(s, x, y, w, h, CLR_BORDER);
    scr_box_title(s, x, y, w, "LOG", CLR_YELLOW, CLR_BORDER);

    if (!app->last_log) return;

    int inner_h = h - 2;
    int inner_w = w - 2;

    /* Count lines and clamp scroll */
    int total_lines = 0;
    for (const char *c = app->last_log; *c; c++)
        if (*c == '\n') total_lines++;
    int max_scroll = total_lines - inner_h;
    if (max_scroll < 0) max_scroll = 0;
    if (app->log_scroll > max_scroll) app->log_scroll = max_scroll;

    int start = app->log_scroll;
    const char *p = app->last_log;
    int line_no = 0;
    while (line_no < start && *p) {
        if (*p == '\n') line_no++;
        p++;
    }

    for (int i = 0; i < inner_h && *p; i++) {
        const char *eol = strchr(p, '\n');
        int len = eol ? (int)(eol - p) : (int)strlen(p);
        if (len > inner_w) len = inner_w;

        char buf[512];
        int cpy = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
        memcpy(buf, p, (size_t)cpy);
        buf[cpy] = '\0';

        scr_str_n(s, x + 1, y + 1 + i, buf, inner_w, (Color){200,200,220}, CLR_BG, 0);
        p = eol ? eol + 1 : p + strlen(p);
    }
}

/* ── Controls view ────────────────────────────────────────────── */

static void render_controls(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);
    scr_box(s, x, y, w, h, CLR_CYAN);
    scr_box_title(s, x, y, w, "CONTROLS", CLR_CYAN, CLR_CYAN);

    if (app->filter_count == 0) return;
    int gi = app->filtered[app->selected];
    if (IS_HEADER(gi)) return;
    const Game *g = &GAMES[gi];

    int ix = x + 2;
    int iw = w - 4;
    int row = y + 2;

    /* Game name */
    scr_str_n(s, ix, row, g->name, iw, CLR_CYAN, CLR_BG, 1);
    row += 2;

    if (!g->keys || !g->keys[0]) {
        scr_str_n(s, ix, row, "No controls listed.", iw, CLR_HINT, CLR_BG, 0);
        return;
    }

    /* Column widths: find widest key */
    int key_col = 0;
    for (int i = 0; g->keys[i]; i++) {
        const char *pipe = strchr(g->keys[i], '|');
        if (pipe) {
            int klen = (int)(pipe - g->keys[i]);
            if (klen > key_col) key_col = klen;
        }
    }
    key_col += 2; /* padding */
    if (key_col > iw / 2) key_col = iw / 2;

    /* Scroll support */
    int num_keys = 0;
    while (g->keys[num_keys]) num_keys++;

    int inner_h = h - 6; /* minus border, title, game name, bottom */
    if (inner_h < 1) inner_h = 1;
    int max_scroll = num_keys - inner_h;
    if (max_scroll < 0) max_scroll = 0;
    if (app->panel_scroll > max_scroll) app->panel_scroll = max_scroll;

    int start = app->panel_scroll;
    for (int i = 0; i < inner_h && start + i < num_keys; i++) {
        const char *entry = g->keys[start + i];
        const char *pipe = strchr(entry, '|');
        if (!pipe) {
            scr_str_n(s, ix, row, entry, iw, CLR_DIMWHITE, CLR_BG, 0);
        } else {
            int klen = (int)(pipe - entry);
            char keybuf[64];
            int cpy = klen < (int)sizeof(keybuf) - 1 ? klen : (int)sizeof(keybuf) - 1;
            memcpy(keybuf, entry, (size_t)cpy);
            keybuf[cpy] = '\0';

            scr_str_n(s, ix, row, keybuf, key_col, CLR_YELLOW, CLR_BG, 1);
            scr_str_n(s, ix + key_col, row, pipe + 1, iw - key_col, CLR_DIMWHITE, CLR_BG, 0);
        }
        row++;
    }

    /* Scroll indicator */
    if (max_scroll > 0) {
        int indicator_row = y + h - 2;
        char sbuf[32];
        snprintf(sbuf, sizeof(sbuf), "[%d/%d]", start + 1, num_keys);
        scr_str_n(s, ix, indicator_row, sbuf, iw, CLR_HINT, CLR_BG, 0);
    }
}

/* ── Source selection overlay ─────────────────────────────────── */

static void render_source_select(Screen *s, App *app, int x, int y, int w, int h) {
    scr_fill(s, x, y, w, h, CLR_BG);
    scr_box(s, x, y, w, h, CLR_YELLOW);
    scr_box_title(s, x, y, w, "SELECT SOURCE", CLR_YELLOW, CLR_YELLOW);

    if (app->filter_count == 0) return;
    int gi = app->active_game;
    const Game *g = &GAMES[gi];

    int ix = x + 2;
    int iw = w - 4;
    int row = y + 2;

    /* Game name */
    scr_str_n(s, ix, row, g->name, iw, CLR_CYAN, CLR_BG, 1);
    row += 2;

    /* List sources */
    for (int i = 0; i < g->num_sources && row < y + h - 2; i++) {
        const Source *src = &g->sources[i];
        int is_sel = (i == app->source_selected);
        Color bg = is_sel ? CLR_SELBG : CLR_BG;
        scr_fill(s, x + 1, row, w - 2, 1, bg);

        int cx = ix;
        /* Number */
        char num[16];
        snprintf(num, sizeof(num), "%d.", i + 1);
        cx += scr_str_n(s, cx, row, num, 3, CLR_CYAN, bg, 1);
        cx += scr_str_n(s, cx, row, " ", 1, CLR_WHITE, bg, 0);

        /* Label */
        cx += scr_str_n(s, cx, row, src->label, iw - (cx - ix), CLR_WHITE, bg, 0);

        /* Method badge */
        const char *mstr = acquire_str(src->method);
        int badge_w = (int)strlen(mstr) + 2;
        if (cx + badge_w + 2 < ix + iw) {
            cx += scr_str_n(s, cx, row, "  ", 2, CLR_BG, bg, 0);
            scr_badge(s, cx, row, mstr, CLR_BLACK,
                      src->method == ACQUIRE_CARGO ? CLR_CYAN : CLR_GREEN, 0);
        }

        /* Selection arrow */
        if (is_sel) {
            scr_put(s, x + w - 2, row, 0x25B8, CLR_CYAN, bg, 0);
        }

        row++;
    }
}

/* ── Footer (row h-2 = separator, row h-1 = keys + status) ──── */

static void render_footer(Screen *s, App *app, int y) {
    int w = s->w;

    /* Row y: separator */
    scr_hline(s, 0, y, w, 0x2500, CLR_BORDER, CLR_BG);

    /* Row y+1: keys left, status right */
    int ky = y + 1;
    scr_fill(s, 0, ky, w, 1, CLR_BG);

    int cx = 1;

    if (app->mode == MODE_INSTALLING || app->mode == MODE_RUNNING) {
        if (app->child.done) {
            cx += scr_str_n(s, cx, ky, "Any key", w - 2, CLR_CYAN, CLR_BG, 1);
            cx += scr_str_n(s, cx, ky, " close", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        } else {
            cx += scr_str_n(s, cx, ky, "j/k", w - 2, CLR_CYAN, CLR_BG, 1);
            cx += scr_str_n(s, cx, ky, " scroll", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
            if (app->mode == MODE_RUNNING) {
                cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
                cx += scr_str_n(s, cx, ky, "Esc", w - cx - 1, CLR_RED, CLR_BG, 1);
                cx += scr_str_n(s, cx, ky, " stop", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
            }
        }
    } else if (app->mode == MODE_VIEWLOG) {
        cx += scr_str_n(s, cx, ky, "j/k", w - 2, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " scroll", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "PgUp/PgDn", w - cx - 1, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " page", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Esc", w - cx - 1, CLR_RED, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " close", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
    } else if (app->mode == MODE_CONTROLS) {
        /* Same as VIEWLOG minus PgUp/PgDn */
        cx += scr_str_n(s, cx, ky, "j/k", w - 2, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " scroll", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Up/Down", w - cx - 1, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " scroll", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Esc", w - cx - 1, CLR_RED, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " close", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
    } else if (app->mode == MODE_SOURCE_SELECT) {
        cx += scr_str_n(s, cx, ky, "j/k", w - 2, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " select", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Enter", w - cx - 1, CLR_GREEN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " confirm", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Esc", w - cx - 1, CLR_RED, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " cancel", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
    } else if (app->mode == MODE_SEARCH) {
        cx += scr_str_n(s, cx, ky, "Type", w - cx - 1, CLR_CYAN, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " to search", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "Enter/Esc", w - cx - 1, CLR_YELLOW, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " done", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
    } else {
        /* Normal mode keys */
        struct { const char *key; const char *label; Color color; } keys[] = {
            {"j/k", "nav", CLR_CYAN},
            {"h/l", "cat", (Color){180,140,220}},
            {"p", "platform", (Color){220,180,50}},
            {"/", "search", CLR_YELLOW},
            {"Enter", "play", CLR_GREEN},
            {"i", "install", CLR_YELLOW},
            {"d", "remove", CLR_RED},
            {"c", "keys", (Color){220,180,100}},
            {NULL, NULL, CLR_NONE},
        };
        for (int ki = 0; keys[ki].key; ki++) {
            if (ki > 0) {
                cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
            }
            cx += scr_str_n(s, cx, ky, keys[ki].key, w - cx - 1, keys[ki].color, CLR_BG, 1);
            cx += scr_str_n(s, cx, ky, " ", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
            cx += scr_str_n(s, cx, ky, keys[ki].label, w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
        }
        cx += scr_str_n(s, cx, ky, "  ", w - cx - 1, CLR_BG, CLR_BG, 0);
        cx += scr_str_n(s, cx, ky, "q", w - cx - 1, (Color){180,80,80}, CLR_BG, 1);
        cx += scr_str_n(s, cx, ky, " quit", w - cx - 1, CLR_DARKGRAY, CLR_BG, 0);
    }

    /* Right side: status message */
    if (app->has_message) {
        int mlen = (int)strlen(app->message);
        int mx = w - mlen - 2;
        if (mx > cx + 2) {
            scr_str_n(s, mx, ky, app->message, mlen, app->msg_ok ? CLR_GREEN : CLR_RED, CLR_BG, 1);
        }
    }
}

/* ── Main draw ────────────────────────────────────────────────── */

void ui_draw(Screen *s, App *app) {
    screen_clear(s, CLR_BG);

    int header_h = 2;  /* row 0 = header, row 1 = separator */
    int footer_h = 2;  /* row h-2 = separator, row h-1 = keys */
    int main_h = s->h - header_h - footer_h;
    if (main_h < 4) main_h = 4;

    /* Decrement border flash */
    if (app->border_flash > 0) app->border_flash--;

    render_header(s, app);

    int list_w = 28;
    /* Narrow mode */
    if (s->w < 70) list_w = 22;

    if (app->mode == MODE_VIEWLOG) {
        render_log_view(s, app, 0, header_h, s->w, main_h);
    } else if (app->mode == MODE_INSTALLING || app->mode == MODE_RUNNING) {
        /* Two-column: list + output */
        int output_w = s->w - list_w;
        render_game_list(s, app, 0, header_h, list_w, main_h);
        render_output_panel(s, app, list_w, header_h, output_w, main_h);
    } else if (app->mode == MODE_CONTROLS) {
        /* Two-column: list + controls */
        int detail_w = s->w - list_w;
        render_game_list(s, app, 0, header_h, list_w, main_h);
        render_controls(s, app, list_w, header_h, detail_w, main_h);
    } else if (app->mode == MODE_SOURCE_SELECT) {
        /* Two-column: list + source picker */
        int detail_w = s->w - list_w;
        render_game_list(s, app, 0, header_h, list_w, main_h);
        render_source_select(s, app, list_w, header_h, detail_w, main_h);
    } else {
        /* Two-column: list + details */
        int detail_w = s->w - list_w;
        render_game_list(s, app, 0, header_h, list_w, main_h);
        render_details(s, app, list_w, header_h, detail_w, main_h);
    }

    render_footer(s, app, s->h - footer_h);
}
