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
| 5. Definition owners | `acc13c758` | 2 | 1 | +2151 / -2120 | +0.84% | +0.06% | +0.93% | +0.46% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-5.json` |
| 6. Alias provenance | `1e4f5f760` | 1 | 0 | +2623 / -2450 | +0.83% | -0.01% | +0.10% | +0.42% | 1305/1305 | 4860/4860 | not required | `/tmp/cppgm-class-use-phase-6.json` |
| 7. Final observer and cleanup | `0662098ba` | 1 | 0 | +3219 / -4462 | -0.20% confirmed | -1.02% | +0.04% | +0.47% | 1305/1305 | 4860/4860 | clean | `/tmp/cppgm-class-use-final-confirmation.json` |

## Hot structure sizes

| Checkpoint | `Type` | `TemplateArgument` | `ClassInfo` | `OutOfClassStaticMemberDecl` | Retained source-result bytes or count |
| --- | ---: | ---: | ---: | ---: | ---: |
| Fixed | 280 | 136 | 1,136 | 136 | 0 |
| Phase 1 | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 80 bytes, stack-scoped |
| Phase 2 | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 96 bytes, stack-scoped; no retained allocation |
| Phase 3A | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 96 bytes; `RetainedDependentClassTemplateId`: 168 bytes, witness-session side store only |
| Phase 3B | 280 | 136 | 1,136 | 136 | `ResolvedAliasTemplateIdView`: 72 bytes, stack-scoped; class retained storage unchanged |
| Phase 4 | 280 | 136 | 1,136 | 136 | `ResolvedQualifiedId`: 40 bytes, stack-scoped; retained storage unchanged |
| Phase 5 | 280 | 136 | 1,136 | 136 | `ResolvedOwnerReference`: 32 bytes, witness-session side store only; `OutOfClassMemberFunctionDecl` remains 240 bytes |
| Phase 6 | 280 | 136 | 1,136 | 136 | `RetainedAliasClassUse`: 16 bytes, witness-session side store only; other retained result sizes unchanged |
| Phase 7 | 280 | 136 | 1,136 | 136 | `ResolvedClassTemplateIdView`: 104 bytes; `ResolvedAliasTemplateIdView`: 80 bytes; `ResolvedQualifiedId`: 40 bytes; `ResolvedOwnerReference`: 32 bytes; `RetainedAliasClassUse`: 16 bytes; all retained storage remains witness-session-only |

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

## Phase 5 evidence

- Correctness commit: `acc13c758b577a63e4702f4d7e2334ad3072991f`
- Candidate SHA-256:
  `dd12ee64f3b12855a8423f7d1b7226f5842ef8819c25969398367edb39195031`
- Instructions: `177998788270`, `+0.84%` versus fixed and `+0.06%`
  versus rolling
- Maximum RSS: `769835008`, `+0.93%` versus fixed and `+0.20%`
  versus rolling
- Peak footprint: `592838656`, `+0.46%` versus fixed and `+0.06%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-class-use-phase5-final-provenance.fJkM8P`
- Strict provenance report:
  `/tmp/cppgm-class-use-phase5-final-provenance-report.json`
- Provenance report SHA-256:
  `03a8d0a635e29f6d22dc78d93d6fe5cbe2dc79cd4bf9ce4fde40e81fa74d9ecd`
- `class.callsemantic.13` and `class.template_instantiation` make zero
  attempts and their producer IDs are absent. Their 29 and 24 final-visible
  rows are now owned by `class.class_template_reference.02`, whose final-
  visible count rises from 1,869 to 1,922, exactly the 53 transferred rows.
  The static-member-definition AST replay route is zero. The sole remaining
  replay route is `class_use.resolved_alias_type` (`1185`).
- Method, special-member, and static-member binding package the already-
  selected owner and structured qualifier syntax directly into a 32-byte
  `ResolvedOwnerReference`; they do not repeat lookup or specialization
  selection. Delayed definitions store a compact side-store handle and
  substitute the concrete owner when applied. The old qualifier text
  canonicalizer, direct owner analyzer, applied-definition analyzer, and AST
  replay interface are deleted.
- The witness-session side store is absent from ordinary compilation. `Type`,
  `TemplateArgument`, `ClassInfo`, `OutOfClassStaticMemberDecl`, and
  `OutOfClassMemberFunctionDecl` retain their fixed sizes.
- The phase deletes 311 net lines. Cumulatively, production code is
  `+2151 / -2120` versus fixed.
- Ordinary binary: 17,247,648 bytes; Mach-O `__TEXT` 13,135,872 bytes and
  `__DATA` 446,464 bytes. Ordinary builds contain no provenance symbols.
- Cleanup obligation remains open: the fixed-baseline instruction result is
  above the `+0.5%` investigation threshold, although this phase adds only
  `+0.06%` versus rolling. Phase 7 must remove the accumulated scaffolding and
  finish below the fixed median.

## Phase 6 evidence

- Correctness commit: `1e4f5f7609ab6abfdff4c3789a4e38f4bc38977e`
- Candidate SHA-256:
  `e3d0d3b259a99d65a997cf5864ff08d9e9e4568b2a318f373e28fa569cdf14f1`
- Instructions: `177974535739`, `+0.83%` versus fixed and `-0.01%`
  versus rolling
- Maximum RSS: `763510784`, `+0.10%` versus fixed and `-0.82%`
  versus rolling
- Peak footprint: `592592896`, `+0.42%` versus fixed and `-0.04%`
  versus rolling
- Strict provenance trace:
  `/tmp/cppgm-class-use-phase6-provenance.YUx9rW`
- Strict provenance report:
  `/tmp/cppgm-class-use-phase6-provenance-report.json`
- Provenance report SHA-256:
  `f7e6a11a0e9baaf2369d74cdd51b49ff002d7d8de079f8a32ed21d20fe2ca94c`
- `class.callsemantic.10` makes zero attempts and its implementation producer
  ID is absent. The canonical `class.class_template_reference.02` producer
  owns 1,952 final-visible rows, exactly the Phase 5 total plus the 30 migrated
  alias rows. All four replay routes report zero calls.
- Alias expansion retains a 16-byte origin/instance pair and propagates its
  handle through the same type-analysis result used by ordinary declaration
  parsing. Namespace, statement, and constant-evaluation alias declarations
  all register that result through one public operation; none performs class
  recovery, source scanning, or independent specialization selection.
- The alias audit found one invalid intermediate pairing where a template
  origin could accompany an unrelated ordinary class instance. Retention now
  enforces `instance->source_template == origin` at the creation boundary.
  The result channel also validates that the final declaration type is the
  retained class, which prevents nested alias machinery from becoming a
  second class-use analyzer.
- The old token-range probe, recursive template-id spelling search,
  post-`TypePtr` class recovery, replay interface, route ID, and producer ID
  are deleted. The 42-fixture migration set, including reference-shell,
  member-alias, alias-template, and MP11 cases, passes in full.
- The phase adds 142 net production lines while introducing the retained
  alias side store. Cumulatively, production code is `+2623 / -2450`, or 173
  net added lines versus fixed. Phase 7 must delete this scaffolding and finish
  net negative.
- Ordinary binary: 17,259,432 bytes; Mach-O `__TEXT` 13,144,064 bytes and
  `__DATA` 446,464 bytes. Hot semantic structure sizes are unchanged.
- Cleanup obligation remains open: instructions are `+0.83%` versus fixed.
  Phase 7 must remove the accumulated collection and emission scaffolding and
  satisfy the repeatable final reduction rule.

## Phase 7 evidence

- Correctness and cleanup commits: `4368a3cba`, `76477f727`, `4a7ef4cdf`, and
  `0662098ba13097ae0ac059593443a311f769a59b`.
- The final production inventory has one direct `witness::emit_class_use`
  call, one direct `witness::emit_alias_use` call, and no replay interface.
  The completed resolved-source collection owns class publication; the old
  location, AST, template-argument, alias-type, and delayed-definition
  recovery arms are absent.
- Final strict provenance trace:
  `/tmp/cppgm-class-use-final-provenance.5eQcbQ`
- Final strict provenance report:
  `/tmp/cppgm-class-use-final-provenance-report.json`
- Provenance report SHA-256:
  `e94d543d32cd24cbf43c36b8386de1a3570fd2842308246d781be68c7013485f`
- The trace contains 59,412 records across 1,180 files, no unknown producer,
  no source or lifecycle collision pair, and zero calls to all four retired
  replay routes. `class.class_template_reference.02` is the sole class owner:
  4,530 attempts and 1,952 final-visible rows. Its 56 replacements are
  same-producer occurrence enrichment, not competing semantic ownership.
- The first final performance batch is
  `/tmp/cppgm-class-use-final.json` (SHA-256
  `8f7425d341669d8c0389653711b3414ec792bc63b238755f5f1bca9dbdab289f`):
  176,074,869,502 instructions (`-0.25%`), 762,454,016 maximum RSS
  (`-0.03%`), and 593,039,360 peak footprint (`+0.49%`) versus fixed.
- Because the instruction reduction was below 0.5%, the required independent
  confirmation batch is `/tmp/cppgm-class-use-final-confirmation.json`
  (SHA-256
  `bc6c01464e356e2c4da69f0a63e3baec74e7322f15931030540afaabea566203`):
  176,156,071,669 instructions (`-0.20%`), 763,035,648 maximum RSS
  (`+0.04%`), and 592,871,424 peak footprint (`+0.47%`). Both instruction
  medians are below fixed, so the repeatable-reduction rule passes. The
  confirmation batch improves instructions by 1.02% versus Phase 6.
- Production code is cumulatively `+3219 / -4462`, a net deletion of 1,243
  lines and a final `dev/src` count of 416,807 lines. Ordinary `cppgm++` is
  17,136,464 bytes; Mach-O `__TEXT` is 13,070,336 bytes and `__DATA` is
  446,464 bytes. Compared with fixed, the file shrank by 126,928 bytes and
  `__TEXT` by 77,824 bytes; hot semantic structures did not grow.
- Direct LowIR strict passes 1,305/1,305, the PA1-PA38 report passes
  4,860/4,860, and the diagnostic provenance build passes the same strict
  1,305/1,305 corpus. The ordinary binary contains no provenance symbols.
- `make inception CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` passes. The
  self-hosted objects and final `cppgm++-inception` binary match the host-built
  results.
- The cumulative performance cleanup obligation is closed: ordinary parsing
  bypasses witness source assembly, callback bundles are reused, and witness
  source-capture predicates return before constructing source context.

## Producer migration

| Deleted producer | Canonical result owner | Unique rows transferred | Targeted fixtures | Status |
| --- | --- | ---: | --- | --- |
| `class.callsemantic.08` | resolved dependent class-template-id | 4 | reference-shell current specialization plus PA23 owner and PA24 integral-constant unique rows | migrated in `e05e22320` |
| `class.callsemantic.06` | nested typed source results | 35 | local variable nested class instantiation and strict unique-owner set | migrated in `e96c44379` |
| `class.callsemantic.07` | template pattern semantic results | 75 | nested template parameter scope and strict unique-owner set | migrated in `9ea979c45` |
| `class.constant_value_lookup.02` | resolved selected-call owner chain | 2 | constexpr duration member calls | migrated in `e4f3fada5` |
| `class.constant_value_lookup.03` | resolved qualified-id | 199 | dependent qualified static member value and strict unique-owner set | migrated in `e4f3fada5` |
| `class.callsemantic.13` | resolved out-of-class owner | 29 | partial-specialization member outdef | migrated in `acc13c758` |
| `class.template_instantiation` | substituted retained owner pattern | 24 | static member assignment and strict unique-owner set | migrated in `acc13c758` |
| `class.callsemantic.10` | retained alias expansion result | 30 | materialized alias declaration and strict unique-owner set | migrated in `1e4f5f760` |
| `class.class_template_reference.02` | final resolved-source observer | 1,118 | full strict set | sole final owner in `0662098ba` |

## Performance cleanup obligations

| Opened in phase | Metric and delta | Evidence for cause | Required cleanup | Due phase | Status |
| --- | --- | --- | --- | --- | --- |
| 4 | instructions `+0.84%` versus fixed at Phase 5 | Four Phase 4 medians were between `+0.59%` and `+0.80%`; Phase 5 adds only `+0.06%` versus rolling, so the remaining increase is cumulative result/observer scaffolding rather than its owner substitution | Remove remaining transient result/observer scaffolding and finish with a repeatable reduction below the fixed median | 7 | closed in `0662098ba`; two final medians are `-0.25%` and `-0.20%` |

## Final audit

- [x] One direct class producer and one observation pass
- [x] Zero replay routes
- [x] Net production source-line deletion
- [x] Repeatable instruction reduction against the fixed checkpoint
- [x] Peak footprint within 1%
- [x] Maximum RSS clears the 3% warning rule
- [x] Strict witness and direct LowIR comparison pass
- [x] PA1-PA38 report passes
- [x] Inception passes
- [x] No open cleanup obligation

## Follow-up witness-family convergence audit

The final provenance trace also exposes where the same convergence method can
pay off outside class use. A single direct emitter is not sufficient evidence
of semantic convergence: alias and function output each have one emitter but
still receive independently reconstructed decisions.

| Family | Static shape | Final strict evidence | Assessment |
| --- | --- | --- | --- |
| Lifecycle | 11 producer sites; 10 exercised across `template_api.cpp`, `callsemantic.cpp`, and `constant_value_lookup.cpp` | 34,383 attempts, 24,072 exact duplicates, 267 enrichments, 10,044 surviving events | Strongest next target. Function, class, and variable state transitions should publish non-owning typed transition views through one observer instead of formatting events at acquisition, explicit-instantiation, nested/anonymous-class, and constant-lookup sites. |
| Alias use | One emitter and one producer, but four static feeds into `observe_resolved_alias_template_id` plus a recursive template-pattern AST visitor with five origin modes | 1,326 attempts, 387 exact duplicates, 14 same-producer replacements, 564 final-visible rows; renderer construction replaces 61 alias events and later drops 55 source-spelled duplicates | Same hidden shape as pre-convergence class use. Retain resolved alias children during normal template-parameter/type analysis and delete `record_alias_uses` plus its direct/nested/qualified/pattern syntax arms. Do not grow `Type`, `TemplateArgument`, or `AliasTemplateDecl`; use a witness-session side collector and stack views. |
| Function call | One emitter and producer, but four request-building feeds: overload selection, conversion-function selection, `declval`, and constexpr direct-call lookup | 920 attempts, 133 exact duplicates, 599 final-visible rows; no competing producer or replacement | Worth a smaller typed-result pass. The constexpr path currently rediscovers the source template and may resolve explicit arguments again. Carry the selected binding, candidate counts/drops, anchors, and resolved arguments from overload/conversion results into constant evaluation. The builtin `declval` arm may remain a distinct result constructor. |
| Variable use | One emitter, one producer, one request-construction path | 31 attempts, no duplicates, replacements, or collisions; all 31 rows final-visible | Already converged. The pending-finalization side store changes publication time, not semantic ownership; leave it alone unless later evidence finds a second analyzer. |

Recommended order is lifecycle, alias, then function call. Before changing any
family, add diagnostic-only route IDs for the semantic feeds hidden behind its
current shared producer. Preserve the fixed performance epoch, use non-owning
views or witness-session storage for intermediate metadata, and require net
source and instruction reduction at the end of each family project. Renderer
passes are not the first target: every remaining class policy acts in the final
trace, while the alias renderer replacements are downstream evidence of the
upstream alias duplication described above.
