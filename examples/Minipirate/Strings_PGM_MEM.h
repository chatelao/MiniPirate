
#ifndef str_pgmmem_h
#define str_pgmmem_h

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef ESP8266
#if defined(__has_include)
#if __has_include(<avr/pgmspace.h>)
#include <avr/pgmspace.h>
#elif __has_include(<pgmspace.h>)
#include <pgmspace.h>
#endif
#else
#include <avr/pgmspace.h>
#endif

void printProgramString (const char * str PROGMEM, Print & target);

#define SERIAL_PRINT_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial);};
#define SERIAL_PRINTLN_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial); Serial.println(); };
#else
#define SERIAL_PRINT_PGM(a) { Serial.print(a); };
#define SERIAL_PRINTLN_PGM(a) { Serial.println(a); };
#endif

#endif

