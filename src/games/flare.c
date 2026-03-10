#include "../catalog.h"

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

static const char *play[] = {"./flare", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~30 MB)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/flareteam/flare-game/releases/download/v1.14/flare-linux64-v1.14.AppImage",
    .dir = "flare",
    .bin = "flare",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Flare RPG", .icon = "F",
    .desc = "An isometric action RPG inspired by Diablo. Explore dungeons, fight monsters, collect loot, and level up. Fully self-contained with original art and story.",
    .keys = keys, .category = "RPG",
    .engine = "SDL2", .website = "https://flarerpg.org/",
    .repo = "https://github.com/flareteam/flare-game",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_flare(void) { return &game_data; }
