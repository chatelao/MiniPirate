# MiniPirate

A simple commandline to scan I2C, read/write GPIO, read/write EEPROM and read CPU informations from any Serial terminal. Ideal for exploring new devices without any code writing.

## List of supported commands:

### General Commands

| Command | Description | Example |
|---|---|---|
| `h` / `?` | Show this help | `h` |
| `v` | Show AVR VCC reading | `v` |
| `t` | Show AVR internal temperature reading | `t` |
| `f` | Show free memory | `f` |
| `u` | Show system uptime (or clock) | `u` |
| `e` | Erase EEPROM | `e` |
| `*` | Reboot | `*` |
| `x` | save current config to eeprom | `x` |
| `y` | load last config from eeprom | `y` |

### GPIO Commands

| Command | Description | Example |
|---|---|---|
| `p` | Show all port values & directions | `p` |
| `q` | Show all port values & directions (quick) | `q` |
| `<` | Set a port as INPUT | `< 3` (sets pin 3 to INPUT)|
| `>` | Set a port as OUTPUT | `> 4` (sets pin 4 to OUTPUT) |
| `/` | Set a port to HIGH (clock up) | `/ 5` (sets pin 5 to HIGH) |
| `\` | Set a port to LOW (clock down) | `\ 6` (sets pin 6 to LOW) |
| `^` | Set a port LOW-HIGH-LOW (one clock) | `^ 7` (sends a clock pulse to pin 7) |
| `$` | Do a pin sweep | `$` |
| `c` | Set port to clock high and low with given duration | `c 8 100` (clocks pin 8 with 100ms duration)|
| `g` | Set analog (pwm) value | `g 9 128` (sets PWM on pin 9 to 128)|
| `s` | Set servo value | `s 10 90` (sets servo on pin 10 to 90 degrees)|
| `\A2/A3`| Set Pin A2 to low, Pin A3 to high (and both to output) | `\A2/A3` |
| `z` | set all ports to input and low | `z` |


### I2C Commands

| Command | Description | Example |
|---|---|---|
| `i` | Scan i2c device addresses | `i` |
| `#` | Set i2c device active | `# 42` (sets device with address 42 as active) |
| `r #` | Read n bytes from active device | `r 10` (reads 10 bytes from the active I2C device) |
| `w # # #` | Write bytes to active device | `w 1 2 3` (writes bytes 1, 2, and 3 to the active I2C device) |
| `i 2 r 10`| Switch to I2C device 2 and read 10 bytes | `i 2 r 10` |
