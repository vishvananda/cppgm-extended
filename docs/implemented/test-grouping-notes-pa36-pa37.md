# PA36-PA37 Test Grouping Notes

## PA36 - 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout kept as-is:
  - `pa36/tests/o0`: direct `lowiropt -O0` canonical LowIR round-trip.
  - `pa36/tests/o1`: direct `lowiropt -O1` structural optimizer oracle, including CFG cleanup, propagation, CSE/DCE, EH guards, and inlining.
  - `pa36/tests/o2`: direct `lowiropt -O2` structural optimizer oracle, including slot promotion and O2 preservation guards.
  - `pa36/tests/driver/o1` and `pa36/tests/driver/o2`: source-driven `cppgm++ --emit-lowir -g0 -O*` integration through the same optimizer.
  - `pa36/tests/debuginfo/o1`, `pa36/tests/debuginfo/o2`, `pa36/tests/debuginfo/driver/o1`, and `pa36/tests/debuginfo/driver/o2`: debug metadata preservation through direct and driver optimizer surfaces.
- Tests moved: none. There is no misleading `tests/spec` or `tests/derived` split, and the existing optimization-level / driver / debug-info buckets are meaningful oracle buckets.
- Tests kept in role buckets and why:
  - All direct LowIR optimizer cases stayed under `o0`, `o1`, or `o2` because the contract is optimized LowIR shape, not source-language conformance.
  - Driver cases stayed under `driver/o*` because they validate `cppgm++ --emit-lowir -O*` integration separately from direct `lowiropt`.
  - Debug-info cases stayed under `debuginfo/...` because `!dbg(...)` preservation is a distinct metadata oracle.
- Tests needing N3485 comments: none. No PA36 test was placed in or retained under `tests/spec`; the test contracts are optimizer/backend contracts over LowIR.
- Missing optimizer/backend tests to add later:
  - Direct `lowiropt -O0` debug-info round-trip coverage, if O0 debug metadata preservation becomes a public PA36 expectation.
  - Driver-side O1/O2 behavioral-preservation smokes that compile beyond `--emit-lowir`, if PA36 later wants a secondary runtime lane.
  - Negative/malformed LowIR optimizer diagnostics for invalid optimization inputs, if the standalone tool's diagnostic surface becomes graded.
  - Additional O2 alias/escape boundary cases around indirect stores and calls once alias-sensitive promotion policy is expanded.
- README changes:
  - Documented that buckets are oracle/optimization-level surfaces rather than N3485 spec buckets.
  - Documented direct `lowiropt`, driver `cppgm++ --emit-lowir`, debug-info, and direct LowIR text comparison expectations.
- Validation:
  - `git diff --check`: passed.
  - `make -C pa36 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`: passed (`o0` 1/1, `o1` 34/34, `o2` 11/11, `driver/o1` 3/3, `driver/o2` 6/6).
  - `make -C pa36 test-debuginfo CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`: passed (`debuginfo/o1` 2/2, `debuginfo/o2` 1/1, `debuginfo/driver/o1` 1/1, `debuginfo/driver/o2` 1/1).
  - Note: an initial parallel attempt to run PA36 and PA37 debug-info targets together failed in the shared dev rebuild on `../obj/generated/cppgm_builtin_host_config.h.tmp`; rerunning the debug-info targets sequentially with `CPPGM_SKIP_DEV_REBUILD=1` passed.
- Open questions:
  - Whether `make test` should eventually include debug-info buckets, or whether `test-debuginfo` should remain a separate opt-in lane.

## PA37 - 2026-05-11 Test Grouping / Rename Pass

- Final bucket layout kept as-is:
  - `pa37/tests/o1`: local machine-backend `lowir2native -O1` MIR/runtime oracle.
  - `pa37/tests/o2`: whole-function machine-backend `lowir2native -O2` MIR/runtime oracle, including repeated O1 coverage plus O2-only layout/frame cases.
  - `pa37/tests/debuginfo/o1` and `pa37/tests/debuginfo/o2`: debug-only MIR rewrite lanes for tests carrying `!dbg(...)` metadata.
- Tests moved: none. There is no misleading `tests/spec` or `tests/derived` split, and the existing optimization-level / debug-info buckets are meaningful role buckets.
- Tests kept in role buckets and why:
  - Non-debug cases stayed under `o1` or `o2` because the contract is machine-IR quality plus preserved native runtime behavior at a specific backend optimization level.
  - Debug-info cases stayed under `debuginfo/o*` because location metadata preservation is a distinct oracle layered on MIR rewrites.
- Tests needing N3485 comments: none. No PA37 test was placed in or retained under `tests/spec`; handwritten inputs are LowIR backend fixtures, not C++ language conformance fixtures.
- Missing optimizer/backend tests to add later:
  - O2-only branch-layout cases with conditional traces and multiple cold exits.
  - Spill/reload and register-pressure reductions once those optimizations are added.
  - Stack-frame shrink cases involving outgoing call arguments, locals, and callee-saved interaction.
  - Backend optimization preservation across calls that clobber caller-saved registers.
  - Failure/diagnostic cases for unsupported or malformed LowIR, if PA37 decides to grade native-backend diagnostics directly.
- README changes:
  - Documented that buckets are machine-backend optimization role buckets rather than N3485 spec buckets.
  - Documented `make test`, `make test-debuginfo`, MIR structural comparison, generated-program behavior checks, and debug-info buckets.
  - Updated stale `tests/o1/README.md` and `tests/o2/README.md` text from "reserved" to populated bucket descriptions.
- Validation:
  - `git diff --check`: passed.
  - `make -C pa37 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`: passed (`o1` 8/8, `o2` 10/10).
  - `make -C pa37 test-debuginfo CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_SKIP_DEV_REBUILD=1`: passed (`debuginfo/o1` 4/4, `debuginfo/o2` 4/4).
  - Note: an initial parallel attempt to run PA36 and PA37 debug-info targets together failed in the shared dev rebuild on `../obj/generated/cppgm_builtin_host_config.h.tmp`; rerunning the debug-info targets sequentially with `CPPGM_SKIP_DEV_REBUILD=1` passed.
- Open questions:
  - Whether debug-info buckets should be wired into the default PA37 report surface or remain behind `test-debuginfo`.
