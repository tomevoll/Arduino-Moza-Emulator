#ifndef I2C_HANDLER_HPP
#define I2C_HANDLER_HPP

#include <cstdint>
#include <atomic>
#include <pigpio.h>
#include "moza_wheel_state.hpp"

namespace moza {

constexpr uint8_t SLAVE_ADDRESS = 0x09;

constexpr uint8_t FC_PAYLOAD = 0x04;  // response for DW: FC -> DR: 04
constexpr uint8_t F9_PAYLOAD = 0x02;  // response for DW: F9 -> DR: 02

// finite state machine states matching original Arduino code exactly
enum SlaveState {
    NONE,
    FC_RECEIVED,
    F9_RECEIVED,
    DD_RECEIVED,
    DB_RECEIVED,
    DE_RECEIVED,
};

class I2CHandler {
public:
    explicit I2CHandler(MozaWheelState& wheelState, uint8_t slaveAddress = SLAVE_ADDRESS);
    ~I2CHandler();

    // Initialize pigpio and BSC I2C Slave interface
    bool initialize();

    // Process pending I2C transactions
    void process();

    // Stop I2C Slave interface
    void stop();

    // Check running status
    bool isRunning() const { return running_; }

private:
    MozaWheelState& wheelState_;
    uint8_t slaveAddress_;
    std::atomic<bool> running_{false};

    SlaveState currentState_{NONE};
    int8_t btnSeqIdx_{-1};

    bsc_xfer_t xfer_{};

    SlaveState getNextState(uint8_t received);
    void updateTxBufferForState(SlaveState state);
    void processReceivedByte(uint8_t byte);
};

} // namespace moza

#endif // I2C_HANDLER_HPP
