# C-Four: Bomb Defusal Simulator — Design Spec

Date: 2026-10-20
Status: Draft for review

## 1. Purpose and scope

A console game written in C for a group school assignment (open-ended, no
fixed requirements). The player must defuse a C4 bomb by entering the correct
secret code before a live countdown reaches zero. The look is retro arcade
("pixel art"), built from text characters, block glyphs and colors.

Idea originated from the group leader. In scope: the single-player game
described below. Out of scope (deliberately): sound effects, high scores,
saving to files, networking, true pixel graphics or any graphics library.

## 2. Platform and build

- Target: Windows only (Command Prompt / PowerShell / Windows Terminal).
- Language: C (C99), compiled with MinGW gcc or Code::Blocks.
- Windows-specific pieces: `conio.h` (`_kbhit`, `_getch`) for non-blocking
  keys, `windows.h` (`Sleep`, `GetTickCount64`, console mode and code page).
- No external libraries.
- Build: `gcc -std=c99 -Wall -Wextra *.c -o cfour.exe`

## 3. Gameplay rules

1. Player picks a difficulty: Normal, Intermediate or Hard.
2. The game generates a secret code of N digits. Each digit is independently
   random 0-9, so repeats and leading zeros are allowed.
3. A countdown starts when the round starts and ticks live, once per second,
   while the player types.
4. The player types digits to fill N input slots. Backspace erases the last
   digit. Enter submits only when all N slots are filled; otherwise Enter does
   nothing. All other keys are ignored.
5. On submit:
   - All N digits in place: the bomb is DEFUSED, the player wins.
   - Otherwise: the guess and its hint are added to the on-screen history,
     the difficulty's time penalty is subtracted immediately, and the timer
     flashes red.
6. If remaining time reaches zero or below at any moment (including because
   of a penalty), the bomb explodes (BOOM screen), the player loses.
7. There is no cap on guesses; the time penalty is the only cost of a wrong
   guess.
8. The end screen shows the result, time left (win only) and guesses used,
   then asks "Play again? (Y/N)".

### Difficulty presets

| Difficulty   | Digits | Time (s) | Penalty per wrong guess (s) |
|--------------|--------|----------|-----------------------------|
| Normal       | 4      | 90       | 5                           |
| Intermediate | 5      | 75       | 10                          |
| Hard         | 6      | 60       | 15                          |

### Hints (Mastermind style)

After each wrong guess the player sees two numbers:
- `in_place`: digits correct and in the right position.
- `misplaced`: digits that exist in the code but are in the wrong position.

Example: code `4827`, guess `4278` gives 1 in place (the 4), 3 misplaced (the 8, 2 and 7).

Algorithm (must handle repeated digits correctly):
1. Count positions where guess[i] == code[i]; these are `in_place`. Mark those
   positions as used in both strings.
2. Count how many times each digit 0-9 appears in the unused code positions
   and in the unused guess positions.
3. `misplaced` = sum over digits of min(code_count[d], guess_count[d]).

Example with repeats: code `1123`, guess `1111` gives 2 in place, 0 misplaced.

## 4. Architecture

Five modules, each a `.c` plus `.h`:

| Module      | Responsibility |
|-------------|----------------|
| `main.c`    | Top-level flow: title, difficulty menu, round, result, play again. Seeds the random generator once with `srand(time(NULL))`. |
| `menu.c`    | Draws title screen and difficulty menu; returns the chosen `Difficulty`. |
| `game.c`    | Pure logic, no printing and no keyboard: difficulty presets, code generation, guess evaluation (hints). |
| `round.c`   | Runs one live round: polling loop, input buffer, timer and penalty math, win/lose decision. |
| `display.c` | All drawing: ANSI colors, cursor control, ASCII bomb, block-digit timer, hint history, DEFUSED and BOOM screens. |

Dependencies flow one way: `main` uses `menu` and `round`; `round` uses `game`
and `display`; `menu` uses `display`; `game` depends on nothing but the
standard library. This keeps each group member's file independent.

### Data types

```c
typedef enum { DIFF_NORMAL, DIFF_INTERMEDIATE, DIFF_HARD } Difficulty;

typedef struct {
    int digits;      /* code length */
    int seconds;     /* starting time */
    int penalty;     /* seconds lost per wrong guess */
} GameConfig;

typedef struct {
    int in_place;
    int misplaced;
} GuessResult;
```

The secret code and guesses are stored as null-terminated character arrays
(for example `"0482"`), max length 6 plus terminator.

### Public interface of `game.c`

```c
GameConfig  game_get_config(Difficulty d);
void        game_generate_code(char *code, int digits);
GuessResult game_evaluate(const char *code, const char *guess, int digits);
```

## 5. Round loop and timing

- At round start, record `start_ms = GetTickCount64()` and set
  `penalty_ms = 0`.
- Remaining time: `remaining_ms = seconds*1000 - (now_ms - start_ms) - penalty_ms`.
- Loop: poll keys with `_kbhit()`; compute remaining time; redraw the timer
  line only when the displayed second changes; `Sleep(50)` each iteration so
  the CPU is not busy-waiting.
- Wrong guess: `penalty_ms += penalty * 1000`, trigger red flash.
- Lose when `remaining_ms <= 0`.
- Timer is shown as `MM:SS`, turns red when 10 seconds or fewer remain.
- The history panel shows the most recent 8 guesses with their hints, so the
  layout always fits the screen.

## 6. Visual design

- Fixed layout of about 80x25 characters.
- Block characters (full, dark, light shade) and box borders for the arcade look.
- Title screen: large block-letter "C-FOUR".
- Main screen: ASCII C4 charge (explosive bricks, wires, display panel),
  large block-digit timer, input slots, hint history.
- Default color green; timer red under 10 seconds; red flash on wrong guess;
  distinct DEFUSED (green) and BOOM (red) end screens.
- Redraw by moving the cursor to the top-left (`ESC[H`) instead of clearing
  the screen each frame, to avoid flicker.
- At startup: `SetConsoleOutputCP(CP_UTF8)` for block glyphs, and enable
  `ENABLE_VIRTUAL_TERMINAL_PROCESSING` for ANSI colors.

## 7. Error handling

- Non-digit keys are ignored; Enter with an incomplete code does nothing.
- If ANSI color support cannot be enabled, the game falls back to plain,
  uncolored output instead of crashing.
- Menu and play-again prompts re-ask on invalid input.

## 8. Testing

- Automated: `test_game.c` using `assert`, covering `game_evaluate`:
  exact match, no match, all misplaced, mixed, and repeated-digit cases
  (`1123` vs `1111`, `1123` vs `3211`, `0000` vs `0000`), plus
  `game_generate_code` producing the right length with only digits.
- Manual playtest checklist: one win and one timeout on each difficulty;
  a penalty that pushes the time to zero ends the round; Backspace and Enter
  behave as in section 3; play-again returns to the menu cleanly.

## 9. Suggested work split (optional)

Because modules are independent, a group can divide the work by file:
`display.c`, `menu.c`, `game.c` plus tests, `round.c`, and `main.c` as
integration. Agree on the `.h` interfaces first.
