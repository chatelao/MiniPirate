# How to Flash MiniPirate on STM32 Nucleo F446RE

This guide provides step-by-step instructions for flashing MiniPirate firmware onto an **STM32 Nucleo-64 F446RE** development board.

---

## 📋 Board Overview & Details

- **Board Name:** STM32 Nucleo F446RE (Nucleo-64 series)
- **Architecture:** ARM Cortex-M4 (STM32F4 Series with FPU)
- **FQBN:** `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE`
- **Core Package Index:** `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
- **Release Assets:** `Minipirate-nucleo_f446re.bin`, `Minipirate-nucleo_f446re.hex`

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

## ⚡ Integrated ST-LINK Programmer

The Nucleo-64 F446RE board includes an integrated ST-LINK/V2-1 interface:
- Connect the mini-USB cable to the USB port on the top edge of the board (ST-LINK side).
- The ST-LINK interface provides both SWD flashing and USB Mass Storage drag-and-drop drive (usually mounted as `NUCLEO` or `NODE_F446RE`).

---

## 🚀 Flashing Pre-compiled Binaries (`.bin` / `.hex`)

### Method 1: ST-LINK Mass Storage Drag-and-Drop
1. Download `Minipirate-nucleo_f446re.bin` from release assets.
2. Connect the Nucleo F446RE board to your PC via USB.
3. Open the mounted mass storage volume (e.g. `NODE_F446RE`).
4. Copy `Minipirate-nucleo_f446re.bin` into the root directory of the drive.
5. The board LED will flicker briefly while flashing, and automatically execute the new firmware.

### Method 2: Using STM32CubeProgrammer CLI
```bash
STM32_Programmer_CLI -c port=SWD -w Minipirate-nucleo_f446re.bin 0x08000000 -v -rst
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json` under **Preferences > Additional Boards Manager URLs**.
2. Install **STM32 MCU based boards** from Boards Manager.
3. Configure settings:
   - **Board:** `Nucleo-64`
   - **Board part number:** `Nucleo F446RE`
   - **U(S)ART support:** `Enabled (generic 'Serial')`
   - **Upload method:** `STM32CubeProgrammer (SWD)` or `Mass Storage`
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the board's Virtual COM Port via USB.
2. Configure serial terminal parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Send `h` to retrieve the MiniPirate CLI menu.
4. Verify hardware functions:
   - `v` -> Display VCC voltage level
   - `t` -> Display internal MCU chip temperature
   - `p` -> View digital and analog pin states
