# Boost B2 Frontier V2 Tracker

This is the live tracker for `docs/boost-b2-frontier-v2-plan.md`. It starts from
zero credited Boost suites. V1 pass/fail state is historical only.

## Start State

- started: `2026-07-13`
- compiler baseline: `db9879223b4249a9e5516de0205e76fa158d5549`
- base branch: `main`
- frontier branch: `boost-frontier-v2`
- Boost tree: `/Users/vishvananda/boost_1_91_0`
- Boost release: `1.91.0`
- suite inventory: `docs/boost-b2-suite-status-20260511.md`
- suite count: `147`
- completed suites: `0 / 147`
- current cursor: `#1 libs/accumulators/test`
- active compiler frontier: none; baseline bootstrap is not complete

## Baseline Gates

| Gate | Status | Evidence |
|---|---|---|
| Exact compiler start commit | pass | `main`, `origin/main`, and the merged PR head were verified at `db9879223` before this branch was created. |
| PR Tests workflow | pass | GitHub Actions run `29298098410` completed successfully with all 21 checks green across GCC/libstdc++ and Clang/libc++ on Ubuntu 24.04 and 26.04. |
| PR-triggered inception | pending | Run `29299551532` was still in `Compare cppgm++ inception` when V2 planning started. Do not credit suite 1 until this is successful. |
| Warning-clean `dev/cppgm++` build | pass | Last build at the baseline head completed without warnings. Rerun before suite 1. |
| Full direct-LowIR report | pending refresh | PR CI `test-report` passed in all four lanes. Record the V2 local baseline command before suite 1. |
| Strict direct-LowIR report | pass, pending refresh | PR CI `test-strict` passed in all four lanes. Record the V2 local baseline command before suite 1. |
| Strict text-reparse audit | pass | All 23 categories were zero; 14 audit unit tests passed at the baseline head. Rerun before suite 1. |
| PA placement audit | pass | PR CI placement audit passed. |
| Fixed V2 performance baseline | pending | Record `/tmp/cppgm-boost-frontier-v2-db9879223-baseline.json` with three runs at the exact baseline commit. |

## Environment

Fill this before the first suite run. Keep these values stable for comparable
suite evidence and performance measurements.

| Item | Value |
|---|---|
| host OS/version | pending |
| CPU | pending |
| host C compiler | `/usr/local/opt/llvm/bin/clang` pending version capture |
| host C++ compiler | `/usr/local/opt/llvm/bin/clang++` pending version capture |
| C++ standard library | libc++ pending version capture |
| B2 wrapper | `/Users/vishvananda/boost_1_91_0/run-cppgm-b2.sh` |
| compiler under test | `/Users/vishvananda/cppgm-extended/dev/cppgm++` |
| default suite jobs | `8` |
| default suite timeout | `1800s`, adjusted upward for known large suites |

## Fixed Performance Baseline

| Commit | Baseline file | Instructions | Max RSS | Peak footprint | Wall time | Status |
|---|---|---:|---:|---:|---:|---|
| `db9879223` | `/tmp/cppgm-boost-frontier-v2-db9879223-baseline.json` | pending | pending | pending | pending | not recorded |

## Performance Ledger

Every production compiler commit gets a row against the fixed baseline. Add a
rolling delta only when it helps isolate the incremental cost.

| Commit | Frontier | Fixed instruction delta | Max RSS delta | Footprint delta | Rolling delta | Report | Decision |
|---|---|---:|---:|---:|---:|---|---|

## Suite Cursor

The inventory order comes from `docs/boost-b2-suite-status-20260511.md`. Add a
row when a suite is attempted. Do not prepopulate passes from V1.

| # | Suite | V2 status | Commit | Forced run evidence | Notes |
|---:|---|---|---|---|---|
| 1 | `libs/accumulators/test` | pending | `db9879223` | not run | First V2 suite. V2 does not carry forward the old pass state. |

Allowed statuses are `pending`, `running`, `frontier`, `blocked-external`, and
`pass`. A timeout is evidence, not a pass.

## Active Frontier

- suite: `#1 libs/accumulators/test`
- focused target: pending first forced suite run
- failure phase: pending
- pre-fix diagnostic: pending
- reduced repro: pending
- owning PA/cluster: pending
- implementation area: pending
- performance risk: pending
- next action: complete baseline gates, then run suite 1 with a forced rebuild

## Fix Ledger

Add one row per coherent compiler fix. Keep detailed logs in `/tmp`; keep the
stable command, diagnostic, reducer, validation, and measured deltas here.

| Status | Suite/target | Root cause and typed fix | Owner regression | Pre-fix evidence | Validation | Perf vs fixed baseline | Commit |
|---|---|---|---|---|---|---|---|

## Decision Log

- `2026-07-13`: Started V2 at `db9879223` because removal of semantic text
  reparsing and the following template-resolution fixes may have changed suites
  previously marked passing.
- `2026-07-13`: Reset suite credit to zero. The V1 tracker remains historical
  evidence and is not a V2 cursor.
- `2026-07-13`: Chose `libs/accumulators/test`, inventory row 1, as the first
  suite.
- `2026-07-13`: Made the start-commit performance baseline immutable. Rolling
  baselines may characterize a fix but cannot replace the fixed cumulative
  gate.

## Next Commands

```sh
make -C dev -j8 cppgm++

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false \
  make test-report

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
  make test-strict

python3 scripts/audit_text_reparse.py --strict --list-sites
python3 -m unittest scripts.tests.test_audit_text_reparse

scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-boost-frontier-v2-db9879223-baseline.json \
  --runs 3

env CPPGM_BOOST_B2_FRONTIER=1 \
  CPPGM_B2_CXX=/Users/vishvananda/cppgm-extended/dev/cppgm++ \
  CPPGM_B2_HOST_CC=/usr/local/opt/llvm/bin/clang \
  CPPGM_B2_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  python3 scripts/run_boost_b2_suite_survey.py \
  --suite 1 \
  --jobs 8 \
  --timeout 1800 \
  --output-dir /tmp/boost-frontier-v2-suite-001-db9879223
```
