#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

namespace template_resolution {

bool resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out);

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

bool trailing_pack_accepts_argument_count(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t argument_count);

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
