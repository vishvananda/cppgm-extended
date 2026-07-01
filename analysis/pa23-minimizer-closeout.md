# PA23 Minimizer Closeout

## Outcome

The PA23 minimizer/backfill pass is complete for the current branch. Reducers
that proved missing earlier-PA coverage were promoted to the earliest
assignment owner that could express the feature. PA23 originals were then
reviewed separately so PA23 keeps integration cases and drops isolated
single-feature duplicates.

The current PA23 original-disposition report is clean:

```text
pa23-original-removed	183
keep-pa23-integration	53
```

No `retire-pa23-duplicate`, `review-retire-or-simplify`,
`review-near-duplicate`, or `keep-pa23-integration-candidate` rows remain.

## Reducer Inventory

Saved reducer files remain under `analysis/reducers/` as audit evidence:

```text
pa18 reducers 3
pa19 reducers 5
pa20 reducers 1
pa21 reducers 83
pa22 reducers 94
total reducers 186
```

The reducer files are intentionally kept because the ledgers and tracker notes
refer to them by path. Removing or moving them would make the audit trail worse
without changing the assignment test surface.

## Current Test Counts

Tracked `.t` tests in the PA18-PA23 window at closeout:

| PA | spec | general | total |
| --- | ---: | ---: | ---: |
| pa18 | 42 | 151 | 193 |
| pa19 | 22 | 98 | 120 |
| pa20 | 5 | 56 | 61 |
| pa21 | 51 | 129 | 180 |
| pa22 | 50 | 126 | 176 |
| pa23 | 70 | 301 | 371 |

## Remaining Exceptions

Remaining non-promoted reducer notes are intentional dispositions rather than
active minimizer cleanup:

- `no-backfill-start-pass`: the reduced/focused shape already passes at the
  Opus PA23 start anchor, so it does not prove missing earlier coverage.
- `historical-validation-missing`: current accepts the reducer, but no
  reachable Opus start/fix transition was found for the focused shape.
- `pa23-integration-static-emission-contract`: the PA23 original is testing a
  PA23 output/static-emission integration contract, not a missing earlier
  semantic reducer.

## Validation

Closeout validation:

```sh
scripts/report_pa23_original_disposition.py --summary
git grep -n 'run-pass' -- . ':!obj' ':!*.my' ':!*.check' || true
git diff --check
```

The broad PA19-PA23 LowIR report and focused reducer checks were run during the
individual reducer batches recorded in `analysis/reference-reducer-failures.md`.
This closeout commit is documentation-only.
