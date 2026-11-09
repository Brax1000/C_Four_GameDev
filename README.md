# 💣 C-Four: Bomb Defusal Simulator (Team GameDev Edition)

> **BSIT - 1C Collaborative Game Development Project**  
> **Technological University of the Philippines - Manila (TUP Manila)**  
> Developed in **C Language** featuring multiple game architectures and evolutionary versions.

---

## 👥 The Development Team (BSIT - 1C)

This project was built collaboratively as a beginner game development and systems programming showcase by:
- 🎖️ **Braxten Orline Urdelas** (`Brax1000`) — Lead Developer, C-Four2 Engine & Raylib Integration
- 🧠 **John Hayden Nell** — Architecture, Modular C Console Engine & Specifications (`Nell/c-four`)
- 🛠️ **David Pansoy** — Logic Testing, Design Support & Quality Assurance

---

## 🎮 Project Versions Overview

This repository contains **three versions** tracking our team's learning progression in C:

```text
C_Four/
├── C-Four2/       ⭐ REFINED VERSION: "Last Chance" Retro CRT Text-Mode Edition (Raylib + Audio)
├── C-Four1/       Initial working prototype with basic sound & graphics
└── Nell/          Modular pure C99 console architecture with unit tests & design specs
```

---

## ⭐ C-Four2 — The Refined Edition (Highlighted)

**`C-Four2`** represents the pinnacle of our game's evolution. It reimagines the tactical defusal simulator as a **retro text-mode CRT experience** running on an 80x30 virtual terminal powered by the **Raylib** library.

### 🌟 Key Highlights & Features
- **Retro CRT Shader & 4 Color Themes**: Press **`T`** anytime to cycle themes:
  - 🟢 **GREEN**: Classic IBM 5151 Phosphor CRT
  - 🟠 **AMBER**: High-contrast Amber terminal
  - 🟣 **C64**: Commodore 64 aesthetic
  - 🟩 **GAMEBOY**: Nostalgic 4-shade LCD green
- **Dynamic Particle Physics**: Custom particle systems for sparks, smoke, and explosion detonation.
- **Matrix Code Rain**: Procedural code rain animation on successful defusal.
- **Atmospheric Audio Engine**: Integrated background soundtrack (`assets/background.mp3`) with audio device management.
- **Tactical Clues & Star Ratings**:
  - Live ticking countdown timer
  - Range narrowing clues (Low / High bounds)
  - Post-mission performance evaluation (1 to 3 stars based on attempts and time left)
- **Interactive Story & Animated Boot Sequences**: Cinematic boot loader, mission briefings, and speech box dialogues.

---

## 🕹️ Other Versions & Credits

### 1. `C-Four1` (The Initial Prototype)
- The first proof-of-concept version that bridged simple console input with Raylib audio and graphics.
- Laid the foundation for input handling, sound integration, and the countdown loop.

### 2. `Nell` (Modular Pure C99 Console Edition)
- Created by **John Hayden Nell**.
- Features professional modular C design without third-party graphics libraries:
  - Clean separation into `display.c`, `menu.c`, `round.c`, and `game.c`.
  - Comprehensive documentation and design specifications in `docs/superpowers/`.
  - Standalone unit test suite (`tests/test_game.c`, `round_demo.c`).
  - Code::Blocks project configuration (`c-four.cbp`).
  - Utilizes Windows Console APIs (`conio.h`, `windows.h`) for high-speed buffer rendering.

---

## 🚀 How to Run the Games (Beginner Tutorial)

### ⚡ Quickest Way (Double-Click & Play)
You don't need any compiler installed to play right now:

1. **To play the refined version (C-Four2)**:
   - Navigate into the `C-Four2` folder.
   - Double-click **`game.exe`**.
   - Enjoy the retro CRT soundtrack and graphics!
2. **To play the pure console version (Nell)**:
   - Navigate into `Nell/c-four`.
   - Double-click **`cfour.exe`**.

---

## 🛠️ How to Compile from Source Code (C Language Tutorial)

If you are learning C or want to modify the source code, follow these guides:

### Compiling `C-Four2` (Requires Raylib & GCC)
`C-Four2` utilizes the lightweight C game development library [Raylib](https://www.raylib.com/).

1. **Install Raylib & MinGW/GCC**:
   - The easiest way on Windows is via **w64devkit** or **MSYS2 UCRT64**:
     ```bash
     pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
     ```
2. **Compile command**:
   ```bash
   cd C-Four2
   gcc main.c -lraylib -lopengl32 -lgdi32 -lwinmm -o game.exe
   ```
3. Run:
   ```bash
   ./game.exe
   ```

### Compiling `Nell/c-four` (Pure Standard C99)
This version has **zero dependencies** and compiles with standard MinGW:

```bash
cd Nell/c-four
gcc -std=c99 -Wall -Wextra *.c -o cfour.exe
./cfour.exe
```

---

## 📚 Core C Programming Concepts Practiced in this Project

For beginners reviewing this codebase, this project demonstrates:

1. **Structs & State Machines**:
   - `typedef struct { ... } Game;` manages current screen states (`BOOT`, `STORY`, `MENU`, `PLAYING`, `RESULTS`).
2. **Dynamic Timing & Delta Time**:
   - Calculating frame-independent timers using `GetFrameTime()` and clock timers.
3. **Array Parsing & Buffer Management**:
   - 80x30 text buffer grids, character array strings, and safe formatting with `snprintf()`.
4. **Pseudo-Random Number Generation**:
   - Seeding with `time(NULL)` and computing bounded random ranges: `min + rand() % (max - min + 1)`.
5. **Modular Architecture & Header Guards**:
   - Splitting systems into `.h` headers and `.c` translation units (`#ifndef`, `#define`, `#endif`).
6. **Audio & Media Device Lifecycles**:
   - Initializing, streaming, and cleanly disposing audio devices to prevent memory leaks.

---

## 📜 License & Acknowledgments

- Built by **Braxten Orline Urdelas**, **John Hayden Nell**, and **David Pansoy** (BSIT - 1C, TUP Manila).
- Developed for educational and collaborative learning purposes.
