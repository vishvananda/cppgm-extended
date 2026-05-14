# LowIR Optimization Enablement Plan

## Purpose

This plan is now complete and ready to archive under `docs/implemented/`.

This plan defines the next LowIR-facing evolution lane after the completed
`PA35` optimizer buildout and the completed current-syntax optimizer follow-ups.

It exists to answer a narrower question than the earlier broad LowIR cleanup
plans:

- which new LowIR facts now matter specifically for further optimization
- what order they should land in
- how to add them without repeating the old pattern of one-off syntax churn

This plan should be read together with:

- [docs/implemented/lowir-evolution-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-evolution-plan.md)
- [docs/implemented/lowir-second-tranche-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-second-tranche-plan.md)
- [docs/implemented/pa35-lowir-syntax-followups.md](/Users/vishvananda/cppgm/docs/implemented/pa35-lowir-syntax-followups.md)

## Historical Rules To Keep

The earlier LowIR evolution work established a pattern that we should keep:

1. Add LowIR surface only when the fact is genuinely IR-level and broadly
   useful.
2. Land changes as coherent slices, not as unrelated tiny syntax tweaks.
3. For each slice, update the parser, printer, validator, PA13 spec/docs, and
   owned tests together.
4. Revalidate the broader emitted-LowIR lane after each slice instead of
   treating parser acceptance alone as completion.

The first two LowIR evolution tranches were successful largely because they
followed that shape.

## Current State

The practical current-syntax PA35 follow-up work is now in place:

- executable-edge-aware CFG propagation for values and available expressions
- commutative-expression normalization
- compare-direction normalization
- boolean compare cleanup
- identity convert cleanup
- local integer constant reassociation
- promotable-slot propagation and dead-store cleanup

That means the next broad optimization wins are now blocked less by missing
peepholes and more by missing join/effect/alias facts.

## Main Direction

The next optimization-enabling lane should be:

1. internal SSA groundwork first
2. explicit call-effect metadata
3. explicit capture/alias metadata
4. public SSA join syntax only if the optimizer still needs LowIR itself to
   carry join values
5. richer memory-region/object-identity metadata only after the above

This keeps the public LowIR contract stable until there is a clear reason to
change it, while still moving the IR toward the facts future optimization
passes need.

## Execution Slices

## Slice 0. Internal SSA Groundwork

This is the first serious implementation step, but it is intentionally **not**
the first public LowIR syntax change.

### Scope

- build an internal SSA form inside the optimizer from the existing block/temp
  LowIR
- use that form for:
  - mem2reg across joins
  - SCCP-style propagation
  - stronger global value numbering
  - partial redundancy elimination groundwork

### Why First

The current PA35 optimizer note already identifies missing join values as the
main blocker for the next broad scalar wins. Internal SSA lets us prove the
actual optimizer need before deciding whether public `phi` or block-parameter
syntax is really required.

### Non-Goals

- do not change public LowIR text yet
- do not force the backend or earlier assignments to consume SSA

### Implemented Outcome

The optimizer now has a shared executable-edge-aware internal value-dataflow
state that is reused by both:

- the `-O1` CFG value/expression propagation pass
- the `-O2` promotable-slot analysis

This is not a full public SSA surface, but it is enough internal join-analysis
groundwork to keep public `phi` / block-parameter syntax deferred for now.

## Slice 1. Explicit Call-Effect Metadata

This is the first planned public LowIR syntax tranche in this lane, and the
first slice to start implementing now.

### Scope

Extend function/call-boundary metadata so LowIR can express optimization-relevant
facts such as:

- `effects=readnone|readonly|readwrite`
- `unwind=no|may`
- `return=noreturn|returns`

These should live on the existing function-boundary metadata path so they apply
to:

- function declarations
- function definitions
- explicit indirect-call signatures

### Goals

1. Calls should stop being an undifferentiated conservative barrier in LowIR.
2. The parser/printer/validator should understand and preserve the metadata.
3. The optimizer should later be able to use the metadata for call DCE, load
   forwarding across calls, and call CSE.

### Initial Implementation Rule

Do not guess these facts broadly in the frontend. Only emit them where the
compiler already knows them truthfully. The first implementation slice may land
the metadata surface before widespread source-origin emission exists.

## Slice 2. Explicit Capture And Alias Metadata

### Scope

Add a narrow first public metadata slice for pointer/indirect-memory facts,
preferably on parameter and call boundaries first:

- `capture=nocapture|maycapture`
- limited alias metadata where the frontend truly knows it
- explicit read-vs-write summaries for indirect memory operations if needed

### Goals

- support stronger load/store cleanup beyond non-escaping local slots
- make future memory optimization depend on explicit facts rather than backend
  guesses

### Non-Goals

- do not add a full alias-region system in the first pass

### Implemented Outcome

The completed narrow Slice 2 checkpoint is:

- parameter-level `capture=nocapture|maycapture`
- parameter-level indirect-memory `access=none|read|write|readwrite`
- parameter-level `alias=noalias`
- the same metadata on explicit indirect-call signatures
- truthful emitted metadata only for builtin/runtime boundaries where the
  compiler already knows the fact, such as `memcpy` / `memmove` / `strlen`
  class boundaries

Broader alias-region or noalias work should wait until this narrower capture
surface is explicit, tested, and understood by the emitted-LowIR lane.

## Slice 3. Public SSA Join Representation

This slice should only start after Slice 0 shows whether internal SSA is
enough.

### Candidate Surfaces

- block parameters
- `phi` nodes

### Preferred Direction

Prefer block parameters if public join syntax becomes necessary. They match the
existing block/edge structure more cleanly and avoid introducing a second
special “must appear first in block” instruction family.

### Goals

- make join values explicit in textual LowIR if that becomes important for
  interchange, validation, or student-facing ownership
- unlock cleaner public representations for SSA-based optimization

## Slice 4. Memory Region / Object Identity Metadata

This is later work, not the next tranche.

### Scope

- alias scopes
- object IDs
- or another structured memory-region model

### Why Later

The earlier LowIR tranches already concluded that current layout/address facts
are explicit enough for today’s boundary. The remaining blocker is not field
offset visibility but memory-identity and effect precision.

## Completed Outcome

This execution lane completed in three practical pieces:

1. internal optimizer groundwork:
   - a shared executable-edge-aware internal value-dataflow state now feeds the
     existing CFG propagation and the `-O2` promotable-slot analysis
   - that was enough to keep current join reasoning internal, without changing
     the public LowIR text contract
2. explicit function/call-boundary effect metadata:
   - `effects=readnone|readonly|readwrite`
   - `unwind=no|may`
   - `return=noreturn|returns`
3. explicit pointer-boundary metadata:
   - `capture=nocapture|maycapture`
   - `access=none|read|write|readwrite`
   - `alias=noalias`

The emitted-LowIR lane now carries those facts truthfully only where the
frontend already knows them, and the parser/printer/validator plus owned
PA13/PA15 coverage are in place.

The resulting decision is:

- do not add public SSA join syntax yet
- keep public `phi` / block-parameter work deferred until a later optimizer
  tranche proves that the internal join-analysis seam is no longer enough
- treat the remaining work as optimizer-depth follow-on work, not as another
  missing LowIR metadata blocker

## Validation Rules

For each public LowIR slice:

1. update `pa13/lowir.md` and `pa13/README.md`
2. update parser / printer / validator support in `lowir_internal.*`
3. add owned `PA13` syntax/validation tests
4. refresh affected emitted-LowIR refs only where the frontend truthfully emits
   the new facts
5. rerun the broad affected validation lane before closing the slice

## Completion Criteria

This plan is complete when:

- the first optimization-relevant LowIR metadata slices are explicit and tested
- internal SSA work is far enough along to decide whether public join syntax is
  still needed
- the next remaining optimization blockers are genuinely in implementation
  depth, not in missing IR facts

Those completion criteria are now satisfied.
