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
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "chess-tui", uninstall,
}};

static const Game game_data = {
    "Chess TUI", "K",
    "Full chess in the terminal. Play vs a friend locally, against a UCI engine (Stockfish), or online via Lichess. Supports multiple board skins.",
    keys, "Strategy",
    "ratatui", "https://crates.io/crates/chess-tui",
    NULL, NULL, 0, sources, 1,
};

const Game *game_chess_tui(void) { return &game_data; }
