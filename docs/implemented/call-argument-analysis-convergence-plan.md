# Call Argument Analysis Convergence Plan

## Purpose

This plan targets the remaining duplicated argument/subexpression analysis
logic in the call/overload layer.

The code already has a much better shared structure than before, but
`semantic_overload.cpp` still contains several closely related local analysis
paths that differ only because the policy/service boundary is not quite
finished.

## Narrow Goal

Make call-argument analysis consistent across:

- ordinary call analysis
- nested restricted analysis
- target-aware argument analysis
- candidate gathering and rematerialization

After this plan:

- call paths should stop manually branching on nested call-expression handling
  versus ordinary expression analysis
- target-aware and generic argument analysis should use one shared decision
  path
- cached argument analysis should not live in several slightly different forms

## Current Inconsistency

The main remaining duplication is in
[semantic_overload.cpp](../dev/src/semantic_overload.cpp):

- repeated `!instantiate_bodies && child.kind == call_expression` handling
- local cached argument analysis inside candidate gathering
- separate target-aware and generic argument analysis choices
- rematerialization-specific conversion-policy translation helpers

That code works, but it still makes policy-sensitive call analysis more
fragile than it needs to be.

## Concrete Invariant

Given a call argument and a semantic analysis policy, the semantic layer should
have one consistent answer to:

- should the argument be analyzed under restricted nested-call rules?
- should the target type influence analysis?
- may cached analysis be reused here?
- should rematerialization reuse the same argument-analysis policy?

Different consumers may request different outputs, but the decision process
should be shared.

## Planned Work

### 1. Classify Existing Argument-Analysis Paths

Separate the current argument-analysis logic into:

- policy differences that are semantically real
- policy differences that are just historical implementation drift

### 2. Introduce A Shared Call-Argument Analysis Helper/Service

Create one shared helper or small service that owns:

- nested restricted-call analysis
- target-aware preference decisions
- cached argument reuse

It should be used by candidate gathering and main call analysis rather than
being local to one branch.

### 3. Unify Rematerialization With The Same Policy Model

Rematerialization should not need its own semi-independent translation layer
for conversion policy unless there is a real semantic reason.

Where it truly differs, the difference should be explicit and named.

### 4. Reduce Local One-Off Analysis Lambdas

Once the shared helper exists, shrink the large local analysis lambdas inside
`semantic_overload.cpp` and route them through the shared path.

## Suggested Execution Order

1. classify the current argument-analysis branches
2. introduce the shared call-argument analysis helper/service
3. migrate candidate gathering
4. migrate main call-expression argument handling
5. migrate rematerialization and delete the leftover local translation helpers

## Completion Criteria

This plan is complete only when:

- nested call-argument analysis rules are not duplicated across multiple local
  call-analysis lambdas
- target-aware argument analysis uses the same shared policy path as ordinary
  argument analysis
- rematerialization policy is expressed in the same structured style as the
  rest of semantic call analysis
