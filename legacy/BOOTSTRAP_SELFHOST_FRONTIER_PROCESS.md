# Bootstrap Self-Host Frontier Process

This document defines the active workflow for bootstrap/self-host frontier work.

It is intentionally aligned with `HOSTED_HEADER_FRONTIER_PROCESS.md`. The same ownership,
regression-placement, validation, and checkpointing rules apply. The only substantive change is
how the next frontier item is discovered:

- hosted frontier discovery comes from the hosted-header sweep
- bootstrap frontier discovery comes from a serial self-compile probe

Active state for this phase lives in `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md`.

For bootstrap items that have already reached link stage, use the narrower
[BOOTSTRAP_SELFHOST_LINK_PROCESS.md](/Users/vishvananda/cppgm/legacy/BOOTSTRAP_SELFHOST_LINK_PROCESS.md)
as a supplement to this document.

## Goal

Turn self-host/bootstrap failures into:

- a concrete implementation fix in `dev/`
- a correctly owned assignment-local regression in the earliest real `paN`
- a durable tracker entry that records both the active bootstrap frontier and its closure

The current targeted bug queue lives in
[docs/bootstrap-frontier-active-bugs.md](/Users/vishvananda/cppgm/docs/bootstrap-frontier-active-bugs.md).

## Shared Rules

Unless this document says otherwise, follow the same process and decision rules from
`HOSTED_HEADER_FRONTIER_PROCESS.md`.

In particular:

- use `ROADMAP.md` and the owning `paN/README.md` to assign ownership
- use the build-first root runners for all `pa10+` direct probes:
  `make run-cppgm CPPGM_ARGS='...'` for one-off compiler invocations,
  `make bootstrap-frontier*` for bootstrap discovery,
  and only use `./dev/cppgm++` directly after an explicit fresh `make build` when a timed perf
  measurement must exclude make's no-op overhead
- the bootstrap discovery targets now default to compact inline diagnostics plus sidecar dumps:
  `BOOTSTRAP_DIAG_MODE=compact-sidecar`, `BOOTSTRAP_DIAG_MAX_STACK=8`,
  `BOOTSTRAP_DIAG_MAX_LINE=220`; override with `BOOTSTRAP_DIAG_MODE=full` when you explicitly
  need the legacy full inline output
- the underlying compiler mode is still the single `cppgm++` driver:
  `cppgm++ --emit-ast/--emit-types/--emit-semantics/--emit-lowir` for the earlier text modes,
  and plain `cppgm++ -E/-c/...` for the PA29-31 hosted/toolchain path
- move standard-language bugs backward into the earliest real owning milestone
- keep true bootstrap-only integration issues in `PA32`
- if reduction, regression placement, or ref analysis exposes a broader deferred LowIR cleanup
  or metadata gap that is not being implemented in the current fix, append a short note to
  [`docs/lowir-evolution-plan.md`](/Users/vishvananda/cppgm/docs/lowir-evolution-plan.md)
  before closing the bootstrap item
- keep hosted/bootstrap language mode aligned with the project target language surface unless the
  project target itself changes; do not "fix" a blocker by widening `__cplusplus`, host probe
  flags, or libc++ gating to a newer standard mode than the active target (currently C++11)
- add the real fix, not a bootstrap-only workaround or the smallest source edit that only dodges
  the current compiler-source failure
- reduce before fixing, and prove the reduction separates the last broken commit from the first
  fixed commit
- if reduction uncovers a smaller real owner bug underneath the current bootstrap blocker, record
  it as an explicit child item in a LIFO root-cause stack and close that child independently
  before resuming its parent
- treat async small-fix discovery notes as candidate imports only; before pulling one into the
  active bootstrap branch, revalidate its closure commit on current `HEAD`
- when a child fix or async import is isolated to a small set of files, prefer staged-child
  validation in the warm main checkout with `git add ...` plus
  `git stash push --keep-index -u` so the existing `obj/` tree stays warm
- fall back to the persistent clean intake worktree only when the staged path is unsafe or
  unreliable, such as overlapping files, index/stash failures, or a required pristine clean-HEAD
  proof
- validate and checkpoint each closed frontier item before exploring the next one

## Status Labels

- `open`: blocker identified, no fix yet
- `in-progress`: reduction or implementation is underway
- `completed`: fix landed, owning regression exists, and the tracker row is fully recorded

## Discovery

Use the fast bootstrap frontier probe for day-to-day frontier work:

```sh
make bootstrap-frontier
```

That runs:

```sh
python3 scripts/report_bootstrap_frontier.py --compiler ./dev/cppgm++ \
  --output-prefix /tmp/bootstrap-selfhost-frontier-fast \
  --resume-state /tmp/bootstrap-selfhost-frontier-fast.state.json
```

The fast probe now keeps its build artifacts under a deterministic directory:

- `/tmp/bootstrap-selfhost-frontier-fast.build`
- `/tmp/bootstrap-selfhost-frontier-fast.state.json`
- `/tmp/bootstrap-selfhost-frontier-fast.md`
- `/tmp/bootstrap-selfhost-frontier-fast.json`
- `/tmp/bootstrap-selfhost-frontier-fast.trace.txt`
- `/tmp/bootstrap-selfhost-frontier-fast.trace.details.txt`
- `/tmp/bootstrap-selfhost-frontier-fast.trace.json`

The fast mode trusts the already-passing prefix from its saved state, recompiles the current
frontier file until it passes, then keeps moving forward in source order until it hits the next
failure. That is the default inner-loop bootstrap discovery flow.

The saved fast state is only reusable while the host-built `./dev/cppgm++` and the tracked
bootstrap inputs are unchanged. If the compiler binary or those inputs changed since the last
fast probe, the report now resets the trusted prefix automatically, clears the saved build dir,
and records the reset reason in the markdown/json report. Treat a reset as expected after
bootstrap-related fixes, cherry-picks, or fresh branch syncs; it means the next fast probe is
starting from a clean prefix again instead of trusting stale objects.
Use the explicit cluster pass when you want to expose likely follow-on blockers:

```sh
make bootstrap-frontier-cluster
```

That runs a parallel compile discovery pass over the bootstrap source set and reports:

- the first failing source in source order
- the next likely failures
- grouped failure clusters when several files fail for the same normalized reason

The cluster pass now preserves its object batch at:

- `/tmp/bootstrap-selfhost-frontier-cluster.build`
- `/tmp/bootstrap-selfhost-frontier-cluster.trace.txt`
- `/tmp/bootstrap-selfhost-frontier-cluster.trace.details.txt`
- `/tmp/bootstrap-selfhost-frontier-cluster.trace.json`

For post-link runtime work, treat that preserved cluster build as the default fast-refresh base.
The new helper target:

```sh
make bootstrap-refresh BOOTSTRAP_REFRESH_CHANGED='dev/src/foo.cpp dev/src/bar.h'
```

uses the current host-build depfiles in `obj/release/.d` to map the changed file set back to the
affected bootstrap source closure, recompiles only those preserved objects in
`/tmp/bootstrap-selfhost-frontier-cluster.build`, then reruns the normal link/runtime probe with
`--skip-compile`. Its sidecar summary lands at:

- `/tmp/bootstrap-selfhost-frontier-cluster.refresh.md`
- `/tmp/bootstrap-selfhost-frontier-cluster.refresh.json`

Use `bootstrap-refresh` as the default inner loop once the active frontier has moved to:

- `link-failed`
- `self-compile-smoke-failed`
- `self-run-smoke-failed`

Escalate back to a full frontier compile when the helper flags a broad invalidation or when the
touched files are known hot paths, especially:

- `dev/src/callsem_output.h`
- `dev/src/callsemantic.cpp`
- `dev/src/lowirgensemantic.cpp`
- `dev/src/semantic_context.h`
- `dev/src/semantic_model.h`
- `dev/src/symbol_linkage.cpp`
- `dev/src/template_resolution.cpp`

Keep the true serial bootstrap probe in place for final self-hosting confirmation:

```sh
make bootstrap-frontier-serial
```

That runs:

```sh
python3 scripts/report_bootstrap_frontier.py --compiler ./dev/cppgm++ --jobs 1
```

The serial pass now preserves its object batch at:

- `/tmp/bootstrap-selfhost-frontier-serial.build`
- `/tmp/bootstrap-selfhost-frontier-serial.trace.txt`
- `/tmp/bootstrap-selfhost-frontier-serial.trace.details.txt`
- `/tmp/bootstrap-selfhost-frontier-serial.trace.json`

Those persisted `.trace.*` files are now reusable inputs as well as artifacts: if a later
link-failed frontier rerun produces the same frontier JSON and the trace-analysis script has not
changed, `report_bootstrap_frontier.py` reuses the existing sidecar instead of recomputing it.

The build-first `make bootstrap-frontier`, `make bootstrap-frontier-cluster`, and
`make bootstrap-frontier-serial` targets should be the default documented entry points. Use the
`*-nobuild` variants only when you intentionally want to reuse the already-built `dev/cppgm++`
from the immediately preceding step.

Bootstrap artifact cleanup is now manual. Use:

```sh
make bootstrap-clean
```

when you intentionally want to remove the saved frontier reports, state files, and preserved
`.build` directories plus persisted `.trace.*` analysis sidecars under `/tmp`.

The fast and serial probes are staged:

1. compile the bootstrap source set one translation unit at a time
2. stop at the first compile failure
3. if compile succeeds fully, host-link the self-built `cppgm++`
4. if link succeeds, use that self-built `cppgm++` to compile a tiny smoke program
5. if the smoke compiles, run it

The first failing stage is the active bootstrap frontier.

For any non-success frontier, the runner now also writes the full untrimmed output of the active
failing stage to:

- `<output-prefix>.<stage>.stdout.txt`

Use that sidecar first when the markdown summary only shows the truncated tail of a long compile,
link, or self-compile failure.

For crash frontiers, the runner now captures debugger sidecars automatically when the failing
stage exits on a signal:

- `<output-prefix>.<stage>.crash.txt` for the exact crashing command
- `<output-prefix>.self-binary-noargs.crash.txt` when a `self-compile-smoke` crash also reproduces
  by running the self-built compiler with no arguments

The markdown and JSON reports point at those sidecars directly, so the normal discovery loop no
longer requires a separate manual LLDB/GDB pass just to get the first backtrace.

## Trace-First Compile And Link Triage

Use the persisted frontier sidecars and `scripts/bootstrap_trace_report.py` before falling back
to ad hoc debugger or `nm` work. The trace tool is now the default first pass for focused
semantic-output and link-root-cause analysis.

For compile and late semantic/output failures, start with the already-written stderr sidecar when
you have one:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --trace-stderr /tmp/bootstrap-selfhost-frontier-fast.compile.stdout.txt \
  --focus '<symbol-or-fragment>' \
  --write-prefix /tmp/<tag>
```

`--trace-stderr` is the no-recompile path. Use it on the frontier `*.stdout.txt` sidecar, a saved
manual stderr capture, or a previously persisted raw trace file.

When you need a fresh focused rerun on one bootstrap source, prefer the preset-driven trace mode:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --trace-source dev/src/semantic_lookup.cpp \
  --preset output-lifecycle \
  --focus construct_at \
  --write-prefix /tmp/construct_at
```

Preset guidance:

- `template-lifecycle`: overload selection, template candidate selection, deduction, and
  instantiation flow
- `output-lifecycle`: semantic output requirement, export decisions, output audit, and late
  ownership checks
- `linkage`: symbol identity / linkage drift where you already know the compile passed but symbol
  naming or export shape is suspect
- `full-link-root-cause`: broad symbol-lifecycle tracing when the failure boundary is still mixed

When the compile succeeds but the failure is late semantic-output / LowIR closure, prefer the
pipeline view:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --pipeline-source dev/src/semantic_lookup.cpp \
  --focus construct_at \
  --write-prefix /tmp/construct_at_pipeline
```

Use that path for errors like:

- `lowir exported symbol missing semantic owner`
- symbol exported by requirement tracking but never emitted by semantic output
- output-audit mismatches that need one stitched lifecycle

For link failures, start from the preserved cluster build or the link stderr sidecar instead of
rebuilding a fresh frontier immediately:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --cluster /tmp/bootstrap-selfhost-frontier-cluster.json \
  --cluster-probe \
  --focus '<symbol-substring>' \
  --write-prefix /tmp/<tag>
```

If you only have the linker stderr text, use:

```sh
python3 scripts/bootstrap_trace_report.py \
  --repo-root . \
  --link-stdout /tmp/bootstrap-selfhost-frontier-fast.link.stdout.txt \
  --focus '<symbol-substring>' \
  --write-prefix /tmp/<tag>
```

`--cluster-probe` is the preferred link-debug path when the preserved cluster objects are current,
because it can diff consumer/provider objects directly and point at the likely missing export or
provider mismatch without recompiling the whole bootstrap set.

Do not use the cluster probe to replace the active frontier. Use it to discover likely next items
and failure families, but keep the active frontier anchored to the first failing serial or
resume-fast step.

The fast probe is intentionally optimistic and can miss newly introduced regressions in earlier
files, so use `make bootstrap-frontier-serial` or `make bootstrap-frontier-serial-nobuild` once
the fast frontier has been cleared and before treating the branch as fully self-hosting.

Use the explicit self-compile sweep when you need a source-set-wide map of what currently:

- compiles cleanly,
- times out under a fixed per-file limit, or
- fails quickly with a concrete diagnostic family.

Recommended command:

```sh
make bootstrap-self-sweep
```

That runs a bounded per-file self-compile over the bootstrap source set and writes:

- `/tmp/bootstrap-self-compile-sweep.md`
- `/tmp/bootstrap-self-compile-sweep.json`

Use the sweep in these cases:

1. after a broad semantic/template refactor that may have moved the slow frontier
2. when the serial frontier degrades into "stall here" and you need concrete derived work beyond
   the first stuck file
3. when you want to know whether the current branch is blocked by one hotspot family or by many
   independent correctness gaps

For one-file bootstrap blockers, especially when the default sweep budget is too short for slow
translation units such as `callsemantic.cpp`, prefer a no-timeout single-source sweep rerun that
reuses the same self-host object layout as the PA37 ladder:

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
  --output-prefix /tmp/bootstrap-<tag>
```

Use that command when you need a trustworthy answer to:

- does the active bootstrap blocker still fail when compiled alone?
- can the resulting `.o` be reused directly by the PA37 ladder without rebuilding?
- is the current issue a real compile failure rather than a short-budget timeout artifact?

The sweep does not replace the active serial frontier, but it is the preferred way to discover
post-stall derived items because it separates:

- `timeout`: likely algorithmic or compile-time scaling work
- `error`: likely correctness / compatibility work
- `ok`: files that no longer need direct attention

The default sweep timeout is intentionally short. Treat the default `30s` budget as a discovery
classifier only:

- `ok` means the file is comfortably below the current sweep budget
- `error` means the file failed quickly with a concrete diagnostic
- `timeout` means only "did not finish within the current classifier budget"

Do not treat a `30s` timeout as proof that the file is fundamentally stalled. Once the quick
error families have been cleared, rerun the timeout set with a larger budget such as `120s`
before deciding which files are true long-running hotspot items.

Recommended follow-up:

```sh
make bootstrap-self-sweep-nobuild \
  BOOTSTRAP_SELF_SWEEP_TIMEOUT_SEC=120 \
  BOOTSTRAP_SELF_SWEEP_JOBS=1
```

For active hotspot work, narrowing the sweep to the representative timeout set with a longer
budget is preferred over immediately widening the whole source-set sweep.

When the bootstrap targets report a hosted/STL-heavy failure, check the inline diagnostic first
and then follow the emitted `Diagnostic detail: /tmp/cppgm-diag-*` path for the full-fidelity
stack and raw message. The sidecar is the authoritative dump; the inline text is the compact
index for discovery.

For template/operator failures, pay close attention to this trace pattern from
`template-lifecycle` reports:

- `deduce-substitution-failed template=<name> reason=failed type template argument resolution:
  <qualified-local-type> [scope <declaring-scope>]`

When the failed type is a function-local or lambda-local type spelled with a qualified name from
another namespace, that usually means the bug is not overload ranking itself. The likely problem
is that local named-type overlay or actual-argument type-text resolution is happening in the
template's declaring/bound scope instead of a use scope that knows the local type.

Before reducing an STL-heavy failure of that shape, build a non-STL probe that preserves only the
relevant structure:

1. declare a wrapper template and free operator/function templates in one namespace
2. declare the caller and a function-local type in another namespace
3. instantiate `Wrapper<Local*>` or the equivalent concrete local type
4. trigger the same operator/call site directly

That reduction pattern is usually enough to decide whether the owner is the overload/template lane
rather than libc++-specific behavior.

## Derived Discovery Past A Stall

If the serial bootstrap probe does not surface a concrete failing stage and instead sits inside
one self-compile for an impractical amount of time, keep that serial stall as the active bootstrap
frontier, but allow timed direct self-host compile probes to continue fix-finding beyond it.

Recommended probe shape:

```sh
CPPGM_DIAG_MODE=compact-sidecar make run-cppgm \
  CPPGM_ARGS='-c -I dev/src -o /tmp/<stem>.o dev/src/<file>.cpp'
```

Run those probes under an external timeout so they either:

- fail quickly with a concrete language / hosted-compatibility blocker, or
- confirm that the file is still part of the long-running stall

Rules for these derived probes:

1. They do not replace the serial bootstrap frontier while the serial probe still only says
   "stall here".
2. Record them separately in `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md` as derived post-stall
   blockers or notes.
3. Use them to find owner-level fixes behind the stall, but rerun `make bootstrap-frontier`
   after every closed item.
4. Each derived item still needs the normal owner assignment, earliest real regression,
   `make verify-bootstrap-compile-fast CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`,
   and a fresh bootstrap rerun.
5. If the serial bootstrap probe later starts failing quickly on a concrete stage again,
   immediately update the tracker and treat that concrete serial failure as the active frontier.

Recommended discovery order past a stall:

1. rerun `make bootstrap-frontier-serial-nobuild` once to confirm the active item is still a real
   stall and not just a long compile that now finishes
2. run `make bootstrap-self-sweep-nobuild` to classify the whole source set into `ok`,
   `timeout`, and `error`
2a. once the repeated quick `error` families are understood, rerun the surviving timeout set with
    a larger timeout budget such as `120s` so you can distinguish "slow but finishing" from "real
    hotspot / stall"
3. group the `error` items by normalized signature and fix the broadest real owner bug first
4. only after the quick `error` families are exhausted, start deep hotspot work on the smallest
   `timeout` item that is still representative of the stall family

Do not jump straight into the first timed-out file if the sweep is also showing several repeated
quick failures with the same concrete diagnostic. Clear the correctness family first; it often
shrinks or reshapes the timeout frontier.

## Async Discovery Intake

Bootstrap work should periodically check `async-small-fix-discovery/` for relevant committed
closures from parallel discovery work.

Required moments to do that intake check:

1. after closing a bootstrap frontier item and before beginning deeper work on the next one
2. when the current serial bootstrap blocker has been sharply reduced but still needs deeper
   root-cause work
3. when a long-running serial stall suggests a recurring hosted/compiler-source family that may
   already have async prior art

Treat async notes as prior art and candidate imports only. Do not merge an async closure commit
just because its standalone note says it passed in its own worktree.

The default import gate should reuse the warm main checkout when the async closure can be isolated
cleanly with `git stash --keep-index`. Use the persistent clean intake worktree only when that
fast path is unsafe or unreliable.

The required import gate is:

1. apply the async closure on current `HEAD` in the main checkout, typically with
   `git cherry-pick -n <commit>`
2. stage only the imported files you want to validate
3. stash unrelated live work with `git stash push --keep-index -u`
4. run one incremental `make -C dev CXX=/usr/local/opt/llvm/bin/clang++`, or the note's required
   narrower `dev/` build target(s) if the note records them
5. rerun the note's direct repro or representative file(s)
6. rerun `make verify-bootstrap-compile-fast CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
7. rerun the current parent reduced repro or `make bootstrap-frontier-nobuild`, whichever is the
   live serial gate
8. commit the import if it passes, then restore the stashed live work

If the staged fast path is unsafe, repeat the same gate in the persistent clean intake worktree
instead of forcing it through the main checkout.

Do not rerun a separate owning assignment-local suite during intake unless you are using it as a
faster local iteration step before the full `test-report` gate.

Only pull the async closure into the active bootstrap branch if that current-`HEAD` revalidation
passes. If it fails, keep the note as useful context, but continue the live serial bootstrap
process from current `HEAD` rather than assuming the async closure is still valid.

## Reduction

Reduce the frontier only far enough to answer:

- what is the real language/toolchain/bootstrap feature failing?
- is it a standard-language bug, hosted compatibility issue, or bootstrap-only integration bug?
- what is the earliest assignment that should own the regression?

Typical bootstrap reductions:

- compile the single failing `dev/src/*.cpp` file directly with `cppgm++ -c`
- reduce the failing construct into `/tmp`
- if the failure is link-time, reduce the source set to the smallest set of objects that still
  reproduces the host-link failure
- if the failure is self-built-driver execution, reduce to the smallest source file the
  self-built `cppgm++` cannot compile or run correctly

For stall-driven items, keep two reductions if needed:

- a semantic or language reducer that proves the real owner and becomes the earliest `paN`
  regression, and
- a file-level or TU-level perf reproducer that preserves the compile-time blowup

It is common for the smallest semantic reducer to lose the algorithmic failure mode. That is not a
reason to skip the regression. Keep the small correctness reducer for ownership and a larger timed
reproducer for hotspot work.

## Telemetry-Guided Stall Triage

When the active item is a timeout or a long-running compile hotspot, use the built-in telemetry
before changing code. The goal is to prove whether the problem is:

- repeated template-instantiation / definition-upgrade work,
- repeated text/fragment lookup work that the caches are not collapsing,
- recursive closure/output growth, or
- a real unsupported-language/error path hidden under the long compile.

Start with the smallest source file that still preserves the timeout and rerun it with an external
timeout plus semantic counters:

```sh
CPPGM_SEMANTIC_STATS=1 CPPGM_SEMANTIC_CACHE_STATS=1 \
make run-cppgm-nobuild CPPGM_ARGS='-c -I dev/src -o /tmp/<stem>.o dev/src/<file>.cpp' \
  2> /tmp/<stem>.metrics.log
```

Those counters currently expose:

- `template-instantiation-requests`
- `template-definition-upgrades`
- `required-definition-requests`
- `required-definition-upgrades`
- per-cache `hits` / `misses`
- cache table sizes via `CPPGM_SEMANTIC_CACHE_STATS`

Use that first pass to answer:

1. Is template-instantiation work exploding?
2. Are required-definition upgrades churning repeatedly?
3. Are the same caches taking many misses with poor hit recovery?
4. Is cache size growing without reducing repeated work?

Then use the hotspot tracer to localize repeated queries or fragment reparsing:

```sh
CPPGM_SEMANTIC_HOTSPOT=1 \
CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY='*' \
CPPGM_SEMANTIC_HOTSPOT_DUMP_FRAGMENT='*' \
CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT=32 \
make run-cppgm-nobuild CPPGM_ARGS='-c -I dev/src -o /tmp/<stem>.o dev/src/<file>.cpp' \
  2> /tmp/<stem>.hotspot.log
```

Narrower hotspot tracing is preferred after the first broad dump:

- `CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY=<substring>`
- `CPPGM_SEMANTIC_HOTSPOT_TRACE_FRAGMENT=<substring>`
- `CPPGM_SEMANTIC_HOTSPOT_TRACE_NODE=<substring>`
- `CPPGM_SEMANTIC_HOTSPOT_TRACE_LIMIT=<N>`
- `CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT=<N>`

Interpretation rules:

1. Prefer algorithmic fixes over memoization-only fixes. If one query text, fragment text, or
   template binding dominates the hotspot dump, first ask why it is being reissued so many times.
2. Treat repeated `miss` growth in `expression_fragment`, `type_fragment`,
   `qualified_type_lookup`, or `dependent_type_resolution` as a likely canonicalization /
   lookup-shape bug, not just a missing cache entry.
3. Treat large `template-instantiation-requests` or `template-definition-upgrades` counts as a
   sign to audit instantiation ownership, explicit/implicit upgrade paths, and recursion guards
   before adding another cache.
4. Treat large `required-definition-upgrades` or output-closure churn as a sign that emission /
   requirement bookkeeping may be revisiting the same bindings too many times.
5. If telemetry shows many different queries with no single hotspot, prefer source-set splitting
   or source-order sweeps over micro-reducers; the problem may still be dominated by the TU
   preamble or broad semantic closure.

Record in the tracker which telemetry you used and the strongest signal it produced. For example:

- top hotspot query/fragment family
- dominant cache-miss category
- before/after template-instantiation counts
- before/after timeout classification from `make bootstrap-self-sweep-nobuild`

As with hosted frontier work, the reduction is not an owning regression unless it is checked on
both the last broken commit and the first fixed commit.

Once the reduction identifies the owner, implement the full owner-level feature or compatibility
surface. Do not stop at "the bootstrap source compiles now" if the real issue is a broader PA31
compatibility gap or an earlier standard-language feature that still needs proper coverage.

If the reduction instead exposes a smaller real owner bug underneath the current bootstrap item,
push that child item onto the tracker as the top of the current root-cause stack. Do not keep
multiple lower-layer fixes mixed together in one uncommitted batch. Finish the deepest child
item first, validate it, commit it, rerun the parent repro/frontier, and only then continue
unwinding back up.

## Ownership

Use the same ownership rules as the hosted frontier, with one added bootstrap-specific rule:

- if the failure is truly about the bootstrap orchestration itself rather than the language,
  semantics, backend contract, or hosted compatibility, it belongs to `PA32`

Examples of likely `PA32` ownership:

- bootstrap source-set ordering
- self-build driver flow
- link/run mismatches between host-built and self-built compiler binaries

Examples that should still move backward:

- parser failures on ordinary compiler source constructs
- semantic/template failures while compiling compiler sources
- backend/object emission mismatches visible during bootstrap compilation
- hosted standard-library regressions rediscovered during bootstrap

## Validation

Every closed bootstrap frontier item must run:

- the owning assignment-local regression suite
- `make verify-bootstrap-compile-fast CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- `make bootstrap-frontier-nobuild`
- one small compile-time sanity probe when the fix touches parser, semantic, overload,
  template, fragment-reparse, or other known hot-path code
- the default perf sanity probe below, or a narrower documented substitute when the active item
  already has one
- when the closed item was discovered from a timeout / stall family, either:
  - rerun `make bootstrap-self-sweep-nobuild` if the source-set classification changed, or
  - rerun the current timeout-cluster subset with the same timeout/telemetry settings used during
    reduction

Checkpoint-only validation must additionally run:

- `make verify-bootstrap-full CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- `make bootstrap-frontier-serial-nobuild CXX=/usr/local/opt/llvm/bin/clang++`

For stacked child items, also rerun the parent reduced repro or parent direct bootstrap probe
before resuming work upward, so the unwind is confirmed from the committed child fix.

`make bootstrap-frontier-nobuild` is now the fast resume probe. Use it as the normal inner-loop
bootstrap gate while clearing compile-frontier items. Use `make bootstrap-frontier-cluster-nobuild`
when you need likely next failures and failure-family clustering. Once the fast frontier runs
clean through all remaining files, run the full serial check with
`make bootstrap-frontier-serial-nobuild` before treating the branch as fully self-hosting.

If `make bootstrap-frontier-nobuild` advances, update the tracker immediately before exploring the next
item.

Default perf sanity probe:

```sh
make build CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
/usr/bin/time -p env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  ./dev/cppgm++ -c dev/src/template_audit.cpp -o /tmp/template_audit.perf.o
```

Rebuild immediately before the timed command so the binary is fresh without including make's
no-op overhead in the perf number.

Allowed substitute:

- the current reduced perf reproducer documented in an active hotspot note when that probe is
  narrower and more representative than `template_audit.cpp`

Rules:

- use the same `/usr/local/opt/llvm/bin/clang++` host toolchain as the active frontier work
- compare against the last known good baseline for that probe, not just one noisy run
- treat obvious multi-second jumps or roughly `15%+` regressions as validation failures unless
  the change is intentionally a temporary diagnostics/profiling step
- record the probe and before/after timing in the tracker row whenever the perf sanity check
  materially influenced the keep/reject decision
- when semantic telemetry drove the fix, record the before/after counter deltas or hotspot-family
  changes in the tracker row as well, not just the wall-clock timing

For isolated child items, prefer staging just the child files in the main checkout and stashing
the parent restack with `git stash push --keep-index -u` before this validation block. Reuse a
separate clean worktree only when file overlap, index failures, or proof requirements make the
staged path unsafe.

## Checkpointing

Once a bootstrap frontier item is fixed, validated, and recorded:

1. commit it
2. rerun `make bootstrap-frontier-nobuild`
3. record the next active item in `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md`
4. only then start work on the next blocker

This applies equally to deeper root-cause children. Commit each child item before returning to
its parent; do not accumulate several lower-layer fixes and only commit after the parent closes.
