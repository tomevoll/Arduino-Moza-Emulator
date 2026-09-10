#include "button_handler.h"

const uint8_t payloadSize = 5;

const ButtonMapping buttonMap[] = {
    // right 4 buttons
    {'b', &btnPayloads[btn_1.index], btn_1.value},  // button 1
    {'c', &btnPayloads[btn_2.index], btn_2.value},  // button 2
    {'d', &btnPayloads[btn_3.index], btn_3.value},  // button 3
    {'e', &btnPayloads[btn_4.index], btn_4.value},  // button 4

    // left 4 buttons
    {'1', &btnPayloads[btn_5.index], btn_5.value},  // button 5
    {'2', &btnPayloads[btn_6.index], btn_6.value},  // button 6
    {'3', &btnPayloads[btn_7.index], btn_7.value},  // button 7
    {'4', &btnPayloads[btn_8.index], btn_8.value},  // button 8

    // bottom 2 buttons
    {'p', &btnPayloads[btn_22.index], btn_22.value},  // button 22
    {'h', &btnPayloads[btn_35.index], btn_35.value},  // button 35
};

const size_t buttonMapSize = sizeof(buttonMap) / sizeof(ButtonMapping);

volatile uint8_t btnPayloads[payloadSize] = {0x00, 0x00, 0x00, 0x00, 0x00};  // Stores button states

void ButtonHandler::updateButtonState(char key, KeyState state) {
    for (size_t i = 0; i < buttonMapSize; i++) {
        if (key == buttonMap[i].key) {
            if (state == PRESSED) {
                *buttonMap[i].payload += buttonMap[i].value;
            } else if (state == RELEASED) {
                *buttonMap[i].payload -= buttonMap[i].value;
            }
            break;
        }
    }
}

void ButtonHandler::resetPayload() {
    for (int i = 0; i < payloadSize; i++) {
        btnPayloads[i] = 0x00;
    }
}
