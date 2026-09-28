# How to Flash MiniPirate on Raspberry Pi Pico

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Raspberry Pi Pico** (RP2040).

---

## 📋 Board Overview & Details

- **Board Name:** Raspberry Pi Pico
- **Architecture:** RP2040 (ARM Cortex-M0+)
- **FQBN:** `rp2040:rp2040:rpipico`
- **Core Package Index:** `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
- **Release Assets:** `Minipirate-rpipico.uf2`, `Minipirate-rpipico.bin`

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

## ⚡ Bootloader Mode (BOOTSEL)

To put the Raspberry Pi Pico into bootloader mode:
1. Unplug the USB cable from the Pico.
2. Hold down the **BOOTSEL** button on the Pico board.
3. Plug the USB cable back into your computer while holding **BOOTSEL**.
4. Release the **BOOTSEL** button.
5. The board will appear as a mass storage device named `RPI-RP2`.

---

## 🚀 Flashing Pre-compiled Binaries (`.uf2`)

### Drag and Drop (Easiest)
1. Download `Minipirate-rpipico.uf2` from the release assets.
2. Put board in **BOOTSEL** mode (`RPI-RP2` drive appears).
3. Drag and drop `Minipirate-rpipico.uf2` onto the `RPI-RP2` drive.
4. The Pico will automatically reboot and start running MiniPirate.

### Using `picotool`
```bash
picotool load Minipirate-rpipico.uf2
picotool reboot
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn rp2040:rp2040:rpipico examples/Minipirate/Minipirate.ino

# Upload (when connected via serial or UF2 bootloader)
arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:rpipico examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json` to **Preferences > Additional Boards Manager URLs**.
2. Open **Tools > Board > Boards Manager**, search `rp2040`, and install **Raspberry Pi Pico/RP2040**.
3. Select **Tools > Board > Raspberry Pi RP2040 > Raspberry Pi Pico**.
4. Click **Upload**.

---

## 🔌 Serial Connection & Initial Verification

1. Connect to the board's USB CDC Serial port.
2. Set terminal connection parameters:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `CR` & `LF` (Newline)
3. Type `h` and press Enter to view help commands.
4. Test commands:
   - `f` -> Check free RAM and system heap info
   - `t` -> Read RP2040 internal temperature
   - `p` -> View GPIO pin matrix
