#include <stdio.h>
#include "../display.h"
#include "../menu.h"

int main(void) {
    Difficulty d;
    display_init();
    d = menu_run();
    printf("\nchosen difficulty value: %d\n", (int)d);
    return 0;
}
