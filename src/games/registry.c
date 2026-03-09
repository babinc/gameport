#include "../catalog.h"

/* Game accessor declarations */
const Game *game_minesweeper(void);
const Game *game_tetris(void);
const Game *game_chess_tui(void);
const Game *game_2048(void);
const Game *game_recwars(void);
const Game *game_space_invaders(void);
const Game *game_asteroids(void);
const Game *game_anarch(void);
const Game *game_chocolate_doom(void);
const Game *game_openttd(void);
const Game *game_supertuxkart(void);
const Game *game_warzone2100(void);
const Game *game_widelands(void);
const Game *game_srb2(void);
const Game *game_red_eclipse(void);
const Game *game_thedarkmod(void);
const Game *game_brogue(void);
const Game *game_cataclysm_dda(void);
const Game *game_nethack(void);
const Game *game_ttyper(void);
const Game *game_wesnoth(void);
const Game *game_endless_sky(void);
const Game *game_dcss(void);
const Game *game_supertux(void);
const Game *game_flare(void);
const Game *game_megaglest(void);
const Game *game_zeroad(void);
const Game *game_naev(void);
const Game *game_pioneer(void);
const Game *game_openra(void);
const Game *game_pokerth(void);
const Game *game_shattered_pd(void);
const Game *game_cdogs(void);
const Game *game_classicube(void);
const Game *game_teeworlds(void);
const Game *game_unciv(void);

typedef const Game *(*GameFn)(void);

static const GameFn game_fns[] = {
    game_minesweeper,
    game_tetris,
    game_chess_tui,
    game_2048,
    game_recwars,
    game_space_invaders,
    game_asteroids,
    game_anarch,
    game_chocolate_doom,
    game_openttd,
    game_supertuxkart,
    game_warzone2100,
    game_widelands,
    game_srb2,
    game_red_eclipse,
    game_thedarkmod,
    game_brogue,
    game_cataclysm_dda,
    game_nethack,
    game_ttyper,
    game_wesnoth,
    game_endless_sky,
    game_dcss,
    game_supertux,
    game_flare,
    game_megaglest,
    game_zeroad,
    game_naev,
    game_pioneer,
    game_openra,
    game_pokerth,
    game_shattered_pd,
    game_cdogs,
    game_classicube,
    game_teeworlds,
    game_unciv,
};

#define MAX_GAMES 64

Game GAMES[MAX_GAMES];
int NUM_GAMES;

void catalog_init(void) {
    NUM_GAMES = (int)(sizeof(game_fns) / sizeof(game_fns[0]));
    if (NUM_GAMES > MAX_GAMES) NUM_GAMES = MAX_GAMES;
    for (int i = 0; i < NUM_GAMES; i++)
        GAMES[i] = *game_fns[i]();
}
