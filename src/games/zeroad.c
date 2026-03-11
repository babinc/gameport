#include "../core/catalog.h"

static const char *keys[] = {
    "LMB|Select / command",
    "RMB|Move / attack / gather",
    "Ctrl+#|Assign group",
    "#|Select group",
    "H|Halt",
    "Del|Delete unit",
    "Space|Focus on alert",
    "Tab|Toggle diplomacy",
    "F10|Game menu",
    "P|Pause",
    "Esc|Deselect / menu",
    NULL
};

static const char *play[] = {"./zeroad", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~1.7 GB)",
    .platforms = PLAT_LINUX,
    .url = "https://releases.wildfiregames.com/0ad-0.28.0-x86_64.AppImage",
    .dir = "zeroad",
    .bin = "zeroad",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "0 A.D.", .icon = "0",
    .desc = "A free, open-source RTS spanning ancient civilizations. Build cities, raise armies, and wage war across historically inspired factions with AAA-quality art and gameplay.",
    .keys = keys, .category = "Strategy",
    .engine = "Pyrogenesis", .website = "https://play0ad.com/",
    .repo = "https://github.com/0ad/0ad",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_zeroad(void) { return &game_data; }
