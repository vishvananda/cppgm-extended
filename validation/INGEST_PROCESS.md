# Validation Candidate Ingest Process

This process is for promoting items from [`validation/tests`](./tests) into the
normal `pa*` regression suites.

Unlike [`docs/spec-conformance-audit.md`](../docs/spec-conformance-audit.md),
these candidates already come with a concrete seed test. The ingest loop is
therefore:

1. Verify the candidate against [`n3485.txt`](../n3485.txt).
   - Read the clause(s) cited in [`README.md`](./README.md).
   - If the candidate is wrong, incomplete, or narrower/broader than the spec
     text, update the validation docs first.

2. Confirm the candidate’s expected behavior outside our compiler.
   - `run-pass`: host-compile and run it with `clang++ -std=c++11`.
   - `compile-pass`: host-compile it with `clang++ -std=c++11 -c`.
   - `compile-fail`: confirm `clang++ -std=c++11 -c` rejects it.
   - If host behavior and the cited N3485 wording disagree, treat the spec text
     as authoritative and note the host mismatch in the tracker.

3. Choose the earliest natural PA.
   - Put the regression in the earliest PA that can naturally express and check
     the rule.
   - Prefer pure-language earlier PAs over PA32 whenever possible.
   - If an exact existing regression already covers the same rule, record that
     existing regression in the tracker instead of adding a duplicate file.

4. Check the candidate on our compiler.
   - `compile-pass` and `compile-fail` candidates can be preflighted with:
     - `make run-cppgm CPPGM_ARGS='-c -o /tmp/<id>.out validation/tests/<file>.cpp'`
   - `run-pass` candidates should be promoted into the chosen PA first and then
     checked through the normal PA harness, since compile-only success is not a
     sufficient oracle.
   - Also do a LowIR sanity pass whenever the feature is supposed to lower in
     our current compiler.
     - Use `--emit-lowir` through the normal PA harness or through
       `make run-cppgm CPPGM_ARGS='--emit-lowir -o /tmp/<id>.lowir validation/tests/<file>.cpp'`.
     - This is a manual inspection step: read the emitted LowIR and reason about
       whether it actually matches the C++ source behavior being tested.
     - Do not treat successful lowering to LowIR, successful assembly/codegen,
       or a passing runtime alone as sufficient if the emitted LowIR is
       obviously wrong for the tested rule.
     - If the candidate is promoted into an earlier non-LowIR PA, still run a
       one-off LowIR probe during ingest when the feature should already lower
       correctly in the current compiler.

5. Promote the test into the PA suite.
   - Copy the validation source into the chosen `pa*/tests/...` location.
   - Keep the validation seed in `validation/tests/`; promotion does not delete
     the source candidate.
   - Generate `.ref`, `.ref.stdout`, and `.ref.exit_status` in the normal PA
     format after the behavior is correct.
   - When the promoted regression has a LowIR-producing harness, generate the
     `.ref` from emitted LowIR only after manually sanity-checking that the
     lowering structurally matches the source intent. Do not bless the first
     emitted LowIR blindly just because later stages accept it.

6. Branch by outcome.
   - If our compiler already matches the required behavior:
     - keep the promoted regression,
     - keep the LowIR sanity note with the tracker item,
     - run the targeted PA regression,
     - run full root `make test-report`,
     - commit the regression + tracker update.
   - If our compiler does not match:
     - keep the promoted regression,
     - fix the implementation in `dev/`,
     - rerun the LowIR sanity check after the fix,
     - rerun the targeted regression,
     - run full root `make test-report`,
     - commit the regression + fix + tracker update.

7. Handle ref churn conservatively.
   - Only update unrelated refs when the change is:
     - naming/order-only and codegen-equivalent, or
     - a demonstrable correctness fix.

8. Update the ingest tracker.
   - Use [`INGEST_TRACKER.md`](./INGEST_TRACKER.md) as the sequential checklist.
   - Record:
     - chosen PA,
     - resulting regression path,
     - status,
     - commit,
     - LowIR sanity notes when applicable,
     - any deferred follow-up (for example when a conversion rule is fixed but a
       larger expression form remains unsupported).

## Status Key

- `todo`: not worked yet
- `host-verified`: spec checked and host behavior confirmed, but not yet promoted
- `covered`: exact behavior already covered by an existing PA regression
- `promoted-pass`: promoted regression already passed on our compiler
- `promoted-fixed`: promoted regression failed first, implementation fixed, now green
- `deferred`: verified item, but blocked on a prerequisite
- `not-a-bug`: candidate or suspicion does not describe a real C++11 mismatch

## Recommended Per-Item Checklist

1. Read the validation source and cited N3485 text.
2. Confirm host behavior.
3. Pick the earliest PA.
4. Check whether an exact PA regression already exists.
5. Run and manually inspect a LowIR sanity probe if the feature should already lower.
6. Promote or record the existing regression.
7. Fix `dev/` if needed.
8. Run targeted regression(s).
9. Run full root `make test-report`.
10. Update [`INGEST_TRACKER.md`](./INGEST_TRACKER.md).
11. Commit one item at a time.

## Tracker Maintenance

[`INGEST_TRACKER.md`](./INGEST_TRACKER.md) is generated from the candidate matrix
in [`README.md`](./README.md) by:

```sh
python3 scripts/generate_validation_ingest_tracker.py
```

Regenerate it if the candidate matrix changes structurally. Normal day-to-day
status updates should be made directly in the tracker.
