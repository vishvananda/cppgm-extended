#pragma once

#include "class_template_mangle_info.h"
#include "semantic_model.h"

namespace semantic_model {

inline std::shared_ptr<
    const std::vector<template_model::TemplateParameterInfo> >
retained_class_template_mangle_parameters(const ClassTemplateDecl & decl)
{
  if(!decl.retained_mangle_parameters) {
    decl.retained_mangle_parameters.reset(
        new std::vector<template_model::TemplateParameterInfo>(
            decl.parameters));
  }
  return decl.retained_mangle_parameters;
}

inline const std::vector<template_model::TemplateParameterInfo> *
class_template_mangle_parameters(
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info)
{
  return info.template_parameters.empty() ?
      nullptr :
      &info.template_parameters.const_values();
}

}  // namespace semantic_model
