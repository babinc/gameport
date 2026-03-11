#include "../core/catalog.h"

static const char *keys[] = {
    "Up|Accelerate",
    "Down|Brake",
    "Left / Right|Steer",
    "Space|Gear up",
    "X|Gear down",
    "F2|Driver view",
    "Esc|Menu / pause",
    NULL
};

static const char *play[] = {"./speed-dreams", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~500 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://files.speed-dreams.net/public/speed-dreams-v2.4.2-x86_64.AppImage",
    .dir = "speed-dreams",
    .bin = "speed-dreams",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Speed Dreams", .icon = "S",
    .desc = "Open-source motorsport simulator with high-quality 3D graphics and accurate physics. Fork of TORCS targeting maximum realism.",
    .keys = keys, .category = "Racing",
    .engine = "TORCS (OpenGL)", .website = "https://www.speed-dreams.net/",
    .repo = "https://forge.a-lec.org/speed-dreams/speed-dreams-code",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_speed_dreams(void) { return &game_data; }
