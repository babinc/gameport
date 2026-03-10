#include "../catalog.h"

static const char *keys[] = {
    "WASD / Arrows|Move robot / camera",
    "Space|Jump",
    "Enter|Execute program",
    "F1|Help",
    "LMB|Select / interact",
    "RMB|Context action",
    "Esc|Menu",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "git submodule update --init --recursive\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./build/colobot", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsdl2-image-dev",
    "libsdl2-ttf-dev", "libsndfile1-dev", "libvorbis-dev", "libogg-dev",
    "libpng-dev", "libglew-dev", "libopenal-dev", "libphysfs-dev",
    "gettext", "libboost-dev", "libglm-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s cmake libsdl2-dev libglew-dev libphysfs-dev libboost-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "cmake libsdl2-dev libglew-dev libphysfs-dev libboost-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/colobot/colobot.git",
    .dir = "colobot",
    .build_cmd = build, .play_cmd = play,
    .bin = "colobot",
}};

static const Game game_data = {
    .name = "Colobot", .icon = "C",
    .desc = "Educational game teaching programming through entertainment. Program robots with a C++/Java-like language to colonize a new planet.",
    .keys = keys, .category = "Puzzle",
    .engine = "SDL2 (OpenGL)", .website = "https://colobot.info/",
    .repo = "https://github.com/colobot/colobot",
    .platforms = PLATFORMS_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_colobot(void) { return &game_data; }
