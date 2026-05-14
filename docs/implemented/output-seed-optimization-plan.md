# Output Seed Optimization Plan

## Current Evidence

The current stable compile baseline is still dominated by semantic work. The
latest clean stats-enabled run measured:

- `real=75.82s`
- `instructions=334,268,627,728`
- `max_rss=1,228,107,776`
- `peak_footprint=936,488,960`

Top phase totals from that run:

| Phase | Time |
| --- | ---: |
| `semantic.output_seed` | `22.13s` |
| `semantic.fixpoint.late_required_synthesized_output` | `11.81s` |
| `semantic.fixpoint.instantiated_template_output` | `10.28s` |
| `semantic.collect_declarations` | `7.02s` |
| `semantic.fixpoint.synthetic_function_output` | `3.98s` |
| all LowIR collect phases | `~3.31s` |

The fresh sample taken during the run confirms that `semantic.output_seed` is
not mostly top-level declaration walking. The seed loop visits only two
top-level nodes and appends one output node, but that one seed emits and
semantically analyzes a large primary-source function body.

The sampled hot path is:

1. `Analyzer::analyze`
2. `semantic.output_seed`
3. `analyze_declaration_output_impl`
4. `analyze_function_definition`
5. `analyze_function_binding_output_impl`
6. `semantic_statement::analyze_statement`
7. `semantic_expression::analyze_member_expression`
8. `complete_class_type_for_lookup`
9. `Analyzer::complete_class_type`
10. `populate_class_info`
11. `collect_class_simple_declaration`
12. `maybe_complete_class_member_object_type`

Inside that chain, the sample repeatedly shows:

- full class completion for field offsets,
- transitive field-member class completion,
- `prepare_named_type_member_scope`,
- `ensure_class_reference_members`,
- `collect_template_declaration`.

This points to repeated construction of class member semantic state, not LowIR
or source reparse.

Follow-up instrumentation added after the first pass now reports per-class
output-seed materializations and reference-before-full duplicate walks. On the
stable `semantic_overload.cpp` self-compile, the stats-enabled run measured:

- `semantic.output_seed=25.26s`
- `real=106.03s` with stats enabled
- `instructions=336,645,789,995`
- `max_rss=1,200,963,584`
- `peak_footprint=950,972,416`

The stats-disabled check stayed instruction-stable for the same compile:

- `real=109.95s` on a loaded machine
- `instructions=334,375,298,338`
- `max_rss=1,224,052,736`
- `peak_footprint=950,038,528`

The top output-seed class materializations by direct class-body child count were:

| Rank | Class | AST children |
| --- | --- | ---: |
| 1 | `semantic_metrics::AnalyzerCounters` | 156 |
| 2 | `std::__1::unordered_map<unsigned long int, vector<FunctionBinding *>, ...>` | 106 |
| 3 | `semantic_model::FunctionBinding` | 79 |
| 4 | `semantic_model::ClassInfo` | 68 |
| 5 | `semantic_model::FunctionTemplateDecl` | 45 |
| 6 | `std::__1::basic_ostream<char, typename _Traits>` | 45 |
| 7 | `cpp_decl::Type` | 44 |
| 8 | `semantic_model::Scope` | 44 |
| 9 | `std::__1::unique_ptr<CppAstNode, std::__1::default_delete<CppAstNode>>` | 42 |
| 10 | `std::__1::__wrap_iter<semantic_model::FunctionBinding **>` | 34 |

The top reference-before-full duplicate walks overlap heavily with those same
compiler model classes:

| Rank | Class | Reference children | Full children |
| --- | --- | ---: | ---: |
| 1 | `semantic_metrics::AnalyzerCounters` | 156 | 156 |
| 2 | `semantic_model::FunctionBinding` | 79 | 79 |
| 3 | `semantic_model::ClassInfo` | 68 | 68 |
| 4 | `CallSemNode` | 49 | 49 |
| 5 | `semantic_model::FunctionTemplateDecl` | 45 | 45 |
| 6 | `cpp_decl::Type` | 44 | 44 |
| 7 | `semantic_model::Scope` | 44 | 44 |
| 8 | `semantic_model::ValueBinding` | 38 | 38 |
| 9 | `std::__1::locale` | 35 | 35 |
| 10 | `CppAstNode` | 31 | 31 |

Additional member-object completion attribution shows that the layout frontier
is broad rather than concentrated in a few repeated classes. A stats-enabled
`semantic_overload.cpp` self-compile measured:

- `member-object-completion-calls=4,207`
- `member-object-completion-complete-type-calls=538`
- `member-object-completion-layout-syncs=538`
- `member-object-completion-classes=538`
- output-seed parent demand: `893` calls, `0` complete-type calls
- field-member-object parent demand: `905` calls, `203` complete-type calls
- class-layout parent demand: `79` calls, `76` complete-type calls

The top attributed member-object completions each occurred once, which means a
layout-only path needs to make one-time class layout cheaper across many
libc++/compiler-model instantiations rather than caching repeated completions
of one class. The top entries by class-body child count were:

| Rank | Class | AST children |
| --- | --- | ---: |
| 1 | `std::__1::deque<PostToken, std::__1::allocator<PostToken>>` | 168 |
| 2 | `std::__1::deque<RecogToken, std::__1::allocator<RecogToken>>` | 168 |
| 3 | `std::__1::__hash_table<...CachedArgumentConversionKey...>` | 158 |
| 4 | `std::__1::__hash_table<...std::string, std::vector<unsigned long int>...>` | 158 |
| 5 | `std::__1::__hash_table<...std::string, unsigned long int...>` | 158 |
| 6 | `std::__1::__hash_table<...unsigned long int, std::vector<FunctionBinding *>...>` | 158 |
| 7 | `std::__1::__hash_table<std::string const *, ...>` | 158 |
| 8 | `std::__1::vector<CallSemNode, std::__1::allocator<CallSemNode>>` | 144 |
| 9 | `std::__1::vector<RecogToken, std::__1::allocator<RecogToken>>` | 144 |
| 10 | `std::__1::vector<SourceLocation, std::__1::allocator<SourceLocation>>` | 144 |

A source-location gating experiment around `semantic_hotspot` expression and
constant-evaluation hooks was rejected. It improved noisy wall/cycle timings in
one run but raised retired instructions from `334.38B` to `335.61B` and then
`337.55B` on repeat.

A narrow output-seed-only reference/full reuse experiment was also rejected.
The prototype collected ordinary data fields during output-seed reference
collection for classes without bases/templates/nested definitions/anonymous
members/in-class function bodies, then let full completion skip the reset and
member-body rescan for those reusable classes. It passed the direct
`semantic_overload.cpp` self-compile, but the duplicate-walk counters barely
moved and stats-disabled retired instructions regressed to `336.64B` and then
`337.59B`. After backing it out, strict LowIR compare still passes with only
the attribution counters kept.

A `prepare_named_type_member_scope` request-kind split was also rejected. The
prototype tagged callers as type lookup, value lookup, or scope-only and let
scope-only requests return the existing member scope without
`ensure_class_reference_members`. On the stable `semantic_overload.cpp`
self-compile, the stats run classified `61,074` prepares as `61,013` type
lookups, `61` value lookups, and `0` scope-only prepares. The output-seed
demand alone had `15,012` prepares, `754` ensures, and still `0` scope-only
prepares. Since the safe skip surface was absent and the stats-disabled sample
retired `336,468,023,813` instructions, the prototype was backed out.

A blanket transitive member-object completion disable was run only as an
upper-bound experiment and immediately backed out. The code already skips
direct `maybe_complete_class_member_object_type` work under `CDK_OUTPUT_SEED`;
the experiment additionally returned for all other parent demands after
counting the call. Strict LowIR compare failed in a bounded set:

- pa18: 4 failures
- pa19: 1 failure
- pa21: 1 failure
- pa22: 4 failures

The stable `semantic_overload.cpp` self-compile did not reach the benchmark
phase. It failed during declaration collection for `std::nested_exception`:
`unsupported class member object name=__ptr_ type=class std::exception_ptr`.
That confirms the path cannot be skipped wholesale; the repair needs a
layout-only completion mode that can populate object size/alignment without
full semantic completion.

## Working Theory

The seed phase is paying for at least three overlapping kinds of class work:

1. Full class completion to answer member-expression layout/offset questions.
2. Reference-member collection to make member type/template/alias lookup work
   while parsing declarations in those class completions.
3. Transitive member-object completion from `maybe_complete_class_member_object_type`
   while collecting fields, even when the immediate seed expression only needs
   enough layout to compute one access path.

Those paths all walk parts of the same class ASTs and build overlapping state.
Small local early returns have mostly failed because they add probes to hot
paths without actually avoiding the expensive class walks. The optimization
needs to collapse or reuse those walks, not just skip one predicate.

## Plan

### 1. Add Seed-Specific Attribution Counters

Before changing behavior, add stats-mode counters that explain the output-seed
class work directly:

- Number of function bodies emitted in `semantic.output_seed`.
- Number of statements and expressions analyzed under `semantic.output_seed`.
- Number of `complete_class_type` calls under `CDK_OUTPUT_SEED` that actually
  materialize a class.
- Number of `maybe_complete_class_member_object_type` calls, broken down into:
  already-layout, dependent skip, no class, completed class, and populated
  direct fallback.
- Number of classes where reference-member collection happens before full
  member collection in the same compile.
- Number of class AST children walked in reference collection and then walked
  again during full collection.
- Number of `prepare_named_type_member_scope` calls that only need member scope
  existence versus calls that actually require reference member declarations.

Acceptance for the instrumentation step: strict LowIR compare passes, and the
stable compile does not materially regress with stats disabled. The counters
should make it possible to say which classes and which semantic action produce
the seed long pole.

### 2. Collapse Reference Collection And Full Collection

Reference collection and full class collection are separate passes today, but
the sample shows reference collection being triggered inside class completion
through `prepare_named_type_member_scope`. That means we often build a partial
member view, then later parse the same class body again for full layout/method
state.

Target design:

- Introduce a reusable per-class member declaration summary built from the class
  AST once.
- Store enough structured entries for nested classes, enum declarations, alias
  declarations, alias templates, class templates, using declarations, special
  members, function definitions, fields, and bit-fields.
- Let reference collection consume the summary to bind only lookup-visible
  type/template/alias names.
- Let full collection consume the same summary for fields, functions,
  constants, and layout.
- If full collection runs first, mark reference collection satisfied.
- If reference collection runs first, retain the summary so full collection does
  not rescan the AST and rebuild the same template/alias shape.

Rejected first experiment: collect ordinary fields during output-seed reference
collection and reuse that partial state for base-free, non-anonymous,
non-template class bodies. This was too shallow: it did not materially reduce
`reference-before-full` work and raised disabled retired instructions. Do not
retry this without a real member summary that avoids the extra reference-path
parsing cost.

Accepted narrow reuse experiment: when resetting a class member scope between
reference collection and full collection, preserve nested class type bindings
whose recorded `named_member_owner_type` is the class being reset. This keeps
already-created nested `ClassInfo` identities available to the full pass while
still dropping aliases, values, templates, and other reference-only state.

Same-tree A/B on the stable `semantic_overload.cpp` compile:

| Variant | Real | Instructions | Max RSS | Peak Footprint |
| --- | ---: | ---: | ---: | ---: |
| preserve nested class types | `82.29s` | `337,931,919,711` | `1,222,451,200` | `955,580,416` |
| reset nested class types | `85.37s` | `339,332,955,346` | `1,227,882,496` | `955,584,512` |

The patch saved `1,401,035,635` retired instructions, about `3.1s` wall time,
and about `5.4MB` max RSS in that A/B. Strict LowIR compare passed before the
A/B. A phase+stats run with the kept patch measured
`semantic.output_seed=26.715s`, `reference-member-collections=5,325`,
output-seed reference collections `838`, and `221` reference-before-full
classes covering `2,009` duplicate child walks. This is not the full summary
design, but it confirms that preserving class identity across the reset can
pay off without broad semantic risk.

Rejected adjacent experiment: also preserving nested `ClassTemplateDecl`
bindings declared directly in the class scope passed strict, but regressed the
same plain benchmark to `338,702,223,989` retired instructions, `85.46s` real
time, and `1,241,714,688` max RSS. Keep only the nested concrete class type
bindings for now; class-template reuse appears to retain or invalidate enough
extra state to lose the instruction win.

Next experiment: implement the summary only for non-anonymous, non-dependent
class members and fall back to the current code for anonymous unions/classes and
known tricky friend/using cases. The goal is to prove that reused structured
member summaries reduce seed time without broad correctness risk.

Validation: strict LowIR compare, then stable stats run. Watch
`semantic.output_seed`, `field-member-object` demand, `ensure_class_reference_members`
sample presence, instructions, max RSS, and peak footprint.

### 3. Split Layout Completion From Full Semantic Completion

The most expensive seed stack starts with a member expression needing a class
layout/offset, but class completion currently builds much more than a layout
summary. It also touches member function/template/alias machinery needed for
later semantic features.

Target design:

- Add a layout-only class completion mode for field offsets and object size.
- Layout-only completion should collect bases, fields, alignment, size, empty
  state, host ABI chunks, and only the static data needed for that layout.
- It should not collect or instantiate member functions, function templates,
  alias templates, friend functions, or constexpr member values unless layout
  truly depends on them.
- Full completion can upgrade from layout-only completion without reparsing the
  class body, using the same member summary from step 2.

First experiment: do not build the full layout-only mode immediately. Instead,
add counters to classify why each `maybe_complete_class_member_object_type`
completion was needed and run a risky disable of transitive member-object
completion to measure the upper bound and strict failure set. If failures are
small, repair by requesting layout-only completion at those specific sites.

Rejected upper-bound disable: returning from
`maybe_complete_class_member_object_type` for all non-output-seed parent
demands produced 10 strict failures and stopped the stable self-compile on an
unsupported `std::exception_ptr` member object. The failure mode is exactly a
missing layout for a class-typed field, so the next experiment should be a
targeted layout-only completion for member objects rather than another skip.

Follow-up attribution shows that the expensive member-object frontier is
distributed: `538` complete-type calls across `538` classes. The first
layout-only prototype should therefore avoid special-casing individual classes.
A reasonable narrow target is a reusable "layout collected" state for class
templates/ordinary classes that:

- have no virtual bases and do not need vtable construction,
- can parse bases and fields using already-collected reference member names,
- defer member functions/templates/static constexpr evaluation,
- mark only `named_has_layout`, field offsets, `nonvirtual_size`, and
  `nonvirtual_alignment`, leaving full semantic completion for later.

Expected win: this is the most likely route to a dramatic seed reduction,
because the sample shows nested full class completion dominating the seed body
analysis.

Rejected standalone layout-only prototype: a conservative implementation added
a temporary layout-collected state, parsed bases/fields, skipped functions,
templates, static members, and virtual/dependent cases, and fell back to full
completion on unsupported declarations. After tightening it to avoid dependent
nested classes, strict LowIR compare passed. The stats run proved the mechanism
worked mechanically: member-object complete-type calls dropped from `538` to
`2`, and one sample had `semantic.output_seed=21.84s`. However it increased
other hot work (`class-info-for-type-calls=5.42M`, `reference-member-collections=5,742`)
and retired `344,128,613,178` instructions with stats enabled. The disabled
sample also regressed to `341,665,883,033` retired instructions with
`peak_footprint=959,815,680`. This was backed out. Do not retry layout-only as
a separate class-body walk; it needs the shared member declaration summary from
step 2 so layout collection does not add another reference/scope walk.

### 4. Avoid Rebuilding Template Member Scope State

`prepare_named_type_member_scope` currently calls `ensure_class_reference_members`
before returning a member scope for named class types, except for narrow
in-progress cases. In the seed hot path this happens while parsing class member
declarations. The key question is whether the lookup needs reference-declared
members or only an existing member scope.

Target design:

- Add a structured request kind to `prepare_named_type_member_scope`, such as
  `NeedMemberScopeOnly`, `NeedTypeNames`, `NeedTemplates`, or `NeedValues`.
- For member-declaration parsing where only a scope object is needed to parse a
  qualified type, return the existing member scope without collecting reference
  members.
- For real member lookup, collect only the required category from the member
  summary.

First experiment: trace or counter the call sites from
`template_argument_semantics::prepare_concrete_type_member_scope` and classify
which lookups immediately probe a member name. Then disable reference collection
only for scope-only requests and run strict to identify missing semantic data.

Rejected first experiment: the request-kind prototype found no scope-only
prepares in the stable self-compile. Nearly all calls immediately support type
lookup, with a small number of value lookups. Do not continue this split unless
a different workload shows a meaningful scope-only population; the useful next
step is the shared member declaration summary from step 2.

### 5. Revisit Seed Body Output Persistence After Class Work

After class completion is reduced, reevaluate whether seed still spends time in
body tree construction and post-body scans:

- `append_function_exception_spec_candidates`
- `function_binding_is_nothrow`
- `callsem_node_can_throw`
- `collect_required_callees_from_node`
- parameter virtual-base layout attachment
- `CallSemNode` metadata stored only for later LowIR/witness consumers

Do this after class work because current samples show these are secondary.
Potential directions:

- Collect direct callee/materialization obligations during statement analysis,
  avoiding a second body tree walk.
- Make nothrow/can-throw lazy or memoized per emitted function body.
- Split no-witness/no-trace `CallSemNode` metadata from witness/source metadata.

These should be handled as separate measured patches because earlier
source-location and callee-scan micro-edits showed that local-looking savings
can regress global instructions or memory.

Body-output follow-up results:

- Rejected kind guards around the individual
  `collect_required_callees_from_node` support collectors. The change passed
  strict LowIR compare, but raised the stable benchmark from the fresh
  `337,709,313,750` retired-instruction baseline to `338,319,969,637`.
  Keep the simple collector calls; the extra branch structure costs more than
  the skipped helper prologues.
- Accepted a narrow function-qualifier prefilter before exception-spec parsing.
  `parse_explicit_function_nothrow_parse_state` now avoids `trim_space` unless
  the raw qualifier contains `noexcept` or `throw`, and
  `append_function_exception_spec_candidates` avoids trimming qualifiers that
  cannot contain a dynamic `throw(...)` specification. Strict LowIR compare
  passed. Samples on the stable benchmark were mixed but favorable enough for a
  small keep: `337,324,151,830`, `338,078,746,433`, and `337,195,126,041`
  retired instructions, with peak footprint staying below the fresh
  `958,820,352` baseline in all samples.
- Rejected a named-type guard in
  `class_info_for_virtual_base_layout_param`. It passed strict, but the layered
  stable benchmark regressed to `338,145,565,971` retired instructions and
  higher RSS. The existing `class_info_for_type`/`complete_class_type` path is
  already cheap enough that the extra branch loses.
- Rejected removing the duplicate-looking function source-location call before
  `set_dump_token`. It passed strict, but regressed to `339,229,005,330`
  retired instructions. Leave function source/token metadata in the existing
  shape unless a broader metadata representation change is implemented.

## Suggested Execution Order

1. Add seed-specific counters and trace points.
2. Use the counters to identify the top 10 classes completed during
   `semantic.output_seed` and the top reference-before-full duplicates.
3. Disable transitive member-object completion as an upper-bound experiment;
   record strict failures and benchmark deltas, then revert.
4. Build the per-class member declaration summary for the subset shown by the
   counter data.
5. Use that summary to make reference collection and full collection share work.
6. Add layout-only completion for member-expression and field-layout needs.
7. Re-sample; only then target body-output persistence and post-body scans.

## Keep/Reject Bar

Keep a patch only if it passes strict LowIR compare and improves at least one of:

- instructions retired,
- max RSS / peak footprint,
- `semantic.output_seed` phase time,

without materially regressing the other two. For seed work, phase time alone is
not enough if instructions or memory move the wrong way; several recent
experiments already demonstrated that wall-time-only wins can be noise or
allocation tradeoffs.
