#include "../catalog.h"

static const char *keys[] = {
    "Arrows / HJKL|Move cursor",
    "Space|Select / place piece",
    "Esc|Deselect / close popup",
    "S|Cycle board skins",
    "B|Back to menu",
    "?|Help menu",
    "Q|Quit",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "chess-tui", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_CARGO, .label = "cargo install",
    .bin = "chess-tui", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "Chess TUI", .icon = "K",
    .desc = "Full chess in the terminal. Play vs a friend locally, against a UCI engine (Stockfish), or online via Lichess. Supports multiple board skins.",
    .keys = keys, .category = "Strategy",
    .engine = "ratatui", .repo = "https://crates.io/crates/chess-tui",
    .sources = sources, .num_sources = 1,
};

const Game *game_chess_tui(void) { return &game_data; }
