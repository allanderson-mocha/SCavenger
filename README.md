# The SCavenger

An interactive, portable adventure journal that combines puzzle-solving with physical movement to promote healthier habits and social play. Developed for USC's EE 459 Embedded Systems Design Lab in collaboration with Otis College of Art and Design.

## 🧭 Overview

**The SCavenger** reimagines relaxation and entertainment through a gamified experience. Players advance through six unique puzzles using real-world actions, engaging sensors and interacting with a microcontroller-driven system. The end reward? Unlocking a secret compartment with a meaningful item left by a previous player.

## 🛠️ Features

- 6 unique physical and cognitive puzzles
- Sensor-based interactions (light, motion, altitude, GPS)
- Narrative progression via LCD and audio
- Hint system and replayable experience
- Secret final reward with solenoid-activated compartment

## 🧩 Puzzles Breakdown

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 1        | TSL2591 Light Sensor       | Adjust light levels                        |
| 2        | MPL3115A2 + LEDs           | Decode Morse code and raise temperature    |
| 3        | MPL3115A2 Altitude Sensor  | Increase elevation (stairs, hills)         |
| 4        | LIS3DH Accelerometer       | Reach step count goal                      |
| 5        | MTK3339 GPS Module         | Navigate to new coordinates                |
| 6        | Solenoid                   | Unlock final secret compartment            |

## 🔌 Hardware Architecture

- **Microcontroller:** ATmega328P
- **Sensors:**
  - TSL2591 (Light)
  - LIS3DH (Accelerometer)
  - MPL3115A2 (Altitude & Temperature)
  - MTK3339 (GPS via UART)
- **Output:**
  - 20x4 LCD (Parallel)
  - Buzzer (PWM)
  - LEDs (Morse code)
  - Solenoid (via 2N2222 NPN transistor)
- **Power:**
  - 4x AA batteries stepped down via buck converter (5V)
  - 3.3V rail and level shifter for ESP32-C3 (planned feature)

## 🧠 Software Structure

- **Language:** C (AVR-GCC)
- **Architecture:** Event-driven main loop with finite state machine
- **Game States:**
  - `DIALOGUE`: Narrative scenes with typewriter-style LCD output
  - `PUZZLE`: Sensor-driven challenges
- **Persistence:** Game progress stored in EEPROM
- **Input:** Debounced button interrupts (Power, Clue, Back, Next)
- **Display:** Character-based LCD with PROGMEM-backed narrative text
- **Hints:** Up to 3 hints per puzzle accessible via the clue button

## 🚧 Known Issues

- **ESP32-C3 WiFi functionality** was not operational at demo time due to I2C communication bugs.
- GPS parsing occasionally overloaded RAM; flash-based dialogue strings helped mitigate this.

## 📦 Build & Flash

> **Note:** This project uses the ATmega328P. You’ll need an AVR toolchain and ISP programmer.

```bash
# Compile
avr-gcc -mmcu=atmega328p -Os main.c -o scavenger.elf

# Convert to HEX
avr-objcopy -O ihex -R .eeprom scavenger.elf scavenger.hex

# Flash
avrdude -c usbtiny -p m328p -U flash:w:scavenger.hex
