#include "semantic_template_class.h"

#include <vector>

#include "callsemantic/template_source_utils.h"
#include "semantic_context.h"
#include "template_api.h"

namespace semantic_template_class {

void append_base_clause_template_value_dependencies(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & base_name,
    const cpp_decl::TemplateIdSyntax * base_template_syntax,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * base_arg_syntaxes,
    std::vector<template_model::TemplateValueDependency> & out)
{
  if(ctx.template_witness_context().session == nullptr) {
    return;
  }
  template_api::type::append_structured_bool_value_dependencies_in_expression_ast(
      ctx,
      scope,
      base_name,
      out);
  template_api::type::append_base_template_value_dependencies(ctx,
                                                              scope,
                                                              base_name,
                                                              base_template_syntax,
                                                              base_arg_syntaxes,
                                                              out);
}

bool note_constant_value_member_instantiations_in_expression(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & expr)
{
  if(ctx.template_witness_context().session == nullptr) {
    return false;
  }
  return template_api::type::note_constant_value_member_instantiations_in_expression(
      ctx,
      scope,
      expr);
}

void emit_constructor_initializer_template_id_source_use(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & initializer_id,
    const cpp_decl::TemplateIdSyntax & syntax,
    const std::string & use_location,
    const std::vector<std::string> & argument_locations)
{
  if(!witness::source_capture_enabled(ctx.template_witness_context())) {
    return;
  }
  const bool names_alias_template =
      template_api::type::template_id_names_alias_template(ctx, scope, syntax);
  const bool names_class_template =
      template_api::type::template_id_names_class_template(ctx, scope, syntax);
  if(!names_alias_template && !names_class_template) {
    return;
  }

  const template_api::ScopedTemplateArgumentSourceLocations
      argument_source_locations(syntax.arguments, argument_locations);
  const callsemantic::ScopedTemplateUseLocation use_location_guard(use_location);
  cpp_decl::TypePtr ignored;
  if(template_api::type::resolve_template_id_syntax_type(
         ctx,
         scope,
         syntax,
         true,
         use_location,
         ignored,
         &scope,
         template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly) &&
     names_class_template &&
     ignored) {
    ctx.record_class_use_for_resolved_type_node(scope,
                                                initializer_id,
                                                ignored,
                                                use_location,
                                                true);
  }
}

}  // namespace semantic_template_class
