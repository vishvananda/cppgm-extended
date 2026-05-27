# Weak Linkage / COMDAT Buildout Plan

## Status

Complete for the PA30-owned object/linkage scope.

PA30 now owns and implements the duplicate-definition object semantics needed for host-linkable
separate compilation:

- target-aware weak linkage in the compiler pipeline
- ELF weak definitions plus COMDAT grouping for duplicate-emitted code
- Mach-O weak-definition / weak-reference emission
- internal-linker weak-definition resolution with strong-over-weak precedence
- generic PA30 object-inspection oracles for weak definitions, weak undefined references, and
  duplicate-definition grouping

The remaining PA32 failures should now be treated as PA32 hosted compile/link issues, not as
evidence that PA30 is still missing basic weak/coalesced object semantics.

## Why This Is PA30

Weak duplicate-definition behavior belongs to PA30, not PA32.

PA30 owns:

- host-linker-compatible object output
- practical interoperability with the real host linker
- object semantics needed for separate compilation across multiple translation units

PA32 owns hosted source/header compatibility, not object-file semantics.

## Goal

Replace stdlib-path suppression heuristics with real host-object semantics:

- emit supported duplicate definitions when they are semantically required
- mark them with target-appropriate weak/coalesced semantics
- let the real host linker deduplicate them

## Delivered

1. Linkage model above raw symbol names

- `symbol_linkage::{SL_EXTERNAL, SL_WEAK}` exists and propagates through LowIR/object export
  handling.
- Weak exported functions now use real host-visible Itanium-style object names for the supported
  duplicate-emission cases covered by PA30.

2. Machine-object weak/coalesced representation

- `machine_object::Symbol::SB_WEAK` exists.
- object symbols can carry `comdat_group`.
- undefined weak references are representable at the object-model layer.

3. ELF support

- weak definitions emit with `STB_WEAK`
- duplicate-emitted code gets COMDAT-style grouped sections
- the object-inspection oracle checks duplicate definitions structurally on Linux

4. Mach-O support

- weak definitions emit with `N_WEAK_DEF`
- weak undefined references emit with `N_WEAK_REF`
- emitted symbols keep real object-visible names so the host linker can coalesce them

5. Native-linker support

- the internal linker recognizes weak definitions
- strong definitions win over weak definitions when both are present

6. PA30 coverage

- duplicate inline/header definitions:
  - [115-inline-header-duplicate.t](/Users/vishvananda/cppgm/pa30/tests/spec/115-inline-header-duplicate.t)
- function-template duplicate definitions:
  - [116-function-template-duplicate.t](/Users/vishvananda/cppgm/pa30/tests/spec/116-function-template-duplicate.t)
- class-template member duplicate definitions:
  - [117-class-template-member-duplicate.t](/Users/vishvananda/cppgm/pa30/tests/spec/117-class-template-member-duplicate.t)
- polymorphic inline-header duplicate definitions:
  - [118-polymorphic-inline-header-duplicate.t](/Users/vishvananda/cppgm/pa30/tests/spec/118-polymorphic-inline-header-duplicate.t)
- `extern template` split with weak undefined reference coverage:
  - [119-extern-template-split.t](/Users/vishvananda/cppgm/pa30/tests/spec/119-extern-template-split.t)
- complex templated-owner ABI spelling coverage:
  - [124-complex-template-owner-duplicate.t](/Users/vishvananda/cppgm/pa30/tests/spec/124-complex-template-owner-duplicate.t)

## Practical Outcome

The PA30 boundary is now strong enough that visibility/output cleanup no longer needs to assume
"stdlib means suppress emission by path" just to avoid duplicate host-link failures.

That does not mean every remaining PA32 failure is solved. It means the remaining PA32 failures
should be debugged on their own merits:

- hosted semantic selection / output ownership
- missing runtime or builtin coverage
- remaining hosted compile/link gaps unrelated to weak/coalesced duplicate-definition semantics

## Follow-Up

No additional PA30 weak-linkage phases are required before returning to PA32.

If a future regression shows a supported duplicate-emission case still falls back to synthetic
symbol spelling, add a focused PA30 structural test for that shape and extend
[dev/src/symbol_linkage.cpp](/Users/vishvananda/cppgm/dev/src/symbol_linkage.cpp) accordingly.
