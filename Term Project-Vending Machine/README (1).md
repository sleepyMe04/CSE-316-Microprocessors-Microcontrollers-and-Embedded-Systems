#  RFID-Based Smart Vending Machine

> **Course:** CSE 316 — Microprocessors, Microcontrollers and Embedded Systems  
> **Institution:** Bangladesh University of Engineering and Technology (BUET)  
> **Project Type:** Hardware + Firmware

---

##  Overview

An automated vending machine that uses **RFID cards as digital wallets**. Balance is stored directly on the card (Mifare Classic), loaded into the machine's RAM for a session, and written back after purchase — no central server or database required.

The machine uses an **ultrasonic sensor** to verify stock before dispensing and a **servo motor** to physically release items. An I2C LCD provides real-time feedback throughout.

---

##  Features

-  **Card-as-wallet** — balance stored on RFID card (block 2), zeroed after loading
-  **UID session locking** — write-back is only allowed from the same card that loaded the balance
-  **Overflow protection** — rejects corrupted cards with abnormally high balances
-  **Ultrasonic stock detection** — prevents dispensing when stock is empty
-  **Jam detection with timeout** — servo closes and balance is refunded if dispensing fails
-  **Auto-save after purchase** — balance is written back to card automatically post-dispense
- **Button debouncing** — prevents accidental double-presses
-  **Low balance warning** — LCD alerts when balance drops below item cost
-  **Debug mode** — toggle Serial logging with a single `#define`

---

## 🔧 Hardware Components

| Component | Model / Spec | Purpose |
|---|---|---|
| Microcontroller | Arduino Uno (ATmega328P) | Main controller |
| RFID Reader | MFRC522 | Read/write RFID cards |
| RFID Cards | Mifare Classic 1K | Store user balance |
| LCD Display | 16×2 I2C (address `0x3F`) | User interface |
| Ultrasonic Sensor | HC-SR04 | Stock level detection |
| Servo Motor | SG90 / MG995 | Snack dispensing mechanism |
| Push Buttons | Tactile (×3) | Read / Write / Dispense |
| Power Supply | 5V (USB or adapter) | Arduino + peripherals |

---

##  Circuit Diagram

### Pin Connections

| Arduino Pin | Connected To |
|---|---|
| `D9` | MFRC522 RST |
| `D10` | MFRC522 SDA (SS) |
| `D11` | MFRC522 MOSI (SPI) |
| `D12` | MFRC522 MISO (SPI) |
| `D13` | MFRC522 SCK (SPI) |
| `D5` | Servo Signal |
| `D6` | HC-SR04 ECHO |
| `D7` | HC-SR04 TRIG |
| `A0` | Button — Dispense |
| `A1` | Button — Read Card |
| `A2` | Button — Write Card |
| `A4` | LCD SDA (I2C) |
| `A5` | LCD SCL (I2C) |
| `5V / GND` | Power to all modules |

> **Note:** MFRC522 must be powered at **3.3V**, not 5V — connect its VCC to Arduino's 3.3V pin.

---

##  Libraries Required

Install these from the Arduino Library Manager (`Sketch → Include Library → Manage Libraries`):

| Library | Version | Purpose |
|---|---|---|
| `MFRC522` by GithubCommunity | ≥ 1.4.10 | RFID reader |
| `LiquidCrystal_I2C` by Frank de Brabander | ≥ 1.1.2 | I2C LCD |
| `Servo` | Built-in | Servo motor control |
| `Wire` | Built-in | I2C communication |
| `SPI` | Built-in | SPI communication |

---

##  Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/SleepyMe04/CSE-316-Microprocessors-Microcontrollers-and-Embedded-Systems.git
cd CSE-316-Microprocessors-Microcontrollers-and-Embedded-Systems
```

### 2. Open in Arduino IDE

Open `vending_machine.ino` in the Arduino IDE (v1.8+ or v2.x).

### 3. Install Libraries

Install all libraries listed above via Library Manager.

### 4. Configure LCD Address

If your LCD doesn't show anything, the I2C address may differ. Change this line:

```cpp
LiquidCrystal_I2C lcd(0x3F, 16, 2);
//                    ^^^^
//                    Try 0x27 if 0x3F doesn't work
```

Run an [I2C scanner sketch](https://playground.arduino.cc/Main/I2cScanner/) to find your LCD's address.

### 5. Upload

Select your board (`Tools → Board → Arduino Uno`) and the correct port, then upload.

---

##  How to Use

### Loading Balance onto the Machine

1. Place your RFID card on the reader
2. Press **BTN_READ (A1)**
3. The machine reads the balance from the card, adds it to the session balance, and zeros out the card

### Buying a Snack

1. Ensure balance is loaded (see above)
2. Press **BTN_DISPENSE (A0)**
3. The machine checks stock via ultrasonic sensor
4. If stock is present and balance ≥ 20 Tk, the servo dispenses and balance is deducted
5. Updated balance is automatically written back to your card

### Saving Balance Back to Card

1. Press **BTN_WRITE (A2)** and hold your card on the reader
2. Remaining session balance is written back to the card
3. Session balance resets to 0

>  Always press **Write** before removing your card if you didn't auto-save, or your remaining balance will be lost when the machine powers off.

---

##  Configuration

All key parameters are defined at the top of the sketch for easy tuning:

```cpp
const byte         BLOCK              = 2;      // RFID data block
const unsigned long ITEM_COST         = 20;     // Cost per item in Tk
const unsigned long MAX_BALANCE       = 10000;  // Max allowed balance (Tk)
const unsigned long DISPENSE_TIMEOUT  = 5000;   // Jam timeout (ms)
const int           STOCK_MIN_CM      = 2;      // Min sonar distance (stock present)
const int           STOCK_MAX_CM      = 25;     // Max sonar distance (stock present)
```

### Enabling / Disabling Debug Logs

```cpp
#define DEBUG   // Comment this out to disable all Serial.println() output
```

---

##  System Flow

```
Power On
    │
    ▼
Display "Vending Machine — Ready"
    │
    ▼
┌─────────────────────────────────────┐
│            Main Loop                │
│                                     │
│  [BTN_READ]  ──► Read card balance  │
│                  Add to session     │
│                  Zero card          │
│                                     │
│  [BTN_WRITE] ──► Verify card UID    │
│                  Write balance back │
│                  Clear session      │
│                                     │
│  [BTN_DISPENSE]                     │
│      │                              │
│      ├─ Check sonar (stock?)        │
│      ├─ Check balance ≥ ITEM_COST   │
│      ├─ Deduct cost                 │
│      ├─ Run servo                   │
│      ├─ Wait for item to fall       │
│      ├─ Timeout? → Refund + alert   │
│      └─ Auto-write balance to card  │
└─────────────────────────────────────┘
```

---

##  Known Limitations

- **Single item type** — only one price point (`ITEM_COST`) is supported
- **No PIN / authentication** — anyone who presents the card can use its balance
- **Blocking delays** — uses `delay()` and `pulseIn()` which block the CPU; a future version could use interrupts and non-blocking timers
- **Auto-write on dispense requires card to still be present** — if user removes card before auto-save completes, they must press BTN_WRITE manually on next visit



## Repository Structure

```
CSE-316-Microprocessors-Microcontrollers-and-Embedded-Systems/
│
├── vending_machine/
│   └── vending_machine.ino       # Main Arduino sketch
│
└── README.md                     # This file
```

---

