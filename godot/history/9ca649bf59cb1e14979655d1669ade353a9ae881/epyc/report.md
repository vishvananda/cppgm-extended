# Godot headless benchmarks

Compiler: `9ca649bf59cb1e14979655d1669ade353a9ae881` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1472.96 | 377.57 |
| gcc | 5591.91 | 1408.58 |
| clang | 5595.30 | 1417.23 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.080× | 0.927× | 1.112× | 0.922× |
| arrays | 0.839× | 0.932× | 0.808× | 0.936× |
| strings | 0.896× | 0.856× | 0.879× | 0.848× |
| geometry | 1.163× | 0.882× | 1.208× | 0.871× |
| scene | 0.938× | 0.938× | 0.935× | 0.944× |

A/A variation above 2%: dispatch gcc cpu before: 1.034×; dispatch gcc cpu after: 1.127×; dispatch gcc measured_wall before: 1.041×; dispatch gcc measured_wall after: 1.147×; arrays clang cpu after: 0.978×; arrays clang measured_wall after: 0.980×; scene gcc cpu after: 1.049×; scene gcc measured_wall before: 1.029×; scene gcc measured_wall after: 1.057×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011442) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011442)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011442)
