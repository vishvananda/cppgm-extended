# Boost B2 Frontier Tracker - 2026-05-19

This tracker starts from `origin/fix-lowir-external-lifecycle-perf` and the
rebased Boost frontier branch.

## Baseline

- base branch: `fix-lowir-external-lifecycle-perf`
- base commit: `529454c61befdf931373f6dc32b81983ec0d7390`
- rebased frontier head at tracker start:
  `8c9e3ce62fc95e6b8b88ebbe854f05fadae3627c`
- safety branch:
  `backup/boost-frontier-before-lowir-lifecycle-rebase-20260519-072414`
- perf baseline:
  `/tmp/cppgm-perf-baseline-fix-lowir-external-lifecycle-perf-529454c61-20260519.json`
- rebased stack perf check: passed, instructions `-14.88%`, RSS `-2.77%`,
  footprint `-5.72%`

Local Boost wrapper state:

- `/Users/vishvananda/boost_1_91_0/cppgm-b2-toolchain.sh` points at this
  worktree's `dev/cppgm++`.
- The wrapper routes C and assembly compile/preprocess actions to host clang,
  C++ compile/preprocess actions to `cppgm++`, and link actions to host clang++.

## Sequential Process

1. Run Boost suites in the order listed in
   `docs/boost-b2-suite-status-20260511.md`.
2. Use one suite at a time with B2 compile parallelism, for example:

   ```sh
   cd /Users/vishvananda/boost_1_91_0
   JOBS=12 ./run-cppgm-b2.sh -a libs/accumulators/test
   ```

3. If the suite passes, mark it in this tracker and continue to the next suite.
4. If it fails, identify the first real compiler frontier, reduce it to the
   smallest useful source, and prefer a non-STL reducer when possible.
5. Place the regression in the earliest owning PA and first correct cluster.
   Use `ROADMAP.md`, PA READMEs, the reducer's essential assertion, and:

   ```sh
   python3 scripts/audit_pa_feature_placement.py --pa paNN \
     --markdown-out /tmp/boost-frontier-placement-paNN.md \
     --json-out /tmp/boost-frontier-placement-paNN.json \
     --fail-on-early
   ```

   Include every touched PA. Course tests for PA1-PA9 belong under
   `cppgm.tests/course/pa*` unless the reducer is a hosted C++ extension, in
   which case use the hosted compatibility PAs.
6. Before implementation changes, confirm the reducer fails on the current
   branch and record the pre-fix diagnostic here.
7. Fix the compiler in `dev/`, validate the focused regression, the owner PA
   report, strict LowIR compare when relevant, and the Boost target.
8. Run the perf gate against the baseline above before committing any compiler
   fix.
9. Commit each coherent fix with the regression, implementation, and tracker
   update before moving to the next unrelated frontier.

## Suite Cursor

| # | Suite | Status | Notes |
|---|---|---|---|
| 1 | `libs/accumulators/test` | pending | Starting point for this pass. |

## Fix Ledger

| Status | Frontier seed | Root cause | Owner PA | Regression | Pre-fix result | Validation | Fix commit |
|---|---|---|---|---|---|---|---|
