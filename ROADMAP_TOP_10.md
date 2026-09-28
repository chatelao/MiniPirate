# Top 10 Most Popular Maker Boards - Support Roadmap

This document outlines the comprehensive support roadmap, hardware architecture adaptations, toolchain setup, continuous integration (CI) workflows, and automated release asset pipelines for the **Top 10 Most Popular Maker Development Boards** in MiniPirate.

---

## 1. Overview & Objectives

MiniPirate aims to provide a unified, plug-and-play interactive hardware terminal across the most widely used microcontroller platforms in the maker and embedded engineering communities. By supporting these top 10 maker board families, MiniPirate delivers direct GPIO, I2C, PWM, ADC, clock generation, and diagnostic capabilities without requiring users to write or recompile code for basic hardware testing.

**Key Objectives:**
- Maintain multi-architecture compatibility across 8-bit AVR, 32-bit ARM Cortex-M0+/M4/M7, RISC-V, and Xtensa architectures.
- Standardize hardware diagnostic commands (`v` VCC, `t` temperature, `f` free RAM/heap, `u` uptime).
- Adapt non-volatile memory persistence across physical EEPROM and flash-emulated EEPROM / NVS.
- Provide automated CI build matrix verification and multi-format release binary assets (`.hex`, `.bin`, `.uf2`).

---

## 2. Top 10 Maker Board Matrix

| # | Board Family | Architecture / Microcontroller | Package Index / Core | Example FQBN | Output Format | Status |
|---|---|---|---|---|---|---|
| **1** | **Arduino Uno (R3 / R4)** | 8-bit AVR (ATmega328P) / 32-bit ARM Cortex-M4 (RA4M1) | `arduino:avr` / `arduino:renesas_uno` | `arduino:avr:uno`, `arduino:renesas_uno:minima` | `.hex` / `.bin` | Supported |
| **2** | **Raspberry Pi Pico & Pico 2** | Dual ARM Cortex-M0+ (RP2040) / Dual ARM Cortex-M33 & Hazard3 RISC-V (RP2350) | `rp2040:rp2040` (Earle Philhower Core) | `rp2040:rp2040:rpipico`, `rp2040:rp2040:rpipico2` | `.uf2` | Supported |
| **3** | **ESP32 DevKit / S3 / C3** | Dual-core Xtensa LX6 / LX7 / RISC-V (ESP32-C3) | `esp32:esp32` | `esp32:esp32:esp32`, `esp32:esp32:esp32s3`, `esp32:esp32:esp32c3` | `.bin` | Planned |
| **4** | **Seeed Studio XIAO Series** | SAMD21, RP2040, RP2350, RA4M1, ESP32-C3, ESP32-S3 | `Seeeduino:samd`, `rp2040:rp2040`, `Seeeduino:renesas_uno`, `esp32:esp32` | `rp2040:rp2040:seeed_xiao_rp2040`, `Seeeduino:renesas_uno:XIAO_RA4M1` | `.uf2` / `.bin` | Supported |
| **5** | **ESP8266 NodeMCU & D1 Mini** | 32-bit RISC Xtensa L106 (ESP8266EX) | `esp8266:esp8266` | `esp8266:esp8266:nodemcuv2`, `esp8266:esp8266:d1_mini` | `.bin` | Planned |
| **6** | **STM32 Nucleo & Pill Series** | ARM Cortex-M3 / M4 (STM32F103, STM32F401, STM32G431, STM32F446) | `STMicroelectronics:stm32` | `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB`, `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE` | `.bin` / `.hex` | Supported |
| **7** | **Arduino Nano Family** | 8-bit AVR (ATmega328P, ATmega4809) / ESP32-S3 / nRF52840 | `arduino:avr`, `arduino:megaavr`, `arduino:mbed_nano` | `arduino:avr:nano`, `arduino:megaavr:nanaofresh` | `.hex` / `.bin` | Supported |
| **8** | **Teensy 4.0 / 4.1** | 600 MHz ARM Cortex-M7 (NXP i.MXRT1062) | `teensy:avr` (Teensyduino Core) | `teensy:avr:teensy40`, `teensy:avr:teensy41` | `.hex` / `.hex` | Planned |
| **9** | **Adafruit Feather Series** | ARM Cortex-M4 (SAMD51), RP2040, ESP32-S2/S3 | `adafruit:samd`, `rp2040:rp2040`, `esp32:esp32` | `adafruit:samd:adafruit_feather_m4` | `.uf2` / `.bin` | Planned |
| **10**| **BBC micro:bit v2 / nRF52** | 64 MHz ARM Cortex-M4F (Nordic nRF52833 / nRF52840) | `sandeepmistry:nRF5`, `adafruit:nrf52` | `sandeepmistry:nRF5:BBCmicrobitV2` | `.hex` / `.uf2` | Planned |

---

## 3. Detailed Board Architectures & Toolchain Setup

### 1. Arduino Uno (R3 & R4 Minima / WiFi)
- **Microcontrollers:** ATmega328P (8-bit AVR) & Renesas RA4M1 (32-bit Cortex-M4F).
- **Core Indexes:**
  - AVR: Native Arduino Core
  - R4: `https://downloads.arduino.cc/packages/package_index.json` (`arduino:renesas_uno`)
- **Key Characteristics:** 14 Digital I/O pins, 6 Analog inputs, hardware I2C (`Wire`).
- **NVM Storage:** Standard internal EEPROM (`EEPROM.h`).

### 2. Raspberry Pi Pico & Pico 2 (RP2040 / RP2350)
- **Microcontrollers:** RP2040 (Dual Cortex-M0+) & RP2350 (Dual Cortex-M33 / RISC-V).
- **Core Index:** `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **FQBNs:** `rp2040:rp2040:rpipico`, `rp2040:rp2040:rpipico2`
- **Key Characteristics:** Flexible PIO, hardware I2C on multiple pin pairs, high-speed PWM on all GPIOs.
- **NVM Storage:** Emulated EEPROM in Flash memory.
- **Diagnostics:** `analogReadTemp()` for CPU temperature, `rp2040.getFreeHeap()` for free memory.

### 3. ESP32 DevKit / S3 / C3
- **Microcontrollers:** ESP32 (Xtensa Dual-Core), ESP32-S3 (Xtensa Dual-Core + Vector extensions), ESP32-C3 (RISC-V Single-Core).
- **Core Index:** `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- **FQBNs:** `esp32:esp32:esp32`, `esp32:esp32:esp32s3`, `esp32:esp32:esp32c3`
- **Key Characteristics:** Wi-Fi/Bluetooth integration, hardware I2C, LEDC PWM driver.
- **NVM Storage:** EEPROM library mapped to Flash NVS.
- **Diagnostics:** `temperatureRead()`, `ESP.getFreeHeap()`, `ESP.getVdd()` (or calibration API).

### 4. Seeed Studio XIAO Series
- **Variants:** XIAO SAMD21, XIAO RP2040, XIAO RP2350, XIAO RA4M1, XIAO ESP32-C3, XIAO ESP32-S3.
- **Core Indexes:**
  - Seeed Renesas: `https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
  - Earle Philhower RP2040/RP2350: `package_rp2040_index.json`
  - Espressif ESP32: `package_esp32_index.json`
- **FQBNs:** `rp2040:rp2040:seeed_xiao_rp2040`, `rp2040:rp2040:seeed_xiao_rp2350`, `Seeeduino:renesas_uno:XIAO_RA4M1`
- **Key Characteristics:** Ultra-compact form factor (14 pins), header-compatible pinout.

### 5. ESP8266 NodeMCU & D1 Mini
- **Microcontroller:** ESP8266EX (Xtensa L106 @ 80/160 MHz).
- **Core Index:** `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
- **FQBNs:** `esp8266:esp8266:nodemcuv2`, `esp8266:esp8266:d1_mini`
- **Key Characteristics:** Single analog input pin (A0, max 1.0V/3.3V with board divider), software PWM (`analogWrite`).
- **NVM Storage:** Emulated EEPROM in Flash.
- **Diagnostics:** `ESP.getFreeHeap()`, `ESP.getVcc()`.

### 6. STM32 Nucleo & Blue/Black Pill Series
- **Microcontrollers:** STM32F103C8T6 (Blue Pill), STM32F401/F411, STM32G431RB, STM32F446RE.
- **Core Index:** `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
- **FQBNs:** `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB`, `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE`
- **Key Characteristics:** High pin counts, multiple hardware I2C/SPI/UART peripherals, 12-bit ADCs.
- **Diagnostics:** Standard STM32 LL macros (`__LL_ADC_CALC_VREFANALOG_VOLTAGE`, `__LL_ADC_CALC_TEMPERATURE`), `mallinfo().fordblks` for free SRAM.

### 7. Arduino Nano Family
- **Variants:** Nano Classic (ATmega328P), Nano Every (ATmega4809), Nano ESP32, Nano 33 BLE.
- **Core Indexes:** Standard Arduino core package index.
- **FQBNs:** `arduino:avr:nano`, `arduino:megaavr:nanaofresh`
- **Key Characteristics:** Compact breadboard-friendly form factor with classic pinout mapping.

### 8. Teensy 4.0 / 4.1
- **Microcontroller:** NXP i.MXRT1062 ARM Cortex-M7 @ 600 MHz.
- **Core Index:** Teensyduino board manager / core package.
- **FQBNs:** `teensy:avr:teensy40`, `teensy:avr:teensy41`
- **Key Characteristics:** Extreme performance, native USB CDC, hardware PWM and flexible I2C buses.
- **Diagnostics:** Internal temperature sensor registers, heap memory tracking.

### 9. Adafruit Feather Series
- **Variants:** Feather M4 Express (SAMD51), Feather RP2040, Feather ESP32 / ESP32-S2 / ESP32-S3.
- **Core Indexes:** Adafruit SAMD index, Earle Philhower RP2040 index, Espressif ESP32 index.
- **FQBNs:** `adafruit:samd:adafruit_feather_m4`
- **Key Characteristics:** Standardized Feather pinout, integrated LiPo battery charger monitoring.

### 10. BBC micro:bit v2 / nRF52 Series
- **Microcontrollers:** Nordic nRF52833 (micro:bit v2) & nRF52840 (Feather nRF52840, Dongle).
- **Core Index:** `https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_index.json` or Adafruit nRF52 index.
- **FQBNs:** `sandeepmistry:nRF5:BBCmicrobitV2`
- **Key Characteristics:** Built-in LED matrix, Bluetooth LE, internal temperature sensor.

---

## 4. Hardware Adaptation Strategies

### A. Dynamic Pin Mapping & Limits
- **Pin Bounds Handling:** Define target-aware limits (`NUM_DIGITAL_PINS` and `NUM_ANALOG_INPUTS`).
- **`A0 == 0` Guard:** On targets like Renesas RA4M1 or SAMD21 where `A0` is mapped to integer `0`, buffer allocation (`ALLPINS`) uses `NUM_DIGITAL_PINS` to prevent indexing overflows.
- **PWM Availability:** Macro `digitalPinHasPWM(p)` falls back to `((p) < NUM_DIGITAL_PINS)` on architectures (e.g. RP2040 / RP2350 / ESP32) where every digital pin supports PWM.

### B. Non-Volatile Persistence (EEPROM / NVS)
- **Physical EEPROM:** AVR and select ARM MCUs use standard `<EEPROM.h>` with direct byte read/write.
- **Emulated Flash / NVS:** RP2040, RP2350, ESP32, and ESP8266 require `EEPROM.begin(size)` and `EEPROM.commit()` calls to persist pin states, clocks, and settings.

### C. Unified System Diagnostics
Diagnostic commands standard output format:
- `v`: MCU VCC (e.g. `VCC: 3.30V` / `5.01V`)
- `t`: CPU Internal Temperature in Celsius (or sentinel value `-1000000` when unsupported)
- `f`: Free SRAM / Heap size in bytes
- `u`: System Uptime in seconds

---

## 5. CI / CD Workflow Strategy (`.github/workflows/build.yml`)

The CI pipeline compiles `examples/Minipirate/Minipirate.ino` against board targets using `arduino-cli`:

```yaml
name: Arduino Library Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Setup Arduino CLI
        uses: arduino/setup-arduino-cli@v2

      - name: Install platform and libraries
        run: |
          arduino-cli core update-index --additional-urls \
            https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json,\
            https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json,\
            https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json,\
            https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,\
            http://arduino.esp8266.com/stable/package_esp8266com_index.json

          arduino-cli core install arduino:avr
          arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
          arduino-cli core install STMicroelectronics:stm32 --additional-urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
          arduino-cli core install Seeeduino:renesas_uno --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
          arduino-cli core install esp32:esp32 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
          arduino-cli core install esp8266:esp8266 --additional-urls http://arduino.esp8266.com/stable/package_esp8266com_index.json
          arduino-cli lib install Servo

      - name: Link library
        run: |
          mkdir -p $HOME/Arduino/libraries
          ln -s $GITHUB_WORKSPACE $HOME/Arduino/libraries/MiniPirate

      - name: Compile Sketch Targets
        run: |
          arduino-cli compile --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn rp2040:rp2040:rpipico examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn rp2040:rp2040:rpipico2 examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2040 examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350 examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn esp32:esp32:esp32 examples/Minipirate/Minipirate.ino
          arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 examples/Minipirate/Minipirate.ino
```

---

## 6. Automated Release Pipeline & Asset Packaging (`.github/workflows/release.yml`)

When a new version tag is created or a GitHub release is published, pre-compiled binary artifacts are generated and uploaded automatically using `softprops/action-gh-release@v2`.

### Binary Asset Mapping Matrix

| Platform / Board | Output Binary Name | Asset File Name |
|---|---|---|
| **Arduino Uno** | `Minipirate.ino.hex` | `Minipirate-uno.hex` |
| **Raspberry Pi Pico** | `Minipirate.ino.uf2` | `Minipirate-pico.uf2` |
| **Raspberry Pi Pico 2** | `Minipirate.ino.uf2` | `Minipirate-pico2.uf2` |
| **Seeed XIAO RP2040** | `Minipirate.ino.uf2` | `Minipirate-xiao_rp2040.uf2` |
| **Seeed XIAO RP2350** | `Minipirate.ino.uf2` | `Minipirate-xiao_rp2350.uf2` |
| **Seeed XIAO RA4M1** | `Minipirate.ino.bin` | `Minipirate-xiao_ra4m1.bin` |
| **STM32 Nucleo G431RB** | `Minipirate.ino.bin` | `Minipirate-nucleo_g431rb.bin` |
| **STM32 Nucleo F446RE** | `Minipirate.ino.bin` | `Minipirate-nucleo_f446re.bin` |
| **ESP32 DevKit** | `Minipirate.ino.bin` | `Minipirate-esp32.bin` |
| **ESP8266 NodeMCU** | `Minipirate.ino.bin` | `Minipirate-esp8266.bin` |

---

## 7. Implementation Roadmap & Phasing

- [x] **Phase 1: Core AVR & RP2040 / RP2350 Support**
  - Arduino Uno, Raspberry Pi Pico, Pico 2, XIAO RP2040, XIAO RP2350.
- [x] **Phase 2: STM32 & Renesas Support**
  - STM32 Nucleo G431RB, F446RE, Seeed Studio XIAO RA4M1.
- [ ] **Phase 3: Wireless SoC Integration (ESP32 & ESP8266)**
  - Integrate ESP32 (DevKit, S3, C3) and ESP8266 (NodeMCU, D1 Mini) targets into CI workflow and EEPROM abstraction layer.
- [ ] **Phase 4: High-Performance & Portable Targets (Teensy 4.x, Feather, micro:bit v2)**
  - Add Teensy 4.0/4.1, Adafruit Feather SAMD51/nRF52, and BBC micro:bit v2.
