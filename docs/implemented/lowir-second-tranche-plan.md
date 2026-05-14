# LowIR Second Tranche Plan

This document defines the two broader LowIR follow-on slices that remained
after the completed first concrete LowIR tranche and its immediate follow-ups.

Those completed items now include:

- explicit declarations/imports
- explicit `role=` / `linkage=` / `binding=` / `storage=` metadata slices
- explicit structural well-formedness validation
- explicit `pass=` / `arity=` metadata and indirect-call signatures
- ABI-neutral direct object parameters in printed LowIR
- explicit integer width conversions and first-class scalar cast chains
- explicit `copyobj` / `zeroinit` spans and MachineIR preservation of those
  spans
- first-class LowIR `switch` control flow

That remaining work was treated as two larger execution slices rather than a
long tail of tiny one-off LowIR tweaks.

The convergence pass with current `main` was completed before this tranche
started. This document is now archived because both slices have landed.

## Slice 1: Semantic Storage And Address Model Cleanup

This slice owns the still-blurry semantic boundary between values, storage,
addresses, and decayed views.

It should execute as one coherent tranche because the subproblems overlap in
the same lowering paths and test owners.

Slice 1 is now complete. The landed work in this tranche includes:

- explicit `index [projection=...]` metadata for semantic address provenance
- explicit `unary decay ptr` to mark array/function decay as a first-class LowIR step
- explicit local binding-mode cleanup in lowering, so scalar slots, reference
  slots, decayed pointer-view slots, array storage, and indirect object storage
  are no longer re-derived from scattered ad hoc type checks
- PA13 parser / printer / validator support for the new public surface
- refreshed emitted-LowIR owner refs across the affected `pa14`-`pa29` lane
- broad validation through `pa34`

That work makes the first remaining address/decay/storage boundaries explicit
enough that Slice 2 can now focus on layout/runtime metadata rather than still
sharing ownership with basic lowering semantics.

### Scope

- separate value type from storage type more explicitly
- add first-class address projection kinds
- make array/function decay semantics explicit where they still rely on
  frontend-side flags

### Concrete Goals

1. Loads, stores, and indexing should stop implicitly assuming that the
   semantic value type and the backing storage shape are interchangeable.
2. Address-producing operations should distinguish at least:
   - field projection
   - base-subobject projection
   - reference-field dereference projection
   - array-element projection
3. Decay-sensitive lowering should stop depending on ad hoc semantic-node
   bookkeeping for whether a slot holds:
   - the original object/function entity
   - a decayed pointer view
   - an address to an underlying object

### Expected Deliverables

- explicit LowIR surface or metadata for the remaining value-vs-storage cases
- explicit projection/address distinctions where today we rely on flags such as
  `is_base_subobject`
- explicit or more directly represented decay semantics where the optimizer or
  later lowering would otherwise have to reconstruct them
- refreshed PA13 docs and owned refs
- a broad sanity lane through the affected `pa14`-`pa34` emit-LowIR owners

### Non-Goals

- do not reopen host-specific small-aggregate register chunking in printed
  LowIR
- do not introduce backend-policy-only import/export surface unless a real
  generic IR use case appears during the work

## Slice 2: Layout And Runtime Metadata Cleanup

This slice owned the remaining cases where backends still recovered layout or
runtime meaning from semantic side channels or symbol naming patterns.

It should happen after Slice 1, because explicit value/storage/address meaning
will make the needed remaining metadata clearer and narrower.

Slice 2 is now complete. The landed work in this tranche includes:

- shared runtime/EH symbol classification in
  `runtime_symbol_policy.{h,cpp}` for the remaining host EH helper family:
  `__cxa_*`, `__gxx_personality_v0`, and `_Unwind_Resume`
- LowIR function/global role assignment for generated declarations now driven
  by that shared runtime classification instead of duplicated local name logic
- MachineIR host-EH detection now keyed off structured runtime/EH categories
  rather than a hard-coded imported-symbol list
- object-backend undefined-symbol tolerance for backend-introduced EH helper
  references now keyed off structured runtime roles instead of literal symbol
  spellings
- a layout audit that concluded no extra field-offset metadata was needed:
  the existing `index` offset operand plus `projection=...`, `obj<bytesxalign>`,
  `copyobj`, `zeroinit`, and structured global data items already expose the
  remaining layout facts that truly matter at the current LowIR boundary
- broad validation through `pa34`

### Scope

- explicit object-layout metadata where it is genuinely needed at the IR
  boundary
- structured runtime / exception imports where today we still rely on magic
  symbol naming

### Concrete Goals

1. Backends should not have to rediscover important layout facts from semantic
   type information alone when those facts are already fixed at the LowIR
   boundary.
2. Runtime and EH helpers should move further away from “special symbol name
   implies special meaning” and closer to structured IR metadata/import kinds.

### Outcome

- no extra field-offset metadata was added, because the current LowIR surface
  already carries the needed layout facts explicitly enough after Slice 1
- the remaining worthwhile runtime/EH meaning that belonged in LowIR was moved
  onto shared structured categories rather than backend-local symbol-name
  checks
- broad validation stayed green:
  - `pa13`: `72 / 72`
  - `pa14`-`pa29`: `770 / 770`
  - `pa30`-`pa34`: `309 / 309`

### Non-Goals

- do not try to force every object-file export/local-binding policy choice into
  generic LowIR
- do not treat future C-origin `prototype_relaxed` emission as part of this
  C++-front-end tranche

## Execution Order

The intended order is:

1. Slice 1: semantic storage/address model cleanup
2. broad validation and cleanup
3. Slice 2: layout and runtime metadata cleanup
4. final broad validation and plan closeout

## Completion Criteria

This follow-up plan is complete when:

- the remaining value/storage/address ambiguity is materially reduced by
  explicit LowIR structure rather than frontend-only conventions
- the remaining layout/runtime-sensitive contracts that truly belong in LowIR
  are explicit
- the broader `pa13` through `pa34` validation lanes are green again
- the old “later backlog” notes can be narrowed to genuinely optional future
  work rather than active execution items

Those completion criteria are now satisfied.
