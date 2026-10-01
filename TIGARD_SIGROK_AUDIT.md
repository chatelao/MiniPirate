# Tigard & Sigrok Integration Audit: Logic Analyzer Integration for MiniPirate

## 1. Executive Summary & Scope

This document provides a comprehensive technical audit and architectural blueprint for integrating **Tigard** (FTDI FT2232H-based USB interface) and **Sigrok / PulseView** (the open-source signal analysis suite) with **MiniPirate** (the lightweight multi-platform micro-controller hardware CLI).

### Core Objectives
1. **Analyze Tigard's Hardware & Driver Model:** Evaluate Tigard's FT2232H MPSSE state machine and `libsigrok` driver framework (`ft2232bd` / `ftdi-mpsse`) for high-speed logic capture.
2. **Evaluate Protocol Interfaces:** Compare FTDI MPSSE host-driven sampling with open, microcontroller-native logic analyzer protocols—specifically the **SUMP / OLS (Open Bench Logic Sniffer)** protocol over USB CDC ACM.
3. **Architect MiniPirate Logic Capture:** Define hardware acceleration strategies (RP2040/RP2350 PIO + DMA, STM32 Timer-triggered DMA, AVR/Renesas/ESP32 timer buffers) for native signal capture within MiniPirate.
4. **Establish Bandwidth & Feature Benchmarks:** Compare sampling rates, buffer depths, target voltage flexibility, continuous streaming vs. snapshot capture, and host CPU overhead across Tigard, Bus Pirate v5, and MiniPirate.
5. **Formulate CLI & Workflow Specifications:** Provide concrete command syntax for MiniPirate logic analyzer mode (`l` / `ml`) and step-by-step PulseView / `sigrok-cli` connection workflows.
6. **Chart Strategic Convergence Roadmap:** Define a multi-phase implementation plan bringing full PulseView/Sigrok compatibility to MiniPirate development boards.

---

## 2. Tigard Hardware Architecture & Sigrok Driver Model

### 2.1 Hardware Architecture Overview
Tigard (designed by SecuringHardware) is an open-source USB bridge board constructed around the FTDI FT2232H dual-channel USB 2.0 High-Speed transceiver.

```
                                  Tigard (FT2232H) Architecture
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                         │
│   ┌──────────────┐     USB 2.0 HS      ┌──────────────────┐    MPSSE Engine A           │
│   │   Host PC    │ <=================> │  FT2232H Channel A│ ------------------------┐  │
│   │ (PulseView / │     (480 Mbps)      │  (JTAG/SPI/I2C)  │                         │  │
│   │  sigrok-cli) │                     └──────────────────┘                         │  │
│   └──────────────┘                      ┌──────────────────┐    Async Serial B      │  │
│                                         │  FT2232H Channel B│ --------------------┐ │  │
│                                         │  (UART / GPIO)   │                     │ │  │
│                                         └──────────────────┘                     │ │  │
│                                                                                  │ │  │
│   ┌────────────────────────────────────────────────────────────────────────────┐ │ │  │
│   │ Bidirectional Level Shifters (1.2V - 5.5V Target VREF)                     │ │ │  │
│   └────────────────────────────────────────────────────────────────────────────┘ │ │  │
│                                 │                        │                       │ │  │
│                                 ▼                        ▼                       ▼ ▼  │
│                         ┌──────────────┐         ┌──────────────┐         ┌──────────┐│
│                         │ Target Pin   │         │ Target Pin   │         │ Tigard   ││
│                         │ Header (A)   │         │ Header (B)   │         │ Headers  ││
│                         └──────────────┘         └──────────────┘         └──────────┘│
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

- **USB Interface:** USB 2.0 High-Speed (480 Mbps PHY).
- **Channels:**
  - **Channel A:** Connected to MPSSE (Multi-Protocol Synchronous Serial Engine) for high-speed hardware SPI, I2C, JTAG, SWD, and logic sampling.
  - **Channel B:** Dedicated UART / Async Serial and general-purpose I/O (GPIO).
- **Target Voltage Range:** On-board bidirectional level shifters support target logic levels from **1.2V to 5.5V** with active target VREF tracking.

### 2.2 Sigrok Driver Model (`ft2232bd` / `ftdi-mpsse`)
`libsigrok` communicates with Tigard using the FT2232 driver module layered over `libftdi1` / `libusb-1.0`.

#### Driver Execution Lifecycle
1. **Device Discovery & Scan (`dev_scan`):** Searches USB bus for VID `0x0403` (FTDI) and PID `0x6010` (FT2232H), matching product string "Tigard" or custom USB descriptors.
2. **Context Initialization (`dev_open`):** Opens USB interface 0 (Channel A), resets the MPSSE engine, and flushes host USB buffers.
3. **Configuration Phase (`config_set`):** Sets sample rate (up to 30 MSa/s), channel selection (up to 8 digital channels), and trigger conditions (rising/falling edge or pattern match).
4. **Acquisition Loop (`acquisition_start`):**
   - Configures MPSSE mode `0x8A` (Disable divide-by-5 clock) or `0x8B` (Enable divide-by-5 clock).
   - Sends MPSSE command `0x81` (Read GPIO low byte / DBUS pins) continuously or at scheduled timer intervals.
   - Streams raw sample bytes via USB bulk transfers to host memory buffers in real-time.
5. **Teardown (`acquisition_stop`):** Stops MPSSE transfer loop, clears USB endpoints, and closes USB context.

---

## 3. Sigrok / PulseView Protocol Driver Analysis

To integrate MiniPirate into Sigrok and PulseView, we evaluate two primary driver paradigms available in `libsigrok`:

```
                    libsigrok Hardware Integration Models
┌─────────────────────────────────────────────────────────────────────────────┐
│ 1. FTDI MPSSE Driver (Tigard Model)                                         │
│    Host-driven MPSSE commands ===> FT2232H Chip ===> Continuous USB Stream  │
├─────────────────────────────────────────────────────────────────────────────┤
│ 2. SUMP / OLS Logic Protocol Driver (MiniPirate Model)                      │
│    Binary Commands over USB CDC ===> MCU Buffer Capture ===> Block Transfer │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3.1 FTDI MPSSE Driver vs. SUMP Protocol Driver Comparison

| Dimension | FT2232H MPSSE Driver (`ft2232bd`) | SUMP / OLS Protocol Driver (`ols`) |
|---|---|---|
| **Host Driver Class** | Native `libsigrok` C driver for FTDI chips | Generic `ols` driver built into `libsigrok` |
| **Transport Layer** | Low-level USB Bulk Endpoint (`libusb-1.0`) | Standard USB CDC ACM virtual COM port |
| **Hardware Hardware Requirement** | FTDI FT2232H / FT232H dedicated silicon | Any MCU with USB CDC (RP2040, STM32, AVR, ESP32) |
| **Sampling Paradigm** | Continuous Streaming or Hardware Buffer | On-chip SRAM Buffer Snapshot & Stream |
| **Max Continuous Sample Rate**| Up to 30 MSa/s (USB 2.0 High-Speed limit) | Limited by CDC ACM throughput (~1-2 MSa/s continuous, 100+ MSa/s burst snapshot) |
| **Configuration Commands** | Hardware register bitmask opcodes | 4-byte binary command frames (`0x00`-`0xC0`) |
| **PulseView Compatibility** | Natively supported via FTDI device scan | Natively supported via "Open Bench Logic Sniffer" driver |

---

## 4. MiniPirate SUMP Logic Analyzer Integration Architecture

### 4.1 SUMP Protocol Specification
The Open Bench Logic Sniffer (SUMP) binary protocol uses concise 1-byte or 5-byte command frames sent over serial/CDC ACM to configure logic acquisition:

```
                            SUMP Binary Frame Layout
┌───────────────────────────┬─────────────────────────────────────────────────┐
│ Command Type              │ Frame Format                                    │
├───────────────────────────┼─────────────────────────────────────────────────┤
│ Short Command (1 byte)    │ [ OPCODE (1 byte) ]                             │
│ Extended Command (5 bytes)│ [ OPCODE (1 byte) | PARAM_B3 | PARAM_B2 | B1 | B0 ]│
└───────────────────────────┴─────────────────────────────────────────────────┘
```

#### Key SUMP Opcodes

| Opcode (Hex) | Name | Type | Description / Action |
|---|---|---|---|
| `0x00` | RESET | Short | Resets internal capture state machine and clears buffers. |
| `0x01` | RUN | Short | Starts logic acquisition according to configured sample rate and triggers. |
| `0x02` | ID | Short | Returns 4-byte identification string (`1SLO` / `1SLA`). |
| `0x04` | METADATA | Short | Returns device capability keys (channels, sample rate, SRAM depth). |
| `0x80` | SET_CLOCK | Extended | Sets sample clock divider (`Divider = (SystemClock / TargetRate) - 1`). |
| `0x81` | SET_COUNTS | Extended | Sets pre-trigger and post-trigger sample counts. |
| `0x82` | SET_FLAGS | Extended | Sets sampling flags (Inverted inputs, Demux, External clock, Noise filter). |
| `0xC0`-`0xC3`| SET_TRIGGER_MASK | Extended | Sets 32-bit pin trigger mask for stages 0–3. |
| `0xC4`-`0xC7`| SET_TRIGGER_VAL | Extended | Sets 32-bit expected pin trigger value for stages 0–3. |

### 4.2 Platform Hardware Acceleration Strategies

```
                       MiniPirate Multi-Architecture Capture
┌─────────────────────────────────────────────────────────────────────────────┐
│ RP2040 / RP2350 (PIO + DMA Ring Buffer)                                     │
│ GPIO Pins ===> PIO SM (100 MSa/s) ===> DMA Channel ===> SRAM Capture Buffer │
├─────────────────────────────────────────────────────────────────────────────┤
│ STM32 G431 / F446 (Timer-Triggered GPIO DMA)                                │
│ GPIO Pins ===> TIM2 Trigger ===> DMA2 Channel ===> SRAM Capture Buffer      │
├─────────────────────────────────────────────────────────────────────────────┤
│ AVR ATmega328P / RA4M1 / ESP32-C6 (Timer ISR / Poll Capture)                │
│ GPIO Pins ===> Timer ISR / Assembly Loop ===> SRAM Circular Buffer          │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### A. RP2040 / RP2350 Architecture (Pico, Pico 2, XIAO RP2040/RP2350)
- **Engine:** Dedicated Programmable I/O (PIO) state machine coupled with a high-speed Direct Memory Access (DMA) channel.
- **Mechanism:** PIO SM executes a 1-instruction loop sampling `in pins, 8` or `in pins, 16` directly into the RX FIFO. The DMA channel transfers FIFO words to an SRAM ring buffer without CPU intervention.
- **Performance:** Up to **100+ MSa/s** snapshot capture across 8 or 16 GPIO pins; buffer depth up to 128–192 KB.

#### B. STM32 Architecture (Nucleo G431RB, Nucleo F446RE)
- **Engine:** Hardware Timer (TIM2/TIM3) triggering DMA memory transfers from `GPIOx->IDR` (Input Data Register).
- **Mechanism:** Timer update event triggers DMA transfer directly from Port Input Data Register to SRAM array.
- **Performance:** Up to **20–50 MSa/s** snapshot capture across 8 or 16 pins on a single GPIO port; buffer depth up to 32–64 KB.

#### C. 8-Bit AVR / Renesas RA4M1 / ESP32-C6
- **Engine:** Timer-driven Interrupt Service Routine (ISR) or dedicated fast polling assembly loop.
- **Mechanism:** Timer interrupt or blocked assembly loop reads digital port register (e.g., `PINB` / `PIND`) into internal SRAM array.
- **Performance:** 1–5 MSa/s on AVR (1–2 KB buffer); 10–20 MSa/s on RA4M1/ESP32-C6 (16–32 KB buffer).

---

## 5. Performance, Bandwidth & Feature Trade-Off Matrix

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
│                             Performance & Architecture Comparison                                │
├───────────────────────────┬─────────────────────────┬───────────────────────┬────────────────────┤
│ Feature / Metric          │ Tigard (FT2232H)        │ MiniPirate (RP2040)   │ Bus Pirate (v5)    │
├───────────────────────────┼─────────────────────────┼───────────────────────┼────────────────────┤
│ Max Snapshot Sample Rate  │ 30 MSa/s                │ 100 MSa/s             │ 10-20 MSa/s        │
│ Max Continuous Rate       │ 30 MSa/s (USB HS)       │ ~1-2 MSa/s (USB FS)   │ ~1 MSa/s (USB FS)  │
│ Max Digital Channels      │ 8 channels (DBUS)       │ 8-16 channels         │ 8 channels         │
│ Hardware Buffer Depth     │ Host RAM (Streamed)     │ 128 KB SRAM (~128kSa) │ 128 KB SRAM        │
│ Target Voltage Flexibility│ 1.2V - 5.5V (Adjustable)│ Native MCU (3.3V/5V)  │ 1.2V - 5.0V        │
│ Host CPU Overhead         │ Low (Hardware MPSSE)    │ Zero during capture   │ Zero during capture│
│ Cost                      │ ~$25 - $35              │ $2 - $5               │ ~$35 - $40         │
└───────────────────────────┴─────────────────────────┴───────────────────────┴────────────────────┘
```

---

## 6. CLI Commands & Software Workflow Specifications

### 6.1 MiniPirate CLI Command Set (`l` Command Group)

To integrate logic analyzer functionality seamlessly into MiniPirate's single-letter CLI hierarchy:

```
                               MiniPirate CLI Logic Commands
┌──────────────────┬──────────────────────────────────────────┬──────────────────────────────────────────────────┐
│ Command          │ Syntax                                   │ Description / Example                            │
├──────────────────┼──────────────────────────────────────────┼──────────────────────────────────────────────────┤
│ Enter SUMP Mode  │ `ml` or `m l`                            │ Switches MiniPirate to binary SUMP protocol mode │
│ Status / Info    │ `l` or `l ?`                             │ Displays logic analyzer configuration & status   │
│ Set Sample Rate  │ `l s [freq]`                             │ Sets manual capture sampling rate in Hz          │
│ Set Sample Count │ `l c [samples]`                          │ Sets snapshot sample count (e.g., `l c 10000`)   │
│ Set Trigger Pin  │ `l t [pin] [R|F|H|L]`                    │ Sets pin edge or level trigger condition         │
│ Terminal Capture │ `l r`                                    │ Runs snapshot and prints ASCII logic trace       │
└──────────────────┴──────────────────────────────────────────┴──────────────────────────────────────────────────┘
```

#### Terminal Logic Trace Output Example (`l r`)
```text
> l s 1000000
Sample rate set to 1.00 MHz
> l c 16
Sample count set to 16
> l r
Capturing 16 samples @ 1.00 MHz...
Sample | D0 | D1 | D2 | D3 | State
----------------------------------
     0 |  1 |  0 |  1 |  0 | 0x0A
     1 |  1 |  0 |  1 |  0 | 0x0A
     2 |  0 |  0 |  1 |  0 | 0x02
     3 |  0 |  1 |  1 |  0 | 0x06
     4 |  0 |  1 |  1 |  0 | 0x06
     5 |  1 |  1 |  1 |  0 | 0x0E
```

---

### 6.2 Host GUI Workflow: PulseView / Sigrok-CLI Setup

#### Connecting Tigard to PulseView
1. Connect Tigard to host via USB.
2. Launch PulseView.
3. Click **Connect to Device** -> Select **FT2232H / FTDI MPSSE** driver.
4. Select target sample rate (e.g., 10 MHz or 30 MHz) and click **Run**.

#### Connecting MiniPirate to PulseView (via SUMP Driver)
1. Flash MiniPirate with SUMP mode enabled.
2. Launch PulseView.
3. Click **Connect to Device**:
   - Choose Driver: **Open Bench Logic Sniffer (ols)**.
   - Choose Interface: **Serial Port**.
   - Select Serial Device: `/dev/ttyACM0` (Linux) or `COMx` (Windows).
   - Set Serial Speed: `115200` baud.
4. Click **Scan for Devices** -> Select **Open Bench Logic Sniffer / MiniPirate**.
5. Configure channel count and sample depth in PulseView and click **Run**.

```bash
# Example CLI Acquisition using sigrok-cli with MiniPirate
sigrok-cli -d ols:conn=/dev/ttyACM0:serialcomm=115200/8n1 \
           --config samplerate=1M --samples 10000 \
           --channels 0=SDA,1=SCL -O ascii
```

---

## 7. Strategic Integration Roadmap

To deliver full Sigrok and PulseView integration in MiniPirate, the following phased roadmap is defined:

```
                         Strategic Implementation Roadmap
┌─────────────────────────────────────────────────────────────────────────────┐
│ Phase 1: Core SUMP Protocol & USB CDC Transport                             │
│   - Implement SUMP binary opcode parser (`0x00`-`0x82`).                    │
│   - Implement basic SRAM sample buffer and ASCII CLI renderer (`l r`).      │
├─────────────────────────────────────────────────────────────────────────────┤
│ Phase 2: Hardware Acceleration Engines                                      │
│   - Implement RP2040 / RP2350 PIO + DMA logic capture engine.               │
│   - Implement STM32 Timer-driven GPIO DMA capture.                          │
├─────────────────────────────────────────────────────────────────────────────┤
│ Phase 3: Advanced Triggers & Sigrok Certification                           │
│   - Implement multi-stage edge and pattern matching triggers in SUMP parser.│
│   - Validate compatibility across PulseView, `sigrok-cli`, and PulseView Web.│
└─────────────────────────────────────────────────────────────────────────────┘
```
