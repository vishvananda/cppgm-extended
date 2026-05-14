#pragma once

#include "semantic_conversion.h"
#include "template_api.h"
#include "template_environment.h"
#include "template_service_interfaces.h"

namespace template_api {

bool resolve_template_template_argument_text(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text,
    std::size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    template_model::TemplateArgument & out);

bool resolve_template_template_argument_text(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::string & text,
    std::size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    template_model::TemplateArgument & out);

NonTypeArgumentStatus evaluate_non_type_argument_text(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

NonTypeArgumentStatus evaluate_non_type_argument_text(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::string & text,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

NonTypeArgumentStatus evaluate_non_type_argument_expression(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

std::string lookup_text_for_type_argument(SemanticContext & ctx,
                                          const cpp_decl::TypePtr & type);

bool substitute_type(const cpp_decl::TypePtr & type,
                     const std::vector<template_model::TemplateParameterInfo> & parameters,
                     const std::vector<template_model::TemplateArgument> & arguments,
                     cpp_decl::TypePtr & out);

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);

// template-boundary-audit: end text_recovery_bridge

bool resolve_non_type_template_parameter_type(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out);

bool resolve_non_type_template_parameter_type(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out);

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(TemplateServices & services,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(TemplateServices & services,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(TemplateServices & services,
                               TemplateEnvironmentHandle argument_scope,
                               TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(TemplateServices & services,
                               TemplateEnvironmentHandle argument_scope,
                               TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out);

bool resolve_template_arguments(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope = nullptr);

bool resolve_template_arguments(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope = nullptr);

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope = TemplateEnvironmentHandle());

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope = TemplateEnvironmentHandle());

bool deduce_template_argument(TemplateServices & services,
                              const std::vector<template_model::TemplateParameterInfo> & parameters,
                              const cpp_decl::TypePtr & pattern,
                              const cpp_decl::TypePtr & actual,
                              std::map<std::string, cpp_decl::TypePtr> & deduced,
                              TemplateEnvironmentHandle deduction_scope = TemplateEnvironmentHandle(),
                              bool partial_top_level_cv_deduction = false,
                              TemplateEnvironmentHandle actual_lookup_scope =
                                  TemplateEnvironmentHandle(),
                              bool allow_actual_base_deduction = true);

bool trailing_pack_accepts_argument_count(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t argument_count);

bool deduce_template_argument(SemanticContext & ctx,
                              const std::vector<template_model::TemplateParameterInfo> & parameters,
                              const cpp_decl::TypePtr & pattern,
                              const cpp_decl::TypePtr & actual,
                              std::map<std::string, cpp_decl::TypePtr> & deduced,
                              semantic_model::Scope * deduction_scope = nullptr,
                              bool partial_top_level_cv_deduction = false,
                              semantic_model::Scope * actual_lookup_scope = nullptr,
                              bool allow_actual_base_deduction = true);

bool deduce_function_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool resolve_function_explicit_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_arg_syntaxes = nullptr);

bool deduce_function_template_arguments_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count);

void overlay_instantiation_use_scope_bindings(semantic_model::Scope & target,
                                              const semantic_model::Scope & use_scope,
                                              const semantic_model::Scope * declaring_scope);

void overlay_instantiation_use_scope_bindings(
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::set<std::string> & excluded_names);

void overlay_instantiation_local_named_types(
    SemanticContext & ctx,
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names = nullptr);

void overlay_instantiation_local_named_types(
    TemplateServices & services,
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names = nullptr);

void bind_template_arguments_into_scope(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

void bind_template_arguments_into_scope(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::Scope & bind_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::Scope & bind_class_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

ClassSpecializationSelection select_class_specialization(
    TemplateServices & services,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts = nullptr);

ClassSpecializationSelection select_class_specialization(
    TemplateServices & services,
    semantic_model::ClassTemplateDecl & decl,
    TemplateEnvironmentHandle use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts = nullptr);

VariableSpecializationSelection select_variable_specialization(
    TemplateServices & services,
    semantic_model::VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments);

bool match_partial_class_specialization(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const semantic_model::PartialClassTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes = nullptr);

bool match_partial_variable_specialization(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const semantic_model::VariableTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes = nullptr);

std::string normalize_special_member_template_name(SemanticContext & ctx,
                                                   const std::string & name,
                                                   bool is_constructor,
                                                   bool is_destructor);

std::string normalize_special_member_template_name(TemplateServices & services,
                                                   const std::string & name,
                                                   bool is_constructor,
                                                   bool is_destructor);

void parse_function_template_parameter_clause(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
    std::vector<const CppAstNode *> & default_arguments);

ParsedFunctionTemplateSignature parse_function_template_signature(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator);

ParsedFunctionTemplateSignature parse_function_template_signature(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator);

}  // namespace template_api
