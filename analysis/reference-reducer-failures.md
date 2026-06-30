# Reference-Blocked Reducers

This ledger records reducers that passed enough investigation to look like useful
earlier-PA tests, but could not be promoted because the required
reference-generation workflow rejected the source or produced an incompatible
reference.

Reference-source note: the canonical generator for current assignment refs is
the local `dev/cppgm++` built from this checkout. `cppgm++-ref` binaries under
the Opus tree were generated from older versions of this codebase and should be
used only as historical evidence. Before acting on any entry below, re-test the
saved reducer with the local canonical build and update the disposition if the
older Opus reference result is stale.

Each entry should be treated as either:

- a source-validity question that needs a C++11 standard check; or
- a hosted/vendor intrinsic owner question that needs a portable rewrite or a
  PA34 hosted-compatibility home; or
- a compiler bug/quirk in the current canonical implementation that should be
  fixed before the reducer can be checked in as an assignment test.

Use these dispositions so failed reducers do not get lost in ordinary tracker
cleanup:

- `source-invalid-cxx11`: `g++ -std=c++11 -x c++ -fsyntax-only` rejects the
  reducer, so the source shape should be changed or discarded.
- `hosted-builtin-owner-needed`: the source may be accepted by a host compiler,
  but it depends on a hosted/vendor intrinsic such as `__type_pack_element` or
  `__make_integer_seq`; rewrite it to portable C++11 before using it as an
  earlier core-template assignment test, or cover the intrinsic form in PA34.
- `reference-compiler-bug`: the reducer is valid C++11, but the local
  canonical compiler rejects it, crashes, or emits behaviorally wrong output
  when used as the reference generator. If this only reproduces with an older
  Opus `cppgm++-ref`, reclassify the entry after local re-test.
- `reference-contract-mismatch`: the reducer is valid C++11 and both compilers
  accept it, but the reference output includes extra declarations, metadata, or
  other output-shape differences that block assignment validation.
- `resolved-local-canonical-promoted`: a reducer previously blocked by an older
  reference workflow was re-tested with this checkout's local canonical
  `dev/cppgm++`, generated refs cleanly, and was promoted to an assignment
  test.

For new entries, record the reduced source path, Opus start/fix evidence, a
`g++ -std=c++11 -x c++ -fsyntax-only` validity result, local compiler behavior,
local canonical reference-generation behavior, tracker disposition, and the
next validation step. If the source is not valid C++11, say so directly instead
of classifying it as a reference issue. Do not classify a reducer as
`reference-compiler-bug` until the saved reducer, or the exact source named by
the entry, has passed the C++11 syntax check and reproduced against the local
canonical build.

## Active Reference-Blocked Reducers

### Member Template Implicit Instantiation Not Overload

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/200-member-template-implicit-instantiation-not-overload.t`
- Candidate owner: none for backfill; Opus PA23 start already accepts this
  shape, so it does not prove a missing earlier assignment feature.
- Reducer: the PA23 source row itself.
- Historical evidence: Opus start `1963d796e` accepts and emits LowIR; sampled
  commits through `33af1a9bc` emit the same LowIR hash
  `45b88aca93eb9afc202a6cfdcd08c7943138bf914744e88f9b0e0e9dd9f2ce05`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  source.
- Current compiler behavior: accepts the source.
- External reference behavior: rejects the source as an ambiguous overload for
  the second `stream::async_read_some` call, treating both the cached pointer
  specialization and the function-lvalue deduction as viable overloads.
- Current disposition: tracker row marked `harness-or-reference-issue` /
  `no-action`; no earlier assignment test added.
- Next validation: fix the reference compiler so an implicitly instantiated
  member function template specialization is not retained as a separate overload
  candidate for the later function-lvalue call.

### Partial Specialization Inherited Constructor Template

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-partial-specialization-inherited-constructor-template.t`
- Candidate owner: PA22, with PA21 partial-specialization prerequisites.
- Reducer: `analysis/reducers/pa22-partial-specialization-inherited-constructor-template.t`
- Historical evidence: Opus start `1963d796e` rejects with
  `using-declaration target not found: op<Impl,Work,Handler>::op`; Opus commit
  `0ed91f49d` accepts and emits LowIR.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the
  reducer with no viable constructor for
  `op<impl, work, handler, void(int)>`; this is now treated as stale historical
  evidence rather than a current blocker.
- Current disposition: promoted to
  `pa22/tests/general/400-partial-specialization-inherited-constructor-template.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: focused PA22 check passed; PA22 placement audit passed with
  `--fail-on-early`; focused direct LowIR compare passed with
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make -C pa22 check
  TEST=tests/general/400-partial-specialization-inherited-constructor-template.t`;
  focused PA22/PA23 test report passed.

### Inherited Constructor Template Member Alias Pack

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-inherited-constructor-template-member-alias-pack.t`
- Candidate owner: PA22.
- Reducer: `analysis/reducers/pa22-inherited-constructor-template-member-alias-pack.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `33af1a9bc` accepts and emits LowIR, and `0ed91f49d` also accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the
  reducer with no viable constructor for `key<first, second, third>`; this is
  now treated as stale historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa22/tests/general/500-inherited-constructor-template-member-alias-pack.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: focused PA22 check passed; PA22 placement audit passed with
  `--fail-on-early`; focused direct LowIR compare passed with
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make -C pa22 check
  TEST=tests/general/500-inherited-constructor-template-member-alias-pack.t`;
  focused PA22/PA23 test report passed.

### Dependent ADL Hidden Friend Before Later Value

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-dependent-adl-hidden-friend-before-later-value.t`
- Candidate owner: PA22.
- Reducer: `analysis/reducers/pa22-dependent-adl-hidden-friend-before-later-value.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown type
  `enable_if<traits::prefer_free<T,U>::value,int>::type`; Opus commit
  `8bafc1c42` accepts the same source.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the
  reducer with a failed `static_assert` after missing the hidden friend found
  by ADL; this is now treated as stale historical evidence rather than a
  current blocker.
- Current disposition: promoted to
  `pa22/tests/general/300-dependent-adl-hidden-friend-before-later-value.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 check passed; grouped focused direct LowIR
  compare passed; PA22 placement audit passed with `--fail-on-early`; focused
  PA22/PA23 test report passed.

### Hidden Friend Expression-SFINAE Use-Scope Shadowing

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-hidden-friend-sfinae-use-scope-shadowing.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-hidden-friend-sfinae-use-scope-shadowing.t`
- Historical evidence: Opus start `1963d796e` rejects with `static_assert
  failed`; Opus commit `a6723df7a` and later sampled commits accept.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the
  reducer with `static_assert false:
  query_free<executor,context_as_t<int&>>::value`; this is now treated as stale
  historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa22/tests/general/300-hidden-friend-sfinae-use-scope-shadowing.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 check passed; grouped focused direct LowIR
  compare passed; PA22 placement audit passed with `--fail-on-early`; focused
  PA22/PA23 test report passed.

### Hidden Friend Dependent Return Specialization Scope

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row:
  `pa23/tests/general/300-hidden-friend-dependent-return-specialization-scope.t`
- Candidate owner: PA22.
- Reducer:
  `analysis/reducers/pa22-hidden-friend-dependent-return-specialization-scope.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 row; the saved
  reducer was blocked before promotion by the external reference compiler.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the
  reducer after falling back from overload resolution to builtin `operator==`
  while evaluating `has_equal<any_completion_executor>`; this is now treated as
  stale historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa22/tests/general/300-hidden-friend-dependent-return-specialization-scope.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 check passed; grouped focused direct LowIR
  compare passed; PA22 placement audit passed with `--fail-on-early`; focused
  PA22/PA23 test report passed.

### Template-Template Fixed-Prefix Pack Order

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-template-template-fixed-prefix-pack-order.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-template-template-fixed-prefix-pack-order.t`
- Reducer shape: `pointer_rebinder<alloc<int>, void, 0u>` should select the
  `template<template<class> class Ptr>` partial specialization over the
  `template<template<class, class...> class Ptr>` partial specialization.
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `pointer_rebinder<alloc<int>,void,0u>::value`; Opus commit `07373f00d` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected the PA21
  reducer as an ambiguous partial class specialization; this is now treated as
  stale historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa21/tests/general/400-template-template-fixed-prefix-pack-order.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- Validation: focused PA21 check passed; PA21 placement audit passed with
  `--fail-on-early`; focused direct LowIR compare passed with
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make -C pa21 check
  TEST=tests/general/400-template-template-fixed-prefix-pack-order.t`.
  The broader PA19-PA23 direct LowIR window was run and failed on unrelated
  pre-existing direct-text mismatches outside this reducer.

### Defaulted Class-Template Argument Prefix Deduction

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/spec/200-defaulted-class-template-argument-prefix-deduction.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-defaulted-class-template-argument-prefix-deduction.t`
- Historical evidence: PA23-shaped compound-assignment reducer is fixed by Opus
  commit `553397844`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  ordinary-call reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference binary rejected both
  the compound-assignment form and the smaller ordinary-call reducer; this is
  now treated as stale historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa22/tests/spec/200-defaulted-class-template-argument-prefix-deduction.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: focused PA22 check passed; focused direct LowIR compare passed;
  PA22 placement audit passed with `--fail-on-early`; focused PA22/PA23 test
  report passed.

### Explicit Template Call Transitive-Base Deduction

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/300-explicit-template-call-transitive-base-deduction.t`
- Candidate owner: PA22
- Reducer evidence: PA23-shaped recursive-base reducer failed at Opus start
  `1963d796e` and passed at `fc8434823`; simple control reducer is in
  `analysis/reducers/pa22-explicit-template-call-transitive-base-deduction-simple.t`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the saved
  simple control reducer.
- Current compiler behavior: the simple control accepts but already passed Opus
  start, so it does not prove the feature.
- External/reference workflow behavior: the PA23-shaped reducer's reference
  output included extra EH runtime declarations that current PA22 LowIR did not
  emit, so it failed current validation after reference generation.
- Current disposition: tracker row marked `harness-or-reference-issue` /
  `no-action`; no PA22 assignment test added.
- Next validation: isolate a smaller transitive-base deduction reducer whose
  reference output does not depend on the extra EH declarations, or decide the
  reference/runtime declaration contract first.

### Structured Enable-If `sizeof...` Pack Value

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-structured-enable-if-sizeof-pack-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-structured-enable-if-sizeof-pack-value.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits LowIR without
  the expected `tuple<int>` constructor call; Opus commit `1a232426b` and later
  sampled commits emit the constructor call.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and emits the constructor call;
  local `dev/cppgm++` generated the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer but emitted extra EH runtime declarations
  (`_Unwind_Resume` and `__gxx_personality_v0`) that current PA22 LowIR does not
  emit. This is treated as stale historical evidence now that local
  `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/300-structured-enable-if-sizeof-pack-value.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a normalized
  near-duplicate of the promoted PA22 reducer.

### Qualified Member Alias SFINAE

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/100-qualified-member-alias-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-qualified-member-alias-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `1a232426b` and later sampled commits accept.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source. A smaller type-only
  reducer passed at Opus start and therefore did not prove the missing feature.
- Current disposition: promoted to
  `pa22/tests/general/400-qualified-member-alias-sfinae.t`; tracker row marked
  `missing-earlier-feature` / `test-added`. Placement audit requires the PA22
  `400` cluster for the defaulted non-type guard / pointer-reference NTTP shape.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a normalized
  duplicate of the promoted PA22 reducer.

### Alias SFINAE Inherited Member Value

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-alias-sfinae-inherited-member-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-alias-sfinae-inherited-member-value.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits LowIR selecting
  the wrong constructor shape; Opus commit `1a232426b` and later sampled commits
  emit the converting constructor selected through the alias SFINAE condition.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and emits the expected
  converting-constructor shape; local `dev/cppgm++` generated the canonical PA22
  refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/300-alias-sfinae-inherited-member-value.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source remains as plausible
  integration coverage.

### Inherited Variable-Template Enable-If Return

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/300-inherited-variable-template-enable-if-return.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-inherited-variable-template-enable-if-return.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `traits<Alloc>::construct`; Opus commit `1a232426b` and later sampled commits
  accept.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with `unknown function
  traits<Alloc>::construct`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: confirm whether an inherited protected variable template may
  constrain the return type of a member function template in this use. If valid,
  fix the reference compiler before adding the PA22 test.

### Member-Template Assignment SFINAE Copy Fallback

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/300-member-template-assignment-sfinae-copy-fallback.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-template-assignment-sfinae-copy-fallback.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits LowIR with the
  wrong assignment target shape; Opus commit `1a232426b` and later sampled
  commits emit the copy-assignment fallback shape.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: find a non-codegen oracle for selecting the copy-assignment
  fallback over the member-template assignment candidate, or fix the
  reference/current EH declaration contract for this shape.

### Unnamed NTTP Pack Static Enable-If Default

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/300-unnamed-nontype-pack-static-enable-if-default.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-unnamed-nontype-pack-static-enable-if-default.t`
- Historical evidence: Opus start `1963d796e` exits 0 but does not emit the
  selected constructor template call; Opus commit `1a232426b` and later sampled
  commits emit the selected constructor template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: find a type-only or otherwise ref-stable oracle for unnamed
  NTTP-pack substitution in the static enable_if default, or fix the
  reference/current EH declaration contract for this constructor-template shape.

### Member Alias Template-Template SFINAE Owner

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-member-alias-template-template-sfinae-owner.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-alias-template-template-sfinae-owner.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown type
  `valid<quoted_identity::template fn, void>`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with failed alias template
  argument resolution for `valid`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: confirm the nested member alias template-template argument
  validity rule, then fix the reference compiler or find a smaller ref-stable
  oracle.

### Template-Template Alias Default Arity SFINAE

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-template-template-alias-default-arity-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-template-template-alias-default-arity-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `valid<two>::value`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: emits `false` for `valid<two>` and
  `valid<two, int>`, then accepts `valid<two, int, long>`.
- External reference behavior: accepts the reducer but emits LowIR treating the
  under-arity probes as true, so the reference would return early from the
  wrong branches.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's alias-template arity SFINAE
  handling, or find a smaller oracle with matching reference output.

### Alias Template-Template Defaulted SFINAE Canonical Args

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/500-alias-template-template-defaulted-sfinae-canonical-args.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-alias-template-template-defaulted-sfinae-canonical-args.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown type
  `if_<valid<F,T...>,defer_impl<F,T...>,no_type>`; Opus commit `1a232426b`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with unsupported alias
  template instantiation for `any`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's canonical alias-template
  argument substitution, or reduce to a ref-stable alias validity probe.

### Async Initiate Dependent Return SFINAE

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-async-initiate-dependent-return-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-async-initiate-dependent-return-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with no member
  `async_wait<int>`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  dependent-return SFINAE shape, or reduce to a type-only oracle.

### Constructor SFINAE Member-Template Value

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/500-constructor-sfinae-member-template-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-constructor-sfinae-member-template-value.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  constructor shape; Opus commit `1a232426b` emits the selected constructor
  template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer after a semantic fallback for
  explicit type template argument resolution in `supportable_properties`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's explicit type-argument
  substitution in this nested member-template value path, or find a smaller
  constructor-selection oracle.

### Defaulted Pack Bool Short-Circuit SFINAE

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-defaulted-pack-bool-short-circuit-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-defaulted-pack-bool-short-circuit-sfinae.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  constructor shape; Opus commit `1a232426b` emits the selected constructor
  template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations and different alias
  object surface than current PA22 LowIR, causing relaxed comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: decide the reference/current EH and alias object contract for
  this constructor-template SFINAE shape, or reduce to a non-codegen oracle.

### Member-Template Enable-If Redeclaration Overload

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-member-template-enable-if-redeclaration-overload.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-template-enable-if-redeclaration-overload.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a body returning
  `0` for the pointer overload; Opus commit `1a232426b` emits the selected
  pointer overload returning `2`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for
  out-of-class member-template overload bodies, or reduce to a non-codegen
  redeclaration oracle.

### Source-Owner Member-Template SFINAE Default

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-source-owner-member-template-sfinae-default.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-source-owner-member-template-sfinae-default.t`
- Historical evidence: Opus start `1963d796e` rejects with no matching function
  for `insert`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  source-owner member-template SFINAE shape, or reduce to a type-only oracle.

### TCC Member Constructible Pack SFINAE

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/500-tcc-member-constructible-pack-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-tcc-member-constructible-pack-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: emits the constrained `make<int>` overload
  returning `11`.
- External reference behavior: accepts the reducer but emits LowIR selecting
  the variadic fallback returning `4`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's member-template SFINAE
  selection for dependent pack-expanded constructibility, or find a smaller
  ref-stable oracle.

### Out-Of-Class SFINAE Member-Template Alias Body

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/spec/300-out-of-class-sfinae-member-template-alias-body.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-sfinae-member-template-alias-body.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  out-of-class member-template body shape; Opus commit `1a232426b` emits the
  concrete body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer and emits the concrete
  out-of-class `assign` body.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations and leaves the
  out-of-class `assign` body as an external declaration, causing relaxed LowIR
  comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's out-of-class member-template
  body emission for this alias-SFINAE redeclaration, or reduce to a witness-only
  oracle.

### Out-Of-Class SFINAE Member-Template Body

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/spec/300-out-of-class-sfinae-member-template-body.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-sfinae-member-template-body.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  selected out-of-class member-template body; Opus commit `1a232426b` emits the
  selected bidirectional-iterator overload body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  out-of-class member-template overload shape, or find a ref-stable oracle.

### Defaulted NTTP Qualified Alias Value

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-defaulted-nontype-qualified-alias-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-defaulted-nontype-qualified-alias-value.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `72e996289` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  constructor-template defaulted-NTTP alias value shape, or reduce to a
  non-codegen oracle.

### Alias NTTP Expression Declaration Scope

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-alias-nontype-expression-declaration-scope.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-alias-nontype-expression-declaration-scope.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `meta::check<meta::list<int,char>,1>::value`; Opus commit `3bec27bf4`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer while evaluating the non-type
  template argument `(1 <= size<meta::list<int, char>>::value)`.
- Current disposition: PA21 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's declaration-scope evaluation of
  alias-template non-type arguments, or find a smaller ref-stable oracle.

### Member Alias Template Owner Rebind Cache

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/400-member-alias-template-owner-rebind-cache.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-member-alias-template-owner-rebind-cache.t`
- Historical evidence: Opus start `1963d796e` rejects with non-constant
  `static_assert`; Opus commit `3bec27bf4` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA21 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA21 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  member-alias owner rebinding shape, or find a non-codegen oracle.

### Out-Of-Class Partial Owner Constructor Using Alias

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/300-out-of-class-partial-owner-ctor-using-alias.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-partial-owner-ctor-using-alias.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a different
  constructor body shape; Opus commit `45820ae4b` emits the selected
  out-of-class constructor-template body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  out-of-class partial-owner constructor-template shape, or find a non-codegen
  oracle.

### Alias Template Function Argument CV

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/400-alias-template-function-argument-cv.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-alias-template-function-argument-cv.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a different
  constructor-template LowIR shape; Opus commit `45820ae4b` emits the expected
  handler/invoker shape.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  constructor-template alias-argument shape, or reduce to a non-codegen oracle.

### Alias Pack Function Return Concrete

- Disposition: `portable-rewrite-pending-reference-validation`
- PA23 source row: `pa23/tests/general/500-alias-pack-function-return-concrete.t`
- Candidate owner: PA22 for the portable tuple-element form; PA34 remains the
  owner for direct `__type_pack_element` intrinsic behavior.
- Reducer: `analysis/reducers/pa22-alias-pack-function-return-concrete.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared `get<1>`;
  Opus commit `45820ae4b` accepts.
- C++11 syntax check: `g++ -std=c++11 -x c++ -fsyntax-only` and
  `clang++ -std=c++11 -x c++ -fsyntax-only` accept the portable reducer after
  replacing `__type_pack_element` with a recursive local `pack_element` helper.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA22 LowIR
  does not emit, causing relaxed LowIR comparison to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker row
  is marked `portable-rewrite-pending-validation` / `no-action`.
- Next validation: rerun the PA22 placement/reference gates on the portable
  reducer; if the EH declaration drift remains, keep this as a
  reference/contract blocker rather than a hosted-builtin blocker.

### Dependent Alias Decltype Qualified Function Source Tokens

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/500-dependent-alias-decltype-qualified-function-source-tokens.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-dependent-alias-decltype-qualified-function-source-tokens.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown sizeof
  operand type; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with unsupported template
  declarator in the qualified function-template helper declaration.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's template declarator support for
  this qualified decltype SFINAE helper, or find a smaller ref-stable oracle.

### Decltype Trailing Pack Template-Id Deduction

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/200-decltype-trailing-pack-template-id-deduction.t`
- Candidate owner: PA22.
- Reducer:
  `analysis/reducers/pa22-decltype-trailing-pack-template-id-deduction.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 row; the saved
  reducer was blocked before promotion by the external reference compiler.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with a failed `static_assert`
  because `decltype(f(static_cast<identity<T>*>(0)...))` does not preserve the
  deduced trailing pack as `list<A, B>`.
- Current disposition: no PA22 assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's pack deduction through
  decltype(function-template-call) template-id arguments, then promote this
  PA22 reducer if reference output becomes stable.

### Static Data NTTP Pack Sizeof Bound

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/400-static-data-nttp-pack-sizeof-bound.t`
- Candidate owner: PA22 according to the PA23 backfill cluster, with PA21
  static-data/partial-specialization prerequisites.
- Reducer: `analysis/reducers/pa22-static-data-nttp-pack-sizeof-bound.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 row; the saved
  reducer was blocked before promotion by the external reference compiler.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and emits the `Bytes...`
  static data.
- External reference behavior: rejects the out-of-class static data member
  definition with `sizeof...(Bytes)` in the array bound as an unsupported
  template declarator.
- Current disposition: no assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's support for pack-size array
  bounds in out-of-class static data member definitions, then re-evaluate the
  earliest PA owner before promotion.

### Function Template Parameter Alias Sees Previous Param

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/500-function-template-parameter-alias-sees-previous-param.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-function-template-parameter-alias-sees-previous-param.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown sizeof
  operand type; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with unknown function
  `helper<value>`.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's function-template lookup after
  alias-parameter substitution, or find a smaller ref-stable oracle.

### Index Sequence Alias Constructor Deduction

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/500-index-sequence-alias-constructor-deduction.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-index-sequence-alias-constructor-deduction.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `index_sequence<0>`; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations and an empty helper
  constructor body not emitted by current PA22, causing relaxed LowIR comparison
  to fail.
- Current disposition: PA22 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH and empty-constructor emission
  contract for this index-sequence constructor-deduction shape, or reduce to a
  non-codegen oracle.

### Alias Pack NTTP Expression Fast Path

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-alias-pack-nontype-expression-fast-path.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-alias-pack-nontype-expression-fast-path.t`
- Historical evidence: Opus start `1963d796e` rejects with shadowed template
  parameter `L`; Opus commit `743b7a920` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with unsupported alias
  template instantiation for `count_if`.
- Current disposition: PA21 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's alias-template pack and
  template-template parameter expansion in NTTP expressions, or find a smaller
  ref-stable oracle.

### Qualified Template-Id Current-Scope Alias Shadow

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-qualified-template-id-current-scope-alias-shadow.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-qualified-template-id-current-scope-alias-shadow.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown
  `mp_if<bool_constant<true>,transform_impl<F,L...>,mismatch>::type`; Opus
  commit `743b7a920` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: the reference binary segfaults while compiling
  the reducer.
- Current disposition: PA21 candidate assignment files were removed; tracker
  row is marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler crash in qualified template-id
  alias current-scope rebinding, or find a smaller non-crashing oracle.

### Nested Template-Id Partial Specialization Deduction

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/200-nested-template-id-partial-specialization-deduction.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-nested-template-id-partial-specialization-deduction.t`
- Historical evidence: the PA23-shaped source rejects at Opus start
  `1963d796e` with unknown type
  `aligned_storage<sizeof(T),alignment_of<T>::value>::type`; the reduced
  oracle rejects at Opus start with a failed `static_assert`. Sampled Opus
  commits through `743b7a920` still reject the reduced oracle, so no
  start/fix transition has been identified for a promotable assignment test.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: `./dev/cppgm++ --emit-lowir -O0 -o ...` accepts
  the reducer.
- External reference behavior: rejects the reduced oracle with a failed
  `static_assert`, leaving `value_type` as the fallback primary type instead of
  selecting `node_value<base_node<T, hook<...>, true> >`. The external reference
  also rejects the PA23-shaped source while evaluating `sizeof(T)` for
  `lib::base_node<pair_like<recursive_map const, recursive_map>, ...>`.
- Current disposition: tracker row marked `harness-or-reference-issue` /
  `no-action`; no PA21 assignment test added.
- Next validation: fix the reference compiler's nested class-template-id
  partial-specialization matching and incomplete-type handling, then find the
  actual Opus fix transition before promoting this row.

### Template Instantiation Use Location Explicit Specialization

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/100-template-instantiation-use-location-explicit-specialization.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-template-instantiation-use-location-explicit-specialization-value-object.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with unknown
  type `ops<policy>::iter_move`; Opus commit `fb9e82eeb` accepts the same
  reducer.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and emits construction of the
  local array element plus the move construction of the `sift` local object.
- External reference behavior: accepts the reducer but omits constructor work
  that current emits; the PA21 relaxed LowIR comparison fails on the missing
  default/move-constructor calls and move-constructor body.
- Current disposition: PA21 candidate assignment files were removed; tracker
  row marked `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's value-object construction
  emission for this use-location member-template call, or find a non-codegen
  oracle that still fails at Opus start.

### Explicit Instantiation Static Member Function

- Disposition: `reference-contract-mismatch`
- PA23 source row:
  `pa23/tests/spec/100-explicit-instantiation-static-member-function.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-explicit-instantiation-static-member-function.t`
- Historical evidence: the PA23 source row failed the Opus PA23 start report by
  relaxed LowIR comparison; Opus commit `569fcbd79` is the explicit member
  function instantiation implementation point in the cluster.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and marks the explicitly
  instantiated static member function with `object_root=yes`.
- External reference behavior: accepts the reducer but omits the `object_root`
  metadata, causing relaxed LowIR comparison to fail.
- Current disposition: no PA21 assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: decide the reference/current LowIR metadata contract for
  explicit member-function instantiation roots, then promote the PA21 reducer if
  the reference output becomes stable.

### Explicit Specialization Cross Converting Constructor Body

- Disposition: `reference-contract-mismatch`
- PA23 source rows:
  `pa23/tests/general/100-explicit-specialization-cross-converting-ctor-body.t`,
  `pa23/tests/spec/100-explicit-specialization-cross-converting-ctor-body.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-explicit-specialization-cross-converting-ctor-body.t`
- Historical evidence: the PA23 source rows failed the Opus PA23 start report by
  relaxed LowIR comparison. A reduced no-call constructor-body shape is
  reference-stable but already passes Opus start, so it does not prove a missing
  earlier feature. The value-object reducer keeps the start failure but is not
  reference-stable.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and emits constructor aliases
  without EH runtime declarations for this non-throwing shape.
- External reference behavior: accepts the reducer but emits EH runtime
  declarations and omits constructor aliases, causing relaxed LowIR comparison
  to fail.
- Current disposition: no PA21 assignment test added; tracker rows marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: decide the reference/current LowIR contract for EH runtime
  declarations and constructor aliases in explicit-specialization constructor
  bodies, or find a non-codegen oracle that still fails at Opus start.

### Class Partial Specialization Matching/Ordering Batch

- Candidate owner: PA21
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: PA21 candidate assignment files were removed for these
  rows; tracker rows are marked `harness-or-reference-issue` / `no-action`.

| PA23 source row | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- |
| `pa23/tests/general/200-nontype-pack-fixed-tail-partial-specialization-ordering.t` | `analysis/reducers/pa21-nontype-pack-fixed-tail-partial-specialization-ordering.t` | Opus start `1963d796e` rejects; Opus commit `ee034cde3` accepts. | Rejects while resolving `bin_literal<bytes<>,'0','0','0','0','0','0','0','0'>::size`. | Fix reference compiler non-type pack plus fixed-tail partial-specialization matching. |
| `pa23/tests/general/400-cv-qualified-template-id-wrapper-class-partial-specialization.t` | `analysis/reducers/pa21-cv-qualified-template-id-wrapper-class-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `a3c294645` accepts. | Rejects as an ambiguous partial class specialization for `holder<box<int,long> const>`. | Fix reference compiler ordering between direct cv-qualified and template-id partials. |
| `pa23/tests/general/400-dependent-owner-member-class-template-partial-specialization.t` | `analysis/reducers/pa21-dependent-owner-member-class-template-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `0cf7d2de0` accepts. | Rejects while collecting the out-of-class member class-template partial specialization. | Fix reference compiler support for dependent-owner member class-template partial specializations. |
| `pa23/tests/general/400-function-type-cv-partial-specialization.t` | `analysis/reducers/pa21-function-type-cv-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `b444d928a` accepts. | Rejects as an ambiguous partial class specialization for `holder<void()>`. | Fix reference compiler function-type cv-qualified partial-specialization ordering. |
| `pa23/tests/general/400-function-type-ref-qualified-partial-specialization.t` | `analysis/reducers/pa21-function-type-ref-qualified-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `f31e61547` accepts. | Rejects with failed `static_assert` for `holder<void() &>::value == 2`. | Fix reference compiler ref-qualified function-type partial-specialization matching. |
| `pa23/tests/general/400-repeated-pack-partial-specialization-conflict.t` and `pa23/tests/general/400-repeated-pack-partial-specialization-preference.t` | `analysis/reducers/pa21-repeated-pack-partial-specialization-ordering.t` | Opus start `1963d796e` rejects; Opus commit `b444d928a` accepts. | Rejects as an ambiguous partial class specialization for `similar_impl<list<>,list<> >`. | Fix reference compiler repeated-pack partial-specialization ordering and conflict rejection. |

### Top-CV Pointer Does Not Match Unqualified Pointer Partial

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/100-top-cv-pointer-does-not-match-unqualified-pointer-partial.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-top-cv-pointer-does-not-match-unqualified-pointer-partial.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with a
  failed `static_assert`; Opus commit `15a2e9c8d` accepts the reducer.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer as an ambiguous partial
  class specialization for `holder<B const* volatile>`, allowing the
  unqualified pointer partial to compete with the top-level volatile wrapper.
- Current disposition: no PA21 assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler so class partial-specialization
  matching preserves top-level cv on pointer objects before promoting this PA21
  reducer.

### Qualified Friend Member Function Template

- Disposition: `reference-contract-mismatch`
- PA23 source row: `pa23/tests/general/400-qualified-friend-member-template-access.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-qualified-friend-member-template.t`
- Historical evidence: Opus start `1963d796e` rejects the original value-object
  reducer with `unknown type name: iter<C,B>`; Opus commit `670045ef2` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External/reference workflow behavior: the external reference binary accepts
  the reducer, but emits extra EH runtime declarations that current PA21 LowIR
  does not emit. A static-member/no-constructor variant avoids the reference
  drift but also passes Opus start, so it does not prove the missing feature.
- Current disposition: no PA21 assignment test added; tracker row is marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference/current EH declaration contract for this
  value-object qualified friend member-template call, or find a smaller
  ref-stable oracle that still fails at Opus start.

### Member Template As Qualified Template-Template Argument

- Disposition: `reference-compiler-bug`
- PA23 source row: `pa23/tests/general/400-member-alias-template-template-owner-argument.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-member-template-as-template-template-argument.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with
  `unknown type name: use<quote::template fn>::type`; Opus commit `670045ef2`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the type-only reducer with failed
  `static_assert` because `result_t` remains `use<quote::fn>::type` instead of
  resolving to `int`.
- Current disposition: no PA21 assignment test added; tracker row is marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix reference compiler resolution of a qualified member
  template named with the `template` disambiguator when it is passed as a
  template-template argument.

### Dependent Qualified Sizeof Static Member

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/general/500-dependent-qualified-sizeof-static-member.t`
- Candidate owner: PA22, using PA20 constant-evaluation support for `sizeof`.
- Reducer:
  `analysis/reducers/pa22-dependent-qualified-sizeof-static-member.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 source row; the
  feature was later fixed in the Boost frontier work tracked as
  `8d7d59de4`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with
  `ERROR: invalid sizeof type-id` while evaluating
  `sizeof(bucket_array_base<Flag>::sizes) /
  sizeof(bucket_array_base<Flag>::sizes[0])`, treating the qualified static
  data member expression as a failed `sizeof(type-id)` form.
- Placement note: the saved reducer previously used host predefined macro
  `__SIZE_TYPE__`, which is PA34 hosted predefined-macro compatibility. It has
  been rewritten to a portable `unsigned long` typedef; the reference issue
  still blocks promotion.
- Current disposition: no PA22 assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's `sizeof` expression recovery
  for dependent qualified static data members or reduce to a reference-stable
  oracle before promoting a PA22 test.

### Hidden Friend Query Free Decltype Noexcept

- Disposition: `reference-compiler-bug`
- PA23 source row:
  `pa23/tests/spec/500-hidden-friend-query-free-decltype-noexcept.t`
- Candidate owner: PA22.
- Reducer:
  `analysis/reducers/pa22-hidden-friend-query-free-decltype-noexcept.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 source row; the
  feature was later fixed in the Boost frontier work tracked as `c047d6e93`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- External reference behavior: rejects the reducer with a failed
  `static_assert` for `is_same<query_probe::result_type, int>::value`; the
  reference compiler lets the later ordinary `query` CPO object supply a `char`
  result instead of finding the hidden friend `query` returning `int` by ADL at
  the template definition point.
- Current disposition: no PA22 assignment test added; tracker row marked
  `harness-or-reference-issue` / `no-action`.
- Next validation: fix the reference compiler's dependent `decltype`/`noexcept`
  query path so hidden-friend ADL is evaluated at the correct point and later
  ordinary CPO objects are not visible, then promote this PA22 reducer if
  reference generation becomes stable.

### Constructor/Conversion Deduction Reference-Blocked Batch

- Disposition: `reference-compiler-bug` for every reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: no assignment tests added for these rows; tracker rows
  marked `harness-or-reference-issue` / `no-action`.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-constructor-template-rvalue-beats-const-ref.t` | PA22 | `analysis/reducers/pa22-constructor-template-rvalue-beats-const-ref.t` | Opus start `1963d796e` rejects or mismatches; later PA23 constructor-selection work accepts. | Rejects after selecting the `pair(const A&, const B&)` path and then failing to construct move-only `movable` from `const movable`. | Fix reference compiler constructor-template participation for rvalue arguments before promoting the PA22 reducer. |
| `pa23/tests/general/200-inherited-constructor-template-forwarding.t` | PA22 | `analysis/reducers/pa22-inherited-constructor-template-forwarding.t` | Opus start `1963d796e` rejects; later PA23 inherited-constructor-template work accepts. | Rejects `derived<int>(arg&&)` because the inherited base constructor template is missing from the derived constructor set. | Fix reference compiler inherited constructor-template collection for dependent bases before promoting the PA22 reducer. |
| `pa23/tests/general/300-constructor-template-defaulted-forwarding-lvalue-order.t` | PA22 | `analysis/reducers/pa22-constructor-template-defaulted-forwarding-lvalue-order.t` | Opus start `1963d796e` rejects or mismatches; later PA23 forwarding-constructor work accepts. | Rejects as an ambiguous constructor between `box(const T&)` and the defaulted forwarding constructor template for a const lvalue source. | Fix reference compiler constructor-template partial ordering with defaulted SFINAE parameters before promoting the PA22 reducer. |
| `pa23/tests/general/300-constructor-template-pack-before-defaulted-nontype.t` | PA22 | `analysis/reducers/pa22-constructor-template-pack-before-defaulted-nontype.t` | Opus start `1963d796e` rejects; later PA23 defaulted-NTTP recovery work accepts. | Rejects `tuple_like<first_arg, second_arg>{first, second}` with no viable constructor after losing the defaulted pointer parameter shape. | Fix reference compiler defaulted non-type parameter type recovery after a constructor parameter pack before promoting the PA22 reducer. |
| `pa23/tests/general/400-conversion-function-template-prefers-nontemplate.t` | PA22 | `analysis/reducers/pa22-conversion-function-template-prefers-nontemplate.t` | Opus start `1963d796e` rejects; Opus `f8660aa27` accepts the conversion-assignment family. | Rejects assignment as ambiguous after failing to prefer the non-template conversion function over the conversion-function-template specialization. | Fix reference compiler conversion-function-template ordering and assignment candidate handling before promoting the PA22 reducer. |
| `pa23/tests/general/400-member-template-class-pack-forward-before-token.t` | PA18 | `analysis/reducers/pa18-member-template-class-pack-forward-before-token.t` | Opus start `1963d796e` rejects; later PA23 member-template pack-binding work accepts. | Rejects `combine(static_cast<Args&&>(args)...)` because pack value binding pairs the first `Args` element with the second value argument. | Fix reference compiler member-template parameter-pack value binding when a non-pack parameter follows the pack before promoting the PA18 reducer. |
| `pa23/tests/general/400-nested-member-template-base-param-shadow-value.t` | PA21 | `analysis/reducers/pa21-nested-member-template-base-param-shadow-value.t` | Opus start `1963d796e` already accepts the PA23 source, so it does not prove a missing earlier test. | Rejects the valid source with failed `static_assert(packed::reservable == true)`, rebinding the nested member-template owner value through the dependent base. | Fix reference compiler nested member-template owner-argument binding; no backfill is needed unless a start-failing reducer is found. |

### Template-Template/Alias Forwarding Reference-Blocked Batch

- Disposition: `reference-compiler-bug` for every reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: no assignment tests added for these rows; tracker rows
  marked `harness-or-reference-issue` / `no-action`.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-function-template-variadic-template-template-deduction.t` | PA18 | `analysis/reducers/pa18-function-template-variadic-template-template-deduction.t` | Opus start `1963d796e` rejects; Opus `1434a6e11` accepts the surrounding TTP deduction family. | Rejects the valid variadic template-template call as unknown function `dispatch`. | Fix reference compiler variadic template-template parameter deduction before promoting this PA18/PA22 reducer. |
| `pa23/tests/general/200-template-template-qualified-default-arg-deduction.t` | PA18 | `analysis/reducers/pa18-template-template-qualified-default-arg-deduction.t` | Opus start `1963d796e` rejects; later PA23 qualified default-argument deduction work accepts. | Rejects both `box<int>(ns::tuple<int>())` and the qualified default-tail overload because the constructor template is not viable. | Fix reference compiler TTP deduction through qualified defaulted class-template arguments before promoting this reducer. |
| `pa23/tests/general/400-defaulted-nested-cv-template-template-partial-specialization.t` | PA21 | `analysis/reducers/pa21-defaulted-nested-cv-template-template-partial-specialization.t` | Opus start `1963d796e` rejects; later PA21/PA23 partial-specialization matching work accepts. | Rejects with failed `holder<map<int,value>>::value == 2`, selecting the generic/default-truncated shape instead of the defaulted nested-cv TTP partial. | Fix reference compiler class partial-specialization matching with defaulted nested cv-qualified TTP arguments before promoting this PA21 reducer. |
| `pa23/tests/general/400-member-alias-template-template-dependent-replay.t` | PA21 | `analysis/reducers/pa21-member-alias-template-template-dependent-replay.t` | Opus start `1963d796e` rejects; later PA21 member-alias TTP replay work accepts. | Rejects `replace_if_q<list<X, X volatile>, quote<is_volatile>, void>` as an unsupported alias template instantiation. | Fix reference compiler member alias-template replay when used as a TTP argument before promoting this PA21 reducer. |
| `pa23/tests/general/400-member-alias-template-template-empty-template-id-argument.t` | PA21 | `analysis/reducers/pa21-member-alias-template-template-empty-template-id-argument.t` | Opus start `1963d796e` rejects; later PA21 empty-template-id member-alias work accepts. | Rejects the namespace-scope typedef naming `detail::transform_impl<detail::flatten<n::list<> >::fn, n::list<n::list<> > >::type`. | Fix reference compiler member alias-template TTP application to an empty class template-id argument before promoting this PA21 reducer. |
| `pa23/tests/spec/500-template-template-conversion-operator-reference-target.t` | PA22 | `analysis/reducers/pa22-template-template-conversion-operator-reference-target.t` | Opus start `1963d796e` rejects; later PA22 conversion-template work accepts. | Rejects `view<char, traits> v(s)` because the conversion-function template is not used for the reference target. | Fix reference compiler conversion-function-template deduction against reference targets before promoting this PA22 reducer. |
| `pa23/tests/spec/500-template-template-piecewise-partial-ordering.t` | PA22 | `analysis/reducers/pa22-template-template-piecewise-partial-ordering.t` | Opus start `1963d796e` rejects; Opus `1434a6e11` accepts the TTP partial-ordering family. | Rejects fixed tuple dispatch as ambiguous between fixed-arity overloads and fails to prefer the fixed non-pack pattern over broader alternatives. | Fix reference compiler TTP transformed placeholders and function-template partial ordering before promoting this PA22 reducer. |

### Member Template Ownership Reference-Blocked Batch

- Disposition: `reference-compiler-bug` for every reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: no assignment tests added for these rows; tracker rows
  marked `harness-or-reference-issue` / `no-action`.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/400-recursive-member-template-concrete-owner.t` | PA21 | `analysis/reducers/pa21-recursive-member-template-concrete-owner.t` | Opus start `1963d796e` already accepts the PA23 source, so this row does not prove a missing earlier assignment feature. | Rejects after reaching the explicit-type-argument text fallback while resolving `supportable_properties<I, void(Head)>::template is_valid_target<T>`. | Fix reference compiler structured function-type template arguments in recursive member-template owner replay; no backfill is needed unless a start-failing reducer is found. |
| `pa23/tests/general/400-sibling-namespace-dependent-member-template-id-owner.t` | PA21 | `analysis/reducers/pa21-sibling-namespace-dependent-member-template-id-owner.t` | Opus start `1963d796e` rejects the PA23 source row. | Rejects with `named type size unavailable: detail::result_detail<Sig>::type`, letting the sibling namespace `detail` interfere with the class member owner. | Fix reference compiler dependent member-template-id owner lookup when a sibling namespace has the same name, then promote a PA21 reducer if reference generation becomes stable. |
| `pa23/tests/general/500-out-of-class-member-template-dependent-owner-type.t` | PA22 | `analysis/reducers/pa22-out-of-class-member-template-dependent-owner-type.t` | Opus start `1963d796e` rejects the PA23 source row; Boost frontier notes place this dependent-owner out-of-class definition shape at PA22. | Rejects with `unknown qualified class partial specialization scope` while collecting `service<Mutex>::implementation_type<Traits, Signatures...>` in the out-of-class member-template definition. | Fix reference compiler support for out-of-class member templates whose parameter types name nested dependent owner types, then promote the PA22 reducer if reference generation becomes stable. |
| `pa23/tests/spec/400-conversion-function-template-owner-result-copy-init.t` | PA22 | `analysis/reducers/pa22-conversion-function-template-owner-result-copy-init.t` | Opus start `1963d796e` rejects or mismatches the PA23 source row; Opus `f8660aa27` accepts the surrounding conversion-function-template assignment family. | Accepts the reducer but emits the wrong LowIR path for copy-initialization, using the explicit `basic_string(basic_string_view const&)` constructor shape instead of the conversion-function template returning `basic_string<Ch, char_traits, A>`. | Fix reference compiler copy-initialization viability for conversion-function-template results before promoting this PA22 reducer. |

### Non-Type Template Argument Reference-Blocked Batch

- Disposition: `reference-compiler-bug` for the reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- Current disposition: no assignment test added for this row; tracker row
  marked `harness-or-reference-issue` / `no-action`.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/100-nested-template-static-value-nontype-expression.t` | PA21 | `analysis/reducers/pa21-nested-template-static-value-nontype-expression.t` | Opus start `1963d796e` accepts the PA23 source, so this row does not prove a missing earlier assignment feature. | Rejects with failed non-type template argument evaluation for `calculate_alignment_ct<size_t,56,8,32,8>::initial_alignment` while resolving the nested qualified member class template value. | Fix reference compiler evaluation of nested qualified member-template static values used as non-type template arguments; no backfill is needed unless a start-failing reducer is found. |

### Pack Expansion Reference-Blocked Batch

- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: no assignment tests added for these rows; tracker rows
  marked `harness-or-reference-issue` or `historical-validation-missing` /
  `no-action`.

| PA23 source row | Candidate owner | Reducer | Disposition | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-local-pack-template-id-paren-init-in-if.t` | PA22 | PA23 source row itself | `reference-contract-mismatch` | Opus start `1963d796e` rejects; Opus `1434a6e11` still fails the same relaxed LowIR contract. | Accepts, but emits extra EH runtime declarations that current output does not emit. | Find a ref-stable shape that still start-fails, or fix the EH declaration contract before promoting. |
| `pa23/tests/general/200-local-pack-template-id-paren-init.t` | PA22 | PA23 source row itself | `reference-contract-mismatch` | Opus start `1963d796e` rejects; Opus `1434a6e11` still fails the same relaxed LowIR contract. | Accepts, but emits extra EH runtime declarations that current output does not emit. | Find a ref-stable shape that still start-fails, or fix the EH declaration contract before promoting. |
| `pa23/tests/general/200-member-function-template-address-explicit-pack.t` | PA22 | PA23 source row itself | `reference-compiler-bug` | Opus start `1963d796e` accepts but mismatches the PA23 source row; no promoted reducer because reference generation fails. | Rejects `service_registry::create<Service,context>` while taking the address of an explicit member function-template specialization. | Fix reference compiler qualified member function-template address lookup with explicit pack arguments before promoting. |
| `pa23/tests/general/300-function-template-empty-pack-trailing-default.t` | PA22 | PA23 source row itself | `reference-contract-mismatch` | Opus start `1963d796e` mismatches; Opus `1434a6e11` still fails the relaxed LowIR contract. | Accepts, but emits extra EH runtime declarations that current output does not emit. | Find a ref-stable empty-pack/default-tail reducer or fix the EH declaration contract before promoting. |
| `pa23/tests/general/400-nested-pack-expansion-outer-type-pack.t` | PA21 | PA23 source row itself | `reference-compiler-bug` | Opus start `1963d796e` rejects; Opus `1434a6e11` still does not compare to the external reference. | Accepts, but emits the wrong branch constant for the nested outer type-pack expansion result. | Fix reference compiler nested pack expansion through outer type packs before promoting. |
| `pa23/tests/general/400-pack-base-expansion.t` | PA21 | PA23 source row itself | `reference-contract-mismatch` | Opus start `1963d796e` accepts but mismatches; Opus `1434a6e11` still fails the relaxed LowIR contract. | Accepts, but emits extra EH runtime declarations that current output does not emit. | Find a ref-stable base-pack reducer or fix the EH declaration contract before promoting. |
| `pa23/tests/spec/200-defaulted-class-template-argument-pack-prefix-deduction.t` | PA22 | PA23 source row itself | `reference-compiler-bug` | Opus start `1963d796e` accepts but mismatches the PA23 source row; no promoted reducer because reference generation fails. | Rejects the valid call with unknown function `take` after failing defaulted class-template argument pack-prefix deduction. | Fix reference compiler deduction through defaulted class-template argument pack prefixes before promoting. |
| `pa23/tests/spec/300-constructor-default-pack-partial-ordering.t` | PA22 | PA23 source row itself | `reference-compiler-bug` | Opus start `1963d796e` accepts but mismatches; Opus `1434a6e11` still does not compare to the external reference. | Accepts, but selects the wrong constructor overload, storing `1` where current selects the in-place/default-pack path and stores `2`. | Fix reference compiler constructor partial ordering when default parameters and empty trailing packs interact before promoting. |
| `pa23/tests/spec/400-explicit-pack-type-argument-uses-bound-type.t` | PA19 | PA23 source row itself | `reference-compiler-bug` | Opus start `1963d796e` accepts but mismatches the PA23 source row; no promoted reducer because reference generation fails. | Rejects a valid explicit type-pack instantiation with `non-class braced-init-list requires one element`, losing the bound type argument. | Fix reference compiler explicit type-pack argument binding in braced-init call arguments before promoting. |
| `pa23/tests/spec/400-variadic-base-pack-expansion.t` | PA21 | PA23 source row itself | `reference-contract-mismatch` | Opus start `1963d796e` rejects; Opus `1434a6e11` still fails the relaxed LowIR contract. | Accepts, but emits extra EH runtime declarations that current output does not emit. | Find a ref-stable variadic-base reducer or fix the EH declaration contract before promoting. |
