#include "../catalog.h"

static const char *keys[] = {
    "Arrow keys|Move",
    "Z|Fire",
    "X|Switch weapon",
    "C|Use / interact",
    "Esc|Pause menu",
    NULL
};

static const char *play[] = {"./bin/cdogs-sdl", NULL};
static const char *play_win[] = {".\\cdogs-sdl.exe", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download tar.gz (~20 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/cxong/cdogs-sdl/releases/download/2.4.0/C-Dogs.SDL-2.4.0-Linux.tar.gz",
    .clone_dir = "cdogs",
    .archive_type = "tar.gz",
    .bin = "bin/cdogs-sdl",
    .play_cmd = play,
}, {
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Windows zip (~30 MB)",
    .platforms = PLATFORMS_WINDOWS,
    .clone_url = "https://github.com/cxong/cdogs-sdl/releases/download/2.2.0/C-Dogs_SDL-2.2.0-Windows-x86_64.zip",
    .clone_dir = "cdogs",
    .archive_type = "zip",
    .bin = "cdogs-sdl.exe",
    .play_cmd = play_win,
}};

static const Game game_data = {
    .name = "C-Dogs SDL", .icon = "C",
    .desc = "A classic overhead run-and-gun shooter with co-op multiplayer. Fight through campaigns, use the built-in editor, and enjoy retro pixel art action.",
    .keys = keys, .category = "Shooter",
    .engine = "SDL2", .repo = "https://github.com/cxong/cdogs-sdl",
    .platforms = PLATFORMS_LINUX_WIN,
    .sources = sources, .num_sources = 2,
};

const Game *game_cdogs(void) { return &game_data; }
