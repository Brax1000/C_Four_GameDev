#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "display.h"
#include "menu.h"
#include "round.h"

static int ask_play_again(void) {
    display_flush_input();
    display_set_color(COL_YELLOW);
    display_center("PLAY AGAIN? (Y/N)");
    display_set_color(COL_DEFAULT);
    fflush(stdout);
    for (;;) {
        int k = display_read_key();
        if (k == 'y' || k == 'Y') return 1;
        if (k == 'n' || k == 'N') return 0;
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    display_init();

    if (!display_size_ok()) {
        display_clear();
        puts("Please enlarge the console window to at least 80x25, then press any key.");
        fflush(stdout);
        display_flush_input();
        (void)display_read_key();
    }

    do {
        Difficulty d = menu_run();
        int guesses = 0;
        long long left = 0;
        if (round_play(d, &guesses, &left) == ROUND_WON) display_defused(left, guesses);
        else display_boom(guesses);
    } while (ask_play_again());

    display_shutdown();
    return 0;
}
