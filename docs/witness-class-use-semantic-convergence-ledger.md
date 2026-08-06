# Class-Use Semantic Convergence Ledger

This ledger accompanies `witness-class-use-semantic-convergence-plan.md`.
Update it after each correctness-clean, committed phase. Keep generated JSON,
profiles, and provenance traces outside the repository. Record their paths and
summaries here.

## Fixed checkpoint

- Semantic code: `a42c915e3acd91ee56f9f8913d0c5695ad52ccf5`
- Plan-creation head: `50593a940`
- Workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`
- Runs: 3, median comparison
- Instructions retired: `176517676986`
- Maximum RSS: `762712064`
- Peak footprint: `590123008`
- Fixed artifact: `/tmp/cppgm-class-use-convergence-fixed.json`
- Rolling artifact: `/tmp/cppgm-class-use-convergence-rolling.json`
- Starting provenance report:
  `/tmp/cppgm-witness-provenance-final-post-cleanup-report.json`
- Fixed artifact SHA-256:
  `7f8966704938fb75161cc457a6e9cf55d42985e5284c8ab2e8665ef4ad22adfd`
- Starting provenance report SHA-256:
  `bdc6fde95e42bf17eeb1dc1bea3a73836077acd14ee629dfd8a6d9c66663e66f`

The fixed artifact includes the accepted diagnostic counters. Do not replace
or rerecord it.

## Starting semantic inventory

- Direct class producers: 9
- Public replay routes: 4
- Direct canonical observer: none; emission occurs during several analyzers
- Production source delta from fixed checkpoint: 0
- Production `dev/src` lines: 418,050
- Ordinary `cppgm++` file size: 17,263,392 bytes
- Ordinary `cppgm++` Mach-O `__TEXT`: 13,148,160 bytes
- Ordinary `cppgm++` Mach-O `__DATA`: 446,464 bytes
- Open performance cleanup obligations: none

## Phase results

| Phase | Commit | Class sites | Replay routes | Production lines + / - | Instructions vs fixed | Instructions vs rolling | RSS vs fixed | Footprint vs fixed | Strict | Full | Inception | Artifact |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- |
| Fixed | `a42c915e3` | 9 | 4 | 0 / 0 | baseline | baseline | baseline | baseline | clean | 4860/4860 | not rerun | `/tmp/cppgm-class-use-convergence-fixed.json` |
| 0. Evidence and comparison tool | `546181f8e` | 9 | 4 | 0 / 0 | n/a | n/a | n/a | n/a | validator unit tests clean | n/a | n/a | fixed and rolling copies verified |
| 1. Typed result | `a628fa997` | 9 | 4 | +91 / -16 | +0.16% | +0.16% | +1.03% | +0.05% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-1.json` |
| 2. Dependent pattern | `e05e22320` | 8 | 4 | +230 / -79 | +0.48% | +0.33% | +0.94% | +0.02% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-2.json` |
| 3A. Nested results | `e96c44379` | 7 | 2 | +1120 / -310 | +0.37% | -0.11% | +0.06% | +0.10% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-3a-final.json` |
| 3B. Template patterns | `9ea979c45` | 6 | 2 | +1499 / -936 | +0.31% | -0.06% | +0.54% | +0.11% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-3b.json` |
| 4. Qualified constants | `e4f3fada5` | 4 | 2 | +1700 / -1355 | +0.78% | +0.47% | +0.74% | +0.40% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-4-compact.json` |
| 5. Definition owners | pending | 2 | 1 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 6. Alias provenance | pending | 1 | 0 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 7. Final observer and cleanup | pending | 1 | 0 | pending | pending | pending | pending | pending | pending | pending | clean | pending |

## Hot structure sizes

| Checkpoint | `Type` | `TemplateArgument` | `ClassInfo` | `OutOfClassStaticMemberDecl` | Retained source-result bytes or count |
| --- | ---: | ---: | ---: | ---: | ---: |
| Fixed | 280 | 136 | 1,136 | 136 | 0 |
| Phase 1 | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 80 bytes, stack-scoped |
| Phase 2 | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 96 bytes, stack-scoped; no retained allocation |
| Phase 3A | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 96 bytes; `RetainedDependentClassTemplateId`: 168 bytes, witness-session side store only |
| Phase 3B | 280 | 136 | 1,136 | 136 | `ResolvedAliasTemplateIdView`: 72 bytes, stack-scoped; class retained storage unchanged |
| Phase 4 | 280 | 136 | 1,136 | 136 | `ResolvedQualifiedId`: 40 bytes, stack-scoped; retained storage unchanged |

`OutOfClassMemberFunctionDecl` starts at 240 bytes. The reporter source is
`scripts/report_semantic_structure_sizes.cpp`; compile it against `dev/src`
with the configured host compiler.

## Phase 1 performance evidence

- Candidate SHA-256:
  `ee6e14f3ff53b1d3a97d8e404bbc4d3339365dfbe6394c92a034034d63340668`
- Instructions: `176791783940`, `+0.16%` versus fixed
- Maximum RSS: `770568192`, `+1.03%` versus fixed
- Peak footprint: `590442496`, `+0.05%` versus fixed
- Result storage: one non-owning stack view; no hot semantic structure grew
- Cleanup obligation: none; all metrics remain inside the intermediate
  investigation thresholds

## Phase 2 evidence

- Correctness commit: `e05e223200bf7647fecafd78126820fcc68e9d9e`
- Candidate SHA-256:
  `b8b067ad1852d160e3d5beae2f0baf09db4feab5ddbe3b5dcb883cfe8a47bafe`
- Instructions: `177368727469`, `+0.48%` versus fixed and `+0.33%`
  versus rolling
- Maximum RSS: `769888256`, `+0.94%` versus fixed and `-0.09%`
  versus rolling
- Peak footprint: `590258176`, `+0.02%` versus fixed and `-0.03%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-class-use-phase2-strict-provenance.6N3szo`
- Strict provenance report:
  `/tmp/cppgm-class-use-phase2-strict-provenance-report.json`
- Provenance report SHA-256:
  `fc100cb7f84f100c97c05bde5b6fe272ea53a82514cffc8778cd30143eb601c0`
- The four rows previously unique to `class.callsemantic.08` are uniquely
  owned by `class.class_template_reference.02`; `.08` and its producer ID are
  absent, and the four replay-route counts remain exactly at the fixed values.
- The direct path forwards its already-resolved arguments and selection. A
  cached current-specialization path derives the selection from the retained
  `ClassInfo::template_output_node`; when that concrete instance no longer
  carries dependent source arguments, it resolves only the source arguments
  against the already-selected partial pattern. Phase 3B removes that last
  transient resolution by retaining the pattern result at definition time.
- Ordinary-build dependent shortcuts do not perform selection for source
  observation; the additional selection is guarded by an active witness
  session. No hot semantic structure grew and there is no retained allocation.
- Ordinary binary: 17,264,624 bytes; Mach-O `__TEXT` 13,148,160 bytes and
  `__DATA` 446,464 bytes. The section sizes are unchanged from fixed; the file
  increase is link-edit metadata.
- Cleanup obligation: none; all fixed and rolling metrics remain inside the
  intermediate investigation thresholds.

## Phase 3A evidence

- Correctness commits: `c34eeb27c6b2cb295cfea3175cae94dc66b9bd83`,
  `7fff941d36b1b6a99846256d84ce1f09deb91276`, and
  `e96c4437971eb380337fd6e5a44f9673fa814834`
- Candidate SHA-256:
  `a51805d15ae96f04316dc84092683a47f959903a6ad2e5d4a671418783c2c049`
- Instructions: `177166918329`, `+0.37%` versus fixed and `-0.11%`
  versus rolling
- Maximum RSS: `763179008`, `+0.06%` versus fixed and `-0.87%`
  versus rolling
- Peak footprint: `590716928`, `+0.10%` versus fixed and `+0.08%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-class-use-phase3a-final.X906oe`
- Strict provenance report:
  `/tmp/cppgm-class-use-phase3a-provenance-report.json`
- Provenance report SHA-256:
  `fed9a54672bf613cd32194e5ed5b7ff9a7cd3140338aca0fb41df260940ce9bb`
- `class.callsemantic.06` and its producer ID are absent. The nested AST and
  template-argument route counts are both zero. The two remaining active
  routes are `class_use.resolved_alias_type` (`1185`) and
  `class_use.static_member_definition_ast_node` (`25`).
- The typed observer consumes materialized `TypePtr` metadata, resolved
  template arguments, retained qualifier types, and a compact dependent
  selection result. The default-filled partial pattern in
  `300-partial-specialization-default-function-type-no-eager.t` is formatted
  from the carried arguments and already-selected partial declaration without
  repeating lookup or specialization selection.
- Two rejected diagnostic batches exposed an ordinary-build source scan:
  `/tmp/cppgm-class-use-phase-3a.json` measured `541065170075` instructions,
  and `/tmp/cppgm-class-use-phase-3a-corrected.json` measured `541363324698`.
  The canonical callback was running exact source-syntax lookup before its
  witness predicate. Commit `e96c44379` moved that lookup and observer
  submission under syntax-backed capture. A one-run probe then measured
  `178142728159` instructions before the final three-run batch.
- Ordinary binary: 17,295,408 bytes; Mach-O `__TEXT` 13,168,640 bytes and
  `__DATA` 446,464 bytes. The retained side stores remain empty without a
  witness session, and no hot semantic structure grew.
- Cleanup obligation: none; all fixed and rolling metrics remain inside the
  intermediate investigation thresholds.

## Phase 3B evidence

- Correctness commit: `9ea979c4509341e9ac47ee994a887abc848edd34`
- Candidate SHA-256:
  `9caa6c3c57126ea9f630912b1e950e74909009d1e763ad4fa725c4429793f046`
- Instructions: `177066056074`, `+0.31%` versus fixed and `-0.06%`
  versus rolling
- Maximum RSS: `766816256`, `+0.54%` versus fixed and `+0.48%`
  versus rolling
- Peak footprint: `590761984`, `+0.11%` versus fixed and `+0.01%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-phase3b-nested-patterns.Kak97J`
- Strict provenance report:
  `/tmp/cppgm-phase3b-nested-patterns-report.json`
- Provenance report SHA-256:
  `a6d3feabda320302468d850c07a200e0b3720110f63623e502d0286173f5c404`
- `class.callsemantic.07` and `alias.callsemantic.03` make zero attempts and
  their producer IDs are absent. Their source rows are now owned by the
  canonical class-template reference and alias resolution paths.
- Declaration collection passes its already-parsed pattern scope into the
  definition-time source analysis. Nested member-template declarations create
  one parameterized scope for their definition analysis; the old second
  top-level parameter parse, custom class selection arm, syntax-only alias
  emitter, source-decision cache, and public recovery callback are gone.
- A dependent alias that cannot yet reduce its arguments submits a
  72-byte non-owning `ResolvedAliasTemplateIdView`. The observer formats the
  source binding without changing the semantic return type or allocating in
  ordinary compilation.
- The ordinary and provenance renderer branches now apply source-defined-call
  normalization before member-alias qualification. This fixes a pre-existing
  build-mode ordering divergence that the new canonical alias events exposed.
- The two starting nested replay routes remain zero. The two remaining active
  routes are `class_use.resolved_alias_type` (`1185`) and
  `class_use.static_member_definition_ast_node` (`25`).
- The phase itself removes 247 net production lines. Cumulatively, production
  code is `+1499 / -936` versus fixed.
- Ordinary binary: 17,284,608 bytes; Mach-O `__TEXT` 13,160,448 bytes and
  `__DATA` 446,464 bytes. Hot semantic structure sizes are unchanged.
- Cleanup obligation: none; all fixed and rolling metrics remain inside the
  intermediate investigation thresholds.

## Phase 4 evidence

- Correctness commits: `642ce25dc1c2c89bfac17348c24c77b4fc1fb07f`,
  `220baa421f26a868428023461626da489ea3c9b9`,
  `f8fc7d6d4`, and `e4f3fada5d63ba66dea97d75955cfd15519a89d3`
- Accepted candidate SHA-256:
  `b75a68651140fa7e72cdecbcc593e8b4ad3df6eb2139bed91133626c45b88541`
- Instructions: `177892093111`, `+0.78%` versus fixed and `+0.47%`
  versus rolling
- Maximum RSS: `768319488`, `+0.74%` versus fixed and `+0.20%`
  versus rolling
- Peak footprint: `592461824`, `+0.40%` versus fixed and `+0.29%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-class-use-phase4-clean-provenance.1EHKJk`
- Strict provenance report:
  `/tmp/cppgm-class-use-phase4-clean-provenance-report.json`
- Provenance report SHA-256:
  `0129111e321c91985801a1918792d77f5f060c07629b5ed02be6cc08ed2003f8`
- `class.constant_value_lookup.02` and `.03` make zero attempts and their
  producer IDs are absent. The two and 199 rows respectively unique to those
  producers are now owned by `class.class_template_reference.02`. Its
  final-visible count rises from 1,668 to 1,869, exactly the 201 transferred
  rows. The two remaining replay routes are unchanged:
  `class_use.resolved_alias_type` (`1185`) and
  `class_use.static_member_definition_ast_node` (`25`).
- Normal qualified-id lookup now returns the selected binding and resolved
  owner type in a 40-byte `ResolvedQualifiedId`. Constant evaluation
  materializes only the selected value, and constexpr direct-call observation
  consumes the selected call owner. The independent constant-value class
  lookup, argument resolution, specialization selection, explicit-
  specialization check, and custom emission arms are gone.
- Four correctness-clean commits were measured because each preceding median
  crossed the `+0.5%` instruction investigation threshold. The medians were
  `+0.75%`, `+0.80%`, `+0.59%`, and `+0.78%` versus fixed. Investigation
  removed an ordered-lookup cache bypass, unconditional ordinary-build
  witness-state checks, redundant source-location normalization, unused
  owner/use-scope fields, and 16 bytes from `ResolvedQualifiedId`. The final
  median remains above the investigation threshold but below the cumulative
  pause threshold; memory metrics pass and no hot semantic structure grew.
- The phase reduces cumulative production code by 218 net lines. Cumulatively,
  production code is `+1700 / -1355` versus fixed.
- Ordinary binary: 17,270,920 bytes; Mach-O `__TEXT` 13,152,256 bytes and
  `__DATA` 446,464 bytes. Ordinary builds contain no provenance symbols.
- Cleanup obligation: remove remaining transient resolved-source/observer
  scaffolding and its instruction cost no later than Phase 7. The final fixed-
  baseline instruction median must be below the fixed checkpoint, independent
  of the rolling comparison.

## Producer migration

| Deleted producer | Canonical result owner | Unique rows transferred | Targeted fixtures | Status |
| --- | --- | ---: | --- | --- |
| `class.callsemantic.08` | resolved dependent class-template-id | 4 | reference-shell current specialization plus PA23 owner and PA24 integral-constant unique rows | migrated in `e05e22320` |
| `class.callsemantic.06` | nested typed source results | 35 | local variable nested class instantiation and strict unique-owner set | migrated in `e96c44379` |
| `class.callsemantic.07` | template pattern semantic results | 75 | nested template parameter scope and strict unique-owner set | migrated in `9ea979c45` |
| `class.constant_value_lookup.02` | resolved selected-call owner chain | 2 | constexpr duration member calls | migrated in `e4f3fada5` |
| `class.constant_value_lookup.03` | resolved qualified-id | 199 | dependent qualified static member value and strict unique-owner set | migrated in `e4f3fada5` |
| `class.callsemantic.13` | resolved out-of-class owner | 29 | partial-specialization member outdef | pending |
| `class.template_instantiation` | substituted retained owner pattern | 24 | static member assignment and strict unique-owner set | pending |
| `class.callsemantic.10` | retained alias expansion result | 30 | materialized alias declaration and strict unique-owner set | pending |
| `class.class_template_reference.02` | final resolved-source observer | 1,118 | full strict set | pending migration |

## Performance cleanup obligations

| Opened in phase | Metric and delta | Evidence for cause | Required cleanup | Due phase | Status |
| --- | --- | --- | --- | --- | --- |
| 4 | instructions `+0.78%` versus fixed | Four distinct three-run medians remain between `+0.59%` and `+0.80%`; result compaction and removal of avoidable ordinary-path work did not clear the threshold | Remove remaining transient result/observer scaffolding and finish with a repeatable reduction below the fixed median | 7 | open |

## Final audit

- [ ] One direct class producer and one observation pass
- [ ] Zero replay routes
- [ ] Net production source-line deletion
- [ ] Repeatable instruction reduction against the fixed checkpoint
- [ ] Peak footprint within 1%
- [ ] Maximum RSS clears the 3% warning rule
- [ ] Strict witness and direct LowIR comparison pass
- [ ] PA1-PA38 report passes
- [ ] Inception passes
- [ ] No open cleanup obligation
