# PA23 Feature Backfill Audit Plan

## Objective

PA23 is intended to be a composition stage: it should mostly mix features that
were introduced in PA18-PA22 rather than introduce a large set of new primitive
template features. The Opus run started PA23 with many PA23 tests failing, which
suggests that some earlier assignments may be missing focused tests for features
that PA23 later forced the model to implement.

This audit will use the Opus PA23 implementation history as evidence to decide,
test by test, whether each initially failing PA23 case represents:

- a true missing feature that should have been required earlier;
- a difficult interaction between earlier features that belongs at the earliest
  assignment where all ingredients are available;
- an implementation bug that does not need a new placement test;
- a PA23-only composition case that should remain in PA23;
- a harness/reference/test issue.

The output is a tracker of all PA23 tests, a set of reduced missing-feature
examples, and commits that add those examples to the correct earlier PAs.

## Working Directories

Primary editable worktree:

```sh
/home/vishvananda/cppgm-pa23-feature-audit
```

This is a `cppgm-extended` worktree created from fresh `origin/main`:

```sh
git -C /home/vishvananda/cppgm-extended worktree add \
  -b pa23-feature-audit \
  /home/vishvananda/cppgm-pa23-feature-audit \
  origin/main
```

At creation, `origin/main` was `c29d9ee94 Export LowIR native ref canonicalizer`.

Read-only evidence sources:

```sh
/home/vishvananda/work/opus
/home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl
/home/vishvananda/work/.ralph/opus-opus-xhigh/usage-cache/
/home/vishvananda/work/trusted
/home/vishvananda/work/.ralph/trusted-gpt-5.5-xhigh/events/run.jsonl
/home/vishvananda/work/.ralph/trusted-gpt-5.5-xhigh/usage-cache/
```

The Opus and Trusted checkouts may be part of active or recently active Ralph
runs. Treat both git repositories and the Ralph run files as read-only evidence.

Do not run `make`, `git checkout`, `git reset`, `git clean`, `git apply`, ref
generation, or any command that writes `.my`, `.check`, `.diff`, object, or log
files under `/home/vishvananda/work/opus` or `/home/vishvananda/work/trusted`.
Use only read-only commands there, such as `git log`, `git show`, `git diff`,
`git grep`, `rg`, `sed`, and `jq`.

If a historical Opus or Trusted state must be built or tested, create a separate
disposable clone outside the active run tree and run tests there. Do not add a
git worktree to an active run repository unless explicitly approved, because even
`git worktree add` writes metadata into the source repository.

## Known Opus PA23 Baseline

Opus started PA23 in Ralph turn 51.

Useful run facts:

- Ralph run file:
  `/home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl`
- Turn start timestamp:
  `2026-06-21T16:53:50Z`
- Turn/thread id:
  `18d44157-ac14-48c3-a00e-5ed91da4aada`
- Required command at turn start:
  `make test-report-through-pa23`
- Initial through status:
  `1912/2171` tests passing, PA1-PA22 passing, first failing stage PA23
- Initial first blocker:
  `pa23/tests/general/100-call-operator-template-use-scope-shadowing.t`
- Initial scoped PA23 report reproduced from a disposable clone at `1963d796e`:
  `261/520` PA23 tests passing

Useful commands for locating the baseline evidence:

```sh
rg -n 'Implement `pa23`|pa23 full-stage|test-report-through-pa23|1912/2171' \
  /home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl

git -C /home/vishvananda/work/opus log --oneline --grep='pa23' --all
git -C /home/vishvananda/work/opus show 1963d796e
```

The PA22 audit commit immediately before PA23 in the Opus history was observed
as:

```text
1963d796e pa22: audit findings, changes, and architecture review
```

Use this as a historical anchor, but verify exact parentage before doing
mechanical ranges; the Opus run has many later PA23 commits and may have been
rebased or continued.

Trusted started PA23 in Ralph turn 45.

Useful run facts:

- Ralph run file:
  `/home/vishvananda/work/.ralph/trusted-gpt-5.5-xhigh/events/run.jsonl`
- Turn start timestamp:
  `2026-06-05T05:31:02Z`
- Required command at turn start:
  `make test-report-through-pa23`
- Initial through status:
  `1924/2201` tests passing, PA1-PA22 passing, first failing stage PA23
- Initial first blocker:
  `pa23/tests/general/100-bool-nontype-integral-member-equality.t`
- Initial scoped PA23 report reproduced from a disposable clone at `b8899d281`:
  `242/519` PA23 tests passing

## Seeded Failure Lists

The exact initial failure lists for Opus and Trusted are seeded in:

```text
analysis/pa23-initial-failures/
```

The aggregate starting tracker is:

```text
pa23_feature_backfill_tracker.tsv
```

It contains the union of tests that failed in either initial run:

- `both_fail`: 196 tests
- `opus_only`: 63 tests
- `trusted_only`: 81 tests

## Tracker

Create a machine-editable tracker at the worktree root:

```text
pa23_feature_backfill_tracker.tsv
```

Each row is one PA23 test. Recommended columns:

```text
test
directory
prefix
initial_status
initial_failure_kind
first_passing_commit
fixing_commit_range
opus_plan_evidence
opus_log_evidence
feature_cluster
candidate_owner_pa
classification
minimal_example
proposed_test_path
action_status
notes
```

Classification values:

```text
missing-earlier-feature
missing-feature-owner-unclear
feature-interaction-earliest-owner
pa23-composition-only
implementation-bug-only
harness-or-reference-issue
unknown
```

Action status values:

```text
unreviewed
needs-reducer
needs-owner-decision
ready-to-add-test
test-added
no-action
```

## Goal Operating Loop

Use this as the repeatable unit of work for the backfill goal. The goal can be
paused and resumed at cluster boundaries because every step leaves evidence in
the tracker.

Goal contract:

- Work only rows or small feature clusters already seeded in
  `pa23_feature_backfill_tracker.tsv`.
- Prefer `needs-reducer` rows with a clear `candidate_owner_pa`; defer rows
  whose owner or contract is ambiguous.
- For each selected cluster, either land an earlier-owner reducer test with
  references and validation, or explicitly mark why no test should be added.
- Do not promote a reducer to an assignment test until it passes the Opus
  historical validation gate below.
- Do not close a reducer attempt that fails the reference lane until it is
  classified in `analysis/reference-reducer-failures.md` as invalid C++11, a
  reference compiler bug, or a reference/current output contract mismatch.
- Stop at clean cluster boundaries with tracker rows updated for every covered
  PA23 test.

For each pass through the loop:

1. Select one small feature cluster, preferably rows already marked
   `needs-reducer` with a clear `candidate_owner_pa`.
2. Read the PA23 source tests, the candidate owner README, and nearby existing
   tests to confirm that the reduced behavior belongs earlier.
3. Write the smallest reducer that proves the missing semantic behavior.
4. Run the Opus historical validation gate in a disposable `/tmp` clone: the
   reducer must fail against the oracle at PA23 start and pass at the identified
   feature-fix commit.
5. Validate the reducer as C++11 with
   `g++ -std=c++11 -x c++ -fsyntax-only`.
6. Run the current compiler and the external reference compiler. If either
   rejects a C++11-valid reducer, or if the reference output cannot validate
   against current, record the reducer in
   `analysis/reference-reducer-failures.md` before closing the tracker row.
7. Add the reduced test and references to the earliest owner PA only after the
   historical, C++11, current, and reference gates pass.
8. Run the feature-placement audit for every touched owner PA to catch tests
   that still depend on a later feature or belong in a later cluster.
9. Run focused LowIR/witness validation, then the relevant assignment report.
10. Update `pa23_feature_backfill_tracker.tsv` for every covered PA23 row.

Per-row outcomes:

- `test-added`: reducer passed historical validation, earlier test and refs
  were added, and focused/current assignment validation passed.
- `no-action`: the row is already covered earlier, is PA23 composition only, is
  an implementation bug only, or is a harness/reference issue.
- `needs-owner-decision`: the feature appears real but the owner PA or contract
  wording is unclear.
- `needs-reducer`: reduction or historical validation is still incomplete.

At every stopping point, summarize the changed clusters, validation commands,
current tracker counts, and any skipped or unresolved clusters.

Build the initial list from the current assignment tree, not from stale Opus
artifacts:

```sh
cd /home/vishvananda/cppgm-pa23-feature-audit
find pa23/tests -type f -name '*.t' | sort
```

The simple file count may not exactly match the harness count because support or
course-linked tests may be included by the harness. Reconcile the tracker against
the actual `make test-report ACTIVE_TEST_REPORT_PAS='pa23'` output when running
inside the audit worktree.

## Evidence Workflow

### 1. Identify Initially Failing Tests

The initial tracker has already been seeded from disposable Opus and Trusted
baseline checkouts. Treat that as the starting point. If the seed needs to be
recreated, use the baseline commits and commands documented above rather than
writing into either active run tree.

Preferred evidence:

```sh
rg -n 'make test-report ACTIVE_TEST_REPORT_PAS=.pa23.|TEST SUMMARY|pa23/tests/' \
  /home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl
```

The run log contains command outputs and file reads from the Opus agent. Search
for exact test names to find when the agent inspected a test, what failure it
observed, and what it believed the root cause was:

```sh
TEST='pa23/tests/general/100-call-operator-template-use-scope-shadowing.t'
rg -n "$TEST|$(basename "$TEST" .t)" \
  /home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl
```

Do not rely only on final pass/fail. The important question is whether the test
failed at PA23 start and what change made it pass.

### 2. Build the Opus Commit Timeline

Start with the Opus PA23 commit sequence, not with 340 independent test
investigations. The Opus run made progress commits, and those commits usually
contain the clearest feature-level evidence: commit message, `pa23/plan.md`
state, source diff, and nearby test-summary deltas.

Read-only commands:

```sh
git -C /home/vishvananda/work/opus log --oneline --reverse --grep='pa23' --all
git -C /home/vishvananda/work/opus show --stat <commit>
git -C /home/vishvananda/work/opus show --patch <commit> -- dev/src pa23/plan.md
git -C /home/vishvananda/work/opus show <commit>:pa23/plan.md
```

For each PA23 commit, record:

- the commit message and touched source files;
- the `pa23/plan.md` diff or full historical plan text;
- any nearby `make test-report ACTIVE_TEST_REPORT_PAS='pa23'` summary;
- tests explicitly named in command output, plan text, or the commit message;
- the feature the agent believed it was implementing.

This produces feature clusters first, then test rows can be attached to those
clusters.

### 3. Fill Tracker Rows From Feature Clusters

Use the Opus timeline to fill the tracker in batches. For each feature cluster,
search the seeded failure tracker, Opus run log, and historical plan text for
tests that match the feature.

Useful searches:

```sh
TEST='pa23/tests/general/100-call-operator-template-use-scope-shadowing.t'
BASE=$(basename "$TEST" .t)
rg -n "$TEST|$BASE" \
  /home/vishvananda/work/.ralph/opus-opus-xhigh/events/run.jsonl

git -C /home/vishvananda/work/opus log --oneline --reverse --grep='pa23' --all
git -C /home/vishvananda/work/opus show <commit>:pa23/plan.md
```

When a commit names a feature but not a test, search the run log around that
time and search for representative test names. Opus often documented progress in
`pa23/plan.md`; historical versions of that file are valuable evidence.

Do not force every row to have an exact first-passing commit immediately. If the
feature cluster is clear but the exact passing transition is not, fill the
cluster, classification hypothesis, and evidence notes, then mark the row
`needs-reducer` or `needs-owner-decision`.

### 4. Investigate Unclear Tests One by One

After the commit/feature pass, work test-by-test only for rows that remain
unclear. This avoids spending most of the audit on mechanical search while still
preserving per-test accountability.

If the first passing commit is not obvious, use one of these safe approaches:

- infer from the nearest plan status update and commit message, and mark the row
  as `needs-reducer`;
- make a disposable clone of `/home/vishvananda/work/opus` outside the active
  tree and run a targeted binary search there;
- manually apply the relevant fix idea in this audit worktree and validate a
  reduced test.

Do not run the binary search in `/home/vishvananda/work/opus`.

### 5. Use Trusted as Secondary Evidence

The Trusted baseline is useful as a cross-check, especially for tests that passed
in one run and failed in the other. However, Trusted did not make the same clean
progress commits for PA23, so extracting its reasoning requires parsing the turn
JSON and command transcript.

Use Trusted evidence only when Opus leaves an important question unresolved, for
example:

- a test is `trusted_only` or `opus_only` and the Opus history does not explain
  why;
- two plausible owner PAs exist and Trusted's reasoning helps disambiguate;
- the Opus fix looks like an implementation accident rather than a feature;
- a feature cluster needs independent confirmation before adding earlier tests.

Trusted evidence sources:

```sh
rg -n '<test-name-or-feature-term>' \
  /home/vishvananda/work/.ralph/trusted-gpt-5.5-xhigh/events/run.jsonl
git -C /home/vishvananda/work/trusted show --stat <commit>
git -C /home/vishvananda/work/trusted show --patch <commit> -- dev/src pa23/plan.md
```

Keep Trusted findings in the tracker notes as corroborating evidence, not as the
primary workflow.

### 6. Reduce the Feature

For each initially failing PA23 test that appears to have required a new feature,
reduce it to the smallest source program that demonstrates the missing behavior.

Reduction rules:

- Preserve the single semantic feature under review.
- Remove unrelated PA23 composition scaffolding.
- Avoid libc++/hosted-header dependency unless the feature itself is hosted
  behavior.
- Prefer one assertion in `main` and simple exit-status checks.
- Keep the reduced program within the syntax and semantic surface of the
  proposed earlier PA.
- If the feature needs multiple earlier features, place it at the earliest PA
  where all required ingredients are documented.

### 7. Determine the Owner PA

For each missing feature, read the relevant READMEs before assigning ownership.
Do not guess from PA numbers alone.

Likely source areas to inspect:

```sh
pa18/README.md
pa19/README.md
pa20/README.md
pa21/README.md
pa22/README.md
pa23/README.md
```

Decision rules:

- If the feature is explicitly required by PA18-PA22 but had no focused test,
  add a focused test to that PA.
- If PA23 combines features from several earlier PAs, add the reduced test to
  the earliest PA where all features are available.
- If no earlier README owns the feature, create a `missing-feature-owner-unclear`
  tracker row and propose a specific owner PA plus README language.
- If the failure was caused only by an Opus implementation bug and the feature is
  already well-covered earlier, mark `implementation-bug-only`.
- If the PA23 test is intentionally only a broad integration case, mark
  `pa23-composition-only` and leave it in PA23.

### 8. Reducer Backfill Workflow

Use this workflow for each tracker row or small cluster that is being promoted
from `needs-reducer` to an earlier assignment test.

Start with the lowest-ambiguity rows:

- `classification=missing-earlier-feature` with a single concrete
  `candidate_owner_pa`;
- then `classification=feature-interaction-earliest-owner` once the owner PA is
  resolved to the earliest assignment where all ingredients are available;
- defer `missing-feature-owner-unclear`, `implementation-bug-only`,
  `harness-or-reference-issue`, and `pa23-composition-only` rows until their
  tracker status changes.

For the selected row or cluster:

1. Read the PA23 source test and the candidate owner README.
2. Check existing tests in the candidate owner PA and adjacent template PAs:

   ```sh
   rg -n '<feature terms>' pa18/tests pa19/tests pa20/tests pa21/tests pa22/tests
   sed -n '1,240p' paXX/README.md
   ```

3. Reduce the PA23 test to the smallest program that proves one semantic
   feature. Keep the reducer inside the proposed owner PA's language boundary.
   Do not preserve PA23 composition scaffolding unless the feature needs all of
   it.
4. Record the reducer text or temporary path in `minimal_example`, the intended
   earlier test path in `proposed_test_path`, and leave the row
   `needs-reducer` until the historical validation below passes.

Temporary reducers can live under `analysis/reducers/` while they are being
shrunk and historically validated. Once accepted, copy the final reduced program
into the earliest owner PA and keep the tracker pointing at that permanent test
path.

#### Opus Historical Validation For Reducers

Yes: a reducer should be validated against a disposable Opus checkout before it
is promoted to an earlier assignment test. This is the evidence that the reduced
program is actually tied to the Opus PA23 feature fix, not just a plausible
local example.

Required validation for every reducer that will become an earlier test:

- The reducer must fail on the Opus PA23 start anchor:
  `1963d796e pa22: audit findings, changes, and architecture review`.
- The same reducer must pass at the Opus commit identified as fixing the feature
  cluster, or at the earliest commit found by a targeted bisection.
- "Fail" means fail against the intended oracle, not necessarily exit nonzero.
  Valid failures include rejected-valid diagnostics, accepted-invalid behavior,
  wrong LowIR, wrong witness output, a failing `static_assert`, or the wrong
  return value encoded in LowIR.
- "Pass" means the reducer behavior matches the oracle at the fixing commit and
  the current audit worktree passes the same focused check after the test is
  added.
- For negative tests, the expected behavior is usually a compile failure. In
  that case the start-anchor failure may be "accepted invalid source", and the
  fixing-commit pass is the expected nonzero diagnostic.
- The failure at the start anchor should be the same kind of failure as the
  seeded PA23 row when practical: rejected-valid, accepted-invalid, LowIR
  mismatch, or sanity-validation failure. Exact diagnostic text is not required.
- If the reducer already passes at the start anchor, it is not proving the
  missing feature. Shrink or choose a different reducer.
- If the reducer still fails at the supposed fixing commit, the commit evidence
  is too broad. Search the Opus timeline or bisect in the disposable clone, then
  update the tracker evidence.
- If no exact fixing commit can be isolated cheaply, keep the row
  `needs-reducer` or `needs-owner-decision` and document the uncertainty in
  `notes`; do not add the earlier test yet.
- If the reducer passes historical/current validation but the external
  reference workflow rejects it or produces an incompatible reference, mark the
  tracker row `harness-or-reference-issue` / `no-action` and add an entry to
  `analysis/reference-reducer-failures.md` with the reducer path, reference
  diagnostic, current behavior, and historical evidence.

Never run this validation in `/home/vishvananda/work/opus` or
`/home/vishvananda/work/trusted`. Create a clone outside the active run tree:

```sh
AUDIT=/home/vishvananda/cppgm-pa23-feature-audit
OPUS_TMP=/tmp/cppgm-pa23-opus-reducer
REDUCER="$AUDIT/analysis/reducers/<reducer>.t"
rm -rf "$OPUS_TMP"
git clone --no-hardlinks /home/vishvananda/work/opus "$OPUS_TMP"
```

The reducer path may point back into the audit worktree; the disposable clone
only reads that source file and writes its own build products under `/tmp` and
its cloned `dev/` object tree.

Then test the reducer at the PA23 start anchor. Rebuild after each checkout, and
use the same temporary reducer path for both commits:

```sh
cd "$OPUS_TMP"
git checkout 1963d796e
make -C dev cppgm++ -j"$(getconf _NPROCESSORS_ONLN)"
./dev/cppgm++ --emit-lowir -O0 -o /tmp/reducer-start.lowir "$REDUCER"
echo "start exit: $?"
```

And at the identified feature-fix commit:

```sh
cd "$OPUS_TMP"
git checkout <opus-fix-commit>
make -C dev cppgm++ -j"$(getconf _NPROCESSORS_ONLN)"
./dev/cppgm++ --emit-lowir -O0 -o /tmp/reducer-fix.lowir "$REDUCER"
echo "fix exit: $?"
```

For a reducer that is expected to fail compilation, invert the pass/fail
expectation accordingly and compare exit status only. For a reducer that is
expected to produce LowIR, inspect or compare the output only as needed to prove
the feature. If the start compiler exits 0, compare the emitted LowIR or witness
facts against the known-good oracle rather than treating exit status as success.
Do not regenerate assignment references in the disposable clone.

After the historical validation passes:

- set `action_status=ready-to-add-test`;
- tighten `opus_commit_evidence` to the exact fixing commit when known;
- mention the start-anchor failure and fixing-commit pass in `notes`;
- then add the earlier assignment test in the audit worktree.

### 9. Add Earlier Tests

When a row is `ready-to-add-test`, add the reduced test in the selected earlier
PA using local test naming conventions.

Before adding a test:

```sh
cd /home/vishvananda/cppgm-pa23-feature-audit
rg -n '<important syntax or feature>' pa*/tests cppgm.tests/course
sed -n '1,220p' paXX/README.md
```

After adding a test, generate or update references using the existing extended
repo workflow. Use the same style as adjacent tests in that PA. Then run focused
validation before broad validation.

For PA18-PA24 source-to-LowIR tests, generate focused references with the
external reference binary rather than the current compiler:

```sh
REF_BIN=/home/vishvananda/work/phases/reference-binaries/cppgm++
make -C paXX ref-test TEST=tests/<suite>/<new-test>.t REF_TEST_APP="$REF_BIN"
```

For tests that need witness refs, generate only the focused witness output from
inside the assignment directory:

```sh
cd /home/vishvananda/cppgm-pa23-feature-audit/paXX
CPPGM_APP_ARGS='--emit-lowir -O0' \
  ../scripts/run_witness_tests.pl "$REF_BIN" ref tests/<suite>/<new-test>.t
```

Normalize accidental absolute paths in witness refs before review:

```sh
perl -pi -e 's#/home/vishvananda/cppgm-pa23-feature-audit/##g' \
  paXX/tests/<suite>/<new-test>.ref.witness
```

Then run focused current-compiler checks before broad validation:

```sh
make -C paXX check TEST=tests/<suite>/<new-test>.t

cd /home/vishvananda/cppgm-pa23-feature-audit/paXX
CPPGM_APP_ARGS='--emit-lowir -O0' \
  ../scripts/run_witness_tests.pl ../dev/cppgm++ check tests/<suite>/<new-test>.t
../scripts/compare_witness_results.pl ref check tests/<suite>/<new-test>.t
```

Run the placement audit after the reducer is in its proposed owner PA and before
counting the backfill as accepted. A finding from `--fail-on-early` means the
reducer still contains a later-owned feature or is in too early a cluster; reduce
it further or move it to the owning PA before generating final refs:

```sh
python3 scripts/audit_pa_feature_placement.py --pa paXX --no-course \
  --markdown-out /tmp/paXX-placement-audit.md \
  --json-out /tmp/paXX-placement-audit.json \
  --fail-on-early
```

Typical broad validation:

```sh
make test-report ACTIVE_TEST_REPORT_PAS='paXX'
make test-report-through-paXX
perl scripts/cppgm_file_audit.pl --stage paXX --paths dev/src
```

For tests that affect LowIR or strict references, also run with the lowir compare
mode used by the repo/CI if the target PA requires it.

### 10. Keep Changes Reviewable

Commit in small groups:

- one commit for the tracker setup if needed;
- one commit per feature cluster or owner PA;
- README updates in the same commit as the tests they explain;
- avoid mixing unrelated test-placement fixes.

Each test-adding commit should include:

- the reduced test;
- generated references;
- README or assignment text if the feature owner was unclear;
- tracker updates for the affected rows.

## Initial Feature Clusters From Opus History

These are starting hypotheses only. Confirm each one against the actual test,
Opus fix commits, and assignment READMEs.

- Template-template parameter deduction and partial specialization matching.
- Alias templates in dependent/non-deduced contexts.
- Variable-template and static-data-member specialization interactions.
- Explicit specialization and explicit instantiation of class-template members.
- Out-of-class member function template definitions and replay into
  instantiations.
- Inherited constructor templates from dependent bases.
- Non-type template parameter deduction, including array bounds and nested
  template-ids.
- Pack expansion in function types, constructor initializers, static data
  members, and co-varying function parameter packs.
- `decltype` and dependent qualified type/value use in template arguments.
- Current specialization and injected-class-name behavior in member templates.
- Itanium mangling interactions for local classes, member class templates, and
  repeated function-template instantiations.
- Vendor builtins that PA23 forced Opus to add, such as `__type_pack_element`
  and `__make_integer_seq`; these need careful owner decisions because a vendor
  builtin may be a missing earlier primitive or may be too library-specific for
  assignment requirements.

## Final Deliverables

The audit is complete when:

- every PA23 test has a tracker row;
- every initially failing PA23 test has evidence linking it to a fix, a feature
  cluster, and a classification;
- every `missing-earlier-feature` row has a minimal example and an owner PA;
- all accepted earlier-PA tests have been added with references;
- any feature whose owner is unclear has proposed README wording;
- validation for all touched PAs passes;
- the final tracker explains which PA23 tests remain PA23-only composition tests.
