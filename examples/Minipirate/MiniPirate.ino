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
#include <EEPROM.h>
#ifndef ESP8266
#include <Servo.h>
#endif
#include "pins_arduino.h"
#include "baseIO.h"
#include "modeBase.h"
#include "modeI2C.h"
#include "Strings_PGM_MEM.h"

//-----------------------------------------------------------------------------------------------------------------

#define BAUD_RATE 9600 // Standard baud rate of an out of the box installation

#define ALLPINS (NUM_ANALOG_INPUTS+A0)

// multipled by 1023 since that's the resolution of the ADC
#if defined(__AVR_ATmega8__)
#define INTERNAL_VOLTAGE_REFERENCE 2618880 //   1125300 = 1.1*1023*1000
// ATMega8 uses 2.56V
#else
#define INTERNAL_VOLTAGE_REFERENCE 1125300 //   1125300 = 1.1*1023*1000
#endif

int clock_table[ALLPINS];

long readAVR_VCC (long voltage_reference = INTERNAL_VOLTAGE_REFERENCE);
long readAVRInternalTemp();
int freeRam();
bool checkPinIsOutputMode( int pin_nbre );


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
  SERIAL_PRINTLN_PGM("g - Set analog (pwm) value");

  SERIAL_PRINTLN_PGM("s - Set servo value");

  SERIAL_PRINTLN_PGM("\\A2/A3 - Set Pin A2 to low, Pin A3 to high (and both to output)");
  SERIAL_PRINTLN_PGM("s 5 90 - Set Servo on Pin D5 to 90°");
  SERIAL_PRINTLN_PGM("i 2 r 10 - Switch to I2C device 2 and read 10 bytes");

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

  SERIAL_PRINTLN_PGM("v - Show AVR VCC reading");
  SERIAL_PRINTLN_PGM("t - Show AVR internal temperature reading");
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
#ifndef ESP8266
  VCC = readAVR_VCC()/1000.0f;
  if (VCC < 0.0f) VCC = 5.0f;
#endif
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
       Serial.println();
       mpHelp();
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
#ifndef ESP8266
		int t = readAVRInternalTemp();
#else
		int t = -1;
#endif
		if (t < 0) 	{
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
#ifndef ESP8266
		VCC = readAVR_VCC()/1000.0;
#endif
		if (VCC < 0.0f) 	{
			SERIAL_PRINTLN_PGM("Not supported on this chip");
			VCC=5.0f;
			}
		else {
			Serial.print (VCC);
			SERIAL_PRINT_PGM(" Volts, based on a nominal internal reference of ");
			Serial.print(INTERNAL_VOLTAGE_REFERENCE/1000000.0);
			SERIAL_PRINTLN_PGM(" Volts, +/-10% per chip ");
			}
		}
		break;	
#ifndef ESP8266
	case 'f':
		Serial.println();
		SERIAL_PRINT_PGM("RAM ");
		Serial.print (freeRam());
		SERIAL_PRINT_PGM(" of ");
		Serial.print (RAMEND);
		SERIAL_PRINTLN_PGM(" bytes free");
		
		SERIAL_PRINT_PGM("EEPROM size is ");
		Serial.print (E2END);
		SERIAL_PRINTLN_PGM(" bytes");

		SERIAL_PRINT_PGM("Flash size is ");
		Serial.print (FLASHEND);
		SERIAL_PRINTLN_PGM(" bytes");
 
		break;    
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
    case 'g':
     {
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
    
#ifndef ESP8266    
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

#ifndef ESP8266
long readAVR_VCC(long voltage_reference)
	{
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
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
	}


long readAVRInternalTemp()
	{
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
	return -1;
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
	}

int freeRam()
	{
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	}
#endif

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
