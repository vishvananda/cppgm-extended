#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "template_model.h"

namespace cpp_decl {

struct ClassTemplateSpecializationMangleInfo
{
  void * class_template_decl = nullptr;
  std::string template_scope_prefix;
  std::string template_name;
  std::vector<template_model::TemplateParameterInfo> template_parameters;
  std::vector<template_model::TemplateParameterInfo> mangle_parameters;
  std::vector<template_model::TemplateArgument> mangle_arguments;
  std::vector<template_model::TemplateArgument> arguments;
  std::vector<TemplateArgumentSyntax> argument_syntaxes;
  std::map<std::string, std::size_t> pack_sizes;
  bool force_structured_mangling = false;
};

inline TypePtr named_mangle_base(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_NAMED ? base : TypePtr();
}

inline void set_named_type_class_template_specialization_mangle_info(
    const TypePtr & type,
    const std::shared_ptr<ClassTemplateSpecializationMangleInfo> & info)
{
  TypePtr base = named_mangle_base(type);
  if(base) {
    base->named_class_template_specialization_mangle_info = info;
  }
}

inline std::shared_ptr<ClassTemplateSpecializationMangleInfo>
named_type_class_template_specialization_mangle_info(const TypePtr & type)
{
  TypePtr base = named_mangle_base(type);
  return base ? base->named_class_template_specialization_mangle_info :
                std::shared_ptr<ClassTemplateSpecializationMangleInfo>();
}

inline std::shared_ptr<const ClassTemplateSpecializationMangleInfo>
named_type_class_template_specialization_mangle_info_const(const TypePtr & type)
{
  return named_type_class_template_specialization_mangle_info(type);
}

}  // namespace cpp_decl
