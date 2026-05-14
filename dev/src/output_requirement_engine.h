#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include "semantic_context.h"
#include "semantic_model.h"

namespace output_requirement_engine {

struct State
{
  std::vector<semantic_model::FunctionBinding *> & required_function_definitions;
  std::unordered_set<semantic_model::FunctionBinding *> & required_function_definition_set;
  std::vector<semantic_model::FunctionBinding *> & late_required_class_methods;
  std::unordered_set<semantic_model::FunctionBinding *> & late_required_class_method_set;
  std::vector<semantic_model::FunctionBinding *> & late_required_class_static_functions;
  std::unordered_set<semantic_model::FunctionBinding *> &
      late_required_class_static_function_set;
  std::vector<semantic_model::FunctionBinding *> & instantiated_functions;
  std::unordered_set<semantic_model::FunctionBinding *> & instantiated_function_set;
  std::vector<semantic_model::ClassInfo *> & instantiated_classes;
};

struct Hooks
{
  std::function<bool(const semantic_model::FunctionBinding &)>
      should_materialize_direct_call_output;
  std::function<semantic_model::FunctionBinding *(
      semantic_model::FunctionBinding *)> resolve_output_function_binding;
  std::function<semantic_model::FunctionBinding *(
      semantic_model::FunctionBinding *,
      semantic_model::Scope &)> ensure_function_template_definition;
  std::function<void(semantic_model::FunctionBinding *)> require_function_parameter_abi_output;
  std::function<void(semantic_model::ClassInfo *)> track_instantiated_class;
};

void note_late_required_class_method(State & state,
                                     semantic_model::FunctionBinding * binding);

void note_late_required_class_static_function(
    State & state,
    semantic_model::FunctionBinding * binding);

void note_instantiation(State & state,
                        Hooks & hooks,
                        semantic_model::FunctionBinding * binding,
                        InstantiatedFunctionOutputMode mode);

semantic_model::FunctionBinding * require_definition(State & state,
                                                     Hooks & hooks,
                                                     semantic_model::FunctionBinding * binding,
                                                     OutputReason reason,
                                                     bool enabled = true);

semantic_model::FunctionBinding * refresh_required_definition_binding(
    State & state,
    Hooks & hooks,
    semantic_model::FunctionBinding * binding,
    bool insert_if_missing = true);

semantic_model::FunctionBinding * adopt_output_requirements(
    State & state,
    Hooks & hooks,
    semantic_model::FunctionBinding * source,
    semantic_model::FunctionBinding * target,
    OutputReason reason);

inline void require_declaration(semantic_model::FunctionBinding * binding)
{
  if(binding) {
    semantic_model::add_output_requirement(binding->output_requirements,
                                           semantic_model::ORK_DECLARATION);
  }
}

inline void require_export(semantic_model::FunctionBinding * binding)
{
  if(binding) {
    semantic_model::add_output_requirement(binding->output_requirements,
                                           semantic_model::ORK_EXPORT);
  }
}

}  // namespace output_requirement_engine
