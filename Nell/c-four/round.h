#ifndef ROUND_H
#define ROUND_H

#include "game.h"

typedef enum { ROUND_WON, ROUND_LOST } RoundOutcome;

/* Plays one live round. Sets *guesses_used and *time_left_ms (0 when lost). */
RoundOutcome round_play(Difficulty d, int *guesses_used, long long *time_left_ms);

#endif
