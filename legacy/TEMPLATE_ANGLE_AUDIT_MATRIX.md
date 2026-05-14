# Template Angle Audit Matrix

This matrix is the durable closure surface for parser and semantic fixes around
`<` / `>` disambiguation, nested template-ids, and scoped fragment reparsing.

When frontier work lands in this family, the fix is not considered closed until
the relevant audit rows below have been rerun along with the active smoke and
the hosted sweep.

## Parser / Translation Unit Cases

- Unknown or not-yet-semanticized template-id call expression:
  - `pa10/tests/spec/193-template-condition.t`
- Known value followed by `<` in an ordinary conditional expression:
  - `pa10/tests/spec/227-conditional-simple-type-shift-return.t`
  - `pa10/tests/spec/228-template-conditional-simple-type-shift-return.t`
- Function template-id followed by `)` as an expression argument:
  - `pa10/tests/spec/246-template-id-function-pointer-argument.t`
- Function template-id followed by `;` in an initializer:
  - `pa10/tests/spec/247-template-id-function-pointer-initializer.t`
- Decltype / comparison / shift inside template arguments:
  - `pa10/tests/spec/229-decltype-less-partial-specialization.t`
- Nested unknown template-ids in constructor or member bodies:
  - `pa10/tests/spec/211-placement-new-pack-init.t`
  - `pa10/tests/spec/243-forward-unknown-nested-template-in-ctor-body.t`
- Member-template parameter value vs template-name disambiguation:
  - `pa10/tests/spec/244-member-template-if-less-template-call.t`
  - `pa10/tests/spec/245-member-template-parameter-value-vs-template-name.t`
- Base-clause and inherited typedef/template contexts:
  - `pa10/tests/spec/240-template-member-definition-inherited-typedef-cast.t`
  - `pa10/tests/spec/242-inline-namespace-template-visibility-base.t`

## Semantic Fragment / Reparse Cases

- Scoped semantic template-id parsing through dependent aliases:
  - `pa21/tests/spec/293-explicit-template-call-dependent-alias-conversion.t`
  - `pa21/tests/spec/366-function-template-nested-alias-explicit-call.t`
- Target-aware reparsing for constructor templates:
  - `pa21/tests/spec/362-constructor-template-dependent-alias-target-aware.t`
  - `pa21/tests/spec/365-constructor-template-parameter-shadowing-target-aware.t`
- Default template argument parsing in semantic/template flows:
  - `pa21/tests/spec/115-variable-template-id-default-argument.t`
  - `pa21/tests/spec/344-function-template-default-parameter-instantiation.t`

## Suggested Validation Commands

For parser-angle changes:

```sh
cd pa10
/usr/local/opt/make/libexec/gnubin/make test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1
```

For scoped semantic fragment / template reparsing changes:

```sh
cd pa21
/usr/local/opt/make/libexec/gnubin/make test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1
```

For plan-level closeout:

```sh
cd /Users/vishvananda/cppgm
/usr/local/opt/make/libexec/gnubin/make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6
```
