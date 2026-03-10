#include "../catalog.h"

static const char *keys[] = {
    "W / A / S / D|Move",
    "Space|Jump",
    "Left Shift|Crouch",
    "Left Ctrl / Q|Special ability",
    "LMB|Primary fire",
    "RMB|Secondary fire",
    "R / MMB|Reload",
    "E|Use / interact",
    "1-0|Weapon slots",
    "Mousewheel|Scroll weapons",
    "Tab|Scoreboard",
    "T / Enter|Chat",
    "Y|Team chat",
    "F9|Toggle 3rd person",
    "` / /|Console",
    "Esc|Menu",
    NULL
};

static const char *platforms[] = {"linux", NULL};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Building Red Eclipse...'\n"
    "make -C src install -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)",
    NULL
};
static const char *play[] = {"./redeclipse.sh", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "libsdl2-dev", "libsdl2-image-dev",
    "libsdl2-mixer-dev", "libopenal-dev", "libsndfile-dev",
    "zlib1g-dev", "libgl-dev", "libx11-dev", "libfreetype-dev",
    "pkg-config", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-image-dev libsdl2-mixer-dev libopenal-dev libsndfile-dev libgl-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential libsdl2-dev libsdl2-image-dev libopenal-dev ...",
      linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + make)",
    .clone_url = "https://github.com/redeclipse/base.git",
    .clone_dir = "red-eclipse",
    .build_cmd = build, .play_cmd = play,
    .bin = "redeclipse.sh",
}};

static const Game game_data = {
    .name = "Red Eclipse", .icon = "R",
    .desc = "Free open-source arena FPS with parkour movement, multiple game modes, and built-in map editor. Features impulse boosts, wallrunning, and diverse weapons.",
    .keys = keys, .category = "Shooter",
    .engine = "Cube 2 (SDL2)", .website = "https://www.redeclipse.net/",
    .repo = "https://github.com/redeclipse/base",
    .platforms = platforms, .platform_deps = deps, .num_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_red_eclipse(void) { return &game_data; }
