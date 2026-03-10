#include "../catalog.h"

static const char *keys[] = {
    "W/S|Accelerate / reverse",
    "A/D|Turn left / right",
    "Space|Fire primary",
    "Tab|Target nearest hostile",
    "T|Target nearest",
    "L|Land on planet",
    "J|Jump to system",
    "H|Open hail",
    "M|Star map",
    "I|Inventory",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./naev", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~200 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/naev/naev/releases/download/v0.12.6/naev-0.12.6-linux-x86-64.AppImage",
    .dir = "naev",
    .bin = "naev",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Naev", .icon = "N",
    .desc = "A 2D space trading and combat game inspired by Escape Velocity. Explore a vast galaxy, trade goods, take on missions, fight pirates, and build faction reputation.",
    .keys = keys, .category = "Simulation",
    .engine = "SDL2 (OpenGL)", .website = "https://naev.org/",
    .repo = "https://github.com/naev/naev",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_naev(void) { return &game_data; }
