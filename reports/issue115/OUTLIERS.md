# Issue 115: the next runtime outliers

This batch starts from the qualified fixes in PR #124 (`e53739ed0`). It uses
the eight slowest relative-to-Clang entries in [the previous table](current-summary.md)
to choose investigations. The measured compiler source is `f586f86c4`;
later commits refresh informational reference dumps. The original issue
LowIR, source inputs and generic x86-64 target are unchanged. The separately
supplied Myc index correction and consumed-buffer Base64 source change are
excluded from this comparison.

## Findings and changes

The two largest gaps had distinct causes. Neither requires new frontend
lowering or a LowIR extension.

* **Maze:** Boolean range reasoning stopped at a `select` and an integer
  retype. The simplifier now recognizes selected canonical zero/one values
  and follows their copies when simplifying branch conditions. Existing
  CFG/range simplifications can then combine membership in states 1, 2 and
  3 into a range check. Arbitrary integer narrowing and noncanonical select
  arms retain their semantics; the proof has the existing depth bound.
* **Huffman decoding, register groups:** the allocator stopped considering
  a phi web after assigning its first member. An invariant root can overlap
  a whole chain of changing node values, although successive changing
  values do not overlap one another. Remaining noninterfering subsets may
  now receive another register. Both attempts per web and candidate members
  retain a fixed work limit of 32.
* **Huffman decoding, frequency estimates:** unrolling leaves repeated
  validity/bound guards whose early destination is the enclosing loop's
  latch. Repeated independent 50/50 estimates make the later work appear
  exponentially colder and discourage assigning its values registers.
  Pointer-validity and incremented-counter/invariant-bound guards can use
  the existing weak loop-continuation estimate. Explicit branch predictions
  take precedence. The latch map is computed once; it is likelihood
  evidence, never a proof that a path is unreachable.

The last distinction matters. Applying the continuation estimate to every
early edge regressed Huffman encoding by 19% in executed instructions and
arithmetic encoding by 11%. Conditional encoder work can intentionally run
on a minority of iterations. That broad experiment was rejected; the
narrower version retains the decode gain and restores both encoder counts.

These rules inspect typed values, liveness and control flow. They do not
inspect benchmark/function names or recognize a fixed instruction sequence.

## Focused measurements

Intel Xeon E5-2696 v4, generic x86-64, `-O3`, immutable binaries, CPU 23,
two ABBA blocks per workload. All executions pass their checksums. Speed
ratios are higher-is-better; instruction reductions use full-process user
instruction counts, including setup and warmup.

| Workload | Comparison | Speed ↑ | Fewer instructions |
|---|---|---:|---:|
| Huffman decode | Final narrowed rules / PR #124 | **1.522×** | **19.0%** |
| Maze generation | Boolean rule / register-group-only variant | **1.680×** | **28.2%** |
| Maze BFS | Boolean rule / register-group-only variant | **1.313×** | **19.3%** |
| Maze A* | Boolean rule / register-group-only variant | 1.029× | 1.8% |
| Huffman encode | Final narrowed rules / PR #124 | 1.003× | essentially unchanged |
| Arithmetic encode | Final narrowed rules / PR #124 | 0.998× | essentially unchanged |

The register-group change alone improves decoding 1.108× and removes 3.6%
of instructions. The frequency correction supplies the remaining gain.
The decode function's explicit stack reservation falls from 232 to 48 bytes.
Register-group-only code has unchanged maze instruction counts, so the
Boolean comparison isolates that transformation.

## Complete workload check

Eight complete runs pass all 50 checks: two Clang A/A calibrations, an
old/new/new/old block, and single Clang/GCC context runs. The paired GM totals
fall from 90.932 to 86.380 seconds, an observed **1.053×** speedup. User
instructions fall from 773.784 to 765.701 billion, **1.04% fewer**. This is a
sum of timed cases, not a geometric mean.

The Clang A/A totals differ by 4.1%, the two parent totals by 5.9%, and the
two candidate totals by 8.8%. Treat the aggregate wall result as approximate.
The separate Clang/GCC context totals are 85.416 and 91.815 seconds; they
are not paired reference comparisons and do not supersede the previous
batch's paired ratios.

The apparent matrix slowdowns have unchanged worker/launch instructions
after relocation-address normalization. Pinned controls also have identical
executed instruction counts and within 0.2% of the original on-CPU cycles.
Those controls serialize the benchmark's threads onto one CPU; their wall
times do not measure parallel throughput. Graph DFS likewise has unchanged
instruction counts and improves in its separate pinned control.

Template parsing needs continued attention. Its initial apparent 15% wall
loss shrinks to a 1.3% paired median in three more ABBA blocks, versus 1.8%
in the same-CPU A/A calibration. Instructions remain unchanged; mean cycles
are 3.7% higher, with block ratios ranging from -1.1% to +6.7%. Its run and
template-preparation instruction streams are unchanged after relocation
normalization. These observations do not establish a stable wall regression,
but retain the cycle signal for quieter layout/allocator controls rather
than declaring every workload unchanged.

## Cost of compiling

The frozen PA36 semantic workload was compiled at `-O0` and `-O3` by immutable
parent/candidate binaries, using three ABBA blocks and a separate three-block
A/A calibration for each level. All 48 timed compiles and 24 counter compiles
succeed. `-O0` objects are byte-identical across compilers; every variant's
output is deterministic.

| Level | Candidate / parent instructions | Paired wall change | A/A wall change |
|---|---:|---:|---:|
| `-O0` | 1.000019 | -0.38% | +1.65% |
| `-O3` | 1.000488 | +0.16% | -0.32% |

The added `-O3` work is approximately **0.05%**, beyond the parent batch's
already reduced +0.21%. The wall measurements do not establish a compile-time
regression. This is a shared host; retain the calibration rather than
interpreting sub-percent differences as precise speed changes.

## Remaining investigations

Calculator and Words need controlled repeats before treating their original
full-suite wall gaps as optimizer losses. Focused controls put Calculator
around 0.92× Clang and Words near parity, substantially closer than that
first table. The Calculator profiles spend a majority of cycles in allocation
and deallocation; remaining parser call boundaries deserve investigation.

Brainfuck has a concrete additional opportunity: a redundant null comparison
of a direct local-slot address prevents scalar replacement of its tape state.
An isolated implementation removes about 11% of executed instructions and
passes correctness, including nullable parameters, wrapping pointer arithmetic
and volatile accesses. It also exposes an approximately 10% Huffman-encode
wall regression despite unchanged instruction counts. It is preserved on
`opt/issue115-local-slot-address` and excluded from this batch until the
register/layout sensitivity is understood.

Further priorities are Maze A* (now among the largest contextual gaps), the
remaining decode branch/phi traffic, Brainfuck's dispatch and state
representation, then merge-sort copying and log parsing.
Existing LowIR already represents the needed operations. Diagnose retained
loads, alias facts and machine instructions before adding public facts or
enlarging inlining budgets.

A separate LogParser diagnostic supplied the existing `readnone`,
`repeatable` and `unwind=no` facts for glibc's `__ctype_b_loc`, following its
installed header declaration. It did not reduce executed instructions or
cycles. That is not evidence for extending LowIR or blaming this gap on
missing frontend call attributes. Its drifting wall times are retained with
the other diagnostic observations.

## Qualification

Local qualification passes: 10,601 strict tests, debug-info, self-host through
PA5, byte-identical inception, PA36 smoke, all architecture/format/file/placement
and harness audits. The export verifies 24,216 portable reference outputs
(9,746 status files), 256 documents and all 43 reference wrappers. The complete
PA36 release calibration was not repeated for this final narrowed candidate.

The batch adds PA33 Boolean correctness/quality checks and a PA34 loop/phi
fixture through the normal Perl runner. Sidecars bound output quality without
requiring register names, instruction order or a particular pass design.
The handouts state student requirements; PA34 design notes discuss lifetimes
and frequency estimation. No new syntax or mandatory optimizer algorithm is
introduced.

[Raw observations](outlier-observations.json) retain immutable binary/input
hashes, commands, all runtime observations, compile A/A and ABBA records,
hardware counters, checksum output and the rejected/deferred experiments.
Full logs and intermediate artifacts remain in
`/mnt/vizier/cppgm-issue115-data/next-round`.
