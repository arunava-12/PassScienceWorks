# AI Assistance and Verification Log

## Purpose

AI assistance was used as a development aid during implementation, debugging, documentation planning and technical review of PassScienceWorkbench. It supported the developer with explanations, implementation guidance, troubleshooting and code review.

AI-generated suggestions were not treated as proof of correctness. The developer reviewed, modified, built and tested the implementation.

## Tools and Models

- **AI tool:** ChatGPT
- **Model:** GPT-6
- **Development period:** October–November 2026 (refer to the actual conversation history for the exact dates of individual assistance sessions).

## Assistance Categories

| Work Area | Assistance Received | Developer Validation |
|---|---|---|
| Thermal FEM | Explanations and implementation guidance for assembling and validating a 1D linear-element rod solver | Built the C++ target, compared midpoint errors against the analytical solution and ran ThermalTests |
| Geometry and CSV | Parser design, input validation and sample-data workflow guidance | Built parser tests and loaded the supplied ASCII STL and CSV in the GUI |
| Parallel Benchmark | Worker-pool implementation and benchmark methodology guidance | Ran the benchmark, inspected worker timings and checked serial/parallel numerical differences |
| Backpropagation | Gradient derivation and finite-difference checking guidance | Ran the C++ neural demo and gradient-check test |
| Embedded Architecture | Mock adapter design and electrical-design documentation guidance | Ran HardwareRunner and HardwareTests; clearly labelled all device interactions as simulated |
| Windows GUI | Win32/GDI layout, file-dialog and background benchmark implementation guidance | Rebuilt Launch.exe and manually exercised the GUI |
| Packaging | Runtime dependency inspection and release-folder workflow guidance | Inspected executable dependencies, checked DLL presence and created a staged release directory |

## Independent Verification

Several numerical checks were implemented to verify important parts of the application independently of AI-generated suggestions.

### 1. Thermal FEM verification

The FEM solver was compared against the analytical solution at element midpoints.

| Elements | Maximum Absolute Error |
|---:|---:|
| 10 | 1.25 °C |
| 20 | 0.3125 °C |
| 40 | 0.078125 °C |
| 80 | 0.01953125 °C |

The error decreases as the mesh is refined, showing the expected convergence behaviour for the implemented linear-element method.

### 2. Backpropagation verification

The manual backpropagation implementation was checked against central finite differences using:

- Epsilon: `1e-6`
- Maximum permitted absolute gradient error: `1e-5`

The implementation passed the gradient-check test. No automatic differentiation library was used for the gradient calculation.

### 3. Parallel benchmark verification

The benchmark compares serial and parallel temperature arrays and enforces a maximum numerical difference of `1e-8 °C`.

The benchmark also uses a fixed seed, identical workloads and repeated measurements to improve reproducibility.

## Incorrect or Incomplete AI Suggestions

**Developer must complete this section before submission.**

One genuine example from the development process must be documented:

1. **Original suggestion:** Describe the actual AI suggestion that did not work or was incomplete.
2. **Problem:** Explain why it was incorrect or unsuitable for the project's actual environment.
3. **Verification:** Describe the command, test, build output or manual inspection used to identify the problem.
4. **Correction:** Explain what was changed and how the corrected approach was verified.

Only include an example that genuinely occurred during development. Do not invent an AI mistake or describe a successful suggestion as an incorrect one.

## Developer Responsibility

The developer is responsible for the final source code, implementation decisions, testing and submitted documentation.

AI output was used as development assistance, not as a substitute for understanding or verification. The developer should be able to explain and demonstrate:

- FEM assembly, boundary conditions and numerical convergence.
- Worker-pool execution and serial/parallel comparison.
- Manual backpropagation equations and gradient checking.
- STL and temperature CSV parsing and input validation.
- Win32 GUI message handling and background benchmark execution.
- Mock hardware interfaces and the distinction between simulated and physical devices.