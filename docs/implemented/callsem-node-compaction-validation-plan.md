# CallSemNode Compaction Validation Plan

## Goal

Decide whether semantic-analysis performance is now dominated more by fat
semantic output nodes and allocator/cache pressure than by demand-reachability
work. This plan is a validation gate before continuing into a larger
demand-DAG rewrite.

## Current Facts

- `sizeof(CallSemNode)=552` and `alignof(CallSemNode)=8` with the current
  Homebrew LLVM/libc++ build.
- The largest always-inline members are:
  - seven `std::string` fields at 24 bytes each
  - one inline `std::vector<CallSemNode>` at 24 bytes
  - four `TypePtr` shared pointers at 16 bytes each
  - one `std::vector<pair<string, uint64_t> >` at 24 bytes
  - one `shared_ptr<CallSemNode>` at 16 bytes
  - `symbol_linkage::SymbolIdentity` at 56 bytes
- Phase 0 `semantic_overload.cpp` baseline:
  - median wall time: `203.475s`
  - median max RSS: `2258200 KB`
- Phase 1a negative-class fast path moved query accounting but should not
  materially change wall time:
  - `class-info-for-type-calls`: `9179831 -> 7862373`
  - `class-info-for-type-definitely-not-class-skips`: `1317464`
  - `complete-class-type-calls`: `475171 -> 134482`
  - `complete-class-type-definitely-not-class-skips`: `340692`

## Hypothesis

The compiler is doing too much work per semantic-output node even when the
semantic algorithm is nominally linear. Large inline node size, recursive
`vector<CallSemNode>` ownership, string/vector allocations, and `shared_ptr`
refcount traffic can make large files fall off a cache/allocator cliff that is
not visible on small PA tests.

If this is true, node compaction should move `semantic_overload.cpp` more than
small query-count cleanups do.

## Validation Phases

### Phase A: Improve Measurement

Make `CPPGM_MEMORY_CENSUS=1` report field-aware `CallSemNode` details:

- node count
- inline bytes: `count * sizeof(CallSemNode)`
- per-string payload capacity by field
- child vector storage bytes
- virtual-base layout storage bytes
- `SymbolIdentity` payload bytes
- reachable cached body output vs final translation-unit output

This should remain behind `CPPGM_MEMORY_CENSUS` and must not affect normal
builds or strict output.

### Phase B: Low-Load Census Runs

Run one census on a small fixture and one on `semantic_overload.cpp` when the
host is idle enough. Capture:

- `semantic-memory kind=callsem.output`
- new field-level `semantic-memory-detail` lines
- process RSS from the benchmark harness

Decision rule:

- If `CallSemNode` inline bytes and child-vector payload are a large fraction
  of reported semantic memory, proceed to Phase C.
- If other categories dominate, follow that category instead before changing
  node layout.

### Phase C: Allocator Evidence

Use one of these equivalent checks:

- sample/Instruments wall profile and inspect malloc/free/refcount traffic
- run the same benchmark with a faster allocator if available locally
- run macOS malloc logging only on a smaller hosted fixture if the full middle
  tier is too expensive

Decision rule:

- If allocation/free/refcount traffic is a large sample share, compaction is
  likely the right next optimization.
- If semantic logic dominates samples instead, demand-DAG/fixpoint work stays
  ahead of compaction.

### Phase D: Smallest Compaction Experiment

Try the smallest reversible layout change:

- move rarely populated string fields out of `CallSemNode` into side storage,
  starting with source/debug/symbol spelling fields rather than `text`
- keep APIs stable initially through accessors or explicit helper functions
- measure `semantic_overload.cpp` once against Phase 0/Phase A counters

Candidate fields for the first experiment:

- `source_file`
- `vtt_symbol`
- `vtt_object_symbol`
- `runtime_bridge_symbol`
- `local_static_guard_symbol`

Do not move `text` first unless census proves it is sparse enough and the
touch count is manageable. It is the most broadly used node payload field and
is more likely to require invasive API churn.

## Plan Impact

If Phase B/C confirms the hypothesis, priority changes to:

1. `CallSemNode` compaction and ownership flattening.
2. Demand-DAG/fixpoint removal, with a realistic ceiling around the measured
   reachable fraction.
3. Remaining query representation cleanups.
4. Header/cache work.

If Phase B/C does not confirm it, resume the demand-driven semantic-analysis
plan at the demand-level API phase.

## Phase B Result: Semantic Overload Census

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_MEMORY_CENSUS=1 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-callsem-memory-semantic-overload-fixed
```

Compiler head before committing the census-accounting fix:
`6badb7a1421e3f067d60793838cfbb68238156a6` plus the working tree change that
counts only heap-backed `std::string` storage.

Results:

- wall time: `180.544s`
- max RSS: `2138700 KB`
- semantic census total: `957072019` bytes
- `callsem.output`: `223710964` bytes across `170345` nodes (`23.37%`)
- `callsem.cached_body`: `166674930` bytes across `141564` nodes (`17.42%`)
- combined CallSemNode output/cached body storage: `390385894` bytes (`40.79%`)
- `type`: `205748832` bytes (`21.50%`)
- `scope`: `170076782` bytes (`17.77%`)

CallSemNode details:

- final output inline node bytes: `94030440`
- final output child vector storage: `97284480`
- cached body inline node bytes: `78143328`
- cached body child vector storage: `74571336`
- combined inline node bytes: `172173768`
- combined child vector storage: `171855816`
- inline plus child vector storage: `344029584` bytes (`35.95%` of the
  semantic census)

Interpretation:

- Phase B confirms that `CallSemNode` storage is the largest single semantic
  memory category on the middle-tier fixture.
- The high-value target is not string payload first. After correcting for
  small-string optimization, heap string payload is much smaller than inline
  node bytes and child vector storage.
- The first compaction experiment should therefore target node layout and
  child ownership, not just moving string text fields to side storage.

## Phase C Result: Allocator A/B

Default allocator command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-allocator-default-semantic-overload
```

tcmalloc command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --env DYLD_INSERT_LIBRARIES=/usr/local/opt/gperftools/lib/libtcmalloc.dylib \
  --output-prefix /tmp/cppgm-allocator-tcmalloc-semantic-overload
```

Compiler head: `590cbe9318d3c34a46a5a8f61a923cc1049ab3e5`

Results:

- default allocator: `196.229s`, max RSS `2142024 KB`
- tcmalloc: `151.085s`, max RSS `1866424 KB`
- wall-time delta: `-23.0%`
- RSS delta: `-12.9%`

Interpretation:

- Phase C confirms allocator pressure is a major part of the middle-tier
  slowdown.
- This reinforces the Phase B conclusion: reducing `CallSemNode` count/size
  and recursive child-vector allocation should come before a large demand-DAG
  rewrite.
- Since allocator replacement alone produces a large win, node compaction
  should be measured against both default allocator and tcmalloc. A compaction
  that only helps the default allocator may still be valuable, but the durable
  target is less allocation and better locality independent of allocator.

## Phase D Result: Sparse Extra Sidecar

The compaction work moved rarely populated or cold per-node fields into a
copy-on-write sidecar and then tightened the remaining inline layout. The
sidecar fields now include:

- source file
- VTT internal symbol
- VTT object symbol
- runtime bridge symbol
- local static guard symbol
- lowered condition test
- VTT owner type
- materialization source type
- conversion source type
- virtual base layout
- source line and source column

Each implementation stage was rebuilt with:

```sh
make -C dev cppgm++ -j4 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

and validated with:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-strict-nobuild \
  STRICT_PAS='pa18 pa19 pa21 pa22' \
  STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

All strict runs passed.

Measured retained CallSem storage on `self-semantic-overload`:

| Stage | Head | Node | Extra | Output | Cached Bodies | Combined |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Corrected baseline | `6badb7a` + census fix | 552 | n/a | 223.7 MB | 166.7 MB | 390.4 MB |
| Sparse strings, lowered condition, layout, flags | `2c8ff17` | 360 | 136 | 155.3 MB | 119.3 MB | 274.6 MB |
| Cold type refs in sidecar | `70e5e19` | 312 | 184 | 138.9 MB | 108.2 MB | 247.1 MB |
| Virtual-base layout in sidecar | `4f1d976` | 288 | 208 | 130.6 MB | 102.5 MB | 233.1 MB |
| Source line/column in sidecar | `b7ab183` | 272 | 224 | 125.1 MB | 98.8 MB | 223.9 MB |

The final measured reduction is `166.4 MB` of retained CallSem output/cached
body storage, or about `42.6%` from the corrected baseline. Inline node bytes
plus child-vector element storage dropped from `344.0 MB` to `169.5 MB`; sidecar
inline storage rose to `10.8 MB`, which is the intended tradeoff.

Wall time and RSS were noisy because the host was under load, so the retained
byte counters are the primary signal. The final single-run RSS was still lower
than the baseline (`2084532 KB` vs. `2138700 KB`), but not enough should be read
into that one run.

Stopping decision:

- The next largest inline candidates are `symbol` and `qualified_name_syntax`.
- Moving either could increase memory if it creates sidecars for many nodes that
  currently have none.
- Before moving them, add census counters for non-empty `symbol` and
  `qualified_name_syntax` and estimate sidecar allocation fallout.
- A deeper child-ownership rewrite may still be useful, but it is no longer a
  contained field-layout change.

## Phase E Result: Duplicate Subtree Detection

The first duplicate detector is opt-in through `CPPGM_CALLSEM_DUP_HASH=1`. It
walks the final output tree plus every retained `FunctionBinding::cached_body`
tree and reports:

- exact structural duplicate groups, collision-checked with recursive equality
- same-shape/non-exact candidate groups, where the operation tree matches but
  enrichment differs
- output/cached split counts for each duplicate class

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_CALLSEM_DUP_HASH=1 \
  --env CPPGM_CALLSEM_DUP_HASH_TOP=25 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-callsem-dup-hash-split-semantic-overload
```

Compiler head: `e2d5427997857849dd4e803c3b78acfcd91abe4b`

Result:

- roots walked: `6529`
- node occurrences walked: `312015`
- exact duplicate groups: `99602`
- exact duplicate subtree occurrences beyond first representative: `186602`
- exact duplicate overlap estimate: `889798` nodes / `582645000` bytes
- exact duplicate overlap split:
  - mixed output/cache: `99186` groups / `581654244` bytes
  - output-only: `416` groups / `990756` bytes
  - cached-only: `0` groups / `0` bytes
- same-shape/non-exact candidate groups: `11240`
- same-shape/non-exact candidate subtree occurrences beyond first
  representative: `195992`
- same-shape/non-exact candidate overlap estimate: `563123` nodes /
  `331786426` bytes
- same-shape/non-exact candidate split:
  - mixed output/cache: `9059` groups / `315935500` bytes
  - output-only: `2181` groups / `15850926` bytes
  - cached-only: `0` groups / `0` bytes

The byte numbers are intentionally labeled "overlap" because every duplicated
subtree root includes its descendants; nested duplicate groups double-count the
same retained memory. They are useful for ranking and source classification, not
as a direct RSS/retained-byte savings estimate.

Interpretation:

- Pure duplicate pressure is real, but it is overwhelmingly the same tree
  retained both in final output and in cached bodies.
- The largest exact duplicate groups are whole statement/body subtrees with
  `count=2`, `output=1`, `cached=1`.
- There is almost no cached-only exact duplication, so memoizing duplicate
  cached bodies against each other is not the first lever.
- The practical near-term target is cached-body lifetime/ownership: once a body
  has been copied into final output or consumed by LowIR, avoid retaining a
  second full copy in `FunctionBinding::cached_body_output` unless later
  semantic work can still demand it.
- Same-shape/non-exact candidates are also mostly mixed output/cache. The top
  root-level variation masks are `source,type`, `resolved_name,type`,
  `type`, and descendant-only differences. This suggests many candidate
  "functional duplicates" are the same operation tree enriched differently
  during final output versus cached-body retention.

Next validation:

- Add a root-level cached-body/output ownership probe that identifies which
  cached body roots are also present in final output.
- Estimate non-overlapping savings from dropping or moving those cached roots,
  instead of relying on overlapping subtree sums.
- Only after that, consider a demand-driven output API that prevents building
  or retaining cached body trees that are immediately copied into the final
  output tree.

## Phase F Result: Cached-Body Provenance

The root-level provenance probe is opt-in through
`CPPGM_CALLSEM_PROVENANCE=1`. It indexes final output subtrees, then classifies
each retained `FunctionBinding::cached_body_output` root as:

- `exact-output-match`
- `shape-only-output-match`
- `no-output-match`

Command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_CALLSEM_PROVENANCE=1 \
  --env CPPGM_CALLSEM_DUP_HASH_TOP=25 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-callsem-provenance-semantic-overload
```

Compiler head: `f6e7c316`

Result:

- output tree: `170501` nodes / `140387404` bytes
- cached body roots: `6528`
- cached body storage: `141550` nodes / `101055646` bytes
- exact output matches: `6528` roots / `101055646` bytes
- shape-only output matches: `0`
- no-output matches: `0`

This confirmed that retained cached bodies were not a broad family of
near-misses or independent duplicate computations on this benchmark. They were
all exact body roots also present in final output. The implementation source was
the unconditional post-emission cache refresh in `analyze_function_binding_output`:
after building the function body and copying it into final output, the code also
rebuilt `binding.cached_body_output` from the same body even though the binding's
successful output path sets `definition_output_emitted`.

The provenance probe is useful but expensive on this benchmark (`311.524s`,
`1963328 KB` in this run), so it should stay investigative rather than becoming
a normal regression gate.

## Phase G Result: Drop Emitted Body Caches

Patch:

- Removed the unconditional `binding.cached_body_output.reset(new DumpNode(...))`
  after successful function body output.
- Cleared any preexisting body cache once the binding successfully finishes
  output.
- Kept the earlier lambda return-type cache behavior intact; it can still avoid
  reanalyzing a lambda body before the eventual function output consumes it.
  The cache path now transfers ownership into the binding and then into final
  output instead of copying the same body tree at each handoff.

Compiler head: `155fcd6a`

Validation:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-strict-nobuild \
  STRICT_PAS='pa18 pa19 pa21 pa22' \
  STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

All strict runs passed.

Post-patch memory-census command:

```sh
scripts/run_structured_ast_perf_benchmarks.py \
  --skip-build \
  --benchmark self-semantic-overload \
  --repeat 1 \
  --timeout-sec 1200 \
  --env CPPGM_MEMORY_CENSUS=1 \
  --env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  --output-prefix /tmp/cppgm-callsem-cache-drop-memory-semantic-overload
```

Post-patch retained CallSem output storage:

- `callsem.output`: `170495` nodes / `140385724` bytes
- `callsem.output.lowered_condition_test`: `6` nodes / `1680` bytes
- `callsem.cached_body`: no retained census entry

On the same current-head benchmark shape, this removes about `101.1 MB` of
retained cached-body tree storage after function output. The wall-time sample
was noisy and census-heavy (`391.613s`, `1935108 KB`), so the byte counters are
the reliable signal.
