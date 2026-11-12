# PA37 completion evidence

This is an implementation checklist, not part of the student export. The full
assignment in `pa37/README.md` remains the completion target. A working subset
or a green partial suite does not complete PA37.

Current compiler revision: `953836b1c` on `origin/main` at `e2831a513`.
`33ea331f4` adds only the release workflow trigger.
PA37 implementation and required acceptance checks are complete. Full PA36
release, fresh export, Linux qualification and both Intel macOS inception
comparisons pass. Additional application/upstream CI status is recorded below.

| Requirement | Current evidence | Qualification |
| --- | --- | --- |
| Target selection and typed LowIR | Explicit wasm32/WASI profiles, ILP32 type identity/layout, binary128 constants, aggregate/varargs/structor facts, validated host bindings and alternating-target requests | Passed on Linux and Intel macOS |
| Consumed machine boundary | Shared call facts, exception analyses/tables and global data; target-specific selected streams; [AArch64 representation review](wasm32-assignment/machine-boundary-review.md) and executable probe | Passed on Linux and Intel macOS |
| Scalar, wide and aggregate values | Independent wide-integer/binary128 controls, layout and snapshot cases, source/text paths and mixed LLVM calls | Passed on Linux and Intel macOS |
| CFG and calls | Reducible/nested/irreducible flow, cyclic copies, recursive/indirect calls, reproducible Perl CFG generators and independent arithmetic | Passed on Linux and Intel macOS |
| Objects, ABI and static linker | Linking-v2 objects/archives, weak/COMDAT, typed globals/references/tags, explicit imports/exports, generated aggregate callers/callees, malformed sections/features/relocations and both LLVM directions | Passed on Linux and Intel macOS |
| Portable C++ and modern EH | Class/RTTI/guard/lifetime behavior, cleanup/typed catches/rethrow, opaque payloads, stack restoration, handler-copy termination and cross-compiler boundaries | Passed on Linux and Intel macOS |
| WASI and freestanding initialization | Main adapters, arguments/streams/files/allocation/finalization, same-instance state and exit values including 70 and 124–127 | Passed on Linux and Intel macOS; reserved host exit values now checked through a raw `proc_exit` assertion |
| SIMD and optimization quality | All 214 LowIR vector operations, standard SIMD128 object vocabulary, target costs, consumed MIR cleanup and independent published static/fuel envelopes | Passed on Linux and Intel macOS |
| Bounded Perl test harness | Wall/CPU/process-group RSS/file ceilings, Wasmtime guest-memory limit, cancellation and setup/exit distinction; 12 resource tests and 17 existing timeout tests pass | Linux/macOS enforcement and inception pass |
| Course material and export | PA37 handout, ABI/design/quality documents, target/MIR vocabulary, offline licensed references, seeded Perl generators and normal report discovery | Complete: 275-document inventory, 43 wrappers and 120 exported controls |
| Benchmarks and CI | Local and CI O0/O3 scalar/SIMD campaigns pass all independent oracle cases; compile/link/size/startup/setup/steady phases remain separate | Required Wasm campaigns and Linux/macOS qualification pass |
| Native regressions and efficiency | 10821 strict tests; 120 PA37 controls with host/self tools; debug, PA5, byte-identical inception, smoke, full audits/harness; frozen native outputs/counts and 96 timing observations | Complete: all 17 gates and release margins in both builds |

All raw results, probes, pinned binaries and performance observations live outside
Git under `/mnt/vizier/cppgm-wasm32-evidence`. Earlier qualification applies only
to its recorded snapshot; it does not establish qualification of later changes.

Optional compiler self-hosting and browser inception are investigated in the
[separate plan](wasm-browser-inception-plan.md). Chromium and Firefox execute
the small SIMD/EH/WASI/filesystem probes; the complete compiler has not yet run
inside Wasm. The investigation also exposed missing target-independent builtin templates
and an unresolved weak direct call that prevented ordinary stdio linking.
The target-library checkpoint below fixed both required gaps.
The Clang WASI syntax scan passes 274/406 compiler translation units; the 132
failures identify host padding, resource APIs and x87 floating assumptions.
Compiler-host portability remains optional follow-on work.

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


## SIMD128 object vocabulary checkpoint

The shared Wasm vocabulary now describes all 236 standard SIMD128 operations
from the pinned core specification. Each operation carries its immediate form,
stack form, lane bound and maximum memory alignment. Object decoding and typed
stack validation consume those facts, and the encoder supports fixed sixteen-
byte constants/shuffles, lane bytes and memory/lane operands.

An independent LLVM assembly fixture covers every standard SIMD128 opcode and
its signature. Both linkers execute the load/store/extraction witness; malformed
lane indices, shuffle indices and alignment fields fail with both cppgm and the
independent validator. The required PA37 suite passes 84/84. Architecture,
format and file audits pass (seven inherited warnings), Clang syntax passes,
and all 14 scaffold-audit/vocabulary tests pass. Logs use `simd-object-`.

This is object-profile support, not LowIR-to-SIMD source selection. The latter,
shared target costs and the published quality/performance checks remain next.
The full strict/self-host/export qualification above describes the preceding
EH/runtime checkpoint, not this later SIMD object checkpoint. Final integrated
qualification remains required.

## Complete LowIR vector selection checkpoint

All 214 LowIR vector operations now select to standard SIMD128 instructions or
bounded semantic legalization. Packed shift counts retain the full low 64 bits;
float min/max, scalar upper lanes, explicit rounding and invalid conversions
retain their specified behavior. Carryless multiplication uses a bounded set-bit
loop; reciprocal approximations use exact arithmetic within the required bound
and preserve the profile's endpoint behavior. The common V128 alignment is 16,
matching the existing native value ABI and the Wasm ABI.

Required PA37 controls pass 88/88. The independent native-result sweep passes
205 operations at O0/O3 over three input sets; separate required controls cover
the nine remaining operations, including every eight-bit immediate value.
Reference C++ oracle names differ from builtin names, so missing builtin
lowering cannot silently call the oracle implementation. Additional required
cases cover reciprocal exceptional values, full-width shift counts, mixed
LLVM/cppgm vector boundaries and aligned singleton-vector records. Logs use
`simd-source-`, `simd-controls-`, `simd-approximate-` and `simd-full-shift-`;
the successful broad semantic sweep is `vector-sweep-third.log`.

Full integrated native/self-host/export qualification remains required after
the remaining quality, object-profile and benchmark work. This checkpoint does
not complete PA37.

## Consumed MIR and initial quality-envelope checkpoint

The selected Wasm function now retains the actual allocated frame objects with
shared size/alignment/identity facts and target offsets. Validation checks their
non-overlap, alignment and extent. The dump reads that same selected stream,
prints module signature/function/global/tag mappings, frame objects and locals,
and reports conservative typed effects and available source locations. The
shipped machine vocabulary owns effect spellings; the spelling audit now checks
the Wasm writer too, including a negative misspelling control.

A linear local cleanup folds adjacent set/get pairs into the value stack or a
tee and removes unused declarations without renumbering public parameters.
It preserves the original operand ranges and relocation identities. O3's
calibration recurrence drops from 32 locals/371 body bytes to 18/303; the array
transform drops from 37/641 to 24/573. O1's scalar expression drops from 6/49 to
4/41. These are structural measurements, not elapsed-time claims.

The shipped Perl quality checker measures final-module function bodies rather
than compiler-rendered text. Published sidecars bound locals, body bytes,
instructions, memory operations and control depth at O1–O3. The same bounds
pass cppgm source/LowIR paths and independently produced LLVM objects through
both linkers. Per-call engine fuel bounds include transitive Wasm work and
initialization. External negative probes verify that a zero instruction bound
and an exhausted fuel budget both fail. Eight maintainer unit cases cover
immediate decoding, imports/export aliases, invalid bounds and level selection.
All student-facing execution and quality checks remain Perl and use the normal
parallel runner. The new concise quality document is in the export inventory.

Strict qualification passes 10778/10778; host-built and self-built PA37 tools
pass 89/89. The complete 205-operation external vector sweep still passes at
O0/O3 after local cleanup. PA5 self-host, byte-identical inception, debug info,
PA36 smoke (3/3), Clang syntax, architecture/format/file audits and the full
harness pass. Placement scans 10116 tests with zero placement or local-hygiene
findings; seven inherited file warnings and 89 advisory EH review leads remain.
The first harness run found an oracle filename outside the companion naming
convention. Renaming it to `.oracle.cpp` fixes discovery; the focused test and
full harness pass with the corrected name.

The corrected student export regenerates consistently; a fresh bundle client
passes the 271-document inventory, 64-file prose audit and all 43 wrappers.
The successful snapshot is `student-simd-quality-final-export`; the earlier
snapshot predates the companion-file rename. Logs use `simd-quality-` and
`mir-quality-`; `quality-calibration/` retains both compilers' modules, machine
dumps, structural measurements and every fuel-calibration observation.
Frozen native compilers on the same `e2831a513` base compare the EH checkpoint
with this SIMD/MIR/quality stage. O0 is 3,058,840,896 versus 3,059,251,390
instructions (+0.0134%); O3 is 10,346,620,732 versus 10,346,740,957 (+0.0012%).
Objects are byte-identical. `perf-simd-quality/` retains the exact source patch,
frozen binaries/input hashes, commands and observations; no wall-time claim is
made.

Target cost policy, remaining object/profile coverage, standard benchmarks and
final full Linux/macOS qualification remain required. An independent malformed-
object probe confirms a next reader fix: swapping the type/import sections of
a valid LLVM SIMD object is rejected by wasm-tools but currently accepted by
cppgm's linker. Its bytes, commands and results are in
`object-profile-negatives/`. The remaining element/reference/global-expression
profile also needs validation rather than skipped payloads. This checkpoint
does not complete PA37.

## Typed global/reference object checkpoint

The object reader now enforces core section order, duplicate/count rules,
canonical C table declarations and typed element payloads. It rejects passive/
independent initialization and dynamic-link metadata outside the pinned static
object convention. Linking/relocation customs are decoded after core sections,
so their placement does not change interpretation. Missing/mismatched table or
memory identities and conflicting/duplicate/malformed target features fail.

Typed global definitions retain their exact constant-expression bytes and
relocation identities. Global type/mutability checks apply across definitions;
weak/strong and COMDAT selection include globals. Live initializer dependencies
participate in collection and stable ordering after selection. Explicit host
global imports have their own index space before synthesized and defined globals.
The encoder also writes constant expressions directly from typed instructions.
Read/write probes preserve advanced global objects and repeated-instance state.

The model/validator now covers nullable func/extern references, function
references/declarations, typed reference calls/selections and the C function
table's get/set/copy/fill/size/grow operations. Required tests exercise scalar,
vector and reference globals; earlier immutable dependencies; weak replacement;
complete global COMDAT groups; repeated state; reference calls and table growth.
Malformed section order, duplicate sections, maximal wrong data counts, mutable
or mistyped initializer reads, invalid element payloads and bad/conflicting
features are independently checked. The mutation runner retains immutable
per-unit specifications across optimization levels.

SDK 34's mutable __memory_base import is retained, matching the actual CRT.
LLD needs --growable-table for growth tests. Scalar/null-reference globals,
weak selection and reference operations pass both linkers. The pinned LLD
rejects v128.const/ref.func initializers and global COMDAT members, and leaves
an initializer's global index unrelocated in the supplied case. Core validation
accepts those inputs; its output for the relocation reducer is invalid. These
are documented in the shipped reference-limitations.json with fixture links;
the harness requires a named, fixture-specific entry before omitting LLD.
Both compiler producers still call the advanced objects through cppgm linking,
independent validation and engine execution. Ref-null globals are supported by
LLD and have their own shared positive control, not an exclusion. Pinned core
instruction/module validation references are included with hashes/attribution.

The strict snapshot passes 10784/10784; host-built and self-built PA37 tools
pass 95/95. The later null-global addition also passes with both tools and both
linkers. PA5 self-host, byte-identical inception, debug info, PA36 smoke (3/3),
Clang syntax and architecture/format/file audits pass. The final harness passes
after sorting the document inventory; placement scans 10122 tests with zero
placement/local-hygiene findings. Seven inherited file warnings and 89 advisory
EH review leads remain. The final export regenerates consistently; its fresh
bundle client passes the 274-document inventory, 64-file prose audit and all
43 wrappers. The snapshot is student-object-profile-final-export.

Logs use object-profile-. global-reference-probe/ retains the object codec,
input/output objects/modules and repeated-instance validation. It also retains
minimal, independently validated LLVM limitation reducers and raw diagnostics.
Frozen native binaries on e2831a513 compare 5d98b6b13 with this stage. O0 changes
from 3,059,252,907 to 3,058,429,083 instructions (-0.0269%); O3 from
10,346,741,814 to 10,346,618,061 (-0.0012%). Object bytes match. Source patch,
frozen binary/input hashes, commands and all observations are retained in
perf-object-profile/; these counts make no wall-time claim.

Target cost policy, standard benchmark integration, broader profile/language
boundaries and final Linux/macOS qualification remain required. In particular,
exercise custom imported globals with shifted SP/GOT indices, imported memory/
table limits and remaining malformed-profile boundaries. This checkpoint does
not complete PA37.

## Target-aware optimization checkpoint

The shared target description now supplies addressing, call-preservation and
floating-literal facts to function passes. Wasm keeps invariant scalar/address
work across calls, avoids native scaled-address sinking and literal-load
hoisting, and charges short-loop growth for explicit address arithmetic.
Inlining prices Wasm expansion rather than native x87/SSE/LEA forms, does not
discount addresses as folded memory operands, and leaves variable/unknown
expansions unpriced. Required source inlining remains a separate policy.

A source/serialized-LowIR/mixed-LLVM fixture checks call-preserved invariants
through the normal Perl runner at O0–O3, with portable structural and dynamic
fuel bounds. At 1,000 iterations, the same checked result uses 80,303 -> 68,315
fuel at O2 (-14.9%) and 73,138 -> 61,150 at O3 (-16.4%). Pinned LLVM output
also passes the published bounds. These are engine-work counts, not runtime
speedups. The student quality guidance describes requirements rather than a
required pass order or instruction sequence.

The strict report passes 10785/10785. Host-built and self-built PA37 tools pass
96/96; PA5 self-host, byte-identical inception, debug info, PA36 smoke (3/3),
Clang syntax and architecture/format/file audits pass. The placement audit
scans 10123 tests with no placement or hygiene findings; the seven inherited
file warnings and 89 advisory EH leads remain. The full harness passes.
student-target-cost-export regenerates consistently; its fresh bundle client
passes 274 document entries, 64 reviewed prose files and all 43 wrappers.

Frozen native binaries compare 8654da3c9 with this stage on main base e2831a513.
O0 changes from 3,058,429,628 to 3,058,665,116 instructions (+0.0077%); O3 from
10,346,620,146 to 10,349,700,222 (+0.0298%). Objects are byte-identical. Four
A/A blocks and eight paired ABBA blocks per level retain all 96 elapsed/CPU/RSS
observations and verify every output. Paired elapsed changes are -0.58% at O0
and +0.22% at O3; A/A median changes are +0.19% and -0.59%, respectively. Peak
RSS changes by less than 0.03%. This probe detects no material native regression;
it cannot resolve a timing change near two tenths of a percent. Immutable
binaries/inputs, patch/hash provenance and observations are in perf-target-cost/;
Wasm modules, LowIR and every fuel observation are in target-cost-probes/.
Other validation logs use target-cost-.

Standard benchmark integration, remaining object/type/language boundaries and
final Linux/macOS CI qualification remain. The benchmark plan now specifies a
pinned C-API executor to separate VM compilation, instantiation/startup and
warmed calls, with Perl scheduling and the existing catalog/reporting path.
Also exercise standalone lowiropt's target-library policy: its default GNU
combined-math option currently differs from the explicit source/backend policy.
This checkpoint does not complete PA37.

## Standard benchmark integration checkpoint

PA37 now has a usable `make benchmark` target and shipped measurement guidance.
Its Perl driver compiles frozen scalar/explicit-SIMD source forms, validates all
four kernel families against independent unsigned arithmetic, checks every timed
checksum and output identity, and retains every request/observation. A pinned
Wasmtime 50.0.1 C-API host executor separates engine compilation, instantiation/
start, input preparation and warmed execution. CPU/elapsed tool measurements use
wait4 counters, preserving process startup while excluding Perl notification
latency. Ordinary correctness installation does not require the extra C API.

The existing catalog, normalized renderer and compact publisher handle both O0
and O3. Byte metrics have separate integer fields, retain valid zero sections,
and use exact reference/candidate ratios when the denominator is nonzero. GCC
is explicitly not applicable. Startup/setup remain separate from steady runtime.
The README uses the existing host-labeled badges; placeholder seeding and age
refresh include Wasm. The normal hosted workflow reports and retains results;
only successful main runs publish. Dedicated Intel uses the existing sequential
runner campaign and read/measure versus write/publication job separation.
Raw engine/command observations and binaries stay in Actions artifacts.

Local O0/O3 runs each pass 32 independent cases per compiler/configuration
(256 oracle comparisons across both levels). The shared report validates 136
metric records and four standalone correctness rows. Calibration cautions stay
visible; these loaded-host observations do not establish precise small runtime
differences. The publication/measurement tests, shared reporting/freshness tests,
Intel routing tests and full harness pass; later zero-size ratio refinements pass
all 27 shared tests and six Wasm-specific tests. GCC builds and Clang syntax/
format checks accept the host executor; workflow YAML parses. Both the direct
Make target and the exported Perl driver execute successfully.

The final student export regenerates consistently. Its fresh bundle client
passes 275 document entries, 65 reviewed prose files and all 43 wrappers. Shipped
script/input bytes match source. The final snapshot is
student-wasm-benchmark-final-export with wasm-benchmark-final-reference-binaries.tar.gz.
Logs use wasm-benchmark-. O0/O3/raw campaigns and normalized reports remain in
the external evidence directory. Local report URLs use example.invalid to avoid
pretending these measurements came from an Actions run. The superseded export
and duplicate bundle were removed after verification, freeing 439 MB; their
replacement and archive hash are recorded in wasm-benchmark-intermediate-cleanup.json.
No compiler implementation changed in this checkpoint.

The new workloads expose concrete follow-ups: the linker writes an 8,192-byte
zero-filled global array as encoded data, and source vector construction/multiply/
extraction still round-trips through scalar memory rather than useful vector SSA.
The explicit vector kernel is far slower than LLVM, beyond the observed A/A
spread. Its LowIR reducer is wasm-benchmark-probe/simd.lowir; the relevant source
lowering modules are vector_construction.cpp and vector_lowering.cpp, and data
emission is in wasm/link_output.cpp. Keep the workload and observations intact;
fix general lowering and object principles rather than special-casing it.

Those improvements, standalone lowiropt target-library consistency, custom host
globals, imported memory/table limits, malformed names/profile/type boundaries,
and final Linux/macOS CI qualification remain. This checkpoint does not complete
PA37, and no benchmark results have been published from the dirty local checkout.

## Target builtin templates and weak runtime calls

Wasm now retains the target-independent builtin templates already supplied on
native targets, alongside its own pointer-shaped `va_list`. Its predefined
numeric limits describe IEEE binary32/binary64/binary128, with target-correct
wide-character types and language memory-order constants. These declarations
do not enable threads or atomic instructions outside the target profile.

The linker retains null addresses for absent weak functions and supplies typed
trapping definitions for their live direct-call relocations. Weak references
still do not extract archive members. This fixes the SDK libc file-I/O link
without explicitly extracting `chdir.c.obj` or retaining the whole archive.
Required fixtures cover two call signatures, guarded calls, null address
initializers, traps and an archive member with an otherwise unavailable
constructor dependency. Mixed LLVM/cppgm producers and both linkers agree.

The standard Perl runner now supports expected Wasm traps and per-command
preopened input/output directories. Real tests exercise stat, file streams,
seek/read/write and exact output bytes, plus libc++ numeric limits, tuple,
wide-character declarations and unique ownership. Four deliberate harness
misexpectations reject normal returns as traps, traps as success, wrong file
contents and invalid fixture filenames.

Qualification on the e2831a513 base passes: strict report 10790/10790; PA37
101/101 with host and self-built tools; PA5 self-host; byte-identical native
inception; debug info; PA36 smoke 3/3; architecture, source-set, registry,
scaffold, IR and format audits; Clang 21 changed-source syntax; the complete
harness; student document review. Placement scans 10128 tests with no placement
or hygiene findings, retaining 89 advisory EH leads. Seven inherited file-audit
warnings remain.

The fresh `student-library-boundary-export` regenerates consistently. Its
`library-boundary-reference-binaries.tar.gz` installs in a fresh client with
275 document entries, 65 reviewed prose files and all 43 wrappers passing.
The exported runner executes the file-I/O and numeric/library fixtures, and
all 14 changed shipped handout/fixture/runner files match the source snapshot.
Logs, reductions, immutable binaries and provenance are in `library-boundary/`
under the external evidence directory.

Against the previous target-cost compiler, frozen native output is byte-identical
at O0/O3. Instruction counts change by +0.0001% and -0.0042% respectively. Four
A/A and eight ABBA blocks at each level retain 96 CPU/wall/RSS observations;
paired wall changes are -0.44% O0 and +0.56% O3, versus A/A -1.33% and -0.96%.
Peak RSS changes by less than 0.08%. These observations do not establish a
material timing regression or a speedup.

The optional full-compiler probe advances to the SDK's C11 `_Thread_local`
`errno` declaration in `<sstream>` dependencies. That portability work remains
outside required inception qualification. Remaining required profile hardening,
optimizer/source-lowering follow-ups and final Linux/macOS CI qualification
are unchanged; this checkpoint does not complete PA37.

## Compact data, target-library coherence and name encoding

Static linking now omits trailing zero initialization after relocations, including
entire zero segments, while retaining the original storage sizes, alignments,
addresses and memory span. The required zero-storage control checks a large
volatile array, mixed initialized/zero bytes, relocated pointers, a null weak
address, a live function pointer and persistent writes. Its published 2048-byte
data-section bound at O0–O3 also accepts LLVM's merged layout. The quality checker
and benchmark report share the same module-size reader.

On the existing O3 benchmark inputs, scalar modules shrink from 10921 to 2718
bytes and SIMD128 modules from 11477 to 3274 bytes. Both lose their 8200-byte
data payload. Every other section is byte-identical, including code, globals,
memory/table layout and exports, and all 32 independent cases per compiler/mode
agree. This is a size improvement, not a claim to fix the slow source-vector
lowering. Module bytes, commands, 128 oracle results and section checks are in
`profile-closure/module-comparison/` under the external evidence directory.

The three source/standalone driver paths and the Wasm backend now consume the
same typed target-library policy. `lowiropt` no longer assumes GNU sincos on a
Wasm profile. Its new fixture checks the undefined-function dependency surface
and executes the standalone optimizer's output, alongside the source and normal
serialized paths. The pinned WASI libc happens to provide GNU sincos, but the
default Wasm profiles do not promise that extension; the fixture uses the
freestanding profile with explicit runtime inputs.

Wasm object readers and writers reuse the shared UTF-8 decoder. Required
mutations cover overlong encodings, surrogates, out-of-range scalars and truncated
names in otherwise unknown custom sections; the independent validator rejects
each mutation. A valid non-ASCII export exercises calls, ABI lookup, quality
limits and same-instance sequences. The Perl runner normalizes JSON name keys
and values to UTF-8 bytes once. LowIR accepts and writes quoted object spellings
for Unicode, whitespace, punctuation, escapes and leading digits; PA8 owns
the reduced format checks and the contract ledger records the extension.

Qualification passes the strict report 10796/10796; PA37 105/105 with host-built
and self-built compiler, optimizer and backend tools; PA8; PA5 self-host;
byte-identical native inception; debug info; PA36 smoke 3/3; architecture,
source-set, registry, scaffold, IR, format and document audits; eight Clang 21
changed-source syntax checks; and the complete harness. Ten quality-helper and
six benchmark-helper unit checks pass. Placement scans 10134 tests with no
placement or local hygiene findings; 89 advisory EH leads and seven inherited
file warnings remain.

The final student snapshot is `student-profile-closure-final-export`, with
`profile-closure-final-reference-binaries.tar.gz`. It regenerates consistently,
passes its 275-document inventory, 65 prose checks and all 43 wrappers, and
executes all four new Wasm controls through the shipped Perl runner. All 20
changed shipped documents, scaffold, fixtures and helpers match source. An
earlier export began before new fixtures were staged; it was explicitly rejected,
terminated and removed after the replacement passed. That freed 373 MB. Its
failure and cleanup records remain in `profile-closure/`; it is not qualification
evidence.

Immutable native A/B compilation emits identical O0/O3 objects. Instruction
counts change by -0.0049% O0 and +0.0007% O3; peak RSS changes by less than 0.03%.
Four A/A and eight ABBA blocks at each level retain all 96 observations. Paired
wall changes are +0.78% O0 and +0.94% O3, while A/A changes by +1.61% and +7.36%:
this loaded-host campaign cannot resolve a small timing change. Frozen inputs,
binaries, patches and every observation are in `profile-closure/perf/`.

Subsequent review found an additional quoted-name edge: `object="$prefixed"`
is written unquoted, then rejected on a second read. The reproducer and results
are `profile-closure/quoted-edge*`. Correct that lexical classification and review
NUL object-name handling before final qualification. Remaining profile/import
hardening, source-vector lowering and full Linux/macOS CI qualification remain;
this checkpoint does not complete PA37.


## Unsigned vectors and quoted-name closure

This checkpoint follows `c8dcd6450` on unchanged `origin/main` at `e2831a513`.
Unsigned 32-bit construction, broadcasts, shifts and extraction retain the
canonical signed LowIR operands through bit-preserving copies. Constant-index
reads from vector variables now use the existing extraction path; ordinary
array reads are screened by their typed category first. Volatile accesses and
operand evaluation remain observable. Packed 32-bit multiplication uses a
target capability instead of imposing the native SSE2 restriction on Wasm.
The shared PA30 reducer checks scalar-oracle equality, high bits, side effects,
volatile storage and variable shifts; PA37 adds memory/local/code envelopes
without prescribing instructions. GCC, Clang and cppgm pass its native O0/O3
builds and runs.

LowIR quotes object names beginning with `$`, keeps their complete spelling
through another reader/writer pass, and rejects embedded null bytes instead of
silently truncating them. Constructed metadata and aliases receive the same
name validation. The required PA8 reference sidecars are included. The affected
PA33 exact-text regression reference was regenerated: unsigned shifts and
constant-index extraction no longer use temporary memory.

The final compiler passes the strict report (10798/10798), PA37 with host-built
and self-built tools (106/106), debug-info checks, PA5 self-hosting, byte-identical
native inception, PA36 smoke and architecture/format/file audits. Placement
checks scan 10136 fixtures with no placement or hygiene findings; 89 existing EH
review leads remain advisory. The complete harness passes. Changed sources
also pass Clang 21 syntax checks with warnings as errors. File audits retain
seven inherited header-division warnings.

Frozen native compilers compare the parent and this compiler change on the
same semantic-overload input:

| Compile level | Parent instructions | Candidate instructions | Change |
| --- | ---: | ---: | ---: |
| O0 | 3,058,519,385 | 3,058,346,752 | -0.0056% |
| O3 | 10,349,345,779 | 10,349,213,999 | -0.0013% |

Both generated objects are byte-identical. The immutable compilers, patches,
commands and counters are in `vector-lowering/perf-dispatch/` outside Git.

The standard O3 Wasm campaign passes all 32 independent oracle cases per
compiler/configuration. SIMD transform speed reaches 0.734x Clang (higher is
better). Final-compiler objects and modules are byte-identical to the measured
ones in both scalar and SIMD configurations; the proof, compiler hashes and
all observations are retained in `vector-lowering/`. This does not claim that
the other Wasm runtime gaps have closed.

A concurrent export/debug build exposed shared tool-output paths and failed
while replacing link temporaries. Those attempts are not qualification evidence.
The affected build, strict, debug, smoke and audit steps were rerun serially;
export runs after them. Their individual statuses and original failed logs are
retained. Subsequent qualification uses immutable compiler binaries.

Remaining required work includes explicit host-import binding facts through
source/LowIR, the final object-profile boundary checks and Linux/macOS CI.
Optional compiler/browser inception remains the separate portability project.


The next binding gap has a reduced probe in
`vector-lowering/import-binding-probe/`. The pinned LLVM compiler's
[`import_module`, `import_name` and `export_name` attributes](https://clang.llvm.org/docs/AttributeReference.html#import-module)
produce an object that our own linker accepts; Wasmtime supplies a separate
host module and the call returns 51 as expected. Compiling the same source
with cppgm currently drops those attributes: emitted LowIR has an ordinary
external declaration and the final link reports `undefined Wasm symbol: added`.
This isolates required work to typed source/LowIR binding transport and its
validation, rather than general host-function object linking. Keep the module,
field name and complete signature distinct from the linking-symbol spelling.


The final regenerated export verifies 24,416 portable reference outputs and
passes its 275-document inventory, 65-prose audit and all 43 reference wrappers
in a fresh client. The exported Perl harness runs the new vector control with
the final reference bundle; all 17 changed shipped files match. Superseded and
incomplete exports were removed after the final checks; raw logs are retained.

Frozen native wall-time qualification retains 96 observations: four A/A and
eight paired ABBA blocks at each level, with CPU affinity and output equality.
Paired candidate/parent wall medians are +0.009% at O0 and -1.195% at O3;
A/A medians are -0.586% and +0.392%. Both candidate block-bootstrap intervals
include zero, and the O3 A/A campaign has a large timing outlier. These
measurements do not establish a small native timing change. Paired peak RSS
changes are below 0.05% in magnitude. All observations and the summary remain
in `vector-lowering/perf-dispatch/`.

A separate frozen-module comparison checks 32 independent oracle cases per
compiler in each mode. Scalar module bytes are identical. SIMD module size
falls from 3274 to 3079 bytes. Four baseline A/A blocks and eight ABBA blocks
with 32,768 transform repetitions per observation measure a median 30.47x
parent/candidate speedup, with identical checksums throughout; A/A is 1.026x.
The raw engine responses, requests, commands, module hashes and all paired
ratios are in `vector-lowering/runtime-abba/`. This is the transform improvement,
not an aggregate speedup for the other kernels or compiler build throughput.


## Source/LowIR host bindings

This stage adds typed function import/export records, their reader/writer and
validation, source attributes and target-specific attribute probes. Sparse
semantic facts retain module, field and public names separately from static
linking identities. Function-template specializations preserve inherited
bindings using their own complete signatures. Public exports root optimization;
internal exports retain distinct export-facing identities without wrapper code.
The linker no longer emits an unwanted linking-name alias for a named export.

Required controls cover separate host modules, empty names, persistent i32/i64
globals, direct/indirect imported calls, imported-function re-exports, local
exports, template specializations and bad attribute arguments at O0–O3.
Mixed-producer/linker checks retain the pinned LLVM limitations explicitly:
LLD drops imported-function and local-symbol exports. Our objects link with LLD;
LLVM local exports also run through our linker, the independent validator and
Wasmtime. The shipped reference-limitations JSON records those exceptions.

Independent memory/table probes confirm that both linkers synthesize final
limits from the linked program rather than copying object import limits. cppgm
rejects reversed bounds, oversized wasm32 memories and out-of-profile shared/
64-bit limits; the pinned validator corroborates the malformed cases even where
LLD accepts them. Commands, original objects, mutated objects and every result
are in `function-bindings/limits/` outside Git. Source binding rejection probes
also distinguish Clang warnings from errors: conflicting redeclarations warn
in Clang, whereas this target contract requires agreement; numeric escapes,
non-string names and wrong arity are rejected by both.

The final source passes the strict report (10814/10814), PA37 with self-built
tools (114/114), debug-info checks, PA5 self-host, byte-identical native
inception, PA36 smoke and architecture/format/file checks. The complete harness
passes; the new binding/audit harness checks were repeated after the template
fix. Placement checks find no early-placement or hygiene issues, including a
final PA37 scan. Seven inherited file-audit warnings remain. String literal
facts retain their previous sizes: 32 bytes on native and 24 on Wasm hosts.

Frozen native compilers compare the preceding checkpoint with the final source:
O0 instructions are 3,058,429,576 versus 3,059,003,111 (+0.01875%); O3 are
10,349,592,425 versus 10,350,104,236 (+0.00495%). Both output objects are
byte-identical. The final scalar and SIMD O3 benchmark objects/modules are also
byte-identical to the previous measured campaign. Compiler hashes, patches,
counts and equality checks are in `function-bindings/perf-qualified/` and
`function-bindings/benchmark-equivalence.json` outside Git. Earlier candidate
measurements are retained separately and are not final-source timing evidence.

The full PA36 release check is running. Final paired timing, a fresh student
export and Linux/macOS CI remain pending. This checkpoint does not complete
PA37. Compiler/browser inception remains the separately documented optional
portability project.


## CI warning and external binding validation corrections

The first PR run exposed a GCC 13 vectorizer warning in the lane-duplication
selector and an untrusted container checkout in the new benchmark workflow.
The selector now fills the sixteen output bytes with an explicitly bounded
loop; the workflow trusts only its mounted workspace. The warning reproduces
with isolated GCC 13.4 and disappears with the rewrite. All 181 changed C++
files compile at O3 with warnings as errors under GCC 13 and 14. The original
loop also compiles under GCC 14; the earlier GCC-version attribution was wrong.

Review also found that function-binding validation had been reachable only
through constructed-program checks. External text now runs the same checks.
The six semantic negative PA8 references were regenerated as failures. Earlier
10814-test reports did not prove those rejections: their incorrect success
references depended on generated outputs present in that checkout.

A required bitwise lane-duplication control covers signed high bits and NaN
payload bits, at O0–O3 and through both linkers. The original and rewritten
selectors produce byte-identical objects/modules for that control at all four
levels. Scalar and SIMD standard benchmark objects/modules remain byte-identical
to the measured campaign. Control expectations live in `.wasm-expect`; redundant
control result files were removed from the source tree. The placement auditor
now recognizes named shared WAT host modules and still rejects orphan helpers.

The corrected source passes 10815/10815 strict tests, 115/115 PA37 controls with
self-built tools, debug info, PA5 self-host, byte-identical native inception,
PA36 smoke and architecture/format/file checks. The corrected harness passes.
Native compiler instructions versus the previous binding checkpoint differ by
nine instructions at either level; output objects are identical. A separate
immutable build of current main (`e2831a513`) measures the whole Wasm branch:
O0 is 3,051,970,129 versus 3,058,929,091 (+0.228%); O3 is 10,281,332,042 versus
10,349,556,795 (+0.664%). Both native outputs are byte-identical. Raw commands,
compiler/source hashes and observations are in `native-final-main/` and
`function-bindings/ci-fixes/` outside Git.

The earlier PA36 release attempt was stopped when the compiler source needed
correction; its partial results do not qualify this source. Fresh export,
full release and Linux/macOS CI remain pending. The final scope review must
also close the planned per-child memory/output bounds: the existing Wasm
checker has timeouts, but its direct subprocess captures do not set those
resource limits. This remains required harness work, separately from optional
compiler/browser inception.

## Resource bounds and generated-test closure

`e256ee7db` repairs nested StoreMotion analysis before it can read invalidated
loop facts, and renames private Wasm variadic selector methods that collide
with libc++ macros. The reduced PA33 regression executes 108 input pairs.
The old CI optimizer has a Memcheck invalid read on that input; the fixed
optimizer is clean. Scalar, SSSE3 and AVX2 Godot translation-unit replays pass.
The fix preserves loop promotion and removes the former temporary region vector.

`4e52299e1` supplies deterministic Perl CFG/ABI generators with independent
arithmetic, four required generated controls, the machine-boundary representation
probe, and real host/guest resource ceilings. The new native worker protocol
preserves arbitrary non-NUL argument/environment bytes and empty arguments.
Only generic limited commands use RSS sampling; ordinary compiler requests
retain their existing path. CPU limits are per process, file limits per file,
and sampled RSS is summed across the child process group. The guest memory
limiter does not change Wasmtime's reservation, guard or JIT settings.

`7abffb9fe` fixes a race exposed by the first CI run: the supervisor could
inspect an earlier full capture before the next child truncated it. A nonblocking
setup pipe now distinguishes successful exec from setup failure and delays
capture-size inspection until setup finishes. Wall/CPU/RSS bounds remain active
during setup. A deterministic FIFO regression fails the old supervisor and
passes the fix. The final resource suite has 12 tests, including real ceilings,
cancellation, descendants, valid guest exits and subsequent healthy requests;
the existing 17 timeout tests also pass.

The final compiler passes 10821/10821 strict tests and 120/120 PA37 controls
with self-built tools, debug info, PA5 self-hosting, byte-identical inception,
PA36 smoke, the complete harness and architecture/format/file audits.
Placement scans 10159 fixtures with no early-placement or hygiene findings;
89 existing EH review leads remain advisory. Seven inherited file-audit
warnings remain. Evidence is in `resource-qualification/final/` and
`resource-qualification/capture-race/qualification/`; the latter repeats the
affected checks after the setup-race correction.

Immutable native compilers compare final `7abffb9fe` with the preceding
`83dc85543` checkpoint and main `e2831a513` on the same frozen input:

| Level | Main instructions | Prior checkpoint | Final compiler | Final versus main |
| --- | ---: | ---: | ---: | ---: |
| O0 | 3,051,855,703 | 3,059,003,124 | 3,059,052,729 | +0.2358% |
| O3 | 10,281,696,850 | 10,350,104,295 | 10,349,976,042 | +0.6641% |

All three compilers emit identical native object bytes. The follow-up changes
alone are +0.00162% O0 / -0.00124% O3. Four A/A and eight ABBA blocks per
level retain all 96 timing observations. Paired final/prior wall changes are
+0.455% O0 and -0.782% O3; both block-bootstrap intervals include zero.
A/A is +0.227%/-1.643%, with substantial O3 variation. Paired RSS changes
are below 0.06%. This does not establish a small timing change. Hashes,
commands, counters, objects and observations are in
`resource-qualification/native-perf-final/`.

The local O0/O3 Wasm campaigns and the
[CI campaign](https://github.com/vishvananda/cppgm-extended/actions/runs/43239133413)
pass all 32 independent oracle cases per compiler/configuration. The CI artifact
is downloaded and SHA-verified; its standard report is reproduced locally.
All engine/resource settings and observations are retained. Standard publication
is main-only; this PR does not publish a new badge revision. Final release,
export and host/application qualification are recorded below when complete.


## Darwin supervision and release qualification

`953836b1c` keeps process-group RSS enforcement compatible with native
self-hosting on macOS. Including `libproc.h` brought unrelated Mach message
syntax into the SDK header closure; both LLVM 18 and LLVM 21 CI builds passed
the full report and resource controls, then failed while self-compiling that
header. The supervisor now declares the two required libproc functions using
`sys/resource.h` types: `proc_listpgrppids` supplies process IDs and
`proc_pid_rusage` supplies resident sizes. Counts, errors and disappearing
children retain explicit handling. The narrow declarations match Apple's XNU
headers and wrappers. Linux preprocessed source and the rebuilt compiler are
byte-identical before and after this Darwin-only change; the Linux binary SHA256
is `e6a02297f96b521534ab56fa2f00da28ab040f98912b96a265891b400ae9c64c`.
The existing Linux qualification and frozen measurements therefore still apply.

The full local PA36 release run completed all 100 generated seeds and passed
all ordinary gates in both course and student configurations. Student scaling
was 2.48, however, giving only 20.97% headroom against the 3.0 limit instead of
the required 25%; that release attempt failed. A predeclared set of exactly
three pinned speed-group trials required all three to pass. Two passed, but
the third failed with members scaling 3.71 and a sharp increase in measured
large-input time. Every original and follow-up observation is retained; the
passing trials do not qualify the release. Deterministic and other release
margins passed in the original complete run.

The new PA36 release workflow runs the unchanged complete calibration with
100 seeds on a GitHub runner and retains raw results and measured binaries
even on failure. It preserves all gate limits and release margins. The fresh
[release run](https://github.com/vishvananda/cppgm-extended/actions/runs/43247726609)
and [macOS self-host run](https://github.com/vishvananda/cppgm-extended/actions/runs/43247611125)
remain pending at this checkpoint.


The fresh export at this checkpoint regenerates all tracked references and
debug-info references without differences. Its document audit passes 65
reviewed prose files and the 275-file inventory; the freshly packaged binary
bundle installs successfully through all 43 wrappers. All 120 PA37 controls
pass through those exported wrappers. The 29 changed shipped files are
byte-identical to the current source, including the deterministic generator
and Darwin supervisor fix. Maintainer plans, the representation-review journal
and the local handoff are absent. Evidence is retained in
`resource-qualification/export*.log`, `export-final-status.json` and
`export-shipped-equality.json`.


The complete [Godot qualification](https://github.com/vishvananda/cppgm-extended/actions/runs/43239133589)
now passes on the Linux compiler with the StoreMotion fix: all three compilers
build the unchanged full headless engine, and all five checked workload
families agree with their independent expected results. This closes the original
application-level reproducer as well as its reduced PA33 control. The
[index-cpp campaign](https://github.com/vishvananda/cppgm-extended/actions/runs/43239133306)
also passes all 50 checksums with cppgm, GCC and Clang. Both artifact archives
are downloaded, SHA-verified and retained with provenance and every raw
observation under `resource-qualification/{godot,index}-ci/`. These campaigns
have their own A/A variation and do not establish a small native regression
relative to main.

Post-export architecture/format/scaffold/IR audits, file audit, the entire
harness and placement audit also pass at this source revision. The placement
counts remain 10159 fixtures, zero early-placement/hygiene findings and 89
inherited advisory EH leads. Results are in
`resource-qualification/post-export-status.json`.


The upstream Clang CI retry again failed during Ubuntu snapshot package setup
with HTTP 500/503, before checkout or compiler tests. Both current-head upstream
jobs also hit that outage. Their setup logs are retained. A subsequent GET of
the exact pinned Clang package succeeded with HTTP 200, after which the failed
current-head jobs were retried without changing pins.

Independent local validation uses the same clean upstream revision
`2078da43e25a4623cab2d0d60decddf709aaea28`, Clang 21.1.8 and the exact current CI
manifest options. All 562 native qualification cases pass; both cppgm and Clang
pass all 815 selected language configurations (310 acceptance and 505 required
diagnostics). All 15465 source/RUN exclusions remain explicit; this is not a
claim of full Clang-suite coverage or runtime parity. The local Ubuntu host is
recorded with its package inventory and is distinct from the pinned CI
container. Frozen binaries, manifests, commands, source hashes, raw outputs
and status are in `resource-qualification/upstream-clang-local/`. This validates
the compiler while leaving the CI setup outage visible.


The complete [PA36 CI release](https://github.com/vishvananda/cppgm-extended/actions/runs/43247726609)
passes all 17 gates and every release margin in both course and student
configurations, with 100 generated seeds each. Course/student scaling is
2.05/2.06, giving 46.34%/45.63% headroom against the unchanged 3.0 limit;
the required headroom is 25%. Student O0/O3 compile-time ratios are
0.1479/0.1594 and peak-RSS ratio is 0.4341. Instruction, cycle-proxy, text
and kernel ratios also clear their required margins.

The measured merge revision is `e22f2294eaba03fb20fce1c2de9595b4d821b1c6`
(PR head `33ea331f4`, unchanged main `e2831a513`). The full evidence and
measured binaries are downloaded and SHA-verified in `resource-qualification/pa36-ci/`;
the artifact digest is
`a4b02058652542d025519c6b1177bee65a54f0f1d8241f5564b2f84cdbb25bed`.
Both generated calibration tables are copied byte-for-byte into their owning
PA36 paths, as required by `TESTING_AND_REFERENCES.md`. The tables retain
empty TSV fields and the actual CI provenance. Earlier failed local measurements
remain preserved separately and are not relabeled as passes.


A second fresh export includes the newly generated student calibration. It
again regenerates all references without differences, passes the 275-file
inventory and 43 bundle wrappers, and passes all 120 exported PA37 controls.
All 30 changed shipped files match the source byte-for-byte. Its complete
logs, binary bundle and status are retained separately in
`resource-qualification/release-export/`.

The final [Linux matrix](https://github.com/vishvananda/cppgm-extended/actions/runs/43247726300)
passes all four GCC/Clang and Ubuntu 24.04/26.04 builds, cumulative reports,
debug-info checks, PA5 self-host ladders and optional target-feature tests,
as well as source and placement audits. Current-head Wasm, internal C++,
simdjson and export CI also pass.


## Completed PA37 acceptance

Both [Intel macOS jobs](https://github.com/vishvananda/cppgm-extended/actions/runs/43247611125)
now pass with LLVM 18 and LLVM 21: 10786/10786 platform tests, all 12 resource
controls, optional target features, and byte-identical self-built/inception
objects and compiler binaries. This qualifies the narrow Darwin process API
fix on its affected platform. The complete logs are retained as
`resource-qualification/macos-final-llvm18.log` and `macos-final-llvm21.log`.

The implementation sequence's required exit criteria are satisfied: independent
Wasm behavior and quality checks, both LLVM interoperability directions,
shared-machine representation review, all native regression/performance gates,
Linux/macOS reports and inception, resource bounds, and a clean student export.
The checked CI merge tree is identical to branch revision `33ea331f4`.
Subsequent committed changes contain only the exact generated calibration
tables and their documentation; the compiler has not changed since platform
qualification. The full compiler running inside Wasm/browser remains the
separate optional project described above.

The additional Boost campaign and retried upstream CI jobs were still running
when this closeout was recorded. They are not labeled as passes. Full Godot,
index-cpp, simdjson, internal C++, Wasm, Linux test and export campaigns have
passed; the equivalent local Clang language cohort also passes, with the
snapshot setup failures and retries retained explicitly above. This closes
PA37 for review and does not merge or publish the branch on main.
