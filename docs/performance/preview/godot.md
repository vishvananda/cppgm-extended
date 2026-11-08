# Godot headless build and runtime

Measured compiler: `b866774e011bf6be996f089185079adbbde87b19` · AMD EPYC 9V45 96-Core Processor · GitHub hosted, Ubuntu 26.04 container.

Unmodified Godot 4.7.2 (`ed1daf0bf001b61586d9930840f2f1394092c079`), full headless release, GNU C++17 `-O3`, SSE4.2 and POPCNT. All default modules, 3D, physics and computed GDScript dispatch remain enabled. C compilation and final linking use GCC for all builds.

All **five workloads pass for each compiler**, using independent Python checksums. They cover dispatch, arrays, strings, vector geometry and scene transforms. These are headless engine workloads, not rendering or frame-rate measurements.

[Shared summary](godot-v1/summary.md) · [Per-workload CPU and elapsed tables](godot-v1/godot.md) · [Version-1 records and provenance](godot-v1/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42496249277) · [Original artifact](https://github.com/vishvananda/cppgm-extended/actions/runs/42496249277/artifacts/13624916058). Original CI artifacts have a retention limit; main's publisher retains future observations on `benchmark-badges`.

Full-build CPU/wall times are one clean SCons build per compiler, including linking. Runtime speedups are median paired ABBA/BAAB block ratios, with A/A calibration before and after. The summary weights all five workloads equally. Process CPU includes startup; work elapsed excludes startup. Absolute runtime times are observation medians and need not reproduce paired ratios.

| Compiler | Build CPU seconds | Build wall seconds |
|---|---:|---:|
| cppgm | 872.23 | 225.32 |
| gcc | 3279.84 | 827.01 |
| clang | 2604.17 | 666.91 |

Runtime ratios are **reference time / cppgm time: higher is better**. Work wall time excludes engine startup; process CPU time includes it. Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.

| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |
|---|---:|---:|---:|---:|
| dispatch | 0.807× | 0.678× | 0.776× | 0.652× |
| arrays | 0.855× | 0.833× | 0.854× | 0.819× |
| strings | 0.844× | 0.856× | 0.831× | 0.842× |
| geometry | 0.862× | 0.827× | 0.853× | 0.826× |
| scene | 0.921× | 0.845× | 0.910× | 0.830× |

A/A variation above 2%: dispatch gcc cpu after: 1.050×; dispatch gcc measured_wall after: 1.056×; dispatch clang cpu before: 0.970×; dispatch clang cpu after: 0.933×; dispatch clang measured_wall before: 0.958×; dispatch clang measured_wall after: 0.923×; arrays gcc cpu before: 0.943×; arrays gcc measured_wall before: 0.939×; arrays clang cpu before: 1.047×; arrays clang measured_wall before: 1.049×; strings gcc measured_wall after: 0.970×; strings clang cpu before: 0.977×; strings clang measured_wall before: 0.977×; geometry gcc measured_wall after: 1.020×; scene gcc cpu before: 0.958×; scene clang measured_wall before: 1.024×; scene clang measured_wall after: 1.020×. Every observation is retained.


Exact toolchain versions, compiler/input/binary hashes, build commands, case membership and calibration blocks are retained in the versioned records. Historical OS/library details that were not recorded remain explicitly unknown.
