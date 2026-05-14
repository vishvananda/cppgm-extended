# Plan Implementation Tracker

This tracker is only for active or intentionally deferred planning documents in
`docs/`. Completed plans belong in `docs/implemented/`; stale investigations,
old frontier process notes, and historical debug trackers belong in `legacy/`.

Operational reference docs such as performance validation and source coverage
remain in `docs/`, but they are not ordered implementation plans.

## Status Key

- `pending`: not started yet
- `in_progress`: active implementation work is underway
- `ongoing`: recurring review or triage process
- `blocked`: waiting on an earlier dependency or boundary decision

## Status Review Notes

- The PA33 `540` compile-performance plan reached its stated performance target
  and is archived in `docs/implemented/`.
- The imported-symbol-kind follow-up is not complete. The immediate PA32
  object-vs-function import bug was fixed by splitting function and object maps,
  but the planned typed import registry has not replaced
  `external_function_symbols_` / `external_object_symbols_` in
  `dev/src/lowirgensemantic.cpp`.
- The template/LowIR log convergence plan is not implemented yet. The driver
  still exposes the current witness output paths, and there is no
  `--template-log` option.
- `in_progress` is reserved for work currently being implemented in this
  branch. Deferred cleanup lanes are `pending` unless active work resumes.

## Active Plan Order

| Order | Status | Plan | Purpose | Notes |
| --- | --- | --- | --- | --- |
| 1 | pending | [pa33-pa34-convergence-plan.md](pa33-pa34-convergence-plan.md) | Drive hosted-source and hosted-link compatibility failures to their earliest owning PA. | Historical frontier ledger. Revalidate current `main` before resuming or archiving. |
| 2 | pending | [machine-backend-o0-quality-plan.md](machine-backend-o0-quality-plan.md) | Improve baseline PA23 `-O0` machine-code quality before later backend optimization work. | Partway landed; remaining work is a deferred PA23 quality lane, not active PA36 `-O1`/`-O2` work. |
| 3 | pending | [semantic-fallback-removal-plan.md](semantic-fallback-removal-plan.md) | Replace remaining semantic fallback control flow with explicit structured decisions. | Several hard-fail categories and text-reparse removals landed; deferred rewrite-removal inventory remains. |
| 4 | pending | [template-lowir-log-convergence-plan.md](template-lowir-log-convergence-plan.md) | Move template logging onto the real LowIR closure path. | Not started; depends on the structured witness/source-use boundary staying stable. |
| 5 | pending | [imported-symbol-kind-followup-plan.md](imported-symbol-kind-followup-plan.md) | Carry function-vs-object import kind explicitly through LowIR/object emission. | Immediate PA32 import-kind bug is fixed, but the first-class typed import registry is still future work. |
| 6 | ongoing | [spec-conformance-audit.md](spec-conformance-audit.md) | Spec-first review loop for placing real language gaps in the earliest owner. | Runs alongside other work rather than after it. |
| 7 | blocked | [pa10-34-assignment-cleanup-process.md](pa10-34-assignment-cleanup-process.md) | Assignment boundary, test placement, README, and implementability cleanup. | Passes A-C and the test-grouping cleanup are complete. Only deferred Pass D buildout remains; working tracker: [pa10-34-assignment-cleanup-tracker.md](pa10-34-assignment-cleanup-tracker.md). |
| 8 | ongoing | [deferred-issues-tracker.md](deferred-issues-tracker.md) | Revalidate and close known deferred bugs, workarounds, and transitional gaps. | Only keep items here when they are not already owned by a dedicated plan above. |
| 9 | in_progress | [student-assignment-export-process.md](student-assignment-export-process.md) | Export the cleaned assignments into the student-facing repository format. | README/scaffold inventory prep has started; final generated export remains gated on Pass D and export validation. Working docs: [student-export-readme-scaffold-subagent-plan.md](student-export-readme-scaffold-subagent-plan.md), [student-export-inventory.md](student-export-inventory.md). |

## Active Reference Docs

- [performance-regression-validation.md](performance-regression-validation.md):
  standard perf gate based on instructions and memory rather than noisy wall time.
- [source-coverage-analysis-strategy.md](source-coverage-analysis-strategy.md):
  source coverage workflow and `make source-coverage-report` notes.

## Archive Policy

When finishing a plan:

1. Update this tracker if the plan is listed above.
2. Move the plan to `docs/implemented/`.
3. Move obsolete trackers, prototype notes, and old process docs to `legacy/`.
4. Update links from active docs so `docs/` remains the live working surface.
