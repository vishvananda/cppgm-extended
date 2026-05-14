# Debug Info Implementation Plan

## Status

Implemented for:

- `cppgm++ -g0` and `cppgm++ -gline-tables-only`
- PA13 debug metadata transport and debug-only validation lanes
- PA35 debug metadata transit tests through `-O1` and `-O2`
- object-file DWARF line tables and host-linked debugger smoke coverage

Intentionally out of scope:

- custom-linker debug section propagation
- full debugger-grade scopes, aggregates, and type coverage beyond the current
  scalar-focused support

## Scope

This plan covers emitted program debug information for the maintainer compiler
pipeline in `dev/`, with the goal of making source locations survive from
front-end analysis through object emission.

The intent is to stage this work so that:

1. the driver and test surface become explicit first
2. the IR contracts gain stable debug metadata before backend emission depends on them
3. optimization and backend work can preserve metadata without having to invent it later

This plan is intentionally incremental. The first implementation slices should
favor metadata plumbing and validation over immediately trying to emit full
debugger-grade DWARF.

## Current State

Relevant facts in the current codebase:

- token/source locations already exist in the preprocessor/tokenizer pipeline
- AST nodes retain token spans and semantic analysis can recover `file:line:col`
- emitted LowIR, MachineIR, and `machine_object::ObjectFile` do not currently
  carry source debug location payloads end to end
- the object writer already supports extra sections and relocations that are
  sufficient to host later DWARF-style metadata
- the driver accepts `-g*` today only as a benign ignored flag

The biggest technical gap is not object-section support. It is the loss of
location data between semantic output and the backend IR layers.

## Guiding Decisions

1. Start with a dedicated debug-info lane and keep it out of normal `make test`.
2. Treat line-table quality as the first shipping milestone.
3. Make PA13 own the text IR debug metadata surface before native object
   emission depends on it.
4. Make PA35 metadata preservation explicit. Optimizations may be allowed to
   coarsen metadata in some cases, but they should not silently erase it.
5. Keep the custom executable linker work optional at first. Object-file
   debuginfo and host-linked executables are the earlier milestone.

## Phase Order

### Phase 1: Driver And Validation Scaffolding

Goals:

- add an explicit debug-info mode to the `cppgm++` driver surface
- stop treating `-g*` as a silently ignored benign flag
- add a separate root validation lane, using a distinct target name such as
  `test-debuginfo`

Deliverables:

- parsed debug-info level/mode in the CLI/toolchain invocation path
- debug-info mode threaded to object-build entry points
- top-level debug-only make target that passes the root `CXX` and
  `CPPGM_HOST_CXX` settings through to submakes

Notes:

- this phase does not require emitted DWARF yet
- this is where target naming should stay intentionally distinct from the
  existing assignment-local `test-debug` smoke targets

### Phase 2: PA13 LowIR Metadata Surface

Goals:

- extend the PA13 LowIR contract to describe source debug metadata explicitly
- keep the syntax text-based and deterministic
- make parsing and dumping preserve the new metadata even before it affects codegen

Candidate surface:

- source file table / file ids at program scope
- optional function / block / instruction source location metadata
- optional variable / slot declaration provenance where useful later

Deliverables:

- updates to `pa13/README.md`, `pa13/lowir.md`, and `pa13.gram`
- `lowir_internal.*` parse/dump support for the chosen metadata form
- PA13 parser/round-trip tests in the debug-only lane

Notes:

- this phase is the right place to decide whether the syntax is inline metadata
  or table-driven references
- if the surface proves too broad for one patch, line-table-only instruction
  metadata should land first

### Phase 3: Semantic Capture And Ownership

Goals:

- preserve source location data past translation-unit semantic analysis
- stop dropping the `SourceLocationTable` immediately after `CallSemNode`
  construction

Deliverables:

- an analyzed-translation-unit owner that keeps both semantic output and source
  location data alive
- explicit location fields or references on semantic nodes that will be lowered
  into LowIR
- consistent handling for synthesized nodes and compiler-generated code

Notes:

- synthetic code should have an explicit policy, for example nearest enclosing
  source location or marked compiler-generated locations

### Phase 4: LowIR And PA35 Optimization Preservation

Goals:

- lower debug metadata from semantic output into LowIR
- make PA35 preserve metadata through optimization passes

Deliverables:

- location metadata attached to LowIR functions, blocks, and instructions as needed
- optimizer rules for when metadata is copied, merged, or dropped
- debug-only optimizer regression tests

Optional follow-up:

- richer value-tracking metadata for locals and temporaries can come later if
  line-table preservation lands first

Notes:

- this phase is optional only in ordering, not in eventual ownership
- the minimum acceptable outcome is that PA35 does not accidentally strip all
  debug locations during otherwise valid transforms

### Phase 5: MachineIR And Object Emission

Goals:

- preserve debug metadata into MachineIR
- emit object-file debug sections for supported native object formats

Deliverables:

- MachineIR location payloads or instruction-to-location mapping
- debug section emission in `lowir_object_backend.cpp`
- `machine_object.cpp` support for `.debug_*`/Mach-O debug sections with the
  right flags and relocation handling

Initial target:

- line tables plus compile-unit / subprogram skeleton metadata

Later target:

- local variables
- lexical scopes
- richer type DIEs

### Phase 6: Link And Runtime Follow-Through

Goals:

- decide how much debug metadata survives host linking versus the custom linker
- make the supported path explicit in docs and tests

Deliverables:

- documented supported modes for debug-info output
- optional custom-linker support for carrying debug sections into final native
  executables

Notes:

- this should start as an optional phase
- object files plus host-linked executables are a reasonable first supported
  boundary

## Testing Strategy

Debug-info validation should not run under the normal assignment `test` lane.
Use a separate target, currently planned as:

- root: `make test-debuginfo`
- assignment-local: `make test-debuginfo` where implemented

Suggested test groups:

1. CLI parsing and mode selection for `-g`, `-g0`, and `-gline-tables-only`
2. PA13 metadata parse/dump round-trip tests
3. PA35 preservation tests showing metadata survives representative optimizations
4. object inspection tests for emitted debug sections and relocations
5. debugger-smoke or external tool inspection tests where practical

## First Implementation Slice

The first slice should land only the scaffolding needed to make later work
safe to stage:

1. add the plan and tracker entry
2. add explicit debug-info parsing/plumbing in the driver
3. add the separate top-level debug-info validation target
4. start no-op propagation of debug-info options toward object emission

That gives the later PA13 and PA35 metadata work a stable execution lane and a
real option path to build on.
