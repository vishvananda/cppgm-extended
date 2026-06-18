#include "template_api.h"

#include <algorithm>

#include "template_api_internal.h"
#include "template_argument_semantics.h"
#include "template_services.h"
#include "template_scope.h"

namespace template_api {
namespace type {

namespace {

bool append_template_value_dependency(
    std::vector<template_model::TemplateValueDependency> & out,
    const template_model::TemplateValueDependency & dependency)
{
  if(dependency.entity.empty() || dependency.decl_location.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < out.size(); ++i) {
    if(out[i].entity == dependency.entity &&
       out[i].decl_location == dependency.decl_location) {
      out[i].entity_has_template_identity =
          out[i].entity_has_template_identity ||
          dependency.entity_has_template_identity;
      return false;
    }
  }
  out.push_back(dependency);
  return true;
}

void append_template_argument_value_dependencies(
    std::vector<template_model::TemplateValueDependency> & out,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    for(std::size_t j = 0; j < arguments[i].value_dependencies.size(); ++j) {
      append_template_value_dependency(out, arguments[i].value_dependencies[j]);
    }
  }
}

semantic_model::ClassTemplateDecl * lookup_class_template_for_syntax(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & syntax)
{
  if(syntax.name.name.empty()) {
    return nullptr;
  }
  const std::string qualified_name = template_api::qualified_name_text(syntax.name);
  semantic_model::ClassTemplateDecl * class_template =
      template_argument_semantics::lookup_class_template(services,
                                                         scope,
                                                         qualified_name);
  if(!class_template && qualified_name != syntax.name.name) {
    class_template =
        template_argument_semantics::lookup_class_template(services,
                                                           scope,
                                                           syntax.name.name);
  }
  return class_template;
}

semantic_model::AliasTemplateDecl * lookup_alias_template_for_syntax(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & syntax)
{
  if(syntax.name.name.empty()) {
    return nullptr;
  }
  const std::string qualified_name = template_api::qualified_name_text(syntax.name);
  semantic_model::AliasTemplateDecl * alias_template =
      template_argument_semantics::lookup_alias_template(services,
                                                         scope,
                                                         qualified_name);
  if(!alias_template && qualified_name != syntax.name.name) {
    alias_template =
        template_argument_semantics::lookup_alias_template(services,
                                                           scope,
                                                           syntax.name.name);
  }
  return alias_template;
}

void append_resolved_template_id_value_dependencies(
    template_api::TemplateServices & services,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & syntax,
    std::vector<template_model::TemplateValueDependency> & out)
{
  if(!services.semantic_context || syntax.name.name.empty()) {
    return;
  }

  if(semantic_model::ClassTemplateDecl * class_template =
         lookup_class_template_for_syntax(services, scope, syntax)) {
    std::vector<template_model::TemplateArgument> arguments;
    try {
      if(template_api::resolve_template_arguments(
             services,
             scope,
             class_template->parameters,
             syntax.arguments,
             &syntax.argument_syntaxes,
             arguments,
             class_template->declaring_scope)) {
        append_template_argument_value_dependencies(out, arguments);
        const std::size_t count =
            std::min(class_template->parameters.size(),
                     std::min(syntax.argument_syntaxes.size(), arguments.size()));
        for(std::size_t i = 0; i < count; ++i) {
          const template_model::TemplateParameterInfo & parameter =
              class_template->parameters[i];
          const template_model::TemplateArgument & argument = arguments[i];
          if(parameter.kind != template_model::TemplateParameterInfo::TP_NON_TYPE) {
            continue;
          }
          cpp_decl::TypePtr value_type =
              argument.type ? argument.type : parameter.value_type;
          template_argument_semantics::
              append_non_bool_static_value_dependencies_in_template_argument_syntax(
                  services,
                  template_api::make_template_environment(scope),
                  syntax.argument_syntaxes[i],
                  value_type,
                  out);
        }
      }
    } catch(...) {
    }
  }

  if(semantic_model::AliasTemplateDecl * alias_template =
         lookup_alias_template_for_syntax(services, scope, syntax)) {
    std::vector<template_model::TemplateArgument> arguments;
    try {
      if(template_api::resolve_template_arguments(
             services,
             scope,
             alias_template->parameters,
             syntax.arguments,
             &syntax.argument_syntaxes,
             arguments,
             alias_template->declaring_scope)) {
        append_template_argument_value_dependencies(out, arguments);
        const std::size_t count =
            std::min(alias_template->parameters.size(),
                     std::min(syntax.argument_syntaxes.size(), arguments.size()));
        for(std::size_t i = 0; i < count; ++i) {
          const template_model::TemplateParameterInfo & parameter =
              alias_template->parameters[i];
          const template_model::TemplateArgument & argument = arguments[i];
          if(parameter.kind != template_model::TemplateParameterInfo::TP_NON_TYPE) {
            continue;
          }
          cpp_decl::TypePtr value_type =
              argument.type ? argument.type : parameter.value_type;
          template_argument_semantics::
              append_non_bool_static_value_dependencies_in_template_argument_syntax(
                  services,
                  template_api::make_template_environment(scope),
                  syntax.argument_syntaxes[i],
                  value_type,
                  out);
        }
      }
    } catch(...) {
    }
  }
}

}  // namespace

// template-boundary-audit: begin text_recovery_bridge
std::string lookup_text_for_type_argument(SemanticContext & ctx,
                                          const cpp_decl::TypePtr & type)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return template_argument_semantics::lookup_text_for_type_argument(
            type_system, type);
      });
}

bool substitute_type(const cpp_decl::TypePtr & type,
                     const std::vector<template_model::TemplateParameterInfo> & parameters,
                     const std::vector<template_model::TemplateArgument> & arguments,
                     cpp_decl::TypePtr & out)
{
  return template_argument_semantics::substitute_type(type, parameters, arguments, out);
}

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::resolve_instantiated_dependent_type(
            services, template_api::make_template_environment(scope), type, out);
      });
}

bool resolve_type_argument_input(SemanticContext & ctx,
                                 semantic_model::Scope & scope,
                                 const cpp_decl::TemplateArgumentSyntax * syntax,
                                 bool reference_class_templates_only,
                                 cpp_decl::TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::resolve_type_argument_input(
            services,
            template_api::make_template_environment(scope),
            syntax,
            reference_class_templates_only,
            out);
      });
}

bool text_mentions_template_placeholders(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const std::string & text)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::text_mentions_template_placeholders(
            services, template_api::make_template_environment(scope), text);
      });
}

bool text_mentions_dependent_non_namespace_binding_names(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
            services, template_api::make_template_environment(scope), text);
      });
}

bool text_mentions_non_namespace_binding_names(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text)
{
  (void)ctx;
  return template_argument_semantics::text_mentions_non_namespace_binding_names(
      template_api::make_template_environment(scope), text);
}

bool text_mentions_current_specialization_names(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text)
{
  (void)ctx;
  return template_argument_semantics::text_mentions_current_specialization_names(
      template_api::make_template_environment(scope), text);
}

bool should_defer_unresolved_type_lookup(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const std::string & text)
{
  return text_mentions_template_placeholders(ctx, scope, text) ||
         text_mentions_dependent_non_namespace_binding_names(ctx, scope, text);
}

std::vector<std::string> expand_bound_type_pack_texts(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<std::string> & texts)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::expand_bound_type_pack_texts(
            services, scope, texts);
      });
}

std::vector<std::string> expand_bound_expression_pack_texts(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::expand_bound_expression_pack_texts(
            services, scope, text);
      });
}

std::vector<std::string> rewrite_decltype_expression_pack_texts(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & text)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::rewrite_decltype_expression_pack_texts(
            services, scope, text);
      });
}

bool parse_type_id_node_for_templates(SemanticContext & ctx,
                                      semantic_model::Scope & scope,
                                      const CppAstNode & type_id,
                                      cpp_decl::TypePtr & out,
                                      bool reference_class_templates_only)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::parse_type_id_node_for_templates(
            services, scope, type_id, out, reference_class_templates_only);
      });
}

bool parse_decltype_or_typeof_node(SemanticContext & ctx,
                                   semantic_model::Scope & scope,
                                   const CppAstNode & node,
                                   cpp_decl::TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::parse_decltype_or_typeof_node(
            services, scope, node, out);
      });
}

bool resolve_template_id_syntax_type(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const cpp_decl::TemplateIdSyntax & syntax,
    bool reference_class_templates_only,
    const std::string & source_location,
    cpp_decl::TypePtr & out,
    semantic_model::Scope * argument_scope,
    template_api::ClassTemplateSourceUseMode source_use_mode)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::resolve_template_id_syntax_type(
            services,
            scope,
            syntax,
            reference_class_templates_only,
            source_location,
            out,
            argument_scope ?
                template_api::make_template_environment(*argument_scope) :
                template_api::TemplateEnvironmentHandle(),
            source_use_mode);
      });
}
// template-boundary-audit: end text_recovery_bridge

bool scope_has_type_parameter_pack_name(const semantic_model::Scope & scope,
                                        const std::string & name)
{
  return template_scope::scope_has_type_parameter_pack_name(scope, name);
}

void append_structured_bool_value_dependencies_in_expression_ast(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    std::vector<template_model::TemplateValueDependency> & out)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_argument_semantics::
            append_structured_bool_value_dependencies_in_expression_ast(
                services,
                template_api::make_template_environment(scope),
                node,
                out);
      });
}

void append_base_template_value_dependencies(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & base_name,
    const cpp_decl::TemplateIdSyntax * base_template_syntax,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * base_arg_syntaxes,
    std::vector<template_model::TemplateValueDependency> & out)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        const template_api::TemplateEnvironmentHandle env =
            template_api::make_template_environment(scope);
        if(base_template_syntax) {
          append_resolved_template_id_value_dependencies(services,
                                                         scope,
                                                         *base_template_syntax,
                                                         out);
        }
        for(std::size_t i = 0; i < base_name.qualifier_template_id_syntaxes.size(); ++i) {
          if(const cpp_decl::TemplateIdSyntax * qualifier_template_id =
                 cppast_qualifier_template_id_syntax(base_name, i)) {
            append_resolved_template_id_value_dependencies(services,
                                                           scope,
                                                           *qualifier_template_id,
                                                           out);
          }
        }
        if(base_arg_syntaxes) {
          for(std::size_t i = 0; i < base_arg_syntaxes->size(); ++i) {
            template_argument_semantics::
                append_structured_bool_value_dependencies_in_template_argument_syntax(
                    services,
                    env,
                    (*base_arg_syntaxes)[i],
                    out);
          }
        }
      });
}

bool note_constant_value_member_instantiations_in_expression(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & expr)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_argument_semantics::note_constant_value_member_instantiations_in_expression(
            services,
            scope,
            expr);
      });
}

bool template_id_names_alias_template(SemanticContext & ctx,
                                      semantic_model::Scope & scope,
                                      const cpp_decl::TemplateIdSyntax & syntax)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return lookup_alias_template_for_syntax(services, scope, syntax) != nullptr;
      });
}

bool template_id_names_class_template(SemanticContext & ctx,
                                      semantic_model::Scope & scope,
                                      const cpp_decl::TemplateIdSyntax & syntax)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return lookup_class_template_for_syntax(services, scope, syntax) != nullptr;
      });
}

bool resolve_non_type_template_parameter_type(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_api::resolve_non_type_template_parameter_type(
            services, template_api::make_template_environment(scope), parameter, out);
      });
}

}  // namespace type
}  // namespace template_api
