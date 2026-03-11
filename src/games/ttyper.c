#include "../core/catalog.h"

static const char *keys[] = {
    "Type|Match the displayed words",
    "Esc|End test early and show results",
    "Ctrl+C|Quit without results",
    NULL
};

static const char *uninstall[] = {"cargo", "uninstall", "ttyper", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_CARGO,
    .label = "Install via cargo",
    .bin = "ttyper", .uninstall_cmd = uninstall,
}};

static const Game game_data = {
    .name = "ttyper", .icon = "T",
    .desc = "Terminal-based typing test. Practice touch typing with random English words, see your WPM and accuracy in real time. Supports custom word lists and languages.",
    .keys = keys, .category = "Typing",
    .engine = "ratatui", .repo = "https://github.com/max-niederman/ttyper",
    .platforms = PLAT_ALL, .sources = sources, .num_sources = 1,
};

const Game *game_ttyper(void) { return &game_data; }
