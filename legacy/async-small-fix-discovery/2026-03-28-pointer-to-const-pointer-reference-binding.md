# Pointer To Const-Pointer Reference Binding

This note lives in the main checkout under `async-small-fix-discovery/`; do not commit it from
there.

## Worktree

- path: `/tmp/cppgm-async-small-fix-20260328-1201`
- branch: `async-small-fix-20260328-1201`
- base commit: `bc2069a2d4add7c8b67d2e462c486b707f7d4387`

## Discovery Environment

- seeded `obj/` source: `/Users/vishvananda/cppgm`
- frozen binary: reused the local worktree rebuild of `dev/cpphostinterop`
- `CXX`: `/usr/local/opt/llvm/bin/clang++`
- `CPPGM_HOST_CXX`: `/usr/local/opt/llvm/bin/clang++`
- discovery command(s): reused the earlier size-ordered hosted report from `/tmp/async-discovery-a179d230-size-report.md`

## Cluster

- normalized family: libc++ overload viability gaps around pointer/reference adaptation
- representative file(s): `dev/src/semantic_lookup.cpp`
- bootstrap tracker item checked: current active bootstrap item is `dev/src/nsinit_semantic.cpp` `unknown function lookup_symbol_alias`, so this family is independent
- why this looked like a small fix: the hosted failure reduced cleanly to a single `std::set<const A*>::insert(A*)` call

## Candidate

- chosen file or repro: `std::set<const A*> s; A* p = &a; s.insert(p);`
- why this is not the current bootstrap next item: it is a PA12 reference-binding hole, not the active bootstrap `nsinit_semantic.cpp` lookup blocker
- earliest owning assignment: `PA12`
- regression path(s): `pa12/tests/spec/337-reference-binding-pointee-const-pointer.t`
- required `dev/` build target(s) for later serial intake: `cpphostinterop`, `cppcalls`, `cppclasses`
- bootstrap-sensitive for later serial intake: no

## Reduction

- reduced repro:
  ```cpp
  struct A {};
  void keep(const A * const& x) {}
  int main() {
    A a;
    A* p = &a;
    keep(p);
    return 0;
  }
  ```
- broken proof: pre-fix `cpphostinterop -c` rejected the reducer and `dev/src/semantic_lookup.cpp` failed at `std::set<...>::insert(info)` with `no viable overload for member call insert with 1 argument(s)`
- fixed proof: post-fix the reducer compiles, and hosted `dev/src/semantic_lookup.cpp` advances to a later `std::shared_ptr<cpp_decl::Type>` constructor failure in `lookup_inline_namespace_children`

## Fix

- implementation summary: added a narrow pointer-only fallback in `semantic_conversion::try_argument_conversion` so an lvalue pointer can bind to a `const` lvalue reference to a cv-adjusted pointer type without broadening global copy-initialization rules
- key file(s): `dev/src/semantic_conversion.cpp`

## Validation

- direct repro: passed under `CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ ./dev/cpphostinterop -c`
- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`: still fails on clean `HEAD` at existing baseline `pa15/tests/spec/249-const-subobject-member-call.t`; the same failure reproduces with `dev/src/semantic_conversion.cpp` restored to `HEAD`
- owning suite if separately run: `make -C pa12 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1` passed `104/104`
- discovery rerun: not rerun

## Result

- status: fixed in async worktree; broader fast verification is blocked by an unrelated clean-`HEAD` PA15 baseline failure
- commit: `1c1b766f4e2e0b111d2f84a1d4bf0dabfcdb2bae`
- next visible blocker or remaining family note: hosted `dev/src/semantic_lookup.cpp` now fails later on `no viable constructor [class std::__1::shared_ptr<cpp_decl::Type>]`
