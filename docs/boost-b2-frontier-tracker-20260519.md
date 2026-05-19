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
- rebased-head perf baseline: recorded at branch head and used as the baseline
  for subsequent Boost frontier fixes
  `6735bc6cf22ab9de04de945a25cf6583235acb92`, compiler-equivalent to the
  rebased frontier head; median instructions `312,198,629,210`, RSS `1.10 GiB`,
  footprint `856.84 MiB`
- active perf baseline: `/tmp/cppgm-perf-baseline-boost-frontier-current-8df3b529-20260519.json`
  recorded at `8df3b52971106d849d19f2f5199ee7699c36717b` after accepting the
  current rebased frontier head as the new baseline; median instructions
  `312,214,640,359`, RSS `1.07 GiB`, footprint `857.01 MiB`

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
| 2 | `libs/algorithm/test` | pass | `JOBS=12`, direct run, updated 218 targets after integral NTTP value-binding conversion fix. Expected negative tests still fail as expected under B2. |
| 3 | `libs/align/test` | pass | `JOBS=12`, direct run, updated 60 targets after Apple hosted compiler macro import. |
| 4 | `libs/any/test` | frontier | `JOBS=12`, direct run after constexpr aggregate-member NTTP fix advances past the no-RTTI `ArrayLength` family. Current suite frontier has 7 failed targets: two `basic_any` static-assert compiles, two `unique_any` initializer-list/emplace constructor compiles, and three runtime failures (`any_test`, `unique_base`, `unique_from_any`). |

## Fix Ledger

| Status | Frontier seed | Root cause | Owner PA | Regression | Pre-fix result | Validation | Fix commit |
|---|---|---|---|---|---|---|---|
| committed | `libs/algorithm/test` dependency build via Boost.Test/Boost.Regex; libc++ `std::__1::vector` iterator conversion needed `__wrap_iter<T*>` to `__wrap_iter<T const*>` | Structural alias-template expansion handled a dependent alias before preserving an outer `TK_CV`, so `const remove_reference_t<T> &` with `T=item &` collapsed to `item &` during defaulted SFINAE checks. | `pa22:500` | `pa22/tests/general/500-const-reference-alias-default-sfinae.t` | Non-STL reducer `/tmp/cppgm-boost-reducers-20260519/alias_const_lvalue_ref_default_arg.cpp` failed with explicit type argument fallback / SFINAE rejection; STL reducer `/tmp/cppgm-boost-reducers-20260519/vector_const_iterator.cpp` failed to compile `vector::erase(v.begin())` / `insert(end(), begin(), end())`. | Reducers pass; focused LowIR ref matches; placement audit OK; `ACTIVE_TEST_REPORT_PAS='pa22' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `407/407`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa22` passes; `JOBS=12 ./run-cppgm-b2.sh -a libs/algorithm/test` advances to subrange failures; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.14%` instructions). | `091da7fe6` |
| committed | `libs/algorithm/test` subrange targets; libc++ `std::default_random_engine` instantiates `__lce_ta<__a, __c, __m, _Mp>` from `linear_congruential_engine<unsigned int, ...>` | Non-type template argument resolution required exact integral type equality for named value bindings. A bound `unsigned int` constant could not be reused as an `unsigned long long` NTTP even when representable, so libc++'s `__lce_ta` helper failed when a value binding such as `__c` was carried semantically rather than as a syntax expression. | `pa19:100` | `pa19/tests/general/100-nontype-integral-value-binding-conversion.t` | Pre-fix compiler rejected `/tmp/cppgm-boost-reducers-20260519/nontype_integral_conversion_value_binding.cpp` with `static_assert unevaluated`; Boost `partition_subrange_test.o` and `sort_subrange_test.o` failed while instantiating `__lce_ta<__a,__c,__m,_Mp>`. | Reducer passes; placement audit OK for `pa19:100`; `ACTIVE_TEST_REPORT_PAS='pa19' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `100/100`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa19` passes with `compared=92`; direct subrange compiles pass; `JOBS=12 ./run-cppgm-b2.sh -a libs/algorithm/test` passes, updating 218 targets; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.53%` instructions). | `e0b2dac73` |
| committed | `libs/align/test` targets including `aligned_alloc_test.o`, `aligned_delete_test.o`, and aligned allocator tests include Apple `TargetConditionals.h` through hosted headers. | Host predefined macro import filtered out `__APPLE_CC__`, so Apple `TargetConditionals.h` saw `__GNUC__` without the Apple compiler-identity macro it requires and took its `unknown compiler` error branch. | `pa34:500` | `pa34/tests/compile/500-host-apple-target-conditionals-macro.t` | Reducer `/tmp/cppgm-boost-reducers-20260519/apple_target_conditionals.cpp` failed with `#error TargetConditionals.h: unknown compiler`; `libs/align/test` had four compile failures with the same diagnostic. | Reducer passes; new test passes via `make -C pa34 check TEST=tests/compile/500-host-apple-target-conditionals-macro.t`; PA34 placement audit row for the new test is owner-reached, with unrelated existing PA34 cluster warnings still present; `ACTIVE_TEST_REPORT_PAS='pa34' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `294/294`; `JOBS=12 ./run-cppgm-b2.sh -a libs/align/test` passes, updating 60 targets; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.37%` instructions). | `03a278f97` |
| committed | `libs/any/test` `basic_tests<Any>::copy_counter::count` in `libs/any/test/basic_test.hpp` | The template declaration collector only recognized out-of-class static member definitions when the immediate qualifier was a template-id and resolved the owner path without the template pattern scope. Instantiation replay and output also assumed direct static member keys and source-template classes, so nested non-template class static storage inside a class template was not replayed or emitted. | `pa21:400` | `pa21/tests/general/400-template-nested-static-member-out-of-class-definition.t` | Reducer `/tmp/cppgm-boost-reducers-20260519/nested_template_static_member_definition.cpp` failed with `failed type template argument resolution: T [scope n::<here>] ... lookup_type [n::outer<T>]`; after the collection path was corrected, the same reducer linked with an undefined `n::outer<int>::inner::value` until nested static definition replay/output was added. | Reducer compile/link/run passes; direct static-member control passes; placement audit OK for `pa21:400`; focused LowIR ref matches; `ACTIVE_TEST_REPORT_PAS='pa21' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `175/175`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa21` passes with `compared=164`; `JOBS=12 ./run-cppgm-b2.sh -a libs/any/test` advances to separate `any_cast` compile failures and `unique_any` runtime failures; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.31%` instructions). | `91c14c0a2` |
| committed | `libs/any/test` `boost::any_cast<std::string>(returning_string1())`; Boost `any` has both `any(const T&)` and a forwarding `any(T&&, enable_if... = 0, enable_if... = 0)` constructor | Constructor-template overload selection compared instantiated defaulted `enable_if` parameters when applying the forwarding-reference tie-breaker. The selected candidate lists had different lengths, so the existing lvalue-reference-over-forwarding-reference preference did not run and `any(const T&)` tied with `any(T&&, ...)` for a `const T&` argument. | `pa22:300` | `pa22/tests/general/300-constructor-template-defaulted-forwarding-lvalue-order.t` | Non-STL reducer `/tmp/cppgm-boost-reducers-20260519/dual_constructor_template_const_lvalue.cpp` failed with `ambiguous constructor [class box]`; Boost probe `/tmp/cppgm-boost-reducers-20260519/boost_any_construct_const_string_ref.cpp` failed the same way before the fix. | Non-STL reducer passes; selected-control reducer returns the `const T&` constructor path under clang; Boost `any` construct and `any_cast` probes pass; placement audit OK for `pa22`; focused LowIR ref matches; `ACTIVE_TEST_REPORT_PAS='pa22' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `408/408`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa22` passes with `compared=381`; direct `libs/any/test/any_test.cpp` advances to separate nested placeholder member-initializer failure; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-rebased-8c9e3ce62-20260519.json` passes (`-0.49%` instructions). | `999535cc4` |
| committed | `libs/any/test` `boost::any::placeholder::clone()` in `boost/any.hpp`; nested `boost::any::placeholder` derives from `boost::anys::detail::placeholder`, which has the same unqualified class name. | Recording a direct base class blindly rebound the base injected-class name into the derived member scope. For a same-name nested derived class, that overwrote the derived injected-class-name, so `placeholder * clone() const` was stored as returning `boost::anys::detail::placeholder *` instead of `boost::any::placeholder *`. | `pa15:200` | `pa15/tests/general/200-nested-injected-class-name-hides-base-name.t` | Non-STL reducer `/tmp/cppgm-boost-reducers-20260519/nested_class_name_shadows_base_in_return.cpp` failed with `invalid member initializer`; `CPPGM_TRACE=lifetime.init` showed `target_type=pointer to struct any::placeholder init_type=pointer to struct lib::placeholder`. | Reducer passes; focused PA15 check passes; placement audit OK for `pa15`; `ACTIVE_TEST_REPORT_PAS='pa15' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `129/129`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict` passes PA18/19/21/22 with no LowIR failures; direct `libs/any/test/any_test.cpp` compiles; `JOBS=12 ./run-cppgm-b2.sh -a libs/any/test` advances to 15 later failed targets; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-current-dee7966f4-20260519.json` passes (`+0.26%` instructions, within tolerance). | `fd3a1803d` |
| committed | `libs/any/test` MPL-if `boost::any_cast<double>(a)` and `boost::anys::any_cast<double>(a)` paths. | Scalar by-value returns of reference-typed expressions emitted the reference address directly. `static_cast<T&>(*result)` lowered to `return f64 %ptr`, leaving native lowering to treat a pointer temp as a floating result and fail with `unknown storage %t7`; integer variants compiled but returned pointer bits. | `pa14:200` | `pa14/tests/general/200-scalar-reference-static-cast-return.t` | Non-STL reducer `/tmp/cppgm-boost-reducers-20260519/return_static_cast_ref_double_notemplate.cpp` failed with `ERROR: unknown storage %t1 in @read_value`; Boost `any_test_mplif.cpp` failed with `ERROR: unknown storage %t7 in @boost__any_cast__ov8`. | Reducer LowIR now loads `f64` through the reference address before return; focused PA14 check passes; placement audit OK for `pa14`; `ACTIVE_TEST_REPORT_PAS='pa14' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `51/51`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict-pa22` passes with `compared=381`; direct `any_test_mplif.cpp` and `basic_any_test_mplif.cpp` compiles pass; `JOBS=12 ./run-cppgm-b2.sh -a libs/any/test` advances to 13 failed targets; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-current-dee7966f4-20260519.json` passes (`+0.09%` instructions). | this commit |
| committed | `libs/any/test` no-RTTI `boost::typeindex::detail::after_substrig<ArrayLength - detail::skip().size_at_begin>` path. | The constexpr evaluator treated `T{...}` as a call with a braced argument before attempting typed initialization for class aggregates. A constexpr function returning an aggregate with `return skip_info{1};` therefore could not feed `skip().size_at_begin` into an integral constant expression or explicit non-type template argument. | `pa20:400` | `pa20/tests/general/400-constexpr-function-aggregate-member-nttp.t` | Reducer `/tmp/cppgm-boost-reducers-20260519/constexpr_function_aggregate_member.cpp` failed with `unsupported constexpr variable initializer`; reducer `/tmp/cppgm-boost-reducers-20260519/nontype_param_member_constexpr_template_arg.cpp` failed resolving `after<N-skip().size_at_begin>`; Boost no-RTTI `any_test.cpp` failed in the `ArrayLength`/`after_substrig` family. | Reducers pass; direct Boost no-RTTI `any_test.cpp` compile passes; placement audit OK for `pa20`; focused PA20 check passes; `ACTIVE_TEST_REPORT_PAS='pa20' ORDERED=false TEST_REPORT_SUBTEST_JOBS=12 make test-report` passes `69/69`; `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict` passes; `JOBS=12 ./run-cppgm-b2.sh -a libs/any/test` advances to 7 failed targets; perf check against `/tmp/cppgm-perf-baseline-boost-frontier-current-8df3b529-20260519.json` passes (`-0.48%` instructions, RSS `+2.99%`). | `eec0e35a8` |
