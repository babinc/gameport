#include "../catalog.h"

static const char *keys[] = {
    "Left / Right|Move",
    "Up / Down|Move vertically",
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
static const char *play[] = {"./build/space_invaders", NULL};

/* ── Windows build (PowerShell) ──────────────────────────────── */

static const char *win_build[] = {
    "powershell", "-NoProfile", "-Command",
    "Copy-Item classics\\src\\space_invaders.c space_invaders.c\n"
    "$cmake = @\"\n"
    "cmake_minimum_required(VERSION 3.14)\n"
    "project(space_invaders C)\n"
    "include(FetchContent)\n"
    "FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5 GIT_SHALLOW TRUE)\n"
    "FetchContent_MakeAvailable(raylib)\n"
    "add_executable(space_invaders space_invaders.c)\n"
    "target_link_libraries(space_invaders raylib)\n"
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
static const char *win_play[] = {"build\\Release\\space_invaders.exe", NULL};

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
        .clone_url = "https://github.com/raysan5/raylib-games.git",
        .clone_dir = "space_invaders", .shallow = 1,
        .build_cmd = build, .play_cmd = play,
        .bin = "space_invaders",
    },
    {
        .method = ACQUIRE_GIT, .label = "Build from source (cmake + raylib)",
        .platforms = win_platforms,
        .clone_url = "https://github.com/raysan5/raylib-games.git",
        .clone_dir = "space_invaders", .shallow = 1,
        .build_cmd = win_build, .play_cmd = win_play,
        .bin = "space_invaders.exe",
    },
};

static const Game game_data = {
    .name = "Space Invaders", .icon = "V",
    .desc = "Classic Space Invaders in a graphical window. Shoot descending waves of aliens before they reach the bottom. Built with raylib -- pure C, zero dependencies.",
    .keys = keys, .category = "Action",
    .engine = "raylib", .repo = "https://github.com/raysan5/raylib-games",
    .platforms = PLATFORMS_ALL, .platform_deps = deps, .num_deps = 3,
    .sources = sources, .num_sources = 2,
};

const Game *game_space_invaders(void) { return &game_data; }
