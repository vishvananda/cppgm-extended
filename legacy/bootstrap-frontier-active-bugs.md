# Bootstrap Frontier Active Bugs

This document records the currently active self-host/bootstrap bugs on `main`
while we work them down without repeatedly rerunning the full long self-host
compile.

Current policy for this queue:

- keep the existing `obj/pa37/selfhost` tree warm
- do not start a full `make bootstrap-self-sweep` or broad `pa37` rebuild
  while one of these targeted bugs is still being reduced
- land source-level backports and earliest-owner regressions first where
  practical
- only rerun the long self-host compile after the current targeted queue is
  exhausted or materially narrowed

## Strategy And Acceptance Rubric

Use these rules to keep the frontier honest:

- a fix must preserve the failing source shape
  - replacing `ifstream`/`getline` with `FILE*`/`fgets`, removing templates,
    or otherwise steering the source away from the failing library/codegen path
    is a workaround, not a fix
- a reduction is good when it keeps the same compiler phase and same failure
  family while removing unrelated library or project structure
- host-owner tests are only safety checks for a candidate source change
  - they do not prove the selfhost frontier is fixed
- a bucket closes only when the same kind of source that failed before now works
  with the self-built binary, or the frontier has clearly moved past it

Use this rubric before a full selfhost rebuild:

- rebuild only after at least one of these is true:
  - we have 2 or more source-side fixes staged across active buckets
  - we have one fix that clearly targets the earliest failing bucket and one
    more bucket has been materially narrowed by reduction
  - the current earliest frontier has been reduced as far as practical and
    further progress needs fresh selfhost artifacts
- do not rebuild just to validate a source rewrite that bypasses the failing
  path
- do not rebuild while the current queue still contains an un-reduced earlier
  crash family that is likely to mask later buckets again

## Selfhost Root-Cause Rule

When `cppgm++` and `cppgm++-self` behave differently on the same source, treat
that as a compiler divergence, not as a source-site problem.

Use this three-level model:

1. test/source level
   - the original failing test case or a reduced source that preserves the same
     failure family
2. self-compile spot
   - the compiler source file that happens to misbehave in the self-built
     binary, for example `callsemantic.cpp` or `lowirgensemantic.cpp`
3. underlying compiler level
   - the bug in `cppgm++` that compiled the self binary incorrectly and created
     the host vs self divergence

Debugging rule:

- do not patch level 1 just to avoid the failure
- do not patch level 2 just to make the current self-built binary limp past the
  bad code shape
- fix level 3, then rebuild, then verify that the original level-1 source now
  behaves the same under `cppgm++` and `cppgm++-self`

What counts as a real fix:

- reduce the divergence to the smallest source that still passes with
  `cppgm++` and fails with `cppgm++-self`
- identify the compiler stage where the self binary diverges from the host
  binary
- patch the compiler implementation that created the bad self-built artifact
- add a regression in the earliest owning PA for the reduced source shape
- only keep source-site changes when they are independently correct even under
  a correctly built compiler, not when they merely hide the divergence

Current examples:

- `static_assert(is_same<decltype(a), int>::value, ...)`
  - host rewrites `decltype(a)` to `int`
  - self keeps it as a dependent type and leaves the `static_assert`
    unevaluated
  - the bug to fix is the underlying type-text/decltype resolution path in the
    compiler, not the test and not `callsemantic.cpp`
- `pa27` catch-body EH clear drift
  - host and self produce the same semantics
  - only the self-built binary emits extra `eh_clear_*` blocks after a
    terminated catch body
  - the bug to fix is the compiler/codegen divergence that compiled
    `lowirgensemantic.cpp` incorrectly, not the PA27 test and not the
    catch-lowering source shape itself

## Current Stage Map

Latest targeted `pa37` selfhost status on current `main`:

- PASS:
  - `pa1` through `pa18`
  - `pa20` through `pa26`
  - `pa17`
  - `pa23`
  - `pa24`
  - `pa25`
  - `pa28`
  - `pa29`
- not yet rerun on current `main` after the latest selfhost fixes:
  - `pa19`
  - `pa27`
  - `pa30`
  - `pa31`
  - `pa32`
- older active later-stage evidence still stands until rerun:
  - `pa33` ->
    [542-local-functor-std-function-assignment.t](/Users/vishvananda/cppgm/pa33/tests/compile/542-local-functor-std-function-assignment.t)
  - `pa34` ->
    [651-hosted-unordered-map-string-int-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/651-hosted-unordered-map-string-int-link-smoke.t)
  - isolated spot check:
    - `pa31` [130-static-archive.t](/Users/vishvananda/cppgm/pa31/tests/spec/130-static-archive.t)
      passes on the current staged snapshot, so the older `pa31` evidence is
      stale pending a full rerun

## Current Queue

### BF-001: early `cppgm++-self --emit-types` crash family

- Status: completed
- First failing checkpoint:
  - `make -C pa37 test-pa11 CXX=../dev/cppgm++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- Earliest failing test:
  - [100-empty.t](/Users/vishvananda/cppgm/pa11/tests/spec/100-empty.t)
- Current evidence:
  - the current sweep shows pervasive `Segmentation fault: 11` in `pa11`
  - earlier hardened-malloc work on the same checkpoint hit
    `malloc_error_break` during a destructor path that appeared to tear down
    `semantic_cache::SemanticCache`, but that turned out to be a downstream
    symptom rather than the root cause
  - current tiny reduction:
    - a `Derived : Pad, Base` source with a free function taking `Base &`
      makes `cppgm++-self --emit-types` segfault immediately
    - an even smaller empty-base variant also reproduces:
      `struct Base {}; struct Derived : Base {}; int main() { Derived d; }`
    - the same selfhost binary succeeds on `--emit-semantics` and
      `--emit-lowir` for that source
  - the root cause is now reduced and traced:
    - implicit synthesized class members for anonymous-namespace classes were
      being unconditionally upgraded to `SL_WEAK` in
      [semantic_class_model.cpp](/Users/vishvananda/cppgm/dev/src/semantic_class_model.cpp),
      overriding their earlier correct `internal` classification
    - `bootstrap_trace_report.py --preset linkage` on
      `/tmp/bf001-anon-implicit-dtor.cpp` showed the same anonymous-namespace
      constructor/destructor first classified as `internal` and later
      reclassified `weak`
    - object-symbol proof on the reduced source showed
      `weak external (anonymous namespace)::Analyzer::{Analyzer,~Analyzer}`
      before the fix
    - a standalone two-TU reduction with different anonymous-namespace
      `Analyzer` layouts then linked and aborted at runtime with the wrong
      destructor shape
  - current source-side fix:
    - synthesized class-member entry points now preserve anonymous-namespace
      internal linkage instead of being blindly upgraded to `weak`
  - validating host-side evidence:
    - reduced linkage trace now stays `internal` for the implicit ctor/dtor
    - reduced object output now shows `non-external` ctor/dtor
    - the standalone two-TU collision repro now exits `0`
  - tracked owner regression:
    [208-anon-namespace-implicit-special-member-linkage.t](/Users/vishvananda/cppgm/pa31/tests/spec/208-anon-namespace-implicit-special-member-linkage.t)
  - the latest deliberate selfhost rebuild/recheck is clean: `pa11` passes
- Intended debug direction:
  - closed

### BF-008: `cppgm++-self --emit-semantics` using-declaration call/binding family

- Status: completed
- Affected checkpoints:
  - `pa12`
- Earliest failing tests:
  - [186-using-declaration-call.t](/Users/vishvananda/cppgm/pa12/tests/spec/186-using-declaration-call.t)
  - [216-block-scope-using-declaration.t](/Users/vishvananda/cppgm/pa12/tests/spec/216-block-scope-using-declaration.t)
- Current evidence:
  - the latest staged keep-going rerun passes `pa12` cleanly under
    [cppgm++-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/cppgm++-self)
- Intended debug direction:
  - closed

### BF-009: `cppgm++-self --emit-lowir` constructor/member-init/assignment/decltype family

- Status: completed
- Affected checkpoints:
  - `pa15`
  - `pa16`
- Earliest failing tests:
  - [230-aliased-base-mem-initializer-match.t](/Users/vishvananda/cppgm/pa15/tests/spec/230-aliased-base-mem-initializer-match.t)
  - [255-derived-pointer-member-init.t](/Users/vishvananda/cppgm/pa15/tests/spec/255-derived-pointer-member-init.t)
  - [319-proxy-subscript-assignment.t](/Users/vishvananda/cppgm/pa16/tests/spec/319-proxy-subscript-assignment.t)
- Current evidence:
  - the underlying host-compiler/backend bug is now fixed in
    [lowir_machine_ir.cpp](/Users/vishvananda/cppgm/dev/src/lowir_machine_ir.cpp)
    and owned by
    [740-direct-branch-source-live-across-call.t](/Users/vishvananda/cppgm/pa23/tests/structural/740-direct-branch-source-live-across-call.t)
  - after rebuilding the staged
    [cppgm++-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/cppgm++-self),
    all previously tracked targeted failures in this bucket pass again:
    - [230-aliased-base-mem-initializer-match.t](/Users/vishvananda/cppgm/pa15/tests/spec/230-aliased-base-mem-initializer-match.t)
    - [255-derived-pointer-member-init.t](/Users/vishvananda/cppgm/pa15/tests/spec/255-derived-pointer-member-init.t)
    - [319-proxy-subscript-assignment.t](/Users/vishvananda/cppgm/pa16/tests/spec/319-proxy-subscript-assignment.t)
    - [322-overloaded-deref-user-assignment.t](/Users/vishvananda/cppgm/pa16/tests/spec/322-overloaded-deref-user-assignment.t)
    - [336-decltype-value-category.t](/Users/vishvananda/cppgm/pa16/tests/spec/336-decltype-value-category.t)
    - [353-implicit-move-constructor-moveonly-member.t](/Users/vishvananda/cppgm/pa16/tests/spec/353-implicit-move-constructor-moveonly-member.t)
- Intended debug direction:
  - closed

### BF-002: `cppgm++-self --emit-lowir` template/deduction/current-specialization family

- Status: completed
- Affected checkpoints:
  - `pa18`
  - `pa19`
  - `pa20`
  - `pa21`
  - `pa22`
  - `pa26`
  - `pa27`
  - `pa30`
- Earliest failing tests:
  - [191-basic-template-operator-overloads.t](/Users/vishvananda/cppgm/pa18/tests/spec/191-basic-template-operator-overloads.t)
  - [147-qualified-function-template-call.t](/Users/vishvananda/cppgm/pa19/tests/spec/147-qualified-function-template-call.t)
  - [265-constexpr-decltype-qualified-static-member-lookup.t](/Users/vishvananda/cppgm/pa20/tests/spec/265-constexpr-decltype-qualified-static-member-lookup.t)
  - [135-variable-template-constant-lookup.t](/Users/vishvananda/cppgm/pa21/tests/spec/135-variable-template-constant-lookup.t)
  - [179-alias-template-pointer-cv-cache-distinction.t](/Users/vishvananda/cppgm/pa22/tests/spec/179-alias-template-pointer-cv-cache-distinction.t)
  - [143-template-converting-constructor-call-argument.t](/Users/vishvananda/cppgm/pa26/tests/spec/143-template-converting-constructor-call-argument.t)
  - [208-distinct-lambda-member-template-types.t](/Users/vishvananda/cppgm/pa27/tests/spec/208-distinct-lambda-member-template-types.t)
  - [303-runtime-virtual-member-pointer-multiple-inheritance.t](/Users/vishvananda/cppgm/pa30/tests/spec/303-runtime-virtual-member-pointer-multiple-inheritance.t)
- Current evidence:
  - the backend rebuild that closed `BF-009` also cleared much of the earlier
    selfhost template band:
    - [191-basic-template-operator-overloads.t](/Users/vishvananda/cppgm/pa18/tests/spec/191-basic-template-operator-overloads.t): pass
    - [265-constexpr-decltype-qualified-static-member-lookup.t](/Users/vishvananda/cppgm/pa20/tests/spec/265-constexpr-decltype-qualified-static-member-lookup.t): pass
    - [135-variable-template-constant-lookup.t](/Users/vishvananda/cppgm/pa21/tests/spec/135-variable-template-constant-lookup.t): pass
    - [179-alias-template-pointer-cv-cache-distinction.t](/Users/vishvananda/cppgm/pa22/tests/spec/179-alias-template-pointer-cv-cache-distinction.t): pass
    - [142-conversion-operator-call-argument.t](/Users/vishvananda/cppgm/pa26/tests/spec/142-conversion-operator-call-argument.t): pass
  - the remaining targeted active owners were:
    - [147-qualified-function-template-call.t](/Users/vishvananda/cppgm/pa19/tests/spec/147-qualified-function-template-call.t)
    - [208-distinct-lambda-member-template-types.t](/Users/vishvananda/cppgm/pa27/tests/spec/208-distinct-lambda-member-template-types.t)
    - [303-runtime-virtual-member-pointer-multiple-inheritance.t](/Users/vishvananda/cppgm/pa30/tests/spec/303-runtime-virtual-member-pointer-multiple-inheritance.t)
  - current minimal reduced repro:
    - `/tmp/template_non_token.cpp`
    - source shape: `stream << 1 << 2` with a free function template
      `operator<<`
    - host `cppgm++` compiles this normally
    - selfhost `cppgm++-self` does not select the overloaded operator and
      falls through to builtin shift analysis, ending with
      `ERROR: unsupported shift operands`
    - one `<<` is fine; two chained template calls are enough to reproduce
    - non-template free `operator<<`, member `operator<<`, and non-template
      operand variants all pass under the same selfhost binary
  - host trace on the same reduced source shows the expected path:
    - one visible `operator<<` template candidate
    - one instantiated `operator<<` binding for `int`
    - no ambiguity or fallback
  - the latest `pa27/208` and `pa30/303` LLDB traces both land in the same
    self-only dependent-type resolution path:
    - `Analyzer::append_dependent_type_resolution_cache_key(...)`
    - `Analyzer::try_dependent_type_resolution_cache_key(...)`
    - `Analyzer::resolve_instantiated_dependent_type(...)`
    - then `semantic_overload::append_function_template_call_candidates(...)`
  - `pa30/303` current source shape is:
    `template <class T> int invoke(C&, int (T::*)());`
    with calls like `invoke(c, &A::foo)` and `invoke(c, &B::foo)`
  - host vs self live trace on `pa30/303` is stronger than the abort site:
    - host instantiates `invoke` with stable member-pointer parameter text like
      `member-pointer of struct A to function of () returning int`
    - self logs
      `resolve-instantiated-dependent-type invalid-null key=N:class C:1`
      before deduction completes
    - self then emits `template-drift` with corrupted current signature text
      for `invoke`, including garbage bytes inside the member-pointer owner type
  - the real root cause was lower than template deduction:
    - LowIR cleanup registration for local class objects stored raw
      `destructor_action` nodes and re-resolved them later through the current
      `bindings_` map
    - on early return before inner scopes were popped, outer cleanups for a
      shadowed name rebound to the inner shadow storage instead of the original
      outer object
    - that duplicated inner destruction and skipped the real outer object
      cleanup, which then corrupted later state in tests like `pa19/147`
  - minimal reduced owner repro:
    - nested block-scope class locals with shadowed names and an early return
    - host semantic output is clean
    - emitted LowIR previously destroyed the shadowed inner locals twice on the
      return path
  - fixed in
    [lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp)
    by registering local-object cleanup against the storage pointer at cleanup
    registration time instead of storing a raw semantic node
  - new owning regression:
    [359-shadowed-local-cleanup-rebind-on-return.t](/Users/vishvananda/cppgm/pa16/tests/spec/359-shadowed-local-cleanup-rebind-on-return.t)
  - after rebuilding staged `pa37` at `O0`:
    - [147-qualified-function-template-call.t](/Users/vishvananda/cppgm/pa19/tests/spec/147-qualified-function-template-call.t)
      matches the refreshed ref
    - [208-distinct-lambda-member-template-types.t](/Users/vishvananda/cppgm/pa27/tests/spec/208-distinct-lambda-member-template-types.t)
      matches the refreshed ref
    - [303-runtime-virtual-member-pointer-multiple-inheritance.t](/Users/vishvananda/cppgm/pa30/tests/spec/303-runtime-virtual-member-pointer-multiple-inheritance.t)
      passes again under the staged `pa30` harness
  - the only textual fallout from this fix was ref drift in:
    - [147-qualified-function-template-call.ref](/Users/vishvananda/cppgm/pa19/tests/spec/147-qualified-function-template-call.ref)
    - [208-distinct-lambda-member-template-types.ref](/Users/vishvananda/cppgm/pa27/tests/spec/208-distinct-lambda-member-template-types.ref)
    - both now use the captured cleanup storage pointer directly rather than
      rebuilding `&name` at cleanup emission time
  - reduced owner regression added:
    [193-template-operator-shift-two-step.t](/Users/vishvananda/cppgm/pa18/tests/spec/193-template-operator-shift-two-step.t)
  - host owner checks pass for:
    - [193-template-operator-shift-two-step.t](/Users/vishvananda/cppgm/pa18/tests/spec/193-template-operator-shift-two-step.t)
    - [191-basic-template-operator-overloads.t](/Users/vishvananda/cppgm/pa18/tests/spec/191-basic-template-operator-overloads.t)
    - [192-template-operator-shift-stress-chain.t](/Users/vishvananda/cppgm/pa18/tests/spec/192-template-operator-shift-stress-chain.t)
    - [147-qualified-function-template-call.t](/Users/vishvananda/cppgm/pa19/tests/spec/147-qualified-function-template-call.t)
    - [160-duplicate-template-instantiation-signature.t](/Users/vishvananda/cppgm/pa21/tests/spec/160-duplicate-template-instantiation-signature.t)
    - [225-forwarding-reference-move.t](/Users/vishvananda/cppgm/pa22/tests/spec/225-forwarding-reference-move.t)
    - [143-template-converting-constructor-call-argument.t](/Users/vishvananda/cppgm/pa26/tests/spec/143-template-converting-constructor-call-argument.t)
    - [208-distinct-lambda-member-template-types.t](/Users/vishvananda/cppgm/pa27/tests/spec/208-distinct-lambda-member-template-types.t)
  - an earlier host-built blocker in the same family is now reduced too:
    - `/tmp/member_template_const_internal_call.cpp`
    - source shape:
      - non-const member function template forward declaration
      - const member function template inline definition
      - unqualified call from inside another const member template
    - before the fix, lookup only retained the non-const template candidate and
      rejected it with `implicit object conversion failed`
    - the same shape also explained the hosted libc++ failure in
      `/tmp/const_map_find_string.cpp`, where `const std::map<...>::find`
      could only see the non-const `__tree::__find_equal`
    - root cause:
      [semantic_lookup.cpp](/Users/vishvananda/cppgm/dev/src/semantic_lookup.cpp)
      compared member function template identities without including
      `const`/`volatile`/ref qualifiers, so lookup-time dedup collapsed distinct
      member-template overloads
  - reduced owner regression added:
    [221-const-member-function-template-overload.t](/Users/vishvananda/cppgm/pa18/tests/spec/221-const-member-function-template-overload.t)
- Intended debug direction:
  - closed

### BF-003: retired misowned hosted test

- Status: retired
- Prior evidence:
  - the old `pa18` timeout on
    [219-function-template-default-allocator-local-lambda.t](/Users/vishvananda/cppgm/pa33/tests/compile/720-hosted-function-template-default-allocator-local-lambda-compile.t)
    was real, but the test itself was misowned
  - it uses hosted STL headers and belongs in `pa33/tests/compile`, not `pa18`
- Current disposition:
  - the test has been moved to
    [720-hosted-function-template-default-allocator-local-lambda-compile.t](/Users/vishvananda/cppgm/pa33/tests/compile/720-hosted-function-template-default-allocator-local-lambda-compile.t)
  - do not treat that old timeout as a `pa18` frontier until `pa18` is rerun

### BF-004: non-`cppgm++` selfhost immediate segfault family

- Status: completed
- Affected self-built binaries:
  - [lowir2cy86-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/lowir2cy86-self)
  - [lowir2native-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/lowir2native-self)
  - [cpplink-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/cpplink-self)
  - [cppeh-self](/Users/vishvananda/cppgm/obj/pa37/bin/selfhost/cppeh-self)
- Earliest failing owners:
  - [pa13/tests/spec/100-ret0.t](/Users/vishvananda/cppgm/pa13/tests/spec/100-ret0.t)
  - [pa23/tests/strict/100-ret0.t](/Users/vishvananda/cppgm/pa23/tests/strict/100-ret0.t)
  - [pa24/tests/spec/100-single-object.t](/Users/vishvananda/cppgm/pa24/tests/spec/100-single-object.t)
  - [pa25/tests/spec/100-same-function-catch.t](/Users/vishvananda/cppgm/pa25/tests/spec/100-same-function-catch.t)
- Current evidence:
  - earlier LLDB work showed the shared stop frame at
    `std::__1::locale::locale(std::__1::locale const&)`
  - hardened malloc did not trip `malloc_error_break`, so this is distinct from
    `BF-001`
  - preserved selfhost symptom:
    - host `cppgm++` emits a 90-byte LowIR file for `int main(){return 0;}`
    - both `lowir2cy86-self` and `lowir2native-self` segfault on that file
  - hosted reduction now isolates the underlying compiler bug:
    - [732-hosted-getline-indirect-result-vbptr-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/732-hosted-getline-indirect-result-vbptr-link-smoke.t)
    - source shape: `Result load(std::istream&) { std::string line; std::getline(in, line); return Result{...}; }`
    - an `int`-return variant is fine, but the class-return variant crashed at
      the first `getline` in the same `std::__1::locale::locale` frame
  - root cause on the hosted reduction:
    - the indirect-result call to `load` originally lowered as
      `call void @load(%ret, %istream_base, %ret_pvbptr)`
    - that third argument should have been the hidden virtual-base companion
      pointer for the `std::istream&` parameter, not a pointer derived from the
      return object
  - [lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp)
      now maps parameter hidden-virtual-base companion arguments using lowered
      physical argument slots instead of raw logical parameter indices

## Deferred Follow-Up

### FU-001: non-STL selfhost-only weak-template scale long-run

- Status: deferred for later examination
- Current reduced shape:
  - `/tmp/nonstl_weak_scale_<N>.cpp`
  - repeated instantiations of:
    - `template<int N> struct Inner { ~Inner() { g += 1; } };`
    - `template<int N> struct Outer { Inner<N> inner; ~Outer() {} };`
- Current evidence:
  - this reduction does **not** reproduce the original Mach-O runtime crash
  - tiny and moderate non-STL weak-template executables still link through
    weak same-binary stubs/GOT slots and run correctly
  - but the same source family exposes a selfhost-only compile-time long-run:
    - `N=1200`: `cppgm++-self` compiles and runs in about `45.18s`
    - `N=1800`: `cppgm++-self` times out at `60.02s` and emits no binary
    - host `cppgm++` compiles the larger `N=4000` case in about `11.54s`
- Current interpretation:
  - this looks like a real earlier non-STL selfhost template-performance bug
    or miscompile-adjacent long-run
  - it may be related to the broader template frontier, but it is not yet
    proven to be the same root cause as the hosted Mach-O weak-branch crash
- Intended debug direction:
  - revisit after the current active queue narrows
  - if needed later, turn this into an owned regression at the earliest
    assignment that can honestly carry a selfhost-only long-run/perf guard

### FU-002: top-level alias-declaration of anonymous struct

- Status: fixed in `ffcdd998` follow-up work
- Current reduced shape:
  - `/tmp/stl_tree_piece_probe.cpp`
  - source shape:
    - `using TreePair1 = struct { ... };`
    - `using TreePair2 = struct { ... };`
- Current evidence:
  - Clang accepts the source.
  - `cppgm++` now also accepts:
    - top-level `using X = struct { int n; };`
    - block-scope `using X = struct { int n; };`
  - owned regression:
    - [283-top-level-alias-anonymous-struct.t](/Users/vishvananda/cppgm/pa15/tests/spec/283-top-level-alias-anonymous-struct.t)
- Current interpretation:
  - the bug was a missing alias-specific embedded-type preparation pass
  - normal declarations already named and collected anonymous `class/enum`
    specifiers before type parsing; alias declarations did not
- Intended debug direction:
  - closed

### BF-005: late selfhost compile/object/runtime crash family

- Status: in-progress
- Affected checkpoints:
  - `pa31`
- Earliest failing tests:
  - [130-static-archive.t](/Users/vishvananda/cppgm/pa31/tests/spec/130-static-archive.t)
- Current evidence:
  - `pa30` no longer belongs in this bucket
    - the current isolated repro for
      [303-runtime-virtual-member-pointer-multiple-inheritance.t](/Users/vishvananda/cppgm/pa30/tests/spec/303-runtime-virtual-member-pointer-multiple-inheritance.t)
      aborts during compile inside the same dependent-type/template-deduction
      family tracked in `BF-002`
  - `pa31` is no longer confirmed as active on the current staged snapshot
    - the current isolated repro for
      [130-static-archive.t](/Users/vishvananda/cppgm/pa31/tests/spec/130-static-archive.t)
      passes with `impl.exit_status=0` and `program.exit_status=0`
    - keep this bucket open only until `pa31` is rerun cleanly in the staged
      sweep
  - `pa32` remains green and is out of the active frontier
- Intended debug direction:
  - defer until earlier `cppgm++` frontiers are narrowed

### BF-006: hosted compile/link timeout family

- Status: in-progress
- Affected checkpoints:
  - `pa33`, `pa34`
- Earliest failing tests:
  - [542-local-functor-std-function-assignment.t](/Users/vishvananda/cppgm/pa33/tests/compile/542-local-functor-std-function-assignment.t)
  - [651-hosted-unordered-map-string-int-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/651-hosted-unordered-map-string-int-link-smoke.t)
- Current evidence:
  - `pa33` now fails by compile timeout on hosted/library-heavy sources, not by
    immediate crash
  - `pa33` still has earlier expected-failure owners like
    [508-offsetof-bitfield-bad.t](/Users/vishvananda/cppgm/pa33/tests/compile/508-offsetof-bitfield-bad.t),
    but the first real non-matching owner remains
    [542-local-functor-std-function-assignment.t](/Users/vishvananda/cppgm/pa33/tests/compile/542-local-functor-std-function-assignment.t)
  - `pa34` still makes real forward progress through its worker and is not an
    idle harness hang
  - the rebuilt `pa34` sweep now reaches much further than before, but still
    times out on multiple hosted/library tests
  - the latest sweep reached the `720+` tail before the worker stack stalled
    after writing results, so this bucket is now mostly real hosted timeouts
    plus a small amount of worker teardown noise
  - concrete current `pa34` symptoms include:
    - [651-hosted-unordered-map-string-int-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/651-hosted-unordered-map-string-int-link-smoke.t):
      `EXIT_TIMEOUT (124)`
    - [675-hosted-dynamic-exception-spec-runtime.t](/Users/vishvananda/cppgm/pa34/tests/link/675-hosted-dynamic-exception-spec-runtime.t):
      implementation exit `1`
    - [692-hosted-initializer-list-member-definition-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/692-hosted-initializer-list-member-definition-link-smoke.t):
      implementation exit `139`
    - [708-hosted-shared-ptr-inline-odr-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/708-hosted-shared-ptr-inline-odr-link-smoke.t):
      `EXIT_TIMEOUT (124)`
    - [731-hosted-member-vs-free-shift-overload-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/731-hosted-member-vs-free-shift-overload-link-smoke.t):
      pass under selfhost
    - [732-hosted-getline-indirect-result-vbptr-link-smoke.t](/Users/vishvananda/cppgm/pa34/tests/link/732-hosted-getline-indirect-result-vbptr-link-smoke.t):
      `EXIT_TIMEOUT (124)` under the rebuilt selfhost binary
- Intended debug direction:
  - keep this separate from the earlier template and runtime/object families
  - revisit after the earlier selfhost crash buckets are reduced

### BF-007: self-compare hosted-header/compiler-source compile family

- Status: in-progress
- Blocking validation:
  - isolated `cppgm++` vs `cppgm++-self` `pa1` artifact compare under
    `obj/pa35-bitcmp/selfcmp`
- Current evidence:
  - a fresh isolated `pptoken-self` self-compare build still fails before any
    bit-for-bit artifact diff can be taken
  - rerunning that isolated self-compare build with the staged compiler forced
    to `-O0` still fails in the same pre-diff compile phase, so this bucket is
    not explained away by the current `pa37` self-build optimization issues
  - current `-O0` compare build still dies while compiling:
    - `encoding.o`
    - `pptokenizer.o`
    - `test_runner-enabled.o`
    - `pptoken-runner.o`
  - the preserved failure moved past the old cold-include-path loop and is now
    in hosted-header/template compilation
  - current reduced compiler-source probes:
    - `/tmp/bf-encoding-include-stdexcept.cpp`:
      selfhost exits with
      `ERROR: unknown type kind in template_argument_type_text [function std::__1::swap]`
    - `/tmp/bf-encoding-include-algorithm.cpp`:
      selfhost segfaults
    - `/tmp/bf-encoding-include-both.cpp`:
      selfhost segfaults
  - current smallest reliable selfhost separator is:
    - `/tmp/std_string_min3.cpp`
    - host `./dev/cppgm++ -std=gnu++11 -Wall -O0 -I./dev/src -c`: pass
    - staged `obj/pa37/bin/selfhost/cppgm++-self -std=gnu++11 -Wall -O0 -I./dev/src -c`:
      segfault / `139`
    - reduced source shape:
      `long, long, const std::string&, long, long, long, long, bool`
      with the `"class"` temporary bound to `const std::string &`
  - more recent mixed-object relink probes moved the manifestation forward from
    builtin `initializer_list` instantiation into builtin function registration
    and then into `std::map<std::string, ...>::find()` on corrupted function-set
    keys
    - replacing clang-built `semantic_class_model.cpp`,
      `template_instantiation.cpp`, and `callsemantic.cpp` still crashes
    - adding clang-built `semantic_builtins.cpp`, `semantic_model.cpp`,
      `semantic_lookup.cpp`, and `symbol_linkage.cpp` still crashes
  - the remaining family therefore no longer looks like a single front-end TU
    owner; it now looks like a shared string-lifetime/codegen bug that several
    semantic TUs happen to exercise
  - standalone host-runtime probes that mimic only the surface pattern
    (`std::string` by-ref calls, `std::map<std::string, ...>` insert/find, and
    a small `register_function()`-style mock) still pass under
    `./dev/cppgm++ -O0`, so there is not yet a direct non-selfhost reproducer
- Intended debug direction:
  - do not spend the next rebuild cycle on bit-compare alone
  - treat this as a lower-level codegen/lifetime bucket rather than another
    semantic-template owner
  - come back once the remaining staged `O0` frontier is smaller, or once a
    direct host-runtime reproducer for the string-lifetime pattern is found

## Execution Order

Work this queue in this order:

1. `BF-009` `pa15` / `pa16` constructor/member-init/assignment/decltype family
2. `BF-002` remaining template/deduction/current-specialization family, starting at `pa18`
4. `BF-005` late selfhost runtime/object family
5. `BF-006` hosted compile/link timeout family
6. `BF-007` self-compare hosted-header/compiler-source family

Do not start another long self-host compile until the current queue has either:

- landed concrete source-level fixes that are ready to batch-validate, or
- been narrowed far enough that a broad rerun is likely to produce a new
  frontier instead of repeating the same known failures
