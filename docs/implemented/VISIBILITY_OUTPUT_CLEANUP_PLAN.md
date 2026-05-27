# Visibility / Output Cleanup Plan

## Status

Complete for the pre-PA32 boundary.

The visibility/output rewrite is now closed enough that PA32 failures can be treated as PA32
issues rather than as fallout from half-migrated output bookkeeping.

## Goal

Make output ownership coherent enough that:

1. requirement state changes through central APIs
2. validation runs on the common semantic/LowIR paths
3. emission decisions are driven by one requirement model
4. pre-PA32 suites stay green while the model is tightened

## Landed Cleanup

1. Central requirement APIs

- `require_function_definition(...)` is the central definition-requirement path.
- suppression and removal paths operate through the centralized helpers instead of direct
  ad hoc bit twiddling.

2. Validator on common paths

- required-definition validation now runs on the paths used for ordinary regression work,
  not only on the old PA32 object-build boundary.

3. No raw instantiation-tracking side channel

- template-instantiation output bookkeeping now flows through
  `note_instantiated_function_output(...)`.
- the old raw `track_instantiated_function(...)` hook is no longer present.

4. `output_requirements` is the source of truth

- emission and validation decisions read `ORK_*`
- the compatibility `output_required` field is gone
- trace output now reports derived definition-requirement state from `ORK_DEFINITION`

5. Pre-PA32 baseline restored and held

- full root `make test-report` is green for every suite below PA32
- the current stable boundary is:
  - `1491 / 1498` passed
  - only PA32 failures remain

## Why This Is Enough To Resume PA32

The original cleanup concerns are resolved:

- no direct suppression hacks remain in the semantic/output control path
- no direct output-tracking hook bypasses the central note path
- the validator is active on the paths used for most regression testing
- requirement flags are no longer shadowed by a second boolean source of truth

That means future failures should now be analyzed as:

- real semantic/output ownership bugs
- real hosted compile/link bugs
- real runtime/export issues

and not as ambiguity about which output bit happened to win.

## Regression Guardrails

Keep these in place:

- [scripts/audit_visibility_output_writes.sh](/Users/vishvananda/cppgm/scripts/audit_visibility_output_writes.sh)
- full root `make test-report` after each substantial output/visibility change

If a future regression reintroduces a hidden output-ownership path, add the smallest
non-PA32 regression that exposes it before fixing the PA32 symptom.
