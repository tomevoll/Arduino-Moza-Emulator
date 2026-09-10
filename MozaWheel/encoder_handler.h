#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include <Arduino.h>
#include <Keypad.h>

#include "button_mapping.h"
#include "encoder.h"

constexpr uint8_t payloadSize = 5;

class EncoderHandler {
public:
    static void updateEncoderState(char key, KeyState state);
    static void resetPayload();
};

extern volatile uint8_t rotaryPayloads[];  // Declare rotaryPayloads for external use

#endif
