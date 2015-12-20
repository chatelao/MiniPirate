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

#include <Arduino.h>
#include "baseIO.h"
#include "modeI2C.h"
// #include <Wire.h>
#include "../Wire/Wire.h" // Hack: See http://forum.arduino.cc/index.php/topic,42818.0.html#5


void ModeI2C::setup() {
  Wire.begin();
}

void ModeI2C::init() {

  Serial.println("SEARCHING I2C DEVICES...");
  Serial.println("========================");

  i2c_address_found = 0;
  byte error   = 0;

  for(int address = 0; address <= 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Wire.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      i2c_address_list[i2c_address_found] = address;
      i2c_address_found++;
    }
    else if (error==4)
    {
      printStrHex("Unknow error at address ", address);
      Serial.println("");
    }
  }
  if(i2c_address_found == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.println("I2C devices found:");
    for(int i = 0; i < i2c_address_found; i++) {
      Serial.print(i);
      printStrHex(": ",  i2c_address_list[i]);
      printStrBin(" - ", i2c_address_list[i]);
      Serial.println("");
    }
  }

  // Set the last address as active
  i2c_address_active = i2c_address_found - 1;
}

void ModeI2C::help() {
}

void ModeI2C::select(char sel_nbre) {

  int new_address = (char)sel_nbre - '0';
  
  if(new_address < i2c_address_found) {
    i2c_address_active = new_address;
    Serial.println("");
    Serial.print("Device changed to ");
    Serial.print(i2c_address_active);
    printStrHex(" (", this->getActiveAddress());
    Serial.print(")");
  }
  Serial.println("");
}

int ModeI2C::getActiveAddress() {

  if(i2c_address_active > -1) {
    return i2c_address_list[i2c_address_active];
  } else {
    return -1;
  }
}

void ModeI2C::read(int read_nbre) {
       Serial.println();
       Serial.print("Requested ");
       Serial.print(read_nbre);
       Serial.print(" bytes ");

       int return_nbre = Wire.requestFrom(getActiveAddress(), read_nbre);
       
       Serial.print(" got ");
       Serial.print(return_nbre);
       Serial.print(" bytes from ");
       bpWhex(getActiveAddress());
       Serial.println("");

       for( int i = 0; i < return_nbre; i++) {
         while(Wire.available() == false);
         int value = Wire.read();
         bpWhex(i);
         Serial.print(": ");
         bpWbin(value);
         Serial.print(" ");
         bpWhex(value);
         Serial.print(" ");
         Serial.println(value);
       }
       Wire.endTransmission();
}

void ModeI2C::write() {
       int count = 0;
       Wire.beginTransmission(getActiveAddress());

       Serial.println();
       Serial.print("Wrote ");

       while(isNumberOrBlankPeek()) {
         pollBlanks();
         if(isNumberPeek()) {
           byte write_value = pollInt();
           Wire.write(write_value);
           Serial.print("(");
           Serial.print(write_value);
           Serial.print("),");
           count++;
         }
       }
       Wire.endTransmission();

       Serial.print(" total of ");
       Serial.print(count);
       Serial.print(" bytes to ");
       bpWhex(getActiveAddress());
       Serial.println();
}

