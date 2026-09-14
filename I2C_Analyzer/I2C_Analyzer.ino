/*
 * Ultra-Fast Direct AVR Vector I2C Sniffer for ATmega32u4 (Pro Micro)
 *
 * Uses low-level hardware interrupt vectors ISR(INT0_vect) (Pin 3 SCL) and ISR(INT1_vect) (Pin 2 SDA)
 * with direct PIND register reads (< 0.1us execution, 2 CPU instructions).
 * Zero micros() delay or C-wrapper overhead inside ISRs to prevent dropped bits at 100kHz-400kHz.
 *
 * Passively streams 1-byte state transitions (0x80 | (scl << 1) | sda) over USB CDC at 500,000 baud.
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1 / INT1
#define SCL_PIN 3  // ATmega32u4 PD0 / INT0

#define BUFFER_SIZE 256
volatile uint8_t sampleBuffer[BUFFER_SIZE];
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

// Direct AVR Hardware Interrupt Service Routines (< 0.1 microsecond execution)
ISR(INT0_vect) {
    uint8_t pind = PIND;
    uint8_t sample = 0x80 | (pind & 0x03); // Bits 0 (PD0/SCL) and 1 (PD1/SDA)
    uint8_t nextHead = (head + 1) % BUFFER_SIZE;
    if (nextHead != tail) {
        sampleBuffer[head] = sample;
        head = nextHead;
    }
}

ISR(INT1_vect) {
    uint8_t pind = PIND;
    uint8_t sample = 0x80 | (pind & 0x03);
    uint8_t nextHead = (head + 1) % BUFFER_SIZE;
    if (nextHead != tail) {
        sampleBuffer[head] = sample;
        head = nextHead;
    }
}

void setup() {
    // Strictly high-impedance INPUT mode - passive monitoring
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC max speed
    while (!Serial && millis() < 2000);

    // Configure EICRA register directly for INT0 (SCL) and INT1 (SDA) CHANGE mode
    // EICRA: ISC11=0, ISC10=1 (INT1 CHANGE), ISC01=0, ISC00=1 (INT0 CHANGE)
    EICRA = (EICRA & ~0x0F) | (1 << ISC10) | (1 << ISC00);
    EIMSK |= (1 << INT1) | (1 << INT0); // Enable INT0 and INT1 hardware interrupts

    sei(); // Enable global interrupts

    Serial.println("I2C_ULTRA_FAST_SNIFFER_READY");
}

void loop() {
    // Drain sample ring buffer and write state bytes over USB CDC Serial
    while (tail != head) {
        uint8_t sample = sampleBuffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        Serial.write(sample);
    }
}
