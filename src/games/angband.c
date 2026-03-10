#include "../catalog.h"

static const char *keys[] = {
    "hjklyubn|Move (8 directions)",
    "o|Open door",
    "c|Close door",
    "g|Get (pick up)",
    "d|Drop",
    "i|Inventory",
    "w|Wear / wield",
    "f|Fire ranged weapon",
    "m|Cast spell",
    "R|Rest",
    ">|Go down stairs",
    "<|Go up stairs",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "./autogen.sh\n"
    "./configure --with-no-install\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./src/angband", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "autoconf", "automake", "libncurses-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s autoconf libncurses-dev >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "autoconf", "automake", NULL};
static const char *mac_check[] = {"brew", "list", "autoconf", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential autoconf automake libncurses-dev", linux_install, linux_check, 1 },
    { "macos", "autoconf automake", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + autotools)",
    .url = "https://github.com/angband/angband.git",
    .dir = "angband", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "angband",
}};

static const Game game_data = {
    .name = "Angband", .icon = "A",
    .desc = "Classic dungeon-crawling roguelike. Delve deep into the fortress of Angband to confront the dark lord Morgoth. Dozens of races, classes, and monsters.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .website = "https://rephial.org/",
    .repo = "https://github.com/angband/angband",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_angband(void) { return &game_data; }
