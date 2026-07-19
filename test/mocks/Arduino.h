#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <math.h>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

#define NUM_DIGITAL_PINS 30
#define NUM_ANALOG_INPUTS 4
#define A0 14

typedef bool boolean;
typedef uint8_t byte;

// Print base class to match Arduino Print class
class Print {
public:
    virtual void print(const char* s) = 0;
};

// Mock stream / Serial input/output simulation
class MockSerial : public Print {
private:
    std::string inputBuffer;
    size_t readIndex;
public:
    std::string outputBuffer;

    MockSerial() : readIndex(0) {}

    void setInput(const std::string& input) {
        inputBuffer = input;
        readIndex = 0;
        outputBuffer.clear();
    }

    int available() {
        return readIndex < inputBuffer.size() ? (int)(inputBuffer.size() - readIndex) : 0;
    }

    int peek() {
        if (readIndex < inputBuffer.size()) {
            return inputBuffer[readIndex];
        }
        return -1;
    }

    int read() {
        if (readIndex < inputBuffer.size()) {
            return inputBuffer[readIndex++];
        }
        return -1;
    }

    void print(char c) {
        outputBuffer += c;
    }

    virtual void print(const char* s) override {
        if (s) outputBuffer += s;
    }

    void print(int val) {
        outputBuffer += std::to_string(val);
    }

    void print(float val) {
        outputBuffer += std::to_string(val);
    }

    void println() {
        outputBuffer += "\r\n";
    }

    void println(const char* s) {
        if (s) outputBuffer += s;
        println();
    }

    void println(int val) {
        outputBuffer += std::to_string(val);
        println();
    }

    int parseInt() {
        int val = 0;
        while (available() && peek() == ' ') {
            read(); // skip blanks
        }
        bool neg = false;
        if (available() && peek() == '-') {
            neg = true;
            read();
        }
        while (available() && peek() >= '0' && peek() <= '9') {
            val = val * 10 + (read() - '0');
        }
        return neg ? -val : val;
    }

    void flush() {}
};

extern MockSerial Serial;

// Mock digital / analog I/O state
extern uint8_t mockPinMode[NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS];
extern uint8_t mockPinVal[NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS];
extern int mockAnalogVal[NUM_ANALOG_INPUTS];

inline void pinMode(int pin, int mode) {
    if (pin >= 0 && pin < (NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS)) {
        mockPinMode[pin] = mode;
    }
}

inline void digitalWrite(int pin, int val) {
    if (pin >= 0 && pin < (NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS)) {
        mockPinVal[pin] = val;
    }
}

inline int digitalRead(int pin) {
    if (pin >= 0 && pin < (NUM_DIGITAL_PINS + NUM_ANALOG_INPUTS)) {
        return mockPinVal[pin];
    }
    return LOW;
}

inline int analogRead(int pin) {
    if (pin >= 0 && pin < NUM_ANALOG_INPUTS) {
        return mockAnalogVal[pin];
    }
    return 0;
}

// Dummy mappings
inline int digitalPinToPort(int pin) { return 1; }
inline int digitalPinToBitMask(int pin) { return (1 << (pin % 8)); }
inline volatile uint8_t* portModeRegister(int port) {
    static uint8_t dummyRegister = 0xFF;
    return &dummyRegister;
}
inline int digitalPinToInterrupt(int pin) { return -1; }

#endif // MOCK_ARDUINO_H
