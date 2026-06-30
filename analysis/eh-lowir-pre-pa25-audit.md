# EH-shaped LowIR before PA25

## Scope

This audit ignores PA13 because PA13 is not source-to-LowIR output, and ignores
`unwind=no` because relaxed LowIR comparison strips `unwind` metadata.

The scan covers checked-in `.ref` LowIR for PA15-PA24 and verifies the same
shape is emitted by the current `dev/cppgm++`.

## Comparator behavior

EH control flow is comparison-visible. `scripts/compare_results_common.pl`
validates `eh_try`, `eh_cleanup`, `eh_end`, `throw`, `resume`, and
`exception`, then keeps those instructions in the relaxed compare stream.

EH runtime declarations are also comparison-visible if they appear. Relaxed
comparison strips metadata such as `object=`, `binding=`, `linkage=`,
`unwind=`, `return=`, and `effects=`, but it does not strip `role=eh_*`. A
stray declaration like `role=eh_resume` or `role=eh_personality` would therefore
change the oracle even if there is no EH instruction in the body.

## Current pre-PA25 findings

All original findings were hidden EH: the source did not contain `try`, `catch`,
or `throw`.

| PA | Files | Shape |
| --- | ---: | --- |
| PA15-PA24 | 0 | no remaining hidden EH control flow |

### Disposition

The original 26 rows were handled as follows:

- 19 rows were reduced in place so their PA15-PA24 LowIR no longer emits
  comparison-visible EH control flow.
- 7 rows still inherently exercise exception-aware cleanup around materialized
  temporaries or local construction, so they were moved to PA25 under
  `pa25/tests/general/200-hidden-eh-*`.

### Moved to PA25

These still contain comparison-visible EH instructions and now live at the
first source-to-LowIR assignment that owns exception lowering:

- `pa25/tests/general/200-hidden-eh-default-constructor-protected-base-subobject-scope.ref`
- `pa25/tests/general/200-hidden-eh-condition-call-argument-temporary-cleanup.ref`
- `pa25/tests/general/200-hidden-eh-conditional-expression-temp-cleanup.ref`
- `pa25/tests/general/200-hidden-eh-const-ref-bound-temp-dtor.ref`
- `pa25/tests/general/200-hidden-eh-short-circuit-condition-rhs-temp-cleanup.ref`
- `pa25/tests/general/200-hidden-eh-short-circuit-rhs-temp-cleanup.ref`
- `pa25/tests/general/200-hidden-eh-reference-prvalue-template-member-temp-cleanup.ref`

### Runtime-declaration only

No runtime-declaration-only rows are present in the current PA15-PA24 worktree.
The audit still detects that shape because `role=eh_resume`,
`role=eh_personality`, `_Unwind_Resume`, and `__gxx_personality_v0` would be
comparison-visible if they reappeared.

## Audit recommendation

Do not broaden `exception.try_catch`; that detector should keep meaning
source-level `try`/`catch`/`throw`.

The audit now has a separate hidden-EH LowIR detector that:

- only scans source-to-LowIR PAs before PA25, not PA13
- ignores `unwind=` metadata
- reports `eh_try`, `eh_cleanup`, `eh_end`, `resume`, `exception_selector`,
  `role=eh_resume`, `role=eh_personality`, `_Unwind_Resume`, and
  `__gxx_personality_v0`
- distinguishes `eh-control` from `eh-runtime-declaration-only`

This is implemented as a non-failing review queue in
`scripts/audit_pa_feature_placement.py`. After the reductions and PA25 moves,
the PA14-PA24 audit reports 0 LowIR EH review findings.
