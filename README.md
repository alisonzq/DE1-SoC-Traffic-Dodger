# DE1-SoC VGA & PS/2 Traffic Dodger

C drivers for the DE1-SoC's VGA pixel/character buffers and PS/2 keyboard port, built up into a small lane-dodging traffic game that runs on the board's timer interrupt.

## Overview

Three progressively larger programs, each reusing the drivers built in the one before it.

**task1.c — VGA driver & test screen**
Baseline drivers for the 320x240 pixel buffer and the 80x60 character overlay: `VGA_draw_point`, `VGA_write_char`, and buffer-clear routines. `draw_test_screen()` renders a diagonal RGB gradient and overlays "Hello World!" as a smoke test for both buffers.

**task2.c — PS/2 keyboard input**
Adds `read_PS2_data()` to poll the PS/2 core's RVALID bit and pull a scancode byte, plus `write_hex_digit`/`write_byte_kbrd` to render each incoming scancode as two hex characters. `input_loop_fun()` streams live keypresses to the character buffer, filling the screen left-to-right, top-to-bottom.

**task3.c — traffic dodger game**
Combines the VGA and PS/2 drivers with the ARM A9 private timer to build a real-time game: a player car (driven by the A/D keys) dodges randomly-spawned traffic cars scrolling down a two-lane road. Game state advances only on timer interrupts (~0.05s tick), giving frame-independent movement, increasing speed over time, and a score that increments per car dodged. Collision detection ends the round and displays a "Game Over" / final score screen; pressing S restarts.

## Files

| File | Description |
|---|---|
| `task1.c` | VGA pixel/character buffer drivers + gradient/text test screen |
| `task2.c` | PS/2 keyboard polling driver + live scancode-to-hex display |
| `task3.c` | Full traffic-dodging game: VGA + PS/2 + timer interrupts |

## Key Concepts Demonstrated

- Memory-mapped VGA output: pixel buffer (`0xC8000000`, 16-bit color, `y<<10 | x<<1` addressing) and character buffer (`0xC9000000`, `y<<7 | x` addressing)
- PS/2 polling: reading the RVALID bit (bit 15) at `0xFF200100` and decoding make/break scancodes (`0xF0` break-code prefix, `0x1C`/`0x23`/`0x1B` for A/D/S)
- ARM A9 private timer configuration and polling (`0xFFFEC600`–`0xFFFEC60C`) to drive fixed-tick game logic independent of loop speed
- Basic game architecture: entity arrays (`TrafficCar[]`), spawn/despawn logic, axis-aligned bounding-box collision detection, and a pseudo-random LCG for lane selection
- Flicker-free redraw technique: clearing only the trailing edge of a moving sprite each tick instead of redrawing the full frame

## Build & Run

Compile with an ARM cross-compiler targeting the DE1-SoC's Nios/ARM computer system (e.g., as used in the course's HPS/FPGA lab environment) and run on-board or in the provided system emulator. Each file's `main()` is a standalone entry point — `task3.c` is the complete, runnable game.

## Controls (task3.c)

- **S** — start / restart game after "Game Over"
- **A** — move player car left
- **D** — move player car right
