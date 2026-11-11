# Godot headless benchmarks

Compiler: `8287f480c7b5bdb2074c8941689aa36f9c233bd7` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 911.47 | 235.23 |
| gcc | 3311.12 | 835.28 |
| clang | 2671.06 | 685.76 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.955× | 0.830× | 0.958× | 0.803× |
| arrays | 0.919× | 0.885× | 0.911× | 0.871× |
| strings | 0.869× | 0.904× | 0.857× | 0.894× |
| geometry | 1.000× | 0.924× | 0.995× | 0.927× |
| scene | 0.950× | 0.875× | 0.950× | 0.867× |

A/A variation above 2%: dispatch gcc cpu after: 0.947×; dispatch gcc measured_wall after: 0.935×; dispatch clang cpu after: 1.022×; dispatch clang measured_wall after: 1.026×; arrays gcc cpu before: 1.026×; arrays gcc measured_wall before: 1.027×; strings gcc cpu after: 1.114×; strings gcc measured_wall after: 1.064×; strings clang cpu after: 0.959×; geometry gcc cpu before: 1.061×; geometry gcc measured_wall before: 1.062×; geometry gcc measured_wall after: 1.023×; scene clang cpu before: 0.954×; scene clang cpu after: 0.837×; scene clang measured_wall before: 0.957×; scene clang measured_wall after: 0.829×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798314) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798314)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798314)
