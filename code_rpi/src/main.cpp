#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

#include "moza_wheel_state.hpp"
#include "i2c_handler.hpp"

static volatile bool keepRunning = true;

void signalHandler(int signum) {
    std::cout << "\nSignal " << signum << " received, stopping MOZA ES Wheel Pi Slave..." << std::endl;
    keepRunning = false;
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting MOZA ES Wheel I2C Slave Emulator on Raspberry Pi 4..." << std::endl;
    std::cout << "Target Pins: GPIO 10 (SDA) -> Pin 19, GPIO 11 (SCL) -> Pin 23, GND -> Pin 20" << std::endl;

    moza::MozaWheelState wheelState;
    moza::I2CHandler i2cHandler(wheelState);

    if (!i2cHandler.initialize()) {
        std::cerr << "Initialization failed. Make sure to run with root privileges (sudo) and pigpio is installed." << std::endl;
        return 1;
    }

    std::cout << "I2C Slave running... Press Ctrl+C to exit." << std::endl;
    std::cout << "Current state: No buttons pressed." << std::endl;

    while (keepRunning) {
        i2cHandler.process();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    i2cHandler.stop();
    std::cout << "MOZA ES Wheel Pi Slave stopped cleanly." << std::endl;
    return 0;
}
