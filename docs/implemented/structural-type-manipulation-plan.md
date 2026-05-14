# Implementation Plan: Structural Type Manipulation

Status note:

- The early structural cleanup in this plan was implemented.
  In particular, CV normalization at type construction and the removal of the
  `spelled_top_level_cv_flags()` path both landed.
- The later phases were explored but not carried through as originally written.
  They did not produce enough compile-time improvement to justify the added
  complexity, so the remaining text-assisted structural cleanup was intentionally
  abandoned rather than kept as active plan work.

This document is therefore archived as implemented/closed:

- implemented where the early high-value pieces paid off
- intentionally not pursued further where the later phases did not justify
  themselves

## Goal

Replace text-based type manipulation in the template deduction and substitution paths with structural operations on `TypePtr` trees. This eliminates a class of bugs where whitespace, elaboration prefixes, or display name formatting cause silent deduction failures.

## Background

The `Type` struct (in [`cpp_decl_model.h:24-76`](../dev/src/cpp_decl_model.h)) already provides structural representation with:

- `TK_CV` nodes wrapping an `inner` type, with explicit `cv_const`/`cv_volatile` flags
- `TK_NAMED` nodes with `named_key` (canonical) and `named_display` (human-readable) fields
- `TK_POINTER`, `TK_LVALUE_REFERENCE`, `TK_RVALUE_REFERENCE`, `TK_FUNCTION`, etc.
- Factory functions: `make_cv()`, `strip_top_level_cv()`, `apply_cv()`, `type_equals()`, etc.

The problem is that `TK_NAMED` types sometimes encode CV qualification in the `named_display` string (e.g., `"const Foo"`) rather than wrapping in a `TK_CV` node. The text-based functions exist as workarounds for this inconsistency.

## Phases

### Phase 1: Normalize CV Representation at Construction Time

**Goal**: Ensure every CV-qualified named type is represented as `TK_CV(TK_NAMED(...))`, never as `TK_NAMED("const Foo", ...)`.

**Files to change**:
- `cpp_decl_model.cpp` / `cpp_decl_model.h` -- `make_named()` and any other `TK_NAMED` construction sites
- `typesemantic.cpp` -- wherever named types are built from token streams

**Steps**:

1. Add an assertion or normalization to `make_named()` that strips leading `const`/`volatile` from `display_name` and wraps the result in `make_cv()` if qualifiers were present. This is the single choke point -- all named types flow through `make_named()`.

2. Audit callers of `make_named()` to confirm none depend on CV being part of the display string. Search for patterns like:
   - `named_display.find("const")` or `.compare(0, 6, "const ")`
   - Direct reads of `named_display` that check for qualifiers

3. Run the existing test suite. Failures will reveal downstream code that assumed CV was encoded in the display name.

**Validation**: After this phase, `spelled_top_level_cv_flags()` should never reach its string-parsing fallback (lines 838-860). Add a debug assertion there to confirm, then remove the fallback once stable.

### Phase 2: Replace `spelled_top_level_cv_flags()` with Structural Check

**Goal**: Eliminate the text-based CV detection entirely.

**File**: [`template_resolution.cpp:822-861`](../dev/src/template_resolution.cpp)

**Steps**:

1. Replace `spelled_top_level_cv_flags()` with a thin wrapper around the existing structural `top_level_cv_flags()` function. After Phase 1 normalization, the structural version will always work.

2. Update all call sites (primarily in `deduce_template_argument_impl` around lines 2494-2562) to use `top_level_cv_flags()` directly.

3. Remove the `spelled_top_level_cv_flags()` function.

**Call sites to update** (search for `spelled_top_level_cv_flags`):
- `template_resolution.cpp:2494-2498` (pattern CV extraction)
- `template_resolution.cpp:2499-2502` (actual CV extraction)
- `template_resolution.cpp:2526-2528` (non-partial CV stripping)
- `template_resolution.cpp:2551-2558` (named parameter CV stripping)

### Phase 3: Structural Alias Template Expansion

**Goal**: Replace `rewrite_alias_template_parameter_texts()` with structural type substitution.

**Files**:
- [`template_resolution.cpp`](../dev/src/template_resolution.cpp) -- text-based alias-template argument rewriting used during deduction
- [`template_specialization.cpp`](../dev/src/template_specialization.cpp) -- text-based alias-template expansion and specialization matching
- [`semantic_model.h`](../dev/src/semantic_model.h) -- `AliasTemplateDecl` storage for the structural alias pattern

**Steps**:

1. Add a structural alias expansion function that takes the alias template's pattern `TypePtr` and a vector of `TypePtr` arguments, and returns a substituted `TypePtr`. This mirrors what `rewrite_alias_template_parameter_texts` does but on the type tree:

   ```
   TypePtr substitute_alias_template_type(
       const TypePtr & pattern,
       const std::vector<TemplateParameterInfo> & parameters,
       const std::vector<TypePtr> & arguments);
   ```

2. The function walks the pattern type tree recursively:
   - `TK_NAMED` where `named_key` matches a parameter name: replace with the corresponding argument type
   - `TK_CV`, `TK_POINTER`, `TK_REFERENCE`, `TK_ARRAY`, `TK_FUNCTION`: recurse into children, reconstruct if any child changed
   - `TK_FUNDAMENTAL`: return as-is

3. This requires that `AliasTemplateDecl` stores a `TypePtr` for the alias pattern, not just an AST node. If `alias_template->type_id` is currently only an AST node:
   - Add a `TypePtr resolved_type` field to `AliasTemplateDecl`
   - Populate it when the alias template is first analyzed
   - Use it in both deduction and specialization matching instead of `rebuild_node_text()`

4. Update both alias expansion entry points to call the structural version:
   - deduction-side alias expansion in `template_resolution.cpp`
   - specialization-side alias expansion in `template_specialization.cpp`
   Keep the text-based version as a temporary fallback behind a debug flag during transition if needed.

5. Remove `rewrite_alias_template_parameter_texts()` once the structural version is validated.

### Phase 4: Structural Template ID Matching in Deduction

**Goal**: Replace the three-fallback text-based template ID matching in deduction with structural comparison.

**File**: [`template_resolution.cpp:2580-2640`](../dev/src/template_resolution.cpp)

The current code at line 2580-2640 converts types to text, strips elaborated prefixes, parses template names, splits argument lists, and then compares. This is the most complex text-based path.

**Steps**:

1. Add structural accessors to `TK_NAMED` types that expose template identity:

   ```
   // Returns true if this named type is a template instantiation,
   // and extracts the template name key and argument types.
   bool decompose_template_instantiation(
       const TypePtr & type,
       std::string & template_key,
       std::vector<TypePtr> & arguments);
   ```

   This information may already be available via `ClassInfo::instantiation_arguments` for class types. The function should check whether a `ClassInfo` exists for the type and extract from there.

2. Replace the text-based comparison block (lines 2580-2640) with:

   ```
   std::string pattern_template_key, actual_template_key;
   std::vector<TypePtr> pattern_template_args, actual_template_args;
   if(decompose_template_instantiation(pattern_base, pattern_template_key, pattern_template_args) &&
      decompose_template_instantiation(actual_base, actual_template_key, actual_template_args) &&
      pattern_template_key == actual_template_key) {
       // Structurally deduce from each argument pair
       ...
   }
   ```

3. Remove `type_argument_text_for_deduction()`, `strip_elaborated_type_prefix()`, `named_type_head_text()`, `split_top_level_comma_list()`, `parse_matching_template_id_pair()`, and `canonicalize_template_id_text` once all call sites are converted.

### Phase 5: Structural Pack Expansion in Function Parameters

**Goal**: Replace `contains_identifier_token_text()` with scope-based pack lookup.

**Primary file**: [`template_instantiation.cpp:691-703`](../dev/src/template_instantiation.cpp)

**Related audit sites**:
- [`semantic_output.cpp`](../dev/src/semantic_output.cpp)
- [`semantic_expression.cpp`](../dev/src/semantic_expression.cpp)

The first required fix is in `template_instantiation.cpp`, but the same token-text pack matching pattern appears in a few other places and should be audited before calling the phase complete.

**Steps**:

1. Instead of text-searching parameter declarations for pack names, track which template parameters are packs at the point they're bound into the scope. The scope already has `named_type_packs` -- the issue is identifying which packs apply to a given parameter declaration.

2. When a parameter is identified as a pack parameter (via `declarator_has_parameter_pack()`), look up the pack by the parameter's declared type name in the scope's `named_type_packs` directly, rather than searching the parameter's text for any pack name that happens to appear.

3. This requires that the parameter declaration's type is resolved enough to identify which named types it references. If the parameter type has already been partially resolved, extract the pack name from the type structure rather than the source text.

**Simpler interim fix**: If full structural resolution is too invasive here, at minimum replace `contains_identifier_token_text()` with a token-aware check that only matches whole identifiers at the type-name position, not inside nested template arguments.

## Ordering and Dependencies

```
Phase 1 (CV normalization)
   |
   v
Phase 2 (remove spelled_top_level_cv_flags)
   |
   v
Phase 3 (structural alias expansion across resolution + specialization)
   |
   v
Phase 4 (structural template ID matching)  -- depends on Phase 1 + 3
   |
   v
Phase 5 (structural pack expansion)  -- start in template_instantiation.cpp, then audit sibling text-match helpers
```

Phases 1 and 2 are the highest value. They fix the most common failure mode (CV-qualified types not matching during deduction) with the lowest risk, since they normalize at construction time and all downstream code gets the fix automatically.

## Testing Strategy

Each phase should be validated by:

1. Running the existing compiler test suite after each change
2. Adding targeted test cases for the specific patterns that fail with text-based handling:
   - `const struct Foo` vs `const Foo` (elaborated type with CV)
   - Alias templates with nested template parameters in arguments
   - Templates with multiple pack expansions
   - Pack parameters inside nested types (`vector<T>` where `T` is a pack)
3. A before/after comparison: build a set of input files that currently produce errors, verify they compile after the fix

## Risk Assessment

- **Phase 1** (LOW risk): Normalizing at construction is additive -- it wraps types that should have been wrapped. If a caller breaks, it was already fragile.
- **Phase 2** (LOW risk): Removing dead code after Phase 1 validates the normalization.
- **Phase 3** (MEDIUM risk): Requires `AliasTemplateDecl` to store a resolved type. If alias templates are forward-referenced before their target type is resolved, this needs lazy resolution.
- **Phase 4** (MEDIUM risk): Most complex change. The text-based fallbacks exist because `named_key` isn't always sufficient to identify template instantiations. Need to ensure `ClassInfo::instantiation_arguments` is populated for all relevant types.
- **Phase 5** (LOW risk): Mostly a scope-lookup improvement. The interim fix (token-aware matching) is very low risk.
