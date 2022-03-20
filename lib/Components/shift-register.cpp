#include <Arduino.h>
#include "shift-register.h"

ShiftRegister::ShiftRegister(int signalPin, int shiftClock, int latchClock) {
    this->signalPin = signalPin;
    this->shiftClock = shiftClock;
    this->latchClock = latchClock;

    pinMode(signalPin, OUTPUT);
    pinMode(shiftClock, OUTPUT);
    pinMode(latchClock, OUTPUT);
}

void ShiftRegister::bumpLatch() {
    digitalWrite(signalPin, HIGH);
    cycle(shiftClock);
    cycle(latchClock);
}

void ShiftRegister::resetHigh() {
    digitalWrite(signalPin, HIGH);
    for (int i = 0; i < 8; i++) {
        cycle(shiftClock);
    }
    cycle(latchClock);
}

void ShiftRegister::pushToLatch(bool level) {
    digitalWrite(signalPin, level);
    cycle(shiftClock);
    cycle(latchClock);
}

void ShiftRegister::cycle(int pin) {
    digitalWrite(pin, LOW);
    digitalWrite(pin, HIGH);
}