# PA22 Template Placement Tracker

This tracker is the review queue for template test placement across PA22.
It supports the split into:

- PA18/PA19/PA21 basic template owners
- PA22 advanced single-feature template completion
- PA24 template integration, using the removed PA24 slot until final renumbering
- later owners, split/reduce, or drop decisions

The table below was seeded by the template-placement audit mode.
Treat the bucket and cluster as review leads, not final move decisions.
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
- If two or more template concepts are essential together, place the test in PA24 integration and cluster it by the feature combination.
- If a later non-template feature is essential, move later or split/reduce the test before keeping template coverage.
- Witness refs are golden; do not regenerate witness refs while moving tests.

## PA24 Candidate Clusters

| Cluster | Intended integration shape |
| --- | --- |
| 100 | dependent-name/entity interactions that do not fit a narrower later cluster |
| 200 | deduction, partial ordering, non-deduced contexts, and braced-init deduction combinations |
| 300 | SFINAE, substitution, detector idiom, and no-eager instantiation combinations |
| 400 | pack, member-template, template-template-parameter, alias-template, and variable-template compositions |
| 500 | library-shaped end-to-end reducers without hosted/builtin dependencies |

## Generated Summary

- tests scanned: 55
- feature table entries without detector rules: 0
- later-owner-or-split: 2
- manual-review: 2
- pa22-advanced-single-candidate: 51

## Review Queue

| Status | Test | Current | Bucket | Concepts For Review | Later/Compat Features | Latest Template Owner | PA24 Cluster | Action | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [ ] | `pa22/tests/general/200-pack-expanded-base-instantiation.t` | `pa22:200` | `later-owner-or-split` | pack-expansion | template.alignas_alignof | `pa18:300` |  | Move later-owned behavior, or split/reduce to keep only the PA22 template assertion. |  |
| [ ] | `pa22/tests/general/500-dependent-template-id-no-eager-layout.t` | `pa22:500` | `later-owner-or-split` | alias-template, dependent-name, no-eager-instantiation | template.alignas_alignof | `pa22:300` |  | Move later-owned behavior, or split/reduce to keep only the PA22 template assertion. |  |
| [ ] | `pa22/tests/general/400-exact-overload-beats-user-defined-conversion.t` | `pa22:400` | `manual-review` |  |  | `` |  | Classify by source/ref review; no template concept was detected. |  |
| [ ] | `pa22/tests/general/400-static-cast-explicit-constructor.t` | `pa22:400` | `manual-review` |  |  | `` |  | Classify by source/ref review; no template concept was detected. |  |
| [ ] | `pa22/tests/general/100-bad-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-bad-function-template-deduction-cv-mismatch-call.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-decltype-function-template-deduced-call.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-defaulted-class-template-arg-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-defaulted-nested-class-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-derived-base-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-explicit-function-template-id-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-explicit-template-id-same-signature-free-functions.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-explicit-template-id-user-conversion-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-forwarding-reference-lvalue-overload.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-forwarding-reference-qualified-enumerator.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-reference-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-alias-parameter-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-array-to-pointer-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-const-ref-top-cv-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-defaulted-class-template-arg-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-function-template-elaborated-top-cv-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-local-class-declval-explicit-template-id.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-overload-set-unique-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-pointer-qualification-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/100-template-array-reference-cv-default-arg.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-ambiguous-cv-pointer-partial-ordering-bad.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-function-pointer-vs-const-ref-partial-order.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-function-template-partial-order-class-template-cv.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-function-template-partial-order-const-pointer.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-partial-ordering-pointer-vs-value.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/200-partial-ordering-ref-vs-const-ref.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-class-member-forward-template-alias-no-eager-complete.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-class-member-forward-template-pointer-no-eager-complete.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-constructor-template-collection.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-constructor-template-default-arg-target-aware.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-constructor-template-dependent-alias-target-aware.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-decltype-conditional-no-body-instantiation.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-dependent-class-function-body-skip.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-nontemplate-copy-move-beat-converting-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-parameter-type-no-eager-member-body.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-reentrant-class-template-copy-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-reentrant-pair-template-copy-ctor-template.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-static-cast-rvalue-ref-skips-conversion-operator.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/general/300-worse-conversion-candidate-body-not-instantiated.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-defaulted-nested-class-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-explicit-template-args-plus-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-function-template-array-reference-return-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-function-template-array-to-pointer-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/200-function-template-partial-order-const-pointer.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-constructor-template-cross-specialization.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-conversion-function-template-call-argument.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-conversion-function-template-copy-init.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-conversion-function-template-selection.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-using-class-template-does-not-instantiate-array-member.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
