enum ButtonState { Active, Inactive, ActivePresenting, InactivePresenting };
enum ButtonType { Momentary, SyntheticMomentary };
enum ButtonReleaseBehaviour { None, SyntheticPress };

struct ButtonConfig {
    ButtonType buttonType;
    unsigned char syntheticMomentaryDelay;
    unsigned char pin;
    unsigned char buttonIndex;
    unsigned long debounceDelay;
    unsigned long momentaryDelay;
    ButtonReleaseBehaviour buttonReleaseBehaviour;
};

struct StateTransition {
    ButtonState from;
    ButtonState to;
};

typedef void((*StateTransitionCallback)());

class Button {
    protected:
        ButtonState buttonState;
        unsigned long debounceEndTime;
        unsigned long momentaryEndTime;
        char buttonDelta;
        StateTransitionCallback activeInactive;
        StateTransitionCallback inactiveActive;
    public:
        Button(ButtonConfig buttonConfig);
        struct ButtonConfig buttonConfig;
        unsigned char* registerJoystickButtons(); // Return the button indexes to associate to this button
        char getStateChange(unsigned char buttonIndex); // Get the state delta for the requested button
        void updateState(bool systemState[]); // Update the internal state of the button. Receives the full system state and will extract the reuired information.
        void registerActiveInactiveCallback(StateTransitionCallback callback);
        void registerInactiveActiveCallback(StateTransitionCallback callback);
};

class CompoundButton : public Button {
    private:
        Button internalButton;
    public:
        CompoundButton(ButtonConfig buttonConfig, ButtonConfig internalButtonConfig);
        char getStateChange(unsigned char buttonIndex);
        void updateState(bool systemState[]);
};

class ButtonManager {
    private:
        unsigned char* buttonMap;
        unsigned char buttonScanLength;
        ButtonState* buttonStates;
        ButtonType* buttonTypes;
        char* buttonDeltas;
        unsigned long* debounceDelay;
        unsigned long* momentaryDelay;
        void updateMomentaryButton(bool pinState, int pinIndex);
        void updateSyntheticMomentaryButton(bool pinState, int pinIndex);
    public: 
        /** 
         * Init with a mapping of button number -> button index. 
         * 
         * @param buttonMap This will be used to translate from the position of a given button state to the desired
         *  button number transmitted from the HID. 
         * @param buttonScanLength The number of buttons in a full button scan
         */
        ButtonManager(unsigned char buttonMap[], ButtonType buttonTypes[], char buttonScanLength, char buttonOutputLength); 
        void updateButtons(bool scanResult[]); // Update the internal state of the buttons
        void getButtonBroadcast(char results[]); // Get an array of buttons that have had their states change since the last time this method was called
        unsigned char buttonCount;
        ~ButtonManager()
        {
            delete[] buttonMap;
            delete[] buttonStates;
            delete[] buttonDeltas;
            delete[] debounceDelay;
        }
};