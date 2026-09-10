#include "paddle_handler.h"

const uint8_t payloadSize = 5;

const uint8_t leftPaddlePayload[payloadSize] = {0xC2, 0xC2, 0xC0, 0xC0, 0xC2};
const uint8_t rightPaddlePayload[payloadSize] = {0xC4, 0xC4, 0xC0, 0xC0, 0xC4};

Paddle paddleMap[] = {
    Paddle('7', paddlePayloads, leftPaddlePayload),
    Paddle('i', paddlePayloads, rightPaddlePayload),
};

const size_t paddleMapSize = sizeof(paddleMap) / sizeof(Paddle);

volatile uint8_t paddlePayloads[payloadSize] = {0x00, 0x00, 0x00, 0x00, 0x00};  // Stores paddle states

void PaddleHandler::updatePaddleState(char key, KeyState state) {
    for (size_t i = 0; i < paddleMapSize; i++) {
        if (key == paddleMap[i].getKey()) {
            if (state == PRESSED) {
                paddleMap[i].addPayload();
            } else if (state == RELEASED) {
                paddleMap[i].removePayload();
            }
            break;
        }
    }
}

void PaddleHandler::resetPayload() {
    for (int i = 0; i < payloadSize; i++) {
        paddlePayloads[i] = 0x00;
    }
}
