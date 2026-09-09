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
    // Bit 2: Enable I2C mode
    // Bit 0: Enable BSC peripheral
    xfer_.control = (slaveAddress_ << 16) | 0x05;

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

void I2CHandler::processReceivedByte(uint8_t byte) {
    SlaveState next = getNextState(byte);
    if (next != SlaveState::NONE) {
        currentState_ = next;
        if (next == SlaveState::DD_RECEIVED || next == SlaveState::DE_RECEIVED) {
            btnSeqIdx_++;
            if (btnSeqIdx_ >= 5) {
                btnSeqIdx_ = 0;
            }
        } else if (next == SlaveState::FC_RECEIVED) {
            btnSeqIdx_ = -1;
        }
    } else {
        currentState_ = SlaveState::NONE;
    }
}

uint8_t I2CHandler::prepareResponse() {
    uint8_t response = 0x00;
    size_t idx = (btnSeqIdx_ >= 0 && btnSeqIdx_ < 5) ? static_cast<size_t>(btnSeqIdx_) : 0;

    switch (currentState_) {
        case SlaveState::FC_RECEIVED:
            response = FC_PAYLOAD;
            break;
        case SlaveState::F9_RECEIVED:
            response = F9_PAYLOAD;
            break;
        case SlaveState::DD_RECEIVED:
            response = wheelState_.getButtonPayload(idx);
            break;
        case SlaveState::DE_RECEIVED:
            response = wheelState_.getPaddlePayload(idx);
            break;
        case SlaveState::NONE:
        default:
            response = 0x00;
            break;
    }

    currentState_ = SlaveState::NONE;
    return response;
}

void I2CHandler::process() {
    if (!running_) return;

    // Check BSC status & transfer FIFO
    int status = bscXfer(&xfer_);
    if (status < 0) {
        return;
    }

    // Process received bytes from I2C Master (Wheelbase)
    if (xfer_.rxCnt > 0) {
        for (int i = 0; i < xfer_.rxCnt; ++i) {
            processReceivedByte(reinterpret_cast<uint8_t*>(xfer_.rxBuf)[i]);
        }

        // Prepare response byte for master read
        uint8_t respByte = prepareResponse();
        xfer_.txBuf[0] = static_cast<char>(respByte);
        xfer_.txCnt = 1;

        // Update BSC FIFO transmit buffer
        bscXfer(&xfer_);
    }
}

} // namespace moza
