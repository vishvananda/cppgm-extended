# PA37 Ladder Fix Process

This document defines the active maintainer workflow for the current PA37 fix
phase after the base scaffold landed.

It is intentionally similar to the bootstrap frontier process, but the active
frontier here is the public `pa1` through `pa9` PA37 self-host ladder rather
than the full compiler-source bootstrap set.

Use this process to close the current PA37 ladder issues without turning PA37
into a dumping ground for unrelated large reproductions.

## Goal

Turn each PA37 ladder failure into:

- a concrete implementation fix in `dev/`
- a minimal durable regression in the earliest real owning `paN`
- a locally backtested ladder step
- a coherent checkpointed fix batch before moving farther up the ladder

## Scope

This process is for the current early self-host ladder:

- `pptoken-self`
- `posttoken-self`
- `ctrlexpr-self`
- `macro-self`
- `preproc-self`
- `recog-self`
- `nsdecl-self`
- `nsinit-self`
- `cy86-self`

The active public ladder is ordered strictly from `pa1` to `pa9`.

The full self-built `cppgm++` bootstrap and broader bootstrap frontier remain a
later PA37 phase. Use
[BOOTSTRAP_SELFHOST_FRONTIER_PROCESS.md](/Users/vishvananda/cppgm/legacy/BOOTSTRAP_SELFHOST_FRONTIER_PROCESS.md)
for that broader workflow.

## Core Rules

- `ROADMAP.md` and the owning `paN/README.md` define assignment ownership.
- Start from the earliest ladder checkpoint that is not yet known green.
- Stop at the first failing checkpoint.
- Do not keep scanning later checkpoints while the current failure is still
  unreduced.
- Move every real language/lowering/runtime bug backward to the earliest owning
  `paN`.
- The durable regression is for the compiler bug that the ladder exposed, not
  for the ladder rung itself.
- The owning `paN` is chosen by the feature surface of the bug, not by the
  first `test-through-paN` target that happened to expose it.
- Keep PA37 integration proofs small. PA37 should prove the ladder boundary,
  not own every earlier bug permanently.
- The full regression gate is the batch commit gate, not necessarily the gate
  for every single small local fix.

## Command Surface

Use the `pa37` wrapper directly.

Build or test one checkpoint:

```sh
make -C pa37 pptoken-self CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
make -C pa37 test-pptoken CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
```

Expand the ladder through a checkpoint:

```sh
make -C pa37 through-pa1 CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
make -C pa37 test-through-pa1 CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
make -C pa37 test-through-pa2 CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
...
make -C pa37 test-through-pa9 CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
```

The PA-named `test-through-paN` targets are the default commands for this phase
because they match the ownership/frontier model directly. The checkpoint-named
aliases still exist, but the PA-named surface should be preferred.

Use a host-control comparison when needed:

```sh
make -C pa37 test-through-pa5 \
  CXX=/path/to/host-cxx CPPGM_HOST_CXX=/path/to/host-cxx
```

If the host-control ladder passes and the self-host ladder fails, the bug is in
our compiler output rather than in the PA37 wrapper or assignment harness.

The PA37 wrapper now defaults to separate object roots for the two modes:

- host-control builds use `../obj/pa37/host/...`
- self-host builds use `../obj/pa37/selfhost/...`

Do not intentionally share the same object root between the two modes during
this phase, or the comparison stops being trustworthy.

## Default Workflow

1. Pick the active ladder frontier.
   Usually this is the earliest `test-through-paN` target not yet known green.
2. Run the self-host ladder through that checkpoint.
3. If it fails, confirm the same `test-through-paN` target passes under the host
   control compiler.
4. Debug the first real failing test, compile, link, or runtime issue.
5. Reduce it to the smallest useful reproducer.
6. Identify the earliest real owning `paN`.
7. Validate that new regression against a clean pre-fix tree when practical.
8. Add the durable regression there immediately.
9. Implement the fix in `dev/`.
10. Backtest locally.
11. Continue only after the active ladder step is green again.

## Trace-Aided Compile And Link Triage

Use the trace tools early, after the first honest reduction, instead of waiting until the failure
has already turned into a long manual `lldb`, `nm`, or object-diff session.

For reduced compile or late semantic-output failures, rerun the reduced source with a focused
trace preset:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --trace-source /tmp/reduced.cpp \
  --preset template-lifecycle \
  --focus '<symbol-or-template>' \
  --write-prefix /tmp/<tag>
```

Switch presets according to the question:

- `template-lifecycle` when the issue is overload ranking, deduction, or instantiation choice
- `output-lifecycle` when the issue is why a function, variable, or template instantiation was or
  was not emitted
- `full-link-root-cause` when the boundary is still mixed and you need broader symbol-lifecycle
  context

If the ladder or reduced test already produced stderr, reuse it directly:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --trace-stderr /tmp/<failing>.stderr \
  --focus '<symbol-or-template>' \
  --write-prefix /tmp/<tag>
```

That raw mode is the preferred no-recompile path when you already have a saved compile or trace
failure from a prior run.

If the self-host ladder has already warmed `obj/pa37/selfhost`, reuse that same object tree when
you need to classify a single compile blocker without rebuilding an unrelated object set:

```sh
python3 scripts/report_self_compile_sweep.py \
  --repo-root . \
  --compiler ./dev/cppgm++ \
  --host-cxx /usr/local/opt/llvm/bin/clang++ \
  --frontend cppgm++ \
  --object-layout pa37-selfhost \
  --build-dir ./obj/pa37/selfhost \
  --test-runner 1 \
  --timeout-sec 0 \
  --source dev/src/<failing>.cpp \
  --output-prefix /tmp/pa37-<tag>
```

Use that command when the normal ladder rung has already identified the failing source and you
want a no-timeout answer that keeps the resulting `.o` reusable for the next ladder step.

For reduced `PA31`-style link or runtime-contract failures, use the link-root-cause mode before
hand-comparing many object files:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --link-stdout /tmp/<failing-link>.stderr \
  --focus '<symbol-substring>' \
  --write-prefix /tmp/<tag>
```

If you have already reduced to explicit consumer/provider objects, use the same tool with
`--consumer-object` and `--provider-object` so the object diff stays scripted and reproducible
instead of becoming a one-off manual `nm` session.

For late semantic-output or ownership failures exposed by a ladder compile, prefer the pipeline
view:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --pipeline-source /tmp/reduced.cpp \
  --focus '<symbol-substring>' \
  --write-prefix /tmp/<tag>
```

Use that path when the failure shape is:

- symbol required for output but not emitted
- exported symbol missing semantic owner
- semantic output and LowIR disagree about what survived closure

For template/operator failures, treat this trace signature as a strong hint that the real problem
is scope/local-type handling rather than candidate ranking:

- `deduce-substitution-failed template=<name> reason=failed type template argument resolution:
  <qualified-local-type> [scope <declaring-scope>]`

That pattern usually means a function-local or lambda-local type was reparsed or matched in the
template's declaring scope instead of a use scope that actually knows the local type.

## Reduction And Ownership

When a ladder failure is found:

1. reduce the failure to the smallest source or object/runtime reproducer that
   still demonstrates the bug
2. classify whether the real failure is:
   - compile-time
   - link-time
   - runtime
3. place the durable regression in the earliest real owning `paN`

Examples:

- preprocessing/tokenization bugs belong in `pa1` through `pa5`
- parser / semantic / lowering ownership should move into the earliest real
  owning milestone from `PA10+` according to `ROADMAP.md`
- host object/link/runtime interop belongs in `PA31`
- hosted-header and vendor-compatibility belongs in `PA33`
- only true ladder-only integration proofs should remain in `pa37`

The ladder rung that exposed the bug is only a discovery frontier. It is not
automatically the right home for the permanent regression.

Examples:

- a bug first exposed by `test-through-pa4` may still belong in `pa31` if the
  reduced reproducer is a host object/link/runtime interop failure
- a bug first exposed while self-hosting `macro-self` may belong in `pa33` if
  the reduced reproducer is a hosted compile-compatibility failure, or in
  `pa34` if the hosted code only breaks after emitted definitions have to link
  and run
- a bug first exposed while self-hosting an early ladder binary may belong in
  some later semantic or lowering milestone from `PA10+` if that is where the
  roadmap assigns the underlying feature
- a bug first exposed by a later ladder rung may still belong in `pa2` if the
  real failure is floating literal decoding

If the first reproducer is large, keep reducing until the ownership is clear.
Do not leave a giant PA37-only proof as the main regression unless there is no
smaller honest owner yet.

For bootstrap or ladder failures that first appear inside libc++ or another hosted library, prefer
a non-STL semantic reduction before assuming the owner is hosted-header compatibility. A useful
shape for template/operator issues is:

1. put the wrapper template and free operator/function templates in one namespace
2. put the caller function in another namespace
3. use a function-local or lambda-local type as the concrete template argument
4. preserve only the comparison or call that still fails

That pattern strips away vendor-library noise while keeping the exact local-type/scope interaction
that often determines the real owner. For the recent iterator-style operator failure, that smaller
shape moved the durable regression into `pa18` instead of leaving a large PA37-only bootstrap
proof.

## Local Backtesting

Every fix should be backtested before the next investigation step.

Minimum required local backtesting:

1. rerun the new reduced owner regression
2. rerun the owning assignment suite
3. rerun `make -C pa37 test-through-paN ...` for the active ladder rung

Before committing a new owner regression, also prefer validating it in a clean
head worktree or other pre-fix tree so the regression is known to fail without
the fix. This should follow the same discipline as the broader bootstrap
frontier process.

That third step is the main “did this actually unblock the ladder?” proof.

When a fix touches shared early-front-end code, prefer using the `test-through-paN`
target rather than only the single failing checkpoint so earlier cleared rungs
stay covered.

## Batch And Checkpoint Rules

Full regression is expensive, so this phase may use small coherent fix batches.

Allowed batching model:

- land a few related fixes in one working batch
- each fix in that batch must already have its minimal owner regression in the
  worktree
- each fix must already pass the local backtesting rules above

Recommended batch size:

- `2` to `5` closely related fixes

Do not let a batch sprawl across unrelated failure families.

Run the full regression gate before:

- committing the batch
- moving to a later ladder checkpoint after clearing the current one
- switching to a substantially different failure family

In other words:

- local backtesting is the per-fix gate
- full regression is the batch commit gate

## Suggested Regression Cadence

Use this cadence by default:

1. repeat the local reduction/fix/backtest loop for a small related batch
2. once that batch clears the active ladder frontier, run the broader gate:

```sh
make test-report CXX=/path/to/host-cxx CPPGM_HOST_CXX=/path/to/host-cxx
make -C pa37 test-through-paN \
  CXX=../dev/cppgm++ CPPGM_HOST_CXX=/path/to/host-cxx
```

3. commit the fix batch
4. resume at the next ladder checkpoint

If the batch touched especially broad files or the driver/build surface, run
the broader gate earlier rather than letting the batch grow.

## Escalation

Stay in this PA37 ladder process while the failure is still honestly exposed by
the `pa1` through `pa9` checkpoints.

Escalate to the broader bootstrap process when:

- the current ladder is green through `cy86`
- the remaining failure only appears while building full `cppgm++`
- the issue depends on later bootstrap stages that the early ladder cannot
  express honestly

## Completion Condition For This Phase

This phase is complete when:

- the full `pa1` through `pa9` PA37 ladder is green through
  `make -C pa37 test-through-pa9 ...`
- the discovered real bugs have been moved backward to their earliest owners
- the current fix batch has passed the full regression gate and been committed
