# Witness Convergence Reset and Recovery Plan

## Status and authority

This plan resets execution of the active witness cleanup after the regenerated
strict corpus exposed both new coverage gaps and regressions in the unfinished
alias/class changes. It governs the remaining work in:

- `docs/witness-alias-semantic-convergence-plan.md`;
- `docs/witness-class-materialization-semantic-ownership-plan.md`;
- the function-call, variable-use, and lifecycle gaps exposed by the expanded
  patched-Clang reference corpus.

The earlier plans remain the architectural specifications for their event
families. When their recorded completion state conflicts with the evidence in
this document, this document controls. In particular, neither active plan is
complete, and inception remains forbidden until every final gate here passes.

- Worktree: `/Users/vishvananda/cppgm-alias-consolidation`
- Branch: `experiment-witness-alias-path-consolidation`
- Audit date: 2026-08-08
- Clean control: `5add5290c69be6b76138dfc1f6696915eb0278ae`
- Required host compiler: `/usr/local/opt/llvm/bin/clang++`
- Patched-Clang reference compiler:
  `/Users/vishvananda/llvm-project-template-metrics-20260416/build-clang-template-trace/bin/clang++`
- Patched LLVM checkout: `59c5d9c...`

Do not continue semantic implementation from the present dirty compiler tree.
Preserve its evidence first, then reconstruct the useful changes on the clean
control in small correctness-clean phases.

## Why the reset is necessary

The prior completion claims were based on 1,305 tracked witness references.
Full regeneration added 225 patched-Clang references, producing a 1,530-test
strict surface. The larger corpus shows that the old gate omitted material
alias, class, function, variable, and lifecycle behavior.

The uncommitted implementation also combines several incomplete migrations.
It fixes some of the newly covered cases, but it regresses substantially more
previously passing behavior, grows production code, and has a large measured
performance cost. Continuing to add local corrections would make it harder to
identify the semantic operation that actually owns each witness row.

The recovery therefore has three distinct jobs:

1. restore the previously passing correctness surface;
2. converge each newly exposed event family through its primary semantic
   operation rather than through filters or replay;
3. delete the migration scaffolding and prove a final code and performance
   reduction.

## Audit snapshot

### Worktree and build state

At the audit point, 53 tracked files differ from `HEAD` by 3,382 additions and
2,165 deletions. `dev/src` alone differs by 3,194 additions and 2,165
deletions, a net addition of 1,029 lines. Using the same source-file count on
both trees:

| Tree | `dev/src` lines |
| --- | ---: |
| Clean `HEAD` | 413,807 |
| Dirty audit tree | 414,836 |
| Delta | +1,029 |

The ordinary compiler is 17,006,872 bytes. Its Mach-O `__text` section is
11,787,369 bytes. A Homebrew-Clang rebuild succeeds but reports three unused
variables in migration/provenance code:

- `callsemantic.cpp`: `has_class_context`;
- `template_argument_semantics.cpp`: `retained`;
- `template_selection.cpp`: `dependency_count_before`.

These warnings are cleanup obligations, not harmless final state.

The class-materialization Phase 0 baseline required by its original plan,
`/tmp/cppgm-class-materialization-ownership-fixed.json`, was never recorded.
The implementation therefore advanced without the required clean,
post-diagnostic performance epoch.

### Strict correctness

Both trees were rebuilt with Homebrew Clang and run against the same 1,530
patched-Clang references with direct LowIR comparison.

| Surface | Clean `HEAD` | Dirty audit tree |
| --- | ---: | ---: |
| Comparisons | 1,530 | 1,530 |
| Passing | 1,339 | 1,218 |
| Failing | 191 | 312 |

Set comparison gives the important transition:

| Relationship | Tests |
| --- | ---: |
| Fail on both trees | 173 |
| Fixed by the dirty tree | 18 |
| Regressed only on the dirty tree | 139 |

Of the dirty tree's 312 failures, 133 use previously tracked references and
179 use the newly generated references. Clean `HEAD` passes all 1,305 tracked
references, so the 139 current-only failures consist of 133 tracked
regressions and six newly covered regressions. The dirty tree fixes 18 of the
191 clean-tree coverage gaps but is not a viable correctness base.

Per-PA dirty-tree results are:

| PA | Compared | Failing |
| --- | ---: | ---: |
| PA19 | 279 | 37 |
| PA20 | 158 | 22 |
| PA22 | 293 | 78 |
| PA23 | 385 | 103 |
| PA24 | 415 | 72 |

Failure-family counts below are test counts and overlap when one test has more
than one kind of mismatch:

| Family | Clean `HEAD` gaps | Dirty-tree gaps |
| --- | ---: | ---: |
| Alias use | 24 | 13 |
| Class use | 62 | 195 |
| Function call | 86 | 84 |
| Variable use | 3 | 2 |
| Lifecycle | 79 | 86 |

The dirty tree's row-level differences are:

| Family | Changed rows | Extra rows | Missing rows |
| --- | ---: | ---: | ---: |
| Alias use | 11 | 2 | 1 |
| Class use | 60 | 216 | 34 |
| Function call | 44 | 27 | 54 |
| Variable use | 1 | 1 | 0 |
| Lifecycle | n/a | 130 | 212 |

There are no reorder-only failures. The dominant class symptom is actual
over-publication, not formatting or ordering.

### Broad correctness

The current Homebrew-Clang PA1-PA38 report passes 4,857 of 4,862 tests. Its
five failures are:

1. `pa12/tests/general/200-switch-case-declaration.t`: the compiler now
   correctly rejects a label that bypasses initialization, but this older
   fixture still expects success. Reconcile the fixture/reference with the new
   PA15 and PA19 negative coverage; do not weaken the compiler.
2. `pa23/tests/spec/300-conversion-function-template-owner-result-copy-init.t`:
   generated LowIR loses the conversion-function body and required nested
   definitions.
3. `pa23/tests/spec/300-dependent-member-template-call-enable-if.t`: the
   generated symbol changes a value-template argument from the required
   `Lv0E` form to `Li0E`.
4. `pa30/tests/general/300-runtime-function-local-static-storage.t`: compiler
   execution fails before the expected runtime check.
5. `pa30/tests/general/300-runtime-local-class-enclosing-enumerator.t`:
   compiler execution fails before the expected runtime check.

The two PA23 LowIR failures and two PA30 failures are semantic regressions in
the dirty tree. They are not witness-renderer issues and must be clean before
any witness phase is accepted.

### Static and provenance audits

The current tree does have useful progress worth retaining as design evidence:

- `scripts/audit_witness_materialization.py` reports no findings, two decision
  boundaries, and six forbidden-symbol checks;
- `scripts/audit_text_reparse.py` reports zero hits in all 23 tracked
  categories;
- the provenance analyzer's six unit tests pass;
- the forbidden class materialization helper names are absent.

These checks are necessary but insufficient. The temporary member-alias bridge
performs a second structured AST walk through `parse_template_parameters`.
That is not source-text reparsing, so the text-reparse audit correctly does not
flag it, but it is still duplicate semantic work and may not survive the
recovery.

Temporary diagnostic/scaffold that must not reach the final tree includes:

- alias `lexical-owner` and `active-owner` trace fields;
- `class.materialization` parser tracing;
- the three `current-specialization-*` trace points;
- the local member-alias `Scope`/`AliasTemplateDecl` reconstruction and second
  template-parameter analysis;
- shadow-only counters and fields after their parity question is answered.

### Performance

The fixed diagnostic-inclusive baseline remains
`/tmp/cppgm-alias-convergence-fixed.json`, SHA-256
`cefe54dacaaa8f6c5757cc90b3b9af2738507f55ab40d6abc226466114c2390b`:

| Metric | Fixed baseline | Dirty audit median | Delta |
| --- | ---: | ---: | ---: |
| Instructions | 176,018,488,694 | 192,990,370,390 | +9.64% |
| Maximum RSS | 757,092,352 | 793,448,448 | +4.80% |
| Peak footprint | 593,022,976 | 620,085,248 | +4.56% |

Against the last clean class-materialization confirmation, the dirty median is
+10.12% instructions, +6.84% RSS, and +7.74% footprint.

The first dirty batch's RSS samples are 790,216,704, 793,448,448, and
796,696,576 bytes: a 6,479,872-byte range, or about 0.82% of the median. The
measured 4.80%-6.84% RSS increase is therefore much larger than this batch's
run-to-run variation.

The required second three-run gate confirms the failure:

| Metric | Fixed baseline | Confirmation median | Delta |
| --- | ---: | ---: | ---: |
| Instructions | 176,018,488,694 | 192,910,337,261 | +9.60% |
| Maximum RSS | 757,092,352 | 791,990,272 | +4.61% |
| Peak footprint | 593,022,976 | 620,224,512 | +4.59% |

The confirmation report is
`/tmp/cppgm-reset-current-perf-confirmation.json`, SHA-256
`50655ba49dc3b9d11ec49fda4034f2aa990ebc42f387d9ee746593cef034a957`.
The second RSS median remains above 3%, so the dirty checkpoint fails the RSS
rule in addition to correctness, instructions, and footprint. It may not
become a rolling baseline.

## Architectural diagnosis

### Class materialization is attached too high in the call graph

The unfinished implementation applies source-materialization scopes around
general declaration/type analysis. Those functions are also used for lookup,
SFINAE, replay, and other queries that do not materialize a public source type
node. A typed flag at that level is structured, but it is still the wrong
semantic fact. The 216 extra class rows are the expected symptom.

Materialization must originate only at the operation that consumes a specific
source AST node as a concrete type. Lookup may return the same `TypePtr`; that
does not make the lookup a source materialization.

### Alias completion still has two semantic analyses

The canonical completion object and source-owner improvements fix real cases,
especially current-specialization aliases. However, member aliases that normal
collection did not expose were bridged by constructing a second local semantic
scope and re-running template-parameter analysis. This recreates part of the
declaration collector and perturbs lifetime/lookup behavior.

Factor declaration indexing from alias-target resolution in the primary
collector. The normal collector should publish a compact typed declaration
handle once; source occurrence analysis should consume that handle without
rebuilding the scope, parsing parameters again, or instantiating the alias
target twice.

### Source spelling, selected semantics, and public owner are conflated

The remaining alias failures fall into four clusters:

- six structured spelling/layout differences;
- four wrong source-owner presentations;
- two extra qualified alias rows;
- one missing nested alias row.

The semantic result needs separate fields for resolved meaning, source AST
spelling, and public owner mode. The owner mode must be a fact from lookup
(lexical source owner versus selected concrete owner), not inferred by the
renderer or chosen by whichever completion writes first.

### First-writer occurrence storage hides repeated work

The pending alias occurrence map currently allows one partially populated
completion to win and later completions to enrich or lose. That preserves the
same architectural problem as renderer deduplication at an earlier layer.
Occurrence identity should make repeated analysis unnecessary; the primary
semantic operation should produce the complete result once.

### The expanded corpus exposes additional convergence work

All current function-call mismatches and both variable-use mismatches are in
newly generated references; they are not regressions against the old 1,305
gate. They nevertheless prevent the regenerated corpus from becoming the new
correctness contract. Function calls still have multiple request-building
feeds and generic arbitration, so they need their own typed-result convergence
rather than alias/class exceptions.

Lifecycle mismatches mix 16 tracked regressions with 70 new-reference gaps in
the dirty tree. Lifecycle demand must be generated by definition/instantiation
semantics and must not be used as a class- or alias-visibility policy.

## Recovery rules

These rules apply to every phase:

1. No source-text, token, rendered-name, template-name, fixture, or
   source-location filter may decide whether a row exists.
2. Source AST may be retained for final spelling. It may not be reparsed to
   recover semantic ownership.
3. A phase moves behavior to the semantic operation that already computes it;
   it does not add another observer-side analysis.
4. Do not keep two complete semantic algorithms across more than one shadow
   checkpoint. Shadow code answers one parity question and is then deleted.
5. The old 1,305 tracked-reference gate must remain exact after every behavior
   change. A tracked regression blocks the phase immediately.
6. On the 225 new references, the failing set may only shrink. Record any test
   whose mismatch changes family or payload even when the total count falls.
7. Collect provenance for all 1,530 examples, not only focused fixtures.
   Counters must name the semantic owner and distinguish attempt, insertion,
   repeated analysis, publication, renderer action, and final visibility.
8. Investigate semantic duplication, extra allocation, or representation
   growth before rolling back a valid semantic change merely because a
   performance warning appears.
9. No global witness deduplication is removed until alias, class, function,
   variable, and lifecycle provenance separately proves that the affected
   generic branch has zero destructive actions.
10. Use Homebrew Clang for every build and performance gate. Inception is last.

## Execution plan

### Phase 0: Preserve the experiment and restore a trustworthy base

1. Preserve the current tracked diff, the 18 fixed-test list, the 139
   regression list, focused output diffs, and the 225 new `.ref.witness` files
   under a durable side reference or hashed patch bundle. Exclude `.my`,
   provenance-expanded, object, profiler, and other generated files.
2. Add the 225 patched-Clang `.ref.witness` files to the durable test corpus.
   Record the patched compiler path and LLVM revision in the ledger.
3. Restore production compiler files to clean `5add5290c...`; do not discard
   the preserved experiment. Keep the regenerated references.
4. Reconcile the stale PA12 switch fixture with the correct bypass rejection.
   The positive form needs braces or the reference must expect rejection,
   according to the behavior that fixture intends to own.
5. Rebuild ordinary and provenance compilers with Homebrew Clang. Remove all
   ordinary-build warnings.
6. Establish the recovery checkpoint:
   - tracked strict: 1,305/1,305;
   - expanded strict: 1,339/1,530, with the same 191-gap manifest;
   - broad: 4,862/4,862 after the PA12 fixture correction;
   - text-reparse, materialization, and provenance unit audits clean.
7. Add only the diagnostic counters required by the expanded provenance
   contract, prove ordinary output parity, then record the missing
   `/tmp/cppgm-class-materialization-ownership-fixed.json` three-run baseline.

Do not replay any semantic fix until this phase is committed and its evidence
is recorded.

### Phase 1: Establish expanded-corpus ownership evidence

1. Regenerate ordinary and provenance output for all 1,530 tests.
2. Give each class, alias, function, variable, and lifecycle producer a stable
   diagnostic route ID at its true semantic operation.
3. Produce one report containing, per source occurrence:
   - patched-Clang expected row or expected absence;
   - CPPGM semantic owner and operation kind;
   - attempt/insertion/publication counts;
   - direct, nested, replay, lookup-only, and materialization state;
   - source occurrence ID and selected declaration ID.
4. Record the clean 191-gap manifest by event family and semantic owner.
5. Add reduced tests only for a proven unexercised owner; never add a fixture
   solely to encode an implementation exception.

Acceptance: diagnostic and ordinary output agree; counters cover every
attempt; no unknown producer remains; ordinary structures and allocation
counts do not grow.

### Phase 2: Converge class source materialization

Start from the clean 62 class-gap set, not the dirty 195-gap set.

1. Introduce a compact parameterized source-occurrence handle during primary
   template-id analysis.
2. Produce `ResolvedSourceTypeMaterialization` only when an operation analyzes
   that exact source AST type node into a concrete type. General lookup,
   SFINAE, constant query, and replay paths may consume types but may not set
   the fact.
3. Migrate the five audited positive owners one by one: variable-template
   initializer, static-member initializer, instantiated function-body type,
   declaration type, and conversion-function body.
4. For each owner, compare typed materialization with patched-Clang presence
   and absence across the entire corpus. Keep the 55 known rejected locations
   at zero and classify every newly covered location by the same rule.
5. Delete the overbroad materialization scopes and all ambient lifecycle,
   source-mode, conversion-name, and source-spelling admission logic.
6. Delete shadow fields immediately after parity is established.

Acceptance:

- zero class-use mismatch on 1,530 tests;
- one attempt and one insertion per visible class occurrence;
- zero class renderer visibility actions;
- no materialization fact from lookup-only analysis;
- forbidden materialization reparses remain absent;
- tracked strict and broad suites are exact.

### Phase 3: Finish alias semantic convergence

Start from the clean 24 alias-gap set and preserve the useful behavior shown by
the 12 alias-family fixes in the dirty experiment.

1. Factor declaration indexing from alias-target resolution in the primary
   template declaration collector. Publish one typed member-alias handle.
2. Remove the local reconstructed `Scope`/`AliasTemplateDecl` bridge and the
   second `parse_template_parameters` pass.
3. Make the canonical alias completion result carry separate structured facts
   for:
   - resolved type and arguments;
   - exact source AST and anchor;
   - selected alias declaration;
   - lexical source owner;
   - selected concrete owner;
   - explicit public owner mode from lookup;
   - nested child occurrence ownership.
4. Make builtin, cache, direct, dependent, current-specialization, member, and
   fallback arms finish through that one completion boundary.
5. Resolve the four observed clusters through those facts: spelling, owner
   presentation, extra qualified occurrences, and missing nested occurrence.
6. Delete first-writer enrichment, recursive alias target instantiation,
   consumer replay, and alias-specific table/renderer conflict policy as their
   provenance counts reach zero.

Acceptance:

- zero alias-use mismatch on 1,530 tests;
- one successful completion and one insertion per source occurrence;
- no second AST parameter analysis or alias-target instantiation;
- alias duplicate/reject/replace/enrich actions are zero;
- alias table rows equal public rows;
- tracked strict and broad suites are exact.

### Phase 4: Repair lifecycle ownership independently

1. Classify the clean 79 lifecycle gaps by semantic demand:
   `ensure-definition`, `require-definition`, class instantiation, function
   instantiation, and variable instantiation.
2. Attach each demand to the operation that creates or consumes the required
   semantic entity. Do not infer lifecycle from a source-use row, and do not
   use lifecycle context to admit a source-use row.
3. Collapse repeated demand production at the owning entity/state transition,
   not in the renderer.
4. Recheck the two PA23 LowIR regressions while migrating conversion-function
   and enable-if ownership; lifecycle output and generated definitions must
   agree on the same semantic result.

Acceptance: zero lifecycle mismatches, exact PA23 LowIR, no lifecycle-driven
class/alias admission, and one recorded state transition per public lifecycle
event.

### Phase 5: Converge function-call and variable-use results

1. Inventory the remaining function request-building feeds and measure unique
   row ownership across all 1,530 tests.
2. Make overload/call resolution return one typed source-call result carrying
   callee, selected declaration, bindings, rejected candidates, source anchor,
   and required definition demand.
3. Remove consumer-side request reconstruction and repeated speculative
   publication. A failed candidate may remain debug data but may not create a
   public source row.
4. Apply the same operation-result rule to the three clean variable-use gaps.
5. Delete function/variable table or renderer arbitration only after attempts
   equal insertions and generic destructive-action counters are zero.
6. Re-run the PA30 failures as focused gates after each semantic ownership
   change; the compiler must execute both programs successfully.

Acceptance: zero function-call and variable-use mismatches, exact PA30 focused
runtime tests, and no remaining generic dedup obligation for these families.

### Phase 6: Delete migration scaffolding and prove simplification

1. Delete all temporary trace fields, shadow counters, parity paths, local
   semantic mirrors, stale route enums, and unused adapters.
2. Remove diagnostic code from ordinary layouts and prove that `Type`,
   `TemplateArgument`, and `ClassInfo` do not grow.
3. Audit every remaining source table and renderer pass. Each destructive
   branch must have a named nonzero semantic obligation or be deleted.
4. Run static audits for forbidden text/token reconstruction and duplicate AST
   semantic walks. Extend the audit where the current text-reparse audit cannot
   see structured duplicate analysis.
5. Confirm production source is smaller than the Phase 0 recovery checkpoint
   and that the ordinary build is warning-free.

Acceptance: all migration-only code is gone; production source has a net
deletion; generic dedup remains only for explicitly documented, nonzero
obligations.

### Phase 7: Final correctness and performance gates

Run from a committed, clean worktree with an isolated object root:

1. focused positive/negative fixtures for every migrated owner;
2. ordinary and provenance strict with direct LowIR: 1,530/1,530;
3. PA1-PA38 report: 4,862/4,862 or the then-current fully explained count;
4. provenance invariants for every event family;
5. materialization, text-reparse, duplicate-semantic-walk, and analyzer unit
   audits;
6. ordinary binary and section sizes plus hot structure sizes;
7. one three-run performance candidate against both fixed and rolling
   baselines.

Performance policy:

- instructions: investigate at +0.5% during intermediate work; final median
  must be below the Phase 0 fixed checkpoint;
- peak footprint: at most +1%; final reduction remains a goal;
- maximum RSS: +3% warning starts one second complete three-run gate; a second
  median at or above +3% fails;
- intermediate typed-result phases may temporarily grow, but every
  correctness-clean phase records the divergence and its cleanup owner;
- investigate copies, allocations, repeated semantic operations, code layout,
  and side-store growth before abandoning a valid semantic result;
- promote a rolling baseline only after correctness, provenance, and the
  applicable performance decision are clean.

### Phase 8: Joint inception and closure

Only after Phases 0-7 are committed and both original active plans have their
completion checklists reconciled with the 1,530-reference corpus:

```sh
make inception \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  INCEPTION_OBJ_ROOT_BASE=/tmp/cppgm-witness-convergence-inception
```

Record the isolated object root, host compiler, final commit, and result in all
three ledgers. Inception does not substitute for any earlier correctness,
provenance, size, source-reduction, or performance gate.

## Per-phase validation cadence

For every behavior-changing phase:

1. run focused tests and inspect semantic provenance;
2. run the tracked 1,305-reference strict gate; stop on any regression;
3. run the expanded 1,530-reference gate and prove the remaining set only
   shrinks;
4. run affected PA broad tests, followed by full PA1-PA38 at the phase commit;
5. remove the superseded algorithm before measuring;
6. record source, structure, binary, and three-run performance evidence;
7. commit the phase and update the recovery ledger before advancing.

This cadence is intentionally stricter than the recent implementation loop.
A focused exact match is evidence for one owner, not permission to accumulate
unmeasured regressions elsewhere.

## Final completion criteria

- [ ] Current experiment is preserved without generated artifact noise
- [ ] All 225 new patched-Clang references are durable
- [ ] Recovery checkpoint passes tracked strict and full broad correctness
- [ ] Class materialization originates only at source-node semantic operations
- [ ] Alias declaration indexing has no second semantic parameter analysis
- [ ] Alias, class, function, variable, and lifecycle mismatches are all zero
- [ ] Strict with direct LowIR passes 1,530/1,530
- [ ] PA1-PA38 report passes the complete current corpus
- [ ] PA23 LowIR and PA30 runtime regressions are resolved semantically
- [ ] No forbidden source-text/token reparse or fixture filter exists
- [ ] No migration trace, shadow algorithm, or ordinary-build warning remains
- [ ] Per-family attempts equal insertions and renderer destructive actions are zero
- [ ] Production source is smaller than the recovery fixed checkpoint
- [ ] Final instructions are below the recovery fixed checkpoint
- [ ] Footprint and RSS satisfy the final policy
- [ ] Alias and class plan ledgers are reconciled with expanded-corpus evidence
- [ ] Joint inception passes last
