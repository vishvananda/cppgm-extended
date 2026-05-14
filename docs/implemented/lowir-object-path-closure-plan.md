# LowIR Object-Path Closure Plan

## Problem

The current toolchain has two different ways to build LowIR from C++ sources:

- source-facing LowIR:
  - `generate_lowir_from_cpp_sources(...)` in
    [dev/src/cpp_toolchain.cpp](/tmp/cppgm-debug-info/dev/src/cpp_toolchain.cpp:435)
  - builds with `build_lowir_program(..., false, false, ...)`
- object-path LowIR:
  - `generate_object_lowir_from_cpp_sources(...)` and object generation in
    [dev/src/cpp_toolchain.cpp](/tmp/cppgm-debug-info/dev/src/cpp_toolchain.cpp:454)
  - build with `build_lowir_program(..., true, true, ...)`

That means object generation is not currently driven by a pure
`LowIR -> object-ready LowIR -> native` pipeline.

Instead, object generation rebuilds LowIR from semantic data with extra knobs:

- `validate_closure`
- `emit_runtime_support`

Those knobs currently change program content, not just checking or formatting.

## Required Invariant

All information needed by the backend must be recoverable from LowIR itself.

Acceptable:

- `source -> LowIR -> object-lowir` as a pure LowIR-to-LowIR pass
- `source -> LowIR -> native` where the native path only consumes LowIR plus
  normal backend configuration such as target, CPU, debug level, and
  optimization level

Not acceptable:

- rebuilding LowIR from `CallSemNode` with extra semantic-only switches in
  order to expose EH, RTTI, external runtime symbols, or other backend-visible
  behavior
- backend-relevant behavior that cannot be reproduced from parsed textual LowIR

## Current Violations

The current split is not just optimization or validation.

`emit_runtime_support_` in
[dev/src/lowirgensemantic.cpp](/tmp/cppgm-debug-info/dev/src/lowirgensemantic.cpp:12967)
changes emitted program meaning in several ways:

- exception support is collected from the semantic tree in
  `collect_exception_support(...)`
- host EH/runtime imports are introduced there, for example
  `__gxx_personality_v0`, `_Unwind_Resume`, `__cxa_throw`
- host RTTI objects and aliases are synthesized from semantic type/layout
  information in `ensure_host_class_rtti_global(...)`
- external symbol aliases are registered from semantic-time discovery in
  `register_external_symbol_aliases()`

The result is that:

- `--emit-lowir` is not a lossless representation of what object generation
  really compiles
- `--emit-object-lowir` is not currently “LowIR after an object prep pass”
- some runtime-visible behavior can only be reconstructed from `CallSemNode`

## Design Goal

Make one canonical LowIR program the semantic-output contract.

From there:

- `--emit-lowir` dumps that canonical LowIR
- object generation runs only pure LowIR passes after parsing that LowIR
- `--emit-object-lowir` becomes a debugging view of
  `prepare_object_lowir(parse_lowir(--emit-lowir))`

This is the same architectural shape Clang uses with LLVM IR:
ABI/runtime-relevant information must be encoded into IR before machine
lowering starts.

## Proposed End State

Introduce an explicit object-preparation pass:

- `prepare_object_lowir(lowir_internal::Program, ObjectLowIROptions) -> Program`

This pass may:

- validate symbol closure
- add derived external declarations or aliases
- materialize object-path runtime support globals/functions
- normalize debug visibility
- run the shared LowIR optimizer

But it must take only:

- parsed LowIR
- target/backend options

It must not consult:

- `CallSemNode`
- AST/type graph side tables that were not serialized into LowIR
- semantic-only closure scans

## What Must Move Into LowIR

The current semantic-only information falls into three buckets.

### Bucket 1: Must Be Explicitly Serialized In Canonical LowIR

These are backend-visible facts that cannot be guessed safely later:

- function EH personality / unwind requirements
- whether a function may unwind through cleanups
- explicit runtime helper calls used for throw/catch/typeid/dynamic_cast paths
- RTTI object graphs that codegen may reference
- external symbol identities needed for backend emission

For these, the canonical LowIR should already contain concrete declarations,
globals, metadata, or attributes.

### Bucket 2: May Be Derived From Canonical LowIR

These are fine as pure LowIR-to-LowIR preparation:

- validating that all referenced symbols have closure owners
- adding obvious external declarations for already-referenced runtime symbols
- dropping debug locations when `-g` is off
- running LowIR optimization

### Bucket 3: Backend Configuration, Not Program Semantics

These may remain out-of-band:

- target triple / output target
- CPU / feature set
- optimization level
- debug level
- relocation/object format choices

## Recommended Migration

Do this in four phases.

### Phase 1: Freeze And Audit The Semantic Side Channel

Goal:

- identify every place where object-path output depends on semantic data that
  is not present in canonical LowIR

Work:

- inventory all `emit_runtime_support_` and `validate_closure_` branches in
  [dev/src/lowirgensemantic.cpp](/tmp/cppgm-debug-info/dev/src/lowirgensemantic.cpp:11728)
- classify each branch into the three buckets above
- add a failing audit test that compares:
  - `cppgm++ --emit-object-lowir`
  - `prepare_object_lowir(parse_lowir(cppgm++ --emit-lowir))`

Acceptance:

- we have a complete checklist of semantic-only object-path dependencies
- no new object-path semantic dependency can land untracked

### Phase 2: Make Canonical LowIR Lossless For Backend Use

Goal:

- serialize all Bucket 1 information in canonical LowIR

Likely changes:

- extend LowIR syntax and parser/dumper for:
  - function-level unwind / personality metadata
  - explicit external symbol identity annotations
  - RTTI/import declarations where object code may reference them
- change semantic lowering to emit those items unconditionally into canonical
  LowIR instead of only under `emit_runtime_support_`

Important rule:

- if object generation needs a symbol or metadata edge, that edge must already
  exist in dumped LowIR

Acceptance:

- parsing dumped `--emit-lowir` preserves all backend-relevant edges
- key EH/RTTI programs no longer require semantic rebuild to expose runtime
  support

### Phase 3: Replace Semantic Rebuild With Pure LowIR Preparation

Goal:

- remove the object-path semantic build fork

Work:

- add `prepare_object_lowir(...)` in a new shared toolchain file, likely under
  `dev/src/`
- rewrite:
  - `generate_object_lowir_from_cpp_sources(...)`
  - `build_cpp_object_file(...)`
  - `write_cpp_object_file(...)`
  so they all follow:
  - source -> canonical LowIR
  - canonical LowIR -> object prep pass
  - object prep output -> backend
- stop using `build_lowir_program(..., true, true, ...)` as the object path
- narrow `build_lowir_program(...)` so its options only control semantic
  emission that is part of canonical LowIR, not object-only content

Acceptance:

- `--emit-object-lowir` is produced from canonical LowIR only
- object generation consumes the same prepared LowIR that `--emit-object-lowir`
  prints
- no backend path depends on `CallSemNode` after canonical LowIR is built

### Phase 4: Tighten Contracts And Student Surface

Goal:

- make the ownership boundary obvious in tests and docs

Work:

- keep `--emit-lowir` as the primary student-facing contract unless an
  assignment explicitly teaches object-path closure
- document `--emit-object-lowir` as backend/debugging output if it remains
  exposed
- add round-trip tests that prove:
  - `cppgm++ --emit-lowir foo.cpp`
  - parsed LowIR + object prep
  - native output
  matches direct `cppgm++ -c foo.cpp`

Acceptance:

- students are not asked to reason about hidden object-path semantics
- internal debugging output is clearly separated from canonical assignment
  output

## Test Plan

Add a dedicated ownership family for this in later backend tests.

Minimum cases:

- `throw 1`
- `try` / `catch`
- throw with destructible locals requiring unwind cleanup
- `typeid`
- `dynamic_cast` on reference types
- class RTTI with inheritance
- pointer-valued runtime globals / external symbol aliases

For each case, test three properties:

1. canonical `--emit-lowir` contains all required backend-relevant edges
2. `prepare_object_lowir(parse_lowir(canonical))` matches object-path dump
3. native output from prepared LowIR matches direct object generation

## Recommended Code Changes

The highest-value mechanical changes are:

1. Introduce a new shared `prepare_object_lowir(...)` pass and route all object
   paths through it.
2. Remove object-only semantic branching from `build_lowir_program(...)`.
3. Move runtime-support discovery from semantic-tree scans to explicit LowIR
   references and declarations.
4. Extend textual LowIR so the canonical dump is lossless for backend
   preparation.
5. Keep closure validation as a pure LowIR pass, not a semantic build mode.

## Risks

The hard part is RTTI/EH support.

That logic currently uses semantic type/layout information directly, so this
will likely require either:

- richer explicit RTTI objects in canonical LowIR, or
- a deliberately smaller canonical contract where semantic lowering emits the
  exact runtime-support globals/functions directly

The second option is preferred. It is simpler and matches the desired
invariant: if the backend needs it, canonical LowIR should already say it.

## Non-Goals

This plan does not attempt to:

- redesign LowIR optimization
- change the native backend ABI configuration surface
- change MIR/native debug-only serialization limitations

It is only about removing the semantic side channel between canonical LowIR and
object generation.
