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
    .method = METHOD_CARGO, .label = "cargo install",
    .bin = "cmd-minesweeper", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Minesweeper", .icon = "#",
    .desc = "Reveal tiles without hitting mines. Numbers show adjacent mine count. Flag tiles you think are mines. Clear all safe tiles to win.",
    .keys = keys, .category = "Puzzle",
    .engine = "crossterm", .repo = "https://crates.io/crates/cmd-minesweeper",
    .sources = sources, .num_sources = 1,
};

const Game *game_minesweeper(void) { return &game_data; }
