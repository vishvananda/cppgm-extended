# Scratch reference follow-up

This review starts from `e0c32d14`, after PR75's producer/comparison fixes and
PR76's two explicit language compatibility decisions. It evaluates the remaining
scratch reference changes against the solution compiler; it does not import a
student compiler implementation or its complete reference patch.

## Producer corrections

- Standalone posttoken output keeps maximal-munch user-defined string tokens
  after `operator`, including invalid reserved suffixes. The compiler dialect
  retains the split form needed by its literal-operator parser. PA2's existing
  fixture also distinguishes adjacent, underscore and separated suffixes.
- Constant packed bit-field initialization builds byte placements for shared,
  split and wide units, preserving ordinary neighboring members. It must not
  fall back to late dynamic stores when the value is a constant initializer.
- Anonymous-union injection retains union-derived address provenance. Such
  loads/stores must not acquire the `typed` aliasing fact.
- Preliminary class/union zero-initialization uses the contiguous storage span
  required by PA8, including padding; later scalar volatile operations remain
  scalar volatile operations.
- Constexpr-only constructor support does not become a mandatory object root.
  Actual runtime references and explicit instantiation demands still retain
  required code. Explicitly instantiated static data gets its required root.
- The injected `atexit` declaration states its nonthrowing boundary.
- A call after a logical-argument join reopens its cleanup coverage. A paused
  call marker does not mean the call is still protected after the join.
- Retirement of a class/unknown catch during unwinding guards exception-object
  destruction against a second throw. Scalar catches need no added guard.
- Assignment LowIR cleanup tails resume into the enclosing terminate guard.
  Native object lowering retains its direct-termination form: applying the
  assignment presentation convention there introduced avoidable O3 compiler
  work. Both forms preserve the same exception semantics.
- Lexical dispatches reuse the same complete-context entry and returned-result
  state. Cache hits restore temporarily removed contexts just as misses do;
  the nested-return control exercises both ordinary and throwing retirement.

All references are regenerated from the corrected compiler. No existing expected
success/failure or executable exit status is changed.

## Constant-expression coverage

PA16 now states that a static data-member initializer cannot evaluate an ordinary
member body before its enclosing class completes. A dedicated negative fixture
checks that early call. The original positive emission-demand fixture instead
calls constexpr members of an already complete factory class, preserving its
member-function, scalar/address/object, storage-demand and namespace-definition
assertions. Valid instantiated-template member calls remain supported and are
covered by existing PA16 and hosted integration fixtures.

## Comparison boundaries

The reachability rule now includes optional globals and their dependencies.
Strong, replaceable and object-root definitions, live bodies and their
dependencies remain required. An emitted initializer/body still compares in full.
Ordered references from paired global anchors select a bijective renaming for
construction tables; they do not waive data or instruction comparison.

Candidate unused declarations are pruned only after TLS normalization. Reference
declarations survive until presence/refinement is established. This preserves
the two valid cases scratch's early-pruning pass rejected: declaration-to-
definition refinement and direct TLS-load equivalence. Tests include missing
roots, changed live initializers, live dependencies and invalid table targets.
The 33 reduced audit controls pass with PR75's existing allowance for omission
of optional cycles; scratch's older expectation that cycles must remain is not
adopted.

## Decisions retained

The PA10 CWG2352 binding and PA19 CWG1315 SFINAE decisions remain as documented
in their handouts and fixture comments. The PA12 constructor additions are not
needed for the accepted inputs, and the defaulted-destructor case is preserved
rather than replaced by a nontrivial destructor. Neither change establishes a
new reference defect under the selected compatibility policy.

Standalone local-static execution remains single-threaded, now stated in PA16.
Host objects already use the host guard ABI. Merely adding ABI calls to the
standalone output would not make its bundled guard runtime concurrent.
Pure EH name changes and manual merges without an identical complete context
are not imported.

## Qualification

New executable controls distinguish constant initialization before dynamic
startup, cleanup after logical-argument joins, second-throw termination, and
shared return cleanup across nested handler contexts. The first three expose
failures of the frozen main compiler; GCC/Clang agree with the corrected
language outcomes. The separate constexpr rejection is independently rejected
by both hosts, while the repaired positive input is accepted.

Validation uses the full strict report, debug-info suites, self-host through
PA5, architecture/file/placement audits, harness tests, student-export validation
and PA35 correctness checks. Backend checks compare the four PA35 kernels with
GCC at scale 256, including their edge-input oracles and exact candidate-object
comparison. Compiler-work measurements use immutable baseline/candidate tools,
the manifest-verified frozen PA35 workload, pinned-core ABBA blocks with A/A
calibration and unscaled user-mode instruction/cycle counters.


Final compiler-work results use six ABBA/BAAB paired blocks per mode and three
A/A calibration blocks, retaining every observation. All counters ran without
multiplexing (100% enabled time). Changes below are relative to the frozen
baseline; intervals are 95% whole-block bootstrap intervals.

| Mode | Instructions | Cycles | User time | Wall time |
| --- | --- | --- | --- | --- |
| O0 | -0.019% [-0.020%, -0.017%] | -4.351% [-10.270%, +0.267%] | -4.445% [-10.283%, +0.072%] | -3.340% [-9.623%, +2.647%] |
| O3 | -0.072% [-0.074%, -0.070%] | +1.310% [-1.659%, +5.590%] | +1.174% [-1.647%, +5.558%] | +1.240% [-1.881%, +5.857%] |

The tiny instruction reductions establish no compiler-work regression on this
workload; timing intervals overlap zero, so they establish no wall-time speedup.
The initial unscoped presentation change increased O3 instructions by 1.26%;
isolated builds identify that source, and the final scoped implementation
removes that increase. These trial observations are retained, not discarded.

The four PA35 kernel objects are byte-identical before and after, with unchanged
GCC-relative work (approximately 1.0000x for distinct/alias accessors, 1.1917x
for lookup and 1.1693x for exceptions). Their scalar-oracle checks pass. The
complete compiler workload's objects can differ because these are correctness
changes; each binary's output is byte-stable throughout its measurements.

Baseline compiler SHA256: `20c2512b5f9e9a5f0900325b1768427daaa9f84798c56960bc670421af35ea8a`.
Candidate compiler SHA256: `02fcddbdb1df898b5e5a0ac6de2778ed1526f1ee43d8b1e1d91bf7130747c2af`.
Raw retained observations, counter files, variant builds and validation logs are
under `/tmp/cppgm-scratch-followup/` on the review host; `measure.py` records the
fixed flags, pinned CPU, output hashes and every sample in `perf/raw.jsonl`.
