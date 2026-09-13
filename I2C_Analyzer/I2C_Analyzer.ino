/*
 * Interrupt-Driven High-Speed I2C Bus Sniffer with Microsecond Timestamps
 * for ATmega32u4 (Pro Micro / Micro)
 *
 * Uses hardware pin interrupts (INT0 on Pin 3 SCL, INT1 on Pin 2 SDA) with CHANGE mode.
 * Captures pin transitions and exact microsecond timestamps (micros()) into a fast ring buffer.
 * Streams 5-byte sample packets (Sync+State byte + 32-bit uint32_t timestamp) over USB CDC Serial
 * for accurate time-axis rendering and protocol decoding in the Processing GUI app.
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

// Ring buffer size
#define BUFFER_SIZE 256
volatile Sample sampleBuffer[BUFFER_SIZE];
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

// Interrupt Service Routines
void isrLineChange() {
    uint32_t ts = micros(); // High resolution microsecond timer
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
    // High impedance passive monitoring - strictly INPUT mode (never drive bus)
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC max speed
    while (!Serial && millis() < 2000);

    // Attach hardware pin interrupts for INT0 (Pin 3 / SCL) and INT1 (Pin 2 / SDA)
    attachInterrupt(digitalPinToInterrupt(SCL_PIN), isrLineChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SDA_PIN), isrLineChange, CHANGE);

    // Initial state sample
    isrLineChange();
}

void loop() {
    // Drain ring buffer and stream 5-byte sample packets over USB Serial
    while (tail != head) {
        Sample s = sampleBuffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;

        // Packet format: [Sync+State Byte, TS_Byte3, TS_Byte2, TS_Byte1, TS_Byte0]
        uint8_t pkt[5];
        pkt[0] = s.state;
        pkt[1] = static_cast<uint8_t>((s.timestamp >> 24) & 0xFF);
        pkt[2] = static_cast<uint8_t>((s.timestamp >> 16) & 0xFF);
        pkt[3] = static_cast<uint8_t>((s.timestamp >> 8) & 0xFF);
        pkt[4] = static_cast<uint8_t>(s.timestamp & 0xFF);

        Serial.write(pkt, 5);
    }
}
