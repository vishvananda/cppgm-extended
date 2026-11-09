# Scalar and indirect-dispatch optimization, 2026-11-09

At qualification, the final local candidate reached the 0.95× GCC target for both workload
aggregates. The follow-up below records a later scalar regression and the limits
of that local result. These are local EPYC measurements, not replacements for CI results.
All ratios below are reference CPU / cppgm CPU; higher is better.

| Workload | GCC | Clang 22 |
|---|---:|---:|
| simdjson scalar: Twitter | 1.0471× | 1.0990× |
| simdjson scalar: CITM | 0.9565× | 1.1784× |
| simdjson scalar: equal-weight geometric mean | 1.0008× | 1.1380× |
| Godot: dispatch | 1.0728× | 0.9118× |
| Godot: arrays | 0.7970× | 0.9142× |
| Godot: strings | 0.8525× | 0.8415× |
| Godot: geometry | 1.2952× | 1.0181× |
| Godot: scene | 0.9437× | 0.9145× |
| Godot: equal-weight geometric mean | 0.9772× | 0.9183× |

Individual Godot workloads still leave substantial room, especially arrays and
strings. Meeting the aggregate target does not imply parity for every workload
or on other EPYC generations.

## Changes

- Split fixed fields from a local aggregate when bounded indexed accesses can
  touch only disjoint storage. Close overlap components before promoting fields;
  escaping addresses, whole-object operations and unknown ranges remain barriers.
  Allocate extent storage lazily and sort/merge ranges rather than compare every
  access pair.
- Preserve the complete destination set of indirect jumps in MIR. Forward home
  reloads using all those edges while preserving addressed block entries.
- Separate memory/immediate bit tests from branches at O2+, preserving access
  width. Prefer register/memory integer comparisons and reverse their predicates
  when reversing operands.
- Align indirect entries using the existing bounded loop-alignment policy.
  Include padding in branch-distance bounds and align aliased code positions once.
- Remove a preceding integer mask when a right shift discards all masked bits;
  preserve observable sign bits and clear exactness assumptions that no longer hold.

These decisions use typed IR, alias/range proofs and instruction forms. They do
not inspect workload names or match benchmark source text. AMD's
[EPYC 7003 optimization guide, §2.9.3](https://www.lsferreira.net/public/knowledge-base/x86/upos/amd_zen3.pdf)
documents the comparison/branch fusion restrictions and preferred compare operand
order that motivated the instruction-selection experiments.

Broader materialization of memory/immediate comparisons regressed simdjson and
was withdrawn. Additional entry padding also regressed individual workloads and
was withdrawn. Addressed-function dead-store elimination produced no VM code
improvement, so its extra analysis was not retained.

## Measurement and validation

The host was a Google Cloud c2d VM reporting AMD EPYC 7B13, with SMT disabled.
Runtime processes used one pinned CPU, frozen executables and inputs, A/A checks
before and after paired ABBA blocks, and checked outputs. Godot used five blocks
per baseline, two A/A blocks at each end and at least one second of work per run.
The final scalar screen used three blocks, one A/A block at each end and at
least 0.6 seconds; it also compared directly against the previously qualified
scalar binary, measuring 1.0100× on Twitter and 0.9975× on CITM.

Godot was rebuilt completely with the final compiler. Earlier VM-only object
replacements were screening experiments and are not the final measurements.
GCC and Clang used the same pinned source/configuration as the candidate.
Godot CPU includes startup; measured work-loop elapsed time is retained separately.
The Clang baseline here is version 22, which can differ from other CI toolchains.

The data directory is `/mnt/vizier/cppgm-codegen-data/20261109/workloads`.
`codegen-final-order-results.tar.gz` retains the final runtime observations,
calibration, checksums and compiler/input identities. Immutable compiler binaries
are in `../20261108/bin`; build commands and source hashes are in its
`availability/*-metadata.json`. `qualified-order-cost` retains the final frozen
compiler-input measurements: O3 instruction cost increased about 0.6–0.74% for
the GCC-built compiler and 0.4–0.43% for its rebuilt self-hosted counterpart.
Shared-host wall timings varied; these are compile-cost screens, not CI claims.

Validation passed the strict 9,862-test report, debug checks, self-host through
PA5, inception identity, PA35 smoke and instruction-count kernels, optional
assembly tests, architecture/placement/file/format audits and the test harness.
The new predicate checksum agrees with GCC and Clang at O0 and O3. Student export
and the fresh-client installation of all 42 reference wrappers passed.

The broader PA35 release check passed 16 of 17 gates on its earlier candidate;
its members-scaling timing was 3.02 against 3.00. An isolated final-candidate
rerun measured 1.98, versus 2.06 for the immutable baseline. No gate or
calibration threshold was relaxed.

Boost's normal CI lane retains the 1,818 declarations in the existing 17 suites.
Local validation passed with all three compilers, including the common,
hash-recorded debugger and enum portability adaptations. Unchanged suite results
were retained while affected suites were rerun; these local combined campaigns
are validation-only. Clean CI runs produce the publishable performance results.

## Follow-up: why the qualification did not describe current CI

The qualified compiler was `b7e1f8d5f`; the later CI compiler was `238923cec`.
Current main's compiler also includes the coincident-block-label correction
from `36d65c53c`. The subsequent badge and Boost runner changes do not change
the compiler. Two different effects need to be separated:

| Workload, vs GCC | Original GCloud qualification | Qualified compiler in CI | Later compiler in CI |
|---|---:|---:|---:|
| simdjson scalar | 1.0008× | 1.0124× | 0.8349× |
| Godot | 0.9772× | 0.8706× | 0.8968× |

The earlier [final SSE qualification of 0.965× GCC](simdjson-20261107.md#guarded-integer-normalization)
was measured on the Intel Xeon, separately from this GCloud scalar campaign.
It is not an EPYC SSE qualification. The later EPYC SSE aggregate is 0.9053×
GCC; the recovered hosted Intel result is 0.9474× on an 8573C. The remaining
EPYC SSE gap is separate from the scalar regression isolated below.

The simdjson gain initially reached CI and was subsequently lost. The Godot
host gap was already present with the qualified compiler; later merges did not
create it. These are the retained
[scalar](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/simdjson/history/b7e1f8d5fa08e26637f4de8e241bb27cace292af/epyc/report.md)
and [Godot](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/godot/history/b7e1f8d5fa08e26637f4de8e241bb27cace292af/epyc/report.md)
qualified CI results. CI used EPYC 7763 for scalar and EPYC 9V74 for Godot;
GCloud used EPYC 7B13. GCloud's Clang baseline was version 22; CI used 21.
These EPYC hosts span generations: [Google identifies 7B13 as Milan](https://docs.cloud.google.com/compute/docs/cpu-platforms),
while [AMD identifies 9V74 as a 9004 deployment](https://www.amd.com/en/developer/resources/technical-articles/2026/low-latency-cpu-inference-on-amd-epyc-processors.html).

### Same-host rerun

The original GCloud VM was restarted for paired tests of frozen binaries,
using its original inputs, loader, libraries and GCC/Clang baselines.
The two scalar library object hashes match their corresponding CI objects
exactly: `6910c68763df…` for the qualified compiler and `a88e1f5f8395…`
for the later compiler. Godot was rebuilt completely with the current compiler.
Every run checked its output; A/A observations bracketed paired ABBA blocks.

| Workload | Current / qualified speed | Current vs GCC | Current vs Clang 22 |
|---|---:|---:|---:|
| simdjson scalar: Twitter | 0.8818× | 0.9215× | 0.9655× |
| simdjson scalar: CITM | 0.7886× | 0.7556× | 0.9328× |
| simdjson scalar: geometric mean | 0.8339× | 0.8344× | 0.9490× |
| Godot: geometric mean | 1.0005× | 0.9424× | 0.8888× |

Godot's aggregate is unchanged by the later compiler changes. Individual
results vary: arrays improved about 5%, while geometry fell about 4%.
The original Godot binary and all reference/workload hashes are unchanged,
but today's reference-relative aggregate is lower than the earlier 0.9772×
point estimate. Its qualification margin was therefore not durable across
sessions. These experiments do not establish how much of that difference comes
from physical host placement, virtualization or other execution conditions.
Maximum Godot A/A deviation in this rerun was 3.3%; the scalar regression
campaign's maximum was 0.51%.

### Scalar cause: code placement after return-register preferences

Removing only the return-register preference introduced in `5a1ccaf2c`
recovers 13.7% on Twitter and 25.8% on CITM, matching the qualified binary
within about 0.2%. Removing only that commit's cold-edge inlining policy does
not recover the loss. This isolates the triggering allocation change, but
does not show that its changed arithmetic operations themselves are expensive.

A relocation experiment establishes the layout mechanism. Relink the **same
current library object and driver object**, inserting an unused 32-byte NOP
object before the library. No extra instructions execute in the benchmark:

| Current compiler output | Twitter speed vs ordinary link | CITM speed vs ordinary link |
|---|---:|---:|
| Unused 32-byte prefix | 1.1320× | 1.2609× |

The relocated current binary matches the qualified binary within about 0.5%.
A separate control with the cold inlining policy removed has the same
slowdown; shifting it by 32 bytes also recovers performance, while shifting it
by 64 bytes stays slow. The hot stage-1 function moves from address `0x8770`
to `0x8790`, the qualified binary's address. This is a diagnostic experiment,
not a proposed padding optimization.

Cachegrind reports essentially identical dynamic instructions and branch
counts before and after the regression: about 102.82 million instructions
for eight Twitter iterations and 194.04 million for eight CITM iterations.
An explicit cache simulation also shows essentially unchanged cache misses.
The evidence points to address-sensitive instruction delivery or branch
prediction beyond that model, rather than additional compiler-generated work.
Hardware PMU events are unavailable on this VM, even with root access, so the
specific processor mechanism remains unmeasured.

### Next optimization and validation

1. Isolate the hot fetch/branch regions affected by placement, using multiple
   relocations and counters on an EPYC host that exposes them when available.
   Preserve the useful return-register coalescing where it improves instruction
   selection; do not choose a prefix that happens to favor this program.
2. Improve general function/block placement or hot-region layout. Evaluate
   bounded alignment, branch layout and cold-code separation using typed MIR
   facts. Compare several placements on both Intel and AMD, including size and
   compile-cost checks and individual-program regressions, before selecting a policy.
3. Requalify both workloads against GCC and Clang at the final merged compiler
   revision. Repeat the existing public and self-build performance checks.
   Treat 0.95× as an acceptance target across the intended hosts and tested
   placements, rather than a successful local point estimate.

Raw evidence is retained under the data directory above:
`codegen-gap-20261109.tar.gz` contains five paired campaigns;
`epyc-main-layout32.tar.gz` contains the direct current-object relocation test.
`scalar-gap-cachegrind/` and `scalar-gap-cache-model/` retain the instruction,
branch and cache simulations. Commands, executable/input hashes, individual
observations, A/A results and output checks are included. The diagnostic
compiler and linked executables remain frozen beside the earlier candidates.

## Layout policy follow-up: held for further qualification

The candidate retains return-register coalescing and broadens alignment only at
O2 and O3. Shared natural-loop facts are computed once per function and consumed
by allocation, parameter observation and native placement. Data-dependent exits
can qualify; the encoder compares the full back-edge span and a compact fast
subcycle after branch relaxation. It checks the actual encoded cycle for calls
and opaque assembly, so a slow continuation does not exclude its fast path.
The existing counted-cycle policy is retained.

Placement estimates must be available before extending the data-loop policy.
A bounded typed provenance check avoids new padding on forward scans over small
immutable tables. Branch-distance bounds include potential padding; unplaced
functions keep their prior conservative allowance. Existing 32-byte alignment
metadata is retained, and coincident labels are still padded once. The remaining
code-phase limitation is documented below. O0 skips the loop qualification work. There are no workload
names, source-text patterns, host-CPU switches or benchmark-specific flags.

The earlier broad policies were withdrawn after individual Godot regressions.
Removing padding from small table scans removed extra executed instructions,
but did not alone recover runtime. Software CPU-clock samples spread the
remaining time across interpreter dispatch, construction and conversion. That
supports a layout explanation; hardware PMU events remain unavailable on the
cloud hosts, so it does not identify a specific processor mechanism.

The two new PA33 fixtures exercise a data-dependent loop with multiple back
edges and a call on its slow continuation, plus sentinel scans over selected
readonly tables. The existing Perl runner checks O1/O2/O3 through four compile
and link routes. Student guidance describes the tradeoffs without requiring
specific instructions, addresses or padding. No LowIR syntax or contract changes.

All measurements below use immutable binaries and inputs, output checks, A/A
calibration and paired ABBA/BAAB blocks. Ratios are reference CPU / candidate CPU;
higher is better. GCC 15 and Clang 22 reference executables are frozen across
hosts, so these comparisons isolate host effects rather than CI toolchain
changes. They are local qualification measurements, not CI badge replacements.

This candidate is **not ready to merge**. The scalar recovery at the default
link placement is substantial, but an unused 32-byte object prefix still changes
its speed, and individual Godot regressions remain. The following results
describe the candidate, not acceptance of the layout policy.

| Host | Workload | vs current main | vs GCC 15 | vs Clang 22 |
|---|---|---:|---:|---:|
| EPYC 7B13 | simdjson scalar | 1.2046× | 1.0021× | 1.1523× |
| EPYC 7B13 | simdjson SSE | 0.9931× | 0.8966× | 0.8942× |
| EPYC 7B13 | Godot | 1.0033× | 0.9518× | 0.8904× |
| EPYC 9B14 | simdjson scalar | 1.2981× | 0.9996× | 1.1872× |
| EPYC 9B14 | simdjson SSE | 0.9974× | 0.9901× | 1.0026× |
| EPYC 9B14 | Godot | 1.0068× | 0.8840× | 0.8791× |
| Xeon E5-2696 v4 | simdjson scalar | 1.1833× | 1.2230× | 1.1973× |
| Xeon E5-2696 v4 | simdjson SSE | 1.0273× | 0.9620× | 0.9857× |
| Xeon E5-2696 v4 | Godot | 1.0309× | 0.9085× | 0.9021× |

These are equal-weight geometric means of two JSON inputs or five Godot
workloads. Godot includes process CPU time; its work-loop elapsed time is
retained separately. Intel runs used the shared development host; their
calibration and load limit small timing claims. The EPYC final cohorts used
three paired blocks and A/A blocks before and after, with at least one second
of JSON work or 1.5 seconds of Godot work per process.

Milan geometry measured about 2% slower in the full cohort; a longer seven-block
repeat still measured about 1.4% lower work-loop speed. Scene also needs care:
its longer repeat was about 3.5% slower. Geometry executes essentially the same
number of instructions and branches as main (within 0.002% in the retained
hardware-counter comparison), so the remaining gap is not additional useful
work.

With the exact same scalar library and driver objects, unused link prefixes
gave these speeds relative to the candidate's normal placement:

| Unused prefix | Twitter | CITM |
|---|---:|---:|
| 0 bytes | 0.9976× | 1.0000× |
| 32 bytes | 0.9196× | 0.9776× |
| 64 bytes | 0.9982× | 1.0006× |

The 32-byte prefix switches the shared code section's origin modulo 64 while
preserving its 32-byte alignment. This shows that the current alignment
guarantee is insufficient to make the gain durable. Merely raising that
section's minimum alignment can select the slower phase; it is not an accepted
fix. A separate, instruction-identical VM-section alignment probe recovered
scene speed but did not fully recover geometry. That probe is diagnostic only.

The next step is to investigate preserving the encoder's code phase when
packing shared and COMDAT sections, rather than choosing a favorable constant
offset. Requalify benign link placements and the individual workloads before
merging. No workload-name or source-shape special cases are proposed.

Compile-cost and validation:

- Frozen-input O3 instruction cost decreased about 0.14–0.20% for the GCC-built
  compiler, with smaller reductions for its self-built counterpart.
- The quiet Genoa native O0/O3 and self O3 CPU guards stayed close to baseline
  and their A/A variation; O0 objects remained byte-identical.
- Strict report: 9,931/9,931. Debug-info, inception identity, self-host through
  PA5, PA35 smoke and instruction kernels, and PA36 assembly checks passed.
- Architecture, formatting, feature placement and file audits passed; the file
  audit retained seven existing warnings. The source harness passed during this
  work, and both new fixtures run through the standard Perl driver harness.
- The final student export and fresh-client installation passed: 219 documents,
  the 55-file prose audit and all 42 reference wrappers. Both fixtures were
  compared to the exported copies.
- Five public smoke programs matched their output oracles with all three
  compilers. Four executable sizes were unchanged; pugixml grew by one 4 KiB
  page (about 1%). This is not a full public-suite timing qualification.
- A full PA35 release calibration on a superseded candidate was stopped after
  refinement became necessary. It is not counted as a final release pass.

Reproduction data is retained under
`/mnt/vizier/cppgm-codegen-data/20261109/layout-policy`. The selected compiler is
`bin/local-cycles-native` (SHA-256
`be157beb6aeb5501245b2a2d1ab082e46e8f8a832cf58315e38e7bc64cdc2242`);
`local-cycles.diff` records its compiler source changes. The `godot-local-cycles`,
`scalar/simd-local-cycles` and `sse/simd-local-cycles` metadata record complete
build commands and hashes. `milan-local-cycles-final-raw.tar.gz` and
`genoa-local-cycles-final-raw.tar.gz` retain all final runtime and cloud compile
guard observations, calibration and checksums; compiler guard objects were
omitted from archives but their hashes and frozen inputs were retained. Failed
experiments and their observations remain alongside the final cohorts.
