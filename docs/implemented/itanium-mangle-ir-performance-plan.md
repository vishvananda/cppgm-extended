# Itanium Mangle IR Performance Plan

## Goal

Move Itanium symbol mangling onto a structured, cacheable IR without losing the
strict ABI spelling guarantees that the current `symbol_linkage.cpp` path has
accumulated.

The immediate trigger is the `std::vector`/`std::map` substitution-slot bug
found during the GCC PA38 frontier. A direct string-key fix made the symbols
match Homebrew Clang, but regressed the standard
`semantic_overload.cpp` performance gate by about 5-7% instructions. The
replacement must make the correct substitution sources cheap, not merely add
more long canonical strings.

Priorities:

1. Correctness and single ownership: preserve Homebrew Clang spelling for
   checked symbols and move every active ABI spelling owner onto typed IR.
2. Simplicity: extend the existing `itanium_mangle_ir.h` instead of creating a
   second mangling architecture, and remove compatibility paths as soon as their
   typed replacement is covered by strict tests.
3. Performance: track every slice against the standard perf gate. Temporary
   regressions are allowed while the old and IR paths coexist, but the finished
   transition must recover them before adding broad caching.

## Current State

`dev/src/itanium_mangle_ir.h` now owns a broad set of ABI spelling shapes:

- builtin types
- cv-qualified types
- pointers and references
- arrays
- function types
- member-pointer types
- named types and member-owned named types
- class-template specializations
- type, entity, dependent-expression, and template-template arguments
- pack expansions
- simple qualified function names
- function-template names
- fixed operators, conversion operators, constructors, and destructors
- premangled leaves

The remaining problem is no longer just missing IR coverage. The current
implementation still has multiple ABI spelling owners and several escape
hatches:

- older string-heavy emitters in `dev/src/symbol_linkage.cpp` remain callable
  after a typed path declines a case
- string identity still leaks into typed decisions, including operator-name
  parsing, elaborated type spelling cleanup, and legacy substitution keys
- upstream semantic layers still store premangled fragments such as
  `named_itanium_abi_*`, `lambda_itanium_*`, and `TK_PREMANGLED` leaves
- structural keys still stringify portions of IR with `structural_text()` and
  legacy-key compatibility

That means strict correctness is close, but the current path is expensive and
hard to reason about: it can build typed IR, build legacy text beside it,
precompute ABI fragments in another layer, and then render the same semantic
object more than once.

## Non-Goals

- Do not land the current slow `std::` substitution prototype as-is.
- Do not fix the frontier by matching GCC's libstdc++ raw spelling; cppgm keeps
  matching Homebrew Clang's Itanium spelling.
- Do not cache final rendered fragments that participate in substitutions. The
  `S_`, `S0_`, ... positions are local to each rendered symbol.
- Do not keep adding text reparsers for ABI grammar regions that already have
  semantic or AST structure.
- Do not use a cache to paper over duplicated old-path/IR work. First remove
  the duplicate ownership and premangled producer paths; then optimize the final
  representation.
- Do not keep premangled internal ABI fragments as a normal producer API. A
  premangled leaf is acceptable only for a true external ABI boundary with an
  explicit justification.

## Follow-Up Strategy Update

The next phase should prioritize cleanup over optimization. That plan makes
sense because the current perf regression appears dominated by overlapping work:
typed IR is now broad enough to pass strict/report gates, but legacy emission,
legacy substitution-key construction, and premangled fragments still exist in
parallel. Optimizing a partial state risks preserving compatibility shims that
should disappear.

The revised order is:

1. Remove all remaining active non-IR generation/fallback paths.
2. Remove text-based identity from the typed mangling path.
3. Collapse duplicated mangling logic and remove parallel owners.
4. Audit the simplified design for one more round of unnecessary complexity.
5. Remove premangled encodings from upstream semantic layers so mangling happens
   in one place.
6. Only then perform representation/storage optimization and consider caches.

Perf still runs after each code slice and is recorded in the ledger, but during
cleanup failures are tracked rather than treated as blockers unless they become
large enough to obscure correctness work. Once the old path and premangled
producer paths are gone, the standard perf envelope becomes a hard gate again.

## Current Execution Plan: 2026-05-09

The current working tree is in the final removal lane, not an optimization lane.
The next work should happen in this order:

1. Finish the in-progress removal slice and make all correctness gates pass.
   This means fixing the remaining strict failures by threading the missing typed
   IR data through the mangler, not by re-enabling text/AST fallback emitters.
2. Commit that passing removal slice after the normal validation and perf ledger
   update.
3. Completely remove the remaining old text/AST mangling implementation and dead
   helper code. After this step, every internal ABI spelling path should be owned
   by typed IR and should fail loudly if required structured data is absent.
4. Commit the full old-path deletion after the same validation and perf ledger
   update.
5. Optimize the final typed representation before adding caches:
   - reduce IR storage size and copying
   - replace finite spelling strings with enums or compact ids
   - remove leftover compatibility state that only existed for the old path
6. Use the existing counters to decide whether a cache is still valuable. Add a
   cache only when counters show repeated plan construction or substitution work
   that survives the cleanup and storage optimization.
7. Remove the temporary metrics/counters once they have served the cache and
   optimization decisions.
8. If the migration is still above the active performance baseline after the old
   path, storage overhead, useful caches, and counters are gone, keep optimizing
   until the standard perf gate drops back under the envelope.

The commit and tracking process does not change:

- Each code slice builds the GCC `dev/cppgm++` binary.
- Each landed slice runs full `test-strict` with LowIR direct text compare.
- PA32/PA33 `test-report` runs for symbol-surface changes.
- The standard perf gate runs after correctness validation.
- The perf result is recorded in the ledger before committing.
- Temporary instruction regressions are acceptable only while they are explained
  by incomplete migration state; the finished migration must recover them.

## Design

### 1. Structural Substitution Keys

Replace string substitution identity with compact structural keys in the IR
renderer.

The key should be a small value:

- key kind: named component, complete type, template entity, prefix, function
  type, wrapper type, standard abbreviation source
- stable pointer or interned id for semantic declarations and type nodes
- small wrapper payload for `const`, pointer, reference, array, function, and
  member-pointer types
- value payload only for ABI-relevant non-type template arguments

Legacy string keys may remain behind a compatibility wrapper during migration,
but new migrated regions should not build `name:std::vector<...>` or
`type:lref(...)` strings just to find a substitution slot.

### 2. Mangle Plan vs. Rendered Symbol

Build a typed mangle plan from semantic/AST data, then render the plan with a
fresh per-symbol substitution table.

The plan can be cached because it describes ABI structure. The rendered symbol
cannot be blindly reused inside another symbol because substitutions are
position-dependent.

Expected plan objects:

- semantic type to IR type plan
- class-template specialization component plan
- function qualified-name plan
- template-argument list plan
- RTTI/vtable/thunk name plan

The renderer remains intentionally per-symbol and owns the positional
substitution table.

### 3. Standard Substitutions As Explicit IR

Model standard abbreviations as structured nodes, not side effects hidden in
string emission.

Examples:

- `St` abbreviates namespace `std`.
- `SaI...E` abbreviates the `std::allocator` template name, but the complete
  `std::allocator<T>` specialization is still a normal substitution source.
- `SbI...E` abbreviates the `std::basic_string` template name, but the
  complete specialization is still a normal substitution source.
- `Ss`, `Si`, `So`, and `Sd` abbreviate complete standard types and therefore
  do not create the same additional complete-template substitution source.

The IR should encode this as data such as:

```text
standard-abbrev(kind=std_allocator, source=template_name_only)
standard-abbrev(kind=std_string, source=complete_type)
```

The renderer then registers exactly the substitution sources required by the
Itanium ABI and the Homebrew Clang spelling.

### 4. Context-Sensitive Inputs

The plan cache key must include the context that can change ABI spelling:

- local and anonymous namespace scope
- function-template parameter and argument context
- owner-template parameter suppression
- lambda context and discriminator
- ABI tags
- pack-size metadata
- direct-standard-substitution policy

If a region depends on active substitution slots only, cache the plan but not
the rendered fragment.

### 5. Entity Symbol Caching

Cache final object symbols at stable semantic entities only after the symbol is
context-independent:

- externally visible functions and variables
- known internal function-local entities after local discriminator assignment
- vtables, VTTs, RTTI objects, and thunks

Each cache entry needs an invalidation story. If class completion or template
replay can still mutate required ABI data, cache the mangle plan only, or carry
a semantic generation number and invalidate deliberately.

## Implementation Slices

### Phase 0: Baseline And Counters

Add cheap counters with no behavior change:

- `try_mangle_itanium_function_name_syntax` calls
- type mangle calls by kind
- class-template specialization mangle calls
- substitution lookup/register calls
- legacy substitution-key bytes built
- canonical named text calls and bytes
- IR plan cache hits/misses once the cache exists

Run and record:

```sh
env -u CPPGM_HOST_CXX -u CPPGM_STDLIB_FLAGS \
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json
```

Correctness gates:

```sh
env -u CPPGM_HOST_CXX -u CPPGM_STDLIB_FLAGS \
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
CPPGM_STRICT_TYPE_TEXT_FALLBACKS=1 \
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-strict-nobuild \
  CXX=/usr/local/opt/gcc/bin/g++-15 \
  OBJ=../obj/gcc15
```

### Phase 1: Structural Keys For Existing IR Types

Teach `itanium_mangle_ir.h` and `MangleSubstitutionState` to use structural
keys for the existing context-free type IR.

Keep legacy-string compatibility only at the old boundary. The key result for
this phase is that pointer/reference/function type substitutions no longer need
to build long wrapper strings for migrated IR types.

Validation:

- strict LowIR compare
- PA32 substitution-owner tests
- standard perf gate

### Phase 2: Named Type And Template Component IR

Add IR nodes for:

- source-name component
- qualified/nested name prefix
- named semantic type
- class-template specialization
- template argument list
- standard abbreviation node with explicit substitution-source metadata

Migrate only class-template specialization type mangling first. The target
regression is the PA33 reducer for the PA38 bug:

- `pa33/tests/spec/255-host-std-template-substitution-slots.t`

Also keep PA32 host symbol checks that compare against the host compiler:

- `pa32/tests/spec/209-host-namespaced-enum-template-arg-mangling.t`
- `pa32/tests/spec/226-function-template-nested-ref-substitution.t`
- `pa32/tests/spec/227-namespace-operator-template-std-string-substitution.t`

Do not migrate function names yet except where needed to render the type.

### Phase 3: Template Arguments And NTTPs

Move template arguments into IR:

- type arguments
- integral and enum non-type arguments
- entity references
- member pointers
- dependent expression AST nodes
- template-template arguments
- pack expansions and pack-size metadata

This phase should delete old text fallback entry points only after the
structured equivalent is covered by strict tests.

Validation:

- PA18, PA19, PA21, PA22 strict LowIR compare
- PA32/PA33 report
- focused Homebrew Clang vs cppgm symbol comparison for migrated reducers
- standard perf gate

### Phase 4: Function Name IR

Move function qualified-name mangling into IR:

- namespace/class owner prefixes
- constructors, destructors, conversion operators, and operator names
- function templates and template-prefix substitution
- local entity context
- lambda contexts
- ABI tags

The output should still be rendered once per requested symbol with a fresh
substitution table.

Validation:

- PA32 full report
- PA33 full report
- strict LowIR compare
- standard perf gate

### Phase 5: Remove Remaining Non-IR Generation

Finish moving every active ABI spelling owner onto the IR representation before
doing storage optimization or cache work. A migrated region must either render
through typed IR or fail loudly because required semantic/AST data is missing.
It must not retry through an older text emitter.

Current targets include:

- lambda function contexts
- constructor templates
- dependent expression and type-syntax template arguments
- static member and scoped object names where they still hand-emit nested names
- RTTI/vtable/thunk names if they reuse type/name spelling logic
- function result and parameter tails that still call `try_mangle_type_impl`
  directly after the name IR has been built
- remaining template-argument paths that still call older text helpers after
  `try_build_*_ir` declines

Exit criteria:

- no normal ABI path in `symbol_linkage.cpp` emits a name/type/template argument
  by hand when an IR node exists for that grammar region
- no typed IR builder uses retry/fallback behavior as a control-flow strategy
- remaining non-IR emission sites are listed as explicit external boundaries or
  are converted before the phase closes

Validation:

- full strict LowIR compare for every removal slice
- PA32/PA33 report for symbol-surface changes
- if PA32/PA33 finds a new symbol failure, add or identify an equivalent strict
  test before fixing it where feasible
- standard perf gate recorded in the ledger

### Phase 6: Remove Text Identity From Mangling Decisions

Once the old emitters are no longer active fallbacks, remove typed-path decisions
that depend on rendered text identity.

Targets:

- replace operator-name parsing from strings such as `compact_operator_name()`
  with enum/token/syntax data from the declaration
- represent elaborated dependent type syntax (`typename`, `struct`, `class`,
  `enum`) as semantic/syntax metadata instead of stripping prefixes from lookup
  strings
- replace `named_display`, `named_key`, and normalized source text identity in
  ABI decisions with semantic declarations, AST nodes, type nodes, or interned
  source-name ids
- replace `legacy_text()` and `structural_text()` substitution identity for
  migrated regions with compact structural keys
- make standard substitutions depend on structured namespace/template/type
  identity, not strings such as `std::vector` or `std::allocator`
- keep textual spelling only as renderer output, diagnostics, or a temporary
  strict-test oracle

Validation:

- strict LowIR compare
- PA32/PA33 report for symbol-surface changes
- focused Homebrew Clang spelling comparison for any operator, elaborated type,
  or standard-substitution reducer touched by the slice
- standard perf gate recorded in the ledger

### Phase 7: Collapse Duplicated Mangling Logic

After text identity is removed, merge parallel helper stacks that now do the
same work through slightly different entry points.

Targets:

- one builder path for type IR, including context-sensitive named and template
  types
- one builder path for function-name IR, including plain functions, member
  functions, operators, constructors, destructors, conversion operators, and
  lambda call operators
- one builder path for template arguments and dependent expressions
- one substitution-state API that does not require legacy-key mirroring for
  migrated regions
- one renderer path for named prefixes, standard substitutions, and template
  argument lists

Exit criteria:

- call sites choose the semantic object to mangle, not a string/text mangle
  strategy
- special cases are encoded as IR variants or policy fields, not separate
  near-copy emitters
- temporary trace counters remain available until strict/report gates are clean,
  then are either removed or made explicitly diagnostic-only

Validation:

- strict LowIR compare
- PA32/PA33 report if symbol-surface code moved
- standard perf gate recorded in the ledger

### Phase 8: Simplification Audit

Pause after the duplicate logic is collapsed and review the IR shape before
touching caches.

Audit questions:

- Can any IR nodes be merged because they differ only by renderer spelling?
- Are there string fields whose only remaining use is debug output?
- Are builder context objects carrying policy that should be explicit IR data?
- Are compatibility shims still present only because tests were missing?
- Is there one obvious place to inspect or change each ABI grammar rule?

This phase is allowed to make small cleanup commits, but not broad performance
tuning. Its goal is to reduce future surface area before premangled producer
cleanup.

Validation:

- strict LowIR compare
- PA32/PA33 report for any public symbol-shape movement
- standard perf gate recorded in the ledger

### Phase 9: Remove Premangled Encodings From Upstream Layers

Move ABI fragment construction out of semantic/type construction and into the
single typed mangler.

Inventory to eliminate or replace with structured metadata:

- `Type::named_itanium_abi_encoding`
- `Type::named_itanium_abi_substitution_keys`
- `Type::named_itanium_abi_context_encoding`
- `Type::named_itanium_abi_context_substitution_keys`
- `Type::named_itanium_abi_context_function_*`
- `Type::named_itanium_abi_signature_parameter_types`
- `Type::named_itanium_abi_discriminator`
- `ClassInfo::lambda_itanium_context_encoding`
- `ClassInfo::lambda_itanium_signature_encoding`
- `ClassInfo::lambda_itanium_substitution_keys`
- `FunctionSymbolOptions::lambda_itanium_*`
- `itanium_mangle_ir::Type::premangled` and
  `premangled_preregister_legacy_keys`

Replacement shape:

- semantic layers may record source entities, local/lambda context ownership,
  discriminator values, signature parameter type lists, and ABI-relevant syntax
  metadata
- semantic layers must not pre-render Itanium fragments or substitution-key
  strings
- the mangler owns conversion from structured metadata to ABI bytes and
  substitution registrations

Exit criteria:

- no internal semantic type stores a ready-to-concatenate ABI fragment
- premangled leaves exist only for documented external ABI boundaries, or are
  removed entirely
- lambda/local class spelling is rendered by the same function-name/type IR
  renderer as every other nested name

Validation:

- strict LowIR compare
- PA32/PA33 report
- targeted inception/frontier reducer rerun for the scope/lambda/local symbol
  cases that originally required these fields
- standard perf gate recorded in the ledger

### Phase 10: IR Representation And Storage Optimization

Before adding caches, reduce the cost of the IR itself:

- replace unnecessary string copies in name/type/template-argument components
- store standard substitutions and operator terminals as compact enum data where
  the spelling set is finite
- remove any remaining legacy substitution text for IR-owned regions
- shrink per-component vectors and optional fields that are empty for the common
  source-name path
- consider interning source names and qualified-name components if profiles show
  repeated allocation
- prefer compact enum/pointer/id payloads over rendered strings for structural
  identity

Validation should include the standard perf gate and a brief note about which
copy/storage source was removed.

### Phase 11: Cached Mangle Plans

Add plan caches only after the legacy path is removed, premangled producer paths
are gone, and the IR representation has been tightened.

Cache candidates:

- `Type *` plus context policy to type IR plan
- class-template specialization metadata to component IR plan
- `FunctionBinding`/symbol options to function-name IR plan

Do not cache rendered fragments that depend on active substitutions. Cache only
plans and context-independent final symbols.

Validation must include before/after counter totals showing fewer plan builds
or fewer legacy key bytes, not just wall-time noise.

## Validation Policy

Every landed slice must pass:

```sh
env -u CPPGM_HOST_CXX -u CPPGM_STDLIB_FLAGS \
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
CPPGM_STRICT_TYPE_TEXT_FALLBACKS=1 \
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-strict-nobuild \
  CXX=/usr/local/opt/gcc/bin/g++-15 \
  OBJ=../obj/gcc15
```

For PA32/PA33 symbol-surface changes:

```sh
env -u CPPGM_HOST_CXX -u CPPGM_STDLIB_FLAGS \
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
CPPGM_STRICT_TYPE_TEXT_FALLBACKS=1 \
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
make test-report-nobuild \
  ACTIVE_TEST_REPORT_PAS='pa32 pa33' \
  CXX=/usr/local/opt/gcc/bin/g++-15 \
  OBJ=../obj/gcc15 \
  ORDERED=false
```

Every code slice must also run:

```sh
env -u CPPGM_HOST_CXX -u CPPGM_STDLIB_FLAGS \
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json
```

If the perf gate fails during the temporary old-path/IR overlap, record the
regression here and keep moving through the removal plan. The completed
transition must return below the active envelope. Starting with Phase 10, perf
work should use the standard envelope as a hard gate again.

## Rollout Rule

During the early migration phases, the old path may remain available outside the
region being moved, but migrated ABI regions should have one owner. A new IR
path should either:

- produce exactly the same symbol and replace the old path for that region, or
- fail loudly because required semantic/AST data is missing.

It should not silently fall back to text reparsing or emit a weakly equivalent
spelling. After Phase 5, any surviving non-IR mangling path must be documented
as an external boundary or removed.

## Perf Ledger

All controlling measurements in this ledger use the GCC-built `dev/cppgm++`
binary with the existing baseline at `/tmp/cppgm-semantic-overload-baseline.json`.
The active envelope for this migration is approximately +3% retired
instructions and +3% peak footprint until the remaining temporary mangling
overhead is removed.

Baseline:

- `9c022dbcb3218ba370b81ea47e600bf51b8c9a32`
- instructions: `395,110,254,182`
- peak footprint: `1,011,073,024`

Recorded migration measurements:

| Commit | Slice | Instructions | Peak footprint | Notes |
| --- | --- | ---: | ---: | --- |
| `32b90527` | Local static symbol stabilization after mangle IR slices | +3.88% | +3.32% | Failed the current +3%/+3% envelope. |
| `32b90527` with local-static patch temporarily removed | Isolation run | +3.57% | +3.27% | Shows most remaining overhead predates the local-static fix. |
| `fa072bf8` | Fix dependent member expression strict-gate regression | +3.87% | +3.26% | Full strict passes again; perf remains at the existing GCC overhead level. |
| uncommitted | Earlier Phase 2 class-template/named-type IR, broad contextual coverage | +7.83% | +3.37% | Strict/report passed; superseded by the later broad-contextual run below. |
| `540d487a` | Phase 2 class-template/named-type IR, std-only contextual coverage | +5.63% | +3.47% | Strict/report passed; accepted as temporary while legacy and IR paths coexist. |
| `0ac56a11` | Member-pointer type IR | +5.88% | +3.44% | Strict/report passed; continues removing legacy type-shape coverage while old and IR paths coexist. |
| `2be28772` | Template-template argument IR | +5.73% | +3.48% | Strict/report passed; non-dependent class/alias template arguments now have an IR path. |
| `c87cc52c` | Entity-valued NTTP IR | +5.50% | +3.48% | Strict/report passed; function pointer/reference and object reference template arguments now render through IR. |
| `c3a14b4b` | Broader contextual class-template specialization IR | +8.19% | +3.42% | Strict/report passed; accepted as temporary coverage expansion while legacy and IR paths coexist. |
| `4a5d461f` | Remove legacy entity-valued NTTP fallback | +7.79% | +3.41% | Strict/report passed; removes the duplicate old entity argument renderer. |
| `86026ea0` | Pack-aware class-template argument IR | +7.83% | +3.46% | Strict/report passed; class-template IR now emits pack groups from specialization pack-size metadata. |
| `c578d7c8` | Simple qualified function-name IR | +7.85% | +3.43% | Strict/report passed; simple source-name function IR now owns qualified names, `std::` functions, and ABI tags. |
| `f39d1ee9` | Member-owned named/class-template type IR | +7.92% | +3.43% | Strict/report passed; nested type-prefix rendering now covers member classes and member class-template specializations. |
| `74ac2396` | Function-template name/template-argument IR | +7.93% | +3.40% | Strict/report passed; simple function-template names now render template argument lists through IR while the existing parameter-tail logic remains in place. |
| `a9e8f6c5` | Fixed operator function-name IR | +8.00% | +3.46% | Strict/report passed; non-member fixed operators and operator-template argument lists now render their name prefix through IR. |
| `043ab989` | Simple member function-name IR | +7.81% | +3.43% | Strict/report passed; simple member function names now render nested cv/ref-qualified prefixes through IR. |
| `6d9e40b6` | Simple constructor/destructor name IR | +7.87% | +3.47% | Strict/report passed; non-template special-member entry-point names now render through IR for simple owners. |
| `0472551b` | Member fixed-operator name IR | +7.85% | +3.45% | Strict/report passed; simple member fixed-operator names now share the operator IR prefix path. |
| `28d2cf2a` | Conversion operator name IR | +7.68% | +3.45% | Strict/report passed; non-template conversion operators now render their typed `cv...` terminal through IR. |
| working tree after `28d2cf2a` | Owner template function-name component IR | +8.52% | +3.45% | Strict/report passed; PA32/PA33 passed; non-empty owner template components now render through function-name IR and standard substitutions no longer consume substitution slots. |
| working tree after `48ab29d3` | Strict closure for dependent NTTP, conversion type, and local lookup regressions | +17.22% | +4.84% | Perf gate failed; full strict and PA32/PA33 report passed. Adds builtin type-transform IR, isolates conversion target substitutions, and fixes lookup-based qualification. Needs follow-up optimization plan before more broad IR expansion. |
| working tree after `645c5ac` | Phase 5 type path fallback removal and lambda closure type IR | +14.66% | +4.18% | Perf gate failed; full strict and PA32/PA33 report passed. Deletes the old recursive type emitter stack and adds a typed lambda-closure node for RTTI/type-name symbols. |
| working tree after `a977169d` | Remove full premangled lambda closure producer | +14.30% | +4.21% | Perf gate failed; full strict and PA32/PA33 report passed. Lambda closures now use structured context/signature metadata instead of `named_itanium_abi_encoding`. |
| working tree after `09b73aee` | Remove `named_itanium_abi_encoding` from types | +14.36% | +1.96% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Anonymous enum spelling now comes from typed name-component IR, and the full premangled type field/copy plumbing is gone. |
| working tree after `8722dcb1` | Remove type-level lambda context fragments | +14.31% | +0.45% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Lambda closure types now rebuild the enclosing-function context inside the mangler instead of storing context fragments on `Type`. |
| working tree after `3094494d` | Remove lambda fragments from class/function options | +14.14% | -0.12% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Lambda call operators now carry the closure type and emit the signature from typed parameter IR instead of passing pre-rendered class/options fragments. |
| working tree after `64d2d10a` | Remove semantic lambda signature pre-rendering | +14.25% | -0.28% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Removes semantic-side lambda context/signature rendering and the public lambda signature helper; ordinary free/namespace lambdas now use typed Clang local-source-name spelling. |
| working tree after `3b2dc37c` | Add typed template-parameter type IR | +14.19% | -0.11% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Replaces the `T_`/`Tn_` premangled type leaf with a typed IR node while preserving the existing template-parameter substitution identity. |
| working tree after `8bb5dce3` | Remove premangled type IR support | +14.53% | -0.30% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Deletes `TK_PREMANGLED`, `premangled_leaf`, and premangled substitution-key support; the remaining lookup preregistration is a generic legacy-key seed on typed IR. |
| working tree after `13110868` | Remove public substitution-fragment helpers | +13.92% | -0.12% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Removes the public type substitution-fragment API and makes the function fragment helper file-local for lambda context rendering. |
| working tree after `723f2017` | Pack lambda metadata into optional type storage | +14.44% | -5.63% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Replaces always-present lambda mangling fields on every `Type` with optional structured metadata and removes the last `named_itanium_abi_*` field plumbing from `dev/src`. |
| working tree after `ee6c7d96` | Pack IR substitution metadata into optional type storage | +12.70% | -5.06% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Moves per-IR-type substitution bridge fields and preregistered legacy keys behind optional metadata, reducing the active migration overhead while preserving substitution-slot compatibility. |
| working tree after `78a399aa` | Use named semantic kind for mangle placeholder checks | +12.65% | -5.07% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Replaces remaining `template-parameter`/`dependent` string-prefix checks in the mangler with `Type::named_semantic_kind`; builtin and local/anonymous namespace checks still need typed metadata. |
| working tree after `bb98efbf` | Hash substitution lookup tables | +10.10% | -4.99% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Keeps substitution-slot order in the registration vector but changes string and typed substitution lookup indexes from ordered maps to hash maps, avoiding recursive key comparisons on the typed IR path. |
| `ede58e93` | Dispatch function-name mangling and checkpoint real fallback state | +10.12% | -4.98% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. A temporary counter run showed function-name checkpointing logged about 1.42M substitutions with zero rollbacks on `semantic_overload`; the final slice emits typed function names directly and keeps checkpoints only around real fallback sites. |
| `f43bd806` | Dispatch type IR by semantic type kind | +10.00% | -4.97% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. After the context-free fast path, non-named wrapper types now go straight to the wrapper builder instead of probing all named/dependent/class-template builders first. |
| `2073770a` | Skip legacy lookup on ordinary IR substitution misses | +9.92% | -4.90% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Ordinary IR substitution misses no longer synthesize legacy string keys just to probe the old string map; explicit legacy bridge keys still use the old map. |
| `5fd15e3d` | IR-only substitution state for uncaptured plain-function renders | +9.81% | -4.85% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Complete plain-function IR renders now suppress legacy type substitution keys and avoid mirroring IR substitutions into the legacy string map when no captured substitution state is needed. |
| working tree after `6c5f1ce8` | Remove per-type legacy substitution overrides | +6.67% | -4.93% | Perf gate failed on instructions but recovered about three percentage points from the previous accepted migration state. Full strict and PA32/PA33 report passed; typed decltype lookup now bridges deterministic structural keys instead of storing per-type legacy override keys. |
| working tree after `6ef3f277` | Preserve typed lambda-context substitution slots | +6.79% | -5.01% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Carries IR-only enclosing-context substitution slots for nested lambdas and restores template-prefix slot registration for single-argument `std::operator<<`, fixing PA32/PA33 reference substitution spellings without adding text reparsing. |
| working tree after `62d558b8` | Strict closure for owner NTTP, pack NTTP, and decltype cast IR | +7.44% | -5.01% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Adds typed owner non-type template argument forwarding, non-type pack argument expressions, and function-parameter/member/conversion/cast dependent-expression IR without restoring text/AST fallback. |
| working tree after `d17bc622` | Remove legacy AST/template-id mangling emitters | +7.45% | -5.04% | Perf gate failed on instructions; full strict and PA32/PA33 report passed. Deletes the old template-id, qualified-name, dependent-expression, type-id, type-specifier, and declarator AST string emitters; surviving AST entry points build typed IR and emit through the IR backend. |
| working tree after `58bd2ba6` | Shrink function-template argument IR payloads | +7.08% | -5.47% | Perf gate failed on instructions but improved from the prior slice; full strict and PA32/PA33 report passed. Changes `itanium_mangle_ir::TemplateArgument` optional type payloads from always-present `Type` values to allocated-on-use children, reducing the struct from 1248 bytes to 216 bytes. |
| working tree after `30d863ff` | Shrink dependent-expression IR payloads | +6.96% | -5.43% | Perf gate failed on instructions but improved from the prior slice; full strict and PA32/PA33 report passed. Changes dependent-expression owner/value type payloads from an always-present `Type` value to an allocated-on-use child, reducing the struct from 720 bytes to 208 bytes. |
| working tree after `fece9acf` | Pack type lambda metadata into optional storage | +6.95% | -5.54% | Perf gate failed on instructions but improved slightly from the prior slice; full strict and PA32/PA33 report passed. Moves lambda-only type context/source/discriminator fields behind optional metadata, reducing ordinary `Type` from 528 bytes to 424 bytes. |
| working tree after `050053a9` | Compact builtin type-code storage | +6.66% | -5.57% | Perf gate failed on instructions but improved from the prior slice; full strict and PA32/PA33 report passed. Stores builtin Itanium type terminals in a fixed two-character buffer instead of a full `std::string`, reducing ordinary `Type` from 424 bytes to 392 bytes. |
| working tree after `fd846fe3` | Use hashed pointer cache for context-free type IR | +6.64% | -5.46% | Perf gate failed on instructions but improved slightly from the prior slice; full strict and PA32/PA33 report passed. Switches the existing `Type*` plan cache from ordered maps to `unordered_map`, keeping cache semantics unchanged. |
| working tree after `42eaf141` | Pack named type IR metadata into optional storage | +5.48% | -5.66% | Perf gate failed on instructions but improved from the prior accepted slice; full strict and PA32/PA33 report passed. Moves named/class-template-only fields behind optional metadata, reducing ordinary `Type` from 392 bytes to 256 bytes. |
| working tree after `bd4789b1` | Pack template-argument IR metadata into optional storage | +5.33% | -5.68% | Perf gate failed on instructions but improved from the prior accepted slice; full strict and PA32/PA33 report passed. Moves template-entity, external-entity, and pack-only fields behind optional metadata, reducing `ClassTemplateArgument` and `TemplateArgument` from 216 bytes to 80 bytes. |
| working tree after `d031cfe2` | Pack lambda function-encoding metadata into optional storage | +5.12% | -5.70% | Perf gate failed on instructions but improved from the prior accepted slice; full strict and PA32/PA33 report passed. Moves lambda-only function-name fields behind optional metadata, reducing `FunctionEncoding` from 648 bytes to 520 bytes. |
| working tree after `ea759aaa` | Avoid per-call elaborated-prefix string construction | +4.33% | -5.69% | Perf gate failed on instructions but improved from the prior accepted slice; full strict and PA32/PA33 report passed. `strip_elaborated_type_prefix` now compares against literal prefix spans instead of constructing a `std::string` for every prefix probe. |
| working tree after `f0daccf3` | Use explicit ASCII whitespace in mangle text helpers | +3.80% | -5.64% | Perf gate failed on instructions but improved from the prior accepted slice; full strict and PA32/PA33 report passed. Local `trim_space` and `remove_space_chars` avoid the C library classifier on hot mangling text paths. |
| working tree after `e08b1af1` | Fuse elaborated-prefix stripping with trimming | +2.92% | -5.63% | Perf gate failed at the default 1% instruction tolerance but is below the active 3% migration ceiling; full strict and PA32/PA33 report passed. Common `trim_space(strip_elaborated_type_prefix(...))` sites now avoid building an intermediate stripped string. |
| working tree after `a6fc6ee` | Fast elaborated-prefix dispatch and qualified-name append | +2.77% | -5.65% | Passed the active 3% migration ceiling; full strict and PA32/PA33 report passed. Prefix stripping now dispatches by first character, and qualified component joining reserves once instead of using chained string concatenation. |
| working tree after `6fe86ca2` | Normalize type-parameter spelling checks and tune IR substitution lookup | +1.65% | -5.56% | Passed the active 3% migration ceiling; full strict and PA32/PA33 report passed. Accepted cleanup removes repeated `typename `/`class ` concatenations, caches recursive `SubstitutionKey` hashes, avoids duplicate hash probes, mutates type substitution metadata in place, lazily reserves substitution state capacity, and linearly scans small typed substitution states before using the hash table. |

Rejected/uncommitted experiments:

| Slice | Instructions | Peak footprint | Result |
| --- | ---: | ---: | --- |
| Stop mirroring IR substitutions into legacy string slots for all context-free type IR | not measured | not measured | Rejected before perf: strict LowIR compare changed PA22 symbols in qualified-member/dependent expression cases. |
| Context-free class-template IR plus unqualified plain-function lookup-scope expansion | not measured | not measured | Rejected before perf: strict LowIR compare failed PA22 `474-defaulted-decltype-empty-pack-instantiation.t`. |
| Move-only IR builder/factory cleanup | +10.05% | -4.98% | Rejected after strict/report/perf: restoring the by-value factory API avoided the suspected extra copies, but the remaining local move-only slice did not improve the current GCC perf state. |
| Lazy legacy materialization for IR substitution slots | +10.51% | -5.31% | Rejected after strict/report/perf. Temporary counters showed `449,210` IR-only registrations, `413,250` materialized slots, and `1,124,648` materialization calls on `semantic_overload`, so most IR slots still reached legacy consumers and the extra slot bookkeeping made instructions worse. |
| Typed function-type prerequisite registration | +9.97% | -4.92% | Rejected after strict/report/perf: replacing one legacy wrapper-string parser with typed prerequisite discovery did not improve the current GCC perf state. |
| Extend IR-only substitution suppression to simple member/special-member functions | +9.93% | -4.92% | Rejected after strict/report/perf: broadening the partial suppression beyond complete plain-function renders regressed from the accepted `+9.81%` state, so the next step should remove the legacy bridge instead of widening conditional suppression. |
| Optional `FunctionEncoding` conversion type storage | +7.10% | -5.45% | Rejected after strict/report/perf: reducing `FunctionEncoding` size from 920 bytes to 400 bytes regressed instructions from the accepted `+6.96%` state, so the always-present conversion type was restored. |
| Memoize legacy substitution-key parse bridge | +6.96% | -5.51% | Rejected after strict/report/perf: the memoization overhead regressed from the accepted `+6.64%` state, so the pure parser remains uncached while the remaining legacy bridge is removed or optimized directly. |
| Direct substitution sequence append | +6.70% | -5.58% | Rejected after strict/report/perf: avoiding the temporary `S_`/`S0_` string did not improve the accepted `+6.64%` GCC state, so the simpler helper was restored. |
| Pack dependent-expression IR metadata into optional storage | +5.43% | -5.72% | Rejected after strict/report/perf: reducing `DependentExpression` from 208 bytes to 40 bytes regressed instructions from the accepted `+5.33%` state, so the inline representation was restored. |
| Optional `FunctionEncoding` conversion type storage after IR compaction | +5.21% | -5.63% | Rejected after strict/report/perf: reducing `FunctionEncoding` from 520 bytes to 272 bytes still regressed instructions from the accepted `+5.12%` state, so the inline conversion type remains. |
| Pack function-name component metadata into optional storage | +5.14% | -5.69% | Rejected after strict/report/perf: reducing `FunctionNameComponent` from 160 bytes to 88 bytes regressed instructions from the accepted `+5.12%` state, so the inline component payload remains. |
| Compact standard-substitution spelling as enum | +5.35% | -5.48% | Rejected after strict/report/perf: replacing finite `Sa`/`Sb`/`Ss`/`Si`/`So`/`Sd` strings with an enum regressed instructions from the accepted `+5.12%` state, so the string payload remains. |
| Avoid function-name prefix slice allocation | +5.49% | -5.66% | Rejected after strict/report/perf: rendering namespace/member prefixes from a counted view avoided one temporary vector copy but regressed instructions from the accepted `+5.12%` state, so the simpler slice path remains. |
| Move-aware IR wrapper factories | +5.17% | -5.67% | Rejected after strict/report/perf: adding rvalue overloads and moving local IR temporaries reduced sampled copy/destructor pressure but did not improve the accepted `+5.12%` instruction state, so the simpler factory API remains. |
| ASCII identifier classification in mangle helpers | +4.24% | -5.66% | Rejected after strict/report/perf: replacing locale-aware identifier checks regressed instructions from the accepted `+3.80%` state, so the existing classifier remains. |
| `memcmp` elaborated-prefix comparisons | +3.12% | -5.34% | Rejected after strict/report/perf: replacing `std::string::compare` with `memcmp` inside the first-character prefix dispatch regressed from the accepted fused-trim state and did not pass the active 3% ceiling. |
| String substitution small-state vector lookup | +1.93% | -5.60% | Rejected after strict/perf: scanning legacy string substitution keys was worse than the accepted typed-only small-state lookup. |
| Span-based type-parameter spelling normalization | +2.01% | -5.60% | Rejected after strict/perf: replacing selected temporary strings with local span comparisons added enough overhead to lose the accepted cleanup gain. |
| Direct cached type-IR emit | +1.94% | -5.60% | Rejected after strict/perf: emitting directly from cached entries made the hot path slower than copying the cached IR into the existing renderer path. |
| Lazy typed substitution hash index | +1.98% | -5.60% | Rejected after strict/perf: the corrected lazy-index version preserved substitution spelling but was slower than eagerly maintaining the hash table with a small-state scan. |
| Pointer-keyed selected named-type text cache | +1.69% | -5.60% | Rejected after strict/perf: the cache was nearly flat but still slightly worse than the accepted state, and pointer-identity caches have already proven risky in this path. |
| Typed substitution small-state threshold 24 | +1.73% | -5.60% | Rejected after perf: scanning too many typed substitution keys was slower than the accepted threshold of 12. |
| Typed substitution small-state threshold 8 | +2.10% | -5.64% | Rejected after perf: the lower threshold missed useful vector-scan wins and regressed from the accepted threshold of 12. |
