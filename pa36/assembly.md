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
and invocation argument count. An optional `environment N` before `{` selects
macro definitions after the first N ordered `assembly_global` invocations for
ordinary or raw-function packets; omission selects the empty environment.
The index must be within the module's invocation count. Module invocations
themselves always expand in their declared order. A packet's local definitions
remain within that packet (or consecutive packets of one raw body with the
same environment). Snapshots preserve definitions across later redefinition or
purging; expansion numbers share the unit's bounded work budget. Typed symbol
and outgoing-label pieces remain typed through macro argument substitution.

The vocabulary is in `dev/src/asm_model.h`:

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
Source `.intel_syntax`/`.att_syntax` directives can change dialect and register
prefix mode within a packet; its local-label scope survives those switches.
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
subset is listed in the [handout](README.md). VEX instruction operands can use
literal XMM/YMM views independently of ordinary LowIR value types. A 256-bit payload uses existing `obj<32xalign>` storage with YMM alternatives,
including stored input values or explicit input-storage pointers. Object
alignment describes its storage; transfers must preserve all 32 bytes even
when unaligned. XMM and YMM views of one register share an allocation unit.
Capture full results before ordered output stores. This does not add a wide
vector call ABI; larger values and further register classes remain separate
capabilities.

## Nonreturning invocations

Property 32 declares an invocation with no ordinary continuation. It has no
output operands, and its `asm` instruction is immediately followed by LowIR
`unreachable`. This retains real control flow through validation, optimization
and native emission; the backend must preserve the assembly's effects before
it stops. Unwinding remains a separate capability.

This invocation promise does not replace an assembly-defined function's typed
ABI declaration. Its opaque raw body can return through the ABI it implements.

## Assembly-defined functions

A function implemented entirely in assembly keeps an ordinary typed declaration
for its callers and has an `assembly_function` definition. Declare the function
and define its templates before the raw body:

```text
assembly @add_constant intel none 1 0 0 1 {
  operand i64 0 -1 1 0 0 { alternative 4 0 -1 {} }
  text "lea rax, [rdi + "
  use 0 1 0 0
  text "]; ret"
}
declare function @add_23(%value : i64) -> i64
assembly_function @add_23 {
  block ^entry:
    asm @add_constant(23)
    unreachable
}
```

The body has one block containing one or more `asm` invocations followed by
`unreachable`. It has no slots, computed values or ordinary instructions.
Invocation arguments are literal constants or symbol addresses; operands are
read-only immediate/symbol alternatives. No operand may require compiler
register setup or output storage. The declaration supplies the callable ABI;
its parameters are available in their ABI registers or stack locations,
not as runtime LowIR operands in this body.

The assembly owns the complete entry/exit sequence, preserved registers, stack
alignment and result. There is no automatic prologue, epilogue or return value. By default there
is no unwind description. Arrange an explicit return, tail transfer or other terminal
path; reaching the end traps. Local labels belong to each template invocation;
put cooperating labels and instructions in one template.

Ordinary optimization treats the function as opaque: it must not infer a
constant result, remove its ABI parameters, inline its raw body or infer
`noreturn` from the LowIR `unreachable` marker. Explicit declaration effects
still apply. Normal symbol binding, sections and object-name metadata apply to
its definition. Reject duplicate bodies and attempts to mix raw and ordinary
function definitions. An optional `initial` word after the function name requests entry/exit unwind
records for that raw body. The default (`none` in the vocabulary) supplies no
records. For example, `assembly_function @add_23 initial { ... }` starts with
CFA = `rsp + 8` and the return address at `CFA - 8`. It emits no frame code.
The body supplies CFI for subsequent register/stack changes. A bare body can
provide its own `.cfi_startproc`/`.cfi_endproc` instead. Keep this function-level
choice distinct from property 16, which permits unwinding from a packet.

## Ordered module assembly

`assembly_global @template(arguments)` invokes a previously defined template at
module scope. Its arguments are constants or symbol addresses; it has no
runtime inputs, outputs, outgoing labels or compiler register setup. Templates
use only immediate/symbol alternatives. Invocations remain in source order,
sharing their module symbol and section environment. Each invocation selects
its template's initial dialect; directives can change it within the invocation.

```text
assembly @module_body intel none 1 0 0 1 {
  operand i64 0 -1 1 0 0 { alternative 4 0 -1 {} }
  text ".text\n.globl add_constant\nadd_constant:\nlea rax, [rdi + "
  use 0 1 0 0
  text "]\nret\n"
}
assembly_global @module_body(23)
```

Labels and literal symbol names refer to the target assembler's linker symbol
namespace. Typed symbol pieces retain their identity, including at a definition.
Preserve weak replacement, visibility, aliases and relocations; symbol binding
and type declarations apply to the whole module, including earlier uses.
An exposed symbol call must remain interposable even when its definition
appears nearby.
C++ declarations and other LowIR functions can call these definitions through
ordinary signatures. Unresolved references are resolved by the linker.

The current module subset emits code, data, BSS, strings, alignment, symbol
attributes and aliases into ELF or Mach-O objects. `lowir2native` uses the existing
object linker for module-containing standalone programs. A module has no
ordinary MIR function body; a requested ordinary MIR dump must fail clearly.
Assembly-defined code owns its ABI and supplies its own CFI start/end and
frame rules when it needs an unwind description.
Module macros, repetition and conditionals expand supplied source before its
instruction parse. Typed symbol spans must survive argument substitution and
remain distinct at definitions and references. Definitions persist across
ordered module invocations; `environment` selects a snapshot for inline and raw
invocations. See the handout for supported directives and
absolute expressions. The supported CFI rules and symbol-pointer encodings are listed in the
handout. Wider directive coverage remains a further goal.
