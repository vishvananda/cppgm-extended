## CPPGM Programming Assignment 37 — WebAssembly backend

### Overview

Add a wasm32 backend to `cppgm++` and `lowir2native`. Compile the portable
course language profile to WebAssembly while preserving the existing native
targets. Reuse your frontend, typed LowIR and LowIR optimizer. Refactor your
machine representations where needed so shared analyses do not assume x86
registers, instruction forms or calling conventions.

Complete PA34 and the earlier cumulative assignments first. Continue working
in `dev/` and shared modules under `dev/src/`.
[Design notes](design-notes.md) suggest an approach. The
[target and ABI contract](target-abi.md) defines the shared boundary.

### Targets and required behavior

Support `wasm32-unknown-unknown` for freestanding core modules and
`wasm32-wasip1` for WASI Preview 1 commands. Preserve the existing targets,
default target and optimization levels. Support these forms:

```sh
cppgm++ --target=wasm32-wasip1 -O2 source.cpp -o program.wasm
cppgm++ --target=wasm32-wasip1 -O2 -c source.cpp -o source.o
cppgm++ --target=wasm32-wasip1 source.o other.o -o program.wasm
cppgm++ --target=wasm32-wasip1 --emit-lowir source.cpp -o source.lowir
lowir2native --target=wasm32-wasip1 -O2 -o program.wasm source.lowir
lowir2native --target=wasm32-wasip1 -O2 \
  --dump-machine-ir program.mir -o program.wasm source.lowir
```

Wasm invocations of `lowir2native` may also take relocatable object and
archive inputs, including the supplied arithmetic/runtime libraries.

Freestanding modules use declared imports and exports rather than a WASI
`main`. Follow the supplied target contract for entry points, exported names,
runtime linkage and initialization. Run freestanding dynamic initializers once
at module instantiation; the core start function must not also call `main`.
An unresolved symbol is an error unless
explicitly declared as an import with its module, name and signature.

Produce valid deterministic module bytes and LLVM-compatible relocatable
Wasm objects directly. The compiler must perform its own lowering, encoding
and static linking. LLVM, LLD and external assemblers are reference tools;
the submitted compiler must not invoke them to implement code generation.
Supplied runtime libraries may provide arithmetic and language-runtime
services, as native libraries do in the earlier assignments.

Use `--sysroot <directory>` to select the supplied target headers and libraries,
and `-resource-dir <directory>` for its builtin headers and arithmetic runtime.
For example, after installing the pinned SDK, pass its `share/wasi-sysroot` and
`lib/clang/23` directories respectively. Apply the same target configuration to
source compilation and `lowir2native` links. A WASIp1 link with a sysroot uses
command startup by default; without a sysroot, supply an explicit `_start` or
disable startup. Support `-nostartfiles`, `-nodefaultlibs` and
`-nostdlib` to select startup and libraries explicitly; these flags do not
remove header search paths.

Implement the following at `-O0`, `-O1`, `-O2` and `-O3`:

- Scalar arithmetic and conversions, narrow integer behavior, pointers,
  aggregates, globals, volatile memory, alignment and address-taken locals.
  Preserve wide integer and floating types through valid legalization/helper
  calls where Wasm has no corresponding value type.
- Complete control flow, including phi transfers, switches, nested loops and
  irreducible graphs accepted by LowIR. Preserve every defined exit path.
- Direct and indirect calls, recursion, variadic functions, aggregate argument
  and result passing, and separately compiled objects.
- The portable class/template behavior already required by the course,
  including virtual dispatch, RTTI, local statics, allocation, initialization
  and destruction. Support C++ exceptions, cleanup, matching catches and
  rethrow using modern Wasm exception handling.
- Portable vector operations through SIMD128 or equivalent correct lowering,
  with the published quality bounds at optimizing levels.
- WASIp1 command startup, arguments, the supported I/O services and exit status;
  freestanding initialization and repeated calls within one module instance.

The first target profile is single-threaded and little-endian, with one
32-bit linear memory and one indirect-function table. Its allowed features
are mutable globals, sign extension, nontrapping float-to-int conversion,
bulk memory, extended constant expressions, multi-value, reference types,
SIMD128 and modern exception
handling. Match the supplied profile and feature metadata. The public C ABI
retains its specified return/argument convention even when multi-value is
available internally. Optimization levels must not change public function
parameter/result types, even when a native register convention permits a
representation change.

Threads/shared memory, atomics, TLS, Emscripten glue, components, dynamic
linking and wasm64 are outside this profile. Diagnose these and unsupported
architecture-specific operations, including x86 assembly, CPUID, MXCSR and
hardware frame introspection. Ordinary portable operations must not fail
merely because the native backend implemented them with an x86 instruction.

### Target layout and ABI

Select target facts before preprocessing and semantic analysis. Wasm32 uses
32-bit `int`, `long` and pointers, 64-bit `long long`, and 16-byte binary128
`long double`. Match the supplied ABI for type identity, signedness, layout,
alignment, literals, constant evaluation and calling conventions. Target
headers and builtin macros must describe the target rather than the host.

Carry target/layout metadata and sufficient typed ABI facts in LowIR. Reject
incompatible inputs; do not reinterpret existing x86-layout LowIR as wasm32.
Source compilation and compilation through serialized LowIR must agree.
Function pointers use table references, distinct from linear-memory
addresses. Preserve null values, indirect-call signatures, aggregate copies,
hidden result parameters and variadic argument layout. Preserve the passed
scalar types and aggregate shapes on unnamed arguments and `va_arg` so their
layout survives constant propagation and call cloning. Empty objects, singleton
records and other aggregates have different ABI forms; consume LowIR’s
[`shape` facts](../pa8/lowir.md#aggregate-layout-facts) together with their size
and alignment. An indirect by-value argument needs its own copy, and an object
SSA value must retain its snapshot when the original storage changes.

Interoperate with the pinned LLVM wasm32 toolchain in both directions. A
cppgm-generated object must link with its objects/runtime through LLD, and
your linker must accept the documented object profile produced by LLVM.
Preserve weak/COMDAT definitions, data and function addresses, initializers,
exception tags and supported relocations. Reject mismatched signatures and
features instead of emitting a module that fails only when executed.
Preserve Wasm globals, constant-expression dependencies and typed reference
values. Validate element declarations and all supported index spaces. Where
the pinned linker lacks a required case, the
[reference notes](../doc/wasm/README.md) identify its independent checks.

### Machine representation and optimization

Keep a typed machine representation that emission actually consumes. It must
describe values, control-flow edges, calls, effects, frame objects and debug
locations accurately. Common analyses must not require x86 register names,
eightbyte ABI classes, fixed two-address operations or one physical frame
register. Target-specific instruction and operand forms may remain separate.

Wasm has typed locals and a value stack. Do not require a physical-register
allocator for this target. The supplied machine dump must describe the same
optimized function that is encoded, including typed locals, control regions,
linear-memory frame requirements and call signatures. A WAT/MIR dump is a
view, not a transport format to another compiler or assembler.

Run the existing LowIR optimizer using the selected target's layout,
capabilities and cost policy. Preserve its semantic improvements without
copying x86-specific profitability decisions. Optimizing levels must satisfy
the supplied envelopes for locals, frame accesses, code growth and kernel
behavior. Pass names, stackification order, local indices, block nesting and
individual Wasm instructions are your choices.

### How output is checked

Fixtures check compilation status, independent module validation, program
output, return/exit status and the resulting memory state. ABI cases link
objects from both compilers and exercise calls and exceptions across the
boundary. Repeated-export tests use one module instance to check persistent
state and initialization. A sidecar's `sequences` lists the ordered export calls that share an instance;
ordinary `calls` are independent invocations. Native regression tests remain required.

[Quality checks](quality.md) explain the metrics; fixture sidecars publish
the actual limits and invariants. Tests do not
compare complete instruction sequences against LLVM. Equivalent legal
lowerings are accepted. A simple scalar expression should not need a
linear-memory frame; an ordinary reducible loop should not become a
whole-function dispatch loop. Irreducible graphs still require correct,
bounded lowering.

Fail with nonzero status for invalid inputs, conflicting targets/layouts,
unsupported operations/features, malformed objects, unresolved symbols,
signature/relocation errors and unwritable output. Diagnostic wording is not
prescribed. A successful compile must never hide a fallback to host code.

### Testing

Install the pinned runtime, validator and reference SDK once:

```sh
perl ../scripts/install_wasm_tools.pl --prefix ../obj/wasm-tools
export PATH="$(cd ../obj/wasm-tools/bin && pwd):$PATH"
```

Run `make test` for this assignment or `make check TEST=<fixture>` for a
focused check. Both use the standard parallel runner. Numbered fixture groups
cover targets/modules, values/memory, control/calls, objects/ABI, C++ runtime
and optimization. Install the pinned runtime, validator and reference
toolchain listed in the supplied reference bundle; missing required tools
are reported as prerequisite failures.

Use the assignment target while iterating. Finish with the cumulative
report through this assignment from the repository root with `make test-report-through-pa37`. That command
includes this assignment, so running both is unnecessary for final validation.
The [benchmark target](benchmarks/README.md), `make benchmark`, reports compile time, module size, startup and
steady-state execution separately. Timing comparisons use identical inputs
and runtime settings and are not substitutes for correctness.

### References

Use the [versioned offline reference bundle](../doc/wasm/README.md). Online
sources are the [Wasm core specification](https://webassembly.github.io/spec/core/),
[Basic C ABI](https://github.com/WebAssembly/tool-conventions/blob/main/BasicCABI.md),
[object linking conventions](https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md)
and [WASI SDK](https://github.com/WebAssembly/wasi-sdk).
