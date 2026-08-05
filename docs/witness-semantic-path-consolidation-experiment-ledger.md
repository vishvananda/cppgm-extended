# Witness Semantic-Path Consolidation Experiment Ledger

This ledger records the evidence and acceptance gates for
`witness-semantic-path-consolidation-experiment-plan.md`. Generated provenance
reports remain under `/tmp`; only summarized evidence belongs here.

## Revised performance epoch

- Fixed diagnostic baseline:
  `/tmp/cppgm-witness-consolidation-diagnostic-3run.json`
- Rolling baseline: `/tmp/cppgm-witness-consolidation-rolling.json`
- Recorded head: `ba6b1070c80426a31dff6cceebe3d504450373c9`
- Workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`
- Runs: `3`; summaries use the median
- Instructions retired: `177369144531`
- Maximum resident set size: `760176640`
- Peak memory footprint: `594513920`
- Acceptance limits: instructions `+0.5%`; maximum RSS and peak footprint
  `+1%`

The revised baseline deliberately includes the accepted compile-time-guarded
diagnostic instrumentation. The prior parent and exact one-run reports remain
under `/tmp` for audit but do not gate later slices. Do not rerecord the fixed
diagnostic baseline. Promote only an already-recorded three-run candidate that
passes all three tolerance gates.

## Instrumentation checkpoint

The provenance implementation is an explicit diagnostic build. Enable it with
`CPPGM_WITNESS_PROVENANCE=1` at build time and set
`CPPGM_WITNESS_PROVENANCE_DIR` at run time. The default build compiles the
provenance tables, renderer lineage tracking, route calls, and RAII state out;
producer fields and lifecycle parameters retain their original layout, and the
provenance translation unit is omitted from frontend links. The default
`cppgm++` has no provenance symbols, and setting only the run-time directory
against it creates no trace files.

The diagnostic build assigns all 53 static producer IDs:
24 class-use, 7 alias-use, 4 function-call, 1 variable-use, and 17 lifecycle
sites. It also counts nine public class replay/fanout routes. No attempt in the
strict witness corpus used the `unknown` producer.

Evidence:

- strict trace directory:
  `/tmp/cppgm-witness-provenance-strict.80t4v7`
- strict aggregate report:
  `/tmp/cppgm-witness-provenance-strict-report.json`
- strict result with direct LowIR comparison: all configured PA19, PA20, PA22,
  PA23, and PA24 comparisons passed;
- full PA1-PA38 report with direct LowIR comparison: `4860/4860` passed;
- the ordinary full report has no witness-rendering lane, so it validates
  behavior but does not add provenance records beyond the strict witness
  corpus.

Strict-corpus public route counts:

| Route | Calls |
| --- | ---: |
| callback location replay | 752 |
| location replay | 58 |
| after-location replay | 13 |
| AST replay | 9065 |
| template-argument replay | 63 |
| static-member-definition AST replay | 25 |
| resolved alias-type replay | 1185 |
| resolved type-node replay | 11632 |
| declaration resolved type-node replay | 1015 |

The strongest initial class collision is between the recursive nested syntax
producer and the canonical class-template reference producer: 2,860 collided
attempts. The nested syntax producer made 32,715 attempts, including 32,538
exact duplicates. The semantic-template-class qualifier producer inserted 262
rows, but no row reached visible output; all 262 were removed during renderer
canonicalization.

After accepting the overload-owner removal, the refreshed strict trace is
`/tmp/cppgm-witness-provenance-overload-owner.8374ZI` and its report is
`/tmp/cppgm-witness-provenance-overload-owner-report.json`. All 1,305 strict
comparisons remained clean. The deleted producer is absent, provenance records
fell from 118,957 to 118,158, and renderer canonicalization removals fell from
394 to 132; the exact 262-row reduction belongs to the removed producer.

After accepting the return-type and location-replay removals, the next strict
trace is `/tmp/cppgm-witness-provenance-location-replay.d2XOwd` and its report
is `/tmp/cppgm-witness-provenance-location-replay-report.json`. All 1,305
strict comparisons remained clean. The three accepted producers are absent,
the two location routes are absent, trace records fell to 117,271, and the
remaining seven upstream routes retain their prior counts except template-
argument replay, which fell from 63 to 62 calls.

The first run-time-guarded implementation was discarded after its one allowed
candidate measurement. Instructions improved by 0.09%, but maximum RSS
increased by 0.23% and footprint by 0.11%, so the candidate was not promoted.
The compile-time diagnostic boundary was introduced in response; its candidate
still failed its first measurement with instructions passing, RSS up 0.09%,
and footprint up 0.02%. That candidate was also not promoted or rerun. The
default path was then fully erased through preprocessing and the provenance
object removed from ordinary links. That candidate also remained unpromoted:
its sole run reported instructions up 0.08%, RSS up 1.81%, and footprint up
0.03%. A parent-source comparison then found every affected normal object
byte-identical except for the temporary build's embedded object-root path. The
remaining renderer ABI and helper-shape differences were restored exactly
before the accepted candidate. That run improved instructions by 0.15%, RSS by
1.09%, and footprint by 20 KiB. Its recorded candidate object was promoted to
the rolling baseline without rerecording it.

Sites not exercised by the existing strict witness references are tracked
until they are reached by an earliest-owning-PA reducer or removed as proven
dead/redundant code:

- class: `callsemantic.01`, `.03`, `.05`, `.09`,
  `class_template_reference.01`, `constant_value_lookup.04`, `.05`, and
  `template_declaration_collector`;
- alias: `template_specialization.01`, `.02`, and `callsemantic.01`;
- lifecycle: `template_api.06`, `.08`,
  `template_argument_semantics.01`, and `constant_value_lookup.01`.

## Semantic slice ledger

| Slice | Direct class sites before/after | Upstream routes before/after | Semantic route removed | Responsibility moved to | Strict | Full report | Instructions | Max RSS | Footprint | Renderer passes made idle |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | --- |
| Parent, historical one-run | 24/24 | 9/9 | none | none | clean | clean | 177731452181 | 768765952 | 593932288 | none |
| Diagnostic provenance, historical one-run | 24/24 | 9/9 | none | none | clean | 4860/4860 | 177466053572 | 760369152 | 593911808 | none |
| Diagnostic provenance, revised baseline | 24/24 | 9/9 | none | none | clean | 4860/4860 | 177369144531 | 760176640 | 594513920 | none |
| Overload owner reconstruction | 24/23 | 9/9 | overload-side instantiated class owner lookup, anchor recovery, and binding reconstruction | selected class-reference result | clean | 4860/4860 | 177831649503 (+0.26%) | 766881792 (+0.88%) | 594264064 (-0.04%) | none |
| Explicit-specialization return-type replay | 23/22 | 9/9 | out-of-class definition specifier-side owner lookup, specialization selection, binding reconstruction, and nested-location replay | ordinary resolved return type and canonical out-of-class owner results | clean | 4860/4860 | 177398555921 (-0.24%) | 770449408 (+0.47%) | 594251776 (-0.00%) | none |
| Location-only nested class replay | 22/21 | 9/7 | two public location callbacks, token-location traversal, 810 route calls, and 74 duplicate/rejected class-use attempts | structured template-id, template-argument, and AST resolution results | clean | 4860/4860 | 177950357593 (+0.31%) | 766509056 (-0.51%) | 594440192 (+0.03%) | none |
| Lookup-time alias-to-class recovery | 21/20 | 7/7 | post-lookup class extraction, source-token/argument recovery, selection/anchor reconstruction, and 88 rejected attempts | canonical class-template reference results created during type lookup | clean | 4860/4860 | 177232319852 (-0.40%) | 768655360 (+0.28%) | 590512128 (-0.66%) | none |
| Parameter-declaration class rewalk | 20/19 | 7/7 | post-parse parameter AST traversal, source-location recovery, and 84 duplicate/provisional class-use attempts | canonical resolved parameter types and class-template references | clean | 4860/4860 | 177256271702 (+0.01%) | 766726144 (-0.25%) | 590671872 (+0.03%) | none |
| Constant-value qualifier replay | 19/18 | 7/7 | constexpr-only qualifier syntax checks, owner extraction, specialization selection, anchor/binding reconstruction, and 19 staged attempts | ordinary qualified-id/class-template resolution results | pending | pending | pending | pending | pending | pending |

The current promoted checkpoint is within every rolling gate. Relative to the
fixed diagnostic checkpoint it is `-0.06%` instructions, `+0.86%` maximum RSS,
and `-0.65%` footprint, so it is also within all three final fixed-baseline
limits.

## Rejected slices

- `0f6adcf8f` removed the uncalled dependent-partial class-use resolver
  (`24 -> 23` sites). Strict direct comparison and the full `4860/4860`
  report passed, but the single candidate run regressed instructions by 0.15%
  and RSS by 1.29% (footprint improved by 0.02%). The report is
  `/tmp/cppgm-witness-consolidation-candidate-dead-dependent-partial.json`.
  The candidate was not promoted or rerun and was explicitly reverted by
  `1cc9a2e53`.
  After the performance method changed, the same commit was re-evaluated once
  with three runs against the fixed diagnostic baseline. Its medians were
  `177428521256` instructions (`+0.03%`), `769929216` maximum RSS (`+1.28%`),
  and `593932288` footprint (`-0.10%`). It still failed the 1% RSS gate, so the
  revert remains in place. The revised report is
  `/tmp/cppgm-witness-consolidation-candidate-dead-dependent-partial-3run.json`.
