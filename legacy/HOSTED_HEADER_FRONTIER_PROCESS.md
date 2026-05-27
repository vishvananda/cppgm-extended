# Hosted Header Frontier Process

This document defines the active workflow for hosted-header frontier work.

It replaces the older split between:

- `legacy/HOSTED_HEADER_COMPATIBILITY_TRACKER.md`
- `legacy/HHC_REGRESSION_LEDGER.md`
- `legacy/HHC_REGRESSION_PLACEMENT_PROCESS.md`

Those older files remain useful as historical reference. The completed hosted
frontier tracker is now archived at
[`legacy/HOSTED_HEADER_FRONTIER_TRACKER.md`](/Users/vishvananda/cppgm/legacy/HOSTED_HEADER_FRONTIER_TRACKER.md).
If hosted-header frontier work is reopened, promote that tracker back to an
active location or create a new active tracker before recording new rows.

## Goal

Turn hosted-header and bootstrap-discovered failures into:

- a concrete implementation fix in `dev/`
- a correctly owned assignment-local regression in the earliest real `paN`
- a durable tracker entry that records both the frontier state and the regression outcome

The process should make it hard to lose the current frontier, and hard to ship a fix without
leaving behind the right regression.

## Core Rules

1. `ROADMAP.md` and the owning `paN/README.md` define assignment ownership.
2. Current shared-binary behavior is only evidence, not assignment scope.
3. Standard-language fixes belong in the earliest written milestone that owns them.
4. True hosted/vendor compatibility stays in `PA32`.
5. Implement the actual missing functionality for the true owning surface. Do not "work around"
   a frontier blocker with a header-specific hack, a one-off special case, a source rewrite that
   only sidesteps the current failure, or the smallest patch that only clears the current smoke.
6. Once ownership is identified, the unit of work is the full feature/compatibility surface
   required by that owner, not just the single frontier header. If the proper fix covers a
   broader surface area, implement that broader fix and add enough regressions to cover it
   instead of pretending the frontier issue was only one narrow case.
7. If the fix changes LowIR syntax, semantics, or any backend-facing LowIR contract, add
   direct LowIR/native assignment tests for that change and update the relevant handout/spec
   before returning to hosted frontier work. If reduction, regression-writing, or ref analysis
   uncovers a broader deferred LowIR cleanup or metadata gap that should be handled later rather
   than in the current fix, append a short note to
   [`docs/lowir-evolution-plan.md`](/Users/vishvananda/cppgm/docs/lowir-evolution-plan.md)
   before closing the frontier item.
8. Do not use a long-lived draft regression directory.
9. Use `/tmp` or another scratch location for temporary reductions while investigating.
10. Once ownership is clear and the fix is real, add the regression directly to the owning
   assignment in the same work.
11. If a frontier item lands in a recurring semantic family with an existing closure program,
   run that closure program before continuing to the next hosted smoke.
12. Batch hosted-header sweeps must use the same `CPPGM_HOST_CXX` toolchain as the active
   frontier smoke so the report and the frontier are talking about the same libc++ tree.
13. Once a frontier item is fixed, validated, and its owning regressions are in place,
   checkpoint that item before doing broader reduction or exploratory implementation for the
   next blocker. Do not mix a closed `HHC-###` with partial work for the next one.
14. If reduction uncovers a smaller real owner bug underneath the current frontier item,
    record that child item explicitly instead of silently folding it into the same worktree.
15. Underlying items form a strict LIFO root-cause stack. Only the top item on that stack is
    allowed to receive implementation work.
16. Each stacked child item must get its own regression, validation, tracker update, and
    commit before work resumes on its parent item.
17. If a child fix causes regressions, validation stalls, or materially changes the parent
    failure, stop and re-evaluate from that new top-of-stack state instead of continuing to
    accumulate fixes.
18. Async small-fix discovery notes are candidate imports, not automatically merge-ready patches.
    Revalidate any relevant async closure on current `HEAD` before pulling it into the active
    hosted/bootstrap branch.
19. When a child fix or async import is isolated to a small set of files, prefer staged-child
    validation in the warm main checkout:
    - stage only the candidate files
    - stash unrelated work with `git stash push --keep-index -u`
    - reuse the warm `obj/` tree for the validation build
    - restore the stash after commit or after backing the candidate out
20. Use the persistent clean intake worktree only as a fallback when the fast staged path is not
    trustworthy, for example because:
    - the child and parent touch the same file
    - `git stash --keep-index` or index writes are failing
    - you specifically need a pristine clean-`HEAD` proof
    - a long-running local job would observe the stashed file movement

## Status Labels

- `open`: blocker identified, no fix yet
- `in-progress`: reduction or implementation is underway
- `completed`: fix landed, owning regression exists, and the tracker row is fully recorded

## Workflow

### 1. Discover

When a new hosted frontier failure appears:

- record it immediately in the active hosted frontier tracker
- capture the trigger header or smoke, the visible failure, and the current frontier ordering
- assign an `HHC-###` id right away

Do this before solving the bug, so the frontier can be resumed cleanly after interruptions.

### 2. Reduce

Reduce the failure only far enough to answer these questions:

- what is the actual failing language/toolchain feature?
- is it standard-language behavior or hosted/vendor compatibility?
- what is the earliest assignment that should own the regression?

If the boundary is still unclear, keep the tracker row `open` and continue investigating.

Before implementing the fix, capture a reduced repro that still fails on the current broken
commit and save the exact failing command line. That repro can stay in `/tmp` while ownership
is still unclear, but it must exist before the implementation work starts. Do not "discover"
the reduction after the fix and then assume it was the owning surface unless you have rechecked
it on both:

- the last broken commit
- the first fixed commit

If a post-fix reduction does not separate those two states, it is not yet an owning regression.

After ownership is clear, do not optimize for "first passing smoke". Optimize for the correct
owner-level feature closure. A reduction is there to identify the feature boundary, not to excuse
landing a narrower patch than that boundary actually requires.

### 2a. Run Closure Programs For Recurring Families

If the reduced failure is part of a recurring semantic family, do not stop at the single
smoke. Run the owning closure program and treat the family slice as the unit of work.

Current required closure program:

- `SUBOBJECT_SEMANTIC_CLOSURE.md`
  - use this for class subobjects, nested member access, class-value operators on objects,
    pass/return of objects with subobjects, and template-bound member-object completion
- `TEMPLATE_ANGLE_AUDIT_MATRIX.md`
  - use this for parser/template-angle families: nested template-ids, `<` vs comparison,
    scoped fragment reparsing, and semantic template-id parsing

The intent is to stop rediscovering the same semantic family through slightly different hosted
headers.

For parser/template families specifically, the item is not ready to close until the relevant
rows in `TEMPLATE_ANGLE_AUDIT_MATRIX.md` have been rerun alongside the active smoke and the
batch hosted-header sweep.

### 2b. Use Batch Hosted-Header Sweeps To Discover Clusters

This is not optional when choosing the next frontier item after a closure or when a new blocker
looks like a recurring parser/semantic family. Use the current nearest smoke for sharp local
reduction first, and only pay for the batch sweep once that smoke has advanced or when the new
failure clearly looks like a broader family/cluster case.

Do not advance the frontier by "just taking the next smoke" without checking the sweep. A new
`HHC-###` row or tracker promotion is only ready once you have both:

- a concrete smoke or reduced repro for local debugging
- a sweep-backed cluster note that says either:
  - how many sibling headers fail for the same normalized reason, or
  - that the sweep did not find a broader cluster and this looks isolated

The required behavior here is broader than "confirm the next smoke still fails". The sweep is how
you look for other headers that are failing for the same root cause so the next frontier step is a
clustered language/toolchain issue, not just a single lucky reproduction. But once the backlog is
down to mostly singleton headers, rerunning the sweep before the current smoke advances usually adds
no new information and is not required.

Default rule:

- while the current frontier smoke still fails for the same blocker, iterate on that smoke only
- do not rerun the sweep just because you made the local reduction smaller, added a sharper
  temporary repro, or confirmed the same blocker in another sibling header
- once the current frontier smoke clears, run the sweep once to choose the next representative
  blocker and record whether it is still part of a larger cluster or now effectively isolated
- in the late singleton phase, treat that first bullet as the normal case: do not rerun the
  sweep just because you have a new local reduction or a sharper reproduction of the same blocker

Required moments to run the sweep:

- after closing a frontier item and before choosing the next representative blocker
- when the newly exposed blocker appears to belong to a recurring language family
- after a closure-program fix, to measure which headers advanced and what cluster is now next

You may also run the sweep earlier if the failure signature has changed in a way that strongly
suggests one fix exposed a broader sibling family, but that is the exception, not the default.

Do not promote the next blocker into a new `HHC-###` item until this sweep evidence has been
recorded in the active hosted frontier tracker. The next frontier item is not fully discovered
until both the local smoke and the sweep-backed cluster note are written down.

Run the PA32 batch sweep tool against the same host compiler used by the active frontier:

```sh
cd pa32
env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ make header-sweep
```

Useful variants:

```sh
cd pa32
env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ make header-sweep-recursive
env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ make header-sweep-internal
```

Use the generated report under `pa32/testout/` to:

- cluster many failing headers by normalized diagnostic fingerprint
- find sibling occurrences of the current blocker so you understand the real blast radius
- pick one representative cluster as the next frontier item
- estimate how much surface one fix might unblock

The tracker should reflect both views:

- the concrete nearest advancing smoke
- the broader cluster evidence from the batch sweep when choosing the next `HHC-###`

Do not create one `HHC-###` row per failing header from the sweep. Promote representative
clusters into the tracker, not raw per-header noise.

In practice, each completed frontier turn should leave behind both:

- one sharp smoke or reduced repro for local debugging
- one sweep-informed note about how many other headers fail for the same reason

If one of those is missing, the discovery step is not complete yet.

### 2c. Use Targeted Parser / Template Tracing When Reductions Stall

Do not add ad hoc debug prints when a hosted reduction is stuck in parser, template-angle, or
template-resolution code. Use the structured parser trace first, and keep it filtered enough
that the failure dump still points at the decision that mattered.

The trace system is environment-driven:

- `CPPGM_TRACE=<category[,category...]>`
- `CPPGM_TRACE_FILE=<substring>`
- `CPPGM_TRACE_SYMBOL=<substring>`
- `CPPGM_TRACE_ON_ERROR=1`
- `CPPGM_TRACE_LIMIT=<n>`
- `CPPGM_TRACE_LIVE=1` for immediate streaming when a tiny reduction is already available

Recommended first-pass categories:

- `parser.angle`
  - use for `<` vs template-id disambiguation and nested-angle failures
- `parser.fragment`
  - use for semantic fragment reparsing, template-id suffix/follower failures, and type-vs-expr
    fragment ambiguity
- `template.resolve`
  - use for template argument classification, deduction drops, explicit-argument resolution, and
    overload-candidate survival
- `overload`
  - use when the hosted blocker is an ambiguity or an unexpectedly viable/losing call candidate

Recommended workflow:

```sh
env CPPGM_TRACE=parser.angle,parser.fragment \
    CPPGM_TRACE_FILE=allocator \
    CPPGM_TRACE_ON_ERROR=1 \
    CPPGM_TRACE_LIMIT=400 \
    make run-cppgm CPPGM_ARGS='-c -o /tmp/hhc.o pa32/tests/frontier/HHC-397-allocator-traits-max-size-ambiguity-smoke.t'
```

Then narrow further as needed:

```sh
env CPPGM_TRACE=template.resolve,overload \
    CPPGM_TRACE_SYMBOL=max_size \
    CPPGM_TRACE_ON_ERROR=1 \
    CPPGM_TRACE_LIMIT=250 \
    make run-cppgm CPPGM_ARGS='-c -o /tmp/hhc.o /tmp/hhc397-direct.cpp'
```

Guidelines:

- prefer `CPPGM_TRACE_ON_ERROR=1` over `CPPGM_TRACE_LIVE=1` for real hosted headers
- start with one or two categories, not `all`
- add `CPPGM_TRACE_FILE` or `CPPGM_TRACE_SYMBOL` early, otherwise the trace is usually too noisy
- when the failing family is template-argument classification, start in `template_argument_semantics`
  through `template.resolve` before adding more caller-side logs
- if a new trace site is needed, add it at the shared owning layer instead of the one current
  caller that happened to expose the bug

### 2d. Check Async Small-Fix Discoveries Before Deepening A Frontier

When a hosted blocker is still open after a sharp local reduction, or immediately after closing one
frontier item and before beginning deeper work on the next, check `async-small-fix-discovery/` for
relevant standalone notes.

Treat those notes as prior art and candidate imports only. Do not merge an async closure commit
just because the note says it passed in its own worktree.

The default import path should reuse the warm main checkout when the async closure is isolated
cleanly enough to validate with `git stash --keep-index`. Use the persistent clean intake
worktree only when that staged fast path is unsafe or unreliable.

The required import gate is:

1. apply the async closure on current `HEAD` in the main checkout, typically with
   `git cherry-pick -n <commit>`
2. stage only the imported files you want to validate
3. stash unrelated live work with `git stash push --keep-index -u`
4. run one incremental `make -C dev CXX=/usr/local/opt/llvm/bin/clang++`, or the note's required
   narrower `dev/` build target(s) if the note records them
5. rerun the note's direct repro or representative file(s)
6. rerun `make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
7. rerun the current hosted smoke or reduced repro
8. commit the import if it passes, then restore the stashed live work

If the staged fast path is unsafe, repeat the same gate in the persistent clean intake worktree
instead of forcing it through the main checkout.

Do not rerun a separate owning assignment-local suite during intake unless you are using it as a
faster local iteration step before the full `test-report` gate.

Only pull the async closure into the active branch if that current-`HEAD` revalidation passes.
If it fails, keep the note for context, but continue the serial frontier process from the live
current-`HEAD` state rather than assuming the async closure is still valid.

### 2e. Maintain An Explicit Root-Cause Stack

When reduction of the current frontier item exposes a smaller real owner bug underneath it,
do not treat that as a private scratch note. Record it immediately as a child item.

Required tracking for a child item:

- assign it an `HHC-###` id immediately
- record its parent item
- record the reduced repro and exact failing command
- record why the parent cannot be closed until this child is fixed
- place it at the top of a LIFO root-cause stack in the tracker snapshot/backlog

Execution rules:

1. The parent frontier item stays open while the child is being root-caused.
2. Once the child owner and fix are clear, finish that child completely:
   - add the owning regression
   - validate the owning suite and required repo-level checks
   - confirm the parent repro/smoke changes in the expected way
   - update the tracker
   - commit the child fix
3. Only after that commit may work resume on the parent item.
4. If reduction finds another deeper child, repeat the same rule and push it on top of the
   stack rather than accumulating multiple uncommitted lower-layer fixes.

The intent is to make the unwind explicit:

- push deeper only while root-causing
- close children independently
- pop back to the parent from committed state

This is mandatory whenever a lower-layer bug has become a real owner-level item rather than a
temporary hypothesis.

### 3. Assign Ownership

Use this decision order:

1. Is it true hosted/vendor compatibility?
   - examples: `__has_*`, `#include_next`, GNU attributes, builtin probes, hosted predefined
     macros, libc++ compatibility glue
   - owner: `PA32`
2. Is it standard pre-`PA10` behavior?
   - default policy: do not reopen the `PA1`-`PA9` contracts just because hosted work found it
   - if it is only needed for hosted compatibility, keep it in `PA32`
3. Is it standard C++ behavior already owned by a later front-end milestone?
   - place it in the earliest real `PA10+` owner
4. Is it still ambiguous?
   - keep the tracker row `open`
   - still write a durable temporary regression under `tests/frontier/` in the same `paN`
     that provides the binary currently being used for the frontier investigation
   - record that temporary frontier path in the tracker
   - once ownership is clear, migrate that regression into the final owning assignment-local
     suite and update the tracker accordingly

### 4. Fix

Implement the fix in `dev/`.

While doing so:

- keep the reduction in scratch space until the ownership is stable
- do not create or revive `draft_regressions/`
- keep the tracker row updated if the frontier moves or splits
- prefer principled capability work over smoke-specific unblocking
- split one hosted failure into multiple follow-on items when it actually exposes multiple
  missing standard features or ownership cases
- if final ownership is still unclear, promote the stabilized reduction into
  `tests/frontier/` in the current frontier PA instead of leaving it only in scratch space

### 4a. Full-Fix Standard

The default bar is:

- implement the real capability
- make the result coherent with the written milestone boundary
- leave behind enough regression coverage to keep the whole repaired surface durable

The wrong pattern is:

- special-casing one libc++ spelling
- forcing the current hosted smoke through a narrow branch
- leaving the underlying language/toolchain feature still partially broken

If the choice is between a narrow smoke workaround and a proper implementation of the
underlying feature, this process requires the proper implementation unless there is an
explicit assignment-boundary reason not to do so.

### 5. Place The Regression

Once the fix is real and the owner is clear:

- add the finalized regression directly to the owning `paN`
- use ordinary numeric test numbering, not `HHC-###` filenames
- place it near similar tests
- keep easier tests earlier and harder tests later

If a single fix needs more than one regression:

- use the minimum number of assignment-local tests needed to make the behavior durable
- record each finalized path in the tracker row
- it is normal for one `HHC-###` item to close out into multiple assignment-local tests

If final ownership is still not clear yet:

- place the reduced regression in `tests/frontier/` under the `paN` whose binary is being
  used to validate the current frontier
- keep the `HHC-###` identifier in the temporary filename so it is easy to move later
- treat that frontier regression as temporary holding coverage, not as the final assignment
  placement
- once ownership is known, move it into the proper numbered location in the proper `paN`

### 5a. LowIR-Surface Rule

If the implementation fix changes LowIR itself:

- add direct tests in the earliest LowIR-owning assignment that can observe the change
- update the corresponding handout/spec (`README.md`, `lowir.md`, grammar, or equivalent)
- validate that assignment locally before resuming the hosted smoke chain

Hosted frontier work is allowed to discover LowIR gaps, but it is not allowed to patch over
them silently in `dev/` while leaving the LowIR assignment contract stale.

### 6. Validate

Required validation:

- run `make test` in the owning `paN`

If the item is still in temporary frontier placement:

- run the frontier-capable test target in that current `paN`
- do not leave a temporary frontier regression unvalidated

Recommended additional validation when relevant:

- rerun the active hosted smoke that discovered the issue
- rerun a broader aggregate target if the blast radius is wide
- compare archived old/current binaries only when that evidence is still part of the story
- rerun the batch hosted-header sweep before choosing the next representative blocker when
  the current fix advances the frontier or closes a recurring-family slice

For repo-level verification, use the root `test-report` target when you are doing developer
validation after rebuilding `dev/`:

```sh
make test-report CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Notes:

- `test-report` rebuilds `dev/` once first, then fans out assignment suites in parallel and
  prints every failing assignment/test instead of stopping at the first failure
- the per-assignment harness already supports per-test parallelism via `CPGM_TEST_JOBS`, and
  `test-report` feeds that through as `TEST_REPORT_SUBTEST_JOBS`
- the current root defaults use nested parallelism already:
  - later assignments start first
  - per-assignment subtest jobs default to `2`
  - top-level assignment jobs default to roughly `nproc / 2`
- override those knobs only when you intentionally need a different local tradeoff

Performance sanity check:

- when the fix touches parser, semantic, overload-resolution, template, fragment-reparse, or
  other known hot-path code, add one small compile-time sanity probe to the validation block
- default probe:

```sh
make build CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
/usr/bin/time -p env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  ./dev/cppgm++ -c dev/src/template_audit.cpp -o /tmp/template_audit.perf.o
```

- allowed substitute: the current reduced perf reproducer documented in an active hotspot note
  when it is narrower and more representative than `template_audit.cpp`
- use the same `/usr/local/opt/llvm/bin/clang++` host toolchain as the active frontier work
- compare against the last known good baseline for that probe, not just a single noisy run
- treat obvious multi-second jumps or roughly `15%+` regressions as validation failures unless
  the change is intentionally a temporary diagnostics/profiling step
- record the probe and the before/after timing in the tracker row whenever the perf sanity check
  materially influenced the keep/reject decision

### 7. Record Closure

When the work is done, update the active hosted frontier tracker with:

- final status
- final owning assignment
- finalized regression path(s)
- validation result
- any follow-on item created by splitting the original failure
- parent/child unwind notes if the item was closed as part of a root-cause stack

The default expectation is:

- each completed fix leaves behind its owning regression set before frontier work continues

### 8. Commit Before Moving On

Use this exact ordering:

1. fix the current frontier item
2. place the owning regression(s)
3. validate the owning suite(s) and confirm the parent hosted smoke advances
4. update the tracker row for the item you just closed
5. commit the closure batch
6. only then start reducing or exploring the next hosted blocker

This ordering is mandatory. Do not leave a just-closed frontier batch uncommitted while
starting broader reduction work on the next blocker.

This applies equally to stacked child items. A lower-layer fix must be committed before
returning to its parent frontier item.

Before starting the next hosted frontier item:

- stage and commit the `dev/` implementation change in the main repo
- stage and commit any owning `paN` regression additions or handout/spec updates in the same main-repo history
- commit the top-level tracker/process updates in the main repo
- run git from the repository root when committing
- prefer setting the command working directory to the repository root
- shell examples may use `git ...` from the repo root, but avoid `git -C ...`

Do not leave a completed hosted frontier fix only in the worktree while continuing to the
next blocker. The frontier should always be resumable from committed state.

In particular:

- do not mix exploratory next-blocker reductions into the same worktree as a completed item
- do not let adjacent semantic cleanup discovered while advancing the parent smoke block the
  closure commit for the current item
- if the parent smoke advances into a new area, stop after confirming the advancement, commit
  the current closure batch, and then begin the next reduction from that committed baseline

## Tracker Row Model

Each tracker row should carry the merged information we used to keep separately
in the old tracker and ledger. Historical closed rows live in
[`legacy/HOSTED_HEADER_FRONTIER_TRACKER.md`](/Users/vishvananda/cppgm/legacy/HOSTED_HEADER_FRONTIER_TRACKER.md).

- `ID`
- `Status`
- `Frontier / Issue`
- `Minimal Repro / Smoke`
- `Owning PA`
- `Regression PA`
- `Frontier Regression`
- `Final Regressions`
- `Validation`
- `Notes`

That is enough to answer:

- what broke
- where we are in the frontier
- who owns the fix
- whether the item was discovered as a child in a deeper root-cause stack
- whether there is a temporary frontier regression holding the case while ownership is being
  resolved
- whether the regression has been placed
- whether one frontier item closed out into multiple durable tests
- whether the work is actually closed

## Migration Rule

The old hosted docs are now legacy records.

Use them to recover historical context, but do not add new frontier bookkeeping there.
All new hosted-header discovery and regression placement should go through:

- `HOSTED_HEADER_FRONTIER_PROCESS.md`
- an active hosted frontier tracker, created or promoted before new rows are added
