# How to Flash MiniPirate on Seeed Studio XIAO RP2040

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Seeed Studio XIAO RP2040**.

---

## 📋 Board Overview & Details

- **Board Name:** Seeed Studio XIAO RP2040
- **Architecture:** RP2040 (ARM Cortex-M0+)
- **FQBN:** `rp2040:rp2040:seeed_xiao_rp2040`
- **Core Package Index:** `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **Release Assets:** `Minipirate-xiao_rp2040.uf2`, `Minipirate-xiao_rp2040.bin`

---

## 🛠️ Requirements & Setup

### Core Installation
Use Earle Philhower's `arduino-pico` core:

```bash
arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
arduino-cli lib install Servo
```

---

## ⚡ Bootloader Mode (BOOTSEL / Reset)

To put the Seeed Studio XIAO RP2040 into UF2 bootloader mode:

### Method 1: On-board Buttons
1. Press and hold the **B** (BOOTSEL) button.
2. Press and release the **R** (RESET) button while holding **B**.
3. Release the **B** button.
4. A virtual storage drive named `RPI-RP2` will mount on your computer.

### Method 2: Replug with BOOT Button
1. Disconnect the USB-C cable.
2. Press and hold the **BOOT** button on the XIAO RP2040.
3. Plug in the USB-C cable.
4. Release the **BOOT** button.

---

## 🚀 Flashing Pre-compiled Binaries (`.uf2`)

### Drag and Drop (Easiest)
1. Download `Minipirate-xiao_rp2040.uf2` from release assets.
2. Put the board into bootloader mode (`RPI-RP2` drive appears).
3. Drag and drop `Minipirate-xiao_rp2040.uf2` onto the `RPI-RP2` drive.
4. The board will reset automatically and launch MiniPirate.

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn rp2040:rp2040:seeed_xiao_rp2040 examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:seeed_xiao_rp2040 examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Open Arduino IDE.
2. Select **Tools > Board > Raspberry Pi RP2040 > Seeed XIAO RP2040**.
3. Select the correct serial port under **Tools > Port**.
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the XIAO RP2040 USB serial port.
2. Open terminal emulator:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Type `h` and press Enter to view help options.
4. Test pin status matrix using `p` or compact view using `q`.
