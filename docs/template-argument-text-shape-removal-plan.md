# Template Argument Text-Shape Removal Plan

## Goal

Remove the function-template deduction fallback currently named
`deduce_template_argument_text_shape` in `dev/src/template_resolution.cpp`.
This helper does not call the old type/AST fragment parsers, but it still
recovers semantic template-argument structure from strings. That makes it part
of the same debt class as the reparse fallbacks: source spelling is being used
as a recovery path after typed `TemplateArgument` / `TypePtr` data is missing or
incomplete.

The final state is:

- no `deduce_template_argument_text_shape` helper
- no `deduce-text-shape` trace sites
- no function-template deduction path that compares or splits template argument
  spelling to recover semantic facts
- the text-reparse audit tracks this class and is at zero
- strict LowIR compare and the standard perf gate are clean

The current Bimap/MP11 partial-order WIP is shelved as stash commit
`b50a2856cf6608595429f724c347c09ceda9500d`, created on
`Thu Jul 2 12:51:13 2026` and currently visible as:

```text
stash@{0}: wip-bimap-mp11-partial-order-paused: structured partial-order placeholder path; PA22/PA23 reducers passed; perf not finalized; paused before relying on/removing deduce_template_argument_text_shape legacy fallback
```

Do not resume that WIP by leaning on the text-shape helper. Once this plan is
implemented, reapply the Bimap stash and port it onto the structured deduction
surface.

## Current Findings

The helper is present on the current branch and on `origin/main`. In this
branch ancestry, `git log -S'deduce_template_argument_text_shape'` attributes it
to `5617969f9 Initial commit`; later commits expanded it. It was not
introduced by the current Bimap work.

The non-ancestor cleanup commit `47dddb977 Reduce semantic template text
fallbacks` shows the likely history: a worse parser-backed fallback was removed,
but this local string-shape comparator remained as an allowed-looking recovery
path. That means the issue was missed by the text-reparse cleanup, not newly
created.

Current sites in `dev/src/template_resolution.cpp`:

- helper declaration/definition near `deduce_template_argument_impl`
- suffix stripping for `*`, `&`, and `&&`
- builtin type spelling recognition
- direct template-parameter matching from argument text
- template-id splitting through `semantic_utils::split_top_level_template_id_text`
- string lookups through `lookup_pattern_arg_type` / `lookup_actual_arg_type`
- one outer call from parsed template-id matching

The existing `scripts/audit_text_reparse.py` does not catch this because it
only scans explicit parser bridges (`parse_type_fragment`,
`parse_template_id_string`, `ctx.parse_type_text`, etc.). This helper is ad hoc
text interpretation rather than a named parser bridge.

## Audit Plan

Add a new category to `scripts/audit_text_reparse.py`, with a name such as
`template_argument_text_shape_deduction`.

Initial pattern should be focused and low-noise:

```python
re.compile(
    r"\bdeduce_template_argument_text_shape\b|"
    r"\bdeduce-text-shape\b"
)
```

Add a script test for this category under `scripts/tests/` so future helper
renames cannot hide the same pattern. The test should use a small temporary
source root and assert that:

- `deduce_template_argument_text_shape` is counted
- `deduce-text-shape-*` trace labels are counted
- unrelated diagnostic text is not counted

Use the audit as a ratchet:

1. In the first audit commit, add the category and set the baseline limit to the
   current count so normal audit runs remain usable while this plan is active.
2. During cleanup, lower the baseline in the same commit that removes sites.
3. In the final removal commit, the category limit must be `0`, and
   `python3 scripts/audit_text_reparse.py --strict` must pass.

Do not leave the category permanently baseline-tolerated. Its purpose is to
force the final deletion.

## Removal Strategy

The core workflow is the same one used for the earlier text fallback removals:
remove or disable the fallback first, run the relevant tests, and fix the
missing structured data at the producer boundary.

### Stage 0: Baseline

Start from a clean tree.

- Build `dev/cppgm++`.
- Record a fresh perf baseline with `scripts/validate_perf_regression.py
  record --runs 3`.
- Run the audit and confirm the new category reports only the known helper.
- Run a focused `test-report` / `test-strict` with LowIR direct compare before
  changing deduction behavior.

### Stage 1: Disable the Fallback

Make `deduce_template_argument_text_shape` unreachable or make the outer call
return false. Keep the edit small and intentionally red.

Then run:

- focused PA suites around templates: `pa18 pa19 pa21 pa22 pa23`
- `test-strict` with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1`
- a broader `test-report` once the focused failures are understood

Classify each failure by the missing structured fact, not by the spelling that
used to make the fallback work.

Expected missing-data classes:

- pattern and actual template-id arguments are available only as
  `argument_texts`, while `arguments` is empty or not aligned
- defaulted template arguments are trimmed as text without carrying aligned
  `TemplateArgument`
- dependent alias/class-template metadata preserves text but drops non-type or
  template-template argument identity
- synthetic deduction views carry source text but not `TemplateArgumentSyntax`
  or typed argument vectors

### Stage 2: Replace With Structured Matching

Introduce or extend structured helpers in the deduction path. Acceptable inputs
are:

- `TemplateArgument`
- `TemplateArgumentSyntax`
- `TypePtr`
- `ClassTemplateDecl` / `AliasTemplateDecl` identity
- already-decomposed template instantiation metadata
- source/declaration scope handles required for lookup

Do not pass newly synthesized type text into lookup, parser, or string-splitting
helpers.

Structured matching should cover the cases that the helper currently handles:

- pointer/reference shape through `TypePtr` recursion
- cv qualification through `TypePtr` flags
- builtin type identity through fundamental `Type`
- direct type and non-type template parameters through `TemplateParameterInfo`
  plus structured actual arguments
- nested template-id shape through `source_template` identity and aligned
  `TemplateArgument` vectors
- template-template parameter deduction through structured template entity
  identity

If a case cannot be expressed structurally, fix the producer that dropped the
data. Do not add a string classifier as a bridge.

### Stage 3: Delete the Helper

Once focused and broad tests are green:

- delete `deduce_template_argument_text_shape`
- delete the `deduce-text-shape` trace labels
- delete any local string helpers that only served that fallback
- lower the new audit baseline to zero

Run:

```sh
python3 scripts/audit_text_reparse.py --strict
python3 scripts/audit_semantic_template_boundary.py --strict
python3 scripts/audit_template_boundary.py
```

### Stage 4: Full Validation

Required before committing the final source removal:

```sh
make -C dev cppgm++ -j8

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
  ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22 pa23' \
  ORDERED=false TEST_REPORT_ASSIGNMENT_JOBS=1 TEST_REPORT_SUBTEST_JOBS=8 \
  CPPGM_SKIP_DEV_REBUILD=1 make test-report-nobuild

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 STRICT_SUBTEST_JOBS=8 \
  CPPGM_SKIP_DEV_REBUILD=1 make test-strict-nobuild

scripts/validate_perf_regression.py check \
  --baseline /tmp/<new-text-shape-removal-baseline>.json --runs 3
```

Run a full `make test-report` with LowIR direct compare once the focused lane is
green. Do not run separate `test-report` invocations concurrently.

## Performance Requirements

Instruction count and retained/peak memory should be the primary signals. Wall
time is secondary and expected to be noisy.

Temporary regressions are acceptable while the tree is intentionally red, but
the final removal should be at or below the Stage 0 baseline on retired
instructions and within the standard memory tolerance. If the structured
replacement is allocation-heavy, stop and sample before committing.

Useful diagnostics while iterating:

- `CPPGM_SEMANTIC_PHASE_STATS=1`
- `CPPGM_SEMANTIC_STATS=1`
- semantic hotspot tracing for repeated deduction queries
- one-run perf checks for quick iteration before the final three-run gate

## Commit Strategy

Keep the cleanup split into coherent commits:

1. audit category + plan/tracker update
2. focused structured-data plumbing slices with reducers
3. final helper deletion + audit limit zero + full validation

If a reducer is created while the tree is red, commit it with the fix that makes
it pass. Avoid accumulating untracked reducers across stages.
