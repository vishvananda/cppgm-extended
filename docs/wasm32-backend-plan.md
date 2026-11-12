# A second backend: wasm32 and a reusable machine layer

Status: implementation in progress, 2026-11-12. The working branch is
`feature/wasm32-backend`, rebased onto `origin/main` at `e2831a513`, including
the merged LLVM optimization work. Compiler changes are isolated in
`/mnt/vizier/cppgm-wasm32-backend`; the main checkout is unchanged.

The active student documents are [PA37](../pa37/README.md), its
[design notes](../pa37/design-notes.md), [ABI contract](../pa37/target-abi.md)
and [quality checks](../pa37/quality.md). Earlier handouts introduce target
layout and the shared machine boundary; normal course/export discovery includes
PA37. This plan, the [completion journal](wasm32-completion-status.md) and the
[reference investigation](wasm32-assignment/reference-evidence.md) are
maintainer material and stay out of the student export.

The optional [Wasm/browser inception plan](wasm-browser-inception-plan.md)
records the separate self-host qualification path and executed browser probes.
It does not expand PA37's required closing conditions.

The implementation covers target-aware frontend/LowIR layout, scalar and wide
values, structured/irreducible control, separate objects/archives and static
linking, aggregate/varargs ABI, class/RTTI/runtime boundaries, modern source EH,
WASIp1 commands, same-instance initialization and all 214 LowIR vector
operations. The object vocabulary covers all 236 standard SIMD128 operations.
Shared exception/global-data analyses contain no x86 frame or register facts.
The consumed Wasm MIR retains typed locals, frame objects, effects and source
locations. Its local cleanup and initial published quality bounds have independent
LLVM calibration. Typed globals/constant expressions, reference values,
global COMDATs and object-section/feature validation are implemented. Host-built
and self-built tools pass 95/95 PA37 controls; the strict report passes
10,784/10,784. Pinned LLVM gaps have explicit shipped reference limitations.

This remains an intermediate implementation. Target cost policy, remaining
profile hardening (including imported global/limit boundaries), standard
benchmark publication, broader language/profile cases and final complete
Linux/macOS qualification remain required. Keep the full acceptance checks
below visible rather than narrowing the assignment to the current emitter.
Current evidence and qualification details are in the completion journal.

## Qualification of the first implementation

- The strict report passes 10,664/10,664; PA37 passes 18/18 at O0–O3 with both
  host-built and self-built compiler/backend tools.
- Debug-info checks, the self-host ladder through PA5, byte-identical inception,
  PA36 correctness smoke, architecture/format/file/placement audits and the
  complete harness suite pass.
- A fresh student export regenerates and verifies the reference outputs.
  Its 269-document inventory, standalone scaffolds and reference-bundle
  installation through all 43 wrappers pass. No implementation plan is shipped.
- LLVM 22's wasm32 compiler/linker agrees on 11 export invocations across seven
  header-free source fixtures. Full object/runtime interoperability is still
  a later qualification step.
- Frozen native compilation against the identical LLVM base emits identical
  object bytes at O0/O3. The first core checkpoint adds 0.20%/0.63% instructions
  and leaves peak memory essentially unchanged. A/A and ABBA observations are
  retained outside Git; pinned O0 and ordinary O3 paired wall estimates are
  +0.86%/+0.84%, with A/A estimates of -0.73%/-0.85%. These small timing changes
  are not a reliable regression estimate on this host.

Local evidence, frozen binaries/inputs and all observations are under
`/mnt/vizier/cppgm-wasm32-evidence`. The next ABI step must audit remaining
native-width vtable/member-pointer geometry, binary128 constant operations,
symbol aliases/equivalent entries and every implicit ABI classification before
claiming portable class or wide-value support. PA37's full required behavior
and quality gates remain the completion bar.

## Aggregate ABI implementation

Aggregate parameters/results now carry typed `shape` / `result_shape` facts:
empty, mixed, or a counted scalar composition, independent of size, alignment
and AMD64 eightbyte classes. The Wasm ABI selects ignored values, scalar
coercions or indirect copies. Large trivial source results remain object
values in LowIR and acquire their hidden destination in the backend. The
selected function maps LowIR values to ABI locals explicitly, keeps object SSA
snapshots in frame storage, copies indirect by-value arguments, and aligns
frames beyond 16 bytes when required.

New cases cover nested/empty/union/bit-field/over-aligned/large records, indirect
calls and snapshot preservation. A Perl binary-section reader checks exported
public function types without observing instruction selection. LLVM 22-built
callers agree with cppgm callees at O0–O3 using separate stack regions in a
shared imported memory; the interop probe is retained outside Git under
`aggregate-interop` in the evidence directory.

That interop exposed a native-only optimizer assumption: replacing any small
integer-class aggregate boundary with i64/i128 preserves AMD64 registers, but
changes Wasm's typed public ABI. The transformation now requires a typed ABI
capability; it leaves Wasm boundaries intact. Native scalar rewrites clear the
object-only shape facts. Other LowIR optimizations remain shared.

Qualification: the rebased strict report passes 10,674/10,674; all 23 Wasm
fixtures pass with host-built and self-built tools. Debug-info, PA36 smoke,
byte-identical inception, self-host through PA5, architecture/format/file and
placement audits, harness tests and a regenerated student export pass. The
exported reference bundle and all 43 wrappers install successfully.

Matched immutable binaries measured against the previous stage on the
`190c9a162` snapshot add 0.0026% O0 / 0.0020% O3 compiler instructions and emit
identical native objects. The subsequent LLVM rebase includes that branch's
own native fix, so those measurements must not be presented as its combined
cost. Wall-time A/A calibration drifted by roughly 9–10%; its paired timings
are retained but do not support a timing regression conclusion.

This still leaves object linking, the reverse mixed-producer call direction,
varargs, wide values, full portable runtime/EH and SIMD qualification. The
summary shape intentionally does not claim to encode arbitrary member offsets;
a future ABI that needs those must carry the additional typed projections.

## Next object implementation boundary

The next step must emit real Wasm linking-v2 objects, not embedded LowIR.
Retain typed symbol/type/global/table relocation identities in the selected
MIR before encoding. Function calls, function-pointer constants, global
addresses, call-indirect type indices and the stack pointer need distinct
relocation kinds; matching numeric immediates after emission is insufficient.
Use the existing shared global-data fixups for data relocations. Resolve
symbols/COMDATs and deduplicate function types before patching object fields.

A local LLVM 22 object probe confirms imports of `env.__linear_memory`,
`env.__stack_pointer`, external functions and the indirect function table;
its code uses FUNCTION_INDEX_LEB, GLOBAL_INDEX_LEB, MEMORY_ADDR_LEB/SLEB and
TABLE_INDEX_SLEB relocations. The ELF/Mach-O source naming already retains
correct Itanium object names in Wasm LowIR, so do not add a second mangler.
LLD also resolves `__stack_low`; probe its layout with live data and selected
stack options before adopting it for object-mode bounds checks. Probe source,
object, decoded sections and relocation inventory stay in the external
`object-profile` evidence directory.

## Placement and scope

Append the assignment after PA36 initially. PA8's typed LowIR, PA24's backend,
PA25–32's ABI/runtime/compatibility work, and PA33–34's optimization work are
prerequisites. PA37 is the next free number in this snapshot; use the next
free number if the separately proposed LLVM round-trip lesson lands first.
Do not renumber the existing course in this change. Later placement after
machine optimization can be considered separately from self-hosting.

Completion means compiling and linking the portable portion of the existing
language profile to usable Wasm modules, including optimized code, separate
compilation, classes, virtual calls, RTTI, exceptions and destructors. A
scalar-only emitter is an intermediate milestone. Reimplementing the front
end or the LowIR optimizer is outside the assignment.

Use two explicit targets: `wasm32-unknown-unknown` for freestanding modules
and `wasm32-wasip1` for WASI Preview 1 commands. Both use the same wasm32 C/C++
ABI and linear-memory object model. The first profile is single-threaded,
little-endian, with one 32-bit memory and one indirect-function table. Its
feature allowlist includes mutable globals, sign extension, nontrapping
float-to-int conversion, bulk memory, multi-value, reference types, SIMD128
and modern exception handling. Allowing a feature does not require choosing
its instructions when an equivalent lowering meets the quality bounds.

Exclude Emscripten glue, wasm64, components/WASIp2, dynamic linking, threads
and shared memory from this assignment. Diagnose x86 assembly, CPUID, MXCSR,
x86 hardware intrinsics without a semantic equivalent, target-specific
stack/frame introspection, TLS and atomic operations in this first Wasm
profile. A backend must never silently ignore an unsupported operation or
emit a host-native fallback. Ordinary portable vector operations are required;
x86 header compatibility is not.

Document these target limitations in `LANGUAGE_PROFILE.md`, alongside the
portable language requirements and native-only extensions. Existing native
coverage stays required. A thread-capable profile can later add Wasm
atomics/TLS without changing the shared MIR around x86 register names.

## What the initial architecture required

| Current representation | Consequence for the split |
| --- | --- |
| `dev/src/native/target.h`: `Linux` or `MacOS`; both imply x86-64 | Separate architecture, environment, object format, ABI and data layout. |
| `types/fundamental_type.h`: sizes documented as System V AMD64 | Select target facts before preprocessing, literal typing, constant evaluation and class layout. |
| `lowir_model.h`: pointer alignment defaults to eight; `opt/scalar_fold.cpp::width_of` gives pointers 64 bits | Make pointer storage, arithmetic and folding use the program's layout. Audit every size/alignment helper. |
| `ir_symbol_model.h`: two-eightbyte `ObjectClassBits` | Preserve typed aggregate ABI shape; isolate x86 classification from shared signatures. |
| `native/mir/mir.h`: physical GPR/XMM/x87 registers, RBP offsets, two return parts, 32-bit register masks | Extract reusable machine concepts; do not relabel this whole model as generic. |
| `native/mir/operand.h`: base + index × scale + displacement in every operand | Keep x86 address forms in its target payload. Wasm has a linear address plus a memory offset. |
| `native/backend.cpp`: selection, native runtime/linking and x86 encoding are joined | Dispatch once to a target backend before selection; retain a shared driver and typed program. |

Wasm32 changes source semantics: `long` and pointers are 32 bits, while
`long double` is IEEE binary128. Native x87 `f80` is not its representation.
The existing `lower/soft_float_lowering.cpp` already handles binary128
constants and helper operations; reuse that work after removing its x86 ABI
assumptions. Mapping source `long double` directly to native `f80` is incorrect.
These data-model differences are specified by the
[WebAssembly Basic C ABI](https://github.com/WebAssembly/tool-conventions/blob/main/BasicCABI.md).

## Shared pipeline and MIR boundary

```text
source + immutable target description
  -> existing parser and semantic analysis, using target layout/ABI facts
  -> typed LowIR carrying its target/layout and source ABI shape
  -> existing LowIR optimizer, with target legality and cost policies
  -> shared per-function machine services
       -> x86 selected MIR -> register allocation/layout -> x86 bytes
       -> Wasm CFG MIR -> legalization/structuring/stackification -> Wasm bytes
       -> future ARM selected MIR -> allocation/layout -> ARM bytes
  -> target object writer and linker, with shared symbol-resolution rules
```

Refactor the lower machine representations as part of the work. Share a small
machine core and analyses, with typed target payloads and independently
verifiable phase boundaries. LowIR already supplies generic arithmetic and
memory operations: do not insert a second complete optimizing SSA IR merely
to imitate LLVM. LLVM's separation of legalization from selection is a useful
[design reference](https://llvm.org/docs/GlobalISel/Pipeline.html), not a
dependency or a requirement to reproduce its pass pipeline.

The common machine core should describe:

- Stable function, block, instruction, value, frame-object and symbol IDs;
  compact contiguous storage and reusable per-function scratch.
- Typed value and operand ranges, explicit successors, phi/parallel-copy edge
  moves, debug locations, branch frequencies and exceptional edges. Do not
  impose the current three-operand limit on calls or multi-result operations.
- Explicit and implicit uses, definitions, clobbers, tied operands and
  early-clobber constraints; memory effects, volatility and potential traps.
  Flags are target resources with dependencies, not an assumed universal ALU
  result. Wasm comparisons produce values; x86 and ARM can use condition flags.
- Abstract frame objects with size, alignment and lifetime. Target frame
  lowering chooses concrete bases/offsets, argument areas and saved registers.
- ABI value-piece ranges and calling effects, without two-part, eightbyte,
  GPR/XMM or RBP conventions built into the common record. Keep ABI shape
  separate from the physical location assigned to each piece.

Share CFG, dominance/loop utilities, edge-copy resolution, use/liveness
analysis, reusable frame-object packing and symbol/debug identities where
their contracts actually match. Run an analysis only where a backend needs
it. Preserve target-specific instruction semantics through descriptor tables
or typed views; do not inspect mnemonic strings or add architecture switches
through every pass.

For register targets, physical-register identity and alias units come from a
target register description. Sets must support more than 32 registers, register
banks, subregister overlap, reserved registers and per-call clobbers. The
implementation may specialize the existing x86 set to one word; genericity
must not force a heap bitset for each instruction. Wasm uses typed locals and
an operand stack and does not run a hardware register allocator.

Use target-specific opcodes and addressing payloads behind the shared
instruction/operand iteration interface. Do not build one growing record
containing every target's addressing, EH and encoding fields. Keep uncommon
assembly, unwind and other expensive facts in optional side tables. Common
views must refer to the actual instructions that emission consumes, not a
second copied MIR maintained for dumps or generic passes.

Dispatch at the function/backend boundary. Templates, static tables or a
small backend interface are possible; avoid a virtual call or target lookup
for every operand on the existing native hot path. Keep the production path
in process and encode bytes directly. Textual MIR/WAT remains a diagnostic
view; LLVM, an external assembler and WAT parsers are testing tools only.

Proposed ownership, with exact filenames settled in the refactor:

| Area | Responsibility |
| --- | --- |
| `dev/src/target/` | Architecture/environment identity, immutable data layout, feature sets, typed ABI/capability descriptions. No encoder dependencies. |
| `dev/src/machine/` | Shared per-function identities/views, effects, CFG/edge copies, frame objects and reusable analyses. No x86 opcode or register includes. |
| Existing `dev/src/native/` | Adapt the existing x86 selector/MIR/emitter through the common interfaces; retain its target-specific payloads and efficient representation. |
| New `dev/src/wasm/` | Wasm legalization, typed locals, structured control, stackification, binary/object writer, linker and runtime adapters. |
| `dev/src/driver/` and tool entry points | Parse target/sysroot options once, select shared target facts and dispatch to the chosen backend. |

First expose shared views over the current native records and move only facts
whose semantics are common. Replace sentinel register/frame values with typed
locations at the boundary before changing instruction storage. Then extract
algorithms as a second backend actually uses them. This avoids an all-at-once
rewrite and keeps concurrent LLVM optimizer changes easy to integrate.
Preserve legacy x86 MIR dump spellings through a target formatter; add a
separately versioned Wasm dump vocabulary. An ARM backend should later add its
own legality, ABI/frame, instruction/register and object rules through these
interfaces without modifying LowIR algorithms or the x86 encoder.

### ARM as a concrete design check

Review the common interfaces against AArch64 before freezing the scaffold.
Do not implement an ARM encoder in this PA. Include representation tests for:

| Requirement | Design pressure |
| --- | --- |
| Distinct integer/vector banks, overlapping 32/64-bit register views | Target register units and operand constraints, rather than x86 masks. |
| Three-address operations and variable instruction arity | No two-address or three-operand storage assumption. |
| Homogeneous floating aggregates and an indirect result location | ABI classification and locations cannot be a pair of SysV eightbytes. |
| Different immediate/addressing ranges and instruction sizes | Target legality, costs, branch relaxation and relocation policies. |
| Separate SP/frame/link-register rules and unwind conventions | Abstract frame objects; target prologue, epilogue and unwind lowering. |
| AArch64 Linux and Darwin ABI differences | Architecture alone does not determine OS ABI, layout or runtime. |

The [AAPCS64 reference](https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst)
provides the ABI review cases. Keep vector shape extensible instead of making
128 bits a universal MIR width; scalable-vector code generation and big-endian
support remain future work. An architecture descriptor must not claim either
is implemented merely because it has room to describe it.

## Target facts and LowIR contract

Introduce an immutable target description shared by preprocessing, semantic
analysis, lowering, LowIR validation/optimization and backend selection. Keep
data layout, language type identity, calling ABI, runtime capabilities and
costs distinct. `size_t` and `ptrdiff_t` must have the target's canonical type,
not just an integer of the right width. Target-specific builtin macros,
include search, numeric limits and `long double` literals must agree.

Preserve the existing native CLI aliases and default behavior. Add explicit
Wasm target names to the existing tools; no tenth tool is necessary.
Cross-target source compilation must use target headers/sysroot, never the
host's standard library. Runtime paths and CPU features are not inferred from
the compiler build machine when a target is specified.
Caches must be scoped to a target compilation context or include the relevant
layout/ABI identity in their key; changing targets in one batch process cannot
reuse a class layout, constant, builtin declaration or helper signature from
the preceding request.

Extend the PA8 LowIR contract and its vocabulary/checkers with:

- A versioned target/layout profile in program metadata. Legacy files retain
  their current x86-64 layout; never reinterpret them as ILP32 because a
  command-line option was supplied. Reject layout/target conflicts and mixed
  modules before optimization.
- Pointer-sized memory, index, conversion, literal and relocation rules.
  Distinguish function-table references from memory addresses at the machine
  boundary even though both occupy 32 bits in the C ABI.
- Typed aggregate/signature facts sufficient for target ABI classification:
  field offsets/kinds, nested singleton and alignment information, and
  nontrivial C++ calling requirements. Identically sized objects need not have
  the same ABI. Preserve these through text/object round trips and standalone
  frontends; never reconstruct them from mangled names or source spelling.
- Explicit runtime imports/exports and compatible target features. Calls must
  retain complete signatures. An unresolved symbol is a link error unless an
  import is deliberately declared with module, name and type.
- A capability/operation table: portable operations required on Wasm,
  operations legalized through helpers, and architecture-specific operations
  diagnosed as unsupported. Preserve C++ null/UB facts without equating Wasm
  traps with C++ exceptions or freely speculating faulting memory accesses.

Retain existing x86 ABI metadata for legacy inputs. New portable signature
shape and target classification must agree with it on x86. Choose the precise
versioned spelling and compatibility rules in the contract change, before
adding new fixtures. The draft handout intentionally does not invent a text
grammar that the reader/writer cannot yet implement.

Share one optimization pipeline. Audit pointer widths, endian-sensitive
rewrites, load widening, overflow facts, vector legality, builtin library
substitutions and target cost assumptions. In the LLVM base specifically:

- Reuse field extraction/narrowing, memory forwarding, loop simplification,
  phi equivalence and integer transformations after checking their semantic
  preconditions against the layout and trap contract.
- `opt/address_sinking.cpp` currently favors x86 scaled memory operands and
  displacement limits. Make its profitability a target policy; cloning
  addresses can add Wasm work because Wasm loads do not have x86 scaled-index
  addressing. Do not copy the x86 constants into a Wasm tuning file.
- Keep short-loop unrolling, inlining and switch decisions bounded, but use
  Wasm branch/local/code-size costs. Large repeated dispatch loops must not
  replace ordinary structured loops. GNU `sincos` is not universally available.
- Treat null-edge deductions as source/LowIR facts, not a promise that the
  runtime traps on address zero. Validate load/store motion separately.

## Wasm selection, objects and runtime

Build typed locals for SSA values, resolve phi copies on edges, then form
structured `block`/`loop`/`if`/branch regions. Support irreducible CFGs using a
bounded transformation or a localized dispatcher fallback; do not use a
whole-function dispatcher for every reducible function. Stackification should
keep short single-use expression chains on the operand stack, with locals for
shared values. Address-taken objects use a 16-byte-aligned linear-memory stack;
ordinary temporaries should not all become linear-memory loads/stores.
Define data, stack and heap regions, initial/maximum memory and `memory.grow`
failure behavior. Check address/section arithmetic for overflow and avoid
stack/heap collision; growing memory must preserve offsets used by C++
objects. These limits are target runtime facts, not host allocation sizes.

Legalize i1/narrow values, i128, soft floats, aggregate copies, variadics and
portable vector operations. Preserve division, shift, NaN, signed-zero,
conversion and alignment semantics. Signed remainder must account for the
Wasm instruction's defined minimum/-1 behavior. A literal native instruction
substitution is not a semantic specification.

Write core modules directly: section lengths, LEB128 integers, type/index
spaces, code bodies, data/element segments and feature metadata. Validate
operand/control stacks and all indices before writing successful output.
Use typed indices so a symbol/function/table/memory index cannot be substituted
for another by accident. Produce deterministic output.

Implement relocatable `.o` files using the
[Wasm linking conventions](https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md),
then final-module linking. Reuse resolution, ODR/weak/COMDAT selection and
reachability rules where possible, not ELF relocation encoding. Handle type,
function, table, global, memory-address and exception-tag relocations needed by
the pinned reference/runtime corpus, including padded LEB fields. Reserve null
function-table index zero. Check signatures and incompatible features rather
than making all undefined symbols imports.

The final compiler owns its Wasm linker for the documented static-object
profile; `wasm-ld` is a development/reference oracle. Both mixed directions are
required: cppgm objects linked by LLD, and LLVM objects linked by cppgm. Audit
the actual runtime archive's object/relocation closure before freezing that
profile. Archive extraction and dead stripping must avoid retaining an entire
runtime because one helper is referenced. Reject unsupported dynamic-link
metadata explicitly.

Supply pinned runtime support sources/archives for allocation, memory/libm,
compiler-rt wide arithmetic/soft-float helpers and C++ ABI services. Students
implement calls and ABI lowering, not a new quad-precision math library.
Preserve attribution; runtime libraries are not code-generation delegates.
Use modern Wasm EH for cleanup, matching catches, rethrow and cross-object
exceptions. Preserve foreign exceptions through cleanup and rethrow; a Wasm
trap is not caught by a C++ catch. Do not serialize x86 handler records,
DWARF stack-unwind tables or saved hardware registers into the Wasm runtime.

WASIp1 startup uses the pinned CRT contract: `_start`, arguments, supported
I/O and exit status. Constructors run once and normal termination runs
destructors/`atexit` once; trap/abrupt-exit behavior follows the published runtime
contract. A core start section must not invoke `main` a second time. Define a
separate reactor initialization contract and test repeated exported calls in
one instance. Retain source/name locations; full debugger integration belongs
in the existing debug-info lane unless the assignment explicitly adds it.

## Implementation sequence and acceptance

| Step | Concrete work | Exit evidence |
| --- | --- | --- |
| 0. Freeze the target profile | Pin LLVM/LLD, Wasmtime Cranelift, validator, WASI sysroot/runtime and specifications with versions/hashes. Probe modern EH, quad helpers and both ABI directions. Record supported relocation closure. | Reproducible reference modules run on Linux and macOS. No unresolved prerequisite hidden behind a skip. |
| 1. Describe targets | Add typed target/layout/ABI/capability facts; preserve native aliases and defaults. Add layout context to frontend constants, semantic types and caches. | Native output identity for refactor-only cases; independent wasm32 layout/literal tests, including alternating native/Wasm batch requests. |
| 2. Extract machine services | Introduce common IDs, CFG/effects/frame/ABI ranges and target descriptions; adapt x86 views without moving/rewriting every emitter at once. | Existing MIR/ELF/Mach-O behavior and performance preserved; ARM representation cases fit without x86 sentinel values. |
| 3. Extend the typed boundary | Update LowIR vocabulary, reader/writer, validator, ABI shape, target-aware optimizer and driver round trips. | Native legacy compatibility; incompatible layouts rejected; source and serialized Wasm LowIR produce equivalent programs. |
| 4. First complete scalar path | Direct Wasm encoding, memory/globals, calls, structured and irreducible CFGs, typed locals and linear frames. | O0 scalar/CFG/module validation suites; reference-independent self-checking execution and malformed-input tests. This is not assignment completion. |
| 5. ABI and static linking | Aggregates, varargs, indirect calls, wide values, relocatable objects, archives, COMDAT and dead stripping. | Bidirectional LLVM interop; multi-TU behavior, signatures and linker failures. |
| 6. Portable C++ runtime | WASIp1 command/reactor runtime, startup/finalization, allocation, virtual calls, RTTI, modern EH and soft floats. | Nested cleanup/rethrow, cross-compiler throws/catches, virtual/RTTI interop and wide arithmetic execute correctly. |
| 7. Optimized Wasm | Shared O1–O3 passes, target policies, SIMD128 legalization, local/stack cleanup and bounded code-size/runtime improvements. | Correctness at O0–O3, invariant/quality bounds, measured LLVM comparison and unchanged native regression gates. |
| 8. Course and CI | Land numbered PA, wrappers, Perl harness, scaffold vocabulary, offline references, export updates and standard benchmark entry. | Clean student export, full required report on both hosts, architecture audits and published reproducible benchmark details. |

Steps 1–3 are independent refactor commits with frozen x86 A/B binaries and
outputs. Preserve the LLVM branch's optimizations and narrow-call/return
contracts; do not solve genericity by restoring redundant extension work.
Add reduced tests in the earliest owning PA: target-bound LowIR in PA8,
backend-neutral machine contracts in PA24/34, and Wasm-only behavior in the new
PA. Update the owning handouts when their student requirements change.

After compiler changes run the affected suite, strict full report,
debug-info checks, self-host through PA5, PA36 smoke and the repository's full
architecture/placement/format audits. Run inception for shared code-generation
or self-host behavior changes and PA36 release gates before final qualification.
Measure O0/O3 compiler instructions, memory and paired timing during the
refactor. A new abstraction that increases work needs evidence and revision;
there is no reason for native compilation to pay for unused Wasm facilities.

## Tests, quality and benchmarking

Use Perl drivers with the existing `CppgmBatchWorker.pm`/standard parallel test
runner. Reuse fixture sidecars and failure reporting; no independent serial
runner or Python/JavaScript student-test framework. A pinned runtime/validator
may be a test dependency. A small supplied executor can invoke exports in one
instance when a CLI invocation is insufficient; Perl owns scheduling/checks.
Give every child a timeout and bounded memory/output. Missing required tools
fail prerequisite checks, rather than turning a required family into a skip.

| Numbered family | Main coverage |
| --- | --- |
| 100 target/module | CLI, layout metadata, validation, diagnostics, bad/truncated binaries and feature conflicts. |
| 200 values/memory | Narrow/wide integers, binary128, literals, conversions, object/bit-field layout, pointers, alignment, volatile memory and bounds. |
| 300 control/calls | Cyclic phi copies, critical edges, loops/switches, irreducible flow, recursion, direct/indirect calls and stack restoration. |
| 400 objects/ABI | C/aggregate/varargs ABI, relocations/archives, mixed producers, weak/COMDAT, imports/exports, undefined/mismatched symbols. |
| 500 C++/runtime | Initialization, allocation, classes/virtual calls, RTTI, local statics, exceptions, destructors and command/reactor behavior. |
| 600 optimization | O1–O3 scalar/vector correctness, bounded locals/frame/code growth, loop structure and runtime kernels. |

The same required fixtures run at relevant optimization levels and through
source, text-LowIR, direct-module and separate-object paths. Use cases with
defined target-independent results or explicit per-target results; an LP64
host executable cannot oracle ILP32 `sizeof` behavior. Compare ABI both ways
using the same pinned target flags/runtime and use scalar/self-checking
oracles for vectors. Validate with an independent Wasm validator and execute
with Wasmtime Cranelift; use LLVM output for diagnosis and differential
testing, not exact instruction matching. Property checks must also accept
different valid block nesting, local indices and ABI-conforming encodings.

Add small seeded LowIR/CFG and ABI generators in Perl, covering edge values
and unsupported/malformed metadata. LLVM must not be the sole correctness
oracle. Structural quality checks use semantic invariants and published
envelopes: no linear frame for a pure scalar function, bounded locals and code
growth, effective constant/unreachable cleanup, and no dispatcher for a simple
reducible loop. Publish every bound in an expectation sidecar and calibrate it
against multiple sensible implementations before making it required.

Add Wasm workloads in the standard benchmark format, with pinned module
runtime settings, the same runtime for cppgm/Clang and higher-is-better ratios.
Report compile time, final module code/data size, instantiation/startup and
steady-state runtime separately. Remove VM compilation and input setup from
the steady-state interval. Use immutable modules, output equality, A/A
calibration and paired ABBA blocks. Retain observations in CI artifacts, only
compact reports/badges in Git. Host Cachegrind instructions are runtime/JIT
measurements, not a count of Wasm instructions. Begin timing as advisory; set
course quality limits from repeatable independent workloads, not an invented
universal ratio to LLVM.

For timing, use the pinned engine's C API in a small supplied host executor.
Compile the module once, then measure instantiation/startup, input preparation
and warmed export execution in separate intervals. Do not estimate startup by
subtracting guest time from a CLI process's elapsed time. A Perl benchmark driver
owns deadlines, seeded inputs, A/A calibration and ABBA scheduling; the executor
only loads, invokes and measures. Use the catalog and shared normalized renderer,
with explicit byte-valued size metrics rather than putting bytes in a seconds
field. Record the selected GCC baseline as not applicable to this Wasm target;
do not invent a native/Wasm speedup. Keep both scalar and SIMD128 configurations
visible and retain the exact engine configuration, host executor and module hashes.

## Course integration and export checklist

- Update `spec.md` with supported targets and the common machine boundary;
  update PA8's LowIR contract and the appropriate PA24/34 vocabulary guidance.
  Keep prior x86 requirements valid rather than retroactively asking every
  early assignment to implement Wasm.
- Add the final `paN/README.md`, `design-notes.md`, target/ABI contract,
  Makefile/wrappers and conventionally numbered fixtures. Incorporate this
  draft into those locations only when required behavior is implemented.
- Supply small vocabulary/scaffold headers for target/layout, common machine
  facts and Wasm operation/index spellings. Keep containers, algorithms and
  implementations as student work. Extend `dev_support_files` in
  `scripts/export_student_repo.sh`, source sets and scaffold audits together.
- Update `ROADMAP.md`, course counts, `LANGUAGE_PROFILE.md`, root/student
  README and `TESTING_AND_REFERENCES.md`; add shipped documents to
  `scripts/student_export_documents.txt` and harness scripts to the export's
  shared-script list. The exporter discovers only `pa*/Makefile` directories;
  a draft in `docs/` intentionally does not create a phantom assignment.
- Retain licensed reference excerpts/snapshots under `doc/` with provenance,
  versions/hashes and `NOTICE` updates. Keep this plan, research probes,
  benchmark logs and implementation journals out of the student export.
- Include required Wasm correctness in normal cumulative reports on Linux
  and macOS with pinned prerequisites. Separate heavy performance campaigns
  from correctness; share existing parallel limits and publication machinery.
- Run `make test-harness`, placement/scaffold/LowIR audits, and the student
  export's own validation, including clean discovery and failure paths. Review
  exported handouts and design notes for achievable student requirements and
  absence of solution-specific implementation details.

The closing target is one usable, qualified Wasm backend and assignment, with
shared frontend/LowIR optimizations and a demonstrably extensible machine
boundary. ARM code generation, full libc/STL portability and running cppgm
itself inside Wasm are follow-on projects, not implicit conditions hidden in
this lesson.
