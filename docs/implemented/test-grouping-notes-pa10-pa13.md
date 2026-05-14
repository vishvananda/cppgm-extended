# PA10-PA13 Test Grouping Notes

## PA10

### 2026-05-11 Test Grouping / Rename Pass

- Final layout: `tests/spec/` has 15 N3485-anchored syntax tests; `tests/general/`
  has 109 broader parser regressions and hosted/header reducers. `tests/derived/`
  was removed.
- Moved from `tests/spec/` to `tests/general/`: every former PA10 spec test except
  `100-empty`, `100-decl`, `100-empty-decl`, `100-main`, `100-params`,
  `100-namespace-forms`, `100-template-class`, `100-template-parameters`,
  `100-enum`, `100-switch-try`, `100-array-declarator`,
  `100-nested-declarator`, `200-class-bases-and-ctor-init`,
  `200-bit-field-declaration`, and `200-explicit-instantiation-declaration`.
  This includes the HHC reducers, malformed-input reducers, hosted-header style
  cases, template/body interaction cases, and later parser intake cases.
- Kept in `tests/spec/` with N3485 anchors:
  `100-empty` -> 2.2 `[lex.phases]`; `100-decl` and `100-empty-decl` -> 7
  `[dcl.dcl]`; `100-main` -> 8.4 `[dcl.fct.def]`; `100-params` -> 8.3.5
  `[dcl.fct]`; `100-namespace-forms` -> 7.3.1 `[namespace.def]`, 7.3.3
  `[namespace.udecl]`, 7.3.4 `[namespace.udir]`; `100-template-class` -> 14.1
  `[temp.param]`, 10 `[class.derived]`; `100-template-parameters` -> 14.1
  `[temp.param]`; `100-enum` -> 7.2 `[dcl.enum]`; `100-switch-try` -> 6.4.2
  `[stmt.switch]`, 6.5 `[stmt.iter]`, 15.3 `[except.handle]`;
  `100-array-declarator` -> 8.3.4 `[dcl.array]`; `100-nested-declarator` -> 8
  `[dcl.decl]`; `200-class-bases-and-ctor-init` -> 10 `[class.derived]`,
  12.6.2 `[class.base.init]`; `200-bit-field-declaration` -> 9.6 `[class.bit]`;
  `200-explicit-instantiation-declaration` -> 14.7.2 `[temp.explicit]`.
- Tests still needing N3485 comments: none in `tests/spec/`.
- Missing spec-anchored tests follow-up: added focused PA10 owners for
  template-id-vs-`<` expression parsing, declaration-statement ambiguity
  resolution, and type-id syntax in expression contexts:
  `300-template-id-less-expression`,
  `300-declaration-statement-ambiguity`, and
  `300-type-id-expression-contexts`. Additional parse-fail owners in the
  invalid/boundary family remain deferred until a smaller malformed-input audit.
- Missing general/regression/integration tests to add later: broader realistic
  multi-file driver cases remain outside PA10; future hosted-header reducers
  should stay in `tests/general/` unless reduced to a single grammar clause.
- README changes made: documented `tests/spec/` versus `tests/general/`,
  the N3485 leading-comment convention, and removal of active `tests/derived/`.
- Validation run: `git diff --check` passed; `make -C pa10 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed 127/127 after the missing-test follow-up.
- Open questions/risky moves deferred: some general tests may be promotable to
  `tests/spec/` after a clause-by-clause reduction pass, but this pass avoided
  mass citation over broad reducers.

## PA11

### 2026-05-11 Test Grouping / Rename Pass

- Final layout: `tests/spec/` has 19 N3485-anchored scope/type/lookup tests;
  `tests/general/` has 26 broader semantic regressions. `tests/derived/` was
  removed.
- Moved from `tests/spec/` to `tests/general/`: every former PA11 spec test except
  `100-empty`, `100-global`, `100-alias-and-function`, `100-namespace`,
  `100-using-directive`, `100-using-declaration`, `100-namespace-alias`,
  `100-qualified-type-lookup`, `100-class-scope`, `100-bad-unknown-type`,
  `200-decltype`, `200-sizeof-alignof-bounds`, `200-template-parameter-scope`,
  `200-enum-unscoped`, `200-enum-scoped`, `200-const-int-static-assert`,
  `200-using-directive-values`, `200-opaque-scoped-enum`, and
  `200-namespace-anonymous-union-injected-members`.
- Kept in `tests/spec/` with N3485 anchors:
  `100-empty` -> 2.2 `[lex.phases]`; `100-global` -> 3.3.6
  `[basic.scope.namespace]`, 8.3.5 `[dcl.fct]`; `100-alias-and-function` -> 7.1.3
  `[dcl.typedef]`; `100-namespace` -> 7.3.1 `[namespace.def]`;
  `100-using-directive` -> 7.3.4 `[namespace.udir]`; `100-using-declaration` ->
  7.3.3 `[namespace.udecl]`; `100-namespace-alias` -> 7.3.2
  `[namespace.alias]`; `100-qualified-type-lookup` -> 3.4.3
  `[basic.lookup.qual]`; `100-class-scope` -> 3.3.7 `[basic.scope.class]`;
  `100-bad-unknown-type` -> 3.4.1 `[basic.lookup.unqual]`; `200-decltype` ->
  7.1.6.2 `[dcl.type.simple]`; `200-sizeof-alignof-bounds` -> 5.3.3
  `[expr.sizeof]`, 5.3.6 `[expr.alignof]`; `200-template-parameter-scope` -> 14.1
  `[temp.param]`; `200-enum-unscoped`, `200-enum-scoped`, and
  `200-opaque-scoped-enum` -> 7.2 `[dcl.enum]`; `200-const-int-static-assert` ->
  7 `[dcl.dcl]`; `200-using-directive-values` -> 7.3.4 `[namespace.udir]`, 3.4.1
  `[basic.lookup.unqual]`; `200-namespace-anonymous-union-injected-members` ->
  9.5 `[class.union]`.
- Tests still needing N3485 comments: none in `tests/spec/`.
- Missing spec-anchored tests follow-up: added focused negative PA11 owners for
  namespace aliases that do not name a namespace, opaque enum redeclaration with
  conflicting underlying type, and using-declarations naming template-ids:
  `300-namespace-alias-non-namespace-bad`,
  `300-opaque-enum-redecl-underlying-bad`, and
  `300-using-declaration-template-id-bad`. More realistic inline-namespace /
  alias / value-lookup combinations remain general regressions unless reduced
  to single-clause tests.
- Missing general/regression/integration tests to add later: realistic PA11
  library-header scope reducers that combine inline namespaces, aliases, and
  value lookup without being single-clause tests.
- README changes made: documented `tests/spec/` versus `tests/general/`,
  the N3485 leading-comment convention, and removal of active `tests/derived/`.
- Validation run: `git diff --check` passed; `make -C pa11 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed 48/48 after the missing-test follow-up.
- Open questions/risky moves deferred: the current `tests/general/` enum and
  using cases may later be split into smaller clause owners, but they stayed
  general here because their current forms combine several semantic surfaces.

## PA12

### 2026-05-11 Test Grouping / Rename Pass

- Final layout: `tests/spec/` has 24 N3485-anchored call/conversion/control-flow
  tests; `tests/general/` has 95 broader call-semantic regressions. `tests/derived/`
  was removed.
- Moved from `tests/spec/` to `tests/general/`: every former PA12 spec test except
  `100-empty`, `100-function-decls`, `100-simple-call`, `100-overload-exact`,
  `100-local-arith`, `100-pointer-plus-integral`, `100-integral-conversions`,
  `100-overload-ranking`, `100-pointer-qualification-conversion`,
  `100-reference-binding`, `100-qualified-namespace-call`, `100-bad-no-match`,
  `200-if-control-flow`, `200-for-loop`, `200-do-statement`, `200-switch-statement`,
  `200-function-pointer-call`, `200-array-decay-call`,
  `200-unary-logical-conditional`, `200-subscript-expression`, `200-sizeof-typeid`,
  `300-ellipsis-worse-than-pointer-overload`, `300-bad-ambiguous-overload`, and
  `300-block-scope-namespace-alias-qualified-call`.
- Kept in `tests/spec/` with N3485 anchors:
  `100-empty` -> 2.2 `[lex.phases]`; `100-function-decls` -> 8.3.5 `[dcl.fct]`;
  `100-simple-call` and `200-function-pointer-call` -> 5.2.2 `[expr.call]`;
  `100-overload-exact` and `100-bad-no-match` -> 13.3 `[over.match]`;
  `100-local-arith` -> 5.7 `[expr.add]`, 5.17 `[expr.ass]`;
  `100-pointer-plus-integral` -> 5.7 `[expr.add]`; `100-integral-conversions` ->
  4.7 `[conv.integral]`; `100-overload-ranking` -> 13.3.3.2 `[over.ics.rank]`;
  `100-pointer-qualification-conversion` -> 4.4 `[conv.qual]`;
  `100-reference-binding` -> 8.5.3 `[dcl.init.ref]`;
  `100-qualified-namespace-call` -> 3.4.3 `[basic.lookup.qual]`, 5.2.2
  `[expr.call]`; `200-if-control-flow` -> 6.4.1 `[stmt.if]`; `200-for-loop` ->
  6.5.3 `[stmt.for]`; `200-do-statement` -> 6.5.2 `[stmt.do]`;
  `200-switch-statement` -> 6.4.2 `[stmt.switch]`; `200-array-decay-call` -> 4.2
  `[conv.array]`, 5.2.2 `[expr.call]`; `200-unary-logical-conditional` -> 5.3.1
  `[expr.unary.op]`, 5.14 `[expr.log.and]`, 5.16 `[expr.cond]`;
  `200-subscript-expression` -> 5.2.1 `[expr.sub]`; `200-sizeof-typeid` -> 5.3.3
  `[expr.sizeof]`; `300-ellipsis-worse-than-pointer-overload` -> 13.3.3.2
  `[over.ics.rank]`; `300-bad-ambiguous-overload` -> 13.3.3 `[over.match.best]`;
  `300-block-scope-namespace-alias-qualified-call` -> 7.3.2 `[namespace.alias]`,
  6 `[stmt.stmt]`, 5.2.2 `[expr.call]`.
- Tests still needing N3485 comments: none in `tests/spec/`.
- Missing spec-anchored tests follow-up: added focused PA12 owners for null
  pointer conversion from `nullptr`, compound assignment requiring a modifiable
  lvalue, and condition-declaration scope:
  `300-nullptr-pointer-conversion`,
  `300-compound-assignment-lvalue-bad`, and
  `300-condition-declaration-scope`. More overload-ranking and pointer
  comparison boundary reductions remain deferred because existing broader
  regressions already cover the behavior at the current assignment boundary.
- Missing general/regression/integration tests to add later: realistic
  procedural-header reducers that combine overload sets, namespace imports,
  pointer conversions, and control flow without being single-clause tests.
- README changes made: documented `tests/spec/` versus `tests/general/`,
  the N3485 leading-comment convention, and removal of active `tests/derived/`.
- Validation run: `git diff --check` passed; `make -C pa12 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed 122/122 after the missing-test follow-up.
- Open questions/risky moves deferred: several current `tests/general/` cases
  could be split into smaller spec tests later, especially the pointer/null and
  enum-conversion reducers.

## PA13

### 2026-05-11 Test Grouping / Rename Pass

- Final layout: `tests/spec/` has 85 direct `pa13/lowir.md` LowIR contract tests.
  `tests/debuginfo/` keeps the existing role/oracle split: `o0`, `source`,
  `source-g0`, `machine`, `object`, `object-g0`, `object-o1`, `object-o2`,
  `executable`, `executable-g0`, `executable-multi`, `executable-o1`, and
  `executable-o2`.
- Tests moved: none. The LowIR suite already uses a real spec bucket, and the
  debug-info subdirectories are meaningful oracle buckets rather than cleanup
  artifacts.
- Kept in `tests/spec/` with anchors: all PA13 `tests/spec/*.t` are anchored to
  `pa13/lowir.md` rather than N3485 because they are LowIR input tests, not C++
  source-language tests.
- Tests still needing N3485 comments: none; no C++ source-language tests live in
  PA13 `tests/spec/`.
- Missing spec-anchored tests to add later: smaller parser-only LowIR metadata
  rejection tests for invalid combinations that are currently covered only by
  broader smoke/rejection cases.
- Missing general/regression/integration tests to add later: optional PA13
  runtime-through-CY86 smoke tests remain manual; add a separate role bucket only
  if that becomes a committed oracle.
- README changes made: documented `tests/spec/` as the LowIR contract bucket,
  documented `tests/debuginfo/` and the `make test-debuginfo` oracle split, and
  avoided collapsing debug-info subdirectories.
- Validation run: `git diff --check` passed; `make -C pa13 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed 85/85.
- Open questions/risky moves deferred: no PA13 tests were moved because the
  current split is tied to real tool/oracle differences.
