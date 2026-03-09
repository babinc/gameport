#include "../catalog.h"

static const char *keys[] = {
    "Left / J|Move left",
    "Right / L|Move right",
    "Down / K|Soft drop",
    "Space|Hard drop",
    "Z|Rotate",
    "P|Pause",
    "Esc|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "sxtetris", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_CARGO, .label = "cargo install",
    .bin = "sxtetris", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Tetris", .icon = "T",
    .desc = "Falling block puzzle. Rotate and position tetrominoes to fill complete rows. Completed rows disappear. Game ends when blocks stack to the top.",
    .keys = keys, .category = "Action",
    .engine = "ratatui", .repo = "https://crates.io/crates/sxtetris",
    .sources = sources, .num_sources = 1,
};

const Game *game_tetris(void) { return &game_data; }
