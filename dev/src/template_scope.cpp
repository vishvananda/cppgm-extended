#include "template_scope.h"

#include <sstream>

#include "parser_trace.h"
#include "semantic_lookup.h"
#include "template_binding.h"

namespace template_scope {

using namespace cpp_decl;
using namespace semantic_model;
using namespace template_model;

namespace {

void hash_combine(std::size_t & seed, std::size_t value)
{
  seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

void hash_combine(std::size_t & seed, const std::string & value)
{
  hash_combine(seed, std::hash<std::string>()(value));
}

std::size_t & global_binding_fingerprint_epoch()
{
  static std::size_t epoch = 1;
  return epoch;
}

std::size_t compute_local_scope_binding_fingerprint(const Scope & scope)
{
  std::size_t seed = 0;
  hash_combine(seed, scope.instance_id);
  hash_combine(seed, scope.binding_fingerprint_epoch);
  hash_combine(seed, scope.name);
  hash_combine(seed, scope.namespace_scope ? 1 : 0);
  hash_combine(seed, scope.inline_namespace ? 1 : 0);
  hash_combine(seed, reinterpret_cast<std::size_t>(scope.class_info));
  hash_combine(seed, reinterpret_cast<std::size_t>(scope.function));
  return seed;
}

const char * overlay_mode_name(OverlayMode mode)
{
  return mode == OVERLAY_TEMPLATE_BOUND_ONLY ? "template-bound-only" : "all-bindings";
}

std::string scope_trace_name(const Scope & scope)
{
  if(!scope.name.empty()) {
    return scope.name;
  }
  if(scope.class_info && !scope.class_info->qualified_name.empty()) {
    return scope.class_info->qualified_name;
  }
  if(scope.function && !scope.function->name.empty()) {
    return scope.function->name;
  }
  return "<anonymous>";
}

void note_template_scope_event(const char * action,
                               const Scope & scope,
                               const std::string & binding,
                               std::size_t count = 0,
                               const std::string & detail = std::string())
{
  if(!parser_trace::enabled("template.scope")) {
    return;
  }
  std::ostringstream trace;
  trace << "action=" << action
        << " scope=" << scope_trace_name(scope)
        << " binding=" << binding
        << " instance=" << scope.instance_id;
  if(count != 0) {
    trace << " count=" << count;
  }
  if(!detail.empty()) {
    trace << " detail=" << detail;
  }
  parser_trace::note("template.scope", std::string(), trace.str());
}

}  // namespace

void bump_binding_fingerprint_epoch(Scope & scope)
{
  ++scope.binding_fingerprint_epoch;
  ++global_binding_fingerprint_epoch();
}

std::size_t scope_binding_fingerprint(const Scope & scope)
{
  const std::size_t global_epoch = global_binding_fingerprint_epoch();
  if(scope.cached_binding_scope_fingerprint_valid &&
     scope.cached_binding_scope_fingerprint_epoch == scope.binding_fingerprint_epoch &&
     scope.cached_binding_scope_global_epoch == global_epoch) {
    return scope.cached_binding_scope_fingerprint;
  }

  if(scope.cached_binding_scope_fingerprint_valid &&
     scope.cached_binding_scope_fingerprint_epoch == scope.binding_fingerprint_epoch &&
     (scope.namespace_scope ||
      !scope.parent ||
      (scope.parent->cached_binding_scope_fingerprint_valid &&
       scope.parent->cached_binding_scope_fingerprint_epoch ==
           scope.parent->binding_fingerprint_epoch &&
       scope.parent->cached_binding_scope_global_epoch == global_epoch &&
       scope.cached_binding_scope_parent_fingerprint ==
           scope.parent->cached_binding_scope_fingerprint))) {
    scope.cached_binding_scope_global_epoch = global_epoch;
    return scope.cached_binding_scope_fingerprint;
  }

  const std::size_t parent_fingerprint =
      (!scope.namespace_scope && scope.parent) ? scope_binding_fingerprint(*scope.parent) : 0;
  if(scope.cached_binding_scope_fingerprint_valid &&
     scope.cached_binding_scope_fingerprint_epoch == scope.binding_fingerprint_epoch &&
     scope.cached_binding_scope_parent_fingerprint == parent_fingerprint) {
    scope.cached_binding_scope_global_epoch = global_epoch;
    return scope.cached_binding_scope_fingerprint;
  }

  std::size_t seed = compute_local_scope_binding_fingerprint(scope);
  if(!scope.namespace_scope && scope.parent) {
    hash_combine(seed, parent_fingerprint);
  }

  scope.cached_binding_scope_fingerprint = seed;
  scope.cached_binding_scope_fingerprint_epoch = scope.binding_fingerprint_epoch;
  scope.cached_binding_scope_parent_fingerprint = parent_fingerprint;
  scope.cached_binding_scope_global_epoch = global_epoch;
  scope.cached_binding_scope_fingerprint_valid = true;
  return seed;
}

std::size_t scope_instance_fingerprint(const Scope & scope)
{
  if(scope.cached_instance_scope_fingerprint_valid) {
    return scope.cached_instance_scope_fingerprint;
  }

  std::size_t seed = 0;
  for(const Scope * current = &scope; current; current = current->parent) {
    hash_combine(seed, current->instance_id);
  }

  scope.cached_instance_scope_fingerprint = seed;
  scope.cached_instance_scope_fingerprint_valid = true;
  return seed;
}

template_binding::Hooks make_scope_binding_hooks(Scope & scope)
{
  template_binding::Hooks hooks;
  hooks.bind_pack_size = [&scope](const std::string & name, std::size_t count)
  {
    bind_pack_size(scope, name, count);
  };
  hooks.bind_type_pack = [&scope](const std::string & name,
                                  const std::vector<TypePtr> & bound_pack)
  {
    bind_type_pack(scope, name, bound_pack);
  };
  hooks.bind_non_type_pack =
      [&scope](const std::string & name,
               const TypePtr & bound_value_type,
               const std::vector<TemplateArgument> & bound_pack)
  {
    bind_non_type_pack(scope, name, bound_value_type, bound_pack);
  };
  hooks.bind_named_type = [&scope](const std::string & name, const TypePtr & type)
  {
    bind_named_type(scope, name, type);
  };
  hooks.bind_template_template = [&scope](const std::string & name,
                                          const TemplateArgument & argument)
  {
    bind_template_template_argument(scope, name, argument);
  };
  hooks.bind_non_type = [&scope](const std::string & name,
                                 const TypePtr & bound_value_type,
                                 const TemplateArgument & argument)
  {
    bind_non_type_value(
        scope,
        name,
        bound_value_type,
        argument.value,
        argument.dependent,
        !argument.dependent ? argument.text : std::string(),
        !argument.dependent ?
            const_cast<FunctionBinding *>(argument.function_value) :
            nullptr,
        !argument.dependent ? argument.value_binding : nullptr);
  };
  return hooks;
}

void bind_pack_size(Scope & scope, const std::string & name, std::size_t count)
{
  scope.named_pack_sizes[name] = count;
  bump_binding_fingerprint_epoch(scope);
}

std::string pack_value_alias_name(const std::string & pack_name,
                                  std::size_t index)
{
  if(index == 0) {
    return pack_name;
  }
  return pack_name + "__pack" + std::to_string(index + 1);
}

void bind_empty_parameter_pack(Scope & scope,
                               const TemplateParameterInfo & parameter)
{
  if(parameter.name.empty()) {
    return;
  }
  bind_pack_size(scope, parameter.name, 0);
  if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
    bind_type_pack(scope, parameter.name, std::vector<TypePtr>());
  }
}

void bind_type_pack(Scope & scope,
                    const std::string & name,
                    const std::vector<TypePtr> & bound_pack)
{
  scope.named_type_packs[name] = bound_pack;
  scope.template_bound_type_pack_names.insert(name);
  scope.named_pack_sizes[name] = bound_pack.size();
  bump_binding_fingerprint_epoch(scope);
  note_template_scope_event("bind-type-pack", scope, name, bound_pack.size());
}

void bind_named_type(Scope & scope, const std::string & name, const TypePtr & type)
{
  scope.named_types[name] = type;
  scope.template_bound_type_names.insert(name);
  bump_binding_fingerprint_epoch(scope);
}

void bind_value(Scope & scope,
                const std::string & name,
                const ValueBinding & binding,
                bool template_bound)
{
  scope.values[name] = binding;
  if(template_bound) {
    scope.template_bound_value_names.insert(name);
  }
  bump_binding_fingerprint_epoch(scope);
}

void bind_parameter_value(Scope & scope,
                          const std::string & name,
                          const TypePtr & type)
{
  if(name.empty()) {
    return;
  }
  bind_value(scope, name, ValueBinding(ValueBinding::VK_PARAMETER, name, type));
}

void bind_parameter_value_pack(Scope & scope,
                               const std::string & name,
                               const std::vector<TypePtr> & value_types)
{
  if(name.empty()) {
    return;
  }
  std::vector<ValueBinding> pack_bindings;
  pack_bindings.reserve(value_types.size());
  for(std::size_t i = 0; i < value_types.size(); ++i) {
    const std::string alias_name = pack_value_alias_name(name, i);
    ValueBinding binding(ValueBinding::VK_PARAMETER, alias_name, value_types[i]);
    bind_value(scope, alias_name, binding);
    pack_bindings.push_back(binding);
  }
  bind_value_pack(scope, name, pack_bindings);
}

void bind_non_type_value(Scope & scope,
                         const std::string & name,
                         const TypePtr & value_type,
                         long long value,
                         bool dependent,
                         const std::string & text,
                         FunctionBinding * function_value,
                         const ValueBinding * value_binding)
{
  ValueBinding binding(ValueBinding::VK_VARIABLE, name, value_type);
  TypePtr base_type = strip_top_level_cv(remove_reference_type(value_type));
  const bool reference_nttp =
      value_type &&
      (value_type->kind == Type::TK_LVALUE_REFERENCE ||
       value_type->kind == Type::TK_RVALUE_REFERENCE);
  const bool prefer_textual_binding =
      !text.empty() &&
      (reference_nttp ||
       (base_type &&
        !is_integral_type(base_type) &&
        !is_bool_type(base_type) &&
        !(base_type->kind == Type::TK_NAMED &&
          (base_type->named_key.compare(0, 5, "enum ") == 0 ||
           base_type->named_display.compare(0, 5, "enum ") == 0))));
  if(!dependent) {
    binding.non_type_template_function_value = function_value;
    binding.non_type_template_value_binding = value_binding;
    if(prefer_textual_binding) {
      binding.non_type_template_argument_text = text;
    } else {
      binding.has_constant_value = true;
      binding.constant_value = value;
    }
  } else {
    binding.dependent_template_value = true;
    if(!text.empty()) {
      binding.non_type_template_argument_text = text;
    }
  }
  bind_value(scope, name, binding, true);
}

void bind_non_type_pack(Scope & scope,
                        const std::string & name,
                        const TypePtr & value_type,
                        const std::vector<TemplateArgument> & bound_pack)
{
  scope.named_pack_sizes[name] = bound_pack.size();
  std::vector<ValueBinding> pack_bindings;
  pack_bindings.reserve(bound_pack.size());
  for(std::size_t i = 0; i < bound_pack.size(); ++i) {
    const std::string alias_name = pack_value_alias_name(name, i);
    const bool dependent = bound_pack[i].dependent;
    const long long value = dependent ? 0 : bound_pack[i].value;
    bind_non_type_value(scope,
                        alias_name,
                        value_type,
                        value,
                        dependent,
                        bound_pack[i].text,
                        !dependent ?
                            const_cast<FunctionBinding *>(bound_pack[i].function_value) :
                            nullptr,
                        !dependent ? bound_pack[i].value_binding : nullptr);
    pack_bindings.push_back(scope.values[alias_name]);
  }
  bind_value_pack(scope, name, pack_bindings, true);
  note_template_scope_event("bind-non-type-pack", scope, name, bound_pack.size());
}

void bind_template_argument_pack(Scope & scope,
                                 const TemplateParameterInfo & parameter,
                                 const std::vector<TemplateArgument> & arguments,
                                 bool template_bound)
{
  if(parameter.name.empty()) {
    return;
  }

  bind_pack_size(scope, parameter.name, arguments.size());
  if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
    std::vector<TypePtr> bound_pack;
    bound_pack.reserve(arguments.size());
    for(std::size_t i = 0; i < arguments.size(); ++i) {
      bound_pack.push_back(arguments[i].type);
    }
    bind_type_pack(scope, parameter.name, bound_pack);
    return;
  }

  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
    return;
  }

  std::vector<ValueBinding> bound_pack;
  bound_pack.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    const std::string alias_name = pack_value_alias_name(parameter.name, i);
    const TypePtr value_type =
        arguments[i].type ? arguments[i].type : parameter.value_type;
    if(arguments[i].kind == TemplateArgument::TA_VALUE) {
      bind_non_type_value(scope,
                          alias_name,
                          value_type,
                          arguments[i].value,
                          arguments[i].dependent,
                          arguments[i].text,
                          !arguments[i].dependent ?
                              const_cast<FunctionBinding *>(arguments[i].function_value) :
                              nullptr,
                          !arguments[i].dependent ? arguments[i].value_binding : nullptr);
    } else {
      bind_non_type_value(scope, alias_name, value_type, 0, true);
    }
    bound_pack.push_back(scope.values[alias_name]);
  }
  bind_value_pack(scope, parameter.name, bound_pack, template_bound);
}

void bind_template_parameter_placeholders(
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(parameter.name.empty()) {
      continue;
    }

    erase_template_parameter_binding(scope, parameter.name);

    if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
      std::string placeholder_payload = parameter.placeholder_key;
      static const char template_parameter_prefix[] = "template-parameter ";
      if(placeholder_payload.compare(0,
                                     sizeof(template_parameter_prefix) - 1,
                                     template_parameter_prefix) == 0) {
        placeholder_payload =
            placeholder_payload.substr(sizeof(template_parameter_prefix) - 1);
      }
      bind_named_type(scope,
                      parameter.name,
                      make_semantic_named(
                          std::string("typename ") + parameter.name,
                          Type::NSK_TEMPLATE_PARAMETER,
                          placeholder_payload.empty() ?
                              parameter.name :
                              placeholder_payload,
                          true));
      if(parameter.parameter_pack) {
        scope.template_bound_type_pack_names.insert(parameter.name);
        bump_binding_fingerprint_epoch(scope);
      }
    } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
      bind_non_type_value(scope,
                          parameter.name,
                          parameter.value_type,
                          0,
                          true);
      if(parameter.parameter_pack) {
        scope.template_bound_value_pack_names.insert(parameter.name);
        bump_binding_fingerprint_epoch(scope);
      }
    } else if(parameter.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      bind_template_template_placeholder(scope, parameter.name);
    }
  }
}

void bind_non_type_value_pack(Scope & scope,
                              const std::string & name,
                              const TypePtr & value_type,
                              const std::vector<long long> & values,
                              bool pack_template_bound)
{
  std::vector<ValueBinding> pack_bindings;
  pack_bindings.reserve(values.size());
  for(std::size_t i = 0; i < values.size(); ++i) {
    const std::string alias_name = pack_value_alias_name(name, i);
    bind_non_type_value(scope, alias_name, value_type, values[i], false);
    pack_bindings.push_back(scope.values[alias_name]);
  }
  bind_value_pack(scope, name, pack_bindings, pack_template_bound);
  note_template_scope_event("bind-non-type-value-pack", scope, name, values.size());
}

void bind_value_pack(Scope & scope,
                     const std::string & name,
                     const std::vector<ValueBinding> & bound_pack,
                     bool template_bound)
{
  scope.named_pack_sizes[name] = bound_pack.size();
  scope.named_value_packs[name] = bound_pack;
  if(template_bound) {
    scope.template_bound_value_names.insert(name);
    scope.template_bound_value_pack_names.insert(name);
  }
  bump_binding_fingerprint_epoch(scope);
}

bool erase_template_parameter_binding(Scope & scope, const std::string & name)
{
  bool changed = false;
  changed = scope.named_types.erase(name) != 0 || changed;
  changed = scope.template_bound_type_names.erase(name) != 0 || changed;
  changed = scope.named_type_packs.erase(name) != 0 || changed;
  changed = scope.template_bound_type_pack_names.erase(name) != 0 || changed;
  changed = scope.named_pack_sizes.erase(name) != 0 || changed;
  changed = scope.values.erase(name) != 0 || changed;
  changed = scope.template_bound_value_names.erase(name) != 0 || changed;
  changed = scope.template_bound_value_pack_names.erase(name) != 0 || changed;
  changed = scope.class_templates.erase(name) != 0 || changed;
  changed = scope.alias_templates.erase(name) != 0 || changed;
  changed = scope.template_bound_template_names.erase(name) != 0 || changed;
  changed = scope.template_bound_template_arguments.erase(name) != 0 || changed;
  if(changed) {
    bump_binding_fingerprint_epoch(scope);
  }
  return changed;
}

void bind_class_template(Scope & scope,
                         const std::string & name,
                         ClassTemplateDecl * decl)
{
  scope.class_templates[name] = decl;
  scope.alias_templates.erase(name);
  scope.template_bound_template_names.insert(name);
  scope.template_bound_template_arguments.erase(name);
  bump_binding_fingerprint_epoch(scope);
}

void bind_alias_template(Scope & scope,
                         const std::string & name,
                         AliasTemplateDecl * decl)
{
  scope.alias_templates[name] = decl;
  scope.class_templates.erase(name);
  scope.template_bound_template_names.insert(name);
  scope.template_bound_template_arguments.erase(name);
  bump_binding_fingerprint_epoch(scope);
}

void bind_template_template_placeholder(Scope & scope, const std::string & name)
{
  scope.class_templates[name] = nullptr;
  scope.template_bound_template_names.insert(name);
  bump_binding_fingerprint_epoch(scope);
}

void bind_template_template_argument(Scope & scope,
                                     const std::string & name,
                                     const TemplateArgument & argument)
{
  TemplateArgument stored_argument = argument;
  if(argument.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    AliasTemplateDecl * decl =
        static_cast<AliasTemplateDecl *>(argument.template_decl);
    bind_alias_template(scope, name, decl);
    if(decl && decl->declaring_scope && decl->declaring_scope->class_info) {
      stored_argument.text =
          semantic_lookup::scope_qualified_name(*decl->declaring_scope, decl->name);
    }
  } else {
    ClassTemplateDecl * decl =
        static_cast<ClassTemplateDecl *>(argument.template_decl);
    bind_class_template(scope, name, decl);
    if(decl && decl->declaring_scope && decl->declaring_scope->class_info) {
      stored_argument.text =
          semantic_lookup::scope_qualified_name(*decl->declaring_scope, decl->name);
    }
  }
  scope.template_bound_template_arguments[name] = stored_argument;
}

namespace {

template<class MapT, class SetT>
bool overlay_selected_entries(MapT & target,
                              const MapT & source,
                              const SetT * selected_names,
                              const std::set<std::string> * excluded_names)
{
  bool changed = false;
  if(selected_names) {
    for(const auto & name : *selected_names) {
      if(excluded_names && excluded_names->count(name) != 0) {
        continue;
      }
      typename MapT::const_iterator found = source.find(name);
      if(found != source.end()) {
        changed |= target.insert(*found).second;
      }
    }
    return changed;
  }

  for(const auto & entry : source) {
    if(excluded_names && excluded_names->count(entry.first) != 0) {
      continue;
    }
    changed |= target.insert(entry).second;
  }

  return changed;
}

bool allow_overlay_name(const std::set<std::string> * excluded_names,
                        const std::string & name)
{
  return !excluded_names || excluded_names->count(name) == 0;
}

void overlay_scope_bindings_impl(Scope & target,
                                 const Scope & source,
                                 OverlayMode mode,
                                 const std::set<std::string> * excluded_names)
{
  bool changed = false;
  const std::set<std::string> * type_names =
      mode == OVERLAY_TEMPLATE_BOUND_ONLY ? &source.template_bound_type_names : nullptr;
  changed |=
      overlay_selected_entries(target.named_types, source.named_types, type_names, excluded_names);
  if(type_names) {
    for(const auto & name : source.template_bound_type_names) {
      if(!allow_overlay_name(excluded_names, name)) {
        continue;
      }
      if(source.named_types.count(name) != 0) {
        changed |= target.template_bound_type_names.insert(name).second;
      }
    }
  }

  const std::set<std::string> * pack_names =
      mode == OVERLAY_TEMPLATE_BOUND_ONLY ? &source.template_bound_type_pack_names : nullptr;
  changed |= overlay_selected_entries(target.named_type_packs,
                                      source.named_type_packs,
                                      pack_names,
                                      excluded_names);
  if(pack_names) {
    for(const auto & name : source.template_bound_type_pack_names) {
      if(!allow_overlay_name(excluded_names, name)) {
        continue;
      }
      if(source.named_type_packs.count(name) != 0 ||
         source.named_types.count(name) != 0) {
        changed |= target.template_bound_type_pack_names.insert(name).second;
      }
    }
  }

  if(mode == OVERLAY_TEMPLATE_BOUND_ONLY) {
    for(const auto & name : source.template_bound_type_pack_names) {
      if(!allow_overlay_name(excluded_names, name)) {
        continue;
      }
      std::map<std::string, std::size_t>::const_iterator found = source.named_pack_sizes.find(name);
      if(found != source.named_pack_sizes.end()) {
        changed |= target.named_pack_sizes.insert(*found).second;
      }
    }
    for(const auto & name : source.template_bound_value_pack_names) {
      if(!allow_overlay_name(excluded_names, name)) {
        continue;
      }
      std::map<std::string, std::size_t>::const_iterator size =
          source.named_pack_sizes.find(name);
      if(size != source.named_pack_sizes.end()) {
        changed |= target.named_pack_sizes.insert(*size).second;
      }
      std::map<std::string, std::vector<ValueBinding> >::const_iterator pack =
          source.named_value_packs.find(name);
      if(pack != source.named_value_packs.end()) {
        changed |= target.named_value_packs.insert(*pack).second;
      }
      changed |= target.template_bound_value_pack_names.insert(name).second;
    }
  } else {
    changed |= overlay_selected_entries(target.named_pack_sizes,
                                        source.named_pack_sizes,
                                        static_cast<const std::set<std::string> *>(nullptr),
                                        excluded_names);
    changed |= overlay_selected_entries(target.named_value_packs,
                                        source.named_value_packs,
                                        static_cast<const std::set<std::string> *>(nullptr),
                                        excluded_names);
  }

  const std::set<std::string> * value_names =
      mode == OVERLAY_TEMPLATE_BOUND_ONLY ? &source.template_bound_value_names : nullptr;
  changed |=
      overlay_selected_entries(target.values, source.values, value_names, excluded_names);
  if(value_names) {
    for(const auto & name : source.template_bound_value_names) {
      if(!allow_overlay_name(excluded_names, name)) {
        continue;
      }
      if(source.values.count(name) != 0) {
        changed |= target.template_bound_value_names.insert(name).second;
      }
    }
  } else {
    for(const auto & name : source.template_bound_value_pack_names) {
      if(allow_overlay_name(excluded_names, name)) {
        changed |= target.template_bound_value_pack_names.insert(name).second;
      }
    }
  }

  const std::set<std::string> * template_names =
      mode == OVERLAY_TEMPLATE_BOUND_ONLY ? &source.template_bound_template_names : nullptr;
  if(mode == OVERLAY_ALL_BINDINGS || template_names) {
    changed |= overlay_selected_entries(target.class_templates,
                                        source.class_templates,
                                        template_names,
                                        excluded_names);
    changed |= overlay_selected_entries(target.alias_templates,
                                        source.alias_templates,
                                        template_names,
                                        excluded_names);
    changed |= overlay_selected_entries(target.template_bound_template_arguments,
                                        source.template_bound_template_arguments,
                                        template_names,
                                        excluded_names);
    if(template_names) {
      for(const auto & name : source.template_bound_template_names) {
        if(!allow_overlay_name(excluded_names, name)) {
          continue;
        }
        if(source.class_templates.count(name) != 0 ||
           source.alias_templates.count(name) != 0) {
          changed |= target.template_bound_template_names.insert(name).second;
        }
      }
    }
  }

  if(changed) {
    bump_binding_fingerprint_epoch(target);
  }
}

}  // namespace

void overlay_scope_bindings(Scope & target,
                            const Scope & source,
                            OverlayMode mode)
{
  overlay_scope_bindings_impl(target, source, mode, nullptr);
}

void overlay_ancestor_scope_bindings(Scope & target,
                                     const Scope & source,
                                     const Scope * stop_before,
                                     OverlayMode mode)
{
  for(const Scope * current = &source; current && current != stop_before;
      current = current->parent) {
    if(parser_trace::enabled("template.scope")) {
      std::ostringstream detail;
      detail << "target=" << scope_trace_name(target)
             << " source=" << scope_trace_name(*current)
             << " mode=" << overlay_mode_name(mode);
      note_template_scope_event("overlay-ancestor", target, scope_trace_name(*current), 0, detail.str());
    }
    overlay_scope_bindings_impl(target, *current, mode, nullptr);
  }
}

void overlay_ancestor_scope_bindings_excluding_names(
    Scope & target,
    const Scope & source,
    const Scope * stop_before,
    OverlayMode mode,
    const std::set<std::string> & excluded_names)
{
  for(const Scope * current = &source; current && current != stop_before;
      current = current->parent) {
    if(parser_trace::enabled("template.scope")) {
      std::ostringstream detail;
      detail << "target=" << scope_trace_name(target)
             << " source=" << scope_trace_name(*current)
             << " mode=" << overlay_mode_name(mode)
             << " excluded=" << excluded_names.size();
      note_template_scope_event("overlay-ancestor-excluding", target, scope_trace_name(*current), 0, detail.str());
    }
    overlay_scope_bindings_impl(target, *current, mode, &excluded_names);
  }
}

bool scope_has_type_parameter_pack_name(const Scope & scope,
                                        const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->template_bound_type_pack_names.count(name) != 0 ||
       current->named_type_packs.count(name) != 0) {
      return true;
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool scope_has_value_parameter_pack_name(const Scope & scope,
                                         const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->template_bound_value_pack_names.count(name) != 0 ||
       current->named_value_packs.count(name) != 0) {
      return true;
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

}  // namespace template_scope
