#include "../catalog.h"

static const char *keys[] = {
    "Left / Right|Move",
    "Up / Down|Move vertically",
    "Space|Shoot",
    "P|Pause",
    "Enter|Restart (game over)",
    NULL
};

static const char *platforms[] = {"linux", "windows", "macos", NULL};

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
static const char *uninstall[] = {"git-game-remove", "space_invaders", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libgl1-mesa-dev", NULL};
static const char *linux_check[] = {"dpkg", "-s", "cmake", NULL};
static const char *mac_install[] = {"xcode-select", "--install", NULL};
static const char *mac_check[] = {"xcode-select", "-p", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libgl1-mesa-dev", linux_install, linux_check, 1 },
    { "windows", "Visual Studio Build Tools", NULL, NULL, 0 },
    { "macos", "Xcode CLI tools", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = METHOD_GIT, .label = "Build from source (cmake + raylib)",
    .clone_url = "https://github.com/raysan5/raylib-games.git",
    .clone_dir = "space_invaders", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "space_invaders", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Space Invaders", .icon = "V",
    .desc = "Classic Space Invaders in a graphical window. Shoot descending waves of aliens before they reach the bottom. Built with raylib -- pure C, zero dependencies.",
    .keys = keys, .category = "Action",
    .engine = "raylib", .repo = "https://github.com/raysan5/raylib-games",
    .platforms = platforms, .platform_deps = deps, .num_deps = 3,
    .sources = sources, .num_sources = 1,
};

const Game *game_space_invaders(void) { return &game_data; }
