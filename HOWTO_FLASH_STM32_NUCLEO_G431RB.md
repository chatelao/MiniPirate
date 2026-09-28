# How to Flash MiniPirate on STM32 Nucleo G431RB

This guide provides step-by-step instructions for flashing MiniPirate firmware onto an **STM32 Nucleo-64 G431RB** development board.

---

## 📋 Board Overview & Details

- **Board Name:** STM32 Nucleo G431RB (Nucleo-64 series)
- **Architecture:** ARM Cortex-M4 (STM32G4 Series)
- **FQBN:** `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB`
- **Core Package Index:** `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
- **Release Assets:** `Minipirate-nucleo_g431rb.bin`, `Minipirate-nucleo_g431rb.hex`

---

## 🛠️ Requirements & Setup

### Core Installation
Use the official **STM32duino** core:

```bash
arduino-cli core update-index --additional-urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
arduino-cli core install STMicroelectronics:stm32 --additional-urls https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
arduino-cli lib install Servo
```

---

## ⚡ Integrated ST-LINK On-Board Programmer

The Nucleo-64 G431RB board includes an integrated ST-LINK/V2-1 programmer/debugger via USB:
- Connect your Nucleo board to your PC using the mini-USB / micro-USB port on the ST-LINK side of the board.
- The ST-LINK provides both SWD flashing interface and a USB Mass Storage drag-and-drop drive (usually named `NODE_G431RB` or `NUCLEO`).

---

## 🚀 Flashing Pre-compiled Binaries (`.bin` / `.hex`)

### Method 1: ST-LINK Mass Storage Drag-and-Drop
1. Download `Minipirate-nucleo_g431rb.bin` from release assets.
2. Connect the Nucleo G431RB board to your computer.
3. Open the USB storage volume created by ST-LINK (e.g. `NODE_G431RB`).
4. Drag and drop `Minipirate-nucleo_g431rb.bin` into the drive.
5. The ST-LINK status LED (LD1) will blink red/green while flashing, then settle into green upon completion.

### Method 2: Using STM32CubeProgrammer
```bash
STM32_Programmer_CLI -c port=SWD -w Minipirate-nucleo_g431rb.bin 0x08000000 -v -rst
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB examples/Minipirate/Minipirate.ino

# Upload via ST-LINK / OpenOCD
arduino-cli upload -p /dev/ttyACM0 --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json` under **Preferences > Additional Boards Manager URLs**.
2. Install **STM32 MCU based boards** from Boards Manager.
3. Select configuration:
   - **Board:** `Nucleo-64`
   - **Board part number:** `Nucleo G431RB`
   - **U(S)ART support:** `Enabled (generic 'Serial')`
   - **Upload method:** `STM32CubeProgrammer (SWD)` or `Mass Storage`
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. The ST-LINK USB interface exposes an integrated Virtual COM Port (STMicroelectronics Virtual COM Port).
2. Set terminal connection parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Type `h` and press Enter to view help text.
4. Test STM32 internal channels:
   - `v` -> Print MCU VCC voltage via internal VREF
   - `t` -> Print chip temperature calculated via internal temperature sensor channel
   - `f` -> Print available heap RAM space
