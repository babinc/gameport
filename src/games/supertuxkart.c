#include "../catalog.h"

static const char *keys[] = {
    "Up|Accelerate",
    "Down|Brake / reverse",
    "Left / Right|Steer",
    "Space|Fire (use item)",
    "N|Nitro boost",
    "V|Drift / skid",
    "B|Look back",
    "Backspace|Rescue (reset kart)",
    "Esc|Pause / menu",
    "Enter|Menu select",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Downloading assets (SVN, may take a while)...'\n"
    "if [ ! -d ../stk-assets ]; then\n"
    "  svn co https://svn.code.sf.net/p/supertuxkart/code/stk-assets ../stk-assets\n"
    "else\n"
    "  echo 'Assets already downloaded.'\n"
    "fi\n"
    "mkdir -p cmake_build && cd cmake_build\n"
    "echo 'Configuring cmake...'\n"
    "cmake ..\n"
    "echo 'Building (this may take a while)...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./cmake_build/bin/supertuxkart", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "subversion",
    "libsdl2-dev", "libcurl4-openssl-dev", "libenet-dev",
    "libfreetype6-dev", "libharfbuzz-dev", "libjpeg-dev",
    "libogg-dev", "libopenal-dev", "libpng-dev", "libssl-dev",
    "libvorbis-dev", "pkg-config", "zlib1g-dev", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libsdl2-dev", NULL};
static const char *mac_install[] = {"brew", "install", "cmake", "subversion",
    "sdl2", "curl", "freetype", "harfbuzz", "libogg", "openal-soft",
    "libpng", "libvorbis", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake subversion libsdl2-dev libcurl4-openssl-dev ...",
      linux_install, linux_check, 1 },
    { "macos", "cmake subversion SDL2 freetype harfbuzz ...",
      mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .clone_url = "https://github.com/supertuxkart/stk-code.git",
    .clone_dir = "stk-code",
    .build_cmd = build, .play_cmd = play,
    .bin = "supertuxkart",
}};

static const Game game_data = {
    .name = "SuperTuxKart", .icon = "S",
    .desc = "Free open-source kart racer. Race as Tux and friends on 30+ tracks with powerups, nitro, and multiplayer. All assets included -- fully free.",
    .keys = keys, .category = "Racing",
    .engine = "custom (Irrlicht)", .repo = "https://github.com/supertuxkart/stk-code",
    .platforms = PLATFORMS_POSIX, .platform_deps = deps, .num_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_supertuxkart(void) { return &game_data; }
