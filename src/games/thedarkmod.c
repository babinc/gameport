#include "../catalog.h"

static const char *keys[] = {
    "W / A / S / D|Move",
    "Space|Jump / mantle",
    "X|Crouch",
    "Shift|Run",
    "Ctrl|Creep (slow walk)",
    "Q / E|Lean left / right",
    "F|Lean forward",
    "LMB|Attack",
    "RMB|Frob / interact",
    "Z / MMB|Parry / manipulate",
    "1-0|Select weapon",
    "Mousewheel|Next/prev weapon",
    "` (backtick)|Put away weapon",
    "L|Toggle lantern",
    "G|Spyglass",
    "V|Compass",
    "O|Objectives",
    "M|Cycle maps",
    "P|Cycle lockpicks",
    "[ / ]|Next/prev inventory",
    "Enter / U|Use item",
    "R|Drop item",
    "F4|Quick save",
    "F9|Quick load",
    "Esc|Menu",
    NULL
};

static const char *platforms[] = {"linux", NULL};

static const char *build[] = {
    "bash", "-c",
    "set -e\n"
    "echo 'Downloading game assets via TDM installer...'\n"
    "if [ ! -f ../darkmod/tdm_installer_linux.zip ]; then\n"
    "  mkdir -p ../darkmod\n"
    "  curl -L -o ../darkmod/tdm_installer_linux.zip 'https://www.thedarkmod.com/downloads/tdm_installer_linux.zip'\n"
    "  cd ../darkmod && unzip -o tdm_installer_linux.zip && chmod +x tdm_installer\n"
    "  ./tdm_installer --keep-installer\n"
    "  cd -\n"
    "fi\n"
    "echo 'Fetching third-party libs...'\n"
    "if [ ! -d ThirdParty/artefacts ]; then\n"
    "  svn checkout https://svn.thedarkmod.com/publicsvn/darkmod_src/trunk/ThirdParty/artefacts/ ThirdParty/artefacts\n"
    "fi\n"
    "mkdir -p build && cd build\n"
    "echo 'Configuring cmake...'\n"
    "cmake -DCMAKE_BUILD_TYPE=Release ..\n"
    "echo 'Building...'\n"
    "make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)\n"
    "cp thedarkmod.x64 ../../darkmod/ 2>/dev/null || true",
    NULL
};
static const char *play[] = {"../darkmod/thedarkmod.x64", NULL};

static const char *linux_install[] = {"sudo", "apt", "install", "-y",
    "build-essential", "cmake", "subversion", "mesa-common-dev",
    "libxxf86vm-dev", "libopenal-dev", "libxext-dev",
    "libx11-dev", "unzip", "curl", NULL};
static const char *linux_check[] = {"bash", "-c",
    "dpkg -s cmake subversion mesa-common-dev libxxf86vm-dev libopenal-dev libxext-dev >/dev/null 2>&1", NULL};

static const PlatformDeps deps[] = {
    { "linux", "build-essential cmake subversion mesa-common-dev libopenal-dev ...",
      linux_install, linux_check, 1 },
};

static const Source sources[] = {{
    .method = ACQUIRE_GIT, .label = "Build from source (git + cmake)",
    .url = "https://github.com/stgatilov/darkmod_src.git",
    .dir = "darkmod_src", .shallow = 1,
    .build_cmd = build, .play_cmd = play,
    .bin = "thedarkmod.x64",
}};

static const Game game_data = {
    .name = "The Dark Mod", .icon = "M",
    .desc = "Free standalone stealth game inspired by Thief. Sneak through shadows, pickpocket, lockpick, and explore 170+ fan-made missions. Large download (~4GB assets).",
    .keys = keys, .category = "Stealth",
    .engine = "id Tech 4 (modified)", .website = "https://www.thedarkmod.com/",
    .repo = "https://github.com/stgatilov/darkmod_src",
    .platforms = platforms, .platform_deps = deps, .num_platform_deps = 1,
    .sources = sources, .num_sources = 1,
};

const Game *game_thedarkmod(void) { return &game_data; }
