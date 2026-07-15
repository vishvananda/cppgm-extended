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
- completed suites: `5 / 147`
- current cursor: `#6 libs/asio/test`
- active compiler frontier: pending the initial forced Asio survey

## Baseline Gates

| Gate | Status | Evidence |
|---|---|---|
| Exact compiler start commit | pass | `main`, `origin/main`, and the merged PR head were verified at `db9879223` before this branch was created. |
| PR Tests workflow | pass | GitHub Actions run `29298098410` completed successfully with all 21 checks green across GCC/libstdc++ and Clang/libc++ on Ubuntu 24.04 and 26.04. |
| PR-triggered inception | pass | Run `29299551532` completed `Compare cppgm++ inception` successfully in `56m53s`. |
| Warning-clean `dev/cppgm++` build | pass | `make -C dev -j8 cppgm++` relinked the compiler at `2acedcec7` without warnings; production compiler sources are unchanged from `db9879223`. |
| Full direct-LowIR report | pass | `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false make test-report` passed `3818 / 3819`; the only miss was the known load-sensitive PA9 generated-program timeout. Its immediate isolated direct-LowIR rerun passed `11 / 11`, and the timeout was accepted as load noise. |
| Strict direct-LowIR report | pass | `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict` passed all configured PA18, PA19, PA21, PA22, and PA23 comparisons. |
| Strict text-reparse audit | pass | `audit_text_reparse.py --strict --list-sites` reported all 23 categories at zero; all 14 audit unit tests passed. |
| PA placement audit | pass | PR CI placement audit passed. |
| Fixed V2 performance baseline | pass | Three runs were recorded from the detached worktree `/tmp/cppgm-boost-v2-baseline-db9879223` at exact commit `db9879223`; medians are recorded below. |

## Environment

Fill this before the first suite run. Keep these values stable for comparable
suite evidence and performance measurements.

| Item | Value |
|---|---|
| host OS/version | macOS 26.3 (25D125), Darwin 25.3.0 x86_64 |
| CPU | Intel Core i7-9750H, 6 physical / 12 logical cores, 16 GiB memory |
| host C compiler | `/usr/local/opt/llvm/bin/clang`, Homebrew clang 22.1.0 |
| host C++ compiler | `/usr/local/opt/llvm/bin/clang++`, Homebrew clang 22.1.0 |
| C++ standard library | system libc++ (`/usr/lib/libc++.1.dylib`, current version 2000.67.0; headers `_LIBCPP_VERSION=220100`) |
| B2 wrapper | `/Users/vishvananda/boost_1_91_0/run-cppgm-b2.sh` |
| compiler under test | `/Users/vishvananda/cppgm-extended/dev/cppgm++` |
| default suite jobs | `8` |
| default suite timeout | `1800s`, adjusted upward for known large suites |

## Fixed Performance Baseline

| Commit | Baseline file | Instructions | Max RSS | Peak footprint | Wall time | Status |
|---|---|---:|---:|---:|---:|---|
| `db9879223` | `/tmp/cppgm-boost-frontier-v2-db9879223-baseline.json` | 271,651,249,439 | 1,293,901,824 B | 1,024,110,592 B | 60.43s | recorded, immutable |

## Performance Ledger

Every production compiler commit gets a row against the fixed baseline. Add a
rolling delta only when it helps isolate the incremental cost.

| Commit | Frontier | Fixed instruction delta | Max RSS delta | Footprint delta | Rolling delta | Report | Decision |
|---|---|---:|---:|---:|---:|---|---|
| `(qualified-friend fix)` | Qualified friend class template-id initial parse | +0.17% | +1.66% | +0.00% | n/a | `/tmp/cppgm-boost-frontier-v2-qualified-friend-perf.json` | pass; instruction and memory gates remain within tolerance |
| `(typed-qualified-call fix)` | Substituted qualified static member-template call | -0.08% | +1.15% | -0.01% | n/a | `/tmp/cppgm-boost-frontier-v2-typed-qualified-call-perf.json` | pass; below hotspot threshold |
| `(typed-va-list ABI fix)` | Host `__builtin_va_list` function symbols | -0.11% | +0.22% | -0.03% | n/a | `/tmp/cppgm-boost-frontier-v2-va-list-typed-abi-perf.json` | pass; below hotspot threshold |
| `(typed-member-pointer fix)` | Qualified current-specialization member-function address | -0.10% | -0.81% | +0.01% | n/a | `/tmp/cppgm-boost-frontier-v2-typed-member-pointer-perf.json` | pass; below hotspot threshold |
| `(typed bound-template fix)` | Nested member alias template-template call result | -0.13% | -0.09% | +0.02% | n/a | `/tmp/cppgm-boost-frontier-v2-bound-template-identity-perf.json` | pass; below hotspot threshold |
| `(parenthesized noexcept member fix)` | Parenthesized class member name with a `noexcept` definition | -0.13% | -0.31% | +0.02% | n/a | `/tmp/cppgm-boost-frontier-v2-parenthesized-member-noexcept-perf.json` | pass; below hotspot threshold |
| `(unused default lazy fix)` | Explicit alias argument must not instantiate an unused default | -0.27% | -0.26% | -0.06% | n/a | `/tmp/cppgm-boost-frontier-v2-unused-default-lazy-perf.json` | pass; all cumulative metrics improve |
| `(non-deduced partial pattern fix)` | Reused non-deduced partial-specialization pattern after earlier deductions | +0.75% | -0.56% | +0.04% | eager typed substitution was +0.94%; resolver-first fast path recovered 0.19 percentage points | `/tmp/cppgm-boost-frontier-v2-nondeduced-partial-pattern-fastpath-perf.json` | pass; hard gates pass; hotspot counters and 1 ms sample show no pathological path or cache-masked correctness |
| `(duplicate-base static lookup fix)` | Inherited non-object declaration reached through duplicate base subobjects | +0.74% | +1.05% | +0.06% | -0.01 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-duplicate-base-static-member-perf.json` | pass; all fixed cumulative gates remain within tolerance |
| `(typed base-trait traversal fix)` | Transitive base trait across distinct specializations of the same class template | +0.73% | -0.06% | +0.10% | -0.01 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-base-of-specialization-cycle-key-perf.json` | pass; changed evaluator is absent from the 1 ms sample and semantic text-resolution counters remain zero |
| `(external vbase layout runtime fix)` | Imported virtual-base pointer adjustment for classes without locally observed virtual functions | +0.74% | +0.50% | +0.10% | +0.01 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-external-vbase-layout-runtime-final-perf.json` | pass; below hotspot threshold and all fixed cumulative gates remain within tolerance |
| `(extern-template out-of-class member fix)` | Suppressed out-of-class member ownership under an explicit class-instantiation declaration | +0.88% | -0.05% | +0.06% | +0.14 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-extern-template-out-of-class-member-perf-repeat.json` | pass; repeated without code changes after the first near-limit +0.95% result; incremental movement remains below the hotspot threshold |
| `(dependent alignof NTTP fix)` | Concrete class layout for a substituted `alignof(T)` non-type template argument | +0.68% | +1.59% | +0.06% | -0.20 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-align-dependent-nttp-perf.json` | pass; all cumulative hard gates pass and instructions improve from the preceding frontier |
| `(declarator-owner implicit typename fix)` | Nested member-template type in an out-of-class static data definition | +0.80% | +0.25% | +0.15% | +0.12 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-owner-implicit-typename-perf.json` | pass; all cumulative instruction and memory gates remain within tolerance |
| `(qualified rvalue-reference parser fix)` | Namespace-qualified template-id followed by an `&&` abstract declarator | +0.81% | +0.99% | +0.10% | +0.01 percentage points instructions from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-qualified-rref-parser-perf.json` | pass; initial parsing retains the structured type-id and all cumulative hard gates remain within tolerance |
| `7c7e7cfc9` | Constexpr flat aggregate initialization and array member access | +0.86% | +0.50% | +0.10% | the initial implementation was optimized before acceptance to avoid duplicate semantic work | `/tmp/cppgm-boost-frontier-v2-array-constexpr-final-perf.json` | pass; fixed cumulative gates remain within tolerance after the normal-path optimization |
| `43bd0cc6b` | Constexpr array subobject pointer identity | +0.65% | +0.23% | +0.10% | -0.21 instruction percentage points from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-array-pointer-final-perf.json` | pass; cumulative instructions and memory improve from the preceding frontier |
| `(array cv reference binding fix)` | Top-level cv comparison for array reference binding | +0.73% | +0.26% | +0.08% | +0.09 instruction percentage points from the preceding frontier | `/tmp/cppgm-boost-frontier-v2-array-cv-reference-perf.json` | pass; counters are stable and sampling contains no frame in the changed helper |

## Suite Cursor

The inventory order comes from `docs/boost-b2-suite-status-20260511.md`. Add a
row when a suite is attempted. Do not prepopulate passes from V1.

| # | Suite | V2 status | Commit | Forced run evidence | Notes |
|---:|---|---|---|---|---|
| 1 | `libs/accumulators/test` | pass | `(external vbase layout runtime fix)` | Exact final forced survey rebuilt the full graph for 1800s with zero failures; all five former persistence failures (`rolling_count`, `rolling_sum`, `rolling_moment`, `rolling_variance`, and `rolling_mean`) rebuilt and passed. Log: `/tmp/boost-frontier-v2-suite-001-external-vbase-layout-runtime-final.log`. The non-forced continuation updated the remaining 12 targets, all four tests passed, and B2 exited successfully. Log: `/tmp/boost-frontier-v2-suite-001-external-vbase-layout-runtime-final-continuation.log`. The two compiler children orphaned by the forced timeout were terminated, then their affected `tail_variate_means` targets were forced cleanly; both rebuilt, passed, and B2 updated all 76 requested targets. Log: `/tmp/boost-v2-acc-external-vbase-layout-runtime-final-tail-focused.log`. | Suite closed on the exact final implementation. The final repository direct-LowIR report passed `3830 / 3830`; log `/tmp/cppgm-boost-frontier-v2-external-vbase-layout-runtime-final-test-report.log`. |
| 2 | `libs/algorithm/test` | pass | `(extern-template out-of-class member fix)` | The exact focused target forced 32 targets, rebuilt the Boost.Test archive, and passed compile, link, and runtime; log `/tmp/boost-v2-algorithm-apply-permutation-extern-template-fix.log`. The exact full forced suite rebuilt 218 targets and exited successfully; log `/tmp/boost-frontier-v2-suite-002-extern-template-fix-full.log`. | Initial forced evidence `/tmp/boost-frontier-v2-suite-002-initial-forced.log` had one real failure: `apply_permutation_test` linked against the otherwise redundant Boost.Test archive and reported 27 duplicate symbols. All positives and deliberate compile-fail targets pass with the fix. Final direct-LowIR report passes `3830 / 3830`; log `/tmp/cppgm-boost-frontier-v2-algorithm-extern-template-final-test-report.log`. |
| 3 | `libs/align/test` | pass | `(dependent alignof NTTP fix)` | The focused `alignment_of_test` forced four targets and passed compile, link, and runtime; log `/tmp/boost-v2-align-alignment-of-after-layout-fix.log`. The exact complete forced suite rebuilt all 60 targets and exited successfully; log `/tmp/boost-frontier-v2-suite-003-alignof-layout-fix-full.log`. | The initial forced survey passed 56 targets and had one causal compile failure plus three downstream skips: libc++ `alignment_of<Struct<bool>>` could not instantiate `integral_constant<size_t, alignof(_Tp)>`; log `/tmp/boost-frontier-v2-suite-003-initial-forced.log`. The final direct-LowIR report passes `3831 / 3831`; log `/tmp/cppgm-boost-frontier-v2-align-final-test-report.log`. |
| 4 | `libs/any/test` | pass | `(qualified rvalue-reference parser fix)` | Forced focused `unique_move` and `no_rtti_unique_move` rebuilt eight targets and both passed compile, link, and runtime; log `/tmp/boost-v2-any-unique-move-parser-fix.log`. The exact complete forced survey updated 164 targets in 54.3s and exited successfully; log `/tmp/boost-frontier-v2-suite-004-qualified-rref-parser-fix/libs__any__test.log`. | Initial forced evidence `/tmp/boost-frontier-v2-suite-004-initial-forced.log` had eight failures from the separately committed missing-`typename` defect plus the later move-cast failure. The first fix cleared all no-RTTI failures; the final parser fix clears both remaining move targets. Final direct-LowIR report passes `3833 / 3833`; log `/tmp/cppgm-boost-v2-suite-004-qualified-rref-full-report.log`. |
| 5 | `libs/array/test` | pass | `(three Array fixes)` | Focused constexpr aggregate access passed in `/tmp/boost-v2-array-init-cx-focused-final.log`; focused pointer-identity targets passed in `/tmp/boost-v2-array-pointer-focused-final.log`; focused `to_array_test` passed in `/tmp/boost-v2-array-to-array-fixed.log`. The exact complete forced survey rebuilt all 146 discovered targets in 56.7s and exited successfully; log `/tmp/boost-frontier-v2-suite-005-array-cv-reference-fix/libs__array__test.log`. | Initial forced evidence `/tmp/boost-frontier-v2-suite-005-initial-forced/libs__array__test.log` had six constexpr failures split between structured aggregate access and subobject pointer identity, plus one independent array cv-reference overload failure. Each cause has an earliest owner reducer and uncached proof. Final direct-LowIR report passes `3836 / 3836`; log `/tmp/cppgm-boost-frontier-v2-array-final-full-report.log`. |

Allowed statuses are `pending`, `running`, `frontier`, `blocked-external`, and
`pass`. A timeout is evidence, not a pass.

## Active Frontier

- suite: `#6 libs/asio/test`
- focused targets: pending the initial forced survey
- failure phase: none established
- diagnostic: none established
- reduced repro: none
- owning PA/cluster: pending any real compiler failure
- implementation area: pending any real compiler failure
- performance risk: current cumulative result is +0.73% instructions, +0.26%
  RSS, and +0.08% footprint; all fixed cumulative hard gates pass
- next action: run the exact forced Asio survey and classify its first real
  compiler failure, if any

## Fix Ledger

Add one row per coherent compiler fix. Keep detailed logs in `/tmp`; keep the
stable command, diagnostic, reducer, validation, and measured deltas here.

| Status | Suite/target | Root cause and typed fix | Owner regression | Pre-fix evidence | Validation | Perf vs fixed baseline | Commit |
|---|---|---|---|---|---|---|---|
| fixed | `libs/accumulators/test//count` dependency build | The initial parser treated an unknown qualified template suffix before `;` as expression-ambiguous even after an elaborated class-key. `parse_class_specifier` now selects the existing typed qualified-name mode that permits the final template-id, preserving `QualifiedName`, `TemplateIdSyntax`, qualifier template syntax, and qualifier type syntax on the AST node. | `pa21/tests/general/300-qualified-friend-class-template-id.t`, placed at the `template.friend` owner `pa21:300` | Non-STL reducer and Boost `friend class detail::interface_iarchive<Archive>;` both failed during initial parse; parser trace rejected the class member at `KW_FRIEND`. Focused pre-fix log: `/tmp/boost-v2-acc-count-parser-frontier.log`. | Warning-clean build; reducer accepted by Clang and `cppgm++`; PA10/PA21 direct LowIR report `352/352`; PA21 strict `216/216`, compared `161`, failures `0`; placement audit marks all reducer features `ok`; all 23 text-reparse categories remain zero and 14 audit tests pass; forced Boost rerun clears the parser diagnostic and builds Serialization. | instructions +0.17%; max RSS +1.66%; peak footprint +0.00%; pass | `(this commit)` |
| fixed | `libs/accumulators/test//count` | Substitution had already attached the concrete `support::empty_list` type to `Next` through `qualifier_type_syntax`, but both qualified function lookup paths discarded that semantic type and tried to look up its fully qualified rendering as one name component. `lookup_function_templates_node` and `lookup_functions_node` now consume `cppast_qualifier_type_syntax(...)->semantic_type` first and retain the existing structured template-id/type lookup only as a fallback. This corrects the uncached algorithm; no cache, text parser, or spelling special case is added. | `pa21/tests/general/300-qualified-owner-static-template-nttp.t`, placed at the `template.member_template` owner `pa21:300` | Exact Boost-header reducer `/tmp/cppgm-boost-v2-parameter-arg-list.cpp` and non-STL reducer `/tmp/cppgm-boost-v2-qualified-dependent-static-template.cpp` failed resolving `mp_bool`/`bool_constant`. Semantic trace showed the substituted qualifier type was present while call lookup reported `template_count=0` only for the namespace-qualified concrete owner. | Warning-clean compiler build; exact Boost and non-STL reducers accepted by `cppgm++`, and the non-STL reducer accepted by Clang; focused PA21 check passes; direct PA21 report `217/217`; PA21 strict `217/217`, compared `161`, failures `0`; placement audit reports zero findings and marks all reducer features `ok`; all 23 text-reparse categories remain zero and 14 audit tests pass; forced single-job `count` rebuild compiles `count.o` and advances to Boost.Test `format_report`. | instructions -0.08%; max RSS +1.15%; peak footprint -0.01%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//count` Boost.Test dependency | Lookup found `format_report`, deduction succeeded, and the specialization signature was concrete, but typed symbol construction could not represent the semantic `__builtin_va_list` type and discarded the only candidate. The symbol adapter now maps the builtin through `abi_mangle::Type`: on x86_64 the underlying type is typed as `__va_list_tag[1]`, with the required function-parameter adjustment to `__va_list_tag *`; supported non-x86 host forms are also selected structurally. A separate function-parameter adapter prevents the incorrect shortcut of treating every `va_list` use as a pointer. The PA30 model already provides the necessary typed array, pointer, named-type, and standard-namespace primitives, so no scaffold extension or raw mangled fragment is added. | `pa34/tests/compile/500-builtin-va-list-function-template-symbol.t`, placed in the PA34 host-builtin compile cluster | Non-STL reducer `/tmp/cppgm-v2-format-report-min.cpp` failed after successful template deduction with `failed to build ABI IR function symbol for weak function tools::format_report`; symbol trace identified the `__builtin_va_list` parameter. Clang emits `P13__va_list_tag` for a parameter and `A1_13__va_list_tag` for a template type argument on x86_64. | Warning-clean build; non-STL parameter and type-argument symbols match Clang byte-for-byte; exact Boost.Test `test_tools.cpp` compiles; focused PA34 check passes; direct PA34 report `309/309`; all configured strict suites pass; placement audit reports zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass; forced single-job `count` rebuild compiles the complete Boost.Test archive and leaves only the two Regex objects failing. | instructions -0.11%; max RSS +0.22%; peak footprint -0.03%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//count` Boost.Regex dependency | The parsed `&matcher<A,B,C>::first` node retained its structured qualifier syntax, but two normal semantic consumers discarded the node: the eager initializer constant probe and target-aware overloaded member-pointer resolution both called legacy `lookup_qualified_functions` with the rendered `QualifiedName`. Both paths now use `lookup_functions_node` or `lookup_function_template_id_node`, preserving typed qualifier/template syntax. This corrects the uncached algorithms; no cache, text parser, or spelling special case is added. | `pa26/tests/general/300-current-specialization-qualified-member-pointer.t`, placed at the `class.member_pointer` owner `pa26:300` | Non-STL reducer `/tmp/cppgm-v2-regex-member-pointer.cpp` failed first in the opportunistic constant initializer probe and then, after that path was fixed, in actual target-aware member-pointer resolution. Debug stacks showed both calls entering `lookup_qualified_functions` and decomposing `matcher<A,B,C>` without an AST node. Exact Boost.Regex failures were `posix_api.o` and `wide_posix_api.o` in `perl_matcher::find_imp`. | Warning-clean build; reducer accepted by Clang and `cppgm++`; focused PA26 check passes; PA26 direct report `67/67`; all configured strict suites pass; placement audit reports zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass; forced single-job `count` passes and updates 72 targets; full direct-LowIR report passes `3823/3823`; forced full-suite survey advances to independent Accumulators frontiers. | instructions -0.10%; max RSS -0.81%; peak footprint +0.01%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//covariance` libc++ parameter collection | The initial parser recognized GNU complex types but retained only their spaced display text. When a class-template specialization later collected a member template, the structured declaration parser correctly refused to recover a multi-token type from that string and rejected the parameter clause. The parser now records `_Complex` as a typed builtin transform over the initially parsed component `type_id`; ordinary and template declaration paths consume that retained AST through the existing structured transform resolver. No semantic text lookup, reparse, cache, or libc++ spelling special case is added. | Strengthened `pa34/tests/compile/600-gnu-complex-template-constructor.t` to make the enclosing holder a class template and force specialization population | Non-STL reducer `/tmp/cppgm-v2-gnu-complex-class-template.cpp` and focused Boost `covariance.o` failed with `unsupported function template parameter-clause`; the old PA34 test did not instantiate its non-template holder and therefore never populated the member path. Clang accepts the reducer. | Warning-clean build; reducer and strengthened PA34 test pass; direct PA34 report `309/309`; all configured strict suites pass; PA34 placement audit reports zero findings and zero hygiene findings; all 23 text-reparse categories remain zero and 14 audit tests pass; forced focused `covariance` clears the parameter-clause diagnostic and advances to a typed GNU complex ABI gap. | instructions -0.04%; max RSS +0.66%; peak footprint -0.03%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//covariance` libc++ GNU complex symbols | Semantic GNU complex types were already structured named `TypePtr`s, but the typed symbol adapter did not map them into the ABI model, so weak member symbol construction failed. The adapter now maps complex float, double, and long double to `abi_mangle::Type::builtin` with Itanium codes `Cf`, `Cd`, and `Ce`. PA30's normalized fact grammar, README, and test surface expose the same typed forms, and symbol tracing captures the resulting `AbiMangleTarget`; no raw mangled suffix or rendered-symbol recovery is added. | New `pa30/tests/abi/100-gnu-complex-types.t`; strengthened `pa34/tests/compile/600-gnu-complex-template-constructor.t` with a non-template complex-parameter member | Focused Boost failed building the weak symbol for `std::__1::complex<float>::__builtin(_Complex float)`. Clang's object symbols confirm `Cf`, `Cd`, and `Ce`; the non-STL PA34 trace failed before the map and now records `holder<float>::set_builtin` as `_ZN6holderIfE11set_builtinECf`. | Warning-clean `abimangle` and `cppgm++` builds; PA30 fact output is exactly `Cf`, `Cd`, `Ce`; PA30/PA34 report passes `390/390`; both placement audits report zero findings and zero hygiene findings; all configured strict suites pass; all 23 text-reparse categories remain zero and 14 audit tests pass; focused `covariance` clears the ABI failure and advances to Boost.Parameter result type lookup. | instructions +0.08%; max RSS +1.17%; peak footprint +0.04%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//covariance` Boost.Parameter result type | A bound template-template parameter already retained the selected member alias declaration and its concrete owner type, but substitution replaced the semantic template-id head with the alias's rendered qualified name. Generic lookup then decomposed and re-resolved `arg_list<...>` instead of applying the bound member alias. Substitution now preserves the parameter head when a typed owner is present, bound-template resolution runs before generic template-id lookup, direct fallback lookup retains the AST node, concrete alias owners are materialized from typed specialization metadata, and already concrete declared owners are not rebound as current instantiations. No cache, text lookup, or spelling-specific recovery is added. | `pa22/tests/general/400-member-alias-template-template-call-result-owner.t`, placed at the PA22 full deduction/result-substitution owner | Exact non-STL reducer `/tmp/cppgm-v2-nested-member-alias-call.cpp` reproduced the `B<T...>` member-alias application and extractor call. Focused Boost failed resolving `arg_list<...>::binding::fn<...>` after the typed binding had already been selected; debugger stacks showed generic qualified lookup re-entering the rendered owner. Removing the concrete-owner guard also makes the reducer fail independently. | Warning-clean full compiler rebuild; reducer passes in 0.07s at 12.3 MB RSS and 301,955,485 instructions; focused PA22 neighborhood passes `3/3`; PA22 direct report passes `251/251`; all configured strict suites pass; PA22 placement audit reports zero findings and zero hygiene findings; all 23 text-reparse categories remain zero and 14 audit tests pass; forced focused `covariance` and forced full-suite `covariance`/`weighted_covariance` pass. | instructions -0.13%; max RSS -0.09%; peak footprint +0.02%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//extended_p_square` Boost.Random dependency | The class-model method parser rejected every declarator with a direct nested declarator from its normal typed filtered path. That guard is necessary for complex returned-array/function shapes, but it also rejected a structurally simple parenthesized member name such as `int (value)() noexcept`, leaving the outer function qualifier in the declaration input. The guard now distinguishes the exact `nested-declarator -> declarator -> identifier` name shape from complex nested declarators, so the established typed filter handles its outer parameter clause and `noexcept`. No text parse, fallback lookup, or cache is added. | `pa15/tests/general/200-parenthesized-noexcept-member-definition.t`, placed with ordinary class method and `noexcept` metadata ownership | Exact Boost-shaped reducer `/tmp/cppgm-v2-static-constexpr-parenthesized-member.cpp` reproduced `static constexpr unsigned long long (max)() noexcept`; minimal reducer `/tmp/cppgm-v2-parenthesized-noexcept-member-definition.cpp` reproduced the essential form. The same parenthesized forms without `noexcept` passed before the fix, isolating the incorrectly bypassed function-qualifier filter. Clang accepts both failing reducers. | Warning-clean full compiler build; minimal and exact reducers pass; focused PA15 neighborhood passes `2/2`; PA15 direct report passes `201/201`; all configured strict suites pass; PA15 placement audit reports zero findings and zero hygiene findings; all 23 text-reparse categories remain zero; forced focused and completed full-suite graph clear all 22 `splitmix64` member-definition failures. | instructions -0.13%; max RSS -0.31%; peak footprint +0.02%; pass, below hotspot threshold | `(this commit)` |
| fixed | `libs/accumulators/test//extended_p_square` Boost.Iterator dependency | Structural alias expansion recursively materialized every concrete class-template type argument before resolving the selected outer template. Boost.Iterator's `eval_if_default_t<Value, iterator_value<Base>>` therefore instantiated the unused `iterator_value<unsigned long>` default even though `Value` was explicit; the later Boost.Test `check_frwd` diagnostic was only fallout. Alias expansion now carries typed arguments without recursively completing them. Substitution first preserves source-scope dependent non-type expression metadata, then refreshes stale specialization metadata only for a fully concrete all-type argument list, which retains correct pack expansion without reviving eager completion. No cache, text parse, or spelling special case is added. | `pa22/tests/general/300-alias-explicit-argument-skips-unused-default-instantiation.t`, placed at the PA22 alias-instantiation owner | Exact Boost reducer `/tmp/cppgm-v2-boost-iterator-traversal-alias.cpp` and non-STL reducer `/tmp/cppgm-v2-inherited-deferred-alias.cpp` both failed by materializing an invalid unused default. Restoring the removed eager loop makes the checked-in regression fail. An initial unconditional metadata refresh regressed existing PA22 dependent non-type cases and was discarded. | Warning-clean build; Clang and `cppgm++` accept the new regression and both reducers; new and existing PA22 regressions plus both reducers pass with `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE=1`; PA22 direct report passes `252/252`; all configured strict direct-LowIR suites pass; PA22 placement audit reports zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass; PA35 reverse-iterator control passes; forced focused Boost compile/link/run passes and the completed suite graph clears all three alias-related failures. | instructions -0.27%; max RSS -0.26%; peak footprint -0.06%; pass, all cumulative metrics improve | `(this commit)` |
| fixed | `libs/accumulators/test//error_of` | Partial-specialization matching cached a typed placeholder pattern for a non-deduced later argument, then reused it after an earlier argument had deduced the partial's parameters. The ordinary dependent-type resolver handled most cached patterns, but a still-dependent nested class/alias pattern retained the placeholder binding and selected the fallback specialization. Matching now builds the complete typed template argument list from the deduction state and applies the existing typed `TypePtr` substitution only when the resolver fast path leaves a non-deducible pattern dependent. The substituted pattern is then resolved in the current match scope. No source parse, text decomposition, result cache, or spelling-specific recovery is added. | `pa22/tests/spec/300-nondeduced-partial-pattern-recursive-completion.t`, a header-free reducer at the earliest dependent deduction/recursive completion owner | Exact Boost reducer `/tmp/cppgm-v2-boost-count-construction.cpp` failed with `failed non-type template argument evaluation: ::boost::mpl::aux::nested_type_wknd<T1>::value`; the reduced test failed by selecting `value_of_impl<non_iterator_tag>` after the stale `enable_if<has_fusion_tag<Sequence>>::type` pattern missed. Clang accepts both. | Warning-clean build; exact and checked-in reducers pass normally and with `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE=1` and `CPPGM_DISABLE_DEPENDENT_TYPE_RESOLUTION_CACHE=1`; focused PA22 direct check passes; PA22 direct report passes `253/253`; strict direct-LowIR PA18/19/21/22/23 passes with zero failures; PA22 placement and hygiene audits report zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass; forced single-job Boost target passes compile, link, and runtime with 72 updated targets. Hotspot counters are in `/tmp/cppgm-v2-nondeduced-hotspot.log`; temporary trace accounting on the stable benchmark saw 3,491 eligible patterns, 2,071 still dependent after the fast resolver, and 861 successful typed substitutions. The 1 ms sample `/tmp/cppgm-v2-nondeduced.sample.txt` contains 7,104 samples; the fallback substitution has fewer than five top-of-stack samples and the dependent resolver has 12, while allocation and existing ABI/type work dominate. | instructions +0.75%; max RSS -0.56%; peak footprint +0.04%; pass; optimized from the eager +0.94% version and below all hard gates | `(this commit)` |
| fixed | `libs/accumulators/test//rolling_mean`, `//rolling_variance` compile | Hierarchy lookup established that every path found the same declaration, then unconditionally required a unique base subobject and conflated entity lookup with object adjustment. The lookup result now states whether it needs a unique subobject: fields and callable sets containing nonstatic members retain the ambiguity check, while static data, all-static callable sets, and nested template declarations keep the shared declaration without an object offset. The same traversal and access metadata remain; no cache, source parse, text decomposition, or spelling-specific recovery is added. | `pa26/tests/general/100-duplicate-base-static-member-lookup.t`, paired with the existing `100-bad-ambiguous-member.t` negative control at the PA26 multiple-inheritance owner | Non-STL reducers `/tmp/cppgm-v2-static-object-duplicate-base.cpp` and `/tmp/cppgm-v2-static-function-duplicate-base.cpp` failed with `ambiguous base class path [current D] [target A]`, while Clang accepts both. The corresponding nonstatic-field reducer is rejected by both compilers. Boost failed looking up `tag::lazy_rolling_{mean,variance}::window_size` because `rolling_window_size_<0>` is reached through two dependency bases. | Warning-clean compiler rebuild; static data/function reducers and the expanded nested class/alias/function-template regression pass with Clang and `cppgm++`; nonstatic-field control remains rejected; PA26 direct report passes `68/68`; all configured strict direct-LowIR suites pass; PA26 placement reports zero findings and zero hygiene findings; all 23 text-reparse categories remain zero and 14 audit tests pass. Forced focused Boost rebuild compiles and links both targets, updates 72 targets, and exposes only runtime failures; log `/tmp/boost-v2-acc-duplicate-base-static-fix.log`. | instructions +0.74%; max RSS +1.05%; peak footprint +0.06%; pass | `(this commit)` |
| fixed | `libs/accumulators/test//rolling_mean` lazy result | The transitive `__is_base_of` evaluator used the unspecialized class metadata name as its traversal key. Distinct specializations such as `depends_on<sum,count>` and `depends_on<window>` therefore appeared to be the same visited node, so MPL omitted the rolling-window dependency and materialized the accumulator update order incorrectly. The traversal now tracks typed `TypePtr` identities with `type_equals`. This corrects the traversal algorithm itself; it adds no cache, text parse, rendered identity, or symbol-path change. | `pa34/tests/compile/700-builtin-base-of-specialization-cycle-key.t`, a header-free reducer at the PA34 host builtin-trait owner | Semantic tracing showed `depends_on<window>` rejected as already visited only because a different `depends_on<sum,count>` specialization shared its metadata name. The pre-fix compiler rejects the reducer's transitive base assertion and orders `rolling_sum` before `rolling_window_plus1`, producing `4.2`/`5.4` instead of `4.0`/`5.0`. | Warning-clean build; Clang and the fixed compiler accept the regression; the exact reduced runtime returns the correct rolling results; PA34 focused and direct report pass `310/310`; all configured strict direct-LowIR suites pass; PA34 placement reports zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass. The forced focused Boost target rebuilds 72 targets and leaves only the independent persistence decoder failure; log `/tmp/boost-v2-acc-base-of-specialization-cycle-key-fix.log`. The forced full survey reaches 36 passes with only the exact five persistence failures and zero compile failures before its bound; evidence `/tmp/boost-frontier-v2-suite-001-base-of-specialization-cycle-key`. Hotspot counters are in `/tmp/cppgm-v2-base-of-hotspot.log`; the 1 ms sample `/tmp/cppgm-v2-base-of-hotspot.sample.txt` contains no frames from the changed evaluator. | instructions +0.73%; max RSS -0.06%; peak footprint +0.10%; pass | `(this commit)` |
| fixed | `libs/accumulators/test//rolling_count`, `//rolling_sum`, `//rolling_moment`, `//rolling_variance`, `//rolling_mean` persistence | LowIR virtual-base adjustment required a class to have locally observed virtual functions or a local vtable before using its imported runtime layout. In the exact Boost.Serialization translation unit, `std::basic_ostream` had a typed external vtable candidate and precise virtual-base layout but no locally observed virtual function, so `basic_ostream *` to `basic_ios *` incorrectly fell back to a fixed `+8` adjustment. Dynamic adjustment now accepts the typed imported external-layout fact directly. Pointer conversion on this external-only fallback uses a null-preserving branch before loading the vptr; existing local-vtable paths are unchanged. This corrects the uncached lowering algorithm and adds no cache, text parse, rendered identity, or symbol-path change. | Strengthened `pa33/tests/general/200-host-virtual-base-indirect-base-access.t` by removing `B`'s explicit virtual destructor, leaving virtual inheritance as its sole reason to require the host-provided vtable. This is a header-free host ABI/runtime reducer at the earliest owning PA. | Debugger evidence in `/tmp/cppgm-v2-persistence-lldb-value.txt` and `/tmp/cppgm-v2-persistence-save-counts.txt` showed the archive wrote the correct byte counts but `ostream_iterator<char>::put_val` corrupted the stream state. Pre-fix LowIR in `/tmp/cppgm-v2-basic-text-oprimitive.lowir` used fixed `+8`; exact semantic state contained `class_virtual_base_layouts_` and an external vtable candidate but no local virtual-function mark. The fixed LowIR loads the runtime offset from the vtable `-24` slot. | Exact normal and `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE=1` LowIR are byte-identical in `/tmp/cppgm-v2-boost-basic-text-oprimitive.final.lowir` and `/tmp/cppgm-v2-boost-basic-text-oprimitive.final-nocache.lowir`; corrected Boost persistence roundtrip passes. Focused PA32 null regression, strengthened PA33 owner regression, PA27 direct report `34/34`, PA33 direct report `2256/2256`, all configured strict direct-LowIR suites, PA33 placement/hygiene audit, warning-clean build, and all 23 zero-reparse categories plus 14 audit tests pass. Exact forced Accumulators survey and continuation close the suite with zero failures; final full direct-LowIR report passes `3830/3830`. | instructions +0.74%; max RSS +0.50%; peak footprint +0.10%; pass; +0.01 instruction percentage points from the preceding frontier | `(this commit)` |
| fixed | `libs/algorithm/test//apply_permutation_test` | Explicit class-instantiation suppression had a special empty-member bypass that treated every weak template member as though it were inline. An empty user-defined out-of-class destructor therefore bypassed `extern template` suppression and was emitted weakly in ordinary client translation units. The bypass now uses typed `FunctionBinding` inline/constexpr facts plus the parsed declaration/definition node identity; weak linkage alone cannot establish ownership. No cache, text parse, spelling test, or symbol-text path is added. | Strengthened `pa32/tests/general/200-host-extern-template-ooc-member-reference.t` with a header-free out-of-class destructor and explicit destructor call. Its inspect oracle requires the consumer object to leave `Box<int>::~Box()` undefined while the host provider owns the explicit instantiation; the existing in-class member remains a weak control. | Header-free reducer `/tmp/cppgm-boost-algorithm-extern-template-destructor.cpp` was accepted by Clang but cppgm emitted two weak destructor variants. In the Boost.Test archive, cppgm's weak `std::__1::basic_ios<char>::~basic_ios()` definition caused `execution_monitor.o` to be extracted to satisfy `apply_permutation_test`, producing 27 duplicate framework symbols. The test object links and runs when the redundant archive is omitted, and a host-compiled test object reproduces the duplicate-symbol behavior against the polluted archive. | Warning-clean build; reducer objects from normal and `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE=1` builds are byte-identical and match Clang's undefined destructor symbol; focused PA32 regression passes; PA32 direct report passes `108/108`; all configured strict direct-LowIR suites pass; the strengthened regression has no placement or hygiene finding, while the PA32 audit still reports the separately queued pre-existing attribute-placement violation; all 23 text-reparse categories remain zero and 14 audit tests pass. The rebuilt Boost.Test archive leaves `basic_ios<char>::~basic_ios()` undefined. Forced focused and full Algorithm runs pass, and the final full direct-LowIR report passes `3830/3830`. | instructions +0.88%; max RSS -0.05%; peak footprint +0.06%; pass after a no-code-change repeat; +0.14 instruction percentage points from the preceding frontier | `(this commit)` |
| fixed | `libs/align/test//alignment_of_test` | The retained `alignof(T)` AST was already structurally substituted to a concrete typed operand, but the template-argument leaf constexpr hook returned that `TypePtr` without completing its class layout. The shared evaluator therefore reported `named type alignment unavailable`, after which the dependency check misclassified the concrete expression as still dependent. The hook now completes layout through the existing semantic class-completion service before the evaluator asks for alignment, matching the normal semantic constexpr path. No cache, text parse, spelling test, or symbol path is added. | `pa19/tests/general/100-dependent-alignof-nontype-template-argument.t`, a header-free reducer at the earliest integral NTTP owner `pa19:100` | Exact reducer `/tmp/cppgm-align-dependent-nttp.cpp` and the smaller `Payload` regression both failed before the fix on `constant<size_type, alignof(T)>`; Clang accepts both. The pre-fix failure is unchanged with `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE=1`, and trace evidence `/tmp/cppgm-align-dependent-nttp-eval-trace.log` shows the post-substitution operand is the concrete `struct Struct<bool>` before layout evaluation fails. | Warning-clean build; exact and owner reducers compile and run with Clang, normal cppgm, and cache-disabled cppgm; focused owner check passes; PA19 direct report passes `135/135`; all configured strict direct-LowIR suites pass; PA19 placement and hygiene audits report zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass. Forced focused and complete Align runs pass, and the final full direct-LowIR report passes `3831/3831`. | instructions +0.68%; max RSS +1.59%; peak footprint +0.06%; pass; -0.20 instruction percentage points from the preceding frontier | `(this commit)` |
| fixed | `libs/any/test` no-RTTI compile targets | `resolve_qualified_owner_type_node` correctly retained the parsed `QualifiedName` and qualifier template-id syntax for an out-of-class declarator, but represented the owner nested-name-specifier as an ordinary synthetic `type_name`. That caused the dependent-type resolver to demand `typename` for `outer<T>::inner<D>` even though the declarator-id grammar already establishes each nested-name-specifier component as a type. The synthetic typed node now carries an explicit implicit-`typename` context fact, and the existing diagnostic predicate consumes it. No cache, text parse, source-spelling check, or broad diagnostic suppression is added. | `pa21/tests/general/300-out-of-class-nested-member-template-static-definition.t`, a header-free reducer at the earliest member-template declaration/collection owner `pa21:300` | `/tmp/cppgm-any-nested-member-template-static.cpp` failed with `dependent qualified type requires typename: outer<T,Mode>::inner<D>` both normally and with class-info and dependent-type-resolution caches disabled; Clang accepts it. Boost.Describe's `update_modifiers<T,Bm>::fn<D>::pointer` produced the same diagnostic in eight Any no-RTTI targets. | Warning-clean build; reducer and owner test pass with Clang, normal cppgm, and both relevant caches disabled; focused owner check passes; PA21 direct report passes `218/218`; all configured strict direct-LowIR suites pass; PA21 placement and hygiene audits report zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass. Forced focused Boost passes; the full forced suite clears all eight shared failures and leaves only the two independent `unique_move` variants. | instructions +0.80%; max RSS +0.25%; peak footprint +0.15%; pass; +0.12 instruction percentage points from the preceding frontier | `(this commit)` |
| fixed | `libs/any/test/unique_any//unique_move`, `//no_rtti_unique_move` | Initial template-argument parsing already tokenized `&&` as `OP_LAND`, but the fallback fragment classifier treated only trailing `*` and `&` as possible abstract declarators. It therefore attempted `library::box<int>&&` only as an expression, failed that parse, and left a text-only `TemplateArgumentSyntax`; explicit function-template argument resolution correctly refused to reparse it. Classifying `OP_LAND` as type-then-expression preserves the initial structured qualified template-id plus rvalue-reference declarator. No semantic reparse, cache, rendered identity, or symbol-path change is added. | `pa18/tests/general/100-qualified-template-id-rvalue-reference-argument.t`, a header-free reducer at the first type-template-argument owner `pa18:100` | `/tmp/cppgm-qualified-template-id-rvalue-reference-explicit-arg.cpp` failed with `unknown function api::take<library::box<int>&&>` both normally and with class-info plus dependent-type-resolution caches disabled; Clang accepts and runs it. Trace showed the semantic boundary received `syntax=yes` but no type-id, template-id, or expression, proving loss during the initial parse rather than semantic reparsing. | Warning-clean build; the exact reducer passes with Clang, normal cppgm, and both caches disabled; the PA18 regression emits the expected LowIR; PA18 direct report passes `223/223`; all configured strict direct-LowIR suites pass; PA18 placement/hygiene reports zero findings; all 23 text-reparse categories remain zero and 14 audit tests pass. Both focused Boost targets and the complete forced Any suite pass. Final direct-LowIR report passes `3833/3833`. | instructions +0.81%; max RSS +0.99%; peak footprint +0.10%; pass; +0.01 instruction percentage points from the preceding frontier | `(this commit)` |
| fixed | `libs/array/test` constexpr initialization and access targets | Constexpr class initialization consumed flattened brace arguments as if each argument directly initialized one field, so a nested built-in array aggregate failed before member access. The evaluator now uses the shared structured aggregate-initializer plan when brace elision requires it. It also evaluates array and overloaded `operator[]` access structurally, handles the selected branch of discarded conditionals, and records `__builtin_expect` with a typed `FunctionBinding` enum consumed by constant evaluation. No name spelling is reparsed in the evaluator. | `pa20/tests/general/300-constexpr-template-aggregate-subscript-member.t`, placed at the earliest constexpr class-template evaluation owner `pa20:300` | The initial Array survey failed `array_init_test_cx`, `array_copy_test_cx`, and `array_get_test_cx` while evaluating aggregate initialization or member subscript access. The reduced template aggregate case failed with the same constant-expression diagnostic before the fix. | Warning-clean build; owner regression and PA20 direct report pass; all configured strict direct-LowIR suites, placement checks, and the strict 23-category zero-reparse audit pass. The focused `array_init_test_cx` rebuild passes, and the broader fix clears all aggregate-access failures. | instructions +0.86%; max RSS +0.50%; peak footprint +0.10%; pass after removing duplicate normal-path semantic work | `7c7e7cfc9` |
| fixed | `libs/array/test` constexpr pointer targets | Constexpr aggregate and array values did not propagate storage provenance through member and element access, reference initialization, reference returns, or copied call arguments. Address-of therefore could not produce a stable subobject pointer, and comparisons could not distinguish copied object storage from the source. Structured constant values now carry storage identity and offsets through those operations; by-value aggregate arguments receive distinct storage, while reference operations preserve their referent. Null pointer conversions and zero offset arithmetic are handled in the same typed constant-value model. | `pa20/tests/general/400-constexpr-subobject-pointer-identity.t`, covering array/member addresses, pointer offsets, reference preservation, and by-value copy identity | After the aggregate-access fix, `array_data_test_cx`, `array_iterator_test_cx`, and `array_access_test_cx` failed constexpr pointer assertions. The reduced non-STL cases failed identically with the relevant semantic caches disabled. | Normal and cache-disabled LowIR are byte-identical; warning-clean build, focused PA20 check, PA20 direct report `73/73`, all configured strict suites, placement, and the strict zero-reparse audit pass. All three focused Boost pointer targets compile, link, and run. | instructions +0.65%; max RSS +0.23%; peak footprint +0.10%; pass; cumulative instructions improve 0.21 percentage points | `43bd0cc6b` |
| fixed | `libs/array/test//to_array_test` | Reference binding first compares referent types while ignoring top-level cv. For arrays, that cv belongs to the element type in the semantic representation, so the scalar-only comparison incorrectly treated `int[3]` and `const int[3]` as unrelated. Conversion ranking then fell through to a non-reference qualification conversion, fabricated an array prvalue, and made the `const T (&&)[N]` overload viable for an lvalue. The typed referent comparison now recursively compares identical array shapes and ignores cv at the effective array top level. No template special case, cache, allocation, or text parse is added. | `pa14/tests/general/100-array-cv-rvalue-reference-overload.t`, placed at the earliest audit-approved reference conversion owner `pa14:100` | A non-template reducer reproduced the ambiguity for mutable and const array lvalues before the fix, proving this was a general conversion algorithm defect rather than template deduction. Clang selects the lvalue-reference overload. The pre-fix trace showed correct deduction and rejection of the mutable rvalue-reference candidate, followed by incorrect acceptance of the const rvalue-reference candidate. | The reducer passes normally and with all four relevant template/type caches disabled; the two modes emit byte-identical LowIR. PA14 owner report passes `200/200`; the existing converted-temporary rvalue-reference control passes; strict direct reports, placement, audit unit tests, and all 23 zero-reparse categories pass. Focused `to_array_test`, the complete forced Array survey, and the final direct report `3836/3836` pass. | instructions +0.73%; max RSS +0.26%; peak footprint +0.08%; pass; hotspot counters are stable and the changed helper is absent from the 1 ms sample | `(this commit)` |

## Decision Log

- `2026-07-15`: Closed Array after three independent typed fixes. Structured
  constexpr aggregate access cleared the initialization/copy/get group;
  subobject storage identity cleared the pointer/data/iterator group; and a
  non-template reducer proved `to_array` was a general array cv-reference
  ranking defect rather than template deduction or a cache-masked algorithm.
  Every reducer passes with the relevant caches disabled, all 23 text-reparse
  categories remain zero, the complete forced suite rebuilds all 146 discovered
  targets successfully, and the final direct report passes `3836 / 3836`.
  Cumulative instructions are +0.73% after optimization and sampling shows no
  new hotspot. Suite 5 is closed; the cursor advances to `libs/asio/test`.

- `2026-07-14`: The remaining Any failure was not semantic text reparsing.
  Initial parsing classified the already-tokenized `&&` in a qualified type
  template argument as expression-only, then carried only its display text to
  semantics. Adding `OP_LAND` to the existing type-then-expression classifier
  retains the structured type-id at the first parse. The non-STL PA18 reducer
  passes with both semantic caches disabled, all validation and performance
  gates pass, both focused move targets pass, the complete forced Any survey
  exits successfully, and the final direct report passes `3833 / 3833`. Suite
  4 is closed; the cursor advances to `libs/array/test`.
- `2026-07-14`: The initial forced Any survey found eight no-RTTI failures in
  Boost.Describe out-of-class definitions of nested member-template static data
  plus a later move-cast failure. The owner prefix was already structured at
  initial parse, but a synthetic `type_name` incorrectly activated the ordinary
  missing-`typename` rule. Marking that typed nested-name-specifier context
  clears all eight failures normally and with both semantic caches disabled.
  PA21, strict, placement, and zero-reparse gates pass; cumulative instructions
  are +0.80%. The forced full suite now leaves only `unique_move` and its
  no-RTTI variant, so suite 4 remains open at that independent frontier.

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
- `2026-07-13`: Completed the baseline bootstrap. The fixed three-run baseline
  was measured in a detached worktree at exact commit `db9879223`; the active
  and main branch tips at `2acedcec7` contain only V2 process documents and
  refreshed assignment references beyond that production compiler baseline.
- `2026-07-13`: Accepted one PA9 generated-program timeout in the full report
  as the known load-sensitive test behavior after its isolated direct-LowIR
  report passed `11 / 11` immediately.
- `2026-07-13`: The first forced V2 Accumulators run completed `mixed` in
  491.7s. Classified its earliest causal failure as an initial-parser frontier
  in a Boost.Serialization dependency rather than the later parallel target
  diagnostics.
- `2026-07-13`: Reduced the first frontier to an initial-parser failure on
  `friend class detail::interface_archive<T>;`. The typed class-specifier parse
  now accepts the unambiguous qualified template-id without reparsing text. The
  regression lives at the `pa21:300` template-friend owner, not PA10, under the
  current placement rules.
- `2026-07-13`: The forced `count` rerun cleared the parser failure and advanced
  `count.o` to `mp_bool` alias-template resolution. The same parallel run also
  observed independent Boost.Test `format_report` lookup and Boost.Regex
  template-id lookup failures; these are later dependency frontiers, not the
  earliest causal failure for the focused target.
- `2026-07-13`: Reduced `mp_bool` to a non-STL qualified static member-template
  call whose substituted owner type lives in `qualifier_type_syntax`. Both call
  lookup paths now use that typed owner directly. The forced single-job rebuild
  compiled `count.o` and established Boost.Test `format_report` as the next
  ordered target frontier; Boost.Regex remains a separate later dependency
  frontier.
- `2026-07-13`: Traced the apparent `format_report` lookup failure through
  successful lookup and deduction to typed ABI symbol construction for
  `__builtin_va_list`. Kept parameter adjustment distinct from the underlying
  array type, matched both Clang symbols, and verified that the existing PA30
  typed model needs no new fact or scaffold primitive. The forced rebuild now
  identifies Boost.Regex current-specialization lookup as the sole ordered
  blocker.
- `2026-07-13`: Debug stacks proved that the Regex failure was not parsing or
  cache behavior. Two normal semantic consumers discarded the already parsed
  member-pointer operand and used `QualifiedName`-only function lookup. Both now
  retain the AST node through typed lookup. The forced `count` rebuild passes,
  both Regex POSIX objects compile, and the full survey advances to independent
  Accumulators target failures.
- `2026-07-13`: The post-fix forced full-suite survey updated 114 targets and
  failed 37 compile targets. The first reported target-local failure was
  `covariance`, in libc++ `complex<float>` member-template constructor
  collection. Treat that as the next candidate only after a focused one-job
  rerun confirms it outside parallel scheduling.
- `2026-07-14`: Traced the final `covariance` failure past successful typed
  template-template binding. Substitution had overwritten the bound semantic
  head with its qualified display name, causing generic lookup to re-resolve
  the rendered owner. Preserving the typed binding fixes the uncached algorithm;
  the non-STL reducer, focused target, and full-suite covariance targets pass,
  with cumulative instructions down 0.13% from the immutable baseline.
- `2026-07-14`: The forced post-fix full survey updated 154 targets and failed
  28. The next ordered target is `extended_p_square`; 22 compile failures share
  its Boost.Random `splitmix64` member-definition diagnostic. Later independent
  classes are one MPL constant-evaluation failure, two ambiguous-base failures,
  and three serialization runtime failures.
- `2026-07-14`: Reduced the Boost.Random failure to a parenthesized member name
  followed by an outer `noexcept`. The class-model guard incorrectly classified
  the simple nested identifier as a complex returned declarator and bypassed
  the existing typed qualifier filter. Refining that structural predicate
  clears all 22 shared failures with instructions down 0.13% from the immutable
  baseline. The forced survey timed out while targets were still completing;
  a non-forced continuation completed the graph and established 13 remaining
  targets in six independent failure classes. `extended_p_square` remains the
  ordered target, now at visible `check_frwd` template candidate loss.
- `2026-07-14`: Traced the apparent `check_frwd` loss back through
  `iterator_traversal_t` to eager materialization of every alias argument. The
  compiler was instantiating an unused default that the selected explicit
  argument made irrelevant. Removing that eager normal-path work exposed stale
  all-type pack mangle metadata; the final substitution rule refreshes only
  fully concrete type-only argument lists while preserving dependent non-type
  expressions. Cache-disabled controls pass, and cumulative instructions fall
  0.27%. The focused target and all three affected suite tests pass. The forced
  survey reached its 1800s bound with four active targets; its continuation
  completed with ten independent failures. `error_of` is next.
- `2026-07-14`: The `error_of` diagnostic was downstream of a stale typed
  partial-specialization pattern, not failed MPL constant evaluation. The first
  argument had deduced `Sequence`, but the reused non-deduced `enable_if`
  pattern still carried its placeholder binding and selected the wrong
  iterator tag. The final algorithm runs the existing dependent resolver first
  and performs complete typed substitution only when the pattern remains
  dependent. Both semantic caches can be disabled without changing the result;
  no correctness cache or text reparse was introduced. The focused target
  passes compile, link, and runtime. Cumulative instructions are +0.75% after
  improving the eager implementation's +0.94%; hotspot counters and sampling
  show no pathological recursion or witness-only work on the normal path.
- `2026-07-14`: The post-`error_of` forced survey reached its 1800s bound after
  exercising every failure class. All five prior MPL/non-type compile failures
  pass. The remaining compile failures were `rolling_mean` and
  `rolling_variance`, both from the same declaration being reached through two
  nonvirtual `rolling_window_size_<0>` base subobjects; three rolling
  persistence tests separately failed base64 decoding at runtime.
- `2026-07-14`: Reduced the duplicate-base diagnostic to static data and static
  function lookups in a header-free PA26 multiple-inheritance case. Hierarchy
  lookup had incorrectly required a unique object subobject for every inherited
  entity. Classifying the typed lookup result preserves unique-path enforcement
  for fields and nonstatic callables while allowing one static or nested entity
  reached by multiple paths. The focused targets now compile and link. The
  forced full survey reached its bound with 42 passes, the exact five runtime
  failures, zero compile failures, and only previously passing
  `weighted_tail_variate_means` still active. Cumulative instructions are
  +0.74%, and the next ordered frontier is the `rolling_mean` lazy-window
  result behavior.
- `2026-07-14`: Traced the lazy rolling result to the uncached transitive
  `__is_base_of` algorithm. Its visited key used an unspecialized metadata name,
  so distinct `depends_on<...>` specializations collapsed and MPL omitted the
  rolling-window dependency. Typed `TypePtr` identity restores the correct
  materialization order without text recovery or a cache. The focused target
  clears every rolling-result assertion, and the forced full survey reaches 36
  passes with zero compile failures and only the exact five shared persistence
  failures before its 1800s bound. Cumulative instructions are +0.73%; the
  changed evaluator is absent from sampling and text-resolution counters stay
  at zero.
- `2026-07-14`: Traced the apparent base64 failure back to an incorrect imported
  virtual-base pointer adjustment in Boost.Serialization's output iterator.
  The exact translation unit retained a typed external vtable candidate and
  precise `basic_ostream` virtual-base layout, but lowering rejected that fact
  because it had not locally observed a virtual function and emitted fixed
  `+8` instead of the vtable `-24` slot lookup. The final fix admits the typed
  external layout directly and null-preserves pointer adjustment only on that
  fallback. Cache-disabled LowIR is byte-identical, the strengthened PA33
  header-free host ABI reducer passes, and no text or symbol recovery is added.
  The exact forced Accumulators survey has zero failures, its continuation exits
  successfully, and the final direct-LowIR report passes `3830 / 3830`. Suite
  1 is closed; the cursor advances to `libs/algorithm/test`.
- `2026-07-14`: The initial forced Algorithm survey had one real failure:
  `apply_permutation_test` reported 27 duplicate Boost.Test symbols at link.
  The test includes the header-only framework while its Jamfile also names the
  static framework archive, which is valid when archive extraction is correct.
  cppgm had polluted `execution_monitor.o` with a weak libc++ `basic_ios`
  destructor definition, so that member was extracted to satisfy the test
  object's destructor reference and its unrelated strong framework symbols
  collided. A non-STL reducer proved that the explicit-instantiation
  declaration was incorrectly bypassed for every empty weak member, including
  out-of-class definitions. The bypass now requires typed inline/constexpr or
  in-class definition facts. Normal and cache-disabled objects are identical,
  the rebuilt archive leaves the destructor undefined, and both the focused
  and complete forced suite pass. The repeated fixed-baseline performance gate
  is +0.88% instructions, -0.05% RSS, and +0.06% footprint; the final direct
  report passes `3830 / 3830`. Suite 2 is closed; the cursor advances to
  `libs/align/test`.
- `2026-07-14`: Cleared the pre-existing placement-audit finding in a separate
  test-only follow-on. The `__exclude_from_explicit_instantiation__` attribute
  is essential to the system-include aggregate-copy oracle, so removing it
  would weaken the test. The complete basename-matched sidecar family moved
  from `pa32:200` to PA36's `600` hosted header-emission/link-runtime cluster.
  The focused test passes, the combined PA32/PA36 direct report passes
  `176 / 176`, both placement audits report zero findings and zero hygiene
  findings, and all 23 text-reparse categories remain zero. No production code
  changed, so no performance measurement was required.
- `2026-07-14`: The initial forced Align survey had one causal compile failure:
  libc++ `alignment_of<Struct<bool>>` could not form its `alignof(_Tp)` integral
  constant. A header-free PA19 reducer proved that initial parsing and typed
  substitution were already correct and that disabling the class-info cache did
  not change the failure. The template-argument constexpr hook alone omitted
  concrete class-layout completion before the shared evaluator asked for
  alignment. Reusing the existing typed completion service fixes the uncached
  algorithm without reparsing or adding a cache. The focused and full forced
  Align runs pass, the final direct-LowIR report passes `3831 / 3831`, and the
  fixed-baseline performance result improves to +0.68% instructions. Suite 3 is
  closed; the cursor advances to `libs/any/test`.

## Next Commands

```sh
cd /Users/vishvananda/boost_1_91_0
env CPPGM_BOOST_B2_FRONTIER=1 \
  CPPGM_B2_CXX=/Users/vishvananda/cppgm-extended/dev/cppgm++ \
  CPPGM_B2_HOST_CC=/usr/local/opt/llvm/bin/clang \
  CPPGM_B2_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  python3 /Users/vishvananda/cppgm-extended/scripts/run_boost_b2_suite_survey.py \
  --suite 5 \
  --jobs 8 \
  --timeout 1800 \
  --output-dir /tmp/boost-frontier-v2-suite-005-initial-forced
```
