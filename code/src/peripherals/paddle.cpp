#include "peripherals/paddle.h"

Paddle::Paddle(char key, volatile uint8_t paddlePayload[], const uint8_t payload[])
    : _key(key), _paddlePayload(paddlePayload), _payload(payload) {
}

const char Paddle::getKey() { return _key; }

void Paddle::addPayload() {
    for (int i = 0; i < 5; i++) {
        _paddlePayload[i] += _payload[i];
    }
}

void Paddle::removePayload() {
    for (int i = 0; i < 5; i++) {
        _paddlePayload[i] -= _payload[i];
    }
}