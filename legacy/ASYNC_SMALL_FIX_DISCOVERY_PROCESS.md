# Async Small-Fix Discovery Process

This document defines the workflow for parallel small-fix discovery work that is intentionally
independent of the active hosted-header or bootstrap frontier trackers.

Use this process when you want to mine a broader backlog of compiler-source or hosted failures,
cluster them by shared root cause, and close one small high-confidence fix at a time in separate
worktrees.

Do not use this document to manage the active serial frontier itself. For that, keep using:

- `HOSTED_HEADER_FRONTIER_PROCESS.md`
- `BOOTSTRAP_SELFHOST_FRONTIER_PROCESS.md`

## Goal

Turn broad discovery output into:

- one independent worktree per candidate fix
- one real implementation change in `dev/`
- one correctly owned assignment-local regression in the earliest real `paN`
- one standalone result note written into the main checkout under
  `async-small-fix-discovery/`
- one committed closure batch that can be revalidated and merged independently later

The point of this process is to let several small-fix investigations proceed in parallel without
multiple branches fighting over a shared mutable tracker row.

## Core Rules

1. Start each investigation from a fresh worktree at `HEAD`.
2. Use one worktree for one candidate fix. Do not stack unrelated fixes in the same worktree.
3. Prefer seeding each fresh async worktree's `obj/` directory from a clean validated same-`HEAD`
   seed worktree instead of rebuilding every object file from scratch.
4. Freeze the compiler binary used for discovery into `/tmp` before running long batch probes, so
   the batch does not observe a moving target while you rebuild in the worktree.
5. Pin both `CXX` and `CPPGM_HOST_CXX` to `/usr/local/opt/llvm/bin/clang++` unless the discovery
   surface explicitly requires a different host toolchain.
6. For compiler-source batch discovery, prefer smallest-to-largest source order and a hard 30s
   timeout per file.
7. Record timeouts distinctly from ordinary failures.
8. Cluster by normalized first failure or shared root cause, not by raw file list order.
9. Before choosing a candidate, check `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md` in the main
   checkout. If a discovered family matches the current active bootstrap frontier, the top child
   item on its root-cause stack, or an explicitly recorded next bootstrap blocker, skip it and
   choose a different family.
10. Also skip any async family that already has an unmerged standalone note under
    `async-small-fix-discovery/` with a closure commit that is not yet reachable from current
    `HEAD`.
11. Pick fixes that look small, local, and high-confidence. Do not turn this process into an
   open-ended frontier branch.
12. Put the real regression in the earliest real owning milestone. Do not create a shared draft
   regression area.
13. Standard validation for a closed item is:
    - the direct reduced repro or representative source file(s)
    - one repo-level gate:
      `make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ TEST_REPORT_ASSIGNMENT_JOBS=6 TEST_REPORT_SUBTEST_JOBS=1`
    - rerun the owning assignment-local suite separately only if:
      - it gives faster iteration while reducing the item
14. Write results to a unique standalone note in the main checkout under
    `async-small-fix-discovery/`, not to a shared tracker file, and do not commit that note from
    the main checkout.
15. A committed async closure is a candidate import, not an automatically merge-ready patch.
    The active hosted/bootstrap serial process must still revalidate it on current `HEAD` before
    merging it.
16. Commit the worktree once the item is validated and documented, then stop. Do not begin the
    next candidate in that same worktree.
17. Outside intentionally frozen `/tmp/cppgm++-<slug>` binaries, use the root build-first runner
    for one-off direct probes:
    `make run-cppgm CPPGM_ARGS='...'`

## 1. Create A Fresh Worktree

Use a unique tmp worktree rooted at `HEAD`:

```sh
git worktree add -b async-<slug>-$(date +%Y%m%d) /tmp/cppgm-<slug>-$(date +%Y%m%d) HEAD
cd /tmp/cppgm-<slug>-$(date +%Y%m%d)
```

The worktree should own:

- the implementation fix
- the owning regression

Do not reuse a dirty worktree from a previous async item.
The result note lives in the main checkout under `async-small-fix-discovery/`, not inside the
tmp worktree.

## 1.5 Seed The Build Outputs

Do one cold build per `HEAD`, not one cold build per async branch.

Maintain one clean seed worktree at current `HEAD` whose `obj/` tree has already been validated.
That can be:

- a dedicated seed worktree created only for this purpose, or
- another clean async worktree at the same `HEAD` after its validation completed

Before the first build in a fresh async worktree, copy that `obj/` tree in:

```sh
scripts/seed_async_worktree_obj.sh /tmp/cppgm-async-seed-<date> /tmp/cppgm-<slug>-<date>
```

Then rebuild only the narrow `dev/` target set you need in the new worktree, for example:

```sh
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
```

Rules for seeding:

- only seed from a source worktree at the exact same `HEAD`
- prefer a source worktree that is clean and already passed the intended validation gate
- if `HEAD` changes, rebuild the seed worktree once and start seeding from that new `HEAD`
- do not seed from a dirty source checkout whose built objects may not match tracked source
- after copying, refresh the destination `obj/` mtimes so a fresh worktree checkout does not make
  every seeded object look stale immediately

## 2. Freeze The Discovery Environment

Before running a long discovery batch:

1. seed `obj/` from a clean same-`HEAD` seed worktree when available
2. rebuild the compiler binary you want to evaluate in the new worktree
3. copy that binary to `/tmp`
4. pin the host toolchain environment

Example:

```sh
export CXX=/usr/local/opt/llvm/bin/clang++
export CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
scripts/seed_async_worktree_obj.sh /tmp/cppgm-async-seed-$(date +%Y%m%d) .
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
cp ./dev/cppgm++ /tmp/cppgm++-<slug>-$(date +%Y%m%d-%H%M%S)
```

If the investigation is about an earlier text-output assignment mode, freeze the same `cppgm++`
binary and record the required `--emit-*` flag alongside the probe.

Do not trust a discovery report if it was run against a binary that may have changed mid-flight or
against a host-header/toolchain configuration that does not match the active intended environment.

## 3. Run General Discovery

The discovery step here is intentionally broader than the serial frontier docs. The aim is to find
shared challenges across many files, then choose one promising small fix.

For compiler-source build discovery, the default shape is:

- order files from smallest to largest
- compile one translation unit at a time
- use a hard 30s timeout per file
- record `pass`, `fail`, and `timeout` separately
- normalize the first visible failure for clustering

Representative direct bootstrap-style probe:

```sh
/tmp/cppgm++-<slug> -c -I dev/src -o /tmp/<stem>.o dev/src/<file>.cpp
```

For hosted-header discovery, use the relevant hosted sweep or a narrower direct hosted compile, but
keep the same rules:

- frozen binary
- pinned host toolchain
- clear timeout handling when needed
- clustering by shared failure family

If an existing script does not let you reproduce the correct flags, host toolchain, or timeout
policy, do not trust its defaults. Run the direct probe shape that matches the real environment.

Before you pick a cluster to reduce, read `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md` in the main
checkout and identify the current active bootstrap item. If the tracker already shows the same
failure family, the same top child item, or the same next blocker you were about to take, skip
that family here and keep looking for a different small fix.

Also scan the standalone note files under `async-small-fix-discovery/` and exclude any family
whose closure commit is not yet merged into current `HEAD`. Async discovery should not reopen a
still-outstanding async item in parallel under a different branch name.

## 4. Choose The Candidate

Prefer a candidate that has all of these properties:

- appears in multiple files or a clearly recurring family
- has a sharp first diagnostic
- reduces quickly
- has a clear earliest owning assignment
- looks like a true language or hosted-compatibility fix, not a broad refactor

Avoid choosing:

- long timeouts with no reduction yet
- compiler crashes with unclear ownership
- broad parser/semantic families that obviously need a larger closure program first
- the same family as the current or next bootstrap item already recorded in
  `BOOTSTRAP_SELFHOST_FRONTIER_TRACKER.md`
- anything that would require carrying several unrelated fixes in one worktree

The output of discovery is not "fix the first failing file." The output is "pick one shared
challenge that looks simple enough to close cleanly."

## 5. Reduce And Fix

Reduce the chosen family only far enough to answer:

- what feature is actually missing or wrong?
- what is the earliest real owner?
- what reduced repro separates broken from fixed?

Then implement the real owner-level fix in `dev/` and add the regression directly to the owning
`paN`.

If reduction shows the chosen family actually hides a different, larger, or less independent
problem than expected, stop treating it as an async small-fix item. Write down what you learned in
the worktree note and either:

- stop without landing a fix, or
- spin a new worktree under the more appropriate process

Do not silently let a "small fix" worktree grow into a long-lived mixed branch.

## 6. Validate

Every landed async small-fix item must run:

- the direct reduced repro or representative discovery file(s)
- `make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ TEST_REPORT_ASSIGNMENT_JOBS=6 TEST_REPORT_SUBTEST_JOBS=1`

Also rerun enough of the original discovery slice to prove the chosen family actually moved. For a
clustered compiler-source issue, that usually means:

- the original representative file
- at least one or two sibling files from the same cluster when available

You do not need to rerun a full expensive world-sweep if a smaller targeted rerun proves the
claimed family movement and the standard verification passes.

## 7. Record Results In A Standalone Note

Do not update a shared tracker row for this process.

Instead, create one unique note in the main checkout under `async-small-fix-discovery/` using the
template in `async-small-fix-discovery/TEMPLATE.md`. Do not keep the only copy of the note in the
tmp worktree.

Use a unique filename so parallel branches do not collide, for example:

- `async-small-fix-discovery/2026-03-25-string-comparison.md`
- `async-small-fix-discovery/2026-03-25-private-constructor-friend.md`

Each note should stand on its own and include:

- the worktree path and branch
- the `obj/` seed source, or an explicit note that the worktree was built cold
- the required `dev/` build target(s) for later serial intake revalidation
- whether later serial intake needs a bootstrap smoke or only the parent reduced repro
- the frozen binary and pinned toolchain used for discovery
- the discovery command(s)
- the clustered family and representative files
- the chosen candidate and owning assignment
- the reduced repro
- the validation commands and results
- the final commit hash, or the reason the item was abandoned
- the next visible blocker if the fix advanced the original family

The note is the durable artifact that makes the work mergeable and reviewable without a shared
mutable queue. Leave it uncommitted in the main checkout so multiple async investigations can add
their own notes independently.

## 8. Commit And Stop

Use this exact ordering:

1. implement the fix
2. add the owning regression
3. run the direct repro
4. run `make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ TEST_REPORT_ASSIGNMENT_JOBS=6 TEST_REPORT_SUBTEST_JOBS=1`
5. run the owning suite too only if you used it as a faster local reduction gate
6. rerun the relevant discovery slice to confirm the family moved
7. write the standalone result note into the main checkout under `async-small-fix-discovery/`
8. leave that main-checkout note uncommitted there
9. commit the closure batch in the tmp worktree
10. stop

Do not begin reducing the next async candidate in the same worktree after the commit. Open a new
tmp worktree from fresh `HEAD` instead.

## 9. Import Into The Serial Frontier

Async discovery does not replace the active hosted/bootstrap frontier process. The serial frontier
owns the decision to merge an async closure.

When an async note looks relevant to the current serial blocker, prefer warm staged validation in
the active main checkout:

1. apply the async closure on current `HEAD`, typically with `git cherry-pick -n <commit>`
2. stage only the imported files
3. stash unrelated live work with `git stash push --keep-index -u`
4. run one incremental `make -C dev CXX=/usr/local/opt/llvm/bin/clang++`, or the note's required
   narrower `dev/` build target(s) if the note records them
5. rerun the note's direct repro or representative file(s)
6. rerun `make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ TEST_REPORT_ASSIGNMENT_JOBS=6 TEST_REPORT_SUBTEST_JOBS=1`
7. rerun the active hosted smoke or `make bootstrap-frontier-nobuild`, whichever serial process
   owns the current blocker
8. commit the import if it passes, then restore the stashed live work

Do not rerun a separate owning assignment-local suite during serial intake unless you are using
it as a faster local iteration step before the full `test-report` gate.

If that staged fast path is unsafe or unreliable, fall back to the persistent clean async-intake
worktree and run the same gate there.

Only merge the async closure into the active branch if that current-`HEAD` revalidation passes.
If it fails, keep the note as useful prior art, but do not merge it blindly and do not treat the
recorded async commit as still-closed work until it has been refreshed against current `HEAD`.

That keeps each candidate independently mergeable and avoids accidental cross-coupling between
"easy" fixes.
