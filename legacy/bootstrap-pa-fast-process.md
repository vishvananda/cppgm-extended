# Bootstrap PA Fast Process

This is the quick bootstrap filter for `pa1` through `pa9`.

Use it when the full self-host frontier is too expensive for each iteration and we want a smaller host-linkable binary that is still compiled by `./dev/cppgm++`.

This process is for discovering compiler bugs cheaply, not for replacing the normal regression and commit discipline.

It is also a maintainer workflow, not the final public PA38 build contract.
The maintained frontend source lists now live in
[dev/frontend_source_sets.mk](/Users/vishvananda/cppgm/dev/frontend_source_sets.mk),
and the early-PA wrapper `Makefile`s use those same checked-in lists directly.
This helper remains a seed/debugging aid for self-compiled closure checks.

## Goal

For each implemented early PA frontend:

1. build the minimal fast-link closure for that frontend with the current host-built `./dev/cppgm++`
2. host-link the resulting object set into a runnable binary
3. run that PA's existing test suites against the self-compiled binary

If the self-compiled fast binary crashes or fails tests while the host-built fast binary does not, the issue is in our compiler output, not in the assignment harness.

The success condition is not "find a pile of failures quickly." The success condition is:

1. discover one real compiler bug
2. reduce it to the smallest useful reproducer
3. place that reproducer in the earliest owning PA
4. run the full regression gate
5. commit the fix and regression
6. only then move on to the next PA-fast failure

## Why This Is Faster

- the current checked-in frontend source sets shrink each frontend down to only
  the object files it actually needs
- the helper script compiles only that closure with `./dev/cppgm++`
- link still uses the host compiler, so this stays on the "honest objects, host link" side of the bootstrap process

That gives a much tighter loop than rebuilding the full self-host compiler for every early-frontend regression.

## Default Workflow

1. Make sure the host-built compiler is current enough to test:

```bash
make build
```

2. Run the PA-fast self-compiled check:

```bash
python3 scripts/run_pa_fast_self.py --pa pa1 --host-cxx /usr/local/opt/llvm/bin/clang++
```

3. Continue in order:

```bash
python3 scripts/run_pa_fast_self.py --pa pa2 --host-cxx /usr/local/opt/llvm/bin/clang++
python3 scripts/run_pa_fast_self.py --pa pa3 --host-cxx /usr/local/opt/llvm/bin/clang++
...
python3 scripts/run_pa_fast_self.py --pa pa9 --host-cxx /usr/local/opt/llvm/bin/clang++
```

The helper builds into `/tmp/cppgm-pa-fast-self/<pa>/` and reuses that directory unless `--clean` is passed.

This process now follows the normal self-emitted object path directly. There is
no separate fallback-disable honesty mode anymore because the host source-object
compile fallback machinery has been removed from the implementation.

## Required Bug-Fix Workflow

When a PA-fast run finds a failure, stop the discovery loop and switch into fix mode.

Do not keep scanning later PAs while the current issue is still unregressed and uncommitted.

The required sequence is:

1. confirm the failure is in our compiler
2. reduce it to the smallest focused source or object-level reproducer that still demonstrates the bug
3. identify the earliest assignment that should own the regression
4. add the regression there, even if the bug was first discovered from a later PA-fast binary
5. implement the fix in `dev/`
6. rerun the directly affected assignment suites
7. run the full repo regression gate
8. commit the fix and regression together
9. resume the PA-fast loop at the same PA until it passes cleanly

This is the key rule:

- PA-fast is for discovery
- small earliest-owner tests are for durable proof
- full regression plus a commit is required before advancing

## Comparison Path

If a PA-fast self-compiled binary fails and we need a host-built comparison
binary, build the normal host target:

```bash
make -C pa1 CPPGM_TEST_RUNNER=0 CXX=/usr/local/opt/llvm/bin/clang++
```

Use that host-built binary for symbol inspection, behavior comparison, or
focused `nm` / `lldb` analysis.

## Regression Expectations

Prefer the smallest durable regression that proves the real compiler bug:

1. use a narrow structural or smoke test when it captures the issue cleanly
2. avoid leaving the only proof as a large bootstrap-only reproducer
3. if the first reproducer lands in a later PA, move it down immediately when the real owner is earlier

Examples:

- LowIR typing or lowering bugs should move down to the earliest LowIR-owning PA
- constructor/destructor lifetime bugs should move down to the earliest PA that can express the lifetime rule
- hosted compile-only compatibility bugs may stay in `pa34` when that is truly the first
  owner
- hosted emitted-code link/runtime bugs may stay in `pa35` when that is truly the first
  owner

Full `test-report` remains the gate before the fix is considered closed.

Useful commands after landing a candidate fix:

```bash
make test-report-nobuild ACTIVE_TEST_REPORT_PAS='pa23 pa35 pa34' CPPGM_SKIP_DEV_REBUILD=1 \
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

```bash
make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

If the first durable repro turns out to belong below `pa34`/`pa35`, move it down
immediately rather than leaving it as a frontier-only smoke.

## Escalation Rules

Use the PA-fast loop first for early frontend regressions.

Escalate to the normal bootstrap frontier when:

- the failure only appears in the full `cppgm++` driver binary
- the failing change has broad fanout beyond one frontend closure
- the issue depends on later hosted/runtime/link behavior not exercised by `pa1` through `pa9`

## Notes

- This helper intentionally compiles the closure with `./dev/cppgm++` but host-links the result.
- It does not depend on the `paN` Makefiles supporting `cppgm++` as a drop-in `CXX`.
- It reuses the existing assignment test harnesses instead of inventing a second oracle.
- The checked-in frontend source manifest is the single source of truth for
  these reduced early-frontend builds.
