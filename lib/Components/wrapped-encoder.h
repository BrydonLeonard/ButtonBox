#include <Encoder.h>

class WrappedEncoder {
    public: 
        WrappedEncoder(uint8_t pinUp, uint8_t pinDown);
        int getChange(); // -1, 0, or 1
    
    private:
        Encoder *encoder;
};