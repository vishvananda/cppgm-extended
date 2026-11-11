# Godot headless benchmarks

Compiler: `da7046f6f5518cdabc9d2d0f2cd153e228c7b1bb` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1079.06 | 278.70 |
| gcc | 4193.76 | 1060.55 |
| clang | 3737.76 | 957.97 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.915× | 0.883× | 0.910× | 0.870× |
| arrays | 0.819× | 0.904× | 0.822× | 0.913× |
| strings | 0.839× | 0.824× | 0.820× | 0.820× |
| geometry | 0.892× | 0.866× | 0.873× | 0.859× |
| scene | 0.967× | 0.916× | 0.968× | 0.911× |

A/A variation above 2%: dispatch clang measured_wall before: 0.968×; arrays gcc cpu before: 0.976×; arrays gcc cpu after: 1.025×; arrays gcc measured_wall before: 0.966×; arrays gcc measured_wall after: 1.035×; geometry clang cpu after: 1.103×; geometry clang measured_wall after: 1.069×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893621) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893621)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893621)
