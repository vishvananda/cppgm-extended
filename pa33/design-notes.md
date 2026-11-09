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

Base frame requirements on the selected operations too. A wide integer
conversion need not adjust the stack, while a runtime conversion may require
it. Reserving a register unnecessarily can turn loop state into repeated spills.

Begin with local improvements: identity copies, immediate operands, address
arithmetic and jumps to the next block. Stop propagation at conflicting writes,
unknown clobbers and exceptional boundaries. Keep meaningful debug locations
on surviving instructions.

Preserve loop recognition through exact integer conversions. Check layout
changes with native timing: instruction counts can stay unchanged while speed
changes.

A memory operand can save an instruction while making a branch slower.
Compare load dependencies and compare/branch fusion on the target processors;
preserve the access width and flags when separating a load from its predicate.

Indirect dispatch can repeatedly enter blocks without a direct back edge.
Consider their instruction alignment too, while bounding padding and accounting
for it when choosing short branches. Several labels can denote one code
position; do not charge or insert alignment repeatedly at that position.

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

Count pressure at the actual reads and writes. An operand that dies before
a result reuses its register need not overlap that result. Folded or deferred
uses must remain live until the selected instruction reads them.

Weight uses by expected execution frequency, and measure spills and reloads
inside loops rather than counting only frame slots. Saving a callee-saved
register costs work on each function entry and exit. Rematerializing a cheap
constant or address may be better than carrying it across a long live range.
Branch predictions can guide these costs and block layout. Retain enough
precision to distinguish common and rare paths; a prediction is never proof
that the other path is unreachable.

## Width and representation

Know which readers need a narrow value's upper bits. Removing an extension
is safe for a low-bit store and may be unsafe for an address index. Follow
requirements through phis, selects and copies, not just the next instruction.
Do not assume incoming upper bits are initialized when the ABI leaves them
unspecified. A guard can narrow the possible full-width values enough to
make a later normalization redundant. Check the original range through any
truncation, merge every incoming edge and forget facts at unknown clobbers. A proof such as `convert trunc exact` may help; an arbitrary cast
supplies no such proof.

A calculation that reads only the low bits and then writes the full result
may establish its own representation. Check the encoded width and all operand
uses: addresses consume full-width values, and a shift with a possible zero
count may leave the destination unchanged.

For scalar/vector transfers, track which bits each consumer reads. A shuffle
that uses only supplied lanes may not need cleared upper lanes; a wider unpack
or a second consumer can observe them. Test with nonzero prior register bits,
signed zero and NaN payloads, not only freshly loaded scalar values.

Switch tests can reuse ranges excluded by earlier comparisons on that path.
Check gaps, signed boundaries and the selector's full width before omitting a
bound or extension; out-of-range values must still reach a defined default.
An explicitly unreachable successor supplies another proof: defined executions
cannot take that edge. Consider its effect on dispatch size and range checks,
while preserving reachable actions before an impossible continuation. Include
defined defaults, narrow incoming values and empty sets of live arms in tests.
Equivalent indexed addresses can have different encoding costs. Preserve
scale, width and flags when choosing the order of their operands.
Combining a scale with an add may work with either operand order. Preserve
any later reader of the intermediate scaled value, including partial writes.

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
Removing code can shift hot loops into less favorable fetch windows even when
their instruction counts stay the same. Measure runtime as well as size;
use selective alignment rather than assuming more padding always helps.
Data-dependent exits do not make a repeated path unimportant. A loop with
several back edges can contain a compact fast cycle and a longer slow path;
measure their final encoded spans separately. A short retry path should not
discard a useful alignment of the larger cycle. Reuse loop facts across passes,
bound padding and preserve its alignment through object linking. Compare
several harmless changes to preceding code and linked object placement.
Distinguish an unmeasured edge from an unlikely one, especially with indirect
branches; padding a hot fallthrough can add work on every iteration.
Consider how often a loop can repeat: entry padding may cost more than it
saves for a short scan over a small immutable table.
Entry placement can affect direct calls and table-driven indirect calls
differently. Include both when measuring changes to compact leaf functions.

A short shared dispatch tail can sometimes be copied into its incoming
jumps. Preserve its guards, flag dependencies, memory accesses and exception
boundaries. Count table data as well as instructions when bounding growth.
Measure branch prediction across several layouts; unchanged instruction counts
can still hide a substantial runtime difference. No particular duplication
or block order is required.

## Validation and further experiments

A register sum consumed by the following load can share that load's indexed
address. The arithmetic flags must be dead, and the original sum must be
overwritten before any later reader. A bounded local search can prove this
without changing the selector's scratch-register contract. Partial and
conditional writes are not full overwrites; an escaping or live-out address
still needs its sum. Keep the load's displacement, type and source location.

Logical flags can sometimes survive a change in operand width. An immediate
AND mask confined below both sign bits gives the same zero, sign, parity,
carry and overflow flags as a test of the resulting value at either width.
Masks reaching a sign bit require a separate proof; equality alone does not
justify reusing flags for signed conditions.

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
