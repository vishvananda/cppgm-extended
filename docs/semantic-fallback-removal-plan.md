# Semantic Fallback Removal Plan

## Goal

Remove semantic control flow that tries one expensive interpretation, catches
failure, and then falls back to another interpretation.

The target state is not "no probes". Cheap probes are fine when the probe is
the intended classifier. The target state is:

- choose the semantic strategy from explicit syntax, type, binding, and
  initialization facts before doing expensive analysis
- treat same-site "try candidate A, catch failure, try candidate B" as a bug
  unless the fallback is documented as a cheap classifier
- make remaining fallback transitions hard-failable so tests identify missing
  semantic data instead of silently hiding it
- keep SFINAE and overload candidate rejection separate from fallback cleanup;
  those are language rules, not fallback debt

## Why This Follows Text-Reparse Closure

Text reparsing is now closed by audit. The next source of semantic opacity is
fallback-driven control flow.

These fallbacks are harmful for the same reasons reparsing was harmful:

- correctness depends on exception text or partial failure instead of on the
  actual semantic owner
- expensive candidate analysis may run before a cheap structural discriminator
  was checked
- a failing path can mutate state, instantiate templates, collect witness facts,
  or request output before the fallback path wins
- tests pass while the real missing semantic fact remains unmodeled

## Definitions

`Fallback debt` means a path that:

- attempts one semantic strategy that can do nontrivial lookup, conversion,
  instantiation, or expression analysis
- treats failure as a normal branch
- then invokes a different semantic strategy for the same source construct

`Allowed probe` means a path that:

- is cheap and side-effect-free
- only classifies the source form or an already-available semantic fact
- does not rely on exception text
- does not instantiate templates, request output, or perform full overload
  resolution before deciding to try another strategy

## Hard-Fail Behavior

Classified fallback transitions throw immediately with a stable
diagnostic that includes:

- fallback category
- source location when available
- construct spelling when available
- first failed strategy
- proposed replacement fact

This was introduced as a reducer tool and is now the normal compiler behavior:
run the strict suites, record which tests fail, and replace fallbacks with
explicit semantic data until the category disappears.

## Inventory

### 1. Operator Overload To Builtin Operator

Current sites:

- `semantic_expression.cpp` unary overloaded-operator analysis falls back to
  builtin handling when `analyze_call_expression(...)` throws "unknown" or "no
  viable" operator diagnostics.
- `semantic_expression.cpp` binary overloaded-operator analysis does the same
  before builtin binary lowering.
- `semantic_expression.cpp` compound-assignment analysis repeats the same
  pattern.

Why this is debt:

- the path pays overload-call analysis before deciding that builtin operator
  semantics own the expression
- the fallback predicate depends on diagnostic text from the failed call
- the failed overload path can do template work before builtin lowering wins

Desired replacement:

- compute an `OperatorResolutionPlan` from operand classes/enums, discovered
  operator function/template sets, and builtin operator availability
- skip overload-call analysis entirely when there are no viable overload
  candidates to try
- when both user and builtin candidates are possible, represent builtin
  candidates in the same resolution phase rather than using exception fallback

Initial hard-fail category:

```text
operator-overload-to-builtin
```

Current status:

- The operator-to-builtin fallback no longer parses diagnostic strings to
  decide whether overload lookup failed. Ordinary call analysis now reports
  typed `NoViableOverloadError` and `UnknownFunctionError` soft failures, and
  operator fallback sites catch those types directly. Builtin dispatch is still
  a fallback path and remains instrumented.
- Operator candidate sets are prefiltered by callable arity before overload
  analysis. This handles cases such as unary `&x` seeing an unrelated binary
  `operator&` declaration: builtin address-of now wins because no correctly
  shaped overload candidate exists, not because overload resolution failed.

### 2. Constructor Selection To Aggregate Or Expr Construction

Current sites:

- `semantic_lifetime.cpp` tries direct constructor selection and catches
  `NoViableConstructorError` to retry aggregate construction.
- `semantic_lifetime.cpp` retries expression-based aggregate construction after
  direct constructor selection fails.
- `callsemantic.cpp` target-aware construction catches
  `NoViableConstructorError` and then retries with expression-analyzed source
  arguments.

Why this is debt:

- aggregate/direct/list/copy construction category is already knowable from the
  target class, source syntax, and initialization form
- failure can happen after overload work and argument analysis
- fallback can hide missing initialization-kind data in constructor requests

Desired replacement:

- pass an explicit `ConstructionDispatchPlan` through target-aware analysis and
  constructor action preparation
- choose aggregate construction before overload selection when aggregate rules
  own the form
- choose expression-source constructor selection from request shape instead of
  after a failed node-source selection

Initial hard-fail category:

```text
constructor-selection-fallback
```

Current status:

- Constructor action aggregate fallback is removed for the known aggregate
  cases. Direct-braced aggregate construction is classified before constructor
  overload selection, and non-braced single braced aggregate arguments take the
  same aggregate path before overload selection.
- The stale expression-source retry after direct constructor selection was
  removed from constructor-action lowering and target-aware construction. Node
  source constructor selection now either succeeds or reports the original
  failure instead of retrying with generic expression arguments.

### 3. Target-Aware Expression To Generic Expression

Current site:

- `semantic_expression.cpp::analyze_expression_for_target(...)` tries
  target-aware analysis, then reference-binding source analysis, then generic
  expression analysis, then conversion.

Why this is partly allowed:

- some target-aware probes are cheap syntax classifiers, such as braced-init
  shape or lambda-to-closure target checks
- reference binding has distinct source-expression requirements that can be
  checked before generic analysis

Why this still needs cleanup:

- the caller receives no reason for why target-aware analysis declined
- generic analysis can accidentally recover when the target-aware path was
  missing required state

Desired replacement:

- return a structured `TargetAwareDeclineReason`
- hard-fail only the decline reasons that indicate missing semantic data
- keep cheap syntax-not-applicable declines as allowed probes

Initial hard-fail category:

```text
target-aware-to-generic-expression
```

Current status:

- A strict-only classifier now hard-fails when target-aware analysis declines
  syntax it should own: braced initialization of initializer-list, array, or
  class targets, and type-style construction of class targets.
- Fundamental functional casts such as `int(...)` and `char(...)` were
  classified as allowed cheap declines. Generic expression analysis owns those
  directly, so treating them as target-aware fallback debt produced false
  positives.
- `CPPGM_STRICT_SEMANTIC_FALLBACKS=target-aware-to-generic-expression` over
  `pa18 pa19 pa21 pa22` reaches the existing witness-only floor.

### 4. Template Type Resolution Strategy Cascades

Current sites:

- `template_argument_semantics.cpp::parse_type_id_node_for_templates(...)`
  tries structured qualified lookup, semantic-only text resolution, then broader
  lookup-text resolution.
- `template_resolution.cpp` canonicalizes alias text in a preferred scope, then
  retries in the deduction scope.
- `template_resolution.cpp` actual argument type resolution tries lookup in one
  scope, then another, then semantic-only resolution in both.
- `template_specialization.cpp` partial-specialization matching tries syntax
  type parsing, then semantic-only text resolution.

Why this is mixed:

- scope preference fallback can be a cheap lookup ordering optimization
- same-text semantic resolution fallback is debt when the caller already has AST
  syntax, template argument syntax, or a known source template

Desired replacement:

- use a `TypeResolutionRequest` with explicit source syntax, preferred scope,
  deduction scope, allowed lookup families, and dependency policy
- report `unavailable structured fact` rather than silently widening lookup
- leave explicit multi-scope lookup as an allowed probe only when both scopes
  are named in the request and lookup is side-effect-free

Initial hard-fail category:

```text
template-type-resolution-fallback
```

### 5. Alias Parse To Dependent Alias Placeholder

Current site:

- `semantic_class_model.cpp` used to parse class alias-declaration type-ids
  and catch `TemplateSubstitutionFailure` to synthesize a dependent alias
  placeholder when dependency predicates suggested placeholders or dependent
  bindings were present.

Why this is debt:

- the code already computes a dependency predicate before parsing
- parsing first can force substitution on a form that should have been carried
  as dependent structured state
- text predicates are standing in for dependency facts that should live on the
  alias target syntax/type-id

Desired replacement:

- compute alias target dependency from structured type-id syntax and bound
  template state
- when dependent, create the deferred alias placeholder directly without first
  attempting substitution
- parse only when the dependency classifier says the alias target is concrete

Initial hard-fail category:

```text
alias-parse-to-dependent-placeholder
```

Current status:

- Class alias declarations now use a shared dependency classifier before
  parsing. Explicitly dependent aliases create the dependent placeholder
  directly; concrete-looking aliases parse normally, and substitution failure
  is no longer used as the placeholder branch.

### 6. Friend Function Template Deduction To Instantiation Arguments

Current site:

- `semantic_class_model.cpp` used to try function-template deduction for
  adjusted friend declarations and fall back to class instantiation arguments
  or member-scope named-type bindings when deduction failed.

Why this is debt:

- the fallback is not a cheap alternative candidate; it recovers semantic data
  that should already be attached to the friend declaration or enclosing class
  instantiation
- deduction failure is used as a control signal for "use existing class
  arguments"

Desired replacement:

- carry the enclosing template argument vector on the friend declaration path
- decide whether deduction is required before invoking deduction
- only run deduction when the friend declaration has independent parameters to
  solve

Initial hard-fail category:

```text
friend-deduction-to-enclosing-arguments
```

Current status:

- Empty-template-id friend references now classify the enclosing-context path
  before deduction. If the friend function type names the current class and the
  enclosing class context fully binds the referenced function-template
  parameters, those arguments are used directly. Otherwise normal deduction is
  attempted, and failed deduction no longer recovers through enclosing
  arguments.

### 7. Parameter-Clause AST Parse To Fallback Parser

Current site:

- `cpp_decl_ast.cpp::parse_parameter_clause_ast(...)` falls back to the
  callback parser for shapes its direct AST path cannot currently lower.

Why this can be allowed temporarily:

- this is a parser/AST-coverage seam, not a semantic candidate fallback
- the direct AST path is cheap
- the fallback is localized and explicit

Why it still belongs in the plan:

- hard-fail mode should classify it separately so it does not hide semantic
  fallback failures
- long-term closure should replace the missing AST cases instead of expanding
  the callback path

Initial hard-fail category:

```text
parameter-clause-parser-fallback
```

Default status: classified allowed probe until semantic fallback categories are
closed.

Current status:

- Parameter-clause pack expansion now has an explicit AST hook. The direct
  parameter-clause parser asks for structured pack expansion before parsing the
  expanded clause.
- The generic fallback callback and the duplicate callsemantic fallback
  implementation are removed. A traced strict run over `pa18 pa19 pa21 pa22`
  no longer reports the old `parse-parameter-clause-fallback` path.

### 8. Sizeof Identifier Expression To Type Lookup

Current site:

- `semantic_expression.cpp::analyze_sizeof_expression(...)` used to analyze an
  id-expression operand, catch failure, then check value binding and fall back
  to type lookup.

Why this is debt:

- the parser and name lookup should classify whether the identifier names a
  type, value, or unresolved dependent entity
- exception from unevaluated expression analysis should not be the
  discriminator

Desired replacement:

- perform name classification first
- analyze expression only when value lookup wins
- resolve type-id semantics directly when type lookup wins

Initial hard-fail category:

```text
sizeof-expression-to-type-lookup
```

Current status:

- The exception fallback is removed. `sizeof(id)` now classifies the operand
  with value lookup first; value operands use unevaluated expression analysis,
  and non-values use structured type lookup through `lookup_type_node(...)`.
  Expression-analysis failure no longer widens to type lookup.

### 9. Builtin Call Probe Before Ordinary Call Lookup

Current site:

- `semantic_overload.cpp` tries builtin call expression recognition before
  ordinary id-expression value lookup.

Why this is probably allowed:

- the probe is syntactic and name-table based
- it does not run overload resolution first

Required check:

- verify builtin recognition respects ordinary declarations that should shadow
  builtin handling
- if shadowing works, leave this as an allowed cheap probe

Initial hard-fail category:

```text
builtin-call-probe
```

Default status: classified allowed probe unless tests show shadowing drift.

## Execution Order

1. Add hard-fail instrumentation for the high-risk categories.
2. Run `test-strict-nobuild` for the template strict owners and record the
   first failing category/test matrix.
3. Fix one category at a time by adding the missing semantic request data or
   pre-classifier.
4. For each fixed category, add or identify a regression that fails under hard
   fallback mode before the fix.
5. Keep default test behavior passing after every slice.
6. When a category reaches zero under strict mode, either remove the fallback or
   mark it as an allowed cheap probe with a direct predicate and no exception
   control flow.

## Current Slice

The semantic fallback audit is now part of normal compiler behavior:

```text
semantic_fallback_audit::hard_fail_if_enabled(category, location, detail)
```

Instrumented fallback categories fail unconditionally instead of requiring
`CPPGM_STRICT_SEMANTIC_FALLBACKS`. The current categories are:

- `operator-overload-to-builtin`
- `constructor-selection-fallback`
- `target-aware-to-generic-expression`
- `template-type-resolution-fallback`
- `template-id-text-parse-fallback`

The retired categories are:

- `sizeof-expression-to-type-lookup`, removed by classifying `sizeof(id)` with
  value/type lookup before expression analysis.
- `alias-parse-to-dependent-placeholder`, removed by classifying explicitly
  dependent class alias type-ids before parsing.
- `friend-deduction-to-enclosing-arguments`, removed by classifying
  current-class friend references before template deduction.
- `constructor-selection-fallback`, removed by classifying known aggregate
  forms before constructor selection and deleting the stale expression-source
  retry.
- `parameter-clause-parser-fallback`, removed by routing parameter-pack
  expansion through structured AST hooks and deleting the callback fallback
  seam.

The first replacement slice is in template type-id lookup. The parser and
template layers already carry enough structured syntax for most class/alias
template-ids and qualified namespace names, but
`parse_type_id_node_for_templates(...)` still tried text resolvers before using
that data directly. The new structured path resolves:

- unqualified alias template-ids such as `void_t<...>`
- unqualified class template-ids such as `Pair<int, T*>`
- qualified namespace/type lookups such as `std::_ClassicAlgPolicy`
- current-specialization spellings when the structured spelling matches the
  current instantiation

Discovery before that slice:

- `pa18` had 7 exit failures under
  `CPPGM_STRICT_SEMANTIC_FALLBACKS=template-type-resolution-fallback`.
- `pa19 pa21 pa22` had 54 `template-type-resolution-fallback` diagnostics.
- The failures were dominated by structured template-id spellings being
  widened into semantic-only or lookup-text resolution.

Discovery after that slice:

- `pa18`, `pa19`, and `pa21` no longer hard-fail this category; they show only
  the existing witness floor.
- `pa22/tests/spec/022-void-t-detector.t` exposed the remaining structured
  qualified-name gap. The text fallback had been rewriting
  `T::iterator_category` through the currently bound owner type
  (`iter_like` or `plain`) and using failure to find the member as SFINAE.
- The fix preserves the parser's leading `typename` bit on type-name AST nodes
  and teaches structured qualified-name lookup to resolve bound owner types
  directly. A missing member on a concrete bound owner is now a handled no-match,
  not a reason to widen into text lookup.
- A full `CPPGM_STRICT_SEMANTIC_FALLBACKS=1` run over
  `pa18 pa19 pa21 pa22` reached the existing witness-only floor for the
  categories instrumented at that point.

Follow-up discovery:

- `template-id-text-parse-fallback` marks type lookup that parsed a template-id
  from normalized text even though the request did not carry structured
  template-id syntax.
- `CPPGM_STRICT_SEMANTIC_FALLBACKS=template-id-text-parse-fallback` over
  `pa18 pa19 pa21 pa22` currently hard-fails 95 witness cases. Common spellings
  include `Op<typename Args>`, `enable_if<true,int>`, `allocator<int>`,
  `id<int>`, and `N::plus<void>`.
- The first cleanup moved dependent/template-parameter named-type state out of
  local text-prefix consumers in `template_argument_semantics.cpp`. New
  placeholders in that path are created with typed `NamedSemanticKind` metadata
  through `make_semantic_named(...)`; the legacy prefixed key spelling remains
  only as the compatibility identity string.
- The remaining fix is to carry structured template-id syntax into the
  text-only request sites so `resolve_type_lookup_text(...)` does not need to
  split normalized text to recover the template name and arguments.
- An intermediate version of the same slice raised the normal strict witness
  floor from 68 to 74 without LowIR or exit-code regressions. Those six drifts
  were fixed without text qualifier parsing by selecting the template-id syntax
  nearest the source-use location and by suppressing incidental source captures
  around alias body validation probes.
- The next cleanup reduced the strict `template-id-text-parse-fallback`
  inventory from 85 hard failures to 53 without changing the normal strict
  witness floor. It removed three text-first paths:
  - function-template deduction now resolves stored class-template
    instantiation arguments through `TemplateNamedTypeMetadata` and
    `resolve_selected_class_template_id(...)` instead of rewriting
    `Number<T>`-style spellings into `Number<int>` text;
  - non-dependent class-template types that already carry concrete
    instantiation metadata are kept as typed values instead of being reparsed
    for local-name spellings;
  - alias-template instantiation now tries the structured alias `type_id` AST
    before any rewritten type-id text, which removes the `id<decltype(...)>`
    family of hard failures.
- The dependent qualified-member group now uses structured semantic state
  instead of string prefixes such as `typename ` or `dependent type `. The
  named type carries the owner type, member path, and leading-`typename` bit,
  and dependent resolution walks that typed member path after the owner is
  instantiated. The opaque named key is only identity; semantic dispatch must
  use `NamedSemanticKind` and the structured member fields.
- A small follow-up reduced the strict `template-id-text-parse-fallback`
  inventory from 53 hard failures to 47 by preserving unbound local template
  placeholders as dependent before attempting concrete qualified-member lookup.
  This removes the `typename S<T>::type` family from the fallback inventory,
  where `S<T>` still contains an unbound owner placeholder and therefore should
  not probe concrete member lookup yet.
- The follow-up structured-member slice eliminated the remaining strict
  `template-id-text-parse-fallback` hard-fail inventory for PA18/PA19/PA21/PA22
  (`hard_fail_count 0`). It also replaced an ABI-side `dependent ` key-prefix
  check with `NamedSemanticKind` and uses the owner class's structured
  template arguments only for owner non-type parameters whose parameter type is
  dependent; ordinary function-template argument mangling keeps the Itanium
  dependent-parameter encoding.
- The broad alias/type rewrite compatibility helpers have been removed in small
  validated slices. The latest cleanups deleted the alias instantiation
  `typename`-stripping fallback and the local `__type_pack_element<...>...`
  empty-pack argument eraser; both kept PA18/PA19/PA21/PA22 at the known
  witness-only floor under normal strict mode and
  `CPPGM_STRICT_SEMANTIC_FALLBACKS=template-id-text-parse-fallback`.

Deferred text-rewrite removals to revisit after the current witness work:

- `callsemantic.cpp` still has `rewrite_instantiated_alias_text(...)`, a local
  alias instantiation chain that serializes bound types, templates, value packs,
  `decltype(...)` packs, type packs, non-type values, and visible aliases back
  into text before reparsing. The replacement should be structural alias pattern
  substitution plus typed pack expansion, not another special-case text
  filter.
- `template_argument_semantics.cpp` still prepares dependent named-type lookup
  text with the same kind of bound-name, pack, value, alias, and
  `decltype(...)` rewrites before resolving the prepared string. The next
  removal should carry the already-known owner/member/template-id state through
  `NamedSemanticKind` metadata or a small typed lookup request.
- `template_resolution.cpp` still rewrites default template arguments with bound
  arguments and then may rewrite bound type names/packs before resolving them.
  This should become default-argument instantiation over typed
  `TemplateArgument`/`TemplateArgumentSyntax` state, with text preserved only for
  source reporting.
- The preserving-dependent alias helpers
  (`rewrite_*_preserving_dependent_text`) remain useful as compatibility seams
  in a few resolution paths, but they should only survive where the caller has
  no structured syntax or semantic handle. Any same-site "try structured, then
  reparse rewritten text" fallback should be treated as debt.

Also classified during this slice:

- Partial-specialization matching can receive text-only template argument
  syntax for simple defaulted arguments such as `void`. That is not a failed
  structured candidate; no structured type-id/template-id was available. It is
  tracked as missing syntax capture, not as a hard semantic fallback.

Next fix:

- Move to the next fallback category and keep turning on one category at a time
  over the strict PA set, keeping lowir and exit-code failures fixed before
  committing each slice.

## Validation

Minimum validation per slice:

```sh
make -C dev cppgm++ CXX=/usr/local/opt/llvm/bin/clang++
make test-strict-nobuild \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  STRICT_PAS='pa19 pa20 pa22 pa23 pa24'
```

For non-template semantic categories, add the earliest owner PA strict or local
suite that exercises the construct.

## Completion Criteria

- no high-risk fallback category fires under strict mode for the active strict
  template owners
- default strict validation has no new LowIR or exit-code drift
- remaining fallbacks are documented allowed probes with cheap predicates
- no fallback predicate depends on exception text from a failed semantic
  strategy
