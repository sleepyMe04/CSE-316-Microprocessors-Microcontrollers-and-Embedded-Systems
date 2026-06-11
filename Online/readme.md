
# Online Assessment — Recursive Digit Sum (x86 Assembly)


---

## Overview

An **x86 16-bit assembly** program (8086 / DOS `.model small`) that computes the
**sum of digits** of an integer recursively and prints the result to the console.

For the hardcoded input `23126`:
```
2 + 3 + 1 + 2 + 6 = 14
```

---

## Program Structure

### `main`
- Pushes `23126` onto the stack and calls `sum`
- Passes the returned value in `AX` to `print_number`
- Exits via `INT 21h / AH=4Ch`

### `sum` — near recursive procedure
| Step | Action |
|------|--------|
| Base case | Argument == 0 → return 0 in `AX` |
| Divide | `AX ÷ 10` → quotient in `AX`, last digit in `DX` |
| Recurse | Push quotient; call `sum` |
| Combine | `AX` (recursive result) `+= DX` (saved digit) |
| Clean-up | `ret 2` — callee pops its one pushed argument word |

### `print_number`
- Repeatedly divides `AX` by 10, pushing remainders (digits) onto the stack
- Pops each digit, converts to ASCII (`+ '0'`), prints with `INT 21h / AH=02h`

---

## Files

| File | Description |
|------|-------------|
| `2105114.asm` | MASM-compatible x86 assembly source |
| `Online B2.pdf` | Original assessment problem sheet |

---

## Build & Run

**MASM / TASM (DOSBox or real DOS):**
```
masm 2105114.asm;
link 2105114.obj;
2105114.exe
```

**emu8086:** Open `2105114.asm` directly and click Run.
