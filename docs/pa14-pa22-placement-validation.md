# PA14-PA22 Placement Validation

This is the maintainer validation pass over:

- `docs/pa14-pa16-placement-decisions.md`
- `docs/pa17-pa19-placement-decisions.md`
- `docs/pa20-pa22-placement-decisions.md`

No tests or refs were moved during this pass. The goal here is to decide which
reported moves are mechanically ready and which need a maintainer call before
the move pass.

## Owner Calls Applied

These owner rows have been added to the taxonomy from the maintainer call.
There are no remaining unresolved calls from this validation pass.

| Item | Current Test(s) | Decision |
| --- | --- | --- |
| Dynamic guarded local static class arrays | `pa15/tests/general/200-local-static-local-class-array-init.t` | `lowir.procedural.local_static.dynamic_class`, PA20:400. |
| Capturing lambdas / closure objects | `pa16/tests/spec/300-lambda-capture-mutable.t`, `pa18/tests/spec/100-local-lambda-function-template-concrete-type.t`, `pa22/tests/general/400-lambda-rtti-typeinfo-name.t` | `support.lambda.capture`, PA26:300. |
| Inheriting constructors | `pa16/tests/general/300-inheriting-constructors.t`, `pa18/tests/general/100-inherited-constructor-using-alias-template.t` | `class.inheriting_constructor`, PA15:500 for the non-template feature. PA15 is the closest owner because it already owns single inheritance, class using-declarations, and constructor/lifetime lowering. Template-dependent fixture variants inherit the template owner unless reduced. |

## PA14-PA16

Accepted as reported:

| Validation | Decision |
| --- | --- |
| PA14 enum `static_cast` case | Renumber to PA14:200. |
| PA14 local static scalar/array guard cases | Move constant/static storage cases to PA20:300; move dynamic class local-static cases to PA20:400. |
| PA14 floating conversion and variadic float cases | Remove from PA14 source-to-LowIR coverage or reduce to PA23 backend/LowIR inputs. |
| PA14 qualified namespace overload symbol case | Split/reduce: keep integral namespace-symbol coverage in PA14 and move optional floating coverage to backend/PA23 form. |
| PA14 one-past-end pointer case | Keep in PA14; `double` is a pointer-size fixture here, not floating codegen. |
| PA14 incomplete class referent and qualified nested typedef cases | Move to PA15:100. |
| PA14 `__uint128_t` cases | Move to PA33:600. |
| PA15 default member initializer/basic static/access cases | Keep in PA15:100. |
| PA15 ordinary inheritance, friends, lifetime, and global ctor cases currently in cluster 100 | Renumber to PA15:200. |
| PA15 anonymous member cases | Renumber to PA15:300. |
| PA15 deleted copy assignment | Move to PA16:100. |
| PA15 class value-transfer/copyobj cases | Move to PA16:200. |
| PA15 member-pointer cases | Move to PA28:300. |
| PA15 operator-overload cases | Renumber to PA15:300. |
| PA15 bit-field layout cases | Renumber to PA15:300. |
| PA15 bit-field access/init cases | Renumber to PA15:400. |
| PA15 scalar/placement `new` cases | Move to PA16:300. |
| PA15 conversion-operator cases | Move to PA16:400. |
| PA15 LowIR metadata cases | Keep in PA15. Scanner builtin hits are false positives. |
| PA15 pseudo-destructor case | Renumber to PA15:300. |
| PA15 trailing-return member case | Keep in PA15:300; `auto` is trailing-return syntax, not deduction. |
| PA15 static constexpr array member case | Move to PA20:300. |
| PA15 list-init narrowing diagnostic | Keep in PA15. |
| PA15 unused member static_assert failure | Move to PA19:100. |
| PA16 delegating constructor | Renumber to PA16:300. |
| PA16 derived-to-base by-value overload | Keep in PA16:200. |
| PA16 array new/delete cases | Renumber to PA16:400. |
| PA16 conversion-operator condition/cast cleanup cases | Renumber to PA16:400. |
| PA16 proxy subscript assignment | Keep in PA16:300. |
| PA16 inheriting constructor | Move to PA15:500. |
| PA16 range-for member begin/end | Move to PA26:100. |
| PA16 capturing mutable lambda | Move to PA26:300. |

Adjusted from the first-pass report:

| Test | Validated Decision |
| --- | --- |
| `pa15/tests/general/100-anonymous-compressed-pair-member.t` | Move the existing template test to PA18:100 or reduce a non-template anonymous-member version for PA15:300. A split is optional, not required. |
| PA15 ADL tests with `copyobj` refs | Prefer moving the current tests to PA16:200 unless we create reduced lookup-only PA15 replacements. The current checked refs materially assert by-value class transfer. |
| `pa15/tests/general/200-class-member-object-sizeof.t` | Move current static-assert form to PA19:100; create a reduced PA15 runtime `sizeof` test only if we want duplicate object-size coverage. |
| `pa16/tests/general/300-using-base-overload-set.t` | Reduce to integral overloads and move to PA15:300. Keep a floating overload only as backend/PA23-style coverage if still valuable. |
| `pa15/tests/general/200-local-static-local-class-array-init.t` | Move current dynamic guarded class-array static test to PA20:400. |

## PA17-PA19

Accepted as reported:

| Group | Validation |
| --- | --- |
| G1 | Keep PA17 virtual tests. Scanner vtable-order hits are false positives unless the test asserts order. |
| G2 | Move the PA17 course temporary-derived reference virtual-call test into local PA17:300. |
| G3 | Move PA17 non-primary/multibase virtual adjustment tests to PA29:100. |
| G4 | Renumber override signature mismatch to PA17:200. |
| G5 | Keep basic PA18 template tests. |
| G6 | Renumber PA18 dependent-name/current-instantiation basics to PA18:200. |
| G7 | Renumber PA18 packs/member-template/template-template/friend-template cases to PA18:300. |
| G9 | Move function-template local-static-per-specialization and constexpr linkage cases to PA20. |
| G10 | Move partial-specialization / pack-specialization cases to PA21. |
| G11 | Move full deduction, partial ordering, SFINAE, and no-eager cases to PA22. |
| G12 | Move later support-feature cases to PA26/PA27/PA28/PA29/PA33 as listed in the report. Capturing lambda cases go to PA26:300. |
| G13 | Resolve as PA18:200. `dependent-sizeof-type-array-member` is a dependent type/layout test, not a later template-completion case. |
| G14 | Keep PA19 integral NTTP/static_assert subset tests. |
| G15 | Renumber explicit-specialization tests to PA19:200. |
| G16 | Renumber specialization timing/stale-primary tests to PA19:300. |
| G18 | Move alias, variable-template, and partial-specialization graph tests to PA21. |
| G19 | Move non-integral/function-pointer/reference NTTPs, SFINAE, full deduction, constructor-template deduction, and no-eager cases to PA22. |
| G20 | Move builtin trait/intrinsic cases to PA33 and member-pointer cases to PA28. |

Adjusted from the first-pass report:

| Test(s) | Validated Decision |
| --- | --- |
| `pa18/tests/general/100-empty-base-pack-expansion-sizeof.t` | Renumber to PA18:300 after reducing/removing the `static_assert` if needed. The primary feature is pack expansion in a base list, not PA19 constant evaluation. |
| `pa18/tests/general/100-pack-expansion-array-unknown-bound.t` | Move to PA19:100 because the test uses an integral NTTP pack. |
| `pa18/tests/general/200-template-alignas-nontype-argument.t` | Move to PA19:100 or split: PA15 alignas/alignof coverage plus PA19 NTTP argument coverage. The current test needs the NTTP value. |
| `pa18/tests/general/200-constructor-template-const-ref-conversion.t`, `pa18/tests/general/200-constructor-template-direct-other-specialization.t`, `pa18/tests/general/200-constructor-template-same-owner-nontype-parameter.t`, and matching spec cases | Move to PA22:300. These are constructor-template deduction/participation cases, not just PA19 NTTP cases. |
| `pa18/tests/general/100-inherited-constructor-using-alias-template.t` | Reduce the bool NTTP fixture if possible. Place the non-template inheriting-constructor core at PA15:500; place the class-template/alias interaction at PA18:300 if retained. |
| `pa18/tests/spec/100-type-equivalence-default-argument.t` | Move to PA19:100 because the assertion depends on `static_assert`; the default type argument itself is PA18 support syntax. |
| `pa19/tests/general/100-sizeof-pack-expression.t` | Move/reduce to PA18:300. `sizeof...` over a pack is the primary assertion; `constexpr` is not essential to the runtime check. |
| `pa19/tests/spec/200-static-constexpr-array-template-member-odr-use.t` | Keep the first-pass move to PA20:300. This is constexpr object/static-member behavior. |

## PA20-PA22

Accepted as reported:

| Group | Validation |
| --- | --- |
| PA20 keep group | Keep PA20 constant-evaluation tests where scanner hits are support-only. |
| PA20 constexpr object/static-member group | Renumber to PA20:300. |
| PA20 constexpr/noexcept/dependent template test | Split: keep reduced PA20 constexpr/noexcept coverage and move the full dependent function-template/no-eager form to PA22. |
| PA20 `__uint128_t` constexpr test | Move to PA33:600. |
| PA21 variable-template group | Renumber to PA21:200. |
| PA21 explicit/current-specialization group | Renumber to PA21:300. |
| PA21 explicit-instantiation group | Renumber to PA21:300. |
| PA21 partial-specialization-ordering group | Renumber to PA21:400. |
| PA21 constructor-template/SFINAE group | Move to PA22:300. |
| PA21 full function-template deduction/partial-ordering group | Move to PA22:200. |
| PA21 substitution/no-eager/dependent-alias group | Move to PA22:400 unless the individual test is more specifically SFINAE at PA22:300. |
| PA21 member-pointer template cases | Move to PA28:300. |
| PA21 function-pointer/member-pointer mixed ordering case | Split function-pointer PA21:400 from member-pointer PA28:300. |
| PA21 attribute-on-specialization cases | Reduce to non-attribute PA21 coverage or move attribute variants to PA33:500. |
| PA21 broad specialization/entity graph keep group | Keep in PA21:400 on first pass. |
| PA22 non-deduced/dependent-qualified context group | Renumber to PA22:400. |
| PA22 SFINAE/detector group | Renumber to PA22:300. |
| PA22 constructor-template/no-eager group | Renumber to PA22:300. |
| PA22 conversion-function-template group | Renumber to PA22:300. |
| PA22 stress keep group | Keep in PA22. |
| PA22 member-pointer template tests | Move to PA28:300 or split a reduced non-member-pointer PA22 assertion. |
| PA22 initializer-list tests | Move to PA27:200. |
| PA22 attribute/no_unique_address template layout test | Move to PA33. |
| PA22 lambda RTTI/typeinfo case | Move to PA26:300. |
| PA22 auto-local dependent result tests | Split: reduced no-`auto` PA22 coverage plus PA26 auto coverage. |
| PA22 builtin trait/intrinsic-heavy tests | Reduce intrinsic dependency or move hosted intrinsic variants to PA33:600. |

Resolved from PA21 manual-review group:

| Test(s) | Validated Decision |
| --- | --- |
| `pa21/tests/general/100-dependent-base-using-typename-type-and-value.t` | Move to PA18:200. Primary feature is dependent `typename`/using in a template body. |
| `pa21/tests/general/100-pack-base-expansion.t` | Move to PA18:300. This is pack expansion in a base list without essential multiple-inheritance runtime behavior. |
| `pa21/tests/general/100-pointer-traits-alias-chain-core.t` | Keep in PA21:100; primary feature is class partial specialization through an alias chain. |
| `pa21/tests/general/300-decltype-value-category.t` | Move to PA19:100. The checked assertion is static_assert over decltype value categories. |
| `pa21/tests/general/400-pack-expanded-base-template-parameter-lookup.t` | Move to PA28:100 or split. Current source uses real multiple inheritance through pack-expanded bases. |
| Remaining PA21 manual-review paths in the first-pass report | Keep in PA21 unless they are later matched by the move groups above; their source shapes primarily exercise the PA21 specialization/current-specialization/entity graph. |

## Move-Pass Notes

- For `move` and `renumber`, move `.t`, `.ref`, and `.ref.exit_status` together.
- Do not carry generated `.my*`, `testout`, or local logs.
- For `split` and `reduce-or-remove`, create the reduced test first, regenerate
  only the intended refs, then move or delete the unreduced original.
- After each batch, run `make test-report ACTIVE_TEST_REPORT_PAS='paN ...'`
  over every source and destination PA touched by that batch.
