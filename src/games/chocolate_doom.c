#include "../catalog.h"

static const char *keys[] = {
    "Up / W|Move forward",
    "Down / S|Move backward",
    "Left / Right|Turn",
    "Alt + Left/Right|Strafe",
    "Ctrl|Fire",
    "Space|Use / open door",
    "Shift|Run",
    "1-7|Select weapon",
    "Tab|Toggle automap",
    "-/+|Zoom automap",
    "F|Follow mode (automap)",
    "Esc|Menu",
    "F2|Save game",
    "F3|Load game",
    "F6|Quick save",
    "F9|Quick load",
    NULL
};

static const char *platforms[] = {"linux", "macos", NULL};
static const char *build[] = {
    "bash", "-c",
    "set -e && "
    "mkdir -p build && cd build && "
    "cmake .. -DCMAKE_BUILD_TYPE=Release && "
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) && "
    "cd .. && "
    "echo 'Downloading Freedoom WADs...' && "
    "mkdir -p wads && "
    "curl -L -o /tmp/freedoom.zip https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip && "
    "unzip -o /tmp/freedoom.zip -d /tmp/freedoom && "
    "cp /tmp/freedoom/freedoom-0.13.0/*.wad wads/ && "
    "rm -rf /tmp/freedoom /tmp/freedoom.zip && "
    "echo 'Freedoom WADs installed!'",
    NULL
};
static const char *play[] = {"./build/src/chocolate-doom", "-iwad", "./wads/freedoom2.wad", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "libsdl2-dev", "libsdl2-mixer-dev",
    "libsdl2-net-dev", "libpng-dev", "unzip", "curl", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libsdl2-net-dev", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", "sdl2_mixer", "sdl2_net", "libpng", "libsamplerate", "fluid-synth", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2_net", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libpng-dev", linux_install, linux_check, 1 },
    { "macos", "SDL2 SDL2_mixer SDL2_net libpng", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .clone_url = "https://github.com/chocolate-doom/chocolate-doom.git",
    .clone_dir = "chocolate-doom", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "chocolate-doom",
}};

static const Game game_data = {
    .name = "Chocolate Doom", .icon = "D",
    .desc = "Faithful recreation of the original Doom engine. Plays just like the 1993 classic. Bundled with Freedoom -- free community-made levels and assets.",
    .keys = keys, .category = "Shooter",
    .engine = "SDL2", .repo = "https://github.com/chocolate-doom/chocolate-doom",
    .platforms = platforms, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_chocolate_doom(void) { return &game_data; }
