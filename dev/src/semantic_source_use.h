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

inline bool operator<(const SourceDrop & lhs, const SourceDrop & rhs)
{
  if(lhs.candidate != rhs.candidate) return lhs.candidate < rhs.candidate;
  if(lhs.location != rhs.location) return lhs.location < rhs.location;
  return lhs.reason < rhs.reason;
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

inline void record_source_use(SemanticSourceUseTable & table,
                              const SemanticSourceUse & use)
{
  table.uses.push_back(use);
}

}  // namespace semantic_source_use
