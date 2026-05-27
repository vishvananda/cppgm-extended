# PA38 Self-Host Buildout Process

Archive note: this buildout is complete. Current self-host validation is through
`pa38` `test-through-*` targets and the root `test-report-pa38` wrapper. The
older root bootstrap/frontier targets described by related process docs have
been retired.

## Purpose

This document is a short supplement to
[ASSIGNMENT_BUILDOUT_PROCESS.md](/Users/vishvananda/cppgm/legacy/ASSIGNMENT_BUILDOUT_PROCESS.md)
for building out `PA38`.

`PA38` is different from the earlier assignments because the main goal is no
longer "add one more isolated compiler feature." The goal is:

- build the compiler with the student's own compiler
- produce a working self-built compiler binary
- prove that the self-built compiler can still satisfy the earlier assignment
  contracts

So `PA38` is a self-hosting and regression-preservation milestone, not just a
new syntax/semantic/backend milestone.

The first public part of that milestone should now be the early-frontend
self-host loop that we originally developed as a maintainer-only fast filter.
The initial public ladder should stop at `pa9`. We can extend that later if
the assignment contract benefits from a broader staged ladder, but the first
version should stay bounded and understandable.

## Primary Difference From Normal PA Buildout

For a normal post-PA9 assignment, the contract is usually:

- new owned feature set
- new primary oracle at the first owned boundary
- assignment-local tests proving that new boundary

For `PA38`, the contract is instead:

- self-build smaller earlier frontend binaries first
- self-build `cppgm++`
- use the self-built compiler in a meaningful validation loop
- show that earlier assignment behavior still works through that self-built
  compiler

That changes both the README shape and the test strategy.

## Core Contract For PA38

Before building tests, freeze the `PA38` contract explicitly.

The contract should say, in plain terms:

1. what counts as a successful self-host build
2. which binary is considered the self-built compiler artifact
3. which smaller earlier frontend checkpoints must be rebuilt first through the
   student's compiler
4. which earlier behaviors must still pass when exercised through those
   self-built binaries and the final self-built compiler
5. what is required for the public assignment and what remains undefined or
   intentionally unsupported

The README should avoid pretending that `PA38` is just "make everything work."
It should define a concrete required validation surface.

The contract should also be explicit that the student-facing build path uses
named source-file lists, not only opaque convenience targets. In other words,
the default PA38 build flow should require students to pass the relevant `.cpp`
files to their compiler for each checkpoint binary. That is a little more
demanding, but it is a better self-hosting contract because it proves source
compilation directly rather than hiding behind prewired target names.

For the public assignment shape, those checkpoint source lists should live
directly in the PA38 `Makefile`. They should not depend on generated manifests
or automatic link-set discovery in the student-facing path.

## Oracle For PA38

The primary oracle for `PA38` is not a text dump. It is a staged
self-host-validation result.

Recommended oracle ladder:

1. selected early frontend binaries (`pa1` through `pa9` checkpoints) can be
   rebuilt from explicit `.cpp` source lists with the self-built compiler
2. those rebuilt early binaries still satisfy their assignment contracts
3. self-host compile succeeds
4. self-host link succeeds
5. the self-built compiler can compile a tiny smoke program
6. selected earlier owned test suites still pass when driven by the self-built
   compiler

That means `PA38` should use executable/toolchain validation as the primary
contract, unlike most earlier PAs.

## Test Surface Design

`PA38` tests should be organized in layers.

### 1. Early-Frontend Self-Host Ladder

The first phase of `PA38` should be the `pa1` through `pa9` early self-host
work.

That means:

- rebuild the earlier frontend binaries through the student's compiler
- use explicit source-file lists for those binaries instead of only hidden
  target names
- rerun the existing earlier assignment suites through those rebuilt binaries

This should be the first public proof surface because it is:

- much cheaper than full `cppgm++` self-hosting
- broad enough to expose real runtime/codegen problems
- already aligned with earlier assignment behavior students understand

The important contract changes are:

- the default student-facing build flow should require specifying `.cpp` inputs
  for the checkpoint binaries
- the checkpoint source lists should be written directly in the PA38
  `Makefile`
- generated fast-link manifests and helper scripts can still exist as
  maintainer-only seeding/debugging aids, but not as the public build contract

### 2. Bootstrap Smoke

Smallest possible checks that prove:

- the compiler can build itself far enough to produce a runnable artifact
- the self-built artifact can compile at least one tiny program

These should be fast and should fail with clear stage ownership:

- compile failed
- link failed
- self-built smoke compile failed
- self-built smoke run failed

### 3. Earlier-PA Preservation Suite

`PA38` should then select a defined set of earlier tests that are rerun through
the self-built compiler.

This suite should not be "all tests, always" as the only public assignment
contract. It should be:

- a clearly documented required subset
- chosen to represent the important earlier boundaries
- stable enough that students know what they are aiming for

The full maintainer bootstrap sweep can remain broader than the student-visible
assignment contract.

### 4. Checkpoint Suite

The maintainer buildout for `PA38` should still keep a larger checkpoint gate:

- broader self-host frontier checks
- broader earlier-assignment regression checks
- final full validation

But that larger gate should be treated as maintainer validation, not
necessarily the first public student-facing oracle.

## README Guidance For PA38

`PA38` will likely need more guidance than a typical earlier assignment, but
that guidance should stay general rather than implementation-specific.

The README should include:

- the self-host goal
- the early-frontend self-host ladder
- the required validation steps
- the expected failure stages
- a short debugging strategy section

It should also explain why the default build requires explicit source-file
lists for the checkpoint binaries:

- to keep the self-host contract concrete
- to avoid hiding the real compile surface behind convenience target names
- to make it obvious which source set a student is actually proving

It should also explain that the first public ladder is intentionally limited to
`pa1` through `pa9` for the initial buildout, even though the longer-term
staged validation ladder could extend farther later.

That debugging section should describe general methods such as:

- reduce the failing source
- determine whether the failure is compile, link, or runtime
- move the bug to the earliest real owner conceptually
- add a smaller regression before fixing the larger self-host case

It should **not** describe maintainer-specific file names, hidden bugs, or the
exact fixes in our implementation.

## Unique Guidance To Include For Students

`PA38` is the first assignment where students will likely hit bugs in earlier
subsystems while trying to satisfy one top-level milestone.

So the assignment should explicitly tell them:

- self-hosting failures often reduce to earlier language, lowering, or runtime
  bugs
- they should reduce and fix the smallest real underlying issue first
- they should keep adding smaller regressions as they discover those issues
- they should not treat the first top-level bootstrap failure as the only bug

This is different from earlier PAs, where the lesson boundary is usually much
narrower and more local.

## Public Contract Versus Maintainer Reality

The public assignment should not rely on hidden maintainer knowledge.

That means:

- the published PA38 test surface must be achievable from the shipped contract
- hidden maintainer validation can still be broader
- maintainer-only bootstrap trackers and frontier workflows are support tools,
  not the public assignment spec

The old maintainer process notes are archived under `legacy/`:

- [bootstrap-pa-fast-process.md](../../legacy/bootstrap-pa-fast-process.md)
- [pa38-ladder-fix-process.md](../../legacy/pa38-ladder-fix-process.md)

The public PA38 contract can absorb the idea of the PA-fast ladder without
exposing the internal helper script names directly, and without depending on
generated fast-target plumbing in the student-facing path.

The current maintained execution surface is the `pa38/Makefile`
`test-through-*` family.

## Buildout Sequence

Recommended `PA38` buildout order:

1. freeze the PA38 README contract and required validation surface
2. define the `pa1` through `pa9` early-frontend self-host ladder as the first
   public validation phase
3. switch the public default build contract to explicit source-file lists for
   those checkpoint binaries, written directly in the PA38 `Makefile`
4. define the smallest bootstrap smoke tests for full `cppgm++`
5. define the earlier-PA preservation subset for the public contract
6. build the maintainer-only broader self-host validation gate
7. add assignment-local regressions for every real bug found while closing the
   public contract
8. only after the public contract is stable, expand the broader checkpoint
   validation

## Regression Placement Rule

When `PA38` self-host work finds a real earlier bug:

- put the durable regression in the earliest real owning `paN`
- keep only the self-host integration proof in `PA38`

`PA38` should own:

- self-host integration
- self-built compiler validation
- cross-stage preservation proofs

It should not become the dumping ground for all earlier missing regressions.

## Completion Criteria

`PA38` is ready when:

- the README defines a clear self-hosting contract
- the assignment has a documented `pa1` through `pa9` early-frontend self-host
  phase
- the public build contract uses explicit source-file lists for the checkpoint
  binaries, written directly in the `Makefile`, rather than relying on opaque
  target names or generated manifests
- the assignment has a fast bootstrap smoke
- the assignment has a documented earlier-PA preservation subset
- the public contract is achievable without maintainer-only knowledge
- real earlier bugs discovered during buildout have been moved backward into the
  correct owning PAs
- the broader maintainer self-host validation gate is green
