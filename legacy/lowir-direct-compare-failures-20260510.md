# LowIR Direct Compare Failure Note - 2026-05-10

Command:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report-nobuild \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_SKIP_DEV_REBUILD=1 \
  TEST_REPORT_ASSIGNMENT_JOBS=1 TEST_REPORT_SUBTEST_JOBS=4 ORDERED=1
```

Initial result after commit `4c27e1af`: `2660 / 2710` passed.

After updating the affected LowIR refs, the same command passed `2710 / 2710`.

Classification: all 50 failures were LowIR reference updates, not new semantic
failures.  The diffs are constructor/destructor entry-point text drift from
base-subobject calls now targeting C2/D2 `__base_entry` functions instead of
the complete C1/D1 entries, plus added explicit base-entry definitions where
the LowIR output now preserves them.

The affected tests were:

- `pa15/tests/spec/180-single-inheritance.t`
- `pa15/tests/spec/192-derived-method-hides-base-field-call.t`
- `pa15/tests/spec/193-base-field-access.t`
- `pa15/tests/spec/217-derived-base-constructor-member-init.t`
- `pa15/tests/spec/222-base-default-argument-constructor-action.t`
- `pa15/tests/spec/230-aliased-base-mem-initializer-match.t`
- `pa15/tests/spec/280-derived-shift-prefers-free-char-pointer.t`
- `pa15/tests/spec/312-friend-derived-access-inherited-protected-field.t`
- `pa15/tests/spec/321-inherited-conversion-operator-parameter-binding.t`
- `pa16/tests/spec/270-out-of-class-special-members.t`
- `pa16/tests/spec/326-defaulted-copy-constructor-base-copy-init.t`
- `pa16/tests/spec/332-inheriting-constructors.t`
- `pa16/tests/spec/354-local-prvalue-init-elides-move.t`
- `pa16/tests/spec/355-conditional-local-prvalue-init-elides-copy.t`
- `pa16/tests/spec/376-direct-object-parameter-passthrough-base-copy.t`
- `pa17/tests/spec/300-virtual-base-reference.t`
- `pa17/tests/spec/310-virtual-base-pointer.t`
- `pa17/tests/spec/320-virtual-destructor-override.t`
- `pa17/tests/spec/330-inherited-virtual.t`
- `pa17/tests/spec/340-final-virtual.t`
- `pa17/tests/spec/400-diamond-virtual-destructor-slot-merge.t`
- `pa17/tests/spec/401-multibase-implicit-virtual-destructor-slot-merge.t`
- `pa17/tests/spec/402-virtual-override-dispatch.t`
- `pa17/tests/spec/403-covariant-return-override.t`
- `pa17/tests/spec/406-nonprimary-direct-base-ctor-vtable-offset.t`
- `pa17/tests/spec/407-explicit-virtual-destructor-call-nonvirtual.t`
- `pa17/tests/spec/408-header-out-of-class-virtual-vtable.t`
- `pa17/tests/spec/409-static-reference-downcast-nonprimary-base.t`
- `pa17/tests/spec/410-base-qualified-virtual-call-nonvirtual.t`
- `pa17/tests/spec/411-key-function-vtable-without-local-construction.t`
- `pa18/tests/spec/133-inherited-constructor-using-alias-template.t`
- `pa18/tests/spec/143-template-array-reference-cv-default-arg.t`
- `pa18/tests/spec/146-member-call-ignores-enclosing-template-distractor.t`
- `pa18/tests/spec/153-reentrant-reference-collection-override-param.t`
- `pa21/tests/general/425-explicit-specialization-out-of-class-ctor-replay.t`
- `pa21/tests/spec/035-explicit-specialization-out-of-class-ctor-replay.t`
- `pa21/tests/spec/479-dependent-bool-base-trait-type-argument.t`
- `pa22/tests/general/521-carried-dependent-bool-member-argument.t`
- `pa22/tests/spec/514-defaulted-sfinae-ctor-candidate-drop.t`
- `pa27/tests/spec/140-typeid-polymorphic.t`
- `pa27/tests/spec/150-dynamic-cast-success.t`
- `pa27/tests/spec/160-dynamic-cast-fail.t`
- `pa27/tests/spec/197-bad-dynamic-cast-reference.t`
- `pa27/tests/spec/200-source-base-ref-catch.t`
- `pa28/tests/spec/120-multiple-inheritance-copy.t`
- `pa28/tests/spec/130-dynamic-cast-void.t`
- `pa29/tests/spec/110-nonprimary-virtual-dispatch.t`
- `pa29/tests/spec/120-sibling-dynamic-cast.t`
- `pa29/tests/spec/130-typeid-nonprimary-view.t`
- `pa29/tests/spec/140-multibase-class-return-adjustor-thunk.t`

Notes:

- Exit statuses matched for all 50 tests.
- Four tests had stale or absent `.ref.stdout` files compared with generated
  empty stdout, but the LowIR harness does not compare stdout for these tests.
  No stdout refs were changed as part of this update.
