# Semantic Policy Convergence Plan

## Purpose

This plan defines the next semantic-layer cleanup for policy handling.

The goal is not to change language behavior. The goal is to make semantic
policy decisions flow through one coherent set of policy shapes instead of
being repeatedly reconstructed from partially overlapping option structs and
raw booleans.

## Narrow Goal

Make one consistent policy model authoritative for:

- call analysis
- argument conversion
- constructor selection
- nested expression analysis under restricted modes

After this plan:

- semantic helpers should stop inventing new local boolean combinations such as
  `instantiate_bodies`, `allow_user_defined`, or `allow_explicit`
- semantic service boundaries should take structured policy views
- the current policy types should have clear ownership and non-overlapping
  meaning

## Current Inconsistency

The code is better than it used to be, but there is still avoidable policy
duplication across:

- [`analysis_policy.h`](../dev/src/analysis_policy.h)
- [`semantic_context_facets.h`](../dev/src/semantic_context_facets.h)
- [`callsemantic.cpp`](../dev/src/callsemantic.cpp)
- [`semantic_overload.cpp`](../dev/src/semantic_overload.cpp)
- [`semantic_conversion.cpp`](../dev/src/semantic_conversion.cpp)
- [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp)

Today we have several related policy shapes:

- `AnalysisPolicy`
- `CallAnalysisOptions`
- `ArgumentConversionOptions`
- `ConstructorSelectionOptions`

They overlap, but they do not line up cleanly.

That creates two kinds of complexity:

1. caller code repeatedly translates one policy shape into another
2. the same semantic restriction is expressed differently in different
   subsystems

## Concrete Invariant

The semantic layer should have one explicit answer to each of these questions:

- may this analysis instantiate function bodies?
- may this path consider user-defined conversions?
- may this path consider explicit constructors/conversion functions?
- should this path seed output/materialization side effects?

Those answers should be derived once from an authoritative policy view, not
re-derived at each helper boundary.

## Planned Work

### 1. Classify Existing Policy Fields

Before changing behavior, classify each existing field as one of:

- global semantic-analysis mode
- call-analysis mode
- conversion mode
- constructor-selection mode
- output/materialization mode

The point is to stop treating every boolean as if it were universally relevant.

### 2. Introduce One Derived Policy Flow

Keep `AnalysisPolicy` as the broad semantic-analysis root.

Then define small derived policy views for the places that need narrower
decisions:

- call analysis
- conversion
- constructor selection

Those views should be constructed in one place and passed through as structured
objects.

### 3. Remove Boolean Translation Helpers

Current code still has helper seams that peel structured policy back into raw
booleans or partially reconstructed option objects.

Those should be removed or narrowed so helpers take:

- the structured policy object they actually need
- or a dedicated derived policy view

not:

- a boolean plus a comment explaining what it means here

### 4. Centralize Restricted Nested-Analysis Policy

The nested-call / restricted-analysis cases should stop constructing their own
ad hoc policy variants.

Examples:

- `CallAnalysisOptions(false)`
- conversion paths that suppress user-defined conversions
- constructor viability probes that suppress explicit constructors and body
  instantiation

Those restricted modes should come from named policy constructors rather than
literal boolean combinations.

## Suggested Execution Order

1. classify the existing fields and option conversions
2. define the authoritative derived policy views
3. migrate `callsemantic.cpp` policy application onto those views
4. migrate `semantic_overload.cpp` and `semantic_conversion.cpp`
5. migrate constructor viability and class-model probes
6. remove the leftover boolean translation helpers

## Completion Criteria

This plan is complete only when:

- semantic service boundaries no longer grow by adding one more policy boolean
- `CallAnalysisOptions`, `ArgumentConversionOptions`, and
  `ConstructorSelectionOptions` have clearly separated responsibilities
- the main callers derive restricted policy modes through named structured
  constructors or helpers
- changing one semantic policy rule no longer requires auditing scattered
  boolean translations first
