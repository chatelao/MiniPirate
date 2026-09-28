# How to Flash MiniPirate on Raspberry Pi Pico 2

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Raspberry Pi Pico 2** (RP2350).

---

## 📋 Board Overview & Details

- **Board Name:** Raspberry Pi Pico 2
- **Architecture:** RP2350 (ARM Cortex-M33 / Hazard3 RISC-V)
- **FQBN:** `rp2040:rp2040:rpipico2`
- **Core Package Index:** `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **Release Assets:** `Minipirate-rpipico2.uf2`, `Minipirate-rpipico2.bin`

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

## ⚡ Bootloader Mode (BOOTSEL)

To put the Raspberry Pi Pico 2 into bootloader mode:
1. Unplug the USB cable from the Pico 2.
2. Press and hold the **BOOTSEL** button on the Pico 2 board.
3. Plug the USB cable into your computer while holding **BOOTSEL**.
4. Release the **BOOTSEL** button.
5. The board will mount as a mass storage volume named `RP2350` (or `RPI-RP2`).

---

## 🚀 Flashing Pre-compiled Binaries (`.uf2`)

### Drag and Drop (Easiest)
1. Download `Minipirate-rpipico2.uf2` from release assets.
2. Enter **BOOTSEL** mode (`RP2350` drive appears).
3. Drag and drop `Minipirate-rpipico2.uf2` onto the `RP2350` drive.
4. The board will automatically reboot and start MiniPirate.

### Using `picotool`
```bash
picotool load Minipirate-rpipico2.uf2
picotool reboot
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn rp2040:rp2040:rpipico2 examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:rpipico2 examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Open Arduino IDE.
2. Select **Tools > Board > Raspberry Pi RP2040 > Raspberry Pi Pico 2**.
3. Select the correct serial port under **Tools > Port**.
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the board's USB CDC Serial port.
2. Open terminal with parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Type `h` and press Enter to view MiniPirate CLI help menu.
4. Verify hardware diagnostic output:
   - `v` -> Print MCU VCC voltage
   - `t` -> Print RP2350 internal temperature
   - `p` -> View GPIO status matrix
