#include "../core/catalog.h"

static const char *keys[] = {
    "WASD|Move",
    "Space|Jump",
    "LShift|Sneak",
    "LMB|Dig / attack",
    "RMB|Place / use",
    "I|Inventory",
    "E|Aux1 (sprint / fly fast)",
    "T|Chat",
    "Esc|Menu",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "git submodule update --init --recursive\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release -DRUN_IN_PLACE=TRUE\n"
    "cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./build/bin/luanti", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsqlite3-dev",
    "libcurl4-openssl-dev", "libluajit-5.1-dev", "libfreetype6-dev",
    "libpng-dev", "libjpeg-dev", "zlib1g-dev", "libgl1-mesa-dev",
    "gettext", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev libsqlite3-dev libluajit-5.1-dev cmake >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libsdl2-dev libsqlite3-dev libluajit-5.1-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/luanti-org/luanti.git",
    .dir = "luanti", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "luanti",
}};

static const Game game_data = {
    .name = "Luanti", .icon = "L",
    .desc = "Open-source voxel game engine with easy modding and game creation. Supports custom games and mods via a Lua API. Formerly known as Minetest.",
    .keys = keys, .category = "Action",
    .engine = "Irrlicht", .website = "https://www.luanti.org/",
    .repo = "https://github.com/luanti-org/luanti",
    .platforms = PLAT_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_luanti(void) { return &game_data; }
