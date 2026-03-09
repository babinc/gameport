#include "../catalog.h"

static const char *keys[] = {
    "Arrow keys|Move / look up / duck",
    "Space / Up|Jump",
    "Ctrl|Action (run / grab)",
    "P / Esc|Pause menu",
    NULL
};

static const char *play[] = {"./supertux2", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download AppImage (~90 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/SuperTux/supertux/releases/download/v0.6.3/SuperTux-v0.6.3.glibc2.29-x86_64.AppImage",
    .clone_dir = "supertux",
    .bin = "supertux2",
    .play_cmd = play,
}};

static const Game game_data = {
    .name = "SuperTux", .icon = "S",
    .desc = "Classic 2D side-scrolling platformer starring Tux the Linux mascot. Run, jump, and collect powerups through colorful worlds inspired by Super Mario Bros.",
    .keys = keys, .category = "Platformer",
    .engine = "SDL2 (OpenGL)", .repo = "https://github.com/SuperTux/supertux",
    .platforms = PLATFORMS_LINUX,
    .sources = sources, .num_sources = 1,
};

const Game *game_supertux(void) { return &game_data; }
