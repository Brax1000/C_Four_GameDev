#include <string.h>
#include <conio.h>
#include <windows.h>
#include "display.h"
#include "game.h"
#include "round.h"

#define HISTORY_CAP 32      /* newest guesses kept; at most 18 wrong guesses fit in any round */
#define FLASH_MS 400ULL

RoundOutcome round_play(Difficulty d, int *guesses_used, long long *time_left_ms) {
    GameConfig cfg = game_get_config(d);
    char code[MAX_DIGITS + 1];
    char input[MAX_DIGITS + 1] = "";
    HistoryEntry history[HISTORY_CAP];
    int hist_n = 0, len = 0, guesses = 0, last_flash = 0, dirty = 1;
    long long remaining = 0, last_secs = -1;
    unsigned long long start, penalty_ms = 0, flash_until = 0;
    RoundOutcome outcome = ROUND_LOST;

    game_generate_code(code, cfg.digits);
    display_flush_input();
    display_clear();
    start = GetTickCount64();

    for (;;) {
        unsigned long long now = GetTickCount64();
        int flash, finished = 0;
        long long secs;

        remaining = game_remaining_ms(cfg.seconds, start, now, penalty_ms);
        if (remaining <= 0) { outcome = ROUND_LOST; break; }

        while (!finished && _kbhit()) {
            int k = display_read_key();
            if (k >= '0' && k <= '9') {
                if (len < cfg.digits) { input[len++] = (char)k; input[len] = '\0'; dirty = 1; }
            } else if (k == 8) {
                if (len > 0) { input[--len] = '\0'; dirty = 1; }
            } else if (k == 13 && len == cfg.digits) {
                GuessResult r = game_evaluate(code, input, cfg.digits);
                guesses++;
                if (r.in_place == cfg.digits) { outcome = ROUND_WON; finished = 1; break; }
                if (hist_n == HISTORY_CAP) {
                    memmove(history, history + 1, sizeof history[0] * (HISTORY_CAP - 1));
                    hist_n--;
                }
                strcpy(history[hist_n].guess, input);
                history[hist_n].result = r;
                hist_n++;
                penalty_ms += (unsigned long long)cfg.penalty * 1000ULL;
                flash_until = now + FLASH_MS;
                len = 0;
                input[0] = '\0';
                dirty = 1;
                remaining = game_remaining_ms(cfg.seconds, start, now, penalty_ms);
                if (remaining <= 0) { outcome = ROUND_LOST; finished = 1; }
            }
        }
        if (finished) break;

        flash = now < flash_until;
        secs = (remaining + 999) / 1000;
        if (dirty || secs != last_secs || flash != last_flash) {
            display_round(&cfg, remaining, input, history, hist_n, flash);
            dirty = 0;
            last_secs = secs;
            last_flash = flash;
        }
        Sleep(50);
    }

    if (outcome == ROUND_LOST) remaining = 0;
    *guesses_used = guesses;
    *time_left_ms = remaining;
    return outcome;
}
