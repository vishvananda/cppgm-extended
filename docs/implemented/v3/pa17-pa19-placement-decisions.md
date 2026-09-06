# PA17-PA19 Placement Decisions

First-pass report from the refreshed scanner outputs:

```sh
python3 scripts/audit_pa_feature_placement.py --pa pa17 --pa pa18 --pa pa19 --markdown-out /tmp/pa17-pa19-feature-audit.md --json-out /tmp/pa17-pa19-feature-audit.json --csv-out /tmp/pa17-pa19-feature-audit.csv
```

Coverage: 290 refreshed `needs_review` scanner tests, plus the tracker-known
post-PA9 course-placement case
`cppgm.tests/course/pa17/412-temporary-derived-to-base-reference-virtual-call.t`.
No tests or refs were moved, edited, or regenerated.

## Decision Table

| Test | Current | Decision | Destination | Primary Feature | Essential Later Features | Action | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G1 | `pa17:300` / `pa17:300-spec` | `keep` | current | PA17 single-inheritance virtual dispatch, `override`/`final`, or virtual destructor cases | none | Leave in place. | Scanner `_ZTV` / `virtual` hits are false positives for `polymorphic.vtable_order`; these tests assert ordinary PA17 virtual behavior, not vtable slot ordering. |
| G2 | `cppgm.tests/course/pa17` | `move` | `pa17/tests/general/300-...` | PA17 virtual call through derived-to-base temporary/reference value path | none | Move later, with refs, during the actual placement pass. | Not scanner-flagged, but the tracker says PA10+ course tests should move into local PA ownership. |
| G3 | `pa17:400` / `cppgm.tests/course/pa17` | `move` | `pa27/tests/general/100-...` | `polymorphic.pointer_adjust` | PA26 multiple inheritance on some cases | Move later. | Source/ref inspection shows non-primary base-subobject offsets, downcasts, secondary vptrs, or multi-vptr polymorphic layout. |
| G4 | `pa17:400-spec` | `renumber` | `pa17/tests/spec/200-...` | `polymorphic.override_final` | none | Renumber later. | The `double` parameter is incidental fixture syntax; the checked behavior is `override` signature mismatch. |
| G5 | `pa18:mixed` | `keep` | current | Basic PA18 class/function template instantiation and LowIR lowering | none | Leave in place. | Scanner hits are false positives or already-owned fixture syntax such as plain `typename`, plain `using`, semantic-owner `decltype`, or template-id spelling. |
| G6 | `pa18:mixed` | `renumber` | `pa18/tests/*/200-...` | `template.dependent_name`, `template.current_instantiation`, or dependent disambiguation | none | Renumber later. | Current PA is right, but the first correct cluster is PA18 cluster 200. |
| G7 | `pa18:mixed` | `renumber` | `pa18/tests/*/300-...` | PA18 packs, friend templates, member templates, or template-template parameters | none | Renumber later. | Current PA is right, but the first correct cluster is PA18 cluster 300. C-style `...` / template-comma scanner hits are false positives unless noted elsewhere. |
| G8 | `pa18:mixed` | `move` | `pa19/tests/*/100-...`, `pa19/tests/*/200-...`, or `pa19/tests/*/300-...` | PA19 integral NTTPs, `static_assert`, explicit specialization, or specialization timing | PA18 template support | Move later to the listed PA19 owner cluster. | Latest essential owner is PA19. Use cluster 100 for NTTP/static assert, 200 for explicit specialization, and 300 for timing/stale-primary cases. |
| G9 | `pa18:mixed` | `move` | `pa20/tests/*/100-...` or `pa20/tests/*/300-...` | Full `constexpr` function/object behavior or function-local static template specialization state | PA18 templates | Move later. | Local-static refs include `__local_static__`; constexpr linkage cases rely on PA20 constexpr function/object ownership. |
| G10 | `pa18:mixed` | `move` | `pa21/tests/*/100-...` | Class partial specialization / specialization graph behavior | PA18 packs/member templates | Move later. | Latest essential owner is PA21. |
| G11 | `pa18:mixed` | `move` | `pa22/tests/*/100-...`, `pa22/tests/*/200-...`, or `pa22/tests/*/300-...` | Full deduction, function-template partial ordering, SFINAE/substitution, or no-eager instantiation | PA18/PA19 helper templates | Move later. | Use PA22 cluster 100 for full deduction, 200 for partial ordering/substitution, and 300 for SFINAE/no-eager timing. |
| G12 | `pa18:mixed` | `move` | PA24 / PA25 / PA26 / PA27 / PA34 owner clusters | Lambda/initializer-list/member-pointer/pointer-adjust/builtin-trait support | PA18 templates as fixture | Move later. | Later support feature is essential; specific destinations are in the path list notes. |
| G13 | `pa18:100-spec` | `manual-review` | TBD | dependent sizeof/type-array member fixture | possible PA18 dependent-name vs later substitution split | Inspect before moving. | Scanner finds several later-looking features, but the primary assertion may be reducible to a PA18 dependent-name case. |
| G14 | `pa19:mixed` | `keep` | current | PA19 integral NTTP/static-assert/integral-constant behavior | none | Leave in place. | Scanner `current_specialization`, `constexpr.full`, `function.noexcept`, and similar hits are false positives or fixture syntax for the PA19 integral subset. |
| G15 | `pa19:100` | `renumber` | `pa19/tests/*/200-...` | `template.explicit_specialization` | none | Renumber later. | Current PA is right; first correct cluster is PA19 cluster 200. |
| G16 | `pa19:100/200` | `renumber` | `pa19/tests/*/300-...` | `template.specialization_timing` | explicit specialization | Renumber later. | Current PA is right; first correct cluster is PA19 cluster 300. |
| G17 | `pa19:mixed` | `move` | `pa20/tests/*/100-...` or `pa20/tests/*/300-...` | Full constexpr function/object behavior | PA19 NTTP/static data fixture | Move later. | These require PA20 beyond the PA19 integral-constant subset. |
| G18 | `pa19:mixed` | `move` | `pa21/tests/*/100-...`, `pa21/tests/*/200-...`, or `pa21/tests/*/400-...` | alias templates, variable templates, class partial specialization, or specialization graph behavior | PA19 NTTP/static-assert fixture | Move later. | Latest essential owner is PA21. |
| G19 | `pa19:mixed` | `move` | `pa22/tests/*/100-...`, `pa22/tests/*/300-...`, or `pa22/tests/*/400-...` | SFINAE/substitution/full deduction/no-eager or non-integral NTTP completion | PA19 integral helpers | Move later. | Function-pointer/reference NTTPs use the PA22 non-integral NTTP owner unless member pointers push the case to PA25. |
| G20 | `pa19:mixed` | `move` | PA25 or PA34 owner clusters | member pointers or builtin/hosted trait probes | PA19 metaprogramming fixture | Move later. | Member-pointer cases belong to PA25; `__is_*` / hosted builtin-trait probes belong to PA34. |

## Path Lists

### G1

- `pa17/tests/general/300-derived-virtual-root.t`
- `pa17/tests/general/300-self-subobject-base-path.t`
- `pa17/tests/general/300-virtual-override-most-derived-match.t`
- `pa17/tests/spec/300-bad-final-override.t`
- `pa17/tests/spec/300-final-virtual.t`
- `pa17/tests/spec/300-inherited-virtual.t`
- `pa17/tests/spec/300-pure-virtual-member.t`
- `pa17/tests/spec/300-virtual-base-pointer.t`
- `pa17/tests/spec/300-virtual-base-reference.t`
- `pa17/tests/spec/300-virtual-destructor-override.t`

### G2

- `cppgm.tests/course/pa17/412-temporary-derived-to-base-reference-virtual-call.t`

### G3

- `pa17/tests/general/400-diamond-virtual-destructor-slot-merge.t`
- `pa17/tests/general/400-multibase-implicit-virtual-destructor-slot-merge.t`
- `pa17/tests/general/400-nonprimary-direct-base-ctor-vtable-offset.t`
- `pa17/tests/general/400-secondary-primary-base-vptr-overwrite.t`
- `pa17/tests/general/400-static-reference-downcast-nonprimary-base.t`
- `cppgm.tests/course/pa17/413-static-pointer-downcast-nonprimary-base.t`

### G4

- `pa17/tests/spec/400-override-signature-mismatch-bad.t`

### G5

- `pa18/tests/general/100-bad-deduction.t`
- `pa18/tests/general/100-class-template-field.t`
- `pa18/tests/general/100-class-template-method.t`
- `pa18/tests/general/100-class-template-static-member-assignment-lvalue.t`
- `pa18/tests/general/100-dependent-default-construction-through-template-subscript.t`
- `pa18/tests/general/100-function-template-array-to-pointer-deduction.t`
- `pa18/tests/general/100-function-template-deduction.t`
- `pa18/tests/general/100-function-template-explicit.t`
- `pa18/tests/general/100-lazy-header-parenthesized-qualified-function-template-call.t`
- `pa18/tests/general/100-local-class-declval-explicit-template-id.t`
- `pa18/tests/general/100-local-type-alias-noop.t`
- `pa18/tests/general/100-local-using-directive-qualified-template-argument.t`
- `pa18/tests/general/100-local-using-directive-template-member-enumerator.t`
- `pa18/tests/general/100-namespace-template-class.t`
- `pa18/tests/general/100-namespace-template-function.t`
- `pa18/tests/general/100-required-template-special-member-emission.t`
- `pa18/tests/general/100-specialization-signed-builtin-types.t`
- `pa18/tests/general/100-template-auto-trailing-return.t`
- `pa18/tests/general/100-template-overload-fallback.t`
- `pa18/tests/general/300-dependent-hidden-friend-static-member-definition.t`
- `pa18/tests/spec/100-class-template-default-arg-preserves-template-scope.t`
- `pa18/tests/spec/100-dependent-adl-point-of-instantiation.t`
- `pa18/tests/spec/100-explicit-template-args-plus-deduction.t`
- `pa18/tests/spec/100-function-template-array-reference-return-deduction.t`
- `pa18/tests/spec/100-nondependent-name-binding.t`
- `pa18/tests/spec/100-namespace-qualified-template-id-static-call.t`
- `pa18/tests/spec/100-template-static-object-member-definition.t`
- `pa18/tests/spec/100-template-vs-nontemplate-overload.t`
- `pa18/tests/spec/100-unnamed-namespace-qualified-class-template-id.t`

### G6

- `pa18/tests/general/100-decltype-qualified-type-template-argument.t`
- `pa18/tests/general/100-dependent-decltype-comma-expression.t`
- `pa18/tests/general/100-dependent-decltype-function-pointer-reference-call.t`
- `pa18/tests/general/100-dependent-qualified-return-same-name-function.t`
- `pa18/tests/general/100-local-using-alias-template-member-body.t`
- `pa18/tests/general/100-nested-dependent-template-type-name.t`
- `pa18/tests/general/100-reentrant-reference-collection-override-param.t`
- `pa18/tests/general/100-reference-member-same-template-name.t`
- `pa18/tests/general/100-reused-template-body-qualified-member-type-arg.t`
- `pa18/tests/general/100-template-argument-qualified-member-template-type.t`
- `pa18/tests/general/200-out-of-class-member-alias-return-signature.t`
- `pa18/tests/general/200-out-of-class-member-nested-trailing-return.t`
- `pa18/tests/general/200-out-of-class-nested-member-class-definition.t`
- `pa18/tests/spec/100-forward-owner-default-member-type.t`
- `pa18/tests/spec/100-out-of-class-template-member-inherited-typedef-param.t`
- `pa18/tests/spec/100-out-of-class-template-member-nested-enum-param.t`
- `pa18/tests/spec/100-using-alias-forward-template-member-layout.t`

### G7

- `pa18/tests/general/100-basic-template-operator-overloads.t`
- `pa18/tests/general/100-empty-class-pack-member-template-call.t`
- `pa18/tests/general/100-function-template-leading-fixed-and-pack-call.t`
- `pa18/tests/general/100-function-template-pack-call.t`
- `pa18/tests/general/100-function-template-template-parameter-deduction.t`
- `pa18/tests/general/100-inherited-typedef-hidden-friend-overload.t`
- `pa18/tests/general/100-inline-dependent-pack-result-type.t`
- `pa18/tests/general/100-invoke-template-declaration.t`
- `pa18/tests/general/100-member-template-call-operator.t`
- `pa18/tests/general/100-member-template-local-using-does-not-suppress-adl.t`
- `pa18/tests/general/100-namespace-qualified-class-template-definition.t`
- `pa18/tests/general/100-pack-expansion-aggregate-brace-init.t`
- `pa18/tests/general/100-template-array-reference-cv-default-arg.t`
- `pa18/tests/general/100-template-operator-shift-stress-chain.t`
- `pa18/tests/general/100-template-operator-shift-two-step.t`
- `pa18/tests/general/100-template-parameter-pack-collection.t`
- `pa18/tests/general/100-template-template-parameter.t`
- `pa18/tests/general/200-enum-operator-template-fallback-to-builtin.t`
- `pa18/tests/general/200-function-template-named-pack-call.t`
- `pa18/tests/general/200-function-template-pack-forward-call.t`
- `pa18/tests/general/200-function-template-pack-ref-return.t`
- `pa18/tests/general/200-hidden-friend-template-call-adl.t`
- `pa18/tests/general/200-local-type-cross-namespace-operator-template.t`
- `pa18/tests/general/200-member-rvalue-subscript-overload-binding.t`
- `pa18/tests/general/200-member-template-assignment-not-special-member.t`
- `pa18/tests/general/200-namespace-function-template-hides-outer-callable-object.t`
- `pa18/tests/general/200-nested-class-friend-template-namespace-scope.t`
- `pa18/tests/general/200-nested-namespace-template-static-member-owner-type.t`
- `pa18/tests/general/200-out-of-class-member-template-namespace-typedef.t`
- `pa18/tests/spec/100-compound-assignment-operator-template-distractor.t`
- `pa18/tests/spec/100-const-member-function-template-overload.t`
- `pa18/tests/spec/100-constructor-template-cross-specialization.t`
- `pa18/tests/spec/100-function-template-leading-fixed-and-pack-call.t`
- `pa18/tests/spec/100-function-template-pack-call.t`
- `pa18/tests/spec/100-function-template-pack-forward-call.t`
- `pa18/tests/spec/100-hidden-friend-template-call-adl.t`
- `pa18/tests/spec/100-hidden-friend-template-operator-adl.t`
- `pa18/tests/spec/100-member-call-template-hides-inherited-instantiation.t`
- `pa18/tests/spec/100-member-class-template-out-of-class.t`
- `pa18/tests/spec/100-member-function-template-out-of-class.t`
- `pa18/tests/spec/100-member-operator-template-in-class-template.t`
- `pa18/tests/spec/100-qualified-friend-function-template-member-access.t`
- `pa18/tests/spec/100-template-comma-operator-return-construction.t`
- `pa18/tests/spec/100-template-friend-inside-class-template.t`
- `pa18/tests/spec/100-template-template-parameter-arity-mismatch.t`
- `pa18/tests/spec/100-template-template-parameter-basic.t`
- `pa18/tests/spec/100-template-template-parameter-pack.t`
- `pa18/tests/spec/100-unnamed-template-template-parameter.t`
- `pa18/tests/spec/100-using-declaration-operator-template-adl.t`
- `pa18/tests/spec/100-using-member-operator-template-hides-inherited-instantiations.t`
- `pa18/tests/spec/200-out-of-class-member-template-tag-dispatch-definition.t`

### G8

- `pa18/tests/general/100-cross-instantiation-member-template-source-owner.t`
- `pa18/tests/general/100-empty-base-pack-expansion-sizeof.t`
- `pa18/tests/general/100-inherited-constructor-using-alias-template.t`
- `pa18/tests/general/100-nonmember-template-compound-assignment-const-lhs.t`
- `pa18/tests/general/100-pack-expansion-array-unknown-bound.t`
- `pa18/tests/general/100-unused-static-member-template-return-type.t`
- `pa18/tests/general/100-using-directive-template-member-type-typedef.t`
- `pa18/tests/general/200-constructor-template-const-ref-conversion.t`
- `pa18/tests/general/200-constructor-template-direct-other-specialization.t`
- `pa18/tests/general/200-constructor-template-same-owner-nontype-parameter.t`
- `pa18/tests/general/200-template-alignas-nontype-argument.t`
- `pa18/tests/general/200-template-instantiation-use-location-explicit-specialization.t`
- `pa18/tests/spec/100-constructor-template-const-ref-conversion.t`
- `pa18/tests/spec/100-type-equivalence-default-argument.t`
- `pa18/tests/spec/100-using-inherited-alias-operator-template.t`

### G9

- `pa18/tests/general/100-function-template-local-static-per-specialization.t`
- `pa18/tests/general/200-explicit-function-specialization-constexpr-linkage.t`
- `pa18/tests/spec/100-member-template-cache-hit-concrete-scope.t`

### G10

- `pa18/tests/general/200-function-type-pack-out-of-class-constructor-template.t`
- `pa18/tests/general/200-function-type-pack-out-of-class-constructor.t`
- `pa18/tests/spec/100-variadic-base-pack-expansion.t`

### G11

- `pa18/tests/general/100-dependent-enable-if-return-less-equal.t`
- `pa18/tests/general/100-dependent-enable-if-return-sizeof-less.t`
- `pa18/tests/general/100-empty-pack-member-template-owner-key.t`
- `pa18/tests/general/100-friend-template-adl-existing-definition.t`
- `pa18/tests/general/100-function-template-array-parameter-string-literal.t`
- `pa18/tests/general/100-function-template-pair-vs-range-predicate.t`
- `pa18/tests/general/100-qualified-template-argument-lexical-scope.t`
- `pa18/tests/general/200-forward-only-member-operator-overload.t`
- `pa18/tests/general/200-friend-template-dependent-qualified-member-access.t`
- `pa18/tests/general/200-function-template-forwarding-pack-array-ref.t`
- `pa18/tests/general/200-function-template-partial-order-const-pointer.t`
- `pa18/tests/general/200-member-template-explicit-pack-forward-call.t`
- `pa18/tests/general/200-partial-explicit-function-template-id-call.t`
- `pa18/tests/spec/100-function-template-partial-order-const-pointer.t`
- `pa18/tests/spec/100-inherited-template-param-shadow-forward.t`
- `pa18/tests/spec/100-local-using-template-specialization-does-not-suppress-adl.t`
- `pa18/tests/spec/100-member-template-explicit-pack-forward-call.t`
- `pa18/tests/spec/100-no-eager-instantiation-unused-body.t`
- `pa18/tests/spec/100-out-of-class-member-template-owner-param-rename.t`
- `pa18/tests/spec/100-out-of-class-overloaded-member-template-definition.t`
- `pa18/tests/spec/100-out-of-class-sfinae-member-template-body.t`
- `pa18/tests/spec/100-pack-fallback-partial-ordering.t`
- `pa18/tests/spec/100-typedef-class-template-does-not-instantiate.t`
- `pa18/tests/spec/100-using-declaration-imports-function-template.t`

### G12

- `pa18/tests/spec/100-local-lambda-function-template-concrete-type.t` -> PA26
- `pa18/tests/spec/100-inline-namespace-forward-initlist-declaration.t` -> PA27
- `pa18/tests/spec/100-member-pointer-parameter-variadic-deduction.t` -> PA28
- `pa18/tests/spec/100-overloaded-member-pointer-function-template-deduction.t` -> PA28
- `pa18/tests/general/400-primary-polymorphic-base-before-nonpoly-static-cast.t` -> PA29
- `pa18/tests/general/200-local-constructor-template-member-typedef.t` -> PA34
- `pa18/tests/general/200-local-member-call-constructor-template-instantiation.t` -> PA34
- `pa18/tests/general/200-static-assert-builtin-trait-non-type-argument.t` -> PA34
- `pa18/tests/general/200-template-body-builtin-constant-p.t` -> PA34
- `pa18/tests/spec/100-local-constructor-template-member-typedef.t` -> PA34
- `pa18/tests/spec/100-local-member-call-constructor-template-instantiation.t` -> PA34

### G13

- `pa18/tests/spec/100-dependent-sizeof-type-array-member.t`

### G14

- `pa19/tests/general/100-cast-nontype-template-argument.t`
- `pa19/tests/general/100-integral-constant-template-argument.t`
- `pa19/tests/general/100-dependent-nontype-functional-cast-body-check.t`
- `pa19/tests/general/100-dependent-nontype-template-type.t`
- `pa19/tests/general/100-dependent-sizeof-template-default.t`
- `pa19/tests/general/100-dependent-static-assert-defer.t`
- `pa19/tests/general/100-inherited-static-member-value.t`
- `pa19/tests/general/100-integral-constant-static-value.t`
- `pa19/tests/general/100-sizeof-call-result-nontype-template-argument.t`
- `pa19/tests/general/100-template-static-assert.t`
- `pa19/tests/general/100-type-template-member-value-comparison-argument.t`
- `pa19/tests/general/200-current-class-static-member-nontype-argument-body-check.t`
- `pa19/tests/general/200-defaulted-nontype-expression-syntax-rewrite.t`
- `pa19/tests/general/200-dependent-nontype-parameter-type-default.t`
- `pa19/tests/general/200-dependent-nontype-template-arg-mangle.t`
- `pa19/tests/general/200-dependent-qualified-nontype-base-argument.t`
- `pa19/tests/general/200-dependent-qualified-value-base-member.t`
- `pa19/tests/general/200-dependent-relational-enable-if-return.t`
- `pa19/tests/general/200-explicit-function-template-type-arg-drops-nontype-overload.t`
- `pa19/tests/general/200-nontype-conversion-operator-dependent-template-id.t`
- `pa19/tests/general/200-nontype-template-redecl-typedef-spelling.t`
- `pa19/tests/general/200-qualified-template-id-nontype-argument-scope.t`
- `pa19/tests/spec/100-constexpr-integral-template-argument.t`
- `pa19/tests/spec/100-dependent-logical-or-template-default.t`
- `pa19/tests/spec/100-dependent-nontype-template-parameter-type.t`
- `pa19/tests/spec/100-dependent-nontype-template-type.t`
- `pa19/tests/spec/100-dependent-qualified-member-type-reexport.t`
- `pa19/tests/spec/100-dependent-sizeof-template-default.t`
- `pa19/tests/spec/100-dependent-static-assert-defers.t`
- `pa19/tests/spec/100-inherited-static-member-value.t`
- `pa19/tests/spec/100-integral-constant-static-value.t`
- `pa19/tests/spec/100-integral-nontype-default.t`
- `pa19/tests/spec/100-qualified-function-template-call.t`
- `pa19/tests/spec/100-qualified-template-member-type-cast-static-assert.t`
- `pa19/tests/spec/100-qualified-template-member-type-class-scope-argument.t`
- `pa19/tests/spec/100-template-static-assert.t`
- `pa19/tests/spec/100-unnamed-integral-nontype-parameter.t`
- `pa19/tests/spec/200-inline-explicit-function-specialization-weak.t`
- `pa19/tests/spec/200-rvalue-pointer-derived-template-call.t`
- `pa19/tests/spec/200-sizeof-union-type-nttp.t`

### G15

- `pa19/tests/general/100-class-explicit-specialization.t`
- `pa19/tests/general/100-empty-pack-explicit-specialization.t`
- `pa19/tests/general/100-function-explicit-specialization-deduced.t`
- `pa19/tests/general/100-function-explicit-specialization.t`
- `pa19/tests/general/100-non-type-class-specialization.t`
- `pa19/tests/general/100-qualified-member-class-explicit-specialization.t`
- `pa19/tests/general/100-specialization-builtin-type-spacing.t`
- `pa19/tests/general/100-static-member-on-explicit-specialization.t`
- `pa19/tests/spec/100-class-explicit-specialization-simple.t`
- `pa19/tests/spec/100-explicit-specialization-member-function.t`
- `pa19/tests/spec/100-explicit-specialization-static-data-member.t`
- `pa19/tests/spec/100-function-explicit-specialization-deduced.t`
- `pa19/tests/spec/100-function-explicit-specialization-parameter-name.t`
- `pa19/tests/spec/100-function-explicit-specialization-simple.t`
- `pa19/tests/spec/100-static-member-explicit-specialization-char16.t`

### G16

- `pa19/tests/general/100-explicit-specialization-redeclaration-keeps-definition.t`
- `pa19/tests/general/200-default-nontype-qualified-function-lookup.t`
- `pa19/tests/general/200-dependent-nontype-member-template-owner.t`
- `pa19/tests/general/200-dependent-static-assert-qualified-trait.t`
- `pa19/tests/general/200-inherited-member-template-bool-nontype-argument.t`
- `pa19/tests/general/200-inherited-qualified-member-template-type.t`
- `pa19/tests/general/200-nontype-functional-bool-cast-template-value.t`
- `pa19/tests/general/200-qualified-nontype-pack-function-param-mangle.t`
- `pa19/tests/general/200-substituted-nontype-sizeof-pack-return.t`
- `pa19/tests/spec/100-explicit-specialization-after-instantiation.t`
- `pa19/tests/spec/100-explicit-specialization-refreshes-stale-primary-instantiation.t`
- `pa19/tests/spec/200-dependent-specialized-default-arg-deduction.t`
- `pa19/tests/spec/200-explicit-specialization-out-of-class-member-emits.t`

### G17

- `pa19/tests/general/100-sizeof-pack-expression.t`
- `pa19/tests/spec/200-static-constexpr-array-template-member-odr-use.t`

### G18

- `pa19/tests/general/100-alias-template-decltype-greater-type-argument.t`
- `pa19/tests/general/100-alias-template-decltype-member-type-argument.t`
- `pa19/tests/general/100-alias-template-decltype-shift-type-argument.t`
- `pa19/tests/general/100-dependent-less-than-enable-if-alias-type-id.t`
- `pa19/tests/general/100-dependent-less-than-enable-if-nontype-parameter.t`
- `pa19/tests/general/100-dependent-nontype-braced-cast-argument.t`
- `pa19/tests/general/100-partial-class-specialization-nested-enum-owner.t`
- `pa19/tests/general/100-qualified-nested-template-id.t`
- `pa19/tests/general/100-qualified-nontype-braced-cast-argument.t`
- `pa19/tests/general/100-qualified-static-member-template-value.t`
- `pa19/tests/general/100-variable-template-id-expression.t`
- `pa19/tests/general/200-dependent-class-template-member-type-instantiation.t`
- `pa19/tests/general/200-dependent-default-nontype-argument-eval.t`
- `pa19/tests/general/200-dependent-pack-typename-nontype-expression.t`
- `pa19/tests/general/200-direct-namespace-wins-over-using-directive.t`
- `pa19/tests/general/200-nontype-pack-comma-expression-syntax.t`
- `pa19/tests/general/200-qualified-base-type-alias-from-nontype-pack.t`
- `pa19/tests/general/200-qualified-dependent-typename-argument-instantiation.t`
- `pa19/tests/general/200-sizeof-nontype-pack-recursive-template.t`
- `pa19/tests/spec/100-dependent-qualified-return-type.t`
- `pa19/tests/spec/100-qualified-nested-template-id.t`
- `pa19/tests/spec/100-qualified-static-member-template-value.t`
- `pa19/tests/spec/200-nontype-static-outdef-value-member-preserves-type.t`
- `pa19/tests/spec/200-nontype-static-value-member-preserves-type.t`

### G19

- `pa19/tests/general/100-bound-reference-enable-if-nontype-parameter.t`
- `pa19/tests/general/100-defaulted-enable-if-overload-drop.t`
- `pa19/tests/general/100-dependent-negated-enable-if-nontype-parameter.t`
- `pa19/tests/general/100-dependent-void-t-sfinae-alias.t`
- `pa19/tests/general/100-explicit-function-template-arg-substitution.t`
- `pa19/tests/general/100-nontype-function-parameter-adjustment.t`
- `pa19/tests/general/100-structured-enable-if-sizeof-pack-value.t`
- `pa19/tests/general/100-templated-constructor-special-member-collection.t`
- `pa19/tests/general/100-typename-qualified-type-construction-expression.t`
- `pa19/tests/spec/100-explicit-specialization-type-argument-does-not-eagerly-instantiate.t`
- `pa19/tests/spec/100-late-explicit-specialization-type-argument-does-not-eagerly-instantiate.t`
- `pa19/tests/spec/100-nontype-function-pointer-argument.t`
- `pa19/tests/spec/100-nontype-reference-argument.t`
- `pa19/tests/spec/100-typename-qualified-type-construction-expression.t`
- `pa19/tests/spec/200-defaulted-type-arg-specialization-nontype-value.t`
- `pa19/tests/spec/200-explicit-type-arg-dependent-qualified-member-template-id.t`

### G20

- `pa19/tests/general/100-and-alias-decltype-pack-call.t` -> PA34
- `pa19/tests/general/100-is-assignable-deleted-special-member.t` -> PA34
- `pa19/tests/general/100-pair-template-parameter-clause-smoke.t` -> PA34
- `pa19/tests/general/200-defaulted-dependent-nontype-expression-syntax.t` -> PA34
- `pa19/tests/general/200-structured-nothrow-invocable-cache-default.t` -> PA34
- `pa19/tests/general/200-using-directive-template-id-member-pointer-owner.t` -> PA28
- `pa19/tests/spec/200-bool-alias-base-preserves-nontype-type.t` -> PA34
- `pa19/tests/spec/200-dependent-member-function-pointer-nontype-detection.t` -> PA28
- `pa19/tests/spec/200-member-pointer-nontype-template-parameter.t` -> PA28
