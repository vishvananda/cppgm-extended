# Witness Emission Collapse Plan

## Relationship to existing docs

This document is a tactical companion to
`docs/implemented/witness-emission-consolidation-plan.md`. The existing plan describes
*what* needs to happen (intent-level emitters, centralized binding,
producer-by-producer migration). This document inventories the actual
duplicated code in current `dev/src/` and proposes specific, narrow
collapses. Land these first; the existing plan's later phases become
mechanical once the collapses are in place.

## Goal

Reduce the number of places that have to be edited when witness format,
selection-kind, anchor, or binding policy changes.

## Zero-Drift Baseline Rules

The strict witness baseline is currently clean. Treat the collapses below as
behavior-preserving refactors unless a separate semantic bug is discovered.
Each implementation slice should keep the strict witness output at the same
level or improve it; do not update references just because a consolidation
changed rendering.

- For any new witness drift, assume the fix may belong in semantic analysis,
  the patched-Clang witness producer, or the Python normalization layer. Do not
  force `cppgm++` output to match by reparsing rendered text until those
  possibilities have been checked.
- Prefer semantic fixes over witness-layer recovery. Do not add new source-text
  reparses, renderer scans, or broad witness-side tracking to distinguish cases
  that semantic analysis already knows.
- If a witness mismatch is unclear, inspect the patched-Clang witness producer
  or Python normalization/filtering before assuming the reference is right; the
  reference side can also carry bugs or over-normalization.
- Treat divergence as a possible semantic bug first. A row that "only" differs
  in witness output may indicate that `cppgm++` completed, instantiated, or
  selected an entity on the wrong semantic path and merely happened to produce
  acceptable LowIR.
- Witness-layer changes should be small typed payloads attached at the producer
  only when witness output is active. Avoid auxiliary session-wide sets or
  catch-all bookkeeping unless the semantic model truly has no narrower owner.
- Current strict witness drift is tracked test-by-test in
  `legacy/strict-witness-regression-tracker.md`. Update that tracker after each
  behavior-changing edit so regressions are visible before moving to the next
  failure family.

Current count of direct emission sites (from grep on this branch after the
zero-drift cleanup and structured-AST perf merges):

- `record_class_use_source_use`: **17 semantic call sites** (15 in
  `callsemantic.cpp`, 1 in `template_instantiation.cpp`, 1 in
  `semantic_overload.cpp`)
- `record_alias_use_source_use`: **6 semantic call sites** (2 in
  `template_argument_semantics.cpp`, 2 in `template_specialization.cpp`,
  2 in `callsemantic.cpp`)
- `record_function_call_source_use`: **0 semantic call sites** after adding
  `emit_function_call(...)`; function-call producers still build their
  `FunctionCallSourceDecision` locally where selection, `declval`, or
  candidate-drop policy is site-specific.
- `record_variable_use_source_use`: **1 site**
  (`template_instantiation.cpp`)

Per the existing plan's grouping, every class-use site builds the same
8-field `ClassUseSourceDecision` by hand. Variation across sites is
**policy** (ownership, role, "explicit"/"defaulted"/"deduced", whether
to fan out nested uses) and **inputs** (which `ClassInfo`,
`ClassTemplateDecl`, `arguments`, `arg_texts`). The structural shape
of the decision is identical.

## Concrete duplications visible today

### D1. Builder boilerplate in `witness_api.cpp`

`make_class_use_source_use`, `make_function_call_source_use`,
`make_alias_use_source_use`, `make_variable_use_source_use`
(`witness_api.cpp:78-265`) all do the same eight-step prefix:

1. set `kind`, `role`, `ownership`
2. normalize `location`
3. build `spelling_anchor` from `decision.use_anchor`
4. derive `provenance_anchor` from `location`
5. normalize `selected_decl_anchor` (with the `decl_location` fallback
   in three of the four — class-use does it differently)
6. populate `selected_entity` from `template_name` +
   `selected_decl_location`
7. translate `decision.bindings` → `use.bindings`
8. translate `decision.specialization_bindings` →
   `use.specialization_bindings`

After step 8, each builder appends 0-5 kind-specific fields.

This is the simplest collapse: one `populate_common_source_use_fields(...)`
helper called by all four builders, then each adds its tail.

### D2. The `selected_decl_anchor.kind` ternary

The same 5-line ternary appears in at least 6 places, **always
identical**:

```cpp
decision.selected_decl_anchor.kind =
    source_decl_anchor_has_name_location(decl_anchor) ?
        witness::TemplateWitnessSourceAnchorKind::DeclarationName :
        (decision.selected_decl_location.empty() ?
             witness::TemplateWitnessSourceAnchorKind::None :
             witness::TemplateWitnessSourceAnchorKind::ApproximateDeclaration);
```

Locations:

- `template_argument_semantics.cpp:3310-3316, 3373-3379`
- `template_specialization.cpp:369-374, 426-431`
- `template_instantiation.cpp:4594-4599`
- `semantic_overload.cpp:786-791` (variant: tests `owner_anchor` instead)

Replace with one helper:
`witness::set_selected_decl_anchor(decision, anchor_cache, location)`.
Updates `selected_decl_location` and `selected_decl_anchor` together.
Six identical blocks become six one-liners.

### D3. `use_anchor` initialization

The pattern

```cpp
decision.location = use_location;
decision.use_anchor.location = use_location;
decision.use_anchor.kind = witness::TemplateWitnessSourceAnchorKind::UseSite;
```

appears inline in roughly 20 sites across `callsemantic.cpp`,
`template_argument_semantics.cpp`, `template_specialization.cpp`,
`template_instantiation.cpp`, `semantic_overload.cpp`,
`semantic_conversion.cpp`. Sometimes it is gated on
`source_location_points_at_identifier(...)`, sometimes not.

Collapse with two helpers:

```cpp
witness::set_use_anchor(decision, use_location);          // unconditional
witness::set_use_anchor_if_at_identifier(decision,
                                         use_location,
                                         identifier);     // gated
```

Both update `decision.location`, `decision.use_anchor.location`, and
`decision.use_anchor.kind` together.

### D4. The class-use recipe (the largest collapse)

The 13 class-use call sites in `callsemantic.cpp` follow the same
recipe with policy variation. Representative example pattern (one site
condensed):

```cpp
witness::ClassUseSourceDecision decision;
decision.location = chosen_use_location;
decision.template_name = qualified_template_name;
decision.selection = source_selection_kind_for_match_kind(specialization.kind);
decision.selected_decl_location = selected_decl_location;
decision.selected_decl_anchor.location = selected_decl_location;
decision.selected_decl_anchor.kind =
    selected_decl_location.empty() ?
        witness::TemplateWitnessSourceAnchorKind::None :
        witness::TemplateWitnessSourceAnchorKind::DeclarationName;
if (source_use_spells_template) {
    decision.use_anchor.location = chosen_use_location;
    decision.use_anchor.kind = witness::TemplateWitnessSourceAnchorKind::UseSite;
    if (binding_arg_texts &&
        (binding_arg_texts_from_source_template_id ||
         binding_arg_texts_from_exact_anchor)) {
        // build template_id_occurrence with mention checks
        ...
    }
}
if (binding_arg_texts) {
    template_api::append_template_witness_source_bindings(
        *this, decision.bindings,
        decl.parameters, arguments, *binding_arg_texts,
        "explicit", "defaulted");
} else {
    template_api::append_template_witness_source_bindings(
        *this, decision.bindings,
        decl.parameters, arguments, "explicit");
}
if (bound_parameters && bound_parameters != &decl.parameters) {
    template_api::append_template_witness_source_bindings(
        *this, decision.specialization_bindings,
        *bound_parameters, *bound_arguments, "deduced");
}
witness::record_class_use_source_use(decision, ownership, role);
witness::note_*_class_use_source_decision(decision);
emit_nested_class_use_source_events_from_location(use_scope,
                                                  decision.location,
                                                  force_source_owned,
                                                  decision.template_name);
```

Variation across the 13 sites:

| Axis | Values |
| --- | --- |
| ownership | `Direct`, `SourceOwned`, `NestedDerived` |
| role | `TypeUse`, `QualifierUse` |
| binding policy | `(explicit, defaulted)` with texts vs `explicit` only vs `deduced` only |
| specialization bindings | conditional on `bound_parameters != &decl.parameters` |
| nested fanout | yes (with `force_source_owned`) / no |
| source spelling | `source_use_spells_template` decides whether to build template-id occurrence |

This whole recipe should live in **one** function in `witness_api.cpp`:

```cpp
struct ClassUseEmitRequest {
    SemanticContext * ctx;          // for append_template_witness_source_bindings
    Scope * use_scope;              // for nested fanout
    std::string use_location;
    std::string anchor_identifier;  // if non-empty, gate use_anchor on it
    std::string qualified_template_name;
    template_api::ClassSpecializationSelection selection;
    ClassTemplateDecl * decl;
    std::vector<TemplateArgument> arguments;
    const std::vector<std::string> * binding_arg_texts;     // null → "explicit"/"deduced" without texts
    const std::vector<TemplateParameterInfo> * bound_parameters;     // null → no specialization bindings
    const std::vector<TemplateArgument> * bound_arguments;
    semantic_source_use::SourceUseOwnership ownership;
    semantic_source_use::SourceUseRole role;
    bool emit_nested_uses;
    SourceTemplateIdOccurrencePolicy occurrence_policy;       // None / FromBindingTexts
};

void emit_class_use(const ClassUseEmitRequest & req);
```

The 13 sites become 13 small request fills, each identifying the policy
explicitly. Future format changes touch one function.

The dependency-mention checks
(`template_argument_texts_mention_source_bindings` and the four
sibling predicates) move inside the helper, gated by
`occurrence_policy`.

### D5. The alias-use recipe

The 4 alias-use sites (`template_argument_semantics.cpp:3290-3325`,
`template_argument_semantics.cpp:3355-3395`,
`template_specialization.cpp:356-383`,
`template_specialization.cpp:409-437`) are essentially copy-paste of
each other:

- `decision.location = use_location`
- `decision.use_anchor` set to UseSite
- `decision.template_id_occurrence = make_source_template_id_occurrence(...)`
- `decision.template_name = alias_template_witness_entity(...)`
- `decision.selected_decl_location = alias_template_decl_location(...)`
- `decision.selected_decl_anchor` set with the D2 ternary
- `decision.expanded_to = ...` (only in some sites)
- `append_alias_template_source_bindings(...)` or
  `append_alias_pattern_source_bindings(...)` (different binding builder
  per site)
- `record_alias_use_source_use(...)`
- `note_alias_use_source_decision(...)`

Collapse: one `emit_alias_use(AliasUseEmitRequest)` helper. The two
binding builders become a single binding-builder callback parameter.

### D6. The variable-use recipe

`template_instantiation.cpp:4570-4720` builds a `VariableUseSourceDecision`
plus a custom 80-line `append_variable_use_bindings` lambda that
re-implements pack splitting and `defaulted`/`deduced` source policy.

This lambda largely reproduces what
`template_api::append_template_witness_source_bindings` does. The
difference is small: variable-template default arguments use
`"defaulted"` while non-defaulted use `"deduced"` instead of
`"explicit"`. That is a single boolean policy on the existing helper,
not an 80-line copy.

Collapse:

1. Add a `VariableTemplateBindingPolicy` to
   `append_template_witness_source_bindings`:
   defaults map to `"defaulted"`, non-defaults map to `"deduced"`
   (today the function uses `"explicit"`/`"defaulted"`).
2. Delete the local `append_variable_use_bindings` lambda.
3. Add `emit_variable_use(VariableUseEmitRequest)` mirroring
   `emit_class_use`.

After this, the variable-use site shrinks from ~150 lines to ~10.

### D7. Location selection logic

`source_location_for_name_in_subtree` followed by
`source_location_for_name_in_node` followed by
`source_location_for_node`, then
`normalize_template_witness_source_location`, then a
`source_location_points_at_identifier` guard, appears as ad-hoc cascades
in many places. For example
`callsemantic.cpp:8838-8859`,
`callsemantic.cpp:9121-9136`,
`callsemantic.cpp:17310-17460`.

Collapse with one helper:

```cpp
std::string select_witness_use_location(
    const CppAstNode & primary_subtree,
    const CppAstNode * fallback_subtree,
    const std::string & identifier,
    const TemplateWitnessContext & ctx);
```

Returns the best identifier-anchored location across the subtree
fallbacks, normalized, or empty if no location anchors at the
identifier.

### D8. The `witness_capture_enabled` short-circuit (work-in-progress)

The dirty tree at `/tmp/cppgm-blowup-fix-20260427` already wraps many
of the source-location lookups behind a local
`witness_capture_enabled` boolean (see
`callsemantic.cpp` around lines 3625, 17340, 17400 in that tree).

This pattern should generalize: every `emit_*_use(Request)` helper
should early-return at the top if `!source_capture_enabled(ctx)`. Then
all callers stop having to gate themselves. The current code has
~80 occurrences of `if(!witness::source_capture_enabled(...)) return;`
and `if(witness::source_capture_enabled(...))` blocks scattered across
files. Most of those become redundant once the helper does the check.

### D9. Renderer's parallel `WitnessEvent` model

`template_witness_renderer.cpp:585` defines a renderer-local
`WitnessEvent` that mirrors `SemanticSourceUse` (location, ownership,
selection, anchors, bindings, drops, candidate counts, etc.).

23 `normalize_*` helpers and 8 recovery-style helpers
(`strip_*`/`canonicalize_*`/`recover_*`/`reconstruct_*`/`synthesize_*`)
operate on this local model. The existing plan calls this out as
duplication; the situation has not improved on the current branch.

The collapse here is structural: stop translating
`SemanticSourceUse` into `WitnessEvent`. Make the renderer consume
`SemanticSourceUse` directly. Then the 23 normalizers can apply
in-place to a single canonical model. The renderer keeps only
deterministic ordering and final formatting, both of which are pure
functions of the canonical model.

This is a larger change than D1-D8. Sequence it after them.

## Proposed sequence

The aim is to land these in small, low-risk slices that each obviously
shrink the surface that future witness changes have to touch.

### Slice 1 — Trivial helpers (D1, D2, D3)

Status: partially landed. `witness_api.{h,cpp}` already has
`set_use_anchor`, `set_use_anchor_if_at_identifier`,
`set_selected_decl_anchor`, and a private
`populate_common_source_use_fields` shared by the four `make_*_source_use`
builders. The remaining work in this slice is to replace the residual
manual anchor assignments in semantic call sites where they are still simple
one-for-one uses of those helpers. The selected-declaration helper now also
has overloads for precomputed witness anchors and precomputed
location-plus-kind pairs, and the straightforward class/function producer
copies have been migrated to those helpers. The remaining simple manual
`use_anchor` assignments in semantic producers have also been replaced with
`set_use_anchor`; only explicit cache clearing still touches the field
directly.

Add to `witness_api.h`:

```cpp
namespace witness {

void set_use_anchor(SourceDecisionLike & decision,
                    const std::string & use_location);
void set_use_anchor_if_at_identifier(SourceDecisionLike & decision,
                                     const std::string & use_location,
                                     const std::string & identifier);
void set_selected_decl_anchor(SourceDecisionLike & decision,
                              const semantic_model::SourceDeclAnchorCache & anchor,
                              const std::string & location);

void populate_common_source_use_fields(
    semantic_source_use::SemanticSourceUse & use,
    semantic_source_use::SourceUseKind kind,
    semantic_source_use::SourceUseRole role,
    semantic_source_use::SourceUseOwnership ownership,
    const std::string & location,
    const TemplateWitnessSourceAnchor & use_anchor,
    const TemplateWitnessSourceAnchor & selected_decl_anchor,
    const std::string & selected_decl_location,
    const std::string & template_name);

}
```

Refactor the four `make_*_source_use` builders in `witness_api.cpp` to
call `populate_common_source_use_fields` then add their tail fields.
~150 lines collapse to ~70.

Refactor the six D2 sites and the ~20 D3 sites to use the new helpers.
Each becomes one line.

**Risk:** trivial. No behavior change — the helpers reproduce the
existing inline code byte-for-byte.

**Regression gate:** strict witness output for `pa18 pa19 pa21 pa22` should
remain green. Full `test-report` is still useful before larger commits, but
the collapse itself must not rely on reference churn.

### Slice 2 — `emit_alias_use` and `emit_variable_use` (D5, D6)

Status: implemented for all six alias-use semantic producers and the
variable-template use producer. `emit_alias_use` centralizes alias decision
construction/recording while still allowing producers to pass prebuilt typed
bindings and explicit declaration anchors when needed. `emit_variable_use`
centralizes variable decision construction/recording, and
`append_template_witness_source_bindings` now has an explicit policy for the
variable-template `"deduced"`/`"defaulted"` source split.

Two helpers in `witness_api.{h,cpp}`:

```cpp
struct AliasUseEmitRequest { ... };
void emit_alias_use(const AliasUseEmitRequest & req);

struct VariableUseEmitRequest { ... };
void emit_variable_use(const VariableUseEmitRequest & req);
```

Migrate the 4 alias-use sites and the 1 variable-use site. Delete the
local `append_variable_use_bindings` lambda after extending
`append_template_witness_source_bindings` with a binding-policy
parameter that selects `"defaulted"`/`"deduced"` vs
`"explicit"`/`"defaulted"`.

**Risk:** moderate. Five sites and one binding helper change. The
binding policy extension needs careful testing against
`pa18 pa19 pa21 pa22` strict witness sets.

**Regression gate:**

- `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
- Then full `make test-report`.

### Slice 3 — `emit_class_use` (D4)

The largest collapse. Land in two phases:

**3a.** Migrate the 7 simplest class-use sites
(qualified-owner/applied-definition paths in `callsemantic.cpp`,
overload owner-class in `semantic_overload.cpp`,
out-of-class applied-definition in `template_instantiation.cpp`).
These have stable inputs and no `template_id_occurrence` building.

Status: partially implemented. `emit_class_use` now centralizes construction,
recording, and note selection for the simple class-use request shape. The
strict-clean migration covers the out-of-class owner path, the overload
owner-class path, source-owned owner paths, the suppressed-call source-owned
path, and several straightforward `callsemantic.cpp` owner/qualifier sites.
The direct class-template reference paths that already have structured
decision inputs now fill `ClassUseEmitRequest` instead of recording
`ClassUseSourceDecision` directly. The text-recovery boundaries that first call
`build_class_use_source_decision_from_template_text(...)` now use
`emit_class_use_decision(...)`, keeping recovery localized while centralizing
the record-plus-note policy.

**3b.** Migrate the 6 complex sites
(direct class-template reference paths in `callsemantic.cpp`,
including the
`build_class_use_source_decision_from_template_text` cache and the
nested fanout). Move the cache into the helper.

**Risk:** high — the `template_id_occurrence` building in D4 includes
the dependency-mention checks (`template_argument_texts_mention_*`)
which are policy and must match current behavior on the existing
witness floor.

**Regression gate:** the strict-witness frontier
(`pa18 pa19 pa21 pa22`). The existing
`witness-emission-consolidation-plan.md` warns that pure mechanical
consolidation can freeze incorrect behavior. The way to avoid that
here is to migrate one site at a time and diff
`SemanticSourceUseTable` rows before/after for the affected test
files.

### Slice 4 — Generalize witness-off short-circuit (D8)

After Slices 1-3 land, every `emit_*_use(Request)` helper does its
own final source-capture check at the top.

Status: the request-shaped `emit_class_use`, `emit_alias_use`, and
`emit_variable_use` helpers already guard source capture and empty use
locations. `emit_function_call` now has the same source-capture and
empty-location guard before recording or noting the decision.

Do **not** blindly delete caller-side gates. Several gates intentionally avoid
expensive source-location recovery, AST text reconstruction, or template-id
searches when witness is disabled; removing those would regress the
non-witness hot path. Delete only gates that guard no work beyond filling an
already-cheap request and calling an emitter.

**Risk:** trivial.

**Regression gate:** `make test-report`.

### Slice 5 — Location selection (D7)

Add `select_witness_use_location(...)`. Migrate the obvious cascade
sites (~10 in `callsemantic.cpp`, plus a handful in
`template_*` files).

Status: started with an explicit ordered-candidate helper in
`callsemantic.cpp`. The helper is intentionally candidate-list based so each
call site still documents whether it prefers name-in-node, name-in-subtree, or
node-start fallback. Only identical identifier-anchored cascades should move to
this helper; sites that intentionally defer identifier validation should remain
local until their policy is made explicit. The `declval` template-use witness
path now uses the helper for its identifier-anchored subtree search.

**Risk:** moderate. Different sites use different fallback orders, so
the helper either has to take an explicit ordered fallback list or be
called with the right primary subtree. Prefer the explicit fallback
list shape.

### Slice 6 — Retire renderer's parallel model (D9)

Change `template_witness_renderer.cpp` to consume
`SemanticSourceUse` directly. Move normalizers to operate on the
canonical struct. Delete `WitnessEvent` once nothing references it.

Status: started by replacing the renderer-local `WitnessBinding` and
`WitnessDrop` structs with aliases of `semantic_source_use::SourceBinding` and
`semantic_source_use::SourceDrop`. This removes two duplicate leaf models while
leaving the larger `WitnessEvent` migration for smaller follow-up slices.
Renderer event ownership now also stores the original
`semantic_source_use::SourceUseOwnership`; the old `SourceOwned`-as-`Direct`
behavior is limited to the renderer's ordering policy. Use-anchor and
selected-declaration-anchor fields now use
`semantic_source_use::SourceAnchorKind` directly and preserve the original
semantic kind without import-time conversion. Renderer event selection now uses
`semantic_source_use::SourceSelectionKind` directly; the only remaining
selection mapping is the final display spelling for function-call explicit
specializations versus class/alias/variable explicit specializations.
Renderer event kind now aliases
`semantic_source_use::SourceUseKind`, removing the duplicate local kind enum
and the source-use-to-renderer-kind conversion.

This is a larger change. Defer until Slices 1-5 are settled.

**Risk:** high. Touches public/debug witness output. Will produce
formatting churn even when the underlying semantic data is correct.

## Out of scope here

These belong in the existing
`witness-emission-consolidation-plan.md` and
`witness-renderer-reparse-followup-plan.md`:

- Centralizing template witness binding text canonicalization
  (`int&` vs `int &`, `decltype (x)` vs `decltype(x)`).
- Source-pattern argument text preservation when witness expects
  `Alloc`/`Tp`/`Args...`.
- Inline-namespace lifecycle entity canonicalization.
- Removing source-line scanning in the renderer
  (`find_template_id_occurrences` etc.).

The collapses in Slices 1-6 do not depend on those, and those do not
depend on these collapses. They can run in parallel.

## What "done" looks like

After Slices 1-5:

- 4 `record_*_source_use` builders → 4, but each is ~30 lines instead of
  ~70.
- 13 class-use call sites in `callsemantic.cpp` → 13 sites that each
  fill a `ClassUseEmitRequest` and call `emit_class_use`. Each site is
  ~20 lines instead of ~80-150.
- 4 alias-use sites → 4 sites, each ~15 lines.
- 1 variable-use site → 1 site, ~15 lines, with no local lambda.
- 6 identical `selected_decl_anchor.kind` ternaries → 6 one-line
  helper calls.
- ~20 `use_anchor` triplet assignments → ~20 one-line helper calls.
- ~80 redundant `source_capture_enabled` caller-side gates → 0.
- ~10 ad-hoc location-cascade blocks → ~10
  `select_witness_use_location` calls.

The total line-count savings are not the point. The maintenance
benefit is: changing how `selected_decl_anchor` is computed touches
one helper instead of six call sites. Adding a new field to
`ClassUseSourceDecision` touches one builder instead of 13 call sites.

After Slice 6, the renderer ports from a parallel struct to the
canonical struct, eliminating a third model that has to track
schema changes.
