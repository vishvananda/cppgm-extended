#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "template_api.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

namespace template_instantiation {

std::string template_argument_key_for_instantiation(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateArgument> & arguments);

std::vector<std::string> canonical_instantiation_arg_texts(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateArgument> & arguments);

std::string specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & arguments);

std::string display_specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & arguments);

// template-boundary-audit: begin canonical_key_metadata
bool owner_prefixed_instantiation_key_matches(
    const std::string & owner_prefixed_key,
    const std::string & materialized_key);

bool function_binding_matches_materialized_owner_template_identity(
    const semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key);

bool function_binding_matches_instantiation_identity(
    const semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key);

bool should_preserve_owner_prefixed_template_identity(
    const semantic_model::FunctionBinding & original,
    const semantic_model::FunctionBinding & materialized,
    bool types_equivalent);

std::string class_template_instance_key(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info);

void record_class_template_dependent_argument_texts(
    semantic_model::ClassInfo & info,
    const std::vector<std::string> & argument_texts);

void adopt_materialized_owner_template_identity(
    semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<template_model::TemplateArgument> * instantiation_arguments,
    semantic_model::Scope * declaration_scope);

void adopt_function_template_identity_from_materialized(
    semantic_model::FunctionBinding & target,
    const semantic_model::FunctionBinding & materialized);

void record_function_template_identity(
    semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<template_model::TemplateArgument> * instantiation_arguments);

bool record_class_template_instantiation_state(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool is_explicit_specialization,
    bool suppress_implicit_instantiation_definition,
    bool dependent_arguments,
    const std::vector<std::string> * dependent_argument_texts = nullptr,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * dependent_argument_syntaxes = nullptr,
    const std::vector<template_model::TemplateParameterInfo> *
        dependent_argument_mangle_parameters = nullptr);

using ClassTemplateCompletionPlan = template_api::ClassTemplateCompletionPlan;
using ClassTemplateUseInfo = template_api::ClassTemplateUseInfo;

bool refresh_forward_class_template_selection(SemanticContext & ctx,
                                              semantic_model::ClassInfo & info);

bool class_template_completion_has_owner_definition(
    const semantic_model::ClassInfo & info);

ClassTemplateCompletionPlan class_template_completion_plan(
    const semantic_model::ClassInfo & info);

bool apply_out_of_class_static_member_definitions_to_reference(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info);

void replay_witness_static_member_definition_if_needed(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding,
    const semantic_model::ClassInfo * owner_override = nullptr);

bool apply_out_of_class_member_function_abi_metadata(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info);

bool class_template_use_info_for_type(SemanticContext & ctx,
                                      semantic_model::Scope & scope,
                                      const cpp_decl::TypePtr & type,
                                      ClassTemplateUseInfo & out,
                                      bool select_specialization = true);

bool class_template_use_info_for_class(SemanticContext & ctx,
                                       semantic_model::Scope & scope,
                                       semantic_model::ClassInfo * info,
                                       ClassTemplateUseInfo & out,
                                       bool select_specialization = true);

bool template_id_matches_class_template_origin(
    const cpp_decl::QualifiedName & template_id,
    const ClassTemplateUseInfo & info);

void append_class_template_type_arguments(
    const semantic_model::ClassInfo * info,
    std::vector<cpp_decl::TypePtr> & out);

bool class_template_instantiation_depends_on_template_parameter(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info);

void record_function_template_argument_state(
    semantic_model::FunctionBinding & binding,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    bool mark_has_arguments);

void record_function_template_arguments_preserving_pack_sizes(
    semantic_model::FunctionBinding & binding,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool mark_has_arguments);
// template-boundary-audit: end canonical_key_metadata

semantic_model::ClassInfo * instantiate_class_template(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateArgument> & arguments);

semantic_model::ClassInfo * instantiate_selected_class_template(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization);

void finalize_nested_member_class_instantiation(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & owner_decl,
    semantic_model::ClassInfo & nested_info,
    const std::vector<template_model::TemplateArgument> & owner_arguments,
    bool emit_track_instantiation = true);

void overlay_instantiation_use_scope_bindings(semantic_model::Scope & target,
                                              const semantic_model::Scope & use_scope,
                                              const semantic_model::Scope * declaring_scope);

void overlay_instantiation_use_scope_bindings(
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::set<std::string> & excluded_names);

void overlay_instantiation_local_named_types(
    template_api::TemplateServices & services,
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names = nullptr);

void bind_template_arguments_into_scope(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr,
    semantic_model::ClassInfo * active_owner = nullptr);

}  // namespace template_instantiation
