# Host ABI Runtime Follow-Up Plan

Completed on `2026-04-07`.

Final outcome:

- host source-object compile fallback machinery was removed from the
  implementation
- bootstrap helper scripts/docs no longer depend on fallback-disable honesty
  guards
- runner-backed batch validation passed across `pa1` through `pa32`
- hosted `pa30` / `pa32` validation now runs on the normal self-emitted object
  path

This is the remaining follow-up to the archived broad transition plan:
[implemented/host-abi-runtime-transition-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-transition-plan.md).

The broad host-ABI migration mostly landed already:

- ordinary hosted links now follow the host `clang++` surface
- allocation and common libc/libm-style builtin families no longer require the
  old per-test runtime object pattern
- the wrapper now behaves like a host-driver integration layer instead of a
  private runtime injector

The main missing slice is the remaining hosted runtime/backend work needed to
remove the temporary host source-object compile fallbacks entirely. Hosted EH
is still the largest unfinished part, but it is not the only reason we still
delegate source files to the host compiler today.

## Goal

Finish the remaining hosted runtime/backend work so that:

- `cppgm++ -c` always emits its own host-linkable objects for hosted inputs
- hosted runtime-sensitive sources no longer depend on staged host compilation
  of source files
- bootstrap and other validation flows no longer need special fallback-disable
  honesty guards

The intended end state is:

- host linker: yes
- host compiler fallback for hosted source files: no

## Current Gap

Today, hosted source files can still bypass our object emission
path:

- [`write_cpp_object_file(...)`](/Users/vishvananda/cppgm/dev/src/cpp_toolchain.cpp#L328)
  calls
  [`write_host_cpp_object_file(...)`](/Users/vishvananda/cppgm/dev/src/cpp_toolchain.cpp#L234)
  when
  [`can_use_host_native_compile_fallback(...)`](/Users/vishvananda/cppgm/dev/src/cpp_toolchain.cpp#L173)
  returns true
- direct source-link mode can escalate this to all source inputs in one link
  invocation through
  [`force_host_source_objects`](/Users/vishvananda/cppgm/dev/src/cpp_driver_frontend.cpp#L91)

That predicate currently covers two broad families:

- hosted EH-bearing programs
- hosted programs that still rely on host std-vtable/runtime object behavior

That means the hosted path is still mixed:

- final link: host-native
- ordinary object emission: self-emitted
- runtime-sensitive hosted source objects: sometimes host-compiled

This is transitional behavior, not the target PA30/PA32 contract.

## Remaining Work

The remaining work has two coupled slices:

- finish the hosted EH/backend path
- finish the remaining hosted polymorphic/runtime object-emission path so
  source files no longer need host compilation for vtable/RTTI/runtime reasons

### 1. Preserve Hosted Polymorphic/Runtime Object Semantics In Self-Emitted Objects

The self-emitted object path must stop depending on host compilation for
ordinary hosted runtime surfaces such as polymorphic libc++ code and iostream /
locale / streambuf-heavy call paths.

We need correct self-emission for:

- host std-vtable address points
- RTTI and imported polymorphic support objects
- runtime bridge calls that must stay on the host ABI path
- imported data / relocation patterns that currently only behave correctly when
  the source file is delegated to the host compiler

Without that, turning off host-native compile fallback will keep exposing
runtime mismatches long before we reach hosted EH.

### 2. Preserve Host EH Metadata In The Machine-Object Path

The machine-object layer must stop treating EH metadata as disposable.

We need to preserve and round-trip:

- `.eh_frame`
- `__gcc_except_tab`
- Mach-O / ELF unwind metadata
- relocations attached to those sections

Without that, self-emitted hosted EH cannot survive separate compilation and
final host relink.

### 3. Introduce Structured Hosted Runtime Roles Below LowIR

We should stop treating ad hoc spellings as the only backend contract for
runtime-sensitive hosted code.

Instead, the backend-visible runtime classification layer should explicitly
model roles such as:

- hosted vtable / RTTI / imported-runtime bridge roles
- personality
- throw allocation / throw entry
- catch begin / end
- rethrow
- unwind resume
- catch-type / LSDA payload support

This keeps frontend-facing spellings stable while making the backend migration
explicit.

### 4. Emit Hosted Runtime And EH Control Flow From Our Own Backend

The hosted object path needs a real self-emitted implementation for both:

- ordinary hosted runtime-sensitive code
- EH-bearing hosted code

That means replacing the current staged escape hatch with backend emission that
produces:

- host ABI calls where appropriate
- real unwind metadata
- self-owned hosted object files that the host linker accepts directly

### 5. Remove All Host Source Compile Fallbacks

Only after the hosted runtime and EH backend paths are stable should we delete
the fallback machinery:

- `CPPGM_DISABLE_HOST_NATIVE_COMPILE_FALLBACK`
- `CPPGM_DISABLE_HOST_EXCEPTION_COMPILE_FALLBACK`
- `host_native_compile_fallback_fully_disabled()`
- `host_exception_compile_fallback_disabled()`
- `can_use_host_native_compile_fallback(...)`
- `cpp_source_requires_host_native_object(...)`
- `force_host_source_objects`
- the `write_host_cpp_object_file(...)` escape hatch for hosted source
  compilation

### 6. Collapse The Bootstrap Special Case

Once the fallbacks are gone, remove the bootstrap-only disable flag from the
root [Makefile](/Users/vishvananda/cppgm/Makefile).

At that point bootstrap host-link should be honest without any source-object
special casing.

## Suggested Order

1. Finish the hosted polymorphic/runtime object-emission fixes currently hidden
   by host-native source fallback.
2. Preserve EH metadata and relocations in the machine-object layer.
3. Add explicit hosted runtime-role mapping below LowIR.
4. Teach the hosted backend path to emit host-compatible hosted objects
   directly, including EH-bearing ones.
5. Delete all source-object fallback machinery and bootstrap-only guards.

## Validation

This follow-up is complete only when all of the following are true:

- `cppgm++ -c` emits self-owned objects for hosted inputs that previously fell
  back due to EH or host-native runtime/vtable reasons
- PA30 hosted runtime / EH tests pass without host source compilation
- PA32 hosted compatibility tests pass without host source compilation
- no source-link path silently upgrades source inputs to host-compiled objects

## Regression Surface

The main regression surface should be:

- existing PA30 hosted runtime / EH tests
- existing PA32 hosted compatibility tests, especially the hosted vtable,
  imported-data, and fallback-disable smokes
- broader hosted/runtime validation once the fallbacks are deleted

Any missing durable coverage discovered during this work should be added to the
earliest owning PA, not hidden behind bootstrap-only validation.
