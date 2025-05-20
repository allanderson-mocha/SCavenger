# The SCavenger

An interactive, portable adventure journal that combines puzzle-solving with physical movement to promote healthier habits and social play. Developed for USC's EE 459 Embedded Systems Design Lab in collaboration with Otis College of Art and Design. This project idea was decided by our group of five students to create a health and fitness oriented product. The development of the project was completely independent and faculty was only asked for advice or references.

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

https://github.com/user-attachments/assets/b753697f-50ba-4efc-b494-88a1aad260bc

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 2        | MPL3115A2 + LEDs           | Decode Morse code and raise temperature    |

https://github.com/user-attachments/assets/062caf2c-1c44-411f-9714-7465a4922e33

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 3        | MPL3115A2 Altitude Sensor  | Increase elevation (stairs, hills)         |

https://github.com/user-attachments/assets/7e5139e6-bc53-4f6a-b0c3-46b90cc21e2a

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 4        | LIS3DH Accelerometer       | Reach step count goal                      |

https://github.com/user-attachments/assets/5d389911-7984-437a-baf5-a36a9e93f9ed

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 5        | MTK3339 GPS Module         | Navigate to new coordinates                |

https://github.com/user-attachments/assets/9cc2e4c8-a230-4214-a0b0-255597aed21e

| Puzzle # | Sensor(s) Used            | Interaction                                |
|----------|----------------------------|--------------------------------------------|
| 6        | Solenoid                   | Unlock final secret compartment            |

https://github.com/user-attachments/assets/a6d7a647-325b-4db5-96e8-d265733203f4



## 🔌 Hardware Architecture

- **Microcontrollers:**
  - ATmega328P
  - ESP32-C3-MINI-1
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
  - 3.3V rail and level shifter for ESP32-C3  (built but not used in demo)

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
- **Wi-Fi:** Access point and station mode functionalities
- **APIs:** Use of OverPass API for nearby coordinates of interest

## 🚧 Known Issues

- **ESP32-C3 WiFi functionality** was not operational at demo time due to I2C communication bugs.
- GPS parsing occasionally overloaded RAM; flash-based dialogue strings helped mitigate this.

## 📝 TO-DO for Continued Development

- Debug ESP32 I2C interface to successfully interact with ATmega328P.
- Improve user experience when accessing the ESP32 AP IP or API form web page.
- Transfer system to PCB.
- Manufacture casing for product.

## 📦 Build & Flash

> **Note:** This project uses the ATmega328P. You’ll need an AVR toolchain and ISP programmer.

```bash
# Compile
avr-gcc -mmcu=atmega328p -Os main.c -o scavenger.elf

# Convert to HEX
avr-objcopy -O ihex -R .eeprom scavenger.elf scavenger.hex

# Flash
avrdude -c usbtiny -p m328p -U flash:w:scavenger.hex
