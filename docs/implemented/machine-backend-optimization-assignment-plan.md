# Machine Backend Optimization Assignment Plan

## Purpose

This document records a concrete course-facing plan for adding a new
machine-backend optimization assignment.

The goal is to answer four questions cleanly:

1. where this work should sit in the PA ordering
2. what the public contract should be
3. what belongs in each optimization level
4. how to test it without destabilizing the earlier backend assignments

The immediate motivation is practical:

- host `clang++` builds a much faster `cppgm++` binary than `cppgm++-self`
- the current gap is large enough that backend quality is now a real teaching
  and project concern
- the current native backend is already more sophisticated than a pure "naive
  O0" lowering, so the assignment split must acknowledge that

## Current State

The current native backend in
[lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
is already past a strict stack-machine baseline.

It already includes:

- scalar temp register allocation
- `f32` / `f64` XMM temp allocation
- call-aware callee-saved versus caller-saved temp policy
- direct compare-to-branch lowering for selected patterns
- constant scaled-index folding in `index`
- a tiny local store-to-load forwarding path
- ABI-chunk-aware frame temp sizing

So the backend is currently in an awkward middle state:

- too advanced to describe honestly as a pure backend `-O0`
- not yet organized as an explicit machine optimization pipeline with `-O1`
  and `-O2`

That means the right question is not "how do we preserve an old unoptimized
PA23 surface by picking tests that happen not to trigger optimizations?"

The right question is:

- how do we formalize the current baseline, then layer additional machine
  optimization levels on top of it?

## Design Constraints

The new assignment needs to satisfy these constraints.

### 1. Self-host should remain the last stage

Self-host is an integration milestone, not the right public home for first
teaching backend optimizations.

### 2. LowIR optimization and machine optimization should stay separate

LowIR optimization already has a clean owner in
[pa35/README.md](/Users/vishvananda/cppgm/pa35/README.md).

The machine/backend work should not blur that boundary.

### 3. Earlier assignments must stay stable

We should not rely on "simple tests that probably do not hit the new passes."
That is brittle and will eventually fail.

The robust design is:

- earlier backend tests run at an explicit baseline level
- later backend-opt tests run at higher levels

### 4. Runtime speed is not a grading oracle

The assignment should not grade on wall-clock compile time.

It should grade on:

- explicit `-O*` driver behavior
- dumped machine IR structure
- preserved runtime behavior

## Placement Options

There are three realistic placement choices.

## Option A. Extend PA23

Shape:

- treat `PA23` as the full native backend assignment
- add `-O0` / `-O1` / `-O2` to `lowir2native`
- reorganize the tests so the current backend contract becomes the `-O0`
  contract

Pros:

- backend work stays with backend ownership
- no renumbering later assignments
- existing `--dump-machine-ir` surface is already there

Cons:

- `PA23` becomes much larger and less teachable
- students would meet baseline lowering and backend optimization in the same
  milestone
- current `PA23` README would need a larger rewrite

Assessment:

- workable
- not the cleanest teaching split

## Option B. Insert a new assignment after PA23

Shape:

- keep `PA23` as baseline `lowir2native`
- add a new immediate follow-on assignment, conceptually `PA24`
- that new assignment extends `lowir2native` with machine `-O0` / `-O1` /
  `-O2` and a stronger MIR-quality contract

Pros:

- best architectural fit
- keeps baseline backend and backend optimization separate
- avoids mixing machine work into `PA35`
- does not force self-host to move if numbering is fixed

Cons:

- introduces numbering churn in the existing middle run if the course numbering
  is already frozen
- requires deciding whether the current `PA24` name is reusable in the repo's
  revised assignment history

Assessment:

- best fallback if the late-stage numbering must remain stable

## Option C. Introduce a new PA36 and move self-host to PA37

Shape:

- `PA35`: LowIR optimization
- new `PA36`: machine/backend optimization
- `PA37`: self-host

Pros:

- the cleanest conceptual progression
- preserves the distinction:
  - IR optimization first
  - machine/backend optimization second
  - self-host integration last
- matches the project's current practical maturity well

Cons:

- requires renumbering self-host in the course story
- requires a new assignment README and wrapper/harness slot

Assessment:

- cleanest overall plan if renumbering is acceptable

## Recommendation

The preferred order is:

1. `PA35`: LowIR optimization
2. new `PA36`: machine/backend optimization
3. `PA37`: self-host

That is the clearest student-facing sequence.

If renumbering self-host is not acceptable, the second-best choice is:

1. keep `PA23` as baseline native backend
2. add a new backend-opt follow-on immediately after it
3. do not put this work into `PA35`

The least attractive choice is:

- folding all of this into existing `PA23` without an explicit `-O0`/`-O1`/`-O2`
  split

That would force the test suite to rely on source selection instead of public
driver contracts.

## Public Contract For The New Assignment

The assignment should extend `lowir2native` with explicit machine optimization
levels:

```sh
lowir2native -O0 -o <outfile> <srcfile>...
lowir2native -O1 -o <outfile> <srcfile>...
lowir2native -O2 -o <outfile> <srcfile>...
lowir2native -O1 --dump-machine-ir <mirfile> -o <outfile> <srcfile>...
lowir2native -O2 --dump-machine-ir <mirfile> -o <outfile> <srcfile>...
```

The key rule is:

- the optimization levels are machine/backend levels
- they do not replace or merge with the `PA35` LowIR optimizer

The primary oracle should remain:

- dumped machine IR

with runtime behavior as a secondary confirmation oracle.

## Baseline Versus Higher Levels

The clean way to handle the current backend is:

- treat the current always-on backend quality work as part of the baseline
  machine contract
- then add explicit new backend work at `-O1` and `-O2`

That means the baseline machine level is not "toy unoptimized code."
It is:

- correct native lowering
- plus the current always-on local quality heuristics already built into the
  backend

This is better than trying to rip those features out just to create an
artificially primitive `-O0`.

## Proposed Optimization-Level Split

## `-O0`: Baseline Machine Lowering

`-O0` should include:

- the current temp register allocation model
- the current XMM temp residency for ordinary `f32` / `f64`
- the current direct compare-to-branch lowering
- the current constant scaled-index folding
- the current tiny local store-to-load forwarding path
- the current ABI-width-aware frame/storage sizing

`-O0` should not introduce new global machine optimization passes.

The point of `-O0` is:

- deterministic baseline backend output
- debuggable MIR
- stable oracle for earlier backend functionality

## `-O1`: Local Machine Improvement

`-O1` should own local and cheap backend improvements.

Recommended features:

- local copy coalescing
- redundant move elimination
- compare/branch peepholes
- local addressing-mode folding
- rematerialization of cheap immediates
- direct call-argument shuffle cleanup
- local dead machine-instruction cleanup
- trivial jump and local branch cleanup

These are good `-O1` features because they are:

- mostly block-local
- easy to explain from MIR diffs
- high ROI without requiring whole-function heuristics

## `-O2`: Whole-Function Machine Improvement

`-O2` should own function-wide machine decisions.

Recommended features:

- stronger register allocation heuristics
- live-range splitting where the implementation model supports it
- spill/reload cleanup after allocation
- stack-slot coloring and dead-slot reuse
- callee-saved minimization
- block layout improvement and fallthrough shaping
- small post-RA peephole cleanup
- simple tail duplication only where it directly removes extra jumps

These are good `-O2` features because they depend on:

- whole-function liveness
- frame shape
- CFG structure
- post-allocation cleanup

## Explicitly Out Of Scope

The assignment should explicitly defer:

- vectorization
- PGO
- LTO / ThinLTO
- global microarchitecture scheduling
- target-specific superoptimizer work
- broad interprocedural optimization

Those are not necessary to teach the main backend-quality concepts.

## Testing Strategy

The test design should mirror the public contract rather than elapsed time.

## Oracle Rules

Each successful test should check:

- compiler exit status
- dumped machine IR
- generated program exit status
- generated program stdout where relevant

The primary structural oracle should remain MIR.

Runtime behavior is necessary, but it should not be the only proof.

## Strict Versus Structural MIR

The assignment should reuse the existing PA23 distinction:

- exact raw MIR when exact shape is the point
- canonical/structural MIR when harmless register and frame choices should be
  normalized away

Recommended normalization at higher levels:

- exact free GPR choice
- exact XMM choice
- exact stack offsets

The structural oracle should preserve:

- opcode family
- width
- direct branch shape versus bool materialization
- register versus frame versus immediate location class
- presence or absence of spills/reloads
- addressing-mode shape
- prologue/epilogue preservation shape

## Recommended Test Layout

If this becomes a new assignment, the clean layout is:

- `tests/o0`
- `tests/o1`
- `tests/o2`

Inside each bucket:

- raw `.ref.mir` for strict cases
- canonical `.ref.cmir` for structural cases
- `.ref.impl.exit_status`
- `.ref.program.exit_status`
- `.ref.program.stdout`

This matches the PA35 "levels first" organization while preserving the PA23
backend-oracle style.

## Example Test Families

## `-O0` Baseline Families

These should lock in the current baseline machine contract.

1. direct integer compare feeding branch
2. direct floating compare feeding branch
3. trivial leaf scalar chain stays mostly register-resident
4. small direct object return uses ABI-width-aware temp storage
5. ordinary `f32` / `f64` arithmetic remains on the floating register path

## `-O1` Families

1. local copy chain collapses without changing runtime behavior
2. compare-zero branch lowers to the shorter direct branch shape
3. address computation folds into base-plus-disp or base-plus-index shape
4. immediate rematerialization avoids a pointless spill/reload
5. redundant move before call-argument placement disappears

## `-O2` Families

1. disjoint spills reuse one stack slot
2. block ordering produces a fallthrough on the common path
3. fewer spill/reload pairs under moderate pressure than `-O1`
4. unused callee-saved preservation disappears in a simple function
5. post-RA cleanup removes redundant move chains created during allocation

## Test Authoring Rules

To keep the assignment teachable:

- use tiny LowIR inputs
- isolate one backend choice per test where possible
- prefer structural MIR at `-O2`
- do not grade on compile-time improvement

The MIR should answer:

- what decision did the backend make?

not:

- did this happen to run faster on one machine?

## Migration Plan

The safest rollout is staged.

## Phase 1. Write The Contract First

Add the README and test layout before changing the backend pipeline shape.

That includes:

- assignment README
- `-O0` / `-O1` / `-O2` command-line contract for `lowir2native`
- harness support for level-specific test buckets
- MIR canonicalization rules for the new buckets

## Phase 2. Lock In The Baseline

Add the `-O0` baseline tests that describe the current backend honestly.

Do not try to invent a fake primitive `-O0`.

The baseline should preserve current always-on quality rules.

## Phase 3. Land `-O1`

Introduce an explicit local machine pass pipeline.

Good first passes:

- local copy coalescing
- move cleanup
- compare/branch peepholes
- addressing-mode folding

## Phase 4. Land `-O2`

Introduce whole-function machine passes.

Good first passes:

- stack-slot coloring
- spill/reload cleanup
- block layout cleanup
- callee-saved minimization

## Phase 5. Move Self-Host After It

Only after the machine optimization assignment exists should the self-host
assignment depend on it.

That gives the self-host milestone a stronger compiler binary without making
self-host itself the place students first debug backend quality.

## Recommended Final Ordering

If renumbering is allowed, the clean sequence is:

- `PA23`: native backend baseline
- `PA35`: LowIR optimization
- new `PA36`: machine/backend optimization
- `PA37`: self-host

If renumbering is not allowed, the next-best sequence is:

- `PA23`: native backend baseline
- new immediate follow-on backend-opt assignment after `PA23`
- `PA35`: LowIR optimization
- `PA36`: self-host

## Final Recommendation

The strongest recommendation is:

- do not put machine/backend optimization into `PA35`
- do not rely on PA23 tests "just not triggering" later backend passes
- use explicit backend `-O0` / `-O1` / `-O2` levels
- place the assignment before self-host

If the course can absorb one numbering change, a new machine/backend-opt
`PA36` followed by self-host `PA37` is the cleanest long-term shape.
