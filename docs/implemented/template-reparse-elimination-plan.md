# Template Reparse Elimination Plan

## Purpose

This plan defines the next structural cleanup lane for the semantic/template
layer: remove routine text serialization and reparsing from hot semantic paths,
especially around dependent types, template arguments, partial
specializations, and alias rewriting.

The immediate trigger is the current GCC/libstdc++ bring-up work. That effort
exposed a real pathological frontier where structured placeholder-bearing
types were repeatedly converted back to text and reparsed through template
matching and dependent lookup. A minimal dependency fix now cuts through the
worst loop, but the underlying architecture still relies too heavily on
string-based round-trips.

The latest `chrono` debugging sharpened that further. The current failures did
not come from one giant parser hotspot. They came from a small number of
missing semantic boundaries:

- dependent `common_type<...>::type` queries sometimes entered specialization
  selection too early
- partial-order placeholder types escaped into generic lookup and
  canonicalization helpers
- those helpers were deciding what was "dependent" or "safe to stringify"
  mostly by inspecting text or `named_key` prefixes instead of explicit
  instantiation state

This plan is not just a performance cleanup. It is also a correctness and
maintainability plan:

- correctness:
  structured placeholder state, alias state, and partial-specialization state
  should not be recoverable only by reparsing text
- performance:
  template-heavy hosted headers should not pay repeated parse costs for
  semantic forms we already have structurally
- maintainability:
  fixes should land once in a structured representation rather than in
  multiple text rewrite and parse fallback seams

## Why This Needs A Dedicated Plan

The repo already has two adjacent histories:

- [docs/implemented/TEMPLATE_PARSING_CONSOLIDATION_PLAN.md](/Users/vishvananda/cppgm/docs/implemented/TEMPLATE_PARSING_CONSOLIDATION_PLAN.md)
  consolidated parser and template-angle entrypoints
- [docs/implemented/fragment-parsing-fallback-plan.md](/Users/vishvananda/cppgm/docs/implemented/fragment-parsing-fallback-plan.md)
  demoted fragment parsing from a routine semantic service toward a fallback

Those were necessary but not sufficient.

The remaining issue is a higher-layer semantic design problem: even after
parser consolidation, several hot paths still:

1. take a structured `TypePtr` or structured `TemplateArgument`
2. convert it to text
3. rewrite that text
4. parse the text back into a semantic form

That means parser consolidation alone cannot eliminate the current blowups.
The structured semantic/template layer itself needs a more explicit internal
representation.

## Current Reparse Seams

The current hot or correctness-sensitive text round-trips include:

- [dev/src/callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
  - `lookup_text_for_type_argument(...)`
  - `canonicalize_member_typedef_type(...)`
- [dev/src/template_specialization.cpp](/Users/vishvananda/cppgm/dev/src/template_specialization.cpp)
  - `deduce_from_named_template_id_text(...)`
  - direct fallback to `parse_type_text_for_deduction(...)`
- [dev/src/template_resolution.cpp](/Users/vishvananda/cppgm/dev/src/template_resolution.cpp)
  - `canonicalize_dependent_alias_type_for_deduction(...)`
  - `template_argument_text_for_matching(...)`
  - `decompose_template_instantiation(...)`
- [dev/src/template_argument_semantics.cpp](/Users/vishvananda/cppgm/dev/src/template_argument_semantics.cpp)
  - `rewrite_bound_type_names_preserving_dependent_text(...)`
  - `resolve_type_argument_text(...)`
  - `parse_type_text_for_deduction(...)`
- [dev/src/template_instantiation.cpp](/Users/vishvananda/cppgm/dev/src/template_instantiation.cpp)
  - template argument text emission used as an intermediate semantic form
- [dev/src/semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
  - template argument text recovery in overload/template candidate paths

The recent GCC chrono debugging also showed a concrete failure mode:

- internal partial-order placeholder types were treated as ordinary named
  types
- those structured placeholder forms were converted to text
- the resulting text was reparsed through qualified-name/template-id paths
- repeated reparsing produced pathological fan-out before reaching the actual
  semantic frontier

That specific blowup was partially reduced by treating the placeholder forms as
dependent, but the broader architectural issue remains.

## Recent GCC Chrono Findings

The `std::chrono` / `std::common_type` debugging produced a few concrete
lessons that should change the order of this plan.

### 1. There were two distinct problems

- early entry:
  dependent template-ids like `common_type<duration<_Rep1,_Period1>, ...>`
  were sometimes entering specialization selection before the arguments were
  truly concrete
- internal escape:
  once partial-order comparison created internal placeholders, generic helpers
  such as bound-type matching and "simple dependent text canonicalization"
  treated them as ordinary named types and tried to stringify them

### 2. The old text guards were compensating for missing state

The current code has multiple guards that effectively ask:

- does this `named_key` start with `partial-order `?
- does this text still contain placeholder names?
- should this dependent argument text be canonicalized by reparsing?

Those checks were useful for reduction, but they are not the right long-term
representation. They are standing in for semantic facts that should already be
explicit on the instantiated class or template application.

### 3. The first bridge step should be explicit instantiation state

Before the full structured-node migration, the code should move from
text-prefix guards to explicit booleans or small flags on instantiated template
state. In practice that means `ClassInfo` and equivalent instantiation carriers
should record whether an instantiation:

- is still dependent
- contains internal partial-order placeholders
- is safe for ordinary text canonicalization / text-based lookup helpers

That bridge step does not replace the larger structured-template plan. It
gives the current codebase a correct control surface so generic helpers stop
guessing from text.

## Recent Combined GCC/Clang Findings

The later combined GCC/clang validation added three more concrete lessons.

### 1. Parsed template-id decomposition must beat half-bound `ClassInfo`

`pa33/720` (`std::vector<P>` with an omitted default allocator) regressed when
`decompose_template_instantiation(...)` was widened to prefer `info->source_template`
even when the structured instantiation arguments were not fully bound.

That was the wrong recovery order:

- the parsed template-id already had enough source information to complete the
  default allocator structurally
- the half-bound `ClassInfo` path did not
- preferring the half-bound structured record caused later deduction to fail

Bridge rule:

- if a parsed template-id is available, prefer it unless the `ClassInfo`
  instantiation arguments already fully bind the source template parameters
- do not broaden member-scope or `ClassInfo` recovery just because a parsed
  template-id is "not yet complete"

### 2. Deferred default arguments must carry the **bound** dependent form

The same `pa33/720` regression also showed that deferred default type
arguments cannot safely store the original unbound spelling when caller
bindings are already known.

For shapes like:

- `std::vector<P>`
- default `Alloc = std::allocator<P>`

carrying the original template-default spelling instead of the caller-bound
dependent spelling breaks later matching and deduction.

Bridge rule:

- if a default type argument must remain deferred, rewrite only the already
  bound parameter references once and store that bound dependent text
- do not depend on later deduction or matching passes to "repair" the missing
  substitution

### 3. Eager dependent class-template instantiation is not an acceptable fix

The recent `chrono` performance cliff came back when dependent type lookup was
briefly changed to eagerly call `reference_class_template_instantiation(...)`
for dependent class-template-ids.

That made some deduction paths appear to work, but it also reintroduced the
old hosted-header blowup:

- `pa33/tests/compile/607-chrono-duration-convert-owner.t`
  went from roughly the old GNU baseline to a 4-5x slowdown

Bridge rule:

- keep dependent class-template-ids as dependent semantic forms until a real
  semantic boundary requires instantiation
- do not instantiate them early just to recover template structure that should
  already be available through structured decomposition

## Main Direction

Do not replace the current plain-text intermediate with a richer plain-text
intermediate.

Instead:

1. keep source text parsing only at the true parser boundary
2. use structured semantic/template objects internally
3. derive stable structural keys from those objects for caches and maps
4. keep text only for:
   - diagnostics
   - ownership output
   - parser fallbacks that are still intentionally supported during migration

The central rule for this plan is:

> Once a type, template-id, alias application, or template argument has a
> structured semantic form, hot semantic code should carry that structured form
> forward rather than serializing it and reparsing it.

The immediate bridge rule that follows from the current `chrono` work is:

> Internal placeholder-bearing template instantiations must be marked
> explicitly on their semantic records. Generic lookup, matching, and
> canonicalization helpers should branch on that explicit state rather than on
> `named_key` text or ad hoc string-prefix checks.

Two more bridge rules now follow from the combined GCC/clang fixes:

> If both a parsed template-id and a `ClassInfo` instantiation record are
> available, the parser-derived decomposition should remain authoritative until
> the `ClassInfo` arguments are fully bound.

> If deferred dependent text is still needed temporarily, it must be the
> caller-bound dependent spelling, produced once at the deferral boundary, not
> the original unbound template-default spelling.

## Target Internal Representation

The migration should introduce a small internal set of structured shapes rather
than one giant replacement subsystem.

### 1. `TemplateHeadRef`

This identifies the head of a template application without text parsing.

It should be able to represent:

- `ClassTemplateDecl *`
- `AliasTemplateDecl *`
- template-template parameters
- dependent member templates
- explicit partial-order placeholder heads if needed

### 2. `StructuredTemplateArg`

A structured template argument node with explicit kind:

- type
- non-type constant
- dependent non-type expression
- class template argument
- alias template argument
- pack expansion

This should preserve dependency, pack-ness, and concrete semantic identity
directly instead of relying on `arg.text`.

### 3. `StructuredTypeExpr`

A structured dependent-type tree for the cases we currently flatten to text.

It should be able to represent at least:

- concrete `TypePtr`
- dependent named type
- dependent member type
- dependent template-id
- alias application
- cv/ref/pointer/array/function wrappers
- `decltype(...)`-like deferred forms if needed
- partial-order placeholder nodes

This does **not** need to replace every existing `TypePtr` use immediately.
It only needs to cover the cases where the current system escapes back to text.

### 4. `TemplatePattern`

A once-parsed representation for partial specialization and alias patterns.

It should hold:

- a structured template head
- structured pattern arguments
- direct template-parameter references
- pack markers
- enough source-scope information for the rare remaining fallback lookup case

### 5. Bridge Instantiation Flags

Before the fully structured migration is complete, add a small explicit flag
set on instantiated semantic records, starting with `ClassInfo`.

This bridge state should cover at least:

- `dependent_instantiation`
  This already exists, but it should be made authoritative from structured
  arguments rather than inferred inconsistently from text.
- `contains_internal_placeholder_arguments`
  True when the instantiation arguments still contain partial-order placeholder
  nodes or equivalent internal-only template placeholders.
- `allow_text_canonicalization`
  A derived or explicit flag used to short-circuit generic stringify/reparse
  helpers when the instantiation still carries internal placeholder state.

This is the shortest path from the current code to fewer text guards. It lets
the migration replace several text-pattern checks with a boolean on the
template/class instantiation while the fuller structured-node work lands.

## Stable Structural Keys

The current code often uses text as both:

- a human-readable form
- a cache or instantiation key

That coupling is part of the problem.

This plan should split those roles:

- human-readable text remains for diagnostics and owned outputs
- cache/identity keys become structural keys derived from:
  - head identity
  - argument kind
  - argument structural identity
  - explicit dependency/placeholder state
  - pack expansion shape

The key may still serialize internally for hashing or map lookup, but it
should be serialized **from structure** and should never need to be reparsed as
semantic input.

## Execution Slices

## Slice 0. Audit And Instrumentation

Before broad migration, keep the current seam inventory explicit.

### Scope

- inventory all hot text round-trips in:
  - `callsemantic.cpp`
  - `template_specialization.cpp`
  - `template_resolution.cpp`
  - `template_argument_semantics.cpp`
  - `template_instantiation.cpp`
  - `semantic_overload.cpp`
- add counters or trace tags to measure:
  - `lookup_text_for_type_argument(...)`
  - `resolve_type_argument_text(...)`
  - `parse_type_text_for_deduction(...)`
  - `parse_template_id_string(...)`
- distinguish source-token progress from synthetic fragment/template reparses

### Goal

Make each migration slice measurable. The success condition is not just
"cleaner code"; it is fewer semantic text round-trips on the current hosted
frontiers.

## Slice 1. Replace Text Guards With Explicit Instantiation State

This should be the first implementation slice now.

### Scope

- add explicit instantiation flags on `ClassInfo` and any nearby template
  instantiation carriers that need the same state
- make `dependent_instantiation` authoritative from structured template
  arguments
- add an explicit internal-placeholder boolean such as
  `contains_internal_placeholder_arguments`
- convert the current highest-value text guards to check that explicit state
  first, especially in:
  - `type_depends_on_template_parameter(...)`
  - `canonicalize_simple_dependent_argument_texts(...)`
  - bound-type text matching / lookup helpers
  - early class-template-id deferral checks

### Goal

- stop using `named_key` string prefixes as the primary source of truth for
  placeholder/dependency state
- give the current architecture a stable semantic control surface before the
  larger structured-node migration

### Expected Payoff

- fewer ad hoc string guards in generic helpers
- fewer accidental partial-order placeholder escapes
- clearer separation between "dependent", "internal placeholder", and
  "ordinary concrete instantiation"

## Slice 2. Remove Structured-Actual-Type Reparsing In Partial Matching

This is the first implementation slice and the most direct continuation of the
current GCC chrono debugging.

### Scope

In [dev/src/template_specialization.cpp](/Users/vishvananda/cppgm/dev/src/template_specialization.cpp),
stop reparsing the full `actual_type` text in
`deduce_from_named_template_id_text(...)` when we already have:

- `actual_class->source_template`
- `actual_class->instantiation_arguments`

Use the structured instantiation information directly.

### Goal

- eliminate one of the highest-value text round-trips
- keep actual instantiated types structured during partial-specialization
  matching

### Expected Payoff

- fewer template-id reparses in hosted template matching
- reduced risk that placeholder-bearing actual types leak into ordinary text
  parsing

## Slice 3. Parse Patterns Once

### Scope

At declaration collection time, pre-parse partial-specialization and alias
patterns into `TemplatePattern`.

### Goal

Stop reparsing `partial.arg_texts` and similar pattern text on every match
attempt.

### Rule

The pattern parser should run when the pattern is registered, not every time a
candidate is matched.

## Slice 4. Structured Alias Canonicalization

### Scope

Replace text-based alias rewriting in:

- `canonicalize_member_typedef_type(...)`
- `canonicalize_dependent_alias_type_for_deduction(...)`
- `rewrite_bound_type_names_preserving_dependent_text(...)`

with structured transforms over `StructuredTypeExpr` and
`StructuredTemplateArg`.

### Goal

Alias canonicalization should no longer mean:

1. stringify
2. rewrite string
3. parse rewritten string

Instead it should mean:

1. walk structured node
2. substitute known aliases/bindings
3. produce another structured node

## Slice 5. First-Class Placeholder And Deferred Nodes

### Scope

Represent partial-order placeholder state and similar deferred semantic forms
explicitly instead of encoding them in named-type keys.

### Goal

- dependency checks should use structure, not string-prefix tests
- equality and matching should understand placeholder/deferred state directly
- hot code should stop leaking those nodes through ordinary named-type text

## Slice 6. Structured Template Decomposition And Matching

### Scope

Move `decompose_template_instantiation(...)`,
`template_argument_text_for_matching(...)`, and related matching helpers to
structured decomposition and comparison.

### Goal

When a concrete instantiated class already has:

- `source_template`
- `instantiation_arguments`

matching should use that directly and only fall back to text parsing for legacy
or unresolved cases.

## Slice 7. Demote Text APIs To Boundary-Only Helpers

### Scope

After the earlier slices land, narrow the role of:

- `lookup_text_for_type_argument(...)`
- `resolve_type_argument_text(...)`
- `parse_type_text_for_deduction(...)`
- `parse_template_id_string(...)`

### Goal

These APIs should become:

- parser boundary helpers
- diagnostics/ownership helpers
- explicit compatibility fallbacks

They should no longer be ordinary hot semantic transport.

## Migration Rules

To keep this manageable, each slice should follow these rules:

1. Do not replace everything at once.
2. Introduce structure at one seam, then remove the local string round-trip.
3. Keep fallback text parsing temporarily when necessary, but make it explicit.
4. Add counters or trace notes so the old path can be measured shrinking.
5. Prefer preserving current outward behavior over broad ref churn.
6. Prefer choosing the correct existing structured source over broadening a
   recovery path. The `pa33/720` regression came from using the wrong
   structured carrier, not from a missing fallback.
7. If a temporary text bridge remains necessary, perform the minimum
   substitution once at the deferral boundary and carry that result forward.
   Do not add later repair/reparse stages.

## Non-Goals

This plan does **not** mean:

- replace the public parser with a second parser
- redesign every `TypePtr` representation immediately
- rewrite all semantic caches at once
- block current GCC/libstdc++ frontier debugging until the whole plan lands

The GCC compatibility work should continue in parallel. This plan is the
structural follow-up that prevents more time from being spent chasing new
string-round-trip blowups one by one.

## Success Criteria

This plan is successful when all of the following are true:

1. the hosted template-heavy frontiers no longer spend routine work in repeated
   semantic stringify/reparse loops
2. partial-specialization matching and dependent alias canonicalization carry
   structured forms through their hot paths
3. internal placeholder/deferred states are represented structurally, not by
   special text keys
4. text-based semantic reparsing is measurable as a fallback path rather than a
   routine service
5. future semantic fixes can land in one structured representation instead of
   several text rewrite and parse helpers

## Immediate Next Step

The first concrete implementation step from this plan should now be Slice 1:

- add explicit instantiation flags on `ClassInfo` so the current semantic
  helpers stop using text-prefix checks as the main dependency/placeholder
  signal

The immediate follow-on after that should be Slice 2:

- remove structured-actual-type reparsing from
  `deduce_from_named_template_id_text(...)`

That order matches what the current GCC chrono reductions exposed: the bridge
flags are the shortest path to deleting the current text guards, and the
structured-actual matching cleanup is still the next smallest high-value
structural slice after that.
