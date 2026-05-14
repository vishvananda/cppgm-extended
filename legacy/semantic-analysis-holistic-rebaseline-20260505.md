# Semantic Analysis Holistic Rebaseline - 2026-05-05

## Goal

Recent optimization slices have produced small or noisy wins. This rebaseline
steps back from individual cache experiments and asks where the compile spends
time and memory at phase scale. The target benchmark is still
`benchmarks/self_compile/stable/semantic_overload.cpp`, because it is large
enough to show the STL-heavy behavior without being as expensive as
`callsemantic.cpp`.

## Measurement Commands

Timing, counters, and memory census:

```sh
CPPGM_SEMANTIC_PHASE_STATS=1 scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --counters \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --env CPPGM_MEMORY_CENSUS=1 \
  --output-prefix /tmp/cppgm-perf-holistic-baseline-20260505
```

Startup sample, killed after sampling the declaration collection phase:

```sh
CPPGM_SEMANTIC_PHASE_STATS=1 \
CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
dev/cppgm++ -I dev/src -c -o /tmp/cppgm-holistic-sample-collect-20260505.o \
  benchmarks/self_compile/stable/semantic_overload.cpp
```

The live process was sampled with `sample <pid> 12`.

## Phase Timing

Single-run wall time was `107.024s`; peak RSS was `1,330,512 KB`.

Major semantic phases:

| Phase | Time |
| --- | ---: |
| `semantic.collect_declarations` | `23.042s` |
| `semantic.output_seed` | `18.505s` |
| `semantic.fixpoint.instantiated_template_output` total | `17.617s` |
| `semantic.fixpoint.late_required_synthesized_output` total | `18.670s` |
| `semantic.fixpoint.synthetic_function_output` total | `4.940s` |
| `semantic.fixpoint.late_required_function_output` total | `0.716s` |
| `semantic.validate_required_definitions` | `0.354s` |

LowIR collection after semantic analysis is smaller, but not zero:

| Phase | Time |
| --- | ---: |
| `lowir.collect.parameter_virtual_base_layouts` | `4.672s` |
| `lowir.collect.scope` | `0.886s` |
| `lowir.collect.symbols` | `0.680s` |

Interpretation: the largest semantic blocks are declaration collection, output
seeding, and the first two output fixpoint passes. The last several cache
experiments were aimed at leaf costs inside these phases; they did not change
which phases dominate.

## Memory Census

The semantic memory census accounts for `594,843,268` bytes out of the
`1.33 GB` process RSS. The rest is allocator overhead, parsed AST/source data,
library/runtime overhead, and uncensused transient data.

Top semantic categories:

| Category | Count | Bytes |
| --- | ---: | ---: |
| `type` | `280445` | `179,618,728` |
| `callsem.output` | `169844` | `142,925,804` |
| `scope` | `82832` | `105,256,106` |
| `function_binding` | `33953` | `52,964,208` |
| `template_resolution.resolve_template_arguments_cache` | `41216` | `42,445,748` |
| `function_template` | `12858` | `32,867,934` |
| `class_info` | `10990` | `26,660,014` |
| `cppast.synthetic` | `34090` | `19,020,894` |

`callsem.output` details:

| Detail | Bytes |
| --- | ---: |
| children storage | `50,420,440` |
| inline node storage | `47,556,320` |
| extra inline fields | `11,573,760` |
| symbol payload | `9,989,286` |
| resolved-name string capacity | `9,467,848` |

Template-resolution cache details:

| Detail | Bytes |
| --- | ---: |
| key parameter vector storage | `10,628,400` |
| entry argument vector storage | `10,274,040` |
| map nodes | `5,935,104` |
| entry argument text capacity | `4,389,092` |
| entry expression AST | `4,107,998` |

Interpretation: data-size work still matters, but the memory profile does not
point to a single accidental 3 GB-style regression. The largest memory buckets
are exactly the semantic model objects and output nodes. Reducing the number of
classes/functions/bodies analyzed should beat making one cache representation
slightly cheaper.

## Sampled Hidden Time Sinks

### 1. Eager class population during declaration collection

The startup sample landed inside `semantic.collect_declarations`. The hot path
was:

```text
collect_top_level_declarations
  collect_declaration
    collect_namespace_definition
      collect_class_declaration
        populate_class_info
          collect_class_simple_declaration
            maybe_complete_class_member_object_type
              complete_class_type
                populate_class_info
```

In that 12-second startup sample, declaration collection was not just
registering names. It was recursively completing class member object types and
populating nested classes. This explains why a source file that merely includes
STL headers pays heavily before output reachability is known.

The current API makes this hard to avoid because `populate_class_info` mixes:

- member declaration registration,
- field type validation,
- recursive member object completion,
- implicit special member synthesis,
- virtual/final layout,
- complete layout finalization.

This is the best candidate for a large algorithmic win.

### 2. Output passes still drive body semantic analysis

The main and late samples both spent most time under:

```text
semantic_output::analyze_instantiated_template_output
  analyze_function_binding_output_impl
    semantic_statement::analyze_statement
      semantic_expression::analyze_expression
        semantic_overload::analyze_call_expression
```

This means output scheduling is still the driver for a large amount of semantic
analysis. The fixpoint totals show `instantiated_template_output` and
`late_required_synthesized_output` together costing over `36s`.

This is the second large algorithmic target: move from broad output scans to
explicit dirty queues and output roots, so newly reachable work is analyzed once
because a dependency requires it.

### 3. Function-template deduction local-type overlay

One sampled hot subpath in function-template deduction was:

```text
deduce_function_template_arguments
  overlay_instantiation_local_named_types
    bind_argument_local_named_types
      collect_argument_local_named_types
        describe_named_type_metadata
```

`describe_named_type_metadata` copies rich class metadata, including field/base
vectors, method-derived flags, and instantiation argument vectors. The local
type overlay code only needs a much smaller view: class name, type,
enclosing-function/local marker, and instantiation type arguments to recurse.

This is a good small-to-medium cleanup. It should reduce allocation and copy
pressure in a sampled deduction path, but it is not likely to be the largest
wall-time win by itself.

### 4. Inline-namespace function entity dedupe

The late sample showed repeated work in:

```text
lookup_direct_functions
  same_inline_namespace_function_entity
    same_inline_namespace_function_template_entity
      inline_namespace_collapsed_scope_name
      canonical_function_lookup_name
      same_function_template_entity_type_impl
```

This path allocates temporary strings while deduping lookup results. It is a
real hidden sink in output/body analysis, especially through libc++ inline
namespace lookup. Caching collapsed scope names and canonical lookup names, or
using pointer/typed identity before building strings, should be safe if done
carefully. This is smaller than class demand-splitting, but more concrete than
another broad resolver-cache experiment.

## Recommended Direction

### Phase 1: Add demand-level class collection instrumentation

Do not rewrite class completion blindly. First add counters/timers that split
class work into explicit reasons:

- top-level declaration collection,
- nested class declaration collection,
- field object type validation,
- base class collection,
- implicit special member synthesis,
- virtual/final layout,
- output-forced completion,
- expression/member lookup completion,
- type trait/lifetime completion.

The immediate target is to quantify how much of the `23s` declaration collection
phase is field-object recursive completion and how much is unavoidable name
registration.

### Phase 2: Split class population into reference shape and full completion

Introduce an explicit class demand model:

```cpp
enum class ClassDemand {
  Identity,
  ReferenceMembers,
  MemberLookup,
  Layout,
  Lifetime,
  Output
};
```

Then split `populate_class_info` into stages:

- reference shape: bases as unresolved references, nested names, member function
  signatures, aliases, static members, and enough member-scope data for lookup;
- full object model: field object completion, implicit special members,
  virtual/final layout, and layout finalization.

`collect_class_declaration` should normally build reference shape, not full
layout. Full completion should happen only when layout/lifetime/output requires
it. This is the most likely large win because it removes entire recursive
subtrees of work before they happen.

### Phase 3: Replace broad output scans with demand queues

The existing fixpoint should become dirty queues keyed by the semantic object
that became reachable:

- instantiated function bodies,
- instantiated class layouts/output,
- synthesized special members,
- required definitions,
- callee closure items.

The output seed pass should enqueue roots found during declaration collection
instead of recursively walking every top-level AST declaration. This will make
the output path match the new class demand model.

### Phase 4: Clean up measured small sinks

After the demand split is underway, land the smaller improvements that the
samples exposed:

- replace `collect_argument_local_named_types` full metadata copies with a
  lightweight `ClassInfo` view;
- cache or intern inline-namespace collapsed scope names;
- avoid canonical-function-name temporary strings where the stored map key is
  already canonical;
- revisit the template-resolution cache only after the earlier phases reduce
  duplicate asks, because the current cache already costs `42 MB`.

### Phase 5: Validate with phase and instruction counters

Each phase should be validated with:

- strict semantic LowIR compare for `pa18 pa19 pa21 pa22`;
- `self-semantic-overload` phase timings and memory census;
- a PA33/PA34 STL smoke run;
- an instruction-count sample when wall time is noisy.

Success should be measured primarily by phase-level reductions:

- `semantic.collect_declarations` should drop if class reference shape works;
- `semantic.output_seed` should drop if root queuing works;
- `instantiated_template_output` and `late_required_synthesized_output` should
  drop if dirty queues work;
- memory should drop if fewer `Type`, `Scope`, `FunctionBinding`, and
  `CallSemNode` objects are constructed.

## Current Conclusion

The largest likely win is not another cache lookup tweak. The samples show that
the compiler is doing full class population/completion during declaration
collection and then using output scheduling to drive large repeated body
analysis. The next optimization project should make semantic demand explicit,
starting with class reference-shape vs full-completion splitting, then replace
broad output scans with queues. Cache representation and copy reduction remain
useful, but they should be follow-up work behind the demand split unless new
measurements contradict this profile.

## Phase 1 Demand Instrumentation Result

After adding demand counters, the same benchmark completed in `103.154s` with
`1,346,532 KB` peak RSS. Timing is not comparable to a no-counter run, but the
demand split is useful:

| Demand | Class Info | Complete Class | Populate Class |
| --- | ---: | ---: | ---: |
| collect declarations | `849,249` | `4,863` | `747` |
| output seed | `1,103,557` | `79,496` | `297` |
| instantiated template output | `918,735` | `35,783` | `648` |
| late required synthesized output | `1,794,853` | `49,396` | `515` |
| field member object | `1,319,568` | `14,111` | `1,734` |
| class layout | `59,310` | `11,455` | `0` |

Interpretation: the sample was representative. Field member-object completion
is the largest direct class-population driver, and top-level declaration
collection still performs hundreds of full class populations before output
reachability is known. The first behavior-changing patch should therefore split
class collection into a reference-shape pass and a full object-model completion
pass, then make field-object completion request only the full demand when
layout is actually required.

## Phase 2 Reference-Shape Declaration Result

The first behavior-changing slice changed non-function-local class declarations
to collect reference shape first and reset that partial state when a later full
completion is required. Function-local classes stay on the full path because
template witness identity and local type numbering are sensitive to the old
ordering.

This exposed a separate identity bug in `create_class_info`: if a later
declaration reached the same canonical type key through a scope where the direct
name binding was missing, it could create a duplicate `ClassInfo` and overwrite
the registry entry. The fix recovers the existing registry entry by type key for
non-function scopes and binds it into the current scope instead of creating a
duplicate. The concrete failing case was `std::__1::locale::facet`, where a
later forward declaration could hide the already-seen out-of-class definition.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `99.006s`, `1,331,588 KB`

Single-run benchmark deltas against the Phase 1 demand-instrumented baseline:

| Metric | Phase 1 | Phase 2 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `103.154s` | `99.006s` | `-4.0%` |
| peak RSS | `1,346,532 KB` | `1,331,588 KB` | `-1.1%` |
| collect-declarations populate | `747` | `569` | `-23.8%` |
| field-member-object populate | `1,734` | `1,532` | `-11.6%` |
| total class-info-for-type calls | `7,302,893` | `7,307,975` | neutral |
| total complete-class-type calls | `68,801` | `69,050` | neutral |

Interpretation: the patch proves the demand split can remove eager class
population during declaration collection, but this first conservative slice is
not yet a large win. Most wall time remains in output seed and output fixpoint
passes, and many classes still reach full completion later. The next large
algorithmic target remains output demand queues; class-demand splitting should
continue only where a caller can avoid a later full completion entirely.

## Phase 3 Late Required Function Queue Result

The first output-queue slice changed late required free-function output from
"scan all required definitions every fixpoint pass" to "process new entries,
retry only unresolved entries." This brings it in line with the existing
instantiated-template and late-synthesized retry queues without changing the
emission rules.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `99.875s`, `1,363,300 KB`

Single-run benchmark deltas against Phase 2:

| Metric | Phase 2 | Phase 3 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `99.006s` | `99.875s` | noisy |
| peak RSS | `1,331,588 KB` | `1,363,300 KB` | noisy |
| late required function output scans | `28,587` | `6,733` | `-76.4%` |
| late required function output emits | `131` | `131` | unchanged |

Interpretation: this removes a redundant rescan and makes the output fixpoint
more explicit, but the phase was already small. It is a correctness-preserving
cleanup and a useful queueing pattern, not a major wall-time win. The remaining
large output costs are still `output_seed`, `instantiated_template_output`, and
`late_required_synthesized_output`.

## Phase 4 Inline Namespace Entity Comparison Result

The inline-namespace function/template dedupe path previously compared entity
scope identity by constructing collapsed scope-name strings. The patch compares
the same collapsed scope components directly by walking parent scopes and
ignoring inline namespace components. Function entity comparison still uses the
existing canonical function-name rules for the base name.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `96.728s`, `1,351,620 KB`

Single-run benchmark deltas against Phase 3:

| Metric | Phase 3 | Phase 4 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `99.875s` | `96.728s` | noisy, favorable |
| peak RSS | `1,363,300 KB` | `1,351,620 KB` | `-0.9%` |
| output seed phase | `32.193s` | `31.119s` | `-3.3%` |
| instantiated template output total | `15.767s` | `14.841s` | `-5.9%` |
| late required synthesized output total | `16.558s` | `15.764s` | `-4.8%` |

Interpretation: the single-run wall-time and phase totals moved in the right
direction, but this is still a local string-allocation cleanup rather than a
dominant algorithmic change. It is worth keeping because it removes temporary
scope-name construction from a sampled hot equality path without adding a cache
or changing the semantic comparison.

## Phase 5 Local-Type Overlay Metadata Result

The template instantiation local-type overlay traversal previously called
`describe_named_type_metadata` for each named type it visited. That API builds a
rich class metadata view: bases, fields, constructor/destructor flags, method
derived facts, and copied instantiation argument vectors. The overlay traversal
only needs class name, class type, enclosing-function status, and instantiation
type arguments for recursive local-type discovery.

The patch reads the `ClassInfo` directly from the existing template semantic
model view in this one traversal and avoids constructing the heavy metadata
object.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `96.716s`, `1,352,176 KB`

Single-run benchmark deltas against Phase 4:

| Metric | Phase 4 | Phase 5 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `96.728s` | `96.716s` | neutral |
| peak RSS | `1,351,620 KB` | `1,352,176 KB` | neutral |
| output seed phase | `31.119s` | `30.483s` | noisy, favorable |
| instantiated template output total | `14.841s` | `15.109s` | noisy |
| late required synthesized output total | `15.764s` | `15.862s` | noisy |

Interpretation: this did not move the whole benchmark measurably, but it
removes unnecessary metadata copying from a sampled path and makes the data need
explicit. Keep it as a cleanup; it should not be treated as a major performance
lever.

## Phase 6 Standard-Include Class Output Deferral Result

An output-seed sample showed the largest remaining seed cost was class output
forcing full class completion:

```text
analyze_declaration_output_impl
  analyze_class_output_from_info_impl
    complete_class_type
      populate_class_info
        maybe_complete_class_member_object_type
```

The first broad prototype deferred all incomplete classes with no required
member/static-member output. That was too broad: it preserved strict after
adding witness and hidden-friend exclusions, but the self benchmark later failed
in LowIR because a source-defined class layout had never been completed. The
kept version limits the cutoff to standard-include class definitions only, keeps
witness capture on the old path, and keeps immediate hidden friend definitions
on the old path so source-order function output is preserved.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `95.678s`, `1,359,892 KB`

Single-run benchmark deltas against Phase 5:

| Metric | Phase 5 | Phase 6 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `96.716s` | `95.678s` | noisy, favorable |
| peak RSS | `1,352,176 KB` | `1,359,892 KB` | noisy |
| output seed phase | `30.483s` | `30.041s` | `-1.5%` |
| output-seed populate | `671` | `496` | `-26.1%` |
| complete-class-type calls | `69,051` | `67,549` | `-2.2%` |
| complete-class-type materializations | `1,722` | `1,570` | `-8.8%` |
| class-layout complete calls | `11,460` | `6,126` | `-46.5%` |

Interpretation: this is the first cutoff in this pass that removes a meaningful
amount of class-completion work, but source-defined classes still rely on seed
completion as a layout backstop. A larger version needs a typed layout-demand
queue or a LowIR-side guarantee that every used source class has been completed
before lowering asks for its size.

## Final Status Of This Pass

Kept changes:

- Demand instrumentation for class completion sources.
- Conservative reference-shape collection for non-function-local classes.
- Late required free-function output queueing.
- Structural inline-namespace entity comparison.
- Lightweight local-type overlay traversal.
- Standard-include-only class output deferral.

The most meaningful algorithmic reductions were:

- class declaration reference shape: fewer eager declaration-time populations;
- late required output queueing: `28,587 -> 6,733` scans for the same `131`
  emits;
- standard-include class deferral: `1,722 -> 1,570` total complete-class
  materializations and `11,460 -> 6,126` class-layout complete calls on the
  measured run.

The cleanups that reduced hidden allocation/copy pressure but did not change
request counts materially were:

- structural inline namespace comparison;
- local-type overlay direct `ClassInfo` lookup.

Remaining large lever: source-defined classes still need a proper layout-demand
model. The failed broad prototype showed that simply deferring every unused
class can leave LowIR without a size for source classes. The next safe version
should make layout demand explicit, probably by completing every class whose
type reaches object layout/lifetime/lowering, rather than relying on seed
output as the implicit layout backstop.

## Phase 7 Output Layout Completion Result

The next slice made the class-output cutoff broad again, but added an explicit
`semantic.complete_output_layout_types` phase after output fixpoint validation.
The phase walks the emitted `CallSemNode` tree and completes layouts only for
class value/ABI types that actually reached output. It skips `type-alias` nodes
and pointer/reference-only appearances, so it is tied to LowIR's real layout
need rather than to declaration order.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `94.810s`, `1,350,912 KB`

Single-run benchmark deltas against Phase 6:

| Metric | Phase 6 | Phase 7 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `95.678s` | `94.810s` | noisy, favorable |
| peak RSS | `1,359,892 KB` | `1,350,912 KB` | `-0.7%` |
| output seed phase | `30.041s` | `26.188s` | `-12.8%` |
| explicit output layout phase | none | `2.427s` | new cost |
| output-seed output appends | `2,089` | `1,747` | `-16.4%` |
| output-seed state changes | `27` | `13` | `-51.9%` |
| total complete-class-type calls | `67,549` | `65,744` | `-2.7%` |
| total complete-class materializations | `1,570` | `1,673` | `+6.6%` |
| field-member-object populate | `1,532` | `1,222` | `-20.2%` |
| output-layout-completion populate | none | `358` | new explicit demand |

Interpretation: the phase made the broad cutoff correct, and it moved a few
seconds out of output seed into an explicit layout-demand pass. The net gain is
still too small because the remaining output seed path is not just eager layout
completion. The next step is to resample after this patch and target the new
largest path inside `semantic.output_seed`.

## Phase 8 Lazy Reference Member Aliases

The post-Phase 7 sample showed output seed dominated by source function body
analysis, with repeated class/template work under member calls. A first
experiment tried using reference-member lookup instead of full class completion
for member-function calls. It was reverted: concrete class-template member
bindings often need full collection before overload selection, so the retry path
preserved correctness but did not reduce complete-class counts.

The kept slice makes explicitly dependent member aliases lazy during
reference-member collection. Reference collection now binds the alias name to a
dependent placeholder and stores the alias AST on `ClassInfo`; typed member
lookup resolves the deferred alias only if the alias is actually requested. Full
class completion still parses aliases eagerly, and witness source capture stays
on the old path.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `94.573s`, `1,333,952 KB`

Single-run benchmark deltas against Phase 7:

| Metric | Phase 7 | Phase 8 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `94.810s` | `94.573s` | noisy, slight |
| peak RSS | `1,350,912 KB` | `1,333,952 KB` | `-1.3%` |
| collect declarations | `8.234s` | `7.803s` | `-5.2%` |
| output seed | `26.188s` | `25.798s` | `-1.5%` |
| resolve-template-argument calls | `287,379` | `286,207` | `-0.4%` |
| class-template specialization-name builds | `116,538` | `112,478` | `-3.5%` |

Interpretation: this is a correctness-preserving demand split and a small memory
win, but it is not the large `__trivially_relocatable` reduction. The top
hotspot remains repeated `resolve_template_arguments` for libc++ traits, mostly
outside the deferred alias path. The next larger target should focus on the
trait/alias resolution callers themselves or on reducing repeated source body
analysis in output seed.

## Phase 9 Pre-Expansion Bound-Member Failure Cutoff

The top remaining trait path repeatedly resolved template arguments shaped like
`T, typename T::member`. The old fast path expanded template-argument inputs,
built a bound scope, bound the earlier `T`, and only then discovered that the
later member type lookup was a stable failure for the concrete owner type.

The new cutoff recognizes that general structured shape before expansion. It
fires only outside witness/trace mode, only when the member argument is
structured `typename Owner::member`, and only when `Owner` names an earlier
non-pack type parameter whose concrete argument is already known in the use
scope. The same stable member-failure cache is still the authority for whether
the substitution failure can be reused.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `92.396s`, `1,329,440 KB`

Single-run benchmark deltas against Phase 8:

| Metric | Phase 8 | Phase 9 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `94.573s` | `92.396s` | `-2.3%` |
| peak RSS | `1,333,952 KB` | `1,329,440 KB` | `-0.3%` |
| top `__trivially_relocatable` full query count | `60,900` | `35,100` | `-42.4%` |
| pre-expansion bound-member failures | none | `25,800` | new cutoff |
| simple-fast bound-member failures | `25,800` | `0` | moved earlier |
| expanded template-argument texts | `486,544` | `434,944` | `-10.6%` |

Interpretation: this is the first trait-path cutoff that clearly removes a
large repeated operation count. It does not reduce total
`resolve_template_arguments` calls because those calls are still counted at
entry, but it avoids the expensive expansion and bound-scope path for the
stable failures. The remaining `__trivially_relocatable` queries are not this
negative substitution shape and need a separate typed result/selection analysis.

## Phase 10 Pre-Expansion Simple Type Argument Resolution

After Phase 9, many high-count queries were still fully explicit type-only
argument lists, such as `_Tp`, `_Bp, _Tp`, and `_CharT, _Traits, _Tp`. These
were simple enough for the existing fast path, but only after
`expand_template_argument_inputs` had already run.

The next slice resolves fully explicit, non-pack type-parameter lists before
expansion when each argument is already available as structured syntax, a direct
bound type, a visible named type, a previous parameter binding, or a structured
`typename Previous::member` lookup. It stays disabled under witness source
capture and template-resolution tracing.

Validation:

- strict `pa18 pa19 pa21 pa22`: passed
- `self-semantic-overload`: `ok`, `92.133s`, `1,323,184 KB`

Single-run benchmark deltas against Phase 9:

| Metric | Phase 9 | Phase 10 | Delta |
| --- | ---: | ---: | ---: |
| wall time | `92.396s` | `92.133s` | noisy, slight |
| peak RSS | `1,329,440 KB` | `1,323,184 KB` | `-0.5%` |
| pre-expansion simple type successes | none | `92,205` | new cutoff |
| single-bound fast hits | `56,821` | `0` | moved earlier |
| simple-fast successes | `53,547` | `24,771` | moved earlier |
| expanded template-argument texts | `434,944` | `299,996` | `-31.0%` |
| resolve-template-argument cache misses | `40,241` | `36,335` | `-9.7%` |

Interpretation: this is a broad front-door cleanup rather than a new semantic
rule. It removes a large amount of argument-expansion work that was only needed
to rediscover simple typed data already present in the semantic scope. The wall
time gain is small on this run, so future work should keep targeting larger
body-analysis and overload-selection sinks.
