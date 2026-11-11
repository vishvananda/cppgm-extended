# Godot headless benchmarks

Compiler: `439e2c62e15e7a4d4bf71f07dda6d8e247ed6c9d` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1459.58 | 374.18 |
| gcc | 5586.88 | 1407.64 |
| clang | 5653.73 | 1433.37 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.017× | 1.217× | 1.004× | 1.273× |
| arrays | 0.828× | 0.925× | 0.805× | 0.925× |
| strings | 0.864× | 0.839× | 0.849× | 0.828× |
| geometry | 1.090× | 0.892× | 1.122× | 0.889× |
| scene | 0.901× | 0.900× | 0.892× | 0.895× |

A/A variation above 2%: dispatch gcc cpu after: 1.025×; arrays gcc cpu after: 0.960×; arrays gcc measured_wall after: 0.966×; strings gcc measured_wall before: 1.022×; scene gcc measured_wall before: 1.021×; scene gcc measured_wall after: 1.021×; scene clang cpu before: 1.032×; scene clang cpu after: 1.024×; scene clang measured_wall before: 1.036×; scene clang measured_wall after: 1.027×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872228) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872228)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872228)
