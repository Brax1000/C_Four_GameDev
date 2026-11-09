#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../display.h"
#include "../round.h"

int main(void) {
    int guesses = 0;
    long long left = 0;
    RoundOutcome o;
    srand((unsigned)time(NULL));
    display_init();
    o = round_play(DIFF_NORMAL, &guesses, &left);
    printf("\noutcome=%d guesses=%d time_left_ms=%lld\n", (int)o, guesses, left);
    return 0;
}
