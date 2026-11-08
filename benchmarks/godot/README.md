# Godot build and headless performance

[Results by host](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/report.md)
· [CI runs and artifacts](https://github.com/vishvananda/cppgm-extended/actions/workflows/godot.yml)

This reference-compiler lane runs on pushes to main, pull requests and manual
requests. It builds unmodified Godot 4.7.2, pinned to
`ed1daf0bf001b61586d9930840f2f1394092c079`, with cppgm, GCC and Clang. The full
headless release template retains all default engine modules, 3D, physics and
computed GDScript dispatch. Window, graphics and desktop-service dependencies
are disabled equally for all builds. This does not qualify graphical rendering.

All C++ uses the engine's C++17 release settings, including `-O3`, SSE4.2,
POPCNT and disabled exceptions. cppgm additionally selects `--full-asm`.
C files and final linking use GCC for all three builds; the compiler under test
owns C++ compilation. Full-build CPU/wall times include SCons and linking.
Each is one clean build, so small compile-time differences need repeated runs.
Compiler construction and source fetching are outside these measurements.

The checked [GDScript workloads](workload.gd) exercise dispatch, packed arrays,
string formatting, vector geometry and scene transforms. Every run must produce
a checksum calculated independently by Python. These measurements exercise
headless engine execution, rather than rendering or game frame rates. Work wall
time excludes engine startup; process CPU time includes it.

A GCC pilot chooses identical repeats for at least 500 ms of work. Three paired
ABBA/BAAB blocks follow, with A/A calibration before and after each comparison.
Ratios are **reference time / cppgm time: higher is better**. Reports flag A/A
variation above 2% and retain every observation; performance has no pass cutoff.
Runtime errors, bad checksums and failed builds prevent publication.

Successful main runs publish alongside the other benchmark reports, retaining
history by compiler commit and host. EPYC and GitHub Intel results are separate.
PR results appear in the job summary and downloadable artifact. Artifacts retain
commands, compiler/workload hashes, build flags, raw times and calibration.
The `standard/` directory contains [shared version-1 records](../../docs/performance/results-format.md),
a host-specific summary and per-workload tables. Full builds use the `build`
phase; CPU and elapsed time stay separate. The runtime summary is an equal-weight
geometric mean of all five workloads; individual results and A/A warnings remain
in the detail report.

To reproduce on Linux x86-64 with GCC, Clang and SCons 4.10.1 installed:

```sh
make -C dev cppgm++
python3 scripts/godot_ci.py run --output obj/godot-local --runtime-cpu 0
```

Choose an available quiet CPU and a new output directory. `--jobs`, `--gcc`,
`--cc`, `--clang` and `--scons` select build resources and installed tools;
`--source` reuses a clean upstream checkout at the pinned commit.
This lane is independent of the student assignment requirements.
