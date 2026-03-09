#include "../catalog.h"

static const char *keys[] = {
    "W/S|Increase / decrease thrust",
    "A/D|Yaw left / right",
    "Q/E|Roll left / right",
    "I|Pitch up",
    "K|Pitch down",
    "F1|Sector map",
    "F2|System map",
    "F3|System info",
    "F5|Flight mode",
    "Tab|Time acceleration",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./pioneer", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~60 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/pioneerspacesim/pioneer/releases/download/20260203/Pioneer-x86_64.AppImage",
    .clone_dir = "pioneer",
    .bin = "pioneer",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Pioneer", .icon = "P",
    .desc = "A space adventure game inspired by Frontier: Elite II. Explore a realistic galaxy with Newtonian physics, trade between stations, and take on missions.",
    .keys = keys, .category = "Simulation",
    .engine = "SDL2 (OpenGL)", .repo = "https://github.com/pioneerspacesim/pioneer",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_pioneer(void) { return &game_data; }
