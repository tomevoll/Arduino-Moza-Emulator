# MOZA Wheel I2C Slave Emulator for Raspberry Pi 4

This directory contains the Raspberry Pi 4 (C++) implementation of the MOZA Steering Wheel I2C slave emulator (supporting ES, GS, and FSR wheel identification, LED/Display telemetry handling, and device metadata).

## Hardware Pinout (Raspberry Pi 4 Model B)

Connect the Raspberry Pi 4 to the MOZA Wheelbase using the following physical pin configuration:

| MOZA Signal | Raspberry Pi Pin Name | Raspberry Pi 4 Physical Pin |
|-------------|-----------------------|-----------------------------|
| **SDA**     | GPIO 10 (BSC MOSI/SDA)| **Physical Pin 19**         |
| **SCL**     | GPIO 11 (BSC SCLK/SCL)| **Physical Pin 23**         |
| **GND**     | Ground                | **Physical Pin 20** (or any Ground pin) |

> **Note:** The Broadcom Serial Controller (BSC) slave interface on Raspberry Pi 4 uses GPIO 10 (SDA) and GPIO 11 (SCL).

---

## Architecture & Communication Protocol

### PC / Wheelbase Telemetry Proxying
1. **PC / SimHub / Azom Plugin (`0x17`)**:
   - On the PC side, software telemetry plugins (like Azom or SimHub) communicate with the MOZA Wheelbase over USB using software protocol address `0x17`.
2. **Wheelbase Proxy / Translation**:
   - The MOZA Wheelbase acts as the I2C Master and proxy. It receives telemetry from the PC, formats it, and broadcasts it across the physical wheel I2C bus to slave addresses `0x20` and `0x08`.
3. **Physical I2C Addresses on Wheel Pins**:
   - **`0x09`**: Primary I2C slave address polled continuously by the wheelbase for device identification (`0xFC`), buttons (`0xDD`), and paddles (`0xDE`).
   - **`0x20`**: Physical I2C slave address used by the wheelbase to write screen/display telemetry packets to display-equipped wheels (e.g. GS/FSR).
   - **`0x08`**: Physical I2C slave address used for RPM shift lights and LED controls.

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

```cpp
// Register callback for physical display (0x20) or LED (0x08) I2C packets
wheelState.setTelemetryCallback([](uint8_t address, const std::vector<uint8_t>& data) {
    if (address == moza::LED_ADDRESS) {
        // Handle shift lights / LED data (0x08)
    } else if (address == moza::DISPLAY_ADDRESS) {
        // Handle screen telemetry data (0x20)
    }
});
```

By default, all buttons and paddles are initialized to unpressed (0x00 payloads).
