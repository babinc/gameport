#include "../catalog.h"

static const char *keys[] = {
    "Arrow keys / Numpad|Move",
    ".|Wait one turn",
    "g / ,|Pick up items",
    "d|Drop item",
    "i|Inventory",
    "w|Wear / wield",
    "e|Eat",
    "a|Apply / activate",
    "f|Fire ranged weapon",
    "s|Smash terrain",
    "B|Butcher corpse",
    "m|Map view",
    "?|Help / keybindings",
    "Esc|Cancel / menu",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Building Cataclysm: DDA (terminal version)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./cataclysm", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "libncurses5-dev", "libncursesw5-dev", "gettext", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libncursesw5-dev", NULL};
static const PlatformDeps deps[] = {
    { "linux", "build-essential libncurses5-dev libncursesw5-dev", linux_install, linux_check, 1 },
    { "macos", "Xcode CLI tools", MAC_XCODE_INSTALL, MAC_XCODE_CHECK, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (make, terminal/curses)",
    .clone_url = "https://github.com/CleverRaven/Cataclysm-DDA.git",
    .clone_dir = "cataclysm-dda", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "cataclysm",
}};

static const Game game_data = {
    .name = "Cataclysm: DDA", .icon = "C",
    .desc = "A turn-based survival roguelike set in a post-apocalyptic world. Craft, build, explore, and fight to survive against zombies, mutants, and the elements. Massive open world with deep simulation.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .repo = "https://github.com/CleverRaven/Cataclysm-DDA",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_cataclysm_dda(void) { return &game_data; }
