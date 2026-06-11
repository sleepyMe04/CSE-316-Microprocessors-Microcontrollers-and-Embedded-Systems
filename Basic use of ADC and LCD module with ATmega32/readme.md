
# Experiment 2 — ADC Voltage Reader with 16×2 LCD (ATmega32)



## Overview

Reads an analog voltage (e.g. from a potentiometer) via the ATmega32's built-in
**8-bit ADC** and displays the result as `X.X V` on a **16×2 LCD in 4-bit parallel mode**,
updating continuously in a loop.

---

## Hardware Connections

### LCD (4-bit mode)

| LCD Signal | ATmega32 Pin |
|------------|--------------|
| D4 | PD4 |
| D5 | PD5 |
| D6 | PD6 |
| D7 | PD7 |
| RS | PC6 |
| EN | PC7 |

### ADC Input

| Signal | ATmega32 Pin |
|--------|--------------|
| Analog input (e.g. potentiometer wiper) | PA0 (ADC0) |
| Reference | AVCC = 5 V |

CPU clock: **1 MHz** (`F_CPU 1000000`)

> JTAG is disabled at startup (writing `JTD` twice) to free PC4–PC7 for LCD use.

---

## How It Works

1. **ADC setup**
   - `ADCSRA = 0b10000111` — ADC enabled, prescaler ÷128
   - `ADMUX  = 0b01100000` — AVCC reference, left-adjusted result (`ADLAR=1`), channel ADC0

2. **Conversion** — `ADSC` starts a conversion; firmware busy-waits on `ADIF`

3. **Scaling** — only `ADCH` is read (top 8 bits of the 10-bit result):
   ```
   tenths = (ADCH × 4 × 5) / 102   → voltage in 0.1 V steps (0–50)
   volts  = tenths / 10
   frac   = tenths % 10
   ```

4. **Display** — formatted as `"V.F V"` via `sprintf`, written to LCD row 1 col 6

---

## Files

| File | Description |
|------|-------------|
| `main.c` | ATmega32 C source — ADC read + LCD display loop |
| `lcd.h` | Custom LCD driver for 4-bit parallel mode |
| `January 2025 CSE 316 Experiment 2.pdf` | Official lab sheet and circuit diagram |

---

## Build & Flash

```bash
avr-gcc -mmcu=atmega32 -DF_CPU=1000000UL -O2 -Wall -o main.elf main.c
avr-objcopy -O ihex main.elf main.hex
avrdude -c usbasp -p m32 -U flash:w:main.hex
```
