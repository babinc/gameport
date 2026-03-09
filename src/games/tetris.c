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
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "sxtetris", uninstall,
}};

static const Game game_data = {
    "Tetris", "T",
    "Falling block puzzle. Rotate and position tetrominoes to fill complete rows. Completed rows disappear. Game ends when blocks stack to the top.",
    keys, "Action",
    "ratatui", "https://crates.io/crates/sxtetris",
    NULL, NULL, 0, sources, 1,
};

const Game *game_tetris(void) { return &game_data; }
