# MiniPirate OpenOCD / SWD / JTAG Integration Roadmap

This document outlines the phased roadmap, hardware integration tasks, protocol implementation milestones, toolchain configurations, CI workflows, and release guidelines for incorporating **ARM Serial Wire Debug (SWD)** and **IEEE 1149.1 JTAG** debugging capabilities into MiniPirate, as specified in [`OPENOCD_CONCEPT.md`](OPENOCD_CONCEPT.md) and [`OPENOCD_DESIGN.md`](OPENOCD_DESIGN.md).

---

## 1. Executive Summary & Vision

By equipping MiniPirate with SWD and JTAG probe capabilities, standard development boards (RP2040/RP2350, STM32, Seeed XIAO, AVR) can double as open-source hardware debuggers without requiring dedicated external debug probes (e.g., SEGGER J-Link, ST-LINK).

### Objectives:
- **Dual Protocol Support:** Physical layer drivers for 2-wire ARM SWD and 4/5-wire JTAG.
- **OpenOCD Integration Modes:** Support for `remote_bitbang`, `BBIO_JTAG` (Bus Pirate binary mode), and USB `CMSIS-DAP` (v1/v2).
- **Interactive Terminal Debugging:** Expose an `o` / `mo` command group for manual TAP navigation, SWD line reset sequences, and register inspection straight from a serial monitor.
- **Hardware Acceleration:** Leverage RP2040/RP2350 PIO state machines and STM32 BSRR registers for high clock speeds.

---

## 2. Implementation Phasing & Milestones

```
┌───────────────────────────────────────────────────────────────────────────────────┐
│ Phase 1: Architecture, Physical HAL Primitives & Unit Mock Tests                   │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 2: SWD & JTAG Protocol State Machine Engines                                 │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 3: OpenOCD Translation Interfaces (remote_bitbang & BBIO_JTAG)               │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 4: Standalone Interactive CLI Shell ('o' Command Group)                     │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 5: Hardware Acceleration (RP2040 PIO & STM32 Direct BSRR)                   │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 6: CMSIS-DAP v1/v2 USB Endpoint Drivers                                      │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 7: EEPROM NVS Persistence & Restore Integration                              │
├───────────────────────────────────────────────────────────────────────────────────┤
│ Phase 8: CI Build Matrix, Automated Hardware Verification & Flashing Guides      │
└───────────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Detailed Milestone Checklist

### Phase 1: Hardware Abstraction Layer (HAL) & Core Primitives
- [x] Create `OPENOCD_CONCEPT.md` high-level concept specification.
- [x] Create `OPENOCD_DESIGN.md` software design specification.
- [ ] Define abstract physical HAL macros and primitives (`hal_set_swclk`, `hal_set_swdio`, `hal_read_swdio`, `hal_set_swdio_dir`).
- [ ] Implement software bit-bang HAL drivers for generic Arduino / AVR boards.
- [ ] Implement fast GPIO drivers for STM32 using direct `BSRR`/`IDR` registers.
- [ ] Add unit tests in `test/test_baseIO/test_baseIO.cpp` to verify pin direction switching and timing logic.

### Phase 2: Protocol Engines (SWD & JTAG)
- [ ] Implement SWD frame builder (Start bit, APnDP, RnW, Address, Parity, ACK parsing, Data phase).
- [ ] Implement SWD line reset sequence (50+ SWCLK HIGH cycles + 0xE79E switching code).
- [ ] Implement 16-state IEEE 1149.1 JTAG TAP state machine navigator.
- [ ] Add parity calculation and CRC routines for SWD packet validation.
- [ ] Add host-side mock unit tests for DP/AP register read/write frame generation.

### Phase 3: OpenOCD Host Translation Modes
- [ ] **Mode 1: `remote_bitbang`** - Parse ASCII character stream commands (`'0'..'7'`, `'B'`, `'b'`, `'R'`, `'Q'`) over TCP/Serial.
- [ ] **Mode 2: `BBIO_JTAG`** - Implement Bus Pirate binary mode emulator (20x `0x00` entry, `0x06` mode switch, byte-level bit shifts).
- [ ] Validate OpenOCD host tool connection against `remote_bitbang` interface.

### Phase 4: Interactive CLI Shell (`o` / `mo` Command Group)
- [ ] Derive `ModeOpenOCD` from `ModeBase` (`examples/Minipirate/modeBase.h`).
- [ ] Register `mo` command in `Minipirate.ino` main dispatch handler.
- [ ] Implement CLI sub-commands:
  - `o`: Display active debug status, protocol, and pin mapping.
  - `o m [swd|jtag|remote|bbio|dap]`: Sub-mode selection.
  - `o id`: Scan target and output DP IDCODE or JTAG TAP chain.
  - `o reset [srst|trst|sys]`: Target hardware/software reset.
  - `o p [signal] [pin]`: Dynamic signal pin assignment.
  - `o f [speed_khz]`: Set target clock frequency.
  - `o s [nbits] [hex]`: Raw bit shifting.
  - `o r [dp|ap] [addr]`: Read ARM SWD DP/AP register.
  - `o w [dp|ap] [addr] [val]`: Write ARM SWD DP/AP register.
- [ ] Store CLI help strings in PROGMEM (`SERIAL_PRINTLN_PGM`) in `Strings_PGM_MEM.h`.

### Phase 5: Hardware Acceleration (PIO & BSRR)
- [ ] Write RP2040/RP2350 PIO state machine code (`swd_bitbang.pio`) for hardware SWCLK/SWDIO clocking.
- [ ] Integrate PIO FIFO DMA transfers for high-speed OpenOCD buffer flushing.
- [ ] Optimize STM32 HAL drivers using register pointer caching for 20ns clock edges.

### Phase 6: CMSIS-DAP Native USB Integration
- [ ] Add USB HID / Bulk endpoint definitions for RP2040, STM32, and ESP32-C6 targets.
- [ ] Implement ARM CMSIS-DAP v1 (HID) and v2 (WinUSB Bulk) packet handlers (`DAP_ExecuteCommand`).
- [ ] Verify native PyOCD and Keil MDK connection over CMSIS-DAP endpoint.

### Phase 7: Non-Volatile Memory Persistence
- [ ] Define `OpenOCDConfigSchema` (16 bytes packed) for persistent EEPROM / NVS storage.
- [ ] Integrate active SWD/JTAG pin assignments and clock speeds with `x` (Save) and `y` (Restore) commands.

### Phase 8: CI/CD Workflow, Flashing Guides & Release Assets
- [ ] Update `.github/workflows/build.yml` to compile OpenOCD module across all target platforms.
- [ ] Update `.github/workflows/release.yml` to include pre-compiled binary release assets with OpenOCD feature enabled.
- [ ] Create dedicated flashing/debugging guides for targets (e.g. `HOWTO_OPENOCD_SWD.md`).
- [ ] Update main `README.md` and `ROADMAP.md` feature checklists.

---

## 4. Hardware Pin Alignment Reference

```
┌─────────────────┬─────────────┬─────────────┬─────────────┬─────────────┬─────────────┬─────────────┐
│ Architecture    │ SWCLK / TCK │ SWDIO / TMS │ TDI         │ TDO         │ nSRST       │ nTRST       │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ RP2040 / RP2350 │ GPIO 2      │ GPIO 3      │ GPIO 4      │ GPIO 5      │ GPIO 6      │ GPIO 7      │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ Seeed XIAO      │ D2 (GPIO2)  │ D3 (GPIO3)  │ D4 (GPIO4)  │ D5 (GPIO5)  │ D1 (GPIO1)  │ D0 (GPIO0)  │
│ RP2040 / RP2350 │             │             │             │             │             │             │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ STM32 Nucleo    │ D13 (PA5)   │ D12 (PA6)   │ D11 (PA7)   │ D10 (PB6)   │ D9 (PC7)    │ D8 (PA9)    │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ Seeed XIAO      │ D2 (P104)   │ D3 (P103)   │ D4 (P102)   │ D5 (P101)   │ D1 (P105)   │ D0 (P106)   │
│ RA4M1           │             │             │             │             │             │             │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ Seeed XIAO      │ D2 (GPIO2)  │ D3 (GPIO3)  │ D4 (GPIO4)  │ D5 (GPIO5)  │ D1 (GPIO1)  │ D0 (GPIO0)  │
│ ESP32-C6        │             │             │             │             │             │             │
├─────────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┼─────────────┤
│ AVR ATmega328P  │ Pin 13 (SCK)│ Pin 12      │ Pin 11      │ Pin 10      │ Pin 9       │ Pin 8       │
└─────────────────┴─────────────┴─────────────┴─────────────┴─────────────┴─────────────┴─────────────┘
```

---

## 5. Testing & Validation Matrix

```
┌───────────────────────────┬──────────────────────────────────┬────────────────────────────────────────┐
│ Test Scenario             │ Tool / Setup                     │ Expected Success Criteria              │
├───────────────────────────┼──────────────────────────────────┼────────────────────────────────────────┤
│ Unit Bit Parity Test      │ Unity / `g++` local runner       │ 100% pass on SWD parity calculations   │
│ IDCODE Probe (SWD)        │ PuTTY / `o id` command           │ Returns target Cortex-M DP IDCODE      │
│ TAP Chain Scan (JTAG)     │ PuTTY / `o id` command           │ Detects target JTAG devices & IR len   │
│ OpenOCD remote_bitbang    │ `openocd -f minipirate_remote.cfg`| GDB attaches to localhost:3333         │
│ OpenOCD BBIO_JTAG         │ `openocd -f minipirate_bbio.cfg` | Flash memory dump succeeds via telnet  │
│ PyOCD CMSIS-DAP           │ `pyocd list`                     │ MiniPirate probe recognized over USB   │
│ EEPROM Save / Restore     │ CLI `x` / `y`                    │ Restores pin maps & clock speed        │
└───────────────────────────┴──────────────────────────────────┴────────────────────────────────────────┘
```
