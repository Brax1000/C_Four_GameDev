#ifndef DISPLAY_H
#define DISPLAY_H

#include "game.h"

typedef enum { COL_DEFAULT, COL_GREEN, COL_RED, COL_YELLOW } Color;

typedef struct {
    char guess[MAX_DIGITS + 1];
    GuessResult result;
} HistoryEntry;

int  display_init(void);         /* 1 = ANSI colors enabled, 0 = plain fallback */
void display_shutdown(void);     /* restore colors, cursor, code page; idempotent */
int  display_size_ok(void);      /* 1 if visible console is at least 80x25 */
void display_set_color(Color c);
void display_home(void);         /* cursor to top-left (ESC[H) */
void display_clear(void);
void display_flush_input(void);  /* drop any pending key presses */

/* Draws the 24-line round screen at the top-left. remaining_ms < 0 is shown as 00:00. */
void display_round(const GameConfig *cfg, long long remaining_ms, const char *input,
                   const HistoryEntry *history, int history_count, int flash);
void display_defused(long long remaining_ms, int guesses);
void display_boom(int guesses);

void display_banner(const char *word);  /* block-letter word, centered, 5 rows */
void display_center(const char *s);     /* one centered text line plus newline */
int  display_read_key(void);            /* blocking; arrow/function keys are swallowed whole and return -1 */

#endif
