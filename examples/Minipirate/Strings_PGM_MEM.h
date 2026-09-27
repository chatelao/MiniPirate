#ifndef str_pgmmem_h
#define str_pgmmem_h

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(__has_include)
  #if __has_include(<avr/pgmspace.h>)
    #include <avr/pgmspace.h>
    #define HAS_PGMSPACE 1
  #elif __has_include(<pgmspace.h>)
    #include <pgmspace.h>
    #define HAS_PGMSPACE 1
  #endif
#elif defined(__AVR__) || defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
  #define HAS_PGMSPACE 1
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#if defined(HAS_PGMSPACE) && !defined(ESP8266) && !defined(ESP32)
void printProgramString (const char * str PROGMEM, Print & target);

#define SERIAL_PRINT_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial);};
#define SERIAL_PRINTLN_PGM(a) { static const char str[] PROGMEM = a; printProgramString (str,Serial); Serial.println(); };
#else
#define SERIAL_PRINT_PGM(a) { Serial.print(a); };
#define SERIAL_PRINTLN_PGM(a) { Serial.println(a); };
#endif

#endif
