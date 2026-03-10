#include "../catalog.h"

static const char *keys[] = {
    "WASD|Move",
    "Space|Jump / climb",
    "LShift|Glide",
    "LMB|Attack",
    "RMB|Secondary attack",
    "F|Interact / mount",
    "M|Map",
    "B|Bag / inventory",
    "Alt|Block",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./airshipper", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Airshipper launcher (~30 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://gitlab.com/veloren/airshipper/-/releases/permalink/latest/downloads/binaries/linux-client-x86_64.zip",
    .dir = "veloren", .archive_type = "zip",
    .bin = "airshipper",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "Veloren", .icon = "V",
    .desc = "Multiplayer voxel RPG written in Rust. Open-world action RPG set in a vast fantasy world with crafting, combat, and exploration.",
    .keys = keys, .category = "RPG",
    .engine = "wgpu (Rust)", .website = "https://veloren.net/",
    .repo = "https://gitlab.com/veloren/veloren",
    .platforms = PLAT_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_veloren(void) { return &game_data; }
