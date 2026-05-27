# Text Reparse Closure Plan

## Goal

Remove semantic and template-layer string reparsing as an internal transport
mechanism.

The target state is not "fewer fallback parses". The target state is:

- parse source text at the parser boundary
- carry names, template-ids, arguments, expressions, type-ids, source anchors,
  and source uses as structured state after that boundary
- serialize only for diagnostics, stable output, debug traces, ABI/linkage
  names, or explicitly allowed external text APIs
- fail audits when new semantic code introduces stringify/reparse transport

This plan superseded the older broad cleanup direction in
`template-reparse-elimination-plan.md` for the closure work. Both documents are
now archived under `docs/implemented/`; the active follow-on cleanup lane is
semantic fallback removal.

## Current State

Raw scan of `dev/src` before this plan:

| Pattern | Count | Notes |
| --- | ---: | --- |
| `parse_template_id_string(` | 69 | Mix of real parser helpers, cached compatibility parses, and internal name/template-id transport. |
| `parse_qualified_name_string(` | 91 | Largest remaining name-structure hole; many sites parse `node.value`, binding names, or scope-qualified strings. |
| `parse_expression_fragment(` | 23 | Includes true fragment parser entrypoints plus non-type template argument/default-argument reparsing. |
| `parse_type_fragment(` | 9 | Mostly type-id/default/template-parameter reconstruction seams. |
| `parse_translation_unit_fragment(` | 6 | Parser entrypoints plus parameter-list/template-declaration reconstruction. |
| `rebuild_node_text` / `fully_spaced_node_text` | 80 | Includes diagnostics and output, but also structured AST loss before reparsing. |
| source-line recovery helpers | 29 | Source witness/source-use rows recovered by scanning original lines. |

Those counts are intentionally crude. The next implementation slice adds a
ratchet so counts cannot increase while each category is classified and removed.

## Integration Rebaseline - 2026-04-24

The integration worktree is materially ahead of the accidental `main` text
reparse work. It already has:

- a broad `scripts/audit_text_reparse.py` ratchet over name, template-id,
  fragment, AST-text, and source-line recovery seams
- `template_api::TemplateServices` between template matching and the monolithic
  semantic context
- structured out-of-class member names and source template names in several
  high-value paths

Current audit output in this worktree:

| Category | Count | Baseline | Status |
| --- | ---: | ---: | --- |
| `qualified_name_string_parse` | 0 | 0 | closed |
| `template_id_string_parse` | 0 | 0 | closed |
| `expression_fragment_parse` | 0 | 0 | closed |
| `type_fragment_parse` | 0 | 0 | closed |
| `semantic_type_text_bridge` | 0 | 0 | closed |
| `translation_unit_fragment_parse` | 0 | 0 | closed |
| `ast_text_rebuild` | 0 | 0 | closed |
| `source_line_recovery` | 0 | 0 | closed |

Latest landed slice in this worktree:

- parser-backed qualified names now stay structured on:
  `id_expression`, `class_specifier` / `class_forward_declaration`, declarator
  `identifier` nodes, member-expression identifiers, `using`/namespace-alias
  `target` nodes, and qualified special-member declaration/definition nodes
- semantic consumers that previously reparsed those AST spellings now read the
  stored `QualifiedName` directly in `semantic_expression.cpp`,
  `typesemantic.cpp`, and the out-of-class collection paths in
  `callsemantic.cpp`
- qualified-use source-location collection now also consumes stored AST name
  syntax instead of reparsing `node.value`
- `nsinit_semantic.cpp` now consumes structured namespace-alias / using-target
  names directly from the AST instead of reparsing the `target` spelling
- `nsinit_semantic.cpp` declarator validation and declared-name extraction now
  read structured identifier syntax instead of reparsing the declarator text
- `semantic_overload.cpp` now uses structured callee syntax to detect
  qualified lookup and suppress ADL, instead of reparsing the callee spelling
- `semantic_lookup.cpp` now carries structured declarator names through
  qualified variable parse-scope resolution instead of reparsing the
  declarator spelling to recover the owner scope
- `semantic_declaration.cpp` now keeps `using` targets structured, rewriting
  bound components inside `QualifiedName` instead of reparsing rewritten
  target text
- `semantic_class_model.cpp` now resolves inherited-constructor targets from
  stored target syntax instead of reparsing the rewritten target spelling
- `callsemantic.cpp` now carries declarator identifier syntax directly through
  the out-of-class function/static-member paths instead of reparsing the
  qualified member string
- `semantic_class_model.cpp` friend-function registration now takes declarator
  `QualifiedName` syntax directly from the parsed identifier subtree instead of
  reparsing the friend declarator spelling to recover qualification
- template argument resolution now consumes structured `type_id` syntax where
  available, qualified call/member lookup uses structured template-id anchors,
  and specialization matching avoids fragment reparsing in another group of
  semantic-only type paths
- `CallSemNode` now carries structured qualified-name syntax for emitted
  function/variable nodes, and `nsdecl_semantic.cpp` consumes that directly
  instead of reparsing emitted names to recover namespace qualification
- `nsinit_semantic.cpp` now consumes that same structured call-semantic name
  syntax when deciding whether a record name is already qualified, instead of
  reparsing the emitted entity name
- `callsemantic.cpp` explicit-instantiation fallback now strips the trailing
  template-id from declarator identifier syntax directly instead of reparsing
  the stripped template name text
- `semantic_utils.cpp` now owns the shared trailing-template-argument stripper,
  and both template-argument anchor helpers now derive the anchor identifier
  from string structure directly instead of reparsing the argument as a
  qualified name
- `callsemantic.cpp` now derives template lookup fragments and the
  fast-template-argument gate from top-level string structure directly instead
  of reparsing the lookup text as a qualified name
- qualified-function lookup now crosses the semantic boundary as
  `QualifiedName` instead of text, so `using`-target resolution and explicit
  function-template instantiation no longer reparse the qualified lookup name
- qualified value callers that already hold AST name syntax now route directly
  to `lookup_qualified_value_binding(...)`, leaving the generic `lookup_value`
  path unqualified-only instead of reparsing qualified spellings on demand
- out-of-class owner-class resolution now splits qualified owner text by
  top-level `::` components instead of reparsing the owner as a qualified name
- the remaining legacy out-of-class member text wrapper now builds
  `QualifiedName` by top-level component splitting, so structured binding paths
  no longer route through the qualified-name parser when they hit that fallback
- declared-class scope resolution and non-template type lookup now split
  qualified text by top-level components in their legacy compatibility paths,
  instead of invoking the full qualified-name parser to recover structure
- function-template call candidate collection now threads parsed
  `QualifiedName` callee syntax into qualified template lookup, avoiding a
  reparse of the call-site template name
- AST-backed template-argument leaf value/function lookups now use stored
  `QualifiedName` syntax for qualified ids, allowing the unused qualified
  function text-lookup overload to be deleted
- variable-template lookup now has a structured `QualifiedName` path, and
  using-declaration, id-expression, and constant-value template-id callers use
  it directly instead of stringifying parsed template names for lookup
- using-declaration class-template and alias-template lookup now also consume
  the rewritten `QualifiedName` target directly instead of routing through
  target text
- namespace-alias definitions, using-directives, and using-declaration
  namespace bindings now consume stored `QualifiedName` target syntax when
  available instead of reparsing target text
- the older `typesemantic.cpp` analyzer now uses the same structured namespace
  targets directly and no longer carries a local string-parsing namespace
  lookup wrapper
- member-expression and member-call target resolution now requires the stored
  `QualifiedName` member syntax, so qualified member lookup no longer reparses
  member spellings
- the semantic namespace lookup API no longer exposes a string overload;
  namespace alias/directive consumers and template qualifier-prefix resolution
  now pass structured `QualifiedName` prefixes throughout
- namespace-scope declaration target resolution now consumes the declarator
  identifier's stored `QualifiedName`, removing the string overload for
  namespace entity target resolution
- ADL suppression for explicit template-id calls now reuses the already parsed
  direct callee template-id state instead of reparsing the callee spelling
- class-template, alias-template, and variable-template string lookup overloads
  are now unqualified-only; qualified template-template argument text is split
  at the local compatibility boundary and routed through the structured
  `QualifiedName` overload instead of hiding a parser fallback in every lookup
  wrapper
- function-template string collection is also unqualified-only; target-typed
  function-id resolution and explicit function-template instantiation /
  specialization paths now route existing `QualifiedName` syntax into the
  structured collection path when qualification is possible
- legacy type, function, and constant-value compatibility lookups now use the
  local top-level qualified-name splitter instead of invoking the full
  qualified-name parser to recover scope components from strings
- the top-level qualified-name splitter is now shared in `semantic_utils`, and
  the remaining semantic/template compatibility lookups in `typesemantic.cpp`,
  `template_argument_semantics.cpp`, and `template_resolution.cpp` use it
  instead of invoking the parser bridge
- symbol-linkage mangling now builds `QualifiedName` values from top-level
  string components without calling the parser bridge; conversion operators
  use the already supplied display name so `operator N::T` is not split as an
  owner-qualified function name
- the now-unused analyzer qualified-name parse cache, metric, and wrapper were
  deleted, leaving only the parser bridge declaration/definition
- special-member template-name normalization and template witness anchor
  selection now strip trailing top-level template arguments directly instead
  of parsing template-ids just to recover the head name
- special-member class-name matching and owner-class witness anchoring now
  strip trailing top-level template arguments directly instead of parsing
  template-ids just to recover the template head
- template-argument source anchors and template-member special-member detection
  now recover only the template head by top-level stripping, avoiding parser
  fallback when the argument list is not consumed
- out-of-class owner-template bookkeeping now uses a local unqualified
  template-head check for owner lookup, instead of parsing template-ids when
  argument text is ignored
- id-expression parsing now preserves final-component `TemplateIdSyntax`
  directly on AST nodes, and declval, variable-template id-expression lookup,
  invalid nondependent-id checks, and direct explicit template calls consume
  that structured syntax instead of reparsing `node.value`
- direct `declval` call handling, dependent member-template keyword checks, and
  function-template call candidate collection now also consume AST
  `TemplateIdSyntax`, removing the remaining id-expression call-site parser
  fallbacks in those paths
- explicit function template-id expressions and calls now route stored
  `TemplateIdSyntax` into a structured function-template lookup hook; generic
  function lookup is no longer responsible for reparsing ordinary name text as
  a possible template-id
- friend function-template registration now receives declarator
  `TemplateIdSyntax` directly when matching existing function templates, instead
  of reparsing the rendered friend function name
- id-expression parsing now also preserves qualifier-component
  `TemplateIdSyntax`, and `is_same<T, U>::value` handling consumes that
  structured qualifier syntax instead of reparsing the qualifier text
- constexpr external-value lookup now reads id-expression `TemplateIdSyntax`
  for source anchors and source argument spellings instead of reparsing the
  evaluator's id-expression text
- class specifier / forward declaration nodes now preserve final-component
  `TemplateIdSyntax`, including explicit empty `<>`, and explicit class
  instantiation plus class specialization collection consume that structured
  template-id directly
- member-expression identifiers now preserve `TemplateIdSyntax`, structured
  names strip the `template` disambiguator for lookup, and dependent
  member-template call lookup passes the structured head/arguments directly
- declarator identifiers now preserve `TemplateIdSyntax`; explicit function
  instantiation plus variable/function template specialization collection
  consume that structured declarator syntax instead of reparsing names
- trailing-return AST nodes now preserve `TemplateIdSyntax` for simple
  template-id returns; deduction-guide collection consumes that structured
  syntax instead of reparsing the return type spelling
- base-clause AST nodes now preserve `TemplateIdSyntax`, and base-class
  source-use collection consumes that structured syntax instead of reparsing
  base-name text
- template-argument witness anchor selection now strips top-level template
  arguments directly when it only needs the anchor identifier, instead of
  reparsing argument text as a template-id
- direct type-argument lookup gating now validates the stripped template-id
  head instead of reparsing the whole candidate just to classify its shape
- fast existing class-template instantiation gating now uses the same stripped
  template-id-head validation instead of reparsing candidate argument text
- `type_name` AST nodes now preserve `TemplateIdSyntax`; exact type lookup
  anchors and declaration-specifier class-use source bindings consume that
  structured syntax instead of reparsing type spellings
- declaration-specifier type-name nodes now preserve structured
  `TemplateIdSyntax`, exact type lookup anchors select source argument
  spellings by matching the source template head, and source-owned class-use
  rows are suppressed while the source argument list still names dependent
  local/template bindings
- the unused qualified-name parser bridge was deleted after all consumers moved
  to structured names
- these landed slices lowered `qualified_name_string_parse` from `75` to `0`
- parser-collected `TemplateIdSyntax` now preserves nested template-id
  arguments, and exact local class-use source rows consume that structured
  nested argument syntax instead of reparsing lookup text
- class and variable partial-specialization records now retain normalized
  `TemplateArgumentSyntax` beside their argument text, and the partial matcher
  consumes nested source `TemplateIdSyntax` when deducing from template-id
  pattern arguments
- actual partial-specialization template-id deduction now requires structured
  class-template instantiation metadata from the actual type instead of
  reparsing its display spelling as a fallback
- alias-template pattern expansion now consumes the already-decomposed
  `QualifiedName` and argument spellings from the caller instead of reparsing
  the same template-id text inside the expansion helper
- synthetic `declval` call nodes now carry explicit `declval` callee metadata,
  so nothrow analysis no longer reparses the callee text to recognize
  `declval<T>()`
- source-use binding for resolved class-template owner qualifiers now uses the
  owner's instantiation argument metadata directly instead of reparsing the
  qualifier spelling to recover explicit source arguments
- constant-value lookup for variable-template-ids now uses parser-carried
  `TemplateIdSyntax` from id-expression / consteval hook call sites, so the
  generic constant lookup no longer reparses arbitrary names as template-ids
- constant-value lookup for template-id-qualified member constants now uses
  parser-carried qualifier `TemplateIdSyntax`; the builtin
  `is_same<T, U>::value` path no longer reparses qualifier text as a
  template-id inside generic constant lookup
- qualified class-use source binding for resolved owner classes now trusts the
  owner `ClassInfo`'s `source_template` and `instantiation_arguments` instead
  of reparsing the owner display name to recover missing arguments
- function-template parameter mention checks now inspect structured
  class-template instantiation arguments from `ClassInfo` / template metadata
  instead of reparsing rendered named-type template-ids
- unresolved qualified class-use source binding no longer reparses a single
  qualifier string to synthesize a class-template source-use decision; source
  decisions now require a resolved owner binding with structured template
  identity
- type-lookup request construction now recognizes unqualified template-id
  shape by top-level template-head structure when it only needs to preserve the
  name, instead of invoking the template-id parser to decompose unused
  arguments
- constant-value lookup no longer performs a second alias-template-id parse
  after the ordinary type lookup for a qualifier fails; alias template
  resolution remains owned by type lookup
- out-of-class owner-class resolution now relies on `lookup_type_impl` and the
  shared top-level qualified component splitter, removing the duplicate
  unqualified owner template-id parser fallback
- nested owner component resolution also relies on `lookup_type_impl` for
  template-id components, removing the second manual component parser fallback
- qualified value lookup now decides whether a placeholder-bearing class
  qualifier is concrete from the resolved class's source-template identity and
  structured instantiation arguments instead of reparsing the qualifier text
- direct qualifier type lookup now strips only the template head needed for
  direct-scope ownership and routes the full specialization text through the
  existing type lookup path, instead of reparsing arguments locally
- `decltype` pack-expression rewriting no longer has its own duplicate
  template-id parser fallback; template-argument pack rewriting flows through
  the shared rewrite machinery
- function-template deduction alias-skip checks now recover only the
  template-id head by top-level structure when the argument list is unused,
  instead of reparsing full template-ids in the skip path
- dependent named-type resolution no longer has a duplicate unary
  transform-alias template-id parse before the shared type-lookup path
- default-argument placeholder canonicalization now uses a narrow top-level
  unqualified template-id splitter for the compatibility text it already owns,
  instead of invoking the full template-id parser when it only needs to
  canonicalize the head spelling
- the bound type-name / pack-argument text rewrite compatibility helpers now
  share a top-level template-id splitter instead of invoking the full
  template-id parser while recursively rewriting already-owned argument text
- template type lookup now uses the same top-level template-id splitter before
  trying local unary-transform, alias-template, and class-template resolution,
  instead of invoking the full parser on lookup text it already normalized
- monolithic semantic type lookup uses that same splitter for normalized
  template-id text, including final template-id components after templated
  qualifiers, instead of routing through the scope-aware template-id parser
- template-instantiation decomposition now splits final-component template-id
  text with the shared top-level splitter in both semantic-context and
  `TemplateServices` paths, instead of reparsing rendered type text
- class-use source-decision recovery now uses the shared top-level template-id
  splitter for the already-owned source-use spelling instead of invoking the
  scope-aware template-id parser in that compatibility path
- partial-specialization template-id pattern decomposition now uses the shared
  top-level splitter for complete template-id spellings, and the unused
  template-id parser bridge/cache/helper APIs have been deleted
- explicit `noexcept(expr)` comparison/evaluation now consumes the parser-held
  qualifier expression child instead of reparsing the cached expression text
- dynamic array `new[]` size calculation now uses the parsed array-bound
  expression from the `new_type_id` AST instead of reparsing `Type::bound_text`
- `alignas(...)` operands now stay attached to the AST as parsed type-id or
  expression nodes, and class-layout alignment evaluation consumes those nodes
  directly instead of reparsing the saved operand spelling
- `decltype(...)` / GNU `typeof(...)` specifier nodes now carry their parsed
  operand into declaration/type analysis, so the semantic decltype path no
  longer reparses the operand text to recover an expression AST
- the unused variable-template partial-argument text validator was deleted,
  removing a dead non-type argument expression reparse path
- these landed slices lowered `template_id_string_parse` from `86` to `0`
- these landed slices lowered `expression_fragment_parse` from `23` to `15`
- the pack-count/index cleanup lowered `expression_fragment_parse` from `15`
  to `12` by replacing `__integer_pack` / `__type_pack_element` count reparses
  with literal-or-bound-constant evaluation
- structured expression pack substitution lowered `expression_fragment_parse`
  from `12` to `10` by cloning pack-expanded expression ASTs with bound
  `TypePtr` and value-pack nodes instead of formatting and reparsing each
  element; fold-expression operand expansion now uses the same structured path
- structured parameter-clause initializer recovery lowered
  `expression_fragment_parse` from `10` to `8` by converting ambiguous
  parameter-declaration ASTs such as `b`, `b * c`, `b & c`, `b && c`,
  `b[c]`, and `b(c)` directly into expression ASTs for namespace and local
  direct-initializer recovery instead of rebuilding child text and reparsing it
- structured template-argument type-id transport lowered
  `expression_fragment_parse` from `8` to `7` by preserving parsed
  template-argument type/expression nodes and parsing partial-specialization
  `decltype(...)` type patterns from their stored `type_id` AST
- symbol-linkage mangling now carries owner class-template arguments through
  function symbol options, emits dependent defaulted NTTP types such as
  `enable_if_t<...>` structurally enough to match Clang's Itanium spelling,
  and treats expression pack expansions and template-parameter substitution
  slots with the same granularity Clang uses
- structured parameter-clause pack expansion lowered
  `translation_unit_fragment_parse` from `6` to `4` and `ast_text_rebuild`
  from `73` to `69` by cloning function-parameter ASTs, substituting bound
  type-pack elements into the structured nodes, and parsing the resulting
  parameter-clause AST directly instead of rebuilding parameter text and
  reparsing a synthetic function declaration
- structured type-id consumption for constexpr hooks, trailing returns, and
  dependent alias-template targets lowered `expression_fragment_parse` from
  `7` to `6` and `ast_text_rebuild` from `69` to `67`; the remaining removed
  expression site was dead code after the legacy text-evaluation guard
- a context-free Itanium mangling type IR now handles builtin/cv/pointer/
  reference/function type spelling without adding more text-specific mangling
  cases while preserving the existing substitution table behavior
- legacy non-type template-argument text evaluation no longer reparses
  expressions; builtin type traits are handled explicitly, structured
  type-trait ASTs expand direct bound type packs, and the audit ratchet lowers
  `expression_fragment_parse` from `6` to `5`
- the now-unused semantic expression-fragment parser, semantic-context API,
  cache, and metrics plumbing were deleted, closing
  `expression_fragment_parse` at `0` and lowering
  `translation_unit_fragment_parse` from `4` to `3`
- type fragment parsing now consumes the tokenized type-id directly through the
  template-argument fragment parser instead of synthesizing a translation unit
  with an alias declaration, closing `translation_unit_fragment_parse` at `0`
- the unused semantic-context type-fragment parse API, fragment cache, memory
  census entry, and metric counter were deleted; parsed type text remains
  cached at the higher-level type result boundary, lowering
  `type_fragment_parse` from `9` to `6`
- Analyzer `parse_type_text_scoped` now keeps its parsed-type result cache but
  delegates uncached parsing to `template_decl_ast::parse_type_text_scoped`,
  removing the duplicate direct fragment parse path and lowering
  `type_fragment_parse` from `6` to `4`
- the template-argument `try_parse_type_text_via_ast_hooks` fallback was
  removed; callers now route unresolved text fallback through the shared
  `template_decl_ast` type parser, lowering `type_fragment_parse` from `4` to
  `3`
- source-location recovery for member-call witness anchoring now reuses a
  token-sequence helper shared with decltype source anchoring, deleting the
  overload-side same-line source scan fallback and lowering
  `source_line_recovery` from `26` to `23`
- overload-side checks that a source location points at a specific identifier
  now use exact token/source-location data instead of reopening the source
  file, lowering `source_line_recovery` from `23` to `22`
- overload call-fragment use-location refinement now derives the callee anchor
  identifier from the call AST and searches parser token locations on the
  use-site line, deleting the overload-side source-line cache and AST text
  fallback; this lowers `source_line_recovery` from `22` to `20` and
  `ast_text_rebuild` from `67` to `66`
- callsemantic source-use and witness identifier refinements now search the
  parser token/source-location stream, including same-line and template-open
  checks, instead of rescanning original source lines; this lowers
  `source_line_recovery` from `20` to `10`
- callsemantic location validation, class-template argument source anchors,
  and decltype operand locations now use token/source-location facts or the
  carried operand AST instead of reopening source lines; this lowers
  `source_line_recovery` from `10` to `5`
- nested template-id source-use recovery now walks token spans and reconstructs
  token text/location instead of scanning original source lines, closing
  `source_line_recovery` at `0`
- class alias-declaration handling now consumes the existing `type_id` AST
  directly and no longer rebuilds/reparses alias target text as a fallback,
  lowering `ast_text_rebuild` from `66` to `58`
- value-binding dependency checks now inspect initializer AST nodes directly
  instead of rebuilding initializer text to test for template placeholders,
  lowering `ast_text_rebuild` from `58` to `56`
- builtin type-trait call arguments now consume `type_id` and identifier AST
  nodes before considering legacy text parsing, deleting the rebuilt-argument
  fallback and lowering `ast_text_rebuild` from `56` to `55`
- non-type template argument dependency checks now use syntax-backed expression
  ASTs directly instead of rebuilding the whole expression text first; bare
  identifier leaves are ignored as standalone dependency evidence so resolved
  type-id subtrees such as pack-expanded `first<U...>` do not regress to text
  classification, lowering `ast_text_rebuild` from `55` to `54`
- conversion-operator result types and dynamic exception-spec type lists now
  carry hidden parser metadata with their original `type_id` ASTs; semantic
  consumers read that structure directly instead of reparsing the operator
  suffix or `throw(...)` type-list text. This removes semantic bridge call sites
  but does not change the central `type_fragment_parse` audit count yet

Correction for the active integration branch:

- the symbol-linkage statement above is only partially true; PA22/473 exposed
  that the mangler still falls back to parsing text for dependent NTTP
  parameter types and dependent expression template arguments
- the next work slice must complete structured mangling before any further
  general text-reparse cleanup, because adding more text cases to
  `symbol_linkage.cpp` would hide the same boundary hole this plan is meant to
  close

Current strict-test baseline in this worktree:

- root `make test-strict` now runs all entries in `STRICT_PAS` and reports the
  full failing `pa` set before exiting nonzero
- use this as the regression floor while closing reparses

| PA | Baseline | Notes |
| --- | --- | --- |
| `pa18` | witness compare `27` failures (`87` compared, `3` skipped) | improved after structured decltype/default-argument source-use location fixes |
| `pa19` | witness compare `32` failures (`80` compared, `3` skipped) | clean HEAD remeasurement showed the prior `31` floor was stale; witness compare runs to completion |
| `pa21` | witness compare `13` failures (`96` compared, `2` skipped) | current full strict floor after semantic reparse closures |
| `pa22` | witness compare `126` failures (`239` compared, `11` skipped) | current full strict floor after semantic reparse closures |

Strict harness note:
root `test-strict` now exports `KEEP_GOING=1` like `test-report`, so each strict `pa` reports the full failing set instead of stopping at the first internal failure. Strict local witness generation also uses the shared batched test machinery (`STRICT_SUBTEST_JOBS`, defaulting to `DEFAULT_BUILD_JOBS`) and compares witness output through the shared Perl comparison harness instead of the old standalone Python per-test runner.

Latest boundary validation:

- after the template-head and owner-template parser-fallback removals,
  `make test-strict-nobuild STRICT_PAS='pa18 pa21 pa22'` matched the recorded
  witness baselines exactly: `pa18` `25/87/3`, `pa21` `10/96/2`, and `pa22`
  `137/239/11`
- after restoring structured metadata on synthetic functional-cast callees and
  overloaded-operator callees, the same limited boundary strict command again
  matched the recorded witness baselines exactly: `pa18` `25/87/3`, `pa21`
  `10/96/2`, and `pa22` `137/239/11`
- after the deduction-guide, base-clause, and template-anchor cleanup batch,
  the same limited boundary strict command again matched the then-recorded
  witness baselines exactly: `pa18` `25/87/3`, `pa21` `10/96/2`, and `pa22`
  `137/239/11`
- after the source-template anchor correction for structured type-name syntax,
  `make test-strict-nobuild STRICT_PAS='pa18 pa21 pa22'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` produced `pa18` `25/87/3`,
  `pa21` `10/96/2`, and `pa22` `136/239/11`; the PA22 count improved by one
  because `pa22/tests/general/311-reference-member-class-template-visible.t`
  now passes
- before continuing after the suspected qualifier-template metadata regression,
  clean pre-session `abab1cb3ce80` was rebuilt and checked with the same
  limited strict command for `pa18` and `pa22`; it produced `pa18` `28/87/3`
  and `pa22` `142/239/11`, so those are the current regression floors for this
  branch/environment alongside `pa21` `10/96/2`
- after the duplicate decltype-pack, alias-skip, and unary-transform parser
  removals lowered `template_id_string_parse` to `19`, the same limited strict
  command produced `pa18` `28/87/3`, `pa21` `10/96/2`, and `pa22`
  `142/239/11`
- after the default-argument, text-rewrite, normalized lookup, and monolithic
  type-lookup splitter removals lowered `template_id_string_parse` to `14`,
  the same limited strict command produced `pa18` `28/87/3`, `pa21`
  `10/96/2`, and `pa22` `142/239/11`
- after the class-use split and partial-specialization parser bridge deletion
  closed `template_id_string_parse` at `0`, the same limited strict command
  produced `pa18` `28/87/3`, `pa21` `10/96/2`, and `pa22` `142/239/11`
- after structured decltype/default-argument source-use location fixes, the
  same limited strict command produced `pa18` `27/87/3`, `pa21` `10/96/2`,
  and `pa22` `134/239/11`; these are the current local regression floors
- after the pack-count/index cleanup lowered `expression_fragment_parse` from
  `15` to `12`, `make test-strict-nobuild STRICT_PAS='pa19 pa22'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` produced `pa19` `32/80/3`
  and `pa22` `134/239/11`; a clean HEAD remeasurement of `pa19` also produced
  `32/80/3`, so this slice introduced no strict regression
- after converting strict witness generation to the shared batched harness,
  `make test-strict-nobuild STRICT_SUBTEST_JOBS=2
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the recorded floors:
  `pa18` `27/87/3`, `pa19` `32/80/3`, `pa21` `10/96/2`, and `pa22`
  `134/239/11`
- after raising the strict subtest default from `2` to the host build-job count,
  PA19 combined generation improved from `23.55s` at `2` jobs to `7.32s` at
  `12` jobs on this host, and `make test-strict-nobuild STRICT_PAS='pa19'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the recorded
  `pa19` floor `32/80/3`
- after structured expression pack substitution lowered
  `expression_fragment_parse` from `12` to `10`, `make test-strict-nobuild
  STRICT_PAS='pa18 pa19 pa22' CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the recorded floors:
  `pa18` `27/87/3`, `pa19` `32/80/3`, and `pa22` `134/239/11`
- after structured parameter-clause initializer recovery lowered
  `expression_fragment_parse` from `10` to `8`, the same limited strict
  command matched the recorded floors: `pa18` `27/87/3`, `pa19` `32/80/3`,
  and `pa22` `134/239/11`
- after structured template-argument type-id transport lowered
  `expression_fragment_parse` from `8` to `7`, the same limited strict command
  matched the recorded floors: `pa18` `27/87/3`, `pa19` `32/80/3`, and
  `pa22` `134/239/11`; before the next reparse slice, investigate the
  `pa22/tests/general/474-defaulted-decltype-empty-pack-instantiation.t` LowIR
  mismatch reported inside the existing `pa22` failure floor
- after fixing dependent defaulted NTTP / pack metadata mangling, targeted
  PA22 LowIR checks passed for `361`, `366`, `368`, `384`, `388`, `394`,
  `416`, `434`, `435`, `474`, `478`, `479`, and `spec/038`; direct
  `clang++`/`llvm-nm` comparison matched exactly for the previously divergent
  `366`, `368`, `384`, `416`, `479`, and `spec/038` object names, and
  `make test-strict-nobuild STRICT_PAS='pa22'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the recorded PA22
  floor `134/239/11`
- after structured parameter-clause pack expansion, the limited strict command
  `make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched a clean parent
  remeasurement at `d6dd7b25`: `pa18` `26/87/3`, `pa19` `33/80/3`,
  `pa21` `12/96/2`, and `pa22` `129/239/11`
- after deleting the synthetic translation-unit wrapper from type-fragment
  parsing and closing `translation_unit_fragment_parse` at `0`,
  `make test-strict-nobuild CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the same four-suite
  floor: `pa18` `26/87/3`, `pa19` `33/80/3`, `pa21` `12/96/2`, and `pa22`
  `129/239/11`
- after deleting the unused type-fragment cache/API and centralizing Analyzer
  type-text fallback through `template_decl_ast`, the same full
  `make test-strict-nobuild CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` boundary matched the same
  four-suite floor while `type_fragment_parse` lowered to `4`
- after removing the template-argument AST-hook type-text parser fallback,
  the same full strict boundary again matched the four-suite floor while
  `type_fragment_parse` lowered to `3`
- after replacing callsemantic identifier source-line scans with token/source
  location searches, `make test-strict-nobuild
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` again matched the current
  four-suite floor: `pa18` `27/87/3`, `pa19` `33/80/3`, `pa21` `12/96/2`,
  and `pa22` `129/239/11`
- after removing the remaining callsemantic validation, class-template
  argument, and decltype operand source-line scans, the same full strict
  boundary again matched the current four-suite floor: `pa18` `27/87/3`,
  `pa19` `33/80/3`, `pa21` `12/96/2`, and `pa22` `129/239/11`
- after removing class alias-declaration text rebuild/reparse fallbacks,
  focused PA22 alias tests `362`, `377`, `378`, and `442` passed, and
  `make test-strict-nobuild STRICT_PAS='pa22'
  CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the current PA22
  floor `129/239/11`
- after switching value-binding dependency checks to AST inspection, focused
  PA22 dependency tests `250`, `303`, `309`, and `350` passed, and the same
  PA22 strict no-build boundary matched floor `129/239/11`
- after removing the builtin type-trait rebuilt-argument fallback, focused
  PA22 trait tests `287`, `388`, `414`, and `spec/022` passed, and the same
  PA22 strict no-build boundary matched floor `129/239/11`
- after routing `type_trait_expression` leaves through their structured
  `type_id` children instead of rebuilding the whole expression text, the full
  strict no-build boundary matched the current four-suite floor: `pa18`
  `27/87/3`, `pa19` `33/80/3`, `pa21` `12/96/2`, and `pa22` `129/239/11`;
  the AST path uses the template-aware type-id parser so alias-template source
  use metadata remains unchanged
- after adding structured decltype/call handling for the remaining PA22
  reducers and deleting the unused type-fragment bridge, `type_fragment_parse`
  and `semantic_type_text_bridge` are both closed at `0`; the full strict
  no-build boundary had no exit-code or LowIR failures and reported the current
  witness-only floor: `pa18` `27/87/3`, `pa19` `32/80/3`, `pa21` `13/96/2`,
  and `pa22` `126/239/11`
- after removing diagnostic-only AST text rebuilds, routing builtin
  type-trait handling through structured call ASTs, and deriving trailing
  default type arguments from their `type_id` ASTs instead of rebuilt text,
  `ast_text_rebuild` lowered from `54` to `32`; focused PA21/PA22 LowIR checks
  passed and the full strict no-build boundary remained witness-only at
  `pa18` `27/87/3`, `pa19` `32/80/3`, `pa21` `13/96/2`, and `pa22`
  `126/239/11`
- after replacing the remaining semantic/mangling callers with structured
  AST-leaf handling and deleting the now-unused AST-text formatter API, all
  tracked text-reparse categories are at `0`; focused PA21/PA22 default,
  alias, and mangling LowIR checks passed, the PA22/474 witness canary matched,
  and the full strict no-build boundary remained witness-only at `pa18`
  `27/87/3`, `pa19` `32/80/3`, `pa21` `13/96/2`, and `pa22` `126/239/11`

Validation strategy for this plan:

This cleanup should not run full strict validation for every small commit.
Validation is tiered so reparse debt can be reduced iteratively without making
each local edit pay the full witness-suite cost.

### Commit-Level Validation

Run this for each small commit or local fix before committing:

```sh
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
python3 scripts/audit_text_reparse.py
git diff --check
```

Also run the narrowest reproducer set for the seam being edited. Prefer direct
owner tests over whole assignment suites:

```sh
make -C paNN check TEST=tests/.../case.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
```

Commit-level validation is sufficient when a change:

- removes or reroutes one local reparse site
- has focused tests covering the affected lookup, template-id, type, or
  witness behavior
- does not intentionally change strict witness baselines
- keeps `python3 scripts/audit_text_reparse.py` at or below the committed
  baseline

When a tracked audit count drops, ratchet
`docs/text-reparse-audit-baseline.json` in the same commit and update this
plan's current count table.

Focused witness tests may still report known baseline mismatches. In that case,
do not rebuild clean `HEAD` for every small slice. Cache clean-`HEAD`
`.my.witness` actuals once under:

```sh
/private/tmp/cppgm-witness-baselines/<head-sha>/<focused-set>/
```

Then compare dirty `.my.witness` outputs against that cache. Regenerate only
missing or stale cached cases, or refresh the cache after committing a validated
slice. This answers whether the local edit changed behavior without paying a
full clean rebuild for each known-failing focused run.

Do **not** run root `make test-strict`, full `make test-report`, or all affected
strict suites for every commit. Those are boundary validations.

### Local Boundary Validation

Run a limited strict subset only at a meaningful local boundary, not after every
small commit. Local boundary triggers are:

- finishing a coherent mini-batch in one seam family, for example all lookup
  wrapper reparses or all local namespace-target reparses
- changing behavior that could affect witness ownership or source-use event
  ordering
- before leaving an in-progress reparse category unfinished for a different
  category
- before committing a larger multi-file semantic refactor if focused tests do
  not sufficiently cover the affected owner set

Use the smallest `STRICT_PAS` set that owns the changed behavior:

```sh
make test-strict-nobuild \
  STRICT_PAS='pa18 pa21 pa22' \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Compare only the affected suites against the recorded baseline table above.
For example, a lookup/template cleanup touching `pa18`, `pa21`, and `pa22`
should remain at:

- `pa18`: `27` witness failures, `87` compared, `3` skipped
- `pa21`: `10` witness failures, `96` compared, `2` skipped
- `pa22`: `134` witness failures, `239` compared, `11` skipped

If a local boundary strict run is interrupted or intentionally skipped because
the current change is small and focused tests are sufficient, record that in
the handoff/final note rather than silently implying broad validation ran.

### Milestone Validation

Run the full root strict suite only at milestone boundaries:

```sh
make test-strict \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Milestone triggers are:

- closing a full audit category or a major section of it
- clearing a strict blocker
- intentionally updating the stored strict witness baseline
- before switching focus away from text-reparse cleanup for a larger unrelated
  task
- before preparing the branch for integration or review

Full root strict is expected to report the known baseline failures until those
witness gaps are fixed. Treat increases, new crashes, or changed compared /
skipped counts as regressions.

### Process Rules

- Prefer focused tests and the audit ratchet for ordinary commits.
- Prefer local strict subsets only after a mini-batch or higher-risk semantic
  change.
- Prefer full strict only at milestone boundaries.
- Stop stale background validation jobs when the user redirects work; do not
  let an interrupted strict run keep consuming the worktree.

Carry-over analysis from the accidental `main` branch:

- `Add text reparse audit ratchet` is superseded by the integration audit.
- `Track template instantiation placeholder state` does not apply directly:
  integration routes many of these decisions through `TemplateServices` and
  template metadata. Reintroduce that idea only after defining the state on the
  integration-facing model/API.
- The partial-specialization matching fixes still map to live integration
  holes: duplicate pattern parsing, reparsing `actual.text` after a structured
  `TA_TYPE` argument is already available, and reparsing pattern arguments
  after structured template-id decomposition has failed to resolve them.
- A structured-only probe at `template_specialization.cpp`'s
  `deduce_from_named_template_id_text` confirmed one missing carrier:
  original `TemplateArgumentSyntax` is not enough for canonicalized or
  alias-expanded pattern types. The full template-id parser fallback has since
  been removed, but canonicalized / alias-expanded pattern types still cross
  through type text plus the shared top-level splitter. Closing that remaining
  semantic gap belongs to the `TemplatePattern` / `StructuredTypeExpr` slices,
  not the now-closed `template_id_string_parse` category.
- The transformed partial-specialization cache is still applicable, but it must
  be ported carefully around the integration branch's current local
  `callsemantic.cpp` changes.

The next implementation order for this worktree is therefore:

1. Port narrow partial-specialization fixes that do not touch the dirty
   `callsemantic.cpp` region.
2. Run the focused hosted-template regressions plus the broad audit after each
   slice.
3. Ratchet the broad audit baseline only when a tracked category count drops.
4. Defer placeholder-state propagation until the state is represented through
   `TemplateServices` rather than by duplicating the older `main` shape.

Additional bridge rules carried forward from the older broad plan:

1. Parsed template-id decomposition stays authoritative until integration-side
   instantiation metadata is fully bound.
   - if both parsed template-id syntax and template/class instantiation state
     are available, do not replace the parsed decomposition with a half-bound
     semantic record just because it exists
   - this directly protects the `pa34/720` class of regressions where omitted
     defaults can still be completed structurally from the parsed template-id

2. Temporary deferred defaults must carry the caller-bound dependent form.
   - if text must survive temporarily at a boundary, it must be the already
     substituted dependent spelling produced once at the deferral point
   - do not preserve the original unbound default text and expect later
     deduction or matching passes to repair it

3. Do not eagerly instantiate dependent class-template-ids just to recover
   structure.
   - dependent type/template forms must remain dependent until a real semantic
     boundary requires instantiation
   - if a caller needs template-head or argument structure, add that structure
     to the carried state rather than forcing early instantiation

## Allowed Text Processing

Text parsing is allowed only in these categories:

1. Source ingestion
   - normal preprocessing/tokenization/parsing of original source files
   - explicit test harnesses that parse a source fragment as the unit under
     test

2. Public or compatibility fragment APIs
   - `semantic_fragment_parser::*` entrypoints
   - temporary compatibility shims while they are directly listed in this plan
   - these must be marked as compatibility, not used as ordinary semantic
     transport

3. Formatting-only uses
   - diagnostics
   - witness output rendering after structured facts are already selected
   - debug traces
   - stable assignment output formats

4. Identity serialization without reparse
   - symbol linkage names
   - structural keys serialized from structured data
   - cache keys that are never reparsed as semantic input

Every other semantic stringify/reparse site is debt.

## Remaining Debt Categories

### 1. Parsed Names Stored As Strings

Problem:

- AST and semantic code frequently stores a qualified name or template-id only
  as `CppAstNode::value`, `FunctionBinding::qualified_name`, `ClassInfo` text,
  or a scope-qualified string.
- Later code reparses that string with `parse_qualified_name_string(...)` or
  `parse_template_id_string(...)`.

Replacement:

- add `NameSyntax` / `TemplateIdSyntax` structures carrying:
  - qualifier components
  - unqualified identifier
  - template-head marker
  - structured template arguments
  - spelling/source span
  - declaring/lookup scope when needed
- attach them at parse time to id-expressions, type-names, declarator names,
  qualified member names, destructor/conversion names, and template-id nodes
- store structured names on semantic entities that currently retain only text

Completion rule:

- semantic code may read text from a name node for formatting only
- semantic lookup, matching, and specialization selection must consume
  structured name syntax or structured entity references

### 2. Template Arguments Cross Internal APIs As `vector<string>`

Problem:

- Several template APIs still return or consume argument spellings rather than
  typed argument objects.
- The receiver reparses the strings to decide whether an argument is a type,
  non-type expression, template name, pack, dependent form, or placeholder.

Replacement:

- introduce a single `TemplateArgumentSyntax` / `StructuredTemplateArg` shape:
  - `Type` with structured type-id AST and resolved `TypePtr` when available
  - `NonTypeExpression` with expression AST and optional evaluated value
  - `TemplateName` with structured name syntax/entity reference
  - `PackExpansion` with structured pattern and expansion source span
  - `Dependent` with explicit dependency reason
- use this object in parser collection, explicit template arguments, default
  template arguments, specialization patterns, function-template deduction, and
  instantiated argument lists

Completion rule:

- no internal API whose purpose is semantic template work may expose
  `vector<string>` argument lists
- textual argument lists are allowed only at source parsing and rendering

### 3. Pack Expansion Rebuilds Declarations Or Arguments To Text

Problem:

- Parameter-pack expansion and default-argument expansion often rebuild a
  declaration/argument with `rebuild_node_text(...)`, substitute text, then
  parse the result.

Replacement:

- represent pack expansion as structured nodes over:
  - parameter declaration syntax
  - type-id syntax
  - expression AST
  - template argument syntax
- expansion produces cloned structured nodes with bound pack elements, not
  synthetic source text

Completion rule:

- pack expansion helpers may clone AST/semantic nodes, but may not create text
  and parse it back into nodes

### 4. Non-Type Template Arguments And Defaults Reparse Expressions

Problem:

- Non-type defaults, array extents, traits, and selected rewritten expressions
  still pass through `parse_expression_fragment(...)`.

Replacement:

- keep the original expression AST on template parameters, defaults, explicit
  non-type arguments, and delayed substitutions
- add a structured substitution/evaluation wrapper for the small cases that
  currently rewrite text before consteval
- store evaluated constant values beside the expression when available

Completion rule:

- consteval may evaluate a carried expression AST under a scope/binding set
- it may not require the caller to format and reparse the expression first

### 5. Type Model Loses Template-Id And Dependent Owner Structure

Problem:

- Helpers such as template matching, alias canonicalization, and dependent
  lookup still recover template heads and arguments from type text.
- Some type nodes preserve `source_template` and instantiation arguments, but
  other dependent/member/alias forms degrade to strings.

Replacement:

- extend type/dependent-type representation with:
  - template head identity
  - structured argument vector
  - dependent owner chain
  - alias application identity
  - partial-order placeholder marker
  - source pattern when source spelling matters
- derive display strings and structural keys from this model

Completion rule:

- template matching must prefer structured type/template-id decomposition
- text parsing of type strings is allowed only for explicitly classified legacy
  ingress until the corresponding state is added

### 6. Entity Names Are Reparsed From Display Strings

Problem:

- Class/function/value bindings store qualified display names that are later
  split or reparsed for semantic decisions.

Replacement:

- store a structured `EntityName`:
  - namespace/class owner chain
  - unqualified name
  - template head/argument metadata when the entity is a specialization
  - declaration and definition anchors
- derive `qualified_name` text from `EntityName` for output only

Completion rule:

- no lookup or source-use producer may call a string parser on an entity's
  formatted qualified name

### 7. Source Witness Rows Recover Occurrences By Scanning Source Lines

Problem:

- Several witness/source-use rows are still discovered by reading original
  source lines and searching for identifiers/template-ids.

Replacement:

- finish the gated `SemanticSourceUse` table described in
  `template-boundary-hole-removal-plan.md`
- record one structured row per visible source occurrence during semantic
  analysis
- store exact spelling anchors and selected entity anchors at the point where
  semantic code still has the syntax node

Completion rule:

- renderer must not scan source text to invent missing witness rows
- if a row is missing, the semantic producer must emit a structured source-use
  row

### 8. Template Declaration And Parameter Reconstruction Reparses Fragments

Problem:

- Template parameter lists, default arguments, explicit specializations, and
  function template signatures still rebuild declaration fragments and parse
  them again.

Replacement:

- store structured template declaration syntax:
  - parameter list AST
  - per-parameter kind and name
  - type/non-type/template-template details
  - default argument syntax
  - pack marker
- function/class/alias template records should retain this object directly

Completion rule:

- template declaration registration may normalize syntax structurally
- it may not rebuild a declaration fragment and parse it to recover parameter
  metadata

## Missing Structured State

| Missing State | Owner | Replaces |
| --- | --- | --- |
| `NameSyntax` | AST name nodes and semantic entities | `parse_qualified_name_string(node.value)` and entity-name reparsing. |
| `TemplateHeadRef` | template-id syntax, type decomposition, matching | head recovery from reparsed type/template-id strings. |
| `TemplateIdSyntax` | AST template-id nodes and template entity references | `parse_template_id_string(...)` on `node.value` and qualified-name components. |
| `TemplateArgumentSyntax` | parser, template declarations, instantiations, deduction | `vector<string>` template arguments. |
| `TemplatePattern` | partial-specialization and alias pattern registration | reparsing pattern text/arguments on each match attempt. |
| `StructuredTypeExpr` | type/dependent-type layer | type text reparsing for alias/member/dependent template-id matching. |
| `StructuredNonTypeExpr` | template parameter/default/non-type argument layer | expression fragment reparsing for defaults and delayed substitutions. |
| `TemplateDeclarationSyntax` | template declaration records | translation-unit/type fragment reparsing of rebuilt parameter declarations. |
| `EntityName` | class/function/value/template binding records | reparsing display-qualified names. |
| `AbiManglePattern` | function/class template declarations and `FunctionSymbolOptions` | Itanium ABI mangling from reparsed dependent type/expression text. |
| instantiation bridge flags on integration metadata | `TemplateServices`, class/template instantiation records | text-prefix placeholder/dependency guards and eager-instantiation probes. |
| `SemanticSourceUse` | translation-unit semantic state, gated by consumer | source-line scanning and renderer recovery. |

## Implementation Order

### Blocking Slice. Structured ABI Mangling

This slice blocks the rest of the plan until it is complete. The immediate
failure source is symbol mangling for templated functions and special members,
especially dependent defaulted non-type template parameters such as
`typename enable_if<...>::type = 0`.

Do not add new dependency-expression or type parsers in `symbol_linkage.cpp` to
make individual spellings pass. Any tactical text-parser experiments used for
debugging must be removed before committing.

IR implementation plan:

1. Add a small Itanium mangling IR that mirrors ABI grammar nodes instead of
   rendered C++ fragments. The first committed surface is type IR plus a typed
   substitution-key wrapper; later slices should extend it with name,
   template-argument, and dependent-expression nodes rather than adding more
   `symbol_linkage.cpp` string splitters.
2. Route context-free semantic types through the IR first. This is intentionally
   limited to builtin, cv, pointer/reference, and function-type structure whose
   leaves are already semantic and do not require reparsing. It exercises the IR
   emitter and substitution callback path without changing dependent/template
   ABI spellings.
3. Lift named/template-id mangling next. `TemplateIdSyntax` and semantic
   template declarations should produce IR name/template-argument nodes with
   explicit substitution identities, replacing `TemplateComponent` and
   `parse_template_component(...)` call sites.
4. Lift dependent expression and NTTP mangling after names are structured.
   Expression nodes should carry dependence classification and emit ABI
   expression productions directly; text operator splitting remains a bug signal
   once an AST-backed producer exists.
5. Only after those carriers are in place should the audit baseline be ratcheted
   down. A missing structured node at a migrated boundary should fail locally
   rather than falling back to reparsing the same rendered text.

Required structured state:

- `TemplateParameterInfo` must carry a structured non-type parameter type
  pattern, including the decl-specifier/type-id syntax and any declarator or
  abstract-declarator syntax that changes the parameter type
- `TemplateArgument` must carry structured syntax for type, value, and
  template-template arguments where the caller has parser-owned syntax
- `FunctionTemplateDecl` must retain structured function parameter pattern
  syntax beside `type_pattern` and `params_pattern`
- `FunctionSymbolOptions` must pass those structured pattern objects to the
  mangler, including owner class-template parameters/arguments and function
  template parameters/arguments
- dependent expressions used in ABI-relevant positions must be represented as
  AST-backed expression nodes, not as formatted text

Required mangler APIs:

- `mangle_template_argument_syntax(...)`
- `mangle_template_id_syntax(...)`
- `mangle_type_pattern_syntax(...)`
- `mangle_dependent_expression_ast(...)`
- `mangle_non_type_template_parameter_pattern(...)`

Completion rules:

- dependent NTTP mangling must consume the structured parameter pattern and the
  structured argument value/default; it may not call
  `try_mangle_type_text_impl(...)` on `non_type_decl_specifier_text`
- dependent expression mangling must consume expression AST nodes for
  operators, `sizeof...`, unary `!`, member `::value`, and template-id
  operands; it may not split operator text
- template-id mangling must consume `TemplateIdSyntax` / structured
  instantiation metadata for pack grouping and substitution slots; it may not
  parse rendered template-id strings
- after the structured path is expected to exist, missing syntax is a producer
  bug and should fail a targeted check instead of silently falling back to text

Initial failing/verification set:

- `pa22/tests/general/415-local-alias-explicit-template-pack-decltype.t`
- `pa22/tests/general/473-partial-specialization-enable-if-constructor-selection.t`
- recheck the remaining known PA22 LowIR/object-name mismatches after those
  two pass: `364`, `438`, `441`, `444`, `451`, and `472`

Current status in this branch:

- structured ABI mangling is now required for dependent result types,
  parameter declarations, dependent NTTPs, expression template arguments, and
  `decltype(...)` result expressions; the guarded legacy text path remains a
  failure signal for missing structured state
- the focused PA22 verification set above passes with clang-compatible object
  names as of this slice
- the next step is to run the boundary PA22 validation, fix any remaining
  exit-status/object-name failures, then resume the ordinary text-reparse audit
  slices below
- the structured dependent-declarator slice now handles array bounds, function
  parameter declarators, dependent result template heads, and alias-expansion
  root substitution slots structurally; after updating refs whose current
  object names were confirmed against Clang, PA22 text comparison has 11
  remaining Clang-divergent LowIR/object-name mismatches:
  `281`, `289`, `293`, `294`, `407`, `409`, `469`, `spec/021`, `spec/037`,
  `spec/039`, and `spec/117`
- qualifier template-ids inside dependent nested types are now qualified from
  lookup scope before mangling, including inline namespace owners; after
  updating the Clang-confirmed refs for `293`, `294`, `469`, `spec/037`,
  `spec/039`, and `spec/117`, the remaining PA22 text-compare object-name
  mismatches are `281`, `289`, `407`, `409`, and `spec/021`
- dependent member and qualified-type mangling now preserve alias spelling,
  resolve non-dependent constant member values structurally, and keep
  substitution slots aligned with Clang for the focused `281`, `289`, `407`,
  `415`, and `473` cases
- qualified-type template-name substitution is now suppressed only while
  mangling nested qualifiers inside dependent member-expression template
  arguments. Ordinary function return/parameter types keep the Clang-compatible
  qualifier template-name slot. The focused `225`, `432`, `441`, `453`, `478`,
  `spec/118`, and `spec/119` checks pass, and PA22 keep-going text comparison
  is clean.
- boundary strict validation after the contextual qualifier fix reports the
  PA22 witness floor as `128` failures, `239` compared, and `11` skipped, with
  no PA22 LowIR text-compare mismatches remaining
- explicit template-argument resolution now accepts optional
  `TemplateArgumentSyntax` and evaluates aligned explicit non-type template
  arguments from the stored expression AST. The audit count is intentionally
  unchanged because the legacy text evaluator remains for callers without
  structured syntax and for arguments whose text changed during pack expansion.
- non-type template argument expression evaluation now has a shared structured
  API, and partial-specialization matching uses stored argument expression ASTs
  when comparing concrete value arguments.
- partial-specialization matching no longer falls back to reparsing non-type
  argument text. Actual value arguments must come from stored
  `TemplateArgument` values, and pattern values must come from stored
  `TemplateArgumentSyntax::expression`; normalized default non-type arguments
  now preserve that expression syntax.
- boundary validation after the explicit non-type syntax pass-through reports
  the same PA22 witness floor, `128` failures, `239` compared, and `11`
  skipped; PA22 exit-status mismatches remain `0`, PA22 keep-going text compare
  is clean, and `python3 scripts/audit_text_reparse.py --list-sites` remains
  at `expression_fragment_parse` `7/7`.

Targeted validation for each small structured-mangling commit:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make -C pa22 check TEST=tests/general/281-detected-or-dependent-type-argument-default-enable-if.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/289-dependent-detected-or-missing-member-enable-if-call.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/407-qualified-member-alias-sfinae.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/363-constructor-template-class-alias-enable-if-default.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/364-constructor-template-const-ref-enable-if-conversion.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/415-local-alias-explicit-template-pack-decltype.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
make -C pa22 check TEST=tests/general/473-partial-specialization-enable-if-constructor-selection.t CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
git diff --check
```

Boundary validation before leaving this slice:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
KEEP_GOING=1 make -C pa22 test CXX=../dev/cppgm++ CPPGM_SKIP_DEV_REBUILD=1
(cd pa22 && KEEP_GOING=1 scripts/compare_results.pl ref my tests)
make test-strict-nobuild STRICT_PAS='pa22' CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
python3 scripts/audit_text_reparse.py
git diff --check
```

Latest source-line recovery closure validation:

- `make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++` passed
- `python3 scripts/audit_text_reparse.py` passed with
  `source_line_recovery` at `0/0`
- `git diff --check` passed
- `make test-strict-nobuild CXX=/usr/local/opt/llvm/bin/clang++
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++` matched the expected strict
  floors: PA18 `27/87/3`, PA19 `33/80/3`, PA21 `12/96/2`, PA22 `129/239/11`

For each changed object name, compare against Clang with
`/usr/local/opt/llvm/bin/clang++ -std=c++11 -x c++ -c` and `nm` /
`llvm-nm` before updating `.ref` files. Reference updates are allowed only
after generated names match Clang or the difference is documented as an
intentional ABI-surface limitation.

Exit-code sequencing:

- after structured mangling and LowIR/object-name mismatches are fixed, collect
  any remaining PA22 exit-status failures
- fix exit-code failures before returning to ordinary text-reparse removal
- only resume Slice 3+ cleanup below after the structured mangling slice and
  exit-status pass are committed

### Slice 0. Audit Ratchet

Add an audit script and baseline for text reparsing patterns. The script should:

- count known reparse categories
- distinguish allowed parser-boundary helpers from ordinary semantic sites as
  the allowlist becomes precise
- fail when a category count grows above the committed baseline
- support lowering the baseline after each removal slice

Tests:

```bash
python3 scripts/audit_text_reparse.py
```

Commit after the script and baseline pass.

### Slice 1. Structured Name Syntax

Add `NameSyntax` and `TemplateIdSyntax`, then attach them to AST name nodes that
already contain parsed name structure.

Initial targets:

- id-expression nodes used by `semantic_expression.cpp`
- type-name nodes used by `typesemantic.cpp`
- declarator/function nodes used by `callsemantic.cpp`
- qualified member names used by class/function binding registration

Delete local reparses only where the structured name is present and required.
Do not retain "try structured, then reparse the same string" fallbacks at the
same call site. If a syntax node is missing structured name state, fix the
producer.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-pa7 CXX=/usr/local/opt/llvm/bin/clang++
make test-pa8 CXX=/usr/local/opt/llvm/bin/clang++
```

### Slice 2. Structured Template Argument Transport

Replace internal `vector<string>` argument surfaces with
`TemplateArgumentSyntax`.

Initial targets:

- explicit template-id parsing results
- class-template instantiation arguments
- function-template explicit arguments
- partial-specialization pattern arguments
- alias-template arguments
- exact local class-use witness argument recovery, now backed by nested
  source `TemplateArgumentSyntax` rather than a lookup-text reparse

Delete reparses that only determine argument kind or pack-ness.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
cd pa21 && python3 ../scripts/compare_template_witness_text.py --app ../dev/cppgm++ tests
```

Commit when the existing known witness baseline does not regress.

### Slice 3. Structured Non-Type And Default Arguments

Carry expression AST/default argument syntax from declaration registration to
instantiation/evaluation.

Initial targets:

- complete the explicit-template-argument syntax pass-through by removing the
  legacy `evaluate_non_type_argument_text(...)` fallback where callers are
  guaranteed to have `TemplateArgumentSyntax`
- non-type template parameter defaults
- default function/template arguments
- array extent/count expressions
- trait argument expressions that currently parse a rebuilt expression

Delete expression fragment reparses that receive text produced by the compiler
itself.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-pa21 CXX=/usr/local/opt/llvm/bin/clang++
make test-pa34 CXX=/usr/local/opt/llvm/bin/clang++
```

### Slice 4. Structured Type Template-Id Model

Extend dependent/member/alias type representation so template matching and
alias canonicalization can decompose types without reparsing type strings.

Initial targets:

- `template_specialization.cpp`
- `template_resolution.cpp`
- `template_argument_semantics.cpp`
- `callsemantic.cpp` type-argument helpers

Delete type-fragment reparses used for deduction/canonicalization.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-pa34 CXX=/usr/local/opt/llvm/bin/clang++
```

### Slice 5. Structured Entity Names

Add `EntityName` to binding records and derive formatted qualified names from
it.

Initial targets:

- `ClassInfo`
- `FunctionBinding`
- `ValueBinding`
- alias/template declaration records

Delete reparses of formatted entity names.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-pa7 CXX=/usr/local/opt/llvm/bin/clang++
make test-pa8 CXX=/usr/local/opt/llvm/bin/clang++
make test-pa9 CXX=/usr/local/opt/llvm/bin/clang++
```

### Slice 6. Source-Use Table Completion

Finish `SemanticSourceUse` producers for rows still recovered from source text.

Initial targets:

- qualified class/alias/type uses
- out-of-class member-definition qualifiers
- nested template-id source uses
- function-call selected declaration rows

Delete source-line scanning helpers after the corresponding producers exist.

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
cd pa21 && python3 ../scripts/compare_template_witness_text.py --app ../dev/cppgm++ tests
```

### Slice 7. Template Declaration Syntax

Retain structured template declaration syntax and delete fragment reparses of
rebuilt parameter/declaration text.

Initial targets:

- `template_decl_ast.cpp`
- template parameter default registration
- function template signature capture
- explicit specialization registration

Tests:

```bash
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-pa21 CXX=/usr/local/opt/llvm/bin/clang++
make test-pa34 CXX=/usr/local/opt/llvm/bin/clang++
```

### Slice 8. Remove Compatibility Parsers From Semantic Hot Paths

After the structured carriers are in place:

- remove compatibility wrappers from semantic lookup/matching paths
- keep fragment parsers only at allowed boundaries
- update the audit baseline to zero for closed categories
- convert the audit from a ratchet to a denylist for forbidden categories

Tests:

```bash
make CXX=/usr/local/opt/llvm/bin/clang++
python3 scripts/audit_text_reparse.py --strict
```

## Commit Cadence

Each slice should commit separately.

Minimum gate before each commit:

```bash
git diff --check
python3 scripts/audit_text_reparse.py
```

Behavior-changing slices must also run the focused assignment or witness tests
listed above.

When a slice removes reparse sites, lower the audit baseline in the same commit
so the removed debt cannot come back.

## Definition Of Done

The closure work is complete when:

- semantic/template code no longer uses text parsing as internal data transport
- every remaining text parse is categorized under the allowed list
- the audit runs in strict mode and passes
- source witness rows are backed by structured source-use rows rather than
  renderer source scanning
- template arguments, names, type template-ids, non-type expressions, entity
  names, and template declarations all have structured carriers
- no call site has a same-site fallback from structured state back to reparsing
  the same compiler-produced text
