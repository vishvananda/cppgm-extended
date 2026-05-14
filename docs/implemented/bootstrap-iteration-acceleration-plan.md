# Bootstrap Iteration Acceleration Plan

## Goal

Reduce bootstrap/debug cycle time while preserving ownership discipline:

- find the next real bootstrap blocker quickly
- expose several likely follow-on blockers in the same cycle
- avoid paying for full `test-report` on every small closure
- keep one authoritative serial self-host confirmation before calling a branch clean

The target is not "land multiple unrelated fixes in one commit." The target is:

- discover and pre-reduce multiple likely bugs per cycle
- validate active work with a cheaper high-signal gate
- reserve the full expensive gate for checkpoints

## Current Friction

### 1. The docs require expensive validation too often

The current bootstrap docs still require this after every closed frontier item:

- owning assignment suite
- full root `test-report`
- `make bootstrap-frontier-nobuild`
- perf sanity

That is expensive when:

- `test-report` is still minutes long
- the active fix is small and tightly localized
- the next likely failure is already visible from bootstrap source order

### 2. The written bootstrap modes do not match the actual implementation

The docs describe:

- `bootstrap-frontier-fast`: fast resume
- `bootstrap-frontier-slow`: serial final confirmation

But the current root [Makefile](/Users/vishvananda/cppgm/Makefile) passes `--jobs $(BOOTSTRAP_FRONTIER_JOBS)` to both fast and slow targets, and [report_bootstrap_frontier.py](/Users/vishvananda/cppgm/scripts/report_bootstrap_frontier.py) uses parallel compile discovery whenever:

- there is no resume state
- `--jobs > 1`

So today:

- the current "fast" mode is a resume-based frontier probe
- the current "slow" mode is really a parallel full-source discovery pass, not a true serial confirmation

This mismatch is costing clarity and preventing the parallel compile pass from being used intentionally as a multi-bug discovery tool.

### 3. The bootstrap script already gathers more information than it reports

The current bootstrap script already has:

- a persistent resume build dir
- full-source parallel compile capability
- source-order sorting of results

But it reports only:

- the first failing source/stage

That means the tool is already paying to discover more than one likely blocker in some modes, but the workflow throws the extra information away.

### 4. The docs rely on a fast gate that is no longer first-class

The tracker and frontier docs repeatedly mention:

- `verify-fast`
- `verify-fast-pa10-31-nobuild`

but the current root [Makefile](/Users/vishvananda/cppgm/Makefile) does not expose those targets.

So the written process depends on a cheaper validation tier that is not currently a first-class command.

### 5. The current fast resume intentionally trusts too much prefix

The current fast resume mode is useful, but it intentionally trusts the already-passing prefix and can miss earlier regressions introduced by the new fix.

That is acceptable as an inner loop only if it is paired with:

- a cheap sentinel audit of high-risk earlier files, and
- a slower full confirmation at checkpoints

Right now the fallback from "optimistic resume" is usually "pay for full `test-report`."

### 6. Bootstrap ownership history is concentrated, but the validation gate is not

The current bootstrap tracker history is not evenly distributed across all assignments.

From [BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md](/Users/vishvananda/cppgm/legacy/BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md), the dominant owners are:

- `PA25`: 21
- `PA21`: 20
- `PA31`: 15
- `PA15`: 14
- `PA12`: 13
- `PA18`: 13
- `PA16`: 8
- `PA26`: 6
- `PA14`: 6

So a bootstrap-oriented fast gate should be centered on those suites and only widen when the touched code demands it.

## Desired End State

The bootstrap loop should have four explicit tiers:

1. `frontier-fast`
   - resume from the current trusted prefix
   - report the active frontier quickly

2. `frontier-cluster`
   - compile a larger lookahead in parallel
   - report the first failing source plus the next likely failures and normalized clusters

3. `verify-bootstrap-fast`
   - high-signal assignment subset and sentinels
   - cheap enough to run after most frontier-item closures

4. `frontier-serial` / `verify-bootstrap-full`
   - true serial self-host confirmation
   - full `test-report`
   - only at checkpoints, handoff points, or before merge

This preserves one active frontier while still letting the developer queue and pre-reduce likely next blockers.

## Proposed Changes

## 1. Split Bootstrap Discovery Into Three Explicit Modes

### A. `bootstrap-frontier-fast`

Keep the current resume-state behavior as the default inner-loop probe:

- recompile the current frontier
- if it passes, keep moving until the next failure
- stop on the first failing stage

This should stay optimistic and cheap.

### B. `bootstrap-frontier-cluster`

Add a new explicit cluster-discovery mode built on the existing parallel compile machinery.

Behavior:

- compile the full source set, or at least a configurable lookahead window, in parallel
- still identify the active frontier as the first failing source in source order
- also report:
  - the next `K` failing sources in source order
  - normalized diagnostic fingerprints
  - cluster counts by fingerprint
  - suggested direct repro commands

This becomes the tool for "solve multiple bugs per cycle" in practice:

- fix one blocker
- but leave the next two or three likely blockers already discovered and lightly reduced

Recommended flags:

- `--lookahead-files N`
- `--lookahead-failures K`
- `--cluster-by normalized-diagnostic`
- `--emit-direct-probes`

### C. `bootstrap-frontier-serial`

Make a truly serial confirmation mode:

- force `--jobs 1`
- compile in source order
- link
- self-compile smoke
- self-run smoke

This is the authoritative final self-host check.

### Immediate CLI cleanup

Update the root [Makefile](/Users/vishvananda/cppgm/Makefile) so the names reflect the real behavior:

- `bootstrap-frontier-fast`
  - current resume-state mode
- `bootstrap-frontier-cluster`
  - current parallel full-source discovery mode
- `bootstrap-frontier-serial`
  - new real serial end-to-end confirmation

`bootstrap-frontier-slow` should either:

- be renamed to `bootstrap-frontier-cluster`, or
- be changed to `--jobs 1`

The current docs and implementation should not continue disagreeing.

## 2. Teach The Frontier Report To Surface More Than One Useful Failure

Enhance [report_bootstrap_frontier.py](/Users/vishvananda/cppgm/scripts/report_bootstrap_frontier.py) so it writes more than one actionable item.

Add to the JSON/markdown report:

- `active_frontier`
- `next_failures`
- `failure_clusters`
- `suggested_owner_suites`
- `suggested_direct_probes`

Example output shape:

- active frontier: `dev/src/semantic_lifetime.cpp`
- next likely failures:
  - `dev/src/semantic_class_model.cpp`
  - `dev/src/template_resolution.cpp`
  - `dev/src/lowirgensemantic.cpp`
- normalized clusters:
  - `no viable overload for call any_of`: 2 files
  - `failed type template argument resolution`: 3 files

This lets the active worker stay disciplined while also:

- preparing child items sooner
- spotting when one broader semantic fix could clear several files

## 3. Restore A Real Fast Validation Tier

Reintroduce a first-class root fast gate for bootstrap/frontier work.

Recommended targets:

- `verify-bootstrap-fast`
- `verify-bootstrap-fast-nobuild`
- `verify-bootstrap-full`

### `verify-bootstrap-fast`

This should be the default frontier closure gate instead of full `test-report`.

Initial default assignment set should be based on actual bootstrap owner history:

- `pa12`
- `pa14`
- `pa15`
- `pa16`
- `pa18`
- `pa21`
- `pa25`
- `pa26`
- `pa31`

Optional additions by touched area:

- if backend/LowIR files changed: add `pa22`, `pa23`, `pa24`
- if driver/object/link/runtime files changed: add `pa29`, `pa30`, `pa31`
- if parser/tokenization files changed: add `pa10`, `pa12`, `pa14`

This gate should be implemented as a real target, not only a tracker convention.

### `verify-bootstrap-full`

This should mean:

- full root `test-report`
- `bootstrap-frontier-serial`

Use it only:

- before merge
- after a cluster/closure-program slice lands
- after async import batches
- before declaring a branch fully self-hosting

## 4. Add Cheap Sentinel Audits For Trusted-Prefix Risk

Fast resume is still valuable, but it needs a cheaper hedge than full `test-report`.

Add a small bootstrap sentinel compile set:

- `dev/src/callsemantic.cpp`
- `dev/src/template_resolution.cpp`
- `dev/src/semantic_expression.cpp`
- `dev/src/semantic_lifetime.cpp`
- `dev/src/semantic_class_model.cpp`
- `dev/src/lowirgensemantic.cpp`
- `dev/src/macroizer.cpp`
- `dev/src/cli_batch_frontend.cpp`

Use direct probes of the shape:

```sh
make run-cppgm CPPGM_ARGS='-c -I dev/src -o /tmp/<stem>.o dev/src/<file>.cpp'
```

Add:

- `bootstrap-sentinels`
- `bootstrap-sentinels-nobuild`

These should be cheap enough to run after most frontier-item closures and much cheaper than full `test-report`.

## 5. Make The Fast Gate Family-Aware

The existing docs already use closure programs for recurring families.

Build that into the faster bootstrap loop:

- template family changes
  - add `pa18`, `pa21`, `pa25`, `pa31`
- lambda/capture changes
  - add `pa25`, `pa26`, `pa31`
- object-model/special-member changes
  - add `pa15`, `pa16`, `pa21`, `pa29`, `pa31`
- LowIR/native changes
  - add `pa14`, `pa22`, `pa23`, `pa24`, `pa29`, `pa31`

This avoids both extremes:

- running the whole repo every time
- running only one owner test and missing obvious sibling fallout

## 6. Make Async Discovery A First-Class Output Of Cluster Mode

The current docs already allow async discovery intake.

Use cluster mode to produce queueable next steps:

- a `next_failures` list
- normalized families
- prefilled tracker-note snippets or temp note files

That changes "one bug per cycle" into:

- one committed fix per cycle
- several next likely items already discovered and ready for reduction

That is the safest way to increase throughput without mixing root causes.

## 7. Update Documentation To Match Reality

The bootstrap and hosted frontier docs should be updated together.

### Replace stale `test-report` examples

Many docs still hard-code:

```sh
TEST_REPORT_ASSIGNMENT_JOBS=6 TEST_REPORT_SUBTEST_JOBS=1
```

That is now stale relative to the root [Makefile](/Users/vishvananda/cppgm/Makefile), which defaults to:

- reverse-order `test-report`
- subtest jobs `2`
- top-level jobs derived from processor count

Either:

- omit the knobs in the docs, or
- document the current default behavior instead

### Stop referencing non-first-class fast gates

If `verify-fast-pa10-31-nobuild` is expected, it needs to exist as a real target.

Otherwise the docs should stop naming it.

## 8. Tighten Checkpoint Rules

Recommended validation policy:

### Default per frontier-item closure

- owning regression suite
- `verify-bootstrap-fast`
- `bootstrap-frontier-fast-nobuild`
- perf sanity probe
- parent reduced repro if working through a child

### Required only at checkpoints

- `verify-bootstrap-full`
- `bootstrap-frontier-serial`

Checkpoint moments:

- before merge
- after clearing an entire cluster/family slice
- after link-stage closure batches
- after importing async fixes
- before handing the branch to someone else

This is the main cycle-time reduction lever.

## Recommended Implementation Order

## Phase 1: Correctness And Naming

1. Make a real serial bootstrap target with `--jobs 1`.
2. Rename or split the current parallel "slow" mode into explicit cluster discovery.
3. Update the docs so fast/cluster/serial mean what they say.

## Phase 2: Fast Gate

1. Add `verify-bootstrap-fast` and `verify-bootstrap-full`.
2. Seed the fast gate with the historically dominant owner suites:
   - `pa12`, `pa14`, `pa15`, `pa16`, `pa18`, `pa21`, `pa25`, `pa26`, `pa31`
3. Add backend/toolchain expansions based on touched files.

## Phase 3: Better Discovery Output

1. Extend `report_bootstrap_frontier.py` to emit:
   - first failing source
   - next `K` failing sources
   - normalized clusters
   - suggested direct repro commands
2. Add `bootstrap-frontier-cluster`.

## Phase 4: Sentinel Audits

1. Add `bootstrap-sentinels`.
2. Wire it into `verify-bootstrap-fast`.

## Phase 5: Tracker/Async Integration

1. Teach the tracker workflow to record:
   - active frontier
   - next likely failures
   - whether a cluster/closure-program run is now required
2. Optionally generate async intake note stubs from cluster discovery.

## Expected Impact

If implemented cleanly, the likely gains are:

- much cheaper default validation after each frontier-item closure
- earlier visibility into the next two or three likely blockers
- fewer wasted `test-report` runs while still keeping one hard final gate
- less confusion about whether bootstrap discovery is serial or parallel

The main throughput gain should come from process shape, not just raw speed:

- fix one blocker
- already know the next few likely blockers
- validate with a fast high-signal gate
- only pay for the full repo gate at real checkpoints

That is the safest path to solving more bugs per cycle without losing frontier discipline.
