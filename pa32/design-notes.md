# Designing a LowIR optimizer

The [assignment](README.md) defines the interface and quality bar. These notes
explain ways to reason about improvements; they prescribe neither a pass order
nor the reference implementation's data structures or tuning policy.

## Start with proofs and small examples

For each transformation, state the behavior it preserves, the facts it needs,
and the conditions that prevent it. Write a positive example and an example
that fails each condition. Inspect the resulting LowIR, validate it, and execute
it through the reference backend at `-O0`. This keeps optimizer correctness
separate from machine optimization. Source fixtures should also survive the
serialized-LowIR object path.

Work on typed instructions and their operands. Reuse the parsed LowIR model;
rendering and reparsing text to recognize a pattern loses facts and makes
changes expensive. Keep use/definition information and control-flow analyses
consistent after edits. A transformation that reports no change should not
force another round of unrelated analyses.

A join needs a phi only when the incoming value can be read before another
definition. Liveness can avoid unnecessary phis and stack traffic, but must
include exceptional paths and preserve values a handler observes.

## Scalar and control-flow simplification

Start with constant folding, identities, copies and unreachable blocks.
Compute integer results at the instruction's width. Do not evaluate a division
that can trap or fold a shift with an invalid count. Floating identities need
extra care: replacing `x + 0` with `x` can change negative zero, and reassociating
a sum changes rounding.

Adjacent integer multiply/add operations can sometimes be combined at their
common width. Preserve wrapping, shared intermediate values and observations
between the operations. Check the native cost too: a smaller expression may
replace cheap shifts or address calculations with a slower multiply.

Executable-edge constant propagation can discover constants through branches
and phis. Dominance permits reusing a pure value where its definition always
runs first. A branch can supply facts to the blocks its taken edge dominates;
those facts do not automatically hold at a merge reached another way. Repair
phi inputs whenever edges are removed, redirected or split.

Promoting scalar storage to SSA removes loads and stores, but requires proving
that all its accesses are understood. Splitting an aggregate may expose more
promotable fields. Preserve padding, bit-fields, volatile members, alignment
and ABI carriers; splitting every object is not always profitable.

Conditionally updated fields can need many merge values inside a loop.
Promoting all of them may turn a few memory updates into repeated register
spills and transfers. Compare both native work and size, including inactive
fields and early exits; a smaller SSA graph alone does not establish a win.

## Memory facts and safe motion

Distinguish the proof that accesses cannot overlap from the proof that a load
may run earlier. A disjoint store does not justify reading an invalid pointer
on a zero-iteration path. A call can modify memory or free an allocation.

Useful sources of disjointness include separate local objects, non-overlapping
ranges of one object and the `typed` rules in
[Instruction Facts](../pa8/lowir.md#instruction-facts). Character accesses,
unions, `may_alias` and disabled strict aliasing restrict that last proof.
Signed and unsigned integers of one width are compatible. The absence of a
`typed` fact grants no type-based disjointness.

A local whose address never escapes may retain memory facts across unrelated
calls. A pointer stored into its own object needs an escape analysis too: a
later read of that pointer can expose the object. Allocation results can be
separate from earlier pointers, but passing or publishing one changes what
other code can reach.

Forwarding through object copies must respect the copied byte range. Clearing
an array can expose known-zero fields; overlapping copies need the semantics
of the original operation. Static initialization is possible only when its
observable values and ordering agree with dynamic initialization.

## Loops, counters and exits

Describe a loop's entries, latches and exits before changing it. A preheader
provides a place for invariant work. Rotation may make an existing test run at
the bottom; preserve the initial guard and values read after the loop.

Hoisting a load requires both disjointness and safe execution at the new
position. Sinking a store requires that every original exit observes the right
value and that the store does not run on a path that previously wrote nothing.
A location updated conditionally may need a phi or a retained memory path.
Calls and exception paths are part of these proofs.

A product of an induction variable and an invariant can sometimes become an
induction of its own. Account for every latch, the initial value, overflow and
all users of the original counter. Widening a narrow counter may avoid repeated
extensions only when its narrow values remain representable. LowIR's
`convert trunc exact` states that proof; an ordinary narrowing cast does not.

A short loop with a known bound can be expanded while retaining early exits.
First, last and missing matches in a search must probe the same elements; an
empty range must probe nothing. Pointer bounds require reasoning about offsets
within the same object. Unknown bounds keep a general path. Bound code growth
as well as runtime work, and do not require every small loop to expand. Estimate
the instructions the transformation actually copies, including a final test
when needed, rather than charging an extra iteration of the whole body.

Unswitching moves an invariant choice outside a loop by making specialized
paths. Interchange changes the traversal of a nest. For either, prove memory
dependences and final values, including exceptional exits, before considering
profitability.

A conditional pointer update can leave a segment boundary unchanged on most
iterations. Consider carrying the boundary with that pointer and comparing a
copying pointer against its end. Prove which stores can change a loaded
boundary, preserve zero-work guards, and test segment crossings, overlapping
ranges, byte aliases and volatile accesses. Measure the resulting program;
fewer scalar instructions can still increase register pressure.

## Calls and exceptions

Inlining can expose constants and memory facts, but code growth can make
callers harder to optimize and increase compiler cost. Consider hot work and
cold paths separately, and bound total growth rather than copying a large body
at every site. Partial specialization may remove repeated work without copying
the whole function. Recursion and address-taken functions need separate care.

Membership in a call cycle does not make every member a recursive worker.
Forwarding and dispatch layers can sometimes be combined around a loop while
retaining the calls that perform recursive work. Establish where expansion
stops, preserve argument evaluation and exceptional exits, and recheck the
call structure after edits. A smaller call graph alone does not justify
unbounded code growth or repeated analysis of unchanged functions.

For a body that only forwards a call and returns its result, composing argument
permutations and constants may avoid copying any instructions. Preserve
argument evaluation and boundary conversions, retain observable object copies,
and detect forwarding cycles. Resolve shared chains once rather than walking
the same chain again from every caller. A retained function's address and
external boundary must keep their meaning.

A short returning path around a larger calling path can be useful to inline
even when the whole body is not. Keep growth in that slower path from
duplicating work at every caller or hiding the useful wrapper. Measure both
retained code size and native execution; fewer copies alone are not a win.
An early return around a loop does not make the loop's work cold. Include
zero-work and nonzero iteration cases when checking those decisions.
Compare a callee's size before expansion with its current size. Earlier copies
can make a small query look expensive to inline at its remaining call sites.
A result known on every normal return can remove a caller's guard even when
the callee has effects. Preserve those effects and exceptional exits; do not
confuse a constant result with a removable call.

A direct-call optimization must respect replacement and `no_inline` metadata.
A guessed virtual target needs a runtime identity check and an indirect
fallback; construction of one object does not prove all later calls use its
original table. Calls on both paths must have the same cleanup behavior.

LowIR's [exception instructions](../pa8/lowir.md#exception-instructions)
include implicit edges. Dominance along ordinary edges alone does not establish
what a handler has executed. Values reused by a handler must be valid at every
throw point that can reach it. Inlining a protected region requires balanced
region state on all normal and exceptional paths. A landing that removes its
own record must retain that record on entry, as the format specifies.

A copied handler must preserve an exception its caller can still read or
resume. A continuation reached from both ordinary and handler paths needs
that check too; a normal incoming path does not settle the other path's state.

## Library math and representation

A library function's name is not enough when the unit defines that function.
For a recognized external library operation, preserve every observable effect
as well as its result. In particular, LowIR's `unary sqrt` has no `errno`
effect; replacing an unrestricted library square root with it is unsafe. A
proof of non-negativity or a guarded fallback can make the transformation valid.
Test negative values, infinities, NaNs and both zeros.

Quotient/remainder sharing must preserve signed rounding and the validity of
the divisor. Known-bit information can simplify masks and conversions, but it
must account for wrapping arithmetic and signedness.

A floating/integer `copy` of matching width moves bits; `convert` computes a
numeric conversion. This distinction matters when promoting ABI carriers,
forwarding stores and preserving signed zero or NaN representations.

Repeated logical shifts and conditional XORs can sometimes be combined
algebraically. For a constant word-sized polynomial `P`, the step
`F(x) = (x >> 1) ^ ((x & 1) ? P : 0)` is linear over bits. After `k` steps,
split the input into its low `k` bits and the remaining bits: the latter
contribute `x >> k`, and the former can be evaluated independently. Arithmetic
or readonly lookup data can express the result. Prove the rule for the actual
width and branch polarity, handle a shift equal to the width, and retain work
whose intermediate values have other observable users. Compare table space,
memory latency and compile cost against the arithmetic removed. Variable
polynomials and arithmetic shifts need their own proofs.

## Register pressure and independent work

Reducing LowIR instruction count can lengthen live ranges. A value moved out
of a loop must occupy a register or a spill slot over that loop. Consuming a
dying operand earlier can reduce pressure, while sinking a computation can
avoid executing it on paths that do not use it.

Independent divisions and square roots may benefit from starting before the
first one's consumers. Reordering must preserve dependencies, traps, memory
effects and call boundaries. Measure native execution as well as static work;
PA33's allocator and instruction selection determine the eventual cost.

## Optional vector and traversal experiments

Vectorization, block packing and interchange are optional extensions. See the
[improvement exercises](improvements.md) for inputs, scalar oracles and commands.
Existing PA29 `v128` operations express packed values without new private
frontend information. An optimizer can use baseline operations or retain
scalar code; each operation's semantics remain those in the format.

For a vector loop, check every tail length, both traversal directions,
alignment, overlapping ranges and mixed element widths. A runtime guard can
select the scalar path when ranges overlap. Preserve the order of floating
reductions unless the input contract permits reassociation. Packing independent
operations is different from changing a reduction's order. Guarded zero-work
paths must not gain speculative loads.

## Measure runtime work and compiler work

Use [the quality checks](README.md#quality-bar) to judge behavior and broad
outcomes. Total size, residual repeated memory work and dynamic instruction
counts can be useful bounds; they do not prescribe a mnemonic or a pass.

For a performance experiment, freeze inputs and binaries, verify identical
results, calibrate A/A noise, then retain every observation from paired ABBA
blocks. Compile outside the timed interval. Count instructions to explain
redundant work and use timing to investigate latency and layout. Thread
scheduling can change the executed work as well as elapsed time.

Measure the optimizer's own compile cost and peak memory alongside generated
code. Cache analyses only while their inputs remain valid, check invalidation
in a verification mode, bound fixed-point work, and avoid scanning an entire
unit from every function pass. Choose data structures that make edits local;
the particular containers and thresholds are your design.
