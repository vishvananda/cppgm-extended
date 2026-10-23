# CPPGM Programming Assignment 35: Optimizing the Rust compiler

Complete PA34, then optimize your Rust C++ compiler in two separate senses:
its time and memory compiling C++, and the quality of the machine code it
emits. PA35 adds no language feature or output format. Use the same immutable
C++ source bundle S accepted in PA34; never rewrite it to improve a ratio.

R is the actual Cargo release C++ compiler executable. B=host(S) and A=R(S)
are C++ implementations of the same algorithm, built with identical flags,
configuration, libraries, allocator and host-link recipe. A measures R's
code generation; the later fixed-point generations C and D do not. Cargo's
Rust build time is outside the throughput measurements.

| Gate family | Compiler being measured |
| --- | --- |
| Correctness, generated programs and kernels | R and its own backend |
| Compile time, RSS and scaling | R compiling the frozen C++ workload |
| Identity, codegen instructions/proxy and text | A relative to B |

B and A must emit identical workload objects because they execute the same
frozen algorithm. R's own generated objects may differ from B's. Required
language, ABI, IR facts, course output contracts and Rust quality remain
mandatory; valid optimization/register-allocation choices and internal work
counters may change. An exact port's completed PA34 proof remains a historical
milestone, not a byte-equality constraint on PA35 optimizations.

## Completion criterion

Build the release Rust tools, then run from the repository root:

```sh
make -C pa35 test REFERENCE_BUNDLE=/path/to/reference-source \
  COMPILER=/path/to/release/cppgm LOWIR2NATIVE=/path/to/release/lowir2native
```

All commands below use these same variables. The prepared run installs them
in `pa35/reference.mk`, so its ordinary `make -C pa35 test` is equivalent.
`REFERENCE_OUT` optionally selects the build store, default `pa35/obj/reference`.
The command must print `===== ALL TESTS PASSED SUCCESSFULLY! (17/17) =====`.
Final acceptance also requires PA34's complete bootstrap with the student
profile, earlier course/debug correctness and Rust quality/file audits.

## Frozen workload

`pa35/workload/semantic_overload.cpp` and its 51 project headers are fixed by
`MANIFEST.sha256`. They preprocess to roughly 100,000 lines with libstdc++.
The scripts refuse changed files. Compare only on one host/toolchain; its
headers and libraries affect the work. All compilers use the same representable
long-double macro replacements because Valgrind emulates x87 with doubles.
If R rejects or miscompiles the input, reduce it under `student.tests/` and
fix R before measuring. Keep the full S build for bootstrap/final acceptance
and this workload plus the existing kernels for the optimization inner loop.

## Gates

The groups run in order: correctness, compiler speed, generated code. Failure
stops later groups, which remain unmeasured. `test-correctness`, `test-speed`
and `test-codegen` run an individual group. All 17 published limits apply.

| Group | Gate | Measurement | Limit |
|---|---|---|---|
| correctness | `workload-compiles` | R compiles the workload at `-O0` and `-O3` | required |
| correctness | `behavior` | each `tests/behavior/*.cpp`, compiled by R at `-O0` to `-O3` and linked by the host, prints what the host build prints; its `-O3` LowIR lowers through `lowir2native` with a MIR dump | all pass |
| correctness | `generated` | seeded programs exercising narrow values, Booleans, returned objects, frames and unwinding, at `-O0` to `-O3`, against the host build | all pass |
| speed | `compile-O0-time` | R `-O0` user time on the workload / host `-O0` | <= 0.50 |
| speed | `compile-O3-time` | R `-O3` user time / host `-O3` | <= 0.50 |
| speed | `compile-O3-memory` | R `-O3` peak RSS / host `-O3` | <= 1.00 |
| speed | `compile-scaling` | largest growth of R's compile time per doubling of input size, over the synthetic families of `scaling.pl` | <= 3.0 |
| codegen | `workload-identical` | B and A write byte-identical objects for the workload at `-O0` and `-O3` | required |
| codegen | `codegen-O0-instructions` | A / B instructions executed in the compiler binary's own code (callgrind), workload at `-O0` | <= 1.08 |
| codegen | `codegen-O3-instructions` | the same at `-O3` | <= 1.12 |
| codegen | `codegen-O0-cycle-proxy` | A / B cycle proxy for the whole process at `-O0` | <= 1.09 |
| codegen | `codegen-O3-cycle-proxy` | the same at `-O3` | <= 1.10 |
| codegen | `text-size` | A / B `.text` bytes | <= 1.15 |
| codegen | `kernel-*` (4) | each kernel in `kernels/`, compiled by R and by GCC and linked against one GCC-built driver: instructions for the measured input relative to GCC | <= 1.30 each |

The cycle proxy is `Ir/3 + 15 Bcm + 20 Bim + 10 I1mr + 10 (D1mr + D1mw)`.
Callgrind counts instructions in each executable's own code for the instruction
gates and whole-process events for the proxy. Hardware perf counts are
whole-process diagnostics; do not apply the own-binary limits to them.
The four kernels cover accessor chains (distinct/alias pointers), hash lookup
and exception-bearing loops. Scaling families cover scopes, members,
templates, macros, long functions and many functions: linear growth is about
2 per doubling, quadratic about 4.

`test-progress` reports saved rung counts without measuring. Rungs are the
limit times `(1 + step)^k`; steps follow measurement noise (0.5% for instruction
and simulation counts, 1% for kernels/text, 2% for memory, 5% for timing).
Receipts bind each group to current binaries, backend, S, workload, harness,
paths and toolchain. Missing, stale, failed or skipped observations cannot
pass. Rechecking unchanged correctness does not invalidate valid later groups.
The compile-time gates require at most half the host's user time at matching
optimization levels. Use gates.txt for acceptance; do not adapt limits to the
candidate or infer current results from historical measurements.

## Work loop

Work in order: workload correctness, bounded speed/scaling, generated code,
then speed-only changes preserving output. Run focused correctness and two
useful scales for a demonstrated mechanism; do not expand benchmark matrices
or repeatedly requalify unchanged inputs and host baselines.

```sh
make -C pa35 test-smoke                  # correctness with five generated seeds
make -C pa35 test-speed                  # compiler throughput, RSS and scaling
make -C pa35 quick LEVELS=O3             # cached hardware counts when available
make -C pa35 test-codegen                # expensive simulation and kernel gates
make -C pa35 test                        # final 17-gate acceptance
make -C pa35 test-progress               # reads current saved group receipts
```

Use the configured reference variables for each command. `quick` builds B/A
only when their complete cached inventories/dependencies are invalid, counts
the selected workload levels and checks kernels/behavior with R. Hardware
instructions are counted at native speed when available; callgrind is the
fallback. Full simulation belongs to final acceptance or a stated unresolved
question. Missing candidate `lowir2native` is an error, not a skipped MIR check.

Retain the old R before a codegen change. This builds the same S with old/new
compilers and compares the resulting executables on constant work:

```sh
perl pa35/scripts/ab.pl /path/to/old-R /path/to/new-R \
  --reference-bundle /path/to/reference-source --counter auto
```

`make -C pa35 freeze LABEL=before` preserves a host/candidate-built pair;
`measure LABEL=before SIM=1` measures it, and `abba LABEL=before` provides
paired timing when a claim needs confirmation. Use a binary's comparison
with itself to calibrate noise. Do not multiply timing campaigns by default.
For compiler-speed changes compare R's outputs directly on fixed supported
inputs; codegen attribution always uses the same frozen S. Keep every failure
and raw observation. Avoid full S rebuilds for unrelated evidence updates.

`speed.pl --compiler R` compares matching O0/O0 and O3/O3 levels.
`scaling.pl --compiler R` and bounded
`profile.pl --compiler R` diagnose the Rust compiler itself. Select the
smallest input exposing repeated work. Commands are time/memory bounded;
reference builds use at most eight jobs. Set `PA35_JOBS` lower on a shared host.

## Correctness and boundaries

Preserve earlier PA contracts and required debug/MIR/ABI facts. Do not
recognize benchmark filenames, symbols or source spellings. No profile
feedback, LTO, whole-program optimization or new ISA extensions: compare
translation-unit `-O3` builds with the same ordinary allocator and host link.
Never copy reference compiler logic into Rust or delegate a production phase.

Run reduced behavior controls after transformations and the supplied generated
program suite after allocator/MIR changes (`generated.pl --compiler R --seeds
2000`). Refresh earlier correctness and the bootstrap after coherent fixes;
complete the full proof and gates on final code. A fixed point alone cannot
rule out wrong code, so retain all rebuilt suites and Rust audits.

## Design notes (non-normative)

The earlier assignments' design notes describe the structures this work
stands on: an in-memory LowIR that a function can edit in place, with every
fact spelled and dense numbering (PA8), and the facts the lowering states
(`lowir.md`, "Facts The C++ Lowering States"); an exception shape with
regions closed before joins (PA21); a machine IR whose instructions state the
registers they define, use and clobber, with placement decided apart from
selection and the frame finished last (PA24); unwind tables generated from
the frame facts the encoder records (PA26); an optimizer that iterates per
function and keeps its own work bounded (PA32); and a frequency-weighted
allocator (PA33). Where one of those is missing, building it is usually the
first and largest step.

Measure both workload levels from the start: `-O0` mostly exercises the front
end and instruction selection, `-O3` the optimizer's own code, and the two
gaps can differ widely. Use instruction counts to iterate and paired timing
to confirm; frame homes, taken branches and store-forwarding stalls cost more
time than their instruction counts suggest, while register moves and
well-predicted branches cost almost nothing. Small counted wins add up even
when no single one is visible in time.

Attribute before changing anything. The excess by instruction class usually
names the mechanism: frame loads and stores point at allocation, callee-saved
saves at callee-saved policy and shrink-wrapping, register moves at
coalescing, jumps and compares at block placement and switch lowering,
extensions at narrow-value handling, calls at inlining. Compare functions one
by one only when their call counts match in B and A; otherwise inlining
has moved cost between them.

Incremental work depends on a few foundations. Check them first:

- register assignment separate from instruction selection, over a machine IR
  that records each instruction's defs, uses, clobbers and tied operands, so
  allocation and later cleanup passes can work on the whole function;
- an optimizer that can rewrite a function in place and iterate, so
  simplification, promotion and inlining feed each other;
- no blanket exclusions such as "no dataflow for functions with a call inside
  a loop" or "no promotion in functions with cleanups": they exclude most hot
  code, and usually hide a weakness in a later stage that should be fixed
  where it lives;
- a correctness net as strong as the changes: reduced tests for each new
  transformation, and many generated seeds after allocator or machine-IR
  changes (`generated.pl --seeds 2000`).

Useful directions, in the order the measurements usually point:

- allocation weighted by estimated block frequency rather than loop depth,
  with coalescing, splitting around loops and clobbering blocks,
  rematerialization, callee-saved costs per function entry, shrink-wrapping,
  and allocation inside functions that handle exceptions;
- selection that fuses compares with branches, folds addresses and loads
  into memory operands, keeps narrow values unextended where possible, and
  expands small copies, switches and constant divisions well;
- inlining decided on simplified callees, bottom-up, without cascades: raising
  a global size cap makes callees grow past their own callers' caps;
- block placement, loop rotation and jump threading;
- alias facts, scalar replacement of small aggregates, and copies that do not
  reload just-stored narrow fields as wider words.

Prefer fixing an over-conservative model (a constraint stricter than emission
needs) to tuning heuristics, which usually nets little. Re-measure a dropped
idea after related changes land. PA32 and PA33's fixtures require specific
outcomes and upper bounds. If a broad change breaks one, investigate the
extra work or lost simplification before adjusting the change.

Compile time that grows faster than its input usually comes from per-query
work proportional to the whole translation unit: a lookup, overload check or
instantiation that scans every scope, declaration or specialization, or
allocates a structure sized to them; or from repeated work: re-parsing text,
re-instantiating, re-checking, or lowering and emitting definitions nothing
uses. A front end that is uniformly slow usually pays for its representation:
per-character lexing with per-character records, names compared and hashed as
strings, a hash lookup on every syntax-tree edge, facts kept in side tables
instead of on the nodes, fat value types copied around, and allocation per
node. Those need representation changes, not tuning. In the optimizer, look
for passes that report changes they did not make, analyses recomputed for
unchanged functions and structures rebuilt after local edits. Keep
compiler-speed changes free of any effect on generated code, and check that
with `make -C pa35 identity`, so speed and codegen effects stay separable.
