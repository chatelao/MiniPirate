// --------------------------------------
// MiniPirate
//
//   A human readable serial protocol
//   for basic I/O tasks.
//
// Heavily oriented at "Bus Pirate"
// http://dangerousprototypes.com/docs/Bus_Pirate_menu_options_guide
//
// Pin Layout:
// - A4: SDA
// - A5: SCL
//
// Add command line UI and Port manipulations by O. Chatelain
// Including parts of I2C Scanner adapted by Arduino.cc user Krodal
// Including parts of Bus Pirate library by Ian Lesnet
//
// April 2014
// Using Arduino 1.0.1
//
// April 2015
// Update for Use with IDE-1.6.3
//

#include <ctype.h>
#include <Wire.h>
#include "baseIO.h"
#ifdef __AVR__
#include <EEPROM.h>
#endif
#if defined(ARDUINO_ARCH_STM32)
#include <malloc.h>
#endif
#ifndef ESP8266
#include <Servo.h>
#endif
#include "pins_arduino.h"
#include "modeBase.h"
#include "modeI2C.h"
#include "Strings_PGM_MEM.h"

//-----------------------------------------------------------------------------------------------------------------

#define BAUD_RATE 57600

#define ALLPINS (NUM_ANALOG_INPUTS+A0)

// multipled by 1023 since that's the resolution of the ADC
#if defined(__AVR_ATmega8__)
#define INTERNAL_VOLTAGE_REFERENCE 2618880 //   1125300 = 1.1*1023*1000
// ATMega8 uses 2.56V
#else
#define INTERNAL_VOLTAGE_REFERENCE 1125300 //   1125300 = 1.1*1023*1000
#endif

#if defined(ARDUINO_ARCH_STM32)
uint32_t get_stm32_adc_resolution_macro(int res) {
  switch(res) {
    case 6:  return LL_ADC_RESOLUTION_6B;
    case 8:  return LL_ADC_RESOLUTION_8B;
    case 10: return LL_ADC_RESOLUTION_10B;
    case 12: return LL_ADC_RESOLUTION_12B;
    default: return LL_ADC_RESOLUTION_12B;
  }
}
#endif

int clock_table[ALLPINS];

long readMCU_VCC (long voltage_reference = INTERNAL_VOLTAGE_REFERENCE);
long readMCUInternalTemp();
int freeRam();
bool checkPinIsOutputMode( int pin_nbre );


int adc_resolution = 10;
float VCC;
#ifndef ESP8266
Servo     servo;
#endif
ModeI2C   modeI2C;

enum mpModes { mNONE = 'n', mI2C = 'i', mSPI = 's', mMEMORY = 'm', mEEPROM = 'e', mFLASH = 'f' };
mpModes mpMode;

char const * printMode () {
  switch(mpMode)  {
    case mNONE:   return "";
    case mI2C:    return "I2C";
    case mSPI:    return "SPI";
    case mMEMORY: return "Memory";
    case mEEPROM: return "EEPROM";
    case mFLASH:  return "Flash";
  }
  return "";
}

char getHelpSubcommand() {
  unsigned long start = millis();
  // Wait up to 50ms for any character to be available
  while (!Serial.available() && (millis() - start < 50)) {
    delay(1);
  }
  if (!Serial.available()) {
    return -1;
  }

  // Skip leading spaces, but also handle newline/carriage return
  while (Serial.available()) {
    char p = Serial.peek();
    if (p == ' ') {
      Serial.read(); // consume space
      // wait a bit for the next character if buffer becomes empty
      start = millis();
      while (!Serial.available() && (millis() - start < 50)) {
        delay(1);
      }
    } else if (p == '\r' || p == '\n') {
      return -1;
    } else {
      break;
    }
  }

  if (!Serial.available()) {
    return -1;
  }

  // Now read the subcommand character
  char sub = tolower(Serial.read());
  Serial.print(sub); // echo it back

  // Wait, what if it's a multi-character subcommand like 'a' (could be 'aa' or 'ar') or 'g' (could be 'gg')?
  if (sub == 'a' || sub == 'g') {
    start = millis();
    while (!Serial.available() && (millis() - start < 30)) {
      delay(1);
    }
    if (Serial.available()) {
      char next = tolower(Serial.peek());
      if ((sub == 'a' && (next == 'a' || next == 'r')) || (sub == 'g' && next == 'g')) {
        Serial.read(); // consume it
        Serial.print(next);
        if (sub == 'a' && next == 'a') return '2'; // 'aa'
        if (sub == 'a' && next == 'r') return '3'; // 'ar'
        if (sub == 'g' && next == 'g') return '4'; // 'gg'
      }
    }
  }
  return sub;
}

void mpHelpCommand(char cmd) {
  Serial.println();
  switch (cmd) {
    case 'p':
      SERIAL_PRINTLN_PGM("Help for 'p': Show all port values & directions");
      SERIAL_PRINTLN_PGM("  Prints a detailed status of all digital and analog pins.");
      SERIAL_PRINTLN_PGM("  Includes pin number, direction (INPUT/OUTPUT), state (HIGH/LOW),");
      SERIAL_PRINTLN_PGM("  PWM capability (if applicable), and active interrupt number.");
      SERIAL_PRINTLN_PGM("  For analog pins, displays raw ADC counts and calculated voltage.");
      break;
    case 'q':
      SERIAL_PRINTLN_PGM("Help for 'q': Show port values & directions (quick)");
      SERIAL_PRINTLN_PGM("  Displays all port directions and states in a compact matrix.");
      SERIAL_PRINTLN_PGM("  D0-D7 are grouped, as are other digital and analog pins.");
      SERIAL_PRINTLN_PGM("  Format: '<' for INPUT, '>' for OUTPUT, followed by 1 or 0.");
      break;
    case '<':
      SERIAL_PRINTLN_PGM("Help for '<': Set port as INPUT");
      SERIAL_PRINTLN_PGM("  Syntax: < <pin>");
      SERIAL_PRINTLN_PGM("  Configures the specified pin as an INPUT pin.");
      SERIAL_PRINTLN_PGM("  Example: < 3  (sets digital pin 3 to INPUT mode)");
      break;
    case '>':
      SERIAL_PRINTLN_PGM("Help for '>': Set port as OUTPUT");
      SERIAL_PRINTLN_PGM("  Syntax: > <pin>");
      SERIAL_PRINTLN_PGM("  Configures the specified pin as an OUTPUT pin.");
      SERIAL_PRINTLN_PGM("  Example: > 4  (sets digital pin 4 to OUTPUT mode)");
      break;
    case '/':
      SERIAL_PRINTLN_PGM("Help for '/': Set port to HIGH (clock up)");
      SERIAL_PRINTLN_PGM("  Syntax: / <pin>");
      SERIAL_PRINTLN_PGM("  Sets the specified pin to HIGH.");
      SERIAL_PRINTLN_PGM("  If the pin was not already an OUTPUT, it is changed to OUTPUT.");
      SERIAL_PRINTLN_PGM("  Example: / 5  (sets digital pin 5 to HIGH)");
      break;
    case '\\':
      SERIAL_PRINTLN_PGM("Help for '\\': Set port to LOW (clock down)");
      SERIAL_PRINTLN_PGM("  Syntax: \\ <pin>");
      SERIAL_PRINTLN_PGM("  Sets the specified pin to LOW.");
      SERIAL_PRINTLN_PGM("  If the pin was not already an OUTPUT, it is changed to OUTPUT.");
      SERIAL_PRINTLN_PGM("  Example: \\ 6  (sets digital pin 6 to LOW)");
      break;
    case '^':
      SERIAL_PRINTLN_PGM("Help for '^': Set port LOW-HIGH-LOW (one clock)");
      SERIAL_PRINTLN_PGM("  Syntax: ^ <pin>");
      SERIAL_PRINTLN_PGM("  Generates a single short clock pulse on the specified pin.");
      SERIAL_PRINTLN_PGM("  It sets the pin to LOW, then HIGH, then back to LOW.");
      SERIAL_PRINTLN_PGM("  Example: ^ 7  (pulses digital pin 7)");
      break;
    case '$':
      SERIAL_PRINTLN_PGM("Help for '$': Do a pin sweep");
      SERIAL_PRINTLN_PGM("  Sweeps through all digital pins sequentially.");
      SERIAL_PRINTLN_PGM("  Each pin is temporarily configured as an OUTPUT, its digital state");
      SERIAL_PRINTLN_PGM("  is toggled for 250ms, and then restored to its original state.");
      break;
    case 'c':
      SERIAL_PRINTLN_PGM("Help for 'c': Continuous clock on port");
      SERIAL_PRINTLN_PGM("  Syntax: c <pin> [delay]");
      SERIAL_PRINTLN_PGM("  Toggles the pin between HIGH and LOW at the specified delay (ms).");
      SERIAL_PRINTLN_PGM("  If [delay] is omitted or 0, continuous clocking on <pin> is stopped.");
      SERIAL_PRINTLN_PGM("  Also displays a summary of all pins currently being clocked.");
      SERIAL_PRINTLN_PGM("  Example: c 8 500  (clocks digital pin 8 every 500ms)");
      break;
    case 'a':
      SERIAL_PRINTLN_PGM("Help for 'a': Analog reading");
      SERIAL_PRINTLN_PGM("  Syntax: a <pin>");
      SERIAL_PRINTLN_PGM("  Reads the analog voltage and raw ADC counts on the specified pin.");
      SERIAL_PRINTLN_PGM("  Supports 'a0'-'a5' or simple channel numbers '0'-'5'.");
      SERIAL_PRINTLN_PGM("  Example: a a0  (reads analog input channel 0)");
      break;
    case '2':
      SERIAL_PRINTLN_PGM("Help for 'aa': Continuous analog reading");
      SERIAL_PRINTLN_PGM("  Syntax: aa [interval]");
      SERIAL_PRINTLN_PGM("  Repeatedly reads and displays all analog input pins.");
      SERIAL_PRINTLN_PGM("  An optional interval (in milliseconds) can be specified.");
      SERIAL_PRINTLN_PGM("  Press any key to stop the continuous reading loop.");
      SERIAL_PRINTLN_PGM("  Example: aa 100  (reads all analog channels every 100ms)");
      break;
    case '3':
      SERIAL_PRINTLN_PGM("Help for 'ar': Set ADC resolution");
      SERIAL_PRINTLN_PGM("  Syntax: ar [resolution]");
      SERIAL_PRINTLN_PGM("  Configures the Analog-to-Digital Converter resolution in bits.");
      SERIAL_PRINTLN_PGM("  If [resolution] is omitted, displays the current resolution.");
      SERIAL_PRINTLN_PGM("  Note: Only supported on certain non-AVR platforms (e.g. RP2040, STM32).");
      SERIAL_PRINTLN_PGM("  Example: ar 12  (sets ADC resolution to 12-bit)");
      break;
    case 'g':
      SERIAL_PRINTLN_PGM("Help for 'g': Set analog (PWM) value");
      SERIAL_PRINTLN_PGM("  Syntax: g <pin> <value>");
      SERIAL_PRINTLN_PGM("  Applies a PWM duty cycle value (0 to 255) to the specified pin.");
      SERIAL_PRINTLN_PGM("  Configures the pin to OUTPUT and stops any clock on it.");
      SERIAL_PRINTLN_PGM("  Example: g 9 128  (sets pin 9 to ~50% duty cycle PWM)");
      break;
    case '4':
      SERIAL_PRINTLN_PGM("Help for 'gg': Change analog (PWM) frequency");
      SERIAL_PRINTLN_PGM("  Syntax: gg <frequency>");
      SERIAL_PRINTLN_PGM("  Changes the base PWM frequency in Hz for PWM-enabled pins.");
      SERIAL_PRINTLN_PGM("  Note: Only supported on certain platforms (e.g. ESP8266, RP2040).");
      SERIAL_PRINTLN_PGM("  Example: gg 1000  (sets PWM frequency to 1 kHz)");
      break;
    case 's':
      SERIAL_PRINTLN_PGM("Help for 's': Set servo value");
      SERIAL_PRINTLN_PGM("  Syntax: s <pin> <value>");
      SERIAL_PRINTLN_PGM("  Attaches a servo motor to the specified pin and sets its position/angle");
      SERIAL_PRINTLN_PGM("  (typically 0 to 180 degrees). Cancels any active clock on the pin.");
      SERIAL_PRINTLN_PGM("  Example: s 10 90  (moves servo on pin 10 to 90 degrees)");
      break;
    case 'i':
      SERIAL_PRINTLN_PGM("Help for 'i': Scan I2C device addresses");
      SERIAL_PRINTLN_PGM("  Initializes the I2C interface and scans for connected I2C devices.");
      SERIAL_PRINTLN_PGM("  Lists the hex addresses of all responsive devices found on the bus.");
      break;
    case '#':
      SERIAL_PRINTLN_PGM("Help for '#': Set active I2C device address");
      SERIAL_PRINTLN_PGM("  Syntax: # <address>");
      SERIAL_PRINTLN_PGM("  Sets the specified address as the active target for subsequent I2C operations.");
      SERIAL_PRINTLN_PGM("  Example: # 0x50  (sets active target to I2C address 0x50)");
      break;
    case 'r':
      SERIAL_PRINTLN_PGM("Help for 'r': Read I2C bytes");
      SERIAL_PRINTLN_PGM("  Syntax: r <n>");
      SERIAL_PRINTLN_PGM("  Reads <n> bytes of data from the active I2C device and prints them.");
      SERIAL_PRINTLN_PGM("  Example: r 4  (reads 4 bytes from active I2C device)");
      break;
    case 'w':
      SERIAL_PRINTLN_PGM("Help for 'w': Write I2C bytes");
      SERIAL_PRINTLN_PGM("  Syntax: w <byte1> [byte2] ... [byteN]");
      SERIAL_PRINTLN_PGM("  Writes the specified sequence of space-separated bytes to the active I2C device.");
      SERIAL_PRINTLN_PGM("  Example: w 0x00 0xAB  (writes bytes 0x00 and 0xAB to active I2C device)");
      break;
    case 'x':
      SERIAL_PRINTLN_PGM("Help for 'x': Save configuration to EEPROM");
      SERIAL_PRINTLN_PGM("  Saves the current state of all pin modes, digital values, and active");
      SERIAL_PRINTLN_PGM("  clocks (clock table) to the microcontroller's EEPROM.");
      SERIAL_PRINTLN_PGM("  This configuration will persist across system reboots.");
      break;
    case 'y':
      SERIAL_PRINTLN_PGM("Help for 'y': Load configuration from EEPROM");
      SERIAL_PRINTLN_PGM("  Loads and applies the saved configuration (pin directions, values,");
      SERIAL_PRINTLN_PGM("  and clock frequencies) from EEPROM. Automatically displays the new port states.");
      break;
    case 'z':
      SERIAL_PRINTLN_PGM("Help for 'z': Reset all ports");
      SERIAL_PRINTLN_PGM("  Resets all digital and analog pins to INPUT mode and writes a LOW value");
      SERIAL_PRINTLN_PGM("  to them. Stops all active continuous clocks.");
      break;
    case 'v':
      SERIAL_PRINTLN_PGM("Help for 'v': Show MCU supply voltage (VCC)");
      SERIAL_PRINTLN_PGM("  Measures and displays the current microcontroller supply voltage.");
      SERIAL_PRINTLN_PGM("  Useful for checking power supply stability or battery charge levels.");
      break;
    case 't':
      SERIAL_PRINTLN_PGM("Help for 't': Show MCU internal temperature");
      SERIAL_PRINTLN_PGM("  Reads the chip's internal temperature sensor (if supported by hardware)");
      SERIAL_PRINTLN_PGM("  and displays the result in degrees Celsius.");
      break;
    case 'f':
      SERIAL_PRINTLN_PGM("Help for 'f': Show free memory");
      SERIAL_PRINTLN_PGM("  Measures and displays the amount of free dynamic RAM (SRAM) currently");
      SERIAL_PRINTLN_PGM("  available, along with system information such as flash/EEPROM capacity.");
      break;
    case 'u':
      SERIAL_PRINTLN_PGM("Help for 'u': Show system uptime");
      SERIAL_PRINTLN_PGM("  Displays the total elapsed time in seconds since the microcontroller");
      SERIAL_PRINTLN_PGM("  was last powered on or reset.");
      break;
    case 'e':
      SERIAL_PRINTLN_PGM("Help for 'e': Erase EEPROM");
      SERIAL_PRINTLN_PGM("  Overwrites all cells of the internal EEPROM with 0.");
      SERIAL_PRINTLN_PGM("  Note: This operation can take a few moments depending on EEPROM size.");
      break;
    case '*':
      SERIAL_PRINTLN_PGM("Help for '*': Reboot");
      SERIAL_PRINTLN_PGM("  Performs a software reset of the microcontroller, restarting MiniPirate.");
      break;
    default:
      SERIAL_PRINTLN_PGM("Unknown subcommand. Type 'h' to see the list of supported commands.");
      break;
  }
}

void mpHelp() {

  SERIAL_PRINTLN_PGM("LIST OF SUPPORTED COMMANDS");
  SERIAL_PRINTLN_PGM("==========================");
  SERIAL_PRINTLN_PGM("h/? - Show this help");
  
  //
  // Arduino port manipulations
  //
  SERIAL_PRINTLN_PGM("p - Show all port values & directions");
  SERIAL_PRINTLN_PGM("q - Show all port values & directions (quick)");

//   SERIAL_PRINTLN_PGM(". - Show port value & direction");
  SERIAL_PRINTLN_PGM("< - Set a port as INPUT");
  SERIAL_PRINTLN_PGM("> - Set a port as OUTPUT");

  SERIAL_PRINTLN_PGM("/ - Set a port to HIGH (clock up)");
  SERIAL_PRINTLN_PGM("\\ - Set a port to LOW (clock down)");
  SERIAL_PRINTLN_PGM("^ - Set a port LOW-HIGH-LOW (one clock)");
  SERIAL_PRINTLN_PGM("$ - Do a pin sweep");
  SERIAL_PRINTLN_PGM("c - Set port to clock high and low with given delay");

  // Serial.println("b - Show bar graph of analog input");
  SERIAL_PRINTLN_PGM("a - Analog reading");
  SERIAL_PRINTLN_PGM("aa - Continuous analog reading");
  SERIAL_PRINTLN_PGM("ar - Set ADC resolution (or show if no value)");
  SERIAL_PRINTLN_PGM("g - Set analog (pwm) value");
  SERIAL_PRINTLN_PGM("gg - Change analog (pwm) frequency");

  SERIAL_PRINTLN_PGM("s - Set servo value");

  //
  // I2C communication
  //
  // tbd: Serial.println("mi - Scan i2c device addresses");
  SERIAL_PRINTLN_PGM("i - Scan i2c device addresses");
  SERIAL_PRINTLN_PGM("# - Set i2c device active x ");
  SERIAL_PRINTLN_PGM("r # - Read i2c n bytes from active device");
  SERIAL_PRINTLN_PGM("w # # # - Write i2c bytes to active device");


  //
  // tbd: add SPI communication
  //
  // Serial.println("ms - spi enabled");
  // Serial.println("r # - spi read n bytes from active device");
  // Serial.println("w # # # - spi write bytes to active device");

  //
  // tbd: add LCD communication
  //
  // Serial.println("ml - LCD enabled");
  // Serial.println("r # - LCD read n bytes from active device");
  // Serial.println("w # # # - LCD write bytes to active device");

  //
  // tbd: add Memory access
  //
  // Serial.println("mm - Memory access enabled");
  // Serial.println("# - Set memory position to");
  // Serial.println("r # - Read n bytes from memory");
  // Serial.println("w # # # - Write bytes to memory");
  // Serial.println("| # # # - Or bit mask");
  // Serial.println("& # # # - And bit mask");

  //
  // tbd: add EEPROM access
  //
  // Serial.println("me - EEPROM access enabled");
  // Serial.println("# - Set EEPROM position to");
  // Serial.println("r # - Read n bytes from EEPROM");
  // Serial.println("w # # # - Write bytes to EEPROM");

  //
  // tbd: add FLASH access
  //
  // Serial.println("mf - Flash access enabled");
  // Serial.println("# - Set flash position to");
  // Serial.println("r # - Read n bytes from flash");
  // Serial.println("w # # # - Write bytes to flash");

  //
  // Storing a config to recover after power-up
  //
  SERIAL_PRINTLN_PGM("x - save current config to eeprom");
  SERIAL_PRINTLN_PGM("y - load last config from eeprom");
  SERIAL_PRINTLN_PGM("z - set all ports to input and low");

  SERIAL_PRINTLN_PGM("v - Show MCU VCC reading");
  SERIAL_PRINTLN_PGM("t - Show MCU internal temperature reading");
  SERIAL_PRINTLN_PGM("f - Show free memory");
  SERIAL_PRINTLN_PGM("u - Show system uptime (or clock)");
  SERIAL_PRINTLN_PGM("e - Erase EEPROM");
  SERIAL_PRINTLN_PGM("* - Reboot");

}

void setPin(int pin, int value) {

  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);
  Serial.println();
  SERIAL_PRINT_PGM("New value on pin ");
  printPin(pin);
  if(pin < 10) Serial.print (' ');
  SERIAL_PRINT_PGM(": ");
  printHighLow(value);
  Serial.println();
}
//-----------------------------------------------------------------------------------------------------------------
void clearClockTable();

void setup()
{

//  mode = mpModes('n');
  mpMode = mNONE;

  modeI2C.setup();

  Serial.begin(BAUD_RATE);
  SERIAL_PRINTLN_PGM("MiniPirate: v0.3 ( " __TIMESTAMP__ " ) ");
  SERIAL_PRINT_PGM("Device has ");
  Serial.print (NUM_DIGITAL_PINS - NUM_ANALOG_INPUTS); 
  SERIAL_PRINT_PGM(" digital pins and ");
  Serial.print (NUM_ANALOG_INPUTS); 
  SERIAL_PRINTLN_PGM(" analog pins.");
  SERIAL_PRINT_PGM("CPU is set to ");
  Serial.print ((float) F_CPU / 1023000.0); 
  SERIAL_PRINTLN_PGM("Mhz");

// Run initial scan
  Serial.println();
#if defined(__AVR__) || defined(ARDUINO_ARCH_STM32)
  VCC = readMCU_VCC()/1000.0f;
#else
  VCC = -1.0f;
#endif
  if (VCC < 0.0f) {
#if defined(ARDUINO_ARCH_RP2040)
    VCC = 3.3f;
#else
    VCC = 5.0f;
#endif
  }
  clearClockTable();

  mpHelp();
}
//-----------------------------------------------------------------------------------------------------------------

const int RECORD_SIZE=4;


void loop()
{
  char c;
  Serial.println();
  Serial.print(printMode());
//   Serial.print(mpMode);
/*  
  Serial.print("I2C");
  if(i2c_address_active >= 0) {
    printStrDec("[",   i2c_address_active);
    printStrHex(" - ", getActiveAddress());
    Serial.print("] ");
  }
*/  
  SERIAL_PRINT_PGM("> ");
  Serial.flush();
  c = -1;
  while (!Serial.available()) 
	  {
	  unsigned long now = millis();
	  for (int a=0;a<NUM_DIGITAL_PINS;a++)
		  if (clock_table[a]>0) 
			  digitalWrite(a,(now / clock_table[a]) & 1);
	  }
  c = pollLowSerial();

  switch (c) {
    case '?':
    case 'h':
       {
         char sub = getHelpSubcommand();
         if (sub == -1) {
           Serial.println();
           mpHelp();
         } else {
           mpHelpCommand(sub);
         }
       }
    break;
	case '*':
		{
			Serial.println();
			SERIAL_PRINTLN_PGM("Rebooting...");
			Serial.println();
			delay(1000);
			void(* resetFunc) (void) = 0; //declare reset function @ address 0
			resetFunc();
		}
		break;
	case 'u':
		{
			Serial.println();
			unsigned long now = millis();
			Serial.print (now/1000.0f);
			SERIAL_PRINTLN_PGM(" seconds");
		}
		break;
	case 't':
		{
		Serial.println();
		long t = readMCUInternalTemp();
		if (t == -1000000) 	{
			SERIAL_PRINTLN_PGM("Not supported on this chip");
			}
		else {
			Serial.print (t/1000.0f);
			SERIAL_PRINTLN_PGM("'C");
			}
		}
		break;	
	case 'v':
		{
		Serial.println();
		long v_val = readMCU_VCC();
		if (v_val < 0) 	{
			SERIAL_PRINTLN_PGM("Not supported on this chip");
			}
		else {
			VCC = v_val / 1000.0f;
			Serial.print (VCC);
#if defined(__AVR__)
			SERIAL_PRINT_PGM(" Volts, based on a nominal internal reference of ");
			Serial.print(INTERNAL_VOLTAGE_REFERENCE/1000000.0);
			SERIAL_PRINTLN_PGM(" Volts, +/-10% per chip ");
#elif defined(ARDUINO_ARCH_STM32)
			SERIAL_PRINTLN_PGM(" Volts, based on internal factory-calibrated VREFINT reference ");
#else
			SERIAL_PRINTLN_PGM(" Volts");
#endif
			}
		}
		break;	
	case 'f':
		Serial.println();
		SERIAL_PRINT_PGM("RAM ");
		Serial.print (freeRam());
#if defined(__AVR__)
		SERIAL_PRINT_PGM(" of ");
		Serial.print (RAMEND);
		SERIAL_PRINTLN_PGM(" bytes free");
		
		SERIAL_PRINT_PGM("EEPROM size is ");
		Serial.print (E2END);
		SERIAL_PRINT_PGM(" bytes");
		Serial.println("");

		SERIAL_PRINT_PGM("Flash size is ");
		Serial.print (FLASHEND);
		SERIAL_PRINTLN_PGM(" bytes");
#else
		SERIAL_PRINTLN_PGM(" bytes free");
#endif
		break;    
#if defined(__AVR__)
	case 'e':

		Serial.println();
		SERIAL_PRINT_PGM("Erasing ");
		Serial.print (E2END);
		SERIAL_PRINTLN_PGM(" bytes....this may take a minute...");
		for (int i=0;i<E2END;i++)
			EEPROM.write(i,0);
		SERIAL_PRINTLN_PGM("done");

		break;   
#endif
	case 'm':
     {
      char d = pollLowSerial();
      Serial.println();
      switch (d) {
        
        case 'i':
          // Enable I2C
          mpMode = mI2C;
          modeI2C.init();
        break;

        case 's':
          // Enable SPI
        break;

        case 'l':
          // Enable LCD
        break;
        
        case 'm':
          // Enable Memory
        break;

        case 'e':
          // Enable EEPROM
        break;

        case 'f':
          // Enable FLASH
        break;
      }
     }
     break;
    case 'a':
     {
       if (tolower(pollPeek()) == 'a') {
         pollSerial(); // consume second 'a'
         pollBlanks();
         int interval = 0;
         if (isNumberPeek()) {
           interval = pollInt();
         }
         Serial.println();
         while (!Serial.available()) {
           unsigned long start_ms = millis();
           for (int i = 0; i < NUM_ANALOG_INPUTS; i++) {
             Serial.print(analogRead(i));
             Serial.print(';');
           }
           Serial.println();
           if (interval > 0) {
             while (millis() - start_ms < (unsigned long)interval) {
               if (Serial.available()) {
                 break;
               }
               delay(1);
             }
           }
         }
         if (Serial.available()) {
           Serial.read(); // consume the character that stopped the loop
         }
       } else if (tolower(pollPeek()) == 'r') {
         pollSerial(); // consume second 'r'
         pollBlanks();
         if (isNumberPeek()) {
           int res = pollInt();
#if !defined(__AVR__) && !defined(ESP8266)
           analogReadResolution(res);
           adc_resolution = res;
           Serial.println();
           SERIAL_PRINT_PGM("ADC resolution set to ");
           Serial.print(adc_resolution);
           SERIAL_PRINTLN_PGM(" bits");
#else
           Serial.println();
           SERIAL_PRINTLN_PGM("Not supported on this chip");
#endif
         } else {
           Serial.println();
           SERIAL_PRINT_PGM("ADC resolution: ");
           Serial.print(adc_resolution);
           SERIAL_PRINTLN_PGM(" bits");
         }
       } else {
         Serial.println();
         int pin_nbre = pollPin();
         int analog_channel = -1;
         if (pin_nbre >= A0 && pin_nbre < A0 + NUM_ANALOG_INPUTS) {
           analog_channel = pin_nbre - A0;
         } else if (pin_nbre >= 0 && pin_nbre < NUM_ANALOG_INPUTS) {
           analog_channel = pin_nbre;
           pin_nbre = A0 + pin_nbre;
         }

         if (analog_channel >= 0) {
           int a_value = analogRead(analog_channel);
           SERIAL_PRINT_PGM("Analog value on pin ");
           printPin(pin_nbre);
           printStrDec(": ", a_value);
           SERIAL_PRINT_PGM(" / ");
           Serial.print(a_value / (float)((1 << adc_resolution) - 1) * VCC);
           SERIAL_PRINTLN_PGM("V");
         } else {
           SERIAL_PRINT_PGM("Pin ");
           if (pin_nbre >= 0) {
             printPin(pin_nbre);
           } else {
             SERIAL_PRINT_PGM("invalid");
           }
           SERIAL_PRINTLN_PGM(" does not support analog input");
         }
       }
     }
    break;

    case 'g':
     {
       if (tolower(pollPeek()) == 'g') {
         pollSerial(); // consume second 'g'
         pollBlanks();
         if (isNumberPeek()) {
           int freq = pollInt();
           Serial.println();
#if defined(ESP8266) || defined(ARDUINO_ARCH_RP2040)
           analogWriteFreq(freq);
           SERIAL_PRINT_PGM("New PWM frequency set to ");
           Serial.print(freq);
           SERIAL_PRINTLN_PGM(" Hz");
#else
           SERIAL_PRINTLN_PGM("Changing PWM frequency is not supported on this chip");
#endif
         } else {
           Serial.println();
           SERIAL_PRINTLN_PGM("Invalid frequency value!");
         }
       } else {
         int pin_nbre = pollPin();
         pollBlanks();
         checkPinIsOutputMode(pin_nbre);

         if(pin_nbre >= 0 && isNumberPeek()) {
           clock_table[pin_nbre] = 0;
#ifdef digitalPinHasPWM
           if (digitalPinHasPWM(pin_nbre))  {
             int value = pollInt();
             analogWrite(pin_nbre, value);
             Serial.println();
             SERIAL_PRINT_PGM("New analog value on pin ");
             printPin(pin_nbre);
             printStrDec(": ", value);
             Serial.println();
           } else
#endif
           {
             Serial.println();
             SERIAL_PRINT_PGM("Pin ");
             printPin(pin_nbre);
             SERIAL_PRINT_PGM(" does not support PWM output");
             Serial.println();
           }
         }
       }
     }
    break;

	// sweep through all outputs, pulsing them high briefly (250ms), then back to previous state
	case '$':
// 			for(int i = 0; i < NUM_DIGITAL_PINS+A0; i++) {
// 				digitalWrite(i, LOW);
// 			}
		 Serial.println();
			SERIAL_PRINTLN_PGM("Starting sweep of all pins in sequence:");
			SERIAL_PRINTLN_PGM("Each pin will be briefly set to output, flipped state, and then restored");
			SERIAL_PRINTLN_PGM("Clocks are stopped. ");
			for (int a=0;a<NUM_DIGITAL_PINS;a++)
				{
				int original_pin_mode = getPinMode(a);
				pinMode (a,OUTPUT);
				digitalWrite (a,!digitalRead(a));
				SERIAL_PRINT_PGM(" ");
				printPin(a);	
				
				delay (250);
				digitalWrite (a,!digitalRead(a));
				pinMode (a,original_pin_mode?OUTPUT:INPUT);
				if (a % 8 ==7) Serial.println();
				}
			break;
	case 'c':
		{
		int pin_nbre = pollPin();
		pollBlanks();
		checkPinIsOutputMode(pin_nbre);

		if(pin_nbre >= 0)	{
			if (isNumberPeek()) {
				clock_table[pin_nbre] =pollInt(); 
				Serial.println();
				SERIAL_PRINT_PGM("Clocking pin ");
				printPin(pin_nbre);
				SERIAL_PRINT_PGM(" with delay of ");
				printStrDec("",clock_table[pin_nbre]);
				SERIAL_PRINTLN_PGM("ms");
				}
			else {
				clock_table[pin_nbre] =0; 
				Serial.println();
				SERIAL_PRINT_PGM("Stopping clock on pin ");
				printPin(pin_nbre);
				Serial.println();
				}
			}		
			SERIAL_PRINTLN_PGM("Clocking pins: ");
			for (int a=0;a<NUM_DIGITAL_PINS;a++)
				if (clock_table[a] > 0) 
				{
					printPin(a);
					SERIAL_PRINT_PGM(" delay = ");
					printStrDec("",clock_table[a]);
					SERIAL_PRINTLN_PGM("ms");
				}
		}
		break;

    case 's':
     {
       int pin = pollPin();
       pollBlanks();
       if(pin >= 0 && isNumberPeek()) {
           int value = pollInt();
           checkPinIsOutputMode(pin);		
#ifndef ESP8266
           servo.attach(pin);
           servo.write(value);
#endif           
           Serial.println();
           SERIAL_PRINT_PGM("New servo value on pin ");
           printPin(pin);
           printStrDec(": ", value);
           Serial.println();
           
           // Keep the position until next input
           pollPeek();
#ifndef ESP8266
           servo.attach(pin);
#endif
		   clock_table[pin] = 0;
       }
     }
    break;

    case '/':
     {
      int pin = pollPin();
      if(pin >= 0) {
        setPin(pin,1);
		// this is not an error, as you can set an input pin (to enable / disable pull up resistors, for example) 
		// but let's warn the user, just in case
		checkPinIsOutputMode(pin);	
		clock_table[pin] = 0;
      }
     }
    break;

    case '\\':
     {
      int pin = pollPin();
      if(pin >= 0) {
   	    checkPinIsOutputMode(pin);
        setPin(pin, 0);
		clock_table[pin] = 0;
      }
     }
    break;

    case '^':
     {
      int pin = pollPin();
      if(pin >= 0) {
		checkPinIsOutputMode(pin);
        setPin(pin, 0);
        setPin(pin, 1);
        setPin(pin, 0);
		clock_table[pin] = 0;
      }
     }
    break;

/* tbd
    case '.':
     {
      int pin = pollPin();
      if(pin >= 0) {
        printPin(pin);
      }
     }
    break;
*/

    case '<':
     {
      Serial.println();
      int pin = pollPin();
      if(pin >= 0) {
        pinMode(pin, INPUT);
        Serial.println();
        SERIAL_PRINT_PGM("Pin ");
        printPin(pin);
        SERIAL_PRINTLN_PGM(" is now INPUT");
       }
      }
    break;

    case '>':
     {
      Serial.println();
      int pin = pollPin();
      if(pin >= 0) {
        pinMode(pin, OUTPUT);
        Serial.println();
        SERIAL_PRINT_PGM("Pin ");
        printPin(pin);
        SERIAL_PRINTLN_PGM(" is now OUTPUT");
       }    
      }
     break;
    
    case 'p':
       Serial.println();
       printPorts();
    break;
	case 'q':
		Serial.println();
		printPortsQuick();
		break;

    case 'i':
       Serial.println();
       mpMode = mI2C;
       modeI2C.init();
    break;

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '0':
      modeI2C.select(c);
    break;

    case 'r':
       pollBlanks();
       if(isNumberPeek()) {
         int read_nbre = pollInt();       
         modeI2C.read(read_nbre);
       }
    break;

    case 'w':
      modeI2C.write(); 
    break;
    
#ifdef __AVR__
    case 'x':
     {
       // Write all directions to EEPROM
       // Write all digital values to EEPROM
       // Write all pwm values to EEPROM
       for(int i = 0; i < ALLPINS; i++) {
		   int pin_mode  = getPinMode(i);
		   // != 0;
           int pin_value = digitalRead(i);
		   
		   int offset = E2END-1-RECORD_SIZE*i;
		   if (offset >=3) { 
				EEPROM.write(offset,   pin_mode);
				EEPROM.write(offset-1, pin_value);
				EEPROM.write(offset-2, clock_table[i]>>8);
				EEPROM.write(offset-3, clock_table[i]& 0xff);
			 }
       }
     }
     Serial.println();
     SERIAL_PRINT_PGM("Saved state to EEPROM");
    break;
    
    case 'y':
     {
       // Read all directions to EEPROM				
       // Read all digital values to EEPROM
       // Read all pwm values to EEPROM

	 // read / write the EEPROM from the back end, so that we might co-exist with other sketches on the device which 
	 // would normally save from the start of eeprom.
	   clearClockTable();
       for(int i = 0; i < ALLPINS; i++) {
		   int offset = E2END-1-RECORD_SIZE*i;
		   if (offset > 3) { 
			   int pin_mode  = EEPROM.read(offset);
			   int pin_value = EEPROM.read(offset - 1);
			   byte cv1 = EEPROM.read(offset- 2);
			   byte cv2 = EEPROM.read(offset - 3);
			   clock_table[i] = cv1<<8 | cv2;
			   pinMode(i, pin_mode);
			   //if(pin_mode != 0) {
			   digitalWrite(i, pin_value);
			   //}
			   }
       }
     }
     Serial.println();
     SERIAL_PRINTLN_PGM("Loaded state from EEPROM");
     printPorts();
     break;
#endif
     
   case 'z':
     Serial.println();
     SERIAL_PRINT_PGM("Reset ...");
	 clearClockTable();
     for(int i = 0; i < ALLPINS; i++) {
       pinMode(i, INPUT);
       digitalWrite(i, LOW);
     }
     break;
  }
}

long readMCU_VCC(long voltage_reference)
	{
#if defined(__AVR__)
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	ADMUX = _BV(MUX3) | _BV(MUX2);
#else
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring

	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
	uint8_t high = ADCH; // unlocks both

	long result = (high<<8) | low;

	result = voltage_reference / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return result; // Vcc in millivolts
#elif defined(ARDUINO_ARCH_STM32) && defined(AVREF)
	int raw_vref = analogRead(AVREF);
	return __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw_vref, get_stm32_adc_resolution_macro(adc_resolution));
#else
	return -1;
#endif
	}


long readMCUInternalTemp()
	{
#if defined(__AVR__)
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
	return -1000000;
#endif
	long result; // Read temperature sensor against 1.1V reference
	ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
	delay(20); // Wait for Vref to settle - 2 was inadequate
	ADCSRA |= _BV(ADSC); // Convert
	while (bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |= ADCH<<8;
	result = (result - 125) * 1075;
	return result;
#elif defined(ARDUINO_ARCH_RP2040)
	float t = analogReadTemp();
	return (long)(t * 1000.0f);
#elif defined(ARDUINO_ARCH_STM32) && defined(ATEMP) && defined(AVREF)
	int raw_vref = analogRead(AVREF);
	int raw_temp = analogRead(ATEMP);
	uint32_t vref_mv = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw_vref, get_stm32_adc_resolution_macro(adc_resolution));
	int32_t temp_c = __LL_ADC_CALC_TEMPERATURE(vref_mv, raw_temp, get_stm32_adc_resolution_macro(adc_resolution));
	return (long)(temp_c * 1000);
#else
	return -1000000;
#endif
	}

int freeRam()
	{
#if defined(__AVR__)
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#elif defined(ARDUINO_ARCH_RP2040)
	return rp2040.getFreeHeap();
#elif defined(ARDUINO_ARCH_STM32)
	struct mallinfo mi = mallinfo();
	return mi.fordblks;
#else
	return -1;
#endif
	}

//-----------------------------------------------------------------------------------------------------------------
bool checkPinIsOutputMode( int pin_nbre )
	{
	if (pin_nbre<0) return false;
	if ( getPinMode(pin_nbre)==0){ 
		SERIAL_PRINTLN_PGM("Warning: pin is not set to input");
		return false;
		}
	return true;
	}
//-----------------------------------------------------------------------------------------------------------------
void clearClockTable()
	{
	for (int a=0;a< ALLPINS;a++)
		clock_table[a] = a*0;

	}
//-----------------------------------------------------------------------------------------------------------------
