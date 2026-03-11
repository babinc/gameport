#include "../core/catalog.h"

static const char *keys[] = {
    "RMB / Arrows|Scroll map",
    "Ctrl+ +/-|Zoom in/out",
    "Space|Toggle build help",
    "M|Minimap",
    "Home|Scroll to HQ",
    "C|Show/hide census",
    "S|Show/hide statistics",
    "L|Show/hide soldiers",
    "N|Messages",
    "T|Objectives",
    "D|Diplomacy",
    "1-9|Go to landmark",
    "Ctrl+1-9|Set landmark",
    "Page Up/Down|Speed up/down",
    "Pause|Pause game",
    "Ctrl+S|Save",
    "Ctrl+Q|Exit",
    "F1|Encyclopedia",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake -DCMAKE_BUILD_TYPE=Release ..\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./build/src/widelands", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "g++", "libglew-dev", "libpng-dev",
    "libsdl2-dev", "libsdl2-image-dev", "libsdl2-mixer-dev",
    "libsdl2-ttf-dev", "python3", "zlib1g-dev", "libminizip-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libglew-dev libsdl2-ttf-dev libsdl2-image-dev libminizip-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "cmake", "glew",
    "sdl2", "sdl2_image", "sdl2_mixer", "sdl2_ttf", "libpng",
    "minizip", "ninja", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2_ttf", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libglew-dev libsdl2-dev libsdl2-ttf-dev ...",
      linux_install, linux_check, 1 },
    { "macos", "cmake glew SDL2 SDL2_ttf SDL2_mixer ...",
      mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .url = "https://github.com/widelands/widelands.git",
    .dir = "widelands", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "widelands",
}};

static const Game game_data = {
    .name = "Widelands", .icon = "L",
    .desc = "Free open-source colony-building strategy game inspired by Settlers II. Grow settlements, manage resources, and conquer territory. All assets included.",
    .keys = keys, .category = "Strategy",
    .engine = "SDL2 (OpenGL)", .website = "https://www.widelands.org/",
    .repo = "https://github.com/widelands/widelands",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_widelands(void) { return &game_data; }
