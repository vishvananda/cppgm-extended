# Witness Convergence Reset and Recovery Plan

## Status and authority

This plan resets execution of the active witness cleanup after the regenerated
strict corpus exposed both new coverage gaps and regressions in the unfinished
alias/class changes. It governs the remaining work in:

- `docs/witness-alias-semantic-convergence-plan.md`;
- `docs/witness-class-materialization-semantic-ownership-plan.md`;
- the function-call, variable-use, and lifecycle gaps exposed by the expanded
  patched-Clang reference corpus.

The earlier plans remain the architectural specifications for their event
families. When their recorded completion state conflicts with the evidence in
this document, this document controls. In particular, neither active plan is
complete, and inception remains forbidden until every final gate here passes.

- Worktree: `/Users/vishvananda/cppgm-alias-consolidation`
- Branch: `experiment-witness-alias-path-consolidation`
- Audit date: 2026-08-08
- Clean control: `5add5290c69be6b76138dfc1f6696915eb0278ae`
- Required host compiler: `/usr/local/opt/llvm/bin/clang++`
- Patched-Clang reference compiler:
  `/Users/vishvananda/llvm-project-template-metrics-20260416/build-clang-template-trace/bin/clang++`
- Patched LLVM checkout: `59c5d9c...`

The branch has advanced past the rejected experiment described below. The
active uncommitted implementation and its evidence belong to the current
work. Preserve them. Do not reset the worktree to `b03f2530d` or the clean
control. Read `handoff.md` before editing; it records the current build,
focused results, reference changes, known defects, and next steps. The
following "Current decision" section remains as recovery history for the
rejected typedef-deferral experiment.

## Semantic-argument checkpoint, 2026-08-09

The current Phase 3 slice is a promotable intermediate checkpoint, not Phase 3
completion. It consolidates alias source-occurrence arguments and ownership,
restores source traversal order and macro provenance, narrows class
materialization admission, and repairs the direct-materialization constructor
profile exposed by the broad gate.

Correctness evidence from the Homebrew-Clang build in
`obj/witness-recovery-alias-semantic-arguments-20260809`:

- the original tracked strict manifest passes 1,305/1,305;
- the expanded strict corpus passes 1,391/1,530, improving the preceding
  1,386/1,530 checkpoint by five outputs with no new mismatch;
- the canonical PA1-PA38 report passes 4,862/4,862;
- the witness analyzer, matcher, normalization, and performance unit suites
  pass 96/96;
- the apparent PA22 direct-LowIR complaint is a harness artifact: the generated
  `300-member-operator-template-active-owner.my.witness.lowir` is byte-identical
  to its `.ref` output.

The final expanded report is
`/tmp/cppgm-recovery-final-strict-20260809.json`, SHA-256
`ef8139da4804ce8963e74a55cbafff513665c86c4be51982813580a8512b03ee`.
The preserved 1,305-reference manifest is
`/tmp/cppgm-original-1305-refs.txt`, SHA-256
`c4a6fa94406234ddfa02c568c04e3026afdb6a00d5eaa69e832e8fcedfb6b536`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.87% | -1.24% | -4.39% | `/tmp/cppgm-alias-convergence-checkpoint-20260809.json` |
| Class-materialization rolling baseline | -1.13% | -0.06% | -1.57% | `/tmp/cppgm-class-materialization-ownership-checkpoint-20260809.json` |

The report SHA-256 values are respectively
`7232cc723e00335f133b973813f6527ae6a02bdafb869986cde77d90d8146985`
and
`5177ae085a178c3c079b87752be704e7172ba998cdd250ba95f0349c7144ccf7`.
The instruction and memory reductions permit this slice to become the next
rolling checkpoint. The remaining 139 expanded mismatches, repeated alias
completion scaffolding, and second source-pattern parameter analysis remain
Phase 3 debt; inception is still forbidden.

## Member-template source ownership checkpoint, 2026-08-10

This is another promotable intermediate Phase 3 checkpoint, not Phase 3
completion. It removes the second semantic template-parameter analysis from
generic class-member source scanning. Member function templates now receive an
early lexical, alias-only source walk; their typed class and type semantics stay
with the canonical function-template collector. Structured identifier and type
alias facts preserve outer class-template dependency without reparsing template
parameters or template-argument source text. Member variable templates retain
the lexical source walk needed for their distinct source-use behavior.

The alias completion boundary now checks stable source-syntax identity before
replaying source-witness completion. The declaration source walker is the sole
recursive owner of nested alias occurrences, while repeated SFINAE
instantiations still perform their required semantic work. Provenance across all
1,530 tests records:

- 835 alias candidates, 835 collected occurrences, and 835 published
  occurrences, with zero early repeats and zero prepublication merges;
- zero repeated central semantic-completion replays, down from 1,279;
- 376 nested source-walk visits covering 36 unique occurrences, with 340
  revisits explicitly classified and ignored before semantic resolution;
- 835 first completions split into 144 concrete, 375 parameterized, and 316
  source-collection operations.

Correctness evidence from the final Homebrew-Clang ordinary and provenance
builds:

- the original tracked strict manifest remains byte-exact at 1,305/1,305;
- the expanded strict corpus passes 1,392/1,530, fixing the member-variable
  template alias case and leaving 138 mismatches with no new mismatch family;
- the canonical PA1-PA38 report with direct LowIR comparison passes
  4,862/4,862;
- the focused convergence, provenance, materialization, text-reparse, and path
  normalization unit suites pass 42/42;
- both materialization decision boundaries pass audit with no finding, and all
  23 forbidden text-reparse categories remain zero;
- ordinary and provenance builds produce no warning.

Full repository unit discovery ran 249 tests with one skip and one unrelated,
reproducible error in
`BatchTimeoutHarnessTests.test_driver_assignment_wrapper_uses_worker_script`:
its temporary `basic.my.impl.exit_status` file is absent. The focused suites
covering this checkpoint all pass; the independent batch-timeout harness defect
is not treated as evidence against this semantic checkpoint.

Removing the duplicate semantic pre-walk also changes only private LowIR
constructor suffix allocation (`__ov2`/`__ov3`) in
`pa22/tests/spec/300-member-operator-template-active-owner.ref`. Calls,
function bodies, Itanium object symbols, runtime behavior, and witness output
are unchanged. The fixture now records the canonical no-witness build order,
and PA22 passes 308/308 with direct comparison.

The final provenance analysis is
`/tmp/cppgm-member-source-owner-provenance.rl3vcg/provenance-analysis.json`,
SHA-256
`ba716077aff8d74c252a0e1330b8b4f4c352fee715d739017269439486707a31`.
Its convergence report is
`/tmp/cppgm-member-source-owner-provenance.rl3vcg/convergence.json`, SHA-256
`2462ecf7ca21f3f29754fbcc0193c8658ece452f535dbf1b1eced11c9e3aebe1`.
The broad report is
`/tmp/cppgm-member-source-owner-broad-final.nBnyF1/report.log`, SHA-256
`52e3ef7bea1a674d8e1a508d626e741bc51250698564a5d352e5be97a0d6ab8c`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.33% | -0.51% | -4.04% | `892a7a2aacc436789d20566c3ae7fae03aef005dcd746080dcd9d20cf5363fef` |
| Prior rolling baseline | +0.30% | +1.39% | +0.39% | `a58802cab06702a55030977971dcf0f46a09c2fe55e8c9a60dfbbea64fc51c55` |

The reports are
`/tmp/cppgm-member-source-owner-vs-fixed-20260810.json` and
`/tmp/cppgm-member-source-owner-vs-rolling-20260810.json`. Their candidate
metadata names the preceding commit because the measurements cover this
uncommitted worktree immediately before its checkpoint commit.

Phase 3 remains open: the nested traversal still performs 340 guarded revisits,
the completed-occurrence set remains an ownership registry, and 138 expanded
class-use, function-call, and lifecycle mismatches remain. Inception is still
forbidden.

## Out-of-class owner presence checkpoint, 2026-08-10

This promotable Phase 3 checkpoint moves out-of-class class-use presence onto
typed declaration and materialization facts. Out-of-class member-template and
static-member collection now retain the source class-template declaration,
the structured source template-id, and, when semantic lookup succeeds, the
immediate member-owner template. Replayed nested static definitions match the
concrete owner through the canonical class-template declaration identity; they
do not recover the owner from rendered template names or reparsed source text.

Static-definition owner occurrences are collected before their storage is
necessarily required, but publication is now gated by the variable
instantiation transition for the same semantic owner type. A more-specific
nested owner replaces an enclosing-owner candidate for the same source
occurrence. This separates source discovery from the semantic fact that makes
the row visible and prevents unused static definitions from creating public
class-use rows.

The expanded corpus improves from 1,392/1,530 to 1,398/1,530 with no added or
changed mismatch. Ten missing class-use occurrences are restored across the
following source-owner families:

- nested static members in PA19 and PA20;
- nested methods, constructors, address-pack statics, and member-template
  statics in PA22;
- all four partial-owner `traits<A, false>` occurrences in PA24.

Six outputs become exact. The PA24 partial-owner fixture retains only its two
pre-existing function-call drop mismatches. The class-use inventory falls from
56 failing tests and 31 missing rows to 49 failing tests and 21 missing rows;
the 54 changed rows, two unexpected rows, and two ordering-only cases are
unchanged. The full convergence result is 132 remaining mismatches: 49
class-use tests, 60 function-call tests, and 48 lifecycle tests, with overlap
between families.

Correctness evidence from the final ordinary and provenance Homebrew-Clang
builds:

- the original tracked strict manifest remains byte-exact at 1,305/1,305;
- the expanded strict corpus passes 1,398/1,530;
- ordinary and provenance builds produce identical 3,060-file output
  manifests and no warning;
- all 1,530 provenance sessions flush, producing 61,316 records with no
  unknown producer and no unexercised producer site;
- class provenance records 2,613 attempts, 2,612 insertions/public rows, and
  the one known exact nested-source duplicate;
- the canonical PA1-PA38 report with direct LowIR comparison passes
  4,862/4,862;
- the focused convergence, provenance, materialization, text-reparse, and path
  normalization suites pass 42/42;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The final convergence report is
`/tmp/cppgm-class-presence-convergence-final-20260810.json`, SHA-256
`cae0bb144a284194a84f70332c492354fa856316dc797494ecf38de76625f2e5`.
The provenance analysis and convergence reports are
`/tmp/cppgm-class-presence-provenance.5Y5C3C/provenance-analysis.json` and
`/tmp/cppgm-class-presence-provenance.5Y5C3C/convergence.json`, with SHA-256
values `657b9d7e64ec787908a29abeda3ddee28dd4765b04f193a89b75d633e4981309`
and `bd1b2b996c875b8ca48ce9623f2b0955cbff317f4010e7a971e8d418ee58d1cc`.
The broad report is `/tmp/cppgm-class-presence-broad-20260810.log`, SHA-256
`b7f26853da7c693a05da4d95b8db38985e9ce06ef3e86b9df24325c311608dad`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.34% | -0.18% | -4.01% | `837f7c4e888052232c18a4e85ff2f814431b1c883034a2db886f66f8ff341c71` |
| Prior rolling baseline | -0.08% | -0.26% | +0.04% | `a69930a1d5b815dfa0b2ec67bc0297ecb36f3f7343a3520eda248583d7f1f505` |

The reports are `/tmp/cppgm-class-presence-vs-fixed-20260810.json` and
`/tmp/cppgm-class-presence-vs-rolling-20260810.json`. Their candidate metadata
names the preceding commit because the measurements cover this uncommitted
worktree immediately before its checkpoint commit.

Phase 3 remains open. Twenty-one missing class-use occurrences still divide
between typed materialization, rooted static, member-default, and related
semantic-owner families; function-call and lifecycle convergence also remain.
Inception is still forbidden.

## Alias declaration-result materialization checkpoint, 2026-08-10

This promotable Phase 3 checkpoint connects named alias and typedef
declarations to the concrete class-template result returned by typed lookup.
The declaration collector records alias identity even when no concrete class
handle exists yet. A later exact leaf lookup supplies the resolved type and
declaring scope, upgrades that identity to the canonical class instance, and
publishes a declaration-type source occurrence only when the declaration is
semantically materialized. This path uses `Scope`, `Type`, `ClassInfo`, and
`ClassTemplateDecl` identity throughout; it does not inspect rendered names,
source lines, or location priorities.

Class member typedefs participate in the same identity registry, and selected
partial-specialization arguments now come from the existing typed
`ClassTemplateUseInfo` selection. Braced initialization remains on its own
typed initializer/materialization path, so aggregate declarations do not also
publish a duplicate alias declaration-type occurrence.

Four previously missing class-use rows are restored:

- the concrete `integral_constant<bool, true>` result of the PA23
  `invocable_impl<...>::type` member alias;
- the partial `graph<...>` result of the PA23 local `graph_type` typedef;
- the partial `async_result<...>` result of the PA24 local `result` typedef;
- the primary `fork_t<0>` result of the PA24
  `relationship_t::fork_t` member typedef.

The `async_result` output becomes exact. The graph and fork fixtures retain
unrelated pre-existing class/lifecycle differences, and the bool fixture now
has the recovered same-location row as an ordering-only difference. Ordering
is deliberately left for a later ordering cluster rather than encoded as a
location or priority exception.

The expanded corpus improves from 1,398/1,530 to 1,399/1,530. Remaining
mismatches fall from 132 to 131. The class-use inventory improves from 49 to 47
failing tests and from 21 to 17 missing rows; its 54 changed rows and two
unexpected rows are unchanged, while ordering-only cases rise from two to
three because of the recovered bool row. Function-call and lifecycle
inventories are unchanged.

Correctness evidence from the final ordinary and provenance Homebrew-Clang
builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- the expanded strict corpus passes 1,399/1,530;
- ordinary and provenance builds produce byte-identical witness and LowIR
  output for all 3,060 compared files and no build warning;
- all 1,530 provenance sessions flush, producing 61,329 records with no
  unknown producer and no unexercised producer site;
- class consolidation records 2,976 completed candidates, 3,300 early
  repeats, 348 prepublication merges, 2,628 collected occurrences, and 2,617
  publications;
- the canonical PA1-PA38 report with direct LowIR comparison passes
  4,862/4,862;
- the focused convergence, provenance, materialization, text-reparse, and path
  normalization suites pass 42/42;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The final ordinary convergence report is
`/tmp/cppgm-alias-result-convergence-final-20260810.json`, SHA-256
`3c39e98b1f437cde3a479359d47b7f796c6af34aab5e28252abef30677372d4d`.
The provenance analysis and convergence reports are
`/tmp/cppgm-alias-result-provenance.5EW1qq/provenance-analysis.json` and
`/tmp/cppgm-alias-result-provenance.5EW1qq/convergence.json`, with SHA-256
values `3ea531e5a255e2317990e84ccc7896a86e5bca3726ad7f67b914071215174e4f`
and `678ece93a2fb7b046bd0b038dd9897998d9c1aed18ec396516e02f833e0183df`.
The broad report is `/tmp/cppgm-alias-result-broad-20260810.log`, SHA-256
`52e3ef7bea1a674d8e1a508d626e741bc51250698564a5d352e5be97a0d6ab8c`.
The materialization audit is
`/tmp/cppgm-alias-result-materialization-audit-20260810.json`, SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.27% | -1.14% | -3.99% | `d0d7fa373026b5846229821d20b470ec90da40cd650116fd0499e5a0fb9a15a4` |
| Prior rolling checkpoint | +0.07% | -0.96% | +0.03% | `9be4538f2a18947695aac472d0af255c90265dd5d0c6b20465b61ed913b46f1f` |

The reports are `/tmp/cppgm-alias-result-vs-fixed-20260810.json` and
`/tmp/cppgm-alias-result-vs-rolling-20260810.json`. Their candidate metadata
names the preceding commit because the measurements cover this uncommitted
worktree immediately before its checkpoint commit.

Phase 3 remains open. Seventeen missing class-use rows, the same-location bool
ordering case, function-call convergence, and lifecycle convergence remain.
Inception is still forbidden.

## Cv-qualified variable construction checkpoint, 2026-08-10

This promotable Phase 3 checkpoint connects direct class declaration types to
the patched-Clang oracle's deduced variable-construction occurrence.
`TemplateWitnessVisitor::VisitVarDecl` emits that second occurrence
when a variable initializer is a `CXXConstructExpr` and the declaration's
`TypeLoc` is not itself a direct template-specialization location. A top-level
cv wrapper creates that shape: the written template-id remains an
explicit class-use, while the semantic construction contributes a deduced use
at the same template-id anchor.

The compiler retains the source occurrence ID with the existing typed
class-result capture. A non-braced, cv-qualified object declaration with an
initializer publishes the additional materialized use from the canonical
`ClassInfo` and `ClassTemplateDecl` result. Alias declarations keep their
existing materialization behavior, and braced initialization stays on its
separate typed path. The implementation does not inspect a source line,
rendered name, fixture identity, or location priority.

This checkpoint restores three missing class-use rows:

- both `constexpr U<int>` variables in the PA23 constexpr-union fixture;
- the `const result<int>` variable in the PA24 conversion-function-template
  top-cv fixture.

The two fixtures become exact. Expanded convergence improves from
1,399/1,530 to 1,401/1,530 because the two PA23 rows share one output.
Remaining mismatches fall from 131 to 129. The class-use inventory improves
from 47 to 45 failing tests and from 17 to 14 missing rows; its 54 changed
rows, two unexpected rows, and three ordering-only cases are unchanged.
Function-call and lifecycle inventories are unchanged.

Measurements rejected two broader policies before promotion. Treating
generic qualified-value queries as exact source-type materialization produced
101 unrelated implicit query rows and regressed convergence to 1,353/1,530.
Admitting every exact source node with a committed semantic owner produced
304 unexpected class rows across 217 tests and regressed convergence to
1,251/1,530. The accepted patch excludes both experiments. Source-node
exactness and semantic-owner commitment cannot distinguish public source
occurrences when implicit instantiation reuses an AST. The accepted path uses
the typed variable-declaration result visited by the oracle.

Correctness evidence from the final ordinary and provenance Homebrew-Clang
builds:

- the preserved original strict witness manifest remains byte-exact at
  1,305/1,305;
- the expanded strict corpus passes 1,401/1,530 with 129 remaining mismatches;
- ordinary and provenance builds produce byte-identical witness and LowIR
  output for all 3,060 compared files and no build warning;
- all 1,530 provenance sessions flush, producing 61,338 records with no
  unknown producer and no unexercised producer site;
- class consolidation records 2,981 completed candidates, 3,300 early
  repeats, 350 prepublication merges, 2,631 collected occurrences, and 2,620
  publications;
- the canonical PA1-PA38 report with direct LowIR comparison passes
  4,862/4,862;
- the focused convergence, provenance, materialization, text-reparse, and path
  normalization suites pass 42/42;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The final ordinary convergence report is
`/tmp/cppgm-cv-construction-convergence-final-20260810.json`, SHA-256
`177db6a42cc80861f04d1cfa52afa87c361d166e19ab61f530e312e5c42d06bc`.
The provenance analysis and convergence reports are
`/tmp/cppgm-cv-construction-provenance.acZJxc/provenance-analysis.json` and
`/tmp/cppgm-cv-construction-provenance.acZJxc/convergence.json`, with SHA-256
values `50f532221619ef9049d892dca0c3d5e166e6186f0790a1b30353ae64a61bb5c9`
and `599b7b475cc47e6425fd80efc1c1782c246ede294a675c66d8af708c947efef3`.
The broad report is `/tmp/cppgm-cv-construction-broad-20260810.log`, SHA-256
`ef0e6e0c7c8e6a91152de872101a331af400a4b326fd68589be528a4cb345d5f`.
The materialization audit is
`/tmp/cppgm-cv-construction-materialization-audit-20260810.json`, SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.30% | -0.42% | -3.95% | `4a4a7a2d616d7756528b36d06735bfc375c3429b82236c43e7bfbcf3bf8eba06` |
| Prior rolling checkpoint | +0.04% | -0.24% | +0.07% | `d45a67fbcd31e32bedd20a41546f98bd3e6537606980e0805de355e38b73f6fc` |

The reports are `/tmp/cppgm-cv-construction-vs-fixed-20260810.json` and
`/tmp/cppgm-cv-construction-vs-rolling-20260810.json`. The shared raw
three-run candidate is
`/tmp/cppgm-cv-construction-raw-candidate-20260810.json`, SHA-256
`bbcf0057db91a2322fecbb39a153e11efebfc613bfc1055ce1d5b3c2ca9bfbf0`.
Their candidate metadata names the preceding commit because the measurements
cover this uncommitted worktree immediately before its checkpoint commit.

## Static-member initializer replay checkpoint, 2026-08-10

The out-of-class static-member replay now revisits a general initializer when
a semantic value acquisition requires that definition. The replay uses the
retained AST, the concrete member binding, and its bound instantiation scope.
It suppresses output materialization while running normal expression
semantics. Aggregate and function-pointer initializers keep their existing
typed paths.

Header and output scans can request storage without acquiring the value. The
member-value transition now promotes an active public semantic use to an
explicit initializer-replay fact. Retained dependencies and output-only
visits leave it false. A separate one-time bit lets a later value acquisition
revisit an initializer after an earlier output visit recorded the definition
owner. This boundary prevents an unused `keyword<Tag>::instance` definition
from acquiring a constructor occurrence. The decision uses semantic
acquisition state, not a fixture, identifier, source line, or location
ordering.

This checkpoint restores three class-use rows:

- the constructed `n::keyword<tag::color_map>` initializer in the rooted PA22
  static definition;
- both `bucket_array_base<true>` qualifiers recovered from PA24 `sizeof`
  type-id alternatives.

The class-use inventory improves from 14 to 11 missing rows. Its 45 failing
tests, 54 changed rows, two unexpected rows, and three ordering-only cases are
unchanged. Expanded convergence remains 1,401/1,530 because both affected
outputs retain earlier mismatches. No mismatch is added or changed.

Correctness evidence from the final ordinary and provenance Homebrew-Clang
builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- the expanded corpus remains 1,401/1,530, with exactly three missing
  class-use occurrences removed and no added occurrence;
- ordinary and provenance builds produce byte-identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,362 records with no
  unknown producer and no unexercised producer site;
- class consolidation records 2,985 completed candidates, 3,301 early
  repeats, 351 prepublication merges, 2,634 collected occurrences, and 2,623
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, and path suites
  pass 42/42;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The final ordinary convergence report is
`/tmp/cppgm-static-initializer-convergence-final2-20260810.json`, SHA-256
`36305b18333bce7a7fd5c0f3a6dabd23e5be9f2055f836e6475bd4c8223bd500`.
The provenance analysis and convergence reports are
`/tmp/cppgm-static-initializer-provenance-final2.692LsF/provenance-analysis.json`
and
`/tmp/cppgm-static-initializer-provenance-final2.692LsF/convergence.json`, with
SHA-256 values
`d00770bd54a7b28eaa071785ba0c762395c21e50c68deadd3a3a2d71777f10b5` and
`d6e95d33f189c2b5183261829fdd257ca3c1e3fde6cb4f6d64329c445e4a3128`.
The broad report is
`/tmp/cppgm-static-initializer-broad-final2-20260810.log`,
SHA-256
`cd0e33e0b6b496e590a3e05e940de8c07498c14d79d7bfbe8420bbed7a119040`.
The materialization audit remains byte-identical to the prior checkpoint at
SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.26% | -0.63% | -3.99% | `eca2dcce72b871b22d87c38b4f103551137e95c36070418dd06c6f33f0f80baa` |
| Prior rolling checkpoint | +0.04% | -0.20% | -0.05% | `5a49d5566d5ffc0bc62bd3b4fe29e59d3c05f2377033b9f1ce0d3144676d1674` |

The reports are
`/tmp/cppgm-static-initializer-vs-fixed-final2-20260810.json` and
`/tmp/cppgm-static-initializer-vs-rolling-final2-20260810.json`. The shared raw
candidate is
`/tmp/cppgm-static-initializer-raw-candidate-final2-20260810.json`,
SHA-256
`83dd3ca2663e910ed0d440767e060ac6296c43495c64503fa719a772c86d0bd6`.
Its candidate metadata names the preceding commit because the measurements
cover this uncommitted worktree immediately before its checkpoint commit.

Phase 3 remains open. Eleven missing class-use rows still divide between
typed declaration materialization, rooted static definitions, member-template
defaults, nested static owners, and related semantic-owner families.
Function-call, lifecycle, and ordering convergence remain.
Inception is still forbidden.

## Member-template lexical source-scope checkpoint, 2026-08-10

Class-template reference resolution now keeps the scope used to bind a
selected template separate from the scope that resolved its written template
arguments. Member-template lookup can select an instantiated member scope even
when the source occurrence was written at file scope. Source-dependency
classification now uses the argument scope for bound-name, fixed-binding,
current-specialization, dependent-owner, and typed-materialization decisions.
The selected binding scope continues to own instantiation and lookup.

This distinction fixes four class-use rows in
`pa23/tests/general/500-member-template-default-qualified-suffix-owner.t`.
The explicit `begin_iter` argument resolves to the same type as an enclosing
`iterator` specialization inside the selected member-template scope. It is not
a source occurrence of that current specialization because the argument was
written and resolved at file scope. The prior classifier inferred lexical
ownership from the selected scope and suppressed the `defaults` and `equal_to`
uses at lines 48, 49, 50, and 54. The accepted change carries the existing
argument-scope result through the semantic interface. It does not inspect an
alias spelling, a template name, a fixture, or a source location.

The focused PA23 witness is now byte-exact. Expanded convergence improves from
1,401 to 1,402 matching outputs. The class-use inventory falls from 11 to seven
missing rows and from 45 to 44 failing tests. Its 54 changed rows, two
unexpected rows, and three ordering-only cases are unchanged. Function-call
and lifecycle inventories are unchanged.

Correctness evidence from the ordinary and provenance Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- the expanded corpus passes 1,402/1,530 with 128 known mismatches;
- ordinary and provenance builds produce byte-identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,374 records with no
  unknown producer and no unexercised producer site;
- class consolidation records 2,989 completed candidates, 3,302 early
  repeats, 351 prepublication merges, 2,638 collected occurrences, and 2,627
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path, and
  performance unit suites pass 57/57;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The ordinary convergence report is
`/tmp/cppgm-member-default-convergence-20260810.json`, SHA-256
`9f417670b42abecb5f8b70dc74bf8a228dfe482d25691b1cc092ba0689d797fd`.
The provenance analysis and convergence reports are
`/tmp/cppgm-member-default-provenance.iVxiZR/provenance-analysis.json` and
`/tmp/cppgm-member-default-provenance.iVxiZR/convergence.json`, with SHA-256
values `162cf83da992c4237a259953cf5b55f0ed1189032f1bc2046d72bccf2077640f`
and `d6b7b647bbbe33d616e9e23bd1770e49fe554cbb905e41b04612869a41abe2ea`.
The broad report is `/tmp/cppgm-member-default-broad-20260810.log`, SHA-256
`1f5b8ce24acb1ddcb834e86fd67e5581f2505cc5fbc74c9465c8d826384594b6`.
The materialization audit remains byte-identical to the prior checkpoint at
SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.98% | -0.68% | -3.95% | `870be6c2f3da83ebc9951def8c81f2c5c3f5fd41fd250391abc4a2b533aabc47` |
| Prior rolling checkpoint | +0.29% | -0.05% | +0.04% | `26cc77e6b3c1b51b5f305bcd2afa7af605d7cc82ee71480fc69ff724236def28` |

The reports are `/tmp/cppgm-member-default-vs-fixed-20260810.json` and
`/tmp/cppgm-member-default-vs-rolling-20260810.json`. The shared raw candidate
is `/tmp/cppgm-member-default-raw-candidate-20260810.json`, SHA-256
`b51708577c6cc33e40f995a03eea61e3a2b1ca4ca148135ded05daca18ee1a53`.
Its candidate metadata names the preceding commit because the measurements
cover this uncommitted worktree immediately before its checkpoint commit.

Phase 3 remains open. Seven missing class-use rows remain across typed member
declarations, dependent current-instantiation values, rooted static
definitions, nested static owners, and function-result ownership.
Function-call, lifecycle, and ordering convergence remain. Inception is still
forbidden.

## Fixed-member and static-declaration class-use checkpoint, 2026-08-10

Class source materialization now distinguishes fixed member bindings from
bindings that only become concrete during replay. The witness session records
source-template dependency for member typedefs and static values by semantic
owner and member name. Type dependency includes direct template parameters and
transitive member aliases. The legacy fixed-binding path is unchanged; the
extended path admits a dependent source template-id only when every argument
is a fixed source member binding.

Out-of-class static-member reconstruction now carries the resolved declaration
type and its retained template-id syntax. Rooted declaration types produce
their explicit and materialized class-use occurrences, while nested owner
arguments are paired with their semantic template arguments recursively. This
uses retained AST and semantic types. It does not reparse source text or filter
by fixture, template name, or source location.

The checkpoint restores five of the seven missing class-use rows:

- `lock<int>` from a fixed member typedef;
- `bool_constant<false>` from a fixed current-instantiation static value;
- the explicit and materialized `n::keyword<tag::color_map>` declaration-type
  occurrences in the rooted static definition;
- the nested `bytes<4, 7>` occurrence in the static data-member definition.

The lock and `bool_constant` witnesses are byte-exact. Rooted static class-use
presence is exact, but that fixture retains a lifecycle mismatch. The bytes
occurrence is present, but its non-type argument rendering remains changed.
The two `graph` function-result occurrences remain missing.

Expanded convergence improves from 1,402 to 1,405 matching outputs and from
128 to 125 known mismatches. The class-use inventory falls from seven to two
missing rows and from 44 to 41 failing tests. It has 55 changed rows, two
unexpected rows, and three ordering-only cases. Function-call and lifecycle
inventories are unchanged.

Correctness evidence from the ordinary and provenance Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- the expanded corpus passes 1,405/1,530 with 125 known mismatches;
- ordinary and provenance builds produce byte-identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,389 records with no
  unknown producer and no unexercised producer site;
- class consolidation records 2,997 completed candidates, 3,303 early
  repeats, 354 prepublication merges, 2,643 collected occurrences, and 2,632
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, and semantic-boundary unit suites pass 57/57;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The ordinary convergence report is
`/tmp/cppgm-class-presence-convergence-final8-20260810.json`, SHA-256
`3d026293f60ab37fe742bbb06a6e03eb5dd0237f15c4368301283a28b90e2a8f`.
The provenance analysis and convergence reports are
`/tmp/cppgm-class-presence-provenance-final.pt0aG2/provenance-analysis.json`
and
`/tmp/cppgm-class-presence-provenance-final.pt0aG2/convergence.json`, with
SHA-256 values
`57d77a843c2b4c6cf051ba8145ce24958c44c808eee430a3b5acb6e556c6a9d8`
and
`03f775e380113b59e0894cf91ba2a861a8103dfeb9327322f1ef2aa3e3ce1032`.
The broad report is `/tmp/cppgm-class-presence-broad-20260810.log`, SHA-256
`40fc8a1fd12832f2b6b066a51229984981f2dc4ce6934d0feca0f57da0fb904d`.
The materialization audit remains byte-identical to the prior checkpoint at
SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.08% | -0.07% | -3.97% | `2fc0b6a0b0f4e89b08e0f7345b9add91f194cf4454f1a58981df5e8a1d08d663` |
| Prior rolling checkpoint | -0.10% | +0.60% | -0.01% | `96e34b2a746326c67fb7408e8a7348bfe70a0d9bf1f7b570eb39363b080b70ba` |

The reports are `/tmp/cppgm-class-presence-vs-fixed-final-20260810.json` and
`/tmp/cppgm-class-presence-vs-rolling-final-20260810.json`. The shared raw
candidate is
`/tmp/cppgm-class-presence-raw-candidate-final-20260810.json`, SHA-256
`3b2ea968461a61a7396e73b20ad59563d86acfd87eefb7d74946e26c7afae4ec`.
Its candidate metadata names the preceding commit because the measurements
cover this uncommitted worktree immediately before its checkpoint commit.

Phase 3 remains open. Two missing class-use rows remain in the function-result
owner path for the `graph` fixture. Function-call, lifecycle, payload, and
ordering convergence remain. Inception is still forbidden.

## Current-specialization result-owner class-use checkpoint, 2026-08-10

Concrete class collection now carries the selected `ClassInfo` through source
type resolution for deferred member aliases. The owner frame uses the typedef
specifier sequence or alias type-id as its source root, so the frame covers the
written template-id instead of the declarator that names the alias.

The class-template reference path replays a retained current-specialization
use only when the source pattern remains dependent, the committed declaration
owner matches the selected class instance by identity, and the active AST root
contains the source occurrence. The replay resolves partial-specialization
arguments in their pattern scope, keeps the concrete member scope as the
observation owner, and preserves Clang's trailing primary-default bit. A
single template API predicate checks owner kind, commitment, and identity.
The implementation does not parse source text or filter by template name,
fixture, or source location.

This checkpoint restores both `graph` class-use rows at lines 41 and 45 in
`pa23/tests/spec/500-function-result-template-id-shadowed-argument.t`. The
CPPGM witness matches the reference byte for byte, including the explicit
partial parameters and the defaulted sixth binding.

Homebrew Clang produced the following correctness evidence:

- the preserved original strict manifest matches 1,305/1,305 outputs;
- the expanded corpus matches 1,406/1,530 outputs, with 124 known
  mismatches;
- the class-use inventory has zero missing rows, 55 changed rows, two
  unexpected rows, three ordering cases, and 40 failing tests;
- function-call and lifecycle inventories match the preceding checkpoint;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,395 records with no
  unknown producer and no unexercised producer site;
- class consolidation reports 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper unit suites pass
  60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The standalone semantic/template boundary ratchet repeats the parent
checkpoint's legacy counts: five output-readiness queries, ten template
service mentions, and nine internal-header sites. Its unit tests pass. This
ratchet is not an acceptance gate for the current witness slice.

The ordinary convergence report is
`/tmp/cppgm-current-result-owner-convergence-final-20260810.json`, SHA-256
`eacbfc9c2dbb9976ec9af2c2e832ca6eec410e13edab554f169f3be0afcfcd61`.
The provenance analysis and convergence reports are
`/tmp/cppgm-result-owner-provenance.7djiaJ/provenance-analysis.json` and
`/tmp/cppgm-result-owner-provenance.7djiaJ/convergence.json`, with SHA-256
values
`802c02c99441b0eb536bd2f9f5c8a17d3c4460364547bfca82b7f5cd2b1cfe28`
and
`80adc43c9e39d0316890c142c1bc8219b724e119da4ba5a74c01b6950f67acce`.
The broad report is
`/tmp/cppgm-current-result-owner-broad-20260810.log`, SHA-256
`e8c46dce5f21842ef075303de56f5b9cab812f3beb2e3c56bf65be9ad6cf6be9`.
The materialization audit remains byte-identical to the prior checkpoint at
SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both required three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.01% | +0.22% | -3.92% | `aa6e0f76b79c17415d3f1e539c3aee512fd3a7b879b7b4af604277da16e50bcc` |
| Prior rolling checkpoint | +0.07% | +0.30% | +0.05% | `f98a8d425066c47d23624ab3afb2c0ec0b20687af7e4778f166bac9dc70f96df` |

The reports are `/tmp/cppgm-current-result-owner-vs-fixed-20260810.json`
and `/tmp/cppgm-current-result-owner-vs-rolling-20260810.json`. The shared
raw candidate is
`/tmp/cppgm-current-result-owner-raw-candidate-20260810.json`, SHA-256
`7b08c5047098ac974d25de208462e2c81329e303c2bb913b3d527ce4567edc7c`.
Its metadata names the preceding commit because the measurements cover this
uncommitted worktree before the checkpoint commit.

Phase 3 remains open. The class-use family has no missing rows, but 55 changed
and two unexpected class rows remain. Function-call, lifecycle, payload, and
ordering convergence also remain. Inception is still forbidden.

## Selected partial pack partition checkpoint, 2026-08-10

Partial-specialization selection already retained the exact number of
arguments deduced for each parameter pack, including zero-length packs. Class
witness construction discarded that map and divided the flattened argument
vector greedily: the first pack consumed every argument not reserved for a
later non-pack parameter, leaving later packs empty.

The shared source-binding builder now accepts the selected pack-size map. It
looks up the current parameter by name or placeholder key and consumes exactly
that many flattened arguments. When selection has no size for a pack, the
existing trailing-non-pack fallback remains in force. Resolved source uses,
materialized alias class uses, and out-of-class static declaration uses pass
the committed selection map; unrelated binding paths keep their prior
behavior.

This clears six class-use rows across five tests. The repaired cases cover two
adjacent non-type/type packs, repeated packs, an empty prefix pack, a recursive
pack with a fixed middle parameter, and a bound template-template application.
Four complete outputs leave the failure manifest. The recursive-pack test
still has its preexisting lifecycle mismatch, although its class-use row is
now exact.

Correctness and diagnostic evidence from fresh isolated Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence improves from 1,406 to 1,410 matching outputs, leaving
  120 known mismatches and no new failing output;
- the class-use inventory falls from 55 to 49 changed rows and from 40 to 35
  failing tests; it still has zero missing rows, two unexpected rows, and
  three ordering cases;
- function-call and lifecycle inventories are unchanged;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,389 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper unit suites pass
  60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The template-side boundary ratchet is byte-identical to the parent checkpoint.
The semantic-side audit retains the parent's five output-readiness queries,
ten template-service mentions, and nine internal-header sites. These existing
counts are not acceptance gates for this payload slice.

The ordinary convergence report is
`/tmp/cppgm-pack-partition-convergence-20260810.json`, SHA-256
`b6d5213cb50d4d5bbfc16990ea7ffb9c53be1b4b7c63779cbf9b56538f52130d`.
The provenance analysis and convergence reports are
`/tmp/cppgm-pack-partition-provenance.8Ew1PD/provenance-analysis.json` and
`/tmp/cppgm-pack-partition-provenance.8Ew1PD/convergence.json`, with SHA-256
values
`937606185e5736fbac472286be055887e1e4b0595f52063a42a047c0c1bbdc7f`
and
`4a6a9c4c85fc28ee6af88c43c685dc09190b285a114250cb54fa1c4e58720888`.
The byte-identical ordinary/provenance output manifest has SHA-256
`5fbf777c8c3f4728ed4078192c2a4b51fd9cb5c9e82144121bd5e1ebd007b34c`.
The broad report is `/tmp/cppgm-pack-partition-broad-20260810.log`, SHA-256
`b16a58f16ae10a73ae7fe6d1e12686d41cb68e6e3c4d696e65b00fa323ff57ff`.
The materialization audit remains byte-identical to the preceding checkpoints
at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.02% | -1.14% | -3.96% | `3450887838108377c9b5eafdea6019b7e7160bb4849f49e5527b59ee3932bc43` |
| Prior rolling checkpoint | -0.01% | -1.36% | -0.04% | `9693d5cb598951f5bb8a489c2ecbc142c67841244db3e6a29289ab2dafe58b52` |

The reports are `/tmp/cppgm-pack-partition-vs-fixed-20260810.json` and
`/tmp/cppgm-pack-partition-vs-rolling-20260810.json`. The shared raw candidate
is `/tmp/cppgm-pack-partition-raw-candidate-20260810.json`, SHA-256
`0338a1c39d2a99116b2d06c56e695ae3fbee55fb0bbc1bbf6420d41e34970f70`.
Its metadata names commit `28950ee69` because the measurements cover this
uncommitted checkpoint.

Phase 3 remains open. The next class payload work must recover structured
argument spelling, binding-source labels, selection payloads, and ordering.
The two unexpected class rows also remain. Inception is still forbidden.

## Structured class argument spelling checkpoint, 2026-08-10

Class source bindings carried only a broad `type_like` bit into the renderer.
The renderer consequently had to infer function declarator layout from text,
and its general type normalizer discarded semantic array and cv spellings.
That lost the space before function parameter lists, shortened `unsigned int`
inside arrays, and collapsed `const volatile` fundamental types to their
written source order.

The source-binding carrier now records two narrower semantic facts: whether
the argument is a function type, and whether its structured semantic spelling
must survive rendering. Explicit array arguments and `const volatile`
fundamental arguments use the semantic type printer. Function-type arguments
use their typed status to restore the parameter-list space after identifier or
template-id results while leaving pointer results unchanged. Both facts cross
the source-use consolidation boundary, participate in equality, and appear in
provenance records. The renderer cache key includes the structured-spelling
fact, avoiding traversal-order-dependent reuse.

This clears nine class-use rows: five function-type spellings, two cv
spellings, and two direct array spellings. Six complete outputs leave the
failure manifest. The nested `filter_core` row in the PA24 array fixture still
shortens `block<unsigned int[4], 4>` internally, so that test remains open.

Correctness and diagnostic evidence from fresh isolated Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence improves from 1,410 to 1,416 matching outputs, leaving
  114 known mismatches and no new failing output;
- the class-use inventory falls from 49 to 40 changed rows and from 35 to 29
  failing tests; it still has zero missing rows, two unexpected rows, and
  three ordering cases;
- function-call and lifecycle inventories are unchanged;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,361 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper unit suites pass
  60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The template-side boundary ratchet is byte-identical to the parent checkpoint.
The semantic-side audit retains the parent's five output-readiness queries,
ten template-service mentions, and nine internal-header sites. These existing
counts are not acceptance gates for this payload slice.

The ordinary convergence report is
`/tmp/cppgm-structured-type-convergence-20260810.json`, SHA-256
`e9e8b9df1305ed8fb86a5038eec963a24314320c831c7f999bffad2ad4864448`.
The provenance analysis and convergence reports are
`/tmp/cppgm-structured-type-provenance.MqslIt/provenance-analysis.json` and
`/tmp/cppgm-structured-type-provenance.MqslIt/convergence.json`, with SHA-256
values
`619264dcffcc12a1a35ac030368dada1857994bac551b543742dbd4c667f0423`
and
`9d2a706a844a57e52dc2161f197f4365c79af5a4e5e5bafce8b7dec2e0547344`.
The byte-identical ordinary/provenance output manifest has SHA-256
`90e17c91b525f0ecba99c36d4655e17e689bbe0dc89993c4679d687baf451eb8`.
The broad report is `/tmp/cppgm-structured-type-broad-20260810.log`, SHA-256
`80730739ddf67f969e1c8c840033972b1f6f6fbcbb2e6dcfc203d3557f78d522`.
The materialization audit remains byte-identical to the preceding checkpoints
at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.09% | +0.66% | -3.89% | `304506e8f706d75bf78ffe1f89075ec64c9fae03820adb4c9cc263ba9b13921b` |
| Prior rolling checkpoint | -0.07% | +1.82% | +0.07% | `94ea840cd4746131ec1d6fe1b033928d748706a39845d8c4a62a521548aa304c` |

The reports are `/tmp/cppgm-structured-type-vs-fixed-20260810.json` and
`/tmp/cppgm-structured-type-vs-rolling-20260810.json`. The shared raw
candidate is `/tmp/cppgm-structured-type-raw-candidate-20260810.json`,
SHA-256
`f34a3c954695914365b516b6305723b6d41af99f62b98ed6ce81c399bfc99de1`.
Its metadata names commit `ce1ce270f` because the measurements cover this
uncommitted checkpoint.

Phase 3 remains open. The next class payload work must recover the remaining
nested structured spellings, binding-source labels, selection payloads, and
ordering. The two unexpected class rows also remain. Inception is still
forbidden.

## Default binding provenance checkpoint, 2026-08-10

Two independent shortcuts were relabeling supplied class arguments as
defaults. The deduced binding builder marked every trailing parameter that had
a declared default as `defaulted`, even when a supplied class-type argument
was structurally different. Later, the renderer used any shortened class alias
to relabel the same trailing positions again, including positions visibly
spelled in the source template-id.

The deduced builder now checks class-type arguments against the declared
default before assigning the defaulted label. Fundamental and non-type
arguments retain the established fallback because their default provenance can
be lost during rebinding. The renderer also retains an explicit label when the
structured source occurrence contains that argument position. These two facts
remove the false default alias that shortened a nested argument list; the full
type consequently survives into its function-call and constructor lifecycle
events.

The same defaulted-binding builder now prints a typed null pointer as
`nullptr`. Semantic template-name arguments discard a redundant leading global
qualifier. Together, this clears four class-use rows and three complete
outputs. The imported-member fixture also clears one function-call row, two
missing lifecycle rows, one unexpected lifecycle row, and one additional
definition demand.

Correctness and diagnostic evidence from fresh isolated Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence improves from 1,416 to 1,419 matching outputs, leaving
  111 known mismatches and no new failing output;
- the class-use inventory falls from 40 to 36 changed rows and from 29 to 26
  failing tests; it still has zero missing rows, two unexpected rows, and
  three ordering cases;
- the function-call inventory falls from 42 to 41 changed rows and from 60 to
  59 failing tests;
- the lifecycle inventory falls from 14 to 13 additional definition demands,
  from 85 to 83 missing rows, from 56 to 55 unexpected rows, and from 48 to 47
  failing tests;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,358 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper unit suites pass
  60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The template-side and semantic-side boundary reports are byte-identical to the
parent checkpoint. The semantic audit therefore retains five output-readiness
queries, ten template-service mentions, and nine internal-header sites. These
existing counts are not acceptance gates for this payload slice.

The ordinary convergence report is
`/tmp/cppgm-default-source-convergence-20260810.json`, SHA-256
`a50f65a5e047d5525592e94a51235682faddd312d70ee93588251cc1cb759baa`.
The provenance analysis and convergence reports are
`/tmp/cppgm-default-source-provenance.LQZ3Zl/provenance-analysis.json` and
`/tmp/cppgm-default-source-provenance.LQZ3Zl/convergence.json`, with SHA-256
values
`74fcc053596918a17b8b13e2ede317827b0f138722030cb4cf0feab3e26a6802`
and
`e99d4c318a90d0045910de17d732a3eb7c1187253c62b47ad3a99ec131636a42`.
The byte-identical ordinary/provenance output manifest has SHA-256
`3ccddc1df5c745cebd718fe7aad73f663e79f0ccc9ada6561bc2f37a7a473b8c`.
The broad report is `/tmp/cppgm-default-source-broad-20260810.log`, SHA-256
`2b7fcd185f3cca40e405ab13b1bb6b81add75dde95292d29b4985e1e0c8517c7`.
The materialization audit remains byte-identical to the preceding checkpoints
at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.99% | +0.43% | -3.85% | `185e4060544263ee173f247a7219cdd9b211bc9d1a9005070c8019539d1a97d1` |
| Prior rolling checkpoint | +0.11% | -0.23% | +0.05% | `5eaa6b51fd8e9f1515101537c1314b1f317b4814a398d5e61abf330a0d317bd7` |

The reports are `/tmp/cppgm-default-source-vs-fixed-20260810.json` and
`/tmp/cppgm-default-source-vs-rolling-20260810.json`. The shared raw candidate
is `/tmp/cppgm-default-source-raw-candidate-20260810.json`, SHA-256
`1672aa8ead29eb7f897743425f072f26b9b9c646e33ed85e6762ee5d8be87a2a`.
Its metadata names commit `d9dbc0706` because the measurements cover this
uncommitted checkpoint.

Phase 3 remains open. Thirty-six changed class rows, two unexpected class
rows, and three class ordering cases remain. Nested structured spellings and
selection payloads are the next class-use clusters. Inception is still
forbidden.

## Object-pointer binding spelling checkpoint, 2026-08-10

Object-pointer non-type template arguments retained their resolved variable
binding, but witness construction still printed the local source spelling.
That lost the declaration namespace in both a direct class binding and a
rebound nested specialization name.

The witness argument printer now follows the retained non-type value binding
to its declaration and renders the address with the declaration scope's
qualified name. The formatter is limited to nondependent object pointers. It
rejects function pointers, parameters and fields, function-local declarations,
invalid names, and unresolved or cyclic rebinding chains. The specialization,
mangle-info, source-binding, and explicit-value paths all use the same typed
formatter; none performs a lookup or reparses source text.

This clears three class-use rows in two PA23 tests. Both complete outputs now
match, which also removes their derivative lifecycle differences: three
additional definition demands, seven missing rows, and four unexpected rows.

Correctness and diagnostic evidence from fresh isolated Homebrew-Clang builds:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence improves from 1,419 to 1,421 matching outputs, leaving
  109 known mismatches and no new failing output;
- the class-use inventory falls from 36 to 33 changed rows and from 26 to 24
  failing tests; it still has zero missing rows, two unexpected rows, and
  three ordering cases;
- the function-call inventory is unchanged at 41 changed rows across 59
  failing tests;
- the lifecycle inventory falls from 13 to ten additional definition demands,
  from 83 to 76 missing rows, from 55 to 51 unexpected rows, and from 47 to 45
  failing tests;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,358 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper unit suites pass
  60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The template-side and semantic-side boundary reports are byte-identical to the
parent checkpoint. The semantic audit therefore retains five output-readiness
queries, ten template-service mentions, and nine internal-header sites. These
existing counts are not acceptance gates for this payload slice.

The ordinary convergence report is
`/tmp/cppgm-object-pointer-ordinary-convergence-20260810.json`, SHA-256
`9ef048c58785ec5d01e0b0dbb0d6a4edeca7f1171ae9acf617c4ae1d53dc174d`.
The provenance analysis and convergence reports are
`/tmp/cppgm-object-pointer-provenance.1OPvHl/provenance-analysis.json` and
`/tmp/cppgm-object-pointer-provenance.1OPvHl/convergence.json`, with SHA-256
values
`ad07cc58711c59d768c6e3b21a684c3c40c6a7c535f6aee12a32fb94c9d213c2`
and
`1fa5d25503764bd4ca17a3c88075d8d85b3c3a99791796c8bb803775ac05620d`.
The byte-identical ordinary/provenance output manifest has SHA-256
`25624a32eea081e0a6a42381d5034a00dd8dd16a799605eb365822eb95594a0b`.
The broad report is `/tmp/cppgm-object-pointer-broad-20260810.log`, SHA-256
`f0ef1ab0a7465f3b373203cf660ae8cd1b5fa8874c055f72ad5e6f6bd03eae16`.
The materialization audit remains byte-identical to the preceding checkpoints
at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

Both three-run performance comparisons pass:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report SHA-256 |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.00% | -7.51% | -3.98% | `c87904c6f813324aa83532c28129bb4324430f7760a2f4b27d520106aac750bd` |
| Prior rolling checkpoint | -0.01% | -7.91% | -0.14% | `53ad206d111fae52c92b391f4d931202709682bec14da39c0d2dd226bae53175` |

The reports are `/tmp/cppgm-object-pointer-vs-fixed-20260810.json` and
`/tmp/cppgm-object-pointer-vs-rolling-20260810.json`. The shared raw candidate
is `/tmp/cppgm-object-pointer-raw-candidate-20260810.json`, SHA-256
`39342e173c2711b43f8ed32a8fcac0b4e720a03a17d4fc02dde05238855d0f9c`.
Its metadata names commit `a5d38157a` because the measurements cover this
uncommitted checkpoint.

Phase 3 remains open. Thirty-three changed class rows, two unexpected class
rows, and three class ordering cases remain. Selection payloads, owner
qualification, and dependent pack spelling are the next class-use clusters.
Inception is still forbidden.

## Retained enum-value spelling checkpoint, 2026-08-10

Resolved enum-valued non-type template arguments retained their type and
integral value but not the semantic enumerator that supplied the witness
spelling. Nested specialization and rebound default paths therefore printed a
cast or integral value after the original source argument was no longer the
public semantic argument.

Argument resolution and class-specialization selection now retain a unique
matching enumerator in the active witness session. The retained fact consists
of the enumerator binding and enum scope, indexed by canonical enum type and
value. Structured class-template arguments are traversed recursively so nested
enum arguments use the same carrier. Retention rejects dependent arguments,
non-enum values, and enum values with more than one matching enumerator.

Witness formatting consumes the retained binding and scope. It does not scan a
semantic scope, perform a lookup, or reparse source text. Scoped enums include
the enum name; unscoped enums use the enumerator's declaration scope. The
carrier is not part of `TemplateArgument::RareData`: it exists only in
`TemplateWitnessSession`, and the retention helpers return before inspecting
arguments or constructing traversal sets when no witness session is active.

The change clears eight class-use rows across three PA23 tests and one
function-call row in PA20. The two PA23 result-SFINAE outputs also lose nine
missing and nine unexpected lifecycle facts derived from the old spelling.
Expanded convergence improves from 1,421 to 1,423 matching outputs with no new
mismatch:

- class-use changes fall from 33 to 25 rows and from 24 to 21 tests, with two
  unexpected rows and three ordering-only cases unchanged;
- function-call changes fall from 41 to 40 rows and from 59 to 58 tests;
- lifecycle missing rows fall from 76 to 67 and unexpected rows from 51 to 42;
  the ten additional definition demands, 45 failing tests, and nine warnings
  are unchanged.

Correctness and diagnostic evidence from the final guarded implementation:

- the preserved original strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,423/1,530, leaving 107 known mismatches;
- final isolated ordinary and provenance compilers produce identical witness
  and LowIR output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,352 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the canonical PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- the ordinary semantic-structure size report is byte-identical to the parent;
  `TemplateArgument` remains 136 bytes, `Type` 280 bytes, and `ClassInfo` 1,136
  bytes;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero.

The ordinary convergence report is
`/tmp/cppgm-enum-sidecar-guarded-convergence-20260810.json`, SHA-256
`c3a6a6ba6971f12f80100abfd1114a0be7049779154544a95f2a7c8faec1239b`.
The final provenance analysis and convergence reports are
`/tmp/cppgm-enum-sidecar-guarded-provenance.I92Ys1/provenance-analysis.json`
and
`/tmp/cppgm-enum-sidecar-guarded-provenance.I92Ys1/convergence.json`, with
SHA-256 values
`f04d14e2dd1df120aaa4a4d6e19865d452b99868f7aacb045502fb39c8ff99bd`
and
`790f6e0363f08cc9dabbbce111b828a7c9e94b65c2c76af82c680980fd4f0879`.
The byte-identical output manifest is
`/tmp/cppgm-enum-sidecar-guarded-output-manifest-20260810.txt`, SHA-256
`4f6d78323982450c284e5fa15bba1b8cca9200250605f826e1dcdbd855a705d2`.
The broad report is
`/tmp/cppgm-enum-sidecar-guarded-broad-20260810.log`, SHA-256
`cd0e33e0b6b496e590a3e05e940de8c07498c14d79d7bfbe8420bbed7a119040`.
The materialization audit remains unchanged at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.
The structure-size report is
`/tmp/cppgm-enum-sidecar-guarded-structure-sizes-20260810.txt`, SHA-256
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

Performance required a baseline investigation. The guarded three-run candidate
passes the fixed alias-convergence baseline at -1.00% instructions, -0.21% RSS,
and -3.93% footprint. Against the historical object-pointer rolling file it is
-0.00% instructions, +7.90% RSS, and +0.06% footprint. The required second
batch remains above the historical RSS threshold at +8.60%, while instructions
are -0.11% and footprint is +0.14%. The historical RSS gate therefore fails;
it is not reported as a pass.

The regression investigation rebuilt the exact parent commit `04236cb18` from
an empty object root and measured it in the current host state. Its
760,057,856-byte RSS median is also above the historical 700,223,488-byte
median. The historical parent batch ranges from 671,674,368 to 736,587,776
bytes, while the contemporaneous guarded candidate is 755,519,488 bytes.
Comparison with the contemporaneous parent passes at -0.17% instructions,
-0.60% RSS, and +0.01% footprint. This identifies host RSS state, not the
candidate, as the source of the historical warning. The historical file
remains preserved, and the contemporaneous exact-parent measurement becomes
the refreshed rolling reference for this checkpoint.

The fixed-baseline report is
`/tmp/cppgm-enum-sidecar-guarded-vs-fixed-20260810.json`, SHA-256
`84ef60c86db375591a25bb654146c7ad291b8e7ff94272ac6a500ed3ca9b3c7b`.
The historical rolling confirmation report is
`/tmp/cppgm-enum-sidecar-guarded-confirmation-vs-rolling-20260810.json`,
SHA-256
`aab8a3e38d90eab88f8b935555fbf1325dd7c58e07260d9ff22f3444bc2f2310`.
The refreshed parent raw record and passing comparison are
`/tmp/cppgm-object-pointer-parent-contemporary-raw-20260810.json` and
`/tmp/cppgm-enum-sidecar-guarded-primary-vs-contemporary-parent-20260810.json`,
with SHA-256 values
`1cefbf0ac1bf9a3123e20a8b0dcc9519472bd76379c0ede7cc6e1133c949e502`
and
`1495f65f2f1e5ed8af9bf10ecf706d429a6578bec9dfbbb01b33be4d94cccfbd`.

Phase 3 remains open. Twenty-five changed class rows, two unexpected class
rows, and three class ordering cases remain. Selection payloads, owner
qualification, and dependent pack spelling remain the next class-use clusters.
Inception is still forbidden.

## Typed character argument checkpoint, 2026-08-10

Resolved `char`, `signed char`, and `unsigned char` non-type template
arguments carried a canonical type and integral value, while the generic
argument printer emitted the value as a decimal integer. Class bindings,
partial-specialization bindings, and lifecycle entity names lost their
character-literal spelling. Numeric user-defined literal lowering also
built a synthetic template-id that overload resolution treated as an explicit
template-id. The source witness then labeled the literal operator's character
pack `source=explicit`.

The template witness formatter derives narrow-character spelling from the
fundamental type and value. Direct template-argument contexts add the
`signed char` or `unsigned char` cast used by Clang's `TemplateArgument`
printer. Nested template-id contexts emit the character literal without that
cast. The formatter handles named control escapes, printable ASCII, quote and
backslash escapes, and two-digit lowercase hexadecimal escapes.

Numeric literal-operator lowering marks its synthetic `TemplateIdSyntax`
arguments as source-deduced. Overload resolution consumes the synthetic
character arguments, while function-call witness attribution uses an explicit
argument count of zero. The four custom template-id clone paths preserve the
flag. `TemplateIdSyntax` remains 160 bytes.

The change updates five witness outputs and makes four tests byte-exact. The
PA23 static-data case retains one independent selection defect at its first
use: CPPGM selects the explicit specialization where the reference selects the
partial specialization. All other character spellings in that test match,
including its lifecycle entity.

Expanded convergence improves from 1,423 to 1,427 matching outputs:

- class-use changes fall from 25 to 19 rows and from 21 to 19 tests; the two
  unexpected rows and three ordering cases remain;
- function-call changes fall from 40 to 37 rows and from 58 to 56 tests; the
  missing, unexpected, and ordering inventories remain unchanged;
- lifecycle missing rows fall from 67 to 64, unexpected rows fall from 42 to
  39, and failing tests fall from 45 to 42. The ten additional definition
  demands and nine warning tests remain.

Correctness evidence from the Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,427/1,530, leaving 103 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,358 records with no
  unknown producer and no unexercised producer site;
- class consolidation retains 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report matches the parent. `Type` remains 280 bytes,
  `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and `ClassInfo`
  1,136 bytes.

The standalone semantic/template boundary ratchet repeats the parent counts:
five output-readiness queries, ten template-service mentions, and nine
internal-header sites. The plan does not use that ratchet as an acceptance gate
for this witness slice.

The ordinary convergence report is
`/tmp/cppgm-char-origin-convergence-20260810.json`, SHA-256
`139941045eb1c0474da32650375c132ba734328df8620bdc3b6f346422602e89`.
The provenance analysis and convergence reports are
`/tmp/cppgm-char-origin-provenance-20260810/provenance-analysis.json` and
`/tmp/cppgm-char-origin-provenance-20260810/convergence.json`, with SHA-256
values
`645e19efe6df73bae8dff2a23c6abce9b90fa72a478e0bfc255498dd74387bd6`
and
`1805e8c1858813ffe5661dbce206dd48beeae7249895574b5cdad9ba0e7c1431`.
The byte-identical output manifest is
`/tmp/cppgm-char-origin-output-manifest-20260810.txt`, SHA-256
`c6c3d9dc32fb6f7cf4655ccfd720bc62b54a7f1aa33d7b6e3858ac6631d2b797`.
The broad report is `/tmp/cppgm-char-origin-broad-20260810.log`, SHA-256
`762686ab2c9a42e7da8a3c3aa418e358893d135baae4d9028574f7062d2a2022`.
The structure-size report is
`/tmp/cppgm-char-origin-structure-sizes-20260810.txt`, SHA-256
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The materialization audit retains SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.

The three-run performance record passes both comparison gates:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.93% | -0.32% | -3.97% | `/tmp/cppgm-char-origin-vs-fixed-20260810.json` |
| Contemporaneous parent | -0.10% | -0.71% | -0.03% | `/tmp/cppgm-char-origin-vs-contemporary-parent-20260810.json` |

The raw candidate record has SHA-256
`95711d5127961270f6547a9c8e132c043db7b8f1d43b1d911af24eeab06fcf33`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`36a3ef19b538f95ca99f108e9a3ccc3ff167383c0357b90a89bd68481dd7662d`
and
`ab9e7254c1fcd70c83ca94a859cf66bb8770e8316409fa8ffbc31b5c54205af2`.

Phase 3 remains open. Nineteen changed class rows, two unexpected rows, and
three ordering cases remain. Selection payloads and owner qualification are
the next class-use clusters. Inception is still forbidden.

## Default-equivalent rebinding checkpoint, 2026-08-10

The `offset_ptr` rebinding fixture supplied the primary template's three
trailing defaults explicitly. The initial class-use path recognized the two
fundamental type defaults, but not the declaration-scope constant
`alignment_value`. Dependent rebinding then retained all four arguments in
its mangle metadata and printed the expanded spelling in nested class,
function-call, and lifecycle entities.

The shared witness default-equivalence check now covers all consumers. It
compares canonical fundamental spellings such as `long int` and `long`, and it
resolves a named integral default through the template declaration scope
before comparing the retained constant value. Class source bindings,
class-instantiation bindings, nested mangle names, and lifecycle names use the
same result. The three class source-binding call sites pass the primary
template's declaring scope instead of asking the renderer to guess where a
named default came from.

Explicit specializations remain a separate semantic identity. Before eliding
an explicit-equivalent argument list, the mangle path checks the primary
template's explicit-specialization registry. The two `plus<void>` PA24
fixtures therefore keep their full specialization spelling. This guard is
based on the structured template argument identity key, not a template name,
source location, or fixture filter.

The same focused fixture exposed an independent rejection label. Constructor
selection already records `argument count/type shape mismatch`; the witness
drop classifier now maps that existing rejection to `too_many_arguments`.
The classifier previously fell through to `substitution_failure` because it
recognized only the shorter `argument count mismatch` spelling.

Exactly three expanded outputs change from the typed-character checkpoint:

- `pa23/tests/spec/500-defaulted-rebind-constructor-deduction.t` becomes
  byte-exact, including its direct LowIR output;
- `pa22/tests/spec/300-nested-member-template-definition-parameter-alias-default.t`
  remains mismatched, but one extra candidate drop is now classified as
  `too_many_arguments` instead of `substitution_failure`;
- `pa23/tests/general/500-weak-ptr-shared-ptr-template-ctor.t` remains
  mismatched for independent presence, ordering, and rejection rows, while
  the same arity drop receives the corrected label.

No previously exact output regresses. Expanded convergence improves from
1,427 to 1,428 matching outputs:

- class-use changes fall from 19 to 16 rows and from 19 to 18 tests; the two
  unexpected rows and three ordering cases remain;
- function-call changes fall from 37 to 36 rows and from 56 to 55 tests; the
  18 missing rows, 13 unexpected rows, and one ordering case remain;
- lifecycle additional definition demands fall from ten to eight, missing
  rows fall from 64 to 55, unexpected rows fall from 39 to 32, failing tests
  fall from 42 to 41, and warning tests fall from nine to eight.

Correctness and diagnostic evidence from the Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,428/1,530, leaving 102 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,342 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report is byte-identical to the parent checkpoint.
  `Type` remains 280 bytes, `TemplateArgument` 136 bytes,
  `TemplateIdSyntax` 160 bytes, and `ClassInfo` 1,136 bytes.

The standalone boundary reports also retain the parent counts. The
template-side audit has four service adapters, two service bundles, 15 direct
semantic-service accesses, 115 text-recovery bridges, 65 canonical-key
metadata sites, 140 witness source-location sites, and 197 mixed
`callsemantic.cpp` exceptions. The semantic-side audit retains five
output-readiness queries, ten template-service mentions, and nine internal
header sites. These ratchets did not grow in this checkpoint.

The ordinary convergence report is
`/tmp/cppgm-default-elision-convergence-final-20260810.json`, SHA-256
`6dfe21694ac8a517102e2183eb8a7ec3d4b0a583fe154925cc2ed69fe79a1dc1`.
The provenance analysis and convergence reports are
`/tmp/cppgm-default-elision-provenance-final-20260810/provenance-analysis.json`
and
`/tmp/cppgm-default-elision-provenance-final-20260810/convergence.json`, with
SHA-256 values
`e70259c4054fe02e025c7e77bc55ba2f12232a1f4b59f9fe2fd6c0fb9b4664f9`
and
`2df9702c79f145dfee50a68a6af82fe6b4ce2a08cd70e7efaf901aba273da959`.
The byte-identical output manifest is
`/tmp/cppgm-default-elision-output-manifest-20260810.txt`, SHA-256
`036f7dcde4746d12e33a7b5e44615ff89335c956f715b8d462df17761f8f76b9`.
The broad report is `/tmp/cppgm-default-elision-broad-20260810.log`, SHA-256
`c2ea7c619b29208514e15674bd4d5fc973d8076f6e8f95b37326a371af5b7e4a`.
The materialization audit remains byte-identical to the parent at SHA-256
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`.
The structure-size report retains SHA-256
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The shared three-run candidate record passes both performance comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.06% | +0.68% | -3.89% | `/tmp/cppgm-default-elision-vs-fixed-20260810.json` |
| Typed-character parent | -0.13% | +1.01% | +0.09% | `/tmp/cppgm-default-elision-vs-parent-20260810.json` |

The raw candidate record has SHA-256
`c06ebe4bb267663079cf3024541eea135cf731d9360c0b9c58239ed3a34ce34e`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`f2845a4fc12feef01da62816ad9035b8ff6e43e6780e6037192333bfeca76f66`
and
`9a3da9d3e90fc2f98021000e306fd5d41a4b6bef3c5b2e15672a771728ec00df`.
The candidate metadata names commit `8fe771919` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. Sixteen changed class rows, two unexpected class rows,
and three class ordering cases remain. Selection payloads and owner
qualification are still the next class-use clusters. Inception remains
forbidden.

## Semantic source-binding identity checkpoint, 2026-08-10

Template-template and nested class source bindings could retain a spelling
from the use site after resolution had identified a different semantic owner.
That left unqualified template identities in deduced payloads, dropped a real
namespace owner from member alias-template arguments, exposed the anonymous
namespace in source bindings, and preserved a namespace alias on a resolved
enumerator.

Resolved class- and alias-template arguments now render their structured
template entity. A member template uses its resolved owner type; a namespace
template uses its retained declaration-scope prefix. Structured class-template
types have a separate source-binding name policy based on their declaration
scope. It omits an unnamed namespace from the public source binding, while
nested arguments keep the established renderer unless the nested declaration
itself has an unnamed-namespace owner. That narrow recursion boundary avoids
rebuilding unrelated cv-qualified, defaulted, and deeply recursive types.

Explicit enum-value bindings now prefer the unique retained semantic
enumerator before the written qualified identifier. Namespace aliases such as
`bi::aq` therefore become the declaration identity `boost::aq`; ambiguous enum
values still fall back to the source spelling. None of these paths reparses
source text or filters by a template, namespace, fixture, or source location.

Exactly four expanded witness outputs change from the default-equivalence
checkpoint:

- `pa22/tests/spec/200-deduced-template-template-qualified-identity.t` and
  `pa22/tests/general/400-member-alias-template-template-empty-template-id-argument.t`
  become byte-exact;
- `pa22/tests/general/200-alias-template-template-argument-use-scope.t`
  clears all three class-use differences and remains mismatched only for an
  independent missing lifecycle row;
- `pa24/tests/general/400-concrete-recursive-node-layout-retry.t` clears its
  namespace-alias enum binding and retains its independent lifecycle gaps.

No LowIR, stdout, stderr, or exit-status output changes, and no previously
exact witness regresses. Expanded convergence improves from 1,428 to 1,430
matching outputs. Changed class-use rows fall from 16 to nine and affected
tests from 18 to 14; the two unexpected rows and three ordering cases remain.
Function-call and lifecycle inventories are unchanged.

Correctness and diagnostic evidence from the isolated Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,430/1,530, leaving 100 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,347 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR run passes all 4,862 tests. The first aggressive
  `5 x 12` run passed 4,847 and timed out 15 hosted PA35/PA36 tests; the
  conservative PA35/PA36 retry passes 182/182;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- both static materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and
  `ClassInfo` 1,136 bytes.

The standalone boundary reports retain the parent counts: four service
adapters, two service bundles, 15 direct semantic-service accesses, 115
text-recovery bridges, 65 canonical-key metadata sites, 140 witness
source-location sites, and 197 mixed `callsemantic.cpp` exceptions on the
template side; five output-readiness queries, ten template-service mentions,
and nine internal header sites on the semantic side.

The ordinary binary is 17,151,160 bytes, 5,696 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by one 4,096-byte page to 13,041,664 bytes;
`__DATA_CONST` remains 61,440 bytes and `__DATA` remains 442,368 bytes. The
ordinary binary contains no provenance symbols.

The provenance analysis and convergence reports are
`/tmp/cppgm-owner-qualified-provenance-final-20260810.Mi0iRj/provenance-analysis.json`
and
`/tmp/cppgm-owner-qualified-provenance-final-20260810.Mi0iRj/convergence.json`,
with SHA-256 values
`e30a215c1aab1956127a0749ee54f5e2c5f98a7ad25995ff8495c420c51b916d`
and
`fada3fdf84a961198f9c607ccb8bbe7a0dd0d69eaa204ff5e17de0fde5234974`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-owner-qualified-output-manifest-20260810.txt`, SHA-256
`c294156ba09a9c8dfb4c1048cd3338a616f59c04ff5490b68ae9f4db46a9f495`.
The initial broad report and conservative retry have SHA-256 values
`dc7a333a78e75b0b16fa73dde6035c133d183259d33f2568a54091d7cbb92e8d`
and
`ddc2525f99912ceab20504906ab419959de3f4f0e6c82e9018567aaa1c6b942f`.
The materialization and structure-size reports remain byte-identical to the
parent at SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The shared three-run candidate record passes both performance comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.75% | -0.30% | -3.91% | `/tmp/cppgm-owner-qualified-vs-fixed-20260810.json` |
| Default-equivalence parent | +0.32% | -0.97% | -0.02% | `/tmp/cppgm-owner-qualified-vs-parent-20260810.json` |

The raw candidate record has SHA-256
`3b3f69b22119cfd9ebdc37f25be76b7ca783295911da0ab0286a1bfe1fd36c99`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`cc97e81a6f62d6329a33f427a1bc6d1b08cbe61848375f9c8c89aa83baa8e325`
and
`c9dcf88137edfff205cdebf4e4d7b2ca2cb1b37e93da99f1a5096928be1f1b5a`.
The candidate metadata names commit `004056c58` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. Nine changed class rows, two unexpected class rows, and
three class ordering cases remain. Partial-selection payloads and dependent
pack spellings are the next coherent class-use clusters. Inception remains
forbidden.

## Canonical nested parameter binding checkpoint, 2026-08-10

The retained current-specialization path already canonicalized a template
parameter when it was the complete type argument. It stopped at an enclosing
class-template specialization, however. Out-of-class partial-specialization
definitions therefore exposed lexical names such as `Pair<Key, Value>` and
`V<A>` in their first owner binding, while a later occurrence of the same
owner used the stable `type-parameter-0-*` identity.

The same recursive canonicalizer now consumes both structured class-type
carriers: concrete specialization mangle metadata and retained dependent
class-template metadata. It walks semantic type and value arguments, matches
parameters by placeholder identity, preserves written non-parameter
arguments, and omits retained defaulted arguments. The dependent carrier is
converted according to its stored argument kind and value facts. The path
does not parse source text or select behavior by template name, fixture, or
source location.

Exactly two expanded witness outputs change from the semantic source-binding
checkpoint, and both become byte-exact:

- `pa22/tests/general/100-partial-specialization-member-typedef-outdef.t`;
- `pa22/tests/general/100-partial-static-out-of-class-dependent-return-definition.t`.

No LowIR, stdout, stderr, or exit-status file changes, and no previously exact
witness regresses. Expanded convergence improves from 1,430 to 1,432 matching
outputs. Changed class-use rows fall from nine to seven and affected tests
fall from 14 to 12; the two unexpected rows and three ordering cases remain.
Function-call and lifecycle inventories are unchanged.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,432/1,530, leaving 98 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,346 records with no
  unknown producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862 on the first conservative
  `4 x 4` run;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- both static materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and
  `ClassInfo` 1,136 bytes.

The boundary reports also retain their parent counts: four service adapters,
two service bundles, 15 direct semantic-service accesses, 115 text-recovery
bridges, 65 canonical-key metadata sites, 140 witness source-location sites,
and 197 mixed `callsemantic.cpp` exceptions on the template side; five
output-readiness queries, ten template-service mentions, and nine internal
header sites on the semantic side.

The ordinary binary is 17,155,544 bytes, 4,384 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by one 4,096-byte page to 13,045,760 bytes;
`__DATA_CONST` remains 61,440 bytes and `__DATA` remains 442,368 bytes. The
ordinary binary contains no provenance symbols.

The final ordinary convergence report is
`/tmp/cppgm-placeholder-canonical-final-convergence-20260810.json`, SHA-256
`e4675c78eb768c3ee0d4c0277bf4f4ef6588a7c7955adb5b8c2a746993306fc2`.
The provenance analysis and convergence reports are
`/tmp/cppgm-placeholder-canonical-provenance-final2-20260810.p0DEy4/provenance-analysis.json`
and
`/tmp/cppgm-placeholder-canonical-provenance-final2-20260810.p0DEy4/convergence.json`,
with SHA-256 values
`0f5c6bbe6ef50dceb9224271759db0a50f3174ec0e795b4341f53495860f0d16`
and
`83ba218ee8c7d06ccfb627942fe79a529f290793dab810488adc2b4dac421f9e`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-placeholder-canonical-final-output-manifest-20260810.txt`, SHA-256
`df7e0751d8efae599375411575f1ab6a67fab199c3e1896b9176377a874ccf7a`.
The broad report is
`/tmp/cppgm-placeholder-canonical-broad-20260810.log`, SHA-256
`a28c59811cdf1fa8d23568f611809709796ff3d26a998261a36517ace1a1ffdf`.
The materialization and structure-size reports remain byte-identical to the
parent at SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.02% | +1.11% | -3.90% | `/tmp/cppgm-placeholder-canonical-final-vs-fixed-20260810.json` |
| Semantic source-binding parent | -0.27% | +1.42% | +0.00% | `/tmp/cppgm-placeholder-canonical-final-vs-parent-20260810.json` |

The raw candidate record has SHA-256
`20e56ef4bcddb688fdb1abfadae812043d50c4ecfd4c4c34fd5e1748b176337f`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`7ba5ba3b708bf9f9934c2aa7910ce9e59a13b077fc520945d0843c10fac8281b`
and
`f4b0275409f229d7ccde315877f3740350e877da6caae741db0b64d1de86e1aa`.
The candidate metadata names commit `5546143b9` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. Seven changed class rows, two unexpected class rows, and
three class ordering cases remain. Partial-selection payloads and dependent
pack spellings are still the next class-use clusters. Inception remains
forbidden.

## Materialized nested partial-selection checkpoint, 2026-08-10

The patched Clang witness path resolves a nested class-template-id through its
`ClassTemplateSpecializationDecl`. It reports a partial specialization only
after that specialization declaration has been instantiated from the partial.
CPPGM selected the matching partial earlier, while resolving the reference.
Publishing that eager selection immediately exposed a partial for identity-only
uses where Clang still reported the primary template.

Nested source-argument requests now retain their semantic instance and defer
partial-selection visibility until final class-use collection. Collection
builds one index of the direct semantic base instances present in the pending
source graph. A deferred selection remains partial when that specialization is
materialized as an enclosing source class's base; otherwise the published use
is demoted to the primary template and its partial bindings are removed. The
same specialization identity can make the partial visible across occurrences,
matching Clang's declaration-level state. The path uses semantic instance and
base relationships only; it does not inspect template names, fixture paths,
source locations, or source text.

Exactly two expanded witness outputs change from the canonical nested parameter
binding checkpoint, and both become byte-exact:

- `pa24/tests/specialization/400-defaulted-nested-class-argument-partial-specialization.t`;
- `pa24/tests/specialization/400-dependent-nested-nontype-partial-specialization.t`.

Three focused controls remain byte-exact, including nested partial matches used
only through a pointer, a type-identity argument, or another non-base source
position. No LowIR, stdout, stderr, or exit-status file changes, and no
previously exact witness regresses. Expanded convergence improves from 1,432
to 1,434 matching outputs. Changed class-use rows fall from seven to five and
affected tests fall from 12 to ten; the two unexpected rows and three ordering
cases remain. Function-call and lifecycle inventories are unchanged.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,434/1,530, leaving 96 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR output
  for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,346 records with no unknown
  producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862 on the first conservative
  `4 x 4` run;
- the convergence, provenance, materialization, text-reparse, path,
  performance, semantic-boundary, and class-audit helper suites pass 60/60;
- both static materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and
  `ClassInfo` 1,136 bytes.

The boundary reports also retain their parent counts: four service adapters,
two service bundles, 15 direct semantic-service accesses, 115 text-recovery
bridges, 65 canonical-key metadata sites, 140 witness source-location sites,
and 197 mixed `callsemantic.cpp` exceptions on the template side; five
output-readiness queries, ten template-service mentions, and nine internal
header sites on the semantic side.

The ordinary binary is 17,155,776 bytes, 232 bytes larger than the parent. Its
Mach-O loadable segments are unchanged: `__TEXT` is 13,045,760 bytes,
`__DATA_CONST` is 61,440 bytes, and `__DATA` is 442,368 bytes. The increase is
confined to link-edit/debug metadata, and the ordinary binary contains no
provenance symbols.

The final ordinary convergence report is
`/tmp/cppgm-nested-partial-base-expanded2-convergence-20260810.json`, SHA-256
`3bdd078fa7d2d2486792b0c1833fd0993090a098e706595f7d7727b1c27df41c`.
The provenance analysis and convergence reports are
`/tmp/cppgm-nested-partial-base-provenance-final-20260810/provenance-analysis.json`
and
`/tmp/cppgm-nested-partial-base-provenance-final-20260810/convergence.json`,
with SHA-256 values
`59c865c604ef019ba8453d7ad278c54b682fd2e58fe1c3b98c4e6e0d516b50b0`
and
`48597a397e664a9a4efae558925085f926806da5d94abd7b683a9079bb465be0`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-nested-partial-base-output-manifest-20260810.txt`, SHA-256
`3aa8a4b5a614c49d54f990f0015be2d82f36bec896553c0c49ae665e199008`.
The broad report is
`/tmp/cppgm-nested-partial-base-broad-final-20260810.log`, SHA-256
`4f27fd87b43730307b07a833ba928215f2e325966f0a7a194051365755fa140f`.
The materialization and structure-size reports remain byte-identical to the
parent at SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.04% | -0.45% | -3.93% | `/tmp/cppgm-nested-partial-base-vs-fixed-final-20260810.json` |
| Canonical nested binding parent | -0.03% | -1.55% | -0.03% | `/tmp/cppgm-nested-partial-base-vs-parent-20260810.json` |

The raw candidate record has SHA-256
`1186edaaade22cf51eda7c151fb0da47c3eb808d29acf15276f682de9f8b28e9`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`0fc54708f916044429a1026d4a17ccd359264c1e924c98d2cd0722f7d8aae9d9`
and
`783715117b8e0d585572d39bbee2abcabe2272aaaadb507688d4d24e513ca434`.
The candidate metadata names commit `47484f893` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. Five changed class rows, two unexpected class rows, and
three class ordering cases remain. Dependent pack spellings are the next
coherent class-use cluster. Inception remains forbidden.

## Residual structured class payload checkpoint, 2026-08-10

Five changed class-use rows remained after nested partial-selection deferral.
Four came from semantic facts already present in the compiler but omitted from
the final class-use request:

- a nonnested partial specialization had a materialized definition or owned a
  nested class use, but the final collector still demoted it to the primary;
- a concrete out-of-class static-data owner retained the primary selection
  made while parsing its dependent declaration instead of selecting the
  partial against the resolved arguments;
- a defaulted unsigned non-type argument passed through signed decimal
  formatting;
- an array nested inside a class-template type argument lost its structured
  fundamental-type spelling.

Final class-use collection now indexes both direct semantic bases and enclosing
semantic member owners. Deferred nonnested partial selections remain visible
when the selected instance has materialized semantic definition state or owns
a nested pending use. Nested source arguments keep the stricter direct-base
condition from the previous checkpoint. Concrete out-of-class owners reselect
their specialization through `ClassTemplateUseInfo`, retain the selected
declaration anchor, and carry structured main and partial bindings, including
pack sizes. Template binding rendering now applies typed character and unsigned
integral formatting to defaulted values and recursively detects structured
array or cv-qualified type arguments. Template-internal queries expose the
materialization and enclosing-owner facts, so `callsemantic.cpp` does not add a
mixed metadata exception.

Exactly four expanded witness outputs change from the materialized nested
partial-selection checkpoint, and all four become byte-exact:

- `pa22/tests/general/400-partial-specialization-redecl-member-template-empty-pack.t`;
- `pa23/tests/general/400-static-data-nttp-pack-sizeof-bound.t`;
- `pa23/tests/general/500-constructor-sfinae-namespace-constant-symbol.t`;
- `pa24/tests/general/500-array-type-argument-sfinae-static-value.t`.

No LowIR, stdout, stderr, or exit-status file changes, and no previously exact
witness regresses. Expanded convergence improves from 1,434 to 1,438 matching
outputs. Changed class-use rows fall from five to one and affected tests fall
from ten to six. The two unexpected rows and three ordering cases remain.
Function-call and lifecycle inventories are unchanged.

The sole changed class-use row is
`pa22/tests/general/400-template-template-fixed-prefix-pack-order.t`. The
patched Clang oracle selects the line 9 variadic partial and its executable
returns 1. CPPGM and GCC select the line 16 fixed-arity partial and the
assignment executable returns 0. Changing CPPGM to reproduce the Clang witness
would break the existing runtime contract. The row therefore remains a known
oracle divergence; witness publication continues to describe CPPGM's selected
specialization.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,438/1,530, leaving 92 known mismatches;
- ordinary and provenance compilers produce identical witness and LowIR output
  for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,345 records with no unknown
  producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862 on the first conservative
  `4 x 4` run;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both static materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and
  `ClassInfo` 1,136 bytes.

A separate non-gate discovery of all 249 script unit tests has one pre-existing
batch-wrapper error and one platform skip. The error reproduces in isolation;
the failing test and every wrapper it exercises are unchanged from the parent.
The scoped 60-test helper gate and the official assignment report both pass.

The boundary reports retain their parent counts: four service adapters, two
service bundles, 15 direct semantic-service accesses, 115 text-recovery
bridges, 65 canonical-key metadata sites, 140 witness source-location sites,
and 197 mixed `callsemantic.cpp` exceptions on the template side; five
output-readiness queries, ten template-service mentions, and nine internal
header sites on the semantic side.

The ordinary binary is 17,160,888 bytes, 5,112 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by one 4,096-byte page to 13,049,856 bytes;
`__DATA_CONST` remains 61,440 bytes and `__DATA` remains 442,368 bytes. The
ordinary binary contains no provenance symbols.

The final ordinary convergence report is
`/tmp/cppgm-residual-class-payload-final-convergence-20260810.json`, SHA-256
`cc54fb30abdcb1260e4b0bb42ae72e7e3d1031a8871637189b301710eacab209`.
The provenance analysis and convergence reports are
`/tmp/cppgm-residual-class-payload-provenance-final-20260810.FzQOEX/provenance-analysis.json`
and
`/tmp/cppgm-residual-class-payload-provenance-convergence-20260810.json`, with
SHA-256 values
`b6085c79ee1666bf6269efe397ec52a924ea9486bacf1e28c179d55cc243b1f5`
and
`554cc88c3e585e025983e865e069db959fc8e045825ab70755305937dcb30fff`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-residual-class-payload-output-manifest-20260810.txt`, SHA-256
`b890ddac2303fd80b27d120e45a0eefe22efc767ba4f60cd3e3758e19a10d559`.
The broad report is
`/tmp/cppgm-residual-class-payload-broad-20260810.log`, SHA-256
`ffaab2c0d5d75431a84b26b850a9835e72ce7cdb50ce2c5ec5521bf9711ca9b0`.
The materialization and structure-size reports remain byte-identical to the
parent at SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The final template and semantic boundary reports have SHA-256 values
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`
and
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.97% | -0.16% | -3.89% | `/tmp/cppgm-residual-class-payload-perf-fixed-20260810.txt` |
| Materialized nested partial parent | +0.07% | +0.29% | +0.04% | `/tmp/cppgm-residual-class-payload-perf-parent-20260810.txt` |

The raw candidate record has SHA-256
`1853f455f5ab10d6b6a8053e83d91ffb7b58c2ed7bf59cc0a7a2b7f4c8417e4b`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`0808041444ec0517892a264e816fbd6f1bb2a4bf9a88ef24f028a4b6e313fd3b`
and
`70d9afe7e5c7f220787b13bab2dedbc00c161237971c0329a3771a05430baccb`.
The candidate metadata names commit `37e59031a` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. One changed class row is the recorded Clang/runtime
oracle divergence. Two unexpected class rows and three class ordering cases
remain as implementation work. The ordering cases are the next coherent
class-use cluster. Inception remains forbidden.

## Same-location semantic event ordering checkpoint, 2026-08-10

The renderer sorted events at one source location by traversal order before it
considered the semantic event family. That order works when each producer uses
the same source token space. Instantiated function-call nodes can carry a local
order such as 1, while a sibling source class-template-id carries its
translation-unit order such as 67 or 193. The renderer then placed the call
before the class use even though Clang visits the class type first.

The renderer now ranks same-location semantic families before their traversal
orders: class use, alias use, function call, then variable use. The existing
partial-owner member-call group keeps its explicit call-before-owner order. A
second structured group handles a materialized class result that has no
traversal order and shares a location with a source-spelled partial owner. The
group gives both events the owner's source order and places the materialized
result first. Both rules use event kind, source role, selection kind, source
occurrence facts, specialization identity, and traversal order. They do not
inspect template names, fixture paths, source text, or selected locations.

The checkpoint removes all three class-use ordering rows:

- `pa23/tests/general/500-bool-alias-function-template-result-metadata.t`;
- `pa23/tests/spec/300-current-specialization-constructor-template-canonical-owner.t`;
- `pa23/tests/spec/300-current-specialization-constructor-template-owner.t`.

The two constructor-template outputs become byte-exact. The bool-alias output
still lacks its known `integral_constant<bool, true>::value`
`variable-instantiation` lifecycle event, but its class-use section now matches
the reference. No previously exact witness regresses. No LowIR, stdout, stderr,
or exit-status file changes.

Expanded convergence improves from 1,438 to 1,440 matching outputs and leaves
90 known mismatches. Class-use debt now consists of one changed row and two
unexpected rows in three tests. The changed row remains the recorded
Clang/runtime oracle divergence. Function-call and lifecycle inventories are
unchanged.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- expanded convergence passes 1,440/1,530 with no missing output;
- ordinary and provenance compilers produce identical witness and LowIR output
  for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,345 records with no unknown
  producer and no unexercised producer site;
- class consolidation remains at 3,000 completed candidates, 3,303 early
  repeats, 355 prepublication merges, 2,645 collected occurrences, and 2,634
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862 on the first conservative
  `4 x 4` run;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both static materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- the structure-size report matches the parent byte for byte. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes, and
  `ClassInfo` 1,136 bytes.

The boundary reports retain their parent counts: four service adapters, two
service bundles, 15 direct semantic-service accesses, 115 text-recovery
bridges, 65 canonical-key metadata sites, 140 witness source-location sites,
and 197 mixed `callsemantic.cpp` exceptions on the template side; five
output-readiness queries, ten template-service mentions, and nine internal
header sites on the semantic side.

The six-record increase from the preceding provenance run is confined to
rejected class-materialization decision probes for
`pa23/tests/spec/200-dependent-specialized-default-arg-deduction.t`. Two
consecutive isolated runs of the same provenance binary produced 79 and 73
records for that test while witness and LowIR output remained byte-exact.
Producer coverage and class consolidation did not change, so the aggregate
record count documents this run rather than a deterministic trace-count
contract.

The ordinary binary is 17,160,928 bytes. Its Mach-O `__TEXT` segment
remains 13,049,856 bytes, `__DATA_CONST` remains 61,440 bytes, and `__DATA`
remains 442,368 bytes. The ordinary binary contains no provenance symbols.
The frozen ordinary binary is
`/tmp/cppgm-source-event-semantic-order-ordinary-final2-20260810`, SHA-256
`3969332940921d9b13cf0865f710d75fc45b857e7429e47352c0c07b445db018`.

The final ordinary convergence report is
`/tmp/cppgm-source-event-semantic-order-final2-convergence-20260810.json`,
SHA-256
`63a6189a8146a2a39396a0770092fc4d5babaa9008cfa9a27c2b9701e39dca26`.
The provenance analysis and convergence reports are
`/tmp/cppgm-source-event-semantic-order-provenance-final2-20260810.xuwlDH/provenance-analysis.json`
and
`/tmp/cppgm-source-event-semantic-order-provenance-final2-convergence-20260810.json`,
with SHA-256 values
`d3868b76e878044a84fab103ce1bb0e5cde56957165b6b4e8c6d3de4e037c0f6`
and
`3ff62b05f54d8edb8449213450d490ff3944156b9338d65e5d83cb93e659633c`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-source-event-semantic-order-final2-output-manifest-20260810.txt`,
SHA-256
`609b327109c645136e51447239bad24f6232e57fc2390777c5fba6176f72c6f5`.
The broad report is
`/tmp/cppgm-source-event-semantic-order-final2-broad-20260810.log`, SHA-256
`536862d2247603cbf8be235d66f64f9303ac3879608854168e7984c67f32543d`.
The materialization and structure-size reports match the parent at SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The zero-finding text-reparse report retains SHA-256
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`.
The template and semantic boundary reports retain SHA-256 values
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`
and
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.98% | -0.27% | -3.95% | `/tmp/cppgm-source-event-semantic-order-final2-perf-fixed-20260810.txt` |
| Residual structured payload parent | -0.01% | -0.11% | -0.06% | `/tmp/cppgm-source-event-semantic-order-final2-perf-parent-20260810.txt` |

The raw candidate record is
`/tmp/cppgm-source-event-semantic-order-final2-raw-candidate-20260810.json`,
SHA-256
`8ce08fdf9786dcb96e4ba334204d622d612afb2fc770251f9b1ce32301ef217d`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`c253dc3879dd6d2ab75b0325c2f7ee94870185c8a8c2e7d985bb7eafcb91c58e`
and
`6b4dacdf16191077402d6d68c6056fe414d7af248ab60a75e5ede069a70426e4`.
The candidate metadata names commit `b6d58b1d2` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. The recorded Clang/runtime divergence needs no CPPGM
change. Two unexpected class rows remain in
`pa23/tests/general/300-boost-enable-if-type-condition-static-keyword-overload.t`
and
`pa24/tests/general/500-dependent-qualified-sizeof-static-member.t`. They form
the next class-use cluster. Inception remains forbidden.

## Semantic static-member publication checkpoint, 2026-08-10

Out-of-class static-member owner rows were promoted when any member variable
of the same class had a variable-instantiation event. That owner-only test
published definitions that Clang did not materialize in
`300-boost-enable-if-type-condition-static-keyword-overload.t` and
`500-dependent-qualified-sizeof-static-member.t`.

Pending owner rows now carry the static member name from the resolved binding.
Lifecycle events carry a copied semantic key made from the stable owner type
and member name. Final collection promotes a pending row only when that full
key has a public variable-instantiation event. The copied name is required:
debugger inspection of the first pointer-based implementation found that a
reconstructed nested `ValueBinding` had already expired before final
collection. The semantic key also preserves the expected nested owner row in
`300-template-nested-static-member-out-of-class-definition.t`.

Two implementation-only expression paths no longer publish member-variable
lifecycle events. Object lifetime analysis synthesizes an id-expression for
the object whose initialization actions it is constructing, and constant
evaluation resolves a `sizeof` operand through a second unevaluated type
query. A narrow static-member publication scope covers those operations.
Static-member materialization and initializer replay still run, but the
public lifecycle observer is not entered. The existing unevaluated-expression
scope also prevents constant folding, output requirements, and lifecycle
publication while the ordinary `sizeof` operand is typed. No renderer,
source-name, source-location, fixture, or rendered-text rule was added.

The checkpoint removes the two remaining unexpected class-use rows and five
unexpected lifecycle rows. Three of the lifecycle rows are the synthetic
`formatter`, `out`, and `what` member probes in
`pa22/tests/general/400-partial-specialization-conversion-operator-pointer-binding.t`;
that witness is now byte-exact. The PA24 `sizes` witness is also byte-exact.
The PA23 `keyword::instance` class and lifecycle rows are gone, leaving only
the pre-existing missing
`std::is_same<boost::parameter::in_reference,
boost::parameter::consume_reference>::value` event.

Expanded convergence improves from 1,440 to 1,442 matching outputs and leaves
88 known mismatches with no missing output file. Class-use debt is now the
single recorded changed row in
`pa22/tests/general/400-template-template-fixed-prefix-pack-order.t`.
Function-call inventory is unchanged. Lifecycle debt now has 55 missing and
27 unexpected terminal facts in 39 tests, plus eight additional-definition-
demand warnings.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- ordinary and provenance compilers produce identical witness and LowIR output
  for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,321 records with no unknown
  producer and no unexercised producer site;
- class consolidation keeps 3,000 completed candidates, 3,303 early repeats,
  355 prepublication merges, and 2,645 collected occurrences; publications
  fall from 2,634 to 2,632 with the two rejected owner rows;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template and semantic boundary
  counts are unchanged;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes,
  `ClassInfo` 1,136 bytes, and `TemplateLifecycleTransition` 80 bytes.

The optional full repository unit discovery repeated its already documented
result: 249 tests ran with one skip and the unrelated
`BatchTimeoutHarnessTests.test_driver_assignment_wrapper_uses_worker_script`
error because its temporary `basic.my.impl.exit_status` file is absent. The
focused 60-test checkpoint suite passes in full.

The ordinary binary is 17,162,504 bytes. Its Mach-O `__TEXT` segment remains
13,049,856 bytes, `__DATA_CONST` remains 61,440 bytes, and `__DATA` remains
442,368 bytes. It contains no provenance symbols. The frozen ordinary binary
is `/tmp/cppgm-semantic-static-member-ordinary-final2-20260810`, SHA-256
`27f85992415e0abbe2a7cbe00bceeda12ca2a2979e7636922d0eb3d6235243cc`.

The final ordinary convergence report is
`/tmp/cppgm-semantic-static-member-final2-convergence-20260810.json`, SHA-256
`9f55a7498c392e29212712451b3ec1131309ccfb2ede828046d49662ed0eb473`.
The provenance analysis and convergence reports are
`/tmp/cppgm-semantic-static-member-provenance-final2-20260810.ngowea/provenance-analysis.json`
and
`/tmp/cppgm-semantic-static-member-provenance-final2-convergence-20260810.json`,
with SHA-256 values
`641a4acfe8cb06750a0616def057e3b88db8fed5d00cad44914f5151e6211bba`
and
`15a823729899bc009a28aa4d4616ce624f6238d5c3b9f468979d88329eaeaa29`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-semantic-static-member-final2-output-manifest-20260810.txt`,
SHA-256
`bbb2d4eb965065efe3309790e05089a5e678c14b6337fce80cc946d765a4e55e`.
The broad report is
`/tmp/cppgm-semantic-static-member-final2-broad-20260810.log`, SHA-256
`a1cd8904fe388e5eb720df340edf24fa22f8ef74000895e19bbb34645dc896b2`.

The materialization, structure-size, and zero-finding text-reparse reports
retain their parent SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`,
and
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`.
The template and semantic boundary reports retain SHA-256 values
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`
and
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.15% | +0.83% | -3.91% | `/tmp/cppgm-semantic-static-member-final2-perf-fixed-20260810.txt` |
| Same-location ordering parent | -0.17% | +1.10% | +0.04% | `/tmp/cppgm-semantic-static-member-final2-perf-parent-20260810.txt` |

The candidate medians are 173,986,033,714 instructions, 763,371,520 bytes
maximum RSS, and 569,851,904 bytes peak footprint. The raw candidate record is
`/tmp/cppgm-semantic-static-member-final2-raw-candidate-20260810.json`,
SHA-256
`35272cd94e94e96a960b810bf08e9bf128d880fc011b29db6c8207f9bacedc62`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`88a8c51851d7e9be8395dbfa9b3a05f4f367040f69cdf31c2430170698899cbb`
and
`42158fd8b5bd5e4e311b2b708467e083061f73839fddeac334e69b62214285c1`.
The candidate metadata names commit `44f13cab6` because the measurements cover
this uncommitted checkpoint.

Phase 3 remains open. The only remaining class-use row is the recorded
Clang/runtime oracle divergence; the next coherent work should move to the
function-call or lifecycle inventory. Inception remains forbidden.

## Semantic value-entity checkpoint, 2026-08-10

This promotable Phase 4 slice canonicalizes resolved non-type template
arguments when they identify lifecycle entities. Concrete class
specialization keys already carried resolved enum values, but their public
class and variable-instantiation names preferred retained source syntax such
as `LinkMode`, `BaseHookType`, `ck::by`, and `bi::aq`. An unsigned non-type
argument whose stored value was `-1` was likewise printed as signed instead
of according to its semantic type width.

Semantic class and entity output now prefers the retained resolved enumerator
and formats integral values through their actual unsigned type. Source-binding
output continues to preserve its source-first spelling. The implementation
uses the existing structured template arguments and semantic enum facts; it
adds no source-name, source-location, fixture, rendered-text, or reparsing
rule.

The checkpoint removes eight missing/unexpected variable-instantiation pairs
without adding an occurrence. They were distributed across four tests:

- one pair in
  `pa23/tests/general/400-enum-nttp-cstyle-cast-default-rebind.t`;
- one pair in
  `pa23/tests/general/500-owner-enum-nontype-result-sfinae.t`;
- five pairs in
  `pa24/tests/general/400-concrete-recursive-node-layout-retry.t`;
- one pair in
  `pa24/tests/general/500-constructor-template-default-constraint-previous-param.t`.

Only these four witness files change relative to the semantic static-member
parent, and all LowIR files remain byte-identical. The first two witnesses are
now exact. The recursive-node witness retains unrelated missing class and
function lifecycle facts, while the constructor witness retains an unrelated
function-call rejection-payload difference.

Expanded convergence improves from 1,442 to 1,444 matching outputs. The final
inventory has 1,530 references, 86 known mismatches, eight warning outputs,
and no missing actual file. Class-use remains one changed row in one test.
Function-call inventory is unchanged at 36 changed, 18 missing, 13
unexpected, and one ordering-only occurrence across 55 tests. Lifecycle debt
improves from 55 missing and 27 unexpected facts in 39 tests to 47 missing
and 19 unexpected facts in 36 tests, plus the same eight explained
additional-definition-demand warnings.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the preserved strict manifest remains byte-exact at 1,305/1,305;
- ordinary and provenance compilers produce byte-identical witness and LowIR
  output for all 3,060 files;
- all 1,530 provenance sessions flush, producing 61,327 records with no
  unknown producer and no unexercised producer site;
- the six-record increase from the immediately preceding diagnostic run is
  rejected class-materialization decision multiplicity; public output and
  all class consolidation counts are unchanged;
- class consolidation keeps 3,000 completed candidates, 3,303 early repeats,
  355 prepublication merges, 2,645 collected occurrences, and 2,632
  publications;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- the focused convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template and semantic boundary
  counts are unchanged;
- the structure-size report is byte-identical to the parent. `Type` remains
  280 bytes, `TemplateArgument` 136 bytes, `TemplateIdSyntax` 160 bytes,
  `ClassInfo` 1,136 bytes, and `TemplateLifecycleTransition` 80 bytes.

The ordinary binary is 17,166,600 bytes. Its Mach-O `__TEXT` segment is
13,053,952 bytes, one page larger than the parent; `__DATA_CONST` remains
61,440 bytes and `__DATA` remains 442,368 bytes. It contains no provenance
symbols. The frozen ordinary binary is
`/tmp/cppgm-semantic-value-entities-final-ordinary-20260810`, SHA-256
`40f719f21fd20e2ba05f60ef22328cc97194102e92d55ebd038e891bf12c2d1e`.

The final ordinary convergence report is
`/tmp/cppgm-semantic-value-entities-final-convergence-20260810.json`, SHA-256
`3249d3e36cc96900a80bd315fd04f760e9db4e3307d83fe3df0e4e0242a2b40e`.
The provenance analysis and convergence reports are
`/tmp/cppgm-semantic-value-entities-final-provenance-20260810.XuLPWT/provenance-analysis.json`
and
`/tmp/cppgm-semantic-value-entities-final-provenance-convergence-20260810.json`,
with SHA-256 values
`e7b117282575d93477c1ed95a855f59cb2762cbef95bed495a2b69eff07df821`
and
`0a93764ed3175f807676e5b772347c9ee10549dbfd31f9822c5d2bc29953c6b9`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-semantic-value-entities-final-output-manifest-20260810.txt`,
SHA-256
`8489f8fe8b9f62b1a21e5ec531d0604c124148cc29f61be64ef120d12a6036ca`.
The broad report is
`/tmp/cppgm-semantic-value-entities-broad-20260810.log`, SHA-256
`f2d0c710e7a61931549ecacfaa1425a73d3f41c702ffca7b1bd01a4ca9599068`.

The materialization, structure-size, and zero-finding text-reparse reports
retain their parent SHA-256 values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`,
and
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`.
The template and semantic boundary reports retain SHA-256 values
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`
and
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`.
The 60-test helper report is
`/tmp/cppgm-semantic-value-entities-helper-tests-20260810.log`, SHA-256
`4b0b2ee09967b4e688e4fa94a2730de1b3a9d8fcd4c379b84a24d42a9dbba573`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.94% | +0.22% | -3.89% | `/tmp/cppgm-semantic-value-entities-final-perf-fixed-20260810.txt` |
| Semantic static-member parent | +0.22% | -0.60% | +0.01% | `/tmp/cppgm-semantic-value-entities-final-perf-parent-20260810.txt` |

The candidate medians are 174,369,067,851 instructions, 758,771,712 bytes
maximum RSS, and 569,933,824 bytes peak footprint. Wall time is recorded but
is not the gate signal. The raw candidate record is
`/tmp/cppgm-semantic-value-entities-final-raw-candidate-20260810.json`,
SHA-256
`f7b88e61064502b07417deee8a85d61e46fb3c40fd54ce728972c694584f0a27`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`e06b85a7efc7ad74a8440c6a27da782c21bbd00b3586c31be1b90c02670e2104`
and
`bc987c962335312b252fd96b15694bbb60a1840c695e0322b1446445929f26b6`.
The candidate metadata names commit `e54dfe019` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open. Its lifecycle inventory still contains 47 missing and
19 unexpected terminal facts across 36 tests, with eight recorded
additional-definition-demand warnings. The single recorded class-use oracle
divergence and function-call inventory also remain. Inception remains
forbidden.

## Unevaluated function lifecycle checkpoint, 2026-08-11

This promotable Phase 4 slice prevents unevaluated expressions from
publishing function lifecycle output. `decltype` and `noexcept` still require
overload resolution, return-type analysis, constant evaluation, and in some
cases a function body. Those semantic operations continue. The analysis
policy suppresses direct-call materialization intent, and the output tracker
rejects the terminal function-instantiation or definition transition that an
evaluated call would publish.

The first implementation stopped function-body acquisition globally. It
removed the spurious lifecycle facts but broke the PA21
constexpr/noexcept/decltype static-assert control and the PA34 libstdc++
uninitialized-copy local-alias control. The final implementation keeps body
acquisition available for type and constexpr correctness, carries an
unevaluated-aware call policy through overload selection and conversion, and
declines lifecycle claims at the output-tracking boundaries. Both controls
pass in the final build.

The focused matrix covers seven affected families:

- a forward-only member operator;
- a member-template call operator queried by `noexcept`;
- qualified-alias member deduction;
- a converting constructor with a concrete owner pack;
- a hidden-friend query in `decltype` and `noexcept`;
- out-of-class member-template cache reset behavior;
- a function-type pack template argument.

Six target witnesses are exact and the seventh retains only its known
same-location call-ordering difference. Three of four independent controls
are exact; the fourth retains its unchanged pre-existing ensure-definition
warning. All eleven focused LowIR outputs are byte-identical to their
references.

Expanded convergence improves from 1,444 to 1,450 matching outputs. The final
inventory has 1,530 references, 80 known mismatches, one warning output, and
no missing actual file. Class-use remains one changed row in one test.
Function-call inventory remains 36 changed, 18 missing, 13 unexpected, and
one ordering-only occurrence across 55 tests. Lifecycle debt now has 47
missing and 11 unexpected facts across 28 tests, plus one explained
additional-definition-demand warning. The parent had the same 47 missing
facts, 19 unexpected facts, and eight additional-definition-demand warnings.

Correctness and diagnostic evidence from the final Homebrew-Clang builds:

- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- ordinary and provenance compilers produce byte-identical witness and LowIR
  output for all 1,530 sessions;
- all 1,530 provenance sessions flush, producing 61,255 records with no
  unknown producer attempt and no unexercised producer site;
- alias and class consolidation counts remain unchanged: class-use keeps
  3,000 completed candidates, 3,303 early repeats, 355 prepublication merges,
  2,645 collected occurrences, and 2,632 publications;
- all 16 typed class-materialization admissions remain unchanged; only six
  rejected lookup-only decisions disappear relative to the parent;
- the focused convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template, semantic-boundary,
  and structure-size reports are byte-identical to the parent.

The historical dynamic class-materialization audit builder still contains
its pre-existing stale five-accept expectation. This checkpoint does not
count that builder as a passing gate. Its focused unit/helper suite passes,
while the public class route, typed admissions, and consolidation counts
above provide the parent-ratchet evidence for this non-class change.

The ordinary convergence report is
`/tmp/cppgm-unevaluated-function-lifecycle-final-20260811.json`, SHA-256
`62476a7c9495a9f26064234ed882e7e5d813460c404b623288d9ba2392184cb5`.
The provenance analysis and provenance-correlated convergence reports are
`/tmp/cppgm-unevaluated-function-lifecycle-provenance-analysis-20260811.json`
and
`/tmp/cppgm-unevaluated-function-lifecycle-provenance-convergence-20260811.json`,
with SHA-256 values
`798832b8fdb75b18330cb86c59790cbb68e2d163dff5d975544f41362631f4d2`
and
`36fea8402b1692442c382894f17dab57e9a9e8eedeb1db0f6387b952751a3a0a`.
The byte-identical ordinary/provenance output manifest is
`/tmp/cppgm-unevaluated-function-lifecycle-output-manifest-20260811.txt`,
SHA-256
`4d54a18bc35fcbf90d82be831ee39711b85d24d33d38a6dda024bd9e77f74146`.
The broad report is
`/tmp/cppgm-unevaluated-function-lifecycle-broad-final-20260811.log`,
SHA-256
`78b100ad578e57206924bbbea8697635cb1593247a264698198ceec746abcbbb`.

The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The 60-test helper report is
`/tmp/cppgm-unevaluated-function-lifecycle-helper-tests-20260811.log`,
SHA-256
`d796303094fc166d4c0a8ca259345ca0ab0f6da96fd9245a5b09b49542cbb20c`.

The ordinary binary is 17,166,840 bytes, 240 bytes larger than the parent.
Its Mach-O `__TEXT`, `__DATA_CONST`, and `__DATA` segments remain unchanged at
13,053,952, 61,440, and 442,368 bytes. It contains no provenance symbols. The
frozen binary is
`/tmp/cppgm-unevaluated-function-lifecycle-ordinary-20260811`, SHA-256
`446a9c092ab8c5666aeb5a9b2137e21fb53ec1774b53754195a76fdf0731fde5`.

The final three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.00% | -0.70% | -4.00% | `/tmp/cppgm-unevaluated-function-lifecycle-perf-fixed-20260811.txt` |
| Semantic value-entity parent | -0.06% | -0.92% | -0.11% | `/tmp/cppgm-unevaluated-function-lifecycle-perf-parent-20260811.txt` |

The candidate medians are 174,264,985,242 instructions, 751,800,320 bytes
maximum RSS, and 569,323,520 bytes peak footprint. Wall time is informational.
The raw candidate record is
`/tmp/cppgm-unevaluated-function-lifecycle-raw-candidate-20260811.json`,
SHA-256
`8fe6dc115c845f7a4abde623209033ebd563854f01973058fcacc4f36554c019`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`033672db6633e6a6861b0924dbb4502cc0f9a5de324ec98f16bab1c422764130`
and
`063af7b5467dbd42a32295f0a68e6b1cea1acb2f8a913b3fa2da18fba24eda92`.
The candidate metadata names commit `239b5e16a` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open. This checkpoint requires passing focused, broad,
provenance-equivalence, audit, binary, and performance gates. The recovery
plan tracks convergence debt as a separate inventory. Its remaining 47
missing and 11 unexpected lifecycle facts, one lifecycle warning, class-use
divergence, and function-call inventory remain future work. Inception remains
forbidden.

## Function-local class lifecycle checkpoint, 2026-08-11

This Phase 4 slice publishes lifecycle facts for named classes declared in
function-template specializations. Class collection now records finalization
and instantiation after a complete, nondependent local class acquires template
identity. Function output records a local member definition only after the
compiler emits that definition. The public projection keeps the class facts
and the terminal member-instantiation fact while omitting internal
require/ensure-definition companion transitions.

Local lifecycle identities include the enclosing function specialization and
its semantic parameter spellings. This distinguishes, for example, the two
`visit` specializations in the PA19 provenance fixture. The same identity
qualifies local constructors, destructors, and methods. Two diagnostic flags
mark local classes and their members without changing the 80-byte ordinary
`TemplateLifecycleTransition` structure. The witness normalizer also removes
the stray space before a top-level enclosing-function parameter list.

Three target witnesses and their LowIR outputs now match byte for byte:

- `pa19/tests/general/100-local-dependent-base-lookup-provenance.t`;
- `pa19/tests/general/300-function-template-local-class-specialization-identity.t`;
- `pa20/tests/general/200-member-init-covarying-type-index-pack.t`.

These targets account for ten missing lifecycle facts: six local-class
finalization/instantiation facts and four local-member function-instantiation
facts. Four independent local-class controls retain exact witness and LowIR
output:

- `pa19/tests/general/100-local-type-cross-namespace-operator-template.t`;
- `pa19/tests/general/100-template-member-local-class-functional-cast-argument.t`;
- `pa19/tests/general/100-template-local-anonymous-union-nested-struct.t`;
- `pa19/tests/general/100-nested-class-template-local-class-argument.t`.

Expanded convergence improves from 1,450 to 1,453 matching outputs. The
current inventory contains 1,530 references, 77 known mismatches, one warning
output, and no missing actual file. Class-use remains one changed row in one
test. Function-call inventory remains 36 changed, 18 missing, 13 unexpected,
and one ordering-only occurrence across 55 tests. Lifecycle debt falls from
47 to 37 missing facts and remains at 11 unexpected facts. It now spans 25
tests and retains one explained additional-definition-demand warning.

The final Homebrew-Clang validation records:

- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and diagnostic witness sessions complete;
- all 3,060 ordinary and diagnostic witness/LowIR files match byte for byte;
- all 1,530 provenance sessions flush, producing 61,303 records with no
  unknown producer attempt and no unexercised producer site;
- lifecycle attempts rise from 6,037 to 6,058, while alias and class
  consolidation counts remain unchanged;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- template-boundary, semantic-boundary, and structure-size reports match the
  parent byte for byte.

The ordinary convergence report is
`/tmp/cppgm-function-local-lifecycle-convergence-final-20260811.json`, SHA-256
`54249a26583624e70bdf88c701bcce4289e9b7005583e4d7af1fe0cc09ee58f3`.
The provenance trace directory is
`/tmp/cppgm-function-local-lifecycle-provenance-final-20260811.CDNxM6`.
The provenance analysis and correlated convergence reports are
`/tmp/cppgm-function-local-lifecycle-provenance-analysis-final-20260811.json`
and
`/tmp/cppgm-function-local-lifecycle-provenance-convergence-final-20260811.json`,
with SHA-256 values
`9ef15f74cdfa7a7077a97da432d5a6ed76b72ed5a11c0dfe25f25d2a3c7656e1`
and
`491f888fb47380772d261b95c3e127e19732ed08b29d4121b681bbff2aad98e1`.
The 3,060-file ordinary/diagnostic manifest is
`/tmp/cppgm-function-local-lifecycle-output-manifest-final-20260811.txt`,
SHA-256
`739f5f3cbaebccc085f1dbf9104df985f82f5368688db49f84c91d7374941467`.
The broad report is
`/tmp/cppgm-function-local-lifecycle-broad-final-20260811.log`, SHA-256
`535eaf3777ab5311f1c6fad6af1b8e2cea690dee48edfc2612a04ca50f6d1ce3`.

The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The 60-test helper report is
`/tmp/cppgm-function-local-lifecycle-helper-tests-20260811.log`, SHA-256
`34140db7ba6d61034a26db5fbb60af90796d0f7742a2fdfc0ae74fe5a3cf5b7f`.

The ordinary binary is 17,167,592 bytes, 752 bytes larger than the parent. Its
Mach-O `__TEXT`, `__DATA_CONST`, and `__DATA` segments remain unchanged at
13,053,952, 61,440, and 442,368 bytes. It contains no provenance symbols. The
frozen binary is
`/tmp/cppgm-function-local-lifecycle-ordinary-20260811`, SHA-256
`a7b17160064efcd9f7fee23588e8e0d541a07e5d84c4bf6b321f5846ce4a4393`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.12% | +0.05% | -3.97% | `/tmp/cppgm-function-local-lifecycle-perf-fixed-20260811.txt` |
| Unevaluated-function parent | -0.13% | +0.75% | +0.02% | `/tmp/cppgm-function-local-lifecycle-perf-parent-20260811.txt` |

The candidate medians are 174,038,816,975 instructions, 757,473,280 bytes
maximum RSS, and 569,450,496 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-function-local-lifecycle-raw-candidate-20260811.json`, SHA-256
`c6621eab23329828a7579c9c10b23a49db2acfd096da8e179b2190242d9ed973`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`4db57c3979924aea62b980c60bb23b041028438278fe1f9ebc8d12f37b0752df`
and
`58ad00e33ed8b389e8c9558c509041ad6763e731de8e646d5a169f6405f5ceda`.
The candidate metadata names commit `181a660ce` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open with 37 missing and 11 unexpected lifecycle facts across
25 tests and one lifecycle warning. The class-use divergence and function-call
inventory remain. Inception remains forbidden.

## Static-member definition-demand checkpoint, 2026-08-11

This Phase 4 slice separates a static member's semantic-value observation from
an expression that actually demands the member definition. A typed
`DefinitionDemand` origin now joins ordinary semantic use and retained
dependency replay. Definition demands bypass source-type collection filters,
publish a source-required variable-instantiation transition, and may replay a
static-member initializer when the enclosing source use is public. A later
definition demand can upgrade an earlier constant-cache semantic claim exactly
once, so eager constant folding cannot hide the terminal lifecycle fact.

Qualified constant reads, materialized leaf member reads, and successful
structured-bool evaluations now send the typed definition demand. Unqualified
cache reads remain semantic observations; this preserves the suppression of
direct cached NTTP constants such as `_Rp`, `Limits::max`, and `M<int>::v`.
Structured-bool dependency discovery no longer recurses through every type
argument, which removes the false
`integral_constant<bool, false>::value` publication. The namespace-alias
fallback scan is also removed so selected/result types do not expose unrelated
members such as `forward_reference::value`.

Two narrow completion paths cover the remaining positive cases in this group.
A concrete primary selection reached after reentrant partial matching commits
the provisional primary's retained value dependencies, because those demands
participated in choosing the enclosing specialization rather than a rejected
candidate. A materialized template conversion operator revisits its concrete
body on the witness path when definition acquisition completed without
ordinary function output, allowing folded constexpr conversions to expose
their body-owned value demand. This body revisit is deliberately limited to
conversion operators; applying it to every acquired function produced
speculative constructor and function lifecycle facts in the rejected trial.

Five complete outputs leave the mismatch inventory, with no newly mismatching
output:

- `pa22/tests/general/400-recursive-pack-alias-carries-expanded-type-syntax.t`;
- `pa23/tests/general/300-boost-enable-if-type-condition-static-keyword-overload.t`;
- `pa23/tests/general/500-bool-alias-function-template-result-metadata.t`;
- `pa23/tests/general/500-constructor-pack-default-rewritten-pointer.t`;
- `pa24/tests/general/500-reentrant-static-query-callable-enable-if-cache.t`.

The variable facts also become exact in
`pa23/tests/general/400-member-alias-template-template-partial-deduction-owner.t`
and `pa23/tests/spec/100-explicit-member-template-leading-pack.t`; those two
outputs retain unrelated class/default-source differences.

Expanded convergence improves from 1,453 to 1,458 matching outputs. The
current inventory contains 1,530 references, 72 known mismatches, one warning
output, and no missing actual file. Class-use remains one changed row in one
test. Function-call inventory remains 36 changed, 18 missing, 13 unexpected,
and one ordering-only occurrence across 55 tests. Lifecycle debt falls from
37 to 24 missing facts and from 11 to ten unexpected facts. It now spans 19
tests and retains the one explained additional-definition-demand warning.

The final Homebrew-Clang validation records:

- the preserved pre-expansion strict manifest passes 1,305/1,305 byte for
  byte;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and diagnostic witness sessions complete;
- all 3,060 ordinary and diagnostic witness/LowIR files match byte for byte;
- all 1,530 provenance sessions flush, producing 61,819 records with no
  unknown producer attempt and no unexercised producer site;
- lifecycle attempts rise from 6,058 to 6,321 as definition-demand
  observations replace semantic-only observations;
- class materialization admits three additional conversion function-body
  scopes, while public class-use output remains unchanged;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, and all 23
  forbidden text-reparse categories remain zero;
- template-boundary, semantic-boundary, and structure-size reports match the
  parent byte for byte.

The ordinary convergence report is
`/tmp/cppgm-static-member-definition-demand-convergence-final-20260811.json`,
SHA-256
`da6daf21353aa5dcce9eaa449446813041f3957523fbd1e8874c658fecade129`.
The provenance trace directory is
`/tmp/cppgm-static-member-definition-demand-provenance-final-20260811.ADPkuf`.
The provenance analysis and correlated convergence reports are
`/tmp/cppgm-static-member-definition-demand-provenance-analysis-final-20260811.json`
and
`/tmp/cppgm-static-member-definition-demand-provenance-convergence-final-20260811.json`,
with SHA-256 values
`5f3e48c9759ebfaeb82665bef1348f1b658b71f369acf131dbbef4095f0d1c00`
and
`39e0b3010613093f326f06d927fc018fcbefa6c9fcf6d57746b7d50639274062`.
The 3,060-file ordinary/diagnostic manifest is
`/tmp/cppgm-static-member-definition-demand-output-manifest-final-20260811.txt`,
SHA-256
`b9f1861dc198b2e2c65c90866b3e3872a3bdbed0aa8888bf22ad8a3076865770`.
The broad report is
`/tmp/cppgm-static-member-definition-demand-broad-20260811.log`, SHA-256
`707b292e3b7f1da2ba45ac6fa125de4bfde2854f9528ca4f04f6fd2cf239b9bb`.

The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The 60-test helper report is
`/tmp/cppgm-static-member-definition-demand-helper-tests-20260811.log`,
SHA-256
`89dca6d45768101e3cdb4abd11a17579376aedc2934d91af7bd609e8749aab43`.

The ordinary binary is 17,167,792 bytes, 200 bytes larger than the parent. Its
Mach-O `__TEXT`, `__DATA_CONST`, and `__DATA` segments remain unchanged at
13,053,952, 61,440, and 442,368 bytes. It contains no provenance symbols. The
frozen binary is
`/tmp/cppgm-static-member-definition-demand-ordinary-20260811`, SHA-256
`2115cb1d26c3695d58323b5accb5415e7f0bb54447364fb1557a2efc27236a6b`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.13% | -0.58% | -3.99% | `/tmp/cppgm-static-member-definition-demand-perf-fixed-20260811.txt` |
| Function-local-lifecycle parent | -0.00% | -0.63% | -0.01% | `/tmp/cppgm-static-member-definition-demand-perf-parent-20260811.txt` |

The candidate medians are 174,034,768,225 instructions, 752,672,768 bytes
maximum RSS, and 569,384,960 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-static-member-definition-demand-raw-candidate-20260811.json`,
SHA-256
`64ddb7d7c63a63e97ff7fced23faf0798b23ba5fce3d72a69ba5fe6ea47bca18`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`e461ae26b2879d00241f435473984d1dd8ca3bfe8f35d5b2c51111196c5d34cf`
and
`6826e42a259123648983fbc319d3f6f61dc0fd942950c9d85cc34fb06287b180`.
The candidate metadata names commit `1baac354a` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open with 24 missing and ten unexpected lifecycle facts across
19 tests and one lifecycle warning. The remaining static-member-only debt is
two missing and five unexpected variable-instantiation facts in signature and
candidate-transaction cases. The class-use divergence and function-call
inventory remain. Inception remains forbidden.

## Static-value signature-transaction checkpoint, 2026-08-11

This Phase 4 slice closes the remaining seven-fact static-member group by
separating committed signature dependencies from speculative template and
conversion probes. A nested function-template signature now collects member
value dependencies transactionally. The dependencies follow the resulting
function binding and publish when that binding is acquired or emitted; a
rejected nested signature does not leak its fixed predicate observations.
Conversion-function-template target probes use the same transaction and
publish only a viable candidate for the requested target when no ordinary
standard conversion already satisfies the request.

Structured dependency discovery now walks concrete components inside an
otherwise dependent template argument and expands resolved alias-template
targets without requiring every argument to be nondependent. The scan has an
alias recursion guard and pauses witness source capture while inspecting the
target. A concrete class's durable source-member classification distinguishes
fixed predicates from dependent initializers: fixed sibling predicates remain
conditional during nested substitution, while a demanded dependent initializer
can publish the semantic value facts that define it. Full materialization of a
concrete class also scans the structured declaration signature of method-like
member templates, excluding their bodies.

Two narrower rules prevent the positive discovery paths from restoring false
facts. A fixed current-class member used only while materializing a source type
node does not emit an ordinary lifecycle observation. Function signature
dependencies bubble through an enclosing collection before publication, so a
cached signature behaves like a newly instantiated one. These are typed
semantic decisions; the implementation adds no fixture, name, source-location,
or source-text filter.

The checkpoint removes these five unexpected variable-instantiation facts:

- `M<int>::v` from
  `pa20/tests/general/100-lazy-static-value-before-shadowing-typedef.t`;
- two rejected `is_same` predicates from
  `pa23/tests/general/400-defaulted-sfinae-conversion-function-template-symbol.t`;
- `first::is_same<X, int>::value` from
  `pa23/tests/spec/300-dependent-nontype-result-lexical-template-lookup-sfinae.t`;
- `has_get<const json_like &, json_like>::value` from
  `pa23/tests/spec/400-dependent-decltype-member-template-conversion-operator.t`.

It restores both expected definition demands in
`pa23/tests/general/500-reference-member-dependent-variadic-return.t`:
`integral_constant<bool, false>::value` and
`integral_constant<unsigned long, 2>::value`.

Three complete outputs leave the mismatch inventory, with no newly mismatching
output. Expanded convergence improves from 1,458 to 1,461 matching outputs.
The current inventory contains 1,530 references, 69 known mismatches, one
warning output, and no missing actual file. Class-use remains one changed row
in one test. Function-call inventory remains 36 changed, 18 missing, 13
unexpected, and one ordering-only occurrence across 55 tests. Lifecycle debt
falls from 24 to 22 missing facts and from ten to five unexpected facts. It now
spans 14 tests and retains the one explained additional-definition-demand
warning.

The final Homebrew-Clang validation records:

- the focused seven-fact set has the expected fact-level result, and all 28
  nearby regression controls remain exact;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and provenance witness sessions complete;
- all 3,060 ordinary/provenance witness and LowIR files match byte for byte;
- all 1,530 provenance sessions flush, producing 69,895 records with no
  unknown producer attempt and no unexercised producer site;
- lifecycle attempts fall from 6,321 to 6,298 as the five rejected facts are
  removed and the two dependent-initializer facts are restored;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and structure-size reports match the parent byte for byte.

The focused fact and 28-fixture control artifacts are
`/tmp/cppgm-seven-facts-dependent-publish.5UGuyy` and
`/tmp/cppgm-seven-facts-regression-final2.ZYzqDl`. The ordinary convergence
report is
`/tmp/cppgm-seven-static-values-convergence-final2-20260811.json`, SHA-256
`f4cbf5ddcb66121e64e7e376ba50d65e3c49b72770ef97815f25f8a41613d285`.
The provenance trace directory is
`/tmp/cppgm-seven-static-values-provenance-final-20260811.ID314X`. The
provenance analysis and correlated convergence reports are
`/tmp/cppgm-seven-static-values-provenance-analysis-final-20260811.json` and
`/tmp/cppgm-seven-static-values-provenance-convergence-final-20260811.json`,
with SHA-256 values
`10351e7b56ded2e4e309c912e6100f99b21159272b51d0a3ebf5978c6eaf6bcd`
and
`4a722e8692561a49faa48176fc6942012ac34c81625bc466bfcfa3f78d4956cf`.
The 3,060-file ordinary/provenance output manifest is
`/tmp/cppgm-seven-static-values-output-manifest-final-20260811.txt`, SHA-256
`b6fbeb6bf01ab1b25d2bed872a64d64d89ed67ad219d4f5907df81fb7f4c6493`.

The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.
The 60-test helper report is
`/tmp/cppgm-seven-static-values-helper-tests-20260811.log`, SHA-256
`48bbbdc24aaa676955756ee6f7ca6f80241b0a93814a4e89b9d906ad767878c0`.

The ordinary binary is 17,182,936 bytes, 15,144 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by 8,192 bytes to 13,062,144 bytes;
`__DATA_CONST` and `__DATA` remain unchanged at 61,440 and 442,368 bytes. It
contains no witness-provenance symbols. The frozen binary is
`/tmp/cppgm-seven-static-values-ordinary-20260811`, SHA-256
`3cb9a38560c3b05d888fd29c6e8034ea8f9ceac4ec1d17e373e775bfc4a4de24`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.96% | -0.48% | -4.00% | `/tmp/cppgm-seven-static-values-perf-fixed-20260811.txt` |
| Static-member-definition parent | +0.17% | +0.10% | -0.01% | `/tmp/cppgm-seven-static-values-perf-parent-20260811.txt` |

The candidate medians are 174,336,505,809 instructions, 753,442,816 bytes
maximum RSS, and 569,323,520 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-seven-static-values-raw-candidate-20260811.json`, SHA-256
`b9e87e3261a8c1042b9bb38e0d61bccecdb16450724b10b2bfba311d741347ab`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`f101e62b07565c1fefa9cfdce9e88a053273140fc7e3a926ae1e77adc1ba8077`
and
`a0b51c0713d56ffb559251bb4f081999607f7ca2271aff7b35e680c4fb37ca48`.
The candidate metadata names commit `a50e3035a` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open with 22 missing and five unexpected lifecycle facts
across 14 tests and one lifecycle warning. The class-use divergence and
function-call inventory remain. Inception remains forbidden.

## Member-class reference lifecycle checkpoint, 2026-08-11

This Phase 4 slice moves member-class instantiation publication to the typed
class-reference completion boundary. Once semantic reference members have
been collected, a non-template nested class with a concrete enclosing
template owner records and publishes its class-instantiation transition. The
observation is repeated after the analyzer-level ensure so a class first
collected during paused source capture can publish on its later committed
use. Named function-local classes retain their separate lifecycle owner.

Anonymous-union reference storage now retains its unnamed source-class
identity. A non-function-local unnamed member delegates completion to the
anonymous-member lifecycle path, preserving the public `anonymous union`
entity, while function-local unnamed classes retain their existing finalized
and instantiated transitions. A typed concrete outer-owner check permits a
stale dependent bit without admitting a genuinely dependent class.

Nested class-template specializations no longer borrow the lifecycle fact for
a non-template member class, and the old nested completion helper excludes
them as well. Explicit class-template finalization now uses the canonical
qualified witness entity, repairing `box` to `ns::box`. These decisions use
class declarations, template bindings, and scope ownership; no fixture,
source-location, rendered-name, or source-text filter was added.

The checkpoint restores eight missing lifecycle facts and removes two
unexpected facts. Six complete outputs become exact, with no newly
mismatching output. Expanded convergence improves from 1,461 to 1,467 matching
outputs. The current inventory contains 1,530 references, 63 known
mismatches, no warning output, and no missing actual file. Class-use remains
one changed row in one test. Function-call inventory remains 36 changed, 18
missing, 13 unexpected, and one ordering-only occurrence across 55 tests.
Lifecycle debt falls from 22 to 14 missing facts and from five to three
unexpected facts, now spanning eight tests.

The remaining class-only exception is the unexpected
`basic_tree<int, int, int>::iterator` instantiation in
`pa19/tests/spec/300-current-specialization-nested-constructor-param-alias.t`.
The missing `w<base_node<recursive_slist>*>::x` class fact in
`pa24/tests/general/400-concrete-recursive-node-layout-retry.t` follows the
still-missing `base_node<recursive_slist>::base_node` function demand and is
assigned to the next function/definition lifecycle slice. The other residual
facts are function instantiations or definition demands, including the
adaptor member pair, allocator assignment, extractor call, `N::B::pick`,
`N::pair`, and `box<int>::add`/`touch` cases.

The final Homebrew-Clang validation records:

- the focused member-class fact set and its function-local, dependent-owner,
  anonymous-union, and nested-class-template controls have the expected
  exact lifecycle sets;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and provenance witness sessions complete;
- all 6,120 saved ordinary/provenance witness and LowIR files match byte for
  byte;
- all 1,530 provenance sessions flush, producing 69,899 records and 6,300
  lifecycle attempts with no unknown producer attempt and no unexercised
  producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and structure-size reports match the parent byte for
  byte.

The ordinary convergence report is
`/tmp/cppgm-member-class-reference-convergence-inventory2-20260811.json`,
SHA-256
`6b09caf59535b1eeb348f9877aa0d33855fc72909a47830c716be93a004b82fa`.
The provenance trace directory is
`/tmp/cppgm-member-class-reference-provenance-20260811.ZS3sca`. The provenance
analysis and correlated convergence reports are
`/tmp/cppgm-member-class-reference-provenance-analysis-20260811.json` and
`/tmp/cppgm-member-class-reference-provenance-convergence-20260811.json`,
with SHA-256 values
`95f6d0814613d1999c2c3d44c8d3eae97ac219cf5deeb6cc0b70b11ef251d22e`
and
`5f6a9b8af83247372141b404f772ee759927f03168178c8c5f4788b8cee0b993`.
The 6,120-file ordinary/provenance output manifest is
`/tmp/cppgm-member-class-reference-output-manifest-20260811.txt`, SHA-256
`a5ac06c4dee5c719626cafbae955b90d468e7a82fe5e5b3f52468e99e7b29853`.
The byte-difference manifest is empty.

The broad and 60-test helper reports are
`/tmp/cppgm-member-class-reference-broad-20260811.log` and
`/tmp/cppgm-member-class-reference-helper-tests-20260811.log`, with SHA-256
values
`33addf729d3c116c74cfbfef28ac99e17dee46296dcf8705a1c9a257e7d796b8`
and
`836c4542e9a125277ccc4239235c4a3b603f46597085d475d0a7182cd74d641f`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary is 17,183,296 bytes, 360 bytes larger than the parent. Its
Mach-O `__TEXT`, `__DATA_CONST`, and `__DATA` segments remain unchanged at
13,062,144, 61,440, and 442,368 bytes. It contains no witness-provenance
symbols. The frozen binary is
`/tmp/cppgm-member-class-reference-ordinary-20260811`, SHA-256
`d9b8f4071cd2b52b2d6c91dac4d183e143deb392681063252a39f7f4a1f5a1d5`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.10% | -0.08% | -4.04% | `/tmp/cppgm-member-class-reference-perf-fixed-20260811.txt` |
| Static-value parent | -0.15% | +0.40% | -0.05% | `/tmp/cppgm-member-class-reference-perf-parent-20260811.txt` |

The candidate medians are 174,079,089,956 instructions, 756,486,144 bytes
maximum RSS, and 569,053,184 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-member-class-reference-raw-candidate-20260811.json`, SHA-256
`50a2bf12d96b49222f68090a387afe940ac648c5c47fe4b10fef09cc16ae29ab`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`7742fef73a89399c01818c707e7692b4a2e424ee21ec74e797b420cf667aeaa2`
and
`0b1287b7f3ee62ca280c5f61ce50980fbdfb72d8761e29b4d4e3962adac2ac93`.
The candidate metadata names commit `ab68479d5` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open with 14 missing and three unexpected lifecycle facts
across eight tests. The class-use divergence and function-call inventory
remain. Inception remains forbidden.

## Function definition-demand visibility checkpoint, 2026-08-11

This Phase 4 slice makes ordinary closure visibility follow committed typed
function demands. A specialization that has not materialized a definition is
no longer hidden when the same semantic entity has a direct
`RequireDefinition` transition. This restores declaration-only template
specializations selected from instantiated bodies without admitting
speculative candidates that never acquire a definition requirement.

Explicit class instantiation now distinguishes a function definition owned by
the explicit instantiation from a nested or member-template definition
required later. The lifecycle event carries the binding's explicit-definition,
constructor, member-function-template, and enclosing-closure facts. Direct
members already instantiated by an explicit class definition remain hidden as
definition demands; a nested constructor reached through a committed function
body, and a member template excluded from extern-class suppression, remain
visible. An explicitly defaulted copy or move assignment carries its own typed
classification and publishes the required-definition fact without a false
function-instantiation outcome.

The implementation changes only lifecycle event metadata and closure output
policy. It does not change source-use admission, parse source text, inspect a
fixture name or location, or manufacture an event absent from the typed
lifecycle table.

The checkpoint restores 13 missing lifecycle facts and removes one unexpected
terminal fact. Five complete outputs become exact, with no newly mismatching
output. Expanded convergence improves from 1,467 to 1,472 matching outputs.
The current inventory contains 1,530 references, 58 known mismatches, no
warning output, and no missing actual file. Class-use remains one changed row
in one test. Function-call inventory remains 36 changed, 18 missing, 13
unexpected, and one ordering-only occurrence across 55 tests. Lifecycle debt
falls from 14 to one missing fact and from three to two unexpected terminal
facts, now spanning three tests.

The exact residual lifecycle set is:

- unexpected `basic_tree<int, int, int>::iterator` class instantiation in
  `pa19/tests/spec/300-current-specialization-nested-constructor-param-alias.t`;
- unexpected `std::forward` function instantiation in
  `pa24/tests/general/200-constructor-template-parameter-shadows-instantiated-type.t`;
- missing
  `boost::container::w<boost::container::base_node<recursive_slist> *>::x`
  class instantiation in
  `pa24/tests/general/400-concrete-recursive-node-layout-retry.t`.

The final Homebrew-Clang validation records:

- the adaptor nested-member, defaulted assignment, extractor, dependent-pack,
  recursive-node function-demand, extern member-template, and direct explicit
  class controls have the expected lifecycle facts;
- an intermediate additional-definition-demand warning for the direct
  `holder<int>::holder` explicit-class constructor was found by the expanded
  gate, classified with typed constructor and enclosing-closure ownership, and
  eliminated before promotion;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and provenance witness sessions complete;
- all 3,060 ordinary/provenance witness and LowIR pairs match byte for byte;
- all 1,530 provenance sessions flush, producing 69,899 records and 6,300
  lifecycle attempts with no unknown producer attempt and no unexercised
  producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and structure-size reports match the parent byte for
  byte.

The ordinary convergence report is
`/tmp/cppgm-function-definition-demand-convergence-final-20260811.json`,
SHA-256
`d513d18063f7035dbe956387215498b9107d2917420e0c70b9d5e7cac7d79926`.
The provenance trace directory is
`/tmp/cppgm-function-definition-demand-provenance-20260811.Mr4cBG`. The
provenance analysis and correlated convergence reports are
`/tmp/cppgm-function-definition-demand-provenance-analysis-20260811.json` and
`/tmp/cppgm-function-definition-demand-provenance-convergence-20260811.json`,
with SHA-256 values
`174c1eb268a2e72edb4b28807a9a23ba200b0db26761d222eb8fab1034694033`
and
`3509ea63a88a7e6c9728f9c22a4d6b2ec6f7c15f34deed7a6f6c4c0aaf89e540`.
The 6,120-file ordinary/provenance output manifest is
`/tmp/cppgm-function-definition-demand-output-manifest-20260811.txt`, SHA-256
`436f450e4b635496529f201dad22fb48072ee3d6772e343d7777fd2463006a81`.
The byte-difference manifest is empty.

The broad and 60-test helper reports are
`/tmp/cppgm-function-definition-demand-broad-20260811.log` and
`/tmp/cppgm-function-definition-demand-helper-tests-20260811.log`, with
SHA-256 values
`108a895e913d52df95ca7aec5c706ad7da74bcff263753472867f7f7bc6fdef2`
and
`b8d82e4e4ae09b03d209d6d8c985bec55477704286a91a32f1a172a0465d3620`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary remains 17,183,296 bytes. Its Mach-O `__TEXT`,
`__DATA_CONST`, and `__DATA` segments remain unchanged at 13,062,144, 61,440,
and 442,368 bytes. It contains no witness-provenance symbols. The frozen
binary is `/tmp/cppgm-function-definition-demand-ordinary-20260811`, SHA-256
`54683204f8b2d845ab1d070a7e128d80c6625c24915953248af10fd2d2e8bff8`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.10% | -0.02% | -3.97% | `/tmp/cppgm-function-definition-demand-perf-fixed-20260811.txt` |
| Member-class reference parent | +0.00% | +0.06% | +0.07% | `/tmp/cppgm-function-definition-demand-perf-parent-20260811.txt` |

The candidate medians are 174,085,575,017 instructions, 756,977,664 bytes
maximum RSS, and 569,466,880 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-function-definition-demand-raw-candidate-20260811.json`, SHA-256
`8d40350d62d2b658ccf9cde46f90b5dcb910191acc59c3c7dae46cf9e460c550`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`f21823951f4143c2d8c9d8921fd9b86f5dd18d9f63f8eb388f7ce6e806842a5c`
and
`b6e6ed8732e7ee3235bb9db1bd3d8848dbcfc887182267a6e2655f1da2bc24b4`.
The candidate metadata names commit `83169fe19` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open with one missing and two unexpected lifecycle facts
across three tests. The class-use divergence and function-call inventory
remain. Inception remains forbidden.

## Inheritance and standard-builtin lifecycle checkpoint, 2026-08-11

This promotable Phase 4 slice removes the final two unexpected terminal
outcomes without manufacturing or hiding a semantic demand.

Speculative derived-to-base conversion screening now collects only the typed
reference-member inheritance graph. It rejects unrelated class types without
completing either class, and completes both sides only after finding a real
base path for a materialized conversion. Successful reference, pointer, and
object conversions then recompute the path and adjustment from the completed
classes. This removes the false
`basic_tree<int, int, int>::iterator` class-instantiation fact while preserving
the exact LowIR for non-primary-base adjustments and virtual dispatch.

The function binding model also classifies Clang's standard-library builtin
domain from structured namespace ownership and the unary non-variadic
function signature. The lifecycle event carries that typed fact to public
closure policy. A standard builtin keeps its direct definition demand and all
raw CPPGM materialization provenance, but it does not publish the terminal
function-instantiation outcome that Clang skips when its builtin body is not
required. The renderer does not inspect a rendered entity name, source
location, fixture, or source text. Renaming the same `forward` declaration out
of `std` retains the function-instantiation fact, and unrelated `std`
templates remain unchanged.

The checkpoint removes the unexpected `iterator` class-instantiation and
`std::forward` function-instantiation outcomes. Expanded convergence remains
at 1,473 exact outputs because the `std::forward` fixture still has an
independent function-call rejection-order difference; the iterator fixture
is now fully exact. The 1,530-reference inventory has 57 known mismatches, no
warning output, and no missing actual file. Class-use remains one changed row
in one test. Function-call remains 36 changed, 18 missing, 13 unexpected, and
one ordering-only occurrence across 55 tests. Lifecycle now has zero
unexpected terminal outcomes and one missing class-instantiation fact in one
test.

The remaining lifecycle fact is
`boost::container::w<boost::container::base_node<recursive_slist> *>::x` in
`pa24/tests/general/400-concrete-recursive-node-layout-retry.t`. Patched Clang
eagerly evaluates the target of `de::c` while instantiating the class even
though no later operation uses that typedef. CPPGM deliberately indexes that
alias declaration without evaluating its unused target; commits `a15320571`
and `3ecce93b9` established and guarded that lazy boundary. Reintroducing eager
alias evaluation would violate the existing alias contract and its negative
controls. Phase 4 therefore remains open on this one cross-phase oracle
divergence until a real typed demand for the alias target exists or the final
oracle policy is reconciled.

The final Homebrew-Clang validation records:

- the iterator fixture and successful inheritance reference, non-primary-base
  pointer, and inherited virtual-dispatch controls retain exact witness and
  LowIR output;
- the `std::forward` fixture has the exact lifecycle set and byte-exact LowIR,
  while the structurally equivalent non-`std` forwarding control remains
  fully byte-exact with its function-instantiation outcome;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862;
- all 1,530 ordinary and provenance witness sessions complete, and all 3,060
  ordinary/provenance witness and LowIR pairs match byte for byte;
- all 1,530 provenance sessions flush, producing 69,894 records and 6,298
  lifecycle attempts with no unknown producer attempt and no unexercised
  producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and structure-size reports match the parent byte for
  byte.

The ordinary convergence report is
`/tmp/cppgm-final-lifecycle-convergence-20260811.json`, SHA-256
`c4c684f95d2b08867092e3d241d765137f4847a2773ae09ddf8c006d87a5578b`.
The provenance trace directory is
`/tmp/cppgm-final-lifecycle-provenance-20260811.W9ni9o`. The provenance
analysis and correlated convergence reports are
`/tmp/cppgm-final-lifecycle-provenance-analysis-20260811.json` and
`/tmp/cppgm-final-lifecycle-provenance-convergence-20260811.json`, with
SHA-256 values
`a05d149ffc081079969087b9bc6626707089804f01f2483e6c32129460a9b5f1`
and
`cd9a94de2d198f0d8cab0110f968ccd6ddfbae6b9de5d5f23eced61509efbe68`.
The byte-identical ordinary and provenance output manifests both have
SHA-256
`023315506c7cab1cc3409f5b1c4971a24bcf00833e33ae7cffa50087d5c23386`;
their empty difference has SHA-256
`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The broad and 60-test helper reports are
`/tmp/cppgm-final-lifecycle-broad-20260811.log` and
`/tmp/cppgm-final-lifecycle-helper-tests-20260811.log`, with SHA-256 values
`ff9ff2fdf2e2b6b1778d5245b7a8d32dd1c9ab948363eb326c03d95eee2c6418`
and
`5f4639385ecd9520fc1d30714bb9a5990ee98be3ca282edcf8c1c622ce1b9c37`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary is 17,187,960 bytes, 4,664 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by 4,096 bytes to 13,066,240 bytes;
`__DATA_CONST` and `__DATA` remain unchanged at 61,440 and 442,368 bytes. It
contains no witness-provenance symbols. The frozen binary is
`/tmp/cppgm-final-lifecycle-ordinary-20260811`, SHA-256
`22f84177df2d624f37e4de2c5be71dbc100b85674b0c40b7bc9a3b0ed06b67cd`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.89% | -0.21% | -3.92% | `/tmp/cppgm-final-lifecycle-perf-fixed-20260811.txt` |
| Function-demand parent | +0.21% | -0.20% | +0.06% | `/tmp/cppgm-final-lifecycle-perf-parent-20260811.txt` |

The candidate medians are 174,455,791,159 instructions, 755,466,240 bytes
maximum RSS, and 569,782,272 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-final-lifecycle-raw-candidate-20260811.json`, SHA-256
`01ecf6e82392157527734dee18a95ded9090e4c5f7b087b5ee0fa55556925257`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`138c1c9bf0a700c3b1af788f52ad974a19cefe8956c9879cf9ec63c762700658`
and
`7c097d4ccdbfae7abb2d057568db1b2c8311441c2d637e39d9d84787e87810cb`.
The candidate metadata names commit `e6d6811bf` because the measurements cover
this uncommitted checkpoint.

Phase 4 remains open on the one lazy-alias lifecycle divergence. The class-use
divergence and function-call inventory remain. Inception remains forbidden.

## Function-call template-binding checkpoint, 2026-08-11

This first promotable Phase 5 slice replaces consumer reconstruction of
function-template binding provenance with the concrete typed arguments already
committed by deduction and substitution. Non-pack and pack arguments now use
`TemplateArgument::source_defaulted`; explicit provenance continues to follow
the source parameter position required for a leading explicit pack, and all
other committed arguments are deduced. The obsolete scan that inferred
defaulting from whether a parameter pattern appeared in the call signature is
deleted.

Function binding text now also projects three typed semantic cases directly.
A concrete null pointer argument prints `nullptr` from its pointer type and
zero value. A concrete function pointer prints the qualified semantic function
binding, using its primary template entity when appropriate. Deduced type
arguments use the resolved semantic type, while explicit type arguments retain
their source spelling. These rules contain no rendered-name, fixture, source
location, or source-text exception.

The slice makes eight complete outputs exact:

- defaulted non-type deduction overriding a declaration default;
- braced member-template deduction with a defaulted trailing parameter;
- explicit member-template leading-pack provenance;
- constructor-template and dependent-pointer null arguments;
- direct and specialized function-pointer non-type arguments;
- the cached async initiation result whose deduced argument must be
  `executor_binder<handler, executor>`, not its source alias expression.

The specialized function-pointer fixture still differs byte-for-byte only
because patched Clang emits a duplicate `ensure-definition sample` event that
the normalized closure comparison treats as equivalent to CPPGM's committed
definition demand. Its source-use row is now exact.

Expanded convergence improves from 1,473 to 1,481 exact outputs. The current
1,530-reference inventory has 49 known mismatches, no warning output, and no
missing actual file. Function-call debt falls from 36 to 27 changed rows and
from 55 to 47 affected tests; its 18 missing, 13 unexpected, and one
ordering-only occurrences are unchanged. Class-use remains one changed row in
one test, and lifecycle remains the one lazy-alias class-instantiation gap
documented by the preceding checkpoint. The ordinary and provenance strict
runs therefore intentionally report the same 49 residual tests; correctness
for this intermediate checkpoint means that the explained set only shrinks,
not that Phase 5 acceptance has been reached.

The final Homebrew-Clang validation records:

- all eight corrected fixtures and the explicit/deduced pack controls retain
  exact source-use output and byte-exact LowIR;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862, including the PA30
  runtime surface;
- all 1,530 ordinary and provenance witness sessions complete, and all 3,060
  ordinary/provenance witness and LowIR pairs match byte for byte;
- all 1,530 provenance sessions flush, producing 69,889 records, 5,014 source
  attempts, and 6,298 lifecycle attempts with no unknown producer attempt and
  no unexercised producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and structure-size reports match the parent byte for
  byte.

The remaining function publication debt is deliberately not hidden by this
slice. The semantic producer still makes 1,515 attempts for 1,037 inserted
rows, including 478 exact duplicates. The renderer still removes 239
source-defined calls, nine location duplicates, three header-pattern rows, and
one visible duplicate. Those counters remain Phase 5 ownership work; removing
the blanket source-defined-call pass before semantic admission distinguishes
evaluated source calls from unevaluated or replayed calls regresses the corpus.

The ordinary convergence report is
`/tmp/cppgm-phase5-bindings-convergence-20260811.json`, SHA-256
`c2b7d2af51b10ee679757e56867d12b632c595eed94a5e3d817acb065066e48d`.
The provenance trace directory is
`/tmp/cppgm-phase5-bindings-provenance-20260811.MH1cUg`. The provenance
analysis and correlated convergence reports are
`/tmp/cppgm-phase5-bindings-provenance-analysis-20260811.json` and
`/tmp/cppgm-phase5-bindings-provenance-convergence-20260811.json`, with
SHA-256 values
`52bae5a9b8e1e2f3e1021d7b14b64a8cd476a41c6a7f35f723d2a4ede7d0c092`
and
`03c8429c825f85a969b5bf7de05422d660ec546a8b5d264066f842576a78c85a`.
The byte-identical ordinary and provenance output manifests have SHA-256
`156033a2743f7a7d9deaaa1e25e11ebf065143206c54ec1c2c6bb4079fef29b2`;
their empty difference has SHA-256
`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The strict, broad, and 60-test helper reports are
`/tmp/cppgm-phase5-bindings-strict-20260811.log`,
`/tmp/cppgm-phase5-bindings-broad-20260811.log`, and
`/tmp/cppgm-phase5-bindings-helper-tests-20260811.log`, with SHA-256 values
`b44bed82db425bce3dfe62782d6bb11e848fd580a52ccbd80424874a46bc9fb3`,
`2c8a4ec00b35d1ee56b723b5f5e0e32c4f13ea955cbc00134cfe15d3d283589e`,
and
`570f2a92161246eb29097f917100f33889c4746086ca9e121b64ec7238791b86`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary is 17,188,440 bytes, 480 bytes larger than the parent.
Its Mach-O `__TEXT`, `__DATA_CONST`, and `__DATA` segments remain unchanged at
13,066,240, 61,440, and 442,368 bytes. It contains no witness-provenance
symbols. The frozen binary is
`/tmp/cppgm-phase5-bindings-ordinary-20260811`, SHA-256
`eb01fe03ec785b32802b9d5315133aedee9ca83c0a2063169ca1f97cf831a0b9`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.14% | +1.08% | -4.01% | `/tmp/cppgm-phase5-bindings-perf-fixed-20260811.txt` |
| Inheritance/lifecycle parent | -0.25% | +1.29% | -0.09% | `/tmp/cppgm-phase5-bindings-perf-parent-20260811.txt` |

The candidate medians are 174,019,949,538 instructions, 765,247,488 bytes
maximum RSS, and 569,257,984 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-phase5-bindings-raw-candidate-20260811.json`, SHA-256
`46d8397ea6747207c48956f706d0d570e959c2fddba92584ef50fa36d6c6ae42`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`7d467b7a0758c50581df7731e79688e283738f503fb414b63fd4f6fd40fe7d4c`
and
`e685566a336910c192ce33ef2a5397d3ac967ddd8b963c8ad1870fd835169607`.
The candidate metadata names commit `517229262` because the measurements cover
this uncommitted checkpoint.

Phase 5 remains open on the 47 function-call tests and the one class-use test.
Phase 4 remains open on the lazy-alias lifecycle oracle divergence. Inception
remains forbidden.

## Typed constructor-rejection checkpoint, 2026-08-11

This Phase 5 slice replaces three generic constructor-candidate fallbacks with
typed semantic outcomes. The deduction-failure classifier walks dependent
class-template and alias-template argument carriers to find template-parameter
uses. If both sides identify class templates, the classifier compares their
structured declarations with inline-namespace equivalence instead of relying
on display-name fallback.

The constructor fast filter returns `Match`, `ArgumentCountMismatch`,
`NonForwardingRvalueMismatch`, or `TemplateEntityMismatch`. Its caller maps
those results to the public arity, conversion, and non-deduced failure reasons.
This removes the false `substitution_failure` results for the defaulted member
call and distinct friend class-template entities, and it changes the rejected
`weak_ptr` constructor template from a false arity failure to
`non_deduced_mismatch`.

User-defined conversion construction carries its semantic source order and
drop-order ownership through the typed function-call request, decision,
source-use table, and renderer event. A conversion from a prvalue, or a
constructor template selected entirely from defaulted template arguments,
publishes its ordered deduction and candidate phases. In that
same semantic domain, an ordinary zero-explicit-argument constructor is not
published as an arity failure beside the selected converting constructor
template. Direct construction and lvalue-deduced conversion keep the existing
candidate-priority policy. The rule uses value category, template-argument
provenance, constructor kind, and parameter shape; it does not inspect a
rendered name, fixture path, source line, or source text.

These changes make four complete outputs exact:

- `pa20/tests/general/400-defaulted-template-member-call-rematerialization.t`;
- `pa22/tests/general/300-friend-function-template-distinct-class-template-entities.t`;
- `pa22/tests/spec/300-nested-member-template-definition-parameter-alias-default.t`;
- `pa23/tests/general/500-weak-ptr-shared-ptr-template-ctor.t`.

The direct-construction, default-argument, and lvalue-deduced constructor
controls retain exact witness and byte-exact LowIR. The expanded comparison
reports 1,485 exact outputs, up from 1,481. The 1,530-reference inventory has
45 known mismatches, no warning output, and no missing actual file.
Function-call debt is now 23 changed, 18 missing, 13 unexpected, and one
ordering-only occurrence across 43 tests. Class-use remains one changed row in
one test, and lifecycle remains the one lazy-alias class-instantiation gap.
The ordinary and provenance strict runs both report PA19 0, PA20 0, PA22 8,
PA23 27, and PA24 10 residuals.

The final Homebrew-Clang validation produced these results:

- the PA1-PA38 direct-LowIR report passes 4,862/4,862, including PA30 runtime;
- all 1,530 ordinary and provenance witness sessions complete, and all 3,060
  witness and LowIR files match byte for byte;
- all 1,530 provenance sessions flush, producing 69,884 records, 5,014 source
  attempts, and 6,298 lifecycle attempts with no unknown producer and no
  unexercised producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and tracked structure-size reports match the parent byte
  for byte.

Function publication ownership remains open. The semantic producer makes
1,515 attempts for 1,037 inserted rows, including 478 exact duplicates. The
renderer removes 239 source-defined calls, nine
location duplicates, three header-pattern rows, and one visible duplicate;
its legacy drop-order pass rewrites 43 events. The new typed order bit
narrows that arbitration for the two semantic conversion domains proved here,
but it is migration scaffolding to delete after the producer owns canonical
ordering for every call. This checkpoint adds 253 and removes 21 production
lines; Phase 5 owns that net growth and the final simplification gate.

The ordinary convergence report is
`/tmp/cppgm-phase5-drop-convergence-20260811.json`, SHA-256
`a75af0cf11f6c30f149a72aba7e83588f4f692a0007a759edb56cccf110c97cb`.
The provenance trace directory is
`/tmp/cppgm-phase5-drop-provenance-trace-20260811`. The provenance analysis
and correlated convergence reports are
`/tmp/cppgm-phase5-drop-provenance-analysis-20260811.json` and
`/tmp/cppgm-phase5-drop-provenance-convergence-20260811.json`, with SHA-256
values
`079c93e333871ad74e78db94e6fe82151c53f07ec02f5cae81b325b9787b6a1e`
and
`b6354c0cab12d0eb08c295fd684e5393bad8b9833bd8704fca87e6168e491baf`.
The byte-identical ordinary and provenance output manifests both have SHA-256
`72ce0dd2a807fbe3fdcba1e61e54541b40672d51f1636a7cb6accae24c2367e1`;
their empty difference has SHA-256
`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The strict, broad, and 60-test helper reports are
`/tmp/cppgm-phase5-drop-strict-20260811.log`,
`/tmp/cppgm-phase5-drop-broad-20260811.log`, and
`/tmp/cppgm-phase5-drop-helper-tests-20260811.log`, with SHA-256 values
`d4f050cc8a663108d4bb86565246035f31cd52489da81943ff11620efd8359bf`,
`ef5cd67f7ea9c1e84b245b4b9b621ae212b882c47f633a972873b1ac80e205b0`,
and
`c4519d5ea01ae0b752cae3cf083beb96ac856296f05d903a00b6ccff48c54d99`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary is 17,193,384 bytes, 4,944 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by 4,096 bytes to 13,070,336 bytes;
`__DATA_CONST` and `__DATA` remain unchanged at 61,440 and 442,368 bytes. It
contains no witness-provenance symbols. The frozen binary is
`/tmp/cppgm-phase5-drop-ordinary-20260811`, SHA-256
`c61e6af17fb95b0fc7a1945d5eaf08743ed27c08435bd83e480c66e86cadc2cf`.

The three-run performance record passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -0.97% | +0.33% | -3.95% | `/tmp/cppgm-phase5-drop-perf-fixed-20260811.txt` |
| Function-binding parent | +0.17% | -0.74% | +0.05% | `/tmp/cppgm-phase5-drop-perf-parent-20260811.txt` |

The candidate medians are 174,319,837,098 instructions, 759,595,008 bytes
maximum RSS, and 569,569,280 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-phase5-drop-raw-candidate-20260811.json`, SHA-256
`1d490f4b497d9727d5a05e03de693a9e6d4841d55cdd77e5de8ad29c3b84f07c`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`5b8eda17719cbdd1686f3702d190cbe87168675086100fd0a10b0ffbcdc2dd52`
and
`a8c037d66668bf5ae56abc2a1e23d58271d68de8b56bfe43ba630b5843fe1c5a`.
The candidate metadata names commit `d87aa8723` because the measurements cover
this uncommitted checkpoint.

Phase 5 remains open on the 43 function-call tests and the one class-use test.
Phase 4 remains open on the lazy-alias lifecycle oracle divergence. Inception
remains forbidden.

## Unified user-defined conversion result checkpoint, 2026-08-11

This Phase 5 checkpoint gives an enclosing function call typed ownership of
the user-defined conversion selected for each argument. Candidate matches and
the overload cache retain an optional `ArgumentConversionSelection` only while
witness capture is active. Constructor probes report a typed
`ConstructorSourceCallResult`, including the selected function, failed
candidate drops, explicit and total argument counts, and the narrow semantic
ordering domains required by constructor conversion. The enclosing call is
then the single final publisher. Direct conversions continue to publish in
their semantic traversal order.

The constructor lifecycle profile now carries typed
`user_defined_conversion_source` and `non_explicit_construction` intent. The
overload path no longer discovers the user-defined conversion case through a
context string. Function-template witness identity, declaration location, and
conversion-operator parameter identity are likewise retained as semantic
facts. Conversion result locations come from the earliest retained syntax
location in the converted argument, which also corrects the prior string
literal end-location result. These rules contain no fixture name, path,
rendered-name, source-line, or source-text filter.

The checkpoint makes these 11 complete outputs exact:

- `pa23/tests/general/300-constructor-template-keeps-ctor-refinement-viable.t`;
- `pa23/tests/general/300-copy-init-ignores-explicit-converting-ctor-template.t`;
- `pa23/tests/general/400-defaulted-sfinae-conversion-function-template-symbol.t`;
- `pa23/tests/spec/300-constructor-template-initial-sequence-beats-const-conversion-template.t`;
- `pa23/tests/spec/300-conversion-function-template-object-result-copy-init.t`;
- `pa23/tests/spec/300-conversion-function-template-owner-result-copy-init.t`;
- `pa23/tests/spec/500-conversion-function-template-same-name-target.t`;
- `pa23/tests/spec/500-template-template-conversion-operator-reference-target.t`;
- `pa24/tests/general/200-constructor-template-parameter-shadows-instantiated-type.t`;
- `pa24/tests/general/500-constructor-template-default-constraint-previous-param.t`;
- `pa24/tests/spec/400-concrete-alias-target-nested-conversion-template.t`.

The focused run covers those outputs and 12 direct-conversion, construction,
assignment, and cached-overload controls; all 23 retain byte-exact witness and
LowIR output. Expanded convergence improves from 1,485 to 1,496 exact outputs.
The 1,530-reference inventory has 34 known mismatches, no warning output, and
no missing actual file. Exactly the 11 listed tests leave the mismatch set;
there is no new or reclassified residual. Function-call debt is now 11
changed, 17 missing, 13 unexpected, and one ordering-only occurrence across
32 tests. Class-use remains one changed row in one test, and lifecycle remains
the one lazy-alias class-instantiation gap.

The final Homebrew-Clang validation produced these results:

- the ordinary strict run reports PA19 279 compared with zero failures, PA20
  158 with zero, PA22 293 with eight residuals, PA23 385 with 19, and PA24 415
  with seven; its expected nonzero exit is therefore exactly the documented
  34-test residual set;
- the PA1-PA38 direct-LowIR report passes 4,862/4,862, including PA30 runtime;
- all 1,530 ordinary and provenance witness sessions complete, and all 3,060
  witness and LowIR files match byte for byte;
- all 1,530 provenance sessions flush, producing 69,876 records, 5,015 source
  attempts, and 6,298 lifecycle attempts with no unknown producer attempt and
  no unexercised producer site;
- the convergence, provenance, materialization, text-reparse, path,
  performance, template-boundary, and class-audit helper suites pass 60/60;
- both materialization decision boundaries have no finding, all 23 forbidden
  text-reparse categories remain zero, and the template-boundary,
  semantic-boundary, and tracked structure-size reports match the parent byte
  for byte.

Function publication ownership is smaller but remains open. The semantic
producer makes 1,516 attempts for 1,038 inserted rows, including 478 exact
duplicates, and leaves 786 final visible rows. The renderer removes 239
source-defined calls, nine location duplicates, three header-pattern rows,
and one visible duplicate. Its legacy drop-order pass rewrites 35 events,
down from 43 at the parent. The structured drop order is now producer-owned
for the direct-constructor phase containing both arity and conversion failure,
and for non-explicit construction containing both explicit rejection and
conversion failure. Broader renderer arbitration remains Phase 5 debt.

The ordinary convergence report is
`/tmp/cppgm-phase5-conversion-perf-gated-convergence-20260811.json`, SHA-256
`1cd0bd58142eda37b83c49c0214a30b3bab2226f27a9b040ab29c36637015736`.
The provenance trace directory is
`/tmp/cppgm-phase5-conversion-perf-gated-provenance-trace-20260811`. The
provenance analysis and correlated convergence reports are
`/tmp/cppgm-phase5-conversion-perf-gated-provenance-analysis-20260811.json`
and
`/tmp/cppgm-phase5-conversion-perf-gated-provenance-convergence-20260811.json`,
with SHA-256 values
`c7c76d5bb167df83b5772e73bd832ab23e92caf6070b43712aab48f59a619f11`
and
`6e81f5c58ff4ee5277c76d7d465c737dab631965ed8fdecabd5515a49d0d296f`.
The byte-identical ordinary and provenance output manifests both have SHA-256
`c80049e58ee071ecb904ec3544b60b008cf54780ba1256238e72945881ed2192`;
their empty difference has SHA-256
`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.

The strict, broad, and 60-test helper reports are
`/tmp/cppgm-phase5-conversion-perf-gated-strict-20260811.log`,
`/tmp/cppgm-phase5-conversion-perf-gated-broad-20260811.log`, and
`/tmp/cppgm-phase5-conversion-perf-gated-helper-tests-20260811.log`, with
SHA-256 values
`f021b83aaf2dcd4af3f286008c2f5fd4a211f39578c448e92afbfb3a7ae5bee0`,
`2c55d9a8f8656a12ffb2682082debe472fd9300b392c48fc1dbf2e0fbbe99850`,
and
`83c9dea806f41e69c0dfbae199d5bc1b2ab36915fe469a4b83c1ea8fa567dc0a`.
The materialization, zero-finding text-reparse, template-boundary,
semantic-boundary, and structure-size reports retain their parent SHA-256
values
`27acfb819a6872ffb36e59e33cccdec28a83ea0543b69f5b4c0a8bb3ee33e526`,
`1de948196cc856fc673897264f3b7210dab0ab768743743555644db743b7c515`,
`46ac0175a42595f5a98767eb76039a534543acd9059db17ce714150fcb7118ad`,
`a8654f85de246d956481db71e121f1e8ff01fbf2e003bc2b9d968a847121dff2`,
and
`5fc6f13207db17161c012cf7e08327ab3c2ef0f6c04ad1b2e7c4355dbc40ec01`.

The ordinary binary is 17,233,064 bytes, 39,680 bytes larger than the parent.
Its Mach-O `__TEXT` segment grows by 28,672 bytes to 13,099,008 bytes;
`__DATA_CONST` and `__DATA` remain 61,440 and 442,368 bytes, and `__LINKEDIT`
grows by 12,288 bytes to 4,067,328 bytes. It contains no witness-provenance
symbols. The frozen binary is
`/tmp/cppgm-phase5-conversion-perf-gated-ordinary-20260811`, SHA-256
`456c68a4ee499c7194771e28e2990ed01b01a352fc22a324903b6598b6538356`.

The first complete implementation was correctly rejected by the performance
gate. Its three-run medians were 211,253,294,271 instructions, 751,890,432
bytes maximum RSS, and 568,934,400 bytes peak footprint. It regressed
instructions by 20.02% against the fixed baseline and 21.19% against the
parent because ordinary witness-off compilation still allocated and populated
witness-only selections, drops, and source-location traversal. The raw record
and two reports are
`/tmp/cppgm-phase5-conversion-fully-final-raw-candidate-20260811.json`,
`/tmp/cppgm-phase5-conversion-fully-final-perf-fixed-20260811.txt`, and
`/tmp/cppgm-phase5-conversion-fully-final-perf-parent-20260811.txt`, with
SHA-256 values
`1a6e5450792aff4485a9a63b54617d5d6b93a122c7a89bfd5fcaabf842be83b5`,
`cdbe222810a1aa1d03646090d490b2f5bc8981c5654f055dc989ddede0e6f2f3`,
and
`e52c6b581b09490f13381fa0478ef0f5c22bcf3b90e6cf8e635275eab1cf74d7`.

The implementation was changed in scope: selection storage is optional,
cache ownership is optional, and all new source-fact collection and traversal
is gated on active witness capture. The corrected candidate was then measured
once with a fresh three-run record and passes both comparisons:

| Comparison | Instructions | Maximum RSS | Peak footprint | Report |
| --- | ---: | ---: | ---: | --- |
| Fixed alias-convergence baseline | -1.25% | +0.10% | -4.03% | `/tmp/cppgm-phase5-conversion-perf-gated-fixed-20260811.txt` |
| Typed constructor-rejection parent | -0.28% | -0.23% | -0.08% | `/tmp/cppgm-phase5-conversion-perf-gated-parent-20260811.txt` |

The corrected medians are 173,824,076,764 instructions, 757,870,592 bytes
maximum RSS, and 569,106,432 bytes peak footprint. Wall time remains an
informational measurement. The raw candidate record is
`/tmp/cppgm-phase5-conversion-perf-gated-raw-candidate-20260811.json`,
SHA-256
`d580307d76449e1f904c93c6525d304908194411cc326d4fc67206f7a39a1061`.
The fixed-baseline and parent-comparison reports have SHA-256 values
`0827dc51d12b5ad7c305576b9cdc6da49879c5723f5c025f23a06900f62b917a`
and
`6b1c348cf97f832b36e69c29129b674bc06c096334b1d2859549f20c2f425925`.
The candidate metadata names commit `d1b17272d` because the measurements cover
this uncommitted checkpoint.

This checkpoint adds 754 and removes 84 production lines before this ledger
entry. Phase 5 owns that net growth and the final simplification gate. It
remains open on the 32 function-call tests and the one class-use test. Phase 4
remains open on the lazy-alias lifecycle oracle divergence. Inception remains
forbidden.

## Current decision, 2026-08-09

Commit `b03f2530dad6513aabfa1064a8919bb61fea7d3f` is the restart point. It adds
diagnostic class-owner evidence to the Phase 1 checkpoint without changing
ordinary output. Two uncommitted files contain a rejected typedef-deferral
experiment:

- `dev/src/semantic_class_model.cpp`;
- `dev/src/semantic_model.h`.

The experiment adds 152 lines and removes eight. Its patch SHA-256 is
`f40f0f9420534a5740d9e288b30acad4481252450c5dba51d2065cc8d5c92879`.
Preserve that identity and the evidence in the recovery ledger, then remove
the experiment before the next semantic slice. Do not layer fixes onto it.

| State | Strict witness | Other strict failures | PA1-PA38 | Performance |
| --- | ---: | ---: | ---: | --- |
| Restart point | 1,339/1,530, with 191 mismatches | 0 direct-LowIR regressions | 4,862/4,862 | valid Phase 1 gate |
| Rejected experiment | 1,322/1,530, with 208 mismatches | 14 LowIR-only failures; 222 distinct failing strict tests | 4,722/4,862 | benchmark compilation fails |

The rejected experiment fixes none of the 191 restart mismatches and adds 17
witness failures. It also prevents 18 provenance sessions from flushing and
breaks hosted libc++ compilation in PA35 through PA37. No valid performance
report exists for this state because the frozen benchmark exits during its
first run.

### Remaining mismatch inventory at the restart point

The 191 failing outputs overlap event families. They contain 565 row-level
differences:

| Family | Failing outputs | Changed rows | Missing rows | Extra rows | Ordering only |
| --- | ---: | ---: | ---: | ---: | ---: |
| Alias use | 24 | 18 | 16 | 2 | 0 |
| Class use | 62 | 57 | 36 | 10 | 0 |
| Function call | 86 | 40 | 62 | 35 | 1 |
| Variable use | 3 | 1 | 0 | 2 | 0 |
| Lifecycle | 79 | 0 | 158 | 127 | 0 |

The 36 alias row differences form five concrete groups:

| Alias group | Rows | Required semantic owner |
| --- | ---: | --- |
| Missing source-pattern occurrences | 16 | primary source-pattern analysis |
| Extra implicit-instantiation occurrences | 2 | explicit-source traversal boundary |
| Wrong argument or empty-pack provenance | 9 | structured template-argument result |
| Wrong owner presentation | 3 | selected and lexical owner facts |
| Wrong semantic argument rendering | 6 | structured template-argument printer |

### Patched-Clang alias text contract, 2026-08-09

The alias oracle does not copy template-argument text from the source file.
At patched LLVM commit `59c5d9c70`,
`clang/lib/Frontend/FrontendAction.cpp` does the following work for an alias
use:

1. The default `RecursiveASTVisitor` visits a written
   `TemplateSpecializationTypeLoc`. Its default
   `shouldVisitTemplateInstantiations()` result is `false`, so the visitor
   skips implicit instantiation bodies.
2. `VisitTemplateSpecializationTypeLoc` takes the occurrence location from
   `getTemplateNameLoc()` and the selected alias from the semantic
   `TypeAliasTemplateDecl`.
3. It takes arguments from
   `TemplateSpecializationType::template_arguments()`. It renders each one
   with `TemplateArgument::print(PrintingPolicy, ..., true)`. Clang prints
   types through `QualType::print`, dependent expressions through
   `Expr::printPretty`, template names through `TemplateName::print`, and
   packs by printing their structured elements.
4. `buildBindings` zips the alias parameter list with that source-occurrence
   semantic argument list. `TemplateSpecializationTypeLoc::getNumArgs()` is
   the size of the same list. Omitted defaults and omitted empty packs do not
   create alias bindings.
5. The checked-in reference generator reads the JSON and passes it to
   `render_emit_templates_text`. Its public normalization changes a
   small set of generic type spellings. It does not recover alias arguments
   from source lines, token ranges, or `TemplateArgumentLoc` text.

The direct JSON entry point is important here. `clang_witness_json_to_ref.py`
calls `render_emit_templates_text` on the parsed document; it does not call
`parse_emit_templates_text`. The source-scanning occurrence recovery,
inline-namespace discovery, location repair, and binding inference in the
latter function are therefore legacy text-import behavior, not part of the
patched-Clang reference contract.

The direct JSON renderer performs only these public transformations:

- normalize location paths by converting separators, replacing a Homebrew or
  other libc++ include prefix with `libc++/`, and removing the `paN/` prefix
  before `tests/` or `course/`;
- remove checkout paths embedded in local/lambda entity names while retaining
  their line and column, and remove Clang's `__local_N` discriminator;
- normalize a narrow group of type spellings: east-to-west `const`, integral
  literal suffixes, spacing between pointer stars and pointer cv-qualifiers,
  built-in spellings such as `unsigned int`, and five dependent member names
  such as `T::value_type`;
- collapse an exactly duplicated entity-owner prefix, remove a contradictory
  `worse_conversion` drop when the same function declaration already has a
  nonviability reason, and coalesce exactly identical raw source-event
  objects; closure rows are sorted and coalesced by kind and normalized
  entity because the compact projection omits their provenance.

The first two groups are portability presentation. The narrow spelling group
can change the exact reference text, but it cannot add an alias binding,
choose a declaration, expand an injected class name, or change which source
occurrence is visited. The final group requires separate source-use and
closure policies. Exact raw source-event coalescing cannot merge two events
that disagree semantically or only become equal after spelling normalization.
Compact closure coalescing can merge provenance-distinct lifecycle demands,
but that provenance remains available in raw JSON and debug witness output.

The normalization audit found one concrete oracle defect in the supposedly
narrow spelling group. `normalize_const_order` had two no-whitespace regular
expressions that did not require `const` to begin at a token boundary. They
therefore treated identifier suffixes as type qualifiers: for example,
`remove_const` became `constremove_`, `is_const` became `constis_`, and
`forwarding_top_const` became `constforwarding_top_`. This is the source of
the `constremove_` rows previously attributed to CPPGM alias spelling. The
unsafe expressions are removed and covered by a renderer unit test. All
affected references must be regenerated from fresh patched-Clang JSON; CPPGM
must continue to print the semantically correct identifiers.

This finding also changes how the remaining renderer rewrites are treated.
Only path relocation and removal of checkout paths from compiler-generated
local-entity names are presumed harmless. Integral-suffix removal, dependent
member-owner stripping, duplicate-owner collapse, and compact closure
coalescing are lossy presentation operations. Before accepting the
regenerated corpus, record how many raw JSON fields or rows each operation
changes and inspect every operation that changes an alias row. A
normalization that changes a user identifier, merges two distinct source-use
events, or repairs an impossible producer spelling is an oracle bug to remove,
not a parity rule to reproduce in CPPGM.

### Raw duplicate audit, 2026-08-09

Removing both Python coalescing steps and regenerating all 1,530 strict
references exposed 152 additional rows in 64 files. The result is sharply
split:

- 150 rows are compact closure rows: 26 `ensure-definition`, 52
  `function-instantiation`, 53 `require-definition`, and 19
  `variable-instantiation` rows;
- none of those 150 rows is an exact duplicate in patched-Clang JSON; every
  pair differs in declaration/use location, trigger, trigger declaration, or
  reason, but the compact closure format does not print those fields;
- only two additional source-use rows exist, both `class-use` rows for
  `sink<int>` in
  `pa24/tests/spec/100-out-of-class-conversion-operator-definition.t`;
- those two are exact raw JSON duplicates. Clang's default
  `RecursiveASTVisitor` visits a conversion operator's conversion type once
  through `TraverseDeclarationNameInfo` and again through the function
  `TypeSourceInfo`;
- there are no duplicate raw alias-use, function-call, or variable-use rows in
  the corpus.

The invariant is therefore family-specific. Source-use convergence remains
one row per explicit source occurrence, especially for aliases; the two
conversion-type traversal overlaps are harmless exact-event coalescing at the
oracle boundary and are not a shape CPPGM should manufacture. Closure
lifecycle collection may contain repeated `(kind, entity)` rows when their
provenance differs. The compact public projection continues to coalesce those
rows, while debug output retains them. CPPGM must not add semantic machinery
solely to reproduce a multiplicity that the public format cannot distinguish.

Closure correctness is compared as normalized semantic facts, not as a raw
event log and not as one undifferentiated "some closure happened" bit:

- `require-definition` and `ensure-definition` both produce one
  `definition-demand` fact for their normalized entity;
- `function-instantiation`, `class-instantiation`, `alias-instantiation`,
  `variable-instantiation`, and `class-finalization` remain distinct terminal
  outcome facts;
- event multiplicity, source location, trigger, trigger declaration, reason,
  and detail do not participate in the ordinary correctness key; they remain
  available in raw JSON and debug witness output;
- a missing reference fact fails the closure gate;
- an unexpected terminal outcome fails because it can indicate eager or
  incorrect instantiation;
- an additional `definition-demand` fact is reported as a warning for semantic
  and performance investigation, but does not fail correctness by itself.

Thus "at least one" means at least one event for every required normalized
closure fact. It never means that one arbitrary closure row is sufficient for
the translation unit or even for an entity. The matching harness must compare
these fact sets explicitly so correctness does not depend on renderer
multiplicity or ordering.

Parity work must consequently target the patched-Clang structured argument
printer before Python presentation normalization. CPPGM may apply the same
pure path and spelling presentation at its final text boundary, but semantic
resolution must not learn these rewrites. In particular, do not use owner
collapse or broad normalized-row deduplication to compensate for repeated
source-use semantic work.

The semantic printer preserves structured sugar and qualifier form; it does
not canonicalize every argument to an unsugared type. It also does not retain
the writer's whitespace. Fresh raw JSON from the patched compiler shows the
boundary:

| Written argument | Patched-Clang argument |
| --- | --- |
| `sizeof(Next::has_key(...)) == sizeof(support::no_tag)` | `sizeof (Next::has_key(...)) == sizeof(support::no_tag)` |
| `Args&&...` | `Args &&...` |
| `T(Args...)` | `T (Args...)` |
| `list<I + 1, void(Tail...)>` | `list<I + 1, void (Tail...)>` |
| `typename Parameters::binding` | `typename Parameters::binding` |
| unqualified `has_context_from` | unqualified `has_context_from` |

The last two rows matter as much as whitespace. Clang prints the semantic
template-name form held by the argument. It neither expands
`Parameters::binding` with inferred owner arguments nor replaces an
unqualified dependent template name with the selected declaration's qualified
name.

The 1,530-reference corpus contains 835 alias-use events and 1,274 alias
bindings. All 1,274 bindings are `source=explicit`. Six alias events have no
binding. No alias binding is defaulted, deduced, or rendered as an empty
`<>` pack. These counts match Clang's source-occurrence argument model and
explain the empty-pack failures.

CPPGM uses a different payload boundary. Alias resolution carries
resolved `TemplateArgument` values, `source_argument_texts`, and
`TemplateArgumentSyntax`. The observer favors source-derived strings in
several paths, then `template_witness_source_binding_arg_text` returns those
strings for most explicit type arguments. Later helpers remove selected text
fragments or normalize whitespace. The resolved argument vector can also
contain defaulted or expanded pack elements that Clang's alias
`TemplateSpecializationType` does not publish. This combination causes the
six spelling mismatches, the owner changes, and the synthetic empty-pack
bindings.

The paused `ExplicitArgumentsOnly` probe fixes the omission of zero-length
pack bindings, but it still infers Clang's source-occurrence list from a
resolved argument vector and a string count. The source-spacing probe is also
the wrong model. Copied fragment AST nodes use fragment-relative token spans,
and those spans cannot recover original-file text. Neither probe is a
promotable Phase 3 implementation.

Phase 3 will use this parity contract:

- source syntax owns the occurrence identity, source anchor, traversal
  eligibility, and structured qualifier form;
- the selected semantic alias declaration owns the public template entity;
- the normal argument-resolution operation produces a stack-scoped
  source-occurrence argument result, with one converted semantic argument per
  written template argument before default insertion or pack flattening;
- one Clang-compatible structured printer renders those semantic arguments;
- the alias binding builder zips parameters with source-occurrence arguments
  and marks each emitted binding explicit;
- alias publication never copies raw argument text, scans a source line,
  repairs token spacing, or reconstructs a binding from the expanded alias
  target.

Source strings may still enter parsing and lookup before the compiler has a
semantic argument. They stop being witness payload once normal resolution has
produced the structured result.

### Findings that change the execution order

The patched-Clang witness visitor uses the default
`RecursiveASTVisitor` template policy. It visits explicit source patterns and
explicit specializations, and it does not traverse implicit template
instantiations. Class uses come from `TemplateSpecializationTypeLoc`, deduced
type locations, one CTAD fallback, and nested-name-specifier types inside a
materialized variable-template initializer. Alias uses come from the same
explicit source type-location traversal.

CPPGM resolves many class-member typedef targets while collecting a
concrete class specialization. The rejected experiment tried to defer that
work before all consumers shared a demand boundary. It exposed two missing
prerequisites:

1. The early return for a typedef declaration skipped semantic children such
   as the anonymous enum and enumerators in `typedef enum { white, black }
   color_type;`.
2. Many semantic paths fetch entries from `Scope::named_types`. A dependent
   placeholder can escape without calling `lookup_member_type`, so lookup,
   overload resolution, SFINAE, hosted headers, and code generation observe an
   unresolved type.

Class-member declaration indexing, named-type lookup, and alias-target
resolution must converge before target evaluation becomes lazy. That
convergence is the first behavior-preserving implementation stage.

The first lookup-migration slice adds a further prerequisite. Completing
member resolution is not interchangeable with inspecting bindings that already
exist. Generic lexical search, in-progress class collection,
inherited-using discovery, type-equivalence checks, and binding-cache probes
must not trigger class completion or mutate semantic state. These become
explicit non-completing query modes before their raw map reads are removed.
Registration, reset, template-parameter binding, declaration indexing, and
diagnostic iteration remain storage operations rather than lookup consumers.

### Short execution order

1. Preserve the rejected experiment evidence and restore `b03f2530d`.
2. Reconfirm 1,339/1,530 strict and 4,862/4,862 broad from a clean generated
   output surface. Record a new rolling performance baseline after all current
   diagnostics compile in.
3. Route completing class-member named-type consumers through the canonical
   resolving lookup. Give already-bound, in-progress, equivalence, cache, and
   inherited-using probes explicit non-completing query modes. Keep eager
   target resolution during this migration.
4. Split declaration indexing from target resolution while preserving nested
   declarations, access, lookup, and overload obligations. Enable laziness
   only after strict and broad parity.
5. Finish class source-occurrence convergence, then alias convergence, using
   the patched-Clang explicit-source traversal boundary.
6. Converge lifecycle transitions, followed by function-call and variable-use
   results.
7. Delete migration scaffolding and generic arbitration whose counters reach
   zero.
8. Pass strict, broad, provenance, size, and performance gates. Run inception
   last.

## Why the reset is necessary

The prior completion claims were based on 1,305 tracked witness references.
Full regeneration added 225 patched-Clang references, producing a 1,530-test
strict surface. The larger corpus shows that the old gate omitted material
alias, class, function, variable, and lifecycle behavior.

The uncommitted implementation also combines several incomplete migrations.
It fixes some of the newly covered cases, but it regresses substantially more
previously passing behavior, grows production code, and has a large measured
performance cost. Continuing to add local corrections would make it harder to
identify the semantic operation that actually owns each witness row.

The recovery therefore has three distinct jobs:

1. restore the previously passing correctness surface;
2. converge each newly exposed event family through its primary semantic
   operation rather than through filters or replay;
3. delete the migration scaffolding and prove a final code and performance
   reduction.

## Audit snapshot

### Worktree and build state

At the audit point, 53 tracked files differ from `HEAD` by 3,382 additions and
2,165 deletions. `dev/src` alone differs by 3,194 additions and 2,165
deletions, a net addition of 1,029 lines. Using the same source-file count on
both trees:

| Tree | `dev/src` lines |
| --- | ---: |
| Clean `HEAD` | 413,807 |
| Dirty audit tree | 414,836 |
| Delta | +1,029 |

The ordinary compiler is 17,006,872 bytes. Its Mach-O `__text` section is
11,787,369 bytes. A Homebrew-Clang rebuild succeeds but reports three unused
variables in migration/provenance code:

- `callsemantic.cpp`: `has_class_context`;
- `template_argument_semantics.cpp`: `retained`;
- `template_selection.cpp`: `dependency_count_before`.

These warnings are cleanup obligations, not harmless final state.

The class-materialization Phase 0 baseline required by its original plan,
`/tmp/cppgm-class-materialization-ownership-fixed.json`, was never recorded.
The implementation therefore advanced without the required clean,
post-diagnostic performance epoch.

### Strict correctness

Both trees were rebuilt with Homebrew Clang and run against the same 1,530
patched-Clang references with direct LowIR comparison.

| Surface | Clean `HEAD` | Dirty audit tree |
| --- | ---: | ---: |
| Comparisons | 1,530 | 1,530 |
| Passing | 1,339 | 1,218 |
| Failing | 191 | 312 |

Set comparison gives the important transition:

| Relationship | Tests |
| --- | ---: |
| Fail on both trees | 173 |
| Fixed by the dirty tree | 18 |
| Regressed only on the dirty tree | 139 |

Of the dirty tree's 312 failures, 133 use previously tracked references and
179 use the newly generated references. Clean `HEAD` passes all 1,305 tracked
references, so the 139 current-only failures consist of 133 tracked
regressions and six newly covered regressions. The dirty tree fixes 18 of the
191 clean-tree coverage gaps but is not a viable correctness base.

Per-PA dirty-tree results are:

| PA | Compared | Failing |
| --- | ---: | ---: |
| PA19 | 279 | 37 |
| PA20 | 158 | 22 |
| PA22 | 293 | 78 |
| PA23 | 385 | 103 |
| PA24 | 415 | 72 |

Failure-family counts below are test counts and overlap when one test has more
than one kind of mismatch:

| Family | Clean `HEAD` gaps | Dirty-tree gaps |
| --- | ---: | ---: |
| Alias use | 24 | 13 |
| Class use | 62 | 195 |
| Function call | 86 | 84 |
| Variable use | 3 | 2 |
| Lifecycle | 79 | 86 |

The dirty tree's row-level differences are:

| Family | Changed rows | Extra rows | Missing rows |
| --- | ---: | ---: | ---: |
| Alias use | 11 | 2 | 1 |
| Class use | 60 | 216 | 34 |
| Function call | 44 | 27 | 54 |
| Variable use | 1 | 1 | 0 |
| Lifecycle | n/a | 130 | 212 |

There are no reorder-only failures. The dominant class symptom is actual
over-publication, not formatting or ordering.

### Broad correctness

The current Homebrew-Clang PA1-PA38 report passes 4,857 of 4,862 tests. Its
five failures are:

1. `pa12/tests/general/200-switch-case-declaration.t`: the compiler now
   correctly rejects a label that bypasses initialization, but this older
   fixture still expects success. Reconcile the fixture/reference with the new
   PA15 and PA19 negative coverage; do not weaken the compiler.
2. `pa23/tests/spec/300-conversion-function-template-owner-result-copy-init.t`:
   generated LowIR loses the conversion-function body and required nested
   definitions.
3. `pa23/tests/spec/300-dependent-member-template-call-enable-if.t`: the
   generated symbol changes a value-template argument from the checked-in
   `Lv0E` form to `Li0E`. Subsequent oracle validation found the checked-in
   LowIR reference stale: Homebrew Clang 22, patched Clang 23, and Clang 22
   targeting both x86-64 and AArch64 Linux all emit `Li0E`. The fixture must be
   corrected rather than restoring the old compiler's untyped-value encoding.
4. `pa30/tests/general/300-runtime-function-local-static-storage.t`: compiler
   execution fails before the expected runtime check.
5. `pa30/tests/general/300-runtime-local-class-enclosing-enumerator.t`:
   compiler execution fails before the expected runtime check.

The two PA23 LowIR failures and two PA30 failures are semantic regressions in
the dirty tree. They are not witness-renderer issues and must be clean before
any witness phase is accepted.

### Static and provenance audits

The current tree does have useful progress worth retaining as design evidence:

- `scripts/audit_witness_materialization.py` reports no findings, two decision
  boundaries, and six forbidden-symbol checks;
- `scripts/audit_text_reparse.py` reports zero hits in all 23 tracked
  categories;
- the provenance analyzer's six unit tests pass;
- the forbidden class materialization helper names are absent.

These checks are necessary but insufficient. The temporary member-alias bridge
performs a second structured AST walk through `parse_template_parameters`.
That is not source-text reparsing, so the text-reparse audit correctly does not
flag it, but it is still duplicate semantic work and may not survive the
recovery.

Temporary diagnostic/scaffold that must not reach the final tree includes:

- alias `lexical-owner` and `active-owner` trace fields;
- `class.materialization` parser tracing;
- the three `current-specialization-*` trace points;
- the local member-alias `Scope`/`AliasTemplateDecl` reconstruction and second
  template-parameter analysis;
- shadow-only counters and fields after their parity question is answered.

### Performance

The fixed diagnostic-inclusive baseline remains
`/tmp/cppgm-alias-convergence-fixed.json`, SHA-256
`cefe54dacaaa8f6c5757cc90b3b9af2738507f55ab40d6abc226466114c2390b`:

| Metric | Fixed baseline | Dirty audit median | Delta |
| --- | ---: | ---: | ---: |
| Instructions | 176,018,488,694 | 192,990,370,390 | +9.64% |
| Maximum RSS | 757,092,352 | 793,448,448 | +4.80% |
| Peak footprint | 593,022,976 | 620,085,248 | +4.56% |

Against the last clean class-materialization confirmation, the dirty median is
+10.12% instructions, +6.84% RSS, and +7.74% footprint.

The first dirty batch's RSS samples are 790,216,704, 793,448,448, and
796,696,576 bytes: a 6,479,872-byte range, or about 0.82% of the median. The
measured 4.80%-6.84% RSS increase is therefore much larger than this batch's
run-to-run variation.

The required second three-run gate confirms the failure:

| Metric | Fixed baseline | Confirmation median | Delta |
| --- | ---: | ---: | ---: |
| Instructions | 176,018,488,694 | 192,910,337,261 | +9.60% |
| Maximum RSS | 757,092,352 | 791,990,272 | +4.61% |
| Peak footprint | 593,022,976 | 620,224,512 | +4.59% |

The confirmation report is
`/tmp/cppgm-reset-current-perf-confirmation.json`, SHA-256
`50655ba49dc3b9d11ec49fda4034f2aa990ebc42f387d9ee746593cef034a957`.
The second RSS median remains above 3%, so the dirty checkpoint fails the RSS
rule in addition to correctness, instructions, and footprint. It may not
become a rolling baseline.

## Architectural diagnosis

### Class materialization is attached too high in the call graph

The unfinished implementation applies source-materialization scopes around
general declaration/type analysis. Those functions are also used for lookup,
SFINAE, replay, and other queries that do not materialize a public source type
node. A typed flag at that level is structured, but it is still the wrong
semantic fact. The 216 extra class rows are the expected symptom.

Materialization must originate only at the operation that consumes a specific
source AST node as a concrete type. Lookup may return the same `TypePtr`; that
does not make the lookup a source materialization.

### Alias completion still has two semantic analyses

The canonical completion object and source-owner improvements fix real cases,
especially current-specialization aliases. However, member aliases that normal
collection did not expose were bridged by constructing a second local semantic
scope and re-running template-parameter analysis. This recreates part of the
declaration collector and perturbs lifetime/lookup behavior.

Factor declaration indexing from alias-target resolution in the primary
collector. The normal collector should publish a compact typed declaration
handle once; source occurrence analysis should consume that handle without
rebuilding the scope, parsing parameters again, or instantiating the alias
target twice.

### Source spelling, selected semantics, and public owner are conflated

The remaining alias failures fall into four clusters:

- six structured spelling/layout differences;
- four wrong source-owner presentations;
- two extra qualified alias rows;
- one missing nested alias row.

The semantic result needs separate fields for resolved meaning, source AST
spelling, and public owner mode. The owner mode must be a fact from lookup
(lexical source owner versus selected concrete owner), not inferred by the
renderer or chosen by whichever completion writes first.

### First-writer occurrence storage hides repeated work

The pending alias occurrence map currently allows one partially populated
completion to win and later completions to enrich or lose. That preserves the
same architectural problem as renderer deduplication at an earlier layer.
Occurrence identity should make repeated analysis unnecessary; the primary
semantic operation should produce the complete result once.

### The expanded corpus exposes additional convergence work

All current function-call mismatches and both variable-use mismatches are in
newly generated references; they are not regressions against the old 1,305
gate. They nevertheless prevent the regenerated corpus from becoming the new
correctness contract. Function calls still have multiple request-building
feeds and generic arbitration, so they need their own typed-result convergence
rather than alias/class exceptions.

Lifecycle mismatches mix 16 tracked regressions with 70 new-reference gaps in
the dirty tree. Lifecycle demand must be generated by definition/instantiation
semantics and must not be used as a class- or alias-visibility policy.

## Recovery rules

These rules apply to every phase:

1. No source-text, token, rendered-name, template-name, fixture, or
   source-location filter may decide whether a row exists.
2. Source AST may be retained for final spelling. It may not be reparsed to
   recover semantic ownership.
3. A phase moves behavior to the semantic operation that already computes it;
   it does not add another observer-side analysis.
4. Do not keep two complete semantic algorithms across more than one shadow
   checkpoint. Shadow code answers one parity question and is then deleted.
5. The old 1,305 tracked-reference gate must remain exact after every behavior
   change. A tracked regression blocks the phase immediately.
6. On the 225 new references, the failing set may only shrink. Record any test
   whose mismatch changes family or payload even when the total count falls.
7. Collect provenance for all 1,530 examples, not only focused fixtures.
   Counters must name the semantic owner and distinguish attempt, insertion,
   repeated analysis, publication, renderer action, and final visibility.
8. Investigate semantic duplication, extra allocation, or representation
   growth before rolling back a valid semantic change merely because a
   performance warning appears.
9. No global witness deduplication is removed until alias, class, function,
   variable, and lifecycle provenance separately proves that the affected
   generic branch has zero destructive actions.
10. Use Homebrew Clang for every build and performance gate. Inception is last.

## Execution plan

### Phase 0: Preserve the experiment and restore a trustworthy base

1. Preserve the current tracked diff, the 18 fixed-test list, the 139
   regression list, focused output diffs, and the 225 new `.ref.witness` files
   under a durable side reference or hashed patch bundle. Exclude `.my`,
   provenance-expanded, object, profiler, and other generated files.
2. Add the 225 patched-Clang `.ref.witness` files to the durable test corpus.
   Record the patched compiler path and LLVM revision in the ledger.
3. Restore production compiler files to clean `5add5290c...`; do not discard
   the preserved experiment. Keep the regenerated references.
4. Reconcile the stale PA12 switch fixture with the correct bypass rejection.
   The positive form needs braces or the reference must expect rejection,
   according to the behavior that fixture intends to own.
5. Rebuild ordinary and provenance compilers with Homebrew Clang. Remove all
   ordinary-build warnings.
6. Establish the recovery checkpoint:
   - tracked strict: 1,305/1,305;
   - expanded strict: 1,339/1,530, with the same 191-gap manifest;
   - broad: 4,862/4,862 after the PA12 fixture correction;
   - text-reparse, materialization, and provenance unit audits clean.
7. Add only the diagnostic counters required by the expanded provenance
   contract, prove ordinary output parity, then record the missing
   `/tmp/cppgm-class-materialization-ownership-fixed.json` three-run baseline.

Do not replay any semantic fix until this phase is committed and its evidence
is recorded.

### Phase 1: Establish expanded-corpus ownership evidence

1. Regenerate ordinary and provenance output for all 1,530 tests.
2. Give each class, alias, function, variable, and lifecycle producer a stable
   diagnostic route ID at its true semantic operation.
3. Produce one report containing, per source occurrence:
   - patched-Clang expected row or expected absence;
   - CPPGM semantic owner and operation kind;
   - attempt/insertion/publication counts;
   - direct, nested, replay, lookup-only, and materialization state;
   - source occurrence ID and selected declaration ID.
4. Record the clean 191-gap manifest by event family and semantic owner.
5. Add reduced tests only for a proven unexercised owner; never add a fixture
   solely to encode an implementation exception.

Acceptance: diagnostic and ordinary output agree; counters cover every
attempt; no unknown producer remains; ordinary structures and allocation
counts do not grow.

### Phase 2: Converge class source materialization

Start from the clean 62 class-gap set, not the dirty 195-gap set.

1. Inventory every direct class-member read from `Scope::named_types`. Route
   consumers that may encounter a deferred declaration through one canonical
   member-type lookup operation. Keep current eager resolution until this
   migration passes strict and broad tests. Do not route generic lexical
   search, in-progress collection, equivalence/cache probes, or inherited-using
   discovery through completing lookup. Introduce named non-completing query
   modes for those consumers, then remove their duplicate raw-map traversal.
   Leave registration, indexing, reset, binding, and diagnostic iteration as
   explicit storage operations.
2. Split declaration indexing from alias-target evaluation in the primary
   class collector. Index the name, access, source declaration, and embedded
   semantic declarations once. A typedef that declares an enum or class must
   still collect that type and its members before the collector returns.
3. Add focused parity coverage for anonymous enum typedefs, multiple
   declarators, current-specialization lookup, out-of-class definitions,
   friend access, member signatures, SFINAE, and hosted libc++ traits.
4. Introduce a compact parameterized source-occurrence handle during primary
   template-id analysis. Preserve the explicit source pattern instead of
   manufacturing an occurrence from each implicit instantiation.
5. Produce `ResolvedSourceTypeMaterialization` only when an operation analyzes
   the exact source AST type node into a concrete type. Lookup, SFINAE,
   constant queries, and code-generation replay may consume a type without
   creating another source occurrence.
6. Match the patched-Clang traversal domain: source patterns and explicit
   specializations are primary; deduced type locations, CTAD fallback, and a
   materialized variable-template initializer use their explicit semantic
   boundaries.
7. Resolve the 36 missing and ten extra presence rows before changing payload
   formatting. Resolve the 57 changed payload rows from the same completed
   source result.
8. Delete overbroad materialization scopes plus ambient lifecycle,
   source-mode, conversion-name, and source-spelling admission logic. Delete
   each shadow field after its parity question closes.

Acceptance:

- zero class-use mismatch on 1,530 tests;
- one attempt and one insertion per visible class occurrence;
- zero class renderer visibility actions;
- no materialization fact from lookup-only analysis;
- forbidden materialization reparses remain absent;
- tracked strict and broad suites are exact.

### Phase 3: Finish alias semantic convergence

Start from the clean 24 alias-gap set and preserve the useful behavior shown by
the 12 alias-family fixes in the dirty experiment.

1. Reuse the Phase 2 declaration index and canonical named-type lookup. Publish
   one typed member-alias handle after the collector has preserved all
   declaration-side semantic work.
2. Remove the local reconstructed `Scope`/`AliasTemplateDecl` bridge and the
   second `parse_template_parameters` pass.
3. Produce the 16 missing alias rows from primary explicit-source analysis.
   Stop the two extra rows by excluding implicit-instantiation traversal.
4. Make the canonical alias completion result carry separate structured facts
   for:
   - resolved type and arguments;
   - source-occurrence arguments before defaults and pack flattening;
   - parsed source AST, qualifier form, and anchor;
   - selected alias declaration;
   - lexical source owner;
   - selected concrete owner;
   - explicit public owner mode from lookup;
   - nested child occurrence ownership.
5. Add one semantic template-argument printer with the same kind dispatch as
   Clang's `TemplateArgument::print`: type, value or expression, template
   name, template expansion, and pack. Reuse the compiler's structured type
   and expression printers. Extend those printers where their formatting
   differs from the patched-Clang policy; do not add alias-only string edits.
6. Build public alias bindings by zipping alias parameters with the
   source-occurrence semantic arguments. Do not emit omitted defaults or an
   omitted empty pack. Fix the nine argument and pack rows through this
   builder, the three owner rows through selected declaration and qualifier
   facts, and the six rendering rows through the semantic printer.
7. Produce the source-occurrence arguments during the existing argument
   resolution. Do not run a second parse, a second semantic argument
   resolution, or alias-target instantiation to create witness payload.
8. Make builtin, cache, direct, dependent, current-specialization, member, and
   fallback arms finish through that one completion boundary.
9. Delete `source_argument_texts` from alias payload selection, the
   `ExplicitArgumentsOnly` probe, source-spacing repair, first-writer
   enrichment, recursive alias target instantiation,
   consumer replay, and alias-specific table/renderer conflict policy as their
   provenance counts reach zero.

Acceptance:

- zero alias-use mismatch on 1,530 tests;
- one successful completion and one insertion per source occurrence;
- no second AST parameter analysis or alias-target instantiation;
- all public alias bindings come from the source-occurrence semantic argument
  result and are explicit;
- no alias binding comes from raw source text or renderer spacing repair;
- alias duplicate/reject/replace/enrich actions are zero;
- alias table rows equal public rows;
- tracked strict and broad suites are exact.

### Phase 4: Repair lifecycle ownership independently

1. Classify the clean 79 lifecycle gaps by normalized semantic fact:
   `definition-demand`, class instantiation, function instantiation, alias
   instantiation, variable instantiation, and class finalization.
2. Attach each demand to the operation that creates or consumes the required
   semantic entity. Do not infer lifecycle from a source-use row, and do not
   use lifecycle context to admit a source-use row.
3. Permit provenance-distinct repeated lifecycle events internally. Do not
   add semantic state solely to reproduce or suppress Clang event
   multiplicity. Preserve locations, triggers, reasons, and details in debug
   output.
4. Make the strict matcher compare normalized closure fact sets. Missing
   facts and unexpected terminal outcomes fail; additional definition-demand
   facts warn and require an explanation before the phase is promoted.
5. Recheck the two PA23 LowIR regressions while migrating conversion-function
   and enable-if ownership; lifecycle output and generated definitions must
   agree on the same semantic result.

Acceptance: zero missing closure facts, zero unexpected terminal outcomes,
all extra definition-demand warnings explained, exact PA23 LowIR, and no
lifecycle-driven class/alias admission. Raw event multiplicity and provenance
are diagnostic rather than ordinary correctness requirements.

### Phase 5: Converge function-call and variable-use results

1. Inventory the remaining function request-building feeds and measure unique
   row ownership across all 1,530 tests.
2. Make overload/call resolution return one typed source-call result carrying
   callee, selected declaration, bindings, rejected candidates, source anchor,
   and required definition demand.
3. Remove consumer-side request reconstruction and repeated speculative
   publication. A failed candidate may remain debug data but may not create a
   public source row.
4. Apply the same operation-result rule to the three clean variable-use gaps.
5. Delete function/variable table or renderer arbitration only after attempts
   equal insertions and generic destructive-action counters are zero.
6. Re-run the PA30 failures as focused gates after each semantic ownership
   change; the compiler must execute both programs successfully.

Acceptance: zero function-call and variable-use mismatches, exact PA30 focused
runtime tests, and no remaining generic dedup obligation for these families.

### Phase 6: Delete migration scaffolding and prove simplification

1. Delete all temporary trace fields, shadow counters, parity paths, local
   semantic mirrors, stale route enums, and unused adapters.
2. Remove diagnostic code from ordinary layouts and prove that `Type`,
   `TemplateArgument`, and `ClassInfo` do not grow.
3. Audit every remaining source table and renderer pass. Each destructive
   branch must have a named nonzero semantic obligation or be deleted.
4. Run static audits for forbidden text/token reconstruction and duplicate AST
   semantic walks. Extend the audit where the current text-reparse audit cannot
   see structured duplicate analysis.
5. Confirm production source is smaller than the Phase 0 recovery checkpoint
   and that the ordinary build is warning-free.

Acceptance: all migration-only code is gone; production source has a net
deletion; generic dedup remains only for explicitly documented, nonzero
obligations.

### Phase 7: Final correctness and performance gates

Run from a committed, clean worktree with an isolated object root:

0. Align the performance tool with this plan before recording a candidate.
   `check` must default to three runs, 0.5% instructions, 1% footprint, and a
   3% RSS warning. The RSS warning must start one second three-run batch, and a
   second median at or above 3% must fail. Add parser-default tests so the
   documented policy cannot drift from the executable defaults.

1. focused positive/negative fixtures for every migrated owner;
2. ordinary and provenance strict with direct LowIR: 1,530/1,530, using exact
   source-use matching and normalized closure-fact matching;
3. PA1-PA38 report: 4,862/4,862 or the then-current fully explained count;
4. provenance invariants for every event family;
5. materialization, text-reparse, duplicate-semantic-walk, and analyzer unit
   audits;
6. ordinary binary and section sizes plus hot structure sizes;
7. one three-run performance candidate against both fixed and rolling
   baselines.

Performance policy:

- instructions: investigate at +0.5% during intermediate work; final median
  must be below the Phase 0 fixed checkpoint;
- peak footprint: at most +1%; final reduction remains a goal;
- maximum RSS: +3% warning starts one second complete three-run gate; a second
  median at or above +3% fails;
- intermediate typed-result phases may temporarily grow, but every
  correctness-clean phase records the divergence and its cleanup owner;
- investigate copies, allocations, repeated semantic operations, code layout,
  and side-store growth before abandoning a valid semantic result;
- promote a rolling baseline only after correctness, provenance, and the
  applicable performance decision are clean.

### Phase 8: Joint inception and closure

Only after Phases 0-7 are committed and both original active plans have their
completion checklists reconciled with the 1,530-reference corpus:

```sh
make inception \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  INCEPTION_OBJ_ROOT_BASE=/tmp/cppgm-witness-convergence-inception
```

Record the isolated object root, host compiler, final commit, and result in all
three ledgers. Inception does not substitute for any earlier correctness,
provenance, size, source-reduction, or performance gate.

## Per-phase validation cadence

For every behavior-changing phase:

1. run focused tests and inspect semantic provenance;
2. run the tracked 1,305-reference strict gate; stop on any regression;
3. run the expanded 1,530-reference gate and prove the remaining set only
   shrinks;
4. run affected PA broad tests, followed by full PA1-PA38 at the phase commit;
5. remove the superseded algorithm before measuring;
6. record source, structure, binary, and three-run performance evidence;
7. commit the phase and update the recovery ledger before advancing.

This cadence is intentionally stricter than the recent implementation loop.
A focused exact match is evidence for one owner, not permission to accumulate
unmeasured regressions elsewhere.

## Final completion criteria

- [ ] Current experiment is preserved without generated artifact noise
- [ ] All 225 new patched-Clang references are durable
- [ ] Recovery checkpoint passes tracked strict and full broad correctness
- [ ] Class materialization originates only at source-node semantic operations
- [ ] Alias declaration indexing has no second semantic parameter analysis
- [ ] Alias, class, function, and variable source-use mismatches are all zero
- [ ] Closure fact gate has no missing facts or unexpected terminal outcomes
- [ ] Strict with direct LowIR passes 1,530/1,530
- [ ] PA1-PA38 report passes the complete current corpus
- [ ] PA23 LowIR and PA30 runtime regressions are resolved semantically
- [ ] No forbidden source-text/token reparse or fixture filter exists
- [ ] No migration trace, shadow algorithm, or ordinary-build warning remains
- [ ] Per-family attempts equal insertions and renderer destructive actions are zero
- [ ] Production source is smaller than the recovery fixed checkpoint
- [ ] Final instructions are below the recovery fixed checkpoint
- [ ] Footprint and RSS satisfy the final policy
- [ ] Alias and class plan ledgers are reconciled with expanded-corpus evidence
- [ ] Joint inception passes last
