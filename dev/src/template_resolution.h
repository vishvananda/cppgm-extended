#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

class SemanticContext;

namespace template_resolution {

bool resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out);

bool make_shallow_bound_alias_template_id_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateIdSyntax & syntax,
    cpp_decl::TypePtr & out);

bool expand_dependent_alias_pattern_for_partial_order(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const cpp_decl::TypePtr & pattern,
    cpp_decl::TypePtr & out);

bool function_parameter_is_nondeduced_type_context_for_partial_order(
    SemanticContext & ctx,
    const semantic_model::FunctionTemplateDecl & decl,
    std::size_t parameter_index,
    const cpp_decl::TypePtr & pattern);

bool resolve_template_argument(template_api::TemplateServices & services,
                               template_api::TemplateEnvironmentHandle argument_scope,
                               template_api::TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out);

bool resolve_template_argument(template_api::TemplateServices & services,
                               template_api::TemplateEnvironmentHandle argument_scope,
                               template_api::TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out);

bool resolve_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope =
        template_api::TemplateEnvironmentHandle());

bool resolve_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope =
        template_api::TemplateEnvironmentHandle());

bool refresh_dependent_defaulted_non_type_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::vector<template_model::TemplateArgument> & arguments,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope =
        template_api::TemplateEnvironmentHandle());

bool complete_template_arguments_with_default_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    std::vector<template_model::TemplateArgument> & out,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope =
        template_api::TemplateEnvironmentHandle());

bool trailing_pack_accepts_argument_count(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t argument_count);

bool deduce_template_argument(
    template_api::TemplateServices & services,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const cpp_decl::TypePtr & pattern,
    const cpp_decl::TypePtr & actual,
    std::map<std::string, cpp_decl::TypePtr> & deduced,
    std::map<std::string, long long> & deduced_values,
    std::map<std::string, std::vector<template_model::TemplateArgument> > &
        deduced_pack_arguments,
    template_api::TemplateEnvironmentHandle deduction_scope =
        template_api::TemplateEnvironmentHandle(),
    bool partial_top_level_cv_deduction = false,
    template_api::TemplateEnvironmentHandle actual_lookup_scope =
        template_api::TemplateEnvironmentHandle(),
    bool allow_actual_base_deduction = true);

bool deduce_template_argument(
    template_api::TemplateServices & services,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const cpp_decl::TypePtr & pattern,
    const cpp_decl::TypePtr & actual,
    std::map<std::string, cpp_decl::TypePtr> & deduced,
    std::map<std::string, long long> & deduced_values,
    template_api::TemplateEnvironmentHandle deduction_scope =
        template_api::TemplateEnvironmentHandle(),
    bool partial_top_level_cv_deduction = false,
    template_api::TemplateEnvironmentHandle actual_lookup_scope =
        template_api::TemplateEnvironmentHandle(),
    bool allow_actual_base_deduction = true);

bool deduce_template_argument(
    template_api::TemplateServices & services,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const cpp_decl::TypePtr & pattern,
    const cpp_decl::TypePtr & actual,
    std::map<std::string, cpp_decl::TypePtr> & deduced,
    template_api::TemplateEnvironmentHandle deduction_scope =
        template_api::TemplateEnvironmentHandle(),
    bool partial_top_level_cv_deduction = false,
    template_api::TemplateEnvironmentHandle actual_lookup_scope =
        template_api::TemplateEnvironmentHandle(),
    bool allow_actual_base_deduction = true);

void dump_template_resolution_cache_memory_census(std::ostream & out);

}  // namespace template_resolution
