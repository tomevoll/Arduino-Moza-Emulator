#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include <Wire.h>

// i2c slave address
#define SLAVE_ADDRESS 0x09

// payloads for different sequences
constexpr uint8_t FC_PAYLOAD = 0x04;  // response for DW: FC -> DR: 04
constexpr uint8_t F9_PAYLOAD = 0x02;  // response for DW: F9 -> DR: 02

// finite state machine states
enum SlaveState {
    NONE,
    FC_RECEIVED,
    F9_RECEIVED,
    DD_RECEIVED,
    DB_RECEIVED,
    DE_RECEIVED,
};

extern volatile SlaveState currentState;
extern volatile int8_t btnSeqIdx;

// function prototypes for i2c event handlers
void requestEvent();
void receiveEvent(int bytes);

#endif
