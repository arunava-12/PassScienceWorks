
# Embedded Hardware Integration Design

## 1. Overview

The workbench uses a modular host-connected architecture
with three device categories:

- Microcontroller development board
- FPGA development board
- JTAG debug adapter

The current implementation uses C++ mock adapters.
No physical hardware is connected.

## 2. Architecture

Host PC
  |
  +-- USB connection
        |
        +-- Microcontroller adapter
        |
        +-- FPGA adapter
        |
        +-- JTAG debug adapter

The DeviceAdapter interface provides a common software
contract for discovery, capabilities, connection,
disconnection and telemetry.

## 3. Interface and electrical assumptions

| Device | Interface | Signal / voltage assumptions |
|---|---|---|
| MCU | USB / UART | Example STM32 target; GPIO voltage depends on the exact board, commonly 3.3 V |
| FPGA | USB / JTAG | Example Artix-7 target; bank I/O voltage depends on board configuration |
| JTAG | USB to JTAG | TCK, TMS, TDI, TDO, GND and target reference voltage (VTref) |

These are illustrative design assumptions, not verified
pin assignments for a particular physical board.

## 4. Power and grounding

- USB may power a development board only when its design
  explicitly supports USB power.
- External supplies must follow the board manufacturer's
  voltage and current specifications.
- Host and target signal grounds must share a suitable
  reference when required by the interface.
- Never connect two independent power sources without
  checking the board's power-path design.

## 5. Voltage domains and level shifting

- Identify the target I/O voltage before connecting signals.
- Do not assume all MCU and FPGA boards use the same
  logic voltage.
- Use suitable level translators when signal voltage
  domains are incompatible.
- Check translator direction, speed and electrical
  characteristics before selecting a component.

## 6. Connectors and host communication

- MCU: USB connector, with UART exposed where supported.
- FPGA: board-specific USB/JTAG programming interface.
- JTAG: adapter-specific host connector and target-side
  debug connector.

Connector type and pin mapping must be checked against
the exact board and adapter documentation.

## 7. JTAG signal ownership

The debug adapter drives TCK, TMS and TDI.
The target device returns TDO.
Ground and target reference voltage are required as
specified by the adapter.

A JTAG adapter is a debug interface, not a general-purpose
compute board.

## 8. Mock implementation

The C++ implementation demonstrates:

- Device discovery
- Capability reporting
- Connect and disconnect
- One telemetry message per device
- Simulated connection timeout
- Successful retry after timeout

All generated telemetry and connection states are
simulated and do not represent physical measurements.

## 9. Safe integration plan

1. Identify the exact board and adapter models.
2. Read their official pinout and electrical specifications.
3. Verify target voltage and power requirements.
4. Confirm connector orientation and signal ownership.
5. Check grounding and isolation requirements.
6. Begin with current-limited power and no signal connection.
7. Verify signal levels before connecting interfaces.
8. Test communication with a low-risk discovery operation.

## 10. Limitations

- No physical devices are detected.
- No firmware is flashed.
- No FPGA bitstream is programmed.
- No real JTAG debugging is performed.
- Telemetry values are synthetic.
- Electrical values and pin assignments must be
  verified against selected hardware before real integration.
