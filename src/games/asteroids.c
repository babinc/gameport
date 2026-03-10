#include "../catalog.h"

static const char *keys[] = {
    "Left / Right|Rotate ship",
    "Up|Thrust forward",
    "Down|Brake / reverse",
    "Space|Shoot",
    "P|Pause",
    "Enter|Restart (game over)",
    NULL
};

static const char *win_platforms[] = {"windows", NULL};

/* ── POSIX build (bash) ──────────────────────────────────────── */

static const char *build[] = {
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
static const char *play[] = {"./build/asteroids", NULL};

/* ── Windows build (PowerShell) ──────────────────────────────── */

static const char *win_build[] = {
    "powershell", "-NoProfile", "-Command",
    "Copy-Item classics\\src\\asteroids.c asteroids.c\n"
    "$cmake = @\"\n"
    "cmake_minimum_required(VERSION 3.14)\n"
    "project(asteroids C)\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)\n"
    "FetchContent_MakeAvailable(raylib)\n"
    "add_executable(asteroids asteroids.c)\n"
    "target_link_libraries(asteroids raylib)\n"
    "\"@\n"
    "$cmake | Set-Content CMakeLists.txt\n"
    "Write-Host 'Configuring cmake (fetching raylib)...'\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "if ($LASTEXITCODE -ne 0) { exit 1 }\n"
    "Write-Host 'Building...'\n"
    "cmake --build build --config Release\n"
    "if ($LASTEXITCODE -ne 0) { exit 1 }",
    NULL
};
static const char *win_play[] = {"build\\Release\\asteroids.exe", NULL};

/* ── Platform deps ───────────────────────────────────────────── */

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libgl1-mesa-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s cmake libgl1-mesa-dev >/dev/null 2>&1", NULL};
static const char *win_check[] = {"cmake", "--version", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libgl1-mesa-dev", linux_install, linux_check, 1 },
    { "macos", "Xcode CLI tools", MAC_XCODE_INSTALL, MAC_XCODE_CHECK, 0 },
    { "windows", "cmake + Visual Studio Build Tools", NULL, win_check, 0 },
};

/* ── Sources ─────────────────────────────────────────────────── */

static const Source sources[] = {
    {
        .method = ACQUIRE_GIT, .label = "Build from source (cmake + raylib)",
        .platforms = PLATFORMS_POSIX,
        .url = "https://github.com/raysan5/raylib-games.git",
        .dir = "asteroids", .shallow = 1,
        .build_cmd = build, .play_cmd = play,
        .bin = "asteroids",
    },
    {
        .method = ACQUIRE_GIT, .label = "Build from source (cmake + raylib)",
        .platforms = win_platforms,
        .url = "https://github.com/raysan5/raylib-games.git",
        .dir = "asteroids", .shallow = 1,
        .build_cmd = win_build, .play_cmd = win_play,
        .bin = "asteroids.exe",
    },
};

static const Game game_data = {
    .name = "Asteroids", .icon = "A",
    .desc = "Classic Asteroids in a graphical window. Pilot a ship, rotate and thrust to dodge, shoot to break asteroids into smaller pieces. Built with raylib -- pure C.",
    .keys = keys, .category = "Action",
    .engine = "raylib", .repo = "https://github.com/raysan5/raylib-games",
    .platforms = PLATFORMS_ALL, .platform_deps = deps, .num_platform_deps = 3,
    .sources = sources, .num_sources = 2,
};

const Game *game_asteroids(void) { return &game_data; }
