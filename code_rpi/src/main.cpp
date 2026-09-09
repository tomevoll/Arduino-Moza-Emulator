#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

#include "moza_wheel_state.hpp"
#include "i2c_handler.hpp"

static volatile bool keepRunning = true;

void signalHandler(int signum) {
    std::cout << "\nSignal " << signum << " received, stopping MOZA Wheel Pi Slave..." << std::endl;
    keepRunning = false;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting MOZA GS Wheel I2C Slave Emulator on Raspberry Pi 4..." << std::endl;
    std::cout << "Target Pins: GPIO 10 (SDA) -> Pin 19, GPIO 11 (SCL) -> Pin 23, GND -> Pin 20" << std::endl;

    // Configure wheel model as GS Wheel (0x08 payload)
    moza::MozaWheelState wheelState(moza::WheelModel::GS);

    // Register optional telemetry / LED callback
    wheelState.setTelemetryCallback([](uint8_t address, const std::vector<uint8_t>& data) {
        if (address == moza::LED_ADDRESS) {
            std::cout << "[LED Update] Received " << data.size() << " bytes on 0x08" << std::endl;
        } else if (address == moza::DISPLAY_ADDRESS) {
            std::cout << "[Display Telemetry] Received " << data.size() << " bytes on 0x20" << std::endl;
        }
    });

    moza::I2CHandler i2cHandler(wheelState);

    if (!i2cHandler.initialize()) {
        std::cerr << "Initialization failed. Make sure to run with root privileges (sudo) and pigpio is installed." << std::endl;
        return 1;
    }

    std::cout << "I2C Slave running as MOZA GS Wheel (FC Payload: 0x"
              << std::hex << (int)wheelState.getFcPayload() << std::dec << ")..." << std::endl;
    std::cout << "Current state: No buttons pressed. Listening for telemetry on 0x08 and 0x20." << std::endl;

    while (keepRunning) {
        i2cHandler.process();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    i2cHandler.stop();
    std::cout << "MOZA Wheel Pi Slave stopped cleanly." << std::endl;
    return 0;
}
