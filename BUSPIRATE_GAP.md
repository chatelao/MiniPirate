# MiniPirate vs. Bus Pirate: Feature & Syntax Gap Analysis

This document provides a comprehensive comparison between **MiniPirate** and the original **Bus Pirate** (v3, v4, and v5 series by Dangerous Prototypes / Bus Pirate team). It details command syntax differences, supported hardware features, protocol capabilities, architectural choices, and a roadmap gap analysis.

---

## 1. Overview & Architectural Philosophy

| Dimension | **MiniPirate** | **Bus Pirate** (v3 / v4 / v5) |
|---|---|---|
| **Primary Goal** | Lightweight, cross-platform CLI library turning standard maker MCUs into interactive serial protocols & hardware tools. | Dedicated open-source multi-protocol hardware hacking and bus analyzer tool. |
| **Hardware** | Generic Arduino boards (AVR, RP2040, RP2350, STM32, Renesas RA4M1, ESP32-C6). | Custom hardware with programmable power supplies, onboard pull-ups, voltage buffers, and display screens (BPv5). |
| **Syntax Philosophy** | Simple, space-delimited CLI commands (`< 3`, `> 4`, `w 0x00 0x12`, `r 10`, `c 8 100`). | Sequence/Macro-based bus syntax with inline brackets, repeats, and delay operators (`[ 0x90 0x00 r:10 ]`). |
| **Persistence** | Microcontroller internal EEPROM storage (`x` to save pin states/clocks, `y` to restore). | Configuration memory and mode settings saved to onboard flash / EEPROM across resets. |
| **Execution Model** | Direct execution per command line / non-blocking background pin clocks. | Sequential execution of chained syntax macros (`[` start, `]` stop, `r:5` read 5 bytes). |

---

## 2. Syntax Comparison Matrix

### 2.1 General & System Commands

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax (v3/v4/v5) | Notes |
|---|---|---|---|
| **Main Help** | `h` or `?` | `?` or `h` | MiniPirate also supports `h [cmd]` for extended help. |
| **Extended Help** | `h p`, `h g`, `h i`, etc. | `?` (context menu) | MiniPirate includes PROGMEM extended help per command. |
| **System Info / Status** | Boot printout / `f` (free RAM/Flash) | `i` | MiniPirate shows RAM, EEPROM, and Flash memory sizes. |
| **Voltage Measurement** | `v` | `v` / `V` | MiniPirate measures MCU VCC; Bus Pirate measures external ADC probe / rails. |
| **Internal Temperature** | `t` | `t` (v5) / `v` | MiniPirate measures internal MCU chip temperature (if supported). |
| **Uptime** | `u` | `u` | MiniPirate prints uptime in seconds (`millis()/1000.0`). |
| **Software Reset** | `*` | `#` | MiniPirate reboots MCU via zero-pointer jump; Bus Pirate resets state machine / MCU. |
| **Global Pin Reset** | `z` | `m 1` (HiZ mode) | MiniPirate sets all pins to INPUT & LOW and stops active clocks. |
| **Erase EEPROM** | `e` | `e` | MiniPirate fills EEPROM with zeros (`0`). |
| **Save State** | `x` | `x` / configuration menu | MiniPirate saves pin modes, states, and clocks to internal EEPROM. |
| **Restore State** | `y` | Restored at boot if configured | MiniPirate restores saved pin directions, states, and clock generators. |

---

### 2.2 Digital GPIO & Pin Manipulation

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax | Notes |
|---|---|---|---|
| **Port Status (Detailed)** | `p` | `v` / `i` | MiniPirate shows all digital/analog pins, directions, states, PWM & IRQ assignments. |
| **Port Status (Compact Matrix)**| `q` | `v` | MiniPirate prints a quick matrix of all pin directions and states. |
| **Set Pin INPUT** | `< [pin]` (e.g. `< 3`) | `@ [pin]` or mode config | MiniPirate accepts pin numbers or analog aliases (`< a0`). |
| **Set Pin OUTPUT** | `> [pin]` (e.g. `> 4`) | `O [pin]` or mode config | MiniPirate sets pin direction to OUTPUT. |
| **Set Pin HIGH** | `/ [pin]` (e.g. `/ 5`) | `H` or `1` / `a` / `A` / `w` | MiniPirate explicitly sets specified pin HIGH. |
| **Set Pin LOW** | `\ [pin]` (e.g. `\ 6`) | `L` or `0` / `a` / `A` / `w` | MiniPirate explicitly sets specified pin LOW. |
| **Pulse Pin (Clock Pulse)** | `^ [pin]` | `^` (tick clock) | MiniPirate pulses pin LOW -> HIGH -> LOW. |
| **Automated Clock Generator** | `c [pin] [ms]` (e.g. `c 8 100`) | Clock frequency commands (`g`) | MiniPirate runs non-blocking millisecond background clocks on any pin. |
| **Stop Clock Generator** | `c [pin]` (e.g. `c 8`) | `g` (disable) | MiniPirate stops background clocking on specified pin. |
| **Pin Sweep** | `$` | N/A | MiniPirate temporarily flips state of all pins sequentially for 250ms. |

---

### 2.3 Analog, PWM & Servo Control

| Feature / Action | MiniPirate Syntax | Bus Pirate Syntax | Notes |
|---|---|---|---|
| **Single ADC Reading** | `a [pin]` (e.g. `a a0`) | `d` / `D` / `v` | MiniPirate reads raw ADC and converts to Voltage (`VCC` scaled). |
| **Continuous ADC Reading** | `aa [ms]` (e.g. `aa 100`) | `D` (continuous voltage measure) | MiniPirate continuously prints all analog pin values at `ms` interval. |
| **ADC Resolution Control** | `ar [bits]` / `ar` | Fixed 10-bit / 12-bit per HW | MiniPirate dynamically configures MCU ADC resolution (e.g. 10/12-bit). |
| **PWM Output** | `g [pin] [val]` (e.g. `g 9 128`)| `g` (Frequency generator / PWM) | MiniPirate outputs PWM (0-255 duty cycle) on any PWM pin. |
| **PWM Frequency Control** | `gg [freq]` (e.g. `gg 1000`) | `g` menu | MiniPirate configures PWM frequency on supported chips (ESP8266/RP2040). |
| **Servo Motor Control** | `s [pin] [angle]` (e.g. `s 10 90`)| N/A (requires custom macros) | MiniPirate attaches servo motor and sets angle (0-180°). |

---

### 2.4 I2C Protocol Syntax

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

## 3. Bus Pirate Macro Syntax vs. MiniPirate Command Stream

The most significant syntax divergence between Bus Pirate and MiniPirate lies in how multi-byte protocol operations are formatted:

### Bus Pirate Macro Syntax
Bus Pirate combines start bits, data bytes, reads, delays, and stop bits into a single sequential macro expression:
```text
Bus Pirate: [ 0xA0 0x00 0x01 [ 0xA1 r:4 ]
```
- `[` = Send I2C Start Bit
- `0xA0 0x00 0x01` = Write device address and register pointer bytes
- `[` = Send Repeated Start Bit
- `0xA1` = Write read address
- `r:4` = Read 4 bytes with ACK, last byte NACK
- `]` = Send I2C Stop Bit

### MiniPirate Interactive Syntax
MiniPirate decomposes transactions into persistent target selection and direct action commands:
```text
MiniPirate:
> # 0x50       (Select target I2C device address 0x50)
> w 0x00 0x01  (Write register pointer bytes)
> r 4          (Read 4 bytes from current device)
```
*Benefits:* Highly readable for terminal users, less prone to syntax typo failures, non-blocking execution model.

---

## 4. Feature Gap Matrix & Protocol Support

| Capability / Feature | MiniPirate | Bus Pirate (v3/v4/v5) | Status / Roadmap |
|---|---|---|---|
| **I2C Master** | Yes | Yes | Fully supported in MiniPirate (`i`, `#`, `r`, `w`). |
| **SPI Master / Slave** | Planned (Roadmap) | Yes | MiniPirate has `m s` mode stub reserved. |
| **UART Terminal / Bridge** | Basic Serial CLI | Yes (Baud/Parity/Bridge) | Planned in MiniPirate Roadmap. |
| **1-Wire (DS18B20/EEPROM)** | Planned (Roadmap) | Yes | Planned in MiniPirate Roadmap. |
| **CAN Bus / JTAG / SWD** | Planned (Roadmap) | Yes | Bus Pirate v5 feature. |
| **Logic Analyzer / Sniffer** | Planned (Roadmap) | Yes (OLS / SUMP) | Planned in MiniPirate Roadmap. |
| **Pull-Up Resistor Control** | Internal MCU Pullups (`<`, `/`) | Onboard switched pullups | MiniPirate uses MCU pullup configuration (`pinMode(INPUT_PULLUP)`). |
| **Programmable Power Supply**| N/A (MCU board fixed) | Onboard 3.3V / 5.0V VREG | MiniPirate relies on board power rails (3.3V / 5V). |
| **Frequency Generator** | `c [pin] [ms]` / `gg [freq]` | `g` PWM module | MiniPirate supports millisecond background clocks on all digital pins. |
| **Servo Motor Support** | `s [pin] [deg]` | Requires custom script | Fully supported natively in MiniPirate CLI. |

---

## 5. Key Advantages of MiniPirate

1. **Zero Hardware Cost / Universal Availability**: Runs on any $2-$5 Arduino-compatible development board (RP2040, RP2350, STM32, AVR, ESP32-C6, RA4M1).
2. **Background Pin Clocking**: Non-blocking background clocking (`c 8 100`) allows multiple pins to run as automated clocks concurrently while using the CLI.
3. **Integrated Servo & PWM Control**: Native CLI commands for driving hobby servos (`s 10 90`) and frequency-controlled PWM outputs (`g 9 128`).
4. **Rich MCU Diagnostics**: Reports exact VCC voltage, chip internal temperature, free RAM, uptime, and flash sizes across ARM, AVR, RISC-V, and Renesas MCUs.
5. **Human-Readable Help System**: Extended built-in help per command (`h p`, `h g`, `h i`) stored in Flash memory to minimize RAM usage.

---

## 6. Recommendations for Future MiniPirate Syntax Convergence

To bridge the gap with Bus Pirate syntax while retaining MiniPirate's human-friendly CLI approach, the following enhancements are recommended:

1. **Inline Bracket Parser (`[ ... ]`)**: Add an optional Bus Pirate style bracket parser mode for I2C and SPI, enabling standard sequence expressions like `[ 0x50 0x00 r:4 ]`.
2. **SPI Protocol Engine (`m s`)**: Implement SPI master mode (`w`, `r`, chip select `/` `\`) under `m s`.
3. **UART Passthrough / Bridge (`m u`)**: Add configurable baud rate UART bridge mode with sniffing capabilities.
4. **1-Wire Module (`m 1`)**: Add DS18B20 temperature reading and ROM search macros.
5. **Configurable Pin Pullups**: Add an explicit pull-up command (e.g. `^ [pin]` or `p [pin] pullup`) to easily toggle MCU internal pull-up resistors.
