#include "../catalog.h"

static const char *keys[] = {
    "Arrow keys|Navigate / steer ship",
    "Space|Fire",
    "Enter|Special weapon",
    "Esc|Menu",
    NULL
};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Downloading UQM source + content...'\n"
    "dl=$(mktemp)\n"
    "curl -fSL -o \"$dl\" 'https://sourceforge.net/projects/sc2/files/UQM/0.8/uqm-0.8.0-src.tgz/download'\n"
    "tar xzf \"$dl\" --strip-components=1\n"
    "rm -f \"$dl\"\n"
    "mkdir -p content/packages\n"
    "for p in uqm-0.8.0-content.uqm uqm-0.8.0-voice.uqm uqm-0.8.0-3domusic.uqm; do\n"
    "  echo \"Downloading $p...\"\n"
    "  curl -fSL -o \"content/packages/$p\" \"https://sourceforge.net/projects/sc2/files/UQM/0.8/$p/download\"\n"
    "done\n"
    "echo 'Building...'\n"
    "./build.sh uqm\n"
    "echo 'Done!'",
    NULL
};
static const char *play[] = {"./uqm", NULL};
static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "libsdl2-dev", "libsdl2-image-dev",
    "libogg-dev", "libvorbis-dev", "zlib1g-dev", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-dev libvorbis-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "libsdl2-dev libsdl2-image-dev libvorbis-dev ...", linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Download + build from source",
    .platforms = PLATFORMS_LINUX,
    .url = "https://github.com/sc2/uqm.git",
    .dir = "uqm", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "uqm",
}};

static const Game game_data = {
    .name = "The Ur-Quan Masters", .icon = "U",
    .desc = "Open-source port of Star Control 2. Explore the galaxy, form alliances, and battle alien species in this classic sci-fi space adventure.",
    .keys = keys, .category = "RPG",
    .engine = "SDL2 (custom C)", .website = "https://sc2.sourceforge.net/",
    .repo = "https://sourceforge.net/projects/sc2/",
    .platforms = PLATFORMS_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_uqm(void) { return &game_data; }
