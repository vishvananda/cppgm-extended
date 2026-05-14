# PA23-PA29 Test Grouping Notes

## PA23

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/strict/` for raw MIR comparison with minimal normalization;
  `tests/structural/` for canonical MIR comparison plus generated-program behavior.
- Tests moved: none. The strict/structural split is a real PA23 oracle split and was kept.
- Tests kept in `tests/spec/`: none. PA23 is a LowIR-to-native backend contract, not an
  N3485 C++ source-language spec bucket.
- Tests that still need N3485 comments: none in the current layout.
- Missing tests to add later: only add new PA23 owners when backend-quality work reopens the
  milestone; choose `strict` or `structural` based on whether exact raw MIR or canonical MIR
  shape is the intended oracle.
- README changes: documented that `strict` and `structural` are PA23-specific oracle
  buckets and that no `tests/spec/` bucket exists.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa23 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (`strict` 33/33, `structural` 67/67).
- Open questions: none for this grouping pass.

## PA24

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for the source-driven `cpplink -c -> cpplink`
  link/runtime regression suite.
- Tests moved: `git mv tests/spec tests/general`; all existing anchors moved together with
  `.ref.*` files and `.t.N` companion LowIR inputs. Anchor families moved: `100` single
  object, `100-160` cross-object/runtime/relocation owners, `100-195` link-error owners,
  and `200` atomic cross-object owner.
- Tests kept in `tests/spec/`: none. The current PA24 suite is a compiler-owned LowIR
  object/link pipeline suite rather than an N3485 source-language spec suite.
- Tests that still need N3485 comments: none in the current layout.
- Missing tests to add later: mixed-target rejection, incompatible/corrupt object rejection,
  and newly supported relocation families if they are not already represented.
- README changes: documented `tests/general/`, the absence of `tests/spec/`, and later
  coverage gaps.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa24 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (9/9).
- Open questions: none for this grouping pass.

## PA25

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for the source-driven `cppeh -c -> cppeh`
  private-EH link/runtime regression suite.
- Tests moved: `git mv tests/spec tests/general`; all five anchors (`100` same-function
  catch, `110` cross-function catch, `120` cross-object catch, `130` cleanup resume, `140`
  unhandled throw) moved with `.ref.*` files and `.t.N` companion LowIR inputs.
- Tests kept in `tests/spec/`: none. The current PA25 suite tests the compiler-private EH
  runtime ABI rather than an N3485 source-language spec bucket.
- Tests that still need N3485 comments: none in the current layout.
- Missing tests to add later: add private-EH owners only if runtime metadata, cleanup/resume,
  or cross-object unwind handling changes beyond the current `100-140` surface.
- README changes: documented `tests/general/`, the absence of `tests/spec/`, and later
  private-EH coverage criteria.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; full `make -C pa25 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  did not reach tests because the shared dev rebuild failed at
  `../obj/generated/cppgm_builtin_host_config.h.tmp`; focused `make -C pa25 test
  TEST_DEPS= CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passed (5/5) using existing
  `../dev/cppeh`.
- Open questions: none for this grouping pass.

## PA26

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for PA26 source-to-LowIR regressions, cross-feature
  combinations, implementation bug reducers, and boundary cases.
- Tests moved: `git mv tests/spec tests/general`; all 85 current `.t` anchors moved with
  checked-in `.ref`, `.ref.stdout`, and `.ref.exit_status` sidecars. No renumbering was done.
- Tests kept in `tests/spec/`: none. One moved general regression,
  `200-class-template-lambda-static-member-template-call.t`, already had an N3485 focus
  comment and kept it, but the suite as a whole is still a general LowIR oracle suite until
  spec cases are curated.
- Tests that still need N3485 comments: none in the current layout; future PA26
  `tests/spec/` cases should use leading `// N3485 focus: ...` comments.
- Missing tests to add later: focused N3485-anchored spec tests for `auto`, list
  initialization, lambdas, range-for, and placeholder-return rejection after PA26/PA27
  ownership is stable.
- README changes: documented `tests/general/`, the reserved `tests/spec/` citation
  convention, and later coverage gaps. Updated `test-debug` path in `Makefile`.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa26 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (85/85).
- Open questions: PA26 still has a broad semantic/lowering surface and should not grow new
  spec tests until the PA26/PA27 boundary is rechecked.

## PA27

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for PA27 source-to-LowIR regressions over capturing
  lambdas, initializer-list interoperation, RTTI, `typeid`, `dynamic_cast`, and exception
  source interactions.
- Tests moved: `git mv tests/spec tests/general`; all 34 current `.t` anchors moved with
  checked-in `.ref`, `.ref.stdout`, and `.ref.exit_status` sidecars. No renumbering was done.
- Tests kept in `tests/spec/`: none. One moved general regression,
  `200-explicit-this-capture-implicit-return.t`, already had an N3485 focus comment and kept
  it, but the suite includes several cross-feature regression owners and is not yet a
  curated spec bucket.
- Tests that still need N3485 comments: none in the current layout; future PA27
  `tests/spec/` cases should use leading `// N3485 focus: ...` comments.
- Missing tests to add later: focused N3485-anchored spec tests for lambda capture rules,
  `std::initializer_list`, `typeid`, and pointer-form `dynamic_cast` after PA27/PA28
  ownership is stable.
- README changes: documented `tests/general/`, the reserved `tests/spec/` citation
  convention, and later coverage gaps. Updated `test-debug` path in `Makefile`.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa27 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (34/34).
- Open questions: none beyond the PA27/PA28 ownership line for later spec-test buildout.

## PA28

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for PA28 source-to-LowIR regressions over
  non-virtual multiple inheritance, generated members, `dynamic_cast<void*>`, and ambiguity
  rejection.
- Tests moved: `git mv tests/spec tests/general`; all five anchors (`100`, `110`, `120`,
  `130`, `190`) moved with checked-in `.ref`, `.ref.stdout`, and `.ref.exit_status`
  sidecars.
- Tests kept in `tests/spec/`: none. The current suite is a compact milestone regression
  suite and has no leading N3485 focus comments.
- Tests that still need N3485 comments: none in the current layout; future PA28
  `tests/spec/` cases should use leading `// N3485 focus: ...` comments.
- Missing tests to add later: focused N3485-anchored spec tests for base-clause lookup,
  ambiguous member lookup, base-subobject initialization order, and `dynamic_cast<void*>`
  if they stay within the PA28 boundary.
- README changes: documented `tests/general/`, the reserved `tests/spec/` citation
  convention, and later coverage gaps. Updated `test-debug` path in `Makefile`.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa28 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (5/5).
- Open questions: none for this grouping pass.

## PA29

### 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout: `tests/general/` for PA29 source-to-LowIR regressions over virtual
  inheritance, non-primary polymorphic views, sibling `dynamic_cast`, and RTTI through
  adjusted base views.
- Tests moved: `git mv tests/spec tests/general`; all 13 current `.t` anchors moved with
  checked-in `.ref`, `.ref.stdout`, and `.ref.exit_status` sidecars.
- Tests kept in `tests/spec/`: none. The current suite has no leading N3485 focus comments
  and is a milestone regression/oracle suite.
- Tests that still need N3485 comments: none in the current layout; future PA29
  `tests/spec/` cases should use leading `// N3485 focus: ...` comments.
- Missing tests to add later: focused N3485-anchored spec tests for virtual-base
  layout/access, non-primary virtual dispatch, sibling `dynamic_cast`, and `typeid` through
  non-primary views after the PA29/PA30 handoff is stable.
- README changes: documented `tests/general/`, the reserved `tests/spec/` citation
  convention, and later coverage gaps. Updated `test-debug` path in `Makefile`.
- Validation: `git diff --check -- pa23 pa24 pa25 pa26 pa27 pa28 pa29
  docs/implemented/test-grouping-notes-pa23-pa29.md` passed; `make -C pa29 test
  CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
  passed (13/13).
- Open questions: none for this grouping pass.
