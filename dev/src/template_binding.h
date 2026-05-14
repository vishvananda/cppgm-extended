#pragma once

#include <map>
#include <functional>
#include <string>
#include <vector>

#include "cpp_decl_model.h"
#include "template_model.h"

namespace template_binding {

struct Hooks
{
  std::function<void(const std::string &, std::size_t)> bind_pack_size;
  std::function<void(const std::string &, const std::vector<cpp_decl::TypePtr> &)> bind_type_pack;
  std::function<void(const std::string &,
                     const cpp_decl::TypePtr &,
                     const std::vector<template_model::TemplateArgument> &)> bind_non_type_pack;
  std::function<void(const std::string &, const cpp_decl::TypePtr &)> bind_named_type;
  std::function<void(const std::string &, const template_model::TemplateArgument &)> bind_template_template;
  std::function<cpp_decl::TypePtr(const template_model::TemplateParameterInfo &,
                                  const template_model::TemplateArgument &)> resolve_non_type_value_type;
  std::function<void(const std::string &,
                     const cpp_decl::TypePtr &,
                     const template_model::TemplateArgument &)> bind_non_type;
};

void bind_arguments(const std::vector<template_model::TemplateParameterInfo> & parameters,
                    const std::vector<template_model::TemplateArgument> & arguments,
                    const Hooks & hooks,
                    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

}  // namespace template_binding
