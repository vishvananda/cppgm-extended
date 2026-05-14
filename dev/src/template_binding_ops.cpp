#include "template_api.h"

#include "template_api_internal.h"
#include "template_instantiation.h"
#include "template_services.h"
#include "template_scope.h"

namespace template_api {
namespace binding {

void bind_named_type(semantic_model::Scope & scope,
                     const std::string & name,
                     const cpp_decl::TypePtr & type)
{
  template_scope::bind_named_type(scope, name, type);
}

void overlay_direct_scope_bindings(semantic_model::Scope & target,
                                   const semantic_model::Scope & source)
{
  template_scope::overlay_scope_bindings(
      target, source, template_scope::OVERLAY_ALL_BINDINGS);
}

void overlay_ancestor_scope_bindings(
    semantic_model::Scope & target,
    const semantic_model::Scope & source,
    const semantic_model::Scope * stop_before)
{
  template_scope::overlay_ancestor_scope_bindings(
      target, source, stop_before, template_scope::OVERLAY_ALL_BINDINGS);
}

void overlay_instantiation_use_scope_bindings(semantic_model::Scope & target,
                                              const semantic_model::Scope & use_scope,
                                              const semantic_model::Scope * declaring_scope)
{
  template_instantiation::overlay_instantiation_use_scope_bindings(
      target, use_scope, declaring_scope);
}

void overlay_instantiation_use_scope_bindings(
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::set<std::string> & excluded_names)
{
  template_instantiation::overlay_instantiation_use_scope_bindings(
      target, use_scope, declaring_scope, excluded_names);
}

void overlay_instantiation_local_named_types(
    SemanticContext & ctx,
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_instantiation::overlay_instantiation_local_named_types(
            services, target, use_scope, declaring_scope, arguments, excluded_names);
      });
}

void bind_template_arguments_into_scope(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_instantiation::bind_template_arguments_into_scope(
            services, scope, parameters, arguments, pack_sizes);
      });
}

semantic_model::Scope & bind_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  return template_api::bind_template_arguments(
      ctx, declaring_scope, parameters, arguments, pack_sizes);
}

semantic_model::Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    semantic_model::ClassInfo * active_owner)
{
  return template_instantiation::bind_template_arguments_for_instantiation(
      ctx,
      declaring_scope,
      use_scope,
      parameters,
      arguments,
      pack_sizes,
      active_owner);
}

semantic_model::Scope & bind_class_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  return template_api::bind_class_template_arguments_for_instantiation(
      ctx, declaring_scope, use_scope, parameters, arguments, pack_sizes);
}

}  // namespace binding
}  // namespace template_api
