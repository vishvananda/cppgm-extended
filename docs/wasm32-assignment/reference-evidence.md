# Reference investigation

Maintainer research, 2026-11-11; excluded from the proposed student export.
This records reference-tool probes, not an implemented cppgm Wasm backend.
The [plan](../wasm32-backend-plan.md) separates the remaining qualification
work from the observed results.

## Repository base

The plan branch starts from `opt/llvm-gap-claude` at
`fbe6140a4531ab9a660e499e67191dde9b8e7928`. That committed tip includes
field-access narrowing, address sinking, memory forwarding, loop unrolling,
phi simplification and narrow call/return work. No uncommitted changes were
copied from its worktree.

Reviewed the existing target, LowIR types/signatures, optimizer, MIR operands,
register/frame model, backend dispatch and student exporter. The current
backend is x86-64-specific; Wasm32 cannot be added correctly by changing only
the final instruction encoder.

## Tools and reproducible probes

Observed locally: Ubuntu Clang 21.1.8, LLD 22.1.2, LLVM inspection tools 22,
and Node v22.22.1. The mixed Clang/LLD versions are sufficient for this small
object/link smoke probe; they are not a qualified final course toolchain.
Wasmtime and a standalone Wasm validator were not installed. All generated
files were kept outside Git under `/mnt/vizier/cppgm-wasm32-evidence`.

From a temporary directory, with `PROBES` set to this directory's absolute
`probes/` path:

```sh
clang++-21 --target=wasm32-unknown-unknown -std=c++11 -O0 -nostdinc++ \
  -S -emit-llvm "$PROBES/abi.cpp" -o abi.ll
clang++-21 --target=wasm32-unknown-unknown -std=c++11 -O0 -nostdinc++ \
  -S "$PROBES/abi.cpp" -o abi.s
clang++-21 --target=wasm32-unknown-unknown -std=c++11 -O2 -nostdinc++ \
  -c "$PROBES/abi.cpp" -o abi.o
wasm-ld-22 --no-entry --export=run --export=pair --export=one \
  --export=calls abi.o -o abi.wasm
llvm-readobj-22 --sections abi.o
node -e 'const fs = require("fs"); const bytes = fs.readFileSync("abi.wasm"); if (!WebAssembly.validate(bytes)) process.exit(1); WebAssembly.instantiate(bytes, {}).then(({instance}) => { const result = instance.exports.run(); console.log(result); if (result !== 21) process.exit(1); }).catch(e => {console.error(e); process.exit(1);});'
```

The module validated and `run()` returned **21**. No unresolved-symbol import
allowlist was needed. Dead stripping removes the uncalled quad function and
its runtime helper; this smoke does **not** test executing quad arithmetic,
indirect calls or every ABI entry. The Node invocation is a research check,
not a proposed student harness; student-facing scheduling/checks use Perl and
the standard runner.

The source's static assertions confirmed 4-byte pointers/`long`, 16-byte
size/alignment for `long double` and 113 significand bits. Builtin-macro
inspection also reported `size_t` as `unsigned long`, `ptrdiff_t` as `long`
and `wchar_t` as `int`.

| C++ signature | LLVM's final Wasm signature in the O0 assembly view |
| --- | --- |
| `One one(One)` with one `int` | `(i32) -> i32` |
| Nested singleton aggregate | `(i32) -> i32` |
| Two-int `Pair pair(Pair)` | `(i32 result_address, i32 argument_address) -> ()` |
| Overaligned singleton `Aligned aligned(Aligned)` | `(i32 result_address, i32 argument_address) -> ()` |
| `__int128 wide(__int128)` | `(i32 result_address, i64 low, i64 high) -> ()` |
| `long double quad(long double)` | `(i32 result_address, i64 low, i64 high) -> ()` |
| Signed-char identity | `(i32) -> i32`, with narrow extension semantics |
| `int sum(int, ...)` | `(i32 count, i32 varargs_address) -> i32` |

The LLVM IR view still uses `i128` and `fp128` function types; the final
machine ABI legalizes their results through a hidden pointer. Looking only
at LLVM IR would miss this. The object contained `linking`, `reloc.CODE` and
`target_features` custom sections. These observations support the typed ABI
shape and machine-value legalization requirements in the plan.

## Exception encoding probe

```sh
clang++-21 --target=wasm32-unknown-unknown -std=c++11 -O1 -nostdinc++ \
  -fwasm-exceptions -S "$PROBES/eh.cpp" -o eh-legacy.s
clang++-21 --target=wasm32-unknown-unknown -std=c++11 -O1 -nostdinc++ \
  -fwasm-exceptions -mllvm -wasm-use-legacy-eh=false \
  -S "$PROBES/eh.cpp" -o eh-modern.s
```

This Clang version defaults to legacy `try`/`catch`/`rethrow` encoding with
`-fwasm-exceptions`. Explicitly disabling legacy EH produced `try_table`,
`catch_ref`, `catch_all_ref` and `throw_ref`, with an `__cpp_exception` tag.
Both paths referenced C++ ABI/personality runtime services. Generating this
assembly does not establish working mixed-compiler exception execution.

Before implementing the lesson, pin a modern-EH-compatible runtime/library
closure and test real throw/catch/cleanup/rethrow interop. Do not assume that
an installed WASI sysroot contains a matching exception runtime, or that
Clang's default EH encoding is the intended course profile.

## ARM interface check

```sh
clang++-21 --target=aarch64-unknown-linux-gnu -std=c++11 -O2 -nostdinc++ \
  -S -emit-llvm "$PROBES/arm.cpp" -o arm.ll
clang++-21 --target=aarch64-unknown-linux-gnu -std=c++11 -O2 -nostdinc++ \
  -S "$PROBES/arm.cpp" -o arm.s
```

The floating-pair argument is represented as two float elements in LLVM IR,
and the larger aggregate uses an indirect result. The integer selector emits
three-address arithmetic and conditional selection. This is a design probe,
not an ARM execution test or proof of cppgm ARM support.

## Primary references and remaining qualification

- [WebAssembly core specification](https://webassembly.github.io/spec/core/):
  validation, instruction semantics and binary encoding. Pin a snapshot before
  freezing the feature profile.
- [Basic C ABI](https://github.com/WebAssembly/tool-conventions/blob/main/BasicCABI.md):
  wasm32 ILP32, binary128 `long double`, scalar/aggregate/varargs conventions.
- [Linking conventions](https://github.com/WebAssembly/tool-conventions/blob/main/Linking.md)
  and [LLD Wasm documentation](https://lld.llvm.org/WebAssembly.html): object
  metadata, relocations, feature compatibility and linker behavior.
- [LLVM machine pipeline](https://llvm.org/docs/GlobalISel/Pipeline.html)
  and [legalization](https://llvm.org/docs/GlobalISel/Legalizer.html): useful
  separation of legal types/operations and target instruction selection.
- [AAPCS64](https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst):
  register/aggregate ABI design checks for a future register target.
- [WASI SDK](https://github.com/WebAssembly/wasi-sdk) and
  [Wasmtime proposal support](https://docs.wasmtime.dev/stability-wasm-proposals.html):
  select a pinned sysroot, runtime and engine configuration. The current
  Wasmtime matrix lists modern EH among supported proposals, while its Winch
  footnotes differ; the plan uses Cranelift for normative execution.

Still unmeasured: cppgm Wasm correctness/performance, runtime archive
relocation coverage, mixed-compiler C++ ABI/EH execution, WASIp1 startup,
macOS dependency setup, export behavior for the future PA, and calibrated
quality/performance limits. They are explicit implementation milestones.

## SDK 34 variadic checks

Both LLVM/cppgm producer directions now exercise scalar varargs, coerced
singleton records, indirect records, binary128, argument copies and hidden
result storage through both linkers. SDK 34 uses at least four bytes per
variadic slot. Its [pinned frontend ABI implementation](https://github.com/llvm/llvm-project/blob/895aa2c896ada719451be2e3673c83da8ddf1141/clang/lib/CodeGen/Targets/WebAssembly.cpp)
classifies records before packing; a same-sized record and wide scalar need
not have the same buffer representation.

A reference limitation is isolated outside the student export. SDK 34 omits an
empty record at the call but advances its argument cursor when reading one.
An all-LLVM reducer with an empty read before later arguments returns 48 where
the source calculation requires 89. Raw source, LLVM IR and execution output
are retained under `cppgm-wasm32-evidence/varargs-empty-reference-only.*` and
`varargs-aggregate-{caller,provider}.cpp`. The mixed-producer aggregate test
puts its empty read last to avoid relying on that reference defect. A separate
cppgm behavior test checks empty records between narrow values without consuming
a slot. This does not prescribe the reference compiler's erroneous cursor
advance as a student requirement.


## Scalar i128 and the next class boundary

The scalar i128 checkpoint is `091293697` on local LLVM base `ac254985a`.
An SDK 34 assembly probe confirms two i64 arguments and an indirect result,
including compiler-rt multiplication/division and integer/float conversions.
The numerical fixture's digest (1897815904) agrees with independent GCC 15 and
Clang 21 builds. Commands, assembly, immutable binaries and all observations
are retained in the external evidence directory.

Class support is still incomplete. Three independent LLVM/cppgm reducers
make the next changes concrete:

- `class-layout-probe.cpp`: LLVM accepts an eight-byte polymorphic object
  containing a vptr and int; cppgm fails that assertion.
  `Analyzer::place_table_pointer` uses a fixed 64-bit count before placing
  members, despite using the target pointer size for size/alignment.
- `class-member-pointer-neighbor-probe.cpp`: a member-function pointer followed
  by a sentinel works with LLVM and traps with cppgm. The semantic size is
  correctly eight bytes, but lowering still uses the native i128 carrier and
  64-bit function/adjustment words. A pointer stored alone can appear to work;
  the neighboring field exposes the representation mismatch.
- `class-virtual-probe.cpp`: vtable/typeinfo output still requires a target
  C++ runtime and target-sized table entries/headers. Native eight-byte table
  strides and sixteen-byte RTTI address points remain in lowering.

Retain those reducers for the class stage. Use one typed target C++ ABI policy
for pointer-word width and member-pointer tagging, carry it through layout,
constants, data emission and calls, and preserve native output. Wasm's function
member discriminator is in the adjustment word; mixed producer tests must
check both directions, field neighbors, multiple/virtual inheritance, nulls,
RTTI, guards and destruction. An internally consistent but native-shaped
representation does not establish target ABI compatibility.

The pinned LLVM default Wasm target rejects source `_Float16` and `__bf16`;
`half-reference-probe.cpp` records that capability check. Do not infer their
public C ABI from native half conventions or silently use an x86 helper ABI.
The target's binary128 and i128 boundaries are independently established.


## Class implementation evidence

The earlier class reducers now pass. SDK 34 IR confirms that member-function
pointers use two i32 fields but cross calls as indirect records, rather than
an i64 scalar. The LLVM WebAssembly C++ ABI selects adjustment-word method
pointer tags and four-byte ARM-style guards. Its constructors and non-deleting
destructors physically return `this`; CGVTables forwards the adjusted target's
return through destructor thunks. Deleting destructors retain a void result.
These are typed ABI distinctions, not constructor-name recognition in Wasm.

SDK libc++abi objects also use `GOT.func`/`GOT.mem` address imports. Their
GLOBAL_INDEX relocations name data/function symbols: a static linker must
materialize address globals, not substitute a data address as a global index.
The pinned linking conventions and local LLD implementation establish this
profile. `__dso_handle` is the static module's memory base. The new behavioral
fixtures exercise address globals and class runtime objects with both linkers.

## Modern EH implementation boundary

`modern-eh-class-probe.cpp` independently exercises a cleanup, typed catch,
rethrow and outer catch-all. SDK 34 with `-fwasm-exceptions -mllvm
-wasm-use-legacy-eh=false` builds a valid module returning 17 for argument 4.
Its assembly uses `try_table`, `catch_ref`, `catch_all_ref` and `throw_ref`,
with `exnref` locals and multi-result `(i32, exnref)` blocks. Its object has
`R_WASM_TAG_INDEX_LEB` relocations to the standard `__cpp_exception` tag.
The supplied EH `libunwind.a` defines that tag and `__wasm_lpad_context`.
All commands, assembly, objects, symbol output and execution logs are retained
with the `modern-eh-class-` prefix in the external evidence directory.

The pinned `libunwind/src/Unwind-wasm.c` establishes the landing-pad interface:
write a landing index and LSDA address to the 12-byte context, invoke
`_Unwind_CallPersonality` with the unwind pointer, and read its selector.
The C++ personality performs matching/conversion; cleanup paths retain the
exception reference for `throw_ref`. A VM trap must not become a C++ catch.

Implementation order:

1. Generalize typed Wasm signatures/control validation to result vectors,
   block parameters, signed type indices and exception-reference values.
   Preserve the existing public C ABI and validate relocated type fields.
2. Carry tag imports/definitions, symbol binding and tag relocations through
   object reading, validation and linking. Consume the actual runtime's tag.
3. Extract reusable handler/action facts from the native EH analysis where
   possible; keep target frame registers and unwind encodings separate.
4. Select modern EH in the consumed MIR, including landing stack restoration,
   cleanup, matching, resume/rethrow, nested handlers and the explicit LowIR
   payload forms. Encode target LSDA/personality inputs from typed facts.
5. Integrate the supplied EH runtime profile and test both compiler/linker
   directions, including converted catches, destructor exceptions, noexcept,
   persistent state and portable C++14/17 cases. Keep all student checks in
   the standard Perl runner and retain native/export qualification.

Steps 1 and 2 now have required object fixtures. They cover multi-value
parameters/results and branches, relocated signatures, all modern catch kinds,
exception references, typed select, tag exports and malformed fields. Both
linker directions pass at O0–O3. Cppgm's own linker also executes the probe and
the LLVM C++ provider with the supplied EH archives. Reference-library queries
honor `-fwasm-exceptions`, so this evidence uses the EH profile rather than the
SDK's default no-EH archives.

SDK 34's assembler spells an untyped integer select `i32.select` and a typed
exception-reference select `select exnref`. Its accepted `exnref.select` spelling
emits an untyped select that the independent validator rejects. Fixture objects
are independently validated before they become linker inputs.

These results qualify object/runtime interoperation, not cppgm's source EH
lowering. Steps 3–5, SIMD, quality and benchmarks remain required.
