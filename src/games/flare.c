#include "../core/catalog.h"

static const char *keys[] = {
    "LMB|Move / attack",
    "RMB|Use power",
    "1-0|Action bar slots",
    "I|Inventory",
    "C|Character sheet",
    "L|Log",
    "M|Minimap",
    "Esc|Menu",
    NULL
};

static const char *play_linux[] = {"./flare", NULL};
static const char *play_win[] = {"flare.exe", NULL};

static const Source sources[] = {
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~30 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/flareteam/flare-game/releases/download/v1.14/flare-linux64-v1.14.AppImage",
    .dir = "flare",
    .bin = "flare",
    .play_cmd = play_linux,
},
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download zip (~30 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/flareteam/flare-game/releases/download/v1.14/flare-win-v1.14.zip",
    .dir = "flare", .archive_type = "zip",
    .bin = "flare.exe",
    .play_cmd = play_win,
},
};

static const Game game_data = {
    .name = "Flare RPG", .icon = "F",
    .desc = "An isometric action RPG inspired by Diablo. Explore dungeons, fight monsters, collect loot, and level up. Fully self-contained with original art and story.",
    .keys = keys, .category = "RPG",
    .engine = "SDL2", .website = "https://flarerpg.org/",
    .repo = "https://github.com/flareteam/flare-game",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_flare(void) { return &game_data; }
