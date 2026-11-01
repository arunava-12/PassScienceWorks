# Software Architecture

## 1. System overview

PASS Science Workbench is a C++20 Windows desktop application. The code is divided into a native Win32/GDI presentation layer and reusable computational modules. Separate command-line demonstration targets and automated test executables exercise the core modules independently of the GUI.

```text
                  Launch (Win32 / GDI)
                          |
        +-----------------+------------------+
        |                 |                  |
   geometry_core     thermal_core       hardware_core
        |                 |                  |
   STL parser        1D thermal FEM     mock device adapters
   CSV parser                              MCU / FPGA / JTAG

                 performance_core
                        |
                 thermal_core
                 worker pool

                    neural_core
                        |
             manual backpropagation

        BenchmarkRunner / BackpropRunner / HardwareRunner
                         |
                  Automated tests
```

## 2. Presentation layer

`src/ui/MainWindow.cpp` implements the Windows GUI using Win32 controls, GDI drawing and common file dialogs. It provides Geometry, Thermal, Performance and Hardware views. The UI calls core APIs rather than implementing the numerical methods itself.

The performance benchmark is executed on a background `std::thread`. Completion is posted back to the window message loop; UI controls are updated on the UI thread. The worker does not directly manipulate window controls.

## 3. Core modules

| Module | Main implementation | Responsibility |
|---|---|---|
| Thermal | `src/core/thermal/ThermalSolver.h/.cpp` | 1D linear-element FEM, exact reference solution, midpoint error and CSV export |
| Geometry | `src/core/geometry/STLParser.h/.cpp` | ASCII STL parsing and triangle extraction |
| Temperature data | `src/core/geometry/TemperatureCSV.h/.cpp` | Header/field validation, unique node IDs, finite numeric values and temperature range |
| Performance | `src/core/performance/Benchmark.h/.cpp`, `WorkerPool.h` | Deterministic workload, serial/parallel execution, median timing, speedup, efficiency and numerical comparison |
| Neural | `src/core/neural/Backpropagation.h/.cpp` | Explicit forward pass, analytical gradients, parameter updates and finite-difference validation |
| Hardware | `src/core/hardware/DeviceAdapter.h`, `MockDevices.h/.cpp` | Device abstraction and simulated MCU, FPGA and JTAG adapters |

## 4. Thermal solver method

The thermal module assembles the one-dimensional stiffness matrix and source vector for two-node linear elements. Dirichlet endpoint temperatures are applied, and the resulting linear system is solved using Gaussian elimination with partial pivoting. The FE solution is compared against the analytical solution at each element midpoint.

For constant properties and uniform volumetric generation:

`T(x) = T_left + (T_right - T_left)x/L + [q/(2k)]x(L-x)`

## 5. Concurrency

Independent thermal cases are generated from a fixed seed before timing. The worker pool writes each result to its corresponding index, preserving input order. The benchmark compares parallel nodal temperatures against the serial reference and rejects a maximum difference above `1e-8 °C`.

## 6. Error handling and input validation

The core parsers reject malformed records, duplicate node IDs and non-finite numeric values. Thermal parameters are checked for finite values and physically valid positive dimensions/properties. The parsers impose a two-million-record limit to bound input size.

## 7. Build and dependencies

The project uses CMake and C++20. The Windows GUI links the Windows system libraries required for Win32 controls, GDI and common dialogs. Thread support is provided by `Threads::Threads`. The release package includes the MinGW runtime DLLs required by the built executable.

## 8. Known limitations

- The STL reader handles ASCII STL only.
- Temperature samples are drawn as points; no volumetric or surface interpolation is implemented.
- Hardware adapters are mock implementations, not physical drivers.
- The GUI is Windows-specific.
- Portable execution must be confirmed on a clean Windows machine; development-machine success alone does not prove all target machines have the required OS runtime components.
