# Optional assembly boundary

An assembly template has a separate name space from functions and globals.
Define it before invoking it. It owns source fragments and operand descriptions;
an invocation supplies ordinary LowIR values, so no frontend AST or temporary
identity crosses this boundary.

```text
assembly @stream intel write 1 0 0 2 {
  operand ptr 0 -1 1 0 0 {
    alternative 1 0 -1 { group (r10:0) }
  }
  operand v128 1 -1 1 0 0 {
    alternative 1 0 -1 { group (xmm14:0) }
  }
  text "movntps ["
  use 0 0 0 0
  text "], "
  use 1 0 0 0
}
function @stream_ps(%p : ptr, %v : v128) -> void {
  block ^entry:
    asm @stream(%p, %v)
    return void
}
```

The registers above are deliberately fixed to make the example small. General
register constraints list several `group` choices; allocation may choose any
valid joint assignment. An ordinary operand group contains one register; wider
bindings may list several. `:0` means to derive the register's view from the
operand type. Register names otherwise specify their architectural view;
`:bits[:offset]` can override it explicitly, including mask-register views.

The template header gives dialect (`att` or `intel`), memory effect (`none`,
`read`, `write`, `readwrite`), property bits, state-read bits, state-write bits,
and invocation argument count. The vocabulary is in `dev/src/asm_model.h`:

| Property bit | Meaning |
|---|---|
| 1 | Observable side effect |
| 2 | Producer permits duplication/elimination according to its language rules |
| 4 | Assembly may use the stack |
| 8 | Assembly may write the red zone |
| 16 | Unwinding is permitted |
| 32 | No normal continuation |
| 64 | Advisory small inlining cost |

State bits are condition flags (1), x87 status (2), MXCSR status (4), x87
control (8), MXCSR control (16) and the direction flag (32). They describe
promises about machine state, not hardware fences. A memory clobber is a
compiler effect; it must not introduce a CPU fence.

An `operand` gives its value type, input argument index, output-address argument
index, access bits, input-storage flag and flag-output condition. `-1` means
absent. Access bits are read (1), write (2), commutative pair (4), flag output
(8), volatile storage access (16), prohibited memory binding (32) and exit-tag
output (64). The memory-binding flag permits producers to materialize value-only output places,
such as bit-fields, without offering that temporary as an assembly memory
operand. Register inputs are captured values;
explicit storage inputs and output arguments are pointers. Flag conditions use
the architectural x86 condition-code nibble. Output places may alias: capture
results before storing them in operand order. Discarded outputs have no address.

Each `alternative` gives allowed location bits, an early-write flag and a
matching-output operand index (`-1` if absent). Location bits are register (1),
memory (2), immediate (4), symbol (8) and label (16). All ordinary operands have
the same number of alternatives, and one alternative column is chosen jointly.
An early output cannot overlap unrelated inputs or their address registers.
A matching input shares its output's location. A commutative flag pairs two
adjacent read-only operands. Their evaluated inputs may exchange constraint
slots when their value types agree; retain whether each input denotes captured
bits or explicit storage. A pair cannot overlap another pair.
An explicit storage input can use a memory location or be loaded into a selected
register. An output-only memory location uses the output-address argument.
For a read/write memory binding, the input-storage and output arguments name
one place. Distinct input and output places can instead use a register binding.

A register alternative lists `group (...)` choices. Immediate domains use
`range <kind> <signed> <low-lo> <low-hi> <high-lo> <high-hi>`; kinds are integer
range (0), floating bits (1), relocatable (2), with each endpoint represented by
two unsigned 64-bit words. An omitted domain permits any constant of the type.
A symbol location uses an ordinary `ptr` input naming a LowIR symbol address,
optionally through `addr` and a constant `index`. The symbol ID and byte addend
remain typed through native parsing and object fixups. Symbol operands are
distinct from numeric immediate ranges. RIP-relative operands can use
`@GOTPCREL`; direct symbol calls use the target's branch relocation rules.
The stack-use property keeps the ABI's call alignment even in a frame-free
function. The producer supplies the call's clobbers and memory/state effects.
`clobber <register>` records an architectural clobber independently of operands.
Integer ranges compare the invocation's complete two-word value. A frontend
that uses machine-mode constants normalizes them before constructing LowIR.

Template pieces are `text "..."`, `use <operand> <format> <bits> <offset>`, or
`unique`. Formats are default (0), bare (1), negated (2), label (3), suffix (4).
Zero width chooses the operand's view; offset 8 selects an available high-byte
view. The frontend handles its own string escapes and placeholder syntax. The
backend parses the resulting assembly source once and retains typed operands.
Named/numeric labels and branches stay within the template. Numeric `1f`/`1b`
references select the nearest matching label in that direction. A `unique`
piece supplies a label suffix; every emitted occurrence has its own label
identities, including clones of one invocation.

An outgoing label operand has type `i32`, read access, a constant tag in
`1..2147483647`, and only label alternatives. A packet containing labels has
exactly one exit-tag output: type `i32`, access `66` (write + exit tag), an
output-address argument, no input and no alternatives. It has no template
reference and consumes no register constraint. A label reference uses format
0 or 3, with zero width/offset. Labels cannot participate in commutative pairs.

The output receives zero on fallthrough or the chosen label's tag. Feed it to
ordinary LowIR control flow so dominance, liveness, cleanup and cloning retain
the outgoing edges. For example:

```text
assembly @take intel none 1 0 0 2 {
  operand i32 0 -1 1 0 0 { alternative 16 0 -1 {} }
  operand i32 -1 1 66 0 0 {}
  text "jmp "
  use 0 3 0 0
}
function @take_edge() -> i32 {
  slot $tag : i32
  block ^entry:
    %p = addr $tag
    asm @take(7, %p)
    %edge = load i32 $tag
    switch %edge, ^fallthrough, 7:^taken
  block ^taken:
    return i32 1
  block ^fallthrough:
    return i32 0
}
```

Capture ordinary outputs before dispatching the edge. The source frontend
enforces its language's output availability and scope-entry rules and emits
any required cleanup on the selected edge. Assembly must restore its stack
state before reaching an outgoing label, just as on normal continuation.

The reader checks bounds, alternative columns, access promises, references and
invocation types. Native capability checks are separate: a packet can be
structurally valid while requiring an unsupported register class, instruction
form or control-flow feature. Such cases must fail clearly. The current native
subset is listed in the [handout](README.md).
