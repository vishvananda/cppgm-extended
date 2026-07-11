#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_context.h"
#include "semantic_lookup.h"
#include "semantic_model.h"
#include "symbol_linkage.h"
#include "template_api.h"
#include "template_model.h"
#include "witness_api.h"

namespace callsemantic {

enum class QualifiedOwnerClassResolution
{
  Complete,
  ReferenceMembers,
};

struct TemplateDeclarationCollectorState
{
  std::vector<std::unique_ptr<semantic_model::ClassTemplateDecl> > & class_templates;
  std::vector<std::unique_ptr<semantic_model::AliasTemplateDecl> > & alias_templates;
  std::vector<std::unique_ptr<semantic_model::FunctionTemplateDecl> > & function_templates;
  std::vector<std::unique_ptr<semantic_model::VariableTemplateDecl> > & variable_templates;
};

struct TemplateDeclarationSourceServices
{
  virtual ~TemplateDeclarationSourceServices() {}

  virtual std::string spaced_node_text(const CppAstNode & node) const = 0;
  virtual std::string source_location_for_name_in_subtree(
      const CppAstNode & node,
      const std::string & name,
      bool prefer_last) const = 0;
  virtual std::string earliest_qualified_use_location_for_prefix(
      const std::string & prefix) const = 0;
  virtual void emit_nested_class_use_source_events_from_location(
      semantic_model::Scope & scope,
      const std::string & location,
      witness::SourceUseOwnership ownership,
      const std::string & skip_exact_template_name) = 0;
  virtual void emit_out_of_class_owner_class_use_if_needed(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::string & qualified_name,
      const CppAstNode * anchor_node,
      semantic_model::ClassInfo * owner_override,
      const std::vector<template_model::TemplateParameterInfo> *
          canonical_parameters) = 0;
  virtual void emit_out_of_class_owner_class_use_if_needed(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const CppAstNode * anchor_node,
      semantic_model::ClassInfo * owner_override,
      const std::vector<template_model::TemplateParameterInfo> *
          canonical_parameters) = 0;
};

struct OutOfClassMemberResolutionServices
{
  virtual ~OutOfClassMemberResolutionServices() {}

  virtual semantic_model::ClassInfo * resolve_out_of_class_owner_class(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual semantic_model::ClassInfo * resolve_qualified_owner_class(
      semantic_model::Scope & scope,
      const std::string & owner_name,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual bool resolve_out_of_class_named_method_binding(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::string & member_name,
      const cpp_decl::TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier,
      semantic_model::FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual bool resolve_out_of_class_named_method_binding(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const std::string & member_name,
      const cpp_decl::TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier,
      semantic_model::FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual bool out_of_class_special_member_template_parameters_match(
      semantic_model::Scope & lhs_scope,
      const std::vector<template_model::TemplateParameterInfo> & lhs,
      semantic_model::Scope & rhs_scope,
      const std::vector<template_model::TemplateParameterInfo> & rhs) const = 0;
  virtual bool out_of_class_special_member_template_param_types_match(
      const cpp_decl::TypePtr & lhs,
      const std::vector<template_model::TemplateParameterInfo> & lhs_parameters,
      const cpp_decl::TypePtr & rhs,
      const std::vector<template_model::TemplateParameterInfo> & rhs_parameters)
      const = 0;
  virtual semantic_model::FunctionTemplateDecl *
  resolve_out_of_class_special_member_template(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual semantic_model::FunctionTemplateDecl *
  resolve_out_of_class_special_member_template(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual std::string describe_template_parameter_infos(
      const std::vector<template_model::TemplateParameterInfo> & parameters)
      const = 0;
  virtual std::string describe_out_of_class_special_member_template_lookup(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual std::string describe_out_of_class_special_member_template_lookup(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual std::string describe_out_of_class_special_member_binding_lookup(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual std::string describe_out_of_class_special_member_binding_lookup(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual semantic_model::FunctionTemplateDecl *
  resolve_out_of_class_method_template(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier) = 0;
  virtual semantic_model::FunctionTemplateDecl *
  resolve_out_of_class_method_template(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const std::string & member_name,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier) = 0;
  virtual bool resolve_out_of_class_method_binding_with_resolution(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      const cpp_decl::TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier,
      semantic_model::FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual bool resolve_out_of_class_method_binding_with_resolution(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      const cpp_decl::TypePtr & declared_type,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier,
      semantic_model::FunctionBinding *& out,
      QualifiedOwnerClassResolution resolution) = 0;
  virtual bool resolve_out_of_class_static_member_binding(
      semantic_model::Scope & scope,
      const cpp_decl::QualifiedName & qualified,
      semantic_model::ValueBinding *& out) = 0;
  virtual bool resolve_out_of_class_static_member_binding(
      semantic_model::Scope & scope,
      const std::string & qualified_name,
      semantic_model::ValueBinding *& out) = 0;
};

struct FunctionTemplateDeclarationPolicy
{
  virtual ~FunctionTemplateDeclarationPolicy() {}

  virtual bool function_template_entities_match(
      const semantic_model::FunctionTemplateDecl & existing,
      semantic_model::Scope & candidate_entity_scope,
      semantic_model::Scope & candidate_template_scope,
      const std::string & candidate_name,
      const std::vector<template_model::TemplateParameterInfo> &
          candidate_parameters,
      const cpp_decl::TypePtr & candidate_type,
      const CppAstNode & candidate_result_type_pattern,
      bool candidate_special_member_template,
      bool candidate_is_static_member,
      bool candidate_is_const_method,
      bool candidate_is_volatile_method,
      semantic_model::RefQualifier candidate_ref_qualifier,
      bool candidate_is_deleted) const = 0;
  virtual void inherit_pending_friend_function_template_access(
      semantic_model::FunctionTemplateDecl & decl) = 0;
  virtual bool explicit_function_nothrow_specifications_match(
      semantic_model::FunctionBinding & existing,
      const CppAstNode * qualifier) = 0;
  virtual bool declaration_marks_exclude_from_explicit_instantiation(
      const CppAstNode * declaration_node) const = 0;
  virtual symbol_linkage::SymbolLinkage function_symbol_linkage(
      semantic_model::Scope & scope,
      const CppAstNode * declaration_node,
      const CppAstNode * body,
      bool is_c_linkage,
      const CppAstNode * function_qualifier,
      const FunctionTemplateRegistrationIdentity & template_identity,
      bool is_defaulted,
      const semantic_model::ClassInfo * lexical_access_class,
      bool in_class_member_definition_context) const = 0;
  virtual void upgrade_function_symbol_linkage(
      semantic_model::FunctionBinding & binding,
      const std::string & qualified_name,
      const std::string & name,
      const cpp_decl::TypePtr & type,
      symbol_linkage::SymbolLinkage linkage) = 0;
  virtual void refresh_definition_parameter_names(
      semantic_model::FunctionBinding & binding,
      const semantic_model::FunctionBinding & source) = 0;
  virtual void refresh_definition_parameter_names(
      semantic_model::FunctionBinding & binding,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params) = 0;
  virtual CppAstNode filtered_function_declarator(
      const CppAstNode & declarator) const = 0;
  virtual void record_friend_function_template_declaration(
      semantic_model::ClassInfo & info,
      semantic_model::Scope & entity_scope,
      semantic_model::Scope & template_scope,
      const std::string & name,
      const std::vector<template_model::TemplateParameterInfo> &
          template_parameters,
      const cpp_decl::TypePtr & type_pattern,
      const std::vector<std::pair<std::string, cpp_decl::TypePtr> > &
          params_pattern,
      const CppAstNode & result_type_pattern,
      const std::vector<const CppAstNode *> & default_arguments_pattern,
      const std::vector<const CppAstNode *> & parameter_declarations_pattern,
      bool is_static_member,
      bool is_const_method,
      bool is_volatile_method,
      semantic_model::RefQualifier ref_qualifier,
      bool is_deleted,
      bool qualified_friend_name,
      const CppAstNode * specifiers,
      const CppAstNode * declarator,
      const CppAstNode * body,
      const CppAstNode * declaration_node) = 0;
};

struct TemplateDeclarationParsingServices
{
  virtual ~TemplateDeclarationParsingServices() {}

  virtual bool parse_template_parameters(
      const CppAstNode & clause,
      std::vector<template_model::TemplateParameterInfo> & out,
      semantic_model::Scope * placeholder_scope,
      std::string * failure_reason) = 0;
  virtual void collect_deduction_guide_declaration(
      semantic_model::Scope & scope,
      const CppAstNode & node,
      semantic_model::Scope * pattern_scope,
      const std::vector<template_model::TemplateParameterInfo> *
          template_parameters) = 0;
  virtual void record_template_parameter_clause_source_uses(
      semantic_model::Scope & scope,
      const CppAstNode & node) = 0;
  virtual void record_class_template_base_source_uses(
      const CppAstNode * class_node,
      semantic_model::Scope * pattern_scope) = 0;
  virtual bool fill_trailing_default_template_argument_texts(
      semantic_model::Scope & pattern_scope,
      const std::vector<template_model::TemplateParameterInfo> & parameters,
      const std::vector<std::string> & texts,
      semantic_model::Scope * default_argument_scope,
      std::vector<std::string> & out) = 0;
};

struct TemplateDeclarationCollectorServices
{
  TemplateDeclarationParsingServices * declaration_services = nullptr;
  TemplateDeclarationSourceServices * source_services = nullptr;
  OutOfClassMemberResolutionServices * out_of_class_services = nullptr;
  FunctionTemplateDeclarationPolicy * function_policy = nullptr;
};

void collect_template_declaration_impl(
    SemanticContext & ctx,
    TemplateDeclarationCollectorState & state,
    const TemplateDeclarationCollectorServices & services,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    semantic_model::MemberAccess access,
    const std::vector<template_model::TemplateParameterInfo> *
        inherited_template_parameters);

}  // namespace callsemantic
