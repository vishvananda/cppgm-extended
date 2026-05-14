# Deref String Literal

This note lives in the main checkout under `async-small-fix-discovery/`; do not commit it from
there.

## Worktree

- path: `/tmp/cppgm-async-small-fix-20260328-060249`
- branch: `async-small-fix-20260328-060249`
- base commit: `df0290a9b5d75030b5bd27690f0367ed6eb3cc3d`

## Discovery Environment

- seeded `obj/` source: `/Users/vishvananda/cppgm`
- frozen binary: `/tmp/cpphostinterop-async-small-fix-20260328-060249`
- `CXX`: `/usr/local/opt/llvm/bin/clang++`
- `CPPGM_HOST_CXX`: `/usr/local/opt/llvm/bin/clang++`
- discovery command(s):
  - reused the existing `df0290a9` size-ordered hosted discovery report instead of rerunning the
    sweep
  - direct hosted probe:
    `/tmp/cpphostinterop-async-small-fix-20260328-060249 -c -I dev/src -o /tmp/preprocessor.async.o dev/src/preprocessor.cpp`

## Cluster

- normalized family: unary `*` rejects string-literal / array operands before array-to-pointer decay
- representative file(s): `dev/src/preprocessor.cpp`
- bootstrap tracker item checked: active bootstrap item remained the `lowirgensemantic.cpp`
  map-iterator constructor family; skipped here
- why this looked like a small fix:
  - sharp first diagnostic
  - immediate expression-level reducer
  - clear earliest owner
  - independent of the active bootstrap item and the other open async items in this reused worktree

## Candidate

- chosen file or repro: hosted `dev/src/preprocessor.cpp` and reduced `*"abc"` repro
- why this is not the current bootstrap next item:
  - not in `lowirgensemantic.cpp`
  - not in the active bootstrap tracker family
  - different from the still-open empty-brace and template-argument ADL items
- earliest owning assignment: `PA12`
- regression path(s): `pa12/tests/spec/336-deref-string-literal.t`
- required `dev/` build target(s) for later serial intake:
  - `cppcalls`
  - `cpphostinterop`
- bootstrap-sensitive for later serial intake: no

## Reduction

- reduced repro:
  - `int main() { return *"abc"; }`
- broken proof:
  - `cppcalls` failed with `indirection requires pointer`
  - hosted `dev/src/preprocessor.cpp` failed in `build_compiler_probe_commands` on
    `*CPPGM_DEFAULT_HOST_CXX`
- fixed proof:
  - `cppcalls` reducer: pass
  - hosted `dev/src/preprocessor.cpp`: pass

## Fix

- implementation summary:
  - use the normal value-conversion type in unary `*` analysis so array operands decay to pointer
    before the pointer-kind check
- key file(s):
  - `dev/src/semantic_expression.cpp`
  - `pa12/tests/spec/336-deref-string-literal.t`

## Validation

- direct repro:
  - reduced `*"abc"` repro under `cppcalls`: pass
  - `cpphostinterop -c dev/src/preprocessor.cpp`: pass
- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`:
  - pass
- owning suite if separately run:
  - `make -C pa12 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`
  - pass `103/103`
- discovery rerun:
  - not rerun; representative hosted file moved from failure to pass

## Result

- status: fixed and committed in reused async worktree
- commit: `6fc5b5221bb1686f5f879cbfc6aba8c6f4e775c5`
- next visible blocker or remaining family note:
  - the broader `df0290a9` discovery report still has unrelated families in
    `template_selection.cpp`, `macroizer.cpp`, `semantic_declaration.cpp`,
    `semantic_output.cpp`, and the active bootstrap `lowirgensemantic.cpp` item

## Process Note

- this item intentionally reused the same async worktree as the earlier
  `2026-03-28-empty-brace-scalar-init` and `2026-03-28-template-argument-adl-call` fixes because
  the user explicitly requested reusing it to save time
