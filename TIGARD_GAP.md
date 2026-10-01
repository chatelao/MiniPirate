# Tigard vs. Bus Pirate & MiniPirate: Feature & Syntax Gap Analysis

## 1. Executive Summary

This document provides a comprehensive comparative analysis between **Tigard** (an open-source FT2232H-based multi-protocol hardware tool designed by SecuringHardware) and the **Bus Pirate** / **MiniPirate** ecosystem (microcontroller-based interactive CLI hardware tools).

While both tools target hardware debugging, reverse engineering, and sensor/peripheral interfacing, they rely on fundamentally different architectural paradigms:
- **Tigard** is an **on-host USB bridge board** built on the FTDI FT2232H chip. It relies on standard host utilities (e.g., OpenOCD, Flashrom, PyFTDI, Sigrok/PulseView) for protocol execution, high-speed MPSSE transfers, and boundary scanning.
- **Bus Pirate & MiniPirate** are **microcontroller-based interactive CLI devices** running custom firmware. They provide a human-readable serial terminal shell (`i`, `#`, `w`, `r`, `<`, `>`, `/`, `\`, `a`, `g`, `s`, `v`, `t`, `f`, `u`) for standalone interactive experimentation without needing specialized drivers or Python packages on the host.

---

## 2. Architectural Comparison

| Dimension | Tigard | Bus Pirate / MiniPirate |
|---|---|---|
| **Core Processor / Chip** | FTDI FT2232H (Dual High-Speed USB Bridge) | ATmega328P / RP2040 / RP2350 / STM32 / RA4M1 / ESP32 MCU |
| **Execution Paradigm** | Host-driven MPSSE engine (Host executes logic, sends USB packets) | On-chip microcontroller execution (Firmware handles timing, CLI, and peripherals) |
| **User Interface** | Host software CLI / APIs (OpenOCD, Flashrom, PyFTDI, Serial) | Standalone Serial Terminal (PuTTY, Minicom, Serial Monitor) |
| **Operating Voltages** | Flexible Level Shifting: **1.2V to 5.5V** (VTARGET sensing or internal supply) | Fixed 3.3V / 5.0V native I/O (VCC sensing on supported MCUs) |
| **USB Bandwidth** | USB 2.0 High-Speed (480 Mbps transceiver) | USB 2.0 Full-Speed CDC ACM or UART Serial Bridge (12 Mbps / 57.6k-115.2k baud CLI) |
| **Clock Frequencies** | Up to **30 MHz** (SPI/MPSSE), high-baud UART (up to 12 Mbps) | Up to 400kHz - 1MHz (I2C/SPI), software/timer clock output (ms-resolution) |
| **Debugging Protocols** | Native hardware JTAG, SWD, SWO | Logic/Protocol interaction (JTAG/SWD planned via bitbang/PIO) |
| **Analog & Actuators** | None (pure digital bridge) | ADC pin reading, PWM output, Servo motor PWM, internal temp/VCC |

---

## 3. Protocol & Hardware Feature Comparison

### Feature Comparison Matrix

| Protocol / Feature | Tigard (FT2232H) | Bus Pirate v3/v4 | MiniPirate |
|---|---|---|---|
| **Interactive Terminal Shell** | ❌ None (Relies on host CLI/scripts) | ✅ Built-in VT100 CLI | ✅ Built-in Human-Readable Serial CLI |
| **I2C Master** | ✅ PyFTDI / `i2c-tools` / libftdi | ✅ Standard Bitbang/Hardware I2C | ✅ Active device selection, read/write, scanning (`i`, `#`, `r`, `w`) |
| **SPI Master** | ✅ High-Speed MPSSE (up to 30 MHz) | ✅ Standard Hardware/Software SPI | 🔄 Planned (See `ROADMAP.md`) |
| **UART / Async Serial** | ✅ Dual Hardware UART channels (up to 12 Mbps) | ✅ Hardware UART Bridge | 🔄 Terminal mode / Bridge mode planned |
| **JTAG Debugging** | ✅ Native MPSSE OpenOCD Support | ⚠️ Slow Bitbang JTAG | 🔄 Planned via PIO/Bitbang |
| **SWD Debugging** | ✅ Native OpenOCD / OpenSWD Support | ❌ No native SWD | 🔄 Planned via PIO/Bitbang |
| **Logic Analyzer / Sniffing** | ✅ Sigrok / PulseView integration | ✅ 5-channel 4MHz Logic Sniffer | 🔄 Planned |
| **GPIO Pin Control** | ✅ PyFTDI GPIO API / `gpiod` | ✅ Interactive CLI Pin Commands | ✅ Interactive CLI (`<`, `>`, `/`, `\`, `^`, `$`, `c`) |
| **ADC / Voltage Measurement** | ❌ Not available | ✅ 1-channel ADC pin | ✅ Multi-channel ADC (`a`, `aa`, `ar`), calculated VCC (`v`) |
| **PWM / Waveform Output** | ❌ Not available | ✅ 1-channel PWM generator | ✅ Configurable PWM (`g`, `gg`), Automated Clock (`c`) |
| **Servo Motor Control** | ❌ Not available | ❌ Not available | ✅ Direct Servo control (`s`) |
| **Internal MCU Diagnostics** | ❌ N/A (FTDI chip) | ⚠️ Limited VREG monitoring | ✅ VCC (`v`), Chip Temp (`t`), Free RAM (`f`), Uptime (`u`) |
| **Target Voltage Translation** | ✅ Adjustable 1.2V - 5.5V level shifters | ❌ Fixed 3.3V / 5.0V | ❌ Target-native logic levels (3.3V or 5V) |
| **EEPROM State Persistence** | ❌ N/A | ✅ Save/Restore settings | ✅ Persistent Pin/Clock states (`x`, `y`, `e`) |

---

## 4. Syntax & Interaction Model Comparison

### Tigard Host-Driven Command Paradigm
Tigard does not expose a command-line prompt directly over serial for hardware manipulation. Instead, users invoke specialized host applications that speak FTDI MPSSE or UART commands to Channel A (JTAG/SPI/I2C/SWD) and Channel B (UART/GPIO):

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

### Bus Pirate / MiniPirate Interactive CLI Paradigm
Bus Pirate and MiniPirate use a single serial terminal connection to execute interactive commands directly on the hardware:

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

## 5. Feature & Syntax Gap Analysis

```
                       ┌──────────────────────────────────────────────────────────┐
                       │                     HARDWARE TOOLING                     │
                       └────────────────────────────┬─────────────────────────────┘
                                                    │
                      ┌─────────────────────────────┴─────────────────────────────┐
                      ▼                                                           ▼
         ┌─────────────────────────┐                                 ┌─────────────────────────┐
         │         TIGARD          │                                 │  BUS PIRATE / MINIPIRATE│
         │  (FT2232H USB Bridge)   │                                 │  (Microcontroller CLI)  │
         └────────────┬────────────┘                                 └────────────┬────────────┘
                      │                                                           │
       ┌──────────────┴──────────────┐                             ┌──────────────┴──────────────┐
       ▼                             ▼                             ▼                             ▼
 ┌───────────┐                 ┌───────────┐                 ┌───────────┐                 ┌───────────┐
 │ High-Speed│                 │ Multi-Volt│                 │ Standalone│                 │ Analog,   │
 │ JTAG/SWD/ │                 │ 1.2V-5.5V │                 │ Serial CLI│                 │ PWM, ADC, │
 │ MPSSE SPI │                 │ Translation│                │ Interactive│                │ Servos    │
 └───────────┘                 └───────────┘                 └───────────┘                 └───────────┘
```

### Key Gaps: What Tigard Provides (MiniPirate Lacks)
1. **High-Speed MPSSE Hardware Engine:** Tigard achieves up to 30 MHz SPI and high-speed JTAG transfers using FTDI hardware state machines, whereas microcontroller bit-banging is speed-limited.
2. **Native SWD & JTAG Target Debugging:** Direct integration with OpenOCD and GDB for flashing and ARM Cortex / MIPS / RISC-V silicon debugging.
3. **Wide Target Voltage Range (1.2V - 5.5V):** Directional logic level translators (e.g., NXP NTB0104 / Texas Instruments TXB/TXS series) allow safe interfacing with low-voltage application processors (1.2V/1.8V/2.5V/3.3V/5V).
4. **Standard Toolchain Integration:** Works natively with standard tools like `flashrom`, `openocd`, `sigrok-cli`, and `PyFTDI` out of the box.

### Key Gaps: What Bus Pirate / MiniPirate Provides (Tigard Lacks)
1. **Zero-Driver Standalone Terminal CLI:** No Python dependencies, specialized FTDI drivers, or complex host software setup required; operates with standard serial software (`screen`, `picocom`, PuTTY, Arduino Serial Monitor).
2. **Analog & Mixed-Signal Capabilities:** Multi-channel ADC sampling (`a`, `aa`), configurable resolution (`ar`), PWM output (`g`, `gg`), and Servo motor positioning (`s`).
3. **Automated Hardware Signal Generation:** Automated clock generation (`c`), GPIO pulse creation (`^`), and sequential pin sweeps (`$`).
4. **On-Chip Diagnostics & State Persistence:** Microcontroller VCC (`v`), internal temperature (`t`), SRAM tracking (`f`), uptime (`u`), and non-volatile state restoration across reboots (`x`, `y`, `e`).

---

## 6. Strategic Recommendations & Roadmap for MiniPirate

To bridge the feature gap between MiniPirate and FT2232H-based tools like Tigard while retaining MiniPirate's superior standalone CLI usability, the following roadmap enhancements are recommended:

1. **High-Speed Hardware SPI & UART Bridging:** Implement dedicated hardware SPI master commands (`ms`, `r`, `w`) and UART pass-through bridge mode to utilize MCU hardware SPI/UART peripherals.
2. **Bitbang / PIO JTAG & SWD Support:** Leverage RP2040/RP2350 Programmable I/O (PIO) or state machines to provide SWD line probing and JTAG boundary scan primitives within the CLI.
3. **Hardware Level Shifting Expansion Shield:** Design an optional MiniPirate hardware companion board/shield featuring adjustable buck/boost converters and bidirectional level shifters (1.2V - 5.0V).
4. **Logic Analyzer Integration:** Implement Sigrok SUMP/Logic protocol support over USB CDC to allow PulseView visualization directly from MiniPirate capture buffers.
