#include <unity.h>
#include "Arduino.h"

// Define global variables referenced by baseIO.cpp
float VCC = 5.0f;
int adc_resolution = 10;

// Include the source files directly to test their functions
#include "../../examples/Minipirate/baseIO.cpp"
#include "../../examples/Minipirate/Strings_PGM_MEM.cpp"

void setUp(void) {
    // Reset Serial buffers and digital/analog IO mocks
    Serial.setInput("");
    memset(mockPinMode, 0, sizeof(mockPinMode));
    memset(mockPinVal, 0, sizeof(mockPinVal));
    memset(mockAnalogVal, 0, sizeof(mockAnalogVal));
    adc_resolution = 10;
}

void tearDown(void) {
}

// Test pollInt parsing DECIMAL numbers
void test_pollInt_decimal(void) {
    Serial.setInput("123 ");
    int val = pollInt();
    TEST_ASSERT_EQUAL_INT(123, val);
}

// Test pollInt parsing HEXADECIMAL numbers
void test_pollInt_hex(void) {
    Serial.setInput("0x2A ");
    int val = pollInt();
    TEST_ASSERT_EQUAL_INT(42, val);

    Serial.setInput("0XF ");
    val = pollInt();
    TEST_ASSERT_EQUAL_INT(15, val);
}

// Test pollInt parsing BINARY numbers
void test_pollInt_bin(void) {
    Serial.setInput("0b1010 ");
    int val = pollInt();
    TEST_ASSERT_EQUAL_INT(10, val);

    Serial.setInput("0B1111 ");
    val = pollInt();
    TEST_ASSERT_EQUAL_INT(15, val);
}

// Test pollPin parsing digital pin with 'd' prefix
void test_pollPin_digital(void) {
    Serial.setInput("d5 ");
    int pin = pollPin();
    TEST_ASSERT_EQUAL_INT(5, pin);

    Serial.setInput("5 ");
    pin = pollPin();
    TEST_ASSERT_EQUAL_INT(5, pin);
}

// Test pollPin parsing analog pin with 'a' prefix
void test_pollPin_analog(void) {
    Serial.setInput("a1 ");
    int pin = pollPin();
    // A0 is 14, so A1 should be 15
    TEST_ASSERT_EQUAL_INT(A0 + 1, pin);
}

// Test printStrDec formatting
void test_printStrDec(void) {
    printStrDec("Value: ", 75, 3);
    // 75 with 3 digits padding should print '075'
    TEST_ASSERT_EQUAL_STRING("Value: 075", Serial.outputBuffer.c_str());
}

// Test printStrHex formatting
void test_printStrHex(void) {
    printStrHex("Hex: ", 255);
    TEST_ASSERT_EQUAL_STRING("Hex: 0xFF", Serial.outputBuffer.c_str());
}

// Test printStrBin formatting
void test_printStrBin(void) {
    printStrBin("Bin: ", 5);
    TEST_ASSERT_EQUAL_STRING("Bin: 0b00000101", Serial.outputBuffer.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_pollInt_decimal);
    RUN_TEST(test_pollInt_hex);
    RUN_TEST(test_pollInt_bin);
    RUN_TEST(test_pollPin_digital);
    RUN_TEST(test_pollPin_analog);
    RUN_TEST(test_printStrDec);
    RUN_TEST(test_printStrHex);
    RUN_TEST(test_printStrBin);
    return UNITY_END();
}
