# Godot headless benchmarks

Compiler: `043e34737771945b5b28325856dadffa564bb383` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1057.95 | 272.88 |
| gcc | 4071.44 | 1030.09 |
| clang | 3831.40 | 980.69 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.915× | 0.846× | 0.904× | 0.833× |
| arrays | 0.827× | 0.887× | 0.820× | 0.886× |
| strings | 0.829× | 0.829× | 0.810× | 0.808× |
| geometry | 0.886× | 0.886× | 0.876× | 0.872× |
| scene | 0.952× | 0.935× | 0.953× | 0.946× |

A/A variation above 2%: dispatch gcc cpu after: 1.050×; dispatch gcc measured_wall after: 1.055×; arrays gcc cpu before: 0.938×; arrays gcc cpu after: 0.969×; arrays gcc measured_wall before: 0.931×; strings gcc cpu after: 1.026×; scene gcc cpu after: 0.960×; scene gcc measured_wall after: 0.956×; scene clang cpu after: 1.026×; scene clang measured_wall after: 1.039×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386121) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386121)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386121)
