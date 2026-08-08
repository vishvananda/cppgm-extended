# Witness alias semantic convergence ledger

This ledger records the implementation evidence for
`docs/witness-alias-semantic-convergence-plan.md`. The fixed comparison point
is lifecycle Phase 6 at `05b0c7a21ff497cd2186fabc2096bf04cc6e931b`.

## Fixed evidence

- Original fixed performance artifact, lost in the reboot:
  `/tmp/cppgm-alias-convergence-fixed.json`, copied byte-for-byte from
  `/tmp/cppgm-lifecycle-phase-6-final.json`.
- Original fixed and initial rolling SHA-256:
  `f3321ba42cf500112b8d183a903a73cb92a5a58604ff6ff986043c9d4cf012ca`.
- Original fixed medians: 175,889,730,826 instructions, 763,817,984 bytes
  maximum RSS, and 592,760,832 bytes peak footprint.
- Workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`.
- Corrected lifecycle prerequisite: `025730e00d8ea16aa1b98a4257a53b35481d9e69`
  in this worktree. Its committed production diff from the fixed checkpoint is
  43 additions and 313 deletions, a net deletion of 270 lines.
- The current hot sizes are: `Type` 280, `TemplateArgument` 136,
  `ClassInfo` 1,136, `FunctionBinding` 824, `ValueBinding` 504,
  `AliasTemplateDecl` 264,
  starting `ResolvedAliasTemplateIdView` 80, `RetainedAliasClassUse` 16,
  `ResolvedClassTemplateIdView` 104, `ResolvedQualifiedId` 40, and
  `ResolvedOwnerReference` 32 bytes. These match the corrected starting point.
- `--witness` and `--witness-debug` are not equivalent. On
  `pa24/tests/spec/500-direct-alias-remains-deduced.t` the public log has 16
  lines and the debug log has 36. Debug output adds declaration locations,
  named bindings, candidate counts, drop locations, and lifecycle detail.

### Recreated fixed artifact after reboot

The reboot on 2026-08-07 removed the fixed file from `/tmp`. With user
approval, the fixed compiler was rebuilt at the exact post-counter commit
`05b0c7a21ff497cd2186fabc2096bf04cc6e931b` and the unchanged frozen workload
was recorded for three runs. The replacement artifact is
`/tmp/cppgm-alias-convergence-fixed.json`, SHA-256
`cefe54dacaaa8f6c5757cc90b3b9af2738507f55ab40d6abc226466114c2390b`.
Its medians are 176,018,488,694 instructions, 757,092,352 bytes maximum RSS,
and 593,022,976 bytes peak footprint. The workload epoch and 51-header digest
remain unchanged.

The historical phase deltas in this ledger continue to describe the original
fixed artifact. Measurements after the reboot compare with the recreated
artifact and identify it explicitly. Its instruction median is 0.07% above
the lost artifact, RSS is 0.88% below it, and footprint is 0.04% above it; the
replacement is not uniformly more favorable.

## Checkpoints

| Phase | Commit | Alias attempts | Insert / duplicate / reject / replace | Strict | Broad | Performance | State |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| 0. Route evidence | `e99c510d3` | 1,326 | 766 / 387 / 159 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.23%, RSS +0.15%, footprint -0.07% | complete |
| 1. Completed result | `497376554` | 1,326 | 766 / 387 / 159 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.17% fixed / +0.06% rolling; RSS -0.20% / -0.35%; footprint -0.09% / -0.02% | complete |
| 2. Direct source owner | `f5529cc60` | 1,416 | 653 / 573 / 176 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.43% fixed / -0.26% rolling; RSS -2.55% / -2.35%; footprint -2.94% / -2.86% | complete |
| 3. Dependent pattern result | `bd3b81402` | 1,416 | 671 / 579 / 164 / 2 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.12% fixed / +0.31% rolling; RSS -1.43% / +1.15%; footprint -2.93% / +0.02% | complete |
| 4. Pattern analysis | `2252c751b` | 1,211 | 854 / 328 / 27 / 2 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.39% fixed / -0.27% rolling; RSS -2.14% / -0.72%; footprint -2.94% / -0.01% | complete |
| 5. Structured source facts | `7b2ecdadf` | 564 | 564 / 0 / 0 / 0 | diagnostic and ordinary 1,305/1,305 | 4,860/4,860 | instructions -0.10% fixed / +0.34% rolling; RSS -1.05% / +0.87%; footprint -2.81% / +0.14% | complete |
| 6. Occurrence idempotence | `ed605692b` | 564 | 564 / 0 / 0 / 0 | diagnostic and ordinary 1,305/1,305 | 4,860/4,860 | confirmed instructions -0.33% fixed / +0.10% rolling; RSS -2.08% / -0.17%; footprint -2.96% / -0.01% | complete |
| 7. Audit and handoff | `ed605692b` | 564 | 564 / 0 / 0 / 0 | diagnostic and ordinary 1,305/1,305 | 4,860/4,860 | final gate passes; no RSS warning | joint inception pending |

## Phase 0: upstream route evidence

- Diagnostic trace:
  `/tmp/cppgm-alias-phase0-strict-provenance.*`, with the exact directory in
  `/tmp/cppgm-alias-phase0-strict-provenance.path`.
- Analyzer report:
  `/tmp/cppgm-alias-phase0-strict-provenance-report.json`, SHA-256
  `f26b4ebe924ec7707de90846e961bfe419be1f558ff6b92b6c51bb2578bcbe5e`.
- The 1,181 traces contain 23,970 records. Every alias submission has a known
  producer and upstream route.
- Route actions:

| Route | Attempts | Inserted | Exact duplicate | Rejected | Replaced | Final visible ownership |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Resolved instantiation | 879 | 569 | 289 | 9 | 12 | 513 |
| Direct template argument | 272 | 175 | 95 | 0 | 2 | 169 |
| Template declaration pattern walk | 143 | 1 | 0 | 142 | 0 | 109 |
| Dependent pattern | 32 | 21 | 3 | 8 | 0 | 29 |

The route totals reproduce the fixed 1,326 attempts and all table actions.
Route ownership can overlap after table collision and renderer lineage is
merged, so the surviving and visible columns are not additive. The recursive
declaration walk is the clearest duplicate semantic branch: 142 of 143
submissions are rejected after it repeats alias lookup and instantiation.
Resolved and direct routes also overlap in the source table and renderer,
including 61 shared build-event replacements.

The analyzer unit tests cover route attribution through table rows, renderer
actions, final visible rows, and the explicit `unknown` bucket. Ordinary builds
compile each route scope to an empty object and do not create provenance state.
The ordinary `cppgm++` is 17,107,840 bytes; Mach-O `__TEXT` is 13,049,856
bytes and `__DATA` is 446,464 bytes.

The three-run candidate is `/tmp/cppgm-alias-phase-0.json`, SHA-256
`b5d9a3581a7645429172a648b8db788633dfd3327b559b5fe12d0294b74afdfe`.
Its medians are 175,484,611,677 instructions, 764,981,248 bytes maximum RSS,
and 592,363,520 bytes peak footprint. Both fixed and initial rolling
comparisons pass without a warning. This exact candidate is the new rolling
baseline.

## Phase 1: one completed alias result

`instantiate_resolved_alias_template_impl` now returns one stack-scoped
`ResolvedAliasTemplateId`. Builtin transform, structural, direct-syntax, AST,
cache-hit, dependent, and fallback arms only complete that result. The nine
arm-local `note_alias_use` calls and their lambda are gone. One completion
function owns observation, trace output, and the existing structured-value
closure before either public alias-instantiation entry point returns its
`TypePtr`.

The result remains 80 bytes and non-owning. `Type`, `TemplateArgument`,
`AliasTemplateDecl`, and the other hot structures retain their Phase 0 sizes.
The production diff for the phase is 247 additions and 230 deletions, a net
addition of 17 lines while the operation boundary is introduced.

Diagnostic provenance is
`/tmp/cppgm-alias-phase1-strict-provenance-report.json`. It is byte-identical
to the Phase 0 report, with SHA-256
`f26b4ebe924ec7707de90846e961bfe419be1f558ff6b92b6c51bb2578bcbe5e`.
All route, table, renderer, and visible-ownership counts match field for field;
there are no unknown routes or producers. Both the ordinary and diagnostic
direct-LowIR strict gates pass 1,305/1,305.

The three-run candidate is `/tmp/cppgm-alias-phase-1.json`, SHA-256
`fd5819ebc64b4042218b7fea925a09dbaa62cd1126f6da869a276cfe983ef4ac`.
Its medians are 175,593,460,455 instructions, 762,298,368 bytes maximum RSS,
and 592,236,544 bytes peak footprint. Both advisory comparisons pass without
a warning, and this exact candidate is the new rolling baseline. The ordinary
binary is 17,107,280 bytes; Mach-O `__TEXT` remains 13,049,856 bytes and
`__DATA` remains 446,464 bytes.

## Phase 2: direct source resolution owns the result

The direct `template_argument_semantics` resolver and its second observation
path are gone. Structured template-id syntax now enters the canonical alias
operation with its argument scope and exact source location. All nine semantic
result arms still finish through the Phase 1 completion boundary. The phase
deletes 1,802 production lines and adds 360, a net deletion of 1,442 lines.
The cumulative production diff from the fixed checkpoint is now 746 additions
and 2,311 deletions, a net deletion of 1,565 lines.

The final diagnostic report is
`/tmp/cppgm-alias-phase2-final-provenance-report.json`, SHA-256
`ad2418e5e65aa24a8dcde3247bcffcaccaa2228949916a26edf3851cd36cb618`.
It covers 1,181 traces and 23,780 records. The direct-template-argument route
has zero attempts. The canonical resolved-instantiation route has 1,230
attempts: 632 insertions, 565 exact duplicates, 19 rejections, and 14
replacements. The dependent-pattern route has 41 attempts and the recursive
declaration route has 145; all 145 recursive submissions are rejected. These
remaining actions are obligations for Phases 3 through 6, not acceptable
final deduplication.

The first performance batch exposed a lifetime error in the consolidated
implementation. It retained 51,807 alias-instantiation scopes, raised the
retained-memory census from 309,553,473 to 321,996,161 bytes, and produced a
2.47% fixed-baseline footprint increase. That artifact is preserved at
`/tmp/cppgm-alias-phase-2-retained-scope-warning.json`, SHA-256
`2e6c3a2c0f6f2f5724c607766d8aa69ca066ce77fa620c0f474e51d7f870af13`.

Alias syntax resolution now uses an operation-local binding scope. The final
census has 36,868 scopes and 300,155,749 retained semantic bytes, below the
Phase 1 counts of 43,329 scopes and 309,553,473 bytes. No hot structure grows:
`Type` remains 280 bytes, `TemplateArgument` 136, `ClassInfo` 1,136,
`AliasTemplateDecl` 264, and `ResolvedAliasTemplateId` 80.

The replacement three-run candidate is `/tmp/cppgm-alias-phase-2.json`,
SHA-256
`71652cb617c4f5919515f80da69c971a6feee7358a4c9d0ef37a9e69b883ffde`.
Its medians are 175,141,188,595 instructions, 744,357,888 bytes maximum RSS,
and 575,315,968 bytes peak footprint. That is -0.43%, -2.55%, and -2.94%
against the fixed baseline, and -0.26%, -2.35%, and -2.86% against rolling.
Both comparisons pass without a warning. The same candidate becomes the new
rolling baseline.

## Phase 3: dependent patterns complete as typed results

Dependent alias resolution no longer calls the observer from a catch or an
incomplete-resolution arm. It constructs parameterized `TemplateArgument`
values from the exact source syntax and finishes through the same completion
boundary as successful resolution. An anchored source definition may retain
its pattern when argument substitution raises; an unanchored speculative
replay cannot publish.

The diagnostic report is
`/tmp/cppgm-alias-phase3-provenance-report.json`, SHA-256
`37074a59bb25a4869f936e68577e9f26a7cfcd9d63d66dbf216e692ccef36f1c`.
The dependent route still has 41 source-pattern completions, but its rejected
actions fall from 12 to zero. It records 27 insertions and 14 exact repeated
completions. Cross-route replacements fall from 14 to 2. The repeated
completed occurrences remain a Phase 6 ownership obligation.

The three-run candidate is `/tmp/cppgm-alias-phase-3.json`, SHA-256
`03b6993d55acf1016c49b16c24dca9ef6ad2d7422adfcb1953f44e34cf6673b0`.
Its medians are 175,678,585,277 instructions, 752,881,664 bytes maximum RSS,
and 575,414,272 bytes peak footprint. Both comparisons pass without a warning,
and this candidate becomes the rolling baseline.

## Phase 4: declaration patterns stop expanding alias targets

Template declaration analysis now selects the alias and resolves only its
source arguments. It publishes that typed parameterized result directly. The
old branch no longer instantiates the alias target, materializes classes,
reconstructs an exact lookup guard, or submits a second pattern row. Stable
source occurrences are cached by declaration syntax and location; later
template instantiations skip both lookup and argument resolution. Patterns
with nested template arguments remain eligible for a more precise binding
frame.

The diagnostic report is
`/tmp/cppgm-alias-phase4-cache-provenance-report.json`, SHA-256
`f242e8c103e624ae1409833725ba8dd28a27c52ecf08d9abc4368fc3fe7b9de2`.
Total alias attempts fall from 1,416 to 1,211. The declaration-pattern route
has 536 attempts: 456 insertions, 70 exact repeats, and 10 rejections. The
resolved-instantiation route falls from 1,230 to 664 attempts. The remaining
pattern retries are tracked for occurrence ownership rather than treated as a
completed zero-duplication result.

The three-run candidate is `/tmp/cppgm-alias-phase-4.json`, SHA-256
`47d8532e47feda7b86a010f67e0e0072e0a5647e0b91cc7255d497d8dc848e3b`.
Its medians are 175,199,806,939 instructions, 747,466,752 bytes maximum RSS,
and 575,352,832 bytes peak footprint. Both comparisons pass without a warning,
and this candidate becomes the rolling baseline.

### Phase 4 boundary amendment

Deleting `analyze_template_declaration_source_patterns` did not transfer its
rows to normal semantic analysis. It removed required alias output from 88
strict fixtures. Template headers, bases, defaults, declaration types, and
unevaluated expressions are not all otherwise analyzed when the enclosing
template remains uninstantiated. Making the ordinary completion paths run
during the source-capture pause was also invalid: it created five false member
alias rows, and one concrete replay replaced a source-pattern binding.

The retained traversal is therefore a primary source analyzer, not a replay of
an already-owned semantic result. Its alias branch selects the declaration,
resolves structured source arguments when possible, creates typed dependent
arguments otherwise, and enters `complete_resolved_alias_template_id`. It no
longer instantiates the alias target, reconstructs an exact-lookup guard, calls
the observer directly, or uses a separate producer route. The old
`template_declaration_pattern` route is gone. Later template instantiations may
complete the same occurrence again for compilation, but they cannot publish a
second source-table row.

## Phase 5: structured source facts replace recovery

Commit `7b2ecdadf7ea57aafeb3e26000440b7f5bd06140` completes the location and
owner cleanup:

- Alias occurrence identity is the selected alias identity plus the stable
  source-location ID. Syntax identity and normalized location are fallbacks
  only when the stable ID is unavailable.
- The same-line pending-class scan used to recover a member alias owner is
  deleted. Qualified owners come from `TemplateIdSyntax` and the selected
  alias declaration.
- Alias completion and source-pattern analysis use structured dependency
  checks. The `template_argument_texts_mention_*` checks that selected among
  competing alias rows are gone; helpers with those names remain only where
  class semantics still use them.
- `qualify_member_alias_events_from_class_uses`,
  `canonicalize_placeholder_member_alias_owners`,
  `normalize_selected_decl_to_line_start`, `unwrap_single_pack_binding`, and
  `use_template_argument_binding_policy` are absent.
- The walker creates typed parameterized `TemplateArgument` values and enters
  the canonical completion boundary. There is one static
  `observe_resolved_alias_template_id` call and one direct `emit_alias_use`
  call outside `witness_api.cpp`.

The first final performance batch at this commit exposed structured dependency
checks on every alias completion. They are needed only for member aliases. The
corrected commit moves those checks behind that condition and removes the
avoidable work. The retained result is 72 bytes, down from 80 bytes.

## Phase 6: one alias publication per occurrence

The alias source table now consumes the canonical occurrence vector directly.
Alias rows bypass generic source-table deduplication, so the final provenance
gate would expose any repeated publication. The alias replacement map in
`WitnessBuilder`, alias preference and collapse passes, renderer owner repair,
`AliasUseEmissionOrigin`, capture-pause alias policy, old route IDs, and the
old producer IDs are deleted.

Commit `ed605692bb9b55cbad9f510ac1a85fd3352ed574` narrows the last
prepublication policy. A first-writer experiment changed only four strict
comparisons. In those cases the source pattern preceded its class context and
therefore spelled a current specialization as `function` or `Iter`; the later
typed class context supplied `function<R (Args...)>` or `Iter<B>`. The final
collector no longer ranks parameterized, concrete, context-rich, or
structured-owner payloads. It keeps the first result and permits only two
materialization transitions:

1. attach a class context to a retained parameterized source pattern;
2. fill current-specialization fields when the concrete class replay supplies
   them.

This is completion of one retained occurrence, not selection between two
public rows. Removing the generic ranking also deletes the unused
`has_structured_owner` state. From the end of Phase 4 through this commit, the
production `dev/` diff is 228 additions and 376 deletions, a net deletion of
148 lines.

### Final provenance

- Trace: `/tmp/cppgm-alias-final-provenance3.1uzSbj`.
- Report: `/tmp/cppgm-alias-final-provenance3-report.json`.
- Report SHA-256:
  `d05a15c2442a888286ac650ccb1b06f112035eaf29180a752ccb58476c1060ce`.
- Corpus: 1,305 trace files and 22,476 records.

The alias route has 564 attempts, 564 insertions, 564 surviving table rows,
and 564 final visible rows. Exact duplicates, rejections, replacements,
enrichments, unknown producers, and unknown routes are all zero. Renderer work
on aliases is formatting only: 267 binding rewrites and six name rewrites. No
renderer pass removes or replaces an alias row.

The semantic consolidation counters are earlier than publication. They record
1,332 completed alias candidates, 768 repeated completions of an existing
source occurrence, 564 collected occurrences, and 564 publications. These
repeats include ordinary alias resolution required by separate template
instantiations and the source-pattern/concrete-materialization transition.
They do not perform another alias-target instantiation for witness capture and
do not reach the source table. The only payload update allowed by the collector
is the named class/current-specialization materialization above.

## Phase 7: final audit and handoff

Ordinary direct-LowIR strict validation passes 1,305/1,305. The diagnostic
provenance build passes the same 1,305/1,305 corpus. The full PA1-PA38 report
passes 4,860/4,860 with direct LowIR comparison. The provenance analyzer unit
tests pass three tests.

The cumulative production diff from the fixed checkpoint is 2,125 additions
and 5,033 deletions under `dev/`, a net deletion of 2,908 lines. Under
`dev/src/` it is 2,123 additions and 5,033 deletions, a net deletion of 2,910
lines. The analyzer changes under `scripts/` are a net addition of 197 lines
and are not production compiler code.

The final ordinary binary is 16,989,176 bytes. Mach-O `__TEXT` is 12,947,456
bytes, `__DATA_CONST` is 57,344 bytes, and `__DATA` is 442,368 bytes. Relative
to the fixed binary, the file is 118,664 bytes smaller, `__TEXT` is 102,400
bytes smaller, and `__DATA` is 4,096 bytes smaller. The ordinary binary has no
`witness_provenance` symbols.

Final hot structure sizes are:

| Type | Bytes |
| --- | ---: |
| `Type` | 280 |
| `TemplateArgument` | 136 |
| `ClassInfo` | 1,136 |
| `FunctionBinding` | 824 |
| `ValueBinding` | 504 |
| `AliasTemplateDecl` | 264 |
| `TemplateLifecycleTransition` | 80 |
| `OutOfClassStaticMemberDecl` | 136 |
| `OutOfClassMemberFunctionDecl` | 240 |
| `ResolvedClassTemplateIdView` | 104 |
| `ResolvedAliasTemplateId` | 72 |
| `ResolvedQualifiedId` | 40 |
| `ResolvedOwnerReference` | 32 |
| `RetainedAliasClassUse` | 16 |

No hot semantic structure grew from the fixed checkpoint. Direct alias results
remain operation-local. The publication map retains one request per canonical
source occurrence only while witness capture is active; ordinary compilation
does not allocate it without a witness session.

### Final performance

The first exact-checkpoint batch is
`/tmp/cppgm-alias-final-materialization.json`, SHA-256
`bcf19725b6939ef28c4f01ca4632d8d68d712b6b33e6ae5b19a6c848235d6a96`.
Its medians are 175,254,366,045 instructions, 743,993,344 bytes maximum RSS,
and 575,062,016 bytes peak footprint. Against the recreated fixed baseline,
those are -0.43%, -1.73%, and -3.03%. Against the rolling class-materialization
checkpoint, they are -0.00%, +0.18%, and -0.08%.

Because the instruction reduction is below 0.5%, the required independent
confirmation is `/tmp/cppgm-alias-final-materialization-confirmation.json`,
SHA-256
`f3987022a055666467c99304a514129e4230bde6d1da3025cc3c86d2350eacb7`.
Its medians are 175,436,303,700 instructions, 741,371,904 bytes maximum RSS,
and 575,488,000 bytes peak footprint. Against fixed, those are -0.33%, -2.08%,
and -2.96%; against rolling they are +0.10%, -0.17%, and -0.01%. Both
instruction medians are below fixed. The confirmation has the less favorable
instruction result and is the reported final value. Neither batch triggers the
3% RSS warning.

### Non-alias handoff

Class publication now has 1,953 attempts, insertions, table rows, and visible
rows, with no table or renderer arbitration. Its semantic collector records
2,209 completed candidates, 2,190 early repeats, 250 prepublication merges,
1,959 collected occurrences, and 1,953 publications. The six nonpublished
occurrences are owned by the class-materialization policy and its separate
plan.

Function calls remain the global deduplication owner. The strict corpus has
920 function attempts, 787 insertions, 133 exact source-table duplicates, and
599 visible rows. The renderer removes 177 source-defined calls, nine rows in
location canonicalization, one template-header pattern, and one final visible
duplicate. A function-call convergence phase must add upstream route identity
before deleting a feed, then move those visibility decisions to the typed call
result. Variable uses are already 31 attempts, 31 insertions, and 31 visible
rows.

Shared table and renderer deduplication therefore remain for function calls,
not aliases. `--witness-debug` also remains: its output is not equal to
`--witness`, so alias convergence does not justify removing that interface.

### Inception state

No final inception comparison has run for this checkpoint. An early invocation
exited during the prerequisite frontend build, before self-host compilation,
because the reduced `nsdecl` link omitted `witness_text` while `callsemantic`
referenced anonymous-namespace normalization. Commit
`5677fb2175ca50e13ab3f2a9684e65aaaf092663` adds `witness_text` to the
`nsdecl` and `nsinit` reduced-link source sets. Targeted links and the ordinary
root build pass after that correction; the measured `cppgm++` source set was
already correct.

Per user direction, inception remains pending until this alias plan and
`docs/witness-class-materialization-semantic-ownership-plan.md` have both
completed. The joint runner must use a fresh isolated object root. This is a
deferred joint gate, not a failed self-host result.
