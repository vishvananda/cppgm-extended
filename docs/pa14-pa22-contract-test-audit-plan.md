# PA14-PA22 Contract And Test Audit Plan

## Purpose

PA14 through PA22 are the front-end and LowIR assignments where the language
surface grows fastest. PA14 is the procedural LowIR baseline, PA15-PA17 add the
object model, and PA18-PA22 complete the template/constant-evaluation language
frontier before native backend work starts. The current suites contain tests
whose required features are not always spelled out in the owning README, and
some tests use supporting syntax that may belong to a later PA.

This audit must end with:

- an exact, public assignment contract in each `pa14` through `pa22` README
- every local and course test classified against that contract
- tests moved, reduced, split, or documented so each one lives in the earliest
  PA and first test cluster that own the behavior it is asserting
- automated feature auditors that detect important source-language features and
  flag tests placed before their owning PA/cluster

The current audit helper is `scripts/audit_pa_feature_placement.py`. It scans
test source and adjacent `.ref` output files because some LowIR/codegen
features, such as local static guards or ABI-shaped operations, are easier to
detect in expected output than in source syntax alone.

## Inputs

Use these sources in this order:

1. `ROADMAP.md`
2. `paN/README.md`
3. `pa13/lowir.md` for LowIR presentation and ordering requirements
4. `paN/paN.gram` where present for accepted syntax
5. `paN/tests/**/*.t`
6. `cppgm.tests/course/paN/**/*.t`
7. current `.ref` files only to understand the asserted behavior, not as the
   source of truth for the contract

`cppgm.tests/course` is only an allowed final location for PA1-PA9
maintainer-written course tests. For PA14-PA22 it is a current-state input only:
any `cppgm.tests/course/pa10+` test must be moved into the earliest owning
`paN/tests/` cluster during the audit, with generated `.my*` sidecars dropped.

## End State

For each PA in scope:

- `README.md` has an `Assignment Boundary` that exactly matches the tested
  feature surface.
- `README.md` has an `Out Of Scope` section that does not contradict any
  checked-in test.
- Tests are grouped and numbered by the feature they primarily assert.
- Support-fixture syntax from later PAs is either removed, reduced, or explicitly
  called out as non-owning fixture syntax.
- The tracker records every moved/deleted/split test and every README contract
  change.
- Feature auditors can scan the suite and report likely placement violations.
- The feature table gives a default placement for new tests as
  `(feature id, N3485 reference, earliest PA, first cluster)`.
- No maintainer-written `cppgm.tests/course/pa10+` tests remain, aside from
  empty scaffold files such as `.gitkeep` if the export layout still needs
  them.

## Placement Rule

A test belongs in the earliest PA and first cluster that own the behavior being
asserted.

Supporting syntax does not automatically move a test later if it is incidental
and already accepted by the parser/semantic scaffold, but the README must make
that distinction clear. If the later feature is essential to the expected
result, the test belongs in the later owner or must be reduced to remove that
dependency.

When a test asserts two newly owned features, split it if possible. If splitting
would destroy the value of the test, keep it in the later of the two owners and
record the reason.

Maintainer-written tests after PA9 belong under the owning `paN/tests/` tree,
not under `cppgm.tests/course`. Moving a PA10+ course test is still a feature
placement decision: choose the earliest owning PA and the appropriate local
cluster, then record the move in the tracker.

Use cluster numbers as part of the contract:

- `100`: first isolated tests for an easy or foundational feature
- `200`: ordinary edge cases for one feature, with only already-owned support
- `300`: interactions between the new feature and earlier owned features
- `400`: hard semantic/codegen cases or inherently complex multi-feature tests
- `500+`: stress or regression tests that are not good first examples

The feature taxonomy must record the first cluster where each feature can appear
as the primary assertion. Complex tests using several features should be placed
at least as late as the maximum `(PA, cluster)` required by their primary
features.

For existing tests, `current_cluster` is the numeric filename prefix such as
`100`, `200`, `300`, or `400`; tests without that prefix should be flagged for
manual placement.

For standard C++ language features, the taxonomy must also record the clearest
chapter or subclause reference from `doc/n3485.txt`, using the existing style
such as `14.8.2 [temp.deduct]`. Use `N/A` only for CPPGM LowIR contracts,
implementation extensions, hosted/vendor-only probes, or features without a
clear single N3485 location.

## Feature Ownership Baseline

This is the initial audit baseline. The accepted feature table in the tracker
becomes the canonical ownership source for this audit; README and Roadmap text
should be updated after the table and per-test moves are accepted.

### PA14: Procedural C++ To LowIR

PA14 owns the first C++ source-to-LowIR lowering boundary over the PA12
procedural semantic subset:

- `cppgm++ --emit-lowir -O0`
- valid PA13 LowIR output for the supported procedural subset
- namespace-scope function declarations and definitions in one generated LowIR
  program, including named namespaces
- a required `main` definition
- functions returning integral, pointer, or `bool` results from the supported
  PA12 subset
- supported PA12 procedural parameters and call boundaries
- global integral, pointer, and function-pointer objects with constant
  initializers or zero-initialization
- local scalar objects
- local scalar and function references
- local function pointers and references
- bounded arrays in the supported PA12 procedural type subset
- expression statements
- `return`
- `if` / `else`
- `switch`
- `while`
- `do`
- `for`
- `break` / `continue`
- direct calls to resolved non-template namespace-scope functions
- calls through function pointers and function references in the PA12 subset
- integer literals and `true` / `false`
- id-expressions naming supported locals, globals, and resolved functions
- `sizeof(expr)` and `sizeof(type-id)` when PA12 has resolved them
- unary `+`, `-`, `!`, `~`, `&`, `*`, prefix/postfix `++`, and prefix/postfix
  `--`
- simple assignment to supported lvalues
- built-in arithmetic, bitwise, shift, logical, comparison, conditional, comma,
  and subscript forms from the PA12 procedural subset
- canonical top-level LowIR phase order inherited from `pa13/lowir.md`
- deterministic source-owned function/global order
- direct short-circuit LowIR control flow for `&&` / `||` used directly as
  statement conditions
- accepted scalar immediate canonicalization where the PA14 README permits it

PA14 does not own class/object semantics, synthesized class helper output,
template code generation, exception-aware control flow, floating-point code
generation, string-literal-backed object initialization, or native execution.

Known PA14 contract questions to resolve:

- `pa14/tests/general/200-five-arg-call.t` may contradict the current "up to
  four parameters" README wording
- `cppgm.tests/course/pa14/232-const-ref-converted-float-argument.t` may
  exercise floating conversion support that the README currently places out of
  scope
- local static and array initialization tests need a precise PA14 boundary line
  so they do not blur into PA15 lifetime or PA20 constant-evaluation ownership

### PA15: Basic Object Model

PA15 owns the non-polymorphic class/object layer over the PA14 procedural LowIR
surface:

- namespace-scope and nested-namespace class/struct definitions and forward
  declarations
- nested class/struct declarations when they are needed by the PA15 object model
- complete non-static data-member layout in declaration order
- empty class layout, alignment, padding, and object size
- ordinary integral and enum bit-field layout, including zero-width unnamed
  separators
- self-referential pointer members
- previously completed class-type members
- single non-virtual direct inheritance with the base subobject at offset `0`
- access control for fields and methods in the single-inheritance model
- direct and inherited field lookup
- direct and inherited method lookup
- `this`, implicit member lookup inside methods, `.`, and `->`
- non-static member function calls with explicit LowIR `this` parameter lowering
- ordinary non-template operator overloads over the PA15 object subset,
  including member operators such as `operator[]`
- hidden-friend and namespace-scope non-member operators found through ordinary
  lookup or ADL
- chained reference-returning operators such as `operator<<`
- non-template non-member calls found through associated-namespace lookup /
  hidden-friend ADL when arguments stay inside the PA15 object subset
- in-class member-function definitions
- out-of-class definitions for ordinary non-static member functions when parsed
  as qualified function definitions
- in-class constructors and destructors
- implicit default constructors and destructors when no user-declared one exists
- constructor initializer lists for the single direct base and non-static data
  members
- non-static default member initializers for supported scalar and supported
  class/aggregate subobject construction forms
- constructor execution for local and namespace-scope class objects
- destructor execution at block exit, `return`, loop exit, and program shutdown
- recursive construction/destruction for supported class-type members and bases
- demand-driven LowIR emission of required constructor/destructor helpers
- `@__cppgm_init` / `@__cppgm_fini` for namespace-scope class-object lifetime
- complete class types in `sizeof(type-id)`, `sizeof(expr)`, local object
  declarations, and namespace-scope object declarations

PA15 must not own these unless the audit intentionally changes the contract:

- virtual dispatch, vtables, vpointers, virtual inheritance
- multiple inheritance
- RTTI and `dynamic_cast`
- copy/move value transfer as a primary feature
- pass-by-value / return-by-value of class objects as a primary feature
- eager emission of unused special-member helpers
- template-backed operator overloads

Known PA15 contract questions to resolve:

- bit-field member initializer and access tests may exceed "layout only"
- conversion-operator tests currently appear in PA15 but the README also calls
  broader conversion operators out of scope
- member-pointer tests need an explicit owner decision
- some out-of-class constructor tests may contradict the current PA15 out-of-
  scope text

### PA16: Non-Polymorphic Value Semantics

PA16 owns value transfer for non-polymorphic class objects:

- implicit copy constructors in supported field-wise/base-wise cases
- implicit copy assignment in supported field-wise/base-wise cases
- implicit move constructors in supported field-wise/base-wise cases used by
  the value paths
- implicit move assignment in supported field-wise/base-wise cases used by the
  value paths
- user-declared copy/move constructors in ordinary non-template class cases
- user-declared copy/move assignment operators in ordinary non-template class
  cases
- defaulted/deleted copy/move special members in the supported patterns
- value passing of complete class objects
- return-by-value of complete class objects
- indirect LowIR parameters for supported by-value class parameters
- indirect LowIR return destinations for supported return-by-value class
  results
- return-slot reuse for supported `return local;` cases
- temporary class-object materialization required by copy initialization,
  pass-by-value arguments, and return forwarding
- `const&` / `T&&` binding to materialized class temporaries in the supported
  value paths
- demand-driven LowIR emission of copy/move/value helpers
- direct `copyobj` lowering for supported trivial class value transfers
- direct `copyobj` lowering for supported leading trivial storage prefixes
  inside synthesized copy/move special members
- destructor cleanup for supported materialized temporaries and value-transfer
  locals
- delegating constructors
- out-of-class constructor definitions
- out-of-class destructor definitions
- supported ref-qualified member functions needed by value-semantics tests
- supported `operator new` / `operator delete` call surfaces used by PA16 value
  and cleanup tests

Known PA16 contract questions to resolve:

- lambda, range-for, `decltype`, and template syntax appear in PA16 tests as
  support fixtures and need explicit classification or rehoming
- inheriting constructors are owned by the PA15 class/using-declaration surface
  as a late cluster, so PA16 tests should be rehomed or reduced
- exception-aware cleanup during value transfer is currently out of scope, but
  tests may require narrow cleanup/EH declaration facts

### PA17: Polymorphism

PA17 owns polymorphic class semantics and LowIR lowering:

- virtual member declarations and definitions
- vpointer placement for supported polymorphic objects
- vtable global emission for supported polymorphic classes
- constructor/destructor vpointer writes
- virtual calls through object expressions, pointers, and references to
  polymorphic class type
- direct-call lowering for explicitly qualified virtual calls
- virtual destructors
- override checking for the supported virtual subset
- method-level `final` checking for the supported virtual subset
- inherited virtual member lookup and overriding by supported signature match
- covariant return checks over the supported pointer/reference class subset if
  kept in PA17 tests
- pure virtual declarations as declarations if kept in PA17 tests
- key-function / vtable binding behavior if kept in PA17 tests

Current PA17 tests also exercise a broader multiple-base subset. The audit must
choose one of these outcomes and make README/tests match:

- PA17 explicitly owns the tested limited multiple-direct-base subset, including
  secondary vptrs, secondary vtable views, base-subobject pointer adjustment,
  destructor slot merging, and small adjustor thunks; or
- those tests move to the later PA that owns the needed ABI/object-model
  behavior.

PA17 does not own full virtual inheritance, full RTTI, `dynamic_cast`, full
template-aware virtual dispatch, or general multiple-inheritance ABI behavior
unless the audit changes the contract.

### PA18: Basic Templates

PA18 owns the first usable template instantiation layer:

- class templates with type parameters
- class templates with type parameter packs
- class templates with template-template type parameters
- function templates with type parameters
- function templates with type parameter packs
- function templates with template-template type parameters when supplied
  explicitly
- default template arguments for supported type and template-template parameter
  forms, including defaults referring to earlier parameters
- explicit template-id use for supported class and function templates
- template argument deduction for supported function-template calls from
  ordinary argument types
- on-demand class-template and function-template instantiation
- member templates
- templated member operators
- templated call operators
- out-of-class definitions of nested classes declared inside supported class
  templates when their bodies stay within PA15-PA17
- ordinary PA10 function declarator forms, including trailing return types, on
  supported function templates
- instantiated specialization names participating in ordinary PA15-PA17
  class/method/codegen behavior
- template-backed overload participation where the non-template machinery
  already exists, including function-template operator overloads

PA18 does not own non-type template parameters/arguments, explicit
specialization, partial specialization, full two-phase lookup, SFINAE-heavy
metaprogramming, full `constexpr`, or template-aware virtual dispatch beyond
ordinary instantiated class reuse.

### PA19: First Metaprogramming Layer

PA19 owns the first practical compile-time template value layer:

- integral non-type template parameters
- integral non-type template parameter packs
- explicitly supplied integral non-type function-template arguments
- integral constant-expression template arguments over the supported expression
  subset
- ordinary character literals as integral template arguments
- `true` / `false` as integral template arguments
- id-expressions naming supported constant bindings in template arguments
- unary, binary, conditional, `sizeof...`, `sizeof(type-id)`, `alignof(type-id)`,
  and supported cast expressions that fold to integral template arguments
- explicit specialization of supported class templates
- explicit specialization of supported function templates
- constant-valued template bindings, including class-scope `static const` and
  `static constexpr` members used by lookup, template arguments, or
  `static_assert`
- practical dependent qualified type/value lookup needed by the supported
  metaprogramming subset
- `static_assert` declarations whose condition is in the supported integral
  constant subset, including template-dependent conditions deferred until
  instantiation

PA19 does not own partial specialization, SFINAE-heavy metaprogramming, full
two-phase lookup, constexpr function evaluation, or function-template deduction
of non-type arguments unless the audit changes the contract.

### PA20: Constant Evaluation

PA20 owns full `constexpr` / constant evaluation over the implemented language
surface inherited from PA19:

- reusable constant-expression evaluation for the implemented expression/type
  subset
- `constexpr` free functions
- recursive `constexpr` calls
- `constexpr` calls with default arguments
- `constexpr` constructors
- `constexpr` member functions
- `constexpr` variables and constant initialization
- floating-point constant evaluation over the implemented scalar surface
- scalar, enum, `nullptr`, pointer, reference, object, aggregate, and array
  constant values where the underlying language feature is already implemented
- unary, arithmetic, comparison, bitwise, logical, conditional, cast, `sizeof`,
  `alignof`, `sizeof...`, and `noexcept` constant expressions where those
  operators are already supported
- member access on constant objects
- array and string-literal element access in constant evaluation
- lookup and reuse of previously computed constants, including qualified lookup
  and static data members
- semantic validation of C++11-facing `constexpr` rules over the supported
  language subset

PA20 does not own template-language features deferred to PA21/PA22, post-C++11
constant-evaluation features, or hosted/vendor-only compatibility forms.

### PA21: Template Entities And Specialization Model

PA21 owns the template declaration graph and specialization model:

- alias templates
- variable templates
- class template partial specialization
- function template partial specialization where supported by this course
  model
- partial ordering and specialization selection
- explicit specialization declarations and definitions
- explicit-instantiation declarations and definitions over the supported
  surface, if kept by the audit/roadmap alignment
- extern-template declaration behavior over the supported surface
- collection and ownership behavior for constructor/member-template
  specializations
- deterministic ownership for selected specializations
- dependent-name and instantiation behavior strictly required to make the
  specialization model work

PA21 does not own full function-template deduction over the intended language
surface, SFINAE/substitution-failure completion, no-eager-instantiation timing
that belongs with substitution behavior, hosted/vendor-only extensions, or
post-C++11 template-language features.

Known PA21 contract questions to resolve:

- the README currently emphasizes explicit-instantiation declarations, while
  roadmap/tests also cover emitted explicit-instantiation definitions
- tests with `sfinae` in the name must be audited carefully because SFINAE is
  out of PA21 scope unless the test is only a specialization-graph proxy

### PA22: Deduction, Substitution, And SFINAE Completion

PA22 owns the remaining template-language completion layer before native backend
work:

- full function-template deduction over the intended C++11 subset
- deduction with explicit template arguments mixed with deduced arguments
- function-address and conversion-function template deduction where supported
- constructor-template participation in deduction/overload sets
- non-deduced contexts
- array-bound and conversion-corner deduction cases
- substitution behavior
- substitution-failure candidate dropping
- `enable_if`-style SFINAE
- `void_t` / detector-idiom style SFINAE behavior
- remaining dependent-call behavior
- remaining dependent-alias behavior
- no-eager-instantiation timing needed for full template usability
- instantiated declarations that lower through ordinary LowIR without template
  subset special-casing

PA22 does not own hosted/vendor-only extensions, post-C++11 template-language
features, or backend/toolchain behavior that belongs to PA28 and later.

Known PA22 contract questions to resolve:

- PA18-PA21 tests with SFINAE, no-eager-instantiation, or full deduction
  behavior may need to move here
- PA22 itself may contain tests whose primary assertion is earlier PA18-PA21
  template entity behavior rather than deduction/substitution completion

## Audit Workflow

### Pass 1: Freeze Feature Taxonomy

- Assign every feature above a stable feature id.
- Add ids for support-only syntax, for example `support.lambda`,
  `support.range_for`, and `support.decltype`.
- Record the clearest `doc/n3485.txt` chapter/subclause reference for every
  standard-language feature.
- Record the earliest owning PA for each feature id.
- Record the first cluster where the feature should appear as a primary
  assertion.
- Record whether the feature is `owned`, `support_allowed`, or `out_of_scope`.

### Pass 2: Build Test Manifest

Create a machine-readable manifest with one row per test:

- `path`
- `current_pa`
- `current_cluster`
- `test_role` (`general`, `spec`, `course-current`, or other)
- `expected_exit`
- `primary_asserted_features`
- `support_features`
- `detected_features`
- `n3485_refs`
- `earliest_required_pa`
- `earliest_required_cluster`
- `classification` (`keep`, `move`, `split`, `reduce`, `remove`, `readme_fix`)
- `target_pa`
- `target_cluster`
- `notes`

### Pass 3: Implement Feature Auditors

Create a source scanner that reports feature ids per test. The first version can
be heuristic, but it must be explicit and reviewable.

Required auditor families:

- enum, extended-integer, floating, cast, reference, pointer, array, and
  variadic-call features in the procedural LowIR subset
- class/object model features
- aggregate/list/default initialization and default member initialization
- access control, nested types, static members, friends, ADL, operator
  overloads, `using`, and other lookup features
- allocation, deallocation, placement new, pseudo-destructors, anonymous
  members, unions, attributes, alignment, default arguments, `noexcept`, and
  trailing return syntax
- special members and object lifetime
- value transfer and temporary materialization
- virtual/polymorphic features
- multiple-base and pointer-adjustment features
- class-template, function-template, template parameter/declaration, default
  argument, dependent-name, current-instantiation, and disambiguator features
- non-type template arguments and metaprogramming features
- partial ordering for function templates and specialization selection
- builtin trait/intrinsic support used by template tests
- constexpr/constant-evaluation features
- specialization/explicit-instantiation features
- `initializer_list` and braced-init deduction features
- support-only syntax features used before their primary owner

Each auditor should produce both:

- a per-test JSON/CSV report
- a placement-violation summary: `test path`, `detected feature`, `current PA`,
  `current cluster`, `owning PA`, `first cluster`

### Pass 4: Human Audit Flagged Tests

For every flagged test:

- read the test source
- read the `.ref` only to identify the behavior asserted
- decide whether the later feature is primary or incidental
- decide whether to keep, move, reduce, split, remove, or update README
- record the decision in the tracker

### Pass 5: Update Contracts And Tests

For each PA, update in this order:

1. README exact contract
2. tests moved/reduced/split
3. refs regenerated where required
4. tracker decisions
5. focused validation
6. through-stage validation

### Pass 6: Validate Cross-PA Placement

Run the feature auditors over `pa14` through `pa22` after each batch of moves.
The final report must have no unexplained placement violations.

Run:

```sh
make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22'
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict
```

If strict failures are expected for unrelated known witness drift, record that
explicitly in the tracker.

## Auditor Design Notes

The auditor should start with source-level detection, not semantic execution.
It should be conservative: false positives are acceptable if they are
actionable, but silent false negatives for owned features are not.

Recommended implementation shape:

- `scripts/audit_pa_feature_placement.py`
- feature rules in a data file or a clearly separated table inside the script
- JSON output for automation
- Markdown summary for human review
- optional `--pa pa14 --pa pa22` narrowing
- optional `--feature virtual.multibase` narrowing

Detection can initially combine:

- token/regex checks for obvious syntax
- lightweight brace/comment stripping
- path/name hints as secondary evidence only
- `.ref.exit_status` inspection for positive vs negative tests

Do not classify solely from file names. Names are useful for triage but the
scanner must inspect source text.
