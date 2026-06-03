# Machine Backend O0 Quality Plan

## Purpose

This plan defines the missing backend-quality tranche that should have landed
 before the later `PA37` machine-IR optimization assignment.

The current backend already has:

- a real `LowIR -> machine IR -> native` path in `PA23`
- a later `PA37` assignment for explicit machine/backend `-O1` / `-O2` work

What is still missing is a disciplined baseline-quality pass for unoptimized
 native code generation.

That missing tranche matters because the current `-O0` backend is still too
 conservative in exactly the places that determine code shape:

- parameter home-slot handling
- temp register selection around call setup
- address formation
- redundant frame traffic during direct lowering

The result is that later `-O1` / `-O2` work is building on top of a baseline
 backend that is still heavier than an ordinary non-optimizing compiler.

This plan therefore treats backend `-O0` quality as a **PA23-owned** buildout,
 not as a `PA37` optimization slice.

## Goal

By the end of this plan, the emitted code quality for the targeted hot-path
 families should be equivalent in quality to Clang `-O0`.

Equivalent does **not** mean bit-identical assembly.

It means:

- no obviously avoidable stack traffic in ordinary helper-sized code
- no avoidable callee-saved preservation caused only by backend policy
- no gratuitous temp materialization for simple address/index work
- helper-sized functions and hot-path owners with instruction counts in-family
  with Clang `-O0`

The concrete acceptance bar is:

- the `PA23` structural MIR owners express the intended shape
- the minimal hot helper repros are in-family with Clang `-O0`
- `ctrlexpr` materially improves at `-O0`, and the hot helper disassembly no
  longer shows the current backend-specific excess traffic

## Current State

As of `2026-04-19`, this plan is partway complete.

The currently landed `PA23`-owned backend slices on `codex/debug-info` are:

- `178a6c3f` `Fix elided direct-branch temp reloads`
- `dc3f2906` `Drop dead scalar call-result materialization`
- `2557a822` `Skip single-use call-setup preserves`
- `06299d80` `Keep direct call-index bases live through call setup`
- `25ab8008` `Forward read-only scalar params in registers`
- `11130b9f` `Iterate O0 mov artifact cleanup to fixed point`
- `d51edf7f` `Iterate O0 mov artifact cleanup further`
- `3e25b665` `Fix O2 edge liveness for forwarded params`
- `b9f8e371` `Fix machine_ir cleanup liveness and clobbers`
- `c9264bdc` `Fix indirect-call index base liveness`
- `7aee3095` `Load global pointer call targets by value`

The extra caller-saved temp-pool experiment that widened allocation to `r10`
was tested and intentionally **not** kept. It improved some MIR shapes but
caused a real end-to-end regression in `ctrlexpr`, especially at `-O1/-O2`.
The current committed state keeps the correctness fixes but leaves the temp
pool at `r8/r9`.

The two most recent correctness slices closed real backend bugs that had been
masking the next stage of work:

- indirect-call index-base liveness:
  - minimal failure: copy construction of `std::unordered_set<int>` under
    self-host `-O2`
  - root cause: forwarded parameter bases used only through the direct-call
    index fast path were not participating in named-value liveness, so `%this`
    could be left in a caller-clobbered temp across an earlier call
  - owner test: `pa23/tests/structural/858-indirect-call-reference-preserve.t`
- global pointer call targets by value:
  - minimal failure: `global @fp : ptr = addr @ret3`, then `%p = addr @fp`,
    then `call %p()`
  - root cause: indirect calls through temps defined as `addr @G` jumped to
    `&G` instead of loading the pointer stored in scalar `ptr` global `G`
  - owner test: `pa23/tests/structural/859-global-pointer-address-call.t`

The latest benchmark split from the current committed state is:

- self-host `cppgm++`:
  - `O0`: `20.55s`
  - `O1`: `19.10s`
  - `O2`: `18.45s`
- host `clang++`:
  - `O0`: `12.41s`
  - `O1`: `2.29s`
  - `O2`: `2.27s`

That means the backend-quality work has improved the old baseline materially,
but the remaining gap is still large:

- self `O2` is still slower than host `O0`
- the dominant remaining difference is no longer a single hidden backend
  correctness bug
- the dominant remaining difference versus Clang is still the `-O1` step,
  where Clang eliminates tiny hot helper boundaries entirely

The clearest current evidence for that is still in `calculator.o`:

- host `-O1` no longer has `Calculator::get_result` in the hot object
- host `-O1` no longer has visible `CalcToken` copy/move assignment or dtor
  helpers in the hot object
- self `-O2` still does

So the current state of this plan is:

- baseline backend correctness is substantially better than when this plan
  started
- several `PA23` slices are complete and should be preserved
- there is still some remaining backend `O0` quality work available
- but the largest remaining performance gap is now at the boundary to the
  follow-up `O1` inlining / IPA plan

## Scope

This plan covers:

- `lowir2native` baseline lowering quality
- `cppgm++` object generation only insofar as it reuses the same backend
- `PA23` tests and README contract
- measurement against Clang `-O0`

This plan does **not** cover:

- `PA37` `-O1` / `-O2` optimization design
- inlining / IPA / scalar replacement across function boundaries
- whole-program optimization

Those belong to a later follow-up once the baseline backend is no longer doing
 obviously avoidable work.

## Ownership

### `PA23`

`PA23` owns this work because the feature is baseline backend quality at the
 direct `LowIR -> machine IR -> native` boundary.

The tests for these slices should therefore live in:

- `pa28/tests/structural/` for canonical MIR shape
- `pa28/tests/strict/` only when exact raw MIR is the real contract

`PA37` should continue to own only explicit machine/backend optimization levels
 on top of that baseline.

### `PA37`

`PA37` should not absorb these regressions unless a slice is specifically about
 machine-IR optimization after baseline lowering.

## Main Findings Driving The Plan

The current backend gap is not one isolated bug.

The key structural issues are:

1. Temp register allocation is too conservative.
   `used_in_call_setup` currently forces the same callee-saved policy as
   `live_across_call`, which creates unnecessary `rbx` / `r12` preservation.

2. Parameter home slots are treated as mandatory backend storage.
   That is often fine for debug-style dumps, but it is too expensive as a
   universal lowering rule for helper-sized functions that simply forward
   parameters.

3. Address formation is not destination-driven.
   Simple `index` and related pointer-address shapes still go through
   fixed-register `mov/add/store-temp` patterns that Clang `-O0` does not need.

4. The MIR cleanup layer is too small to recover from conservative lowering.
   The existing `machine_ir_optimizer` intentionally does only a narrow set of
   local cleanups. That means baseline code shape must improve earlier in the
   lowering path.

## Execution Strategy

Work this plan in narrow slices.

Each slice should include:

1. one concrete backend change
2. a focused `PA23` regression
3. any README contract updates needed for students
4. targeted validation
5. a standalone commit

Do **not** defer tests to the end.

If a slice exposes a lower-level backend bug, fix that underlying bug and keep
the higher-level improvement whenever the intended code-shape change is still
correct. Do **not** paper over that situation by simply backing the slice out.
The goal of this plan is to remove avoidable backend conservatism, not to
preserve earlier behaviour when that behaviour only hid a deeper defect.

## Slice Order

### Slice 1: Call-Setup Temp Policy

Stop treating call-setup-only temps as if they were live across calls.

Targeted change:

- refine temp interval policy in `lowir_machine_ir.cpp`
- keep true `live_across_call` values conservative
- stop needlessly forcing short-lived call-setup temps into callee-saved
  registers

Expected result:

- fewer `preserve rbx/r12/...` cases in helper-sized functions
- less entry/exit register traffic
- simpler call-adjacent MIR in `PA23`

Tests:

- `PA23` structural owner for direct-call pressure without true across-call
  liveness
- update the existing call-pressure owner if the new shape is the right one

Status:

- substantially complete
- landed through `2557a822`, `06299d80`, `3e25b665`, `b9f8e371`, and
  `c9264bdc`
- the widened `r10` temp-pool experiment is explicitly **not** part of the
  current plan state

### Slice 2: Destination-Driven Index/Address Lowering

Improve direct lowering for `index` and similar address-producing shapes.

Targeted change:

- reuse the destination temp register where safe
- prefer `lea` / direct address formation where the MIR can represent it
- avoid fixed `rax/rcx` plus store-back churn for trivial pointer arithmetic

Expected result:

- smaller helper/basic-block instruction counts
- better `PA23` pointer-index structural shape
- fewer artificial move chains before loads and stores

Tests:

- extend the `570-ptr-index-arithmetic` family
- add one small owner where the indexed value is consumed immediately by a load
  or call

Status:

- partially complete
- the `lea` path and several direct-index owners are already improved
- the indirect-call address/call-target corner cases discovered during this
  work are now covered by `858` and `859`
- broader destination-driven cleanup is still available if a later disassembly
  pass shows obvious remaining address churn

### Slice 3: Read-Only Parameter Forwarding Cleanup

Reduce needless frame traffic for simple read-only parameter forwarding.

Targeted change:

- eliminate or coalesce backend-only shadow traffic where the parameter already
  has a stable location and the lowering does not require an addressable copy

Expected result:

- helper-sized wrappers stop reloading params from `[rbp-...]` before every use
- `assign_move` / `assign_copy` style owners match Clang `-O0` much more
  closely

Tests:

- new `PA23` structural owner for wrapper-style forwarding
- verify no loss of address-taking correctness

Status:

- substantially complete
- landed through `25ab8008` and the later liveness/correctness follow-ups that
  made the forwarding path safe at `-O2`

### Slice 4: Small MIR Combiner For Baseline Lowering

Add a small post-lowering cleanup pass that is still appropriate for baseline
 backend quality.

Targeted change:

- fold trivial `load+mov`, `mov+add`, and direct-temp store-back sequences
- keep this as baseline cleanup, not an `-O1` optimizer

Expected result:

- cleanup of conservative lowering artifacts that are not semantically useful
- better code shape without needing a broader optimizing pipeline

Tests:

- `PA23` structural owners for each newly folded shape

Status:

- partially complete
- `dc3f2906`, `11130b9f`, and `d51edf7f` removed several obvious move/store
  artifacts
- further combiner work is still possible, but it is no longer the clearest
  explanation for the remaining `ctrlexpr` gap

### Slice 5: Revalidation And Gap Review

Once the above slices land:

- rerun the helper repros
- rerun `ctrlexpr` `-O0`
- re-review the remaining hot disassembly against Clang `-O0`

If the remaining gap is still primarily backend shape, continue with another
 narrow `PA23` slice.

If the remaining gap is now mostly cross-function copy/move/dtor traffic, stop
 this plan and open the follow-up `O1` inlining / IPA plan instead.

Status:

- effectively reached
- the current evidence says the remaining dominant gap is the cross-function
  helper boundary, not one more hidden baseline lowering bug
- after the requested cleanup pass, the next plan should be the separate
  `O1` inlining / IPA plan

## Validation

Every slice should run the smallest useful set first:

- targeted `pa23` owner tests
- `make test-pa23`

At meaningful checkpoints, rerun:

- `scripts/bench_ctrlexpr.sh --self-only --levels O0`
- minimal helper repro disassembly/MIR inspection

At the end of the whole plan:

- full `test-report`
- `ctrlexpr` O0 benchmark comparison versus cached Clang `-O0`
- a written residual-gap analysis before starting the separate inlining plan

## Follow-Up After This Plan

After the baseline backend is fixed, create a separate plan for:

- tiny-function inlining at `O1`
- synthesized special-member inlining
- small wrapper inlining such as `get_result`
- post-inline scalar cleanup

Current handoff point into that follow-up:

- start with a tiny-function / hot-helper inliner, not whole-program work
- target `Calculator::get_result` and the tiny `CalcToken` special members first
- keep the current `PA23` baseline fixes intact while doing that cleanup and
  follow-up planning

That follow-up is required for Clang-like `-O1` / `-O2` behavior, but it should
 not be mixed into this baseline `PA23` quality tranche.
