# PA37 completion evidence

This is an implementation checklist, not part of the student export. The full
assignment in `pa37/README.md` remains the completion target. A working subset
or a green partial suite does not complete PA37.

| Requirement | Current evidence | Remaining qualification |
| --- | --- | --- |
| Target selection and LowIR round-trip | Explicit wasm32/WASI profiles; ILP32 pointer/long types, aggregate shapes; source and text paths in PA37 | Complete binary128 evaluation, target headers and all ABI-sensitive class facts |
| Scalar and aggregate execution | 23 fixtures at O0–O3; LLVM caller into cppgm aggregate provider | Wide integers/floats, varargs, complete type/layout edge cases |
| CFG and calls | Reducible/nested/irreducible loops, cyclic phi transfers, recursive/indirect calls | Expanded switch/stack restoration and independent generated cases |
| Standard objects and own static linker | Linking-v2 writer/reader/own linker; both LLVM interop directions at O0–O3; GNU/BSD archive probes; 32 normal fixtures | Complete EH-tag/wide/runtime object profile and malformed-feature/relocation cases |
| C++ runtime | Dynamic initializer execution in a core start function | Allocation, virtual dispatch, RTTI, local statics, finalization and cross-compiler modern EH |
| Modern EH execution dependency | Pinned Wasmtime executes an independent try_table/throw probe | Compiler lowering, tags, cleanup/catch/rethrow and runtime libraries |
| WASI commands and freestanding reactors | Freestanding module exports and memory | Pinned sysroot/runtime, startup/args/I/O/exit, repeated-export state checks |
| SIMD and shared optimization | Shared LowIR optimizer; native-only aggregate carrier rewrite gated by target ABI | Portable SIMD legalization, target costs, local/stack cleanup and published quality envelopes |
| Robust diagnostics and determinism | Bad target/version/shape tests; bounded MIR validation | Complete malformed object/feature/signature/relocation coverage and deterministic multi-object linking |
| Course material and export | PA37 handout/design notes, target/MIR scaffolds, offline source/licence pins, normal Perl runner | Final scope and scaffold review, complete reference-tool prerequisites, clean export qualification |
| Benchmarks and CI | Required Wasm lane integrated with pinned runtime/validator/reference SDK on both hosts | Standard benchmark entry, separate compile/size/startup/runtime measurements and final Linux/macOS results |
| Native regressions and efficiency | Previous aggregate snapshot: 10674 strict tests, debug info, PA5 self-host, byte-identical inception; aggregate change +0.002% instructions | Repeat required checks and native performance comparison for the completed implementation on the current LLVM base |

All raw results, probes, pinned binaries and performance observations live outside
Git under `/mnt/vizier/cppgm-wasm32-evidence`. Earlier qualification applies only
to its recorded snapshot; it does not establish qualification of later changes.
