# MiniPirate ROADMAP

This document outlines the planned features and improvements for MiniPirate. The goal is to make it a more powerful and versatile tool for electronics enthusiasts and professionals.

## Core & Usability

- [ ] Command history and recall
- [ ] Tab completion for commands and arguments
- [ ] User-defined aliases for common command sequences
- [ ] Verbose mode for detailed command execution feedback
- [ ] Quiet mode for script-friendly output
- [ ] Support for different output formats (e.g., JSON, CSV)
- [ ] Built-in calculator for hex/dec/bin conversions
- [ ] Persistent configuration across sessions
- [ ] Multi-language support
- [ ] Theming and color customization

## GPIO

- [ ] Read all GPIO states at once
- [ ] Write to multiple GPIO pins simultaneously
- [ ] Configure pull-up/pull-down resistors
- [ ] Interrupt handling and reporting
- [ ] Pulse width modulation (PWM) output on all PWM-capable pins
- [ ] Frequency generation on a pin
- [ ] Pulse counting on a pin
- [ ] Support for debouncing inputs
- [ ] Port expander support (e.g., MCP23017)
- [ ] Shift register support (e.g., 74HC595)

## I2C

- [ ] High-speed I2C (400kHz and 1MHz)
- [ ] I2C sniffing and logging
- [ ] I2C EEPROM read/write commands
- [ ] I2C device discovery with name resolution
- [ ] Support for 10-bit I2C addresses
- [ ] I2C clock stretching
- [ ] I2C bus recovery
- [ ] Smart I2C address scanning (skip known non-responsive addresses)
- [ ] I2C register map decoding
- [ ] Support for SMBus protocols

## SPI

- [ ] Full SPI master support
- [ ] Full SPI slave support
- [ ] Configurable SPI mode (CPOL/CPHA)
- [ ] Configurable SPI clock speed
- [ ] SPI sniffing and logging
- [ ] Chip select (CS) management
- [ ] Support for multi-byte SPI transfers
- [ ] SPI EEPROM read/write commands
- [ ] SPI flash memory interaction
- [ ] Bit-banged SPI for any GPIO pins

## UART

- [ ] UART sniffing and logging
- [ ] UART bridge mode
- [ ] Configurable baud rate, parity, and stop bits
- [ ] Send and receive files over UART
- [ ] Support for binary data
- [ ] Break signal generation
- [ ] LIN bus support
- [ ] DMX512 support
- [ ] MIDI support
- [ ] RS-485 support

## 1-Wire

- [ ] 1-Wire device discovery
- [ ] Read temperature from DS18B20 sensors
- [ ] Read/write to 1-Wire EEPROMs
- [ ] Parasitic power mode support
- [ ] ROM search and matching
- [ ] Overdrive speed support
- [ ] CRC checking
- [ ] Generic 1-Wire read/write commands
- [ ] Support for multiple 1-Wire busses
- [ ] 1-Wire bus sniffing

## ADC & Analog

- [ ] Continuous ADC sampling
- [ ] ADC data logging
- [ ] Configurable ADC resolution
- [ ] Configurable ADC reference voltage
- [ ] Differential ADC readings
- [ ] Internal temperature sensor reading
- [ ] Internal voltage reference reading
- [ ] Simple oscilloscope mode
- [ ] Analog comparator support
- [ ] Triggered ADC sampling

## DAC

- [ ] Digital-to-Analog Converter (DAC) output
- [ ] Waveform generation (sine, square, triangle)
- [ ] Arbitrary waveform generation
- [ ] Voltage reference output
- [ ] Support for external DAC chips
- [ ] Audio output capabilities
- [ ] Function generator mode
- [ ] Frequency sweep
- [ ] Amplitude control
- [ ] Offset control

## Power & Voltage

- [ ] Programmable voltage output
- [ ] Current measurement
- [ ] Power consumption measurement
- [ ] Over-current protection
- [ ] Battery charging and monitoring
- [ ] Power supply sequencing
- [ ] Voltage ramp generation
- [ ] Logic level shifting
- [ ] Short circuit detection
- [ ] Brown-out detection

## Display & Feedback

- [ ] Support for LCD and OLED displays
- [ ] On-board LED for status indication
- [ ] Buzzer for audible feedback
- [ ] RGB LED for color-coded status
- [ ] Graphical user interface (GUI) on a connected computer
- [ ] Web interface for remote control
- [ ] Data plotting and visualization
- [ ] Real-time clock (RTC) integration
- [ ] Temperature and humidity sensor integration
- [ ] Barometric pressure sensor integration
