
# Experiment 1 — 8×8 LED Matrix Display (ATmega32)


---

## Overview

Drives an **8×8 LED matrix** from an ATmega32 using column-scan multiplexing.
A fixed bitmap image is displayed, and two external-interrupt push buttons let the
user toggle live animations without stopping the display loop.

---

## Hardware Connections

| ATmega32 | Connected to |
|----------|-------------|
| PORTA (PA0–PA7) | Column select lines of the 8×8 matrix |
| PORTB (PB0–PB7) | Row data lines of the 8×8 matrix |
| PD2 (INT0) | Push button — toggles vertical scroll |
| PD3 (INT1) | Push button — toggles horizontal bit-rotation |

CPU clock: **1 MHz** (`F_CPU 1000000`)

---

## How It Works

### Column-scan multiplexing (`displayFrame`)
One column is activated at a time via `PORTA` (one-hot, MSB-first).
The matching row byte from `image[8]` is written to `PORTB`, held for 0.5 ms,
then the next column is selected. Cycling all 8 columns fast enough produces a
flicker-free image.

### Interrupt-controlled animations

| Interrupt | Pin | Effect |
|-----------|-----|--------|
| INT0 | PD2 | Toggles `toggle_scroll` — advances `column_selector` each frame (vertical scroll) |
| INT1 | PD3 | Toggles `toggle_shift_bits` — rotates every row byte right each frame (horizontal shift) |

Activating one animation automatically disables the other (each ISR clears the
opposing flag). Both interrupts fire on any logical change (`ISC00`/`ISC10` set).

After every 50 rendered frames the main loop checks the flags and updates the
image state accordingly.

---

## Files

| File | Description |
|------|-------------|
| `main.c` | Full ATmega32 C source |
| `Experiment 1 (LED Matrix).pdf` | Official lab sheet and circuit diagram |

---

## Build & Flash

```bash
avr-gcc -mmcu=atmega32 -DF_CPU=1000000UL -O2 -Wall -o main.elf main.c
avr-objcopy -O ihex main.elf main.hex
avrdude -c usbasp -p m32 -U flash:w:main.hex
```
