# Godot headless benchmarks

Compiler: `e2831a513b668c602faa8098e55eb22ee08a670f` · CPU: Intel(R) Xeon(R) 6973P-C

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 848.38 | 218.23 |
| gcc | 3525.49 | 889.28 |
| clang | 2789.56 | 711.33 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.068× | 0.962× | 1.070× | 0.985× |
| arrays | 0.967× | 0.992× | 0.967× | 0.998× |
| strings | 0.909× | 0.902× | 0.892× | 0.899× |
| geometry | 0.992× | 0.850× | 0.990× | 0.844× |
| scene | 0.885× | 0.886× | 0.875× | 0.885× |

A/A variation above 2%: arrays gcc cpu before: 1.059×; arrays gcc measured_wall before: 1.054×; strings gcc cpu after: 1.123×; strings gcc measured_wall after: 1.121×; scene gcc cpu after: 1.028×; scene gcc measured_wall after: 1.025×; scene clang cpu after: 1.065×; scene clang measured_wall after: 1.088×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898265)
