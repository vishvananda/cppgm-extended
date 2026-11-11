# Godot headless benchmarks

Compiler: `a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1057.07 | 273.03 |
| gcc | 4074.49 | 1030.51 |
| clang | 3671.45 | 937.39 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.930× | 0.855× | 0.920× | 0.845× |
| arrays | 0.770× | 0.889× | 0.749× | 0.880× |
| strings | 0.881× | 0.846× | 0.859× | 0.834× |
| geometry | 0.895× | 0.902× | 0.885× | 0.884× |
| scene | 0.935× | 0.893× | 0.925× | 0.874× |

A/A variation above 2%: arrays gcc cpu before: 0.976×; arrays gcc cpu after: 1.033×; arrays gcc measured_wall before: 0.977×; arrays gcc measured_wall after: 1.026×; strings gcc measured_wall after: 0.975×; strings clang measured_wall after: 1.020×; scene clang measured_wall before: 1.032×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658259) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658259)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658259)
