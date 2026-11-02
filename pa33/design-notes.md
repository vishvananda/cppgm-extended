# Approaching machine optimization

Start with the MIR and native backend you built in PA24. The
[assignment](README.md) defines the required behavior and code-quality bounds;
these suggestions describe one route to them, and what the course solution
does.

First establish which values and physical registers each instruction reads,
writes or clobbers. Include call arguments, fixed-register operations, both
parts of indexed addresses, and values live across control-flow and exception
edges. Incorrect use information makes even a redundant-copy removal unsafe.

For O1, small local rewrites are useful: remove identity copies, forward a
value into its next use, fold a cheap address or immediate, and eliminate a
jump to the next block. A constant added to an index belongs in the
displacement when only addresses read the sum: `p[i-1]` and `p[i+1]` then
share `i`, and neither needs a register or an addition of its own. Stop
propagation at a conflicting definition or clobber. Preserve volatile effects,
exception boundaries and debug locations when replacing instructions.

Recognize what the lowering leaves behind. PA29's `__builtin_fabs` reaches the
backend as a copy to an integer, a mask and a copy back; the course solution
selects `fabs` instead when nothing else reads the mask, clearing the sign in
the XMM register.

For O2, compute liveness across blocks and calls, and give registers to the
values that live across blocks. The bounds can be met by a small allocator,
but PA34 runs this backend on the whole compiler, where functions are large,
most have cleanups and calls sit inside loops. These choices are the ones
that paid there in the course solution:

- Weigh each use by an estimate of how often its block runs: a loop runs
  several times per entry, and a path that only throws or ends in a call that
  does not return weighs nothing. A frame home's store and reload cost more
  time than their instruction count suggests.
- Keep a value in one register over its whole life where possible. When that
  fails, split it: keep the register outside a loop it crosses without a use,
  or outside just the blocks that clobber or crowd it, rather than giving the
  value a frame home everywhere. Floating values need this more than integers:
  no XMM register survives a call, so a loop with one rare call would otherwise
  keep every floating value in memory.
- Allocate floating values as carefully as integers. Every XMM register except
  your scratch can hold one, though none survives a call. SSE arithmetic
  overwrites its first operand, so a result that takes the register of a first
  operand dying there needs no copy. So does a `select` and an arm dying there,
  and an immediate shift in `rcx`. A subtraction or division cannot do that
  with its second operand, which copying the first would destroy. In the
  floating-point loops profiled for this, reloads and copies were the largest
  difference from GCC's code. A floating
  read of a home an XMM register still holds reads the register, as a read that
  soon after the store would wait for it.
- An unsigned divide by an even constant can take a shorter path. Where the odd
  divisor's multiplier needs 65 bits, shift the divisor's low zeros out of the
  dividend first: the dividend is shorter, and a 64-bit multiplier then serves.
- Divide at the width of the type. A 64-bit `idiv` of two `int`s gives the
  right answer, but on many x86 cores it costs several times what the 32-bit
  one does, and an instruction count will not show it. The 32-bit form traps
  on `INT_MIN / -1`, which a program may not write but a 64-bit divide
  forgives, so keep a divisor you can see is `-1` out of it.
- Check what each shortcut assumes of its operands. Letting a truncation share
  its source's register is sound for a reader of the low bits only, and keeping
  a `nowrap` add computed at 64 bits is sound for canonical operands only. Each
  is correct alone, and together they turn `(int)wide + y` into a wrong index.
- `sqrtsd`, `cvtss2sd` and `cvtsd2ss` keep the destination's upper bits, so they
  wait for whatever last wrote it, even a long division. Compute a square root
  in place after a move, and clear a conversion's destination first, unless the
  operand already lies there.
- Above `-O1` a `copy` of a floating literal is a value of its own, not an alias
  of the literal: the optimizer wrote it to keep the literal in a register
  through a loop.
- An `i32` add or subtract only read in its low bits uses the 32-bit form; one
  read in full stays at 64 bits, and a `nowrap` one is canonical there, so a test
  of either against zero reads the flags it set.
- Extend a narrow value only for a reader of its upper bits. A compare, a store,
  a branch, a 32-bit divide, a wrapping add, a negation, a shift's count, a copy
  to the other sign, a return and a host call's `i32` argument read the low bits alone: the
  psABI leaves the upper bits to the reader, so a caller extends a narrow call
  result itself even when this unit defines the callee. A hardware divide's
  dividend and a shift's count may be computed in `rax` and `rcx`: that early
  clobber is their own load, so a quotient divided again stays in `rax`. A divide
  by a constant writes `rax` with its product and then reads the dividend again,
  so its dividend stays out of `rax`. A phi, a
  select, an `and`, a complement and a `nowrap` add pass their readers' needs to their operands,
  so grow the set to a fixed point. A sum of `int` products that is only stored
  then needs no `movslq` per iteration. The same question decides whether a
  parameter needs one on entry: the psABI leaves the bits above an `int` argument
  unset, and a `nowrap` add of it into an index reads them.
- `x + (a < b)` needs no `cmov`. `lea` and `cmov` make a chain of two instructions
  per pass; `adc rax, 0` or `sbb rax, -1` make a chain of one, when the carry flag
  decides the compare. That holds for the unsigned compares and for `>=` on
  floats: `ucomisd` sets the carry flag when unordered.
- Divide a 32-bit value by a constant with a two-operand `imul` on 64 bits: the
  multiplier fits, the product does too, and `imul r, r` is one micro-operation
  where `mul` needs rdx:rax and three.
- A float load read once by the next `addsd`, `mulsd`, `divsd` or `ucomisd` is that
  instruction's memory operand. The load's register and its `movsd` go. Only the
  operand read second may be memory, so check the compare's operand swap. Unlike
  packed instructions, the scalar forms take unaligned memory.
- A call through a register that a load just filled can read the target from
  memory: `call *(%rax)`. The register must be caller-saved and carry no argument;
  only moves into other registers may separate the two.
- Know what sits between a compare and the `cmov` reading it. `xor r, r` is the
  short zero, and it clears the flags: a scan that stops at the first instruction
  it does not recognize, such as a sign extension, lets a zero through as an
  `xor` and the select quietly stops selecting. Only spilled operands reach this.
- A block that starts with the compare its predecessors each ended with reads
  the flags they left, as in `if (a > b) ... else if (a < b)`. Only compares of
  registers and literals, and only with no edge from elsewhere: a landing pad
  or a jump table target keeps its compare.
- A `cmov` tests one condition. `ucomisd` reports a NaN as unordered, so `lt`, `le`,
  `gt`, `ge` and their `u` forms each map to one condition, but `eq` and `ne` need
  parity as well and stay materialized.
- A branch on `x != x` is a NaN test. `ucomisd` of a register with itself sets
  the parity flag only for a NaN, so the branch is one `jp`, with no `jne`.
  Predict the NaN side rare and place it out of line.
- Let a phi's sources and result share a register when their ranges do not
  overlap, so its edge transfers move nothing. A transfer on a conditional
  edge is executed in that edge's block, not at the end of the predecessor.
  Modeling it there is what lets the web share.
- An integer `select` keeps one arm in place too: `cmov` writes its
  destination only when the condition holds. Read the arms before the result
  is written, so an arm that dies at the select may share the result's
  register. A loop whose select updates a phi then moves nothing on the back
  edge.
- When one member of a phi web crosses a call and has to be split around it, let
  that member choose the register first. It has the fewest choices. Choosing for
  the others first gives a sum in a loop two registers and a move on each side.
- Give a value a callee-saved register only when its weight pays for the
  save and restore on every entry to the function. Rematerialize constants
  and frame addresses instead of keeping them live.
- Treat a landing pad as a successor of every call that can unwind into it,
  so values in functions with cleanups can stay in registers where the
  exception model allows.
- Lay out the frame last: reuse slots whose lifetimes do not overlap, put the
  most accessed objects nearest the frame register, and give each return its
  own epilogue.
- Chain blocks along their likely edges, put blocks that only throw or exit
  last, and rotate a loop whose test is at its top so the test also ends the
  body.
- A jump to a short block that ends `jcc X; jmp Y` may take a copy of that
  block when the jumping block's values decide the test at `X` or `Y`. The
  copy branches straight to the decided side. A latch entered with a constant
  from the test at its loop's head then skips that test on its way around.
  In a recursive function inlined into itself, this took 10% of the cycles
  off.
- Start each loop head on a 16-byte boundary when that takes at most 10 bytes
  of padding, else on an 8-byte one, as GCC does at `-O2`. Padding is `nop`s;
  they run once per entry to the loop, not per iteration, so leave out a block
  mostly reached from the block before it, where they would run each time. The fetch and
  prediction hardware works in 16 and 32-byte windows, so a short loop that
  crosses one can cost several times its cycles: on the machine this was
  measured on, a 7-instruction loop that ran 4 to 15 times had its exit
  mispredicted on most entries until it was aligned. A branch that may jump
  over padding must allow for it when you shorten it to `rel8`.
- The decoded-instruction cache delivers one 32-byte window per cycle, so an
  innermost loop of up to 64 bytes that a 16-byte head would spread over one
  more window starts on a 32-byte boundary instead. The function then starts
  on one too, and so must its text section. Decide this after the branches are
  shortened, since their lengths set the loop's size; until then, allow 31
  bytes of padding before such a head. In a 7-instruction matrix loop that
  crossed a window, this took 15% of the cycles off. Do this only for a
  counted loop, one whose every exit tests a counter stepped by a constant
  against a value the loop does not compute. The padding runs on each entry,
  and a search loop that usually stops after one or two iterations ran 7%
  slower with it.
- A load forwards from one earlier store that covers it, not from two. An
  8-byte object returned in `rax` from two 4-byte stores to its slot waits
  about 12 cycles for the stores to retire. Form the value in `rax` from the
  stored registers instead (`zext.i32` of the low half, `or` of the high half
  shifted by 32). In a program whose bit-board operators return such objects,
  this took 6% of the cycles off.

When a change does less than expected, look for a constraint stricter than
emission needs before tuning a heuristic. A phi transfer modeled at the
predecessor instead of on its edge is one such constraint, and removing it
is worth more than any threshold. Raising thresholds and caps usually nets
little.

The course solution meets the bounds with a liveness-based allocator of this
kind for values that cross blocks. It is computed before instruction
selection. Values used inside one block get registers as they are selected,
and the selector keeps two general and two vector registers back as scratch.
A function with exception regions is not a reason to keep its cross-block
values in memory. A value needs a home a handler can find only where that
handler reads it, and only across the calls that can reach the handler;
model what the runtime does to registers at a throw as what those calls
clobber, and let the allocator price each value's home from that. Every
other value keeps its register as in any function. The exceptions kernel of
`make test-kernels` measures this directly. Selecting into virtual registers
and assigning them afterwards, with a coloring or linear-scan allocator,
meets the same bounds, and later work has found it easier to extend.

Keep each improvement small enough to validate with an existing fixture.
Compare your own O0 and optimized dumps to understand the redundant work, then
read the failing expectation to see the required bound. Reference dumps are
examples of one successful answer. They are not pass recipes.

For performance investigation beyond the assignment, use programs that run
long enough to measure and depend on runtime inputs. Keep their results equal,
build outside the timed region, and compare immutable executable versions in
wall-time ABBA order. Repeat A/A runs to estimate noise before interpreting a
small difference. Instruction counts and simulated cache and branch events
are deterministic and resolve smaller differences than time does. Such
measurements help choose profitable changes; the assignment's completion
checks remain deterministic.
