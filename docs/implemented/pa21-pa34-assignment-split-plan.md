# PA21/PA30/PA31 Assignment Split Plan

## Goal

Split the three assignment milestones that have grown past a clean
single-boundary scope:

- `PA21` template completion
- `PA30` host interop
- `PA31` hosted compatibility

The split should use full assignment numbers, not `A/B` suffixes. That means
renumbering every downstream assignment once the new boundaries are adopted.

## Why These Three

These are the milestones where the current public contract no longer lines up
well with one coherent student goal:

- `PA21` mixes two different template problems:
  - specialization/entity modeling
  - deduction/substitution/SFINAE completion
- `PA30` now mixes:
  - ordinary host object/toolchain interoperability
  - host C++ ABI/runtime correctness for RTTI, vtables, and EH
- `PA31` already has a clean harness split between:
  - hosted source/header preprocessing and compile acceptance
  - hosted header-emission and link/runtime behavior

The intent is to make each public assignment own one main proof obligation.

## Numbering Policy

Do not publish split milestones as `PA21A`, `PA21B`, and so on.

Instead:

- insert new numbered assignments
- renumber all downstream assignments
- update `ROADMAP`, per-PA READMEs, harness docs, export scripts, and any
  numbering-sensitive references together

With the three inserted milestones below, the net shift after current `PA31`
is `+3`. In particular:

- current `PA32 bootstrap` becomes `PA36 bootstrap`
- current `PA33 optimize` becomes `PA35 optimize`
- current `PA34 independence` becomes `PA37 independence`

## Proposed Sequence

### Current-To-Proposed Renumbering

| Current | Proposed | Notes |
| --- | --- | --- |
| `PA21 templatecomplete` | `PA21 templatespec` | first half of current `PA21` |
| new | `PA22 templatecomplete` | second half of current `PA21` |
| `PA22 nativebackend` | `PA23 nativebackend` | renumber only |
| `PA23 link` | `PA24 link` | renumber only |
| `PA24 exceptrt` | `PA25 exceptrt` | renumber only |
| `PA25 langcore` | `PA26 langcore` | renumber only |
| `PA26 langcomplete` | `PA27 langcomplete` | renumber only |
| `PA27 objectcomplete` | `PA28 objectcomplete` | renumber only |
| `PA28 multivirt` | `PA29 multivirt` | renumber only |
| `PA29 toolchain` | `PA30 toolchain` | renumber only |
| `PA30 hostinterop` | `PA31 hostinterop` | first half of current `PA30` |
| new | `PA32 hostabi` | second half of current `PA30` |
| `PA31 hostedcompat` | `PA33 hostedcompat` | first half of current `PA31` |
| new | `PA34 hostedlink` | second half of current `PA31` |
| `PA32 bootstrap` | `PA36 bootstrap` | renumber only |
| `PA33 optimize` | `PA35 optimize` | renumber only |
| `PA34 independence` | `PA37 independence` | renumber only |

The exact short names can still be adjusted, but the split points should stay
the same.

## Split 1: Current PA21

### New PA21: `templatespec` — Template Entities and Specialization Model

This assignment should own the parts of the template system that establish the
declaration model and specialization model:

- alias templates
- variable templates
- explicit specialization declarations/definitions
- class/function partial specializations
- specialization selection and partial ordering
- explicit-instantiation declarations across the supported surface
- the dependent-name / instantiation behavior strictly required to make the
  specialization model work

The student goal should be:

- "the template declaration graph is complete and the right specialization is
  selected"

It should not yet own the full tail of deduction/substitution/SFINAE.

### New PA22: `templatecomplete` — Deduction, Substitution, and SFINAE Completion

This assignment should finish the remainder of the current `PA21` surface:

- full function-template deduction over the intended C++11 surface
- non-deduced contexts and array-bound / conversion-corner deduction cases
- substitution failure and candidate dropping
- `enable_if` / `void_t` / detected-idiom style SFINAE behavior
- the remaining dependent-call / dependent-alias / no-eager-instantiation
  behavior needed for full template usability

The student goal should be:

- "generic code stops depending on a pragmatic template subset"

### PA21 Test Division

The current flat `pa21/tests/spec` directory should be regrouped before the
renumbering so the split is mechanical.

Tests that should move into new `PA21 templatespec`:

- partial specialization and specialization selection families
- alias-template and variable-template declaration/selection families
- `extern template` / explicit-instantiation-declaration families
- explicit specialization declaration/definition ownership tests
- constructor/member-template ownership tests whose main question is "who owns
  this specialization?"

Representative current anchors:

- `100` through `125`
- `185` through `199`
- `205` through `215`
- `235`
- `339`
- `352` / `353`

Tests that should move into new `PA22 templatecomplete`:

- forwarding-reference and overload-viability deduction
- `enable_if`, detected-or, and `void_t` SFINAE fallback
- non-deduced-context and array-bound deduction cases
- explicit template-id deduction edge cases
- "body should not instantiate eagerly" families
- dependent alias/call/conversion cases whose main question is substitution
  timing rather than specialization ownership

Representative current anchors:

- `175` through `184`
- `225` through `233`
- `249` through `289`
- `304` through `324`
- `360` through `402`

The exact anchor cut does not need to match those ranges perfectly, but the
division should follow the underlying skill boundary, not raw test count.

## Split 2: Current PA30

### New PA31: `hostinterop` — Host Object and Toolchain Interoperability

This assignment should own:

- host-linker-compatible relocatable object output
- hosted `main(argc, argv)` under the host CRT
- cross-translation-unit host link
- ordinary `extern "C"` object/function interop
- static archive and shared-library interop
- weak/coalesced duplicate-definition semantics for inline/template output
- non-ABI-specific later-language host-link smokes

The student goal should be:

- "my `cppgm++ -c` objects behave like ordinary host-linkable objects"

### New PA32: `hostabi` — Host C++ ABI and Runtime Interoperability

This assignment should own the host-linked ABI/runtime surface that has now
grown well beyond a single smoke:

- virtual dispatch and vtable ownership/import/export
- RTTI object ownership and `dynamic_cast` / `typeid`
- covariant return adjustment
- ordinary host-linked EH behavior in the tested subset
- foreign/host ABI interaction cases that are not just plain symbol import

The student goal should be:

- "my host-linked objects participate correctly in the ordinary host C++ ABI"

This is distinct from `PA25` private `exceptrt`, which still owns the
`cppgm_eh_*` internal runtime path.

### PA30 Test Division

Current `pa30/tests/spec` should split by boundary, not by chronology.

Tests that should stay with new `PA31 hostinterop`:

- host entrypoint and cross-object link:
  - `100`
  - `110`
- duplicate-definition and explicit-instantiation ownership where the main
  question is object semantics rather than RTTI/vtable ABI:
  - `115`
  - `116`
  - `117`
  - `119`
  - `124`
- plain `extern "C"` interop:
  - `120`
  - `121`
  - `122`
  - `123`
- archive/shared-library interop:
  - `130`
  - `140`
- link-time structural error cases:
  - `160`
  - `170`
  - `180`
- later non-ABI host-link smokes:
  - `195` through `199`

Tests that should move into new `PA32 hostabi`:

- polymorphic duplicate/vtable-import boundary owners:
  - `118`
  - `125`
  - `150`
- the STL-free RTTI/vtable ladder:
  - `200` through `218`
- the host-linked EH / foreign-EH ladder:
  - `191` through `194`
  - `219`
  - `220`
  - `221`

The rule for edge cases is:

- if the failure is "host linker rejects these objects as ordinary objects,"
  it belongs in `PA31 hostinterop`
- if the failure is "link succeeds but host C++ ABI/runtime behavior is wrong,"
  it belongs in `PA32 hostabi`

## Split 3: Current PA31

### New PA33: `hostedcompat` — Hosted Source/Header Compatibility

This assignment should own the source-compatibility surface:

- hosted `-E` support
- practical predefined macro import
- `_Pragma`
- `__has_*`
- `#include_next`
- ignored unknown pragmas
- GNU/Clang parser concessions used by the selected hosted environment
- builtin traits, transforms, intrinsics, and builtin families used during
  hosted compile acceptance
- hosted source/header compile-only acceptance

The student goal should be:

- "the compiler can preprocess and compile the hosted environment"

### New PA34: `hostedlink` — Hosted Header Emission and Link Compatibility

This assignment should own the second half of current `PA31`:

- emitted inline/template/header definitions from hosted headers
- hosted standard-library code that now compiles, but still has to link and run
  through the plain host toolchain
- hosted link smokes where the main question is emitted symbol ownership,
  ABI spelling, or runtime behavior of hosted header-generated code

The student goal should be:

- "once hosted headers compile, their emitted code also links and runs"

This is still not a second general runtime-ABI assignment. It remains scoped to
hosted header-emission and hosted library compatibility on top of the ordinary
host ABI path already owned by `PA32 hostabi`.

### PA31 Test Division

This one is the cleanest mechanically:

- new `PA33 hostedcompat` gets:
  - current `pa31/tests/preproc`
  - current `pa31/tests/compile`
- new `PA34 hostedlink` gets:
  - current `pa31/tests/link`

Current `pa31/tests/frontier` should stay internal and should not become a
public assignment surface.

If a current `compile` test is really just a preparation step for a hosted-link
ownership issue, it may move later, but the default split should follow the
existing preproc/compile/link harness boundary.

## Execution Order

This split should not be applied piecemeal in public docs.

Do it as one coordinated renumbering pass:

1. freeze the new boundaries in `ROADMAP`
2. rewrite the affected READMEs around the new scopes
3. regroup tests into the new assignment directories
4. renumber downstream assignment directories and references
5. update wrapper scripts, export tooling, and any per-PA harness references
6. only then resume building out the post-bootstrap assignments under the new
   numbers

That is why this plan must land before the current `PA33 optimize` buildout.

## Completion Bar

This plan is complete when:

- `ROADMAP` reflects the inserted assignment numbers
- the affected assignment READMEs match the new scopes
- the tests are divided according to the boundaries above
- downstream assignment numbers are updated consistently
- the former `PA33 optimize` work has a stable new number before its buildout
  continues
