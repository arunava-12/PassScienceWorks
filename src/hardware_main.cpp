
#include "MockDevices.h"

#include <iostream>
#include <memory>
#include <vector>

void demonstrateDevice(hardware::DeviceAdapter& device) {
    const auto info = device.discover();

    std::cout << "\n====================================\n";
    std::cout << info.name << '\n';
    std::cout << "ID: " << info.id << '\n';
    std::cout << "Connection: " << info.connection << '\n';
    std::cout << "Status: SIMULATED DEVICE\n";

    std::cout << "\nCapabilities:\n";

    for (const auto& capability : device.capabilities()) {
        std::cout << " - " << capability << '\n';
    }

    const auto connection = device.connect(1000);

    std::cout << "\nConnect: " << connection.message << '\n';

    if (connection.success) {
        const auto telemetry = device.readTelemetry();

        std::cout << "Telemetry: "
                  << telemetry.message
                  << " = "
                  << telemetry.value
                  << ' '
                  << telemetry.unit
                  << " [SIMULATED]\n";

        const auto disconnection = device.disconnect();

        std::cout << "Disconnect: "
                  << disconnection.message << '\n';
    }
}

int main() {
    std::cout << "PASS SCIENCE WORKBENCH\n";
    std::cout << "EMBEDDED HARDWARE SIMULATION\n";
    std::cout << "No physical hardware connected.\n";

    hardware::MockMicrocontrollerAdapter mcu;
    hardware::MockFPGAAdapter fpga;
    hardware::MockJTAGAdapter jtag;

    demonstrateDevice(mcu);
    demonstrateDevice(fpga);
    demonstrateDevice(jtag);

    std::cout << "\n====================================\n";
    std::cout << "TIMEOUT RECOVERY DEMONSTRATION\n";

    fpga.simulateNextTimeout();

    const auto firstAttempt = fpga.connect(1000);

    std::cout << "First attempt: "
              << firstAttempt.message << '\n';

    const auto retry = fpga.connect(1000);

    std::cout << "Retry: "
              << retry.message << '\n';

    if (retry.success) {
        fpga.disconnect();
    }

    std::cout << "\nHardware simulation completed.\n";

    return 0;
}
