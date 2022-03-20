#include "button-manager.h"
#include <Arduino.h>

/**
 * Button states are managed internally using the actual button matrix positions, but getButtonBroadcast
 * can translate those into the desired button numbers
 */
ButtonManager::ButtonManager(unsigned char buttonMap[], char buttonScanLength, char buttonOutputLength) {
    this->buttonMap = buttonMap;
    this->buttonScanLength = buttonScanLength;
    this->buttonStates = new bool[buttonScanLength];
    this->buttonDeltas = new char[buttonScanLength];
    this->debounceDelay = new unsigned long[buttonScanLength];
    this->buttonCount = buttonOutputLength;

    for (int i = 0; i < buttonScanLength; i++) {
        this->buttonStates[i] = false;
        this->buttonDeltas[i] = 0;
        this->debounceDelay[i] = 0;
    }
}

void ButtonManager::updateButtons(bool scanResult[]) {
    for (int i = 0; i < buttonScanLength; i++) {
        if (debounceDelay[i] < millis()) {
            if (scanResult[i] < buttonStates[i]) {
                buttonDeltas[i] = -1;
                buttonStates[i] = scanResult[i];
                debounceDelay[i] = millis() + 50;
            } else if (scanResult[i] > buttonStates[i]) {
                buttonDeltas[i] = 1;
                buttonStates[i] = scanResult[i];
                debounceDelay[i] = millis() + 50;
            } else {
                buttonDeltas[i] = 0;
            }
            // Set the state so we can use it for future scans
        } else {
            buttonDeltas[i] = 0;
        }
    }
}

void ButtonManager::getButtonBroadcast(char results[]) {
    for (int i = 0; i < buttonCount; i++) {
        results[i] = buttonDeltas[buttonMap[i]];
    }
}