# Boost B2 Frontier V2 Reopened Suites Tracker

This tracker is intentionally separate from
`docs/boost-b2-frontier-v2-tracker.md`. It records the independent validation
of the three V2 suites that were previously blocked by missing host
dependencies, so the results can be merged into the canonical tracker without
interleaving with its active suite cursor.

## Isolation

- branch: `boost-frontier-v2-test-intake`
- starting commit: `3373a8dcc`
- compiler under test:
  `/Users/vishvananda/cppgm-test-intake/dev/cppgm++`
- host compiler: Homebrew Clang 22.1.0 at
  `/usr/local/opt/llvm/bin/clang++`
- Boost tree: `/Users/vishvananda/boost_1_91_0`
- B2 build roots: `/private/tmp/cppgm-boost-reopened-v2/build-*`
- logs: `/private/tmp/cppgm-boost-reopened-v2/logs`
- B2 configuration:
  `/private/tmp/cppgm-boost-reopened-v2-user-config.jam`
- canonical tracker policy: unchanged until these independent rows are ready
  to merge

Every run uses an isolated `--build-dir`, `-a`, `pch=off`, forced C++11,
CPPGM for C++ actions, and explicitly pinned Homebrew Clang paths for host
C/assembly/link actions. The validation and performance ladder follows
`docs/boost-b2-frontier-v2-plan.md`.

## Suite Status

| Suite | Path | Prior blocker | Status | Evidence |
|---:|---|---|---|---|
| 57 | `libs/leaf/test` | missing `nlohmann/json.hpp` | pass | Final forced graph updated 482 targets: 123 tests passed, all 22 negative compilations failed as expected, and there were no skips or unexpected failures. The nlohmann header and positive/negative serialization cases were included. |
| 51 | `libs/graph_parallel/test` | missing MPI, runner, and Python target | pending | Open MPI 5.0.9 and Python 3.14 are now installed; exact graph pending after LEAF. |
| 68 | `libs/mpi/test` | missing MPI compiler and runner | pending | `/usr/local/bin/mpic++` and `/usr/local/bin/mpirun` are now available; exact graph pending after Graph Parallel. |

## Frontier Log

- `2026-08-03`: Created the independent reopened-suite lane after rebasing the
  intake branch onto the finalized Boost.JSON frontier and passing the full
  direct-LowIR report `4734/4734`. The canonical V2 tracker remains untouched.
- `2026-08-03`: Reopened `libs/leaf/test` with the installed Homebrew
  nlohmann-json headers. The initial full graph exposed three independent
  compiler defects: contextual logical-bool probing instantiated a worse
  conversion-template body; two same-spelled function-local polymorphic
  classes shared internal vtable identity; and retained dependent `decltype`
  syntax did not expand a pack nested in a structured template-id. Reduced
  owning regressions landed in PA21, PA17, and PA23 respectively. The related
  placement-audit false positive was narrowed so `__vmi_class_type_info`
  requires source `dynamic_cast` or `typeid` evidence before implying PA27.
- `2026-08-03`: Closed LEAF on the final narrowed compiler. The exact forced
  B2 run (`-a -j2`, C++11, PCH off, static, hidden visibility) updated 482
  targets with 123 passes, 22 failures-as-expected, no skips, no unexpected
  failures, zero swaps, and 648,364,032 B maximum RSS. Repository validation
  passed the direct-LowIR report `4737/4737`; strict comparisons passed PA18
  `263`, PA19 `145`, PA21 `234`, PA22 `290`, and PA23 `369`; all 23 strict
  text-reparse categories remained zero and all 14 audit unit tests passed;
  the placement-audit unit suite passed all 35 tests; and normal, every
  individual cache-disabled mode, and all-disabled mode emitted byte-identical
  LowIR for all three reducers. The frozen three-run performance gate passed at
  -37.01% instructions, -41.60% max RSS, and -42.34% peak footprint
  (`/private/tmp/cppgm-boost-reopened-v2/leaf-final-candidate.json`). PA17,
  PA21, and PA23 already document the tested assignment surfaces, so no README
  expansion was needed. The canonical V2 tracker remains unchanged.
