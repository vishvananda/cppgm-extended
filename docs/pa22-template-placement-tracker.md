# PA22 Template Placement Tracker

This tracker is the review queue for template test placement across PA22.
It supports the split into:

- PA18/PA19/PA21 basic template owners
- PA22 advanced single-feature template completion
- PA23 template integration
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
- basic-owner-candidate: 48
- manual-review: 2
- pa22-advanced-single-candidate: 1
- PA23 integration candidates: 0 open after the 2026-06-19 placement cleanup

## 2026-06-19 PA23 Integration Cleanup

The refreshed no-course template-placement scan found 17 live PA22 tests whose
essential behavior combined multiple template concepts and belonged in PA23.
They were moved to PA23 clusters 100, 200, 300, 400, or 500 according to the
PA23 README cluster definitions. A fresh PA22 template-placement scan now has no
`pa23-integration-candidate` rows.

Moved from PA22 to PA23:

- `pa22/tests/general/100-explicit-function-template-type-arg-drops-nontype-overload.t` -> `pa23/tests/general/400-explicit-function-template-type-arg-drops-nontype-overload.t`
- `pa22/tests/general/300-static-cast-rvalue-ref-skips-conversion-operator.t` -> `pa23/tests/general/400-static-cast-rvalue-ref-skips-conversion-operator.t`
- `pa22/tests/general/300-partial-enable-if-inherited-bool-value.t` -> `pa23/tests/general/300-partial-enable-if-inherited-bool-value.t`
- `pa22/tests/general/400-forwarding-pack-cast-trailing-return.t` -> `pa23/tests/general/200-forwarding-pack-cast-trailing-return.t`
- `pa22/tests/general/400-forwarding-pack-pointer-cast-trailing-return.t` -> `pa23/tests/general/200-forwarding-pack-pointer-cast-trailing-return.t`
- `pa22/tests/general/400-reference-nontype-template-parameter-pack.t` -> `pa23/tests/general/400-reference-nontype-template-parameter-pack.t`
- `pa22/tests/general/500-dependent-alias-template-id-syntax-clone.t` -> `pa23/tests/general/500-dependent-alias-template-id-syntax-clone.t`
- `pa22/tests/general/500-dependent-std-or-enable-if-defers.t` -> `pa23/tests/general/500-dependent-std-or-enable-if-defers.t`
- `pa22/tests/general/500-reentrant-static-query-enable-if-partial.t` -> `pa23/tests/general/500-reentrant-static-query-enable-if-partial.t`
- `pa22/tests/general/500-template-template-alias-pack-scope-sfinae.t` -> `pa23/tests/general/500-template-template-alias-pack-scope-sfinae.t`
- `pa22/tests/spec/300-conversion-function-template-call-argument.t` -> `pa23/tests/spec/400-conversion-function-template-call-argument.t`
- `pa22/tests/spec/300-conversion-function-template-copy-init.t` -> `pa23/tests/spec/400-conversion-function-template-copy-init.t`
- `pa22/tests/spec/300-conversion-function-template-selection.t` -> `pa23/tests/spec/400-conversion-function-template-selection.t`
- `pa22/tests/spec/300-trailing-return-expression-sfinae-default-param.t` -> `pa23/tests/spec/300-trailing-return-expression-sfinae-default-param.t`
- `pa22/tests/spec/400-function-template-nontype-function-pointer-call.t` -> `pa23/tests/spec/100-function-template-nontype-function-pointer-call.t`
- `pa22/tests/spec/400-function-template-nontype-function-pointer-specialization-call.t` -> `pa23/tests/spec/100-function-template-nontype-function-pointer-specialization-call.t`
- `pa22/tests/spec/400-nontype-function-pointer-argument.t` -> `pa23/tests/spec/100-nontype-function-pointer-argument.t`

## Review Queue

| Status | Test | Current | Bucket | Concepts For Review | Later/Compat Features | Latest Template Owner | PA23 Cluster | Action | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [D] | `pa22/tests/general/200-pack-expanded-base-instantiation.t` | `pa22:200` | `later-owner-or-split` | pack-expansion | template.alignas_alignof | `pa19:200` |  | Move later-owned behavior, or split/reduce to keep only the PA22 template assertion. | Source test was removed by the PA placement cleanup; no live PA22 source test remains for this row. |
| [x] | `pa22/tests/general/500-dependent-template-id-no-eager-layout.t` | `pa22:500` | `later-owner-or-split` | alias-template, no-eager-instantiation |  | `pa22:300` |  | Move later-owned behavior, or split/reduce to keep only the PA22 template assertion. | Reduced in place by removing incidental `alignas`/`alignof`; the test now asserts the PA22 alias-template/no-eager behavior only. |
| [D] | `pa22/tests/general/400-exact-overload-beats-user-defined-conversion.t` | `pa22:400` | `manual-review` |  |  | `` |  | Classify by source/ref review; no template concept was detected. | Source test was removed by the PA placement cleanup; no live PA22 source test remains for this row. |
| [D] | `pa22/tests/general/400-static-cast-explicit-constructor.t` | `pa22:400` | `manual-review` |  |  | `` |  | Classify by source/ref review; no template concept was detected. | Source test was removed by the PA placement cleanup; no live PA22 source test remains for this row. |
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
| [x] | `pa22/tests/general/300-static-cast-rvalue-ref-skips-conversion-operator.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. | Moved to `pa23/tests/general/400-static-cast-rvalue-ref-skips-conversion-operator.t` during the 2026-06-19 PA23 integration cleanup. |
| [ ] | `pa22/tests/general/300-worse-conversion-candidate-body-not-instantiated.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-defaulted-nested-class-template-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-explicit-template-args-plus-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-function-template-array-reference-return-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/100-function-template-array-to-pointer-deduction.t` | `pa22:100` | `pa22-advanced-single-candidate` | function-deduction |  | `pa22:100` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/200-function-template-partial-order-const-pointer.t` | `pa22:200` | `pa22-advanced-single-candidate` | function-partial-ordering |  | `pa22:200` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [ ] | `pa22/tests/spec/300-constructor-template-cross-specialization.t` | `pa22:300` | `pa22-advanced-single-candidate` | constructor-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
| [x] | `pa22/tests/spec/300-conversion-function-template-call-argument.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. | Moved to `pa23/tests/spec/400-conversion-function-template-call-argument.t` during the 2026-06-19 PA23 integration cleanup. |
| [x] | `pa22/tests/spec/300-conversion-function-template-copy-init.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. | Moved to `pa23/tests/spec/400-conversion-function-template-copy-init.t` during the 2026-06-19 PA23 integration cleanup. |
| [x] | `pa22/tests/spec/300-conversion-function-template-selection.t` | `pa22:300` | `pa22-advanced-single-candidate` | conversion-deduction |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. | Moved to `pa23/tests/spec/400-conversion-function-template-selection.t` during the 2026-06-19 PA23 integration cleanup. |
| [ ] | `pa22/tests/spec/300-using-class-template-does-not-instantiate-array-member.t` | `pa22:300` | `pa22-advanced-single-candidate` | no-eager-instantiation |  | `pa22:300` |  | Place in PA22 and renumber if the current cluster is earlier than the owner cluster. |  |
