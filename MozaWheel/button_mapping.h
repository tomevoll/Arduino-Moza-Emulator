#ifndef BUTTON_MAPPING_H
#define BUTTON_MAPPING_H

#include <Arduino.h>

struct ButtonMapping {
    char key;
    volatile uint8_t *payload;
    uint8_t value;
};

struct ButtonConfig {
    uint8_t index;  // The index for DD response, can be 0 - 4
    uint8_t value;  // The value for DD response payload
};

// Assigning individual buttons with their index and value
constexpr ButtonConfig btn_1{1, 0x40};
constexpr ButtonConfig btn_2{1, 0x80};
constexpr ButtonConfig btn_3{1, 0x20};
constexpr ButtonConfig btn_4{0, 0x80};
constexpr ButtonConfig btn_5{4, 0x80};
constexpr ButtonConfig btn_6{3, 0x10};
constexpr ButtonConfig btn_7{3, 0x08};
constexpr ButtonConfig btn_8{4, 0x40};

constexpr ButtonConfig btn_19{4, 0x10};
constexpr ButtonConfig btn_20{4, 0x20};
constexpr ButtonConfig btn_21{4, 0x08};
constexpr ButtonConfig btn_22{3, 0x40};
constexpr ButtonConfig btn_23{3, 0x80};
constexpr ButtonConfig btn_24{3, 0x20};
constexpr ButtonConfig btn_25{2, 0x40};

constexpr ButtonConfig btn_32{0, 0x40};
constexpr ButtonConfig btn_33{0, 0x10};
constexpr ButtonConfig btn_34{0, 0x20};
constexpr ButtonConfig btn_35{1, 0x10};
constexpr ButtonConfig btn_36{2, 0x80};
constexpr ButtonConfig btn_37{2, 0x10};
constexpr ButtonConfig btn_38{2, 0x20};

// btnPayloads is defined in keypadHandler.cpp
extern volatile uint8_t btnPayloads[];

// define button mapping array
extern const ButtonMapping buttonMap[];
extern const size_t buttonMapSize;

#endif
