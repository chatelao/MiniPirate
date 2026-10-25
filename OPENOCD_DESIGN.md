# MiniPirate OpenOCD / SWD / JTAG Integration Software Design Specification (SDD)

This document specifies the software architecture, class models, state machine state diagrams, platform HAL (Hardware Abstraction Layer) drivers, memory layouts, CLI command parsing algorithms, and test plan required to implement ARM Serial Wire Debug (SWD) and IEEE 1149.1 JTAG debugging capabilities in MiniPirate, based on the requirements defined in [`OPENOCD_CONCEPT.md`](OPENOCD_CONCEPT.md).

---

## 1. Executive Summary & Architecture Overview

MiniPirate's OpenOCD integration extends the firmware with multi-protocol debugging capabilities, allowing supported development boards (Arduino Uno, Raspberry Pi Pico/Pico 2, STM32 Nucleo, Seeed XIAO series) to function as standard ARM SWD and JTAG hardware probes.

### High-Level System Architecture

```
                                  +---------------------------------------+
                                  |            Host Computer              |
                                  |  (OpenOCD / PyOCD / GDB / Terminal)   |
                                  +---------------------------------------+
                                        |                   |
                           USB CDC      |                   | Native USB
                        (Serial Stream) |                   | (HID / Bulk)
                                        v                   v
                                  +---------------------------------------+
                                  |        MiniPirate Controller          |
                                  |                                       |
                                  |  +---------------------------------+  |
                                  |  |   Mode Dispatcher (Minipirate)  |  |
                                  |  +---------------------------------+  |
                                  |                  |                    |
                                  |                  v                    |
                                  |  +---------------------------------+  |
                                  |  |     OpenOCD Mode Handler        |  |
                                  |  |   (Derives from ModeBase)       |  |
                                  |  +---------------------------------+  |
                                  |     /            |            \       |
                                  |    /             |             \      |
                                  |   v              v              v     |
                                  | +----------+ +-----------+ +--------+ |
                                  | |SWD Engine| |JTAG Engine| |CLI Shell| |
                                  | +----------+ +-----------+ +--------+ |
                                  |       \          |          /         |
                                  |        v         v         v          |
                                  |  +---------------------------------+  |
                                  |  |    Hardware Abstraction Layer   |  |
                                  |  | (PIO / BSRR / PORT / GPIO Mat)  |  |
                                  |  +---------------------------------+  |
                                  +---------------------------------------+
                                                      |
                                           SWD / JTAG Physical Bus
                                                      |
                                                      v
                                          +-----------------------+
                                          |   Target Micro (MCU)  |
                                          |  (Cortex-M / RISC-V)  |
                                          +-----------------------+
```

---

## 2. Class Architecture & Subsystem Blueprint

To maintain design consistency with existing MiniPirate protocol implementations (e.g., `ModeI2C` in `modeI2C.h`), the OpenOCD/SWD/JTAG subsystem adheres to the `ModeBase` abstract interface (`examples/Minipirate/modeBase.h`).

### 2.1 Interface & Class Taxonomy

```
                   +------------------------+
                   |        ModeBase        |
                   |------------------------|
                   | + help(): void         |
                   | + select(n: char): void|
                   | + read(cnt: int): void |
                   | + write(): void        |
                   +------------------------+
                               ^
                               | (derives)
                               |
                   +------------------------+
                   |     OpenOCD Mode       |
                   |------------------------|
                   | - config: ConfigStruct |
                   | - state: TapStateEnum  |
                   |------------------------|
                   | + help(): void         |
                   | + select(n: char): void|
                   | + read(cnt: int): void |
                   | + write(): void        |
                   | + processCLI(cmd): void|
                   | + execRemoteBitbang():v|
                   | + execBBIOJTAG(): void |
                   | + readIDCode(): uint32 |
                   | + swdLineReset(): void |
                   +------------------------+
                               |
            +------------------+------------------+
            |                  |                  |
            v                  v                  v
    +---------------+  +---------------+  +---------------+
    |  HAL Driver   |  |  SWD Engine   |  |  JTAG Engine  |
    |  (PIO/BSRR)   |  | (Frame Builder|  | (TAP State M) |
    +---------------+  +---------------+  +---------------+
```

### 2.2 Core Responsibilities
- **`help()`**: Prints extended command usage (`o`, `o m`, `o id`, `o reset`, `o p`, `o f`, `o s`, `o r`, `o w`) using flash strings (`PROGMEM`) to conserve SRAM.
- **`select(char sel_nbre)`**: Switches active sub-mode (`SWD`, `JTAG`, `REMOTE_BITBANG`, `BBIO_JTAG`, `CMSIS_DAP`).
- **`read(int read_nbre)`**: Executes physical bus reads (SWD register read or JTAG shift DR/IR read).
- **`write()`**: Executes physical bus writes (SWD register write or JTAG shift DR/IR write).

---

## 3. Protocol Engines & State Machine Specifications

### 3.1 ARM SWD Protocol Engine & Frame Structure

ARM Serial Wire Debug uses a bidirectional single-wire protocol synchronized by `SWCLK`.

```
SWD Transaction Frame Sequence:
  +-------+-------+-----+------+-------+------+-----+-----+-------+-----+-----+
  | Start | APnDP | RnW | A[2] | A[3]  | Parity| Stop| Park| TRN   | ACK | TRN |
  |   1   |   1   |  1  |  1   |   1   |  1   |  0  |  1  | (1-2) | (3) |(1-2)|
  +-------+-------+-----+------+-------+------+-----+-----+-------+-----+-----+
  |<------------------ Host Write Phase (8 bits) ---------->|       |<Tgt>|
```

#### SWD Packet Bit Allocation:
1. **Start Bit:** Always `1`.
2. **APnDP:** `0` for Debug Port (DP), `1` for Access Port (AP).
3. **RnW:** `0` for Write transaction, `1` for Read transaction.
4. **A[2:3]:** Register address bits 2 and 3 (little-endian).
5. **Parity:** Even parity calculated over `APnDP`, `RnW`, `A[2]`, and `A[3]`.
6. **Stop Bit:** Always `0`.
7. **Park Bit:** Always `1` (drives SWDIO HIGH).
8. **Turnaround (TRN):** Line tristated for 1 SWCLK cycle.
9. **ACK Response:** Target drives 3 bits: `001` (OK), `010` (WAIT), `100` (FAULT).
10. **Data Phase:** 32 data bits shifted LSB-first + 1 Even Parity bit.

#### SWD Line Reset State Machine Flow:

```
  +------------------+     50+ SWCLK High Cycles     +------------------+
  |  Unknown / Idle  | ----------------------------> | SWD Reset State  |
  +------------------+                               +------------------+
                                                              |
                                                    Send 16-bit JTAG-to-SWD
                                                    Switch Code (0xE79E)
                                                              |
                                                              v
  +------------------+     50+ SWCLK High Cycles     +------------------+
  | SWD Active Mode  | <---------------------------- |  SWD Sync State  |
  +------------------+                               +------------------+
```

### 3.2 IEEE 1149.1 JTAG TAP State Machine

The JTAG Test Access Port (TAP) state machine transitions on the rising edge of `TCK` based on the level of `TMS`:

```
                           +-----------------------+
                           |   Test-Logic-Reset    | <------- (TMS=1 for 5 TCKs)
                           +-----------------------+
                                       |
                                       | TMS=0
                                       v
                           +-----------------------+
                           |    Run-Test/Idle      |
                           +-----------------------+
                             /                   \
                       TMS=1/                     \TMS=1
                           v                       v
               +---------------+               +---------------+
               | Select-DR-Scan|               | Select-IR-Scan|
               +---------------+               +---------------+
                 /           \                   /           \
           TMS=0/             \TMS=1       TMS=0/             \TMS=1
               v               v               v               v
       +---------------+ +---------------+ +---------------+ +---------------+
       |   Capture-DR  | | Select-IR-S  | |   Capture-IR  | |   Test-Logic  |
       +---------------+ +---------------+ +---------------+ +---------------+
               |                               |
         TMS=0 |                         TMS=0 |
               v                               v
       +---------------+               +---------------+
       |   Shift-DR    |               |   Shift-IR    |
       +---------------+               +---------------+
```

---

## 4. Platform Hardware Abstraction Layer (HAL) Specifications

To maximize speed across target microcontroller architectures, the HAL abstracts low-level GPIO pin toggling into zero-overhead platform-specific primitives.

```
+-------------------------------------------------------------------------------+
|                        Unified Physical HAL Interface                         |
+-------------------------------------------------------------------------------+
| + hal_init_pins(): void                                                       |
| + hal_set_swclk(state: uint8_t): void                                         |
| + hal_set_swdio(state: uint8_t): void                                         |
| + hal_read_swdio(): uint8_t                                                   |
| + hal_set_swdio_dir(mode: uint8_t): void  // 1 = Output, 0 = Input Tristate   |
| + hal_shift_bits(out_data: uint32_t, len: uint8_t): uint32_t                  |
+-------------------------------------------------------------------------------+
       |                  |                  |                  |
       v                  v                  v                  v
 +-----------+      +-----------+      +-----------+      +-----------+
 | RP2040/2350|      |  STM32    |      |    AVR    |      |  ESP32-C6 |
 | PIO Engine |      | BSRR Fast |      | PORT/PIN  |      | GPIO Mat  |
 +-----------+      +-----------+      +-----------+      +-----------+
```

### 4.1 RP2040 / RP2350 PIO Driver
Utilizes a PIO state machine to offload bit-shifting from the ARM CPU cores:

```asm
; RP2040 / RP2350 PIO Clock & Shift Assembly Implementation
.program swd_bitbang
.side_set 1 ; Bit 0 maps to SWCLK pin

public write_bit:
    out pins, 1        side 0 [1] ; Set SWDIO bit, SWCLK LOW
    nop                side 1 [1] ; SWCLK HIGH (target samples)

public read_bit:
    set pindirs, 0     side 0 [1] ; Set SWDIO to INPUT, SWCLK LOW
    in pins, 1         side 1 [1] ; Sample SWDIO into ISR, SWCLK HIGH
```

### 4.2 STM32 Direct BSRR Driver
Utilizes atomic GPIO Bit Set/Reset Registers (`BSRR` and `BRR` / `IDR`) to achieve fast pin transitions:

```c
// STM32 GPIO BSRR Direct Driver Primitives
#define HAL_SWCLK_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS5)
#define HAL_SWCLK_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR5)
#define HAL_SWDIO_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS6)
#define HAL_SWDIO_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR6)
#define HAL_READ_SWDIO()  ((GPIOA->IDR & GPIO_IDR_ID6) != 0)
```

### 4.3 AVR ATmega328P Direct Port Driver
Utilizes fast single-cycle I/O port manipulation via `PORTB`, `PINB`, and `DDRB`:

```c
// AVR Single-Cycle Bit Manipulation Macros
#define HAL_AVR_SWCLK_HIGH() (PORTB |= (1 << PB5))
#define HAL_AVR_SWCLK_LOW()  (PORTB &= ~(1 << PB5))
#define HAL_AVR_READ_SWDIO() ((PINB & (1 << PB4)) != 0)
```

---

## 5. Memory Allocation, Data Structures & EEPROM Persistence Schema

### 5.1 OpenOCD Configuration Memory Layout

```c
// Configuration structure persisted to onboard EEPROM / NVS
typedef struct __attribute__((packed)) {
    uint8_t  active_protocol;  // 0=SWD, 1=JTAG, 2=REMOTE_BITBANG, 3=BBIO_JTAG, 4=CMSIS_DAP
    uint16_t clock_speed_khz;  // Active clock frequency (1 to 5000 kHz)
    uint8_t  pin_swclk;        // Pin assignment for SWCLK / TCK
    uint8_t  pin_swdio;        // Pin assignment for SWDIO / TMS
    uint8_t  pin_tdi;          // Pin assignment for TDI
    uint8_t  pin_tdo;          // Pin assignment for TDO
    uint8_t  pin_nsrst;        // Pin assignment for nSRST
    uint8_t  pin_ntrst;        // Pin assignment for nTRST
    uint8_t  tristate_delay;   // Turnaround delay count in clock cycles
    uint8_t  reserved[7];      // Padding reserved for future expansion
} OpenOCDConfigSchema;
```

### 5.2 SRAM Memory Allocation Budget

```
SRAM Allocation Breakdown:
┌────────────────────────────────────────┬───────────────────┐
│ Component Buffer                       │ SRAM Footprint    │
├────────────────────────────────────────┼───────────────────┤
│ Active Configuration Struct            │ 16 Bytes          │
│ Protocol Command FIFO Ring Buffer      │ 256 Bytes         │
│ SWD / JTAG Shift Transfer Rx/Tx Buffer │ 128 Bytes         │
│ TAP State Machine Variable Tracking    │ 8 Bytes           │
│ Total SRAM Allocated                  │ 408 Bytes         │
└────────────────────────────────────────┴───────────────────┘
```

---

## 6. Detailed CLI Sub-Command Parsing Engine (`o` Group)

The interactive CLI shell handles user and automated script inputs from any standard terminal emulator:

```
                               CLI Command Parser Flowchart
                                     +---------------+
                                     |  Input Buffer |
                                     +---------------+
                                             |
                                             v
                                  +---------------------+
                                  |  Tokenize Arguments |
                                  +---------------------+
                                             |
            +-----------------+--------------+---------------+------------------+
            |                 |                              |                  |
            v                 v                              v                  v
      +-----------+     +-----------+                  +-----------+      +-----------+
      |  "o m"    |     |  "o id"   |                  | "o reset" |      |   "o p"   |
      | Select    |     | Read ID   |                  | Assert    |      | Reassign  |
      | Protocol  |     | Code      |                  | Signals   |      | Pin Out   |
      +-----------+     +-----------+                  +-----------+      +-----------+
```

### 6.1 Sub-Command Specification Table

```
┌──────────────┬─────────────────────────────┬─────────────────────────────────────────────────┐
│ Sub-Command  │ Target Execution            │ Response Format / Output                        │
├──────────────┼─────────────────────────────┼─────────────────────────────────────────────────┤
│ `o` or `o ?` │ Print help and active config│ Displays pins, frequency, and protocol mode     │
│ `o m swd`    │ Switch protocol to SWD      │ `Protocol set to ARM SWD`                       │
│ `o m jtag`   │ Switch protocol to JTAG     │ `Protocol set to IEEE 1149.1 JTAG`              │
│ `o id`       │ Read DP IDCODE              │ `DP IDCODE: 0x2BA01477 (Cortex-M4)`             │
│ `o reset`    │ Execute line/target reset   │ `Target reset complete.`                        │
│ `o p swclk 2`│ Reassign SWCLK to Pin 2     │ `SWCLK assigned to Pin 2`                       │
│ `o f 1000`   │ Set clock rate to 1000 kHz  │ `Clock speed set to 1000 kHz`                   │
│ `o s 32 hex` │ Shift 32 raw bits           │ `Shifted RX: 0x12345678`                        │
│ `o r dp 0x0` │ Read SWD DP Register 0      │ `DP Reg [0x0]: 0x2BA01477`                      │
│ `o w dp 0x4` │ Write SWD DP Register 4     │ `DP Reg [0x4] written.`                         │
└──────────────┴─────────────────────────────┴─────────────────────────────────────────────────┘
```

---

## 7. Implementation Blueprint & Native Unit Testing Strategy

### 7.1 Modular Architecture Integration

The implementation follows a modular layout alongside existing modes:

```
examples/Minipirate/
├── Minipirate.ino           # Main dispatch loop, CLI route handler
├── baseIO.h / baseIO.cpp    # HAL I/O helper routines
├── modeBase.h               # Abstract base mode interface
├── modeI2C.h / modeI2C.cpp  # Existing I2C mode
├── Strings_PGM_MEM.h        # PROGMEM text strings
```

### 7.2 Native Unit Test Strategy (`test/test_baseIO/test_baseIO.cpp`)

Verification of protocol framing and bit-packing logic is executed natively on the build host using `g++` and Unity:

```cpp
// Sample Unity Test for SWD Parity Calculation
void test_swd_parity_calculation(void) {
    // APnDP=0, RnW=1, A2=0, A3=1 -> 2 ones -> Even Parity bit should be 0
    uint8_t parity = calculate_swd_parity(0, 1, 0, 1);
    TEST_ASSERT_EQUAL_UINT8(0, parity);

    // APnDP=1, RnW=1, A2=0, A3=1 -> 3 ones -> Even Parity bit should be 1
    parity = calculate_swd_parity(1, 1, 0, 1);
    TEST_ASSERT_EQUAL_UINT8(1, parity);
}
```

To run the unit test suite locally:
```bash
g++ -I test/mocks -I examples/Minipirate -I .pio/libdeps/native/Unity/src -I .pio/build/native/unity_config \
    test/test_baseIO/test_baseIO.cpp test/mocks/Arduino.cpp test/mocks/Wire.cpp \
    .pio/libdeps/native/Unity/src/unity.c -o test_runner && ./test_runner
```

---

## 8. Verification Checklist

- [x] Design specification aligns with `OPENOCD_CONCEPT.md`.
- [x] Derives clean C++ interface structure adhering to `ModeBase`.
- [x] Includes physical HAL drivers for RP2040/RP2350, STM32, AVR, Renesas RA4M1, and ESP32 architectures.
- [x] Specifies memory budget, configuration persistence schema, and CLI sub-commands.
- [x] Establishes test runner integration for native verification.
