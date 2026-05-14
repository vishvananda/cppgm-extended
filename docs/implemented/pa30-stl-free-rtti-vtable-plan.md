**PA30 STL-Free RTTI/Vtable Plan**

**Goal**
Build a robust `pa30` test ladder that exercises the core hosted polymorphism ABI surface without depending on the STL. The purpose is to expose:

- vtable ownership and coalescing mistakes
- RTTI emission/import mistakes
- address-point and offset-to-top mistakes
- `this`-adjustment and thunk mistakes
- construction/destruction dispatch mistakes

The intent is to catch the core ABI/runtime bugs in small tests before they are amplified by libc++.

**Why This Helps**
Many of the recent failures were discovered through STL code, but the underlying bugs are mostly not STL-specific. The STL is acting as a dense consumer of:

- virtual dispatch
- RTTI
- multiple/virtual inheritance
- imported polymorphic definitions
- cross-TU key-function ownership

So a smaller `pa30` suite can give earlier and more legible signal while keeping `pa31` focused on library-specific hosted compatibility.

**Existing Seeds**
The repository already has a few useful `pa30` starting points:

- [150-host-virt-smoke.t.1](/Users/vishvananda/cppgm/pa30/tests/spec/150-host-virt-smoke.t.1)
  - basic single-inheritance virtual dispatch
- [125-inline-ctor-external-vtable-import.helper.h](/Users/vishvananda/cppgm/pa30/tests/spec/125-inline-ctor-external-vtable-import.helper.h)
  - host-owned polymorphic definition imported by a `cppgm++` TU
- [118-polymorphic-inline-header-duplicate.helper.h](/Users/vishvananda/cppgm/pa30/tests/spec/118-polymorphic-inline-header-duplicate.helper.h)
  - duplicate/coalescing behavior for inline polymorphic definitions

These should be treated as the base of a larger `pa30` ABI ladder rather than isolated spot checks.

**Test Design Rules**
Prefer tests with these properties:

- no STL containers, streams, locales, or `std::function`
- no dependency on `<typeinfo>` unless the specific goal is to test that surface
- exit-code assertions instead of output formatting
- tiny class hierarchies with obvious expected behavior
- one ABI mechanism per test family where possible
- object inspection alongside runtime checking

Recommended harness shape:

- one `.helper.h` for the shared class declarations
- one or more `.lib.*.cpp` files compiled by the host compiler when host ownership/import is part of the test
- one or more `.t.*` files compiled by `cppgm++`
- runtime result checked by exit status
- `.inspect.plan` or `.inspect.cmd` used to check object-surface expectations such as:
  - `_ZTV*`
  - `_ZTI*`
  - unresolved imports that should remain external
  - duplicate/coalesced ownership cases

**Recommended Families**

1. Single-Inheritance Dispatch

- direct virtual call through base pointer
- virtual destructor through base pointer
- out-of-line key function ownership

Purpose:

- basic vtable emission
- destructor variant selection
- simplest host-imported vtable surface

2. External Key-Function Ownership

- declare a polymorphic class in a header
- define the key function in a host-compiled `.lib.*.cpp`
- use the class from a `cppgm++` TU

Purpose:

- imported vtable/RTTI ownership
- correct unresolved host imports
- avoidance of accidental duplicate ownership

This is the generalization of the existing `125` pattern.

3. RTTI Without The STL

Use `dynamic_cast`, not `typeid`, for the first RTTI wave.

Add cases for:

- successful downcast
- failed downcast returning null
- reference-form `dynamic_cast` that throws on failure, if exception coverage is desired in `pa30`

Purpose:

- RTTI object emission/import
- dynamic type identity
- address-point correctness as used by runtime casts

4. Multiple Inheritance

Use two simple bases and one derived:

- call virtual through each base
- cast from one base pointer to the complete object and back

Purpose:

- secondary-vtable handling
- `this` adjustment
- non-zero base offsets

This is where many “looks fine in single inheritance” bugs start to surface.

5. Virtual Inheritance

Add a diamond or near-diamond hierarchy:

- one shared virtual base
- dispatch or field access through different base paths
- `dynamic_cast<void*>` can be useful here if desired

Purpose:

- offset-to-top correctness
- virtual-base offset handling
- construction vtable / address-point correctness

This is likely the highest-value STL-free family for the current hosted ABI work.

6. Covariant Return / Thunks

Use a tiny hierarchy where an override returns a more-derived pointer reference than the base signature.

Purpose:

- return thunks
- combined `this`/result adjustment
- vtable slot correctness when the nominal signature matches but ABI behavior differs

7. Construction / Destruction Dispatch

Add tests where:

- a base constructor calls a virtual
- a base destructor calls a virtual
- the expected target is the base implementation, not the most-derived override

Purpose:

- construction-vtable behavior
- destructor-phase dynamic type behavior
- address-point selection during lifetime transitions

8. Cross-TU Duplicate / Coalescing

Extend existing duplicate-owner tests to polymorphic classes with:

- inline virtual definitions in headers
- out-of-line key functions in one TU only
- duplicated inclusion in multiple `cppgm++` TUs

Purpose:

- weak/comdat ownership
- duplicate symbol suppression without losing object-surface visibility
- interaction between polymorphism and existing duplicate-owner rules

**Suggested Rollout Order**

Start with the lowest-complexity, highest-signal families:

1. strengthen single-inheritance dispatch and destructor tests
2. expand external key-function ownership/import tests
3. add RTTI through `dynamic_cast`
4. add multiple-inheritance tests
5. add virtual-inheritance tests
6. add covariant-return/thunk tests
7. add construction/destruction dispatch tests
8. expand duplicate/coalescing polymorphic tests

That ordering should expose most core hosted ABI breakage before the library-specific `pa31` layer is involved.

**What Should Stay In PA31**
This `pa30` suite should not try to own library-specific ABI surfaces such as:

- libc++ inline-namespace and ABI-tag behavior
- `std::type_info`, `std::bad_cast`, or facet-specific runtime contracts
- iostream/locale virtual slot assumptions
- imported stdlib explicit-instantiation ownership

Those remain better covered by `pa31` hosted compatibility tests.

**Completion Criteria**
A useful first version of this plan is complete when `pa30` has:

- a clear STL-free single-inheritance polymorphism owner test set
- at least one host-imported key-function ownership test
- at least one RTTI `dynamic_cast` success/failure family
- at least one multiple-inheritance test
- at least one virtual-inheritance test
- object-surface inspection on the ownership/import-sensitive cases

At that point `pa30` should be able to catch a large fraction of RTTI/vtable regressions before they first appear inside STL-heavy `pa31` cases.

**Completion Note**
The first ladder described here landed and this plan is now archived. The
current implemented `pa30` ABI owner set includes:

- existing ownership/inspection seeds:
  - `118-polymorphic-inline-header-duplicate`
  - `125-inline-ctor-external-vtable-import`
- basic virtual dispatch:
  - `150-host-virt-smoke`
- RTTI / `dynamic_cast`:
  - `200-host-single-inheritance-dynamic-cast`
  - `201-host-failed-dynamic-cast-null`
  - `206-host-dynamic-cast-void`
  - `209-host-virtual-inheritance-dynamic-cast-void`
- imported RTTI / polymorphic ownership:
  - `205-host-external-rtti-import`
- multiple inheritance and thunk-adjacent behavior:
  - `207-host-multiple-inheritance-virtual-dispatch`
  - `208-host-imported-covariant-return-adjustment`

Deeper virtual-inheritance dispatch, destructor-phase behavior, and broader
thunk coverage remain useful future regression work, but the initial goal of a
small STL-free ABI ladder was met.
