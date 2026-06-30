# PA23 Original Test Disposition

## Purpose

The PA19-PA22 backfill reducers prove that many PA23 failures were caused by
earlier missing coverage. This pass asks a separate question: after adding those
earlier reducers, does the original PA23 test still add useful PA23 integration
coverage, or is it only a duplicate that was accidentally in the PA23 bucket?

PA23 owns composition among earlier template features. A source-identical
promoted reducer is a review signal, not an automatic deletion rule: PA23 may
still keep byte-identical or near-identical source when manual inspection shows
that the source is a realistic combined-feature exercise and the earlier PA is
using it as focused backfill coverage. Delete the PA23 copy only when the source
is just an isolated single-feature check with no meaningful PA23 integration
value.

## Repeatable Report

Run:

```sh
scripts/report_pa23_original_disposition.py --summary
scripts/report_pa23_original_disposition.py
```

The script reads `pa23_feature_backfill_tracker.tsv`, filters to
`action_status=test-added`, compares each PA23 source with the promoted reducer,
applies any reviewed decisions from
`analysis/pa23-original-disposition-overrides.tsv`, and classifies the PA23
original:

- `retire-pa23-duplicate`: byte-identical or identical after whitespace/comment
  normalization.
- `pa23-original-removed`: the PA23 original was retired and the promoted
  earlier-owner reducer still exists.
- `keep-pa23-integration`: a manually reviewed PA23 original remains because it
  is materially different from the promoted reducer and the placement audit
  reports multi-concept PA23 integration.
- `review-retire-or-simplify`: at least 0.98 normalized source similarity.
- `review-near-duplicate`: at least 0.90 normalized source similarity.
- `keep-pa23-integration-candidate`: materially different source shape.

This is a review aid, not a semantic proof. Deleting a PA23 test still requires
checking the test itself and its references.

## Current Result

Current summary:

```text
pa23-original-removed        127
keep-pa23-integration        50
retire-pa23-duplicate        0
review-retire-or-simplify    0
review-near-duplicate        0
keep-pa23-integration-candidate 0
```

Interpretation:

- The retired PA23 originals were manually treated as isolated single-feature
  checks or non-integration duplicates. Their promoted earlier-owner reducers
  remain in PA19, PA21, or PA22.
- The 50 kept rows were manually reviewed and kept as PA23 integration tests,
  including several byte-identical or near-identical sources whose library-like
  shape combines multiple earlier features. Their explicit keep decisions live
  in `analysis/pa23-original-disposition-overrides.tsv`.

## Retired Exact Duplicates

These PA23 originals were duplicated by a promoted earlier-owner reducer and
have been removed from PA23:

The list below records the exact or normalized duplicate subset. The full
current retired set, including manually reviewed near-duplicates, is available
from:

```sh
scripts/report_pa23_original_disposition.py | awk -F '\t' '$1=="pa23-original-removed" {print}'
```

```text
pa23/tests/general/100-class-partial-specialization-selection.t -> pa21/tests/spec/100-partial-specialization-cv-pointer-selection.t
pa23/tests/general/100-forward-primary-partial-switch-value.t -> pa21/tests/general/400-forward-primary-partial-switch-value.t
pa23/tests/general/100-partial-specialization-concrete-namespace-argument-order.t -> pa21/tests/general/400-partial-specialization-concrete-namespace-argument-order.t
pa23/tests/general/100-reference-member-lookup-in-progress-base-typedef.t -> pa21/tests/general/400-reference-member-lookup-in-progress-base-typedef.t
pa23/tests/general/100-template-friend-class-constructor-access.t -> pa21/tests/general/300-template-friend-class-constructor-access.t
pa23/tests/general/100-template-specialization-qualified-constructor-declaration.t -> pa21/tests/general/300-template-specialization-qualified-constructor-declaration.t
pa23/tests/general/100-unevaluated-sizeof-call-surrogates.t -> pa22/tests/general/300-unevaluated-sizeof-call-surrogates.t
pa23/tests/general/200-empty-pack-static-assert-trait-expansion.t -> pa22/tests/general/100-empty-pack-static-assert-trait-expansion.t
pa23/tests/general/200-function-pointer-varargs-partial-specialization-order.t -> pa21/tests/general/400-function-pointer-varargs-partial-specialization-order.t
pa23/tests/general/200-function-template-trailing-pack-partial-order.t -> pa22/tests/general/200-function-template-trailing-pack-partial-order.t
pa23/tests/general/200-nondeduced-context-only-bad.t -> pa22/tests/spec/200-nondeduced-context-only-bad.t
pa23/tests/general/200-overload-set-address-nondeduced-bad.t -> pa22/tests/spec/200-overload-set-address-nondeduced-bad.t
pa23/tests/general/200-repeated-argument-partial-specialization-ordering.t -> pa21/tests/general/400-repeated-argument-partial-specialization-ordering.t
pa23/tests/general/300-dependent-detected-or-instantiation-scope.t -> pa22/tests/general/300-dependent-detected-or-instantiation-scope.t
pa23/tests/general/300-dependent-enable-if-nontype-candidate-drop.t -> pa22/tests/general/300-dependent-enable-if-nontype-candidate-drop.t
pa23/tests/general/300-detected-or-alias-template-argument.t -> pa22/tests/general/300-detected-or-alias-template-argument.t
pa23/tests/general/300-function-template-nested-alias-explicit-call.t -> pa22/tests/general/300-function-template-nested-alias-explicit-call.t
pa23/tests/general/300-local-alias-explicit-template-pack-decltype.t -> pa22/tests/general/300-local-alias-explicit-template-pack-decltype.t
pa23/tests/general/400-alias-pack-expansion-through-alias.t -> pa21/tests/general/400-alias-pack-expansion-through-alias.t
pa23/tests/general/400-alias-template-decltype-greater-type-argument.t -> pa21/tests/general/400-alias-template-decltype-greater-type-argument.t
pa23/tests/general/400-alias-template-decltype-member-type-argument.t -> pa21/tests/general/400-alias-template-decltype-member-type-argument.t
pa23/tests/general/400-alias-template-decltype-shift-type-argument.t -> pa21/tests/general/400-alias-template-decltype-shift-type-argument.t
pa23/tests/general/400-alias-template-pack-id-preserves-syntax.t -> pa21/tests/general/400-alias-template-pack-id-preserves-syntax.t
pa23/tests/general/400-alias-template-pointer-cv-cache-distinction.t -> pa21/tests/general/400-alias-template-pointer-cv-cache-distinction.t
pa23/tests/general/400-alias-value-expression-type-argument.t -> pa21/tests/general/400-alias-value-expression-type-argument.t
pa23/tests/general/400-bad-nontype-template-argument-type-pack.t -> pa19/tests/general/200-bad-nontype-template-argument-type-pack.t
pa23/tests/general/400-bool-or-dependent-member-type-conditional-base.t -> pa21/tests/general/400-bool-or-dependent-member-type-conditional-base.t
pa23/tests/general/400-dependent-alias-default-recomputed-after-substitution.t -> pa22/tests/general/400-dependent-alias-default-recomputed-after-substitution.t
pa23/tests/general/400-dependent-conditional-default-alias-recomputed.t -> pa21/tests/general/400-dependent-conditional-default-alias-recomputed.t
pa23/tests/general/400-dependent-pack-typename-nontype-expression.t -> pa21/tests/general/400-dependent-pack-typename-nontype-expression.t
pa23/tests/general/400-function-parameter-pack-alias-expansion.t -> pa21/tests/general/400-function-parameter-pack-alias-expansion.t
pa23/tests/general/400-instantiated-function-fixed-prefix-pack-tail.t -> pa19/tests/general/200-instantiated-function-fixed-prefix-pack-tail.t
pa23/tests/general/400-nontype-pack-comma-expression-syntax.t -> pa21/tests/general/400-nontype-pack-comma-expression-syntax.t
pa23/tests/general/400-nttp-pack-void-comma-expression.t -> pa21/tests/general/400-nttp-pack-void-comma-expression.t
pa23/tests/general/400-qualified-base-type-alias-from-nontype-pack.t -> pa21/tests/general/400-qualified-base-type-alias-from-nontype-pack.t
pa23/tests/general/400-reference-member-depth-pack-sum.t -> pa21/tests/general/400-reference-member-depth-pack-sum.t
pa23/tests/general/400-single-pack-cast-target.t -> pa21/tests/general/300-single-pack-cast-target.t
pa23/tests/general/400-sizeof-nontype-pack-recursive-template.t -> pa22/tests/general/300-sizeof-nontype-pack-recursive-template.t
pa23/tests/general/500-alias-pack-enable-if-constexpr-constructor.t -> pa22/tests/general/500-alias-pack-enable-if-constexpr-constructor.t
pa23/tests/general/500-base-qualified-template-value-arg-syntax.t -> pa22/tests/general/300-base-qualified-template-value-arg-syntax.t
pa23/tests/general/500-dependent-alias-member-template-id-defer.t -> pa21/tests/general/400-dependent-alias-member-template-id-defer.t
pa23/tests/general/500-dependent-member-alias-function-return.t -> pa22/tests/general/500-dependent-member-alias-function-return.t
pa23/tests/general/500-member-template-dependent-owner-defaulted-sfinae.t -> pa22/tests/general/500-member-template-dependent-owner-defaulted-sfinae.t
pa23/tests/general/500-pack-alias-functional-bool-trait-sfinae.t -> pa22/tests/general/500-pack-alias-functional-bool-trait-sfinae.t
pa23/tests/general/500-range-array-reference-mutable-begin.t -> pa22/tests/general/200-range-array-reference-mutable-begin.t
pa23/tests/general/500-short-circuit-alias-member-sfinae.t -> pa22/tests/general/500-short-circuit-alias-member-sfinae.t
pa23/tests/general/500-template-deduction-rejects-value-base-argument.t -> pa22/tests/general/100-template-deduction-rejects-value-base-argument.t
pa23/tests/spec/100-constructor-template-const-ref-conversion.t -> pa22/tests/spec/300-constructor-template-const-ref-conversion.t
pa23/tests/spec/100-extern-template-constructor-declaration.t -> pa21/tests/spec/300-extern-template-constructor-declaration.t
pa23/tests/spec/100-nontype-reference-argument.t -> pa22/tests/spec/400-nontype-reference-argument.t
pa23/tests/spec/100-nontype-static-outdef-value-member-preserves-type.t -> pa21/tests/spec/100-nontype-static-outdef-value-member-preserves-type.t
pa23/tests/spec/200-dependent-specialized-default-arg-deduction.t -> pa22/tests/spec/200-dependent-specialized-default-arg-deduction.t
pa23/tests/spec/200-function-template-array-bound-deduction.t -> pa22/tests/spec/100-function-template-array-bound-deduction.t
pa23/tests/spec/200-member-template-explicit-pack-forward-call.t -> pa22/tests/spec/200-member-template-explicit-pack-forward-call.t
pa23/tests/spec/300-typedef-class-template-does-not-instantiate.t -> pa22/tests/spec/300-typedef-class-template-does-not-instantiate.t
pa23/tests/spec/300-void-t-detector.t -> pa22/tests/spec/300-void-t-detector.t
pa23/tests/spec/500-array-reference-cv-partial-ordering.t -> pa22/tests/spec/200-array-reference-cv-partial-ordering.t
pa23/tests/spec/500-member-alias-pack-owner-sfinae.t -> pa22/tests/spec/500-member-alias-pack-owner-sfinae.t
```

## Cleanup Workflow

1. Process `retire-pa23-duplicate` rows first, preferably one cluster at a time.
   The current target is zero active rows in this category.
2. For each row, manually inspect the PA23 source. Either delete the PA23 `.t`
   and its tracked `.ref*` siblings because it is an isolated single-feature
   check, keep it with an override because it is useful PA23 integration
   coverage, or rewrite it into a genuinely combined PA23 case.
3. Run:

   ```sh
   make test-report ACTIVE_TEST_REPORT_PAS='pa19 pa20 pa21 pa22 pa23'
   ```

4. Rerun `scripts/report_pa23_original_disposition.py --summary`. The first
   target is zero `retire-pa23-duplicate` rows; retired rows should appear as
   `pa23-original-removed`.
5. Then inspect `review-retire-or-simplify`,
   `review-near-duplicate`, and `keep-pa23-integration-candidate` rows. The
   current target is zero rows in all three categories. Keep rows where a manual
   read identifies a real PA23 composition point, and record that decision in
   `analysis/pa23-original-disposition-overrides.tsv`.
