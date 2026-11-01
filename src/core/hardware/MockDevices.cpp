
#include "MockDevices.h"

#include <utility>

namespace hardware {

BaseMockDevice::BaseMockDevice(
    const std::string& id,
    const std::string& name,
    DeviceType type,
    const std::string& connection
) {
    info_.id = id;
    info_.name = name;
    info_.type = type;
    info_.connection = connection;
    info_.simulated = true;
}

DeviceInfo BaseMockDevice::discover() const {
    return info_;
}

OperationResult BaseMockDevice::connect(unsigned timeoutMs) {
    if (connected_) {
        return {true, "Device is already connected."};
    }

    if (failNextConnection_) {
        failNextConnection_ = false;
        return {
            false,
            "Simulated connection timeout. Please retry."
        };
    }

    if (timeoutMs == 0) {
        return {
            false,
            "Connection timeout must be greater than zero."
        };
    }

    connected_ = true;

    return {
        true,
        "Connected to simulated device successfully."
    };
}

OperationResult BaseMockDevice::disconnect() {
    if (!connected_) {
        return {
            true,
            "Device is already disconnected."
        };
    }

    connected_ = false;

    return {
        true,
        "Device disconnected successfully."
    };
}

void BaseMockDevice::simulateNextTimeout() {
    failNextConnection_ = true;
}

bool BaseMockDevice::isConnected() const {
    return connected_;
}

// Microcontroller

MockMicrocontrollerAdapter::MockMicrocontrollerAdapter()
    : BaseMockDevice(
        "MCU-001",
        "STM32 Development Board (Mock)",
        DeviceType::Microcontroller,
        "USB / UART"
    ) {
}

std::vector<std::string>
MockMicrocontrollerAdapter::capabilities() const {
    return {
        "32-bit ARM Cortex-M MCU",
        "GPIO digital input/output",
        "12-bit ADC",
        "UART, SPI and I2C",
        "PWM control"
    };
}

TelemetryMessage
MockMicrocontrollerAdapter::readTelemetry() const {
    return {
        info_.id,
        "Simulated board temperature",
        "deg C",
        32.5,
        true
    };
}

// FPGA

MockFPGAAdapter::MockFPGAAdapter()
    : BaseMockDevice(
        "FPGA-001",
        "Xilinx Artix-7 Development Board (Mock)",
        DeviceType::FPGA,
        "USB / JTAG"
    ) {
}

std::vector<std::string>
MockFPGAAdapter::capabilities() const {
    return {
        "Programmable logic fabric",
        "Configurable logic blocks",
        "Block RAM",
        "DSP slices",
        "Digital I/O"
    };
}

TelemetryMessage MockFPGAAdapter::readTelemetry() const {
    return {
        info_.id,
        "Simulated FPGA utilization",
        "%",
        42.0,
        true
    };
}

// JTAG Debug Adapter

MockJTAGAdapter::MockJTAGAdapter()
    : BaseMockDevice(
        "JTAG-001",
        "USB JTAG Debug Adapter (Mock)",
        DeviceType::JTAG,
        "USB / JTAG"
    ) {
}

std::vector<std::string>
MockJTAGAdapter::capabilities() const {
    return {
        "JTAG debugging interface",
        "TCK clock signal",
        "TMS mode selection",
        "TDI data input",
        "TDO data output",
        "Target debugging support"
    };
}

TelemetryMessage MockJTAGAdapter::readTelemetry() const {
    return {
        info_.id,
        "Simulated JTAG clock frequency",
        "MHz",
        10.0,
        true
    };
}

}
