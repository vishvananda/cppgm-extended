# PA30-PA35 Test Grouping Notes

## PA30

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` with 57 source-driven driver/link/runtime tests.
- Tests moved: all tracked files from `tests/spec/` to `tests/general/`, including `.ref*`, `.flags`, `.stdin`, helper `.lib.*`, `.t.N`, and include-support directories.
- Tests kept in spec and N3485 anchors: none. PA30 tests exercise the practical `cppgm++ -c` and link-driver contract, not direct N3485 clauses.
- Tests that still need N3485 comments: none because no `tests/spec/` bucket remains.
- Missing tests to add later: focused driver-mode owners for any remaining `-c`/link edge cases after the PA32-PA35 split settles; possible future N3485-specific linkage tests should go in `tests/spec/` with a leading `// N3485 focus: ...` comment.
- README changes: documented `tests/general/`, clarified that PA30 currently has no `tests/spec/`, and recorded the citation convention for any future spec tests.
- Validation: `git diff --check` and `git diff --cached --check` pass.
  `make -C pa30 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passes `57/57`.
- Open questions: none for grouping.

## PA32

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` with 62 host-object/link interoperability tests.
- Tests moved: all tracked files from `tests/spec/` to `tests/general/`.
- Tests renamed: none for uniqueness. A follow-up numbering correction restored
  pa1-pa9-style shared hundred-family prefixes after the move to
  `tests/general/`.
- Tests kept in spec and N3485 anchors: none. PA32 tests target host object format, host link, symbol spelling, coalescing, and interoperability.
- Tests that still need N3485 comments: none because no `tests/spec/` bucket remains.
- Missing tests to add later: no new cases added; future gaps should stay focused on host-linkable object behavior, with standard-language-only failures moved to earlier owning PAs or to a cited `tests/spec/` case.
- README changes: documented `tests/general/`, host-link/object-inspection role, and future N3485 citation convention.
- Validation: `git diff --check` and `git diff --cached --check` pass.
  `make -C pa32 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passes `62/62`.
- Open questions: none for grouping.

## PA33

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` with 77 host C++ ABI/runtime tests.
- Tests moved: all tracked files from `tests/spec/` to `tests/general/`.
- Tests renamed: none for uniqueness. A follow-up numbering correction restored
  pa1-pa9-style shared hundred-family prefixes after the move to
  `tests/general/`, including the intentional shared host ABI family.
- Tests kept in spec and N3485 anchors: none. PA33 tests target host ABI/runtime behavior, EH, RTTI/vtables, thunks, and related object facts.
- Tests that still need N3485 comments: none because no `tests/spec/` bucket remains.
- Missing tests to add later: no new cases added; any future EH/RTTI/vtable gaps should preserve the host-link/run plus inspect oracle, while pure language conformance belongs in an earlier cited spec bucket.
- README changes: documented `tests/general/`, host ABI/runtime oracle split, and future N3485 citation convention.
- Validation: `git diff --check` and `git diff --cached --check` pass.
  `make -C pa33 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passes `77/77`.
- Open questions: none for grouping.

## PA34

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/preproc/` with 37 hosted preprocessor tests and `tests/compile/` with 214 hosted compile-only tests. The former internal `tests/frontier/` discovery files are not part of the tested surface and were removed from the repository.
- Tests moved: none across public buckets; `preproc` and `compile` remain meaningful role buckets.
- Tests renamed: none. A follow-up numbering correction kept the existing
  pa1-pa9-style shared hosted-family prefixes rather than forcing unique
  numeric prefixes.
- Tests kept in spec and N3485 anchors: none. PA34 tests are hosted/vendor compatibility, reducer, sentinel, and bootstrap-facing compile cases.
- Tests that still need N3485 comments: none because no `tests/spec/` bucket exists.
- Missing tests to add later: no new cases added; future public additions should keep preprocessor-only cases in `tests/preproc/` and hosted compile acceptance in `tests/compile/`.
- README changes: documented the preproc/compile oracle split and future N3485 citation convention.
- Validation: `git diff --check` and `git diff --cached --check` pass.
  `make -C pa34 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed `preproc` 37/37 and `compile` 214/214 during coordinator review.
  Final coordinator root validation also passed:
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed `2787 / 2787`.
- Open questions: none for grouping.

## PA35

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/link/` with 114 hosted compile plus host link/run tests.
- Tests moved: none across buckets; `tests/link/` is kept because link/run and optional symbol inspection are the PA35 oracle.
- Tests renamed: none. A follow-up numbering correction kept the existing
  pa1-pa9-style shared hosted-link family prefixes rather than forcing unique
  numeric prefixes.
- Tests kept in spec and N3485 anchors: none. PA35 tests are hosted link/runtime, ABI spelling, object-inspection, and regression cases.
- Tests that still need N3485 comments: none because no `tests/spec/` bucket exists.
- Missing tests to add later: no new cases added; future additions should stay in `tests/link/` unless they are hosted compile-only regressions that belong back in PA34.
- README changes: documented `tests/link/` as the intentional public bucket and recorded future N3485 citation convention.
- Validation: `git diff --check` and `git diff --cached --check` pass.
  Final coordinator root validation passed:
  `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed `2787 / 2787`.
- Open questions: none for grouping.
