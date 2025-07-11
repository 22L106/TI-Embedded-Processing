# 8051-Based 3-Digit Counter with EEPROM and 7-Segment Display

This project implements a persistent 3-digit decimal counter (000–999) using an 8051 microcontroller. The current count is stored in an external I2C EEPROM and is displayed on three multiplexed 7-segment displays. A push button is used to increment the counter, and the display refresh is handled via Timer0 interrupts.

---

## 🔧 Features

- 3-digit counter (000 to 255)
- Non-volatile storage using I2C EEPROM (24C02)
- Multiplexed display using Timer0 interrupts
- Push-button based counter increment
- EEPROM-backed power-fail retention

---

## 🛠️ Hardware Requirements

| Component         | Description                     |
|------------------|---------------------------------|
| Microcontroller  | AT89C51 / AT89S52 (8051 Core)   |
| EEPROM            | 24C02 (I2C-compatible)          |
| 7-Segment Display | 3-digit, common cathode         |
| Push Button       | Active-low input                |
| Resistors         | Pull-up and current-limiting    |
| Power Supply      | +5V DC                          |

---

## 🧩 Pin Connections

| 8051 Pin | Connected To                |
|----------|-----------------------------|
| P1.0–P1.7 | Segment lines (a–g, dp)     |
| P2.0     | Digit 1 Select (Units)      |
| P2.1     | Digit 2 Select (Tens)       |
| P2.2     | Digit 3 Select (Hundreds)   |
| P0.0     | I2C SDA (EEPROM Data)       |
| P0.1     | I2C SCL (EEPROM Clock)      |
| P0.7     | Push Button (Active Low)    |



---

## 🧠 How It Works

1. On startup, the counter value is read from EEPROM.
2. Timer0 is configured in Mode 1 to generate periodic interrupts.
3. Each interrupt refreshes one of the three digits (multiplexing).
4. A push-button increments the counter and updates EEPROM.
5. The counter wraps from 999 → 000.


---

## ⏱️ Timer Configuration

- Timer0 Mode 0 (13-bit)

---

## 🔄 EEPROM Storage Format

-  Byte of count stored at EEPROM address `0x00`

---


## 👨‍💻 Author

**Akshay Muthu Shankar G**  
Project: I2C EEPROM-based persistent counter with 8051 MCU


