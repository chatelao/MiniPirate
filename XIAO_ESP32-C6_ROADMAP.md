# Seeed Studio XIAO ESP32-C6 Support Roadmap

This document details the complete roadmap and implementation steps required to add and maintain full board support for the **Seeed Studio XIAO ESP32-C6** in MiniPirate, including core toolchain setup, sketch adaptations, CI build workflows, and release asset packaging.

---

## 1. Overview & Objectives

The Seeed Studio XIAO ESP32-C6 is an ultra-compact development board powered by the Espressif ESP32-C6 32-bit RISC-V single-core microcontroller (up to 160 MHz), supporting 2.4 GHz Wi-Fi 6, Bluetooth 5 (LE), and IEEE 802.15.4 (Zigbee / Thread). Adding support for this target expands MiniPirate's hardware compatibility to the modern ESP32 RISC-V architecture.

**Key Goals:**
- Enable local and CI compilation targeting the Seeed Studio XIAO ESP32-C6 board.
- Handle pin-mapping, PWM driver, and PROGMEM string macro edge cases specific to the ESP32 RISC-V core.
- Implement chip diagnostic helper functions (`v`, `t`, `f`) tailored for ESP32-C6 memory and internal sensors.
- Integrate automated CI builds in GitHub Actions.
- Package binary files (`.bin`) for automated GitHub release assets.

---

## 2. Toolchain & Core Environment Setup

### Board Package Index & FQBN
- **Package Index URL:** `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
- **Core Name:** `esp32:esp32`
- **Fully Qualified Board Name (FQBN):** `esp32:esp32:XIAO_ESP32C6`

### Local Setup via Arduino CLI
```bash
# Update core index including Espressif ESP32 package URL
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Install ESP32 core
arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Compile MiniPirate for XIAO ESP32-C6
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 examples/Minipirate/Minipirate.ino
```

---

## 3. Firmware & Source Code Adaptations

### A. Fallback Macros in `examples/Minipirate/baseIO.h`
Ensure fallback limits for digital and analog pin counts are defined if omitted by the ESP32 core definitions:

```cpp
#ifndef NUM_DIGITAL_PINS
#define NUM_DIGITAL_PINS 11
#endif

#ifndef NUM_ANALOG_INPUTS
#define NUM_ANALOG_INPUTS 7
#endif
```

### B. Pin Mapping & PWM Functionality
- **PWM Support:** The ESP32 core handles PWM via the `ledc` peripheral driver. Ensure `analogWrite()` and PWM pin state macros map correctly for all digital output pins on the XIAO footprint.
- **Port Output:** Verify dynamic loop bounds in `printPorts()` and `printPortsQuick()` so digital and analog pins are indexed accurately without array overflow.

### C. Flash Memory & String Macros (`Strings_PGM_MEM.h`)
The ESP32 architecture uses flat memory mapping where PROGMEM macros do not require special reading routines (`pgm_read_byte`). Ensure conditional preprocessor directives handle ESP32 cleanly:

```cpp
#if defined(HAS_PGMSPACE) && !defined(ESP8266) && !defined(ESP32)
  // Standard AVR / SAMD PROGMEM handling
#else
  // Direct memory access for ESP32 architecture
#endif
```

### D. MCU Diagnostics
Implement ESP32-C6 platform diagnostics in `baseIO.cpp`:
- `readMCU_VCC()`: Return target regulated operating voltage (`3.30V`) or ADC calibration reading if VREF channel is available.
- `readMCUInternalTemp()`: Utilize `temperatureRead()` on-chip sensor if supported, or return the sentinel `-1000000` value.
- `freeRam()`: Implement heap sensing using `ESP.getFreeHeap()`.

---

## 4. Continuous Integration (`.github/workflows/build.yml`)

Add the Espressif package index and board installation step to the GitHub Actions build workflow:

```yaml
- name: Install platform and libraries
  run: |
    arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json,https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json,https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json,https://espressif.github.io/arduino-esp32/package_esp32_index.json
    arduino-cli core install arduino:avr
    arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    arduino-cli core install STMicroelectronics:stm32 --additional-urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
    arduino-cli core install Seeeduino:renesas_uno --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
    arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
    arduino-cli lib install Servo

- name: Compile Sketch
  run: |
    # ... existing boards ...
    arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 examples/Minipirate/Minipirate.ino
```

---

## 5. Release Asset Packaging (`.github/workflows/release.yml`)

Automate binary export and asset uploading when publishing new MiniPirate releases:

### Compilation & Export Step
```yaml
- name: Compile Sketch
  run: |
    # Export build binaries for XIAO ESP32-C6
    arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 -e examples/Minipirate/Minipirate.ino
```

### Prepare & Upload Assets
```yaml
- name: Prepare Release Assets
  run: |
    # Copy exported binary to assets folder
    cp examples/Minipirate/build/esp32.esp32.XIAO_ESP32C6/Minipirate.ino.bin assets/Minipirate-xiao_esp32c6.bin || true

- name: Upload Release Assets
  uses: softprops/action-gh-release@v2
  with:
    files: |
      # ... existing assets ...
      assets/Minipirate-xiao_esp32c6.bin
```

---

## 6. Verification Checklist

- [x] Local build test passes using `arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 examples/Minipirate/Minipirate.ino`.
- [x] Documentation updated in `README.md` board support matrix.
- [x] `.github/workflows/build.yml` compiles XIAO ESP32-C6 successfully on CI.
- [x] `.github/workflows/release.yml` produces `Minipirate-xiao_esp32c6.bin` asset.
