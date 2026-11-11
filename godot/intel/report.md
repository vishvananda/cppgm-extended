# Godot headless benchmarks

Compiler: `9ca649bf59cb1e14979655d1669ade353a9ae881` · CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1402.67 | 107.21 |
| gcc | 5358.12 | 395.77 |
| clang | 4658.29 | 366.56 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.978× | 0.935× | 0.937× | 0.853× |
| arrays | 1.039× | 1.287× | 1.083× | 1.323× |
| strings | 0.853× | 0.858× | 0.827× | 0.844× |
| geometry | 0.977× | 0.992× | 0.997× | 0.943× |
| scene | 0.910× | 0.891× | 0.926× | 0.887× |

A/A variation above 2%: dispatch gcc cpu after: 1.031×; dispatch gcc measured_wall before: 1.032×; dispatch gcc measured_wall after: 1.072×; dispatch clang cpu after: 0.964×; dispatch clang measured_wall before: 1.070×; dispatch clang measured_wall after: 0.884×; arrays gcc cpu after: 0.772×; arrays gcc measured_wall after: 0.735×; arrays clang cpu before: 0.678×; arrays clang cpu after: 1.162×; arrays clang measured_wall before: 0.663×; arrays clang measured_wall after: 1.216×; strings gcc cpu before: 1.072×; strings gcc measured_wall before: 1.031×; strings clang measured_wall before: 1.029×; geometry gcc cpu after: 1.024×; geometry gcc measured_wall before: 0.964×; geometry gcc measured_wall after: 0.958×; geometry clang measured_wall before: 1.054×; scene gcc measured_wall before: 1.023×; scene gcc measured_wall after: 1.056×; scene clang cpu before: 0.969×; scene clang measured_wall before: 1.025×; scene clang measured_wall after: 1.032×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42713606884) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42713606884)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42713606884)
