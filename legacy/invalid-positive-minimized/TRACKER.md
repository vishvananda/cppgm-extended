# Invalid Positive Follow-Up Tracker

These entries started in `holding/invalid-positive-minimized/`, then moved into
the appropriate PA with compiler fixes once each repro was confirmed minimal.

## Status

- `280-template-member-noexcept-mismatch-bad.t`
  - status: done
  - target PA: `pa22`
  - note: landed as `pa22/tests/general/482-out-of-class-special-member-noexcept-mismatch-bad.t`; fixed source fixture `280-qualified-special-member-definitions.t`
- `141-missing-nontype-arg-in-dependent-return-type-bad.t`
  - status: done
  - target PA: `pa19`
  - note: landed as `pa19/tests/general/198-bad-nontype-template-argument-type-pack.t`; repaired positive kept in `pa19/tests/general/141-dependent-qualified-return-type.t`
- `147-template-deduction-cv-mismatch-call-bad.t`
  - status: done
  - target PA: `pa18`
  - note: landed as `pa18/tests/general/225-bad-function-template-deduction-cv-mismatch-call.t`; deleted duplicate-invalid `pa19/tests/general/147-qualified-function-template-call.t`
- `210-pointer-to-reference-alias-bad.t`
  - status: done
  - target PA: `pa11`
  - note: landed as `pa11/tests/spec/299-bad-pointer-to-reference-alias.t`; repaired positive kept in `pa21/tests/general/210-single-pack-cast-target.t`
- `211-invalid-operator-template-explicit-instantiation-bad.t`
  - status: done
  - target PA: `pa21`
  - note: landed as `pa21/tests/general/472-extern-template-builtin-operator-function-declaration-bad.t`; repaired positive kept in `pa21/tests/general/211-extern-template-operator-function-declaration.t`
- `270-integer-pack-builtin-bad.t`
  - status: not-a-bug
  - target PA: `pa34`
  - note: GNU `__integer_pack` hosted-compat case; original positive moved to `pa34/tests/compile/746-hosted-integer-pack-tuple-defer-compile.t`
- `304-nondependent-member-body-name-lookup-bad.t`
  - status: done
  - target PA: `pa22`
  - note: landed as `pa22/tests/general/484-nondependent-template-member-body-lookup-bad.t`; repaired positives kept in `304`, `312`, `313`, and `314`
- `330-protected-member-typedef-access-bad.t`
  - status: done
  - target PA: `pa15`
  - note: landed as `pa15/tests/spec/294-protected-member-typedef-access-bad.t`; repaired positive kept in `pa22/tests/general/330-reference-shell-qualified-storage-type-recursion.t`
- `425-explicit-specialized-ctor-definition-template-header-bad.t`
  - status: done
  - target PA: `pa21`
  - note: landed as `pa21/tests/general/473-explicit-specialized-ctor-template-header-bad.t`; repaired positive kept in `pa21/tests/general/425-explicit-specialization-out-of-class-ctor-replay.t`
- `432-local-using-shadows-template-parameter-bad.t`
  - status: done
  - target PA: `pa22`
  - note: landed as `pa22/tests/general/483-local-alias-shadows-template-parameter-bad.t`; repaired positive renamed to `432-local-alias-bound-type-forwarding.t`
