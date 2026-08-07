#pragma once

#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "semantic_source_use.h"
#include "template_api.h"
#include "template_model.h"

namespace semantic_model {
struct ClassInfo;
struct ClassTemplateDecl;
struct AliasTemplateDecl;
struct FunctionBinding;
struct Scope;
}

namespace resolved_source_semantics {

// Non-owning view of the semantic facts produced while resolving one source
// class-template-id. Retained source results copy compact handles from this
// view instead of owning another template argument graph.
struct ResolvedClassTemplateIdView
{
  semantic_model::ClassTemplateDecl * origin = nullptr;
  semantic_model::ClassInfo * instance = nullptr;
  semantic_model::Scope * use_scope = nullptr;
  const std::vector<template_model::TemplateArgument> * arguments = nullptr;
  const template_api::ClassSpecializationSelection * selection = nullptr;
  const std::vector<std::string> * source_argument_texts = nullptr;
  const std::vector<cpp_decl::TemplateArgumentSyntax> *
      source_argument_syntaxes = nullptr;
  const cpp_decl::TemplateIdSyntax * source_syntax = nullptr;
  const std::string * source_location = nullptr;
  const std::string * instantiation_key = nullptr;
  template_api::ClassTemplateSourceUseMode source_use_mode =
      template_api::ClassTemplateSourceUseMode::EmitClassUse;
  semantic_source_use::SourceUseOwnership source_ownership =
      semantic_source_use::SourceUseOwnership::SourceOwned;
  semantic_source_use::SourceUseRole source_role =
      semantic_source_use::SourceUseRole::TypeUse;
  bool dependent_arguments = false;
  bool nested_source_use = false;
  bool clear_template_id_occurrence = false;
  bool source_is_nested_template_argument = false;
  bool source_is_qualified_member_owner = false;
  bool source_is_conversion_result = false;

  bool valid() const
  {
    return origin && use_scope && arguments && selection;
  }
};

// Compact semantic child carried through a type alias. The selected instance
// owns its canonical arguments; retaining only these two stable declarations
// avoids copying another TypePtr graph for witness observation.
struct RetainedAliasClassUse
{
  semantic_model::ClassTemplateDecl * origin = nullptr;
  semantic_model::ClassInfo * instance = nullptr;

  bool valid() const
  {
    return origin && instance;
  }
};

// Stack-scoped semantic result for one source alias-template-id. A dependent
// pattern may be valid before its arguments can be reduced to TemplateArgument
// values; in that case the canonical source syntax is the parameterized
// result. All pointers refer to storage owned by the active resolution
// operation.
struct ResolvedAliasTemplateId
{
  semantic_model::AliasTemplateDecl * origin = nullptr;
  semantic_model::Scope * use_scope = nullptr;
  cpp_decl::TypePtr resolved_type;
  const std::vector<template_model::TemplateArgument> * arguments = nullptr;
  const std::vector<std::string> * source_argument_texts = nullptr;
  const std::vector<cpp_decl::TemplateArgumentSyntax> *
      source_argument_syntaxes = nullptr;
  const std::string * source_location = nullptr;
  const semantic_source_use::SourceTemplateIdOccurrence * source_occurrence =
      nullptr;
  witness::AliasUseEmissionOrigin emission_origin =
      witness::AliasUseEmissionOrigin::NestedSourceTemplateId;
  bool dependent_pattern = false;
  bool normalize_selected_decl_to_line_start = false;
  bool unwrap_single_pack_binding = false;
  bool use_template_argument_binding_policy = false;

  bool valid() const
  {
    return origin && use_scope && (source_argument_texts || arguments);
  }
};

// Non-owning result of one qualified-id lookup. The resolved owner type keeps
// the already-selected owner chain; callers that only need the selected
// entity do not allocate or reconstruct that chain.
struct ResolvedQualifiedId
{
  cpp_decl::TypePtr resolved_owner_type;
  const semantic_model::ValueBinding * selected_value = nullptr;
  semantic_model::FunctionBinding * selected_function = nullptr;
  const cpp_decl::TemplateIdSyntax * source_owner_syntax = nullptr;
};

// Semantic owner selected while binding an out-of-class member declaration.
// The source syntax is owned by the declaration AST. Delayed declarations
// retain this result through a witness-session side-store handle rather than
// copying template arguments or growing the class semantic graph.
struct ResolvedOwnerReference
{
  semantic_model::ClassInfo * owner = nullptr;
  semantic_model::Scope * source_scope = nullptr;
  const cpp_decl::TemplateIdSyntax * source_syntax = nullptr;
  const CppAstNode * source_anchor = nullptr;

  bool valid() const
  {
    return owner && source_scope && source_syntax;
  }
};

}  // namespace resolved_source_semantics
