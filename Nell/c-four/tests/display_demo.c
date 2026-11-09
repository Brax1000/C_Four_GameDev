#include <stdio.h>
#include <conio.h>
#include "../display.h"

static void pause_key(void) { (void)_getch(); }

int main(void) {
    int color = display_init();
    GameConfig cfg = game_get_config(DIFF_NORMAL);
    HistoryEntry h[12];

    printf("color enabled: %d\n", color);
    display_set_color(COL_GREEN);   printf("green line\n");
    display_set_color(COL_RED);     printf("red line\n");
    display_set_color(COL_YELLOW);  printf("yellow line\n");
    display_set_color(COL_DEFAULT); printf("default line\n");
    printf("size ok (80x25): %d\n", display_size_ok());
    printf("press a key through each screen; shutdown runs via atexit\n");
    pause_key();

    for (int i = 0; i < 12; i++) {
        snprintf(h[i].guess, sizeof h[i].guess, "%04d", 1000 + i * 37);
        h[i].result.in_place = i % 4;
        h[i].result.misplaced = (i * 3) % 5;
    }
    display_clear();
    display_round(&cfg, 90000, "", h, 0, 0);   pause_key();   /* fresh round */
    display_round(&cfg, 9000, "48", h, 12, 0); pause_key();   /* red timer, only last 8 history rows */
    display_round(&cfg, -2000, "4821", h, 12, 0); pause_key(); /* must show 00:00 */
    display_round(&cfg, 45000, "48", h, 3, 1); pause_key();   /* flash frame */
    display_defused(23000, 5);                 pause_key();
    display_boom(7);                           pause_key();
    return 0;
}
