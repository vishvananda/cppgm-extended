# Alias-Use Canonical Emission Plan

## Goal

Make `alias_use` source-row emission as explicit as the new class-use path:
callers should describe why an alias-template use is source-facing, while
`witness_api` owns capture gating, request-to-decision construction, and
semantic dedupe.

This is a maintainability plan. It is not intended to make witness generation
faster or to change expected output as a side effect.

## Current Shape

Alias-use emission is already funneled through
`witness::AliasUseEmitRequest`, but the request still exposes the policy bit
`allow_source_capture_pause`. That recreates the hard-to-reason-about pattern
removed from class-use emission: semantic producers must know whether their
alias row should bypass the normal source-capture pause.

Current producers include:

- `template_argument_semantics.cpp`
  - direct alias-template source use
  - nested alias-template IDs discovered inside explicit arguments
- `template_specialization.cpp`
  - alias-template IDs in specialization patterns
  - alias-template IDs recovered from pattern text
- `callsemantic.cpp`
  - alias-template IDs in base clauses
  - alias-template IDs encountered while resolving type/qualified-name paths

Each site builds the same conceptual row:

- source use location and use anchor
- template-id occurrence payload
- alias-template witness entity
- selected declaration anchor
- expanded type text when available
- primary bindings

The renderer still performs alias-specific cleanup after semantic emission:

- `WitnessBuilder` dedupes alias events by a rendered event key.
- `drop_class_template_local_alias_source_events` removes local helper rows.
- `drop_member_alias_parameter_events` removes member-alias parameter noise.
- `prefer_source_spelled_alias_events` chooses source-spelled variants.
- `collapse_duplicate_current_specialization_alias_pack_events` and
  `prefer_current_specialization_alias_duplicates` collapse current
  specialization variants.
- `dedupe_visible_events` performs several final visible-output dedupe passes.

Those renderer passes are an important safety net today, but they also hide
whether semantic emission is canonical.

## Target Rule

An alias-use frame should be emitted when semantic analysis has an alias
template-id that is independently source-facing:

- a direct alias-template ID spelled by the user;
- an alias-template ID in a specialization pattern;
- an alias-template ID in a base clause or qualified source expression;
- a nested alias-template ID inside a source-spelled template argument list,
  when that nested ID has its own source spelling.

An alias-use frame should not be emitted when:

- source witness capture is disabled;
- the request has no exact source location for the alias name;
- the alias use is only an implementation artifact from speculative type
  lookup, overload probing, or SFINAE checking;
- a semantically equivalent alias-use row with a better source anchor or more
  concrete template-id occurrence is already present;
- the only difference from an existing row is binding whitespace or a less
  specific current-specialization spelling.

Function-call capture pauses should not leak into alias-use call sites. If a
specific alias origin must record while function-call source capture is paused,
that should be expressed as alias-use provenance, not as a generic
`allow_source_capture_pause` escape hatch.

## Implementation Steps

1. Add `AliasUseEmissionOrigin`.

   Start with origins that map to current producer families instead of one
   boolean:

   - `ResolvedAliasTemplateId`
   - `PatternTemplateId`
   - `BaseClauseTemplateId`
   - `QualifiedSourceTemplateId`
   - `NestedSourceTemplateId`

   The exact names can change during implementation, but the enum should answer
   "why is this row canonical?" rather than "which suppression path may this
   bypass?"

2. Replace `allow_source_capture_pause`.

   Remove `AliasUseEmitRequest::allow_source_capture_pause` and the
   `record_alias_use_source_use_allow_source_capture_pause` helper. Add an
   origin-aware predicate in `witness_api`, analogous to
   `class_use_recording_enabled(origin)`.

   Ordinary resolved alias uses should record only when normal source capture is
   open. Source-owned pattern/base/nested origins may record during
   function-call speculation if they represent exact user spelling.

3. Centralize request-to-decision conversion.

   Keep callers responsible for semantic inputs, but make `emit_alias_use`
   the only place that:

   - validates the request location;
   - applies the alias-use recording gate;
   - sets use and selected-declaration anchors;
   - records into `SemanticSourceUseTable`;
   - notes the legacy/debug source decision.

4. Move semantic alias dedupe into `semantic_source_use`.

   Add an alias-use equivalence helper beside the class and variable helpers.
   It should initially cover only semantic identity that is safe to collapse
   before rendering:

   - same kind, location, selected declaration, template name, and expansion;
   - equivalent bindings ignoring irrelevant whitespace;
   - preference for source-spelled template-id occurrence data over synthesized
     or incomplete data;
   - preference for current-specialization payload when it represents the same
     source decision.

   Leave source-text formatting, local helper suppression, and final visible
   signature dedupe in the renderer until tests prove the semantic rows are
   complete enough to remove those passes.

5. Migrate producers by family.

   Suggested slices:

   - direct alias-template uses in `template_argument_semantics.cpp`;
   - nested alias-template argument uses in `template_argument_semantics.cpp`;
   - specialization-pattern uses in `template_specialization.cpp`;
   - base-clause and qualified-source uses in `callsemantic.cpp`.

   After each slice, the strict witness suite should remain clean.

6. Remove renderer cleanup only after proving it is redundant.

   Add temporary debug counters or targeted assertions if needed to compare raw
   `SemanticSourceUseTable` alias rows before and after renderer cleanup. Only
   remove a renderer pass when it consistently drops no alias rows for the
   strict set.

## Acceptance Criteria

- No alias-use API exposes `allow_source_capture_pause`.
- Alias-use capture decisions are readable from `AliasUseEmissionOrigin`.
- Alias-use semantic dedupe lives in `semantic_source_use` for identity cases
  that do not require rendered text.
- The renderer remains allowed to normalize formatting, but it is not the first
  owner of alias-use policy.
- This command passes after each behavior slice:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Completion

Implemented direction:

- `AliasUseEmissionOrigin` replaces the caller-controlled pause bypass.
- `emit_alias_use` now applies the origin-aware recording gate and writes
  through one source-use table helper.
- alias-use semantic dedupe now handles equivalent rows that differ only in
  harmless binding spacing and preserves the more concrete template-id
  occurrence.

Validation:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Result: all requested strict tests passed.
