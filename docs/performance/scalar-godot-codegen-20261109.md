# Scalar and indirect-dispatch optimization, 2026-11-09

The final local candidate reaches the 0.95× GCC target for both workload
aggregates. These are local EPYC measurements, not replacements for CI results.
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
