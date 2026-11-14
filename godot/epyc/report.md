# Godot headless benchmarks

Compiler: `9ed2f1eea641727dd56efede0b4ca09c045e9f7f` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1029.22 | 266.19 |
| gcc | 4085.93 | 1033.49 |
| clang | 3705.19 | 949.36 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.043× | 0.983× | 1.056× | 0.983× |
| arrays | 0.908× | 0.988× | 0.899× | 1.008× |
| strings | 0.904× | 0.896× | 0.897× | 0.904× |
| geometry | 1.018× | 1.000× | 1.026× | 1.016× |
| scene | 1.026× | 0.966× | 1.031× | 0.972× |

A/A variation above 2%: dispatch gcc cpu before: 1.139×; dispatch gcc measured_wall before: 1.162×; arrays gcc cpu after: 1.039×; arrays gcc measured_wall after: 1.065×; strings clang measured_wall after: 0.979×; geometry gcc cpu after: 0.974×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267279)
