# index-cpp optimization plan

Status: implementation in progress. See
[investigation records](index-cpp-optimization-progress.md) for evidence and remaining work.
This is a maintainer document and must remain outside the student export.

Improve generated code on the slowest index-cpp workloads while preserving
compiler throughput, existing workload performance, correctness and code size.
Implement general transformations justified by typed IR and measured behavior.
A workload name, source spelling or expected checksum must never select an
optimization. Constants may justify a rewrite only through a general rule
whose legality is proved independently of the benchmark.

## Baseline and scope

- Compiler: `60e73459e`, the C++14/17 compatibility branch.
- Unmodified upstream: `kostya/index-cpp` at
  `79e0b423094d420da37fb714f20c53fc35eeb2a3`.
- Measured host: Intel Xeon E5-2696 v4; GCC 15.2 and Clang 22.1.2;
  `-std=c++17 -O3`, stock reference flags, GCC final link for all objects.
- All 50 workloads pass. Equal-weight generated-code speed is 0.901x GCC
  and 0.885x Clang; higher is better. Reference repeat timings varied 5–7%,
  with larger individual variation, so these measurements prioritize discovery;
  a small claimed improvement needs fresh calibration and additional pairs.
- Actual executable `.text`: cppgm 387,154 bytes, GCC 280,921 and Clang
  263,041. Track code growth alongside instruction-cache and branch behavior.
- Local receipts: `/home/vishvananda/work/index-cpp/validation/20261105-comparison/`
  contains `observations.json`, `summary.json` and `per-benchmark.csv`.
  The sibling `20261105-baseline/frozen-instructions/` retains the frozen
  compiler A/A and ABBA instruction-count check.

Keep these inputs and binaries immutable. Record each candidate's source commit,
binary hashes, toolchains, source/header manifests and commands. Measure both
against this campaign baseline and the last accepted candidate to expose
cumulative regressions. Preserve GCC and Clang as separate scoreboards; the
faster reference is only a way to prioritize investigation.

The optional C++14/17 suite and upstream benchmark must not become student
requirements. Keep general C++11 reductions in their earliest owning assignment;
keep cases inherently requiring later standards in PA29/PA31's explicitly
optional `test-modern-cxx` lanes. Those fixtures and optional handout sections
ship to students, while the default course suites remain C++11. Reference CI
checks the optional lanes separately.

## Initial queue

Times are means of two full-workload passes, in the upstream timed sections.
The first group combines the largest relative gaps with recoverable runtime;
repeat focused measurements before treating its internal order as definitive.

| Priority | Workload | cppgm seconds | cppgm speed vs GCC / Clang | First questions to answer |
|---|---|---:|---:|---|
| 1 | Brainfuck::Recursion | 5.475 | 0.37x / 0.34x | Which visitor/dispatch calls survive? Are tape state and addresses repeatedly reloaded across calls? What prevents existing inlining or promotion? |
| 2 | Hash::CRC32 | 1.445 | 0.73x / 0.22x | What does Clang transform in the eight-step bit recurrence? Are branch simplification, bounded unrolling or bit facts missing or applied in the wrong order? |
| 3 | Calculator::Ast | 3.128 | 0.49x / 0.61x | How much time is parsing versus allocation, moves and destruction? Which temporary/aggregate, exception or character-range operations survive unnecessarily? |
| 4 | Base64::Decode | 2.470 | 0.60x / 0.64x | Are byte classification, table accesses, narrow conversions, bounds facts or output-pointer updates inefficient? |
| 5 | Compress::HuffDecode | 1.393 | 0.84x / 0.57x | Are pointer selection, bit-loop conditions, shared-pointer accessors or loop-carried state responsible? Separate decoding from tree construction. |
| 6 | CLBG::Nbody | 1.904 | 0.68x / 0.68x | Are repeated address calculations, loads, dependency chains or ordered SIMD opportunities responsible under the same floating-point rules? |

Calculator::Ast builds and replaces a parsed tree; visiting/evaluating it is
Calculator::Interpreter. Do not assume their hot paths are the same. Likewise,
Brainfuck's recursive visitor does not establish that tail-recursion elimination
is applicable: its loop work may have a continuation after a recursive call.

## 1. Establish a repeatable investigation loop

Adapt the existing measurement helpers into a small reference-only index-cpp
runner. Pin upstream, expose its existing single-workload CLI filter, retain
all observations, and parse checksum/status separately from variable timestamps
and timings. `TEST=1` is a fast correctness check, not the performance input.
Keep the original full-workload inputs, iteration counts and checksums unchanged.

For each target, collect matched release runs from cppgm, GCC and Clang:

1. A/A calibration, then paired ABBA/BAAB blocks on fixed CPU affinity.
   Measure upstream timed loops and process CPU/wall time separately; report
   per-program ratios and all-50 geometric means without dropping slow cases.
2. Hardware instructions, cycles, branches and branch misses. Add cache events
   in separate supported groups when needed; record counter coverage and avoid
   relying on multiplexed or load-contaminated observations.
3. Sample hot symbols and call chains. Count calls, spills/reloads and code
   bytes in the hot functions. Separate generated code from allocator, libc and
   standard-library costs. Use profiles to localize, not as timing evidence.
4. Capture unoptimized and optimized host-object LowIR, MIR, disassembly and
   existing `--stats` / `--stats-functions` output. For host-object LowIR use
   `--emit-lowir -std=c++17 -g0 ... -o file.lowir`; preserve the serialized
   boundary when replaying it through optimizer and backend tools.

Deliver one short diagnosis record per target: hot region, reference difference,
first phase that lost the opportunity, legality conditions, expected benefit,
compile-cost risk and proposed general reducer. Sampling alone is insufficient.

## 2. Prove the gap before implementing it

Compare the two reference compilers' optimization remarks and generated code.
Use reference-only ablations such as disabling vectorization or inlining to
identify the mechanism; retain stock reference builds as the scoreboard.

Classify each finding before writing a pass:

- Missing lowering facts: preserve an already-proved property in typed LowIR.
  Do not let the optimizer query frontend state or reparse source. A genuinely
  new serialized fact needs reader/writer/validator, contract and audit updates.
- Existing pass misses the shape: fix canonicalization, eligibility or analysis
  reuse in the owning pass. Show why the existing transformation did not fire.
- Existing pass fires too early/late: add a bounded, change-triggered cleanup
  at the dependency point rather than another whole-pipeline fixed-point loop.
- LowIR is already good: fix instruction selection, register allocation,
  address formation or branch layout in the backend.
- Reference gain depends on different ISA, floating-point permissions or
  runtime behavior: normalize the experiment and document the distinction.

The optimizer already has three inlining waves, tail-recursion handling,
rotation, unswitching, unrolling, loop-memory promotion and vectorization.
Do not treat a slow workload as evidence that the corresponding pass is absent.

## 3. Implement the first group, one proven mechanism at a time

**Brainfuck::Recursion:** inspect the small visitor and accessor call chains,
recursive call-graph components, immutable dispatch targets, aliasing through
visitor references, and loop-carried tape state. Candidate work includes bounded
wrapper inlining/dispatch simplification, promotion and call-effect precision.
Keep genuine recursive edges and observable mutation; do not remove recursion
or exception paths without a proof. Inspect rejection reasons before changing
inlining policy. Do not raise global inlining budgets to make this case pass.

**Hash::CRC32:** isolate the byte/bit recurrence with varied trip counts,
constants and input values. Test the general interaction of branch-to-select
formation, known bits, shifts/XORs and existing unrolling profitability. Any
idiom lowering must prove its semantics from typed operations and respect the
target ISA. Validate algebraic rules with varied polynomials and inputs;
never substitute a benchmark-specific table or precomputed answer.

**Calculator::Ast:** attribute parser, allocation and object-lifetime costs
separately. Investigate aggregate scalar replacement, copy/move forwarding,
character-range propagation and elimination of unnecessary cleanup work using
existing effect facts. Preserve allocation behavior where observable, throwing
operations, ownership transfer and destructor ordering. Never infer that a
library operation cannot throw merely because this input did not throw.

Likely owners are `dev/src/opt/inliner.cpp`, `interprocedural.cpp`,
`aggregate_slots.cpp`, `memory_forwarding.cpp`, `value_simplify.cpp`,
`cfg_cleanup.cpp`, `loop_passes.cpp` and the corresponding native selection /
allocation modules. Choose the owner from the diagnosis, not this list.

Each candidate needs a small correctness reducer, negative legality cases,
a non-benchmark example, a complexity bound and an independent measurement.
Test semantic results and broad quality bounds rather than requiring an exact
instruction sequence, register choice or source shape.
Keep unrelated transformations in separate commits. Preserve the small `-O0`
path; gate expensive analysis by level and cheap eligibility, keep analyses at
their natural unit, reuse scratch storage and invalidate only changed facts.

The first review point is after these three diagnoses and their justified
fixes: rerun all 50, publish remaining causes, and rerank before expanding scope.

## 4. Continue from the new ranking

Work through Base64 decoding, Huffman decoding and Nbody if they remain leading
gaps. Reuse proven scalar/loop improvements from the first group before adding
new mechanisms. Preserve strict floating-point behavior; do not use fast math
or reassociation to make the floating-point comparisons look better.

Then inspect the remaining parser, interpreter, SHA256, maze and container
workloads according to the refreshed profiles. Protect current wins as well
as losses. Track GCC and Clang comparisons even when one reference is unusually
fast or slow on a particular program.

## 5. Periodic checks against existing performance workloads

| Checkpoint | Required checks |
|---|---|
| Every candidate transformation | Focused correctness plus negative/varied-input reducers; target and one unrelated workload; frozen compiler `-O0` and `-O3` compile instructions and peak RSS against immutable baseline and previous accepted compiler. |
| Every accepted optimizer/backend commit | PA35 smoke, affected PA32/PA33 suites and kernel checks; `TEST=1` for all 50; paired focused target timings. Investigate repeatable native slowdowns rather than averaging them away. |
| Every 2–3 accepted changes, and after any inlining/loop/allocation policy change | `make -C pa35 quick`: build seed/self, check their frozen-workload instruction counts, identity, kernels and behavior. Run a preselected existing public-benchmark sentinel set covering recursion/dispatch, containers/JSON, regex, hashing and floating-point loops. Include known sensitive Boost container/Xpressive cases when those policies are affected. |
| End of each group and before a PR | Full existing public benchmark suite at `-O0`/`-O3` against GCC and Clang, all-50 index-cpp paired measurements, full PA35 gates, strict report, debug-info, self-host through PA5 and architecture/placement/format/file audits. Run inception for changes affecting self-host behavior; check export when shipped material changes. |

The frozen source/header epoch must validate before measurements. On Linux,
count the exact frozen compile command with `perf stat`; the existing
`scripts/run_frozen_compile_benchmark.sh` provides CPU/wall ABBA measurements,
not instruction counts. Preserve observations and verify deterministic output.
Changes to optimization can legitimately change object bytes: require A/A
repeatability, baseline/candidate behavior equality and direct-versus-serialized
path agreement; require seed/self byte identity where the existing gates do.

Use the existing performance helper's tolerances as investigation triggers:
0.5% compile instructions, 1% physical footprint and 3% peak RSS, checking
cumulative change too. Record which memory metric the host supports rather
than treating footprint and RSS as interchangeable. Remeasure effects near calibration
noise. Reduce analysis work or redesign a transform when compiler cost grows
without demonstrated native benefit. Preserve the existing PA35 text-size bar
and record index-cpp `.text` separately; reject indiscriminate code growth and
investigate every repeatable individual regression. Do not loosen gates or
change benchmark inputs to retain a candidate.

## Review and completion records

For each landed mechanism record baseline/candidate hashes, legality proof,
reducers, before/after IR and machine-code evidence, target runtime/counters,
frozen `-O0`/`-O3` cost, public sentinel results and text growth. Distinguish
confirmed missing transformations from rejected hypotheses and deferred work.
A group closes with a full-scoreboard comparison and a new ranked backlog;
compiler throughput and existing workload performance remain acceptance criteria
throughout, not checks deferred to the end.
