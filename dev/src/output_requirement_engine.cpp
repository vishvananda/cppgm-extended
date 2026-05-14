#include "output_requirement_engine.h"

#include <algorithm>
#include <sstream>

#include "parser_trace.h"
#include "symbol_linkage.h"
#include "template_api.h"

namespace output_requirement_engine {

using namespace semantic_model;

namespace {

FunctionBinding * resolve_required_binding(Hooks & hooks, FunctionBinding * binding)
{
  if(!binding || !hooks.resolve_output_function_binding) {
    return binding;
  }
  FunctionBinding * resolved = hooks.resolve_output_function_binding(binding);
  if(resolved && resolved != binding) {
    resolved->output_requirements |= binding->output_requirements;
  }
  return resolved ? resolved : binding;
}

const char * output_reason_name(OutputReason reason)
{
  switch(reason) {
  case OutputReason::DirectCall:
    return "direct-call";
  case OutputReason::ConstructorUse:
    return "constructor-use";
  case OutputReason::FunctionIdUse:
    return "function-id-use";
  case OutputReason::NewExpression:
    return "new-expression";
  case OutputReason::VTableSlot:
    return "vtable-slot";
  case OutputReason::TemplateUpgrade:
    return "template-upgrade";
  case OutputReason::SyntheticDependency:
    return "synthetic-dependency";
  case OutputReason::RuntimeDependency:
    return "runtime-dependency";
  }
  return "unknown";
}

const char * bool_name(bool value)
{
  return value ? "yes" : "no";
}

std::string binding_trace_entity(const FunctionBinding * binding)
{
  if(!binding) {
    return "<none>";
  }
  const std::string entity = function_binding_qualified_name_for_symbol(*binding);
  return entity.empty() ? std::string("<anonymous>") : entity;
}

void note_output_require_event(const char * action,
                               const FunctionBinding * binding,
                               const char * reason,
                               const std::string & detail = std::string())
{
  if(!parser_trace::enabled("output.require")) {
    return;
  }

  std::ostringstream trace;
  trace << "action=" << action;
  trace << " entity=" << binding_trace_entity(binding);
  if(binding) {
    trace << " binding=" << static_cast<const void *>(binding)
          << " name=" << binding->name
          << " symbol=" << binding->symbol.internal_symbol
          << " object=" << binding->symbol.object_symbol
          << " owner="
          << (binding->owner_class ? binding->owner_class->qualified_name :
                                     std::string("<none>"))
          << " key=" << template_api::function_binding_template_trace_key(binding)
          << " has-definition=" << bool_name(binding->has_definition)
          << " has-body=" << bool_name(binding->body != nullptr)
          << " source-template="
          << bool_name(template_api::function_binding_has_source_template_identity(
                 binding))
          << " deleted=" << bool_name(binding->is_deleted);
    if(binding->declaration_node) {
      trace << " decl-kind=" << cppast_kind_text(binding->declaration_node->kind);
    }
    if(binding->definition_node) {
      trace << " def-kind=" << cppast_kind_text(binding->definition_node->kind);
    }
    if(binding->owner_class) {
      trace << " owner-dependent="
            << bool_name(binding->owner_class->dependent_instantiation);
    }
  }
  if(reason && *reason) {
    trace << " reason=" << reason;
  }
  if(!detail.empty()) {
    trace << " detail=" << detail;
  }
  parser_trace::note("output.require", std::string(), trace.str());
}

void rekey_required_definition(State & state,
                               FunctionBinding * old_binding,
                               FunctionBinding * new_binding)
{
  if(!old_binding || old_binding == new_binding || !new_binding) {
    return;
  }

  const auto rekey_vector_and_set =
      [&](std::vector<FunctionBinding *> & bindings,
          std::unordered_set<FunctionBinding *> & binding_set)
      {
        const bool had_old = binding_set.erase(old_binding) != 0;
        if(had_old) {
          binding_set.insert(new_binding);
        }
        bool saw_new = false;
        std::size_t out = 0;
        for(std::size_t i = 0; i < bindings.size(); ++i) {
          FunctionBinding * current =
              bindings[i] == old_binding ? new_binding : bindings[i];
          if(current == new_binding) {
            if(saw_new) {
              continue;
            }
            saw_new = true;
          }
          bindings[out++] = current;
        }
        bindings.resize(out);
      };

  rekey_vector_and_set(state.required_function_definitions,
                       state.required_function_definition_set);
  rekey_vector_and_set(state.late_required_class_methods,
                       state.late_required_class_method_set);
  rekey_vector_and_set(state.late_required_class_static_functions,
                       state.late_required_class_static_function_set);
  rekey_vector_and_set(state.instantiated_functions,
                       state.instantiated_function_set);
  for(std::size_t i = 0; i < state.instantiated_classes.size(); ++i) {
    ClassInfo * info = state.instantiated_classes[i];
    if(!info) {
      continue;
    }
    for(std::size_t j = 0; j < info->method_declaration_order.size(); ++j) {
      if(info->method_declaration_order[j] == old_binding) {
        info->method_declaration_order[j] = new_binding;
      }
    }
    for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it =
            info->methods.begin();
        it != info->methods.end();
        ++it) {
      bool saw_method = false;
      std::size_t out = 0;
      for(std::size_t j = 0; j < it->second.size(); ++j) {
        FunctionBinding * current =
            it->second[j] == old_binding ? new_binding : it->second[j];
        if(current == new_binding) {
          if(saw_method) {
            continue;
          }
          saw_method = true;
        }
        it->second[out++] = current;
      }
      it->second.resize(out);
    }
    for(std::size_t j = 0; j < info->vtable_entries.size(); ++j) {
      if(info->vtable_entries[j] == old_binding) {
        info->vtable_entries[j] = new_binding;
      }
    }
  }
}

}  // namespace

void note_late_required_class_method(State & state, FunctionBinding * binding)
{
  if(!binding || !binding->owner_class ||
     (!binding->is_method && !binding->is_constructor && !binding->is_destructor)) {
    return;
  }
  binding->owner_class->has_late_required_class_method_output = true;
  if(state.late_required_class_method_set.insert(binding).second) {
    state.late_required_class_methods.push_back(binding);
  }
}

void note_late_required_class_static_function(State & state, FunctionBinding * binding)
{
  if(!binding || !binding->owner_class ||
     binding->is_method || binding->is_constructor || binding->is_destructor) {
    return;
  }
  binding->owner_class->has_late_required_class_static_function_output = true;
  if(state.late_required_class_static_function_set.insert(binding).second) {
    state.late_required_class_static_functions.push_back(binding);
  }
}

void note_instantiation(State & state,
                        Hooks & hooks,
                        FunctionBinding * binding,
                        InstantiatedFunctionOutputMode mode)
{
  if(!binding) {
    return;
  }
  if(mode == InstantiatedFunctionOutputMode::RequireDefinition) {
    require_definition(state, hooks, binding, OutputReason::TemplateUpgrade);
    return;
  }

  note_output_require_event("track-instantiation", binding, "track-only");
  if(state.instantiated_function_set.insert(binding).second) {
    state.instantiated_functions.push_back(binding);
  }
}

FunctionBinding * require_definition(State & state,
                                     Hooks & hooks,
                                     FunctionBinding * binding,
                                     OutputReason reason,
                                     bool enabled)
{
  if(!enabled || !binding || binding->is_deleted) {
    note_output_require_event("skip",
                              binding,
                              output_reason_name(reason),
                              !enabled ? "disabled" :
                              (!binding ? "null-binding" : "deleted"));
    return binding;
  }
  if((reason == OutputReason::ConstructorUse ||
      reason == OutputReason::DirectCall ||
      reason == OutputReason::FunctionIdUse ||
      reason == OutputReason::NewExpression) &&
     hooks.should_materialize_direct_call_output &&
     !hooks.should_materialize_direct_call_output(*binding)) {
    note_output_require_event("skip",
                              binding,
                              output_reason_name(reason),
                              "direct-call-output-suppressed");
    return binding;
  }

  if(template_api::function_binding_output_suppressed_by_explicit_instantiation(*binding)) {
    add_output_requirement(binding->output_requirements, ORK_DECLARATION);
    if(symbol_linkage::has_exported_object_symbol(binding->symbol)) {
      add_output_requirement(binding->output_requirements, ORK_EXPORT);
    }
    note_output_require_event("suppress-definition",
                              binding,
                              output_reason_name(reason),
                              "explicit-instantiation-suppression");
    return refresh_required_definition_binding(state, hooks, binding, false);
  }

  note_output_require_event("require-definition", binding, output_reason_name(reason));
  add_output_requirement(binding->output_requirements, ORK_DECLARATION);
  add_output_requirement(binding->output_requirements, ORK_DEFINITION);
  if(symbol_linkage::has_exported_object_symbol(binding->symbol)) {
    add_output_requirement(binding->output_requirements, ORK_EXPORT);
  }
  return refresh_required_definition_binding(state, hooks, binding, true);
}

FunctionBinding * refresh_required_definition_binding(State & state,
                                                      Hooks & hooks,
                                                      FunctionBinding * binding,
                                                      bool insert_if_missing)
{
  FunctionBinding * original_binding = binding;
  binding = resolve_required_binding(hooks, binding);
  if(!binding || binding->is_deleted) {
    if(original_binding && original_binding != binding) {
      state.required_function_definition_set.erase(original_binding);
    }
    return binding;
  }

  Scope * emit_scope =
      binding->owner_class && binding->owner_class->member_scope ?
          binding->owner_class->member_scope.get() :
          binding->declaration_scope;
  if(emit_scope &&
     (!binding->has_definition ||
      template_api::function_binding_has_template_identity(binding))) {
    FunctionBinding * upgraded =
        hooks.ensure_function_template_definition ?
            hooks.ensure_function_template_definition(binding, *emit_scope) :
            binding;
    if(upgraded) {
      upgraded->output_requirements |= binding->output_requirements;
      upgraded->is_explicit_instantiation_definition =
          upgraded->is_explicit_instantiation_definition ||
          binding->is_explicit_instantiation_definition;
      if(upgraded != binding) {
        note_output_require_event("upgrade-binding",
                                  upgraded,
                                  "refresh",
                                  std::string("from=") + binding->symbol.internal_symbol);
      }
      binding = upgraded;
    }
  }
  binding = resolve_required_binding(hooks, binding);
  if(!binding || binding->is_deleted) {
    if(original_binding && original_binding != binding) {
      state.required_function_definition_set.erase(original_binding);
    }
    return binding;
  }

  rekey_required_definition(state, original_binding, binding);
  if(insert_if_missing &&
     state.required_function_definition_set.insert(binding).second) {
    state.required_function_definitions.push_back(binding);
    note_output_require_event("insert-required-definition", binding, "refresh");
  }
  if(hooks.require_function_parameter_abi_output) {
    hooks.require_function_parameter_abi_output(binding);
  }
  note_late_required_class_method(state, binding);
  note_late_required_class_static_function(state, binding);
  if(template_api::function_binding_has_source_template_identity(binding)) {
    note_instantiation(state, hooks, binding, InstantiatedFunctionOutputMode::TrackOnly);
  }
  if(template_api::class_has_template_identity(binding->owner_class) &&
     !binding->owner_class->dependent_instantiation &&
     hooks.track_instantiated_class) {
    hooks.track_instantiated_class(binding->owner_class);
  }
  return binding;
}

FunctionBinding * adopt_output_requirements(State & state,
                                            Hooks & hooks,
                                            FunctionBinding * source,
                                            FunctionBinding * target,
                                            OutputReason reason)
{
  if(!target) {
    return target;
  }
  if(source && source != target) {
    target->output_requirements |= source->output_requirements;
    target->is_explicit_instantiation_definition =
        target->is_explicit_instantiation_definition ||
        source->is_explicit_instantiation_definition;
  }
  if(!source) {
    return target;
  }
  if(has_output_requirement(source->output_requirements, ORK_DEFINITION) &&
     !target->is_deleted) {
    return require_definition(state, hooks, target, reason);
  }
  if(has_output_requirement(source->output_requirements, ORK_DECLARATION)) {
    require_declaration(target);
  }
  if(has_output_requirement(source->output_requirements, ORK_EXPORT)) {
    require_export(target);
  }
  return target;
}

}  // namespace output_requirement_engine
