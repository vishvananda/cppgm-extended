#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "recog_token_buffer.h"
#include "semantic_model.h"
#include "template_api.h"
#include "template_model.h"
#include "witness_api.h"

class SemanticContext;

namespace callsemantic {

struct ClassTemplateReferenceCallbacks
{
  std::function<std::string(std::size_t)> source_location_for_token_index;
  std::function<bool(const std::string &, const std::string &, std::size_t &)>
      token_index_for_source_location;
  std::function<bool(std::size_t, const std::string &, std::size_t &)>
      find_next_token_source_on_same_line;
  std::function<bool(std::size_t, std::vector<std::pair<std::size_t, std::size_t> > &)>
      template_argument_token_ranges_from_open;
  std::function<bool(const std::string &)> template_id_at_location_is_nested;
  std::function<bool(const std::string &)>
      template_id_at_location_is_qualified_member_owner;
  std::function<bool(const std::string &)>
      template_id_at_location_is_conversion_operator_result;
  std::function<const cpp_decl::TemplateIdSyntax *(
      const std::string &,
      const std::string &)> template_id_syntax_at_location;
  std::function<const RecogToken &(std::size_t)> peek_token;
  std::function<void(semantic_model::ClassTemplateDecl &,
                     const template_api::ClassSpecializationSelection &)>
      record_selected_class_template_base_source_uses;
  std::function<void(semantic_model::Scope &,
                     const std::vector<cpp_decl::TemplateArgumentSyntax> &,
                     witness::SourceUseOwnership,
                     const std::string &)>
      emit_nested_class_use_source_events_from_syntaxes;
  std::function<bool()> class_template_declarations_complete;
};

std::vector<std::string> class_template_argument_source_locations_for_current_use(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    const std::string & template_name,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & arg_texts);

semantic_model::ClassInfo * reference_class_template_instantiation(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<std::string> & arg_texts);

semantic_model::ClassInfo * reference_class_template_instantiation_with_syntax(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<std::string> & arg_texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes,
    template_api::ClassTemplateSourceUseMode source_use_mode =
        template_api::ClassTemplateSourceUseMode::EmitClassUse);

semantic_model::ClassInfo * reference_selected_class_template_instantiation(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization,
    const std::vector<std::string> * source_arg_texts = nullptr,
    template_api::ClassTemplateSourceUseMode source_use_mode =
        template_api::ClassTemplateSourceUseMode::EmitClassUse,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * source_arg_syntaxes = nullptr,
    const std::string * precomputed_key = nullptr,
    semantic_model::FunctionBinding * source_function = nullptr);

semantic_model::ClassInfo * reference_selected_class_template_instantiation_with_key(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization,
    const std::vector<std::string> * source_arg_texts,
    const std::string * precomputed_key,
    template_api::ClassTemplateSourceUseMode source_use_mode =
        template_api::ClassTemplateSourceUseMode::EmitClassUse,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * source_arg_syntaxes = nullptr);

semantic_model::ClassInfo * instantiate_class_template_with_syntax(
    SemanticContext & ctx,
    const ClassTemplateReferenceCallbacks & callbacks,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<std::string> & arg_texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * arg_syntaxes = nullptr);

}  // namespace callsemantic
