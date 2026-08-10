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
