# Empty Brace Scalar Init

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
  - size-ordered hosted sweep with a frozen `/tmp` binary and a hard `30s` per-file timeout
  - direct hosted probe:
    `/tmp/cpphostinterop-async-small-fix-20260328-060249 -c -I dev/src -o /tmp/posttokenizer.async.o dev/src/posttokenizer.cpp`

## Cluster

- normalized family: empty scalar braced initialization rejected as needing one element
- representative file(s): `dev/src/posttokenizer.cpp`
- bootstrap tracker item checked: active bootstrap item was the `lowirgensemantic.cpp`
  map-iterator constructor family in `LowIRFunctionBuilder::emit_switch_body(...)`; skipped here
- why this looked like a small fix:
  - sharp first diagnostic
  - easy reduction
  - clear earliest owner
  - independent of the active bootstrap item and outstanding async notes

## Candidate

- chosen file or repro: `dev/src/posttokenizer.cpp` and reduced scalar/member `{}` repros
- why this is not the current bootstrap next item:
  - not in `lowirgensemantic.cpp`
  - not in the active bootstrap tracker family
  - not one of the open async families already noted under `async-small-fix-discovery/`
- earliest owning assignment: `PA10`
- regression path(s): `pa10/tests/spec/268-empty-brace-scalar-init.t`
- required `dev/` build target(s) for later serial intake:
  - direct reduction used `cpplangcore`
  - representative hosted proof used `cpphostinterop`
  - later gate used the already-built `dev/` tree for `verify-fast-pa10-31-nobuild`
- bootstrap-sensitive for later serial intake: no

## Reduction

- reduced repro:
  - `int main() { int x{}; return x; }`
  - `struct S { int x; S() : x{} {} }; int main() { S s; return s.x; }`
- broken proof:
  - `cpphostinterop -c dev/src/posttokenizer.cpp` failed with
    `ERROR: scalar braced-init-list requires one element [function decode_float]`
  - `cpplangcore`, `cppcalls`, and `cpplowir` all failed on the reduced repro, establishing
    `PA10` ownership
- fixed proof:
  - direct reduced repros now compile
  - hosted `dev/src/posttokenizer.cpp` now compiles under `cpphostinterop`

## Fix

- implementation summary:
  - treat empty scalar braced-init-lists as value-initialization instead of rejecting them
  - apply that consistently for plain scalar initialization, scalar members, and scalar bit-fields
- key file(s):
  - `dev/src/semantic_lifetime.cpp`
  - `pa10/tests/spec/268-empty-brace-scalar-init.t`

## Validation

- direct repro:
  - reduced plain scalar `{}` repro: pass
  - reduced member mem-init `{}` repro: pass
  - `cpphostinterop -c dev/src/posttokenizer.cpp`: pass
- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`:
  - pass
- owning suite if separately run:
  - `make -C pa10 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`
  - pass `114/114`
- discovery rerun:
  - not rerun; representative hosted file moved from failure to pass

## Result

- status: fixed and committed in async worktree
- commit: `0a24daeed2381435b5ba52faad9dccf7a9c0d213`
- next visible blocker or remaining family note:
  - the broader partial sweep still had unrelated families in `semantic_trace.cpp`,
    `template_selection.cpp`, `macroizer.cpp`, and `semantic_declaration.cpp`
