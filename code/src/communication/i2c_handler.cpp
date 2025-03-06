#include <Wire.h>

#include "communication/i2c_handler.h"
#include "handlers/keypad_handler.h"  // for btnPayloads
#include "handlers/encoder_handler.h"

volatile SlaveState currentState = NONE;
volatile int8_t btnSeqIdx = -1;

SlaveState getNextState(uint8_t received)
{
    switch (received)
    {
        case 0xFC: return FC_RECEIVED;
        case 0xF9: return F9_RECEIVED;
        case 0xDD: return DD_RECEIVED;
        case 0xDE: return DE_RECEIVED;
        default: return NONE;
    }
}

// i2c request event: master read handler
void requestEvent()
{
    static uint8_t response;

    switch (currentState)
    {
        case FC_RECEIVED:
            response = FC_PAYLOAD;
            break;
        case F9_RECEIVED:
            response = F9_PAYLOAD;
            break;
        case DD_RECEIVED:
            response = btnPayloads[btnSeqIdx] + rotaryPayloads[btnSeqIdx];
            if (rotaryPayloads[btnSeqIdx] != 0x00)
                EncoderHandler::resetPayload();
            break;
        case DE_RECEIVED:
            response = paddlePayloads[btnSeqIdx];
            break;
        case NONE:
        default:
            break;
    }

    currentState = NONE;
    Wire.write(response);
}

void receiveEvent(int bytes)
{
    while (Wire.available())
    {
        uint8_t received = Wire.read();
        SlaveState nextState = getNextState(received);

        if (nextState != NONE)
        {
            currentState = nextState;
            if (nextState == DD_RECEIVED)
                btnSeqIdx++;  // Increment sequence index for button handling
            else if (nextState == FC_RECEIVED)
                btnSeqIdx = -1;  // Reset index
        }
        else
        {
            currentState = NONE;
        }
    }
}