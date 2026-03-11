#include "../core/catalog.h"

static const char *keys[] = {
    "Arrows|Move",
    "Space|Jump",
    "L.Ctrl|Switch character",
    "Enter|Pause / confirm",
    "Esc|Quit / cancel",
    NULL
};

static const char *play_win[] = {"opensurge.exe", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download zip (~40 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/alemart/opensurge/releases/download/v0.6.1.3/opensurge-0.6.1.3-windows.zip",
    .dir = "opensurge", .archive_type = "zip",
    .bin = "opensurge.exe",
    .play_cmd = play_win,
}};

static const Game game_data = {
    .name = "Open Surge", .icon = "S",
    .desc = "Sonic-inspired 2D platformer with a built-in level editor. "
            "Run, jump, and spin through colorful levels at high speed. "
            "Create your own levels and games with the SurgeScript engine.",
    .keys = keys, .category = "Platformer",
    .engine = "Allegro", .website = "https://opensurge2d.org",
    .repo = "https://github.com/alemart/opensurge",
    .platforms = PLAT_WINDOWS,
    .sources = sources, .num_sources = 1,
};

const Game *game_opensurge(void) { return &game_data; }
