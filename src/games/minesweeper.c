#include "../catalog.h"

static const char *keys[] = {
    "Arrows / WASD|Move cursor",
    "Q|Uncover cell",
    "E|Flag cell",
    "Ctrl+Q / Esc|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "cmd-minesweeper", NULL};

static const Source sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "cmd-minesweeper", uninstall,
}};

static const Game game_data = {
    "Minesweeper", "#",
    "Reveal tiles without hitting mines. Numbers show adjacent mine count. Flag tiles you think are mines. Clear all safe tiles to win.",
    keys, "Puzzle",
    "crossterm", "https://crates.io/crates/cmd-minesweeper",
    NULL, NULL, 0, sources, 1,
};

const Game *game_minesweeper(void) { return &game_data; }
