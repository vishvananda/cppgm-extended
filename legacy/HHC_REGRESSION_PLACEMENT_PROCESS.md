# Hosted Regression Placement Process

Legacy note:

- The active combined process is now documented in
  `HOSTED_HEADER_FRONTIER_PROCESS.md`.
- Use `HOSTED_HEADER_FRONTIER_TRACKER.md` for new frontier items and new regression
  placement tracking.

This process is for turning hosted-header tracker items into durable assignment-local
regressions directly in the live tree.

## Goal

Move each implemented HHC item out of the generic tracker bucket and into the earliest real
`paN/tests/spec/` location that matches the written assignment contract.

## Workflow

1. Start from the tracker item, but treat the tracker PA as a hint only.
2. Re-check ownership against `ROADMAP.md` and the target `paN/README.md`.
3. If the item is before `pa10`, defer it for now.
4. If it is true hosted/vendor compatibility work, leave it in the PA32 bucket.
5. Otherwise reduce it to the earliest standard-language milestone that really owns it.
6. Write the smallest fresh reduction that captures the bug clearly.
7. Validate with the archived binaries when that is practical, using the per-PA
   `run_one_test` wrapper rather than rerunning full suites during reduction work.
8. Expected result is `old fail / current pass` when archived binaries are available. If old
   passes or current fails, keep notes in
   the ledger instead of forcing a false completion.
9. Finalize the test directly in the owning PA directory as part of the fix.

## Placement Rules

- Use the ordinary numeric sequence, not `HHC-###` filenames.
- Put the test near similar existing cases.
- Keep easier tests earlier and harder tests later so the sequence still suggests an
  implementation order.
- Leave gaps where practical for future insertions.
- Preserve HHC traceability through the tracker and ledger, and in the test body if a local
  note is helpful.

## Required Updates

When a regression is finalized:

- rename the test and all sidecars (`.ref`, `.ref.exit_status`, `.ref.stdout`) to the chosen
  numeric slot
- update `HHC_REGRESSION_LEDGER.md` with the final numbered path
- update `HOSTED_HEADER_COMPATIBILITY_TRACKER.md` with `Final regression: ...`
- if the new test expands the written milestone contract, update both:
  - the owning `paN/README.md`
  - `ROADMAP.md`

## Validation After Placement

- rerun the affected `make test-paN` suite
- keep the archived old/current binaries in sync with the recorded snapshot metadata when they
  are part of the validation story
