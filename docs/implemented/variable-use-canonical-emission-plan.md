# Variable-Use Canonical Emission Plan

## Goal

Keep `variable_use` emission simple while making its one remaining policy bit
explicit. Unlike class, alias, and function-call rows, variable-template source
use is already close to canonical because it has one main producer.

The goal is not to add a larger abstraction. The goal is to make the existing
replacement rule obvious and keep renderer cleanup from becoming the hidden
owner of variable-use identity.

## Current Shape

Variable-use rows are produced mainly by
`template_instantiation.cpp::instantiate_variable_template`.

The producer constructs `VariableUseEmitRequest` with:

- source use location and optional anchor identifier;
- template name;
- ownership (`Direct` or `NestedDerived`);
- selected specialization kind;
- selected declaration anchor;
- primary bindings;
- specialization bindings;
- `replace_prior_source_use`.

`witness::emit_variable_use` converts the request into
`VariableUseSourceDecision`, then either:

- calls `record_variable_use_source_use`; or
- calls `replace_variable_use_source_use` when `replace_prior_source_use` is
  true.

The replacement helper delegates to
`semantic_source_use::replace_equivalent_variable_source_use`, which replaces an
existing variable-use row when the rows are equivalent ignoring source
location. This is a real semantic policy, but the request bit does not explain
why replacement is valid.

Renderer behavior is simpler than for the other facts:

- `WitnessBuilder` key-dedupes variable events.
- `dedupe_visible_events` can still drop rendered duplicates.
- variable events are intentionally spared by one template-header pattern drop
  path because their source placement is different from class/alias rows.

## Target Rule

A variable-use frame should be emitted when semantic analysis instantiates or
selects a variable template because of a source-facing variable-template use.

It should be emitted with:

- `Direct` ownership for the source use currently being analyzed;
- `NestedDerived` ownership only for nested replay from a source location;
- explicit specialization or partial-specialization selection when that is the
  selected entity;
- bindings derived from source-spelled template arguments when those are
  available, otherwise deduced/defaulted bindings according to the existing
  template-instantiation policy.

A variable-use frame should not be emitted when:

- source witness capture is disabled;
- no source use location is available;
- the request is a nested replay and an equivalent direct row already exists;
- the request only replaces an earlier row without a semantic reason to prefer
  the new location or anchor.

The replacement case should be named as a merge policy. It should not remain a
boolean that leaves callers and future readers guessing why prior source use
may be overwritten.

## Implementation Steps

1. Replace `replace_prior_source_use` with an enum.

   Add a small merge policy:

   ```cpp
   enum class VariableUseMergePolicy
   {
     AppendIfNew,
     ReplaceEquivalentSourceUse
   };
   ```

   `AppendIfNew` should preserve the current default behavior.
   `ReplaceEquivalentSourceUse` should be used only at the existing call site
   that can justify replacing an equivalent row with the current source
   location.

2. Centralize variable-use table writes.

   Keep `emit_variable_use` as the single public request entry point. Have it
   call one internal helper that:

   - validates source capture and use location;
   - builds the `VariableUseSourceDecision`;
   - applies the merge policy;
   - notes the legacy/debug decision for accepted rows.

   This keeps the variable path intentionally smaller than the class and alias
   origin-based paths.

3. Make replacement semantics local to `semantic_source_use`.

   Rename or document `replace_equivalent_variable_source_use` so the
   equivalence key is obvious:

   - variable-use kind;
   - role and ownership;
   - selected declaration/entity;
   - template name;
   - selection kind;
   - expansion text;
   - bindings and specialization bindings ignoring irrelevant spacing;
   - location intentionally excluded.

   If direct-versus-nested precedence is needed later, add it to
   `record_source_use` rather than to renderer-only cleanup.

4. Avoid adding an origin enum unless a second real origin appears.

   Variable-use emission currently does not have a capture-bypass problem. Do
   not copy the class/alias/function origin pattern just for symmetry. Add
   provenance only if another producer appears and needs a different recording
   rule.

5. Measure renderer dependence.

   After the merge-policy rename, inspect whether renderer variable-event
   dedupe drops anything in the strict set. If it does not, leave the renderer
   pass as a harmless global safety net. If it does, decide whether the drop is
   semantic identity that belongs in `semantic_source_use` or rendered-text
   normalization that should stay in the renderer.

## Acceptance Criteria

- No variable-use request exposes a vague `replace_prior_source_use` boolean.
- The replacement rule is named as a merge policy and documented at the
  semantic-source-use helper.
- Variable-use emission remains single-producer and small; no new origin system
  is introduced without a real second policy source.
- This command passes after the refactor:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Completion

Implemented direction:

- `VariableUseMergePolicy` replaces the `replace_prior_source_use` boolean.
- variable-use append and equivalent replacement now share one source-use table
  helper.
- the unused public replacement entry point was removed.
- `replace_equivalent_variable_source_use` now documents the location-free
  replacement key.

Validation:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Result: all requested strict tests passed.
