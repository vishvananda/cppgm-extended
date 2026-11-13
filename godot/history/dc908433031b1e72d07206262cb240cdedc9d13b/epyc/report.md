# Godot headless benchmarks

Compiler: `dc908433031b1e72d07206262cb240cdedc9d13b` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1050.95 | 271.78 |
| gcc | 4163.82 | 1053.80 |
| clang | 3751.89 | 959.55 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.983× | 0.903× | 0.982× | 0.901× |
| arrays | 0.861× | 0.941× | 0.844× | 0.929× |
| strings | 0.880× | 0.879× | 0.876× | 0.871× |
| geometry | 0.952× | 0.929× | 0.951× | 0.935× |
| scene | 0.929× | 0.904× | 0.938× | 0.896× |

A/A variation above 2%: arrays gcc measured_wall before: 1.032×; scene gcc cpu before: 0.944×; scene gcc measured_wall before: 0.952×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345195)
