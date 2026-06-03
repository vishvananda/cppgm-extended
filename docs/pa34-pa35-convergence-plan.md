# PA34/PA35 Convergence Plan

## Goal

Make the full `pa34` and `pa35` test sets work without turning hosted
compatibility into a dumping ground for earlier language bugs.

`pa34` and `pa35` are discovery gates:

- `pa34` exposes hosted header/source compatibility failures.
- `pa35` exposes hosted header-emission, link, symbol, ABI, and runtime
  failures.

The repair gate is still the earliest owning assignment. Standard-language
bugs discovered through hosted/STL tests should be reduced and backported to
the strict PA that owns the feature whenever possible. Hosted-only and
vendor-extension failures may stay in `pa34`; emitted-code/link/runtime
failures that first require hosted linking may stay in `pa35`.

## Reference Inputs

- Roadmap ownership: `ROADMAP.md`
- Template standard source: `doc/n3485.txt`
- Template-bank ownership map:
  `validation/templates/ASSIGNMENT_SLICES.md`
- Current PA34 contract: `pa34/README.md`
- Current PA35 contract: `pa35/README.md`
- Strict template owners: `pa18`, `pa19`, `pa21`, `pa22`

## Phase 0: Establish The Baseline

1. Generate patched-Clang witness references for C++ translation-unit tests in
   `pa34` and `pa35`:

   ```sh
   python3 validation/templates/materialize_pa_tests.py pa34 pa35
   ```

   The generated `.ref.witness` files are debugging references for this phase.
   They are not a requirement that `pa34`/`pa35` reach full witness parity now.
   The generator intentionally skips preprocessor-only tests and expands
   `pa35/tests/link/*.t` manifests to their real numbered translation units
   (`*.t.1`, `*.t.2`, ...), writing adjacent `*.t.N.ref.witness` files.
   Warnings from the generator are triage input: classify whether they are
   expected negative tests, patched-Clang gaps, or test/header environment
   problems.

   Initial generator audit:

   - The first pass incorrectly invoked PA35 link-test manifests as C++
     inputs. The generator now expands those manifests to their numbered
     translation units.
   - Running the same failed inputs with regular Homebrew Clang produced the
     same failures, so the remaining missing references are not patched-Clang
     regressions.
   - After fixing link-test handling, the PA35 witness gap is one
     Clang-incompatible source,
     `pa35/tests/link/645-builtin-operator-new-delete-link-smoke.t.1`, which
     redeclares Clang builtin `__builtin_operator_new`.
   - PA34 preprocessor tests are intentionally skipped. They are
     preprocessor-output fixtures, not C++ translation units.
   - The generator now follows the harness default `gnu++11` mode and honors
     per-test `.compile.flags`. Four valid post-C++11 parser-concession tests
     carry explicit language-mode flags for witness generation:
     `522`, `523`, and `524` use `-std=gnu++17`; `HHC-334` uses
     `-std=gnu++14`.
   - Current witness coverage after that cleanup:
     PA34 has 306 expected-success C++ translation-unit candidates, 302 with
     witness refs, and four missing Clang witness refs. PA35 has 98/98 link
     translation units covered. The four PA34 gaps are hosted/compiler-extension
     or target-specific surfaces that stock patched Clang cannot parse under a
     suitable local invocation: `557`, `598`, `729`, and `746`. Keep them in
     the failure ledger if they become useful for debugging, but do not treat
     missing Clang witness files for those tests as a PA34/PA35 correctness
     blocker.
   - Invalid pre-repair versions of the six PA34 sources that needed
     source-level cleanup are preserved under
     `validation/templates/deferred_negative_inputs/pa34-invalid-witness-sources`.
     They are not active tests. After PA34/PA35 convergence, use them as seeds
     for minimal negative regressions if `cppgm++` still accepts the invalid
     forms.

2. Collect the active failure set:

   ```sh
   make test-report ACTIVE_TEST_REPORT_PAS='pa34 pa35' \
     CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

   Use `test-report` as the authoritative PA34/PA35 progress view. It should
   record compile, LowIR, link, runtime, and exit-code behavior. Witness
   compares are a debugger and classifier, not the main gate for this phase.
   Strict semantic fallback rejection, lazy hosted-header function bodies, and
   lazy class-template references are now the default compiler behavior. If this
   mode exposes additional failures, refresh this baseline before continuing
   reductions rather than fixing against an older permissive failure list.

3. Create and maintain a failure ledger in this document or a follow-up table.
   Each row should record:

   - failing test
   - symptom: preprocessor, compile, LowIR, link, runtime, exit-code, timeout,
     or witness-only
   - whether `cppgm++` witness output is correct, missing, or divergent
   - root cause once known
   - earliest owning PA
   - `doc/n3485.txt` clause if it is a standard-language gap
   - regression test path
   - fix commit
   - validation command

## Failure Workflow

For each failure:

1. Reproduce it directly with the smallest assignment harness command.
2. Determine whether the visible symptom is the bug or only fallout.
3. Use witness output to classify the semantic state:
   - correct witness with incorrect compile/link/runtime output is high
     priority, because semantic selection may be correct while lowering,
     materialization, symbol ownership, or ABI emission is wrong.
   - divergent witness plus bad output usually means semantic/template
     analysis is wrong or incomplete.
   - witness-only drift in `pa34`/`pa35` should be noted but is not a blocking
     target in this phase unless it hides a compile/link/runtime bug.
4. Reduce away hosted/STL dependencies when possible.
5. Place the minimal regression at the earliest owner.
6. Fix implementation in `dev/`.
7. Re-run the owner strict suite and the PA34/PA35 focused report.

Do not encode host ABI, compiler, or library selection into the regression
test. It is fine if a test only reproduces in one compiler/host-ABI setup, but
the test source itself should be environment-agnostic unless the feature being
tested is explicitly hosted/vendor behavior.

## Baseline: 2026-04-29 PA34/PA35 Report

Command:

```sh
make test-report ACTIVE_TEST_REPORT_PAS='pa34 pa35'
```

Result: `183 / 273 TESTS PASSED`.

This is the initial permissive baseline. The next baseline refresh should use
`CPPGM_STRICT_SEMANTIC_FALLBACKS=1` and explicit Homebrew clang settings as
specified in Phase 0, because the convergence process now treats newly exposed
strict fallback failures as first-class compile failures.

Checkpoint status before the next strict baseline refresh:

- Current commit boundary contains the already reduced PA15/PA16/PA18/PA21/PA26
  fixes and regressions for friend `noexcept`, builtin delete `noexcept`,
  dependent anonymous member lookup, scoped-enum functional casts in template
  bodies, empty class-template-id argument resolution, and explicit
  instantiation argument syntax. It also includes strict fallback cleanup for
  partial-specialization pattern matching and node-aware template-id qualifier
  lookup exposed while validating those reducers.
- The permissive PA34 frontier moved past the original `540`/`542` failures.
  Strict mode then exposed the next text-reparse dependency in the hosted
  `__to_address_helper<_Pointer>` / `decltype(...)` path. That is the next
  reduction target after this checkpoint, not part of this commit boundary.
- Checkpoint validation passed with `CPPGM_STRICT_SEMANTIC_FALLBACKS=1` and
  Homebrew clang settings: `pa18 pa19 pa21 pa22` strict owner suite plus focused
  PA15, PA16, and PA26 reducer checks.

Strict hosted frontier updates after that checkpoint:

- `pa34/tests/compile/540-reference-wrapper-smoke.t` exposed a structured
  qualified template-id call gap in `decltype(AddressHelper<Pointer>::call(...))`.
  The fix makes qualified function-template lookup consume the AST node and its
  template-id syntax instead of resolving the qualifier from lookup text.
  Regression: `pa22/tests/general/485-dependent-decltype-qualified-template-id-call.t`.
- The next strict failure was a dependent non-type expression in a functional
  cast type, reduced from libc++ `_Rp != 0`. The fix keeps the body checker and
  overload functional-cast path on `lookup_type_node`, preserving template
  argument syntax for `integral_constant<bool, _Rp != 0>`.
  Regression: `pa19/tests/general/199-dependent-nontype-functional-cast-body-check.t`.
- The following hosted failure was injected current-class functional-cast lookup
  in `duration(duration_values<rep>::zero())`. The body checker now treats the
  current class name as a known type while checking template bodies.
  Regression: `pa18/tests/general/228-current-class-functional-cast-body-check.t`.
- The next strict fallback was `typename common_type<_Rep1, _Rep2>::type` inside
  a class-template partial specialization. The structured type lookup now
  chooses member-type lookup based on the resolved qualifier target: class scopes
  use semantic member lookup, including inherited typedefs, while namespace
  scopes use direct namespace named-type lookup. This avoids text fallback for
  inherited member typedefs.
  Regression: `pa22/tests/general/486-dependent-qualified-inherited-member-type.t`.
- After those fixes, the current `540` frontier moved to an unrelated template
  body-check failure in libc++ `istreambuf_iterator`: nested class `__proxy` is
  not recognized as a type in a member function body returning `__proxy(...)`.
  That reduced to PA18. The template body checker now collects nested member
  type names before validating member bodies.
  Regression: `pa18/tests/general/229-nested-member-type-functional-cast-body-check.t`.
- The next hosted body-check failure was an unqualified call to member function
  template `__on_zero_shared_impl()` from another member body. The same member
  collection now descends through `template-declaration` members so member
  function templates are visible during early template-body validation.
  Regression: `pa18/tests/general/230-member-template-call-body-check.t`.
- After those PA18 body-check fixes, the current `540` frontier moved to
  template argument mangling for a dependent type in libc++ `__hash_table`:
  `legacy dependent template argument text reached ABI mangler:
  std::__1::__hash_node<typename _Tp, __alloc_traits::void_pointer>`.
  The fix preserves structured type arguments for dependent aliases and
  dependent class-template instantiations so ABI mangling does not reparse
  dependent template-argument text. Alias preservation is deliberately narrow:
  normal semantic/SFINAE paths still defer dependent aliases, while the
  reference-only class-template collection path may keep the structured alias
  target only for all-type-parameter aliases.
  Regression: `pa33/tests/spec/231-host-dependent-alias-template-mangling.t`.
  Validation: strict `pa18 pa19 pa21 pa22` owner suite plus focused PA19/PA22
  SFINAE checks and the new PA33 reducer.
- The current `540` frontier is now unrelated type template argument
  resolution in libc++ `tuple_cat`:
  `failed type template argument resolution: tuple<>` in
  `__tuple_cat<tuple<>, __index_sequence<>, _T0Indices>()(...)`.
  That reduced to PA22 alias pack-substitution timing: reference-only
  collection was trying to expand a dependent alias template with a parameter
  pack through rewritten alias-target text, losing structured syntax for the
  pack-expanded alias arguments. The fix keeps parameter-pack aliases on the
  dependent-alias path during reference-only collection; the previous
  structured alias-target preservation remains limited to all-type,
  non-pack aliases.
  Regression:
  `pa22/tests/general/487-dependent-pack-alias-target-empty-template-id.t`.
  Validation: strict `pa18 pa19 pa21 pa22` owner suite plus focused PA19/PA22
  SFINAE checks, the PA33 mangling reducer, and the new PA22 reducer.
- The current `540` frontier is now strict template-id text fallback in
  libc++ tuple constraints:
  `semantic fallback disabled [category template-id-text-parse-fallback] type
  lookup parsed template-id text without structured syntax [name
  __enable_if_t<typename _Pred::value>]`.
  That reduced to PA22 function-template result instantiation: a dependent
  function-template instantiation was forcing result-type recovery even though
  the result still depended on template parameters, which reparsed a
  pack-expanded alias result through text. The fix skips result-type recovery
  for dependent function-template instantiations; non-dependent instantiations
  still recover/collapse bound result types.
  Regression:
  `pa22/tests/general/488-dependent-function-result-pack-alias-no-recovery.t`.
- The current `540` frontier is now ABI mangling for a dependent non-type
  builtin trait argument:
  `legacy dependent non-type template argument text reached ABI mangler:
  __is_nothrow_constructible(_Tp)`.
  That was reduced and fixed before the current checkpoint.
- After the qualified template type reducers, `540` exposed an out-of-class
  member-template definition whose parameter list uses a typedef inherited from
  a base class, reduced from libc++ `messages<_CharT>::do_get(catalog, ...)`.
  Template declarator parsing now asks the structured template type service for
  class member-type lookup, including inherited typedefs, instead of seeing only
  direct `named_types`.
  Regression:
  `pa18/tests/spec/123-out-of-class-template-member-inherited-typedef-param.t`.
  Validation: strict `pa18 pa19 pa21 pa22` owner suite.
- The next `540` frontier was special-member-template binding for libc++
  `vector<bool, _Allocator>::vector(_InputIterator, _InputIterator, const
  allocator_type&)`: collection found constructor-template candidates but did
  not match the out-of-class special-member definition with the inherited
  `allocator_type` parameter and extra SFINAE non-type template parameter. The
  fix compares owner-prefix and member-template suffix parameter lists
  separately and resolves dependent qualified member aliases through structured
  owner type state. The witness side also records the source-spelled dependent
  `shell<bool, Alloc>::allocator_type` owner use with canonical template
  parameter bindings instead of changing the test source.
  Regression:
  `pa22/tests/general/491-out-of-class-partial-owner-ctor-using-alias.t`.
  Validation: focused PA22 lowir/witness compare, old compiler lowir mismatch,
  strict `pa18 pa19 pa21 pa22` owner suite.
- The current `540` frontier moved past the `vector<bool>` constructor binding
  and is now unrelated template body collection in libc++ `moneypunct`: the
  body checker reports `unknown id-expression symbol` for the `money_base`
  pattern enumerators in `pattern __p = {{symbol, sign, none, value}}`.
  That reduced to PA18. The template body checker now seeds visible names from
  typed nondependent base-class reference metadata, including inherited
  unscoped enumerators, instead of only looking at the current class AST.
  Regression:
  `pa18/tests/general/237-template-body-inherited-enumerator.t`.
- The `540` performance tranche later recovered compile time and the refreshed
  strict PA34/PA35 baseline no longer lists `540` as a failing hosted compile
  test. The next convergence frontier should therefore start from the current
  strict baseline below rather than the older `540`-only notes.

## Baseline: 2026-04-30 Strict PA34/PA35 Report

Command:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-report ACTIVE_TEST_REPORT_PAS='pa34 pa35' \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Original result: `198 / 273 TESTS PASSED`.

Refresh after commit `b48ebc7e`: `207 / 273 TESTS PASSED`.

Refresh after commit `a62f7fd4`: `227 / 273 TESTS PASSED`.

Fixed after this baseline:

- `pa34/tests/compile/532-anonymous-enum-template-argument.t` reduced to PA19:
  unscoped enum constants are valid converted constant expressions for
  integral non-type template parameters even when the enumerator comes from an
  anonymous enum. Regression:
  `pa19/tests/general/202-anonymous-enum-nontype-template-argument.t`.
- `pa34/tests/compile/542-local-functor-std-function-assignment.t` first failed
  while completing libc++ `aligned_storage`: `alignas(_Align)` was parsed as a
  type-id even though `_Align` is a known non-type template parameter. The
  parser now prefers expression parsing for `alignas(...)` operands that start
  with a known value name. Regression:
  `pa18/tests/general/239-template-alignas-nontype-argument.t`. The hosted test
  still remains in the failure list below because it now reaches a later
  `_Rp(_ArgTypes...)` type-template-argument failure.
- The next `542` subfailure was completing libc++ `function<_Rp(_ArgTypes...)>`:
  function-type text resolution treated `_ArgTypes...` as one type argument
  instead of expanding the bound type pack while resolving the out-of-class
  constructor owner `function<_Rp(_ArgTypes...)>`. Regression:
  `pa18/tests/general/240-function-type-pack-out-of-class-constructor.t`. The
  hosted test still remains in the failure list below because it now reaches
  `std::function` assignment operator-template viability.
- The next `542` subfailure was libc++ `_And<true_type, true_type>::value`:
  expanding `__enable_if_t<Pred::value>...` substituted the expression text but
  left the AST qualifier as the original type-pack name, so member-value lookup
  could not evaluate the concrete `integral_constant::value`. Regression:
  `pa22/tests/general/493-pack-expanded-enable-if-member-value.t`. The hosted
  test still remains in the failure list below because it now reaches a later
  strict structured type-resolution fallback in `__member_pointer_class_type`.
- The next `542` subfailure was libc++ member-pointer helper SFINAE:
  `typename __member_pointer_class_type<_DecayFp>::type` is a structured
  qualified-template member type lookup that can legitimately have no `type`
  member for non-member-pointer functors. Structured lookup now reports that as
  a no-match instead of falling back to text. Regression:
  `pa22/tests/general/494-qualified-template-member-type-sfinae.t`. The hosted
  test still remains in the failure list below because it now reaches the
  underlying `std::function` assignment operator-template viability failure.
- The next `542` subfailure was `std::function` assignment viability for a
  local functor. The fix teaches typed leaf call analysis to instantiate viable
  function-template calls without text fallback, resolves dependent builtin
  type transforms such as `__remove_reference_t`/`__decay`, preserves concrete
  constexpr member values after template finalization, and records named
  function-local classes as a semantic fact for witness rendering. Regressions:
  `pa22/tests/general/495-function-assignment-invocable-and-helper.t`,
  `pa22/tests/general/496-dependent-remove-reference-transform-forwarding.t`,
  and `pa22/tests/general/497-dependent-builtin-decay-transform-return.t`.
  Validation: focused `542` hosted compile and strict
  `pa18 pa19 pa21 pa22` owner suite.
- `pa34/tests/compile/601-gnu-complex-template-constructor.t` is a PA34 hosted
  GNU extension case: `_Complex float` appears as a structured multiword
  builtin decl-specifier inside a function-template parameter clause. The
  template type-lookup request builder now permits exact identifier-word type
  names so direct semantic lookup can resolve builtin spellings such as
  `_Complex float` without parsing arbitrary type text. Validation: focused
  `601` hosted compile and strict `pa18 pa19 pa21 pa22` owner suite.
- `pa34/tests/compile/606-using-if-exists-missing-target.t` is a PA34 hosted
  Clang attribute case. A missing rooted target such as `::missing_name`
  should be ignored when `__using_if_exists__` is present, but diagnostic-only
  namespace probing dereferenced an empty qualifier list before the ignore path.
  The collector now returns for `using_if_exists` before error diagnostics.
  Validation: focused `606` PA34 harness check and strict
  `pa18 pa19 pa21 pa22` owner suite.
- The first `609-regex-iterator-difference-alias.t` subfailure was a PA19
  dependent non-type template argument ABI-mangling gap. Libc++ declares
  `__deque_iterator` with parameter name `_BS` but later defines members with
  `_BlockSize`; the dependent class-template metadata is positional, so the
  mangler must emit the template-parameter index for an identifier-valued
  dependent non-type argument even when the declaration-local names differ.
  Regression: `pa19/tests/general/203-dependent-nontype-template-arg-mangle.t`.
  The hosted `609` test remains in the failure list below because it now reaches
  a separate `basic_regex::__get_grammar` lookup failure.
- The next `609` subfailure was libc++ `basic_regex::__get_grammar(__flags_)`:
  template-body validation rejected the unqualified callee before overload
  resolution could use ADL from the enum-typed `__flags_` argument. Enum types
  now retain their declaring scope for ADL, and the template body checker skips
  the callee only when structured argument types find a concrete ADL candidate.
  Regression: `pa18/tests/general/241-template-body-enum-adl-call.t`. The
  hosted `609` test remains in the failure list below because it now reaches a
  separate `basic_string<const char, ...>` template argument resolution failure
  for `typename traits_type::char_type`.
- `pa34/tests/compile/610-random-to-address-qualified-call.t` reduced to PA19:
  early template-body call checking must not force dependent current-class
  static constants while only deciding whether an id-expression names a type.
  Regression:
  `pa19/tests/general/204-current-class-static-member-nontype-argument-body-check.t`.
- `pa34/tests/compile/627-anonymous-allocator-traits-pointer.t` reduced to
  PA21: current-specialization lookup must compare both canonical and display
  names so anonymous-namespace template arguments do not fall back to parsing
  template-id text. Regression:
  `pa21/tests/general/476-current-specialization-display-name-member-alias.t`.
- `pa34/tests/compile/644-unnamed-nested-enum-allocator-pointer.t` reduced to
  PA21: partial-specialization matching must resolve nested template-id pattern
  arguments from their carried syntax, and class-template reference code must
  stabilize caller-owned syntax before recursive specialization selection.
  Regression:
  `pa21/tests/general/477-partial-specialization-nested-template-id-pattern.t`.
- `pa34/tests/compile/654-string-compare-prefix-substr.t` reduced to PA18:
  enum operands legitimately trigger operator overload lookup through ADL, but
  a non-viable user operator candidate must not prevent builtin enum/integral
  arithmetic from being selected. Regression:
  `pa18/tests/general/242-enum-operator-template-fallback-to-builtin.t`.
- `pa34/tests/compile/655-const-unordered-map-find.t` reduced to PA22:
  `enable_if`-style default function-template parameters may evaluate a
  qualified static constexpr member function-template call after a defaulted
  type parameter is bound. Leaf constexpr evaluation now instantiates the
  structured function-template callee directly and does not reinterpret known
  function-template calls as type-initializer syntax. Regression:
  `pa22/tests/general/499-qualified-member-template-call-enable-if.t`.
- The follow-up `655` strict hosted frontier exposed carried dependent
  class-template argument replay through libc++'s
  `__hash_map_iterator<typename __hash_table<...>::iterator>`. The carried
  structured type was available, but dependent class-template instantiation
  rebuilt arguments from text and hit the internal `$dqmember` identity
  spelling. Dependent class-template carried-syntax replay now resolves
  all-type carried arguments structurally before the text fallback. The active
  reproducer for this slice is the hosted PA34 `655` test; no smaller non-STL
  reducer has isolated the same internal carried-identity path yet.
- The follow-up strict hosted frontier exposed structured template-argument
  syntax holes while validating `540`, `609`, and `656`. Template-argument
  resolution cache entries are now keyed by both expanded text and carried
  syntax fingerprint so one syntactic form cannot poison another with the same
  spelling. Function-template explicit argument resolution preserves syntax
  through fixed/pack splitting, constant functional casts use node-aware type
  lookup, and witness nested class-use emission consumes carried template-id
  syntax before falling back to source-location scanning. Regression:
  `pa22/tests/general/500-cv-qualified-qualified-type-template-arg.t`.
  Validation: focused `540`, `609`, and `656` hosted compiles plus strict
  `pa18 pa19 pa21 pa22` owner suite.
- The next `542-local-functor-std-function-assignment.t` subfailure was the
  libc++ internal type-transform alias `__remove_cvref_t` inside
  `_And<_IsNotSame<__remove_cvref_t<_Fp>, function>, ...>`. The concrete unary
  transform resolver now recognizes libc++'s internal transform aliases such as
  `__remove_cvref_t` directly, so SFINAE type arguments resolve through carried
  template-id syntax instead of falling back to nested template-id text parsing.
  Regression:
  `pa22/tests/general/501-internal-remove-cvref-alias-sfinae.t`.
  Validation: focused `542` hosted compile.

Additional fixes after the `a62f7fd4` ledger:

- `pa34/tests/compile/540-reference-wrapper-smoke.t` exposed a rewritten
  default template argument syntax bug. The fallback rewrote `_Clock::duration`
  to the concrete `std::__1::chrono::system_clock::duration` text but kept the
  stale original AST, so strict structured resolution failed before the
  rewritten spelling could be used. Rewritten default type arguments now carry
  matching substituted syntax when available. Validation: focused `540` hosted
  compile and strict `pa18 pa19 pa21 pa22` owner suite. Commit: `6a08a28b`.
- `pa34/tests/compile/542-local-functor-std-function-assignment.t` then
  exposed `typeid(_Fp)` where `_Fp` is a bound template type parameter inside
  libc++ `std::function`. `typeid` now mirrors `sizeof` disambiguation: an
  identifier operand with no value binding is first resolved as a structured
  type operand before expression analysis. Regression:
  `pa27/tests/spec/131-typeid-template-type-parameter.t`. Validation: focused
  PA27 regression, focused `542` hosted compile, and strict
  `pa18 pa19 pa21 pa22` owner suite. Commit: `d6626dda`.

## Baseline: 2026-05-01 Strict+Lazy PA34/PA35 Report

Command:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
CPPGM_LAZY_HEADER_FUNCTION_BODIES=1 \
CPPGM_LAZY_CLASS_TEMPLATE_REFERENCES=1 \
make test-report ACTIVE_TEST_REPORT_PAS='pa34 pa35' \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Result after commit `d6626dda`: `253 / 276 TESTS PASSED`.

Active PA34 compile exit-status failures:

- `pa34/tests/compile/656-forward-array-string-pair.t` - unclassified compile
  exit failure after the previous carried-syntax fixes.
- `pa34/tests/compile/665-hosted-pointer-traits-pair-pointer-to.t` -
  unclassified compile exit failure.
- `pa34/tests/compile/676-hosted-algorithm-copy-n-using-directive.t` -
  unclassified compile exit failure.
- `pa34/tests/compile/721-hosted-unreachable-inline-callee-export-closure.t` -
  unclassified compile exit failure.
- `pa34/tests/compile/725-hosted-piecewise-pair-index-sequence-alias.t` -
  unclassified compile exit failure.
- `pa34/tests/compile/736-hosted-local-class-distinct-member-symbols-compile.t`
  - unclassified compile exit failure.

Active PA35 implementation/link/runtime exit-status failures:

- `pa35/tests/link/651-hosted-unordered-map-string-int-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/652-hosted-unordered-set-pointer-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/654-hosted-forward-as-tuple-rvalue-ref-runtime.t` -
  implementation exit failure.
- `pa35/tests/link/695-hosted-std-vector-pair-abi-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/698-hosted-template-angle-vector-pair-substitution-link-smoke.t`
  - implementation exit failure.
- `pa35/tests/link/702-hosted-local-wide-string-return-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/705-hosted-inline-header-odr-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/710-hosted-set-insert-count-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/724-hosted-vector-class-brace-init-ref-capture-runtime-smoke.t`
  - implementation exit failure.
- `pa35/tests/link/727-hosted-vector-char-assign-initlist-runtime-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/729-hosted-unreachable-inline-callee-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/733-hosted-map-find-iterator-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/735-hosted-map-subscript-piecewise-construct-link-smoke.t` -
  implementation exit failure.
- `pa35/tests/link/741-hosted-deque-move-assign-link-smoke.t` -
  implementation exit failure.

## Earliest Owner Mapping

Use this mapping before leaving a failure in `pa34` or `pa35`.

- `pa18`: first-tier class/function templates, type and template-template
  parameters, type packs, defaults, basic deduction, member templates,
  on-demand instantiation, and template-backed overloads over the already
  supported object model.
- `pa19`: integral non-type template parameters and arguments, integral
  constant-expression template arguments, explicit specialization, supported
  template `static_assert`, and practical dependent constant/value bindings.
- `pa21`: alias templates, variable templates, class/function partial
  specialization, specialization selection, explicit instantiation, and the
  deterministic template entity graph.
- `pa22`: full function-template deduction over the intended C++11 surface,
  non-deduced contexts, substitution failure, `enable_if`/`void_t`/detected
  idiom behavior, dependent calls/aliases, and no-eager-instantiation cleanup.
- `pa34`: hosted preprocessor compatibility, GNU/Clang parser concessions,
  hosted builtin traits/intrinsics, vendor header source patterns, and
  compile-only hosted compatibility that is not a missing standard-language
  feature from earlier PAs.
- `pa35`: hosted emitted definitions, inline/template emitted-code correctness,
  host link/runtime behavior, symbol ownership, ABI spelling, and hosted
  library behavior that only fails once code is linked or executed.

When a template-related hosted failure aligns with `doc/n3485.txt`, prefer a
small non-STL `tests/spec` regression in the mapped strict PA. If the standard
rule is already covered but the hosted form still fails because of vendor
extensions or header shape, keep a focused hosted regression in `pa34` or
`pa35`.

## Regression Requirements

Every backported strict regression should have:

- a minimal source file that does not depend on STL headers when a non-STL
  reducer is possible
- a LowIR/compile/exit oracle appropriate for the owner PA
- a strict witness oracle when the owner PA participates in template witness
  strict checking
- a short source comment or test name that identifies the feature being pinned
- an `N3485 focus` comment when the test is directly tied to a standard clause

For new strict tests, validate both observable output and witness output. A fix
is not complete if the new test compiles but strict witness output drifts.

## Validation Gates

Per fix:

```sh
make -C dev cppgm++ \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22 pa23' STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
make test-report-nobuild ACTIVE_TEST_REPORT_PAS='pa34 pa35' \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
python3 scripts/audit_template_boundary.py
python3 scripts/audit_semantic_template_boundary.py --strict
python3 scripts/audit_text_reparse.py
git diff --check
```

Use narrower targeted commands while reducing, but do not commit a fix that
introduces LowIR, exit-code, or new strict witness drift in the owner suite.
Do not add "try structured, then parse/rewrite text" fallbacks to make a hosted
test pass. If a direct structured parse fails but a nearby whole-declarator parse
succeeds, fix the structured parse data path so both consumers see the same typed
state.

## Iteration Order

1. Generate the PA34/PA35 witness references.
2. Run the PA34/PA35 `test-report` and record the failure set.
3. Pick failures in earliest-frontier order, but batch obviously related
   symptoms after confirming they share one root cause.
4. For each root cause, add the earliest-owner regression before or alongside
   the fix.
5. Commit after the current failure family is fixed and validated.
6. Re-run PA34/PA35 reporting to confirm the failure set shrank before moving
   to an unrelated family.

## Progress Log

- Fixed PA34 `540-reference-wrapper-smoke` strict fallback on
  `tuple<__decay_t<_BoundArgs>...>` by preserving structured template argument
  syntax through alias-template pack expansion, selected class-template lookup,
  and dependent class-template metadata. Added PA21 regression
  `475-alias-template-pack-id-preserves-syntax.t`.
- Validation: focused PA21 text+witness check passed; strict owner suite
  `pa18 pa19 pa21 pa22` passed with `CPPGM_STRICT_SEMANTIC_FALLBACKS=1`.
- Fixed the next PA34 `540` frontier, unsupported
  `alignas(__alignof(_Tp))` while instantiating libc++ aligned-storage helpers.
  The parser was trying `alignas` operands as type-ids first, so GNU
  `__alignof(T)` was misclassified as a type named `__alignof` instead of a
  structured type-trait expression. The alignas operand parser now prefers
  expression parsing for known type-trait starts while preserving type-id
  handling for ordinary `alignas(T)`. Added PA18 regression
  `238-template-alignas-gnu-alignof-instantiation.t`.
- Fixed the PA34 `542-local-functor-std-function-assignment` subfailure where
  libc++ `_And` expands a bound type pack through an explicit function-template
  argument list (`__and_helper<_Pred...>`). The function-template explicit
  argument resolver now expands the explicit tail consumed by a template
  parameter pack before resolving each pack element. Added PA22 regression
  `492-explicit-function-template-pack-alias-sfinae.t`. The hosted `542`
  frontier then moved to a separate two-predicate `_And` value-evaluation
  failure.
- Fixed the follow-on `542` `_And` value-evaluation failure. Type-pack
  expression substitution now carries each concrete pack element as structured
  qualifier type syntax, letting `Pred::value` resolve through the instantiated
  `integral_constant` member scope without reparsing the substituted text. Added
  PA22 regression `493-pack-expanded-enable-if-member-value.t`. The hosted
  frontier then moved to a strict type-resolution fallback for
  `__member_pointer_class_type<_DecayFp>::type`.
- Fixed the `542` `__member_pointer_class_type<_DecayFp>::type` fallback.
  Qualified-template member type lookup now returns a structured no-match when
  the instantiated qualifier has no requested member type, preserving SFINAE
  instead of falling through to prohibited text recovery. Added PA22 regression
  `494-qualified-template-member-type-sfinae.t`. The hosted frontier then moved
  to the remaining `std::function` assignment operator-template viability
  failure.
- Fixed the PA34 `609-regex-iterator-difference-alias` frontier through the
  non-STL dependent type-transform reduction. Builtin cv-removal transforms are
  now preserved while their argument is dependent and resolved after
  substitution, and alias-template structural expansion no longer treats a
  pattern that still rewrites under instantiated bindings as concrete. Added
  PA22 regression
  `498-dependent-remove-cv-transform-alias-substitution.t`. The hosted `609`
  compile check now passes under strict fallback mode.
- Fixed the PA34 `610-random-to-address-qualified-call` frontier through a
  non-STL PA19 reduction. Template-body validation was resolving a qualified
  callee template-id solely to decide whether the call looked like a type
  construction, which forced evaluation of a dependent current-class static
  member used as an integral non-type template argument. The body checker now
  treats substitution-dependent lookup failures as inconclusive during that
  probe instead of aborting declaration collection. Added PA19 regression
  `204-current-class-static-member-nontype-argument-body-check.t`.
  Validation: focused `610` hosted compile and strict `pa18 pa19 pa21 pa22`
  owner suite.
- Fixed the PA34 `627-anonymous-allocator-traits-pointer` frontier through a
  non-STL PA21 reduction. Current-specialization lookup could only rebuild
  template-id names from canonical argument text, so anonymous-namespace display
  spelling made `Traits<Alloc<Block>>` miss the active class specialization and
  fall into prohibited template-id text parsing. Current-specialization lookup
  now also matches the active class's recorded qualified and display-qualified
  names before considering text fallback. Added PA21 regression
  `476-current-specialization-display-name-member-alias.t`.
  Validation: focused `627` hosted compile and strict `pa18 pa19 pa21 pa22`
  owner suite.
- Fixed the structured template-argument syntax propagation slice exposed by
  the next hosted frontier checks. Cache keys now distinguish carried syntax,
  explicit function-template arguments keep syntax through pack expansion, and
  witness nested class-use emission uses structured template-id arguments when
  available. Added PA22 regression
  `500-cv-qualified-qualified-type-template-arg.t`. Validation: focused `540`,
  `609`, and `656` hosted compiles with Homebrew clang host settings, plus
  strict `pa18 pa19 pa21 pa22` owner suite.
- Fixed the next `542` SFINAE frontier by recognizing libc++ internal unary
  type-transform aliases (`__remove_cvref_t` and siblings) in the concrete
  transform resolver. Added PA22 regression
  `501-internal-remove-cvref-alias-sfinae.t`. Validation: focused `542`
  hosted compile and strict `pa18 pa19 pa21 pa22` owner suite.
- Refreshed the strict PA34/PA35 baseline after the transform-alias fix:
  `233 / 273 TESTS PASSED`. The PA34 frontier moved to
  `657-getline-friend-lambda-access`.
- Fixed the `657` failure chain. Structured qualified constant lookup now
  enters enum/class scopes through the resolved qualifier type, so scoped enum
  constants in dependent constant expressions do not fall back to text lookup;
  added PA11 regression `300-scoped-enum-cast-constant.t`. Repeated collection
  of the same template-origin member definition is now idempotent, dependent
  qualified member aliases preserve their dependent semantic marker, and
  out-of-class member definitions are applied only to the target owner class
  rather than unrelated nested helper classes. Function exception-spec matching
  now compares explicit parse state before semantic values so dependent
  `noexcept` expressions do not mismatch declarations and definitions.
- Fixed the final `657` `basic_string::__init` overload failure. Function
  template collection now compares non-type template parameter `value_type`
  when deciding whether two same-signature member-template declarations are the
  same entity, preserving SFINAE-distinct overloads whose ordinary function
  parameter lists are identical. Explicit-call failures with dependent defaulted
  non-type `enable_if` parameters continue to report witness drop reason
  `substitution_failure`, matching the clang witness oracle. Added PA22
  regression `502-member-template-enable-if-redeclaration-overload.t`.
  Validation: focused `657` hosted compile, PA11 suite, and strict
  `pa18 pa19 pa21 pa22` owner suite.
- Fixed the PA34 `660-hosted-forward-as-tuple-rvalue-ref` frontier, also
  advancing `658-istream-static-member-mask-access` past its previous hosted
  lookup symptom. Function-template result substitution could leave a
  class-template-id return type such as `tuple<T&&...>` dependent even after
  pack deduction; the resolver now uses the carried dependent class-template
  argument syntax to expand and resolve the instantiated return type without
  parsing fallback text. The internal recovery path suppresses source class-use
  emission, and the local declaration alias witness hook no longer emits a
  deduced alias event when the source spelling is the direct class-template
  name already recorded as an explicit class-use. Added PA18 regression
  `243-function-template-pack-ref-return.t`. Validation: focused `660` and
  `658` hosted compiles now reach the next unrelated
  `__is_constructible(_Tp,_Args...)` non-type argument evaluation frontier, and
  strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the next PA34 builtin-trait frontier exposed by `658`/`660`.
  Dependent builtin trait expressions replayed while collecting
  `integral_constant<bool, ...>` reference members must stay dependent when the
  carried type-id AST still names template parameters or packs, even if the
  current instantiation scope has rebound the first non-type parameter to
  `bool`. The dependency checker now consults structured type-id syntax, and
  hosted `__is_base_of` has concrete semantic and template-side class metadata
  evaluation. Added PA34 regression
  `747-builtin-dependent-trait-integral-constant.t`. Validation: focused PA34
  regression check passed; focused `660` and `658` hosted compiles now reach
  the next unrelated `__is_invocable_r_v<_Ret, _Func, _Args...>` frontier; strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the follow-on hosted builtin trait replay slice. The dependency marker
  is now carried in structured template argument syntax, template-side
  evaluation supports the additional hosted traits needed by libc++ replay
  (`__has_virtual_destructor`, `__is_abstract`, `__is_literal_type`,
  `__is_polymorphic`, and `__array_rank`), and builtin type-trait expression
  result typing preserves integral-valued traits such as `__array_rank` instead
  of collapsing all trait results to `bool`. Added PA34 regression
  `748-hosted-dependent-builtin-trait-replay.t`. Validation: focused PA34
  regression check passed; focused `660` now reaches the tuple `__base_`
  member-object frontier and focused `658` reaches the `std::__to_unsigned_like`
  lookup frontier; strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the `658` `std::__to_unsigned_like` lookup frontier through a PA22
  non-STL reduction. Reference-only collection of a function-template return
  type could keep an alias-target member alias as `copy_cv<From>::apply<To>`
  after the outer function template only owned `T`, so instantiation retained a
  dependent return type and dropped the candidate. The collector now detects
  function-template result types that still carry unowned template parameters in
  structured dependent alias/member syntax and reparses that declarator without
  the reference-only class-template restriction. Added PA22 regression
  `503-dependent-member-alias-function-return.t`. Validation: focused PA22
  LowIR and witness checks, focused `658` hosted harness check, and strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the follow-on hosted tuple/reference mangling recursion exposed by
  `660`. Mangling an owner template argument such as `T&...` while emitting the
  owner template parameter `T` recursively substituted the same owner argument
  into itself. The ABI mangler now suppresses the active owner argument index
  while emitting that argument, so the nested parameter occurrence emits the
  template parameter index instead of re-expanding the owner argument. Added
  PA33 regression `234-host-owner-pack-reference-mangling.t`. Validation:
  focused PA33 regression check passed; focused `660` now reaches the unrelated
  tuple `__base_` member-object frontier.
- Fixed the `660` tuple `__base_` member-object frontier through a PA21
  partial-specialization reduction. Class partial-specialization matching could
  not structurally expand an alias-template pattern whose target used a
  non-type parameter pack, so `tuple_impl<index_sequence<I...>, T...>` failed
  to match the instantiated `tuple_impl<integer_sequence<unsigned long, 0>,
  string&&>` form and selected the incomplete primary. Alias pattern expansion
  now handles type and non-type alias parameters, substitutes non-type packs in
  carried class-template-id metadata, and allows dependent alias expansion only
  in the partial-specialization deduction path. Added PA21 regression
  `478-alias-nontype-pack-partial-specialization-pattern.t`. Validation:
  focused PA21 LowIR and witness checks passed; focused `660` now reaches the
  unrelated strict template-id fallback for
  `is_nothrow_default_constructible<basic_string&&>`; strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the follow-on `660` strict template-id fallback for
  `is_nothrow_default_constructible<T>::value`. A qualified-id whose qualifier
  is a class-template-id can be either a dependent type or a dependent
  member-value expression; template-argument syntax capture was keeping only
  the type interpretation, forcing non-type argument evaluation back through
  template-id text. The parser now preserves both structured interpretations
  for qualified template-id member references, so non-type argument resolution
  can evaluate the expression node directly. Added PA22 regression
  `504-qualified-template-member-value-argument-syntax.t`. Validation: focused
  PA22 LowIR and witness checks passed; focused `660` now reaches the unrelated
  `((void)false,true)` non-type argument text-evaluation frontier; strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the `((void)false,true)` frontier in libc++ `__all<false>`. Pack
  expansion was correctly rewriting the non-type argument text from
  `((void)B,true)...` to the concrete expression, but the expanded argument
  dropped its carried expression AST and then strict mode rejected raw text
  evaluation. Template-argument input expansion now preserves substituted
  expression syntax for expression-pack expansions. Added PA19 regression
  `205-nontype-pack-comma-expression-syntax.t`. Validation: focused PA19 LowIR
  and witness checks passed; focused `660` now reaches the unrelated strict
  fallback for `_BoolConstant<sizeof...(_Up)==sizeof...(_Tp)>`; strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the `_BoolConstant<sizeof...(_Up)==sizeof...(_Tp)>` frontier from
  `660` through a PA22 non-STL reduction. Constant-value lookup already had
  structured qualifier template-id syntax for `And<...>::value`; if the
  structured alias resolution still cannot prove a concrete class during an
  early dependency probe, it must stop and let dependency handling continue
  instead of reparsing the qualifier template-id as text. Non-type template
  parameters with structured decl-specifier syntax are now rechecked through
  the structured type parser before finalization so dependent defaulted
  `enable_if<And<...>::value, int>::type = 0` parameters stay viable during
  deduction. Added PA22 regression
  `505-defaulted-nontype-qualified-alias-value.t`. Validation: focused PA22
  LowIR and witness checks passed; focused `660` now reaches the unrelated
  `basic_string` static-assert frontier.
- Fixed the `basic_string` static-assert frontier from `660` through a PA22
  non-STL reduction. A dependent qualified member type whose owner is a bound
  template argument was using direct member-scope lookup, so inherited typedefs
  such as `traits_type::char_type` were invisible when the owner resolved to an
  explicit specialization with a templated base. Bound-owner qualified type
  lookup now uses the semantic concrete-type lookup helper for intermediate and
  final member types, preserving inherited-member lookup without falling back to
  text. Added PA22 regression
  `506-bound-owner-inherited-member-type.t`. Validation: focused PA22 LowIR
  and witness checks passed; focused `660` now reaches the unrelated tuple
  forwarding constructor viability frontier.
- Fixed the `660` tuple forwarding constructor viability frontier through a
  PA22 non-STL reduction. Template-side builtin trait replay now routes
  two-argument `__is_constructible`/`__is_nothrow_constructible` requests
  through the binary trait evaluator, and qualified leaf-value lookup now uses
  the typed class member hierarchy when the direct class scope does not contain
  the requested static member. This preserves inherited `integral_constant`
  values such as `is_constructible<T, U>::value` without text fallback. Added
  PA22 regression
  `507-defaulted-nontype-enable-if-constructible-ref.t`. Validation: focused
  PA22 LowIR/witness check passed, focused `660` hosted compile passed, strict
  `pa22` passed, and the latest PA34/PA35 report is `236 / 275 TESTS PASSED`.
  The current PA34 frontier is `635-nested-class-constructor-reentry.t`; PA35
  still has link/runtime failures starting at
  `651-hosted-unordered-map-string-int-link-smoke.t`.
- Fixed the PA34 `635-nested-class-constructor-reentry` frontier through a
  PA33 non-STL reduction. ABI mangling for a dependent non-type owner argument
  now treats text-only template-parameter arguments such as `B` as structured
  template-parameter expressions even when the formal parameter type is
  non-dependent, instead of reaching the legacy dependent-text error path.
  Added PA33 regression
  `235-host-dependent-bool-owner-argument-mangling.t`. Validation: focused
  PA33 host-interop check passed, focused `635` LowIR compile passed, and
  strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34 `662-hosted-ostringstream-unsigned-int` frontier through a
  PA19 non-STL reduction. Dependent class-template metadata recorded only the
  explicit source argument syntax, so a defaulted dependent non-type argument
  such as `is_enum<T>::value` was replayed later as text-only and tripped
  strict template-id fallback. Class-template instantiation state now preserves
  the full canonical argument text vector and fills missing syntax from the
  resolved `TemplateArgument.expression`. Added PA19 regression
  `206-defaulted-dependent-nontype-expression-syntax.t`. Validation: focused
  PA34 `662` LowIR compile passed, and strict `pa18 pa19 pa21 pa22` owner suite
  passed.
- Fixed the first PA34 `676-hosted-algorithm-copy-n-using-directive` frontier
  as a PA34 hosted extension reduction. libc++ uses a static member
  `operator()` on algorithm implementation objects; operator registration was
  incorrectly applying the non-member overloaded-operator class/enum operand
  rule inside class scope. Member operator registration now keeps that rule on
  non-members only. Added PA34 regression
  `749-static-call-operator-extension.t`. Validation: focused reducer LowIR
  compile passed, focused `676` advanced to an unrelated `std::make_pair`
  lookup failure, and strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the follow-on PA34 `676` `std::make_pair` lookup failure through a PA22
  non-STL reduction. Function-template result recovery was blocked whenever the
  instantiated function scope still had local template placeholders, even when
  the call's template arguments were concrete. That left helper return types
  such as `unwrap_range` dependent and caused later body binding to search for
  `make_pair` through the wrong result type. Result recovery now uses concrete
  template arguments independently from local placeholder state, while still
  skipping pack-expanded dependent alias results that must remain lazy; witness
  source-call capture is paused for the unevaluated result-type recovery probe.
  Added PA22 regression
  `508-dependent-function-result-recovery.t`. Validation: focused `676` hosted
  compile passed and strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34 `678-hosted-deque-member-template-include` frontier through a
  PA19 non-STL reduction. A defaulted non-type template argument expression was
  rewritten with preceding bound type arguments, but the expression AST was
  dropped whenever the text changed. That later forced member-value lookup for
  class-template defaults such as `__deque_block_size<T, D>::value` back through
  template-id text parsing. Defaulted non-type arguments now retain a
  substituted expression node, preserving structured template-id syntax after
  type-argument substitution. Added PA19 regression
  `207-defaulted-nontype-expression-syntax-rewrite.t`. Validation: focused
  `678` hosted compile passed and strict `pa18 pa19 pa21 pa22` owner suite
  passed.
- Fixed the PA34
  `720-hosted-function-template-default-allocator-local-lambda-compile`
  frontier through a PA22 non-STL reduction. Function-template deduction over a
  class-template parameter with a namespaced default type argument used a
  dependent text placeholder such as `Alloc<P>` instead of the structured
  `lib::Alloc<P>` class-template instantiation. That caused a viable overload
  like `push(lib::Vec<P>&)` to be dropped when the actual argument was the
  fully qualified instantiated type. Default type arguments now substitute and
  resolve the default-argument AST before falling back to deferred text, keeping
  the source class-template identity intact. Added PA22 regression
  `509-defaulted-class-template-arg-deduction.t`. Validation: focused `720`
  hosted compile passed and strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34
  `721-hosted-unreachable-inline-callee-export-closure` strict frontier through
  a PA22 non-STL reduction. Constant-expression reference discovery walked
  short-circuited operands after evaluating a non-type template argument; when
  the unreachable operand contained an invalid qualified alias-template member
  type such as `typename remove_const_ref_t<Arg>::first_type`, that witness-only
  walk re-entered type lookup as template-id text and tripped strict fallback.
  Leaf constexpr type-node lookup now tries structured qualified/template-id
  resolution before text lookup, and reference discovery still records valid
  short-circuited RHS uses while ignoring strict fallback only for unreachable
  invalid operands. Added PA22 regression
  `510-short-circuit-alias-member-sfinae.t`. Validation: focused `721` hosted
  compile passed and strict `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34
  `725-hosted-piecewise-pair-index-sequence-alias` frontier through a PA22
  non-STL reduction. Function-template deduction over an alias-template pattern
  with non-type packs expanded `index_sequence<I...>` only after deduction had
  failed, and the recovery path did not deduce the pack values from the alias
  target. Deduction now recognizes dependent alias arguments as references to
  unbound function-template parameters and can retry against a structurally
  expanded alias pattern while carrying the original template-argument syntax,
  so strict mode does not reparse libc++ helper aliases from text. Added PA22
  regression `511-index-sequence-alias-constructor-deduction.t`. Validation:
  focused PA22 regression check passed, related PA21 alias-pack regression
  passed, focused `725` hosted compile passed, and strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34
  `734-synthetic-self-referential-holder-compile` frontier through a PA18
  non-STL reduction. Completing an instantiated class template eagerly walked
  and fully materialized all direct named nested member classes; for
  self-referential owners this made an otherwise-unused nested class instantiate
  an object member whose type was still incomplete. Direct nested-member
  finalization now collects reference members and out-of-class definitions
  without forcing full completion, while semantic output preserves the old
  nested-definition scan order by marking unrequired nested definitions as
  visited rather than completing them. Nested classes with required member
  output still materialize and emit witness closure events. Added PA18
  regression `244-unused-nested-class-instantiation.t`. Validation: focused
  PA34 `734` compile passed, the PA18 reducer passed, PA22 nested-class
  ordering/witness regressions passed, and strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34
  `735-nonstl-qualified-templateid-pointer-return-compile` frontier through a
  PA18 non-STL reduction. Template-layer structured type lookup handled
  qualified template-id qualifiers as class/type scopes only, so a namespace
  qualifier in `Q::V<Q::V<int> *>` caused `Q::V<int> *` template-argument
  resolution to fail before the semantic resolver could run. Structured
  qualified-template lookup now resolves namespace qualifiers before class-type
  qualifiers. Added PA18 regression
  `245-qualified-template-id-pointer-argument.t`. Validation: focused PA18 and
  PA34 checks passed.
- Fixed the first PA34
  `736-hosted-local-class-distinct-member-symbols-compile` subfailure through a
  PA18 non-STL reduction. Instantiated function-template bodies use synthetic
  fragment source locations, so nested lookup during body emission was using
  `1:1` as the template-use point and incorrectly treating an already-prior
  explicit class-template specialization as after-instantiation. Function
  template instantiations now record the logical use location and replay it
  while emitting the instantiated body, so specialization-order checks see the
  real point of instantiation. Added PA18 regression
  `246-template-instantiation-use-location-explicit-specialization.t`.
  Validation: focused PA18 regression, local 736 reducers, and strict
  `pa18 pa19 pa21 pa22` owner suite passed. The hosted `736` frontier now
  reaches an unrelated ABI-mangling strict failure for
  `is_reference<__deref_t<_Iter>>::value`.
- Fixed the PA34
  `736-hosted-local-class-distinct-member-symbols-compile` ABI-mangling
  subfailure through a PA33 non-STL reduction. Dependent alias expansion for an
  `enable_if_t<is_reference<deref_t<Iter>>::value, int>` non-type template
  parameter reached the ABI mangler with structured argument syntax preserved,
  but `deref_t` expanded to `decltype(*declval<Iter&>())` and the structured
  dependent-expression mangler did not encode unary dereference. The mangler now
  handles unary dependent expressions directly, fails closed after rolling back
  partial alias-expansion output, and suppresses expression qualifier
  template-name registration for dependent `decltype` alias arguments to keep
  Clang-compatible substitution indexes. Added PA33 regression
  `236-host-dependent-enable-if-alias-expression-mangling.t` with a raw-symbol
  inspect check. Validation: focused PA33 regression and hosted PA34 `736`
  compile passed.
- Fixed the PA34 `745-class-alias-dependent-destroy-compile` frontier through
  a PA22 non-STL reduction. Qualified function-template call setup had
  structured syntax for the callee node, but the instantiation-use-scope path
  resolved the qualifier `Traits<ValueAlloc<Candidate>>` through plain text,
  forcing the nested `ValueAlloc<Candidate>` argument back through strict
  template-id parsing. `SemanticContext` now exposes structured
  qualified-scope resolution for callee nodes, and overload candidate setup uses
  it whenever qualifier template-id/type syntax is present. Added PA22
  regression `126-qualified-template-id-function-template-use-scope.t`.
  Follow-up witness audit found that the public closure renderer was exposing a
  cross-owner `ensure-definition` for non-constexpr ordinary member functions
  already represented by a `function-instantiation` event; lifecycle events now
  carry an explicit constexpr-function fact so constexpr allocator-style
  ensures remain visible while ordinary member ensures are suppressed. Added
  PA18 regression `122-cross-owner-member-function-instantiation-witness.t`.
  Validation: focused PA18/PA21/PA22/PA34 checks passed and strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the PA34 `610-random-to-address-qualified-call` strict-lazy frontier.
  The first subfailure was a no-eager-instantiation violation: a typedef naming
  a class-template specialization should not instantiate the class body.
  Typedef declaration parsing now keeps class-template references lazy, and the
  reduction lives in PA18 as
  `124-typedef-class-template-does-not-instantiate.t`. The later hosted
  `shuffle_order_engine` symptom was qualified constant lookup returning a
  reference-only dependent static member stub even though the qualifier class
  was ready to complete; qualified value lookup now completes that class and
  re-reads the member before returning the binding. Validation: focused PA18
  reducer, focused hosted `610` compile, and strict `pa18 pa19 pa21 pa22`
  owner suite passed.
- Fixed strict-lazy owner-suite fallout exposed while validating the `610`
  fixes. Constructor overload selection now completes the canonical constructor
  target before synthesizing implicit special members, avoiding stale
  constructor bindings when class completion resets a referenced instantiation.
  Nested member-class completion now stabilizes the owning class-template
  instantiation first, re-resolves the nested class from the completed owner
  scope, syncs layout back to the queried type, and applies stored
  out-of-class special-member definitions during nested finalization. Existing
  PA22 coverage includes
  `476-out-of-class-nested-template-owner-constructor.t`; focused PA22
  273/274/122/476 checks and the strict `pa18 pa19 pa21 pa22` owner suite
  passed.
- Fixed the PA34 `651-allocator-deallocate-included-class-layout` strict-lazy
  frontier. Lazy header-body skipping was using source-location file insertion
  order to decide whether a function body came from a header, so a primary
  source whose first emitted tokens came after an include was incorrectly
  treated as non-primary. The token stream now exposes the primary source path,
  and lazy body/class skipping compares source locations against that path
  before falling back to the old index heuristic. Added non-STL PA34 reducer
  `750-lazy-header-body-keeps-primary-source.t`.
- Fixed the PA34 `655-const-unordered-map-find` strict-lazy frontier. The
  hosted failure split into three related lazy/reference-collection issues:
  class member function redeclaration matching was treating return-type changes
  from placeholder aliases to concrete instantiated aliases as distinct
  overloads, reference-member collection was manufacturing a dependent member
  alias before consulting already-known direct aliases, and leaf constexpr
  SFINAE evaluation rejected lazy header member function-template calls because
  duplicate discovery looked ambiguous and because the selected constexpr body
  remained a lazy-body placeholder. Member redeclaration matching now compares
  the function signature without the return type and refreshes the binding from
  the incoming definition, reference collection prefers direct aliases when
  present, leaf function lookup appends unique bindings, and constexpr
  evaluation materializes lazy function bodies on demand. Added non-STL PA34
  reducer `751-lazy-header-constexpr-sfinae-pair.t`.
- Fixed the follow-on PA34 `676-hosted-algorithm-copy-n-using-directive`
  strict-lazy frontier. Explicit member function-template constexpr calls were
  seeding overload candidates from already-instantiated leaf function bindings,
  which made stale specializations compete with the requested template-id
  instantiation during non-type SFINAE evaluation. Leaf constexpr calls now let
  explicit template-id lookup instantiate from the function-template
  declaration, while non-template-id lookup still merges leaf bindings with
  semantic duplicate suppression. Added non-STL PA34 reducer
  `752-lazy-header-explicit-constexpr-template-id.t`.
- Fixed the PA34 `678-hosted-deque-member-template-include` strict-lazy
  frontier. A class-template reset discarded every function pointer visible
  through the class member scope, including base-class methods imported by a
  `using` declaration. When the base layout class was later reused, the derived
  split-buffer class retained dangling imported method pointers and member-call
  lookup lost `begin()`. Class reset now discards only function bindings owned
  by the class being reset or declared directly in that class member scope.
  Added non-STL PA34 reducer
  `753-lazy-using-import-retains-base-method.t`, which fails on the pre-fix
  binary under strict-lazy mode and passes after the fix.
- Fixed the follow-on PA34
  `725-hosted-piecewise-pair-index-sequence-alias` strict-lazy frontier through
  a PA22 non-STL reduction. The `__type_pack_element` builtin expanded a bound
  type pack to text and reparsed entries like `V const &` from inside the
  library scope, turning concrete local types into dependent names. The builtin
  now preserves structured `TypePtr` entries for directly bound packs and
  prefers bound-pack matches when text expansion is unavoidable. Added PA22
  regression `513-type-pack-element-preserves-concrete-argument.t`. Validation:
  focused PA22 regression check and witness compare passed, focused `725`
  hosted compile/check passed, and strict `pa18 pa19 pa21 pa22` owner suite
  passed.
- Fixed the first PA35
  `651-hosted-unordered-map-string-int-link-smoke` strict-lazy link failure
  through a PA18 non-STL reduction. Reference-member collection and the
  out-of-class member-template matcher could treat an enclosing namespace
  typedef in an in-class member-template declaration as a dependent current-class
  member type, so the out-of-class definition was stored as an ordinary member
  definition and the instantiated member-template body was never attached. The
  lookup/matcher now consult enclosing typed scopes before keeping that
  placeholder and resolves dependent qualified member placeholders against
  enclosing typed scopes when the member is not actually a class member. Added
  PA18 regression
  `247-out-of-class-member-template-namespace-typedef.t`. Validation: focused
  PA18 reducer passed, focused PA35 `651` link check passed, and strict
  `pa18 pa19 pa21 pa22` owner suite passed.
- Fixed the follow-on PA35
  `652-hosted-unordered-set-pointer-link-smoke` strict-lazy link failure.
  During class reference-member collection, unqualified lookup for `size_t`
  inside a hosted class-template member-template declaration fabricated a
  dependent current-class member type,
  `std::__1::__hash_table<...>::size_t`, before ordinary lookup could find the
  enclosing/global typedef. That poisoned the parsed `__construct_node_hash`
  parameter type and later made overload resolution reject the
  `std::__try_key_extraction` path. The in-progress placeholder lookup is now
  limited to names already present in the class member type scope, so unknown
  names continue to enclosing scopes. Earliest owner is PA18
  template/class-member lookup, but a reliable non-STL reducer has not yet been
  found: local reductions reparse the member-template declaration to the
  correct enclosing type before the hosted failure mode is reached. Validation:
  focused PA35 `652` link check passed, and strict `pa18 pa19 pa21 pa22` owner
  suite passed.
- Improved the PA35
  `651-hosted-unordered-map-string-int-link-smoke` strict-lazy compile time
  without relying on the raw class-reference cache. The repeated work was not a
  dependent `tuple_size<_Tp>` query that eventually became complete; each
  query already finalized as a dependent primary/forward-only reference. The
  repetition stopped only when the outer semantic output closure reached its
  required-definition fixpoint. The expensive inner loop came from class
  partial-specialization matching reparsing nested pattern arguments even when
  the actual argument was a bare type template parameter and only a direct type
  parameter pattern could match. Partial-specialization deduction now rejects
  that top-level shape before parsing nested patterns. Focused `651`
  `--emit-semantics` improved from about `26s` to `8s`, and hotspot query
  volume dropped from about `888k` to `57k`. Validation: focused PA21/PA22
  partial-specialization regressions passed, focused PA35 `651` link check
  passed, and strict `pa18 pa19 pa21 pa22` owner suite passed.

## Completion Criteria

This plan is complete when:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa34 pa35'` is green, or every
  remaining failure is explicitly deferred with a root cause and owner.
- All standard-language failures found through PA34/PA35 have minimal
  earliest-owner regressions.
- Template regressions in `pa18`, `pa19`, `pa21`, or `pa22` match strict
  witness output.
- No known case remains where `cppgm++` witness output is correct but compile,
  LowIR, link, or runtime behavior is wrong without an active owner bug.
- Hosted-only failures left in `pa34`/`pa35` are documented as hosted/vendor or
  link/runtime compatibility issues, not unexamined STL reductions.
