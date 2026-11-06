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

A private experiment recognizes a complete constant-byte fill at entry,
rejects every other write, escape, volatile access or unsupported observation,
and replaces ordinary integer loads with their repeated-byte value. It needs
no benchmark name, fixed table length, expected index or checksum. It preserves
entry order and rejects entry EH regions; it never speculates another load.
The address can follow variable indices and copies, but a merge or unrecognized
use conservatively rejects the proof. A production version should share the
existing address/use walk, rather than adding another unconditional scan.

The private build passes all 50 workloads. An independent host oracle checks
98,304 cases per path over three fill bytes, four integer widths, mutations,
escaping callbacks, volatile reads, partial/conditional refills and observed
memset return values. GCC, Clang, both compilers and serialized replay agree
at every optimization level. Five paired blocks give 1.189× instruction speed
and 1.382× process CPU speed against the ninth compiler. Reference timing
calibration warns, so their apparent runtime advantage is not an acceptance
claim. Frozen `-O3` compile instructions increase about 0.016%; `-O0` objects
are unchanged. This remains private, with required fixtures, raw negative
cases and full production validation still to do. Receipts:
`uniform-local-fill-v2/`, `uniform-local-fill-v2-oracle/` and
`uniform-local-fill-v2-frozen/`. The first prototype's unchanged matcher and
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

A controlled link-only padding sweep is in progress. It changes no compiler
decision or measured workload and cannot become a shipping workaround. The
next optimizer/backend question is whether shared dispatch tails lose useful
predecessor context, and whether a bounded, general tail-duplication or switch
selection rule improves this across independent dispatch examples and code
layouts. Preserve recursive continuation, tape mutation, exceptions and code
size. Do not restore deleted stores or select padding by benchmark name.
Receipts: `exact-address-store-after-slot-fix-brain-control/`,
`late-packed-invariants-ported-brain-control/` and
`exact-store-brain-regression/`.

## Other refreshed targets: evidence and remaining proof

All twelve named remaining targets have fresh profiles, O0/O3 host-object
LowIR, actual host MIR and allocator census excerpts in
`remaining-workloads-profiles-dse/`. Its manifests identify each input and
filter. Reference warm-symbol disassembly is retained in
`remaining-workloads-profiles-v2/code-manifest.json`; CSV's namespace is
`csvm`, and its earlier empty disassembly capture needs replacement.
These records establish the following boundaries, but do not yet close every
target's diagnosis.

| Target | Located work and typed/native evidence | Next proof before another transformation |
| --- | --- | --- |
| Calculator::Interpreter | Variant visits remain in separate helpers; the references fold visits into their evaluators. The binary-op evaluator is 58 MIR instructions with no spills. This is distinct from AST construction. | Inspect the recursive component and concrete inlining rejection; reduce the wrapper/visitor boundary while preserving alternative selection and exceptional state. Avoid a global budget increase. |
| Hash::SHA256 | Almost all samples are the byte hash's digest. The exact-address overwrite fix removes its redundant intermediate store and cuts instructions by 5.55%. | Explain Clang's remaining scalar-loop difference. Its four-byte SIMD path only applies to short inputs; disabling vectorization changes other code too, so that ablation does not prove long-input vectorization is missing. |
| Maze::Generator | About 89% of samples are recursive `dig`: 104 MIR instructions, twelve frame loads, three frame stores and no spills or new frame homes. | Compare pointer/neighbor live ranges and branch selection against stock references. Recursion is semantically necessary; do not infer that more slot promotion is useful here. |
| Maze::BFS | About 76% is traversal; its body has 639 MIR instructions and seven frame homes, with no allocation spills. Queue movement and visited-state accesses remain visible in optimized LowIR. | Separate queue maintenance from neighbor/visited queries and attribute the repeated loads to calls versus aliasing. A smaller repeated native loss requires a calibrated control first. |
| Maze::AStar | About 79% is traversal: 651 MIR instructions, nine frame homes and no spills. Heap operations and integer distance arithmetic survive. | Compare heap updates and heuristic address arithmetic. Both references already use integer division; its presence alone proves no missing narrowing optimization. |
| Etc::CacheSimulation | Hash-table lookup/insertion and string work dominate; printf also has a material share. The run body has two allocation spills and eleven frame homes. | Map the hot table specializations and remaining calls to their effect/live-range constraints. Both cppgm and GCC use 64-bit variable remainder here; do not invent a missing 32-bit rule without a range proof. |
| CSV::Parse | Roughly 42% is strtod; parse_record takes about 17%. Its host MIR has 186 instructions and no spills or frame homes. | Compare the character scanner and vector/string-view boundaries. Preserve numeric conversion, locale and error semantics; runtime parsing costs cannot be deleted because this input succeeds. |
| Json::ParseDom | strtod, allocator and strncmp are substantial; generated parse_value is about 9%. Recursion, allocations and cleanup remain in optimized IR. | Isolate generated scanner/copy work from shared libc costs. Retain observable allocation failure and cleanup ordering. |
| Json::ParseMapping | Similar parser/libc mix to DOM; parse_value is about 9%. A smaller apparent slowdown also existed before the eighth change. | Attribute mapping versus common parser work and repeat the suspicious native case before changing policy. |
| Template::Parse | Hash-table helpers take about 41%; run is 435 MIR instructions with ten frame homes and zero spills. GCC folds more table work into run. | Inspect helper eligibility/effects and measured costs. More inlining is a hypothesis, not proof; preserve exceptions, string lifetime and current Clang wins. |
| Distance::NGram | Hash lookup and insertion dominate. The main body has 260 MIR instructions and one frame home; separate table helpers remain. | Compare the lookup/insertion boundary, redundant key/hash work and call-preserved state. Use independently varied duplicate keys and collision patterns for a reducer. |

Base64 and Huffman remain in the queue as well. Base64's full-input decoder
is mostly inlined into cppgm's wrapper, while GCC retains a separate worker.
The actual GCC worker was captured; comparing only wrappers was misleading.
Narrow-shift, saved-register retry and local-address rematerialization probes
did not produce a resolved native gain and stay private. Huffman's indexed
field selection is accepted, but the Clang gap remains a pointer/bit-loop and
dependency-chain question. Use their retained IR/MIR and full-input profiles,
not a substituted decoding algorithm.
