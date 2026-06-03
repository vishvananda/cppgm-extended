# PA14-PA22 Feature Allocation Audit

## Purpose

This audit happens before moving tests. Its job is to decide whether the feature
order itself makes sense, so each assignment has a constrained implementation
surface and later assignments build progressively on earlier ones.

The output of this audit should be an accepted `(feature, owning PA, first
cluster, N3485 reference)` table. Test relocation should wait until that table
is stable.

During this pass, the feature ownership table is the draft canonical source of
truth. Existing README and Roadmap text can provide useful intent signals, but
those files should be updated later to match the accepted table.

The table distinguishes language/semantic ownership from LowIR-test placement.
PA10-PA12 can own syntax and semantic rules, but not `cppgm++ --emit-lowir`
output. LowIR tests using those features must be placed in the first
LowIR-producing PA whose primary behavior requires them, or in the later owner
of the surrounding feature when they are only fixtures. PA28 and PA29 do produce
LowIR, so they can directly own source-to-LowIR tests. PA23 is the opposite: it
owns LowIR-to-native/backend behavior, not C++ source-to-LowIR behavior.

## Allocation Principles

- Each PA should introduce one main conceptual layer.
- A feature should appear as a primary assertion only in the first PA and first
  cluster that own it.
- Later tests may use earlier features as fixtures, but should not require
  unowned features just to prove the current PA.
- A test that needs two newly owned features belongs at the later owner and at
  least the later cluster, unless it can be split.
- Support syntax is allowed only when it is already implemented and is not the
  behavior being tested; otherwise reduce the test or move it later.
- Negative tests should fail for the PA-owned reason, not because an unrelated
  future feature is missing.
- Hosted/vendor compatibility forms should not become assignment requirements
  unless the assignment explicitly owns that standard-language surface.

## Proposed Progressive Spine

| PA | Primary Layer | Keep As Primary Assertions | Defer Or Reduce Out |
| --- | --- | --- | --- |
| `pa14` | Procedural source-to-LowIR lowering | functions, globals, locals, references, pointers, arrays, scalar expressions, control flow, direct/function-pointer calls, enums if PA12 already owns them | runtime floating/variadic backend parity (`pa23`), function-local static guard/init (`pa20`), `__int128`/hosted extensions (`pa34`) |
| `pa15` | Basic non-polymorphic object model | class/struct layout, access, member lookup, static members, single inheritance, constructors/destructors, basic lifetime, supported default member init, focused friend/ADL/operator overloads, aggregate init over the PA15 subset, inheriting constructors as a late class/using-declaration integration case | member pointers (`pa28`), conversion operators (`pa16` hard case), multiple inheritance (`pa28`), `new`/`delete` (`pa16`), `[[no_unique_address]]`/vendor attributes (`pa34`), auto-return/lambda/range closure (`pa26`) |
| `pa16` | Value semantics | copy/move construction and assignment, by-value params/returns, temporary materialization, ref-qualified value paths, delegating/out-of-class ctors/dtors, scalar allocation as a late lifetime case, non-template conversion operators as a late value case | lookup-only using-directive cases (`pa12`), lambda/range-for/auto (`pa26`), multiple inheritance/member pointers (`pa28`), template-aware value semantics |
| `pa17` | Single-inheritance virtual dispatch | virtual calls, vtables, vptr writes, virtual destructors, `override`/method `final`, deterministic vtable/destructor slot order for the supported single-inheritance model | multiple inheritance (`pa28`), non-primary polymorphic pointer adjustment/multi-vptr dispatch (`pa29`), virtual inheritance, pure-virtual/abstract enforcement unless explicitly added later |
| `pa18` | Basic templates | class/function templates, type parameters, type/template defaults, basic function-template deduction, member templates, template friends, packs, template-template parameters, dependent names needed for those basics | NTTPs (`pa19`), explicit specialization (`pa19`), partial specialization/entity graph (`pa21`), full function-template partial ordering/deduction/SFINAE/no-eager timing (`pa22`), full constexpr (`pa20`) |
| `pa19` | First metaprogramming layer | integral NTTPs, integral constant-expression template arguments, `static_assert`, explicit specialization, class-scope constant bindings, explicit-specialization timing that does not require SFINAE/no-eager failure behavior | alias/variable templates (`pa21`), partial specialization (`pa21`), broad constexpr function evaluation (`pa20`), non-integral/pointer/member NTTPs (`pa22` plus dependent feature owners) |
| `pa20` | Full constant evaluation | constexpr functions/constructors/member functions/variables over the implemented language, object/reference/pointer/floating constant values, constexpr validation, constant-initialized local statics, dynamic guarded class local statics as a late local-static/object-lifetime integration case | new template entity features (`pa21`), SFINAE/deduction completion (`pa22`), hosted/vendor builtin probes (`pa34`) |
| `pa21` | Template entity and specialization graph | alias templates, variable templates, partial specialization, specialization selection/order, explicit instantiation, current-specialization/entity ownership | full function-template deduction/partial ordering, SFINAE/substitution failure, no-eager dependent-call timing, detector idioms (`pa22`) |
| `pa22` | Template completion | full deduction, function-template partial ordering, non-deduced contexts, substitution/candidate dropping, non-integral NTTPs, `enable_if`/`void_t`/detector idioms, remaining dependent-call/alias/no-eager timing | initializer-list language/library interop (`pa27`), member-pointer NTTP cases until member pointers are owned, hosted/vendor-only extensions (`pa34`) |

## N3485 Reference Review

The feature list in
[pa14-pa22-contract-test-audit-tracker.md](/home/vishvananda/cppgm/docs/pa14-pa22-contract-test-audit-tracker.md)
now records the clearest `doc/n3485.txt` clause for each standard-language
feature where one exists. The spec references are useful for review, but they
should not force the assignments to follow chapter order. The assignment order
should follow implementation dependencies:

- PA14 lowers the already-owned procedural language subset, even though those
  features are spread across N3485 clauses 3-8.
- PA15-PA17 build the class/object model before templates, even though overload
  and conversion clauses appear outside the class clauses.
- PA18-PA22 split N3485 clause 14 across multiple assignments because templates
  are too large to assign as one unit.

Initial ordering review after adding N3485 references:

| Area | Progressive? | Reason |
| --- | --- | --- |
| PA14 procedural references | mostly yes | Statements, references, arrays, casts, and enums are already prerequisite language for source-to-LowIR. Runtime floating, local statics, and `__int128` remain questionable because they expand the scalar/runtime surface. |
| PA15 object references | mixed | Class layout, access, members, friends, ADL, and basic operators fit the canonical PA15 object-model core. Member pointers, conversion operators, `new/delete`, `no_unique_address`, and broad init corners are separate integration axes. |
| PA16 value references | mostly yes | Copy/move, temporaries, ref-qualified members, and delegating constructors build on PA15. Using-directive, lambda/range-for, union, and allocation tests should be kept only if they are fixtures for value semantics. |
| PA17 virtual references | no for multi-base cases | 10.3 `[class.virtual]` fits PA17; 10.1 `[class.mi]` and non-primary pointer adjustment do not fit a single-inheritance virtual milestone. |
| PA18 template references | mixed | 14.1-14.5 basic template declarations fit PA18. 14.7 specialization, 14.8 SFINAE/deduction tails, and partial-specialization-shaped fixtures are not progressive here. |
| PA19 metaprogramming references | mixed | 14.3.2 NTTPs, 5.19 integral constants, `static_assert`, and 14.7.3 explicit specialization fit PA19. Alias templates, partial specialization, and SFINAE are not progressive in PA19. |
| PA20 constexpr references | yes if fixtures stay earlier-owned | 7.1.5 `[dcl.constexpr]` and 5.19 `[expr.const]` naturally depend on PA19 constants and feed PA21/PA22 templates. |
| PA21 specialization references | mostly yes | 14.5.5, 14.5.7, and 14.7.2 form a coherent entity/specialization milestone. Substitution failure and no-eager timing should stay PA22. |
| PA22 completion references | yes | 14.8.2/14.8.3 deduction/substitution and detector idioms are the right final template-language layer. |

Net result: the high-level PA14-PA22 order is sound once the cross-cutting
features are assigned to their real owners. The feature table now resolves the
draft owners and clusters; the per-test audit should use that table before any
test movement.

## Implementation Coupling Review

This pass checked where each feature is implemented in `dev/src/` and evaluated
the shape a student would need to build. A PA is risky when one apparently small
feature requires simultaneous work in distant layers:

- parse/model surface: AST nodes, type nodes, declaration records
- semantic core: expression analysis, declaration analysis, lookup, overloads
- lifetime/output requirements: constructors, destructors, cleanup, witness
- template engine: substitution, instantiation, specialization, deduction
- LowIR/backend/ABI: lowering, symbol/linkage/mangling, runtime helpers

The exact files in this compiler are not requirements for students, but they are
a useful dependency map. Features that touch three or more distant layers should
usually be late-cluster hard cases or move to a later PA unless that PA is
explicitly the integration milestone for those layers.

| Feature Area | Main Implementation Surface In This Compiler | Student-Ordering Risk | Placement Guidance |
| --- | --- | --- | --- |
| PA14 procedural integer/pointer/reference LowIR | `semantic_expression.*`, `semantic_statement.*`, `lowirgensemantic.*`, `lowir_internal.*` | Coherent: semantic analysis already owns the language shape and PA14 adds lowering. | Keep as PA14 core. |
| PA14 runtime floating lowering | `posttokenizer.*`, `cpp_decl_model.*`, `semantic_expression.*`, `semantic_conversion.*`, `semantic_builtins.*`, `constant_value.*`, `lowir_*`, `host_builtin_runtime.*` | Too broad for a procedural LowIR milestone if tests require runtime ABI/backend behavior rather than simple scalar typing. | Canonical owner: PA28 native/backend coverage when runtime/backend parity is required. |
| PA14 local statics/string-backed initialization | `semantic_statement.*`, `semantic_lifetime.*`, `callsemantic.*`, `lowirgensemantic.*`, object/backend support | Pulls initialization guards, storage duration, cleanup, and lowering into one early feature. | Canonical owner: `pa20` cluster `300` for constant-initialized local statics and `pa20` cluster `400` for dynamic guarded class local statics. |
| PA14 `__int128` | `cpp_decl_model.*`, `constant_value.*`, `semantic_builtins.*`, `semantic_expression.*`, `semantic_conversion.*`, `lowirgensemantic.*`, machine/object backends | Compiler extension plus constant folding and backend width support. | Canonical owner: `pa34` cluster `600` as hosted/vendor compatibility. |
| PA15 basic class object model | `cppast_parser.*`, `cpp_decl_model.*`, `semantic_class_model.*`, `semantic_lookup.*`, `semantic_lifetime.*`, `lowirgensemantic.*` | Broad but conceptually adjacent: declarations, layout, member lookup, simple lifetime, lowering. | Keep PA15 constrained to this base object model. |
| PA15 friend/ADL/operators | `semantic_lookup.*`, `semantic_overload.*`, `semantic_class_model.*`, `callsemantic.*` | Crosses class ownership, lookup, and overload, but it is still adjacent to object-model usability when kept non-template and non-value-heavy. | Keep in PA15 only as focused operator/ADL cases; avoid coupling to templates or advanced conversion machinery. |
| PA15 bit-field access/init | `cppast_parser.*`, `semantic_class_model.*`, `semantic_expression.*`, `semantic_lifetime.*`, `lowirgensemantic.*` | Layout alone is local, but read/write/init lowering crosses many layers. | Split layout/declaration from access/init; access/init should be late PA15 or later if it blocks object-model progress. |
| PA15 member pointers | `cpp_decl_model.*`, `semantic_conversion.*`, `semantic_expression.*`, `semantic_overload.*`, `template_argument_semantics.*`, `lowirgensemantic.*`, `symbol_linkage.*`, mangling | Very high coupling: type system, conversions, calls, overloads, NTTP-like constants, ABI representation. | Canonical owner: `pa28` cluster `300` over the completed non-virtual object model. |
| PA15 conversion operators | `cppast_parser.*`, `semantic_conversion.*`, `semantic_overload.*`, `callsemantic.*`, template/constexpr paths | Forces user-defined conversion selection across overload and later constant/template cases. | Canonical owner: `pa16` cluster `400` for non-template conversion operators. |
| PA15 `new`/`delete` | `semantic_expression.*`, `semantic_builtins.*`, `semantic_lifetime.*`, `callsemantic.*`, runtime symbols | Allocation syntax immediately combines expression semantics, ctor/dtor lifetime, cleanup, and runtime naming. | Canonical owner: `pa16` cluster `300` for scalar allocation and cluster `400` for array allocation. |
| PA15 `[[no_unique_address]]` | `cppast_parser.*`, `semantic_class_model.*`, trait/type analysis | Post-C++11 layout feature, not needed for the basic class arc. | Canonical owner: `pa34` cluster `600`. |
| PA16 copy/move/value semantics | `semantic_class_model.*`, `semantic_lifetime.*`, `semantic_overload.*`, `callsemantic.*`, `lowirgensemantic.*` | Broad but coherent: PA16 is the value/lifetime integration point. | Keep as PA16 core, with tests ordered from simple transfer to cleanup-heavy cases. |
| PA16 using directives | `typesemantic.*`, `semantic_scope_mutation.*`, `semantic_lookup.*`, `callsemantic.*`, template lookup paths | Pure lookup feature; not naturally part of value semantics. | Canonical owner: `pa12` cluster `200`. |
| PA16 unions | `semantic_class_model.*`, `semantic_lifetime.*`, `semantic_builtins.*`, `lowirgensemantic.*`, template paths | Looks like object layout, but special members/storage-copy behavior is its own hard lifetime axis. | Keep only as late PA16 hard value cases, not early PA16 requirements. |
| PA17 single-inheritance virtuals | `semantic_class_model.*`, `semantic_overload.*`, `semantic_lifetime.*`, `callsemantic.*`, `lowirgensemantic.*` | Large but coherent: vtable order, vptr writes, virtual call lowering, virtual destructor cleanup. | Keep PA17 focused on single-inheritance virtual dispatch. |
| PA17 multi-base/non-primary adjustment | `semantic_class_model.*`, `semantic_output.*`, `symbol_linkage.*`, `lowirgensemantic.*` | Pulls multiple inheritance, base-offset adjustment, thunks, and ABI lowering into the virtual PA. | Canonical owners: `pa28` for non-virtual multiple inheritance and `pa29` for polymorphic non-primary adjustment. |
| PA18 basic templates | `template_angle_parser.*`, `cpp_decl_ast.*`, `callsemantic/template_declaration_collector.*`, `template_argument_semantics.*`, `template_instantiation.*`, `template_resolution.*` | Coherent only if limited to declarations, type parameters, basic instantiation, and simple deduction. | Keep PA18 basic; avoid specialization and SFINAE fixtures. |
| PA18 dependent lookup | `semantic_lookup.*`, `template_argument_semantics.*`, `template_resolution.*`, `template_instantiation.*`, `symbol_linkage.*` | Easy to over-expand into two-phase lookup and no-eager timing. | Allow only dependent-name behavior required by PA18 basics; defer full timing and failure behavior. |
| PA18 specialization/SFINAE-shaped tests | `template_specialization.*`, `template_selection.*`, `template_instantiation.*`, `semantic_template_function.*`, `template_kernel.*`, `semantic_overload.*`, `symbol_linkage.*` | This is a different subsystem from basic templates and can dominate the implementation. | Move explicit specialization to PA19, partial/entity specialization to PA21, and SFINAE to PA22. |
| PA19 NTTP/static assertions | `template_argument_semantics.*`, `semantic_declaration.*`, `semantic_consteval.*`, `constexpr_eval.*`, `constant_value.*` | Coherent if restricted to integral constants; it naturally seeds constexpr. | Keep PA19 integral-only unless the canonical feature table explicitly expands the constant domain. |
| PA20 constexpr | `semantic_consteval.*`, `constexpr_eval.*`, `constant_value.*`, `semantic_expression.*`, `callsemantic/constant_value_lookup.*` | Coherent as a dedicated evaluator milestone, but should not import new template entity features. | Keep constexpr fixtures to PA19-owned templates and earlier class/value features. |
| PA21 template entities/specialization graph | `template_specialization.*`, `template_selection.*`, `template_instantiation.*`, `template_resolution.*`, `callsemantic/template_declaration_collector.*` | Coherent if it owns alias/variable templates, partial specialization, explicit instantiation, and selection order. | Keep PA21 entity/specialization focused; do not also require SFINAE candidate dropping. |
| PA22 deduction/SFINAE completion | `semantic_template_function.*`, `template_kernel.*`, `template_resolution.*`, `template_specialization.*`, `template_argument_semantics.*`, `semantic_overload.*`, `symbol_linkage.*` | Broad, but it is the natural final template integration layer. | Keep PA22 as the owner for substitution failure, detector idioms, non-deduced contexts, and no-eager timing. |

Implementation-coupling conclusions:

1. PA14 should not silently own runtime/ABI scalar extensions. Floating runtime
   lowering moves to `pa23`, guarded local statics move to `pa20`, and
   `__int128` moves to `pa34`.
2. PA15 should stay about the first class/object model. Member pointers,
   conversion operators, allocation, post-C++11 layout attributes, and bit-field
   read/write/init are separate integration axes with their own later or late
   clusters.
3. PA16 should stay about value transfer and cleanup. Lookup-only features and
   unrelated syntax should not be primary PA16 assertions.
4. PA17 should stay single-inheritance virtual dispatch. Multiple inheritance
   belongs to `pa28`; polymorphic non-primary adjustment belongs to `pa29`.
5. PA18 must be protected from PA21/PA22 work. Specialization and SFINAE are
   implemented by different machinery than basic template declarations and
   instantiation, and full function-template partial ordering belongs to PA22.
6. PA19 should be the small bridge from templates to integral constant
   metaprogramming. Broad constexpr, non-integral NTTPs, and partial
   specialization should stay out.
7. PA20-PA22 are already ordered well if PA20 does not depend on PA21/PA22
   template features and PA21 does not absorb PA22 substitution failure.

## Current Allocation Concerns

These are triage signals for the per-test audit. The ownership table above is
now the draft decision source; individual tests still need reduction/move
decisions before files are changed.

The initial scan found these high-signal early-feature patterns:

| Pattern | Earlier-Than-Owner Hits | Main Concern |
| --- | ---: | --- |
| explicit specialization before PA19 | 10 | PA18 tests may be proving specialization, not basic templates. |
| partial-specialization-shaped tests before PA21 | 39 | PA18/PA19 may be taking on PA21 specialization work. |
| `enable_if` / `void_t` / detector names before PA22 | 23 | PA18/PA19/PA21 may be taking on SFINAE/substitution work. |
| PA17 multi-base/non-primary-base virtual tests | 6 named tests | Canonical PA17 is single-inheritance virtual dispatch; multi-base/polymorphic adjustment belongs to PA28/PA29. |
| PA14 runtime floating tests | 3 named tests plus incidental float uses | Canonical PA14 is procedural integer/pointer/reference LowIR; runtime floating/backend parity belongs to PA28. |

Counts are triage signals, not final classifications; each flagged test still
needs a source-level decision after the ownership table is accepted.

### PA14

PA14 is supposed to be procedural LowIR over the PA12 semantic subset. The
current tests include features that look beyond that boundary:

- runtime floating conversion/promotion tests:
  - `pa14/tests/general/200-floating-return-integral-conversion.t`
  - `pa14/tests/general/200-variadic-float-argument-promotes-to-double.t`
  - `cppgm.tests/course/pa14/232-const-ref-converted-float-argument.t`
- string-literal-backed local static array initialization:
  - `pa14/tests/general/200-function-local-static-array-guard.t`
- `inline constexpr` / 128-bit scalar fixtures:
  - `pa14/tests/general/200-unused-inline-u128-global.t`
  - `pa14/tests/general/200-u128-constant-runtime-init-not-truncated.t`

Recommendation: keep PA14 constrained to procedural integer/pointer/reference
LowIR unless we explicitly decide that runtime floating and extended integer
scalars are part of the PA14 scalar contract. Local-static guard/string-literal
cases should not be primary PA14 features.

### PA15

PA15 owns the broad object-model base. The risky part is that PA15 currently
also contains tests for features that canonical ownership now assigns to later
or harder clusters:

- member pointers
- conversion operators
- `new`/placement-new
- `[[no_unique_address]]`
- bit-field access and initializer lowering
- broad aggregate/list-initialization corners
- trailing return syntax as a primary assertion

Recommendation: keep class layout/access/member lookup/static members/basic
operators/friend ADL in PA15. Treat bit-field access/init as PA15 cluster `400`;
move `new`/`delete` and non-template conversion operators to PA16; move member
pointers and multiple inheritance to PA28; move hosted attributes to PA34.

### PA16

PA16 has a coherent core: copy/move, by-value transfer, temporaries, cleanup,
delegating constructors, and out-of-class ctor/dtor definitions. Some tests
look like they are really lookup or later syntax tests:

- using-directive/imported lookup behavior
- lambda and range-for syntax
- array new/delete and global operator new/delete
- union special-member/storage-copy corners

Recommendation: keep ref-qualified members and simple operator assignment if
they directly exercise value transfer. Reduce or defer tests whose primary
assertion is lookup syntax or allocation machinery rather than value semantics.
Union support can remain only if it is explicitly framed as a hard value-copy
storage case.

### PA17

PA17 is clearly framed as single-inheritance virtual dispatch. Several tests
exercise multi-base/non-primary-base behavior:

- `pa17/tests/general/400-diamond-virtual-destructor-slot-merge.t`
- `pa17/tests/general/400-multibase-implicit-virtual-destructor-slot-merge.t`
- `pa17/tests/general/400-nonprimary-direct-base-ctor-vtable-offset.t`
- `pa17/tests/general/400-secondary-primary-base-vptr-overwrite.t`
- `pa17/tests/general/400-static-reference-downcast-nonprimary-base.t`
- `cppgm.tests/course/pa17/413-static-pointer-downcast-nonprimary-base.t`

Recommendation: do not make multiple inheritance a PA17 requirement.
Single-inheritance vtable/destructor slot ordering can be PA17 `400`;
non-primary base pointer adjustment should move to PA28/PA29 depending on
whether the primary assertion is non-virtual object layout or polymorphic
dispatch adjustment.

### PA18

PA18 should be basic templates. The corpus includes explicit/partial
specialization and SFINAE-style helpers before their owners. Examples:

- `pa18/tests/general/100-dependent-enable-if-return-less-equal.t`
- `pa18/tests/general/100-dependent-enable-if-return-sizeof-less.t`
- `pa18/tests/spec/100-out-of-class-sfinae-member-template-body.t`
- `pa18/tests/general/200-explicit-function-specialization-constexpr-linkage.t`

Recommendation: PA18 should not require explicit specialization, partial
specialization, `enable_if`, `void_t`, or no-eager-instantiation behavior. If a
test's true point is basic template instantiation or deduction, reduce out the
future feature. Otherwise move it to PA19, PA21, or PA22 after the taxonomy is
accepted.

### PA19

PA19's clean role is integral NTTPs, integral constant expressions,
`static_assert`, and explicit specialization. Current tests include several
SFINAE/partial-specialization/alias-template shapes that are likely too broad:

- `pa19/tests/general/100-defaulted-enable-if-overload-drop.t`
- `pa19/tests/general/100-dependent-void-t-sfinae-alias.t`
- `pa19/tests/general/100-bound-reference-enable-if-nontype-parameter.t`

Recommendation: keep `constexpr` only as the integral constant-binding subset
needed for NTTP/static_assert. Do not make alias templates, partial
specialization, or SFINAE primary PA19 requirements.

### PA20

PA20 is well placed as the constant-evaluation layer. The main risk is not PA20
itself, but tests that use PA21/PA22 template machinery to exercise constexpr.

Recommendation: PA20 tests should use only PA19-owned template machinery unless
the primary assertion is explicitly a constexpr interaction with an already
owned feature. Hosted/vendor trait probes should not become PA20 requirements.

### PA21

PA21 should stabilize template entities and specialization selection. It should
not finish substitution failure. Tests with `sfinae`, `enable_if`, detector
idioms, or no-eager dependent-call behavior should be scrutinized:

- `pa21/tests/general/400-array-qualified-member-type-sfinae.t`
- `pa21/tests/general/400-dependent-alias-helper-partial-specialization.t`
- `pa21/tests/general/400-dependent-bool-base-trait-type-argument.t`

Recommendation: keep alias templates, variable templates, partial
specialization, explicit instantiation, and specialization ordering here. Move
or reduce tests whose expected result depends on substitution-failure candidate
dropping or delayed instantiation timing.

### PA22

PA22 is the correct owner for SFINAE, full deduction, non-deduced contexts,
detector idioms, and no-eager dependent timing. It also contains many tests that
may only prove earlier entity behavior after reduction.

Recommendation: once PA18-PA21 are cleaned up, audit PA22 in reverse. Tests
whose primary assertion is only a basic template, explicit specialization,
alias/variable template, or partial specialization behavior should move earlier
only if they can be reduced without losing the assertion.

## Draft Ownership Moves

These moves are now reflected in the feature table and should drive the
per-test audit:

1. Runtime floating/variadic backend parity moves out of PA14 to `pa23` cluster
   `500` as LowIR/backend coverage. Source-to-LowIR floating or variadic tests
   need a separate LowIR-producing owner if the audit decides to keep them.
2. Function-local static guard/init moves out of PA14 to `pa20` cluster `300`;
   dynamic guarded class local statics use `pa20` cluster `400`.
3. GNU/hosted extensions such as `__int128`, `[[no_unique_address]]`, generic
   attribute compatibility, and builtin traits move to `pa34`.
4. Bit-field access/init stays in PA15 but becomes cluster `400`; layout-only
   bit-field declarations are cluster `300`.
5. Scalar `new`/`delete` moves to `pa16` cluster `300`; array allocation is
   `pa16` cluster `400`.
6. Non-template conversion operators move to `pa16` cluster `400`.
7. Multiple inheritance moves to `pa28`; polymorphic non-primary adjustment
   moves to `pa29`.
8. Lookup/declarator support that is not object/value work moves to earlier
   owners: default arguments to `pa12`, using-directive value lookup to `pa12`,
   `noexcept`/trailing-return/`decltype` type support to `pa11`.
9. General `auto`, range-for, and captureless lambda semantics move to `pa26`;
   capturing lambda/closure-object cases use `pa26` cluster `300`, and
   initializer-list interop moves to `pa27`.
10. Function-template partial ordering and non-integral NTTP completion move to
    `pa22`; detector/SFINAE idioms remain `pa22` cluster `300`.

## Next Audit Step

Before moving tests, rerun the feature scan against the updated ownership table
and generate a per-test move/reduction list. Each moved test still needs a
decision-log entry that records its primary assertion and target owner.
