#include "../catalog.h"

static const char *keys[] = {
    "h j k l / Arrows|Move (cardinal)",
    "y u b n|Move (diagonal)",
    ".|Wait one turn",
    ",|Pick up item",
    "d|Drop item",
    "i|Inventory",
    "w|Wield weapon",
    "W|Wear armor",
    "e|Eat food",
    "q|Quaff potion",
    "r|Read scroll/book",
    "z|Zap wand",
    "f|Fire projectile",
    "o|Open door",
    "c|Close door",
    "s|Search for traps",
    "< / >|Go up / down stairs",
    "#pray|Pray to your god",
    "S|Save and quit",
    "?|Help",
    NULL
};

static const char *platforms[] = {"linux", "macos", NULL};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Fetching Lua...'\n"
    "make fetch-lua\n"
    "echo 'Configuring NetHack...'\n"
    "sed -i 's|^#PREFIX=.*|PREFIX='\"$(pwd)\"'/install|' sys/unix/hints/linux\n"
    "sed -i 's|^PREFIX=.*|PREFIX='\"$(pwd)\"'/install|' sys/unix/hints/linux\n"
    "sed -i 's|^HACKDIR=.*|HACKDIR='\"$(pwd)\"'/install/games/lib/nethackdir|' sys/unix/hints/linux\n"
    "sed -i 's|^SHELLDIR=.*|SHELLDIR='\"$(pwd)\"'/install/games|' sys/unix/hints/linux\n"
    "sed -i 's|^INSTDIR=.*|INSTDIR='\"$(pwd)\"'/install/games/lib/nethackdir|' sys/unix/hints/linux\n"
    "sed -i 's|^VARDIR=.*|VARDIR='\"$(pwd)\"'/install/games/lib/nethackdir/var|' sys/unix/hints/linux\n"
    "sys/unix/setup.sh sys/unix/hints/linux\n"
    "echo 'Building NetHack...'\n"
    "make -j$(nproc 2>/dev/null || echo 4)\n"
    "echo 'Installing locally...'\n"
    "make install",
    NULL
};
static const char *play[] = {"./install/games/nethack", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "bison", "flex", "libncurses5-dev", NULL};
static const char *linux_check[] = {"dpkg", "-s", "bison", NULL};
static const char *mac_install[] = {"brew", "install", "bison", "flex", NULL};
static const char *mac_check[] = {"brew", "list", "bison", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential bison flex libncurses5-dev", linux_install, linux_check, 1 },
    { "macos", "bison flex (via Homebrew)", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (make, terminal)",
    .clone_url = "https://github.com/NetHack/NetHack.git",
    .clone_dir = "nethack", .shallow = 0,
    .build_cmd = build, .play_cmd = play,
    .bin = "nethack",
}};

static const Game game_data = {
    .name = "NetHack", .icon = "N",
    .desc = "The granddaddy of roguelikes. Descend into the Mazes of Menace, retrieve the Amulet of Yendor, and ascend. Incredibly deep gameplay with emergent interactions between hundreds of items and monsters.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .repo = "https://github.com/NetHack/NetHack",
    .platforms = platforms, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_nethack(void) { return &game_data; }
