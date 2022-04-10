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

/*
    // Button number: button type is a convenient format to pass into this method, but
    // it's nicer to have pin index: button type internally. Do that conversion here.
    for (int i = 0; i < buttonOutputLength; i++) {
        this->buttonTypes[buttonMap[i]] = buttonTypes[i];
    }*/
}

long DEBOUNCE_DURATION = 40;
long MOMENTARY_DURATION = 50; // In addition to debounce duration


void ButtonManager::updateButtons(bool scanResult[]) {
    for (int i = 0; i < buttonScanLength; i++) {
        
    }
}

void ButtonManager::getButtonBroadcast(char results[]) {
    for (int i = 0; i < buttonCount; i++) {
        results[i] = buttonDeltas[buttonMap[i]];
    }
}

Button::Button(ButtonConfig buttonConfig) {
    this->buttonConfig = buttonConfig;
    this->buttonState = Inactive;
    this->debounceEndTime = 0;
    this->momentaryEndTime = 0;
    this->buttonDelta = 0;
}

void Button::registerActiveInactiveCallback(StateTransitionCallback callback) {
    this->activeInactive = callback;
}

void Button::registerInactiveActiveCallback(StateTransitionCallback callback) {
    this->inactiveActive = callback;
}

unsigned char* Button::registerJoystickButtons() {
    return new unsigned char[1] { this->buttonConfig.buttonIndex };
}

char Button::getStateChange(unsigned char buttonIndex) {
    if (buttonIndex == this->buttonConfig.buttonIndex) {
        return this->buttonDelta;
    }
}

/**
 * Assumes that LOW == Active. Update the internal button state.
 * ┌─────────┐
 * │Inactive │◄─X─────────────┬───────┐
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
 * (X) - Triggers a synthetic release button press (if enabled). The synthetic button follows the behaviour below.
 */
void Button::updateState(bool systemState[]) {
    bool pinState = systemState[this->buttonConfig.pin];

    switch (this->buttonState) {
            case Inactive:
                if (pinState == LOW) {
                    this->buttonState = Active;
                    this->buttonDelta = -1;
                    this->debounceEndTime = millis() + DEBOUNCE_DURATION;
                    this->momentaryEndTime = millis() + DEBOUNCE_DURATION + MOMENTARY_DURATION;
                } else {
                    this->buttonDelta = 0;
                }
                break;
            case Active:
                if (pinState == HIGH) {
                    this->buttonState = ActivePresenting;
                    this->buttonDelta = 0;
                } else if ((this->momentaryEndTime < millis()) && (this->buttonConfig.buttonType == SyntheticMomentary)) {
                    this->buttonState = InactivePresenting;
                    this->buttonDelta = 1;
                } else {
                    this->buttonDelta = 0;
                }
                break;
            case InactivePresenting:
                if (pinState == HIGH) {
                    this->buttonState = Inactive;
                }
                this->buttonDelta = 0;
                break;
            case ActivePresenting:
                if (pinState == LOW) {
                    this->buttonState = Active;
                    this->buttonDelta = 0;
                } else if (pinState == HIGH && this->debounceEndTime < millis()) {
                    this->buttonState = Inactive;
                    this->buttonDelta = 1;
                } else {
                    this->buttonDelta = 0;
                }
                break;
        }

    if (this->buttonDelta == 1) {
        if (this->activeInactive != NULL) {
            this->activeInactive();
        }
    } else if (this->buttonDelta == -1) {
        if (this->inactiveActive != NULL) {
            this->inactiveActive();
        }
    }
}

CompoundButton::CompoundButton(ButtonConfig buttonConfig, ButtonConfig internalButtonConfig) : Button(buttonConfig) {
    this->internalButton = Button(internalButtonConfig);
}

char CompoundButton::getStateChange(unsigned char buttonIndex) {
    // It's possible that the internal button will have the same joystick button as the parent, so fetch both
    char delta = Button::getStateChange(buttonIndex);
    char internalButtonDelta = this->internalButton.getStateChange(buttonIndex);

    if (delta != internalButtonDelta && delta != 0) {
        return delta;
    }

    if (delta == 0) {
        return internalButtonDelta;
    }

    return delta;
}

void CompoundButton::updateState(bool systemState[]) {
    Button::updateState(systemState);
    this->internalButton.updateState(systemState);
}