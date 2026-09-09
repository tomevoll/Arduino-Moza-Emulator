#ifndef MOZA_WHEEL_STATE_HPP
#define MOZA_WHEEL_STATE_HPP

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <array>
#include <vector>
#include <string>
#include <functional>

namespace moza {

// Physical I2C slave addresses on wheel bus
constexpr uint8_t SLAVE_ADDRESS   = 0x09; // Input state & wheel identification
constexpr uint8_t LED_ADDRESS     = 0x08; // Shift lights & LED controls
constexpr uint8_t DISPLAY_ADDRESS = 0x20; // Display screen telemetry

enum class WheelModel {
    ES = 0x04,   // Standard MOZA ES Wheel
    GS = 0x08,   // MOZA GS GT Wheel (supports display/LEDs)
    FSR = 0x0C   // MOZA FSR Formula Wheel with Screen
};

enum class PaddleId {
    LEFT = 13,
    RIGHT = 14
};

struct ButtonConfig {
    uint8_t buttonNum;
    uint8_t index;  // DD sequence index (0-4)
    uint8_t value;  // Bitmask value
};

using TelemetryCallback = std::function<void(uint8_t address, const std::vector<uint8_t>& data)>;

class MozaWheelState {
public:
    static constexpr size_t PAYLOAD_SIZE = 5;
    static constexpr size_t TELEMETRY_BUFFER_SIZE = 256;

    explicit MozaWheelState(WheelModel model = WheelModel::GS);

    // Wheel model configuration
    void setWheelModel(WheelModel model);
    WheelModel getWheelModel() const;
    uint8_t getFcPayload() const;

    // Device Metadata strings
    void setDeviceName(const std::string& name);
    std::string getDeviceName() const;

    void setFirmwareVersion(const std::string& version);
    std::string getFirmwareVersion() const;

    void setSerialNumber(const std::string& sn);
    std::string getSerialNumber() const;

    // Formatted ASCII info response helper
    std::vector<uint8_t> getInfoResponse(uint8_t queryType = 0x00) const;

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

    // Telemetry / LED data from wheelbase (Physical Addresses 0x08 & 0x20)
    void updateLedData(const uint8_t* data, size_t length);
    void updateDisplayTelemetry(const uint8_t* data, size_t length);

    std::vector<uint8_t> getLedData() const;
    std::vector<uint8_t> getDisplayTelemetry() const;

    void setTelemetryCallback(TelemetryCallback cb);

    // Reset all inputs to unpressed state
    void reset();

private:
    mutable std::mutex stateMutex_;

    WheelModel model_{WheelModel::GS};

    std::string deviceName_{"MOZA GS Wheel"};
    std::string firmwareVersion_{"v1.2.0.8"};
    std::string serialNumber_{"GS2023080001"};

    std::array<uint8_t, PAYLOAD_SIZE> btnPayloads_{};
    std::array<uint8_t, PAYLOAD_SIZE> paddlePayloads_{};

    bool leftPaddlePressed_{false};
    bool rightPaddlePressed_{false};

    // Bitfield tracking for individual button states
    uint64_t buttonStates_{0};

    // Telemetry & LED data buffers
    std::vector<uint8_t> ledBuffer_{};
    std::vector<uint8_t> displayBuffer_{};

    TelemetryCallback telemetryCallback_{nullptr};

    void updatePaddlePayloads();
};

} // namespace moza

#endif // MOZA_WHEEL_STATE_HPP
