#ifndef PADDLE_HANDLER_H
#define PADDLE_HANDLER_H

#include <Arduino.h>
#include <Keypad.h>

#include "peripherals/paddle.h"

class PaddleHandler {
public:
    static void updatePaddleState(char key, KeyState state);
    static void resetPayload();
};

extern volatile uint8_t paddlePayloads[];  // Declare paddlePayloads for external use

#endif
