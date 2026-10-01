# Unified Gap Analysis: Tigard, Bus Pirate, and MiniPirate

## 1. Executive Summary & Architectural Philosophy

This document provides a comprehensive comparative analysis across three major paradigms in open-source hardware hacking, debugging, and protocol analysis tools:

1. **Tigard** (designed by SecuringHardware): An **on-host USB bridge board** built on the FTDI FT2232H chip. It offloads all protocol execution logic to host utilities (e.g., OpenOCD, Flashrom, PyFTDI, Sigrok/PulseView) via high-speed MPSSE hardware engines.
2. **Bus Pirate** (v3, v4, and v5 series by Dangerous Prototypes / Bus Pirate team): A **dedicated open-source multi-protocol hardware hacking tool** running custom firmware on dedicated hardware. It features macro-driven bus syntax, programmable power supplies, on-board pull-ups, and voltage buffers.
3. **MiniPirate**: A **lightweight, cross-platform CLI library** turning standard, low-cost maker microcontrollers (AVR, RP2040, RP2350, STM32, Renesas RA4M1, ESP32-C6) into interactive serial hardware tools with human-readable CLI commands, background pin clocks, and rich MCU diagnostics.

---

### Architectural Comparison Matrix

| Dimension | Tigard (FT2232H) | Bus Pirate (v3 / v4 / v5) | MiniPirate |
|---|---|---|---|
| **Primary Architectural Role** | On-host High-Speed USB Bridge | Dedicated Multi-Protocol Bus Analyzer | Lightweight, Universal Microcontroller CLI Engine |
| **Core Hardware / Processor** | FTDI FT2232H (Dual USB 2.0 High-Speed Transceiver) | PIC24 (v3/v4) / RP2040 (v5) Dedicated Board | Generic Maker MCUs (ATmega328P, RP2040, RP2350, STM32, RA4M1, ESP32-C6) |
| **Execution Paradigm** | Host-driven MPSSE state machine (Host executes logic) | Custom firmware executing queued sequential syntax macros | Direct execution per human-readable CLI command line |
| **User Interface** | Host CLI / APIs (`flashrom`, `openocd`, `pyftdi`, `picocom`) | Dedicated VT100 Serial CLI / Display Screen (v5) | Standalone Serial Terminal (PuTTY, Minicom, Serial Monitor) |
| **Operating Voltages** | Flexible Level Shifting: **1.2V to 5.5V** | Programmable rails (3.3V/5V) & voltage buffers | Fixed 3.3V / 5.0V native I/O (VCC sensing on supported MCUs) |
| **USB Bandwidth** | USB 2.0 High-Speed (**480 Mbps**) | USB 2.0 Full-Speed CDC ACM (12 Mbps) | USB 2.0 Full-Speed CDC ACM / UART Serial (12 Mbps / 57.6k-115.2k CLI) |
| **Clock Frequencies** | Up to **30 MHz** (SPI/MPSSE), UART up to 12 Mbps | Up to 400kHz - 8MHz (I2C/SPI) | Up to 400kHz - 1MHz (I2C/SPI), background ms clock generator |
| **Debugging Protocols** | Native Hardware JTAG, SWD, SWO | Slow Bitbang JTAG (v3/v4), Hardware JTAG/SWD (v5) | Protocol/Logic interaction (JTAG/SWD planned via bitbang/PIO) |
| **Analog & Actuators** | None (pure digital bridge) | 1-channel ADC probe / VREG monitoring | Multi-channel ADC (`a`, `aa`), PWM (`g`, `gg`), Servo motor control (`s`) |
| **Persistence** | N/A (Host-driven configuration) | Onboard Flash / EEPROM configuration storage | Internal EEPROM state persistence (`x` save, `y` restore, `e` erase) |

---

## 2. Hardware & Protocol Feature Matrix

| Feature / Protocol | Tigard (FT2232H) | Bus Pirate v3/v4 | Bus Pirate v5 | MiniPirate |
|---|---|---|---|---|
| **Interactive Terminal Shell** | ❌ None (Requires host tools/scripts) | ✅ Built-in VT100 CLI | ✅ Built-in VT100 CLI + LCD | ✅ Human-Readable Serial CLI (`h`, `?`, `h [cmd]`) |
| **I2C Master** | ✅ PyFTDI / `i2c-tools` / libftdi | ✅ Standard Hardware/Bitbang | ✅ Native Hardware Engine | ✅ Device scan, active address select, read/write (`i`, `#`, `r`, `w`) |
| **SPI Master / Slave** | ✅ High-Speed MPSSE (up to 30 MHz) | ✅ Hardware / Bitbang SPI | ✅ High-Speed Hardware SPI | 🔄 Planned (Reserved stub `m s`) |
| **UART / Async Serial** | ✅ Dual Hardware UART channels (up to 12 Mbps) | ✅ Hardware UART Bridge | ✅ Hardware UART / Sniffer | 🔄 Terminal / Bridge mode planned |
| **1-Wire Protocol** | ❌ Bitbang via host API | ✅ Native 1-Wire Support | ✅ Native 1-Wire Support | 🔄 Planned |
| **JTAG Debugging** | ✅ Native MPSSE OpenOCD Support | ⚠️ Slow Bitbang JTAG | ✅ Hardware JTAG Engine | 🔄 Planned via PIO/Bitbang |
| **SWD Debugging** | ✅ Native OpenOCD / OpenSWD Support | ❌ No native SWD | ✅ Hardware SWD Engine | 🔄 Planned via PIO/Bitbang |
| **Logic Analyzer / Sniffing** | ✅ Sigrok / PulseView integration | ✅ 5-channel SUMP Logic Sniffer | ✅ High-speed Logic Analyzer | 🔄 Planned (SUMP / CDC integration) |
| **GPIO Control** | ✅ PyFTDI GPIO API / `gpiod` | ✅ Interactive Pin Commands | ✅ Interactive Pin Commands | ✅ Interactive CLI (`<`, `>`, `/`, `\`, `^`, `$`, `z`) |
| **ADC / Voltage Measure** | ❌ Not available | ✅ 1-channel ADC probe | ✅ Multi-channel ADC | ✅ Multi-channel ADC (`a`, `aa`, `ar`), MCU VCC (`v`) |
| **PWM Output** | ❌ Not available | ✅ 1-channel PWM generator | ✅ Multi-channel PWM | ✅ Configurable PWM (`g`, `gg`), Background Clock (`c`) |
| **Servo Motor Control** | ❌ Not available | ❌ Requires custom macros | ❌ Requires custom macros | ✅ Direct Servo motor control (`s [pin] [deg]`) |
| **Internal Diagnostics** | ❌ N/A (FTDI chip) | ⚠️ Limited VREG monitoring | ✅ Rail monitoring | ✅ VCC (`v`), Chip Temp (`t`), Free RAM (`f`), Uptime (`u`) |
| **Level Shifting** | ✅ Adjustable 1.2V - 5.5V shifters | ❌ Fixed 3.3V / 5.0V | ✅ Programmable 1.2V - 5.0V | ❌ Target-native logic levels (3.3V or 5V) |
| **Power Supply Control** | ❌ Fixed board rails | ✅ Switched 3.3V / 5.0V rails | ✅ Programmable 1.2V - 5.0V VREG | ❌ Relies on board power rails (3.3V / 5V) |
| **State Persistence** | ❌ N/A | ✅ Save/Restore settings | ✅ Save/Restore settings | ✅ EEPROM state persistence (`x`, `y`, `e`) |

---

## 3. Syntax & Interaction Models

### 3.1 Tigard Host-Driven Command Paradigm
Tigard does not expose a command-line prompt directly over serial for hardware manipulation. Instead, users invoke specialized host applications that communicate FTDI MPSSE or UART commands to Channel A (JTAG/SPI/I2C/SWD) and Channel B (UART/GPIO):

```bash
# Example 1: I2C Scanning / Interaction via PyFTDI / i2c-tools
pyftdi-i2c scan ft232h://ftdi:2232:tigard/1

# Example 2: SPI Flash Dumping via Flashrom
flashrom -p ft2232_spi:type=2232H,port=A -r backup.bin

# Example 3: JTAG Target Debugging via OpenOCD
openocd -f interface/ftdi/tigard.cfg -f target/stm32f4x.cfg

# Example 4: Serial Console via Minicom or Picocom
picocom -b 115200 /dev/ttyUSB1
```

---

### 3.2 Bus Pirate Macro Syntax Paradigm
Bus Pirate combines start bits, data bytes, reads, delays, and stop bits into a single sequential macro expression:

```text
Bus Pirate Macro Expression:
[ 0xA0 0x00 0x01 [ 0xA1 r:4 ]
```
- `[` = Send I2C Start Bit
- `0xA0 0x00 0x01` = Write device address and register pointer bytes
- `[` = Send Repeated Start Bit
- `0xA1` = Write read address
- `r:4` = Read 4 bytes with ACK, last byte NACK
- `]` = Send I2C Stop Bit

---

### 3.3 MiniPirate Interactive CLI Paradigm
MiniPirate decomposes transactions into persistent target selection and direct human-readable commands:

```text
MiniPirate Serial CLI Output:

MiniPirate: v0.3
Device has 14 digital pins and 6 analog pins.
CPU is set to 16.00Mhz

> i
Scanning I2C bus...
Device found at address 0x27 (39)

> # 0x27
Selected I2C device address: 0x27

> w 0x00 0x12
Writing to I2C device 0x27: 0x00 0x12

> r 4
Reading 4 bytes from 0x27: 0x10 0x20 0x30 0x40

> > 3
Pin 3 is now OUTPUT

> / 3
New value on pin 3 : HIGH

> c 8 100
Clocking pin 8 with delay of 100ms

> a a0
Analog value on pin A0: 512 / 2.50V

> s 10 90
New servo value on pin 10: 90
```

---

### 3.4 Detailed Command Syntax Mapping (MiniPirate vs. Bus Pirate)

#### System & Diagnostic Commands

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax (v3/v4/v5) | Notes |
|---|---|---|---|
| **Main Help** | `h` or `?` | `?` or `h` | MiniPirate supports `h [cmd]` for extended help. |
| **Extended Help** | `h p`, `h g`, `h i`, etc. | `?` (context menu) | MiniPirate stores extended help in Flash (PROGMEM). |
| **System Info / Status** | Boot printout / `f` (free RAM/Flash) | `i` | MiniPirate displays RAM, EEPROM, and Flash memory sizes. |
| **Voltage Measurement** | `v` | `v` / `V` | MiniPirate measures MCU VCC; Bus Pirate measures external ADC probe/rails. |
| **Internal Temperature** | `t` | `t` (v5) / `v` | MiniPirate measures internal MCU chip temperature (if supported). |
| **Uptime** | `u` | `u` | MiniPirate prints uptime in seconds (`millis()/1000.0`). |
| **Software Reset** | `*` | `#` | MiniPirate reboots MCU via zero-pointer jump; Bus Pirate resets state machine / MCU. |
| **Global Pin Reset** | `z` | `m 1` (HiZ mode) | MiniPirate sets all pins to INPUT & LOW and stops active clocks. |
| **Erase EEPROM** | `e` | `e` | MiniPirate fills internal EEPROM with zeros (`0`). |
| **Save State** | `x` | `x` / configuration menu | MiniPirate saves pin modes, states, and clocks to EEPROM. |
| **Restore State** | `y` | Restored at boot if configured | MiniPirate restores saved pin directions, states, and clock generators. |

#### Digital GPIO & Pin Control

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax | Notes |
|---|---|---|---|
| **Port Status (Detailed)** | `p` | `v` / `i` | MiniPirate shows all digital/analog pins, directions, states, PWM & IRQ assignments. |
| **Port Status (Compact Matrix)**| `q` | `v` | MiniPirate prints a quick matrix of pin directions and states. |
| **Set Pin INPUT** | `< [pin]` (e.g. `< 3`) | `@ [pin]` or mode config | MiniPirate accepts pin numbers or analog aliases (`< a0`). |
| **Set Pin OUTPUT** | `> [pin]` (e.g. `> 4`) | `O [pin]` or mode config | MiniPirate sets pin direction to OUTPUT. |
| **Set Pin HIGH** | `/ [pin]` (e.g. `/ 5`) | `H` or `1` / `a` / `A` / `w` | MiniPirate explicitly sets specified pin HIGH. |
| **Set Pin LOW** | `\ [pin]` (e.g. `\ 6`) | `L` or `0` / `a` / `A` / `w` | MiniPirate explicitly sets specified pin LOW. |
| **Pulse Pin (Clock Pulse)** | `^ [pin]` | `^` (tick clock) | MiniPirate pulses pin LOW -> HIGH -> LOW. |
| **Automated Clock Generator** | `c [pin] [ms]` (e.g. `c 8 100`) | Clock frequency commands (`g`) | MiniPirate runs non-blocking millisecond background clocks on any pin. |
| **Stop Clock Generator** | `c [pin]` (e.g. `c 8`) | `g` (disable) | MiniPirate stops background clocking on specified pin. |
| **Pin Sweep** | `$` | N/A | MiniPirate temporarily flips state of all pins sequentially for 250ms. |

#### Analog, PWM & Servo Control

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax | Notes |
|---|---|---|---|
| **Single ADC Reading** | `a [pin]` (e.g. `a a0`) | `d` / `D` / `v` | MiniPirate reads raw ADC and converts to Voltage (`VCC` scaled). |
| **Continuous ADC Reading** | `aa [ms]` (e.g. `aa 100`) | `D` (continuous voltage measure) | MiniPirate continuously prints all analog pin values at `ms` interval. |
| **ADC Resolution Control** | `ar [bits]` / `ar` | Fixed 10-bit / 12-bit per HW | MiniPirate dynamically configures MCU ADC resolution (e.g. 10/12-bit). |
| **PWM Output** | `g [pin] [val]` (e.g. `g 9 128`)| `g` (Frequency generator / PWM) | MiniPirate outputs PWM (0-255 duty cycle) on any PWM pin. |
| **PWM Frequency Control** | `gg [freq]` (e.g. `gg 1000`) | `g` menu | MiniPirate configures PWM frequency on supported chips (ESP8266/RP2040). |
| **Servo Motor Control** | `s [pin] [angle]` (e.g. `s 10 90`)| N/A (requires custom macros) | MiniPirate attaches servo motor and sets angle (0-180°). |

#### I2C Protocol Syntax

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax | Notes |
|---|---|---|---|
| **Enable I2C Mode** | `m i` or `i` | `m` -> Select `I2C` | MiniPirate activates I2C mode and automatically scans bus. |
| **I2C Address Scan** | `i` | `(1)` macro / `search` | MiniPirate lists all responding addresses in Hex and Binary. |
| **Select Active Address** | `# [addr]` (e.g. `# 0x27`, `# 39`) | Inlined in stream (`[ 0xA0 ... ]`) | MiniPirate retains active device selection for subsequent reads/writes. |
| **Select Active Device Index**| `1`, `2`, `3` ... | N/A | MiniPirate allows choosing device from scan list index. |
| **Read Bytes** | `r [count]` (e.g. `r 10`) | `r` or `r:10` | MiniPirate reads `count` bytes from selected active device address. |
| **Write Bytes** | `w [b1] [b2] ...` (e.g. `w 0x00 0x12`) | `0xXX` / `0bXX` / `123` | MiniPirate accepts space-separated Hex, Dec, or Bin byte lists. |
| **I2C Start Bit** | Automated in `w` / `r` | `[` | Bus Pirate uses explicit `[` macro bit. |
| **I2C Stop Bit** | Automated in `w` / `r` | `]` | Bus Pirate uses explicit `]` macro bit. |
| **I2C ACK / NAK Handling** | Automatic HAL Wire | `r` (ACK) / `r` last byte (NACK) | Bus Pirate provides bit-level ACK/NACK control. |

---

## 4. Gap & Ecosystem Analysis

```
                       ┌──────────────────────────────────────────────────────────┐
                       │                     HARDWARE TOOLING                     │
                       └────────────────────────────┬─────────────────────────────┘
                                                    │
       ┌────────────────────────────────────────────┼────────────────────────────────────────────┐
       ▼                                            ▼                                            ▼
 ┌─────────────────────────┐              ┌─────────────────────────┐              ┌─────────────────────────┐
 │         TIGARD          │              │       BUS PIRATE        │              │       MINIPIRATE        │
 │  (FT2232H USB Bridge)   │              │   (Dedicated Hacking)   │              │  (Universal MCU CLI)    │
 └────────────┬────────────┘              └────────────┬────────────┘              └────────────┬────────────┘
              │                                        │                                        │
 ┌────────────┴────────────┐              ┌────────────┴────────────┐              ┌────────────┴────────────┐
 ▼                         ▼              ▼                         ▼              ▼                         ▼
┌───────────┐         ┌───────────┐      ┌───────────┐         ┌───────────┐      ┌───────────┐         ┌───────────┐
│ High-Speed│         │ Multi-Volt│      │ Macro     │         │ Switched  │      │ Standalone│         │ Mixed-Sig │
│ JTAG/SWD/ │         │ 1.2V-5.5V │      │ Protocol  │         │ Power &   │      │ Serial CLI│         │ ADC, PWM, │
│ MPSSE SPI │         │ Leveling  │      │ Syntax    │         │ Pullups   │      │ Universal │         │ Servos    │
└───────────┘         └───────────┘      └───────────┘         └───────────┘      └───────────┘         └───────────┘
```

### Key Gaps: What Tigard Provides (MiniPirate & Bus Pirate Lack)
1. **High-Speed MPSSE Engine:** Tigard achieves up to 30 MHz SPI and high-speed JTAG transfers using FTDI hardware state machines, avoiding microcontroller bit-banging limits.
2. **Native SWD & JTAG Target Debugging:** Direct out-of-the-box integration with OpenOCD and GDB for flashing and ARM Cortex / MIPS / RISC-V silicon debugging.
3. **Wide Target Voltage Range (1.2V - 5.5V):** On-board bidirectional level shifters allow safe interfacing with low-voltage application processors (1.2V/1.8V/2.5V/3.3V/5V).
4. **Standard Toolchain Integration:** Works natively with host tools like `flashrom`, `openocd`, `sigrok-cli`, and `PyFTDI`.

### Key Gaps: What Bus Pirate Provides (MiniPirate & Tigard Lack)
1. **Sequential Syntax Macro Expressions:** Inlined start/stop/read/write sequences (`[ 0xA0 0x00 r:4 ]`) allow complex multi-step bus interactions in a single line.
2. **On-Board Switched Pullups & Power Rails:** Controllable 3.3V / 5.0V power supplies and onboard pull-up resistors for open-drain buses.
3. **Dedicated Protocol Engines:** Hardware and bitbang support for 1-Wire, SPI, UART bridge, and CAN bus.

### Key Gaps: What MiniPirate Provides (Tigard & Bus Pirate Lack)
1. **Zero Hardware Cost & Universal Availability:** Runs on any $2–$5 standard development board (RP2040, RP2350, STM32, AVR, ESP32-C6, Renesas RA4M1) without specialized boards.
2. **Background Non-Blocking Pin Clocks:** Automated background clock generator (`c 8 100`) allows running multiple automated clocks concurrently while continuing CLI interactions.
3. **Integrated Servo Motor & Actuator Control:** Native CLI commands for driving hobby servos (`s 10 90`) and frequency-controlled PWM (`g 9 128`, `gg 1000`).
4. **Deep MCU Diagnostics:** Displays exact VCC voltage (`v`), chip internal temperature (`t`), free SRAM (`f`), uptime (`u`), and Flash/EEPROM geometry across ARM, AVR, RISC-V, and Renesas MCUs.
5. **Human-Readable Flash Help System:** Command-specific extended help guides (`h p`, `h g`, `h i`) stored in Flash memory (PROGMEM) to conserve precious SRAM.

---

## 5. Strategic Convergence Roadmap for MiniPirate

To bridge feature gaps with both FT2232H-based tools (like Tigard) and dedicated multi-protocol tools (like Bus Pirate) while maintaining MiniPirate's zero-cost, universal MCU philosophy, the following roadmap enhancements are planned:

### 5.1 High-Speed SPI & UART Engines
- Implement dedicated hardware SPI master commands (`m s`, `w`, `r`, chip select `/` `\`) utilizing hardware SPI peripherals across AVR, RP2040/RP2350, STM32, and ESP32.
- Implement UART bridge and pass-through terminal mode (`m u`) with configurable baud rates and sniffer capabilities.

### 5.2 Bitbang & PIO JTAG / SWD Support
- Leverage RP2040 / RP2350 Programmable I/O (PIO) state machines or bit-banging to provide SWD line probing and JTAG boundary scan primitives inside the CLI shell.

### 5.3 Inline Bracket Syntax Parser (`[ ... ]`)
- Add an optional Bus Pirate style bracket parser for I2C and SPI modes, enabling standard sequence expressions (e.g., `[ 0x50 0x00 r:4 ]`) alongside MiniPirate's interactive command stream.

### 5.4 1-Wire Protocol Module (`m 1`)
- Integrate 1-Wire bus discovery, ROM search macros, and DS18B20 temperature reading functions.

### 5.5 Logic Analyzer & Sigrok SUMP Integration
- Implement Sigrok SUMP / Logic protocol support over USB CDC ACM, turning MiniPirate into a PulseView-compatible logic analyzer.

### 5.6 Hardware Expansion Shield Specification
- Design an open-source MiniPirate expansion shield/companion board featuring adjustable buck/boost converters, target voltage sensing, and bidirectional level shifters (1.2V – 5.0V).
