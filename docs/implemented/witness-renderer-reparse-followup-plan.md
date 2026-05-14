# Witness Renderer Semantic Data Plan

## Goal

Make the witness source formatter a formatter again. It should project
`SemanticSourceUse` rows into text, apply presentation-only normalization, and
dedupe repeated rows. It should not reopen the source file to infer facts that
semantic/template analysis already knew.

The strict witness output for `pa18 pa19 pa21 pa22` should not change.

## Current Source-Reparse Sites

`template_witness_renderer.cpp` still calls `read_source_lines(...)` from these
passes:

- `canonicalize_function_pointer_binding_args(...)`
- `drop_class_template_local_alias_source_events(...)`
- `drop_template_header_pattern_events(...)`
- `drop_member_alias_parameter_events(...)`
- `prefer_source_spelled_alias_events(...)`
- `synthesize_nested_template_source_events(...)`
- `normalize_event_names(...)`
- `normalize_event_locations_and_decls(...)`
- `normalize_source_defined_template_calls(...)`

Some of these passes are now redundant, but the durable fix is to replace each
remaining source question with typed semantic data.

## Semantic Data Additions Needed

### 1. Binding Parameter Shape

Formatter behavior:

- `canonicalize_function_pointer_binding_args(...)` scans a template
  declaration to decide whether a non-type parameter is a function pointer and
  whether binding `fn` should render as `&fn`.

Semantic replacement:

- Add `function_pointer_parameter` to `TemplateWitnessSourceBinding` and
  `semantic_source_use::SourceBinding`.
- Set it wherever class/function/alias/variable witness bindings are built by
  inspecting `TemplateParameterInfo::value_type`.
- Render `&name` from that flag, not by parsing declaration text.

### 2. Public Source Visibility

Formatter behavior:

- `drop_template_header_pattern_events(...)` finds template headers and drops
  class/function rows whose bindings mention template parameters.
- `drop_member_alias_parameter_events(...)` drops member-alias parameter rows by
  matching source-line prefixes.
- `drop_class_template_local_alias_source_events(...)` drops class rows whose
  binding argument is a class-template-local alias.
- `normalize_source_defined_template_calls(...)` drops most function/class/alias
  rows that were emitted while analyzing a template definition body.

Semantic replacement:

- Add a source-use visibility/context enum, for example:
  `PublicSource`, `TemplatePattern`, `TemplateDefinitionBodyProbe`,
  `MemberAliasParameter`, `DeclarationOnly`, and `InternalSubstitution`.
- Producers should only render `PublicSource` rows. Internal rows may still be
  recorded for debug output, but visible witness output should not need textual
  suppression.
- Thread the context through `ClassUseSourceDecision`,
  `FunctionCallSourceDecision`, `AliasUseSourceDecision`, and
  `VariableUseSourceDecision`.
- Set the context at emission time, where the compiler already knows whether it
  is checking a template parameter clause, a declaration pattern, a template
  definition body, a member alias parameter, or a real source use.
- Preserve the current exceptions explicitly: variable-template uses in
  template bodies remain public, and concrete source-spelled class/alias
  template-ids in template bodies remain public.

### 3. Source-Spelled Binding Arguments

Formatter behavior:

- `prefer_source_spelled_alias_events(...)` groups alias rows at the same
  location and keeps the one whose rendered bindings appear on the source line.
- `normalize_source_defined_template_calls(...)` also uses this source-line
  match to decide whether an alias row in a template body is a real
  source-spelled template-id.

Semantic replacement:

- Add a per-binding source-spelled decision to `SourceBinding`, not just an
  occurrence-level flag. The producer must say whether the rendered binding
  argument was actually chosen from the explicit source argument text.
- Preserve the source argument fragments used to make that decision for packs.
  A single string is not enough: `cat_t<T0, Ts...>` can render one binding as
  `T0`, while `Wrap<Args>...` can produce both source-owned `Args` and
  substituted `Args...`, `int`, or `float` rows at the same source location.
- Set the boolean at the binding construction site, where the code decides
  between explicit source text, normalized semantic text, substituted concrete
  text, and pack aggregate text.
- Tie-break duplicate alias rows by this producer-owned boolean. Template-body
  visibility should use the same boolean.

Implementation note: a trial that inferred the decision in the formatter from
`SourceTemplateIdOccurrence.arguments` and a single per-binding source spelling
was not sufficient. Substituted rows can carry source-looking occurrences, and
pack aliases need per-rendered-binding fragment ownership. The renderer should
not try to rediscover that mapping.

### 4. Exact Use And Declaration Anchors

Formatter behavior:

- `normalize_event_locations_and_decls(...)` searches source text to repair
  class template-id locations, variable declaration anchors, operator token
  columns, constructor/direct-init columns, candidate drop locations, and
  plausible function-call rows.

Semantic replacement:

- Every visible row should carry an exact `spelling_anchor` when it represents a
  source occurrence.
- Class and alias rows should use `SourceTemplateIdOccurrence.name_anchor` and
  typed argument occurrences instead of rediscovering template-ids.
- Function call rows should carry the actual call token anchor:
  operator token for overloaded operators, constructor type/name token for
  constructor calls, and callee identifier for ordinary calls.
- Variable rows should carry a selected declaration anchor from declaration
  parsing, not from a prior-line text search.
- Candidate drops should carry exact declaration-name anchors when the drop is
  recorded.
- Rows that cannot prove a public source occurrence should be marked with the
  visibility/context enum above instead of being filtered by plausibility
  source scans.

### 5. Nested Template-ID Source Rows

Formatter behavior:

- `synthesize_nested_template_source_events(...)` reparses source text at event
  and declaration locations, then clones class-use rows for nested template IDs.

Semantic replacement:

- Treat nested template IDs as normal source-use rows emitted by semantic
  analysis.
- Use existing `TemplateIdSyntax` / source-token-index paths such as
  `emit_nested_class_use_source_events_from_syntaxes(...)` and
  `emit_nested_class_use_source_events_from_location(...)`.
- Any remaining renderer synthesis user indicates a missing semantic emission
  call site, not a formatter responsibility.

### 6. Inline Namespace Presentation

Formatter behavior:

- `normalize_event_names(...)` scans source lines for `inline namespace` names
  and strips those segments from rendered entity text.

Semantic replacement:

- Entity printers should produce public witness names through a single
  inline-namespace-aware presentation helper.
- That helper should use `Scope::inline_namespace` and
  `semantic_lookup::inline_namespace_collapsed_scope_name(...)`, not source
  text.
- Apply the same presentation to selected entities, expanded aliases, binding
  arguments, and dropped candidates before the rows reach the formatter.

## Implementation Plan

1. Add typed binding parameter shape and delete function-pointer declaration
   parsing.
2. Remove source-recovery passes that are already redundant under the strict
   suite, one small group at a time.
3. Add source visibility/context to source-use decisions and replace
   header/body/member/local-alias suppression with producer classification.
4. Add source-spelled binding text for alias rows and replace alias tie-break
   source-line matching.
5. Thread exact call/declaration anchors through function, constructor,
   operator, variable, and drop producers; then delete location repair scans.
6. Move inline namespace collapse into semantic name presentation and delete
   source-line inline namespace discovery.

Each implementation stage should rebuild `dev/cppgm++`, run the strict witness
suite, and commit only when green.

## Validation Command

```sh
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```
