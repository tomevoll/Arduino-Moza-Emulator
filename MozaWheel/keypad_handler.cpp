#include "keypad_handler.h"

KeypadHandler::KeypadHandler()
    : kpd(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS) {}

void KeypadHandler::initialize() {
    // setting the hold time to lower delay for encoder inputs
    kpd.setHoldTime(10);
}

void KeypadHandler::processKeypad() {
    if (kpd.getKeys()) {
        for (int i = 0; i < LIST_MAX; i++) {
            if (kpd.key[i].stateChanged) {
                KeyState state = kpd.key[i].kstate;
                char keyChar = kpd.key[i].kchar;

                PaddleHandler::updatePaddleState(keyChar, state);
                ButtonHandler::updateButtonState(keyChar, state);
                EncoderHandler::updateEncoderState(keyChar, state);
            }
        }
    }
}
