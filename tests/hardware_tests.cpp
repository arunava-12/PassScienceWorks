
#include "MockDevices.h"

#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testDiscovery() {
    hardware::MockMicrocontrollerAdapter mcu;
    hardware::MockFPGAAdapter fpga;
    hardware::MockJTAGAdapter jtag;

    require(mcu.discover().simulated, "MCU must be labelled simulated.");
    require(fpga.discover().simulated, "FPGA must be labelled simulated.");
    require(jtag.discover().simulated, "JTAG must be labelled simulated.");

    require(mcu.discover().type == hardware::DeviceType::Microcontroller,
            "Incorrect MCU type.");

    require(fpga.discover().type == hardware::DeviceType::FPGA,
            "Incorrect FPGA type.");

    require(jtag.discover().type == hardware::DeviceType::JTAG,
            "Incorrect JTAG type.");
}

void testCapabilities() {
    hardware::MockMicrocontrollerAdapter mcu;
    hardware::MockFPGAAdapter fpga;
    hardware::MockJTAGAdapter jtag;

    require(!mcu.capabilities().empty(), "MCU capabilities are missing.");
    require(!fpga.capabilities().empty(), "FPGA capabilities are missing.");
    require(!jtag.capabilities().empty(), "JTAG capabilities are missing.");
}

void testConnectionAndDisconnection() {
    hardware::MockMicrocontrollerAdapter mcu;

    const auto connected = mcu.connect(1000);

    require(connected.success, "MCU connection failed.");
    require(mcu.isConnected(), "MCU should be connected.");

    const auto disconnected = mcu.disconnect();

    require(disconnected.success, "MCU disconnection failed.");
    require(!mcu.isConnected(), "MCU should be disconnected.");
}

void testTimeoutRecovery() {
    hardware::MockFPGAAdapter fpga;

    fpga.simulateNextTimeout();

    const auto failedConnection = fpga.connect(1000);

    require(!failedConnection.success, "Expected simulated timeout.");
    require(!fpga.isConnected(), "FPGA should remain disconnected.");

    const auto retry = fpga.connect(1000);

    require(retry.success, "FPGA retry should succeed.");
    require(fpga.isConnected(), "FPGA should now be connected.");
}

void testTelemetry() {
    hardware::MockMicrocontrollerAdapter mcu;
    hardware::MockFPGAAdapter fpga;
    hardware::MockJTAGAdapter jtag;

    require(mcu.readTelemetry().simulated, "MCU telemetry must be simulated.");
    require(fpga.readTelemetry().simulated, "FPGA telemetry must be simulated.");
    require(jtag.readTelemetry().simulated, "JTAG telemetry must be simulated.");

    require(mcu.readTelemetry().unit == "deg C", "Incorrect MCU telemetry unit.");
    require(fpga.readTelemetry().unit == "%", "Incorrect FPGA telemetry unit.");
    require(jtag.readTelemetry().unit == "MHz", "Incorrect JTAG telemetry unit.");
}

}

int main() {
    try {
        testDiscovery();
        testCapabilities();
        testConnectionAndDisconnection();
        testTimeoutRecovery();
        testTelemetry();

        std::cout << "All hardware simulation tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Hardware test failed: " << error.what() << '\n';
        return 1;
    }
}
