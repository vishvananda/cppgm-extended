# PA15-PA23 Placement Validation

This is the maintainer validation pass over:

- `docs/pa15-pa17-placement-decisions.md`
- `docs/pa18-pa20-placement-decisions.md`
- `docs/pa21-pa23-placement-decisions.md`

No tests or refs were moved during this pass. The goal here is to decide which
reported moves are mechanically ready and which need a maintainer call before
the move pass.

## Owner Calls Applied

These owner rows have been added to the taxonomy from the maintainer call.
There are no remaining unresolved calls from this validation pass.

| Item | Current Test(s) | Decision |
| --- | --- | --- |
| Dynamic guarded local static class arrays | `pa16/tests/general/200-local-static-local-class-array-init.t` | `lowir.procedural.local_static.dynamic_class`, PA21:400. |
| Capturing lambdas / closure objects | `pa17/tests/spec/300-lambda-capture-mutable.t`, `pa19/tests/spec/100-local-lambda-function-template-concrete-type.t`, `pa23/tests/general/400-lambda-rtti-typeinfo-name.t` | `support.lambda.capture`, PA27:300. |
| Inheriting constructors | `pa17/tests/general/300-inheriting-constructors.t`, `pa19/tests/general/100-inherited-constructor-using-alias-template.t` | `class.inheriting_constructor`, PA16:500 for the non-template feature. PA16 is the closest owner because it already owns single inheritance, class using-declarations, and constructor/lifetime lowering. Template-dependent fixture variants inherit the template owner unless reduced. |

## PA15-PA17

Accepted as reported:

| Validation | Decision |
| --- | --- |
| PA15 enum `static_cast` case | Renumber to PA15:200. |
| PA15 local static scalar/array guard cases | Move constant/static storage cases to PA21:300; move dynamic class local-static cases to PA21:400. |
| PA15 floating conversion and variadic float cases | Remove from PA15 source-to-LowIR coverage or reduce to PA29 backend/LowIR inputs. |
| PA15 qualified namespace overload symbol case | Split/reduce: keep integral namespace-symbol coverage in PA15 and move optional floating coverage to backend/PA29 form. |
| PA15 one-past-end pointer case | Keep in PA15; `double` is a pointer-size fixture here, not floating codegen. |
| PA15 incomplete class referent and qualified nested typedef cases | Move to PA16:100. |
| PA15 `__uint128_t` cases | Move to PA34:600. |
| PA16 default member initializer/basic static/access cases | Keep in PA16:100. |
| PA16 ordinary inheritance, friends, lifetime, and global ctor cases currently in cluster 100 | Renumber to PA16:200. |
| PA16 anonymous member cases | Renumber to PA16:300. |
| PA16 deleted copy assignment | Move to PA17:100. |
| PA16 class value-transfer/copyobj cases | Move to PA17:200. |
| PA16 member-pointer cases | Move to PA29:300. |
| PA16 operator-overload cases | Renumber to PA16:300. |
| PA16 bit-field layout cases | Renumber to PA16:300. |
| PA16 bit-field access/init cases | Renumber to PA16:400. |
| PA16 scalar/placement `new` cases | Move to PA17:300. |
| PA16 conversion-operator cases | Move to PA17:400. |
| PA16 LowIR metadata cases | Keep in PA16. Scanner builtin hits are false positives. |
| PA16 pseudo-destructor case | Renumber to PA16:300. |
| PA16 trailing-return member case | Keep in PA16:300; `auto` is trailing-return syntax, not deduction. |
| PA16 static constexpr array member case | Move to PA21:300. |
| PA16 list-init narrowing diagnostic | Keep in PA16. |
| PA16 unused member static_assert failure | Move to PA20:100. |
| PA17 delegating constructor | Renumber to PA17:300. |
| PA17 derived-to-base by-value overload | Keep in PA17:200. |
| PA17 array new/delete cases | Renumber to PA17:400. |
| PA17 conversion-operator condition/cast cleanup cases | Renumber to PA17:400. |
| PA17 proxy subscript assignment | Keep in PA17:300. |
| PA17 inheriting constructor | Move to PA16:500. |
| PA17 range-for member begin/end | Move to PA27:100. |
| PA17 capturing mutable lambda | Move to PA27:300. |

Adjusted from the first-pass report:

| Test | Validated Decision |
| --- | --- |
| `pa16/tests/general/100-anonymous-compressed-pair-member.t` | Move the existing template test to PA19:100 or reduce a non-template anonymous-member version for PA16:300. A split is optional, not required. |
| PA16 ADL tests with `copyobj` refs | Prefer moving the current tests to PA17:200 unless we create reduced lookup-only PA16 replacements. The current checked refs materially assert by-value class transfer. |
| `pa16/tests/general/200-class-member-object-sizeof.t` | Move current static-assert form to PA20:100; create a reduced PA16 runtime `sizeof` test only if we want duplicate object-size coverage. |
| `pa17/tests/general/300-using-base-overload-set.t` | Reduce to integral overloads and move to PA16:300. Keep a floating overload only as backend/PA29-style coverage if still valuable. |
| `pa16/tests/general/200-local-static-local-class-array-init.t` | Move current dynamic guarded class-array static test to PA21:400. |

## PA18-PA20

Accepted as reported:

| Group | Validation |
| --- | --- |
| G1 | Keep PA18 virtual tests. Scanner vtable-order hits are false positives unless the test asserts order. |
| G2 | Move the PA18 course temporary-derived reference virtual-call test into local PA18:300. |
| G3 | Move PA18 non-primary/multibase virtual adjustment tests to PA30:100. |
| G4 | Renumber override signature mismatch to PA18:200. |
| G5 | Keep basic PA19 template tests. |
| G6 | Renumber PA19 dependent-name/current-instantiation basics to PA19:200. |
| G7 | Renumber PA19 packs/member-template/template-template/friend-template cases to PA19:300. |
| G9 | Move function-template local-static-per-specialization and constexpr linkage cases to PA21. |
| G10 | Move partial-specialization / pack-specialization cases to PA22. |
| G11 | Move full deduction, partial ordering, SFINAE, and no-eager cases to PA23. |
| G12 | Move later support-feature cases to PA27/PA28/PA29/PA30/PA34 as listed in the report. Capturing lambda cases go to PA27:300. |
| G13 | Resolve as PA19:200. `dependent-sizeof-type-array-member` is a dependent type/layout test, not a later template-completion case. |
| G14 | Keep PA20 integral NTTP/static_assert subset tests. |
| G15 | Renumber explicit-specialization tests to PA20:200. |
| G16 | Renumber specialization timing/stale-primary tests to PA20:300. |
| G18 | Move alias, variable-template, and partial-specialization graph tests to PA22. |
| G19 | Move non-integral/function-pointer/reference NTTPs, SFINAE, full deduction, constructor-template deduction, and no-eager cases to PA23. |
| G20 | Move builtin trait/intrinsic cases to PA34 and member-pointer cases to PA29. |

Adjusted from the first-pass report:

| Test(s) | Validated Decision |
| --- | --- |
| `pa19/tests/general/100-empty-base-pack-expansion-sizeof.t` | Renumber to PA19:300 after reducing/removing the `static_assert` if needed. The primary feature is pack expansion in a base list, not PA20 constant evaluation. |
| `pa19/tests/general/100-pack-expansion-array-unknown-bound.t` | Move to PA20:100 because the test uses an integral NTTP pack. |
| `pa19/tests/general/200-template-alignas-nontype-argument.t` | Move to PA20:100 or split: PA16 alignas/alignof coverage plus PA20 NTTP argument coverage. The current test needs the NTTP value. |
| `pa19/tests/general/200-constructor-template-const-ref-conversion.t`, `pa19/tests/general/200-constructor-template-direct-other-specialization.t`, `pa19/tests/general/200-constructor-template-same-owner-nontype-parameter.t`, and matching spec cases | Move to PA23:300. These are constructor-template deduction/participation cases, not just PA20 NTTP cases. |
| `pa19/tests/general/100-inherited-constructor-using-alias-template.t` | Reduce the bool NTTP fixture if possible. Place the non-template inheriting-constructor core at PA16:500; place the class-template/alias interaction at PA19:300 if retained. |
| `pa19/tests/spec/100-type-equivalence-default-argument.t` | Move to PA20:100 because the assertion depends on `static_assert`; the default type argument itself is PA19 support syntax. |
| `pa20/tests/general/100-sizeof-pack-expression.t` | Move/reduce to PA19:300. `sizeof...` over a pack is the primary assertion; `constexpr` is not essential to the runtime check. |
| `pa20/tests/spec/200-static-constexpr-array-template-member-odr-use.t` | Keep the first-pass move to PA21:300. This is constexpr object/static-member behavior. |

## PA21-PA23

Accepted as reported:

| Group | Validation |
| --- | --- |
| PA21 keep group | Keep PA21 constant-evaluation tests where scanner hits are support-only. |
| PA21 constexpr object/static-member group | Renumber to PA21:300. |
| PA21 constexpr/noexcept/dependent template test | Split: keep reduced PA21 constexpr/noexcept coverage and move the full dependent function-template/no-eager form to PA23. |
| PA21 `__uint128_t` constexpr test | Move to PA34:600. |
| PA22 variable-template group | Renumber to PA22:200. |
| PA22 explicit/current-specialization group | Renumber to PA22:300. |
| PA22 explicit-instantiation group | Renumber to PA22:300. |
| PA22 partial-specialization-ordering group | Renumber to PA22:400. |
| PA22 constructor-template/SFINAE group | Move to PA23:300. |
| PA22 full function-template deduction/partial-ordering group | Move to PA23:200. |
| PA22 substitution/no-eager/dependent-alias group | Move to PA23:400 unless the individual test is more specifically SFINAE at PA23:300. |
| PA22 member-pointer template cases | Move to PA29:300. |
| PA22 function-pointer/member-pointer mixed ordering case | Split function-pointer PA22:400 from member-pointer PA29:300. |
| PA22 attribute-on-specialization cases | Reduce to non-attribute PA22 coverage or move attribute variants to PA34:500. |
| PA22 broad specialization/entity graph keep group | Keep in PA22:400 on first pass. |
| PA23 non-deduced/dependent-qualified context group | Renumber to PA23:400. |
| PA23 SFINAE/detector group | Renumber to PA23:300. |
| PA23 constructor-template/no-eager group | Renumber to PA23:300. |
| PA23 conversion-function-template group | Renumber to PA23:300. |
| PA23 stress keep group | Keep in PA23. |
| PA23 member-pointer template tests | Move to PA29:300 or split a reduced non-member-pointer PA23 assertion. |
| PA23 initializer-list tests | Move to PA28:200. |
| PA23 attribute/no_unique_address template layout test | Move to PA34. |
| PA23 lambda RTTI/typeinfo case | Move to PA27:300. |
| PA23 auto-local dependent result tests | Split: reduced no-`auto` PA23 coverage plus PA27 auto coverage. |
| PA23 builtin trait/intrinsic-heavy tests | Reduce intrinsic dependency or move hosted intrinsic variants to PA34:600. |

Resolved from PA22 manual-review group:

| Test(s) | Validated Decision |
| --- | --- |
| `pa22/tests/general/100-dependent-base-using-typename-type-and-value.t` | Move to PA19:200. Primary feature is dependent `typename`/using in a template body. |
| `pa22/tests/general/100-pack-base-expansion.t` | Move to PA19:300. This is pack expansion in a base list without essential multiple-inheritance runtime behavior. |
| `pa22/tests/general/100-pointer-traits-alias-chain-core.t` | Keep in PA22:100; primary feature is class partial specialization through an alias chain. |
| `pa22/tests/general/300-decltype-value-category.t` | Move to PA20:100. The checked assertion is static_assert over decltype value categories. |
| `pa22/tests/general/400-pack-expanded-base-template-parameter-lookup.t` | Move to PA29:100 or split. Current source uses real multiple inheritance through pack-expanded bases. |
| Remaining PA22 manual-review paths in the first-pass report | Keep in PA22 unless they are later matched by the move groups above; their source shapes primarily exercise the PA22 specialization/current-specialization/entity graph. |

## Move-Pass Notes

- For `move` and `renumber`, move `.t`, `.ref`, and `.ref.exit_status` together.
- Do not carry generated `.my*`, `testout`, or local logs.
- For `split` and `reduce-or-remove`, create the reduced test first, regenerate
  only the intended refs, then move or delete the unreduced original.
- After each batch, run `make test-report ACTIVE_TEST_REPORT_PAS='paN ...'`
  over every source and destination PA touched by that batch.
