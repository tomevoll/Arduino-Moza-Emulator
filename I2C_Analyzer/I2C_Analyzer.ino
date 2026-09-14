/*
 * 100% Passive Promiscuous I2C Bus Sniffer for ATmega32u4 (Pro Micro)
 *
 * Monitors ALL I2C bus traffic for ALL slave addresses without acting as a slave or master.
 * Operates strictly in high-impedance INPUT mode (never pulls SDA or SCL LOW).
 *
 * Pins:
 *   SDA -> Pin 2 (PD1 / INT1)
 *   SCL -> Pin 3 (PD0 / INT0)
 *
 * Streams formatted ASCII events over high-speed USB CDC Serial (500000 baud):
 *   E,TIMESTAMP,START
 *   E,TIMESTAMP,ADDR,0x09,W,ACK
 *   E,TIMESTAMP,DATA,0xFC,ACK
 *   E,TIMESTAMP,STOP
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1 / INT1
#define SCL_PIN 3  // ATmega32u4 PD0 / INT0

// Fast direct port bit reads for ATmega32u4 PIND register
#define READ_SDA() ((PIND & (1 << 1)) != 0)
#define READ_SCL() ((PIND & (1 << 0)) != 0)

enum EventType : uint8_t {
    EV_START = 1,
    EV_STOP  = 2,
    EV_ADDR  = 3,
    EV_DATA  = 4
};

struct I2CEvent {
    uint32_t timestamp;
    uint8_t type;
    uint8_t val;
    uint8_t flags; // bit 0: isAck, bit 1: isRead
};

#define BUFFER_SIZE 256
volatile I2CEvent eventBuffer[BUFFER_SIZE];
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

volatile bool inFrame = false;
volatile uint8_t currByte = 0;
volatile uint8_t bitIdx = 0;
volatile uint8_t byteIdx = 0;

void pushEvent(uint32_t ts, uint8_t type, uint8_t val, uint8_t flags) {
    uint8_t nextHead = (head + 1) % BUFFER_SIZE;
    if (nextHead != tail) {
        eventBuffer[head].timestamp = ts;
        eventBuffer[head].type = type;
        eventBuffer[head].val = val;
        eventBuffer[head].flags = flags;
        head = nextHead;
    }
}

void isrSdaChange() {
    bool sda = READ_SDA();
    bool scl = READ_SCL();
    uint32_t ts = micros();

    // START Condition: SDA falls while SCL is HIGH
    if (!sda && scl) {
        inFrame = true;
        currByte = 0;
        bitIdx = 0;
        byteIdx = 0;
        pushEvent(ts, EV_START, 0, 0);
    }
    // STOP Condition: SDA rises while SCL is HIGH
    else if (sda && scl && inFrame) {
        inFrame = false;
        pushEvent(ts, EV_STOP, 0, 0);
    }
}

void isrSclRising() {
    if (!inFrame) return;

    bool sda = READ_SDA();
    uint32_t ts = micros();

    if (bitIdx < 8) {
        // Collect 8 Bits MSB First
        currByte = (currByte << 1) | (sda ? 1 : 0);
        bitIdx++;
    } else if (bitIdx == 8) {
        // 9th Pulse: ACK (LOW) or NACK (HIGH)
        bool ack = !sda;
        byteIdx++;

        if (byteIdx == 1) {
            // First byte is 7-bit Address + R/W
            uint8_t addr = (currByte >> 1) & 0x7F;
            bool isRead = (currByte & 0x01) != 0;
            uint8_t flags = (isRead ? 0x02 : 0x00) | (ack ? 0x01 : 0x00);
            pushEvent(ts, EV_ADDR, addr, flags);
        } else {
            // Data Byte
            uint8_t flags = (ack ? 0x01 : 0x00);
            pushEvent(ts, EV_DATA, currByte, flags);
        }

        currByte = 0;
        bitIdx = 0;
    }
}

void setup() {
    // 100% Passive High-Impedance INPUT Mode (never drive outputs)
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC Max speed
    while (!Serial && millis() < 2000);

    // Attach hardware pin interrupts
    attachInterrupt(digitalPinToInterrupt(SDA_PIN), isrSdaChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SCL_PIN), isrSclRising, RISING);

    Serial.println("I2C_PASSIVE_PROMISCUOUS_SNIFFER_READY");
}

void loop() {
    // Drain event ring buffer and stream formatted events over USB CDC Serial
    while (tail != head) {
        uint32_t ts = eventBuffer[tail].timestamp;
        uint8_t type = eventBuffer[tail].type;
        uint8_t val = eventBuffer[tail].val;
        uint8_t flags = eventBuffer[tail].flags;
        tail = (tail + 1) % BUFFER_SIZE;

        Serial.print("E,");
        Serial.print(ts);
        Serial.print(",");

        if (type == EV_START) {
            Serial.println("START");
        } else if (type == EV_STOP) {
            Serial.println("STOP");
        } else if (type == EV_ADDR) {
            bool isRead = (flags & 0x02) != 0;
            bool isAck = (flags & 0x01) != 0;
            Serial.print("ADDR,0x");
            if (val < 0x10) Serial.print("0");
            Serial.print(val, HEX);
            Serial.print(",");
            Serial.print(isRead ? "R" : "W");
            Serial.print(",");
            Serial.println(isAck ? "ACK" : "NACK");
        } else if (type == EV_DATA) {
            bool isAck = (flags & 0x01) != 0;
            Serial.print("DATA,0x");
            if (val < 0x10) Serial.print("0");
            Serial.print(val, HEX);
            Serial.print(",");
            Serial.println(isAck ? "ACK" : "NACK");
        }
    }
}
