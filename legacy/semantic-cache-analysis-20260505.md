# Semantic Cache Analysis - 2026-05-05

Benchmark:

```sh
CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  ./dev/cppgm++ -I dev/src -c -o /tmp/semantic_overload.o \
  benchmarks/self_compile/stable/semantic_overload.cpp
```

Strict validation after code changes:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict \
  STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Cache Inventory

The hot semantic path currently uses these caches:

- `SemanticCache::identifier_token_cache`
- `SemanticCache::template_placeholder_mentions_cache`
- `SemanticCache::non_namespace_binding_mentions_cache`
- `SemanticCache::dependent_non_namespace_binding_mentions_cache`
- `SemanticCache::qualified_type_lookup_cache`
- `SemanticCache::parsed_type_text_cache`
- `SemanticCache::dependent_type_resolution_cache`
- `Analyzer::class_info_for_type_cache_`
- `Analyzer::class_info_for_type_named_key_cache_`
- `template_resolution::stable_bound_member_failure_cache`
- `template_resolution::resolve_template_arguments_fast_cache`
- `template_resolution::resolve_template_arguments_cache`

The benchmark does not exercise `identifier_token_cache` or
`parsed_type_text_cache` directly. The local-scope capture cache is not part of
the template query hot path on this benchmark and was not separately disabled.

## Disable Results

Serial sweep at `a58df281` with `CPPGM_SEMANTIC_STATS=1`:

| Variant | Real s | RSS bytes | Notable counter movement |
| --- | ---: | ---: | --- |
| baseline | 73.98 | 1,156,669,440 | `resolve-template-argument-calls=262628` |
| no template-placeholder mentions | 74.25 | 1,143,128,064 | `scope-cache-key-calls` 473848 -> 257525 |
| no non-namespace mentions | 76.98 | 1,129,844,736 | `scope-cache-key-calls` 473848 -> 312912 |
| no dependent non-namespace mentions | 77.63 | 1,145,810,944 | `scope-cache-key-calls` 473848 -> 429731 |
| no qualified type lookup | 78.16 | 1,135,521,792 | low hit count: 179 hits / 9129 misses |
| no dependent type resolution | 77.43 | 1,124,298,752 | first run still built the key before bypass |
| no class-info-for-type | 79.06 | 1,132,597,248 | map lookups 10838 -> 6808946 |
| no bound-member failure | 77.57 | 1,136,963,584 | 684 avoided repeated failures |
| no template-arguments fast | 78.09 | 1,119,838,208 | full key builds 34623 -> 105966 |
| no template-arguments | 95.92 | 1,087,541,248 | calls 262628 -> 470003 |
| no parsed-type text | 77.27 | 1,135,685,632 | inactive on this benchmark |

Direct `time -l` reruns at `a58df281` for the most important cases:

| Variant | Real s | RSS bytes | Instructions |
| --- | ---: | ---: | ---: |
| baseline | 73.29 | 1,122,783,232 | 414,849,103,182 |
| no template-arguments | 92.00 | 1,069,342,720 | 535,502,675,261 |
| no template-arguments fast | 74.98 | 1,144,709,120 | 417,119,952,712 |
| no class-info-for-type | 85.00 | 1,130,348,544 | 425,147,839,375 |
| no template-placeholder mentions | 74.10 | 1,135,775,744 | 419,115,646,526 |

After correcting the dependent-type disable path so it does not build the
stable text key first, disabling that cache was roughly neutral/slightly lower
instruction count on this benchmark. That means the current stable-text key is
expensive enough that this cache is not a clear win for `semantic_overload.cpp`.

## Decisions

- Keep `resolve_template_arguments_cache`. It is the largest confirmed win:
  disabling it adds about 120.7B instructions and 18.7s on the direct run.
- Keep `resolve_template_arguments_fast_cache`. It mostly avoids retained-key
  construction. Disabling it increases full key builds from 34,623 to 105,966.
- Keep `class_info_for_type_cache_` and named-key cache. Disabling them turns
  10,838 class map lookups into about 6.8M lookups. A pointer `unordered_map`
  experiment was not a win, so the existing `std::map` remains.
- Keep the mention caches. They are not huge wall-time wins individually, but
  direct timing shows the template-placeholder cache saves about 4.3B
  instructions on this run.
- Keep `qualified_type_lookup_cache` for now. It has low hit rate on this
  benchmark, but retains only successful results and costs very little memory.
- Keep `stable_bound_member_failure_cache`; it avoids 684 repeated determinate
  member failures with only 138 entries.
- Treat `dependent_type_resolution_cache` as suspicious. The cache may help on
  other inputs, but this benchmark says the recursive stable string key is too
  expensive. A safe cheaper key likely needs type immutability/versioning before
  pointer-keying can replace the stable text.
- `identifier_token_cache` and `parsed_type_text_cache` need separate targeted
  workloads; they are inactive on this benchmark.

## Implemented Changes

- Added env toggles for cache-disable measurements:
  - `CPPGM_DISABLE_TEMPLATE_PLACEHOLDER_MENTIONS_CACHE`
  - `CPPGM_DISABLE_NON_NAMESPACE_BINDING_MENTIONS_CACHE`
  - `CPPGM_DISABLE_DEPENDENT_NON_NAMESPACE_BINDING_MENTIONS_CACHE`
  - `CPPGM_DISABLE_QUALIFIED_TYPE_LOOKUP_CACHE`
  - `CPPGM_DISABLE_DEPENDENT_TYPE_RESOLUTION_CACHE`
  - `CPPGM_DISABLE_CLASS_INFO_FOR_TYPE_CACHE`
  - `CPPGM_DISABLE_BOUND_MEMBER_FAILURE_CACHE`
  - `CPPGM_DISABLE_TEMPLATE_ARGUMENTS_CACHE`
  - `CPPGM_DISABLE_TEMPLATE_ARGUMENTS_FAST_CACHE`
- Interned retained `resolve_template_arguments_cache` parameter keys and text
  keys. Memory census for that cache dropped from about 35.5MB to 22.9MB.
- Fixed the dependent-type cache disable path to bypass stable key construction.

## Follow-Ups

- Add targeted workloads for `identifier_token_cache`,
  `parsed_type_text_cache`, and witness-heavy dependent type resolution.
- If dependent type resolution remains hot, add a cheap type state/version
  identifier before attempting a pointer-keyed first-level cache.
- Consider per-query counters for stable key construction bytes/time so cache
  usefulness can be judged by avoided work, not only hit count.
