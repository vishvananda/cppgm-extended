# Godot headless benchmarks

Compiler: `48e44d6ab2db3dd6d667a9aef1a21fcb36b65f3f` · CPU: Intel(R) Xeon(R) 6973P-C

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 948.50 | 243.98 |
| gcc | 3783.59 | 953.10 |
| clang | 3025.80 | 775.27 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.026× | 0.957× | 1.027× | 0.957× |
| arrays | 0.928× | 0.984× | 0.930× | 0.990× |
| strings | 0.898× | 0.922× | 0.890× | 0.920× |
| geometry | 0.992× | 0.888× | 0.981× | 0.878× |
| scene | 0.927× | 0.860× | 0.922× | 0.862× |

A/A variation above 2%: dispatch gcc cpu after: 0.969×; dispatch gcc measured_wall after: 0.961×; dispatch clang cpu before: 0.959×; dispatch clang cpu after: 1.027×; dispatch clang measured_wall before: 0.961×; dispatch clang measured_wall after: 1.042×; arrays gcc cpu before: 1.061×; arrays gcc measured_wall before: 1.068×; geometry gcc cpu after: 0.958×; geometry gcc measured_wall after: 0.971×; scene gcc cpu before: 1.088×; scene gcc measured_wall before: 1.097×; scene gcc measured_wall after: 1.025×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204700) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204700)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204700)
