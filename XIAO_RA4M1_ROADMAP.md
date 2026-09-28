# Seeed Studio XIAO RA4M1 Support Roadmap

This document details the complete roadmap and implementation steps required to add and maintain full board support for the **Seeed Studio XIAO RA4M1** in MiniPirate, including core toolchain setup, sketch adaptations, CI build workflows, and release asset packaging.

---

## 1. Overview & Objectives

The Seeed Studio XIAO RA4M1 is an ultra-small development board powered by the Renesas RA4M1 32-bit ARM Cortex-M4F microcontroller. Adding support for this target expands MiniPirate's hardware compatibility.

**Key Goals:**
- Enable local and CI compilation targeting the Seeed Studio XIAO RA4M1 board.
- Handle pin-mapping edge cases specific to the Renesas core (e.g., `A0 == 0`, pin counts).
- Integrate automated CI builds in GitHub Actions.
- Package binary files (`.hex` / `.bin`) for automated GitHub release assets.

---

## 2. Toolchain & Core Environment Setup

### Board Package Index & FQBN
- **Package Index URL:** `https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
- **Core Name:** `Seeeduino:renesas_uno`
- **Fully Qualified Board Name (FQBN):** `Seeeduino:renesas_uno:XIAO_RA4M1`

### Local Setup via Arduino CLI
```bash
# Update core index including Seeed Studio package URL
arduino-cli core update-index --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

# Install Renesas UNO core for Seeed Studio
arduino-cli core install Seeeduino:renesas_uno --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

# Compile MiniPirate for XIAO RA4M1
arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino
```

---

## 3. Firmware & Source Code Adaptations

### A. Fallback Macros in `examples/Minipirate/baseIO.h`
Certain board definitions in third-party cores may omit `NUM_DIGITAL_PINS` or `NUM_ANALOG_INPUTS`. Define fallback limits to prevent compilation errors:

```cpp
#ifndef NUM_DIGITAL_PINS
#define NUM_DIGITAL_PINS 10
#endif

#ifndef NUM_ANALOG_INPUTS
#define NUM_ANALOG_INPUTS 6
#endif
```

### B. Pin Mapping & `A0` Edge Case (`Minipirate.ino` & `baseIO.cpp`)
On the XIAO RA4M1 core, analog pin macro `A0` may be defined as `0` (overlapping digital pin indexing).
- **Buffer Allocation:** Define `ALLPINS` as `NUM_DIGITAL_PINS` when `A0 == 0` (or `NUM_ANALOG_INPUTS + A0` when `A0 > 0`) to prevent out-of-bounds array indexing in `clock_table`.
- **Port Printing:** In `printPorts()` and `printPortsQuick()`, dynamically adjust upper bounds based on pin definitions so digital pins D0–D9 and analog inputs A0–A5 display correctly.

### C. MCU Diagnostics
Ensure chip diagnostic commands (`v`, `t`, `f`) handle RA4M1 gracefully:
- `readMCU_VCC()`: Return `3.3V` fallback or platform-specific ADC reference reading if available.
- `readMCUInternalTemp()`: Return `-1000000` sentinel value if internal temperature sensing is unsupported on the Renesas core.
- `freeRam()`: Implement heap sensing or safe fallback value for Cortex-M4 Renesas architecture.

---

## 4. Continuous Integration (`.github/workflows/build.yml`)

Add the package index and board installation step to the GitHub Actions build workflow:

```yaml
- name: Install platform and libraries
  run: |
    arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json,https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json,https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
    arduino-cli core install arduino:avr
    arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    arduino-cli core install STMicroelectronics:stm32 --additional-urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
    arduino-cli core install Seeeduino:renesas_uno --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
    arduino-cli lib install Servo

- name: Compile Sketch
  run: |
    # ... existing boards ...
    arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino
```

---

## 5. Release Asset Packaging (`.github/workflows/release.yml`)

Automate binary export and asset uploading when releasing new MiniPirate versions:

### Compilation & Export Step
```yaml
- name: Compile Sketch
  run: |
    # Export build binaries for XIAO RA4M1
    arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 -e examples/Minipirate/Minipirate.ino
```

### Prepare & Upload Assets
```yaml
- name: Prepare Release Assets
  run: |
    # Copy exported binary to assets folder
    cp examples/Minipirate/build/Seeeduino.renesas_uno.XIAO_RA4M1/Minipirate.ino.bin assets/Minipirate-xiao_ra4m1.bin || true
    cp examples/Minipirate/build/Seeeduino.renesas_uno.XIAO_RA4M1/Minipirate.ino.hex assets/Minipirate-xiao_ra4m1.hex || true

- name: Upload Release Assets
  uses: softprops/action-gh-release@v2
  with:
    files: |
      # ... existing assets ...
      assets/Minipirate-xiao_ra4m1.bin
      assets/Minipirate-xiao_ra4m1.hex
```

---

## 6. Verification Checklist

- [x] Local build test passes using `arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino`.
- [x] Documentation updated in `README.md` board support matrix.
- [x] `.github/workflows/build.yml` compiles XIAO RA4M1 successfully on CI.
- [x] `.github/workflows/release.yml` produces `Minipirate-xiao_ra4m1.bin` and/or `Minipirate-xiao_ra4m1.hex` assets.
