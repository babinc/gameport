#include "../core/catalog.h"

static const char *keys[] = {
    "Arrow keys|Move / attack adjacent",
    ".|Wait one turn",
    "o|Auto-explore",
    "G|Auto-travel",
    "Tab|Auto-fight nearest",
    "g|Pick up item",
    "i|Inventory",
    "d|Drop item",
    "z|Cast spell",
    "f|Fire ranged weapon",
    "< / >|Use stairs",
    "X|Examine map",
    "Ctrl+S|Save and quit",
    "?|Help",
    NULL
};

static const char *play_linux[] = {"./dcss", NULL};
static const char *play_win[] = {"crawl.exe", NULL};

static const Source sources[] = {
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (console, ~11 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/crawl/crawl/releases/download/0.34.0/dcss-0.34.0-linux-console.x86_64.AppImage",
    .dir = "dcss",
    .bin = "dcss",
    .play_cmd = play_linux,
},
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download zip (tiles, ~25 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/crawl/crawl/releases/download/0.34.0/dcss-0.34.0-win32-tiles.zip",
    .dir = "dcss", .archive_type = "zip",
    .bin = "crawl.exe",
    .play_cmd = play_win,
},
};

static const Game game_data = {
    .name = "Dungeon Crawl Stone Soup", .icon = "D",
    .desc = "One of the greatest traditional roguelikes. Descend through procedurally generated dungeons, choose from dozens of species and backgrounds, and face permadeath in deep tactical combat.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .website = "https://crawl.develz.org/",
    .repo = "https://github.com/crawl/crawl",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_dcss(void) { return &game_data; }
