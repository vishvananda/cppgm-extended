# index-cpp optimization investigation records

Maintainer-only companion to [the plan](index-cpp-optimization-plan.md).
The original plan remains the scope; the first fix does not close the group.
See [refreshed target diagnoses](index-cpp-target-diagnoses.md) for current
IR/MIR evidence, rejected explanations and the remaining proofs.

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

## First-group checkpoint

Three optimizer mechanisms are pushed through `8b3b6efc6`: unused-ellipsis
inlining, bit-recurrence folding and bounded recursive loop dispatch. Their
all-50 comparison against the original campaign compiler gives a 1.06547x
equal-weight geometric instruction speed, or 6.145% fewer instructions. The
largest changes are recursive Brainfuck (2.373x instruction speed), CRC32
(6.279x) and Calculator AST (1.324x). Frozen `-O0` cost remains unchanged;
the cumulative `-O3` instruction cost is about 0.30% higher.

The completed full timing comparison is still provisional: A/A checks and individual
blocks show substantial host variation. Preserve the raw results, including
apparent slowdowns, rather than treating the overall mean as proof that every
program is safe. It measures 1.004x GCC and 0.949x Clang overall, with
instruction speeds of 0.950x and 0.952x respectively. A/A warnings occur in
26 of 50 GCC pairs and 20 of 50 Clang pairs. Results are under
`optimization/20261105/group1-scc-full-scoreboard-isolated/`.

The fourth mechanism uses an existing LowIR fact: a switch successor containing
only `unreachable` cannot occur in a defined execution. Discard such arms before
pricing dispatch, and omit a table's range check only when the default has that
same proof. A call followed by `unreachable` can still perform observable work
or unwind, so it does not qualify. This changes optimized native selection;
the `-O0` path and the serialized LowIR contract stay unchanged.

This reduces Brainfuck instructions by another 8.40% and Calculator
AST instructions by 1.44%; CRC instructions are unchanged. Index-cpp `.text`
falls from 391,250 to 389,538 bytes. Independent host calls check narrow signed
and unsigned arguments, defined defaults, throwing defaults and varied inputs.
Course tests use behavior and broad dispatch bounds. All 50 checksums, the
8,043-test strict report, debug checks, self-host through PA5, PA35 quick/smoke,
optional modern-C++ checks and architecture/placement/format/file audits pass.
The harness also passes with its normal environment: an initial invocation
inherited strict text-comparison mode and failed its relaxed-mode controls;
both attempts are retained. All six public sentinels and the FlatMap/Xpressive
objects are byte-identical to the preceding compiler at both optimization
levels. Frozen compiler instructions and object bytes are unchanged. A longer
CRC repeat gives 1.006x wall speed across nine paired blocks, with unchanged
instructions and about 1% A/A drift: no resolved runtime change. It does not
reproduce the earlier apparent slowdown. The fresh student export verifies
22,535 portable reference outputs, passes its document audits and installs all
42 wrappers; the new fixture also passes through the exported tool bundle.
Receipts are under `switch-unreachable-prototype/`,
`switch-unreachable-ported/` and `switch-unreachable-frozen/`.

## Direct forwarding without body copying

The fifth mechanism redirects calls through a function whose body consists of
one call and its return. Resolve shared chains once and compose parameter
permutations, constants and symbol arguments. Keep every original argument's
evaluation and lifetime work. Require compatible scalar boundaries, retain
materialization and object-copy semantics, honor no-inline and replaceable
definitions, and reject pure forwarding cycles. The final argument count
cannot exceed the original call boundary, which bounds both composition and
rewriting. No body cloning or caller simplification is needed. The retained
function's address and external ABI are unchanged; ordinary pruning still
decides which unobserved definitions may disappear.

An earlier broad recursive inlining prototype cost 1.60% extra frozen compile
instructions. This implementation adds about 0.015% against the preceding
compiler, with cumulative `-O3` cost about 0.31% above the campaign baseline.
`-O0` instructions and objects remain unchanged; peak RSS stays within 0.2%.
Nine additional paired compile blocks with precise CPU/task-clock measurements
do not reproduce an initial apparent timing loss: median CPU speed is 0.997x,
inside A/A variation. Preserve every observation, including busy-sibling cases.

The final port emits an index-cpp object identical to the measured combined
prototype. Against the preceding switch fix, six paired blocks give Calculator
AST 1.058x instruction speed and approximately 1.129x wall speed; recursive
Brainfuck gives 1.008x instruction speed and 0.996x wall speed. A controlled
fixed-ASLR diagnosis also measures Brainfuck at parity. Ordinary-ASLR results
remain the primary comparison. Earlier forwarding-only attempts had an
unresolved Brainfuck timing loss and remain archived; accept the combined
implementation's receipts, not those earlier attempts. Index-cpp `.text`
falls another 528 bytes, to 389,010.

The strict report passes 8,046 tests, together with debug checks, self-host
through PA5, PA35 quick/smoke, optional modern-C++ checks and every required
architecture, placement, format, file and harness audit. GCC and Clang pass
`-Wall -Werror`. Independent source reductions pass all four optimization
levels with both reference compilers, the previous compiler and the candidate;
eight explicit LowIR paths also pass. They cover argument permutations,
effects and exceptions, narrow conversions, object snapshots, nontrivial
copies and destruction, no-inline boundaries, and strong host replacement of
a weak definition. Course tests use behavior and broad quality outcomes;
direct/serialized source objects also agree for the new reduction.

All six baseline sentinel recompiles reproduce the preceding compiler's
objects at both levels. The candidate changes only JSON at `-O3`. It keeps identical output and
executes 0.097% fewer instructions. Nine paired timing blocks give 0.989x CPU
speed, inside the 2–6% A/A variation; this does not establish a runtime change.
FlatMap and Xpressive `-O0` objects stay identical. Xpressive `-O3` changes, so
12 blocks of ten executions each compare against the same host-built framework:
instruction speed is 1.00012x and task-clock speed 1.004x, inside calibration
variation. The fresh student export verifies 22,541 portable reference outputs,
installs all 42 wrappers and passes both new fixtures through the bundle.

Receipts are in `forward-calls-ported/`, its two sibling frozen-guard directories,
`forward-calls-ported-public-objects/` and `forward-calls-ported-boost-sentinels/`.
The final all-50 comparison and full public run use this immutable compiler.
The all-50 comparison, full PA35 release check and full public run have now
finished.

## Five-fix comparison

All 50 checksum workloads pass for every measured executable. The final
comparison pairs each workload independently, with three separate CPU lanes
for single-threaded cases; the threaded matrix cases then use 18 CPUs.
Every reference remains its unchanged campaign executable.

| Reference | Geometric loop wall speed | Geometric instruction speed | A/A warnings |
|---|---:|---:|---:|
| Original campaign compiler | 1.058x | 1.06969x | 21/50 |
| Previous switch fix | 1.024x | 1.00166x | 32/50 |
| GCC | 0.998x | 0.95402x | 30/50 |
| Clang | 0.961x | 0.95603x | 23/50 |

The instruction result establishes a 6.515% geometric reduction against the
campaign baseline. Recursive Brainfuck executes 61.71% fewer instructions,
Calculator AST 29.66% fewer and CRC32 84.07% fewer. The many A/A warnings make
small wall-time differences provisional. Both the aggregate and individual
slow cases remain in the retained results.

An unusually low Brainfuck/Clang timing in that full run does not reproduce in
the longer focused control. Nine blocks, with two A/A blocks before and after
each comparison and recorded CPU-sibling activity, give Brainfuck wall speeds
of 1.001x the earlier SCC fix, 0.870x GCC and 0.823x Clang. The switch/forwarding
changes reduce its instructions another 9.1% against the SCC fix, while runtime
is at parity. There is still a latency gap against both stock compilers.
These controls are in `group1-final-brain-control/`; the complete 200-pair
scoreboard, calibrations and raw observations are in
`group1-final-forwarded-scoreboard/`.

The full PA35 release recipe passes for both the course build and the student
build, with 100 generated seeds and the existing release margins. Calibration
outputs and all measurements are retained under `forward-calls-ported/pa35-release/`.

The completed public suite passes all 81 programs at both optimization levels
against GCC and Clang. Against the preceding full run (the first three fixes),
79 optimized executables are byte-identical. Only JSON and tramp3d change.
All program outputs agree, all unoptimized executable sizes agree, and the
43 counted native workloads have unchanged aggregate instruction cost within
0.00001%. Across the 13 counted compilations, geometric instruction cost grows
0.0225%; the largest increase is JSON at 0.262%, below the investigation trigger.

The new public runtime CPU scoreboard gives 1.006x GCC and 0.993x Clang, versus
0.983x and 0.974x in the preceding run. These cross-run changes cannot establish
a gain: most executables are unchanged, and load variation remains substantial.
JSON's focused control is described above. For tramp3d, nine paired blocks with
two A/A blocks before and after give 1.002x CPU speed, 0.997x task-clock speed
and unchanged instructions. Calibration drifts by up to 4%, so there is no
resolved runtime regression. Receipts are `forward-calls-ported/public-full-v2*`,
`public-full-comparison.json` and `tramp-parent-paired/` in that directory.

## Narrow consumers: redundant extension cleanup

The sixth optimizer/backend mechanism is committed as `3a87da68c`. The existing
local MIR cleanup can omit an in-place 32-bit zero-extension when the next
calculation reads only the low bits and itself clears the destination's upper
half. Admission follows the actual emitted width. Memory operands, wider
operations and conditional moves do not qualify; shifts and rotates require
a nonzero constant effective count. A possible zero count can leave the
destination unchanged. Flags and every other operand remain unchanged.

This uses existing typed widths and the existing linear cleanup, with no new
allocation or optimization wave. The C++11 driver covers signed byte/word
inputs, arithmetic, shifts/rotates, wider live values and address consumers.
An independent GCC-built caller checks GCC, Clang, the preceding compiler and
the candidate at O0 through O3: all 16 combinations agree. The LowIR reducer
also preserves zero/variable shifts and wider readers. Its aggregate arithmetic
function permits 15 instructions: the candidate uses 12 and the older backend
uses 16. The bound allows different instructions, registers and scheduling.

Nine paired blocks on Base64 decoding give 1.03104x instruction speed, or
3.01% fewer instructions, against the preceding compiler. Loop wall speed is
1.0245x, within A/A variation of about 3.4%, so the instruction reduction is
the firmer result. Stock-reference wall speeds are 0.612x GCC and 0.681x Clang;
instruction speeds are 0.720x and 0.813x. A substantial gap remains.

Index-cpp text size stays at 389,010 bytes. Only the two Base64 helpers and one
regex helper change bytes or addresses among the executable's defined
functions. The regex control gives 0.999x loop wall speed and unchanged
instructions: no resolved regression. Huffman's decode function is byte- and
address-identical; its repeat varies within calibration noise. All six public
sentinels, FlatMap and Xpressive objects are byte-identical at both levels.

Frozen compiler instructions are unchanged within 0.003% against the preceding
compiler; cumulative O3 instruction cost remains about 0.31% above the campaign
baseline. Frozen objects agree and peak RSS remains within 0.25%. A short
compile-time estimate suggested a slowdown, but longer nine-block controls
did not reproduce it. O0 task-clock speed is 1.045x with 14% A/A drift; O3 is
1.009x with 13% drift. On the O0 control, the SMT sibling was busy 21–100% of
the time and its activity correlates with cycles per instruction (0.89).
These timings cannot resolve a small compiler throughput change.

All 50 checksum workloads pass, and the direct/serialized index-cpp objects
match. The 8,048-test strict report, debug-info checks, self-host through PA5,
PA35 smoke/quick, optional modern-C++ checks, architecture/placement/format/file
audits, harness checks and GCC/Clang `-Wall -Werror` checks pass. Receipts are
`narrow-consumer-final/`, its frozen-guard sibling directories and its public
and Boost object comparisons. The fresh student export verifies 22,547 portable
reference outputs, installs all 42 wrappers and passes the new native fixture
and all 12 driver paths through the exported tools.

## Refreshed investigation queue

The completed five-fix reranking keeps Base64 decoding, Huffman decoding and
Calculator AST among the largest remaining gaps. Huffman's profile and typed
LowIR show repeated bit-extraction work and branch/load diamonds; Clang selects
the child address before loading. Select formation currently excludes loads
from speculation, correctly: a possible improvement must move the one selected
load, rather than speculating both reads. The first narrow-extension improvement
is now accepted; further Base64 work remains warranted by the stock comparisons.

A private selected-address/load prototype passes all 50 checksums and preserves
whole-index-cpp direct/replayed object identity. A three-block screen gives
about 1.049x Huffman wall speed, but instruction speed is 0.881x: it executes
13.5% more instructions. Branch count falls, while measured branch misses do
not improve. Frozen compile instructions grow only 0.05% at O3, with unchanged
objects. This is evidence for further work, not an accepted transformation.
An initial hypothesis about lost compare/select fusion does not hold for this
input: extending operand lifetimes across pure address setup emits an object
and executable byte-identical to the first prototype. Different timing results
for those identical programs underline the remaining measurement variation.
Inspect the emitted address selection and reduce its extra work before
acceptance; stronger correctness checks also remain outstanding. Receipts are
`huffman-select-load-v{1,2}/` and `huffman-select-load-v1-frozen/`.

Nbody's refreshed gap is smaller than the initial noisy estimate. Its movement
helper is already inlined and its hot loop already uses packed SSE2 arithmetic.
Missing inlining or wholly absent vectorization is not its cause. Profiles and
disassembly are retained in `group2-{huffman,nbody}-discovery/` for the next work.

A broader whole-index-cpp LowIR replay probe exposed a pre-existing byte-identity
difference: it reproduced with both the preceding compiler and the candidate,
although both replayed programs pass all 50 checksum checks. Match the
`__OPTIMIZE__` preprocessing epoch when comparing paths; that alone does not
resolve the difference. The investigation below isolates and fixes this
boundary gap before using that replay to assess new optimizations.

Private tracing finds identical canonical input text on both paths and the
same live instruction count in every function throughout optimization. After
normalizing local names and phi-input order, all 1,550 final function bodies
also agree. Comparing the actual ELF sections then resolves the question:
every code and data byte agrees, as do every defined symbol's name, type and
size. Only symbol ordering, string ordering and references to symbol indexes
differ. This is not a register-allocation or generated-instruction gap.

Equivalent function entries share another definition's placement and have no
code range of their own. Object symbol registration handled ordinary code
ranges first, then mixed shared entries with globals in program order. The
LowIR writer groups globals before functions; direct lowering can interleave
them. Registering shared entries after ordinary functions and before globals
removes that accidental order dependency. It adds no sorting, allocation or
whole-program optimization pass. No tracing or environment reads were added
to the production compiler.

A C++11 constructor/destructor reduction fails byte identity at O1 through O3
with the preceding compiler and passes at every level with the fix. Both paths
also execute correctly. The whole index-cpp direct and replayed objects now
match exactly; code and data remain identical to the accepted five-fix object,
and all 50 checksum workloads pass. Frozen compile instructions and objects
are unchanged at O0 and O3, with peak RSS within 0.05%. Receipts are
`shared-entry-order/` and `shared-entry-order-frozen/`.
The linked executable's allocated sections also agree with the accepted
executable except for its build ID, so the retained native timings still apply.
The fix passes the 8,047-test strict report, debug-info checks, self-host
through PA5, PA35 smoke/quick, optional modern-C++ checks, harness checks and
all architecture, placement, format and file audits. Both GCC and Clang pass
the changed source's `-Wall -Werror` check.
The fresh student export verifies 22,541 portable reference outputs, installs
all 42 reference wrappers and passes the new roundtrip fixture through the
exported compiler. The fix is committed as `cc0e4f763`.

The earlier private narrow-consumer oracle is retained in
`group2-next-u32-oracle/`; the accepted implementation and fresh controls are
described above.

## Bit-indexed conditional field loads

The refreshed Huffman profile puts over 94% of execution in the decoder for
all three compilers. The decoder already uses raw pointers and an eight-bit
unrolled loop. Its remaining conditional child accesses compute two field
addresses and branch to one load. Clang instead uses the tested bit as an
address index. The missing opportunity is address formation, not reference
counting or absent loop unrolling.

The new rule proves a one-bit test from typed AND/shift/comparison operations,
then checks two constant indexes with the same base and element type. A
power-of-two byte distance permits a scaled bit, optional polarity reversal
and constant bias. The computed index uses the original base and retains no
field-specific or speculative arithmetic facts. A compatible ordinary read
can be factored through an acyclic, effect-free diamond only when each load
feeds the single merge result. Volatile accesses, exceptional regions and
load-dependent traversal latency retain their existing safeguards. The merge
result becomes the one load in place; no extra optimizer pass or analysis is
introduced.

An earlier generic address-select/load prototype increased instructions by
13.5% and was rejected. Requiring the indexed-address proof avoids that cost.
A proposed compare-fusion change produced identical code and was also rejected.
The accepted source matches the measured final prototype's benchmark object.
Nine paired blocks, bracketed by A/A controls, show instruction speed 1.01538
against the preceding compiler (1.51% fewer instructions). The observed wall
ratio is 1.16559, but calibration ranges from 0.74 to 1.23, so it is not a
precise timing claim. The paired wall/instruction comparisons remain 0.982/0.838
against GCC and 0.777/0.781 against Clang; a residual Clang gap remains.
Executable `.text` decreases from 389,010 to 388,946 bytes.

Independent checks cover 1,840 typed LowIR cases across 640 inputs and both
host callers at O0 through O3, plus 20 source/replay/compiler paths through a
cyclic traversal oracle. All pass. C++11 course fixtures exercise shifted,
inverted and high-bit tests, negative offsets, unequal strides, selected
volatile reads, cyclic and load-dependent traversals and null unselected
pointers. Their quality bounds allow equivalent implementations. The complete
index-cpp object is identical through direct and serialized LowIR paths,
and all 50 checksum workloads pass.

Frozen compile instructions change by about +0.002% at O0 and +0.025% at O3;
objects are unchanged on that input and peak RSS stays within 0.1%. Longer
controls show paired CPU speeds 0.995 at O0 and 1.001 at O3, within the
bracketing A/A variation; all initial observations are retained.
All six public sentinels and both Boost container/Xpressive sentinels produce
identical objects at both levels. The strict report passes 8,051 tests;
debug-info, self-host through PA5, PA35 smoke/quick, optional modern-C++,
architecture, placement, formatting, file and harness checks all pass.
Receipts: `huffman-indexed-ported/`, `huffman-indexed-ported-frozen/`,
`huffman-indexed-choice-matrix-v6/`, `huffman-indexed-source-oracle-v6/`,
`huffman-select-load-v5/runtime/` and the ported public/Boost sentinel folders.
The fresh student export verifies the portable reference outputs, all 55
student documents and 42 wrappers. The new LowIR, host and object-roundtrip
fixtures also pass through its installed reference tools. Export receipts are
`huffman-indexed-ported/export-results-v2.json`; the export is
`student-export-huffman-indexed-v2/`.

## Continuous index-cpp reporting

A separate `index-cpp benchmarks` workflow now runs on pushes and PR updates.
It builds the pinned upstream with all three compilers, requires every checksum,
pairs compilation and full-input runtime measurements with A/A calibration,
and records code size, peak RSS, available counters and source/header/binary
provenance. The detail table contains all 50 workloads and separate higher-is-
better GCC and Clang comparisons. It does not change the public-suite aggregate
or the default C++11 student requirements.

PR jobs retain artifacts without publication credentials. A separate trusted
main job publishes the table, result JSON and raw observation JSONL alongside
the public benchmark data, retaining per-commit history. Incomplete, dirty,
wrong-revision or wrong-host results cannot replace a published comparison.
The public benchmark page links to the new detail table.

Local validation exercised a real pinned network fetch, repeated builds,
all three checksum runs, both full-input A/A and ABBA comparisons, report
generation, failure retention and publication persistence/rejection tests.
Nine new CI tests and four measurement-runner tests pass, as does actionlint.
The full harness and existing public reporting tests also pass. The first real
trial exposed loss of Clang's C++ driver configuration when resolving its
symlink; compiler invocation now preserves that name and directory while
hashing its binary. The failed receipt is retained.

The successful local trial uses one paired/A/A block per reference; CI defaults
to three paired blocks. It records 16 compile and 19 runtime/check observations.
Its modified checkout is explicitly marked. Timings warn on 31 GCC and 44 Clang
workloads, so this validates the pipeline rather than replacing the calibrated
optimization scoreboard. Receipts are `index-cpp-ci-local-validation-v2/`,
`index-cpp-ci-fetch-validation.json` and `index-cpp-ci-harness.log`.
The [GitHub run of `2b7b4840a`](https://github.com/vishvananda/cppgm-extended/actions/runs/42194810727)
also passes: all 50 workloads validate on an AMD EPYC 9V45. The generated-code
speed aggregates are 0.990× GCC and 0.992× Clang. Hardware counters are explicitly
unavailable on that runner. The retained artifact also passes the publisher's
revision/host/completeness checks and writes both the latest table and per-commit
history in a local publication test; publication to the benchmark branch runs
when this workflow reaches main. Receipts are `index-cpp-ci-github-2b7b/`.
The first GitHub attempt exposed Git's mounted-checkout ownership check;
the workflow now trusts its exact workspace, with a regression test verifying
that a sibling checkout remains untrusted.

A further private logical-shift selection experiment passes all checks but
leaves Base64 and SHA256 runtime instruction counts unchanged. Admitting a
signed carrier does not change its benchmark object; the captured optimized
input uses unsigned carriers for those shifts. Neither variant is accepted.
A bounded retry of values rejected before another callee-save register opens
also leaves the Base64/Huffman instruction counts unchanged; it remains a
diagnostic prototype. Fresh warm-function disassembly corrects an older GCC
capture accidentally overwritten by its cold clone: GCC's small string wrapper
calls a separate decoder, so its wrapper size alone is not an inlining result.
The remaining Base64 diagnosis must include that worker and register pressure.

## Correctness follow-up: scalar slot value reads

Review of the next store-elimination candidate exposed a pre-existing LowIR
correctness bug. A scalar `$slot` value reads memory where it appears, but
SSA reuse and memory forwarding could overlook that implicit read. Small
reducers returned 0 instead of 11, 3 instead of 14, and 0 instead of 6 at
`-O3`; the previous compiler also fails them. The optimizer now makes scalar
value reads explicit in its existing operand-materialization walk before
analysis, retaining storage/address operands and aggregate boundaries.
Direct and variadic calls preserve value-versus-storage passing.

A required PA32 LowIR fixture covers reads across writes, calls, conversions,
floating arithmetic, indexing, wide shifts and control flow. Its behavior
fails on the old compiler at every optimized level and passes at all four
levels after the fix. Independent host-driver checks cover 140,000 varied
integer cases per level and 20,000 mixed direct/variadic argument cases per
level. One older structural control accidentally passed a slot value where
it intended an address; its explicit-address repair retains the original
promotion proof and passes its native behavior check. The LowIR handout and
PA32 design notes explain the distinction without prescribing a pass shape.

All 13 gates pass, including the strict 8,052-test report, debug info,
self-host through PA5, PA35 smoke/quick, modern-C++ tests, audits, harness,
and GCC/Clang warnings-as-errors. All 50 index-cpp checksums pass; source and
serialized object paths agree. The 74 public sentinel objects and four Boost
sentinel objects build successfully. Changed public programs retain their
outputs; Boost objects are unchanged. JSON and RE2 native instruction counts
are unchanged within calibration. A 10-repeat RE2 CPU control does not
reproduce its initial instrumented timing loss; every observation is retained.

Against the seventh accepted compiler, frozen compile instructions increase
0.01% at `-O0` and 0.07% at `-O3`; peak RSS is within 0.1%. The longer CPU
controls are 0.998× and 1.005×, both within A/A variation. Cumulative `-O3`
compile instructions are 0.40% above the original compiler, below the 0.5%
investigation trigger. Index executable text remains 388,946 bytes. Timings
remain noisy, so these are correctness and cost controls, not a claimed
native speed improvement.

A fresh student export verifies all 22,556 reference outputs and 9,038 exit
statuses, the document inventory and installed reference tools. The new
fixture also passes through its reference wrappers. The export caught one
expected reference change: a scalar return through cleanup storage can now
promote to a phi. That reference was regenerated normally and its behavior
rechecked; both the failed export receipt and the successful rerun are kept.

Receipts are `slot-value-reads-v4/`, `slot-value-width-oracle-v4/`,
`slot-call-value-oracle-v4/`, `slot-value-reads-v4-frozen/`,
`slot-value-reads-v4-cumulative-frozen-v2/` and the public/Boost sentinel
folders; export receipts are `export-results-v5.json`, with the export at
`student-export-slot-values-v5/`. The eighth optimization was revalidated on
this corrected memory model as described below.

## Eighth mechanism: remove an unobserved overwrite through an exact address

The SHA-named workload is a byte-wise eight-accumulator hash, rather than a
cryptographic SHA implementation. Its hot loop stores an intermediate value
through a dynamic index and then replaces that value through the same address.
The existing memory pass could remove covered writes to escaped local slots,
but required a fixed slot offset. The optimized host-object LowIR therefore
retained a write whose address identity already proved it unobservable.

Extend that proof to equal address values and equal scalar access types.
Potential alias reads, calls, exceptions, volatile and atomic accesses stop
the proof; unsupported aggregate or extended-precision stores conservatively
stop it too. Partial writes cannot replace a larger write through this rule.
Integer address literals compare their numeric address, so serialization's
literal numbering cannot affect the result. Retain the existing bounded local
search and collect eligible blocks during the memory walk already performed.
An earlier prototype scanned every block and cost 0.24% compile instructions;
collecting blocks with multiple stores reduces that overhead substantially.

The baseline compiler hash is
`e22b4b8d6fd7754e322629cc8ee99453fdf375c9898b44919c727b357498ceaf`;
the accepted compiler is
`4f68ce2264d1fdb6f8b6c02f50a203bea8eec033e49523e657c5464e22957927`.
The resulting index binary hash is
`e66430bfe837a47223c6ff62317f5044d3c9039da8df3dda22acb6a7f04a18e2`.
All 50 checksum workloads pass and direct/serialized object bytes agree.
Required PA32 LowIR, host-callback and object-roundtrip fixtures cover the
general rule and its barriers. Independent source checks exercise 1,024 seeds,
four alias gaps, nine observation cases, dynamic indices, throwing callbacks,
floating NaN payloads and signed zero across all optimization levels and both
object paths. Separate literal-address checks also pass.

In three paired blocks with A/A calibration, the hash uses 5.55% fewer native
instructions and gives 1.118× process CPU speed against the corrected baseline.
Its instruction speeds are 1.058× GCC and 0.941× Clang. Base64 instructions are
unchanged, with timing differences within calibration. Index executable text
falls from 388,946 to 386,626 bytes. Frozen compile instructions are unchanged
at `-O0` and increase 0.024% at `-O3`; cumulative `-O3` cost is 0.43% above the
original compiler, below the investigation trigger. Peak RSS remains within
0.2%. All 74 public sentinel objects and four Boost objects build; changed
programs preserve their output. Paired controls of changed public programs and
the container/Xpressive sentinels show no resolved native loss.

All 13 validation gates pass, including the strict 8,054-test report,
debug-info checks, self-host through PA5, PA35 smoke/quick, modern C++, audits,
harness and GCC/Clang warnings-as-errors. Inception preserves compiler identity.
The full PA35 release passes all 17 original gates. Its first student scaling
measurement missed the release margin (2.49); one repeat with the same immutable
seed on fixed affinity gives 2.25, and the unchanged release judge passes.
Both receipts are retained. No threshold was loosened.

A fresh student export verifies 22,562 portable references and 9,041 exit
statuses, installed tools, and the new raw, host and roundtrip fixtures.
The export caught four expected reference changes from removing overwritten
stores; normal reference regeneration and targeted checks resolve them.
Receipts are in `exact-address-store-after-slot-fix/`, the corresponding
source/literal-oracle and frozen/public/Boost folders, and
`student-export-exact-address-after-slot-fix-v3/`. The full paired all-50
scoreboard is complete in `group2-exact-store-full-scoreboard/`; an earlier
attempt with one-CPU affinity for threaded workloads is explicitly invalid
and retained separately.

The new complete comparison uses three isolated serial lanes and sixteen
physical cores for threaded matrix workloads, retaining every case and all
calibration warnings. Higher-is-better geometric means are:

| Reference | Timed loops | Process CPU | Native instructions |
| --- | ---: | ---: | ---: |
| Original campaign compiler | 1.056× | 1.057× | 1.072× |
| Corrected seventh compiler | 0.998× | 0.998× | 1.001× |
| GCC | 1.005× | 1.003× | 0.956× |
| Clang | 0.964× | 0.964× | 0.958× |

These two-block timings contain substantial A/A warnings. They also expose a
real individual regression: nine focused pairs reproduce a 12% Brainfuck
recursion loss, with unchanged instructions and substantially more branch
misses. The hot body is unchanged; removing other code moved its indirect
dispatch address. This remains an open acceptance issue for closing the group,
not something the aggregate cancels. The interpreter's apparent loss does not
reproduce in a longer control. See the refreshed diagnosis and retained
`exact-address-store-after-slot-fix-{brain,interpreter}-control/` observations.

## Ninth mechanism: revisit invariants introduced by packing

The Nbody diagnosis identifies a phase-ordering gap: packing creates invariant
floating broadcasts after ordinary loop motion has finished. Reuse the existing
motion pass once, only after packing changed a function that had loops. The
earlier loop-presence check supplies that guard; there is no new whole-pipeline
round or change to inlining, unrolling or allocation budgets.

Compiler hash:
`2bd2ecc6f4438b63df2e58f402a25af91dbfc2353ecc773560c284b268b13201`.
Index binary hash:
`60a1cdff823f1cf473ac37da6860456df2001fe75e11e8f50c50ae82f0363dca`.
The port emits the private prototype's exact object bytes. All 50 checksums
pass and direct/serialized objects agree. Five calibrated pairs give 1.024×
Nbody instruction speed and 1.012× process CPU speed; small timing differences
in unchanged Base64/Huffman work remain within calibration. Index text stays
at 386,626 bytes.

The new required PA32 source behavior fixture exercises zero-trip null
pointers and division, aliases, volatile factors, callbacks, cleanup on throw,
NaNs, infinities and signed zero. The shared scalar quality oracle checks
adjacent updates through four observer aliases, with broad LowIR/native
instruction bounds. Independent GCC/Clang and source/replay checks cover
16,384 varied semantic cases per path. Student guidance describes changed-loop
cleanup and safe motion without requiring our pass order or packed code.

All 13 gates pass, including the strict 8,058-test report, debug info,
self-host through PA5, PA35 smoke/quick, modern C++, audits and harness.
All 74 public and four Boost sentinel objects are identical to the eighth
compiler's outputs. Frozen compile instructions are unchanged within
calibration at both levels; the separate whole-index compile control finds
essentially unchanged instruction cost. The short frozen CPU measurements
have substantial A/A variation and support no speed claim. Cumulative compile
instruction cost remains about 0.43% above the original compiler.

A fresh export verifies 22,565 portable references and 9,042 exit statuses,
the installed tools, new source behavior fixture and both quality paths.
No old reference output changes. Receipts are in
`late-packed-invariants-ported/`, its oracle/frozen/public/Boost companion
folders, `late-packed-invariants-v2/index-compile-cost/` and
`student-export-late-packed-invariants/`. The separate Brainfuck layout loss
persists and still needs resolution before this group can close.

## Tenth mechanism: copy bounded dispatch tails

The Brainfuck regression is a branch-prediction effect. Removing overwritten
stores elsewhere moves the recursive visitor, while its hot LowIR and native
instruction sequence remain unchanged. Nine calibrated pairs reproduce a 12%
loss; last-branch records isolate almost all added misses to its shared
indirect dispatch. A link-only padding sweep does not restore the earlier
speed and supplies no shipping workaround.

Extend the existing short-tail copying routine to straight-line terminal
dispatch blocks reached by an unconditional jump. Keep guards, effects,
landing pads and exceptional boundaries in place, and charge table entries
against the existing growth bound. Each emitted copy receives its own table
label. This uses typed MIR and changes no inlining or allocation budget.
The loop-test and dispatch cases share their copying code. Geometric storage
growth avoids repeated exact-size reservations: an independent 32–1,024-switch
family preserves identical MIR/executable bytes and uses up to 47% fewer
compile instructions than the initial implementation.

Final compiler hash:
`c292d37ba433839bca449ea4cf0ff13aca76b8fed948e5bad7ea2d8bf329fd60`.
Index binary hash:
`3a143c3e96b160e6457ea85a3bf918b818dc5cd5aca15143c574d6bee568681d`.
Its object is identical to the measured private prototype, and direct and
serialized paths agree. All 50 checksums pass. The visitor shrinks from 480
to 420 bytes; complete index text falls from 386,626 to 386,514 bytes.

Five paired blocks give 1.428× CPU speed against the ninth compiler and
1.226× against the corrected seventh compiler, with unchanged instructions.
Median branch misses fall from about 61 million to 8.5 million. Five
independently linked offsets preserve a 20–26% gain against the seventh
compiler. Reference CPU speeds are 1.096× GCC and 1.003× Clang; Clang's
calibration warns. Those offsets are diagnostic, never compiler policy.

The full 50-workload comparison retains separate original, previous, GCC and
Clang scoreboards, including all observations and timing warnings. Nine
apparent losses receive longer controls: seven pairs for threaded Matmul T8
and nine for each other case. Their loop medians range from 0.992× to 1.039×
the ninth compiler, with unchanged instruction counts; the earlier large
losses do not reproduce. Words retains a small apparent loss within its
substantial calibration variation. Do not claim that every runtime is exact.
All 74 public and four Boost sentinel objects remain byte-identical.

Independent guarded and unguarded packet-dispatch oracles cover 32,768 cases
per path, including wide tags, zero-trip null/division inputs, callbacks,
volatile accesses and unwinding. The required PA33 fixtures add an independent
source oracle and raw signed/unsigned guards, phi inputs and broad quality
bounds. Student guidance describes legality, growth and measurement without
requiring duplication or any particular instruction sequence.

All 13 validation gates pass, including the strict 8,059-test report,
debug info, self-host through PA5, PA35 smoke/quick, modern C++, audits,
harness and GCC/Clang warnings-as-errors. PA35's complete release check
passes all 17 original gates and margins in both views; inception matches.
A fresh export passes its document/reference bundle checks and both new
fixtures through the installed tools. Frozen compile instructions grow
about 0.008% at `-O3` against the ninth compiler and 0.438% cumulatively;
`-O0` and peak RSS remain within calibration. CPU calibration is too noisy
to support a compiler speed claim.

Receipts: `dispatch-tail-final/` and its oracle/frozen/public/Boost companions,
`dispatch-tail-duplication-v4/`, `dispatch-tail-efficient-growth-v2/`,
`group3-dispatch-full-scoreboard/`, `group3-dispatch-native-controls/` and
`student-export-dispatch-tail-final/`. The interrupted pre-refinement
validation and initial invalid raw fixture are retained; final validation
uses the corrected fixture and the amortized-growth implementation.

## Eleventh mechanism: propagate uniform local contents

The log parser's matcher has already lost its unused capture parameters
through specialization. Its remaining initialization work comes from a local
table filled with `0xff` and subsequently only read as integers. Increasing
inlining or specialization budgets would not address this memory fact.

The new proof admits a whole constant-byte fill at entry, an unused return,
and ordinary integer observations through derived addresses. It rejects
other writes, address observations, merges, escapes, volatile/atomic reads,
unsupported types and entry exception regions. Replace admitted loads by
their repeated-byte values and remove the unobserved fill. A cheap entry
scan avoids full-function analysis when no fill qualifies. Eligible fills
share one replacement walk and reuse an instruction mask; the initial
prototype instead scanned its fill list at every instruction.

Compiler hash:
`e95310aed96a6206af4c38fc9fe892b8cf355e24159982129ee35c3edb3d6a2b`.
Index binary hash:
`9b814f2e33348c1ef0671fdf6ad005ce64ce3c3eff461b177a35ba794f63b8b0`.
The production compiler and native object are identical to the measured
prototype. All 50 checksums pass and direct/serialized objects agree.
Index text falls another 320 bytes to 386,194.

Five paired blocks give log parsing 1.189× instruction speed and 1.350×
process CPU speed against the tenth compiler. The GCC and Clang comparisons
remain separate in the receipts, with calibration warnings. Brainfuck's
seven-pair control preserves the dispatch gain with unchanged instructions.
An unrelated template-regex control initially shows about 3% lower CPU speed
in two trials. A third, separately calibrated cache-counter trial gives
0.998× CPU speed, unchanged instructions and no resolved instruction-cache
increase; its calibration varies more than the suspected effect. Retain that
uncertainty in the full-scoreboard controls instead of asserting exact times.

Independent GCC/Clang, baseline/candidate and replay paths pass 98,304 cases
each. Eight additional raw-IR paths pass 57,344 cases each, including pointer
phis, before-fill reads, partial writes, opaque mutations, volatile/atomic
accesses, floating values, wide loads and copied contents. A 64–1,024-fill
scaling family produces identical objects and correct executables; the largest
case uses about 32% fewer compile instructions than the initial prototype.

All 13 validation gates pass, including the strict 8,061-test report,
debug info, self-host PA5, PA35 smoke/quick, modern C++, audits, harness and
GCC/Clang warnings-as-errors. Inception matches. The new raw expectation
initially lacked its normally generated outcomes; regeneration and the
targeted fixture/audit rechecks pass, with the first receipt retained.
All 74 public and four Boost sentinel objects are byte-identical to the
tenth compiler. Frozen `-O3` compile instructions grow about 0.020% for this
change and 0.457% cumulatively; `-O0` and RSS remain within the investigation
bars. Linux measures peak RSS here, not physical footprint.

A fresh student export passes document, reference-bundle and installed-tool
checks, including both new required C++11 fixtures. Student guidance describes
content propagation and its proof without prescribing a pass or instruction
sequence. Receipts: `uniform-local-fill-ported/`, `uniform-local-fill-v4/`,
its oracle/edge/frozen companions, `uniform-fill-linear-scaling/`,
`uniform-fill-ported-*` and `student-export-uniform-fill-ported/`.

## Twelfth mechanism: expose contiguous case membership

Maze generation counts three neighboring cell kinds. The optimized LowIR
retains a switch and a conditional increment; both references expose an
interval predicate. Normalize a switch whose cases all share one destination
when their typed values form a contiguous unsigned interval. Preserve the
default and use wrapping subtraction where necessary. Existing predication
then decides whether the guarded work can move; its effect and speculation
checks remain unchanged. Singletons keep their existing equality path.

The final implementation shares select formation's walk and preserves the
unique successor set, so it reuses the CFG. The initial late prototype made
an extra scan and rebuild. Earlier cleanup prototypes fired before the full
case set formed and did not produce the desired simplification; they are not
accepted performance results. No new LowIR fact or grammar is needed.

Compiler hash:
`e40b7bbfae16dc612b34c05f3a019f1101307f27b1f3e007a41bd7ca3bc4e6d7`.
Index binary hash:
`a0249b0cb1b895e085df693a30222f0ff655988c53549d326dbbe5d5c2ba7f6b`.
The production compiler and measured private prototype emit identical native
objects. All 50 checksums pass and direct/serialized output agrees. Index
text falls another 224 bytes to 385,970, below the original 387,154 bytes.

Five paired blocks give maze generation 1.226× process CPU speed and 1.046×
instruction speed against the eleventh compiler. The same control gives BFS
1.021× CPU speed and AStar 1.009×, with much smaller instruction changes.
GCC and Clang remain separate references, with timing warnings retained.
Seven-pair Brainfuck, log-parser and template-regex controls preserve their
instruction counts and do not resolve a new runtime loss.

Independent source oracles pass 98,608 cases per path; raw-IR oracles pass
7,280,490. They cover widths, signed boundaries, duplicates, singletons,
full ranges, holes, cyclic sets, unsupported wide values, guarded overflow,
loads/division, throwing callbacks and cleanup. Uniform-fill oracles also
pass again on the combined compiler. Course fixtures check behavior and broad
quality bounds. An existing structural control required a 128-case switch;
it now also accepts equivalent unsigned range decisions, retaining its wrong-
bound, wrong-edge, signed, hole and effect negatives. Fourteen control tests
pass, and the earlier singleton regression reference stays byte-unchanged.

All 13 validation gates pass, including the strict 8,063-test report,
debug info, self-host PA5, PA35 smoke/quick, optional modern C++, audits,
harness and GCC/Clang warnings-as-errors. A temporary full disk interrupted
receipt writing after ten successful gates. Their original logs and recorded
statuses are retained; the final three checks were rerun successfully.
`validation-results-recovered.json` records this distinction explicitly.
Full PA35 release passes all 17 original gates and margins; inception matches.
The fresh student export verifies 22,583 portable reference outputs, audits
all 55 student documents and installs all 42 reference wrappers. Both new
fixtures pass through its installed reference tools. All four export checks
and all three closure steps pass.

Frozen `-O3` compile instructions grow about 0.005% for this change and
0.461% cumulatively. `-O0` cumulative growth is about 0.017%; its object
bytes are unchanged. Peak RSS remains within the investigation bars; physical
footprint is unavailable here. The walk reuse preserves identical native
output while avoiding the earlier prototype's roughly 0.026% compile increment.

Of 74 existing public objects, only JSON and RE2 at `-O3` change. Of four
Boost sentinels, only release Xpressive changes. All behavior checks pass.
Their paired native instruction speeds are effectively 1.000×; precise CPU
speeds are respectively 0.999×, 1.017× and 0.992×, within the observed timing
variation. Xpressive runs the fixed full regression input and requires all
429 tests on every invocation, with the same host-built test framework.
The first link attempt omitted that framework object; its failure is retained
separately and is not a compiler correctness result.

Receipts: `range-switch-final/`, `range-switch-v6-*`, `range-switch-v4/`,
`range-switch-final-public-objects/`, `range-switch-final-boost-sentinels/`,
`range-switch-final-changed-native-recovery/` and
`range-switch-final-xpressive-native/`.

## Fourth full-scoreboard checkpoint

The final native payload completes all 50 full-input comparisons against the
campaign baseline, previous accepted compiler, GCC and Clang: 200 pairs of
workload/reference results, with no omissions. All ratios are higher is better.

| Reference | Loop wall speed | Process CPU speed | Instruction speed |
| --- | ---: | ---: | ---: |
| Campaign baseline | 1.075× | 1.075× | 1.077× |
| Eleventh compiler | 1.020× | 1.018× | 1.001× |
| GCC | 1.007× | 1.009× | 0.961× |
| Clang | 0.991× | 0.990× | 0.963× |

These are equal-weight geometric means in this paired experiment. There are
81 calibration warnings across the 200 comparisons. They do not establish
small timing leads, nor should they multiply the initial uncalibrated 0.901×
GCC estimate. The stable campaign instruction gain is about 7.2% fewer
instructions. Original inputs and checksums remain unchanged; every measured
binary has recorded provenance.

Longer controls check the apparent individual losses against the eleventh
compiler. The large cache-simulation, words, template-regex, single-matrix,
arithmetic-decoder and array-Brainfuck losses do not reproduce. SortMerge and
GraphAStar retain smaller apparent losses, roughly 1.1% and 1.6% in process
CPU, within substantial A/A variation. Their instructions are unchanged;
keep the uncertainty rather than claiming every runtime is exactly preserved.
Threaded matrix timing also separates its nearly unchanged compute loop from
more variable process CPU. Every initial observation remains in the receipts.

The disk interruption truncated part of the first control reports. Complete
observations were checked against output/status, counters, order and binary
hashes before reuse; incomplete groups were rerun in fresh directories.
Receipts: `group4-range-full-scoreboard/`, `group4-range-native-controls/`
and `group4-range-native-controls-recovery/`.

## Remaining work and ranked follow-ups

Local validation is complete. Push the two validated local commits and review
index-cpp, public and Boost CI at that head.
The automatic index-cpp job is already validated locally and on GitHub; its
first main execution will publish results beside the public suite.

The new ranking and refreshed diagnoses leave these follow-ups. A diagnosis
with an open legality or cost proof is not an implemented optimization.

| Priority | Follow-up | Required evidence before a change |
| --- | --- | --- |
| 1 | NeuralNet and Base64 decode: leading GCC gaps, about 0.69–0.70× loop speed | Profile NeuralNet's newly leading region. For Base64, use the actual decoder worker and the retained pressure/shift probes; those probes found no resolved gain. |
| 2 | Calculator AST and interpreter ownership/dispatch | Preserve the AST allocation/cleanup constraints. The final private trace proves the interpreter's non-loop recursive-wrapper rejection before pricing; replace the rejected broad policy with a cheaper guarded-forwarder design. |
| 3 | Huffman and Base64 encode: leading Clang gaps | Isolate pointer/bit dependencies and encoding separately. The accepted indexed-load fix does not close the remaining Huffman gap; retain stock ISA and algorithms. |
| 4 | Template parsing, NGram and cache/container helpers | Distinguish EH eligibility from helper pricing using the final admission trace. Prove collision, alias, lifetime and exception behavior; caller budget increases do not address the observed rejections. |
| 5 | Nbody, SHA256 and maze residuals | Nbody needs an ordered producer/extraction packing and pressure proof. SHA256 needs an explanation of Clang's scalar-loop difference. Maze cursor promotion needs a memory/capture proof across growth calls. |
| 6 | CSV and JSON scanners | Separate generated character/copy work from strtod, allocation and other shared runtime costs; preserve conversion and failure semantics. |

`index-cpp-target-diagnoses.md` links the full IR/MIR, stock-reference
disassembly, profiles, rejected experiments and first-phase evidence. Future
groups must retain the frozen compiler, existing workload and individual
regression checks instead of treating this aggregate as a blanket exemption.
