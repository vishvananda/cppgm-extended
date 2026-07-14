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
- active compiler frontier: typed non-type template argument evaluation cannot
  resolve `::boost::mpl::aux::nested_type_wknd<T1>::value` while compiling
  Accumulators `error_of`

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

## Suite Cursor

The inventory order comes from `docs/boost-b2-suite-status-20260511.md`. Add a
row when a suite is attempted. Do not prepopulate passes from V1.

| # | Suite | V2 status | Commit | Forced run evidence | Notes |
|---:|---|---|---|---|---|
| 1 | `libs/accumulators/test` | frontier | `(unused default lazy fix)` | Forced single-job `count` rebuild passed end to end and updated 72 targets; log `/tmp/boost-v2-acc-count-typed-member-pointer-fix.log`. Focused `covariance` passed after preserving the typed bound member-alias head and updated 72 targets; log `/tmp/boost-v2-acc-covariance-bound-template-identity-b2-j4.log`. The bound-template full survey updated 154 targets and failed 28; log `/tmp/boost-v2-suite-001-bound-template-identity-full.log`. A forced focused `extended_p_square` run reproduced the Boost.Random member-definition failure; log `/tmp/boost-v2-acc-extended-p-square-member-definition-focused.log`. After the fix, the forced focused target cleared Boost.Random and advanced to the alias failure; log `/tmp/boost-v2-acc-extended-p-square-parenthesized-noexcept-fix.log`. The lazy-instantiation fix's forced focused run passed compile, link, and runtime with 72 updated targets; log `/tmp/boost-v2-acc-extended-p-square-unused-default-lazy-fix.log`. Its forced full survey reached target 200 and 34 passing tests before the 1800s harness timeout; a non-forced continuation completed the graph with 12 updated and 10 failed targets. Logs: `/tmp/boost-frontier-v2-suite-001-unused-default-lazy-fix/libs__accumulators__test.log` and `/tmp/boost-v2-suite-001-unused-default-lazy-fix-resume.log`. | The apparent `check_frwd` failures were downstream diagnostics from eager instantiation of an unused Boost.Iterator default. `extended_p_square`, `extended_p_square_quantile`, and `weighted_extended_p_square` now pass. The remaining ten targets split into five MPL non-type evaluation failures, two ambiguous-base failures, and three runtime failures. The next ordered target is `error_of`. |

Allowed statuses are `pending`, `running`, `frontier`, `blocked-external`, and
`pass`. A timeout is evidence, not a pass.

## Active Frontier

- suite: `#1 libs/accumulators/test`
- focused target: `libs/accumulators/test//error_of`
- failure phase: template resolution while evaluating a non-type template
  argument
- diagnostic: failed non-type template argument evaluation for
  `::boost::mpl::aux::nested_type_wknd<T1>::value` in
  `boost::mpl::aux::or_impl<false, ...>`
- reduced repro: pending reduction outside the assignment tree
- owning PA/cluster: pending reduction; place at the earliest PA owning the
  essential member constant-evaluation behavior
- implementation area: typed dependent member constant evaluation and class
  completion
- performance risk: normal template-resolution path; inspect hotspot
  counters and sample if the fix moves instructions by more than 0.25%
- next action: rerun `error_of` with one job, reduce
  `nested_type_wknd<T1>::value` without Boost or the STL, and trace the typed
  member value through substitution, class completion, and constant evaluation

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

## Next Commands

```sh
cd /Users/vishvananda/boost_1_91_0
env CPPGM_BOOST_B2_FRONTIER=1 \
  CPPGM_B2_CXX=/Users/vishvananda/cppgm-extended/dev/cppgm++ \
  CPPGM_B2_HOST_CC=/usr/local/opt/llvm/bin/clang \
  CPPGM_B2_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  JOBS=1 \
  /usr/local/bin/timeout 900 \
  ./run-cppgm-b2.sh -a libs/accumulators/test//error_of
```
