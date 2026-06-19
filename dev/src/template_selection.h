#pragma once

#include <map>
#include <string>
#include <vector>

#include "semantic_model.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

namespace template_selection {

enum MatchKind
{
  MS_PRIMARY,
  MS_EXPLICIT_SPECIALIZATION,
  MS_PARTIAL_SPECIALIZATION
};

struct ClassSpecializationSelection
{
  const CppAstNode * class_node = nullptr;
  semantic_model::Scope * binding_scope = nullptr;
  const std::vector<template_model::TemplateParameterInfo> * parameters = nullptr;
  std::vector<template_model::TemplateArgument> arguments;
  const std::vector<cpp_decl::TemplateArgumentSyntax> * argument_syntaxes = nullptr;
  std::map<std::string, std::size_t> pack_sizes;
  std::string selection_key;
  MatchKind kind = MS_PRIMARY;
  bool reentrant_primary = false;
};

struct VariableSpecializationSelection
{
  semantic_model::Scope * binding_scope = nullptr;
  const std::vector<template_model::TemplateParameterInfo> * parameters = nullptr;
  std::vector<template_model::TemplateArgument> arguments;
  std::map<std::string, std::size_t> pack_sizes;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * initializer = nullptr;
  std::string selection_key;
  MatchKind kind = MS_PRIMARY;
};

ClassSpecializationSelection select_class_specialization(
    template_api::TemplateServices & services,
    semantic_model::ClassTemplateDecl & decl,
    template_api::TemplateEnvironmentHandle use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts = nullptr);

VariableSpecializationSelection select_variable_specialization(
    template_api::TemplateServices & services,
    semantic_model::VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments);

}  // namespace template_selection
