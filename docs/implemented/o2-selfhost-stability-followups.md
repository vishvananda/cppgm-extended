# O2 Self-Host Stability Follow-Ups

## Purpose

This document is the central place for the correctness-first patches and
temporary guardrails added while bringing `cppgm++ -O2` back to a working
self-host state.

It exists so these changes do not silently become permanent just because they
made the current system stable enough to keep moving.

Use this doc when:

- debugging the current `PA37` / `PA3` `-O2` self-host failures
- deciding whether a current restriction is still needed
- planning the order for re-enabling stronger optimization once the backend is
  robust enough

## Current Context

The immediate debugging target is the self-host-built `pa3` binary under
`cppgm++ -O2`, starting from the `300-triple.t` workload and the reduced
one-line `ctrlexpr` smoke that aborts during `Calculator` teardown when the
system is broken.

Several fixes in this lane are ordinary correctness fixes and should remain.
Others are intentionally conservative guardrails that should be revisited once
the backend has real CFG-aware liveness and better merge-aware value handling.

## Current Patch Inventory

### 1. Object-backed local storage uses real `obj<...>` slots

Files:

- [dev/src/lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp)
- [dev/src/lowir_internal.cpp](/Users/vishvananda/cppgm/dev/src/lowir_internal.cpp)
- [pa13/lowir.md](/Users/vishvananda/cppgm/pa13/lowir.md)
- [pa13/README.md](/Users/vishvananda/cppgm/pa13/README.md)
- [pa13/tests/spec/293-object-slot-smoke.t](/Users/vishvananda/cppgm/pa13/tests/spec/293-object-slot-smoke.t)

What changed:

- object-backed locals, hidden object temporaries, and initializer-list backing
  storage stopped using repeated `i64` placeholder slots and now use a single
  `obj<bytesxalign>` slot
- LowIR parsing/validation now accepts direct object-typed slots

Why it landed:

- the earlier malformed storage shape produced bad hosted object layout and
  invalid runtime behavior before optimization quality was even the issue

Removal intent:

- none
- this is a real correctness fix, not a temporary optimization guardrail

### 2. Cross-block available-expression reuse now includes pointer results

File:

- [dev/src/lowir_optimizer.cpp](/Users/vishvananda/cppgm/dev/src/lowir_optimizer.cpp)

What changed:

- cached expressions with result type `ptr` are again eligible to survive
  block-boundary expression-cache meets

Why it landed:

- the earlier restriction was a temporary guardrail while pointer live-range
  handling and merge behavior were still being stabilized
- after the backend CFG-aware temp-interval tranche and pointer-slot-promotion
  re-enablement landed, both `make test-pa37` and the fresh self-host `PA3`
  `-O2` `300-triple.t` compare stayed green with pointer results allowed across
  block boundaries again

Removal intent:

- none
- this follow-up is complete and should remain in place

### 3. O2 slot promotion now includes pointer-typed slots

File:

- [dev/src/lowir_optimizer.cpp](/Users/vishvananda/cppgm/dev/src/lowir_optimizer.cpp)

What changed:

- O2 promotable-slot analysis again allows eligible non-escaping `ptr` slots

Why it landed:

- the earlier pointer-slot restriction was a temporary guardrail while backend
  temp liveness was still block-local
- after the CFG-aware temp-interval tranche landed, self-host `PA3` `-O2`
  returned to a clean `300-triple.t` compare with pointer slot promotion
  enabled again

Removal intent:

- none
- this follow-up is complete and should remain in place

### 4. Backend temp intervals are now CFG-aware across blocks and loops

File:

- [dev/src/lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)

What changed:

- temp interval construction now computes block successors plus block-level
  `use` / `def` / `live_in` / `live_out`, then extends linear intervals across
  blocks where a temp is live
- the earlier "used outside defining block stays in frame storage" restriction
  was removed

Why it landed:

- the earlier machine-IR temp allocator used a linear position interval model
  that was not CFG-aware
- values defined before a loop and reused in the loop header can be treated as
  dead before calls in the loop body, then assigned caller-saved registers and
  clobbered on the next iteration
- the reduced `std::deque<CalcToken>::~deque()` failure showed exactly this:
  the cached `end()` value survived the first iteration, then came back
  corrupted on the second loop trip because the allocator did not model the
  backedge/call interaction correctly

Removal intent:

- none
- this follow-up is complete and should remain in place

### 5. Explicit function-boundary `nothrow` metadata only uses cheap forms

Files:

- [dev/src/callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- [dev/src/cppast_parser.cpp](/Users/vishvananda/cppgm/dev/src/cppast_parser.cpp)
- [dev/src/semantic_output.cpp](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp)

What changed:

- ordinary function declarators now retain parsed `noexcept(...)` AST under the
  existing `function_qualifier` node
- explicit source `nothrow` metadata emission now treats only cheap forms as
  authoritative:
  - `noexcept`
  - `throw()`
  - non-empty `throw(...)` / dynamic exception specs as explicitly throwing
  - trivial boolean `noexcept(...)` expressions such as `true`, `false`,
    parenthesized forms, unary `!`, and boolean `&&` / `||` combinations of
    those same trivial forms
- general `noexcept(expr)` semantic evaluation is intentionally skipped for
  emitted function-boundary metadata

Why it landed:

- the first metadata slice made heavy hosted compiles dramatically slower even
  at `-O0`
- on `pa33/tests/compile/655-const-unordered-map-find.t`, enabling general
  explicit `noexcept(expr)` evaluation exploded semantic hotspot counts from
  roughly `108k` query requests / `53k` fragment requests to roughly `707k`
  query requests / `452k` fragment requests
- narrowing metadata collection back to cheap explicit forms restored the
  hotspot counts and wall time to the frozen `main` baseline

Removal intent:

- revisit once semantic exception-spec evaluation has a genuinely cheap path
  with stronger caching or non-text-based trait evaluation
- until then, prefer compile throughput and self-host stability over fully
  modeling arbitrary explicit `noexcept(expr)` on emitted LowIR function
  metadata

## Intended Re-Enablement Order

The tracked guardrails in this note are now either permanent correctness fixes
or completed removals. Keep using the validation lane below if a future
optimizer/backend change reopens the same self-host path.

## Validation To Re-Run When Relaxing These Guards

- direct one-line `ctrlexpr` smoke: `printf '1\n' | <self-host ctrlexpr -O2 binary>`
- `pa3/tests/300-triple.t` output and timing comparison for self-host `-O0`
  versus self-host `-O2`
- `make test-pa37`
- the active `PA37` self-host ladder checkpoint that currently depends on the
  same backend/runtime path

## Working Rule

Do not remove one of these restrictions just because a smaller repro stops
crashing. Remove them only after:

1. the actual backend or optimizer invariant is restored
2. the direct self-host repro is green
3. the broader `PA37` self-host and `PA35` optimizer validation lane still
   passes
