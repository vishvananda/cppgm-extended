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
| 57 | `libs/leaf/test` | missing `nlohmann/json.hpp` | in progress | Homebrew nlohmann-json is now installed at `/usr/local/opt/nlohmann-json/include`; focused positive target and full exact graph pending. |
| 51 | `libs/graph_parallel/test` | missing MPI, runner, and Python target | pending | Open MPI 5.0.9 and Python 3.14 are now installed; exact graph pending after LEAF. |
| 68 | `libs/mpi/test` | missing MPI compiler and runner | pending | `/usr/local/bin/mpic++` and `/usr/local/bin/mpirun` are now available; exact graph pending after Graph Parallel. |

## Frontier Log

- `2026-08-03`: Created the independent reopened-suite lane after rebasing the
  intake branch onto the finalized Boost.JSON frontier and passing the full
  direct-LowIR report `4734/4734`. The canonical V2 tracker remains untouched.

