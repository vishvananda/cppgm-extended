#include "witness_api.h"

#include <cctype>
#include <string>
#include <vector>

#include "semantic_model.h"
#include "semantic_trace.h"
#include "parser_trace.h"

namespace witness {
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

semantic_source_use::SourceAnchorKind source_anchor_kind_from_event(
    TemplateWitnessSourceAnchorKind kind)
{
  switch(kind) {
  case TemplateWitnessSourceAnchorKind::None:
    return semantic_source_use::SourceAnchorKind::None;
  case TemplateWitnessSourceAnchorKind::UseSite:
    return semantic_source_use::SourceAnchorKind::Spelling;
  case TemplateWitnessSourceAnchorKind::DeclarationName:
    return semantic_source_use::SourceAnchorKind::DeclarationName;
  case TemplateWitnessSourceAnchorKind::ApproximateDeclaration:
    return semantic_source_use::SourceAnchorKind::ApproximateDeclaration;
  }
  return semantic_source_use::SourceAnchorKind::None;
}

semantic_source_use::SourceAnchor source_anchor_from_event(
    const TemplateWitnessSourceAnchor & anchor)
{
  semantic_source_use::SourceAnchor out;
  out.location = anchor.location;
  out.kind = source_anchor_kind_from_event(anchor.kind);
  return out;
}

TemplateWitnessSourceAnchor normalize_source_anchor(
    const TemplateWitnessSourceAnchor & anchor);
TemplateWitnessSourceAnchor normalized_decl_anchor_or_default(
    const TemplateWitnessSourceAnchor & anchor,
    const std::string & decl_location);

semantic_source_use::SourceBinding source_binding_from_event(
    const TemplateWitnessSourceBinding & binding);

std::string populate_common_source_use_fields(
    semantic_source_use::SemanticSourceUse & use,
    semantic_source_use::SourceUseKind kind,
    semantic_source_use::SourceUseRole role,
    semantic_source_use::SourceUseOwnership ownership,
    const std::string & location,
    const TemplateWitnessSourceAnchor & use_anchor,
    const TemplateWitnessSourceAnchor & selected_decl_anchor,
    const std::string & selected_decl_location,
    const std::string & template_name,
    bool default_selected_decl_anchor)
{
  use.kind = kind;
  use.role = role;
  use.ownership = ownership;
  use.location = normalize_template_witness_source_location(location);
  use.spelling_anchor = source_anchor_from_event(
      normalize_source_anchor(use_anchor));
  use.provenance_anchor.location = use.location;
  use.provenance_anchor.kind = use.location.empty() ?
      semantic_source_use::SourceAnchorKind::None :
      semantic_source_use::SourceAnchorKind::Provenance;
  const std::string normalized_selected_decl_location =
      normalize_template_witness_source_location(selected_decl_location);
  use.selected_decl_anchor = source_anchor_from_event(
      default_selected_decl_anchor ?
          normalized_decl_anchor_or_default(selected_decl_anchor,
                                            normalized_selected_decl_location) :
          normalize_source_anchor(selected_decl_anchor));
  use.template_name = template_name;
  return normalized_selected_decl_location;
}

semantic_source_use::SemanticSourceUse make_class_use_source_use(
    const ClassUseSourceDecision & decision,
    semantic_source_use::SourceUseOwnership ownership,
    semantic_source_use::SourceUseRole role)
{
  semantic_source_use::SemanticSourceUse use;
  const std::string selected_decl_location = populate_common_source_use_fields(
      use,
      semantic_source_use::SourceUseKind::ClassUse,
      role,
      ownership,
      decision.location,
      decision.use_anchor,
      decision.selected_decl_anchor,
      decision.selected_decl_location,
      decision.template_name,
      false);
  use.template_id_occurrence = decision.template_id_occurrence;
  use.selection = source_selection_kind_from_event(decision.selection);
  if(!decision.template_name.empty() || !decision.selected_decl_location.empty()) {
    use.selected_entity.kind = semantic_source_use::EntityRefKind::Named;
    use.selected_entity.name = decision.template_name;
    use.selected_entity.decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < decision.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(decision.bindings[i]));
  }
  for(std::size_t i = 0; i < decision.specialization_bindings.size(); ++i) {
    use.specialization_bindings.push_back(
        source_binding_from_event(decision.specialization_bindings[i]));
  }
  return use;
}

ClassUseSourceDecision class_use_source_decision_from_request(
    const ClassUseEmitRequest & request)
{
  ClassUseSourceDecision decision;
  CPPGM_SET_WITNESS_PRODUCER(decision, request.producer_site);
  decision.location = request.location;
  if(request.use_anchor_present && !request.use_anchor_location.empty()) {
    decision.use_anchor.location = request.use_anchor_location;
    decision.use_anchor.kind = TemplateWitnessSourceAnchorKind::UseSite;
  }
  decision.template_name = request.template_name;
  decision.selection = request.selection;
  decision.selected_decl_location = request.selected_decl_location;
  decision.selected_decl_anchor = request.selected_decl_anchor;
  decision.template_id_occurrence = request.template_id_occurrence;
  decision.bindings = request.bindings;
  decision.specialization_bindings = request.specialization_bindings;
  return decision;
}

void record_class_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const ClassUseSourceDecision & decision,
    SourceUseOwnership ownership,
    SourceUseRole role)
{
  if(table == nullptr) {
    return;
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const semantic_source_use::SemanticSourceUse use =
      make_class_use_source_use(decision, ownership, role);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      decision.producer_site,
      use);
  semantic_source_use::record_source_use(*table, use);
#else
  semantic_source_use::record_source_use(
      *table,
      make_class_use_source_use(decision, ownership, role));
#endif
}

semantic_source_use::SourceBinding source_binding_from_event(
    const TemplateWitnessSourceBinding & binding)
{
  semantic_source_use::SourceBinding out;
  out.param = binding.param;
  out.arg = normalize_source_event_angle_spacing(binding.arg);
  out.source = binding.source;
  out.type_like = binding.type_like;
  out.preserve_qualified_member = binding.preserve_qualified_member;
  out.pack_binding = binding.pack_binding;
  out.pack_aggregate = binding.pack_aggregate;
  out.pack_arguments.reserve(binding.pack_arguments.size());
  for(std::size_t i = 0; i < binding.pack_arguments.size(); ++i) {
    out.pack_arguments.push_back(
        normalize_source_event_angle_spacing(binding.pack_arguments[i]));
  }
  out.function_pointer_parameter = binding.function_pointer_parameter;
  return out;
}

TemplateWitnessSourceAnchor normalize_source_anchor(
    const TemplateWitnessSourceAnchor & anchor)
{
  TemplateWitnessSourceAnchor out = anchor;
  out.location = normalize_template_witness_source_location(out.location);
  if(out.location.empty()) {
    out.kind = TemplateWitnessSourceAnchorKind::None;
  }
  return out;
}

TemplateWitnessSourceAnchor normalized_decl_anchor_or_default(
    const TemplateWitnessSourceAnchor & anchor,
    const std::string & decl_location)
{
  TemplateWitnessSourceAnchor normalized = normalize_source_anchor(anchor);
  if(!normalized.location.empty()) {
    return normalized;
  }
  normalized.location = decl_location;
  normalized.kind = normalized.location.empty() ?
      TemplateWitnessSourceAnchorKind::None :
      TemplateWitnessSourceAnchorKind::ApproximateDeclaration;
  return normalized;
}

semantic_source_use::SemanticSourceUse make_function_call_source_use(
    const FunctionCallSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUse use;
  const std::string selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::FunctionCall,
          decision.role,
          source_use_ownership_from_event(decision.ownership, false),
          decision.location,
          decision.use_anchor,
          decision.selected_decl_anchor,
          decision.selected_decl_location,
          decision.template_name,
          true);
  use.selected = decision.selected;
  use.selected = normalize_source_event_angle_spacing(use.selected);
  use.selection = source_selection_kind_from_event(decision.selection);
  if(!decision.selected.empty() || !decision.template_name.empty()) {
    use.selected_entity.kind = semantic_source_use::EntityRefKind::Named;
    use.selected_entity.name =
        normalize_source_event_angle_spacing(
            !decision.selected.empty() ? decision.selected : decision.template_name);
    use.selected_entity.decl_location = selected_decl_location;
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

SourceUseOwnership alias_use_ownership_for_origin(AliasUseEmissionOrigin origin)
{
  switch(origin) {
  case AliasUseEmissionOrigin::NestedSourceTemplateId:
    return SourceUseOwnership::NestedDerived;
  case AliasUseEmissionOrigin::ResolvedAliasTemplateId:
  case AliasUseEmissionOrigin::DirectSourceTemplateId:
  case AliasUseEmissionOrigin::PatternTemplateId:
  case AliasUseEmissionOrigin::QualifiedSourceTemplateId:
    return SourceUseOwnership::Direct;
  }
  return SourceUseOwnership::Direct;
}

semantic_source_use::SemanticSourceUse make_alias_use_source_use(
    const AliasUseSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUse use;
  const std::string selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::AliasUse,
          semantic_source_use::SourceUseRole::TypeUse,
          decision.ownership,
          decision.location,
          decision.use_anchor,
          decision.selected_decl_anchor,
          decision.selected_decl_location,
          decision.template_name,
          true);
  use.expanded_to = decision.expanded_to;
  use.template_id_occurrence = decision.template_id_occurrence;
  if(!decision.template_name.empty()) {
    use.selected_entity.kind = semantic_source_use::EntityRefKind::Named;
    use.selected_entity.name = decision.template_name;
    use.selected_entity.decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < decision.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(decision.bindings[i]));
  }
  return use;
}

void record_alias_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const AliasUseSourceDecision & decision)
{
  if(table == nullptr) {
    return;
  }
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const semantic_source_use::SemanticSourceUse use =
      make_alias_use_source_use(decision);
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      decision.producer_site,
      use);
  semantic_source_use::record_source_use(*table, use);
#else
  semantic_source_use::record_source_use(
      *table,
      make_alias_use_source_use(decision));
#endif
}

semantic_source_use::SemanticSourceUse make_variable_use_source_use(
    const VariableUseSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUse use;
  const std::string selected_decl_location =
      populate_common_source_use_fields(
          use,
          semantic_source_use::SourceUseKind::VariableUse,
          semantic_source_use::SourceUseRole::ValueUse,
          decision.ownership,
          decision.location,
          decision.use_anchor,
          decision.selected_decl_anchor,
          decision.selected_decl_location,
          decision.template_name,
          true);
  use.selection = source_selection_kind_from_event(decision.selection);
  if(!decision.template_name.empty()) {
    use.selected_entity.kind = semantic_source_use::EntityRefKind::Named;
    use.selected_entity.name = decision.template_name;
    use.selected_entity.decl_location = selected_decl_location;
  }
  for(std::size_t i = 0; i < decision.bindings.size(); ++i) {
    use.bindings.push_back(source_binding_from_event(decision.bindings[i]));
  }
  for(std::size_t i = 0; i < decision.specialization_bindings.size(); ++i) {
    use.specialization_bindings.push_back(
        source_binding_from_event(decision.specialization_bindings[i]));
  }
  return use;
}

void record_variable_use_source_use_in_table(
    CPPGM_WITNESS_PROVENANCE_PARAMETER(TemplateWitnessSession * session)
    semantic_source_use::SemanticSourceUseTable * table,
    const VariableUseSourceDecision & decision)
{
  if(table == nullptr) {
    return;
  }
  const semantic_source_use::SemanticSourceUse use =
      make_variable_use_source_use(decision);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const witness_provenance::ScopedSourceUseAttempt provenance_attempt(
      session,
      table,
      decision.producer_site,
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
    const VariableUseSourceDecision & decision)
{
  const semantic_source_use::SemanticSourceUse use =
      make_variable_use_source_use(decision);
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
        pending.producer_site = decision.producer_site;
#endif
      }
      return;
    }

    if(template_api::template_witness_detail::prefer_later_source_location(
           pending.source_use.location,
           use.location) == use.location) {
      pending.source_use = use;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
      pending.producer_site = decision.producer_site;
#endif
    }
    return;
  }

  template_api::PendingVariableSourceUse pending;
  pending.semantic_owner = semantic_owner;
  pending.source_use = use;
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  pending.producer_site = decision.producer_site;
#endif
  session.pending_variable_source_uses.push_back(std::move(pending));
}

void note_class_use_source_decision_impl(const ClassUseSourceDecision & decision)
{
  (void)decision;
}

}  // namespace

void set_use_anchor(std::string & decision_location,
                    TemplateWitnessSourceAnchor & use_anchor,
                    const std::string & use_location)
{
  decision_location = use_location;
  use_anchor.location = use_location;
  use_anchor.kind = TemplateWitnessSourceAnchorKind::UseSite;
}

bool set_use_anchor_if_at_identifier(std::string & decision_location,
                                     TemplateWitnessSourceAnchor & use_anchor,
                                     const std::string & use_location,
                                     const std::string & identifier)
{
  if(!semantic_trace::source_location_points_at_identifier(use_location,
                                                           identifier)) {
    return false;
  }
  set_use_anchor(decision_location, use_anchor, use_location);
  return true;
}

void set_selected_decl_anchor(std::string & selected_decl_location,
                              TemplateWitnessSourceAnchor & selected_decl_anchor,
                              const std::string & decl_location,
                              bool has_name_location)
{
  selected_decl_location =
      normalize_template_witness_source_location(decl_location);
  selected_decl_anchor.location = selected_decl_location;
  selected_decl_anchor.kind =
      has_name_location ?
          TemplateWitnessSourceAnchorKind::DeclarationName :
          (selected_decl_location.empty() ?
               TemplateWitnessSourceAnchorKind::None :
               TemplateWitnessSourceAnchorKind::ApproximateDeclaration);
}

void set_selected_decl_anchor(std::string & selected_decl_location,
                              TemplateWitnessSourceAnchor & selected_decl_anchor,
                              const std::string & decl_location,
                              TemplateWitnessSourceAnchorKind kind)
{
  selected_decl_location = decl_location;
  selected_decl_anchor.location = decl_location;
  selected_decl_anchor.kind = kind;
}

void set_selected_decl_anchor(std::string & selected_decl_location,
                              TemplateWitnessSourceAnchor & selected_decl_anchor,
                              const TemplateWitnessSourceAnchor & decl_anchor)
{
  selected_decl_location = decl_anchor.location;
  selected_decl_anchor = decl_anchor;
}

void set_selected_decl_anchor(
    std::string & selected_decl_location,
    TemplateWitnessSourceAnchor & selected_decl_anchor,
    const semantic_model::SourceDeclAnchorCache & decl_anchor)
{
  set_selected_decl_anchor(
      selected_decl_location,
      selected_decl_anchor,
      semantic_model::source_decl_anchor_location(decl_anchor),
      semantic_model::source_decl_anchor_has_name_location(decl_anchor));
}

void emit_class_use(const ClassUseEmitRequest & request)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       request.location,
                       std::string("emit-class-use global template=") +
                           request.template_name +
                           " origin=" +
                           std::to_string(static_cast<int>(request.origin)));
  }
  if(request.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         request.location)) {
    return;
  }
  const ClassUseSourceDecision decision =
      class_use_source_decision_from_request(request);
  emit_class_use_decision(decision,
                          request.ownership,
                          request.role,
                          request.origin);
}

void emit_class_use(const TemplateWitnessContext & ctx,
                    const ClassUseEmitRequest & request)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       request.location,
                       std::string("emit-class-use ctx template=") +
                           request.template_name +
                           " origin=" +
                           std::to_string(static_cast<int>(request.origin)));
  }
  if(request.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, request.location)) {
    return;
  }
  if(!class_use_recording_enabled(ctx, request.origin)) {
    return;
  }
  const ClassUseSourceDecision decision =
      class_use_source_decision_from_request(request);
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  record_class_use_source_use_in_table(
                                       CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session)
                                       table,
                                       decision,
                                       request.ownership,
                                       request.role);
  if(request.ownership == SourceUseOwnership::SourceOwned) {
    note_source_owned_class_use_source_decision(decision);
  } else {
    note_class_use_source_decision(decision);
  }
}

void emit_class_use_decision(
    const ClassUseSourceDecision & decision,
    SourceUseOwnership ownership,
    SourceUseRole role,
    ClassUseEmissionOrigin origin)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       decision.location,
                       std::string("emit-class-use-decision template=") +
                           decision.template_name +
                           " origin=" +
                           std::to_string(static_cast<int>(origin)));
  }
  if(decision.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  if(!class_use_recording_enabled(origin)) {
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  if(table == nullptr) {
    return;
  }
  record_class_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          template_api::current_template_witness_session())
      table,
      decision,
      ownership,
      role);
  if(ownership == SourceUseOwnership::SourceOwned) {
    note_source_owned_class_use_source_decision(decision);
  } else {
    note_class_use_source_decision(decision);
  }
}

void record_class_use_source_use(
    const ClassUseSourceDecision & decision,
    SourceUseOwnership ownership,
    SourceUseRole role)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       decision.location,
                       std::string("record-class-use-source-use template=") +
                           decision.template_name);
  }
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  if(table == nullptr || !class_use_recording_enabled()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  record_class_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          template_api::current_template_witness_session())
      table,
      decision,
      ownership,
      role);
}

void record_source_owned_class_use_source_use(
    const ClassUseSourceDecision & decision,
    SourceUseRole role)
{
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       decision.location,
                       std::string("record-source-owned-class-use template=") +
                           decision.template_name);
  }
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  if(table == nullptr || !class_use_recording_enabled()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  record_class_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          template_api::current_template_witness_session())
      table,
      decision,
      semantic_source_use::SourceUseOwnership::SourceOwned,
      role);
}

void record_source_owned_class_use_source_use(
    const TemplateWitnessContext & ctx,
    const ClassUseSourceDecision & decision,
    SourceUseRole role)
{
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  const bool capture_enabled = ctx.session != nullptr ?
      class_use_recording_enabled(ctx) :
      class_use_recording_enabled();
  if(table == nullptr || !capture_enabled) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, decision.location)) {
    return;
  }
  record_class_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session)
      table,
      decision,
      semantic_source_use::SourceUseOwnership::SourceOwned,
      role);
}

void record_function_call_source_use(
    const FunctionCallSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  if(table == nullptr || !function_call_recording_enabled(decision.origin)) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  record_function_call_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          template_api::current_template_witness_session())
      table,
      decision);
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
  note_function_call_source_decision(decision);
}

void emit_function_call(const FunctionCallSourceDecision & decision)
{
  if(!function_call_recording_enabled(decision.origin) ||
     decision.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  record_function_call_source_use(decision);
  note_function_call_source_decision(decision);
}

void record_alias_use_source_use(
    const AliasUseSourceDecision & decision)
{
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  if(table == nullptr || !source_capture_enabled()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         decision.location)) {
    return;
  }
  record_alias_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          template_api::current_template_witness_session())
      table,
      decision);
}

void record_alias_use_source_use(
    const TemplateWitnessContext & ctx,
    const AliasUseSourceDecision & decision)
{
  if(!alias_use_recording_enabled(ctx)) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, decision.location)) {
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  record_alias_use_source_use_in_table(
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
  if(!alias_use_recording_enabled(ctx, request.origin)) {
    return;
  }
  AliasUseSourceDecision decision;
  CPPGM_SET_WITNESS_PRODUCER(decision, request.producer_site);
  set_use_anchor(decision.location,
                 decision.use_anchor,
                 request.use_location);
  decision.template_id_occurrence = request.template_id_occurrence;
  decision.template_name = request.template_name;
  decision.ownership = alias_use_ownership_for_origin(request.origin);
  if(request.selected_decl_anchor_explicit) {
    decision.selected_decl_location = request.selected_decl_location;
    decision.selected_decl_anchor = request.selected_decl_anchor;
  } else {
    set_selected_decl_anchor(decision.selected_decl_location,
                             decision.selected_decl_anchor,
                             request.selected_decl_location,
                             request.selected_decl_has_name_location);
  }
  decision.expanded_to = request.expanded_to;
  decision.bindings = request.bindings;
  semantic_source_use::SemanticSourceUseTable * table = ctx.source_use_table;
  if(table == nullptr && ctx.session != nullptr) {
    table = &ctx.session->source_use_table;
  }
  record_alias_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(ctx.session) table, decision);
  note_alias_use_source_decision(decision);
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
  VariableUseSourceDecision decision;
  CPPGM_SET_WITNESS_PRODUCER(decision, request.producer_site);
  decision.location = request.use_location;
  if(!request.use_anchor_identifier.empty()) {
    set_use_anchor_if_at_identifier(decision.location,
                                    decision.use_anchor,
                                    request.use_location,
                                    request.use_anchor_identifier);
  } else {
    set_use_anchor(decision.location,
                   decision.use_anchor,
                   request.use_location);
  }
  decision.template_name = request.template_name;
  decision.ownership = request.ownership;
  decision.selection = request.selection;
  decision.selected_decl_location = request.selected_decl_location;
  decision.selected_decl_anchor = request.selected_decl_anchor;
  decision.bindings = request.bindings;
  decision.specialization_bindings = request.specialization_bindings;
  TemplateWitnessSession * session =
      template_api::current_template_witness_session();
  if(request.retain_until_semantic_finalization &&
     request.semantic_owner != nullptr &&
     session != nullptr) {
    retain_variable_use_source_use(*session,
                                   request.semantic_owner,
                                   decision);
    note_variable_use_source_decision(decision);
    return;
  }
  semantic_source_use::SemanticSourceUseTable * table =
      template_api::current_semantic_source_use_table();
  record_variable_use_source_use_in_table(
      CPPGM_WITNESS_PROVENANCE_ARGUMENT(
          session)
      table,
      decision);
  note_variable_use_source_decision(decision);
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

void note_class_use_source_decision(const ClassUseSourceDecision & decision)
{
  note_class_use_source_decision_impl(decision);
}

void note_source_owned_class_use_source_decision(
    const ClassUseSourceDecision & decision)
{
  note_class_use_source_decision_impl(decision);
}

void note_alias_use_source_decision(const AliasUseSourceDecision & decision)
{
  (void)decision;
}

void note_variable_use_source_decision(const VariableUseSourceDecision & decision)
{
  (void)decision;
}

void note_function_call_source_decision(const FunctionCallSourceDecision & decision)
{
  (void)decision;
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
