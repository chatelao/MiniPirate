# How to Flash MiniPirate on Seeed Studio XIAO ESP32-C6

This guide provides step-by-step instructions for flashing MiniPirate firmware onto a **Seeed Studio XIAO ESP32-C6**.

---

## 📋 Board Overview & Details

- **Board Name:** Seeed Studio XIAO ESP32-C6
- **Architecture:** ESP32 RISC-V 32-bit single-core (ESP32-C6 @ 160 MHz)
- **FQBN:** `esp32:esp32:XIAO_ESP32C6`
- **Core Package Index:** `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
- **Release Asset:** `Minipirate-xiao_esp32c6.bin`

---

## 🛠️ Requirements & Setup

### Core Installation
Install Espressif's ESP32 core via Arduino CLI:

```bash
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install Servo
```

---

## ⚡ Bootloader Mode (Boot Button / Pin Switch)

To put the Seeed Studio XIAO ESP32-C6 into bootloader flashing mode manually:
1. Press and hold the **BOOT** button (or short GPIO 9 / BOOT pad to GND).
2. Connect the XIAO ESP32-C6 to your PC via a USB-C cable.
3. Release the **BOOT** button.
4. The board will enter download mode and accept new firmware flashes over the serial USB interface.

---

## 🚀 Flashing Pre-compiled Binaries (`.bin`)

### Using `esptool.py`
```bash
esptool.py --chip esp32c6 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 Minipirate-xiao_esp32c6.bin
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:XIAO_ESP32C6 examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Add `https://espressif.github.io/arduino-esp32/package_esp32_index.json` under **Preferences > Additional Boards Manager URLs**.
2. Open **Boards Manager**, search for `esp32` by Espressif, and install the package.
3. Select **Tools > Board > ESP32 Arduino > XIAO_ESP32C6**.
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
   - `v` -> View MCU operating voltage
   - `t` -> View MCU internal temperature
   - `f` -> View free heap memory
