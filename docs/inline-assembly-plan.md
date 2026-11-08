# Full inline assembly across frontends

Status: implementation in progress, 2026-11-07. This is a maintainer plan, outside the student
export. It changes neither the current 35-assignment contract nor the backend
capabilities advertised to other frontends.

Recommendation: implement one frontend-neutral assembly facility at the LowIR
boundary, backed by an integrated assembler and explicit allocation constraints.
Make the C++ course extension a new **optional PA36**, rather than expanding
PA29's required compatibility work into a full assembler. RustGM already assigns
its corresponding language and target closure to **PA41**; reuse that ownership.
Its first required core-library consumer must land earlier, during Rust PA32.

## What exists

Reviewed cppgm at `adb8a65ad5137658bf87d446e9fd90e8b1dac65e` and the local
`~/rustgm` checkout at `2a13ab4a97857070f9d5d4190044b823170ad0e2`.

| Boundary | Current implementation | Missing for general assembly |
|---|---|---|
| C++ syntax and semantics | [GNU asm parser](../dev/src/syntax/parser_statements.cpp), [typed operands](../dev/src/sema/analyzer_asm.cpp) and a [template lifter](../dev/src/sema/asm_statements.h) | Full constraint alternatives, clobbers, template modifiers, goto labels and ordinary executable assembly templates |
| PA29 compatibility | [Selected GNU asm forms](../pa29/README.md), including CPU queries, packed AND and namespace data definitions | A general instruction/directive language; the existing forms are lifted into other typed operations |
| LowIR | Typed operations, calls, control flow, symbols, debug and object facts | An assembly effect with operands, outputs, constraints, successors and a durable source payload |
| Native backend | Direct x86 encoding, ELF/Mach-O objects, compact per-function MIR | An assembly parser, broad instruction forms, constrained allocation, assembly-defined code/data and their fixups |
| Register model | [16 GPRs and 16 XMM registers in 32-bit sets](../dev/src/native/mir/registers.h); reserved emitter scratch registers | Register views and overlapping units, arbitrary declared clobbers and wider target register classes |
| RustGM | Checked assembly templates/pieces, register inputs and output places, options, source ownership and body effect/output nodes | Native lowering currently reports `inline assembly requires an unavailable locked backend capability`; broader source forms and body transport also remain incomplete |

Rust evidence is in `dev/src/syntax/assembly.rs`, `dev/src/body/assembly.rs`,
`dev/src/body/build/assembly.rs`, `dev/src/lower/function.rs`,
`docs/source-asm-vector-join.md` and
[PA41](https://github.com/vishvananda/rustgm/blob/2a13ab4a97857070f9d5d4190044b823170ad0e2/pa41/README.md).
Its current register classes are narrower than the full Rust x86 contract.
Frontend acceptance therefore does not establish backend support.

RustGM's `backend.lock.json` currently pins cppgm `b7c6b8e72595a524e1b3f147212d69c02b257dd8`.
Its reviewed object capability is x86-64 ELF `lowir-to-object`. Updating cppgm
alone will not update those immutable tools or authorize assembly emission.

## Scope and completion language

Start with the existing targets: x86-64 Linux ELF and Intel macOS Mach-O. RustGM's
initial integration is Linux; macOS backend support must be qualified separately
before RustGM advertises it. Other architectures and Microsoft's `__asm { ... }`
are separate extensions.

For those targets, the eventual contract covers:

- GNU C++ basic/extended asm, operand constraints and modifiers, flag outputs,
  clobbers, asm goto and assembly-defined functions/data.
- Rust `asm!`, `global_asm!` and `naked_asm!`, including its operand directions,
  register classes/views, const/symbol/label operands, ABI clobbers and options.
- Intel and AT&T assembly syntax, local labels, expressions, instruction
  prefixes and addressing, target directives, relocations and unwind directives.

Freeze a supported ISA and assembler-dialect version. Maintain a coverage matrix
for forms, register classes, directives and object relocations. Include base
x86-64, x87/MMX/SSE, VEX/AVX and EVEX/AVX-512 coverage; track newer extensions
explicitly rather than implying that every future x86 instruction is supported.
Encoding support does not permit executing instructions absent from the host.

Language acceptance, allocation support, instruction encoding, object-format
support and native execution are distinct capabilities. Partial releases name
their supported subset. Neither one successful Rust intrinsic nor a `.byte`
escape earns a claim of full inline-assembly support.

The source-language references are [GNU extended asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html),
[GNU constraints](https://gcc.gnu.org/onlinedocs/gcc/Constraints.html) and
[Rust inline assembly](https://doc.rust-lang.org/reference/inline-assembly.html).
Pin the references used by each frontend; RustGM's course source profile is
Rust 1.98.0. A newer online reference does not silently extend that profile.

## 1. Establish the shared contract

Add an assembly operation plus optional owned payloads to LowIR. C++ and Rust
construct the same model directly; the textual LowIR reader/writer is the adapter
for an external frontend. Rust source must never pass through C++ syntax or sema.

The payload needs these facts:

| Area | Required information |
|---|---|
| Source | Target, assembly dialect, template fragments and operand references/modifiers, source spans and stable ownership |
| Operands | Typed inputs and multiple outputs; register, memory, immediate, symbol and label kinds; alternatives, ties and early/late overlap rules |
| Machine state | Register classes or explicit physical views, declared clobber units, flag/state effects and ABI-expanded clobbers |
| Memory | No access, read-only or read/write; explicit memory operands and conservative unknown memory effects |
| Execution | Observable side effects, permission to eliminate/duplicate, stack/red-zone permissions, normal and exceptional exits, divergence |
| Control flow | Fallthrough and named outgoing edges; availability of results at each successor; assembly-local labels kept separate from IR block labels |
| Linkage | Real symbol identities, addends, relocation intent and feature requirements; no reconstruction from mangled-name strings |

Use a side table or equivalent optional storage so ordinary instructions keep
their current compact representation. Design multi-output use/definition and
edge-result rules before adding opcodes; a result-list or effect/result model
must participate in ordinary dominance, liveness and cloning.

Translate frontend rules once. GNU constraint alternatives remain a joint
choice across operands; Rust's single register-class request is simpler. Rust
`out`/`lateout` and `inout`/`inlateout` need different overlap lifetimes. Rust
operand evaluation and output stores retain its checked left-to-right order,
including overlapping output places. Do not impose that ordering on C++ where
its contract differs. Constants and symbols come from the existing constant and
symbol producers. ABI names expand to target-owned register sets before native
allocation, without importing Rust types into cppgm.

Default to conservative memory, state and side-effect facts. Volatility, unknown
memory access and a hardware fence are different properties. In particular a
compiler memory clobber must not synthesize a CPU fence. Narrower Rust options
are producer promises, not facts inferred by recognizing an instruction string.
Unwinding permission follows the pinned language contract: unsupported unwinding
must be rejected, not silently treated as a normal call.

Publish the model, builder, text grammar, reader/writer, validator and capability
description together. Update the LowIR contract/audit tables and shipped
vocabulary consistently. Missing capabilities must give a useful diagnostic at
the originating assembly, including when supplied through external LowIR.

Keep ownership in the existing layers: C++ spelling/constraint checks in
`syntax`/`sema`, C++ value/place evaluation in `lower`, the shared contract and
adapters in `lowir`, assembly parsing/binding/encoding under `native`, and
symbol/section/relocation emission in `native/object`. A prospective
`native/asm` module should not depend on C++ syntax or sema. Register new source
files in `dev/frontend_source_sets.mk`. RustGM owns its source rules, checked
body codec and Rust-to-LowIR producer, with tool installation in its existing
external backend bridge.

## 2. Parse assembly once and emit objects directly

[spec.md section 7](../spec.md) now explicitly permits **user-supplied assembly
templates**. Bind their typed placeholders, then parse each resulting source
instance once into typed assembly operands and encode bytes/fixups directly.
Rendering ordinary compiler-generated instructions to assembly and parsing them
back, and external-assembler fallbacks, remain prohibited.

Frontend parsers handle their own string escapes and placeholder grammar. Keep
those references typed through LowIR; the backend performs their language-defined
expansion after binding, then owns the only instruction/directive parse. This
also handles placeholders embedded in mnemonic suffixes or symbol spellings
without an early speculative instruction parse. Cache the parsed representation
where bindings are identical. Unused inline bodies retain the existing
demanded-emission boundary for native limitations. Macro/repetition expansion
must have explicit work limits and parse each resulting occurrence once.

Keep opaque assembly intact through optimization. The optimizer may use its
declared effects, but must not replace an explicitly requested instruction with
a purportedly equivalent C++ operation. Existing PA29 lifted forms can retain
their current baseline contract while the general facility is introduced.

Assembly-local label identities must survive inlining and cloning without
collisions, including numeric forward/backward labels and GNU unique labels.
Module assembly requires an ordered assembly environment for macros, aliases,
section state and forward symbol references; it is not just a function-local
operation placed in a synthetic function. Naked functions need their own
prologue, ABI, debug and unwind policy.

## 3. Extend allocation and machine-state accounting

This is a correctness prerequisite, not merely an encoder extension:

- Solve register/memory/immediate alternatives with matching operands,
  commutativity, early clobbers, fixed registers and address-register liveness.
  Preserve live values with explicit spill/reload or moves; diagnose impossible
  constraints. Use deterministic bounded searches over declared alternatives.
- Model overlapping views: AL/AH/AX/EAX/RAX, high-byte/REX restrictions,
  XMM/YMM/ZMM overlap and extended vector/mask registers. Add x87 stack, MMX and
  other advertised state/classes as their coverage is enabled.
- Honor clobbers even for registers ordinary selection never allocates. The
  emitter's current scratch-register convention cannot bypass an asm operand
  or destroy an assembly output. Declared callee-saved clobbers require ABI
  preservation and matching unwind information.
- Handle stack alignment, temporary stack use, red-zone invalidation, flags and
  relevant FP/control state. Stack/frame-pointer restrictions remain
  target/frontend rules; support naked assembly through its separate policy.
- Make asm-goto and Rust label exits real CFG edges. Put result transfers on
  the correct edges and test live values on every successor. A nonreturning
  operation has no invented fallthrough.

Keep the common register path compact. An optional richer assembly register-unit
model can map overlapping ordinary GPR/XMM units onto the existing allocator,
without widening every ordinary record immediately. Wider asm values need real
size/alignment, spill and ABI rules; two unrelated scalar carriers do not supply
a vector-register operand. This does not require an autovectorizer first.

All MIR cleanup, scheduling, frame, branch-relaxation and emission consumers must
recognize the opaque instruction's reads, writes and effects. A raw byte blob
inserted after allocation would evade those correctness checks.

## 4. Choose and qualify the assembler implementation

Prefer a table-driven integrated parser/encoder, emitting into our existing
object/symbol/fixup machinery and leaving ordinary instruction selection fast.
The production encoder remains entirely in cppgm, as explicitly selected by
the user. Extend our existing x86 encoder with reviewed instruction-form tables.
Do not add XED or LLVM as a build, link or runtime dependency. Their encoders,
and GNU as, may be independent private test oracles; oracle availability must
not be required to build or use cppgm. Keep any oracle measurements separate
from evidence for our own encoder's correctness and cost.

Close the remaining assembler work explicitly:
expressions, addressing and prefixes; symbols/aliases/visibility/weakness;
local branches and relaxation; data/alignment/section directives; GOT/PLT/TLS
and supported ELF/Mach-O relocations; macros and conditional/repeated assembly;
and source-file inclusion policy. Preserve required target differences rather
than silently translating unsupported directives.

Integrate [CFI directives](https://sourceware.org/binutils/docs/as/CFI-directives.html)
with frame/unwind state. Test exception unwinding across assembly-defined
functions, and reject incompatible directives. An object that disassembles
correctly can still have broken linkage, stack rules or unwind tables.

## 5. Deliver in independently checked stages

| Stage | Deliverable | Exit evidence |
|---|---|---|
| A. Contract and inventory | Pin source/dialect/ISA profiles; inventory actual asm in Rust core/stdarch and supported C++ headers; select assembler component; specify LowIR payload and architecture amendment | Reviewed coverage/dependency matrix and positive/negative outcome declarations |
| B. First vertical slice | Generic assembly infrastructure sufficient for Rust `_mm_stream_ps`: `movntps [{p}],{a}`, pointer `in(reg)`, real 128-bit `in(xmm_reg)`, `nostack` and `preserves_flags` | Original Rust source through checked body, LowIR, allocation, object and native execution; full bits, alignment, sentinels, ABI and required instruction verified |
| C. Operand/effect closure | GNU alternatives/ties/memory/flags/clobbers; Rust directions/options/const/sym/ABI clobbers; pressure and alias handling | Independent compiler observers, optimizer opponents and invalid-constraint diagnostics at O0/O2/O3 |
| D. Control flow and assembly items | GNU goto, Rust labels, internal labels, global/naked assembly, directives, symbols, unwind and both object formats | Multi-exit, cloned-label, cross-TU link, naked ABI and unwind tests on native Linux/macOS |
| E. ISA and dialect closure | Remaining advertised register classes, prefixes/forms, assembler macros/directives and relocations | Differential encoding/relocation corpus against pinned independent assemblers; coverage matrix has no unqualified supported entries |
| F. Release and cross-frontend qualification | Rust body export/import, backend capability/pin refresh, optional student export and CI | Clean-install native tests using only the new immutable pin; complete optional assignment checks, baseline correctness and performance controls |

Stage B is a first supported subset, not completion of PA36 or Rust PA41.
Use the actual source and helpers in RustGM's
`pa41/tests/spec/asm-frontend/stream-ps.rs` and `asm-vectors/actual-stream.rs`;
do not replace them with a frontend spelling match. A real non-temporal store is
required; an ordinary vector store is not an acceptable implementation. Put any
fence needed by the observer in that observer rather than changing source
semantics. Qualify shared backend output on ELF and Mach-O; separately require
RustGM's initial Linux source/native path.

Rust integration must preserve the operation through instances and the checked
body codec, with source ownership, symbols, constants and operand places intact.
Update the reviewed LowIR contract, tool hashes, capability manifest and
distribution archive together. Do not modify a frozen pin in place or advertise
native support based only on successful parsing or metadata export.

## Assignment ownership and student scope

**cppgm: propose optional PA36, "Inline assembly and shared backend contracts".**
PA29's current GNU-header compatibility remains a bounded prerequisite. PA24
already owns native selection/encoding, PA27 owns hosted object/ABI work and
PA32/33 own optimization. None currently specifies the combination of an
assembler language, constrained allocation and cross-frontend serialization.
Adding it to PA29 would substantially enlarge an existing required milestone;
making it a PA33 optimization would conflate new semantics with code quality.

PA36 can depend on PA35 in the cumulative sequence while identifying the actual
technical prerequisites as PA8, PA24, hosted ABI and PA32/33. Publish optional
cross-references in the owning handouts, especially PA8's LowIR handout, when
the extension exists. Keep the original 35 assignments achievable independently.

Ship reviewed ISA/register/constraint vocabulary and encoding data as starter
material, with attribution and clear maintenance rules. Students should implement
the assembly model, binding/allocation, integration and validation, rather than
transcribe an entire ISA manual. Stage the handout's supported profiles; the
full reference implementation and each advertised student profile need explicit
completion criteria. Required behavior must not prescribe a particular parser,
allocator algorithm or incidental surrounding instruction sequence.

Provide explicit optional commands such as `test-inline-asm` and a PA36 lane;
these names are proposals, not existing targets. Default validation through PA35
must not acquire the extension. Export its optional documentation, scaffolds and
fixtures consistently, and update discovery/placement audits that currently
enumerate PA1 through PA35. Baseline regressions still belong in their earliest
owning assignment; new full-asm fixtures belong to the optional contract.

**RustGM: retain PA41, with the LIB-045 prerequisite during PA32.** Its handout
already assigns general assembly/target closure there and expressly requires
the first core-library source slice earlier. PA25 retains const evaluation,
PA29 checked body transport, PA31 builtin/feature provenance, and PA27/28 place,
safety and memory rules. No additional Rust assignment is needed.

## Validation and performance controls

Tests should prove behavior, effects and ABI, using independent GCC/Clang/rustc
observers and assemblers as appropriate. Cover difficult register pressure,
tied/early/late operands, input evaluation once, output alias order, memory
aliasing and barriers, flag outputs, callee-saved clobbers, calls/stack/red-zone
use, local-label cloning, goto successors, constants/symbol relocations,
negative options/features and unsupported target forms. Include direct LowIR
producers so every test does not depend on C++ parsing.

Check instruction bytes or disassembly where the assembly source itself demands
that instruction, such as `movntps`; combine those checks with execution and ABI
observers. Avoid matching unrelated register choices, spills or instruction
sequences. Assembly-only testing of advanced ISA forms can run without host
support; native tests must detect host capabilities and report explicit skips.

During development run affected tests and small scale/pressure opponents. At
release run the repository's strict report, debug checks, architecture/placement
audits, self-host through PA5, inception and PA35 smoke/release checks, plus the
optional assembly matrix and a fresh student export. RustGM follows its own
resource-controlled focused/release validation policy.

Keep immutable before/after compilers and frozen inputs. Check ordinary public,
self-build and simdjson/index-cpp workloads periodically, using instruction
counts and calibrated paired ABBA runs with output equality. Track no-assembly
allocation/record costs separately from assembly parsing, alternatives, operands
and fixups. No assembler tables, extra body allocation or global rescans should
become routine costs for source that contains no assembly.

Implementation should begin with stage A and the shared LowIR contract, then
stage B's original Rust consumer. The major risk is the combined assembler and
allocation surface, not recognition of the Rust macro or one more mnemonic.

## Implementation checkpoint (2026-11-07)

The `compat/inline-assembly` branch contains the initial shared packet, LowIR
reader/writer/validator, opaque native effect, constrained GPR/XMM binding and
in-tree integer/SSE form tables. The optional PA36 lane currently checks ten
runtime fixtures and six malformed-contract fixtures through the shared Perl
parallel runner. It makes 40 native/LowIR-roundtrip observations plus host
comparisons, including C++ source through `--full-asm`,
128-bit streaming stores, Godot's original CPUID sequence, aliased outputs,
flag outputs, early writes, read/write operands and named matching constraints.
A private corpus compared 6,429 Intel and 6,429 AT&T encodings with LLVM 22;
all matched. These are initial supported forms, not full ISA qualification.
The required strict report passed 8,099/8,099 at this checkpoint.

The encoder has no production XED/LLVM dependency. Rust source lowering and its
qualified backend pin, symbol operands, control flow/global assembly,
directives, wider register classes and broad ISA closure remain open.
See PA36's current-subset declaration before interpreting its passing tests.

### Memory and immediate checkpoint (2026-11-08)

GNU `m`/`o`, `i`/`n`, mixed locations and target integer ranges now use the same
LowIR boundary. Allocation chooses a location and register jointly across one
alternative column. Explicit memory inputs retain their storage addresses;
register alternatives load their values once. Output-only memory has no
invented load. A read/write memory location names one place; distinct input and
output places can choose a register alternative. Binding caches distinguish
invocations, including different constants supplied to one template.

The optional suite has 26 fixtures covering these rules, aggregate and volatile
memory, aliases, effects across optimization/inlining and malformed domains.
All pass at O0/O2/O3 with host and self-built compilers; portable source cases
were also checked with GCC and Clang. The number reader now accepts every
unsigned 64-bit range word while rejecting overflow. The default placement
audit includes the optional lane as well as its numbering checks.

The required strict report remains 8,099/8,099. Debug information, self-host
through PA5, inception object/compiler equality, PA35 smoke and architecture
audits pass. Frozen compile instruction work changes by -0.0024% at O0 and
+0.0130% at O3 against the pre-assembly baseline; output objects are identical.
This is an instruction-work sanity check, not a wall-time measurement.

The initial register subset also passed native Intel macOS CI under LLVM 18 and
21. That run predates this operand extension. Remaining operand work includes
full register-class transfers, commutative swaps, symbols and bit-field output
places. The later control-flow, module assembly, unwind, ISA and Rust release
stages are still required before claiming full assembly support.

### Register, output-place and local-branch checkpoint (2026-11-08)

Scalar floating values can bind GPRs and integer values can bind XMM registers,
with bit-preserving transfers and staged setup that does not assume the emitter's
scratch registers are free. Incoming condition flags survive operand setup.
Bit-field outputs capture their selected places once, preserve neighboring bits
and commit register outputs in operand order. Conditional, comma, assignment
and prefix-increment designators have behavioral coverage. A GNU-native fixture
uses its observer directly where GCC drops conditional bit-field writeback.

The integrated parser resolves named and numeric local labels to typed targets;
each emitted occurrence receives fresh label identities, including GNU unique
labels and optimized clones. Direct jumps and all condition-code branches stay
inside one assembly packet. The optional suite has 35 fixtures, all passing at
O0/O2/O3 with host-built and self-built compilers. The required strict report
passes 8,099/8,099, as do debug info, self-host through PA5, inception equality,
PA35 smoke, architecture/placement audits and the harness checks.

Symbol/relocation operands, outgoing CFG edges, assembly items/unwind state,
wider ISA/register coverage and Rust source/pin qualification remain open.
Godot is paused until the assembly work is finished.

### Symbol and integer-form checkpoint (2026-11-08)

The native boundary now binds actual LowIR symbol IDs and byte addends, including
constant index projections. GNU address constants come from semantic constant
evaluation. Typed source-span bindings retain their identity through the single
assembly parse; native fixups do not recover identities from rendered names.
RIP-relative loads/addresses, GOT references and direct symbol calls have native
observers. Declared stack effects preserve ABI call alignment in frame-free
bodies. The stack check stays behind the optional per-function assembly state.

Commutative pairs can exchange values or explicit storage inputs while retaining
their original types and storage identities. Overlapping pairs are rejected.
The form tables additionally cover unary arithmetic, shifts/rotates, test,
multiply, locked exchange/add and compare/exchange, and repeated string forms.
Prefix validation rejects incompatible or incomplete combinations.

All 50 fixtures pass at O0/O2/O3 with GCC and Clang observers and the self-built
compiler, including contended atomic updates, guarded copies and high-byte
aliases. A private LLVM 22 oracle corpus covers 9,264 Intel and 9,264 AT&T
encodings, with no mismatches after accounting for 64 legal legacy-prefix-order
variants. The required strict report, debug-info, self-host through PA5, inception
equality, PA35 smoke, harness and mandatory audits pass. Frozen instruction work
changes by -0.0016% at O0 and +0.0130% at O3 against the pre-assembly baseline,
with byte-identical output objects. This is not a wall-time measurement.

Outgoing exits, global/naked assembly, directives/unwind state, wider register/ISA
coverage and Rust source/immutable-pin qualification remain open. Godot is
paused. The implementation is still a supported subset, not full assembly.

### Outgoing control-flow checkpoint (2026-11-08)

GNU asm goto now lowers named/numeric labels, read/write outputs, backward
edges and C++ scope-exit cleanup through ordinary LowIR control flow. The shared
packet carries positive constant i32 label tags and one special exit-tag output;
zero means fallthrough. A normal LowIR switch dispatches the result. This makes
SSA, liveness and cleanup edges visible without a language-specific backend
opcode. Other producers can construct the same packet and dispatch.

The backend binds label uses as typed source spans and emits typed exit nodes,
without rendering generated assembly for reparsing. The exit storage is an
explicit MIR frame operand, so frame layout can see and rewrite it. It needs
no spare GPR, including when a packet clobbers every allocatable GPR.

A pressure probe exposed a separate reserved-frame-register bug: overwriting a
compiler frame base corrupted over-aligned locals. Declared writes now preserve
the function's reserved register set around the effect, capture physical outputs
before restoration, and preserve flags. Independent LowIR tests exercise the
reserved register as an output as well as a clobber.

The optional suite now has 67 fixtures, including rejected label/selector
contracts and nonconstant, zero or oversized tags. All pass at O0/O2/O3; the
64-case suite before the final three malformed-tag cases also passed Clang
observers. A direct observer checks C++ cleanup because the current host GCC
skips destruction on the asm-goto scope exit (observed results 111/1, expected
111/11). The required strict report remains 8,099/8,099. Debug info, self-host
through PA5, inception equality, self-built optional checks, PA35 smoke and the
mandatory audits pass. The LowIR handout's exit example was read and compiled.

Global/naked assembly, directives/unwind, wider register/ISA coverage and Rust
source/pin qualification still remain before claiming full assembly support.


## 2026-11-08 checkpoint: opaque raw functions

GNU `asm goto` and the shared exit-tag contract are complete. Raw function
support now adds `assembly_function` definitions with ordinary typed ABI
declarations, constant/symbol arguments and opaque bodies. C++ `naked` basic
statements join their user fragments before the sole instruction parse. The
native emitter adds no frame, setup moves or inferred return. Explicit returns,
push/pop, local calls, indirect transfers and symbol tail jumps use cppgm's
encoder. Typed symbol arguments observe their targets' full ABI in reachability
and interprocedural analysis. Virtual thunks, weak binding and COMDAT behavior
recognize raw definitions.

Validation: 78 optional cases pass at O0/O2/O3 with the host-built and self-built
compilers, including independent observers where Clang cannot provide an oracle
(its naked virtual-thunk code generation crashes, and it inherits naked onto
an explicit specialization differently from GCC). Required strict report is
8099/8099; debug information, self-host through PA5, inception compiler/object
equality, PA35 smoke and the harness pass. The shape validator rejects runtime
inputs, register setup, ordinary bodies and duplicate definitions. The handout,
LowIR contract ledger, vocabulary checker and optional suite were updated.

Private independent encoding coverage is 9357 cases per dialect, with zero
mismatches after accounting for 64 legal legacy-prefix-order variants. Frozen
compile instruction work versus the pre-assembly compiler changes by about +0.0083%
at O0 and +0.016% at O3; output objects are byte-identical. Every observation
and compiler hash is retained under the private assembly experiment directory.
These are instruction-work measurements, not wall-time claims. Fresh export
and native macOS qualification follow this commit.

Remaining full-plan work: ordered module assembly/data and symbol bindings;
directives/macros/repetition and unwind/CFI; wider register/ISA encoding and its
pinned coverage inventory; original Rust source/body transport, backend lock
and clean-install qualification. Raw functions do not establish those broader
capabilities. Godot remains paused until assembly support is finished.


## 2026-11-08 checkpoint: conditional, bit and extension forms

Added conditional moves and byte results, bit scans/counts, byte reversal,
bit tests/updates, double shifts, wide compare/exchange and mixed-width
sign/zero-extension moves. Their widths, prefixes and operand roles are explicit
architectural facts. BSWAP shares the existing native encoder primitive.
The corpus covers 19033 Intel and 19033 AT&T cases against LLVM 22, with zero
mismatches after 112 legal prefix-order variants. Native observers guard optional
CPU instructions before execution and exercise flags, memory boundaries, shifts,
high-byte views and wide atomic success/failure. Invalid width/prefix/REX forms
have normal rejection checks. The subset has 88 optional cases.

All 88 pass at O0/O2/O3 using host-built and self-built compilers, with GCC and
Clang observers. The strict report remains 8099/8099; debug information,
self-host PA5, inception compiler/object equality, PA35 smoke, harness and audits
pass. A larger literal opcode table exposed a false positive in the header-body
heuristic. The audit now excludes literal constant-array initializer rows from
that heuristic, while retaining file/function limits and keeping calls, lambdas
and side effects visible. Focused positive/negative audit controls pass; the
shipped header passes the export-style audit without warnings.

Frozen compile instruction work versus the pre-assembly baseline changes by
+0.00655% at O0 and +0.02492% at O3, with byte-identical output objects. The
immutable compiler, hashes and observations are retained privately. Fresh
student export and CI qualification follow this checkpoint.

Module-ordering evidence: a GCC source probe keeps a later namespace macro
definition after an earlier function using it; Clang's integrated path behaves
differently. Preserve explicit module ordering/environment epochs rather than
hoisting all global assembly into every function's environment. This work,
directives/CFI, wider ISA/register support and Rust qualification remain open.


## 2026-11-08 checkpoint: scalar moves and nonreturning packets

Scalar SSE moves preserve bits, including NaN payloads, and their memory widths.
Instruction form selection distinguishes scalar-double `movsd` from the
zero-operand dword string form; Intel and AT&T string names are qualified.
Ordinary sign-extended 64-bit immediate moves use their shorter encoding,
while explicit `movabs` retains the absolute form. Typed symbol immediates and
addends are accepted by `movabs`. Absolute-text-address fixtures explicitly
request a non-PIE observer executable through a validated link-mode sidecar.
Ordinary fixtures retain the host link defaults.

Nonreturning packets have no output operands and require an immediately
following `unreachable` instruction. Native emission preserves their effects
before divergence. Raw assembly-defined functions remain opaque ABI definitions:
the marker does not infer that an assembly function containing an ABI return
is nonreturning. A native observer checks both behaviors independently.

94 optional fixtures pass at O0/O2/O3 with GCC/Clang observers and the self-built
compiler. Strict report8099/8099, debug checks, selfhost PA5, inception equality,
PA35 smoke, harness, mandatory audits and placement checks pass. The independent
corpus has19394 encodings per dialect, zero mismatches after112 legal prefix
order variants; all207 declared form mnemonics occur in the corpus.

Frozen compile instruction work versus the pre-assembly baseline changes by
+0.01055% at O0 and +0.02993% at O3, with byte-identical output. The O3 measurement
was interrupted and repeated as a fresh pair; both completed measurements and
the interrupted raw logs remain retained. Fresh export and CI follow this
checkpoint. Module assembly, directives/CFI, wider ISA/classes and Rust producer
qualification remain incomplete; Godot is paused.


## 2026-11-08 checkpoint: ordered module code and data

`assembly_global` imports ordered module invocations with constant/symbol inputs.
The optional C++ namespace adapter retains original source fragments. Module
assembly emits directly into the object model: it is not a synthetic ordinary
function. Code/data/BSS, scalar/address data, strings, alignment, section state,
binding/visibility, size/type and aliases have native behavior checks. Literal
names use the target linker namespace; typed pieces retain identity at uses and
definitions. Object-symbol references and nearby weak calls retain interposition,
including binding declarations placed after the code. Attribute declarations
apply to the unit; code, data and section transitions remain ordered.

The existing object compiler/linker also serves module-containing standalone
`lowir2native` programs. The optional Perl lane now tests that path and builds
all three tools it requires. Ordinary MIR function dumps explicitly reject
module items. Instruction syntax is parsed once after typed placeholder
expansion. Static arguments, malformed section state, duplicate definitions,
alias cycles and storage bounds have normal rejection checks. An older PA27
filename detector now defers to explicit assembly-profile sidecars, with a
focused discovery regression check; standard parallel ownership stays unique.

110 optional cases pass at O0/O2/O3 with host-built/self-built compilers and
GCC/Clang observers. Strict report8099/8099, debug checks, selfhost through PA5,
inception compiler/object equality, PA35 smoke and mandatory audits pass.
The independent encoding corpus remains19394 cases per dialect with zero
mismatches after112 legal prefix variants. The frozen final compiler changes
compile instruction work by+0.00763% at O0 and+0.01595% at O3 versus the original
pre-assembly compiler, with byte-identical output objects. Observations and
immutable hashes are retained privately; these are not wall-time claims.

Remaining: macros/repetition/conditional assembly and source environments shared
with inline statements; CFI/unwind; broader expressions, registers and ISA;
capability manifest and original Rust source/body transport/backend-lock
qualification. Student material names the supported subset. Godot remains
paused until assembly closure is complete.

## 2026-11-08 checkpoint: module source expansion

Ordered module invocations now share macro definitions. Expansion retains typed
symbol spans through positional/named arguments, defaults and raw variadic
arguments, including definitions produced by macros. Nested macros, unique
labels, repetition, conditional blocks and bounded recursive expansion have
native behavior fixtures. Numeric conditions use GNU assembler precedence and
truth values. Input, intermediate and emitted source share a bounded byte
budget, alongside work and nesting limits. Instructions are parsed once after
expansion; declaration prepasses see only expanded source.

124 optional cases pass at O0/O2/O3 with GCC/Clang observers and the self-built
compiler. Strict 8099/8099, debug checks, selfhost PA5, inception equality, PA35
smoke, harness and mandatory audits pass. The independent checks include 3009
absolute expressions and six macro/data groups against GNU as. The final frozen
compiler adds about 0.0114% instructions at O0 and 0.0179% at O3 versus the
pre-assembly baseline, with byte-identical output objects. These are compile
instruction observations, not wall-time measurements.

The LowIR documentation audit now requires record spellings in code markup, so
ordinary prose cannot mask a missing documented record. Student material names
module expansion and its limits. Symbol-dependent expressions, sharing macros
with inline statements, CFI/unwind, broader ISA/register coverage and original
Rust integration remain open. Earlier 94-case native Mac checkpoints passed
LLVM18/21; the current module/macro checkpoint needs its own native Mac run.
