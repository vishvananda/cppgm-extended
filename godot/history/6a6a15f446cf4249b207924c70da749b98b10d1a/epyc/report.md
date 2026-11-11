# Godot headless benchmarks

Compiler: `6a6a15f446cf4249b207924c70da749b98b10d1a` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 890.43 | 230.00 |
| gcc | 3486.69 | 879.92 |
| clang | 2900.38 | 745.60 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.803× | 0.715× | 0.785× | 0.697× |
| arrays | 0.885× | 0.831× | 0.868× | 0.819× |
| strings | 0.860× | 0.864× | 0.829× | 0.853× |
| geometry | 0.872× | 0.851× | 0.856× | 0.838× |
| scene | 0.885× | 0.814× | 0.879× | 0.800× |

A/A variation above 2%: dispatch clang cpu after: 0.944×; dispatch clang measured_wall after: 0.950×; arrays gcc cpu before: 0.958×; arrays gcc cpu after: 0.929×; arrays gcc measured_wall before: 0.972×; arrays gcc measured_wall after: 0.924×; arrays clang cpu before: 1.045×; arrays clang measured_wall before: 1.046×; strings gcc cpu before: 1.076×; strings gcc measured_wall before: 1.086×; geometry gcc cpu after: 1.050×; geometry gcc measured_wall after: 1.064×; geometry clang cpu before: 1.079×; geometry clang cpu after: 1.026×; geometry clang measured_wall before: 1.085×; scene gcc cpu before: 1.042×; scene gcc cpu after: 0.966×; scene gcc measured_wall before: 1.039×; scene gcc measured_wall after: 0.941×; scene clang measured_wall before: 1.031×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561989) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561989)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561989)
