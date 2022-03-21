#include "button-manager.h"
#include <Arduino.h>


/**
 * Button states are managed internally using the actual button matrix positions, but getButtonBroadcast
 * can translate those into the desired button numbers
 * 
 * @param buttonMap a mapping of output number: pin index
 * @param buttonTypes a mapping of output number: pin type
 */
ButtonManager::ButtonManager(unsigned char buttonMap[], ButtonType buttonTypes[], char buttonScanLength, char buttonOutputLength) {
    this->buttonMap = buttonMap;
    this->buttonScanLength = buttonScanLength;
    this->buttonStates = new ButtonState[buttonScanLength];
    this->buttonTypes = new ButtonType[buttonScanLength];
    this->buttonDeltas = new char[buttonScanLength];
    this->debounceDelay = new unsigned long[buttonScanLength];
    this->momentaryDelay = new unsigned long[buttonScanLength];
    this->buttonCount = buttonOutputLength;


    for (int i = 0; i < buttonScanLength; i++) {
        this->buttonStates[i] = Inactive;
        this->buttonDeltas[i] = 0;
        this->debounceDelay[i] = 0;
        this->momentaryDelay[i] = 0;
        this->buttonTypes[i] = Momentary;
    }

    // Button number: button type is a convenient format to pass into this method, but
    // it's nicer to have pin index: button type internally. Do that conversion here.
    for (int i = 0; i < buttonOutputLength; i++) {
        this->buttonTypes[buttonMap[i]] = buttonTypes[i];
    }
}

long DEBOUNCE_DURATION = 50;
long MOMENTARY_DURATION = 50; // In addition to debounce duration

/**
 * Assumes that LOW == Active. Update the internal button state.
 * 
 * 
 * TODO: Need to implement momentary switch functionality on all
 *  of the switches so that games that trigger on button up can
 *  understand what's going on. It'll look something like:
 * 
 * ┌─────────┐
 * │Inactive │◄───────────────┬───────┐
 * └─┬───────┘                │       │
 *   ┼                 Debounce delay │
 *   │                 passed │       │
 *  Pressed                   │       │
 *   │                        │       │
 *   ▼                        │       │
 * ┌───────┐                  │       │
 * │Active*│◄───Pressed────┐  │       │
 * └──┬───┬┘               │  │       │
 *    ┼   │                │  │       │
 *    │   └──Released────┐ │  │       │
 *  Synthetic            │ │  │       │
 *  momentary delay      ▼ │  ┼       │
 *  passed           ┌─────┴──┴─┐     │
 *    │              │Active    │     │
 *    ▼              │presenting│     │
 * ┌──────────┐      └──────────┘     │
 * │Inactive  │                       │
 * │presenting├─────Released──────────┘
 * └──────────┘
 * 
 * (*) - Sets both the momentary and debounce delays
 * (+) - These transitions change the visible state of the button 
 */
void ButtonManager::updateButtons(bool scanResult[]) {
    for (int i = 0; i < buttonScanLength; i++) {
        switch (buttonStates[i]) {
            case Inactive:
                if (scanResult[i] == LOW) {
                    buttonStates[i] = Active;
                    buttonDeltas[i] = -1;
                    debounceDelay[i] = millis() + DEBOUNCE_DURATION;
                    momentaryDelay[i] = millis() + DEBOUNCE_DURATION + MOMENTARY_DURATION;
                } else {
                    buttonDeltas[i] = 0;
                }
                break;
            case Active:
                if (scanResult[i] == HIGH) {
                    buttonStates[i] = ActivePresenting;
                    buttonDeltas[i] = 0;
                } else if ((momentaryDelay[i] < millis()) && (buttonTypes[i] == SyntheticMomentary)) {
                    buttonStates[i] = InactivePresenting;
                    buttonDeltas[i] = 1;
                } else {
                    buttonDeltas[i] = 0;
                }
                break;
            case InactivePresenting:
                if (scanResult[i] == HIGH) {
                    buttonStates[i] = Inactive;
                }
                buttonDeltas[i] = 0;
                break;
            case ActivePresenting:
                if (scanResult[i] == LOW) {
                    buttonStates[i] = Active;
                    buttonDeltas[i] = 0;
                } else if (scanResult[i] == HIGH) {
                    buttonStates[i] = Inactive;
                    buttonDeltas[i] = 1;
                }
                break;
        }
    }
}

void ButtonManager::updateMomentaryButton(bool pinState, int pinNumber) {
    
}

void ButtonManager::getButtonBroadcast(char results[]) {
    for (int i = 0; i < buttonCount; i++) {
        results[i] = buttonDeltas[buttonMap[i]];
    }
}