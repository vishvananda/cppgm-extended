#include "semantic_template_variable.h"

#include "parser_trace.h"
#include "semantic_context.h"
#include "semantic_trace.h"
#include "template_api.h"
#include "witness_api.h"

namespace semantic_template_variable {

namespace {

std::string variable_template_source_use_location(SemanticContext & ctx,
                                                  const CppAstNode & source_node,
                                                  const std::string & template_name)
{
  if(!witness::source_capture_enabled(ctx.template_witness_context())) {
    return std::string();
  }

  std::string source_use_location =
      ctx.source_location_for_name_in_node(source_node, template_name);
  if(!semantic_trace::source_location_points_at_identifier(source_use_location,
                                                           template_name)) {
    source_use_location.clear();
  }

  const std::string trace_use_location = parser_trace::current_use_location();
  if(source_use_location.empty() &&
     semantic_trace::source_location_points_at_identifier(trace_use_location,
                                                          template_name)) {
    source_use_location = trace_use_location;
  }

  if(source_use_location.empty()) {
    const std::string node_location = ctx.source_location_for_node(source_node);
    if(semantic_trace::source_location_points_at_identifier(node_location,
                                                            template_name)) {
      source_use_location = node_location;
    }
  }
  return source_use_location;
}

}  // namespace

const semantic_model::ValueBinding * acquire_variable_template_binding(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::Scope & source_use_scope,
    const std::string & source_use_location)
{
  template_api::TemplateVariableInstantiationRequest request;
  request.decl = &decl;
  request.arguments = arguments;
  request.intent = template_api::TemplateInstantiationIntent::TrackInstantiation;
  request.source_use_scope = &source_use_scope;
  request.source_use_location = source_use_location;
  return template_api::acquire_variable_instantiation(ctx, request).value_binding;
}

const semantic_model::ValueBinding * acquire_variable_template_binding_for_source_use(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::Scope & source_use_scope,
    const CppAstNode & source_node,
    const std::string & template_name)
{
  return acquire_variable_template_binding(ctx,
                                           decl,
                                           arguments,
                                           source_use_scope,
                                           variable_template_source_use_location(ctx,
                                                                                 source_node,
                                                                                 template_name));
}

const semantic_model::ValueBinding *
acquire_variable_template_binding_for_template_id_source_use(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    semantic_model::Scope & source_use_scope,
    const CppAstNode & source_node,
    const cpp_decl::TemplateIdSyntax & template_id)
{
  std::vector<template_model::TemplateArgument> arguments;
  if(!template_api::resolve_template_arguments(ctx,
                                               source_use_scope,
                                               decl.parameters,
                                               template_id.arguments,
                                               &template_id.argument_syntaxes,
                                               arguments,
                                               decl.declaring_scope)) {
    return nullptr;
  }
  return acquire_variable_template_binding_for_source_use(ctx,
                                                          decl,
                                                          arguments,
                                                          source_use_scope,
                                                          source_node,
                                                          template_id.name.name);
}

}  // namespace semantic_template_variable
