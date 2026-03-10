#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select / interact",
    "RMB|Context menu",
    "Mousewheel|Zoom",
    "Arrow keys|Scroll map",
    "F2|Research",
    "F3|Production",
    "F10|Menu",
    "Enter|End turn",
    "Esc|Deselect / close",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./build/freeorion", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libboost-all-dev", "libsdl2-dev",
    "libfreetype6-dev", "libgl1-mesa-dev", "libopenal-dev",
    "libvorbis-dev", "libogg-dev", "python3-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s cmake libboost-all-dev libsdl2-dev python3-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "cmake libboost-all-dev libsdl2-dev python3-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/freeorion/freeorion.git",
    .dir = "freeorion", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "freeorion",
}};

static const Game game_data = {
    .name = "FreeOrion", .icon = "O",
    .desc = "Turn-based space empire and galactic conquest 4X game. Explore, expand, exploit, and exterminate across the galaxy.",
    .keys = keys, .category = "Strategy",
    .engine = "GiGi (OpenGL)", .website = "https://www.freeorion.org/",
    .repo = "https://github.com/freeorion/freeorion",
    .platforms = PLATFORMS_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_freeorion(void) { return &game_data; }
