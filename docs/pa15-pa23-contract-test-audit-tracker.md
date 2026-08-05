# PA15-PA23 Contract And Test Audit Tracker

This tracker implements
[pa15-pa23-contract-test-audit-plan.md](/home/vishvananda/cppgm/docs/pa15-pa23-contract-test-audit-plan.md).

## Status Key

- `pending`: not started
- `in_progress`: active audit work underway
- `drafted`: initial draft exists, not validated
- `audited`: human audit complete
- `updated`: README/tests updated
- `validated`: required validation passed

## Overall Status

| Area | Status | Notes |
| --- | --- | --- |
| Feature allocation audit | drafted | See `docs/pa15-pa23-feature-allocation-audit.md`; draft canonical owners/clusters are filled and should drive the per-test audit. |
| Feature taxonomy | drafted | Draft owners, first clusters, and N3485 references are filled. Needs acceptance after scanner/test validation. |
| Test manifest | drafted | Post-move inventory refreshed below. Needs final per-test decisions after the remaining split/reduce pass. |
| Feature auditors | drafted | `scripts/audit_pa_feature_placement.py` scans source plus adjacent `.ref` output. It is a triage tool; human placement decisions still need per-test review. |
| README contract updates | pending | Must happen after test audit decisions. |
| Test moves/reductions | in_progress | Mechanical move batches completed. Split/reduce and PA29/PA34 harness-rewrite items are deferred below. |
| Final validation | pending | Run PA15-PA23 report and strict as described in plan. |

## Test Inventory

Current source-test counts:

| PA | Local tests | Course tests | Total |
| --- | ---: | ---: | ---: |
| `pa15` | 48 | 1 | 49 |
| `pa16` | 128 | 0 | 128 |
| `pa17` | 123 | 0 | 123 |
| `pa18` | 21 | 0 | 21 |
| `pa19` | 151 | 0 | 151 |
| `pa20` | 97 | 0 | 97 |
| `pa21` | 66 | 0 | 66 |
| `pa22` | 161 | 0 | 161 |
| `pa23` | 390 | 0 | 390 |
| **total** | **1,185** | **1** | **1,186** |

## Per-PA Audit Status

| PA | Contract Audit | Test Audit | Auditor Coverage | README Update | Test Moves | Validation | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `pa15` | pending | pending | drafted | pending | in_progress | pending | Mechanical local-static/class-owner moves done. Extended-integer cases moved to PA34. Source-to-LowIR floating and variadic lowering are settled as PA15-owned; backend/runtime parity remains PA29-owned. |
| `pa16` | pending | pending | drafted | pending | in_progress | pending | Mechanical inheritance/operator/bitfield/conversion/member-pointer moves done. No-unique-address hosted cases moved to PA34. |
| `pa17` | pending | pending | drafted | pending | in_progress | pending | Mechanical value/allocation/conversion/lambda/range-for/inheriting-constructor moves done. `using` overload split remains. |
| `pa18` | pending | pending | drafted | pending | in_progress | pending | PA18 course tests moved out of `cppgm.tests/course`; multi-base pointer-adjustment cases moved to PA30. |
| `pa19` | pending | pending | drafted | pending | in_progress | pending | Mechanical dependent-name/pack/template-deduction moves done. Hosted builtin-trait case moved to PA34. Inheriting-constructor template reduction and one owner recheck remain. |
| `pa20` | pending | pending | drafted | pending | in_progress | pending | Mechanical explicit-specialization, timing, alias/variable-template, SFINAE, and member-pointer moves done. Hosted builtin-trait cases moved to PA34. |
| `pa21` | pending | pending | drafted | pending | in_progress | pending | Mechanical constexpr object/static-member renumbers done. Extended-integer hosted case moved to PA34. Noexcept/template split remains. |
| `pa22` | pending | pending | drafted | pending | in_progress | pending | Mechanical variable-template, explicit-instantiation, partial-ordering, SFINAE, and member-pointer moves done. Hosted attribute cases moved to PA34; one nodebug-attribute fixture reduced. |
| `pa23` | pending | pending | drafted | pending | in_progress | pending | Mechanical PA23 cluster renumbers and PA27/PA28/PA29 support-feature moves done. Hosted builtin/attribute cases moved to PA34; declval body sentinels reduced. Auto-local reductions remain. |

## Move Results Log

Mechanical moves used `git mv` for each `.t` file and tracked adjacent `.ref*`
sidecars. Generated `.my*` files and local logs were intentionally not moved.
Witness refs are kept only when the destination remains in the strict PA set
(`pa19`, `pa20`, `pa22`, `pa23`); strict-to-non-strict moves drop witness
sidecars.

| Batch | Scope | Tests moved | Tracked files changed | Result |
| --- | --- | ---: | ---: | --- |
| 1 | PA15-PA18 plus first PA21/PA27/PA29/PA30 support moves | 97 | 386 | Completed. A sidecar-rotation bug in the first script was repaired immediately; every moved file was byte-checked against its original content. |
| 2 | PA19 and PA20 renumbers/moves through PA21/PA23/PA27-PA30 destinations | 187 | 921 | Completed with pre-move collision checks and post-move byte verification. |
| 3 | PA21-PA23 renumbers/moves through PA19/PA20/PA27-PA29 destinations | 155 | 770 | Completed with pre-move collision checks and post-move byte verification. |
| 4 | Hosted compatibility moves to PA34 plus sentinel reductions in PA22/PA23 | 25 | 218 | Completed. Original hosted builtin/attribute/extended-integer/no-unique-address tests moved to PA34 compile coverage; three PA23 no-body sentinel tests and one PA22 alias test were reduced in place. |
| 5 | Strict-to-non-strict witness cleanup | 0 | 9 | Removed stale `.ref.witness` sidecars from PA21, PA27, and PA29 destinations. No non-strict tests moved into the strict PA set. |
| **total** |  | **464** | **2,304** |  |

Post-move scanner refresh over PA15-PA23 plus PA27-PA30:

- tests scanned: 1,377
- tests needing review: 86
- placement findings: 90
- semantic-owner notes: 222
- detector gaps: 0
- placement violations: 0
- cluster-early findings: 0

Move validation:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa15 pa16 pa17 pa18 pa19 pa20 pa21 pa22 pa23 pa27 pa28 pa29 pa30' ORDERED=false`
  passed: 1,403 / 1,403.
- `make test-report ACTIVE_TEST_REPORT_PAS='pa34' ORDERED=false`
  passed: 291 / 291.
- `make test-report ACTIVE_TEST_REPORT_PAS='pa15 pa16 pa19 pa20 pa21 pa22 pa23' ORDERED=false`
  passed: 1,042 / 1,042.

Remaining PA10+ course placement:

- None. Maintainer-written PA10+ course tests have been moved into local PA
  test directories; PA1-PA9 remain the course-test boundary.

Deferred split/rewrite items:

- PA17 reduction: `pa17/tests/general/300-using-base-overload-set.t`.
- PA19 owner recheck/reduction:
  `pa19/tests/general/100-inherited-constructor-using-alias-template.t` and
  `pa19/tests/spec/100-member-template-cache-hit-concrete-scope.t`.
- PA21 split:
  `pa21/tests/general/400-constexpr-noexcept-decltype-static-assert.t`.
- PA22 split/reduce: top-level function ordering, pack-expanded base multiple
  inheritance, and attribute-on-specialization variants.
- PA23 split/reduce: auto-local dependent-result tests and builtin-trait-heavy
  SFINAE/deduction variants.

## Known Preliminary Findings

These are the first items the individual test audit must classify against the
draft canonical feature table.

### Post-PA9 Course Tests

Only PA1-PA9 should use `cppgm.tests/course` for maintainer-written tests. The
PA10+ course-test placement cleanup is complete; current PA10+ regression tests
live under local `paN/tests/` owners.

### PA15

Potential ownership mismatches:

- `pa15/tests/general/200-five-arg-call.t`
- `pa15/tests/general/200-function-local-static-array-guard.t`

Audit action:

- Keep true PA15 procedural LowIR cases, including source-to-LowIR floating
  scalar conversions and C-style variadic argument lowering. Classify native
  runtime/backend parity for those operations as PA29 and local-static
  guard/init as PA21 unless a test can be reduced to an earlier-owned assertion.

### PA16

Potential ownership mismatches:

- `pa16/tests/general/200-bit-field-constructor-member-init.t`
- `pa16/tests/general/200-bit-field-sparse-member-init.t`
- `pa16/tests/general/200-bitfield-aggregate-init.t`
- `pa16/tests/general/300-class-pointer-conversion-builtin-eq.t`
- `pa16/tests/general/300-user-defined-conversion-second-rank.t`
- `pa16/tests/spec/300-inherited-conversion-operator-parameter-binding.t`
- `pa16/tests/general/200-nested-out-of-class-constructor-enclosing-type.t`

Audit action:

- Keep PA16 object-model assertions. Classify bit-field access/init as PA16
  cluster `400`, conversion operators as PA17, and member pointers as PA27
  unless a test can be reduced to an earlier-owned assertion.

### PA17

Potential support-syntax tests needing classification:

- `pa17/tests/spec/300-lambda-capture-mutable.t`
- `pa17/tests/spec/300-range-for-member-begin-end.t`
- `pa17/tests/general/300-inheriting-constructors.t`

Audit action:

- Keep value-semantics assertions in PA17. Rehome or reduce tests whose primary
  assertion is lambda/range-for/auto or unrelated support syntax.

### PA18

Potential contract conflicts:

- `pa18/tests/general/400-diamond-virtual-destructor-slot-merge.t`
- `pa18/tests/general/400-multibase-implicit-virtual-destructor-slot-merge.t`
- `pa18/tests/general/400-nonprimary-direct-base-ctor-vtable-offset.t`
- `pa18/tests/general/400-secondary-primary-base-vptr-overwrite.t`
- `pa18/tests/general/400-static-reference-downcast-nonprimary-base.t`
- `cppgm.tests/course/pa18/413-static-pointer-downcast-nonprimary-base.t`

Audit action:

- Keep single-inheritance virtual assertions in PA18. Move non-virtual
  multiple-inheritance assertions to PA29 and polymorphic non-primary
  adjustment assertions to PA30 unless reduced.

### PA19

Potential too-late features appearing in file names:

- explicit specialization
- non-type template arguments
- `static_assert`
- `constexpr`
- partial ordering / partial specialization language
- PA18 multiple-base fixture reuse
- `pa19/tests/general/200-forwarding-static-cast.t`
- `pa19/tests/general/400-primary-polymorphic-base-before-nonpoly-static-cast.t`

Audit action:

- Keep basic-template assertions in PA19. Move NTTP/static-assert/explicit
  specialization assertions to PA20, specialization-graph assertions to PA22,
  and full deduction/SFINAE/no-eager assertions to PA23.

### PA20

Potential too-late features appearing in file names:

- partial specialization
- variable templates
- constexpr static data members
- member-pointer non-type template parameters

Audit action:

- Keep integral NTTP/static-assert/explicit-specialization assertions in PA20.
  Move partial/alias/variable specialization graph assertions to PA22 and
  non-integral NTTP completion to PA23 unless reduced.

### PA21

Potential cross-feature areas:

- constexpr function templates
- dependent constexpr static members
- template-qualified static member arithmetic
- noexcept/decltype static assertions

Audit action:

- Keep constant-evaluation assertions in PA21. Move tests whose primary
  assertion is template entity ownership or specialization selection to PA22.

### PA22

Potential boundary concerns:

- tests with `sfinae` in the name
- explicit-instantiation definition tests
- function-template deduction edge cases
- no-eager-instantiation timing tests
- `pa22/tests/general/300-decltype-value-category.t`

Audit action:

- Keep entity/specialization graph assertions in PA22. Move SFINAE, full
  function-template deduction, function partial ordering, and no-eager timing
  assertions to PA23.

### PA23

Potential reverse-placement concerns:

- tests whose primary assertion is only basic template declaration,
  instantiation, explicit specialization, constexpr, alias, or variable-template
  behavior already owned by PA19-PA22
- tests that use full deduction/substitution syntax but can be reduced to an
  earlier PA-owned feature without losing the assertion

Audit action:

- Keep tests that assert full deduction, substitution, SFINAE, or remaining
  dependent-instantiation timing. Move tests whose primary assertion reduces to
  PA19-PA22 entity behavior.

## Feature Taxonomy Gap Pass

The first gap pass scanned all 1,262 PA15-PA23 source tests by filename cluster
and source syntax. The original feature table was missing several recurring
families now added to the auditor queue:

- PA15 procedural subfeatures: condition declarations, references, arrays and
  pointers, enums, builtin casts, variadic promotions, and extended integers.
- PA16 object-model support: aggregate/list/default initialization, default
  member initializers, access control, nested types, static members, anonymous
  members, friends, ADL, operators, default arguments, `noexcept`, new/delete,
  pseudo-destructors, `using`, attributes, and trailing returns.
- PA17 value/lookup extensions: ref-qualified members, unions, array
  new/delete, and using-directive/imported lookup behavior.
- PA18 virtual-layout details: deterministic vtable/destructor slot order and
  secondary-vptr overwrite cases.
- PA19 template basics: separate class/function template coverage, template
  defaults, dependent names, current instantiation, disambiguators, friend
  templates, function-template partial ordering, and alignment syntax.
- PA20-PA22 metaprogramming details: pointer/member NTTPs, specialization
  timing, constexpr subfamilies, current-specialization identity, and
  partial-specialization ordering.
- PA23 completion details: detector idioms, constructor/conversion-template
  deduction, initializer-list support, braced-init deduction, and non-deduced
  contexts.

Rows in this table are the draft canonical ownership target for the audit. The
README and Roadmap files should be updated later to match this table, not used
as the authority for it.

Ownership is not always the same as LowIR-test placement:

- PA10-PA12 owners are syntax/semantic owners. They cannot own LowIR output
  tests directly. A LowIR test that uses a PA10-PA12 feature belongs in the
  first LowIR-producing PA whose primary behavior needs that feature, or in the
  later feature owner if the PA10-PA12 feature is only fixture syntax.
- PA15-PA23 and PA27-PA30 `cppgm++ --emit-lowir` owners can own source-to-LowIR
  tests directly.
- PA29 owners are LowIR-to-native/backend owners. Source-level tests should not
  move to PA29 unless they are reduced to LowIR/backend inputs or the source
  lowering owner is separately defined.
- PA34 owners are hosted/source-compatibility owners, not core-language owners.

## Feature Auditor Work Queue

First clusters are draft placement targets until the per-test audit validates
them. N3485 references cite `doc/n3485.txt`; `N/A` marks CPPGM LowIR contracts,
implementation extensions, or cases without a clear single C++11 clause.

| Feature Family | Canonical Owner PA | Owner Cluster | N3485 Reference | Auditor Status | Required Detections |
| --- | --- | ---: | --- | --- | --- |
| `lowir.procedural` | `pa15` | 100 | N/A: CPPGM LowIR lowering contract | pending | `--emit-lowir` source tests, functions/globals, procedural statements, scalar expressions. |
| `stmt.condition_declaration` | `pa15` | 100 | 6.4 `[stmt.select]`, 6.7 `[stmt.dcl]` | pending | `if`/`switch` condition declarations and condition-scope lifetime. |
| `expr.reference` | `pa15` | 100 | 8.3.2 `[dcl.ref]`, 8.5.3 `[dcl.init.ref]` | pending | lvalue references, reference parameters/locals, reference calls, reference aliasing. |
| `expr.array_pointer` | `pa15` | 100 | 4.2 `[conv.array]`, 5.2.1 `[expr.sub]`, 5.7 `[expr.add]`, 8.3.4 `[dcl.array]` | pending | array decay, subscript, pointer arithmetic, one-past-end pointers, pointer compound assignment scaling. |
| `lang.enum` | `pa15` | 100 | 7.2 `[dcl.enum]` | pending | scoped/unscoped enums, enum promotion, enum comparison/lowering. |
| `lowir.procedural.local_static` | `pa21` | 300 | 3.7.1 `[basic.stc.static]`, 6.7 `[stmt.dcl]` | pending | constant-initialized function-local `static` objects, guard variables, static array initialization. |
| `lowir.procedural.local_static.dynamic_class` | `pa21` | 400 | 3.7.1 `[basic.stc.static]`, 3.8 `[basic.life]`, 6.7 `[stmt.dcl]` | pending | dynamically-initialized function-local static class objects or arrays, first-use guard emission around constructor calls, local-class static objects. |
| `lowir.procedural.float_conversion` | `pa15` | 200 | 4.6 `[conv.fpprom]`, 4.8 `[conv.double]`, 4.9 `[conv.fpint]` | pending | Source-to-LowIR floating scalar literals, promotions, conversions, returns, and branch normalization. Native/backend execution parity remains PA29 and should be tested with LowIR/backend inputs. |
| `expr.cast.builtin` | `pa15` | 200 | 5.2.9 `[expr.static.cast]`, 5.2.11 `[expr.const.cast]`, 5.4 `[expr.cast]` | pending | C-style casts, `static_cast`, `const_cast`, scalar/function/reference/pointer casts. |
| `call.variadic_promotions` | `pa15` | 200 | 5.2.2 `[expr.call]`, 4.6 `[conv.fpprom]` | pending | Source-to-LowIR argument lowering for C-style variadic calls and default promotions. Native ABI/runtime parity for variadic calls remains PA29-owned backend coverage. |
| `lang.extended_integer` | `pa30` | 300 | N/A: GNU/Clang `__int128` extension | pending | Runtime lowering for `__int128` / `__uint128_t` values through source-driver programs. PA34 keeps hosted compile-only stress for vendor typedefs, literals, and header compatibility. |
| `class.basic` | `pa16` | 100 | 9 `[class]`, 9.2 `[class.mem]` | pending | `class`/`struct`, members, methods, access labels, nested class use. |
| `class.access_control` | `pa16` | 100 | 11 `[class.access]` | pending | public/private/protected member and constructor access checks. |
| `class.nested_type` | `pa16` | 100 | 9.7 `[class.nest]`, 9.9 `[class.nested.type]`, 3.4.3.1 `[class.qual]` | pending | nested classes, nested typedefs, qualified member type names, inherited member typedefs. |
| `class.static_member` | `pa16` | 100 | 9.4 `[class.static]`, 9.4.1 `[class.static.mfct]`, 9.4.2 `[class.static.data]` | pending | static data members, static member functions, qualified static member access. |
| `class.default_member_initializer` | `pa16` | 100 | 9.2 `[class.mem]` | pending | scalar/class default member initializers and override by constructor/member initializers. |
| `class.aggregate_init` | `pa16` | 200 | 8.5.1 `[dcl.init.aggr]` | pending | aggregate initialization over the PA16 object subset, appertainment, reference member binding, subobject init targets. |
| `class.anonymous_member` | `pa16` | 300 | 9.5 `[class.union]`, 9.2 `[class.mem]` | pending | anonymous structs/unions used as members and their injected member lookup/layout. |
| `class.friend` | `pa16` | 200 | 7.1.4 `[dcl.friend]`, 11.3 `[class.friend]` | pending | friend declarations/definitions, friend member access, hidden friend declarations. |
| `class.layout.bitfield` | `pa16` | 300 | 9.6 `[class.bit]` | pending | `:` bit-field declarators, zero-width unnamed bit fields. |
| `class.bitfield.access_or_init` | `pa16` | 400 | 9.6 `[class.bit]`, 8.5.1 `[dcl.init.aggr]` | pending | bit-field member access, assignment, constructor/member/aggregate initialization. |
| `class.inheritance.single` | `pa16` | 200 | 10 `[class.derived]` | pending | one direct non-virtual base. |
| `class.attribute.no_unique_address` | `pa34` | 600 | N/A: post-C++11 attribute; 7.6 `[dcl.attr]` for attribute syntax | pending | `[[no_unique_address]]` layout, empty member overlap, recursive empty holder cases in hosted/source-compatibility inputs. |
| `expr.new_delete` | `pa17` | 300 | 5.3.4 `[expr.new]`, 5.3.5 `[expr.delete]`, 12.5 `[class.free]` | pending | scalar `new`/`delete`, placement new, class construction through allocation expressions. |
| `expr.pseudo_destructor` | `pa16` | 300 | 5.2.4 `[expr.pseudo]` | pending | scalar and class pseudo/explicit destructor-call syntax. |
| `function.default_argument` | `pa12` | 300 | 8.3.6 `[dcl.fct.default]` | pending | default arguments on namespace/member functions and constructor calls. LowIR tests using plain procedural default arguments need a PA15-or-later LowIR owner; class/template interactions inherit their later feature owners. |
| `function.noexcept` | `pa11` | 300 | 5.3.7 `[expr.unary.noexcept]`, 15.4 `[except.spec]` | pending | `noexcept` declarations, redeclarations, and metadata. LowIR tests should treat declaration-only `noexcept` as support syntax; constant-expression evaluation belongs to `constexpr.noexcept`. |
| `exception.try_catch` | `pa26` | 100 | 15 `[except]`, 15.1 `[except.throw]`, 15.3 `[except.handle]` | pending | source-level `try`/`catch`/`throw`, exception handlers, catch matching, and source-to-LowIR EH control flow. NOTE: EH also appears legitimately in non-LowIR tool-PAs — `pa10` (`--emit-ast`) parses the syntax, `pa13` (`lowir2cy86`) consumes EH LowIR; those are not source→LowIR placements and the source-pattern match over-flags them. |
| `exception.guarded_static_cleanup` | `pa26` | 200 | 3.7.1 `[basic.stc.static]`, 6.7 `[stmt.dcl]`, 12.2 `[class.temporary]`, 15 `[except]` | pending | A function-local dynamic class static whose checked LowIR contains EH control. The initialization edge must finish initializer-temporary cleanup before joining the already-initialized edge. |
| `exception.aggregate_cleanup` | `pa26` | 200 | 8.5.1 `[dcl.init.aggr]`, 12.2 `[class.temporary]`, 15 `[except]` | pending | An aggregate return over destructible class members whose checked LowIR contains EH control. Nontrivial members use constructor actions, and cleanup tracks the constructed subobjects. |
| `exception.class_value_argument_cleanup` | `pa26` | 200 | 5.2.2 `[expr.call]`, 12.2 `[class.temporary]`, 12.4 `[class.dtor]`, 15 `[except]` | pending | A destructible class value parameter whose checked LowIR contains EH control. The caller transfers the materialized argument to the callee and must not retain a second normal-path cleanup for that object. |
| `exception.throw_operand_cleanup` | `pa26` | 200 | 12.2 `[class.temporary]`, 15.1 `[except.throw]` | pending | A class throw operand with destructible temporaries and checked LowIR EH control. Operand temporaries are destroyed after exception-object initialization and removed from later sibling unwind snapshots. |
| `exception.handler_branch_cleanup` | `pa26` | 200 | 12.4 `[class.dtor]`, 15.3 `[except.handle]` | pending | A branch inside a typed handler can reach a throwing call while an outer object remains live. The call's unwind path must finish the catch and destroy outer objects before resuming. |
| `host.eh_object` | `pa31` | 100 | N/A: host object and Itanium C++ ABI EH contract | pending | PA31 `cppgm++ -c` object/runtime coverage: EH runtime imports, personality references, unwind and LSDA facts, typed-catch references, or final host-unwinder behavior. Distinguish this layer from PA26 source-to-LowIR exception lowering. |
| `host.object_interop` | `pa32` | 100 | N/A: host object/toolchain interoperability contract | pending | PA32 `cppgm++ -c` coverage with an object-inspection sidecar or an anchor contract that names host/compiler-object behavior. Do not infer this feature from every PA32 path; runtime-only language regressions still need ownership review. |
| `host.object_attribute` | `pa32` | 200 | N/A: host object linkage and section contract | pending | GNU `weak`, `section`, `noinline`, and `visibility` attributes when they control host-object symbol binding, section placement, or inspected symbol visibility. Broader hosted attribute parsing remains PA34-owned. |
| `host.abi_name_attribute` | `pa33` | 200 | N/A: Itanium host ABI name contract | pending | ABI-tag attributes on constructors, destructors, and other object-inspected host symbols. General attribute parsing and hosted-header compatibility remain PA34-owned. |
| `host.abi_builtin_type` | `pa33` | 200 | N/A: Itanium host ABI name contract | pending | A compiler builtin transform type that appears in an object-inspected host symbol. General builtin-trait and transform source compatibility remains PA34-owned. |
| `hosted.runtime_compat` | `pa34` | 800 | N/A: hosted link/run compatibility contract | pending | PA34 `tests/run` smokes that compile hosted or vendor source forms and then link and execute them. Their underlying syntax may first appear in an earlier PA34 compile cluster. |
| `lookup.adl` | `pa16` | 200 | 3.4.2 `[basic.lookup.argdep]` | pending | associated namespaces/classes, hidden friends, ADL suppression and ambiguity. |
| `operator.overload` | `pa16` | 300 | 13.5 `[over.oper]` | pending | member/nonmember/friend operators, operator function ids, overloaded subscript/shift/call operators over the PA16 object subset. |
| `class.using_declaration` | `pa16` | 300 | 7.3.3 `[namespace.udecl]`, 10.2 `[class.member.lookup]` | pending | using declarations that re-expose inherited members or affect access. |
| `class.inheriting_constructor` | `pa16` | 500 | 12.9 `[class.inhctor]`, 7.3.3 `[namespace.udecl]`, 10 `[class.derived]` | pending | `using Base::Base` inheriting constructors, inherited constructor candidate lookup, and construction of the derived object through an inherited base constructor. Template-dependent inherited constructor fixtures inherit the enclosing template owner if template behavior is essential. |
| `class.inheritance.multiple` | `pa27` | 100 | 10.1 `[class.mi]` | pending | Multiple direct non-virtual bases, repeated base names, and non-primary non-virtual base paths. |
| `class.member_pointer` | `pa27` | 300 | 4.11 `[conv.mem]`, 5.5 `[expr.mptr.oper]`, 8.3.3 `[dcl.mptr]` | pending | `T C::*`, `.*`, `->*`, member pointer conversions/null tests over the completed non-virtual object model. Virtual-base and polymorphic adjustment cases inherit the PA28 pointer-adjustment owner. |
| `class.conversion_operator` | `pa17` | 400 | 12.3.2 `[class.conv.fct]` | pending | non-template `operator T()` declarations and calls over the completed PA17 value subset. |
| `function.trailing_return` | `pa11` | 300 | 8.3.5 `[dcl.fct]`, 7.1.6.4 `[dcl.spec.auto]` | pending | trailing return type syntax as type/declarator analysis. LowIR tests inherit the enclosing function kind's LowIR owner; auto return deduction belongs to `support.auto`. |
| `support.attribute` | `pa34` | 500 | 7.6 `[dcl.attr]` | pending | standard/GNU attributes outside `no_unique_address`, including hosted/vendor declaration attributes and attributes on specialization declarations. |
| `lifetime.ctor_dtor` | `pa16` | 200 | 3.8 `[basic.life]`, 12.1 `[class.ctor]`, 12.4 `[class.dtor]` | pending | constructors, destructors, local/global lifetime, init/fini refs. |
| `value.copy_move` | `pa17` | 100 | 12.8 `[class.copy]`, 8.4.2 `[dcl.fct.def.default]`, 8.4.3 `[dcl.fct.def.delete]` | pending | copy/move ctors/assignments, `= default`, `= delete`, move-only patterns. |
| `value.by_value_abi` | `pa17` | 200 | 5.2.2 `[expr.call]`, 6.6.3 `[stmt.return]`, 12.8 `[class.copy]` | pending | class by-value params/returns, indirect return refs. |
| `value.temporary` | `pa17` | 200 | 12.2 `[class.temporary]`, 3.10 `[basic.lval]` | pending | prvalue materialization, const-ref temp binding, return-slot reuse. |
| `value.ref_qualified_member` | `pa17` | 200 | 8.3.5 `[dcl.fct]`, 9.3.1 `[class.mfct.non-static]` | pending | `&`/`&&` ref-qualified member functions and out-of-class definitions. |
| `value.delegating_ctor` | `pa17` | 300 | 12.6.2 `[class.base.init]` | pending | delegating constructor initializer naming same class. |
| `class.union` | `pa17` | 300 | 9.5 `[class.union]` | pending | union special-member behavior, trivial storage copy, anonymous union support. |
| `expr.array_new_delete` | `pa17` | 400 | 5.3.4 `[expr.new]`, 5.3.5 `[expr.delete]` | pending | array new/delete, global operator new/delete overloads, delete-array behavior. |
| `lookup.using_directive` | `pa12` | 200 | 7.3.4 `[namespace.udir]`, 3.4.6 `[basic.lookup.udir]` | pending | using directives, imported value lookup, ambiguity, hiding, and function-body lookup effects. LowIR tests using procedural imported lookup need a PA15-or-later LowIR owner; class/template contexts inherit their enclosing feature owner. |
| `support.lambda` | `pa25` | 200 | 5.1.2 `[expr.prim.lambda]` | pending | Captureless lambda semantic/lowering support. |
| `support.lambda.capture.ref_this` | `pa25` | 200 | 5.1.2 `[expr.prim.lambda]` | pending | Explicit by-reference local capture and explicit/implicit `this` capture in the PA25-supported subset. |
| `support.lambda.capture` | `pa26` | 100 | 5.1.2 `[expr.prim.lambda]` | pending | By-copy, default, mutable, and broader capturing-lambda closure object support. |
| `support.range_for` | `pa25` | 100 | 6.5.4 `[stmt.ranged]` | pending | Range-for semantic/lowering support over already-owned begin/end and array forms. |
| `support.decltype` | `pa11` | 200 | 7.1.6.2 `[dcl.type.simple]` | pending | `decltype` type analysis. LowIR tests inherit the enclosing feature owner unless `decltype` itself is the primary semantic assertion. |
| `support.auto` | `pa25` | 100 | 7.1.6.4 `[dcl.spec.auto]` | pending | `auto` in declarations and non-template return type deduction. |
| `support.bodyless_placeholder_return` | `pa34` | 500 | N/A: C++14 placeholder-return extension | pending | Bodyless function declarations with a deduced `auto` return, including deleted overloads accepted while compiling newer hosted headers. PA25 owns only definitions with a visible deduction body. |
| `support.host_predefined_macro` | `pa34` | 300 | N/A: hosted predefined macro import | pending | Host/compiler predefined macros imported from the configured host toolchain, including compiler/target identity macros, `__SIZE_TYPE__`, `__PTRDIFF_TYPE__`, `__CHAR_BIT__`, and other type/limit/width macros. |
| `support.preprocessor_probe` | `pa34` | 300 | N/A: hosted preprocessor compatibility | pending | Hosted preprocessor probe operators and function-like probes such as `__has_builtin`, `__has_feature`, `__has_extension`, `__has_include`, `__has_attribute`, and `__is_identifier`. |
| `rtti.typeid` | `pa26` | 100 | 5.2.8 `[expr.typeid]` | pending | `typeid(type-id)`, supported `typeid(expr)`, and deterministic RTTI/typeinfo LowIR for the PA26 RTTI subset. |
| `rtti.dynamic_cast.pointer` | `pa26` | 100 | 5.2.7 `[expr.dynamic.cast]` | pending | Pointer-form `dynamic_cast<T*>` over the supported single-inheritance polymorphic subset and the associated `__dynamic_cast`/RTTI helper facts. |
| `rtti.dynamic_cast.void` | `pa27` | 100 | 5.2.7 `[expr.dynamic.cast]` | pending | `dynamic_cast<void*>` over the existing single-vptr polymorphic ABI and the `_ZTIv` helper fact. |
| `rtti.dynamic_cast.multi_vptr` | `pa28` | 100 | 5.2.7 `[expr.dynamic.cast]`, 10.1 `[class.mi]` | pending | `dynamic_cast` and `typeid` cases that require the multi-vtable / virtual-base RTTI ABI, including VMI RTTI helper facts. |
| `polymorphic.basic` | `pa18` | 100 | 10.3 `[class.virtual]` | pending | `virtual`, virtual calls, vtables, vptr stores. |
| `polymorphic.override_final` | `pa18` | 200 | 10.3 `[class.virtual]`, 9.2 `[class.mem]` | pending | `override`, method-level `final`. |
| `polymorphic.vdtor` | `pa18` | 300 | 10.3 `[class.virtual]`, 12.4 `[class.dtor]` | pending | virtual destructors, destructor override. |
| `polymorphic.vtable_order` | `pa18` | 400 | N/A: ABI/LowIR representation detail for 10.3 `[class.virtual]` | pending | virtual declaration order and destructor slot order for the single-inheritance virtual model. |
| `polymorphic.pointer_adjust` | `pa28` | 100 | 10.1 `[class.mi]`, 4.10 `[conv.ptr]` | pending | Polymorphic non-primary base casts, adjustor thunk-like references, virtual-base address forwarding, and multi-vptr dispatch adjustment. |
| `template.type` | `pa19` | 100 | 14.1 `[temp.param]`, 14.3.1 `[temp.arg.type]` | pending | `template<class/typename>`, template-id type args. |
| `template.class` | `pa19` | 100 | 14.5.1 `[temp.class]` | pending | class template declarations, instantiation, member definitions, static members. |
| `template.function` | `pa19` | 100 | 14.5.6 `[temp.fct]`, 14.8 `[temp.fct.spec]` | pending | function template declarations, calls, and basic overload participation; local static tests inherit the local-static owner. |
| `template.default_argument` | `pa19` | 200 | 14.1 `[temp.param]` | pending | supported type-parameter defaults and preservation of default-argument scope; template-template defaults inherit the template-template owner, and non-type defaults inherit the NTTP owner. |
| `template.dependent_name` | `pa19` | 300 | 14.6.2 `[temp.dep]`, 14.6.4 `[temp.dep.res]` | pending | dependent qualified names, dependent member lookup, delayed body checks. |
| `template.friend` | `pa22` | 300 | 14.5.4 `[temp.friend]`, 14.6.5 `[temp.inject]` | pending | friend templates, hidden friend templates, namespace-scope friend definitions. Rebalanced from PA19 because this is template declaration-graph and ownership behavior, not first-template instantiation. |
| `template.current_instantiation` | `pa19` | 300 | 14.6.2.1 `[temp.dep.type]` | pending | current instantiation lookup and owner identity in template bodies. |
| `template.disambiguator` | `pa19` | 300 | 14.2 `[temp.names]`, 14.6.2.1 `[temp.dep.type]` | pending | dependent `typename` and `template` disambiguators. |
| `template.function_partial_ordering` | `pa23` | 200 | 14.5.6.2 `[temp.func.order]`, 14.8.2.4 `[temp.deduct.partial]` | pending | function-template partial ordering, pack fallback ordering, overload selection beyond the PA19 basic deduction subset. |
| `template.alignas_alignof` | `pa16` | 300 | 7.6.2 `[dcl.align]`, 5.3.6 `[expr.alignof]` | pending | standard `alignas` layout and `alignof`; GNU `__alignof__` remains hosted/vendor compatibility. |
| `template.pack` | `pa20` | 200 | 14.5.3 `[temp.variadic]` | pending | `...` packs and pack expansions. Rebalanced from PA19 so PA20 owns the first metaprogramming/variadic extension over basic templates. |
| `template.template_parameter` | `pa22` | 200 | 14.1 `[temp.param]`, 14.3.3 `[temp.arg.template]` | pending | template-template parameter syntax. Rebalanced from PA19 because this is second-order template entity modeling. |
| `template.member_template` | `pa22` | 300 | 14.5.2 `[temp.mem]` | pending | member templates, templated operators/call operators. Rebalanced from PA19 because member-template collection/ownership fits the PA22 entity graph. |
| `template.builtin_traits` | `pa34` | 500 | N/A: compiler intrinsics/hosted trait probes | pending | builtin trait/intrinsic expressions such as `__is_*`, `__builtin_*`, builtin type transforms such as `__remove_cv(T)` / `__decay(T)`, and pack/sequence intrinsics such as `__type_pack_element` / `__make_integer_seq`. |
| `template.nttp` | `pa20` | 100 | 14.1 `[temp.param]`, 14.3.2 `[temp.arg.nontype]` | pending | integral non-type template params/args. |
| `template.nttp.pointer_member` | `pa23` | 400 | 14.3.2 `[temp.arg.nontype]` | pending | pointer, reference, enum, and class/static-member NTTP values; member-pointer NTTP cases also require the member-pointer owner. |
| `template.explicit_specialization` | `pa20` | 300 | 14.7.3 `[temp.expl.spec]` | pending | `template<>` explicit specialization. |
| `template.specialization_timing` | `pa20` | 400 | 14.6.4.1 `[temp.point]`, 14.7.1 `[temp.inst]`, 14.7.3 `[temp.expl.spec]` | pending | late explicit-specialization visibility and stale primary refresh; no-eager dependent failures belong to `template.no_eager_instantiation`. |
| `constexpr.integral_subset` | `pa20` | 100 | 5.19 `[expr.const]`, 7.1.5 `[dcl.constexpr]` | pending | integral constants used for template args/static_assert. |
| `static_assert` | `pa20` | 100 | 7 `[dcl.dcl]` static_assert-declaration | pending | `static_assert` over integral/template-dependent constant conditions; static assertions that require constexpr function evaluation inherit the PA21 constexpr owner. |
| `constexpr.full` | `pa21` | 100 | 7.1.5 `[dcl.constexpr]`, 5.19 `[expr.const]` | pending | `constexpr` functions/constructors/variables and constant object evaluation. |
| `constexpr.control_flow` | `pa21` | 100 | 7.1.5 `[dcl.constexpr]`, 5.19 `[expr.const]`, 6 `[stmt.stmt]` | pending | constexpr calls with branches, loops, recursion, and condition declarations. |
| `constexpr.default_argument` | `pa21` | 100 | 7.1.5 `[dcl.constexpr]`, 8.3.6 `[dcl.fct.default]` | pending | constexpr evaluation through function/default arguments and enum default arguments. |
| `constexpr.floating` | `pa21` | 200 | 7.1.5 `[dcl.constexpr]`, 4.8 `[conv.double]`, 4.9 `[conv.fpint]` | pending | floating literals, conversions, function calls, suffixes, and static assertions in constexpr. |
| `constexpr.noexcept` | `pa21` | 200 | 7.1.5 `[dcl.constexpr]`, 5.3.7 `[expr.unary.noexcept]`, 15.4 `[except.spec]` | pending | `noexcept` as a constant expression and interaction with constructor/member traits. |
| `constexpr.object` | `pa21` | 300 | 7.1.5 `[dcl.constexpr]`, 3.8 `[basic.life]`, 8.5 `[dcl.init]` | pending | constexpr class objects, aggregate arrays, references, static members, conversions. |
| `template.partial_specialization` | `pa22` | 100 | 14.5.5 `[temp.class.spec]` | pending | class partial specializations plus later variable/alias analogue patterns; flag any function-shaped cases for reduction. |
| `template.alias` | `pa22` | 200 | 14.5.7 `[temp.alias]` | pending | alias templates and alias specialization. |
| `template.variable` | `pa22` | 200 | N/A: variable templates are post-N3485; use 14 `[temp]` by analogy | pending | variable template declarations/uses/specializations. |
| `template.current_specialization` | `pa22` | 300 | 14.6.2.1 `[temp.dep.type]` | pending | current-specialization identity across aliases, constructors, variable templates, and member bodies. |
| `template.explicit_instantiation` | `pa22` | 300 | 14.7.2 `[temp.explicit]` | pending | `extern template`, explicit instantiation declarations/definitions. |
| `template.specialization_partial_ordering` | `pa22` | 400 | 14.5.5.2 `[temp.class.order]`, 14.8.2.4 `[temp.deduct.partial]` | pending | partial-specialization ordering, repeated arguments, cv/ref/function-pointer ordering. |
| `template.deduction_full` | `pa23` | 100 | 14.8.2 `[temp.deduct]`, 14.8.2.1 `[temp.deduct.call]` | pending | function-template deduction beyond the PA22 subset, explicit/deduced argument mixing. |
| `template.detector_idiom` | `pa23` | 300 | 14.8.2 `[temp.deduct]`, 14.5.7 `[temp.alias]` | pending | `detected_or`, detector-style aliases, detection fallback selection. |
| `template.substitution` | `pa23` | 200 | 14.8.2 `[temp.deduct]`, 14.8.3 `[temp.over]` | pending | substitution contexts, dependent alias/type substitution, candidate dropping. |
| `template.conversion_deduction` | `pa23` | 300 | 14.8.2.3 `[temp.deduct.conv]`, 12.3.2 `[class.conv.fct]` | pending | conversion-function template and converting-constructor template deduction/overload behavior. |
| `template.constructor_deduction` | `pa23` | 300 | 14.8.2 `[temp.deduct]`, 13.3.1.3 `[over.match.ctor]` | pending | constructor-template participation in overload sets and braced/pack initialization. |
| `template.initializer_list` | `pa26` | 200 | 8.5.4 `[dcl.init.list]`, 18.9 `[support.initlist]` | pending | Builtin `initializer_list`, private constructor arguments, backing-array materialization, and special-member interactions. |
| `template.no_eager_instantiation` | `pa23` | 300 | 14.6.4.1 `[temp.point]`, 14.7.1 `[temp.inst]` | pending | tests relying on delayed instantiation or unevaluated dependent failures. |
| `sfinae` | `pa23` | 300 | 14.8.2 `[temp.deduct]`, 14.8.3 `[temp.over]` | pending | substitution failure, enable_if candidate dropping, SFINAE naming/hints. |
| `template.braced_init_deduction` | `pa23` | 400 | 8.5.4 `[dcl.init.list]`, 14.8.2.5 `[temp.deduct.type]` | pending | braced-init-list deduction and non-deduced braced initializer failures. |
| `template.non_deduced_context` | `pa23` | 400 | 14.8.2.5 `[temp.deduct.type]` | pending | explicit non-deduced contexts, secondary parameters, array/reference nondeduction. |

## Per-Test Decision Log

Fill this table as tests are audited. Do not move a test without an entry.

| Test | Current PA | Current Cluster | Detected Features | N3485 Refs | Primary Assertion | Decision | Target PA | Target Cluster | Notes |
| --- | --- | ---: | --- | --- | --- | --- | --- | ---: | --- |
| `pa15/tests/general/200-u128-constant-runtime-init-not-truncated.t` | `pa15` | 200 | `lang.extended_integer` | N/A GNU extension | Extended integer lowering/constant arithmetic is hosted compatibility, not PA15 procedural LowIR. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-u128-constant-runtime-init-not-truncated.t`. |
| `pa15/tests/general/200-unused-inline-u128-global.t` | `pa15` | 200 | `lang.extended_integer`, `constexpr.integral_subset` | N/A GNU extension | Inline constexpr `__uint128_t` globals are hosted compatibility. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-unused-inline-u128-global.t`. |
| `pa16/tests/general/200-anonymous-helper-followed-by-nested-type-no-unique-address.t` | `pa16` | 200 | `class.attribute.no_unique_address`, `support.attribute` | N/A post-C++11 attribute | Primary assertion is `[[no_unique_address]]`/hosted layout. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-anonymous-helper-followed-by-nested-type-no-unique-address.t`. |
| `pa16/tests/general/200-no-unique-address-empty-member-size.t` | `pa16` | 200 | `class.attribute.no_unique_address`, `support.attribute` | N/A post-C++11 attribute | Primary assertion is `[[no_unique_address]]` hosted layout. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-no-unique-address-empty-member-size.t`. |
| `pa16/tests/general/200-recursive-no-unique-address-empty-holder.t` | `pa16` | 200 | `class.attribute.no_unique_address`, `template.builtin_traits` | N/A hosted | Primary assertion combines `[[no_unique_address]]` with builtin empty-trait behavior. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-recursive-no-unique-address-empty-holder.t`. |
| `pa19/tests/general/200-static-assert-builtin-trait-non-type-argument.t` | `pa19` | 200 | `template.builtin_traits`, `static_assert` | N/A hosted trait probes | The useful shape is static assertion through a hosted builtin trait wrapper. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-static-assert-builtin-trait-non-type-argument.t`. |
| `pa20/tests/general/100-pair-template-parameter-clause-smoke.t` | `pa20` | 100 | `template.builtin_traits` | N/A hosted trait probes | `__is_same` in an enable-if NTTP clause is hosted builtin compatibility. | Move original | `pa34` | 700 | Restored original `__is_same` expression; now `pa34/tests/compile/700-pair-template-parameter-clause-smoke.t`. |
| `pa20/tests/spec/200-bool-alias-base-preserves-nontype-type.t` | `pa20` | 200 | `template.builtin_traits` | N/A hosted trait probes | `__is_constructible`/`__is_base_of` alias-base interaction is hosted builtin compatibility. | Move original | `pa34` | 700 | Restored original builtin expressions; now `pa34/tests/compile/700-bool-alias-base-preserves-nontype-type.t`. |
| `pa20/tests/general/100-and-alias-decltype-pack-call.t` | `pa20` | 100 | `template.builtin_traits` | N/A hosted trait probes | Alias and decltype shape depends on `__is_assignable`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-and-alias-decltype-pack-call.t`. |
| `pa20/tests/general/100-is-assignable-deleted-special-member.t` | `pa20` | 100 | `template.builtin_traits` | N/A hosted trait probes | Direct `__is_assignable` special-member behavior is hosted builtin compatibility. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-is-assignable-deleted-special-member.t`. |
| `pa20/tests/general/200-defaulted-dependent-nontype-expression-syntax.t` | `pa20` | 200 | `template.builtin_traits` | N/A hosted trait probes | Dependent NTTP default uses `__is_enum`; PA20 already has non-builtin default-expression coverage. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-defaulted-dependent-nontype-expression-syntax.t`. |
| `pa21/tests/general/400-local-u128-constexpr-not-truncated.t` | `pa21` | 400 | `lang.extended_integer` | N/A GNU extension | Local constexpr `__uint128_t` arithmetic is hosted extended-integer compatibility. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-local-u128-constexpr-not-truncated.t`. |
| `pa22/tests/general/100-template-specialization-qualified-constructor-attribute-declaration.t` | `pa22` | 100 | `support.attribute` | 7.6 `[dcl.attr]` / hosted GNU attribute | The attribute-bearing specialization declaration belongs to hosted compatibility. | Move original | `pa34` | 500 | Now `pa34/tests/compile/500-template-specialization-qualified-constructor-attribute-declaration.t`; non-attribute constructor specialization remains covered in PA22. |
| `pa22/tests/general/100-template-specialization-qualified-destructor-attribute-declaration.t` | `pa22` | 100 | `support.attribute` | 7.6 `[dcl.attr]` / hosted GNU attribute | The attribute-bearing specialization declaration belongs to hosted compatibility. | Move original | `pa34` | 500 | Now `pa34/tests/compile/500-template-specialization-qualified-destructor-attribute-declaration.t`. |
| `pa22/tests/general/400-alias-rebind-partial-specialization-shadow.t` | `pa22` | 400 | `support.attribute` | 14.5.5.2 `[temp.class.order]`, 14.5.7 `[temp.alias]` | Primary assertion is alias rebind partial-specialization selection; GNU `nodebug` attributes were incidental. | Reduce in place | `pa22` | 400 | Removed `[[__gnu__::__nodebug__]]` attributes. |
| `pa23/tests/general/200-dependent-base-builtin-trait.t` | `pa23` | 200 | `template.builtin_traits` | N/A hosted trait probes | Dependent base using `__is_convertible` is hosted builtin compatibility. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-dependent-base-builtin-trait.t`. |
| `pa23/tests/general/300-common-type-declval-no-body-instantiation.t` | `pa23` | 300 | `template.builtin_traits` | 14.6.4.1 `[temp.point]`, 14.7.1 `[temp.inst]` | Primary assertion is that the `declval` body is not instantiated; builtin was only a dependent false sentinel. | Reduce in place | `pa23` | 300 | Replaced `!__is_same(T, T)` with `sizeof(T) == 0`. |
| `pa23/tests/general/300-common-type-recursive-declval-no-body-instantiation.t` | `pa23` | 300 | `template.builtin_traits` | 14.6.4.1 `[temp.point]`, 14.7.1 `[temp.inst]` | Recursive common-type variant of the no-body-instantiation sentinel. | Reduce in place | `pa23` | 300 | Replaced `!__is_same(T, T)` with `sizeof(T) == 0`. |
| `pa23/tests/general/300-decltype-conditional-no-body-instantiation.t` | `pa23` | 300 | `template.builtin_traits` | 14.6.4.1 `[temp.point]`, 14.7.1 `[temp.inst]` | Conditional-decltype variant of the no-body-instantiation sentinel. | Reduce in place | `pa23` | 300 | Replaced `!__is_same(T, T)` with `sizeof(T) == 0`. |
| `pa23/tests/general/300-template-anonymous-packed-bitfield-layout.t` | `pa23` | 300 | `class.attribute.no_unique_address`, `support.attribute` | N/A hosted attributes | Packed anonymous bitfield plus `[[no_unique_address]]` is hosted layout compatibility. | Move original | `pa34` | 600 | Now `pa34/tests/compile/600-template-anonymous-packed-bitfield-layout.t`. |
| `pa23/tests/general/400-constructible-derived-base-template-ctor.t` | `pa23` | 400 | `template.builtin_traits` | N/A hosted trait probes | Direct `__is_constructible` assertion is hosted builtin compatibility. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-constructible-derived-base-template-ctor.t`. |
| `pa23/tests/general/400-constructor-template-leading-const-pointer-convertible-enable-if.t` | `pa23` | 400 | `template.builtin_traits` | N/A hosted trait probes | Constructor-template enable-if depends on `__is_convertible`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-constructor-template-leading-const-pointer-convertible-enable-if.t`. |
| `pa23/tests/general/400-constructor-template-pointer-convertible-default-enable-if.t` | `pa23` | 400 | `template.builtin_traits` | N/A hosted trait probes | Constructor-template enable-if depends on `__is_convertible`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-constructor-template-pointer-convertible-default-enable-if.t`. |
| `pa23/tests/general/400-member-alias-assignable-rvalue-assignment.t` | `pa23` | 400 | `template.builtin_traits` | N/A hosted trait probes | Member alias assignment condition depends on `__is_assignable`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-member-alias-assignable-rvalue-assignment.t`. |
| `pa23/tests/general/400-partial-specialization-enable-if-constructor-selection.t` | `pa23` | 400 | `template.builtin_traits` | N/A hosted trait probes | Complex constructor-selection reducer depends on `__is_convertible`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-partial-specialization-enable-if-constructor-selection.t`. |
| `pa23/tests/general/500-bound-owner-inherited-member-type.t` | `pa23` | 500 | `template.builtin_traits` | N/A hosted trait probes | Inherited member-type check used `__is_same`; similar non-builtin PA23 coverage remains. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-bound-owner-inherited-member-type.t`. |
| `pa23/tests/general/500-carried-dependent-bool-member-argument.t` | `pa23` | 500 | `template.builtin_traits` | N/A hosted trait probes | EBO/default-argument shape depends on `__is_final`/`__is_empty`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-carried-dependent-bool-member-argument.t`. |
| `pa23/tests/general/500-compatible-alias-converting-ctor.t` | `pa23` | 500 | `template.builtin_traits` | N/A hosted trait probes | Boost/libc++ compatibility reducer wraps `__is_convertible`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-compatible-alias-converting-ctor.t`. |
| `pa23/tests/general/500-defaulted-nontype-enable-if-constructible-ref.t` | `pa23` | 500 | `template.builtin_traits` | N/A hosted trait probes | Defaulted NTTP enable-if shape depends on hosted `__is_constructible`/`__is_same`. | Move original | `pa34` | 700 | Now `pa34/tests/compile/700-defaulted-nontype-enable-if-constructible-ref.t`. |
| `pa19/tests/general/100-inherited-constructor-using-alias-template.t` | `pa19` | 100 | `template.nttp` | 14.1 `[temp.param]`, 14.3.2 `[temp.arg.nontype]` | Primary assertion is alias-template inherited-constructor lookup; bool NTTP was incidental. | Reduce in place | `pa19` | 100 | Removed the bool NTTP fixture. |
| `pa19/tests/general/300-empty-base-pack-expansion-sizeof.t` | `pa19` | 300 | `static_assert` | 14.5.3 `[temp.variadic]` | Primary assertion is empty base pack expansion; static assertion was not required. | Reduce in place | `pa19` | 300 | Replaced `static_assert` with runtime return check. |
| `pa19/tests/general/300-sizeof-pack-expression.t` | `pa19` | 300 | `constexpr.noexcept` | 14.5.3 `[temp.variadic]` | Primary assertion is `sizeof...`; constexpr/noexcept member was incidental. | Reduce in place | `pa19` | 300 | Replaced constexpr function with enum constant. |
| `pa19/tests/spec/300-member-call-template-hides-inherited-instantiation.t` | `pa19` | 300 | `template.nttp` | 14.5.2 `[temp.mem]` | Primary assertion is member-call template hiding inherited instantiation; NTTP parameter was incidental. | Reduce in place | `pa19` | 300 | Replaced integer NTTP with type template parameter. |
| `pa19/tests/general/200-local-constructor-template-member-typedef.t` | `pa19` | 200 | `template.builtin_traits`, `sfinae` | 14.8 `[temp.fct.spec]` | Local constructor-template member typedef assertion needs PA23 deduction; hosted builtin traits were incidental. | Move and reduce | `pa23` | 300 | Now `pa23/tests/general/300-local-constructor-template-member-typedef.t`; stripped hosted trait enable-if. |
| `pa19/tests/general/200-local-member-call-constructor-template-instantiation.t` | `pa19` | 200 | `template.builtin_traits`, `sfinae` | 14.8 `[temp.fct.spec]` | Local member-call constructor-template assertion needs PA23 deduction; hosted builtin traits were incidental. | Move and reduce | `pa23` | 300 | Now `pa23/tests/general/300-local-member-call-constructor-template-instantiation.t`; stripped hosted trait enable-if. |
| `pa19/tests/spec/100-local-constructor-template-member-typedef.t` | `pa19` | 100 | `template.builtin_traits`, `sfinae` | 14.8 `[temp.fct.spec]` | Spec twin of local constructor-template member typedef PA23 behavior. | Move and reduce | `pa23` | 300 | Now `pa23/tests/spec/300-local-constructor-template-member-typedef.t`; stripped hosted trait enable-if. |
| `pa19/tests/spec/100-local-member-call-constructor-template-instantiation.t` | `pa19` | 100 | `template.builtin_traits`, `sfinae` | 14.8 `[temp.fct.spec]` | Spec twin of local member-call constructor-template PA23 behavior. | Move and reduce | `pa23` | 300 | Now `pa23/tests/spec/300-local-member-call-constructor-template-instantiation.t`; stripped hosted trait enable-if. |

## README Update Log

| PA | Change | Status | Notes |
| --- | --- | --- | --- |
| `pa15` | Exact procedural LowIR contract | pending | Resolve five-arg calls and local static boundary. Floating conversion and variadic source-to-LowIR placement is settled as PA15-owned; native/backend parity remains PA29-owned. |
| `pa16` | Exact object-model/support-feature contract | pending | Resolve known conflicts first. |
| `pa17` | Exact value-semantics/support-syntax contract | pending | Classify lambda/range-for/template fixture tests. |
| `pa18` | Exact polymorphism/multiple-base decision | pending | Decide whether to own tested limited multiple-base subset. |
| `pa19` | Exact basic-template boundary | pending | Remove/clarify accidental PA20/PA22 feature tests. |
| `pa20` | Exact metaprogramming boundary | pending | Separate explicit specialization from partial specialization. |
| `pa21` | Exact constant-evaluation boundary | pending | Ensure template usage is support-only unless PA22-owned. |
| `pa22` | Exact specialization/entity boundary | pending | Align explicit-instantiation wording with roadmap/tests. |
| `pa23` | Exact deduction/substitution/SFINAE contract | pending | Move earlier-owned tests out after reduction. |

## Validation Log

| Date | Command | Result | Notes |
| --- | --- | --- | --- |
| _pending_ | | | |
