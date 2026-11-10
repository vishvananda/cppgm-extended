# EPYC trailing-bit scans and object-carrier fields

Measured 2026-11-09 against main `9ca649bf59cb1e14979655d1669ade353a9ae881`.
This is maintainer investigation material, outside the student export.

## simdjson SSE

The gap was concentrated in structural scanning. On a quiet EPYC 7B13
(Milan) VM, main took about 15–17% more CPU time than GCC in stage 1,
while stage 2 was much closer. A fixed GCC caller, unmodified pinned
simdjson `40a686ab89dc5ee58ec10d0514dd5c99abbc472b`, and identical
compiler flags separate library code generation from caller differences.
The reference toolchains are GCC 15.2 and Clang 22.1.2; CI uses Clang 21.1.8.

Repeated vector stack loads are one inefficiency, but caching them in an
unused vector scratch improved whole parsing by only 0.6–1%. That additional
optimizer pass was discarded. Software instruction-pointer profiles pointed
instead to the loop enumerating structural-mask bits.

cppgm emitted ordinary BSF for trailing-zero counts; GCC and Clang emitted
REP BSF. This encoding executes TZCNT on supporting CPUs and retains the
nonzero BSF result on older CPUs. LowIR permits an unspecified result for zero,
and the typed count lowering does not consume the instruction's flags.
The change applies to the operation generally, without workload recognition,
CPU-model checks, padding rules or a new optimizer pass. BSR is unchanged:
prefixing it would change nonzero results.

[uops.info's BSF measurements](https://uops.info/html-instr/BSF_R64_R64.html#ZEN3)
report three-cycle reciprocal throughput and six executed micro-ops on Zen 3.
The earlier investigation update's three-micro-op figure was incorrect.
[LLVM's emission rule](https://llvm.org/doxygen/X86MCInstLower_8cpp_source.html)
and [GCC's original change](https://gcc.gnu.org/legacy-ml/gcc-patches/2012-04/msg01855.html)
both describe the compatible prefix and the need for dead flags.

All ratios below are reference CPU time / candidate CPU time; higher is better.
One A/A block before and after three paired ABBA/BAAB blocks, affinity to one
CPU, adaptive repeat counts, and full canonical JSON equality checks were used.

| Input | Main vs GCC | Candidate vs GCC | Candidate vs Clang 22 | Candidate vs main |
|---|---:|---:|---:|---:|
| twitter.json | 0.901× | 1.030× | 1.029× | 1.141× |
| citm_catalog.json | 0.902× | 1.031× | 1.032× | 1.142× |

Stage 1 improves by 21.8% on Twitter and 22.8% on CITM. The direct-reference
whole-parse A/A controls varied by less than 0.5%. The scalar library object is
byte-identical to main. The combined field-extraction candidate produces
exactly the same scalar and SSE objects as the isolated prefix experiment.
Linked phase executables have identical disassembly and addresses; their ELF
file hashes differ in non-code metadata, so they are retained separately.

The first PR run on 2026-11-10 used an EPYC 9V45, GCC 15.2 and Clang 21.1.8.
[Run 42732808847](https://github.com/vishvananda/cppgm-extended/actions/runs/42732808847)
tests merge commit `4718037818c46f15a769fc0fdc44b03f315a198d`, containing
branch tip `1f6cb7116`. All canonical JSON checks pass:

| Input | CI SSE vs GCC | CI SSE vs Clang 21 |
|---|---:|---:|
| twitter.json | 0.950× | 0.963× |
| citm_catalog.json | 0.978× | 1.004× |

These are a separate host/toolchain comparison, not a repeat of the Milan
measurement. The SSE Twitter/Clang A/A controls vary by up to 6.1%, so small
differences need confirmation. The unchanged scalar path measures
1.058–1.066× GCC and 1.032–1.052× Clang on this host. Raw CI results are saved
in `pr-simdjson-results/` alongside the local evidence.

The VM has no usable hardware PMU. The native timing and cpu-clock profiles
are independent of the local Callgrind instruction/memory counts. In those
instrumented phase runs, use the `totals:` record: Callgrind 3.26's header
`summary:` underflowed after explicit zeroing. Summed exclusive instruction
costs independently verified the totals. Similar instruction counts concealed
both the repeated stack reads and the more expensive bit-scan encoding.

## Direct object fields

`copyobj` plus a field address/load remains the LowIR contract. Existing ABI
parameter/local storage sharing and register-home dataflow already expose a
covering integer carrier; the backend can extract a contained subrange.
An upper signed 32-bit field now needs a copy and arithmetic right shift,
with no parameter store, reload or unused frame reservation. Interior
extractions are committed only when the shared frame/ABI proof shows that
materialization can disappear. This matters: the initial broad rewrite
increased fmt's instructions by 0.199% and timed about 1.3% slower in a
noisy local paired run. The bit-scan-only build was byte-identical to main
for fmt. The final profitability guard restores that exact executable.
No benchmark name, source pattern or CPU-model exception enters the policy.

The analysis still meets predecessor facts and invalidates them for overlapping
writes, aliases, calls and register clobbers. New extraction is limited to
integer bytes within one known GPR carrier. A bounded flags check precedes
shifts, and a still-live carrier is copied. Volatile reads, cross-carrier
loads and unsupported SSE/mixed shapes retain the existing path. Debug,
exception, dynamic-stack and ABI constraints still govern frame removal.
No new LowIR or MIR vocabulary, frontend special case or environment switch
was added. Emission/insertion helpers keep the existing dataflow module
within the repository's file-size limit.

The PA24 fixture checks basic object semantics at O0. PA33's maintainer lane
also runs raw and optimized LowIR at O0–O3, checking that eleven eligible callees need no object
memory traffic or frame storage. It accepts different instructions and
registers. Mutation, escaped storage, volatile reads, calls, unaligned/narrow
fields, multiple uses, two-eightbyte objects, both branch paths and unsupported
carrier shapes have execution checks. A retained-frame case bounds total
MIR work without prescribing opcodes or registers; it catches an extraction
that adds work while an escaped address still requires storage. The private
checker is explicitly omitted from the export. Student notes describe the general
coverage/lifetime/flags problem; PA24 does not acquire an optimized-code
requirement from this follow-up.
The original `obj<8x4>` Point reproducer also passes all eight field/level
executions; its optimized upper field is a register copy and arithmetic shift,
with zero frame storage. These inputs and results are in `projection-original/`.

## Compiler and public-workload cost

Immutable normal release compilers compiled the same three frozen preprocessed
inputs at O0 and O3. Native instruction counters had at least 99.5% coverage;
A/A and paired blocks retain every observation. Changes are candidate/main:

| Frozen input | Compile instructions, O0 | Compile instructions, O3 |
|---|---:|---:|
| compiler-loop | +0.0027% | -0.0292% |
| frozen-semantic | +0.0103% | -0.0395% |
| boost-flat-map | -0.0094% | -0.0582% |

The corresponding LowIR dumps match byte for byte at both levels. Native
objects are deterministic within each compiler. Local wall timings were
contaminated by concurrent builds and are not used as a speed claim.
The final rewrite walks reachable blocks in instruction order after the
available-carrier fixed point converges. Their independent input rows are
unchanged, so insertion prefixes arrive ordered without a sort or sort
scratch. A cheap ABI check rejects obviously retained frames before planning
interior extraction. The remaining plans use the same storage-elision proof
as final frame cleanup; unsuccessful plans restore the original loads.

Earlier quiet-VM timing runs belong to rejected broad candidates and are
retained separately. Final instruction-count changes above are tiny;
the projection analysis runs only at O1 and above.

All 81 public O3 programs match GCC's exit codes and output hashes. Seventy-eight
candidate executables are byte-identical to main, including fmt and Bullet.
Only re2, re2-test-re2 and re2-test-search change; their native instruction
ratios are within 0.003% of main. The final scalar and SSE objects are
byte-identical to the independently timed compatible-prefix experiment.

## Godot follow-up

Fresh main CI results confirmed that the Godot gap was not merely a stale
badge. Two clean full Godot builds, using immutable main and candidate
compilers, both pass. The pinned source, C compiler, linker and CI flags
are identical; GCC 15 and Clang 22 reference binaries use the same upstream
revision. All five independent GDScript checksum oracles pass on every binary.
The following Milan timing table is from the initial broad field candidate,
before the profitability guard, and is diagnostic rather than a claim about
the final branch. Three paired blocks include A/A controls:

| Workload | Candidate vs main | Candidate vs GCC | Candidate vs Clang 22 |
|---|---:|---:|---:|
| dispatch | 1.068× | 1.066× | 0.942× |
| arrays | 0.994× | 0.832× | 0.935× |
| strings | 0.989× | 0.872× | 0.881× |
| geometry | 1.031× | 1.108× | 1.000× |
| scene | 1.082× | 0.996× | 0.899× |

Do not treat the apparent dispatch/geometry/scene gains as an established
algorithmic improvement. The core and GDScript objects are byte-identical to
main; 31 of 2337 objects change, mainly rendering/physics dependencies.
Some controls are noisy: geometry's Clang after-control differs by 9.2%,
scene's main after-control by 4.4%, and several others exceed 1–2%.
The arrays and strings gaps against the external compilers remain substantial.
Their candidate/main changes are small (about −0.5% and −1.1% CPU speed),
with no claim that this patch improves them. Native software profiles identify a separate gap from simdjson's bit enumeration.
The arrays run spends 66.5% in `GDScriptFunction::call`, 9.5% in packed-array
get, 8.5% in validated set and 7.3% in Variant assignment. GCC spends 64.6%,
8.3%, 10.0% and 6.4% respectively. Strings split their work among the
interpreter, `String::sprintf`, append/resize, integer formatting and libc
allocation. These are sampling shares, not per-instruction hardware counts.

Godot links with `-s`, so raw profiles initially lack engine symbol names.
Relinking the same objects without stripping produces an identical `.text`;
its symbol map resolves the original sampled addresses. Neither timed
executable was replaced with a symbolized build.

One concrete form to investigate is indirect dispatch: cppgm's sampled
interpreter loop loads a target into a register and jumps through it, while
GCC uses a memory-indirect jump. The existing encoder accepts memory targets.
Folding needs a proof that the target register is dead at every successor,
that its address survives, and that the load's effects/order are preserved.
A mere adjacent instruction pattern is insufficient when the value is live
past the jump. Its performance contribution remains unmeasured.

The next optimization round should:

1. Reduce interpreter operand decoding and packed-array get/set to small
   fixed-caller workloads, preserving dynamic indices, bounds and Variant
   type/error behavior.
2. Compare native work and dependency chains with GCC and Clang. Inspect
   repeated decoding, sign extensions, address calculation, dispatch and
   register spills before attributing the difference to one EPYC instruction.
3. Isolate string append/resize and formatting separately from libc allocation,
   with equal allocation/output behavior and sufficient repeated work.
4. Implement general typed-dataflow or instruction-selection improvements,
   add independent reducers, and check the public suite and self-build cost.
   Keep interpreter opcode numbers and benchmark identities out of policy.
5. Repeat paired timing on the Intel and Milan lanes, retaining noisy runs
   and checking several linked layouts before claiming a small gain.

Godot Callgrind runs did not complete within their 180-second limit and
provide no usable instruction totals. They are not included as evidence of
an instruction improvement. Native timing observations are complete and
were saved before the VM's one-hour automatic stop; profiling uses a later
boot, kept separate from those observations.

## Validation and evidence

Strict report: 9997/9997. PA33 course checks, debug-info, self-host through PA5,
PA35 smoke, architecture/placement/format/file audits, harness checks, complete
student export and its 42 fresh-client reference wrappers pass. The existing
return-from-halves regression reference was regenerated because two unused
frame reservations disappeared. No unrelated references were changed.
The final compiler passes all 17 PA35 gate limits in both course and student
configurations. The full run's course scaling measurement, 2.61×, falls short
of the extra 25% release margin against the 3.00× limit. Its template family
takes approximately 100 ms, near the probe's noise threshold. An A/A and ABBA
diagnostic retains unchanged-main and candidate observations at that boundary.
One complete official course speed-group repeat, pinned to CPU 82 and using
the identical immutable compiler, measures 2.12×; the student build measures
1.79×. The release evaluator then passes 17/17, including every margin.
The original full run, repeat, rejected candidates and generated tables are
all retained. `qualified-with-speed-repeat/provenance.json` identifies the
measurement inputs; `qualified-release-evaluation.log` records the final
evaluation. No gate, margin or checked-in calibration table changes.

Raw inputs, hashes, commands, all observations and rejected prototypes are under
`/mnt/vizier/cppgm-codegen-data/20261109/epyc-sse-throughput/`.
The frozen candidate compiler SHA256 is
`7c2ee8342cbeeed0b5a6c581195ff6449f56b847660fed5db4163c973f1fe2e8`.
Relevant files include `profit-metadata.json`, `phase-baseline/`,
`repbsf-screen/`, `repbsf-references/`, `software-profiles/`, `counts/`,
`profit-compile-cost/`, `profit-public-sanity/`, `godot-native/`,
`godot-symbolized-profiles.json`, and the validation logs.
