#ifndef KEYPAD_CONFIG_H
#define KEYPAD_CONFIG_H

#include <Arduino.h>

// Defining button matrix pins and columns for micro
// This is the specific case for using the bu0836a matrix breakout
constexpr byte KEYPAD_ROWS = 6;
constexpr byte KEYPAD_COLS = 6;

const char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '7', 'd', 'j', 'p', 'v'},
    {'2', '8', 'e', 'k', 'q', 'w'},
    {'3', '9', 'f', 'l', 'r', 'x'},
    {'4', 'a', 'g', 'm', 's', 'y'},
    {'5', 'b', 'h', 'n', 't', 'z'},
    {'6', 'c', 'i', 'o', 'u', '0'}
};

// Define pin assignments
const byte rowPins[KEYPAD_ROWS] = {19, 18, 15, 14, 16, 10};
const byte colPins[KEYPAD_COLS] = {4, 5, 6, 7, 8, 9};

#endif
