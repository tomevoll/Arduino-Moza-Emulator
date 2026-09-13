/*
 * I2C Passive Bus Sniffer & Logic Analyzer for ATmega32u4 (Pro Micro / Micro)
 *
 * Passive non-intrusive monitoring on default I2C pins:
 *   SDA -> Pin 2 (PD1 / INT1)
 *   SCL -> Pin 3 (PD0 / INT0)
 *
 * Streams live pin state samples (L,SCL,SDA) and decoded I2C events
 * (START, STOP, RESTART, Address HEX + R/W, Data HEX, ACK/NACK) to Processing GUI.
 */

#include <Arduino.h>

#define SDA_PIN 2  // ATmega32u4 PD1
#define SCL_PIN 3  // ATmega32u4 PD0

// Direct port bit reads for ATmega32u4 PIND register
#define READ_SDA() ((PIND & (1 << 1)) != 0)
#define READ_SCL() ((PIND & (1 << 0)) != 0)

inline void sendLineState(bool scl, bool sda) {
    Serial.print("L,");
    Serial.print(scl ? 1 : 0);
    Serial.print(",");
    Serial.println(sda ? 1 : 0);
}

void setup() {
    // High impedance passive monitoring - strictly INPUT mode
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);

    Serial.begin(500000); // USB CDC serial at max USB speed
    while (!Serial && millis() < 2000);

    Serial.println("I2C_ANALYZER_READY");
}

void loop() {
    static bool lastSDA = true;
    static bool lastSCL = true;

    bool curSDA = READ_SDA();
    bool curSCL = READ_SCL();

    // Stream pin state changes for live waveform graph
    if (curSDA != lastSDA || curSCL != lastSCL) {
        sendLineState(curSCL, curSDA);

        // Detect START Condition: SDA drops LOW while SCL is HIGH
        if (lastSDA && !curSDA && curSCL) {
            Serial.println("E,START");

            bool inTransfer = true;
            uint8_t byteIndex = 0;

            // Loop reading bytes until STOP condition or NACK/Timeout
            while (inTransfer) {
                uint8_t rxByte = 0;

                // Read 8 Data Bits (MSB First: bit 7 downto 0)
                for (int b = 7; b >= 0; b--) {
                    // Step A: Wait for SCL to fall LOW
                    while (READ_SCL()) {
                        bool sdaNow = READ_SDA();
                        // Check for STOP condition while SCL is HIGH
                        if (!lastSDA && sdaNow && READ_SCL()) {
                            Serial.println("E,STOP");
                            inTransfer = false;
                            break;
                        }
                        lastSDA = sdaNow;
                    }
                    if (!inTransfer) break;

                    // Step B: Wait for SCL rising edge to go HIGH
                    while (!READ_SCL());

                    // Sample SDA bit value while SCL is HIGH
                    bool bitVal = READ_SDA();
                    if (bitVal) {
                        rxByte |= (1 << b);
                    }

                    // Report line state at SCL rising edge
                    sendLineState(true, bitVal);

                    // Step C: Wait for SCL to fall back LOW
                    while (READ_SCL()) {
                        bool sdaNow = READ_SDA();
                        // Check for STOP condition (SDA rising while SCL HIGH)
                        if (!lastSDA && sdaNow && READ_SCL()) {
                            Serial.println("E,STOP");
                            inTransfer = false;
                            break;
                        }
                        // Check for REPEATED START (SDA falling while SCL HIGH)
                        if (lastSDA && !sdaNow && READ_SCL()) {
                            Serial.println("E,RESTART");
                            byteIndex = 0;
                            break;
                        }
                        lastSDA = sdaNow;
                    }
                    if (!inTransfer) break;
                }

                if (!inTransfer) break;

                // Step D: Read 9th Pulse (ACK / NACK)
                // Wait for 9th SCL rising edge
                while (!READ_SCL());
                bool ackVal = !READ_SDA(); // ACK is LOW (0), NACK is HIGH (1)
                sendLineState(true, !ackVal);

                // Wait for SCL 9th pulse to fall LOW
                while (READ_SCL());

                byteIndex++;
                if (byteIndex == 1) {
                    // First byte in frame is 7-bit Address + R/W
                    uint8_t addr = (rxByte >> 1);
                    bool isRead = (rxByte & 0x01) != 0;

                    Serial.print("E,ADDR,0x");
                    if (addr < 0x10) Serial.print("0");
                    Serial.print(addr, HEX);
                    Serial.print(",");
                    Serial.print(isRead ? "R" : "W");
                    Serial.print(",");
                    Serial.println(ackVal ? "ACK" : "NACK");
                } else {
                    // Subsequent bytes are Data
                    Serial.print("E,DATA,0x");
                    if (rxByte < 0x10) Serial.print("0");
                    Serial.print(rxByte, HEX);
                    Serial.print(",");
                    Serial.println(ackVal ? "ACK" : "NACK");
                }
            }
        }

        // Detect STOP Condition: SDA rises HIGH while SCL is HIGH
        if (!lastSDA && curSDA && curSCL) {
            Serial.println("E,STOP");
        }

        lastSDA = curSDA;
        lastSCL = curSCL;
    }
}
