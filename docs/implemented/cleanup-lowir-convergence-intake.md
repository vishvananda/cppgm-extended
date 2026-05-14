# Cleanup LowIR Convergence Intake

This checklist records the commits on `main` that are not yet patch-equivalent
in `cleanup-passb-20260409`, along with how each should be handled during the
convergence process.

Source command:

```bash
git cherry -v cleanup-passb-20260409 main
```

The intent is:

- pull clean docs / harness / Makefile commits first
- import genuinely new tests before their matching semantic changes
- manually port semantic changes that overlap diverged hot files
- ignore stale ref churn on older numbering/layout until the end

## Intake Checklist

### Clean Or Mostly-Clean Build / Harness Intake

- [x] `d20e78d9` `Parallelize pa37 ladder stage builds`
  - classification: clean `pa37/Makefile` intake
  - expected handling: cherry-pick cleanly

- [x] `c937b61e` `Cache shared pa37 self-host objects across checkpoints`
  - classification: clean `pa37/Makefile` intake
  - expected handling: cherry-pick cleanly

- [x] `a1405066` `Export pa37 ladder test environment settings`
  - classification: clean `pa37/Makefile` intake
  - expected handling: cherry-pick cleanly

- [x] `03307654` `Add shared run/check targets and fix pa37 runner rule`
  - classification: harness / Makefile intake
  - touched files:
    - `scripts/pa_run_check_targets.mk`
    - `pa12` through `pa29` `Makefile`
    - `pa37/Makefile`
  - expected handling: cherry-pick if clean; validate that it does not disturb
    the cleanup-pass Makefile changes

- [x] `ca145cb0` `Default sparse frontend builds and extend pa37 through pa34`
  - classification: build-system intake with generated file
  - touched files:
    - `dev/Makefile`
    - `scripts/generate_pa_fast_link_sets.py`
    - `dev/generated_pa_fast_link_sets.mk`
    - `pa37/Makefile`
  - expected handling: manual port or careful cherry-pick, then regenerate the
    generated file in the current tree if needed

### Semantic / LowIR Commits With New Tests

- [x] `cc155d41` `Fix function-reference call lowering`
  - classification: semantic `lowirgensemantic` change plus new owner test
  - new test:
    - `pa14/tests/spec/217-function-reference-static-cast-call.t`
  - expected handling:
    - import the new test in the current numbering/layout
    - port the semantic change
    - validate the direct owner before moving on

- [x] `e97f92a7` `Fix hosted ostream virtual-base parameter lowering`
  - classification: semantic `lowirgensemantic` change plus new hosted runtime
    test
  - new test:
    - `pa34/tests/link/725-hosted-ostream-char-sequence-parameter-runtime-smoke.t`
  - expected handling:
    - import the new test in the current suite shape
    - port the semantic change
    - validate the new hosted owner directly

- [x] `3df3e311` `Fix pa37 host link default and ostream ref-member lowering`
  - classification: semantic `lowirgensemantic` change plus new hosted runtime
    test and `pa37/Makefile` adjustment
  - new test:
    - `pa34/tests/link/726-hosted-ostream-ref-member-char-sequence-runtime-smoke.t`
  - expected handling:
    - import the test first
    - port the semantic change carefully
    - handle the `pa37/Makefile` part with the other harness intake

- [x] `3592e6d8` `Fix float-width lowering and pa8 scalar init bytes`
  - classification: semantic/backend change plus new tests
  - touched code:
    - `dev/src/lowir_machine_ir.cpp`
    - `dev/src/lowirgensemantic.cpp`
    - `dev/src/nsinit_semantic.cpp`
  - new tests:
    - `pa23/tests/structural/720-f64-f80-implicit-store-return-convert.t`
    - `pa34/tests/link/727-hosted-vector-char-assign-initlist-runtime-smoke.t`
  - expected handling:
    - import the tests first
    - port the code change carefully
    - validate `pa23`, `pa8`, and the new `pa34` owner directly

- [x] `5097b3f6` `Fix local-type template matching and semantic display names`
  - classification: semantic/template-resolution change plus new tests
  - touched code:
    - `callsemantic.cpp`
    - `semantic_overload.cpp`
    - `template_resolution.cpp`
    - related semantic/template files
  - new tests:
    - `pa18/tests/spec/215-local-constructor-template-member-typedef.t`
    - `pa18/tests/spec/216-local-member-call-constructor-template-instantiation.t`
  - expected handling:
    - import the tests first
    - manually port the semantic changes
    - validate the direct `pa18` owner lane before moving on

- [x] `82675ffc` `Fix special-member output and hosted EH regressions`
  - classification: large semantic/output/class-model change with one genuinely
    new test and heavy old ref churn
  - touched hot files:
    - `callsemantic.cpp`
    - `semantic_class_model.cpp`
    - `semantic_output.cpp`
    - `lowirgensemantic.cpp`
    - related semantic model files
  - genuinely new test:
    - `pa16/tests/spec/353-implicit-move-constructor-moveonly-member.t`
  - old ref churn:
    - many existing `pa12` through `pa29` refs
  - expected handling:
    - import only the genuinely new owner test first
    - manually port the semantic changes
    - ignore old-number ref churn until final normalization
    - validate each affected earliest owner before continuing
  - convergence note:
    - imported the genuinely new current owner as
      `pa16/tests/spec/353-implicit-move-constructor-moveonly-member.t`
    - kept the semantic/model/output changes and discarded the stale old-number
      ref churn from `main`
    - normalized the affected current-branch refs in `pa12`, `pa15`, `pa16`,
      `pa18`, `pa21`, `pa22`, `pa26`, `pa27`, and `pa29`
    - verified owner suites directly:
      - `pa12`
      - `pa15`
      - `pa16`
      - `pa17`
      - `pa18`
      - `pa21`
      - `pa22`
      - `pa26`
      - `pa27`
      - `pa29`
      - `pa34`

### Semantic / LowIR Commits With Mixed Ref Churn

- [x] `65486cdc` `Fix self-host literal lowering and restore green test report`
  - classification: semantic change with mixed build/ref fallout
  - touched code:
    - `constant_value.cpp`
    - `constexpr_eval.cpp`
    - `lowir_internal.cpp`
  - touched non-code:
    - `pa23/tests/structural/720-f64-f80-implicit-store-return-convert.ref.cmir`
    - `pa27/tests/spec/120-initializer-list-call.ref`
    - `pa37/Makefile`
    - `dev/generated_pa_fast_link_sets.mk`
  - expected handling:
    - port the semantic change after `3592e6d8`
    - treat the `pa23` `720` ref as attached to the imported new test
    - ignore unrelated old ref churn until current numbering/layout is settled
  - convergence note:
    - imported the semantic/build pieces plus the new `pa23/720` owner
    - intentionally deferred the old `pa27/120` ref churn to final normalization

## Current Recommended Pull Order

1. `d20e78d9`
2. `c937b61e`
3. `a1405066`
4. `03307654`
5. `ca145cb0`
6. `cc155d41`
7. `e97f92a7`
8. `3df3e311`
9. `3592e6d8`
10. `65486cdc`
11. `5097b3f6`
12. `82675ffc`

## Validation Rule

Before moving to the next semantic intake item:

1. the newly imported tests must exist in the correct current assignment/path
2. those new tests must pass, or fail for an understood pre-port reason
3. the earliest owner suite for the semantic change must pass after the port

Do not advance the intake checklist while any newly imported semantic owner is
still ambiguous.
