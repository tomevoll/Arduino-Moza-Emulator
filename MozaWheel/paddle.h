#ifndef PADDLE_H
#define PADDLE_H

#include <Arduino.h>
#include <Keypad.h>

class Paddle {
   public:
    Paddle(char key, volatile uint8_t paddlePayload[], const uint8_t payload[]);

    void addPayload();
    void removePayload();

    const char getKey();

   private:
    char _key;
    volatile uint8_t *_paddlePayload;
    const uint8_t *_payload;
};

extern Paddle paddleMap[];
extern const size_t paddleMapSize;

#endif
