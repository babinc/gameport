#include "../catalog.h"

static const char *keys[] = {
    "Arrows / hjkl|Move cursor",
    "Space / Enter|Reveal cell",
    "f|Flag cell",
    "q|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "minesweep", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_CARGO, .label = "cargo install",
    .bin = "minesweep", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Minesweeper", .icon = "#",
    .desc = "Reveal tiles without hitting mines. Numbers show adjacent mine count. Flag tiles you think are mines. Clear all safe tiles to win.",
    .keys = keys, .category = "Puzzle",
    .engine = "ratatui", .website = "https://github.com/cpcloud/minesweep-rs",
    .repo = "https://github.com/cpcloud/minesweep-rs",
    .platforms = PLAT_ALL, .sources = sources, .num_sources = 1,
};

const Game *game_minesweeper(void) { return &game_data; }
