#ifndef WIRE_H
#define WIRE_H

class TwoWire {
public:
    void begin() {}
    void beginTransmission(int address) {}
    int endTransmission() { return 0; }
    int requestFrom(int address, int quantity) { return quantity; }
    void write(int data) {}
    int read() { return 0; }
    int available() { return 0; }
};

extern TwoWire Wire;

#endif
