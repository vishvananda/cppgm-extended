# LLVM gap strategy: 10 November 2026

The working target is cppgm-built LLVM compiling the 325 frozen cppgm units
in at most 1.05 times native-built LLVM's elapsed time, at every level.
The latest completed full O1 comparison is the r17 aggregate checkpoint:
1.06952 times native elapsed (95% interval 0.96356–1.17548) and 1.08778 times
native cycles (1.06635–1.10921). The target remains unmet. The r18 scratch-register
correctness repair is qualified separately; subsequent inlining results are
private partial-build screens. O2/O3 have not been remeasured after these changes.
Historical comparisons below retain their original baselines and scope.

## Method adopted from `~/gap.md`

First measure sibling calls, existing aggregate splitting and select formation
at O1, independently. Then prioritize aggregate representation, executed code
footprint, inlining, branch shape and address folding. Screen immutable private compilers by
rebuilding the profile-selected LLVM objects, verifying their outputs, then
running a deterministic ledger. The panel comprises number_literal, store_motion,
program_lowering and class_templates; its six metrics are executed distinct code
bytes, modelled L2 code misses, bytes per instruction, instructions, conditional
branches and call instructions. Add Instructions.cpp to the original 34-provider
slice: it contains CmpPredicate::getMatching; Instruction.cpp alone does not.

Retain the hardware event panel for store-forwarding blocks, resteers,
mispredictions, instruction TLB events, code-cache misses, micro-operations and
fetch stalls. Check both hardware siblings for load. Keep raw observations and
flag cycle outliers; instruction reductions alone are not acceptance evidence.
Fewer events per group avoid multiplexing on this host. Historical whole-workload
records omit several of these events, so they cannot fit all proposed mechanism
costs; assumed event costs are not calibrated or causal attribution.

Require a mechanism bound before changing policy. Measure compile instructions
on semantic_overload.cpp at O0, O1, O2 and O3. The original baseline is the
immutable 558f42c8 compiler adopted in 54bdf818b. The user subsequently clarified
that moving existing work between optimization levels establishes a new
baseline: remeasure after the three O1 moves, then charge future structural
work against that compiler. Retain the old comparison as the cost of changing
level contents, rather than counting it as new aggregate work.
A batch up to 0.5% uses the normal gate;
cumulative growth up to 2% requires the predicted panel movement. Above 2%,
cumulative growth must be covered point for point by proven runtime gain at
that same level: the lower 95% bound on the whole-workload cycle-ratio drop,
over at least five ABBA pairs. Above 10% also needs user sign-off. All outputs
must match the native oracle in every observation, with the predicted event
movement; a cycle-only change requires checks with three link orders. O0,
RSS and PA36 release limits remain unchanged. Report per-level baseline,
current cost, growth, proven gain and headroom together.

Before using more than 0.5%, require a cheap no-candidate exit, shared analyses,
no per-candidate body rescans, an on/off cost measurement and removal of
superseded special cases. Qualify commits with the usual tests and audits. Rebuild full
LLVM and make an elapsed comparison after a workstream, with at least five ABBA
pairs. The elapsed target stays 1.05, independently of the working counter ledger.

### Investigate unexpected results before abandoning a workstream

A failed screen rejects the measured prototype, not the underlying mechanism.
Related changes may need to work together before their benefit appears. When
the ledger moves against the prediction, preserve the immutable compiler and
raw results, then attribute the excess work and check for confounders: pass
ordering, lost facts, representation conversions, changed inline decisions,
register pressure, code layout and interactions with other optimizations.

Use small reproducers and controlled ablations to separate those explanations.
Make a few focused iterations, combining dependent fixes where the evidence
supports them, and repeat the same output-checked screen. Do not abandon an
approach solely because its first incomplete implementation regresses. Record
what each iteration disproves or repairs, what remains unimplemented, and why
the mechanism is retained or eventually deprioritized. Acceptance still needs
the normal correctness, compile-cost and whole-workload performance gates.

The two-word aggregate prototype illustrates this rule. It removed packing at
some outgoing boundaries but left pack/unpack chains after inlining and did
not preserve every scalar extension requirement. The first regression is a
reason to investigate those interactions; it does not disprove keeping small
aggregate values in registers. Root-cause controls and ablations are retained
in `gap-A-native-word-parts-root-cause-r1/` in the campaign evidence directory.

## Correctness prerequisite: constructor exports

The full LLVM rebuilt with 54bdf818b exposed 131 undefined weak constructor or
destructor entry names, although the focused matrix passed. The first executed
failure called APInt's complete-object move constructor through address zero.
A base-only COMDAT group could discard another group's additional alias exports.
Exporting reverse aliases fixed the all-cppgm build but failed with native Clang,
whose corresponding group can legitimately export only the base entry. That
private approach was rejected and is absent from the final change.

The object writer now gives a group with multiple exported aliases a signature
encoding its entire sorted export set. Equal bundles coalesce; a group exporting
fewer names cannot discard a required alias. Single-export groups retain their
ordinary keys. Synthetic ELF signatures are local, non-function symbols; Mach-O
keeps its existing behavior. No constructor-name parsing or extra course IR
exports are needed.

The PA33 hosted regression combines a base-only producer and a producer needing
both entries. It failed before at O1-O3 with the base-only object first. A separate
384-case matrix covers mixed optimization levels, GCC/Clang objects, BFD/LLD and
all six link orders. All pass after the final fix. Full LLVM rebuilds, and its
325 frozen object files and linked cppgm executable match native byte for byte.
There are no undefined weak constructor/destructor entries in that LLVM image.

Validation: strict report 10,587/10,587; debug checks; self-host through PA5;
inception; PA36 quick/smoke; harness; architecture, format, file and placement
audits. Frozen compile-instruction ratios versus 54bdf818b are 1.001161 at O0
and 1.000078 at O3; RSS stays within the gate. No runtime gain is claimed for
this correctness repair.

## Aggregate work in progress

Private prototypes widen overlapping integer slot accesses, enable O1 packing,
and replace eligible incoming eightbyte object homes with scalar carriers.
The original getMatching example loses its argument homes. Further prototypes
cover eightbyte call results and return construction. They pass standalone
controls, including escaping addresses, volatile storage, floating ABI classes,
stack arguments, mixed widths and inlined object returns. The wider LLVM slice
exposed additional integration defects. A private late word-return prototype
now builds full LLVM and reproduces all 325 native objects and the linked
executable. Its getMatching fast path keeps the 12-byte payload in registers,
but its fallback still saves registers on the stack. The 35-object screen
reduces blocked store forwarding by 2.3% while increasing instructions by 1%;
fetch stalls and code-cache misses are flat. The full replay also increases
instructions, so this prototype is not adopted. Its temporary return
representation is native-only and lacks serialized IR support. No aggregate
exit criterion from gap.md is yet met.

## O1 level contents

PA33 leaves level contents to the implementation; PA34 specifies behavior and
quality bounds. Both handouts have been read. No handout change is necessary.
Sibling calls retain the frame, stack-argument, exception-model and addressed
block guards. Ordinary aggregate splitting already runs at O1; the experiment
moves its remaining packing, scalar-address and indexed-region guards, plus
the addressed-block invocation. Select formation is tested independently of
the rest of the O2 late pipeline. Existing tests that forbid O1 tail jumps
need their level expectation changed while retaining the correctness guards.

Each feature has a separate immutable compiler and 35-object LLVM relink.
The separate O1 compile-instruction costs versus the preceding compiler are
0.013% for sibling calls, 1.112% for the aggregate guards and 1.957% for select
formation. Combined, the level moves cost 3.127% at O1. The O0, O2 and O3
pipelines are unchanged; their frozen objects match the preceding compiler.

The qualified baseline after moving the existing passes is below. Future
growth and proven gains are measured from
this baseline, per the user's clarification. Headroom is the unproven 2-point
allowance; O0 retains its existing, separate limit.

| Level | Baseline instructions | Current instructions | Growth | Proven gain | Headroom |
| --- | ---: | ---: | ---: | ---: | ---: |
| O0 | 3.276616 G | 3.276616 G | 0% | 0 points | Existing O0 gate |
| O1 | 7.674418 G | 7.674418 G | 0% | 0 points | 2 points |
| O2 | 9.070676 G | 9.070676 G | 0% | 0 points | 2 points |
| O3 | 9.421919 G | 9.421919 G | 0% | 0 points | 2 points |

On the number_literal relink screen, sibling calls reduce actual calls by
4.35%; select formation increases conditional moves/setcc by 30.7% and reduces
conditional branches by 1.46%. These are partial-rebuild mechanism results,
not runtime gains. The complete LLVM built with all three moves produces all
325 cppgm objects and the linked executable byte-identically to native LLVM.
The five-block whole-workload comparison reports the following ratios to native:

| Metric | Mean ratio | 95% interval |
| --- | ---: | ---: |
| Elapsed | 1.1299 | 1.0968-1.1631 |
| Cycles | 1.1417 | 1.1196-1.1637 |
| Instructions | 1.06591 | 1.06589-1.06593 |
| L1 code misses | 1.1654 | 1.1368-1.1939 |
| Fetch stalls | 1.3029 | 1.2735-1.3323 |

The run uses fresh native A/A calibration and five alternating ABBA blocks,
12 fixed physical cores, unchanged binaries and frozen inputs. All 325 object
hashes and the linked executable match the native oracle on all 22 observations.
Raw sibling load records show shared-host activity during the run; every
observation is retained and the interval reflects block variation. This is a
new baseline score, not proof of a same-level gain funding future compile cost.

Qualification passes: strict report 10,589/10,589 including the updated
survivor controls; debug checks; self-host through PA5; inception; PA36
quick/smoke; harness; architecture, format, file and global placement audits.
The complete harness passes on retry after one timeout-cleanup fixture failed
when the runner was restricted to one CPU. Its separate retry also passes;
no timeout harness code was changed. The placement audit finds no fixture moves.
On store_motion, the complete LLVM's count panel reports blocked store
forwarding down 7.65%, mispredictions down 4.49% and fetch stalls down 2.66%
against the previous build. All outputs match; cycle outliers remain recorded.

## Structural probes and casting-fact review

The completed aggregate boundary prototype does not improve the full workload.
Five symmetric three-way blocks compare native, the accepted 8f76ebb0d compiler
and the prototype concurrently. Every two-side subsequence is ABBA. All 325
objects and the executable match in all 32 observations, including native A/A.
The accepted compiler measures 1.1331 times native elapsed; the prototype
measures 1.1378. Prototype/accepted elapsed is 1.0042 (95% interval
0.9978-1.0106); cycles are 1.0072 (1.0009-1.0134). Although blocked store
forwarding falls about 15%, executed code remains essentially unchanged and
code-cache misses rise. The prototype stays private. Its compile-instruction
cost of about 1.2-1.4% at O1-O3 has not purchased a demonstrated runtime gain.

One earlier number_literal ledger used a partial relink as its baseline.
That overstated the aggregate and inlining probes' benefits. The authoritative
full accepted baseline is `gap-step0-full-baseline-ledger-number-r2`; retain
the older raw observations but do not use their comparisons. Corrected
four-input aggregate results are in
`gap-A-boundary-materialize-four-panel-summary-r3.json`, and corrected inliner
oracle results are in `gap-C-oracle-ledger-summary-r4.json`. Neither oracle
policy is a bound on a complete replacement inliner.

Compact scalar frame homes and promotion of selected frame values into unused
callee-saved registers also remain private. Their output comparisons pass,
but neither moves executed footprint usefully; the latter increases executed
instructions and code-cache misses. Static access counts do not establish
that paying a save/restore cost benefits the executed path.

The casting review confirms that the earlier pointer-copy, non-null and
alignment-return fixes are present. It also demonstrates a remaining fact
consumption gap at both O1 and O3: after `(x & 7) == 0` succeeds, clearing those
bits again still emits an instruction. Separate successful tests of disjoint
zero bits do not simplify a later test of their combined mask. These are
optimization gaps, not observed incorrect C++ results. The LowIR contains
the necessary tests. Value simplification retains equality and nonzero facts
but does not project masked comparisons back into operand bit facts.
Jump threading has additional bounded range proofs at O2+, so this is not a
claim that the compiler has no range analysis.

A private dominator-scoped bit-fact prototype removes the redundant masks.
It tracks SSA values, restores facts when leaving a dominance region, and
does not attach facts to mutable memory. Twenty-two control functions over
65,536 inputs agree between GCC, Clang, the accepted compiler and the prototype
at all four levels. The 35-provider LLVM relink produces all 325 objects and
the executable identically to native. Frozen compile instructions rise 0.231%
at O1, 0.219% at O2 and 0.219% at O3; O0 and RSS are unchanged within repeat
variation. The first deterministic number_literal screen is nevertheless
too small to justify qualification:

| Metric | Prototype / accepted |
| --- | ---: |
| Executed distinct code bytes | 1.000104 |
| Instructions | 0.999783 |
| Bytes per executed instruction | 1.000081 |
| Conditional branches | 0.999806 |
| Calls | 1.000928 |
| Modelled L2 code misses | 1.003733 |

Adding consumption of known-one bits produces identical objects in all 35
providers. These probes remain outside the compiler branch. No full LLVM
build or elapsed gain is claimed for them.

Source-line attribution needs two controls. Adding debug information preserves
all executable sections in each of the 35 rebuilt objects. Native's linked
code is identical too; cppgm's linked layout changes, so its debug image uses
a fresh profile. Ambiguous overlapping line ranges are excluded, with unique
address mappings checked against llvm-addr2line. Coverage is only about
43% of native and 47% of cppgm executed instructions. Moreover, the two
compilers assign inlined instructions to different source locations. Header
totals therefore locate candidates; their differences are not removable work.

Matching function counts provide useful counterexamples to that inference:
`castFromDeclContext` executes nine instructions per call versus native's ten;
`getNumParams` executes almost exactly the same instructions per call but is
called more often. Some other apparent function excesses are work native
outlines into a callee. Rank actual conditional jump instructions, excluding
Callgrind's branch counts for REP iterations, then include the related callees
before attributing a cost to a missing fact or selecting another fix. The
next provider slice must also cover the newly identified high-cost routines
outside the original 35 files.

## Evidence

All raw records are under `/mnt/vizier/cppgm-llvm-evidence`:

- `gap-ledger-baseline-import-r1/manifest.json`: copied and hashed original report evidence.
- `gap-comdat-export-bundles-main-r1-validation-status.json`: normal qualification.
- `gap-comdat-export-bundles-main-r1-frozen-perf/summary.json`: compile-cost gate.
- `gap-comdat-export-bundles-mixed-r2/summary.json`: mixed compiler matrix.
- `gap-comdat-export-bundles-full-output-r1/receipt.json`: complete output equality.
- `gap-comdat-export-bundles-diagnostic-r1-llvm-build-status.json`: full build identity.
- `gap-checkpoint-ledger-number-r1/`: repaired baseline deterministic ledger.
- `gap-A-register-aggregates-compiler-r6/` and `gap-A-register-aggregates-top35-r6/`:
  private aggregate probe and relink, including unsuccessful validation logs.
- `gap-step0-four-level-frozen-r1/`: independent feature costs against the old baseline.
- `gap-step0-new-baseline-four-level-frozen-r1/`: measured baseline after the moves.
- `gap-step0-reference-review-r1/`: old/new output review excluding unrelated examples.
- `gap-step0-fixture-before-r1/`: the two new quality regressions fail before the moves.
- `gap-step0-baseline-full-output-r1/`: full LLVM's 325-object and executable equality.
- `gap-step0-baseline-hw-panel-r1/`: full LLVM's hardware mechanism panel.
- `gap-step0-baseline-main-r2-strict-status.json`: strict report after control updates.
- `gap-step0-baseline-audit-r2-validation-status.json`: architecture, file and placement checks.
- `logs/gap-step0-baseline-main-r3-harness.log`: complete final harness pass.
- `gap-step0-baseline-native-five-pairs-r1/`: all 22 raw observations and intervals.
- `gap-A-boundary-materialize-full-threeway-r3/`: full aggregate prototype comparison.
- `gap-step0-full-baseline-ledger-number-r2/`: corrected full baseline ledger.
- `gap-source-line-attribution-r3/`: source-line localization with ambiguity exclusions.
- `gap-casting-facts-r1/`: reductions, corrected bit-fact screen and function mechanism rankings.
- `gap-D-mask-facts-controls-r2/`: 22-function independent compiler matrix.
- `gap-D-mask-facts-full-output-r2/`: complete output comparison after the private relink.
- `gap-D-mask-facts-four-level-frozen-r2/`: compile-cost measurements.
- `gap-D-mask-facts-hw-panel-r2/`: all hardware observations and retained outlier flags.
## Final callee pricing: initial screening

The next C experiment removes the fourfold loop-site allowance for callees
larger than the ordinary cap and judges a callee after its own inlining. It
removes the fallback that admitted an expanded callee using its smaller input
shape, together with that fallback's shape copies and counters. Correctness,
ABI, recursion, exception and explicit inline-attribute checks remain in place.
This is a partial implementation of C4, not the proposed complete cost model.

The private compiler is `gap-C-final-shape-compiler-r3/cppgm++` under the
evidence root. Its 40 rebuilt LLVM objects and linked diagnostic Clang are
byte-identical to the preceding r2 experiment, which retained the obsolete
bookkeeping. All 325 frozen workload objects and the linked executable match
native. The four-input ledger reports these changes against the accepted full
8f76ebb0d build, not the superseded partial baseline:

| Input | Executed code bytes | Instructions | Bytes/instruction | Conditional branches | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| number_literal | -0.467% | -0.028% | -0.172% | +0.096% | -0.519% | -2.044% |
| store_motion | -0.528% | +0.040% | -0.215% | -0.013% | +0.505% | -1.646% |
| program_lowering | -0.513% | +0.246% | -0.238% | +0.211% | +0.844% | -1.746% |
| class_templates | -0.532% | +0.221% | -0.260% | +0.144% | +0.987% | -1.623% |

The hardware screen on store_motion records L2 code misses -2.63%, front-end
resteers -4.83%, instruction-TLB second-level hits -3.28%, and walk duration
-2.54%. Fetch stalls rise 3.27% in their separate counter group, blocked store
forwarding rises 1.76%, and retired micro-operations rise 0.15%. Thus this is
evidence of a smaller executed footprint and fewer code misses, not a proven
elapsed-time improvement. Every observation and flagged outlier is retained.

Frozen compile instructions for r3 change by -0.001% at O0, -0.439% at O1,
-2.142% at O2 and -2.436% at O3; RSS remains within the gate. These were the private
candidate costs at the screening checkpoint. The compile-cost baseline remains
8f76ebb0d; the completed qualification below records the runtime gain.
The full O1 LLVM rebuild completes in 394.68 seconds under load, with no C++
fallback; this is an unpaired build observation, not a compile-speed claim.
Its Clang SHA-256 is
`4a5325778b87aa9e7f7f09187dae7b2bb21f595863ab38e2872d736d9bc15e84`.
The output-verified five-block comparison and normal qualification are
reported below.

One existing maintainer control explicitly required the old large-body loop
multiplier: `check_lowir_o3_loop_priority_inline.pl` passes with the baseline
and rejects the candidate because loop_first retains two calls instead of one.
The qualification replaces that exact admission requirement with behavior
and code-growth bounds while preserving its noreturn/phi safety coverage.

Evidence: `gap-C-final-shape-four-panel-summary-r2.json`,
`gap-C-final-shape-hw-panel-r2/`,
`gap-C-final-shape-four-level-frozen-r3/`,
`gap-C-final-shape-full-output-r2/`, and
`gap-C-final-shape-top40-r{2,3}/`.

### Tiny helpers and a layout-sensitive control

A separate private experiment estimates scalar leaf operations during the
existing callee-shape walk and compares them with argument setup plus the call
and return. Predicted non-growing leaves bypass growth budgets. All 325 outputs
match; the four-level compile cost is approximately +0.05--0.07% at O1--O3.
It has not reduced executed footprint: number_literal changes by +0.007%.
The combined final-shape/leaf experiment also preserves every output, but its
footprint reduction (-0.443%) does not exceed final-shape pricing alone. Neither
the leaf rule nor the combined compiler is selected for the full rebuild.

The leaf-only screen also exposes a limit of treating deterministic instruction
counts as direct code-generation attribution. DeclContext::lookupImpl has the
same instructions and control-flow graph in both builds after resolving address
differences, and both profiles enter it 77,398 times. The candidate executes
185,732 additional hash-table probes. Exactly twelve instructions and two
conditional branches per probe account for all of this routine's increase:
2,228,784 instructions and 371,464 branches. DenseMap hashes pointer-valued keys;
this is consistent with layout-sensitive key placement, not additional
instructions emitted into the routine. Do not subtract these observations from
the performance result or classify them as a missed optimization. Distinguish
path-frequency changes from code changes when ranking residual costs.

Evidence: `gap-C-leaf-cost-analysis-r3/lookupImpl.diff` (empty normalized
assembly diff), `lookupImpl-path-counts.json`, `gap-C-leaf-cost-ledger-number-r3/`,
`gap-C-leaf-cost-hw-panel-r3/`, and the corresponding `gap-C-final-leaf-*` files.

The sparse-switch tree and dense-subrange experiments also preserve all 325
outputs and have effectively flat compile cost. On number_literal, the latter
reduces executed footprint only 0.174% and raises modeled L2 code misses 0.941%.
It remains private. The tree-only screen used 39 providers; the dense-subrange
screen includes the actual X86EncodingOptimization.cpp provider as its 40th,
so their differences must not be attributed solely to dense subranges.

The duplicated-epilogue bound has also been rerun against the correct full
baseline (`gap-B-epilogue-bound-r2.json`). Even assuming short jumps to shared
tails, it can remove at most 39,907 executed bytes (0.424%). This still does not
support epilogue sharing as the main footprint remedy.

A separate B2 screen lowers the register allocator's price for opening another
callee-saved register from two weighted accesses to one. All 40 provider objects
change and all 325 workload outputs still match. Push/pop executions rise about
4%, while stack loads/stores fall 2.6--3.0%, as predicted. However, executed
footprint falls only 0.138%, instructions rise 0.610%, and modeled L2 code misses
rise 1.920%; hardware code misses rise 0.740%. Frozen compile instructions fall
about 0.22% at O1--O3. The register/memory tradeoff moves, but the intended code
cache benefit does not. Keep this prototype private rather than adopting the
lower opening price. Evidence: `gap-B-callee-save-price-*`.


## Final callee pricing: qualified checkpoint

The compiler now prices a full callee after its own inlining and grants the
fourfold loop allowance only when that final hot body fits the ordinary cap.
The obsolete input-shape snapshots and admission counters are removed. This
is still only part of C4. Partial-copy policy and the remaining cost caps have
not been replaced.

The complete O1 LLVM build passes all 325 object comparisons and linked
executable comparisons in all 32 observations: native A/A followed by five
symmetric three-way blocks. Candidate/accepted cycles are 0.97472 (95% interval
0.96258–0.98685). The cycle ratio against native falls from 1.13996 to 1.11111,
a gain of 2.885 points, with a conservative proven gain of 1.489 points. The
O1 target is not met.

Elapsed candidate/accepted is 1.01675 (95% interval 0.88322–1.15027), so no
elapsed improvement is established. One candidate observation takes 74.83 s
while ordinary candidate observations take about 49–51 s; its compiler CPU
time and cycles do not increase proportionately. Some sibling activity also
rises during the run. All observations remain in the statistics; the elapsed
outlier's exact cause is not established. There is no cgroup CPU throttling
recorded in the inspected counters.

Whole-workload instruction-cache misses fall 4.50% and fetch stalls fall 4.93%,
while instructions rise 0.557%. The full build's deterministic ledger confirms
smaller executed code on all four inputs. Values below are changes from the
full accepted 8f76ebb0d build; modeled cache counts cover the main executable,
matching the instruction-mix scope rather than whole-process totals.

| Input | Executed code bytes | Instructions | Bytes/instruction | Conditional branches | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| number_literal | -2.432% | +0.999% | -0.476% | +0.868% | +0.750% | -1.105% |
| store_motion | -2.650% | +0.848% | -0.695% | +0.113% | +2.618% | -2.635% |
| program_lowering | -2.969% | +0.761% | -0.560% | +0.367% | +2.108% | -3.467% |
| class_templates | -2.966% | +0.948% | -0.716% | +0.313% | +2.865% | -3.641% |

The full-build hardware panel on store_motion records L2 code misses -4.97%,
resteers -7.41%, instruction-TLB second-level hits -6.69%, TLB walk duration
-8.67%, mispredictions -0.63% and fetch stalls -3.34%. Blocked store forwarding
rises 6.62% and micro-operations rise 0.97%. All 80 object outputs match, and
17 cycle outliers are flagged and retained. This improves the code-footprint
mechanism while leaving the aggregate problem open.

The compile-cost baseline remains the measured post-move 8f76ebb0d baseline.
A compiler checkpoint does not reset the cumulative budget. The table uses
the paired repeats of that same immutable compiler from this cost gate.
Proven gain is the lower 95% bound and is credited only at O1; O2/O3 runtime
has not been remeasured for this checkpoint. Headroom is to the unproven +2%
line, not permission to bypass a normal per-batch gate. O0 retains its separate
unchanged limits. All RSS ratios are within 0.12% of baseline.

| Level | Baseline instructions | Current instructions | Cumulative growth | Proven gain (points) | Headroom to +2% |
| --- | ---: | ---: | ---: | ---: | ---: |
| O0 | 3.276466 G | 3.276430 G | -0.001% | 0 | separate O0 limit |
| O1 | 7.674457 G | 7.640779 G | -0.439% | 1.489 | 2.439 points |
| O2 | 9.070773 G | 8.876450 G | -2.142% | 0 | 4.142 points |
| O3 | 9.421966 G | 9.192478 G | -2.436% | 0 | 4.436 points |

Qualification passes: PA33, strict report 10,592/10,592, debug information,
self-host through PA5, inception with matching objects and compiler, PA36
smoke/quick, harness, architecture/format/file audits and global placement.
The PA33 cluster-500 regressions fail the old compiler's size envelope and
pass at O1–O3; behavior agrees with GCC and Clang. The expanded helper makes
260 loop-body instructions before and 22 after. The old exact loop-inlining
control now checks behavior, bounded growth and noreturn/phi safety.

Evidence: `gap-C-final-shape-qualification-current-r1.json`,
`gap-C-final-shape-full-threeway-r3/`,
`gap-C-final-shape-full-four-panel-summary-r1.json`,
`gap-C-final-shape-full-hw-panel-r1/`, and
`gap-C-final-shape-regression-r4/quality-regression-proof.json`.

## Further casting-fact checks

Controls using LLVM 22.1.2's actual Casting.h pass under GCC, native Clang,
the accepted compiler and this checkpoint at O0–O3, including nonzero base
offsets, null inputs, repeated type checks and a call that changes the type
tag. The last case must reload the tag: a const-qualified accessor is not a
promise that intervening calls leave the object's storage unchanged.

One missing lowering fact is confirmed. On the successful non-null path of
a statically analyzed base/derived pointer conversion, the adjusted pointer
is also non-null. Lowering emits an unqualified LowIR index, losing that
source-level guarantee. The existing generic raw-address index must not be
assumed non-null. A private prototype records the guarantee with the existing
nonnull pointer-copy fact at the conversion site. The checked_cast example
falls from nine to six machine instructions at O1/O3, removing the adjustment
and redundant null test; nullable conversions still preserve null. The O0
control object's text size is unchanged. Speculation already drops nonnull
facts when moving the copy onto paths where the original instruction did not
execute.

All 325 workload objects and the executable still match after a 40-provider
relink (26 provider objects change), and frozen compile cost is essentially
flat at all four levels. Nevertheless, the first number_literal screen moves
executed bytes only -0.008%, instructions +0.228%, branches +0.225%, calls
+0.466% and modeled L2 misses +0.172%. This remains private. Function totals
also change through inlining and pointer-key lookup path frequencies; these
are not evidence that the three removed instructions became more expensive.
The follow-up must account for fact-only copies in the inliner's cost model
rather than treating new semantic knowledge as executable work.

A second example eliminates the repeated type test but still branches to
constant boolean returns at both O1 and O3, where native emits setcc. A
conservative scan of the accepted number_literal profile finds only 75
recognizable epilogue-free sites, 95,957 executed branches and 9,082 modeled
mispredictions. That bound is too small for a separate optimization milestone;
it does not cover arbitrary return bodies.

Evidence: `gap-casting-header-controls-r1/`,
`gap-E-cast-result-fact-{compiler,top40,full-output,ledger-number,four-level-frozen}-r1/`,
and `gap-D-terminal-select-bound-r1/`.

A separate C4 screen prices a call's setup and control work in the existing
callee-shape walk. It reduces frozen compile instructions by 4.42%, 5.68% and
6.00% at O1–O3. Its first 40-provider profile reduces executed bytes 0.717%
and modeled L2 misses 3.331%, while calls rise 2.536%. The completed four-input
panel has calls up 2.46–2.69% and instructions up 0.33–0.55%, despite modeled L2
misses falling 2.71–3.43%. Its full five-block comparison has cycles 1.31%
worse than its before binary (95% interval -0.064% to +2.683%), with no proven
gain; all outputs match. This experiment is not part of the qualified compiler.
Its elapsed estimate is especially noisy and is not an acceptance score.
Evidence: `gap-C-call-price-*`.

The prototype adds `3 + argument_count` for each call *inside the callee* to
its admission price, then applies the existing caps. It never credits the
caller for the call sequence being removed. This is an incomplete cost
comparison and plausibly explains the extra retained calls; that attribution
still needs an admission trace joined to the dynamic profile. Do not rerun
this unchanged penalty as the proposed C4 cost model, or silently raise its
caps. Revisit it after the aggregate checkpoint with explicit admission
reasons and the caller/callee cost difference, preserving resource limits.

## Two-word prototype: investigate the unexpected regression

The accepted work was rebased onto main `535ed4f12` as `a30986549`.
The new immutable compiler builds LLVM at O1 in a diagnostic 304 seconds;
all 325 frozen cppgm objects and the linked executable match native LLVM.
That is a correctness replay, not an elapsed comparison. The investigation
below deliberately keeps the preserved pre-rebase compiler and LLVM image
fixed, so changes from main do not confound the experiment.

The first native word-pair prototype increased number_literal instructions
by 18.62%, distinct executed bytes by 13.46%, and modeled L2 code misses by
34.96%. Of the 202.4 million extra instructions, 144.6 million were stack
loads and stores. The hypothesis that incoming parameter homes alone caused
the regression was incomplete: already-inlined values repeatedly packed two
words into I128 and immediately extracted them again.

Three interacting omissions are now demonstrated:

- The existing packed-part simplifier excludes I128. The new aggregate
  representation therefore bypassed that simplification. An inline
  identity/accessor chain grew from two machine instructions to 76;
  `FoldingSetNodeID::operator==` grew from 18 to 65, with its frame increasing
  from 8 to 168 bytes. Typed packed-part simplification removes those chains.
- The same conversions inflated inline costs. Instrumented admission for
  a hot `DenseMap::find_as` helper changed from hot cost 73 to 184, exceeding
  the hinted cap of 96. Typed cleanup brings it to 75 and restores admission.
  Increasing the inline threshold would conceal the representation problem.
- The private argument path omitted the existing scalar path's deferred
  address handling for a spilled base. It could emit `lea 48` instead of
  `base + 48`. A reduced mixed-toolchain test fails in the original prototype
  and passes after sharing that behavior. Eight GCC/Clang cases cover O0–O3.

A native-only extraction-alias experiment also exposed stale scaled-address
facts: turning a word into a constant after computing its scale produced a
three-byte end offset instead of twelve in `std::max({a,b,c})`. Reordering
the analysis fixed that reproducer. The retained experiment instead folds
the typed IR before native analysis, which also exposes the simpler bodies
to the inliner. These failed private variants remain recorded.

| Change from accepted pre-rebase compiler | Instructions | Executed bytes | Modeled L2 code misses |
| --- | ---: | ---: | ---: |
| Native word-pair handling alone | -0.14% | +0.01% | -0.18% |
| Aggregate coercion without early wide expansion | +6.21% | +6.05% | +14.42% |
| Original combined prototype | +18.62% | +13.46% | +34.96% |
| Typed cleanup and spilled-address repair combined | +1.28% | +1.19% | +1.80% |

The combined repairs remove about 93% of the extra instructions. All 325
objects and the executable match native; the 32 mixed ABI controls, 52
optimized-IR replay/Casting.h controls, and eight spilled-base controls pass.
This is still a partial-rebuild screen, and it still regresses: no aggregate
milestone or runtime acceptance is claimed. Continue the related work on
incoming two-word values and partially occupied carriers, then port the
combined changes onto the rebased source while preserving main's aggregate
improvements. Do not discard the workstream on the first prototype's result.

The completed store_motion hardware panel confirms a residual regression:
instructions are about 1.98% higher, group median cycles 2.18–3.03% higher,
blocked store forwarding 4.76% higher and L2 code misses 5.65% higher. All 80
outputs match; seven cycle outliers are retained. These counters support
continuing the missing boundary work, not accepting this intermediate result.

Evidence: `gap-A-native-word-parts-root-cause-r1/`, including the instruction
attribution, three inliner traces, reduced failing programs and ablations;
`gap-A-root-cause-opt-fold-late-{top40,full-output,ledger-number}-r1/`;
`compiler-gap-rebased-535ed-r1/` and `gap-rebased-535ed-full-output-r1/`.

### Continue the combined changes on the rebased source

The private combined prototype now preserves main's partial aggregate splitting
and other changes. Incoming I128 parameter and call-result words are defined
at their ABI boundary, with the same definitions used by liveness, register
allocation and emission. The first mixed-toolchain control exposed simultaneous
definitions competing for a register: the first word could take the second
word's planned global register. Reserving both destinations repairs that conflict.
The completed incoming variant passes 32 mixed ABI cases, 52 IR replay/casting
checks and 16 reduced controls. Its 40-provider LLVM relink reproduces all 325
objects and the executable byte for byte.

These screens use the new full LLVM baseline built from `a30986549`; they must
not be compared directly with the preceding pre-rebase percentages.

| Rebased private variant | Instructions | Executed bytes | Modeled L2 code misses |
| --- | ---: | ---: | ---: |
| Ported combined repairs | +1.793% | +1.192% | +4.093% |
| Incoming scalar words | +0.689% | +0.183% | +1.303% |
| Also support 9–15-byte values | +0.596% | +0.216% | +0.124% |
| Also model short zero writes as scalar accesses | +0.669% | +0.182% | +0.235% |

Partially occupied carriers read exactly the source object's bytes and pad only
private destination homes. Sixteen mixed GCC/Clang guard-page cases cover all
sizes from nine through fifteen bytes at O0–O3. The other 84 ABI/replay controls
and all 325 workload outputs also pass. These are correctness and mechanism
screens, not full LLVM qualification or runtime acceptance.

The incoming variant's hardware panel still shows blocked store forwarding
up 5.08%, L2 code misses up 2.22% and instructions up about 0.65%. Cycle group
medians range from -0.17% to +3.10%; all 80 outputs match and all 20 flagged
outliers are retained. The predicted store-forwarding reduction is absent.

Investigating that discrepancy found another missing interaction in the actual
`CmpPredicate::getMatching` body: the empty optional initializes one byte with
`zeroinit`. Treating it only as a bulk span prevents the entire return slot from
being split. A follow-up models short zero writes as scalar accesses and rewrites
them only when their slot splits. Combined with the preceding changes, it removes
all return-object memory operations from the actual LLVM function; its assembly
has no stack memory operands or stack adjustment. Its normal register saves
remain. The 84 ABI/replay checks, sixteen guard-page cases and sixteen additional
optional-value cases pass, as do all 325 workload objects and the executable.
The hardware panel now reduces blocked store forwarding by 0.92% relative to
the rebased accepted compiler. That is better than the preceding prototype's
5.08% increase, but substantially below the intended reduction. Instructions
remain about 0.7% higher; code misses rise 1.41%. Cycle group medians range from
-0.59% to +1.48%, with eleven outliers retained and all eighty outputs equal.
This is not a demonstrated runtime improvement.

The paired frozen compile-cost check against the rebased compiler measures
+0.485%, -0.175%, +0.830% and +0.804% instructions at O0–O3, with RSS growth at most
0.11%. O0 output is unchanged. The separate cumulative O1–O3 comparison remains
below the campaign's post-level-move baseline. The O2/O3 batch costs exceed the
ordinary 0.5% gate, so reduce unnecessary analysis and representation work
before qualification; do not hide that cost behind cumulative headroom.

The user requested continued investigation of unintended regressions. The
latest instruction attribution leaves 7.18 million extra instructions, led by
2.36 million register moves, 0.92 million stack loads and 0.60 million stack
stores. Calls rise by 266 thousand. Several helpers become out of line, so
trace their admission and conversion costs on identical input before changing
thresholds. Preserve the successful incoming-word and optional-storage fixes
while testing these interactions; no aggregate workstream is accepted yet.

Evidence: `gap-A-{combined,incoming,partial}-rebased-*`,
`gap-A-incoming-rebased-hw-panel-r2/`, `gap-A-partial-code-shape-r3/`,
`gap-A-scalar-zero-code-shape-r4/`, `gap-A-scalar-zero-rebased-*`,
`gap-A-scalar-zero-guard-controls-r4/` and `gap-A-optional-zero-controls-r4/`.

### Repair inline return merges and separate layout effects

The next trace identifies another representation regression. Inlining a
multiple-return I128 helper recreates a memory merge after the early wide-value
expansion has finished. The resulting stack store/load and pack/unpack chain
raises `SmallDenseMap::insert` from the accepted compiler's cost of 88 to 99,
above the unchanged hinted limit of 96. Merging the two words with I64 phis
instead removes the new merge slot, reduces the cost to 92 and restores the
inline decision. This does not raise any inline limit or run another whole-body
wide expansion for every call.

The combined r5 private screen reduces the preceding instruction regression
from 0.669% to 0.008%. Executed bytes fall 0.258% and bytes per instruction fall
0.580% against the rebased accepted compiler. Calls remain 0.856% higher and
modeled L2 code misses 0.167% higher. Dynamic stack loads and stores fall by
2.35 million and 3.14 million, respectively. These counts alone are not runtime
acceptance. The hardware panel has 80 matching outputs and fourteen retained
cycle outliers: instructions fall about 0.25–0.31%, blocked store forwarding
falls 1.32%, but resteers rise 4.01%, L2 code misses 0.77% and fetch stalls 1.89%.
Cycle group medians range from -1.08% to +0.75%.

The 32 mixed ABI cases, 52 optimized-IR replay/casting checks, sixteen spilled
pointer/roundtrip controls, sixteen guard-page cases, sixteen optional-value
cases and twelve new wide-arithmetic/nested-return cases pass. All 325 workload
objects and the linked executable match native. The initial new arithmetic
test used the freestanding link path with `printf`; that harness invocation
failed to link. Its successful replacement compiles objects and links through
the host toolchain, as the other hosted controls do.

The r6 follow-up removes five unnecessary uses of the new native operand
visitor: four analyses run before word facts exist, and dead pure operations
cannot be the call/store/return consumers it changes. All forty LLVM objects
and the linked slice are byte-identical to r5. The paired frozen compile-cost
changes are +0.442%, -0.282%, +0.485% and +0.472% at O0–O3; all meet the ordinary
0.5% batch limit, with RSS within 0.04%. O0 output is unchanged. The historical
cumulative budget still uses the post-level-move baseline.

The remaining attribution also exposes a measurement confounder. In
`DeclContext::lookupImpl`, the lookup count and hit/miss outcomes are identical,
but collision-loop iterations increase from 175,007 to 381,089. Twelve
instructions per additional probe account for 2.473 million instructions,
almost the entire measured 2.454 million increase in that function. These
pointer-key hashes depend on allocation addresses; deterministic instruction
counts are not automatically independent of binary layout or argument layout.

A diagnostic relink places an otherwise unused writable byte at the same final
load address in both binaries. It preserves their code sequences and workload
outputs but changes allocation layout: the lookup difference reverses to
-722,095 instructions, and whole-binary instruction growth changes from +0.008%
to -0.268%. This supports a layout explanation; it is not an acceptance score
or a proposal to pad the production executable. The first anchor attempt placed
the section before ordinary sections and did not fix the image end; it is
retained as a separate failed experimental control. Use instruction attribution
to distinguish changed work per probe from changed probe counts, and retain
layout sensitivity alongside the usual paired hardware and elapsed results.

Another traced difference remains: the diagnostic-attribute helper grows from
380 to 665 instructions after its own inline decisions and crosses the existing
single-use cap. Its extra caller/callee work is much smaller than the misleading
lookup total. Investigate that interaction without raising the cap blindly.

The full O1 LLVM build with the combined r6 compiler completes in 303.17 seconds
and reproduces all 325 objects and the executable. Its deterministic screen
reduces executed bytes by 1.830% and modeled L2 code misses by 3.303%. However,
the full-build hardware panel increases blocked store forwarding by 8.46%,
where the partial rebuild reduced it slightly. The aggregate workstream remains
private and unaccepted; the complete panel, paired workload timing and normal
repository qualification are pending.

Evidence: `gap-A-inline-merge-rebased-*`, `gap-A-inline-merge-controls-r5/`,
`gap-A-inline-merge-reductions-r5/`, `gap-A-inline-wide-return-controls-r5a/`,
`gap-A-inline-{guard,optional}-controls-r5/`, `gap-A-inline-merge-admission-r5/`,
`gap-A-operand-scan-rebased-*`, `gap-A-lookup-shape-r6/`,
`gap-A-diagnose-shape-r6/`, `gap-A-heap-anchor-rebased-r{6,7}/` and
`gap-A-inline-merge-O1-r6-llvm-build-status.json`.

### Complete the small-size and partial-copy cases

The scalar ABI prototype originally excluded values smaller than eight bytes.
Extending the same exact-width loading to sizes one through seven covers
`SourceLocation` and other small values without reading past their storage.
The r7 candidate passes the 84 ABI/replay controls, sixteen guard-page cases
covering every size from one through fifteen, and all 325 workload outputs.
Its full LLVM build also passes those 325 object comparisons and the executable
comparison. Paired frozen compile costs are +0.441%, -0.416%, +0.363% and +0.350%
at O0–O3.

The full r7 hardware panel finally moves the intended mechanism substantially:
blocked store forwarding falls 29.51% against the rebased accepted compiler.
Instructions fall about 0.4–0.5%. Resteers and instruction-TLB hits still rise
6.14% and 5.13%, and fetch stalls rise 3.08%; cycle group medians range from
-1.68% to +0.84%. All eighty outputs match and fourteen outliers are retained.
This is mechanism evidence, not a demonstrated whole-workload elapsed win.

Sampling the r6 store-forwarding regression identifies `children<BasicBlock*>`
as roughly half its additional samples (536 of 1,068). Sampling is non-precise
and may skid; the assembly independently confirms the mechanism. This helper
copies a twelve-byte iterator payload between sixteen-byte homes, then reads
eight bytes after storing four. Its temporary memory transfers also
prevent the baseline's shrink-wrapped frame. The linker map identifies the
selected weak definition in `LoopInfo.cpp.o`, outside the forty-file screen.
The expanded quick set adds LoopInfo.cpp, Local.cpp and BasicBlock.cpp, which
provide several of the newly identified symbols.

The r9 experiment breaks short partial copies that straddle existing scalar
carriers into exact-width loads and stores, then uses the ordinary packed-field
logic. It reads all pieces before writing, preserves untouched bytes and leaves
volatile copies alone. Candidate discovery shares the existing address and
whole-word facts; a changed function is reanalyzed once, not once per copy.
The reduced padded-iterator test loses every temporary slot, and the actual
LLVM `children()` body has no stack memory operands after rebuilding its
provider. Sixteen mixed-toolchain padded-iterator cases and the 84 ABI/replay
controls pass. Compile costs remain within the ordinary gate: +0.444%, -0.449%,
+0.366% and +0.358% at O0–O3. Wider workload screens are in progress.

The separate r8 inliner experiment addresses the diagnostic helper's growth:
smaller child helpers now qualify for inlining, so their parent grows beyond
the single-use cap. It lets a small caller forwarding its sole internal callee's
result absorb that definition within the existing once budgets. Explicit limits
remain effective, and a caller with values live after the call does not qualify.
Eight mixed runtime controls check both cases. The forty-provider screen removes
28,810 calls relative to r7 but slightly increases instructions and modeled code
misses. Preserve this as a separate experiment; r9 starts from r7 and does not
include the unproven inliner policy.

Evidence: `gap-A-small-carriers-rebased-*`, `gap-A-small-carriers-full-*`,
`gap-A-small-carrier-{controls,guard-controls}-r7/`,
`gap-A-full-store-forward-samples-r6/symbol-summary.json`,
`gap-A-full-provider-map-r7/`, `gap-A-children-store-forward-shape-r7/`,
`gap-A-partial-copy-*`, `gap-A-partial-carrier-byte-controls-r9/`,
`gap-aggregate-units43-r9.json`, and `gap-A-forward-once-*`.


### Full aggregate experiment and qualification repairs (2026-11-11)

The full r9 LLVM build reproduces all 325 native objects and the linked
executable. Its hardware panel, paired against the compiler rebased on 535ed4f12,
reduces blocked store forwarding by 60.63%, to 1.323 times native. Instructions
fall 1.17–1.21%, modeled code misses on number_literal fall 5.17%, and executed
code bytes fall 4.60%. Hardware cycle group medians improve 1.36–3.55%, but still
stand at 1.080–1.105 times native. All eighty outputs agree; seventeen outliers
are retained. The separate five-block whole-workload timing is complete:
all 325 objects and the linked executable match in all 32 observations. The r9
cycle ratio is 1.1105 (95% interval 1.0673–1.1536), against 1.1152 for accepted;
the gain's lower confidence bound is negative. Elapsed is 1.0942 (interval
0.9795–1.2088), with large load effects retained. The mechanism improves but this
run proves neither a whole-workload gain nor the 1.05 elapsed target. The later
correctness repairs also prevent accepting r9 itself.

All four deterministic panels finished. Ratios to native are:

| Input | Executed bytes | Instructions | Bytes/instruction | Conditional branches | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| number_literal | 1.311 | 1.043 | 1.093 | 1.124 | 1.160 | 1.157 |
| store_motion | 1.313 | 1.055 | 1.086 | 1.107 | 1.146 | 1.156 |
| program_lowering | 1.307 | 1.063 | 1.083 | 1.121 | 1.145 | 1.169 |
| class_templates | 1.305 | 1.072 | 1.084 | 1.132 | 1.141 | 1.164 |

The executed-byte breakdown attributes 28% of the residual byte excess to
long-displacement stack accesses, 17% to long conditional branches, 17% to
address calculations and 15% to other memory stores. These categories are
footprint attribution, not independent cycle savings. Large X86 opcode switches
lead the common-function byte differences; rank and bound this family before
changing switch lowering. Existing frame-window and epilogue bounds remain
applicable and must not be rediscovered as new fixes.

The r10 follow-up recognizes an eight-byte partial copy starting at offset four,
using the already collected copy addresses. The 43-provider relink changes only
Attributes.cpp against r9; the number_literal instruction difference is six
instructions. Treat its modeled-cache change as layout-sensitive, not a proven
runtime gain. Module cleanup in the new aggregate-checkpoint worktree preserves
all 43 objects and the linked slice byte for byte against r10. It moves aggregate
ABI normalization to aggregate_boundaries.cpp and wide extraction simplification
to integer_patterns.cpp, retaining the original algorithms and source-list audits.

Qualification uncovered additional defects and incomplete interactions:

- Public LowIR replay rejected i128 integer_integer class metadata. This metadata
  preserves an aggregate's eight-byte stack alignment, whereas a bare i128 uses
  sixteen. The reader now accepts exactly that scalar class combination. PA8
  fixtures cover accepted boundaries and rejected SSE/memory scalar classes;
  PA33 host tests put a sixteen-byte aggregate after an odd stack word in both
  call directions with GCC and Clang.
- Packing same-width signed and unsigned fields emitted invalid extensions or
  truncations. Those operations are bit copies. A separate object snapshot must
  get explicit storage before its bits are read; it is not an address to its
  original source, which may already have changed. PA33 regressions cover both.
- Packing floating fields can preserve their bits with typed copies through
  integer carriers. The new fixture checks both halves, signed zero and a NaN
  payload, with explicit successful execution of original and optimized LowIR.
  Larger mixed-copy examples still retain memory and need further investigation;
  do not claim that the whole floating-field interaction is solved.
- Self-hosting exposed stale upper bits of a materialized Boolean adjacent to an
  enum in a returned LanguageMode. The scalar word consumer had become invisible
  to byte-use analysis when its i128 pack died. The resulting setne al left the
  high bits of a negative comparison result in rax. Byte-use analysis now follows
  split words. The reduced PA34 fixture fails with r14 and passes with r15 across
  negative inputs and all 256 adjacent byte values. Self-host through PA5 passes
  with r15; it failed in PA1 with r14.
- Two older control scripts demanded memory slots for safe partial accesses and
  object-valued argument copies. Replace those representation requirements with
  execution checks, including every valid byte offset and a previously uncalled
  object-copy control. Volatile effects remain checked.

The r15 compile-cost gate versus rebased accepted is +0.436%, -0.411%, +0.384%
and +0.367% at O0–O3, with RSS unchanged within noise. Cumulative budget still
uses 8f76ebb0d, not a per-prototype reset; that measurement is in progress.
The r15 strict report passes all 10,614 fixtures and its control checks.
Self-host through PA5, PA36 smoke, architecture audits and harness checks pass.
Debug-info checks passed on r14 and will be repeated on the final candidate.
The r15 full LLVM build remains in progress. Its 43-provider slice reproduces
all 325 objects and the linked executable. Nothing in this batch is accepted yet.

Continue investigating unintended regressions before judging this approach.
The mixed-copy reduction revealed a lost address fact: wide-word expansion
forms an index directly from a slot, whereas aggregate analysis only recognizes
indices from temporary pointers. It therefore treats the known private slot as
escaping and retains memory. The next private iteration teaches the existing
address lookup about direct slot bases; it does not add another analysis pass.
Run the fast slice and focused controls before another full qualification.
The r15 full build gets an output check and hardware panel, but repeat the
expensive four-input ledger only after the next iteration has been screened.

Evidence: gap-A-partial-copy-full-{output,hw-panel,workload-abba}-r9;
gap-A-partial-copy-full-ledger-*-r9 (panel-six-metric-summary.json and
execbytes.py.r9.txt/calls2.py.r9.txt/fncompare.py.r9.txt);
gap-A-offset-copy-*; compiler-gap-A-checkpoint-r{11,14,15};
gap-A-checkpoint-{focused-r13,onoff-frozen-r14,self-pa5-r15};
gap-A-selfhost-hex-failure-r14; gap-A-boolean-regression-r15.log;
and gap-A-checkpoint-fullbase-top43-r15. The qualification checkout is
/mnt/vizier/cppgm-llvm-aggregate-checkpoint; the accepted source remains in
/mnt/vizier/cppgm-llvm-baseline-call-frames at a309865490cf8584a6fb02fb1724e1ca6523bec9.


### Direct-slot facts and low-only incoming words (r16–r17)

The mixed-floating-copy regression was a lost address fact, not a need for
more packing rules. Aggregate analysis now recognizes a slot itself as an
address with zero extent. The existing field analysis and promotion then remove
all stack traffic in the mixed-copy reduction. The complex loop again evaluates
its imaginary result only on the NaN path. PA33's new direct-slot fixture checks
both branch outcomes and preserves a separate escaping-address control.
The complete PA33 course suite and its six quality kernels pass; the two
regression references were regenerated after reviewing the removed memory work.

All 43 r16 provider objects, and their linked LLVM, equal the r15 slice byte for
byte. Its completed 325-object replay also agrees. Reuse r15's deterministic
ledger for that identical binary; a redundant r16 Callgrind replay was stopped
and its partial artifacts retained with an equivalence receipt. This repair
restores a real fixture regression but is not a measured LLVM speedup.

A second incomplete interaction appears when only the low half of an incoming
or returned i128 survives. The word-parts pass's candidate scan recognized
i128-producing operations but missed a lone i128-to-i64 extraction. Enabling
the existing analysis for that extraction removes the otherwise unused return
frame (16 bytes to zero in the reduced call). The PA34 word-ABI fixture now
checks this case and its runtime result. The r17 slice changes 38 of 43 objects
against full r9 (r16 changed 30); all 84 mixed/replay controls and the complete
PA33 regression suite pass. Its full build and final qualification are running.

The full r15 LLVM build and all 325 outputs match native. Its hardware panel
measures 1.087–1.109 times native cycles and 1.324 times native blocked store
forwarding. This particular panel used the earlier post-move comparison binary,
explicitly identified in its protocol; do not label its before/after ratios as
comparisons against rebased accepted. r17's queue specifies that baseline.
Cumulative r15 compile cost against 8f76ebb0d is +0.442%, -0.772%, -1.562% and
-1.854% for O0–O3, with RSS within 0.3%. No compile-cost budget reset or runtime
acceptance is implied.

Evidence: compiler-gap-A-{direct-slot-r16,low-word-r17};
gap-A-{direct-slot-top43-r16,low-word-top43-r17};
gap-A-low-word-controls-r16 (before/after MIR reduction);
gap-A-low-word-controls-r17; gap-A-checkpoint-full-output-r15;
gap-A-checkpoint-full-hw-r15 (including baseline-note.json);
gap-A-checkpoint-cumulative-frozen-r15; gap-A-low-word-full-qualification-r17.log.

The conservative switch-chain bound finds 7.626 million instructions and 50,467
executed bytes in runs of at least eight adjacent immediate compare/branch
pairs: only 0.72% of instructions and 0.58% of executed bytes. These are upper
bounds on this family's removable work; native dispatch still has a cost.
The largest individual X86 differences are real but do not by themselves
justify the broad footprint milestone. Preserve the bound and defer that
change until a wider mechanism is established. Evidence:
gap-switch-linear-chain-bound-r1{,-summary}.json.


Final r17 normal qualification: self-host through PA5, debug-info, PA36 smoke,
architecture audits, file audit and placement all pass. The initial complete
strict report was 10,614/10,615: the new native quality sidecar counted the call's
`[args=...]` metadata as a memory operand. Its intended requirement is no frame
access and no allocated frame. After correcting that authored expectation, the
complete PA34 strict report passes 409/409. Together these cover every fixture
in the full report with unchanged compiler binaries. The regression proof
explicitly fails r15's three stores/one load and sixteen-byte call frame, then
passes r17. The formatting check uses CI's pinned clang-format 21.1.8; the host
22.1.2 reports one unrelated existing formatting difference.

Paired compile-cost growth against rebased accepted is +0.438%, -0.396%,
+0.395% and +0.376% at O0–O3; cumulative growth against 8f76ebb0d is +0.437%,
-0.760%, -1.554% and -1.844%. RSS is within the normal gate. The full r17 LLVM
build completes in 562.03 seconds under load (diagnostic only), and its Clang
reproduces all 325 objects and the linked executable. The full hardware panel
compared with rebased accepted reduces blocked loads by 60.86%, to 1.331 times
native. Cycle medians are 1.088–1.111 times native and 0.970–0.994 times accepted;
all eighty outputs match, with 24 outliers retained. This remains short of the
target and is not a whole-workload confidence claim.

The full number_literal ledger now reduces executed bytes 4.63%, instructions
1.61% and modeled L2 misses 5.26% relative to accepted. The other three ledgers
and subsequent five-block whole-workload comparison remain in progress.
The fetched origin/main is still 535ed4f12, already an ancestor of this branch.
There is no additional rebase to perform at this checkpoint.

Evidence: gap-A-followup-regression-proof-r17/proof.json;
gap-A-low-word-{strict-r17,strict-pa34-r17b,self-pa5-r17,debuginfo-r17,smoke-r17}.log;
gap-A-low-word-{format-r17,file-audit-r17,placement-r17}.log;
gap-A-low-word-{onoff,cumulative}-frozen-r17;
gap-A-low-word-O1-r17-llvm-build-status.json;
gap-A-low-word-full-{output,hardware}-r17;
gap-A-low-word-full-ledger-number_literal-r17.

### Completed r17 comparison and the next inlining interaction

The five symmetric blocks are complete: 32 observations including native A/A,
with all 325 objects and the linked compiler identical in every observation.
No inputs or binaries changed. Compared with rebased accepted, r17 averages
0.98005 times the cycles (95% interval 0.96048–0.99961). The cycle ratio to native
falls from 1.10995 to 1.08778. This is a 2.217-point mean improvement, but its
lower 95% bound is only 0.047 point; do not spend the mean as proven compile-cost
headroom. The existing ordinary and cumulative compile-cost gates already pass.

Elapsed time averages 1.06952 times native (95% interval 0.96356–1.17548), and
0.99494 times accepted (0.93887–1.05102). This does not meet the 1.05 target or
establish an elapsed improvement. Whole-workload instruction-cache misses fall
2.05% and fetch-stall counts fall 2.27% against accepted, consistent with the
footprint mechanism. Retain every observation, including the noisy elapsed runs.

All four deterministic panels are now complete. Ratios below are r17/native:

| Input | Executed bytes | Instructions | Bytes/instruction | Conditional branches | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| number_literal | 1.311 | 1.040 | 1.094 | 1.121 | 1.160 | 1.156 |
| store_motion | 1.312 | 1.055 | 1.085 | 1.108 | 1.146 | 1.150 |
| program_lowering | 1.306 | 1.063 | 1.083 | 1.120 | 1.145 | 1.168 |
| class_templates | 1.305 | 1.071 | 1.083 | 1.132 | 1.141 | 1.161 |

Continue investigating interactions rather than abandoning aggregate promotion.
A decoded call-edge comparison finds tiny helpers returning out of line after
the aggregate change. For example, QualType::operator-> entries rise from 1,278
to 54,986 on number_literal, and several flag accessors reappear. Their callers
reach ordinary/late budgets of zero to two. This is an unintended interaction
to investigate; moving a callee's self instructions into its caller does not
make that work disappear. Total call instructions rise 0.52% against accepted
despite the other aggregate gains.

The diagnostic admission trace changes no decisions: all 43 provider objects
and the linked slice match r17 byte for byte. Native inline remarks for the same
43 providers also preserve every allocated section and normalized relocation;
only debug information and corresponding symbol indexes change. Joining the
remarks yields 264,432 unambiguous matching caller/callee pairs, including
9,935 possible new admissions and 6,746 possible new declines. An experimental
compiler follows those matched choices while retaining safety checks and
required source inlining. Unmatched choices retain r17's policy. This partial
oracle is a diagnostic, not a production policy or a complete upper bound.

The oracle fails correctness before its performance screen: Clang crashes in
X86FixupBWInstImpl::getSuperRegDestIfDead while compiling constant_expressions.
Object bisection isolates X86InstrInfo.cpp; caller bisection isolates
X86InstrInfo::copyPhysReg; edge bisection isolates inlining isHReg(Register).
The emitted LLVM machine IR changes valid XMM-register copies into MOV8rr with
XMM operands. The later crash is a consequence of this earlier wrong opcode.
An empty diagnostic filter reproduces the original object exactly, and the
complete filter reproduces the oracle object exactly. Keeping BuildMI out of
line alone passes and is not the cause. Small synthetic register-class tests,
including two wrapper types and externally defined class tables, currently
pass at O0–O3; they do not yet reproduce the failure. Reduce the actual failing
path and repair the compiler before resuming the oracle ledger. Do not interpret
this failed diagnostic as a performance result, or undo the aggregate milestone
to hide the newly exposed defect.

The production checkpoint remains cf3e0fca5. A fresh fetch after the completed
comparison still finds origin/main at 535ed4f12, already an ancestor; no rebase
is needed.

Evidence: gap-A-low-word-full-workload-r17/summary.json;
gap-A-low-word-full-four-panel-r17.json; gap-C-admission-trace-equivalence-r17;
gap-C-call-edges-r17; gap-C-native-all-inline-remarks-r17/receipt.json;
gap-C-native-dynamic-join-r17; gap-C-native-oracle-current-r17.json;
gap-C-native-oracle-output-r17; gap-C-native-oracle-bisect-r17;
gap-C-oracle-{caller,edge}-bisect-r17; gap-C-oracle-mir-failure-r17;
gap-C-native-oracle-buildmi-top1-r17b; gap-C-reghigh-reduction-r17{,b,c,d}.


### Scratch-register correctness repair exposed by the oracle

Replaying the actual optimized host LowIR through the O0 backend passes and
produces the native object. The O1 selector instead widens a spilled byte into
R11, swaps it into the first operand of a tested AND, then loads the other
operand into R11 as well. The resulting test is mask against itself. In LLVM,
this incorrectly classifies an XMM register as an eight-bit register and emits
MOV8rr. Inlining isHReg exposes the defect by giving a shared shift another use,
so it no longer folds into a single-use bit test.

The selector now preserves the first operand's scratch register when loading
the second. PA34's behavior regression keeps enough values live to spill the
promoted byte and the reused mask. It checks clear, set and out-of-byte bits.
The old compiler fails O1–O3; the fix passes O0–O3. Another 128 private controls
cover widths, signedness, operand order, inverted predicates and all levels.
The exact single-edge LLVM reproducer now emits the native object.

Validation passes: strict report 10,616/10,616, debug information, self-host
through PA5 in a new object root, PA36 smoke, architecture/format/file audits
and global placement. The fixture needs no move. Against r17, frozen compile
instructions change -0.0044%, +0.0019%, +0.0059% and +0.0057% at O0–O3;
all four output objects remain identical and RSS stays within 0.06%.
Cumulative growth against 8f76ebb0d is +0.4454%, -0.7614%, -1.5526% and
-1.8466%. This correctness fix does not reset the baseline or claim runtime gain.

With the repair, the partial native-decision oracle reproduces all 325 frozen
objects and the linked compiler byte for byte. Its number_literal screen has
calls down 5.50%, instructions down 0.99% and modeled L2 code misses down 4.07%
against r17, while executed bytes rise 0.090%. The other inputs and hardware
panel are still running. The oracle is private diagnostic data, not a proposed
name-based compiler policy. A separate controlled probe changes only ordinary
inlining's growth charge from the greater of original/final size to final size;
its purpose is to test whether stale charges exhaust budgets after successful
simplification. Existing admission caps, safety checks and unit limits remain.

Evidence: gap-C-host-ir-r17/receipt.json;
gap-C-host-ir-replay-r17/output-equality.json;
gap-C-scratch-{regression-proof,controls}-r18; gap-C-scratch-validation-r18.json;
gap-C-scratch-{batch,cumulative}-r18/summary.json;
gap-C-scratch-edge-top1-r18/test.json;
gap-C-scratch-oracle-{top43,output}-r18;
gap-C-scratch-oracle-ledger-number_literal-r18;
gap-C-final-charge-compiler-r18; gap-C-original-price-trace-compiler-r18.


### Completed native-decision screen and cost-accounting controls

The scratch repair is committed as 30e2ab26e. The native-decision diagnostic's
four-input panel is complete. Changes below are against the full r17 compiler;
the candidate itself is a 43-provider relink, so these are screening results.

| Input | Executed bytes | Instructions | Bytes/instruction | Conditional branches | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| number_literal | +0.090% | -0.992% | +0.210% | -0.097% | -5.499% | -4.074% |
| store_motion | +0.076% | -1.148% | +0.245% | -0.124% | -6.042% | -1.765% |
| program_lowering | +0.052% | -1.107% | +0.226% | -0.074% | -5.920% | -2.210% |
| class_templates | +0.045% | -1.124% | +0.216% | -0.224% | -5.817% | -1.886% |

The hardware screen retains all eighty matching outputs and fourteen flagged
cycle outliers. Group medians are 0.974–0.990 times r17 cycles and 1.070–1.084
times native cycles. Blocked store forwarding falls 7.53%, hardware L2 code
misses 1.71%, instruction-TLB second-level hits 2.95%, TLB walk duration 4.05%,
micro-operations 1.53% and fetch stalls 4.42%. Mispredictions are essentially
flat (+0.041%). This is enough evidence to pursue general inlining cost
accounting; it is not a whole-workload confidence or elapsed acceptance claim.

A decision-preserving trace confirms 1,647 ordinary-wave decisions where the
final body fits the caller's remaining budget but the original body does not.
Across admitted trace events, original-size charges sum to 2,492,486 versus
1,351,104 final-size units. These are repeated static decisions across sites,
providers and waves, not dynamic instructions saved. All traced objects match
the uninstrumented r18 compiler (42 also match r17; the X86InstrInfo difference
is reproduced by the scratch fix alone).

The isolated final-size charging probe changes 29 providers and preserves all
325 outputs. It barely changes executed footprint (+0.012% and +0.018% on
number_literal/store_motion) or calls (-0.232% and -0.068%). Modeled L2 misses
fall 1.294% and 0.331%. Number_literal instructions rise 0.220%, but attribution
finds 2,342,490 extra instructions in DeclContext::lookupImpl alone. Its
normalized assembly is identical and both runs enter it 77,398 times; the
extra work is repeated pointer-key hash probes. Retain the raw total and this
layout-sensitive confounder rather than treating it as worse instruction
selection or subtracting it from an acceptance score. Store_motion instructions
fall 0.050%. Stale charges are real, but removing them alone is insufficient.

A further private C4 probe prices executable operations and nested calls during
the existing shape walk, subtracts the replaced call's argument/control cost,
and compares net growth with one ordinary/late threshold (40, or 60 with an
inline hint). It removes the body-shape caps and loop multiplier in those waves,
preserves safety, required substitution, definition-removing inlining and raw
unit growth limits, and keeps the once wave restricted to its original role.
Constant-argument specialization is still missing; this is an initial model,
not production code. The first draft accidentally also used the comparison
in the once wave; it was corrected before any LLVM screen (r19b is the tested
variant). Frozen compile costs and output-checked screens are running.

The updated frame-window encoding bound is only 96,889 bytes, or 1.04% of the
executed store_motion footprint, in functions with no executed RBP use. Its
445,398-byte all-function bound ignores new spills and other costs. This does
not establish a useful standalone frame-base optimization after aggregation.

Evidence: gap-C-scratch-oracle-four-panel-r18.json;
gap-C-scratch-oracle-hardware-r18/summary.json; gap-C-original-prices-r18.json;
gap-C-original-price-trace-top43-r18; gap-C-final-charge-panel-r18.json;
gap-C-final-charge-attribution-r18/lookup-path-counts.json;
gap-C-net-cost-{compiler,top43}-r19b; gap-B-frame-window-bound-r18.json.

### Inlining cost-model iterations and the next fact-consumption check

The broad r19b comparison preserves all 325 workload objects and the linked
compiler. On number_literal/store_motion it removes 7.96%/8.66% of calls, but
increases executed bytes 0.260%/0.259%; modeled L2 code misses change
-0.802%/+0.211%. Its frozen compile instruction increases against the qualified
scratch repair are 14.22%, 17.14% and 17.49% at O1–O3. Cumulative increases
against 8f76ebb0d are 13.35%, 15.32% and 15.33%. This is unsuitable for
production. The score-only control costs 0.16–0.18% with identical outputs;
the decision changes copy 22–25% more LowIR and leave larger bodies for later
passes. Computing the score is not the main expense.

Subsequent private experiments separate conservative admission from cloning
overhead. All percentages below are frozen compile instructions against r18,
before the rebase described below. Every observation and output hash remains
in the evidence directory; these experiments do not reset the cumulative gate.

| Probe | O1 | O2 | O3 |
| --- | ---: | ---: | ---: |
| r20b: only estimated non-growing call-free copies bypass budgets | +2.395% | +3.592% | +3.595% |
| r21: conservative scalar/control/address costs; unknown operations unpriced | +0.200% | +0.199% | +0.196% |
| r22b: existing straight-copy path for ordinary inlining, original decisions | -0.639% | -0.577% | -0.553% |
| r22: r21 plus straight-copy cloning | -0.452% | -0.390% | -0.363% |
| r23: r22 plus final-size growth charging | +4.515% | +6.068% | +6.257% |

r20b, r21 and r22 each preserve all 325 objects and the linked compiler in
their LLVM slice checks. r22b is a compile-cost control; r23 was not advanced
to an LLVM screen after its compile-cost result. No inliner policy from this
table is committed. The scalar estimate deliberately excludes bulk, wide,
vector and unknown operations instead of treating each as one instruction;
it also prices addresses, control flow and multiple-return merges. A trace
that preserves the frozen output shows r20b copying 135,999 instructions and
leaving 108,302, versus r21 copying 132,829 and leaving 106,158. The original
counts are 132,669 and 106,157. Many changed admissions are vector size/empty
accessors, string construction and multi-return min/max helpers, so the cost
reduction cannot be attributed to bulk operations alone.

r21's number_literal/store_motion screen changes calls -0.675%/-0.198%,
executed bytes -0.00005%/+0.0186%, instructions +0.422%/-0.0738%, and modeled
L2 code misses -0.696%/-0.992%. r22 changes calls -0.703%/-0.257%, bytes
-0.00475%/+0.0107%, instructions +0.464%/-0.110%, and modeled L2 misses
-0.188%/-0.756%. These small effects are not whole-workload runtime wins.

The r21 number_literal instruction increase includes 4,403,304 extra
instructions in the IdentifierInfo/MacroState DenseMap lookup alone. Its
normalized assembly is identical and both versions enter it 34,646 times;
the difference is 259,017 additional collision-probe iterations. Another
462,559 instructions come from DeclContext::lookupImpl, also with identical
assembly and 77,398 entries. Preserve these raw totals as layout-sensitive
confounders, not evidence of worse instruction selection and not quantities
to subtract from acceptance results. QualType::operator-> self instructions
fall from 219,944 to 1,288; other flag accessors still stay out of line.

Ordinary inlining currently splits even a single-block scalar callee into
entry/continuation blocks and subsequently merges them. Required substitution
already has a direct-copy path. r22/r22b reuse that path for ordinary copies
while keeping recursive expansions on their existing path. This is a compiler
efficiency opportunity to qualify with the eventual policy change.

The next related experiment must preserve and consume existing typed range
facts. The actual optimized Decl::isInvalidDecl body loads a field, shifts and
masks it with one, retypes it, and truncates it to u8. The backend can see that
the truncation needs no instruction, but the inliner still charges it. Test
recording a proved exact truncation in LowIR and consuming that fact in the
cost model. Prove representability at both source and destination signedness;
do not treat arbitrary narrowing casts as free or perform a fresh whole-body
analysis at each call. Add positive and signed/out-of-range negative controls
if this proceeds to production. Constant-argument specialization and the
complete replacement cost policy remain unfinished.

At the user's request, the clean worktree was rebased onto origin/main
04d285786. All three local commits replayed unchanged: the scratch repair is
now d799d5b13 (formerly 30e2ab26e), and the branch tip is 4fb2946bb. Main adds
pointer-bound facts, phi equivalence, conditional-store work and packed-field
load fixes. The pre-rebase branch is retained as
backup/llvm-aggregate-pre-main-04d285786; the private binaries above are
immutable. Rebuild and check the combined tree before the next experiment;
remeasure its local comparison without resetting the cumulative 8f76ebb0d
baseline. The full O1 runtime result at the top of this document is still the
last qualified full-build comparison.

Evidence: gap-C-net-cost-{batch,cumulative}-r19b/summary.json;
gap-C-cost-only-frozen-r19/summary.json; gap-C-optimizer-stats-r19/receipt.json;
gap-C-non-growing-{batch,panel}-r20b; gap-C-priced-leaf-{batch,panel}-r21;
gap-C-flat-{leaf-batch,only-batch}-r22{,b}; gap-C-flat-leaf-panel-r22.json;
gap-C-combined-cost-batch-r23/summary.json; gap-C-leaf-price-trace-r21;
gap-C-priced-leaf-attribution-r21; gap-C-priced-helpers-r21/helpers.lowir.

Rebase validation is complete: strict 10,627/10,627, debug information,
self-host through PA5 in a new object root, PA36 smoke, architecture/format/
file audits and placement all pass. Placement scans 9,958 tests and requires
no fixture moves. The immutable rebased compiler is
compiler-gap-C-main-04d-r24/cppgm++, SHA ac9793804d2df9807b06fe2137f42f73040c3696565333c721b544cd6b405b8f.
Against r18, its O0–O3 frozen instructions change +0.0008%, +0.0702%,
+0.0975% and +0.1638%. Its 43-provider LLVM slice reproduces all 325 native
objects and the linked compiler. A full O1 LLVM rebuild is running to measure
the combined main changes beyond that slice.

The private r25 experiment implements the range-fact check with the existing
possible-bits query, and lets the scalar cost estimate consume exact
truncations. The source and destination signedness checks reject a signed
negative input and the signed destination's high-bit boundary. Forty-four
typed IR checks and twelve GCC/Clang/candidate runtime controls pass. An initial
runtime harness invocation used the course linker for printf; the corrected
control compiles objects and links all three with the same host linker, and
retains the earlier harness failure. Frozen compile instructions against the
rebased compiler change +0.0002%, -0.3625%, -0.2979% and -0.2674% at O0–O3.
Its LLVM output check and ledger are running. This experiment is still private;
these checks do not establish a runtime benefit or production qualification.

Evidence: gap-C-main-04d-validation-r24.json;
gap-C-main-04d-{batch,output}-r24; gap-C-main-04d-O1-r24-llvm-build-status.json;
gap-C-exact-leaf-compiler-r25; gap-C-exact-leaf-controls-r25{,b};
gap-C-exact-leaf-batch-r25/summary.json.

### Full rebase checks and the missing packed-value prices

The full r24 O1 LLVM rebuild succeeds. It reproduces all 325 frozen native
objects and the linked compiler. All four deterministic ledger inputs pass.
Against r17, executed bytes change by less than 0.023% and instructions by
-0.105% to +0.287%; modeled L2 code misses fall 0.75–1.49%. The hardware
panel retains all 80 observations, including five flagged cycle outliers.
Its cycle medians vary from -0.19% to +0.80% against r17 across event groups;
this is not a resolved runtime improvement. All outputs match. The rebase
does not reset the cumulative compile-cost baseline: r24 remains +0.443%,
-0.691%, -1.456% and -1.685% at O0–O3 against 8f76ebb0d.

r25 also reproduces every workload output. Against the matching r24
43-provider slice, its number_literal/store_motion calls change
-0.948%/-0.348%, executed bytes -0.011%/+0.017%, instructions
-0.148%/-0.046%, and modeled L2 misses -1.307%/-0.166%. Range-fact
consumption helps some accessors, but this is insufficient as a standalone
runtime result.

The next private comparison, r26, uses one net hot-cost test for ordinary
and late admission (12, or 18 with an inline hint), prices calls inside
forwarding wrappers, and retains the safety and definition-removing policies.
It removes the former ordinary/late shape caps and loop multiplier. Frozen
compile instructions fall 15.49%, 12.40% and 12.69% at O1–O3, but the runtime
ledger exposes the cost: relative to r17, number_literal/store_motion execute
2.14%/2.40% fewer distinct code bytes and make 42.21%/54.78% more calls.
Instructions rise 8.87%/11.44% and modeled L2 misses rise 0.87%/1.53%.
All 325 outputs match. Faster compilation is not an acceptable substitute
for the codegen objective.

r27 adds a compact summary of single-block pure bodies and the parameters
they use. When all used arguments are literals, normal inlining and folding
can remove their work. This fixes the existing affine-i128/ordered-floating
constant-call quality check, and all 325 workload outputs still match, but
the ledger barely changes. This is a profitability prediction, not permission
to discard effects or assume that every arithmetic operation folds. General
multi-block constant-argument specialization remains unfinished.

A diagnostic shape trace reproduces all 43 uninstrumented r27 objects.
It identifies a confounder in the cost model: aggregation has converted
small class values into packed i128 values, while the model rejects every
body containing an unpriced i128 operation or object copy. Examples include
a reverse-iterator copy whose final body is a 12-byte copy and a return
(788,796 extra calls), small DenseMap range accessors, APInt move helpers,
and an EVT legality check. The latter has a small scalar body but an
i128-to-i64 extraction is enough to reject it. These are diagnostic symbol
attributions, not names used by any optimization policy.

The largest extra callee, SmallVector<unsigned>::push_back, accounts for
4,158,338 extra calls on store_motion. Its optimized cost is 27 at one
observed shape, with a short path around a growth call; it has no unpriced
operations. Thus packed-value pricing cannot explain the entire regression.
Retain these two causes separately and test both before abandoning the
replacement policy.

r28 prices two-word integer operations and fixed copies of at most 64 bytes
without changing the 12/18 thresholds. Its estimates count scalar word
operations, not a promise about final register allocation. r29 changes only
the thresholds to 24/36 on top of r28. These are private controls. Neither
policy is adopted; evaluate the output-checked ledger and hardware screen
before considering production changes. The r28 O1 tests retain five
reference/quality differences: two added exact-conversion facts, and three
inlining expectations. The affine constant-call failure is gone. Do not
blindly update policy-pinning references or raise caps to satisfy them.

Evidence: gap-C-main-04d-full-checks-r24.json;
gap-C-main-04d-full-four-panel-r24.json;
gap-C-main-04d-full-hardware-r24/summary.json;
gap-C-exact-leaf-versus-main-r25.json;
gap-C-unified-cost-{batch,panel}-r26;
gap-C-constant-cost-{batch,panel}-r27;
gap-C-unified-extra-call-shapes-r27.json;
gap-C-unified-shapes-r27/receipt.json;
gap-C-packed-o1-tests-r28/all-compare-summary.json.

r28's completed ledger confirms that packed-value prices explain a substantial
part, but not all, of the regression. Against r17, number_literal/store_motion
calls now rise 25.48%/32.43% rather than 42.21%/54.78%, instructions rise
6.00%/7.40%, executed bytes fall 2.65%/2.86%, and modeled L2 misses fall
3.56%/2.42%. All 325 objects and the linked compiler match. Frozen compile
instructions against r24 fall 15.03%, 11.71% and 12.01% at O1–O3.

r29 also reproduces all 325 objects and the compiler, with frozen compile
instructions down 6.91%, 3.08% and 2.77% at O1–O3 against r24. Its completed
number_literal ledger against the matching r24 slice shows calls +8.49%,
instructions +2.35% and executed bytes -1.89%. Its store_motion hardware
panel completes all 80 observations with identical outputs and eleven
flagged cycle outliers retained. Instructions rise about 2.85%, store-forward
blocks 3.24% and micro-operations 3.35%; L2 code misses fall 3.66%, TLB
second-level hits 2.38%, TLB walk cycles 8.04% and fetch stalls 2.37%.
Cycle changes range from -0.13% to +0.82% across event groups. Footprint
improves, but the remaining extra work consumes that benefit. Do not claim a
whole-workload or elapsed-time win from this screen.

The next private control, r30, prices a switch's comparison/branch pairs
instead of rejecting every switch-containing body. Residual callees include
DeclContext::isFunctionOrMethod and ValueLatticeElement::destroy. Keep r29's
24/36 thresholds so this tests missing cost coverage separately from a
threshold increase. Another follow-up to inspect is branch-prediction facts:
SmallVector's reserve path uses LLVM_LIKELY, but the callee hot-cost summary
only discounts noreturn paths. The existing caller-side rare-region walk
does consume these predictions, at O2 and above. Any extension should reuse
typed branch facts and the existing CFG, preserve the cost of copied cold
code, and measure both code footprint and calls. It is not implemented yet.

Evidence: gap-C-packed-cost-{batch,panel,output}-r28;
gap-C-mid-cost-{batch,output,hardware}-r29;
gap-C-mid-extra-calls-r29.json;
gap-C-switch-cost-compiler-r30/receipt.json.

r29's second deterministic input has completed. Against the matching r24
slice, store_motion has calls +11.05%, instructions +3.04%, executed bytes
-2.03%, and modeled L2 code misses -4.32%; number_literal modeled L2 misses
fall 5.45%. These agree with the hardware tradeoff above. r30's frozen gate
changes instructions -0.010%, -6.906%, -3.080% and -2.765% at O0–O3 against
r24, essentially unchanged from r29. Its LLVM output check and ledger remain
pending. Evidence: gap-C-mid-cost-versus-main-r29.json;
gap-C-switch-cost-batch-r30/summary.json.

### Continued cost-model controls and higher-level measurements

Fetched origin and attempted the requested rebase on 2026-11-11. Origin/main
is still 04d285786, already an ancestor of this worktree; Git reports the
branch up to date. The following measurements use immutable binaries from
that base. No private inliner policy has been adopted.

r30 now passes all 325 objects and the linked executable. Three subsequent
controls retain the same 24/36 base allowances:

- r31 consumes typed branch predictions in the existing callee CFG. It
  discounts known cost on predicted rare paths while preserving a quarter
  for copied code; unpriced operations remain unpriced. This analysis is
  built per callee, not per call site.
- r33 prices address calculations before examining the indexed element
  type. A constant local address used exactly once as a load/store address
  receives the discount for folding the backend already performs. Shared,
  returned or passed addresses retain their cost. This removes double
  counting, not a new memory transformation.
- r34 gives a loop call one additional removed-call cost as a benefit.
  Actual copy growth and caller budgets are unchanged; it does not restore
  the former fourfold body-size allowance.

All four controls reproduce every workload output. The following changes
are against the matching r24 43-provider slice, on store_motion:

| Control | Executed bytes | Instructions | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: |
| r30 switch prices | -2.055% | +2.937% | +10.430% | -2.953% |
| r31 rare-path prices | -1.983% | +2.743% | +9.649% | -3.004% |
| r33 address prices | -1.821% | +2.473% | +8.503% | -2.422% |
| r34 loop benefit | -1.709% | +2.065% | +6.655% | -2.241% |

r34's frozen compile instructions change -0.004%, -5.875%, -2.292% and
-1.975% at O0–O3 against r24. Its 80-observation O1 hardware screen preserves
all outputs and all ten flagged cycle outliers. Instructions rise 1.94–1.97%,
micro-operations 2.34%; store-forward blocks fall 3.49%, hardware L2 code
misses 1.89%, TLB walk cycles 6.94% and fetch stalls 1.82%.
Cycle changes range from -0.44% to +1.26% across event groups: no resolved
runtime win. The remaining extra callees include SmallPtrSet::insert_imp,
FoldingSetNodeID::AddInteger and redeclaration accessors. Unresolved
address-only symbols are retained but are not treated as identified callees.

The same private policy was screened at O2/O3 using new full r32 baselines
at the matching producer level, with native O2/O3 oracles. Both sets of 325
objects and executables match. Hardware instructions rise 1.68%/1.73%;
cycles rise 1.31–3.66%/0.43–1.32% across event groups. Store-forward blocks
improve substantially, but extra work and instruction fetch costs prevent
a win. Six/twenty flagged outliers are retained. This is a reason to keep
investigating the replacement model, not to ship it or report a gain.

Local LLVM 22.1.2 InlineCost.cpp exposes another controlled difference:
direct calls cost an instruction, argument setup and a five-instruction
call penalty. The prototype uses a smaller call overhead. Private r36
tests six units plus argument setup for both nested and removed calls.
r37 separately tests LLVM's default 45/65 ordinary/hinted allowances in
those units. These are diagnostics; the two IRs and their simplifications
are not assumed to have identical cost. Both results remain pending.

The accepted r24 compiler also rebuilt all 2,577 target C++ units of LLVM
at O2 and O3, without fallback. Unpaired build times under load were 351.54
and 386.88 seconds; these are capacity observations, not speed comparisons.
Both resulting LLVMs reproduce all 325 frozen cppgm objects and the linked
executable. The full O2 trial has completed 32 observations in five paired
blocks, including native A/A, with every output identical and binaries and
inputs unchanged:

| LLVM producer | Elapsed/native, mean [95% interval] | Cycles/native, mean [95% interval] |
| --- | --- | --- |
| Prior O2 tables-cost-returns r4 | 1.1838 [1.1537, 1.2138] | 1.1861 [1.1689, 1.2033] |
| Accepted main/aggregate O2 r32 | 1.1380 [1.1274, 1.1487] | 1.1475 [1.1408, 1.1542] |

The paired elapsed change is -3.835% [-6.226%, -1.445%]; cycle change is
-3.247% [-4.438%, -2.056%]. Instructions fall 1.155%, instruction-cache
misses 5.345% and fetch stalls 5.783%. The cycle-ratio drop is 3.862 points
[2.400, 5.323]. This compares the complete accepted changes with the prior
O2 checkpoint, not the private inliner, and is not isolated attribution to
one aggregate change. O2 is still outside the 5% target. O3 timing follows.
The cumulative compile baseline remains 8f76ebb0d.

Evidence: gap-C-{switch-cost,predicted-cost,address-cost,loop-benefit}-versus-main-*
JSON summaries; gap-C-loop-benefit-extra-calls-r34.json;
gap-C-loop-benefit-O{1,2,3}-hardware-r35/summary.json;
gap-C-o23-screen-r35.json;
gap-C-main-04d-O2-full-workload-r32/summary.json.

r36 and r37 now pass all 325 objects and the linked executable. Raising
the call price alone leaves extra work: r36 number_literal/store_motion
instructions are +1.545%/+1.812% and calls +4.594%/+5.232% against the
matching r24 slice. r37's separate allowance control recovers that loss:

| r37 against matching r24 slice | Executed bytes | Instructions | Calls | Modeled L2 code misses |
| --- | ---: | ---: | ---: | ---: |
| number_literal | -0.412% | -0.400% | -3.722% | -3.203% |
| store_motion | -0.437% | -0.315% | -4.198% | -1.136% |

Frozen compile instructions for r37 fall 3.367%, 2.485% and 2.326% at
O1–O3 against r24. Its hardware screen has instructions -0.35%, blocked
store-forward loads -11.83%, TLB second-level hits -6.17% and
micro-operations -0.61%, but L2 code misses +0.12% and fetch stalls +0.75%.
Cycle ratios vary from -4.73% to +0.77% with 27 flagged observations
retained. Do not treat that spread as a resolved runtime gain.

r39 adds constant-argument path pricing. When the entry branch directly
tests an integer parameter, a per-callee summary records each outcome's
remaining cost using the existing CFG. A call site with a literal argument
evaluates the typed predicate; it does not clone or rescan the callee.
Normal inlining and constant propagation still perform all transformations.
Safety checks, raw copy limits and the non-growing rule stay unchanged.
This is a limited implementation of context pricing, not general partial
evaluation or function-pointer specialization.

The equality, signed-comparison and reversed-comparison controls retain
observable effects and results in sixteen GCC/Clang/before/after executions
at O0–O3. The new compiler removes the three constant fast-path calls that
r37 retained. This control uses host linking; an initial attempt with the
course executable linker failed to resolve printf and is retained in the
log. All 325 LLVM-produced objects and the compiler also match. Eighteen
of the 43 provider objects change from r37. Against r24, number_literal/
store_motion instructions change -0.468%/-0.276%, calls -3.812%/-4.206%,
executed bytes -0.383%/-0.405%, and modeled L2 code misses -4.565%/-1.809%.
The r39-versus-r37 hardware screen shows L2 code misses -1.11%, fetch stalls
-3.28% and branch mispredictions -0.56%; instructions are approximately
flat. Nine flagged observations are retained. Full-workload confirmation
is still required.

Six PA33 LowIR/source lanes at O1–O3 report no behavior failure. Five
reference/quality differences remain: two exact-conversion fact comparisons,
the hinted 23-call body, the whole-body fast-wrapper size cap, and a vector
callee rejected because its instructions are unpriced. Expectations have
not been changed. r41 supplies prices for explicit vector operations from
the existing VectorShape vocabulary and ordinary f32/f64 operations. That
restores the vector check; the other four differences remain. All 43 LLVM
provider objects and the entire linked binary equal r39 exactly. The
redundant unfinished ledgers were stopped, with artifacts retained, and
r39's evidence reused by hash. r41 independently passed the full object
and executable replay before that reuse.

r42 removes analysis left over from the deleted dispatch admission rule.
The replacement model was still completing callee dominators/loop forests
and scanning caller bodies for a result it ignored. Removing that work
reduces frozen compile instructions by 0.593%, 0.572% and 0.550% at O1–O3
against r41. Frozen outputs at all four levels, the 43 provider objects and
the complete LLVM binary are identical. This is a confirmed compile-cost
recovery inside the private policy, not a new codegen result. Known-function
argument specialization remains a separate missing capability; do not
restore the old shape cap simply to consume its former analysis.

The fast-wrapper cap was checked separately without changing its fixture.
Over 100,000 calls the new and accepted builds produce equal results.
The fast-path whole-process instruction count differs by five instructions
total; the slow path saves 199,995, approximately two per call. This supports
reviewing the whole-body cap, but an isolated wrapper does not establish
the workload's instruction-cache behavior. The 23-call hinted body is also
a policy expectation: an inline hint does not require copying all its calls.
Any adoption must retain behavior, no-inline and meaningful quality checks,
then pass normal strict tests, audits, self-host and performance gates.

A full private O1 LLVM build of r42 is queued after the accepted O3 timing.
It will replay all 325 objects and the compiler, then run the four-input
ledger and hardware screen against full r24. Many small extra callees in
the native comparison are unchanged by the current slice; only the complete
build can establish the effect outside those 43 providers. No replacement
policy is adopted, and no 5% target is claimed.

Evidence: gap-C-{call-cost,native-allowance,entry-specialization}-versus-main-*
JSON summaries; gap-C-{call-cost,native-allowance}-hardware-r38/summary.json;
gap-C-entry-controls-r39/receipt.json;
gap-C-entry-specialization-hardware-r40/summary.json;
gap-C-entry-course-tests-r40/summary.json;
gap-C-vector-course-tests-r41/summary.json;
gap-C-vector-cost-evidence-reuse-r41.json;
gap-C-prune-analysis-batch-r42/summary.json;
gap-C-wrapper-guard-r44/summary.json;
queue_gap_C_cost_model_full_r43.py.

The r42 cumulative gate has also completed against the unchanged 8f76ebb0d
baseline: O0 +0.445%, O1 -4.474%, O2 -4.413%, O3 -4.428% compile
instructions. RSS ratios are 1.00009, 1.00007, 0.99998 and 1.00092.
All repeated outputs match and frozen inputs and binaries remain unchanged.
These savings do not establish runtime improvement; no additional runtime
credit is claimed for the private policy. Evidence:
gap-C-cost-model-cumulative-r43/summary.json.

The accepted O3 full-workload trial has completed all 32 observations, with
every object and executable identical and binaries and inputs unchanged:

| LLVM producer | Elapsed/native, mean [95% interval] | Cycles/native, mean [95% interval] |
| --- | --- | --- |
| Prior O3 tables-cost-returns r4 | 1.2025 [1.1829, 1.2221] | 1.2032 [1.1965, 1.2100] |
| Accepted main/aggregate O3 r32 | 1.1763 [1.1099, 1.2427] | 1.1630 [1.1562, 1.1698] |

Paired elapsed changes -2.184% [-7.291%, +2.924%], so the wall-time gain
is unresolved. Cycles change -3.341% [-4.243%, -2.439%], instructions
-1.229%, instruction-cache misses -5.072%, and fetch stalls -5.739%.
The cycle-ratio drop is 4.022 points [2.920, 5.125]. This is the complete
accepted main/aggregate checkpoint compared with prior r4; it is not the
private replacement inliner or isolated attribution to one pass. O3 also
remains outside the 5% target. Evidence:
gap-C-main-04d-O3-full-workload-r32/summary.json.

The private r43 full LLVM build has now started after the O3 trial ended.
