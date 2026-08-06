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
| 0. Evidence and comparison tool | pending commit | 9 | 4 | 0 / 0 | n/a | n/a | n/a | n/a | validator unit tests clean | n/a | n/a | fixed and rolling copies verified |
| 1. Typed result | pending | 9 | 4 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 2. Dependent pattern | pending | 8 | 4 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 3A. Nested results | pending | pending | pending | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 3B. Template patterns | pending | 6 | 2 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 4. Qualified constants | pending | 4 | 2 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 5. Definition owners | pending | 2 | 1 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 6. Alias provenance | pending | 1 | 0 | pending | pending | pending | pending | pending | pending | pending | as needed | pending |
| 7. Final observer and cleanup | pending | 1 | 0 | pending | pending | pending | pending | pending | pending | pending | clean | pending |

## Hot structure sizes

| Checkpoint | `Type` | `TemplateArgument` | `ClassInfo` | `OutOfClassStaticMemberDecl` | Retained source-result bytes or count |
| --- | ---: | ---: | ---: | ---: | ---: |
| Fixed | 280 | 136 | 1,136 | 136 | 0 |

`OutOfClassMemberFunctionDecl` starts at 240 bytes. The reporter source is
`scripts/report_semantic_structure_sizes.cpp`; compile it against `dev/src`
with the configured host compiler.

## Producer migration

| Deleted producer | Canonical result owner | Unique rows transferred | Targeted fixtures | Status |
| --- | --- | ---: | --- | --- |
| `class.callsemantic.08` | resolved dependent class-template-id | 4 | reference-shell current specialization | pending |
| `class.callsemantic.06` | nested typed source results | 35 | local variable nested class instantiation and strict unique-owner set | pending |
| `class.callsemantic.07` | template pattern semantic results | 75 | nested template parameter scope and strict unique-owner set | pending |
| `class.constant_value_lookup.02` | resolved selected-call owner chain | 2 | constexpr duration member calls | pending |
| `class.constant_value_lookup.03` | resolved qualified-id | 199 | dependent qualified static member value and strict unique-owner set | pending |
| `class.callsemantic.13` | resolved out-of-class owner | 29 | partial-specialization member outdef | pending |
| `class.template_instantiation` | substituted retained owner pattern | 24 | static member assignment and strict unique-owner set | pending |
| `class.callsemantic.10` | retained alias expansion result | 30 | materialized alias declaration and strict unique-owner set | pending |
| `class.class_template_reference.02` | final resolved-source observer | 1,118 | full strict set | pending migration |

## Performance cleanup obligations

| Opened in phase | Metric and delta | Evidence for cause | Required cleanup | Due phase | Status |
| --- | --- | --- | --- | --- | --- |
| none | | | | | |

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
