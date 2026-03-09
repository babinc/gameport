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

static const char *platforms[] = {"linux", "windows", "macos", NULL};

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
static const char *uninstall[] = {"git-game-remove", "asteroids", NULL};
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
    METHOD_GIT, "Build from source (cmake + raylib)",
    "https://github.com/raysan5/raylib-games.git", "asteroids", 1,
    build, play, "asteroids", uninstall,
}};

static const Game game_data = {
    "Asteroids", "A",
    "Classic Asteroids in a graphical window. Pilot a ship, rotate and thrust to dodge, shoot to break asteroids into smaller pieces. Built with raylib -- pure C.",
    keys, "Action",
    "raylib", "https://github.com/raysan5/raylib-games",
    platforms, deps, 3, sources, 1,
};

const Game *game_asteroids(void) { return &game_data; }
