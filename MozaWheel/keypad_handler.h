#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <Keypad.h>
#include "keypad_config.h"   // Contains keypad matrix and pin definitions
#include "button_handler.h"  // Handles button input
#include "encoder_handler.h" // Handles encoder input
#include "paddle_handler.h"  // Handles paddle input

class KeypadHandler {
public:
    KeypadHandler();
    void initialize();
    void processKeypad();  // Call this in the main loop

private:
    Keypad kpd;
};

#endif
