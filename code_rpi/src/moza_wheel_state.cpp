#include "moza_wheel_state.hpp"
#include <algorithm>

namespace moza {

static constexpr ButtonConfig BUTTON_MAP[] = {
    {1,  1, 0x40},
    {2,  1, 0x80},
    {3,  1, 0x20},
    {4,  0, 0x80},
    {5,  4, 0x80},
    {6,  3, 0x10},
    {7,  3, 0x08},
    {8,  4, 0x40},
    {19, 4, 0x10},
    {20, 4, 0x20},
    {21, 4, 0x08},
    {22, 3, 0x40},
    {23, 3, 0x80},
    {24, 3, 0x20},
    {25, 2, 0x40},
    {32, 0, 0x40},
    {33, 0, 0x10},
    {34, 0, 0x20},
    {35, 1, 0x10},
    {36, 2, 0x80},
    {37, 2, 0x10},
    {38, 2, 0x20}
};

static constexpr uint8_t LEFT_PADDLE_PATTERN[5]  = {0xC2, 0xC2, 0xC0, 0xC0, 0xC2};
static constexpr uint8_t RIGHT_PADDLE_PATTERN[5] = {0xC4, 0xC4, 0xC0, 0xC0, 0xC4};

MozaWheelState::MozaWheelState() {
    reset();
}

void MozaWheelState::reset() {
    std::lock_guard<std::mutex> lock(stateMutex_);
    btnPayloads_.fill(0x00);
    paddlePayloads_.fill(0x00);
    buttonStates_ = 0;
    leftPaddlePressed_ = false;
    rightPaddlePressed_ = false;
}

bool MozaWheelState::setButton(uint8_t buttonNum, bool pressed) {
    const ButtonConfig* config = nullptr;
    for (const auto& btn : BUTTON_MAP) {
        if (btn.buttonNum == buttonNum) {
            config = &btn;
            break;
        }
    }

    if (!config) {
        return false; // Invalid button number
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    bool currentlyPressed = (buttonStates_ & (1ULL << buttonNum)) != 0;

    if (currentlyPressed == pressed) {
        return true; // No state change needed
    }

    if (pressed) {
        buttonStates_ |= (1ULL << buttonNum);
        btnPayloads_[config->index] |= config->value;
    } else {
        buttonStates_ &= ~(1ULL << buttonNum);
        btnPayloads_[config->index] &= ~(config->value);
    }

    return true;
}

bool MozaWheelState::getButton(uint8_t buttonNum) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return (buttonStates_ & (1ULL << buttonNum)) != 0;
}

void MozaWheelState::setPaddle(PaddleId paddle, bool pressed) {
    if (paddle == PaddleId::LEFT) {
        setLeftPaddle(pressed);
    } else if (paddle == PaddleId::RIGHT) {
        setRightPaddle(pressed);
    }
}

void MozaWheelState::setLeftPaddle(bool pressed) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (leftPaddlePressed_ != pressed) {
        leftPaddlePressed_ = pressed;
        updatePaddlePayloads();
    }
}

void MozaWheelState::setRightPaddle(bool pressed) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (rightPaddlePressed_ != pressed) {
        rightPaddlePressed_ = pressed;
        updatePaddlePayloads();
    }
}

bool MozaWheelState::getPaddle(PaddleId paddle) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (paddle == PaddleId::LEFT) return leftPaddlePressed_;
    if (paddle == PaddleId::RIGHT) return rightPaddlePressed_;
    return false;
}

void MozaWheelState::updatePaddlePayloads() {
    for (size_t i = 0; i < PAYLOAD_SIZE; ++i) {
        uint8_t val = 0x00;
        if (leftPaddlePressed_) {
            val |= LEFT_PADDLE_PATTERN[i];
        }
        if (rightPaddlePressed_) {
            val |= RIGHT_PADDLE_PATTERN[i];
        }
        paddlePayloads_[i] = val;
    }
}

uint8_t MozaWheelState::getButtonPayload(size_t index) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (index < PAYLOAD_SIZE) {
        return btnPayloads_[index];
    }
    return 0x00;
}

uint8_t MozaWheelState::getPaddlePayload(size_t index) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (index < PAYLOAD_SIZE) {
        return paddlePayloads_[index];
    }
    return 0x00;
}

void MozaWheelState::setButtonPayload(size_t index, uint8_t value) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (index < PAYLOAD_SIZE) {
        btnPayloads_[index] = value;
    }
}

void MozaWheelState::setPaddlePayload(size_t index, uint8_t value) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (index < PAYLOAD_SIZE) {
        paddlePayloads_[index] = value;
    }
}

} // namespace moza
