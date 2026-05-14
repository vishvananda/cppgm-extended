# Template-Argument ADL Call

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
    `/tmp/cpphostinterop-async-small-fix-20260328-060249 -c -I dev/src -o /tmp/semantic_trace.async.o dev/src/semantic_trace.cpp`

## Cluster

- normalized family: unqualified call misses ADL through template-argument-associated namespaces
- representative file(s): `dev/src/semantic_trace.cpp`
- bootstrap tracker item checked: active bootstrap item remained the `lowirgensemantic.cpp`
  map-iterator constructor family; skipped here
- why this looked like a small fix:
  - sharp first diagnostic
  - reduced quickly to a standalone language rule gap
  - clear earliest owner
  - independent of the active bootstrap item and the still-open empty-brace async item

## Candidate

- chosen file or repro: `dev/src/semantic_trace.cpp` and reduced ADL call repros
- why this is not the current bootstrap next item:
  - not in `lowirgensemantic.cpp`
  - not in the active bootstrap tracker family
  - different from the open `2026-03-28-empty-brace-scalar-init.md` item
- earliest owning assignment: `PA12`
- regression path(s): `pa12/tests/spec/335-template-argument-adl-call.t`
- required `dev/` build target(s) for later serial intake:
  - `cppcalls`
  - `cpphostinterop`
- bootstrap-sensitive for later serial intake: no

## Reduction

- reduced repro:
  - `namespace W { template<class T> struct Box {}; }`
  - `namespace N { struct T {}; int f(W::Box<T>); }`
  - `int g(W::Box<N::T> x) { return f(x); }`
- broken proof:
  - `cppcalls` failed the custom `Box<T>` reducer with `unknown function f`
  - `cppcalls` also failed the `std::shared_ptr<T>` variant
  - hosted `dev/src/semantic_trace.cpp` failed with `unknown function describe_type`
- fixed proof:
  - custom `Box<T>` reducer: pass
  - `std::shared_ptr<T>` reducer: pass
  - hosted `dev/src/semantic_trace.cpp`: pass

## Fix

- implementation summary:
  - make associated-namespace collection recurse through wrapped types and instantiated class
    template arguments instead of only looking at the top-level resolved class scope
  - this lets normal ADL see namespaces contributed by template arguments and pointer/reference
    wrappers
- key file(s):
  - `dev/src/semantic_lookup.cpp`
  - `pa12/tests/spec/335-template-argument-adl-call.t`

## Validation

- direct repro:
  - custom `Box<T>` ADL reducer under `cppcalls`: pass
  - `std::shared_ptr<T>` ADL reducer under `cppcalls`: pass
  - `cpphostinterop -c dev/src/semantic_trace.cpp`: pass
- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`:
  - pass
- owning suite if separately run:
  - `make -C pa12 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`
  - pass `102/102`
- discovery rerun:
  - not rerun; representative hosted file moved from failure to pass

## Result

- status: fixed and committed in reused async worktree
- commit: `560c3c4f30d76115716b490a54226759c3e69708`
- next visible blocker or remaining family note:
  - the broader `df0290a9` discovery report still has unrelated families in
    `template_selection.cpp`, `macroizer.cpp`, `semantic_declaration.cpp`,
    `semantic_output.cpp`, and the active bootstrap `lowirgensemantic.cpp` item

## Process Note

- this item intentionally reused the same async worktree as the earlier
  `2026-03-28-empty-brace-scalar-init` fix because the user explicitly requested reusing it to
  save time
