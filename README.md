MiniPirate
==========

A simple commandline to scan I2C, read/write GPIO, read/write EEPROM and read CPU informations from any Serial terminal. Ideal for exploring new devices without any code writing.

List of supported commands:
---------------------------
+ h/? - Show this help
+ p - Show all port values & directions
+ q - Show all port values & directions (quick)
+ < - Set a port as INPUT
+ > - Set a port as OUTPUT
+ / - Set a port to HIGH (clock up)
+ \ - Set a port to LOW (clock down)
+ ^ - Set a port LOW-HIGH-LOW (one clock)
+ $ - Do a pin sweep
+ c - Set port to clock high and low with given delay
+ g - Set analog (pwm) value
+ s - Set servo value
+ \A2/A3 - Set Pin A2 to low, Pin A3 to high (and both to output)
+ s 5 90 - Set Servo on Pin D5 to 90°
+ i 2 r 10 - Switch to I2C device 2 and read 10 bytes
+ i - Scan i2c device addresses
+ # - Set i2c device active x 
+ r # - Read i2c n bytes from active device
+ w # # # - Write i2c bytes to active device
+ x - save current config to eeprom
+ y - load last config from eeprom
+ z - set all ports to input and low
+ v - Show AVR VCC reading
+ t - Show AVR internal temperature reading
+ f - Show free memory
+ u - Show system uptime (or clock)
+ e - Erase EEPROM
+ * - Reboot
