#include "../core/catalog.h"

static const char *keys[] = {
    "Up|Accelerate",
    "Down|Brake",
    "Left / Right|Steer",
    "Space|Gear up",
    "X|Gear down",
    "F2|Driver view",
    "Esc|Menu / pause",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "git submodule update --init --recursive\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "mkdir -p lib/games\n"
    "cp -a build/lib/games/speed-dreams-2 lib/games/\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./build/games/speed-dreams-2", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libopenscenegraph-dev",
    "libcurl4-openssl-dev", "libsdl2-dev", "libsdl2-mixer-dev", "libsdl2-ttf-dev",
    "libplib-dev", "libopenal-dev", "libenet-dev",
    "libminizip-dev", "librhash-dev", "libglm-dev",
    "libtinygltf-dev", "libcjson-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libopenscenegraph-dev libsdl2-dev libsdl2-mixer-dev libcurl4-openssl-dev cmake >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "cmake libopenscenegraph-dev libsdl2-dev libsdl2-mixer-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake + SDL2)",
    .platforms = PLAT_LINUX,
    .url = "https://forge.a-lec.org/speed-dreams/speed-dreams-code.git",
    .dir = "speed-dreams", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "speed-dreams-2",
}};

static const Game game_data = {
    .name = "Speed Dreams", .icon = "S",
    .desc = "Open-source motorsport simulator with high-quality 3D graphics and accurate physics. Fork of TORCS targeting maximum realism.",
    .keys = keys, .category = "Racing",
    .engine = "TORCS (OpenGL)", .website = "https://www.speed-dreams.net/",
    .repo = "https://forge.a-lec.org/speed-dreams/speed-dreams-code",
    .platforms = PLAT_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_speed_dreams(void) { return &game_data; }
