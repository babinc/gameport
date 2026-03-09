#include "../catalog.h"

static const char *keys[] = {
    "Arrows / WASD|Slide tiles",
    "Q|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "cli_2048", NULL};

static const Source sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "2048", uninstall,
}};

static const Game game_data = {
    "2048", "2",
    "Slide all tiles in one direction. Matching numbers merge and double. Keep merging to reach 2048. Board fills up = game over.",
    keys, "Puzzle",
    "crossterm", "https://crates.io/crates/cli_2048",
    NULL, NULL, 0, sources, 1,
};

const Game *game_2048(void) { return &game_data; }
