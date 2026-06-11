# CSE 316 — Microprocessors, Microcontrollers and Embedded Systems



This repository contains lab experiments, an online assessment, and a term project for CSE 316.

---

## Repository Structure

```
CSE-316-Microprocessors-Microcontrollers-and-Embedded-Systems/
│
├── ATMEGA 32 LED Matrix/                           # Experiment 1
│   ├── main.c                                      # ATmega32 C source
│   ├── Experiment 1 (LED Matrix).pdf               # Lab sheet
│   └── readme.md
│
├── Basic use of ADC and LCD module with ATmega32/  # Experiment 2
│   ├── main.c                                      # ATmega32 C source
│   ├── lcd.h                                       # Custom 4-bit LCD driver
│   ├── January 2025 CSE 316 Experiment 2.pdf       # Lab sheet
│   └── readme.md
│
├── Online/                                         # Online assessment
│   ├── 2105114.asm                                 # x86 assembly source
│   ├── Online B2.pdf                               # Problem sheet
│   └── readme.md
│
├── Term Project-Vending Machine/                   # Term project
│   ├── vending_machine.ino                         # Arduino sketch
│   └── README (1).md
│
└── README.md
```

---

## Contents at a Glance

| Folder | Platform | Topic |
|--------|----------|-------|
| [ATMEGA 32 LED Matrix](./ATMEGA%2032%20LED%20Matrix/) | ATmega32 (AVR C) | 8×8 LED matrix with interrupt-driven scroll & shift animations |
| [Basic use of ADC and LCD module with ATmega32](./Basic%20use%20of%20ADC%20and%20LCD%20module%20with%20ATmega32/) | ATmega32 (AVR C) | ADC voltage reading displayed live on a 16×2 LCD |
| [Online](./Online/) | x86 Assembly (8086 / DOS) | Recursive digit-sum via stack-based near procedure |
| [Term Project-Vending Machine](./Term%20Project-Vending%20Machine/) | Arduino Uno (ATmega328P) | RFID smart vending machine with servo dispensing & ultrasonic stock detection |
