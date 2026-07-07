# Opus PA35/PA36 Reducer Extraction Plan

This plan describes how to mine the Opus run for missing focused tests from
the PA35 and PA36 implementation work. The goal is to turn each real compiler
fix from that run into a minimal, properly placed reducer in `cppgm-extended`.

The Opus checkout is evidence only. Do not edit it, commit in it, reset it, or
run commands that mutate its working tree. Use disposable worktrees for all
adjacent-commit validation.

## Objective

For every Opus commit from the first PA35 implementation commit through the
commit where PA36 first became fully passing:

1. Identify whether the commit fixed one or more tests.
2. Understand the underlying compiler issue, not just the hosted STL symptom.
3. Build a minimal non-STL reducer for that issue.
4. Place the reducer in the earliest assignment that owns the required feature.
5. Validate that the reducer fails before the Opus fix and passes after it.
6. Validate that the reducer passes in `cppgm-extended`.
7. Generate references, without witness refs, and commit the reducer separately.

Documentation-only Opus commits still need to be inspected because they often
explain the root cause and list the tests fixed by the surrounding code commits,
but they do not need reducer commits unless they reveal a missing test.

## Inputs

- Main implementation repo: `/home/vishvananda/cppgm-extended`
- Opus evidence repo: `/home/vishvananda/work/opus`
- Opus evidence is read-only.
- Use `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1` for report validation when a reducer
  exercises LowIR-output PAs.
- Use `python3 scripts/audit_pa_feature_placement.py --fail-on-early` to check
  that a reducer is not placed before its owning feature.

At the time this plan was written, the Opus history has clear commit subjects
prefixed with `pa35:` and `pa36:`. Derive the exact ranges from git rather than
hard-coding hashes in the workflow.

## Setup

Work on a fresh branch from current `origin/main` in `cppgm-extended`.

```sh
cd /home/vishvananda/cppgm-extended
git fetch origin
git switch -c opus-pa35-pa36-reducers origin/main
```

Create a scratch area for Opus worktrees. Keep it outside the live Opus tree.

```sh
OPUS=/home/vishvananda/work/opus
SCRATCH=/tmp/opus-pa35-pa36-reducers
mkdir -p "$SCRATCH"
```

Find the commit range.

```sh
FIRST_PA35=$(
  git -C "$OPUS" log --reverse --format='%H %s' |
    awk '/ pa35:/{print $1; exit}'
)

LAST_PA36=$(
  git -C "$OPUS" log --format='%H %s' --grep='^pa36:' |
    awk 'NR == 1 {print $1; exit}'
)

git -C "$OPUS" log --reverse --first-parent --format='%H %s' \
  "${FIRST_PA35}^..${LAST_PA36}" > "$SCRATCH/opus-pa35-pa36-commits.txt"
```

If the run history later proves non-linear, replace `--first-parent` with an
explicit ancestry range and record the reason in the tracker.

## Tracker

Create a tracker before adding reducers:

`docs/opus-pa35-pa36-reducer-tracker.md`

Use one row per Opus transition that might contain a fix.

| Status | Opus transition | Original tests fixed | Root cause | Reducer path | Target PA | Prev fails | Next passes | Extended passes | Audit | Commit |
|---|---|---|---|---|---|---|---|---|---|---|

Status values:

- `todo`: not examined yet.
- `evidence-only`: documentation or audit commit; no behavior change.
- `needs-reducer`: real fix found, reducer not written yet.
- `reduced`: reducer written and all validations pass.
- `covered`: issue already has an equivalent focused reducer; cite it.
- `blocked`: needs a decision because a non-STL reducer is not possible or the
  owning assignment is unclear.

## Per-Commit Workflow

Process the commits in chronological order.

For each adjacent pair `<prev> -> <next>`:

1. Inspect the commit subject and diff.

   ```sh
   git -C "$OPUS" show --stat --oneline <next>
   git -C "$OPUS" show --name-only --format=fuller <next>
   ```

2. Classify the commit.

   - Compiler-code change: likely needs reducer analysis.
   - Test/reference-only change: check whether it represents a missing upstream
     test or just local Opus bookkeeping.
   - `plan.md` or audit-only change: mine for evidence, then mark
     `evidence-only` unless it identifies an uncovered issue.

3. Determine which original tests changed status.

   Prefer evidence in this order:

   - Commit message progress markers such as `47->49`, `69/69`, or named tests.
   - Nearby `plan.md` or audit changes in the Opus diff.
   - Ralph JSON/session evidence for that turn, if the commit does not say
     which tests changed.
   - Direct before/after execution in disposable worktrees.

4. Create adjacent Opus worktrees only when validation requires executing the
   old and new compilers.

   ```sh
   rm -rf "$SCRATCH/prev" "$SCRATCH/next"
   git -C "$OPUS" worktree add "$SCRATCH/prev" <prev>
   git -C "$OPUS" worktree add "$SCRATCH/next" <next>
   ```

5. Confirm the original symptom, when practical.

   Use focused commands first. Avoid broad hosted-header or selfhost runs unless
   the commit cannot be understood any other way.

6. Reduce the issue.

   Start from the failing hosted test or from the root cause in the Opus notes,
   then remove STL and hosted-header dependencies. Preserve the language shape
   that exposed the bug: overload ranking, conversion, object lifetime, EH,
   layout, vtable emission, MIR lowering, LowIR serialization, or host ABI
   interaction.

7. Place the reducer.

   Put the reducer in the earliest PA whose README owns all required features.
   If a reducer requires a later feature than the original symptom appeared to
   use, place it at the later PA and record why. Do not move a test earlier just
   because the Opus bug was discovered while implementing PA35 or PA36.

8. Generate references.

   Generate only the refs required by that test bucket. Do not add witness refs
   for this extraction pass.

9. Validate the reducer against Opus.

   The reducer must fail at `<prev>` and pass at `<next>`. If it passes at both,
   it is not testing the actual fix. If it fails at both, either the reducer is
   incomplete or the Opus fix depended on another later commit; narrow the first
   passing commit before committing the reducer.

10. Validate in `cppgm-extended`.

    Run the focused test, the owning PA report, and relevant through/strict
    checks. Use direct LowIR comparison where appropriate.

11. Run placement audit.

    ```sh
    python3 scripts/audit_pa_feature_placement.py --fail-on-early \
      --markdown-out /tmp/pa-feature-placement-audit.md
    ```

12. Commit one reducer at a time.

    Each reducer commit should contain only the test, references, and any README
    clarification needed for that test. If a reducer requires implementation
    changes in `cppgm-extended`, split the implementation fix and the test into
    separate commits unless they cannot be validated independently.

## Reducer Rules

- Prefer no STL and no hosted headers. Replace `std::vector`, `std::function`,
  iostreams, locales, maps, strings, etc. with small test-owned classes or
  templates that preserve the relevant semantic shape.
- Keep the reducer source small. Remove unrelated constructors, methods,
  template parameters, includes, and runtime checks.
- If the issue is genuinely host ABI, EH, RTTI, or hosted-header compatibility
  and cannot be expressed without hosted entities, keep the hosted surface as
  small as possible and document why a non-STL reducer is not adequate.
- Validate host-language intent with `g++ -std=gnu++11` when the reducer is a
  positive C++ source test and the behavior is not cppgm-specific.
- Avoid tests that assert private libstdc++ or libc++ implementation names.
- Avoid adding mangling-only tests outside the ABI naming assignment. If the
  only issue is spelling, convert it to the ABI fact test surface instead.
- Do not add witness refs during this pass.

## Validation Commands

Exact commands vary by target PA and bucket, but each reducer needs the same
validation shape.

Focused test:

```sh
make -C paNN check TEST='tests/<bucket>/<name>.t'
```

Owning PA report:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ACTIVE_TEST_REPORT_PAS='paNN' \
  make test-report
```

Through check when the PA has a through target:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report-through-paNN
```

Strict checks when the reducer belongs to the strict LowIR/template band:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 STRICT_PAS='pa18 pa19 pa21 pa22 pa23' \
  make test-strict
```

Placement audit:

```sh
python3 scripts/audit_pa_feature_placement.py --fail-on-early \
  --markdown-out /tmp/pa-feature-placement-audit.md
```

If a command is too broad for the local iteration step, run the focused form
first, then record the broader command as pending in the tracker until it passes.

## Commit Message Format

Use commit messages that preserve traceability to Opus without depending on the
Opus repository for future understanding.

```text
paNN: add reducer for <short issue name>

Opus transition: <prev-short> -> <next-short>
Original symptom: <PA35/PA36 test or cluster>
Root cause: <one or two sentences>
Validation:
- previous Opus commit fails: <command/result>
- next Opus commit passes: <command/result>
- cppgm-extended passes: <commands>
- placement audit passes
```

## Completion Criteria

The extraction is complete when:

- Every Opus commit from the first PA35 commit through the final passing PA36
  commit is represented in the tracker.
- Every behavior-changing compiler fix is marked `reduced`, `covered`, or
  `blocked` with a concrete reason.
- All new reducers are committed individually.
- The full placement audit passes.
- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report` passes after the final
  reducer batch.
- No reducer introduces witness refs or unnecessary hosted/STL dependencies.

## Risks and Mitigations

- Opus commits may contain multiple fixes. Split them into multiple reducer rows
  and commits when the fixes are independent.
- A reducer may fail at more than one adjacent transition. Use Opus history to
  find the first commit where it passes, then update the tracker with that exact
  transition.
- A hosted symptom may reduce to an earlier core-language issue. Place the test
  at the earlier owning PA, not PA35/PA36.
- A hosted symptom may depend on private STL implementation structure. If a
  portable non-STL reducer is not possible, record the limitation and use the
  narrowest host-facing test that exercises the public requirement.
- The placement audit is conservative. Treat findings as review blockers until
  the target PA is justified or the test is moved.
