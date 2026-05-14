# Witness Formatter Semantic Output Plan

## Goal

Make the witness output formatter a formatter again: it should format typed
semantic source-use rows, not reopen source files and reconstruct or suppress
facts by parsing source text.

This is a maintainability cleanup. The expected witness output should remain
unchanged for the strict `pa18 pa19 pa21 pa22` suite.

## Current Shape

The renderer still contains substantial text parsing and rewriting:

- `read_source_lines(...)` feeds several source-line recovery passes.
- `normalize_event_locations_and_decls(...)` scans source lines to repair class,
  variable, function, operator, constructor, and drop locations.
- `synthesize_nested_template_source_events(...)` reparses template-id text and
  clones class-use events for nested template IDs.
- `drop_class_template_local_alias_source_events(...)`,
  `drop_template_header_pattern_events(...)`, and
  `drop_member_alias_parameter_events(...)` suppress events by matching source
  lines and template headers.
- `prefer_source_spelled_alias_events(...)` chooses alias-use rows by checking
  whether rendered binding text appears on the source line.
- `canonicalize_function_pointer_binding_args(...)` reparses declaration text
  to decide whether to add `&` to a binding argument.
- `normalize_event_names(...)` reads the source file to discover inline
  namespace names and rewrite event text.

Some string normalization should stay in the formatter for now: spacing,
stable entity spelling, visible-output dedupe keys, and final block formatting
are legitimately output concerns. The problem is source-file reparsing that
infers semantic facts after emission.

## Audit Result

After rebuilding the compiler for each attempted removal, the strict suite no
longer needs these source-line recovery behaviors:

- local class-template alias suppression;
- nested template-id source-event synthesis;
- member-alias parameter suppression;
- source parsing for function-pointer non-type template arguments.

The function-pointer case now has semantic data: template bindings carry a
`function_pointer_parameter` flag derived from the template parameter value
type, so the formatter no longer reparses the declaration text to decide
whether to render `&name`.

Several larger passes are still semantically meaningful today and cannot be
deleted without producer-side work:

- `normalize_event_locations_and_decls(...)` still repairs use anchors,
  operator token locations, constructor call locations, declaration anchors,
  and some duplicate function/class source rows.
- `drop_template_header_pattern_events(...)` still hides pattern/header
  artifacts that should instead be classified as non-public source uses by the
  producer.
- `prefer_source_spelled_alias_events(...)` still breaks ties between alias rows
  whose semantic arguments differ from source-spelled explicit arguments.
- `normalize_event_names(...)` still needs inline-namespace metadata that is
  currently discovered from source text.
- `normalize_source_defined_template_calls(...)` still suppresses template-body
  events that should be marked by semantic origin/ownership.

Every accepted slice must pass this command after rebuilding:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

The strategy is therefore incremental: delete only formatter parsing whose
semantic replacement exists, then add semantic fields or emission
classification before removing the remaining passes.

## Target Rule

The source witness renderer should:

- consume `SemanticSourceUseTable` rows directly;
- trust `spelling_anchor`, `selected_decl_anchor`, `template_id_occurrence`,
  bindings, specialization bindings, and candidate/drop locations from semantic
  emission;
- normalize text only for presentation consistency;
- dedupe by semantic/rendered identity where needed;
- avoid reopening source files to discover events, repair locations, or decide
  whether a row should exist.

The renderer should not:

- synthesize new class/alias/variable/function source facts from source text;
- parse source lines to decide whether an event is a template header artifact;
- parse source lines to prefer one alias-use row over another;
- scan source lines for operator tokens or constructor spellings when semantic
  use anchors are already precise enough.

## Implementation Steps

1. Remove redundant recovery calls from `collect_rendered_source_events`.

   Delete these calls first, keeping strict output unchanged:

   - `drop_class_template_local_alias_source_events`
   - `synthesize_nested_template_source_events`
   - `drop_member_alias_parameter_events`

2. Replace function-pointer declaration parsing with typed binding data.

   Add a binding flag for non-type template parameters whose value type is a
   pointer to function, set it when bindings are built, and have the formatter
   use that flag to render `&name`.

3. Delete dead source-line parser helpers.

   Once the calls are gone, remove the helper clusters that only served those
   passes:

   - nested template-id source scanners;
   - local class-template alias source scanners;
   - member-alias parameter source scanners;
   - function-pointer declaration scanners;

4. Keep typed occurrence handling.

   Keep renderer logic that consumes already-typed
   `SourceTemplateIdOccurrence` data, such as alias source-spelled binding
   application and same-line template-id checks. That logic does not reopen
   source files.

5. Leave broader string canonicalization for a later slice.

   `normalize_binding_arg_for_event`, `normalize_entity_name_for_event`, and
   visible-output dedupe still do string work. They should eventually move
   toward shared witness text utilities or semantic spelling fields, but they
   are not the source-file reparse problem being removed in this stage.

6. Validate and commit.

   After pruning the renderer, run the requested strict command. Commit only
   after the suite passes.

## Future Semantic Data If Needed

If future tests still need any deleted recovery behavior, restore the behavior
by threading semantic data, not by adding a new source-line parser:

- exact operator/call token anchors should be recorded in
  `FunctionCallSourceDecision`;
- nested template IDs should be emitted by semantic analysis as source-use rows
  with `SourceUseOwnership::NestedDerived` or `SourceOwned`;
- local alias/template-header suppression should be represented as an emission
  origin or producer-side noncanonical drop;
- function-pointer binding spelling should be captured as a typed binding
  property, not inferred from declaration text.
