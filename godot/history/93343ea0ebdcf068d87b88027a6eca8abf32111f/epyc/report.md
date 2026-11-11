# Godot headless benchmarks

Compiler: `93343ea0ebdcf068d87b88027a6eca8abf32111f` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1082.79 | 279.74 |
| gcc | 4159.18 | 1048.71 |
| clang | 3711.09 | 946.06 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.915× | 0.860× | 0.912× | 0.847× |
| arrays | 0.800× | 0.880× | 0.786× | 0.873× |
| strings | 0.856× | 0.816× | 0.833× | 0.806× |
| geometry | 0.894× | 0.879× | 0.889× | 0.871× |
| scene | 0.952× | 0.882× | 0.955× | 0.875× |

A/A variation above 2%: arrays gcc cpu after: 0.975×; arrays gcc measured_wall before: 0.976×; arrays gcc measured_wall after: 0.975×; strings gcc measured_wall after: 0.972×; scene clang cpu after: 1.026×; scene clang measured_wall after: 1.035×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904445) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904445)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904445)
