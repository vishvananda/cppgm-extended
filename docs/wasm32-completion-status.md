# PA37 completion evidence

This is an implementation checklist, not part of the student export. The full
assignment in `pa37/README.md` remains the completion target. A working subset
or a green partial suite does not complete PA37.

| Requirement | Current evidence | Remaining qualification |
| --- | --- | --- |
| Target selection and LowIR round-trip | Explicit wasm32/WASI profiles; ILP32 pointer/long types, aggregate shapes, target-sized class ABI and physical structor returns | Broader portable C++ target cases and final integrated qualification |
| Scalar and aggregate execution | Qualified binary128 and variadic boundaries; scalar i128 selection and mixed LLVM calls now implemented | Remaining float formats and complete type/layout edge cases |
| CFG and calls | Reducible/nested/irreducible loops, cyclic phi transfers, recursive/indirect calls | Expanded switch/stack restoration and independent generated cases |
| Standard objects and own static linker | Linking-v2 objects, multi-result control/signatures, modern EH tags/references, both LLVM interop directions at O0–O3, archives and static GOT address globals | Defined globals, remaining object/feature/SIMD profile and malformed inputs |
| C++ runtime | Class ABI, allocation, dispatch, RTTI, guards, finalization and modern source EH; automatic EH/no-EH runtime selection | Broader portable C++ cases and final integrated qualification |
| Modern EH execution dependency | Source/serialized EH, typed catches, nested cleanup contexts, stack recovery, opaque LowIR payloads and mixed LLVM producers/linkers pass | Additional merged-context/foreign-language cases and final integrated qualification |
| WASI commands and freestanding reactors | Qualified WASI startup, argument/I/O/exit behavior, freestanding initialization and same-instance state/EH sequences | Broader C++ services and final integrated qualification |
| SIMD and shared optimization | Shared LowIR optimizer; native-only aggregate carrier rewrite gated by target ABI | Portable SIMD legalization, target costs, local/stack cleanup and published quality envelopes |
| Robust diagnostics and determinism | Bad target/version/shape tests; bounded MIR validation | Complete malformed object/feature/signature/relocation coverage and deterministic multi-object linking |
| Course material and export | PA37 handout/design notes, target/MIR scaffolds, offline source/licence pins, normal Perl runner | Final scope and scaffold review, complete reference-tool prerequisites, clean export qualification |
| Benchmarks and CI | Required Wasm lane integrated with pinned runtime/validator/reference SDK on both hosts | Standard benchmark entry, separate compile/size/startup/runtime measurements and final Linux/macOS results |
| Native regressions and efficiency | Latest EH snapshot: 10772 strict tests, 83 Wasm controls with host/self tools, debug info, PA5 self-host and byte-identical inception | Repeat required checks and native performance comparison for the completed implementation on the current LLVM base |

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

Full portable C++ classes/RTTI/EH, wide scalar legalization, SIMD, quality
and benchmarks remain required work. Variadic qualification follows below.


## Variadic checkpoint

`92d37bcee` preserves explicit unnamed scalar types and aggregate shapes in
LowIR and implements target `va_list`, caller-owned buffers, promoted scalar
arguments, singleton/empty/indirect records, owned copies, recursion and WASI
formatted output. The shared argument-fact record remains 12 bytes and
`FunctionBody` remains 248 bytes. The LLVM empty-record `va_arg` discrepancy
is recorded separately in the maintainer reference evidence; a behavioral
fixture checks the correct empty-record semantics independently.

The strict cumulative report passes 10725/10725; PA37 passes 54/54 with both
host-built and self-built tools. Debug info, PA5 self-host, byte-identical
inception, PA36 smoke, the complete harness, architecture/format/file checks
and Clang 21 syntax checks pass. The student export regenerates consistently;
a fresh bundle client passes its 270-document inventory and all 43 wrappers.
Placement checks report no early placement errors; eight inherited header
division warnings remain. Raw results have the prefix `varargs-`.

Frozen native compilers compare the preceding WASI compiler and this snapshot:
O0 changes from 3,055,699,746 to 3,056,095,465 instructions (+0.0130%); O3
from 9,896,896,312 to 9,897,026,753 (+0.0013%). Objects remain byte-identical.
Every observation, immutable binary, input hash and candidate source patch is
retained in `perf-varargs-final/`; no wall-time claim is made.


## Scalar i128 checkpoint

The selector now preserves scalar i128 snapshots, literal halves, narrow views,
volatile storage, signed/unsigned comparisons, carry/borrow, bit operations,
word-boundary shifts and narrow/wide integer conversions. Its public ABI uses
two i64 arguments and an indirect result, shared with ordinary/indirect calls,
singleton records and variadic buffers. Backend legalization turns costly
arithmetic and f32/f64 conversions into typed calls to supplied compiler-rt;
existing binary128 conversions use the same wide integer boundary.

Six required behavioral fixtures cover these operations, loops, overlapping
LowIR storage views, exact ABI signatures, both producer/linker directions and
numerical checks at O0–O3. The 256-iteration numerical digest is 1897815904 with
independent GCC 15 and Clang 21 native builds and with the Wasm compiler. The
existing LowIR restriction against i128 phi/select remains unchanged; wide
values crossing source merges remain in slots, and the shared optimizer may
expand them into i64 halves. Scalar typed control results are now validated,
including the required else arm of a value-producing if.

`091293697` is rebased onto local LLVM tip `ac254985a`. The strict report
passes 10731/10731, and PA37 passes 60/60 with both host-built and self-built
tools. Debug-info checks, PA5 self-host, byte-identical inception and PA36
smoke (3/3) pass. Architecture/source/format/scaffold/file checks and Clang 21
syntax checks pass; the eight inherited file-audit warnings remain. The
complete harness passes. Placement qualification reports zero placement and
local-hygiene findings; its 89 EH review leads are advisory.

The student export regenerates consistently, including the current LLVM
branch's informational MIR references. A fresh bundle client passes its
270-document inventory and all 43 wrapper checks. The previous informational
reference-only regeneration commit was superseded during the rebase by the
newer LLVM references. Logs have the prefix `i128-`.

The native performance comparison uses frozen binaries on the same LLVM base,
rebuilding the preceding Wasm source set and reusing identical common/native
objects. At O0, instructions change from 3,058,641,559 to 3,058,753,481
(+0.0037%); at O3 from 9,951,635,124 to 9,951,563,214 (-0.00072%). Objects
are byte-identical. Build commands, baseline source archive, input paths,
binary hashes and every observation are retained in `perf-i128-rebased/`.
These counts make no wall-time claim.

Full classes/RTTI/EH, SIMD, remaining target type/layout boundaries, quality,
benchmarks and final complete qualification remain required. Concrete class
layout/member-pointer reducers are recorded in the maintainer reference
evidence; this checkpoint does not complete PA37.


## Class ABI stage

The current implementation uses typed target facts for vptr storage, vtable/VTT
rows, RTTI pointers, member-pointer representation, array cookies and guards.
Wasm member-function pointers are indirect records with adjustment-word tags.
LowIR preserves the physical constructor/destructor return convention with
`this_return=yes`; adjusting destructor adapters return the adjusted pointer.
The shared boundary metadata remains 12 bytes and `FunctionBody` 248 bytes.

The class tests cover neighboring fields, direct/virtual member pointers,
variadic record boundaries, multiple/virtual inheritance, construction VTTs,
covariant returns, RTTI/downcasts, local-static guards, allocation and lifetime.
Mixed LLVM/cppgm objects use both linkers at O0–O3. Static GOT address imports
from the supplied C++ libraries resolve to ordinary table/data addresses;
serialized LowIR finalizers register through the supplied `atexit` interface.

`fac42ea38` is rebased onto LLVM tip `0df33eb68`. The strict report passes
10750/10750; PA37 passes 67/67 with host-built and self-built tools. Debug-info,
PA5 self-host, byte-identical inception, PA36 smoke (3/3), the complete harness,
Clang 21 changed-source syntax and all architecture/format/file audits pass.
Placement scans 10090 tests with zero placement or local-hygiene findings;
the 89 EH review leads and eight inherited file warnings remain advisory.
The student export regenerates consistently. Its fresh reference-bundle client
passes the 270-document inventory and all 43 wrappers, and includes the new
runtime fixture header while excluding this journal.

The isolated native comparison uses immutable `091293697` and `72efa1681`
binaries on the same preceding LLVM base, `ac254985a`. O0 changes from
3,058,630,066 to 3,058,796,283 instructions (+0.0054%); O3 from
9,950,886,800 to 9,951,175,498 (+0.0029%). Native objects are byte-identical.
`perf-class/` retains the commands, input paths, hashes, source patch and every
observation. These counts do not measure wall time or the subsequent LLVM
rebase's combined cost.

Class tests use the pinned no-EH C++ runtime to qualify their ABI independently.
This is not completion of the required exception profile. Modern EH, SIMD,
the rest of the object profile, quality envelopes, benchmarks and final
qualification remain required. Raw class results have the prefix `class-`.

## Multi-value and modern EH object stage

Object signatures now preserve result tuples. Typed control consumes block
parameters, loop backedges carry loop parameters, other branches carry results,
and block types use signed 33-bit encodings. Validation consumes relocated
function/type/global/tag identities rather than trusting stale placeholder
indices. Signed immediate overflow and absent required index relocations fail.

Tags, all four modern catch kinds, exception-reference locals, typed select,
reference null tests and `throw_ref` survive object reading, validation, weak/
COMDAT resolution and static linking. Supplied LLVM C++ objects exercise
cleanup, typed catch, rethrow and catch-all against the actual EH runtime.
Raw-tag interop separately checks payload signatures and linear-stack recovery.
Neither fixture substitutes LLVM for cppgm's source lowering: C++ source EH
selection remains unfinished.

The strict report passes 10753/10753; PA37 passes 70/70 with host-built and
self-built tools. Debug info, PA5 self-host, byte-identical inception, PA36
smoke, the complete harness, architecture/format/file audits and Clang 21
changed-source syntax checks pass. The student export regenerates consistently;
its initial attempt omitted the then-untracked new Perl helper, and the tracked
rerun includes the helper and all three fixtures. A fresh bundle client passes
its 270-document inventory and all 43 wrappers. Placement scans 10093 tests with
zero placement/local-hygiene findings and 89 advisory EH leads. Eight inherited
file warnings remain. Logs use `eh-object-`; the successful export is
`export-tracked`.

Frozen native binaries on LLVM base `0df33eb68` compare `9ccfdd291` with this
stage. O0 changes from 3,058,794,073 to 3,058,573,870 instructions (-0.0072%);
O3 from 9,923,807,522 to 9,923,482,955 (-0.0033%). Objects are byte-identical.
`perf-eh-object/` retains the baseline source, reused/rebuilt object provenance,
commands, binary hashes and every observation. These counts make no wall-time
claim. Source EH, SIMD, the remaining object/quality profile, benchmarks and
final full qualification remain required.

## Shared exception-analysis stage

The native handler-state and action-table algorithms now live in `machine/`.
Native selection supplies its existing CFG; it copies small block facts only
for functions containing EH, then assigns native frame/pad locations to the
shared results. The shared tables contain no native frame offset. This makes
the existing nested-handler, selector-pruning and resume rules available to
Wasm without another analysis or another CFG build. Native source selection
and instruction encoding remain in their owning modules.

The strict report passes 10753/10753 and PA37 passes 70/70 with host-built
and self-built tools. Debug info, PA5 self-host, byte-identical inception,
PA36 smoke (3/3), the full harness, Clang 21 syntax and architecture/format/file
checks pass. Moving the PA26 contract citation required moving its audit-table
entry too; the final audit log records the corrected mapping. File warnings
drop from eight to seven because the native state header is smaller.
The student export regenerates consistently; a fresh bundle client passes its
270-document inventory and all 43 wrappers. Placement scans 10093 tests with zero placement/local-hygiene findings and
89 advisory EH leads. Results use `eh-shared-`.

Frozen native compilers compare the preceding object stage with this extraction
on the same LLVM base. O0 changes from 3,058,573,959 to 3,058,038,222 instructions
(-0.0175%); O3 from 9,923,483,044 to 9,924,336,282 (+0.0086%). Objects remain
byte-identical. All commands, binaries, hashes and observations are retained
in `perf-eh-shared/`; no wall-time claim is made.

An independent `source-eh-throw-boundary/` probe also verifies cppgm-generated
throws into an LLVM-generated typed catch at O0/O3 with cppgm's linker. This
does not qualify cppgm-generated handlers. The next selection/runtime steps
are recorded in `docs/wasm32-assignment/source-eh-lowering.md`, which is excluded
from the student export. Full source EH, SIMD, the remaining object/quality
profile, benchmarks and final qualification remain unfinished.


## Source exception-selection checkpoint

Wasm now consumes the shared exception analysis and emits modern try-table,
catch-reference and throw-reference instructions for C++ cleanup, typed catches,
local resumes and rethrow. LSDA data follows the pinned runtime personality ABI
and shares each definition's COMDAT lifetime. Exceptional paths restore the
saved linear-memory stack pointer, including live dynamic allocations.

Active landing contexts retain separate exception references and selectors so
an inlined destructor may catch its own exception during an outer unwind.
Actual exceptional edges participate in structured-control analysis; reducible
EH functions no longer require an unconditional whole-function dispatcher.
The source lifetime policy also applies the platform catch-copy termination
boundaries to Wasm while leaving native target selection unchanged.

Focused PA37 qualification passes 75/75 (`source-eh-pa37-contexts.log`).
The 34 broader source probes matched LLVM before the final structured-control
and context edits; they must be repeated. Five new required controls cover
mixed-compiler cleanup/rethrow, typed enclosing handlers, catch-copy termination,
nested cleanup exceptions and dynamic stack recovery. Full qualification of
this source checkpoint is pending; explicit LowIR payload throws, runtime
selection, SIMD, remaining object/quality behavior and benchmarks remain work.


## Rebased source EH, opaque payload and reactor checkpoint

Rebased all 24 Wasm commits onto current main `e2831a513`, including the
merged LLVM optimization work. The preserved pre-rebase branch is
`backup/wasm-pre-main-20261112`; the rebased source checkpoint is `4bbc71930`.

LowIR now carries the typed `exception_abi=opaque|cxx` target-header fact.
Opaque scalar/pointer payloads use a distinct coalesced two-word tag and need
no C++ runtime. Tests cover integer widths through i128, floating bit patterns,
function/data pointers, cleanup/resume, and mixed LLVM tag objects. Typed
catches in an opaque unit and raw throws in a C++ unit fail. A thrown function
pointer's boundary is not the boundary of a call: the added reducer exposed
and fixed incorrect `unwind=no` propagation into a raw throw.

Sysroot links now supply C++, ABI and unwind libraries automatically, select
no-EH libraries for `-fno-exceptions`, and honor the explicit library-disable
flags. Per-input device/inode identities prevent duplicate default archive
loading without parsing diagnostic member paths. Serialized C++ LowIR defaults
to the EH runtime; callers needing another library configuration may disable
default libraries and supply it explicitly.

The Perl checker now appends and independently validates a small assertion
driver for each same-instance call sequence. It checks persistent memory,
one-time global/local initialization, signed 64-bit constants and repeated EH
and stack recovery. Incorrect state expectations fail at the named assertion.
LLD reference reactors export their constructor entry and the reference host
calls it once, disabling LLD's per-command constructor/destructor wrappers.
Cppgm's freestanding module continues to initialize through its actual start
section. An address-only LLVM interop reducer also exposed a table-liveness
issue: function-address relocations now retain the table symbol even without
an indirect call, following the object convention.

The strict report passes 10772/10772; host-built and self-built PA37 tools pass
83/83. PA5 self-host and byte-identical inception pass. Debug info, PA36 smoke,
Clang syntax and the architecture/file checks pass; seven inherited file
warnings remain. The first placement pass scans 10108 tests with zero placement
or local-hygiene findings and 89 advisory EH leads. The final harness and refreshed student export pass; a fresh bundle client
passes its 270-document inventory, 63-file prose audit and all 43 wrappers.
Frozen native compilers on the same main base compare `4bbc71930` with this
checkpoint: O0 is 3,058,419,177 versus 3,058,840,900 instructions (+0.0138%);
O3 is 10,346,242,774 versus 10,346,620,736 (+0.0037%). Native objects remain
byte-identical. `perf-eh-complete/` retains source patch, binary/input hashes,
commands and every observation; these counts make no wall-time claim.
Raw logs are prefixed `eh-complete-`, with earlier source-only results under
`eh-source-`. The 34 broader portable EH probes still match pinned LLVM.

SIMD, the remaining object/profile checks, quality envelopes, benchmarks and
final full Linux/macOS qualification remain required. This checkpoint does not
complete PA37.
