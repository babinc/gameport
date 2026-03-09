#include "../catalog.h"

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

static const char *linux_platforms[] = {"linux", NULL};

static const char *play[] = {"./dcss", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (console, ~11 MB)",
    .platforms = linux_platforms,
    .clone_url = "https://github.com/crawl/crawl/releases/download/0.34.0/dcss-0.34.0-linux-console.x86_64.AppImage",
    .clone_dir = "dcss",
    .bin = "dcss",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Dungeon Crawl Stone Soup", .icon = "D",
    .desc = "One of the greatest traditional roguelikes. Descend through procedurally generated dungeons, choose from dozens of species and backgrounds, and face permadeath in deep tactical combat.",
    .keys = keys, .category = "Roguelike",
    .engine = "ncurses", .repo = "https://github.com/crawl/crawl",
    .platforms = linux_platforms,
    .sources = sources, .num_sources = 1,
};

const Game *game_dcss(void) { return &game_data; }
