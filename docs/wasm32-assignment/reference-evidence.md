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
