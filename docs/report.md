# PASS Science Workbench — Technical Report

## 1. Objective

The project implements a Windows desktop engineering workbench combining geometry and temperature-data visualization, a one-dimensional thermal finite-element solver, multicore benchmarking, embedded-device interface simulation and a manually implemented neural-network training example.

## 2. Software design

The application separates the Win32/GDI user interface from reusable C++ core modules. Dedicated modules handle thermal FEM, ASCII STL and temperature CSV parsing, parallel benchmarking, manual backpropagation and mock hardware adapters. CMake builds the GUI, demonstration programs and automated tests.

## 3. Thermal finite-element analysis

The thermal model is a steady-state one-dimensional rod with constant cross-sectional area and thermal conductivity, uniform volumetric heat generation, and prescribed endpoint temperatures. Two-node linear elements are assembled into a global stiffness matrix and load vector. Dirichlet boundary conditions are imposed and the linear system is solved using Gaussian elimination with partial pivoting.

For the assignment parameters (`L=1 m`, `A=1e-4 m²`, `k=10 W/(m·K)`, `q=10000 W/m³`, `T(0)=100 °C`, `T(L)=20 °C`), the exact solution is:

`T(x) = 100 - 80x + [10000/(2×10)]x(1-x)`

The computed maximum midpoint errors were:

| Elements | Maximum midpoint error (°C) |
|---:|---:|
| 10 | 1.25 |
| 20 | 0.3125 |
| 40 | 0.078125 |
| 80 | 0.01953125 |

Halving element size reduces the error by a factor of four, consistent with second-order convergence for this midpoint comparison.

## 4. Geometry and temperature visualization

The geometry parser reads ASCII STL and extracts triangular facets. The temperature CSV parser requires the header `node_id,x_m,y_m,z_m,temperature_C`, validates numeric fields, rejects duplicate IDs and non-finite values, and calculates the temperature range. The GUI renders the supplied tetrahedral mesh and a colored 3D temperature point cloud. The supplied sample inputs are synthetic and their provenance is recorded in `testing.md`.

## 5. Multicore performance

The benchmark uses independently generated thermal cases from a fixed random seed. It performs a warm-up, repeats measurements five times, reports median compute time, and compares worker counts. Workload generation, file I/O and rendering are outside the timed region. Thread creation, scheduling and result storage are part of parallel execution.

The source testing record contains one benchmark run with medians of 21.8894, 15.0676, 9.2764 and 6.5484 ms for 1–4 workers, corresponding to speedups of 1.0000, 1.4527, 2.3597 and 3.3427. The GUI was subsequently exercised with another run; its displayed timings differ, as expected for runtime measurements. The run-specific raw CSV should accompany whichever result table is presented.

The serial/parallel maximum temperature difference was reported as 0.000000 °C, within the `1e-8 °C` acceptance tolerance. Timing and speedup are specific to the machine, compiler, workload and system load.

## 6. Manual backpropagation

A single-hidden-unit network uses `h=tanh(wx+b)` and `y_hat=vh+c`, with squared-error loss `J=0.5(y_hat-y)^2`. Gradients are derived explicitly and parameters are updated using gradient descent. The implementation does not use automatic differentiation. A central finite-difference check with epsilon `1e-6` validates the analytical gradients against an absolute tolerance of `1e-5`.

## 7. Embedded hardware architecture

The software defines a common device-adapter interface and mock implementations representing an MCU, FPGA and USB-JTAG adapter. Discovery, capabilities, connection, disconnection, telemetry and recoverable timeout behavior are demonstrated in software. No physical hardware was connected; electrical details are illustrative and must be verified against actual board documentation before use.

## 8. Verification and limitations

The development-machine automated test run passed all five suites: ThermalTests, GeometryTests, PerformanceTests, NeuralTests and HardwareTests. GUI workflows were manually exercised, including file loading, FEM execution, benchmark display/export and mock-device connection.

Limitations include ASCII-only STL support, point-cloud rather than interpolated temperature visualization, a one-dimensional constant-property FEM model, simulated hardware adapters and a Windows-specific GUI. A clean-machine package test remains necessary to verify standalone delivery.

## 9. Conclusion

The workbench integrates the assignment's numerical, data-processing, concurrency, neural-network, embedded-architecture and desktop-interface components in a modular C++ application. Automated tests and manual GUI checks provide evidence of functionality. Final evaluation should use the included source, raw outputs, build instructions and demonstration, with simulation and portability limitations stated explicitly.
