# Godot headless benchmarks

Compiler: `a649ddb21dd810338ac87d93e7d2e713d0f98ab1` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1382.33 | 355.14 |
| gcc | 5534.72 | 1399.35 |
| clang | 5490.23 | 1397.70 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.126× | 0.961× | 1.163× | 0.975× |
| arrays | 0.894× | 1.008× | 0.883× | 1.017× |
| strings | 0.920× | 0.920× | 0.911× | 0.924× |
| geometry | 1.260× | 0.969× | 1.333× | 0.971× |
| scene | 1.154× | 1.144× | 1.198× | 1.186× |

A/A variation above 2%: dispatch gcc cpu after: 1.035×; dispatch gcc measured_wall after: 1.030×; strings clang cpu before: 1.040×; strings clang measured_wall before: 1.045×; scene gcc cpu after: 0.976×; scene clang cpu before: 0.975×; scene clang measured_wall after: 1.027×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274753)
