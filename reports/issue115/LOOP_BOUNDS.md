# Coupled loop bounds: Myc arithmetic decoding

This continues [the independent-field work](FIELD_VIEWS.md), using its
`8fcc6407e` checkpoint as the immutable baseline. The Myc index correction
and consumed-buffer Base64 correction remain applied; sources, inputs,
argv0, libraries and generic x86-64 `-O3` flags are unchanged.

The accepted change saves **13.17% of arithmetic-decoder instructions and
4.81% of user cycles** in the three-block repeat. This is a focused
improvement against the prior compiler, not a new whole-suite Clang score.

## Missing proof and general implementation

The decoder's two search bounds stay in `[0, 256]`. Their midpoint sum
therefore stays nonnegative and fits `i32`, allowing division by two to use
a logical shift. The old counter proof handled an exact add-one recurrence;
it did not prove intervals for coupled updates.

For single-latch natural loops, the existing loop/counter pass proposes the hull of nonnegative literal
entry seeds for a small group of same-width signed phis. It checks every
continuing update against that simultaneous hypothesis. Every entry is
included, and all updates must preserve the interval before any result is
published. Integer addition/subtraction must fit the original signed width;
positive operands alone do not justify ignoring wrapping.

A header guard refines phi views only in body definitions reached after its
in-loop edge. Header computations keep unrefined ranges. A latch guard can
restrict a carried update on the returning edge; its restriction never
changes that value's earlier cached range. This distinction preserves
arithmetic observed before a guard, including a negative divide.

Queries consume typed constants, copies, phis, selects and the supported
integer operations. Unknown external seeds, unsupported values, cycles outside
the hypothesized group, possible wrap and exhausted budgets retain the
original arithmetic. Current limits are eight same-type phis, 64 cached values
and 128 operand queries. Scratch is on the stack; the loop forest and final
power-of-two rewrite already exist, and the existing nonnegative-value vector
is allocated only after a successful useful proof. No unconditional range pass,
new serialized fact, frontend special case, source-name match or layout rule
is added. The `-O2` and `-O3` paths use the proof; `-O0` is unchanged.

## Isolation and focused measurements

Only the arithmetic-decoder function's optimized LowIR changes. Of 425 native
functions, the other **424 retain identical bytes at identical addresses**.
The decoder also retains its start address and 1,199-byte extent; existing
alignment accounts for its unchanged extent. Apart from this function and the
non-executed build-ID note, every allocated section has identical contents,
size and address. There is no new padding policy.

Intel Xeon E5-2696 v4, ASLR disabled, fixed argv0, NUMA node 1. The first
campaign uses core 30, two A/A calibrations and two ABBA blocks per case. The
repeat uses core 43, two A/A calibrations and three ABBA blocks. User counters
include preparation and warmup; the benchmark timer excludes them. Every
selected checksum passes. Ratios are baseline/candidate, higher is better.

| Workload | Runtime vs checkpoint ↑ | Instructions saved | Cycles saved |
|---|---:|---:|---:|
| Compress::ArithDecode | 0.831× | 13.17% | 4.67% |
| Compress::HuffEncode | 0.895× | 0.00% | 0.23% |
| Compress::HuffDecode | 1.079× | -0.00% | 0.17% |
| Brainfuck::Array | 1.007× | -0.00% | 0.06% |
| Etc::LogParser | 1.007× | 0.00% | 0.15% |
| Etc::NeuralNet | 1.017× | 0.00% | -0.45% |
| Json::ParseMapping | 1.165× | -0.00% | -1.64% |
| Compress::ArithDecode (repeat) | 1.049× | 13.17% | 4.81% |
| Etc::NeuralNet (repeat) | 1.002× | -0.00% | 0.38% |
| Json::ParseMapping (repeat) | 1.012× | 0.00% | 1.40% |

The first arithmetic A/A wall times are 3.322 and 2.317 seconds; its apparent
wall regression is inconsistent with the counter improvement and is not an
established code-generation loss. The repeat gives 1.049× wall speed and
1.051× cycle efficiency. Other cases have unchanged generated code;
their mixed first-window timing shifts do not identify changed instructions.
The repeat removes the apparent NeuralNet and JSON cycle losses.

## Compile cost and public sanity

On the immutable PA36 semantic-overload input, two A/A calibrations and two
ABBA blocks per level give candidate/checkpoint compiler instruction ratios
of `0.999989` at `-O0` and `1.000042` at `-O3`.
The measured work is essentially unchanged. `-O0` objects agree between
compilers, and each compiler is deterministic at `-O3`. Shared-host wall
variation is retained without a small compile-time speed claim.

The standard six-program public screen passes against GCC at both levels.
All twelve candidate executables are **byte-identical** to the checkpoint,
including FASTA. Small differences in the runner's counts therefore reflect
launch/runtime conditions rather than different compiler output. The complete
public and EPYC campaigns have not been repeated.

## Correctness and student material

Two required fixtures run the same reducer at `-O2` and `-O3`. They cover
header- and latch-guarded searches, swapped and inverted guards, 8/16/32/64-bit
values, averaging and remainder, zero work, negative and unknown seeds,
possible narrow overflow, decreasing updates and observations before a guard.
Quality predicates bound avoidable arithmetic and accept alternative
implementations; neither instructions nor registers are prescribed.
The handout states the required outcomes, and design notes describe the
entry/update/guard proof. The normal Perl runner discovers both fixtures.

An independent deterministic generator models 256 recurrence graphs with
two alternative external seed pairs each. All 512 checks pass at each of
`-O0`, `-O2` and `-O3`. It varies widths, wrapping seeds, signed/unsigned
comparisons, guard direction and timing, update offsets, divisors and budgets.
The generated stress input and oracle are retained with their hashes outside
the student export.

The strict report passes **10,632/10,632**. Debug-info checks, PA36 smoke,
self-host through PA5 and byte-identical inception pass. Architecture,
source-set, scaffold, format, file, placement and harness checks pass, as do
GCC15/Clang21 warning-as-error module builds. A fresh student export includes
both fixtures and their references, and its reference-wrapper installation,
document inventory and reducer execution are validated. The qualified compiler
emits the same Myc object as the measured frozen candidate.

The [JSON record](loop-bounds-20261111.json) retains counters, provenance,
code/data identity and complete full-suite case timings. All raw commands,
observations, AA controls, generated stress sources, binaries and validation
logs remain in `/mnt/vizier/cppgm-issue115-data/loop-bounds-20261111`.

## Full corrected-suite control

Two A/A calibrations precede one baseline/candidate/candidate/baseline block on cores 30–33. Every run passes 50/50 checks.

| Measurement | Checkpoint mean | Candidate mean | Efficiency ↑ |
|---|---:|---:|---:|
| seconds | 93.1226 | 89.4693 | 1.0408× |
| instructions, billion | 788.3751 | 787.1990 | 1.0015× |
| cycles, billion | 430.3577 | 426.7453 | 1.0085× |

A/A wall spread is 5.41% and cycle spread is 0.01%. These shared-host aggregates retain drift and do not replace the isolated arithmetic result or constitute a new Clang comparison.

The direct same-host arithmetic comparison against Clang22 records 0.892× benchmark time, 0.917× user-cycle efficiency and 0.909× instruction efficiency. All ratios are reference/cppgm; A/A wall and cycle spreads are 6.57% and 0.18%.

The direct same-host arithmetic comparison against GCC15 records 0.929× benchmark time, 0.937× user-cycle efficiency and 0.984× instruction efficiency. All ratios are reference/cppgm; A/A wall and cycle spreads are 8.98% and 0.12%.

## Residual gap and rejected predication screen

Against GCC the arithmetic workload is close in instruction count (0.984× efficiency) but still measures 0.937× cycle efficiency. Its mean branch-miss count is about 102.0M, versus GCC's 82.0M and Clang's 89.0M. This points to control/dependency behavior worth investigating, without establishing one cause.

A matched optimized-IR experiment replaces the search-bound branch with two conditional values. It passes all 50 small checksum checks and the full selected workload. It reduces branch misses by 25.1% yet costs 22.8% more cycles and slightly more instructions. This experiment is excluded; reducing misses alone is insufficient, and loop-carried data dependencies need profiling before changing predication policy. Both binaries are rebuilt through the same reoptimization path; the screen is not compared to a differently processed raw-IR binary.
