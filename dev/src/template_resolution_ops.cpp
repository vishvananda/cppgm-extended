#include "template_api.h"

#include "template_api_internal.h"

namespace template_api {
namespace resolution {

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out)
{
  return template_api::resolve_template_argument(
      ctx, argument_scope, parameter_scope, parameter, text, syntax, out);
}

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out)
{
  return template_api::resolve_template_argument(
      ctx, argument_scope, parameter_scope, parameter, text, out);
}

bool trailing_pack_accepts_argument_count(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t argument_count)
{
  return template_api::trailing_pack_accepts_argument_count(parameters, argument_count);
}

bool deduce_template_argument(SemanticContext & ctx,
                              const std::vector<template_model::TemplateParameterInfo> & parameters,
                              const cpp_decl::TypePtr & pattern,
                              const cpp_decl::TypePtr & actual,
                              std::map<std::string, cpp_decl::TypePtr> & deduced,
                              semantic_model::Scope * deduction_scope,
                              bool partial_top_level_cv_deduction,
                              semantic_model::Scope * actual_lookup_scope)
{
  return template_api::deduce_template_argument(ctx,
                                                parameters,
                                                pattern,
                                                actual,
                                                deduced,
                                                deduction_scope,
                                                partial_top_level_cv_deduction,
                                                actual_lookup_scope);
}

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count)
{
  return template_api::explicit_function_template_arguments_determine_signature(
      ctx, decl, explicit_argument_count);
}

bool resolve_function_explicit_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_arg_syntaxes)
{
  return template_api::resolve_function_explicit_template_arguments(
      ctx, decl, resolution_scope, explicit_arg_texts, out, explicit_arg_syntaxes);
}

}  // namespace resolution
}  // namespace template_api
