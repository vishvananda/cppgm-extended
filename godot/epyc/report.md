# Godot headless benchmarks

Compiler: `b81455bc7d42635d12519c11c3788a20b5fad2f4` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1057.04 | 273.26 |
| gcc | 4257.42 | 1076.81 |
| clang | 3789.94 | 972.55 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.984× | 0.917× | 0.997× | 0.918× |
| arrays | 0.870× | 0.951× | 0.861× | 0.952× |
| strings | 0.886× | 0.886× | 0.879× | 0.877× |
| geometry | 0.960× | 0.944× | 0.964× | 0.944× |
| scene | 1.009× | 0.991× | 1.015× | 0.996× |

A/A variation above 2%: dispatch clang cpu before: 1.136×; dispatch clang cpu after: 0.704×; dispatch clang measured_wall before: 1.147×; dispatch clang measured_wall after: 0.670×; arrays gcc cpu after: 0.973×; arrays gcc measured_wall after: 0.965×; arrays clang cpu before: 1.035×; arrays clang measured_wall before: 1.020×; strings gcc cpu after: 0.959×; strings gcc measured_wall after: 0.963×; geometry gcc cpu before: 1.025×; scene gcc cpu before: 0.913×; scene gcc cpu after: 1.034×; scene gcc measured_wall before: 0.906×; scene gcc measured_wall after: 1.043×; scene clang cpu before: 0.966×; scene clang cpu after: 0.974×; scene clang measured_wall before: 0.968×; scene clang measured_wall after: 0.979×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409678)
