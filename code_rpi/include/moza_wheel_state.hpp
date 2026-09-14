#ifndef MOZA_WHEEL_STATE_HPP
#define MOZA_WHEEL_STATE_HPP

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <array>

namespace moza {

enum class PaddleId {
    LEFT = 13,
    RIGHT = 14
};

struct ButtonConfig {
    uint8_t buttonNum;
    uint8_t index;  // DD sequence index (0-4)
    uint8_t value;  // Bitmask value
};

class MozaWheelState {
public:
    static constexpr size_t PAYLOAD_SIZE = 5;

    MozaWheelState();

    // Button state modification
    bool setButton(uint8_t buttonNum, bool pressed);
    bool getButton(uint8_t buttonNum) const;

    // Paddle state modification
    void setPaddle(PaddleId paddle, bool pressed);
    void setLeftPaddle(bool pressed);
    void setRightPaddle(bool pressed);
    bool getPaddle(PaddleId paddle) const;

    // Direct payload getters for I2C thread
    uint8_t getButtonPayload(size_t index) const;
    uint8_t getPaddlePayload(size_t index) const;

    // Direct payload setters if manual payload override is desired
    void setButtonPayload(size_t index, uint8_t value);
    void setPaddlePayload(size_t index, uint8_t value);

    // Reset all inputs to unpressed state
    void reset();

private:
    mutable std::mutex stateMutex_;

    std::array<uint8_t, PAYLOAD_SIZE> btnPayloads_{};
    std::array<uint8_t, PAYLOAD_SIZE> paddlePayloads_{};

    bool leftPaddlePressed_{false};
    bool rightPaddlePressed_{false};

    // Bitfield tracking for individual button states
    uint64_t buttonStates_{0};

    void updatePaddlePayloads();
};

} // namespace moza

#endif // MOZA_WHEEL_STATE_HPP
