# Function-Call Canonical Emission Plan

## Goal

Make `function_call` source-row emission explicit about which calls are
source-facing and which calls are speculative implementation details. The
function-call path should keep overload-specific facts near overload analysis,
but `witness_api` should own recording gates and source-use table writes.

This plan is about reducing hard-to-get-right policy surface, not performance.

## Current Shape

Function-call source rows are less fragmented than class-use rows, but the
remaining complexity is high value because it crosses overload resolution,
constant evaluation, and special builtin-like calls.

Current producers include:

- `semantic_overload.cpp::note_function_call_source_event`
  - ordinary overload-selected function-template calls
  - candidate counts and candidate-drop reasons
  - owner-class class-use side effects
- `semantic_template_function.cpp::emit_function_template_call_source_use`
  - final conversion from function-template call request to
    `FunctionCallSourceDecision`
- `callsemantic/constant_value_lookup.cpp`
  - constexpr direct calls that need source witness rows
- `callsemantic.cpp::try_analyze_declval_call_expression`
  - `declval<T>()` source rows, currently emitted through
    `emit_function_call(ctx, decision, true)`

The API still exposes a dual recording path:

- `emit_function_call(decision)` records through the current global source-use
  table when source capture is enabled.
- `emit_function_call(ctx, decision, allow_source_capture_pause)` can bypass a
  source-capture pause by using only `witness::enabled(ctx)`.
- `record_function_call_source_use_allow_source_capture_pause` writes the row
  directly through the session/context table.

That boolean is the function-call equivalent of the old class-use escape hatch.
It is currently narrow, but it still makes the allowed recording mode a caller
decision rather than a property of the source-use fact.

The renderer also performs function-call cleanup after semantic emission:

- `WitnessBuilder` stores function calls as a vector, so it does not key-dedupe
  them up front like class, alias, and variable events.
- `drop_function_call_events_with_deduced_trailing_bindings` removes less
  specific function-call rows with trailing deduced bindings.
- final visible-event dedupe removes rows with identical rendered signatures.
- `drop_class_uses_redundant_with_function_deduction` uses function-call rows
  to suppress redundant class-use rows.

## Target Rule

A function-call frame should be emitted when overload/template analysis has a
selected source-facing function-template call, or a special source-facing call
whose witness contract intentionally models it as a function call.

An ordinary overload-selected call should be emitted only when:

- function-call source capture is open;
- there is an exact or normalized source location for the callee spelling;
- a selected function-template binding exists;
- the call is not a type-lookup, SFINAE probe, or other speculative analysis
  artifact;
- candidate-drop facts describe the same overload decision that selected the
  emitted binding.

A special call should be emitted only when its origin makes that behavior
explicit. Today the important special cases are constexpr direct calls and
`declval<T>()`.

A function-call frame should not be emitted when:

- source witness capture is disabled;
- function-call source capture is paused for speculative overload/SFINAE work
  and the call has no explicit source-owned special origin;
- the selected target is not template-related and has no owner-template fact to
  report;
- the row is a less specific duplicate of another function-call row for the
  same source spelling and target.

## Implementation Steps

1. Introduce an origin or request type.

   Add either `FunctionCallEmitRequest` or a small
   `FunctionCallEmissionOrigin` attached to `FunctionCallSourceDecision`.
   The goal is to replace `allow_source_capture_pause` with named provenance.

   Suggested initial origins:

   - `OverloadSelectedCall`
   - `ConstexprDirectCall`
   - `DeclvalCall`

   If implementation shows more categories are needed, add them only when they
   carry different recording rules.

2. Replace the context/bool overload.

   Remove `emit_function_call(ctx, decision, bool)` and
   `record_function_call_source_use_allow_source_capture_pause`. Provide one
   `emit_function_call(ctx, request-or-decision)` path that:

   - rejects empty locations;
   - applies `function_call_recording_enabled(origin)`;
   - writes the semantic source row through a single table helper;
   - notes the legacy/debug decision only for accepted rows.

   Keep the global overload only if it delegates to the same centralized helper
   with a default origin.

3. Keep overload policy in overload code, but make final emission mechanical.

   `semantic_overload.cpp` should continue to own:

   - chosen candidate;
   - candidate counts;
   - candidate-drop construction;
   - owner-class side effects.

   It should not decide capture-bypass behavior. It should produce a structured
   function-call request with origin `OverloadSelectedCall`.

4. Canonicalize duplicate function rows before rendering where safe.

   Add a semantic-source-use helper for function-call rows that can collapse
   strictly equivalent calls. Be conservative at first:

   - same source location, selected declaration, selected entity, selection
     kind, bindings, specialization bindings, drops, and candidate counts;
   - preference for a row with more complete selected-declaration anchor data;
   - no drop of rows with distinct candidate/drop payloads.

   Keep `drop_function_call_events_with_deduced_trailing_bindings` in the
   renderer until semantic emission can prove which row is canonical without
   relying on rendered signatures.

5. Make special cases explicit.

   Convert `declval<T>()` to `FunctionCallEmissionOrigin::DeclvalCall`. Decide
   in `witness_api` whether that origin records during function-call capture
   pause. This preserves the current behavior while making the exception
   auditable.

   Convert constexpr direct call emission to
   `FunctionCallEmissionOrigin::ConstexprDirectCall`. If it does not need any
   bypass behavior, the origin still documents why the call is emitted outside
   ordinary overload reporting.

6. Re-evaluate renderer coupling.

   After semantic duplicate collapse is in place, instrument or inspect how
   many function-call rows are dropped by renderer dedupe. Only move the
   trailing-deduced-binding rule out of the renderer if the semantic producer
   can identify the canonical selected call without rendered text.

## Acceptance Criteria

- No function-call API takes an `allow_source_capture_pause` boolean.
- Every function-call row records through one `witness_api` table-write path.
- Special cases such as `declval<T>()` are named by origin, not by bypass flag.
- Overload candidate/drop construction remains in overload analysis, where the
  data is semantically available.
- This command passes after each behavior slice:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Completion

Implemented direction:

- `FunctionCallEmissionOrigin` names ordinary overload-selected calls,
  constexpr direct calls, and `declval<T>()`.
- the context overload no longer accepts a pause-bypass boolean;
  `witness_api` applies the origin-aware recording gate.
- all function-call rows write through one source-use table helper.
- conservative function-call semantic dedupe now handles equivalent rows that
  only differ in harmless binding spacing.

Validation:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Result: all requested strict tests passed.
