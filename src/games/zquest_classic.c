#include "../catalog.h"

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

static const char *play_linux[] = {"./bin/zplayer", NULL};
static const char *play_win[] = {"zplayer.exe", NULL};
static const char *play_mac[] = {"open", "ZQuest Classic.app", NULL};

static const Source sources[] = {
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download tar.gz (~30 MB)",
    .platforms = PLAT_LINUX,
    .url = "https://github.com/ZQuestClassic/ZQuestClassic/releases/download/2.55.12/2.55.12-linux.tar.gz",
    .dir = "zquest-classic", .archive_type = "tar.gz",
    .bin = "zplayer",
    .play_cmd = play_linux,
},
{
    .method = ACQUIRE_DOWNLOAD,
    .label = "Download zip (~30 MB)",
    .platforms = PLAT_WINDOWS,
    .url = "https://github.com/ZQuestClassic/ZQuestClassic/releases/download/2.55.12/2.55.12-windows-x64.zip",
    .dir = "zquest-classic", .archive_type = "zip",
    .bin = "zplayer.exe",
    .play_cmd = play_win,
},
};

static const Game game_data = {
    .name = "ZQuest Classic", .icon = "Z",
    .desc = "Free Zelda fan game engine with a faithful recreation of the original NES Legend of Zelda plus hundreds of community-made quests. Includes a quest editor.",
    .keys = keys, .category = "Action",
    .engine = "Allegro", .website = "https://zquestclassic.com/",
    .repo = "https://github.com/ZQuestClassic/ZQuestClassic",
    .platforms = PLAT_LINUX | PLAT_WINDOWS,
    .sources = sources, .num_sources = 2,
};

const Game *game_zquest_classic(void) { return &game_data; }
