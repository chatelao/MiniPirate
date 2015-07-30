/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef modeI2C_h
#define modeI2C_h

#include "modeBase.h"
#include <Wire.h>
// #include "../Wire/Wire.h" // Hack: See http://forum.arduino.cc/index.php/topic,42818.0.html#5

#define I2C_LIST_SIZE 127

class ModeI2C : ModeBase {
private:
  int i2c_address_list[I2C_LIST_SIZE];
  int i2c_address_active;
  int i2c_address_found;
  void list();
  int getActiveAddress();
public:
  void setup();
  void help();
  void init();
  void select(char sel_nbre);
  void read(int read_nbre);
  void write();
};

#endif

