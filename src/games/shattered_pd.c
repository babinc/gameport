#include "../catalog.h"

static const char *keys[] = {
    "LMB / Tap|Move / interact",
    "RMB|Examine",
    "I|Inventory",
    "W|Wait one turn",
    "S|Search",
    "E|Examine",
    "Q|Quick slot",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./bin/Shattered Pixel Dungeon", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Linux zip (~40 MB)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/00-Evan/shattered-pixel-dungeon/releases/download/v3.3.7/ShatteredPD-v3.3.7-Linux.zip",
    .dir = "shattered-pd",
    .archive_type = "zip",
    .bin = "bin/Shattered Pixel Dungeon",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Shattered Pixel Dungeon", .icon = "P",
    .desc = "A hugely popular roguelike RPG dungeon crawler. Explore randomized floors, collect loot, and fight through permadeath with pixel art charm. Four hero classes with unique abilities.",
    .keys = keys, .category = "Roguelike",
    .engine = "libGDX", .website = "https://shatteredpixel.com/",
    .repo = "https://github.com/00-Evan/shattered-pixel-dungeon",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_shattered_pd(void) { return &game_data; }
