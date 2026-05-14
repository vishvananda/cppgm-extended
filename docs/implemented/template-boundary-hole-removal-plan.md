# Semantic Source-Use Table Plan

## Goal

Keep template computation fully structured, and stop treating source witness
events as the primary storage for source-facing semantic facts.

The intended split is:

- semantic model owns stable entity source facts
- a translation-unit owned source-use table records per-occurrence source uses
  when a consumer needs them
- source witness output becomes a projection of that table
- lifecycle and temporal template activity remains event-based

This is the path to deleting renderer recovery rather than continuing to move
it around.

## Status

The structured semantic/template boundary work is already done:

1. class instantiation no longer crosses the boundary with raw argument text
2. function deduction no longer crosses the boundary with raw explicit
   argument text
3. witness API has been split from template computation surfaces

The current problem is no longer the semantic/template boundary itself.

The remaining divergence comes from source-facing witness rows still depending
on transient event payloads plus renderer-side recovery, fanout, and source
text scanning.

## Recommendation

Do **not** solve the remaining divergence by making source witness events richer
and richer.

Do **not** push all source-use information into the EST / `CallSemNode` tree.

Do **not** store large per-occurrence witness lists directly on semantic
entities like `ClassInfo` or `FunctionBinding`.

Instead:

1. keep always-useful entity anchors directly in semantic model objects
2. add a dedicated translation-unit owned `SemanticSourceUse` table for
   per-occurrence source references
3. build that table only when a consumer is enabled, starting with `--witness`
4. render source witness rows from the table
5. keep lifecycle witness rows event-based

That gives us:

- low overhead when witness is off
- a persistent semantic representation for source-facing facts
- a clean separation between semantic facts and temporal trace
- a direct path to deleting renderer fanout and source-text recovery

## Why This Shape

### Why Not The EST

The EST is the wrong shape for many source-facing witness rows:

- qualified type uses
- alias uses in declarations
- out-of-class member-definition qualifiers
- nested template-id uses inside declarations
- explicit-specialization replay sites

Those are semantic source references, not expression-tree nodes.

Using the EST as the storage layer would require inventing ghost nodes for many
rows that are not real EST concepts.

### Why Not Entity-Local Use Lists

`ClassInfo`, `FunctionBinding`, and similar objects are the wrong ownership
boundary for per-occurrence source uses.

Uses are:

- many-to-one
- translation-unit specific
- role-specific
- potentially numerous

Storing all of them on entities would turn semantic objects into witness
bookkeeping dumps.

### Why A Source-Use Table

A translation-unit owned table is the right shape because source uses are:

- recorded during semantic analysis
- specific to one translation unit
- naturally emitted one visible occurrence at a time
- useful to more than one consumer once structured

The table gives witness output a stable semantic backing without polluting core
entity state.

## End State

### Always-On Semantic State

Keep stable entity anchors in semantic model objects because they are useful
outside witness and do not scale with the number of uses:

- declaration-name anchor
- definition-name anchor
- stable selected-declaration identity
- truly intrinsic template metadata

These belong on semantic entities where appropriate:

- `ClassInfo`
- `FunctionBinding`
- `ValueBinding`
- template declaration objects

### Gated Per-Occurrence Table

Add a translation-unit owned table that is only populated when a consumer is
enabled.

Representative shape:

```cpp
struct SemanticSourceUse {
  SourceUseKind kind;              // class, alias, variable, function-call
  SourceUseRole role;              // qualifier, type-use, call, base-specifier
  SourceOwnership ownership;       // direct, replayed, nested-derived

  SourceAnchor spelling_anchor;    // exact visible token location
  SourceAnchor provenance_anchor;  // semantic trigger location if different
  SourceAnchor selected_decl_anchor;

  EntityRef selected_entity;       // semantic entity key / pointer wrapper
  SelectionKind selection;         // primary, partial, explicit, instantiation

  vector<Binding> bindings;
  vector<Binding> specialization_bindings;
};
```

Important rule:

- one `SemanticSourceUse` row per visible source occurrence that should appear
  in the final source witness output

If the final witness should contain two `Box<bool>` rows on one line, semantic
analysis should record two table rows. The renderer should not discover them by
scanning source text later.

### Witness Split

After the table is authoritative, witness splits into two pipelines:

1. source witness rows
   - rendered from `SemanticSourceUse`

2. lifecycle witness rows
   - still emitted as events from template machinery
   - examples:
     - `RequireDefinition`
     - `EnsureDefinition`
     - `FunctionInstantiation`
     - `ClassInstantiation`
     - `ClassFinalization`

Long-term target:

- source witness: table-backed
- lifecycle witness: event-backed

## Capture Policy

The source-use table should **not** be always on.

Recommended policy:

- always-on:
  - stable entity anchors
  - intrinsic semantic metadata
- gated:
  - per-occurrence `SemanticSourceUse` capture

The gating surface should be a semantic service or sink, not raw witness flag
checks spread throughout semantic code.

Representative shape:

```cpp
struct SemanticSourceUseSink {
  virtual void record(const SemanticSourceUse & use) = 0;
};

struct NullSemanticSourceUseSink : SemanticSourceUseSink {
  void record(const SemanticSourceUse &) override {}
};
```

That keeps call sites small and allows the same structured data to serve:

- witness
- diagnostics
- audits
- teaching or debug tooling

## Migration Plan

## Phase 1: Classify Existing Source-Fact Storage

### Scope

Audit current semantic and witness-adjacent storage so each field is moved to
the right long-term home.

### Changes

1. Inventory current fields and channels carrying source-use data
   - semantic entity fields such as `first_qualifier_use_location`
   - parser-trace based location propagation
   - source witness event structs
   - renderer-side recovered locations

2. Classify each item as one of:
   - stable entity anchor
   - `SemanticSourceUse` row data
   - lifecycle event data
   - obsolete transitional state

3. Update the plan of record for each currently overloaded field before deeper
   refactors begin

### Success Criteria

- every current source-related field has a declared long-term home
- parser-trace and witness-event leakage is explicitly identified, not left
  implicit

## Phase 2: Normalize Stable Entity Anchors

### Scope

Make declaration and definition anchors explicit and stable on semantic
entities, independent of witness.

### Changes

1. Add or normalize exact declaration-name anchors
2. Add or normalize exact definition-name anchors where applicable
3. Remove or demote ad hoc entity-local witness leakage that should not live on
   the entity itself

### Success Criteria

- stable entity anchors no longer depend on renderer recovery
- entity-local source fields are reduced to intrinsic semantic state

## Phase 3: Add A Gated `SemanticSourceUse` Service

### Scope

Introduce the source-use table and the service or sink that records into it.

### Changes

1. Add core types
   - `SemanticSourceUse`
   - `SourceUseKind`
   - `SourceUseRole`
   - `SourceOwnership`
   - `SourceAnchor`
   - `EntityRef`

2. Add the capture interface
   - `SemanticSourceUseSink`
   - null sink
   - table-backed sink

3. Thread the service through semantic analysis state
   - default to null when witness is off
   - instantiate a real table only when source-use capture is requested

### Success Criteria

- semantic code can record source uses without depending on witness formatting
- witness-off mode stays near-zero overhead for per-occurrence capture

## Phase 4: Record Source Uses In Semantic Analysis

### Scope

Move source-facing witness facts into the `SemanticSourceUse` table, beginning
with the template-related rows that currently need renderer recovery.

### Changes

1. Record class uses
   - direct type-id uses
   - qualified owner uses
   - nested derived uses where they are real visible occurrences
   - out-of-class member-definition qualifier uses
   - explicit-specialization replay sites

2. Record alias and variable uses
   - same data model
   - no renderer fanout

3. Record function-call uses
   - exact call spelling anchor
   - fully qualified selected callee identity
   - selected declaration-name anchor

4. Ensure one row per visible occurrence
   - repeated same-line uses
   - qualifier uses
   - return-type uses
   - nested template-id occurrences that truly appear in source

### Success Criteria

- semantic analysis explicitly records the rows the final source witness needs
- source witness no longer depends on renderer discovery of missing occurrences

## Phase 5: Render Source Witness From The Table

### Scope

Make source witness output a projection of `SemanticSourceUse`.

### Changes

1. Add a rendering path from the table
2. Run old and new source rendering in parallel temporarily
3. Diff the outputs internally until the table-backed path is trustworthy
4. Cut public source witness rendering over to the table-backed path

### Success Criteria

- source witness formatting becomes mostly mechanical
- renderer no longer needs to rediscover source facts from text

## Phase 6: Remove Source Witness Events And Fanout

### Scope

Delete the old source-event machinery once the table-backed path is
authoritative.

### Changes

1. Remove renderer source fanout and recovery
   - source occurrence scanning
   - class-use source scanning
   - function-call qualification recovery

2. Remove source witness event emission that has been replaced by table
   recording

3. Keep lifecycle events intact

### Success Criteria

- source witness rows are table-backed only
- lifecycle rows are event-backed only
- source-facing witness no longer depends on source event payloads

## Phase 7: Cleanup Remaining Reparse Debt

### Scope

After the table is authoritative, clean up remaining semantic and witness
reparsing that only existed to support the old source-event path.

### Return Point

Do **not** wait until all witness compares pass before returning to reparse
removal.

The right time to pivot back is:

1. after the remaining direct source-use producers are in place
   - qualified member alias/class uses
   - out-of-class constructor/destructor/member-definition qualifier uses
   - nested class-use rows that still depend on renderer fanout
2. after lifecycle events carry the ownership facts still being inferred from
   normalized strings

Operational rule:

- finish the current source-row / ownership cluster first
- once the remaining failures are mostly reason or ordering issues rather than
  missing or extra source rows, immediately return to removing string reparsing
  instead of continuing general cleanup

### Targets

1. semantic-side type or name reparsing that only existed for witness recovery
2. parser-trace sourced location recovery that became obsolete
3. witness or debug text fields that can now be derived from semantic source
   uses or stable entity anchors
4. transitional fields and adapters left over from migration

### Success Criteria

- remaining text processing is either real parsing or optional formatting
- witness output no longer depends on renderer heuristics for source facts

## Validation Strategy

For phases that change behavior:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
cd pa21
python3 ../scripts/compare_template_witness_text.py --app ../dev/cppgm++ tests
```

During migration, compare:

- old source witness path
- new table-backed source witness path

before deleting the old one.

## Final Success Criteria

We are done when all of the following are true:

- template computation input is fully structured
- stable entity anchors live in semantic model state
- per-occurrence source uses live in a dedicated gated translation-unit table
- source witness is rendered from that table
- lifecycle witness remains event-based
- source witness events and renderer fanout are removed
- witness-off mode does not pay significant per-occurrence capture cost
