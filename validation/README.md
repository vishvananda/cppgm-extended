# Validation Candidate Set From N3485

This directory is a seed bank of additional language-validation targets derived from the
local `n3485.txt` working draft file, beyond the items already tracked in
[`docs/spec-conformance-audit.md`](../docs/spec-conformance-audit.md).

It is intentionally narrower than the whole draft. The goal is to keep a practical list of
core-language features that:

- are plausible sources of semantic regressions in the current compiler,
- can be reduced to small self-contained tests,
- and can later be promoted into the earliest natural PA test suite.

The working process for promotion is documented in
[`INGEST_PROCESS.md`](./INGEST_PROCESS.md), and the sequential status tracker is
[`INGEST_TRACKER.md`](./INGEST_TRACKER.md).

## Validation Kinds

- `run-pass`: should compile and run with exit status `0`
- `compile-pass`: should compile cleanly; any `main` present is only a stub
- `compile-fail`: should be rejected; the file documents the intended failure

## Candidate Matrix

| ID | N3485 focus | Feature | Kind | Test |
|---|---|---|---|---|
| V001 | `3.4.2 [basic.lookup.argdep]`, `11.3 [class.friend]` | hidden friend found by ADL | `run-pass` | [`tests/001-hidden-friend-adl.cpp`](./tests/001-hidden-friend-adl.cpp) |
| V002 | `7.3.3 [namespace.udecl]`, `10.2 [class.member.lookup]` | `using` restores base overload set | `run-pass` | [`tests/002-using-base-overload-set.cpp`](./tests/002-using-base-overload-set.cpp) |
| V003 | `12.9 [class.inhctor]` | inheriting constructors | `run-pass` | [`tests/003-inheriting-constructors.cpp`](./tests/003-inheriting-constructors.cpp) |
| V004 | `12.6.2 [class.base.init]` | delegating constructors | `run-pass` | [`tests/004-delegating-constructor.cpp`](./tests/004-delegating-constructor.cpp) |
| V005 | `9.2 [class.mem]`, `12.6.2 [class.base.init]` | in-class member initializers | `run-pass` | [`tests/005-in-class-member-initializer.cpp`](./tests/005-in-class-member-initializer.cpp) |
| V006 | ref-qualifier wording in `8.3.5` and overload resolution in `13.3` | ref-qualified member overload selection | `run-pass` | [`tests/006-ref-qualified-member-overload.cpp`](./tests/006-ref-qualified-member-overload.cpp) |
| V007 | `2.14.7 [lex.nullptr]`, `4.10 [conv.ptr]`, `13.3 [over.match]` | `nullptr` overload resolution | `run-pass` | [`tests/007-nullptr-overload-resolution.cpp`](./tests/007-nullptr-overload-resolution.cpp) |
| V008 | `7.1.6.2 [dcl.type.simple]` | `decltype` lvalue/prvalue distinction | `run-pass` | [`tests/008-decltype-value-category.cpp`](./tests/008-decltype-value-category.cpp) |
| V009 | `6.5.4 [stmt.ranged]` | range-for over member `begin`/`end` | `run-pass` | [`tests/009-range-for-member-begin-end.cpp`](./tests/009-range-for-member-begin-end.cpp) |
| V010 | `5.1.2 [expr.prim.lambda]` | lambda capture and `mutable` semantics | `run-pass` | [`tests/010-lambda-capture-mutable.cpp`](./tests/010-lambda-capture-mutable.cpp) |
| V011 | `14.5.7 [temp.alias]` | alias template substitution | `compile-pass` | [`tests/011-alias-template-substitution.cpp`](./tests/011-alias-template-substitution.cpp) |
| V012 | `14.6.2 [temp.dep]` | dependent `typename` use | `compile-pass` | [`tests/012-dependent-typename.cpp`](./tests/012-dependent-typename.cpp) |
| V013 | `14.2 [temp.names]`, `14.6.4 [temp.dep.res]` | dependent member template call via `template` disambiguator | `run-pass` | [`tests/013-dependent-member-template-call.cpp`](./tests/013-dependent-member-template-call.cpp) |
| V014 | `14.1 [temp.param]`, `14.5 [temp.decl]` | default template argument merge across redeclarations | `run-pass` | [`tests/014-default-template-argument-merge.cpp`](./tests/014-default-template-argument-merge.cpp) |
| V015 | `14.5.5 [temp.class.spec]` | class template partial specialization selection | `run-pass` | [`tests/015-class-partial-specialization-selection.cpp`](./tests/015-class-partial-specialization-selection.cpp) |
| V016 | `7.2 [dcl.enum]` | scoped enum with fixed underlying type | `run-pass` | [`tests/016-scoped-enum-underlying-type.cpp`](./tests/016-scoped-enum-underlying-type.cpp) |
| V017 | `7.1.5 [dcl.constexpr]` | constexpr function and constructor constant evaluation | `compile-pass` | [`tests/017-constexpr-function-and-constructor.cpp`](./tests/017-constexpr-function-and-constructor.cpp) |
| V018 | `10.3 [class.virtual]` | virtual override dispatch | `run-pass` | [`tests/018-virtual-override-dispatch.cpp`](./tests/018-virtual-override-dispatch.cpp) |
| V019 | `14.6.2 [temp.dep]`, `14.6.4 [temp.dep.res]` | dependent base member via `this->` | `run-pass` | [`tests/019-dependent-base-member-this.cpp`](./tests/019-dependent-base-member-this.cpp) |
| V020 | `14.6.2 [temp.dep]`, `7.3.3 [namespace.udecl]` | dependent-base `using` restores overload set | `run-pass` | [`tests/020-dependent-base-using-overload.cpp`](./tests/020-dependent-base-using-overload.cpp) |
| V021 | `14.6.2 [temp.dep]` | current-instantiation qualified type name | `compile-pass` | [`tests/021-current-instantiation-qualified-type.cpp`](./tests/021-current-instantiation-qualified-type.cpp) |
| V022 | `3.4.2 [basic.lookup.argdep]`, `14.6.4 [temp.dep.res]` | hidden friend ADL for class templates | `run-pass` | [`tests/022-template-hidden-friend-adl.cpp`](./tests/022-template-hidden-friend-adl.cpp) |
| V023 | `14.2 [temp.names]`, `14.6.4 [temp.dep.res]` | dependent member template call with `template` disambiguator | `run-pass` | [`tests/023-dependent-member-template-disambiguator.cpp`](./tests/023-dependent-member-template-disambiguator.cpp) |
| V024 | `4.11 [conv.mem]` | base-to-derived pointer-to-data-member conversion | `run-pass` | [`tests/024-pointer-to-member-data-conversion.cpp`](./tests/024-pointer-to-member-data-conversion.cpp) |
| V025 | `4.11 [conv.mem]` | base-to-derived pointer-to-member-function conversion | `run-pass` | [`tests/025-pointer-to-member-function-conversion.cpp`](./tests/025-pointer-to-member-function-conversion.cpp) |
| V026 | `4.5 [conv.prom]`, `13.3 [over.match]` | unscoped enum promotion in overload resolution | `run-pass` | [`tests/026-unscoped-enum-promotion-overload.cpp`](./tests/026-unscoped-enum-promotion-overload.cpp) |
| V027 | `10.3 [class.virtual]` | covariant return through virtual dispatch | `run-pass` | [`tests/027-covariant-return-override.cpp`](./tests/027-covariant-return-override.cpp) |
| V028 | `8.5.1 [dcl.init.aggr]` | aggregate brace elision in nested aggregates | `run-pass` | [`tests/028-aggregate-brace-elision.cpp`](./tests/028-aggregate-brace-elision.cpp) |
| V029 | `8.5.4 [dcl.init.list]` | direct-list-init may use an explicit constructor | `run-pass` | [`tests/029-direct-list-init-explicit-ctor.cpp`](./tests/029-direct-list-init-explicit-ctor.cpp) |
| V030 | `14.5.4 [temp.friend]` | friend function template grants specialization access | `run-pass` | [`tests/030-friend-function-template-access.cpp`](./tests/030-friend-function-template-access.cpp) |
| V031 | `14.8.2.1 [temp.deduct.call]`, `8.3.2` | forwarding-reference deduction distinguishes lvalues and rvalues | `run-pass` | [`tests/031-forwarding-reference-deduction.cpp`](./tests/031-forwarding-reference-deduction.cpp) |
| V032 | `14.8.2.5 [temp.deduct.type]` | non-deduced context may be satisfied by another parameter | `run-pass` | [`tests/032-nondeduced-context-secondary-parameter.cpp`](./tests/032-nondeduced-context-secondary-parameter.cpp) |
| V033 | `14.8.2.1 [temp.deduct.call]` | array bound deduced through reference parameter | `run-pass` | [`tests/033-array-reference-deduction.cpp`](./tests/033-array-reference-deduction.cpp) |
| V034 | `14.8.2.1 [temp.deduct.call]` | function type deduced through function-reference parameter | `run-pass` | [`tests/034-function-reference-deduction.cpp`](./tests/034-function-reference-deduction.cpp) |
| V035 | `14.8.2.4 [temp.deduct.partial]`, `13.3 [over.match]` | partial ordering prefers non-const lvalue-reference template | `run-pass` | [`tests/035-partial-ordering-ref-vs-const-ref.cpp`](./tests/035-partial-ordering-ref-vs-const-ref.cpp) |
| V036 | `14.8.2.4 [temp.deduct.partial]` | pointer-pattern template is more specialized than value template | `run-pass` | [`tests/036-partial-ordering-pointer-vs-value.cpp`](./tests/036-partial-ordering-pointer-vs-value.cpp) |
| V037 | `14.8.2.1 [temp.deduct.call]` | pointer deduction allows qualification conversion on pointee type | `run-pass` | [`tests/037-pointer-qualification-deduction.cpp`](./tests/037-pointer-qualification-deduction.cpp) |
| V038 | `14.8.2.1 [temp.deduct.call]` | deduction accepts derived-to-base simple-template-id binding | `run-pass` | [`tests/038-derived-base-template-deduction.cpp`](./tests/038-derived-base-template-deduction.cpp) |
| V039 | `14.8.2.1 [temp.deduct.call]` | overload set with a unique match participates in deduction | `run-pass` | [`tests/039-overload-set-unique-deduction.cpp`](./tests/039-overload-set-unique-deduction.cpp) |
| V040 | `8.4.3 [dcl.fct.def.delete]`, `12.8 [class.copy]` | deleted copy plus defaulted move yields move-only construction | `run-pass` | [`tests/040-moveonly-defaulted-move.cpp`](./tests/040-moveonly-defaulted-move.cpp) |
| V041 | `12.8 [class.copy]` | user-declared copy constructor suppresses implicit move constructor | `run-pass` | [`tests/041-copy-suppresses-implicit-move.cpp`](./tests/041-copy-suppresses-implicit-move.cpp) |
| V042 | `8.4.3 [dcl.fct.def.delete]`, `12.8 [class.copy]` | move-only type supports defaulted move assignment | `run-pass` | [`tests/042-moveonly-defaulted-move-assignment.cpp`](./tests/042-moveonly-defaulted-move-assignment.cpp) |
| V043 | `5.1.2 [expr.prim.lambda]` | explicit `[this]` capture permits member access in lambda body | `run-pass` | [`tests/043-explicit-this-capture.cpp`](./tests/043-explicit-this-capture.cpp) |
| V044 | `7.3.1.1 [namespace.unnamed]`, `3.4.1 [basic.lookup.unqual]` | unnamed-namespace functions participate in enclosing unqualified lookup | `run-pass` | [`tests/044-unnamed-namespace-unqualified-call.cpp`](./tests/044-unnamed-namespace-unqualified-call.cpp) |
| V045 | `7.1.1 [dcl.stc]`, `9.4.2 [class.static.data]` | `static thread_local` class data member definition and use | `run-pass` | [`tests/045-static-thread-local-member.cpp`](./tests/045-static-thread-local-member.cpp) |
| V046 | `5.2.1 [expr.sub]` | builtin pointer subscript may yield class lvalues/references | `run-pass` | [`tests/046-pointer-subscript-class-reference.cpp`](./tests/046-pointer-subscript-class-reference.cpp) |
| V047 | `5.1.2 [expr.prim.lambda]`, `14.2 [temp.names]` | lambda inside class template can call static member function template by unqualified name | `run-pass` | [`tests/047-lambda-static-member-template-call.cpp`](./tests/047-lambda-static-member-template-call.cpp) |
| V048 | `5.5 [expr.mptr.oper]` | `.*` supports data-member and member-function access | `run-pass` | [`tests/048-dot-star-member-access.cpp`](./tests/048-dot-star-member-access.cpp) |
| V049 | `5.5 [expr.mptr.oper]` | `->*` supports data-member and member-function access | `run-pass` | [`tests/049-arrow-star-member-access.cpp`](./tests/049-arrow-star-member-access.cpp) |
| V101 | `12.3.2 [class.conv.fct]`, `8.5 [dcl.init]` | explicit conversion function must not enable copy-init | `compile-fail` | [`tests/101-explicit-conversion-copy-init.cpp`](./tests/101-explicit-conversion-copy-init.cpp) |
| V102 | `7.2 [dcl.enum]`, conversion rules in clause 4 | scoped enum must not convert implicitly to `int` | `compile-fail` | [`tests/102-scoped-enum-no-implicit-int.cpp`](./tests/102-scoped-enum-no-implicit-int.cpp) |
| V103 | `10.3 [class.virtual]` | `override` mismatch must be rejected | `compile-fail` | [`tests/103-override-signature-mismatch.cpp`](./tests/103-override-signature-mismatch.cpp) |
| V104 | `14.2 [temp.names]`, `14.6.4 [temp.dep.res]` | missing `template` disambiguator in dependent member call | `compile-fail` | [`tests/104-dependent-member-template-missing-keyword.cpp`](./tests/104-dependent-member-template-missing-keyword.cpp) |
| V105 | `4.4 [conv.qual]` | invalid multilevel qualification conversion | `compile-fail` | [`tests/105-multilevel-qualification-conversion.cpp`](./tests/105-multilevel-qualification-conversion.cpp) |
| V106 | `4.11 [conv.mem]` | invalid reverse pointer-to-member conversion | `compile-fail` | [`tests/106-pointer-to-member-invalid-reverse.cpp`](./tests/106-pointer-to-member-invalid-reverse.cpp) |
| V107 | `8.5.4 [dcl.init.list]` | copy-list-init must reject an explicit constructor | `compile-fail` | [`tests/107-copy-list-init-explicit-ctor.cpp`](./tests/107-copy-list-init-explicit-ctor.cpp) |
| V108 | `8.5.4 [dcl.init.list]` | narrowing in list-initialization must be rejected | `compile-fail` | [`tests/108-list-init-narrowing.cpp`](./tests/108-list-init-narrowing.cpp) |
| V109 | `8.4.3 [dcl.fct.def.delete]`, `12.8 [class.copy]` | deleted copy constructor blocks copy-init | `compile-fail` | [`tests/109-deleted-copy-constructor.cpp`](./tests/109-deleted-copy-constructor.cpp) |
| V110 | `14.6.2.1 [temp.dep.type]` | current-instantiation missing member is ill-formed | `compile-fail` | [`tests/110-current-instantiation-missing-member.cpp`](./tests/110-current-instantiation-missing-member.cpp) |
| V111 | `10.3 [class.virtual]` | `final` function must not be overridden | `compile-fail` | [`tests/111-final-override-redeclaration.cpp`](./tests/111-final-override-redeclaration.cpp) |
| V112 | `14.8.2.5 [temp.deduct.type]` | parameter used only in a non-deduced context must not deduce | `compile-fail` | [`tests/112-nondeduced-context-only.cpp`](./tests/112-nondeduced-context-only.cpp) |
| V113 | `14.8.2.4 [temp.deduct.partial]`, `13.3 [over.match]` | cv-symmetric pointer templates should remain ambiguous | `compile-fail` | [`tests/113-ambiguous-cv-pointer-partial-ordering.cpp`](./tests/113-ambiguous-cv-pointer-partial-ordering.cpp) |
| V114 | `14.8.2.2 [temp.deduct.funcaddr]` | overloaded function address without enough context must fail deduction | `compile-fail` | [`tests/114-overload-set-nondeduced.cpp`](./tests/114-overload-set-nondeduced.cpp) |
| V115 | `14.8.2.5 [temp.deduct.type]` | function parameter pack not at end is a non-deduced context | `compile-fail` | [`tests/115-pack-not-at-end-nondeduced.cpp`](./tests/115-pack-not-at-end-nondeduced.cpp) |
| V116 | `14.8.2.5 [temp.deduct.type]`, `8.5.4 [dcl.init.list]` | ordinary template parameter must not deduce from braced-init-list | `compile-fail` | [`tests/116-braced-init-list-nondeduced.cpp`](./tests/116-braced-init-list-nondeduced.cpp) |
| V117 | `8.4.3 [dcl.fct.def.delete]`, `12.8 [class.copy]` | deleted move assignment must be rejected when selected | `compile-fail` | [`tests/117-deleted-move-assignment.cpp`](./tests/117-deleted-move-assignment.cpp) |
| V118 | `8.4.2 [dcl.fct.def.default]`, `12.8 [class.copy]` | defaulted move assignment can be implicitly deleted | `compile-fail` | [`tests/118-defaulted-move-assignment-deleted.cpp`](./tests/118-defaulted-move-assignment-deleted.cpp) |
| V119 | `8.4.3 [dcl.fct.def.delete]`, `12.8 [class.copy]` | deleted copy assignment blocks lvalue assignment | `compile-fail` | [`tests/119-deleted-copy-assignment.cpp`](./tests/119-deleted-copy-assignment.cpp) |

## Notes

- These are candidate validations, not yet assignment harness tests.
- Positive tests are written to avoid hosted-library dependencies.
- Negative tests are intentionally small and should be checked as rejection tests when promoted.
- Existing audit items in [`docs/spec-conformance-audit.md`](../docs/spec-conformance-audit.md) are not duplicated here unless the coverage shape is materially different.

## Promotion Rule

When one of these is turned into a real regression:

1. verify the exact N3485 wording first,
2. place the test in the earliest PA that naturally supports it,
3. sanity-check the emitted LowIR whenever the feature should already lower in
   the current compiler, even if the primary regression lives in an earlier PA;
   this means reading the LowIR and checking that it structurally matches the
   source rule being validated, not merely proving that later stages accept it,
4. keep the test here only if it still serves as a reusable spec-validation seed.

For the full one-by-one ingest workflow, use
[`INGEST_PROCESS.md`](./INGEST_PROCESS.md).
