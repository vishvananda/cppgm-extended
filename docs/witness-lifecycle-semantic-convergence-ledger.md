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
| `lifecycle.template_api.01` | 24,679 | 15,705 | 265 | 8,709 | removed in Phase 3 |
| `lifecycle.template_api.02` | 8,023 | 7,317 | 2 | 704 | removed in Phase 1 |
| `lifecycle.template_api.03` | 52 | 5 | 0 | 47 | removed in Phase 5 |
| `lifecycle.template_api.04` | 2 | 0 | 0 | 2 | removed in Phase 5 |
| `lifecycle.template_api.05` | 1 | 0 | 0 | 1 | removed in Phase 5 |
| `lifecycle.template_api.06` | 0 | 0 | 0 | 0 | removed in Phase 5; Clang reducer moved to canonical observer |
| `lifecycle.template_api.07` | 1,449 | 963 | 0 | 486 | removed in Phase 4 |
| `lifecycle.template_api.09` | 51 | 4 | 0 | 47 | removed in Phase 1 |
| `lifecycle.callsemantic.01` | 7 | 0 | 0 | 7 | removed in Phase 4 |
| `lifecycle.callsemantic.02` | 8 | 0 | 0 | 8 | removed in Phase 2 |
| `lifecycle.constant_value_lookup.02` | 111 | 78 | 0 | 33 | removed in Phase 1A |

## Phase results

| Phase | Commit | Strict | Report | Inception | Instructions vs fixed | RSS vs fixed | Footprint vs fixed | Production net lines | Status |
| --- | --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| 0, evidence and route map | `c2b81588d`, `bbf774d43` | inherited 1305/1305 | inherited 4860/4860 | parent passed | n/a | n/a | n/a | 0 | complete |
| 1, value transitions | `93961d96a`, `61ce47d28` | 1305/1305 | 4860/4860 | n/a | +0.00% | +0.45% | -0.04% | +312 | complete |
| 2, function acquisition | `644b561a2` | 1305/1305 | 4860/4860 | n/a | -0.17% | -0.09% | -0.01% | +390 | complete |
| 3, definition closure | `274cf0a32` | 1305/1305 | 4860/4860 | n/a, hot layouts unchanged | -0.11% | +1.02% | -0.02% | +518 | complete |
| 4, class acquisition | `164b9addb` | 1305/1305 | 4860/4860 | n/a, hot layouts unchanged | -0.19% | +0.71% | -0.06% | +599 | complete |
| 5, nested and unnamed classes | `313662d30` | 1305/1305 | 4860/4860 | n/a, hot layouts unchanged | -0.14% | +0.76% | -0.06% | +540 | complete |
| 6, final observer and cleanup | pending | pending | pending | pending | pending | pending | pending | pending | pending |

## Hot structure sizes

| Structure | Fixed size | Phase 1 | Phase 2 | Phase 3 | Phase 4 | Phase 5 | Final |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `Type` | 280 | 280 | 280 | 280 | 280 | 280 | pending |
| `TemplateArgument` | 136 | 136 | 136 | 136 | 136 | 136 | pending |
| `ClassInfo` | 1136 | 1136 | 1136 | 1136 | 1136 | 1136 | pending |
| `FunctionBinding` | 824 | 824 | 824 | 824 | 824 | 824 | pending |
| `ValueBinding` | 504 | 504 | 504 | 504 | 504 | 504 | pending |
| `TemplateLifecycleTransition` | n/a | 72 | 88 | 88 | 96 | 96 | pending |

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

## Phase 3 evidence: canonical definition closure

- Correctness commit: `274cf0a3280ab427b6663924b702f725b66a915f`.
- `acquire_function_binding_in_current_context` now owns the required,
  ensured, and materialized state edges around its one call to
  `ensure_function_template_definition`. Definition closure, call semantics,
  constant evaluation, conversion, lifetime, and explicit instantiation all
  submit typed acquisition requests to that operation.
- The phase deletes `lifecycle.template_api.01`, the generic closure-event
  wrapper, the direct definition-ensure and definition-materialized helpers,
  and the remaining callsemantic require-definition arms. Lifecycle producer
  IDs fall from eight to seven. The surviving observer is the only function
  lifecycle recorder caller.
- Function transition identity is session-side and contains semantic and
  presentation boundary facts: canonical overload-set, declaring source,
  owner instantiation, event anchor, cause, and flags. It is 40 bytes and
  retains no owning string or hot semantic-object field. Equivalent cloned
  bindings therefore share state without collapsing distinct source events.
- Full strict provenance trace:
  `/tmp/cppgm-lifecycle-phase3-strict-provenance.B6n4wj`.
- Full strict report:
  `/tmp/cppgm-lifecycle-phase3-strict-provenance-report.json`, SHA-256
  `1056c92091bba2f72574ed9e338d293d06505eaaa9d0cb234f21bcf257b59a7e`.
  The 1,180 traces contain 25,009 records and no unknown producer attempt.
  `lifecycle.transition_observer.01` makes 3,999 attempts and inserts all
  3,999. It records no exact duplicate, enrichment, rejection, or
  replacement. The lifecycle collision matrix is empty, and the deleted
  `.01` producer is absent.
- The diagnostic strict run used one batch worker and passed 1,305/1,305 with
  direct LowIR comparison. The ordinary eight-worker strict run also passed
  1,305/1,305, and the ordinary PA1-PA38 report passed 4,860/4,860.
- Candidate performance file: `/tmp/cppgm-lifecycle-phase-3.json`, SHA-256
  `ec446f1870ccb5f86bf719334ea624a897a63d48979fe2812055d97274b6f153`.
  Median instructions are `175962780696`, `-0.11%` versus fixed and `+0.07%`
  versus Phase 2. Median maximum RSS is `770822144`, `+1.02%` versus fixed and
  `+1.11%` versus Phase 2. Median peak footprint is `592760832`, `-0.02%`
  versus fixed and `-0.01%` versus Phase 2. Both comparisons pass. RSS does
  not trigger a confirmation batch. The rolling file contains this exact
  candidate.
- `Type`, `TemplateArgument`, `ClassInfo`, `FunctionBinding`, and
  `ValueBinding` retain their fixed sizes. `TemplateLifecycleTransition`
  remains 88 bytes. The ordinary `cppgm++` is 17,140,728 bytes; Mach-O
  `__TEXT` is 13,069,554 bytes and `__DATA` is 442,600 bytes.
- Phase 3 adds 401 and removes 273 production lines, a net addition of 128.
  Cumulative production change from the fixed checkpoint is 1,044 additions
  and 526 deletions, or +518 lines. The final phase still owes a net deletion;
  class convergence and recorder cleanup must remove the superseded wrappers,
  flags, scans, and identity branches.

## Phase 4 evidence: canonical class acquisition and finalization

- Correctness commit: `164b9addbcebb46b7b8fbedd87d2ac6daef032ac`.
- Generic and selected class acquisition now create typed class-instantiation
  transitions. Class finalization creates the corresponding typed transition,
  and explicit instantiation declarations and definitions submit their
  resolved `ClassTemplateDecl`, source node, and cause through the same
  operation. Call semantics no longer rebuilds the entity or either source
  location.
- The phase deletes `lifecycle.template_api.07`,
  `lifecycle.callsemantic.01`, `note_class_closure_event`, and the explicit
  callsemantic emitter. Lifecycle producer IDs fall from seven to five. The
  remaining four legacy IDs are the nested and unnamed paths assigned to
  Phase 5; the shared observer owns every function, value, and generic class
  transition.
- Class transition identity uses the selected declaration anchor, canonical
  owner-qualified class identity, semantic instantiation key, request anchor,
  transition kind, and cause. It does not treat a retry's `created-new` value
  or nested closure trigger as a second state change. Cloned `ClassInfo`
  objects for the same specialization therefore converge, while the same
  nested declaration under different owner instantiations stays distinct.
- Final strict provenance trace:
  `/tmp/cppgm-lifecycle-phase4-final3-provenance.d5mNwz`.
- Final strict report:
  `/tmp/cppgm-lifecycle-phase4-final3-provenance-report.json`, SHA-256
  `1e3d33a99ea7ed6f3163e31fdeb9d0fa7fd2a64fd5d6f9f95b60836975ec100a`.
  The 1,180 traces contain 23,950 records and no unknown producer attempt.
  `lifecycle.transition_observer.01` makes 4,430 attempts and inserts all
  4,430. It records no exact duplicate, enrichment, rejection, or
  replacement. The lifecycle collision matrix is empty, and both retired
  producer IDs are absent.
- The old generic class path made 1,435 strict attempts: 425 class
  instantiations, 47 finalizations, and 963 exact duplicates. The direct
  explicit path added seven more events. The canonical observer retains 431
  additional class transitions over the Phase 3 count. It removes all 963
  duplicate recorder calls and 48 pre-render rows that represented the same
  class, source anchor, kind, and cause under retry or nested closure context.
- Removing the two generic `template_instantiation_log_emitted` checks exposes
  13 distinct nested transitions to the observer. The field remains only in
  the nested and unnamed machinery; Phase 5 owns its final removal together
  with producers `.03` through `.06`.
- The diagnostic and ordinary direct-LowIR strict runs pass 1,305/1,305. The
  ordinary PA1-PA38 report passes 4,860/4,860.
- Candidate performance file: `/tmp/cppgm-lifecycle-phase-4.json`, SHA-256
  `8cc93244e124ae1e5b9e30672d54db8cdf6ff8234836169fd3df33ba04965b09`.
  Median instructions are `175818428240`, `-0.19%` versus fixed and `-0.08%`
  versus Phase 3. Median maximum RSS is `768446464`, `+0.71%` versus fixed and
  `-0.31%` versus Phase 3. Median peak footprint is `592515072`, `-0.06%`
  versus fixed and `-0.04%` versus Phase 3. Both comparisons pass. RSS does
  not trigger a confirmation batch. The rolling file contains this exact
  candidate.
- `Type`, `TemplateArgument`, `ClassInfo`, `FunctionBinding`, and
  `ValueBinding` retain their fixed sizes. `TemplateLifecycleTransition`
  grows from 88 to 96 bytes to carry an explicit class-template declaration;
  it is a stack/result value, not a hot semantic object. The ordinary
  `cppgm++` is 17,139,592 bytes; Mach-O `__TEXT` is 13,069,446 bytes and
  `__DATA` is 442,600 bytes.
- Phase 4 adds 328 and removes 247 production lines, a net addition of 81.
  Cumulative production change from the fixed checkpoint is 1,365 additions
  and 766 deletions, or +599 lines. Phase 5 and final cleanup must delete more
  than 599 net lines before the final gate.

## Phase 5 evidence: nested and unnamed class completion

- Correctness commit: `313662d3045293796a2802c4e5e74441f3b7007b`.
- Nested member completion returns a typed class transition with the selected
  declaration node. The observer formats its entity and source anchor and is
  the only layer allowed to pass a completed semantic transition through a
  lifecycle pause.
- Unnamed local classes are marked before their members are populated, so
  nested child transitions retain the correct unnamed parent. Anonymous
  members create their transition when semantic class population records the
  member. The recursive field walk, its four direct producers, and
  `ClassInfo::template_instantiation_log_emitted` are deleted.
- The existing PA19 dependent-anonymous-member reducer now forces
  `Box<int>` completion with a namespace-scope `static_assert`. Patched Clang
  generated its updated witness reference. Its LowIR remains byte-identical to
  the existing reference, and CPPGM matches the new lifecycle row.
- Final strict provenance trace:
  `/tmp/cppgm-lifecycle-phase5-final-provenance.wI0FZ5`.
- Final strict report:
  `/tmp/cppgm-lifecycle-phase5-final-provenance-report.json`, SHA-256
  `1702282cba1fab28ff742ce4f2aa6767dedd2e3ec6a2078506587206dd94130e`.
  The 1,181 traces contain 23,974 records and no unknown producer attempt.
  `lifecycle.transition_observer.01` makes 4,493 attempts and inserts all
  4,493. It records no exact duplicate, enrichment, rejection, replacement,
  or lifecycle collision.
- The diagnostic and ordinary direct-LowIR strict runs pass 1,305/1,305. The
  ordinary PA1-PA38 report passes 4,860/4,860. The promoted PA19 reducer also
  passes its focused ordinary and strict checks.
- Candidate performance file: `/tmp/cppgm-lifecycle-phase-5.json`, SHA-256
  `ce9e0194552a764042a15d509cde13d46e34cbb28e90c51181e21c2dc63b1707`.
  Median instructions are `175907081553`, `-0.14%` versus fixed and `+0.05%`
  versus Phase 4. Median maximum RSS is `768823296`, `+0.76%` versus fixed and
  `+0.05%` versus Phase 4. Median peak footprint is `592523264`, `-0.06%`
  versus fixed and effectively unchanged from Phase 4. Both comparisons pass
  the 0.5% instruction, 3% RSS warning, and 1% footprint thresholds. No RSS
  confirmation run is required. The rolling file contains this exact
  candidate.
- Hot semantic structures retain their fixed sizes, and
  `TemplateLifecycleTransition` remains 96 bytes. The ordinary `cppgm++` is
  17,133,616 bytes; Mach-O `__TEXT` is 13,066,142 bytes and `__DATA` is
  442,600 bytes.
- Phase 5 adds 239 and removes 298 production lines, a net deletion of 59.
  Cumulative production change from the fixed checkpoint is 1,579 additions
  and 1,039 deletions, or +540 lines. Final observer and recorder cleanup must
  remove at least 541 more net lines and retain the instruction reduction.

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
- The Phase 0 probe showed that patched Clang emitted one anonymous-member
  class-instantiation event while CPPGM emitted none. Phase 5 moved that event
  into class population and promoted the reducer to PA19.

## Open obligations

- [x] Add a targeted `.06` anonymous-member oracle reducer before deleting its
  route.
- [x] Promote the anonymous-member reducer to PA19 after the canonical class
  transition matches the Clang reference.
- [x] Record fixed `sizeof(FunctionBinding)` and `sizeof(ValueBinding)`.
- [x] Confirm the original class-use inception run and copy its result into the
  parent ledger.
- [x] Use `/tmp/cppgm-witness-lifecycle-obj` for worktree builds.
- [x] Record the Phase 1 candidate batch and promote it to the rolling file.
- [ ] Record one candidate batch for each remaining semantic phase. Phases 1
  through 5 are complete.
- [ ] Investigate each instruction, RSS, or footprint warning before advancing.
- [ ] Finish with a net production line and instruction reduction.

## Follow-up queue

1. Alias source-use convergence: four feed paths and a recursive template
   pattern walker remain behind one emitter.
2. Function-call source-use convergence: overload, conversion, declval, and
   constexpr-direct feeds remain behind one emitter.
3. Variable source use: retain the current single feed unless new provenance
   shows overlap.
