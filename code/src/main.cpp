#include <Arduino.h>
#include <Wire.h>

#include "handlers/keypad_handler.h"
#include "communication/i2c_handler.h"

KeypadHandler keypad;

// setup: initialize i2c and register event handlers
void setup() {
  Wire.begin(SLAVE_ADDRESS);        // initialize as i2c slave for 
  Wire.setClock(400000);            // set i2c clock speed

  Wire.onRequest(requestEvent);     // register master read handler
  Wire.onReceive(receiveEvent);     // register master write handler


  keypad.initialize();
}

// loop: process keypad events
void loop() {
    keypad.processKeypad();
}
