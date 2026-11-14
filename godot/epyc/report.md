# Godot headless benchmarks

Compiler: `1597e573a4882a000810eb15f86d4cb230ce4afd` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1028.60 | 265.79 |
| gcc | 4070.34 | 1029.58 |
| clang | 3714.13 | 949.78 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.044× | 0.982× | 1.054× | 0.985× |
| arrays | 0.897× | 1.024× | 0.880× | 1.026× |
| strings | 0.928× | 0.913× | 0.920× | 0.910× |
| geometry | 1.000× | 1.009× | 1.025× | 1.015× |
| scene | 1.034× | 0.966× | 1.059× | 0.967× |

A/A variation above 2%: dispatch gcc measured_wall after: 1.029×; dispatch clang cpu before: 0.774×; dispatch clang measured_wall before: 0.738×; dispatch clang measured_wall after: 0.971×; arrays gcc cpu after: 0.949×; arrays gcc measured_wall after: 0.950×; geometry clang cpu before: 1.026×; scene gcc measured_wall before: 1.021×; scene clang measured_wall after: 1.024×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026850)
