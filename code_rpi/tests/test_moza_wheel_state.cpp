#include <iostream>
#include <cassert>
#include "moza_wheel_state.hpp"

void testDefaultState() {
    moza::MozaWheelState wheel;
    for (size_t i = 0; i < moza::MozaWheelState::PAYLOAD_SIZE; ++i) {
        assert(wheel.getButtonPayload(i) == 0x00);
        assert(wheel.getPaddlePayload(i) == 0x00);
    }
    std::cout << "[PASS] Default state test" << std::endl;
}

void testButtonSetting() {
    moza::MozaWheelState wheel;

    // Button 1 maps to index 1, value 0x40
    assert(wheel.setButton(1, true));
    assert(wheel.getButton(1) == true);
    assert(wheel.getButtonPayload(1) == 0x40);

    // Button 2 maps to index 1, value 0x80
    assert(wheel.setButton(2, true));
    assert(wheel.getButton(2) == true);
    assert(wheel.getButtonPayload(1) == (0x40 | 0x80));

    // Release Button 1
    assert(wheel.setButton(1, false));
    assert(wheel.getButton(1) == false);
    assert(wheel.getButtonPayload(1) == 0x80);

    // Release Button 2
    assert(wheel.setButton(2, false));
    assert(wheel.getButton(2) == false);
    assert(wheel.getButtonPayload(1) == 0x00);

    std::cout << "[PASS] Button setting test" << std::endl;
}

void testPaddleSetting() {
    moza::MozaWheelState wheel;

    // Left Paddle pattern: {0xC2, 0xC2, 0xC0, 0xC0, 0xC2}
    wheel.setLeftPaddle(true);
    assert(wheel.getPaddle(moza::PaddleId::LEFT) == true);
    assert(wheel.getPaddlePayload(0) == 0xC2);
    assert(wheel.getPaddlePayload(2) == 0xC0);

    // Right Paddle pattern: {0xC4, 0xC4, 0xC0, 0xC0, 0xC4}
    wheel.setRightPaddle(true);
    assert(wheel.getPaddle(moza::PaddleId::RIGHT) == true);
    // uint8_t overflow: 0xC2 + 0xC4 = 0x186 -> 0x86
    assert(wheel.getPaddlePayload(0) == static_cast<uint8_t>(0xC2 + 0xC4));
    assert(wheel.getPaddlePayload(2) == static_cast<uint8_t>(0xC0 + 0xC0));

    // Release Left Paddle
    wheel.setLeftPaddle(false);
    assert(wheel.getPaddlePayload(0) == 0xC4);

    // Release Right Paddle
    wheel.setRightPaddle(false);
    assert(wheel.getPaddlePayload(0) == 0x00);

    std::cout << "[PASS] Paddle setting test" << std::endl;
}

int main() {
    std::cout << "Running MozaWheelState unit tests..." << std::endl;
    testDefaultState();
    testButtonSetting();
    testPaddleSetting();
    std::cout << "All unit tests passed successfully!" << std::endl;
    return 0;
}
