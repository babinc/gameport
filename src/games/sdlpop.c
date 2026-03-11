#include "../core/catalog.h"

static const char *keys[] = {
    "Arrows|Move / jump / crouch",
    "Shift|Pick up / strike",
    "Up|Defend (combat)",
    "Home/PgUp|Diagonal jumps",
    "Esc|Pause",
    "Ctrl+A|Restart level",
    "F6|Quick save",
    "F9|Quick load",
    "Alt+Enter|Toggle fullscreen",
    "Ctrl+Q|Quit",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e && make all -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};

static const char *play[] = {"./prince", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "libsdl2-image-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-image-dev >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2_image", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2_image", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-image-dev", linux_install, linux_check, 1 },
    { "macos", "SDL2_image", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + make)",
    .url = "https://github.com/NagyD/SDLPoP.git",
    .dir = "sdlpop", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "prince",
}};

static const Game game_data = {
    .name = "Prince of Persia", .icon = "P",
    .desc = "Open-source reimplementation of the classic 1989 Prince of Persia. "
            "Run, jump, and swordfight through deadly traps and palace guards. "
            "Includes all original levels.",
    .keys = keys, .category = "Platformer",
    .engine = "SDL2", .website = "https://github.com/NagyD/SDLPoP",
    .repo = "https://github.com/NagyD/SDLPoP",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_sdlpop(void) { return &game_data; }
