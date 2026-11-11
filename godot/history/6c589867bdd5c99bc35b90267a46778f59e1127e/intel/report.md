# Godot headless benchmarks

Compiler: `6c589867bdd5c99bc35b90267a46778f59e1127e` · CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release template, C++17 `-O3`, all default engine modules, 3D and computed GDScript dispatch retained. C files and final linking use GCC for all three builds.

Full-build times below are one clean build per compiler, including SCons and linking. They are not paired microbenchmark measurements.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 1338.08 | 100.90 |
| gcc | 5309.80 | 382.42 |
| clang | 4616.17 | 363.14 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 1.007× | 0.946× | 1.019× | 0.961× |
| arrays | 0.983× | 1.017× | 1.022× | 1.066× |
| strings | 0.913× | 0.810× | 0.908× | 0.791× |
| geometry | 0.978× | 0.985× | 1.003× | 0.997× |
| scene | 0.939× | 0.890× | 0.921× | 0.883× |

A/A variation above 2%: dispatch gcc cpu before: 0.927×; dispatch gcc cpu after: 0.978×; dispatch gcc measured_wall before: 0.919×; dispatch gcc measured_wall after: 0.975×; dispatch clang cpu before: 1.065×; arrays gcc cpu before: 1.051×; arrays gcc measured_wall before: 1.053×; arrays clang cpu after: 0.973×; arrays clang measured_wall before: 1.020×; strings gcc cpu before: 1.041×; strings gcc measured_wall before: 1.032×; strings clang measured_wall after: 0.980×; geometry clang cpu after: 1.031×; geometry clang measured_wall after: 1.021×; scene gcc cpu before: 0.978×; scene clang measured_wall after: 1.021×. Every observation is retained.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and build flags](metadata.json) · [Full-build observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42696709132) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42696709132)

[Shared comparison tables](standard/summary.md) · [Versioned records](standard/records.json) · [CI run and artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42696709132)
