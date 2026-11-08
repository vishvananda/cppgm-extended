## CPPGM Programming Assignment 36 (optional inline assembly)

### Overview

Extend `cppgm++`, `lowir` and the native backend with a shared assembly boundary.
C++ source describes operands and effects; another language frontend can supply
that same contract directly as LowIR. Generate native instructions and object
fixups with your own encoder.

PA36 is optional and outside the required 35-assignment ladder. The current
contract is the operand and local-control-flow subset below. The broader extensions at the end
are separate goals. PA29's bounded GNU compatibility remains unchanged.

### Prerequisites and starter kit

Complete the native, host-object and optimization work through PA33. PA34 and
PA35 are independent of this extension. Reuse your existing implementation in
`dev/` and `dev/src/`; add reusable sources to `dev/frontend_source_sets.mk`.

The starter kit supplies:

- [the assembly boundary](assembly.md), extending PA8's LowIR format;
- [`asm_model.h`](../dev/src/asm_model.h): frontend-neutral operand, constraint, register-view
  and effect vocabulary;
- [`x86_asm_model.h`](../dev/src/x86_asm_model.h): initial x86 instruction-form vocabulary and
  encoding facts;
- `pa36/Makefile`, fixtures under `tests/spec/`, `tests/source/` and `tests/reject/`, and
  `scripts/check_inline_assembly.pl` with host-built native observers.

These headers provide a starting vocabulary. Your containers, parsing,
allocation policy and encoding implementation are your choice. Implement the
compiler in `dev/`; preserve the supplied fixtures and observers.

### Command-line contract

```sh
lowir -o canonical.lowir program.lowir
cppgm++ --full-asm -O0 -c source.cpp -o source.o
cppgm++ -O0 -c program.lowir -o program.o
```

`--full-asm` selects this optional C++ path. Preserve the default PA29 behavior.
Assembly supplied through LowIR needs no C++ flag. Both paths must work at
`-O0` and `-O3`, producing host-linkable x86-64 objects. The initial execution
hosts are Linux and Intel macOS; qualify their object formats separately.

### Assignment boundary

The current subset covers GNU basic/extended statements with register, memory
and integer-immediate or symbol operands, fixed GPR and XMM domains, alternative columns,
matching constraints, early writes, read/write operands, flag outputs and
declared register clobbers. Support `m`/`o` memory constraints, `i`/`n`
constants, `g` mixed locations and the integer ranges in `x86_asm_model.h`.
Letters within a constraint alternative describe a union; select one alternative
column jointly across every operand. A commutative pair may exchange its captured
inputs to satisfy the joint constraints, preserving each value and storage
identity. Overlapping pairs are invalid. GNU immediate constraints see the
operand's machine-mode bits, including sign extension of narrow constants.
GNU `i` accepts link-time symbol addresses; `n` requires a numeric constant.
Transport symbol identities and byte addends through LowIR. Support
RIP-relative symbol addresses/loads, GOT-relative addresses and direct symbol
calls. Preserve declared stack effects and ABI alignment at assembly calls;
the producer must declare all registers and machine state its call can change.
Output places include scalar/vector lvalues and bit-fields. Capture each
selected place once, then assign register results in operand order, preserving
neighboring bits and volatile accesses. Constraints requiring a bit-field's
memory address are impossible and must fail. Register classes select the
physical bank: scalar floating values can travel through GPRs, and scalar
integer values through XMM registers, preserving their bits.
Preserve named operands, supported width modifiers, volatility and memory/state
effects through the [documented LowIR records](assembly.md).
Evaluate each memory address once. A register alternative may load from that
captured address; a memory alternative names its storage directly. Do not add an
unrequested read of an output-only memory operand.

Support the initial integer/SSE forms described by the supplied encoding facts,
in Intel and AT&T syntax, including their register widths and addressing forms.
Include unary arithmetic, shifts/rotates, `test`, multiply forms and
`xadd`/`cmpxchg`. Validate `lock` against a lockable memory destination and
repeat prefixes against string instructions. Implicit register operands and
their clobbers remain the producer's responsibility.
Support named and numeric local labels, forward/backward numeric references,
GNU `%=` labels, and direct `jmp`/condition-code branches within one template.
Keep label identities distinct when optimization clones or inlines a template.
Support GNU `asm goto`, named/numeric outgoing label references, outputs on
outgoing edges and normal C++ scope-exit cleanup. Numeric label references
count a `+` output twice. Represent destinations as real LowIR control flow;
the shared boundary's exit-tag output can feed an ordinary `switch`.
Preserve over-aligned local storage across declared register clobbers.
LowIR selects the dialect; the current GNU source adapter uses AT&T. Choose one
alternative column jointly across operands. Respect register aliases
and lifetimes, preserve the host ABI, and capture outputs before assigning
possibly aliased output places in the producer's order.

Keep ordinary compiler code in typed LowIR and MIR. Parse supplied assembly once
after binding its placeholders, then emit bytes and fixups directly. Production
compilation must not invoke an external assembler or depend on an encoding
library. A compiler memory barrier must not introduce a hardware fence.

### Suggested implementation order

1. Represent, validate and roundtrip templates and typed invocations.
2. Bind a pointer and a full 128-bit input for the streaming-store fixture.
3. Add outputs, ties, alternatives, early writes, flags and clobbers, checking
   aliasing and live values around the effect.
4. Extend instruction forms and addressing through the same parser and encoder.
5. Validate hand-written LowIR independently of C++ source. An external
   frontend can construct those same records.

### Testing and error handling

From the repository root:

```sh
make -C pa36 test
make -C pa36 check TEST='tests/source/200-cpuid.t'
```

These targets use the shared Perl parallel runner and comparer. Set
`CPPGM_TEST_JOBS` to limit workers. Use `test-nobuild` after building the tools
when iterating. Set both `CXX` and
`CPPGM_HOST_CXX` when selecting a host toolchain. The observer needs `objdump`
on Linux or `otool` on Intel macOS.

Tests check native behavior, complete value bits, guard bytes, ABI preservation,
output ordering and LowIR roundtrips. Portable source cases also run with the host
compiler. A `gnu-native` profile uses the supplied observer directly when host
compiler behavior does not provide an independent oracle; it still checks
source compilation and LowIR roundtrips at every optimization level. Allocation choices and surrounding instructions are unconstrained;
assembly instructions explicitly requested by the source must be emitted.
Reject malformed syntax, inconsistent constraints and type errors with failure.
Report unsupported native capabilities clearly. Exact diagnostic text is not
part of the contract.

Finish this optional subset with `make -C pa36 test`, while keeping the required
`make test-report-through-pa33` and `make inception` checks passing. PA36 is not
included in those reports or in the required course completion criteria.

### Further extensions

Global/naked assembly, directives, further relocations, unwind state and broader
x86/VEX/EVEX coverage remain extension goals. Original Rust assembly is a useful
cross-frontend consumer. These goals require additional contracts and tests;
the current suite does not establish their completion. Header vocabulary for
additional register banks or extension records does not itself add a native
requirement to the current subset.
