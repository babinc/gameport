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

static void start_cmake_install(App *app, const Source *src) {
    /* For cmake games, we need to orchestrate: clone repo, copy source, cmake, build.
       We use a helper shell script approach: run our internal cmake-game command.
       Since we don't have the internal commands as binaries, we'll do it inline
       by spawning a shell with the full build script. */
    char *gdir = games_dir();

    /* Build a shell script that does the cmake dance */
    char script[4096];
    snprintf(script, sizeof(script),
        "set -e\n"
        "GAMES_DIR='%s'\n"
        "NAME='%s'\n"
        "SRC_REL='%s'\n"
        "REPO_DIR=\"$GAMES_DIR/raylib-games\"\n"
        "GAME_DIR=\"$GAMES_DIR/$NAME\"\n"
        "mkdir -p \"$GAME_DIR\"\n"
        "\n"
        "# Clone raylib-games repo if needed\n"
        "if [ ! -d \"$REPO_DIR/.git\" ]; then\n"
        "  echo 'Cloning raylib-games repo (shallow)...'\n"
        "  git clone --depth 1 https://github.com/raysan5/raylib-games.git \"$REPO_DIR\"\n"
        "else\n"
        "  echo 'Source repo already cloned.'\n"
        "fi\n"
        "\n"
        "# Copy source file\n"
        "echo \"Copying $SRC_REL...\"\n"
        "cp \"$REPO_DIR/$SRC_REL\" \"$GAME_DIR/$NAME.c\"\n"
        "\n"
        "# Write CMakeLists.txt\n"
        "cat > \"$GAME_DIR/CMakeLists.txt\" << 'CMAKEOF'\n"
        "cmake_minimum_required(VERSION 3.14)\n"
        "project(%s C)\n"
        "include(FetchContent)\n"
        "FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)\n"
        "FetchContent_MakeAvailable(raylib)\n"
        "add_executable(%s %s.c)\n"
        "target_link_libraries(%s raylib)\n"
        "if(WIN32)\n"
        "  target_link_libraries(%s winmm)\n"
        "endif()\n"
        "CMAKEOF\n"
        "echo 'Wrote CMakeLists.txt'\n"
        "\n"
        "echo ''\n"
        "echo 'Configuring cmake (fetching raylib)...'\n"
        "cd \"$GAME_DIR\"\n"
        "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
        "\n"
        "echo ''\n"
        "echo 'Building...'\n"
        "cmake --build build --config Release\n"
        "echo 'Build complete!'\n",
        gdir, src->clone_dir, src->build_cmd[2], /* SRC_REL is 3rd element */
        src->clone_dir, src->clone_dir, src->clone_dir,
        src->clone_dir, src->clone_dir);
    free(gdir);

    const char *cmd[] = {"bash", "-c", script, NULL};
    child_start(&app->child, cmd, NULL);
}

static void start_cmake_remove(App *app, const Source *src) {
    char *gdir = games_dir();
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", gdir, src->clone_dir);
    free(gdir);

    char script[1024];
    snprintf(script, sizeof(script), "echo 'Removing %s...' && rm -rf '%s' && echo 'Removed!'",
             src->clone_dir, path);
    const char *cmd[] = {"bash", "-c", script, NULL};
    child_start(&app->child, cmd, NULL);
}

static void start_git_install(App *app, const Source *src) {
    char *gdir = games_dir();

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
        "GAMES_DIR='%s'\n"
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
        "echo 'installed' > \"$GAME_DIR/.arcade-ready\"\n"
        "echo 'Game ready!'\n",
        gdir, src->clone_dir, src->clone_url,
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

    char script[1024];
    snprintf(script, sizeof(script), "echo 'Removing %s...' && rm -rf '%s' && echo 'Removed!'",
             src->clone_dir, path);
    const char *cmd[] = {"bash", "-c", script, NULL};
    child_start(&app->child, cmd, NULL);
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
            case KEY_CHAR:
                if (key.ch == 'j') app.panel_scroll++;
                else if (key.ch == 'k' && app.panel_scroll > 0) app.panel_scroll--;
                break;
            case KEY_UP:
                if (app.panel_scroll > 0) app.panel_scroll--;
                break;
            case KEY_ESC:
                if (app.child.done) {
                    /* Save log */
                    free(app.last_log);
                    /* Build log string from output lines */
                    size_t total = 0;
                    for (int i = 0; i < app.child.output.count; i++)
                        total += strlen(app.child.output.lines[i]) + 1;
                    app.last_log = malloc(total + 1);
                    app.last_log[0] = '\0';
                    for (int i = 0; i < app.child.output.count; i++) {
                        strcat(app.last_log, app.child.output.lines[i]);
                        strcat(app.last_log, "\n");
                    }

                    int ok = app.child.ok;
                    child_cleanup(&app.child);
                    app.child.pipe_fd = -1;
                    app.child.pid = 0;
                    app_refresh(&app);
                    app_rebuild_filter(&app);
                    if (ok) {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s done!", GAMES[app.active_game].name);
                        app_set_message(&app, msg, 1);
                    } else {
                        char msg[128];
                        snprintf(msg, sizeof(msg), "%s failed. Press [L] for log.", GAMES[app.active_game].name);
                        app_set_message(&app, msg, 0);
                    }
                    app.mode = MODE_NORMAL;
                } else if (app.mode == MODE_RUNNING) {
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
                if (key.type == KEY_CHAR && key.ch == 'q') {
                    if (app.child.done) {
                        /* Same as Esc when done */
                        free(app.last_log);
                        size_t total = 0;
                        for (int i = 0; i < app.child.output.count; i++)
                            total += strlen(app.child.output.lines[i]) + 1;
                        app.last_log = malloc(total + 1);
                        app.last_log[0] = '\0';
                        for (int i = 0; i < app.child.output.count; i++) {
                            strcat(app.last_log, app.child.output.lines[i]);
                            strcat(app.last_log, "\n");
                        }
                        child_cleanup(&app.child);
                        app.child.pipe_fd = -1;
                        app.child.pid = 0;
                        app_refresh(&app);
                        app_rebuild_filter(&app);
                        app.mode = MODE_NORMAL;
                    }
                }
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
                if (is_git_cloned_not_ready(g)) {
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

                        if (strcmp(src->method, "cmake") == 0) {
                            char *exe = cmake_game_exe(src);
                            /* We need exe to persist — store in static-ish buffer */
                            static char exe_buf[1024];
                            snprintf(exe_buf, sizeof(exe_buf), "%s", exe);
                            free(exe);
                            cmd[ci++] = exe_buf;
                        } else if (strcmp(src->method, "git") == 0) {
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
                                 src->method, runtime_install_hint(src->method));
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
                            if (strcmp(src->method, "cargo") == 0) {
                                start_cargo_install(&app, src);
                            } else if (strcmp(src->method, "cmake") == 0) {
                                start_cmake_install(&app, src);
                            } else if (strcmp(src->method, "git") == 0) {
                                start_git_install(&app, src);
                            }
                            app.panel_label = "INSTALLING";
                        } else {
                            if (strcmp(src->method, "cargo") == 0) {
                                start_cargo_uninstall(&app, src);
                            } else if (strcmp(src->method, "cmake") == 0 ||
                                       strcmp(src->method, "git") == 0) {
                                if (strcmp(src->method, "cmake") == 0)
                                    start_cmake_remove(&app, src);
                                else
                                    start_git_remove(&app, src);
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
