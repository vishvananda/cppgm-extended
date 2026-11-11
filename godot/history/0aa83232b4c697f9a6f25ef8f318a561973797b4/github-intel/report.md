# Godot headless benchmarks

Compiler: `0aa83232b4c697f9a6f25ef8f318a561973797b4` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1296.56 | 331.21 |
| gcc | 5203.82 | 1311.99 |
| clang | 4351.80 | 1105.36 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.976× | 0.897× | 0.975× | 0.894× |
| arrays | 0.983× | 1.051× | 0.990× | 1.057× |
| strings | 0.861× | 0.843× | 0.847× | 0.841× |
| geometry | 0.937× | 0.905× | 0.941× | 0.903× |
| scene | 0.938× | 0.872× | 0.933× | 0.868× |

A/A variation above 2%: arrays clang cpu after: 0.939×; scene gcc measured_wall before: 0.975×; scene clang cpu before: 1.026×; scene clang measured_wall before: 1.026×; scene clang measured_wall after: 1.022×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114115) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114115)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114115)
