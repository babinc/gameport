#include "../catalog.h"

static const char *keys[] = {
    "W / Up|Forward",
    "S / Down|Backward",
    "A|Strafe left",
    "D|Strafe right",
    "Q|Turn left",
    "E|Turn right",
    "Space|Fire",
    "X|Drop mine",
    "F|Cycle weapon",
    "P / O|Next / prev weapon",
    "Tab|Toggle map",
    "; / `|Open console",
    "G|Self-destruct",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "rec-wars", NULL};

static const Source sources[] = {{
    METHOD_CARGO, "cargo install",
    "", "", 1, NULL, NULL, "rec-wars", uninstall,
}};

static const Game game_data = {
    "RecWars", "W",
    "Top-down tank deathmatch. Drive tanks, hovercraft, or hummers and blast opponents with 8 weapons. Free-for-all, team war, or capture the cow. Graphical window.",
    keys, "Action",
    "macroquad", "https://crates.io/crates/rec-wars",
    NULL, NULL, 0, sources, 1,
};

const Game *game_recwars(void) { return &game_data; }
