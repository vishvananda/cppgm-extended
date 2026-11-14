# Godot headless benchmarks

Compiler: `411e9fbf346b7707d45b8b4e8cbfa3fea6e2fafd` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1247.24 | 320.61 |
| gcc | 5246.76 | 1326.03 |
| clang | 4174.87 | 1066.79 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.073× | 0.991× | 1.090× | 0.996× |
| arrays | 0.976× | 1.000× | 0.981× | 1.008× |
| strings | 0.934× | 0.934× | 0.922× | 0.938× |
| geometry | 0.975× | 0.862× | 0.971× | 0.847× |
| scene | 0.969× | 0.961× | 0.970× | 0.952× |

A/A variation above 2%: strings gcc cpu after: 1.032×; strings gcc measured_wall after: 1.031×; scene gcc cpu before: 1.024×; scene gcc measured_wall before: 1.023×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051587)
