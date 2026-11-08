## CPPGM Programming Assignment 36 (optional inline assembly)

### Overview

Extend `cppgm++`, `lowir` and the native backend with a shared assembly boundary.
C++ source describes operands and effects; another language frontend can supply
that same contract directly as LowIR. Generate native instructions and object
fixups with your own encoder.

PA36 is optional and outside the required 35-assignment ladder. The current
contract covers operands, local control flow, raw functions and module assembly below. The broader extensions at the end
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
Include returns, push/pop, local/symbol/indirect calls and jumps,
unary arithmetic, shifts/rotates, `test`, multiply forms and
`xadd`/`cmpxchg`. Include conditional moves and byte condition results, bit
scans/counts, byte reversal, bit tests/updates, double shifts and wide
compare/exchange. Sign/zero-extension moves have distinct source and destination
widths, including their AT&T names; reject invalid high-byte/REX combinations.
Feature-dependent native observations must check the host CPU before execution.
Preserve scalar SSE move bits and memory widths, including NaN payloads.
Distinguish scalar-double `movsd` from its zero-operand 32-bit string form.
Both dialects' string spellings are supported. Immediate moves preserve flags;
explicit `movabs` accepts numeric and typed symbol addresses, including addends.
Nonreturning packets have no outputs and use an explicit `unreachable`
continuation in LowIR. Validate `lock` against a lockable memory destination and
repeat prefixes against string instructions. Implicit register operands and
their clobbers remain the producer's responsibility.
The VEX form tables in `x86_asm_model.h` add XMM/YMM moves, packed and scalar
arithmetic, comparisons, shuffles, variable/immediate shifts, lane
insertion/extraction, broadcasts, mask extraction and fused arithmetic. Preserve
scalar memory widths and merged upper lanes. Explicit XMM/YMM views must agree
with the form; reject mixed widths, oversized control bytes and legacy prefixes
before VEX. Support `vzeroupper` and `vzeroall`. Native observers check CPU support
before executing feature-dependent forms. Use `obj<32xalign>` storage for 256-bit operand payloads and YMM register
alternatives. Capture every input byte before setting assembly registers and
every output byte before assigning output places. Preserve XMM/YMM aliases,
ties, early writes, volatile places and over-aligned local storage. This adds
assembly transfers, not a general wide-vector call ABI.

Support named and numeric local labels, forward/backward numeric references,
GNU `%=` labels, and direct `jmp`/condition-code branches within one template.
Keep label identities distinct when optimization clones or inlines a template.
Support GNU `asm goto`, named/numeric outgoing label references, outputs on
outgoing edges and normal C++ scope-exit cleanup. Numeric label references
count a `+` output twice. Represent destinations as real LowIR control flow;
the shared boundary's exit-tag output can feed an ordinary `switch`.
Preserve over-aligned local storage across declared register clobbers.
Support GNU `__attribute__((naked))` functions under `--full-asm`. Their bodies
contain only basic asm, nested braces and empty statements; reject extended
operands and ordinary C++ statements. Combine their user source fragments so
labels can span basic statements. The assembly owns all ABI inputs, results,
stack changes and preserved registers. Emit no automatic frame or return.
Use the shared `assembly_function` contract for this and other frontends' raw
functions; callers retain ordinary typed declarations. Keep raw bodies opaque
to inlining and return/argument inference. Request initial unwind state for GNU naked functions; describe subsequent
stack and saved-register changes explicitly with the directives below.

LowIR selects the initial dialect; the GNU source adapter starts in AT&T.
Honor `.intel_syntax` and `.att_syntax` with `prefix`/`noprefix` register modes.
Syntax changes preserve the block's named and numeric labels. Choose one
alternative column jointly across operands. Respect register aliases
and lifetimes, preserve the host ABI, and capture outputs before assigning
possibly aliased output places in the producer's order.

Keep ordinary compiler code in typed LowIR and MIR. Parse supplied assembly once
after binding its placeholders, then emit bytes and fixups directly. Production
compilation must not invoke an external assembler or depend on an encoding
library. A compiler memory barrier must not introduce a hardware fence.

### Module assembly

Under `--full-asm`, namespace asm uses the shared `assembly_global` boundary.
Keep module invocations ordered and preserve their symbol/section state. Emit
code, scalar data, pointer relocations, BSS, strings and alignment directly.
Support `.text`, `.data`, `.bss`, `.rodata`, `.section`, the section stack and
`.previous`; `.byte`/`.short`/`.word`/`.long`/`.quad`, `.zero`/`.space`/`.skip`,
`.ascii`/`.asciz`/`.string`, and `.p2align`/`.balign`/`.align`. Respect target
alignment and section spelling differences.

Preserve global/weak/local binding, hidden visibility, `.type`, `.size` and
symbol aliases with `.set`/`.equ`. Support symbol addresses and constant addends,
direct/conditional symbol branches, PLT references and calls to other objects.
A weak definition must be replaceable even for a nearby call in its assembly
buffer. Typed symbol pieces may define or reference linker symbols without
reconstructing frontend names. Reject duplicate definitions, invalid section
state, cyclic aliases and unbounded storage requests.

The same module records work in host-linkable objects and standalone
`lowir2native` executables. Ordinary function MIR dumps do not represent module
items; reject that unsupported combination. Support `.macro`/`.endm` and `.purgem`, positional or named arguments, defaults,
`:req` and final `:vararg` parameters. Preserve quoted argument grouping and raw
variadic source, including typed symbol pieces. Macro names are case insensitive;
parameter names are case sensitive. Include nested definitions, `\@` unique
expansion numbers and `\()` parameter separators.

Support `.rept`, `.irp` and `.irpc` blocks ending in `.endr`, and `.exitm` from
the current macro or repetition. Include `.if`/`.ifne`, `.ifeq`, signed
`.ifge`/`.ifgt`/`.ifle`/`.iflt`, blank `.ifb`/`.ifnb` and string
`.ifc`/`.ifnc`/`.ifeqs`/`.ifnes` conditions, with `.elseif`, `.else` and `.endif`.
Numeric conditions and repetition counts accept absolute integer expressions,
using GNU assembler precedence and truth values. Expansion must have bounded
nesting, work and output size; malformed blocks and runaway recursion fail.

Namespace macro definitions are available to later inline and naked assembly.
Capture the source environment at each statement: subsequent redefinition or
purging must not change earlier code, including deferred members, template
instances and inlined copies. Preserve typed operands and outgoing labels
through expansion. Definitions inside one inline packet remain local to it.
Symbol-dependent expressions remain an extension goal.

### Unwind descriptions

Module assembly supports `.cfi_startproc` (including `simple`) and
`.cfi_endproc`. Raw functions may instead request the shared boundary's
`initial` mode: it supplies the entry/exit unwind records, with the initial
x86-64 CFA at `rsp + 8` and the return address at `CFA - 8`. GNU naked functions
use that mode. It adds no frame instructions. The producer describes its own
stack changes and saved registers; omit explicit start/end directives in an
`initial` body.

Support CFA definition/register/offset/adjustment, saved-register offset,
relative offset and value offset, register transfers, restore/undefined/same
value, remember/restore state, return column and signal-frame directives.
CFI does not interrupt assembly-local label scope. `.cfi_escape` emits the
supplied bytes without changing the assembler's logical CFA bookkeeping, as
in GNU assembly. It permits explicit DWARF expressions and vendor extensions.

Preserve `.cfi_personality` and `.cfi_lsda` symbol identities and pointer
encodings, including omission (255), absolute or PC-relative 32/64-bit fields
and personality indirection. Respect the target's relocation rules; on Darwin
an indirect PC-relative personality uses its GOT slot. `.cfi_sections .eh_frame`
selects the supported runtime-unwind section. Reject malformed procedure/state
nesting, impossible offsets and unsupported encodings or sections clearly.
Native tests unwind host exceptions through both module and raw frames and
observe personality and exception-data addresses.

These directives currently describe module and raw-function assembly. Combining
user CFI with an ordinary compiler-owned inline frame remains an extension goal.

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
A fixture using absolute text addresses declares `no-pie` in its
`.assembly-link-mode` sidecar; other fixtures use the host's default link mode.

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

Broader directives/expressions, further relocations, inline-frame unwind state and broader
x86/VEX/EVEX coverage remain extension goals. Original Rust assembly is a useful
cross-frontend consumer. These goals require additional contracts and tests;
the current suite does not establish their completion. Header vocabulary for
additional register banks or extension records does not itself add a native
requirement to the current subset.
