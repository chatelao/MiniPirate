# How to Flash MiniPirate on Seeed Studio XIAO RA4M1

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Seeed Studio XIAO RA4M1**.

---

## 📋 Board Overview & Details

- **Board Name:** Seeed Studio XIAO RA4M1
- **Architecture:** Renesas RA4M1 (32-bit ARM Cortex-M4F)
- **FQBN:** `Seeeduino:renesas_uno:XIAO_RA4M1`
- **Core Package Index:** `https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
- **Release Assets:** `Minipirate-xiao_ra4m1.bin`, `Minipirate-xiao_ra4m1.hex`

---

## 🛠️ Requirements & Setup

### Core Installation
Use Seeed Studio's Renesas Uno core:

```bash
arduino-cli core update-index --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
arduino-cli core install Seeeduino:renesas_uno --additional-urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
arduino-cli lib install Servo
```

---

## ⚡ Bootloader Mode (Double-Tap Reset)

To put the Seeed Studio XIAO RA4M1 into DFU / bootloader mode:
1. Connect the XIAO RA4M1 to your PC via a USB-C cable.
2. Quickly double-tap the **R** (RESET) pad / button on the board.
3. The board LED will pulsate softly, indicating it has entered bootloader mode.
4. On supported hosts, a USB virtual COM port or DFU bootloader device will appear.

---

## 🚀 Flashing Pre-compiled Binaries (`.bin` / `.hex`)

### Using `bossac` / `rfp-cli` / Arduino CLI Upload
```bash
bossac -i -d --port=/dev/ttyACM0 -U -e -w -v Minipirate-xiao_ra4m1.bin -R
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn Seeeduino:renesas_uno:XIAO_RA4M1 examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json` under **Preferences > Additional Boards Manager URLs**.
2. Open **Boards Manager**, search for `XIAO RA4M1` or `Seeeduino Renesas`, and install the package.
3. Select **Tools > Board > Seeed Renesas Boards > Seeed Studio XIAO RA4M1**.
4. Select the port under **Tools > Port**.
5. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the board via USB-C serial.
2. Open serial terminal with settings:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Send `h` to open the main MiniPirate CLI menu.
4. Verify board functionality:
   - `p` -> View GPIO pin matrix
   - `q` -> View compact port status matrix
   - `i` -> Scan active I2C bus devices
