#include "../catalog.h"

static const char *keys[] = {
    "Mouse|Fly toward cursor / interact",
    "Up / W|Fire primary weapons",
    "Shift|Fire secondary weapons",
    "Tab|Select nearest hostile",
    "T|Talk to / board target",
    "L|Land on planet",
    "J|Jump to next system",
    "H|Hyperspace (selected system)",
    "M|Map view",
    "I|Player info / fleet",
    "P|Pause",
    "Esc|Menu",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Configuring cmake...'\n"
    "cmake -B build -DCMAKE_BUILD_TYPE=Release\n"
    "echo 'Building (this may take a while)...'\n"
    "cmake --build build --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./build/endless-sky", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "ninja-build",
    "libsdl2-dev", "libpng-dev", "libjpeg-dev",
    "libgl1-mesa-dev", "libglew-dev", "libopenal-dev",
    "libmad0-dev", "uuid-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libglew-dev libsdl2-dev libopenal-dev libmad0-dev cmake >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "cmake",
    "sdl2", "libpng", "jpeg", "glew", "openal-soft", "mad", NULL};
static const char *mac_check[] = {"brew", "list", "glew", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake libsdl2-dev libglew-dev libopenal-dev ...", linux_install, linux_check, 1 },
    { "macos", "cmake SDL2 libpng glew openal-soft ...", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (cmake + SDL2 + OpenGL)",
    .clone_url = "https://github.com/endless-sky/endless-sky.git",
    .clone_dir = "endless-sky", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "endless-sky",
}};

static const Game game_data = {
    .name = "Endless Sky", .icon = "E",
    .desc = "Open-world space exploration, trading, and combat game. Start with a tiny shuttle, trade cargo, take on missions, upgrade your ship, and build a fleet. All assets included.",
    .keys = keys, .category = "Simulation",
    .engine = "SDL2 (OpenGL)", .website = "https://endless-sky.github.io/",
    .repo = "https://github.com/endless-sky/endless-sky",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_endless_sky(void) { return &game_data; }
