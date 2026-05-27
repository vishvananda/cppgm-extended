# Structured-AST Performance Plan

## Status

This plan is written *after* the structured-AST rewrite. Compile time has
roughly doubled relative to the previous text-based pipeline. The previous
`docs/PERFORMANCE_ANALYSIS.md` and the root-level `MAP_INSERT_PAIR_*`,
`STRING_OSTREAM_*`, `MEMOIZATION_CACHE_*` documents all predate the rewrite
and should be treated as historical context, not as descriptions of the
current bottlenecks.

The text-reparse audit (`docs/text-reparse-audit-baseline.json`) is closed:
all reparse limits are zero. That work paid off in correctness terms. The
slowdown comes from a different shape of cost.

## Core thesis

The rewrite added structured representations *alongside* the text
representations rather than *replacing* them. Almost every hot semantic path
now produces both a structured form and a string form, builds cache keys
from both, and walks both. The compiler is doing the new pipeline plus most
of the old pipeline.

That dual-representation overhead, plus a small number of structural
inefficiencies (fat AST node, no string interning, recursive scope
fingerprint, witness-mode cache disable, fixpoint rescan), accounts for the
2x regression.

The fastest way to recover compile time is **to finish retiring the text
pipeline**, not to micro-optimize the dual path that exists today.

## Evidence in the current tree

Quoted line numbers refer to `dev/src/` at HEAD of
`codex/template-main-integration-20260419` on this worktree.

### 1. Template-argument resolution is still text-keyed

- `template_resolution.cpp:5042` — `resolve_template_arguments(...)` takes
  `const std::vector<std::string> & texts`. Structured `syntaxes` is an
  optional aux parameter.
- `template_resolution.cpp:5053-5063` — first action is
  `expand_bound_type_pack_texts` and `expand_bound_expression_pack_texts`,
  both string-to-string transforms.
- `template_resolution.cpp:670-712` — cache key
  (`make_resolve_template_arguments_cache_key`) hashes a vector of trimmed
  expanded strings, plus per-parameter strings (`name`,
  `non_type_decl_specifier_text`), plus two scope binding fingerprints.
  Key equality re-runs all of those string compares.
- `template_resolution.cpp:5091` — the cache is **disabled** whenever
  `template_argument_source_locations_active()`. With witness sessions in
  the test workload, every resolve is uncached.
- `template_instantiation.cpp:3439-3440` — every `ClassInfo` carries both
  `instantiation_arguments` (structured) and `instantiation_arg_texts`
  (vector of canonical strings).
- `callsemantic.cpp:6002, 8766, 9329, 9376` and similar — many call sites
  read or write `instantiation_arg_texts` for identity comparisons.

### 2. `CppAstNode` is a fat value type, copied not moved

`cppast_ast.h:175-205` — every node carries:

```
std::string                        value;
TypePtr                            semantic_type;
shared_ptr<QualifiedName>          qualified_name_syntax;
shared_ptr<TemplateIdSyntax>       template_id_syntax;
shared_ptr<CppAstNode>             conversion_type_id_syntax;
vector<TemplateIdSyntax>           qualifier_template_id_syntaxes;
vector<CppAstNode>                 qualifier_type_syntaxes;       // recursive
vector<CppAstNode>                 exception_type_id_syntaxes;
vector<string>                     abi_tags;
vector<string>                     alignment_specifiers;
vector<CppAstNode>                 alignment_specifier_nodes;
vector<CppAstNode>                 children;
```

The setters (`set_cppast_*`, `cppast_ast.h:220-285`) all do `reset(new T(arg))` —
heap allocation per set, plus a copy-construct from the argument.

The parser uses `out.children.push_back(declaration)` rather than
`std::move` in 30+ sites in `cppast_parser.cpp` (e.g. lines 1114, 1254,
2070, 2101, 2145). Each `push_back` of a local node deep-copies the whole
subtree, including all of the above vectors and the `shared_ptr` atomic
refcount bumps.

`parse_declaration` (`cppast_parser.cpp:1135-1205`) tries 13 alternatives
sequentially against the same `out` reference. Most fail. Each failed
branch may build partial nodes (with strings, vectors, `shared_ptr`s)
before backing out.

`parse_non_type_template_default_argument` (`cppast_parser.cpp:4616`)
constructs **nested `CppAstParser` instances** on token slices for every
comma/angle boundary it sees, just to disambiguate the boundary.

### 3. Scope fingerprint walks ancestors on every call

`template_scope.cpp:87-107` —

```cpp
std::size_t parent_fingerprint =
    (!scope.namespace_scope && scope.parent)
        ? scope_binding_fingerprint(*scope.parent)
        : 0;
if (cached_valid && cached_epoch == epoch
    && cached_parent_fp == parent_fingerprint) {
   return cached;
}
```

The recursive call up the parent chain happens before the cache check,
because the cache validity test depends on the parent fingerprint. With
10-30 nested class/template scopes during instantiation, every cache-key
construction (`scope_text_cache_key`, `parsed_type_text_cache_key`,
`qualified_type_lookup_cache_key`,
`make_resolve_template_arguments_cache_key`) costs O(depth) recursive
calls.

### 4. Witness mode disables the largest cache

`template_resolution.cpp:5091`:

```cpp
const bool cache_enabled =
    !template_api::current_template_argument_source_locations_active();
```

Recent commits (the last ~30 on this branch) are almost entirely witness
work. If the test workload runs with witness sessions, the
resolve-template-arguments cache is bypassed for every call.

### 5. Container choices

- 342 `std::map<...>` declarations vs 40 `std::unordered_map`. 76
  `std::map<std::string, ...>` in headers.
- No global string interner (a grep for `intern`, `StringPool`, `Atom`
  matches `internal_symbol`, `internal_namespace`, etc., never an
  interner type).
- `Scope` (`semantic_model.h:233-251`) holds 12+
  `std::map<std::string, ...>` members. Each name lookup is O(log n) tree
  walk × O(strlen) per compare. Names like `std`, `vector`, `iterator`,
  `__1` get compared character by character on every visit.
- `std::set<std::string>` for dedup in `semantic_overload.cpp:5331, 6020`,
  keyed on a freshly built `ostringstream` per candidate
  (`candidate_match_identity`).

### 6. Class-method emit fixpoint still rescans

`semantic_output.cpp:4757-4773`:

```cpp
do {
  emitted_any = false;
  class_output_readiness_cache.clear();
  for (size_t i = 0; i < state.classes.size(); ++i) {
    for (auto & method_set : state.classes[i]->methods) {
      for (auto * binding : method_set.second) {
        emit_required_class_method(binding, emitted_any);
      }
    }
    emit_required_class_static_functions(*state.classes[i], emitted_any);
  }
  for (auto * late : state.late_required_class_methods) {
    emit_required_class_method(late, emitted_any);
  }
} while (emitted_any);
```

The neighbouring `expand_required_function_definition_closure` (4792) and
`expand_emitted_output_callee_closure` (4808) already use index-based
worklists — the right pattern is established but the class-method emit
loop has not been converted.

## Overlap with `/tmp/cppgm-blowup-fix-20260427`

A WIP tree at `/tmp/cppgm-blowup-fix-20260427` already attacks several of
the same problems. Cross-referenced against the recommendations below:

| WIP change | Files | Maps to |
| --- | --- | --- |
| `RecogTokenRangeSequence` view (no vector copy) | `cppast_parser.cpp` | Patch 4, partial |
| `suppress_template_argument_fragment_syntax` flag | `cppast_parser.{h,cpp}` | Patch 4, direct |
| `inherit_name_lookup_state_from` short-circuit when local scopes are empty | `cppast_parser.cpp` | Patch 3, partial |
| Direct `resolve_template_id_syntax_type` from `syntax->template_id` in `resolve_template_argument` | `template_resolution.cpp`, `template_argument_semantics.{h,cpp}` | Patch 1, partial |
| `resolve_template_arguments` now also receives `argument_syntaxes` | `callsemantic.cpp` | Patch 1, plumbing |
| Resolve `X<>` (empty arg list) structurally rather than rejecting | `template_argument_semantics.cpp` | Patch 1, fix |
| Witness work gated behind `witness_capture_enabled` in many callsites | `callsemantic.cpp` | Patch 8, complementary |
| `register_nothrow_function` builtin hook for operator delete | `semantic_builtins.{h,cpp}`, `callsemantic.cpp` | unrelated, correctness |
| Anonymous-class member name recursion in `collect_class_member_names_for_template_body` | `callsemantic.cpp` | unrelated, correctness |
| Pass `prepared_method.syntax.function_qualifier` for friend declarations | `semantic_class_model.cpp` | unrelated, correctness |

The WIP work covers Patch 4 (parser speculation) almost completely and
seeds Patch 1 (canonical structured template arguments). It does not yet
touch Patches 2, 3, 5, 6, 7, 9.

The witness-capture gating is complementary to Patch 8: the WIP makes the
non-witness path stop *paying* for witness work, while Patch 8 makes the
witness path stop bypassing the cache. Both are wanted.

## Patches

Each patch is sized to be landable independently with a regression gate.
Order is by expected payoff per unit risk, with WIP work folded in where
applicable.

### Patch 1 — Make structured template arguments authoritative

Goal: every hot template-argument path takes structured arguments. Text is
produced only for diagnostics and only on demand.

Concrete changes:

1. Land the WIP `resolve_template_id_syntax_type` direct path as-is. Then
   extend it: any call site that has a structured `TemplateIdSyntax`
   should reach `resolve_template_argument` with the syntax populated, so
   the new branch is actually hit.
2. Change `resolve_template_arguments(...)`
   (`template_resolution.cpp:5042`) signature from
   `const std::vector<std::string> & texts` to
   `const std::vector<TemplateArgument> & arguments`, with a separate
   optional `vector<TemplateArgumentSyntax>` for source locations. Convert
   callers; the WIP plumbing of `argument_syntaxes` is one converted
   caller already.
3. Replace `expand_bound_type_pack_texts` /
   `expand_bound_expression_pack_texts` with structural pack expansion on
   `TemplateArgument`. Keep the string versions only where they feed
   diagnostics.
4. Delete `instantiation_arg_texts` from `ClassInfo`. Identity comparison
   becomes structural argument-vector comparison — `TypePtr` pointer
   equality after canonicalization (Patch 2), NTTP value equality,
   template handle equality.
5. Cache key for the resolve-arguments cache becomes
   `(scope_id, template_decl*, args_canonical_id)`. With Patch 2, this is
   three integers; without Patch 2, it is the structured argument vector
   compared element-wise (still cheaper than text).

Why first: this single change collapses the dual-representation cost on
the hottest path. It also unblocks Patches 2 and 8.

Regression gate:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
- Do not use the whole PA34 harness as the routine gate for this slice.

Performance check:

- `resolve_template_arguments` cache hit ratio in non-witness mode must
  not regress.
- Compile-time wall clock on a representative template-heavy file (a
  focused long-running `pa34/tests/compile/` benchmark that exercises
  hosted STL template machinery) drops by at least 20%.
  If hosted PA34 is known-failing on the branch, count these as
  known-error/measurable timing probes, not correctness gates.

### Patch 2 — Canonicalize types and template arguments behind opaque IDs

Goal: equal types and equal template-argument lists collapse to a single
canonical handle. All hot caches become integer-keyed.

Concrete changes:

1. Add `TypeInterner` keyed by structural Type identity (kind, qualifiers,
   inner, parameter list, template-id components). Returns a canonical
   `const Type *` (or `TypeId` uint32). Two equal types produce one
   handle.
2. Same for `TemplateArgument` — composite of TypeId, value bits,
   template handle.
3. Migrate the hot caches to integer keys:
   `unscoped_template_id_cache`,
   `qualified_name_cache`,
   `dependent_type_resolution_cache`,
   `template_placeholder_mentions_cache`,
   `dependent_non_namespace_binding_mentions_cache`,
   `resolve_template_arguments_cache`.
4. Equal-type queries (`TypePtr a == TypePtr b`) reduce to pointer
   compare after the interner is in the build path.

Why second: every cache in the compiler benefits. This is the
order-of-magnitude move on cache-key cost. It also retires the remaining
text fields in cache keys.

Regression gate:

- ABI mangling tests are the most sensitive — any canonicalization bug
  shows up there.
- `make test-report ACTIVE_TEST_REPORT_PAS='pa15 pa16 pa18 pa19 pa21 pa22'`
- Run focused long PA34 probes for performance signal instead of the whole
  hosted correctness harness. Do not use PA35 as a routine gate or benchmark
  group here; it is long-running and has known failures on this branch.

Performance check:

- `resolve_template_arguments_cache` lookup time must drop by at least
  the cost of string hashing.
- Memory footprint must not grow — interning should net-shrink because
  duplicate types collapse.

### Patch 3 — Shrink `CppAstNode` and stop copying it

Goal: reduce per-node memory and stop deep-copying subtrees in the
parser.

Concrete changes:

1. Move the rare aux fields off the common-case node. `qualifier_type_syntaxes`,
   `exception_type_id_syntaxes`, `alignment_specifier_nodes`, `abi_tags`,
   `alignment_specifiers`, `qualifier_template_id_syntaxes` are present
   on every node but used by maybe 5%. Box them all in a single
   `unique_ptr<CppAstNodeAux>`. The common-case node shrinks by ~300
   bytes.
2. Audit `out.children.push_back(node)` in `cppast_parser.cpp` (line list
   in the evidence above) and switch to
   `out.children.push_back(std::move(node))` where the local is dead
   after the call. Mechanical change.
3. Add rvalue overloads for the `set_cppast_*_syntax` helpers and move
   qualified-name, template-id, conversion type-id, exception type-id, and
   qualifier syntax payloads into nodes where ownership is transferred.
4. Land the WIP `inherit_name_lookup_state_from` short-circuit (avoids
   materializing a merged `vector<NameSet>` when the local has no
   names).
5. Optional, larger: switch `CppAstNode` to arena-allocated
   `CppAstNode *` children with a per-translation-unit arena. Eliminates
   the copy concern entirely and turns destruction into `arena.reset()`.
   Higher risk; defer if the boxing in step 1 is sufficient.

Why third: the AST is built and destroyed once per TU but visited many
times. Shrinking it improves cache locality for every later phase.

Regression gate:

- `make -C dev cppgm++`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (parser/AST-sensitive)
- `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`

Performance check:

- Peak RSS on a moderate TU drops.
- AST-build phase wall clock drops by 10-20%.

### Patch 4 — Eliminate parser speculation churn

Goal: stop rebuilding token vectors and nested parsers for every
ambiguity probe.

Concrete changes:

1. Land the WIP `RecogTokenRangeSequence` (a view, not a copy) and use it
   everywhere `tokens.slice(start, end)` currently feeds a nested parser.
   That includes `parse_template_argument_fragment_syntax`,
   `parse_non_type_template_default_argument`, and any other site
   constructing nested `CppAstParser` instances.
2. Land the WIP `suppress_template_argument_fragment_syntax` flag so
   nested parsers do not themselves recursively build fragment syntax.
   Current partial implementation also skips the type parser for template
   argument fragments that obviously begin as expressions, avoiding one
   nested parser for literal/unary NTTP arguments while preserving the
   copied-fragment behavior that current PA10 refs expect.
3. Add per-token-position cheap classifiers used by `parse_declaration`'s
   13-way `try_core` and by `parse_non_type_template_default_argument`'s
   ambiguity probe. A failed probe at token position N stamps "not a
   declaration of kind K starting at N" so a sibling caller does not
   redo the work. Current partial implementation covers the
   `parse_non_type_template_default_argument` declaration-boundary probe
   with a conservative punctuation prefilter plus per-call cache; the wider
   `parse_declaration` classifier is still deferred.
4. Replace the "construct nested parser, try `parse_declaration`"
   pattern in `next_starts_declaration` with a syntactic predicate
   walker (no parser construction).

Why fourth: parser speculation cost scales with depth of template
arguments. STL headers hit this constantly.

Regression gate:

- `make -C dev cppgm++`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`

Performance check:

- Per-TU `parse.declaration` time on a representative STL-using TU drops
  by at least 25%.
- Allocator calls during the parse phase drop measurably (track via
  malloc-stack instrumentation if available).

### Patch 5 — Add a string interner; replace `std::map<std::string, ...>`

Goal: name lookup becomes integer-key map lookup.

Concrete changes:

1. Add `Identifier = uint32_t` and a global `IdentifierTable` populated
   by the preprocessor and the parser. Identifier creation is cheap;
   compare/hash become integer ops.
2. Replace `Scope::named_types`, `function_sets`, `class_templates`,
   `alias_templates`, `variable_templates`, `values`,
   `namespace_bindings`, `function_set_access_overrides`, etc. with
   `unordered_map<Identifier, ...>` (or a flat sorted-vector for small
   scopes — most class scopes hold under 20 names).
3. Audit other `std::map<std::string, ...>` sites (76 in headers, more
   in `.cpp`). Migrate the high-traffic ones; leave low-traffic ones for
   later.

Why fifth: this is mechanical but touches many files. Patch 2 already
removes much of the *cache-key* text cost; this patch removes the
*lookup-key* text cost.

Regression gate:

- Full `make test-report`. Migration is wide enough that nothing smaller
  is meaningful.

Performance check:

- Name-lookup phase wall clock drops by 15-30% on STL-heavy files.

### Patch 6 — Scope fingerprint: skip the parent walk when the chain is fresh

Goal: amortize `scope_binding_fingerprint` to O(1) when nothing has
changed.

Concrete change to `template_scope.cpp:87`:

```cpp
std::size_t scope_binding_fingerprint(const Scope & scope) {
  if (scope.cached_binding_scope_fingerprint_valid
      && scope.cached_binding_scope_fingerprint_epoch
         == scope.binding_fingerprint_epoch
      && (scope.namespace_scope || !scope.parent
          || (scope.parent->cached_binding_scope_fingerprint_valid
              && scope.parent->cached_binding_scope_fingerprint_epoch
                 == scope.parent->binding_fingerprint_epoch
              && scope.cached_binding_scope_parent_fingerprint
                 == scope.parent->cached_binding_scope_fingerprint))) {
    return scope.cached_binding_scope_fingerprint;
  }
  // …existing recompute path…
}
```

The cheap path checks parent epoch directly instead of recursing first.
Recursion only happens when something actually changed.

Why sixth: the change is mechanical and the function is on every cache
key construction.

Regression gate:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`

Performance check:

- `scope_cache_key_calls` counter (already wired through
  `semantic_metrics`) should fall to roughly 1 per cache-key
  construction instead of `depth+1`.

### Patch 7 — Convert the class-method emit fixpoint to a worklist

Goal: replace the do-while-emitted_any rescan in
`semantic_output.cpp:4757` with an index-based queue, matching the
pattern already used by `expand_required_function_definition_closure`.

Concrete change: track `state.required_class_method_refresh_index` and
`state.required_class_static_function_refresh_index`. Each iteration
processes only the slice
`[refresh_index, current_size)`. New emissions append to the end.
The first naive class/late-method index attempt changed LowIR order for
`pa15/tests/spec/273-global-class-array-init.t` by emitting a synthesized
destructor before the constructor. Any retry must preserve the existing
fixpoint pass ordering for synthesized class methods.

Why seventh: the loop is small, but it scales like
`classes × methods × iterations`, and the inner work is non-trivial
(`emit_required_class_method` walks output state).

Regression gate:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16 pa18 pa21 pa22'`

Performance check:

- Track the number of times `emit_required_class_method` is called
  for each method. Should converge to ~1 per method instead of `n`.

### Patch 8 — Make the resolve-arguments cache safe under witness mode

Goal: stop bypassing the cache in `template_resolution.cpp:5091`.

Two equivalent options; pick one:

- **8a (preferred).** Cache only the resolved structure
  (`vector<TemplateArgument>` without source-location annotations). On
  hit, re-attach source locations from the live frame. Source-location
  attach is cheap because the locations live in
  `ScopedTemplateArgumentSourceLocations` already.
- **8b.** Split the cache key. The `(scope, template, args)` part is
  shareable; the source-location overlay is computed separately on every
  call.

Combine with the WIP witness-capture gating in `callsemantic.cpp` (which
already short-circuits non-witness mode out of all the source-location
walks) so that *both* modes pay only what they need.

Why eighth: large effect on workloads that run with witness sessions
(today, most of them).

Regression gate:

- Full `make test-report` with witness session enabled (default).

Performance check:

- `resolve_template_arguments_cache` hit ratio in witness mode rises
  from 0% to non-trivial.

### Patch 9 — Build an incremental semantic dependency engine (optional)

Defer this until Patches 1-8 land and have measurable impact.

Goal: ask expensive semantic questions once and wake only the
dependents that actually need reprocessing.

Changes:

- Record dependency edges between function bindings, class completions,
  template instantiations, required-definition consumers.
- Replace the remaining whole-state passes with edge-driven worklists.

Why last: high risk. Only worth doing if the previous patches still
leave fixpoint-style churn visible in the profile.

## Sequencing notes

- Patches 1, 4, 8 should land first. They are the highest-leverage and
  the WIP tree at `/tmp/cppgm-blowup-fix-20260427` already has partial
  starts on 1, 4, and a complementary path on 8.
- Patch 2 needs Patch 1 to be useful — without canonical handles you
  cannot cheaply key caches on structured arguments.
- Patches 3 and 6 are independent; either can land at any time.
- Patch 5 is wide and disruptive — schedule after 1-4 settle.
- Patches 7 and 8 are independent; both small.
- Patch 9 is optional and depends on the rest.

## Measurement discipline

For every patch, record:

- HEAD before and after.
- Wall clock on a fixed small benchmark set (a handful of
  `pa34/tests/compile/*.t`, plus one `dev/src/*.cpp` if it builds).
- Counters from `CPPGM_SEMANTIC_STATS=1` and
  `CPPGM_SEMANTIC_HOTSPOT=1` (the existing instrumentation).
- Peak RSS.

Run each measurement at least three times; compare medians, not best
runs. Reject changes whose wins are inside run-to-run noise.

Do **not** trust the historical numbers in `docs/PERFORMANCE_ANALYSIS.md`
as a baseline. Rerun on current HEAD before claiming any improvement.

## What success looks like

- Patch 1 alone: 20-30% improvement on STL-heavy compiles.
- Patches 1-4 together: 50% improvement, restoring roughly pre-rewrite
  parity.
- Patches 1-8 together: meaningfully *better* than the pre-rewrite
  baseline, because the structured pipeline avoids the reparse cost
  that was the previous bottleneck while the dual-representation
  overhead is gone.

The structured-AST rewrite is sound. The performance regression is the
cost of an incomplete migration. Finishing the migration recovers the
performance and keeps the correctness gains.
