/*
 * High-Speed Minimal-Overhead Passive I2C Bus Sniffer for ATmega32u4 (Pro Micro)
 *
 * Microcontroller performs ZERO protocol decoding to maximize sampling speed.
 * Uses hardware pin interrupts (INT0 on Pin 3 SCL, INT1 on Pin 2 SDA) with CHANGE mode.
 * On line transition, captures 1-byte state (0x80 | (SCL << 1) | SDA) + 4-byte micros() timestamp.
 * Streams 5-byte sample packets over high-speed USB CDC Serial (500,000 baud).
 *
 * Passive non-intrusive monitoring in high-impedance INPUT mode.
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1 / INT1
#define SCL_PIN 3  // ATmega32u4 PD0 / INT0

// Fast direct port access for ATmega32u4 PIND register
#define READ_SDA() ((PIND & (1 << 1)) != 0)
#define READ_SCL() ((PIND & (1 << 0)) != 0)

struct Sample {
    uint32_t timestamp; // microsecond timestamp
    uint8_t state;      // 0x80 | (scl << 1) | sda
};

#define BUFFER_SIZE 256
volatile Sample sampleBuffer[BUFFER_SIZE];
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

void isrLineChange() {
    uint32_t ts = micros();
    uint8_t pind = PIND;
    uint8_t scl = (pind & (1 << 0)) ? 1 : 0;
    uint8_t sda = (pind & (1 << 1)) ? 1 : 0;

    uint8_t stateByte = 0x80 | (scl << 1) | sda;

    uint8_t nextHead = (head + 1) % BUFFER_SIZE;
    if (nextHead != tail) {
        sampleBuffer[head].timestamp = ts;
        sampleBuffer[head].state = stateByte;
        head = nextHead;
    }
}

void setup() {
    // Strictly high-impedance INPUT mode - never drive outputs
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC max speed
    while (!Serial && millis() < 2000);

    // Attach hardware interrupts on CHANGE mode
    attachInterrupt(digitalPinToInterrupt(SCL_PIN), isrLineChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SDA_PIN), isrLineChange, CHANGE);

    // Initial state
    isrLineChange();
}

void loop() {
    // Drain sample ring buffer and stream 5-byte packets over USB Serial
    while (tail != head) {
        uint32_t ts = sampleBuffer[tail].timestamp;
        uint8_t st = sampleBuffer[tail].state;
        tail = (tail + 1) % BUFFER_SIZE;

        uint8_t pkt[5];
        pkt[0] = st;
        pkt[1] = static_cast<uint8_t>((ts >> 24) & 0xFF);
        pkt[2] = static_cast<uint8_t>((ts >> 16) & 0xFF);
        pkt[3] = static_cast<uint8_t>((ts >> 8) & 0xFF);
        pkt[4] = static_cast<uint8_t>(ts & 0xFF);

        Serial.write(pkt, 5);
    }
}
