# PA22 Template Placement Tracker

This tracker is the review queue for template test placement across PA22.
It supports the split into:

- PA18/PA19/PA21 basic template owners
- PA22 advanced single-feature template completion
- PA23 template integration
- later owners, split/reduce, or drop decisions

The table below was seeded by the template-placement audit mode.
Treat the bucket and cluster as review leads, not final move decisions.
Filename-only matches are retained as path hints and do not drive placement failures.
After review starts, do not overwrite this tracker without preserving status and notes.

Seed command:

```sh
python3 scripts/audit_pa_feature_placement.py --pa pa22 --no-course --template-placement \
  --markdown-out docs/pa22-template-placement-tracker.md \
  --csv-out /tmp/template-placement.csv \
  --json-out /tmp/template-placement.json
```

Status legend: `[ ]` todo · `[~]` in progress · `[x]` placed · `[D]` dropped · `[-]` deferred

## Review Rules

- A test goes to the earliest PA/cluster that owns the behavior it asserts.
- Support syntax does not control placement when it is already implemented and not essential to the expected output.
- If two or more template concepts are essential together, place the test in PA23 integration and cluster it by the feature combination.
- If a later non-template feature is essential, move later or split/reduce the test before keeping template coverage.
- Witness refs are golden; do not regenerate witness refs while moving tests.

## PA23 Candidate Clusters

| Cluster | Intended integration shape |
| --- | --- |
| 100 | dependent-name/entity interactions that do not fit a narrower later cluster |
| 200 | deduction, partial ordering, non-deduced contexts, and braced-init deduction combinations |
| 300 | SFINAE, substitution, detector idiom, and no-eager instantiation combinations |
| 400 | pack, member-template, template-template-parameter, alias-template, and variable-template compositions |
| 500 | library-shaped end-to-end reducers without hosted/builtin dependencies |

## Generated Summary

- tests scanned: 51
- feature table entries without detector rules: 0
- pa22-advanced-single-candidate: 51

## 2026-06-19 Placement Refresh

The refreshed template-placement classifier now uses filename/path hints as
review evidence while keeping enforced placement failures source/ref-backed
only. PA22-owned deduction, partial-ordering, constructor-deduction, SFINAE,
and no-eager-instantiation rows are therefore classified by their PA22 feature
instead of by the earlier template machinery they use as support.

Current live PA22 result: all 51 scanned rows are PA22 advanced single-feature
candidates and have been reviewed as placed. There are no live `basic-owner-candidate`,
`pa23-integration-candidate`, or `later-owner-or-split` rows. The earlier
PA22-to-PA23 integration cleanup remains resolved; the 17 moved tests are
recorded in the Boost frontier tracker entry for commit `74dd5b69d`.
The no-eager reducer previously left in cluster 500 has been moved to cluster
300, matching the PA22 owner row.

## Review Queue

| Status | Test | Current | Bucket | Concepts For Review | Later/Compat Features | Latest Template Owner | PA23 Cluster | Late Candidate | Late Confidence | Path Hints | Action | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [x] | `pa22/tests/general/100-bad-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-bad-function-template-deduction-cv-mismatch-call.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-decltype-function-template-deduced-call.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-defaulted-class-template-arg-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:200` | `specific` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-defaulted-nested-class-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:200` | `specific` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-derived-base-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-explicit-function-template-id-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-explicit-template-id-same-signature-free-functions.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-explicit-template-id-user-conversion-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-forwarding-reference-lvalue-overload.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `` | `` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-forwarding-reference-qualified-enumerator.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `` | `` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-reference-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-alias-parameter-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `` | `` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-array-to-pointer-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-const-ref-top-cv-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-defaulted-class-template-arg-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:200` | `specific` | template.deduction_full, template.default_argument | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-function-template-elaborated-top-cv-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-local-class-declval-explicit-template-id.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-overload-set-unique-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-pointer-qualification-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/100-template-array-reference-cv-default-arg.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:200` | `specific` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. | Reduced to a non-polymorphic PA22 deduction/default-argument guard and kept a complete-constructor anchor so normal and witness LowIR request the same symbols; full virtual cleanup fixture moved to `pa24/tests/general/200-template-array-reference-cv-polymorphic-cleanup.t`. |
| [x] | `pa22/tests/general/200-ambiguous-cv-pointer-partial-ordering-bad.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/200-function-pointer-vs-const-ref-partial-order.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/200-function-template-partial-order-class-template-cv.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/200-function-template-partial-order-const-pointer.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/200-partial-ordering-pointer-vs-value.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/200-partial-ordering-ref-vs-const-ref.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-abstract-array-parameter-sfinae.t` | `pa22:300` | `pa22-advanced-single-candidate` | sfinae |  | `pa22:300` |  | `pa21:300` | `specific` | sfinae | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-class-member-forward-template-alias-no-eager-complete.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa18:100` | `broad-owner-only` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-class-member-forward-template-pointer-no-eager-complete.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa18:100` | `broad-owner-only` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-constructor-template-default-arg-target-aware.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | lifetime.ctor_dtor, template.constructor_deduction, template.default_argument | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-constructor-template-dependent-alias-target-aware.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | lifetime.ctor_dtor, template.constructor_deduction, template.dependent_name | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-decltype-conditional-no-body-instantiation.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa19:100` | `specific` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-defaulted-sfinae-ctor-candidate-drop.t` | `pa22:300` | `pa22-advanced-single-candidate` | sfinae |  | `pa22:300` |  | `pa21:300` | `specific` | sfinae, template.substitution | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-dependent-class-function-body-skip.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa18:100` | `broad-owner-only` | template.dependent_name, template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-nontemplate-copy-move-beat-converting-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | template.constructor_deduction | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-parameter-type-no-eager-member-body.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa18:100` | `broad-owner-only` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-reentrant-class-template-copy-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | template.constructor_deduction | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-reentrant-pair-template-copy-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | template.constructor_deduction | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-worse-conversion-candidate-body-not-instantiated.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa21:300` | `specific` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/general/300-dependent-template-id-no-eager-layout.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa21:200` | `specific` | template.dependent_name, template.no_eager_instantiation | Placed in PA22 cluster 300. | Moved from `500` to match the PA22 no-eager owner. |
| [x] | `pa22/tests/spec/100-defaulted-nested-class-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:200` | `specific` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/100-explicit-template-args-plus-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/100-explicit-template-argument-overload-rejects-short-candidate.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa21:300` | `specific` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/100-function-template-array-reference-return-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/100-function-template-array-to-pointer-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | `pa18:100` | `broad-owner-only` | template.deduction_full | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/200-function-template-partial-order-const-pointer.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | `pa18:100` | `broad-owner-only` | template.function_partial_ordering | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/300-constructor-template-cross-specialization.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | `pa21:300` | `specific` | lifetime.ctor_dtor, template.constructor_deduction | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/300-expression-sfinae-decltype-conversion.t` | `pa22:300` | `pa22-advanced-single-candidate` | sfinae |  | `pa22:300` |  | `pa21:300` | `specific` | sfinae | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/300-using-class-template-does-not-instantiate-array-member.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | `pa18:100` | `broad-owner-only` | template.no_eager_instantiation | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
