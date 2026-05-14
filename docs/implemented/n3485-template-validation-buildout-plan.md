# N3485 Template Validation Buildout Plan

## Goal

Build a robust clang-validated template test bank from `n3485.txt` that is strong
enough to serve as the replacement student-facing oracle for the template
assignment flow.

The intended guarantee is:

- if a student implementation passes this bank, it should be hard to find a
  remaining core-language template hole through ordinary validation, and
- any later failures in hosted/STL code should usually reduce to one or more
  existing bank cases rather than revealing an entirely missing language rule.

The landing target is not "extra validation beside the old PA suites." The bank
should replace the template-focused PA-local suites with a better-factored
surface that gives implementers:

- a direct compilation oracle at the first owned boundary, and
- a stricter semantic witness oracle for template decisions.

The assignment READMEs make the milestone ownership clearer than the old local
test directories do:

- `pa17` is not a template milestone and should keep its existing polymorphism
  suite.
- `pa18` owns first-tier templates.
- `pa19` owns C++11 integral NTTPs and explicit specialization.
- `pa21` owns the specialization/entity graph and specialization selection.
- `pa22` owns full deduction, substitution, and SFINAE.

So the bank should replace the template suites for `pa18`, `pa19`, `pa21`, and
`pa22`, not force template coverage into `pa17`.

## Landing Model

Each bank-backed assignment slice should carry two linked oracles:

1. LowIR / compile oracle
   - the primary student-facing contract
   - positive tests prove successful lowering or successful compilation at the
     owned source subset
   - negative tests prove deterministic rejection

2. Template witness oracle
   - the stricter semantic contract
   - generated from the patched clang witness and stored in the bank
   - checked by comparing `cppgm++ --emit-templates` against the stored witness

In other words:

- LowIR/compile proves "the program compiles or rejects correctly"
- witness matching proves "the compiler made the right template decisions on the
  way there"

The landing shape is:

- `tests/spec` for the assignment-owned slice
- `tests/general` for the non-owned or extra local examples
- adjacent `.ref*` and `.ref.witness` files beside the `tests/spec` sources
- `make test` runs the inventory without checking the oracles
- `make test-strict` checks the adjacent LowIR/compile and witness oracles

This follows `ASSIGNMENT_BUILDOUT_PROCESS.md`: the primary oracle stays at the
first owned boundary, while the stricter witness lane prevents semantic drift
from hiding behind matching LowIR.

## Oracle And Format

Use the existing validation style:

- one self-contained source file per rule,
- `// VALIDATION: run-pass`, `compile-pass`, or `compile-fail`,
- a short `// N3485 focus:` comment naming the governing clause(s),
- ordinary `main`-based runtime checks for positive cases,
- clang as the immediate source-of-truth validator.

The bank should stay intentionally reduction-oriented:

- simple source snippets first,
- no unnecessary library dependence for core rules,
- hosted/STL tests only at the end as integration sentinels.

## Coverage Model

The template bank should cover the whole clause-14 surface we actually care
about for C++11 template usability:

### Group A: `14.1 [temp.param]`

- type, non-type, and template-template parameters
- default template arguments
- redeclaration merge behavior
- parameter-scope / shadowing rules
- variadic parameter packs at declaration sites

### Group B: `14.2 [temp.names]`

- simple-template-id parsing and lookup
- explicit template-id on function and member calls
- injected-class-name and current-specialization naming
- alias template naming and qualified template-ids

### Group C: `14.3 [temp.arg]`

- type arguments
- non-type arguments
- template-template arguments
- argument equivalence and canonicalization edges

### Group D: `14.4 [temp.type]`

- type equivalence across aliases, cv/ref adjustment, function types,
  array/function shapes, and current specialization

### Group E: `14.5 [temp.decls]`

- class templates
- member templates
- variadic templates
- template friends
- class partial specializations
- function template declarations and redeclarations
- alias templates

### Group F: `14.6 [temp.res]`

- local names in templates
- dependent names
- non-dependent names bound at definition time
- dependent resolution at point of instantiation
- friend injection / friend names inside class templates

### Group G: `14.7 [temp.spec]`

- implicit instantiation
- explicit instantiation declarations and definitions
- explicit specialization declarations and definitions
- no-eager-instantiation boundaries where they affect correctness

### Group H: `14.8 [temp.fct.spec]`

- explicit template argument specification
- function template argument deduction
- non-deduced contexts
- function-address deduction
- partial ordering
- overload resolution involving templates and non-templates
- SFINAE / substitution failure / candidate dropping
- conversion-function-template and constructor-template deduction edges

### Group I: Integration

- reduced libc++-style `enable_if` / `void_t` / detector patterns
- reduced trait and forwarding patterns
- hosted header smoke cases that map to the owned language rules
- a final STL-facing compile-pass bucket for confidence, not for first-line
  debugging

## Existing Coverage And Main Gaps

Already present in `validation/tests` and promoted `pa21` / `pa22`:

- alias template substitution
- dependent `typename`
- dependent member template disambiguation
- default template argument merge
- basic class partial specialization selection
- forwarding-reference deduction
- non-deduced context basics
- array/function-reference deduction
- basic partial ordering
- overload-set deduction participation
- some negative deduction and ambiguity cases

Main gaps still worth filling explicitly:

- template-template argument matching
- type-equivalence coverage
- member-template redeclaration and out-of-class definition rules
- template-friend corner cases
- non-dependent name binding at definition time
- point-of-instantiation-sensitive lookup differences
- explicit instantiation declarations / definitions
- explicit specialization ordering and visibility
- constructor-template / conversion-function-template selection
- richer function-template overload-resolution interactions
- SFINAE patterns beyond the current `enable_if` seeds
- reduced hosted patterns for `std::function`, `std::map`, `std::pair`,
  `iterator_traits`, and tuple/forwarding machinery

## Phases

### Phase 1: PA18 First-Tier Templates

Add small non-hosted tests for:

- template-template parameters
- type-equivalence / alias-equivalence cases
- member-template declaration and definition rules
- template friend rules
- non-dependent vs dependent lookup inside templates
- packs, forwarding, and basic function-template deduction
- explicit template-id calls and template-backed operators
- out-of-class nested member definitions inside supported template owners

Target outcome:

- the full `pa18` README-owned subset is covered without relying on STL tests
- no `pa18` behavior depends on NTTPs, explicit specialization, or SFINAE-heavy
  cases that belong later

### Phase 2: PA19 NTTP And Explicit Specialization

Add tests for:

- C++11 integral non-type template parameters and arguments
- integral constant-expression template arguments
- explicit specializations of class, member, function, and static data members
- explicit specialization ordering / visibility constraints
- no-eager-instantiation boundaries where explicit specialization correctness
  depends on them

Target outcome:

- the full `pa19` README-owned extension over PA18 is represented directly in
  the bank
- explicit specialization is no longer treated as "early PA21 material" when
  the README assigns it to PA19

### Phase 3: PA21 Specialization Graph And Ownership

Add tests for:

- alias templates
- variable templates
- class and function partial specialization
- partial ordering and specialization selection
- explicit-instantiation declarations over the supported surface
- constructor/member-template specialization ownership
- the dependent-name / instantiation behavior strictly required to make the
  specialization graph coherent

Target outcome:

- `pa21` can be proved by a declaration/specialization graph oracle rather than
  only by downstream LowIR or STL behavior
- specialization/entity correctness stops depending on the much larger `pa22`
  deduction suite

### Phase 4: PA22 Deduction, Substitution, And SFINAE

Add tests for:

- explicit template arguments mixed with deduced ones
- deduction from conversions, cv/ref collapsing, and pointer/reference shapes
- function-address and overload-set deduction
- richer non-deduced contexts
- constructor-template and conversion-function-template participation
- template/non-template overload interaction
- partial ordering with packs, function types, and pointer/reference patterns
- alias-template SFINAE
- member-typedef probes
- expression-SFINAE through `decltype`
- dependent variable-template gating
- defaulted unnamed non-type enable-if parameters
- detector / `void_t` / `enable_if` composition patterns

Target outcome:

- the full `pa22` README-owned remainder of C++11 template semantics is covered
  without needing hosted sentinels as the first-line oracle

### Phase 5: Hosted And STL Sentinels

Add clang-only integration tests for:

- `std::function` assignment / callable wrapping
- `std::pair` and `std::tuple` forwarding/constructor patterns
- `std::map` emplacement / piecewise construction shape
- `iterator_traits` / alias-heavy dependent lookup
- one or two broader compile-pass STL sentinels

Target outcome:

- late hosted failures should mostly indicate reduction gaps, not unknown
  language gaps

## Test Kinds

Use all three validation kinds deliberately:

- `compile-pass`
  - for lookup, declaration, instantiation, and type-identity rules
- `run-pass`
  - for observable specialization or overload selection
- `compile-fail`
  - for ambiguity, missing disambiguators, forbidden deduction, inaccessible or
    ill-formed specialization usage

Prefer `run-pass` only when the runtime behavior is the cleanest oracle.
Otherwise keep the test compile-only.

## Replacement Strategy

The bank should be partitioned into assignment-owned slices:

- `pa18`: first-tier template surface only
- `pa19`: NTTP + explicit-specialization layer
- `pa21`: specialization/entity graph layer
- `pa22`: deduction/substitution/SFINAE completion

For each slice:

- the primary shipped tests should be bank tests, not the older PA-local
  template regressions
- the LowIR/compile oracle should remain the default student-facing contract
- the witness oracle should live beside the spec tests and be checked by
  `test-strict` via `cppgm++ --emit-templates`
- hosted `090+` sentinels should stay late-stage confidence checks and not be
  used as first-line debugging or grading inputs for the early slices

## Proposed Bank Layout

Create a dedicated template-focused validation bank in:

- `validation/templates/README.md`
- `validation/templates/tests/*.cpp`
- `validation/templates/support.h`
- `validation/templates/run_with_clang.py`

This keeps the existing general validation bank intact while allowing the
template bank to grow much larger without making `validation/README.md`
unwieldy.

## Initial Implementation Order

1. Bank scaffold and clang harness.
2. Phase-1 core template language seeds.
3. Phase-2 specialization / instantiation seeds.
4. Phase-3 deduction / overload seeds.
5. Phase-4 SFINAE / detector seeds.
6. Phase-5 hosted integration sentinels.

## Definition Of Done

The buildout is complete when:

- every major N3485 template subclause from `14.1` through `14.8.3` is mapped
  to one or more concrete tests,
- the bank has both positive and negative coverage for the major deduction and
  specialization rules,
- the bank runs under a clang-based harness,
- the final section contains at least a small hosted/STL sentinel set,
- and the remaining known template failures in our compiler can be described as
  failures against existing bank cases rather than uncategorized template bugs.
