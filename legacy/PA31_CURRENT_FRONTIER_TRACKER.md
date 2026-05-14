# PA31 Current Frontier Tracker

Updated: 2026-04-01

## Goal

Finish the remaining PA31 failures without losing the current investigation state, and add
earlier non-PA31 regressions for each general compiler bug we uncover.

## Current Rule

When a PA31 failure turns out to be a general lowering/codegen/runtime bug rather than a
hosted-header-specific issue:

1. fix the compiler bug
2. add the smallest earlier regression in the appropriate pre-PA31 assignment
3. keep PA31 green while doing the later full regression sweep

Current expected earlier assignment ownership:

- internal lowering / runtime / compile+link behavior: `PA29`
- host object / weak symbol / duplicate host-link behavior: `PA30`
- hosted-header compatibility specifics: `PA31`

## Preferred Rerun Path

Use the explicit PA31 batch target while the frontier is still narrow:

```sh
make -C pa31 test-batch CPPGM_BATCH_TESTS=1 \
  PREPROC_TEST_ROOT=/tmp/pa31-empty-tests/preproc \
  COMPILE_TEST_ROOT=/tmp/pa31-empty-tests/compile \
  LINK_TEST_ROOT=tests/link/652-hosted-unordered-set-pointer-link-smoke.t
```

Toolchain rule:

- the PA31 host-interop worker should prefer `CPPGM_HOST_CXX`
- if `CPPGM_HOST_CXX` points at Homebrew LLVM `clang++`, the host link step should go through
  that driver too

## Current Remaining PA31 Failures

Current full-suite status:

- root `make test-report` passes: `1541 / 1541`
- no remaining checked-in `PA31` harness failures

## Current Active Investigation

Current checkpoint after the hosted `ostringstream` / `__to_chars_integral<2>` fix:

- explicit template-id lookup no longer aborts on the wrong overload during
  `std::__to_chars_integral<2>(...)`
- hosted stream insertion for unsigned integral types is covered by:
  - `PA31` `662-hosted-ostringstream-unsigned-int`
- the self-host compile of [`callsemantic_text.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic_text.cpp)
  succeeds again without the temporary `to_string(...)` workaround

## Next Fix

Next return point:

1. keep the separate qualified explicit-template-id reducer tracked below
2. resume the `callsemantic` breakup / hotspot work from the current all-green checkpoint
3. if a new hosted/bootstrap regression appears, add the earliest practical regression before
   widening the fix

## Required Earlier Regression To Add

None currently outstanding for the checked-in PA31 fixes in this tracker.

## Additional Notes

- both remaining PA31 link-suite failures are now post-link execution failures
- do not treat the PA31 work as done until the earlier regression for each general bug is
  landed too

## Separate Reducer Notes

Keep this separate from the current hosted `ostringstream` fix:

- qualified explicit function-template-id lookup for user code still fails before deduction
- minimal reducer:

```cpp
#include <type_traits>

namespace ns {
template <unsigned Base, class T, std::enable_if_t<!std::is_signed<T>::value, int> = 0>
int f(T value) {
  return Base + (int)value;
}
}

int main() {
  unsigned int x = 7;
  return ns::f<2>(x) == 9 ? 0 : 1;
}
```

Current symptom:

- `unknown function ns::f<2>`
- diagnostics show:
  - `qualified_target ns::ns`
  - `qualified_target_function_templates f`

Interpretation:

- this is a distinct explicit-template-id qualified lookup bug
- do not mix it into the `std::__to_chars_integral<2>` substitution fix
