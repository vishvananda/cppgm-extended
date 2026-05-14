#include "semantic_template_output_policy.h"

#include "symbol_linkage.h"
#include "template_api.h"

namespace semantic_template_output_policy {

namespace {

ClassOutputReadiness to_semantic_readiness(
    const template_api::TemplateClassOutputReadiness & readiness)
{
  ClassOutputReadiness out;
  out.templated_context = readiness.templated_context;
  out.suppress_implicit_definition = readiness.suppress_implicit_definition;
  out.non_dependent = readiness.non_dependent;
  out.complete = readiness.complete;
  out.output_blocked_by_placeholders = readiness.output_blocked_by_placeholders;
  return out;
}

}  // namespace

const semantic_model::ClassInfo * effective_class_output_owner(
    const semantic_model::ClassInfo & info)
{
  return template_api::effective_instantiated_class_output_owner(info);
}

ClassOutputReadiness class_output_readiness(SemanticContext & ctx,
                                            const semantic_model::ClassInfo & info)
{
  return to_semantic_readiness(
      template_api::compute_instantiated_class_output_readiness(ctx, info));
}

bool implicit_instantiation_definition_suppressed(const semantic_model::ClassInfo * info)
{
  return template_api::class_suppresses_implicit_instantiation_definition(info);
}

bool function_obeys_implicit_instantiation_definition_suppression(
    const semantic_model::FunctionBinding & binding)
{
  return !template_api::function_binding_excluded_from_explicit_instantiation(binding) &&
         !template_api::function_binding_bypasses_explicit_instantiation_suppression(binding);
}

bool should_emit_instantiated_class_method_definition(
    const ClassOutputReadiness & class_output_readiness,
    const semantic_model::FunctionBinding & binding)
{
  const bool definition_required =
      semantic_model::has_output_requirement(binding.output_requirements,
                                             semantic_model::ORK_DEFINITION);
  const bool synthesized_definition =
      binding.synthesized && !symbol_linkage::has_weak_linkage(binding.symbol);
  if(binding.is_deleted) {
    return false;
  }
  if(!class_output_readiness.templated_context) {
    return definition_required || synthesized_definition;
  }
  if(class_output_readiness.suppress_implicit_definition &&
     function_obeys_implicit_instantiation_definition_suppression(binding)) {
    return false;
  }
  if(definition_required &&
     (class_output_readiness.non_dependent ||
      class_output_readiness.output_blocked_by_placeholders)) {
    return true;
  }
  return class_output_readiness.complete &&
         (definition_required || synthesized_definition);
}

bool function_has_tracked_template_body(
    const semantic_model::FunctionBinding & binding,
    bool declaration_node_is_definition_syntax)
{
  return template_api::function_binding_has_template_identity(&binding) &&
         binding.has_definition &&
         !binding.definition_node &&
         !declaration_node_is_definition_syntax;
}

bool function_needs_template_definition_acquisition(
    const semantic_model::FunctionBinding & binding)
{
  return !binding.has_definition ||
         template_api::function_binding_has_template_identity(&binding);
}

bool function_needs_source_template_definition_acquisition(
    const semantic_model::FunctionBinding & binding)
{
  return !binding.has_definition ||
         template_api::function_binding_has_source_template_identity(&binding);
}

bool owner_class_instantiation_waits_for_non_dependent_arguments(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return template_api::class_has_template_identity(&info) &&
         !info.dependent_instantiation &&
         !template_api::instantiated_class_arguments_non_dependent(ctx, info) &&
         (!info.member_scope ||
          !ctx.scope_has_template_placeholders(*info.member_scope));
}

bool function_is_declaration_only_template(
    const semantic_model::FunctionBinding & binding)
{
  return template_api::function_binding_is_declaration_only_template(binding);
}

bool function_is_declaration_only_user_function(
    const semantic_model::FunctionBinding & binding,
    bool declaration_node_is_definition_syntax)
{
  const bool declaration_only_template =
      function_is_declaration_only_template(binding);
  return !declaration_node_is_definition_syntax &&
         (!template_api::function_binding_has_source_template_identity(&binding) ||
          declaration_only_template);
}

bool function_instantiation_arguments_complete(SemanticContext & ctx,
                                               const semantic_model::FunctionBinding & binding)
{
  return template_api::function_binding_instantiation_arguments_complete(ctx, binding);
}

bool function_instantiation_arguments_dependent(SemanticContext & ctx,
                                                const semantic_model::FunctionBinding & binding)
{
  return template_api::function_binding_instantiation_arguments_dependent(ctx, binding);
}

}  // namespace semantic_template_output_policy
