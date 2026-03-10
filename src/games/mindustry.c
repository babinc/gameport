#include "../catalog.h"

static const char *keys[] = {
    "WASD|Pan camera",
    "LMB|Place / mine",
    "RMB|Deselect / break",
    "Mousewheel|Zoom",
    "R|Rotate building",
    "E|Research tree",
    "Q|Command mode",
    "Tab|Toggle minimap",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"java", "-jar", "Mindustry.jar", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download JAR (~60 MB, needs Java)",
    .url = "https://github.com/Anuken/Mindustry/releases/download/v146/Mindustry.jar",
    .dir = "mindustry",
    .bin = "Mindustry.jar",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Mindustry", .icon = "M",
    .desc = "Automation tower defense RTS. Build elaborate supply chains of conveyor belts, defend your base with turrets, and conquer sectors.",
    .keys = keys, .category = "Strategy",
    .engine = "Arc (libGDX)", .website = "https://mindustrygame.github.io/",
    .repo = "https://github.com/Anuken/Mindustry",
    .platforms = PLAT_ALL, .sources = sources, .num_sources = 1,
};

const Game *game_mindustry(void) { return &game_data; }
