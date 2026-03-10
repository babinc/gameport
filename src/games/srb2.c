#include "../catalog.h"

static const char *keys[] = {
    "W / A / S / D|Move",
    "Space|Jump",
    "Left Shift|Spin",
    "Left/Right Arrow|Turn",
    "Up/Down Arrow|Look up/down",
    "Left Ctrl|Center view",
    "Right Ctrl / LMB|Fire (ring toss)",
    "1-0|Weapon slots",
    "Mousewheel|Next/prev weapon",
    "V|Toggle 3rd person",
    "T|Chat",
    "Tab|Scores",
    "P|Pause",
    "Esc|Menu",
    NULL
};


static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Building SRB2...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "echo 'Downloading game assets...'\n"
    "ASSETS_DIR=\"$HOME/.srb2\"\n"
    "mkdir -p \"$ASSETS_DIR\"\n"
    "if [ ! -f \"$ASSETS_DIR/srb2.pk3\" ]; then\n"
    "  echo 'Downloading srb2.pk3 and other assets...'\n"
    "  BASE_URL='https://github.com/STJr/SRB2/releases/download/SRB2_release_2.2.15'\n"
    "  curl -L -o /tmp/srb2-full.zip \"$BASE_URL/SRB2-v2215-Full.zip\"\n"
    "  unzip -o /tmp/srb2-full.zip -d /tmp/srb2-assets\n"
    "  cp /tmp/srb2-assets/*.pk3 \"$ASSETS_DIR/\" 2>/dev/null || true\n"
    "  cp /tmp/srb2-assets/*.dat \"$ASSETS_DIR/\" 2>/dev/null || true\n"
    "  cp -r /tmp/srb2-assets/models \"$ASSETS_DIR/\" 2>/dev/null || true\n"
    "  rm -rf /tmp/srb2-full.zip /tmp/srb2-assets\n"
    "  echo 'Assets installed!'\n"
    "else\n"
    "  echo 'Assets already downloaded.'\n"
    "fi",
    NULL
};
static const char *play[] = {"./bin/lsdl2srb2", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "libsdl2-dev", "libsdl2-mixer-dev",
    "libpng-dev", "libcurl4-openssl-dev", "libgme-dev",
    "libopenmpt-dev", "libminiupnpc-dev", "unzip", "curl", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s libsdl2-mixer-dev libgme-dev libopenmpt-dev libminiupnpc-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential libsdl2-dev libsdl2-mixer-dev libpng-dev ...",
      linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + make)",
    .url = "https://github.com/STJr/SRB2.git",
    .dir = "SRB2", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "lsdl2srb2",
}};

static const Game game_data = {
    .name = "Sonic Robo Blast 2", .icon = "B",
    .desc = "Fan-made 3D Sonic the Hedgehog platformer built on Doom engine. Singleplayer campaign, multiplayer modes, and mod support. Free game assets downloaded automatically.",
    .keys = keys, .category = "Platformer",
    .engine = "Doom (SDL2)", .website = "https://www.srb2.org/",
    .repo = "https://github.com/STJr/SRB2",
    .platforms = PLATFORMS_LINUX, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_srb2(void) { return &game_data; }
