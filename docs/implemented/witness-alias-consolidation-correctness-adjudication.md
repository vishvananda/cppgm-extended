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

## Cross-host CI follow-up, 2026-08-15

The first Linux GCC lane exposed two order-sensitive failures after the direct
alias-resolution fast path landed.  Neither difference changed the reference
oracle.

`pa35/tests/compile/600-shared-ptr-allocator-shadowing.t` failed because the
shared direct alias resolver let an invalid nested `decltype` probe escape as
`std::logic_error`.  This operation is a non-authoritative local type probe in
ordinary semantic analysis, so the failure means "no type" and participates
in SFINAE.  The resolver now converts that exception to a failed local lookup.
Witness capture remains authoritative and rethrows it; swallowing the same
exception there creates an incorrect extra PA24 value-instantiation event.

`pa24/tests/general/500-reentrant-static-query-callable-enable-if-cache.t`
missed the concrete fork-property value instantiation only on GCC.  Partial
specialization matching correctly isolated dependencies from rejected
candidates, but the selected-candidate replay inspected only the expanded
alias target.  A cached `enable_if_t<true>` result no longer contains the
source expression that proved viability.  The replay now scans the selected
partial specialization's original structured argument syntax before alias
target expansion.  It therefore recovers
`is_applicable_property<executor, fork_t<0>>::value` transactionally without
publishing facts from losing candidates or adding a witness cache mirror.

The placement audit also found that
`pa19/tests/general/300-dependent-anonymous-member-field-lookup.t` used a
PA20-owned `static_assert` only to force `Box<int>` and its anonymous member to
instantiate.  A namespace-scope enum initializer now takes `sizeof(Box<int>)`
instead.  This preserves the PA19 instantiation purpose and the existing LowIR
byte-for-byte while removing the later feature.  Its source-location-only
witness change was regenerated through the patched-Clang materializer.

The LLVM 21 libc++ lane exposed a separate qualified-lookup gap through its
new compressed-pair layout spelling.  The alignment expression
`::std::__compressed_pair_alignment<allocator_type>` names a variable template
declared in libc++'s inline ABI namespace.  Direct value, class-template, and
alias-template lookup already included inline namespace children, but direct
variable-template lookup inspected only the written namespace's local map.
`lookup_direct_variable_template` now applies the same inline-child lookup
rule, so all of its constant-expression and ordinary-expression consumers see
the injected declaration.

The header-free PA22 reducer is
`pa22/tests/general/200-variable-template-alignas-member-alias.t`.  It combines
an inline-namespace variable template with an outer member alias used by an
anonymous nested record's `alignas`.  Host Clang accepts it while the pre-fix
CPPGM compiler reported `unsupported alignas`; the fixed compiler emits and
runs the expected eight-byte layout.  Its ordinary LowIR reference was
generated after the host-oracle check, and its witness reference came from the
patched-Clang materializer.

Validation used only the standard `~/ralph-ci` `u24-gcc` lane, with no other
lane active.  The exact PA35 compile case and exact PA24 witness comparison
pass.  With `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1`, the configured strict suites
pass PA19 `279/279`, PA20 `158/158`, PA22 `293/293`, PA23 `385/385`, and PA24
`415/415`; the full PA1-PA38 report passes `4925/4925`.  A PA39 self-host ladder
through PA10 also passes from the isolated object root
`../obj/pa39/u24-gcc-alias-j4`; both the ladder and self-host object build were
capped at four jobs.

On the sole `~/ralph-ci` `u26-clang-libcxx` lane, the reducer passes the full
PA22 ordinary suite (`309/309`), and the original failing PA35 libc++ `<string>`
case compiles.  The direct-text strict gate passes all `1,531` configured
witness comparisons, and the full direct-text PA1-PA38 report passes
`4,926/4,926`.

The PA39 ladder then exposed a distinct two-phase lookup defect while the
self-host compiler compiled libc++ 21's hash-table implementation.  An
inherited member function template was declared in a base specialization that
defined `node_value_type` as its node wrapper.  The derived/use scope defined
the same alias as the container pair.  CPPGM correctly chose the base as the
deduction parent, but its use-site binding overlay installed the derived alias
directly in that scope.  The derived spelling therefore shadowed the ordinary
type found by unqualified lookup at the function template declaration.  Two
complementary `enable_if` overloads both appeared viable, and CPPGM selected
the node-only body for a pair argument.

Deduction overlays now exclude ordinary type names visible through the
function template's declaring scope, as well as template parameter and
template-bound names.  The overlay still supplies concrete enclosing-template
arguments, but it can no longer replace a lexical type declaration.  The
header-free regression is
`pa23/tests/general/300-inherited-member-template-sfinae-lexical-type-shadow.t`.
It binds a use-site template parameter with the colliding name, verifies both
complementary candidate selections, and uses only C++11 traits owned by PA23.
Host Clang accepts the fixture warning-clean; the pre-fix CPPGM compiler fails
inside the node-only body; and the fixed compiler passes its ordinary check.
Its ordinary reference was generated from the corrected compiler behavior,
while its witness reference was generated independently by the patched-Clang
materializer.  The PA23 placement audit classifies it at `pa23:300` with no
later-feature dependency.

Validation again used only the standard `~/ralph-ci` lane, with
`u26-clang-libcxx` as the sole active flavor and all builds capped at four jobs.
The focused ordinary and witness comparisons pass, PA23 passes `401/401`, and
the full direct-text strict gate passes PA19 `279/279`, PA20 `158/158`, PA22
`294/294`, PA23 `386/386`, and PA24 `415/415` (`1,532` comparisons total).  The
full direct-text PA1-PA38 report passes `4,927/4,927`, and the debugger gate
passes for PA13, PA37, and PA38.  The isolated PA39 self-host ladder at
`../obj/pa39/u26-clang-libcxx-alias-final-j4` passes through PA10.  In
particular, the original `macroizer.cpp` failure point, the modified
`template_resolution.cpp`, and the final `cppgm++` link all build successfully
against libc++ 21 before PA10 passes `157/157`.

## Final cross-lane validation, 2026-08-15

We validated commit `834ccd8f6` through `~/ralph-ci` with one active flavor at
a time.  We ran each test command to completion before starting the next and
limited every build and subtest pool to four jobs.

The `u24-clang-libcxx` flavor passed the full direct-text PA1-PA38 report at
`4,927/4,927`, the strict gate at `1,532/1,532`, and the PA13/PA37/PA38 debugger
gate.  Its isolated PA39 object root,
`../obj/pa39/u24-clang-libcxx-alias-final-j4`, passed the ladder through PA10,
including PA10 at `157/157`.

The `u26-gcc` flavor passed the same `4,927/4,927` report, `1,532/1,532` strict
gate, and PA13/PA37/PA38 debugger gate.  Its isolated PA39 object root,
`../obj/pa39/u26-gcc-alias-final-j4`, also passed through PA10 at `157/157`.
That run self-compiled both `macroizer.cpp` and the modified
`template_resolution.cpp` before linking the final driver.

[GitHub Actions run 31883595723](https://github.com/vishvananda/cppgm-extended/actions/runs/31883595723)
tested the final commit in all four supported Linux flavors: Ubuntu 24.04 and
26.04, each with GCC/libstdc++ and Clang/libc++.  GitHub Actions marked every
build, placement, direct-text report, strict, debugger, and
self-host-through-PA10 job as passed.
