# 2026-03-28: Templated Lambda Default Arguments

## Worktree

- path: `/tmp/cppgm-async-small-fix-20260328-060249`
- branch: `async-small-fix-20260328-060249`
- base commit: `6fc5b5221bb1686f5f879cbfc6aba8c6f4e775c5`

## Bootstrap Exclusion Check

- active bootstrap item checked before selection
- this fix does not overlap the active bootstrap `callsemantic.cpp` frontier
- this fix does not overlap the open iterator-conversion `std::vector::insert` async follow-up

## Discovery

- hosted reducer:

```cpp
int main() {
  return []<class U>(U u, int y = 3) -> int {
    return u + y;
  }(1) - 4;
}
```

- hosted representative file: `dev/src/template_instantiation.cpp`
- initial failure: `no viable overload for call ... with 1/2 argument(s)`

## Root Cause

- templated lambda call-operator templates were not preserving usable default-argument AST nodes through synthesis and instantiation
- template call arity acceptance was still using exact parameter counts in cases where trailing parameters had defaults
- non-id callee expressions whose semantic result was a synthetic function id still needed to reconnect to the registered function binding so overload resolution could see defaults

## Fix

- parse and store lambda default arguments during lambda preparation and synthetic binding creation
- for templated lambdas, reparse the durable owned declarator so `default_arguments_pattern` does not point at temporary AST storage
- copy `default_arguments_pattern` into instantiated function bindings
- let function-template argument-count acceptance account for trailing default arguments
- reconnect non-id call expressions that semantically name a registered function binding so the normal candidate path sees defaults

## Regression

- owner regression: `pa31/tests/compile/643-instantiated-templated-lambda-default-arg.t`

## Validation

- direct hosted reducer: pass
- direct `cpphostinterop -c dev/src/template_instantiation.cpp`: pass
- `make -C pa31 test-worker CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`: pass
- `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`: pass

## Final Commit

- `de76acd00d71ab1140d78d79514c65b1bc175b78`
