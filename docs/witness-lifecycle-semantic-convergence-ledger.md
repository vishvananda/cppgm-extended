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
| `lifecycle.template_api.02` | 8,023 | 7,317 | 2 | 704 | removed in Phase 1 |
| `lifecycle.template_api.03` | 52 | 5 | 0 | 47 | pending |
| `lifecycle.template_api.04` | 2 | 0 | 0 | 2 | pending |
| `lifecycle.template_api.05` | 1 | 0 | 0 | 1 | pending |
| `lifecycle.template_api.06` | 0 | 0 | 0 | 0 | Clang oracle reducer added; current CPPGM event missing |
| `lifecycle.template_api.07` | 1,449 | 963 | 0 | 486 | pending |
| `lifecycle.template_api.09` | 51 | 4 | 0 | 47 | removed in Phase 1 |
| `lifecycle.callsemantic.01` | 7 | 0 | 0 | 7 | pending |
| `lifecycle.callsemantic.02` | 8 | 0 | 0 | 8 | removed in Phase 2 |
| `lifecycle.constant_value_lookup.02` | 111 | 78 | 0 | 33 | removed in Phase 1A |

## Phase results

| Phase | Commit | Strict | Report | Inception | Instructions vs fixed | RSS vs fixed | Footprint vs fixed | Production net lines | Status |
| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| 0, evidence and route map | `c2b81588d`, `bbf774d43` | inherited 1305/1305 | inherited 4860/4860 | parent passed | n/a | n/a | n/a | 0 | complete |
| 1, value transitions | `93961d96a`, `61ce47d28` | 1305/1305 | 4860/4860 | n/a | +0.00% | +0.45% | -0.04% | +312 | complete |
| 2, function acquisition | `644b561a2` | 1305/1305 | 4860/4860 | n/a | -0.17% | -0.09% | -0.01% | +390 | complete |
| 3, definition closure | pending | pending | pending | pending if layout changes | pending | pending | pending | pending | pending |
| 4, class acquisition | pending | pending | pending | pending if layout changes | pending | pending | pending | pending | pending |
| 5, nested and unnamed classes | pending | pending | pending | n/a | pending | pending | pending | pending | pending |
| 6, final observer and cleanup | pending | pending | pending | pending | pending | pending | pending | pending | pending |

## Hot structure sizes

| Structure | Fixed size | Phase 1 | Phase 2 | Phase 3 | Phase 4 | Phase 5 | Final |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `Type` | 280 | 280 | 280 | pending | pending | pending | pending |
| `TemplateArgument` | 136 | 136 | 136 | pending | pending | pending | pending |
| `ClassInfo` | 1136 | 1136 | 1136 | pending | pending | pending | pending |
| `FunctionBinding` | 824 | 824 | 824 | pending | pending | pending | pending |
| `ValueBinding` | 504 | 504 | 504 | pending | pending | pending | pending |
| `TemplateLifecycleTransition` | n/a | 72 | 88 | pending | pending | pending | pending |

## Route map

### Value lifecycle

- `.02` observes member-value materialization and retained dependency replay.
- `.09` observes variable-template acquisition.
- `constant_value_lookup.02` reconstructs
  `integral_constant<bool, value>::value` before normal specialization
  selection.
- Starting waste: 8,185 attempts produce 784 inserted events, with 7,399
  exact duplicates and two enrichments.

## Phase 1A evidence: remove the pre-selection constant-value event

- `note_integral_constant_bool_value_for_witness` used to construct and insert
  an `integral_constant<...>::value` event before it selected the class
  specialization. The same function then instantiated the selected class,
  looked up its `value` binding, and called the normal member-value lifecycle
  operation.
- Phase 1A deletes the first event construction and its rendered entity and
  declaration-anchor work. The selected `ValueBinding` path remains the only
  semantic source for this event.
- The retired producer made 111 strict attempts: 33 insertions and 78 exact
  duplicates. Its producer enum, name mapping, and analyzer inventory entry
  are deleted.
- All 24 strict files that exercised the producer retain byte-identical witness
  output. Targeted artifacts live at
  `/tmp/cppgm-lifecycle-value-targeted.koUfZ4`.
- Targeted provenance trace:
  `/tmp/cppgm-lifecycle-value-provenance.1Hmxv8`.
- Targeted provenance report:
  `/tmp/cppgm-lifecycle-value-provenance-report.json` with SHA-256
  `ff0ac159df1d0621d10a1e95afb5c655d31f29280e54f6cc3a8568d6556b9acf`.
- The report contains 3,681 records and no unknown producer. The retired site
  is absent. `lifecycle.template_api.02` owns the value events from this
  targeted set: 1,997 attempts, 1,909 exact duplicates, and 88 inserted
  events.
- Direct-LowIR strict passes 1,305/1,305. The PA1-PA38 report passes
  4,860/4,860 after all frontends were built in
  `/tmp/cppgm-witness-lifecycle-obj`.
- Production code is cumulatively `+0 / -37` from the fixed checkpoint. Phase
  1 remains open because `.02` and `.09` still use separate lifecycle paths
  and `.02` still generates repeated attempts.

## Phase 1 evidence: canonical value transitions

- Correctness commit: `61ce47d287574d19e21ee0b0b7e2b90c9f9d3114`.
- `materialize_template_member_value_transition` returns one typed value
  transition. `observe_template_lifecycle_transition` owns the only value
  lifecycle recorder call. Variable-template acquisition submits the same
  transition type.
- Retained value dependencies carry the selected binding, semantic owner, and
  visible owner-argument count. The transition carries no rendered entity or
  location strings. The owner-argument view preserves distinct requests such
  as `table<>::sizes` and `table<void>::sizes` without duplicate recorder
  attempts.
- Targeted provenance trace:
  `/tmp/cppgm-lifecycle-phase1-targeted-provenance.KgxeJb`.
- Targeted report:
  `/tmp/cppgm-lifecycle-phase1-targeted-provenance-report.json`, SHA-256
  `637a04d947027eb0cb0ffb6c9750bbb006f0cfdd2ea9836cb3a7721427c004ba`.
  The 24 files produce 1,706 records. The transition observer makes 55
  attempts and inserts all 55; it records no duplicate, enrichment, rejection,
  or replacement action.
- Full strict provenance trace:
  `/tmp/cppgm-lifecycle-phase1-strict-provenance.xVq8G3`.
- Full strict report:
  `/tmp/cppgm-lifecycle-phase1-strict-provenance-report.json`, SHA-256
  `effb9e4cc75d6546d7456b411303e5e4bc9cd994fd8f7be3ceb83ed441caf914`.
  The 1,180 traces contain 51,583 records and no unknown producer attempts.
  The observer inserts all 570 attempts. The report contains no `.02`, `.09`,
  or `constant_value_lookup.02` site.
- The diagnostic and ordinary direct-LowIR strict runs pass 1,305/1,305. The
  ordinary PA1-PA38 report passes 4,860/4,860.
- Candidate performance file: `/tmp/cppgm-lifecycle-phase-1.json`, SHA-256
  `8a1c1deb5a5a0b99428bb60239adc072658621f86259abf4a477a955737d6187`.
  Median instructions are `176160343936`, `+0.00%` versus fixed. Median
  maximum RSS is `766439424`, `+0.45%`. Median peak footprint is `592625664`,
  `-0.04%`. Both fixed and rolling advisory comparisons pass without an RSS
  warning. The rolling file contains this candidate.
- `Type`, `TemplateArgument`, `ClassInfo`, `FunctionBinding`, and
  `ValueBinding` retain their fixed sizes. `TemplateLifecycleTransition` is 72
  bytes. Phase 1 adds 481 and removes 169 `dev/src` lines, a net addition of
  312 lines. Phases 2-6 must offset this intermediate growth by deleting the
  remaining producer arms and legacy wrappers before the final line and
  instruction gate.

## Phase 2 evidence: function acquisition transitions

- Correctness commit: `644b561a216376bd8e061a58d3b1865eab73537a`.
- `acquire_function_instantiation` and `acquire_function_binding` create
  typed function-instantiation transitions. The transition observer derives
  the witness entity, declaration anchor, source location, cause, and detail
  from the selected `FunctionBinding` and acquisition request.
- Explicit function-instantiation analysis sends the selected binding and
  source node through `acquire_function_binding`. The phase deletes the direct
  `lifecycle.callsemantic.02` emitter and its rendered entity and location
  recovery. Definition-closure declaration acquisition uses the same result.
- Full strict provenance trace:
  `/tmp/cppgm-lifecycle-phase2-strict-provenance.NG9K10`.
- Full strict report:
  `/tmp/cppgm-lifecycle-phase2-strict-provenance-report.json`, SHA-256
  `ecc0243d19ae8930bf93689818dcb847239f3b4f09c05e49acadb4af9227e5f6`.
  The 1,180 traces contain 50,923 records and no unknown producer attempts.
  The transition observer makes 799 attempts: 786 insertions and 13 exact
  duplicates, with no enrichment, rejection, or replacement. The retired
  `lifecycle.callsemantic.02` site is absent.
- The 13 observer duplicates involve acquisition and definition-materialization
  paths that refer to cloned or mutable function bindings. Phase 3 will replace
  those paths with one definition state machine and one semantic transition
  identity. A function-identity-only trial increased observer duplicates from
  13 to 69, so this phase does not keep that temporary scheme.
- The diagnostic and ordinary direct-LowIR strict runs pass 1,305/1,305. The
  ordinary PA1-PA38 report passes 4,860/4,860.
- Candidate performance file: `/tmp/cppgm-lifecycle-phase-2.json`, SHA-256
  `017ff03052306b14883960ace3537cc45bad95fa2ee664e20dc10104c13c3933`.
  Median instructions are `175848424332`, `-0.17%` versus fixed and `-0.18%`
  versus Phase 1. Median maximum RSS is `762363904`, `-0.09%` versus fixed.
  Median peak footprint is `592830464`, `-0.01%` versus fixed. Both advisory
  comparisons pass without an RSS warning. The rolling file contains this
  candidate.
- `Type`, `TemplateArgument`, `ClassInfo`, `FunctionBinding`, and
  `ValueBinding` retain their fixed sizes. `TemplateLifecycleTransition` is 88
  bytes. The ordinary `cppgm++` is 17,141,080 bytes; Mach-O `__TEXT` is
  13,074,432 bytes and `__DATA` is 446,464 bytes.
- Production code has 668 additions and 278 deletions from the
  fixed checkpoint, a net addition of 390 lines. Phase 2 adds transition facts
  while the Phase 3 definition wrappers remain. Phase 3 owns their deletion.

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
- Patched Clang emits one class-instantiation event for
  `validation/templates/witness-lifecycle-anonymous-member.cpp`. Current CPPGM
  emits no anonymous-member lifecycle event and records only `.01` function
  attempts. The validation trace is
  `/tmp/cppgm-lifecycle-anonymous.wLf7xB`. Phase 5 must move the missing event
  into the class-completion transition before the reducer enters PA19.

## Open obligations

- [x] Add a targeted `.06` anonymous-member oracle reducer before deleting its
  route.
- [ ] Promote the anonymous-member reducer to PA19 after the canonical class
  transition matches the Clang reference.
- [x] Record fixed `sizeof(FunctionBinding)` and `sizeof(ValueBinding)`.
- [x] Confirm the original class-use inception run and copy its result into the
  parent ledger.
- [x] Use `/tmp/cppgm-witness-lifecycle-obj` for worktree builds.
- [x] Record the Phase 1 candidate batch and promote it to the rolling file.
- [ ] Record one candidate batch for each remaining semantic phase. Phases 1
  and 2 are complete.
- [ ] Investigate each instruction, RSS, or footprint warning before advancing.
- [ ] Finish with a net production line and instruction reduction.

## Follow-up queue

1. Alias source-use convergence: four feed paths and a recursive template
   pattern walker remain behind one emitter.
2. Function-call source-use convergence: overload, conversion, declval, and
   constexpr-direct feeds remain behind one emitter.
3. Variable source use: retain the current single feed unless new provenance
   shows overlap.
