#pragma once

#include <cctype>
#include <string>
#include <vector>

namespace semantic_model {
struct ClassTemplateDecl;
}

namespace semantic_source_use {

enum class SourceUseKind
{
  FunctionCall,
  ClassUse,
  AliasUse,
  VariableUse,
};

enum class SourceUseRole
{
  Unknown,
  TypeUse,
  MaterializedTypeUse,
  QualifierUse,
  StaticMemberDefinitionOwner,
  ValueUse,
  CallUse,
  DeclvalCall,
};

enum class SourceUseOwnership
{
  Direct,
  NestedDerived,
  SourceOwned,
};

enum class SourceSelectionKind
{
  None,
  Primary,
  PartialSpecialization,
  ExplicitSpecialization,
  Instantiation,
};

struct SourceTemplateArgumentOccurrence
{
  std::string text;
  std::string semantic_text;
  bool source_spelled = false;
  bool dependent = false;
  bool current_specialization = false;
  bool preserve_qualified_member = false;
  bool referenced_value_initializer_uses_template = false;
  std::vector<std::string> referenced_value_entities;
  std::vector<std::string> referenced_value_decl_locations;
};

// Renderer decisions should consume this typed occurrence data instead of
// adding new source-text parsing. If witness rendering needs more facts later,
// extend this payload at the semantic construction site.
struct SourceTemplateIdOccurrence
{
  bool present = false;
  bool source_spelled = false;
  bool argument_list_spelled = false;
  bool empty_argument_list = false;
  bool in_template_body = false;
  bool synthesized = false;
  bool exact_source_arguments = false;
  bool conversion_result_type_use = false;
  bool function_result_type_use = false;
  bool current_specialization_use = false;
  bool has_dependent_argument = false;
  bool has_current_specialization_argument = false;
  std::string name_location;
  std::vector<SourceTemplateArgumentOccurrence> arguments;
};

struct SourceBinding
{
  std::string param;
  std::string arg;
  std::string source;
  bool type_like = false;
  bool function_type_argument = false;
  bool structured_type_spelling = false;
  bool preserve_qualified_member = false;
  bool pack_binding = false;
  bool pack_aggregate = false;
  std::vector<std::string> pack_arguments;
  bool function_pointer_parameter = false;
};

struct SourceDrop
{
  std::string candidate;
  std::string location;
  std::string reason;
};

struct SemanticSourceUse
{
  SourceUseKind kind = SourceUseKind::FunctionCall;
  SourceUseRole role = SourceUseRole::Unknown;
  SourceUseOwnership ownership = SourceUseOwnership::Direct;
  // One-based traversal order in the translation unit's post-token stream.
  // Zero means that the semantic producer does not own a source span.
  std::size_t source_traversal_order = 0;
  // A call whose callable expression is itself a call is visited before that
  // nested callee by the source AST traversal, even when both share the first
  // token location.
  bool source_call_precedes_nested_callee = false;
  // Stable semantic identity used only to order a member call before the
  // class-template owner named by the same source occurrence.
  semantic_model::ClassTemplateDecl * semantic_class_template_identity =
      nullptr;
  std::string semantic_class_specialization_key;
  semantic_model::ClassTemplateDecl *
      semantic_owner_class_template_identity = nullptr;
  std::string semantic_owner_class_specialization_key;

  std::string location;
  std::string selected_decl_anchor_location;
  SourceTemplateIdOccurrence template_id_occurrence;

  std::string selected_entity_decl_location;
  std::string template_name;
  std::string selected;
  SourceSelectionKind selection = SourceSelectionKind::None;

  std::vector<SourceBinding> bindings;
  std::vector<SourceBinding> specialization_bindings;
  std::vector<SourceDrop> drops;

  int candidate_count = -1;
  int candidates_built = -1;
  int candidates_viable = -1;
};

struct SemanticSourceUseTable
{
  std::vector<SemanticSourceUse> uses;
};

inline bool operator==(const SourceTemplateArgumentOccurrence & lhs,
                       const SourceTemplateArgumentOccurrence & rhs)
{
  return lhs.text == rhs.text &&
         lhs.semantic_text == rhs.semantic_text &&
         lhs.source_spelled == rhs.source_spelled &&
         lhs.dependent == rhs.dependent &&
         lhs.current_specialization == rhs.current_specialization &&
         lhs.preserve_qualified_member == rhs.preserve_qualified_member &&
         lhs.referenced_value_initializer_uses_template ==
             rhs.referenced_value_initializer_uses_template &&
         lhs.referenced_value_entities == rhs.referenced_value_entities &&
         lhs.referenced_value_decl_locations ==
             rhs.referenced_value_decl_locations;
}

inline bool operator==(const SourceTemplateIdOccurrence & lhs,
                       const SourceTemplateIdOccurrence & rhs)
{
  return lhs.present == rhs.present &&
         lhs.source_spelled == rhs.source_spelled &&
         lhs.argument_list_spelled == rhs.argument_list_spelled &&
         lhs.empty_argument_list == rhs.empty_argument_list &&
         lhs.in_template_body == rhs.in_template_body &&
         lhs.synthesized == rhs.synthesized &&
         lhs.exact_source_arguments == rhs.exact_source_arguments &&
         lhs.conversion_result_type_use == rhs.conversion_result_type_use &&
         lhs.function_result_type_use == rhs.function_result_type_use &&
         lhs.current_specialization_use == rhs.current_specialization_use &&
         lhs.has_dependent_argument == rhs.has_dependent_argument &&
         lhs.has_current_specialization_argument ==
             rhs.has_current_specialization_argument &&
         lhs.name_location == rhs.name_location &&
         lhs.arguments == rhs.arguments;
}

inline bool operator==(const SourceBinding & lhs, const SourceBinding & rhs)
{
  return lhs.param == rhs.param &&
         lhs.arg == rhs.arg &&
         lhs.source == rhs.source &&
         lhs.type_like == rhs.type_like &&
         lhs.function_type_argument == rhs.function_type_argument &&
         lhs.structured_type_spelling == rhs.structured_type_spelling &&
         lhs.preserve_qualified_member == rhs.preserve_qualified_member &&
         lhs.pack_binding == rhs.pack_binding &&
         lhs.pack_aggregate == rhs.pack_aggregate &&
         lhs.pack_arguments == rhs.pack_arguments &&
         lhs.function_pointer_parameter == rhs.function_pointer_parameter;
}

inline bool operator==(const SourceDrop & lhs, const SourceDrop & rhs)
{
  return lhs.candidate == rhs.candidate &&
         lhs.location == rhs.location &&
         lhs.reason == rhs.reason;
}

inline bool operator<(const SourceDrop & lhs, const SourceDrop & rhs)
{
  if(lhs.candidate != rhs.candidate) return lhs.candidate < rhs.candidate;
  if(lhs.location != rhs.location) return lhs.location < rhs.location;
  return lhs.reason < rhs.reason;
}

inline bool operator==(const SemanticSourceUse & lhs,
                       const SemanticSourceUse & rhs)
{
  return lhs.kind == rhs.kind &&
         lhs.role == rhs.role &&
         lhs.ownership == rhs.ownership &&
         lhs.location == rhs.location &&
         lhs.selected_decl_anchor_location ==
             rhs.selected_decl_anchor_location &&
         lhs.template_id_occurrence == rhs.template_id_occurrence &&
         lhs.selected_entity_decl_location ==
             rhs.selected_entity_decl_location &&
         lhs.template_name == rhs.template_name &&
         lhs.selected == rhs.selected &&
         lhs.selection == rhs.selection &&
         lhs.bindings == rhs.bindings &&
         lhs.specialization_bindings == rhs.specialization_bindings &&
         lhs.drops == rhs.drops &&
         lhs.candidate_count == rhs.candidate_count &&
         lhs.candidates_built == rhs.candidates_built &&
         lhs.candidates_viable == rhs.candidates_viable;
}

inline std::string source_use_arg_compact_key(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(std::isspace(static_cast<unsigned char>(text[i]))) {
      continue;
    }
    out.push_back(text[i]);
  }
  return out;
}

inline bool source_template_id_occurrence_has_data(
    const SourceTemplateIdOccurrence & occurrence)
{
  return occurrence.present ||
         occurrence.source_spelled ||
         occurrence.argument_list_spelled ||
         occurrence.empty_argument_list ||
         occurrence.in_template_body ||
         occurrence.synthesized ||
         occurrence.exact_source_arguments ||
         occurrence.conversion_result_type_use ||
         occurrence.function_result_type_use ||
         occurrence.current_specialization_use ||
         occurrence.has_dependent_argument ||
         occurrence.has_current_specialization_argument ||
         !occurrence.name_location.empty() ||
         !occurrence.arguments.empty();
}

inline bool source_template_id_occurrence_has_richer_semantic_text(
    const SourceTemplateIdOccurrence & candidate,
    const SourceTemplateIdOccurrence & existing)
{
  if(candidate.arguments.size() != existing.arguments.size()) {
    return false;
  }
  bool fills_missing_text = false;
  for(std::size_t i = 0; i < candidate.arguments.size(); ++i) {
    if(candidate.arguments[i].text != existing.arguments[i].text) {
      return false;
    }
    if(!existing.arguments[i].semantic_text.empty() &&
       candidate.arguments[i].semantic_text.empty()) {
      return false;
    }
    if(existing.arguments[i].semantic_text.empty() &&
       !candidate.arguments[i].semantic_text.empty()) {
      fills_missing_text = true;
    }
  }
  return fills_missing_text;
}

inline bool source_template_id_occurrence_is_more_concrete(
    const SourceTemplateIdOccurrence & candidate,
    const SourceTemplateIdOccurrence & existing)
{
  if(!source_template_id_occurrence_has_data(candidate)) {
    return false;
  }
  if(!source_template_id_occurrence_has_data(existing)) {
    return true;
  }
  if(candidate.present != existing.present ||
     candidate.source_spelled != existing.source_spelled ||
     candidate.argument_list_spelled != existing.argument_list_spelled ||
     candidate.in_template_body != existing.in_template_body ||
     candidate.arguments.size() != existing.arguments.size()) {
    return false;
  }
  if(existing.exact_source_arguments && !candidate.exact_source_arguments) {
    return false;
  }
  if(candidate.exact_source_arguments && !existing.exact_source_arguments) {
    return true;
  }
  const bool existing_result_type_use =
      existing.conversion_result_type_use ||
      existing.function_result_type_use;
  const bool candidate_result_type_use =
      candidate.conversion_result_type_use ||
      candidate.function_result_type_use;
  if(existing_result_type_use && !candidate_result_type_use) {
    return false;
  }
  if(candidate_result_type_use && !existing_result_type_use) {
    return true;
  }
  if(existing.current_specialization_use &&
     !candidate.current_specialization_use) {
    return false;
  }
  if(candidate.current_specialization_use &&
     !existing.current_specialization_use) {
    return true;
  }
  if(source_template_id_occurrence_has_richer_semantic_text(candidate,
                                                            existing)) {
    return true;
  }
  if(existing.has_current_specialization_argument &&
     !candidate.has_current_specialization_argument) {
    bool same_source_text = true;
    for(std::size_t i = 0; i < candidate.arguments.size(); ++i) {
      if(candidate.arguments[i].text != existing.arguments[i].text) {
        same_source_text = false;
        break;
      }
    }
    if(same_source_text) {
      return false;
    }
  }
  const bool candidate_concrete =
      !candidate.has_dependent_argument &&
      !candidate.has_current_specialization_argument;
  const bool existing_concrete =
      !existing.has_dependent_argument &&
      !existing.has_current_specialization_argument;
  return candidate_concrete && !existing_concrete;
}

inline bool source_bindings_equivalent_ignoring_space(
    const std::vector<SourceBinding> & lhs,
    const std::vector<SourceBinding> & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs.size(); ++i) {
    if(lhs[i].param != rhs[i].param ||
       lhs[i].source != rhs[i].source ||
       lhs[i].type_like != rhs[i].type_like ||
       lhs[i].function_type_argument != rhs[i].function_type_argument ||
       lhs[i].structured_type_spelling != rhs[i].structured_type_spelling ||
       lhs[i].preserve_qualified_member != rhs[i].preserve_qualified_member ||
       lhs[i].pack_aggregate != rhs[i].pack_aggregate ||
       lhs[i].function_pointer_parameter != rhs[i].function_pointer_parameter ||
       source_use_arg_compact_key(lhs[i].arg) !=
           source_use_arg_compact_key(rhs[i].arg)) {
      return false;
    }
  }
  return true;
}

inline bool function_call_equivalent_ignoring_binding_spacing(
    const SemanticSourceUse & lhs,
    const SemanticSourceUse & rhs)
{
  return lhs.kind == SourceUseKind::FunctionCall &&
         rhs.kind == SourceUseKind::FunctionCall &&
         lhs.role == rhs.role &&
         lhs.ownership == rhs.ownership &&
         lhs.location == rhs.location &&
         lhs.selected_decl_anchor_location ==
             rhs.selected_decl_anchor_location &&
         lhs.template_id_occurrence == rhs.template_id_occurrence &&
         lhs.selected_entity_decl_location ==
             rhs.selected_entity_decl_location &&
         lhs.template_name == rhs.template_name &&
         lhs.selected == rhs.selected &&
         lhs.selection == rhs.selection &&
         source_bindings_equivalent_ignoring_space(lhs.bindings, rhs.bindings) &&
         source_bindings_equivalent_ignoring_space(lhs.specialization_bindings,
                                                   rhs.specialization_bindings) &&
         lhs.drops == rhs.drops &&
         lhs.candidate_count == rhs.candidate_count &&
         lhs.candidates_built == rhs.candidates_built &&
         lhs.candidates_viable == rhs.candidates_viable;
}

inline std::string function_call_effective_location(
    const SemanticSourceUse & use)
{
  return use.location;
}

inline std::string function_call_selected_decl_location(
    const SemanticSourceUse & use)
{
  return use.selected_decl_anchor_location.empty() ?
      use.selected_entity_decl_location : use.selected_decl_anchor_location;
}

inline bool function_calls_share_selected_source_identity(
    const SemanticSourceUse & lhs,
    const SemanticSourceUse & rhs)
{
  return lhs.kind == SourceUseKind::FunctionCall &&
         rhs.kind == SourceUseKind::FunctionCall &&
         lhs.role == rhs.role &&
         lhs.ownership == rhs.ownership &&
         function_call_effective_location(lhs) ==
             function_call_effective_location(rhs) &&
         function_call_selected_decl_location(lhs) ==
             function_call_selected_decl_location(rhs) &&
         lhs.template_name == rhs.template_name &&
         lhs.selected == rhs.selected &&
         lhs.selection == rhs.selection &&
         source_bindings_equivalent_ignoring_space(lhs.bindings, rhs.bindings) &&
         source_bindings_equivalent_ignoring_space(lhs.specialization_bindings,
                                                   rhs.specialization_bindings);
}

inline bool source_drops_contain(const std::vector<SourceDrop> & superset,
                                 const std::vector<SourceDrop> & subset)
{
  for(std::size_t i = 0; i < subset.size(); ++i) {
    bool found = false;
    for(std::size_t j = 0; j < superset.size(); ++j) {
      if(superset[j] == subset[i]) {
        found = true;
        break;
      }
    }
    if(!found) {
      return false;
    }
  }
  return true;
}

inline bool function_call_candidate_facts_dominate(
    const SemanticSourceUse & candidate,
    const SemanticSourceUse & existing)
{
  if(!source_drops_contain(candidate.drops, existing.drops)) {
    return false;
  }
  const int candidate_metrics[] = {
      candidate.candidate_count,
      candidate.candidates_built,
      candidate.candidates_viable,
  };
  const int existing_metrics[] = {
      existing.candidate_count,
      existing.candidates_built,
      existing.candidates_viable,
  };
  for(std::size_t i = 0; i < 3; ++i) {
    if(existing_metrics[i] >= 0 &&
       candidate_metrics[i] != existing_metrics[i]) {
      return false;
    }
  }
  return true;
}

inline bool function_call_binding_prefix_matches(
    const std::vector<SourceBinding> & prefix,
    const std::vector<SourceBinding> & full)
{
  if(prefix.size() >= full.size()) {
    return false;
  }
  for(std::size_t i = 0; i < prefix.size(); ++i) {
    if(prefix[i].param != full[i].param ||
       prefix[i].source != full[i].source ||
       source_use_arg_compact_key(prefix[i].arg) !=
           source_use_arg_compact_key(full[i].arg)) {
      return false;
    }
  }
  for(std::size_t i = prefix.size(); i < full.size(); ++i) {
    if(full[i].source != "deduced") {
      return false;
    }
  }
  return true;
}

inline bool function_call_is_deduced_trailing_binding_replay(
    const SemanticSourceUse & shorter,
    const SemanticSourceUse & longer)
{
  const std::string shorter_target =
      shorter.selected.empty() ? shorter.template_name : shorter.selected;
  const std::string longer_target =
      longer.selected.empty() ? longer.template_name : longer.selected;
  const std::string shorter_decl =
      function_call_selected_decl_location(shorter);
  const std::string longer_decl =
      function_call_selected_decl_location(longer);
  return shorter.kind == SourceUseKind::FunctionCall &&
         longer.kind == SourceUseKind::FunctionCall &&
         function_call_effective_location(shorter) ==
             function_call_effective_location(longer) &&
         shorter_target == longer_target &&
         !shorter_decl.empty() &&
         !longer_decl.empty() &&
         shorter_decl != longer_decl &&
         function_call_binding_prefix_matches(shorter.bindings,
                                              longer.bindings);
}

inline void merge_function_call_source_metadata(
    SemanticSourceUse & preferred,
    const SemanticSourceUse & other)
{
  if(preferred.source_traversal_order == 0 ||
     (other.source_traversal_order != 0 &&
      other.source_traversal_order < preferred.source_traversal_order)) {
    preferred.source_traversal_order = other.source_traversal_order;
  }
  preferred.source_call_precedes_nested_callee =
      preferred.source_call_precedes_nested_callee ||
      other.source_call_precedes_nested_callee;
  if(source_template_id_occurrence_is_more_concrete(
         other.template_id_occurrence,
         preferred.template_id_occurrence)) {
    preferred.template_id_occurrence = other.template_id_occurrence;
  }
}

inline bool alias_use_equivalent_ignoring_binding_spacing(
    const SemanticSourceUse & lhs,
    const SemanticSourceUse & rhs)
{
  return lhs.kind == SourceUseKind::AliasUse &&
         rhs.kind == SourceUseKind::AliasUse &&
         lhs.role == rhs.role &&
         lhs.ownership == rhs.ownership &&
         lhs.location == rhs.location &&
         lhs.selected_decl_anchor_location ==
             rhs.selected_decl_anchor_location &&
         lhs.selected_entity_decl_location ==
             rhs.selected_entity_decl_location &&
         lhs.template_name == rhs.template_name &&
         lhs.selected == rhs.selected &&
         lhs.selection == rhs.selection &&
         source_bindings_equivalent_ignoring_space(lhs.bindings, rhs.bindings) &&
         source_bindings_equivalent_ignoring_space(lhs.specialization_bindings,
                                                   rhs.specialization_bindings);
}

inline bool class_use_equivalent_ignoring_binding_spacing(
    const SemanticSourceUse & lhs,
    const SemanticSourceUse & rhs)
{
  return lhs.kind == SourceUseKind::ClassUse &&
         rhs.kind == SourceUseKind::ClassUse &&
         lhs.role == rhs.role &&
         lhs.location == rhs.location &&
         lhs.selected_decl_anchor_location ==
             rhs.selected_decl_anchor_location &&
         lhs.selected_entity_decl_location ==
             rhs.selected_entity_decl_location &&
         lhs.template_name == rhs.template_name &&
         lhs.selected == rhs.selected &&
         lhs.selection == rhs.selection &&
         source_bindings_equivalent_ignoring_space(lhs.bindings, rhs.bindings) &&
         source_bindings_equivalent_ignoring_space(lhs.specialization_bindings,
                                                   rhs.specialization_bindings);
}

inline bool variable_use_equivalent_ignoring_location(
    const SemanticSourceUse & lhs,
    const SemanticSourceUse & rhs)
{
  return lhs.kind == SourceUseKind::VariableUse &&
         rhs.kind == SourceUseKind::VariableUse &&
         lhs.role == rhs.role &&
         lhs.ownership == rhs.ownership &&
         lhs.selected_decl_anchor_location ==
             rhs.selected_decl_anchor_location &&
         lhs.selected_entity_decl_location ==
             rhs.selected_entity_decl_location &&
         lhs.template_name == rhs.template_name &&
         lhs.selection == rhs.selection &&
         source_bindings_equivalent_ignoring_space(lhs.bindings, rhs.bindings) &&
         source_bindings_equivalent_ignoring_space(lhs.specialization_bindings,
                                                   rhs.specialization_bindings);
}

inline void record_source_use(SemanticSourceUseTable & table,
                              const SemanticSourceUse & use)
{
  if(use.kind == SourceUseKind::AliasUse) {
    table.uses.push_back(use);
    return;
  }
  if(use.kind == SourceUseKind::FunctionCall) {
    for(std::size_t i = 0; i < table.uses.size(); ++i) {
      if(function_call_equivalent_ignoring_binding_spacing(table.uses[i], use)) {
        merge_function_call_source_metadata(table.uses[i], use);
        return;
      }
    }
    for(std::size_t i = 0; i < table.uses.size(); ++i) {
      if(!function_calls_share_selected_source_identity(table.uses[i], use)) {
        continue;
      }
      const bool use_dominates =
          function_call_candidate_facts_dominate(use, table.uses[i]);
      const bool existing_dominates =
          function_call_candidate_facts_dominate(table.uses[i], use);
      if(use_dominates && !existing_dominates) {
        SemanticSourceUse replacement = use;
        merge_function_call_source_metadata(replacement, table.uses[i]);
        table.uses[i] = replacement;
        return;
      }
      if(existing_dominates) {
        merge_function_call_source_metadata(table.uses[i], use);
        return;
      }
    }
    for(std::size_t i = 0; i < table.uses.size(); ++i) {
      if(function_call_is_deduced_trailing_binding_replay(table.uses[i], use)) {
        merge_function_call_source_metadata(table.uses[i], use);
        return;
      }
      if(function_call_is_deduced_trailing_binding_replay(use, table.uses[i])) {
        SemanticSourceUse replacement = use;
        merge_function_call_source_metadata(replacement, table.uses[i]);
        table.uses[i] = replacement;
        return;
      }
    }
  }
  for(std::size_t i = 0; i < table.uses.size(); ++i) {
    if(table.uses[i] == use) {
      if(table.uses[i].source_traversal_order == 0) {
        table.uses[i].source_traversal_order = use.source_traversal_order;
      }
      return;
    }
  }
  table.uses.push_back(use);
}

}  // namespace semantic_source_use
