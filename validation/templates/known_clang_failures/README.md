# Known Clang Template Failures

These are small expected-pass sources that isolate a patched-Clang reference
divergence. They are deliberately outside `validation/templates/tests`, so the
normal Clang-oracle bank does not mistake an upstream compiler failure for a
CPPGM conformance test.

## PA22 combined template-template and template-id pack ordering

`pa22-p3310-cwg1432-combined-pack-order.cpp` reduces the only adjudicated
cross-oracle result to an expected-pass `static_assert`. Separate diagnostic
controls establish that Clang 22 selects fixed arity when either ordering
dimension occurs alone:

- a template-template parameter pack versus fixed arity selects fixed;
- a nested trailing template-argument pack versus fixed arity selects fixed;
- combining those same two ordering relations should still select fixed.

Clang 15 through 19, Apple Clang 17, GCC 15, and CPPGM accept the source.
Upstream Clang 20 through 22 and the patched Clang 23 reject its assertion.
Clang 20 accepts it with
`-fno-relaxed-template-template-args`, pinning the behavior change to LLVM
PR 124137's P3310/CWG2398 template-template matching rewrite.

The repository targets the N3485 C++11 rules, so the PA22 runtime and LowIR
contract retain the pre-P0522 fixed-arity selection. Reference generation
hash-checks the source and raw patched-Clang rendering, applies the exact
specialization-binding correction declared in
`witness_cross_oracle_adjudications.json`, verifies the corrected hash, and
writes the corrected text as the adjacent `.ref.witness`. The comparison
harness then performs its normal literal comparison without an exception.

Run the reproducer with:

```sh
/usr/local/opt/llvm/bin/clang++ -std=c++11 -pedantic-errors -fsyntax-only \
  validation/templates/known_clang_failures/pa22-p3310-cwg1432-combined-pack-order.cpp
```

LLVM references:

- https://github.com/llvm/llvm-project/pull/124137
- https://github.com/llvm/llvm-project/pull/129436
- https://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1432
- https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3310r5.html
