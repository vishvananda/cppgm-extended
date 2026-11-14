# Godot headless benchmarks

Compiler: `01bac61953a982df28933dfde0fa03eb65838ea4` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1377.05 | 354.11 |
| gcc | 5520.36 | 1396.02 |
| clang | 5488.64 | 1393.74 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.128× | 1.037× | 1.174× | 1.053× |
| arrays | 0.896× | 1.015× | 0.874× | 1.021× |
| strings | 0.863× | 0.862× | 0.847× | 0.851× |
| geometry | 1.130× | 0.817× | 1.151× | 0.797× |
| scene | 1.078× | 1.043× | 1.118× | 1.073× |

A/A variation above 2%: dispatch gcc cpu after: 1.024×; dispatch gcc measured_wall after: 1.030×; dispatch clang cpu after: 1.087×; dispatch clang measured_wall before: 1.026×; dispatch clang measured_wall after: 1.100×; strings gcc cpu before: 0.976×; geometry clang cpu after: 0.978×; geometry clang measured_wall after: 0.956×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814680)
