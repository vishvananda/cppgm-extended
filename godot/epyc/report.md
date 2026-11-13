# Godot headless benchmarks

Compiler: `72d294c916f0f3f43e50e812f7efa91937eb9b66` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1038.61 | 268.64 |
| gcc | 4210.48 | 1069.06 |
| clang | 3855.92 | 990.30 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.992× | 0.926× | 0.991× | 0.923× |
| arrays | 0.861× | 0.932× | 0.848× | 0.936× |
| strings | 0.912× | 0.919× | 0.906× | 0.915× |
| geometry | 0.944× | 0.929× | 0.956× | 0.943× |
| scene | 0.944× | 0.898× | 0.945× | 0.892× |

A/A variation above 2%: dispatch gcc cpu after: 1.025×; dispatch gcc measured_wall after: 1.033×; arrays gcc cpu after: 1.033×; arrays gcc measured_wall before: 1.023×; arrays gcc measured_wall after: 1.056×; strings gcc cpu after: 0.969×; strings gcc measured_wall after: 0.956×; scene gcc measured_wall before: 1.022×; scene gcc measured_wall after: 1.020×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824630)
