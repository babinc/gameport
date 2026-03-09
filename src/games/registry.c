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
};

#define MAX_GAMES 32

Game GAMES[MAX_GAMES];
int NUM_GAMES;

void catalog_init(void) {
    NUM_GAMES = (int)(sizeof(game_fns) / sizeof(game_fns[0]));
    if (NUM_GAMES > MAX_GAMES) NUM_GAMES = MAX_GAMES;
    for (int i = 0; i < NUM_GAMES; i++)
        GAMES[i] = *game_fns[i]();
}
