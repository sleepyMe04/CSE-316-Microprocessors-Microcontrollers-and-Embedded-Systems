#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// ─── Debug ───────────────────────────────────────────────
#define DEBUG  // Comment out to disable Serial debug logs

#ifdef DEBUG
  #define LOG(x)   Serial.println(x)
  #define LOGF(x)  Serial.print(x)
#else
  #define LOG(x)
  #define LOGF(x)
#endif

// ─── Pin Definitions ─────────────────────────────────────
#define RST_PIN      9
#define SS_PIN       10
#define TRIG_PIN     7
#define ECHO_PIN     6
#define SERVO_PIN    5
#define BTN_DISPENSE A0
#define BTN_READ     A1
#define BTN_WRITE    A2

// ─── Constants ───────────────────────────────────────────
const byte    BLOCK           = 2;
const unsigned long ITEM_COST = 20;
const unsigned long MAX_BALANCE = 10000;
const unsigned long DISPENSE_TIMEOUT_MS = 5000;
const unsigned long DEBOUNCE_MS = 50;
const int     STOCK_MIN_CM    = 2;
const int     STOCK_MAX_CM    = 25;

// ─── Hardware ────────────────────────────────────────────
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
LiquidCrystal_I2C lcd(0x3F, 16, 2);
Servo dispenser;

// ─── State ───────────────────────────────────────────────
unsigned long balance     = 0;
long          cm          = 0;
bool          balanceChanged = true;  // force first LCD update

// Stored UID of the card that loaded the current balance
byte  sessionUID[10];
byte  sessionUIDLen = 0;
bool  sessionActive = false;

// ─────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────

void lcdMsg(const char* line1, const char* line2 = nullptr, int waitMs = 2000) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  if (waitMs > 0) delay(waitMs);
}

bool debounceButton(byte pin) {
  if (digitalRead(pin) == LOW) {
    delay(DEBOUNCE_MS);
    return digitalRead(pin) == LOW;
  }
  return false;
}

void numberToBytes(unsigned long num, byte s[16]) {
  for (int i = 0; i < 4; i++) {
    s[i] = (byte)(num >> (i * 8));
  }
}

unsigned long bytesToNumber(byte s[]) {
  unsigned long num = 0;
  for (int i = 3; i >= 0; i--) {
    num = (num << 8) | s[i];
  }
  return num;
}

void sonarUpdate() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  cm = duration * 0.034 / 2;
}

// ─────────────────────────────────────────────────────────
//  RFID
// ─────────────────────────────────────────────────────────

bool scanCard() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false;
  return true;
}

// Returns true if the scanned card matches the session UID
bool verifyUID() {
  if (!sessionActive) return true;  // no session to verify against
  if (rfid.uid.size != sessionUIDLen) return false;
  for (byte i = 0; i < sessionUIDLen; i++) {
    if (rfid.uid.uidByte[i] != sessionUID[i]) return false;
  }
  return true;
}

void saveSessionUID() {
  sessionUIDLen = rfid.uid.size;
  for (byte i = 0; i < sessionUIDLen; i++) {
    sessionUID[i] = rfid.uid.uidByte[i];
  }
  sessionActive = true;
}

void clearSession() {
  sessionActive = false;
  sessionUIDLen = 0;
}

int readBlock(byte blockNumber, byte arrayAddress[]) {
  int sectorTrailer = (blockNumber / 4) * 4 + 3;

  MFRC522::StatusCode status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, sectorTrailer, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    LOG("Auth failed (read)");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return 1;
  }

  byte buffersize = 18;
  status = rfid.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (status != MFRC522::STATUS_OK) {
    LOG("Read failed");
    return 2;
  }
  return 0;
}

int writeBlock(byte blockNumber, byte arrayAddress[]) {
  int sectorTrailer = (blockNumber / 4) * 4 + 3;

  MFRC522::StatusCode status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, sectorTrailer, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    LOG("Auth failed (write)");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return 1;
  }

  status = rfid.MIFARE_Write(blockNumber, arrayAddress, 16);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (status != MFRC522::STATUS_OK) {
    LOG("Write failed");
    return 2;
  }
  return 0;
}

// ─────────────────────────────────────────────────────────
//  Dispenser
// ─────────────────────────────────────────────────────────

bool dispenseSnack() {
  dispenser.attach(SERVO_PIN);
  dispenser.write(0);  // open gate

  unsigned long start = millis();
  sonarUpdate();

  while ((cm > STOCK_MAX_CM || cm < STOCK_MIN_CM) &&
         (millis() - start < DISPENSE_TIMEOUT_MS)) {
    sonarUpdate();
    delay(100);
  }

  dispenser.write(90);  // close gate
  delay(500);
  dispenser.detach();

  if (millis() - start >= DISPENSE_TIMEOUT_MS) {
    LOG("Dispense timeout — possible jam");
    return false;  // jammed or failed
  }
  return true;
}

// Write current balance back to card (auto-save after dispense)
bool writeBalanceToCard() {
  if (!scanCard()) return false;
  if (!verifyUID()) {
    LOG("UID mismatch — write aborted");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return false;
  }
  byte writeData[16] = {0};
  numberToBytes(balance, writeData);
  return (writeBlock(BLOCK, writeData) == 0);
}

// ─────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BTN_DISPENSE, INPUT_PULLUP);
  pinMode(BTN_READ,     INPUT_PULLUP);
  pinMode(BTN_WRITE,    INPUT_PULLUP);

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  lcdMsg(" Vending Machine", "  Ready to use  ", 2000);
  lcd.clear();
  LOG("System ready.");
}

// ─────────────────────────────────────────────────────────
//  Main Loop
// ─────────────────────────────────────────────────────────

void loop() {

  // ── Update balance display only when it changes ──────
  if (balanceChanged) {
    lcd.setCursor(0, 0);
    lcd.print("Balance:        ");
    lcd.setCursor(9, 0);
    lcd.print(balance);
    lcd.print(" Tk ");

    // Low balance warning on row 2
    if (balance < ITEM_COST && balance > 0) {
      lcd.setCursor(0, 1);
      lcd.print("Low balance!    ");
    } else if (balance == 0 && !sessionActive) {
      lcd.setCursor(0, 1);
      lcd.print("Tap card to load");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    balanceChanged = false;
  }

  // ── READ button — load balance from card ─────────────
  if (debounceButton(BTN_READ)) {
    lcdMsg("Reading Card...", nullptr, 1500);
    if (scanCard()) {
      saveSessionUID();
      byte data[18];
      if (readBlock(BLOCK, data) == 0) {
        unsigned long val = bytesToNumber(data);

        // Sanity check
        if (val > MAX_BALANCE) {
          lcdMsg("Invalid card!", "Balance too high", 2000);
          LOG("Rejected: balance overflow");
          clearSession();
        } else {
          balance += val;
          if (balance > MAX_BALANCE) balance = MAX_BALANCE;

          // Zero out card after reading
          byte zeroData[16] = {0};
          // re-scan to write zero (card was halted after read)
          if (scanCard()) writeBlock(BLOCK, zeroData);

          LOGF("Loaded: "); LOG(val);
          LOGF("Total:  "); LOG(balance);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Added: ");
          lcd.print(val);
          lcd.print(" Tk");
          lcd.setCursor(0, 1);
          lcd.print("Total: ");
          lcd.print(balance);
          lcd.print(" Tk");
          delay(2000);
          balanceChanged = true;
        }
      } else {
        lcdMsg("Read Failed!", nullptr, 2000);
        clearSession();
      }
    } else {
      lcdMsg("No card found!", nullptr, 2000);
    }
    lcd.clear();
    balanceChanged = true;
  }

  // ── WRITE button — save balance back to card ─────────
  if (debounceButton(BTN_WRITE)) {
    lcdMsg("Present card...", nullptr, 1000);
    if (scanCard()) {
      if (!verifyUID()) {
        lcdMsg("Wrong card!", "Use same card", 2000);
        LOG("UID mismatch on write");
      } else {
        byte writeData[16] = {0};
        numberToBytes(balance, writeData);
        if (writeBlock(BLOCK, writeData) == 0) {
          balance = 0;
          clearSession();
          lcdMsg("Saved to card!", "Balance: 0 Tk", 2000);
          LOG("Write successful");
        } else {
          lcdMsg("Write Failed!", "Try again", 2000);
        }
      }
    } else {
      lcdMsg("No card found!", nullptr, 2000);
    }
    lcd.clear();
    balanceChanged = true;
  }

  // ── DISPENSE button ───────────────────────────────────
  if (debounceButton(BTN_DISPENSE)) {
    lcdMsg("Checking stock...", nullptr, 500);
    sonarUpdate();

    bool inRange = (cm > STOCK_MAX_CM || cm < STOCK_MIN_CM);

    if (!inRange) {
      lcdMsg("Stock Empty!", "Refill needed", 2000);
      LOG("No stock detected");
    } else if (balance < ITEM_COST) {
      lcdMsg("Insufficient Tk!", "Tap card to load", 2000);
      LOG("Insufficient balance");
    } else {
      balance -= ITEM_COST;
      lcdMsg("Dispensing...", nullptr, 0);

      bool ok = dispenseSnack();

      if (ok) {
        // Auto-save updated balance to card
        lcdMsg("Saving balance..", nullptr, 500);
        if (sessionActive) {
          bool saved = writeBalanceToCard();
          if (saved) {
            LOGF("Auto-saved balance: "); LOG(balance);
          } else {
            LOG("Auto-save failed — user must press Write");
          }
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enjoy your snack");
        lcd.setCursor(0, 1);
        lcd.print("Bal: ");
        lcd.print(balance);
        lcd.print(" Tk");
        delay(2000);

        // Log transaction
        LOGF("[TX] Dispensed item. Cost: ");
        LOGF(ITEM_COST);
        LOGF(" Remaining: ");
        LOG(balance);

      } else {
        // Dispense failed / jammed — refund
        balance += ITEM_COST;
        lcdMsg("Jam detected!", "Please get help", 2000);
        LOG("Dispense failed — balance refunded");
      }
    }
    lcd.clear();
    balanceChanged = true;
  }
}
