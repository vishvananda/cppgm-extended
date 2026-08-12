#pragma once

#include <string>
#include <vector>

#include "template_witness.h"
#include "witness_provenance.h"

class SemanticContext;
namespace semantic_model {
struct ClassInfo;
struct ClassTemplateDecl;
struct FunctionBinding;
struct SourceDeclAnchorCache;
struct ValueBinding;
}

namespace witness {

using template_api::TemplateClosureReason;
using template_api::TemplateLifecycleCause;
using template_api::TemplateLifecycleEvent;
using template_api::TemplateLifecycleEventKind;
using template_api::TemplateWitnessContext;
using template_api::TemplateWitnessEntryContext;
using template_api::TemplateWitnessOrigin;
using template_api::TemplateWitnessSession;
using template_api::ScopedTemplateWitnessEntryContext;
using template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause;
using template_api::ScopedTemplateWitnessSession;
using template_api::ScopedTemplateWitnessSourceCapturePause;
using template_api::ScopedTemplateWitnessSourceTypeLookup;
using template_api::ScopedTemplateWitnessTypeLookupPause;

enum class ClassUseEmissionOrigin
{
  ResolvedTemplateId,
  DeclarationTypeSource,
  ExplicitSpecializationSource,
  QualifiedValueSource,
  NestedSourceTemplateId
};

enum class FunctionCallEmissionOrigin
{
  OverloadSelectedCall,
  AdmittedSourceCall,
  ConstexprDirectCall,
  DeclvalCall
};

inline const char * source_selection_text(
    semantic_source_use::SourceSelectionKind kind)
{
  switch(kind) {
  case semantic_source_use::SourceSelectionKind::None:
    return "";
  case semantic_source_use::SourceSelectionKind::Primary:
    return "primary";
  case semantic_source_use::SourceSelectionKind::PartialSpecialization:
    return "partial";
  case semantic_source_use::SourceSelectionKind::ExplicitSpecialization:
    return "explicit";
  case semantic_source_use::SourceSelectionKind::Instantiation:
    return "instantiation";
  }
  return "";
}

inline bool enabled(const TemplateWitnessContext & ctx)
{
  return ctx.session != nullptr;
}

inline bool source_capture_enabled()
{
  return template_api::template_witness_source_capture_enabled();
}

inline bool source_capture_enabled(const TemplateWitnessContext & ctx)
{
  return enabled(ctx) && source_capture_enabled();
}

bool source_capture_enabled(const SemanticContext & ctx);

inline bool source_file_matches_primary_file(const std::string & source_file,
                                             const std::string & primary_file)
{
  if(source_file.empty() || primary_file.empty()) {
    return false;
  }
  if(source_file == primary_file) {
    return true;
  }
  if(primary_file.size() > source_file.size() &&
     primary_file.compare(primary_file.size() - source_file.size(),
                          source_file.size(),
                          source_file) == 0 &&
     primary_file[primary_file.size() - source_file.size() - 1] == '/') {
    return true;
  }
  if(source_file.size() > primary_file.size() &&
     source_file.compare(source_file.size() - primary_file.size(),
                         primary_file.size(),
                         primary_file) == 0 &&
     source_file[source_file.size() - primary_file.size() - 1] == '/') {
    return true;
  }
  return false;
}

inline bool source_location_is_from_primary_file(
    const std::string & primary_source_file,
    const std::string & location)
{
  if(primary_source_file.empty()) {
    return true;
  }
  const template_api::template_witness_detail::ParsedSourceLocation parsed =
      template_api::template_witness_detail::parse_source_location(
          template_api::normalize_template_witness_source_location(location));
  return parsed.valid &&
         source_file_matches_primary_file(parsed.file, primary_source_file);
}

inline bool source_location_is_from_primary_file(
    const TemplateWitnessSession * session,
    const std::string & location)
{
  return session == nullptr ||
      source_location_is_from_primary_file(session->primary_source_file,
                                           location);
}

inline bool source_location_is_from_primary_file(
    const TemplateWitnessContext & ctx,
    const std::string & location)
{
  if(!ctx.primary_source_file.empty()) {
    return source_location_is_from_primary_file(ctx.primary_source_file,
                                                location);
  }
  return source_location_is_from_primary_file(ctx.session, location);
}

inline bool source_location_capture_enabled(
    const TemplateWitnessContext & ctx,
    const std::string & location)
{
  return source_capture_enabled(ctx) &&
         source_location_is_from_primary_file(ctx, location);
}

inline bool function_call_source_capture_enabled()
{
  return template_api::template_witness_function_call_source_capture_enabled();
}

inline bool class_use_source_capture_enabled()
{
  return source_capture_enabled();
}

inline bool class_use_source_capture_enabled(const TemplateWitnessContext & ctx)
{
  return source_capture_enabled(ctx);
}

inline bool class_use_origin_records_during_call_speculation(
    ClassUseEmissionOrigin origin)
{
  return origin != ClassUseEmissionOrigin::ResolvedTemplateId;
}

inline ClassUseEmissionOrigin nested_class_use_origin_for_ownership(
    semantic_source_use::SourceUseOwnership ownership)
{
  return ownership == semantic_source_use::SourceUseOwnership::SourceOwned ?
      ClassUseEmissionOrigin::NestedSourceTemplateId :
      ClassUseEmissionOrigin::ResolvedTemplateId;
}

inline bool class_use_recording_enabled(
    ClassUseEmissionOrigin origin = ClassUseEmissionOrigin::ResolvedTemplateId)
{
  if(!class_use_source_capture_enabled()) {
    return false;
  }
  return function_call_source_capture_enabled() ||
      class_use_origin_records_during_call_speculation(origin);
}

inline bool class_use_recording_enabled(
    const TemplateWitnessContext & ctx,
    ClassUseEmissionOrigin origin = ClassUseEmissionOrigin::ResolvedTemplateId)
{
  if(!class_use_source_capture_enabled(ctx)) {
    return false;
  }
  return function_call_source_capture_enabled() ||
      class_use_origin_records_during_call_speculation(origin);
}

inline bool function_call_origin_records_during_source_capture_pause(
    FunctionCallEmissionOrigin origin)
{
  return origin == FunctionCallEmissionOrigin::AdmittedSourceCall ||
      origin == FunctionCallEmissionOrigin::DeclvalCall;
}

inline bool function_call_recording_enabled(
    FunctionCallEmissionOrigin origin = FunctionCallEmissionOrigin::OverloadSelectedCall)
{
  (void)origin;
  return function_call_source_capture_enabled();
}

inline bool function_call_recording_enabled(
    const TemplateWitnessContext & ctx,
    FunctionCallEmissionOrigin origin = FunctionCallEmissionOrigin::OverloadSelectedCall)
{
  if(source_capture_enabled(ctx) && function_call_source_capture_enabled()) {
    return true;
  }
  return enabled(ctx) &&
      function_call_origin_records_during_source_capture_pause(origin);
}

using template_api::create_template_witness_session;
using template_api::current_template_witness_entry_context;
using template_api::make_template_closure_entry_context;
using template_api::normalize_template_witness_source_location;
using template_api::preferred_fragment_use_location;
using template_api::template_witness_lifecycle_events_by_origin;
using template_api::template_witness_source_type_lookup_active;
struct ClassUseEmitRequest : semantic_source_use::SemanticSourceUse
{
  ClassUseEmitRequest()
  {
    kind = semantic_source_use::SourceUseKind::ClassUse;
    role = semantic_source_use::SourceUseRole::TypeUse;
  }

  uint32_t source_occurrence_id = 0;
  const semantic_model::ClassInfo * semantic_instance = nullptr;
  ClassUseEmissionOrigin origin = ClassUseEmissionOrigin::ResolvedTemplateId;
  // Static-definition source rows remain pending until this semantic member
  // has a corresponding variable-instantiation transition.
  const semantic_model::ClassInfo * static_member_owner = nullptr;
  std::string static_member_name;
  // Reference resolution can select a partial before its definition is
  // instantiated. Defer public visibility until the final semantic graph
  // records the corresponding specialization materialization.
  bool partial_selection_visibility_deferred = false;
  // Nested source arguments use the selected instance as a type argument
  // before CPPGM completes it. For those occurrences, a direct enclosing-base
  // relationship is the materialization fact that matches Clang's AST.
  bool partial_selection_visibility_requires_enclosing_base = false;
  bool record_during_source_capture_pause = false;
};

struct VariableUseEmitRequest : semantic_source_use::SemanticSourceUse
{
  VariableUseEmitRequest()
  {
    kind = semantic_source_use::SourceUseKind::VariableUse;
    role = semantic_source_use::SourceUseRole::ValueUse;
  }

  const semantic_model::ValueBinding * semantic_owner = nullptr;
  bool retain_until_semantic_finalization = false;
  bool record_during_source_capture_pause = false;
};

inline semantic_source_use::SourceTemplateIdOccurrence
make_source_template_id_occurrence(
    const std::string & use_location,
    const std::vector<std::string> & source_arguments)
{
  semantic_source_use::SourceTemplateIdOccurrence occurrence;
  occurrence.present = true;
  occurrence.source_spelled = true;
  occurrence.argument_list_spelled = true;
  occurrence.empty_argument_list = source_arguments.empty();
  occurrence.name_location = use_location;
  for(std::size_t i = 0; i < source_arguments.size(); ++i) {
    semantic_source_use::SourceTemplateArgumentOccurrence argument;
    argument.text = source_arguments[i];
    argument.source_spelled = true;
    occurrence.arguments.push_back(argument);
  }
  return occurrence;
}

void set_selected_decl_anchor(std::string & selected_decl_location,
                              std::string & selected_decl_anchor_location,
                              const std::string & decl_location);
void set_selected_decl_anchor(
    std::string & selected_decl_location,
    std::string & selected_decl_anchor_location,
    const semantic_model::SourceDeclAnchorCache & decl_anchor);

bool emit_class_use(const TemplateWitnessContext & ctx,
                    const ClassUseEmitRequest & request);
void emit_alias_use(const TemplateWitnessContext & ctx,
                    semantic_source_use::SemanticSourceUse use);
void emit_variable_use(const VariableUseEmitRequest & request);
void finalize_variable_use_source_uses(TemplateWitnessSession * session);
void emit_function_call(const TemplateWitnessContext & ctx,
                        semantic_source_use::SemanticSourceUse use,
                        FunctionCallEmissionOrigin origin);

bool append_source_drop(std::vector<semantic_source_use::SourceDrop> & out,
                        const std::string & candidate,
                        const std::string & location,
                        const std::string & reason);

}  // namespace witness
