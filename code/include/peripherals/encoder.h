#ifndef ENCODER_H
#define ENCODER_H

#include <Keypad.h>

class Encoder {
public:
    Encoder(char keyA, char keyB, volatile uint8_t* payloadCW, volatile uint8_t* payloadCCW, uint8_t valueCW, uint8_t valueCCW);

    void update(char key, KeyState state);
    
    char getKeyA();
    char getKeyB();

private:
    char _keyA, _keyB;
    volatile uint8_t *_payloadCW, *_payloadCCW;
    uint8_t _valueCW, _valueCCW;
    
    int _lastState;
    bool _keyAState, _keyBState;

    int encode(bool keyA, bool keyB);
};

extern Encoder encoderMap[];
extern const size_t encoderMapSize;

#endif