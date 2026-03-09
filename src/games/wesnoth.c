#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select / move / attack",
    "RMB|Deselect / info",
    "Arrow keys|Scroll map",
    "+/-|Zoom in / out",
    "N|Next unit with moves",
    "Space|End turn",
    "U|Undo last move",
    "R|Recruit unit",
    "Ctrl+R|Recall unit",
    "L|Show leader",
    "1-7|Select weapon (in attack)",
    "Ctrl+S|Save game",
    "Ctrl+O|Load game",
    "Esc|Menu",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake -DCMAKE_BUILD_TYPE=Release ..\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./build/wesnoth", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libboost-all-dev",
    "libsdl2-dev", "libsdl2-image-dev", "libsdl2-mixer-dev", "libsdl2-ttf-dev",
    "libcairo2-dev", "libpango1.0-dev", "libvorbis-dev",
    "libcurl4-openssl-dev", "libssl-dev", "libreadline-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libboost-all-dev libsdl2-image-dev libcairo2-dev libpango1.0-dev libvorbis-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "cmake", "boost",
    "sdl2", "sdl2_image", "sdl2_mixer", "sdl2_ttf",
    "cairo", "pango", "libvorbis", "openssl", "readline", NULL};
static const char *mac_check[] = {"brew", "list", "boost", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libboost-all-dev libsdl2-dev ...", linux_install, linux_check, 1 },
    { "macos", "cmake boost SDL2 SDL2_image SDL2_mixer SDL2_ttf ...", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (cmake + boost + SDL2)",
    .clone_url = "https://github.com/wesnoth/wesnoth.git",
    .clone_dir = "wesnoth", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "wesnoth",
}};

static const Game game_data = {
    .name = "Battle for Wesnoth", .icon = "W",
    .desc = "Turn-based tactical strategy in a high fantasy setting. Lead armies through full campaigns, recruit units, and fight on hex-grid battlefields. Rich multiplayer and modding community.",
    .keys = keys, .category = "Strategy",
    .engine = "SDL2", .repo = "https://github.com/wesnoth/wesnoth",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_wesnoth(void) { return &game_data; }
