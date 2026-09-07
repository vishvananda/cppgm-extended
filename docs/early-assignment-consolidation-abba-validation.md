# Consolidation: native ABBA performance validation

Status: completed measurement accepted by the user on 2026-09-07. Further
precision measurements were stopped at the user's instruction.

The user requested wall-time ABBA on 2026-09-07, replacing the unavailable
hardware-counter gate for this consolidation. This follows the measurement
discipline in the [O2/O3 plan](implemented/v3/PLAN-O2-O3-OPTIMIZATION.md#measurement-model)
and its [A/A and ABBA protocol](implemented/v3/PLAN-PERF.md#4-measurement-protocol-for-an-intermittently-loaded-host).

## Protocol declared before measurement

- A is commit `70a634a73`; B is the current consolidated source snapshot.
  Both `cppgm++` binaries are freshly built with GCC 15.2.0, C++11,
  `-O3 -fno-strict-aliasing`, the same allocator and enabled batch runner.
  Measurement uses immutable copies, with executable and source hashes saved.
- Wall time is the primary metric. Record user/system CPU, peak RSS, all
  observations, host load and CPU pressure, object hashes and linked output.
- First run two A/A ABBA blocks, then six A/B blocks. Keep all four samples
  once a block starts. Before a block, defer when load1 exceeds 32 or CPU
  pressure `some avg10` exceeds 5%; no result-based removal of outliers.
- The narrow workload is the frozen semantic-overload translation unit at
  requested `-O0`, matching the previous standing performance gate. Verify
  its epoch manifest, source and all 51 frozen headers; do not use live
  compiler headers.
- The broad workload rebuilds the complete baseline `cppgm++` at requested
  `-O1`, with exactly 32 compilation jobs, fixed source/includes/host linker,
  and a fresh identical object-root path for every observation. A and B
  compile the same source set. Retired assignment workloads cannot make B
  appear faster. Validate every object and the linked compiler by hash.
- Use block-paired candidate/baseline ratios and medians. This is a
  no-regression check, not an optimization speedup target: assess the wall
  ratio against the O2/O3 plan's 1% uncertainty allowance, supported by the
  A/A noise and a paired bootstrap interval. Extend an unresolved boundary
  result to twelve blocks instead of selecting favorable runs. Keep the
  existing 3% RSS warning/confirmation policy.

These measurements compare changes to the compiler implementation. They do
not repeat the historical O1/O2/O3 producer-quality matrix or claim an O3
optimization gain.

## Evidence

Work root: `/tmp/cppgm-consolidation-abba-20260907`. The source snapshots,
build logs, host compiler version, candidate source manifest, frozen-workload
validation and raw timing reports are kept there.

The measured host-built producers are:

- A: `2ef55dc241c260c0ebdc665d7ea05fca916c1d82bf43227ec72dc169379847d2`
- B: `317022179bf63084f78be0df9212839b30263235ebdf5aa583f394d60d08ff25`

The frozen source and 51-header closure pass `PERF_EPOCH.json` validation.
The compiler source files still match B's recorded snapshot after measurement,
and the live `dev/cppgm++` has B's exact hash. All 18,244 tracked references
remain unchanged.

| Workload | A wall median | B wall median | Paired wall change | Paired 95% interval | Paired RSS change |
| --- | ---: | ---: | ---: | ---: | ---: |
| Frozen semantic-overload, requested O0 | 24.760 s | 24.725 s | −0.060% | −0.604% to +0.111% | +0.044% |
| Full 234-object compiler, requested O1, initial six blocks | 21.760 s | 21.995 s | +0.629% | −0.647% to +2.434% | −0.244% |
| Full compiler, twelve blocks | 21.875 s | 21.960 s | +0.194% | −0.647% to +1.202% | +0.107% |

The frozen A/A calibration's paired wall difference is −0.140%. Its six-block
A/B result is neutral at that noise level, with the upper confidence bound
well inside the 1% no-regression allowance. All 24 A/B observations produced
the same object bytes; the paired aggregate CPU difference is −0.061%.
No run was removed. Raw reports are `frozen-aa.json`, `frozen-ab.json` and
`summary.json` under the work root.

The initial full-build result is unresolved: its interval exceeds the 1%
allowance. All 24 observations matched the complete 234-object manifest and
linked compiler, and paired aggregate CPU changed by +0.163%. The original
window is preserved as `full-ab-six-blocks.json` and `summary-six-blocks.json`.
The predeclared extension to twelve blocks appends another six blocks to the
same report, retaining every original observation.

At twelve blocks, the median is near neutral but the upper interval still
slightly exceeds the allowance. All 48 observations match the full object
manifest and linked compiler; paired aggregate CPU changes by +0.162%.
Full-build RSS is GNU time's reported maximum for a process or child, not
the aggregate memory footprint of all 32 concurrent jobs. The complete window
is preserved as `full-ab-twelve-blocks.json` and `summary-twelve-blocks.json`.
The paired bootstrap uses 200,000 whole-block resamples with fixed seed
20260907 for the larger windows.

A further twelve-block BAAB precision extension was started. The user then
accepted the twelve-block result as sufficient and instructed proceeding;
the extra experiment was stopped. Its raw observations remain in
`full-ab-abandoned-precision.json` with the interruption and acceptance
recorded in `acceptance.json`. They are outside the accepted comparison.
The main `full-ab.json` and `summary.json` now contain the accepted twelve-block
window; all earlier reports and raw logs remain available.

This closes the consolidation's performance validation by explicit user
acceptance. The full-build interval remains −0.647% to +1.202%; the result does
not establish an upper bound of 1%. No further timing runs are required for
this task.

The convenience script `scripts/run_frozen_compile_benchmark.sh` now defaults
to this repository's checked-in frozen source and header directory. Its old
default named an external checkout affected by assignment renumbering and
used live headers. Default/override argument checks and shell syntax pass;
the measurements above invoke the underlying ABBA harness with pinned inputs
explicitly.
