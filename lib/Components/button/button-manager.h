#include "button.h"

class ButtonManager {
    private:
        unsigned short buttonScanLength;
        Button** buttons;        
        unsigned int realButtonCount;
        unsigned int syntheticButtonCount;
    public: 
        /** 
         * Init with a mapping of button number -> button index. 
         * 
         * @param buttonMap This will be used to translate from the position of a given button state to the desired
         *  button number transmitted from the HID. 
         * @param buttonScanLength The number of buttons in a full button scan
         */
        ButtonManager(Button** buttons, unsigned short buttonCount, unsigned short syntheticButtonCount); 
        void updateButtons(bool scanResult[]); // Update the internal state of the buttons
        void getButtonBroadcast(short results[]); // Get an array of buttons that have had their states change since the last time this method was called
        int buttonCount;
        ~ButtonManager();
};