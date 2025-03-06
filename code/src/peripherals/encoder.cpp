#include "peripherals/encoder.h"

Encoder::Encoder(char keyA, char keyB, volatile uint8_t* payloadCW, volatile uint8_t* payloadCCW, uint8_t valueCW, uint8_t valueCCW)
    : _keyA(keyA), _keyB(keyB), _payloadCW(payloadCW), _payloadCCW(payloadCCW), _valueCW(valueCW), _valueCCW(valueCCW), _lastState(0), _keyAState(false), _keyBState(false) {}

char Encoder::getKeyA() {return _keyA;}
char Encoder::getKeyB() {return _keyB;}

int Encoder::encode(bool keyA, bool keyB)
{
    if (keyA == 0 && keyB == 0)
        return 0;
    else if (keyA == 1 && keyB == 0)
        return 1;
    else if (keyA == 1 && keyB == 1)
        return 2;
    else // (keyA == 0 && keyB == 1)
        return 3;
}

void Encoder::update(char key, KeyState state) {
    bool setKey = (state == HOLD);

    if (key == _keyA) {
        _keyAState = setKey;
    } else if (key == _keyB) {
        _keyBState = setKey;
    }

    int currentState = encode(_keyAState, _keyBState);
    int diff = currentState - _lastState;
    _lastState = currentState;

    // Determine rotation direction and update payload
    if (diff == -1 || diff == 3) {
        *_payloadCW += _valueCW;
    } else if (diff == 1 || diff == -3) {
        *_payloadCCW += _valueCCW;
    }
}
