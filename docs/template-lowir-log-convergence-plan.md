# Template LowIR Log Convergence Plan

## Problem

The current template witness lane is not running through the same analysis
shape as the student-facing LowIR path.

Today:

- the validation bank compares patched-Clang witness output against
  `cppgm++ --emit-templates`
- the LowIR path goes through closure-enabled analysis in
  `dev/src/cpp_toolchain.cpp`
- `--emit-templates` is a separate template-oriented output mode

That is good enough for ordinary source-level template decisions, but it is
not good enough for the full `PA18` / `PA19` / `PA21` / `PA22` ownership
surface.

The LowIR path performs extra output-closure and materialization work that can
re-enter template acquisition:

- required-definition refresh through `dev/src/output_requirement_engine.cpp`
- function definition upgrade through `dev/src/callsemantic.cpp`
- closure expansion and definition binding replay through
  `dev/src/semantic_output.cpp`
- function/class instantiation coordination through the template
  instantiation/materialization path

Those extra calls are real template behavior, but they are not currently part
of the template witness oracle. As a result:

- `--emit-templates` is not proving the whole template interface used by
  `--emit-lowir`
- patched-Clang witness output is not yet describing the same closure-driven
  template activity
- hidden regressions can survive the current witness lane and appear later
  only under LowIR or object generation

## Required Invariant

Template logging must run on the same semantic/closure path as LowIR whenever
we want a complete template oracle.

Acceptable:

- `cppgm++ --emit-lowir --template-log <path> ...` produces LowIR and a
  template log from the same analysis run
- the log distinguishes source-driven template events from closure-driven
  materialization events
- patched Clang emits the same two-layer oracle shape

Not acceptable:

- a template-oracle mode that misses closure-driven template activity
- a closure log that can only be reconstructed from ad hoc debug traces
- silently mixing source and closure events without marking their origin

## Ownership

This work belongs with the template assignments, not the later backend-only
milestones.

Based on the assignment READMEs:

- `pa18/README.md` already owns on-demand template instantiation through the
  ordinary `cppgm++ --emit-lowir` boundary
- `pa19/README.md` extends that with explicit specialization and C++11 NTTP
  behavior
- `pa21/README.md` owns specialization/entity graph behavior and
  explicit-instantiation ownership
- `pa22/README.md` owns no-eager-instantiation timing, deduction,
  substitution, and SFINAE

Later hosted/backend milestones should reuse this template-log machinery, but
they should not be the first place where closure/materialization template
activity is specified.

## Design Goal

Use one per-translation-unit template witness session and let every
semantic-to-template entry declare its origin explicitly.

The intended external driver surface is:

```sh
cppgm++ --emit-lowir -o out.lowir --template-log out.templates src.cpp
```

This is better than keeping a separate `--emit-templates` execution mode as
the primary oracle because it guarantees that the template log sees the same
closure/materialization behavior as LowIR generation.

`--emit-templates` may remain temporarily as a compatibility wrapper, but it
should become a thin way to drive the same logging machinery rather than a
separate semantic mode.

## Internal Boundary Shape

The distinction between ordinary source activity and closure/materialization
activity must be expressed per call into the template layer, not via one
global process flag.

The same translation unit can legitimately perform both:

- ordinary source-triggered deduction, lookup, and instantiation
- later closure-triggered template definition upgrades and finalization

So the boundary needs a small entry-context object, conceptually:

```cpp
enum class TemplateWitnessOrigin {
  Source,
  Closure,
};

enum class TemplateClosureReason {
  None,
  TrackInstantiation,
  RequireDefinition,
  EnsureDefinition,
  FinalizeClass,
};

struct TemplateWitnessEntryContext {
  TemplateWitnessOrigin origin = TemplateWitnessOrigin::Source;
  TemplateWitnessContext source;
  TemplateClosureReason closure_reason = TemplateClosureReason::None;
  std::string trigger_entity;
  std::string trigger_decl_location;
};
```

This should live next to the existing witness/session carrier types in
`dev/src/template_witness.h`.

The template layer should then see:

- one `TemplateWitnessSession` for the translation unit
- one `TemplateWitnessEntryContext` per semantic-to-template entry

That keeps the origin split explicit without widening the public semantic API
to expose the internal event model.

## Logging Model

The log should contain two explicitly separated layers:

1. source template events
   - the current witness-style semantic decisions
   - class use, alias use, variable use, function call
   - binds, specializations, drops, and the existing source-oriented context
2. closure template events
   - materialization, definition, and finalization activity that only appears
     because LowIR analysis enables output closure

The important rule is:

- source events should stay source-oriented
- closure events should stay high-level and stable
- internal helper chatter should not become part of the public oracle

The minimal useful closure event families are:

- require-definition / ensure-definition
- function-template body materialization
- class finalization
- closure-triggered class/alias/variable/function instantiations when the
  reason for the work is closure rather than the original source use

This log can still be text-backed initially, but it must be a real
template-log format, not just raw `parser_trace(...)` debugging output.

## Cppgm Implementation Plan

### Phase 1: Introduce The LowIR-Coupled Driver Surface

Goal:

- make template logging available on the real LowIR path

Work:

- add `--template-log <path>` to the `cppgm++` driver/help surface
- thread it into the `--emit-lowir` path in `dev/src/cpp_toolchain.cpp`
- create one `TemplateWitnessSession` for the translation unit when template
  logging is requested

Acceptance:

- one `--emit-lowir` run can emit both LowIR and a template log
- disabling `--template-log` leaves ordinary LowIR behavior unchanged

### Phase 2: Add Per-Entry Origin/Reason Context

Goal:

- make source-vs-closure provenance explicit at the semantic/template boundary

Work:

- add `TemplateWitnessEntryContext` beside the existing witness types in
  `dev/src/template_witness.h`
- thread it through the semantic-to-template entry sites
- default ordinary semantic/template entrypoints to `origin=Source`

Acceptance:

- the template layer can distinguish ordinary source-origin calls from
  closure-origin calls without consulting global mutable state

### Phase 3: Mark Closure-Origin Entry Points

Goal:

- capture the extra template work that `--emit-lowir` performs under output
  closure

Work:

- mark closure-driven template entry points in:
  - `dev/src/output_requirement_engine.cpp`
  - `dev/src/callsemantic.cpp`
  - `dev/src/semantic_output.cpp`
  - the template instantiation/materialization coordination path
- classify them with `TemplateClosureReason`
- include stable trigger metadata where available

Acceptance:

- the log distinguishes source-origin and closure-origin template activity
- closure-triggered template events are no longer hidden behind raw debug
  traces

### Phase 4: Converge The Output Format

Goal:

- make one template log format cover both source and closure events

Work:

- extend the existing template witness output so it can dump:
  - source event section
  - closure event section
- keep the current source witness grammar stable where possible
- add only the new closure families needed for output/materialization
  behavior

Acceptance:

- one log file can act as the oracle for both source-level and closure-level
  template behavior

### Phase 5: Retire The Split Oracle

Goal:

- stop relying on a separate `--emit-templates` semantic mode as the primary
  template oracle

Work:

- make the validation bank drive `--emit-lowir --template-log ...`
- keep `--emit-templates` only as a temporary compatibility wrapper if still
  needed
- eventually retire it once the LowIR-coupled log is the only maintained path

Acceptance:

- template witness validation exercises the same analysis shape as LowIR
- no template-only oracle mode remains that can miss closure-driven work

## Patched Clang Plan

The patched Clang side must emit the same source/closure split so the oracle
stays comparable.

That means:

- extend the patched witness collector with the same origin model:
  - `Source`
  - `Closure`
- record closure reasons for:
  - definition upgrades
  - explicit/implicit materialization
  - class finalization / completion work
- emit one combined log with explicit source and closure sections

The patched Clang implementation does not need to mirror cppgm internals, but
it does need to match the externally visible contract of the combined log.

## Execution Notes

- This plan is about oracle convergence, not about eliminating the remaining
  recursive semantic query surface by itself.
- The template-semantic boundary plan should continue to shrink the actual
  interface at the same time.
- The first implementation step here should be low-risk:
  introduce the logging/session/origin plumbing before trying to retire the
  old `--emit-templates` wrapper.
