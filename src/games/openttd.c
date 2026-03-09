#include "../catalog.h"

static const char *keys[] = {
    "Mouse|Point, click, drag to build",
    "Scroll / Z|Zoom in/out",
    "Arrows|Scroll map",
    "Del|Demolish",
    "Shift+F7|Rail construction",
    "Shift+F8|Road construction",
    "Shift+F9|Dock construction",
    "Shift+F10|Airport construction",
    "M|Minimap",
    "P / F1|Pause",
    "X|Toggle transparency",
    "Esc|Close active tool",
    "Ctrl+S|Screenshot",
    NULL
};

static const char *platforms[] = {"linux", "macos", NULL};
static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake .. -DCMAKE_BUILD_TYPE=Release\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc)",
    NULL
};
static const char *play[] = {"./build/openttd", NULL};
static const char *uninstall[] = {"git-game-remove", "OpenTTD", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libfreetype-dev",
    "libfontconfig1-dev", "libicu-dev", "libharfbuzz-dev",
    "liblzma-dev", "libpng-dev", "zlib1g-dev", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libsdl2-dev", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", "freetype", "fontconfig",
    "icu4c", "harfbuzz", "xz", "libpng", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev libfreetype-dev libfontconfig1-dev libicu-dev", linux_install, linux_check, 1 },
    { "macos", "SDL2 freetype fontconfig icu4c harfbuzz", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = METHOD_GIT, .label = "Build from source (git + cmake)",
    .clone_url = "https://github.com/OpenTTD/OpenTTD.git",
    .clone_dir = "OpenTTD",
    .build_cmd = build, .play_cmd = play,
    .bin = "OpenTTD", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "OpenTTD", .icon = "O",
    .desc = "Open-source Transport Tycoon Deluxe. Build rail, road, air, and sea networks to transport passengers and cargo. Compete against AI or friends. Free assets included.",
    .keys = keys, .category = "Simulation",
    .engine = "SDL2", .repo = "https://github.com/OpenTTD/OpenTTD",
    .platforms = platforms, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_openttd(void) { return &game_data; }
