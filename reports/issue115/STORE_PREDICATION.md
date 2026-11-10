# Issue 115: conditional stores and the remaining outliers

This batch starts at main `535ed4f12`, after PRs #124 and #125. The measured
compiler source is `d1c586291`; `89a836742` only corrects two host-test
helpers. Inputs are the original frozen issue LowIR and unchanged generic
x86-64 target. The separate Myc index and consumed-buffer Base64 source
patches remain excluded.

## Merge-sort gap

Clang's LLVM IR selects the value to write and advances the corresponding
cursor. cppgm retained a branch with a store on each arm. Its original
output executes fewer instructions than Clang, but pays more cycles for
unpredictable comparisons. This is an optimizer admission gap: existing
LowIR and native conditional-value lowering can express the useful result.

The optimizer now factors one ordinary store through a small unhinted
diamond when both arms address the same location. A bounded proof compares
pure address expressions; independent loads are never equated. Hoisted
work must be safe on either path and loses arm-only arithmetic promises.
Alias-observing reads, callbacks, volatile accesses, extra writes and
potentially trapping operations prevent this transformation. At most two
merge values change; load-dependent cycles require sequential unit cursor
updates rather than general pointer chasing.

An initial version also predicated an invariant choice inside a loop,
preventing later unswitching and vectorization. The strict suite caught
that interaction. The final rule retains invariant loop choices, using a
loop forest built lazily only after a store candidate passes admission.
The existing unswitching reference is unchanged. No fixture names,
benchmark names, new IR syntax or frontend policy are involved.

## Focused measurements

Intel Xeon E5-2696 v4, immutable binaries, CPU 23, memory node 1, ASLR
disabled, fixed argv0; two ABBA blocks per comparison. All checksums pass.
Ratios below use mean timed-case durations and full-process user counters.
Each comparison has its own paired baseline; absolute times across blocks
are not interchangeable.

| Workload | Comparison | Speed ↑ | Instructions, candidate / reference | Cycles, candidate / reference |
|---|---|---:|---:|---:|
| Merge sort | candidate / main | **1.412×** | 1.228× | 0.698× |
| Merge sort | candidate / Clang 22 | **1.070×** | 0.899× | 0.936× |
| Huffman encoding | candidate / main | 1.009× | 1.000× | 0.998× |

Merge sort has approximately **91% fewer branch misses** than main. More
instructions are useful here because removing unpredictable branches saves
cycles. The complete input object's reported text size changes from 222,041
to 222,113 bytes, an increase of 72 bytes (0.032%).

## Complete workload check

All eleven full runs pass all 50 checks: ten paired/calibration runs and
one GCC context run. The main/new block totals
are 89.323 and 89.681 seconds, an observed 0.996× speed ratio; its full-process
instructions increase 0.165%, cycles increase 0.229%, and branch misses fall
8.3%. Merge sort falls from 1.458 to 1.019 seconds in this block (**1.430×**).
Huffman encoding is unchanged at 1.340 seconds.

The separate Clang/new block totals are 86.289 and 87.998 seconds, an observed
**0.981× Clang**. Candidate instructions are 1.152× Clang's and cycles are
1.019×. These are sums of timed cases, not geometric means. See the
[complete paired table](store-predication-table.md), sorted by observed
relative-to-Clang time. Its outlier ranking is a guide for focused follow-up,
not a stable ranking established by this shared-host campaign.

The single GCC context run totals 82.765 seconds. It is not a
paired GCC speed comparison.

The initial Clang A/A totals differ by 0.9%, the main repetitions by 4.3%,
and the later Clang pair by 8.5%. Candidate totals in the Clang block range
from 84.247 to 91.750 seconds. Those spreads exceed the aggregate changes;
they do not establish a broad runtime improvement or regression. The focused
merge result above has consistent instruction/cycle/branch evidence.

The protocol uses 16 physical cores on memory node 1, with explicit CPU
and memory binding and ASLR disabled. Qualification runs use other cores.
The sequence is two Clang A/A runs, main/new/new/main, Clang/new/new/Clang,
then one GCC context run. All observations, including calibration and
unfavorable timings, are retained. This replaces the earlier unrestricted
core pool; compare paired ratios within this run window, not absolute totals
against the previous report. Clang's total shifts from about 77 seconds in
the initial A/A pair to 90 seconds later in the campaign, so short-window
calibration does not remove the longer-window drift.

Relocation-normalized native comparison finds 423 unchanged function bodies
and four changed bodies: JSON value printing, regex parsing, merge copying
and spectral-norm preparation. In particular, the Brainfuck, A*, Huffman
encoding, template parsing and matrix worker code is unchanged by this
batch. This establishes instruction-stream equality after symbolic
relocation normalization, not immunity to cache, layout or host-load effects.

## Cost of compiling

The frozen PA36 semantic workload was compiled by immutable main/candidate
binaries at `-O0` and `-O3`: three ABBA wall blocks, a separate three-block
A/A calibration at each level, and three counter blocks per level. All
48 wall and 24 counter compiles succeed. `-O0` outputs are byte-identical;
optimized output is deterministic for each compiler.

| Level | Candidate / main instructions | Paired wall change | A/A wall change |
|---|---:|---:|---:|
| `-O0` | 0.999983 | +2.03% | +1.64% |
| `-O3` | 1.000296 | -0.92% | +3.27% |

Added `-O3` work is approximately **0.03%**. No new optimization runs at
`-O0`; its executed work and output are unchanged. The wall spread and A/A
calibration do not establish a stable compile-time change. Raw results
retain the observed `-O0` increase rather than treating unchanged instruction
counts as proof of unchanged wall time.

## Deferred opportunities

**Brainfuck:** proving a direct local-slot address nonnull enables scalar
replacement and removes about 11% of its executed instructions. Controlled
runs improve it approximately 17%, but Huffman encoding loses about 8%
with unchanged instruction counts. The experiment remains on
`opt/issue115-local-slot-address` and is excluded here. Recovering an
existing jump table from an equal-work comparison ladder supplies no stable
additional gain and slightly increases Brainfuck's instructions; that trial
is excluded too.

**Maze A\*:** fixed-binary timings vary substantially despite unchanged
instruction counts, ASLR control and memory binding. One process incurs
roughly 703,000 minor faults. Those observations require separating
allocator/kernel time from compiler-generated search work before choosing
another transformation. They do not justify a precise A* wall-speed claim.

The LLVM IR comparison nevertheless identifies useful follow-ups. Clang
keeps queue pointer/size state in loop values and selects between heap
indices; cppgm retains more field reloads and branches. cppgm also emits
runtime overlap checks between distinct allocation-backed initialization
arrays. Allocation-separation proofs already exist in the optimizer, while
the vectorizer's dependence admission does not consume that proof. These
are opportunities to test existing fact consumption and memory promotion;
they are not evidence that a new LowIR extension is required. Any change
must retain alias/capture and allocation-failure behavior and improve stable
on-CPU measurements before being accepted.

## macOS host-test correction

Main's LLVM 18 macOS report fails two PA33 constructor-entry checks because
their host helpers use raw ELF assembler names. The generated Mach-O objects
correctly define the ABI entries with the platform's additional underscore.
The helpers now use the target prefix, following the existing host-test
convention. The original helpers reproduce the unresolved symbols locally;
the corrected objects link with no unresolved symbols using cross-target
Clang 18 and 22 plus LLVM's Mach-O linker. Both fixtures also pass through
the normal Perl host runner on Linux. Full native macOS execution remains a
CI check; no production symbol-emission change was needed.

## Qualification

Local qualification passes: 10,603 strict tests, all architecture/layout,
LowIR, source-set, scaffold, expectation, formatting, file, placement and
harness audits; PA36 smoke; debug-info checks; self-host through PA5; and
byte-identical inception. The student export verifies 24,222 portable
reference outputs (9,748 status files), 256 documents and all 43 reference
wrappers. Both corrected host tests also pass a targeted normal-runner
check after their helper edit. The full PA36 release performance gate was
not repeated for this final candidate.

PA33's student requirements describe the conditional-store quality bounds
and observations that must be preserved. Design notes discuss address,
speculation and loop-invariance proofs as advice. Tests use the standard
parallel Perl runner and behavioral checks plus broad branch/store bounds;
they do not require particular instructions or register choices.

[Raw observations](store-predication-observations.json) retain input/binary
hashes, commands, checksum output, counters, A/A calibration and all paired
observations. Complete logs and intermediate artifacts remain under
`/mnt/vizier/cppgm-issue115-data/third-round`.
