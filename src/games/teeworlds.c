#include "../catalog.h"

static const char *keys[] = {
    "WASD|Move",
    "LMB|Fire",
    "RMB|Hook / grapple",
    "Space|Jump",
    "1-5|Switch weapon",
    "Tab|Scoreboard",
    "T|Chat",
    "F1|Console",
    "Esc|Menu",
    NULL
};

static const char *play[] = {"./teeworlds", NULL};
static const char *play_win[] = {"teeworlds.exe", NULL};

static const Source sources[] = {{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download tar.gz (~10 MB)",
    .platforms = PLATFORMS_LINUX,
    .clone_url = "https://github.com/teeworlds/teeworlds/releases/download/0.7.5/teeworlds-0.7.5-linux_x86_64.tar.gz",
    .clone_dir = "teeworlds",
    .archive_type = "tar.gz",
    .bin = "teeworlds",
    .play_cmd = play,
}, {
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download Windows zip (~10 MB)",
    .platforms = PLATFORMS_WINDOWS,
    .clone_url = "https://github.com/teeworlds/teeworlds/releases/download/0.7.5/teeworlds-0.7.5-win64.zip",
    .clone_dir = "teeworlds",
    .archive_type = "zip",
    .bin = "teeworlds.exe",
    .play_cmd = play_win,
}};

static const Game game_data = {
    .name = "Teeworlds", .icon = "T",
    .desc = "A retro multiplayer 2D shooter with grappling hook mechanics. Fast-paced online matches with custom maps and an active competitive community.",
    .keys = keys, .category = "Shooter",
    .engine = "SDL2 (OpenGL)", .repo = "https://github.com/teeworlds/teeworlds",
    .platforms = PLATFORMS_LINUX_WIN,
    .sources = sources, .num_sources = 2,
};

const Game *game_teeworlds(void) { return &game_data; }
