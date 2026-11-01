
#pragma once

#include <string>
#include <vector>

namespace hardware {

enum class DeviceType {
    Microcontroller,
    FPGA,
    JTAG
};

struct DeviceInfo {
    std::string id;
    std::string name;
    DeviceType type;
    std::string connection;
    bool simulated = true;
};

struct OperationResult {
    bool success = false;
    std::string message;
};

struct TelemetryMessage {
    std::string deviceId;
    std::string message;
    std::string unit;
    double value = 0.0;
    bool simulated = true;
};

class DeviceAdapter {
public:
    virtual ~DeviceAdapter() = default;

    virtual DeviceInfo discover() const = 0;

    virtual std::vector<std::string> capabilities() const = 0;

    virtual OperationResult connect(unsigned timeoutMs) = 0;

    virtual OperationResult disconnect() = 0;

    virtual TelemetryMessage readTelemetry() const = 0;

    virtual void simulateNextTimeout() = 0;

    virtual bool isConnected() const = 0;
};

}
