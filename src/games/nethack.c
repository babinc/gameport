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


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Fetching Lua...'\n"
    "make fetch-lua\n"
    "echo 'Generating hints file...'\n"
    "P=$(pwd)/install\n"
    "H=$P/games/lib/nethackdir\n"
    "cat > sys/unix/hints/gameport << HINTS\n"
    "# GamePort-generated hints file\n"
    "PREFIX=$P\n"
    "HACKDIR=$H\n"
    "SHELLDIR=$P/games\n"
    "INSTDIR=$H\n"
    "VARDIR=$H/var\n"
    "POSTINSTALL=cp -n sys/unix/sysconf $H/sysconf\n"
    "CC=gcc\n"
    "CFLAGS=-g -O2 -I../include -DNOTPARMDECL\n"
    "CFLAGS+=-DDLB\n"
    "CFLAGS+=-DCOMPRESS=\\\"/bin/gzip\\\" -DCOMPRESS_EXTENSION=\\\".gz\\\"\n"
    "CFLAGS+=-DSYSCF -DSYSCF_FILE=\\\"$H/sysconf\\\"\n"
    "CFLAGS+=-DGCC_WARN -Wall -Wextra -Wno-missing-field-initializers\n"
    "LINK=gcc\n"
    "WINTTYLIB=-lncurses\n"
    "WINSRC=\\$(WINTTYSRC)\n"
    "WINOBJ=\\$(WINTTYOBJ)\n"
    "WINLIB=\\$(WINTTYLIB)\n"
    "HINTS\n"
    "sys/unix/setup.sh sys/unix/hints/gameport\n"
    "echo 'Building NetHack...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Installing locally...'\n"
    "make install",
    NULL
};
static const char *play[] = {"./install/games/nethack", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "bison", "flex", "libncurses5-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s bison flex libncurses5-dev >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "bison", "flex", NULL};
static const char *mac_check[] = {"brew", "list", "bison", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential bison flex libncurses5-dev", linux_install, linux_check, 1 },
    { "macos", "bison flex (via Homebrew)", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (make, terminal)",
    .clone_url = "https://github.com/NetHack/NetHack.git",
    .clone_dir = "nethack", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "install/games/nethack",
}};

static const Game game_data = {
    .name = "NetHack", .icon = "N",
    .desc = "The granddaddy of roguelikes. Descend into the Mazes of Menace, retrieve the Amulet of Yendor, and ascend. Incredibly deep gameplay with emergent interactions between hundreds of items and monsters.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .website = "https://www.nethack.org/",
    .repo = "https://github.com/NetHack/NetHack",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_nethack(void) { return &game_data; }
