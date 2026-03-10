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


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake .. -DCMAKE_BUILD_TYPE=Release\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "BASE_DIR=\"$HOME/.local/share/openttd/baseset\"\n"
    "mkdir -p \"$BASE_DIR\"\n"
    "for PACK in "
    "'opengfx-releases/7.1/opengfx-7.1-all.zip opengfx-7.1 OpenGFX' "
    "'opensfx-releases/1.0.3/opensfx-1.0.3-all.zip opensfx-1.0.3 OpenSFX' "
    "'openmsx-releases/0.4.2/openmsx-0.4.2-all.zip openmsx-0.4.2 OpenMSX'; do\n"
    "  set -- $PACK\n"
    "  if [ ! -d \"$BASE_DIR/$2\" ]; then\n"
    "    echo \"Downloading $3...\"\n"
    "    dl=$(mktemp)\n"
    "    curl -fSL -o \"$dl\" \"https://cdn.openttd.org/$1\"\n"
    "    unzip -o \"$dl\" -d \"$BASE_DIR\"\n"
    "    rm -f \"$dl\"\n"
    "    for tarfile in \"$BASE_DIR\"/$2*.tar; do\n"
    "      [ -f \"$tarfile\" ] && tar xf \"$tarfile\" -C \"$BASE_DIR\" && rm -f \"$tarfile\"\n"
    "    done\n"
    "    echo \"$3 installed!\"\n"
    "  else\n"
    "    echo \"$3 already installed.\"\n"
    "  fi\n"
    "done",
    NULL
};
static const char *play[] = {"./build/openttd", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libfreetype-dev",
    "libfontconfig1-dev", "libicu-dev", "libharfbuzz-dev",
    "liblzma-dev", "libpng-dev", "zlib1g-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev libfontconfig1-dev libicu-dev libharfbuzz-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", "freetype", "fontconfig",
    "icu4c", "harfbuzz", "xz", "libpng", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev libfreetype-dev libfontconfig1-dev libicu-dev", linux_install, linux_check, 1 },
    { "macos", "SDL2 freetype fontconfig icu4c harfbuzz", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .url = "https://github.com/OpenTTD/OpenTTD.git",
    .dir = "OpenTTD",
    .build_cmd = build, .play_cmd = play,
    .bin = "openttd",
}};

static const Game game_data = {
    .name = "OpenTTD", .icon = "O",
    .desc = "Open-source Transport Tycoon Deluxe. Build rail, road, air, and sea networks to transport passengers and cargo. Compete against AI or friends. Free graphics, sounds, and music included.",
    .keys = keys, .category = "Simulation",
    .engine = "SDL2", .website = "https://www.openttd.org/",
    .repo = "https://github.com/OpenTTD/OpenTTD",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_openttd(void) { return &game_data; }
