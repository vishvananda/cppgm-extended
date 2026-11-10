# LLVM gap strategy: 10 November 2026

The working target is cppgm-built LLVM compiling the 325 frozen cppgm units
in at most 1.05 times native-built LLVM's elapsed time, at every level.
The qualified O1 level moves now measure 12.99% overhead over five ABBA
blocks (95% interval 9.68%-16.31%). The target remains unmet. The preceding
measurements were 16.96%, 18.25% and 19.19% at O1, O2 and O3; O2 and O3 have
not been remeasured after this checkpoint. Those older runs and the current
one are not a paired proof of the improvement between compiler revisions.

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
## Final callee pricing: private screening

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
-2.142% at O2 and -2.436% at O3; RSS remains within the gate. These are private
candidate costs. The accepted baseline and proven-gain ledger do not change.
The full O1 LLVM rebuild completes in 394.68 seconds under load, with no C++
fallback; this is an unpaired build observation, not a compile-speed claim.
Its Clang SHA-256 is
`4a5325778b87aa9e7f7f09187dae7b2bb21f595863ab38e2872d736d9bc15e84`.
The output-verified five-block comparison is running. Normal qualification
and promotion depend on those results.

One existing maintainer control explicitly requires the old large-body loop
multiplier: `check_lowir_o3_loop_priority_inline.pl` passes with the baseline
and rejects the candidate because loop_first retains two calls instead of one.
If this policy change is adopted, replace that exact admission requirement
with behavior and code-growth bounds while preserving its noreturn/phi safety
coverage. This is a known qualification item, not a passing test claim.

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
