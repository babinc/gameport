#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select action",
    "F|Fold",
    "C|Check / call",
    "R|Raise",
    "A|All-in",
    "Space|Start next hand",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./pokerth", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~30 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/pokerth/pokerth/releases/download/v2.0.6/PokerTH-2.0.6.AppImage",
    .clone_dir = "pokerth",
    .bin = "pokerth",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "PokerTH", .icon = "$",
    .desc = "Texas Hold'em poker with AI opponents and online multiplayer. Features a polished interface, lobby system, and customizable rules.",
    .keys = keys, .category = "Card",
    .engine = "Qt/SDL", .website = "https://www.pokerth.net/",
    .repo = "https://github.com/pokerth/pokerth",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_pokerth(void) { return &game_data; }
