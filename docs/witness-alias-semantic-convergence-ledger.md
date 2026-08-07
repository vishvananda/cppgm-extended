# Witness alias semantic convergence ledger

This ledger records the implementation evidence for
`docs/witness-alias-semantic-convergence-plan.md`. The fixed comparison point
is lifecycle Phase 6 at `05b0c7a21ff497cd2186fabc2096bf04cc6e931b`.

## Fixed evidence

- Fixed performance artifact:
  `/tmp/cppgm-alias-convergence-fixed.json`, copied byte-for-byte from
  `/tmp/cppgm-lifecycle-phase-6-final.json`.
- Fixed and initial rolling SHA-256:
  `f3321ba42cf500112b8d183a903a73cb92a5a58604ff6ff986043c9d4cf012ca`.
- Fixed medians: 175,889,730,826 instructions, 763,817,984 bytes maximum RSS,
  and 592,760,832 bytes peak footprint.
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

## Checkpoints

| Phase | Commit | Alias attempts | Insert / duplicate / reject / replace | Strict | Broad | Performance | State |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| 0. Route evidence | `e99c510d3` | 1,326 | 766 / 387 / 159 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.23%, RSS +0.15%, footprint -0.07% | complete |
| 1. Completed result | `497376554` | 1,326 | 766 / 387 / 159 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.17% fixed / +0.06% rolling; RSS -0.20% / -0.35%; footprint -0.09% / -0.02% | complete |
| 2. Direct source owner | `f5529cc60` | 1,416 | 653 / 573 / 176 / 14 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.43% fixed / -0.26% rolling; RSS -2.55% / -2.35%; footprint -2.94% / -2.86% | complete |
| 3. Dependent pattern result | `bd3b81402` | 1,416 | 671 / 579 / 164 / 2 | diagnostic and ordinary 1,305/1,305 | not required | instructions -0.12% fixed / +0.31% rolling; RSS -1.43% / +1.15%; footprint -2.93% / +0.02% | complete |

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
