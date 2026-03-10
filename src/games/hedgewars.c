#include "../catalog.h"

static const char *keys[] = {
    "Left / Right|Walk",
    "Up / Down|Aim",
    "Space|Fire (hold for power)",
    "Enter|Jump",
    "Backspace|Jump back",
    "Tab|Switch hedgehog",
    "F1-F9|Select weapon",
    "T|Chat",
    "Esc|Menu",
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
static const char *play[] = {"./build/bin/hedgewars", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsdl2-mixer-dev",
    "libsdl2-image-dev", "libsdl2-net-dev", "libsdl2-ttf-dev",
    "qtbase5-dev", "fpc", "libphysfs-dev", "libpng-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s cmake libsdl2-dev qtbase5-dev fpc >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "cmake libsdl2-dev qtbase5-dev fpc ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/hedgewars/hw.git",
    .dir = "hedgewars", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "hedgewars",
}};

static const Game game_data = {
    .name = "Hedgewars", .icon = "H",
    .desc = "Turn-based strategy with hedgehogs, inspired by Worms. Destructible terrain, varied weapons, and multiplayer mayhem.",
    .keys = keys, .category = "Strategy",
    .engine = "SDL2 (Free Pascal)", .website = "https://www.hedgewars.org/",
    .repo = "https://github.com/hedgewars/hw",
    .platforms = PLATFORMS_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_hedgewars(void) { return &game_data; }
