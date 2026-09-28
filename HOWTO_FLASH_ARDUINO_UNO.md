# How to Flash MiniPirate on Arduino Uno

This guide provides step-by-step instructions for flashing MiniPirate firmware onto an **Arduino Uno** (ATmega328P).

---

## 📋 Board Overview & Details

- **Board Name:** Arduino Uno Rev3 (and compatible ATmega328P boards)
- **Architecture:** AVR (8-bit)
- **FQBN:** `arduino:avr:uno`
- **Release Assets:** `Minipirate-uno.hex`, `Minipirate-uno-with_bootloader.hex`

---

## 🛠️ Requirements & Setup

### Core Installation
The Arduino AVR core comes pre-installed in the Arduino IDE. If using `arduino-cli`:

```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli lib install Servo
```

---

## ⚡ Bootloader & Reset Mode

- Connect your Arduino Uno to your PC via a USB Type-B cable.
- The standard Arduino bootloader uses auto-reset via DTR/RTS signals on serial connection.
- No physical buttons or jumper changes are required to enter bootloader mode for upload.

---

## 🚀 Flashing Pre-compiled Binaries (`.hex`)

If you downloaded pre-compiled `.hex` assets from MiniPirate releases:

### Using `avrdude` (CLI)
Replace `/dev/ttyACM0` (Linux) or `COM3` (Windows) with your actual serial port:

```bash
avrdude -v -p atmega328p -c arduino -P /dev/ttyACM0 -b 115200 -D -U flash:w:Minipirate-uno.hex:i
```

---

## 💻 Compiling & Flashing from Source

### Using Arduino CLI
```bash
# Compile
arduino-cli compile --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino
```

### Using Arduino IDE
1. Open Arduino IDE.
2. Open `examples/Minipirate/Minipirate.ino`.
3. Select **Tools > Board > Arduino AVR Boards > Arduino Uno**.
4. Select the corresponding Port under **Tools > Port**.
5. Click **Upload** (Ctrl + U / Cmd + U).

---

## 🔌 Serial Connection & Initial Verification

1. Open your terminal emulator (Arduino Serial Monitor, PuTTY, TeraTerm, Screen, etc.).
2. Set serial parameters:
   - **Baud Rate:** `57600`
   - **Data Bits:** 8, **Parity:** None, **Stop Bits:** 1
   - **Line Ending:** दोन्ही/Both `CR` & `LF` (Newline)
3. Type `h` or `?` and press Enter to open the MiniPirate CLI menu.
4. Test commands:
   - `v` -> Print MCU VCC voltage reading
   - `t` -> Print internal temperature reading
   - `p` -> Display GPIO port matrix and pin states
