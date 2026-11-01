
# Testing and Sample Data

## Geometry sample

File: data/geometry/sample_tetrahedron.stl

- Format: ASCII STL
- Geometry: Synthetic tetrahedron
- Facets: 4
- Purpose: Validate STL parsing and triangle extraction
- Provenance: Created specifically for this project.
- It is not an externally sourced engineering model.

## Temperature sample

File: data/thermal/sample_temperature.csv

- Format: CSV
- Columns: node_id, x_m, y_m, z_m, temperature_C
- Nodes: 8
- Temperature range: 40°C to 100°C
- Provenance: Synthetic demonstration data created specifically
  for this project.
- It is not exported from ANSYS or a physical experiment.

## Automated tests

Run:

```powershell
cmake --build build --config Release -j 8
ctest --test-dir build --output-on-failure
```

The test suite covers:
- Thermal FEM boundary conditions
- Midpoint error calculation
- Mesh convergence
- Invalid thermal parameters
- ASCII STL parsing
- Temperature CSV parsing
- Duplicate node rejection
- Non-finite temperature rejection


# Multicore Performance Benchmark

## Test environment

| Property | Value |
|---|---|
| CPU | Intel Core Ultra 7 255H |
| Physical cores reported by Windows | 16 |
| Logical processors | 16 |
| Installed RAM | 31.4 GB |
| Compiler | GCC 16.2.0 (MSYS2 UCRT64) |
| CMake | 4.4.4 |
| Build configuration | Release |
| C++ standard | C++20 |

## Workload configuration

- Independent thermal FEM cases: 128
- Elements per case: 100–150
- Random seed: 20261101
- Worker counts: 1, 2, 3, 4
- Warm-up runs: 1
- Measured repetitions: 5
- Reported statistic: Median runtime
- Numerical consistency tolerance: 1e-8 °C

The workload is generated before timing begins.
The timed region excludes compilation, file loading, CSV export,
and rendering. Thread creation, scheduling and result storage
are included in the parallel execution time.

## Measured results

| Workers | Median (ms) | Speedup | Efficiency |
|---:|---:|---:|---:|
| 1 | 21.8894 | 1.0000 | 100.00% |
| 2 | 15.0676 | 1.4527 | 72.64% |
| 3 | 9.2764 | 2.3597 | 78.66% |
| 4 | 6.5484 | 3.3427 | 83.57% |

## Numerical consistency

Maximum temperature difference between serial and parallel
results: 0.000000 °C.

The measured difference is within the required 1e-8 °C tolerance.

## Interpretation

The benchmark demonstrates improved execution time as the
worker count increases for this particular workload.

Four workers achieved approximately 3.34x speedup with
approximately 83.57% parallel efficiency.

The result is workload- and machine-dependent. It does not
establish that four workers are optimal for all workloads.

## Raw results

File: results/csv/performance_benchmark.csv

The CSV contains worker count, median execution time,
speedup, efficiency and maximum temperature difference.
