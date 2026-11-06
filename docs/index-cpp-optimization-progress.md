# index-cpp optimization investigation records

Maintainer-only companion to [the plan](index-cpp-optimization-plan.md).
The original plan remains the scope; the first fix does not close the group.

## Measurement setup

Baseline compiler is the immutable `60e73459e` snapshot from
`/home/vishvananda/work/index-cpp/validation/20261105-comparison/`.
The compatibility branch's subsequent optional-test refactor changes no compiler
source. Upstream source remains pinned and unchanged.

Current receipts live under
`/home/vishvananda/work/index-cpp/optimization/20261105/`:

- `profiles/`: sampled cycles for the first three workloads on all three
  compilers, plus relevant machine code.
- `index-O3.lowir`, `lowir.stderr`: baseline optimized typed IR and work census.
- `variadic-inline-paired/`: all observations from A/A followed by three
  ABBA/BAAB blocks for Brainfuck recursion, Calculator AST and CRC32.
- `variadic-inline-safe-provenance.json`: corrected compiler/source hashes and
  byte identity of its object with the object used for runtime measurements.
- `variadic-inline-safe-frozen/`: source/header-validated frozen compiler
  instruction and memory checks at `-O0` and `-O3` for the corrected candidate.
- `public-baseline*`, `public-variadic-inline*`: existing public sentinels,
  built with the respective immutable compiler and both reference compilers.

The reusable `scripts/index_cpp_benchmarks.py` retains process counters, raw
output, hashes and calibration separately from upstream loop wall time. Full
inputs run during measurement; every executable first passes all 50 `TEST=1`
checks. Later receipts include host load snapshots and explicit drift warnings.

## Brainfuck recursion: unused variadic arguments

Baseline sampled cycles: 37.3% in a helper returning its first argument,
30.7% in variant dispatch, 19.8% in the recursive loop visitor and 9.9% in
the outer visitation wrapper. The first helper is a library lambda with a C
ellipsis, not a function that reads a `va_list`.

The baseline LowIR body is simply `return ptr %__v`, with a variadic boundary.
The native body nevertheless stores six GPRs and eight whole XMM registers,
allocates 224 stack bytes and keeps a frame pointer. The inliner treats every
variadic signature as unsafe, so it cannot remove this call.

The first candidate substitutes named parameters when a body contains no
`va_start`, `va_arg`, dynamic stack allocation or other existing cloning restriction.
Extra arguments remain evaluated in the caller. Existing exception, aliasing,
ABI, replacement, recursion and profitability checks continue to apply.
No budget was raised and no source/library name participates in admission.

Review found another fixed-arity assumption in folded-copy specialization:
unnamed argument positions could be mistaken for ordinary temporary IDs. A
cleanup-bearing reducer with many constant unnamed arguments made the initial
prototype return an incorrect result. Restricting constant substitution and
dispatch-target inspection to named parameters fixes the reproducer. Both
versions emit byte-identical index-cpp objects; the runtime observations apply
to the corrected candidate too. The unsafe prototype is not an accepted fix.

Coverage includes unused arguments with observable effects, floating/stack
arguments, a volatile input, a throwing argument with destructor cleanup,
`noinline`, real `va_start`/`va_arg`, and the folded-copy negative case. The
driver fixture requires removal of the small identity call and loose size
bounds, rather than a particular instruction sequence.

Measured speed against the baseline is about 1.71x in loop wall time and
1.825x in process instructions (45.2% fewer). The three instruction-count
ratios agree closely. Wall-time blocks range from 1.42x to 1.83x; A/A drift
reached 26%, so the precise wall-time gain needs a quieter confirmation run.

Remaining opportunities: collapse justified forwarding wrappers within a call
cycle without recursive expansion; inspect dispatch/state reloads after that;
omit register-save work for non-inlined variadic bodies that never use it.
Keep those mechanisms separate from the first inlining fix.

## Calculator AST: variant movement and destruction

Baseline sampling places substantial time in variant move construction,
destruction and dispatch, with parser routines and allocation also visible.
This workload constructs/replaces trees; it does not evaluate their visitors.
The unused-ellipsis helper appears in those variant paths as well.

The shared first fix measures about 1.22x loop-wall speed and 1.324x process
instruction speed (24.5% fewer instructions). Wall calibration drift was 4.1%;
the instruction counts are stable. Investigate the residual recursive ownership
and move/dispatch paths before adding scalar replacement or cleanup changes.
Preserve allocation observability, exception behavior and destructor order.

## CRC32: a confirmed loop-idiom gap

Baseline LowIR already unrolls the eight bit steps. Seven become selects; the
last remains a branch and phi. GCC also performs the arithmetic recurrence.
Clang emits a table lookup combined with a shift, processing two bytes per
outer iteration. This is a different transformation, not merely faster
instruction selection for the existing arithmetic.

An independent pointer/length reducer confirms that Clang's
`LoopIdiomRecognizePass` introduces the table (`crc-clang-passes.log`,
`crc-clang.ll`), without a special target ISA. A general implementation must
derive its table from the recurrence's typed operations and constants, prove
the rewrite for all input words, bound compile-time/table size, and cover
different polynomials, word widths and iteration counts. No benchmark table
or expected checksum may be embedded in the optimizer.

The first inlining fix leaves CRC32 process instructions unchanged; its
observed wall ratio, 1.004x, is within calibration noise.

## Compiler cost and checkpoints

The initial inlining candidate changes frozen compile instruction counts by
less than 0.003% at both levels, within A/A noise. Peak RSS changes by less
than 0.1%; both frozen object hashes are identical across baseline and candidate.
Compile CPU timings are noisy and cannot establish a wall-time improvement or
regression. The corrected candidate also changes instruction counts by less
than 0.005%, with identical objects; its peak RSS is within 0.2% of baseline.

Index-cpp `.text` grows from 387,154 to 391,618 bytes (1.15%). That cost must
remain visible alongside the runtime gain; existing PA35 size gates remain
unchanged. The baseline was already larger than the two references.

The initial prototype passed the full strict report (8,038/8,038), debug info,
self-host through PA5, PA35 quick/smoke and architecture/format/file audits.
The corrected candidate also passes the strict report, debug info, self-host
through PA5, PA35 quick/smoke and audits, including the stronger reducer. The
fresh student export verifies 22,517 portable reference files; all 42 reference
wrappers install, and both new fixtures pass through exported wrappers.

Six existing public sentinels cover recursion, containers, JSON, regex, hashing
and floating point. All pass with GCC and Clang references; binary sizes are
unchanged and counted instruction costs are unchanged within noise. FlatMap
objects at both levels and Xpressive traits at `-O0` are byte-identical to the
baseline. The Xpressive `-O3` object is 293 bytes smaller: six shared-pointer
constructor aliases shrink after an empty variadic helper disappears.

For Xpressive traits, 12 paired blocks with ten executions per observation
confirm exact output equality and 0.327% fewer process instructions. Precise
CPU/task-clock and cycle medians are within 0.6% of baseline, inside A/A drift;
there is no resolved timing regression. An earlier three-block estimate using
coarse CPU timings was unreliable. The raw attempts and final controls remain
in `boost-sentinels/`; `runtime-precise-v2/report.json` records the calibrated
check with an identical host-built test framework and a fixed input hash.

The first mechanism is pushed as `e6a636868`. Boost advisory CI passed; the
public benchmark workflow was still running at this checkpoint. This does not
close the first group or replace its required full-scoreboard comparison.

## CRC bit-recurrence folding

The second mechanism derives readonly data from closed chains of logical
shift/conditional-XOR operations. It accepts arbitrary constant polynomials,
8/16/32/64-bit words and normalized branch polarities. For `k` steps, split
the input into its low `k` bits and its remaining bits: the latter contribute
`x >> k`, while the table evaluates the former using the original operations.
An equal-width shift contributes zero. Narrow indices are zero-extended.
Variable polynomials, arithmetic shifts and observable intermediate uses do
not acquire this rewrite without the necessary proof.

Tables share a 4 KiB compilation-unit budget and are deduplicated by their
mathematical inputs. They are ordinary serialized LowIR globals. Explicit
addresses allow the existing loop-motion and native addressing logic to keep
the base outside a loop and combine the index with its load. No polynomial,
source spelling, benchmark name or expected checksum selects the rewrite.

Three paired blocks on the original full CRC workload give these
higher-is-better speeds:

| Reference | Loop wall speed | Process instruction speed |
|---|---:|---:|
| Previous accepted compiler | 4.223x | 6.279x |
| GCC | 3.108x | 4.978x |
| Clang | 1.034x | 0.723x |

Treat the Clang timing as parity: its small measured difference and block
spread do not establish a lead. Clang still executes fewer instructions;
its two-byte unrolling and addressing differ. CRC process instructions fall
84.1% against our previous compiler. The first prototype left the table base
inside the loop and measured only 0.802x Clang; exposing the address to the
existing machinery removed that deficit without raising general loop budgets.

Index-cpp `.text` falls from 391,618 to 391,234 bytes, while `.rodata` grows
by 1,008 bytes including alignment changes. All 50 upstream checks pass.
Brainfuck and Calculator AST instruction counts are unchanged by this fix;
their timing blocks are noisy and show no resolved new effect.

The final implementation batches substitutions and retains original block,
predecessor and use facts while matching. It avoids a whole-function scan per
recurrence. Independent reductions with 16, 128 and 1,024 recurrences compile
with identical output to the initial implementation, at compiler instruction
speeds of 1.058x, 1.670x and 6.641x respectively. A host-built arithmetic
oracle also checks the large reduction across 256 seeds.

Frozen compiler instruction cost grows 0.018% against the campaign baseline
and 0.022% against the previous accepted compiler at `-O3`; `-O0` stays within
0.002%. Objects remain identical and peak RSS stays within 0.25%. Linux RSS is
the available memory metric here; these observations do not measure physical
footprint. Raw CPU timings vary too much to resolve a small compile-time change.

The final candidate passes the strict report (8,040/8,040), debug-info checks,
self-host through PA5, PA35 quick/smoke and architecture, placement, format and file
audits. The final frozen checks and direct/serialized source-object comparison
preserve identical output.
The optional C++14/17 lane also passed. Course tests use broad size bounds and
independent behavior drivers: exhaustive byte inputs, wider seeds, multiple
polynomials, affine polarities, equal-width shifts, chained reductions and
negative legality cases. They prescribe no table or instruction sequence.

All six existing public sentinels pass against GCC and Clang; counted native
instructions and executable sizes are unchanged. Recompiling their objects
with the final batching implementation preserves every byte. FlatMap and
Xpressive objects are also unchanged at both optimization levels. The final
compiler emits an index-cpp object identical to the measured candidate, so
the retained executable and runtime receipts apply to that final code too.
The refreshed student export verifies 22,523 portable reference outputs;
all 42 wrappers install and both new fixtures pass through the exported tools.

Receipts remain under `optimization/20261105/`: `bit-recurrences-v3-paired/`
contains native measurements; `bit-recurrences-v4-{frozen,previous-frozen}/`
contain compiler guards; `many-recurrences-v4-paired/` records scaling.
`bit-recurrences-v4-provenance.json` identifies the final compiler, sources,
object and reused identical native executable. Public/Boost object checks and
student-export checks have separate receipts in that directory.

## Recursive loop dispatch: bounded call-cycle collapse

The current Brainfuck profile attributes about 55% to the variant dispatcher,
26% to loop work and 17% to forwarding/invocation layers. The loop still called
a checked visitor wrapper and a separate dispatcher; GCC and Clang combine
those layers with the loop. The ordinary inliner rejected every member of a
recursive call-graph component before considering its existing size budgets.

At an optimized loop call site, allow a callee in the caller's component when
the callee is loop-free, has no exception region, has no direct self call and
has exactly one onward component call. Retain the actual recursive worker,
the ordinary no-inline, effect and linkage checks, and every existing growth
budget. Refresh these call counts with the existing function-shape scan after
edits, so a newly exposed worker cannot be mistaken for the original forwarder.
No source spelling or workload identity selects this rule.

The first prototype also considered sites outside loops. It helped Calculator
AST, but increased frozen `-O3` compile instructions by 1.60%. Restricting the
rule to loop sites preserves the Brainfuck gain at 0.28% extra compile
instructions against the preceding compiler and 0.30% against the campaign
baseline. `-O0` instructions and object bytes are unchanged. Peak RSS stays
within 0.2%; physical footprint is not available on this Linux host. A longer
`-O0` timing repeat did not reproduce an initial apparent slowdown. Broad
non-loop forwarding remains an opportunity requiring a more efficient design.

For Brainfuck, three paired blocks give 1.300x instruction speed (23.1% fewer
instructions) against the preceding compiler and approximately 1.44x loop wall
speed. The wall blocks vary substantially, so the instruction reduction is the
firmer result. Separate stock-reference pairs give 0.856x GCC and 0.847x Clang
wall speed, with instruction speeds of 0.888x and 0.810x. There is still a
dispatch/layout gap; this fix does not establish parity with either compiler.
Calculator AST and CRC instruction counts are unchanged by the loop-site rule.
Index-cpp `.text` grows by 16 bytes to 391,250, with unchanged read-only data.
All 50 upstream checksum workloads pass.

The final source passes the strict report (8,042/8,042), debug-info checks,
self-host through PA5, PA35 quick/smoke, the optional C++14/17 suite and all
architecture, placement, format, file and harness audits. GCC and Clang pass
the CI `-Wall -Werror` syntax check. C++11 reducers exercise observable argument
evaluation, recursive continuations, fan-out, no-inline functions and unwinding;
they check broad call/size bounds and behavior through the serialized object
path. Student guidance describes the proof obligations without private tuning.

The six public sentinels pass against both reference compilers. FlatMap objects
remain identical at both levels, as does Xpressive at `-O0`. Xpressive `-O3`
changes, so it was linked against the same host-built test framework and run
in 12 paired blocks of ten executions each, with A/A checks before and after.
Its instruction speed is 0.99987x and task-clock speed 1.00035x: no resolved
runtime regression. JSON also changes; six paired blocks show 1.021x instruction speed. Its A/A
timing drifts by 15–42%, so these runs do not resolve a small wall-time effect.
The fresh student export verifies 22,529 portable reference outputs and
installs all 42 wrappers; both new fixtures pass through the exported tools.

Receipts are under `optimization/20261105/scc-dispatch-prototype/` and
`scc-dispatch-ported/`; the latter's manifest identifies the final compiler,
source hashes and object. The final object matches the isolated loop-site
prototype, so the native executable is its unchanged copy. Compiler guards are
in `scc-dispatch-ported-{frozen,cumulative-frozen}/`; Xpressive's raw repetitions
are in `scc-dispatch-ported-boost-sentinels/runtime-precise/`.

## Outstanding work

Close the first group with a full 50-workload paired reranking, the full public
benchmark comparison and PA35 release gates. Profile Brainfuck's residual call
chain and Calculator AST's remaining ownership/dispatch work on the new code.
Investigate Base64 decoding, Huffman decoding and Nbody, then the named parser,
interpreter, SHA256, maze and container workloads using that refreshed evidence.
Retain existing workload wins and every repeatable individual regression.
