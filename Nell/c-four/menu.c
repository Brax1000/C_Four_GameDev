#include <stdio.h>
#include "display.h"
#include "menu.h"

static void draw_menu(void) {
    static const char *names[3] = {"NORMAL", "INTERMEDIATE", "HARD"};
    char line[100];

    display_clear();
    display_set_color(COL_GREEN);
    fputs("\n", stdout);
    display_banner("C-FOUR");
    fputs("\n", stdout);
    display_center("BOMB DEFUSAL SIMULATOR");
    fputs("\n", stdout);
    display_set_color(COL_YELLOW);
    display_center("SELECT DIFFICULTY");
    fputs("\n", stdout);
    display_set_color(COL_GREEN);
    for (int i = 0; i < 3; i++) {
        GameConfig c = game_get_config((Difficulty)i);
        snprintf(line, sizeof line, "[%d] %-13s %d-DIGIT CODE  %3d SEC  -%d SEC PER MISS",
                 i + 1, names[i], c.digits, c.seconds, c.penalty);
        display_center(line);
    }
    fputs("\n", stdout);
    display_set_color(COL_YELLOW);
    display_center("PRESS 1, 2 OR 3");
    display_set_color(COL_DEFAULT);
    fflush(stdout);
}

Difficulty menu_run(void) {
    display_flush_input();
    draw_menu();
    for (;;) {
        int k = display_read_key();
        if (k >= '1' && k <= '3') return (Difficulty)(k - '1');
    }
}
