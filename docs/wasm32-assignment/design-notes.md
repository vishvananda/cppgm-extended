# Approaching a second backend

The [assignment](README.md) defines behavior and the target profile. These
notes discuss design choices; they do not require a particular pass pipeline
or instruction sequence.

## Separate semantics from machine choices

LowIR is the reusable optimization boundary. Its operations describe typed
program behavior; machine lowering decides how the target carries it out.
Make source layout a target choice before producing LowIR. Changing a pointer
from eight bytes to four after laying out classes cannot repair earlier
`sizeof`, template arguments or constant expressions.

Some backend decisions need facts that object size alone cannot recover.
For example, the Wasm ABI can pass a singleton aggregate like its scalar
member, while a same-size aggregate with a different shape may be indirect.
Carry typed shape/calling facts from semantic analysis rather than guessing
from a name or a byte count.

## Share the useful machine concepts

Functions, blocks, use/definition relationships, control edges, frame objects,
call effects and source locations are useful across targets. Physical
registers, flag resources, addressing modes, prologues and instruction
encodings are target choices. A common machine layer can expose the former
without making every instruction carry every target's fields.

Use operand and ABI-piece ranges rather than assuming a fixed number of
arguments or two result pieces. Preserve implicit clobbers and unusual
assembly constraints on native targets. Represent a frame object by identity,
size and alignment until its backend assigns a location. Keep target-neutral
analyses independent of a particular frame register or register-set bit width.

Check the design against a register target as well as Wasm. An ARM-style
three-address operation and several floating aggregate argument pieces should
fit without pretending they are x86 instructions. Wasm locals need types and
lifetimes but do not need callee-saved register masks. Supporting a common
analysis does not mean running it in every backend.

Keep per-function work bounded, reuse scratch storage, and let dumps read the
same model that emission uses. Avoid copying a complete instruction stream
merely to give a generic pass another view of it.

## Lower control flow deliberately

Begin with typed locals and correct explicit edge transfers. Phi moves are
parallel assignments: a cycle such as swapping two values needs a temporary
or an equivalent safe sequence. Critical edges may need their own move sites.

Find natural loops and joins before forming Wasm control regions. Preserve
branch targets when nesting blocks, and remember that branch depths change
as nesting changes. Irreducible flow needs a general fallback, but a fallback
used for every ordinary loop adds dispatch work unnecessarily. Bound code
duplication when simplifying difficult regions.

After correctness is established, keep short expression chains on the value
stack. A value with multiple readers often belongs in a local; duplicating its
producer is valid only when effects, traps and cost permit it. Address-taken
objects belong in linear memory, while ordinary arithmetic values usually do
not. Restore the linear-memory stack across normal and exceptional exits.

## Distinguish representations and index spaces

An `i32` value may represent a signed integer, an unsigned integer, a memory
address or a function-table index. Their identical storage width does not
make their uses interchangeable. Null function pointers and table element
placement must agree across separately compiled objects.

Preserve narrow signedness at calls and memory accesses. Wide integers and
binary128 values need legalization and ABI handling even when arithmetic is
supplied by helpers. Source `long double` on Wasm is not an x87 value with a
different byte count. Reuse constant/soft-float work only where its format and
rounding agree with the target.

Keep section indices, symbol identities, function indices, table entries,
types, tags and linear-memory addresses distinct while constructing objects.
A relocation points to an encoded field, whose reserved size and signedness
matter. Finalizing one section can change later offsets, so decide lengths
without repeatedly rebuilding the complete module.

## Reuse optimizations with target-aware costs

Constant propagation, dead-code removal, inlining and memory reasoning remain
useful. Audit their type widths and semantic assumptions first. Then ask if a
rewrite is profitable for this target. Sinking an address can reduce native
register pressure but repeat explicit shift/add work on Wasm. A larger switch
table or an unrolled loop can reduce branches while increasing module size.

Do not widen or speculate a load solely because a host CPU tolerates it. A new
out-of-bounds access may trap. Preserve volatile effects and the distinction
between a C++ exception, undefined source behavior and a runtime trap.
Floating rewrites must retain the required NaN, signed-zero and rounding
behavior unless the program explicitly permits otherwise.

## Check the boundary in both directions

Small mixed-object tests expose ABI mistakes more clearly than a large
program. Vary caller and callee producer, singleton versus padded aggregate,
direct versus indirect call, varargs, and wide parameters/results. Test
initialization and cleanup as observable sequences, including exceptions that
cross translation units.

Use LLVM's output to understand a difference, then reduce it to a semantic
test with a clear expected result. An independent validator checks module
structure, not source correctness; an executable that returns the right value
once does not establish interoperability or persistent-state behavior.

Compare performance with the same engine and settings. Separate compilation,
module instantiation and repeated kernel execution, and consume computed
results. A local-count reduction or shorter module is useful evidence, but
measure runtime before concluding that it improved execution.
