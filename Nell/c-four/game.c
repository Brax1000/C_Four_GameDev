#include <stdlib.h>
#include "game.h"

GameConfig game_get_config(Difficulty d) {
    GameConfig c;
    switch (d) {
        case DIFF_INTERMEDIATE: c.digits = 5; c.seconds = 75; c.penalty = 10; break;
        case DIFF_HARD:         c.digits = 6; c.seconds = 60; c.penalty = 15; break;
        case DIFF_NORMAL:
        default:                c.digits = 4; c.seconds = 90; c.penalty = 5;  break;
    }
    return c;
}

void game_generate_code(char *code, int digits) {
    for (int i = 0; i < digits; i++) code[i] = (char)('0' + rand() % 10);
    code[digits] = '\0';
}

GuessResult game_evaluate(const char *code, const char *guess, int digits) {
    GuessResult r = {0, 0};
    int code_count[10] = {0}, guess_count[10] = {0};
    for (int i = 0; i < digits; i++) {
        if (guess[i] == code[i]) {
            r.in_place++;
        } else {
            code_count[code[i] - '0']++;
            guess_count[guess[i] - '0']++;
        }
    }
    for (int d = 0; d < 10; d++)
        r.misplaced += code_count[d] < guess_count[d] ? code_count[d] : guess_count[d];
    return r;
}

long long game_remaining_ms(int seconds, unsigned long long start_ms,
                            unsigned long long now_ms, unsigned long long penalty_ms) {
    return (long long)seconds * 1000 - (long long)(now_ms - start_ms) - (long long)penalty_ms;
}
