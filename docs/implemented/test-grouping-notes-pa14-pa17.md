# PA14-PA17 Test Grouping Notes

## PA14

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` has 54 tracked `.t` tests; `tests/spec/` has no tracked tests.
- Tests moved: all tracked PA14 tests and sidecars moved from `tests/spec/` to `tests/general/`, including the orphan `100-bad-loop.ref*` sidecars and the `200-included-namespace-global-definition.h` support header.
- Tests kept in spec and N3485 anchors: none. The current PA14 suite is a LowIR lowering oracle rather than direct C++ standard-conformance coverage.
- Tests still needing N3485 comments: none in tracked `tests/spec/`.
- Missing tests to add later: direct standard-anchored procedural language cases could be added to `tests/spec/` after deciding whether they belong in PA12 semantics or PA14 lowering; additional deterministic PA14 lowering rejection owners remain reserved around the `300-349` band.
- README changes: documented `tests/general/`, reserved `tests/spec/` for future N3485-anchored cases, and removed any public role for `tests/derived/`.
- Validation: `git diff --check` and `git diff --cached --check` passed. `make -C pa14 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passed local `54/54` and course `1/1`.
- Open questions: whether PA14 should eventually own any spec-anchored C++ language cases, or only LowIR lowering regressions over earlier semantic ownership.

## PA15

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` has 155 tracked `.t` tests; `tests/spec/` has 7 tracked `.t` tests.
- Tests moved: all non-spec-anchored tracked PA15 tests and sidecars moved from `tests/spec/` to `tests/general/`, including `300-header-static-class-init.h`.
- Tests kept in spec and N3485 anchors: `200-aggregate-brace-elision` (`8.5.1 [dcl.init.aggr]`), `200-direct-list-init-explicit-ctor` / `200-list-init-narrowing-bad` (`8.5.4 [dcl.init.list]`), `200-member-pointer-invalid-reverse-bad` (`4.11 [conv.mem]`), `300-const-reference-binds-derived-pointer-prvalue` (`8.5.3 [dcl.init.ref]`, `4.10 [conv.ptr]`), `300-inherited-conversion-operator-parameter-binding` (`12.3.2 [class.conv.fct]`), and `300-unused-member-function-static-assert-bad` (`7 [dcl.dcl] static_assert-declaration`).
- Tests still needing N3485 comments: none in tracked `tests/spec/`.
- Missing tests to add later: consider adding smaller spec owners for access control, member lookup, aggregate classification, pointer-to-member conversions, and static assertion diagnostics if those are meant to be PA15-public rather than general regressions.
- README changes: documented `tests/general/` as the object-model LowIR/regression oracle and `tests/spec/` as the N3485-anchored bucket; removed any public role for `tests/derived/`.
- Validation: `git diff --check` and `git diff --cached --check` passed. `make -C pa15 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passed `162/162`.
- Open questions: several general tests are plausible standard-conformance candidates but need a clause-by-clause audit before moving back into `tests/spec/`.

## PA16

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` has 100 tracked `.t` tests; `tests/spec/` has 15 tracked `.t` tests.
- Tests moved: all non-spec-anchored tracked PA16 tests and sidecars moved from `tests/spec/` to `tests/general/`.
- Tests kept in spec and N3485 anchors: `300-range-for-member-begin-end` (`6.5.4 [stmt.ranged]`), `300-lambda-capture-mutable` (`5.1.2 [expr.prim.lambda]`), `300-scoped-enum-underlying-type` / `300-scoped-enum-no-implicit-int-bad` (`7.2 [dcl.enum]`), and the copy/move/default/delete family `340`, `341`, `342`, `344`, `345`, `346`, `347`, `353`, `363`, `372`, `373` (`8.4.2 [dcl.fct.def.default]`, `8.4.3 [dcl.fct.def.delete]`, and/or `12.8 [class.copy]`).
- Tests still needing N3485 comments: none in tracked `tests/spec/`.
- Missing tests to add later: focused spec owners for copy-constructor default parameters, implicit move suppression edge cases, reference-member special-member behavior, and value-category-driven copy/move selection could be added after separating standard conformance from LowIR helper-shape expectations.
- README changes: documented `tests/general/` as the value-semantics LowIR/regression oracle and `tests/spec/` as the N3485-anchored bucket; removed any public role for `tests/derived/`.
- Validation: `git diff --check` and `git diff --cached --check` passed. `make -C pa16 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passed `115/115`.
- Open questions: some moved `defaulted-*`, `decltype`, and overload-resolution cases may deserve future `tests/spec/` placement after more specific N3485 anchoring.

## PA17

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` has 12 tracked `.t` tests; `tests/spec/` has 12 tracked `.t` tests.
- Tests moved: non-spec PA17 regressions moved from `tests/spec/` to `tests/general/`: `305`, `335`, `350`, `399`, `400`, `401`, `405`, `406`, `407`, `408`, `409`, and `411`, including `400-header-out-of-class-virtual-vtable.h`.
- Tests kept in spec and N3485 anchors: `300`, `310`, `320`, `330`, `340`, `390`, `395`, `402`, `403`, `404`, and `410` cite `10.3 [class.virtual]`; `300-pure-virtual-member` cites `10.4 [class.abstract]`.
- Tests still needing N3485 comments: none in tracked `tests/spec/`; this pass added missing comments to the older PA17 spec tests.
- Missing tests to add later: direct spec owners for abstract-class object rejection, virtual dispatch during construction/destruction, and deleted virtual overrides could be added if they are within the PA17 public boundary. Broader vtable/key-function/MI ABI coverage should stay general or move to later ABI/RTTI assignments.
- README changes: documented `tests/general/` as the polymorphic LowIR/regression oracle and `tests/spec/` as the N3485-anchored bucket; removed any public role for `tests/derived/`.
- Validation: `git diff --check` and `git diff --cached --check` passed. `make -C pa17 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passed local `24/24` and course `2/2`.
- Open questions: PA17 general contains some multi-base and ABI-shape regression coverage that exceeds the narrow README assignment boundary; keep it general unless the PA17 public boundary is intentionally expanded.
