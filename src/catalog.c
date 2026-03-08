#include "catalog.h"
#include <string.h>

/* ── Platform detection ───────────────────────────────────────── */

const char *current_platform(void) {
#if defined(__linux__)
    return "linux";
#elif defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#else
    return "unknown";
#endif
}

int game_supports_platform(const Game *g) {
    if (!g->platforms) return 1;
    const char *plat = current_platform();
    for (int i = 0; g->platforms[i]; i++) {
        if (strcmp(g->platforms[i], plat) == 0) return 1;
    }
    return 0;
}

const PlatformDeps *platform_deps_for_current(const Game *g) {
    const char *os = current_platform();
    for (int i = 0; i < g->num_deps; i++) {
        if (strcmp(g->platform_deps[i].os, os) == 0)
            return &g->platform_deps[i];
    }
    return NULL;
}

const Source *default_source(const Game *g) {
    for (int i = 0; i < g->num_sources; i++) {
        if (strcmp(g->sources[i].method, "binary") != 0)
            return &g->sources[i];
    }
    if (g->num_sources > 0) return &g->sources[0];
    return NULL;
}

/* ── Static data ──────────────────────────────────────────────── */

/* Minesweeper */
static const char *ms_uninstall[] = {"cargo", "uninstall", "cmd-minesweeper", NULL};

/* Tetris */
static const char *tet_uninstall[] = {"cargo", "uninstall", "sxtetris", NULL};

/* Chess TUI */
static const char *chess_uninstall[] = {"cargo", "uninstall", "chess-tui", NULL};

/* 2048 */
static const char *t48_uninstall[] = {"cargo", "uninstall", "cli_2048", NULL};

/* RecWars */
static const char *rw_uninstall[] = {"cargo", "uninstall", "rec-wars", NULL};

/* Space Invaders — cmake */
static const char *si_platforms[] = {"linux", "windows", "macos", NULL};
static const char *si_build[] = {"cmake-game", "space_invaders", "classics/src/space_invaders.c", NULL};
static const char *si_uninstall[] = {"cmake-game-remove", "space_invaders", NULL};
static const char *si_linux_install[] = {"sudo", "apt", "install", "-y", "build-essential", "libgl1-mesa-dev", NULL};
static const char *si_linux_check[] = {"dpkg", "-s", "libgl1-mesa-dev", NULL};
static const char *si_mac_install[] = {"xcode-select", "--install", NULL};
static const char *si_mac_check[] = {"xcode-select", "-p", NULL};

static const PlatformDeps si_deps[] = {
    { "linux", "build-essential libgl1-mesa-dev", si_linux_install, si_linux_check, 1 },
    { "windows", "Visual Studio Build Tools", NULL, NULL, 0 },
    { "macos", "Xcode CLI tools", si_mac_install, si_mac_check, 0 },
};

/* Asteroids — cmake (shares deps with Space Invaders) */
static const char *ast_build[] = {"cmake-game", "asteroids", "classics/src/asteroids.c", NULL};
static const char *ast_uninstall[] = {"cmake-game-remove", "asteroids", NULL};

/* Anarch — git */
static const char *an_platforms[] = {"linux", "windows", "macos", NULL};
static const char *an_build[] = {"bash", "make.sh", "sdl", NULL};
static const char *an_play[] = {"./anarch", NULL};
static const char *an_uninstall[] = {"git-game-remove", "anarch", NULL};
static const char *an_linux_install[] = {"sudo", "apt", "install", "-y", "libsdl2-dev", "g++", NULL};
static const char *an_linux_check[] = {"dpkg", "-s", "libsdl2-dev", NULL};
static const char *an_mac_install[] = {"brew", "install", "sdl2", NULL};
static const char *an_mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps an_deps[] = {
    { "linux", "libsdl2-dev g++", an_linux_install, an_linux_check, 1 },
    { "windows", "SDL2, MinGW or MSVC", NULL, NULL, 0 },
    { "macos", "SDL2", an_mac_install, an_mac_check, 0 },
};

/* ── Sources ──────────────────────────────────────────────────── */

static const Source ms_sources[] = {{
    "cargo", "cargo install",
    "", "", 1, NULL, NULL, "cmd-minesweeper", ms_uninstall,
}};

static const Source tet_sources[] = {{
    "cargo", "cargo install",
    "", "", 1, NULL, NULL, "sxtetris", tet_uninstall,
}};

static const Source chess_sources[] = {{
    "cargo", "cargo install",
    "", "", 1, NULL, NULL, "chess-tui", chess_uninstall,
}};

static const Source t48_sources[] = {{
    "cargo", "cargo install",
    "", "", 1, NULL, NULL, "2048", t48_uninstall,
}};

static const Source rw_sources[] = {{
    "cargo", "cargo install",
    "", "", 1, NULL, NULL, "rec-wars", rw_uninstall,
}};

static const Source si_sources[] = {{
    "cmake", "Build from source (cmake)",
    "https://github.com/raysan5/raylib-games", "space_invaders", 1,
    si_build, NULL, "space_invaders", si_uninstall,
}};

static const Source ast_sources[] = {{
    "cmake", "Build from source (cmake)",
    "https://github.com/raysan5/raylib-games", "asteroids", 1,
    ast_build, NULL, "asteroids", ast_uninstall,
}};

static const Source an_sources[] = {{
    "git", "Build from source (git + gcc)",
    "https://gitlab.com/drummyfish/anarch.git", "anarch", 1,
    an_build, an_play, "anarch", an_uninstall,
}};

/* ── Game catalog ─────────────────────────────────────────────── */

const Game GAMES[] = {
    {
        "Minesweeper", "#",
        "Reveal tiles without hitting mines. Numbers show adjacent mine count. Flag tiles you think are mines. Clear all safe tiles to win.",
        "WASD move, Q uncover tile, E flag/unflag", "Puzzle",
        "crossterm", "https://crates.io/crates/cmd-minesweeper",
        NULL, NULL, 0, ms_sources, 1,
    },
    {
        "Tetris", "T",
        "Falling block puzzle. Rotate and position tetrominoes to fill complete rows. Completed rows disappear. Game ends when blocks stack to the top.",
        "Left/Right move, Up rotate, Down soft drop, Space hard drop, P pause", "Action",
        "ratatui", "https://crates.io/crates/sxtetris",
        NULL, NULL, 0, tet_sources, 1,
    },
    {
        "Chess TUI", "K",
        "Full chess in the terminal. Play vs a friend locally, against a UCI engine (Stockfish), or online via Lichess. Supports multiple board skins.",
        "Arrows/hjkl move cursor, Space select/place, ? help, s cycle skins, q quit", "Strategy",
        "ratatui", "https://crates.io/crates/chess-tui",
        NULL, NULL, 0, chess_sources, 1,
    },
    {
        "2048", "2",
        "Slide all tiles in one direction. Matching numbers merge and double. Keep merging to reach 2048. Board fills up = game over.",
        "Arrow keys / WASD to slide all tiles", "Puzzle",
        "crossterm", "https://crates.io/crates/cli_2048",
        NULL, NULL, 0, t48_sources, 1,
    },
    {
        "RecWars", "W",
        "Top-down tank deathmatch. Drive tanks, hovercraft, or hummers and blast opponents with 8 weapons. Free-for-all, team war, or capture the cow. Graphical window.",
        "Arrows drive, Ctrl fire, ; or ` console, Tab switch weapon", "Action",
        "macroquad", "https://crates.io/crates/rec-wars",
        NULL, NULL, 0, rw_sources, 1,
    },
    {
        "Space Invaders", "V",
        "Classic Space Invaders in a graphical window. Shoot descending waves of aliens before they reach the bottom. Built with raylib -- pure C, zero dependencies.",
        "Left/Right move, Space shoot", "Action",
        "raylib", "https://github.com/raysan5/raylib-games",
        si_platforms, si_deps, 3, si_sources, 1,
    },
    {
        "Asteroids", "A",
        "Classic Asteroids in a graphical window. Pilot a ship, rotate and thrust to dodge, shoot to break asteroids into smaller pieces. Built with raylib -- pure C.",
        "Left/Right rotate, Up thrust, Space shoot", "Action",
        "raylib", "https://github.com/raysan5/raylib-games",
        si_platforms, si_deps, 3, ast_sources, 1,
    },
    {
        "Anarch", "F",
        "Retro first-person shooter inspired by Doom. 10 levels, multiple weapons, all assets embedded in source code. Tiny, fast, zero external files needed.",
        "WASD move, Mouse aim, LMB shoot, RMB next weapon, Space jump", "Action",
        "SDL2", "https://gitlab.com/drummyfish/anarch",
        an_platforms, an_deps, 3, an_sources, 1,
    },
};

const int NUM_GAMES = sizeof(GAMES) / sizeof(GAMES[0]);
