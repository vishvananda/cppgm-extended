#pragma once

#include <map>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "cpp_scope_lookup.h"
#include "cppast_ast.h"
#include "semantic_model.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

namespace template_specialization {

struct AliasSubstitutionFailure
{
  enum Kind
  {
    SF_NONE,
    SF_MISSING_NONDEPENDENT_QUALIFIED_MEMBER_TYPE
  };

  AliasSubstitutionFailure()
    : kind(SF_NONE),
      stable_for_reuse(false)
  {}

  void reset()
  {
    kind = SF_NONE;
    alias_name.clear();
    owner_type_key.clear();
    owner_type_display.clear();
    member_name.clear();
    stable_for_reuse = false;
  }

  bool active() const
  {
    return kind != SF_NONE;
  }

  Kind kind;
  std::string alias_name;
  std::string owner_type_key;
  std::string owner_type_display;
  std::string member_name;
  bool stable_for_reuse;
};

bool expand_alias_template_pattern_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const std::string & pattern_text,
    const cpp_decl::QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    std::string & expanded_text,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes = nullptr,
    template_api::TemplateEnvironmentHandle argument_scope =
        template_api::TemplateEnvironmentHandle(),
    AliasSubstitutionFailure * substitution_failure = nullptr);

bool expand_alias_template_pattern_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const cpp_decl::QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    cpp_decl::TypePtr & expanded_type,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes = nullptr,
    template_api::TemplateEnvironmentHandle argument_scope =
        template_api::TemplateEnvironmentHandle(),
    bool allow_dependent_expansion = false,
    AliasSubstitutionFailure * substitution_failure = nullptr);

bool match_partial_class_specialization(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const semantic_model::PartialClassTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes = nullptr);

bool match_partial_variable_specialization(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const semantic_model::VariableTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes = nullptr);

int compare_partial_class_specialization_preference(
    template_api::TemplateServices & services,
    const semantic_model::PartialClassTemplateSpecializationDecl & current,
    const semantic_model::PartialClassTemplateSpecializationDecl & best);

int compare_partial_variable_specialization_preference(
    template_api::TemplateServices & services,
    const semantic_model::VariableTemplateSpecializationDecl & current,
    const semantic_model::VariableTemplateSpecializationDecl & best);

}  // namespace template_specialization
