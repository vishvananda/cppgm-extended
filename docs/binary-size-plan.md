# Binary size investigation and implementation

Bring generated compiler text and the stripped compiler close to 1.15 times
GCC while preserving native speed and compiler throughput. The size target is
soft: a reproducible individual slowdown outweighs a small size overrun.
No candidate is accepted for main yet. This maintainer record is excluded from
the student export.

## Closeout

Compiler changes are frozen at `f8bfc70ab`. The user approved closing the size
work and raising PA35's text-size allowance to 1.20 times the seed, retaining
the existing 1% release headroom. The unpublished inlining rollback was
reverted; literal, encoding and expanded-unrolling experiments stay outside
the PR. No further optimizer changes are planned for this closeout.

Fresh matching-source builds measure course text/stripped ratios of
1.1383/1.1556 times GCC and student ratios of 1.1413/1.1586. The frozen
251-unit rebuild confirms 6,370,434 text bytes, or 1.1383 times GCC. All 81
executables rebuilt from the published source match the measured candidate's
hashes. The small stripped-size overrun is within the user's stated allowance.

All 17 normal PA35 gates passed in both configurations. The completed release
measurements pass all 17 additional release checks after re-evaluation at the
approved 1.20 text limit; compiler measurements were not repeated. Strict
8,015-case, debug, self-host, inception, architecture/harness and export checks
pass. Boost CI passes 1,818 checks in 17 suites. Matching final GNU/self-built
compile-cost receipts have also been collected. Public EPYC CI is pending.

Known local individual runtime losses remain documented in `~/comparison.md`
and the private measurement receipts; these results do not claim every program
became faster. The following sections retain the investigation, including
rejected experiments, for review.

## Current approach

The coherent review checkout is `affine-review-source`, branch
`review/general-affine`: the general candidate, exact unrolling cost, guarded
typed-EH inlining, modular integer composition and scalar lane-width proofs.
It retains the compact unwind encoding. The `compiler` checkout now holds this validated source and the gate-only
closeout changes; the rejected layout work remains archived separately. Experimental
mask narrowing, scalar/shuffle folding and padding encodings remain separate.
Retain the established counted-loop and function alignment policy. Experiments
with tiny-entry spacing cutoffs, floating-function entry phases and
backedge-length thresholds exposed layout sensitivity but did not establish a
general profitability rule; they are rejected controls.

Benchmarks diagnose costs; they are not optimizer inputs or specifications.
Require a semantic proof or a general encoding/copy-cost argument, and check
neighboring positive and negative cases. Never identify a benchmark by name,
source spelling, expected instruction sequence or desired function address.

The retained reductions address retained code, not a large pool of unreachable
blocks. Reachability checks found no disconnected LowIR blocks in three
representative inputs, including exception edges; linker garbage collection
removed only 47,904 compiler text bytes. In the investigated Boost container
program, extra moves and address calculations account for roughly two thirds
of the extra decoded bytes.

## Retained mechanisms

- Bound repeated expansion of larger calling bodies outside loops in large
  callers. Preserve useful loop and handler sites, small helpers, mandatory
  copies and definition-removing inlining; retain caller and unit budgets.
  Same-literal return proofs include all integer widths and both I128 words.
- Retain addressable floating fields when promotion creates costly non-header
  merges across a larger set of fields. Preserve ordinary scalar promotion.
- Combine complete adjacent zero stores and remove stores proved dead on
  ordinary and exceptional paths. Preserve holes, volatile accesses, atomics
  and every handler's observable state.
- Omit unused machine epilogues and unwind states beyond the last instruction.
  Exclude trailing jump-table data from FDE coverage. Share non-local EH
  reference cells through hidden weak COMDAT definitions.
- Share strongly bound equivalent ABI entries only after inlining and only
  with a serialized LowIR proof. Weak and local entries remain independent;
  do not introduce forwarding jumps. O0 skips the equivalence index.
- Reuse already excluded switch ranges, delay selector widening until needed,
  commute equivalent indexed addresses to avoid a displacement byte, and
  cache an unchanged call fact across encoding iterations. No new CFG pass.
- Price unrolling by the instructions actually copied: full iterations plus
  the final header test. Existing growth and trip limits remain unchanged.
  Independent signed ascending/descending and empty-loop cases check calls,
  aliased state, order and final counters at O0 through O3.

## Size and compile cost

Frozen source is the same 251 translation units at `8d4c68b9e379`, GNU C++11,
`-O3`, libstdc++ and `-fno-strict-aliasing` on both sides. A changed-source
self build and the student/plain-flag configuration remain separate checks.

| Frozen-source build | Text bytes | Text / GCC |
| --- | ---: | ---: |
| GCC | 5,596,214 | 1.0000 |
| Original cppgm++ | 7,143,970 | 1.2766 |
| General candidate | 6,333,890 | 1.1318 |
| General candidate plus exact unrolling cost | 6,334,210 | 1.1319 |
| Lean guarded-EH candidate | 6,335,970 | 1.1322 |
| Guarded EH, original few-call-site budget restored | 6,370,434 | 1.1383 |

The general candidate's stripped size is 7,420,264 bytes versus GCC's
6,449,536, or 1.1505 times GCC. This small overrun is preferable to the
rejected layout policies' native slowdowns. The exact unrolling fix adds 320
text bytes and leaves the stripped file size unchanged. The original-budget
variant is 7,457,128 stripped bytes, or 1.1562 times GCC; it is still undergoing
runtime validation. This small overrun does not authorize an individual
slowdown.

On the fixed semantic compile workload, the general candidate's GNU-built
instruction efficiency against the original compiler is 0.9993 at O0 and
1.0140 at O3. Its self-built efficiencies are 0.9916 and 1.0020. Adding the
unrolling correction changes GNU-built instruction counts by less than
0.003% at either level. Shared-host A/A timing variation prevents equally
precise wall-time claims. Final self-built and affected Boost checks remain
required.

## Fresh comparisons with GCC and Clang

All 81 public programs were rebuilt with actual GCC 15.2 and Clang 22.1.2,
identical pinned inputs and the public O3 flags. All outputs agree. Regression
comparisons separately use the original cppgm binary. Do not mistake the
historical `gcc` label in private regression build records for actual GCC.

The table is the diagnostic baseline before the EH follow-up. It shows
reference CPU time divided by candidate CPU time, so higher is faster. Life has six calibrated mirrored paired blocks per reference and
includes the integrated unrolling fix. Other general-candidate rows have
three screening blocks and require follow-up before precise ranking.

| Program | vs GCC | vs Clang | Evidence and next distinction |
| --- | ---: | ---: | --- |
| Life | 1.026 | 0.962 | Exact copy pricing enables useful bounded unrolling. |
| fmt | 0.814 | 0.860 | Copy/append helpers account for about half the extra work vs GCC. |
| Spirit | 0.823 | 0.939 | Same mutex work; investigate parser/shared-pointer overhead. |
| Matrix | 1.001 | 0.782 | Clang vectorizes and heavily unrolls; both others remain scalar. |
| Bullet | 0.889 | 0.871 | Inspect hot constraint solving, register traffic and scalar work. |
| tramp3d | 0.842 | 0.961 | GCC executes substantially fewer instructions. |

Life is about 8.4% faster in CPU time than the original cppgm build and
executes about 9% fewer instructions. All other 80 executable hashes are
unchanged by the unrolling correction.

A private straight-line return-cost credit improves fmt to 0.851 versus GCC
and 0.903 versus Clang, but repeated Box2D blocks show about 0.6% slower code
than original cppgm. It is not integrated or accepted. Do not narrow the rule
to benchmark-specific shapes to preserve only its favorable results.

Callgrind counts Spirit at 23.643 billion instructions for GCC, 27.766 billion
for Clang and 27.994 billion for cppgm. Mutex lock and unlock instruction
counts are identical across all three. GCC folds more surrounding work into
the skipper; cppgm retains a costly weak-pointer lock helper with real typed
catches. Review the EH inliner's proof boundary, nested cleanup and selector
mapping before considering broader admission. For fmt, inspect aggregate
argument/return copies and missed scalar replacement as well as inlining.
These are candidates for general work reduction, not reasons to raise budgets
indiscriminately. A constant-return summary may also expose caller facts
without copying a side-effecting callee; this is a proposed follow-up.

A hosted-header macro diagnostic (`-U__NO_INLINE__`) changed none of the 81
executables. It does not explain these gaps; no production macro change was
made.

### Refreshed reference comparisons and a larger arithmetic opportunity

Fresh mirrored comparisons use the restored-budget/lane-demand candidate and
the actual GCC/Clang reference ELFs above. Three calibrated blocks per cell
give these diagnostic CPU efficiencies (reference time / candidate time):

| Program | vs GCC | vs Clang |
| --- | ---: | ---: |
| Matrix | 1.003 | 0.787 |
| fmt | 0.830 | 0.885 |
| Spirit | 0.865 | 0.974 |
| Loop unrolling, before affine rewrite | 1.917 | 0.401 |
| Loop unrolling, affine prototype | 2.795 | 0.583 |

These are distinct from original-cppgm regression comparisons. In the matrix
kernel, native profiles put over 99% of each program's cycles in `main`.
Disassembly confirms packed integer multiplies in Clang and scalar multiplies
in both GCC and cppgm. The existing interchange proof requires statically
distinct symbol/slot roots; the input uses separately allocated rows reached
through pointers. Broadening that requires a valid dependence proof or
runtime checks, not merely admitting the loop or raising its growth budget.

The arithmetic comparison exposed a separate missing simplification. GCC
composes adjacent integer constant multiply/add operations, while cppgm kept
three dependent multiplies. The prototype composes
`((x * a) +/- b) * c` into `x * (a * c) +/- (b * c)` at the common integer
width. It consumes single-use intermediates, clears invalid arithmetic facts
and leaves floating operations ordered. This reduces work in the existing
simplifier without adding a pass, increasing inlining budgets or recognizing
source names/constants. Independent encoding checks found that composing two
cheap scaled products could introduce a slower multiply. A guard now retains
those products when the combined factor needs a general multiply. The guard
changes none of the 81 public executables relative to the arithmetic prototype.

All 81 public outputs agree. An independent GCC-built modular arithmetic
oracle checks 99 functions, including signed/unsigned 8/16/32-bit, 64/128-bit,
shared-producer and cross-block cases, at all four levels with 10,000 samples
per function. Ordered floating controls pass, and optimizer pass verification
passes at O1/O2/O3. The loop-unrolling program measures 1.431 CPU efficiency
and 1.435 instruction efficiency against original cppgm in three paired
blocks. Only four public executable hashes change relative to the immediately
preceding candidate. The other three changed programs have six paired blocks
each: miniFE CPU efficiency 1.0044, PAQ8p 1.0111 and minisat 1.0017 against
original cppgm. None demonstrates a slowdown in these cohorts.
Frozen compiler text stays 6,370,434 bytes, 1.1383 times GCC. The coherent
source and matching nine GNU tools are in `affine-review-source`. Its current
8,015-case strict report, debug checks, self-host through PA5 and matching
inception compiler/objects pass. None of this accepts the outstanding
per-program regressions.

The new PA32 arithmetic fixture checks wrapping at every integer width,
shared intermediates and a floating-rounding counterexample through behavior
and broad size envelopes. Two O2 host cases cover nested catches and a shared
normal/handler continuation. The host runner and placement auditor discover
numbered `.t.N.lowir` inputs and reject duplicate translation-unit indexes.
PA33 covers scalar shuffle selections, poisoned upper lanes, multiple uses,
live scalars and reloads across an ABI call. The width counterexample fails
the old backend at O1/O2/O3 and passes the corrected backend at all levels.

Architecture/format audits, the complete harness suite, file/placement audits,
all three PA35 smoke checks and the student document audit pass. A fresh
student export verifies 22,415 portable reference output files and its six
new cases pass through the exported runners. The current GNU source build
emits no warnings. Final PA35 release reassessment, matching GNU/self compile-cost checks and
Boost CI have completed. See the closeout above for remaining runtime caveats.

Separate encoding experiments narrow only zero-flag register mask tests,
fold adjacent scalar insertion/shuffle operations, and pack existing padding
with the generic ten-byte NOP form used by
[LLVM](https://llvm.org/doxygen/X86MCInstLower_8cpp_source.html). None changes
the production checkpoint yet. The padding-only experiment preserves every
code address, non-NOP byte, symbol table and unwind byte in all 81 programs.
Combining folding and padding removes the extra executed instructions in
FloatMM, but its first six-block cohort still has a native slowdown. This is
not a basis for accepting the candidate or adding benchmark-specific layout
rules.

## Latest EH and regression follow-up

The unrestricted EH prototype removes the weak-pointer lock helpers and
reduces Spirit's executed instructions by about 5.2%, but expands exception
work in an exception-only dispatcher. Most extra instructions execute in
libgcc's CFI interpreter. The lean candidate limits the new typed-EH admission
to callees with an already-proven call-free normal return; it retains the
existing budgets and does not add a size threshold. Six paired blocks give
Spirit CPU efficiency 1.0233 against original cppgm. The exception stress
program's executable is byte-identical to the preceding validated candidate
and measures neutral-to-faster in six follow-up blocks.

Handler reachability is a safety condition, not a profitable-shape match.
Independent nested catches, rethrows, cleanups, noexcept recovery and 16 mixed
compiler links pass at all four levels. A separate LowIR/host test merges
normal and handler control flow before a nested catch. A negative control
using the old dominance-only handler test corrupts the original exception at
O2/O3; the reachability guard preserves it.

The explicit-unwind-rule experiment preserved every decoded rule across all
18,610 public FDE ranges and 9,162 frozen-compiler ranges, including all 346
stack-argument transitions. It added about 82 KB to the stripped compiler
without enough native benefit and is excluded. The constant-return-summary
experiment is also excluded after a repeatable RapidJSON slowdown. All raw
observations and both successful and rejected controls are retained.

Restoring the original 128-instruction few-call-site budget and original
small-caller rule removes the later loop-specific exception. This is the exact
original general policy, not a new tuned threshold. It adds about 34 KB of
text and improves fmt in its first six-block cohort; other changed programs
still need validation.

Do not declare the native bar met. Repeated lean-candidate checks still show
losses in FloatMM, Mandelbrot, fannkuch and Box2D. Diagnostic disassembly shows
Mandelbrot 5 and fannkuch 3 have identical instructions and relative offsets,
with function starts shifted by 16 bytes. FloatMM instead has an inlined
matrix body and extra executed instructions. A global ablation of LEA operand
commutation changes none of those four numeric executables, excluding that
rewrite as their cause. Front-end/uop counters are being collected before
considering a general layout change. Inspect FloatMM's inlining/register
pressure separately; do not encode benchmark-specific padding or rules.

## Validation and completion

The integrated unrolling candidate passes all 8,009 strict cases, debug-info
checks, self-host through PA5, matching inception compiler/objects and all
architecture/format checks. Its only changed existing exact reference is the
PA32 unroll-budget regression, regenerated with the owning runner. The new
course fixture has generated size bounds and behavior checks, with no exact
instruction-matching requirement. File/placement audits and all three PA35 smoke checks pass. Full release,
generated-program and export validation still need to cover the final chosen
source.

The preceding general candidate passed 6,002 generated programs at each
level and 40 mixed GCC/Clang links. The exact-unrolling checkpoint now passes
all 17 PA35 release gates in both course and student configurations, including
the unchanged additional margins. The student scaling follow-up is 2.22;
the earlier failed 2.64 observation remains retained. The lean guarded-EH
review candidate independently passes all 8,009 strict cases and its mixed EH
checks; its final self-host, release and student export checks are pending.
The original-budget variant has not inherited those validation claims.

Finish by resolving every repeatable individual regression, then freeze the
simplest accepted source, measure GNU/self-built O0/O3 cost and Boost behavior,
run all required checks and validate a student export. Update
`~/comparison.md` and the draft PR using higher-is-better performance ratios
and explicit measurement limits. Main and release tags remain unchanged.

Raw commands, every observation, rejected controls and receipts are under
`/home/vishvananda/work/boost-1.92/validation/20261103-size-implementation`.
The adjacent `20261103-code-size-attribution` contains the initial size study.
