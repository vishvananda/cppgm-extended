# Template Bank Assignment Slices

This document maps the template validation bank onto the assignment milestones
that actually own template behavior.

The prose below is mirrored by machine-readable manifests under
[`slices/`](./slices/), which are the intended inputs for future harness
replacement work.

It follows the ownership stated in the assignment READMEs:

- `pa17`: polymorphism only, not owned by the template bank
- `pa18`: first-tier templates
- `pa19`: C++11 integral NTTPs and explicit specialization
- `pa21`: specialization/entity graph and specialization selection
- `pa22`: deduction, substitution, and SFINAE completion

## Oracle Lanes

The intended landing model has two lanes:

- `test-strict`
  - the direct assignment contract
  - LowIR compare for positive/negative assignment-owned cases
  - may include compile-pass / compile-fail checks where LowIR is not the
    cleanest proof

- `strict`
  - the stricter semantic lane
  - compare `cppgm++ --emit-lowir --template-log <path>` against the stored
    clang witness oracle
  - can also require successful compilation when the test is positive

The important rule is:

- `test-strict` proves the owned boundary works
- `strict` proves the template decisions match the oracle

## PA17

`pa17` should keep its existing suite.

The README makes templates explicitly out of scope here. The template bank
should not replace `pa17` tests.

## PA18 Slice

`pa18` owns first-tier templates:

- class templates
- function templates
- type parameters and template-template parameters
- defaults over that subset
- basic deduction
- on-demand instantiation
- member templates / templated operators in the supported object model
- out-of-class nested member definitions inside supported template owners

### Bank Tests

These bank tests fit the `pa18` ownership boundary:

- `001` template-template parameter basic
- `002` template-template parameter arity mismatch
- `005` type equivalence through defaults
- `006` member function template out-of-class definition
- `007` member class template out-of-class definition
- `008` friend function template inside class template
- `009` non-dependent name binding
- `010` dependent ADL at point of instantiation
- `011` variadic base pack expansion
- `016` explicit template arguments plus deduction
- `017` constructor template cross-specialization conversion
- `019` template vs non-template overload
- `023` no eager instantiation of unused dependent body
- `027` fixed-arity template beats pack fallback
- `048` dependent default construction through template subscript API
- `050` local-type conversion into `const&` template parameter
- `061` unnamed template-template parameter
- `062` template forward definition parameter rename
- `063` function-template pack forward call
- `064` hidden friend template operator ADL
- `065` function-template partial ordering on const pointer
- `066` simple function-template pack call
- `067` function-template leading fixed parameter plus pack call
- `068` array-to-pointer deduction
- `069` member-template explicit pack forward call
- `070` hidden friend template call ADL
- `071` defaulted nested class-template deduction
- `072` const member-function template overload
- `073` repeated implicit function-template call
- `105` namespace-qualified forward definition parameter rename
- `106` constructor template const-reference conversion
- `107` class-template default argument preserves template scope
- `108` default template argument over const-pointer alias
- `109` local lambda calling function template on local concrete type
- `110` inline-namespace forward `initializer_list` declarations
- `120` local constructor-template member-typedef conversion
- `121` local member call triggering constructor-template instantiation

### Ownership Note

`214-hidden-friend-adl.t` should not be promoted into the template bank.

It is a non-template hidden-friend / ADL test, and `pa15` already owns that
ordinary class/object-model surface. If that case moves during test-bank
consolidation, it should land with the non-template class/object suite rather
than with `pa18`.

## PA19 Slice

`pa19` extends `pa18` with:

- integral NTTPs and integral NTTP packs
- integral constant-expression template arguments
- explicit specialization of supported class/function templates
- `static_assert` over the supported integral constant subset

### Bank Tests

These bank tests fit the `pa19` ownership boundary:

- `051` integral non-type parameter on function template
- `052` unnamed integral non-type template parameter declaration
- `053` integral non-type template parameter default
- `054` dependent `sizeof` in integral non-type default
- `055` simple function explicit specialization
- `056` simple class explicit specialization
- `057` out-of-class static member definition on explicit specialization
- `058` `constexpr` integral expression as template argument
- `059` integral constant static value
- `060` inherited static member value
- `074` dependent logical-or in integral non-type default
- `075` non-type template parameter pack declaration
- `076` dependent array-bound function-template declaration
- `077` deduced call selects explicit function specialization
- `078` template `static_assert` over integral constant expression
- `079` dependent `static_assert` defers until instantiation
- `086` `long long` integral non-type template argument
- `111` dependent non-type template parameter type
- `112` dependent non-type template type
- `113` dependent qualified return type
- `114` qualified nested template-id
- `115` qualified function-template call
- `125` qualified static member template value
- `126` dependent qualified member type reexport
- `127` qualified dependent type-construction expression
- `014` explicit specialization of member function
- `015` explicit specialization of static data member
- `030` explicit specialization after prior implicit instantiation rejects
- `031` explicit specialization refreshes stale primary
- `032` specialization as type argument avoids eager primary instantiation
- `033` late specialization as type argument avoids eager primary instantiation

### Bank Tests Outside Current PA19 Ownership

These are valid C++11 NTTP cases, but they are broader than the current `pa19`
README, which only promises integral NTTPs:

- `003` function-pointer NTTP
- `004` reference NTTP

They should stay in the full bank, but they should not be first-line `pa19`
grading inputs unless the README boundary is widened.

The remaining `pa19` gap is now narrower. The bank covers the basic
integral-NTTP / explicit-specialization / `static_assert` spine well, but it is
still lighter on a few dependent qualified/value lookup shapes than the local
`pa19` suite.

## PA21 Slice

`pa21` owns the specialization/entity graph:

- alias templates
- variable templates
- class/function partial specialization
- specialization selection
- explicit-instantiation ownership
- constructor/member-template specialization ownership

### Bank Tests

These bank tests fit the `pa21` ownership boundary:

- `012` explicit instantiation of function template
- `013` explicit instantiation of class template
- `034` specialized member definition after forward use
- `035` specialized out-of-class constructor replay
- `042` specialized cross-converting constructor body
- `043` cross-specialization converting constructor/operator compile surface
- `047` function-signature partial specialization plus functor assignment
- `049` nested dependent alias lookup core
- `080` qualified member alias template
- `081` variable-template specialization selection
- `082` extern-template class declaration
- `083` extern-template member-function declaration
- `084` extern-template static-data declaration
- `100` extern-template constructor declaration
- `101` extern-template operator-function declaration
- `102` shared partial-specialization variable selection
- `103` partial specialization uses primary default argument

### Promotion Candidates Still Missing From The Bank

The bank already has a coherent specialization-ownership core. The most useful
future promotions here should focus on genuinely new alias/variable/partial
ownership shapes rather than direct duplicates of the existing extern-template
coverage now already represented by `082`/`083`/`084`/`100`/`101`.

## PA22 Slice

`pa22` owns the remainder of template semantics:

- full deduction
- substitution and candidate dropping
- `enable_if`, `void_t`, detected-idiom behavior
- non-deduced contexts
- explicit template-id deduction edges
- remaining no-eager-instantiation and dependent-call timing

### Bank Tests

These bank tests fit the `pa22` ownership boundary:

- `018` conversion function template selection
- `020` alias-template SFINAE fallback
- `021` expression SFINAE through `decltype`
- `022` `void_t` detector
- `028` overloaded function address deduction
- `036` defaulted `enable_if` after array-bound deduction
- `037` explicit template call with dependent alias SFINAE overload
- `038` empty-pack `enable_if` selection
- `039` inline namespace + using-directive SFINAE
- `040` conversion function template copy-init
- `041` conversion function template function-argument
- `085` `void_t` + `decltype` function-call partial specialization
- `087` unnamed `enable_if` default template-id return
- `088` function-template array-bound deduction
- `089` out-of-class conversion operator definition
- `104` unnamed non-type default template-id return
- `116` inline-namespace qualified `decltype` lookup
- `117` explicit template call through dependent alias conversion
- `118` non-deduced qualified-parameter deduction
- `119` non-deduced secondary-parameter deduction
- `122` inline-namespace function-template parameter scope
- `123` dependent member template call
- `124` dependent base member access through `this->`

### Promotion Candidates Still Missing From The Bank

The current bank already covers the direct local PA22 deduction subset well.
The remaining useful promotions are now narrower and should focus on any future
local tests that introduce a genuinely new non-deduced-context, dependent-call,
or candidate-drop shape rather than duplicating existing bank coverage.

## Hosted Confidence Range

The `090+` tests are hosted confidence checks, not primary assignment slices:

- `090` `std::function`
- `091` `std::map`
- `092` `std::iterator_traits`
- `093` `std::getline`
- `094` `istream` extraction
- `095` nested callbacks/containers/export-closure

These belong after the non-hosted slices are stable. They should usually be
used as late confidence tests and reduction feeders, not first-line grading
inputs.
