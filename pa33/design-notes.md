# Approaching machine optimization

Start with the MIR and backend from PA24. The [assignment](README.md) defines
the behavior, quality envelopes and kernel limits. These notes explain design
questions and failure cases, without prescribing an allocator, instruction
sequence or tuning policy.

## Describe what instructions do

Record each instruction's uses, definitions and clobbers, including call
arguments, fixed registers, tied operands and both parts of indexed addresses.
Selection may fold operations or introduce temporaries, so LowIR liveness alone
need not describe the selected instructions. Rewrites must keep those facts
consistent with emission.

Begin with local improvements: identity copies, immediate operands, address
arithmetic and jumps to the next block. Stop propagation at conflicting writes,
unknown clobbers and exceptional boundaries. Keep meaningful debug locations
on surviving instructions.

Preserve loop recognition through exact integer conversions. Check layout
changes with native timing: instruction counts can stay unchanged while speed
changes.

## Allocate across the control-flow graph

Compute liveness across ordinary and exception edges. Model a phi transfer on
its edge, especially when its predecessor has more than one successor. Parallel
copies must preserve cycles. A reload and a phi assigning one register on the
same edge must agree on which value is live afterwards.

Values live through a call need a surviving register or a correctly placed
save/reload. Split a live range where that reduces repeated memory traffic;
a cold call need not force a loop's values into memory on every iteration.
Handler-visible values must also be available at every relevant throw point.

Sharing a dying operand's register can remove a copy, but operand order and
early clobbers matter. A two-address subtraction cannot overwrite its right
operand before reading it. A conditional move leaves its destination unchanged
on one path; model both paths. Fixed-register division and shifts can overlap
other operands, implicit uses and ABI arguments.

Weight uses by expected execution frequency, and measure spills and reloads
inside loops rather than counting only frame slots. Saving a callee-saved
register costs work on each function entry and exit. Rematerializing a cheap
constant or address may be better than carrying it across a long live range.

## Width and representation

Know which readers need a narrow value's upper bits. Removing an extension
is safe for a low-bit store and may be unsafe for an address index. Follow
requirements through phis, selects and copies, not just the next instruction.
Do not assume incoming upper bits are initialized when the ABI leaves them
unspecified. A proof such as `convert trunc exact` may help; an arbitrary cast
supplies no such proof.

Test signed extrema and wrapping unsigned arithmetic. Select a division at the
intended width and preserve its trapping preconditions. A high-half product
must retain the correct signedness and any other users of the full product.
A constant-divide sequence must preserve a dividend that it reads after a
fixed-register multiplication.

Treat floating/integer carrier copies as bit transfers. A full XMM frame
access covers sixteen bytes, even if a later operation reads only the upper
half. Slot liveness and incoming argument spills must include all those bytes.

## Floating selection and scheduling

A scalar memory operand can remove a separate load, but the instruction's
operand order, alignment and aliasing requirements still apply. A compare may
have swapped operands; preserve its predicate, including unordered results.
Signed zero and NaNs distinguish apparently interchangeable minimum/maximum
sequences.

Flags are a value too. A zeroing instruction between a compare and a branch
or select can destroy the condition. Prove the flags survive every intervening
instruction and predecessor before reusing them.

Independent roots or divisions can overlap in execution. Starting more of
them can also increase XMM pressure and create spills. Partial-register
semantics may introduce dependencies on old destination bits. Measure a
representative chain and inspect the selected operands; a smaller instruction
count alone does not establish a latency improvement.

## Frame layout, blocks and encoding

Adjacent ordinary field stores can offer another size improvement: combine
their written bytes when fewer memory operations suffice. Preserve holes,
nonzero fields and the separate effects of volatile or atomic accesses.
Check both code size and runtime work; a shorter encoding alone is not a
performance result. The adjacent-zero-fields fixture bounds memory work
without prescribing a particular clearing instruction.

Lay out the final frame after allocation. Preserve object alignment, reuse
storage only when lifetimes do not overlap, and derive unwind information from
the actual saved registers and stack changes. Test leaf functions, calls,
alternate exits and exceptional returns. Omitting a frame pointer changes
addressing and unwind requirements; it is not just deleting two instructions.

A shared return sequence is useful only when a path reaches it. After giving
exits their own teardown, check whether a shared copy remains reachable through
a branch or fallthrough. Retain unwind rows for every emitted exit and calls
in cleanup code, including calls with stack arguments.

When eliminating stack stores, include each throwing call's handler reads.
A later overwrite on the normal path cannot replace a value the handler
needs before that overwrite. Check nested handlers and partial-width accesses.

Place frequently followed edges together and keep cold paths out of hot loops
where profitable. Alignment can improve fetch behavior or add work at every
entry to a short loop. Branch shortening and padding interact, so validate
final displacements and relocations after layout. No particular padding size,
block order or prologue sequence is a quality requirement.

## Validation and further experiments

Use a behavior test for each counterexample, then an envelope broad enough to
accept different registers, offsets and instruction orders. The reference MIR
is one example. Prefer total work, memory traffic and frame bounds to a
requirement that one named instruction appear.

The [optional improvement exercises](../pa32/improvements.md) cover vector
loops, packing, ordered reductions and independent math chains. PA33 can reuse
their scalar oracles while changing only native optimization. Accepting
serialized vector LowIR and deciding to vectorize scalar input are separate
responsibilities.

For timing, use immutable executables, runtime inputs, equal output checks,
A/A calibration and paired ABBA observations. Compile outside the measured
interval and retain every observation. Dynamic counts explain work for a
deterministic workload; timing investigates latency and layout. Measure on
both available CPU lanes before generalizing a small hardware-dependent gain.
