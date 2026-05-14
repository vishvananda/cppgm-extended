# Memoization / Caching Analysis

## Question

After the recent algorithmic reductions, is there still room for significant
performance gains from memoization / caching?

Short answer:

- yes, but not from broad “cache everything” semantic memoization
- the best remaining opportunities are narrow, repeated, pure operations
- the highest-confidence target is fragment parsing

This analysis uses the current `main` after:

- [semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
  constructor and ordinary-overload replay reductions
- [STRING_OSTREAM_HOTSPOT_ANALYSIS.md](/Users/vishvananda/cppgm/legacy/STRING_OSTREAM_HOTSPOT_ANALYSIS.md)

Measured workload:

- `dev/src/template_audit.cpp`

with:

```sh
CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
env CPPGM_SEMANTIC_HOTSPOT=1 \
    CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY='*' \
    CPPGM_SEMANTIC_HOTSPOT_DUMP_FRAGMENT='*' \
    ./dev/cpphostcompat -c dev/src/template_audit.cpp -o /tmp/template_audit.o
```

## Current Baseline

- `template_audit.cpp` wall time: about `16.10s`
- `query_requests=236270`
- `fragment_requests=35445`
- `fragment_parses=35445`
- `query_dump matches=20116`
- `fragment_dump matches=1538`

Important implication:

- fragment requests and fragment parses are still identical
- so exact repeated fragment texts are being reparsed every time
- there is no meaningful fragment parse cache today

## What The Data Says

### 1. Fragment parse caching has the clearest remaining upside

The strongest repeated fragment rows are:

- `1081` parses of `class std::__1::basic_ostream<_CharT, _Traits>`
- `436` parses of `std::__1::basic_string<char,...>const`
- `335` parses of `std::__1::basic_string<char,...>const const&`
- `549` parses of `__void_t<_Op<_Args...>>`
- `590` parses of `_Cp::__storage_type`
- `288` parses each of `__static_bounded_iter<_Iterator,_Size>`,
  `__bounded_iter<_Iterator>`, and `__wrap_iter<_It>`

And the global ratio is:

- `35445` fragment requests
- only `1538` distinct dumped fragment keys

So the best-case upper bound for an exact fragment cache is eliminating about
`33907` fragment reparses on this file, roughly `95.7%` of fragment parses.

That does **not** mean `95.7%` wall-time improvement, because parsing is only
part of the total compile cost. But it does mean fragment parsing is still a
large repeated-work reservoir.

### 2. `resolve_template_arguments(...)` is a strong cache candidate

Summed query volume:

- `resolve_template_arguments total = 30105`

Top exact repeats:

- `2595` `params=1 texts=1 [char]`
- `2126` `params=1 texts=1 [_Tp]`
- `1828` `params=2 texts=2 [_CharT,_Traits]`
- `1657` `params=3 texts=3 [char,std::__1::char_traits<char>,std::__1::allocator<char>]`
- `1140` `params=1 texts=1 [allocator_type]`
- `1019` each for `[_T1,_U1]` and `[_T2,_U2]`

This is attractive because:

- the operation is text/substitution oriented
- the exact same parameter/text combinations recur hundreds to thousands of times
- many of the repeated fragment parses above are driven by this path

This is the best semantic-result cache after fragment parsing.

### 3. Function-template deduction caching could help, but it is more complex

Summed query volume:

- `deduce_function_template_arguments total = 1368`
- `operator<<` subset alone = `924`

Repeated exact rows include:

- `352` `operator<<(ostream&, const basic_string& )`
- `308` `operator<<(ostream&, basic_string )`
- `264` `operator<<(ostream&, array of 2 const char)`

This suggests real repeated deduction work still exists even after the replay
fixes. A deduction-result cache keyed by:

- template declaration identity
- explicit template arguments, if any
- canonical argument types
- argument value categories
- relevant use-scope identity / generation

could be worthwhile.

But this is more dangerous than fragment caching because the deduction result is
more context-sensitive and interacts with lookup state.

### 4. Some hot query counts are already mostly “cached hits”

Example:

- `reference_class_template_instantiation total = 16625`
- `reference_class_template_instantiation_hit total = 13246`

And exact rows like:

- `1648` `basic_string<char,...>`
- `1699` `basic_string<char,...>` hit
- `1280` `allocator<char>`
- `1280` `allocator<char>` hit
- `1279` `char_traits<char>`
- `1279` `char_traits<char>` hit

This means we are already reusing the instantiated class results. The remaining
cost here is mostly the repeated lookup / surrounding work that leads to those
hits, not the lack of an instantiation cache.

So this area is **not** the first place to add more caching.

### 5. `complete_class_type(...)` is hot, but a generic cache is not the best answer

Summed query volume:

- `complete_class_type total = 34387`

Top rows are mostly obvious non-class cases:

- `3694` `unsigned long int`
- `3112` `char`
- `2709` `<null-type>`
- `1549` `int`
- `1397` `bool`
- `1227` `pointer to const char`

This is a real volume problem, but a general cache is awkward because:

- some named types can transition from incomplete to complete
- a broad negative cache risks hiding those state transitions

The safer fix here is:

- add earlier fast-path guards for obvious never-class kinds
- avoid calling `complete_class_type(...)` when the caller only wants a cheap
  “could this be a class?” screening check

So this should stay an algorithm/callsite cleanup target, not a primary cache
project.

## Recommendation Order

### High confidence, high value

1. Fragment parse cache

Safe version:

- key by `(fragment kind, raw text)`
- cache the parsed AST prototype
- return a cloned AST on each use instead of sharing the same mutable nodes

Why first:

- strongest measured repetition
- lowest semantic risk
- directly attacks `fragment_requests == fragment_parses`

### Medium-high confidence, high value

2. `resolve_template_arguments(...)` result cache

Safe version:

- key by parameter-list identity/signature
- substituted text list
- a scope generation / binding-generation token

Why second:

- enormous repeat counts
- drives many repeated fragment parses
- should compose well with the fragment cache

### Medium confidence, moderate value

3. `deduce_function_template_arguments(...)` result cache

Safe version:

- key by template identity
- explicit args
- canonical arg types and categories
- relevant use-scope generation

Why third:

- repeated exact operator deduction traffic is real
- but the correctness surface is much larger than fragment parsing

## What Not To Do First

- Do not start with broad expression-analysis memoization.
  The semantic layer has too many stateful edges for that to be a safe first
  cache.

- Do not start with another cache around class-template instantiation hits.
  That area already shows hit-heavy reuse.

- Do not treat `complete_class_type(...)` counts as proof that a cache there is
  the best win.
  The dominant problem is probably overcalling, not recomputing.

## Expected Outcome

Would caching achieve significant gains?

- yes, likely

Most credible source of a meaningful win:

- fragment parse caching

Most credible follow-on:

- `resolve_template_arguments(...)` memoization

Most likely combined result:

- a real additional speedup on hosted-heavy files like
  [template_audit.cpp](/Users/vishvananda/cppgm/dev/src/template_audit.cpp)
- without depending on fragile whole-expression semantic caches

## Practical Next Step

If we decide to implement caching, the right next experiment is:

1. add an exact fragment parse cache with clone-on-return
2. rerun `template_audit.cpp`
3. compare:
   - wall time
   - `fragment_requests`
   - `fragment_parses`
4. only then decide whether `resolve_template_arguments(...)` memoization is
   still worth the added complexity
