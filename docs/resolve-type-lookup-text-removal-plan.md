# `resolve_type_lookup_text` Removal Plan

## Goal

Remove `resolve_type_lookup_text(...)` completely from production compiler
code. Any caller that currently resolves type semantics by sending normalized
text through this helper should instead use already-available semantic facts:
`CppAstNode` type-id syntax, `TemplateIdSyntax`, `QualifiedName`, owner
`TypePtr`, `TemplateArgumentSyntax`, `TemplateArgument`, or
`TemplateNamedTypeMetadata`.

This is not a ref-update exercise. Witness, LowIR, or exit-code drift should be
treated as a sign that the replacement lost semantic information unless a
clang-generated reference or the assignment contract proves otherwise.

## Current Inventory

Current grep:

```sh
rg -n '\bresolve_type_lookup_text\b' dev/src scripts docs
```

Live compiler inventory:

- `36` occurrences under `dev/src`
- `27` real compiler call sites
- `9` declarations, definitions, or wrapper forwarding sites

Real call-site distribution:

- `19` in `dev/src/template_argument_semantics.cpp`
- `3` in `dev/src/template_resolution.cpp`
- `2` in `dev/src/semantic_class_model.cpp`
- `1` in `dev/src/callsemantic/constant_value_lookup.cpp`
- `1` in `dev/src/template_decl_ast.cpp`
- `1` in `dev/src/template_specialization.cpp`

Wrapper and API surface to delete at the end:

- `template_api::type::resolve_type_lookup_text(...)`
- `template_argument_semantics::resolve_type_lookup_text(...)`
- `semantic_dependent_type::resolve_type_lookup_text(...)`
- `template_type_ops::resolve_type_lookup_text(...)`
- script audit exceptions that still mention the API

## Strategy

Use the strict suite as the discovery mechanism, but make each fix by moving
semantic information to the caller instead of adding another text parser.

The practical loop is:

1. Start from a clean baseline.
2. Poison one remaining text path or call-site group.
3. Run strict with direct LowIR compare.
4. Record the exact failing tests and failure mode.
5. Replace that call-site group with typed semantic data.
6. Rerun strict before moving to the next group.

The poison can be temporary local code, not necessarily committed. The useful
form is an unconditional hard fail at the entry of
`resolve_type_lookup_text(...)`:

```cpp
semantic_fallback_audit::hard_fail(
    "resolve-type-lookup-text",
    source_location,
    "type lookup reached text resolver [name " + text + "]");
```

If that produces too much noise, move the hard fail outward and poison one
family at a time. The important rule is that failures should point at the
semantic fact that is missing, not at an incidental spelling rewrite.

## Validation Command

Run this after every replacement slice:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

For quicker iteration after the build is current:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict-nobuild \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

If a fix touches a construct owned by a non-strict PA, also run the targeted
report for that PA:

```sh
make test-report \
  ACTIVE_TEST_REPORT_PAS='paXX' \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

## Replacement Building Blocks

Prefer these existing structured paths before adding anything new:

- `parse_type_id_node_for_templates(...)` when a `CppAstNode` type-id is
  available.
- `resolve_template_id_syntax_type(...)` when a `TemplateIdSyntax` is
  available.
- `TemplateTypeLookupRequest` when lookup is a real name lookup request.
- `TemplateArgumentSyntax` when explicit template arguments are available.
- `TemplateArgument` and `TemplateNamedTypeMetadata` when arguments or names
  are already semantically bound.
- owner `TypePtr` plus member path for dependent or concrete qualified-member
  types.

Add narrow typed helpers only when the same semantic pattern appears at several
sites. Good candidate helpers:

- resolve a type from `QualifiedName` plus qualifier template-id syntax
- resolve a member type from owner `TypePtr` plus member name/template args
- resolve a type argument from `TemplateArgumentSyntax` without falling back to
  the syntax text
- resolve default template arguments from typed parameter and argument state

Avoid helpers that take arbitrary text plus optional syntax. That shape is the
current problem.

## Work Slices

### 1. Baseline And Tracker

Run strict with direct LowIR compare before touching code and record the
current floor. Then add a small tracker section to this file with:

- failing test path
- failure kind: exit, witness, LowIR, or compile timeout
- call-site group
- intended semantic replacement
- status

Do not update witness refs during this slice.

### 2. Template Argument Semantics Internal Callers

This is the largest slice: `19` real call sites live in
`template_argument_semantics.cpp`.

Expected groups:

- type-id parser hooks currently falling from structured node lookup to text
  lookup
- qualified owner and member lookup that builds owner text such as
  `A::B<T>`
- leaf constexpr/type hooks that receive an AST node but still recover by text
- value-initialization callee classification from a callee spelling
- semantic-only type argument paths that rewrite bound aliases, packs, or
  `typename` text before resolving

Replacement direction:

- keep `lookup_type_node` and `parse_type_id_node_for_templates` as the AST
  entrypoints
- route template-id spellings through `resolve_template_id_syntax_type`
- carry owner type and member path instead of recomputing owner text
- resolve dependent names using `NamedSemanticKind` and metadata, not string
  prefixes
- when only text is available, move the missing `TemplateArgumentSyntax` or
  `CppAstNode` to the caller rather than making the text resolver smarter

Validation:

- strict direct LowIR compare after each small group
- if a group changes dependent witness spelling, isolate the single test before
  broad changes and verify the semantic owner is still the same

### 3. Cross-Module Wrapper Callers

Once the internal sites are reduced, update the external callers.

`semantic_class_model.cpp`:

- base specifier handling should use the base type-id node or
  `TemplateIdSyntax` directly, not `resolve_base_type_text(...)`
- class-use recording for initializer template-ids should pass the
  initializer's `TemplateIdSyntax` to a typed resolver

`template_resolution.cpp`:

- deduction lookup should take a typed request or already-resolved `TypePtr`
- explicit type argument fallback should use `TemplateArgumentSyntax`
- recoverable bound template-id logic should use bound `TemplateArgument`
  metadata instead of reparsing the template-id text

`constant_value_lookup.cpp`:

- builtin trait arguments such as `is_same<T, U>` should resolve from each
  argument's `TemplateArgumentSyntax`; if the syntax is missing, fix syntax
  capture at the trait construction site

`template_decl_ast.cpp` and `template_specialization.cpp`:

- replace local fallback bridges with typed entrypoints
- if partial specialization matching receives text-only default arguments,
  classify that as missing syntax capture and fix the producer

Validation:

- strict direct LowIR compare after each file
- targeted `test-report` for the earliest PA that owns the construct if strict
  does not cover it

### 4. Collapse The API

After all real callers are gone:

1. Delete the declarations from public headers.
2. Delete wrapper definitions in `semantic_dependent_type.cpp` and
   `template_type_ops.cpp`.
3. Delete the implementation in `template_argument_semantics.cpp`.
4. Remove audit script allow-list references.
5. Run:

```sh
rg -n '\bresolve_type_lookup_text\b' dev/src scripts
```

Expected result: no production or audit-script hits.

### 5. Final Validation

Required final checks:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++

make test-report \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

If strict exposes witness-only drift, do not update refs first. Debug the typed
replacement until semantic output matches. Only regenerate refs from clang when
the reference source itself is known to have changed.

## Completion Criteria

- `rg -n '\bresolve_type_lookup_text\b' dev/src scripts` has no hits.
- The strict suite passes with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1`.
- The full `test-report` passes.
- No replacement path parses template-id, qualified-owner, `typename`, or
  dependent-name structure from arbitrary normalized type text.
- Remaining text in type diagnostics is only reporting identity, not semantic
  control flow.
