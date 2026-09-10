# MOZA ES Wheel I2C Slave Emulator for Raspberry Pi 4

This directory contains the Raspberry Pi 4 (C++) implementation of the MOZA ES Steering Wheel I2C slave emulator.

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

### Programmatic Input State

The button and paddle payload management is provided via the thread-safe `moza::MozaWheelState` class:

```cpp
#include "moza_wheel_state.hpp"

moza::MozaWheelState wheelState;

// Set button state (button 1-38 mapped according to MOZA Pithouse protocol)
wheelState.setButton(1, true);   // Press button 1
wheelState.setButton(1, false);  // Release button 1

// Set paddle state
wheelState.setLeftPaddle(true);  // Press left paddle
wheelState.setRightPaddle(false); // Release right paddle

// Query button state
bool isBtn1Pressed = wheelState.getButton(1);
```

By default, all buttons and paddles are initialized to unpressed (0x00 payloads).
