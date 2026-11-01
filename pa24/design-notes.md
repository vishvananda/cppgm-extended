# PA24 design example

The [assignment README](README.md) defines the requirements. These notes
describe the structure of the course solution's backend and the seams that
later assignments build on.

### Design Notes (Non-Normative)

The course solution's backend runs these stages for each function:

- read LowIR into the PA8 model (`cppgm++` hands the backend the same model
  directly)
- decide where each value that lives across blocks is placed
- select instructions into a machine-IR function, giving each value used only
  inside one block a register as it is selected
- finish the frame: its size, slot offsets, saved registers, prologue and
  epilogues
- dump that machine-IR function deterministically, or encode the same
  function into target code and data and write the image

Reuse PA8's LowIR machinery and build the encoding, layout, and fixup
components introduced in [the native encoding lesson](native-encoding.md).

At `-O0` the placement is simple. A value used outside its defining block
takes one register over its whole span in block order: a linear scan gives
at most seven general registers out at once, preferring a parameter's
incoming register and, for a value live across a call, a callee-saved
register. Any other such value takes a frame home, and a function with
exception regions keeps all of them in frame homes. A value used only inside
its block takes a free register as it is selected; one that finds none is
computed in a reserved scratch register and stored to a frame home.

#### Seams for later work

PA24's fixtures are small functions at `-O0`. PA33 adds register allocation
and machine-IR passes to this backend, and PA34 compiles the compiler with
it. These seams cost little to leave in place now and much to cut in later:

- **Keep values distinct from their locations.** Prefer selecting into typed
  virtual registers, with fixed ABI operands and instruction constraints
  explicit, then assigning physical registers or frame homes over the selected
  instructions. A simple scan suffices at `-O0`; PA33 should replace the
  placement policy without rewriting selection. Early placement, as in the
  example above, is another option, but retain value identities and keep its
  predicted uses, definitions and clobbers consistent with emission. Later
  passes should not have to reconstruct value identity from reused physical
  registers or frame bytes. The required MIR dump still describes the final
  physical placement, not this optional internal virtual form.
- **Each machine instruction states the registers it defines, uses and
  clobbers**, fixed registers (division, variable shifts, calls, string
  operations) and tied operands included, so that liveness, allocation and
  later cleanups read one description. If clobbers are predicted apart from
  emission, the two must agree: a disagreement is wrong code.
- **Keep back only the registers an encoding needs.** A register set aside
  as the selector's scratch is unavailable to every value of every function,
  and is paid for in spills and in longer encodings. The course solution keeps
  two general and two vector registers back.
- **Finish the frame last.** Assign frame offsets after placement and
  selection, so that a later pass can drop unused slots, share slots whose
  lives do not overlap, and put the most used objects within an 8-bit
  displacement.
- **Generate prologues, epilogues and unwind rows from the final frame
  facts** that the dump records (`frame_pointer keep|omit`,
  `epilogues shared|duplicated`, the `preserve` list), not from a fixed
  `push rbp; mov rbp, rsp` shape. PA26 writes host unwind tables from the
  same facts, so an optimizing backend can omit the frame pointer, push
  callee-saved registers or give each return its own epilogue without
  changing the unwind writer.
- **Keep one control-flow view of the machine IR** (blocks, successors, loop
  depth) that every pass shares, so that a switch lowered to a table is a
  terminator whose targets the passes, the block layout and the encoder all
  know.
- **Keep each operand's width**, so that a later pass can remove an extension
  whose upper bits nothing reads.

Before finishing PA24, trace a value across a call, a fixed-register operation
and a control-flow join. Check that changing its placement needs neither a
second selector nor an analysis that recovers its identity from emitted moves.
Keep the allocator's input faithful when selection folds an operation or
introduces a temporary; LowIR liveness alone does not describe those changes.

#### Shapes the fixtures check

The README's goals give the shapes that the strict MIR fixtures check, and
each holds under any placement policy: integer immediates kept as operands,
memory right operands, extending narrow loads, `index` addresses folded into
memory operands, division and variable-shift operands placed directly in
their fixed registers, a parameter or call result read from its ABI register
while nothing clobbers it, phis lowered as parallel copies on their edges,
and object copies and zeroing kept whole as `copy_bytes` and `zero_bytes`.

These are suggestions, not required internal data structures or algorithms.
Any implementation is acceptable if it preserves program behavior and produces
the checked strict or structural MIR output.
