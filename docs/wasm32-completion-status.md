# PA37 completion evidence

This is an implementation checklist, not part of the student export. The full
assignment in `pa37/README.md` remains the completion target. A working subset
or a green partial suite does not complete PA37.

| Requirement | Current evidence | Remaining qualification |
| --- | --- | --- |
| Target selection and LowIR round-trip | Explicit wasm32/WASI profiles; ILP32 pointer/long types, aggregate shapes; source and text paths in PA37 | Broader C++ target headers and all ABI-sensitive class facts |
| Scalar and aggregate execution | 23 fixtures at O0–O3; LLVM caller into cppgm aggregate provider | Wide integers/floats, varargs, complete type/layout edge cases |
| CFG and calls | Reducible/nested/irreducible loops, cyclic phi transfers, recursive/indirect calls | Expanded switch/stack restoration and independent generated cases |
| Standard objects and own static linker | Linking-v2 writer/reader/own linker; both LLVM interop directions at O0–O3; GNU/BSD archive probes; 32 normal fixtures | Complete EH-tag/wide/runtime object profile and malformed-feature/relocation cases |
| C++ runtime | Dynamic initializer execution in a core start function | Allocation, virtual dispatch, RTTI, local statics, finalization and cross-compiler modern EH |
| Modern EH execution dependency | Pinned Wasmtime executes an independent try_table/throw probe | Compiler lowering, tags, cleanup/catch/rethrow and runtime libraries |
| WASI commands and freestanding reactors | Freestanding module exports and memory | Repeated-export state checks and broader C++ services |
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

## Binary128 runtime checkpoint

`d62abdef7` adds the binary128 argument/result boundary, target-aware soft-float
classification and comparison return width, portable memory builtins, imported
runtime globals, relative relocations and explicit runtime archive inputs for
`lowir2native`. Pure sign-bit negation removes the unavailable `__negtf2` helper
without changing NaN payloads. Mixed-producer tests exercise arithmetic and
precision beyond x87 through the supplied compiler-rt and libc archives.

This checkpoint passes the strict report (10687/10687), the integrated-assembly suite (PA29, 177/177), PA37
with both host-built and self-built tools (35/35), debug-info checks, PA5
self-host, byte-identical inception, PA36 smoke, source/placement/format/file
audits and Clang 21 syntax checks. The export regenerates consistently. Full
logs are prefixed `runtime-` in the external evidence directory.

Relative to frozen `f547e4f42`, native semantic-overload compilation changes
from 3,055,513,686 to 3,055,676,669 instructions at O0 (+0.0053%), and from
9,788,509,988 to 9,788,496,028 at O3 (-0.00014%). Native objects are identical.
Every instruction-count observation is retained in `perf-runtime/`; no wall-time
inference is made from these runs.

## Target-precision constant evaluation checkpoint

`d7b8cd4c8` evaluates binary128 arithmetic, comparisons, casts, builtin special
values and object bytes without rounding through host `long double`. The cached
representation bit fits existing type flags; `Type` remains 72 bytes and
`ConstValue` 48 bytes on the qualification host. PA30 now states the target
precision requirement and checks exact/inexact wide integer list initialization.

After rebasing onto local LLVM tip `3b78b4521`, the strict report passes
10696/10696, PA37 passes 40/40 with both host-built and self-built tools,
debug info and PA5 self-host pass, inception is byte-identical, PA36 smoke is
3/3, the complete harness passes, and architecture/format/file/placement checks
pass (the same eight inherited file warnings). PA30 passes 73 preprocessing,
498 compile and 211 run checks. GCC and Clang accept the positive native
precision reducer. Independent numerical probes check 300,000 arithmetic and
integer cases, including cancellation, and 279,800 conversion cases against GCC.

Frozen native compilers on the same LLVM base compare `6b170dd28` to
`d7b8cd4c8`: O0 instructions change from 3,055,770,328 to 3,055,879,270
(+0.0036%); O3 from 9,896,408,719 to 9,896,312,452 (-0.0010%). Native objects
are identical. All observations and hashes remain in `perf-constant/` outside
Git. These counts make no wall-time claim.

The rebase changes native block placement. The first fresh export therefore
reported 28 informational PA34 MIR/compact-MIR differences; `2ccb2dff5`
regenerates just those 14 affected fixtures through the normal reference target.
Export qualification will be repeated with the next runtime checkpoint.

## WASI runtime checkpoint

`3c152bd73` runs both main signatures through the pinned SDK CRT, libc and
compiler-rt with cppgm's own linker. Target header selection, core object exports,
constructor wrappers, argument setup, heap growth, stream I/O and finalization
have focused tests. Archive extraction precedes section collection so dead
references can bring in strong replacements for live weak fallbacks; a reducer
pins this behavior without naming a runtime implementation.

The strict report passes 10703/10703; PA37 passes 47/47 with host-built and
self-built tools, including mixed LLVM/cppgm objects with both linkers. Debug
info, PA5 self-host, byte-identical inception, PA36 smoke, the full harness,
architecture/source/format/file audits and Clang 21 syntax checks pass. The
student export regenerates consistently, including all 47 Wasm controls. A
fresh reference-bundle client passes its 270-document inventory and 43 wrappers.
Placement verification reports zero placement and local-hygiene findings; the
full report is retained in `wasi-placement.log`.

Frozen native instruction counts versus `d7b8cd4c8` are 3,055,879,786 versus
3,055,728,337 at O0 (-0.0050%) and 9,896,313,186 versus 9,896,110,222 at O3
(-0.0021%). Generated objects are identical. Commands, hashes and every
observation remain in `perf-wasi/`; these counts make no wall-time claim.

Full portable C++ classes/RTTI/EH, wide scalar legalization, varargs, SIMD,
quality and benchmarks remain required work. The next variadic reducer is
`wasm-varargs-probe.cpp` outside Git: source acceptance still assumes AMD64's
`__va_list_tag`, while this target's argument list is a pointer. Its lowering
and the backend boundary must use the target representation together.
