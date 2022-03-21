enum ButtonState { Active, Inactive, ActivePresenting, InactivePresenting };
enum ButtonType { Momentary, SyntheticMomentary };

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