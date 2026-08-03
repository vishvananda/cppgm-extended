#include "semantic_scope_mutation.h"

#include <algorithm>

#include "semantic_lookup.h"
#include "template_api.h"

namespace semantic_scope_mutation {

namespace {

bool add_using_directive_raw(semantic_model::Scope & scope,
                             semantic_model::Scope & target,
                             std::size_t source_token_start = 0)
{
  std::vector<semantic_model::UsingDirectiveEntry>::iterator found =
      std::find(scope.using_directives.begin(),
                scope.using_directives.end(),
                &target);
  if(found != scope.using_directives.end()) {
    if(source_token_start != 0 &&
       (found->first_token_start == 0 ||
        source_token_start < found->first_token_start)) {
      found->first_token_start = source_token_start;
      return true;
    }
    return false;
  }
  scope.using_directives.push_back(
      semantic_model::UsingDirectiveEntry(&target, source_token_start));
  return true;
}

}  // namespace

void note_binding_mutation(semantic_model::Scope & scope)
{
  template_api::bump_scope_template_binding_fingerprint_epoch(scope);
}

void bind_named_type(semantic_model::Scope & scope,
                     const std::string & name,
                     const cpp_decl::TypePtr & type)
{
  scope.named_types[name] = type;
  note_binding_mutation(scope);
}

void bind_named_type_with_access(semantic_model::Scope & scope,
                                 const std::string & name,
                                 const cpp_decl::TypePtr & type,
                                 semantic_model::MemberAccess access)
{
  scope.named_types[name] = type;
  scope.named_type_access[name] = access;
  note_binding_mutation(scope);
}

void bind_template_named_type(semantic_model::Scope & scope,
                              const std::string & name,
                              const cpp_decl::TypePtr & type)
{
  template_api::binding::bind_named_type(scope, name, type);
}

void ensure_template_named_type(semantic_model::Scope & scope,
                                const std::string & name,
                                const cpp_decl::TypePtr & type)
{
  bool changed = false;
  if(scope.named_types.count(name) == 0) {
    scope.named_types[name] = type;
    changed = true;
  }
  changed = scope.template_bound_type_names.insert(name).second || changed;
  if(changed) {
    note_binding_mutation(scope);
  }
}

void bind_template_named_type_with_access(semantic_model::Scope & scope,
                                          const std::string & name,
                                          const cpp_decl::TypePtr & type,
                                          semantic_model::MemberAccess access)
{
  bind_template_named_type(scope, name, type);
  scope.named_type_access[name] = access;
  note_binding_mutation(scope);
}

void bind_namespace(semantic_model::Scope & scope,
                    const std::string & name,
                    semantic_model::Scope * target,
                    std::size_t source_token_start)
{
  scope.namespace_bindings[name] = target;
  if(source_token_start != 0) {
    if(!scope.namespace_binding_first_token_starts) {
      scope.namespace_binding_first_token_starts.reset(
          new std::map<std::string, std::size_t>());
    }
    std::size_t & first = (*scope.namespace_binding_first_token_starts)[name];
    if(first == 0 || source_token_start < first) {
      first = source_token_start;
    }
  }
  note_binding_mutation(scope);
}

void add_using_directive_if_needed(semantic_model::Scope & scope,
                                   semantic_model::Scope & target,
                                   std::size_t source_token_start)
{
  const bool changed =
      add_using_directive_raw(scope, target, source_token_start);
  if(changed) {
    note_binding_mutation(scope);
  }
}

void import_inline_namespace_members(semantic_model::Scope & scope,
                                     semantic_model::Scope & target)
{
  bool changed = false;
  for(auto it =
          target.namespace_bindings.begin();
      it != target.namespace_bindings.end(); ++it) {
    scope.namespace_bindings[it->first] = it->second;
    if(target.namespace_binding_first_token_starts) {
      const auto first =
          target.namespace_binding_first_token_starts->find(it->first);
      if(first != target.namespace_binding_first_token_starts->end()) {
        if(!scope.namespace_binding_first_token_starts) {
          scope.namespace_binding_first_token_starts.reset(
              new std::map<std::string, std::size_t>());
        }
        std::size_t & destination =
            (*scope.namespace_binding_first_token_starts)[it->first];
        if(destination == 0 || first->second < destination) {
          destination = first->second;
        }
      }
    }
    changed = true;
  }

  for(auto it = target.named_types.begin();
      it != target.named_types.end(); ++it) {
    if(scope.named_types.count(it->first) == 0) {
      scope.named_types[it->first] = it->second;
      changed = true;
    }
  }

  for(std::map<std::string, semantic_model::ValueBinding>::iterator it =
          target.values.begin();
      it != target.values.end(); ++it) {
    if(scope.values.count(it->first) == 0) {
      scope.values[it->first] = it->second;
      changed = true;
    }
  }

  for(std::map<std::string, std::vector<semantic_model::FunctionBinding *> >::iterator it =
          target.function_sets.begin();
      it != target.function_sets.end(); ++it) {
    std::vector<semantic_model::FunctionBinding *> & slot =
        semantic_lookup::direct_function_set_slot(scope, it->first);
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      bool duplicate = std::find(slot.begin(), slot.end(), it->second[i]) != slot.end();
      if(!duplicate) {
        for(std::size_t j = 0; j < slot.size(); ++j) {
          if(cpp_decl::type_equals(slot[j]->type, it->second[i]->type)) {
            duplicate = true;
            break;
          }
        }
      }
      if(!duplicate) {
        slot.push_back(it->second[i]);
        changed = true;
      }
    }
  }

  for(std::map<std::string, semantic_model::ClassTemplateDecl *>::iterator it =
          target.class_templates.begin();
      it != target.class_templates.end(); ++it) {
    if(scope.class_templates.count(it->first) == 0) {
      scope.class_templates[it->first] = it->second;
      changed = true;
    }
  }

  for(std::map<std::string, std::vector<semantic_model::FunctionTemplateDecl *> >::iterator it =
          target.function_templates.begin();
      it != target.function_templates.end(); ++it) {
    std::vector<semantic_model::FunctionTemplateDecl *> & slot =
        semantic_lookup::direct_function_template_slot(scope, it->first);
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      if(std::find(slot.begin(), slot.end(), it->second[i]) == slot.end()) {
        slot.push_back(it->second[i]);
        changed = true;
      }
    }
  }

  for(std::map<std::string, semantic_model::AliasTemplateDecl *>::iterator it =
          target.alias_templates.begin();
      it != target.alias_templates.end(); ++it) {
    if(scope.alias_templates.count(it->first) == 0) {
      scope.alias_templates[it->first] = it->second;
      changed = true;
    }
  }

  for(std::map<std::string, semantic_model::VariableTemplateDecl *>::iterator it =
          target.variable_templates.begin();
      it != target.variable_templates.end(); ++it) {
    if(scope.variable_templates.count(it->first) == 0) {
      scope.variable_templates[it->first] = it->second;
      changed = true;
    }
  }

  changed = add_using_directive_raw(scope, target) || changed;
  if(changed) {
    note_binding_mutation(scope);
  }
}

void bind_value(semantic_model::Scope & scope,
                const std::string & name,
                const semantic_model::ValueBinding & binding)
{
  scope.values[name] = binding;
  note_binding_mutation(scope);
}

void bind_values(semantic_model::Scope & scope,
                 const std::vector<semantic_model::ValueBinding> & bindings)
{
  if(bindings.empty()) {
    return;
  }
  for(std::size_t i = 0; i < bindings.size(); ++i) {
    scope.values[bindings[i].name] = bindings[i];
  }
  note_binding_mutation(scope);
}

void bind_value_aliases(semantic_model::Scope & scope,
                        const std::string & primary_name,
                        const std::string & alias_name,
                        const semantic_model::ValueBinding & binding)
{
  scope.values[primary_name] = binding;
  if(!alias_name.empty() && alias_name != primary_name) {
    scope.values[alias_name] = binding;
  }
  note_binding_mutation(scope);
}

void bind_named_pack_size(semantic_model::Scope & scope,
                          const std::string & name,
                          std::size_t size)
{
  scope.named_pack_sizes[name] = size;
  note_binding_mutation(scope);
}

void bind_value_pack(semantic_model::Scope & scope,
                     const std::string & name,
                     const std::vector<semantic_model::ValueBinding> & bindings)
{
  std::vector<semantic_model::ValueBinding> stored;
  stored.reserve(bindings.size());
  for(std::size_t i = 0; i < bindings.size(); ++i) {
    scope.values[bindings[i].name] = bindings[i];
    stored.push_back(scope.values[bindings[i].name]);
  }
  scope.named_value_packs[name] = stored;
  note_binding_mutation(scope);
}

void bind_class_template(semantic_model::Scope & scope,
                         const std::string & name,
                         semantic_model::ClassTemplateDecl * decl)
{
  scope.class_templates[name] = decl;
  note_binding_mutation(scope);
}

void bind_alias_template(semantic_model::Scope & scope,
                         const std::string & name,
                         semantic_model::AliasTemplateDecl * decl)
{
  scope.alias_templates[name] = decl;
  note_binding_mutation(scope);
}

void bind_variable_template(semantic_model::Scope & scope,
                            const std::string & name,
                            semantic_model::VariableTemplateDecl * decl)
{
  scope.variable_templates[name] = decl;
  note_binding_mutation(scope);
}

void bind_template_template_parameter(semantic_model::Scope & scope,
                                      const std::string & name,
                                      semantic_model::ClassTemplateDecl * decl)
{
  scope.class_templates[name] = decl;
  scope.template_bound_template_names.insert(name);
  note_binding_mutation(scope);
}

void bind_dependent_template_value(semantic_model::Scope & scope,
                                   const std::string & name,
                                   const cpp_decl::TypePtr & type)
{
  semantic_model::ValueBinding binding(semantic_model::ValueBinding::VK_VARIABLE, name, type);
  binding.dependent_template_value = true;
  scope.values[name] = binding;
  scope.template_bound_value_names.insert(name);
  note_binding_mutation(scope);
}

void append_function_bindings(semantic_model::Scope & scope,
                              const std::string & name,
                              const std::vector<semantic_model::FunctionBinding *> & functions,
                              semantic_model::MemberAccess access,
                              std::size_t source_token_start)
{
  if(functions.empty()) {
    return;
  }
  std::vector<semantic_model::FunctionBinding *> & slot =
      semantic_lookup::direct_function_set_slot(scope, name);
  slot.insert(slot.end(), functions.begin(), functions.end());
  if(source_token_start != 0) {
    if(!scope.function_binding_first_token_starts) {
      scope.function_binding_first_token_starts.reset(
          new std::map<
              std::string,
              std::map<const semantic_model::FunctionBinding *, std::size_t> >());
    }
    std::map<const semantic_model::FunctionBinding *, std::size_t> & starts =
        (*scope.function_binding_first_token_starts)[name];
    for(std::size_t i = 0; i < functions.size(); ++i) {
      std::size_t & first = starts[functions[i]];
      if(first == 0 || source_token_start < first) {
        first = source_token_start;
      }
    }
  }
  if(scope.class_info) {
    for(std::size_t i = 0; i < functions.size(); ++i) {
      semantic_lookup::set_direct_function_access_override(scope, name, functions[i], access);
    }
  }
  note_binding_mutation(scope);
}

void append_unique_function_templates(
    semantic_model::Scope & scope,
    const std::string & name,
    const std::vector<semantic_model::FunctionTemplateDecl *> & templates,
    const CppAstNode * introduction_node)
{
  if(templates.empty()) {
    return;
  }
  std::vector<semantic_model::FunctionTemplateDecl *> & slot =
      semantic_lookup::direct_function_template_slot(scope, name);
  bool changed = false;
  for(std::size_t i = 0; i < templates.size(); ++i) {
    if(std::find(slot.begin(), slot.end(), templates[i]) == slot.end()) {
      slot.push_back(templates[i]);
      changed = true;
    }
    if(introduction_node) {
      if(!scope.function_template_introduction_nodes) {
        scope.function_template_introduction_nodes.reset(
            new std::map<
                std::string,
                std::map<const semantic_model::FunctionTemplateDecl *,
                         const CppAstNode *> >());
      }
      std::map<const semantic_model::FunctionTemplateDecl *,
               const CppAstNode *> & introductions =
          (*scope.function_template_introduction_nodes)[name];
      // Semantic collection visits declarations in source order. Keep the
      // first using-declaration so a later repeated import cannot move the
      // template's visibility point forward.
      introductions.insert(std::make_pair(templates[i], introduction_node));
    }
  }
  if(changed) {
    note_binding_mutation(scope);
  }
}

}  // namespace semantic_scope_mutation
