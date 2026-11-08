# Producing the same machine code with less work

This change reduces the implementation cost of register-home forwarding in
the native backend. It preserves the analysis lattice, predecessor order,
reverse-postorder rounds, transfer rules, rewrite order and chosen registers.
The baseline is `e28ecf189`, including the frontend dependence-cache change.
This is maintainer measurement material, outside the student export.

## Changes

- Find a replacement holder only during rewriting. Analysis-only transfers
  need the resulting home facts, so searching sixteen registers for an unused
  replacement is unnecessary.
- Calculate an instruction's width only on paths that consume it.
- Meet complete, contiguous predecessor rows. The old loop rescanned the
  predecessor list separately for each of the sixteen registers.
- Remember whether a predecessor's output changed. A clean block skips its
  meet; a dirty block still compares its input before transferring. The
  existing traversal order and fixed point remain intact.
- Compare and copy output rows as a unit instead of branching for every
  register. Skip graph construction when there is no nonvolatile direct
  frame read this pass can rewrite.

The last guard is conservative: it can include reads that will not forward,
but every eligible load, compare or test is included. Exceptional entry cuts,
mid-block conditional exits, scratch lifetimes, calls and memory invalidation
retain their existing behavior. Initialized state vectors and the existing
CSR construction remain; alternative storage prototypes added little benefit.

## Instruction counts

Immutable GCC-built and self-built compiler controls compile the same three
frozen, preprocessed inputs at `-O3`. Each control has A/A calibration followed
by two paired ABBA/BAAB blocks. Intel PMU counters are pinned to CPU 39 with
unmultiplexed user events. Every output object is hashed and compared.

Scores below are **baseline instructions / candidate instructions**, so higher
is better. The column identifies how cppgm itself was built.

| Frozen input | GCC-built cppgm ↑ | Self-built cppgm ↑ |
| --- | ---: | ---: |
| Compiler loop workload | 1.0052× | 1.0070× |
| Semantic workload | 1.0063× | 1.0086× |
| Boost flat-map workload | 1.0083× | 1.0112× |

That is about **0.5–1.1% fewer total compile instructions**. Instruction A/A
scores are within 0.003% of parity. RSS is effectively unchanged. The same
controls at `-O0` stay within 0.02% of instruction parity; this optimization
pass runs only at `-O1` and above.

Callgrind, collecting only `native::forward_home_reloads` on the frozen
semantic input, attributes the savings directly:

| Compiler implementation | Baseline instructions | Candidate instructions | Efficiency ↑ |
| --- | ---: | ---: | ---: |
| GCC-built | 128,255,689 | 72,529,105 | 1.7683× |
| Self-built | 175,853,863 | 97,684,323 | 1.8002× |

The pass performs **43–44% fewer instructions**. Its limited share of total
compilation explains the smaller overall gain. Local CPU and wall-time
calibration is noisy, so the local instruction reduction is the resolved
performance result.

## Quiet EPYC timing

The same frozen binaries and inputs were measured on an EPYC 7B13 VM with SMT
disabled, pinned to one of its two physical cores. A pinned runtime loader and
libraries keep the controls independent of the VM's installed C++ runtime.
For each input and compiler implementation, two warmups precede two A/A blocks
and three ABBA/BAAB blocks. All 132 observations and object hashes are retained.

Scores are **baseline compile CPU / candidate compile CPU**; higher is better.

| Frozen input | GCC-built A/A | GCC-built CPU ↑ | Self-built A/A | Self-built CPU ↑ |
| --- | ---: | ---: | ---: | ---: |
| Compiler loop workload | 1.0024× | 1.0019× | 0.9947× | 1.0022× |
| Semantic workload | 1.0018× | 1.0016× | 0.9994× | 1.0024× |
| Boost flat-map workload | 0.9993× | 1.0017× | 1.0010× | 1.0072× |

The geometric mean of the three median CPU scores is 1.0017× for GCC-built
cppgm and 1.0039× for self-built cppgm. Most changes are near calibration
variation. The self-built flat-map case has the clearest gain: its three paired
scores are 1.0101×, 1.0072× and 1.0057×. These measurements support a small
timing benefit, without establishing a general 1% wall-time improvement.

## Output and correctness

All `-O0` and `-O3` measured objects match their baseline byte for byte. As a
broader control, the candidate compiles the complete frozen baseline compiler:
all **321 objects** match, and the linked self-built compiler has the same
SHA-256, `ab701839743b3504f32ab951e7fec9e46709119ae1447b96400bb66281b218c1`.
This checks existing compiler output across the complete implementation.

The qualified native compiler matches the final measured native binary. The
profiled and final measured variants also have identical native `.text` on
each toolchain; differences in private source-file metadata do not affect the
measured instructions.

- Strict report: 9,860/9,860; debug-info suites pass.
- Optional C++14/17, 120 target-feature and 182 assembly checks pass.
- Self-host through PA5 passes; inception rebuilds an identical compiler.
- PA35 smoke: 3/3.
- Architecture, layout, exceptions, source sets, registries, scaffold, IR,
  formatting, file and feature-placement audits pass. The file audit reports
  its seven existing header-organization warnings.

There is no new language, LowIR, MIR or student code-quality requirement.
Existing behavior suites and output equality validate this implementation
change without prescribing a student's optimizer algorithm.

## Remaining pass work

Continue profiling allocation, branch placement and integer-range propagation.
Look for repeated ancestor updates, graph construction within an unchanged
phase, and dataflow visits whose inputs did not change. Reuse facts only while
their owning representation remains unchanged. Preserve deterministic choices
and verify complete output equality for implementation-only changes.

Register equivalence across joins and economical loop-constant lifetimes remain
separate generated-code improvements in the [codegen plan](frontend-inception-20261108.md#codegen-work).
Those require execution improvements in addition to a justified compile cost.

## Receipts

Commands, immutable binary/input hashes, A/A and paired observations, PMU
running percentages, object hashes, all prototype results, Callgrind profiles
and validation logs are retained under
`/mnt/vizier/cppgm-codegen-data/20261108`. Rejected prototypes remain in that
directory; they are not shipped to students.
