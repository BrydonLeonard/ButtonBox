#include <Vector.h>

enum ButtonState { Active, Inactive, ActivePresenting, InactivePresenting };
enum ButtonType { Momentary, SyntheticMomentary };
enum ButtonReleaseBehaviour { None, SyntheticPress };

struct ButtonConfig {
    ButtonType buttonType;
    unsigned short pin;
    unsigned short buttonIndex;
    unsigned long debounceDelay;
    unsigned long momentaryDelay;
    ButtonReleaseBehaviour buttonReleaseBehaviour;
};

struct ButtonNumberDeltaPair {
    unsigned short button;
    unsigned short delta;
};

class Button {
    protected:
        ButtonState buttonState;
        unsigned long debounceEndTime;
        unsigned long momentaryEndTime;
        short buttonDelta;
    public:
        Button();
        Button(ButtonConfig* buttonConfig);
        struct ButtonConfig* buttonConfig;
        unsigned short* registerJoystickButtons(); // Return the button indexes to associate to this button
        virtual unsigned short getStateChange(ButtonNumberDeltaPair* buttonNumberDeltaPair); // Get the state delta for the requested button
        virtual void updateState(bool systemState[]); // Update the internal state of the button. Receives the full system state and will extract the reuired information.
        void updatePinState(bool pinState);
        ~Button();
};

class CompoundButton : public Button {
    private:
        Button* internalButton;
    public:
        CompoundButton(ButtonConfig* buttonConfig, ButtonConfig* internalButtonConfig);
        unsigned short getStateChange(ButtonNumberDeltaPair* buttonNumberDeltaPair);
        void updateState(bool systemState[]);
        ~CompoundButton();
};