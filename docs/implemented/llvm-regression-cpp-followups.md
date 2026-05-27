# LLVM `Regression/C++` Follow-Ups

Status note:

- This follow-up set is complete for the captured `SingleSource/Regression/C++`
  bucket.
- The original failure inventory has been reduced to repo-owned regressions and
  fixes, and the document now records an all-green closure state (`31 / 31`,
  `0 / 31` remaining).

This document is archived as implemented/closed.

This note captures the current failures from LLVM test-suite
`SingleSource/Regression/C++` after switching the CMake wrapper to:

- use `cppgm++` for `-c` / `-E`
- use `cppgm++` as the compiler driver for final link, with host-toolchain
  delegation underneath on hosted targets

Reference invocation:

```bash
cmake -G "Unix Makefiles" \
  -DTEST_SUITE_SUBDIRS='SingleSource/Regression/C++' \
  -DTEST_SUITE_RUN_BENCHMARKS=OFF \
  -DTEST_SUITE_COLLECT_CODE_SIZE=OFF \
  -DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang \
  -DCMAKE_CXX_COMPILER=/Users/vishvananda/cppgm/scripts/cppgm-cmake-wrapper.sh \
  /Users/vishvananda/llvm-test-suite

make -k -j8
```

Current snapshot:

- `31 / 31` regression targets now build successfully
- `0 / 31` still needs work or reduction tests

Recently fixed in repo-native regressions:

- `simple_throw.cpp`: fixed via `pa29` runtime smoke `293-runtime-throw-catch-ellipsis` plus wrapper-link smoke `668-cmake-wrapper-simple-throw-link-smoke`
- `ctor_dtor_count-2.cpp`: fixed via `pa29` runtime smokes `294-runtime-thrown-class-catch-by-value` and `295-runtime-unnamed-catch-by-value-copy-lifetime`
- `ctor_dtor_count.cpp`: fixed via `pa29` runtime smokes `296-runtime-ctor-throw-destroys-constructed-member`, `297-runtime-discarded-class-prvalue-destruction`, and `298-runtime-ctor-member-throw-destroys-prior-member`, plus `pa32` link/runtime smoke `670-delete-class-pointer-destroys-object-runtime`; constructor throws now unwind both direct and nested partially-constructed subobjects, discarded class prvalues destroy their completed subobjects, and plain / thrown class-pointer deletes destroy the allocated object before deallocation
- `2003-05-14-expr_stmt.cpp`: fixed via `pa29` runtime smoke `299-runtime-gnu-statement-expression-class-rhs`; GNU statement-expressions now parse, analyze, and materialize class-valued results through LowIR
- `recursive-throw.cpp`, `throw_rethrow_test.cpp`, and `inlined_cleanup.cpp`: now build and run successfully after the EH runtime / cleanup fixes already landed in `dev/`
- `dead_try_block.cpp` and `simple_rethrow.cpp`: now build successfully after the EH runtime / `catch (...)` / rethrow fixes already landed in `dev/`
- wrapper-linked programs now route ordinary allocation/deallocation through the host runtime surface via `pa32` link smoke `669-cmake-wrapper-new-delete-link-smoke`
- `exception_spec_test.cpp`: fixed via `pa32` hosted link/runtime smoke `675-hosted-dynamic-exception-spec-runtime`; hosted `std::set_terminate` / `std::set_unexpected` now mangle to host-compatible `std` free-function symbols, dynamic exception specifications dispatch disallowed escapes through `std::unexpected` / `std::terminate`, and the final hosted link no longer needs a helper EH runtime object

Each row below names the original LLVM source file so it can be rerun directly and reduced into a repo-native regression.

## Parser / Frontend Gaps

| Source file | LLVM target | Current failure | Suggested smaller regression to add |
| --- | --- | --- | --- |
| `global_type.cpp` | `Regression-C++-global_type` | fixed via `pa14` LowIR regression `214-global-array-one-past-end-pointer`; global pointer initializers now lower array decay plus constant one-past-end offsets in static storage | keep the original LLVM source as the hosted compile smoke when touching global address lowering |
| `function_try_block.cpp` | `Regression-C++-function_try_block` | fixed via `pa29` runtime smokes `300-runtime-free-function-try-block`, `301-runtime-constructor-function-try-block`, and `302-runtime-constructor-function-try-block-catches-init-throw`; free-function and constructor function-try-blocks now parse, catch mem-initializer throws, unwind constructed subobjects, and auto-rethrow from constructor handlers | keep the original LLVM file as a full mixed EH smoke when touching constructor lowering |
| `class_hierarchy.cpp` | `Regression-C++-class_hierarchy` | fixed via `pa32` hosted compile smoke `673-hosted-dynamic-array-new-bound-expression` plus hosted link/runtime smoke `674-hosted-constructor-assert-preserves-this`; hosted `new[]` bounds now lower directly from the semantic AST, and discarded `assert` conditionals no longer materialize bogus `void` result slots that could clobber constructor locals | keep the original LLVM source as a full hosted EH/object-model smoke when touching hosted assert lowering or dynamic array new-expression semantics |

## Object Model / Virtual / Member Pointer Gaps

| Source file | LLVM target | Current failure | Suggested smaller regression to add |
| --- | --- | --- | --- |
| `2003-06-08-VirtualFunctions.cpp` | `Regression-C++-2003-06-08-VirtualFunctions` | fixed via `pa32` link smoke `671-polymorphic-constructor-vtable-link-smoke`; non-template polymorphic classes with constructor/destructor definitions now claim and emit weak vtables even when their virtual bodies are only implicitly-inline | keep the original LLVM source as a vtable-ownership smoke when changing class output |
| `pointer_method2.cpp` | `Regression-C++-pointer_method2` | fixed via `pa29` runtime smoke `303-runtime-virtual-member-pointer-multiple-inheritance`; virtual member-function pointers now lower to dispatch thunks so multiple-inheritance calls preserve both base adjustment and virtual dispatch instead of taking direct base method addresses | keep the original LLVM source as the hosted compile/link smoke when touching member-function pointer lowering |

## EH Runtime / Cleanup / Unwind Gaps

| Source file | LLVM target | Current failure | Suggested smaller regression to add |
| --- | --- | --- | --- |
| `simple_throw.cpp` | `Regression-C++-simple_throw` | fixed via `pa29` runtime smoke `293-runtime-throw-catch-ellipsis` plus wrapper-link smoke `668-cmake-wrapper-simple-throw-link-smoke` | smallest possible `throw int` / `catch (...)` runtime smoke |

## Hosted libc++ / Builtin Coverage Gaps

| Source file | LLVM target | Current failure | Suggested smaller regression to add |
| --- | --- | --- | --- |
| `ofstream_ctor.cpp` | `Regression-C++-ofstream_ctor` | fixed via `pa32` link smoke `672-hosted-ofstream-default-constructor-link-smoke`; hosted default construction of `std::ofstream` now links after carrying libc++ explicit-instantiation suppression through semantic output and emitting host-compatible imported special-member manglings for libc++ inline namespaces | keep the original LLVM source as a hosted libc++ link smoke when touching imported class special members or explicit instantiation suppression |

## Passed `Regression/C++` Cases

These already built in the current setup and do not need immediate reduction:

- `2003-05-14-array-init.cpp`
- `2003-06-08-BaseType.cpp`
- `2003-06-13-Crasher.cpp`
- `2003-08-20-EnumSizeProblem.cpp`
- `2003-09-29-NonPODsByValue.cpp`
- `2008-01-29-ParamAliasesReturn.cpp`
- `2011-03-28-Bitfield.cpp`
- `BuiltinTypeInfo.cpp`
- `ConditionalExpr.cpp`
- `custom_section_members.cpp`
- `ctor_dtor_count-2.cpp`
- `dead_try_block.cpp`
- `fixups.cpp`
- `global_ctor.cpp`
- `pointer_member.cpp`
- `pointer_method.cpp`
- `short_circuit_dtor.cpp`
- `simple_rethrow.cpp`
- `simple_throw.cpp`
