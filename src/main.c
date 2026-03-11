#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE       /* glibc: expose SIGWINCH alongside _POSIX_C_SOURCE */
#ifdef __APPLE__
#define _DARWIN_C_SOURCE      /* macOS: expose SIGWINCH alongside _POSIX_C_SOURCE */
#endif
#include <signal.h>
#endif

#include "ui/term.h"
#include "ui/ui.h"
#include "core/catalog.h"
#include "core/install.h"
#include "core/util.h"
#include "core/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
static volatile int g_resize = 0;

static void sigwinch_handler(int sig) {
    (void)sig;
    g_resize = 1;
}
#endif

/* ── Command chain helpers ────────────────────────────────────── */

/* Duplicate a NULL-terminated command array (caller must free_cmd) */
static char **dup_cmd(const char **cmd) {
    int n = 0;
    while (cmd[n]) n++;
    char **out = malloc((size_t)(n + 1) * sizeof(char *));
    for (int i = 0; i < n; i++) out[i] = strdup(cmd[i]);
    out[n] = NULL;
    return out;
}

/* Set the next command to run after current child exits OK */
static void chain_next(App *app, const char **cmd, const char *cwd) {
    app->next_cmd = dup_cmd(cmd);
    app->next_cwd = cwd ? strdup(cwd) : NULL;
}

/* Start the next chained command (if any). Returns 1 if started. */
static int chain_advance(App *app) {
    if (!app->next_cmd) return 0;
    char **cmd = app->next_cmd;
    char *cwd = app->next_cwd;
    app->next_cmd = NULL;
    app->next_cwd = NULL;
    child_start(&app->child, (const char **)cmd, cwd);
    free_cmd(cmd);
    free(cwd);
    return 1;
}

static void chain_clear(App *app) {
    if (app->next_cmd) { free_cmd(app->next_cmd); app->next_cmd = NULL; }
    free(app->next_cwd); app->next_cwd = NULL;
}

/* ── Forward declarations ─────────────────────────────────────── */

static void begin_install(App *app, const Source *src);
static void begin_uninstall(App *app, const Source *src);
static int  try_install_source(App *app, Screen *scr, int *w, int *h);

/* ── Source cwd helper ─────────────────────────────────────────── */

/* Fill buf with the game directory for local-install sources; return buf or NULL */
static const char *source_cwd(const Source *src, char *buf, size_t buflen) {
    if (src->method == ACQUIRE_GIT || src->method == ACQUIRE_DOWNLOAD) {
        char *gdir = games_dir();
        snprintf(buf, buflen, "%s/%s", gdir, src->dir);
        free(gdir);
        return buf;
    }
    return NULL;
}

/* ── Game launch helper ──────────────────────────────────────── */

static void launch_game(App *app, Screen *scr, int *w, int *h) {
    int gi = app->filtered[app->selected];
    if (app->game_running) {
        app_set_message(app, "A game is already running.", 0);
        return;
    }
    app->active_game = gi;
    const Game *g = &GAMES[gi];
    if (app->cloned[gi]) {
        app_set_message(app, "Cloned but not built. Press [i] to build.", 0);
        return;
    }
    if (!app->installed[gi]) {
        app_set_message(app, "Not installed. Press [i] to install.", 0);
        return;
    }
    const Source *src = default_source(g);
    if (!src) return;

    int is_terminal = (strcmp(g->engine, "crossterm") == 0 ||
                       strcmp(g->engine, "ratatui") == 0 ||
                       strcmp(g->engine, "ncurses") == 0);

    const char *cmd[16];
    char cwd_buf[PATHBUF];
    const char *cwd = source_cwd(src, cwd_buf, sizeof(cwd_buf));
    int ci = 0;

    if (src->play_cmd && src->play_cmd[0]) {
        for (int i = 0; src->play_cmd[i] && ci < 15; i++)
            cmd[ci++] = src->play_cmd[i];
    } else {
        cmd[ci++] = src->bin;
    }
    cmd[ci] = NULL;

    if (is_terminal) {
        screen_resize(scr, *w, *h);
        int ok = run_visible(cmd, cwd);
        term_get_size(w, h);
        screen_resize(scr, *w, *h);
        if (!ok) {
            char msg[128];
            snprintf(msg, sizeof(msg), "%s exited with error", g->name);
            app_set_message(app, msg, 0);
        } else {
            app_clear_message(app);
        }
    } else {
        child_start(&app->child, cmd, cwd);
        app->game_running = 1;
        char msg[128];
        snprintf(msg, sizeof(msg), "Playing %s...", g->name);
        app_set_message(app, msg, 1);
    }
}

/* ── Install/uninstall handler ───────────────────────────────── */

static void handle_install_key(App *app, Screen *scr, int *w, int *h, int installing) {
    if (app->game_running) {
        app_set_message(app, "A game is running. Close it first.", 0);
        return;
    }
    if (app->filter_count == 0) return;
    int idx = app->filtered[app->selected];
    if (IS_HEADER(idx)) return;

    app->active_game = idx;
    const Game *g = &GAMES[idx];
    const Source *src = default_source(g);

    if (installing && !game_supports_platform(g)) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s not supported on %s", g->name, current_platform());
        app_set_message(app, msg, 0);
    } else if (!src) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s has no install source", g->name);
        app_set_message(app, msg, 0);
    } else if (installing && app->installed[idx]) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s already installed!", g->name);
        app_set_message(app, msg, 1);
    } else if (!installing && !app->installed[idx]) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s not installed", g->name);
        app_set_message(app, msg, 0);
    } else if (installing) {
        int dsi = default_source_index(g);
        app->source_selected = dsi >= 0 ? dsi : 0;
        if (count_platform_sources(g) > 1) {
            app->mode = MODE_SOURCE_SELECT;
        } else {
            try_install_source(app, scr, w, h);
        }
    } else {
        int dsi = default_source_index(g);
        app->source_selected = dsi >= 0 ? dsi : 0;
        begin_uninstall(app, src);
    }
}

/* ── Helpers to build & start install/uninstall ───────────────── */

static void start_cargo_install(App *app, const Source *src) {
    const char *cmd[] = {"cargo", "install", src->bin, NULL};
    child_start(&app->child, cmd, NULL);
}

static void start_cargo_uninstall(App *app, const Source *src) {
    if (!src->uninstall_cmd || !src->uninstall_cmd[0]) return;
    child_start(&app->child, src->uninstall_cmd, NULL);
}

static void start_git_acquire(App *app, const Source *src) {
    char *gdir = games_dir();
    char game_path[PATHBUF], git_path[PATHBUF + 6];
    snprintf(game_path, sizeof(game_path), "%s/%s", gdir, src->dir);
    snprintf(git_path, sizeof(git_path), "%s/.git", game_path);

    /* Chain the build command (runs after acquire finishes) */
    if (src->build_cmd && src->build_cmd[0]) {
        chain_next(app, src->build_cmd, game_path);
    }

    if (plat_file_exists(git_path)) {
        /* Already cloned — pull latest */
        const char *cmd[] = {"git", "pull", NULL};
        child_start(&app->child, cmd, game_path);
    } else {
        /* Fresh clone */
        if (src->shallow) {
            const char *cmd[] = {"git", "clone", "--depth", "1",
                                 src->url, game_path, NULL};
            child_start(&app->child, cmd, NULL);
        } else {
            const char *cmd[] = {"git", "clone", src->url, game_path, NULL};
            child_start(&app->child, cmd, NULL);
        }
    }
    free(gdir);
}

static void start_local_remove(App *app, const Source *src) {
    char *gdir = games_dir();
    char path[PATHBUF];
    snprintf(path, sizeof(path), "%s/%s", gdir, src->dir);
    free(gdir);

    /* Remove synchronously (directory delete is fast) */
    linebuf_init(&app->child.output);
    char msg[1024];
    snprintf(msg, sizeof(msg), "Removing %s...", src->dir);
    linebuf_push(&app->child.output, msg);

    if (plat_rmdir_rf(path)) {
        linebuf_push(&app->child.output, "Removed!");
        app->child.ok = 1;
    } else {
        linebuf_push(&app->child.output, "Error: failed to remove directory");
        app->child.ok = 0;
    }
    app->child.done = 1;
}

static void start_download_acquire(App *app, const Source *src) {
    char *gdir = games_dir();
    char game_path[PATHBUF];
    snprintf(game_path, sizeof(game_path), "%s/%s", gdir, src->dir);

    /* Chain the build command if any */
    if (src->build_cmd && src->build_cmd[0]) {
        chain_next(app, src->build_cmd, game_path);
    }

    char script[4096];
    const char *at = src->archive_type;

#ifdef _WIN32
    /* ── Windows: PowerShell + curl.exe/tar.exe ──────────────── */
    int is_tar = at && (strcmp(at, "tar.gz") == 0 ||
                        strcmp(at, "tar.bz2") == 0 ||
                        strcmp(at, "tar.xz") == 0);
    if (is_tar) {
        snprintf(script, sizeof(script),
            "$ErrorActionPreference='Stop'\n"
            "New-Item -ItemType Directory -Force -Path '%s' | Out-Null\n"
            "Write-Host 'Downloading %s...'\n"
            "$dl = Join-Path $env:TEMP ('gp_dl_' + [System.IO.Path]::GetRandomFileName() + '.tar')\n"
            "curl.exe -fSL '%s' -o $dl\n"
            "tar.exe xf $dl -C '%s' --strip-components=1\n"
            "Remove-Item $dl -Force -ErrorAction SilentlyContinue\n"
            "Write-Host 'Done!'",
            game_path, src->dir, src->url, game_path);
    } else if (at && strcmp(at, "zip") == 0) {
        snprintf(script, sizeof(script),
            "$ErrorActionPreference='Stop'\n"
            "New-Item -ItemType Directory -Force -Path '%s' | Out-Null\n"
            "$dl = Join-Path $env:TEMP ('gp_dl_' + [System.IO.Path]::GetRandomFileName() + '.zip')\n"
            "Write-Host 'Downloading %s...'\n"
            "curl.exe -fSL -o $dl '%s'\n"
            "Expand-Archive -Path $dl -DestinationPath '%s' -Force\n"
            "Remove-Item $dl -Force -ErrorAction SilentlyContinue\n"
            "Write-Host 'Done!'",
            game_path, src->dir, src->url, game_path);
    } else {
        /* Raw binary / exe download */
        snprintf(script, sizeof(script),
            "$ErrorActionPreference='Stop'\n"
            "New-Item -ItemType Directory -Force -Path '%s' | Out-Null\n"
            "Write-Host 'Downloading %s...'\n"
            "curl.exe -fSL -o '%s\\%s' '%s'\n"
            "Write-Host 'Done!'",
            game_path, src->dir,
            game_path, src->bin, src->url);
    }
    const char *cmd[] = {"powershell", "-NoProfile", "-Command", script, NULL};
#else
    /* ── POSIX: bash + curl + tar/unzip ──────────────────────── */
    int is_tar = at && (strcmp(at, "tar.gz") == 0 ||
                        strcmp(at, "tar.bz2") == 0 ||
                        strcmp(at, "tar.xz") == 0);
    if (is_tar) {
        const char *flag = "z";
        if (strcmp(at, "tar.bz2") == 0) flag = "j";
        else if (strcmp(at, "tar.xz") == 0) flag = "J";
        snprintf(script, sizeof(script),
            "set -e\n"
            "mkdir -p '%s'\n"
            "echo 'Downloading %s...'\n"
            "curl -fSL '%s' | tar x%s -C '%s' --strip-components=1\n"
            "echo 'Done!'",
            game_path, src->dir, src->url, flag, game_path);
    } else if (at && strcmp(at, "zip") == 0) {
        snprintf(script, sizeof(script),
            "set -e\n"
            "mkdir -p '%s'\n"
            "dl=$(mktemp /tmp/gp_dl.XXXXXX.zip)\n"
            "trap 'rm -f \"$dl\"' EXIT\n"
            "echo 'Downloading %s...'\n"
            "curl -fSL -o \"$dl\" '%s'\n"
            "unzip -o \"$dl\" -d '%s'\n"
            "echo 'Done!'",
            game_path, src->dir, src->url, game_path);
    } else {
        /* Raw binary download */
        snprintf(script, sizeof(script),
            "set -e\n"
            "mkdir -p '%s'\n"
            "echo 'Downloading %s...'\n"
            "curl -fSL -o '%s/%s' '%s'\n"
            "chmod +x '%s/%s'\n"
            "echo 'Done!'",
            game_path, src->dir,
            game_path, src->bin, src->url,
            game_path, src->bin);
    }
    const char *cmd[] = {"bash", "-c", script, NULL};
#endif

    child_start(&app->child, cmd, NULL);
    free(gdir);
}

/* ── Start install/uninstall with a chosen source ────────────── */

static void begin_install(App *app, const Source *src) {
    switch (src->method) {
    case ACQUIRE_CARGO:    start_cargo_install(app, src);    break;
    case ACQUIRE_GIT:      start_git_acquire(app, src);      break;
    case ACQUIRE_DOWNLOAD: start_download_acquire(app, src); break;
    default: break;
    }
    app->panel_label = "INSTALLING";
    app->mode = MODE_INSTALLING;
    app->panel_scroll = 0;
    app_clear_message(app);
}

static void begin_uninstall(App *app, const Source *src) {
    kill_game_process(src->bin);
    switch (src->method) {
    case ACQUIRE_CARGO:    start_cargo_uninstall(app, src); break;
    case ACQUIRE_GIT:      start_local_remove(app, src);      break;
    case ACQUIRE_DOWNLOAD: start_local_remove(app, src);      break;
    default: break;
    }
    app->panel_label = "REMOVING";
    app->mode = MODE_INSTALLING;
    app->panel_scroll = 0;
    app_clear_message(app);
}

/* ── Close install/run panel ───────────────────────────────────── */

/* Returns a static buffer with the log file path (valid until next call) */
static const char *save_log_to_file(App *app) {
    static char path[PATHBUF];
    char *ldir = logs_dir();
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    char safe_name[128];
    sanitize_name(GAMES[app->active_game].name, safe_name, sizeof(safe_name));

    snprintf(path, sizeof(path), "%s/%s_%04d%02d%02d_%02d%02d%02d.log",
             ldir, safe_name,
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    free(ldir);

    FILE *f = fopen(path, "w");
    if (f) {
        for (int i = 0; i < app->child.output.count; i++)
            fprintf(f, "%s\n", app->child.output.lines[i]);
        fclose(f);
        return path;
    }
    return NULL;
}

/* Try to install the currently selected source; returns 1 if install started */
static int try_install_source(App *app, Screen *scr, int *w, int *h) {
    const Game *g = &GAMES[app->active_game];
    const Source *src = &g->sources[app->source_selected];

    if (!has_runtime(&app->toolchains, src->method)) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Missing %s! %s",
                 acquire_str(src->method), runtime_install_hint(src->method));
        app_set_message(app, msg, 0);
        app->mode = MODE_NORMAL;
        return 0;
    }

    /* Install platform deps if needed */
    const PlatformDeps *deps = platform_deps_for_current(g);
    if (deps && deps->install_cmd && deps->install_cmd[0]
        && !deps_check_satisfied(deps)) {
        screen_resize(scr, *w, *h);
        int ok = run_visible(deps->install_cmd, NULL);
        term_get_size(w, h);
        screen_resize(scr, *w, *h);
        if (!ok) {
            app_set_message(app, "Dep install failed. Press [i] to retry.", 0);
            app->mode = MODE_NORMAL;
            return 0;
        }
    }

    begin_install(app, src);
    return 1;
}

static void close_panel(App *app) {
    chain_clear(app);
    int ok = app->child.ok;
    const Game *g = &GAMES[app->active_game];

    /* Track install method */
    if (ok) {
        int was_installing = (strcmp(app->panel_label, "INSTALLING") == 0);
        if (was_installing) {
            int si = app->source_selected;
            if (si >= 0 && si < g->num_sources)
                save_install_method(g->name, g->sources[si].label);
        } else {
            clear_install_method(g->name);
        }
    }

    /* Save log to file (before building in-memory log so path is excluded from file) */
    const char *log_path = save_log_to_file(app);

    /* Append log path to output so it shows in [L] view */
    if (log_path) {
        char note[PATHBUF + 16];
        snprintf(note, sizeof(note), "Log saved: %s", log_path);
        linebuf_push(&app->child.output, note);
    }

    /* Save in-memory log for [L] key (single-pass to avoid O(n^2) strcat) */
    free(app->last_log);
    size_t total = 0;
    for (int i = 0; i < app->child.output.count; i++)
        total += strlen(app->child.output.lines[i]) + 1;
    app->last_log = malloc(total + 1);
    char *cursor = app->last_log;
    for (int i = 0; i < app->child.output.count; i++) {
        size_t len = strlen(app->child.output.lines[i]);
        memcpy(cursor, app->child.output.lines[i], len);
        cursor += len;
        *cursor++ = '\n';
    }
    *cursor = '\0';

    child_cleanup(&app->child);
    app_refresh(app);
    app_rebuild_filter(app);

    if (ok) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s done!", GAMES[app->active_game].name);
        app_set_message(app, msg, 1);
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s failed. Press [L] for log.", GAMES[app->active_game].name);
        app_set_message(app, msg, 0);
    }
    app->mode = MODE_NORMAL;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
#ifndef _WIN32
    /* Handle SIGWINCH for terminal resize */
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);
#endif

    catalog_init();
    term_init();

    int w, h;
    term_get_size(&w, &h);
    Screen *scr = screen_create(w, h);

    App app;
    app_init(&app);

    while (!app.should_quit) {
        /* Handle resize */
#ifndef _WIN32
        if (g_resize) {
            g_resize = 0;
            term_get_size(&w, &h);
            screen_resize(scr, w, h);
        }
#else
        {
            int nw, nh;
            term_get_size(&nw, &nh);
            if (nw != w || nh != h) {
                w = nw; h = nh;
                screen_resize(scr, w, h);
            }
        }
#endif

        /* Poll child process for output */
        if ((app.mode == MODE_INSTALLING || app.mode == MODE_RUNNING) && !app.child.done) {
            child_poll(&app.child);
            /* If child finished OK and there's a chained command, start it */
            if (app.child.done && app.child.ok && app.next_cmd) {
                plat_proc_close(&app.child.proc);
                chain_advance(&app);
            }
        }

        /* Poll background game process */
        if (app.game_running && !app.child.done) {
            child_poll(&app.child);
        }
        if (app.game_running && app.child.done) {
            app.game_running = 0;
            child_cleanup(&app.child);
            if (!app.child.ok) {
                char msg[128];
                snprintf(msg, sizeof(msg), "%s exited with error",
                         GAMES[app.active_game].name);
                app_set_message(&app, msg, 0);
            } else {
                app_clear_message(&app);
            }
        }

        /* Draw */
        ui_draw(scr, &app);
        screen_flush(scr);
        app.tick++;

        /* Lazily compute disk size for the selected game (outside render) */
        if (app.mode == MODE_NORMAL && app.filter_count > 0 &&
            !IS_HEADER(app.filtered[app.selected])) {
            app_ensure_disk_size(&app, app.filtered[app.selected]);
        }

        /* Poll for input (100ms timeout for animations) */
        KeyEvent key = term_poll_key(100);
        if (key.type == KEY_NONE) continue;

        switch (app.mode) {
        case MODE_INSTALLING:
        case MODE_RUNNING:
            switch (key.type) {
            case KEY_DOWN:
                app.panel_scroll++;
                break;
            case KEY_UP:
                if (app.panel_scroll > 0) app.panel_scroll--;
                break;
            case KEY_CHAR:
                if (key.ch == 'j') app.panel_scroll++;
                else if (key.ch == 'k' && app.panel_scroll > 0) app.panel_scroll--;
                else if (app.child.done) close_panel(&app);
                break;
            case KEY_ESC:
            case KEY_ENTER:
                if (app.child.done) {
                    close_panel(&app);
                } else if (key.type == KEY_ESC) {
                    chain_clear(&app);
                    child_kill(&app.child);
                    child_cleanup(&app.child);
                    app_refresh(&app);
                    app_rebuild_filter(&app);
                    app.mode = MODE_NORMAL;
                    app_clear_message(&app);
                }
                break;
            default:
                if (app.child.done) close_panel(&app);
                break;
            }
            break;

        case MODE_VIEWLOG:
            switch (key.type) {
            case KEY_ESC:
                app.mode = MODE_NORMAL;
                break;
            case KEY_DOWN:
                app.log_scroll++;
                break;
            case KEY_UP:
                if (app.log_scroll > 0) app.log_scroll--;
                break;
            case KEY_PGDN:
                app.log_scroll += 10;
                break;
            case KEY_PGUP:
                if (app.log_scroll >= 10) app.log_scroll -= 10;
                else app.log_scroll = 0;
                break;
            case KEY_CHAR:
                if (key.ch == 'j') app.log_scroll++;
                else if (key.ch == 'k' && app.log_scroll > 0) app.log_scroll--;
                else if (key.ch == 'q' || key.ch == 'l') app.mode = MODE_NORMAL;
                break;
            default:
                break;
            }
            break;

        case MODE_CONTROLS:
            switch (key.type) {
            case KEY_ESC:
                app.mode = MODE_NORMAL;
                break;
            case KEY_DOWN:
                app.panel_scroll++;
                break;
            case KEY_UP:
                if (app.panel_scroll > 0) app.panel_scroll--;
                break;
            case KEY_CHAR:
                if (key.ch == 'j') app.panel_scroll++;
                else if (key.ch == 'k' && app.panel_scroll > 0) app.panel_scroll--;
                else if (key.ch == 'q' || key.ch == 'c') app.mode = MODE_NORMAL;
                break;
            default:
                break;
            }
            break;

        case MODE_SOURCE_SELECT: {
            const Game *g = &GAMES[app.active_game];
            switch (key.type) {
            case KEY_ESC:
                app.mode = MODE_NORMAL;
                break;
            case KEY_DOWN:
                if (app.source_selected < g->num_sources - 1) app.source_selected++;
                break;
            case KEY_UP:
                if (app.source_selected > 0) app.source_selected--;
                break;
            case KEY_ENTER:
                try_install_source(&app, scr, &w, &h);
                break;
            case KEY_CHAR:
                if (key.ch == 'j' && app.source_selected < g->num_sources - 1)
                    app.source_selected++;
                else if (key.ch == 'k' && app.source_selected > 0)
                    app.source_selected--;
                else if (key.ch >= '1' && key.ch <= '9') {
                    int idx = key.ch - '1';
                    if (idx < g->num_sources) {
                        app.source_selected = idx;
                        try_install_source(&app, scr, &w, &h);
                    }
                }
                break;
            default:
                break;
            }
            break;
        }

        case MODE_SEARCH:
            if (key.type == KEY_ESC || key.type == KEY_ENTER) {
                app.mode = MODE_NORMAL;
            } else if (key.type == KEY_BACKSPACE) {
                if (app.search_len > 0) {
                    app.search[--app.search_len] = '\0';
                    app_rebuild_filter(&app);
                }
            } else if (key.type == KEY_CHAR && app.search_len < 62) {
                app.search[app.search_len++] = (char)key.ch;
                app.search[app.search_len] = '\0';
                app_rebuild_filter(&app);
            }
            break;

        case MODE_NORMAL:
            switch (key.type) {
            case KEY_ESC:
                if (app.search_len > 0) {
                    /* Clear search first */
                    app.search_len = 0;
                    app.search[0] = '\0';
                    app_rebuild_filter(&app);
                } else {
                    app.should_quit = 1;
                }
                break;
            case KEY_DOWN:
                app_next(&app);
                break;
            case KEY_UP:
                app_prev(&app);
                break;
            case KEY_LEFT:
                if (app.cat_index > 0) {
                    app.cat_index--;
                    app.selected = 0;
                    app_rebuild_filter(&app);
                }
                break;
            case KEY_RIGHT:
                if (app.cat_index < NUM_CATEGORIES - 1) {
                    app.cat_index++;
                    app.selected = 0;
                    app_rebuild_filter(&app);
                }
                break;
            case KEY_ENTER: {
                if (app.filter_count == 0) break;
                int gi = app.filtered[app.selected];
                if (IS_HEADER(gi)) {
                    int ci = HEADER_CAT(gi);
                    app.cat_collapsed[ci] = !app.cat_collapsed[ci];
                    app_rebuild_filter(&app);
                    break;
                }
                launch_game(&app, scr, &w, &h);
                break;
            }
            case KEY_CHAR: {
                if (key.ch == 'q') {
                    app.should_quit = 1;
                } else if (key.ch == 'j') {
                    app_next(&app);
                } else if (key.ch == 'k') {
                    app_prev(&app);
                } else if (key.ch == '/') {
                    app.mode = MODE_SEARCH;
                    app.search_len = 0;
                    app.search[0] = '\0';
                    app_rebuild_filter(&app);
                } else if (key.ch == 'h') {
                    if (app.cat_index > 0) {
                        app.cat_index--;
                        app.selected = 0;
                        app_rebuild_filter(&app);
                    }
                } else if (key.ch == 'l') {
                    if (app.cat_index < NUM_CATEGORIES - 1) {
                        app.cat_index++;
                        app.selected = 0;
                        app_rebuild_filter(&app);
                    }
                } else if (key.ch == 'r') {
                    app_refresh(&app);
                    app_rebuild_filter(&app);
                    app_set_message(&app, "Refreshed!", 1);
                } else if (key.ch == 'p') {
                    app.plat_filter = (app.plat_filter + 1) % NUM_PLAT_FILTERS;
                    app.selected = 0;
                    app_rebuild_filter(&app);
                } else if (key.ch == 'c') {
                    if (app.filter_count > 0 && !IS_HEADER(app.filtered[app.selected])) {
                        app.mode = MODE_CONTROLS;
                        app.panel_scroll = 0;
                    }
                } else if (key.ch == 'w') {
                    if (app.filter_count > 0 && !IS_HEADER(app.filtered[app.selected])) {
                        const Game *g = &GAMES[app.filtered[app.selected]];
                        const char *url = g->website ? g->website : g->repo;
                        if (url) {
                            plat_open_url(url);
                            char msg[128];
                            snprintf(msg, sizeof(msg), "Opening %s...", url);
                            app_set_message(&app, msg, 1);
                        }
                    }
                } else if (key.ch == 'L') {
                    if (app.last_log) {
                        app.mode = MODE_VIEWLOG;
                        app.log_scroll = 0;
                    }
                } else if (key.ch == '-') {
                    /* Collapse all categories */
                    for (int ci = 1; ci < NUM_CATEGORIES; ci++)
                        app.cat_collapsed[ci] = 1;
                    app_rebuild_filter(&app);
                } else if (key.ch == '=' || key.ch == '+') {
                    /* Expand all categories */
                    for (int ci = 1; ci < NUM_CATEGORIES; ci++)
                        app.cat_collapsed[ci] = 0;
                    app_rebuild_filter(&app);
                } else if (key.ch == 'i' || key.ch == 'd') {
                    handle_install_key(&app, scr, &w, &h, key.ch == 'i');
                }
                break;
            }
            default:
                break;
            }
            break;
        }
    }

    chain_clear(&app);
    app_cleanup(&app);
    screen_destroy(scr);
    term_cleanup();
    return 0;
}
