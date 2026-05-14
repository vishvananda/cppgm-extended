# PA32 RTTI / Vtable / Thunk Lowering Review Plan

## Goal

Apply the same explicit lowering-review method used for PA23 to the PA32
RTTI/vtable/thunk surface, with a C++ ABI-specific oracle.

For PA32, runtime alone is not enough. The review should inspect:

- runtime behavior
- thunk / dispatch / ctor-dtor disassembly shape
- normalized symbol and relocation facts around vtables, RTTI objects, address
  points, and covariant-return thunks

The purpose is to expose simple backend/object-lowering issues before they only
show up in STL-heavy or bootstrap-scale failures.

## Assignment Boundary

This plan stays inside PA32's owned host-linked RTTI/vtable/thunk subset:

- imported/exported vtable ownership
- RTTI ownership and `dynamic_cast` / `typeid`
- covariant return adjustment
- ctor/dtor-phase virtual dispatch behavior
- the EH/RTTI interaction points where throw/catch behavior depends directly on
  RTTI or vtable ownership

It does not own:

- the private `cppeh` EH ABI, which belongs in PA25
- the broader host-linked EH lowering review, which belongs in the companion
  PA32 host-EH lowering review plan
- hosted-header/source compatibility, which belongs later in PA33/PA34

## Review Method

For each PA32 owner family:

1. identify or add the smallest useful source-driven owner test
2. build a host-compiler analogue of the same semantic case
3. inspect:
   - runtime behavior
   - disassembly for dispatch / casts / thunks / ctor-dtor phases
   - normalized symbol+relocation facts from the produced object(s)
4. record the comparison explicitly in a tracker with:
   - owner case name
   - analogue source
   - embedded disassembly/object snippets
   - status: `reviewed / fine`, `simplified here`, `tracked gap`, or
     `not meaningfully comparable`
5. land the simplest worthwhile fixes and place each new owner at the earliest
   PA32 surface

## Oracle Shape

PA32 review should combine:

- runtime result
- disassembly for:
  - direct virtual dispatch
  - secondary-base dispatch
  - covariant return adjustment
  - ctor/dtor-phase virtual calls
  - RTTI query/cast sequences
- normalized symbol/relocation inspection for:
  - `_ZTV*` / vtable ownership
  - address-point use vs table-base use
  - `_ZTI*` / RTTI object ownership
  - thunks and adjusting entrypoints
  - imports vs locally owned definitions
  - relocation classes for vtable, RTTI, GOT/PC-relative, and call edges

## Initial Owner Families

Start with the current PA32 ABI owner ladder and extend where coverage is thin:

1. external vtable import / duplicate ownership
2. RTTI success/failure for `dynamic_cast`
3. null-polymorphic `typeid`
4. multiple-inheritance secondary-base dispatch
5. multiple-inheritance cross-cast
6. virtual-inheritance RTTI and dispatch
7. covariant return adjustment
8. ctor-phase virtual dispatch
9. dtor-phase virtual dispatch / delete-through-base
10. EH/RTTI interaction only where RTTI ownership or typeinfo references are
    the core issue

## Expected Simple Gaps

The likely simple review wins are:

- using the right vtable symbol but the wrong address point
- secondary-base / virtual-base view selection errors
- overly indirect thunk materialization
- avoidable extra loads around covariant-return or `this`-adjustment paths
- relocation-class mismatches for imported RTTI/vtable references

## Deliverables

A complete PA32 lowering-review tranche should leave behind:

- a checked-in tracker with every reviewed owner family listed explicitly
- added or tightened STL-free PA32 owners where current coverage is thin
- normalized inspect helpers for Mach-O and ELF when raw symbol spelling differs
- backend/object fixes for the simple worthwhile gaps
- README language making clear that PA32 ownership includes the observable host
  ABI/runtime object surface, not just final runtime behavior

## Completion Note

This review tranche is complete at its first declared bar.

The checked-in tracker now covers the targeted ownership-heavy PA32 ladder
explicitly, the helper-based normalized object review works on both Mach-O and
ELF, and the worthwhile simple gaps from the review were fixed directly:

- readonly RTTI/vtable/typeinfo-name placement
- ordinary host RTTI import / `__dynamic_cast` ownership fixes
- covariant-return and destructor thunk export surface
- host-visible `_ZTC*` / `_ZTT*` construction-data symbols plus VTT-slice use in
  the reviewed virtual-inheritance ctor/dtor path

The remaining differences called out by the tracker are quality-level ones, not
missing PA32-owned ABI surface.
