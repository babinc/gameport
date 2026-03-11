#include "../core/catalog.h"

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

static const char *play_linux[] = {"./openra-ra", NULL};
static const char *play_win[] = {"RedAlert.exe", NULL};

static const Source sources[] = {
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~40 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/OpenRA/OpenRA/releases/download/release-20250330/OpenRA-Red-Alert-x86_64.AppImage",
    .dir = "openra-ra",
    .bin = "openra-ra",
    .play_cmd = play_linux,
},
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download portable zip (~70 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/OpenRA/OpenRA/releases/download/release-20250330/OpenRA-release-20250330-x64-winportable.zip",
    .dir = "openra-ra", .archive_type = "zip",
    .bin = "RedAlert.exe",
    .play_cmd = play_win,
},
};

static const Game game_data = {
    .name = "OpenRA: Red Alert", .icon = "R",
    .desc = "A modern reimplementation of Command & Conquer: Red Alert with updated UI, multiplayer, and AI. Build bases, harvest ore, and command Allied or Soviet forces.",
    .keys = keys, .category = "Strategy",
    .engine = "Mono (OpenGL)", .website = "https://www.openra.net/",
    .repo = "https://github.com/OpenRA/OpenRA",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_openra(void) { return &game_data; }
