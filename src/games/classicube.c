#include "../catalog.h"

static const char *keys[] = {
    "WASD|Move",
    "Space|Jump",
    "LMB|Destroy block",
    "RMB|Place block",
    "MMB|Pick block",
    "B|Open inventory",
    "1-9|Select hotbar slot",
    "T|Open chat",
    "Tab|Player list",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./ClassiCube", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download tar.gz (~5 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://cdn.classicube.net/client/release/nix64/ClassiCube.tar.gz",
    .clone_dir = "classicube",
    .archive_type = "tar.gz",
    .bin = "ClassiCube",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "ClassiCube", .icon = "C",
    .desc = "A lightweight Minecraft Classic client written from scratch in C. Build, explore, and play multiplayer on ClassiCube servers. Extremely fast and portable.",
    .keys = keys, .category = "Action",
    .engine = "OpenGL (custom C)", .repo = "https://github.com/ClassiCube/ClassiCube",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_classicube(void) { return &game_data; }
