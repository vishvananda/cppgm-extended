# Godot headless benchmarks

Compiler: `238923cec31b1e64421dd46e8a3fecfe57d9fddb` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1065.41 | 274.42 |
| gcc | 4100.84 | 1033.70 |
| clang | 3708.77 | 960.93 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.902× | 0.871× | 0.890× | 0.866× |
| arrays | 0.815× | 0.876× | 0.789× | 0.872× |
| strings | 0.892× | 0.877× | 0.880× | 0.867× |
| geometry | 0.901× | 0.879× | 0.882× | 0.876× |
| scene | 0.983× | 0.949× | 0.985× | 0.946× |

A/A variation above 2%: dispatch gcc cpu after: 0.975×; dispatch clang measured_wall after: 1.027×; arrays gcc cpu after: 0.975×; arrays gcc measured_wall before: 0.968×; arrays gcc measured_wall after: 0.964×; strings clang cpu before: 0.968×; strings clang measured_wall before: 0.967×; scene gcc cpu after: 0.966×; scene gcc measured_wall after: 0.979×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778967) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778967)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778967)
