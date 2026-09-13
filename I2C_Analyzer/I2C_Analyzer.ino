/*
 * I2C Passive Bus Sniffer & Line Status Analyzer for ATmega32u4 (Pro Micro)
 *
 * Passive non-intrusive monitoring on default I2C pins:
 *   SDA -> Pin 2 (PD1 / INT1)
 *   SCL -> Pin 3 (PD0 / INT0)
 *
 * Outputs continuous RAW LINE STATUS (L,SCL,SDA) for exact waveform graphing
 * AND DECODED I2C PROTOCOL EVENTS (E,START, E,ADDR,0x09,W,ACK, E,DATA,0xFC,ACK, E,STOP)
 * over high-speed USB CDC Serial (500000 baud).
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1
#define SCL_PIN 3  // ATmega32u4 PD0

// Direct port bit reads for ATmega32u4 PIND register
#define READ_SDA() ((PIND & (1 << 1)) != 0)
#define READ_SCL() ((PIND & (1 << 0)) != 0)

inline void sendRawLineStatus(bool scl, bool sda) {
    Serial.print("L,");
    Serial.print(scl ? 1 : 0);
    Serial.print(",");
    Serial.println(sda ? 1 : 0);
}

void setup() {
    // High impedance passive monitoring - strictly INPUT mode (never drive outputs)
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC serial at max USB speed
    while (!Serial && millis() < 2000);

    Serial.println("I2C_ANALYZER_READY");
}

void loop() {
    static bool prevSDA = true;
    static bool prevSCL = true;

    bool curSDA = READ_SDA();
    bool curSCL = READ_SCL();

    // 1. ALWAYS emit RAW LINE STATUS whenever either line changes level
    if (curSDA != prevSDA || curSCL != prevSCL) {
        sendRawLineStatus(curSCL, curSDA);

        // 2. Detect START Condition: SDA drops LOW while SCL is HIGH
        if (prevSDA && !curSDA && curSCL) {
            Serial.println("E,START");

            bool inTransfer = true;
            uint8_t byteIndex = 0;

            while (inTransfer) {
                uint8_t rxByte = 0;

                // Read 8 Data Bits (Bit 7 down to Bit 0)
                for (int bit = 7; bit >= 0; bit--) {
                    // Wait for SCL to go LOW
                    while (READ_SCL()) {
                        bool sdaNow = READ_SDA();
                        if (sdaNow != curSDA) {
                            sendRawLineStatus(true, sdaNow);
                            curSDA = sdaNow;
                            // Check for STOP condition while SCL is HIGH
                            if (sdaNow) {
                                Serial.println("E,STOP");
                                inTransfer = false;
                                break;
                            }
                        }
                    }
                    if (!inTransfer) break;

                    // Emit SCL LOW line status
                    sendRawLineStatus(false, curSDA);

                    // Wait for SCL rising edge to go HIGH
                    while (!READ_SCL()) {
                        bool sdaNow = READ_SDA();
                        if (sdaNow != curSDA) {
                            sendRawLineStatus(false, sdaNow);
                            curSDA = sdaNow;
                        }
                    }

                    // Sample SDA bit while SCL is HIGH
                    curSDA = READ_SDA();
                    sendRawLineStatus(true, curSDA);
                    if (curSDA) {
                        rxByte |= (1 << bit);
                    }

                    // Wait for SCL to fall back LOW
                    while (READ_SCL()) {
                        bool sdaNow = READ_SDA();
                        if (sdaNow != curSDA) {
                            sendRawLineStatus(true, sdaNow);
                            curSDA = sdaNow;
                            // Check for STOP condition
                            if (sdaNow) {
                                Serial.println("E,STOP");
                                inTransfer = false;
                                break;
                            }
                            // Check for REPEATED START
                            if (!sdaNow) {
                                Serial.println("E,RESTART");
                                byteIndex = 0;
                                break;
                            }
                        }
                    }
                    if (!inTransfer) break;

                    sendRawLineStatus(false, curSDA);
                }

                if (!inTransfer) break;

                // 3. Read 9th Pulse: ACK / NACK
                while (!READ_SCL()) {
                    bool sdaNow = READ_SDA();
                    if (sdaNow != curSDA) {
                        sendRawLineStatus(false, sdaNow);
                        curSDA = sdaNow;
                    }
                }

                // Sample 9th bit on SCL HIGH
                curSDA = READ_SDA();
                bool isAck = !curSDA; // ACK is LOW, NACK is HIGH
                sendRawLineStatus(true, curSDA);

                // Wait for 9th SCL pulse to fall LOW
                while (READ_SCL()) {
                    bool sdaNow = READ_SDA();
                    if (sdaNow != curSDA) {
                        sendRawLineStatus(true, sdaNow);
                        curSDA = sdaNow;
                    }
                }
                sendRawLineStatus(false, curSDA);

                // Output decoded byte frame event
                byteIndex++;
                if (byteIndex == 1) {
                    uint8_t addr = (rxByte >> 1);
                    bool isRead = (rxByte & 0x01) != 0;
                    Serial.print("E,ADDR,0x");
                    if (addr < 0x10) Serial.print("0");
                    Serial.print(addr, HEX);
                    Serial.print(",");
                    Serial.print(isRead ? "R" : "W");
                    Serial.print(",");
                    Serial.println(isAck ? "ACK" : "NACK");
                } else {
                    Serial.print("E,DATA,0x");
                    if (rxByte < 0x10) Serial.print("0");
                    Serial.print(rxByte, HEX);
                    Serial.print(",");
                    Serial.println(isAck ? "ACK" : "NACK");
                }
            }
        }

        // Detect STOP Condition: SDA rises HIGH while SCL is HIGH
        if (!prevSDA && curSDA && curSCL) {
            Serial.println("E,STOP");
        }

        prevSDA = curSDA;
        prevSCL = curSCL;
    }
}
