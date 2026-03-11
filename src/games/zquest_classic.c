#include "../core/catalog.h"

static const char *keys[] = {
    "Arrows|Move Link",
    "A|Sword (B button)",
    "S|Use item (A button)",
    "Q|Use subscreen item",
    "Esc|Subscreen / Pause",
    "Enter|Start",
    "F5|Save game",
    "F6|Load save slot",
    "F12|Toggle fullscreen",
    NULL
};

static const char *play_linux[] = {"./bin/zlauncher", NULL};
static const char *play_win[] = {"zlauncher.exe", NULL};

static const char *fetch_quests_linux[] = {
    "bash", "-c",
    "set -e\n"
    "QUEST_DIR=\"$PWD/share/zquestclassic/quests\"\n"
    "mkdir -p \"$QUEST_DIR\"\n"
    "if [ ! -f \"$QUEST_DIR/demo_quests_downloaded\" ]; then\n"
    "  echo 'Downloading demo quest collection...'\n"
    "  dl=$(mktemp /tmp/gp_zc_quests.XXXXXX.zip)\n"
    "  trap 'rm -f \"$dl\"' EXIT\n"
    "  curl -fSL -o \"$dl\" 'https://www.purezc.net/index.php?page=download&section=Quests&id=770'\n"
    "  unzip -o \"$dl\" -d \"$QUEST_DIR\"\n"
    "  touch \"$QUEST_DIR/demo_quests_downloaded\"\n"
    "  echo 'Demo quests installed!'\n"
    "else\n"
    "  echo 'Demo quests already installed.'\n"
    "fi",
    NULL
};

static const char *fetch_quests_win[] = {
    "powershell", "-NoProfile", "-Command",
    "$ErrorActionPreference='Stop'\n"
    "$qdir = Join-Path $PWD 'share\\zquestclassic\\quests'\n"
    "New-Item -ItemType Directory -Force -Path $qdir | Out-Null\n"
    "$marker = Join-Path $qdir 'demo_quests_downloaded'\n"
    "if (-not (Test-Path $marker)) {\n"
    "  Write-Host 'Downloading demo quest collection...'\n"
    "  $dl = Join-Path $env:TEMP ('gp_zc_quests_' + [System.IO.Path]::GetRandomFileName() + '.zip')\n"
    "  curl.exe -fSL -o $dl 'https://www.purezc.net/index.php?page=download&section=Quests&id=770'\n"
    "  Expand-Archive -Path $dl -DestinationPath $qdir -Force\n"
    "  Remove-Item $dl -Force -ErrorAction SilentlyContinue\n"
    "  New-Item -ItemType File -Path $marker | Out-Null\n"
    "  Write-Host 'Demo quests installed!'\n"
    "} else {\n"
    "  Write-Host 'Demo quests already installed.'\n"
    "}",
    NULL
};

static const Source sources[] = {
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download tar.gz (~30 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/ZQuestClassic/ZQuestClassic/releases/download/2.55.12/2.55.12-linux.tar.gz",
    .dir = "zquest-classic", .archive_type = "tar.gz",
    .bin = "zplayer",
    .build_cmd = fetch_quests_linux,
    .play_cmd = play_linux,
},
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download zip (~30 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/ZQuestClassic/ZQuestClassic/releases/download/2.55.12/2.55.12-windows-x64.zip",
    .dir = "zquest-classic", .archive_type = "zip",
    .bin = "zplayer.exe",
    .build_cmd = fetch_quests_win,
    .play_cmd = play_win,
},
};

static const Game game_data = {
    .name = "ZQuest Classic", .icon = "Z",
    .desc = "Free Zelda fan game engine with 700+ community quests. "
            "Includes a demo quest pack and quest editor. "
            "Browse and download more quests at purezc.net. "
            "For the original NES Zelda experience, search PureZC for '1st Quest' or 'Classic Zelda'.",
    .keys = keys, .category = "Action",
    .engine = "Allegro", .website = "https://zquestclassic.com/",
    .repo = "https://github.com/ZQuestClassic/ZQuestClassic",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_zquest_classic(void) { return &game_data; }
