# SwissHandmade MiniPirate

[![Arduino Library Build](https://github.com/chatelao/MiniPirate/workflows/Arduino%20Library%20Build/badge.svg)](https://github.com/chatelao/MiniPirate/actions)

**MiniPirate** is a powerful, lightweight, human-readable serial protocol tool and Arduino library for direct hardware interaction and debugging. Inspired heavily by the legendary **Bus Pirate**, it allows you to scan I2C buses, read/write GPIOs, run automated clocks, control servo motors, read/write EEPROM, and monitor micro-controller CPU details (temperature, VCC, free RAM) straight from any serial terminal — **no code writing or compiling required for each new experiment.**

It is an ideal utility for electronics enthusiasts, hardware hackers, and professionals looking to quickly bring up or diagnose new sensors, actuators, and target boards.

---

## 🚀 Key Features

- **I2C Bus Diagnostics:** Full support for scanning I2C addresses, selecting active devices, and performing non-blocking reads and writes.
- **Rich GPIO Control:** Instantly change pin directions (INPUT/OUTPUT), toggle states, read analog/digital values, and perform sequential pin sweeps.
- **Automated Clock Generator:** Generate a clock signal on any pin with custom millisecond-level intervals.
- **Servo Motor Support:** Directly control servo angles from the CLI.
- **Persistent State Storage:** Save and restore GPIO configurations (directions, values, clocks) to/from internal EEPROM across power cycles.
- **Deep MCU Monitoring:** Read on-chip temperature, VCC voltage levels, uptime, and free SRAM (available on supported platforms).
- **Interactive Help System:** Built-in extended, command-specific help guides (e.g., `h p`, `h g`, `h i`) using minimal SRAM overhead.

---

## 🔌 Supported Boards & Architectures

MiniPirate is designed as a highly cross-platform Arduino library supporting a wide range of architectures:

| Platform | Board Variant | FQBN / Core |
|---|---|---|
| **AVR** | Arduino Uno, Nano, Mega, etc. | `arduino:avr:uno` |
| **RP2040** | Raspberry Pi Pico, Seeed Studio XIAO RP2040 | `rp2040:rp2040:rpipico`, `rp2040:rp2040:seeed_xiao_rp2040` |
| **RP2350** | Raspberry Pi Pico 2, Seeed Studio XIAO RP2350 | `rp2040:rp2040:rpipico2`, `rp2040:rp2040:seeed_xiao_rp2350` |
| **ARM Cortex-M4** | STM32 Nucleo G431RB, STM32 Nucleo F446RE | `STMicroelectronics:stm32:Nucleo_64` |
| **Renesas RA4M1** | Seeed Studio XIAO RA4M1 | `Seeeduino:renesas_uno:XIAO_RA4M1` |

---

## 🏁 Getting Started

### 1. Hardware Connection & Setup
1. Flash the MiniPirate firmware to your microcontroller (see the [Installation & Compilation](#-installation--compilation) section below).
2. Connect the board to your PC via a USB/Serial cable.
3. Open your favorite Serial Terminal (e.g., Arduino Serial Monitor, PuTTY, TeraTerm, Minicom, screen).
4. Configure your connection settings:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `NL & CR` or `LF / CR` (newline is used to terminate multi-argument commands).

### 2. General Usage
Upon boot, MiniPirate outputs system diagnostic information (detected pin counts, frequency, and help instructions). To show the main help screen, type `h` or `?` and press enter.

For extended help on any specific command, type `h <command>` (for example, `h p` for ports, or `h g` for PWM).

---

## 🛠️ List of Supported Commands

### General Commands

| Command | Description | Output Example |
|---|---|---|
| `h` / `?` | Show main help menu | *(Help list)* |
| `h [cmd]` | Show command-specific extended help | `HELP: Port Status ('p' and 'q') ...` |
| `v` | Show MCU VCC reading | `VCC: 5.01V` |
| `t` | Show MCU internal temperature reading | `Temp: 25.40'C`|
| `f` | Show free RAM, EEPROM & Flash size | `RAM 1234 bytes free` |
| `u` | Show system uptime | `Uptime: 12.34 seconds` |
| `e` | Erase EEPROM (filled with zeros) | `Erasing... done` |
| `*` | Software Reboot | `Rebooting...` |
| `x` | Save current pin directions, states, and clocks to EEPROM | `Saved state to EEPROM` |
| `y` | Load and restore configuration from EEPROM | `Loaded state from EEPROM` |

### GPIO & Analog Commands

| Command | Description | Example |
|---|---|---|
| `p` | Detailed view of all port directions, states, PWM & Interrupt status | `p` |
| `q` | Compact matrix view of all ports (quick scan) | `q` |
| `< [pin]` | Set specified pin as **INPUT** | `< 3` (sets pin 3 to INPUT)|
| `> [pin]` | Set specified pin as **OUTPUT** | `> 4` (sets pin 4 to OUTPUT) |
| `/ [pin]` | Set pin value to **HIGH** | `/ 5` (sets pin 5 to HIGH) |
| `\ [pin]` | Set pin value to **LOW** | `\ 6` (sets pin 6 to LOW) |
| `^ [pin]` | Pulse pin: LOW -> HIGH -> LOW (single clock) | `^ 7` (sends clock pulse to pin 7) |
| `$` | Sequential pin sweep (flips pin state for 250ms then restores) | `$` |
| `c [pin] [ms]`| Start automated clock generator (toggles pin state every `ms`) | `c 8 100` (clocks pin 8 with 100ms interval) |
| `c [pin]` | Stop automated clock generator on specified pin | `c 8` |
| `a [pin]` | Read analog value and calculate voltage | `a a0` or `a 0` (reads from analog pin A0) |
| `aa [ms]` | Start continuous analog readings of all analog pins | `aa 100` (reads all analog inputs every 100ms) |
| `ar [bits]`| Set ADC reading resolution | `ar 12` (sets ADC resolution to 12 bits) |
| `ar` | Show current ADC reading resolution | `ar` |
| `g [pin] [val]`| Set analog / PWM output value (0-255) | `g 9 128` (sets PWM on pin 9 to 50% duty cycle)|
| `gg [freq]`| Set analog / PWM output frequency in Hz (RP2040/ESP8266) | `gg 1000` (sets PWM frequency to 1000Hz) |
| `s [pin] [deg]`| Set servo position angle (0-180) | `s 10 90` (sets servo on pin 10 to 90 degrees)|
| `z` | Global Reset: Set all ports to INPUT mode and write LOW | `z` |

### I2C Commands

| Command | Description | Example |
|---|---|---|
| `i` | Scan the I2C bus for connected device addresses | `i` |
| `# [addr]` | Set the specified I2C address as active | `# 0x27` (accepts Hex `0x27`, Bin `0b0100111`, or Dec `39`) |
| `r [count]` | Read `count` bytes from the active I2C device | `r 10` (reads 10 bytes from active device) |
| `w [bytes...]`| Write bytes to the active I2C device | `w 0x00 0x12 0xFF` (writes three bytes to active device) |

---

## 🛠️ Installation & Compilation

### Requirements
- **Arduino IDE** (v1.8.x or v2.x) OR **Arduino CLI**
- Dependencies: `Servo` library (usually built-in or easily installed via Library Manager).

### Using Arduino IDE
1. Download this repository as a `.zip` file or clone it into your `Arduino/libraries/` folder:
   ```bash
   git clone https://github.com/chatelao/MiniPirate.git
   ```
2. Open the Arduino IDE.
3. Navigate to **File** > **Examples** > **SwissHandmade MiniPirate** > **Minipirate**.
4. Select your target Board (e.g., Arduino Uno, Raspberry Pi Pico, or STM32 Nucleo-64) from **Tools** > **Board**.
5. Click **Upload** to build and flash.

### Using Arduino CLI
Ensure you have the core for your target architecture installed. For example, to compile for Arduino Uno:

```bash
# Install AVR core and dependencies
arduino-cli core install arduino:avr
arduino-cli lib install Servo

# Compile the sketch
arduino-cli compile --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino

# Upload the sketch (replace /dev/ttyACM0 with your board port)
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino
```

---

## 📜 License

This project is released under the MIT License. See [LICENSE.txt](LICENSE.txt) for more details.

---

## 🗺️ Roadmap

Interested in what is coming next? Check out [ROADMAP.md](ROADMAP.md) for planned future features, including SPI, UART, 1-Wire protocols, and logic analyzer capabilities!
