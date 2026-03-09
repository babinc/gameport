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

static const char *platforms[] = {"linux", NULL};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "git submodule update --init --recursive --depth 1\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake -DCMAKE_BUILD_TYPE=Release ..\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc)",
    NULL
};
static const char *play[] = {"./build/src/warzone2100", NULL};
static const char *uninstall[] = {"git-game-remove", "warzone2100", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "ninja-build", "pkg-config", "gettext",
    "libphysfs-dev", "libpng-dev", "libopenal-dev", "libvorbis-dev",
    "libopus-dev", "libtheora-dev", "libxrandr-dev", "libfreetype-dev",
    "libfribidi-dev", "libharfbuzz-dev", "libcurl4-gnutls-dev",
    "libsodium-dev", "libsqlite3-dev", "libzip-dev", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libphysfs-dev", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libphysfs-dev libpng-dev libopenal-dev ...",
      linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = METHOD_GIT, .label = "Build from source (git + cmake)",
    .clone_url = "https://github.com/Warzone2100/warzone2100.git",
    .clone_dir = "warzone2100", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "warzone2100", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Warzone 2100", .icon = "W",
    .desc = "Open-source real-time strategy set in a post-apocalyptic future. Research tech, design vehicles, and command armies. Fully free with campaign and multiplayer.",
    .keys = keys, .category = "Strategy",
    .engine = "custom (OpenGL)", .repo = "https://github.com/Warzone2100/warzone2100",
    .platforms = platforms, .platform_deps = deps, .num_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_warzone2100(void) { return &game_data; }
