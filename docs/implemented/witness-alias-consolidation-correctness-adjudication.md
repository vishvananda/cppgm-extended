# Witness Alias Consolidation Correctness Adjudication

## Scope

The final alias-consolidation validation exposed three differences in the
PA22-PA24 strict corpus.  They did not have one common language cause.  This
record classifies each difference, identifies the authoritative behavior, and
records the correction point.  Witness references remain outputs of the
patched-Clang generator; none of the fixes below rewrites a witness reference
from CPPGM output.

## PA22: patched-Clang witness generation

`pa22/tests/spec/300-member-operator-template-active-owner.t` originally
exposed a missing Clang witness event.  The correction belongs at witness
generation in the patched Clang, not in CPPGM's renderer or in a post-generation
adjudication pass.  Commit `8a18628f7` corrected that generator and regenerated
the authoritative witness reference.

After the PA24 semantic fix below, this fixture's ordinary LowIR deterministically
renumbered three constructor overloads.  Repeated ordinary generation and a
witness-enabled generation produced the same SHA-256,
`24e466ff4cce6ee78ddcec6e5bed770662f45e967ca26afc2a677f13f89f6b9e`.
The Itanium object names, function bodies, aliases, and calls remain paired;
only internal `ov2`/`ov3` identifiers changed with declaration-registration
order.  The LowIR reference was updated to that deterministic spelling.  The
witness reference was not changed by this follow-up.

## PA23: libstdc++ tuple-constraint shortcut provenance

`pa23/tests/general/500-tcc-member-constructible-pack-sfinae.t` deliberately
defines a user class with libstdc++-shaped `_TCC` and
`__is_implicitly_constructible` names.  CPPGM's tuple-constraint shortcut
recognized the names alone, substituted the boolean template argument, and
skipped the user's authoritative `constexpr` function body.  That incorrectly
kept the constrained overload viable.

The shortcut now first resolves its owning class template and requires
`comes_from_standard_include_path`.  User declarations take the general
constant-evaluation and substitution path, where this fixture's false body
removes the constrained overload and selects the variadic fallback returning
4.  GCC 16 independently selects the fallback in a reduced runtime check.
The test and its LowIR reference now assert that result.

This is not a witness adjudication: the old CPPGM semantic result was wrong,
and both ordinary compilation and witness observation must consume the
corrected semantic decision.

## PA24: instantiated member-alias declarations

`pa24/tests/general/400-concrete-recursive-node-layout-retry.t` requires the
member typedef declaration of a concrete class-template specialization to be
instantiated.  N3485 14.7.1 states that class-specialization instantiation
instantiates member declarations; typedef and alias declarations are not in
the set of member definitions that remain deferred.

`concrete_class_alias_requires_immediate_resolution` now distinguishes tracked
language-level instantiations from untracked reference shells.  A real user
specialization resolves its alias declaration immediately; a reference shell
remains observational and may not create a completion demand.  Standard
library implementation aliases remain demand-driven because those hosted
declarations are trusted input and eagerly replaying every unused typedef
materialized 448 otherwise unreachable vendor class specializations in the
frozen self-compile.  User declarations retain the N3485 diagnostic rule.

The earliest-owner negative regression is
`pa19/tests/spec/300-instantiated-member-typedef-declaration-bad.t`.  It
instantiates a class whose otherwise-unused member typedef names a member of
an incomplete specialization.  CPPGM now rejects it, matching GCC 16 and
Clang.  Existing forward-alias controls continue to compile.  The PA24 fixture
now matches the existing patched-Clang witness reference exactly; no PA24
witness reference changed.

## Nested witness observation boundary

Eagerly resolving real alias declarations exposed a separate observer problem
in `pa23/tests/spec/300-nondeduced-partial-pattern-recursive-completion.t`.
Nested reference-member collection attempted authoritative class completion
while traversing witness-only shells, so an observer could diagnose a program
that ordinary semantic analysis accepted.

Reference-member collection now owns one thread-local nesting depth.  At depth
greater than one, witness-enabled traversal treats incomplete class completion
as non-authoritative.  A nested `std::logic_error` resets the partial reference
shell and defers the demand to ordinary semantic analysis.  Shallow collection
still resolves its own members, and non-witness compilation is unchanged.
This confines the policy to the witness observer boundary instead of adding
guards to template lookup, type resolution, alias resolution, and base lookup.

The detector control
`pa23/tests/spec/300-current-instantiation-qualified-detector-call.t`, the
recursive-completion fixture, and the PA24 recursive-layout fixture all match
their patched-Clang witness references after this change.

## Validation

- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict`: passed all configured
  PA19, PA20, PA22, PA23, and PA24 comparisons with zero failures.
- `ACTIVE_TEST_REPORT_PAS='pa19' ORDERED=false make test-report`: 295/295
  passed, including the new negative regression.
- Full direct-text PA1-PA38 `make test-report`: 4,859/4,863 passed.  The four
  PA30 runtime failures occurred while a frozen performance compile was
  competing with the report; an isolated direct-text PA30 rerun passed 88/88.
  The user accepted this combined broad evidence without another full rerun.
- Final three-run frozen semantic-overload comparison against pre-fix commit
  `8a18628f7`: passed.  Median instructions were 174,283,362,240 (-0.22%),
  maximum RSS was 749,625,344 bytes (-1.98%), and peak footprint was
  568,500,224 bytes (-0.14%).  The comparison report is
  `/tmp/cppgm-correctness-final-perf.json`.
