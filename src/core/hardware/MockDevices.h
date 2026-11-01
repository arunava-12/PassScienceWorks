
#pragma once

#include "DeviceAdapter.h"

namespace hardware {

class BaseMockDevice : public DeviceAdapter {
protected:
    DeviceInfo info_;
    bool connected_ = false;
    bool failNextConnection_ = false;

    BaseMockDevice(
        const std::string& id,
        const std::string& name,
        DeviceType type,
        const std::string& connection
    );

public:
    DeviceInfo discover() const override;

    OperationResult connect(unsigned timeoutMs) override;

    OperationResult disconnect() override;

    void simulateNextTimeout() override;

    bool isConnected() const override;
};

class MockMicrocontrollerAdapter : public BaseMockDevice {
public:
    MockMicrocontrollerAdapter();

    std::vector<std::string> capabilities() const override;

    TelemetryMessage readTelemetry() const override;
};

class MockFPGAAdapter : public BaseMockDevice {
public:
    MockFPGAAdapter();

    std::vector<std::string> capabilities() const override;

    TelemetryMessage readTelemetry() const override;
};

class MockJTAGAdapter : public BaseMockDevice {
public:
    MockJTAGAdapter();

    std::vector<std::string> capabilities() const override;

    TelemetryMessage readTelemetry() const override;
};

}
