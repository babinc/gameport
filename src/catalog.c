#include "catalog.h"
#include <string.h>

const char *method_str(MethodType m) {
    switch (m) {
    case METHOD_CARGO: return "cargo";
    case METHOD_GIT:   return "git";
    }
    return "unknown";
}

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

/* Space Invaders — raylib (single-file game from raylib-games repo) */
static const char *si_platforms[] = {"linux", "windows", "macos", NULL};
static const char *si_build[] = {
    "bash", "-c",
    "set -e\n"
    "cp classics/src/space_invaders.c space_invaders.c\n"
    "cat > CMakeLists.txt << 'CMEOF'\n"
    "cmake_minimum_required(VERSION 3.14)\n"
    "project(space_invaders C)\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)\n"
    "FetchContent_MakeAvailable(raylib)\n"
    "add_executable(space_invaders space_invaders.c)\n"
    "target_link_libraries(space_invaders raylib)\n"
    "CMEOF\n"
    "echo 'Configuring cmake (fetching raylib)...'\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "echo 'Building...'\n"
    "cmake --build build --config Release",
    NULL
};
static const char *si_play[] = {"./build/space_invaders", NULL};
static const char *si_uninstall[] = {"git-game-remove", "space_invaders", NULL};
static const char *si_linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libgl1-mesa-dev", NULL};
static const char *si_linux_check[] = {"dpkg", "-s", "cmake", NULL};
static const char *si_mac_install[] = {"xcode-select", "--install", NULL};
static const char *si_mac_check[] = {"xcode-select", "-p", NULL};

static const PlatformDeps si_deps[] = {
    { "linux", "build-essential cmake libgl1-mesa-dev", si_linux_install, si_linux_check, 1 },
    { "windows", "Visual Studio Build Tools", NULL, NULL, 0 },
    { "macos", "Xcode CLI tools", si_mac_install, si_mac_check, 0 },
};

/* Asteroids — raylib (single-file game from raylib-games repo, shares deps) */
static const char *ast_build[] = {
    "bash", "-c",
    "set -e\n"
    "cp classics/src/asteroids.c asteroids.c\n"
    "cat > CMakeLists.txt << 'CMEOF'\n"
    "cmake_minimum_required(VERSION 3.14)\n"
    "project(asteroids C)\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)\n"
    "FetchContent_MakeAvailable(raylib)\n"
    "add_executable(asteroids asteroids.c)\n"
    "target_link_libraries(asteroids raylib)\n"
    "CMEOF\n"
    "echo 'Configuring cmake (fetching raylib)...'\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "echo 'Building...'\n"
    "cmake --build build --config Release",
    NULL
};
static const char *ast_play[] = {"./build/asteroids", NULL};
static const char *ast_uninstall[] = {"git-game-remove", "asteroids", NULL};

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

/* Chocolate Doom — git + cmake */
static const char *cd_platforms[] = {"linux", "macos", NULL};
static const char *cd_build[] = {
    "bash", "-c",
    "set -e && "
    "mkdir -p build && cd build && "
    "cmake .. -DCMAKE_BUILD_TYPE=Release && "
    "make -j$(nproc) && "
    "cd .. && "
    "echo 'Downloading Freedoom WADs...' && "
    "mkdir -p wads && "
    "curl -L -o /tmp/freedoom.zip https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip && "
    "unzip -o /tmp/freedoom.zip -d /tmp/freedoom && "
    "cp /tmp/freedoom/freedoom-0.13.0/*.wad wads/ && "
    "rm -rf /tmp/freedoom /tmp/freedoom.zip && "
    "echo 'Freedoom WADs installed!'",
    NULL
};
static const char *cd_play[] = {"./build/src/chocolate-doom", "-iwad", "./wads/freedoom2.wad", NULL};
static const char *cd_uninstall[] = {"git-game-remove", "chocolate-doom", NULL};
static const char *cd_linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsdl2-mixer-dev",
    "libsdl2-net-dev", "libpng-dev", "unzip", "curl", NULL};
static const char *cd_linux_check[] = {"dpkg", "-s", "libsdl2-net-dev", NULL};
static const char *cd_mac_install[] = {"brew", "install", "sdl2", "sdl2_mixer", "sdl2_net", "libpng", NULL};
static const char *cd_mac_check[] = {"brew", "list", "sdl2_net", NULL};

static const PlatformDeps cd_deps[] = {
    { "linux", "libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libpng-dev", cd_linux_install, cd_linux_check, 1 },
    { "macos", "SDL2 SDL2_mixer SDL2_net libpng", cd_mac_install, cd_mac_check, 0 },
};

/* ── Sources ──────────────────────────────────────────────────── */

static const Source ms_sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "cmd-minesweeper", ms_uninstall,
}};

static const Source tet_sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "sxtetris", tet_uninstall,
}};

static const Source chess_sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "chess-tui", chess_uninstall,
}};

static const Source t48_sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "2048", t48_uninstall,
}};

static const Source rw_sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "rec-wars", rw_uninstall,
}};

static const Source si_sources[] = {{
    METHOD_GIT, "Build from source (cmake + raylib)",
    "https://github.com/raysan5/raylib-games.git", "space_invaders", 1,
    si_build, si_play, "space_invaders", si_uninstall,
}};

static const Source ast_sources[] = {{
    METHOD_GIT, "Build from source (cmake + raylib)",
    "https://github.com/raysan5/raylib-games.git", "asteroids", 1,
    ast_build, ast_play, "asteroids", ast_uninstall,
}};

static const Source an_sources[] = {{
    METHOD_GIT, "Build from source (git + gcc)",
    "https://gitlab.com/drummyfish/anarch.git", "anarch", 1,
    an_build, an_play, "anarch", an_uninstall,
}};

static const Source cd_sources[] = {{
    METHOD_GIT, "Build from source (git + cmake)",
    "https://github.com/chocolate-doom/chocolate-doom.git", "chocolate-doom", 1,
    cd_build, cd_play, "chocolate-doom", cd_uninstall,
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
    {
        "Chocolate Doom", "D",
        "Faithful recreation of the original Doom engine. Plays just like the 1993 classic. Bundled with Freedoom -- free community-made levels and assets.",
        "WASD move, Mouse aim, Ctrl fire, Space use/open, Shift run", "Shooter",
        "SDL2", "https://github.com/chocolate-doom/chocolate-doom",
        cd_platforms, cd_deps, 2, cd_sources, 1,
    },
};

const int NUM_GAMES = sizeof(GAMES) / sizeof(GAMES[0]);
