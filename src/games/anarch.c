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

static const char *platforms[] = {"linux", "windows", "macos", NULL};
static const char *build[] = {"bash", "make.sh", "sdl", NULL};
static const char *play[] = {"./anarch", NULL};
static const char *uninstall[] = {"git-game-remove", "anarch", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y", "libsdl2-dev", "g++", NULL};
static const char *linux_check[] = {"dpkg", "-s", "libsdl2-dev", NULL};
static const char *mac_install[] = {"brew", "install", "sdl2", NULL};
static const char *mac_check[] = {"brew", "list", "sdl2", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev g++", linux_install, linux_check, 1 },
    { "windows", "SDL2, MinGW or MSVC", NULL, NULL, 0 },
    { "macos", "SDL2", mac_install, mac_check, 0 },
};

static const Source sources[] = {{
    METHOD_GIT, "Build from source (git + gcc)",
    "https://gitlab.com/drummyfish/anarch.git", "anarch", 1,
    build, play, "anarch", uninstall,
}};

static const Game game_data = {
    "Anarch", "F",
    "Retro first-person shooter inspired by Doom. 10 levels, multiple weapons, all assets embedded in source code. Tiny, fast, zero external files needed.",
    keys, "Action",
    "SDL2", "https://gitlab.com/drummyfish/anarch",
    platforms, deps, 3, sources, 1,
};

const Game *game_anarch(void) { return &game_data; }
