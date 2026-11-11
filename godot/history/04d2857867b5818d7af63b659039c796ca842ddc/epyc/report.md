# Godot headless benchmarks

Compiler: `04d2857867b5818d7af63b659039c796ca842ddc` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 855.35 | 221.14 |
| gcc | 3346.54 | 846.44 |
| clang | 2736.34 | 701.10 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.864× | 0.767× | 0.851× | 0.743× |
| arrays | 0.882× | 0.835× | 0.859× | 0.820× |
| strings | 0.877× | 0.884× | 0.852× | 0.875× |
| geometry | 0.890× | 0.887× | 0.884× | 0.872× |
| scene | 0.883× | 0.823× | 0.866× | 0.817× |

A/A variation above 2%: dispatch gcc cpu before: 1.035×; dispatch gcc measured_wall before: 1.035×; dispatch clang cpu after: 1.030×; arrays gcc cpu before: 0.968×; arrays gcc cpu after: 1.022×; arrays gcc measured_wall before: 0.964×; arrays gcc measured_wall after: 1.035×; strings gcc cpu after: 0.960×; geometry gcc cpu before: 0.965×; geometry clang measured_wall after: 1.023×; scene gcc cpu before: 0.976×; scene gcc measured_wall before: 0.972×; scene clang cpu before: 0.958×; scene clang cpu after: 1.026×; scene clang measured_wall before: 0.939×; scene clang measured_wall after: 1.034×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951040163) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951040163)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42951040163)
