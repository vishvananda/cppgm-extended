# Godot headless benchmarks

Compiler: `393d1b1d09089ffdade676f4e7cbad8759c15d2a` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 871.85 | 226.04 |
| gcc | 3614.41 | 916.01 |
| clang | 2840.01 | 733.78 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.000× | 0.855× | 1.007× | 0.852× |
| arrays | 1.009× | 0.938× | 1.018× | 0.950× |
| strings | 0.875× | 0.874× | 0.853× | 0.878× |
| geometry | 0.960× | 0.934× | 0.954× | 0.935× |
| scene | 0.881× | 0.813× | 0.872× | 0.807× |

A/A variation above 2%: dispatch clang cpu before: 1.021×; dispatch clang measured_wall before: 1.025×; arrays gcc cpu after: 1.027×; arrays gcc measured_wall after: 1.035×; strings gcc cpu before: 1.090×; strings gcc measured_wall before: 1.096×; strings clang cpu before: 0.957×; strings clang measured_wall before: 0.956×; geometry gcc cpu after: 1.025×; geometry gcc measured_wall before: 0.977×; geometry clang cpu after: 1.027×; geometry clang measured_wall after: 1.028×; scene clang cpu before: 1.074×; scene clang measured_wall before: 1.083×; scene clang measured_wall after: 0.973×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224857)
