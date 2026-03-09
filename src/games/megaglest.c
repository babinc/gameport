#include "../catalog.h"

static const char *keys[] = {
    "LMB|Select / command",
    "RMB|Move / attack",
    "Ctrl+#|Assign group",
    "#|Select group",
    "A|Attack move",
    "S|Stop",
    "H|Hold position",
    "P|Produce unit",
    "B|Build structure",
    "Space|Cycle idle units",
    "F5|Quick save",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./megaglest", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~70 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/MegaGlest/megaglest-source/releases/download/3.13.0/MegaGlest-3.13.0-x86_64.AppImage",
    .clone_dir = "megaglest",
    .bin = "megaglest",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "MegaGlest", .icon = "M",
    .desc = "A 3D real-time strategy game with multiple factions set in fantasy and historical worlds. Build bases, train armies, and battle in single-player campaigns or multiplayer.",
    .keys = keys, .category = "Strategy",
    .engine = "OpenGL (custom)", .repo = "https://github.com/MegaGlest/megaglest-source",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_megaglest(void) { return &game_data; }
