#include "../catalog.h"

static const char *keys[] = {
    "A / D|Walk left / right",
    "W / Space|Jump / climb",
    "S|Crouch / dig down",
    "Q / E|Throw / use item",
    "LMB|Use equipped item",
    "RMB|Context menu",
    "1-0|Select inventory",
    "F|Drop",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./openclonk", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download release (~120 MB)",
    .platforms = PLATFORMS_LINUX,
    .url = "https://www.openclonk.org/builds/release/8.1/openclonk-8.1-x64.tar.bz2",
    .dir = "openclonk", .archive_type = "tar.bz2",
    .bin = "openclonk",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "OpenClonk", .icon = "C",
    .desc = "Multiplayer action game where you control nimble humanoids called clonks. Features mining, settling, and tactical combat.",
    .keys = keys, .category = "Action",
    .engine = "OpenGL (custom)", .website = "https://www.openclonk.org/",
    .repo = "https://github.com/openclonk/openclonk",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_openclonk(void) { return &game_data; }
