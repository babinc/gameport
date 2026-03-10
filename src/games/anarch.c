#include "../catalog.h"

static const char *keys[] = {
    "W / Up|Move forward",
    "S / Down|Move backward",
    "A|Strafe left",
    "D|Strafe right",
    "Q|Turn left",
    "E|Turn right",
    "J / Ctrl / LMB|Shoot",
    "K / Shift|Strafe modifier",
    "Space|Jump",
    "F / MMB|Cycle weapon",
    "P / X / Wheel Up|Next weapon",
    "O / Y / Wheel Down|Prev weapon",
    "Tab|Toggle map",
    "RMB|Free look toggle",
    "Esc|Menu",
    NULL
};


static const char *build[] = {"bash", "make.sh", "sdl", NULL};
static const char *play[] = {"./anarch", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y", "libsdl2-dev", "g++", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev g++ >/dev/null 2>&1", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev g++", linux_install, linux_check, 1 },
    { "macos", "SDL2", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + gcc)",
    .url = "https://gitlab.com/drummyfish/anarch.git",
    .dir = "anarch", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "anarch",
}};

static const Game game_data = {
    .name = "Anarch", .icon = "F",
    .desc = "Retro first-person shooter inspired by Doom. 10 levels, multiple weapons, all assets embedded in source code. Tiny, fast, zero external files needed.",
    .keys = keys, .category = "Action",
    .engine = "SDL2", .website = "https://drummyfish.gitlab.io/anarch/",
    .repo = "https://gitlab.com/drummyfish/anarch",
    .platforms = PLAT_LINUX | PLAT_MACOS, .platform_deps = deps, .num_platform_deps = 2,
    .sources = sources, .num_sources = 1,
};

const Game *game_anarch(void) { return &game_data; }
