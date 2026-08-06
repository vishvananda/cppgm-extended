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
- Acceptance limits: instructions `+0.5%` and peak footprint `+1%`; maximum
  RSS warns at `+3%` and fails only when one confirmation batch also reaches
  or exceeds `+3%`

The revised baseline deliberately includes the accepted compile-time-guarded
diagnostic instrumentation. The prior parent and exact one-run reports remain
under `/tmp` for audit but do not gate later slices. Do not rerecord the fixed
diagnostic baseline. Promote only an already-recorded three-run candidate that
passes both hard gates and the RSS warning rule.

The revised method requires a regression investigation before rollback. An
unchanged failed commit receives no extra run beyond the automatic RSS
confirmation. A targeted in-scope correction creates a new candidate and must
repeat correctness and performance validation.

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

After the later accepted class-route removals and the `declval` analyzer merge,
the clean refreshed trace is
`/tmp/cppgm-witness-provenance-current-clean.9xqzNA` and its report is
`/tmp/cppgm-witness-provenance-current-clean-report.json`. All 1,305 strict
comparisons remained clean. It contains 117,024 records across 1,296 files,
the consolidated overload-side `declval` producer owns all 62 attempts, and
the removed producer is absent. Six class replay routes remain. The resolved-
type producer made 1,391 attempts and owns no final row uniquely, but it still
supplied a retained occurrence once and received five occurrence replacements;
that route therefore needs a real payload merge before deletion.

After revalidating and promoting every slice that qualifies under the 3% RSS
rule, the refreshed strict trace is
`/tmp/cppgm-witness-provenance-post-revalidation.r9W2RP` and its report is
`/tmp/cppgm-witness-provenance-post-revalidation-report.json`. All 1,305 strict
comparisons remained clean. The trace contains 102,430 records across 1,296
files. The current diagnostic inventory has 38 static producer IDs: 13 class,
4 alias, 3 function-call, 1 variable, and 17 lifecycle sites. Five class replay
routes remain. Every exercised class producer owns visible output except
`class.callsemantic.11`, which supplies five retained occurrence replacements;
it still needs a payload merge before deletion. Every exercised alias and
function-call producer owns visible output. The variable producer owns 31
visible rows and performs one same-producer location/anchor replacement.
Lifecycle collisions remain concentrated between
`constant_value_lookup.03` and `template_argument_semantics.02`, and between
`semantic_class_model` and `template_argument_semantics.02`; those transitions
need ownership analysis before consolidation.

After moving function-result occurrence ownership into the canonical template
source scan and deleting resolved-type-node replay, the refreshed strict trace
is `/tmp/cppgm-witness-provenance-resolved-replay-removed.znfTuD` and its
report is
`/tmp/cppgm-witness-provenance-resolved-replay-removed-report.json`. All 1,305
strict comparisons remained clean. The trace contains 101,745 records across
1,296 files. The diagnostic inventory now has 36 static producer IDs: 12
class, 4 alias, 3 function-call, 1 variable, and 16 lifecycle sites. Four
class replay routes remain. `class.callsemantic.11` and the resolved-type-node
route are absent; before deletion that route made 415 calls, while its producer
owned no visible row uniquely. Every exercised remaining class producer owns
visible output uniquely.

A broader diagnostic reachability probe compiled 2,473 PA15-PA35 test sources
successfully with witness capture and produced 421,584 provenance records in
`/tmp/cppgm-witness-provenance-all-tests-probe.2HFlMJ`. It is not a correctness
gate, because it also invoked tests that have no witness reference, but it
distinguishes dormant sites from rare live sites. In particular,
`lifecycle.template_api.06` produced 83 anonymous-member-class events and must
remain. The two string-based constant-value class-use sites still produced no
attempts in either this probe or the strict corpus, so their post-lookup owner,
source, selection, and binding reconstruction was removed. The current static
inventory is 34 producer IDs: 10 class, 4 alias, 3 function-call, 1 variable,
and 16 lifecycle sites.

The dependent partial-specialization branch in the canonical class-reference
path likewise made no attempt in either provenance corpus. Removing it also
deleted its private parameter-name and binding canonicalization machinery.
The current inventory is 33 producer IDs: 9 class, 4 alias, 3 function-call,
1 variable, and 16 lifecycle sites. Every remaining class producer is
exercised by the strict corpus and owns visible output uniquely.

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

Sites not exercised by the refreshed strict witness references are tracked
until they are reached by an earliest-owning-PA reducer or removed as proven
dead/redundant code:

- alias: `template_specialization.01`;
- lifecycle: `template_api.06`, `template_argument_semantics.01`, and
  `constant_value_lookup.01`.

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
| Constant-value qualifier replay | 19/18 | 7/7 | constexpr-only qualifier syntax checks, owner extraction, specialization selection, anchor/binding reconstruction, and 19 staged attempts | ordinary qualified-id/class-template resolution results | clean | 4860/4860 | 177064422264 (-0.11%) | 766369792 (-0.05%) | 591138816 (+0.08%) | none |
| Static-member initializer tail scan | 18/17 | 7/6 | public after-location callback, token scan after initializer start, and 13 calls that produced no class-use attempt | structured initializer expression semantics | clean | 4860/4860 | 177151210925 (+0.05%) | 765370368 (-0.13%) | 590516224 (-0.11%) | none |
| Out-of-class owner replay | 17/16 | 6/6 | declaration-collector specifier-side owner lookup, specialization selection, binding reconstruction, and an unexercised class-use producer | canonical out-of-class owner emission after method binding resolution | clean | 4860/4860 | 176487122962 (-0.13%) | 759590912 (+1.17%) | 590536704 (+0.01%) | none |
| Lookup-time concrete class-use replay | 16/15 | 6/6 | post-lookup class extraction, source-token and argument recovery, selection and anchor reconstruction, and an unexercised producer | canonical class-template references created during type lookup | clean | 4860/4860 | 176973791780 (+0.00%) | 762613760 (-0.80%) | 590491648 (-0.06%) | none |
| Declaration resolved-type replay | 15/15 | 6/5 | declaration-type replay route and its semantic-context plumbing | retained function-result replay for functional-cast occurrence metadata | clean | 4860/4860 | 176930812572 (-0.02%) | 761339904 (-0.17%) | 589987840 (-0.09%) | none |
| Class base-clause reconstruction | 15/14 | 5/5 | direct class-template lookup, argument resolution, source-decision reconstruction, and a dormant producer during a base-clause rewalk | canonical base-type resolution during class-template instantiation | clean | 4860/4860 | 176608992504 (-0.05%) | 766824448 (+0.73%) | 590385152 (+0.02%) | none |
| Dead dependent-partial resolver | 14/13 | 5/5 | uncalled `record_dependent_partial_class_use_for_resolved_template_id` and its dormant producer | live selected class-reference resolution | clean | 4860/4860 | 176796004199 (+0.11%) | 753594368 (-1.73%) | 590069760 (-0.05%) | none |
| Resolved type-node class replay | 13/12 | 5/4 | public resolved-type callback, 415 strict-corpus calls, post-resolution source scans, and replay-only `sizeof`, qualifier, conversion, declaration, and constructor-initializer helpers | canonical template-id source scanning and selected class-reference occurrence metadata | clean | 4860/4860 | 176684351239 (-0.21%) | 756359168 (-1.11%) | 590614528 (+0.01%) | none |
| Dormant string-based constexpr owner replay | 12/10 | 4/4 | two unexercised post-lookup class-owner reconstruction sites in the string-based constant-value fallback | structured constant-expression and qualified-id resolution paths | clean | 4860/4860 | 176822455088 (+0.08%) | 769830912 (+1.78%) | 590475264 (-0.02%) | none |
| Dormant dependent-partial class-reference branch | 10/9 | 4/4 | dependent partial-specialization source-use reconstruction plus private parameter-name and binding canonicalization | live canonical class-reference producer | clean | 4860/4860 | 176855672482 (+0.02%) | 761241600 (-1.12%) | 590778368 (+0.05%) | none |

The current promoted checkpoint is within every rolling gate. Relative to the
fixed diagnostic checkpoint it is `-0.29%` instructions, `+0.14%` maximum RSS,
and `-0.63%` footprint, so it also clears the final fixed-baseline gates.

## Other source-use slice ledger

| Slice | Kind/sites before/after | Semantic route removed | Responsibility moved to | Strict | Full report | Instructions | Max RSS | Footprint |
| --- | --- | --- | --- | --- | --- | ---: | ---: | ---: |
| Duplicate `declval` analyzer | function call 4/3 | callsemantic-side recognition, type resolution, expression construction, and function-call emission | canonical overload-expression analyzer and its selected-call result | clean | 4860/4860 | 176724057359 (-0.24%) | 750813184 (-1.90%) | 590462976 (-0.01%) |
| Recursive nested-alias replay | alias use 7/6 | nested template-id traversal, alias lookup, argument resolution, source reconstruction, and 34 colliding attempts | canonical direct alias producer and callsemantic alias path | clean | 4860/4860 | 177114959852 (+0.36%) | 761901056 (+0.30%) | 590557184 (+0.00%) |
| Base-clause alias replay | alias use 6/5 | base-clause alias reconstruction, its unused emission-origin branch, and its provenance entry | canonical alias-reference producers reached during base-clause resolution | clean | 4860/4860 | 176968071900 (-0.08%) | 768757760 (+0.90%) | 590872576 (+0.05%) |
| Text-backed partial-match alias replay | alias use 5/4 | post-deduction syntax walk, alias lookup, source-argument recovery, and text-only pack-binding reconstruction | structured alias-template-id expansion and ordinary resolved alias uses | clean | 4860/4860 | 176707655948 (-0.13%) | 757301248 (-0.53%) | 590295040 (+0.05%) |
| Eager child-`declval` replay | function call 3/3 | pre-analysis callee and argument walk plus repeated child expression construction solely for witness capture | ordinary recursive argument expression analysis | clean | 4860/4860 | 176703697462 (-0.00%) | 761233408 (+0.52%) | 590262272 (-0.01%) |

## Lifecycle slice ledger

| Slice | Lifecycle sites before/after | Transition path removed | Responsibility moved to | Strict | Full report | Instructions | Max RSS | Footprint |
| --- | --- | --- | --- | --- | --- | ---: | ---: | ---: |
| Dead value-binding closure hook | 17/16 | uncalled public `note_value_binding_closure_event` API and its unexercised producer | existing variable-instantiation transition owners | clean | 4860/4860 | 177058170973 (+0.15%) | 764841984 (+1.49%) | 590528512 (+0.08%) |
