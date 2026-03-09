#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select / command",
    "RMB|Move / attack",
    "Ctrl+#|Assign group",
    "#|Select group",
    "A|Attack move",
    "S|Stop",
    "G|Guard",
    "X|Scatter",
    "F|Force fire",
    "Tab|Cycle bases",
    "F5|Quick save",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./openra-ra", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~40 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/OpenRA/OpenRA/releases/download/release-20250330/OpenRA-Red-Alert-x86_64.AppImage",
    .clone_dir = "openra-ra",
    .bin = "openra-ra",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "OpenRA: Red Alert", .icon = "R",
    .desc = "A modern reimplementation of Command & Conquer: Red Alert with updated UI, multiplayer, and AI. Build bases, harvest ore, and command Allied or Soviet forces.",
    .keys = keys, .category = "Strategy",
    .engine = "Mono (OpenGL)", .website = "https://www.openra.net/",
    .repo = "https://github.com/OpenRA/OpenRA",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_openra(void) { return &game_data; }
