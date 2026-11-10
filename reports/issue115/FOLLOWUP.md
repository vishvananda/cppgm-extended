# Issue 115: follow-up fixes and final comparison

This completes the follow-up to [the first fixes](FIXES.md). The compiler
branch is rebased on LLVM worktree tip `528523f7a`. That LLVM tip has the same
compiler sources as the previously measured baseline `8a56e8811`. The follow-up
runtime compiler source is `e621392ea`; the compile-efficiency follow-up is
`23f3811bb`. It produces a byte-identical object for the frozen index-c input
and for the PA36 semantic workload. The measurements below retain the issue's
original frozen LowIR so that frontend changes do not get credited to the
backend.

## General transformations

The Huffman encode regression exposed an aggregate optimization gap. A large
object can contain both a variable-indexed payload and a few fixed fields
updated on every loop pass. Whole-object copies previously prevented scalar
replacement of those fields. The optimizer now splits recurrent fields while
preserving the remaining byte spans, including padding, at each snapshot.
It leaves cold fields in memory to avoid creating unnecessary loop state and
register pressure. The analysis has a fixed 16-field work limit and retains
the existing escape, overlap and volatile restrictions.

Load hoisting now distinguishes private fresh allocations from older loaded
pointers. A function-local proof tracks exact allocation roots and captures,
allowing invariant reads to move only while writes are proved disjoint.
Publishing or returning the pointer, opaque callees and unsupported pointer
flows invalidate the proof. The proof is built once for loop hoisting; no
new public LowIR fact is needed.

Native cleanup can reuse the sign flag for a signed comparison of an
arithmetic result with zero. It rewrites signed-less/greater-equal conditions
to sign/non-sign conditions because an explicit test clears overflow while
the arithmetic operation may set it. This runs after loop placement so it
does not hide tests needed by earlier loop transformations. Reducers cover
overflow, mixed condition-code consumers and both integer widths.

These are general rules over typed IR and control flow. No benchmark names,
function-name filters, instruction padding, larger inlining budgets or
benchmark-specific layout exceptions were added. Required tests use the
normal parallel Perl runner and behavior/quality bounds. The PA33 README
states student requirements; design notes explain the proofs and tradeoffs.

The compile-efficiency review also removed a canonicalization cycle:
object-valued `copyobj` was converted to a store and immediately back again,
reporting a change despite identical resulting IR. Accepting both forms
directly avoids unnecessary analysis invalidation and repeated optimizer
rounds. The dense SSA type table also rules out object-value work in
scalar-only functions before two linked-instruction walks. These changes
preserve the measured native output.

## Myc pointer-index correction

Myc revision `d31bd5b0112d54471b544991be688be83607f40f` emits every
nonliteral pointer index as `convert sext i64 i32`, even when its actual type
is 64-bit or unsigned. This truncates 64-bit indices and makes unsigned
32-bit indices above `INT32_MAX` negative. The correction preserves 64-bit
indices and extends narrower ones according to their actual signedness.
Wider indices are explicitly truncated to the target's pointer-index width.

The [companion patch](myc-gm-index-width.patch) changes
`src/backend/gm/bb.cr::offset`; it is supplied separately and has not been
merged upstream. The patch and [reproducer](myc-index-width.c) are also in
[the issue comment](https://github.com/vishvananda/cppgm-extended/issues/115#issuecomment-6778382054).
The reproducer checks
pointer differences at 2^32, unsigned 2^31 and signed -1. It allocates a large
virtual address range without touching a large physical buffer; the original
frontend fails and the corrected one passes. Both regenerated benchmark
inputs pass all 50 small checks.

The fact audit also checked signed-overflow and typed-access assumptions.
Myc's common IR explicitly requires signed wrapping, including
`spec/examples/ir/int_wrap/1.myc`; a signed source type does not justify
blanket `nowrap`. Its general emitter also lacks sufficient union/bitcast
provenance for blanket typed-access promises. Those annotations were not
added. The safe type information recovered here is the actual pointer-index
width and signedness; the backend derives allocation and induction proofs
from the existing contract.

## Final measurements

Measured on the Intel Xeon E5-2696 v4 with generic x86-64, `-O3`,
the same frozen LowIR and sixteen physical cores. Clang 22.1.2 and GCC 15.2.0
compile the pinned C input. Two Clang A/A runs precede separate ABBA blocks
for the original LLVM baseline, Clang and GCC. All 14 full runs pass 50/50.
Ratios use each paired block's means and are higher-is-better:

| Comparison | GM speed ↑ |
|---|---:|
| Final fixes versus original LLVM baseline | **1.249×** |
| Final GM versus Clang | **0.956×** |
| Final GM versus GCC | **0.975×** |

The original-baseline block falls from 114.161 s to 91.378 s, 20.0% less time.
Full-process user instructions fall from 913.551 to 773.783 billion,
15.3% fewer. The compact [summary and eight worst outliers](current-summary.md)
and [complete 50-case table](final-table.md) preserve the reference comparisons.
The summary is the benchmark's sum of timed cases, not a geometric mean.

Clang A/A totals differ by 0.8%; the original baseline's two totals differ
by 9.1%, and the final compiler's totals vary across blocks. This is a shared
host. Paired blocks limit drift, but small wall-time differences are noisy;
the tables do not establish parity for individual cases. Hardware counters
include preparation, warmup and threads, unlike the per-case timers.

Three additional single-core ABBA blocks put Huffman encode at 1.653 s before
and 1.499 s after: **1.103× faster**, with **6.6% fewer instructions**. Its
A/A imbalance is 4.3%, and all three A/B blocks improve. The first iteration's
regression is resolved. DFS executes 36.0% fewer instructions than the original
baseline. Maze generation and Huffman decode improve, but remain the two worst
relative gaps against Clang and are the next optimization targets.

The final compile sanity check uses the frozen PA36 semantic workload and
its documented long-double macro replacements. Three A/A blocks, three A/B
blocks and three counter blocks per level retain 48 compile timings and 24
counter observations. O0 objects match the original baseline; each O3 output
is deterministic. The efficiency follow-up preserves both measured workloads'
native objects byte for byte.

| Compile level | Instructions after/before ↓ | Paired wall after/before ↓ | Paired user after/before ↓ |
|---|---:|---:|---:|
| O0 | 1.00005 | 1.0069 | 1.0199 |
| O3 | **1.00209** | 0.9707 | 0.9657 |

The O0 timing difference is within its A/A block range (-1.8% to +2.7%).
The O3 instruction increase is **0.21%**, down from **0.96%** before the
canonicalization-efficiency change, relative to the same LLVM baseline.
These ratios exclude the earlier LLVM optimization work. The O3 wall result
improves in all three blocks; no timing regression is established.

[followup-observations.json](followup-observations.json) retains all 306 runtime
observations, including rejected experiments explicitly marked as excluded,
the earlier and final compile campaigns, direct efficiency comparisons,
commands, hashes and the input manifest. Measurements were built before the
final documentation-only rebase; compiler sources are identical afterward.

## Qualification

The final efficiency revision passes:

- Strict full report: 10,599/10,599.
- Debug-info checks, self-host through PA5 and byte-identical inception.
- PA36 smoke correctness, architecture/layout/contract/source-set/scaffold/
  expectation/format audits and file audit. The file audit reports seven
  pre-existing substantial-header warnings.
- Student export: 24,207 portable reference outputs, including 9,743 exit
  status files; the 256-document inventory, prose audit and all 43 wrappers.
  Export regeneration agrees with source references. The two changed native
  text references were regenerated through the normal harness.

The complete PA36 release check passed all 17 gates in both course and
student builds on the source now at `e621392ea`, before the final compile-efficiency revision.
Its measured original-input and semantic-workload objects are byte-identical
after that revision. The final revision repeats smoke, strict, export,
debug-info and inception checks. Release snapshots and logs are retained
with the raw data; generated calibration files are excluded from the branch.

Harness checks and the feature-placement audit also pass. Student handout
changes are included in the validated export; this performance report and
the diagnostic experiments are outside its document inventory.

The separate [Base64 source correction](base64-consume-output.patch) makes
the benchmark consume encoded bytes. It is not applied to the original-input
tables. The original length-only workload validly permits dead payload
elimination in both compilers; its large speedup is not encoding throughput.

Raw logs, immutable binaries, commands and rejected experiments remain under
`/mnt/vizier/cppgm-issue115-data/followup`. Measurements with overlapping
pinned workloads are retained as excluded experiments, not combined with the
final results. The earlier [FIXES.md](FIXES.md) measurements remain a
historical first iteration; compare only observations from the same window.
