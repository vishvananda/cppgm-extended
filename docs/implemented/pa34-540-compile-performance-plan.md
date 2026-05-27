# PA34 540 Compile Performance Plan

## Goal

Use `pa34/tests/compile/540-reference-wrapper-smoke.t` as the primary hosted
compile benchmark and get its wall time back near the old text-based compiler
baseline.

Original qualitative baseline:

- `cppgm++` structured pipeline: roughly minute-scale on a direct strict
  `540` compile after the correctness fix.
- old text-based compiler: about `8s` for the same case.
- Homebrew Clang: under `2s`.

The first target is text-pipeline parity: median `540` compile time at or below
`10s` on the same machine. If the first tranche cannot reach that, every landed
slice must still reduce the median by a clearly measurable amount and explain
which remaining cost dominates.

Current status after the source-location performance tranche:

- Focused benchmark `pa34-reference-wrapper-smoke` now has a three-run median
  of `6.881s` and median RSS `206476 KB`.
- The source-token witness lookup now builds one reusable identifier/location
  index instead of repeatedly scanning the full token stream.
- Source location insertion now only coalesces adjacent duplicate locations and
  formats location text with direct string appends instead of maintaining a
  global string-keyed location map and using stream formatting.
- The earlier algorithmic fix made template-argument fragment child parsers
  borrow the parent parser's template-parameter lookup state instead of copying
  large inherited lookup stacks for every nested fragment parse.
- Later experiments around class-completion fast paths, whitespace location
  suppression, semantic-output symbol gates, and simple type-id shortcuts are
  intentionally not part of this baseline tranche.
- `cppgm++ -E` was verified to work in this worktree; it emitted an 8.3 MB
  preprocessed `540` file with exit code `0`, so it remains a valid phase-cut
  probe.
- Focused strict validation is clean for `pa18 pa19 pa21 pa22` with strict
  semantic fallbacks enabled.

## Principles

- Prefer algorithmic fixes over caches.
- Prefer explicit decisions over fallback probing.
- Prefer deferred work over eager processing.
- Prefer views, handles, and immutable source nodes over deep copies.
- Produce strings only for diagnostics, witness output, or final textual
  output, not for internal identity and routing.
- Keep correctness gates focused on the owning strict PA suites; do not use the
  full PA34/PA35 report as the normal iteration loop.

Caches are allowed only after the underlying repeated work is understood and
reduced. A cache that hides "try everything and recover" behavior is not an
acceptable first fix.

## Benchmark Contract

Add `540` to the structured-AST benchmark runner as a named focused benchmark:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --benchmark pa34-reference-wrapper-smoke \
  --output-prefix /tmp/cppgm-pa34-540-baseline
```

The direct command behind that benchmark should be equivalent to:

```sh
/usr/bin/time -lp \
  env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
  ./dev/cppgm++ -c \
    -o /tmp/cppgm-pa34-540.o \
    pa34/tests/compile/540-reference-wrapper-smoke.t
```

Also collect controls:

```sh
/usr/bin/time -lp \
  /usr/local/opt/llvm/bin/clang++ -std=gnu++11 -stdlib=libc++ -c \
    -o /tmp/clang-pa34-540.o \
    pa34/tests/compile/540-reference-wrapper-smoke.t
```

Run phase cuts to localize the cost:

```sh
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 ./dev/cppgm++ -E -o /tmp/540.ii pa34/tests/compile/540-reference-wrapper-smoke.t
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 ./dev/cppgm++ --emit-ast -o /tmp/540.ast pa34/tests/compile/540-reference-wrapper-smoke.t
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 ./dev/cppgm++ --emit-semantics -o /tmp/540.sem pa34/tests/compile/540-reference-wrapper-smoke.t
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 ./dev/cppgm++ --emit-lowir -O0 -o /tmp/540.lowir pa34/tests/compile/540-reference-wrapper-smoke.t
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 ./dev/cppgm++ -c -o /tmp/540.o pa34/tests/compile/540-reference-wrapper-smoke.t
```

Interpretation:

- `-E` slow means host include/preprocessor cost.
- `--emit-ast` slow means parser speculation, token slicing, AST allocation, or
  AST copying.
- `--emit-semantics` slow means declaration collection, template lookup,
  class-template instantiation, body checking, or semantic closure.
- `--emit-lowir` / `-c` slow only after semantics means output closure, LowIR,
  object generation, or symbol emission.

## Required Instrumentation

Collect one baseline run with existing instrumentation:

```sh
env \
  CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
  CPPGM_FILE_TIMING=1 \
  CPPGM_FILE_TIMING_LIMIT=80 \
  CPPGM_SEMANTIC_HOTSPOT=1 \
  CPPGM_SEMANTIC_STATS=1 \
  CPPGM_SEMANTIC_CACHE_STATS=1 \
  CPPGM_MEMORY_CENSUS=1 \
  ./dev/cppgm++ -c \
    -o /tmp/cppgm-pa34-540.o \
    pa34/tests/compile/540-reference-wrapper-smoke.t \
  2>/tmp/cppgm-pa34-540-hotspot.log
```

Add targeted counters before changing algorithms if the existing output does
not explain the cost. The counters should be cheap and should answer:

- how many class-template instantiations are requested, created, reset, and
  completed
- how many class completions request only name, shape, layout, member
  declarations, member bodies, or output
- how many function bodies are parsed, body-checked, semantically analyzed, and
  output-emitted
- how many template argument resolutions are performed from structured syntax
  versus text
- how many `CppAstNode` and `TemplateArgumentSyntax` deep clones happen, with
  approximate child-count / text-byte totals
- how many output-closure fixpoint passes run, and how many classes/functions
  are rescanned per pass

Sampling is useful once the counter shape is known:

```sh
sample <cppgm-pid> 10 1 -file /tmp/cppgm-pa34-540.sample.txt
```

or with `xctrace` if a longer profiler capture is needed.

## Working Hypotheses

### 1. Eager Semantic Closure

`540` should need a very small amount of executable code. If we are analyzing
or emitting large sections of `functional`, `tuple`, `type_traits`, or
allocator machinery, the compiler is doing work for declarations that are only
needed as types.

Expected fix direction:

- introduce explicit completion requests: `NameOnly`, `DeclarationShape`,
  `Layout`, `MemberDeclarations`, `MemberBodies`, `Output`
- make callers request the narrowest completion level
- do not analyze member bodies when class layout, `sizeof`, `alignof`, or type
  trait evaluation only needs declarations and layout
- do not enter output closure for functions/classes without a required output
  edge

### 2. Class Template Instantiation Does Too Much

Hosted headers create many class-template references where only a type identity,
base/member shape, or layout is needed. Current paths often populate full
`ClassInfo`, collect methods, synthesize special members, check bodies, and then
later discover that no output is required.

Expected fix direction:

- split class-template instantiation into shape and body/output phases
- make type traits and `alignof` complete only layout, not member bodies
- defer implicit special-member synthesis until construction, destruction,
  assignment, or output actually requires it
- avoid resetting and repopulating an instantiation unless the selected source
  node or template argument identity truly changed

### 3. Template Body Checks Are Too Eager

The structured cleanup added more typed checks in template bodies. Correctness
needs those checks when a body is instantiated or when an owner PA explicitly
requires body validation, but hosted compile throughput suffers if every member
template body in included headers is fully walked just because its enclosing
class was referenced.

Expected fix direction:

- classify body checks as immediate-context checks versus instantiation-time
  checks
- collect cheap declaration facts up front, but defer expression-level body
  checking until the function body is selected for instantiation/output
- keep strict fallback assertions active in the deferred path so deferral does
  not reintroduce text fallback

### 4. Structured Syntax Is Being Deep-Copied

The structured pipeline fixed correctness by carrying AST and template syntax
through more paths. The likely cost is clone-on-read: `CppAstNode`,
`TemplateIdSyntax`, and `TemplateArgumentSyntax` are copied while expanding
packs, preserving source arguments, creating dependent type metadata, and
passing requests across template/semantic boundaries.

Expected fix direction:

- add clone counters around `clone_expression_node_for_template_substitution`,
  `clone_argument_syntax_for_template_substitution`, and request construction
- pass immutable syntax by pointer/reference where lifetime is already tied to
  the parsed translation unit or durable template declaration
- represent substitutions as overlays when possible instead of materializing a
  copied AST
- materialize a copied syntax node only when a consumer mutates it, such as
  pack expansion that truly changes the argument list
- move local parser nodes into owners on successful parse paths; never copy
  whole subtrees just to append them

### 5. Text And Structured Identity Still Coexist

Some hot paths still preserve structured data and also build canonical text for
the same entity. On `540`, the cost may be dominated by repeated template
argument text construction, qualified-name text normalization, and diagnostic
string assembly.

Expected fix direction:

- route template selection and instantiation identity through structured
  `TemplateArgument` / `TypePtr` / declaration handles
- make diagnostic text lazy
- remove text from internal keys when a structured key is available
- ensure witness-only source-location text is not recovered unless witness
  capture is active

Residual cleanup targets found during the May 2026 performance pass:

- `tokenized_text_for` and the standalone semantic fragment parser were removed
  after coverage showed no active callers; `parse_type_text` and
  `parse_type_text_scoped` remain as the remaining text-to-type reparsing paths
- template argument rewriting still has direct text walkers in
  `rewrite_bound_type_names_in_text` and `rewrite_bound_type_packs_in_text`
- identifier-token collection still scans text fragments for template
  placeholder and dependent binding checks, and some template-argument paths
  call `collect_identifier_tokens` directly instead of going through the
  semantic cache
- normalized textual type lookup still flows through
  `normalize_type_lookup_name`, especially in template argument semantics and
  semantic lookup
- selected semantic decisions still call `node_text`, especially around
  constant-initializer checks, friend declarations, and template parameter/type
  text extraction

The cleanup direction is to replace these with typed or interned identities:
store stable text atoms when text remains unavoidable, pass structured
`TypePtr` / `TemplateArgument` / declaration handles where available, and make
diagnostic/witness strings lazy.

### 6. Decision Logic Still Falls Back Through Multiple Strategies

Even if strict fallback disables text recovery, the compiler may still try
several expensive structured strategies before choosing the right one.

Expected fix direction:

- make syntax kind and semantic request kind choose the lookup path directly
- replace "try class, try alias, try namespace, try member" chains with a small
  decision table
- add counters for failed strategy attempts, not just successful queries

### 7. Output Closure Rescans Instead Of Using Worklists

Existing code has both worklist-style closure and older fixpoint-style class
method rescans. `540` is small enough that any repeated scan over all hosted
classes/functions is likely waste.

Expected fix direction:

- replace class-method emit fixpoint scans with an indexed worklist
- enqueue only newly required functions/classes
- make late-required synthesized work explicit edges rather than "scan again"
  state

## Implementation Order

### Branch Notes

- Started branch `codex/pa34-540-compile-performance-20260430` from
  integration commit `5981fc5b`.
- The `~/cppgm` performance branch is based on an older, more text-centric
  compiler. Use its semantic/template work as strategy input rather than a
  patch source. Prefer direct ports only for layers that still match here, such
  as token/source-location plumbing, benchmark tooling, and low-level
  allocation/string reductions.
- Tested a portable `PostTokenizer` source-location fast path from the older
  branch. It was not kept: clean baseline median `7.787s`, patched median
  `7.796s` on `pa34-reference-wrapper-smoke`.
- Because this machine has noisy background work, compare future slices with
  `scripts/run_ab_compile_benchmark.py` using backed-up compiler binaries and
  alternating `baseline/candidate/candidate/baseline` pairs.
- Lazy-output experiment: make broad class output scans incremental and rely on
  the late-required owner-class function queue for requirements added after a
  class has already been scanned.

### Stage 0: Freeze Measurement

- Status: complete for the first tranche.
- Add `pa34-reference-wrapper-smoke` to
  `scripts/run_structured_ast_perf_benchmarks.py`.
- Record three-run medians for `540`, Clang `540`, and the existing focused
  PA34 perf controls.
- Record one instrumented `540` hotspot/census log.

Exit criteria:

- a checked-in benchmark entry exists
- `/tmp` baseline JSON/log paths are recorded in this plan or progress notes
- the slowest phase is identified

### Stage 1: Account For Eager Work

- Status: complete enough for the first tranche.
- Add cheap semantic counters for completion request kinds, template
  instantiation lifecycle, body checks, output closure passes, and syntax clone
  volume.
- Do not optimize yet unless the counter insertion exposes an obvious one-line
  bug.

Exit criteria:

- we can say exactly whether `540` is dominated by class completion, body
  checking, output closure, syntax cloning, text construction, or backend work

### Stage 2: Narrow Class Completion Requests

- Introduce or formalize completion request kinds.
- Convert `sizeof`, `alignof`, type traits, base lookup, and member type lookup
  callers to request only the required level.
- Defer method body checks and implicit special-member synthesis when only
  shape/layout is requested.

Expected win:

- fewer completed classes and fewer body checks on `540`
- less method/special-member output churn

### Stage 3: Defer Template Body Analysis

- Split immediate-context declaration checks from instantiated body analysis.
- Keep cheap member-name/member-type collection.
- Analyze member function bodies only when instantiation/output requires them.

Expected win:

- fewer expression-analysis node visits and constructor-selection calls in
  hosted headers

### Stage 4: Remove Clone-On-Read Syntax Transport

- Status: partially complete for template-argument fragment parsing.
- Replace durable copied syntax in hot template requests with references or
  handles where safe.
- Keep copies only for mutated pack-expanded forms.
- Convert local `push_back(node)` parser paths to `push_back(std::move(node))`
  where ownership transfers.

Expected win:

- lower RSS and lower wall time in parser/template-argument phases

### Stage 5: Convert Output Closure To Worklists

- Replace broad class/function rescans with queues of newly required output
  edges.
- Keep validation pass separate and debug-only if needed.

Expected win:

- output phase cost proportional to required output, not all seen hosted
  declarations

### Stage 6: Collapse Remaining Text Identity

- Remove canonical text from hot identity keys when structured data is present.
- Make diagnostic and witness strings lazy.
- Keep strict semantic fallback enabled in all validation.

Expected win:

- fewer string allocations and comparisons in template-heavy headers

### Stage 7: Add Narrow Caches Only After The Shape Is Clean

Only after the above stages:

- cache pure structured template-argument resolution by declaration handle and
  canonical argument identity
- cache class completion results by requested completion level
- cache expensive negative decisions only when the decision inputs are explicit
  and stable

These caches should be small and explainable. They should not make fallback
strategy probing acceptable.

## Validation

Correctness gates for normal slices:

```sh
env CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_TEXT_TEST_TIMEOUT_SEC=120 \
  make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```

Add targeted owner checks for any earlier PA touched by a slice. Do not run the
full PA34/PA35 report as the routine loop.

Performance gates:

- `pa34-reference-wrapper-smoke` median must improve outside noise
- existing PA34 focused controls must not materially regress
- default structured-AST benchmark set must not regress by more than noise
- counters must move in the direction implied by the change

## Completion Criteria

- `540` median compile time is at or below `10s`, or the remaining gap is
  attributed to a specific, documented subsystem with a follow-up plan.
- The compiler no longer eagerly performs full semantic/body/output work for
  hosted declarations when only type identity, member shape, layout, or trait
  answers are needed.
- Hot syntax transport uses references/handles or moves instead of deep copies
  on read-only paths.
- Any caches added after the algorithmic cleanup are narrow, structured, and do
  not hide fallback probing.

## 2026-04-30 alternating A/B results

The machine was noisy, so candidate binaries were copied to `/tmp` and compared with `scripts/run_ab_compile_benchmark.py` using alternating baseline/candidate pairs. Clean-vs-clean showed roughly a 1% median noise floor.

- `lazy-output`: broad class output scan made incremental and widened late owner-class queue. Result: neutral, median -0.03%, average +0.65%; not kept.
- `scope-global-epoch`: global epoch short-circuit for scope fingerprint caching. First pass looked positive, longer confirmation reversed to median +1.2%, average +1.3%; not kept.
- `complete-class-nonnamed-fastpath`: early non-named return in `complete_class_type`. Result: median +1.7%, average +2.1%; not kept.
- `fast-common-type-aliases`: extra exact type-alias fast paths. Result: median +1.3%, average +1.1%; not kept.
- `diag-frame-inplace`: construct diagnostic frames in-place. First pass looked positive, longer confirmation reversed to median +0.45%, average +1.43%; not kept.
- `tokenizer-nonlower-ident`: skip keyword map for non-lowercase identifier starts. Result: median +3.9%, average +1.5%; not kept.

Current conclusion: the obvious micro-optimizations are within or below the machine noise floor and several add branch overhead. The next promising direction is a larger lazy strategy around reducing repeated template argument resolution or output closure rescans with correctness-aware queues, rather than isolated hot-path branches.

## 2026-04-30 larger lazy-output attempts

The next larger attempts focused on making output proportional to required
definitions rather than all hosted definitions seen during declaration
traversal.

- `class-template-reference-cache`: cached class-template reference resolution
  by template declaration, argument identity, and scope identity. This was
  rejected: even after narrowing the key to persistent scopes and pointer
  identity, candidate runs showed large slowdowns and occasional multi-second
  outliers. The cost of key construction and cache management outweighed the
  avoided work in this version of the compiler.
- `lazy-inline-free-output`: kept. Namespace-scope inline free-function
  definitions are no longer emitted eagerly unless definition output was
  required, `emit_all_source_function_definitions` is enabled, or the function
  is synthesized. Longer alternating A/B against integration commit
  `5981fc5b`: baseline median `7.614s`, average `7.605s`; candidate median
  `7.500s`, average `7.581s`. This is about `1.5%` median faster and `0.3%`
  average faster, which is small but directionally positive under the measured
  noise floor.
- `lazy-value-output`: not kept. A comprehensive required-value-definition
  queue was ported conceptually from the older `~/cppgm` branch, with a safety
  improvement to separate value declaration output from definition output.
  The first confirmation run regressed to baseline median `6.878s`, candidate
  median `6.956s` and baseline average `6.868s`, candidate average `6.978s`
  (`+1.13%` median, `+1.60%` average). A tightened variant that skipped queue
  hits for already-emitted definitions and short-circuited inline/constexpr
  syntax before standard-include path checks was still neutral-to-slower:
  baseline median `7.003s`, candidate median `7.065s`; average `7.041s` vs
  `7.035s`. The value queue machinery was reverted.

Current kept code change: lazy emission suppression for unrequired inline
free-function definitions in `semantic_output.cpp`, plus the alternating A/B
benchmark harness. Larger lazy work should next target function/class output
closure directly, where requirement queues already exist, rather than adding a
new value-definition queue without evidence that values dominate `540`.

## 2026-04-30 follow-up iteration

The next pass tightened the kept lazy-inline output path and sampled current
hotspots.

- `inline-standard-include-short-circuit`: tried avoiding the standard-include
  definition path check when the cheap lazy-inline check already answered the
  emission decision. Rejected: alternating A/B against the previous lazy-inline
  candidate regressed to median `+0.93%`, average `+0.61%`.
- `direct-inline-free-check`: kept. For namespace-scope free functions, the
  lazy-inline decision now checks only the declaration specifiers for `inline`
  or `constexpr` instead of calling the generic `function_binding_is_inline`
  helper, whose class-member and hidden-friend cases are already excluded by
  the surrounding predicate. Longer A/B against the previous lazy-inline
  candidate: baseline median `7.475s`, average `7.475s`; candidate median
  `7.407s`, average `7.430s` (`-0.91%` median, `-0.60%` average).
- `exact-fundamental-conversion-fastpath`: tried a narrow early return in
  `try_argument_conversion` for exact non-reference fundamental conversions,
  motivated by hotspot data showing repeated `const char <- char` conversions.
  Rejected: median `+0.52%`, average `+1.96%`.
- Cumulative final A/B against integration baseline commit `5981fc5b` remained
  noisy: baseline median `7.467s`, average `7.437s`; final candidate median
  `7.427s`, average `7.478s` (`-0.54%` median, `+0.55%` average). Treat this
  as a small/local improvement, not a decisive throughput shift.

Current hotspot sample after the kept lazy-output work:

- `resolve-template-argument-calls=8131`, with `4939` key builds and `4915`
  cache misses.
- `class-template-reference-requests=3103`, with `713` creates and `24` resets.
- `complete-class-type-calls=7083`, mostly no-class or already-complete
  queries; the earlier generic non-class fastpath was measured slower and is
  not kept.
- `try_argument_conversion` still shows repeated trivial conversions, but the
  simple exact-fundamental fastpath was slower, so the next conversion change
  needs a more targeted call-site reduction rather than another branch inside
  the central conversion function.

Next likely productive target: reduce template-argument key construction and
failed cache probes in structured template argument resolution. The older
`~/cppgm` text-based patches are not directly portable here; any cache must
avoid rebuilding the same text key it is trying to save.

## 2026-04-30 source-location prefix index pass

A short `sample` profile of the retained direct-inline candidate showed that
`540` was still spending visible CPU in declaration collection, especially in
`earliest_qualified_use_location_for_prefix`. The old `~/cppgm` work also had
source-location concepts, but the current structured compiler already has a
central qualified-use occurrence collection path, so the portable fix was to
index that data instead of repeatedly scanning it.

Kept changes:

- `qualified-use-prefix-index`: when qualified-use occurrences are first built,
  precompute the earliest source location for the full qualified name and each
  `::`-separated owner prefix. This makes later prefix lookups mostly O(1)
  instead of scanning all occurrences for each new prefix.
- `qualified-use-prefix-fallback-allocation`: the fallback scanner now builds
  `prefix + "::"` once and uses `compare`, instead of allocating the concatenated
  prefix inside the occurrence loop.

Rejected follow-up experiments in this pass:

- `recent-only-template-cache`: removed the unordered_map lookup from template
  argument resolution and relied on the existing 64-entry recent cache. Rejected:
  median `+0.12%`, average `+1.16%` against the direct-inline candidate.
- `prefilter-dependent-rewrite`: added identifier-token prefiltering to
  `rewrite_bound_type_names_preserving_dependent_text`. Rejected from the final
  patch set because the signal was sub-noise: median `-0.21%`, average `+0.73%`.
- `nonnamed-complete-fastpath`: skipped `complete_class_type` diagnostics for
  non-named types in non-instrumented runs. Rejected from the final patch set:
  confirmation median was only `-0.34%` and average was `+0.53%`, and the change
  also reduced diagnostic context on failure paths.
- `single-template-arg-fastcache`: added an O(1) single-argument resolver cache
  before the existing linear recent cache. Rejected: median was flat (`+0.00%`)
  against direct-inline and the extra cache complexity did not buy throughput.

Benchmark results:

- Clean prefix-index candidate vs retained direct-inline baseline:
  `/tmp/cppgm-pa34-540-ab-prefix-index-clean.md`
  - direct-inline median `7.170s`, average `7.224s`
  - prefix-index-clean median `7.036s`, average `7.208s`
  - delta `-1.88%` median, `-0.22%` average
- Final clean candidate vs integration baseline commit `5981fc5b`:
  `/tmp/cppgm-pa34-540-ab-final-prefix-index-vs-integration.md`
  - integration baseline median `7.124s`, average `7.135s`
  - final-prefix-index median `6.963s`, average `6.935s`
  - delta `-2.26%` median, `-2.79%` average

Current kept code changes after this pass:

- lazy suppression for unrequired namespace-scope inline/constexpr free-function
  definitions in `semantic_output.cpp`
- direct declaration-specifier check for that lazy inline/constexpr free-function
  predicate
- qualified-use prefix indexing in `callsemantic.cpp`
- single-allocation fallback prefix matching in
  `callsemantic/source_location_utils.cpp`

Next likely productive target: parser/preprocessor throughput. The sample shows
large time in tokenization and AST parsing before semantic analysis, especially
identifier/whitespace token handling and token buffer movement. Further semantic
caches are likely to be sub-percent unless they remove a sampled allocator-heavy
path.

## Early-layer collapse strategy: token batches and parser syntax facts

The next performance tranche should target duplicated work across the
preprocessor, tokenizer, parser, and early semantic collection layers. The goal
is not to collapse semantic phases into parsing. The goal is to make lower
layers produce durable, cheap syntax facts and contiguous token views so later
layers stop reconstructing the same text and shape information repeatedly.

This should be treated as a multi-patch migration. Individual early patches may
not show a clear `540` benchmark improvement because they add compatibility
plumbing while old callers still use the text-heavy paths. A meaningful
benchmark is expected only after enough high-volume callers are moved to the new
views/facts.

### Constraints

- Preserve parser and semantic layer responsibilities: parsing may expose syntax
  facts, but it must not perform semantic lookup, template substitution,
  overload resolution, or class completion.
- Keep old text APIs during migration so early PA fallout can be isolated and
  fixed incrementally.
- Prefer token spans, immutable views, and bit flags over new strings or copied
  AST fragments.
- Benchmark only after a complete vertical slice removes real callers from the
  old text reconstruction path.
- Expect early PA regressions, especially tests that rely on exact diagnostic
  text, spacing, token boundaries, or template/source witness locations.

### Patch 1: instrumentation and accounting

Add cheap counters around the duplicated work before changing behavior:

- calls and bytes for `node_text`, `spaced_node_text`, and token-span text
  reconstruction
- calls and bytes for template argument text/range extraction
- calls to decl-specifier token scans such as `inline`, `constexpr`, `friend`,
  `static`, and `explicit`
- calls to qualified-name parsing from AST/text
- calls to template-id suffix/range parsing from token streams
- source-location identifier lookup and qualified-prefix lookup counts

This patch should not change behavior. It creates the before/after proof for the
migration and identifies the highest-volume call sites.

### Patch 2: token-span view primitive

Introduce a small immutable token-span abstraction over the existing recognized
or preprocessing token storage:

- token begin/end indices
- compact text materialization only on demand
- identifier/operator lookup helpers that operate on tokens rather than rebuilt
  strings
- stable comparison/hash helpers for common spans

This should initially wrap existing storage and avoid changing token ownership.
The first users should be diagnostics or tracing helpers where behavior is easy
to compare.

### Patch 3: parser syntax summaries on AST nodes

Extend the parser to attach or side-table cheap syntax facts while token locality
is still available. Candidate facts:

- decl-specifier flags: `inline`, `constexpr`, `friend`, `static`, `explicit`,
  `virtual`, `typedef`, `using`, cv/ref qualifiers where applicable
- qualified-name summary: unqualified name span, qualifier spans, full span,
  template-id span if present
- template-id argument ranges: top-level argument token ranges and close-token
  metadata
- declarator summary: pointer/reference/function/array shape and parameter
  clause range
- class/function/member declaration source ranges needed by semantic collection

Keep these summaries syntax-only. They should not mention resolved scopes,
classes, templates, overloads, or types.

### Patch 4: migrate high-volume semantic collection callers

Move declaration collection and template declaration collection off repeated AST
text reconstruction where the syntax summaries are sufficient:

- replace decl-specifier rescans with parser-produced flags
- replace qualified-name reparsing from `node.value` with qualified-name
  summaries
- replace template argument text splitting with argument token ranges where
  callers only need identity, source locations, or later lazy materialization
- use source spans for template/witness location anchoring instead of subtree
  name searches when the parser already knows the name span

This is the first patch likely to affect many PA tests. Keep compatibility
fallbacks for missing summaries and add targeted diagnostics to identify any
summary/parser mismatches.

### Patch 5: batch token production under existing APIs

Once span consumers exist, reduce per-token overhead below the parser:

- add a `fill_many` or chunk API under `RecogTokenBuffer::ensure`
- batch `PPTokenizer` output for a physical file/include into contiguous token
  storage
- let `Preprocessor::TokenCursor::read_file` consume file token batches while
  preserving macro expansion and include semantics
- keep macro-injected tokens compatible with existing single-token paths until a
  later cleanup

The benchmark should start moving only when this patch combines with the prior
span migration. Batching alone may not help if upper layers immediately
materialize strings again.

### Patch 6: remove text-heavy compatibility paths from hot callers

After tests are green for the migrated vertical slice, remove or quarantine old
hot-path fallbacks:

- forbid `node_text` in selected semantic/template hot paths except under
  diagnostics or output rendering
- avoid reconstructing template argument text when a token range plus syntax
  summary is enough
- make expensive text materialization visible in counters so regressions are
  caught early

This patch should produce the first meaningful `540` A/B benchmark for the full
strategy.

### Correctness and PA test strategy

Run early PA suites in narrow batches after each vertical slice, not only after
the final benchmark patch. Expected fallout:

- early parser/token tests may expose spacing or token-boundary differences
- declaration tests may expose missing syntax flags or wrong declarator summaries
- template tests may expose argument range mistakes around nested template-ids,
  packs, default arguments, attributes, and dependent qualified names
- witness/source-location tests may expose anchoring differences where the new
  span-based location is earlier or more precise than the old subtree search

Recommended validation order after each risky patch:

1. parser/token focused PAs that own preprocessing, tokenization, and AST shape
2. declaration/type PAs that exercise declarators and decl specifiers
3. template PAs around nested template-ids, default arguments, and packs
4. PA34 `540` focused benchmark only after the migrated slice is behaviorally
   stable

### Benchmark contract for this tranche

Do not over-interpret the first scaffolding patches. Use `540` A/B only at these
points:

- after Patch 4, to see whether syntax summaries reduce semantic collection cost
- after Patch 5, to see whether batching token production pays once strings are
  no longer immediately rematerialized
- after Patch 6, as the final tranche benchmark against the current
  prefix-index candidate and integration baseline

The expected successful shape is a visible reduction in allocator-heavy sampled
stacks: `node_text`/token-span text materialization, qualified-name rescans,
template argument splitting, `BufferedIterator::Buffer::push_back`, and
per-token `RecogTokenBuffer::ensure`/`peek` overhead.

### 2026-04-30 early-layer batching experiment

Implemented a low-level batching pass under the existing parser API:

- `RecogTokenBuffer` now fills from `IRecogTokenSource::get_many` in 256-token chunks instead of pulling one recognized token per buffer miss.
- `RecogTokenizer` implements `get_many` and preserves the existing `>>` split/pending-token behavior.
- `IPostTokenSource`/`PostTokenizer` now support `get_many`, letting recognized-token batching avoid re-entering `PostTokenizer::get()` for every token.
- `PostTokenizer::capture_current_location()` caches the last source file index, avoiding repeated `SourceLocationTable::add_file()` lookups for every token in the same file.

Measured artifacts:

- Baseline before early-layer batching: `/tmp/cppgm-pa34-540-early-layer-baseline-cppgm++`
- Recognized-token batch candidate: `/tmp/cppgm-pa34-540-recog-token-batch-cppgm++`
- Post-token batch/source-location-cache candidate: `/tmp/cppgm-pa34-540-post-token-batch-cppgm++`
- Final kept binary after reverting scratch reuse: `/tmp/cppgm-pa34-540-final-early-layer-batch-cppgm++`

Results on `pa34/tests/compile/540-reference-wrapper-smoke.t`:

- Recognized-token batching vs prefix-index-clean baseline: median `-0.79%`, average `-3.13%` (`/tmp/cppgm-pa34-540-ab-recog-token-batch.md`).
- Post-token batching plus source-file-index cache vs recognized-token batching: median `-1.46%`, average `-0.99%` (`/tmp/cppgm-pa34-540-ab-post-token-batch.md`).
- Early phase check for post-token batching vs recognized-token batching:
  - `-E`: median `-0.62%`, average `-2.27%`.
  - `--emit-ast`: median `-0.05%`, average `-0.10%`.
  - Report: `/tmp/cppgm-pa34-540-ab-post-token-early-phases.md`.

Profile notes:

- Before the post-token/source-location-cache change, the sample still showed hot recognized/post token pull overhead and repeated source file table work: `RecogTokenBuffer::peek/ensure`, `RecogTokenizer::get_many`, `PostTokenizer::get`, `Preprocessor::TokenCursor::get/read_file`, and `SourceLocationTable::add_file`.
- After caching the current file index, `SourceLocationTable::add_file` dropped out of the top stack counts (`/tmp/cppgm-pa34-540-post-token-batch.sample.txt`). The remaining early-layer heat is now mostly `RecogTokenBuffer::peek/ensure`, `RecogTokenizer::get_many`, `PostTokenizer::get_many`, `Preprocessor`, `Macroizer`, `PPTokenizer`, and parser expression descent.

Rejected follow-up:

- Tried reusing scratch vectors for `PostToken`/`RecogToken` batch fills. Full compile timing was too noisy and superficially positive, but cheaper phase checks were consistently slower:
  - `-E`: median `+1.28%`, average `+1.51%`.
  - `--emit-ast`: median `+1.75%`, average `+1.89%`.
  - Report: `/tmp/cppgm-pa34-540-ab-reused-token-early-phases.md`.
- Reverted this scratch-vector reuse. It is not worth keeping without a clearer profile-backed win.

Next likely early-layer opportunities:

- Push batching one more layer down only if we can make `IPPTokenSource`/`Preprocessor` batching avoid real work, not just wrap one-token calls.
- Investigate parser expression descent and template-argument AST parsing separately; `--emit-ast` being almost neutral means the current batching mostly helps token/preprocessor pull overhead, not AST construction.
- Avoid more speculative allocator micro-optimizations unless a profile identifies a specific allocation site with a narrow ownership boundary.

## 2026-04-30: macro and translator-layer follow-up

Additional benchmarking used back-to-back alternating A/B runs because the host was noisy. For `-E`, the benchmark source was a frozen copy of `dev/src/callsemantic.cpp` at `/tmp/cppgm-pa34-540-frozen-callsemantic.cpp`, run with `-E -Idev/src`.

Kept changes:

- `dev/src/macroizer.{h,cpp}` now fast-path simple object-like and function-like macro replacement lists that do not need token pasting/stringizing. Macro definitions precompute whether their replacement list contains `##`/`%:%:` or `#`/`%:` so each expansion does not rescan for those features.
- `dev/src/pptokenizer.cpp` now lets `UTF8Translator`/`FullTranslator` return ordinary ASCII directly on the common path instead of buffering every character in the translator layer and then buffering it again in the tokenizer layer. Special characters that can trigger trigraphs, UCNs, splices, UTF-8 decoding, and EOF still use the existing buffered path.

Rejected change:

- A repeated macro-argument expansion cache was tried for function-like macros whose replacement list references the same parameter more than once. It regressed the large `-E` benchmark, so it was reverted.

Benchmark reports:

- Macro object fast path vs early-layer batch: `/tmp/cppgm-pa34-540-ab-macro-object-fastpath.md` showed full compile median -1.04%, average -1.70%.
- Macro simple function fast path vs object-only: `/tmp/cppgm-pa34-540-ab-macro-simple-function-fastpath-large-preprocess.md` showed large `-E` median -1.42%, average -2.83%.
- Macro precomputed flags vs simple-function: `/tmp/cppgm-pa34-540-ab-macro-precomputed-flags-large-preprocess.md` showed large `-E` median -3.98%, average -2.39%; `/tmp/cppgm-pa34-540-ab-macro-precomputed-flags.md` showed full compile median -0.46%, average -0.60%.
- Rejected repeated-argument cache vs precomputed-flags: `/tmp/cppgm-pa34-540-ab-macro-param-cache-large-preprocess.md` showed large `-E` median +2.28%, average +3.95%.
- Translator ASCII direct vs macro-fastpath: `/tmp/cppgm-pa34-540-ab-translator-ascii-direct-large-preprocess.md` showed large `-E` median -6.81%, average -6.44%; `/tmp/cppgm-pa34-540-ab-translator-ascii-direct.md` showed full compile median -3.68%, average -3.46%.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-translator-ascii-direct-cppgm++`.

## 2026-04-30: `-E` output batching follow-up

After the translator-layer ASCII direct path, a large `-E` profile still showed visible time in `write_preprocessed_posttoken_output` and ostream insertion. `dev/src/preproc_output.cpp` now buffers post-token textual output in a 1 MiB string and flushes with `ostream::write`, while preserving the existing token text format.

Benchmark report:

- Preprocessor output buffering vs translator ASCII direct: `/tmp/cppgm-pa34-540-ab-preproc-output-buffer-large-preprocess.md` showed large `-E` median -7.05%, average -4.56%.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-preproc-output-buffer-cppgm++`.

Rejected follow-up:

- Switching `stream_post_tokens` to use `IPostTokenSource::get_many` was tried after output buffering. It regressed the large `-E` benchmark, likely because the extra vector materialization outweighed fewer virtual calls in this path, so it was reverted. Report: `/tmp/cppgm-pa34-540-ab-stream-post-tokens-batch-large-preprocess.md` showed median +1.67%, average +1.10%.

Rejected follow-up:

- A `PostTokenizer::emit_identifier` prefilter to skip impossible keyword/operator unordered-map lookups was tested and reverted. Report: `/tmp/cppgm-pa34-540-ab-posttoken-keyword-prefilter-large-preprocess.md` showed large `-E` median +1.12%, average +1.90%.

## 2026-04-30: startup and host-probe cache

Startup instrumentation was added behind `CPPGM_STARTUP_PROFILE=1` / `CPPGM_STARTUP_TIMING=1`. It reports milestones from `main`, driver parsing, preprocessor construction, first preprocessor token fetch, first post-token emission, and stream completion. Normal runs do not emit timing output.

The fixed pre-token startup cost was traced to `Preprocessor` construction. On empty input, `host_predefined_macros()` spent roughly 490-550 ms spawning the host compiler for `-std=gnu++11 -dM -E -x c++ -`. On include-heavy input, first token production also paid roughly 490 ms for the standard include path probe (`-E -x c++ - -v`).

A persistent raw-output cache was added for host compiler probe commands in `dev/src/preprocessor.cpp`:

- Cache key is based on the exact probe command plus host tool file identity when available.
- Default cache directory is `/tmp`.
- `CPPGM_HOST_PROBE_CACHE_DIR` overrides the cache directory.
- `CPPGM_DISABLE_HOST_PROBE_CACHE=1` disables the cache.

Benchmark reports:

- Empty `-E`, warm cache vs disabled cache: `/tmp/cppgm-pa34-540-ab-host-probe-cache-empty.md` showed median -89.66%, average -89.83%.
- Frozen `callsemantic.cpp -E`, warm cache vs disabled cache: `/tmp/cppgm-pa34-540-ab-host-probe-cache-large-preprocess.md` showed median -11.24%, average -12.93%.
- Full 540 compile, warm cache vs disabled cache: `/tmp/cppgm-pa34-540-ab-host-probe-cache-full.md` showed median -8.81%, average -8.85%.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-host-probe-cache-cppgm++`.

## 2026-04-30: host probe cache superseded by build-time host defaults

The runtime host-probe cache experiment was replaced with build-time generated host defaults. `dev/Makefile` now generates `obj/generated/cppgm_builtin_host_config.h` from the build-time `CPPGM_HOST_CXX` and `CPPGM_STDLIB_FLAGS` using `dev/gen_builtin_host_config.pl`. The generated header contains:

- The build-time host compiler string.
- The build-time stdlib flags string.
- Host predefined macro output from the build-time host compiler.
- Standard include search paths from the build-time host compiler.

`dev/src/preprocessor.cpp` now uses those compiled-in defaults when runtime `CPPGM_HOST_CXX` and `CPPGM_STDLIB_FLAGS` are unset or match the build-time values. It only runs a live host compiler probe when runtime host settings differ from the compiled-in build configuration. This keeps the clean deterministic behavior without managing a runtime cache directory.

Startup instrumentation remains opt-in behind `CPPGM_STARTUP_PROFILE=1` / `CPPGM_STARTUP_TIMING=1`.

Benchmark reports:

- Empty `-E`, build-time defaults vs runtime host probe: `/tmp/cppgm-pa34-540-ab-buildtime-host-defaults-empty.md` showed median -91.78%, average -90.99%.
- Frozen `callsemantic.cpp -E`, build-time defaults vs runtime host probe: `/tmp/cppgm-pa34-540-ab-buildtime-host-defaults-large-preprocess.md` showed median -12.57%, average -12.30%.
- Full 540 compile, build-time defaults vs runtime host probe: `/tmp/cppgm-pa34-540-ab-buildtime-host-defaults-full.md` showed median -11.23%, average -8.40%.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-buildtime-host-defaults-cppgm++`.

## 2026-04-30 update: PPTokenizer identifier dispatch fast path

Kept a low-level lexer dispatch fast path in `PPTokenizer::get` for common identifiers that cannot be string-literal prefixes. The fast path avoids failed pp-number/operator/stringlike probes for ordinary identifiers while preserving the old path for `u`, `U`, `L`, and `R` prefix-sensitive tokens.

A/B reports:

- Large frozen `callsemantic.cpp -E`: `/tmp/cppgm-pa34-540-ab-pptoken-identifier-fastpath-large-preprocess.md` showed median `-4.97%`, average `-1.30%` vs build-time host defaults.
- Full 540 compile: `/tmp/cppgm-pa34-540-ab-pptoken-identifier-fastpath-full.md` showed median `-0.51%`, average `-0.13%` vs build-time host defaults.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-pptoken-identifier-fastpath-cppgm++`.

## 2026-04-30 update: inline lexer buffer and `next()` fast path

Kept a lexer buffer change in `BufferedIterator::Buffer` and `BufferedIterator::next`:

- Replaced the always-heap-allocated 32-slot ring buffer with inline 32-slot storage and dynamic overflow only when lookahead exceeds the inline capacity.
- Optimized the common one-buffered-character `next()` path to replace the front slot with the next source code point instead of doing a pop followed by a push.

A/B reports against `/tmp/cppgm-pa34-540-final-pptoken-identifier-fastpath-cppgm++`:

- Large frozen `callsemantic.cpp -E`: `/tmp/cppgm-pa34-540-ab-buffer-inline-next-large-preprocess.md` showed median `-12.74%`, average `-9.48%`.
- Full 540 compile: `/tmp/cppgm-pa34-540-ab-buffer-inline-next-full.md` showed median `-4.45%`, average `-3.32%`.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-buffer-inline-next-cppgm++`.

## 2026-04-30 rejected experiment: horizontal whitespace fast path

Tried a `PPTokenizer::get` fast path for ordinary horizontal whitespace runs, falling back to the comment-aware scanner when the run reached `/`.

Rejected after `/tmp/cppgm-pa34-540-ab-whitespace-fastpath-large-preprocess.md`: median was `-1.52%`, but average was `+1.14%` against `/tmp/cppgm-pa34-540-final-buffer-inline-next-cppgm++`. The result was too noisy and the change carried extra token-boundary risk around whitespace/comment coalescing, so it was reverted.

## 2026-04-30 rejected experiment: `FullTranslator::operator++` override

Tried adding a `FullTranslator::operator++` override for the common empty-buffer ASCII advancement path, delegating to `BufferedIterator::operator++` only when the translator had buffered translated output.

Rejected after `/tmp/cppgm-pa34-540-ab-fulltranslator-increment-large-preprocess.md`: median was `+1.39%`, average `+2.68%` against `/tmp/cppgm-pa34-540-final-buffer-inline-next-cppgm++`.

## 2026-04-30 rejected experiment: `PostTokenizer` direct common-token return

Tried bypassing the `ready` deque for common one-to-one post tokens (`PP_IDENTIFIER`, `PP_PREPROCESSING_OP`, invalid header/non-whitespace, and EOF) when no string-literal concatenation was pending.

Rejected after `/tmp/cppgm-pa34-540-ab-posttoken-direct-large-preprocess.md`: median was `+6.56%`, average `+6.62%` against `/tmp/cppgm-pa34-540-final-buffer-inline-next-cppgm++`. The extra branching/construction cost outweighed removing the deque push/pop path.

## 2026-04-30 update: post-token preprocessing-op switch lookup

Kept a `PostTokenizer` operator-classification fast path that replaces the unordered-map lookup for `PP_PREPROCESSING_OP` spellings with a compact size/character switch. Identifier/keyword classification still uses the existing map; this avoids repeating the previously rejected identifier keyword prefilter.

A/B reports against `/tmp/cppgm-pa34-540-final-buffer-inline-next-cppgm++`:

- Large frozen `callsemantic.cpp -E`: `/tmp/cppgm-pa34-540-ab-posttoken-op-switch-large-preprocess.md` showed median `-2.71%`, average `-2.87%`.
- Full 540 compile: `/tmp/cppgm-pa34-540-ab-posttoken-op-switch-full.md` showed median `-0.90%`, average `-9.40%`, with noisy full-compile outliers but no median regression.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`.

## 2026-04-30 rejected experiment: force-inline lexer buffer methods

Tried forcing `BufferedIterator::Buffer` tiny methods (`empty`, `front`, `size`, `push_back`, `pop_front`, etc.) to inline with compiler attributes.

Rejected after `/tmp/cppgm-pa34-540-ab-buffer-always-inline-large-preprocess.md`: median was `+9.04%`, average `+11.62%` against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`. The likely cause is code-size / optimizer-heuristic fallout, so the methods were restored to normal definitions.

## 2026-04-30 rejected experiment: `PostTokenizer` source-location guard

Tried skipping `capture_current_location()` calls when `PostTokenizer` has no source-location table/provider installed.

Rejected after `/tmp/cppgm-pa34-540-ab-posttoken-location-guard-large-preprocess.md`: median was `+0.18%`, average `+0.81%` against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`. The compiler already handles the inline null checks well enough, so the extra outer branch was not useful.

## 2026-04-30 rejected experiment: common `PostToken` emplace constructors

Tried adding common-case `PostToken` constructors and using `ready.emplace_back` in `push_invalid`, `push_simple`, and `push_identifier` to avoid constructing the full temporary with empty vector/string arguments.

Rejected after `/tmp/cppgm-pa34-540-ab-posttoken-emplace-large-preprocess.md`: median was `+2.47%`, average `+2.89%` against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`. The existing temporary path appears to optimize better than the added overloads/emplace path.

## 2026-04-30 rejected experiment: full keyword switch lookup

Tried replacing the identifier keyword unordered-map lookup with a complete size/first-character switch for C++ keywords, keeping the operator switch for `PP_PREPROCESSING_OP`.

Rejected after the corrected candidate passed full compilation but regressed `/tmp/cppgm-pa34-540-ab-posttoken-keyword-switch-full.md`: median `+1.46%`, average `+0.52%` against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`. The large `-E` run was modestly positive (`/tmp/cppgm-pa34-540-ab-posttoken-keyword-switch-large-preprocess.md`, median `-1.25%`, average `-2.11%`), but the full compile path remains the controlling benchmark.

## 2026-04-30 rejected experiment: `match_whitespace` boolean return

Tried changing `match_whitespace` from returning a consumed-token count to returning a boolean flag, since its caller only checks truthiness.

Rejected after `/tmp/cppgm-pa34-540-ab-whitespace-bool-large-preprocess.md`: median was `+1.25%`, average `+2.31%` against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`. The original counter loop appears to optimize better.

## 2026-04-30 update: known-initial identifier scan helper

Kept a `consume_identifier_known_initial` helper for the `PPTokenizer::get` identifier fast path. The dispatch path already proves the first code point is a valid identifier start, so this avoids repeating the initial identifier-range check in the hot common path while leaving the generic `consume_identifier` validation path intact for string-literal suffixes and fallback callers.

A/B reports against `/tmp/cppgm-pa34-540-final-posttoken-op-switch-cppgm++`:

- Large frozen `callsemantic.cpp -E`: `/tmp/cppgm-pa34-540-ab-identifier-known-initial-large-preprocess.md` showed median `-1.21%`, average `+0.51%` under noisy samples.
- Full 540 compile: `/tmp/cppgm-pa34-540-ab-identifier-known-initial-full.md` showed median `-10.93%`, average `-4.64%`, also with heavy machine-noise outliers in both variants.

Current final candidate binary: `/tmp/cppgm-pa34-540-final-identifier-known-initial-cppgm++`.

## 2026-04-30 rejected experiment: 64-byte identifier ASCII scratch

Tried increasing `consume_identifier`'s local ASCII staging buffer from 32 bytes to 64 bytes to reduce append flushes for long identifiers.

Rejected after `/tmp/cppgm-pa34-540-ab-identifier-ascii64-large-preprocess.md`: median was `+1.76%`, average `+2.73%` against `/tmp/cppgm-pa34-540-final-identifier-known-initial-cppgm++`. The larger stack scratch likely hurt cache/register behavior more than it helped long names.

## 2026-04-30 rejected experiment: `Normalizer::operator++` with `snextc()`

Tried replacing the `sbumpc()` + `sgetc()` pair in `Normalizer::operator++` with `streambuf::snextc()`.

Rejected after `/tmp/cppgm-pa34-540-ab-normalizer-snextc-large-preprocess.md`: median was nearly flat at `-0.35%`, but average regressed `+1.42%` against `/tmp/cppgm-pa34-540-final-identifier-known-initial-cppgm++`. The existing explicit pair remains clearer and not measurably worse.

## Lazy Header Function Body Strategy

The next significant performance experiment is to stop eagerly parsing hosted-header
function bodies that are unlikely to be needed for `540`.

Current phase-cut data puts the local floor around:

- `-E`: about `1.90s`
- `--emit-ast`: about `3.18s`
- `-c`: about `5.93s`

That means eager AST construction costs roughly `1.28s` over preprocessing, and
full compile adds another roughly `2.75s` beyond AST construction. Lazy function
body parsing can recover part of the AST cost, but it only gets us near the clean
`~2s` target if it also prevents semantic body work and unnecessary template or
class completion.

### First Implementation Slice

The first slice is intentionally experimental and opt-in:

- Gate lazy body parsing behind `CPPGM_LAZY_HEADER_FUNCTION_BODIES=1`.
- Keep default compiler behavior unchanged.
- Only skip plain compound function bodies whose source location comes from a
  non-primary source file.
- Keep user-file bodies, especially the `540` `main`, eagerly parsed.
- Represent skipped bodies with an explicit `lazy-function-body` AST node rather
  than an empty `compound-statement`.
- Preserve the original token span on the lazy body node so later materialization
  has a source range.
- Count skipped bodies and skipped tokens with `CPPGM_LAZY_BODY_STATS=1`.

This should provide a quick signal for hosted headers without first solving the
hard correctness problem of restoring parser scope for arbitrary delayed body
materialization.

### Why Header Bodies First

General lazy body parsing is possible, but body parsing depends on parser lookup
state. The parser uses signature type hints, parameter value names, template
parameter scopes, namespace scopes, class-member name hints, and visible names to
disambiguate declarations from expressions inside a function body. A fully lazy
implementation must snapshot or reconstruct that state before reparsing a body.

For `540`, the low-risk path is different: most expensive bodies are expected to
come from `<functional>` and related hosted headers, while the translation unit's
own `main` body is tiny and immediately needed. Skipping only non-primary-file
plain compound bodies should show whether body deferral has enough leverage while
leaving the user body path unchanged.

### Expected Signal

Useful benchmark commands once this slice builds:

```sh
env CPPGM_LAZY_BODY_STATS=1 \
  ./dev/cppgm++ -c -o /tmp/540.lazy.o \
  pa34/tests/compile/540-reference-wrapper-smoke.t
```

A good signal would be:

- many skipped hosted-header bodies
- many skipped hosted-header body tokens
- no or very few paths requiring skipped bodies
- a measurable `-c` reduction versus the current `~5.93s` median

If `-c` barely moves despite substantial skipped body tokens, the dominant cost
is probably semantic class/template declaration work rather than function body
AST parsing.

### Follow-Up Materialization Slice

If the first slice is promising, add real on-demand materialization:

- Add a semantic helper that recognizes `lazy-function-body` and reparses its
  token span only when a body consumer actually needs children.
- Store materialized bodies in an analyzer-owned lifetime pool.
- Route all direct `body->children` users through the helper.
- Add materialization counters and reasons.
- Expand support from plain compound bodies to constructor initializers,
  special-member bodies, and function-try-bodies only after the plain-body path
  is stable.

### Larger Fallback If Body Laziness Is Not Enough

If lazy header function bodies do not produce a significant `540` improvement,
the next structural target is lazy class/template completion:

- Instrument class completion request levels.
- Separate name/shape/layout/member-declaration/member-body/output completion.
- Avoid collecting or checking hosted-header member bodies when only type
  identity, layout, or trait evaluation is required.
- Make output closure worklist-driven so unused hosted declarations are not
  repeatedly rescanned.

### Lazy Body Prototype Status

Implemented in this worktree and now enabled by default:

- Lazy parsing skips plain compound function bodies from non-primary source
  files.
- `CPPGM_LAZY_BODY_STATS=1` reports parser skip counts and semantic
  materialization counts.
- Skipped bodies are represented by an explicit `lazy-function-body` AST node.
- Semantic body lookup now treats `lazy-function-body` as a definition body
  placeholder.
- Output-demanded lazy bodies are materialized on first use by reparsing the
  recorded token span and caching the materialized AST node in the analyzer.
- The lazy skipper was changed to scan forward with `peek()` instead of calling
  `tokens.size()`, because `size()` forces full token materialization on the
  streaming `RecogTokenBuffer` and made the first prototype slower.
- `complete_class_type` now fast-returns for null and non-`TK_NAMED` types before
  building diagnostic/hotspot strings, avoiding thousands of scalar no-class
  completion attempts from the expensive path.

Current lazy-body signal on `540`:

- Lazy parser stats: `4724` hosted-header bodies skipped, `145833` body tokens
  skipped, `0` skip failures.
- Full compile materialization stats: `20` demanded bodies, `372` materialized
  tokens, `0` materialization failures.
- `--emit-semantics` sample after lazy body support: baseline `6.68s`, lazy
  `6.28s`.
- Clean alternating `-c` sample after the streaming-skip and scalar class
  completion fixes: baseline `5.46 / 6.86 / 5.96 / 8.70`, lazy `4.99 / 5.46 /
  5.77 / 7.44`.
- That noisy set moves the median from about `6.41s` to about `5.62s`.

Interpretation:

- Lazy function bodies are viable and correct enough for the `540` smoke path.
- The win is real but not large enough to approach the `~2s` clean target.
- The dominant remaining work is semantic template/class machinery, not function
  body parsing.
- Hotspot after this slice still shows thousands of template argument
  resolutions, class-template reference lookups, and output closure activity.
- Next high-leverage direction is lazy class/template completion rather than
  more function-body laziness.

## Lazy class/template reference prototype status

Implemented a first class-template laziness slice and enabled it by default:

- Reference-only class template uses are cached separately from full instantiations.
- Full completion and full instantiation promote the reference entry so type identity is preserved.
- Repeated class-template references with an existing non-forward `ClassInfo` now skip specialization reselection and source-use bookkeeping when template tracing and witness source capture are disabled.

The 540 stats run with lazy function bodies plus class-template reference laziness still sees the same reference volume (`class-template-reference-requests=3072`, `fast-existing-hits=2302`, `creates=700`), but specialization-name construction dropped from the earlier profiled `6473` to `1400`. Wall-clock timing on the current machine is too noisy for a reliable conclusion; user CPU was roughly flat in the short alternating sample, so the next likely win is reducing repeated template-argument resolution/key construction rather than just separating the reference/full caches.

### Shared Text Interning Rollout - 2026-05-01

The first profitable interning target was the identifier-token mention cache used by template argument semantic checks. That cache now stores interned text atoms rather than owning duplicate strings.

The broader retained rollout uses a shared `text_intern` atom pool for repeated identifier-like identities where pointer equality replaces repeated string storage:

- semantic identifier-token sets collected from text fragments
- parser/template-angle `NameSet` scope stacks
- semantic text identifier-token cache keys

A direct conversion of `SemanticCache::intern_text` to the global atom pool was tested and rejected. That cache is already local and already interns full text fragments; routing every fragment through the global atom table added hash traffic and regressed the 540 timing.

Alternating 540 timing against the pre-identifier-interning binary on the noisy machine averaged about 5.08s real / 4.47s user for the shared interning build versus 5.20s real / 4.72s user before interning. This keeps the interning direction, but the next conversions should remain limited to high-duplication identity sets/caches rather than full semantic model string-key maps unless we also redesign lookup APIs around atoms.
