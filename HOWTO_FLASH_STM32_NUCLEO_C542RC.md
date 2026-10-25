# How to Flash MiniPirate on STM32 C5 Boards (NOD_C542RC, XIAO-STM32C5, NOD_C562RC)

This guide provides step-by-step instructions for flashing MiniPirate firmware onto **STM32 Nucleo C542RC (`NOD_C542RC`)**, **Seeed Studio XIAO-STM32C5**, and **STM32 Nucleo C562RE (`NOD_C562RC`)** development boards.

---

## 📋 Board Overview & Details

- **Board Variants & FQBNs:**
  - **NOD_C542RC (Nucleo C542RC / Generic C542RC):** `STMicroelectronics:stm32:GenC5:pnum=GENERIC_C552RCTX`
  - **XIAO-STM32C5 (Seeed Studio XIAO STM32C5):** `STMicroelectronics:stm32:GenC5:pnum=GENERIC_C562RETX`
  - **NOD_C562RC / Nucleo C562RE:** `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_C562RE`
- **Architecture:** ARM Cortex-M33 (STM32C5 Series with TrustZone / Security Features & HAL v2)
- **Core Package Index:** `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
- **Release Assets:**
  - `NOD_C542RC`: `Minipirate-nod_c542rc.bin`, `Minipirate-nod_c542rc.hex`
  - `XIAO-STM32C5`: `Minipirate-xiao_stm32c5.bin`, `Minipirate-xiao_stm32c5.hex`
  - `NOD_C562RC`: `Minipirate-nod_c562rc.bin`, `Minipirate-nod_c562rc.hex`, `Minipirate-nucleo_c562re.bin`, `Minipirate-nucleo_c562re.hex`

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

The Nucleo-64 C542RC board includes an integrated ST-LINK/V3 or ST-LINK/V2-1 interface:
- Connect your Nucleo board to your PC using the USB-C / micro-USB port on the ST-LINK side of the board.
- The ST-LINK interface provides both SWD flashing and USB Mass Storage drag-and-drop drive (mounted as `NODE_C542RC`, `NOD_C542RC`, or `NUCLEO`).

---

## 🚀 Flashing Pre-compiled Binaries (`.bin` / `.hex`)

### Method 1: ST-LINK Mass Storage Drag-and-Drop
1. Download the appropriate `.bin` file (`Minipirate-nod_c542rc.bin`, `Minipirate-xiao_stm32c5.bin`, or `Minipirate-nod_c562rc.bin`) from release assets.
2. Connect your target board to your PC via USB.
3. Open the mounted mass storage volume (e.g. `NOD_C542RC`, `NOD_C562RE`, or `DAPLINK`).
4. Copy the release binary into the root directory of the drive.
5. The status LED will flash while burning, and reset upon completion.

### Method 2: Using STM32CubeProgrammer CLI
```bash
STM32_Programmer_CLI -c port=SWD -w Minipirate-nod_c542rc.bin 0x08000000 -v -rst
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile for NOD_C542RC
arduino-cli compile --fqbn STMicroelectronics:stm32:GenC5:pnum=GENERIC_C552RCTX examples/Minipirate/Minipirate.ino

# Compile for XIAO-STM32C5
arduino-cli compile --fqbn STMicroelectronics:stm32:GenC5:pnum=GENERIC_C562RETX examples/Minipirate/Minipirate.ino

# Compile for NOD_C562RC / Nucleo C562RE
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_C562RE examples/Minipirate/Minipirate.ino

# Upload (example for NOD_C542RC)
arduino-cli upload -p /dev/ttyACM0 --fqbn STMicroelectronics:stm32:GenC5:pnum=GENERIC_C552RCTX examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json` under **Preferences > Additional Boards Manager URLs**.
2. Install **STM32 MCU based boards** from Boards Manager.
3. Configure board settings:
   - **Board:** `Nucleo-64`
   - **Board part number:** `Nucleo C542RC`
   - **U(S)ART support:** `Enabled (generic 'Serial')`
   - **Upload method:** `STM32CubeProgrammer (SWD)` or `Mass Storage`
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the board's Virtual COM Port via USB.
2. Set terminal connection parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Send `h` or `?` to display the MiniPirate help menu.
4. Test hardware status:
   - `v` -> Print MCU VCC voltage via internal VREFINT reference
   - `t` -> Print internal temperature
   - `p` -> View all pin directions, states, and capabilities
