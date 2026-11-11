# Godot headless benchmarks

Compiler: `e28ecf1897299c1d70259b05c8315697e3bf589b` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1095.02 | 282.11 |
| gcc | 4288.81 | 1081.54 |
| clang | 3822.46 | 981.34 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.882× | 0.831× | 0.861× | 0.806× |
| arrays | 0.790× | 0.856× | 0.761× | 0.846× |
| strings | 0.831× | 0.830× | 0.815× | 0.816× |
| geometry | 0.843× | 0.836× | 0.829× | 0.819× |
| scene | 0.922× | 0.898× | 0.919× | 0.895× |

A/A variation above 2%: dispatch gcc cpu before: 1.034×; dispatch gcc measured_wall before: 1.026×; strings clang measured_wall before: 0.979×; scene gcc cpu before: 1.033×; scene gcc measured_wall before: 1.056×; scene clang cpu before: 1.027×; scene clang cpu after: 0.974×; scene clang measured_wall before: 1.031×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308682) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308682)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308682)
