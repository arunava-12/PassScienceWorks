# AI Assistance and Verification Log

## Purpose

AI assistance was used as a development aid during implementation, debugging, documentation planning and review. The submitted source code was built and tested by the developer. AI-generated suggestions were not treated as proof of correctness.

## Tools and models

Record the exact ChatGPT model label and dates from the conversation history before submission. Do not guess or backdate model/version information.

## Assistance categories

| Work area | Assistance received | Developer validation |
|---|---|---|
| Thermal FEM | Explanations and implementation guidance for assembling and validating a 1D linear-element rod solver | Built the C++ target; compared midpoint errors against the analytical solution; ran ThermalTests |
| Geometry and CSV | Parser design, input validation and sample-data workflow guidance | Built parser tests; loaded the supplied ASCII STL and CSV in the GUI |
| Parallel benchmark | Worker-pool and benchmark methodology guidance | Ran the benchmark, inspected worker timings and checked serial/parallel numerical differences |
| Backpropagation | Gradient derivation and finite-difference checking guidance | Ran the C++ neural demo and gradient-check test |
| Embedded architecture | Mock adapter and electrical-design documentation guidance | Ran HardwareRunner and HardwareTests; clearly labelled all device interactions as simulated |
| Windows GUI | Win32/GDI layout, file-dialog and background benchmark implementation guidance | Rebuilt Launch.exe and manually exercised the GUI |
| Packaging | Runtime dependency inspection and release-folder workflow | Used `objdump`, checked DLL presence and created a staged release directory |

## Example challenge / independent verification

The project includes a finite-difference gradient check for the manual backpropagation implementation. Analytical gradients are compared with central finite differences using epsilon `1e-6`; the test enforces an absolute error tolerance of `1e-5`. This is an independent numerical check rather than relying only on the AI-proposed derivation.

The benchmark also compares serial and parallel temperature arrays and enforces a maximum difference of `1e-8 °C`.

## Incorrect or unsuitable suggestions

Before submission, the developer must add one genuine example from the development conversation where an AI suggestion was incorrect, incomplete or unsuitable, and explain:
1. What the suggestion was.
2. Why it was wrong or incomplete.
3. The test or inspection used to identify the issue.
4. How the implementation was corrected.

Do not invent an example. A suitable candidate may be a path, UI, build or packaging suggestion that did not work in the actual environment, but quote the real exchange accurately.

## Developer responsibility

The developer reviewed, edited, built and tested the resulting implementation. AI output was used as assistance, not as a substitute for understanding. The developer should be prepared to explain the FEM assembly, worker-pool execution, gradient equations, parser validation, GUI message handling and mock hardware interfaces.
