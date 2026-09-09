#include "i2c_handler.hpp"
#include <iostream>
#include <cstring>

namespace moza {

I2CHandler::I2CHandler(MozaWheelState& wheelState, uint8_t slaveAddress)
    : wheelState_(wheelState), slaveAddress_(slaveAddress) {}

I2CHandler::~I2CHandler() {
    stop();
}

bool I2CHandler::initialize() {
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize pigpio library." << std::endl;
        return false;
    }

    // Configure BSC (Broadcom Serial Controller) Slave mode
    // GPIO 10 = SDA, GPIO 11 = SCL (Pin 19 and Pin 23 on Pi 4)
    // Control word:
    // Bits 31-16: I2C slave address (0x09)
    // Bit 3: Enable TX FIFO (0x08)
    // Bit 2: Enable RX FIFO (0x04)
    // Bit 0: Enable BSC peripheral (0x01)
    // Total control mask: 0x0D (0b00001101)
    xfer_.control = (slaveAddress_ << 16) | 0x0D;

    // Pre-stage default FC response in TX FIFO
    xfer_.txBuf[0] = static_cast<char>(wheelState_.getFcPayload());
    xfer_.txCnt = 1;

    int status = bscXfer(&xfer_);
    if (status < 0) {
        std::cerr << "Failed to initialize BSC I2C Slave. Error code: " << status << std::endl;
        gpioTerminate();
        return false;
    }

    running_ = true;
    std::cout << "BSC I2C Slave initialized on GPIO 10 (SDA) and GPIO 11 (SCL) at address 0x"
              << std::hex << (int)slaveAddress_ << std::dec << std::endl;
    return true;
}

void I2CHandler::stop() {
    if (running_) {
        xfer_.control = 0; // Disable BSC peripheral
        bscXfer(&xfer_);
        gpioTerminate();
        running_ = false;
        std::cout << "BSC I2C Slave stopped." << std::endl;
    }
}

SlaveState I2CHandler::getNextState(uint8_t received) {
    switch (received) {
        case 0xFC: return SlaveState::FC_RECEIVED;
        case 0xF9: return SlaveState::F9_RECEIVED;
        case 0xDD: return SlaveState::DD_RECEIVED;
        case 0xDE: return SlaveState::DE_RECEIVED;
        default:   return SlaveState::NONE;
    }
}

void I2CHandler::updateTxBufferForState(SlaveState state) {
    switch (state) {
        case SlaveState::FC_RECEIVED:
            xfer_.txBuf[0] = static_cast<char>(wheelState_.getFcPayload());
            xfer_.txCnt = 1;
            break;
        case SlaveState::F9_RECEIVED:
            xfer_.txBuf[0] = static_cast<char>(F9_PAYLOAD);
            xfer_.txCnt = 1;
            break;
        case SlaveState::DD_RECEIVED:
            // Load all 5 button sequence payload bytes into TX FIFO buffer
            for (size_t i = 0; i < 5; ++i) {
                xfer_.txBuf[i] = static_cast<char>(wheelState_.getButtonPayload(i));
            }
            xfer_.txCnt = 5;
            break;
        case SlaveState::DE_RECEIVED:
            // Load all 5 paddle sequence payload bytes into TX FIFO buffer
            for (size_t i = 0; i < 5; ++i) {
                xfer_.txBuf[i] = static_cast<char>(wheelState_.getPaddlePayload(i));
            }
            xfer_.txCnt = 5;
            break;
        case SlaveState::NONE:
        default:
            xfer_.txCnt = 0;
            break;
    }
}

void I2CHandler::processReceivedByte(uint8_t byte) {
    SlaveState next = getNextState(byte);
    if (next != SlaveState::NONE) {
        currentState_ = next;
        updateTxBufferForState(next);
    } else {
        currentState_ = SlaveState::NONE;
    }
}

void I2CHandler::process() {
    if (!running_) return;

    // Check BSC status & transfer FIFO
    int status = bscXfer(&xfer_);
    if (status < 0) {
        return;
    }

    // Process received bytes from I2C Master (Wheelbase) on I2C slave address (0x09)
    if (xfer_.rxCnt > 0) {
        for (int i = 0; i < xfer_.rxCnt; ++i) {
            processReceivedByte(reinterpret_cast<uint8_t*>(xfer_.rxBuf)[i]);
        }

        // Update BSC FIFO transmit buffer with full prepared payload
        bscXfer(&xfer_);
    }
}

} // namespace moza
