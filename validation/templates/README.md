# N3485 Template Validation Bank

This directory is the replacement template validation bank for the post-PA17
template milestones.

It follows the same pattern as `validation/tests`:

- one self-contained source per rule,
- `// VALIDATION:` marker on each test,
- `compile-pass` and `compile-fail` kinds,
- clang as the immediate oracle.

The goal is to make clause-14 coverage explicit and deep enough that template
failures are found here before they are discovered indirectly through STL
compilation, and to give students a complete oracle surface for the template
assignments.

## Ownership

Based on the assignment READMEs, the template flow is:

- `pa17`: polymorphism only, not owned by this bank
- `pa18`: first-tier templates
- `pa19`: C++11 NTTPs and explicit specialization
- `pa21`: specialization/entity graph and specialization selection
- `pa22`: full deduction, substitution, and SFINAE

So this bank is intended to replace the template-focused suites for
`pa18` / `pa19` / `pa21` / `pa22`, not `pa17`.

The exact bank-to-assignment mapping lives in
[`ASSIGNMENT_SLICES.md`](./ASSIGNMENT_SLICES.md).

Machine-readable slice manifests live under [`slices/`](./slices/):

- [`slices/pa18.txt`](./slices/pa18.txt)
- [`slices/pa19.txt`](./slices/pa19.txt)
- [`slices/pa21.txt`](./slices/pa21.txt)
- [`slices/pa22.txt`](./slices/pa22.txt)
- [`slices/hosted.txt`](./slices/hosted.txt)

## Oracle

Run the bank with:

```sh
python3 validation/templates/run_with_clang.py
```

Override the compiler with `CXX=/path/to/clang++` if needed.

## Dual-Oracle Model

Each bank test is intended to carry two linked contracts:

1. LowIR / compile contract
   - the primary student-facing proof
   - positive tests prove successful lowering or compilation
   - negative tests prove deterministic rejection

2. Template witness contract
   - the stricter semantic proof
   - the oracle is produced from the patched clang witness
   - `cppgm++ --emit-lowir --template-log <path>` is compared against the
     stored witness

The intended landing model is:

- the PA-local `tests/spec` directories hold the assignment-owned slice
- the PA-local `tests/general` directories hold everything else worth keeping
- every `tests/spec` test carries adjacent oracle files
- `make test` runs the same tests without checking oracles
- `make test-strict` runs the same tests and checks both the LowIR/compile
  oracle and the adjacent `.ref.witness` text when present

This keeps the primary oracle at the first assignment-owned boundary while
still giving students a direct semantic oracle for template decisions.

## Public Template Witness Format

Adjacent `.ref.witness` files are generated from the patched Clang witness
materialization flow. They are intentionally a public, student-facing
projection rather than a dump of every compiler-internal trace field.

Source locations start at `tests/` or `course/`. The Clang and compiler
renderers remove the checkout prefix and outer `paNN/` directory so moving a
test between assignments does not change its witness output.

The public `translation-unit` section records source-visible decisions:

- template use kind and source use location
- selected template or overload result
- binding and specialization argument/provenance by parameter ordinal
- coarse candidate-drop reasons where they explain overload behavior

Maintainer-only details stay out of the public witness by default:

- selected declaration locations
- deduction-guide declaration locations
- source template parameter spellings
- candidate inventory/count fields
- candidate declaration locations

When closure events are present, the public `template-closure-events` section
records stable lifecycle facts as a sorted, deduplicated set:

- lifecycle kind, such as `function-instantiation` or `class-finalization`
- normalized entity name

Closure declaration locations, closure reasons, triggers, causes, and detail
payloads are debug-only. They are useful while reducing compiler bugs, but they
are too sensitive to valid implementation scheduling choices to be part of the
assignment oracle.

## Relationship To Existing Validation Seeds

This bank is meant to replace the PA-local template suites, while still being
used together with the already-existing general template seeds in
`validation/tests`, especially:

- `011` alias template substitution
- `012` dependent `typename`
- `013`, `023` dependent member template disambiguation
- `014` default template argument merge
- `015` class partial specialization selection
- `019`, `020`, `021`, `022`
- `030` through `039`
- `104`, `110`, `112` through `116`

Those files already cover a meaningful slice of:

- `14.1 [temp.param]`
- `14.2 [temp.names]`
- `14.5.4 [temp.friend]`
- `14.5.5 [temp.class.spec]`
- `14.5.7 [temp.alias]`
- `14.6.*`
- `14.8.2.*`
- `14.8.3 [temp.over]`

The bank below fills the remaining major gaps.

## Candidate Matrix

| ID | N3485 focus | Feature | Kind | Test |
|---|---|---|---|---|
| T001 | `14.3.3 [temp.arg.template]` | template-template parameter basic match | `compile-pass` | [`tests/001-template-template-parameter-basic.cpp`](./tests/001-template-template-parameter-basic.cpp) |
| T002 | `14.3.3 [temp.arg.template]` | template-template parameter arity mismatch rejects | `compile-fail` | [`tests/002-template-template-parameter-arity-mismatch.cpp`](./tests/002-template-template-parameter-arity-mismatch.cpp) |
| T003 | `14.3.2 [temp.arg.nontype]` | function pointer as non-type template argument | `compile-pass` | [`tests/003-nontype-function-pointer-argument.cpp`](./tests/003-nontype-function-pointer-argument.cpp) |
| T004 | `14.3.2 [temp.arg.nontype]` | reference as non-type template argument | `compile-pass` | [`tests/004-nontype-reference-argument.cpp`](./tests/004-nontype-reference-argument.cpp) |
| T005 | `14.4 [temp.type]` | type equivalence through default template arguments | `compile-pass` | [`tests/005-type-equivalence-default-argument.cpp`](./tests/005-type-equivalence-default-argument.cpp) |
| T006 | `14.5.2 [temp.mem]` | member function template out-of-class definition | `compile-pass` | [`tests/006-member-function-template-out-of-class.cpp`](./tests/006-member-function-template-out-of-class.cpp) |
| T007 | `14.5.2 [temp.mem]` | member class template out-of-class definition | `compile-pass` | [`tests/007-member-class-template-out-of-class.cpp`](./tests/007-member-class-template-out-of-class.cpp) |
| T008 | `14.5.4 [temp.friend]`, `14.6.5 [temp.inject]` | friend function template inside class template | `compile-pass` | [`tests/008-template-friend-inside-class-template.cpp`](./tests/008-template-friend-inside-class-template.cpp) |
| T009 | `14.6.3 [temp.nondep]` | non-dependent name binds at template definition time | `compile-pass` | [`tests/009-nondependent-name-binding.cpp`](./tests/009-nondependent-name-binding.cpp) |
| T010 | `14.6.4 [temp.dep.res]` | dependent unqualified call finds later ADL candidate at instantiation | `compile-pass` | [`tests/010-dependent-adl-point-of-instantiation.cpp`](./tests/010-dependent-adl-point-of-instantiation.cpp) |
| T011 | `14.5.3 [temp.variadic]` | variadic pack expansion through base list | `compile-pass` | [`tests/011-variadic-base-pack-expansion.cpp`](./tests/011-variadic-base-pack-expansion.cpp) |
| T012 | `14.7.2 [temp.explicit]` | explicit instantiation declaration and definition of function template | `compile-pass` | [`tests/012-explicit-instantiation-function.cpp`](./tests/012-explicit-instantiation-function.cpp) |
| T013 | `14.7.2 [temp.explicit]` | explicit instantiation of class template | `compile-pass` | [`tests/013-explicit-instantiation-class.cpp`](./tests/013-explicit-instantiation-class.cpp) |
| T014 | `14.7.3 [temp.expl.spec]` | explicit specialization of member function of class template | `compile-pass` | [`tests/014-explicit-specialization-member-function.cpp`](./tests/014-explicit-specialization-member-function.cpp) |
| T015 | `14.7.3 [temp.expl.spec]` | explicit specialization of static data member of class template | `compile-pass` | [`tests/015-explicit-specialization-static-data-member.cpp`](./tests/015-explicit-specialization-static-data-member.cpp) |
| T016 | `14.8.1 [temp.arg.explicit]`, `14.8.2 [temp.deduct]` | explicit template arguments plus deduction for remaining parameters | `compile-pass` | [`tests/016-explicit-template-args-plus-deduction.cpp`](./tests/016-explicit-template-args-plus-deduction.cpp) |
| T017 | `14.8 [temp.fct.spec]` | constructor template cross-specialization conversion | `compile-pass` | [`tests/017-constructor-template-cross-specialization.cpp`](./tests/017-constructor-template-cross-specialization.cpp) |
| T018 | `12.3.2 [class.conv.fct]`, `14.8 [temp.fct.spec]` | conversion function template selection | `compile-pass` | [`tests/018-conversion-function-template-selection.cpp`](./tests/018-conversion-function-template-selection.cpp) |
| T019 | `14.8.3 [temp.over]`, `13.3 [over.match]` | non-template overload beats template when otherwise comparable | `compile-pass` | [`tests/019-template-vs-nontemplate-overload.cpp`](./tests/019-template-vs-nontemplate-overload.cpp) |
| T020 | `14.8.2 [temp.deduct]`, `14.8.3 [temp.over]` | alias-template SFINAE fallback | `compile-pass` | [`tests/020-alias-template-sfinae-fallback.cpp`](./tests/020-alias-template-sfinae-fallback.cpp) |
| T021 | `14.8.2 [temp.deduct]` | expression SFINAE via `decltype` | `compile-pass` | [`tests/021-expression-sfinae-decltype.cpp`](./tests/021-expression-sfinae-decltype.cpp) |
| T022 | `14.8.2 [temp.deduct]` | `void_t` detector for nested member presence | `compile-pass` | [`tests/022-void-t-detector.cpp`](./tests/022-void-t-detector.cpp) |
| T023 | `14.7.1 [temp.inst]` | unused dependent template body is not eagerly instantiated | `compile-pass` | [`tests/023-no-eager-instantiation-unused-body.cpp`](./tests/023-no-eager-instantiation-unused-body.cpp) |
| T027 | `14.8.3 [temp.over]` | fixed-arity function template beats pack fallback | `compile-pass` | [`tests/027-pack-fallback-partial-ordering.cpp`](./tests/027-pack-fallback-partial-ordering.cpp) |
| T028 | `14.8.2.2 [temp.deduct.funcaddr]` | overloaded function address deduction succeeds with enough context | `compile-pass` | [`tests/028-overloaded-function-address-context.cpp`](./tests/028-overloaded-function-address-context.cpp) |
| T029 | `14.1 [temp.param]`, `14.6.1 [temp.local]` | template parameter name cannot be redeclared in template scope | `compile-fail` | [`tests/029-template-parameter-shadowing.cpp`](./tests/029-template-parameter-shadowing.cpp) |
| T030 | `14.7.3 [temp.expl.spec]` | explicit specialization after prior implicit instantiation rejects | `compile-fail` | [`tests/030-explicit-specialization-after-instantiation.cpp`](./tests/030-explicit-specialization-after-instantiation.cpp) |
| T031 | `14.7.3 [temp.expl.spec]` | explicit specialization refreshes a stale primary instantiation placeholder | `compile-pass` | [`tests/031-explicit-specialization-refreshes-stale-primary-instantiation.cpp`](./tests/031-explicit-specialization-refreshes-stale-primary-instantiation.cpp) |
| T032 | `14.7.3 [temp.expl.spec]`, `14.7.1 [temp.inst]` | explicit specialization used as a template argument does not eagerly instantiate the primary | `compile-pass` | [`tests/032-explicit-specialization-type-argument-does-not-eagerly-instantiate.cpp`](./tests/032-explicit-specialization-type-argument-does-not-eagerly-instantiate.cpp) |
| T033 | `14.7.3 [temp.expl.spec]`, `14.7.1 [temp.inst]` | late explicit specialization used as a template argument does not eagerly instantiate the primary | `compile-pass` | [`tests/033-late-explicit-specialization-type-argument-does-not-eagerly-instantiate.cpp`](./tests/033-late-explicit-specialization-type-argument-does-not-eagerly-instantiate.cpp) |
| T034 | `14.7.3 [temp.expl.spec]`, `14.5.2 [temp.mem]` | explicit specialized class member definition remains callable after earlier forward use | `compile-pass` | [`tests/034-explicit-specialization-member-definition-after-forward-use.cpp`](./tests/034-explicit-specialization-member-definition-after-forward-use.cpp) |
| T035 | `14.7.3 [temp.expl.spec]`, `14.5.2 [temp.mem]` | explicit specialized out-of-class constructors replay correctly | `compile-pass` | [`tests/035-explicit-specialization-out-of-class-ctor-replay.cpp`](./tests/035-explicit-specialization-out-of-class-ctor-replay.cpp) |
| T036 | `14.8.2 [temp.deduct]`, `14.8.3 [temp.over]` | defaulted `enable_if` after array-bound deduction remains viable | `compile-pass` | [`tests/036-defaulted-enable-if-after-array-bound-deduction.cpp`](./tests/036-defaulted-enable-if-after-array-bound-deduction.cpp) |
| T037 | `14.8.1 [temp.arg.explicit]`, `14.8.2 [temp.deduct]`, `14.8.3 [temp.over]` | explicit template call with dependent alias parameter selects the right SFINAE overload | `compile-pass` | [`tests/037-explicit-template-call-dependent-alias-sfinae-overload.cpp`](./tests/037-explicit-template-call-dependent-alias-sfinae-overload.cpp) |
| T038 | `14.8.2 [temp.deduct]`, `14.8.3 [temp.over]` | empty pack remains visible to `enable_if` overload selection | `compile-pass` | [`tests/038-empty-pack-enable-if-selection.cpp`](./tests/038-empty-pack-enable-if-selection.cpp) |
| T039 | `14.8.3 [temp.over]`, `7.3.4 [namespace.udir]` | inline namespace plus using-directive preserves SFINAE overload selection | `compile-pass` | [`tests/039-inline-namespace-using-directive-sfinae-overloads.cpp`](./tests/039-inline-namespace-using-directive-sfinae-overloads.cpp) |
| T040 | `12.3.2 [class.conv.fct]`, `14.8 [temp.fct.spec]` | conversion function template participates in copy-initialization | `compile-pass` | [`tests/040-conversion-function-template-copy-init.cpp`](./tests/040-conversion-function-template-copy-init.cpp) |
| T041 | `12.3.2 [class.conv.fct]`, `14.8 [temp.fct.spec]` | conversion function template participates in function argument passing | `compile-pass` | [`tests/041-conversion-function-template-call-argument.cpp`](./tests/041-conversion-function-template-call-argument.cpp) |
| T042 | `14.7.3 [temp.expl.spec]`, `14.8 [temp.fct.spec]` | out-of-class body for explicit specialized cross-converting constructor | `compile-pass` | [`tests/042-explicit-specialization-cross-converting-ctor-body.cpp`](./tests/042-explicit-specialization-cross-converting-ctor-body.cpp) |
| T043 | `14.8 [temp.fct.spec]`, `14.7.3 [temp.expl.spec]` | cross-specialization converting constructor/operator template compile surface | `compile-pass` | [`tests/043-cross-specialization-converting-ctor-operator-template.cpp`](./tests/043-cross-specialization-converting-ctor-operator-template.cpp) |
| T047 | `14.5.5 [temp.class.spec]`, `14.8 [temp.fct.spec]` | function-type partial specialization plus functor assignment without STL | `compile-pass` | [`tests/047-function-signature-partial-specialization-functor-assignment.cpp`](./tests/047-function-signature-partial-specialization-functor-assignment.cpp) |
| T048 | `14.5.1 [temp.class]` | dependent default construction and reference return through subscript-style template API | `compile-pass` | [`tests/048-dependent-default-construction-through-template-subscript.cpp`](./tests/048-dependent-default-construction-through-template-subscript.cpp) |
| T049 | `14.6.2 [temp.dep.type]`, `14.5.7 [temp.alias]` | nested dependent alias lookup without `<iterator>` | `compile-pass` | [`tests/049-pointer-traits-alias-chain-core.cpp`](./tests/049-pointer-traits-alias-chain-core.cpp) |
| T050 | `14.5.1 [temp.class]`, `14.8 [temp.fct.spec]` | local-type iterator conversion into `const&` function parameter without STL | `compile-pass` | [`tests/050-local-constref-converting-iterator.cpp`](./tests/050-local-constref-converting-iterator.cpp) |
| T051 | `14.1 [temp.param]`, `14.3.2 [temp.arg.nontype]` | integral non-type parameter on function template | `compile-pass` | [`tests/051-integral-function-nontype-parameter.cpp`](./tests/051-integral-function-nontype-parameter.cpp) |
| T052 | `14.1 [temp.param]` | unnamed integral non-type template parameter declaration | `compile-pass` | [`tests/052-unnamed-integral-nontype-parameter.cpp`](./tests/052-unnamed-integral-nontype-parameter.cpp) |
| T053 | `14.1 [temp.param]`, `14.3.2 [temp.arg.nontype]` | integral non-type template parameter default | `compile-pass` | [`tests/053-integral-nontype-default.cpp`](./tests/053-integral-nontype-default.cpp) |
| T054 | `14.1 [temp.param]`, `14.3.2 [temp.arg.nontype]` | dependent `sizeof` in integral non-type default | `compile-pass` | [`tests/054-dependent-sizeof-template-default.cpp`](./tests/054-dependent-sizeof-template-default.cpp) |
| T055 | `14.7.3 [temp.expl.spec]` | simple function explicit specialization | `compile-pass` | [`tests/055-function-explicit-specialization-simple.cpp`](./tests/055-function-explicit-specialization-simple.cpp) |
| T056 | `14.7.3 [temp.expl.spec]` | simple class explicit specialization | `compile-pass` | [`tests/056-class-explicit-specialization-simple.cpp`](./tests/056-class-explicit-specialization-simple.cpp) |
| T057 | `14.7.3 [temp.expl.spec]` | out-of-class static member definition on explicit specialization | `compile-pass` | [`tests/057-static-member-explicit-specialization-char16.cpp`](./tests/057-static-member-explicit-specialization-char16.cpp) |
| T058 | `14.3.2 [temp.arg.nontype]` | `constexpr` integral expression as template argument | `compile-pass` | [`tests/058-constexpr-integral-template-argument.cpp`](./tests/058-constexpr-integral-template-argument.cpp) |
| T059 | `14.3.2 [temp.arg.nontype]` | integral constant static value binding | `compile-pass` | [`tests/059-integral-constant-static-value.cpp`](./tests/059-integral-constant-static-value.cpp) |
| T060 | `14.3.2 [temp.arg.nontype]` | inherited integral constant static member value | `compile-pass` | [`tests/060-inherited-static-member-value.cpp`](./tests/060-inherited-static-member-value.cpp) |
| T061 | `14.3.3 [temp.arg.template]` | unnamed template-template parameter | `compile-pass` | [`tests/061-unnamed-template-template-parameter.cpp`](./tests/061-unnamed-template-template-parameter.cpp) |
| T062 | `14.1 [temp.param]`, `14.5.1 [temp.class]` | forward declaration and later definition may rename template parameters | `compile-pass` | [`tests/062-template-forward-definition-parameter-rename.cpp`](./tests/062-template-forward-definition-parameter-rename.cpp) |
| T063 | `14.5.3 [temp.variadic]`, `14.8.2 [temp.deduct]` | variadic function-template forwards whole pack into another call | `compile-pass` | [`tests/063-function-template-pack-forward-call.cpp`](./tests/063-function-template-pack-forward-call.cpp) |
| T064 | `14.5.4 [temp.friend]`, `14.6.4 [temp.dep.res]` | hidden friend template operator found by ADL | `compile-pass` | [`tests/064-hidden-friend-template-operator-adl.cpp`](./tests/064-hidden-friend-template-operator-adl.cpp) |
| T065 | `14.8.3 [temp.over]` | const-pointer partial ordering beats variadic fallback | `compile-pass` | [`tests/065-function-template-partial-order-const-pointer.cpp`](./tests/065-function-template-partial-order-const-pointer.cpp) |
| T066 | `14.5.3 [temp.variadic]`, `14.8.2 [temp.deduct]` | simple function-template pack call | `compile-pass` | [`tests/066-function-template-pack-call.cpp`](./tests/066-function-template-pack-call.cpp) |
| T067 | `14.5.3 [temp.variadic]`, `14.8.2 [temp.deduct]` | leading fixed parameter plus function-template pack call | `compile-pass` | [`tests/067-function-template-leading-fixed-and-pack-call.cpp`](./tests/067-function-template-leading-fixed-and-pack-call.cpp) |
| T068 | `14.8.2 [temp.deduct]` | array argument deduces through pointer parameter | `compile-pass` | [`tests/068-function-template-array-to-pointer-deduction.cpp`](./tests/068-function-template-array-to-pointer-deduction.cpp) |
| T069 | `14.5.2 [temp.mem]`, `14.5.3 [temp.variadic]`, `14.8.1 [temp.arg.explicit]` | member-template explicit pack forward call | `compile-pass` | [`tests/069-member-template-explicit-pack-forward-call.cpp`](./tests/069-member-template-explicit-pack-forward-call.cpp) |
| T070 | `14.5.4 [temp.friend]`, `14.6.4 [temp.dep.res]` | hidden friend template ordinary-call ADL | `compile-pass` | [`tests/070-hidden-friend-template-call-adl.cpp`](./tests/070-hidden-friend-template-call-adl.cpp) |
| T071 | `14.1 [temp.param]`, `14.8.2 [temp.deduct]` | nested defaulted class-template arguments deduce through outer specialization | `compile-pass` | [`tests/071-defaulted-nested-class-template-deduction.cpp`](./tests/071-defaulted-nested-class-template-deduction.cpp) |
| T072 | `14.5.2 [temp.mem]`, `14.8 [temp.fct.spec]` | const member-function template overload wins inside const call | `compile-pass` | [`tests/072-const-member-function-template-overload.cpp`](./tests/072-const-member-function-template-overload.cpp) |
| T073 | `14.7.1 [temp.inst]`, `14.8 [temp.fct.spec]` | repeated implicit function-template calls reuse the same instantiation | `compile-pass` | [`tests/073-repeated-implicit-function-template-call.cpp`](./tests/073-repeated-implicit-function-template-call.cpp) |
| T074 | `14.1 [temp.param]`, `14.3.2 [temp.arg.nontype]` | dependent logical-or in integral non-type default | `compile-pass` | [`tests/074-dependent-logical-or-template-default.cpp`](./tests/074-dependent-logical-or-template-default.cpp) |
| T075 | `14.1 [temp.param]` | non-type template parameter pack declaration | `compile-pass` | [`tests/075-nontype-template-parameter-pack.cpp`](./tests/075-nontype-template-parameter-pack.cpp) |
| T076 | `14.3.2 [temp.arg.nontype]`, `14.8.2 [temp.deduct]` | dependent array bound in function-template declaration | `compile-pass` | [`tests/076-dependent-array-bound-function-template.cpp`](./tests/076-dependent-array-bound-function-template.cpp) |
| T077 | `14.7.3 [temp.expl.spec]`, `14.8.1 [temp.arg.explicit]` | deduced call dispatches to explicit function specialization | `compile-pass` | [`tests/077-function-explicit-specialization-deduced.cpp`](./tests/077-function-explicit-specialization-deduced.cpp) |
| T078 | `14.3.2 [temp.arg.nontype]`, `7 [dcl.pre]` | template `static_assert` over integral constant expression | `compile-pass` | [`tests/078-template-static-assert.cpp`](./tests/078-template-static-assert.cpp) |
| T079 | `14.6 [temp.dep]`, `7 [dcl.pre]` | dependent `static_assert` defers until instantiation | `compile-pass` | [`tests/079-dependent-static-assert-defers.cpp`](./tests/079-dependent-static-assert-defers.cpp) |
| T080 | `14.5.7 [temp.alias]`, `14.6.2 [temp.dep.type]` | qualified member alias template through dependent owner | `compile-pass` | [`tests/080-qualified-member-alias-template.cpp`](./tests/080-qualified-member-alias-template.cpp) |
| T081 | `14.5.6 [temp.var]`, `14.5.5 [temp.class.spec]` | variable-template specialization selection | `compile-pass` | [`tests/081-variable-template-specialization-selection.cpp`](./tests/081-variable-template-specialization-selection.cpp) |
| T082 | `14.7.2 [temp.explicit]` | extern-template class declaration | `compile-pass` | [`tests/082-extern-template-class-declaration.cpp`](./tests/082-extern-template-class-declaration.cpp) |
| T083 | `14.7.2 [temp.explicit]` | extern-template member-function declaration | `compile-pass` | [`tests/083-extern-template-member-function-declaration.cpp`](./tests/083-extern-template-member-function-declaration.cpp) |
| T084 | `14.7.2 [temp.explicit]` | extern-template static-data declaration | `compile-pass` | [`tests/084-extern-template-static-data-declaration.cpp`](./tests/084-extern-template-static-data-declaration.cpp) |
| T085 | `14.8.2 [temp.deduct]`, `14.8.3 [temp.over]` | `void_t` + `decltype` function-call partial specialization | `compile-pass` | [`tests/085-void-t-decltype-function-template-call-partial-specialization.cpp`](./tests/085-void-t-decltype-function-template-call-partial-specialization.cpp) |
| T086 | `14.3.2 [temp.arg.nontype]` | `long long` integral non-type template argument | `compile-pass` | [`tests/086-long-long-nontype-template-argument.cpp`](./tests/086-long-long-nontype-template-argument.cpp) |
| T087 | `14.8.1 [temp.arg.explicit]`, `14.8.3 [temp.over]` | unnamed `enable_if` default on template-id return declaration | `compile-pass` | [`tests/087-unnamed-enable-if-default-template-id-return.cpp`](./tests/087-unnamed-enable-if-default-template-id-return.cpp) |
| T088 | `14.8.2 [temp.deduct]` | function-template array-bound deduction | `compile-pass` | [`tests/088-function-template-array-bound-deduction.cpp`](./tests/088-function-template-array-bound-deduction.cpp) |
| T089 | `12.3.2 [class.conv.fct]`, `14.5.2 [temp.mem]` | out-of-class conversion operator definition | `compile-pass` | [`tests/089-out-of-class-conversion-operator-definition.cpp`](./tests/089-out-of-class-conversion-operator-definition.cpp) |
| T090 | hosted sentinel over `std::function` | callable assignment into `std::function` | `compile-pass` | [`tests/090-std-function-lambda-assignment.cpp`](./tests/090-std-function-lambda-assignment.cpp) |
| T091 | hosted sentinel over `std::map` | `std::map::operator[]` piecewise construction path | `compile-pass` | [`tests/091-std-map-subscript-piecewise.cpp`](./tests/091-std-map-subscript-piecewise.cpp) |
| T092 | hosted sentinel over `std::iterator_traits` | alias-heavy iterator-traits value type lookup | `compile-pass` | [`tests/092-iterator-traits-alias-chain.cpp`](./tests/092-iterator-traits-alias-chain.cpp) |
| T093 | hosted sentinel over `std::getline` | friend/lambda-heavy string extraction path in `<sstream>` | `compile-pass` | [`tests/093-std-getline-hosted-sentinel.cpp`](./tests/093-std-getline-hosted-sentinel.cpp) |
| T094 | hosted sentinel over `basic_istream` | extraction operator path with static mask access in `<sstream>` | `compile-pass` | [`tests/094-istream-string-extraction-hosted-sentinel.cpp`](./tests/094-istream-string-extraction-hosted-sentinel.cpp) |
| T095 | hosted sentinel over nested callbacks and containers | inline lambda/export-closure path over `set`, `vector`, and `ostringstream` | `compile-pass` | [`tests/095-inline-lambda-export-closure-hosted-sentinel.cpp`](./tests/095-inline-lambda-export-closure-hosted-sentinel.cpp) |
| T100 | `14.7.2 [temp.explicit]` | extern-template constructor declaration | `compile-pass` | [`tests/100-extern-template-constructor-declaration.cpp`](./tests/100-extern-template-constructor-declaration.cpp) |
| T101 | `14.7.2 [temp.explicit]` | extern-template operator-function declaration | `compile-pass` | [`tests/101-extern-template-operator-function-declaration.cpp`](./tests/101-extern-template-operator-function-declaration.cpp) |
| T102 | `14.5.5 [temp.class.spec]`, `14.5.6 [temp.var]` | shared partial-specialization variable selection | `compile-pass` | [`tests/102-shared-partial-specialization-variable-selection.cpp`](./tests/102-shared-partial-specialization-variable-selection.cpp) |
| T103 | `14.5.5 [temp.class.spec]` | partial specialization uses primary default argument | `compile-pass` | [`tests/103-partial-specialization-uses-primary-default-argument.cpp`](./tests/103-partial-specialization-uses-primary-default-argument.cpp) |
| T104 | `14.8.1 [temp.arg.explicit]` | unnamed non-type default on template-id return declaration | `compile-pass` | [`tests/104-unnamed-nontype-default-template-id-return.cpp`](./tests/104-unnamed-nontype-default-template-id-return.cpp) |
| T105 | `14.1 [temp.param]`, `14.5.1 [temp.class]` | namespace-qualified forward definition may rename template parameters | `compile-pass` | [`tests/105-namespace-template-forward-definition-parameter-rename.cpp`](./tests/105-namespace-template-forward-definition-parameter-rename.cpp) |
| T106 | `14.8 [temp.fct.spec]` | constructor template converts from const reference to other specialization | `compile-pass` | [`tests/106-constructor-template-const-ref-conversion.cpp`](./tests/106-constructor-template-const-ref-conversion.cpp) |
| T107 | `14.1 [temp.param]`, `14.5.1 [temp.class]` | class-template default arguments preserve template scope in later static member definition | `compile-pass` | [`tests/107-class-template-default-arg-preserves-template-scope.cpp`](./tests/107-class-template-default-arg-preserves-template-scope.cpp) |
| T108 | `14.1 [temp.param]`, `14.3.1 [temp.arg.type]` | default template argument preserves const-pointer alias shape | `compile-pass` | [`tests/108-default-template-arg-const-pointer-alias.cpp`](./tests/108-default-template-arg-const-pointer-alias.cpp) |
| T109 | `14.6.1 [temp.local]`, `14.8.2 [temp.deduct]` | local lambda body calls function template over local concrete type | `compile-pass` | [`tests/109-local-lambda-function-template-concrete-type.cpp`](./tests/109-local-lambda-function-template-concrete-type.cpp) |
| T110 | `14.8 [temp.fct.spec]`, `7.3.1 [namespace.def]` | inline-namespace forward declarations for `declval` and `initializer_list` compile | `compile-pass` | [`tests/110-inline-namespace-forward-initlist-declaration.cpp`](./tests/110-inline-namespace-forward-initlist-declaration.cpp) |
| T111 | `14.1 [temp.param]`, `14.3.2 [temp.arg.nontype]` | dependent non-type template parameter type on integral constant | `compile-pass` | [`tests/111-dependent-nontype-template-parameter-type.cpp`](./tests/111-dependent-nontype-template-parameter-type.cpp) |
| T112 | `14.1 [temp.param]`, `14.6.2 [temp.dep.type]` | dependent qualified type names a non-type template parameter type | `compile-pass` | [`tests/112-dependent-nontype-template-type.cpp`](./tests/112-dependent-nontype-template-type.cpp) |
| T113 | `14.6.2 [temp.dep.type]`, `14.8 [temp.fct.spec]` | dependent qualified return type through `tuple_element` | `compile-pass` | [`tests/113-dependent-qualified-return-type.cpp`](./tests/113-dependent-qualified-return-type.cpp) |
| T114 | `14.2 [temp.names]`, `14.6.2 [temp.dep.type]`, `14.5.7 [temp.alias]` | qualified nested template-id with `template` disambiguator | `compile-pass` | [`tests/114-qualified-nested-template-id.cpp`](./tests/114-qualified-nested-template-id.cpp) |
| T115 | `14.2 [temp.names]`, `14.8.1 [temp.arg.explicit]`, `14.8.2 [temp.deduct]` | qualified namespace function-template call | `compile-pass` | [`tests/115-qualified-function-template-call.cpp`](./tests/115-qualified-function-template-call.cpp) |
| T116 | `14.6.2 [temp.dep.type]`, `14.8.2 [temp.deduct]` | inline-namespace qualified `decltype` lookup | `compile-pass` | [`tests/116-inline-namespace-qualified-decltype-lookup.cpp`](./tests/116-inline-namespace-qualified-decltype-lookup.cpp) |
| T117 | `14.8.1 [temp.arg.explicit]`, `14.6.2 [temp.dep.type]` | explicit template call through dependent alias conversion | `compile-pass` | [`tests/117-explicit-template-call-dependent-alias-conversion.cpp`](./tests/117-explicit-template-call-dependent-alias-conversion.cpp) |
| T118 | `14.8.2.5 [temp.deduct.type]` | qualified parameter is a non-deduced context | `compile-pass` | [`tests/118-nondeduced-qualified-parameter-deduction.cpp`](./tests/118-nondeduced-qualified-parameter-deduction.cpp) |
| T119 | `14.8.2.5 [temp.deduct.type]` | secondary parameter still deduces through non-deduced first parameter | `compile-pass` | [`tests/119-nondeduced-context-secondary-parameter.cpp`](./tests/119-nondeduced-context-secondary-parameter.cpp) |
| T120 | `14.5.2 [temp.mem]`, `14.6.1 [temp.local]`, `14.8 [temp.fct.spec]` | local type through constructor-template member typedef conversion | `compile-pass` | [`tests/120-local-constructor-template-member-typedef.cpp`](./tests/120-local-constructor-template-member-typedef.cpp) |
| T121 | `14.5.2 [temp.mem]`, `14.6.1 [temp.local]`, `14.8 [temp.fct.spec]` | local member call triggers constructor-template instantiation | `compile-pass` | [`tests/121-local-member-call-constructor-template-instantiation.cpp`](./tests/121-local-member-call-constructor-template-instantiation.cpp) |
| T122 | `14.1 [temp.param]`, `14.8 [temp.fct.spec]`, `7.3.1 [namespace.def]` | inline-namespace function-template parameter scope | `compile-pass` | [`tests/122-inline-namespace-function-template-parameter-scope.cpp`](./tests/122-inline-namespace-function-template-parameter-scope.cpp) |
| T123 | `14.2 [temp.names]`, `14.6.4 [temp.dep.res]` | dependent member template call with explicit `template` disambiguator | `compile-pass` | [`tests/123-dependent-member-template-call.cpp`](./tests/123-dependent-member-template-call.cpp) |
| T124 | `14.6.2 [temp.dep]`, `14.6.4 [temp.dep.res]` | dependent base member access through `this->` | `compile-pass` | [`tests/124-dependent-base-member-this.cpp`](./tests/124-dependent-base-member-this.cpp) |
| T125 | `14.6.2 [temp.dep.type]`, `14.3.2 [temp.arg.nontype]` | qualified static member template value in dependent context | `compile-pass` | [`tests/125-qualified-static-member-template-value.cpp`](./tests/125-qualified-static-member-template-value.cpp) |
| T126 | `14.6.2 [temp.dep.type]`, `14.5.2 [temp.mem]` | dependent qualified member type re-exported through nested aliases | `compile-pass` | [`tests/126-dependent-qualified-member-type-reexport.cpp`](./tests/126-dependent-qualified-member-type-reexport.cpp) |
| T127 | `14.6.2 [temp.dep.type]`, `14.8 [temp.fct.spec]` | qualified dependent type-construction expression in templated call | `compile-pass` | [`tests/127-typename-qualified-type-construction-expression.cpp`](./tests/127-typename-qualified-type-construction-expression.cpp) |

## Notes

- This bank deliberately stays within an N3485/C++11 language surface.
- The intended assignment slices are:
  - `pa18`: core first-tier templates from the non-hosted range
  - `pa19`: NTTP and explicit-specialization cases
  - `pa21`: alias/variable/partial-specialization/entity-graph cases
  - `pa22`: deduction/substitution/SFINAE cases
- Hosted sentinel tests are late-stage confidence checks, not first-line
  debugging or reduction targets. They are intentionally numbered in the
  `090+` range to keep them visually separate from the core-language tests that
  should not be first-line grading inputs for the early slices.
- The `090`-`095` block is a hosted pocket. Later non-hosted promotions may
  continue at `100+` without changing that interpretation.
- If a hosted sentinel fails, reduce it back into the smallest matching
  core-language rule and add that reduction if it does not already exist.
