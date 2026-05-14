# Bootstrap Dirty Tree Audit

Base: `HEAD` (`30a7c022`)

Scope of this audit:
- tracked changes in `dev/src/*` after the last commit
- classify pure debugging vs substantive behavior
- record the most likely target issue for each change so the tree can be cleaned up systematically

## Debug-Only Audit

Removed during this audit as temporary, issue-specific debugging:
- `dev/src/template_resolution.cpp`
  - targeted `basic_string` deduction tracing
  - targeted `operator==` finalization tracing
  - targeted `decompose-template-instantiation` tracing for `basic_string`
- `dev/src/semantic_expression.cpp`
  - `cast-debug` detail appended to unsupported-cast diagnostics

Kept as low-cost diagnostics worth preserving for now:
- `dev/src/callsemantic.cpp`
  - `expand-pack-argument-node` parser trace on pack-expansion rewrite failures
  - rationale: generic, trace-gated, and directly useful when template pack substitution goes wrong
- `dev/src/semantic_expression.cpp`
  - binary-operator overload/fallback trace notes
  - rationale: generic, trace-gated, and useful for ADL/operator debugging
- `dev/src/semantic_overload.cpp`
  - extra overload trace identity and member-template listing in failure diagnostics
  - rationale: generic candidate-disambiguation context
- `dev/src/semantic_output.cpp`
  - extra body-function binding context in output failures
  - rationale: only on hard failure; useful for scope-cache debugging

## Substantive Change Inventory

### 1. Binding Fingerprint / Cache Invalidation Family

These changes look like one coherent fix family: local scope bindings were mutating without bumping the binding fingerprint epoch, so cached lookup / deduction results could go stale.

- `dev/src/template_scope.cpp`
  - `overlay_selected_entries(...)` now returns whether it changed the target.
  - `overlay_scope_bindings_impl(...)` now tracks mutation and calls `bump_binding_fingerprint_epoch(target)` when bindings actually change.
  - Intended fix:
    - invalidate scope-dependent caches after overlaying template-bound names, packs, values, or templates.
  - Confidence: high.

- `dev/src/semantic_declaration.cpp`
  - adds `note_scope_binding_mutation(scope)` helper.
  - bumps fingerprint epoch after:
    - inline-namespace member injection
    - `using` declarations for types, namespaces, values, functions, templates
    - local type-alias declarations collected into scope
  - Intended fix:
    - avoid stale lookup after declaration-time scope mutation.
  - Confidence: high.

- `dev/src/semantic_statement.cpp`
  - adds the same mutation helper for statement scopes.
  - bumps fingerprint epoch after:
    - catch declarations
    - condition declarations
    - statement-level aliases and variables
    - `for`/range-`for` hidden temporaries and loop bindings
  - Intended fix:
    - avoid stale lookup/template binding inside statement scopes.
  - Confidence: high.

- `dev/src/semantic_output.cpp`
  - bumps fingerprint epoch when binding:
    - function parameters
    - pack sizes
    - value packs
  - Intended fix:
    - keep output-time function-scope lookup synchronized with newly bound parameters/packs.
  - Confidence: high.

- `dev/src/cpp_scope_lookup.h`
  - adds `lookup_results_same(TypePtr, TypePtr)` using structural `type_equals(...)`.
  - Intended fix:
    - make lookup-cache equality use semantic type equivalence instead of pointer identity.
  - Confidence: medium-high.

### 2. Inline Namespace / Equivalent Entity Lookup Family

These changes all point at one problem family: `std`/`std::__1`-style inline namespace exposure and `using`-directive import paths were producing either false ambiguity or missed lookup hits.

- `dev/src/semantic_lookup.h`
  - exports `lookup_direct_value(...)` and `same_value_binding_entity(...)`.

- `dev/src/semantic_lookup.cpp`
  - adds entity-equivalence helpers for:
    - template parameter lists
    - class templates
    - alias templates
    - value bindings
    - function bindings
  - adds `collect_decl_lookup_from_using_directives(...)`.
  - adds `lookup_unqualified_decl_with_entity_equivalence(...)`.
  - deduplicates appended functions/function templates using semantic equivalence rather than raw pointer identity.
  - changes qualified/unqualified value merging to use `same_value_binding_entity(...)` instead of raw pointer comparison.
  - changes class-template and alias-template lookup to use entity equivalence.
  - Intended fix:
    - collapse equivalent inline-namespace entities into one semantic result instead of treating them as ambiguous duplicates.
  - Confidence: high.

- `dev/src/callsemantic.cpp`
  - qualified type lookup now tries direct lookup plus recursive inline-namespace child lookup.
  - unqualified/qualified value lookup now uses `lookup_direct_value(...)` and `same_value_binding_entity(...)`.
  - Intended fix:
    - recover inline-namespace-exposed names from hosted STL and stop false ambiguity from equivalent imported bindings.
  - Confidence: high.

### 3. Function-Local Type Shadow / Lexical Class Lookup Family

These changes look tied to self-host failures where a function body introduces a local alias or where lexical/member lookup incorrectly treated the current method body like a foreign lexical context.

- `dev/src/callsemantic.cpp`
  - adds `lookup_exact_local_type_name(...)`.
  - `make_decl_hooks(...)` now prefers an exact local type binding before the broader `lookup_type(...)`.
  - Intended fix:
    - prefer true function-local type aliases that shadow outer bindings, especially `using _Up = ...` patterns in hosted utilities like `std::move`/`std::forward`.
  - Confidence: very high.

- `dev/src/semantic_expression.cpp`
  - adjusts lexical-class lookup:
    - introduces `has_lexical_class`
    - only uses `lexical_only` when the current function is not already a method on that same class
  - Intended fix:
    - stop misclassifying current-method lookup as “lexical-only”, which can hide real member bindings.
  - Confidence: medium-high.

- `dev/src/semantic_lookup.cpp`
  - same `has_lexical_class` / `lexical_only` refinement in function-template lookup.
  - Intended fix:
    - same lookup family as above, but for function templates.
  - Confidence: medium-high.

- `dev/src/callsemantic.cpp`
  - same `has_lexical_class` / `lexical_only` refinement in value lookup paths.
  - Intended fix:
    - same lookup family as above, but for values and member-value discovery.
  - Confidence: medium-high.

### 4. Template Pack Expansion / Pack Binding Family

- `dev/src/callsemantic.cpp`
  - pack collection now deduplicates `named_type_packs` and `named_value_packs` by name across the scope chain.
  - value-pack substitution now emits the constant literal when `ValueBinding` has a constant value, instead of always substituting the binding name.
  - Intended fix:
    - avoid duplicate pack discovery through nested overlays/shadows.
    - avoid producing rewritten expression text that still contains a non-constant dependent alias name when the actual pack element is a constant.
  - Likely symptom fixed:
    - pack-expansion parse failures or wrong substitution in template argument/value pack expansion.
  - Confidence: high.

### 5. Template Deduction Recovery / Partial Ordering Family

- `dev/src/template_resolution.cpp`
  - `decompose_template_instantiation(...)` can now recover missing template arguments from:
    - stored `instantiation_arg_texts`
    - member-scope named type bindings
    - member-scope value bindings
    - member-scope class/alias template bindings
  - Intended fix:
    - recover full template argument lists when `ClassInfo` stores incomplete or partially textual instantiation state.
  - Likely symptom fixed:
    - hosted STL template-id matching where the class knows its source template but not a fully materialized argument vector.
  - Confidence: high.

- `dev/src/template_resolution.cpp`
  - adds `function_template_parameter_has_non_dependent_binding(...)`.
  - adds `type_mentions_unbound_function_template_parameter(...)`.
  - `prepare_function_template_deduction_pattern(...)` now falls back to the original pattern if the resolved pattern erased still-unbound function template parameters.
  - `can_skip_resolved_non_dependent_pattern_check(...)` now also sees the original pattern and refuses to skip if the original still mentions a function template parameter.
  - Intended fix:
    - stop over-concretizing deduction patterns after partial binding.
  - Likely symptom fixed:
    - deduction failures where a resolved pattern looked non-dependent even though the original source pattern still depended on a function template parameter.
  - Confidence: high.

- `dev/src/template_resolution.cpp`
  - direct template-parameter matching now strips `typename`, `class`, and `struct` prefixes.
  - Intended fix:
    - recognize direct parameter matches from elaborated template argument text.
  - Confidence: high.

- `dev/src/template_specialization.cpp`
  - `deduce_from_named_template_id_text(...)` now allows the template-id head itself to be a direct template-template parameter.
  - it captures the actual template name text and feeds it into `deduce_template_template_parameter_from_text(...)`.
  - Intended fix:
    - allow deduction of template-template parameters from `X<...>`-style heads, not just from trailing template arguments.
  - Confidence: high.

- `dev/src/template_argument_semantics.cpp`
  - `scope_has_template_template_placeholder(...)` now treats a template name as a placeholder only when:
    - the name is marked template-bound, and
    - the bound class/alias template entry is actually `nullptr`
  - Intended fix:
    - stop treating real class/alias template bindings as unresolved template-template placeholders.
  - Confidence: high.

- `dev/src/semantic_overload.cpp`
  - adds partial-order placeholder-key collection and specificity comparison.
  - adds function-template parameter-count preference as a late tie-breaker when types already match.
  - Intended fix:
    - stabilize template partial ordering when transformed function types are otherwise equal.
  - Likely symptom fixed:
    - ambiguous function-template candidate selection in hosted/operator-heavy code.
  - Confidence: medium-high.

- `dev/src/callsemantic.cpp`
  - template-template arity extraction now treats a trailing parameter pack as “accepts any arity” by storing `size_t(-1)`.
  - Intended fix:
    - variadic template-template parameters should not be rejected for mismatched fixed arity.
  - Confidence: medium-high.

### 6. Call / Conversion Option Plumbing

These are mostly mechanical but still important because they keep the tree on the centralized options APIs introduced earlier.

- `dev/src/callsemantic.cpp`
  - some `analyze_call_expression(..., false)` call sites now use `semantic_overload::CallAnalysisOptions(false)`.

- `dev/src/semantic_overload.cpp`
  - same propagation of `CallAnalysisOptions(...)`.

- `dev/src/semantic_expression.cpp`
  - `try_argument_conversion(...)` calls now use `ArgumentConversionOptions(true)`.
  - nested call analysis also uses `CallAnalysisOptions(false)`.

- `dev/src/semantic_lifetime.cpp`
  - `try_argument_conversion(...)` uses `ArgumentConversionOptions(true)`.

- `dev/src/semantic_builtins.cpp`
  - `try_argument_conversion(...)` uses `ArgumentConversionOptions(true)`.

Assessment:
- likely little or no intended behavior change by themselves
- likely necessary follow-through after the callsemantic/output convergence work

### 7. Reference Cast / Equivalent Object-Type Family

- `dev/src/semantic_expression.cpp`
  - `static_cast` reference checks now normalize and compare:
    - raw object type
    - resolved dependent object type
    - unqualified object type
    - class identity
    - compatible top-level cv variants
  - Intended fix:
    - permit valid reference casts when the source and target are semantically the same object type after resolving aliases/dependent forms.
  - Likely symptom fixed:
    - hosted utility code using `static_cast<_Up&&>(...)` or equivalent reference-preserving cast patterns.
  - Confidence: high.

### 8. LowIR / Indirect-Value Materialization Family

- `dev/src/lowirgensemantic.cpp`
  - local-expression lowering now computes `local_indirect_value` from the local binding’s semantic type rather than from the node’s expression type.
  - Intended fix:
    - keep address-vs-value lowering correct when a local storage slot is indirect even if the expression node’s type has been transformed.
  - Likely symptom fixed:
    - by-value ABI / indirect-result materialization bugs in generated LowIR.
  - Confidence: medium-high.

### 9. Compatibility / Simplicity Workaround

- `dev/src/pptokenizer.cpp`
  - replaces `std::copy_n(...)` ring-buffer expansion with explicit loops.
  - Theory:
    - this was likely a self-host compatibility workaround rather than a semantic compiler fix.
    - likely intended to avoid a template/iterator/library path that the self-host compiler mishandled.
  - Confidence: medium.

### 10. Deferred Bootstrap Workaround: Namespace-Scope Aggregate Array Initialization

- `dev/src/runtime_symbol_policy.cpp`
  - rewrites the static `RuntimeSymbolTableEntry[]` definition into direct `if(name == ...)`
    dispatch to avoid the current bootstrap blocker.
  - real issue:
    - [lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp)
      still throws `unsupported global array initializer` for namespace-scope brace-initialized
      arrays whose elements are aggregates / class-like objects.
  - status:
    - explicitly deferred only to keep the bootstrap frontier moving.
    - this is not the intended long-term shape of `runtime_symbol_policy.cpp`.
  - regression follow-up:
    - add a dedicated repo-native regression for namespace-scope brace-initialized aggregate
      arrays once the current bootstrap blocker set is reduced.
    - place that regression in the earliest assignment that owns namespace-scope aggregate
      initialization lowering, then replace the `runtime_symbol_policy.cpp` workaround with the
      real compiler fix.
  - confidence:
    - very high.

## Recommended Systematic Follow-Up Order

1. Binding fingerprint / cache invalidation family
2. Inline namespace / equivalent-entity lookup family
3. Function-local type shadow + lexical class lookup family
4. Template deduction recovery / partial ordering family
5. Reference-cast equivalence family
6. LowIR indirect-value materialization family
7. Compatibility-only workarounds such as `pptokenizer.cpp`

This ordering should let us validate whole semantic clusters rather than flipping isolated hunks one at a time.

## Current Regression Failures On Dirty Tree

Full run on current dirty tree:
- `make test-report`
- result: `1613 / 1626`

Current failing cases:

### PA21

- [302-template-static-constant-minimum-chain.t](/Users/vishvananda/cppgm/pa21/tests/spec/302-template-static-constant-minimum-chain.t)
  - error: output mismatch
  - likely family: template deduction recovery / non-type binding

- [303-template-static-constant-nontype-argument.t](/Users/vishvananda/cppgm/pa21/tests/spec/303-template-static-constant-nontype-argument.t)
  - error: output mismatch
  - likely family: template deduction recovery / non-type binding

- [347-reference-shell-current-specialization-alias.t](/Users/vishvananda/cppgm/pa21/tests/spec/347-reference-shell-current-specialization-alias.t)
  - error: output mismatch
  - likely family: current-specialization / template lookup family

- [349-reference-shell-out-of-class-current-specialization-iterator.t](/Users/vishvananda/cppgm/pa21/tests/spec/349-reference-shell-out-of-class-current-specialization-iterator.t)
  - error: output mismatch
  - likely family: current-specialization / out-of-class template lookup family

- [385-member-template-shadowing-dependent-enable-if.t](/Users/vishvananda/cppgm/pa21/tests/spec/385-member-template-shadowing-dependent-enable-if.t)
  - error: output mismatch
  - likely family: shadowing / exact-local-type-name / template binding cleanup

- [390-out-of-class-conversion-operator-definition.t](/Users/vishvananda/cppgm/pa21/tests/spec/390-out-of-class-conversion-operator-definition.t)
  - error: output mismatch
  - likely family: out-of-class template lookup / binding fingerprint / output binding context

- [391-explicit-specialization-cross-converting-ctor-body.t](/Users/vishvananda/cppgm/pa21/tests/spec/391-explicit-specialization-cross-converting-ctor-body.t)
  - error: output mismatch
  - likely family: template specialization / template-id decomposition

- [445-default-template-argument-merge.t](/Users/vishvananda/cppgm/pa21/tests/spec/445-default-template-argument-merge.t)
  - error: output mismatch
  - likely family: template specialization / template argument merge

### PA18

- [190-bad-deduction.t](/Users/vishvananda/cppgm/pa18/tests/spec/190-bad-deduction.t)
  - error: expected failure, got success
  - likely family: template deduction recovery is now over-accepting

- [207-constructor-template-direct-other-specialization.t](/Users/vishvananda/cppgm/pa18/tests/spec/207-constructor-template-direct-other-specialization.t)
  - error: output mismatch
  - likely family: constructor template deduction / partial ordering

- [210-defaulted-nested-class-template-deduction.t](/Users/vishvananda/cppgm/pa18/tests/spec/210-defaulted-nested-class-template-deduction.t)
  - error: expected success, got failure
  - likely family: nested template deduction / template-id decomposition

### PA16

- [314-base-rvalue-reference-assignment.t](/Users/vishvananda/cppgm/pa16/tests/spec/314-base-rvalue-reference-assignment.t)
  - error: output mismatch
  - likely family: reference-cast / reference binding equivalence

### PA15

- [255-derived-pointer-member-init.t](/Users/vishvananda/cppgm/pa15/tests/spec/255-derived-pointer-member-init.t)
  - error: output mismatch
  - likely family: member lookup / current-specialization / pointer-member semantic drift

## Immediate Triage Order

Best first triage order based on concentration and likely shared causes:

1. PA18 template deduction failures
   - `[190]`, `[207]`, `[210]`
   - highest signal for whether the template-resolution family is over- or under-constraining deduction

2. PA21 current-specialization and template-specialization failures
   - `[347]`, `[349]`, `[385]`, `[391]`, `[445]`

3. PA21 non-type/static-constant failures
   - `[302]`, `[303]`

4. PA16/PA15 semantic drift cases
   - `[314]`, `[255]`

5. PA21 out-of-class conversion body
   - `[390]`
   - keep slightly later because it may collapse after the broader template/current-specialization fixes
