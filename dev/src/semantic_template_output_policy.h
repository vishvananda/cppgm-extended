#pragma once

#include "semantic_context.h"
#include "semantic_model.h"

namespace semantic_template_output_policy {

struct ClassOutputReadiness
{
  bool templated_context = false;
  bool suppress_implicit_definition = false;
  bool non_dependent = true;
  bool complete = true;
  bool output_blocked_by_placeholders = false;
};

const semantic_model::ClassInfo * effective_class_output_owner(
    const semantic_model::ClassInfo & info);

ClassOutputReadiness class_output_readiness(SemanticContext & ctx,
                                            const semantic_model::ClassInfo & info);

bool implicit_instantiation_definition_suppressed(const semantic_model::ClassInfo * info);

bool function_obeys_implicit_instantiation_definition_suppression(
    const semantic_model::FunctionBinding & binding);

bool should_emit_instantiated_class_method_definition(
    const ClassOutputReadiness & class_output_readiness,
    const semantic_model::FunctionBinding & binding);

bool function_has_tracked_template_body(
    const semantic_model::FunctionBinding & binding,
    bool declaration_node_is_definition_syntax);

bool function_needs_template_definition_acquisition(
    const semantic_model::FunctionBinding & binding);

bool function_needs_source_template_definition_acquisition(
    const semantic_model::FunctionBinding & binding);

bool owner_class_instantiation_waits_for_non_dependent_arguments(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info);

bool function_is_declaration_only_template(
    const semantic_model::FunctionBinding & binding);

bool function_is_declaration_only_user_function(
    const semantic_model::FunctionBinding & binding,
    bool declaration_node_is_definition_syntax);

bool function_instantiation_arguments_complete(SemanticContext & ctx,
                                               const semantic_model::FunctionBinding & binding);

bool function_instantiation_arguments_dependent(SemanticContext & ctx,
                                                const semantic_model::FunctionBinding & binding);

}  // namespace semantic_template_output_policy
