# Alias-Use Semantic Convergence Plan

## Status and handoff

This plan follows the completed class-use and lifecycle convergence work. It
starts from the lifecycle Phase 6 checkpoint, where the compiler has one
direct alias-use emitter but still reaches it through repeated semantic and
syntax-recovery routes.

- Repository: `/Users/vishvananda/cppgm-alias-consolidation`
- Branch: `experiment-witness-alias-path-consolidation`
- Fixed checkpoint: `05b0c7a21ff497cd2186fabc2096bf04cc6e931b`
- Parent branch: `experiment-witness-semantic-path-consolidation`
- Prior plan: `docs/witness-lifecycle-semantic-convergence-plan.md`
- Execution ledger to create in Phase 0:
  `docs/witness-alias-semantic-convergence-ledger.md`

The fixed checkpoint includes the witness provenance counters and the final
lifecycle observer cleanup. Performance comparisons therefore include the
diagnostic code that could affect compiler layout, even though ordinary runs
compile the diagnostic paths out.

Implementation update, 2026-08-08:

- Alias implementation, provenance, strict, broad, size, and performance work
  is complete through `ed605692bb9b55cbad9f510ac1a85fd3352ed574`.
- Inception is a joint gate for this plan and
  `docs/witness-class-materialization-semantic-ownership-plan.md`. Per user
  direction, do not run it until both plans have finished their implementation
  and evidence work. This plan remains open only on that joint gate.
- Phase 4 was amended after deletion experiments showed that ordinary semantic
  analysis does not visit every template header, base, default, declaration
  type, and unevaluated expression that owns a public alias row. The retained
  source-pattern traversal is the primary analyzer for those positions. It no
  longer instantiates the alias target, reconstructs an observer request, or
  publishes through a separate route; it builds typed arguments and enters the
  canonical completion boundary. The execution ledger records the failed
  deletion evidence and the resulting boundary.

## Objective

Make alias-template resolution produce one typed source result and publish one
`alias-use` row for each visible source occurrence. Remove the repeated alias
resolution, AST traversal, source-location recovery, and post-emission conflict
policy that currently select among competing versions of the same row.

The final implementation must have:

- one semantic alias-template resolution result carrying the resolved type,
  alias declaration, resolved arguments, exact source syntax, source anchor,
  dependency state, and expansion provenance;
- one successful exit from alias instantiation that submits the completed
  semantic result, instead of nine `note_alias_use` calls embedded in result
  construction arms;
- one parameterized result for dependent alias-template-ids, created during
  normal template analysis and reused during substitution;
- no recursive template-declaration walk that re-resolves or reinstantiates an
  alias already owned by normal semantic analysis; source-pattern analysis may
  cover declaration positions that normal semantic analysis does not visit;
- no source-token scan to relocate an alias name after resolution;
- no second alias observation in `template_argument_semantics` after the same
  template-id was already resolved;
- one direct `emit_alias_use` call outside `witness_api.cpp`;
- one inserted semantic source-use row per canonical alias occurrence, with no
  alias duplicate, rejection, replacement, or enrichment action;
- no renderer pass that removes one alias row in favor of another;
- fewer production source lines and fewer retired instructions than the fixed
  checkpoint.

Peak footprint must stay within 1% of the fixed checkpoint. Maximum RSS uses
the 3% warning and confirmation rule described below. Memory reduction is a
goal, not a prerequisite beyond those final gates.

## Starting evidence

The final lifecycle provenance report is:

- report: `/tmp/cppgm-lifecycle-phase6-final-provenance-report.json`;
- SHA-256:
  `1702282cba1fab28ff742ce4f2aa6767dedd2e3ec6a2078506587206dd94130e`;
- trace directory:
  `/tmp/cppgm-lifecycle-phase6-final-provenance.c1rbOl`.

The report records this alias flow across the strict corpus:

| Stage or action | Rows |
| --- | ---: |
| Emission attempts | 1,326 |
| Inserted table rows | 766 |
| Exact duplicate attempts | 387 |
| Rejected attempts | 159 |
| Replaced table rows | 14 |
| Final visible rows | 564 |

Ownership explains most rejected attempts:

| Action | Direct | Nested-derived |
| --- | ---: | ---: |
| Inserted | 744 | 22 |
| Exact duplicate | 384 | 3 |
| Rejected | 9 | 150 |
| Replaced | 14 | 0 |

The renderer removes the 202 table rows that do not reach public output:

| Renderer pass | Alias rows removed or replaced |
| --- | ---: |
| `WitnessBuilder::alias_events` map replacement | 61 |
| `canonicalize_locations_and_dedupe` | 57 |
| `normalize_source_defined_calls` | 1 |
| `prefer_source_spelled_alias` | 55 |
| `dedupe_visible_events` | 28 |
| Total | 202 |

Presentation passes also rewrite alias names and bindings. A rewrite is not a
deduplication target when it only converts semantic data to the public text
format. The plan must distinguish formatting from a pass that chooses one
semantic result over another.

The current provenance producer ID does not distinguish upstream routes. All
1,326 attempts appear as `alias.callsemantic.02` because the ID is assigned in
`observe_resolved_alias_template_id`. Phase 0 must add route identity before
deleting a feed.

## Current semantic routes

Static inspection finds four calls that feed
`observe_resolved_alias_template_id`, including the recursive pattern route:

1. `instantiate_alias_template_with_syntax` observes an unresolved dependent
   pattern from its failure and incomplete-resolution arms.
2. `instantiate_resolved_alias_template_impl` observes successful, cached,
   builtin, dependent, and fallback results through `note_alias_use`.
3. `record_direct_alias_template_source_use_if_needed` in
   `template_argument_semantics.cpp` reconstructs occurrence facts after direct
   template-id resolution and submits another observation.
4. `analyze_template_declaration_source_patterns` recursively scans a template
   declaration, instantiates alias templates again, and submits an extra
   dependent pack-pattern observation.

The successful instantiation path is itself fragmented. Its local
`note_alias_use` lambda is called from nine return arms:

- builtin type transform;
- resolved structural dependent alias;
- direct type-id syntax;
- substituted AST type-id;
- general structural alias;
- cache hit;
- deferred dependent alias;
- dependent fallback;
- final parsed alias.

Those arms compute different forms of the same semantic result. Moving their
nine calls behind another witness helper would preserve the architectural
problem. The implementation must make the arms return or fill one resolution
object and perform observation at the operation boundary.

## Architectural cause

Alias instantiation returns `TypePtr`. The caller loses the association between
that type and the source template-id that selected it. Later code reconstructs
the missing facts from `parser_trace::current_use_location`, an exact-lookup
guard, template argument text, AST children, owner scopes, and the expanded
type.

`ResolvedAliasTemplateIdView` already describes many required fields, but it is
created after the semantic operation and acts as an observation request. It is
not the operation's result. Different callers fill different subsets, so the
table and renderer choose the best payload after emission.

The correction is to make source identity part of alias resolution. The
ordinary result remains the resolved `TypePtr`; the source result preserves how
that type arose. One collector owns the source occurrence and prevents repeated
consumers from publishing it again.

## Target semantic model

### Alias-template resolution result

Introduce a result with a name such as `ResolvedAliasTemplateId`. It should
contain or reference:

- `AliasTemplateDecl *` for the selected alias template;
- the resolved `TypePtr`, when resolution completed;
- the canonical resolved arguments and their default or pack provenance;
- the exact `TemplateIdSyntax` and alias-name token anchor;
- the semantic use scope;
- dependency, current-specialization, and source-pack facts;
- the alias expansion's retained class or nested source results;
- a stable source-occurrence identity;
- a parameterized pattern handle when concrete argument values do not yet
  exist.

Do not copy argument vectors or source strings merely for witness output. Use
pointers into operation-owned storage while the result is local. Retain compact
handles only for template patterns that outlive the operation.

The result must not contain rendered witness names, `bind` lines, or normalized
location strings. The observer builds those values under witness capture.

### Source occurrence identity

Define semantic identity before attempting to remove deduplication. Start with
the source syntax or token anchor plus the selected alias declaration. Add a
substitution or pattern identity only when provenance proves that one source
spelling legitimately produces multiple public rows.

The collector must answer these questions explicitly:

- Is this the source pattern or a replay of that pattern under instantiation?
- Does a direct source result already own a nested result at the same spelling?
- Is the source occurrence parameterized, concrete, or a materialized delayed
  dependent owner?
- Are two payloads distinct public uses or two analyses of one use?

Do not use rendered strings as identity. Do not move the current
location-and-name conflict rules into a new map unchanged.

### Dependent pattern result

Represent an unresolved dependent alias-template-id as an immutable semantic
pattern. Retain:

- exact source syntax;
- selected alias declaration when lookup is nondependent;
- unresolved qualified owner information when lookup is dependent;
- parameter references for each argument;
- source scope and declaration context;
- nested resolved-source children.

Substitution instantiates this pattern without reparsing source text or walking
the declaration AST. Public witness output should normally describe the source
pattern once. Concrete substitution may update semantic closure state, but it
must not create another source row for the same occurrence.

### Result ownership and memory

Keep direct results on the stack or in caller-owned operation storage. Use the
existing semantic side stores for delayed source results where possible. If a
new store is required, use compact integer handles and reuse AST and semantic
storage already owned by the translation unit.

The implementation must not grow `Type`, `TemplateArgument`, `ClassInfo`,
`FunctionBinding`, or `ValueBinding`. Record those sizes, `AliasTemplateDecl`,
the current view, the new result, and each retained entry after every phase.

Avoid:

- one heap allocation per alias use;
- copied `vector<TemplateArgument>` or `vector<string>` payloads;
- a `shared_ptr` graph for source children;
- normalized source locations in retained semantic objects;
- persistent provenance data in ordinary builds.

## Deduplication and `--witness-debug` disposition

### Alias deduplication is in scope

The alias branch in `semantic_source_use::record_source_use` currently:

- removes nested-derived rows when direct or source-owned rows arrive;
- rejects nested-derived rows when a direct row already exists;
- compares rows after binding-spacing normalization;
- adopts a more concrete template-id occurrence;
- adopts richer pack binding data.

These are semantic conflict rules. Delete them after the canonical collector
produces one complete alias row per occurrence.

The following renderer mechanisms are also alias deletion targets once their
strict and broad hit counts reach zero:

- the `WitnessBuilder::alias_events` replacement map;
- alias hits in `canonicalize_locations_and_dedupe`;
- `prefer_source_spelled_alias_events`;
- `collapse_duplicate_current_specialization_alias_pack_events`;
- `prefer_current_specialization_alias_duplicates`;
- alias hits in `dedupe_visible_events`;
- semantic repair in `qualify_member_alias_events_from_class_uses` and
  `canonicalize_placeholder_member_alias_owners`.

Pure text normalization may remain after it stops selecting between semantic
rows.

### Global witness deduplication is not yet removable

The same final provenance report records:

| Kind | Attempts | Exact duplicates | Table rows | Visible rows |
| --- | ---: | ---: | ---: | ---: |
| Class use | 4,531 | 2,310 | 2,049 | 1,953 |
| Function call | 920 | 133 | 787 | 599 |
| Variable use | 31 | 0 | 31 | 31 |

Class use now has one static producer, but repeated semantic execution still
submits the same occurrence. Function call still has several semantic feeds.
Removing shared table or renderer deduplication in this alias phase would
expose duplicate class and function rows.

This plan therefore removes alias conflict policy and proves zero alias hits in
generic passes. It leaves shared deduplication in place for other event kinds.
A later function-call convergence phase must address the 133 function
duplicates. Class occurrence idempotence must also reach zero duplicate
submissions before a global dedup path can be deleted.

### `--witness-debug` is not equivalent to `--witness`

Both modes call `collect_rendered_source_events`, so both consume the same
post-dedup event set. Debug mode then renders additional data:

- selected declaration locations;
- semantic parameter names instead of positional binding numbers;
- overload candidate counts;
- rejected candidate locations;
- guide declaration locations;
- lifecycle declaration locations, reasons, triggers, causes, and detail.

A probe using
`pa24/tests/spec/500-direct-alias-remains-deduced.t` produced 16 public lines
and 36 debug lines. The outputs are not exact matches today, and removing alias
deduplication would not make them match.

No repository test or script invokes `--witness-debug`; only CLI plumbing and
the renderer APIs reference it. That makes removal plausible as a separate API
cleanup, but output equality is not its justification. This plan does not
remove the option. If the debug-only fields are deliberately retired or moved
to the guarded provenance tooling, remove `--witness-debug` in a separate
commit with explicit approval and a user-facing compatibility note.

## Performance policy

### Fixed and rolling baselines

Use the lifecycle Phase 6 candidate as the fixed baseline. It was recorded at
the exact starting commit with three runs:

- artifact: `/tmp/cppgm-lifecycle-phase-6-final.json`;
- SHA-256:
  `f3321ba42cf500112b8d183a903a73cb92a5a58604ff6ff986043c9d4cf012ca`;
- instructions: `175889730826`;
- maximum RSS: `763817984`;
- peak footprint: `592760832`;
- workload epoch:
  `9764b3835e3c6996b6b80803054f80e1cf50f98e`.

Initialize the new artifacts without re-recording the baseline:

```sh
cp /tmp/cppgm-lifecycle-phase-6-final.json \
  /tmp/cppgm-alias-convergence-fixed.json
cp /tmp/cppgm-alias-convergence-fixed.json \
  /tmp/cppgm-alias-convergence-rolling.json
```

Verify the commit, metrics, workload identity, and digest before implementation.
If the artifact disappears, recover this exact file. Do not create a more
favorable replacement baseline. A new performance epoch requires a written
plan amendment and user approval.

The artifact was lost when `/tmp` was cleared on 2026-08-07. The user approved
recreating the evidence. The replacement was recorded at the same fixed commit
with the same compiler configuration, frozen workload epoch, and header
digest. Its SHA-256 is
`cefe54dacaaa8f6c5757cc90b3b9af2738507f55ab40d6abc226466114c2390b`;
its medians are 176,018,488,694 instructions, 757,092,352 bytes maximum RSS,
and 593,022,976 bytes peak footprint. Historical phase entries retain their
original comparison. New measurements identify and use this recreated fixed
artifact.

### Phase measurements

Commit each correctness-clean phase, then record one three-run candidate:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-alias-phase-N.json \
  --runs 3
```

Compare the recorded candidate with both baselines without rerunning it:

```sh
scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-alias-convergence-fixed.json \
  --candidate /tmp/cppgm-alias-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01

scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-alias-convergence-rolling.json \
  --candidate /tmp/cppgm-alias-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01
```

After recording the ledger entry, promote the same candidate file:

```sh
cp /tmp/cppgm-alias-phase-N.json \
  /tmp/cppgm-alias-convergence-rolling.json
```

### Intermediate interpretation

Intermediate phases may add a result before deleting its old feed. They do not
need to reduce every metric. The thresholds start an investigation:

- instructions at or above `+0.5%`;
- peak footprint above `+1%`;
- maximum RSS at or above `+3%`.

Investigate the changed semantic work, allocations, object sizes, and binary
sections before reverting a valid semantic change. Fix avoidable copies,
allocation, repeated traversal, or dual-path work within the phase. A changed
candidate gets a new commit and one new three-run batch. Do not rerun an
unchanged failed commit.

An RSS warning starts one confirmation batch of three runs. Record both
medians. A second result at or above 3% creates a failed final gate. During an
intermediate phase it creates a blocking cleanup obligation before more
metadata is added.

Pause metadata expansion if cumulative instructions exceed the fixed baseline
by 2% or footprint exceeds it by 3%. Reduce the representation before starting
another phase.

### Final performance gate

The final checkpoint compares with the fixed baseline:

- median instructions must be below `176018488694` for measurements made
  after the 2026-08-07 baseline recreation;
- a reduction smaller than 0.5% receives one confirmation batch, and both
  medians must remain below the fixed value;
- peak footprint may not exceed `598953205` bytes, the recreated fixed value
  plus 1%;
- RSS at or above `779805122` bytes, the recreated fixed value plus 3%,
  receives one three-run confirmation batch;
- a second RSS median at or above that threshold fails;
- production source lines under `dev/` and `dev/src/` must show a net deletion
  from the fixed checkpoint.

Report the less favorable instruction median when a confirmation is required.
If instructions do not decrease, continue cleanup and profiling. Correctness
and zero duplicate counts do not replace the final performance goal.

## Phase 0: Freeze evidence and make upstream routes visible

1. Copy and verify the fixed and rolling performance artifacts.
2. Preserve the final provenance trace and report outside the worktree. Record
   their paths and digests in the ledger.
3. Record production source lines, ordinary binary size, Mach-O text and data
   sections, and the hot structure sizes listed above.
4. Add diagnostic-only alias route IDs at the four observation feeds. Keep the
   final direct producer ID separate from route identity.
5. Extend the provenance analyzer to report alias attempts, table actions,
   renderer actions, and unique visible ownership by upstream route.
6. Add targeted coverage for an unexercised route only when no existing strict
   test reaches it.
7. Record the current `--witness` and `--witness-debug` difference as a contract
   fact. Do not turn that comparison into a parity gate.

No semantic behavior change belongs in this phase. Ordinary builds must not
retain route IDs or allocate provenance state.

Exit evidence:

- every alias observation call has a distinct route count;
- all 1,326 baseline attempts are attributed to a known route;
- the sum of route actions matches the source table and renderer totals;
- ordinary strict output and performance remain clean.

## Phase 1: Make alias instantiation return one completed result

Rewrite `instantiate_resolved_alias_template_impl` so its semantic arms produce
one result object.

1. Add the local `ResolvedAliasTemplateId` result without retaining new
   translation-unit storage.
2. Move common cache, source context, expansion, and observation work to the
   operation boundary.
3. Make builtin, structural, direct-syntax, AST, cache-hit, dependent, and
   fallback arms set the result and finish through that boundary.
4. Delete the local `note_alias_use` lambda and its nine call sites.
5. Keep resolution errors and substitution failure behavior unchanged.
6. Under provenance, compare the old and new observation payloads field by
   field for one transition commit only.
7. Delete the parity path before advancing.

This phase changes control flow, not source ownership. Repeated callers may
still observe the same occurrence, so alias table actions are not expected to
reach zero yet.

Exit evidence:

- one successful alias-resolution submission point;
- exact strict witness and LowIR output;
- no new hot-structure growth;
- fixed and rolling performance comparisons recorded.

## Phase 2: Make direct source resolution the sole owner

Move exact source-template-id facts into the normal semantic operation.

1. Pass the exact `TemplateIdSyntax`, name anchor, argument syntax, and source
   scope into alias resolution.
2. Build `SourceTemplateIdOccurrence` once from that structured syntax.
3. Preserve value-owner, qualified-member, current-specialization, default,
   and pack facts in the semantic result.
4. Build witness bindings once in the observer from the result.
5. Delete `record_direct_alias_template_source_use_if_needed` and its second
   `observe_resolved_alias_template_id` call.
6. Remove duplicate occurrence and binding construction from
   `template_argument_semantics.cpp`.
7. Replace source-token relocation with the exact syntax anchor for direct
   source uses.

Use these fixtures first:

- `pa24/tests/spec/500-direct-alias-remains-deduced.t`;
- `pa24/tests/spec/400-alias-template-empty-nested-pack-member-type.t`;
- `pa22/tests/general/400-alias-nontype-pack-partial-specialization-pattern.t`;
- `pa22/tests/general/400-alias-pack-expansion-through-alias.t`.

Exit evidence:

- the direct-template-argument route reaches zero;
- all rows uniquely owned by that route move to the canonical result;
- occurrence and binding fields match the baseline;
- table replacement counts do not increase.

## Phase 3: Replace dependent observation with a typed pattern

Remove observation side effects from failure and incomplete-resolution arms.

1. Make unresolved dependent resolution return a parameterized alias result
   instead of calling `observe_dependent_pattern` from `catch` and early-return
   paths.
2. Retain the source pattern when the surrounding template declaration is
   analyzed.
3. Let substitution consume the retained pattern and semantic binding frame.
4. Keep one source-use identity across repeated substitutions.
5. Separate semantic substitution failure from witness publication so failed
   speculative resolution does not create a row.
6. Delete `dependent_pattern` observation policy that can now be derived from
   the result.

Target high-overlap fixtures include:

- `pa24/tests/general/500-defaulted-pack-bool-short-circuit-sfinae.t`;
- `pa24/tests/general/500-member-template-dependent-owner-defaulted-sfinae.t`;
- `pa24/tests/general/500-mp11-append-alias-template-sfinae.t`;
- `pa24/tests/spec/300-alias-template-sfinae-fallback.t`.

Exit evidence:

- no alias observation from exception handling;
- repeated substitutions reuse one occurrence handle;
- exact duplicate attempts fall by the measured contribution of this route;
- failed speculative paths own no visible output.

## Phase 4: Converge template-pattern alias analysis

Move nested alias ownership into typed children produced by normal type,
expression, qualified-id, and template-argument analysis. Keep a source-pattern
analyzer only for declaration positions that those operations do not visit.

1. Retain child alias results while analyzing template parameter clauses,
   declaration types, bases, return types, and expressions.
2. Preserve nested template-id syntax and semantic owner selection in those
   child results.
3. Submit child occurrences through the same source collector as direct alias
   results.
4. Remove alias target instantiation, exact-lookup guard reconstruction, and
   direct observation from `analyze_template_declaration_source_patterns`.
5. For a source position with no normal semantic owner, select the alias once,
   build typed parameterized arguments from structured syntax, and enter the
   canonical completion boundary.
6. Delete traversal of any position that normal semantic analysis demonstrably
   owns. Record the strict-corpus rows lost when a proposed deletion has no
   replacement owner.
7. Remove dead route IDs and `AliasUseEmissionOrigin` cases as they reach zero.

The migration suite must cover nested type-ids, template-template arguments,
qualified member aliases, defaults, and pack expansion. Start with fixtures
that hit `prefer_source_spelled_alias` and the nested-derived rejection path.

Exit evidence:

- the old recursive-pattern producer route is zero;
- nested-derived submissions are created only when they are independently
  source-facing;
- no second alias target instantiation or observer request occurs for witness
  capture;
- all previously unique walker rows have a typed semantic owner.

## Phase 5: Remove location, owner, and payload recovery

Finish member and qualified alias handling from structured semantic results.

1. Carry exact qualified owner results and alias declaration anchors through
   normal lookup.
2. Derive current-scope and dependent-owner state from the selected semantic
   owner, not source-text searches.
3. Remove calls that scan forward or across a line to find the alias
   identifier.
4. Remove `template_argument_texts_mention_*` checks used only to decide which
   competing alias row should survive.
5. Produce the final qualified alias name and preserved member binding from the
   source result.
6. Drive `qualify_member_alias_events_from_class_uses` and
   `canonicalize_placeholder_member_alias_owners` to zero alias hits, then
   delete their alias repair logic.
7. Remove `normalize_selected_decl_to_line_start`,
   `unwrap_single_pack_binding`, and
   `use_template_argument_binding_policy` after their policy moves to the
   semantic result or observer.

Exit evidence:

- every alias use has an exact source anchor before rendering;
- the renderer no longer repairs alias ownership or placeholder owners;
- no retained object stores normalized source text;
- output remains exact on member-alias and qualified-owner fixtures.

## Phase 6: Install occurrence idempotence and delete alias deduplication

Make the source collector the only alias publication owner.

1. Key collected alias results by semantic source occurrence and required
   pattern identity.
2. Publish each completed occurrence once after required semantic
   materialization.
3. Remove remaining observation calls from alias-resolution consumers.
4. Delete alias conflict handling from
   `semantic_source_use::record_source_use`.
5. Replace `WitnessBuilder::alias_events` with direct ordered consumption.
6. Delete alias-only renderer preference and duplicate-collapse functions
   after both strict and broad provenance show zero hits.
7. Remove alias branches from generic dedup passes only where they are
   separately expressed. Keep the generic pass for other event kinds.
8. Delete `AliasUseEmissionOrigin`, dead capture-pause policy, temporary route
   IDs, parity diagnostics, and unused adapters.
9. Compact result storage and remove fields no longer read.

Required provenance result:

- alias attempts equal alias insertions;
- exact duplicate, rejected, replaced, and enriched alias actions are zero;
- alias table rows equal final visible alias rows;
- renderer removed or replaced actions for alias are zero;
- unknown alias producers and routes are zero.

The observer may still invoke pure name, path, and binding text normalization.
Those passes must not decide which semantic alias result survives.

## Phase 7: Final audit and global handoff

1. Run the full provenance corpus and verify every Phase 6 alias invariant.
2. Record remaining class, function, and variable table and renderer actions.
3. Confirm that shared deduplication is retained only for named non-alias
   obligations.
4. Write the function-call convergence handoff from current route counts and
   unique-output ownership.
5. Record final production lines, structure sizes, binary sections, strict and
   broad correctness, and performance.
6. After this plan and the class-materialization ownership plan are both
   complete, run their shared inception gate from an isolated object root.
7. Close every ledger obligation or name its owner and next phase.

Do not remove `--witness-debug` in this phase under an output-parity claim. If
the project chooses to retire the diagnostic interface, make that a separate
approved cleanup after preserving any required diagnostics elsewhere.

## Correctness gates

### Targeted gate

Run the fixtures owned by the route or renderer policy being removed. Compare
public witness text exactly and inspect diagnostic provenance for field
ownership.

### Strict gate

Run strict witness validation with direct LowIR comparison:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-strict \
  STRICT_PAS='pa19 pa20 pa22 pa23 pa24' \
  STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Run the same strict corpus with witness provenance enabled. A removed route or
policy must have zero hits, not merely zero visible rows.

### Broad gate

Run the full report with direct LowIR comparison:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-report \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

The expected broad surface at the fixed checkpoint is 4,860 tests. Record the
actual count in the ledger because later test additions may raise it.

### Inception gate

Run only after the final committed cleanup for both active witness plans:

```sh
make inception \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  INCEPTION_OBJ_ROOT_BASE=/tmp/cppgm-witness-alias-inception
```

Use a fresh isolated root if that path already contains another experiment.

## Commit and ledger discipline

Keep each semantic phase as a short sequence:

1. add the result or route parity diagnostic;
2. move callers to the result;
3. delete the old algorithm and producer;
4. run correctness and provenance;
5. commit the clean phase;
6. record one three-run candidate and both comparisons;
7. promote the candidate to the rolling baseline.

Do not keep two complete alias-resolution algorithms across more than one
measured checkpoint. Do not accept a row because the renderer hides its
duplicate. Do not revert a valid semantic result before investigating an
instruction or memory warning and attempting an in-scope correction.

The ledger entry for each phase must include:

- commit and production line delta;
- route counts and source table actions;
- renderer alias actions;
- targeted, strict, broad, and inception state as applicable;
- fixed and rolling instruction, RSS, and footprint deltas;
- RSS confirmation result when triggered;
- hot structure sizes and retained-result allocation counts;
- any temporary scaffold and the phase that deletes it.

## Completion criteria

The work is complete when:

- alias instantiation has one completed semantic result and one observation
  boundary;
- the nine result-arm observation calls, direct replay, dependent side effect,
  and recursive alias target instantiation are gone;
- every visible alias row comes from a typed source occurrence;
- alias table arbitration and renderer row selection are gone;
- alias provenance reports one insertion per attempt and no destructive
  renderer action;
- strict and broad gates pass, followed by the joint inception gate after both
  active witness plans are complete;
- production code is net smaller than the fixed checkpoint;
- median instructions are lower than the fixed checkpoint;
- footprint and RSS meet their final gates;
- remaining global deduplication is tied to explicit class or function
  obligations rather than alias behavior.
