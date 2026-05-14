# PA18-PA22 Test Grouping Notes

## PA18

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/spec/` has 66 N3485-anchored `.t` tests;
  `tests/general/` has 107 broader regression/integration `.t` tests.
- Moved from `tests/spec/` to `tests/general/`: `100-dependent-default-construction-through-template-subscript`,
  `100-required-template-special-member-emission`,
  `100-new-expression-completes-template-layout`,
  `100-local-using-alias-template-member-body`,
  `100-local-using-directive-template-member-enumerator`,
  `100-lazy-header-parenthesized-qualified-function-template-call`,
  `100-inherited-constructor-using-alias-template`,
  `100-namespace-qualified-class-template-definition`,
  `100-lazy-header-qualified-function-template-text-lookup`,
  `100-namespace-template-function-before-tls-object`,
  `100-function-template-template-parameter-deduction`,
  `100-inherited-typedef-hidden-friend-overload`,
  `100-template-array-reference-cv-default-arg`,
  `100-reentrant-reference-collection-override-param`,
  `100-template-argument-qualified-member-template-type`,
  `100-functional-cast-argument-nested-type-hides-function`,
  `100-using-directive-template-member-type-typedef`,
  `100-inline-dependent-pack-result-type`,
  `100-unused-class-template-hidden-friend-body`,
  `100-class-template-static-member-assignment-lvalue`,
  `100-dependent-enable-if-return-less-equal`,
  `100-dependent-enable-if-return-sizeof-less`,
  `100-member-template-local-using-does-not-suppress-adl`,
  `100-local-using-directive-qualified-template-argument`,
  `100-decltype-qualified-type-template-argument`,
  `100-pack-expansion-aggregate-brace-init`,
  `100-cross-instantiation-member-template-source-owner`,
  `100-qualified-using-directive-function-template-call`,
  `100-function-template-local-static-per-specialization`,
  `100-empty-class-pack-member-template-call`,
  `100-friend-template-adl-existing-definition`,
  `100-function-template-pair-vs-range-predicate`, and
  `100-empty-pack-member-template-owner-key`.
- Kept in spec: anchored first-tier template tests under `14.1`, `14.2`,
  `14.3.1`, `14.3.3`, `14.4`, `14.5.1`, `14.5.2`, `14.5.3`,
  `14.5.4`, `14.5.6`, `14.6.1`, `14.6.3`, `14.6.4`, `14.7.1`,
  `14.8.1`, `14.8.2`, and `14.8.3`, with supporting core clauses where
  needed.
- Tests still needing N3485 comments in spec: none found after the move.
- Follow-up duplicate cleanup removed canonical-token duplicate general copies
  that are now covered by N3485-anchored spec tests.
- Missing tests to add later: reduced template-template matching,
  member-template redeclaration, friend-template, dependent/non-dependent
  lookup, and pack/forwarding deduction probes.
- README changes: documented `tests/spec/` as N3485 anchored, `tests/general/`
  as regression/integration coverage, and noted `make test-strict` witness
  comparisons.
- Validation: after duplicate cleanup, root `make test-strict ...` passed PA18
  with `SUMMARY compared=169 failures=0 skipped=4`; root `make test-report ...`
  passed `2766 / 2766`.
- Open questions: whether some general PA18 reducers with existing N3485-like
  comments should later be reduced into new spec tests instead of being kept as
  broad regressions.

## PA19

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/spec/` has 48 N3485-anchored `.t` tests;
  `tests/general/` has 75 broader regression/integration `.t` tests.
- Moved from `tests/spec/` to `tests/general/`: `100-using-namespace-ambiguous-less-than-or`,
  `100-is-assignable-deleted-special-member`,
  `200-nontype-template-redecl-typedef-spelling`,
  `200-dependent-static-assert-qualified-trait`,
  `200-enum-nontype-template-vtable-mangling`,
  `200-dependent-qualified-nontype-base-argument`,
  `200-dependent-qualified-value-base-member`,
  `200-substituted-nontype-sizeof-pack-return`,
  `200-dependent-nontype-vtable-redecl-owned-syntax`,
  `200-nontype-conversion-operator-dependent-template-id`,
  `200-explicit-function-template-type-arg-drops-nontype-overload`,
  `200-dependent-nontype-parameter-type-default`,
  `200-dependent-qualified-type-missing-typename-bad`,
  `200-qualified-dependent-typename-argument-instantiation`,
  `200-dependent-relational-enable-if-return`,
  `200-sizeof-nontype-pack-recursive-template`,
  `200-dependent-pack-typename-nontype-expression`,
  `200-qualified-base-type-alias-from-nontype-pack`, and
  `200-qualified-nontype-pack-function-param-mangle`.
- Kept in spec: anchored PA19 metaprogramming tests under `14.1`,
  `14.2`, `14.3.1`, `14.3.2`, `14.5.1`, `14.5.2`, `14.5.7`,
  `14.6.2`, `14.7.1`, `14.7.3`, `14.8.1`, and `14.8.2`, with
  supporting constexpr/static assertion clauses where needed.
- Tests still needing N3485 comments in spec: none found after the move.
- Follow-up duplicate cleanup removed canonical-token duplicate general copies
  that are now covered by N3485-anchored spec tests.
- Missing tests to add later: reduced explicit specialization ordering and
  visibility, integral non-type argument equivalence, dependent non-type
  parameter type, and static data member specialization probes.
- README changes: documented `tests/spec/` as N3485 anchored, `tests/general/`
  as regression/integration coverage, and noted `make test-strict` witness
  comparisons.
- Validation: after duplicate cleanup, root `make test-strict ...` passed PA19
  with `SUMMARY compared=119 failures=0 skipped=4`; root `make test-report ...`
  passed `2766 / 2766`.
- Open questions: whether the moved dependent qualified-name reducers should
  become smaller PA19 spec tests or remain general self-host sentinels.

## PA20

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/spec/` has 2 N3485-anchored `.t` tests;
  `tests/general/` has 56 broader constexpr regression/integration `.t`
  tests.
- Moved from `tests/spec/` to `tests/general/`: all previous spec tests except
  `400-constexpr-function-and-constructor` and
  `400-constexpr-reference-parameter-conversion-constructor`. The moved set is
  `100` through `458` excluding those two basenames.
- Kept in spec: `400-constexpr-function-and-constructor` and
  `400-constexpr-reference-parameter-conversion-constructor`, both anchored to
  `7.1.5 [dcl.constexpr]`.
- Tests still needing N3485 comments in spec: none found after the move.
- Missing tests to add later: reduced C++11 `constexpr` declaration validity,
  literal type, constant initialization, core constant-expression rejection,
  pointer/reference constant evaluation, and aggregate/object-valued constant
  evaluation probes.
- README changes: documented `tests/spec/` as N3485 anchored, `tests/general/`
  as regression/integration coverage, and noted that PA20 has no strict witness
  target.
- Validation: `make -C pa20 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed `58/58`.
- Open questions: whether some general PA20 constexpr tests should be annotated
  and promoted back to spec once the exact N3485 clauses are audited.

## PA21

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/spec/` has 27 N3485-anchored `.t` tests;
  `tests/general/` has 96 broader regression/integration `.t` tests.
- Moved from `tests/spec/` to `tests/general/`: `100-function-signature-partial-specialization-functor-assignment`,
  `100-pointer-traits-alias-chain-core`,
  `100-variable-template-run-specialization-selection`,
  `100-shared-partial-specialization-variable-selection`,
  `400-pointee-cv-class-partial-specialization`,
  `400-template-id-pointee-cv-class-partial-specialization`,
  `400-forwarding-reference-qualified-enumerator`,
  `400-nttp-pack-void-comma-expression`,
  `400-relative-qualified-partial-specialization`,
  `400-dependent-bool-base-trait-type-argument`,
  `400-inherited-qualified-alias-template-type`,
  `400-extern-template-empty-function-template-id-deduces`,
  `400-alias-rebind-partial-specialization-shadow`,
  `400-dependent-bool-partial-static-value-mangle`,
  `400-partial-specialization-current-instantiation-dependent-nontype`, and
  `400-void-head-pack-partial-specialization-ordering`.
- Kept in spec: anchored specialization/entity tests under `14.5.5`,
  `14.5.5.2`, `14.5.7`, `14.7.1`, `14.7.2`, `14.7.3`, and
  `14.8.2.4`, with supporting member-template clauses where needed. A
  follow-up citation audit moved variable-template cases out of `tests/spec/`
  because N3485 has no `temp.var` clause.
- Tests still needing N3485 comments in spec: none found after the move.
- Follow-up duplicate cleanup removed canonical-token duplicate general copies
  that are now covered by N3485-anchored spec tests.
- Missing tests to add later: reduced alias-template entity,
  class/function partial specialization selection, explicit-instantiation
  ownership, constructor/member-template specialization ownership, and partial
  ordering boundary probes.
- README changes: documented `tests/spec/` as N3485 anchored, `tests/general/`
  as regression/integration coverage, and noted `make test-strict` witness
  comparisons.
- Validation: after duplicate cleanup, root `make test-strict ...` passed PA21
  with `SUMMARY compared=121 failures=0 skipped=2`; root `make test-report ...`
  passed `2766 / 2766`.
- Open questions: whether the moved high-numbered partial-specialization
  reducers should be split into smaller spec probes later.

## PA22

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/spec/` has 35 N3485-anchored `.t` tests;
  `tests/general/` has 290 broader regression/integration `.t` tests.
- Moved from `tests/spec/` to `tests/general/`: `100-variable-template-detected-idiom-direct-arg`,
  `400-member-template-outer-ref-trailing-return-decltype`,
  `400-constructor-template-default-arg-target-aware`,
  `400-local-pack-template-id-paren-init`,
  `400-reference-alias-top-cv-return-binding`,
  `400-member-template-assignment-sfinae-copy-fallback`,
  `400-destructor-template-id-sfinae`,
  `400-local-pack-template-id-paren-init-in-if`,
  `400-alias-template-overload-sfinae`,
  `400-explicit-template-call-transitive-base-deduction`,
  `400-conversion-operator-qualified-template-result-call`,
  `400-structured-lazy-and-enable-if-pack-size`,
  `400-incomplete-sizeof-partial-specialization-sfinae`,
  `400-member-template-enable-if-dependent-qualified-return`,
  `400-qualified-explicit-template-alias-return-sfinae`,
  `400-alias-sfinae-inherited-member-value`,
  `400-dependent-owner-lifecycle-validation`,
  `400-member-alias-assignable-rvalue-assignment`,
  `400-constructible-derived-base-template-ctor`,
  `400-structured-type-id-bool-template-sfinae`,
  `500-structured-bool-functional-trait-sfinae`,
  `500-pack-alias-functional-bool-trait-sfinae`,
  `500-alias-pack-enable-if-constexpr-constructor`,
  `500-pack-alias-inherited-bool-constant`,
  `500-decltype-call-substitution-failure-partial-specialization`,
  `500-dependent-template-id-no-eager-layout`,
  `500-sfinae-primary-static-const-value-fold`,
  `500-abstract-array-parameter-sfinae`,
  `500-compatible-alias-converting-ctor`,
  `500-dependent-typename-enable-if-candidate`,
  `500-dependent-typename-member-enable-if-return`,
  `500-defaulted-sfinae-ctor-candidate-drop`,
  `500-source-owner-member-template-sfinae-default`,
  `500-source-namespace-base-sfinae-chain`,
  `500-range-array-reference-mutable-begin`, and
  `500-alias-default-dependent-qualifier-type-trait`.
- Kept in spec: anchored deduction/substitution/SFINAE tests under `14.1`,
  `14.2`, `14.5.1`, `14.5.2`, `14.5.7`, `14.6.2`,
  `14.6.4`, `14.6.4.1`, `14.7.1`, `14.8`, `14.8.1`, `14.8.2`,
  `14.8.2.1`, `14.8.2.2`, `14.8.2.3`, `14.8.2.4`, `14.8.2.5`,
  and `14.8.3`, with supporting conversion, overload, namespace, temporary,
  destructor, and initializer-list clauses where needed. A follow-up citation
  audit moved the variable-template detector case out of `tests/spec/` because
  N3485 has no `temp.var` clause.
- Tests still needing N3485 comments in spec: none found after the move.
- Follow-up duplicate cleanup removed canonical-token duplicate general copies
  that are now covered by N3485-anchored spec tests.
- Missing tests to add later: reduced explicit-template-argument deduction,
  function-address deduction, conversion-function-template deduction,
  constructor-template participation, non-deduced contexts, and compact
  `enable_if` / `void_t` / detector probes.
- README changes: documented `tests/spec/` as N3485 anchored, `tests/general/`
  as regression/integration coverage, and noted `make test-strict` witness
  comparisons.
- Validation: after duplicate cleanup, root `make test-strict ...` passed PA22
  with `SUMMARY compared=311 failures=0 skipped=14`; root `make test-report ...`
  passed `2766 / 2766`.
- Open questions: whether some moved high-numbered SFINAE reducers should be
  split into smaller spec probes after the N3485 clause audit.
