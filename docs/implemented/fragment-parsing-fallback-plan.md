# Fragment Parsing Fallback Plan

## Purpose

This plan is the remaining semantic-complexity cleanup around fragment parsing
and text reparsing.

Recent memory/perf work improved the common `text -> TypePtr` cache path, but
that only made the existing design cheaper. It did not yet make fragment
parsing a true fallback-only mechanism.

## Narrow Goal

Demote fragment parsing from a normal semantic service to an explicit fallback
path.

After this plan:

- semantic code should prefer stored semantic data over reparsing text
- fragment parsing should be reserved for the places that really need syntax
  recovery from text
- fallback use should be measurable and explainable

## Current Inconsistency

Fragment/text reparsing still sits behind several semantic flows:

- type-text reconstruction and lookup
- dependent-name and template-argument recovery
- declaration/model paths that still fall back to text interpretation

The recent cache work reduced the storage cost, but the semantic layer still
often treats reparsing as a normal tool rather than as an exceptional tool.

## Concrete Invariant

A semantic path should only use fragment parsing when at least one of these is
true:

- the original semantic structure is genuinely unavailable
- the caller truly needs syntax, not just semantic type/category information
- the path is intentionally operating on deferred textual form

If a caller only needs a semantic type or binding fact, it should not pay for
fragment parsing just because that was historically convenient.

## Planned Work

### 1. Inventory Fragment-Parsing Call Sites

Classify call sites as:

- syntax-required
- semantic-data-only
- historical convenience fallback

This classification should be explicit before replacing anything.

### 2. Narrow The Common Semantic-Only Callers

Replace the highest-frequency semantic-only callers with direct semantic data
paths wherever possible.

This is where the complexity reduction comes from, not from rewriting the
fragment parser itself.

### 3. Keep Fallback Paths Explicit

Where fallback reparsing remains necessary, keep it explicit in naming and
structure so it is obvious that the path is exceptional rather than primary.

### 4. Add Metrics/Tracing For Remaining Fallback Usage

The remaining fallback usage should be observable.

That can be lightweight, but the code should make it easy to answer:

- which semantic paths still rely on reparsing?
- how often?
- on which heavy translation units?

## Suggested Execution Order

1. inventory fragment-parsing call sites
2. classify syntax-required versus semantic-only uses
3. migrate the highest-value semantic-only callers off reparsing
4. make the remaining reparsing paths explicit fallback helpers
5. keep lightweight metrics for the remaining fallback usage

## Completion Criteria

This plan is complete only when:

- fragment parsing is no longer treated as a routine semantic service in the
  common semantic-only paths
- the remaining reparsing users are explicitly justified fallback callers
- the semantic layer can explain where and why fallback reparsing still occurs
