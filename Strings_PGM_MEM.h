#pragma once

#include <avr/pgmspace.h>

#include <WProgram.h>
#include <Arduino.h>


void printProgramString (prog_char * str, Print & target);

#define SERIAL_PRINT_PGM(a) { static prog_char str[] PROGMEM = a; printProgramString (str,Serial);};
#define SERIAL_PRINTLN_PGM(a) { static prog_char str[] PROGMEM = a; printProgramString (str,Serial); Serial.println(); };

