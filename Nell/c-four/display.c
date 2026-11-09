#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include "display.h"

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static int s_color_ok = 0;
static int s_initialized = 0;
static UINT s_orig_cp = 0;
static DWORD s_orig_mode = 0;
static int s_have_mode = 0;
static CONSOLE_CURSOR_INFO s_orig_cursor;
static int s_have_cursor = 0;

static BOOL WINAPI ctrl_handler(DWORD type) {
    (void)type;
    display_shutdown();
    return FALSE; /* let the default handler end the process */
}

int display_init(void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    s_orig_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);

    if (GetConsoleMode(out, &mode)) {
        s_orig_mode = mode;
        s_have_mode = 1;
        s_color_ok = SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
    }

    if (GetConsoleCursorInfo(out, &s_orig_cursor)) {
        CONSOLE_CURSOR_INFO hidden = s_orig_cursor;
        hidden.bVisible = FALSE;
        SetConsoleCursorInfo(out, &hidden);
        s_have_cursor = 1;
    }

    s_initialized = 1;
    atexit(display_shutdown);
    SetConsoleCtrlHandler(ctrl_handler, TRUE);
    return s_color_ok;
}

void display_shutdown(void) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!s_initialized) return;
    s_initialized = 0;
    if (s_color_ok) { fputs("\x1b[0m", stdout); fflush(stdout); }
    if (s_have_cursor) SetConsoleCursorInfo(out, &s_orig_cursor);
    if (s_have_mode) SetConsoleMode(out, s_orig_mode);
    if (s_orig_cp) SetConsoleOutputCP(s_orig_cp);
}

int display_size_ok(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) return 1;
    int w = info.srWindow.Right - info.srWindow.Left + 1;
    int h = info.srWindow.Bottom - info.srWindow.Top + 1;
    return w >= 80 && h >= 25;
}

void display_set_color(Color c) {
    if (!s_color_ok) return;
    switch (c) {
        case COL_GREEN:  fputs("\x1b[92m", stdout); break;
        case COL_RED:    fputs("\x1b[91m", stdout); break;
        case COL_YELLOW: fputs("\x1b[93m", stdout); break;
        case COL_DEFAULT:
        default:         fputs("\x1b[0m", stdout);  break;
    }
}

void display_home(void) {
    if (s_color_ok) {
        fputs("\x1b[H", stdout);
    } else {
        COORD origin = {0, 0};
        fflush(stdout);
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), origin);
    }
}

void display_clear(void) {
    if (s_color_ok) { fputs("\x1b[2J\x1b[H", stdout); fflush(stdout); }
    else system("cls");
}

void display_flush_input(void) {
    while (_kbhit()) (void)_getch();
}

int display_read_key(void) {
    int c = _getch();
    if (c == 0 || c == 0xE0) { (void)_getch(); return -1; }  /* arrow / function key */
    return c;
}

/* ---------- screens ---------- */

#define SCREEN_W 80
#define LEFT_W 37
#define RIGHT_W 38
#define BODY_ROWS 18
#define HISTORY_ROWS 8
#define PANEL_INNER 26

typedef struct { char ch; const char *rows[5]; } Glyph;

static const Glyph GLYPHS[] = {
    {'0', {"#####", "#   #", "#   #", "#   #", "#####"}},
    {'1', {"  #  ", " ##  ", "  #  ", "  #  ", " ### "}},
    {'2', {"#####", "    #", "#####", "#    ", "#####"}},
    {'3', {"#####", "    #", "#####", "    #", "#####"}},
    {'4', {"#   #", "#   #", "#####", "    #", "    #"}},
    {'5', {"#####", "#    ", "#####", "    #", "#####"}},
    {'6', {"#####", "#    ", "#####", "#   #", "#####"}},
    {'7', {"#####", "    #", "    #", "    #", "    #"}},
    {'8', {"#####", "#   #", "#####", "#   #", "#####"}},
    {'9', {"#####", "#   #", "#####", "    #", "#####"}},
    {':', {"   ",   " # ",   "   ",   " # ",   "   "}},
    {'D', {"#### ", "#   #", "#   #", "#   #", "#### "}},
    {'E', {"#####", "#    ", "#### ", "#    ", "#####"}},
    {'F', {"#####", "#    ", "#### ", "#    ", "#    "}},
    {'U', {"#   #", "#   #", "#   #", "#   #", "#####"}},
    {'S', {"#####", "#    ", "#####", "    #", "#####"}},
    {'B', {"#### ", "#   #", "#### ", "#   #", "#### "}},
    {'O', {"#####", "#   #", "#   #", "#   #", "#####"}},
    {'M', {"#   #", "## ##", "# # #", "#   #", "#   #"}},
    {'C', {"#####", "#    ", "#    ", "#    ", "#####"}},
    {'R', {"#### ", "#   #", "#### ", "#  # ", "#   #"}},
    {'-', {"     ", "     ", " ### ", "     ", "     "}},
};

static int vis_width(const char *s) {
    int n = 0;
    for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++;
    return n;
}

static void append(char *dst, size_t cap, const char *src) {
    size_t n = strlen(dst);
    if (n < cap) snprintf(dst + n, cap - n, "%s", src);
}

static void put_rep(const char *glyph, int n) { while (n-- > 0) fputs(glyph, stdout); }

static void put_pad(const char *s, int width) {
    int w = vis_width(s);
    fputs(s, stdout);
    for (; w < width; w++) fputc(' ', stdout);
}

static void put_center(const char *s) {
    int lead = (SCREEN_W - vis_width(s)) / 2;
    for (int i = 0; i < lead; i++) fputc(' ', stdout);
    fputs(s, stdout);
    fputc('\n', stdout);
}

static const Glyph *find_glyph(char ch) {
    for (size_t i = 0; i < sizeof GLYPHS / sizeof GLYPHS[0]; i++)
        if (GLYPHS[i].ch == ch) return &GLYPHS[i];
    return &GLYPHS[0];
}

/* One of the 5 rows of a word drawn in block letters ('#' becomes a full block). */
static void word_row(char *dst, size_t cap, const char *word, int row) {
    dst[0] = '\0';
    for (const char *p = word; *p; p++) {
        const char *src = find_glyph(*p)->rows[row];
        if (p != word) append(dst, cap, " ");
        for (; *src; src++) append(dst, cap, *src == '#' ? "█" : " ");
    }
}

static void put_banner(const char *word) {
    char buf[200];
    for (int r = 0; r < 5; r++) {
        word_row(buf, sizeof buf, word, r);
        put_center(buf);
    }
}

static void edge(char *dst, size_t cap, const char *l, const char *fill, int n, const char *r) {
    dst[0] = '\0';
    append(dst, cap, "  ");
    append(dst, cap, l);
    for (int i = 0; i < n; i++) append(dst, cap, fill);
    append(dst, cap, r);
}

static void panel(char *dst, size_t cap, const char *inner) {
    char pad[PANEL_INNER + 1];
    int n = PANEL_INNER - vis_width(inner);
    if (n < 0) n = 0;
    memset(pad, ' ', (size_t)n);
    pad[n] = '\0';
    snprintf(dst, cap, "  ║%s%s║", inner, pad);
}

static void build_left(char left[][96], Color lcol[]) {
    const char *bricks[3] = {
        " ████ ████ ████ ████ ████ ",
        " ▓▓▓▓ ▓▓▓▓ ▓▓▓▓ ▓▓▓▓ ▓▓▓▓ ",
        " ░░░░ ░░░░ ░░░░ ░░░░ ░░░░ "
    };
    for (int r = 0; r < BODY_ROWS; r++) { left[r][0] = '\0'; lcol[r] = COL_GREEN; }
    edge(left[1], 96, "╔", "═", PANEL_INNER, "╗");
    panel(left[2], 96, bricks[0]);
    panel(left[3], 96, bricks[0]);
    panel(left[4], 96, bricks[1]);
    panel(left[5], 96, bricks[1]);
    panel(left[6], 96, bricks[2]);
    edge(left[7], 96, "╚", "═", PANEL_INNER, "╝");
    snprintf(left[8], 96, "      │         │         │");
    snprintf(left[9], 96, "      │         │         │");
    snprintf(left[10], 96, "  ╔═══╧═════════╧═════════╧══╗");
    panel(left[11], 96, "  STATUS:  ARMED");
    lcol[11] = COL_RED;
    panel(left[12], 96, "  [ C-4 ]  ENTER THE CODE");
    edge(left[13], 96, "╚", "═", PANEL_INNER, "╝");
}

void display_round(const GameConfig *cfg, long long remaining_ms, const char *input,
                   const HistoryEntry *history, int history_count, int flash) {
    char left[BODY_ROWS][96];
    char right[BODY_ROWS][160];
    Color lcol[BODY_ROWS], rcol[BODY_ROWS];
    char ts[8], info[48], title[160], slot[8];
    long long secs = remaining_ms <= 0 ? 0 : (remaining_ms + 999) / 1000;
    int mm = (int)(secs / 60), ss = (int)(secs % 60);
    Color tcol = (flash || remaining_ms <= 10000) ? COL_RED : COL_GREEN;
    Color bcol = flash ? COL_RED : COL_GREEN;
    size_t in_len = input ? strlen(input) : 0;

    if (mm < 0) mm = 0;
    if (mm > 99) mm = 99;
    if (ss < 0) ss = 0;
    if (ss > 59) ss = 59;
    snprintf(ts, sizeof ts, "%02d:%02d", mm, ss);

    build_left(left, lcol);
    for (int r = 0; r < BODY_ROWS; r++) { right[r][0] = '\0'; rcol[r] = COL_GREEN; }

    for (int r = 0; r < 5; r++) {
        char row[140];
        word_row(row, sizeof row, ts, r);
        snprintf(right[1 + r], sizeof right[1 + r], "     %s", row);
        rcol[1 + r] = tcol;
    }

    snprintf(right[7], sizeof right[7], "  CODE: ");
    for (int i = 0; i < cfg->digits; i++) {
        snprintf(slot, sizeof slot, "[%c]%s", (size_t)i < in_len ? input[i] : '_',
                 i < cfg->digits - 1 ? " " : "");
        append(right[7], sizeof right[7], slot);
    }

    snprintf(right[9], sizeof right[9], "  %-11s%-10s%s", "GUESS", "IN PLACE", "MISPLACED");
    {
        int start = history_count > HISTORY_ROWS ? history_count - HISTORY_ROWS : 0;
        for (int k = start; k < history_count; k++)
            snprintf(right[10 + (k - start)], sizeof right[10], "  %-11s%-10d%d",
                     history[k].guess, history[k].result.in_place, history[k].result.misplaced);
    }

    snprintf(info, sizeof info, "%d-DIGIT CODE  |  -%ds PER MISS ", cfg->digits, cfg->penalty);
    snprintf(title, sizeof title, "  %-42s%s", "C-FOUR :: BOMB DEFUSAL SIMULATOR", info);

    display_home();
    display_set_color(bcol);
    fputs("╔", stdout); put_rep("═", LEFT_W + 1 + RIGHT_W); fputs("╗\n", stdout);
    fputs("║", stdout); put_pad(title, LEFT_W + 1 + RIGHT_W); fputs("║\n", stdout);
    fputs("╠", stdout); put_rep("═", LEFT_W); fputs("╦", stdout);
    put_rep("═", RIGHT_W); fputs("╣\n", stdout);
    for (int r = 0; r < BODY_ROWS; r++) {
        display_set_color(bcol); fputs("║", stdout);
        display_set_color(lcol[r]); put_pad(left[r], LEFT_W);
        display_set_color(bcol); fputs("║", stdout);
        display_set_color(rcol[r]); put_pad(right[r], RIGHT_W);
        display_set_color(bcol); fputs("║\n", stdout);
    }
    fputs("╠", stdout); put_rep("═", LEFT_W); fputs("╩", stdout);
    put_rep("═", RIGHT_W); fputs("╣\n", stdout);
    fputs("║", stdout);
    put_pad("  DIGITS = TYPE    BACKSPACE = ERASE    ENTER = SUBMIT", LEFT_W + 1 + RIGHT_W);
    fputs("║\n", stdout);
    fputs("╚", stdout); put_rep("═", LEFT_W + 1 + RIGHT_W); fputs("╝\n", stdout);
    display_set_color(COL_DEFAULT);
    fflush(stdout);
}

void display_defused(long long remaining_ms, int guesses) {
    long long secs = remaining_ms <= 0 ? 0 : (remaining_ms + 999) / 1000;
    char line[100];
    display_clear();
    display_set_color(COL_GREEN);
    fputs("\n\n\n", stdout);
    put_banner("DEFUSED");
    fputs("\n", stdout);
    put_center("BOMB DEFUSED - YOU SAVED THE DAY");
    snprintf(line, sizeof line, "Time left: %02d:%02d     Guesses used: %d",
             (int)(secs / 60), (int)(secs % 60), guesses);
    put_center(line);
    fputs("\n", stdout);
    display_set_color(COL_DEFAULT);
    fflush(stdout);
}

void display_boom(int guesses) {
    char line[100];
    display_clear();
    display_set_color(COL_RED);
    fputs("\n", stdout);
    put_center("░░▒▒▓▓██▓▓▒▒░░");
    fputs("\n", stdout);
    put_banner("BOOM");
    fputs("\n", stdout);
    put_center("░░▒▒▓▓██▓▓▒▒░░");
    fputs("\n", stdout);
    put_center("THE TIMER HIT ZERO - THE BOMB WENT OFF");
    snprintf(line, sizeof line, "Guesses used: %d", guesses);
    put_center(line);
    fputs("\n", stdout);
    display_set_color(COL_DEFAULT);
    fflush(stdout);
}

void display_banner(const char *word) { put_banner(word); }

void display_center(const char *s) { put_center(s); }
