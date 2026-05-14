# Direct Source Witness Plan

## Goal

Make `cppgm++ --emit-lowir --template-log <path>` render source witness output
from direct structured witness events emitted at the real semantic/template
decision sites.

The end state is:

- source witness does not depend on parsing `trace_events`
- closure witness continues to use structured `log_events`
- rendering does only light output work:
  - path normalization
  - spelling cleanup
  - deterministic ordering / small dedupe where the semantic layer still emits
    equivalent events more than once

## Why

The current trace-derived source witness path is brittle:

- constructor-template selection does not reliably flow through the ordinary
  `call-candidates` / `call-match-accept` trace path
- direct `class-use` / `alias-use` / `variable-use` producers already have
  structured data before they stringify it
- function overload selection already has the chosen binding, viable counts,
  drops, and argument/binding state in memory

So the trace text is no longer the right interface.

## Stages

### Stage 1: Session And Source Event Infrastructure

- add structured source witness event storage to `TemplateWitnessSession`
- add source witness binding / drop / event helpers
- update the renderer to consume structured source events
- keep a temporary trace fallback only for function-call events not yet
  converted

Validation:

- `make -C dev cppgm++`
- direct canaries should still run:
  - `104-extern-template-function-call-suppresses-materialization`
  - `017-constructor-template-cross-specialization`
  - `125-default-argument-instantiation-independence`

### Stage 2: Direct Use Events

- emit structured `class-use` events from `callsemantic.cpp`
- emit structured `alias-use` events from `callsemantic.cpp`
- emit structured `variable-use` events from `template_instantiation.cpp`
- stop depending on trace parsing for those event kinds

Validation:

- focused strict/spec tests covering class / alias / variable uses
- `pa18` / `pa19` / `pa21` / `pa22` targeted strict slices as needed

### Stage 3: Direct Function Call Events

- emit structured function-call witness events from ordinary overload
  resolution in `semantic_overload.cpp`
- emit structured function-call witness events from assignment/operator
  selection
- emit structured constructor-call witness events from constructor selection
- carry direct binding / drop / candidate counts from the real selection data

Validation:

- focused canaries:
  - `017`
  - `104`
  - `125`
- rerun `make test-strict` slices for `pa18` / `pa19` / `pa21` / `pa22`

### Stage 4: Remove Trace-Derived Source Witness Dependence

- remove trace fallback from the source witness renderer
- leave trace capture only as a debugging aid
- remove dead cppgm-side adapter remnants no longer used by the active path

Validation:

- broad strict witness run
- inspect remaining mismatches as real semantic / instantiation divergence,
  not renderer reconstruction drift

## Non-Goals

- changing the closure-event contract in this slice
- removing trace logging in general
- redesigning the patched-Clang materialization path
