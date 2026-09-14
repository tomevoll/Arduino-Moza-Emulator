#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include <Keypad.h>

#include "button_mapping.h"

class ButtonHandler {
public:
    static void updateButtonState(char key, KeyState state);
    static void resetPayload();
};

extern volatile uint8_t btnPayloads[];  // Declare btnPayloads for external use

#endif
