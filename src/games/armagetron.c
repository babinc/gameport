#include "../core/catalog.h"

static const char *keys[] = {
    "Left / Right|Turn",
    "Up|Boost",
    "Down / Space|Brake",
    "Enter|Chat",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./armagetron", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~20 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://launchpad.net/armagetronad/0.2.9/0.2.9.2.5/+download/ArmagetronAdvanced.AppImage",
    .dir = "armagetron",
    .bin = "armagetron",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Armagetron Advanced", .icon = "A",
    .desc = "Multiplayer 3D light cycle game based on the movie Tron. Navigate your light cycle and force opponents to crash into walls.",
    .keys = keys, .category = "Action",
    .engine = "OpenGL (custom)", .website = "https://www.armagetronad.org/",
    .repo = "https://gitlab.com/armagetronad/armagetronad",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_armagetron(void) { return &game_data; }
