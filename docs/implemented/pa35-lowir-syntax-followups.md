# PA35 LowIR Syntax Followups

## Purpose

This note records the next optimizer-facing LowIR changes that would be useful
after the current-syntax PA35 work.

The important split is:

- work we can still do on today's LowIR text contract
- work that probably wants new internal IR structure or new public LowIR
  syntax / metadata

## Current Status

The current-syntax tranche on this branch now covers the practical low-risk
optimizations that fit cleanly into the existing non-SSA pass shape:

- executable-edge-aware CFG propagation for values and available expressions
- commutative-expression normalization
- compare-direction normalization
- boolean compare cleanup against `0` / `1`
- identity convert cleanup
- local integer constant reassociation
- executable-edge-aware promotable-slot propagation and dead-store cleanup

There are still theoretical current-syntax optimizations we could add, but the
remaining high-value work is no longer "another clean local tranche." The next
substantial wins want either:

- an internal SSA form inside the optimizer, or
- new public LowIR syntax / metadata

## Current Recommendation

Keep the public LowIR text stable unless we decide the optimizer itself should
expose richer facts directly in the textual contract.

The existing IR already has:

- explicit blocks and labels
- explicit temp values
- explicit slots, loads, and stores
- explicit scalar arithmetic / compare / convert instructions

That is still enough for niche additional intra-procedural cleanups, but the
obvious broad-coverage wins are now blocked more by missing join/effect facts
than by missing peepholes.

## Remaining Current-Syntax Work

The remaining no-syntax items are mostly diminishing-return options such as:

- more ad hoc SCCP-like propagation over the current graph
- broader regional value numbering without SSA
- more special-cased slot/load-store cleanup

None of those are impossible, but they are now much less attractive than
moving to richer value/effect representation.

## The First Really Useful Syntax Or Metadata Additions

## 1. SSA Join Representation

The biggest current limitation is that LowIR has no first-class join value such
as:

- `phi` nodes
- block parameters

That is why the current slot-promotion lane must use an "all predecessors
agree or give up" rule instead of materializing a merged value at a join.

Useful follow-on optimizations unlocked by a join representation include:

- full mem2reg across CFG joins
- stronger global value numbering
- partial redundancy elimination
- cleaner loop-invariant code motion
- more effective sparse conditional constant propagation

Recommendation:

- build an internal SSA form first if we want to keep public LowIR stable
- add public `phi` or block-parameter syntax only if we decide the textual
  LowIR contract itself should become SSA-oriented

## 2. Call Effect Metadata

For stronger optimization across calls, the optimizer needs explicit call-side
effect information that today's LowIR does not expose directly.

The highest-value metadata would be things like:

- `readonly` or read-only memory effects
- `pure` / `readnone`-style no-memory-write behavior
- `nounwind`
- `noreturn`

This would enable more aggressive:

- dead-call elimination where the result is unused
- load forwarding across calls
- common-subexpression reuse across calls
- code motion around calls

## 3. Capture And Alias Metadata

Current LowIR can tell us that slots and explicit loads/stores exist, but it
does not carry much explicit alias or capture information for pointer values.

Useful future metadata would include:

- `nocapture`-style parameter facts
- non-aliasing / unique-object facts where the frontend truly knows them
- more explicit read-vs-write effect summaries for indirect memory operations

This is the main missing ingredient for stronger memory optimization without
guessing.

## 4. Explicit Memory Regions Or Object Identity

If we later want more aggressive memory optimization, it would help to know
whether two memory operations definitely target:

- the same object
- disjoint objects
- an unknown overlapping region

That could take the form of:

- alias scopes
- object IDs
- a more structured address / aggregate memory model

This is lower priority than SSA joins and call-effect metadata, but it becomes
important once we want to optimize memory beyond simple non-escaping slots.

## Recommended Order

The best follow-up order now looks like:

1. Stop adding more one-off current-syntax peepholes unless a very clear missed
   case appears.
2. Build internal SSA in the optimizer as the next serious implementation step.
3. Add explicit call-effect metadata when call-side conservatism becomes the
   main blocker after SSA-based scalar cleanup.
4. Only after that consider public SSA syntax or richer alias-region syntax.
