# PA15-PA23 Test Placement Agent Prompt

You are auditing test placement only. Do not move, rename, delete, edit, or
regenerate tests or references.

## Inputs

Read these before making decisions:

- `docs/pa15-pa23-contract-test-audit-plan.md`
- `docs/pa15-pa23-contract-test-audit-tracker.md`
- the relevant `paN/README.md` files
- the assigned test source files and adjacent `.ref` / `.ref.exit_status` files

Use the scanner as a triage aid:

```sh
python3 scripts/audit_pa_feature_placement.py --pa pa15 --pa pa16 --pa pa17 \
  --markdown-out /tmp/pa15-pa17-feature-audit.md \
  --json-out /tmp/pa15-pa17-feature-audit.json \
  --csv-out /tmp/pa15-pa17-feature-audit.csv
```

The scanner checks source and matching LowIR/output `.ref` files. Treat its
matches as review leads, not final placement decisions.

## Decision Rules

For each assigned test, decide the earliest correct PA and cluster.

- The canonical owner table is in
  `docs/pa15-pa23-contract-test-audit-tracker.md`.
- A test belongs at the earliest PA/cluster that owns the behavior it asserts.
- Incidental support syntax does not control placement when it is already
  accepted and not essential to the expected result.
- If multiple essential features are asserted together, place the test at the
  latest required `(PA, cluster)` among those essential features.
- If multiple essential features are separable, recommend splitting the test
  and give one destination for each proposed split.
- PA10-PA12 semantic owners cannot be final locations for LowIR output tests;
  choose the LowIR-owning feature that is actually being asserted.
- PA29 backend-only features should not receive source-to-LowIR tests unless
  the test is reduced to a backend/LowIR input. For source tests, identify the
  correct source-to-LowIR owner or recommend reducing/removing the backend-only
  assertion.
- If the current PA is correct but the cluster is early, keep the PA and
  recommend the first correct cluster.
- If the scanner match is a false positive, mark it as ignored and explain why.

## Output

Write a Markdown report for your assigned range. Do not edit tests.

Use this table:

| Test | Current | Decision | Destination | Primary Feature | Essential Later Features | Action | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |

Decision values:

- `keep`
- `move`
- `renumber`
- `split`
- `reduce-or-remove`
- `manual-review`

Destination format is `paN/tests/<cluster>-...` when a single destination is
clear, or a short list for split cases. Include enough notes for a maintainer to
understand the reason without rereading every file.
