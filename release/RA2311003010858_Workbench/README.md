# PASS Science Workbench

A C++20 Windows desktop engineering workbench developed for the PASS SCIENCE WORKS R&D Internship 2026–2027 assignment.

## Features

- ASCII STL mesh import and interactive 3D viewing.
- ANSYS-style temperature CSV import and colored 3D node visualization.
- One-dimensional steady-state thermal finite-element solver.
- Serial and multithreaded thermal workload benchmark.
- Host-side mock adapters for MCU, FPGA and USB-JTAG device concepts.
- Manual single-sample neural-network backpropagation with finite-difference gradient checking.

## Run the application

On a supported 64-bit Windows system:

1. Extract the complete release folder.
2. Keep `Launch.exe` and the three MinGW runtime DLLs in the same folder.
3. Double-click `Launch.exe`.

The application uses the `data` and `results` folders relative to its working directory. Keep the folder structure intact. If file dialogs are used, sample files are under `data\\geometry` and `data\\thermal`.

This package has been built and tested on the development Windows 11 machine. A clean-machine test is still required before claiming universal portability. The Microsoft Universal CRT is expected to be supplied by supported Windows installations.

## Sample data provenance

- `data/geometry/sample_tetrahedron.stl`: synthetic ASCII STL tetrahedron created for this project; four facets.
- `data/thermal/sample_temperature.csv`: synthetic demonstration data created for this project; eight nodes, 40–100 °C. It is not an ANSYS export or physical measurement.

## Project structure

- `Launch.exe`: native Windows GUI.
- `data/`: sample input files.
- `results/csv/`: example FEM, benchmark and neural-training outputs.
- `results/plots/`: generated SVG loss plot.
- `docs/`: architecture, testing, embedded design, AI-use and ownership documentation.

## Build from source

Requirements: Windows 11 x64, CMake, GCC/MinGW-w64 (MSYS2 UCRT64), C++20 support, and threads support.

From the repository root:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

The GUI target is `Launch`. Other demonstration targets include `BenchmarkRunner`, `BackpropRunner` and `HardwareRunner`.

## Important scope and limitations

- STL import supports ASCII STL, not binary STL.
- Embedded-device integration is simulated. No physical MCU, FPGA or JTAG adapter is controlled.
- Temperature CSV data is visualized as a 3D point cloud; it is not an interpolated surface solution.
- The thermal FEM model is a one-dimensional steady-state rod with constant cross-sectional area and conductivity.
- The neural demonstration is a small single-hidden-unit network trained on one sample.

## Verification

The development-machine CTest run reported five passing suites: ThermalTests, GeometryTests, PerformanceTests, NeuralTests and HardwareTests. GUI interactions were also manually exercised on the development machine. See `docs/testing.md` for details and benchmark context.
