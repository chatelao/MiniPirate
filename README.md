# SwissHandmade MiniPirate

[![Arduino Library Build](https://github.com/chatelao/MiniPirate/workflows/Arduino%20Library%20Build/badge.svg)](https://github.com/chatelao/MiniPirate/actions)

**MiniPirate** turns any microcontroller into an interactive hardware Swiss Army Knife. Connect via serial terminal and immediately inspect I2C buses, toggle GPIOs, generate clock signals, sweep analog pins, control servos, and read real-time CPU telemetry — **all without writing or compiling a single line of code for each experiment.**

Inspired by the legendary **Bus Pirate**, MiniPirate is the ultimate lightweight utility for hardware hackers, electronics hobbyists, and embedded engineers to bring up new chips, debug sensors, and probe target boards in seconds.

---

## ✨ Feature Highlights

### 🔍 Instant I2C Bus Discovery & Packet Operations
Scan the entire I2C address space with a single keypress. Lock onto target devices and execute non-blocking multi-byte reads or writes directly from your terminal.
```text
> i
I2C SCAN...
  FOUND: 0x27 (PCF8574 I/O Expander)
  FOUND: 0x68 (DS3231 RTC)
> # 0x27
ACTIVE I2C ADDRESS: 0x27
> w 0x00 0xFF
I2C WRITE 2 BYTES OK
> r 4
I2C READ 4 BYTES: 0x12 0x34 0x56 0x78
```

### ⚡ On-the-Fly GPIO & PWM Manipulation
Instantly switch pin modes between INPUT and OUTPUT, drive pins HIGH or LOW, read digital or analog levels, and generate variable frequency PWM signals on demand.
```text
> > 5        # Set pin 5 to OUTPUT
> / 5        # Set pin 5 HIGH
> \ 5        # Set pin 5 LOW
> g 9 128    # Set pin 9 PWM duty cycle to 50% (128/255)
> a A0       # Read analog voltage on pin A0
A0: 512 (2.50V)
```

### ⏱️ Automated Clock Generator & Servo Controller
Run millisecond-precision background clock pulses on any GPIO pin or sweep servo motor angles directly without blocking serial interaction.
```text
> c 8 100    # Start automated 100ms clock toggle on pin 8
> s 10 90    # Move servo on pin 10 to 90 degrees
> c 8        # Stop clock generator on pin 8
```

### 🌡️ Real-Time On-Chip Telemetry & Diagnostics
Monitor chip health in real-time, including operational VCC voltage, internal silicon die temperature, uptime, and available SRAM memory.
```text
> v          # Read MCU supply voltage
VCC: 5.01V
> t          # Read internal CPU temperature
Temp: 25.40'C
> f          # Read memory stats
RAM: 1234 bytes free
```

### 💾 Non-Volatile EEPROM State Persistence
Save your complete hardware layout (pin directions, logic output states, active clock configurations) to onboard EEPROM and restore them automatically across power cycles.
```text
> x          # Save active configuration to EEPROM
EEPROM: Pin configurations saved.
> y          # Load configuration from EEPROM
EEPROM: Restored pin configuration.
```

---

## ⚡ Quick Start in 60 Seconds

1. **Plug in your board** via USB.
2. **Open any Serial Terminal** (Arduino Serial Monitor, PuTTY, TeraTerm, minicom, `screen`, or Web Serial).
3. **Configure Terminal Settings**:
   - **Baud Rate:** `57600`
   - **Line Ending:** Both `NL & CR` (or `LF / CR`)
4. Type `h` or `?` and press **Enter** to display the interactive help system!

---

## 🔌 Supported Boards & Architectures

MiniPirate runs natively across a broad range of 8-bit AVR, 32-bit ARM Cortex-M, ESP32 RISC-V, and Renesas microcontrollers:

| Platform | Board Variant | FQBN / Core | Flashing Guide |
|---|---|---|---|
| **AVR** | Arduino Uno, Nano, Mega | `arduino:avr:uno` | [HOWTO_FLASH_ARDUINO_UNO.md](HOWTO_FLASH_ARDUINO_UNO.md) |
| **RP2040** | Raspberry Pi Pico | `rp2040:rp2040:rpipico` | [HOWTO_FLASH_RASPBERRY_PI_PICO.md](HOWTO_FLASH_RASPBERRY_PI_PICO.md) |
| **RP2040** | Seeed Studio XIAO RP2040 | `rp2040:rp2040:seeed_xiao_rp2040` | [HOWTO_FLASH_SEEED_XIAO_RP2040.md](HOWTO_FLASH_SEEED_XIAO_RP2040.md) |
| **RP2350** | Raspberry Pi Pico 2 | `rp2040:rp2040:rpipico2` | [HOWTO_FLASH_RASPBERRY_PI_PICO_2.md](HOWTO_FLASH_RASPBERRY_PI_PICO_2.md) |
| **RP2350** | Seeed Studio XIAO RP2350 | `rp2040:rp2040:seeed_xiao_rp2350` | [HOWTO_FLASH_SEEED_XIAO_RP2350.md](HOWTO_FLASH_SEEED_XIAO_RP2350.md) |
| **ARM Cortex-M4** | STM32 Nucleo G431RB | `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G431RB` | [HOWTO_FLASH_STM32_NUCLEO_G431RB.md](HOWTO_FLASH_STM32_NUCLEO_G431RB.md) |
| **ARM Cortex-M4** | STM32 Nucleo F446RE | `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F446RE` | [HOWTO_FLASH_STM32_NUCLEO_F446RE.md](HOWTO_FLASH_STM32_NUCLEO_F446RE.md) |
| **ARM Cortex-M33** | STM32 Nucleo C562RE | `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_C562RE` | [HOWTO_FLASH_STM32_NUCLEO_C542RC.md](HOWTO_FLASH_STM32_NUCLEO_C542RC.md) |
| **Renesas RA4M1** | Seeed Studio XIAO RA4M1 | `Seeeduino:renesas_uno:XIAO_RA4M1` | [HOWTO_FLASH_SEEED_XIAO_RA4M1.md](HOWTO_FLASH_SEEED_XIAO_RA4M1.md) |
| **ESP32 RISC-V** | Seeed Studio XIAO ESP32-C6 | `esp32:esp32:XIAO_ESP32C6` | [HOWTO_FLASH_SEEED_XIAO_ESP32C6.md](HOWTO_FLASH_SEEED_XIAO_ESP32C6.md) |

---

## 📖 Complete Command Reference

Type `h <command>` (e.g. `h p` or `h g`) directly in the terminal for command-specific extended documentation.

### 🌐 System & Diagnostic Commands

| Command | Description | Example / Output |
|---|---|---|
| `h` / `?` | Display main interactive help menu | `h` |
| `h [cmd]` | Show command-specific extended help | `h p` |
| `v` | Read MCU operational supply voltage (VCC) | `VCC: 5.01V` |
| `t` | Read MCU internal die temperature sensor | `Temp: 25.40'C` |
| `f` | Read available free SRAM, EEPROM, and Flash space | `RAM 1234 bytes free` |
| `u` | Display system uptime since boot | `Uptime: 12.34 seconds` |
| `e` | Erase internal EEPROM state storage | `Erasing... done` |
| `*` | Trigger software system reboot | `Rebooting...` |
| `x` | Save active pin directions, states, and clocks to EEPROM | `Saved state to EEPROM` |
| `y` | Restore pin states and configurations from EEPROM | `Loaded state from EEPROM` |
| `w` | Scan visible WiFi networks (supported chips, e.g. ESP32) | `SCANNING WIFI NETWORKS...` |

### 🔌 Digital GPIO Commands

| Command | Description | Example |
|---|---|---|
| `p` | Detailed view of port directions, states, PWM & interrupts | `p` |
| `q` | Compact pin matrix quick scan | `q` |
| `< [pin]` | Configure pin as **INPUT** | `< 3` |
| `> [pin]` | Configure pin as **OUTPUT** | `> 4` |
| `/ [pin]` | Set output pin state **HIGH** | `/ 5` |
| `\ [pin]` | Set output pin state **LOW** | `\ 6` |
| `^ [pin]` | Pulse pin: LOW → HIGH → LOW (single pulse) | `^ 7` |
| `$` | Sequential pin sweep (flips each pin for 250ms and restores) | `$` |
| `c [pin] [ms]`| Start automated clock generator on `pin` with `ms` interval | `c 8 100` |
| `c [pin]` | Stop automated clock generator on `pin` | `c 8` |
| `z` | Global Reset: Set all GPIO pins to INPUT / LOW | `z` |

### 🎛️ Analog, PWM & Servo Commands

| Command | Description | Example |
|---|---|---|
| `a [pin]` | Read analog value and calculate voltage | `a A0` or `a 0` |
| `aa [ms]` | Start continuous sampling of all analog inputs | `aa 100` |
| `ar [bits]`| Set ADC resolution (e.g. 8, 10, 12 bits) | `ar 12` |
| `ar` | Show current ADC resolution setting | `ar` |
| `g [pin] [val]`| Set PWM output duty cycle (0–255) | `g 9 128` |
| `gg [freq]`| Set PWM base frequency in Hz (RP2040 / ESP8266) | `gg 1000` |
| `ggg` | Emergency PWM stop: Disable all active PWM channels | `ggg` |
| `s [pin] [deg]`| Set servo motor position angle (0–180°) | `s 10 90` |

### 🤝 I2C Protocol Commands

| Command | Description | Example |
|---|---|---|
| `i` | Scan I2C bus for connected target addresses | `i` |
| `# [addr]` | Select active target I2C address (Hex, Bin, or Dec) | `# 0x27` or `# 39` |
| `r [count]` | Read `count` bytes from active I2C device | `r 10` |
| `w [bytes...]`| Write byte sequence to active I2C device | `w 0x00 0x12 0xFF` |

---

## 🛠️ Installation & Compilation

### Requirements
- **Arduino IDE** (v1.8.x or v2.x) OR **Arduino CLI**
- Dependencies: `Servo` library (built-in or available via Library Manager).

### Using Arduino IDE
1. Clone or download this repository into your `Arduino/libraries/` folder:
   ```bash
   git clone https://github.com/chatelao/MiniPirate.git
   ```
2. Open the Arduino IDE.
3. Open **File** > **Examples** > **SwissHandmade MiniPirate** > **Minipirate**.
4. Select your board from **Tools** > **Board**.
5. Click **Upload**.

### Using Arduino CLI
```bash
# Install core & dependencies (e.g. AVR core)
arduino-cli core install arduino:avr
arduino-cli lib install Servo

# Compile sketch
arduino-cli compile --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino

# Flash board (adjust serial port)
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno examples/Minipirate/Minipirate.ino
```

---

## 🗺️ Roadmap

Check out [ROADMAP.md](ROADMAP.md) for planned future features, including SPI, UART, 1-Wire protocols, logic analyzer capabilities, and expanded chip support!

---

## 📜 License

This project is licensed under the MIT License. See [LICENSE.txt](LICENSE.txt) for details.
