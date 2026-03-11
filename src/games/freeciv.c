#include "../core/catalog.h"

static const char *keys[] = {
    "Arrow keys|Move units",
    "G|Go to",
    "S|Sentry",
    "F|Fortify",
    "B|Build city",
    "R|Build road",
    "I|Build irrigation",
    "N|Next unit",
    "Enter|End turn",
    "Esc|Close dialog",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Configuring with meson...'\n"
    "meson setup build --buildtype=release\n"
    "echo 'Building (this may take a while)...'\n"
    "meson compile -C build\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./build/freeciv-gtk4", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "meson", "libsdl2-mixer-dev",
    "libgtk-4-dev", "libcurl4-openssl-dev", "libbz2-dev", "zlib1g-dev",
    "libreadline-dev", "liblzma-dev", "liblua5.4-dev", "gettext",
    "libsqlite3-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s meson libgtk-4-dev liblua5.4-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "meson libgtk-4-dev libsdl2-mixer-dev liblua5.4-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + meson)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/freeciv/freeciv.git",
    .dir = "freeciv", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "freeciv-gtk4",
}};

static const Game game_data = {
    .name = "FreeCiv", .icon = "C",
    .desc = "Free, open-source empire-building strategy game inspired by the history of human civilization. Turn-based, multiplayer-capable Civilization clone.",
    .keys = keys, .category = "Strategy",
    .engine = "GTK4 / SDL2", .website = "https://www.freeciv.org/",
    .repo = "https://github.com/freeciv/freeciv",
    .platforms = PLAT_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_freeciv(void) { return &game_data; }
