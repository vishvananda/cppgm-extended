# parse_type_argument_text removal (anchor-threading refactor)

Goal: remove the `parse_type_argument_text` text re-parse from the explicit
type-argument resolver (`template_resolution.cpp` resolve_template_argument).
Cases reach it with `syntax == nullptr` because a template-id was resolved by
NAME without registering a `ScopedExactTemplateTypeLookupAnchor` carrying
`argument_syntaxes`. Fix: register anchors at the producer sites.

Plan doc: docs/template-type-argument-reparse-removal-plan.md (Stages 1-4).
Already done (committed): removed 3 redundant text-keyed lookups + dead helper
(c09bcc26a) + dead imports (d7d245cae). Kept lookup_direct_bound + parse.

Probe: disabling parse_type_argument_text -> 35+ strict failures
(pa18 1, pa21 ?, pa23 34). Load-bearing.

## Anchor recipe (from callsemantic.cpp:11475)
ExactTemplateTypeLookupAnchor anchor;
anchor.template_text = template_id_syntax_text_preserving_spacing(*ts);
anchor.identifier    = template_lookup_fragment_identifier(anchor.template_text);
if(anchor.identifier.empty()) anchor.identifier = ts->name.name;
anchor.compact_key   = compact_lookup_text(anchor.template_text);
anchor.arg_texts     = ts->arguments;
anchor.arg_syntaxes  = ts->argument_syntaxes;
anchor.has_argument_list = true;
anchor.location      = normalize_template_witness_source_location(
    source_location_for_location_id(template_witness_context(), ts->source_location_id));
const ScopedExactTemplateTypeLookupAnchor guard(anchor);
// by-name resolution call ...
Pickup in lookup_type_impl: exact_template_type_lookup_anchor_arg_syntaxes (7418),
current_exact_template_type_lookup_anchor (7424).

## Workflow
- Env-gate the disable (CPPGM_NO_TYPE_ARG_REPARSE) so default build stays green.
- Trace (CPPGM_STAGEC_TRACE) in plural resolve_template_arguments when syntaxes
  null/empty: print join_template_texts + backtrace. Singular is inlined.
- Iterate: repro with disable+trace -> find site -> register anchor -> rebuild.
- Final: remove parse_type_argument_text + trace, full report green, commit.

## Sites found / fixed
### Site 1 (pa23/tests/spec/400-qualified-member-template-id-bool-constant.t)
Null-syntax texts: `[0, void(execution::prefer_only<execution::detail::tracked_t<0>>)]`
(the OWNER template-id's args; the function-type `void(...)` is what reparses).
Backtrace producer chain (top=reparse):
  resolve_template_arguments(plural, NULL syntaxes)
  <- try_resolve_class_template_id_locally
  <- resolve_template_id_syntax_type
  <- parse_template_id_argument_text
  <- parse_type_argument_text            (the reparse)
  <- resolve_member_template_owner_type_text  (owner passed as TEXT, tries
       parse_type_argument_text FIRST @2954, then a split->resolve fallback that
       builds owner_syntax.argument_syntaxes from TEXT only)
  <- try_resolve_qualified_member_template_id_type (has member-id `syntax`, NOT
       the owner's structured arg syntaxes)
  <- resolve_template_id_syntax_type
  <- evaluate_structured_bool_template_value
  <- evaluate_structured_template_member_bool_value (frame 8, HAS CppAstNode +
       qualifier_template_id = cppast_qualifier_template_id_syntax(expr,i) WITH
       argument_syntaxes)

ATTEMPTS (both FAILED to clear it):
1. Register ScopedExactTemplateTypeLookupAnchor at frame 8 with
   qualifier_template_id's arg_syntaxes -> NOT picked up: the owner-resolution
   path goes through resolve_member_template_owner_type_text /
   resolve_template_id_syntax_type, which DON'T consult the anchor (only
   lookup_type_impl @7418 does).
2. Pick up the anchor inside resolve_member_template_owner_type_text and override
   owner_syntax.argument_syntaxes -> STILL null at resolve_template_arguments:
   resolve_template_id_syntax_type(owner_syntax) -> try_resolve_class_template_id_
   locally drops the syntaxes (calls resolve_template_arguments with NULL).

ROOT FINDING: the structured argument_syntaxes are dropped at MULTIPLE layers
(resolve_template_id_syntax_type -> try_resolve_class_template_id_locally ->
resolve_template_arguments), and this case is RECURSIVELY nested
(supportable_properties<...>::is_valid_target<target>::value, function-type
bound `props`). The plan's single-anchor recipe assumes owner resolution flows
through lookup_type_impl; THIS chain does not. Real fix = thread
argument_syntaxes through resolve_template_id_syntax_type ->
try_resolve_class_template_id_locally -> resolve_template_arguments (multi-
signature), AND handle nested qualifiers. Large multi-layer refactor.

## Status
Stage 1 instrumentation built (env-gated CPPGM_NO_TYPE_ARG_REPARSE +
CPPGM_STAGEC_TRACE backtrace in plural resolve_template_arguments). Site 1 traced
+ 2 fix attempts (reverted). Full removal is a large multi-layer / multi-site
refactor (~37 strict, ~125 full failures). Committed cleanup: c09bcc26a +
d7d245cae (−122 lines, the cleanly-removable redundant lookups). The reparse
itself (parse_type_argument_text) is load-bearing and NOT yet removed.

## PROGRESS (keep-going session)
Mechanism SOLVED. Infrastructure (all default-green, behavior-preserving):
- template_source_utils.cpp: exact_template_type_lookup_anchor_arg_syntaxes now
  searches the WHOLE anchor stack, two-pass (exact compact-key match preferred
  over loose identifier match). KEY fix for nested owners.
- template_argument_semantics.cpp: resolve_member_template_owner_type_text tries
  the structured-anchor owner path FIRST (before parse_type_argument_text);
  evaluate_structured_template_member_bool_value registers anchors for ALL
  qualifier template-ids; build_owner_lookup_anchor helper.
- template_declaration_collector.cpp: owner_lookup_anchor_for_node helper +
  ScopedExactTemplateTypeLookupAnchor guards at out-of-class binding sites
  (conversion 866; regular method 1741).
CHAINS FIXED: chain 1 (qualified member template-id bool, 3 tests); chain 2
partial (out-of-class method binding via 1741). reparse-off strict failures:
~37 -> ~29. Default (reparse on) FULL report 3282/3282.
REMAINING reparse-off strict fails: pa18(1) pa21(2) pa23(28). More out-of-class
sub-paths (2415/2457/2681 method_template/with_resolution; special/static member;
explicit specialization) + member-typedef + std::function chains.
