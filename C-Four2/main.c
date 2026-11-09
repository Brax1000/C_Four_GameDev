#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/*
 * C-FOUR: LAST CHANCE  --  retro text-mode edition
 * The whole game is an 80x30 character screen, like an old DOS terminal.
 * Everything (titles, bars, boxes, art) is made of characters.
 */

#define GAME_W 1280
#define GAME_H 800
#define COLS 80
#define ROWS 30
#define CW 16.0f
#define CH 26.0f
#define FS 20.0f
#define OX 0.0f
#define OY 10.0f

#define MAX_GUESSES 10
#define MAX_PARTICLES 300

/* -------------------------------------------------- */
/* THEMES (press T)                                   */
/* -------------------------------------------------- */

typedef struct {
    const char *name;
    Color main, dim, accent, bg, red;
} Theme;

static const Theme THEMES[4] = {
    {"GREEN",   { 70, 255, 120, 255}, { 45, 170,  85, 255}, {200, 255, 120, 255}, { 3, 12,  7, 255}, {255,  85,  70, 255}},
    {"AMBER",   {255, 176,   0, 255}, {175, 115,   0, 255}, {255, 226, 140, 255}, {14,  8,  0, 255}, {255,  90,  60, 255}},
    {"C64",     {162, 155, 255, 255}, {125, 118, 225, 255}, {255, 255, 255, 255}, {64, 49, 141, 255}, {255, 119, 119, 255}},
    {"GAMEBOY", {155, 188,  15, 255}, { 95, 135,  50, 255}, {222, 240, 160, 255}, {15, 56, 15, 255}, {255, 110,  90, 255}}
};

static int themeIndex = 0;

enum { K_MAIN = 0, K_DIM = 1, K_ACC = 2, K_RED = 3, K_BG = 4, K_NONE = 255 };

static Color Pal(int k)
{
    const Theme *t = &THEMES[themeIndex];
    switch (k)
    {
        case K_MAIN: return t->main;
        case K_DIM:  return t->dim;
        case K_ACC:  return t->accent;
        case K_RED:  return t->red;
        default:     return t->bg;
    }
}

typedef enum {
    BOOT,
    STORY,
    GAMEOVER,
    MENU,
    DIFFICULTY,
    PLAYING,
    PAUSED,
    CONTROLS,
    RESULTS
} Screen;

typedef struct {
    Screen screen;
    Screen previousScreen;
    float screenT;
    int menuSel;
    int storyPage;
    bool boomDone;

    int level;
    int minimum;
    int maximum;
    int secret;
    int low;
    int high;

    int attempts;
    int maxAttempts;

    int guesses[MAX_GUESSES];
    int guessDir[MAX_GUESSES];
    int guessCount;

    char input[8];
    int inputLength;

    float timer;
    float animation;

    bool won;
    bool warned;
    int score;
    int best;
    int stars;

    char message[120];
    int messageColor;

    char opLine[128];
    float opT;
    int opMood;
    float opMoodT;
} Game;

typedef struct {
    Vector2 pos, vel;
    float life, maxLife, size;
    int color;
    char ch;
} Particle;

static Font retroFont;
static float glyphW[128];
static Music music;
static bool musicReady = false;
static bool musicEnabled = true;
static bool exitGame = false;

static Camera2D camera = {0};
static Vector2 mouseWorld = {0};
static bool uiLocked = false;

static float shake = 0.0f;
static float flash = 0.0f;
static float glitch = 0.0f;

static Particle particles[MAX_PARTICLES];
static int particleNext = 0;

/* -------------------------------------------------- */
/* CHARACTER SCREEN BUFFER                            */
/* -------------------------------------------------- */

static unsigned char cellCh[ROWS][COLS];
static unsigned char cellFg[ROWS][COLS];
static unsigned char cellBg[ROWS][COLS];

static float Rand01(void)
{
    return GetRandomValue(0, 1000) / 1000.0f;
}

static void BufClear(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
        {
            cellCh[r][c] = ' ';
            cellFg[r][c] = K_MAIN;
            cellBg[r][c] = K_NONE;
        }
}

static void Pt(int c, int r, unsigned char ch, int fg, int bg)
{
    if (c < 0 || c >= COLS || r < 0 || r >= ROWS) return;
    cellCh[r][c] = ch;
    cellFg[r][c] = (unsigned char)fg;
    cellBg[r][c] = (unsigned char)bg;
}

static void TxtB(int c, int r, const char *s, int fg, int bg)
{
    for (int i = 0; s[i]; i++)
        Pt(c + i, r, (unsigned char)s[i], fg, bg);
}

static void Txt(int c, int r, const char *s, int fg)
{
    TxtB(c, r, s, fg, K_NONE);
}

static void TxtC(int r, const char *s, int fg)
{
    Txt((COLS - (int)strlen(s)) / 2, r, s, fg);
}

static void Fill(int c, int r, int w, int h, int bg)
{
    for (int y = r; y < r + h; y++)
        for (int x = c; x < c + w; x++)
            Pt(x, y, ' ', K_MAIN, bg);
}

static void Frm(int c, int r, int w, int h, const char *title)
{
    Fill(c, r, w, h, K_NONE);

    for (int x = c; x < c + w; x++)
    {
        Pt(x, r, '=', K_MAIN, K_NONE);
        Pt(x, r + h - 1, '=', K_MAIN, K_NONE);
    }

    for (int y = r; y < r + h; y++)
    {
        Pt(c, y, '|', K_MAIN, K_NONE);
        Pt(c + w - 1, y, '|', K_MAIN, K_NONE);
    }

    Pt(c, r, '+', K_MAIN, K_NONE);
    Pt(c + w - 1, r, '+', K_MAIN, K_NONE);
    Pt(c, r + h - 1, '+', K_MAIN, K_NONE);
    Pt(c + w - 1, r + h - 1, '+', K_MAIN, K_NONE);

    if (title)
    {
        char t[64];
        snprintf(t, sizeof(t), "[ %s ]", title);
        Txt(c + 2, r, t, K_ACC);
    }
}

static void Header(const char *rightText)
{
    Fill(0, 0, COLS, 1, K_MAIN);
    TxtB(1, 0, " C-FOUR :: LAST CHANCE ", K_BG, K_MAIN);

    int len = (int)strlen(rightText);
    TxtB(COLS - 2 - len, 0, rightText, K_BG, K_MAIN);
}

static void Footer(const char *text)
{
    Fill(0, ROWS - 1, COLS, 1, K_DIM);
    TxtB(1, ROWS - 1, text, K_MAIN, K_DIM);
}

static bool CellHover(int c, int r, int w, int h)
{
    if (uiLocked) return false;

    float mx = (mouseWorld.x - OX) / CW;
    float my = (mouseWorld.y - OY) / CH;

    return mx >= c && mx < c + w && my >= r && my < r + h;
}

/* clickable text, inverse video on hover */
static bool TxtBtn(int c, int r, const char *s, int fg, int hoverBg)
{
    int w = (int)strlen(s);
    bool hv = CellHover(c, r, w, 1);

    if (hv) TxtB(c, r, s, K_BG, hoverBg);
    else    Txt(c, r, s, fg);

    return hv && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

/* keyboard + mouse menu list. returns chosen index or -1 */
static int MenuList(Game *g, int c, int r, int w, int spacing,
                    const char **items, int n)
{
    int result = -1;
    Vector2 d = GetMouseDelta();
    bool moved = (d.x != 0 || d.y != 0);

    if (!uiLocked)
    {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            g->menuSel = (g->menuSel + 1) % n;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            g->menuSel = (g->menuSel + n - 1) % n;
    }

    for (int i = 0; i < n; i++)
    {
        int row = r + i * spacing;
        bool hv = CellHover(c, row, w, 1);

        if (hv && moved) g->menuSel = i;

        char line[96];
        bool sel = (i == g->menuSel);

        snprintf(line, sizeof(line), "%c %s", sel ? '>' : ' ', items[i]);

        if (sel)
        {
            Fill(c, row, w, 1, K_MAIN);
            TxtB(c, row, line, K_BG, K_MAIN);
        }
        else
        {
            Txt(c, row, line, K_MAIN);
        }

        if (!uiLocked)
        {
            if (hv && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) result = i;
            if (sel && IsKeyPressed(KEY_ENTER)) result = i;
        }
    }

    return result;
}

static void GoTo(Game *g, Screen s)
{
    g->screen = s;
    g->menuSel = 0;
    g->screenT = 0.0f;
    g->storyPage = 0;
    g->boomDone = false;
}

/* word wrap in character cells */
static int WrapPut(int col, int row, int width, const char *text,
                   int fg, int maxLines)
{
    char line[128] = "";
    char word[64];
    const char *p = text;
    int lines = 0;

    while (*p)
    {
        while (*p == ' ') p++;

        int w = 0;
        while (*p && *p != ' ' && w < 63) word[w++] = *p++;
        word[w] = '\0';
        if (w == 0) break;

        int ll = (int)strlen(line);

        if (ll > 0 && ll + 1 + w > width)
        {
            if (lines >= maxLines) return lines;
            Txt(col, row + lines, line, fg);
            lines++;
            snprintf(line, sizeof(line), "%s", word);
        }
        else
        {
            if (ll > 0) { line[ll++] = ' '; line[ll] = '\0'; }
            snprintf(line + ll, sizeof(line) - ll, "%s", word);
        }
    }

    if (line[0] && lines < maxLines)
    {
        Txt(col, row + lines, line, fg);
        lines++;
    }

    return lines;
}

/* -------------------------------------------------- */
/* 5x5 BLOCK FONT                                     */
/* -------------------------------------------------- */

typedef struct { char ch; const char *r[5]; } Glyph;

static const Glyph FONT5[] = {
    {' ', {".....", ".....", ".....", ".....", "....."}},
    {'-', {".....", ".....", "#####", ".....", "....."}},
    {'!', {"..#..", "..#..", "..#..", ".....", "..#.."}},
    {':', {".....", "..#..", ".....", "..#..", "....."}},
    {'A', {".###.", "#...#", "#####", "#...#", "#...#"}},
    {'B', {"####.", "#...#", "####.", "#...#", "####."}},
    {'C', {".####", "#....", "#....", "#....", ".####"}},
    {'D', {"####.", "#...#", "#...#", "#...#", "####."}},
    {'E', {"#####", "#....", "####.", "#....", "#####"}},
    {'F', {"#####", "#....", "####.", "#....", "#...."}},
    {'G', {".####", "#....", "#..##", "#...#", ".####"}},
    {'V', {"#...#", "#...#", "#...#", ".#.#.", "..#.."}},
    {'H', {"#...#", "#...#", "#####", "#...#", "#...#"}},
    {'I', {"#####", "..#..", "..#..", "..#..", "#####"}},
    {'L', {"#....", "#....", "#....", "#....", "#####"}},
    {'M', {"#...#", "##.##", "#.#.#", "#...#", "#...#"}},
    {'N', {"#...#", "##..#", "#.#.#", "#..##", "#...#"}},
    {'O', {".###.", "#...#", "#...#", "#...#", ".###."}},
    {'P', {"####.", "#...#", "####.", "#....", "#...."}},
    {'R', {"####.", "#...#", "####.", "#.#..", "#..##"}},
    {'S', {".####", "#....", ".###.", "....#", "####."}},
    {'T', {"#####", "..#..", "..#..", "..#..", "..#.."}},
    {'U', {"#...#", "#...#", "#...#", "#...#", ".###."}},
    {'0', {".###.", "#..##", "#.#.#", "##..#", ".###."}},
    {'1', {"..#..", ".##..", "..#..", "..#..", ".###."}},
    {'2', {".###.", "#...#", "..##.", ".#...", "#####"}},
    {'3', {"####.", "....#", ".###.", "....#", "####."}},
    {'4', {"#..#.", "#..#.", "#####", "...#.", "...#."}},
    {'5', {"#####", "#....", "####.", "....#", "####."}},
    {'6', {".###.", "#....", "####.", "#...#", ".###."}},
    {'7', {"#####", "....#", "...#.", "..#..", "..#.."}},
    {'8', {".###.", "#...#", ".###.", "#...#", ".###."}},
    {'9', {".###.", "#...#", ".####", "....#", ".###."}}
};

static void BigText(int c, int r, const char *s, int fg)
{
    int n = (int)(sizeof(FONT5) / sizeof(FONT5[0]));

    for (int i = 0; s[i]; i++)
    {
        const Glyph *gl = NULL;

        for (int k = 0; k < n; k++)
            if (FONT5[k].ch == s[i]) { gl = &FONT5[k]; break; }

        if (!gl) continue;

        for (int y = 0; y < 5; y++)
            for (int x = 0; x < 5; x++)
                if (gl->r[y][x] == '#')
                    Pt(c + i * 6 + x, r + y, 1, fg, K_NONE);
    }
}

static void BigTextS(int c, int r, const char *s, int fg)
{
    BigText(c + 1, r + 1, s, K_DIM);
    BigText(c, r, s, fg);
}

/* -------------------------------------------------- */
/* PARTICLES (flying characters)                      */
/* -------------------------------------------------- */

static void Spawn(float x, float y, float vx, float vy,
                  float life, float size, int color, char ch)
{
    Particle *p = &particles[particleNext];
    particleNext = (particleNext + 1) % MAX_PARTICLES;

    p->pos = (Vector2){x, y};
    p->vel = (Vector2){vx, vy};
    p->life = life;
    p->maxLife = life;
    p->size = size;
    p->color = color;
    p->ch = ch;
}

static void Burst(float x, float y, int n, float speed, int color, const char *chars)
{
    int len = (int)strlen(chars);

    for (int i = 0; i < n; i++)
    {
        float a = Rand01() * 2.0f * PI;
        float sp = speed * (0.3f + Rand01());

        Spawn(x, y, cosf(a) * sp, sinf(a) * sp,
              0.5f + Rand01() * 0.6f, 20 + Rand01() * 14, color,
              chars[GetRandomValue(0, len - 1)]);
    }
}

static void CodeRain(int n)
{
    const char *chars = "01*+$&#@";

    for (int i = 0; i < n; i++)
        Spawn(Rand01() * GAME_W, -20 - Rand01() * 200,
              (Rand01() - 0.5f) * 30, 90 + Rand01() * 160,
              4.5f + Rand01() * 2.0f, 20 + Rand01() * 12,
              GetRandomValue(0, 2) == 0 ? K_ACC : K_MAIN,
              chars[GetRandomValue(0, 7)]);
}

static void Explosion(float x, float y)
{
    const char *chars = "*#%@!+x";
    int cols[3] = {K_RED, K_ACC, K_RED};

    flash = 1.0f;
    shake = 22.0f;
    glitch = 1.6f;

    for (int i = 0; i < 140; i++)
    {
        float a = Rand01() * 2.0f * PI;
        float sp = 120 + Rand01() * 520;

        Spawn(x, y, cosf(a) * sp, sinf(a) * sp,
              0.8f + Rand01() * 0.9f, 22 + Rand01() * 18,
              cols[GetRandomValue(0, 2)], chars[GetRandomValue(0, 6)]);
    }
}

static void UpdateParticles(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &particles[i];
        if (p->life <= 0) continue;

        p->life -= dt;
        p->vel.y += 160.0f * dt;
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
    }
}

static void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &particles[i];
        if (p->life <= 0) continue;

        float a = (p->life / p->maxLife) * 3.0f;
        if (a > 1.0f) a = 1.0f;

        char s[2] = {p->ch, '\0'};
        DrawTextEx(retroFont, s, p->pos, p->size, 0, ColorAlpha(Pal(p->color), a));
    }
}

/* -------------------------------------------------- */
/* RENDER THE CHARACTER BUFFER                        */
/* -------------------------------------------------- */

static void RenderBuffer(void)
{
    ClearBackground(Pal(K_BG));

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            unsigned char ch = cellCh[r][c];
            int bg = cellBg[r][c];
            float x = OX + c * CW;
            float y = OY + r * CH;

            if (bg != K_NONE)
                DrawRectangle((int)x, (int)y, (int)CW, (int)CH, Pal(bg));

            if (ch == ' ') continue;

            Color fg = Pal(cellFg[r][c]);

            if (ch == 1)
            {
                DrawRectangle((int)x, (int)y + 1, (int)CW, (int)CH - 2, fg);
            }
            else if (ch == 2)
            {
                for (int py = 0; py < 24; py += 4)
                    for (int px = 0; px < 16; px += 4)
                        if (((px / 4) + (py / 4)) % 2 == 0)
                            DrawRectangle((int)x + px, (int)y + py + 1, 4, 4, fg);
            }
            else if (ch == 3)
            {
                for (int py = 0; py < 24; py += 8)
                    for (int px = 0; px < 16; px += 8)
                        DrawRectangle((int)x + px + ((py / 8) % 2) * 4,
                                      (int)y + py + 3, 2, 2, fg);
            }
            else if (ch >= 32 && ch < 127)
            {
                char b[2] = {(char)ch, '\0'};
                float w = glyphW[ch];

                DrawTextEx(retroFont, b,
                           (Vector2){x + (CW - w) / 2.0f, y + (CH - FS) / 2.0f},
                           FS, 0, fg);
            }
        }
    }
}

static void CrtOverlay(void)
{
    for (int y = 0; y < GAME_H; y += 3)
        DrawRectangle(0, y, GAME_W, 1, (Color){0, 0, 0, 18});

    DrawRectangleGradientV(0, 0, GAME_W, 70, (Color){0, 0, 0, 70}, (Color){0, 0, 0, 0});
    DrawRectangleGradientV(0, GAME_H - 70, GAME_W, 70, (Color){0, 0, 0, 0}, (Color){0, 0, 0, 70});
    DrawRectangleGradientH(0, 0, 60, GAME_H, (Color){0, 0, 0, 60}, (Color){0, 0, 0, 0});
    DrawRectangleGradientH(GAME_W - 60, 0, 60, GAME_H, (Color){0, 0, 0, 0}, (Color){0, 0, 0, 60});

    DrawRectangle(0, 0, GAME_W, GAME_H, (Color){0, 0, 0, (unsigned char)GetRandomValue(0, 9)});
}

/* falling code in the background */
static void Rain(float t)
{
    static const char R[] = "01#@%&*+=<>/|";

    for (int i = 0; i < 26; i++)
    {
        int col = (i * 3 + (i * 7) % 3) % COLS;
        float speed = 4.0f + (i % 5) * 2.0f;
        float head = fmodf(i * 5.3f + t * speed, ROWS + 10.0f);

        for (int k = 0; k < 7; k++)
        {
            int row = (int)head - k;
            if (row < 0 || row >= ROWS) continue;

            char ch = R[(i * 11 + row * 5 + (k == 0 ? (int)(t * 8) : 0)) % 13];
            Pt(col, row, (unsigned char)ch, K_DIM, K_NONE);
        }
    }
}

/* -------------------------------------------------- */
/* ASCII ART                                          */
/* -------------------------------------------------- */

static void RobotArt(int c, int r, float t, int mood)
{
    const char *eyes = mood == 1 ? "^ ^" : (mood == 2 ? "O O" : (mood == 3 ? "x x"
                     : (fmodf(t, 4.0f) > 3.85f ? "- -" : "o o")));
    const char *mouth = mood == 1 ? "\\_/" : (mood == 2 ? "~~~" : (mood == 3 ? "/^\\" : "---"));
    char ant = fmodf(t * 2.0f, 1.0f) < 0.5f ? '*' : 'o';
    char l0[32], l3[32], l4[32];
    int fg = mood == 2 ? K_ACC : K_MAIN;

    snprintf(l0, sizeof(l0), "      %c      ", ant);
    snprintf(l3, sizeof(l3), "  |  %s  |  ", eyes);
    snprintf(l4, sizeof(l4), "  |  %s  |  ", mouth);

    Txt(c, r + 0, l0, fg);
    Txt(c, r + 1, "      |      ", fg);
    Txt(c, r + 2, "  .-------.  ", fg);
    Txt(c, r + 3, l3, fg);
    Txt(c, r + 4, l4, fg);
    Txt(c, r + 5, "  '-------'  ", fg);
    Txt(c, r + 6, " /|[=====]|\\ ", fg);
    Txt(c, r + 7, "  | OP-07 |  ", fg);
    Txt(c, r + 8, "  '-------'  ", fg);
}

/* C-4 device, 24 characters wide and 8 rows tall */
static void BombArt(int c, int r, const char *display, bool ledOn, int fg)
{
    char top[32], bot[32], l[6][32], tmp[32];

    top[0] = '.'; bot[0] = '\'';
    for (int i = 1; i <= 22; i++) { top[i] = '-'; bot[i] = '-'; }
    top[23] = '.'; bot[23] = '\'';
    top[24] = bot[24] = '\0';

    snprintf(l[0], sizeof(l[0]), "|%-22s|", " [C-4]==[C-4]");
    snprintf(l[1], sizeof(l[1]), "|%-22s|", " |###|  |###|");
    snprintf(l[2], sizeof(l[2]), "|%-22s|", " '---'  '---'");

    snprintf(tmp, sizeof(tmp), " +-------+   %s", ledOn ? "(*)" : "( )");
    snprintf(l[3], sizeof(l[3]), "|%-22s|", tmp);

    snprintf(tmp, sizeof(tmp), " | %-5s |", display);
    snprintf(l[4], sizeof(l[4]), "|%-22s|", tmp);

    snprintf(l[5], sizeof(l[5]), "|%-22s|", " +-------+");

    Txt(c, r, top, fg);
    for (int i = 0; i < 6; i++) Txt(c, r + 1 + i, l[i], fg);
    Txt(c, r + 7, bot, fg);

    /* the LED glows */
    if (ledOn) Txt(c + 16, r + 4, "(*)", K_RED);
}

static void ExplosionArt(int c, int r, float t)
{
    char star = fmodf(t * 8.0f, 1.0f) < 0.5f ? '*' : '#';
    char l2[32];
    int fg = fmodf(t * 6.0f, 1.0f) < 0.5f ? K_RED : K_ACC;

    snprintf(l2, sizeof(l2), " -- %cBOOM!%c -- ", star, star);

    Txt(c, r + 0, "   \\   |   /   ", fg);
    Txt(c, r + 1, "    \\  |  /    ", fg);
    Txt(c, r + 2, l2, fg);
    Txt(c, r + 3, "    /  |  \\    ", fg);
    Txt(c, r + 4, "   /   |   \\   ", fg);
}

/* -------------------------------------------------- */
/* GAME LOGIC                                         */
/* -------------------------------------------------- */

static const char *HIGHER_LINES[4] = {
    "Target is HIGHER. Adjust your guess.",
    "Too low. Go up.",
    "Negative. The code is bigger.",
    "Scanner says: higher!"
};

static const char *LOWER_LINES[4] = {
    "Target is LOWER. Adjust your guess.",
    "Too high. Come down.",
    "Negative. The code is smaller.",
    "Scanner says: lower!"
};

static void Say(Game *g, const char *text, int mood, float moodTime)
{
    snprintf(g->opLine, sizeof(g->opLine), "%s", text);
    g->opT = 0.0f;
    g->opMood = mood;
    g->opMoodT = moodTime;
}

static int OpMood(Game *g)
{
    if (g->opMoodT > 0) return g->opMood;

    if ((g->screen == PLAYING || g->screen == PAUSED) &&
        (g->timer < 15.0f || g->attempts <= 2))
        return 2;

    return 0;
}

static const char *TimeStr(float t)
{
    if (t < 0) t = 0;
    return TextFormat("%02d:%02d", (int)t / 60, (int)t % 60);
}

static void ResetMission(Game *g)
{
    g->inputLength = 0;
    g->input[0] = '\0';

    g->guessCount = 0;
    g->attempts = g->maxAttempts;
    g->low = g->minimum;
    g->high = g->maximum;

    g->timer = 90.0f;
    g->won = false;
    g->warned = false;

    snprintf(g->message, sizeof(g->message),
             "DEVICE ONLINE. ENTER THE DEACTIVATION CODE.");
    g->messageColor = K_ACC;

    Say(g, "Device armed. Enter the code and I will watch the clock.", 0, 0.0f);
}

static void SetDifficulty(Game *g, int level)
{
    g->level = level;

    if (level == 0)      { g->minimum = 1; g->maximum = 10;  g->maxAttempts = 5;  }
    else if (level == 1) { g->minimum = 1; g->maximum = 50;  g->maxAttempts = 7;  }
    else if (level == 2) { g->minimum = 1; g->maximum = 100; g->maxAttempts = 8;  }
    else                 { g->minimum = 1; g->maximum = 500; g->maxAttempts = 10; }

    g->secret = GetRandomValue(g->minimum, g->maximum);
    ResetMission(g);

    /* throw away typed characters so they don't leak into the game */
    while (GetCharPressed() > 0) {}
}

static void AddDigit(Game *g, int digit)
{
    int maxDigits = g->maximum >= 100 ? 3 : 2;

    if (g->inputLength >= maxDigits)
        return;

    if (g->inputLength == 0 && digit == 0)
        return;

    g->input[g->inputLength++] = (char)('0' + digit);
    g->input[g->inputLength] = '\0';
}

static void ClearInput(Game *g)
{
    g->inputLength = 0;
    g->input[0] = '\0';
}

static void EndMission(Game *g, bool won)
{
    g->won = won;
    GoTo(g, won ? RESULTS : GAMEOVER);

    if (won)
    {
        g->score = g->attempts * 150 + (int)g->timer * 10 + (g->level + 1) * 100;
        if (g->score > g->best) g->best = g->score;

        float ratio = (float)g->guessCount / (float)g->maxAttempts;
        g->stars = ratio <= 0.4f ? 3 : (ratio <= 0.7f ? 2 : 1);

        Say(g, "DEVICE DISARMED. Outstanding work, operative!", 1, 9999.0f);
        CodeRain(120);
    }
    else
    {
        g->score = 0;
        g->stars = 0;

        Say(g, "Oh no... it was a NUKE! Mochi-Ville is gone. We can try again!", 3, 9999.0f);
    }
}

static void SubmitGuess(Game *g)
{
    if (g->inputLength == 0)
    {
        snprintf(g->message, sizeof(g->message), "PLEASE ENTER A NUMBER FIRST.");
        g->messageColor = K_ACC;
        return;
    }

    int guess = atoi(g->input);
    ClearInput(g);

    if (guess < g->minimum || guess > g->maximum)
    {
        snprintf(g->message, sizeof(g->message),
                 "INVALID NUMBER. RANGE: %d TO %d.", g->minimum, g->maximum);
        g->messageColor = K_RED;
        shake = 5.0f;
        return;
    }

    for (int i = 0; i < g->guessCount; i++)
    {
        if (g->guesses[i] == guess)
        {
            snprintf(g->message, sizeof(g->message), "YOU ALREADY TRIED THAT NUMBER.");
            g->messageColor = K_RED;
            return;
        }
    }

    if (g->guessCount < MAX_GUESSES)
    {
        g->guesses[g->guessCount] = guess;
        g->guessDir[g->guessCount] = guess < g->secret ? 1 : -1;
        g->guessCount++;
    }

    if (guess == g->secret)
    {
        EndMission(g, true);
        return;
    }

    g->attempts--;

    if (guess < g->secret)
    {
        if (guess + 1 > g->low) g->low = guess + 1;
        snprintf(g->message, sizeof(g->message), "INCORRECT! THE SECRET CODE IS HIGHER.");
    }
    else
    {
        if (guess - 1 < g->high) g->high = guess - 1;
        snprintf(g->message, sizeof(g->message), "INCORRECT! THE SECRET CODE IS LOWER.");
    }

    g->messageColor = K_RED;
    shake = 7.0f;
    Burst(OX + 16 * CW, OY + 24 * CH, 14, 170, K_RED, "*#%@!+x");

    if (g->attempts <= 0)
    {
        EndMission(g, false);
        return;
    }

    if (g->attempts <= 2)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "Warning: only %d %s left. Stay calm.",
                 g->attempts, g->attempts == 1 ? "try" : "tries");
        Say(g, buf, 2, 3.0f);
    }
    else
    {
        Say(g, guess < g->secret ? HIGHER_LINES[GetRandomValue(0, 3)]
                                 : LOWER_LINES[GetRandomValue(0, 3)],
            2, 1.8f);
    }
}

static void ToggleMusic(void)
{
    musicEnabled = !musicEnabled;

    if (musicReady)
        SetMusicVolume(music, musicEnabled ? 0.30f : 0.0f);
}

/* -------------------------------------------------- */
/* BOOT SCREEN                                        */
/* -------------------------------------------------- */

static void DrawBoot(Game *g)
{
    const char *lines[10] = {
        "C-FOUR SYSTEMS (C) 2026",
        "BSIT - 1C",
        "BRAXTEN ORLINE URDELAS",
        "JOHN HAYDEN NELL",
        "DAVID PANSOY",
        "BONO ISAIAN TENDENCIA",
        "BIOS v2.04  EXPLOSIVE ORDNANCE UNIT",
        "",
        "CHECKING MEMORY..............  640K OK",
        "DETECTING DISPLAY............  CRT-80x30 OK",
        "DETECTING AUDIO..............  ",
        "LINKING TO REMOTE DEVICE.....  ONLINE",
        "LOADING DEACTIVATION PROTOCOL...",
        "",
        "PRESS ANY KEY TO CONTINUE"
    };

    int budget = (int)(g->screenT * 75.0f);

    for (int i = 0; i < 10 && budget > 0; i++)
    {
        char buf[96];
        snprintf(buf, sizeof(buf), "%s%s", lines[i],
                 i == 5 ? (musicReady ? "OK" : "NO MUSIC FILE") : "");

        int len = (int)strlen(buf);
        int show = budget < len ? budget : len;
        buf[show] = '\0';

        Txt(2, 2 + i, buf, i == 9 ? K_ACC : K_MAIN);
        budget -= len + 4;
    }

    if (g->screenT > 3.2f && fmodf(g->screenT, 1.0f) < 0.5f)
        Txt(28, 12, "_", K_ACC);

    RobotArt(33, 16, g->animation, 0);
}

/* -------------------------------------------------- */
/* MENU                                               */
/* -------------------------------------------------- */

static void DrawMenu(Game *g)
{
    static const char *tips[4] = {
        "Greetings, operative. I am OP-07, your support unit. Ready to defuse some bombs?",
        "Pick a threat level and I will guide you through it.",
        "Tip: every wrong guess narrows the known range.",
        "Stay calm and watch the fuse. We can do this."
    };

    BufClear();
    Rain(g->animation);
    Header("MAIN MENU");

    BigTextS(22, 3, "C-FOUR", K_ACC);
    BigTextS(7, 9, "LAST CHANCE", K_MAIN);

    TxtC(15, "EXPLOSIVE ORDNANCE // NUMBER DEACTIVATION PROTOCOL", K_DIM);

    const char *items[4] = {
        "START MISSION",
        "HOW TO PLAY",
        musicEnabled ? "MUSIC: ON   [M]" : "MUSIC: OFF  [M]",
        "EXIT GAME"
    };

    int pick = MenuList(g, 8, 17, 30, 2, items, 4);

    if (pick == 0) GoTo(g, DIFFICULTY);
    else if (pick == 1) { g->previousScreen = MENU; GoTo(g, CONTROLS); }
    else if (pick == 2) ToggleMusic();
    else if (pick == 3) exitGame = true;

    RobotArt(54, 16, g->animation, 0);

    int idx = (int)(g->animation / 6.0f) % 4;
    int chars = (int)(fmodf(g->animation, 6.0f) * 30.0f);
    char buf[128];
    int len = (int)strlen(tips[idx]);
    if (chars < len) len = chars;
    memcpy(buf, tips[idx], len);
    buf[len] = '\0';

    Txt(8, 26, "OP-07>", K_ACC);
    WrapPut(15, 26, 62, buf, K_MAIN, 2);

    Txt(8, 28, TextFormat("BEST SCORE: %d", g->best), K_ACC);

    Footer(TextFormat("UP/DOWN+ENTER=SELECT  T=THEME[%s]  M=MUSIC  F11=FULLSCREEN",
                      THEMES[themeIndex].name));
}

/* -------------------------------------------------- */
/* DIFFICULTY                                         */
/* -------------------------------------------------- */

static void DrawDifficulty(Game *g)
{
    BufClear();
    Rain(g->animation);
    Header("MISSION CONFIGURATION");

    BigTextS(14, 2, "SELECT", K_ACC);
    TxtC(8, "CHOOSE YOUR THREAT LEVEL  (PRESS 1-4 OR CLICK)", K_DIM);

    static char buf[4][96];
    const char *items[4];
    const char *names[]  = {"EASY  ", "NORMAL", "HARD  ", "INSANE"};
    const char *ranges[] = {"1 - 10 ", "1 - 50 ", "1 - 100", "1 - 500"};
    const char *tries[]  = {"5 TRIES ", "7 TRIES ", "8 TRIES ", "10 TRIES"};
    const char *danger[] = {"[#---]", "[##--]", "[###-]", "[####]"};

    for (int i = 0; i < 4; i++)
    {
        snprintf(buf[i], sizeof(buf[i]), "[%d] %s   %s   %s   DANGER %s",
                 i + 1, names[i], ranges[i], tries[i], danger[i]);
        items[i] = buf[i];
    }

    int pick = MenuList(g, 8, 10, 64, 2, items, 4);

    if (pick >= 0)
    {
        SetDifficulty(g, pick);
        GoTo(g, STORY);
        return;
    }

    BombArt(28, 19, "????", fmodf(g->animation * 1.5f, 1.0f) < 0.5f, K_MAIN);

    Txt(8, 18, "[ESC] BACK TO MENU", K_DIM);

    Footer("UP/DOWN+ENTER=SELECT  1-4=QUICK PICK  ESC=BACK");
}

/* -------------------------------------------------- */
/* GAME SCREEN                                        */
/* -------------------------------------------------- */

static void SlotBox(int c, int r, int w, int h, int fg, int bg)
{
    Fill(c, r, w, h, bg);

    for (int x = c; x < c + w; x++)
    {
        Pt(x, r, '=', fg, bg);
        Pt(x, r + h - 1, '=', fg, bg);
    }

    for (int y = r; y < r + h; y++)
    {
        Pt(c, y, '|', fg, bg);
        Pt(c + w - 1, y, '|', fg, bg);
    }

    Pt(c, r, '+', fg, bg);
    Pt(c + w - 1, r, '+', fg, bg);
    Pt(c, r + h - 1, '+', fg, bg);
    Pt(c + w - 1, r + h - 1, '+', fg, bg);
}

static void DrawGameScreen(Game *g)
{
    BufClear();
    Header("LIVE DEVICE // CONNECTION ACTIVE");

    bool danger = g->timer < 15;
    bool blink = fmodf(g->animation * (danger ? 6.0f : 1.5f), 1.0f) < 0.5f;
    bool cursorOn = fmodf(g->animation, 0.8f) < 0.4f;
    int timerFg = danger ? K_RED : K_MAIN;

    float progress = g->timer / 90.0f;
    if (progress < 0) progress = 0;

    /* ---- timer + burning fuse ---- */
    Frm(1, 1, 38, 11, "TIMER");
    Txt(4, 2, "TIME REMAINING", K_MAIN);
    BigText(4, 3, TimeStr(g->timer), timerFg);

    int spark = (int)(progress * 28);
    Pt(4, 9, '[', K_MAIN, K_NONE);
    for (int i = 0; i < 28; i++)
    {
        if (i < spark)       Pt(5 + i, 9, '=', K_MAIN, K_NONE);
        else if (i == spark) Pt(5 + i, 9, blink ? '*' : '+', K_RED, K_NONE);
    }
    Pt(33, 9, ']', K_MAIN, K_NONE);

    if (danger) Txt(4, 10, blink ? "STATUS: CRITICAL!!" : "                  ", K_RED);
    else        Txt(4, 10, "STATUS: FUSE BURNING", K_MAIN);

    /* ---- operator ---- */
    Frm(40, 1, 39, 11, "OPERATOR");
    RobotArt(42, 2, g->animation, OpMood(g));
    Txt(57, 2, "OP-07>", K_ACC);

    {
        char buf[160];
        int len = (int)strlen(g->opLine);
        int chars = (int)(g->opT * 45.0f);
        if (chars < len) len = chars;
        if (len > 159) len = 159;
        memcpy(buf, g->opLine, len);
        buf[len] = '\0';
        WrapPut(57, 3, 20, buf, K_MAIN, 8);
    }

    /* ---- BIG code entry ---- */
    Frm(1, 12, 78, 13, "CODE ENTRY");

    {
        char lab[80];
        snprintf(lab, sizeof(lab), ">> ENTER DEACTIVATION CODE  (%d - %d) <<",
                 g->minimum, g->maximum);
        TxtC(13, lab, K_ACC);
    }

    int digits = g->maximum >= 100 ? 3 : 2;
    int total = digits * 10 + (digits - 1) * 2;
    int start = (COLS - total) / 2;

    for (int i = 0; i < digits; i++)
    {
        int c = start + i * 12;
        bool filled = (i < g->inputLength);
        bool active = (i == g->inputLength);

        SlotBox(c, 14, 10, 7, active ? K_ACC : K_MAIN,
                (filled || active) ? K_DIM : K_NONE);

        if (filled)
        {
            char d[2] = {g->input[i], '\0'};
            BigText(c + 2, 15, d, K_ACC);
        }
        else if (active)
        {
            if (cursorOn) Txt(c + 2, 19, "-----", K_ACC);
        }
        else
        {
            Txt(c + 4, 17, "..", K_MAIN);
        }
    }

    /* message bar: inverse video so it is easy to read */
    {
        int mc = g->messageColor;
        int len = (int)strlen(g->message);

        Fill(3, 21, 74, 1, mc);
        TxtB((COLS - len) / 2, 21, g->message, K_BG, mc);
    }

    /* clickable keypad */
    for (int i = 0; i < 10; i++)
    {
        int digit = (i + 1) % 10;
        char key[4] = {'[', (char)('0' + digit), ']', '\0'};

        if (TxtBtn(16 + i * 4, 22, key, K_MAIN, K_MAIN))
            AddDigit(g, digit);
    }

    if (TxtBtn(57, 22, "[DEL]", K_MAIN, K_MAIN) && g->inputLength > 0)
    {
        g->inputLength--;
        g->input[g->inputLength] = '\0';
    }

    if (TxtBtn(3, 23, "[ PAUSE (P) ]", K_MAIN, K_MAIN))
        GoTo(g, PAUSED);

    if (TxtBtn(30, 23, "[  SUBMIT CODE  ]", K_ACC, K_ACC))
        SubmitGuess(g);

    if (TxtBtn(66, 23, "[ CLEAR ]", K_MAIN, K_MAIN))
        ClearInput(g);

    /* ---- intel strip ---- */
    Frm(1, 25, 78, 4, "INTEL");

    {
        char pips[64] = "";
        for (int i = 0; i < g->maxAttempts; i++)
            strcat(pips, i < g->attempts ? "[#]" : "[ ]");

        Txt(3, 26, "TRIES", K_MAIN);
        Txt(9, 26, pips, K_ACC);
        Txt(42, 26, TextFormat("KNOWN RANGE: %d - %d", g->low, g->high), K_MAIN);
    }

    {
        char line[40];
        int range = g->maximum - g->minimum;
        if (range < 1) range = 1;

        for (int i = 0; i < 35; i++) line[i] = '.';
        line[35] = '\0';

        int i0 = (g->low - g->minimum) * 34 / range;
        int i1 = (g->high - g->minimum) * 34 / range;
        for (int i = i0; i <= i1 && i < 35; i++)
            if (i >= 0) line[i] = '=';

        for (int k = 0; k < g->guessCount; k++)
        {
            int pos = (g->guesses[k] - g->minimum) * 34 / range;
            if (pos >= 0 && pos < 35) line[pos] = 'x';
        }

        Txt(3, 27, line, K_MAIN);

        int first = g->guessCount > 5 ? g->guessCount - 5 : 0;
        for (int i = first; i < g->guessCount; i++)
            Txt(42 + (i - first) * 7, 27,
                TextFormat("[%3d%c]", g->guesses[i], g->guessDir[i] > 0 ? '^' : 'v'),
                g->guessDir[i] > 0 ? K_MAIN : K_ACC);
    }

    Footer(TextFormat("0-9=DIGIT ENTER=SUBMIT P=PAUSE T=THEME[%s] M=MUSIC",
                      THEMES[themeIndex].name));
}

/* -------------------------------------------------- */
/* PAUSE                                              */
/* -------------------------------------------------- */

static void DrawPause(Game *g)
{
    /* lock the UI so clicks don't reach the game underneath */
    uiLocked = true;
    DrawGameScreen(g);
    uiLocked = false;

    /* dither the game behind the pause window */
    for (int r = 1; r < ROWS - 1; r++)
        for (int c = 0; c < COLS; c++)
            if ((r + c) % 2 == 0)
                cellFg[r][c] = K_DIM;

    Frm(24, 7, 32, 16, "SYSTEM HALTED");
    TxtC(9, "MISSION PAUSED", K_ACC);

    const char *items[4] = {"RESUME", "RESTART MISSION", "HOW TO PLAY", "MAIN MENU"};
    int pick = MenuList(g, 28, 12, 24, 2, items, 4);

    if (pick == 0) GoTo(g, PLAYING);
    else if (pick == 1)
    {
        g->secret = GetRandomValue(g->minimum, g->maximum);
        ResetMission(g);
        GoTo(g, PLAYING);
    }
    else if (pick == 2) { g->previousScreen = PAUSED; GoTo(g, CONTROLS); }
    else if (pick == 3) GoTo(g, MENU);
}

/* -------------------------------------------------- */
/* CONTROLS                                           */
/* -------------------------------------------------- */

static void DrawControls(Game *g)
{
    BufClear();
    Rain(g->animation);
    Header("FIELD MANUAL");

    BigTextS(18, 2, "HELP", K_ACC);

    Frm(10, 9, 60, 17, "CONTROLS");

    Txt(14, 11, "0 - 9        Enter a digit", K_MAIN);
    Txt(14, 12, "ENTER        Submit your code", K_MAIN);
    Txt(14, 13, "BACKSPACE    Delete a digit", K_MAIN);
    Txt(14, 14, "ESC / P      Pause the game", K_MAIN);
    Txt(14, 15, "1 - 4        Quick-pick a difficulty", K_MAIN);
    Txt(14, 16, "MOUSE        Click keypad and menus", K_MAIN);
    Txt(14, 17, "M            Toggle background music", K_ACC);
    Txt(14, 18, "T            Change color theme", K_ACC);
    Txt(14, 19, "F11          Fullscreen / windowed", K_ACC);

    Txt(14, 21, "OBJECTIVE", K_ACC);
    WrapPut(14, 22, 52,
            "Guess the secret code before the fuse burns out or your tries run out. "
            "Every wrong guess tells you if the code is higher or lower.",
            K_MAIN, 3);

    const char *items[1] = {"BACK"};
    int pick = MenuList(g, 30, 27, 20, 1, items, 1);

    if (pick == 0 || IsKeyPressed(KEY_ESCAPE))
        GoTo(g, g->previousScreen);

    Footer("ENTER / ESC = BACK");
}

/* -------------------------------------------------- */
/* RESULTS                                            */
/* -------------------------------------------------- */

static void DrawResults(Game *g)
{
    BufClear();
    Rain(g->animation);
    Header("MISSION REPORT");

    if (g->won) BigTextS(16, 2, "DISARMED", K_ACC);
    else        BigTextS(25, 2, "BOOM!", K_RED);

    Frm(14, 9, 52, 12, "REPORT");

    Txt(18, 11, g->won ? "DEVICE SUCCESSFULLY DISARMED" : "DEVICE DETONATION DETECTED",
        g->won ? K_MAIN : K_RED);

    Txt(18, 13, TextFormat("SECRET CODE : %d", g->secret), K_ACC);
    Txt(18, 14, TextFormat("GUESSES MADE: %d", g->guessCount), K_MAIN);
    Txt(18, 15, TextFormat("TIME LEFT   : %s", TimeStr(g->timer)), K_MAIN);
    Txt(18, 16, TextFormat("SCORE       : %d", g->score), K_ACC);
    Txt(18, 17, TextFormat("BEST        : %d", g->best), K_MAIN);

    if (g->won)
        BombArt(40, 12, "SAFE", false, K_MAIN);
    else
        ExplosionArt(44, 13, g->animation);

    TxtC(22, TextFormat("RATING: %s %s %s",
                        g->stars >= 1 ? "[*]" : "[ ]",
                        g->stars >= 2 ? "[*]" : "[ ]",
                        g->stars >= 3 ? "[*]" : "[ ]"), K_ACC);

    const char *items[2] = {"PLAY AGAIN", "MAIN MENU"};
    int pick = MenuList(g, 30, 24, 20, 2, items, 2);

    if (pick == 0) GoTo(g, DIFFICULTY);
    else if (pick == 1) GoTo(g, MENU);

    {
        char buf[100];
        int len = (int)strlen(g->opLine);
        int chars = (int)(g->opT * 45.0f);
        if (chars < len) len = chars;
        if (len > 99) len = 99;
        memcpy(buf, g->opLine, len);
        buf[len] = '\0';

        Txt(1, 28, "OP-07>", K_ACC);
        Txt(8, 28, buf, K_MAIN);
    }

    Footer("UP/DOWN+ENTER=SELECT");
}

/* -------------------------------------------------- */
/* STORY CUTSCENE                                     */
/* -------------------------------------------------- */

static const char *STORY_TEXT[5] = {
    "Welcome to Mochi-Ville, the sweetest little town in the whole world! Today is PUDDING DAY and everybody is happy!",
    "But oh no! A naughty villain hid a C-4 bomb inside the giant Birthday Pudding! Tick tock, tick tock...",
    "Hi hi! I'm OP-07, your tiny helper bot! I would defuse it myself, but my little paws are too small. That's where YOU come in!",
    "The bomb is locked with a secret number code. Guess it! I'll tell you if the real code is higher or lower.",
    "You only have 90 seconds. Don't worry, I believe in you! Let's save Pudding Day together! Ready?"
};

static void Twinkle(int c, int r, float t, float ph, int fg)
{
    float v = sinf(t * 4.0f + ph);
    Pt(c, r, v > 0.3f ? '*' : (v > -0.4f ? '+' : '.'), fg, K_NONE);
}

static void DrawStoryScene(Game *g)
{
    float t = g->animation;
    bool blink = fmodf(t * 2.0f, 1.0f) < 0.5f;
    char row[80];

    switch (g->storyPage)
    {
        case 0:
        {
            Txt(34, 2, "  \\  |  /  ", K_ACC);
            Txt(34, 3, "-- (^o^) --", K_ACC);
            Txt(34, 4, "  /  |  \\  ", K_ACC);

            for (int i = 0; i < 56; i++) row[i] = (i % 2) ? ' ' : 'v';
            row[56] = '\0';
            Txt(12, 6, row, K_ACC);

            for (int k = 0; k < 3; k++)
            {
                int c = 18 + k * 19;
                Txt(c, 8,  "  /\\  ", K_MAIN);
                Txt(c, 9,  " /  \\ ", K_MAIN);
                Txt(c, 10, "/____\\", K_MAIN);
                Txt(c, 11, "|[][]|", K_MAIN);
                Txt(c, 12, "|_/\\_|", K_MAIN);
            }

            for (int i = 0; i < 60; i++) row[i] = (i % 2) ? '^' : '~';
            row[60] = '\0';
            Txt(10, 13, row, K_DIM);

            for (int k = 0; k < 4; k++)
            {
                int rr = 12 - (int)fmodf(t * 1.5f + k * 2.5f, 8.0f);
                if (rr >= 5) Txt(24 + k * 12, rr, "<3", K_ACC);
            }

            TxtC(15, "~ MOCHI-VILLE ~", K_ACC);
            break;
        }

        case 1:
        {
            Txt(31, 2, "   .-~~~~~~~~-.   ", K_ACC);
            Txt(31, 3, "  (  ^      ^  )  ", K_ACC);
            Txt(31, 4, "  (    \\__/    )  ", K_ACC);
            Txt(31, 5, "   '-.______.-'   ", K_ACC);

            BombArt(28, 7, "??:??", blink, K_MAIN);

            if (blink) Txt(54, 11, "<-- UH OH!", K_RED);
            break;
        }

        case 2:
        {
            static const int sp[6][2] = {{24, 4}, {54, 5}, {28, 10}, {50, 11}, {40, 2}, {20, 8}};

            RobotArt(33, 3, t, 1);

            for (int k = 0; k < 6; k++)
                Twinkle(sp[k][0], sp[k][1], t, k * 1.3f, K_ACC);

            TxtC(13, "~ OP-07, TINY HELPER BOT ~", K_ACC);
            break;
        }

        case 3:
        {
            const char *kp[9] = {
                "+---+---+---+", "| 1 | 2 | 3 |", "+---+---+---+",
                "| 4 | 5 | 6 |", "+---+---+---+", "| 7 | 8 | 9 |",
                "+---+---+---+", "    | 0 |    ", "    +---+    "
            };

            for (int i = 0; i < 9; i++)
                Txt(33, 2 + i, kp[i], K_MAIN);

            Txt(50, 4, "HIGHER ^", K_ACC);
            Txt(50, 8, "LOWER  v", K_ACC);
            Txt(24, 6, blink ? "???" : "   ", K_RED);
            break;
        }

        default:
        {
            RobotArt(33, 3, t, 1);

            for (int k = 0; k < 5; k++)
            {
                int rr = 11 - (int)fmodf(t * 1.6f + k * 2.0f, 9.0f);
                if (rr >= 2) Txt(22 + k * 9, rr, "<3", K_ACC);
            }

            TxtC(13, "<3  GOOD LUCK!  <3", blink ? K_ACC : K_MAIN);
            break;
        }
    }
}

static void DrawStory(Game *g)
{
    BufClear();
    Header("STORY");

    DrawStoryScene(g);

    Frm(6, 17, 68, 10, "STORY");
    Txt(64, 17, TextFormat("[ %d/5 ]", g->storyPage + 1), K_ACC);

    const char *full = STORY_TEXT[g->storyPage];
    char buf[256];
    int len = (int)strlen(full);
    int chars = (int)(g->screenT * 45.0f);
    bool done = chars >= len;

    if (!done) len = chars;
    if (len > 255) len = 255;
    memcpy(buf, full, len);
    buf[len] = '\0';

    Txt(9, 18, g->storyPage < 2 ? "NARRATOR>" : "OP-07>", K_ACC);
    WrapPut(9, 19, 62, buf, K_MAIN, 6);

    if (done && fmodf(g->animation, 1.0f) < 0.5f)
        Txt(60, 25, "ENTER >>", K_ACC);

    Footer("ENTER / CLICK = NEXT     ESC = SKIP STORY");
}

/* -------------------------------------------------- */
/* GAME OVER: THE NUKE                                */
/* -------------------------------------------------- */

static int Hash3(int a, int b, int c)
{
    return (a * 131 + b * 17 + c * 7) & 0x7fffffff;
}

static void DrawGameOver(Game *g)
{
    float t = g->screenT;
    const int ground = 20;
    const int cx = 40;

    BufClear();
    Header("GAME OVER");

    for (int c = 0; c < COLS; c++)
        Pt(c, ground + 1, '=', K_DIM, K_NONE);

    if (t < 1.9f)
    {
        /* the town, and the nuke falling on it */
        for (int k = 0; k < 5; k++)
        {
            int c = 4 + k * 15;
            Txt(c, 16, "  /\\  ", K_MAIN);
            Txt(c, 17, " /  \\ ", K_MAIN);
            Txt(c, 18, "/____\\", K_MAIN);
            Txt(c, 19, "|[][]|", K_MAIN);
            Txt(c, 20, "|_/\\_|", K_MAIN);
        }

        if (fmodf(t * 4.0f, 1.0f) < 0.6f)
            TxtC(2, "!! WARNING: NUCLEAR DEVICE INBOUND !!", K_RED);

        int r0 = 3 + (int)(t / 1.9f * 11.0f);

        for (int rr = 3; rr < r0; rr++)
            Pt(cx, rr, ':', K_DIM, K_NONE);

        Txt(cx - 4, r0 + 0, "   /\\   ", K_ACC);
        Txt(cx - 4, r0 + 1, "  /  \\  ", K_ACC);
        Txt(cx - 4, r0 + 2, " |NUKE| ", K_ACC);
        Txt(cx - 4, r0 + 3, " |____| ", K_ACC);
        Txt(cx - 4, r0 + 4, "  \\  /  ", K_ACC);
        Txt(cx - 4, r0 + 5, "   \\/   ", K_ACC);
    }
    else
    {
        /* rubble where the town used to be */
        for (int c = 0; c < COLS; c++)
        {
            int h = Hash3(c, ground, 5) % 4;
            if (h == 0) Pt(c, ground, '.', K_DIM, K_NONE);
            if (h == 1) Pt(c, ground, ':', K_DIM, K_NONE);
            if (h == 2) Pt(c, ground - 1, ',', K_DIM, K_NONE);
        }

        /* mushroom cloud grows */
        float s = (t - 1.9f) / 3.2f;
        if (s > 1.0f) s = 1.0f;
        s = 1.0f - (1.0f - s) * (1.0f - s);

        int stemH = (int)(7.0f * s);
        float ry = 1.0f + 3.0f * s;
        float rx = 3.0f + 13.0f * s;
        int capC = ground - stemH - (int)(ry * 0.6f) - 1;
        int tick = (int)(t * 14.0f);
        const char *fx = ".:*%#@o";

        for (int r = 1; r <= ground; r++)
        {
            for (int c = cx - 30; c <= cx + 30; c++)
            {
                float dx = (c - cx) / rx;
                float dy = (r - capC) / ry;
                float d = dx * dx + dy * dy;
                int h = Hash3(r, c, tick) % 7;

                int w = 1 + (int)(1.5f * s);
                if (r >= ground - 1) w += 3;

                if (d <= 1.0f)
                    Pt(c, r, (unsigned char)fx[h], d < 0.4f ? K_ACC : K_RED, K_NONE);
                else if (r > capC && abs(c - cx) <= w)
                    Pt(c, r, (unsigned char)fx[h], (h % 2) ? K_RED : K_DIM, K_NONE);
                else if (r >= ground - 1 && fabsf((float)(c - cx)) <= 4.0f + 14.0f * s)
                    Pt(c, r, (unsigned char)fx[h], r == ground ? K_ACC : K_RED, K_NONE);
            }
        }

        /* shockwave */
        float rad = (t - 1.9f) * 22.0f;
        if (rad < 38.0f)
        {
            for (int rr = ground - 2; rr <= ground; rr++)
            {
                Pt(cx - (int)rad, rr, '(', K_ACC, K_NONE);
                Pt(cx + (int)rad, rr, ')', K_ACC, K_NONE);
            }
        }
    }

    if (t > 2.8f)
        BigTextS(13, 1, "GAME OVER", fmodf(t * 5.0f, 1.0f) < 0.8f ? K_RED : K_ACC);

    if (t > 4.2f)
    {
        TxtC(22, TextFormat("THE SECRET CODE WAS %d", g->secret), K_ACC);
        TxtC(23, "Mochi-Ville is gone... but you can try again!", K_MAIN);

        const char *items[3] = {"RETRY MISSION", "MAIN MENU", "QUIT GAME"};
        int pick = MenuList(g, 28, 25, 24, 1, items, 3);

        if (pick == 0)
        {
            SetDifficulty(g, g->level);
            GoTo(g, PLAYING);
            return;
        }
        else if (pick == 1) GoTo(g, MENU);
        else if (pick == 2) exitGame = true;

        Footer("UP/DOWN+ENTER=SELECT");
    }
    else
    {
        Footer("...");
    }
}

/* -------------------------------------------------- */
/* UPDATE                                             */
/* -------------------------------------------------- */

static void UpdateGame(Game *g, float delta)
{
    g->animation += delta;
    g->screenT += delta;
    g->opT += delta;

    if (g->opMoodT > 0 && g->opMoodT < 9000.0f)
        g->opMoodT -= delta;

    if (g->screen == BOOT)
    {
        if (g->screenT > 0.4f &&
            (GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
            GoTo(g, MENU);
    }
    else if (g->screen == STORY)
    {
        int len = (int)strlen(STORY_TEXT[g->storyPage]);
        int typed = (int)(g->screenT * 45.0f);
        bool click = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (click)
        {
            if (typed < len)
                g->screenT = len / 45.0f + 0.05f;
            else
            {
                g->storyPage++;
                g->screenT = 0.0f;

                if (g->storyPage >= 5)
                {
                    GoTo(g, PLAYING);
                    return;
                }
            }
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            GoTo(g, PLAYING);
            return;
        }
    }
    else if (g->screen == GAMEOVER)
    {
        if (!g->boomDone && g->screenT > 1.9f)
        {
            g->boomDone = true;
            Explosion(GAME_W / 2.0f, OY + 20 * CH);
        }
    }
    else if (g->screen == DIFFICULTY)
    {
        for (int i = 0; i < 4; i++)
        {
            if (IsKeyPressed(KEY_ONE + i))
            {
                SetDifficulty(g, i);
                GoTo(g, STORY);
                return;
            }
        }

        if (IsKeyPressed(KEY_ESCAPE))
            GoTo(g, MENU);
    }
    else if (g->screen == PLAYING)
    {
        g->timer -= delta;

        if (g->timer <= 0)
        {
            g->timer = 0;
            EndMission(g, false);
            return;
        }

        if (!g->warned && g->timer < 15.0f)
        {
            g->warned = true;
            Say(g, "WARNING! Fifteen seconds left! Hurry!", 2, 5.0f);
            shake = 4.0f;
        }

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P))
        {
            GoTo(g, PAUSED);
            return;
        }

        if (IsKeyPressed(KEY_BACKSPACE) && g->inputLength > 0)
        {
            g->inputLength--;
            g->input[g->inputLength] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER))
            SubmitGuess(g);

        int character = GetCharPressed();

        while (character > 0)
        {
            if (character >= '0' && character <= '9')
                AddDigit(g, character - '0');

            character = GetCharPressed();
        }
    }
    else if (g->screen == PAUSED)
    {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P))
            GoTo(g, PLAYING);
    }
    else if (g->screen == RESULTS)
    {
        if (g->won && GetRandomValue(0, 4) == 0)
            CodeRain(1);
    }

    /* typed characters outside of gameplay must not pile up */
    if (g->screen != PLAYING)
        while (GetCharPressed() > 0) {}
}

/* -------------------------------------------------- */
/* MAIN                                               */
/* -------------------------------------------------- */

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(GAME_W, GAME_H, "C-FOUR: LAST CHANCE");

    SetWindowMinSize(900, 600);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    /* Start fullscreen */
    ToggleFullscreen();

    /* raylib's built-in pixel font = authentic retro look */
    retroFont = GetFontDefault();
    SetTextureFilter(retroFont.texture, TEXTURE_FILTER_POINT);

    for (int i = 32; i < 127; i++)
    {
        char b[2] = {(char)i, '\0'};
        glyphW[i] = MeasureTextEx(retroFont, b, FS, 0).x;
    }

    /* AUDIO */
    InitAudioDevice();

    if (IsAudioDeviceReady() &&
        FileExists("assets/background.mp3"))
    {
        music = LoadMusicStream("assets/background.mp3");

        if (music.stream.buffer != NULL)
        {
            music.looping = true;
            SetMusicVolume(music, 0.30f);
            PlayMusicStream(music);
            musicReady = true;
        }
    }

    SetRandomSeed((unsigned int)time(NULL));

    Game game = {0};
    game.screen = BOOT;
    game.messageColor = K_ACC;
    snprintf(game.opLine, sizeof(game.opLine), "%s", "Greetings, operative.");

    while (!WindowShouldClose() && !exitGame)
    {
        float delta = GetFrameTime();

        if (delta > 0.05f)
            delta = 0.05f;

        /* Fullscreen toggle */
        if (IsKeyPressed(KEY_F11))
            ToggleFullscreen();

        /* Theme toggle */
        if (IsKeyPressed(KEY_T))
            themeIndex = (themeIndex + 1) % 4;

        /* Music toggle */
        if (IsKeyPressed(KEY_M))
            ToggleMusic();

        if (musicReady)
            UpdateMusicStream(music);

        /* Scale virtual coordinates to the actual screen */
        float scaleX = (float)GetScreenWidth() / GAME_W;
        float scaleY = (float)GetScreenHeight() / GAME_H;
        float scale = fminf(scaleX, scaleY);

        shake -= delta * 30.0f;
        if (shake < 0) shake = 0;
        flash -= delta * 1.6f;
        if (flash < 0) flash = 0;
        glitch -= delta;
        if (glitch < 0) glitch = 0;

        camera.target = (Vector2){GAME_W / 2.0f, GAME_H / 2.0f};
        camera.offset = (Vector2){
            GetScreenWidth() / 2.0f + (Rand01() - 0.5f) * 2.0f * shake * scale,
            GetScreenHeight() / 2.0f + (Rand01() - 0.5f) * 2.0f * shake * scale
        };
        camera.rotation = 0;
        camera.zoom = scale;

        mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);

        UpdateGame(&game, delta);
        UpdateParticles(delta);

        /* build the text screen */
        switch (game.screen)
        {
            case BOOT:       BufClear(); DrawBoot(&game);        break;
            case STORY:      DrawStory(&game);                   break;
            case GAMEOVER:   DrawGameOver(&game);                break;
            case MENU:       DrawMenu(&game);                    break;
            case DIFFICULTY: DrawDifficulty(&game);              break;
            case PLAYING:    DrawGameScreen(&game);              break;
            case PAUSED:     DrawPause(&game);                   break;
            case CONTROLS:   DrawControls(&game);                break;
            case RESULTS:    DrawResults(&game);                 break;
        }

        /* signal glitch after an explosion */
        if (glitch > 0)
        {
            const char *junk = "#@%&*01<>/|";
            int n = (int)(glitch * 60);

            for (int i = 0; i < n; i++)
                Pt(GetRandomValue(0, COLS - 1), GetRandomValue(1, ROWS - 2),
                   (unsigned char)junk[GetRandomValue(0, 10)],
                   GetRandomValue(0, 1) ? K_RED : K_DIM, K_NONE);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        RenderBuffer();
        DrawParticles();

        if (flash > 0)
            DrawRectangle(0, 0, GAME_W, GAME_H,
                          ColorAlpha((Color){255, 190, 90, 255}, flash * 0.75f));

        CrtOverlay();

        EndMode2D();

        EndDrawing();
    }

    if (musicReady)
    {
        StopMusicStream(music);
        UnloadMusicStream(music);
    }

    if (IsAudioDeviceReady())
        CloseAudioDevice();

    CloseWindow();

    return 0;
}
