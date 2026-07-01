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
- `no-backfill-start-pass`: the candidate source is valid and the local
  canonical compiler accepts it, but the portable/focused shape already passes
  at the Opus PA23 start anchor, so it does not prove a missing earlier
  assignment test.
- `historical-validation-missing`: the candidate source is valid and the local
  canonical compiler accepts it, but no reachable Opus commit has been found
  that both fixes the Opus start failure and provides the required start/fix
  gate for promotion.

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

- Disposition: `no-backfill-start-pass`
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
- Historical external reference behavior: rejects the source as an ambiguous
  overload for the second `stream::async_read_some` call, treating both the
  cached pointer specialization and the function-lvalue deduction as viable
  overloads. This does not prove a missing earlier assignment feature because
  the Opus start compiler already accepts the source.
- Current disposition: tracker row marked `no-missing-opus-feature` /
  `no-action`; no earlier assignment test added.
- Next validation: none for the Opus-gated backfill workflow. Revisit only if a
  start-failing reducer is found for the cached implicit-specialization overload
  behavior.

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
  A follow-up PA19-PA23 direct LowIR report passed after refreshing stale refs
  from the placement-reducer batch.

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

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-explicit-template-call-transitive-base-deduction.t`
- Candidate owner: PA22
- Reducer evidence: exact PA23 source row is the PA22-shaped recursive-base
  reducer; the simpler saved control in
  `analysis/reducers/pa22-explicit-template-call-transitive-base-deduction-simple.t`
  already passed Opus start and is kept only as a non-promoted control.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the exact
  PA23 source row.
- Historical gate: Opus start `1963d796e` rejects the exact source with
  `ERROR: use of undeclared identifier: ns::helper<1>`; Opus commit
  `fc8434823` accepts and emits the selected helper call.
- Current compiler behavior: accepts the exact source and generated local
  canonical PA22 refs without EH runtime declarations.
- Historical external reference behavior: an older external reference workflow
  emitted extra EH runtime declarations for this non-throwing shape. This is
  treated as stale historical evidence now that local `dev/cppgm++` is the
  canonical ref source.
- Current disposition: promoted the exact PA23 source to
  `pa22/tests/general/300-explicit-template-call-transitive-base-deduction.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as focused PA22 deduction coverage. The
  source isolates explicit template arguments plus deduction through a
  transitive dependent base and does not add a separate PA23 integration
  ingredient.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `fc8434823` accepted; local canonical
  PA22 refs were generated from `dev/cppgm++`.

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

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-inherited-variable-template-enable-if-return.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-inherited-variable-template-enable-if-return.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `traits<Alloc>::construct`; Opus commit `1a232426b` and later sampled commits
  accept.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external reference behavior: rejects the reducer with
  `unknown function traits<Alloc>::construct`; this older Opus-generated
  reference is not the current canonical source.
- Current disposition: promoted to
  `pa22/tests/general/300-inherited-variable-template-enable-if-return.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a near-duplicate. The PA23 source kept
  the same focused allocator-traits-shaped source as the promoted PA22 reducer
  and did not add a separate PA23 integration ingredient.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `1a232426b` accepted; focused PA22
  check and direct LowIR compare passed; PA22 placement audit passed with
  `--fail-on-early`.

### Member-Template Assignment SFINAE Copy Fallback

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-member-template-assignment-sfinae-copy-fallback.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-template-assignment-sfinae-copy-fallback.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits LowIR with the
  wrong assignment target shape; Opus commit `1a232426b` and later sampled
  commits emit the copy-assignment fallback shape.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/300-member-template-assignment-sfinae-copy-fallback.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 575/575 after PA23 integration restoration. A
  smaller non-template `box` assignment reducer passed Opus start
  `1963d796e`, so the `function<R(Args...)>` partial-specialization wrapper
  is part of the minimal historical signal. The PA23 source was restored
  and retained as member-template assignment/copy-fallback integration
  coverage while the PA22 reducer remains focused regression coverage.

### Unnamed NTTP Pack Static Enable-If Default

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-unnamed-nontype-pack-static-enable-if-default.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-unnamed-nontype-pack-static-enable-if-default.t`
- Historical evidence: Opus start `1963d796e` exits 0 but does not emit the
  selected constructor template call; Opus commit `1a232426b` and later sampled
  commits emit the selected constructor template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/400-unnamed-nontype-pack-static-enable-if-default.t`;
  tracker row marked `missing-earlier-feature` / `test-added`. Placement audit
  requires the PA22 `400` cluster for the pointer/reference NTTP pack shape.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a normalized
  near-duplicate of the promoted PA22 reducer.

### Member Alias Template-Template SFINAE Owner

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-member-alias-template-template-sfinae-owner.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-alias-template-template-sfinae-owner.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown type
  `valid<quoted_identity::template fn, void>`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference compiler
  rejected the reducer with failed alias template argument resolution for
  `valid`. This is treated as stale historical evidence now that local
  `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/400-member-alias-template-template-sfinae-owner.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a
  byte-identical duplicate of the promoted PA22 reducer.

### Template-Template Alias Default Arity SFINAE

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-template-template-alias-default-arity-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-template-template-alias-default-arity-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `valid<two>::value`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: matches the C++11 oracle: `valid<two>` and
  `valid<two, int>` are substitution failures, while `valid<two, int, long>`
  succeeds.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer but emitted LowIR treating the under-arity probes as
  true, so it would return early from the wrong branches. This is treated as
  stale historical evidence now that local `dev/cppgm++` is the canonical ref
  source.
- Current disposition: promoted to
  `pa22/tests/general/400-template-template-alias-default-arity-sfinae.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a
  byte-identical duplicate of the promoted PA22 reducer.

### Alias Template-Template Defaulted SFINAE Canonical Args

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-alias-template-template-defaulted-sfinae-canonical-args.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-alias-template-template-defaulted-sfinae-canonical-args.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown type
  `if_<valid<F,T...>,defer_impl<F,T...>,no_type>`; Opus commit `1a232426b`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference compiler
  rejected the reducer with unsupported alias template instantiation for `any`.
  This is treated as stale historical evidence now that local `dev/cppgm++` is
  the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-alias-template-template-defaulted-sfinae-canonical-args.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 574/574 after PA23 integration restoration. The PA23 source was restored and retained as integration coverage for MP11-style alias canonical arguments; the PA22 reducer remains focused earlier coverage.

### Async Initiate Dependent Return SFINAE

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-async-initiate-dependent-return-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-async-initiate-dependent-return-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with no member
  `async_wait<int>`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-async-initiate-dependent-return-sfinae.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 574/574 after PA23 integration restoration. The PA23 source was restored and retained as integration coverage for async-initiate dependent return SFINAE; the PA22 reducer remains focused earlier coverage.

### Constructor SFINAE Member-Template Value

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-constructor-sfinae-member-template-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-constructor-sfinae-member-template-value.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  constructor shape; Opus commit `1a232426b` emits the selected constructor
  template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference compiler
  rejected the reducer after a semantic fallback for explicit type template
  argument resolution in `supportable_properties`. This is treated as stale
  historical evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-constructor-sfinae-member-template-value.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 574/574 after PA23 integration restoration. The PA23 source was restored and retained as integration coverage for constructor/member-template value SFINAE; the PA22 reducer remains focused earlier coverage.

### Defaulted Pack Bool Short-Circuit SFINAE

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-defaulted-pack-bool-short-circuit-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-defaulted-pack-bool-short-circuit-sfinae.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  constructor shape; Opus commit `1a232426b` emits the selected constructor
  template.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations and a
  different alias object surface than current PA22 LowIR. This is treated as
  stale historical evidence now that local `dev/cppgm++` is the canonical ref
  source.
- Current disposition: promoted to
  `pa22/tests/general/500-defaulted-pack-bool-short-circuit-sfinae.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 574/574 after PA23 integration restoration. The PA23 source was restored and retained as integration coverage for defaulted pack bool short-circuit constructor SFINAE; the PA22 reducer remains focused earlier coverage.

### Member-Template Enable-If Redeclaration Overload

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-member-template-enable-if-redeclaration-overload.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-member-template-enable-if-redeclaration-overload.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a body returning
  `0` for the pointer overload; Opus commit `1a232426b` emits the selected
  pointer overload returning `2`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-member-template-enable-if-redeclaration-overload.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a normalized
  duplicate of the promoted PA22 reducer.

### Source-Owner Member-Template SFINAE Default

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-source-owner-member-template-sfinae-default.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-source-owner-member-template-sfinae-default.t`
- Historical evidence: Opus start `1963d796e` rejects with no matching function
  for `insert`; Opus commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer; local `dev/cppgm++` generated
  the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer, but emitted extra EH runtime declarations that current
  PA22 LowIR does not emit. This is treated as stale historical evidence now
  that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-source-owner-member-template-sfinae-default.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 574/574 after PA23 integration restoration. The PA23 source was restored and retained as integration coverage for source-owner member-template SFINAE defaults; the PA22 reducer remains focused earlier coverage.

### TCC Member Constructible Pack SFINAE

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-tcc-member-constructible-pack-sfinae.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-tcc-member-constructible-pack-sfinae.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `1a232426b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: emits the constrained `make<int>` overload
  returning `11`; local `dev/cppgm++` generated the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference binary
  accepted the reducer but emitted LowIR selecting the variadic fallback
  returning `4`. This is treated as stale historical evidence now that local
  `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-tcc-member-constructible-pack-sfinae.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- Validation: grouped focused PA22 `check` passed, grouped focused direct LowIR
  compare passed, PA22 placement audit passed with `--fail-on-early`, and
  PA22/PA23 report passed 569/569. The PA23 source was retired as a
  byte-identical duplicate of the promoted PA22 reducer.

### Out-Of-Class SFINAE Member-Template Alias Body

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/spec/300-out-of-class-sfinae-member-template-alias-body.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-sfinae-member-template-alias-body.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  out-of-class member-template body shape; Opus commit `1a232426b` emits the
  concrete body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer, emits the concrete
  out-of-class `assign` body, and generated local canonical PA22 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations and
  left the out-of-class `assign` body as an external declaration. This is
  treated as stale historical evidence now that local `dev/cppgm++` is the
  canonical ref source.
- Current disposition: promoted to
  `pa22/tests/spec/300-out-of-class-sfinae-member-template-alias-body.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond out-of-class
  member-template body emission through alias-SFINAE redeclaration.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted the wrong out-of-class member-template body shape;
  Opus fix `1a232426b` emitted the concrete `assign` body; local canonical PA22
  refs were generated from `dev/cppgm++`.

### Out-Of-Class SFINAE Member-Template Body

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/spec/300-out-of-class-sfinae-member-template-body.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-sfinae-member-template-body.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits the wrong
  selected out-of-class member-template body; Opus commit `1a232426b` emits the
  selected bidirectional-iterator overload body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA22 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/spec/300-out-of-class-sfinae-member-template-body.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond selected
  out-of-class member-template body emission.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted the wrong selected out-of-class member-template
  body; Opus fix `1a232426b` emitted the selected bidirectional-iterator
  overload body; local canonical PA22 refs were generated from `dev/cppgm++`.

### Defaulted NTTP Qualified Alias Value

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-defaulted-nontype-qualified-alias-value.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-defaulted-nontype-qualified-alias-value.t`
- Historical evidence: Opus start `1963d796e` rejects with a parse error; Opus
  commit `72e996289` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA22 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-defaulted-nontype-qualified-alias-value.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond
  constructor-template deduction with a defaulted NTTP depending on a qualified
  alias-template member value.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected with a parse error; Opus fix `72e996289` accepted;
  local canonical PA22 refs were generated from `dev/cppgm++`.

### Alias NTTP Expression Declaration Scope

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-alias-nontype-expression-declaration-scope.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-alias-nontype-expression-declaration-scope.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `meta::check<meta::list<int,char>,1>::value`; Opus commit `3bec27bf4`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external reference behavior: an older external reference binary
  rejected the reducer while evaluating the non-type template argument
  `(1 <= size<meta::list<int, char>>::value)`. This is treated as stale
  historical evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa21/tests/general/400-alias-nontype-expression-declaration-scope.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA21 reducer;
  it did not add a separate PA23 integration ingredient beyond alias-template
  non-type expression evaluation in declaration scope.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected with undeclared
  `meta::check<meta::list<int,char>,1>::value`; Opus fix `3bec27bf4` accepted;
  local canonical PA21 refs were generated from `dev/cppgm++`.

### Member Alias Template Owner Rebind Cache

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-member-alias-template-owner-rebind-cache.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-member-alias-template-owner-rebind-cache.t`
- Historical evidence: Opus start `1963d796e` rejects with non-constant
  `static_assert`; Opus commit `3bec27bf4` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA21 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa21/tests/general/400-member-alias-template-owner-rebind-cache.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA21 reducer;
  it did not add a separate PA23 integration ingredient beyond member alias
  owner rebinding.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected with a non-constant `static_assert`; Opus fix
  `3bec27bf4` accepted; local canonical PA21 refs were generated from
  `dev/cppgm++`.

### Out-Of-Class Partial Owner Constructor Using Alias

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/300-out-of-class-partial-owner-ctor-using-alias.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-out-of-class-partial-owner-ctor-using-alias.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a different
  constructor body shape; Opus commit `45820ae4b` emits the selected
  out-of-class constructor-template body.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA22 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/300-out-of-class-partial-owner-ctor-using-alias.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond the
  out-of-class partial-owner constructor-template alias shape.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted the older constructor body shape; Opus fix
  `45820ae4b` emitted the selected out-of-class constructor-template body; local
  canonical PA22 refs were generated from `dev/cppgm++`.

### Alias Template Function Argument CV

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-alias-template-function-argument-cv.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-alias-template-function-argument-cv.t`
- Historical evidence: Opus start `1963d796e` exits 0 but emits a different
  constructor-template LowIR shape; Opus commit `45820ae4b` emits the expected
  handler/invoker shape.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: an older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA22 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/400-alias-template-function-argument-cv.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond the
  constructor-template alias-argument shape.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted the older constructor-template LowIR shape; Opus
  fix `45820ae4b` emitted the expected handler/invoker shape; local canonical
  PA22 refs were generated from `dev/cppgm++`.

### Alias Pack Function Return Concrete

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-alias-pack-function-return-concrete.t`
- Candidate owner: PA22 for the portable tuple-element form; PA34 remains the
  owner for direct `__type_pack_element` intrinsic behavior.
- Reducer: `analysis/reducers/pa22-alias-pack-function-return-concrete.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared `get<1>`;
  Opus commit `45820ae4b` accepts.
- C++11 syntax check: `g++ -std=c++11 -x c++ -fsyntax-only` and
  `clang++ -std=c++11 -x c++ -fsyntax-only` accept the portable reducer after
  replacing `__type_pack_element` with a recursive local `pack_element` helper.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: the older external reference
  binary accepted the reducer, but emitted extra EH runtime declarations that
  current PA22 LowIR does not emit. This is treated as stale historical evidence
  now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-alias-pack-function-return-concrete.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a near-duplicate. The PA23 source had
  already been rewritten to the same portable recursive `pack_element` helper
  and did not add a separate PA23 integration ingredient. Intrinsic-specific
  `__type_pack_element` coverage remains later-owned.
- Validation: `g++` and `clang++` C++11 syntax checks passed; local
  `dev/cppgm++` accepted; Opus start `1963d796e` rejected; Opus fix
  `45820ae4b` accepted; focused PA22 check and direct LowIR compare passed;
  PA22 placement audit passed with `--fail-on-early`.

### Dependent Alias Decltype Qualified Function Source Tokens

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-dependent-alias-decltype-qualified-function-source-tokens.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-dependent-alias-decltype-qualified-function-source-tokens.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown sizeof
  operand type; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external reference behavior: an older external reference binary
  rejected the reducer with unsupported template declarator in the qualified
  function-template helper declaration. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-dependent-alias-decltype-qualified-function-source-tokens.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer;
  it did not add a separate PA23 integration ingredient beyond dependent alias
  SFINAE with a qualified `decltype` function-template call.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected with unknown sizeof operand type; Opus fix
  `45820ae4b` accepted; local canonical PA22 refs were generated from
  `dev/cppgm++`.

### Decltype Trailing Pack Template-Id Deduction

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row:
  `pa23/tests/general/200-decltype-trailing-pack-template-id-deduction.t`
- Candidate owner: PA22.
- Reducer:
  `analysis/reducers/pa22-decltype-trailing-pack-template-id-deduction.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 row; the saved
  reducer was blocked before promotion by an older external reference compiler.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Historical gate: Opus start `1963d796e` rejects with
  `ERROR: unsupported decltype operand`; Opus commit `b449e7c8c` accepts after
  expanding a function parameter pack whose pattern nests the pack in a
  template-id.
- Current compiler behavior: accepts the exact source and generated local
  canonical PA22 refs.
- Historical external reference behavior: an older external reference compiler
  rejected the reducer with a failed `static_assert` because
  `decltype(f(static_cast<identity<T>*>(0)...))` did not preserve the deduced
  trailing pack as `list<A, B>`. This is treated as stale historical evidence
  now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted the exact PA23 source to
  `pa22/tests/general/300-decltype-trailing-pack-template-id-deduction.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical focused PA22 reducer
  for decltype-of-call deduction through a trailing pack template-id pattern.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `b449e7c8c` accepted; local canonical
  PA22 refs were generated from `dev/cppgm++`.

### Static Data NTTP Pack Sizeof Bound

- Disposition: `resolved-local-canonical-promoted`
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
  static data; local `dev/cppgm++` generated the canonical PA22 refs.
- Historical reference workflow behavior: the older external reference compiler
  rejected the out-of-class static data member definition with
  `sizeof...(Bytes)` in the array bound as an unsupported template declarator.
  This is treated as stale historical evidence now that local `dev/cppgm++` is
  the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/400-static-data-nttp-pack-sizeof-bound.t`; tracker row
  marked `missing-earlier-feature` / `test-added`.
- Validation: focused PA22 `check` passed, focused direct LowIR compare passed,
  PA22 placement audit passed with `--fail-on-early`, and PA22/PA23 report
  passed 569/569. The PA23 source is byte-identical to the promoted
  reducer and was retired because it is an isolated single-feature test,
  not a PA23 integration composition.

### Function Template Parameter Alias Sees Previous Param

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-function-template-parameter-alias-sees-previous-param.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-function-template-parameter-alias-sees-previous-param.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown sizeof
  operand type; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external reference behavior: rejects the reducer with unknown
  function `helper<value>`; this older Opus-generated reference is not the
  current canonical source.
- Current disposition: promoted to
  `pa22/tests/general/500-function-template-parameter-alias-sees-previous-param.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical duplicate of the
  promoted PA22 focused reducer.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `45820ae4b` accepted; focused PA22
  check and direct LowIR compare passed; PA22 placement audit passed with
  `--fail-on-early`.

### Index Sequence Alias Constructor Deduction

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/500-index-sequence-alias-constructor-deduction.t`
- Candidate owner: PA22
- Reducer: `analysis/reducers/pa22-index-sequence-alias-constructor-deduction.t`
- Historical evidence: Opus start `1963d796e` rejects with undeclared
  `index_sequence<0>`; Opus commit `45820ae4b` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA22 refs.
- Historical external/reference workflow behavior: the older external
  reference binary accepted the reducer, but emitted extra EH runtime
  declarations and an empty helper constructor body not emitted by current
  PA22. This is treated as stale historical evidence now that local
  `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa22/tests/general/500-index-sequence-alias-constructor-deduction.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical duplicate of the
  promoted PA22 focused reducer.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `45820ae4b` accepted; focused PA22
  check and direct LowIR compare passed; PA22 placement audit passed with
  `--fail-on-early`.

### Alias Pack NTTP Expression Fast Path

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-alias-pack-nontype-expression-fast-path.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-alias-pack-nontype-expression-fast-path.t`
- Historical evidence: Opus start `1963d796e` rejects with shadowed template
  parameter `L`; Opus commit `743b7a920` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external reference behavior: rejects the reducer with unsupported
  alias template instantiation for `count_if`; this older Opus-generated
  reference is not the current canonical source.
- Current disposition: promoted to
  `pa21/tests/general/400-alias-pack-nontype-expression-fast-path.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical duplicate of the
  promoted PA21 focused reducer.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `743b7a920` accepted; focused PA21
  check and direct LowIR compare passed; PA21 placement audit passed with
  `--fail-on-early`.

### Qualified Template-Id Current-Scope Alias Shadow

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-qualified-template-id-current-scope-alias-shadow.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-qualified-template-id-current-scope-alias-shadow.t`
- Historical evidence: Opus start `1963d796e` rejects with unknown
  `mp_if<bool_constant<true>,transform_impl<F,L...>,mismatch>::type`; Opus
  commit `743b7a920` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external reference behavior: the older external reference binary
  segfaults while compiling the reducer; this older Opus-generated reference is
  not the current canonical source.
- Current disposition: promoted to
  `pa21/tests/general/400-qualified-template-id-current-scope-alias-shadow.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a byte-identical duplicate of the
  promoted PA21 focused reducer.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `743b7a920` accepted; focused PA21
  check and direct LowIR compare passed; PA21 placement audit passed with
  `--fail-on-early`.

### Nested Template-Id Partial Specialization Deduction

- Disposition: `historical-validation-missing`
- PA23 source row:
  `pa23/tests/general/200-nested-template-id-partial-specialization-deduction.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-nested-template-id-partial-specialization-deduction.t`
- Historical evidence: the PA23-shaped source rejects at Opus start
  `1963d796e` with unknown type
  `aligned_storage<sizeof(T),alignment_of<T>::value>::type`; the reduced
  oracle rejects at Opus start with a failed `static_assert`. Under the
  corrected `--emit-lowir -o` gate, Opus final `1434a6e11` still rejects with
  the same failed `static_assert`, while current local `dev/cppgm++` accepts.
  No reachable Opus start/fix transition has been identified for a
  promotable assignment test.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: `./dev/cppgm++ --emit-lowir -O0 -o ...` accepts
  the reducer.
- Historical external reference behavior: rejects the reduced oracle with a
  failed `static_assert`, leaving `value_type` as the fallback primary type
  instead of selecting `node_value<base_node<T, hook<...>, true> >`. The
  external reference also rejects the PA23-shaped source while evaluating
  `sizeof(T)` for
  `lib::base_node<pair_like<recursive_map const, recursive_map>, ...>`.
- Current disposition: tracker row marked `historical-validation-missing` /
  `no-action`; no PA21 assignment test added.
- Next validation: find or implement a reachable Opus/current transition for
  nested class-template-id partial-specialization matching before promoting this
  row; local current acceptance alone is not enough for the Opus-gated backfill
  workflow.

### Template Instantiation Use Location Explicit Specialization

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row:
  `pa23/tests/general/100-template-instantiation-use-location-explicit-specialization.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-template-instantiation-use-location-explicit-specialization-value-object.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with unknown
  type `ops<policy>::iter_move` and rejects the exact PA23 source with unknown
  type `Ops<Policy>::iter_move`; Opus commit `fb9e82eeb` accepts both shapes.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer and exact PA23 source.
- Current compiler behavior: accepts the exact PA23 source, emits construction
  of the local array element plus the move construction of the `sift` local
  object, and generated local canonical PA21 refs.
- Historical external reference behavior: an older external reference binary
  accepted the reducer but omitted constructor work that current emits. This is
  treated as stale historical evidence now that local `dev/cppgm++` is the
  canonical ref source.
- Current disposition: promoted the exact PA23 source to
  `pa21/tests/general/300-template-instantiation-use-location-explicit-specialization.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a focused PA21 reducer; the saved
  reducer only differs by naming and a typedef removal, and the source does not
  add a separate PA23 integration ingredient beyond use-location explicit
  specialization for a member function template call.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `fb9e82eeb` accepted; local canonical
  PA21 refs were generated from `dev/cppgm++`.

### Explicit Instantiation Static Member Function

- Disposition: `resolved-local-canonical-promoted`
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
- Historical gate: Opus start `1963d796e` accepts but emits the static member
  function without `object_root=yes`; Opus commit `569fcbd79` accepts and emits
  the explicit-instantiation root marker.
- Current compiler behavior: accepts the reducer, emits the explicitly
  instantiated static member function with `object_root=yes`, and generated
  local canonical PA21 refs.
- Historical external reference behavior: an older external reference binary
  accepted the reducer but omitted the `object_root` metadata. This is treated
  as stale historical evidence now that local `dev/cppgm++` is the canonical ref
  source.
- Current disposition: promoted the exact PA23 source to
  `pa21/tests/spec/300-explicit-instantiation-static-member-function.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as a focused PA21 reducer; the saved
  reducer only differs by comments/formatting, and the source does not add a
  separate PA23 integration ingredient beyond explicit instantiation of a static
  member function.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted no explicit-instantiation root marker; Opus fix
  `569fcbd79` emitted `object_root=yes`; local canonical PA21 refs were
  generated from `dev/cppgm++`.

### Explicit Specialization Cross Converting Constructor Body

- Disposition: `resolved-local-canonical-promoted`
- PA23 source rows:
  `pa23/tests/general/100-explicit-specialization-cross-converting-ctor-body.t`,
  `pa23/tests/spec/100-explicit-specialization-cross-converting-ctor-body.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-explicit-specialization-cross-converting-ctor-body.t`
- Historical evidence: the PA23 source rows failed the Opus PA23 start report by
  relaxed LowIR comparison. The value-object reducer keeps the start failure by
  forcing the converting constructor body to be emitted.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Historical gate: Opus start `1963d796e` accepts but emits both the selected
  constructor body and a duplicate `__base_entry` body; Opus commit
  `0f4ce78a9` accepts and emits only the single selected constructor body.
- Current compiler behavior: accepts the reducer, emits constructor object
  metadata and C2 aliases without EH runtime declarations for this non-throwing
  shape, and generated local canonical PA21 refs.
- Historical external reference behavior: an older external reference binary
  accepted the reducer but emitted EH runtime declarations and omitted
  constructor aliases. This is treated as stale historical evidence now that
  local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted the focused reducer to
  `pa21/tests/spec/300-explicit-specialization-cross-converting-ctor-body.t`;
  tracker rows marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed both PA23 duplicates. They added a second
  `imag` field/accessor around the same explicit-specialization converting
  constructor-body issue, but no separate PA23 integration ingredient beyond
  the focused PA21 reducer.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` emitted the duplicate `__base_entry` constructor body; Opus
  fix `0f4ce78a9` emitted one selected constructor body; local canonical PA21
  refs were generated from `dev/cppgm++`.

### Class Partial Specialization Matching/Ordering Batch

- Disposition: `resolved-local-canonical-promoted`
- Candidate owner: PA21
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: local canonical `dev/cppgm++` accepts every
  reducer listed below and generated refs.
- Older reference behavior: an older external reference lane rejected these
  reducers; this is now treated as stale historical evidence rather than a
  current blocker.
- Current disposition: promoted to PA21 focused tests listed below; the PA23
  originals were retired because they were the same focused single-feature
  probes rather than useful PA23 integration compositions. Tracker rows marked
  `missing-earlier-feature` / `test-added`.
- Validation: grouped PA21 `check` passed, and grouped direct LowIR compare
  passed with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make -C pa21 check
  TEST='tests/general/400-nontype-pack-fixed-tail-partial-specialization-ordering.t
  tests/general/400-cv-qualified-template-id-wrapper-class-partial-specialization.t
  tests/general/400-dependent-owner-member-class-template-partial-specialization.t
  tests/general/400-function-type-cv-partial-specialization.t
  tests/general/400-function-type-ref-qualified-partial-specialization.t
  tests/general/400-repeated-pack-partial-specialization-ordering.t'`.
  Focused PA21/PA23 report passed 579/579. A follow-up PA19-PA23 direct LowIR
  report passed after refreshing stale refs from the placement-reducer batch.

| PA23 source row | Reducer | Promoted test | Historical evidence | Older external reference behavior |
| --- | --- | --- | --- | --- |
| `pa23/tests/general/200-nontype-pack-fixed-tail-partial-specialization-ordering.t` | `analysis/reducers/pa21-nontype-pack-fixed-tail-partial-specialization-ordering.t` | `pa21/tests/general/400-nontype-pack-fixed-tail-partial-specialization-ordering.t` | Opus start `1963d796e` rejects; Opus commit `ee034cde3` accepts. | Rejects while resolving `bin_literal<bytes<>,'0','0','0','0','0','0','0','0'>::size`. |
| `pa23/tests/general/400-cv-qualified-template-id-wrapper-class-partial-specialization.t` | `analysis/reducers/pa21-cv-qualified-template-id-wrapper-class-partial-specialization.t` | `pa21/tests/general/400-cv-qualified-template-id-wrapper-class-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `a3c294645` accepts. | Rejects as an ambiguous partial class specialization for `holder<box<int,long> const>`. |
| `pa23/tests/general/400-dependent-owner-member-class-template-partial-specialization.t` | `analysis/reducers/pa21-dependent-owner-member-class-template-partial-specialization.t` | `pa21/tests/general/400-dependent-owner-member-class-template-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `0cf7d2de0` accepts. | Rejects while collecting the out-of-class member class-template partial specialization. |
| `pa23/tests/general/400-function-type-cv-partial-specialization.t` | `analysis/reducers/pa21-function-type-cv-partial-specialization.t` | `pa21/tests/general/400-function-type-cv-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `b444d928a` accepts. | Rejects as an ambiguous partial class specialization for `holder<void()>`. |
| `pa23/tests/general/400-function-type-ref-qualified-partial-specialization.t` | `analysis/reducers/pa21-function-type-ref-qualified-partial-specialization.t` | `pa21/tests/general/400-function-type-ref-qualified-partial-specialization.t` | Opus start `1963d796e` rejects; Opus commit `f31e61547` accepts. | Rejects with failed `static_assert` for `holder<void() &>::value == 2`. |
| `pa23/tests/general/400-repeated-pack-partial-specialization-conflict.t` and `pa23/tests/general/400-repeated-pack-partial-specialization-preference.t` | `analysis/reducers/pa21-repeated-pack-partial-specialization-ordering.t` | `pa21/tests/general/400-repeated-pack-partial-specialization-ordering.t` | Opus start `1963d796e` rejects; Opus commit `b444d928a` accepts. | Rejects as an ambiguous partial class specialization for `similar_impl<list<>,list<> >`. |

### Top-CV Pointer Does Not Match Unqualified Pointer Partial

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row:
  `pa23/tests/general/100-top-cv-pointer-does-not-match-unqualified-pointer-partial.t`
- Candidate owner: PA21.
- Reducer:
  `analysis/reducers/pa21-top-cv-pointer-does-not-match-unqualified-pointer-partial.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with a
  failed `static_assert`; Opus commit `15a2e9c8d` accepts the reducer.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: local canonical `dev/cppgm++` accepts the reducer
  and generated refs.
- Older reference behavior: an older external reference lane rejected the
  reducer as an ambiguous partial class specialization for
  `holder<B const* volatile>`, allowing the unqualified pointer partial to
  compete with the top-level volatile wrapper. This is now treated as stale
  historical evidence rather than a current blocker.
- Current disposition: promoted to
  `pa21/tests/general/100-top-cv-pointer-does-not-match-unqualified-pointer-partial.t`;
  tracker row marked `missing-earlier-feature` / `test-added`. The PA23
  original was retired as the same focused single-feature probe.
- Validation: focused PA21 `check` passed; focused direct LowIR compare passed;
  PA21 placement audit passed with `--fail-on-early`.

### Qualified Friend Member Function Template

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-qualified-friend-member-template-access.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-qualified-friend-member-template.t`
- Historical evidence: Opus start `1963d796e` rejects the original value-object
  reducer with `unknown type name: iter<C,B>`; Opus commit `670045ef2` accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external/reference workflow behavior: the older external
  reference binary accepts the reducer, but emits extra EH runtime declarations
  that current PA21 LowIR does not emit. This is treated as stale historical
  evidence now that local `dev/cppgm++` is the canonical ref source.
- Current disposition: promoted to
  `pa21/tests/general/300-qualified-friend-member-template-access.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as the same focused value-object qualified
  friend member-template access shape as the promoted PA21 reducer.
- Validation: C++11 syntax checks passed for the saved reducer and exact PA23
  source; local `dev/cppgm++` accepted; Opus start `1963d796e` rejected both
  source shapes; Opus fix `670045ef2` accepted both; focused PA21 check and
  direct LowIR compare passed; PA21 placement audit passed with
  `--fail-on-early`.

### Member Template As Qualified Template-Template Argument

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row: `pa23/tests/general/400-member-alias-template-template-owner-argument.t`
- Candidate owner: PA21
- Reducer: `analysis/reducers/pa21-member-template-as-template-template-argument.t`
- Historical evidence: Opus start `1963d796e` rejects the reducer with
  `unknown type name: use<quote::template fn>::type`; Opus commit `670045ef2`
  accepts.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and generated local canonical
  PA21 refs.
- Historical external reference behavior: rejects the type-only reducer with
  failed `static_assert` because `result_t` remains `use<quote::fn>::type`
  instead of resolving to `int`; this older Opus-generated reference is not the
  current canonical source.
- Current disposition: promoted to
  `pa21/tests/general/300-member-template-as-template-template-argument.t`;
  tracker row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: retained as PA23 integration coverage because it
  combines a variadic alias member template, pack forwarding, and
  template-template application through `quote`/`apply0`, while the PA21 reducer
  isolates qualified member-template-as-template-template argument lookup.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `670045ef2` accepted; focused PA21
  check and direct LowIR compare passed; PA21 placement audit passed with
  `--fail-on-early`.

### Dependent Qualified Sizeof Static Member

- Disposition: `no-backfill-start-pass`
- PA23 source row:
  `pa23/tests/general/500-dependent-qualified-sizeof-static-member.t`
- Candidate owner: PA22, using PA20 constant-evaluation support for `sizeof`.
- Reducer:
  `analysis/reducers/pa22-dependent-qualified-sizeof-static-member.t`
- Historical evidence: the original Opus PA23 start source used hosted
  predefined macro `__SIZE_TYPE__`. After the portable rewrite to
  `unsigned long`, Opus start `1963d796e` accepts the exact PA23 source and
  emits `observed : i64 = 3`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer and the exact PA23 source.
- Historical external reference behavior: an older reference lane rejected the
  reducer with `ERROR: invalid sizeof type-id` while evaluating
  `sizeof(bucket_array_base<Flag>::sizes) /
  sizeof(bucket_array_base<Flag>::sizes[0])`. This no longer blocks a current
  local canonical ref, but the reducer also no longer has the required Opus
  start/fix signal after the hosted macro is removed.
- Current disposition: no PA22 assignment test added; tracker row marked
  `no-missing-opus-feature` / `no-action`.
- Next validation: none for the portable PA22 reducer. Revisit only if a
  smaller dependent-qualified `sizeof` shape is found that fails at Opus start
  for the language behavior rather than for PA34 hosted predefined-macro
  compatibility.

### Hidden Friend Query Free Decltype Noexcept

- Disposition: `resolved-local-canonical-promoted`
- PA23 source row:
  `pa23/tests/spec/500-hidden-friend-query-free-decltype-noexcept.t`
- Candidate owner: PA22.
- Reducer:
  `analysis/reducers/pa22-hidden-friend-query-free-decltype-noexcept.t`
- Historical evidence: Opus start `1963d796e` rejects the PA23 source row. The
  nonlocal Boost frontier notes tracked this as `c047d6e93`, but the reachable
  local Opus transition is `ca52fe6ac`: commit `b50d7227e` still rejects with
  unknown `enable_if<call_traits<...>>::type`, while `ca52fe6ac` accepts after
  building member-template generic dump bodies best-effort.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the exact source and generated local
  canonical PA22 refs.
- Historical external reference behavior: an older external reference compiler
  rejected the reducer with a failed `static_assert` for
  `is_same<query_probe::result_type, int>::value`; it let the later ordinary
  `query` CPO object supply a `char` result instead of finding the hidden friend
  `query` returning `int` by ADL at the template definition point. This is
  treated as stale historical evidence now that local `dev/cppgm++` is the
  canonical ref source.
- Current disposition: promoted the exact PA23 source to
  `pa22/tests/spec/500-hidden-friend-query-free-decltype-noexcept.t`; tracker
  row marked `missing-earlier-feature` / `test-added`.
- PA23 original disposition: removed as focused PA22 deduction/SFINAE coverage;
  the saved reducer only differs by comments and the source does not add a
  separate PA23 integration ingredient beyond the hidden-friend
  `decltype`/`noexcept` query.
- Validation: C++11 syntax check passed; local `dev/cppgm++` accepted; Opus
  start `1963d796e` rejected; Opus fix `ca52fe6ac` accepted; local canonical
  PA22 refs were generated from `dev/cppgm++`.

### Constructor/Conversion Deduction Reference-Blocked Batch

- Disposition: mixed. The first five PA22 reducers and the PA21
  member-template pack-binding reducer are `resolved-local-canonical-promoted`;
  the trusted-only nested-member row remains unresolved.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: local canonical `dev/cppgm++` accepts every
  reducer listed below and generated refs for the five promoted PA22 reducers.
- Older reference behavior: an older external reference lane rejected these
  reducers; this is now treated as stale historical evidence for the promoted
  PA22 rows rather than a current blocker.
- Current disposition: promoted PA22 tests listed below; the corresponding
  PA23 originals were retired because they were focused single-feature probes.
  The member-template pack-binding reducer was retargeted from PA18 to PA21
  because PA18 explicitly excludes parameter packs and member templates.
  Tracker rows for the promoted tests are marked `missing-earlier-feature` /
  `test-added`.
- Validation for promoted PA22 rows: grouped PA22 `check` passed, grouped
  direct LowIR compare passed, and PA22 placement audit passed with
  `--fail-on-early`. `constructor-template-pack-before-defaulted-nontype` was
  placed at `pa22:400` because the pointer/reference NTTP guard is first owned
  there.

| PA23 source row | Candidate owner | Reducer | Promoted test | Historical evidence | Older external reference behavior / next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-constructor-template-rvalue-beats-const-ref.t` | PA22 | `analysis/reducers/pa22-constructor-template-rvalue-beats-const-ref.t` | `pa22/tests/general/200-constructor-template-rvalue-beats-const-ref.t` | Opus start `1963d796e` rejects deleted copy construction; Opus `f8660aa27` accepts. | Older external lane rejected after selecting the `pair(const A&, const B&)` path and failing move-only construction. |
| `pa23/tests/general/200-inherited-constructor-template-forwarding.t` | PA22 | `analysis/reducers/pa22-inherited-constructor-template-forwarding.t` | `pa22/tests/general/200-inherited-constructor-template-forwarding.t` | Opus start `1963d796e` rejects `using base<T>::base`; Opus `f8660aa27` accepts. | Older external lane rejected `derived<int>(arg&&)` because the inherited base constructor template was missing from the derived constructor set. |
| `pa23/tests/general/300-constructor-template-defaulted-forwarding-lvalue-order.t` | PA22 | `analysis/reducers/pa22-constructor-template-defaulted-forwarding-lvalue-order.t` | `pa22/tests/general/300-constructor-template-defaulted-forwarding-lvalue-order.t` | Opus start `1963d796e` rejects the source call shape; Opus `f8660aa27` accepts. | Older external lane rejected as ambiguous between `box(const T&)` and the defaulted forwarding constructor template for a const lvalue source. |
| `pa23/tests/general/300-constructor-template-pack-before-defaulted-nontype.t` | PA22 | `analysis/reducers/pa22-constructor-template-pack-before-defaulted-nontype.t` | `pa22/tests/general/400-constructor-template-pack-before-defaulted-nontype.t` | Opus start `1963d796e` rejects with a parse error; Opus `f8660aa27` accepts. | Older external lane rejected `tuple_like<first_arg, second_arg>{first, second}` after losing the defaulted pointer parameter shape. |
| `pa23/tests/general/400-conversion-function-template-prefers-nontemplate.t` | PA22 | `analysis/reducers/pa22-conversion-function-template-prefers-nontemplate.t` | `pa22/tests/general/400-conversion-function-template-prefers-nontemplate.t` | Opus start `1963d796e` accepts but emits a direct `box(int)` assignment path; Opus `f8660aa27` emits the non-template conversion-function path. | Older external lane rejected assignment as ambiguous instead of preferring the non-template conversion function. |
| `pa23/tests/general/400-member-template-class-pack-forward-before-token.t` | PA21 | `analysis/reducers/pa21-member-template-class-pack-forward-before-token.t` | `pa21/tests/general/300-member-template-class-pack-forward-before-token.t` | Opus start `1963d796e` rejects with no member `send`; Opus `f8660aa27` accepts. | Older external lane rejects `combine(static_cast<Args&&>(args)...)` because pack value binding pairs the first `Args` element with the second value argument. |
| `pa23/tests/general/400-nested-member-template-base-param-shadow-value.t` | PA21 | `analysis/reducers/pa21-nested-member-template-base-param-shadow-value.t` | not promoted | Correct Opus gate accepts the reducer at start `1963d796e`, Opus final `1434a6e11`, and current. | No backfill: the focused shape has no start/fix transition under the current workflow. |

### Template-Template/Alias Forwarding Reference-Blocked Batch

- Disposition: mixed. Six reducers are
  `resolved-local-canonical-promoted`; the qualified-default deduction row is
  `no-backfill-start-pass`.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: local canonical `dev/cppgm++` accepts every
  reducer listed below and generated refs for the six promoted reducers.
- Current disposition: promoted tests listed below; tracker rows for the
  promoted tests are marked `missing-earlier-feature` / `test-added`. The
  qualified-default row remains in PA23 because Opus start already accepts the
  focused shape and no earlier backfill is needed.
- Validation: grouped PA21 and PA22 `check` commands passed; grouped PA21 and
  PA22 direct LowIR checks passed; PA21 and PA22 placement audits passed with
  `--fail-on-early`; `make test-report ACTIVE_TEST_REPORT_PAS='pa21 pa22 pa23'`
  passed. A follow-up
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
  ACTIVE_TEST_REPORT_PAS='pa19 pa20 pa21 pa22 pa23'` passed after refreshing
  stale refs from the placement-reducer batch.

| PA23 source row | Candidate owner | Reducer | Promoted test | Historical evidence | Older external reference behavior / next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-function-template-variadic-template-template-deduction.t` | PA22 | `analysis/reducers/pa18-function-template-variadic-template-template-deduction.t` | `pa22/tests/general/100-function-template-variadic-template-template-deduction.t` | Opus start `1963d796e` rejects with unknown `dispatch`; Opus `2c47e3a86` accepts. | Older external lane rejected the valid variadic template-template call as unknown function `dispatch`; local canonical refs now generated. PA18 was rejected as owner because PA18 explicitly excludes parameter packs and template-template parameters. |
| `pa23/tests/general/200-template-template-qualified-default-arg-deduction.t` | none for backfill | `analysis/reducers/pa18-template-template-qualified-default-arg-deduction.t` | not promoted | Opus start `1963d796e` accepts the focused source shape. | No earlier assignment test added; tracker row marked `no-missing-opus-feature` / `no-action`. |
| `pa23/tests/general/400-defaulted-nested-cv-template-template-partial-specialization.t` | PA21 | `analysis/reducers/pa21-defaulted-nested-cv-template-template-partial-specialization.t` | `pa21/tests/general/400-defaulted-nested-cv-template-template-partial-specialization.t` | Opus start `1963d796e` rejects with failed `static_assert`; Opus `33a806b05` accepts. | Older external lane selected the generic/default-truncated shape instead of the defaulted nested-cv TTP partial; local canonical refs now generated. |
| `pa23/tests/general/400-member-alias-template-template-dependent-replay.t` | PA21 | `analysis/reducers/pa21-member-alias-template-template-dependent-replay.t` | `pa21/tests/general/400-member-alias-template-template-dependent-replay.t` | Opus start `1963d796e` rejects with unknown `replace_if_impl<L,Q::template fn,W>::type`; Opus `670045ef2` accepts. | Older external lane rejected member alias-template replay when used as a TTP argument; local canonical refs now generated. |
| `pa23/tests/general/400-member-alias-template-template-empty-template-id-argument.t` | PA21 | `analysis/reducers/pa21-member-alias-template-template-empty-template-id-argument.t` | `pa21/tests/general/400-member-alias-template-template-empty-template-id-argument.t` | Opus start `1963d796e` rejects with unknown `detail::transform_impl<detail::flatten<n::list<>>::fn,n::list<n::list<>>>::type`; Opus `54e85f719` accepts. | Older external lane rejected member alias-template TTP application to an empty class template-id argument; local canonical refs now generated. |
| `pa23/tests/spec/500-template-template-conversion-operator-reference-target.t` | PA22 | `analysis/reducers/pa22-template-template-conversion-operator-reference-target.t` | `pa22/tests/spec/500-template-template-conversion-operator-reference-target.t` | Opus start `1963d796e` crashes/rejects; Opus `9bb9ba6ea` accepts. | Older external lane rejected the conversion-function-template reference target path for `view<char, traits> v(s)`; local canonical refs now generated. |
| `pa23/tests/spec/500-template-template-piecewise-partial-ordering.t` | PA22 | `analysis/reducers/pa22-template-template-piecewise-partial-ordering.t` | `pa22/tests/spec/500-template-template-piecewise-partial-ordering.t` | Opus start `1963d796e` rejects with unknown `fixed_dispatch`; Opus `7626d2f4f` accepts. | Older external lane rejected fixed tuple dispatch as ambiguous and did not prefer the fixed non-pack TTP pattern; local canonical refs now generated. |

### Member Template Ownership Reference-Blocked Batch

- Disposition: `reference-compiler-bug` for every reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: two rows were promotable once the historical Opus
  validation was rerun with the correct `--emit-lowir -o` invocation. The
  sibling-owner reducer is now covered by
  `pa21/tests/general/300-sibling-namespace-dependent-member-template-id-owner.t`;
  the conversion-function-template copy-init reducer is now covered by
  `pa22/tests/spec/300-conversion-function-template-owner-result-copy-init.t`.
  The recursive and out-of-class dependent-owner reducers are still no-action
  rows because the reduced shapes already pass at Opus start `1963d796e`.
- PA23 original disposition: the sibling-owner and conversion-function-template
  copy-init PA23 originals were removed as comment/format-only duplicates of
  the promoted earlier reducers; they did not add a separate PA23 integration
  ingredient.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/400-recursive-member-template-concrete-owner.t` | PA21 | `analysis/reducers/pa21-recursive-member-template-concrete-owner.t` | Correct Opus gate accepts the reducer at start `1963d796e` and all sampled fixes. | Current local compiler accepts and can generate LowIR; no reference block remains for the current workflow. | No backfill: the reduced shape has no start/fix transition. |
| `pa23/tests/general/400-sibling-namespace-dependent-member-template-id-owner.t` | PA21 | `analysis/reducers/pa21-sibling-namespace-dependent-member-template-id-owner.t` | Correct Opus gate rejects at start `1963d796e` with `unknown type name: detail::result_detail<Sig>::type` and accepts at `f8660aa27`/current. | Current local compiler accepts and generated refs. | Promoted to `pa21/tests/general/300-sibling-namespace-dependent-member-template-id-owner.t`; PA23 original retired as a comment/format-only duplicate. |
| `pa23/tests/general/500-out-of-class-member-template-dependent-owner-type.t` | PA22 | `analysis/reducers/pa22-out-of-class-member-template-dependent-owner-type.t` | Correct Opus gate accepts the reducer at start `1963d796e`, regresses at `42beb854b`/`e8325194b`, and accepts again at `fd6f3e485`/current. | Current local compiler accepts and can generate LowIR; no reference block remains for the current workflow. | No backfill from this reducer: the reduced shape is start-pass rather than start-fail. |
| `pa23/tests/spec/400-conversion-function-template-owner-result-copy-init.t` | PA22 | `analysis/reducers/pa22-conversion-function-template-owner-result-copy-init.t` | Correct Opus gate rejects at start `1963d796e` with `explicit constructor in copy-initialization` and accepts at `f8660aa27`/current. | Current local compiler accepts and generated refs. | Promoted to `pa22/tests/spec/300-conversion-function-template-owner-result-copy-init.t`; PA23 original retired as a comment/format-only duplicate. |

### Non-Type Template Argument Reference-Blocked Batch

- Disposition: no missing Opus feature for the reducer listed below.
- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts the
  reducer.
- Current compiler behavior: accepts the reducer.
- Current disposition: no assignment test added for this row; tracker row
  marked `no-missing-opus-feature` / `no-action`.

| PA23 source row | Candidate owner | Reducer | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/100-nested-template-static-value-nontype-expression.t` | PA21 | `analysis/reducers/pa21-nested-template-static-value-nontype-expression.t` | Correct Opus gate accepts both the PA23 source and reducer at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts and can generate LowIR; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |

### Pack Expansion Reference-Blocked Batch

- C++11 validity check: `g++ -std=c++11 -x c++ -fsyntax-only` accepts every
  reducer listed below.
- Current compiler behavior: accepts every reducer listed below.
- Current disposition: four exact focused sources were promotable once the
  historical Opus validation was rerun with the correct `--emit-lowir -o`
  invocation:
  `pa21/tests/general/100-nested-pack-expansion-outer-type-pack.t`,
  `pa21/tests/spec/100-variadic-base-pack-expansion.t`,
  `pa22/tests/general/100-local-pack-template-id-paren-init.t`, and
  `pa22/tests/general/100-local-pack-template-id-paren-init-in-if.t`. The
  remaining six rows are no-action because the exact focused source already
  passes at Opus start `1963d796e`.
- PA23 original disposition: the four promoted pack-expansion originals were
  removed as source-identical or comment/format-only duplicates of the promoted
  PA21/PA22 reducers. They isolated the earlier feature rather than adding
  PA23 integration coverage.

| PA23 source row | Candidate owner | Reducer | Disposition | Historical evidence | External reference behavior | Next validation |
| --- | --- | --- | --- | --- | --- | --- |
| `pa23/tests/general/200-local-pack-template-id-paren-init-in-if.t` | PA22 | PA23 source row itself | `promoted` | Correct Opus gate rejects at start `1963d796e` with `unknown type name: pair<const Args&...>` and accepts at `1434a6e11`/current. | Current local compiler accepts and generated refs. | Promoted to `pa22/tests/general/100-local-pack-template-id-paren-init-in-if.t`; placement audit suggested PA22:100. |
| `pa23/tests/general/200-local-pack-template-id-paren-init.t` | PA22 | PA23 source row itself | `promoted` | Correct Opus gate rejects at start `1963d796e` with `unknown type name: pair<const Args&...>` and accepts at `1434a6e11`/current. | Current local compiler accepts and generated refs. | Promoted to `pa22/tests/general/100-local-pack-template-id-paren-init.t`; placement audit suggested PA22:100. |
| `pa23/tests/general/200-member-function-template-address-explicit-pack.t` | PA22 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/general/300-function-template-empty-pack-trailing-default.t` | PA22 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/general/400-nested-pack-expansion-outer-type-pack.t` | PA21 | PA23 source row itself | `promoted` | Correct Opus gate rejects at start `1963d796e` with `unknown type name: impl<list<A,B>>::type` and accepts at `1434a6e11`/current. | Current local compiler accepts and generated refs. | Promoted to `pa21/tests/general/100-nested-pack-expansion-outer-type-pack.t`; placement audit suggested PA21:100. |
| `pa23/tests/general/400-pack-base-expansion.t` | PA21 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/spec/200-defaulted-class-template-argument-pack-prefix-deduction.t` | PA22 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/spec/300-constructor-default-pack-partial-ordering.t` | PA22 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/spec/400-explicit-pack-type-argument-uses-bound-type.t` | PA19 | PA23 source row itself | `no-missing-opus-feature` | Correct Opus gate accepts at start `1963d796e`, Opus final `1434a6e11`, and current. | Current local compiler accepts; no reference block remains for the current workflow. | No backfill: the focused shape has no start/fix transition. |
| `pa23/tests/spec/400-variadic-base-pack-expansion.t` | PA21 | PA23 source row itself | `promoted` | Correct Opus gate rejects at start `1963d796e` with `use of undeclared identifier: sum_bases<Tail...>::sum` and accepts at `1434a6e11`/current. | Current local compiler accepts and generated refs. | Promoted to `pa21/tests/spec/100-variadic-base-pack-expansion.t`; placement audit suggested PA21:100. |

### Portable Hosted-Builtin Rewrite Batch

- Disposition: mixed. Five hosted/vendor-intrinsic PA23 sources were rewritten
  to portable C++11 helper templates and promoted to PA21/PA22. One portable
  rewrite already passes at Opus start, and one portable sequence-filter shape
  still has no reachable Opus pass.
- Hosted-token check: the portable sources contain none of
  `__type_pack_element`, `__make_integer_seq`, `__decay`,
  `__remove_reference`, `__remove_cv`, `__remove_cvref`, or `__SIZE_TYPE__`.
- C++11/current behavior: `g++ -std=c++11 -x c++ -fsyntax-only` and local
  canonical `dev/cppgm++ --emit-lowir` accept all seven portable sources.
- Current refs: generated for the five promoted tests with local
  `dev/cppgm++`, not an Opus reference binary.
- PA23 original disposition: the five promoted portable PA23 originals were
  removed as source-identical or comment/format-only duplicates of the promoted
  PA21/PA22 reducers. The two non-promoted portable rows remain in PA23.

| PA23 source row | Candidate owner | Promoted test | Historical evidence | Current disposition |
| --- | --- | --- | --- | --- |
| `pa23/tests/general/200-dependent-builtin-decay-transform-return.t` | none for backfill | not promoted | Opus start `1963d796e`, Opus final `1434a6e11`, and current all accept and emit the same semantic `decay_copy`/local-constructor shape modulo modern object metadata. | Tracker row marked `no-missing-opus-feature` / `no-action`; intrinsic-specific coverage remains later-owned. |
| `pa23/tests/general/200-dependent-remove-reference-transform-forwarding.t` | PA22 | `pa22/tests/general/100-dependent-remove-reference-transform-forwarding.t` | Opus start `1963d796e` exits 0 but under-emits the forwarding/value_func/stored constructor path; Opus final `1434a6e11` and current emit the constructor-forwarding LowIR. | Promoted; placement audit suggested PA22:100. |
| `pa23/tests/general/400-dependent-alias-nontype-sequence-filter.t` | none for now | not promoted | Opus start `1963d796e`, `45820ae4b`, `743b7a920`, and final `1434a6e11` all reject the portable sequence/filter shape with unresolved `from_sequence` / `from_sequence_impl`; current accepts. | Tracker row marked `historical-validation-missing` / `no-action`; needs a reachable Opus pass transition or smaller portable reducer before promotion. |
| `pa23/tests/general/400-dependent-remove-cv-transform-alias-substitution.t` | PA21 | `pa21/tests/general/200-dependent-remove-cv-transform-alias-substitution.t` | Opus start `1963d796e` rejects with `expected an integral constant expression`; Opus `45820ae4b`, final, and current accept. | Promoted; placement audit suggested PA21:200. |
| `pa23/tests/general/500-internal-remove-cvref-alias-sfinae.t` | PA22 | `pa22/tests/general/300-internal-remove-cvref-alias-sfinae.t` | Opus start `1963d796e` exits 0 but selects/emits the wrong assignment SFINAE shape; Opus final `1434a6e11` and current emit the local functor assignment probe. | Promoted; placement audit suggested PA22:300. |
| `pa23/tests/general/500-type-pack-element-preserves-concrete-argument.t` | PA22 | `pa22/tests/general/100-type-pack-element-preserves-concrete-argument.t` | Opus start `1963d796e` rejects `get<0>`; Opus `45820ae4b`, final, and current accept. | Promoted; placement audit suggested PA22:100. |
| `pa23/tests/general/500-type-pack-element-result-selects-copy-ctor.t` | PA22 | `pa22/tests/general/100-type-pack-element-result-selects-copy-ctor.t` | Opus start `1963d796e` rejects `get<0>`; Opus `45820ae4b` still rejects the constructor result shape; Opus `743b7a920`, final, and current accept. | Promoted; placement audit suggested PA22:100. |
