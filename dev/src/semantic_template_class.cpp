#include "semantic_template_class.h"

#include <vector>

#include "callsemantic/template_source_utils.h"
#include "semantic_context.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"
#include "parser_trace.h"

namespace semantic_template_class {

namespace {

std::string class_template_source_use_name(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  std::string name =
      semantic_utils::strip_trailing_top_level_template_arguments(
          template_api::class_witness_output_qualified_name(ctx, info));
  if(name.empty() && info.source_template) {
    name = info.source_template->name;
  }
  return name;
}

bool source_location_points_at_identifier(SemanticContext & ctx,
                                          const std::string & location,
                                          const std::string & identifier)
{
  return template_api::template_witness_detail::
      source_location_points_at_identifier_token(
          ctx.template_witness_context(),
          location,
          identifier);
}

}  // namespace

void emit_instantiated_class_template_use_source(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info,
    const std::string & use_location,
    witness::SourceUseRole role)
{
  if(!witness::source_capture_enabled(ctx.template_witness_context()) ||
     !info.source_template ||
     info.instantiation_arguments.empty()) {
    return;
  }

  const std::string public_location =
      template_api::normalize_template_witness_source_location(use_location);
  if(public_location.empty() ||
     !witness::source_location_capture_enabled(ctx.template_witness_context(),
                                               public_location)) {
    return;
  }

  witness::ClassUseEmitRequest request;
  request.location = public_location;
  request.template_name = class_template_source_use_name(ctx, info);
  request.selection = info.is_explicit_specialization ?
      witness::SourceSelectionKind::ExplicitSpecialization :
      witness::SourceSelectionKind::Primary;
  const bool source_use_spells_template =
      source_location_points_at_identifier(ctx,
                                           public_location,
                                           info.source_template->name);
  if(source_use_spells_template) {
    request.use_anchor_present = true;
    request.use_anchor_location = public_location;
  }
  if(info.source_template->class_node) {
    const semantic_model::SourceDeclAnchorCache & class_anchor =
        semantic_trace::class_decl_anchor(ctx, &info);
    witness::set_selected_decl_anchor(request.selected_decl_location,
                                      request.selected_decl_anchor,
                                      class_anchor);
  }
  template_api::append_class_template_witness_bindings(ctx,
                                                       &info,
                                                       request.bindings);
  const std::vector<std::string> * source_args =
      source_use_spells_template ?
          template_api::current_template_id_source_arguments_ptr(
              public_location,
              info.source_template->name) :
          nullptr;
  if(source_use_spells_template && !source_args) {
    return;
  }
  if(source_args) {
    request.template_id_occurrence =
        witness::make_source_template_id_occurrence(public_location,
                                                    *source_args);
  }
  request.role = role;
  if(parser_trace::enabled("witness.emit")) {
    parser_trace::note("witness.emit",
                       public_location,
                       std::string("semantic-template-class emit template=") +
                           request.template_name);
  }
  CPPGM_SET_WITNESS_PRODUCER(
      request,
      witness::WitnessProducerSite::ClassSemanticTemplateClass);
  witness::emit_class_use(request);
}

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
