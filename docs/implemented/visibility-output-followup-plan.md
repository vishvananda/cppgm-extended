# Visibility / Output Follow-Up Plan

This is the remaining follow-up to the archived broad plan:
[implemented/VISIBILITY_OUTPUT_EXECUTION_PLAN.md](/Users/vishvananda/cppgm/docs/implemented/VISIBILITY_OUTPUT_EXECUTION_PLAN.md).

The broad semantic/output cleanup is already in place:

- centralized requirement marking through `require_function_definition(...)`
- explicit requirement kinds (`ORK_*`)
- required-definition closure expansion
- required-definition closure validation

The remaining work is narrower. It is no longer "finish visibility/output" in
general. It is specifically:

- remove the backend-local closure/export ownership heuristics that still live
  in LowIR generation
- keep backend reachability and consistency validation, but stop letting the
  backend act like a second semantic planner

## Narrow Goal

Make semantic required-definition closure the only source of truth for:

- which functions/globals must exist by the LowIR/object boundary
- which bindings are output-required versus merely reachable
- which entities must survive as exports

After this follow-up:

- LowIR generation may compute reachability for emission
- LowIR generation may validate closure consistency
- LowIR generation may not invent new semantic ownership rules

## Remaining Backend-Side Second Sources Of Truth

The main remaining backend-local logic is in
[lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp):

- `collect_reachable_function_symbols()`
- `prune_dead_unowned_exported_symbols()`
- `should_require_internal_closure(...)`
- `validate_symbol_closure()`

These functions are not equally problematic. The right split is:

### Keep As Backend Responsibilities

- ordinary code/data reachability for emission
- final consistency validation that every referenced symbol resolves
- final validation that exported symbols correspond to emitted entities

### Remove Or Narrow As Semantic Planning

- backend-local rules for deciding which internal closure is "really" required
- export pruning based on backend-local liveness rather than semantic output
- backend-local decisions about whether a missing symbol is owned by semantic
  output versus backend emission

## Concrete Invariant To Enforce

The intended invariant should be explicit:

- if a function, method, global, helper, or export is required at the object
  boundary, semantic output must already have marked it and closed over it
- if the backend discovers a missing required symbol, that is a consistency
  failure, not a prompt for backend-local ownership inference

The backend should ask:

- "is this semantic closure complete and internally consistent?"

not:

- "what else should I add so this object looks linkable?"

## Remaining Work

### 1. Classify Each Backend Closure Path

For each of the remaining `lowirgensemantic.cpp` paths, decide whether it is:

- pure reachability
- pure validation
- or accidental semantic planning

That classification should be written down before changing behavior so we do
not delete useful backend checks along with the problematic ownership logic.

Current classification:

| Path | Current role | Keep / move | Why |
| --- | --- | --- | --- |
| `collect_reachable_function_symbols()` | backend reachability | keep | This is ordinary graph reachability over already-known references so emission can keep the transitive function body set. It does not invent new semantic ownership on its own. |
| `validate_symbol_closure()` | backend validation | keep, but narrow its helper policy | This is the right late consistency gate for missing function/global/export owners. The problematic part is not the validation itself, but that it currently defers to `should_require_internal_closure(...)` to decide when a missing symbol is semantically owned versus treated as a backend/runtime exception. |
| `prune_dead_unowned_exported_symbols()` | accidental semantic planning | move / remove | This is deciding export survival from backend-local liveness plus ownership guesses rather than from semantic-output requirement metadata. That is the clearest remaining backend-side second source of truth. |
| `should_require_internal_closure(...)` | mixed helper: runtime/import policy plus semantic-ownership heuristic | split / narrow | The current helper conflates two different questions: "is this one of the explicit backend/runtime exception families?" and "should semantic closure have produced an owner for this symbol?" The former belongs in backend policy; the latter should not be decided here. |

Secondary uses that matter even though they are not top-level entries in the
original list:

- `binding_for_declared_symbol(...)`
  - currently uses `should_require_internal_closure(...)` to decide whether a
    declared symbol should default to internal versus strong/import-like
    binding
  - this is backend policy only if the helper is narrowed to explicit
    runtime/import exception families
- `lookup_runtime_reference_function_symbol(...)`
  - currently uses the same helper to decide whether a symbol may be treated as
    an acceptable runtime/object-boundary reference
  - again, this is only legitimate if the helper stops acting as a semantic
    ownership oracle

### 2. Move Semantic Planning Back Upstream

Any logic that decides:

- what internal methods/functions must exist
- which template-upgraded bindings must be preserved
- which exported entities are semantically required

should move back to semantic output / closure expansion rather than remaining in
LowIR generation.

### 3. Leave Reachability In Place

Do not remove legitimate backend work:

- symbol reachability needed to emit only used code/data
- relocation-target discovery
- final object/export validation

The goal is not to make the backend blind. The goal is to make it non-authoritative
about semantic ownership.

### 4. Make Export Derivation Closure-Driven

Export survival should be driven by semantic-output requirement metadata.

The deciding question should become:

- "did semantic output mark this as required/exported?"

not:

- "did backend collection happen to keep it alive?"

### 5. Keep Only Validator-Style Backend Checks

Useful final backend checks:

- every referenced function/global resolves
- every export refers to an emitted entity
- every relocation target resolves
- every exported symbol has a semantic owner
- no closure-required symbol is lost during backend pruning

## Suggested Order

1. Revalidate the current backend-local closure/export paths against current
   bootstrap and hosted link behavior.
2. Write down the classification of each suspicious function in
   `lowirgensemantic.cpp` as reachability, validation, or semantic planning.
3. Move only the semantic-planning cases back into semantic output / closure
   expansion.
4. Keep backend reachability and validator logic intact.
5. Add small regressions for each moved ownership family before removing the
   old backend-local heuristic.

## Completion Criteria

This follow-up is complete only when:

- semantic-output closure is the only planner for required definitions/exports
- `lowirgensemantic.cpp` no longer decides additional internal closure on its
  own
- missing-symbol failures show up as closure/consistency failures, not late
  backend ownership guesses

Current status:

- completed

Implemented closure slice:

- `prune_dead_unowned_exported_symbols()` was removed
- the old mixed `should_require_internal_closure(...)` helper was narrowed to
  explicit backend passthrough symbol families only
- missing export/closure gaps now surface as validator failures instead of
  being silently pruned away

The validating regression that proved the new behavior was real was the TLS
wrapper family in `pa32`:

- `@...__tls_wrapper` initially surfaced as a closure-owner failure once
  pruning was removed
- that family was then added to the explicit backend passthrough policy

Validation on the backend-heavy lane is green after the change:

- `pa32` TLS import/export owner checks: pass
- `make test-report-nobuild ACTIVE_TEST_REPORT_PAS='pa30 pa32 pa33 pa34 pa35' ...`
  -> `206 / 206`

## Regression Surface

The main regression surface should be:

- existing hosted link smokes that historically exposed missing emitted symbols
- bootstrap/self-host symbol-closure validation
- targeted small regressions for:
  - late-required class methods
  - template-upgraded definitions
  - vtable/export ownership
  - stdlib helper visibility
