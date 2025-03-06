#include "handlers/encoder_handler.h"

Encoder encoderMap[] = {
    Encoder('9', 'a', &rotaryPayloads[btn_33.index], &rotaryPayloads[btn_32.index], btn_33.value, btn_32.value),
    Encoder('f', 'g', &rotaryPayloads[btn_24.index], &rotaryPayloads[btn_37.index], btn_24.value, btn_37.value),
    Encoder('r', 's', &rotaryPayloads[btn_19.index], &rotaryPayloads[btn_20.index], btn_19.value, btn_20.value),
};

const size_t encoderMapSize = sizeof(encoderMap) / sizeof(Encoder);

volatile uint8_t rotaryPayloads[payloadSize] = {0x00, 0x00, 0x00, 0x00, 0x00};  // Stores rotary encoder states

void EncoderHandler::updateEncoderState(char key, KeyState state) {
    for (size_t i = 0; i < encoderMapSize; i++) {
        if (key == encoderMap[i].getKeyA() || key == encoderMap[i].getKeyB()) {
            encoderMap[i].update(key, state);  // Single call, handles everything internally
            break;
        }
    }
}

void EncoderHandler::resetPayload() {
    for (int i = 0; i < payloadSize; i++) {
        rotaryPayloads[i] = 0x00;
    }
}
