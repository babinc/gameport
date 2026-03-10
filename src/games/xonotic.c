#include "../catalog.h"

static const char *keys[] = {
    "WASD|Move",
    "Space|Jump",
    "LCtrl|Crouch",
    "LMB|Primary fire",
    "RMB|Secondary fire",
    "Mousewheel|Switch weapon",
    "1-9|Select weapon",
    "Tab|Scoreboard",
    "T|Chat",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./xonotic-linux-sdl.sh", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download release (~1.2 GB)",
    .platforms = PLAT_LINUX,
    .url = "https://dl.xonotic.org/xonotic-0.8.6.zip",
    .dir = "xonotic", .archive_type = "zip",
    .bin = "xonotic-linux-sdl.sh",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Xonotic", .icon = "X",
    .desc = "Fast-paced arena FPS with crisp movement and a wide array of weapons. Quake-like gameplay, fully free with original art and maps.",
    .keys = keys, .category = "Shooter",
    .engine = "DarkPlaces", .website = "https://xonotic.org/",
    .repo = "https://gitlab.com/xonotic/xonotic",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_xonotic(void) { return &game_data; }
