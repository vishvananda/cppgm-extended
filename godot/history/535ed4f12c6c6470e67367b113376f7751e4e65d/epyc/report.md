# Godot headless benchmarks

Compiler: `535ed4f12c6c6470e67367b113376f7751e4e65d` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1065.17 | 274.89 |
| gcc | 4129.88 | 1044.48 |
| clang | 3716.47 | 951.72 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.910× | 0.836× | 0.901× | 0.827× |
| arrays | 0.776× | 0.863× | 0.760× | 0.852× |
| strings | 0.886× | 0.888× | 0.888× | 0.875× |
| geometry | 0.895× | 0.866× | 0.877× | 0.861× |
| scene | 0.937× | 0.912× | 0.938× | 0.905× |

A/A variation above 2%: dispatch gcc cpu before: 0.930×; dispatch gcc measured_wall before: 0.926×; arrays gcc cpu before: 0.969×; arrays gcc measured_wall before: 0.967×; arrays clang cpu before: 0.913×; arrays clang measured_wall before: 0.908×; strings gcc cpu before: 0.872×; strings gcc measured_wall before: 0.860×; strings clang cpu before: 0.961×; strings clang measured_wall before: 0.962×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136183) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136183)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136183)
