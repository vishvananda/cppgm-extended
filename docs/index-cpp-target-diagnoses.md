# Refreshed index-cpp diagnoses

Maintainer-only evidence for [the optimization plan](index-cpp-optimization-plan.md).
This document does not change the student requirements. Accepted mechanisms
and their validation are in [the progress record](index-cpp-optimization-progress.md).

The upstream input remains `79e0b423094d420da37fb714f20c53fc35eeb2a3`.
Receipts below are relative to
`/home/vishvananda/work/index-cpp/optimization/20261105/`.
The refreshed profiles use the eighth compiler's immutable binary, with stock
GCC 15.2 and Clang 22.1.2. Profiles localize work; calibrated paired runs decide
runtime claims. Every compiler still runs the original full workload.

## Nbody: new invariants appear after motion

The hot region is `CLBG::Nbody::run`, including its fixed warmup. Sampling puts
99.96% of generated work there. The normal host-object selector reports 86
MIR instructions, no calls, no spills and no frame homes. Warm function bodies
are 444 bytes for cppgm, 455 for GCC and 439 for Clang. More stack promotion or
larger inlining budgets would not address this region.

Packed velocity and position updates already exist. The optimized host LowIR
and MIR show a floating scalar broadcast repeated inside the loop after the
last ordinary invariant-motion step. Packing introduces that broadcast late.
Reusing the existing motion pass once after changed packed work removes it;
functions whose packing does not change receive no new cleanup. Reuse the
previous loop-presence result instead of rebuilding loop analysis just to
admit the cleanup. This is the ninth mechanism; its fresh export also passes.

The independent oracle covers 16,384 cases per compilation path, including
zero-trip null pointers, otherwise trapping integer division, aliasing and
volatile inputs, floating infinities/NaNs/signed zero, callbacks and unwinding.
All four optimization levels and source/serialized paths pass. The required
adjacent-update quality kernel uses a separate host scalar oracle and broad
instruction ratios, permitting other implementations.

Five paired blocks give 1.024× instruction speed and 1.012× process CPU speed
against the eighth compiler. Base64 and Huffman instructions are unchanged.
Frozen compile instructions remain within A/A calibration; the independent
whole-index compile control also finds essentially unchanged instruction cost.
All 74 public and four Boost sentinel objects are byte-identical. Receipts:
`nbody-profiles-dse/`, `late-packed-invariants-v1/runtime/`,
`late-packed-invariants-v2/index-compile-cost/`, and
`late-packed-invariants-ported*`.

This only closes the late-invariant issue. Reference-only vectorization
ablations prove another opportunity: stock GCC and Clang use about 7% fewer
instructions than the eighth compiler, while disabling their basic-block
vectorization makes both use more. Our store-rooted packing leaves the
coordinate/distance producers scalar where their values have external users.
Any extension must preserve the ordered squared-distance sum, division and
square root; fast math, approximate reciprocals and reassociation are outside
the experiment. A varied non-benchmark producer/extraction reducer and a
register-pressure cost proof are still needed. Ablations and remarks are in
`nbody-reference-ablations/` and `nbody-reference-remarks/`.

## Regex: specialization works; uniform local memory remains

The hot region is the matcher, plus its initialization. The refreshed cppgm
profile assigns about 76% of samples to the matcher and 21% to libc memset;
Clang also spends substantial time in memset. GCC inlines its fills with
string instructions, so its 99% matcher attribution includes that work.
The host selector reports 1,753 MIR instructions and 20 frame homes. These
homes are not evidence of 20 allocator spills: its spill count is zero.

An earlier hypothesis blamed missing constant-argument specialization. The
optimized LowIR disproves it: the matcher has only its token and text
parameters left. Capture count, start offset and capture-buffer pointers have
already disappeared. Increasing specialization or inlining budgets is not
justified by this profile.

One remaining local table is filled with 2,048 bytes of `0xff`, never written
again and observed through dynamically indexed integer loads. Constant capture
arguments removed its writes. The current bulk-call simplifier handles zero
fills, and constant-slot coalescing handles copies from immutable globals;
neither folds this uniform nonzero local fill. The first missing fact is thus
local-memory content propagation, after the relevant control flow simplifies.

The eleventh mechanism recognizes a complete constant-byte fill at entry,
rejects every other write, escape, volatile access or unsupported observation,
and replaces ordinary integer loads with their repeated-byte value. It needs
no benchmark name, fixed table length, expected index or checksum. It preserves
entry order and rejects entry EH regions; it never speculates another load.
The address can follow variable indices and copies, but a merge or unrecognized
use conservatively rejects the proof. The entry scan gates further analysis;
all eligible fills share a linear replacement walk. A prototype merging the
global-copy and fill address walks costs more on the frozen workload, so
fewer scans alone did not justify that refactor.

The private build passes all 50 workloads. An independent host oracle checks
98,304 cases per path over three fill bytes, four integer widths, mutations,
escaping callbacks, volatile reads, partial/conditional refills and observed
memset return values. GCC, Clang, both compilers and serialized replay agree
at every optimization level. Five paired blocks give 1.189× instruction speed
and 1.382× process CPU speed against the ninth compiler. Reference timing
calibration warns, so their apparent runtime advantage is not an acceptance
claim. The final port confirms 1.189× instruction speed and 1.350× CPU speed
on log parsing against the tenth compiler, with about 0.020% more frozen
`-O3` compile instructions. All 13 validation gates, inception and fresh
student export pass. Added raw negatives cover merges, before-fill reads,
partial writes, escapes, volatile/atomic accesses and unsupported load types.
The progress record retains an inconclusive template-regex timing concern;
its instructions are unchanged. Receipts:
`uniform-local-fill-v2/`, `uniform-local-fill-v2-oracle/` and
`uniform-local-fill-v2-frozen/`, `uniform-local-fill-v4*`,
`uniform-local-fill-ported/` and `uniform-fill-linear-scaling/`.
The first prototype's unchanged matcher and
failed build are retained separately; they do not support the new result.

## Brainfuck: protect the earlier win against layout sensitivity

The complete eighth-compiler comparison shows a new 12% loss against the
corrected seventh compiler, despite identical executed instruction counts.
Nine focused paired blocks reproduce 0.877× CPU/loop speed with 1.001× A/A
calibration. The ninth candidate retains it; its loop change does not address
this separate problem. The interpreter's apparent loss, in contrast, reduces
to 0.998× on a longer control and remains noisy.

The hot recursive visitor's optimized LowIR is identical before and after
the overwrite change. Its 480-byte native body also retains its instructions
and internal branch layout; code removed elsewhere moves its address. Median
branch misses rise from about 34.8 million to 60.8 million, while branches
and instructions stay constant. Last-branch records localize almost all misses
to the visitor's indirect switch dispatch, at function offset `0x89`.
This evidence points to branch prediction and address sensitivity, rather
than extra semantic work or a missed recursive edge.

A link-only padding sweep does not restore the loss. The validated general
remedy copies a short straight-line dispatch into its incoming jump.
This keeps the guard at its original site and permits a fallthrough into the
dispatch instead of branching to a shared site. Table data counts toward the
existing tail-growth bound. Calls, stores, side exits and landing pads are
excluded. The visitor shrinks from 480 to 420 bytes; index text also shrinks.

Five paired blocks give 1.428× CPU speed against the ninth compiler and
1.226× against the corrected seventh compiler. Median branch misses fall
from about 61 million to 8.5 million, with unchanged executed instructions.
Five independently linked code offsets retain a 20–26% gain against the
seventh compiler. The reference comparisons are 1.096× GCC and about 1.003×
Clang, with Clang calibration warnings. Public/Boost sentinel objects are
unchanged. Independent guarded and unguarded packet-dispatch oracles cover
32,768 cases per compilation path, including wide tags, zero trips, calls,
volatile reads and cleanup on throw. This is a general dispatch improvement;
no code or padding is selected by benchmark name.

The implementation also shares the loop-test and dispatch copying code and
uses amortized vector growth. A synthetic 32–1,024-switch family preserves
identical MIR and executable bytes; the largest case uses 47% fewer compile
instructions than reserving exact space for each copy. Frozen `-O3` compile
instructions rise about 0.008% against the ninth compiler, with stable RSS.
The final compiler passes full validation, PA35 release, inception and student
export. The complete scoreboard and longer controls are recorded in the
progress document; the earlier large apparent losses do not reproduce.
Receipts: `exact-address-store-after-slot-fix-brain-control/`,
`late-packed-invariants-ported-brain-control/` and
`exact-store-brain-regression/`, plus `dispatch-tail-duplication-v4/`,
`dispatch-tail-efficient-growth-v2/` and `dispatch-tail-final*`.

## Maze generation: preserve a counting predicate through canonicalization

`dig` is an iterative stack traversal; the separate finish-opening helper is
recursive. About 89% of samples are in `dig`. Its hot neighbor loop counts
three walkable cell kinds. GCC implements the count without a branch, using
an unsigned interval comparison and carry addition. Our optimized LowIR still
has a switch into an increment arm and a merge phi. The native code branches
around the increment. There is no variable divide in this region.

The first lost opportunity is before native selection: select formation
accepts conditional branches, while this predicate is represented as a switch
whose three cases share one destination. The backend already knows how to
lower a conditional increment with carry; adding another native arithmetic
idiom would miss the cause. The candidate is a typed interval predicate for a
contiguous case set, followed by the existing predication proof.

A private early-cleanup prototype passes all 50 workloads but fires too soon:
it converts a partial two-case switch before the final comparison joins it.
The switch former then sees different selectors and cannot combine the whole
three-kind predicate. Its output remains a branch chain, so it establishes
neither the intended simplification nor a native gain. A later private
prototype at select formation exposes the intended interval predicate and
existing conditional increment. The final version normalizes the switch
during that existing walk. Its unique successors do not change, so it can
reuse the CFG rather than rebuilding it after a separate scan. Singletons
retain their existing equality test. This is the twelfth mechanism.

The final compiler and private prototype emit identical index objects. All
50 checksums and direct/serialized identity checks pass. Independent source
oracles check 98,608 cases per path; raw oracles check 7,280,490, including
exhaustive narrow inputs, duplicates, full ranges, signed boundaries, holes,
cyclic sets and unsupported widths. Throwing callbacks, guarded loads and
division, cleanup, and increments that overflow only on an untaken path retain
their behavior. Existing predication checks still decide which work can move.

Five paired blocks give maze generation 1.046× instruction speed and 1.226×
CPU speed against the eleventh compiler. BFS and AStar improve only slightly;
retain their calibration warnings. Reusing the walk cuts the prototype's
frozen compile-instruction increment from about 0.026% to 0.005% at `-O3`.
Index text shrinks another 224 bytes. See the progress record for final
validation and the full-scoreboard controls.

A secondary gap is the explicit vector stack's cursor traffic. The 24-byte
object survives in optimized LowIR across reserve/reallocation calls, while
GCC keeps more cursor state in registers and spills it at the cold growth
call. The census has no allocation spills or new frame homes; its frame
accesses include the retained source object. Eliminating those reads needs a
memory/capture proof, not a larger register budget or recognition of a library
spelling. Test an independently varying growable buffer, escaping callbacks,
alias writes and unwinding before changing that policy. Receipts:
`remaining-workloads-profiles-dse/Maze__Generator/`,
`remaining-workloads-code-dse/Maze__Generator/`, `range-switch-v1/`,
`range-switch-v2/`, `range-switch-v3/`, `range-switch-v4/`,
`range-switch-v6-*`, and `range-switch-final/`. The first three experiments
are retained separately; their results do not replace the final proof.

## Interpreter and hash helpers: measured policy boundaries

A private diagnostic compiler records the existing inlining facts and actual
admission results without changing policy. Its complete native index object
is byte-identical to the final compiler, and all 50 checksums and serialized
replay checks pass. The logging code stays outside the repository.

The recursive interpreter's binary and assignment evaluators call a guarded
six-hot-instruction visitor wrapper. It is loop-free, has no EH region or
direct self call, and has one onward component edge. Its call sites are
outside loops, so the recursive-component gate rejects it before pricing;
the remaining budget is 1,536. The binary evaluator itself has two component
edges and must not be mistaken for a forwarding wrapper. The wrapper also
checks the variant's valueless state and can throw, so replacing it with an
unchecked call is invalid. This closes the rejection diagnosis, not the
optimization: an earlier broad non-loop policy cost 1.60% more frozen compile
instructions. A follow-up needs a bounded guarded-forwarder reduction with
unwinding and recursion tests, without reopening that global policy.

Template parsing retains two different boundaries. Its seven-hot-instruction
hash helper contains a terminate guard around the library hash call and is
rejected by the EH eligibility gate. Its node-search helper has 41 hot
instructions and is unhinted; at a loop site the late call-containing cap
is 24. The available budget is again 1,536. Raising the caller budget would
address neither rejection. A follow-up must prove a reusable exception-effect
fact or a profitable helper reduction, preserve collision and exception
behavior, and recheck frozen compiler cost. Receipts:
`inline-policy-diagnosis-final-v2/` and `inline-policy-diagnosis-final-v3/`.

## Other refreshed targets: evidence and remaining proof

All twelve named remaining targets have fresh profiles, O0/O3 host-object
LowIR, actual host MIR and allocator census excerpts in
`remaining-workloads-profiles-dse/`. Its manifests identify each input and
filter. Warm-symbol disassembly for the same cppgm binary and both stock
references is refreshed in `remaining-workloads-code-dse/manifest.json`.
CSV's namespace is `csvm`; the refreshed capture includes it for all three
compilers and preserves the earlier incomplete receipt.
These records establish the following boundaries, but do not yet close every
target's diagnosis.

| Target | Located work and typed/native evidence | Next proof before another transformation |
| --- | --- | --- |
| Calculator::Interpreter | Variant visits remain in separate helpers; the references fold visits into their evaluators. The private admission trace above proves the non-loop recursive-wrapper rejection before pricing. | A bounded guarded-forwarder reduction needs a cheaper design than the rejected broad policy. Preserve alternative selection and exceptional state. |
| Hash::SHA256 | Almost all samples are the byte hash's digest. The exact-address overwrite fix removes its redundant intermediate store and cuts instructions by 5.55%. | Explain Clang's remaining scalar-loop difference. Its four-byte SIMD path only applies to short inputs; disabling vectorization changes other code too, so that ablation does not prove long-input vectorization is missing. |
| Maze::Generator | About 89% of samples are iterative `dig`. The counting-predicate fix now removes 4.4% of process instructions. | The residual vector-cursor traffic needs memory/capture facts across growth calls; it does not call for recursion elimination. |
| Maze::BFS | About 76% is traversal; its body has 639 MIR instructions and seven frame homes, with no allocation spills. Queue movement and visited-state accesses remain visible in optimized LowIR. | Separate queue maintenance from neighbor/visited queries and attribute the repeated loads to calls versus aliasing. A smaller repeated native loss requires a calibrated control first. |
| Maze::AStar | About 79% is traversal: 651 MIR instructions, nine frame homes and no spills. Heap operations and integer distance arithmetic survive. | Compare heap updates and heuristic address arithmetic. Both references already use integer division; its presence alone proves no missing narrowing optimization. |
| Etc::CacheSimulation | Hash-table lookup/insertion and string work dominate; printf also has a material share. The run body has two allocation spills and eleven frame homes. | Map the hot table specializations and remaining calls to their effect/live-range constraints. Both cppgm and GCC use 64-bit variable remainder here; do not invent a missing 32-bit rule without a range proof. |
| CSV::Parse | Roughly 42% is strtod; parse_record takes about 17%. Its host MIR has 186 instructions and no spills or frame homes. | Compare the character scanner and vector/string-view boundaries. Preserve numeric conversion, locale and error semantics; runtime parsing costs cannot be deleted because this input succeeds. |
| Json::ParseDom | strtod, allocator and strncmp are substantial; generated parse_value is about 9%. Recursion, allocations and cleanup remain in optimized IR. | Isolate generated scanner/copy work from shared libc costs. Retain observable allocation failure and cleanup ordering. |
| Json::ParseMapping | Similar parser/libc mix to DOM; parse_value is about 9%. A smaller apparent slowdown also existed before the eighth change. | Attribute mapping versus common parser work and repeat the suspicious native case before changing policy. |
| Template::Parse | Hash-table helpers take about 41%; run is 435 MIR instructions with ten frame homes and zero spills. The trace above distinguishes EH eligibility from the late helper-size cap. | Prove a reusable exception-effect fact or bounded helper reduction, preserving exceptions, string lifetime and current Clang wins. |
| Distance::NGram | Hash lookup and insertion dominate. The main body has 260 MIR instructions and one frame home; separate table helpers remain. | Compare the lookup/insertion boundary, redundant key/hash work and call-preserved state. Use independently varied duplicate keys and collision patterns for a reducer. |

Base64 and Huffman remain in the queue as well. Base64's full-input decoder
is mostly inlined into cppgm's wrapper, while GCC retains a separate worker.
The actual GCC worker was captured; comparing only wrappers was misleading.
Narrow-shift, saved-register retry and local-address rematerialization probes
did not produce a resolved native gain and stay private. Huffman's indexed
field selection is accepted, but the Clang gap remains a pointer/bit-loop and
dependency-chain question. Use their retained IR/MIR and full-input profiles,
not a substituted decoding algorithm.
