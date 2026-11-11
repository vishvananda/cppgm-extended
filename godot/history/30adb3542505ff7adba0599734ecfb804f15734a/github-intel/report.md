# Godot headless benchmarks

Compiler: `30adb3542505ff7adba0599734ecfb804f15734a` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1351.41 | 345.69 |
| gcc | 5423.51 | 1364.21 |
| clang | 4508.82 | 1148.49 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.968× | 0.888× | 0.973× | 0.874× |
| arrays | 0.983× | 1.033× | 0.991× | 1.052× |
| strings | 0.871× | 0.863× | 0.860× | 0.851× |
| geometry | 0.952× | 0.905× | 0.941× | 0.912× |
| scene | 0.907× | 0.899× | 0.908× | 0.890× |

A/A variation above 2%: strings gcc measured_wall after: 1.024×; scene gcc cpu after: 1.052×; scene gcc measured_wall before: 0.974×; scene gcc measured_wall after: 1.049×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958648) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958648)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958648)
