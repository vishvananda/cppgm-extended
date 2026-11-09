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
