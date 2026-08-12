#include "witness_api.h"

#include <cctype>
#include <map>
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

void normalize_source_binding(semantic_source_use::SourceBinding & binding,
                              bool normalize_angle_spacing)
{
  if(normalize_angle_spacing) {
    binding.arg = normalize_source_event_angle_spacing(binding.arg);
  }
  for(std::size_t i = 0; i < binding.pack_arguments.size(); ++i) {
    if(normalize_angle_spacing) {
      binding.pack_arguments[i] =
          normalize_source_event_angle_spacing(binding.pack_arguments[i]);
    }
  }
}

semantic_source_use::SemanticSourceUse normalized_source_use(
    semantic_source_use::SemanticSourceUse use,
    bool normalize_binding_angle_spacing,
    bool default_selected_decl_anchor)
{
  use.location = normalize_template_witness_source_location(use.location);
  use.selected_entity_decl_location =
      normalize_template_witness_source_location(
          use.selected_entity_decl_location);
  use.selected_decl_anchor_location =
      normalize_template_witness_source_location(
          use.selected_decl_anchor_location);
  if(default_selected_decl_anchor &&
     use.selected_decl_anchor_location.empty()) {
    use.selected_decl_anchor_location = use.selected_entity_decl_location;
  }
  use.selected = normalize_source_event_angle_spacing(use.selected);
  for(std::size_t i = 0; i < use.bindings.size(); ++i) {
    normalize_source_binding(use.bindings[i], normalize_binding_angle_spacing);
  }
  for(std::size_t i = 0; i < use.specialization_bindings.size(); ++i) {
    normalize_source_binding(
        use.specialization_bindings[i], normalize_binding_angle_spacing);
  }
  for(std::size_t i = 0; i < use.drops.size(); ++i) {
    use.drops[i].candidate =
        normalize_source_event_angle_spacing(use.drops[i].candidate);
  }
  return use;
}

void record_class_use_source_use_in_session(
    TemplateWitnessSession * session,
    const ClassUseEmitRequest & request)
{
  if(session == nullptr) {
    return;
  }
  const semantic_source_use::SemanticSourceUse use =
      normalized_source_use(request, true, false);
  semantic_source_use::record_source_use(session->source_use_table, use);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  witness_provenance::note_source_use_publication(
      session,
      use);
#endif
}

void record_function_call_source_use_in_session(
    TemplateWitnessSession * session,
    semantic_source_use::SemanticSourceUse use)
{
  if(session == nullptr) {
    return;
  }
  use = normalized_source_use(std::move(use), true, true);
  semantic_source_use::record_source_use(session->source_use_table, use);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  witness_provenance::note_source_use_publication(
      session,
      use);
#endif
}

void record_alias_use_source_use_in_session(
    TemplateWitnessSession * session,
    semantic_source_use::SemanticSourceUse use)
{
  if(session == nullptr) {
    return;
  }
  // Alias source-occurrence arguments have already been rendered from their
  // semantic AST. A generic angle-space pass cannot distinguish a template
  // closer from `>` or `>>` inside decltype and would corrupt the payload.
  use = normalized_source_use(std::move(use), false, true);
  semantic_source_use::record_source_use(session->source_use_table, use);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  witness_provenance::note_source_use_publication(
      session,
      use);
#endif
}

void record_variable_use_source_use_in_session(
    TemplateWitnessSession * session,
    const VariableUseEmitRequest & request)
{
  if(session == nullptr) {
    return;
  }
  const semantic_source_use::SemanticSourceUse use =
      normalized_source_use(request, true, true);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  const witness_provenance::ScopedUpstreamRoute provenance_route(
      use.ownership == semantic_source_use::SourceUseOwnership::NestedDerived ?
          witness_provenance::WitnessUpstreamRoute::
              VariableInitializerReplay :
          witness_provenance::WitnessUpstreamRoute::
              VariableDirectInstantiation);
#endif
  semantic_source_use::record_source_use(session->source_use_table, use);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
  witness_provenance::note_source_use_publication(
      session,
      use);
#endif
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
  if(ctx.session == nullptr) {
    return false;
  }
  record_class_use_source_use_in_session(ctx.session, request);
  return true;
}

void emit_function_call(const TemplateWitnessContext & ctx,
                        semantic_source_use::SemanticSourceUse use,
                        FunctionCallEmissionOrigin origin)
{
  if(use.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, use.location)) {
    return;
  }
  if(!function_call_recording_enabled(ctx, origin)) {
    return;
  }
  record_function_call_source_use_in_session(ctx.session, std::move(use));
}

void emit_alias_use(const TemplateWitnessContext & ctx,
                    semantic_source_use::SemanticSourceUse use)
{
  if(use.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(ctx, use.location)) {
    return;
  }
  if(!source_capture_enabled(ctx)) {
    return;
  }
  record_alias_use_source_use_in_session(ctx.session, std::move(use));
}

void emit_variable_use(const VariableUseEmitRequest & request)
{
  const bool capture_enabled = source_capture_enabled();
  if((!capture_enabled && !request.record_during_source_capture_pause) ||
     request.location.empty()) {
    return;
  }
  if(!source_location_is_from_primary_file(
         template_api::current_template_witness_session(),
         request.location)) {
    return;
  }
  TemplateWitnessSession * session =
      template_api::current_template_witness_session();
  record_variable_use_source_use_in_session(session, request);
}

void publish_variable_source_use_results(TemplateWitnessSession * session)
{
  if(session == nullptr) {
    return;
  }
  for(std::map<const semantic_model::ValueBinding *,
               semantic_source_use::SemanticSourceUse>::const_iterator it =
          session->variable_source_use_results.begin();
      it != session->variable_source_use_results.end();
      ++it) {
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
    const witness_provenance::ScopedUpstreamRoute provenance_route(
        it->second.ownership ==
                semantic_source_use::SourceUseOwnership::NestedDerived ?
            witness_provenance::WitnessUpstreamRoute::
                VariableInitializerReplay :
            witness_provenance::WitnessUpstreamRoute::
                VariableDirectInstantiation);
#endif
    const semantic_source_use::SemanticSourceUse use =
        normalized_source_use(it->second, true, true);
    semantic_source_use::record_source_use(session->source_use_table, use);
#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
    witness_provenance::note_source_use_publication(
        session,
        use);
#endif
  }
  session->variable_source_use_results.clear();
}

bool append_source_drop(std::vector<semantic_source_use::SourceDrop> & out,
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
  semantic_source_use::SourceDrop drop;
  drop.candidate = candidate;
  drop.location = location;
  drop.reason = reason;
  out.push_back(drop);
  return true;
}

}  // namespace witness
