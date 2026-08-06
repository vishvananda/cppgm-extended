#pragma once

#include <string>
#include <vector>

#include "cpp_decl_model.h"
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
  semantic_model::FunctionBinding * source_function = nullptr;
  template_api::ClassTemplateSourceUseMode source_use_mode =
      template_api::ClassTemplateSourceUseMode::EmitClassUse;
  bool dependent_arguments = false;

  bool valid() const
  {
    return origin && use_scope && arguments && selection;
  }
};

// Compact retained form for a dependent source occurrence. The canonical
// resolver owns the syntax and declarations; delayed observation only needs a
// value copy of the resolved arguments and selection while those owners live.
// Ordinary analysis does not allocate these records unless a witness session
// is active.
struct RetainedDependentClassTemplateId
{
  semantic_model::ClassTemplateDecl * origin = nullptr;
  semantic_model::ClassInfo * instance = nullptr;
  semantic_model::Scope * use_scope = nullptr;
  std::vector<template_model::TemplateArgument> arguments;
  template_api::ClassSpecializationSelection selection;
  bool valid = false;
};

// Non-owning result for one source alias-template-id. A dependent pattern may
// be valid before its arguments can be reduced to TemplateArgument values; in
// that case the canonical source syntax is the parameterized semantic result.
struct ResolvedAliasTemplateIdView
{
  semantic_model::AliasTemplateDecl * origin = nullptr;
  semantic_model::Scope * use_scope = nullptr;
  cpp_decl::TypePtr resolved_type;
  const std::vector<template_model::TemplateArgument> * arguments = nullptr;
  const std::vector<std::string> * source_argument_texts = nullptr;
  const std::vector<cpp_decl::TemplateArgumentSyntax> *
      source_argument_syntaxes = nullptr;
  const std::string * source_location = nullptr;
  bool dependent_pattern = false;

  bool valid() const
  {
    return origin && use_scope && source_argument_texts;
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

}  // namespace resolved_source_semantics
