# Bootstrap Self-Host Link Fix Process

This document defines the workflow for bootstrap issues whose active failure stage is:

- `link-failed`
- `self-compile-smoke-failed`
- `self-run-smoke-failed`

It is a narrow supplement to [BOOTSTRAP_SELFHOST_FRONTIER_PROCESS.md](/Users/vishvananda/cppgm/legacy/BOOTSTRAP_SELFHOST_FRONTIER_PROCESS.md), focused on link-stage reductions and the owner/regression loop around `PA32`.

Direct command lines in this branch should use the build-first root runners:

- `make run-cppgm CPPGM_ARGS='--emit-* ...'` for PA10-28 text-output checks
- `make run-cppgm CPPGM_ARGS='-E/-c/...'` for the PA29-31 hosted/toolchain path used by bootstrap
- `make bootstrap-frontier*` for the serial bootstrap report itself

## When To Use This

Use this process only after the serial bootstrap probe has compiled the full bootstrap source set
and moved past compile-stage frontier discovery.

Discovery should still start from the normal bootstrap probe:

```sh
make bootstrap-frontier-fast-nobuild CXX=/usr/local/opt/llvm/bin/clang++
```

If the report result is `compile-failed`, stay in the normal bootstrap frontier process instead.
If `bootstrap-link-probe` reports `compile-artifacts-missing`, the saved fast frontier object
batch is no longer current. Rerun `make bootstrap-frontier` first so the fast probe can rebuild
the trusted prefix before using the link-only probe again.

The bootstrap frontier runners now preserve their build dirs by default, so the current object
batches normally remain available at:

- `/tmp/bootstrap-selfhost-frontier-fast.build`
- `/tmp/bootstrap-selfhost-frontier-cluster.build`
- `/tmp/bootstrap-selfhost-frontier-serial.build`

Each frontier report also writes a persisted trace-analysis sidecar on link failures:

- `*.trace.txt`
- `*.trace.details.txt`
- `*.trace.json`

When the same link-failed frontier report is regenerated and both the frontier JSON and
`scripts/bootstrap_trace_report.py` are unchanged, the reporter now reuses that persisted
`.trace.*` sidecar instead of rerunning the trace-analysis tool.

Use `make bootstrap-clean` only when you intentionally want to discard those saved object batches
and reports.

## Fast Refresh Loop

Once bootstrap is link-complete and the active failure has moved to `link`, `self-compile-smoke`,
or `self-run-smoke`, do not default to a fresh full frontier compile after every small runtime
fix.

The default runtime inner loop is now:

1. rebuild the host `./dev/cppgm++`
2. refresh only the affected preserved bootstrap objects:

   ```sh
   make bootstrap-refresh BOOTSTRAP_REFRESH_CHANGED='dev/src/foo.cpp dev/src/bar.h' \
     CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

3. read `/tmp/bootstrap-selfhost-frontier-cluster.refresh.md`
4. if the refresh relink/runtime probe still reproduces, continue reducing from there
5. only rerun the full frontier compile when the helper reports a broad invalidation or when the
   touched files are known hot paths

`bootstrap-refresh` uses the warmed host-build depfiles in `obj/release/.d` to compute the
reverse include closure, recompiles just that bootstrap source subset into the preserved
`/tmp/bootstrap-selfhost-frontier-cluster.build`, then reruns the normal preserved link/runtime
probe. This is the preferred replacement for paying the full compile wave on narrow runtime fixes.

## Goal

Turn one bootstrap link-stage failure into:

- one real implementation fix in `dev/`
- one smallest durable owner regression
- any earlier owner regressions uncovered while fixing it
- refreshed refs only where the verified behavior truly changed
- one commit

Then rerun bootstrap discovery and move to the next link-stage issue.

## Frontier Cadence

Do not rerun the full bootstrap frontier after every single symbol-level fix.

The default cadence is:

1. batch work by coherent root-cause family
2. keep each family in its own commit
3. use targeted regressions plus preserved frontier objects and
   `scripts/bootstrap_trace_report.py` between family commits
4. rerun the full bootstrap frontier after every 2-3 family commits

Rerun the full frontier immediately instead when a fix touches a hot file or a
subsystem that frequently shifts the active boundary:

- `dev/src/lowirgensemantic.cpp`
- `dev/src/lowir_object_backend.cpp`
- `dev/src/runtime_symbol_policy.cpp`
- `dev/src/symbol_linkage.cpp`

Safe batching examples:

- a shared libc++ iostream ABI cluster
- a shared compiler-runtime symbol cluster

Keep these kinds of issues separate even if they are all still link-stage:

- `template_angle_parser.cpp` signature drift
- `recog_token_cursor.cpp` mangling drift
- `logic_error` / `runtime_error` constructor issues
- tokenizer vtable emission
- `std::map` copy-ctor ABI issues

Reason:

- those fixes are more likely to uncover second-order bugs immediately
- they tend to cross subsystem boundaries
- batching them makes frontier movement much harder to attribute

The practical rule is:

- batch only clearly shared-family fixes
- keep each family commit coherent
- use the preserved `*.build` dirs and `.trace.*` sidecars for in-between work
- spend full frontier runs on cadence boundaries, not after every symbol

The intended loop is:

1. try to link bootstrap
2. reduce the failure honestly into `PA32` or an earlier owner
3. fix the real bug
4. add the owner regression(s)
5. refresh refs carefully, only where the fix intentionally changed output
6. rerun verify
7. use `bootstrap-refresh` for the narrow runtime relink check when the touched closure is small
8. commit
9. rerun bootstrap and repeat

## Link-Stage Reduction Ladder

1. Reproduce the active stage with the bootstrap probe and keep the exact failing command.

   Use the stage summary from `scripts/report_bootstrap_frontier.py`. For link-stage failures,
   the active stage will be `link`, `self-compile-smoke`, or `self-run-smoke`.
   Start with the auto-written `*.stdout.txt` sidecar for the full failing output instead of
   rerunning immediately just to recover long linker or crash text.
   If the failing stage crashed, start with the auto-written `*.crash.txt` sidecar from the report
   before doing any manual debugger work.

2. Classify the failure before reducing.

   Typical buckets:

   - missing or duplicate runtime / builtin / EH symbols
   - object-emission or link-map contract drift
   - driver-produced objects that link but fail when the self-built compiler is used
   - bootstrap smoke execution mismatches

3. Reduce to the smallest honest reproduction.

   Prefer this order:

   - preserved frontier objects from the current `*.build` dir when they already expose the
     caller/provider mismatch you need
   - smallest direct `/tmp` source or object set that still reproduces the link failure
   - `pa32/tests/link` if the issue is truly a link/runtime contract problem
   - `pa32/tests/compile` if the failure only appears when the self-built compiler compiles a small hosted case
   - earlier `paN` if the reduction exposes a language or LowIR bug that predates `PA32`

   For `pa32/tests/link` failures, inspect both the preserved `*.impl.stderr` and the
   accompanying `*.impl.unresolved_symbols` report. The `nm` report is a fast boundary check over
   the explicit objects and helper libraries in the smoke; it does not replace the real host link,
   but it often shows the whole missing-symbol set more clearly than the final linker message.
   Failed smokes also keep their generated `.o` files, `*.impl.link_command`, and a
   `*.impl.link_verbose` rerun with host-driver `-v`, so you can inspect the exact failed link
   without recompiling the test first.

4. Move the regression backward if the reduction stops being link-only.

   If the real bug is an earlier semantic / special-member / LowIR issue, the durable regression
   belongs in the earliest owning `paN`, not only in `PA32`.

5. If the reduction or regression-writing step exposes a broader deferred LowIR cleanup or
   metadata gap that is not part of the current link fix, record it in
   [`docs/lowir-evolution-plan.md`](/Users/vishvananda/cppgm/docs/lowir-evolution-plan.md)
   before closing the item.

6. Keep one live parent and one live child at a time.

   If fixing the link issue uncovers an earlier owner bug, close that child first, verify it,
   refresh refs if needed, commit it if it stands alone, then rerun the parent link repro.

## `PA32` Placement Rules

Use `pa32/tests/link` when the bug is about linking or runtime contract shape itself:

- builtin runtime symbol ownership
- EH runtime symbol ownership
- host runtime shims
- link-map drift that changes symbol/layout contracts

Use `pa32/tests/compile` when the failing behavior is still hosted compilation, even if it was
discovered from bootstrap link work:

- the self-built compiler cannot compile a reduced hosted program
- the reduction proves the real issue is emitted IR / semantic generation rather than the final link

Move the regression to an earlier `paN` when the reduced bug is clearly not `PA32`-specific:

- special member generation
- conversion / overload resolution
- template ownership
- earlier LowIR behavior

When in doubt, ask: "If this bug had been found from a normal assignment test rather than from
bootstrap linking, where would we have wanted the regression?" Put the durable test there.

## Reduction Shapes

Prefer reductions that preserve the actual failure mode rather than just the same implementation
area.

Typical shapes:

- `pa32/tests/link`: unresolved symbol, duplicate symbol, missing runtime shim, EH/runtime object
  contract mismatch, or hosted object ownership problem
- `pa32/tests/compile`: self-built compiler cannot compile a small hosted case, or the reduced
  issue is still compile-time codegen/semantic behavior rather than final link
- earlier `paN`: a smaller standard-language or LowIR issue now proven underneath the link-stage
  symptom

Use `/tmp` scratch reductions freely while searching, but do not stop there if the case belongs in
`PA32` or an earlier assignment.

## Earlier Regression Loop

For each reduced child that lands earlier than `PA32`:

1. add the earliest owning regression
2. rerun that assignment's worker or targeted test
3. inspect the output diff
4. refresh the ref only if the new output matches the intended semantics
5. rerun the broader verify gate

Do not bless a ref just because it changed while chasing a link failure.

## Ref Refresh Rules

When a link-stage fix changes earlier text output:

1. inspect the exact diff with `diff -u`
2. confirm that the new output reflects the intended semantic change
3. prefer refreshing only the specific affected refs, not a bulk `ref-test`, unless the change
   obviously applies across a whole verified cluster
4. rerun the owning assignment compare step after the refresh

If a full assignment refresh is the right tool, use the existing `ref-test` target and treat it as
a sharp tool:

1. run `make -C paN ref-test`
2. inspect `git diff -- paN/tests`
3. confirm that representative hunks match the intended semantics
4. rerun the assignment worker / compare step
5. only then keep the ref update

Do not use `ref-test` as a substitute for understanding the semantic change.

Bulk `ref-test` is still allowed, but only after:

- the semantic direction is understood
- the changed-file list is reviewed
- representative hunks look consistent

## Validation Gate

Every closed link-stage issue should run this minimum gate:

1. narrow build needed for the reproduction, usually:

   ```sh
   make build CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

2. the reduced repro itself

3. the owning assignment-local regression check

4. the broad later-suite gate:

   ```sh
   make verify-bootstrap-link-fast CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

   That fast gate already includes:

   - compile sentinels for high-risk compiler sources
   - the bootstrap-heavy owner suites
   - the later link/runtime suites
   - a direct bootstrap link probe against the saved frontier object batch

   When the active issue is already in self-host runtime rather than link ownership, prefer the
   narrower incremental check first:

   ```sh
   make bootstrap-refresh BOOTSTRAP_REFRESH_CHANGED='dev/src/foo.cpp dev/src/bar.h' \
     CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

   then spend `verify-bootstrap-link-fast` on the family checkpoint rather than every edit.

5. the perf sanity probe:

   ```sh
   make build CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   /usr/bin/time -p env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
     ./dev/cppgm++ -c dev/src/template_audit.cpp -o /tmp/template_audit.perf.o
   ```

   Rebuild immediately before the timed command so the binary is fresh without including make's
   no-op overhead in the perf number. Use the current reduced perf reproducer instead when an
   active hotspot note has a narrower probe that better represents the touched codepath.

6. the bootstrap frontier probe again:

   ```sh
   make bootstrap-frontier-fast-nobuild CXX=/usr/local/opt/llvm/bin/clang++
   ```

7. at checkpoints or before merge, the full validation gate:

   ```sh
   make verify-bootstrap-full CXX=/usr/local/opt/llvm/bin/clang++ \
     CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
   ```

8. once the fast frontier is clear, the serial self-hosting confirmation:

   ```sh
   make bootstrap-frontier-serial-nobuild CXX=/usr/local/opt/llvm/bin/clang++
   ```

## Commit Discipline

- Keep one link-stage issue per commit.
- Amend the previous commit only when the new fix is clearly fallout from the same root issue.
- If the reduction uncovers an independent earlier-owner bug, do not silently fold it into the
  parent link commit unless it is inseparable from the same root cause.
- After committing, rerun bootstrap discovery immediately and record the next active frontier.

Each commit should explain both sides of the closure:

- the bootstrap symptom it removed
- the durable regression(s) that now own the behavior

## Practical Inner Loop

Use this order in practice:

1. bootstrap probe says link-stage failure
2. reduce it to `/tmp` and then to `PA32` or earlier owner
3. fix the real bug in `dev/`
4. add earlier regressions where needed
5. refresh only the refs that genuinely changed
6. rerun verify and perf sanity
7. commit
8. rerun bootstrap probe
9. repeat on the next link-stage issue
