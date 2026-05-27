# Template Kernel Assignment Buildout Process

## Purpose

This document is a short supplement to
[ASSIGNMENT_BUILDOUT_PROCESS.md](/Users/vishvananda/cppgm/legacy/ASSIGNMENT_BUILDOUT_PROCESS.md)
for building out an experimental template-focused assignment surface.

The main goal is to isolate template correctness and template performance from
the rest of the compiler pipeline so students can work on a scoped subsystem
instead of having to debug:

- preprocessing
- full C++ parsing
- whole-program semantic analysis
- LowIR lowering
- native code generation
- hosted/STL integration

This buildout should start as an experimental surface in a separate worktree.
After the surface is proven, we can decide whether it should become:

1. a new standalone assignment, or
2. the primary oracle/tool surface reused by the existing template assignments
   (currently `PA21` and `PA22`).

## Problem Statement

The existing template split is already close to the right semantic boundary:

- [pa21/README.md](/Users/vishvananda/cppgm/pa21/README.md) owns template
  declaration graphs and specialization selection
- [pa22/README.md](/Users/vishvananda/cppgm/pa22/README.md) owns deduction,
  substitution, and SFINAE

The problem is the primary observable contract.

Today, too much template validation is discovered only through:

- downstream LowIR output differences
- hosted header compile acceptance
- STL/bootstrap frontier breakage

Those are useful integration gates, but they are too far downstream to be the
first student-facing proof of template correctness or template performance.

The template reparse-elimination work is already converging on a better seam:

- structured template inputs
- explicit request/response APIs
- deterministic structured results
- explicit fallback/performance counters

This assignment buildout should turn that seam into a concrete public contract.

## Core Decision

Build an isolated template-kernel tool first.

Working name:

- `tmplsolve`

Working repository surface:

- `template-kernel/`

This tool should answer template questions directly from a small
template-specific input language rather than from full C++ translation units.

That keeps the owned boundary narrow and testable.

## Student-Facing Direction

The kernel should stay as a maintainer oracle and reduction surface.

The student-facing template contract should be a new compiler mode:

- `cppgm++ --emit-templates`

This mode should sit between the existing public layers:

1. `--emit-types`
2. `--emit-semantics`
3. `--emit-templates`
4. `--emit-lowir`

That layering is a better match for the template assignments than the current
practice of discovering template correctness only through downstream LowIR or
hosted-header behavior.

The intended split is:

- `tmplsolve` / `template-kernel/`
  - maintainer-only reduction/oracle surface
  - hidden test reduction bank
  - performance/scaling lab
- `cppgm++ --emit-templates`
  - student-facing semantic witness
  - source-anchored selected specialization / deduction / drop decisions
  - primary PA21/PA22 grading surface
- `cppgm++ --emit-lowir`
  - secondary integration check proving instantiated semantics still lower

## Owned Boundary

The template kernel should own:

- template parameter declarations
- default argument binding
- explicit specialization tables
- partial specialization registration and selection
- function-template deduction
- substitution and candidate dropping
- explicit deferral / dependency state
- structural result formatting
- template hot-path fallback counters

It should not own:

- preprocessing
- full source parsing
- whole-program name lookup outside the modeled surface
- AST building for arbitrary C++
- body lowering / LowIR
- object emission / runtime behavior
- hosted/STL compatibility as the primary contract

## Public Contract Shape

The initial experimental contracts should be:

- maintainer contract:
  - one `.tkq` file containing declarations plus queries
  - one deterministic text file answering those queries
  - checked-in `.ref` files driven by `tmplsolve`
- student contract:
  - one C++ source file
  - one deterministic `--emit-templates` text witness
  - checked-in `.ref` files for the student-owned template layer

The maintainer input language should be deliberately smaller than C++ source.

The point is not to build a second C++ parser. The point is to expose the
template subsystem through stable, isolated interfaces.

## Current Implementation Status

The worktree now contains a runnable experimental kernel:

- binary: `dev/tmplsolve`
- test corpus: `template-kernel/tests/`
- local harness: `template-kernel/Makefile`
- root entrypoint: `make test-template-kernel`

The current prototype already covers a meaningful first assignment slice:

- class-template selection
- variable-template selection
- alias-template expansion
- default argument binding
- explicit specialization selection
- partial specialization selection
- function-template deduction
- simple non-deduced contexts through `nondeduced<T>`
- simple requirement filtering through `when same(T, int)`
- cv/ref-sensitive function-template selection over a bounded type grammar
- structural function/array shape matching through kernel-only wrappers
- bounded bool/int nontype parameters in deduction-oriented reductions
- deterministic stats counters
- patched-Clang witness generation for real-source comparison
- `cppgm` trace-to-witness normalization for focused student-facing comparison
- an initial direct `cppgm++ --emit-templates` prototype for source-anchored
  function-call template decisions

That is enough to validate the assignment shape at the right boundary. It is
not full C++ template semantics, and it is not intended to be.

## Design Rules

1. The primary oracle must stay at the first owned boundary.
   For this buildout that means structured query answers, not LowIR or native
   output.
2. The input language should carry structure explicitly.
   Do not make students recover template structure from free-form source text if
   the assignment is supposed to teach template semantics rather than parsing.
3. Performance should be measured with counters and scale cases first.
   Wall-clock timing is useful for maintainer benchmarking, but student-visible
   contracts should prefer deterministic operation counts or bounded fallback
   counts.
4. Every STL/bootstrap-discovered hole should be reduced before it is promoted.
   The integration frontier remains a discovery source, not the first oracle.
5. Keep the promotion decision open until the kernel surface is strong enough.
   Do not commit early to “new assignment” versus “PA21/PA22 oracle refactor”
   before the experimental tool proves its value.

6. Keep maintainer and student diagnostics separate.
   Maintainer-only counters such as reparses/fallbacks are useful internally,
   but the student-facing `--emit-templates` mode should only expose semantic
   decisions the assignment actually owns.

## Proposed Query Families

The first query families should be:

1. specialization selection
   - which primary/partial/explicit specialization wins
   - which bindings were applied
2. function-template deduction
   - which candidates are viable
   - which bindings were deduced
   - why other candidates were dropped
3. default-argument binding
   - which defaults were applied
   - whether the result remains dependent/deferred
4. stats
   - structured-match count
   - text-fallback count
   - reparse/fallback counters

Later query families can add:

- alias-template canonicalization
- variable-template selection
- deferred/no-eager-instantiation decisions
- partial-ordering comparison details

Implemented in this worktree:

- specialization selection
- function-template deduction
- default-argument binding
- alias expansion
- variable-template selection
- stats

## Initial File Layout

The experimental layout should start with:

- `template-kernel/README.md`
- `template-kernel/tests/`

Early tests should be tiny and explicit:

- one class partial-specialization selection owner
- one function-template deduction owner

## Phases

## Phase 0. Experimental Scaffold

Goal:

- start the work in an isolated branch/worktree
- write down the contract before any implementation code exists
- seed a few tiny example tests that prove the surface is understandable

Work:

- add this plan
- add `template-kernel/README.md`
- add initial `.tkq` and `.ref` seed cases

Exit criteria:

- the experimental surface exists in the repo
- the first input/output examples are concrete
- the next implementation slice is obvious

Status:

- complete

## Phase 1. Freeze The Tool Contract

Goal:

- settle the first public kernel boundary before writing the real tool

Work:

- freeze:
  - binary name
  - file extension
  - query families
  - deterministic output conventions
  - explicit non-goals
- decide whether the input language is line-oriented only or allows simple
  nested block syntax
- decide the minimum required type grammar for the first slice

Exit criteria:

- the README and seed tests define one stable first passing subset

Status:

- complete for the experimental subset
- the current subset is line-oriented, type-only, and deterministic

## Phase 2. Harness And Oracle

Goal:

- make the new surface runnable and regression-friendly

Work:

- add a tiny harness that:
  - runs `tmplsolve`
  - writes `.my`
  - compares against `.ref`
  - compares exit status
- keep the oracle file-based and deterministic
- avoid mixing this harness with existing `pa*` harnesses until the surface is
  mature

Exit criteria:

- `make` or one narrow command can run the kernel tests end-to-end

Status:

- complete
- local command: `make -C template-kernel test`
- root command: `make test-template-kernel`

## Phase 3. TK1: Graph And Selection Kernel

Goal:

- prove the existing PA21-style split at the right boundary

Work:

- implement declaration collection for:
  - class templates
  - alias templates
  - variable templates
  - function templates
- implement:
  - default-argument binding
  - explicit specialization ownership
  - partial specialization registration
  - partial ordering and specialization selection
- answer selection queries without lowering anything

Exit criteria:

- reduced PA21-style cases pass through kernel query output alone

Status:

- complete for the first kernel slice
- implemented declarations: class, variable, alias, explicit, and partial
  specializations
- implemented queries: `select_class`, `select_variable`,
  `bind_defaults class|variable|alias`, and `expand_alias`

## Phase 4. TK2: Deduction And Substitution Kernel

Goal:

- prove the PA22-style split at the right boundary

Work:

- implement function-template deduction
- implement non-deduced-context handling
- implement candidate dropping and substitution-failure reporting
- implement explicit dependent/deferred-state reporting

Exit criteria:

- reduced PA22-style cases pass through kernel query output alone

Status:

- complete for the first kernel slice
- implemented query: `deduce_call`
- implemented behaviors: deduction, non-deduced contexts, defaulted template
  parameters, candidate dropping, and simple requirement filtering

## Phase 5. Discovery Ingest

Goal:

- replace “we only find holes by compiling the STL” with a disciplined reduction
  path

Work:

- create a reduction path from:
  - `pa34/tests/compile`
  - `pa34/tests/frontier`
  - validation seeds
  - hosted/STL/bootstrap regressions
- every new discovered template bug should become:
  1. a tiny reduced kernel case
  2. optionally a retained integration case if it still adds value

Exit criteria:

- the kernel suite becomes the first landing place for new template regressions

Status:

- started but not complete
- initial reductions now exist from:
  - `validation/tests/014-default-template-argument-merge.cpp`
- `validation/tests/112-nondeduced-context-only.cpp`
- `validation/tests/035-partial-ordering-ref-vs-const-ref.cpp`
- `validation/tests/034-function-reference-deduction.cpp`
- `validation/tests/033-array-reference-deduction.cpp`
- `validation/tests/037-pointer-qualification-deduction.cpp`
- `validation/tests/113-ambiguous-cv-pointer-partial-ordering.cpp`
- `pa21/tests/spec/200-forward-declared-class-template-default-merge.t`
- `pa21/tests/spec/215-partial-specialization-uses-primary-default-argument.t`
- `pa22/tests/spec/438-function-template-defaulted-class-template-arg-deduction.t`
- `pa34/tests/compile/656-forward-array-string-pair.t`
- `pa34/tests/compile/542-local-functor-std-function-assignment.t`
- `pa35/tests/link/701-hosted-function-reference-parameter-link-smoke.t.1`
- reduction tracking now lives in `template-kernel/REDUCTIONS.md`

## Phase 6. Performance Surface

Goal:

- make template hot-path costs visible without relying only on broad hosted
  timing

Work:

- expose deterministic stats queries/counters such as:
  - structured match count
  - text fallback count
  - parse fallback count
  - alias rewrite fallback count
- add synthetic scale cases:
  - repeated identical queries
  - deep default chains
  - pack-heavy deduction
  - partial-specialization matrices
- keep broad self/STL benchmarks as maintainer-only secondary gates

Exit criteria:

- performance regressions can be caught on the kernel surface before they
  become `pa34` cliffs

Status:

- partially complete
- `query stats` and counter-based assertions now exist
- a repeated-query scale case now exists in
  `template-kernel/tests/012-stats-scale-repeated-queries.tkq`
- missing pieces are larger synthetic scale cases and reductions from real
  hosted/STL hotspots

## Phase 6A. Student Emit Surface

Goal:

- prove the same important template decisions can be emitted directly from
  `cppgm++` at the semantic boundary students already know

Work:

- add `cppgm++ --emit-templates`
- keep it attached to semantic analysis rather than LowIR lowering
- emit source-anchored, student-owned decisions first:
  - selected function-template call
  - selected overloaded template assignment/operator call
  - template bindings
  - candidate drops where the existing semantic layer already knows them
- keep the initial surface intentionally smaller than the maintainer kernel
- compare the output against patched Clang witnesses and reduced kernel cases

Exit criteria:

- at least a small set of real tests can compare meaningful template decisions
  at the `--emit-templates` layer without looking at LowIR

Status:

- started
- the current worktree prototype is intentionally narrow:
  - direct function-call template decisions are emitted
  - overloaded assignment-template selection is emitted
  - class / alias / variable template uses are not yet emitted directly
  - defaulted/non-deduced/drop reporting is partial and trace-backed rather
    than fully normalized

## Phase 7. Promotion Decision

Goal:

- decide how the experimental kernel becomes part of the public assignment
  story

Options:

1. promote it as a new standalone assignment
2. keep the assignment numbers and make the kernel the primary PA21/PA22 oracle
3. keep it maintainer-only if it turns out not to be pedagogically worth the
   extra surface

Promotion criteria:

- the kernel surface covers a meaningful fraction of current PA21/PA22 logic
- reductions from hosted/STL regressions map naturally onto kernel cases
- the contract is understandable without full-compiler context

Status:

- the standalone-kernel-versus-existing-PA-numbering question is still open
- the current direction is no longer open:
  - keep the kernel as the maintainer oracle
  - move PA21/PA22 toward `cppgm++ --emit-templates` as the student oracle

## Initial Success Criteria

This experiment is successful if:

1. a student can implement meaningful template behavior without needing the full
   compiler pipeline
2. the first oracle for template correctness becomes a deterministic query
   surface rather than STL compile success
3. the first oracle for template performance becomes explicit stats/scale cases
   rather than only `pa34` wall time
4. new template regressions reduce naturally into the kernel surface
5. the same important decisions can also be emitted directly from the main
   compiler in a student-facing `--emit-templates` mode

## Buildout Completion Criteria

For this initial worktree buildout, “complete” means:

1. the template-kernel tool exists and builds cleanly
2. the kernel suite runs through a dedicated harness
3. the README documents the actual supported contract
4. the implemented surface covers a meaningful PA21/PA22-shaped subset
5. future work is clearly separated into adoption/performance/promotion phases

Those criteria are now satisfied in this worktree.

## Next Maintainer Steps

The next useful maintainer work is:

1. reduce 10-20 real template failures into `.tkq` cases
2. add scale-oriented stats cases rather than wall-clock-only checks
3. expand `--emit-templates` from function-call coverage into class/alias/
   variable template uses
4. replace trace-backed formatting with direct semantic witness emission once
   the output shape settles
