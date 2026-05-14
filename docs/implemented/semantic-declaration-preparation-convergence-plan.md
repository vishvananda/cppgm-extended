# Semantic Declaration Preparation Convergence Plan

## Purpose

This plan targets the remaining duplicated declaration-preparation work in the
semantic layer.

The goal is to make declaration preparation and method/classification
extraction more uniform across namespace-scope, block-scope, class-member, and
output-oriented paths.

## Narrow Goal

Make one prepared declaration shape authoritative for:

- filtered specifiers
- filtered declarator shape
- method syntax/classification facts
- owner-sensitive declaration context

After this plan:

- semantic declaration paths should stop recomputing the same prepared
  specifier/declarator/classification state in slightly different ways
- the different declaration owners should still be able to add their own
  behavior, but not their own preparation rules

## Current Inconsistency

The highest-value method-parse duplication is already cleaned up, but the wider
declaration family still spans several seams:

- [`callsemantic.cpp`](../dev/src/callsemantic.cpp)
- [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp)
- [`semantic_output.cpp`](../dev/src/semantic_output.cpp)
- [`semantic_statement.cpp`](../dev/src/semantic_statement.cpp)

The current preparation helpers are useful, but fragmented:

- `prepare_namespace_scope_specifiers(...)`
- `prepare_block_scope_specifiers(...)`
- `prepare_method_parse_context(...)`

That means declaration sites still differ in:

- how specifiers are normalized
- when declarators are filtered or reinterpreted
- how method-like syntax is classified
- when those classification facts are forwarded downstream

## Concrete Invariant

Given a declaration site, semantic preparation should answer these questions
once:

- what is the normalized specifier set?
- what is the normalized declarator shape?
- is the declaration method-like, and if so with what method syntax facts?
- what owner context matters downstream?

Different callers may consume that information differently, but they should not
recompute it differently.

## Planned Work

### 1. Inventory The Current Preparation Families

Classify declaration paths by owner:

- namespace scope
- block scope
- class member scope
- output/registration paths that still need preparation

Then record which prepared facts each owner actually needs.

### 2. Introduce A Broader Prepared Declaration Context

Build on the existing prepared-method context work and define a broader
prepared declaration shape that can represent:

- general declaration preparation
- optional method syntax/classification
- owner-sensitive parse facts

This should not replace every local variable with one giant bag of fields.
It should only capture the prepared state that is currently recomputed.

### 3. Route The Remaining Declaration Callers Through It

Migrate the remaining declaration-heavy sites to consume the shared prepared
context rather than re-running local preparation logic.

The important part is not uniformity for its own sake. The important part is
that later fixes to declaration interpretation should have one place to land.

### 4. Narrow The Output-Oriented Preparation Paths

`semantic_output.cpp` should stop acting like a special declaration-preparation
subsystem wherever it still duplicates semantic preparation.

Output-specific behavior may remain, but declaration interpretation itself
should come from the same prepared context as the main semantic paths.

## Suggested Execution Order

1. inventory remaining declaration-preparation families
2. define the shared prepared declaration context
3. migrate namespace/block/class declaration callers
4. migrate output/registration callers that still duplicate preparation
5. remove the old ad hoc duplicated preparation paths

## Completion Criteria

This plan is complete only when:

- the remaining declaration preparation paths compute normalized
  specifier/declarator state through one shared prepared context family
- method/classification facts are computed once per declaration path
- output-oriented declaration consumers are downstream of semantic preparation,
  not parallel reinterpreters of it
