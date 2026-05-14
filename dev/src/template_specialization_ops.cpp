#include "template_api.h"

#include "template_api_internal.h"
#include "template_services.h"

namespace template_api {
namespace specialization {

ClassSpecializationSelection select_class_specialization(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_api::select_class_specialization(
            services,
            decl,
            template_api::make_template_environment(use_scope),
            key,
            arguments,
            dependent_source_argument_texts);
      });
}

}  // namespace specialization
}  // namespace template_api
