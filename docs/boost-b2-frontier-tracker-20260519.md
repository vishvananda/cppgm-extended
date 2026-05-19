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
  `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json`
- rebased stack perf check: passed, instructions `-14.88%`, RSS `-2.77%`,
  footprint `-5.72%`
- rebased-head perf baseline: recorded at branch head
  `6735bc6cf22ab9de04de945a25cf6583235acb92`, compiler-equivalent to the
  rebased frontier head; median instructions `312,198,629,210`, RSS `1.10 GiB`,
  footprint `856.84 MiB`

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
| 1 | `libs/accumulators/test` | pass | `JOBS=12`, 9s, log `/tmp/boost-frontier-seq-20260519-073659-j12-6735bc6cf/libs__accumulators__test.log`. |
| 2 | `libs/algorithm/test` | next frontier | Re-run after const-reference-alias fix advanced past Boost.Test/Regex dependency builds. Remaining failures: `partition_subrange_test.o`, `sort_subrange_test.o`. |

## Fix Ledger

| Status | Frontier seed | Root cause | Owner PA | Regression | Pre-fix result | Validation | Fix commit |
|---|---|---|---|---|---|---|---|
| committed | `libs/algorithm/test` dependency build via Boost.Test/Boost.Regex; libc++ `std::__1::vector` iterator conversion needed `__wrap_iter<T*>` to `__wrap_iter<T const*>` | Structural alias-template expansion handled a dependent alias before preserving an outer `TK_CV`, so `const remove_reference_t<T> &` with `T=item &` collapsed to `item &` during defaulted SFINAE checks. | `pa22:500` | `pa22/tests/general/500-const-reference-alias-default-sfinae.t` | Non-STL reducer `/tmp/cppgm-boost-reducers-20260519/alias_const_lvalue_ref_default_arg.cpp` failed with explicit type argument fallback / SFINAE rejection; STL reducer `/tmp/cppgm-boost-reducers-20260519/vector_const_iterator.cpp` failed to compile `vector::erase(v.begin())` / `insert(end(), begin(), end())`. | Reducers pass; focused LowIR ref matches; placement audit OK; `ACTIVE_TEST_REPORT_PAS='pa22' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `407/407`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa22` passes; `JOBS=12 ./run-cppgm-b2.sh -a libs/algorithm/test` advances to subrange failures; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.14%` instructions). | `091da7fe6` |
