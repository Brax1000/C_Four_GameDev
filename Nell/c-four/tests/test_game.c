#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game.h"

static void test_config_presets(void) {
    GameConfig n = game_get_config(DIFF_NORMAL);
    assert(n.digits == 4 && n.seconds == 90 && n.penalty == 5);
    GameConfig i = game_get_config(DIFF_INTERMEDIATE);
    assert(i.digits == 5 && i.seconds == 75 && i.penalty == 10);
    GameConfig h = game_get_config(DIFF_HARD);
    assert(h.digits == 6 && h.seconds == 60 && h.penalty == 15);
}

static void test_generate_code_shape(void) {
    char code[MAX_DIGITS + 1], first[MAX_DIGITS + 1] = "";
    int differs = 0;
    srand(12345);
    for (int n = 4; n <= 6; n++) {
        for (int k = 0; k < 500; k++) {
            game_generate_code(code, n);
            assert((int)strlen(code) == n);
            for (int j = 0; j < n; j++) assert(code[j] >= '0' && code[j] <= '9');
            if (k == 0) strcpy(first, code);
            else if (strcmp(first, code) != 0) differs = 1;
        }
    }
    assert(differs);
}

static void check(const char *code, const char *guess, int in_place, int misplaced) {
    GuessResult r = game_evaluate(code, guess, (int)strlen(code));
    assert(r.in_place == in_place);
    assert(r.misplaced == misplaced);
}

static void test_evaluate_hints(void) {
    check("4827", "4827", 4, 0);   check("1111", "2222", 0, 0);
    check("1234", "4321", 0, 4);   check("4827", "4278", 1, 3);
    check("1123", "1111", 2, 0);   check("1123", "3211", 0, 4);
    check("1123", "2111", 1, 2);   check("0000", "0000", 4, 0);
    check("012345", "543210", 0, 6);  check("112233", "121212", 2, 2);
}

static void test_remaining_ms(void) {
    assert(game_remaining_ms(90, 5000, 5000, 0) == 90000);
    assert(game_remaining_ms(90, 5000, 15000, 0) == 80000);
    assert(game_remaining_ms(90, 5000, 15000, 5000) == 75000);
    assert(game_remaining_ms(60, 0, 45000, 15000) == 0);      /* exactly zero */
    assert(game_remaining_ms(60, 0, 50000, 15000) == -5000);  /* past zero */
}

int main(void) {
    test_config_presets();
    test_generate_code_shape();
    test_evaluate_hints();
    test_remaining_ms();
    puts("all tests passed");
    return 0;
}
