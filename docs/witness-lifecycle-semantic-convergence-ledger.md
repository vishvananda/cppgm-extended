# Witness Lifecycle Semantic Convergence Ledger

Plan: `docs/witness-lifecycle-semantic-convergence-plan.md`

## Fixed checkpoint

- Semantic head: `0662098ba13097ae0ac059593443a311f769a59b`
- Fixed performance file:
  `/tmp/cppgm-lifecycle-convergence-fixed.json`
- Rolling performance file:
  `/tmp/cppgm-lifecycle-convergence-rolling.json`
- Artifact SHA-256:
  `bc6c01464e356e2c4da69f0a63e3baec74e7322f15931030540afaabea566203`
- Workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`
- Median instructions: `176156071669`
- Median maximum RSS: `763035648`
- Median peak footprint: `592871424`
- Starting `dev/src` lines: `416807`

The fixed batch contains three runs from the final class-use endpoint and
postdates the diagnostic counter additions.

## Starting provenance

- Trace: `/tmp/cppgm-class-use-final-provenance.5eQcbQ`
- Report: `/tmp/cppgm-class-use-final-provenance-report.json`
- Report SHA-256:
  `e94d543d32cd24cbf43c36b8386de1a3570fd2842308246d781be68c7013485f`
- Files: 1,180
- Records: 59,412
- Unknown producer attempts: 0

| Metric | Count |
| --- | ---: |
| Lifecycle attempts | 34,383 |
| Inserted events | 10,044 |
| Exact duplicates | 24,072 |
| Enrichments | 267 |
| Static producers | 11 |
| Exercised producers | 10 |

## Starting producer inventory

| Producer | Attempts | Exact duplicate | Enriched | Inserted | Status |
| --- | ---: | ---: | ---: | ---: | --- |
| `lifecycle.template_api.01` | 24,679 | 15,705 | 265 | 8,709 | pending |
| `lifecycle.template_api.02` | 8,023 | 7,317 | 2 | 704 | pending |
| `lifecycle.template_api.03` | 52 | 5 | 0 | 47 | pending |
| `lifecycle.template_api.04` | 2 | 0 | 0 | 2 | pending |
| `lifecycle.template_api.05` | 1 | 0 | 0 | 1 | pending |
| `lifecycle.template_api.06` | 0 | 0 | 0 | 0 | needs targeted fixture |
| `lifecycle.template_api.07` | 1,449 | 963 | 0 | 486 | pending |
| `lifecycle.template_api.09` | 51 | 4 | 0 | 47 | pending |
| `lifecycle.callsemantic.01` | 7 | 0 | 0 | 7 | pending |
| `lifecycle.callsemantic.02` | 8 | 0 | 0 | 8 | pending |
| `lifecycle.constant_value_lookup.02` | 111 | 78 | 0 | 33 | pending |

## Phase results

| Phase | Commit | Strict | Report | Inception | Instructions vs fixed | RSS vs fixed | Footprint vs fixed | Production net lines | Status |
| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| 0, evidence and route map | pending | inherited 1305/1305 | inherited 4860/4860 | parent run pending | n/a | n/a | n/a | 0 | in progress |
| 1, value transitions | pending | pending | pending | n/a | pending | pending | pending | pending | pending |
| 2, function acquisition | pending | pending | pending | n/a | pending | pending | pending | pending | pending |
| 3, definition closure | pending | pending | pending | pending if layout changes | pending | pending | pending | pending | pending |
| 4, class acquisition | pending | pending | pending | pending if layout changes | pending | pending | pending | pending | pending |
| 5, nested and unnamed classes | pending | pending | pending | n/a | pending | pending | pending | pending | pending |
| 6, final observer and cleanup | pending | pending | pending | pending | pending | pending | pending | pending | pending |

## Hot structure sizes

| Structure | Fixed size | Phase 1 | Phase 2 | Phase 3 | Phase 4 | Phase 5 | Final |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `Type` | 280 | pending | pending | pending | pending | pending | pending |
| `TemplateArgument` | 136 | pending | pending | pending | pending | pending | pending |
| `ClassInfo` | 1136 | pending | pending | pending | pending | pending | pending |
| `FunctionBinding` | pending | pending | pending | pending | pending | pending | pending |
| `ValueBinding` | pending | pending | pending | pending | pending | pending | pending |
| `TemplateLifecycleTransition` | n/a | pending | pending | pending | pending | pending | pending |

## Route map

### Value lifecycle

- `.02` observes member-value materialization and retained dependency replay.
- `.09` observes variable-template acquisition.
- `constant_value_lookup.02` reconstructs
  `integral_constant<bool, value>::value` before normal specialization
  selection.
- Starting waste: 8,185 attempts produce 784 inserted events, with 7,399
  exact duplicates and two enrichments.

### Function lifecycle

- `.01` receives require, ensure, and materialize calls from function-template
  acquisition, definition closure, call semantics, and semantic template
  function paths.
- `callsemantic.02` emits explicit function instantiation after rebuilding its
  entity and declaration location.
- Starting waste: 24,687 attempts produce 8,717 inserted events, with 15,705
  exact duplicates and 265 enrichments.

### Class lifecycle

- `.07` observes generic class acquisition and finalization.
- `callsemantic.01` emits explicit class finalization.
- `.03` observes nested member completion.
- `.04`, `.05`, and `.06` walk unnamed and anonymous class relationships.
- Starting waste: 1,511 attempts produce 543 inserted events, with 968 exact
  duplicates.

## Open obligations

- [ ] Add a targeted `.06` anonymous-member fixture before deleting its route.
- [ ] Record fixed `sizeof(FunctionBinding)` and `sizeof(ValueBinding)`.
- [ ] Confirm the original class-use inception run and copy its result into the
  parent ledger.
- [ ] Use `/tmp/cppgm-witness-lifecycle-obj` for worktree builds.
- [ ] Record one candidate batch for each committed semantic phase.
- [ ] Investigate each instruction, RSS, or footprint warning before advancing.
- [ ] Finish with a net production line and instruction reduction.

## Follow-up queue

1. Alias source-use convergence: four feed paths and a recursive template
   pattern walker remain behind one emitter.
2. Function-call source-use convergence: overload, conversion, declval, and
   constexpr-direct feeds remain behind one emitter.
3. Variable source use: retain the current single feed unless new provenance
   shows overlap.
