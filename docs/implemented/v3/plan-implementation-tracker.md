# Plan Implementation Tracker

This tracker lists the active and deferred plans in `docs/`.  Finished plans
move to `docs/implemented/` (the plans that shaped the implementation in the
source tree before it became this repository's reference are under
`docs/implemented/v3/`).  Placement decisions and audit records that are
consulted but not executed stay in `docs/` as reference documents.

## Status Key

- `in_progress`: implementation work is underway in this branch
- `pending`: not started, or paused at a recorded point
- `deferred`: intentionally after another plan
- `ongoing`: a recurring review loop

## Active Plan Order

| Order | Status | Plan | Purpose | Notes |
| --- | --- | --- | --- | --- |
| 1 | in_progress | [PLAN-CPPGM-EXTENDED-V4.md](PLAN-CPPGM-EXTENDED-V4.md) | Make the current implementation this repository's reference: one source tree, one suite per assignment, the export from it. | Phases 0 to 5 have landed on `v4`; the trackers are in [v4/](v4/README.md).  Phase 6 (handout review) and Phase 7 (export and CI) follow; Phase 8 is the plan below. |
| 2 | deferred | [assignment-restructure-plan.md](assignment-restructure-plan.md) | Combine and rebalance the assignments once every suite and the export are green. | Waits for the v4 plan's exit criteria. |
| 3 | pending | [PLAN-CODEGEN-AND-SELFHOST-OPTIMIZATION.md](PLAN-CODEGEN-AND-SELFHOST-OPTIMIZATION.md) | Bring the self-compiled compiler's speed toward the host build's. | Carried from the source tree with its measured dispositions; resume from the status line at its top. |
| 4 | pending | [PLAN-O1-PARITY.md](PLAN-O1-PARITY.md) | Close the -O1 gap against the host compiler on the self-host lanes. | Carried from the source tree; the slice space it explored is recorded in the plan. |
| 5 | pending | [PLAN-CLANG-LIBCXX-SUPPORT.md](PLAN-CLANG-LIBCXX-SUPPORT.md) | Host the compiler on clang and libc++ as well as gcc and libstdc++. | Proposed; the probe findings are in the plan. |
| 6 | pending | [PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md](PLAN-EARLY-SEMANTIC-CORE-UNIFICATION.md) | Unify the early semantic surfaces (PA7 to PA12) on the one analyzer. | Planned; no implementation work has started. |
| 7 | pending | [semantic-fallback-removal-plan.md](semantic-fallback-removal-plan.md) | Replace the remaining semantic fallback control flow with explicit decisions. | Several hard-fail categories landed earlier; the deferred inventory remains. |
| 8 | pending | [resolve-type-lookup-text-removal-plan.md](resolve-type-lookup-text-removal-plan.md), [template-type-argument-reparse-removal-plan.md](template-type-argument-reparse-removal-plan.md), [type-text-fallback-removal-tracker.md](type-text-fallback-removal-tracker.md) | Remove the last text-shaped type and lookup paths. | Revalidate against the current tree before resuming. |
| 9 | pending | [pa34-pa35-convergence-plan.md](pa34-pa35-convergence-plan.md) | Drive hosted-source and hosted-link failures to their earliest owning assignment. | Historical frontier ledger; revalidate before resuming or archiving. |
| 10 | ongoing | [spec-conformance-audit.md](spec-conformance-audit.md) | Spec-first review for placing real language gaps in their earliest owner. | Runs alongside other work. |

## Reference Documents

- Placement: [pa15-pa23-contract-test-audit-plan.md](pa15-pa23-contract-test-audit-plan.md)
  and its tracker, the feature allocation audit, the placement decision
  records for PA14 to PA22, [template-strict-placement-tracker.md](template-strict-placement-tracker.md),
  [pa10-37-readme-handout-audit.md](pa10-37-readme-handout-audit.md),
  [pa10-34-assignment-cleanup-process.md](pa10-34-assignment-cleanup-process.md)
  and its tracker (the passes it describes are complete; its Pass D buildout
  is superseded by the v4 plan).
- Export: [student-assignment-export-process.md](student-assignment-export-process.md),
  [student-export-inventory.md](student-export-inventory.md) and the scaffold
  plans describe the export the v4 plan's Phase 7 revises;
  `scripts/export_student_repo.sh` is the executable form.
- Operations: [performance-regression-validation.md](performance-regression-validation.md)
  (the instruction-count and memory gate) and
  [source-coverage-analysis-strategy.md](source-coverage-analysis-strategy.md).
- History: [assignment-numbering-migration-2026-08.md](assignment-numbering-migration-2026-08.md),
  [pa34-35-test-disposition.md](pa34-35-test-disposition.md).

## Retired With The Witness Lane

The witness output and its strict suite were removed with the v4 move; the
witness convergence plans and ledgers (alias, class use, lifecycle, class
materialization, semantic-path consolidation) and the template LowIR log
convergence plan went with them.  The semantic routes they were written to
find are the subject of the early semantic core unification plan.

## Archive Policy

When finishing a plan:

1. Update this tracker.
2. Move the plan to `docs/implemented/`.
3. Update links from active documents so `docs/` stays the live surface.
