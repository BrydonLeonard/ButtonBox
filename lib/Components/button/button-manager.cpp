#include "button-manager.h"
#include <Arduino.h>


/**
 * Button states are managed internally using the actual button matrix positions, but getButtonBroadcast
 * can translate those into the desired button numbers
 * 
 * @param buttonMap a mapping of output number: pin index
 * @param buttonTypes a mapping of output number: pin type
 */
ButtonManager::ButtonManager(Button** buttons, unsigned short realButtonCount, unsigned short syntheticButtonCount) {
    this->buttons = buttons;
    this->realButtonCount = realButtonCount;
    this->syntheticButtonCount = syntheticButtonCount;
    this->buttonCount = realButtonCount + syntheticButtonCount;
}

void ButtonManager::updateButtons(bool scanResult[]) {
    for (unsigned int i = 0; i < realButtonCount; i++) {
        buttons[i]->updateState(scanResult);
    }
}

void ButtonManager::getButtonBroadcast(short results[]) {
    ButtonNumberDeltaPair* pairs[2];
    ButtonNumberDeltaPair temp{NULL, NULL};
    ButtonNumberDeltaPair temp2{NULL, NULL};
    pairs[0] = &temp;
    pairs[1] = &temp2;
    for (unsigned int i = 0; i < realButtonCount; i++) {
        unsigned short stateChangeCount = this->buttons[i]->getStateChange(*pairs);

        for (unsigned int j = 0; j < stateChangeCount; j++) {
            results[pairs[j]->button] = pairs[j]->delta;
        }
    }
}

ButtonManager::~ButtonManager() {
    for (int i = 0; i < realButtonCount; i++) {
        delete buttons[i];
    }
    delete[] buttons;
}