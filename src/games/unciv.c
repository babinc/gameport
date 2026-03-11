#include "../core/catalog.h"

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
static const char *play_win[] = {".\\Unciv.exe", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Linux zip (~50 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/yairm210/Unciv/releases/download/4.19.17-patch1/Unciv-Linux64.zip",
    .dir = "unciv",
    .archive_type = "zip",
    .bin = "Unciv",
    .play_cmd = play,
}, {
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Windows zip (~50 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/yairm210/Unciv/releases/download/4.19.17-patch1/Unciv-Windows64.zip",
    .dir = "unciv",
    .archive_type = "zip",
    .bin = "Unciv.exe",
    .play_cmd = play_win,
}};

static const Game game_data = {
    .name = "Unciv", .icon = "U",
    .desc = "An open-source remake of Civilization V with pixel art. Build empires, research technology, wage war, and pursue victory across multiple conditions. Mod support and multiplayer.",
    .keys = keys, .category = "Strategy",
    .engine = "libGDX", .repo = "https://github.com/yairm210/Unciv",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_unciv(void) { return &game_data; }
