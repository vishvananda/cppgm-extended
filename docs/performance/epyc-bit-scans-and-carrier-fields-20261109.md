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
with no parameter store, reload or unused frame reservation.

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
also runs it at O1–O3, checking that eleven eligible callees need no object
memory traffic or frame storage. It accepts different instructions and
registers. Mutation, escaped storage, volatile reads, calls, unaligned/narrow
fields, multiple uses, two-eightbyte objects, both branch paths and unsupported
carrier shapes have execution checks. Student notes describe the general
coverage/lifetime/flags problem; PA24 does not acquire an optimized-code
requirement from this follow-up.

## Compiler and public-workload cost

Immutable normal release compilers compiled the same three frozen preprocessed
inputs at O0 and O3. Native instruction counters had at least 99.5% coverage;
A/A and paired blocks retain every observation. Changes are candidate/main:

| Frozen input | Compile instructions, O0 | Compile instructions, O3 |
|---|---:|---:|
| compiler-loop | +0.0026% | -0.0181% |
| frozen-semantic | +0.0033% | -0.0282% |
| boost-flat-map | -0.0091% | -0.0530% |

The corresponding LowIR dumps match byte for byte at both levels. Native
objects are deterministic within each compiler. Local wall timings were
contaminated by concurrent builds and are not used as a speed claim.
A separate quiet-VM compiler timing run is retained alongside the counters.

All 81 public O3 programs match GCC's exit codes and output hashes. Seventy-six
candidate executables are byte-identical to main. Changed programs are Bullet,
fmt, re2, re2-test-re2 and re2-test-search. Native instruction changes are
negligible except fmt at +0.199%; this is not a claim about sub-percent timing.

## Validation and evidence

Strict report: 9997/9997. PA33 course checks, debug-info, self-host through PA5,
PA35 smoke, architecture/placement/format/file audits, harness checks, complete
student export and its 42 fresh-client reference wrappers pass. The existing
return-from-halves regression reference was regenerated because two unused
frame reservations disappeared. No unrelated references were changed.
Full PA35 release calibration and Godot profiling are recorded below when complete.

Raw inputs, hashes, commands, all observations and rejected prototypes are under
`/mnt/vizier/cppgm-codegen-data/20261109/epyc-sse-throughput/`.
The frozen candidate compiler SHA256 is
`2116426a79b0aad6c156b3f37308dea62914765b98940ccbcd1b35bd1581f772`.
Relevant files include `final-metadata.json`, `phase-baseline/`,
`repbsf-screen/`, `repbsf-references/`, `software-profiles/`, `counts/`,
`compile-cost/`, `public-sanity/`, and the validation logs.
