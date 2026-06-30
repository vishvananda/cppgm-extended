# Reducer Implementation Fix Plan

## Objective

This phase handles reducers that looked like valid earlier-PA coverage but
could not be promoted because our compiler or output contract could not process
the reduced shape.

The canonical reference generator for current assignment refs is the local
`dev/cppgm++` built from this checkout. Historical `cppgm++-ref` binaries found
under the Opus tree were generated from older versions of this codebase; use
them only as historical evidence, not as the authority for current ref
generation or current reducer disposition.

The goal for each reducer is one of:

- fix the implementation and promote the reducer to the earliest owning PA;
- replace it with an even earlier, smaller reducer if the root bug belongs
  before the current candidate owner;
- prove that the source shape is invalid, later-owned, or only a PA23
  integration case and leave it out of earlier assignments with a recorded
  disposition.

## Source Queues

Start from `analysis/reference-reducer-failures.md`. It currently contains the
active reducers that were promising enough to keep but blocked by one of these
conditions:

- `reference-compiler-bug`
- `reference-contract-mismatch`
- `hosted-builtin-owner-needed`
- `source-invalid-cxx11`

Do not treat every `implementation-bug-only` row in
`pa23_feature_backfill_tracker.tsv` as a current implementation task. Several
of those rows already passed at the Opus PA23 start commit or are Trusted-only
historical notes. Promote a tracker row into this phase only when there is a
saved reducer, a clear current/reference failure, or a fresh reproduction.

Work by root-cause cluster rather than by tracker order. The first useful
clusters are:

- hidden-friend ADL and expression-SFINAE lookup;
- inherited constructor templates and replay through dependent bases;
- template-template partial ordering, aliases, and default arguments;
- defaulted class-template argument deduction and base deduction;
- reference/output contract mismatches, especially hidden EH-shaped LowIR;
- hosted or vendor intrinsic rewrites that need portable C++11 shapes.

## Per-Reducer Loop

For each selected reducer, create a short work record in the ledger before
editing production code:

1. Name the PA23 source row and saved reducer path.
2. Confirm the candidate owner PA from the assignment README and nearby tests.
3. Validate that the reducer is C++11:

   ```sh
   g++ -std=c++11 -x c++ -fsyntax-only analysis/reducers/<name>.t
   ```

4. Reproduce the current failure with the local compiler and local canonical
   reference-generation lane. Save transient logs under `/tmp`, not in the
   repository.
5. Re-run the Opus historical gate in a disposable clone or temporary worktree:
   the reducer should fail at the PA23 start commit and pass at the identified
   fixing commit. If it does not, either find a smaller shape that preserves the
   start/fix signal or change the tracker disposition.
6. Classify the root cause before implementing:
   `semantic implementation`, `reference compiler`, `LowIR/output contract`,
   `canonicalization`, `placement`, `source validity`, or `hosted intrinsic`.

Only start code changes once the failure is reproduced and the root cause is
specific enough to point at an implementation area.

## Implementation Fix Loop

Use the normal production path for real compiler fixes:

- make production changes in `dev/` or `dev/src/`;
- keep assignment directories limited to tests, references, README wording, and
  wrappers;
- keep the reducer in `analysis/reducers/` until it passes the implementation,
  canonical-ref, historical, and placement gates.

After the implementation fix:

1. Re-run the failing reducer against the fixed compiler.
2. Re-run the local canonical reference workflow that previously blocked
   promotion. If the only failure was from an older Opus `cppgm++-ref`, treat
   that as historical evidence and reclassify the reducer based on the local
   build.
3. If the fix exposes a smaller or earlier bug, reduce again and restart the
   loop on the new shape before adding assignment tests.
4. Generate references only after the owner PA and reducer shape are stable.
5. Update `analysis/reference-reducer-failures.md` with the resolution, final
   test path, and validation commands.
6. Update `pa23_feature_backfill_tracker.tsv` from `no-action` or
   `harness-or-reference-issue` to `test-added` only after the assignment test
   and refs are checked in.

## Earliest-Owner Fallback

If fixing the compiler shows that the real missing behavior belongs earlier
than the candidate PA, do not force the original reducer into the later PA.

Instead:

1. Identify the earliest PA whose README owns every required language feature.
2. Write an even smaller reducer against that PA's feature set.
3. Validate the new reducer with the same C++11, current, canonical-ref, and
   Opus start/fix gates.
4. Run the placement audit after adding the test:

   ```sh
   python3 scripts/audit_pa_feature_placement.py --pa paXX --no-course \
     --markdown-out /tmp/paXX-placement-audit.md \
     --json-out /tmp/paXX-placement-audit.json \
     --fail-on-early
   ```

5. If the audit says the new test still depends on a later feature, reduce
   again or move it to the correct owner.
6. Re-run this same disposition workflow for the original PA23 shape. The new
   earlier reducer may make the PA23 original redundant, or it may leave a real
   PA23 integration case that should stay.

## Original PA23 Disposition Check

Every time a reducer is promoted or moved to an earlier PA, run:

```sh
python3 scripts/report_pa23_original_disposition.py --summary
python3 scripts/report_pa23_original_disposition.py
```

Use the report to decide whether the PA23 original test is still useful:

- If the PA23 source is now an exact duplicate or only the same single-feature
  check, remove it and any generated sidecars.
- If the PA23 source combines earlier features in a way the new reducer does
  not cover, keep it and record the reason in
  `analysis/pa23-original-disposition-overrides.tsv`.
- If it is unclear, keep it temporarily as a review item and do not close the
  reducer cluster.

The cluster is not done until the report has no unresolved duplicate,
near-duplicate, or keep-candidate rows for the affected tests.

## Validation Cadence

Run focused validation for every reducer:

```sh
make test-report ACTIVE_TEST_REPORT_PAS='paXX pa23'
```

When a batch adds multiple focused tests in the same assignment, pass them as a
single space-separated `TEST` value instead of running parallel or repeated
single-test checks. The assignment harness writes shared `.check` sidecars, so
grouped checks avoid races while still keeping validation focused:

```sh
make -C paXX check TEST='tests/general/a.t tests/general/b.t'
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
  make -C paXX check TEST='tests/general/a.t tests/general/b.t'
```

For template-heavy fixes in the PA19-PA23 window, also run:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
  make test-report ACTIVE_TEST_REPORT_PAS='pa19 pa20 pa21 pa22 pa23'
```

Run the full LowIR-compare report periodically, not just at the end. Use it:

- after each root-cause cluster that changes shared semantic or LowIR code;
- before committing a batch with more than one implementation fix;
- immediately after any fix that changes canonicalization, EH-shaped LowIR, or
  reference output contracts.

Command:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
```

If the full report is too slow for an inner loop, run the PA19-PA23 LowIR
window first, but the cluster should not be considered ready to commit until a
recent full LowIR-compare report has passed or the reason for deferring it is
recorded.

## Commit Boundaries

Keep commits reviewable:

- one implementation root cause per commit when possible;
- include the promoted reducer tests and refs with the fix that makes them pass;
- keep large PA23 duplicate cleanup in a separate commit when it is mechanical;
- update the ledger and tracker in the same commit as the corresponding test or
  disposition change.

Each commit message should record:

- the reducer cluster;
- the owner PA chosen;
- whether an earlier reducer replaced the original candidate;
- focused validation;
- the most recent LowIR-compare report scope.

## Definition Of Done

A reducer cluster is done when:

- every reducer in the cluster has a recorded disposition;
- every accepted reducer lives in the earliest owning PA with references;
- `scripts/audit_pa_feature_placement.py --fail-on-early` passes for touched
  owner PAs;
- `scripts/report_pa23_original_disposition.py --summary` has no unresolved
  rows for the affected originals;
- focused assignment reports pass;
- a recent `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` has passed, or
  a narrower temporary validation scope and follow-up reason are recorded.
