/*
 * Hardware Clock-Synchronized Passive I2C Bus Sniffer for ATmega32u4 (Pro Micro)
 *
 * Uses SCL Rising Edge Interrupt (INT0 / Pin 3 RISING mode) to sample SDA (Pin 2)
 * on every I2C clock pulse without dropping bits at 100kHz-400kHz speeds.
 * Uses SDA Change Interrupt (INT1 / Pin 2 CHANGE mode) to detect START and STOP.
 *
 * Passive non-intrusive monitoring in high-impedance INPUT mode.
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1 / INT1
#define SCL_PIN 3  // ATmega32u4 PD0 / INT0

// Direct port bit reads for ATmega32u4 PIND register
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

#define BUFFER_SIZE 128
volatile I2CEvent eventBuffer[BUFFER_SIZE];
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

volatile bool inTransfer = false;
volatile uint8_t currByte = 0;
volatile uint8_t bitIdx = 0;
volatile uint8_t byteIdx = 0;

void isrSdaChange() {
    bool sda = READ_SDA();
    bool scl = READ_SCL();
    uint32_t ts = micros();

    // START Condition: SDA falls while SCL is HIGH
    if (!sda && scl) {
        inTransfer = true;
        currByte = 0;
        bitIdx = 0;
        byteIdx = 0;

        uint8_t nextHead = (head + 1) % BUFFER_SIZE;
        if (nextHead != tail) {
            eventBuffer[head] = {ts, EV_START, 0, 0};
            head = nextHead;
        }
    }
    // STOP Condition: SDA rises while SCL is HIGH
    else if (sda && scl && inTransfer) {
        inTransfer = false;
        uint8_t nextHead = (head + 1) % BUFFER_SIZE;
        if (nextHead != tail) {
            eventBuffer[head] = {ts, EV_STOP, 0, 0};
            head = nextHead;
        }
    }
}

void isrSclRising() {
    if (!inTransfer) return;

    bool sda = READ_SDA();
    uint32_t ts = micros();

    if (bitIdx < 8) {
        // Sample MSB First
        currByte = (currByte << 1) | (sda ? 1 : 0);
        bitIdx++;
    } else if (bitIdx == 8) {
        // 9th Pulse: ACK (LOW) or NACK (HIGH)
        bool ack = !sda;
        byteIdx++;

        uint8_t nextHead = (head + 1) % BUFFER_SIZE;
        if (nextHead != tail) {
            if (byteIdx == 1) {
                // First byte is 7-bit Address + R/W
                uint8_t addr = (currByte >> 1) & 0x7F;
                bool isRead = (currByte & 0x01) != 0;
                uint8_t flags = (isRead ? 0x02 : 0x00) | (ack ? 0x01 : 0x00);
                eventBuffer[head] = {ts, EV_ADDR, addr, flags};
            } else {
                // Data Byte
                uint8_t flags = (ack ? 0x01 : 0x00);
                eventBuffer[head] = {ts, EV_DATA, currByte, flags};
            }
            head = nextHead;
        }

        currByte = 0;
        bitIdx = 0;
    }
}

void setup() {
    // High impedance passive monitoring - strictly INPUT mode
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC max speed
    while (!Serial && millis() < 2000);

    // Attach hardware pin interrupts
    attachInterrupt(digitalPinToInterrupt(SDA_PIN), isrSdaChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SCL_PIN), isrSclRising, RISING);

    Serial.println("I2C_SNIFFER_READY");
}

void loop() {
    // Drain event ring buffer and stream formatted events over USB CDC Serial
    while (tail != head) {
        I2CEvent ev = eventBuffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;

        Serial.print("E,");
        Serial.print(ev.timestamp);
        Serial.print(",");

        if (ev.type == EV_START) {
            Serial.println("START");
        } else if (ev.type == EV_STOP) {
            Serial.println("STOP");
        } else if (ev.type == EV_ADDR) {
            bool isRead = (ev.flags & 0x02) != 0;
            bool isAck = (ev.flags & 0x01) != 0;
            Serial.print("ADDR,0x");
            if (ev.val < 0x10) Serial.print("0");
            Serial.print(ev.val, HEX);
            Serial.print(",");
            Serial.print(isRead ? "R" : "W");
            Serial.print(",");
            Serial.println(isAck ? "ACK" : "NACK");
        } else if (ev.type == EV_DATA) {
            bool isAck = (ev.flags & 0x01) != 0;
            Serial.print("DATA,0x");
            if (ev.val < 0x10) Serial.print("0");
            Serial.print(ev.val, HEX);
            Serial.print(",");
            Serial.println(isAck ? "ACK" : "NACK");
        }
    }
}
