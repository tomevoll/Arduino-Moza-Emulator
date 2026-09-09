# MOZA Wheel I2C Slave Emulator for Raspberry Pi 4

This directory contains the Raspberry Pi 4 (C++) implementation of the MOZA Steering Wheel I2C slave emulator (supporting ES, GS, and FSR wheel identification and LED/Display telemetry).

## Hardware Pinout (Raspberry Pi 4 Model B)

Connect the Raspberry Pi 4 to the MOZA Wheelbase using the following physical pin configuration:

| MOZA Signal | Raspberry Pi Pin Name | Raspberry Pi 4 Physical Pin |
|-------------|-----------------------|-----------------------------|
| **SDA**     | GPIO 10 (BSC MOSI/SDA)| **Physical Pin 19**         |
| **SCL**     | GPIO 11 (BSC SCLK/SCL)| **Physical Pin 23**         |
| **GND**     | Ground                | **Physical Pin 20** (or any Ground pin) |

> **Note:** The Broadcom Serial Controller (BSC) slave interface on Raspberry Pi 4 uses GPIO 10 (SDA) and GPIO 11 (SCL).

---

## Prerequisites

1. **Raspberry Pi OS** (32-bit or 64-bit).
2. **`pigpio` Library**:
   ```bash
   sudo apt-get update
   sudo apt-get install pigpio libpigpio-dev
   ```

---

## Building and Running

1. **Compile the code:**
   ```bash
   cd code_rpi
   make
   ```

2. **Run the executable:**
   The `pigpio` library requires root privileges to access memory and hardware peripherals:
   ```bash
   sudo ./moza_rpi_slave
   ```

---

## Features & Code API Overview

### 1. Wheel Model Selection (GS / ES / FSR)

You can set the wheel model when instantiating `MozaWheelState` (defaults to GS wheel):

```cpp
#include "moza_wheel_state.hpp"

// Emulate MOZA GS Wheel (FC response = 0x08)
moza::MozaWheelState wheelState(moza::WheelModel::GS);

// Or switch model at runtime
wheelState.setWheelModel(moza::WheelModel::FSR); // FC response = 0x0C
```

### 2. Programmatic Input State

```cpp
// Set button state (button 1-38 mapped according to MOZA protocol)
wheelState.setButton(1, true);   // Press button 1
wheelState.setButton(1, false);  // Release button 1

// Set paddle state
wheelState.setLeftPaddle(true);  // Press left paddle
wheelState.setRightPaddle(false); // Release right paddle
```

### 3. Display Telemetry (0x20) & LED (0x08) Callbacks

When emulating display-enabled wheels like the GS/FSR, the wheelbase sends shift light / LED data to address `0x08` and display telemetry data to address `0x20`.

You can register a callback to process incoming packets:

```cpp
wheelState.setTelemetryCallback([](uint8_t address, const std::vector<uint8_t>& data) {
    if (address == moza::LED_ADDRESS) {
        // Handle shift lights / LED data (0x08)
    } else if (address == moza::DISPLAY_ADDRESS) {
        // Handle screen telemetry data (0x20)
    }
});
```

By default, all buttons and paddles are initialized to unpressed (0x00 payloads).
