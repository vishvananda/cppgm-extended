#include "semantic_template_function.h"

#include <sstream>

#include "semantic_context.h"
#include "template_api.h"

namespace semantic_template_function {

namespace {

void clear_deduction(FunctionTemplateDeduction & out)
{
  out.arguments.clear();
  out.pack_sizes.clear();
}

bool deduce_function_template(SemanticContext & ctx,
                              const template_api::TemplateFunctionDeductionRequest & request,
                              FunctionTemplateDeduction & out)
{
  clear_deduction(out);
  template_api::TemplateFunctionDeductionResult result;
  if(!template_api::deduce_function_template(ctx, request, result)) {
    return false;
  }
  out.arguments.swap(result.arguments);
  out.pack_sizes.swap(result.pack_sizes);
  return true;
}

bool build_partial_ordering_placeholder_arguments(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::vector<template_model::TemplateArgument> & out)
{
  out.clear();
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    template_model::TemplateArgument argument;
    std::ostringstream key;
    key << "partial-order "
        << (parameters[i].name.empty() ? std::string("<unnamed>") : parameters[i].name)
        << (parameters[i].parameter_pack ? std::string("...") : std::string())
        << "#" << i;
    if(parameters[i].kind == template_model::TemplateParameterInfo::TP_TYPE) {
      argument.kind = template_model::TemplateArgument::TA_TYPE;
      argument.partial_order_placeholder = true;
      argument.type =
          cpp_decl::make_named(
              std::string("typename ") +
                  (parameters[i].name.empty() ? std::string("<unnamed>") :
                                                parameters[i].name),
              key.str(),
              false);
      argument.text = key.str();
    } else if(parameters[i].kind == template_model::TemplateParameterInfo::TP_NON_TYPE) {
      argument.kind = template_model::TemplateArgument::TA_VALUE;
      argument.type = parameters[i].value_type;
      argument.text = key.str();
      argument.dependent = true;
      argument.partial_order_placeholder = true;
    } else if(parameters[i].kind ==
              template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      argument.kind = template_model::TemplateArgument::TA_CLASS_TEMPLATE;
      argument.text = key.str();
      argument.dependent = true;
      argument.partial_order_placeholder = true;
    } else {
      return false;
    }
    out.push_back(argument);
  }
  return true;
}

}  // namespace

bool deduce_function_template_from_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<semantic_conversion::ExprInfo> & args,
    semantic_model::Scope * use_scope,
    FunctionTemplateDeduction & out,
    semantic_model::Scope * resolution_scope,
    const std::vector<template_model::TemplateArgument> * explicit_arguments)
{
  template_api::TemplateFunctionDeductionRequest request;
  request.decl = &decl;
  request.args = &args;
  request.use_scope = use_scope;
  request.resolution_scope = resolution_scope;
  request.explicit_arguments = explicit_arguments;
  return deduce_function_template(ctx, request, out);
}

bool deduce_function_template_from_target_type(SemanticContext & ctx,
                                               semantic_model::FunctionTemplateDecl & decl,
                                               const cpp_decl::TypePtr & target_type,
                                               semantic_model::Scope * use_scope,
                                               FunctionTemplateDeduction & out,
                                               semantic_model::Scope * resolution_scope,
                                               const std::vector<template_model::TemplateArgument> *
                                                   explicit_arguments)
{
  template_api::TemplateFunctionDeductionRequest request;
  request.decl = &decl;
  request.target_type = target_type;
  request.use_scope = use_scope;
  request.resolution_scope = resolution_scope ? resolution_scope : use_scope;
  request.explicit_arguments = explicit_arguments;
  return deduce_function_template(ctx, request, out);
}

bool resolve_explicit_function_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<std::string> & arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    bool require_fully_bound)
{
  out.clear();
  if(!template_api::resolve_template_arguments(ctx,
                                               use_scope,
                                               decl.parameters,
                                               arg_texts,
                                               out,
                                               decl.declaring_scope)) {
    return false;
  }
  return !require_fully_bound ||
         template_model::template_arguments_fully_bind_parameters(decl.parameters, out);
}

bool resolve_call_explicit_function_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_arg_syntaxes)
{
  out.clear();
  return template_api::resolution::resolve_function_explicit_template_arguments(
      ctx,
      decl,
      resolution_scope,
      explicit_arg_texts,
      out,
      explicit_arg_syntaxes);
}

bool transformed_function_template_parameter_types(
    semantic_model::FunctionTemplateDecl & decl,
    std::vector<cpp_decl::TypePtr> & out)
{
  out.clear();
  std::vector<template_model::TemplateArgument> placeholders;
  if(!build_partial_ordering_placeholder_arguments(decl.parameters, placeholders)) {
    return false;
  }

  // A transformed function template has one unique synthesized entity for
  // each template parameter, including each parameter pack.  Present that
  // scalar view to substitution so two independent packs cannot compete for
  // ownership of the flat placeholder vector.
  std::vector<template_model::TemplateParameterInfo> scalar_parameters =
      decl.parameters;
  for(std::size_t i = 0; i < scalar_parameters.size(); ++i) {
    scalar_parameters[i].parameter_pack = false;
  }

  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    cpp_decl::TypePtr transformed;
    if(!template_api::type::substitute_type(decl.params_pattern[i].second,
                                            scalar_parameters,
                                            placeholders,
                                            transformed)) {
      return false;
    }
    out.push_back(transformed);
  }
  return true;
}

bool function_template_accepts_transformed_parameter_types(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<cpp_decl::TypePtr> & actual_params,
    semantic_model::Scope * actual_lookup_scope,
    bool allow_prefix)
{
  const bool has_trailing_pack =
      decl.has_trailing_function_parameter_pack && !decl.params_pattern.empty();
  const std::size_t fixed_count =
      has_trailing_pack ? decl.params_pattern.size() - 1 : decl.params_pattern.size();
  if(has_trailing_pack) {
    if(actual_params.size() < fixed_count) {
      return false;
    }
  } else if(allow_prefix) {
    if(actual_params.size() > decl.params_pattern.size()) {
      return false;
    }
  } else if(decl.params_pattern.size() != actual_params.size()) {
    return false;
  }

  semantic_model::Scope * deduction_scope =
      decl.pattern_scope ? decl.pattern_scope : decl.declaring_scope;
  std::map<std::string, cpp_decl::TypePtr> deduced;
  for(std::size_t i = 0; i < actual_params.size(); ++i) {
    const std::size_t pattern_index =
        has_trailing_pack && i >= fixed_count ? fixed_count : i;
    cpp_decl::TypePtr pattern = decl.params_pattern[pattern_index].second;
    cpp_decl::TypePtr actual = actual_params[i];
    bool pattern_reference_adjusted = false;
    bool actual_reference_adjusted = false;

    cpp_decl::TypePtr pattern_base = cpp_decl::strip_top_level_cv(pattern);
    if(pattern_base &&
       (pattern_base->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ||
        pattern_base->kind == cpp_decl::Type::TK_RVALUE_REFERENCE)) {
      pattern = pattern_base->inner;
      pattern_reference_adjusted = true;
    }
    cpp_decl::TypePtr actual_base = cpp_decl::strip_top_level_cv(actual);
    if(actual_base &&
       (actual_base->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ||
        actual_base->kind == cpp_decl::Type::TK_RVALUE_REFERENCE)) {
      actual = actual_base->inner;
      actual_reference_adjusted = true;
    }

    if(!(pattern_reference_adjusted && actual_reference_adjusted)) {
      pattern = cpp_decl::normalize_parameter_type(pattern);
      actual = cpp_decl::normalize_parameter_type(actual);
    }

    if(!template_api::resolution::deduce_template_argument(ctx,
                                                          decl.parameters,
                                                          pattern,
                                                          actual,
                                                          deduced,
                                                          deduction_scope,
                                                          true,
                                                          actual_lookup_scope)) {
      return false;
    }
  }
  return true;
}

void overlay_instantiation_use_scope_bindings(
    semantic_model::Scope & target,
    const semantic_model::Scope & source,
    const semantic_model::Scope * excluded_declaring_scope)
{
  template_api::binding::overlay_instantiation_use_scope_bindings(
      target,
      source,
      excluded_declaring_scope);
}

void overlay_instantiation_local_named_types(
    SemanticContext & ctx,
    semantic_model::Scope & target,
    semantic_model::Scope & source,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & local_type_arguments,
    const std::set<std::string> * excluded_parameter_names)
{
  template_api::binding::overlay_instantiation_local_named_types(
      ctx,
      target,
      source,
      declaring_scope,
      local_type_arguments,
      excluded_parameter_names);
}

semantic_model::FunctionBinding * acquire_function_template_binding(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::Scope * use_scope,
    const std::map<std::string, std::size_t> * pack_sizes,
    bool include_body,
    semantic_model::ClassInfo * active_owner,
    const std::string & instantiation_use_location)
{
  template_api::TemplateFunctionInstantiationRequest request;
  request.decl = &decl;
  request.arguments = arguments;
  request.active_owner = active_owner;
  request.use_scope = use_scope ? template_api::make_template_environment(*use_scope) :
                                  template_api::TemplateEnvironmentHandle();
  request.include_body = include_body;
  if(pack_sizes) {
    request.pack_sizes = *pack_sizes;
    request.has_pack_sizes = true;
  }
  request.instantiation_use_location = instantiation_use_location;
  return template_api::acquire_function_instantiation(ctx, request).function_binding;
}

namespace {

semantic_model::FunctionBinding * acquire_function_binding(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding,
    semantic_model::Scope & use_scope,
    template_api::TemplateFunctionBindingAcquisitionCause cause)
{
  template_api::TemplateFunctionBindingAcquisitionRequest request;
  request.binding = binding;
  request.use_scope = template_api::make_template_environment(use_scope);
  request.cause = cause;
  return template_api::acquire_function_binding(ctx, request).function_binding;
}

bool function_definition_closure_state(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding,
    template_api::TemplateFunctionDefinitionClosureState & state)
{
  if(!binding) {
    return false;
  }
  state = template_api::function_definition_closure_state(ctx, binding);
  return !state.decl_location.empty();
}

void note_require_definition_closure_event(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding,
    const template_api::TemplateFunctionDefinitionClosureState & state)
{
  template_api::note_function_binding_closure_event(
      ctx,
      template_api::TemplateWitnessLogEventKind::RequireDefinition,
      state.decl_location,
      binding,
      std::string(),
      template_api::TemplateLifecycleCause::RequireDefinition);
}

std::string function_call_template_name(
    const FunctionTemplateCallSourceUseRequest & request)
{
  if(!request.template_name.empty()) {
    return request.template_name;
  }
  return request.binding && request.binding->source_template ?
      request.binding->source_template->name :
      std::string();
}

std::string function_call_selected_name(
    const FunctionTemplateCallSourceUseRequest & request)
{
  if(!request.selected.empty()) {
    return request.selected;
  }
  return template_api::function_binding_witness_entity(request.binding);
}

void set_function_call_selected_decl_anchor(
    SemanticContext & ctx,
    witness::FunctionCallSourceDecision & decision,
    const FunctionTemplateCallSourceUseRequest & request)
{
  if(!request.selected_decl_anchor.location.empty() ||
     request.selected_decl_anchor.kind !=
         witness::TemplateWitnessSourceAnchorKind::None) {
    witness::set_selected_decl_anchor(decision.selected_decl_location,
                                      decision.selected_decl_anchor,
                                      request.selected_decl_anchor);
    return;
  }
  const std::string selected_decl_location =
      request.selected_decl_location.empty() ?
          (request.binding ?
               template_api::function_binding_witness_decl_location(ctx, request.binding) :
               std::string()) :
          request.selected_decl_location;
  witness::set_selected_decl_anchor(decision.selected_decl_location,
                                    decision.selected_decl_anchor,
                                    selected_decl_location,
                                    false);
}

bool template_argument_is_void_type(const template_model::TemplateArgument & argument)
{
  return argument.kind == template_model::TemplateArgument::TA_TYPE &&
         argument.type &&
         cpp_decl::is_void_type(argument.type);
}

bool function_call_has_defaulted_void_type_argument(
    const semantic_model::FunctionBinding * binding)
{
  if(!(binding && binding->has_instantiation_arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < binding->instantiation_arguments.size(); ++i) {
    const template_model::TemplateArgument & argument =
        binding->instantiation_arguments[i];
    if(argument.source_defaulted && template_argument_is_void_type(argument)) {
      return true;
    }
  }
  return false;
}

void suppress_defaulted_void_self_substitution_drop(
    witness::FunctionCallSourceDecision & decision,
    const semantic_model::FunctionBinding * binding)
{
  if(!function_call_has_defaulted_void_type_argument(binding) ||
     decision.candidates_built != 2 ||
     decision.candidates_viable != 1) {
    return;
  }
  std::vector<witness::TemplateWitnessSourceDrop> filtered;
  filtered.reserve(decision.drops.size());
  for(std::size_t i = 0; i < decision.drops.size(); ++i) {
    const witness::TemplateWitnessSourceDrop & drop = decision.drops[i];
    if(drop.reason == "substitution_failure" &&
       drop.candidate == decision.selected) {
      continue;
    }
    filtered.push_back(drop);
  }
  decision.drops.swap(filtered);
}

}  // namespace

semantic_model::FunctionBinding * acquire_function_definition_binding(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding,
    semantic_model::Scope & use_scope)
{
  return acquire_function_binding(
      ctx, binding, use_scope, template_api::TemplateFunctionBindingAcquisitionCause::None);
}

semantic_model::FunctionBinding * acquire_required_function_definition_binding(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding,
    semantic_model::Scope & use_scope)
{
  return acquire_function_binding(ctx,
                                  binding,
                                  use_scope,
                                  template_api::TemplateFunctionBindingAcquisitionCause::
                                      RequireDefinition);
}

void note_required_function_definition_materialized_by_lifecycle(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding)
{
  template_api::TemplateFunctionDefinitionClosureState state;
  if(!function_definition_closure_state(ctx, binding, state)) {
    return;
  }

  const template_api::ScopedTemplateWitnessEntryContext require_context(
      template_api::make_function_binding_closure_entry_context(
          ctx,
          template_api::TemplateClosureReason::RequireDefinition,
          binding));
  note_require_definition_closure_event(ctx, binding, state);
  template_api::note_function_definition_materialized_by_closure(ctx, binding, state);
}

void note_ensured_function_definition_materialized_by_lifecycle(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * binding)
{
  template_api::TemplateFunctionDefinitionClosureState state;
  if(!function_definition_closure_state(ctx, binding, state)) {
    return;
  }

  const template_api::ScopedTemplateWitnessEntryContext require_context(
      template_api::make_function_binding_closure_entry_context(
          ctx,
          template_api::TemplateClosureReason::RequireDefinition,
          binding));
  note_require_definition_closure_event(ctx, binding, state);

  const template_api::ScopedTemplateWitnessEntryContext ensure_context(
      template_api::make_function_binding_closure_entry_context(
          ctx,
          template_api::TemplateClosureReason::EnsureDefinition,
          binding));
  template_api::note_function_definition_ensure_requested(ctx, binding, state);
  template_api::note_function_definition_materialized_by_closure(ctx, binding, state);
}

void emit_function_template_call_source_use(
    SemanticContext & ctx,
    const FunctionTemplateCallSourceUseRequest & request)
{
  semantic_model::FunctionBinding * binding = request.binding;
  const bool has_explicit_source_target =
      !request.template_name.empty() || !request.selected.empty();
  if(!witness::function_call_source_capture_enabled() ||
     (!binding && !has_explicit_source_target) ||
     (binding && !binding->source_template) ||
     witness::template_witness_source_type_lookup_active()) {
    return;
  }

  const std::string public_location =
      template_api::normalize_template_witness_source_location(request.use_location);
  if(public_location.empty() ||
     !witness::source_location_capture_enabled(ctx.template_witness_context(),
                                               public_location)) {
    return;
  }

  witness::FunctionCallSourceDecision decision;
  decision.origin = witness::FunctionCallEmissionOrigin::OverloadSelectedCall;
  witness::set_use_anchor(decision.location,
                          decision.use_anchor,
                          public_location);
  decision.template_name = function_call_template_name(request);
  decision.selected = function_call_selected_name(request);
  decision.selection = request.selection != witness::SourceSelectionKind::None ?
      request.selection :
      (binding && binding->is_explicit_specialization ?
          witness::SourceSelectionKind::ExplicitSpecialization :
          witness::SourceSelectionKind::Instantiation);
  set_function_call_selected_decl_anchor(ctx, decision, request);
  decision.bindings = request.bindings;
  decision.drops = request.drops;
  decision.candidate_count = request.candidate_count;
  decision.candidates_built = request.candidates_built;
  decision.candidates_viable = request.candidates_viable;
  if(binding) {
    template_api::append_function_template_witness_bindings(ctx,
                                                            binding,
                                                            request.explicit_arg_count,
                                                            decision.bindings);
  }
  suppress_defaulted_void_self_substitution_drop(decision, binding);
  witness::emit_function_call(decision);
}

}  // namespace semantic_template_function
