# LLVM O1 code generation investigation

The current target is elapsed time at most **1.05 times native LLVM**, with
equal or faster preferred. The O0 target was already met: 550.105 seconds
against 551.12 seconds, a ratio of 0.9982, with all outputs identical. LLVM O2
and O3 measurements remain deferred until O1 meets its target.

## What the measurement compares

Both producers compile the same local LLVM 22.1.2 sources at `-O1 -DNDEBUG`,
using C++17, with PGO and LTO disabled. The native producer is LLVM; the other
is cppgm. Native C/assembly compilation and the host linker are shared; no
C++ translation unit falls back to another compiler.

Each resulting Clang compiles the same 325 frozen cppgm translation units at
`-O3 -std=gnu++11 -fno-strict-aliasing`. These inputs are frozen at cppgm
revision `264e0e3e97373d8159bfd163a4a001336a951579`; changing the producer
does not change the workload. Both observations use the same object paths,
headers, resource directory and library configuration, with 12 jobs on the
Xeon E5-2696 v4 host. Every object and the linked executable must match the
native oracle in every observation.

The four fresh observations use cppgm/native/native/cppgm order. Two earlier
native A/A observations are retained with their compiler and input hashes;
the native image is unchanged. Frozen compiler performance gates separately
use fresh A/A observations and two paired ABBA blocks at both O0 and O3.
Raw observations, including noisy ones and failed experiments, are retained.

Ratios below are **cppgm-built LLVM / native-built LLVM**; smaller is better.

| Producer change | cppgm-built elapsed | Native elapsed | Elapsed ratio | Instruction ratio | Cycle ratio | I-cache miss ratio |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Rebased baseline | — | — | 1.2075 | 1.1145 | 1.2048 | — |
| Qualified small frame-copy fix, `f64db5623` | 52.740 s | 44.225 s | 1.1925 | 1.0887 | 1.1967 | 1.2107 |
| Three dataflow improvements, private diagnostic | 52.030 s | 44.295 s | 1.1746 | 1.0826 | 1.1864 | 1.2092 |
| Four qualified improvements, `9287a3c51` | 52.385 s | 44.225 s | 1.1845 | 1.0822 | 1.1898 | 1.2131 |
| Qualified return/dataflow batch, `f9dcf8899` | 51.925 s | 44.230 s | 1.1740 | 1.0765 | 1.1898 | 1.2182 |
| Symbolic loop entry plus Index CSE, private diagnostic | 52.290 s | 44.220 s | 1.1825 | 1.0811 | 1.1895 | 1.2144 |
| Sixth preserved register with fixed call areas, private diagnostic | 52.005 s | 44.660 s | 1.1645 | 1.0809 | 1.1845 | 1.2191 |
| Four frame/scheduling/mask/register groups, private R3 | 51.590 s | 44.200 s | 1.1672 | 1.0700 | 1.1748 | 1.1948 |
| Qualified frame/mask batch including alias form, `fc5519bc3` | 51.845 s | 44.165 s | 1.1739 | 1.0699 | 1.1776 | 1.1856 |
| Four memory/microcode changes, private diagnostic | 52.460 s | 44.950 s | 1.1671 | 1.0703 | 1.1779 | 1.2017 |
| Qualified alignment/return/copy batch, `194ce80d0` | 52.800 s | 45.340 s | 1.1645 | 1.0701 | 1.1793 | 1.1989 |

All completed rows passed output equality; the latest full runs
had 100% PMU coverage. The private diagnostic establishes a useful direction,
but it does not replace qualification of the final source tree. The 5% target
has not yet been met. The final four-change result does not establish an
incremental whole-workload benefit from word operations over the three-change
private diagnostic; their instruction ratios are essentially the same.

## Where the remaining work is

The slowdown is broad. In the rebased baseline all 325 translation units were
slower, the median ratio was approximately 1.21, and the 20 largest contributors
accounted for only 15.4% of the excess CPU time. Optimizing a single large input
cannot close the gap.

Disjoint LLVM phase timers on three representative frozen inputs put 55–74%
of their excess time in the frontend. The optimizer accounted for 15–29%; the
remaining phases were smaller. These are attribution measurements for those
inputs, not a census of the complete workload.

At `f64db5623`, full-workload instructions are 8.9% above native while cycles
are 19.7% above native. Cycles per instruction are therefore approximately
9.9% worse as well. Instruction-cache misses are 21.1% higher and instruction
fetch stall counts are 35.5% higher. Reducing instruction count alone is not
enough. An earlier one-input counter breakdown also found approximately 26%
more calls and returns, 17% more conditional branches and 21% more stores.

Assembly comparisons show recurring differences across shared routines:

| Routine | Native bytes | cppgm bytes at `f64db5623` | Native / cppgm static calls |
| --- | ---: | ---: | ---: |
| `GetFullTypeForDeclarator` | 26,611 | 37,340 | 392 / 585 |
| Function `PassManager::run` | 832 | 3,282 | 15 / 34 |
| `Value::getPointerDereferenceableBytes` | 1,114 | 2,606 | 21 / 22 |
| `Sema::CheckCompletedCXXClass` | 8,976 | 17,469 | 220 / 252 |

These sizes identify places to inspect, not runtime weights. A smaller wrapper
can leave another call layer, and a larger body can remove calls. Profiles and
whole-workload counters decide whether an assembly change helped.

## Current batch and regressions

1. Infer a callee's conservative read-only memory fact after its bottom-up
   simplification, making it available when callers simplify. Unknown,
   replaceable, volatile, atomic and opaque effects prevent the proof. This
   does not infer repeatability or move parameter-indexed write summaries
   ahead of signature changes. PA33's `200-callee-readonly-loads` checks both
   retained call results and a mutating aliased call.
2. Replace a whole private scalar copy immediately followed by its scalar
   read with an ordinary load and store using the read's SSA value. Slot
   bytes remain preserved. Volatile, partial, escaping, boolean and aggregate
   cases are excluded. PA33's `200-scalar-copy-read` includes a saved value
   followed by a write through an alias.
3. Reduce exact element distances between addresses with the same root to
   bounded integer offsets. A bounded SSA range proof prevents a wrapped
   byte difference from being mistaken for an unbounded element count.
   PA33's `200-pointer-element-distance` includes negative results and an
   unbounded input control. A cheap opcode check avoids querying this pattern
   on unrelated instructions: the first private implementation failed the
   compile gate at +0.84%; the guarded implementation passed.
4. Use 32-bit unsigned bitwise operations and nonzero constant word shifts
   where the instruction itself establishes the required upper bits. Signed
   wide consumers retain sign extension. The selector, low-bit-use analysis
   and ABI argument analysis share the width decision. A simple unsigned
   shift falls from four instructions to three, and the pointer hash from
   17 bytes to 11. PA34's `100-word-shifts-and-bitwise` checks logical and
   arithmetic shifts, signed wide reads, zero and variable counts, and hashing.

Private cross-compiler checks include 16,384 trials of 14 word-operation
functions called from an assembly shim that deliberately preserves arbitrary
upper argument-register bits. GCC, Clang and cppgm agree at O0 through O3.
Separate alias/copy, pointer-distance and effect-boundary checks also agree at
all four levels. These supplement the required PA33/PA34 behavior and quality
fixtures, full strict report, debug checks, self-host through PA5, inception,
PA36 quick/smoke, harness and architecture audits. Commit `9287a3c51` passed
all of these, including 10,572/10,572 strict tests. Its frozen instruction
ratios were 0.99995 at O0 and 0.99815 at O3, with flat peak memory. The O0
frozen output was byte-identical to the previous compiler.

## Next dataflow investigation

A read-only LowIR trace of the actual LLVM `PassManager::run` reproduces the
qualified object byte for byte. It shows an empty SmallPtrSet lookup whose
start and end addresses differ only in an Index projection label. That label
is nonsemantic, but value numbering currently includes it in expression keys.
Ignoring it enables address CSE. Removing the loop also requires proving
symbolic phi equality on the entry edge and repairing values live after
the loop.

A private prototype removes that loop and agrees with GCC and Clang in
7-function, 8,192-iteration checks at all four levels, including nonempty
loops, conditional entry, parallel phi swaps and NaN controls. Its general
SSA-repair implementation adds 2.35% to frozen O3 compilation and fails the
performance gate; it is not included in the production compiler. A narrower
implementation restricted to a single-phi equality test passes the frozen
gate at +0.44% O3 instructions, with O0 unchanged, and passes the complete
PA33 course suite. Its full LLVM rebuild succeeds, and every output in the
full-workload comparison is byte-identical to native. Elapsed time is 52.29
versus 44.22 seconds: 18.25% slower. Whole-workload instructions improve by
only about 0.1% relative to the qualified compiler and cache counters remain
essentially unchanged. This change alone does not explain the remaining gap.
The new `200-symbolic-loop-entry` regression fails the old compiler at ten
instructions and four blocks versus an envelope of three instructions and
one block; the candidate produces two instructions and one block. A nonempty
volatile traversal remains an observable behavior control. This is a
demonstrated code-shape gap, not yet a demonstrated whole-workload improvement.

A second private probe eliminates equal-width pointer/integer bit-copy round
trips. Such copies already disappear in native selection but remain barriers
in the optimizer value graph, preventing some tagged-pointer mask folds.
This does not change conversions or pointer arithmetic. GCC/Clang comparison
controls and frozen compilation pass; full-workload impact is not yet known.

## Experiments that did not explain the gap

- Removing shrink wrapping entirely left elapsed time essentially unchanged
  and increased instructions and cache misses. It is not promoted.
- Broader addressed-scalar promotion saved some instructions in a small screen
  but did not close the full-workload gap. Merely enabling an O2 policy at O1
  does not address the shared problem.
- More aggressive small-wrapper admission could inline `getTranslationUnitDecl`
  into `getASTContext`, but the scoped runtime measurements did not establish
  a useful improvement and some cache counters worsened. It is not promoted.
- Disabling native TBAA in the two selected hot AST units changed very little
  code and did not explain their gap. This does not rule out alias-analysis
  opportunities elsewhere.
- Fourteen selected LLVM type layout, alignment and triviality queries agree
  between producers. Allocation counts and requested bytes on the profiled
  input differ by less than 0.1%. Those checks did not reveal a large source
  selection or allocation-volume difference.

## Evidence

The machine-local evidence root is `/mnt/vizier/cppgm-llvm-evidence`:

- `o1-course-rebase-main-r1-gap-analysis-r1/`: per-input cost distribution.
- `o1-eff8-phase-timers-r1/`: disjoint phase attribution.
- `o1-current-store-motion-profile-r1/` and `o1-current-hot-code-r1/`: profiles
  and native/current assembly comparisons.
- `o1-small-frame-copy-readers-main-r2-clang-runtime-timing/`: qualified baseline.
- `o1-memory-and-distance-diagnostic-r2-clang-runtime-timing/`: three-change
  diagnostic, all object hashes and raw paired observations.
- `o1-no-shrink-wrap-diagnostic-r1-clang-runtime-timing/`: rejected causal ablation.
- `o1-word-operations-cases-r3/`: GCC/Clang comparisons and the raw-register shim.
- `o1-four-dataflow-regression-before-r1/`: failing pre-fix quality checks.
- `o1-four-dataflow-main-r2-*`: final batch qualification and measurements.

## Subsequent backend probes

A private return-tail cleanup follows at most eight consecutive full-register
copies and lets an independent address/load/extension/constant assignment
produce RAX directly. It excludes frame and callee-saved destinations and
two-address arithmetic. It must run after return-block duplication: an earlier
implementation passed correctness controls but missed the actual hot path.
The later implementation reduces `SmallPtrSet::FindBucketFor` from 115 to 104
bytes and from 39 to 35 instructions. Its combined frozen compile gate passes;
20 GCC/Clang/before/candidate ABI comparisons agree. Whole-workload impact
has not been measured.

Another private probe makes RBP available as a sixth preserved register across
fixed call argument areas aligned at most 16 bytes. If RBP is actually used,
the final frame choice overrides the frame-access-count profitability veto;
dynamic stack, varargs, realignment and other frame safety checks stay in
force. O0 output is unchanged. A hot AST routine shrinks by 127 bytes, but
the AST-only prefix screen is flat in instructions and worse in measured
cycles; it does not establish a runtime win. The full build/comparison
finishes at 52.005 versus 44.66 seconds, with every output identical and
100% PMU coverage. The cycle change is smaller than the elapsed change, so
this is not evidence of a two-percentage-point causal improvement. ABI controls include stack integer/floating arguments, aligned
objects, varargs, alloca, exceptions and two-register returns.

## Qualified return and dataflow batch

The source tree containing Index CSE, symbolic phi equality with bounded
SSA repair, pointer/integer bit-copy round-trip folding and return-tail
coalescing passes 10,575/10,575 strict tests, all architecture and placement
audits, the harness, debug checks, self-host through PA5, inception and
PA36 quick/smoke. The frozen instruction ratios against `9287a3c51` are
0.99997 at O0 and 1.00461 at O3, below the 1.005 gate; peak memory is flat.
O0 output is byte-identical. Unchanged compiler checks from the first
validation are explicitly reused, with a fresh strict report, all audits,
modified-PA placement and harness run after the reference/control updates.

PA33 owns `200-symbolic-loop-entry` and `200-pointer-bit-copy-roundtrip`;
PA34 owns `200-return-tail-copies`. All three fail their pre-fix quality
envelopes and pass with the candidate. Four exact MIR example references
are regenerated with the normal worker/canonicalizer. The old acyclic-phi
control required particular physical register equality, which rejected an
equivalent direct return in RAX. It now executes all four branch choices
and verifies the selected values, earlier store and simultaneously live
value at O0 through O3. It imposes no register-allocation prescription.

The qualified batch's full LLVM rebuild finishes at 51.925 seconds versus
44.230 seconds, with all 325 objects and the executable identical in every
observation. Its elapsed ratio is 1.1740; the target remains unmet. The
RBP and separate-latch scheduling probes subsequently join the next private
batch. The latter extends an existing one-block
loop scheduler to a sole separate latch, bounded to 64 instructions and
constant integer/pointer updates. It removes a real back-edge copy and
passes GCC/Clang controls at all levels. Its frozen O0 and O3 objects are
identical to the baseline, with essentially flat compiler instruction cost.

## Four-group frame and code-size batch

The next private compiler combines the fixed-call-area RBP proof, bounded
separate-latch scheduling, narrower masked loads and mask-window tests, and
preference for shorter caller-saved register encodings. It leaves O0 code
unchanged. Its frozen instruction ratios against `f9dcf8899` are 1.00013
at O0 and 1.00052 at O3, with flat peak memory. GCC/Clang behavior checks
at O0 through O3 include 16 ABI/loop functions over 8,192 trials, 12 masked
load functions over 65,536 trials, and 16 mask-test functions over 65,536
trials. Negative controls cover atomic/volatile accesses, values retained
across an aliasing write, and sign-promoted byte/halfword values whose
neighbors must not be read instead.

This compiler builds all 3,365 LLVM C++ translation units. Its resulting
Clang passes complete native output equality at 51.590 versus 44.200 seconds:
1.1672 times native elapsed, 1.0700 times instructions, 1.1748 times cycles,
1.1948 times I-cache misses and 1.3304 times instruction-fetch stall counts.
PMU coverage is 100%. The remaining cycle gap still includes approximately
9.8% worse cycles per instruction. This is a modest improvement, not a
resolution of the remaining gap. Local wins include a 109-to-96-byte
`getTranslationUnitDecl` and about 1.8% less ASTContext object text from the
register preference alone; those sizes are not runtime weight estimates.

Regression construction then exposes a missed alias form: canonical sign
extensions can already have disappeared before load folding. The next
candidate applies the same bounded read-width proof to that form. Its
`signed_word` and `signed_byte` cases now each need one load and a return;
controls needing sign-promoted high bits still use the original narrow read
and mask. Commit `fc5519bc3` passes 10,578/10,578 strict tests, all architecture
and placement audits, the harness, debug checks, self-host through PA5,
inception and PA36 quick/smoke. Its frozen instruction ratios are 1.00015 at
O0 and 1.00052 at O3, with flat memory and identical O0 output. Required
regressions belong to PA33 for the loop schedule and PA34 for native masks,
frame/ABI and load-width controls.

The final full LLVM measurement is 51.845 versus 44.165 seconds, a ratio of
1.1739. All 325 objects and the executable match native in every observation,
and PMU coverage is 100%. Instructions are 6.99% above native and cycles are
17.76% above native, leaving about 10.1% worse cycles per instruction. I-cache
misses are 18.56% above native and fetch stalls 32.47% above native. The earlier
private row's elapsed improvement is not confirmed by this final result;
instruction and cache ratios improve modestly, while the 5% target remains
unmet. O2 and O3 LLVM measurements remain deferred.

Thirty-seven internal exact text examples change with the register allocation
and counter ordering. Before their normal worker/canonicalizer regeneration,
all existing compile statuses and program outcomes match their old hashes.
The two LowIR diffs contain the same instructions in the same blocks, with
independent counter steps reordered; an additional tail-recursion driver
agrees before/after at all four native levels. Generated executables are
restored rather than committed. The fresh strict report passes after only
these affected textual references are regenerated; the compiler stays
byte-identical throughout the reference review.

A separate size ablation halves hinted and loop-site admission caps and
disables original-size admission of already-grown callees. Across 32
profile-selected LLVM units text falls from 13,557,255 to 12,752,745 bytes.
On the two fixed heavy inputs its A/A plus ABBA screen instead executes
1.3% more instructions, 0.7% more cycles and 3.1% more I-cache misses, with
flat elapsed time. All outputs match native, but the ablation is rejected:
less total text alone does not identify the needed hot-code improvement.

Raw evidence for these measurements is in `o1-codegen-packet-private-r3-*`,
`o1-codegen-packet-diagnostic-r3-*`, `o1-codegen-packet-store-motion-profile-r3/`,
`o1-codegen-packet-*-cases-r3/` and `o1-inline-size-top32-*` under the evidence
root. The alias correction's cross-compiler checks and frozen gate are in
the corresponding `r4` directories.


## Microcode attribution and alignment-return facts

A new PMU breakdown on the same frozen heavy input finds conditional branches
at 1.1646 times native and near calls/returns at 1.2469 times native. Missing
uop-delivery slots are 1.2900 times native; microcode-sequencer uops are
2.0057 times native. These are one-input diagnostic ratios. They do not mean
that microcode accounts for half the whole-program slowdown.

Sampling microcode-sequencer uops, with instruction offsets resolved relative
to ELF symbols to handle ASLR, identifies concrete code differences:

| Routine | Before | After `194ce80d0` |
| --- | --- | --- |
| `ASTContext::getFunctionTypeInternal` | Two 138-byte string copies | Direct chunks; 3,199 to 3,457 bytes |
| `NamedDecl::getLinkageInternal` | 136-byte string zeroing | Direct stores; 164 to 226 bytes |
| `TypeLocVisitor<NextLoc>::Visit` | 13 alignment divisions | No divisions; 1,276 to 1,068 bytes |
| `DataLayout::getTypeAllocSize` | Four alignment divisions | No divisions; 656 bytes either way |
| `DataLayout::getIntegerAlignment` | Byte store followed by wider reload | Exact payload return; 87 to 75 bytes |

The first two changes trade additional instruction bytes for avoiding string
startup overhead. They therefore require workload measurement, even though
the microcode operation disappears. The first combined full run of those
changes, exact small returns, shifted-power proofs and copy-home facts is
1.1671 times native elapsed. Adding the callee-return proof yields 1.1645.
The instruction and cycle ratios are effectively flat between these runs;
the small elapsed difference does not establish a substantial incremental
win. The 5% target remains unmet.

The missing alignment fact is visible in typed SSA: a function returns only
1, 4 or 8, with those alternatives merging through a phi and sometimes a
narrowing conversion. The previous caller proof accepted literal returns
only. The new bounded proof follows copies, integer conversions, selects and
phis, rejecting zero, nonpowers, unsupported operations, cycles beyond its
budget, weak/replaceable definitions and incompatible call results. Calls and
their side effects stay. Each value walk memoizes at most sixteen callee
queries; it does no repeated unbounded body scans. A full-width shift of one
also needs no separate count-range proof on a defined execution. Narrow
sign-extension controls retain the range restriction.

The other fixes read byte/halfword aggregate return payloads at their actual
width, share the direct-copy/zeroing policy through 256 bytes between emission
and clobber analysis, and retain GPR home facts across copies that clobber only
the reserved scratch registers. O0 output remains identical.

Qualification passes 10,583/10,583 strict tests, all architecture and global
placement audits, fresh PA33/34 placement, the harness, debug checks, self-host
through PA5, inception, and PA36 quick/smoke. All 64 producer/level checks across
four independent GCC/Clang matrices pass. Controls include all aggregate
payload sizes from 1 through 16 bytes, copy boundaries around 128 and 256,
canaries and live scalar values, signed extension, nonpower/zero alternatives,
side effects and a host-compiled strong override of a weak definition.
Required regressions live in PA33 and PA34. A leftover maintainer control
required exactly `rep movsb` for 129 bytes. It now executes native and driver
paths at all four levels, leaving encoding choice to the compiler.

The frozen compile gate against `fc5519bc3` reports instruction ratios of
1.000008 at O0 and 0.999828 at O3, with flat memory. The full LLVM build uses
all 3,365 C++ units without fallback and retains identical generated headers.
Every measured object and the linked cppgm executable matches native, with
100% PMU coverage. The final full comparison is 1.0701 times instructions,
1.1793 times cycles, 1.1989 times I-cache misses and 1.3475 times fetch stalls.
The remaining CPI cost is about 10.2% above native.

Raw evidence is in `o1-qualified-microcode-profile-r4/`,
`o1-qualified-frontend-and-branches-r4/`, `o1-return-and-microcode-main-*`, and
`o1-return-and-microcode-diagnostic-r1-*` under the evidence root. The accepted
receipt binds the final source, all tool hashes, normal checks, independent
controls and full LLVM observation. LLVM O2 and O3 remain deferred.
