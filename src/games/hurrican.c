#include "../core/catalog.h"

static const char *keys[] = {
    "Arrows|Move / aim",
    "Enter|Jump",
    "Ctrl|Shoot",
    "Shift|Cycle weapon",
    "Alt|Lightning / special",
    "Esc|Pause / menu",
    "Tab|Console",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e && "
    "git submodule update --init --recursive && "
    "cd Hurrican && "
    "mkdir -p build && cd build && "
    "cmake -DCMAKE_BUILD_TYPE=Release .. && "
    "cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};

static const char *play[] = {"bash", "-c", "cd Hurrican && ./build/hurrican", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsdl2-image-dev",
    "libsdl2-mixer-dev", "libepoxy-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libepoxy-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", "sdl2_image", "sdl2_mixer", "libepoxy", NULL};
static const char *mac_check[] = {"brew", "list", "libepoxy", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libepoxy-dev cmake", linux_install, linux_check, 1 },
    { "macos", "SDL2 SDL2_image SDL2_mixer libepoxy", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .url = "https://github.com/HurricanGame/Hurrican.git",
    .dir = "hurrican",
    .build_cmd = build, .play_cmd = play,
    .bin = "hurrican",
}};

static const Game game_data = {
    .name = "Hurrican", .icon = "H",
    .desc = "Open-source Turrican clone with fast-paced run-and-gun action. "
            "Battle through side-scrolling levels with multiple weapons, "
            "power-ups, and boss fights. Made by die-hard Turrican fans.",
    .keys = keys, .category = "Platformer",
    .engine = "SDL2", .website = "https://www.winterworks.de/project/hurrican/",
    .repo = "https://github.com/HurricanGame/Hurrican",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_hurrican(void) { return &game_data; }
