
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
