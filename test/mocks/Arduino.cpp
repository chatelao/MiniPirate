#include "Arduino.h"

MockSerial Serial;
uint8_t mockPinMode[NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS] = {0};
uint8_t mockPinVal[NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS] = {0};
int mockAnalogVal[NUM_ANALOG_INPUTS] = {0};
