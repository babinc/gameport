#define _POSIX_C_SOURCE 200809L
#include "term.h"
#include "ui.h"
#include "catalog.h"
#include "install.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

static volatile int g_resize = 0;

static void sigwinch_handler(int sig) {
    (void)sig;
    g_resize = 1;
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

static void start_git_install(App *app, const Source *src) {
    char *gdir = games_dir();
    char q_gdir[2048];
    shell_quote(q_gdir, sizeof(q_gdir), gdir);

    /* Build a shell script for git clone + build */
    char script[4096];
    /* Build the build command string */
    char build_str[1024] = "";
    if (src->build_cmd) {
        for (int i = 0; src->build_cmd[i]; i++) {
            if (i > 0) strncat(build_str, " ", sizeof(build_str) - strlen(build_str) - 1);
            strncat(build_str, src->build_cmd[i], sizeof(build_str) - strlen(build_str) - 1);
        }
    }

    snprintf(script, sizeof(script),
        "set -e\n"
        "GAMES_DIR=%s\n"
        "NAME='%s'\n"
        "REPO='%s'\n"
        "GAME_DIR=\"$GAMES_DIR/$NAME\"\n"
        "\n"
        "if [ -d \"$GAME_DIR/.git\" ]; then\n"
        "  echo \"$NAME already cloned, pulling latest...\"\n"
        "  cd \"$GAME_DIR\"\n"
        "  git pull || echo 'git pull failed, continuing...'\n"
        "else\n"
        "  echo \"Cloning $REPO...\"\n"
        "  git clone %s \"$REPO\" \"$GAME_DIR\"\n"
        "fi\n"
        "\n"
        "cd \"$GAME_DIR\"\n"
        "%s%s%s\n"
        "\n"
        "echo 'Game ready!'\n",
        q_gdir, src->clone_dir, src->clone_url,
        src->shallow ? "--depth 1" : "",
        build_str[0] ? "echo 'Building...'\n" : "",
        build_str,
        build_str[0] ? "\n" : "");
    free(gdir);

    const char *cmd[] = {"bash", "-c", script, NULL};
    child_start(&app->child, cmd, NULL);
}

static void start_git_remove(App *app, const Source *src) {
    char *gdir = games_dir();
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", gdir, src->clone_dir);
    free(gdir);

    char q_path[2048];
    shell_quote(q_path, sizeof(q_path), path);
    char script[4096];
    snprintf(script, sizeof(script), "echo 'Removing %s...' && rm -rf %s && echo 'Removed!'",
             src->clone_dir, q_path);
    const char *cmd[] = {"bash", "-c", script, NULL};
    child_start(&app->child, cmd, NULL);
}

/* ── Close install/run panel ───────────────────────────────────── */

/* Returns a static buffer with the log file path (valid until next call) */
static const char *save_log_to_file(App *app) {
    static char path[1024];
    char *ldir = logs_dir();
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    /* Build sanitized game name (spaces -> underscores) */
    char safe_name[128];
    snprintf(safe_name, sizeof(safe_name), "%s", GAMES[app->active_game].name);
    for (char *p = safe_name; *p; p++) {
        if (*p == ' ') *p = '_';
    }

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

static void close_panel(App *app) {
    int ok = app->child.ok;

    /* Save log to file (before building in-memory log so path is excluded from file) */
    const char *log_path = save_log_to_file(app);

    /* Append log path to output so it shows in [L] view */
    if (log_path) {
        char note[1024 + 16]; /* path (max 1024) + "Log saved: " prefix */
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
    app->child.pipe_fd = -1;
    app->child.pid = 0;
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
    /* Handle SIGWINCH for terminal resize */
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);

    term_init();

    int w, h;
    term_get_size(&w, &h);
    Screen *scr = screen_create(w, h);

    App app;
    app_init(&app);

    while (!app.should_quit) {
        /* Handle resize */
        if (g_resize) {
            g_resize = 0;
            term_get_size(&w, &h);
            screen_resize(scr, w, h);
        }

        /* Poll child process for output */
        if ((app.mode == MODE_INSTALLING || app.mode == MODE_RUNNING) && !app.child.done) {
            child_poll(&app.child);
        }

        /* Draw */
        ui_draw(scr, &app);
        screen_flush(scr);
        app.tick++;

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
                } else if (key.type == KEY_ESC && app.mode == MODE_RUNNING) {
                    child_kill(&app.child);
                    child_cleanup(&app.child);
                    app.child.pipe_fd = -1;
                    app.child.pid = 0;
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
                app.active_game = gi;
                const Game *g = &GAMES[gi];
                if (app.cloned[gi]) {
                    app_set_message(&app, "Cloned but not built. Press [i] to build.", 0);
                } else if (!app.installed[gi]) {
                    app_set_message(&app, "Not installed. Press [i] to install.", 0);
                } else {
                    const Source *src = default_source(g);
                    if (!src) break;

                    /* Terminal games need the full terminal */
                    int is_terminal = (strcmp(g->engine, "crossterm") == 0 ||
                                       strcmp(g->engine, "ratatui") == 0);
                    if (is_terminal) {
                        const char *cmd[8];
                        int ci = 0;
                        if (src->play_cmd && src->play_cmd[0]) {
                            for (int i = 0; src->play_cmd[i] && ci < 7; i++)
                                cmd[ci++] = src->play_cmd[i];
                        } else {
                            cmd[ci++] = src->bin;
                        }
                        cmd[ci] = NULL;

                        /* Need full redraw after */
                        screen_resize(scr, w, h);
                        int ok = run_visible(cmd, NULL);
                        term_get_size(&w, &h);
                        screen_resize(scr, w, h);
                        if (!ok) {
                            char msg[128];
                            snprintf(msg, sizeof(msg), "%s exited with error", g->name);
                            app_set_message(&app, msg, 0);
                        } else {
                            app_clear_message(&app);
                        }
                    } else {
                        /* Graphical / captured games */
                        const char *cmd[16];
                        const char *cwd = NULL;
                        char cwd_buf[1024];
                        int ci = 0;

                        if (src->method == METHOD_GIT) {
                            char *gdir = games_dir();
                            snprintf(cwd_buf, sizeof(cwd_buf), "%s/%s", gdir, src->clone_dir);
                            free(gdir);
                            cwd = cwd_buf;
                            if (src->play_cmd) {
                                for (int i = 0; src->play_cmd[i] && ci < 15; i++)
                                    cmd[ci++] = src->play_cmd[i];
                            }
                        } else if (src->play_cmd && src->play_cmd[0]) {
                            for (int i = 0; src->play_cmd[i] && ci < 15; i++)
                                cmd[ci++] = src->play_cmd[i];
                        } else {
                            cmd[ci++] = src->bin;
                        }
                        cmd[ci] = NULL;

                        child_start(&app.child, cmd, cwd);
                        app.mode = MODE_RUNNING;
                        app.panel_label = "RUNNING";
                        app.panel_scroll = 0;
                        app_clear_message(&app);
                    }
                }
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
                } else if (key.ch == 'L') {
                    if (app.last_log) {
                        app.mode = MODE_VIEWLOG;
                        app.log_scroll = 0;
                    }
                } else if (key.ch == 'i' || key.ch == 'd') {
                    if (app.filter_count == 0) break;
                    int installing = (key.ch == 'i');
                    int idx = app.filtered[app.selected];
                    app.active_game = idx;
                    const Game *g = &GAMES[idx];
                    const Source *src = default_source(g);

                    if (installing && !game_supports_platform(g)) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s not supported on %s",
                                 g->name, current_platform());
                        app_set_message(&app, msg, 0);
                    } else if (!src) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s has no install source", g->name);
                        app_set_message(&app, msg, 0);
                    } else if (installing && app.installed[idx]) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s already installed!", g->name);
                        app_set_message(&app, msg, 1);
                    } else if (!installing && !app.installed[idx]) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s not installed", g->name);
                        app_set_message(&app, msg, 0);
                    } else if (installing && !has_runtime(&app.toolchains, src->method)) {
                        char msg[256];
                        snprintf(msg, sizeof(msg), "Missing %s! %s",
                                 method_str(src->method), runtime_install_hint(src->method));
                        app_set_message(&app, msg, 0);
                    } else {
                        /* Install platform deps visibly if needed */
                        if (installing) {
                            const PlatformDeps *deps = platform_deps_for_current(g);
                            if (deps && deps->install_cmd && deps->install_cmd[0]
                                && !deps_check_satisfied(deps)) {
                                screen_resize(scr, w, h);
                                int ok = run_visible(deps->install_cmd, NULL);
                                term_get_size(&w, &h);
                                screen_resize(scr, w, h);
                                if (!ok) {
                                    app_set_message(&app, "Dep install failed. Press [i] to retry.", 0);
                                    break;
                                }
                            }
                        }

                        /* Kill game process before uninstall */
                        if (!installing) {
                            kill_game_process(src->bin);
                        }

                        /* Start install/uninstall */
                        if (installing) {
                            switch (src->method) {
                            case METHOD_CARGO: start_cargo_install(&app, src); break;
                            case METHOD_GIT:   start_git_install(&app, src); break;
                            }
                            app.panel_label = "INSTALLING";
                        } else {
                            switch (src->method) {
                            case METHOD_CARGO: start_cargo_uninstall(&app, src); break;
                            case METHOD_GIT:   start_git_remove(&app, src); break;
                            }
                            app.panel_label = "REMOVING";
                        }
                        app.mode = MODE_INSTALLING;
                        app.panel_scroll = 0;
                        app_clear_message(&app);
                    }
                }
                break;
            }
            default:
                break;
            }
            break;
        }
    }

    app_cleanup(&app);
    screen_destroy(scr);
    term_cleanup();
    return 0;
}
