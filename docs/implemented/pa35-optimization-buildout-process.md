# PA35 Optimization Buildout Process

## Purpose

This document defines the intended buildout approach for `PA35`.

## Completion Note

This plan is now implemented.

The landed PA35 surface in this repo is:

- a standalone `lowiropt` binary with `-O0`, `-O1`, and `-O2`
- a shared LowIR optimizer reused by `lowiropt`, `cppgm++ --emit-lowir -O*`,
  and `cppgm++ -O*` object generation
- an O1 cleanup pipeline with constant folding, local copy/constant
  propagation, CFG cleanup, and safe dead-code elimination
- an O2 slot-promotion pass for simple non-escaping scalar slots, followed by a
  cleanup rerun
- structural PA35 LowIR tests plus source-driven `cppgm++ --emit-lowir -O*`
  integration owners

The document remains useful as a record of the intended PA35 shape, but the
active tracker entry should now point at the archived copy under
[`docs/implemented/`](/Users/vishvananda/cppgm/docs/implemented).

It is a supplement to:

- [ASSIGNMENT_BUILDOUT_PROCESS.md](/Users/vishvananda/cppgm/legacy/ASSIGNMENT_BUILDOUT_PROCESS.md)
- [pa37-selfhost-buildout-process.md](/Users/vishvananda/cppgm/docs/implemented/pa37-selfhost-buildout-process.md)
- [lowir-second-tranche-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-second-tranche-plan.md)

`PA35` now comes before self-hosting and establishes the first real
optimization layer in the compiler.

## Main Recommendation

The best `PA35` shape is:

1. optimize **LowIR first**, not machine IR first
2. use a **dedicated optimizer stage** with LowIR text input and LowIR text
   output as the primary oracle
3. add `-O` levels to `cppgm++` so the same optimization pipeline is used in the
   real compiler path

So the basic user idea is correct, with one refinement:

- the cleanest primary contract is a standalone LowIR optimizer binary
- `cppgm++ -O*` should then reuse that same optimization pipeline internally

That gives us both:

- a deterministic structural oracle
- a practical end-to-end compiler integration path

## Why LowIR-First Is Better Than Machine-IR-First

LowIR is the better first optimization layer because it is:

- easier to read
- easier to diff in tests
- more stable across targets
- more reusable for future non-C++ frontends
- closer to source semantics than machine IR, while still being backend-oriented

Machine-IR peepholes may still be worthwhile later, but they should be a
secondary optimization layer or a future assignment, not the primary `PA35`
contract.

## Recommended Public Shape

## 1. A Dedicated Optimizer Binary

Introduce a small optimizer-stage binary, for example:

- `lowiropt`

Its job:

- read one or more LowIR source files
- run a deterministic optimization pipeline
- write optimized LowIR text

Possible CLI shape:

```text
$ lowiropt -O0 -o <outfile> <srcfile1> ... <srcfileN>
$ lowiropt -O1 -o <outfile> <srcfile1> ... <srcfileN>
$ lowiropt -O2 -o <outfile> <srcfile1> ... <srcfileN>
```

Optional useful dump forms:

```text
$ lowiropt --dump-pre-opt <prefile> -O2 -o <outfile> <srcfile...>
$ lowiropt --dump-post-opt <postfile> -O2 -o <outfile> <srcfile...>
```

But the simplest public contract can just be:

- input LowIR
- output optimized LowIR
- optimization level flag

## 2. `cppgm++` Integration

Then extend `cppgm++` to accept:

- `-O0`
- `-O1`
- `-O2`

with the intended meaning:

- `-O0`: no optimization
- `-O1`: modest cleanup and canonical improvement
- `-O2`: the full `PA35` pass set

The same pipeline should be reused whether LowIR comes from:

- `lowiropt`
- `cppgm++`
- any later backend/test path that wants optimized LowIR

## Primary Oracle

The primary oracle for `PA35` should be:

- deterministic **before/after LowIR optimization**

That means the main spec tests should compare optimized LowIR text, not just
runtime behavior.

This matches the assignment-buildout rule that the primary oracle should sit at
the first new boundary the assignment owns. `PA35` owns the optimization stage
itself, so the structural optimized IR is the right main contract.

## Secondary Oracles

`PA35` should also have secondary behavioral checks:

- optimized and unoptimized output preserve the same program behavior
- `cppgm++ -O2` still passes selected earlier source-driven tests
- optimized LowIR still feeds the native backend correctly

Behavioral tests matter, but they should not be the only contract. Otherwise
students could "optimize" in undocumented ways without any structural proof that
the optimization stage exists and does the intended work.

## Recommended Pass Families

The first pass set should be modest but high value.

## Pass Family 1: Canonical Simplification

This is the cheap cleanup layer and should probably run at `-O1` and above.

Examples:

- constant folding of arithmetic and comparisons with constant operands
- algebraic identities:
  - `x + 0 -> x`
  - `x - 0 -> x`
  - `x * 1 -> x`
  - `x & -1 -> x`
- redundant copy elimination
- redundant cast elimination where source and destination already match
- branch-on-constant folding

This family is easy to test and makes the IR cleaner for later passes.

## Pass Family 2: Copy And Constant Propagation

This is one of the highest-value first optimizations.

Examples:

- replace uses of copied temporaries with their known source
- propagate known constants through later arithmetic and branches
- simplify compare/branch patterns after propagation

This does not need to be full global SSA-based propagation to be useful.
An intra-procedural deterministic propagation pass is already valuable.

## Pass Family 3: CFG Simplification

This should be part of the core `PA35` pipeline.

Examples:

- remove unreachable blocks
- collapse branches with known outcomes
- merge trivial jump-only blocks
- merge straight-line predecessor/successor blocks when safe
- remove empty blocks introduced by previous passes

This family is powerful because it compounds with constant propagation and makes
later passes simpler.

## Pass Family 4: Dead Code Elimination

This should also be core `PA35`.

Examples:

- remove unused pure temporary-producing instructions
- remove dead blocks after CFG cleanup
- remove unused slots/temporaries/functions when provably local and unreferenced

The key rule is to start with obviously safe DCE:

- only delete instructions known to have no side effects
- do not start with aggressive memory-side-effect reasoning

## Pass Family 5: Simple Non-Escaping Slot Promotion

This is the most ambitious pass that still plausibly belongs in `PA35`, and it
is probably the **most powerful** optimization worth targeting in the first
assignment if the LowIR move leaves slots and loads/stores explicit.

Goal:

- promote simple local scalar slots to temporary value flow when:
  - the address is never taken
  - the slot is not used in aliasing-sensitive ways
  - the access pattern is simple enough to analyze deterministically

Why this matters:

- many frontend-lowered values end up in obvious stack slots
- promotion unlocks propagation, folding, and DCE
- it produces outsized benefit without needing full advanced interprocedural
  optimization

This should likely be the hardest required `PA35` pass, and it belongs at `-O2`,
not `-O1`.

## Recommended Optimization Levels

Recommended public levels:

### `-O0`

- no optimization
- preserve the direct lowered form

### `-O1`

- canonical simplification
- copy propagation
- basic constant propagation/folding
- CFG simplification
- safe DCE

### `-O2`

- everything in `-O1`
- simple non-escaping slot promotion
- rerun the cleanup pipeline after promotion

## Initial Buildout Slice

The first implementation slice should land the optimizer boundary itself before
trying to cover the full recommended pass families.

That initial slice should be:

- a standalone `lowiropt` binary
- `cppgm++ -O0` / `-O1` / `-O2` compile/link plumbing that reuses the same
  optimizer
- structural tests over optimized LowIR text

The first concrete pass set can stay narrow:

- fold `branch` on known constants
- fold `switch` on known integer selectors
- bypass trivial jump-only blocks
- remove unreachable blocks

That is intentionally smaller than the full eventual PA35 ambition, but it
still establishes a real reusable optimization stage instead of a placeholder.

This gives students a useful staged target without requiring a large menu of
optimization levels.

## What To Defer

These are good future optimization topics, but they should not be required for
the first optimization assignment:

- full SSA conversion as the public assignment contract
- global value numbering
- partial redundancy elimination
- aggressive dead-store elimination with alias reasoning
- loop-invariant code motion
- loop unswitching, unrolling, vectorization
- function inlining
- interprocedural optimization
- machine-IR scheduling and register-allocation improvements
- size-specific `-Os` / `-Oz`

Those belong in future work, not the first optimization layer.

## Test Strategy

`PA35` tests should be layered.

## 1. Structural Spec Tests

Primary tests should be hand-written LowIR input/output cases for:

- constant folding
- copy propagation
- branch folding
- unreachable block elimination
- trivial block merge
- dead temporary removal
- slot promotion cases
- "must not optimize" safety boundaries

These should compare optimized LowIR text exactly.

## 2. Behavioral Preservation Tests

Use selected LowIR execution tests to prove:

- `-O0` and `-O2` preserve the same stdout and exit status

These can reuse the existing native backend path.

## 3. Source-Driven Integration Tests

Use selected `cppgm++` tests to prove:

- `cppgm++ -O0` and `cppgm++ -O2` both compile and run correctly
- a chosen subset of earlier source-driven tests still pass through the
  optimized path

This should stay a secondary suite, not the main contract.

## 4. "No-Change" Tests

Include cases where optimization should intentionally do nothing.

These are important so the optimizer does not become a "rewrite everything until
it looks different" stage.

## README Guidance

The README for `PA35` should make two things explicit:

1. optimization is a **semantic-preserving transformation**
2. the assignment is about establishing a clean optimization stage, not about
   winning benchmarks

So the README should avoid requiring:

- wall-clock speedups
- target-specific assembly quality
- unstable host-performance measurements

It should instead require:

- deterministic optimized IR
- defined optimization levels
- behavior preservation

## Buildout Sequence

Recommended `PA35` buildout order:

1. freeze the optimization-stage contract
2. add the standalone optimizer binary and `-O0/-O1/-O2` parsing
3. implement canonical simplification
4. add CFG cleanup and safe DCE
5. add propagation
6. add non-escaping slot promotion
7. wire the same pipeline into `cppgm++`
8. add the source-driven integration suite last

## Relationship To The LowIR Move

`PA35` should wait until the LowIR move is complete enough that:

- declarations/imports are explicit enough to validate cleanly
- storage/value semantics are clear enough for safe transformation
- the IR is stable enough that optimizer tests are not immediately invalidated
  by unrelated IR churn

That does **not** mean every deferred LowIR cleanup in
[lowir-second-tranche-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-second-tranche-plan.md)
must be finished first. It means the optimizer needs a stable enough IR
contract to target confidently.

## Completion Criteria

`PA35` is ready when:

- the assignment has a standalone LowIR optimization stage
- `-O0`, `-O1`, and `-O2` are part of the public compiler-driver contract
- the structural spec suite proves the optimizer performs the intended
  transformations
- optimized and unoptimized output preserve behavior on the selected validation
  set
- the chosen high-value pass families are in place
- the optimizer is strong enough to produce visible simplification wins without
  depending on advanced interprocedural or machine-specific optimization
