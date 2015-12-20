
#ifndef str_pgmmem_h
#define str_pgmmem_h

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef ESP8266
#include <avr/pgmspace.h>

void printProgramString (const char * str PROGMEM, Print & target);

#define SERIAL_PRINT_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial);};
#define SERIAL_PRINTLN_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial); Serial.println(); };
#else
#define SERIAL_PRINT_PGM(a) { Serial.print(a); };
#define SERIAL_PRINTLN_PGM(a) { Serial.println(a); };
#endif

#endif

