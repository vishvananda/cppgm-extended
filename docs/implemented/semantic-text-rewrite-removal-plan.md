# Semantic Text Rewrite Removal Plan

## Goal

Remove the remaining semantic `rewrite_*_in_text` APIs and call paths by
preserving typed semantic data across template substitution, default argument
instantiation, dependent alias resolution, pack expansion, and source witness
capture.

The current renderer no longer reparses witness pack text. The remaining text
rewrites are semantic recovery bridges: they substitute names or packs in text
and then parse or compare the resulting text. This plan replaces those bridges
with typed substitution and typed pack expansion.

## Current Rewrite Clusters

1. Current-specialization witness source spelling
   - `rewrite_current_specialization_names_in_text`
   - Scope: `dev/src/callsemantic/template_source_utils.cpp`
   - Replacement: compute semantic source argument text from the resolved
     `TemplateArgument` and current-specialization metadata, not by replacing
     identifiers in a binding string.

2. Dependent type lookup preparation
   - `rewrite_bound_type_names_in_text`
   - `rewrite_bound_template_names_in_text`
   - `rewrite_bound_value_names_in_text`
   - `rewrite_visible_named_type_aliases_in_text`
   - Scope: `resolve_instantiated_dependent_type`,
     current-specialization lookup, `decltype`/`typeof` handling, dependent
     alias instantiation.
   - Replacement: carry structured syntax and resolve/substitute through
     `TypePtr`, `TemplateArgument`, and `CppAstNode` instead of text.

3. Pack expansion in text
   - `rewrite_single_bound_type_packs_in_text`
   - `rewrite_bound_type_packs_in_text`
   - `rewrite_single_bound_value_packs_in_text`
   - Scope: default arguments, function type parameter lists, decltype
     expressions, dependent alias instantiation.
   - Replacement: introduce typed expansion helpers that return
     `vector<TypePtr>`, `vector<TemplateArgument>`, or substituted
     `CppAstNode` sequences.

4. Default argument recovery
   - `rewrite_bound_default_type_argument_text`
   - `rewrite_bound_default_expression_text`
   - Scope: `dev/src/template_resolution.cpp`
   - Replacement: instantiate `TemplateParameterInfo::default_argument`
     directly using bound template arguments. Text should remain only as a
     display/source fallback after the typed result is known.

5. Dependent alias instantiation
   - Local text pipeline in `callsemantic.cpp` around dependent alias replay.
   - Replacement: replay from `AliasTemplateDecl` plus
     `DependentAliasTemplateArgumentSyntax` and structured resolved arguments.

6. Miscellaneous semantic comparison/display callers
   - Friend class names, dependent qualified names, out-of-class member
     template parameter comparison.
   - Replacement: store typed friend/dependent-name data where possible, and
     compare normalized typed structures rather than rewritten strings.

## Implementation Stages

1. Remove current-specialization source rewrite fallback.
   - Use current-specialization semantic binding text when available.
   - When source text mentions a current specialization but a direct source
     rewrite is not possible, use the resolved `TemplateArgument` witness text
     as the semantic spelling.
   - Delete `rewrite_current_specialization_names_in_text`.

2. Add typed pack expansion primitives.
   - Represent expansion of a type pack or value pack as structured elements.
   - Teach template argument input expansion to prefer substituted
     `TemplateArgumentSyntax`/`CppAstNode` data and use text only for display.

3. Replace default argument text recovery.
   - Instantiate default type arguments via syntax/type nodes.
   - Instantiate default non-type arguments via expression nodes.
   - Keep the existing text bridge only as an assertion/fallback during the
     transition, then remove it.

4. Replace dependent alias text replay.
   - Resolve alias bodies from the stored alias declaration and structured
     dependent arguments.
   - Preserve source/witness display text separately from semantic resolution.

5. Replace dependent type lookup text preparation.
   - Extend dependent type nodes so their semantic payload includes structured
     owners, template ids, and argument syntax.
   - Resolve current-specialization and dependent qualified members from that
     payload instead of rewritten lookup text.

6. Remove semantic context rewrite APIs.
   - Delete virtual methods from `SemanticContext`.
   - Delete template API wrappers and implementations once all call sites are
     migrated.
   - Keep normalization/formatting helpers that only format already-typed data.

## Validation

Use the strict witness suite after each stage:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

For smaller stages, first run:

```sh
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict-nobuild STRICT_PAS='pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```
