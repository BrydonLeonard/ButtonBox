#include "button.h"
#include <Arduino.h>

Button::Button(ButtonConfig* buttonConfig) {
    this->buttonConfig = buttonConfig;
    this->buttonState = Inactive;
    this->debounceEndTime = 0;
    this->momentaryEndTime = 0;
    this->buttonDelta = 0;
}

Button::Button() {
    this->buttonState = Inactive;
    this->debounceEndTime = 0;
    this->momentaryEndTime = 0;
    this->buttonDelta = 0;
}

unsigned short* Button::registerJoystickButtons() {
    return new unsigned short[1] { this->buttonConfig->buttonIndex };
}

unsigned short Button::getStateChange(ButtonNumberDeltaPair* buttonNumberDeltaPair) {
    buttonNumberDeltaPair->button = this->buttonConfig->buttonIndex;
    buttonNumberDeltaPair->delta = this->buttonDelta;

    return 1;
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
void Button::updatePinState(bool pinState) {
    switch (this->buttonState) {
            case Inactive:
                if (pinState == LOW) {
                    this->buttonState = Active;
                    this->buttonDelta = -1;
                    this->debounceEndTime = millis() + this->buttonConfig->debounceDelay;
                    this->momentaryEndTime = millis() + this->buttonConfig->debounceDelay + this->buttonConfig->momentaryDelay;
                } else {
                    this->buttonDelta = 0;
                }
                break;
            case Active:
                if (pinState == HIGH) {
                    this->buttonState = ActivePresenting;
                    this->buttonDelta = 0;
                } else if ((this->momentaryEndTime < millis()) && (this->buttonConfig->buttonType == SyntheticMomentary)) {
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
}

void Button::updateState(bool systemState[]) {
    int pin = this->buttonConfig->pin;

    updatePinState(*(systemState + pin));
}

Button::~Button() {
    delete this->buttonConfig;
}

CompoundButton::CompoundButton(ButtonConfig* buttonConfig, ButtonConfig* internalButtonConfig) : Button(buttonConfig) {
    this->internalButton = new Button(internalButtonConfig);
}

unsigned short CompoundButton::getStateChange(ButtonNumberDeltaPair* stateChanges) {
    Button::getStateChange(stateChanges);
    this->internalButton->getStateChange(stateChanges + 1);

    return 2;
}

void CompoundButton::updateState(bool systemState[]) {
    Button::updateState(systemState);

    if (this->buttonDelta == 1) {
        this->internalButton->updatePinState(false);
    } else {
        this->internalButton->updatePinState(HIGH);
    }
}

CompoundButton::~CompoundButton() {
    delete this->internalButton;
}