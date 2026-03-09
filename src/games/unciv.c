#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select / move / interact",
    "RMB|Deselect",
    "Arrow keys|Scroll map",
    "+/-|Zoom in / out",
    "Space|Next turn",
    "N|Next unit",
    "Ctrl+S|Quick save",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./Unciv", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Linux zip (~50 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/yairm210/Unciv/releases/download/4.19.17-patch1/Unciv-Linux64.zip",
    .clone_dir = "unciv",
    .archive_type = "zip",
    .bin = "Unciv",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Unciv", .icon = "U",
    .desc = "An open-source remake of Civilization V with pixel art. Build empires, research technology, wage war, and pursue victory across multiple conditions. Mod support and multiplayer.",
    .keys = keys, .category = "Strategy",
    .engine = "libGDX", .repo = "https://github.com/yairm210/Unciv",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_unciv(void) { return &game_data; }
