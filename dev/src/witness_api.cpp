#include "witness_api.h"

#include <cctype>
#include <string>
#include <vector>

#include "semantic_model.h"
#include "semantic_context.h"
#include "parser_trace.h"

namespace witness {

bool source_capture_enabled(const SemanticContext & ctx)
{
  return source_capture_enabled() &&
         enabled(ctx.template_witness_context());
}

namespace {

std::string normalize_source_event_angle_spacing(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '>') {
      while(!out.empty() &&
            std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
        out.resize(out.size() - 1);
      }
    }
    out.push_back(text[i]);
  }
  return out;
}

semantic_source_use::SourceUseOwnership source_use_ownership_from_event(
    TemplateWitnessSourceOwnership ownership,
    bool source_owned)
{
  if(source_owned) {
    return semantic_source_use::SourceUseOwnership::SourceOwned;
  }
  switch(ownership) {
  case TemplateWitnessSourceOwnership::Direct:
    return semantic_source_use::SourceUseOwnership::Direct;
  case TemplateWitnessSourceOwnership::NestedDerived:
    return semantic_source_use::SourceUseOwnership::NestedDerived;
  }
  return semantic_source_use::SourceUseOwnership::Direct;
}

semantic_source_use::SourceSelectionKind source_selection_kind_from_event(
    TemplateWitnessSelectionKind kind)
{
  switch(kind) {
  case TemplateWitnessSelectionKind::None:
    return semantic_source_use::SourceSelectionKind::None;
  case TemplateWitnessSelectionKind::Primary:
    return semantic_source_use::SourceSelectionKind::Primary;
  case TemplateWitnessSelectionKind::PartialSpecialization:
    return semantic_source_use::SourceSelectionKind::PartialSpecialization;
  case TemplateWitnessSelectionKind::ExplicitSpecialization:
    return semantic_source_use::SourceSelectionKind::ExplicitSpecialization;
  case TemplateWitnessSelectionKind::Instantiation:
    return semantic_source_use::SourceSelectionKind::Instantiation;
  }
  return semantic_source_use::SourceSelectionKind::None;
}

semantic_source_use::SourceBinding source_binding_from_event(
    const TemplateWitnessSourceBinding & binding,
    bool normalize_angle_spacing = true);

std::string populate_common_source_use_fields(
    semantic_source_use::SemanticSourceUse & use,
    semantic_source_use::SourceUseKind kind,
    semantic_source_use::SourceUseRole role,
    semantic_source_use::SourceUseOwnership ownership,
    const std::string & location,
    const std::string & selected_decl_anchor_location,
    const std::string & selected_decl_location,
    const std::string & template_name,
    bool default_selected_decl_anchor)
{
  use.kind = kind;
  use.role = role;
  use.ownership = ownership;
  use.location = normalize_template_witness_source_location(location);
  const std::string normalized_selected_decl_location =
      normalize_template_witness_source_location(selected_decl_location);
  use.selected_decl_anchor_location =
      normalize_template_witness_source_location(
          selected_decl_anchor_location);
  if(default_selected_decl_anchor &&
     use.selected_decl_anchor_location.empty()) {
    use.selected_decl_anchor_location = normalized_selected_decl_location;
  }
  use.template_name = template_name;
  return normalized_selected_decl_location;
}

semantic_source_use::SemanticSourceUse make_class_use_source_use(
    const ClassUseEmitRequest & request,
    semantic_source_use::SourceUseOwnership ownership,
    semantic_source_use::SourceUseRole role)
{
  semantic_source_use::SemanticSourceUse use;
  use.source_traversal_order = request.source_traversal_order;
  use.semantic_class_template_identity =
      request.semantic_template;
  use.semantic_class_specialization_key =
      request.semantic_specialization_key;
  const std::string selected_decl_location = populate_common_source_use_fields(
      use,
      semantic_source_use::SourceUseKind::ClassUse,
      role,
      ownership,
      request.location,
      request.selected_decl_anchor_location,
      request.selected_decl_location,
      request.template_name,
      false);
  use.template_id_occurrence = request.template_id_occurrence;
  use.selection = source_selection_kind_from_event(request.selection);
  if(!request.template_name.empty() || !request.selected_decl_location.empty()) {
    use.selected_entity_decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < request.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(request.bindings[i]));
  }
  for(std::size_t i = 0; i < request.specialization_bindings.size(); ++i) {
    use.specialization_bindings.push_back(
        source_binding_from_event(request.specialization_bindings[i]));
  }
  return use;
}

void record_class_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const ClassUseEmitRequest & request,
    SourceUseOwnership ownership,
    SourceUseRole role)
{
  if(table == nullptr) {
    return;
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const semantic_source_use::SemanticSourceUse use =
      make_class_use_source_use(request, ownership, role);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      request.producer_site,
      use);
  semantic_source_use::record_source_use(*table, use);
#else
  semantic_source_use::record_source_use(
      *table,
      make_class_use_source_use(request, ownership, role));
#endif
}

semantic_source_use::SourceBinding source_binding_from_event(
    const TemplateWitnessSourceBinding & binding,
    bool normalize_angle_spacing)
{
  semantic_source_use::SourceBinding out;
  out.param = binding.param;
  out.arg = normalize_angle_spacing ?
      normalize_source_event_angle_spacing(binding.arg) : binding.arg;
  out.source = binding.source;
  out.type_like = binding.type_like;
  out.function_type_argument = binding.function_type_argument;
  out.structured_type_spelling = binding.structured_type_spelling;
  out.preserve_qualified_member = binding.preserve_qualified_member;
  out.pack_binding = binding.pack_binding;
  out.pack_aggregate = binding.pack_aggregate;
  out.pack_arguments.reserve(binding.pack_arguments.size());
  for(std::size_t i = 0; i < binding.pack_arguments.size(); ++i) {
    out.pack_arguments.push_back(
        normalize_angle_spacing ?
            normalize_source_event_angle_spacing(binding.pack_arguments[i]) :
            binding.pack_arguments[i]);
  }
  out.function_pointer_parameter = binding.function_pointer_parameter;
  return out;
}

semantic_source_use::SemanticSourceUse make_function_call_source_use(
    const FunctionCallSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUse use;
  use.source_traversal_order = decision.source_traversal_order;
  use.source_call_precedes_nested_callee =
      decision.source_call_precedes_nested_callee;
  use.semantic_owner_class_template_identity =
      decision.semantic_owner_class_template_identity;
  use.semantic_owner_class_specialization_key =
      decision.semantic_owner_class_specialization_key;
  const std::string selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::FunctionCall,
          decision.role,
          source_use_ownership_from_event(decision.ownership, false),
          decision.location,
          decision.selected_decl_anchor_location,
          decision.selected_decl_location,
          decision.template_name,
          true);
  use.selected = decision.selected;
  use.selected = normalize_source_event_angle_spacing(use.selected);
  use.selection = source_selection_kind_from_event(decision.selection);
  use.template_id_occurrence = decision.template_id_occurrence;
  if(!decision.selected.empty() || !decision.template_name.empty()) {
    use.selected_entity_decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < decision.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(decision.bindings[i]));
  }
  for(std::size_t i = 0; i < decision.specialization_bindings.size(); ++i) {
    use.specialization_bindings.push_back(
        source_binding_from_event(decision.specialization_bindings[i]));
  }
  for(std::size_t i = 0; i < decision.drops.size(); ++i) {
    semantic_source_use::SourceDrop drop;
    drop.candidate = normalize_source_event_angle_spacing(
        decision.drops[i].candidate);
    drop.location = decision.drops[i].location;
    drop.reason = decision.drops[i].reason;
    use.drops.push_back(drop);
  }
  use.candidate_count = decision.candidate_count;
  use.candidates_built = decision.candidates_built;
  use.candidates_viable = decision.candidates_viable;
  return use;
}

void record_function_call_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const FunctionCallSourceDecision & decision)
{
  if(table == nullptr) {
    return;
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const semantic_source_use::SemanticSourceUse use =
      make_function_call_source_use(decision);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      decision.producer_site,
      use);
  semantic_source_use::record_source_use(*table, use);
#else
  semantic_source_use::record_source_use(
      *table,
      make_function_call_source_use(decision));
#endif
}

semantic_source_use::SemanticSourceUse make_alias_use_source_use(
    const AliasUseEmitRequest & request)
{
  semantic_source_use::SemanticSourceUse use;
  use.source_traversal_order = request.source_traversal_order;
  const std::string location = request.use_location;
  std::string selected_decl_location = request.selected_decl_location;
  std::string selected_decl_anchor_location =
      request.selected_decl_anchor_location;
  if(!request.selected_decl_anchor_explicit) {
    set_selected_decl_anchor(selected_decl_location,
                             selected_decl_anchor_location,
                             request.selected_decl_location);
  }
  const std::string normalized_selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::AliasUse,
          semantic_source_use::SourceUseRole::TypeUse,
          SourceUseOwnership::Direct,
          location,
          selected_decl_anchor_location,
          selected_decl_location,
          request.template_name,
          true);
  use.template_id_occurrence = request.template_id_occurrence;
  if(!request.template_name.empty()) {
    use.selected_entity_decl_location = normalized_selected_decl_location;
  }
  for(std::size_t i = 0; i < request.bindings.size(); ++i) {
    // Alias source-occurrence arguments have already been rendered from their
    // semantic AST.  A generic angle-space pass cannot distinguish a template
    // closer from `>` or `>>` inside decltype and would corrupt the payload.
    use.bindings.push_back(source_binding_from_event(request.bindings[i],
                                                    false));
  }
  return use;
}

void record_alias_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const AliasUseEmitRequest & request)
{
  if(table == nullptr) {
    return;
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const semantic_source_use::SemanticSourceUse use =
      make_alias_use_source_use(request);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      request.producer_site,
      use);
  semantic_source_use::record_source_use(*table, use);
#else
  semantic_source_use::record_source_use(
      *table,
      make_alias_use_source_use(request));
#endif
}

semantic_source_use::SemanticSourceUse make_variable_use_source_use(
    const VariableUseEmitRequest & request)
{
  semantic_source_use::SemanticSourceUse use;
  const std::string location = request.use_location;
  const std::string selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::VariableUse,
          semantic_source_use::SourceUseRole::ValueUse,
          request.ownership,
          location,
          request.selected_decl_anchor_location,
          request.selected_decl_location,
          request.template_name,
          true);
  use.selection = source_selection_kind_from_event(request.selection);
  if(!request.template_name.empty()) {
    use.selected_entity_decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < request.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(request.bindings[i]));
  }
  for(std::size_t i = 0; i < request.specialization_bindings.size(); ++i) {
    use.specialization_bindings.push_back(
        source_binding_from_event(request.specialization_bindings[i]));
  }
  return use;
}

void record_variable_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const VariableUseEmitRequest & request)
{
  if(table == nullptr) {
    return;
  }
  const semantic_source_use::SemanticSourceUse use =
      make_variable_use_source_use(request);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const witness_provenance::ScopedUpstreamRoute provenance_route(
      use.ownership == semantic_source_use::SourceUseOwnership::NestedDerived ?
          witness_provenance::WitnessUpstreamRoute::
              VariableInitializerReplay :
          witness_provenance::WitnessUpstreamRoute::
              VariableDirectInstantiation);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      request.producer_site,
      use);
#endif
  semantic_source_use::record_source_use(*table, use);
}

bool variable_source_uses_match_ignoring_location_and_ownership(
    semantic_source_use::SemanticSourceUse lhs,
    semantic_source_use::SemanticSourceUse rhs)
{
  lhs.ownership = semantic_source_use::SourceUseOwnership::Direct;
  rhs.ownership = semantic_source_use::SourceUseOwnership::Direct;
  return semantic_source_use::variable_use_equivalent_ignoring_location(lhs,
                                                                         rhs);
}

void retain_variable_use_source_use(
    TemplateWitnessSession & session,
    const void * semantic_owner,
    const VariableUseEmitRequest & request)
{
  const semantic_source_use::SemanticSourceUse use =
      make_variable_use_source_use(request);
  for(std::size_t i = 0; i < session.pending_variable_source_uses.size(); ++i) {
    template_api::PendingVariableSourceUse & pending =
        session.pending_variable_source_uses[i];
    if(pending.semantic_owner != semantic_owner ||
       !variable_source_uses_match_ignoring_location_and_ownership(
           pending.source_use,
           use)) {
      continue;
    }

    const bool pending_is_nested =
        pending.source_use.ownership ==
            semantic_source_use::SourceUseOwnership::NestedDerived;
    const bool use_is_nested =
        use.ownership == semantic_source_use::SourceUseOwnership::NestedDerived;
    if(pending_is_nested != use_is_nested) {
      if(!use_is_nested) {
        pending.source_use = use;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
        pending.producer_site = request.producer_site;
#endif
      }
      return;
    }

    if(template_api::template_witness_detail::prefer_later_source_location(
           pending.source_use.location,
           use.location) == use.location) {
      pending.source_use = use;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
      pending.producer_site = request.producer_site;
#endif
    }
    return;
  }

  template_api::PendingVariableSourceUse pending;
  pending.semantic_owner = semantic_owner;
  pending.source_use = use;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  pending.producer_site = request.producer_site;
#endif
  session.pending_variable_source_uses.push_back(std::move(pending));
}

}  // namespace

void set_selected_decl_anchor(std::string & selected_decl_location,
                              std::string & selected_decl_anchor_location,
                              const std::string & decl_location)
{
  selected_decl_location =
      normalize_template_witness_source_location(decl_location);
  selected_decl_anchor_location = selected_decl_location;
}

void set_selected_decl_anchor(
    std::string & selected_decl_location,
    std::string & selected_decl_anchor_location,
    const semantic_model::SourceDeclAnchorCache & decl_anchor)
{
  set_selected_decl_anchor(
      selected_decl_location,
      selected_decl_anchor_location,
      semantic_model::source_decl_anchor_location(decl_anchor));
}

bool emit_class_use(const TemplateWitnessContext & ctx,
                    const ClassUseEmitRequest & request)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       request.location,
                       std::string("emit-class-use ctx template=") +
                           request.template_name +
                           " origin=" +
                           std::to_string(static_cast<int>(request.origin)) +
                           " traversal-order=" +
                           std::to_string(request.source_traversal_order));
  }
  if(request.location.empty()) {
    return false;
  }
  if(!source_location_is_from_primary_file(ctx, request.location)) {
    return false;
  }
  if(!class_use_recording_enabled(ctx, request.origin) &&
     !request.record_during_source_capture_pause) {
    return false;
  }
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  if(table == nullptr) {
    return false;
  }
  record_class_use_source_use_in_table(
                                       CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session)
                                       table,
                                       request,
                                       request.ownership,
                                       request.role);
  return true;
}

void emit_function_call(const TemplateWitnessContext & ctx,
                        const FunctionCallSourceDecision & decision)
{
  if(decision.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, decision.location)) {
    return;
  }
  if(!function_call_recording_enabled(ctx, decision.origin)) {
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  record_function_call_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session) table, decision);
}

void emit_alias_use(const TemplateWitnessContext & ctx,
                    const AliasUseEmitRequest & request)
{
  if(request.use_location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, request.use_location)) {
    return;
  }
  if(!source_capture_enabled(ctx)) {
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  record_alias_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session) table, request);
}

void emit_variable_use(const VariableUseEmitRequest & request)
{
  const bool capture_enabled = source_capture_enabled();
  if((!capture_enabled && !request.record_during_source_capture_pause) ||
     request.use_location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         request.use_location)) {
    return;
  }
  TemplateWitnessSession * session =
      template_api::current_template_witness_session();
  if(request.retain_until_semantic_finalization &&
     request.semantic_owner != nullptr &&
     session != nullptr) {
    retain_variable_use_source_use(*session,
                                   request.semantic_owner,
                                   request);
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  record_variable_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          session)
      table,
      request);
}

void finalize_variable_use_source_uses(TemplateWitnessSession * session)
{
  if(session == nullptr) {
    return;
  }
  for(std::size_t i = 0; i < session->pending_variable_source_uses.size(); ++i) {
    const template_api::PendingVariableSourceUse & pending =
        session->pending_variable_source_uses[i];
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
    const witness_provenance::ScopedUpstreamRoute provenance_route(
        pending.source_use.ownership ==
                semantic_source_use::SourceUseOwnership::NestedDerived ?
            witness_provenance::WitnessUpstreamRoute::
                VariableInitializerReplay :
            witness_provenance::WitnessUpstreamRoute::
                VariableDirectInstantiation);
    const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
        session,
        &session->source_use_table,
        pending.producer_site,
        pending.source_use);
#endif
    semantic_source_use::record_source_use(session->source_use_table,
                                           pending.source_use);
  }
  session->pending_variable_source_uses.clear();
}

bool append_source_drop(std::vector<TemplateWitnessSourceDrop> & out,
                        const std::string & candidate,
                        const std::string & location,
                        const std::string & reason)
{
  if(!source_capture_enabled() ||
     candidate.empty() ||
     location.empty() ||
     reason.empty()) {
    return false;
  }
  TemplateWitnessSourceDrop drop;
  drop.candidate = candidate;
  drop.location = location;
  drop.reason = reason;
  out.push_back(drop);
  return true;
}

bool append_unique_source_drop(SourceDropSet & drop_set,
                               std::vector<TemplateWitnessSourceDrop> & out,
                               const std::string & candidate,
                               const std::string & location,
                               const std::string & reason)
{
  if(!source_capture_enabled() ||
     candidate.empty() ||
     location.empty() ||
     reason.empty()) {
    return false;
  }
  SourceDropKey key;
  key.candidate = candidate;
  key.location = location;
  key.reason = reason;
  if(!drop_set.seen.insert(key).second) {
    return false;
  }
  return append_source_drop(out, candidate, location, reason);
}

}  // namespace witness
