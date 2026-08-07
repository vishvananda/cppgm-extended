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
| 1. Completed result | pending | 1,326 | 766 / 387 / 159 / 14 | diagnostic and ordinary 1,305/1,305 | not required | pending | implementation complete |

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
