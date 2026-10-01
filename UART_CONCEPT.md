# MiniPirate UART Concept & Architecture Specification

This document outlines the architecture, use cases, hardware considerations, and command specification for configuring **USB-Serial speeds** and **hardware UART properties** on microcontrollers supported by MiniPirate.

---

## 1. Overview & Objectives

MiniPirate serves as a human-readable serial protocol tool and direct hardware interface. A core capability required for broad embedded system diagnosis, sensor interfacing, and hardware hacking is flexible **UART (Universal Asynchronous Receiver-Transmitter)** control.

UART interfaces in MiniPirate operate across two distinct domains:

1. **USB-Serial CLI Domain:** The connection between the host PC and the microcontroller running MiniPirate (used for command input, status output, and data terminal interactions).
2. **Hardware UART Peripheral Domain:** The physical serial hardware pins (`TX` / `RX`, and optional `RTS` / `CTS`) on the microcontroller used to communicate with target target chips, sensors, modules, and external serial buses.

```
+------------------+         USB CDC / Serial          +--------------------+         Hardware UART          +--------------------+
|                  | <===============================> |                    | <==============================> |                    |
|  Host PC / CLI   |   (e.g., 57600, 115200 baud)      |  MiniPirate MCU    |   (Custom Baud, 8N1, 7E1, etc.)  | External Device    |
|  Terminal        |                                   |  (RP2040/AVR/STM32) |                                  | (GPS, MIDI, ESP32) |
+------------------+                                   +--------------------+                                  +--------------------+
```

### Key Objectives
- Support configurable USB-Serial CLI baud rates to accommodate high-throughput logging or low-speed noise-immune host connections.
- Provide comprehensive hardware UART control including custom baud rates, data framing (5–9 bits), parity modes (None, Even, Odd, Mark, Space), stop bits (1, 1.5, 2), flow control (RTS/CTS, XON/XOFF), and signal inversion.
- Enable specialized UART operational sub-modes: **Transparent Bridge**, **Passive Sniffer**, **Loopback Self-Test**, and **Break Condition Generation**.
- Abstract platform-specific hardware differences across AVR, RP2040, RP2350, STM32, Renesas RA4M1, and ESP32 architectures.

---

## 2. USB-Serial CLI Interface Speed Use Cases

The USB-Serial connection carries the MiniPirate command prompt and output response logs. Changing the USB-Serial CLI baud rate addresses several specific operational requirements:

### A. High-Speed Data Streaming & Real-Time Logging
- **Continuous ADC / Sensor Dumping (`aa` command):** High baud rates (e.g., `115200`, `230400`, `460800`, or `921600` baud) prevent serial transmission bottlenecks when continuously sampling multi-channel analog inputs at fast intervals (e.g., every 1ms).
- **Rapid Pin Sweeps & Logic Monitoring (`$` and `p` commands):** Facilitates instantaneous terminal updates during rapid pin status dumps or automated clock monitoring.

### B. Host Terminal Compatibility & Legacy Connections
- **Legacy Terminal Software & Industrial PCs:** Older terminal equipment, embedded host controllers, or serial-to-Bluetooth bridges may fixedly operate at `9600`, `19200`, or `38400` baud.
- **Default Speed Consistency:** Default boot baud rate remains `57600` baud for backward compatibility with classic Bus Pirate workflows, while allowing dynamic runtime switching via CLI commands.

### C. Noise-Prone Environments & Long Cable Runs
- **Electrically Noisy Environments:** Lowering USB-UART speeds (e.g., to `9600` or `19200` baud) improves noise margin and data integrity when operating over long unshielded USB-to-UART converter cables or optocoupled galvanic isolators.

---

## 3. Hardware UART Peripheral Configuration Use Cases

When MiniPirate interacts with external targets via its hardware UART peripheral, precise physical layer control is required to match target device specifications.

```
                               UART Parameter Configuration
┌─────────────────────────┬─────────────────────────────┬────────────────────────────────────────────────────────┐
│ Parameter               │ Supported Values            │ Common Use Cases & Applications                        │
├─────────────────────────┼─────────────────────────────┼────────────────────────────────────────────────────────┤
│ Baud Rate               │ 300 to 3,000,000+ bps       │ Standard & non-standard baud rates (GPS, MIDI, ESP)    │
│ Data Bits               │ 5, 6, 7, 8, 9 bits          │ 8 bits (standard), 7 bits (ASCII), 9 bits (MPCB/Addr)  │
│ Parity                  │ None (N), Even (E), Odd (O) │ N (standard), E/O (industrial, Modbus RTU, smart meters)│
│ Stop Bits               │ 1, 1.5, 2 stop bits         │ 1 (standard), 2 (legacy telecom, high-reliability)     │
│ Flow Control            │ None, Hardware, Software    │ Hardware (RTS/CTS), Software (XON/XOFF)                │
│ Signal Inversion        │ Normal, Inverted            │ Active-low logic, SBUS RC receivers, raw RS-232        │
└─────────────────────────┴─────────────────────────────┴────────────────────────────────────────────────────────┘
```

### A. Custom Baud Rate Selection
Different external modules require non-standard or specific baud rates:
- **`31250` baud:** Standard MIDI protocol for musical instruments and synthesizers.
- **`74880` baud:** Native bootloader logging speed for ESP8266 microcontrollers.
- **`9600` baud:** Standard default for NMEA GPS receivers, HC-05/06 Bluetooth modules in data mode, and SIM800/SIM900 GSM modules.
- **`38400` baud:** Command mode (AT mode) for HC-05 Bluetooth modules.
- **`115200` baud:** Default debug log speed for ESP32, STM32 bootloaders, and modern cellular modems.
- **`250000` baud:** DMX512 lighting control networks.

### B. Data Framing & Bit Length (5 to 9 Bits)
- **8 Data Bits (8N1):** Standard for >95% of modern microcontrollers and digital sensors.
- **7 Data Bits (7E1 / 7O1):** Standard for legacy ASCII terminals, financial point-of-sale (POS) equipment, and energy metering protocols (e.g., IEC 62056-21).
- **9 Data Bits (9-bit Mode):** Used in Multi-Processor Communication Protocol (MPCM / MPCB), RS-485 addressing, and custom industrial buses where the 9th bit signifies an address frame versus a data frame.

### C. Parity Checking (None, Even, Odd, Mark, Space)
- **None (`N`):** Default for standard asynchronous communications.
- **Even (`E`) / Odd (`O`):** Essential for industrial Modbus RTU over RS-485, aviation systems, and medical instrumentation requiring single-bit error detection at the hardware layer.
- **Mark (`M`) / Space (`S`):** Fixed parity bit (1 or 0) used in legacy multi-drop serial networks and custom hardware addressing schemes.

### D. Stop Bits (1, 1.5, 2 Stop Bits)
- **1 Stop Bit:** Standard for high-speed digital communications.
- **2 Stop Bits:** Adds frame padding duration, crucial for slow microcontrollers, mechanical teleprinters, legacy RS-232 receivers, and noisy long-distance RS-485 links.

### E. Flow Control (Hardware RTS/CTS & Software XON/XOFF)
- **Hardware Flow Control (RTS/CTS):** Prevents buffer overruns when streaming high-volume data to/from high-speed modules (e.g., Wi-Fi chips, cellular modems, satellite transceivers).
- **Software Flow Control (XON/XOFF):** Uses control characters (`0x11` / `0x13`) for flow pacing on 3-wire interfaces (TX/RX/GND) lacking hardware handshake lines.

### F. Signal Polarity & Logic Inversion
- **Inverted Logic (Active-Low):** Reverses logic levels (idle LOW, start bit HIGH). Required for direct interfacing with inverted signals like Futaba SBUS / S.PORT RC receivers, inverted optical couplers, or raw RS-232 signals prior to level translation.

---

## 4. UART Operational Sub-Modes

MiniPirate specifies four primary operational sub-modes for hardware UART interactions:

```
                      UART Operational Sub-Modes
┌────────────────────────────────────────────────────────────────────────┐
│ 1. Transparent Bridge Mode                                            │
│    Host Terminal <==== USB ====> MiniPirate <==== UART ====> Target    │
├────────────────────────────────────────────────────────────────────────┤
│ 2. Passive Sniffer Mode                                                │
│    Target A TX ----------+                                             │
│                          |---> [MiniPirate RX1/RX2] ---> Host Logs    │
│    Target B TX ----------+                                             │
├────────────────────────────────────────────────────────────────────────┤
│ 3. Loopback & Self-Test Mode                                           │
│    MiniPirate TX ----+                                                 │
│                      |---> [Internal/External Loopback Test]           │
│    MiniPirate RX ----+                                                 │
├────────────────────────────────────────────────────────────────────────┤
│ 4. Break Signal Generation                                             │
│    TX Line held LOW for >1 Frame Duration ===> Target Reset / Sync     │
└────────────────────────────────────────────────────────────────────────┘
```

### 1. Transparent Bridge / Passthrough Mode (`mu` / `m u`)
- **Use Case:** Converts MiniPirate into a direct USB-to-UART adapter. All bytes received from host USB CDC are forwarded to target hardware `TX`, and all bytes from target hardware `RX` are printed directly to host terminal.
- **Application:** Flashing firmware on external target microcontrollers (e.g., ESP8266, ESP32, STM32 via system bootloader) or directly interacting with an AT command modem.

### 2. Passive Sniffer Mode
- **Use Case:** MiniPirate uses two input channels (e.g., hardware `RX1` and secondary pin/`RX2`) to non-intrusively monitor communication between two target ICs without transmitting data.
- **Application:** Debugging existing serial communication between an onboard MCU and a peripheral sensor, displaying interleaved RX/TX packets with timestamps and hex formatting.

### 3. Loopback & Cable Self-Test Mode
- **Use Case:** Connects hardware `TX` directly to `RX` internally or externally.
- **Application:** Verifies baud rate timing accuracy, cable integrity, and hardware UART peripheral functionality.

### 4. Break Signal Generation
- **Use Case:** Pulls the `TX` line LOW for longer than a complete character frame duration (typically 10 to 12 bit periods).
- **Application:** Used to initiate LIN bus synchronization, trigger target MCU bootloader entry modes, or send SysRq break signals to embedded Linux consoles.

---

## 5. Platform-Specific Hardware Considerations

Microcontroller architectures differ in hardware UART capabilities, clock generator resolution, and pin multiplexing options.

```
┌─────────────────┬───────────────────┬────────────────────┬───────────────────────┬────────────────────────┐
│ Target Family   │ Hardware UARTs    │ Native USB CDC     │ Max Tested Baud Rate  │ Key Architectural Notes│
├─────────────────┼───────────────────┼────────────────────┼───────────────────────┼────────────────────────┤
│ AVR (ATmega328P)│ 1 USART           │ No (External Bridge│ 115,200 bps           │ Higher baud rates have │
│                 │                   │ e.g., ATmega16U2)  │ (3.5% clock error)    │ bit timing error @16MHz│
├─────────────────┼───────────────────┼────────────────────┼───────────────────────┼────────────────────────┤
│ RP2040 / RP2350 │ 2 HW UARTs + PIO  │ Yes (Native USB)   │ 921,600+ bps          │ USB speed independent  │
│                 │                   │                    │                       │ of UART baud setting   │
├─────────────────┼───────────────────┼────────────────────┼───────────────────────┼────────────────────────┤
│ STM32 (G431/F446)│ 4-6 USART/UARTs   │ Yes / External USB │ 2,000,000+ bps        │ Fractional baud rate,  │
│                 │                   │                    │                       │ HW RTS/CTS support     │
├─────────────────┼───────────────────┼────────────────────┼───────────────────────┼────────────────────────┤
│ Renesas RA4M1   │ SCI UARTs         │ Yes (Native USB)   │ 921,600 bps           │ Flexible pin selection │
├─────────────────┼───────────────────┼────────────────────┼───────────────────────┼────────────────────────┤
│ ESP32 / ESP32-C6│ 2-3 HW UARTs      │ Yes (USB-JTAG/CDC) │ 3,000,000+ bps        │ GPIO Matrix pin re-map │
└─────────────────┴───────────────────┴────────────────────┴───────────────────────┴────────────────────────┘
```

### A. 8-Bit AVR Architecture (ATmega328P / Arduino Uno)
- **Single Hardware USART:** Shared between USB communications and hardware header pins (D0/RX, D1/TX). Utilizing hardware UART for external devices requires either disconnecting host USB or using `SoftwareSerial` bit-banging on secondary GPIO pins.
- **Baud Rate Clock Error:** At 16 MHz CPU frequency, standard baud rates like `115200` baud exhibit a ~3.5% clock error rate due to integer division in the UBRR register, making high speeds sensitive to temperature and voltage drift.

### B. RP2040 & RP2350 Architecture (Raspberry Pi Pico / Pico 2, XIAO RP2040/RP2350)
- **Native USB CDC:** Host CLI baud rate setting is virtual; USB CDC transfers always execute at full native USB speed (~12 Mbps) regardless of configured baud rate.
- **Dual Hardware UARTs (`UART0`, `UART1`):** Precise integer and fractional baud rate dividers eliminate baud clock error up to multi-megabit speeds.
- **PIO State Machines:** Programmable I/O blocks permit bit-banged UART capability on any arbitrary GPIO pin pair with inverted logic support.

### C. STM32 Architecture (Nucleo G431RB, Nucleo F446RE)
- **Multiple Advanced USARTs:** Include dedicated hardware RTS/CTS flow control, 9-bit mode support, hardware parity calculation, and oversampling options (by 8 or 16).
- **High Baud Rates:** Capable of exceeding 2–5 Mbps without timing error.

### D. Renesas RA4M1 Architecture (Seeed XIAO RA4M1 / Uno R4)
- **Native USB & Serial Communication Interfaces (SCI):** Dedicated hardware channels with configurable noise filters and flexible pin swapping.

### E. ESP32 / ESP32-C6 Architecture (XIAO ESP32-C6)
- **Flexible GPIO Matrix:** Hardware UART peripherals (`UART0`, `UART1`, `UART2`) can be routed to any exposed GPIO pin via internal signal routing.
- **High-Speed Transfers:** Supports non-standard baud rates up to 5 Mbps with hardware RTS/CTS.

---

## 6. Proposed MiniPirate CLI Command Specification (`u` Command Group)

To align with MiniPirate's existing single-letter command convention (`i` for I2C, `p` for Ports, `g` for PWM), UART commands belong to the `u` command group or mode selection `mu`:

```
                                  UART Command Specification
┌──────────────────┬──────────────────────────────────────────┬──────────────────────────────────────────────────┐
│ Command          │ Syntax                                   │ Description / Example                            │
├──────────────────┼──────────────────────────────────────────┼──────────────────────────────────────────────────┤
│ Enter Mode       │ `mu` or `m u`                            │ Selects UART mode context                        │
│ Set Speed        │ `u b [baud]`                             │ Sets UART speed in bps (e.g., `u b 115200`)     │
│ Config Framing   │ `u p [bits][parity][stop]`               │ Configures data format (e.g., `u p 8N1`, `7E1`) │
│ Flow Control     │ `u f [none|hw|sw]`                       │ Configures flow control (e.g., `u f hw`)         │
│ Signal Invert    │ `u i [0|1]`                              │ Enables/disables inverted TX/RX logic            │
│ Read Data        │ `u r [count]`                            │ Reads `count` bytes from hardware UART           │
│ Write Data       │ `u w [bytes...]`                         │ Writes space-separated bytes (Hex/Dec/ASCII)     │
│ Bridge Mode      │ `u m bridge`                             │ Enters transparent USB-to-UART bridge mode       │
│ Sniffer Mode     │ `u m sniff`                              │ Enters passive dual-RX serial sniffer mode       │
│ Break Signal     │ `u k [ms]`                               │ Asserts UART Break condition for `ms` millis     │
│ Show Status      │ `u` or `u ?`                             │ Displays current UART peripheral configuration   │
└──────────────────┴──────────────────────────────────────────┴──────────────────────────────────────────────────┘
```

### Command Example Workflows

#### Example 1: Interfacing with a GPS Module (9600 8N1)
```text
> mu
UART mode selected
> u b 9600
UART speed set to 9600 bps
> u p 8N1
UART format set to 8 Data Bits, No Parity, 1 Stop Bit
> u r 10
Read 10 bytes: $GPGGA,0640
```

#### Example 2: Configuring a MIDI Instrument (31250 8N1)
```text
> mu
UART mode selected
> u b 31250
UART speed set to 31250 bps (MIDI standard)
> u w 0x90 0x3C 0x7F
Wrote 3 bytes: [0x90, 0x3C, 0x7F] (Note On C4)
```

#### Example 3: Industrial Sensor Interfacing (Modbus 9600 8E1)
```text
> mu
UART mode selected
> u b 9600
> u p 8E1
UART format set to 8 Data Bits, Even Parity, 1 Stop Bit
> u w 0x01 0x03 0x00 0x00 0x00 0x02 0xC4 0x0B
Wrote 8 bytes (Modbus Read Holding Registers)
```

---

## 7. Configuration Persistence Integration

In accordance with MiniPirate's existing state storage mechanism (`x` to save to EEPROM / Flash NVS, `y` to restore):

1. **Saved Parameters:** When executing `x`, the system writes the configured hardware UART speed, data bits, parity, stop bits, flow control state, and inversion status to designated EEPROM / Flash NVS offsets.
2. **Boot Restorations:** Executing `y` or booting with auto-restore enabled automatically initializes the hardware UART peripheral to the stored baud rate and framing format.
