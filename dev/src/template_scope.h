#pragma once

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "template_model.h"

namespace template_binding {
struct Hooks;
}

namespace template_scope {

enum OverlayMode
{
  OVERLAY_ALL_BINDINGS,
  OVERLAY_TEMPLATE_BOUND_ONLY
};

void bump_binding_fingerprint_epoch(semantic_model::Scope & scope);

std::size_t scope_binding_fingerprint(const semantic_model::Scope & scope);

std::size_t scope_instance_fingerprint(const semantic_model::Scope & scope);

template_binding::Hooks make_scope_binding_hooks(semantic_model::Scope & scope);

void bind_pack_size(semantic_model::Scope & scope,
                    const std::string & name,
                    std::size_t count);

std::string pack_value_alias_name(const std::string & pack_name,
                                  std::size_t index);

void bind_empty_parameter_pack(
    semantic_model::Scope & scope,
    const template_model::TemplateParameterInfo & parameter);

void bind_template_argument_pack(
    semantic_model::Scope & scope,
    const template_model::TemplateParameterInfo & parameter,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool template_bound = true);

void bind_template_parameter_placeholders(
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters);

void bind_type_pack(semantic_model::Scope & scope,
                    const std::string & name,
                    const std::vector<cpp_decl::TypePtr> & bound_pack);

void bind_named_type(semantic_model::Scope & scope,
                     const std::string & name,
                     const cpp_decl::TypePtr & type);

bool scope_has_type_parameter_pack_name(const semantic_model::Scope & scope,
                                        const std::string & name);

void bind_value(semantic_model::Scope & scope,
                const std::string & name,
                const semantic_model::ValueBinding & binding,
                bool template_bound = false);

void bind_parameter_value(semantic_model::Scope & scope,
                          const std::string & name,
                          const cpp_decl::TypePtr & type);

void bind_parameter_value_pack(semantic_model::Scope & scope,
                               const std::string & name,
                               const std::vector<cpp_decl::TypePtr> & value_types);

void bind_non_type_value(semantic_model::Scope & scope,
                         const std::string & name,
                         const cpp_decl::TypePtr & value_type,
                         long long value,
                         bool dependent,
                         const std::string & text = std::string(),
                         semantic_model::FunctionBinding * function_value = nullptr,
                         const semantic_model::ValueBinding * value_binding = nullptr);

void bind_non_type_pack(semantic_model::Scope & scope,
                        const std::string & name,
                        const cpp_decl::TypePtr & value_type,
                        const std::vector<template_model::TemplateArgument> & bound_pack);

void bind_non_type_value_pack(semantic_model::Scope & scope,
                              const std::string & name,
                              const cpp_decl::TypePtr & value_type,
                              const std::vector<long long> & values,
                              bool pack_template_bound = false);

void bind_value_pack(semantic_model::Scope & scope,
                     const std::string & name,
                     const std::vector<semantic_model::ValueBinding> & bound_pack,
                     bool template_bound = false);

bool erase_template_parameter_binding(semantic_model::Scope & scope,
                                      const std::string & name);

void bind_class_template(semantic_model::Scope & scope,
                         const std::string & name,
                         semantic_model::ClassTemplateDecl * decl);

void bind_alias_template(semantic_model::Scope & scope,
                         const std::string & name,
                         semantic_model::AliasTemplateDecl * decl);

void bind_template_template_placeholder(semantic_model::Scope & scope,
                                        const std::string & name);

void bind_template_template_argument(
    semantic_model::Scope & scope,
    const std::string & name,
    const template_model::TemplateArgument & argument);

void overlay_scope_bindings(semantic_model::Scope & target,
                            const semantic_model::Scope & source,
                            OverlayMode mode);

void overlay_ancestor_scope_bindings(semantic_model::Scope & target,
                                     const semantic_model::Scope & source,
                                     const semantic_model::Scope * stop_before,
                                     OverlayMode mode);

void overlay_ancestor_scope_bindings_excluding_names(
    semantic_model::Scope & target,
    const semantic_model::Scope & source,
    const semantic_model::Scope * stop_before,
    OverlayMode mode,
    const std::set<std::string> & excluded_names);

}  // namespace template_scope
