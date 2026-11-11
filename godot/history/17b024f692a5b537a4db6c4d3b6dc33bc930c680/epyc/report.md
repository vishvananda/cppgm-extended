# Godot headless benchmarks

Compiler: `17b024f692a5b537a4db6c4d3b6dc33bc930c680` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1455.44 | 373.13 |
| gcc | 5610.49 | 1413.66 |
| clang | 5525.41 | 1399.15 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.913× | 0.816× | 0.903× | 0.790× |
| arrays | 0.724× | 0.834× | 0.699× | 0.825× |
| strings | 0.821× | 0.808× | 0.806× | 0.789× |
| geometry | 1.235× | 0.907× | 1.319× | 0.896× |
| scene | 0.926× | 0.811× | 0.939× | 0.781× |

A/A variation above 2%: dispatch gcc cpu before: 0.895×; dispatch gcc measured_wall before: 0.884×; strings clang cpu after: 0.969×; strings clang measured_wall after: 0.980×; scene clang cpu before: 1.037×; scene clang cpu after: 1.063×; scene clang measured_wall before: 1.038×; scene clang measured_wall after: 1.040×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011709) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011709)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011709)
