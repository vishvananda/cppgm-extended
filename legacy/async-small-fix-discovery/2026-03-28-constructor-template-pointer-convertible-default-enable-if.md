# Constructor Template Pointer Convertible Default Enable If

This note lives in the main checkout under `async-small-fix-discovery/`; do not commit it from
there.

## Worktree

- path: `/tmp/cppgm-async-small-fix-20260328-080523`
- branch: `async-small-fix-20260328-080523`
- base commit: `e1a2af6fd962e991e13f0d2f7ee5c018bd7505fb`

## Discovery Environment

- seeded `obj/` source: `/Users/vishvananda/cppgm`
- frozen binary: `/tmp/cpphostinterop-discovery-a179d230`
- `CXX`: `/usr/local/opt/llvm/bin/clang++`
- `CPPGM_HOST_CXX`: `/usr/local/opt/llvm/bin/clang++`
- discovery command(s):
  - size-ordered hosted sweep from `HEAD` with a frozen `/tmp` binary and a hard `60s`
    per-file timeout
  - direct hosted probe:
    `/tmp/cpphostinterop-discovery-a179d230 -c -I dev/src -o /tmp/semantic_declaration.async.o dev/src/semantic_declaration.cpp`

## Cluster

- normalized family: libc++ iterator wrapper / constrained constructor family collapsing into
  defaulted `enable_if_t<..., int> = 0` rejection
- representative file(s): `dev/src/semantic_declaration.cpp`
- bootstrap tracker item checked: active bootstrap item remained the `macroizer.cpp` hosted family;
  skipped here
- why this looked like a small fix:
  - sharp first diagnostic on one representative file
  - easy reduction to a small template constructor case
  - moved a real hosted file when fixed
  - independent of both timeout files and the active bootstrap tracker item

## Candidate

- chosen file or repro: `dev/src/semantic_declaration.cpp` and a reduced `box<int* const*>`
  constructor-template repro using `enable_if_t<__is_convertible(...), int> = 0`
- why this is not the current bootstrap next item:
  - not in `macroizer.cpp`
  - not one of the timeout files
  - not one of the active bootstrap tracker families
- earliest owning assignment: `PA21`
- regression path(s):
  - `pa21/tests/spec/419-constructor-template-pointer-convertible-default-enable-if.t`
- required `dev/` build target(s) for later serial intake:
  - direct reduction used `cpptemplatecomplete`
  - representative hosted proof used `cpphostinterop`
  - later gate used the full `verify-fast-pa10-31` run
- bootstrap-sensitive for later serial intake: no

## Reduction

- reduced repro:
  - `template<class Rep> struct box { template<class Rep2, enable_if_t<__is_convertible(Rep2 const&, Rep), int> = 0> explicit box(const Rep2&); };`
  - instantiate as `box<int* const*> b(pp);` where `pp` is `int**`
- broken proof:
  - `cpphostinterop -c dev/src/semantic_declaration.cpp` failed with
    `ERROR: no viable overload for member call insert with 3 argument(s)`
  - `cpptemplatecomplete` failed on the reduced repro because the defaulted non-type template
    parameter type was not being resolved back to concrete `int`
- fixed proof:
  - the reduced repro now completes
  - `cpphostinterop -c dev/src/semantic_declaration.cpp` advances to a later unrelated blocker:
    `base class must be complete: _Layout<__split_buffer<_Tp,_Allocator,_Layout>,_Tp,_Allocator>`

## Fix

- implementation summary:
  - canonicalize simple rewritten bound type texts through the normal parser/type printer path
    instead of leaving local spellings like `Rep2 const&` in unstable text form
  - when resolving non-type template parameter types from decl-specifier text, parse simple
    non-dependent original text directly so `enable_if_t<..., int>` resolves to concrete `int`
  - keep the canonicalization narrowly scoped so qualified or nested template-id cases continue to
    use the existing path
- key file(s):
  - `dev/src/template_argument_semantics.cpp`
  - `dev/src/callsemantic.cpp`
  - `pa21/tests/spec/419-constructor-template-pointer-convertible-default-enable-if.t`

## Validation

- direct repro:
  - reduced constructor-template repro: pass
  - `cpphostinterop -c dev/src/semantic_declaration.cpp`: pass to the next blocker
- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`:
  - not used for final gate
- owning suite if separately run:
  - `make -C pa21 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`
  - pass `237/237`
- discovery rerun:
  - not rerun; representative hosted file moved from the original `insert(...)` failure to the next
    visible blocker
- final full gate:
  - `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
  - pass through `pa31`

## Result

- status: fixed and committed in async worktree
- commit: `b355ce01623268314840f2bd84243053a1547181`
- next visible blocker or remaining family note:
  - `dev/src/semantic_declaration.cpp` now fails later in libc++ layout completion, so the old
    `insert(...)` family appears closed
