#include "../catalog.h"

static const char *keys[] = {
    "Arrows / WASD|Slide tiles",
    "Q|Quit",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./2048", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", NULL};
static const char *linux_check[] = {"bash", "-c",
    "command -v g++ >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential", linux_install, linux_check, 1 },
    { "macos", "Xcode CLI tools", MAC_XCODE_INSTALL, MAC_XCODE_CHECK, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + make)",
    .url = "https://github.com/plibither8/2048.cpp.git",
    .dir = "2048cpp", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "2048",
}};

static const Game game_data = {
    .name = "2048", .icon = "2",
    .desc = "Slide all tiles in one direction. Matching numbers merge and double. Keep merging to reach 2048. Colorized terminal version with score tracking.",
    .keys = keys, .category = "Puzzle",
    .engine = "ncurses", .website = "https://github.com/plibither8/2048.cpp",
    .repo = "https://github.com/plibither8/2048.cpp",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_2048(void) { return &game_data; }
