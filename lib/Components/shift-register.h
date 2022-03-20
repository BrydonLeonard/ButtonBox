class ShiftRegister {
    public: 
        ShiftRegister(int signalPin, int shiftClock, int latchClock);
        void bumpLatch(); // Move the latch output on by one
        void resetHigh(); // Set the entire shift register to high
        void pushToLatch(bool level); // Push the value all the way through to the latch
    
    private:
        int signalPin;
        int shiftClock;
        int latchClock;
        void cycle(int pin);
};