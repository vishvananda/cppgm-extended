# Map Insert Pair Hotspot Analysis

## Goal

Reduce the semantic-layer explosion seen while compiling hosted STL code, using
`std::map` insertion as a small reproducer for the broader `template_audit.cpp`
slowdown.

This note captures:

- the reduced cases
- the traces that identify the hot path
- the hypotheses that were tested
- what the current evidence supports

The note keeps older passes for context. The authoritative latest numbers are in
`## Current Baseline` and `## Current First-Insert Reduction`.

## Tooling Used

The current reusable tooling for this investigation is:

- `scripts/map_pair_hotspot_ladder.sh`
- filtered hotspot dumps in `dev/src/semantic_hotspot.cpp`
  - `CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY`
  - `CPPGM_SEMANTIC_HOTSPOT_DUMP_FRAGMENT`
  - `CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT`
- exact repeated-query tracing in `dev/src/semantic_hotspot.cpp`
  - `CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY`
  - `CPPGM_SEMANTIC_HOTSPOT_TRACE_LIMIT`
- generic class-population probes in `dev/src/semantic_class_model.cpp`
  - `populate_class_reference_members`
  - `ensure_class_reference_members`
  - `populate_class_info`

All runs below used:

```sh
CXX=/usr/local/opt/llvm/bin/clang++
CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Reduction Ladder

Pair-filtered hotspot totals from `scripts/map_pair_hotspot_ladder.sh`:

| case | query_requests | fragment_requests | pair_matches | pair_total |
| --- | ---: | ---: | ---: | ---: |
| `map_header` | 99080 | 19910 | 56 | 823 |
| `map_alias` | 105645 | 21448 | 280 | 1731 |
| `map_value_type` | 100930 | 20246 | 122 | 1281 |
| `map_decl` | 105830 | 21414 | 318 | 1854 |
| `map_find` | 107334 | 21745 | 332 | 2064 |
| `pair_decl` | 99788 | 20039 | 60 | 904 |
| `make_pair` | 100037 | 20105 | 61 | 941 |
| `pair_convert_lvalue` | 101365 | 20434 | 70 | 1276 |
| `pair_is_constructible_lvalue` | 100035 | 20185 | 61 | 923 |
| `map_subscript` | 105825 | 21414 | 317 | 1848 |
| `map_insert_value_type` | 114881 | 22836 | 399 | 4882 |
| `map_insert_lvalue_pair` | 115906 | 23074 | 412 | 5085 |
| `map_insert` | 116140 | 23096 | 415 | 5096 |

## What The Reduction Proves

### 1. The explosion is real in a small user-level reproducer

The `map_insert_*` cases are much smaller than `template_audit.cpp`, but they
still show a sharp pair-related jump:

- `map_decl`: `pair_total=1854`
- `map_find`: `pair_total=2064`
- `map_insert_value_type`: `pair_total=4882`
- `map_insert_lvalue_pair`: `pair_total=5085`

So `std::map<int, int>::insert(...)` is a valid reduced reproducer for the same
kind of hosted semantic blowup.

### 2. The main multiplier is not just `pair<int,int>` -> `pair<const int,int>`

This was the first theory, but the reduction falsified it.

Evidence:

- direct conversion:
  - `pair_convert_lvalue`: `pair_total=1276`
- standalone trait check:
  - `pair_is_constructible_lvalue`: `pair_total=923`
- map insert with already-correct value type:
  - `map_insert_value_type`: `pair_total=4882`
- map insert with `pair<int, int>`:
  - `map_insert_lvalue_pair`: `pair_total=5085`

The mismatch from `pair<int, int>` to `pair<const int, int>` only accounts for a
small delta:

- `map_insert_lvalue_pair - map_insert_value_type = +203 pair_total`

So the pair-conversion mismatch is real, but it is not the dominant multiplier.

### 3. The main multiplier is not `__find_equal` by itself

`map_find` already exercises `__find_equal`, including its pair return type:

- `map_find`: `pair_total=2064`
- `map_insert_value_type`: `pair_total=4882`

So `__find_equal` contributes pair traffic, but the big step up happens in the
generic insert/emplace path.

## Call Stack Analysis

### Direct pair conversion stack

For:

```cpp
std::pair<int, int> p(1, 2);
std::pair<const int, int> q(p);
```

Tracing
`CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY='pair(struct std::__1::pair<int, int>/0)'`
shows:

```text
analyze_function_binding_output [main]
  analyze_statement [simple-declaration]
    analyze_expression_for_target [ -> struct std::__1::pair<int const, int>]
      semantic_expression::analyze_expression_for_target [ -> struct std::__1::pair<int const, int>]
        try_argument_conversion [target lvalue-reference to const struct std::__1::pair<int const, int>]
          select_constructor_from_exprs [std::__1::pair<int const, int>, args=1]
            deduce_function_template_arguments [pair, args=1]
```

This is a real cost center, but it is only the local pair-conversion core.

### `map::insert(_Pp&&)` stack

For:

```cpp
std::map<int, int> m;
std::pair<int, int> p(1, 2);
m.insert(p);
```

Tracing the same pair query shows:

```text
analyze_function_binding_output [main]
  analyze_statement [expression-statement]
    analyze_expression [] at map_insert_lvalue_pair.cpp:5:3
      semantic_expression::analyze_expression [] at map_insert_lvalue_pair.cpp:5:3
        analyze_call_expression [] at map_insert_lvalue_pair.cpp:5:3
          deduce_function_template_arguments [insert, args=1]
            lookup_type [__enable_if_t<is_constructible<std::__1::pair<int const,int>,std::__1::pair<int,int>&>::value,int>]
              resolve_template_arguments [params=2, texts=2]
                ensure_class_reference_members [std::__1::is_constructible<...>]
                  populate_class_reference_members [std::__1::is_constructible<...>]
                    lookup_type [integral_constant<bool,__is_constructible(_Tp,_Args...)>]
                      reference_class_template_instantiation [integral_constant, args=2]
                        resolve_template_arguments [params=2, texts=2]
                          try_argument_conversion [target struct std::__1::pair<int const, int>]
                            select_constructor_from_exprs [std::__1::pair<int const, int>, args=1]
                              deduce_function_template_arguments [pair, args=1]
```

This shows the direct pair-conversion core embedded inside the insert SFINAE
path, but this stack alone does **not** explain the full multiplier.

## Corrected Theory

The larger replay is caused by the generic `map::insert(_Pp&&)` / `__tree`
insertion machinery reopening many pair-related semantic subproblems, not by one
single repeated `pair(pair<int,int>)` deduction.

The concrete high-level path is:

```text
map::insert(_Pp&&)
  -> __tree::__emplace_unique(_Args&&...)
    -> std::__try_key_extraction<key_type>(with_key, without_key, ...)
      -> __try_key_extraction_impl(...)
      -> __find_equal(...)
      -> pair<iterator, bool> / pair<__end_node_pointer, __node_base_pointer&>
      -> __check_pair_construction / pair constructors
```

This path creates several overlapping pair-heavy semantic workloads:

- forwarding and remove-reference resolution for `_Pp`
- key-extraction helper instantiation
- lambda closure types inside `__emplace_unique`
- `__find_equal` return-pair types
- `pair<iterator, bool>` construction checks
- direct `value_type` conversion viability

## Proof Against The Earlier Narrow Theory

### Narrow theory

The first theory was:

- the blowup is mostly repeated
  `deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]`
- and that replay is mostly coming from standalone
  `is_constructible<pair<const int,int>, pair<int,int>&>`

### Why that theory is wrong

#### A. Standalone `is_constructible` is cheap

```text
pair_is_constructible_lvalue: pair_total=923
map_insert_lvalue_pair:      pair_total=5085
```

So the standalone trait check is nowhere near large enough.

#### B. Exact-value insert is still almost as expensive

```text
map_insert_value_type:  pair_total=4882
map_insert_lvalue_pair: pair_total=5085
delta: +203
```

If the mismatch from `pair<int,int>` to `pair<const int,int>` were the main
problem, that delta would be much larger.

#### C. The exact pair-deduction query is not the main multiplier

Exact dump of
`pair(struct std::__1::pair<int, int>/0)`:

- direct pair conversion: `count=90`
- `m.insert(p)`: `count=45`

So `insert` is **not** simply repeating that one query more often than direct
construction. The extra cost is spread across a broader set of pair-related
queries.

## The Strongest Insert-Specific Deltas

Diffing `map_insert_value_type` against `map_decl` shows the largest new pair
queries are:

```text
+1077 ensure_class_reference_members [__tree<value_type,...> complete=yes ref_members=yes]
+242  resolve_template_arguments [params=1 texts=1 [std::__1::pair<int const,int>&]]
+162  ensure_class_reference_members [__tree<...>::::::__lambda1]
+158  ensure_class_reference_members [__tree<...>::::::__lambda2]
+150  complete_class_type [struct std::__1::pair<int const, int>]
+109  ensure_class_reference_members [std::__1::pair<int const, int>]
+88   ensure_class_reference_members [pair<tree_iterator, bool>]
+87   ensure_class_reference_members [pair<map_iterator, bool>]
+84   try_argument_conversion [target pair<int const,int> expr pair<int const,int>]
+68   ensure_class_reference_members [pair<__end_node_pointer, __node_base_pointer&>]
+60   deduce_function_template_arguments [pair(pair<__end_node_pointer, __node_base_pointer&>/1)]
+48   deduce_function_template_candidate [__find_equal ... returning pair<__end_node_pointer, __node_base_pointer&>]
+36   deduce_function_template_arguments [__find_equal(lvalue-reference to pair<int const, int>/0)]
```

## Current Cold-Path State

After the later steady-state reductions, the current small reproducer numbers
for the same-pair insert path are:

| case | node_visits | query_requests | fragment_requests |
| --- | ---: | ---: | ---: |
| `map_decl` | 16030 | 70826 | 14067 |
| `map_insert_samepair_once` | 17265 | 75626 | 14847 |
| `map_insert_samepair_twice` | 17276 | 75668 | 14855 |

So the current deltas are:

- cold `map_decl -> one insert`: `+4800 query_requests`, `+780 fragment_requests`
- steady `one insert -> two inserts`: `+42 query_requests`, `+8 fragment_requests`

This means the remaining work is mostly in the first insert. The steady-state
replay is already small enough that it is no longer the main owner.

### Proven cold-path improvement

One safe reduction was to parse inline class member definitions in
reference-only mode while collecting signatures:

- `collect_class_method_definition(...)`
- `collect_class_friend_function_definition(...)`
- the function-declarator path in `collect_class_friend_declaration(...)`

Those sites now call `parse_function_definition_base(..., true)` and
`parse_declarator(..., true)` when they are only collecting class member
signatures.

This was proven with a tuple-focused trace. Before that change, the first insert
path was reopening:

```text
lookup_type [std::map<int,int>]
  instantiate_class_template [map, args=2]
    populate_class_info [std::__1::map<...>]
      class_member_function_definition [map:1152:3]
        lookup_type [pair<iterator,bool>]
          instantiate_class_template [pair, args=2]
            populate_class_info [std::__1::pair<...>]
```

and then immediately paying for the tuple/array-heavy member templates in
`pair.h`.

After the change, that `map` inline return-type path disappears from the tuple
trace. The remaining tuple-heavy cold path starts from the user declaration
itself:

```text
analyze_function_binding_output [main]
  analyze_statement [simple-declaration]
    lookup_type [std::pair<int,int>]
      instantiate_class_template [pair, args=2]
        populate_class_info [std::__1::pair<int, int>]
```

So the current evidence is that the next cold-path owner is eager full class
population of `std::pair<int, int>` for the user declaration, not the old
`map::insert` inline return-type path.

### Safe insert-only cleanup

One additional safe reduction is to mark synthetic lambda closures as already
having reference members once they have been fully materialized.

That change is in `synthesize_lambda_closure_class(...)` in
[callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp): after
`ensure_implicit_special_members(...)`, `finalize_class_virtuals(...)`, and
`finalize_class_layout(...)`, the closure now sets
`reference_members_collected = true`.

Why this is safe:

- these closure classes are synthesized directly, not parsed from a class AST
- they are already complete by the time the flag is set
- repeated later `ensure_class_reference_members(...)` calls were pure no-op
  traffic

Measured effect on the reduced case:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| baseline `map_insert_samepair_once` | 75626 | 14847 |
| after synthetic lambda fix | 75446 | 14847 |
| baseline `map_insert_samepair_twice` | 75668 | 14855 |
| after synthetic lambda fix | 75488 | 14855 |

So this does not reduce fragment work, but it does remove `180` repeated query
requests from the insert-only path without changing the fast-verify baseline.

### Exact non-template constructor shortcut

The larger current reduction is in
[semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp):

- after collecting ordinary constructor candidates, both constructor-selection
  entrypoints now check for a unique best candidate whose argument conversions
  are all `CR_EXACT`/`CR_ELLIPSIS`
- if such a winner exists, the compiler returns it immediately instead of
  deducing every constructor template for the same class

This is structural, not memoization:

- the old path always instantiated constructor templates even when overload
  resolution had already found a unique exact non-template winner
- the new path skips that unnecessary template-deduction work entirely

Measured effect on the reduced initial-insert path:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| `map_decl` before | 70826 | 14067 |
| `map_decl` after | 61402 | 11351 |
| `map_pair_decl` before | 71324 | 14150 |
| `map_pair_decl` after | 61832 | 11428 |
| `map_insert_samepair_once` before | 75446 | 14847 |
| `map_insert_samepair_once` after | 64760 | 11886 |
| `map_insert_samepair_twice` before | 75488 | 14855 |
| `map_insert_samepair_twice` after | 64775 | 11888 |

That means:

- cold `map_pair_decl -> one insert` dropped from `+4122 / +697` to
  `+2928 / +458`
- steady `one insert -> two inserts` dropped from `+42 / +8` to `+15 / +2`

The biggest visible effect in the query dump is that the insert-only helper
pair-construction churn collapses:

- `deduce_function_template_arguments [pair(iterator, bool)]`
- `deduce_function_template_arguments [pair(pair<__end_node_pointer,...>)]`

Those were previously being paid even when an exact non-template constructor had
already won the overload.

## Current Steady-State Reduction

The next useful reduction target was not the first `insert`, but the *second*
identical `m.insert(p)` in the same translation unit.

### Why the second insert is useful

The first insert still pays a large one-time hosted-library expansion cost. The
second insert isolates the repeated per-call work better.

Before the current pass:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| one insert | 78381 | 15191 |
| two inserts | 78459 | 15205 |
| delta `1 -> 2` | +78 | +14 |

Tracing
`CPPGM_SEMANTIC_HOTSPOT_TRACE_QUERY='pair(struct std::__1::pair<int, int>/0)'`
showed five repeated pair-constructor deductions, including both of these

## Current Baseline

After the steady-state reductions and the current first-insert passes, the
active reduced-case numbers are:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| `map_subscript` | 55889 | 8256 |
| `map_insert_value_type_now` | 58086 | 8567 |
| `map_insert_samepair_once` | 58379 | 8626 |
| `map_insert_samepair_twice` | 58391 | 8628 |

So the current deltas are:

- cold `map_subscript -> map_insert_value_type_now`: `+2197 query_requests`,
  `+311 fragment_requests`
- cold `map_subscript -> map_insert_samepair_once`: `+2490 query_requests`,
  `+370 fragment_requests`
- steady `one insert -> two inserts`: `+12 query_requests`,
  `+2 fragment_requests`

The steady-state path is still effectively flat. The remaining meaningful work
is almost entirely in the *first* insert.

## Current First-Insert Reduction

The current cold-path reduction now has three distinct passes.

### 1. Lambda-body reuse

The first pass was the lambda-body reuse path in:

- [semantic_expression.cpp](/Users/vishvananda/cppgm/dev/src/semantic_expression.cpp)
- [semantic_statement.cpp](/Users/vishvananda/cppgm/dev/src/semantic_statement.cpp)
- [semantic_output.cpp](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp)
- [semantic_model.h](/Users/vishvananda/cppgm/dev/src/semantic_model.h)

### What the trace showed

Node tracing on the first insert path showed that the same `__tree` call site
inside `__emplace_unique` was being analyzed twice:

- once while preparing the implicit-return lambda used by
  `__try_key_extraction_impl(...)`
- again when outputting the synthesized lambda `operator()`

That made the first insert pay for the lambda body twice even though the body
was already fully analyzed during return-type deduction.

### What changed

For simple lambdas only, the compiler now:

- analyzes the lambda compound body once while collecting return expressions
- caches the resulting `CallSemNode` body on the synthesized `FunctionBinding`
- reuses that body during later output instead of rewalking the statements

This is intentionally narrow. The cache is only enabled when:

- captures are empty, or exactly `[this]`
- the body does not contain label-like control flow:
  `labeled_statement`, `goto_statement`, `case_statement`, or
  `default_statement`

The control-flow guard was required because `pa32/tests/compile/639-lambda-goto-return-deduction.t`
regressed with an early `malformed labeled-statement` failure when the cached
path was applied too broadly.

### Measured effect

| case | query_requests before | query_requests after | fragment_requests before | fragment_requests after |
| --- | ---: | ---: | ---: | ---: |
| `map_subscript` | 58937 | 58937 | 8762 | 8762 |
| `map_insert_value_type_now` | 61730 | 61311 | 9087 | 9077 |
| `map_insert_samepair_once` | 62027 | 61604 | 9146 | 9136 |
| `map_insert_samepair_twice` | 62039 | 61616 | 9148 | 9138 |

So this pass removes about `420` query requests and `10` fragment requests from
the cold insert path without changing the steady-state `+12 / +2` delta.

### 2. Narrow the class-object special-init probe

The next trace showed that the same `__construct_node(...)` call inside
`__tree::__emplace_unique` was still being revisited while lowering this local
declaration:

```cpp
__node_holder __h = __construct_node(std::forward<_Args>(__args2)...);
```

The specific repeated node was:

- `/usr/local/Cellar/llvm/22.1.0/bin/../include/c++/v1/__tree:1043:33`

The stack led through
[semantic_lifetime.cpp](/Users/vishvananda/cppgm/dev/src/semantic_lifetime.cpp),
where local class-object initialization was doing a full
`analyze_expression_for_target(...)` probe on **every** initializer just to see
whether it lowered to a special closure/init-list node. That probe is only
useful for:

- lambda expressions
- braced-init-lists

For an ordinary call expression like `__construct_node(...)`, the result was
discarded and the real constructor-action path analyzed the same initializer
again.

So the probe was narrowed to only those payload forms that can actually produce
`closure_object` or `initializer_list_object`.

Measured effect of that pass:

| case | query_requests before | query_requests after | fragment_requests before | fragment_requests after |
| --- | ---: | ---: | ---: | ---: |
| `map_subscript` | 58844 | 58844 | 8756 | 8756 |
| `map_insert_value_type_now` | 61311 | 61141 | 9077 | 9069 |
| `map_insert_samepair_once` | 61604 | 61434 | 9136 | 9128 |
| `map_insert_samepair_twice` | 61616 | 61446 | 9138 | 9130 |

Most importantly, the exact repeated-node trace moved from:

- `__tree:1043:33` visit count `3`

to:

- `__tree:1043:33` visit count `2`

That proved one whole cold-path replay leg was being paid in the lifetime probe.

### 3. Reuse generic constructor source-argument analysis across candidates

After the lifetime probe trim, the same `__tree:1043:33` node was still visited
twice. The remaining replay was in
[semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp):
[select_constructor](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp).

For constructor selection from AST argument nodes, the code was doing:

- `ctx.analyze_expression(scope, *arg_nodes[j])`

once per constructor candidate whenever target-aware analysis was not required.
For `__node_holder __h = __construct_node(...)`, that meant the same source call
expression was reanalyzed for multiple `unique_ptr` constructor candidates even
though its source semantics were candidate-independent.

The fix was to factor that generic source-argument analysis out of the candidate
loop and reuse it across candidates. This is the same kind of structural
reduction already used in `append_function_template_call_candidates(...)`, not a
fragment-layer cache.

Measured effect of that pass:

| case | query_requests before | query_requests after | fragment_requests before | fragment_requests after |
| --- | ---: | ---: | ---: | ---: |
| `map_subscript` | 58844 | 55889 | 8756 | 8256 |
| `map_insert_value_type_now` | 61141 | 58086 | 9069 | 8567 |
| `map_insert_samepair_once` | 61434 | 58379 | 9128 | 8626 |
| `map_insert_samepair_twice` | 61446 | 58391 | 9130 | 8628 |

The strongest proof is that the repeated-node trace for
`__tree:1043:33` disappeared entirely. After this pass, that specific
`__construct_node(...)` call is no longer a repeated-node hotspot.

### Net effect of the current cold-path passes

Relative to the earlier first-insert baseline:

| case | query_requests before | query_requests now | fragment_requests before | fragment_requests now |
| --- | ---: | ---: | ---: | ---: |
| `map_subscript` | 58937 | 55889 | 8762 | 8256 |
| `map_insert_value_type_now` | 61311 | 58086 | 9077 | 8567 |
| `map_insert_samepair_once` | 61604 | 58379 | 9136 | 8626 |
| `map_insert_samepair_twice` | 61616 | 58391 | 9138 | 8628 |

So the current first-insert work has removed:

- `3225` query requests and `510` fragment requests from `map_insert_value_type_now`
- `3225` query requests and `510` fragment requests from `map_insert_samepair_once`
- `3048` query requests and `506` fragment requests from `map_subscript`

while preserving the steady-state `+12 / +2` replay slope.

The broader hosted timings also improved materially:

- `#include "semantic_model.h"`: about `20.17s -> 16.72s`
- `dev/src/template_audit.cpp`: about `20.68s -> 17.49s`

### Validation

These cold-path passes were validated with:

- the reduced first-insert hotspot reducers above
- direct rerun of
  `pa32/tests/compile/639-lambda-goto-return-deduction.t`
- `CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`

At this point the remaining first-insert growth looks much more distributed.
There is no longer an obvious single repeated-node owner comparable to the
earlier `__tree:1043:33` replay.

## Steady-State Insert Replay

To separate first-use STL expansion from true per-call replay, the next step was
to compare one, two, and three identical inserts in the same TU:

```cpp
std::map<int, int> m;
std::pair<int, int> p(1, 2);
m.insert(p);
m.insert(p);
m.insert(p);
```

### Baseline

Before any targeted pruning:

| case | query_requests | fragment_requests | pair_matches | pair_total |
| --- | ---: | ---: | ---: | ---: |
| `1 insert` | 115906 | 23074 | 412 | 5085 |
| `2 inserts` | 116539 | 23185 | 416 | 5264 |
| `3 inserts` | 117172 | 23296 | 423 | 5450 |

So the steady-state replay was:

- `1 -> 2`: `+633 query_requests`, `+111 fragment_requests`, `+179 pair_total`
- `2 -> 3`: `+633 query_requests`, `+111 fragment_requests`, `+186 pair_total`

That proved the problem was not only first-use expansion. Each extra `insert`
was replaying a stable chunk of pair-related work.

### First targeted pruning: impossible constructor-template candidates

The first algorithmic fix was in constructor candidate selection:

- reject constructor templates whose arity cannot match the current source args
- for one-arg constructor templates, reject obviously impossible template-base
  mismatches before running full deduction
  - for example, do not re-run tuple/array pair-converting constructor
    deduction against a `pair<int, int>` source argument

After that pruning:

| case | query_requests | fragment_requests | pair_matches | pair_total |
| --- | ---: | ---: | ---: | ---: |
| `1 insert` | 113306 | 22080 | 412 | 4879 |
| `2 inserts` | 113775 | 22119 | 416 | 5032 |
| `3 inserts` | 114244 | 22158 | 423 | 5192 |

The key proof from the `1 -> 2` diff was:

```text
before: +30 deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]
after:   +4 deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]
```

So the constructor-template pruning was real and useful, but it did not explain
the whole steady-state replay.

### Second targeted fix: bound type lookup before ordinary type lookup

Tracing the remaining replay showed many extra:

```text
ensure_class_reference_members [std::__1::pair<int const, int> complete=yes ref_members=yes]
```

The trace stack for those calls was important:

```text
deduce_function_template_arguments [insert, args=1]
  ...
  try_argument_conversion [target struct std::__1::pair<int const, int>]
    select_constructor_from_exprs [std::__1::pair<int const, int>, args=1]
      deduce_function_template_arguments [pair, args=1]
        lookup_type [std::__1::pair<_U1,_U2>]
          resolve_template_arguments [params=2, texts=2]
            lookup_type [_U1]
              ensure_class_reference_members [std::__1::pair<int const, int>]
```

That showed the remaining replay was happening while resolving bound template
names like `_U1` / `_U2` inside pair constructor patterns. The lookup route was
doing ordinary `lookup_type(...)` before trying `lookup_bound_type_by_text(...)`,
which meant it re-entered class-scope lookup and re-ensured already-known
`pair<...>` reference members.

Reordering that path to prefer:

1. local dependent placeholder
2. bound type text
3. ordinary type lookup

produced a much larger improvement:

| case | query_requests | fragment_requests | pair_matches | pair_total |
| --- | ---: | ---: | ---: | ---: |
| `1 insert` | 106399 | 20593 | 404 | 3721 |
| `2 inserts` | 106777 | 20632 | 408 | 3784 |
| `3 inserts` | 107155 | 20671 | 415 | 3854 |

The new steady-state replay became:

- `1 -> 2`: `+378 query_requests`, `+39 fragment_requests`, `+63 pair_total`
- `2 -> 3`: `+378 query_requests`, `+39 fragment_requests`, `+70 pair_total`

This is the strongest proof in the investigation so far:

- original `1 -> 2`: `+179 pair_total`
- after constructor pruning: `+153 pair_total`
- after bound-type lookup reordering: `+63 pair_total`

So the remaining replay was not just constructor candidate churn. A large part
of it was lookup-order churn around already-bound template names.

## Current Remaining Steady-State Delta

After both fixes, the top `1 -> 2` pair-related deltas are:

```text
+12 ensure_class_reference_members [std::__1::pair<int const, int>]
+6  complete_class_type [struct std::__1::pair<int, int>]
+6  deduce_function_template_arguments [insert(struct std::__1::pair<int, int>/0)]
+5  select_constructor_from_exprs [std::__1::pair<int const, int>(struct std::__1::pair<int, int>/0)]
+4  deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]
+4  ensure_class_reference_members [std::__1::pair<int, int>]
+4  ensure_implicit_special_members [std::__1::pair<int, int>]
+4  resolve_dependent_named_type_text_fallback [struct std::__1::pair<_U1, _U2>]
```

So the next remaining per-insert cost is much narrower than the original
problem. The dominant replay is now small repeated pair completeness /
special-member / constructor-selection work, not the earlier large template-id
lookup churn.

## Broader Case Impact

The reduced `map::insert` case improved sharply, but the larger hosted files only
improved modestly:

| case | earlier | after both fixes |
| --- | ---: | ---: |
| `#include "semantic_model.h"` | `23.45s` | `22.57s` |
| `dev/src/template_audit.cpp` | `24.02s` | `23.77s` |

That means these fixes are real, but they only cover one portion of the broader
hosted STL slowdown.

## Next Dominant Hotspot After These Fixes

With the map/pair replay reduced, the top `template_audit.cpp` hotspot shifted
back toward the already-known `basic_string` / `enable_if` surface:

```text
count=7832 resolve_dependent_named_type_text_fallback [typename _Tp]
count=4057 ensure_class_reference_members [std::__1::enable_if<0, int>]
count=2589 resolve_template_arguments [params=1 texts=1 [char]]
count=2281 resolve_template_arguments [params=1 texts=1 [_Tp]]
count=2265 complete_class_type [basic_string<char, char_traits<char>, allocator<char>>]
count=2105 resolve_template_arguments [params=2 texts=2 [_Bp,_Tp]]
count=2105 reference_class_template_instantiation [enable_if<_Bp,_Tp>]
```

So the next likely algorithmic target is no longer `map::insert` pair traffic.
It is the repeated `basic_string` / `enable_if` / `_Tp` resolution churn in the
broader hosted header surface.

This is the clearest current evidence:

- the insert blowup is dominated by repeated activity around `__tree`,
  `__try_key_extraction`, `__find_equal`, and pair return/iterator forms
- the pair-conversion core is part of that, but not the whole story

## Secondary Delta: What The `pair<int,int>&` Mismatch Adds

Diffing `map_insert_lvalue_pair` against `map_insert_value_type` isolates the
extra cost of inserting `pair<int, int>` instead of `value_type`:

```text
+242 resolve_template_arguments [params=1 texts=1 [std::__1::pair<int,int>&]]
+95  complete_class_type [struct std::__1::pair<int, int>]
+83  try_argument_conversion [target lvalue-reference to struct std::__1::pair<int, int>]
+61  ensure_class_reference_members [std::__1::pair<int, int>]
+45  ensure_class_reference_members [std::__1::pair<int const, int>]
+45  deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]
+44  try_argument_conversion [target lvalue-reference to struct std::__1::pair<int, int> expr=lvalue-reference ...]
+41  try_argument_conversion [target rvalue-reference to struct std::__1::pair<int, int>]
```

This confirms that the source-pair mismatch adds work, but only as a secondary
layer on top of the much larger generic insert replay.

## Current Best Theory

The current best explanation is:

1. `map::insert(_Pp&&)` enters the generic `__tree::__emplace_unique` path.
2. That path instantiates and analyzes `__try_key_extraction` helpers, lambdas,
   `__find_equal`, and several pair-return / pair-construction helper types.
3. Those helpers repeatedly reopen dependent member/type lookups against the same
   fully complete `__tree<value_type,...>` specialization and related pair forms.
4. The result is not one giant repeated query, but a broad replay fan-out across:
   - `ensure_class_reference_members`
   - `resolve_template_arguments`
   - `complete_class_type`
   - `try_argument_conversion`
   - pair constructor deduction and `__find_equal` deduction

This is an algorithmic replay problem in the semantic pipeline, not a fragment
cache problem.

## What This Suggests For The Fix

The next fix should target the structural replay in the `map::insert` /
`__tree::__emplace_unique` / `__try_key_extraction` path, not generic memoization.

The highest-value places to inspect next are:

- repeated dependent member/type lookup on already-complete `__tree<...>`
- repeated helper/lambda re-entry in `__try_key_extraction`
- repeated reconstruction of pair-return helper types from `__find_equal`
- unnecessary reopening of pair-construction checks for already-known
  `value_type`-shaped arguments

The data above does **not** support treating standalone `is_constructible` or one
specific pair constructor deduction query as the single root cause.

## Current State On `main`

After the later hosted-overload replay reductions on `main` (`7a82c9d`), the
same reduced steady-state case is now:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| `1 insert` | `78381` | `15191` |
| `2 inserts` | `78459` | `15205` |
| `3 inserts` | `78537` | `15219` |

So the current replay slope is:

- `1 -> 2`: `+78 query_requests`, `+14 fragment_requests`
- `2 -> 3`: `+78 query_requests`, `+14 fragment_requests`

That is materially better than the earlier checkpoints recorded above:

- old `current4`: `+84 / +14`
- old `current5`: `+82 / +14`
- current `current6`: `+78 / +14`

The structural changes that produced this latest step were:

- skip rematerializing already-materialized exact winning-call conversions
- reject impossible ordinary function-template candidates by arity before
  running argument analysis and deduction

The second change matters because `insert(p)` was still paying for obviously
impossible one-argument template candidates. After the arity gate, the extra
steady-state `insert(...)` deduction noise dropped again.

### Current Remaining Delta

Diffing `2 inserts` against `1 insert` at the current `current6` state gives:

```text
+10 resolve_template_arguments [params=2 texts=2 [int const,int]]
+10 reference_class_template_instantiation [__check_pair_construction<int const,int>]
+10 reference_class_template_instantiation_hit [__check_pair_construction<int const, int>]
+5  select_constructor_from_exprs [std::__1::pair<int const, int>(struct std::__1::pair<int, int>/0)]
+5  deduce_function_template_arguments [pair(struct std::__1::pair<int, int>/0)]
+2  resolve_template_arguments [params=2 texts=2 [_U1,_U2]]
+2  resolve_template_arguments [params=2 texts=2 [_T1,_U1]]
+2  resolve_template_arguments [params=2 texts=2 [_T2,_U2]]
+2  reference_class_template_instantiation [is_constructible<_T1,_U1>]
+2  reference_class_template_instantiation [is_constructible<_T2,_U2>]
```

So the replay has narrowed further. The remaining steady-state cost is now
mostly the `pair<int,int> -> pair<int const,int>` constructibility path,
especially repeated references to `__check_pair_construction<int const,int>`,
not the broader overload churn that was present earlier in the investigation.

### Broader Hosted Timing At This Point

With the current `main` state:

| case | cppgm |
| --- | ---: |
| `#include "semantic_model.h"` | `20.17s` |
| `dev/src/template_audit.cpp` | `20.68s` |

So the steady-state fixes are real, but there is still a large remaining hosted
compile-time gap. The next likely algorithmic target is the repeated
`__check_pair_construction<int const,int>` / pair-constructor viability path.

## Follow-Up Steady-State Pass

One more steady-state cleanup on top of that point removed an extra dead branch
without changing the overall `query_requests` totals much.

The change was:

- reject impossible ordinary-expression conversion attempts to
  `std::initializer_list<T>` earlier, both in general user-defined conversion
  probing and in the cached overload-screening conversion path

This specifically removed the stale `insert(initializer_list<value_type>)`
screening branch from the reduced `m.insert(p)` case. Before the change, the
filtered pair dump still contained:

```text
target=lvalue-reference to const class std::initializer_list<std::__1::pair<int const, int>> expr=struct std::__1::pair<int, int>/0 allow_ud=no
```

After the change, that line is gone from the reduced `2 inserts` / `3 inserts`
pair dump. The remaining per-call replay is just the direct
`pair<int,int> -> pair<int const,int>` viability path:

```text
try_argument_conversion [target lvalue-reference to const struct std::__1::pair<int const, int> expr=struct std::__1::pair<int, int>/0 allow_ud=no]
try_argument_conversion [target rvalue-reference to struct std::__1::pair<int const, int> expr=struct std::__1::pair<int, int>/0 allow_ud=no]
select_constructor_from_exprs [std::__1::pair<int const, int>(const int/0)]
```

The gross hotspot totals stayed essentially flat:

| case | query_requests | fragment_requests |
| --- | ---: | ---: |
| `1 insert` | `64898` | `11925` |
| `2 inserts` | `64913` | `11927` |
| `3 inserts` | `64910` | `11929` |

So this pass did not reveal another hidden steady-state explosion. At this
point the second/third identical insert cost is dominated by the real
`pair<int,int>` to `map::value_type` constructibility check, not by extra dead
overload branches.

## Earlier Initial-Insert Pass

The next successful cut was broader than `pair` itself.

### What changed

Two structural changes were useful:

- exact non-dependent `__libcpp_remove_reference_t<T&>` now resolves directly
  instead of going through alias-template instantiation
- bare bound placeholder names like `typename _Tp` no longer fall through into
  text rewrite + reparse when the bound type is still dependent

The second change was the important one. The first one only gave a small local
improvement.

### Why this was the right target

The filtered query dump for `map_insert_value_type` showed that the biggest
remaining cold-start traffic was not pair-constructor work. It was repeated
dependent fallback on the same placeholder text:

```text
1259 resolve_dependent_named_type_text_fallback [typename _Tp]
```

with the hottest frames under:

- `reference_class_template_instantiation [atomic, args=1]`
- `reference_class_template_instantiation [unique_ptr, args=2]`
- `reference_class_template_instantiation [array, args=2]`
- `reference_class_template_instantiation [iterator_traits, args=1]`

That meant we were repeatedly reparsing plain bound type placeholders even when
we already had the bound `TypePtr`.

### Before / After

Values below are `query_requests / fragment_requests`.

| case | previous | current |
| --- | --- | --- |
| `map_subscript` | `61560 / 11389` | `58941 / 8766` |
| `map_insert_value_type` | `64586 / 11856` | `61818 / 9175` |
| `map_insert_samepair_once` | `64898 / 11925` | `62115 / 9234` |
| `map_insert_samepair_twice` | `64913 / 11927` | `62127 / 9236` |

### What this proves

1. The large hosted replay was mostly not pair-only work.
   The biggest win came from removing generic dependent-type fallback churn.

2. The steady-state repeated insert cost is now effectively flat.
   After this pass:
   - `2 inserts - 1 insert = +12 query_requests, +2 fragment_requests`

3. There is still real first-insert-specific work left.
   The current cold-start gap is:
   - `map_insert_value_type - map_subscript = +2877 query_requests`
   - `map_insert_value_type - map_subscript = +409 fragment_requests`

### Supporting targeted dumps

After this pass:

- the exact same-pair query dropped to:
  - `resolve_template_arguments [params=1 texts=1 [std::__1::pair<int const,int>&]] = 128`
- the remaining `__libcpp_remove_reference_t` dump is now mostly generic
  dependent `_Tp` traffic, not the exact `pair<int const,int>&` case
- `resolve_dependent_named_type_text_fallback [typename _Tp]` is no longer a
  top repeated semantic query in the reduced map runs

So the broad placeholder fallback cut is the first pass that materially reduced
both baseline hosted cost and the `map::insert` cold-start reproducer.
