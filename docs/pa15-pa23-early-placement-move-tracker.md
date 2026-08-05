# Early Placement Move Tracker

This tracker audits the current source/ref-backed `--fail-on-early` findings
from `scripts/audit_pa_feature_placement.py`. Filename-only matches are not
included as failures; they remain path hints in the audit JSON/CSV.

Seed command:

```sh
python3 scripts/audit_pa_feature_placement.py --markdown-out /tmp/pa-feature-placement-audit.md --json-out /tmp/pa-feature-placement-audit.json --csv-out /tmp/pa-feature-placement-audit.csv
```

Summary from the current run:

- early findings: 77
- tests requiring action: 74
- dominant causes: member templates before PA22, NTTPs before PA20,
  template friends before PA22, pointer/reference NTTPs before PA23, and two
  PA18 polymorphic pointer-adjust tests

Resolution:

- moved 72 source tests with their tracked sidecars to the latest feature owner
- rewrote the 2 PA18 polymorphic pointer-adjust tests into PA30 source-driven
  run tests
- verified `scripts/audit_pa_feature_placement.py --fail-on-early` reports no
  early placement failures after the moves

Status legend: `[ ]` not moved, `[~]` in progress, `[x]` moved/reduced and
validated, `[-]` deferred.

| Status | Test | Signal | Audit Decision | Target | Notes |
| --- | --- | --- | --- | --- | --- |
| [x] | `pa18/tests/general/300-multiple-inheritance-vtable-layout.t` | polymorphic.pointer_adjust->pa30:100 | move/rewrite | `pa30:100` | Polymorphic multi-base/non-primary pointer adjustment; PA18 should remain single-inheritance virtual dispatch. Recast to PA30 harness if needed. |
| [x] | `pa18/tests/general/300-secondary-base-virtual-class-return.t` | polymorphic.pointer_adjust->pa30:100 | move/rewrite | `pa30:100` | Polymorphic multi-base/non-primary pointer adjustment; PA18 should remain single-inheritance virtual dispatch. Recast to PA30 harness if needed. |
| [x] | `pa19/tests/general/100-anonymous-union-storage-constructor-noop.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-basic-template-operator-overloads.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture to keep source/ref/witness files coherent; a smaller PA19-only operator-overload reducer can be added separately if needed. |
| [x] | `pa19/tests/general/100-cast-nontype-template-argument.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/100-class-template-hidden-friend-body-lexical-scope.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/100-enum-nontype-template-vtable-mangling.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/100-explicit-member-call-function-template-id.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-explicit-member-function-template-call.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-friend-existing-template-private-ctor-access.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/100-inherited-class-template-id-member-pointer.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-nontype-expression-template-argument.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/100-nontype-qualified-enum-sum.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/100-nontype-template-redecl-typedef-spelling.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/100-parenthesized-qualified-template-functional-call.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-reference-prvalue-template-member-temp-cleanup.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/100-template-hidden-friend-adl.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/100-unused-class-template-hidden-friend-body.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/100-using-base-same-signature-derived-template-preferred.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/200-duplicate-template-instantiation-signature.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/200-nontype-template-parameter-default.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/200-templated-constructor-special-member-collection.t` | template.member_template->pa22:300, template.nttp->pa20:100 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-abstract-array-parameter-sfinae.t` | template.member_template->pa22:300 | move + detector-gap | `pa23:300` | Member-template overload probe is expression/SFINAE candidate dropping; teach detector to catch this shape. |
| [x] | `pa19/tests/general/300-defaulted-nontype-class-alias-rewrite.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-defaulted-nontype-expression-syntax-rewrite.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-defaulted-sfinae-ctor-candidate-drop.t` | template.member_template->pa22:300 | move + detector-gap | `pa23:300` | Constructor/member-template SFINAE candidate drop; detector caught member template but missed SFINAE shape. |
| [x] | `pa19/tests/general/300-dependent-default-nontype-alias-instantiation.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-dependent-default-nontype-field-instantiation.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-dependent-friend-alias-private-constructor-access.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/300-dependent-friend-self-private-constructor-access.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/300-dependent-hidden-friend-static-member-definition.t` | template.friend->pa22:300 | move | `pa22:300` | Template friend/hidden-friend declaration graph behavior was reclassified out of PA19. |
| [x] | `pa19/tests/general/300-dependent-nontype-parameter-type-default.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-dependent-nontype-template-arg-mangle.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-dependent-nontype-template-parameter-type.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-dependent-nontype-vtable-redecl-owned-syntax.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-explicit-type-arg-decltype-member-access.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-namespace-function-template-hides-outer-callable-object.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-nested-class-template-reference-reset.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-nested-dependent-template-type-name.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-nested-member-partial-specialization-apply-scope.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-nested-member-partial-specialization-survives-reference-reset.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-nontype-conversion-operator-dependent-template-id.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/general/300-reference-member-class-template-visible.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/general/300-reference-shell-nested-class-template-reuse.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/100-compound-assignment-operator-template-distractor.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/100-const-member-function-template-overload.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/100-function-template-nontype-function-pointer-call.t` | template.nttp->pa20:100, template.nttp.pointer_member->pa23:400 | move | `pa23:400` | Pointer/reference/function NTTP value; later than PA20 integral NTTP support. |
| [x] | `pa19/tests/spec/100-member-operator-template-in-class-template.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/200-qualified-function-template-call.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/spec/200-unnamed-nontype-default-template-id-return.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/spec/300-dependent-logical-or-template-default.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/spec/300-dependent-nontype-template-parameter-type.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/spec/300-explicit-template-argument-overload-rejects-short-candidate.t` | template.member_template->pa22:300 | move + detector-gap | `pa23:100` | Explicit template-argument viability/full deduction, not just member-template ownership. |
| [x] | `pa19/tests/spec/300-inherited-static-member-value.t` | template.nttp->pa20:100 | move | `pa20:100` | Integral/enum/dependent NTTP support belongs to PA20. |
| [x] | `pa19/tests/spec/300-member-call-template-hides-inherited-instantiation.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/300-member-operator-template-active-owner.t` | template.member_template->pa22:300 | move | `pa22:300` | Member template or member class template is essential; PA22 owns this surface. |
| [x] | `pa19/tests/spec/300-nontype-function-pointer-argument.t` | template.nttp->pa20:100, template.nttp.pointer_member->pa23:400 | move | `pa23:400` | Pointer/reference/function NTTP value; later than PA20 integral NTTP support. |
| [x] | `pa20/tests/general/100-explicit-function-template-type-arg-drops-nontype-overload.t` | template.member_template->pa22:300 | move + detector-gap | `pa23:100` | Explicit template-argument syntax filters overloads by parameter kind; PA23 full deduction/explicit-argument owner. |
| [x] | `pa20/tests/general/100-out-of-class-member-partial-specialization.t` | template.member_template->pa22:300 | move | `pa22:300` | Member-template behavior is essential or not yet reduced; PA22 owns this entity/collection surface. |
| [x] | `pa20/tests/general/200-array-functional-cast-pack-call.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture to keep source/ref/witness files coherent; a smaller PA20 pack-only reducer can be added separately if needed. |
| [x] | `pa20/tests/general/200-reference-nontype-template-parameter-pack.t` | template.nttp.pointer_member->pa23:400 | move | `pa23:400` | Pointer/reference/function NTTP value; later than PA20 integral NTTP support. |
| [x] | `pa20/tests/general/200-variable-template-forwarding-partial-top-const.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture because the current fixture still exercises member-template callable machinery. |
| [x] | `pa20/tests/general/200-variable-template-forwarding-partial-top-cv.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture because the current fixture still exercises member-template callable machinery. |
| [x] | `pa20/tests/general/200-variable-template-run-specialization-selection.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture because the current fixture still exercises member-template callable machinery. |
| [x] | `pa20/tests/general/200-variable-template-static-assert-specialization-selection.t` | template.member_template->pa22:300 | move whole fixture | `pa22:300` | Moved whole fixture because the current fixture still exercises member-template callable machinery. |
| [x] | `pa20/tests/general/300-member-class-explicit-specialization-owner-lookup.t` | template.member_template->pa22:300 | move | `pa22:300` | Member-template behavior is essential or not yet reduced; PA22 owns this entity/collection surface. |
| [x] | `pa20/tests/spec/100-expression-sfinae-decltype-conversion.t` | template.member_template->pa22:300 | move | `pa23:300` | Expression SFINAE through dependent decltype; PA23 substitution/failure owner. |
| [x] | `pa20/tests/spec/100-function-template-nontype-function-pointer-specialization-call.t` | template.nttp.pointer_member->pa23:400 | move | `pa23:400` | Pointer/reference/function NTTP value; later than PA20 integral NTTP support. |
| [x] | `pa20/tests/spec/100-using-inherited-alias-operator-template.t` | template.member_template->pa22:300 | move | `pa22:300` | Member-template behavior is essential or not yet reduced; PA22 owns this entity/collection surface. |
| [x] | `pa21/tests/general/300-constexpr-static-fn-template-address-pack.t` | template.member_template->pa22:300 | move | `pa22:300` | Combines PA21 constexpr/local-static surface with member-template machinery; latest owner is PA22 member templates. |
| [x] | `pa21/tests/general/300-static-constexpr-function-template-pointer-array.t` | template.member_template->pa22:300 | move | `pa22:300` | Combines PA21 constexpr/local-static surface with member-template machinery; latest owner is PA22 member templates. |
| [x] | `pa21/tests/general/400-function-template-local-static-per-specialization.t` | template.member_template->pa22:300 | move | `pa22:300` | Combines PA21 constexpr/local-static surface with member-template machinery; latest owner is PA22 member templates. |
| [x] | `pa22/tests/general/100-function-signature-partial-specialization-functor-assignment.t` | template.member_template->pa22:300 | renumber | `pa22:300` | Same PA, but member-template owner cluster is later than current cluster. |
| [x] | `pa22/tests/general/100-nested-class-template-current-owner-lookup.t` | template.member_template->pa22:300 | renumber | `pa22:300` | Same PA, but member-template owner cluster is later than current cluster. |
