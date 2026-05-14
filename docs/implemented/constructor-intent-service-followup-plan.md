# Constructor Intent Service Follow-Up Plan

## Purpose

This plan finishes the remaining constructor-intent cleanup in the semantic
layer.

`constructor_lifecycle_service` already centralizes much of constructor
selection and follow-up. The remaining inconsistency is that callers still
choose constructor policy and intent in several different local ways.

## Narrow Goal

Make constructor intent explicit and authoritative across the semantic layer.

After this plan:

- constructor callers should describe *why* they are selecting a constructor
- the service layer should derive the concrete option bundle from that intent
- semantic callers should stop hand-assembling constructor-selection options
  for each local context

## Current Inconsistency

Constructor work is still split across:

- [`semantic_lifetime.cpp`](../dev/src/semantic_lifetime.cpp)
- [`semantic_conversion.cpp`](../dev/src/semantic_conversion.cpp)
- [`semantic_expression.cpp`](../dev/src/semantic_expression.cpp)
- [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp)
- [`callsemantic.cpp`](../dev/src/callsemantic.cpp)

Examples of remaining local reconstruction:

- viability checks that disable user-defined constructors and body instantiation
- conversion-constructor paths that disable explicit constructors
- aggregate/list-init paths that choose explicitness rules locally
- class-model probes that build their own constructor option bundles

## Concrete Invariant

Constructor callers should express an intent such as:

- viability probe
- direct initialization
- copy initialization
- list initialization
- user-defined conversion constructor probe
- implicit special-member viability

The service layer should then derive:

- whether user-defined constructors are allowed
- whether explicit constructors are allowed
- whether bodies may be instantiated
- whether aggregate partial matching is allowed
- whether initializer-list-only behavior applies

## Planned Work

### 1. Classify Existing Constructor Call Sites By Intent

Before changing code, classify constructor sites by semantic intent rather than
by file.

This is the key step that prevents another “service plus local policy forks”
outcome.

### 2. Introduce An Explicit Constructor Intent/Profile Layer

Define a small intent or profile shape that describes the semantic reason for
selection.

The current free-form `context` string can stay for diagnostics, but it should
stop being the nearest thing we have to a semantic intent tag.

### 3. Derive ConstructorSelectionOptions Centrally

Move option derivation behind the service layer so callers request:

- a semantic intent/profile
- plus only the truly site-specific extras

not a fully assembled option bundle every time.

### 4. Migrate Remaining Probes And Conversion Paths

Once the intent layer exists, migrate the remaining constructor probes and
conversion-constructor callers onto it.

The immediate targets are:

- viability checks
- conversion-constructor probing
- list-init and copy-init explicitness handling

## Suggested Execution Order

1. classify constructor call sites by semantic intent
2. define constructor intent/profile types
3. centralize `ConstructorSelectionOptions` derivation
4. migrate viability and conversion-constructor probes
5. migrate the remaining init-style callers

## Completion Criteria

This plan is complete only when:

- constructor call sites mostly express semantic intent rather than raw option
  bundles
- the service layer derives the option policy consistently
- changing one constructor-policy rule no longer requires auditing multiple
  local option assemblers first
