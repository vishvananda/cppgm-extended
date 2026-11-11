# Godot headless benchmarks

Compiler: `6c589867bdd5c99bc35b90267a46778f59e1127e` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1484.94 | 380.39 |
| gcc | 5644.54 | 1422.18 |
| clang | 5695.12 | 1447.40 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.025× | 0.904× | 1.034× | 0.889× |
| arrays | 0.829× | 0.925× | 0.801× | 0.927× |
| strings | 0.838× | 0.840× | 0.810× | 0.827× |
| geometry | 1.034× | 0.856× | 1.052× | 0.851× |
| scene | 0.933× | 0.940× | 0.931× | 0.952× |

A/A variation above 2%: dispatch gcc cpu before: 1.043×; dispatch gcc measured_wall before: 1.043×; dispatch clang cpu before: 0.971×; strings gcc cpu after: 1.031×; scene gcc cpu after: 1.024×; scene gcc measured_wall after: 1.024×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013143) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013143)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013143)
