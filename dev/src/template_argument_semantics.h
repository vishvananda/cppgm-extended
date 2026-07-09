#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "semantic_source_use.h"
#include "template_environment.h"
#include "template_model.h"
#include "template_service_interfaces.h"

namespace template_api {
struct TemplateServices;
struct TemplateTypeSystem;
struct TemplateDependentTypeExprRequest;
}

class SemanticContext;

namespace template_argument_semantics {

enum NonTypeArgumentStatus
{
  NT_ARG_PARSE_FAILED,
  NT_ARG_DEPENDENT,
  NT_ARG_EVAL_FAILED,
  NT_ARG_EVALUATED
};

class ScopedDefaultTemplateArgumentEvaluation
{
public:
  ScopedDefaultTemplateArgumentEvaluation();
  ~ScopedDefaultTemplateArgumentEvaluation();

  ScopedDefaultTemplateArgumentEvaluation(
      const ScopedDefaultTemplateArgumentEvaluation &) = delete;
  ScopedDefaultTemplateArgumentEvaluation & operator=(
      const ScopedDefaultTemplateArgumentEvaluation &) = delete;
};

bool default_template_argument_evaluation_active();

class ScopedBaseSpecifierTypeLookup
{
public:
  explicit ScopedBaseSpecifierTypeLookup(
      const std::string & lookup_text,
      const semantic_model::ClassInfo * owner_class = nullptr);
  ~ScopedBaseSpecifierTypeLookup();

  ScopedBaseSpecifierTypeLookup(const ScopedBaseSpecifierTypeLookup &) = delete;
  ScopedBaseSpecifierTypeLookup & operator=(
      const ScopedBaseSpecifierTypeLookup &) = delete;

private:
  bool active_;
};

bool base_specifier_type_lookup_active();
bool base_specifier_type_lookup_suppresses_inherited_member_lookup(
    const semantic_model::ClassInfo * owner_class);

struct ExpandedTemplateArgumentInputs
{
  std::vector<std::string> texts;
  std::vector<cpp_decl::TypePtr> type_arguments;
  std::vector<const cpp_decl::TemplateArgumentSyntax *> syntaxes;
  std::vector<std::shared_ptr<cpp_decl::TemplateArgumentSyntax> > owned_syntaxes;

  const cpp_decl::TemplateArgumentSyntax * syntax_for(std::size_t index) const;
  cpp_decl::TypePtr type_for(std::size_t index) const;
};

bool resolve_template_template_argument_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text,
    std::size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    template_model::TemplateArgument & out);

bool resolve_template_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    std::size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    template_model::TemplateArgument & out);

bool substitute_type(const cpp_decl::TypePtr & type,
                     const std::vector<template_model::TemplateParameterInfo> & parameters,
                     const std::vector<template_model::TemplateArgument> & arguments,
                     cpp_decl::TypePtr & out);
bool substitute_type(semantic_model::Scope & scope,
                     const cpp_decl::TypePtr & type,
                     const std::vector<template_model::TemplateParameterInfo> & parameters,
                     const std::vector<template_model::TemplateArgument> & arguments,
                     cpp_decl::TypePtr & out);
bool substitute_expression_node_for_template_arguments(
    semantic_model::Scope & scope,
    const CppAstNode & node,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    CppAstNode & out);
bool substitute_type_id_node_for_template_arguments(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    CppAstNode & out);
bool substitute_type_id_node_for_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    CppAstNode & out);
bool substitute_named_type_parameters(
    const cpp_decl::TypePtr & type,
    const std::map<std::string, cpp_decl::TypePtr> & type_replacements,
    cpp_decl::TypePtr & out);

// template-boundary-audit: begin text_recovery_bridge
std::string lookup_text_for_type_argument(template_api::TemplateTypeSystem & type_system,
                                          const cpp_decl::TypePtr & type);
bool normalized_type_lookup_text_matches(const std::string & lhs,
                                         const std::string & rhs);
void canonicalize_simple_dependent_argument_texts(
    template_api::TemplateTypeSystem & type_system,
    std::vector<template_model::TemplateArgument> & arguments);
bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const cpp_decl::TypePtr & type);
bool resolve_instantiated_dependent_type(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);
bool resolve_instantiated_dependent_type_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    cpp_decl::TypePtr & type);

NonTypeArgumentStatus evaluate_non_type_argument_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

NonTypeArgumentStatus evaluate_non_type_argument_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

NonTypeArgumentStatus evaluate_structured_bool_condition_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    bool & out);

NonTypeArgumentStatus evaluate_structured_bool_template_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    bool & out);

NonTypeArgumentStatus evaluate_structured_bool_constant_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TypePtr & type,
    bool & out);

bool note_constant_value_member_instantiations_in_expression(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const CppAstNode & expr);

void note_structured_bool_value_members_in_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateArgument> & arguments);

void note_structured_bool_value_members_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax);
void note_structured_bool_value_member_for_type_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TypePtr & type);

void note_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const semantic_model::ClassInfo & info);

void append_structured_bool_value_dependencies_in_expression_ast(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    std::vector<template_model::TemplateValueDependency> & out);

void append_structured_bool_value_dependencies_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    std::vector<template_model::TemplateValueDependency> & out);

void append_non_bool_static_value_dependencies_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    const cpp_decl::TypePtr & bound_value_type,
    std::vector<template_model::TemplateValueDependency> & out);

void note_template_value_dependencies_for_witness(
    const std::vector<template_model::TemplateValueDependency> & dependencies);
void note_template_value_dependencies_for_witness(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateValueDependency> & dependencies);
bool collect_template_member_value_dependency_if_active(
    const template_model::TemplateValueDependency & dependency);

class ScopedTemplateMemberValueDependencyCollectionPause
{
public:
  explicit ScopedTemplateMemberValueDependencyCollectionPause(bool active = true);
  ~ScopedTemplateMemberValueDependencyCollectionPause();

private:
  bool active_;
};

NonTypeArgumentStatus evaluate_non_type_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    long long & value,
    std::string * eval_error = nullptr,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

NonTypeArgumentStatus evaluate_qualified_member_value_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax,
    long long & value,
    const cpp_decl::TypePtr & target_type = cpp_decl::TypePtr());

bool evaluate_constant_expression_leaf(template_api::TemplateServices & services,
                                       semantic_model::Scope & scope,
                                       const CppAstNode & node,
                                       constant_eval::ConstexprValue & out,
                                       const cpp_decl::TypePtr & target_type =
                                           cpp_decl::TypePtr());

bool lookup_type_member_constant_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TypePtr & type,
    const std::string & member_name,
    constant_eval::ConstexprValue & out);

bool structured_bool_constant_value_for_type(
    template_api::TemplateTypeSystem & type_system,
    const cpp_decl::TypePtr & type,
    bool & out);
bool structured_bool_constant_value_for_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TypePtr & type,
    bool & out);

bool structured_bool_constant_value_for_class_info(
    template_api::TemplateTypeSystem & type_system,
    const semantic_model::ClassInfo & info,
    bool & out);
bool structured_bool_constant_value_for_class_info(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const semantic_model::ClassInfo & info,
    bool & out);

NonTypeArgumentStatus evaluate_standard_invocable_variable_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    bool & out);

bool scope_has_template_placeholders(template_api::TemplateServices & services,
                                     template_api::TemplateEnvironmentHandle scope);

bool text_mentions_template_placeholders(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const std::string & text);

bool text_mentions_dependent_non_namespace_binding_names(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text);

bool ast_node_syntax_has_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node);

void compute_text_template_dependency_flags(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text,
    bool & mentions_template_placeholders,
    bool & mentions_dependent_non_namespace_bindings);

bool text_mentions_non_namespace_binding_names(
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text);

void canonicalize_alias_template_source_argument_texts(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::vector<std::string> & arg_texts);

void mark_alias_template_value_owner_argument_facts(
    template_api::TemplateServices & services,
    semantic_model::Scope * use_scope,
    const semantic_model::AliasTemplateDecl & alias_template,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes,
    semantic_source_use::SourceTemplateIdOccurrence & occurrence);

bool text_mentions_current_specialization_names(
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text);

semantic_model::ClassTemplateDecl * lookup_class_template(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & name);

semantic_model::AliasTemplateDecl * lookup_alias_template(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & name);

cpp_decl::TypePtr lookup_structured_type_node(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    const std::string & lookup_name,
    bool reference_class_templates_only,
    const std::string & source_location = std::string());

bool resolve_template_id_syntax_type(template_api::TemplateServices & services,
                                     semantic_model::Scope & scope,
                                     const cpp_decl::TemplateIdSyntax & syntax,
                                     bool reference_class_templates_only,
                                     const std::string & source_location,
                                     cpp_decl::TypePtr & out,
                                     template_api::TemplateEnvironmentHandle
                                         argument_scope =
                                             template_api::TemplateEnvironmentHandle(),
                                     template_api::ClassTemplateSourceUseMode source_use_mode =
                                         template_api::ClassTemplateSourceUseMode::EmitClassUse,
                                     bool allow_enclosing_current_specializations = true);

bool try_resolve_type_pack_element_template_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes,
    cpp_decl::TypePtr & out);

bool parse_decltype_or_typeof_text(template_api::TemplateServices & services,
                                   semantic_model::Scope & scope,
                                   const std::string & text,
                                   cpp_decl::TypePtr & out);

bool parse_decltype_or_typeof_node(template_api::TemplateServices & services,
                                   semantic_model::Scope & scope,
                                   const CppAstNode & node,
                                   cpp_decl::TypePtr & out);

bool evaluate_dependent_type_expression_leaf(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const template_api::TemplateDependentTypeExprRequest & request,
    cpp_decl::TypePtr & out);

bool resolve_type_argument_input(template_api::TemplateServices & services,
                                 template_api::TemplateEnvironmentHandle scope,
                                 const cpp_decl::TemplateArgumentSyntax * syntax,
                                 bool reference_class_templates_only,
                                 cpp_decl::TypePtr & out);

enum StandardMetaMemberTypeResolution
{
  STANDARD_META_MEMBER_NOT_APPLICABLE,
  STANDARD_META_MEMBER_RESOLVED,
  STANDARD_META_MEMBER_SUBSTITUTION_FAILURE
};

StandardMetaMemberTypeResolution try_resolve_standard_meta_member_type(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    semantic_model::Scope & argument_scope,
    const std::string & member_name,
    const cpp_decl::TemplateIdSyntax & qualifier_template_id,
    cpp_decl::TypePtr & out,
    const std::vector<semantic_model::Scope *> * argument_scopes = nullptr);

bool parse_type_id_node_for_templates(template_api::TemplateServices & services,
                                      semantic_model::Scope & scope,
                                      const CppAstNode & type_id,
                                      cpp_decl::TypePtr & out,
                                      bool reference_class_templates_only = false);

bool resolve_type_argument_expression_syntax(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const CppAstNode & expr,
    bool reference_class_templates_only,
    const std::string & source_location,
    cpp_decl::TypePtr & out);

cpp_decl::TypePtr lookup_exact_local_type_name(template_api::TemplateServices & services,
                                               semantic_model::Scope & scope,
                                               const std::string & name);

std::vector<std::string> expand_bound_type_pack_texts(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<std::string> & texts);

bool expand_bound_packs_in_type_id_node(template_api::TemplateServices & services,
                                        semantic_model::Scope & scope,
                                        const CppAstNode & node,
                                        CppAstNode & out);

std::vector<cpp_decl::TemplateArgumentSyntax> expand_type_pack_argument_syntaxes(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateArgumentSyntax & source_syntax,
    const std::vector<std::string> & expanded_texts);

std::vector<std::string> expand_bound_expression_pack_texts(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & text);

std::vector<std::string> rewrite_decltype_expression_pack_texts(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & text);

ExpandedTemplateArgumentInputs expand_template_argument_inputs(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes);

// template-boundary-audit: end text_recovery_bridge

}  // namespace template_argument_semantics
