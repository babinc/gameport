#include "../core/catalog.h"

static const char *keys[] = {
    "Arrow keys / Numpad|Move",
    "< / >|Ascend / descend stairs",
    "i|Inventory",
    "e|Equipment",
    "a|Apply / use item",
    "d|Drop item",
    "t|Throw item",
    "x|Examine surroundings",
    "z|Rest (one turn)",
    "Z|Auto-rest to full",
    "S|Save and quit",
    "Esc|Cancel / back",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Building Brogue CE...'\n"
    "make bin/brogue",
    NULL
};
static const char *play[] = {"bash", "-c", "cd bin && ./brogue", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "make", "gcc", "diffutils", "libsdl2-dev", "libsdl2-image-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev libsdl2-image-dev >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", "sdl2_image", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2_image", NULL};

static const PlatformDeps deps[] = {
    { "linux", "make gcc libsdl2-dev libsdl2-image-dev", linux_install, linux_check, 1 },
    { "macos", "SDL2 SDL2_image", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (make + SDL2)",
    .url = "https://github.com/tmewett/BrogueCE.git",
    .dir = "brogue-ce", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "brogue",
}};

static const Game game_data = {
    .name = "Brogue CE", .icon = "B",
    .desc = "A beautiful minimalist roguelike. Explore a 26-level dungeon with gorgeous ASCII art, clever puzzles, and deeply tactical combat. Community Edition of the classic.",
    .keys = keys, .category = "Roguelike",
    .engine = "SDL2", .repo = "https://github.com/tmewett/BrogueCE",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_brogue(void) { return &game_data; }
