# Host ABI Runtime Transition Plan

This broad transition plan is mostly implemented. The remaining hosted-EH and
fallback-removal work now lives in
[host-abi-runtime-followup-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-followup-plan.md).

## Goal

Move hosted/native output toward the real host ABI without forcing wholesale
LowIR reference churn.

Assignment ownership for this migration should be:

- **PA30** for ordinary host-ABI object/link/runtime interop
- **PA24** for the still-private exception/runtime ABI surfaces
- **PA32** only for hosted-header/source compatibility cases whose trigger is
  really the hosted source environment rather than the final link/runtime ABI

The key constraint is:

- current LowIR refs use reserved/private runtime names such as
  `@__cppgm_init`, `@__cppgm_fini`, `@__cppgm_eh_*`, and
  `cppgm_builtin_operator_*`
- we do not want to regenerate the LowIR test corpus just because final object
  symbol names change

So the intermediate strategy should be:

1. keep the current LowIR/runtime names stable as the frontend-facing contract
2. reinterpret those names as logical runtime roles below the IR
3. map selected roles to host ABI symbols during object/native emission
4. make ordinary hosted links use the same default runtime surface as host
   `clang++`
5. progressively remove test-time helper runtime objects from `pa32/tests/link`

## Current State

Today there are three layers mixed together:

1. **LowIR/runtime role spelling**
   - reserved names in LowIR and semantic lowering
2. **compiler-private runtime ABI**
   - `cppgm_builtin_operator_new`
   - `cppgm_builtin_operator_delete`
   - `cppgm_eh_top`
   - `cppgm_eh_value`
   - `cppgm_eh_type`
   - `cppgm_eh_unhandled`
3. **host runtime / host ABI**
   - host `operator new/delete`
   - libc/libc++ helpers
   - host exception ABI and unwinder

That layering leak is why `pa32/tests/link` previously needed per-test helper
objects for EH-owned cases such as:

- `646-eh-runtime-link-smoke`
- `675-hosted-dynamic-exception-spec-runtime`

But that should be treated as a transitional placement accident, not the long-
term assignment boundary. Ordinary host-runtime ownership belongs in PA30, while
private EH/runtime ownership belongs conceptually in PA24.

### Progress Snapshot

The transition has already moved several families onto the normal host ABI:

- allocation/deallocation builtins now remap to host `operator new/delete`
- plain libc/libm-style builtins such as `memcpy`, `memmove`, `memcmp`,
  `strlen`, `ceil*`, and `fabs*` now remap through the host link surface
- the CMake wrapper no longer injects any private runtime archive; it now lets
  `cppgm++` own compile/link staging and host-driver delegation
- these hosted PA32 link tests no longer need per-test helper runtime objects:
  - `644-hosted-cmath-ceilf-link-smoke`
  - `645-builtin-operator-new-delete-link-smoke`
  - `647-builtin-memcpy-strlen-link-smoke`
  - `651-hosted-unordered-map-string-int-link-smoke`
  - `652-hosted-unordered-set-pointer-link-smoke`
  - `654-hosted-forward-as-tuple-rvalue-ref-runtime`
  - `670-delete-class-pointer-destroys-object-runtime`
  - `672-hosted-ofstream-default-constructor-link-smoke`
  - `674-hosted-constructor-assert-preserves-this`
- hosted compile-only C++ objects that contain exception constructs now use a
  host-native EH object path, so the last helper-runtime link cases are gone:
  - `646-eh-runtime-link-smoke` has been replaced by
    `pa30/tests/spec/194-host-eh-unhandled-throw-smoke`
  - `675-hosted-dynamic-exception-spec-runtime` now links in `pa32` with no
    helper runtime object

Current staged reality:

- hosted `cppgm++` compile/link now follows a driver-style staged flow:
  source inputs compile to temporary host objects first, then final link
  delegates to the host toolchain on host targets
- that means direct source link, separate compile/link, and wrapper-driven link
  now follow the same hosted path
- the internal LowIR / machine-object EH path still uses `cppgm_eh_*`
- true self-emitted host EH is therefore **not** complete yet; we currently
  rely on staged host-toolchain compile/link for hosted targets instead of a
  self-emitted host EH backend

## Intermediate Design

The right intermediate step is **not** to change LowIR names first.

Instead:

- keep existing reserved/private spellings in LowIR and semantic lowering
- add one backend/runtime mapping layer that classifies those spellings into
  runtime roles
- let object/native emission decide whether each role becomes:
  - a private runtime symbol
  - a direct host ABI symbol
  - or a direct host libcall/runtime import
- make the default hosted-link policy be: "whatever host `clang++` would
  normally provide", not a per-test runtime selection scheme

This gives us host-ABI progress without changing the textual LowIR references.

## Runtime Role Families

We should split the current runtime surface into families.

### Family A: Safe Early Host-ABI Migration

These are the best first candidates for direct host ABI lowering:

- ordinary `operator new`
- ordinary `operator delete`
- `operator new[]`
- `operator delete[]`
- aligned/sized forms where the target host toolchain supports them
- plain libc-style helpers when the target/compiler contract is already clear:
  - `memcpy`
  - `memmove`
  - `memset`
  - `strlen`
  - possibly simple math builtins already mirrored by host libc/libm

Why these first:

- the symbol contracts are comparatively stable
- they are already expected to be provided by the host runtime in normal C++
- they are the source of several current PA32 helper-library hacks

### Family B: Keep Private For Now

These should stay on the compiler-private runtime path until we are ready for a
real ABI implementation:

- `@__cppgm_init`
- `@__cppgm_fini`
- `cppgm_eh_*`
- exception throw/resume/catch support
- any private RTTI/vtable-runtime support still tied to our internal lowering

Why not migrate these first:

- init/fini is object-format and runtime-mechanism specific
- EH is ABI-heavy and easy to get subtly wrong
- these areas are where a half-migration would create confusing hybrids

## Phase Plan

### Phase 0: Make The Mapping Explicit

Add a single runtime symbol classification table in the backend layer.

Inputs:

- current lowered symbol spellings from semantic lowering / LowIR

Outputs:

- runtime role kind
- migration policy
  - `private_runtime`
  - `host_abi`
  - `host_libcall`
  - `reserved_internal`

This should become the only place that knows facts like:

- `cppgm_builtin_operator_new` means allocation role
- `cppgm_builtin_operator_delete` means deallocation role
- `cppgm_eh_unhandled` means private EH fallback role

Do not scatter symbol-remap conditionals through multiple codegen/link paths.

### Phase 1: Direct Host-ABI Remap For Allocation

Teach object/native emission to remap:

- `cppgm_builtin_operator_new`
- `cppgm_builtin_operator_new_aligned`
- `cppgm_builtin_operator_new_array`
- `cppgm_builtin_operator_new_array_aligned`
- matching delete forms

to the true host ABI allocation/deallocation entrypoints.

Important constraint:

- LowIR and semantic lowering still produce the current reserved/private names
- only final emitted symbol references change

Expected result:

- ordinary `new`/`delete` code stops needing `host_builtin_runtime.cpp`
- `pa32` allocation tests can stop linking `libruntime.o`
- the wrapper no longer needs to inject allocation helpers

### Phase 2: Make Host `clang++` The Default Host-Interop Policy

Once allocation remap works, the default host-interop behavior should be:

- invoke host `clang++` for the final link
- get the normal default host runtime surface
- add only ordinary link flags like `-pthread` when a test requires them

In other words, host-ABI tests should stop declaring runtime granularity unless
they are intentionally testing a remaining private-runtime ownership boundary.

After that, reorganize the relevant tests into three categories.

#### 1. Pure Host-ABI Link Smokes (`PA30`)

These should link with just host toolchain inputs and no helper runtime object:

- [645-builtin-operator-new-delete-link-smoke.t.1](/Users/vishvananda/cppgm/pa32/tests/link/645-builtin-operator-new-delete-link-smoke.t.1)
  - likely rewrite/rename away from builtin spelling toward ordinary `new/delete`
- [669-cmake-wrapper-new-delete-link-smoke.t.1](/Users/vishvananda/cppgm/pa32/tests/link/669-cmake-wrapper-new-delete-link-smoke.t.1)
- [670-delete-class-pointer-destroys-object-runtime.t.1](/Users/vishvananda/cppgm/pa32/tests/link/670-delete-class-pointer-destroys-object-runtime.t.1)

Desired invariant:

- no `.lib.runtime.cpp`
- no `__LIBDIR__/libruntime.o`
- host link succeeds because emitted symbols are already host-native

#### 2. Private-Runtime Contract Smokes (`PA24`)

These should eventually be explicit LowIR/runtime-boundary checks rather than
host-link helper hacks. There is no remaining hosted-link helper test in this
bucket now; the old `646-eh-runtime-link-smoke` ownership moved to the PA30
host-link surface once compile-only hosted EH stopped requiring helper objects.

#### 3. Hosted-Library Compatibility Smokes (`PA32`)

These should gradually stop depending on per-test runtime source files and
should default to ordinary host `clang++` linking:

- [644-hosted-cmath-ceilf-link-smoke.t](/Users/vishvananda/cppgm/pa32/tests/link/644-hosted-cmath-ceilf-link-smoke.t)
- [647-builtin-memcpy-strlen-link-smoke.t.1](/Users/vishvananda/cppgm/pa32/tests/link/647-builtin-memcpy-strlen-link-smoke.t.1)
- [651-hosted-unordered-map-string-int-link-smoke.t.1](/Users/vishvananda/cppgm/pa32/tests/link/651-hosted-unordered-map-string-int-link-smoke.t.1)
- [652-hosted-unordered-set-pointer-link-smoke.t.1](/Users/vishvananda/cppgm/pa32/tests/link/652-hosted-unordered-set-pointer-link-smoke.t.1)
- [654-hosted-forward-as-tuple-rvalue-ref-runtime.t.1](/Users/vishvananda/cppgm/pa32/tests/link/654-hosted-forward-as-tuple-rvalue-ref-runtime.t.1)

Desired invariant:

- no per-test helper runtime source file
- no per-test `libruntime.o`
- success/failure is determined by the symbols that host `clang++` would
  normally resolve, plus any explicit ordinary link flags like `-pthread`

### Phase 3: Replace Per-Test Runtime Source Files With A Host-Default Harness

Remove the current pattern:

- `*.lib.runtime.cpp`
- `*.link.flags` containing `__LIBDIR__/libruntime.o`

Replace it with a simpler rule in
[run_cpphostinterop_tests_worker.pl](/Users/vishvananda/cppgm/scripts/run_cpphostinterop_tests_worker.pl):

- ordinary host-interop link tests use plain host `clang++` linking by default
- tests may still use `.link.flags` for normal link options such as `-pthread`
- only rare private-runtime ownership tests get an explicit special path

That keeps the common case aligned with the actual host environment instead of
making every test describe a runtime slice more granular than `clang++` itself
exposes.

### Phase 4: Shrink The Wrapper Runtime Archive

After phases 1 through 3:

- remove allocation/deallocation helpers from the wrapper archive
- remove generic builtin libc helpers as they move to host libc/libm
- keep only the genuinely private runtime surfaces still required

Target progression:

1. full legacy archive
2. EH-only plus a small residual private helper set
3. eventually no wrapper runtime archive for ordinary hosted compilation/link

### Phase 5: Replace The Staged Host-Compile EH Path With True Self-Emitted Host EH

The current hosted EH path removes helper runtimes, but it does **not** yet
mean our own object backend emits native zero-cost EH metadata. The remaining
project is:

1. Extend the machine-object model beyond code/data only.
   It must preserve auxiliary host sections and relocations needed for EH,
   especially `.eh_frame`, `__gcc_except_tab`, and the Mach-O / ELF unwind
   metadata that currently gets dropped on import.

2. Add structured EH support roles below LowIR.
   We should stop hardwiring `cppgm_eh_*` meaning into object emission and
   introduce backend-visible roles for:
   - personality
   - throw allocation / throw entry
   - catch begin/end
   - rethrow / unwind resume
   - catch-type / LSDA payloads

3. Teach the C++ lowering/backend path to emit host EH control flow directly.
   For the hosted object path this means replacing the current
   `IK_EH_TRY` / `IK_THROW` / `IK_RESUME` lowering that becomes
   `MI_EH_PUSH` / `MI_THROW` / `MI_RESUME` over `cppgm_eh_*` with a path that
   produces host ABI calls plus real unwind metadata.

4. Preserve and round-trip host EH metadata in object parsing/writing.
   Today the object parser explicitly ignores relocation-bearing metadata
   sections such as `.eh_frame`. That has to change before self-emitted host EH
   can survive separate compilation and relinking.

5. Only after 1 through 4 is stable, retire the hosted compile fallback.
   At that point `cppgm++ -c` should still pass the PA30/PA32 EH smokes, but
   the emitted objects will be fully self-emitted rather than delegated to the
   host compiler for EH-bearing sources.

## PA32 Test Replacement Plan

### Immediate Cleanup Target

Replace hand-authored per-test runtime objects with a host-default link policy.

Current hacky pattern:

- `*.lib.runtime.cpp`
- `*.link.flags` containing `__LIBDIR__/libruntime.o`

Replacement:

- plain host `clang++` linking as the default
- test-specific `.link.flags` only for normal link options such as `-pthread`
- a small explicit exception path for the few tests that still validate
  private-runtime ownership

As part of this cleanup, the durable homes should be:

- move ordinary host-ABI ownership tests from `pa32/tests/link` to `pa30/tests`
- keep only the truly hosted-source-triggered libc++/vendor tests in `pa32`
- move private EH/runtime ownership checks to the PA24 runtime/EH surface when
  practical

### Suggested Migration By Test

#### Convert To Plain Host `clang++` Linking And Move To `PA30`

- `669-cmake-wrapper-new-delete-link-smoke`
- `670-delete-class-pointer-destroys-object-runtime`
- eventually `645-builtin-operator-new-delete-link-smoke` should be retired or
  rewritten as an ordinary host-ABI allocation smoke

These are fundamentally host-runtime ownership tests, not PA32 hosted-header
tests.

#### Keep As Explicit Private-Runtime Ownership Tests And Move Toward `PA24`

- `668-cmake-wrapper-simple-throw-link-smoke`

These are the rare cases where a specialized helper path is still justified,
because the point of the test is to verify the private-runtime contract itself.

#### Keep In `PA32` As Hosted libc++ / Runtime Smokes Without Helpers

- `675-hosted-dynamic-exception-spec-runtime`

If a given test stops being about hosted-header/source compatibility and becomes
purely a host-runtime ownership check, move it back to `PA30`.

## Implementation Notes

### Avoid LowIR Ref Churn

Do **not** change:

- reserved LowIR hook names
- textual LowIR refs in earlier PAs

during the first migration.

Treat those names as stable intermediate/runtime roles until LowIR itself grows
explicit declaration/import/role metadata.

### Keep One Ownership Table

Add one shared classification/mapping table for:

- reserved runtime hooks
- private runtime helper symbols
- host-ABI remappable symbols

This table should drive:

- object/native emission remaps
- which symbols are expected to resolve through the normal host link surface
- which rare tests still need explicit private-runtime injection
- wrapper runtime injection decisions
- future diagnostics about “private runtime symbol leaked into final object”

It should also drive assignment placement decisions:

- host-runtime ownership issue -> `PA30`
- private EH/runtime ABI ownership issue -> `PA24`
- hosted-header/source compatibility issue -> `PA32`

### Add Regression Coverage At Each Step

For each migrated family, add two kinds of tests:

1. positive hosted link smoke with plain host `clang++` linking
2. negative ownership smoke proving we no longer emit the private symbol name

Examples for `new/delete`:

- ordinary `new/delete` host-link smoke with no helper runtime object
- inspect step that confirms final object/link references host ABI allocation
  symbols rather than `cppgm_builtin_operator_*`

## Recommended Order

1. explicit runtime symbol classification table
2. direct host remap for `new/delete`
3. move ordinary host-runtime ownership coverage toward `PA30`
4. replace PA32 per-test runtime objects with plain host-link defaults
5. direct host remap for libc/libm-style builtin helpers
6. shrink wrapper archive
7. defer EH migration until later

This gets us real host-ABI progress, removes most of the ad hoc PA32 runtime
test machinery, puts the tests under the right assignment owners, and avoids
churn in the LowIR reference corpus.
