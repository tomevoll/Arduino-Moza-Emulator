#ifndef I2C_HANDLER_HPP
#define I2C_HANDLER_HPP

#include <cstdint>
#include <atomic>
#include <pigpio.h>
#include "moza_wheel_state.hpp"

namespace moza {

constexpr uint8_t SLAVE_ADDRESS = 0x09;
constexpr uint8_t LED_ADDRESS   = 0x08;
constexpr uint8_t DISPLAY_ADDRESS = 0x20;

constexpr uint8_t F9_PAYLOAD = 0x02;  // response for DW: F9 -> DR: 02

enum class SlaveState {
    NONE,
    FC_RECEIVED,
    F9_RECEIVED,
    DD_RECEIVED,
    DE_RECEIVED
};

class I2CHandler {
public:
    explicit I2CHandler(MozaWheelState& wheelState, uint8_t slaveAddress = SLAVE_ADDRESS);
    ~I2CHandler();

    // Initialize pigpio and BSC I2C Slave interface
    bool initialize();

    // Process pending I2C transactions (call in loop or thread)
    void process();

    // Stop I2C Slave interface
    void stop();

    // Check running status
    bool isRunning() const { return running_; }

private:
    MozaWheelState& wheelState_;
    uint8_t slaveAddress_;
    std::atomic<bool> running_{false};

    SlaveState currentState_{SlaveState::NONE};

    bsc_xfer_t xfer_{};

    SlaveState getNextState(uint8_t received);
    void updateTxBufferForState(SlaveState state);
    void processReceivedByte(uint8_t byte);
};

} // namespace moza

#endif // I2C_HANDLER_HPP
