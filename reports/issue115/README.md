# Issue 115: index-c reproduction and optimization gap analysis

Measured 2026-11-10 on an Intel Xeon E5-2696 v4. The large runtime gaps
reported in [issue 115](https://github.com/vishvananda/cppgm-extended/issues/115#issuecomment-6767637734)
reproduce with the newest clean LLVM worktree revision available when this
investigation started. Most of the largest gaps are failures to optimize legal
LowIR forms, rather than missing source-language features or x86 instructions.
There is also a measurable native division-selection opportunity.

This document records the initial analysis with unchanged compiler source.
The subsequent production fixes and their qualification are documented in
[FIXES.md](FIXES.md). The [follow-up and final comparison](FOLLOWUP.md) resolve
the first iteration's Huffman regression and record the Myc index correction.
The [compact current table](current-summary.md) lists overall performance and
the remaining worst outliers. The input transformations in this directory
are diagnostic counterfactuals, not proposed production passes. They localize
the missed general transformations; their function-name restrictions select
profiled experiments and must not become optimizer rules.

## Versions and method

This investigation has its own worktree,
`/mnt/vizier/cppgm-issue115-index-c`, branch `investigate/issue115-index-c`.
It is based on LLVM worktree commit
[`e1766cfa5f731378fcf00ad1f714e2bb0ff8cbb6`](https://github.com/vishvananda/cppgm-extended/commit/e1766cfa5f731378fcf00ad1f714e2bb0ff8cbb6),
“Keep COMDAT alias export bundles intact across mixed compilers.” The other
LLVM worktree was left untouched.

Inputs and tools:

- The author's frozen LowIR from the issue's
  [gist](https://gist.github.com/kostya/4a40c20c686a37568298f875edbfbc13),
  raw revision `69c2502df2c4fd2c190a596be79ab404a6e10221`;
  SHA-256 `5cdfcba1a0edb46b42133c58950bf11cb7e34c9edd3d3792a282b7a58c4fbc84`.
- [index-c](https://github.com/kostya/index-c) commit
  `b5dcf9d4ca06bbd70ada603a1478aa5a471c1f6f`.
- [Myc PR 14](https://github.com/kostya/myc/pull/14) head
  `d31bd5b0112d54471b544991be688be83607f40f`.
- Clang/LLVM 22.1.2 and GCC 15.2.0; generic x86-64, `-O3`, no
  `-march=native`, PGO, LTO or fast-math. Myc's LLVM target machine also uses
  empty CPU/features, so it is not getting a native-ISA advantage.
- Myc built with Crystal 1.22.2 and `LLVM_CONFIG=llvm-config-22`.
  Crystal's own bundled LLVM is not the LLVM target backend linked into Myc.
- cppgm built with GCC and warnings as errors. Its binary SHA-256 is
  `009bd3f726ea435492de5afc0eed96199cef40712b75b3785e4b2dd35e9f60be`.

The original experiment compiles C through Myc's libclang frontend, TypedAST
and common Myc IR, then selects a GM, LLVM, QBE or C backend. Its GM driver
uses `-O1` by default and `-O3` with `--final`. The benchmark script runs each
executable once, checks `50, 50, 0`, and parses its `Summary:` timer. This is
not an experiment in cppgm's C++ frontend.

I reproduced the frozen LowIR directly, and separately rebuilt Myc and
regenerated both GM and LLVM outputs from the pinned C source. The regenerated
LowIR differs from the frozen gist, so their numbers are labeled separately.
I did not rebuild the historical cppgm `30adb354...` used by the issue author;
the purpose here is to reproduce the issue on the current LLVM branch.

Full runs use sixteen relatively idle physical cores, with no SMT sibling
duplicated in the set. This preserves the T4/T8/T16 matrix benchmarks' threads.
The baseline has Clang A/A calibration followed by Clang/GM/GM/Clang paired
blocks; GCC and regenerated frontend controls are exploratory single runs.
Focused single-thread cases use separate pinned cores and paired blocks for
the main counterfactuals. Every binary is immutable and hashed before/after.

All recorded runs pass their selected checksum oracles. All diagnostic
executables also pass all 50 small `TEST=1` checks; the combined object/boolean
variant additionally passes two complete normal-size 50-case runs.
These are benchmark checks, not a proof of compiler correctness.

Wall time has visible host noise: identical Clang A/A runs report 81.5610 and
78.4265 seconds, about 4% apart, whereas their user instruction counts differ
by only 0.00012%. Small wall-time differences are not treated as wins. `perf`
counts cover the process including preparation, warmup and worker threads;
the benchmark's per-case timers exclude preparation/warmup. Do not divide one
by the other and call that timed-loop IPC.

## Reproduced results

Higher ratios mean faster GM-generated code: `reference seconds / GM seconds`.
The main total uses the benchmark's `Summary:` sum, not a geometric mean.
Per-case printed timers are rounded to milliseconds, so their sum differs
slightly from the higher-resolution summary.

| Build | Normal suite seconds | Checks |
|---|---:|---:|
| Clang C `-O3`, paired mean | 79.4251 | 50/50 |
| GCC C `-O3`, single run | 81.5139 | 50/50 |
| Frozen issue LowIR, current GM `-O3`, paired mean | 100.1709 | 50/50 |
| Regenerated Myc GM `--final`, single run | 106.8289 | 50/50 |
| Same Myc frontend, LLVM `--final`, single run | 79.8322 | 50/50 |

The Myc default GM (`-O1`) path was also rebuilt and run: 112.8392 s, 50/50.
The adjacent later Clang run was 93.4411 s, also 50/50. This later control
was substantially slower than the initial Clang block, so these observations
are retained separately; they do not support a precise O1/O3 speedup claim.

Frozen-input GM is **0.793× Clang**, or 26.1% longer runtime, and **0.814× GCC**.
The author's `--final` total was 64.5190 versus Clang 50.7452, or 0.787×.
The aggregate result and dominant outliers therefore reproduce despite the
different host and compiler version. This is not evidence of a regression
between those compiler commits: it is not a controlled old/new comparison.

The [complete 50-case table](baseline-table.md) is sorted by absolute extra
time against Clang. The first six cases account for 71% of the sum of positive
per-case excesses. Raytracer and Base64 encode alone account for 51%.

| Case | GM vs Clang ↑ | GM vs GCC ↑ | Main finding |
|---|---:|---:|---|
| TextRaytracer | 0.129× | 0.128× | Object SSA canonicalization, scalar replacement, mixed-width access interference |
| Base64 encode | 0.095× | 0.871× | Clang removes unobserved payload work; GM retains it |
| Huffman decode | 0.409× | 0.437× | Boolean slots block promotion and eight-step unrolling |
| Maze generator | 0.597× | 0.615× | Boolean/range control simplification |
| LogParser | 0.455× | 1.208× | Boolean slots and copies in regex interpreter |
| SHA256-labeled hash | 0.635× | 0.649× | Nonnegative induction range not used for signed remainder |
| Maze AStar | 0.745× | 0.529× | Packed objects, branch chains, spills; residual not fully isolated |
| BWT encode | 0.742× | 0.928× | 64-bit division selection plus loop/address/register costs |
| Graph DFS | 0.714× | 0.756× | Object transfers and loads left inside inner loop |
| Base64 decode | 0.763× | 0.774× | Scalar table/bit assembly; residual not fully isolated |
| Arithmetic decode | 0.769× | 0.778× | 64-bit division selection and binary-search range arithmetic |

Single cold-ish compile observations, including link, were Clang C 3.95 s,
GCC C 6.47 s, Myc GM `--final` 1.75 s and Myc LLVM `--final` 5.36 s.
Frozen LowIR to a GM object alone took 0.48 s. That last number excludes the
C frontend and link and must not be compared as end-to-end compilation.
These build observations are provenance, not a paired compile-speed claim.

A complete diagnostic object/boolean-normalization ABBA block reports a
104.4308 s control mean versus 95.8751 s for the variant, **8.2% less time**.
Full-process user instructions fall from 910.152 to 824.866 billion,
**9.4% fewer**. Both are two-pass inputs, and all four normal runs pass 50/50.
This variant does not include the later mixed-width Raytracer, hash-range or
division experiments; it is not a combined implementation of every finding.

## What is missing, with causal experiments

### 1. Raytracer: whole-object forms and scalar replacement

In optimized GM LowIR, `TextRaytracer_run` has 96 slots, 81 `copyobj`s and
56 whole-object loads. Its native body is 10,619 bytes and reserves 4,856
stack bytes, in addition to saved registers. Clang's body is 1,621 bytes
with a 72-byte stack reservation; Myc LLVM's body is 1,742 bytes with 264
bytes reserved. Myc LLVM carries aggregate SSA values into LLVM's scalar
replacement instead of retaining the chain of temporary Vec3 objects.

There are three separable issues:

1. [`AggregateSlots::objects_through_slots`](../../dev/src/opt/aggregate_slots.cpp)
   recognizes a whole-object `load` consumed by whole-object `store`, but
   considers an equivalent object-valued `copyobj` an unsupported consumer.
   Changing only that spelling reduces focused instructions from 53.749 to
   38.666 billion, **28.1% fewer**.
2. Inlining leaves object-value aliases and `copy obj<24x8> $slot` snapshots.
   Normalizing aliases, object copies and slot snapshots to the existing
   load/store form reduces instructions to 18.106 billion, **66.3% fewer**
   than the matched two-pass control. Merely running O3 a second time leaves
   the original 96 slots and 81 object copies intact.
3. Scalar replacement then encounters both f64 field accesses and overlapping
   v128 accesses of the same Vec3 bytes. `AggregateSlots::settle` rejects
   incompatible/overlapping field partitions. Splitting only the 15 vector
   *memory accesses* into bit-preserving f64 lane accesses lets existing
   scalar replacement remove the remaining temporary objects. Vector
   arithmetic is preserved. A matched third-pass control retains 27 slots
   and 13 copies; the variant has two slots and one copy, for the scene.

The last step cuts focused time from 5.0410 to 1.9242 s and instructions from
17.629 to 13.934 billion. Clang in that block is 1.4697 s / 8.031 billion.
Across the steps, about **74% of the original instructions disappear**.
The remaining roughly 31% time gap is real unfinished work; reduced native
code is 1,902 bytes with 408 bytes reserved, still more than Clang.

Adding `typed` to the ray/helper ordinary struct stores produces an object
byte-identical to the original. Missing alias annotations are not the cause
of this large outlier. Increasing inlining alone is not the right first fix.

**Owner:** LowIR canonicalization and aggregate optimization, with pass-order
interaction. Existing LowIR semantics suffice. Prefer handling all legal
object forms and coherent mixed-width partitions; do not disable vectorization
globally or require one frontend to emit a privileged spelling.

### 2. Base64 encode: allocation-aware dead work

`Base64Encode_run` allocates an output buffer, encodes into it, accumulates
only the returned length, then frees the buffer without reading its bytes.
Its checksum uses a separate prepared buffer. Clang's optimized LLVM IR and
Myc LLVM's output remove allocation and payload encoding, retaining a length
calculation loop. GM still executes the payload algorithm.

Focused observations:

| Variant | Seconds | User instructions, billion |
|---|---:|---:|
| GM, two-run mean | 2.0835 | 32.375 |
| GM, force encoder inline, mean | 2.1681 | 31.656 |
| Clang | 0.2136 | 2.886 |
| Myc LLVM | 0.2153 | 2.886 |
| Clang with both vectorizers disabled | 0.2023 | 2.886 |
| Clang with library builtins disabled | 2.0471 | 30.936 |

The huge Clang ratio is primarily an optimization of the benchmark's
unobserved work, not tenfold better encoding throughput. GCC also retains
much of it, explaining the much smaller GM/GCC gap. A payload-throughput
benchmark should consume the newly encoded bytes, as a separately labeled
test rather than silently changing this reproduction.

The optimizer already recognizes malloc/calloc as fresh allocations
([function_ir.cpp](../../dev/src/opt/function_ir.cpp)); simply adding a
`noalias_result` annotation is not sufficient. Forcing the encoder inline
barely reduces instructions and does not remove its stores/allocation.

**Owner:** LowIR allocation/escape analysis, dead stores to private storage,
and allocation/free-pair elimination, with context-sensitive or inlined call
handling. Library allocation elidability is stronger than noalias and must
have a sound library/runtime model. No new machine instruction is needed.

### 3. Boolean slots: one general issue across several outliers

Myc GM declares short-circuit temporaries as `i1`, stores `i1`, but reloads
them as `i8` ([Myc bb.cr](https://github.com/kostya/myc/blob/d31bd5b0112d54471b544991be688be83607f40f/src/backend/gm/bb.cr)).
[`SlotPromotion::classify`](../../dev/src/opt/slot_promotion.cpp) accepts
matching types or same-width integer twins, explicitly excluding `i1`.
Consequently those private byte-backed booleans stay in memory.

The diagnostic rewrites only nonescaping slots whose uses are ordinary i1
stores and i8 loads. A load becomes `load i1` plus zero extension, preserving
the old result width. It does not assert facts about arbitrary byte memory.

| Focused case | Original instructions, B | Normalized instructions, B | Reduction |
|---|---:|---:|---:|
| Huffman decode | 29.143 | 19.276 | 33.9% |
| LogParser | 21.998 | 14.717 | 33.1% |
| Maze generator | 10.843 | 6.978 | 35.6% |
| Mandelbrot | 14.456 | 12.698 | 12.2% |
| Graph DFS | 6.866 | 6.472 | 5.7% |

Huffman improves from a paired 1.9204 to 1.5106 s. The normalized optimized
IR has no boolean slots and unrolls its eight-bit decode body, which Clang
also does. A remaining gap exists: Clang is 0.8247 s / 12.908 B instructions;
Myc LLVM is 1.0127 s / 17.050 B. Not all of that residue is GM-only.

Maze's exploratory focused times go from 3.5745 to 1.7858 s, versus Clang
1.8528 s. Clang simplifies `kind == 1 || kind == 2 || kind == 3` to an
unsigned range comparison and partially unrolls the neighbor loop. GM's
original hot code retains multiple compare/set/test branches. LogParser
improves from 2.0471 to 1.4738 s; aggregate normalization removes further
regex-state copies, leaving one necessary mask array.

Mandelbrot is scalar in Clang too; disabling vectorization makes essentially
no instruction-count difference. GM's hot compare/set/extend/test chain is
partly a consequence of the boolean memory form. Its backend already supports
direct floating compare/branch fusion; do not diagnose a missing x86 floating
branch instruction from the original assembly alone.

**Owner:** Myc GM lowering can emit consistent boolean accesses immediately;
GM's LowIR passes should also recognize safe equivalent byte-backed boolean
forms. Existing types and conversion instructions suffice. Do not treat any
i8 load as a boolean without a storage/value proof.

### 4. Hash: consume induction-range facts

The case named `Hash::SHA256` actually runs the source's eight-lane synthetic
hash recurrence, not a full SHA-256 compression round. Its main loop uses
signed `i % 8` for a counter starting at zero and increasing while below a
signed size. That counter and its executed increment cannot overflow.

Clang replaces remainder by a mask and partially unrolls the loop. GM emits
the signed power-of-two remainder correction sequence on every byte. In a
matched experiment, adding the valid existing `nowrap` fact to the increment
leaves instructions unchanged at 34.874 B. Replacing the remainder by its
proven equivalent mask reduces them to 26.954 B, **22.7% fewer**, and paired
time from 2.4912 to 1.6515 s. Clang in that block is 1.6611 s / 22.986 B.
The result is close in time but still higher in instructions; no claim of
full parity across hosts follows from this one block.

**Owner:** induction/range analysis and scalar simplification. No new IR fact
is required for this case: the proof is recoverable from the phi, step, signed
bound and exit. Emitting correct source overflow facts helps other cases, but
the isolated experiment shows that annotation alone is not the fix here.

### 5. BWT and arithmetic decode: native division selection

Both optimized LLVM IR and GM LowIR still contain unsigned 64-bit division
or remainder in their hot loops. Clang's machine code checks whether both
operands fit in 32 bits and uses `div r32` when they do, with a `div r64`
fallback. GM emits the 64-bit form unconditionally. This isolates a native
selection difference even after considering the LLVM IR.

A generic checked LowIR fast path was applied to the four variable unsigned
divisions in the two profiled functions. It preserves full-width semantics
and the zero-divisor behavior, and passes the normal-size benchmark checks.

| Case | Matched control seconds | Checked fast path seconds | Time saved | Instructions |
|---|---:|---:|---:|---|
| BWT encode | 2.4858 | 2.0940 | 15.8% | 8.419 → 9.062 B |
| Arithmetic decode | 2.7654 | 2.2586 | 18.3% | 9.109 → 9.940 B |

More instructions are justified here because they avoid a slower instruction
form. Clang remains faster (1.8477 and 1.6443 s in those blocks). BWT also has
loop/address and spill costs; arithmetic decode retains signed midpoint
correction in a nonnegative binary search. The checked path is not the entire
explanation of either case.

**Owner:** native instruction selection and its target cost model, optionally
fed by proven operand-width facts. It needs no new LowIR opcode. Measure it on
Intel and EPYC; avoid a global policy based on instruction count alone.

## Next tier and what remains uncertain

The initial profile pass also covered the following cases. These observations
are useful localization, not a completed causal decomposition of their residue.

| Case | Evidence | Next discriminating experiment |
|---|---|---|
| Maze AStar | 86% of samples in its run body; original has eight slots/eight object copies. Normalization removes those, but normal-suite wall time does not clearly improve. Hot code still has packed pair transfers, compare chains and stack spills. | Compare normalized machine IR live ranges/spills and range-folded walkability predicates against Clang; separate division from queue costs. |
| Graph DFS | LLVM hoists graph adjacency base/count loads before the inner loop. GM repeatedly loads them inside it; original also moves each two-int Pair through object temporaries. | Recheck after aggregate normalization; use shared allocation-disjointness facts in LICM and forwarding, then inspect remaining register spills. |
| Base64 decode | 99.9% in scalar decode code, four byte-table lookups and bit assembly. GM inlines it, Clang retains a helper. | Compare decoded-byte packing, address folding and live ranges without changing the workload; not evidence that more inlining is needed. |
| NeuralNet | GM does much more inlining; exp/libm is a material part of the profile. GM has 18.143 B instructions, Myc LLVM 17.001 B, Clang 16.348 B. | Compare inner dot-product loop and spills before increasing inline budgets; separate libm cost. |

Clang's no-vector control has essentially unchanged instruction counts for
LogParser, Maze, SHA256-labeled hash, Mandelbrot and DFS, and only a tiny change
for BWT on this host. Broadly adding vector instructions is not the best first
response to these outliers. Raytracer, conversely, shows that mixing packed
accesses into a not-yet-promoted aggregate can obstruct more valuable scalar
replacement. Preserve vector operations while fixing that interaction.

## Frontend facts versus LowIR extensions

The frozen and regenerated GM inputs contain **zero `nowrap` facts**.
Myc emits many typed loads but its scalar store emitter omits `typed`.
These are real opportunities to use the existing contract, with C aliasing,
union/member-punning and integer-promotion semantics respected. They must be
attached from typed source facts, not inferred by matching printed C or IR.

The contract already has typed memory, nowrap/exact arithmetic, nonnull,
function memory effects/repeatability and noalias results. None of the causal
experiments above establishes a need for a new public LowIR opcode or fact.
In particular:

- A slot/object canonicalization issue is not missing semantic information.
- The hash counter's range can be derived from existing control/data flow.
- malloc/free dead work needs an accurate allocation model and escape proof;
  `noalias_result` alone does not say a call is elidable or side-effect free.
- The dynamic narrow-divide path is fully expressible with current operations.

Do not expand LowIR preemptively. If allocation elimination later needs a
cross-frontend contract beyond recognized library semantics, design explicit
allocation/escape/elidability guarantees with negative controls. Likewise,
consider broader range or capture summaries only after a reducer demonstrates
that the optimizer cannot soundly derive the needed property.

## Implementation order and validation

1. **General object canonicalization and scalar replacement.** Support object
   `copy`, load/store and object-valued `copyobj` consistently; handle aliases
   and inliner-created snapshots before aggregate partitioning. Account for
   overlapping scalar/vector accesses without losing bytes or alias coherence.
   Raytracer is the largest absolute loss by a wide margin.
2. **Boolean memory normalization.** Fix Myc's emitter and teach safe
   nonescaping i1/i8 promotion to the optimizer. Check that this unlocks CFG
   simplification and bounded unrolling without globally raising budgets.
3. **Induction ranges.** Prove nonnegative no-wrap counters and apply those
   proofs to modulo, extension, comparison and midpoint simplification.
4. **Allocation-aware dead work.** Track escape/capture through calls and
   remove dead writes/allocation pairs with explicit lifetime/failure rules.
   Keep a consumed-payload Base64 control to distinguish real throughput.
5. **Native divide selection and remaining MIR costs.** Introduce checked
   narrowing only where the target cost warrants it; compare full-width,
   mixed-width and zero-divisor controls. Then examine the remaining spills
   and address-generation costs in Raytracer, AStar, DFS and BWT.
6. **Frontend fact emission.** Supply only facts justified by Myc's typed
   semantics; measure separately so benefits are not attributed to new schema.

Each production change needs small general reducers covering equivalent IR
forms and negative cases (escape, alias, overlap, volatility, signed bounds,
NaNs and padding as applicable). Run existing public performance workloads,
compile-time instruction and paired wall checks, and the repository's full
compiler/backend qualification. Avoid importing the diagnostic function-name
filters or expecting exact machine instructions in student fixtures.

No production optimization or student-facing change was made in this analysis,
so the complete course/debug-info/self-host release qualification was not run.
The evidence establishes useful priorities, not a claim that the remaining
gap is closed or a production patch is ready to merge.

## Reproduction and retained evidence

The raw dataset is `/mnt/vizier/cppgm-issue115-data/20261110`. It contains the
pinned source checkouts, immutable executables, exact input/optimized LLVM and
GM IR, disassembly, perf sampling profiles, and every run's stdout, stderr,
counters and identity JSON. [observations.json](observations.json) collects
all completed measurement records with binary hashes, raw-file hashes,
per-case checks/times and counters. [baseline-table.md](baseline-table.md)
contains the complete original-input comparison.

With `R` set to this worktree and `T` to the raw dataset:

```sh
make -C "$R/dev" cppgm++ lowir lowiropt lowir2native -j8 \
  CXX=g++ CPPGM_HOST_CXX=g++ CPPGM_WARNINGS_AS_ERRORS=1
"$R/dev/cppgm++" -O3 -c "$T/index-c.lowir" -o "$T/bin/gm-original.o"
clang --gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15 \
  "$T/bin/gm-original.o" -lm -lpthread -o "$T/bin/gm-original"
clang -O3 "$T/index-c/index.c" -lm -lpthread -o "$T/bin/clang-c"
gcc -O3 "$T/index-c/index.c" -lm -lpthread -o "$T/bin/gcc-c"
CPPGM="$R/dev/cppgm++" "$T/bin/mycc" "$T/index-c/index.c" c \
  --backend gm --final "$T/bin/mycc-gm"
"$T/bin/mycc" "$T/index-c/index.c" c \
  --backend llvm --final "$T/bin/mycc-llvm"
TEST=1 "$T/bin/gm-original"
python3 "$R/reports/issue115/measure.py" --output "$T/repeated-baseline" \
  --cpus "$(cat "$T/cpuset")" \
  a1="$T/bin/clang-c" b1="$T/bin/gm-original" \
  b2="$T/bin/gm-original" a2="$T/bin/clang-c"
```

For IR comparisons, Myc dump mode writes to stdout; its output path is not
a positional argument:

```sh
"$T/bin/mycc" "$T/index-c/index.c" d --backend llvm --final > "$T/ir/mycc-final.ll"
"$T/bin/mycc" "$T/index-c/index.c" d --backend gm --final > "$T/ir/mycc-final.lowir"
"$R/dev/lowiropt" -O3 "$T/index-c.lowir" -o "$T/ir/gm-original-o3.lowir"
```

The diagnostic scripts are small, inspectable transformations of separate
input files. Compile their outputs with the same compiler and link command,
run `TEST=1`, then use `measure.py --case CASE` for focused paired runs:

```sh
python3 "$R/reports/issue115/diagnostic_variants.py" "$T"
python3 "$R/reports/issue115/canonicalize_objects.py" \
  "$T/ir/gm-original-o3.lowir" "$T/ir/canonical-slots.lowir" --slot-loads
python3 "$R/reports/issue115/canonicalize_booleans.py" \
  "$T/index-c.lowir" "$T/ir/bool-slots.lowir"
python3 "$R/reports/issue115/sha_range_probe.py" \
  "$T/ir/gm-original-o3.lowir" "$T/ir/sha-mask.lowir" mask
python3 "$R/reports/issue115/division_probe.py" \
  "$T/ir/gm-original-o3.lowir" "$T/ir/narrow-divisions.lowir"
python3 "$R/reports/issue115/collect_results.py" "$T" "$R/reports/issue115"
```

The mixed-width experiment starts from `canonical-combined-second.lowir`:
canonicalize optimized object forms, normalize its boolean slots, optimize
that input once, then run `split_vector_memory.py`. Its control runs the same
number of O3 passes on the same input without splitting accesses. Repeating
the optimizer is an experimental control, not the recommended production
solution; the production fix belongs within the normal pipeline.

```sh
python3 "$R/reports/issue115/canonicalize_booleans.py" \
  "$T/ir/canonical-slots.lowir" "$T/ir/canonical-combined.lowir"
"$R/dev/lowiropt" -O3 "$T/ir/canonical-combined.lowir" \
  -o "$T/ir/canonical-combined-second.lowir"
python3 "$R/reports/issue115/split_vector_memory.py" \
  "$T/ir/canonical-combined-second.lowir" "$T/ir/split-vector-memory.lowir"
```

Compile `canonical-combined-second.lowir` as the matched third-pass control
and `split-vector-memory.lowir` as the variant, both with `cppgm++ -O3 -c`.
