# Godot headless benchmarks

Compiler: `b7e1f8d5fa08e26637f4de8e241bb27cace292af` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1074.72 | 277.14 |
| gcc | 4117.35 | 1038.36 |
| clang | 3700.96 | 944.03 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.895× | 0.848× | 0.887× | 0.836× |
| arrays | 0.770× | 0.861× | 0.742× | 0.856× |
| strings | 0.868× | 0.847× | 0.856× | 0.840× |
| geometry | 0.893× | 0.885× | 0.889× | 0.878× |
| scene | 0.937× | 0.911× | 0.946× | 0.906× |

A/A variation above 2%: dispatch gcc cpu before: 1.085×; dispatch gcc measured_wall before: 1.090×; arrays gcc cpu before: 1.025×; arrays gcc cpu after: 0.935×; arrays gcc measured_wall before: 1.033×; arrays gcc measured_wall after: 0.919×; arrays clang cpu before: 0.935×; arrays clang measured_wall before: 0.933×; strings gcc cpu after: 0.976×; strings gcc measured_wall after: 0.974×; strings clang cpu before: 1.025×; strings clang measured_wall before: 1.026×; geometry gcc cpu after: 1.068×; geometry gcc measured_wall after: 1.068×; scene gcc cpu before: 0.975×; scene clang cpu after: 1.027×; scene clang measured_wall after: 1.023×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497650) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497650)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497650)
