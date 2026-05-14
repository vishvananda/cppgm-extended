# String/Ostream Hotspot Analysis

## Scope

This note tracks the remaining semantic hot path on the hosted string / ostream
surface after the earlier `map::insert` reductions.

The working reducers are:

- `/tmp/string_header.cpp`
- `/tmp/ostream_header.cpp`
- `/tmp/ostream_string1_clean.cpp`
- `/tmp/ostream_string2_clean.cpp`
- `/tmp/ostream_string3_clean.cpp`

All measurements below use:

```sh
CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

and the current `./dev/cpphostcompat` binary.

## Baseline

Before the ordinary overload fix, the string/ostream ladder looked like:

- `string_header`: `query_requests=30249`, `fragment_requests=4563`
- `ostream_header`: `55528`, `8344`
- `ostream_string1_clean`: `56810`, `8438`
- `ostream_string2_clean`: `57622`, `8485`
- `ostream_string3_clean`: `58434`, `8532`

The important signature was the stable per-extra-operand jump in the ostream
chain:

- `string1 -> string2`: about `+812 query_requests`, `+47 fragment_requests`
- `string2 -> string3`: about `+812 query_requests`, `+47 fragment_requests`

Repeated-node tracing on `ostream_string2_clean.cpp` showed the same user AST
being revisited several times:

- `std::string("payload01")` call-expression at `3:41`
- literal at `3:64`

That put the remaining replay on the ordinary function-call overload path rather
than the constructor path.

## Root Cause

`semantic_overload::analyze_call_expression(...)` still analyzed source
arguments inside the per-candidate loop for regular function overload
resolution.

That meant a chain like:

```cpp
emit_chain() << std::string("payload00") << std::string("payload01");
```

was re-running the same untargeted source-argument analysis for the later
`std::string(...)` operand across multiple `operator<<` candidates, even though
constructors and function-template candidate building already had a generic
source-argument cache for the same pattern.

## Fix

The reduction landed in:

- [semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)

Change:

- add a generic `CachedArgAnalysis` cache in ordinary function-call overload
  resolution
- reuse untargeted source-argument analysis across candidates
- still use target-aware analysis when the parameter actually requires it
  (`initializer`, braced-init, function-id target, lambda-closure target)

So the fix is structural:

- generic source analysis is done once per argument node
- candidate-specific work only starts at conversion / ranking time

## Results

After the fix:

- `string_header`: `30200`, `4563`
- `ostream_header`: `55457`, `8344`
- `ostream_string1_clean`: `56307`, `8438`
- `ostream_string2_clean`: `56687`, `8485`
- `ostream_string3_clean`: `57067`, `8532`

New per-extra-operand cost:

- `string1 -> string2`: about `+380 query_requests`, `+47 fragment_requests`
- `string2 -> string3`: about `+380 query_requests`, `+47 fragment_requests`

This is a material drop from the old `+812/+47` pattern.

## Proof That The Old Replay Is Gone

The exact repeated-node trace on `ostream_string2_clean.cpp` no longer reports
the user `std::string("payload01")` call-expression as a repeated node.

So the old issue:

- same user operand AST reanalyzed several times inside one `operator<<` chain

is no longer present after the fix.

## Remaining Delta: `string1 -> string2`

Top query deltas from `ostream_string1_clean` to `ostream_string2_clean` are now
small and broad:

- `+17` `resolve_template_arguments [params=2 texts=2 [_CharT,_Traits]]`
- `+15` `reference_class_template_instantiation basic_ostream<_CharT,_Traits>`
- `+15` `reference_class_template_instantiation_hit basic_ostream<_CharT, _Traits>`
- `+4` each on several builtin `complete_class_type(...)` probes
  (`int`, `bool`, `float`, `long int`, `unsigned int`, `unsigned long int`,
  `unsigned long long int`, `long double`)
- `+2` `enable_if<_Bp,_Tp>`
- `+2` `basic_string_view<_CharT,_Traits>`
- `+2` `pointer to const char`
- `+2` `char`

This does **not** look like the previous algorithmic replay. It looks like:

- one extra ostream/string operand introducing a smaller amount of real new work
- plus a broad background cost from repeated non-class type completion probes

## Broader Impact

Real timing improved as well:

- `dev/src/template_audit.cpp`: about `17.49s -> 16.10s`
- `/tmp/string_header.cpp`: about `7.97s` on the current binary

Validation:

- `make verify-fast-pa10-31-nobuild CXX=/usr/local/opt/llvm/bin/clang++ VERIFY_ASSIGNMENT_JOBS=6`
  passed

## Conclusion

This pass removed the remaining obvious ordinary-overload replay on the
string/ostream surface.

What remains is:

- a smaller per-operand cost in `basic_ostream` / string template references
- a very broad volume of `complete_class_type(...)` checks on obviously
  non-class types

Those are better candidates for the next performance pass and for evaluating
whether targeted memoization would buy meaningful additional speed.
