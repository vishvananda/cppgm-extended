# Godot headless benchmarks

Compiler: `1ddd7507ee861649d43d1e72dcde7bb17a85a691` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 889.38 | 230.27 |
| gcc | 3402.44 | 861.52 |
| clang | 2578.78 | 660.86 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.863× | 0.742× | 0.859× | 0.728× |
| arrays | 0.865× | 0.830× | 0.863× | 0.811× |
| strings | 0.886× | 0.902× | 0.867× | 0.885× |
| geometry | 0.891× | 0.877× | 0.886× | 0.870× |
| scene | 0.905× | 0.820× | 0.885× | 0.804× |

A/A variation above 2%: dispatch clang cpu before: 0.971×; dispatch clang cpu after: 1.040×; dispatch clang measured_wall before: 0.973×; dispatch clang measured_wall after: 1.034×; arrays gcc cpu before: 0.967×; arrays gcc measured_wall before: 0.959×; geometry gcc measured_wall before: 1.021×; geometry clang measured_wall before: 0.977×; geometry clang measured_wall after: 1.029×; scene gcc cpu before: 1.035×; scene gcc cpu after: 0.948×; scene gcc measured_wall before: 1.050×; scene gcc measured_wall after: 0.948×; scene clang cpu after: 0.972×; scene clang measured_wall after: 0.963×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175077)
