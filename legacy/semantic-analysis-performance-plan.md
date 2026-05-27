# Semantic Analysis Performance Plan

## Goal

Find and remove current semantic-analysis bottlenecks after the text-reparse cleanup work. The target is not a microbenchmark: use a medium-sized, STL-heavy compile that exercises overload resolution, template argument substitution, class template references, and class completion in a way that resembles real course/self-host workloads.

The optimization pass should preserve the existing strict semantic and LowIR comparison behavior. Timing runs should be done without expensive hotspot tracing; counter runs should be separate and used only to localize work.

## Benchmark Selection

Primary benchmark:

- `benchmarks/self_compile/stable/semantic_overload.cpp`
- Frozen from the current `dev/src/semantic_overload.cpp` source because the previous snapshot included removed headers.
- Rough size: about 9k source lines plus heavy libc++ includes.
- Expected current behavior: reaches a late semantic failure on unsupported hosted-library surface, but finishes the semantic workload in roughly 1-2 minutes and emits semantic counters before the failure. This makes it useful for relative performance work even before it becomes a fully green self-compile.

Green controls:

- PA34 STL compile smoke cases such as unordered-map/find and ostringstream tests.
- PA35 hosted-link smoke cases when broader validation is needed.
- Strict semantic suites `pa18 pa19 pa21 pa22` remain the fast correctness gate after each implementation change.

Timing discipline:

- Timing baseline: no `CPPGM_SEMANTIC_HOTSPOT=1`.
- Localization baseline: one counter/hotspot run after timing is recorded.
- Compare wall time, user time, max RSS, and semantic counters. Treat hotspot run wall time as diagnostic only.

## Current Observations

An initial probe on current `dev/src/semantic_overload.cpp` with semantic counters and hotspot tracing showed the dominant work is no longer source reparsing. The large counts are now semantic operations:

- `complete_class_type` called about 250k times, with about 185k no-class returns.
- `resolve_template_arguments` called about 306k times and building about 520k canonical text fragments for keys.
- Class-template reference hot paths repeatedly hit existing specializations such as `enable_if<false, int>` and `enable_if<false, void>`.
- File timing points at `semantic.class-reference` in libc++ headers such as `allocator_traits.h`, `__tree`, `iterator_traits.h`, `pair.h`, and `__hash_table`.

This suggests the first useful fixes should reduce repeated semantic lookup/key work, not parser or source-spelling work.

Phase 1 baseline after refreshing the frozen benchmark:

- `self-semantic-overload`: known-error timing run, `104.813s`, max RSS `1240696 KB`.
- `self-semantic-overload`: known-error diagnostic counter run, `179.782s`, max RSS `1212948 KB`.
- `pa34-long-unordered-map-find`: green control timing run, `13.996s`, max RSS `171064 KB`.

The diagnostic counter run reported:

- `complete-class-type-calls=250633`, including `complete-class-type-no-class=184849`.
- `resolve-template-argument-calls=306251`.
- `resolve-template-argument-key-builds=174604`.
- `class-template-key-builds=176554`.
- `class-template-specialization-name-builds=178646`.
- Parsed/fallback type text resolution counters remained at zero.

After rebasing this work onto current `main` (`0ba01528`), the same frozen source reaches successful completion instead of the earlier hosted-library known error. Current-main measurements:

- No Phase 2 cache patch: `self-semantic-overload` timed out at `260.405s`.
- With the Phase 2 negative class-info cache: `self-semantic-overload` completed in `204.397s`.
- With the per-key class-info cache refinement: `self-semantic-overload` completed in `190.042s`.
- With the per-key class-info cache refinement and semantic stats enabled, but no hotspot tracing: `self-semantic-overload` completed in `190.847s`.
- With the per-key class-info cache refinement: `pa34-long-unordered-map-find` completed in `5.474s`.

The current `main` strict gate is red independent of this branch for:

- `pa19/tests/general/207-defaulted-nontype-expression-syntax-rewrite.t`
- `pa22/tests/general/243-inline-namespace-qualified-decltype-lookup.t`
- `pa22/tests/spec/116-inline-namespace-qualified-decltype-lookup.t`

Those failures reproduce on `/Users/vishvananda/cppgm` at `0ba01528` with the required Clang settings and also reproduce when the Phase 2 cache patch is removed.

## Phase 1: Stable Benchmark And Baseline

1. Refresh/register the frozen `semantic_overload.cpp` benchmark so it builds from current headers.
2. Add it to the structured AST benchmark runner as an allow-failure benchmark until the hosted-library unsupported semantic is fixed.
3. Record:
   - timing baseline without hotspot tracing,
   - diagnostic counter baseline with hotspot tracing,
   - one green PA34 control timing.
4. Commit the benchmark/doc setup before optimization patches.

## Phase 2: Class-Info Lookup Cache

Hypothesis: many `complete_class_type` calls strip to equivalent named types that are not classes, but the existing cache is keyed by `Type *`. Equivalent reconstructed type objects miss that pointer cache and repeatedly search `classes_by_key`.

Implementation:

1. Add a negative-only named-key cache beside the existing pointer cache in `Analyzer::class_info_for_type`.
2. Do not cache positive class-info results by named text; anonymous and layout-sensitive classes can share textual keys while still needing distinct `ClassInfo *` values.
3. Reuse the existing class-count invalidation.
4. Add narrow counters only if needed for attribution.
5. Run strict semantic tests and the semantic-overload benchmark before committing.

Expected result: modest wall-time improvement and reduced map lookup pressure in `complete_class_type`.

Result:

- A broader named-key cache for both positive and negative results was rejected. It changed strict LowIR for anonymous/layout-sensitive cases by collapsing distinct class infos.
- The committed direction is negative-only: repeated no-class lookups can be skipped, while positive class binding still uses the existing pointer cache and authoritative `classes_by_key` lookup.
- Stats from the frozen self-overload benchmark with the negative cache: `class-info-for-type-calls=9885320`, `class-info-for-type-pointer-cache-hits=8373009`, `class-info-for-type-no-class-key-cache-hits=5542`, `class-info-for-type-map-lookups=200419`.
- The follow-up refinement adds per-key epochs for `classes_by_key`, allowing positive and negative named-key cache entries to survive unrelated class insertions while invalidating when the same key is assigned again.
- Stats from the frozen self-overload benchmark with per-key epochs: `class-info-for-type-calls=9885310`, `class-info-for-type-pointer-cache-hits=8372031`, `class-info-for-type-named-key-cache-hits=193772`, `class-info-for-type-map-lookups=13153`, `class-info-for-type-map-hits=11812`, `class-info-for-type-map-misses=1341`.

## Phase 3: Template Argument Resolution Reuse

Hypothesis: existing class-template instantiation hits still repeat expensive argument expansion/resolution and canonical key construction, especially for recurring libc++ traits.

Implementation:

1. Audit `try_fast_existing_class_template_instantiation` and `reference_class_template_instantiation_with_syntax`.
2. Identify repeated raw reference shapes that can be cached safely by declaration, use scope, and structured argument identity.
3. Prefer structured keys and interned semantic objects over text keys. Text may remain only as canonical output/mangling data, not as reparsed input.
4. Preserve witness and strict fallback behavior behind existing guards.
5. Validate on strict semantic tests, PA34 controls, and the primary benchmark.

Expected result: visible reduction in `resolve_template_arguments` calls and class-template key builds.

Attempted results:

- Increasing the resolver's recent-cache ring from 64 to 256 reduced `resolve-template-argument-key-builds` by only about `1.3k` on the frozen self-overload benchmark and slowed the PA34 control. It was rejected.
- Increasing the ring to 128 was also worse on the PA34 control. It was rejected.
- Threading a precomputed class-template argument key through `template_services` removed duplicate counted `class-template-key-builds`, but the key had already been built for specialization selection and the wall time regressed. It was rejected.

Next viable direction:

- Replace the linear recent-cache experiment with a targeted hash/indexed resolver cache only if profiling shows it avoids full key construction without adding a broader per-call scan.
- Add resolver counters that distinguish expansion, probe construction, parameter-key construction, text hashing, full-cache lookup, and actual resolution misses before attempting a larger rewrite.

## Phase 4: Specialization Name And Key Churn

Hypothesis: hit paths build specialization names or canonical argument text earlier than required.

Implementation:

1. Check every class-template hit path that calls `ensure_specialization_name`.
2. Keep name construction lazy unless required for output, diagnostics, tracing, or lookup keys.
3. Ensure timing benchmarks do not accidentally enable tracing-only string construction.
4. Validate with direct LowIR text compare for strict suites.

Expected result: lower allocation/string cost on repeated instantiation hits without changing emitted output.

## Disable-And-Repair Candidate Queue

Baseline after rebasing onto local `main` at `579eb27f`:

- Strict gate: `make test-strict` with LowIR direct text compare passed.
- Stable compile counter run: `82.37s`, `343,994,658,538` instructions,
  max RSS `1,210,736,640`, peak footprint `939,655,168`.
- Long phases: `semantic.output_seed=21,578ms`,
  `semantic.fixpoint.late_required_synthesized_output=14,485ms` total,
  and `semantic.fixpoint.instantiated_template_output=11,724ms` total.
- High-volume counters: `class-info-for-type-calls=6,778,698`,
  `scope-cache-key-calls=504,270`,
  `resolve-template-argument-calls=309,533`,
  `complete-class-type-calls=90,924`.

The next pass should disable one hot recovery path at a time, run strict tests
with LowIR comparison, then replace any failing dependency with a narrower
structured fix.

1. `template_argument_semantics.cpp::try_resolve_direct_concrete_qualified_member_type`
   nested dependent-result repair.
   Disable the block that reparses a still-dependent direct member type,
   resolves a nested owner by text, prepares that owner scope, and performs a
   second member lookup. This path is a rescue after a concrete lookup has
   already returned a dependent type, so strict failures should identify the
   small set of aliases that still need a structured owner/member path.

2. `template_argument_semantics.cpp::resolve_structured_dependent_qualified_member_type`
   owner concretization.
   First disable the `current_specialization_type_for_dependent_owner` and
   `resolve_instantiated_dependent_type` branch for dependent owners. If strict
   failures are small, repair those cases at the declaration/source site that
   should have carried concrete owner metadata. If failures are broad, narrow
   to member-template-id paths only.

3. `template_argument_semantics.cpp::resolve_qualified_scope_for_class_or_namespace_impl`
   and `_node` type fallback for qualifiers.
   Disable the `resolve_type_text_without_fragment_fallback` qualifier lookup
   and then the `<...>` `resolve_type_lookup_text` qualifier lookup separately.
   Namespace resolution and explicit syntax-node semantic types should remain.
   The expected fix is to thread structured qualifier syntax/semantic type
   instead of recovering scope from full qualifier text.

4. `callsemantic.cpp::lookup_function_templates_node` and the parallel ordinary
   function lookup path around the qualifier-template-id handling.
   Disable the class/alias template instantiation plus `complete_class_type`
   path for qualified function names with template-id qualifiers, falling back
   to the existing ordinary qualified lookup. A small failure set would mean
   those AST nodes need pre-attached qualifier scope/type data rather than
   recomputing it during overload lookup.

5. `callsemantic.cpp::instantiate_alias_template` text fallback.
   Disable `parse_instantiated_alias_text_without_source_capture` at the final
   fallback after AST/structural alias resolution fail. This is an explicit
   text reparse path that bypasses the normal strict fallback counters. Failures
   should be fixed by improving `parse_type_id`/structured alias metadata, not
   by retaining the text fallback.

6. `template_resolution.cpp` template-deduction text-shape fallback.
   Disable the final `resolve_type_text_without_fragment_fallback_for_deduction`
   branch in `deduce_template_argument_text_shape`. This should only be needed
   when pattern/actual argument structure was not preserved. Failures point to
   missing structured template argument data.

7. Output fixpoint scan consolidation.
   Current output has only `2` seed nodes but `323,299` rescanned emitted nodes
   and five fixpoint iterations. After the semantic recovery experiments, audit
   `analyze_instantiated_template_output`,
   `analyze_late_required_synthesized_output`, and callee closure expansion for
   queues or dirty sets that can replace full repeated scans.

## Riskier Second-Pass Candidate Queue

The strict-clean fallback removals above left the long pole in output seed and
output fixpoint work. The next candidate set should intentionally include
higher-blast-radius switches that may break multiple strict tests, because a
small failure set will identify places where we are doing large general work for
a few semantic requirements.

1. Required-definition validation as an audit-only pass.
   `validate_required_function_definition_closure` re-resolves every required
   definition after the fixpoint, calls template acquisition again, and then
   recomputes skip policy. The current stats showed this phase at thousands of
   scans and about 1.6M `class_info_for_type` calls. First experiment: no-op the
   validation pass to measure the upper bound and failure set. If strict passes,
   keep the output-affecting refresh/acquire operations in the fixpoint and move
   the final expensive audit behind a debug/strict validation knob.

2. Emitted-output callee closure scan.
   `analyze_function_binding_output_impl` already calls
   `collect_required_callees_from_node` when it emits a function body, but
   `expand_emitted_output_callee_closure` later scans each top-level output node
   again. First experiment: disable the top-level callee closure pass. Expected
   failures should identify non-function output nodes, such as variables,
   vtables, RTTI, or synthesized wrappers, that still need direct requirement
   collection. A narrower fix would collect only for those node kinds at the
   point they are emitted instead of recursively rescanning all output.

3. Synthetic materialization-support collectors.
   The callee collector runs several ABI/lifetime support checks on every node:
   parameter materialization, return-slot transfers, variable initializer
   transfers, full-expression temporaries, exception support, delete support,
   constructor unwind support, and discarded temporaries. Many of these call
   `complete_class_type` and then require synthetic copy/move/destructor
   definitions. Disable them one at a time to see which strict tests fail. If a
   helper is only needed for a small surface, attach that requirement during the
   semantic action that creates the node instead of rediscovering it by scanning
   the whole emitted tree.

4. Complete-output-layout tree pass.
   `complete_output_layout_types` walks the final `CallSemNode` tree and asks
   for layout completion for every ABI-visible semantic type. First experiment:
   skip this pass and compare strict failures. A broad failure set would confirm
   the pass is still carrying essential layout side effects; a small set would
   let us move layout completion to the specific emitters that produce object,
   parameter, and return types.

5. Output seed declaration walk.
   The output seed still recursively analyzes source declarations even though
   declaration collection already built the semantic binding graph and the
   requirement queues later emit definitions. First experiment: in object/LowIR
   mode, seed only primary-source function/variable definitions and explicit
   required roots, leaving class/template/member output to the queues. Expected
   strict breakage is source-order drift and missing declaration-only output.
   That failure set should drive an explicit output-root list instead of a
   recursive AST walk.

6. Persistent class output-readiness cache.
   `class_output_readiness` is recomputed in output phases and validation. A
   deliberately risky experiment is to cache readiness per `ClassInfo *` across
   the entire output fixpoint with no invalidation, to measure the upside and
   expose tests where readiness changes during emission. If the upside is real,
   keep only stable states, such as complete non-dependent readiness, or clear
   the cache on class completion/output-state changes.

7. Late synthesized class sweep.
   The late synthesized pass has direct queues for required class methods and
   static functions, but it also performs a class-wide sweep keyed by
   `has_late_required_class_*_output`. Earlier removal produced mostly LowIR
   source-order drift. Revisit this as a scheduling problem: disable the class
   sweep, record the drift set, then queue class members in declaration order
   when the requirement is first noted instead of discovering them through a
   later broad class scan.

8. Direct-call definition requirement volume.
   `collect_required_callees_from_node` currently treats every emitted direct
   callee as a required definition and lets the central output planner downgrade
   declaration-only/runtime cases later. Disabling or narrowing direct-call
   definition requirements for inline/weak/library functions is likely to break
   many output tests, but it may also cut the `required-definition-requests`
   volume sharply. Use strict failures to separate calls that truly need emitted
   definitions from calls where declaration/export metadata is enough.

9. Stats-only duplicate callee traversal.
   When semantic counters are enabled, `note_required_callee_rescan` recursively
   counts nodes and then the collector recursively walks the same subtree again.
   This does not affect correctness and should be handled separately from the
   risky semantic experiments: fold the node counter into
   `collect_required_callees_from_node` so stats-mode performance does not pay a
   second tree walk.

## Phase 5: Broader Validation

After measurable wins on the primary benchmark:

1. Run `CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`.
2. Run PA34/PA35 targeted `test-report` for STL-heavy cases.
3. Run a full report when the optimized patches are stable.
4. Update this document with final before/after numbers and any remaining bottlenecks.
