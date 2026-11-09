#ifndef GAME_H
#define GAME_H

#define MAX_DIGITS 6

typedef enum { DIFF_NORMAL, DIFF_INTERMEDIATE, DIFF_HARD } Difficulty;

typedef struct {
    int digits;   /* code length */
    int seconds;  /* starting time */
    int penalty;  /* seconds lost per wrong guess */
} GameConfig;

typedef struct {
    int in_place;
    int misplaced;
} GuessResult;

GameConfig game_get_config(Difficulty d);
void game_generate_code(char *code, int digits);
GuessResult game_evaluate(const char *code, const char *guess, int digits);
long long game_remaining_ms(int seconds, unsigned long long start_ms,
                            unsigned long long now_ms, unsigned long long penalty_ms);

#endif
