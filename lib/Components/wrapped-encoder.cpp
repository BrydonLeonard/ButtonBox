#include "wrapped-encoder.h"
#include <Arduino.h>
WrappedEncoder::WrappedEncoder(uint8_t pinUp, uint8_t pinDown) {
    this->encoder = new Encoder(pinUp, pinDown);
    pinMode(pinUp, INPUT);
    pinMode(pinDown, INPUT);
}

int WrappedEncoder::getChange() {
    int encoderPosition = this->encoder->read();
    this->encoder->write(0); // Reset so we don't have to keep track of last position

    return encoderPosition;
}