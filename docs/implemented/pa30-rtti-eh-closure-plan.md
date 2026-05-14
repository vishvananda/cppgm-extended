# PA30 RTTI/EH Closure Plan

## Goal

Close the main remaining STL-free RTTI and EH coverage gaps in `pa30` by
adding real owner tests and fixing the implementation until they pass.

This plan is intentionally execution-oriented:

- add the missing owner tests first, even if they fail
- treat the first failures as implementation work, not as reasons to narrow
  the plan
- only archive the plan once the targeted gaps are covered by passing tests

The purpose is to stop leaving important hosted ABI/runtime behavior visible
only through broader `pa31` or bootstrap failures.

## Why This Is Needed

The initial STL-free `pa30` ABI ladder landed, but it stopped at a first useful
bar. The remaining holes are concentrated in:

- typed RTTI failure paths
- deeper multiple/virtual-inheritance cast paths
- lifetime-phase virtual dispatch
- self-emitted covariant return adjustment
- typed class-exception and rethrow EH behavior

These are exactly the surfaces that tend to turn into hard-to-read hosted
runtime failures later when they are only exercised indirectly through libc++.

## Working Rule

For this plan, we do not close gaps by skipping tests that expose them.

When a new owner test fails:

1. keep the test in the tree
2. diagnose the missing semantic / LowIR / object / runtime behavior
3. implement the missing support
4. rerun the focused owner set until it passes

## Remaining Gaps To Close

### 1. Typed RTTI Failure Paths

Add owner tests for:

- `dynamic_cast<Ref &>` failure on a polymorphic source, caught in user code
- `typeid(*ptr)` on a null polymorphic pointer, caught in user code

Purpose:

- exercise the runtime failure path rather than only successful pointer-form
  casts
- prove that RTTI failure behavior is self-emitted and host-compatible

### 2. Deeper Cast Topologies

Add owner tests for:

- multiple-inheritance cross-cast (`BaseA *` to sibling `BaseB *`)
- virtual-inheritance typed cast across the shared virtual base path

Purpose:

- cover non-trivial adjustment logic beyond single-inheritance and
  `dynamic_cast<void *>`
- expose offset-to-top / secondary-vtable / virtual-base adjustment mistakes

### 3. Lifetime-Phase Virtual Dispatch

Add owner tests for:

- delete-through-base destructor execution
- base-constructor virtual dispatch using the base implementation
- base-destructor virtual dispatch using the base implementation

Purpose:

- exercise construction/destruction vptr state, not just steady-state dispatch
- catch the “links fine, runs wrong” class of vtable bugs earlier

### 4. Self-Emitted Covariant Return Adjustment

Add an owner test where the covariant-return adjustment is emitted by `cppgm++`
itself rather than imported from a host-owned TU.

Purpose:

- cover thunk/adjustment behavior on the self-emitted path
- avoid relying only on the already-landed imported-owner test

### 5. Typed Class EH Paths

Add owner tests for:

- throwing and catching a simple user-defined polymorphic class by base
  reference
- rethrow through nested handlers

Purpose:

- cover class-type EH matching with RTTI involvement
- prove that the rethrow path and class-type catch dispatch survive the
  self-emitted EH path

## Suggested Test Set

The intended closure bar for this plan is a concrete owner set in `pa30`:

1. reference-form `dynamic_cast` failure
2. null-polymorphic `typeid` failure
3. multiple-inheritance cross-cast
4. virtual-inheritance typed cast
5. delete-through-base destructor smoke
6. constructor-phase virtual dispatch smoke
7. destructor-phase virtual dispatch smoke
8. self-emitted covariant-return adjustment smoke
9. typed class-exception base-reference catch smoke
10. rethrow smoke

These tests should stay small, avoid STL containers/iostreams, and use the
existing `pa30` host-interop harness patterns.

## Validation

This plan is complete when:

- the new owner tests exist in `pa30/tests/spec`
- the focused `pa30` owner set passes for both `my` and `ref`
- any required semantic / LowIR / backend fixes are implemented rather than
  worked around by reducing the tests

Once complete, move this document to `docs/implemented/`.
