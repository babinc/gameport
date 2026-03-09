#include "../catalog.h"

static const char *keys[] = {
    "Arrows / WASD|Slide tiles",
    "Q|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "cli_2048", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_CARGO, .label = "cargo install",
    .bin = "2048", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "2048", .icon = "2",
    .desc = "Slide all tiles in one direction. Matching numbers merge and double. Keep merging to reach 2048. Board fills up = game over.",
    .keys = keys, .category = "Puzzle",
    .engine = "crossterm", .repo = "https://crates.io/crates/cli_2048",
    .sources = sources, .num_sources = 1,
};

const Game *game_2048(void) { return &game_data; }
