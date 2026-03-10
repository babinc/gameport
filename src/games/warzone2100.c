#include "../catalog.h"

static const char *keys[] = {
    "Arrows|Move camera",
    "Keypad +/-|Zoom in/out",
    "Keypad 4/6|Rotate camera",
    "Space|Toggle tracking camera",
    "Backspace|Snap view north",
    "LMB|Select / command",
    "RMB|Deselect",
    "Ctrl+A|Select all combat units",
    "Ctrl+0-9|Assign group",
    "0-9|Select group",
    "H|Hold position",
    "S|Stop",
    "G|Guard",
    "P|Pursue",
    "Q|Patrol",
    "R|Return for repair",
    "F1|Manufacture",
    "F2|Research",
    "F3|Build",
    "F4|Design",
    "F5|Intelligence",
    "F7|Quick save",
    "F8|Quick load",
    "Esc|Menu",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "git submodule update --init --recursive --depth 1\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake -DCMAKE_BUILD_TYPE=Release ..\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./build/src/warzone2100", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "ninja-build", "pkg-config", "gettext",
    "libphysfs-dev", "libpng-dev", "libopenal-dev", "libvorbis-dev",
    "libopus-dev", "libtheora-dev", "libxrandr-dev", "libfreetype-dev",
    "libfribidi-dev", "libharfbuzz-dev", "libcurl4-gnutls-dev",
    "libsodium-dev", "libsqlite3-dev", "libzip-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libphysfs-dev libopenal-dev libvorbis-dev libsodium-dev libsqlite3-dev libzip-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "cmake", "pkg-config",
    "gettext", "physfs", "openal-soft", "libvorbis", "opus", "theora",
    "freetype", "fribidi", "harfbuzz", "libsodium", "sqlite",
    "libzip", "libpng", NULL};
static const char *mac_check[] = {"brew", "list", "physfs", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libphysfs-dev libpng-dev libopenal-dev ...",
      linux_install, linux_check, 1 },
    { "macos", "cmake physfs openal-soft libvorbis freetype harfbuzz ...",
      mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .clone_url = "https://github.com/Warzone2100/warzone2100.git",
    .clone_dir = "warzone2100", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "build/src/warzone2100",
}};

static const Game game_data = {
    .name = "Warzone 2100", .icon = "W",
    .desc = "Open-source real-time strategy set in a post-apocalyptic future. Research tech, design vehicles, and command armies. Fully free with campaign and multiplayer.",
    .keys = keys, .category = "Strategy",
    .engine = "custom (OpenGL)", .website = "https://wz2100.net/",
    .repo = "https://github.com/Warzone2100/warzone2100",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_warzone2100(void) { return &game_data; }
