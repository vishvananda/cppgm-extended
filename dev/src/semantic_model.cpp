#include "semantic_model.h"

#include <algorithm>
#include <atomic>
#include <sstream>
#include <utility>

#include "callsemantic_internal.h"
#include "semantic_lookup.h"
#include "semantic_utils.h"

namespace semantic_model {

namespace {

std::string namespace_prefix(const Scope * scope)
{
  std::vector<std::string> parts;
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope &&
       !current->name.empty() &&
       current->name != "<global>" &&
       current->name != "<unnamed>") {
      parts.push_back(current->name);
    }
  }
  std::string out;
  for(std::size_t i = parts.size(); i > 0; --i) {
    out += parts[i - 1];
    out += "::";
  }
  return out;
}

void collect_inline_namespace_prefix_replacements(
    const Scope & scope,
    std::vector<std::pair<std::string, std::string> > & out)
{
  for(std::size_t i = 0; i < scope.namespace_children.size(); ++i) {
    const Scope * child = scope.namespace_children[i].get();
    if(!child) {
      continue;
    }
    if(child->inline_namespace) {
      const std::string before = namespace_prefix(child);
      const std::string after = namespace_prefix(child->parent);
      if(!before.empty() && before != after) {
        out.push_back(std::make_pair(before, after));
      }
    }
    collect_inline_namespace_prefix_replacements(*child, out);
  }
}

const Scope * pretty_function_scope_anchor(const FunctionBinding & binding)
{
  if(binding.declaration_scope) {
    return binding.declaration_scope;
  }
  if(binding.source_template && binding.source_template->declaring_scope) {
    return binding.source_template->declaring_scope;
  }
  if(binding.owner_class && binding.owner_class->enclosing_scope) {
    return binding.owner_class->enclosing_scope;
  }
  return nullptr;
}

std::string collapse_pretty_function_inline_namespaces(
    const FunctionBinding & binding,
    std::string text)
{
  const Scope * root = pretty_function_scope_anchor(binding);
  if(!root) {
    return text;
  }
  while(root->parent) {
    root = root->parent;
  }

  std::vector<std::pair<std::string, std::string> > replacements;
  collect_inline_namespace_prefix_replacements(*root, replacements);
  std::sort(
      replacements.begin(),
      replacements.end(),
      [](const std::pair<std::string, std::string> & lhs,
         const std::pair<std::string, std::string> & rhs)
      {
        return lhs.first.size() > rhs.first.size();
      });
  for(std::size_t i = 0; i < replacements.size(); ++i) {
    std::size_t pos = 0;
    while((pos = text.find(replacements[i].first, pos)) != std::string::npos) {
      text.replace(pos,
                   replacements[i].first.size(),
                   replacements[i].second);
      pos += replacements[i].second.size();
    }
  }
  return text;
}

std::string predefined_pretty_function_type_text(
    const cpp_decl::TypePtr & type)
{
  std::string text = cpp_decl::template_argument_type_text(type);
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base ||
     base->kind != cpp_decl::Type::TK_NAMED ||
     (base->named_key.compare(0, 5, "enum ") != 0 &&
      base->named_key.compare(0, 11, "enum class ") != 0 &&
      base->named_key.compare(0, 12, "enum struct ") != 0)) {
    return text;
  }

  const std::string display =
      semantic_utils::strip_elaborated_type_prefix(
          named_type_display_text(base));
  const std::string qualified =
      semantic_utils::strip_elaborated_type_prefix(base->named_key);
  if(!display.empty() &&
     qualified.find("::") != std::string::npos &&
     text.compare(0, display.size(), display) == 0) {
    text.replace(0, display.size(), qualified);
  }
  return text;
}

Scope * nonmember_hidden_friend_entity_scope(const FunctionBinding & binding)
{
  if(binding.owner_class ||
     !binding.source_template ||
     binding.source_template->friend_access_classes.empty() ||
     !binding.source_template->declaring_scope) {
    return nullptr;
  }

  Scope * scope = binding.source_template->declaring_scope;
  if(scope->class_info && scope->parent) {
    const bool friend_declared_in_owner =
        std::find(binding.source_template->friend_access_classes.begin(),
                  binding.source_template->friend_access_classes.end(),
                  scope->class_info) !=
        binding.source_template->friend_access_classes.end();
    if(friend_declared_in_owner) {
      return scope->parent;
    }
  }
  return scope;
}

}  // namespace

bool text_mentions_template_parameter(
    const std::string & text,
    const std::vector<template_model::TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    bool changed = false;
    if(!parameters[i].name.empty()) {
      callsemantic_internal::replace_identifier_token_text(
          text,
          parameters[i].name,
          std::string(),
          changed);
      if(changed) {
        return true;
      }
    }
    if(!parameters[i].placeholder_key.empty()) {
      callsemantic_internal::replace_identifier_token_text(
          text,
          parameters[i].placeholder_key,
          std::string(),
          changed);
      if(changed) {
        return true;
      }
    }
    for(std::size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      if(parameters[i].alternate_names[j].empty()) {
        continue;
      }
      callsemantic_internal::replace_identifier_token_text(
          text,
          parameters[i].alternate_names[j],
          std::string(),
          changed);
      if(changed) {
        return true;
      }
    }
  }
  return false;
}

bool conversion_display_name_needs_instantiated_result(
    const FunctionBinding & binding,
    const std::string & display_name)
{
  return binding.owner_class &&
         binding.owner_class->source_template &&
         text_mentions_template_parameter(
             display_name,
             binding.owner_class->source_template->parameters);
}

std::size_t next_scope_instance_id()
{
  static std::atomic<std::size_t> next_id(1);
  return next_id.fetch_add(1, std::memory_order_relaxed);
}

std::size_t next_class_instance_id()
{
  static std::atomic<std::size_t> next_id(1);
  return next_id.fetch_add(1, std::memory_order_relaxed);
}

Scope::Scope(const Scope & other)
  : parent(other.parent),
    name(other.name),
    namespace_scope(other.namespace_scope),
    inline_namespace(other.inline_namespace),
    persistent_lifetime(other.persistent_lifetime),
    class_info(other.class_info),
    function(other.function),
    named_types(other.named_types),
    named_type_access(other.named_type_access),
    named_type_packs(other.named_type_packs),
    named_value_packs(other.named_value_packs),
    named_pack_sizes(other.named_pack_sizes),
    template_bound_type_names(other.template_bound_type_names),
    template_bound_type_pack_names(other.template_bound_type_pack_names),
    template_bound_value_names(other.template_bound_value_names),
    template_bound_value_pack_names(other.template_bound_value_pack_names),
    template_bound_template_names(other.template_bound_template_names),
    template_bound_template_arguments(other.template_bound_template_arguments),
    values(other.values),
    namespace_bindings(other.namespace_bindings),
    namespace_binding_first_token_starts(
        other.namespace_binding_first_token_starts ?
            new std::map<std::string, std::size_t>(
                *other.namespace_binding_first_token_starts) :
            nullptr),
    function_sets(other.function_sets),
    function_binding_first_token_starts(
        other.function_binding_first_token_starts ?
            new std::map<std::string,
                         std::map<const FunctionBinding *, std::size_t> >(
                *other.function_binding_first_token_starts) :
            nullptr),
    function_set_access_overrides(other.function_set_access_overrides),
    class_templates(other.class_templates),
    function_templates(other.function_templates),
    collected_template_declarations(other.collected_template_declarations),
    alias_templates(other.alias_templates),
    variable_templates(other.variable_templates),
    using_directives(other.using_directives),
    instance_id(next_scope_instance_id()),
    binding_fingerprint_epoch(other.binding_fingerprint_epoch),
    direct_function_lookup_cache_epoch(other.direct_function_lookup_cache_epoch)
{}

Scope::Scope(Scope && other)
  : parent(other.parent),
    name(std::move(other.name)),
    namespace_scope(other.namespace_scope),
    inline_namespace(other.inline_namespace),
    persistent_lifetime(other.persistent_lifetime),
    class_info(other.class_info),
    function(other.function),
    named_types(std::move(other.named_types)),
    named_type_access(std::move(other.named_type_access)),
    named_type_packs(std::move(other.named_type_packs)),
    named_value_packs(std::move(other.named_value_packs)),
    named_pack_sizes(std::move(other.named_pack_sizes)),
    template_bound_type_names(std::move(other.template_bound_type_names)),
    template_bound_type_pack_names(std::move(other.template_bound_type_pack_names)),
    template_bound_value_names(std::move(other.template_bound_value_names)),
    template_bound_value_pack_names(std::move(other.template_bound_value_pack_names)),
    template_bound_template_names(std::move(other.template_bound_template_names)),
    template_bound_template_arguments(std::move(other.template_bound_template_arguments)),
    values(std::move(other.values)),
    namespace_bindings(std::move(other.namespace_bindings)),
    namespace_binding_first_token_starts(
        std::move(other.namespace_binding_first_token_starts)),
    function_sets(std::move(other.function_sets)),
    function_binding_first_token_starts(
        std::move(other.function_binding_first_token_starts)),
    function_set_access_overrides(std::move(other.function_set_access_overrides)),
    class_templates(std::move(other.class_templates)),
    function_templates(std::move(other.function_templates)),
    collected_template_declarations(std::move(other.collected_template_declarations)),
    alias_templates(std::move(other.alias_templates)),
    variable_templates(std::move(other.variable_templates)),
    using_directives(std::move(other.using_directives)),
    namespace_children(std::move(other.namespace_children)),
    instance_id(other.instance_id),
    binding_fingerprint_epoch(other.binding_fingerprint_epoch),
    direct_function_lookup_cache_epoch(other.direct_function_lookup_cache_epoch)
{}

Scope & Scope::operator=(Scope && other)
{
  if(this == &other) {
    return *this;
  }

  parent = other.parent;
  name = std::move(other.name);
  namespace_scope = other.namespace_scope;
  inline_namespace = other.inline_namespace;
  persistent_lifetime = other.persistent_lifetime;
  class_info = other.class_info;
  function = other.function;
  named_types = std::move(other.named_types);
  named_type_access = std::move(other.named_type_access);
  named_type_packs = std::move(other.named_type_packs);
  named_value_packs = std::move(other.named_value_packs);
  named_pack_sizes = std::move(other.named_pack_sizes);
  template_bound_type_names = std::move(other.template_bound_type_names);
  template_bound_type_pack_names = std::move(other.template_bound_type_pack_names);
  template_bound_value_names = std::move(other.template_bound_value_names);
  template_bound_value_pack_names = std::move(other.template_bound_value_pack_names);
  template_bound_template_names = std::move(other.template_bound_template_names);
  template_bound_template_arguments = std::move(other.template_bound_template_arguments);
  values = std::move(other.values);
  namespace_bindings = std::move(other.namespace_bindings);
  namespace_binding_first_token_starts =
      std::move(other.namespace_binding_first_token_starts);
  function_sets = std::move(other.function_sets);
  function_binding_first_token_starts =
      std::move(other.function_binding_first_token_starts);
  function_set_access_overrides = std::move(other.function_set_access_overrides);
  cached_direct_function_lookups.clear();
  direct_function_lookup_cache_epoch = other.direct_function_lookup_cache_epoch;
  class_templates = std::move(other.class_templates);
  function_templates = std::move(other.function_templates);
  collected_template_declarations = std::move(other.collected_template_declarations);
  alias_templates = std::move(other.alias_templates);
  variable_templates = std::move(other.variable_templates);
  using_directives = std::move(other.using_directives);
  namespace_children = std::move(other.namespace_children);
  binding_fingerprint_epoch = other.binding_fingerprint_epoch;
  instance_id = other.instance_id;
  cached_binding_scope_fingerprint_valid = false;
  cached_binding_scope_fingerprint = 0;
  cached_binding_scope_fingerprint_epoch = 0;
  cached_binding_scope_parent_fingerprint = 0;
  cached_instance_scope_fingerprint_valid = false;
  cached_instance_scope_fingerprint = 0;
  return *this;
}

std::string describe_scope_bindings(const Scope & scope)
{
  std::ostringstream out;
  bool wrote_any = false;

  for(const Scope * current = &scope; current; current = current->parent) {
    // Namespace scopes -- notably the global scope -- accumulate the host
    // compiler's builtin type table, whose contents depend on the host
    // toolchain. Including it makes this diagnostic vary across environments
    // (and bloats it with builtins irrelevant to the failure); the bindings
    // that matter here live in class, function, and template scopes.
    if(current->namespace_scope) {
      continue;
    }

    bool scope_has_any = false;
    std::ostringstream scope_out;

    // named_types is an unordered_map, so emit its entries in a deterministic
    // key-sorted order rather than the implementation-defined bucket order.
    std::vector<const Scope::NamedTypeMap::value_type *> sorted_named_types;
    sorted_named_types.reserve(current->named_types.size());
    for(const auto & named : current->named_types) {
      if(named.first.empty()) {
        continue;
      }
      sorted_named_types.push_back(&named);
    }
    std::sort(sorted_named_types.begin(), sorted_named_types.end(),
              [](const Scope::NamedTypeMap::value_type * a,
                 const Scope::NamedTypeMap::value_type * b) {
                return a->first < b->first;
              });
    for(const Scope::NamedTypeMap::value_type * named_entry : sorted_named_types) {
      const auto & named = *named_entry;
      if(!scope_has_any) {
        scope_out << current->name;
        scope_out << "{types:";
        scope_has_any = true;
      } else {
        scope_out << ",";
      }
      scope_out << named.first << "=" << cpp_decl::describe_type(named.second);
    }

    for(const auto & value : current->values) {
      if(value.first.empty()) {
        continue;
      }
      if(!scope_has_any) {
        scope_out << current->name;
        scope_out << "{values:";
        scope_has_any = true;
      } else {
        scope_out << ",";
      }
      scope_out << value.first << "=" << cpp_decl::describe_type(value.second.type);
      if(value.second.has_constant_value) {
        scope_out << ":" << value.second.constant_value;
      }
    }

    for(const auto & pack : current->named_type_packs) {
      if(pack.first.empty()) {
        continue;
      }
      if(!scope_has_any) {
        scope_out << current->name;
        scope_out << "{type_packs:";
        scope_has_any = true;
      } else {
        scope_out << ",";
      }
      scope_out << pack.first << "=[";
      for(std::size_t i = 0; i < pack.second.size(); ++i) {
        if(i != 0) {
          scope_out << ", ";
        }
        scope_out << cpp_decl::describe_type(pack.second[i]);
      }
      scope_out << "]";
    }

    for(const auto & pack : current->named_value_packs) {
      if(pack.first.empty()) {
        continue;
      }
      if(!scope_has_any) {
        scope_out << current->name;
        scope_out << "{value_packs:";
        scope_has_any = true;
      } else {
        scope_out << ",";
      }
      scope_out << pack.first << "=[";
      for(std::size_t i = 0; i < pack.second.size(); ++i) {
        if(i != 0) {
          scope_out << ", ";
        }
        scope_out << pack.second[i].name << ":" << cpp_decl::describe_type(pack.second[i].type);
      }
      scope_out << "]";
    }

    if(scope_has_any) {
      scope_out << "}";
      if(wrote_any) {
        out << " ";
      }
      out << scope_out.str();
      wrote_any = true;
    }
  }

  return out.str();
}

std::string function_binding_member_name_for_symbol(const FunctionBinding & binding)
{
  const std::string canonical_name =
      semantic_lookup::canonical_function_lookup_name(binding.name);
  if(binding.owner_class && !binding.owner_class->qualified_name.empty()) {
    const std::string owner_name =
        semantic_lookup::canonical_function_lookup_name(
            binding.owner_class->qualified_name);
    if(canonical_name.size() > owner_name.size() + 2 &&
       canonical_name.compare(0, owner_name.size(), owner_name) == 0 &&
       canonical_name.compare(owner_name.size(), 2, "::") == 0) {
      return canonical_name.substr(owner_name.size() + 2);
    }
  }
  return semantic_utils::unqualified_member_name(canonical_name);
}

std::string function_binding_display_name_for_symbol(const FunctionBinding & binding)
{
  const std::string member_binding_name =
      function_binding_member_name_for_symbol(binding);
  std::string display_name = binding.display_name.empty() ?
      member_binding_name :
      binding.display_name;
  if(display_name.empty() &&
     binding.name.compare(0, 8, "operator") == 0) {
    display_name = binding.name;
  }
  const bool is_conversion_function = binding.is_conversion_operator;
  bool rebuilt_conversion_display = false;
  if(is_conversion_function &&
     (display_name.find(' ') == std::string::npos ||
      display_name.compare(0, 8, "operator") != 0)) {
    cpp_decl::TypePtr function_type = binding.declared_type ? binding.declared_type : binding.type;
    if(function_type &&
       function_type->kind == cpp_decl::Type::TK_FUNCTION &&
       function_type->inner) {
      display_name = std::string("operator ") +
          (display_name.compare(0, 8, "operator") == 0 ?
               cpp_decl::describe_type(function_type->inner) :
               cpp_decl::template_argument_type_text(function_type->inner));
      rebuilt_conversion_display = true;
    }
  }
  if(is_conversion_function &&
     (rebuilt_conversion_display ||
      display_name.compare(0, 9, "operator ") == 0)) {
    return display_name;
  }
  const std::size_t display_split = display_name.rfind("::");
  return display_split == std::string::npos ? display_name : display_name.substr(display_split + 2);
}

std::string predefined_pretty_function_parameter_list(
    const cpp_decl::TypePtr & function_type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(function_type);
  if(!base || base->kind != cpp_decl::Type::TK_FUNCTION) {
    return "";
  }

  std::string out;
  for(std::size_t i = 0; i < base->params.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += cpp_decl::template_argument_type_text(base->params[i]);
  }
  if(base->variadic) {
    if(!out.empty()) {
      out += ", ";
    }
    out += "...";
  }
  return out;
}

std::string predefined_pretty_function_template_arguments(
    const FunctionBinding & binding)
{
  std::string out;
  const auto append_arguments =
      [&](const std::vector<template_model::TemplateParameterInfo> & parameters,
          const std::vector<template_model::TemplateArgument> & arguments)
      {
        const std::size_t count = std::min(parameters.size(), arguments.size());
        for(std::size_t i = 0; i < count; ++i) {
          out += out.empty() ? " [" : ", ";
          out += parameters[i].name.empty() ?
              std::string("<anonymous>") :
              parameters[i].name;
          out += " = ";
          if(arguments[i].kind == template_model::TemplateArgument::TA_TYPE &&
             arguments[i].type) {
            out += predefined_pretty_function_type_text(arguments[i].type);
          } else {
            out += template_model::template_argument_text(
                arguments[i],
                predefined_pretty_function_type_text);
          }
        }
      };

  if(binding.owner_class && binding.owner_class->source_template) {
    append_arguments(binding.owner_class->source_template->parameters,
                     binding.owner_class->instantiation_arguments);
  }
  if(binding.source_template && binding.has_instantiation_arguments) {
    append_arguments(binding.source_template->parameters,
                     binding.instantiation_arguments);
  }
  if(out.empty()) {
    return out;
  }
  out += "]";
  return out;
}

std::string predefined_pretty_function_text(const FunctionBinding & binding)
{
  cpp_decl::TypePtr function_type = cpp_decl::strip_top_level_cv(binding.type);
  std::string return_text = "void";
  if(function_type &&
     function_type->kind == cpp_decl::Type::TK_FUNCTION &&
     function_type->inner) {
    return_text = cpp_decl::template_argument_type_text(function_type->inner);
  }

  const std::string function_name =
      !binding.name.empty() ? binding.name :
      !binding.display_name.empty() ? binding.display_name :
      function_binding_display_name_for_symbol(binding);
  return collapse_pretty_function_inline_namespaces(
      binding,
      return_text + " " + function_name + "(" +
          predefined_pretty_function_parameter_list(function_type) + ")" +
          predefined_pretty_function_template_arguments(binding));
}

std::string function_binding_qualified_name_for_symbol(const FunctionBinding & binding)
{
  const std::string simple_name = function_binding_display_name_for_symbol(binding);
  if(binding.owner_class && !binding.owner_class->qualified_name.empty()) {
    return binding.owner_class->qualified_name + "::" + simple_name;
  }
  if(Scope * friend_entity_scope = nonmember_hidden_friend_entity_scope(binding)) {
    return semantic_lookup::scope_symbol_qualified_name(*friend_entity_scope,
                                                        simple_name);
  }
  if(binding.declaration_scope && binding.name.find("::") == std::string::npos) {
    return semantic_lookup::scope_symbol_qualified_name(*binding.declaration_scope, simple_name);
  }
  return binding.name;
}

bool function_binding_qualified_name_syntax_for_symbol(
    const FunctionBinding & binding,
    cpp_decl::QualifiedName & out)
{
  const std::string simple_name = function_binding_display_name_for_symbol(binding);
  if(simple_name.empty()) {
    return false;
  }
  if(binding.owner_class) {
    out = class_output_qualified_name_syntax(*binding.owner_class);
  }
  if(binding.owner_class && !out.name.empty()) {
    out.qualifiers.push_back(out.name);
    out.name = simple_name;
    return true;
  }
  if(binding.owner_class) {
    return false;
  }
  if(Scope * friend_entity_scope = nonmember_hidden_friend_entity_scope(binding)) {
    out = semantic_lookup::scope_symbol_qualified_name_syntax(*friend_entity_scope,
                                                              simple_name);
    return true;
  }
  if(binding.declaration_scope) {
    out = semantic_lookup::scope_symbol_qualified_name_syntax(*binding.declaration_scope,
                                                              simple_name);
    return true;
  }
  return false;
}

}  // namespace semantic_model
