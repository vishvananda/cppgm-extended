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

## Qualified object checkpoint

`f547e4f42` is the object/linker checkpoint on local LLVM base `dd9641977`.
It passes the strict cumulative report (10684/10684), PA37 with both host-built
and self-built tools (32/32), debug info, PA5 self-host, byte-identical inception,
PA36 smoke, architecture/format/file/placement audits and the complete harness.
The student export regenerates references consistently; a fresh bundle client
passes its 270-document inventory and all 43 wrapper checks. File audits retain
eight pre-existing header-division warnings and add none.

Frozen native A/B compiler binaries compare this checkpoint against its parent
`18368732f`, including the same LLVM work. On the frozen semantic-overload input:

| Compile level | Baseline instructions | Candidate instructions | Change |
| --- | ---: | ---: | ---: |
| O0 | 3,055,723,160 | 3,055,513,696 | -0.0069% |
| O3 | 9,788,354,299 | 9,788,509,998 | +0.0016% |

Generated native objects are byte-identical at both levels. This does not make
a wall-time claim. Immutable binaries, commands, input hashes, raw counters and
the candidate patch are retained in `perf-object/` under the evidence directory.

Remaining runtime probes are explicit gaps: SDK 34 startup uses imported
`__memory_base` as well as `__stack_pointer`; C++ entry-point adapters are needed
for its `__main_void`/`__main_argc_argv` conventions. A Wasm `long double` source
probe also exposed a frontend lowering gap: it has binary128 storage/shape facts
but still misses the existing soft-float classification path. These are work
for the next ABI/runtime stage, not reasons to mark PA37 complete.
