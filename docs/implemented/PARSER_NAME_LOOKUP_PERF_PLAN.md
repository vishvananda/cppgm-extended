## Goal

Status note:

- Stages 1 through 4 in this plan landed and produced the large parser-side
  win that justified the work.
- Stage 5 and the broader post-HHC continuation were intentionally not carried
  further as part of this plan.
- After Stage 4, the dominant cost moved out of parser-side name-environment
  churn and into semantic/instantiation work, so additional parser-specific
  cleanup no longer offered enough benefit to justify continuing this exact
  plan.

This document is archived as implemented/closed:

- implemented where the parser-side changes paid off
- intentionally stopped where the remaining work no longer looked like the
  right bottleneck

Reduce parser-side hosted header latency by removing the remaining hot
name-environment churn exposed by `xctrace`, while preserving the recent
template-angle correctness fixes.

## Working Rules

- Validate every stage with:
  - `make verify-fast-pa10-31 CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
- Re-time these representative slow paths after each stage:
  - `cppast` on `#include <algorithm>`
  - `cppast` on `#include <array>`
  - `cpphostcompat -c` on `#include <algorithm>`
  - `cpphostcompat -c` on `#include <array>`
- Commit after each completed stage before starting the next stage.

## Profile Baseline

Measured after the recent template-angle refactor and before this plan:

- warm `verify-fast-pa10-31`: about `27.5s`
- `cppast` on `<algorithm>`: about `44-46s`
- `cppast` on `<array>`: about `44-46s`
- `cpphostcompat -c` on `<algorithm>`: about `60-61s`
- `cpphostcompat -c` on `<array>`: about `59-61s`

`xctrace` shows the remaining parser-heavy cost is dominated by:

- `CppAstParser::parse_declaration(...)`
- `CppAstParser::parse_namespace_declaration(...)`
- `CppAstParser::parse_template_declaration(...)`
- `CppAstParser::make_template_angle_lookup(...)`
- `CppAstParser::~CppAstParser()`
- `std::__tree::__insert_range_unique...`
- `std::__tree::__tree_deleter::operator()`
- `std::__hash_table::__copy_construct...`

That points at repeated construction, copying, and destruction of parser
name-lookup state rather than one isolated semantic routine.

## Stages

- [x] Stage 1: Replace merged template-angle lookup snapshots with a
  stack-backed lookup view.
  Why:
  - `CppAstParser::make_template_angle_lookup(...)` is still a major hotspot
  - the current `NameSetLookup` materializes merged `std::set<std::string>`
    snapshots and pays heavy copy/destructor cost
  Success criteria:
  - parser template-angle lookup no longer merges the active name stacks into
    flat ordered sets on hot paths
  - `xctrace` no longer shows `make_template_angle_lookup(...)` and
    `std::__tree::__insert_range_unique...` as dominant parser frames
  - full validation and timing reruns pass
  Notes:
  - `template_angle_lookup::ScopedNameLookup` now answers membership directly
    from the parser's existing scope stacks instead of materializing merged
    `std::set<std::string>` snapshots
  - `CppAstParser::make_template_angle_lookup(...)` now returns that
    lightweight stack-backed view
  - validation/timing after this stage:
    - warm `verify-fast-pa10-31`: `28.26s`
    - `cppast` `<algorithm>`: `5.94s`
    - `cppast` `<array>`: `6.03s`
    - `cpphostcompat -c` `<algorithm>`: `51.30s`
    - `cpphostcompat -c` `<array>`: `52.84s`

- [x] Stage 2: Reduce temporary `CppAstParser` construction in template
  argument and fragment probe paths.
  Why:
  - `CppAstParser::~CppAstParser()` still shows up prominently
  - temporary parser instances in fragment/template-argument acceptance are
    likely paying repeated scope-vector construction and teardown
  Success criteria:
  - hot parser paths no longer copy full parser name-scope stacks into
    temporary fragment/probe parsers
  - `CppAstParser::~CppAstParser()` and associated hash-table copy frames drop
    materially in the hotspot list
  - full validation and timing reruns pass
  Notes:
  - temporary fragment/probe parsers now inherit the parent's visible name
    stacks by reference, only materializing a flattened inherited view for
    nested probe-parser chains
  - validation/timing after this stage:
    - warm `verify-fast-pa10-31`: `28.44s`
    - `cppast` `<algorithm>`: `2.80s`
    - `cppast` `<array>`: `2.84s`
    - `cpphostcompat -c` `<algorithm>`: `50.48s`
    - `cpphostcompat -c` `<array>`: `52.00s`

- [x] Stage 3: Reduce semantic fragment environment churn.
  Why:
  - after Stage 2, the hosted `xctrace` hotspot moved out of the parser and
    into semantic fragment reparsing
  - `collect_visible_fragment_template_names(...)`,
    `collect_visible_fragment_type_names(...)`, and
    `collect_visible_fragment_value_names(...)` were rebuilding four ordered
    sets by walking the scope chain separately for every fragment parse
  Success criteria:
  - visible fragment names are collected in one pass
  - fragment seeding and angle-lookup setup use hash sets rather than ordered
    sets
  - hosted compile timings drop materially even if the warm suite time stays
    roughly flat
  Notes:
  - semantic fragment seeding now walks the scope chain once into four shared
    hash sets, and `template_angle_lookup::NameSetLookup` now stores those same
    hash sets directly
  - validation/timing after this stage:
    - warm `verify-fast-pa10-31`: `28.67s`
    - `cppast` `<algorithm>`: `2.84s`
    - `cppast` `<array>`: `2.91s`
    - `cpphostcompat -c` `<algorithm>`: `34.22s`
    - `cpphostcompat -c` `<array>`: `34.93s`

- [x] Stage 4: Replace eager fragment scope snapshots with lazy lookup and
  reduce repeated placeholder text scans.
  Why:
  - after Stage 3, the hosted profile moved into
    `semantic_fragment_parser::collect_visible_fragment_names(...)` and
    `Analyzer::text_mentions_template_placeholders(...)`
  - both were doing repeated full-scope or full-string work on every fragment
    parse
  Success criteria:
  - fragment reparsing no longer pre-collects visible scope names into
    temporary hash sets
  - scoped template-angle parsing uses a lazy scope-backed lookup instead of
    rebuilding `NameSetLookup` snapshots
  - placeholder text probes scan identifier tokens once per text instead of
    rescanning the whole string for every candidate name
  - full validation and timing reruns pass
  Notes:
  - `semantic_fragment_parser` now attaches a scope-backed lazy lookup directly
    to `CppAstParser` fragment parses and to scoped template-angle parsing
  - `CppAstParser` stack-backed lookups now fall back to an external
    `NameLookup` when present, so fragment reparsing keeps local parser scopes
    while consulting semantic scope lazily
  - the main placeholder/name-mention helpers in `callsemantic.cpp` now do a
    one-pass identifier-token extraction per text
  - validation/timing after this stage:
    - warm `verify-fast-pa10-31`: `27.06s`
    - `cppast` `<algorithm>`: `2.86s`
    - `cppast` `<array>`: `2.91s`
    - `cpphostcompat -c` `<algorithm>`: `7.30s`
    - `cpphostcompat -c` `<array>`: `7.46s`

- [ ] Stage 5: Re-profile and trim second-order tokenizer/location costs.
  Why:
  - after Stage 4, the dominant parser-specific scope/placeholder churn should
    be largely gone
  - the remaining next-tier hotspots are likely to be
    `Normalizer::operator++`, `PPTokenizer::get`, `consume_identifier`,
    `BufferedIterator::Buffer::push_back`, and `SourceLocationTable::add`
  Success criteria:
  - re-profiled hotspot list is updated
  - at least one second-order tokenizer/location hotspot is reduced without
    regressing correctness
  - full validation and timing reruns pass

## Deferred Post-HHC Work

The remaining hosted slowness is no longer purely parser-side. Do not chase
these performance issues while the hosted-header frontier is still open.
Keep them deferred until the HHC queue is cleared and self-hosting has been
reached, then resume from the measurements and hotspot notes below.

### Valarray Semantic Hotspot

Measured on `2026-03-21` against:

```cpp
#include <valarray>
```

- Clang `-std=c++11 -fsyntax-only`: `1.01s`
- `env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ ./dev/cppast ...`:
  `4.69s`
- `env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ ./dev/cpphostcompat -c ...`:
  timed out at `30.02s`

The hosted compile slowdown here is not a repeat of the earlier parser stall.
Parser-only work is finite, while the hosted path spends most of its sampled
time in semantic lookup / instantiation.

Privileged `sample` output from `/tmp/valarray.sample.txt` shows the main hot
stacks are:

- `Analyzer::lookup_type(...)`
- `Analyzer::lookup_type_impl(...)`
- `cpp_decl::parse_decl_spec_ast(...)`
- `cpp_decl::parse_parameter_clause_ast(...)`
- `semantic_class_model::populate_class_info(...)`
- `semantic_class_model::collect_class_simple_declaration(...)`
- `template_instantiation::instantiate_class_template(...)`
- `template_resolution::resolve_template_arguments(...)`
- smaller but still relevant:
  - `Analyzer::parse_template_id_string_scoped(...)`
  - `template_argument_semantics::resolve_type_argument_text(...)`
  - `Analyzer::text_mentions_template_placeholders(...)`

That means the next performance program after the hosted frontier should target
semantic type lookup, repeated class population, and repeated template
instantiation/completion reuse, not more parser cleanup.
