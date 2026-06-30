# PA23 Initial Failure Evidence

These files seed the PA23 feature backfill tracker from isolated baseline
checkouts of the two Ralph runs.

## Baselines

- Opus: `/home/vishvananda/work/opus` at `1963d796e4cc` (`pa22: audit findings, changes, and architecture review`)
- Trusted: `/home/vishvananda/work/trusted` at `b8899d281a44` (`Audit PA22 template lowering cleanup`)

Both baselines were copied into disposable clones under `/tmp` before running
tests. The active run worktrees were treated as read-only.

## Commands

Each disposable checkout ran:

```sh
make test-report ACTIVE_TEST_REPORT_PAS='pa23'
```

Observed summaries:

- Opus: `===== TEST SUMMARY: 261 / 520 TESTS PASSED =====`
- Trusted: `===== TEST SUMMARY: 242 / 519 TESTS PASSED =====`

## Files

- `opus-pa23-initial-failures.tsv`: exact Opus PA23 failure paths and messages.
- `trusted-pa23-initial-failures.tsv`: exact Trusted PA23 failure paths and messages.
- `opus-pa23-initial-failing-tests.txt`: Opus failure paths only.
- `trusted-pa23-initial-failing-tests.txt`: Trusted failure paths only.

The union is in `../../pa23_feature_backfill_tracker.tsv`.
