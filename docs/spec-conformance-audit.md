# Spec Conformance Audit And Execution Process

## Purpose

This document is both:

- the backlog of suspected C++11 conformance gaps
- the process tracker for working through them one by one

The authority for expected behavior is [`n3485.txt`](../n3485.txt), the C++11 working draft already checked into the repo. The audit is only a hypothesis list; each item must be verified against the spec text before we change code or bless a test.

## Working Loop

Every audit item should be handled with the same sequence:

1. Verify the suspected mismatch against `n3485.txt`.
   - Read the cited clause and any immediately adjacent wording that affects preconditions, ranking, or exceptions.
   - If the audit claim is wrong or incomplete, update this document first.

2. Check whether the behavior is already covered by an existing test.
   - Search `pa*/tests`, `course/`, and existing reducers.
   - If a good existing test already covers the exact spec point, record it in the tracker and use that as the regression.

3. If coverage is missing, add the smallest test that captures the rule.
   - Prefer the earliest PA that can express the language feature.
   - If the failure only appears in hosted/STL code, first try to reduce it to a smaller language-level test in an earlier PA.
   - Keep the hosted regression too when it provides important coverage that the reduced test does not.

4. Run the targeted test before changing code.
   - If it already passes and matches the verified spec behavior, mark the item `covered` or `not-a-bug`.
   - If it fails, keep the new regression and fix the implementation in `dev/`.

5. Fix the implementation.
   - Bias toward the narrowest semantic fix that aligns behavior with the cited N3485 rule.
   - If the fix reveals a broader structural cleanup, note it separately, but do not widen the patch unless it is needed for correctness.

6. Validate.
   - Rerun the targeted regression(s).
   - Run full root `make test-report`.
   - If LowIR refs churn, only update refs when:
     - the change is naming/order-only and codegen-equivalent, or
     - the new output is demonstrably more correct

7. Record the outcome in this file.
   - Update status, test coverage, and commit.
   - If the item is blocked by another audit item, mark it `deferred` and say why.

## Status Key

- `fixed`: verified against spec, regression exists, implementation fixed, full regression green
- `covered`: verified against spec, existing behavior already matches, regression exists
- `needs-test`: spec point verified, but no committed regression yet
- `needs-fix`: regression exists and currently fails
- `deferred`: verified issue, but blocked on another prerequisite
- `not-a-bug`: audit suspicion was wrong after checking `n3485.txt`
- `todo`: not yet worked through

## Tracker

| ID | Topic | N3485 focus | Priority | Status | Coverage | Notes / checkpoint |
|---|---|---|---|---|---|---|
| 1 | Structural CV qualification deduction | 14.8.2.5 | High | fixed | yes | Fixed by `5878206f` |
| 2 | Structural alias template expansion | 14.5.7 | High | fixed | yes | Fixed by `184acfdf` |
| 3 | Multiple pack expansions in deduction | 14.8.2.5 p9 / 14.5.3 | High | not-a-bug | n/a | C++11 makes template-argument-list deduction non-deduced if a pack expansion is not the last template argument; current direct multi-pack rejection in this path is aligned |
| 4 | Structural pack parameter expansion | 14.7 / 14.8 paths | High | fixed | yes | Fixed by `724fce05` |
| 5 | Partial ordering deduction fidelity | 14.8.2.4 | Medium | fixed | yes | Added `pa18/213`; corrected template-instantiation candidate dedup so partial ordering sees distinct template entities; refreshed `pa21/409` for the more-specialized overload |
| 6 | ADL for non-type/template template arguments | 3.4.2 / 6.4.8.2 wording in draft | Medium | fixed | yes | Non-type arguments were already correctly ignored; fixed missing associated-namespace contribution for template-template arguments with `pa21/440` |
| 7 | Reference binding conversion ranking | 13.3.3.1.4 | Medium | covered | yes | Added `pa12/339`; current behavior matches the standard examples for lvalue/rvalue reference ranking and the function-lvalue tie-break |
| 8 | SFINAE candidate pruning vs fail-fast | 14.8.2 p8 | Medium | covered | yes | Existing `pa21/182` and `pa21/286` already cover substitution-failure candidate drop/fallback behavior; current implementation prunes rather than hard-failing |
| 9 | Explicit scope hiding behavior in unqualified lookup | 3.4.1 / 7.3.4 | Medium | fixed | yes | Added `pa16/329` and `pa16/330`; fixed using-directive injection scope plus same-level direct/imported ambiguity handling |
| 10 | Deduction order too strictly left-to-right | 14.8.2.5 p2/p6 | Low | fixed | yes | Added `pa21/441`; dependent qualified-id parameter is now treated as non-deduced so later `P/A` pairs can supply the template argument |
| 11 | Aggregate constructor synthesis validity | 8.5.1 | Low | fixed | yes | Added `pa15/256`-`260` and refreshed existing `pa26/183`; aggregate eligibility now matches C++11 for brace-or-equal members, access, bit-fields, and defaulted/deleted constructors |
| 12 | Member pointer conversions grouped with object pointers | 4.11 | Low | fixed | yes | Added compile-only `pa15/261`; base-to-derived pointer-to-member conversion now follows `conv.mem` in semantic initialization/conversion paths. `.*` / `->*` expression support remains a separate gap. |
| 13 | Qualification conversion ignores member-pointer chains | 4.4 | Low | fixed | yes | Added compile-only `pa15/262`; mixed pointer/member-pointer qualification conversion now follows the structural `conv.qual` rules. `.*` / `->*` expression support remains a separate gap. |

## Execution Rules

### Spec-first

Do not treat this audit as authoritative on its own. The correct sequence is:

1. audit claim
2. N3485 verification
3. regression
4. fix

If step 2 disproves the audit claim, update the audit and stop there.

### Regression placement

- Put the regression in the earliest PA that naturally covers the language rule.
- Use PA34 or PA35 only when the behavior depends on hosted library interaction, hosted
  emitted-code behavior, or compiler builtins.
- If a hosted failure reduces to a smaller pure-language reproducer, add the earlier-PA test first.

### Reference updates

- Prefer keeping existing refs unchanged when possible.
- If refs change, explain whether the change is:
  - neutral naming/order churn, or
  - a real correctness fix

### Commit shape

For each audit item, prefer one checkpoint commit that includes:

- the regression
- the implementation fix
- any justified ref updates
- tracker updates in this file

If the item needs a preparatory cleanup first, use the same pattern as other repo plans:

1. preparatory refactor with full `make test-report`
2. commit
3. regression + fix with full `make test-report`
4. commit

## Recommended Per-Item Checklist

Use this checklist when actively working an item:

- `spec`: cite the exact N3485 clause/paragraph used
- `coverage-existing`: existing test path or `none`
- `coverage-added`: new test path if needed
- `result-before-fix`: pass / fail / no-bug
- `implementation`: files changed in `dev/`
- `refs`: `none`, `neutral churn`, or `correctness update`
- `validation`: targeted tests + full `make test-report`
- `commit`: checkpoint commit hash

## Current Notes

- Items `1`, `2`, and `4` were effectively resolved while executing [structural-type-manipulation-plan.md](implemented/structural-type-manipulation-plan.md).
- The next natural active item is `3` because it is still in the template deduction core and may affect several later audit entries.
- Items `12` and `13` should probably be worked together because both concern member-pointer conversion treatment.
