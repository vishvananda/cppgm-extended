# Subobject Semantic Closure

This document defines the recurring semantic-family closure pass for:

- concrete class subobjects in `PA15`
- class-value / operator / pass-return subobject behavior in `PA16`
- template-bound and reference-only subobject completion in `PA21`

Use this closure pass when a hosted frontier item falls into any of these shapes:

- non-static data members of class type
- nested subobject field access
- prvalue member access or method/operator use on class objects
- pass-by-value or return-by-value of objects with class subobjects
- template specializations or reference-only class shells used as member types

The purpose is to close the semantic family slice instead of fixing only the one hosted smoke
that happened to expose it.

## Coverage Matrix

### PA15: Concrete Complete Subobjects

Use `pa15` to harden the ordinary non-template object model:

- existing lifetime/init coverage:
  - `pa15/tests/spec/172-default-member-initializer-class-member.t`
  - `pa15/tests/spec/173-default-member-initializer-aggregate-member.t`
  - `pa15/tests/spec/174-member-initializer-aggregate-member.t`
  - `pa15/tests/spec/175-member-object-lifetime.t`
- added closure coverage:
  - `pa15/tests/spec/210-simple-class-member-object-access.t`
  - `pa15/tests/spec/211-class-member-object-sizeof.t`
  - `pa15/tests/spec/212-nested-class-member-object-access.t`

### PA16: Class Values Through Subobjects

Use `pa16` to harden prvalue, operator, and pass/return behavior on class objects:

- existing class-value coverage:
  - `pa16/tests/spec/240-pass-by-value-lvalue.t`
  - `pa16/tests/spec/245-pass-return-forwarding.t`
  - `pa16/tests/spec/246-class-reference-return-forward-to-const-ref.t`
  - `pa16/tests/spec/247-class-reference-return-forward-to-rvalue-ref.t`
  - `pa16/tests/spec/290-prvalue-field-access-temporary.t`
  - `pa16/tests/spec/291-prvalue-method-call-temporary.t`
  - `pa16/tests/spec/292-prvalue-field-access-function-return.t`
- added closure coverage:
  - `pa16/tests/spec/294-member-prefix-decrement.t`
  - `pa16/tests/spec/295-member-deref-after-prefix-decrement.t`
  - `pa16/tests/spec/296-subobject-member-deref-after-prefix-decrement.t`
  - `pa16/tests/spec/297-nested-subobject-pass-return-by-value.t`

### PA21: Template-Bound Subobject Completion

Use `pa21` to harden reference-only shells, upgrade-to-complete paths, and template-bound
member types:

- existing completion / reference-only coverage:
  - `pa21/tests/spec/305-template-parameter-class-member-object.t`
  - `pa21/tests/spec/306-sizeof-completes-class-template.t`
  - `pa21/tests/spec/307-derived-layout-completes-template-base.t`
  - `pa21/tests/spec/308-array-element-completes-class-template.t`
  - `pa21/tests/spec/309-reference-member-static-constant-visible.t`
  - `pa21/tests/spec/310-reference-member-alias-visible.t`
  - `pa21/tests/spec/311-reference-member-class-template-visible.t`
  - `pa21/tests/spec/312-parameter-type-no-eager-member-body.t`
  - `pa21/tests/spec/313-member-typedef-no-eager-member-body.t`
  - `pa21/tests/spec/314-alias-template-no-eager-member-body.t`
  - `pa21/tests/spec/315-member-access-completes-returned-template.t`
  - `pa21/tests/spec/316-conversion-completes-template.t`
- added closure coverage:
  - `pa21/tests/spec/317-template-specialization-member-object-access.t`

## How To Run It

Run the owning suites directly:

```sh
/usr/local/opt/make/libexec/gnubin/make -C pa15 test CXX=/usr/local/opt/llvm/bin/clang++ CPGM_TEST_JOBS=1
/usr/local/opt/make/libexec/gnubin/make -C pa16 test CXX=/usr/local/opt/llvm/bin/clang++ CPGM_TEST_JOBS=1
/usr/local/opt/make/libexec/gnubin/make -C pa21 test CXX=/usr/local/opt/llvm/bin/clang++ CPGM_TEST_JOBS=1
```

If the fix has wider blast radius, finish with the aggregate sweep:

```sh
/usr/local/opt/make/libexec/gnubin/make test-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ CPGM_TEST_JOBS=1
```

## Process Rule

When a hosted frontier item lands in this semantic family:

1. fix the owning implementation in `dev/`
2. add the directly owning regression
3. add adjacent closure regressions from this matrix if the repaired surface widened
4. rerun the relevant `PA15` / `PA16` / `PA21` slice before continuing the hosted smoke chain

Do not treat repeated hosted hits in this family as isolated one-off bugs.
