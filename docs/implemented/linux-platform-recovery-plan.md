# Linux Platform Recovery Plan

## Purpose

This plan restores Linux as a first-class development and validation platform
for the active compiler repository.

The goal is broader than one failing test or one backend feature. "Linux works"
should mean:

- the repository builds under the supported Linux host toolchain
- the relevant assignment suites can run under Linux in batch mode
- direct driver and backend validation can use Linux/ELF without ad hoc local
  patches
- host-runtime and host-ABI behavior is split cleanly into:
  - generally portable hosted build/runtime support
  - the still-separate Linux/ELF host-EH metadata work

This plan is intentionally platform-recovery work, not a new language or
backend milestone. It exists so ongoing PA23 / PA37 / export work can validate
against Linux again without rediscovering the same environment and ABI failures.

## Why This Needs Its Own Plan

Recent Linux validation attempts exposed that the current gaps are not one
thing:

1. There is an immediate hosted build portability failure.
   Current Linux Clang 22 builds fail in
   [host_builtin_runtime.cpp](/Users/vishvananda/cppgm/dev/src/host_builtin_runtime.cpp)
   because manually redeclared libc functions such as `memchr(...)` conflict
   with the system headers on glibc-based Linux.

2. There is still Linux/ELF-specific host-ABI debt.
   The deferred tracker already records that host-EH metadata/object emission is
   only implemented for the current Mach-O path:
   [deferred-issues-tracker.md](/Users/vishvananda/cppgm/docs/deferred-issues-tracker.md).

3. Linux validation is not yet a routine maintained lane.
   We can run isolated Docker checks, but there is not yet a durable process
   for:
   - isolated Linux object dirs
   - repeatable Linux batch validation
   - platform-specific expected failures vs true regressions

If we treat each Linux failure as an isolated bug, we will keep fixing them
reactively. The better approach is to restore a general Linux lane and then let
individual plans depend on that lane.

## Scope

This plan owns:

- Linux-hosted build portability for the active `dev/` binaries
- Linux batch validation process and isolated object-dir conventions
- Linux/ELF object/link/runtime validation for ordinary hosted compilation
- cataloging and reducing Linux-specific breakages to earliest-owner tests
- integrating Linux checks into the maintainer workflow once the lane is stable

This plan does **not** by itself complete:

- the full Linux/ELF host-EH metadata path
- broader assignment cleanup
- PA23 machine-IR quality work
- PA37 self-host ladder growth

But it should make those efforts testable on Linux again.

## Current Progress

The first Linux recovery tranche has landed in the separate worktree and has
already re-established the basic lane:

- `host_builtin_runtime.cpp` now builds cleanly under Linux Clang 22 with
  glibc/libc++ headers.
- the repository has a durable Linux regression hook in
  [test_linux_platform_recovery.py](/Users/vishvananda/cppgm/scripts/tests/test_linux_platform_recovery.py)
  for that portability family.
- a documented isolated-object-dir Linux build of `dev/` now completes.
- the first stable Linux batch lane now passes in Docker using the same
  isolated object root:
  - `pa1` through `pa9`
  - `pa31`
- the next hosted probe after `pa31`, `pa32`, now also passes in full on
  Linux:
  - `27 / 27` in batch mode under Linux Clang 22 with the isolated object root
  - ordinary weak-symbol / inspect-surface drift and the first ELF host-EH
    owner cluster (`191`, `192`, `193`, `194`, `220`) are fixed in this lane
- the hosted-source compatibility lane beyond `pa32` is back in the normal
  Linux batch path:
  - `pa33` now passes in full on Linux (`18 / 18` preproc, `139 / 139`
    compile)
  - the remaining issue there was a hidden batch-policy mismatch: even with
    `CPPGM_TEST_JOBS=1`, the compile lane still defaulted to three persistent
    host-compat workers
  - the compile-worker default now follows `CPPGM_TEST_JOBS` unless explicitly
    overridden, so low-contention Linux recovery runs behave as requested
- the next hosted link owner after `pa33`, `pa34`, now also passes in full on
  Linux:
  - `57 / 57` in batch mode under Linux Clang 22
  - the two real Linux fixes there were:
    - hosted runtime-support objects now come from the compiler binary's own
      built object root instead of assuming `../obj`
    - platform-specific Mach-O-only inspect scripts were replaced with generic
      normalized object-surface expectations that work on Mach-O and ELF
- the Linux lane now advances far enough to probe the first `pa37` self-host
  rung:
  - Linux host-control `pa37 test-through-pa1` passes
  - Linux self-host `pa37 test-through-pa1` now gets past the earlier driver
    and hosted-header blockers and exposes a real self-host runtime crash in
    the self-built `pptoken-self`
  - that current blocker is no longer a Linux portability bug; it belongs to
    the active `PA37` ladder-fix process

That means Phases 1 and 2 are no longer speculative, and the hosted recovery
lane is now advanced through `pa34` and into the first `pa37` self-host
checkpoint. The remaining Linux-plan work is to keep that lane documented and
routine, not to absorb the newly exposed cross-platform self-host bug.

## Current Known Linux Gaps

### Immediate Build Portability Failures

- No currently known build-stopper remains at the old `host_builtin_runtime.cpp`
  glibc boundary.
- New Linux build failures should be treated as fresh portability regressions,
  not as fallout from the original manual-libc-declaration issue.

### Hosted Environment / Toolchain Drift

- Linux support currently depends heavily on containerized toolchains rather
  than a maintained always-green host lane.
- Some preprocessor/include-path logic still contains old hard-coded Linux paths
  as fallback values in
  [preprocessor.cpp](/Users/vishvananda/cppgm/dev/src/preprocessor.cpp).

### ELF / Host-ABI Gaps

- Linux/ELF host-EH now has a real self-emitted `.eh_frame` / LSDA path for the
  current `pa32` owner set, and the ordinary hosted compile/link lane now
  advances through `pa34`, but later hosted/self-host Linux probes still need
  to be checked for:
  - deeper ELF host-EH metadata gaps
  - later vtable / RTTI / relocation-owner drift
  - Linux-only hosted runtime behavior beyond the `pa34` surface

### Validation / Process Gaps

- Linux checks are still not yet part of the routine maintainer checkpoint
  flow.
- We need to keep using isolated-object-dir validation so Linux runs do not
  perturb the main macOS build tree.
- Later hosted probes must preserve the same low-contention batch semantics
  when `CPPGM_TEST_JOBS=1`, rather than quietly reintroducing hidden worker
  fanout.
- The current Linux `pa37` blocker after `pa34` is a real self-host runtime
  failure in the rebuilt `pptoken-self` binary, not another Linux portability
  issue. That bug should be fixed under the `PA37` ladder process while this
  plan keeps the Linux validation lane available.

## Plan Structure

### Phase 1: Restore Basic Linux Hosted Builds

Goal:

- `dev/` binaries build again under the supported Linux Clang toolchain in a
  clean isolated object tree.

Initial tasks:

1. Remove or narrow manual libc/libm declarations in
   [host_builtin_runtime.cpp](/Users/vishvananda/cppgm/dev/src/host_builtin_runtime.cpp)
   so the file compiles cleanly under Linux headers.
2. Audit the rest of that file for glibc/libc++ signature drift, not just the
   first `memchr(...)` failure.
3. Add the earliest owner regression for this portability family.
   The regression should be small and should prove that the wrapper-runtime
   source itself builds on Linux, not only that a later suite incidentally gets
   past it.

Exit criteria:

- clean Linux Clang build of the relevant `dev/` target set in Docker using an
  isolated object root

### Phase 2: Re-establish Linux Batch Validation

Goal:

- the repository has a documented, repeatable Linux validation lane that does
  not interfere with the main object tree.

Tasks:

1. Standardize Linux validation on isolated object roots, for example:
   - `/tmp/<name>-linux-obj`
2. Define the first stable Linux validation commands.
   Start with:
   - targeted `dev/` build checks
   - targeted assignment suites already expected to run on Linux
3. Document the Docker invocation pattern, including the current API-version
   pin needed on this host if it remains necessary.
4. Make sure ignored/generated outputs from Linux runs do not dirty the worktree
   beyond normal ignored `.my*` and `obj/` state.

Exit criteria:

- one documented Linux batch path can be rerun by maintainers without bespoke
  cleanup

### Phase 3: Recover Ordinary Linux Hosted Compilation/Linking

Goal:

- ordinary hosted Linux compilation and linking work again for the current
  non-EH path.

Tasks:

1. Identify the earliest-owner Linux failures after the repository builds.
2. Reduce them to durable tests in the earliest owning assignment rather than
   only broad Docker runs.
3. Fix ordinary ELF/runtime issues that are independent of host EH metadata.
   Examples may include:
   - ELF relocation/import handling
   - Linux-specific runtime symbol wiring
   - hosted object parsing/writing drift
4. Keep Linux batch-policy defaults honest when the caller explicitly asks for
   low-contention validation.

- targeted Linux hosted compile/link smokes pass without local workarounds,
  and the lane is advanced at least through the earliest post-`pa34` hosted
  owner suite

### Phase 4: Split Out Linux/ELF Host-EH Work Explicitly

Goal:

- keep Linux recovery honest by separating "ordinary Linux works again" from
  later Linux/ELF hosted-runtime work that may still appear beyond the current
  `pa32` lane.

Tasks:

1. Probe the next Linux hosted owner after `pa34` and reduce any new failures to
   earliest-owner tests.
2. Split broad Linux portability regressions from backend- or ABI-specific
   follow-on plans when the failures are no longer general lane-recovery work.
3. Keep this plan responsible for:
   - exposing the blocker
   - making sure the Linux validation lane shows it clearly
   not for silently folding all later Linux-only backend work into a generic
   portability patch.

Exit criteria:

- Linux recovery no longer depends on hidden Mach-O-only assumptions
- remaining Linux EH work is tracked explicitly rather than rediscovered

### Phase 5: Integrate Linux Back Into Normal Maintainer Workflow

Goal:

- Linux is again a routine validation target, not a special expedition.

Tasks:

1. Decide the durable minimum Linux checks to run at checkpoints.
2. Document when Linux checks are required for:
   - backend/object changes
   - host-runtime changes
   - assignment export changes
3. Keep the lane lightweight enough that it can actually be used.

Likely steady-state shape:

- small Linux build canary
- a focused Linux assignment batch
- broader Linux validation only at major checkpoints

## Testing Strategy

Validation should proceed from narrowest to broadest:

1. file-level / target-level Linux build probes
2. smallest owner regression that reproduces the issue
3. focused assignment suites in Docker with isolated `OBJ=...`
4. only then broader Linux sweeps

Important rule:

- do not rely on the default shared `../obj` tree for Linux validation
- always use an isolated Linux object root in containers or separate Linux
  worktrees

## Next Concrete Execution Slice

The next implementation slice from this plan should be:

1. keep the now-working Docker lane explicit and documented:
   - Linux Clang 22
   - `DOCKER_API_VERSION=1.52` on this host
   - isolated `OBJ=/tmp/<name>-linux-obj`
2. keep the new Linux `pa37` probe in the documented lane so future self-host
   fixes can validate there without rediscovering the earlier portability bugs
3. keep genuine Linux portability / ELF issues separate from the newly exposed
   cross-platform `PA37` self-host failures

That next slice is intentionally chosen because it:

- builds on a stable Linux build plus batch-validation base
- can keep landing from a separate worktree with modest merge risk
- keeps the Linux lane useful for `PA37` without turning this plan into the
  owner of non-Linux self-host bugs

## Relation To Other Active Plans

- [pa37-selfhost-buildout-process.md](/Users/vishvananda/cppgm/docs/implemented/pa37-selfhost-buildout-process.md)
  remains the primary active implementation lane.
  Linux recovery is a parallel enabling lane for cross-platform validation.

- [pa23-machine-ir-quality-plan.md](/Users/vishvananda/cppgm/docs/implemented/pa23-machine-ir-quality-plan.md)
  should not absorb general Linux-recovery work.
  That plan should be able to assume Linux validation exists once this plan
  lands its early phases.

- [student-assignment-export-process.md](/Users/vishvananda/cppgm/docs/student-assignment-export-process.md)
  should not export new Linux-facing workflow assumptions until this plan has
  restored a stable maintainer-side Linux lane.
