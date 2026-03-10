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
    .method = ACQUIRE_CARGO, .label = "cargo install",
    .bin = "rec-wars", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "RecWars", .icon = "W",
    .desc = "Top-down tank deathmatch. Drive tanks, hovercraft, or hummers and blast opponents with 8 weapons. Free-for-all, team war, or capture the cow. Graphical window.",
    .keys = keys, .category = "Action",
    .engine = "macroquad", .repo = "https://crates.io/crates/rec-wars",
    .platforms = PLAT_ALL, .sources = sources, .num_sources = 1,
};

const Game *game_recwars(void) { return &game_data; }
