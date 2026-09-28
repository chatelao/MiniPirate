# How to Flash MiniPirate on Seeed Studio XIAO RP2350

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Seeed Studio XIAO RP2350**.

---

## 📋 Board Overview & Details

- **Board Name:** Seeed Studio XIAO RP2350
- **Architecture:** RP2350 (ARM Cortex-M33)
- **FQBN:** `rp2040:rp2040:seeed_xiao_rp2350`
- **Core Package Index:** `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **Release Assets:** `Minipirate-xiao_rp2350.uf2`, `Minipirate-xiao_rp2350.bin`

---

## 🛠️ Requirements & Setup

### Core Installation
Use Earle Philhower's `arduino-pico` core (supports RP2040 & RP2350):

```bash
arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli lib install Servo
```

---

## ⚡ Bootloader Mode (BOOTSEL / Reset)

To put the Seeed Studio XIAO RP2350 into UF2 bootloader mode:
1. Press and hold the **B** (BOOT) button on the board.
2. Press and release the **R** (RESET) button while continuing to hold **B**.
3. Release the **B** button.
4. A virtual drive named `RP2350` (or `RPI-RP2`) will appear on your system.

---

## 🚀 Flashing Pre-compiled Binaries (`.uf2`)

### Drag and Drop (Easiest)
1. Download `Minipirate-xiao_rp2350.uf2` from release assets.
2. Enter **BOOTSEL** mode on the board (`RP2350` drive appears).
3. Copy/Drag `Minipirate-xiao_rp2350.uf2` to the `RP2350` drive.
4. The board will reset automatically and begin running MiniPirate.

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2350 examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:seeed_xiao_rp2350 examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Open Arduino IDE.
2. Select **Tools > Board > Raspberry Pi RP2040 > Seeed XIAO RP2350**.
3. Select the serial port under **Tools > Port**.
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to board via USB-C serial connection.
2. Open terminal with parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Send command `h` to print the help menu.
4. Test commands:
   - `v` -> Print MCU VCC voltage
   - `t` -> Print MCU internal temperature
   - `f` -> Print free SRAM memory
