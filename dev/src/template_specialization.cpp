#include "template_specialization.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>

#include "cppast_ast.h"
#include "cpp_decl_bridge.h"
#include "parser_trace.h"
#include "semantic_lookup.h"
#include "semantic_conversion.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_utils.h"
#include "template_api_internal.h"
#include "template_argument_semantics.h"
#include "template_binding.h"
#include "template_metadata.h"
#include "template_services.h"
#include "template_scope.h"
#include "witness_api.h"

namespace template_specialization {

using namespace cpp_decl;
using namespace semantic_conversion;
using namespace semantic_model;
using namespace template_model;

namespace {

struct DeducedState
{
  std::map<std::string, TypePtr> types;
  std::map<std::string, std::vector<TypePtr> > type_packs;
  std::map<std::string, long long> values;
  std::map<std::string, TemplateArgument> value_arguments;
  std::map<std::string, std::vector<long long> > value_packs;
  std::map<std::string, ClassTemplateDecl *> class_templates;
  std::map<std::string, AliasTemplateDecl *> alias_templates;
};

bool store_deduced_type(DeducedState & deduced,
                        const std::string & parameter_name,
                        const TypePtr & type)
{
  std::map<std::string, TypePtr>::iterator found =
      deduced.types.find(parameter_name);
  if(found == deduced.types.end()) {
    deduced.types[parameter_name] = type;
    return true;
  }
  return type_equals(found->second, type);
}

bool store_deduced_value(DeducedState & deduced,
                         const std::string & parameter_name,
                         long long value)
{
  std::map<std::string, long long>::iterator found =
      deduced.values.find(parameter_name);
  if(found == deduced.values.end()) {
    deduced.values[parameter_name] = value;
    return true;
  }
  return found->second == value;
}

bool non_type_template_argument_values_match(const TemplateArgument & lhs,
                                             const TemplateArgument & rhs)
{
  if(lhs.kind != TemplateArgument::TA_VALUE ||
     rhs.kind != TemplateArgument::TA_VALUE ||
     lhs.dependent != rhs.dependent) {
    return false;
  }
  if(!type_equals(lhs.type, rhs.type)) {
    return false;
  }
  if(lhs.dependent) {
    return lhs.text == rhs.text;
  }

  TypePtr value_base = strip_top_level_cv(remove_reference_type(lhs.type));
  const bool integral_like =
      !value_base ||
      is_integral_type(value_base) ||
      is_bool_type(value_base) ||
      (value_base->kind == Type::TK_NAMED &&
       (value_base->named_key.compare(0, 5, "enum ") == 0 ||
        value_base->named_display.compare(0, 5, "enum ") == 0));
  if(integral_like) {
    return lhs.value == rhs.value;
  }
  if(lhs.function_value || rhs.function_value) {
    return lhs.function_value == rhs.function_value;
  }
  if(lhs.value_binding || rhs.value_binding) {
    return lhs.value_binding == rhs.value_binding;
  }
  if(!lhs.text.empty() || !rhs.text.empty()) {
    return lhs.text == rhs.text;
  }
  return lhs.value == rhs.value;
}

bool store_deduced_value_argument(DeducedState & deduced,
                                  const std::string & parameter_name,
                                  const TemplateArgument & argument,
                                  const TypePtr & value_type)
{
  TemplateArgument stored = argument;
  stored.kind = TemplateArgument::TA_VALUE;
  stored.type = value_type;
  if(!store_deduced_value(deduced, parameter_name, stored.value)) {
    return false;
  }

  std::map<std::string, TemplateArgument>::iterator found =
      deduced.value_arguments.find(parameter_name);
  if(found == deduced.value_arguments.end()) {
    deduced.value_arguments[parameter_name] = stored;
    return true;
  }
  return non_type_template_argument_values_match(found->second, stored);
}

bool store_deduced_type_pack(DeducedState & deduced,
                             const std::string & parameter_name,
                             const std::vector<TypePtr> & values)
{
  std::map<std::string, std::vector<TypePtr> >::iterator found =
      deduced.type_packs.find(parameter_name);
  if(found == deduced.type_packs.end()) {
    deduced.type_packs[parameter_name] = values;
    return true;
  }
  if(found->second.size() != values.size()) {
    return false;
  }
  for(std::size_t i = 0; i < values.size(); ++i) {
    if(!type_equals(found->second[i], values[i])) {
      return false;
    }
  }
  return true;
}

bool store_deduced_value_pack(DeducedState & deduced,
                              const std::string & parameter_name,
                              const std::vector<long long> & values)
{
  std::map<std::string, std::vector<long long> >::iterator found =
      deduced.value_packs.find(parameter_name);
  if(found == deduced.value_packs.end()) {
    deduced.value_packs[parameter_name] = values;
    return true;
  }
  if(found->second.size() != values.size()) {
    return false;
  }
  for(std::size_t i = 0; i < values.size(); ++i) {
    if(found->second[i] != values[i]) {
      return false;
    }
  }
  return true;
}

bool is_bare_template_parameter_type(const TypePtr & type)
{
  return type &&
         type->kind == Type::TK_NAMED &&
         named_type_is_template_parameter(type);
}

const TemplateParameterInfo * type_pattern_template_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base->named_key);
  if(!parameter) {
    parameter = find_template_parameter_by_name(parameters, base->named_key);
  }
  if(!parameter && base->named_display != base->named_key) {
    parameter = find_template_parameter(parameters, base->named_display);
  }
  if(!parameter && base->named_display != base->named_key) {
    parameter = find_template_parameter_by_name(parameters, base->named_display);
  }
  return parameter && parameter->kind == TemplateParameterInfo::TP_TYPE ?
             parameter :
             nullptr;
}

bool deduce_type_pattern_to_state(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern,
    const TypePtr & actual,
    DeducedState & deduced)
{
  TypePtr pattern_cv_inner;
  TypePtr actual_cv_inner;
  bool pattern_const = false;
  bool pattern_volatile = false;
  bool actual_const = false;
  bool actual_volatile = false;
  if(top_level_cv_flags(pattern,
                        pattern_cv_inner,
                        pattern_const,
                        pattern_volatile) &&
     (pattern_const || pattern_volatile)) {
    if(!top_level_cv_flags(actual,
                           actual_cv_inner,
                           actual_const,
                           actual_volatile) ||
       (pattern_const && !actual_const) ||
       (pattern_volatile && !actual_volatile)) {
      return false;
    }
    return deduce_type_pattern_to_state(parameters,
                                        pattern_cv_inner,
                                        actual_cv_inner,
                                        deduced);
  }

  TypePtr pattern_base = strip_top_level_cv(pattern);
  TypePtr actual_base = actual;
  if(!pattern_base || !actual_base) {
    return false;
  }

  if(const TemplateParameterInfo * parameter =
         type_pattern_template_parameter(parameters, pattern_base)) {
    if(parameter->parameter_pack) {
      std::vector<TypePtr> single;
      single.push_back(actual);
      return store_deduced_type_pack(deduced, parameter->name, single);
    }
    return store_deduced_type(deduced, parameter->name, actual);
  }

  if(pattern_base->kind != actual_base->kind) {
    return false;
  }

  switch(pattern_base->kind) {
  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return type_equals(pattern, actual) || type_equals(pattern_base, actual_base);

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return deduce_type_pattern_to_state(parameters,
                                        pattern_base->inner,
                                        actual_base->inner,
                                        deduced);

  case Type::TK_MEMBER_POINTER:
    return deduce_type_pattern_to_state(parameters,
                                        pattern_base->owner,
                                        actual_base->owner,
                                        deduced) &&
           deduce_type_pattern_to_state(parameters,
                                        pattern_base->inner,
                                        actual_base->inner,
                                        deduced);

  case Type::TK_ARRAY:
    if(pattern_base->has_bound) {
      if(!actual_base->has_bound || pattern_base->bound != actual_base->bound) {
        return false;
      }
    } else if(!pattern_base->bound_text.empty()) {
      return false;
    }
    return deduce_type_pattern_to_state(parameters,
                                        pattern_base->inner,
                                        actual_base->inner,
                                        deduced);

  case Type::TK_FUNCTION:
    if(pattern_base->variadic != actual_base->variadic ||
       pattern_base->prototype_relaxed != actual_base->prototype_relaxed ||
       pattern_base->function_const != actual_base->function_const ||
       pattern_base->function_volatile != actual_base->function_volatile ||
       pattern_base->function_ref_qualifier != actual_base->function_ref_qualifier ||
       !deduce_type_pattern_to_state(parameters,
                                     pattern_base->inner,
                                     actual_base->inner,
                                     deduced)) {
      return false;
    }
    if(!pattern_base->params.empty()) {
      const TemplateParameterInfo * trailing_pack =
          type_pattern_template_parameter(parameters, pattern_base->params.back());
      if(trailing_pack && trailing_pack->parameter_pack) {
        const std::size_t fixed_param_count = pattern_base->params.size() - 1;
        if(actual_base->params.size() < fixed_param_count) {
          return false;
        }
        for(std::size_t i = 0; i < fixed_param_count; ++i) {
          if(!deduce_type_pattern_to_state(parameters,
                                           pattern_base->params[i],
                                           actual_base->params[i],
                                           deduced)) {
            return false;
          }
        }
        std::vector<TypePtr> pack_values;
        pack_values.reserve(actual_base->params.size() - fixed_param_count);
        for(std::size_t i = fixed_param_count; i < actual_base->params.size(); ++i) {
          pack_values.push_back(actual_base->params[i]);
        }
        return store_deduced_type_pack(deduced, trailing_pack->name, pack_values);
      }
    }
    if(pattern_base->params.size() != actual_base->params.size()) {
      return false;
    }
    for(std::size_t i = 0; i < pattern_base->params.size(); ++i) {
      if(!deduce_type_pattern_to_state(parameters,
                                       pattern_base->params[i],
                                       actual_base->params[i],
                                       deduced)) {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool type_pattern_has_deducible_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED: {
    if(named_type_is_template_parameter(base)) {
      return true;
    }
    if(base->named_dependent_qualified_leading_typename) {
      return false;
    }
    template_api::TemplateNamedTypeMetadata metadata;
    if(template_api::describe_named_type_metadata(type_system.model,
                                                  base,
                                                  metadata) &&
       metadata.source_template) {
      for(std::size_t i = 0; i < metadata.instantiation_arguments.size(); ++i) {
        const TemplateArgument & argument = metadata.instantiation_arguments[i];
        if(argument.kind == TemplateArgument::TA_TYPE &&
           type_pattern_has_deducible_template_parameter(type_system, argument.type)) {
          return true;
        }
      }
    }
    return false;
  }
  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_pattern_has_deducible_template_parameter(type_system, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_pattern_has_deducible_template_parameter(type_system, base->owner) ||
           type_pattern_has_deducible_template_parameter(type_system, base->inner);

  case Type::TK_FUNCTION:
    if(type_pattern_has_deducible_template_parameter(type_system, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_pattern_has_deducible_template_parameter(type_system, base->params[i])) {
        return true;
      }
    }
    return false;

  case Type::TK_FUNDAMENTAL:
    return false;
  }

  return false;
}

void promote_single_deduced_type_to_pack(DeducedState & deduced,
                                         const std::string & parameter_name)
{
  std::map<std::string, TypePtr>::iterator single_found =
      deduced.types.find(parameter_name);
  if(single_found == deduced.types.end() ||
     deduced.type_packs.find(parameter_name) != deduced.type_packs.end()) {
    return;
  }
  deduced.type_packs[parameter_name].push_back(single_found->second);
  deduced.types.erase(single_found);
}

void promote_single_deduced_value_to_pack(DeducedState & deduced,
                                          const std::string & parameter_name)
{
  std::map<std::string, long long>::iterator single_found =
      deduced.values.find(parameter_name);
  if(single_found == deduced.values.end() ||
     deduced.value_packs.find(parameter_name) != deduced.value_packs.end()) {
    return;
  }
  deduced.value_packs[parameter_name].push_back(single_found->second);
  deduced.values.erase(single_found);
}

TemplateArgument make_deduced_template_template_argument(
    const TemplateParameterInfo & parameter,
    ClassTemplateDecl * class_template,
    AliasTemplateDecl * alias_template)
{
  TemplateArgument arg;
  if(alias_template) {
    arg.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    arg.template_decl = alias_template;
    arg.text = alias_template->name;
  } else {
    arg.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    arg.template_decl = class_template;
    arg.text = class_template ? class_template->name : parameter.name;
  }
  return arg;
}

using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::trim_space;

// template-boundary-audit: begin semantic_service_access
template_api::TemplateTypeSystem & service_type_system(
    template_api::TemplateServices & services)
{
  return services.type_system;
}
// template-boundary-audit: end semantic_service_access

// template-boundary-audit: begin text_recovery_bridge
std::string specialization_argument_type_text(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(
      type_system, type);
}

// template-boundary-audit: end text_recovery_bridge

std::vector<std::string> source_argument_texts_for_occurrence(
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes)
{
  std::vector<std::string> out = arg_texts;
  if(!arg_syntaxes) {
    return out;
  }
  const std::size_t limit = std::min(out.size(), arg_syntaxes->size());
  for(std::size_t i = 0; i < limit; ++i) {
    std::string text = trim_space((*arg_syntaxes)[i].text);
    if(text.empty()) {
      continue;
    }
    if((*arg_syntaxes)[i].pack_expansion &&
       (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
      text += "...";
    }
    out[i] = text;
  }
  return out;
}

std::string alias_pattern_source_location(
    const template_api::TemplateWitnessContext & ctx,
    const TemplateArgumentSyntax * pattern_syntax)
{
  if(!pattern_syntax) {
    return std::string();
  }
  if(pattern_syntax->has_source_token_start) {
    return template_api::normalize_template_witness_source_location(
        template_api::template_witness_detail::source_location_for_token_index(
          ctx,
          pattern_syntax->source_token_start));
  }
  if(!pattern_syntax->type_id) {
    return std::string();
  }
  return template_api::normalize_template_witness_source_location(
      template_api::template_witness_detail::source_location_for_token_index(
          ctx,
          pattern_syntax->type_id->token_start));
}

std::string alias_template_decl_location(
    const template_api::TemplateWitnessContext & ctx,
    const AliasTemplateDecl & alias_template)
{
  std::string location = source_decl_anchor_location(alias_template.declaration_anchor);
  if(location.empty() && alias_template.type_id) {
    location = template_api::template_witness_detail::source_location_for_token_index(
        ctx,
        alias_template.type_id->token_start);
  }
  return template_api::normalize_template_witness_source_location(location);
}

bool compact_call_space_before(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) ||
         ch == '_' ||
         ch == '>' ||
         ch == ')';
}

std::string compact_source_template_argument_text(const std::string & text)
{
  std::string out;
  const std::string trimmed = trim_space(text);
  out.reserve(trimmed.size());
  for(std::size_t i = 0; i < trimmed.size(); ++i) {
    const char ch = trimmed[i];
    if(ch == '(' && out.size() >= 2 && out[out.size() - 1] == ' ' &&
       compact_call_space_before(out[out.size() - 2])) {
      out.erase(out.size() - 1);
    }
    if(ch == ')' && !out.empty() && out[out.size() - 1] == ' ') {
      out.erase(out.size() - 1);
    }
    out.push_back(ch);
  }
  return out;
}

void append_alias_pattern_source_bindings(
    template_api::TemplateServices & services,
    std::vector<template_api::TemplateWitnessSourceBinding> & out,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto argument_text =
      [&type_system](const TemplateArgument & argument) -> std::string
  {
    return template_model::template_argument_text(
        argument,
        [&type_system](const TypePtr & type)
        {
          return specialization_argument_type_text(type_system, type);
        });
  };
  std::size_t arg_index = 0;
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    template_api::TemplateWitnessSourceBinding binding;
    binding.param = parameters[i].name.empty() ?
        std::string("$") + std::to_string(i + 1) :
        parameters[i].name;
    binding.source = "explicit";
    binding.type_like = parameters[i].kind == TemplateParameterInfo::TP_TYPE;
    if(parameters[i].parameter_pack) {
      binding.pack_binding = true;
      std::size_t trailing_non_pack = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      if(arguments.size() < arg_index + trailing_non_pack) {
        break;
      }
      const std::size_t pack_end = arguments.size() - trailing_non_pack;
      const std::size_t pack_count = pack_end - arg_index;
      const std::size_t remaining_explicit =
          explicit_argument_texts.size() > explicit_index ?
              explicit_argument_texts.size() - explicit_index :
              0;
      const std::size_t explicit_pack_count =
          std::min(pack_count,
                   remaining_explicit > trailing_non_pack ?
                       remaining_explicit - trailing_non_pack :
                       0);
      if(pack_count == 0) {
        binding.arg = "<>";
      } else if(explicit_pack_count == 1) {
        const TemplateArgument & explicit_argument = arguments[arg_index];
        const std::string element_text =
            explicit_argument.kind == TemplateArgument::TA_VALUE ?
                argument_text(explicit_argument) :
                compact_source_template_argument_text(
                    explicit_argument_texts[explicit_index]);
        binding.arg = element_text;
        binding.pack_arguments.push_back(element_text);
      } else if(pack_count == 1) {
        const std::string element_text = argument_text(arguments[arg_index]);
        binding.arg = element_text;
        binding.pack_arguments.push_back(element_text);
      } else {
        binding.pack_aggregate = true;
        std::ostringstream text;
        text << "<";
        for(std::size_t j = arg_index; j < pack_end; ++j) {
          if(j != arg_index) {
            text << ", ";
          }
          const std::size_t explicit_offset = j - arg_index;
          std::string element_text;
          if(explicit_offset < explicit_pack_count) {
            element_text = arguments[j].kind == TemplateArgument::TA_VALUE ?
                argument_text(arguments[j]) :
                compact_source_template_argument_text(
                    explicit_argument_texts[explicit_index + explicit_offset]);
          } else {
            element_text = argument_text(arguments[j]);
          }
          binding.pack_arguments.push_back(element_text);
          text << element_text;
        }
        text << ">";
        binding.arg = text.str();
      }
      out.push_back(binding);
      arg_index = pack_end;
      explicit_index += explicit_pack_count;
      continue;
    }
    if(arg_index >= arguments.size()) {
      break;
    }
    if(explicit_index < explicit_argument_texts.size()) {
      binding.arg = arguments[arg_index].kind == TemplateArgument::TA_VALUE ?
          argument_text(arguments[arg_index]) :
          compact_source_template_argument_text(
              explicit_argument_texts[explicit_index]);
      ++explicit_index;
    } else {
      binding.arg = argument_text(arguments[arg_index]);
    }
    out.push_back(binding);
    ++arg_index;
  }
}

void append_alias_pattern_source_bindings_from_texts(
    std::vector<template_api::TemplateWitnessSourceBinding> & out,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & explicit_argument_texts)
{
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    template_api::TemplateWitnessSourceBinding binding;
    binding.param = parameters[i].name.empty() ?
        std::string("$") + std::to_string(i + 1) :
        parameters[i].name;
    binding.source = "explicit";
    binding.type_like = parameters[i].kind == TemplateParameterInfo::TP_TYPE;
    if(parameters[i].parameter_pack) {
      binding.pack_binding = true;
      std::size_t trailing_non_pack = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      const std::size_t remaining_explicit =
          explicit_argument_texts.size() > explicit_index ?
              explicit_argument_texts.size() - explicit_index :
              0;
      const std::size_t pack_count =
          remaining_explicit > trailing_non_pack ?
              remaining_explicit - trailing_non_pack :
              0;
      if(pack_count == 0) {
        binding.arg = "<>";
      } else if(pack_count == 1) {
        const std::string element_text = compact_source_template_argument_text(
            explicit_argument_texts[explicit_index]);
        binding.arg = element_text;
        binding.pack_arguments.push_back(element_text);
      } else {
        binding.pack_aggregate = true;
        std::ostringstream text;
        text << "<";
        for(std::size_t j = 0; j < pack_count; ++j) {
          if(j != 0) {
            text << ", ";
          }
          const std::string element_text =
              compact_source_template_argument_text(
                  explicit_argument_texts[explicit_index + j]);
          binding.pack_arguments.push_back(element_text);
          text << element_text;
        }
        text << ">";
        binding.arg = text.str();
      }
      out.push_back(binding);
      explicit_index += pack_count;
      continue;
    }
    if(explicit_index >= explicit_argument_texts.size()) {
      break;
    }
    binding.arg = compact_source_template_argument_text(
        explicit_argument_texts[explicit_index]);
    out.push_back(binding);
    ++explicit_index;
  }
}

void record_alias_pattern_source_use(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const AliasTemplateDecl & alias_template,
    const QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    const TemplateArgumentSyntax * pattern_syntax,
    const std::string & expanded_text)
{
  if(!witness::source_capture_enabled(services.witness_context) ||
     qualified.name != alias_template.name) {
    return;
  }
  const std::string use_location =
      alias_pattern_source_location(services.witness_context, pattern_syntax);
  if(use_location.empty()) {
    return;
  }

  std::vector<TemplateArgument> arguments;
  const std::vector<TemplateArgumentSyntax> * arg_syntaxes =
      pattern_syntax && pattern_syntax->template_id ?
          &pattern_syntax->template_id->argument_syntaxes :
          nullptr;
  if(!template_api::resolve_template_arguments(
         services,
         match_scope,
         alias_template.parameters,
         arg_texts,
         arg_syntaxes,
         arguments,
         alias_template.declaring_scope ?
             template_api::make_template_environment(*alias_template.declaring_scope) :
             template_api::TemplateEnvironmentHandle())) {
    return;
  }

  std::vector<std::string> source_arg_texts =
      source_argument_texts_for_occurrence(arg_texts, arg_syntaxes);
  template_argument_semantics::canonicalize_alias_template_source_argument_texts(
      alias_template.parameters,
      source_arg_texts);
  witness::AliasUseEmitRequest request;
  request.use_location = use_location;
  request.template_id_occurrence =
      witness::make_source_template_id_occurrence(
          use_location,
          source_arg_texts);
  request.template_name =
      template_api::alias_template_witness_entity(&alias_template);
  request.origin = witness::AliasUseEmissionOrigin::PatternTemplateId;
  request.selected_decl_location =
      alias_template_decl_location(services.witness_context, alias_template);
  request.selected_decl_has_name_location =
      source_decl_anchor_has_name_location(alias_template.declaration_anchor);
  request.expanded_to = expanded_text;
  append_alias_pattern_source_bindings(services,
                                       request.bindings,
                                       alias_template.parameters,
                                       arguments,
                                       source_arg_texts);
  witness::emit_alias_use(services.witness_context, request);
}

void record_alias_pattern_source_use_from_texts(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    const TemplateArgumentSyntax * pattern_syntax)
{
  if(!witness::source_capture_enabled(services.witness_context)) {
    return;
  }
  Scope & scope = match_scope.require();
  AliasTemplateDecl * alias_template =
      template_argument_semantics::lookup_alias_template(
          services, scope, template_api::qualified_name_text(qualified));
  if(!alias_template ||
     qualified.name != alias_template->name) {
    return;
  }
  const std::string use_location =
      alias_pattern_source_location(services.witness_context, pattern_syntax);
  if(use_location.empty()) {
    return;
  }

  witness::AliasUseEmitRequest request;
  const std::vector<TemplateArgumentSyntax> * arg_syntaxes =
      pattern_syntax && pattern_syntax->template_id ?
          &pattern_syntax->template_id->argument_syntaxes :
          nullptr;
  request.use_location = use_location;
  std::vector<std::string> source_arg_texts =
      source_argument_texts_for_occurrence(arg_texts, arg_syntaxes);
  template_argument_semantics::canonicalize_alias_template_source_argument_texts(
      alias_template->parameters,
      source_arg_texts);
  request.template_id_occurrence =
      witness::make_source_template_id_occurrence(
          use_location,
          source_arg_texts);
  request.template_name =
      template_api::alias_template_witness_entity(alias_template);
  request.origin = witness::AliasUseEmissionOrigin::PatternTemplateId;
  request.selected_decl_location =
      alias_template_decl_location(services.witness_context, *alias_template);
  request.selected_decl_has_name_location =
      source_decl_anchor_has_name_location(alias_template->declaration_anchor);
  append_alias_pattern_source_bindings_from_texts(request.bindings,
                                                  alias_template->parameters,
                                                  source_arg_texts);
  witness::emit_alias_use(services.witness_context, request);
}

template <typename PartialDecl>
bool deduce_from_function_type_pattern(template_api::TemplateServices & services,
                                       const PartialDecl & partial,
                                       DeducedState & deduced,
                                       const TypePtr & pattern_type,
                                       const TemplateArgumentSyntax * pattern_syntax,
                                       const TypePtr & actual_type);

bool try_expand_alias_template_pattern_structurally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const AliasTemplateDecl & alias_template,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    std::string & expanded_text,
    TypePtr * expanded_type = nullptr,
    bool allow_dependent_expansion = false,
    AliasSubstitutionFailure * substitution_failure = nullptr);

bool expand_alias_template_pattern_id_impl(template_api::TemplateServices & services,
                                           template_api::TemplateEnvironmentHandle match_scope,
                                           const std::string & pattern_text,
                                           const QualifiedName & qualified,
                                           const std::vector<std::string> & arg_texts,
                                           const TemplateArgumentSyntax * pattern_syntax,
                                           template_api::TemplateEnvironmentHandle argument_scope,
                                           std::string & expanded_text,
                                           bool allow_dependent_expansion = false,
                                           AliasSubstitutionFailure * substitution_failure = nullptr);

template <typename PartialDecl>
bool deduce_from_named_template_id_text(template_api::TemplateServices & services,
                                        const PartialDecl & partial,
                                        DeducedState & deduced,
                                        Scope & match_scope,
                                        const std::string & pattern_text,
                                        const TemplateArgumentSyntax * pattern_syntax,
                                        const TypePtr & actual_type,
                                        bool structural_only = false);

std::string join_arg_texts(const std::vector<std::string> & items)
{
  std::string out;
  for(std::size_t i = 0; i < items.size(); ++i) {
    if(i != 0) {
      out += "|";
    }
    out += items[i];
  }
  return out;
}

std::string join_display_template_arguments(const std::vector<std::string> & items)
{
  std::string out;
  for(std::size_t i = 0; i < items.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += items[i];
  }
  return out;
}

std::set<std::string> collect_template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::set<std::string> names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(!parameter.name.empty()) {
      names.insert(parameter.name);
    }
    for(std::size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(!parameter.alternate_names[j].empty()) {
        names.insert(parameter.alternate_names[j]);
      }
    }
  }
  return names;
}

void erase_template_parameter_names(
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    template_scope::erase_template_parameter_binding(scope, parameter.name);
    for(std::size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      template_scope::erase_template_parameter_binding(
          scope,
          parameter.alternate_names[j]);
    }
  }
}

bool alias_owner_is_primary_template_instantiation(const ClassInfo & owner)
{
  if(!owner.source_template) {
    return false;
  }
  return !owner.source_template->class_node ||
         !owner.template_output_node ||
         owner.template_output_node == owner.source_template->class_node;
}

bool prepare_alias_body_scope(
    template_api::TemplateServices & services,
    const AliasTemplateDecl & alias_template,
    template_api::TemplateEnvironmentHandle use_scope,
    const std::vector<TemplateArgument> & arguments,
    Scope & body_scope)
{
  if(!alias_template.declaring_scope) {
    return false;
  }

  body_scope = Scope(alias_template.declaring_scope, std::string(), false);

  const std::set<std::string> alias_parameter_names =
      collect_template_parameter_names(alias_template.parameters);
  template_api::overlay_instantiation_use_scope_bindings(
      body_scope,
      use_scope.require(),
      alias_template.declaring_scope,
      alias_parameter_names);
  template_api::overlay_instantiation_local_named_types(
      services,
      body_scope,
      use_scope.require(),
      alias_template.declaring_scope,
      arguments,
      &alias_parameter_names);

  if(alias_template.declaring_scope->class_info) {
    ClassInfo & owner = *alias_template.declaring_scope->class_info;
    if(owner.source_template) {
      erase_template_parameter_names(body_scope,
                                     owner.source_template->parameters);
      if(alias_owner_is_primary_template_instantiation(owner) &&
         !owner.instantiation_arguments.empty()) {
        template_api::bind_template_arguments_into_scope(
            services,
            body_scope,
            owner.source_template->parameters,
            owner.instantiation_arguments);
      }
    }
  }

  template_api::bind_template_arguments_into_scope(
      services,
      body_scope,
      alias_template.parameters,
      arguments);
  return true;
}

bool parse_template_argument_type_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateArgumentSyntax * syntax,
    TypePtr & out,
    bool reference_class_templates_only)
{
  out.reset();
  if(!syntax) {
    return false;
  }
  if(syntax->resolved_type) {
    out = syntax->resolved_type;
    return true;
  }
  const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
      function_call_source_capture_pause;
  if(syntax->type_id &&
     template_argument_semantics::parse_type_id_node_for_templates(
         services, scope, *syntax->type_id, out, reference_class_templates_only) &&
     out) {
    return true;
  }
  if(syntax->template_id &&
     template_argument_semantics::resolve_template_id_syntax_type(
         services,
         scope,
         *syntax->template_id,
         reference_class_templates_only,
         std::string(),
         out,
         template_api::make_template_environment(scope)) &&
     out) {
    return true;
  }
  const std::string text = trim_space(syntax->text);
  if(!text.empty() &&
     text.find('<') == std::string::npos) {
    QualifiedName qualified;
    const std::string lookup_text = strip_elaborated_type_prefix(text);
    if(semantic_utils::split_qualified_name_text(lookup_text, qualified) &&
       !qualified.name.empty() &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      template_api::TemplateTypeLookupRequest request;
      request.scope = &scope;
      request.name = qualified;
      request.allow_class_templates = reference_class_templates_only;
      if(service_type_system(services).resolve_direct_type_lookup(request, out) && out) {
        return true;
      }
    }
  }
  return false;
}

bool parsed_pattern_type_allows_direct_function_fallback(const TypePtr & pattern_type)
{
  if(!pattern_type) {
    return true;
  }
  TypePtr pattern_base = strip_top_level_cv(pattern_type);
  return pattern_base && pattern_base->kind == Type::TK_FUNCTION;
}

bool strip_trailing_pack_ellipsis(const std::string & text, std::string & element_text)
{
  const std::string trimmed = trim_space(text);
  if(trimmed.size() < 4 || trimmed.compare(trimmed.size() - 3, 3, "...") != 0) {
    return false;
  }
  element_text = trim_space(trimmed.substr(0, trimmed.size() - 3));
  return !element_text.empty();
}

bool ast_contains_kind(const CppAstNode & node, CppAstKind kind)
{
  if(node.kind == kind) {
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_contains_kind(node.children[i], kind)) {
      return true;
    }
  }
  return false;
}

const CppAstNode * direct_function_parameter_clause_from_type_syntax(
    const TemplateArgumentSyntax * syntax)
{
  if(!syntax ||
     !syntax->type_id ||
     syntax->type_id->kind != CppAstKind::type_id) {
    return nullptr;
  }
  const CppAstNode * declarator =
      find_child(*syntax->type_id, CppAstKind::abstract_declarator);
  if(!declarator) {
    return nullptr;
  }
  return find_child(*declarator, CppAstKind::parameter_clause);
}

bool function_type_syntax_trailing_parameter_is_pack(
    const TemplateArgumentSyntax * syntax)
{
  const CppAstNode * clause = direct_function_parameter_clause_from_type_syntax(syntax);
  if(!clause || clause->children.empty()) {
    return false;
  }
  const CppAstNode & trailing = clause->children.back();
  if(trailing.kind != CppAstKind::parameter_declaration) {
    return false;
  }
  return ast_contains_kind(trailing, CppAstKind::parameter_pack) ||
         ast_contains_kind(trailing, CppAstKind::ellipsis);
}

void collect_type_parameter_pack_patterns_from_type(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type,
    std::vector<const TemplateParameterInfo *> & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
    if(named_type_is_template_parameter(base)) {
      const TemplateParameterInfo * parameter =
          find_template_parameter(parameters, base->named_semantic_payload);
      if(!parameter) {
        parameter = find_template_parameter(parameters, base->named_display);
      }
      if(parameter &&
         parameter->kind == TemplateParameterInfo::TP_TYPE &&
         parameter->parameter_pack) {
        for(std::size_t i = 0; i < out.size(); ++i) {
          if(out[i]->name == parameter->name) {
            return;
          }
        }
        out.push_back(parameter);
      }
    }
    return;

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    collect_type_parameter_pack_patterns_from_type(parameters, base->inner, out);
    return;

  case Type::TK_MEMBER_POINTER:
    collect_type_parameter_pack_patterns_from_type(parameters, base->owner, out);
    collect_type_parameter_pack_patterns_from_type(parameters, base->inner, out);
    return;

  case Type::TK_FUNCTION:
    collect_type_parameter_pack_patterns_from_type(parameters, base->inner, out);
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      collect_type_parameter_pack_patterns_from_type(parameters, base->params[i], out);
    }
    return;

  case Type::TK_FUNDAMENTAL:
    return;
  }
}

const TemplateParameterInfo * unique_type_parameter_pack_pattern_from_type(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  std::vector<const TemplateParameterInfo *> matches;
  collect_type_parameter_pack_patterns_from_type(parameters, type, matches);
  return matches.size() == 1 ? matches[0] : nullptr;
}

template <typename PartialDecl>
bool deduce_from_function_type_pattern(template_api::TemplateServices & services,
                                       const PartialDecl & partial,
                                       DeducedState & deduced,
                                       const TypePtr & pattern_type,
                                       const TemplateArgumentSyntax * pattern_syntax,
                                       const TypePtr & actual_type)
{
  if(!partial.pattern_scope || !actual_type) {
    return false;
  }

  TypePtr pattern_base = strip_top_level_cv(pattern_type);
  TypePtr actual_base = strip_top_level_cv(actual_type);
  if(!pattern_base ||
     pattern_base->kind != Type::TK_FUNCTION ||
     !actual_base ||
     actual_base->kind != Type::TK_FUNCTION) {
    return false;
  }

  if(pattern_base->variadic != actual_base->variadic ||
     pattern_base->prototype_relaxed != actual_base->prototype_relaxed ||
     pattern_base->function_const != actual_base->function_const ||
     pattern_base->function_volatile != actual_base->function_volatile ||
     pattern_base->function_ref_qualifier != actual_base->function_ref_qualifier ||
     pattern_base->params.empty()) {
    return false;
  }

  if(!function_type_syntax_trailing_parameter_is_pack(pattern_syntax)) {
    return false;
  }

  const TemplateParameterInfo * pack_parameter =
      unique_type_parameter_pack_pattern_from_type(partial.parameters,
                                                   pattern_base->params.back());
  if(!pack_parameter || pack_parameter->name.empty()) {
    return false;
  }

  const std::size_t fixed_argument_count = pattern_base->params.size() - 1;
  if(actual_base->params.size() < fixed_argument_count) {
    return false;
  }

  {
    Scope match_scope =
        make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced);
    if(!template_api::deduce_template_argument(
           services,
           partial.parameters,
           pattern_base->inner,
           actual_base->inner,
           deduced.types,
           template_api::make_template_environment(match_scope),
           true,
           template_api::TemplateEnvironmentHandle())) {
      return false;
    }
  }

  for(std::size_t i = 0; i < fixed_argument_count; ++i) {
    Scope match_scope =
        make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced);
    if(!template_api::deduce_template_argument(
           services,
           partial.parameters,
           pattern_base->params[i],
           actual_base->params[i],
           deduced.types,
           template_api::make_template_environment(match_scope),
           true,
           template_api::TemplateEnvironmentHandle())) {
      return false;
    }
  }

  for(std::size_t i = fixed_argument_count; i < actual_base->params.size(); ++i) {
    std::map<std::string, TypePtr> element_deduced = deduced.types;
    DeducedState match_deduced = deduced;
    for(std::size_t j = 0; j < partial.parameters.size(); ++j) {
      const TemplateParameterInfo & parameter = partial.parameters[j];
      if(parameter.parameter_pack && parameter.kind == TemplateParameterInfo::TP_TYPE) {
        match_deduced.type_packs.erase(parameter.name);
      }
    }
    Scope match_scope =
        make_partial_match_scope(partial.parameters, *partial.pattern_scope, match_deduced);
    if(!template_api::deduce_template_argument(
           services,
           partial.parameters,
           pattern_base->params.back(),
           actual_base->params[i],
           element_deduced,
           template_api::make_template_environment(match_scope),
           true,
           template_api::TemplateEnvironmentHandle())) {
      return false;
    }

    bool saw_pack_deduction = false;
    for(std::size_t j = 0; j < partial.parameters.size(); ++j) {
      const TemplateParameterInfo & parameter = partial.parameters[j];
      if(parameter.name != pack_parameter->name) {
        continue;
      }
      std::map<std::string, TypePtr>::iterator found = element_deduced.find(parameter.name);
      if(found == element_deduced.end()) {
        continue;
      }
      deduced.type_packs[parameter.name].push_back(found->second);
      element_deduced.erase(found);
      saw_pack_deduction = true;
    }
    if(!saw_pack_deduction) {
      return false;
    }

    for(std::map<std::string, TypePtr>::const_iterator it = element_deduced.begin();
        it != element_deduced.end();
        ++it) {
      if(!store_deduced_type(deduced, it->first, it->second)) {
        return false;
      }
    }
  }

  return true;
}

bool partial_specialization_top_cv_matches(const TypePtr & pattern, const TypePtr & actual)
{
  TypePtr pattern_base;
  TypePtr actual_base;
  bool pattern_const = false;
  bool pattern_volatile = false;
  bool actual_const = false;
  bool actual_volatile = false;
  if(!top_level_cv_flags(pattern, pattern_base, pattern_const, pattern_volatile) ||
     !top_level_cv_flags(actual, actual_base, actual_const, actual_volatile)) {
    return false;
  }
  if(!pattern_const && !pattern_volatile) {
    return true;
  }
  return pattern_const == actual_const && pattern_volatile == actual_volatile;
}

bool template_names_match(const QualifiedName & pattern_name,
                          const QualifiedName & actual_name)
{
  if(pattern_name.name != actual_name.name) {
    return false;
  }
  if(pattern_name.rooted) {
    return actual_name.rooted && pattern_name.qualifiers == actual_name.qualifiers;
  }
  if(pattern_name.qualifiers.empty()) {
    return true;
  }
  return !actual_name.rooted && pattern_name.qualifiers == actual_name.qualifiers;
}

QualifiedName qualify_relative_template_name(const Scope & scope,
                                             const QualifiedName & name)
{
  if(name.rooted || name.qualifiers.empty()) {
    return name;
  }

  QualifiedName scoped =
      semantic_lookup::scope_qualified_name_syntax(scope, name.qualifiers[0]);
  scoped.qualifiers.push_back(scoped.name);
  for(std::size_t i = 1; i < name.qualifiers.size(); ++i) {
    scoped.qualifiers.push_back(name.qualifiers[i]);
  }
  scoped.name = name.name;
  return scoped;
}

void collect_partial_order_placeholder_type_keys(const TypePtr & type,
                                                 std::set<std::string> & out)
{
  if(!type) {
    return;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
    if(named_type_is_partial_order_placeholder(base)) {
      out.insert(base->named_key);
    }
    return;
  case Type::TK_FUNCTION:
    collect_partial_order_placeholder_type_keys(base->inner, out);
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      collect_partial_order_placeholder_type_keys(base->params[i], out);
    }
    return;
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
  case Type::TK_ATOMIC:
  case Type::TK_CV:
    collect_partial_order_placeholder_type_keys(base->inner, out);
    if(base->owner) {
      collect_partial_order_placeholder_type_keys(base->owner, out);
    }
    return;
  case Type::TK_MEMBER_POINTER:
    collect_partial_order_placeholder_type_keys(base->owner, out);
    collect_partial_order_placeholder_type_keys(base->inner, out);
    return;
  case Type::TK_FUNDAMENTAL:
    return;
  }
}

void collect_partial_order_placeholder_argument_keys(
    const std::vector<TemplateArgument> & arguments,
    std::set<std::string> & out)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    const TemplateArgument & argument = arguments[i];
    if(argument.kind == TemplateArgument::TA_TYPE) {
      collect_partial_order_placeholder_type_keys(argument.type, out);
    } else if(argument.kind == TemplateArgument::TA_VALUE && !argument.dependent) {
      if(argument.value >= 0x100000) {
        out.insert(std::string("partial-order value#") +
                   std::to_string(argument.value));
      }
    }
  }
}

int compare_transformed_partial_argument_placeholder_specificity(
    const std::vector<TemplateArgument> & current_arguments,
    const std::vector<TemplateArgument> & best_arguments)
{
  std::set<std::string> current_keys;
  std::set<std::string> best_keys;
  collect_partial_order_placeholder_argument_keys(current_arguments, current_keys);
  collect_partial_order_placeholder_argument_keys(best_arguments, best_keys);
  if(current_keys.size() < best_keys.size()) {
    return -1;
  }
  if(best_keys.size() < current_keys.size()) {
    return 1;
  }
  return 0;
}

struct DirectTemplateParameterPattern
{
  const TemplateParameterInfo * parameter = nullptr;
  bool pack_expansion = false;
};

DirectTemplateParameterPattern find_direct_template_parameter_pattern(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & raw_text)
{
  std::string normalized = strip_elaborated_type_prefix(trim_space(raw_text));
  static const char * prefixes[] = {
      "template-parameter ",
      "type-parameter ",
      "dependent type ",
      "dependent alias ",
      "dependent value "
  };
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const std::string prefix(prefixes[i]);
    if(normalized.compare(0, prefix.size(), prefix) == 0) {
      normalized.erase(0, prefix.size());
      break;
    }
  }

  DirectTemplateParameterPattern out;
  if(normalized.size() >= 3 &&
     normalized.compare(normalized.size() - 3, 3, "...") == 0) {
    out.pack_expansion = true;
    normalized.erase(normalized.size() - 3);
    normalized = trim_space(normalized);
  }

  out.parameter = find_template_parameter_by_name(parameters, normalized);
  return out;
}

bool direct_template_parameter_name_from_syntax(
    const TemplateArgumentSyntax & syntax,
    std::string & out)
{
  out.clear();
  if(syntax.pack_expansion) {
    return false;
  }

  if(syntax.type_id &&
     syntax.type_id->kind == CppAstKind::type_id &&
     syntax.type_id->children.size() == 1 &&
     syntax.type_id->children[0].kind == CppAstKind::type_specifier_seq &&
     syntax.type_id->children[0].children.size() == 1 &&
     syntax.type_id->children[0].children[0].kind == CppAstKind::type_name) {
    out = syntax.type_id->children[0].children[0].value;
    return !out.empty();
  }

  if(syntax.expression &&
     syntax.expression->kind == CppAstKind::id_expression &&
     !syntax.expression->value.empty() &&
     syntax.expression->children.empty()) {
    out = syntax.expression->value;
    return true;
  }

  return false;
}

struct DirectTemplateParameterConstraintSet
{
  std::set<std::pair<std::size_t, std::size_t> > repeated_argument_positions;
};

bool partial_constraints_include(
    const DirectTemplateParameterConstraintSet & lhs,
    const DirectTemplateParameterConstraintSet & rhs)
{
  std::set<std::pair<std::size_t, std::size_t> >::const_iterator it =
      rhs.repeated_argument_positions.begin();
  for(; it != rhs.repeated_argument_positions.end(); ++it) {
    if(lhs.repeated_argument_positions.count(*it) == 0) {
      return false;
    }
  }
  return true;
}

template <typename PartialDecl>
DirectTemplateParameterConstraintSet direct_template_parameter_constraints(
    const PartialDecl & partial)
{
  DirectTemplateParameterConstraintSet out;
  std::vector<const TemplateParameterInfo *> direct_parameters(
      partial.arg_texts.size(), nullptr);

  const std::size_t limit =
      std::min(partial.arg_texts.size(), partial.arg_syntaxes.size());
  for(std::size_t i = 0; i < limit; ++i) {
    std::string parameter_name;
    if(!direct_template_parameter_name_from_syntax(partial.arg_syntaxes[i],
                                                   parameter_name)) {
      continue;
    }

    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(partial.parameters, parameter_name);
    if(!parameter || parameter->parameter_pack) {
      continue;
    }
    direct_parameters[i] = parameter;
  }

  for(std::size_t i = 0; i < direct_parameters.size(); ++i) {
    if(!direct_parameters[i]) {
      continue;
    }
    for(std::size_t j = i + 1; j < direct_parameters.size(); ++j) {
      if(direct_parameters[i] == direct_parameters[j]) {
        out.repeated_argument_positions.insert(std::make_pair(i, j));
      }
    }
  }

  return out;
}

template <typename PartialDecl>
int compare_direct_template_parameter_constraint_specificity(
    const PartialDecl & current,
    const PartialDecl & best)
{
  const DirectTemplateParameterConstraintSet current_constraints =
      direct_template_parameter_constraints(current);
  const DirectTemplateParameterConstraintSet best_constraints =
      direct_template_parameter_constraints(best);

  const bool current_includes_best =
      partial_constraints_include(current_constraints, best_constraints);
  const bool best_includes_current =
      partial_constraints_include(best_constraints, current_constraints);

  if(current_includes_best && !best_includes_current) {
    return -1;
  }
  if(best_includes_current && !current_includes_best) {
    return 1;
  }
  return 0;
}

struct TemplateIdHeadPattern
{
  bool valid = false;
  bool direct_template_template_parameter = false;
  bool concrete_template = false;
  std::size_t arity = 0;
};

bool template_id_syntax_head_pattern(
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateIdSyntax & syntax,
    TemplateIdHeadPattern & out)
{
  out = TemplateIdHeadPattern();
  if(syntax.name.name.empty()) {
    return false;
  }

  out.valid = true;
  out.arity = syntax.arguments.size();
  const TemplateParameterInfo * direct_parameter =
      !syntax.name.rooted && syntax.name.qualifiers.empty() ?
          find_template_parameter_by_name(parameters, syntax.name.name) :
          nullptr;
  if(direct_parameter) {
    out.direct_template_template_parameter =
        direct_parameter->kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
        !direct_parameter->parameter_pack;
    out.valid = out.direct_template_template_parameter;
    return out.valid;
  }

  out.concrete_template = true;
  return true;
}

bool template_argument_template_id_head_pattern(
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgumentSyntax * syntax,
    TemplateIdHeadPattern & out)
{
  out = TemplateIdHeadPattern();
  if(!syntax || (!syntax->template_id && !syntax->type_id)) {
    return false;
  }

  if(syntax->template_id &&
     template_id_syntax_head_pattern(parameters, *syntax->template_id, out)) {
    return true;
  }
  if(syntax->type_id) {
    if(const TemplateIdSyntax * type_template_id =
           cppast_template_id_syntax(*syntax->type_id)) {
      if(template_id_syntax_head_pattern(parameters, *type_template_id, out)) {
        return true;
      }
    }
  }

  return false;
}

template <typename PartialDecl>
int compare_template_id_head_specificity(const PartialDecl & current,
                                         const PartialDecl & best)
{
  if(current.arg_syntaxes.empty() || best.arg_syntaxes.empty()) {
    return 0;
  }
  int preference = 0;
  const std::size_t limit =
      std::min(std::min(current.arg_texts.size(), best.arg_texts.size()),
               std::min(current.arg_syntaxes.size(), best.arg_syntaxes.size()));
  for(std::size_t i = 0; i < limit; ++i) {
    TemplateIdHeadPattern current_head;
    TemplateIdHeadPattern best_head;
    if(!template_argument_template_id_head_pattern(current.parameters,
                                                   &current.arg_syntaxes[i],
                                                   current_head) ||
       !template_argument_template_id_head_pattern(best.parameters,
                                                   &best.arg_syntaxes[i],
                                                   best_head) ||
       !current_head.valid ||
       !best_head.valid ||
       current_head.arity != best_head.arity) {
      continue;
    }

    int argument_preference = 0;
    if(current_head.concrete_template &&
       best_head.direct_template_template_parameter) {
      argument_preference = -1;
    } else if(best_head.concrete_template &&
              current_head.direct_template_template_parameter) {
      argument_preference = 1;
    }
    if(argument_preference == 0) {
      continue;
    }
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }

  return preference;
}

struct DirectCvTemplateParameterPattern
{
  bool valid = false;
  int cv_rank = 0;
};

bool direct_cv_template_parameter_pattern(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & raw_text,
    DirectCvTemplateParameterPattern & out)
{
  out = DirectCvTemplateParameterPattern();
  std::string normalized = strip_elaborated_type_prefix(trim_space(raw_text));
  if(normalized.empty()) {
    return false;
  }

  std::vector<std::string> tokens;
  std::string token;
  for(std::size_t i = 0; i <= normalized.size(); ++i) {
    const bool at_end = i == normalized.size();
    const char ch = at_end ? ' ' : normalized[i];
    if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      if(!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
      continue;
    }
    token.push_back(ch);
  }

  std::string parameter_name;
  bool saw_const = false;
  bool saw_volatile = false;
  for(std::size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i] == "const") {
      if(saw_const) {
        return false;
      }
      saw_const = true;
      continue;
    }
    if(tokens[i] == "volatile") {
      if(saw_volatile) {
        return false;
      }
      saw_volatile = true;
      continue;
    }
    if(tokens[i] == "typename") {
      continue;
    }
    if(!parameter_name.empty()) {
      return false;
    }
    parameter_name = tokens[i];
  }

  if(parameter_name.empty() || (!saw_const && !saw_volatile)) {
    return false;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter_by_name(parameters, parameter_name);
  if(!parameter ||
     parameter->kind != TemplateParameterInfo::TP_TYPE ||
     parameter->parameter_pack) {
    return false;
  }

  out.valid = true;
  out.cv_rank = (saw_const ? 1 : 0) + (saw_volatile ? 1 : 0);
  return true;
}

template <typename PartialDecl>
int compare_direct_cv_parameter_template_id_specificity(const PartialDecl & current,
                                                        const PartialDecl & best)
{
  if(current.arg_syntaxes.empty() || best.arg_syntaxes.empty()) {
    return 0;
  }

  int preference = 0;
  const std::size_t limit =
      std::min(std::min(current.arg_texts.size(), best.arg_texts.size()),
               std::min(current.arg_syntaxes.size(), best.arg_syntaxes.size()));
  for(std::size_t i = 0; i < limit; ++i) {
    DirectCvTemplateParameterPattern current_cv;
    DirectCvTemplateParameterPattern best_cv;
    const bool current_is_cv_parameter =
        direct_cv_template_parameter_pattern(current.parameters,
                                             current.arg_texts[i],
                                             current_cv);
    const bool best_is_cv_parameter =
        direct_cv_template_parameter_pattern(best.parameters,
                                             best.arg_texts[i],
                                             best_cv);

    TemplateIdHeadPattern current_head;
    TemplateIdHeadPattern best_head;
    const bool current_is_template_id =
        template_argument_template_id_head_pattern(current.parameters,
                                                   &current.arg_syntaxes[i],
                                                   current_head) &&
        current_head.valid;
    const bool best_is_template_id =
        template_argument_template_id_head_pattern(best.parameters,
                                                   &best.arg_syntaxes[i],
                                                   best_head) &&
        best_head.valid;

    int argument_preference = 0;
    if(current_is_cv_parameter && best_is_template_id) {
      argument_preference = -1;
    } else if(best_is_cv_parameter && current_is_template_id) {
      argument_preference = 1;
    }
    if(argument_preference == 0) {
      continue;
    }
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }

  return preference;
}

const TemplateParameterInfo * direct_template_parameter_from_argument_syntax(
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgumentSyntax & syntax)
{
  if(!syntax.text.empty()) {
    const DirectTemplateParameterPattern direct =
        find_direct_template_parameter_pattern(parameters, syntax.text);
    if(direct.parameter) {
      return direct.parameter;
    }
  }
  if(!syntax.source_text.empty() && syntax.source_text != syntax.text) {
    const DirectTemplateParameterPattern direct =
        find_direct_template_parameter_pattern(parameters, syntax.source_text);
    if(direct.parameter) {
      return direct.parameter;
    }
  }

  std::string parameter_name;
  if(direct_template_parameter_name_from_syntax(syntax, parameter_name)) {
    return find_template_parameter_by_name(parameters, parameter_name);
  }
  return nullptr;
}

const TemplateIdSyntax * template_argument_template_id_syntax(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.template_id) {
    return syntax.template_id.get();
  }
  if(syntax.type_id) {
    return cppast_template_id_syntax(*syntax.type_id);
  }
  return nullptr;
}

bool template_argument_syntax_is_pack_expansion(
    const TemplateIdSyntax & syntax,
    std::size_t index)
{
  if(index < syntax.argument_syntaxes.size() &&
     syntax.argument_syntaxes[index].pack_expansion) {
    return true;
  }
  if(index < syntax.arguments.size()) {
    std::string element;
    return strip_trailing_pack_ellipsis(syntax.arguments[index], element);
  }
  return false;
}

bool template_id_trailing_pack_suffix(
    const TemplateIdSyntax & syntax,
    std::size_t & fixed_count)
{
  fixed_count = syntax.arguments.size();
  bool seen_pack = false;
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(template_argument_syntax_is_pack_expansion(syntax, i)) {
      if(!seen_pack) {
        fixed_count = i;
        seen_pack = true;
      }
      continue;
    }
    if(seen_pack) {
      return false;
    }
  }
  return seen_pack;
}

bool template_parameters_have_direct_template_template_parameter(
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
       !parameters[i].parameter_pack) {
      return true;
    }
  }
  return false;
}

template <typename PartialDecl>
bool partial_arguments_may_have_pack_expansion(const PartialDecl & partial)
{
  for(std::size_t i = 0; i < partial.arg_texts.size(); ++i) {
    if(partial.arg_texts[i].find("...") != std::string::npos) {
      return true;
    }
  }
  for(std::size_t i = 0; i < partial.arg_syntaxes.size(); ++i) {
    const TemplateArgumentSyntax & syntax = partial.arg_syntaxes[i];
    if(syntax.pack_expansion ||
       syntax.text.find("...") != std::string::npos ||
       syntax.source_text.find("...") != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool template_id_fixed_prefix_patterns_compatible(
    const std::vector<TemplateParameterInfo> & current_parameters,
    const TemplateIdSyntax & current,
    const std::vector<TemplateParameterInfo> & best_parameters,
    const TemplateIdSyntax & best,
    std::size_t prefix_count)
{
  if(current.arguments.size() < prefix_count ||
     best.arguments.size() < prefix_count) {
    return false;
  }
  for(std::size_t i = 0; i < prefix_count; ++i) {
    const TemplateArgumentSyntax * current_syntax =
        i < current.argument_syntaxes.size() ? &current.argument_syntaxes[i] : nullptr;
    const TemplateArgumentSyntax * best_syntax =
        i < best.argument_syntaxes.size() ? &best.argument_syntaxes[i] : nullptr;
    const TemplateParameterInfo * current_direct =
        current_syntax ?
            direct_template_parameter_from_argument_syntax(current_parameters,
                                                          *current_syntax) :
            nullptr;
    const TemplateParameterInfo * best_direct =
        best_syntax ?
            direct_template_parameter_from_argument_syntax(best_parameters,
                                                          *best_syntax) :
            nullptr;
    if(current_direct || best_direct) {
      if(!current_direct || !best_direct ||
         current_direct->kind != best_direct->kind ||
         current_direct->parameter_pack != best_direct->parameter_pack) {
        return false;
      }
      continue;
    }

    if(trim_space(current.arguments[i]) != trim_space(best.arguments[i])) {
      return false;
    }
  }
  return true;
}

template <typename PartialDecl>
int compare_template_id_trailing_pack_specificity(const PartialDecl & current,
                                                  const PartialDecl & best)
{
  if(current.arg_syntaxes.empty() || best.arg_syntaxes.empty()) {
    return 0;
  }
  if(!template_parameters_have_direct_template_template_parameter(current.parameters) ||
     !template_parameters_have_direct_template_template_parameter(best.parameters) ||
     (!partial_arguments_may_have_pack_expansion(current) &&
      !partial_arguments_may_have_pack_expansion(best))) {
    return 0;
  }
  int preference = 0;
  const std::size_t limit =
      std::min(current.arg_syntaxes.size(), best.arg_syntaxes.size());
  for(std::size_t i = 0; i < limit; ++i) {
    const TemplateIdSyntax * current_id =
        template_argument_template_id_syntax(current.arg_syntaxes[i]);
    const TemplateIdSyntax * best_id =
        template_argument_template_id_syntax(best.arg_syntaxes[i]);
    if(!current_id || !best_id) {
      continue;
    }

    TemplateIdHeadPattern current_head;
    TemplateIdHeadPattern best_head;
    if(!template_id_syntax_head_pattern(current.parameters,
                                        *current_id,
                                        current_head) ||
       !template_id_syntax_head_pattern(best.parameters,
                                        *best_id,
                                        best_head) ||
       !current_head.direct_template_template_parameter ||
       !best_head.direct_template_template_parameter) {
      continue;
    }

    std::size_t current_fixed = 0;
    std::size_t best_fixed = 0;
    const bool current_has_pack =
        template_id_trailing_pack_suffix(*current_id, current_fixed);
    const bool best_has_pack =
        template_id_trailing_pack_suffix(*best_id, best_fixed);
    int argument_preference = 0;
    if(!current_has_pack && best_has_pack &&
       current_id->arguments.size() == best_fixed &&
       template_id_fixed_prefix_patterns_compatible(current.parameters,
                                                    *current_id,
                                                    best.parameters,
                                                    *best_id,
                                                    best_fixed)) {
      argument_preference = -1;
    } else if(!best_has_pack && current_has_pack &&
              best_id->arguments.size() == current_fixed &&
              template_id_fixed_prefix_patterns_compatible(current.parameters,
                                                           *current_id,
                                                           best.parameters,
                                                           *best_id,
                                                           current_fixed)) {
      argument_preference = 1;
    }
    if(argument_preference == 0) {
      continue;
    }
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }
  return preference;
}

struct TemplateParameterPatternOccurrence
{
  std::string path;
  const TemplateParameterInfo * parameter = nullptr;
};

void collect_repeated_template_parameter_occurrences_from_template_id(
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateIdSyntax & syntax,
    const std::string & path,
    std::vector<TemplateParameterPatternOccurrence> & out);

void collect_repeated_template_parameter_occurrences(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & raw_text,
    const TemplateArgumentSyntax * syntax,
    const std::string & path,
    std::vector<TemplateParameterPatternOccurrence> & out)
{
  if(syntax) {
    const TemplateParameterInfo * direct =
        direct_template_parameter_from_argument_syntax(parameters, *syntax);
    if(direct) {
      TemplateParameterPatternOccurrence occurrence;
      occurrence.path = path;
      occurrence.parameter = direct;
      out.push_back(occurrence);
      return;
    }

    if(syntax->template_id) {
      collect_repeated_template_parameter_occurrences_from_template_id(
          parameters, *syntax->template_id, path, out);
      return;
    }
    if(syntax->type_id) {
      if(const TemplateIdSyntax * type_template_id =
             cppast_template_id_syntax(*syntax->type_id)) {
        collect_repeated_template_parameter_occurrences_from_template_id(
            parameters, *type_template_id, path, out);
        return;
      }
    }
  }

  const std::string text = trim_space(raw_text);
  const DirectTemplateParameterPattern direct =
      find_direct_template_parameter_pattern(parameters, text);
  if(direct.parameter) {
    TemplateParameterPatternOccurrence occurrence;
    occurrence.path = path;
    occurrence.parameter = direct.parameter;
    out.push_back(occurrence);
    return;
  }

  QualifiedName template_name;
  std::vector<std::string> template_args;
  if(!semantic_utils::split_top_level_template_id_text(text,
                                                       template_name,
                                                       template_args)) {
    return;
  }
  for(std::size_t i = 0; i < template_args.size(); ++i) {
    collect_repeated_template_parameter_occurrences(
        parameters,
        template_args[i],
        nullptr,
        path + "/" + std::to_string(i),
        out);
  }
}

void collect_repeated_template_parameter_occurrences_from_template_id(
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateIdSyntax & syntax,
    const std::string & path,
    std::vector<TemplateParameterPatternOccurrence> & out)
{
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    const TemplateArgumentSyntax * child_syntax =
        i < syntax.argument_syntaxes.size() ? &syntax.argument_syntaxes[i] : nullptr;
    collect_repeated_template_parameter_occurrences(
        parameters,
        syntax.arguments[i],
        child_syntax,
        path + "/" + std::to_string(i),
        out);
  }
}

struct RepeatedTemplateParameterConstraintSet
{
  std::set<std::pair<std::string, std::string> > repeated_occurrence_paths;
};

bool repeated_template_parameter_constraints_include(
    const RepeatedTemplateParameterConstraintSet & lhs,
    const RepeatedTemplateParameterConstraintSet & rhs)
{
  std::set<std::pair<std::string, std::string> >::const_iterator it =
      rhs.repeated_occurrence_paths.begin();
  for(; it != rhs.repeated_occurrence_paths.end(); ++it) {
    if(lhs.repeated_occurrence_paths.count(*it) == 0) {
      return false;
    }
  }
  return true;
}

template <typename PartialDecl>
RepeatedTemplateParameterConstraintSet repeated_template_parameter_constraints(
    const PartialDecl & partial)
{
  RepeatedTemplateParameterConstraintSet out;
  std::vector<TemplateParameterPatternOccurrence> occurrences;
  for(std::size_t i = 0; i < partial.arg_texts.size(); ++i) {
    const TemplateArgumentSyntax * syntax =
        i < partial.arg_syntaxes.size() ? &partial.arg_syntaxes[i] : nullptr;
    collect_repeated_template_parameter_occurrences(
        partial.parameters,
        partial.arg_texts[i],
        syntax,
        std::to_string(i),
        occurrences);
  }

  for(std::size_t i = 0; i < occurrences.size(); ++i) {
    if(!occurrences[i].parameter) {
      continue;
    }
    for(std::size_t j = i + 1; j < occurrences.size(); ++j) {
      if(occurrences[i].parameter != occurrences[j].parameter) {
        continue;
      }
      out.repeated_occurrence_paths.insert(
          std::make_pair(occurrences[i].path, occurrences[j].path));
    }
  }
  return out;
}

template <typename PartialDecl>
int compare_repeated_template_parameter_constraint_specificity(
    const PartialDecl & current,
    const PartialDecl & best)
{
  const RepeatedTemplateParameterConstraintSet current_constraints =
      repeated_template_parameter_constraints(current);
  const RepeatedTemplateParameterConstraintSet best_constraints =
      repeated_template_parameter_constraints(best);

  const bool current_includes_best =
      repeated_template_parameter_constraints_include(current_constraints,
                                                      best_constraints);
  const bool best_includes_current =
      repeated_template_parameter_constraints_include(best_constraints,
                                                     current_constraints);

  if(current_includes_best && !best_includes_current) {
    return -1;
  }
  if(best_includes_current && !current_includes_best) {
    return 1;
  }
  return 0;
}

int top_level_cv_rank(const TypePtr & type, TypePtr & base)
{
  bool cv_const = false;
  bool cv_volatile = false;
  if(!top_level_cv_flags(type, base, cv_const, cv_volatile)) {
    base.reset();
    return 0;
  }
  return (cv_const ? 1 : 0) + (cv_volatile ? 1 : 0);
}

bool partial_order_cv_bases_comparable(const TypePtr & lhs,
                                       const TypePtr & rhs)
{
  if(!lhs || !rhs) {
    return false;
  }
  if(type_equals(lhs, rhs)) {
    return true;
  }
  return lhs->kind == rhs->kind;
}

int compare_transformed_partial_argument_top_cv_specificity(
    const std::vector<TemplateArgument> & current_arguments,
    const std::vector<TemplateArgument> & best_arguments)
{
  if(current_arguments.size() != best_arguments.size()) {
    return 0;
  }

  int preference = 0;
  for(std::size_t i = 0; i < current_arguments.size(); ++i) {
    if(current_arguments[i].kind != TemplateArgument::TA_TYPE ||
       best_arguments[i].kind != TemplateArgument::TA_TYPE) {
      continue;
    }

    TypePtr current_base;
    TypePtr best_base;
    const int current_cv = top_level_cv_rank(current_arguments[i].type,
                                            current_base);
    const int best_cv = top_level_cv_rank(best_arguments[i].type, best_base);
    if(current_cv == best_cv) {
      continue;
    }
    if(!partial_order_cv_bases_comparable(current_base, best_base)) {
      return 0;
    }

    const int argument_preference = current_cv > best_cv ? -1 : 1;
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }

  return preference;
}

struct PartialOrderReferenceCvShape
{
  int reference_kind = 0;
  int cv_rank = 0;
  TypePtr base;
};

bool partial_order_reference_cv_shape(const TypePtr & type,
                                      PartialOrderReferenceCvShape & out)
{
  out = PartialOrderReferenceCvShape();
  if(!type) {
    return false;
  }

  TypePtr cv_subject = type;
  if(type->kind == Type::TK_LVALUE_REFERENCE ||
     type->kind == Type::TK_RVALUE_REFERENCE) {
    out.reference_kind =
        type->kind == Type::TK_LVALUE_REFERENCE ? 1 : 2;
    cv_subject = type->inner;
  }

  bool cv_const = false;
  bool cv_volatile = false;
  if(!top_level_cv_flags(cv_subject, out.base, cv_const, cv_volatile)) {
    out.base.reset();
    return false;
  }
  out.cv_rank = (cv_const ? 1 : 0) + (cv_volatile ? 1 : 0);
  return true;
}

int compare_transformed_partial_argument_reference_cv_specificity(
    const std::vector<TemplateArgument> & current_arguments,
    const std::vector<TemplateArgument> & best_arguments)
{
  if(current_arguments.size() != best_arguments.size()) {
    return 0;
  }

  int preference = 0;
  for(std::size_t i = 0; i < current_arguments.size(); ++i) {
    if(current_arguments[i].kind != TemplateArgument::TA_TYPE ||
       best_arguments[i].kind != TemplateArgument::TA_TYPE) {
      continue;
    }

    PartialOrderReferenceCvShape current_shape;
    PartialOrderReferenceCvShape best_shape;
    if(!partial_order_reference_cv_shape(current_arguments[i].type,
                                         current_shape) ||
       !partial_order_reference_cv_shape(best_arguments[i].type,
                                         best_shape)) {
      return 0;
    }
    if(!partial_order_cv_bases_comparable(current_shape.base,
                                          best_shape.base)) {
      return 0;
    }

    int argument_preference = 0;
    if(current_shape.reference_kind != best_shape.reference_kind) {
      if(current_shape.reference_kind != 0 && best_shape.reference_kind != 0) {
        return 0;
      }
      argument_preference =
          current_shape.reference_kind != 0 ? -1 : 1;
    } else if(current_shape.reference_kind != 0 &&
              current_shape.cv_rank != best_shape.cv_rank) {
      argument_preference =
          current_shape.cv_rank > best_shape.cv_rank ? -1 : 1;
    }

    if(argument_preference == 0) {
      continue;
    }
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }

  return preference;
}

struct FunctionPatternPackShape
{
  bool trailing_pack = false;
  std::size_t parameter_count = 0;
  std::size_t fixed_parameter_count = 0;
};

bool function_pattern_pack_shape(const TemplateArgument & argument,
                                 const TemplateArgumentSyntax * syntax,
                                 FunctionPatternPackShape & out)
{
  out = FunctionPatternPackShape();
  if(argument.kind != TemplateArgument::TA_TYPE) {
    return false;
  }

  TypePtr type = strip_top_level_cv(argument.type);
  if(!type || type->kind != Type::TK_FUNCTION) {
    return false;
  }

  out.parameter_count = type->params.size();
  out.trailing_pack = function_type_syntax_trailing_parameter_is_pack(syntax);
  out.fixed_parameter_count =
      out.trailing_pack && out.parameter_count != 0 ?
          out.parameter_count - 1 :
          out.parameter_count;
  return true;
}

template <typename PartialDecl>
int compare_function_type_pack_specificity(
    const PartialDecl & current,
    const PartialDecl & best,
    const std::vector<TemplateArgument> & current_arguments,
    const std::vector<TemplateArgument> & best_arguments)
{
  const std::size_t limit =
      std::min(std::min(current_arguments.size(), best_arguments.size()),
               std::min(current.arg_syntaxes.size(), best.arg_syntaxes.size()));
  int preference = 0;
  for(std::size_t i = 0; i < limit; ++i) {
    FunctionPatternPackShape current_shape;
    FunctionPatternPackShape best_shape;
    if(!function_pattern_pack_shape(current_arguments[i],
                                    &current.arg_syntaxes[i],
                                    current_shape) ||
       !function_pattern_pack_shape(best_arguments[i],
                                    &best.arg_syntaxes[i],
                                    best_shape) ||
       (!current_shape.trailing_pack && !best_shape.trailing_pack)) {
      continue;
    }

    int argument_preference = 0;
    if(current_shape.trailing_pack != best_shape.trailing_pack) {
      if(!current_shape.trailing_pack &&
         current_shape.parameter_count >= best_shape.fixed_parameter_count) {
        argument_preference = -1;
      } else if(!best_shape.trailing_pack &&
                best_shape.parameter_count >= current_shape.fixed_parameter_count) {
        argument_preference = 1;
      }
    } else if(current_shape.fixed_parameter_count !=
              best_shape.fixed_parameter_count) {
      argument_preference =
          current_shape.fixed_parameter_count >
              best_shape.fixed_parameter_count ?
                  -1 :
                  1;
    }

    if(argument_preference == 0) {
      continue;
    }
    if(preference != 0 && preference != argument_preference) {
      return 0;
    }
    preference = argument_preference;
  }

  return preference;
}

struct PartialPatternSpecificity
{
  std::size_t concrete_components = 0;
  std::size_t fixed_components = 0;
  bool uses_pack_expansion = false;
};

void accumulate_partial_pattern_specificity(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & raw_text,
    const TemplateArgumentSyntax * syntax,
    PartialPatternSpecificity & out)
{
  const std::string text = strip_elaborated_type_prefix(trim_space(raw_text));
  if(text.empty()) {
    return;
  }

  const DirectTemplateParameterPattern direct =
      find_direct_template_parameter_pattern(parameters, text);
  if(direct.parameter) {
    if((direct.pack_expansion || (syntax && syntax->pack_expansion)) &&
       direct.parameter->parameter_pack) {
      out.uses_pack_expansion = true;
      return;
    }
    ++out.fixed_components;
    return;
  }

  QualifiedName template_name;
  std::vector<std::string> template_args;
  const TemplateIdSyntax * template_id_syntax =
      syntax && syntax->template_id ? syntax->template_id.get() : nullptr;
  if(!template_id_syntax && syntax && syntax->type_id) {
    template_id_syntax = cppast_template_id_syntax(*syntax->type_id);
  }
  if(template_id_syntax) {
    ++out.fixed_components;
    for(std::size_t i = 0; i < template_id_syntax->arguments.size(); ++i) {
      const TemplateArgumentSyntax * child_syntax =
          i < template_id_syntax->argument_syntaxes.size() ?
              &template_id_syntax->argument_syntaxes[i] : nullptr;
      accumulate_partial_pattern_specificity(parameters,
                                             template_id_syntax->arguments[i],
                                             child_syntax,
                                             out);
    }
    return;
  }

  if(semantic_utils::split_top_level_template_id_text(text,
                                                       template_name,
                                                       template_args)) {
    ++out.fixed_components;
    for(std::size_t i = 0; i < template_args.size(); ++i) {
      accumulate_partial_pattern_specificity(parameters, template_args[i], nullptr, out);
    }
    return;
  }

  ++out.fixed_components;
  ++out.concrete_components;
}

template <typename PartialDecl>
PartialPatternSpecificity partial_pattern_specificity(const PartialDecl & partial)
{
  PartialPatternSpecificity out;
  for(std::size_t i = 0; i < partial.arg_texts.size(); ++i) {
    const TemplateArgumentSyntax * syntax =
        i < partial.arg_syntaxes.size() ? &partial.arg_syntaxes[i] : nullptr;
    accumulate_partial_pattern_specificity(partial.parameters,
                                           partial.arg_texts[i],
                                           syntax,
                                           out);
  }
  return out;
}

template <typename PartialDecl>
int compare_partial_specialization_pack_specificity(const PartialDecl & current,
                                                    const PartialDecl & best)
{
  const PartialPatternSpecificity current_specificity =
      partial_pattern_specificity(current);
  const PartialPatternSpecificity best_specificity =
      partial_pattern_specificity(best);
  if(!current_specificity.uses_pack_expansion &&
     !best_specificity.uses_pack_expansion) {
    return 0;
  }
  if(current_specificity.concrete_components !=
     best_specificity.concrete_components) {
    return 0;
  }

  if(current_specificity.fixed_components >
     best_specificity.fixed_components) {
    return -1;
  }
  if(best_specificity.fixed_components >
     current_specificity.fixed_components) {
    return 1;
  }
  return 0;
}

std::string template_argument_text_for_matching(template_api::TemplateTypeSystem & type_system,
                                                const TemplateArgument & arg);

std::string template_argument_text_for_matching(template_api::TemplateTypeSystem & type_system,
                                                const TemplateArgument & arg)
{
  if(!arg.text.empty()) {
    return trim_space(arg.text);
  }

  if(arg.kind == TemplateArgument::TA_TYPE) {
    if(!arg.type) {
      return std::string();
    }
    if(template_argument_semantics::type_depends_on_template_parameter(type_system, arg.type)) {
      if(arg.type->kind == Type::TK_NAMED) {
        return strip_elaborated_type_prefix(trim_space(arg.type->named_display));
      }
      return std::string();
    }
    return trim_space(specialization_argument_type_text(type_system, arg.type));
  }

  if(arg.kind == TemplateArgument::TA_VALUE) {
    if(arg.dependent) {
      return trim_space(arg.text);
    }
    return trim_space(template_argument_text(
        arg,
        [&type_system](const TypePtr & type)
        {
          return specialization_argument_type_text(type_system, type);
        }));
  }

  if(arg.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
     arg.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    if(arg.kind == TemplateArgument::TA_CLASS_TEMPLATE && arg.template_decl) {
      return trim_space(static_cast<ClassTemplateDecl *>(arg.template_decl)->name);
    }
    if(arg.kind == TemplateArgument::TA_ALIAS_TEMPLATE && arg.template_decl) {
      return trim_space(static_cast<AliasTemplateDecl *>(arg.template_decl)->name);
    }
    return trim_space(arg.text);
  }

  return std::string();
}

bool is_identifier_char(char c)
{
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool is_simple_identifier_text(const std::string & text)
{
  if(text.empty()) {
    return false;
  }
  if(!(std::isalpha(static_cast<unsigned char>(text[0])) ||
       text[0] == '_')) {
    return false;
  }
  for(std::size_t i = 1; i < text.size(); ++i) {
    if(!is_identifier_char(text[i])) {
      return false;
    }
  }
  return true;
}

std::vector<std::string> top_level_array_bound_texts(const std::string & text)
{
  std::vector<std::string> out;
  int angle_depth = 0;
  int paren_depth = 0;
  int brace_depth = 0;
  int bracket_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>') {
      if(angle_depth > 0) {
        --angle_depth;
      }
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
      continue;
    }
    if(ch == ')') {
      if(paren_depth > 0) {
        --paren_depth;
      }
      continue;
    }
    if(ch == '{') {
      ++brace_depth;
      continue;
    }
    if(ch == '}') {
      if(brace_depth > 0) {
        --brace_depth;
      }
      continue;
    }
    if(ch == '[') {
      if(angle_depth == 0 && paren_depth == 0 && brace_depth == 0 &&
         bracket_depth == 0) {
        const std::size_t bound_start = i + 1;
        ++bracket_depth;
        while(i + 1 < text.size() && bracket_depth > 0) {
          ++i;
          if(text[i] == '[') {
            ++bracket_depth;
          } else if(text[i] == ']') {
            --bracket_depth;
          }
        }
        if(bracket_depth == 0) {
          out.push_back(trim_space(text.substr(bound_start, i - bound_start)));
        }
        continue;
      }
      ++bracket_depth;
      continue;
    }
    if(ch == ']') {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      continue;
    }
  }
  return out;
}

bool deduce_array_bound_texts_from_actual_type(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & pattern_text,
    const TypePtr & pattern_type,
    const TypePtr & actual_type,
    DeducedState & deduced)
{
  const std::vector<std::string> bound_texts =
      top_level_array_bound_texts(pattern_text);
  if(bound_texts.empty()) {
    return true;
  }

  std::vector<TypePtr> actual_array_types;
  std::function<void(const TypePtr &, const TypePtr &)> collect_array_matches =
      [&](const TypePtr & raw_pattern, const TypePtr & raw_actual)
  {
    TypePtr pattern = strip_top_level_cv(raw_pattern);
    TypePtr actual = strip_top_level_cv(raw_actual);
    if(!pattern || !actual) {
      return;
    }
    if(pattern->kind == Type::TK_ARRAY) {
      actual_array_types.push_back(actual);
      if(actual->kind == Type::TK_ARRAY) {
        collect_array_matches(pattern->inner, actual->inner);
      }
      return;
    }
    if(pattern->kind != actual->kind) {
      return;
    }
    switch(pattern->kind) {
    case Type::TK_CV:
    case Type::TK_ATOMIC:
    case Type::TK_POINTER:
    case Type::TK_BLOCK_POINTER:
    case Type::TK_LVALUE_REFERENCE:
    case Type::TK_RVALUE_REFERENCE:
      collect_array_matches(pattern->inner, actual->inner);
      return;
    case Type::TK_MEMBER_POINTER:
      collect_array_matches(pattern->owner, actual->owner);
      collect_array_matches(pattern->inner, actual->inner);
      return;
    case Type::TK_FUNCTION:
      collect_array_matches(pattern->inner, actual->inner);
      for(std::size_t i = 0;
          i < pattern->params.size() && i < actual->params.size();
          ++i) {
        collect_array_matches(pattern->params[i], actual->params[i]);
      }
      return;
    case Type::TK_ARRAY:
    case Type::TK_NAMED:
    case Type::TK_FUNDAMENTAL:
      return;
    }
  };
  collect_array_matches(pattern_type, actual_type);

  for(std::size_t i = 0; i < bound_texts.size(); ++i) {
    const std::string bound_text = trim_space(bound_texts[i]);
    if(!is_simple_identifier_text(bound_text)) {
      continue;
    }

    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(parameters, bound_text);
    if(!parameter ||
       parameter->kind != TemplateParameterInfo::TP_NON_TYPE ||
       parameter->parameter_pack) {
      continue;
    }

    if(i >= actual_array_types.size()) {
      return false;
    }
    const TypePtr & actual_array = actual_array_types[i];
    if(!actual_array ||
       actual_array->kind != Type::TK_ARRAY ||
       !actual_array->has_bound ||
       !store_deduced_value(deduced,
                            parameter->name,
                            static_cast<long long>(actual_array->bound))) {
      return false;
    }
  }
  return true;
}

bool stable_type_substitution_argument_key(
    const TypePtr & type,
    AliasTemplateDecl::StableSubstitutionArgumentKey & out)
{
  if(!type) {
    return false;
  }
  if(type->kind == Type::TK_FUNDAMENTAL) {
    out.kind = AliasTemplateDecl::StableSubstitutionArgumentKey::AK_TYPE_FUNDAMENTAL;
    out.pointer = nullptr;
    out.type_code = static_cast<int>(type->fundamental);
    out.value = 0;
    out.value_type_is_fundamental = true;
    return true;
  }
  out.kind = AliasTemplateDecl::StableSubstitutionArgumentKey::AK_TYPE_POINTER;
  out.pointer = type.get();
  out.type_code = 0;
  out.value = 0;
  out.value_type_is_fundamental = false;
  return true;
}

bool stable_substitution_argument_key(
    const TemplateArgument & argument,
    AliasTemplateDecl::StableSubstitutionArgumentKey & out)
{
  out = AliasTemplateDecl::StableSubstitutionArgumentKey();
  switch(argument.kind) {
  case TemplateArgument::TA_TYPE:
    return stable_type_substitution_argument_key(argument.type, out);

  case TemplateArgument::TA_VALUE:
  {
    if(argument.dependent || !argument.type) {
      return false;
    }
    AliasTemplateDecl::StableSubstitutionArgumentKey type_key;
    if(!stable_type_substitution_argument_key(argument.type, type_key)) {
      return false;
    }
    out.kind = AliasTemplateDecl::StableSubstitutionArgumentKey::AK_VALUE;
    out.pointer = type_key.pointer;
    out.type_code = type_key.type_code;
    out.value = argument.value;
    out.value_type_is_fundamental =
        type_key.kind == AliasTemplateDecl::StableSubstitutionArgumentKey::
            AK_TYPE_FUNDAMENTAL;
    return true;
  }

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
    if(argument.dependent || !argument.template_decl) {
      return false;
    }
    out.kind = AliasTemplateDecl::StableSubstitutionArgumentKey::AK_TEMPLATE_DECL;
    out.pointer = argument.template_decl;
    out.type_code = static_cast<int>(argument.kind);
    return true;
  }
  return false;
}

bool stable_substitution_key_for_arguments(
    const std::vector<TemplateArgument> & arguments,
    AliasTemplateDecl::StableSubstitutionKey & out)
{
  out.arguments.clear();
  out.arguments.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    AliasTemplateDecl::StableSubstitutionArgumentKey argument_key;
    if(!stable_substitution_argument_key(arguments[i], argument_key)) {
      out.arguments.clear();
      return false;
    }
    out.arguments.push_back(argument_key);
  }
  return true;
}

bool stable_alias_expansion_type_key(
    const TypePtr & type,
    const void *& type_pointer,
    int & type_code)
{
  type_pointer = nullptr;
  type_code = 0;
  if(!type) {
    return false;
  }
  type_code = static_cast<int>(type->kind);
  if(type->kind == Type::TK_FUNDAMENTAL) {
    type_code = 1000 + static_cast<int>(type->fundamental);
    return true;
  }
  type_pointer = type.get();
  return true;
}

std::string stable_alias_expansion_argument_text(
    const TemplateArgument & argument)
{
  if(!argument.text.empty()) {
    return trim_space(argument.text);
  }
  if(argument.source_syntax) {
    if(!argument.source_syntax->text.empty()) {
      return trim_space(argument.source_syntax->text);
    }
    if(!argument.source_syntax->source_text.empty()) {
      return trim_space(argument.source_syntax->source_text);
    }
  }
  return std::string();
}

bool stable_alias_expansion_argument_key(
    const TemplateArgument & argument,
    AliasTemplateDecl::StableAliasExpansionArgumentKey & out)
{
  out = AliasTemplateDecl::StableAliasExpansionArgumentKey();
  out.kind = static_cast<int>(argument.kind);
  out.dependent = argument.dependent;
  out.source_defaulted = argument.source_defaulted;
  out.function_value = argument.function_value;
  out.value_binding = argument.value_binding;
  if(argument.kind == TemplateArgument::TA_TYPE ||
     argument.kind == TemplateArgument::TA_VALUE) {
    if(argument.type &&
       !stable_alias_expansion_type_key(argument.type,
                                        out.type_pointer,
                                        out.type_code)) {
      return false;
    }
  }
  switch(argument.kind) {
  case TemplateArgument::TA_TYPE:
    if(!argument.type) {
      return false;
    }
    if(argument.dependent) {
      out.text = stable_alias_expansion_argument_text(argument);
    }
    return true;

  case TemplateArgument::TA_VALUE:
    if(argument.dependent) {
      out.text = stable_alias_expansion_argument_text(argument);
      return !out.text.empty();
    }
    out.value = argument.value;
    return true;

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
    if(argument.dependent || !argument.template_decl) {
      out.text = stable_alias_expansion_argument_text(argument);
      return !out.text.empty();
    }
    out.template_decl = argument.template_decl;
    return true;
  }
  return false;
}

AliasTemplateDecl::StableAliasExpansionScopeKey stable_alias_expansion_scope_key(
    const Scope & scope)
{
  AliasTemplateDecl::StableAliasExpansionScopeKey key;
  key.instance_id = scope.instance_id;
  key.binding_fingerprint = template_scope::scope_binding_fingerprint(scope);
  return key;
}

bool stable_alias_expansion_key_for_arguments(
    const Scope & match_scope,
    const Scope & argument_scope,
    const std::vector<TemplateArgument> & arguments,
    bool allow_dependent_expansion,
    AliasTemplateDecl::StableAliasExpansionKey & out)
{
  out = AliasTemplateDecl::StableAliasExpansionKey();
  out.allow_dependent_expansion = allow_dependent_expansion;
  out.match_scope = stable_alias_expansion_scope_key(match_scope);
  out.argument_scope = stable_alias_expansion_scope_key(argument_scope);
  out.arguments.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    AliasTemplateDecl::StableAliasExpansionArgumentKey argument_key;
    if(!stable_alias_expansion_argument_key(arguments[i], argument_key)) {
      out.arguments.clear();
      return false;
    }
    out.arguments.push_back(argument_key);
  }
  return true;
}

bool text_mentions_identifier_token(const std::string & text,
                                    const std::string & name)
{
  if(name.empty() || text.find(name) == std::string::npos) {
    return false;
  }
  std::size_t i = 0;
  while(i < text.size()) {
    if(text.compare(i, name.size(), name) == 0 &&
       (i == 0 || !is_identifier_char(text[i - 1])) &&
       (i + name.size() == text.size() || !is_identifier_char(text[i + name.size()]))) {
      return true;
    }
    ++i;
  }
  return false;
}

bool text_mentions_pack_parameter_reference(const std::string & text,
                                            const std::string & name)
{
  if(name.empty() || text.find(name) == std::string::npos) {
    return false;
  }
  std::size_t i = 0;
  while(i < text.size()) {
    if(text.compare(i, name.size(), name) == 0 &&
       (i == 0 || !is_identifier_char(text[i - 1])) &&
       (i + name.size() == text.size() || !is_identifier_char(text[i + name.size()]))) {
      std::size_t suffix = i + name.size();
      while(suffix < text.size() &&
            std::isspace(static_cast<unsigned char>(text[suffix]))) {
        ++suffix;
      }
      if(suffix + 2 < text.size() && text.compare(suffix, 3, "...") == 0) {
        return true;
      }
    }
    ++i;
  }
  return false;
}

bool alias_template_target_mentions_parameters(
    const std::string & text,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].name.empty()) {
      continue;
    }
    const bool mentions =
        parameters[i].parameter_pack ?
            text_mentions_pack_parameter_reference(text, parameters[i].name) :
            text_mentions_identifier_token(text, parameters[i].name);
    if(mentions) {
      return true;
    }
  }
  return false;
}

bool alias_template_target_mentions_parameters(
    const CppAstNode & node,
    const std::vector<TemplateParameterInfo> & parameters)
{
  const std::string value = node_text(node);
  if(!value.empty() &&
     alias_template_target_mentions_parameters(value, parameters)) {
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(alias_template_target_mentions_parameters(node.children[i], parameters)) {
      return true;
    }
  }
  return false;
}

bool alias_template_type_pattern_mentions_parameters(
    const TypePtr & type,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(!type) {
    return false;
  }
  if(type->kind == Type::TK_NAMED) {
    if(alias_template_target_mentions_parameters(type->named_key, parameters) ||
       alias_template_target_mentions_parameters(type->named_display, parameters) ||
       alias_template_target_mentions_parameters(named_type_semantic_payload(type),
                                                 parameters)) {
      return true;
    }
    for(std::size_t i = 0; i < type->named_dependent_alias_arguments.size(); ++i) {
      const DependentAliasTemplateArgumentSyntax & argument =
          type->named_dependent_alias_arguments[i];
      if(alias_template_target_mentions_parameters(argument.text, parameters) ||
         alias_template_type_pattern_mentions_parameters(argument.type, parameters)) {
        return true;
      }
    }
    for(std::size_t i = 0; i < type->named_dependent_class_arguments.size(); ++i) {
      const DependentAliasTemplateArgumentSyntax & argument =
          type->named_dependent_class_arguments[i];
      if(alias_template_target_mentions_parameters(argument.text, parameters) ||
         alias_template_type_pattern_mentions_parameters(argument.type, parameters)) {
        return true;
      }
    }
    if(alias_template_type_pattern_mentions_parameters(
           type->named_dependent_qualified_owner,
           parameters)) {
      return true;
    }
  }
  if(alias_template_type_pattern_mentions_parameters(type->inner, parameters) ||
     alias_template_type_pattern_mentions_parameters(type->owner, parameters)) {
    return true;
  }
  for(std::size_t i = 0; i < type->params.size(); ++i) {
    if(alias_template_type_pattern_mentions_parameters(type->params[i],
                                                       parameters)) {
      return true;
    }
  }
  return false;
}

bool template_arguments_include_non_type_parameter(
    const std::vector<DependentAliasTemplateArgumentSyntax> & arguments,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(arguments.empty()) {
    return false;
  }
  std::size_t first_pack_parameter = parameters.size();
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      first_pack_parameter = i;
      break;
    }
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    const std::size_t parameter_index =
        first_pack_parameter != parameters.size() && i >= first_pack_parameter ?
            first_pack_parameter :
            i;
    if(parameter_index >= parameters.size()) {
      continue;
    }
    if(parameters[parameter_index].kind == TemplateParameterInfo::TP_NON_TYPE &&
       arguments[i].syntax.dependent) {
      return true;
    }
  }
  return false;
}

bool type_has_dependent_non_type_template_argument(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    int depth = 0)
{
  if(depth > 32) {
    return false;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_NAMED) {
    template_api::TemplateNamedTypeMetadata metadata;
    if(template_api::describe_named_type_metadata(type_system.model,
                                                  base,
                                                  metadata) &&
       metadata.source_template) {
      for(std::size_t i = 0; i < metadata.instantiation_arguments.size(); ++i) {
        const TemplateArgument & argument = metadata.instantiation_arguments[i];
        if(argument.kind == TemplateArgument::TA_VALUE && argument.dependent) {
          return true;
        }
        if(argument.kind == TemplateArgument::TA_TYPE &&
           type_has_dependent_non_type_template_argument(type_system,
                                                         argument.type,
                                                         depth + 1)) {
          return true;
        }
      }
    }

    void * alias_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> alias_args;
    if(named_type_dependent_alias_template(base,
                                           alias_template_decl,
                                           alias_args) &&
       alias_template_decl) {
      const AliasTemplateDecl * alias_template =
          static_cast<const AliasTemplateDecl *>(alias_template_decl);
      if(template_arguments_include_non_type_parameter(alias_args,
                                                       alias_template->parameters)) {
        return true;
      }
      for(std::size_t i = 0; i < alias_args.size(); ++i) {
        if(type_has_dependent_non_type_template_argument(type_system,
                                                         alias_args[i].type,
                                                         depth + 1)) {
          return true;
        }
      }
    }

    void * class_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> class_args;
    if(named_type_dependent_class_template(base,
                                           class_template_decl,
                                           class_args) &&
       class_template_decl) {
      const ClassTemplateDecl * class_template =
          static_cast<const ClassTemplateDecl *>(class_template_decl);
      if(template_arguments_include_non_type_parameter(class_args,
                                                       class_template->parameters)) {
        return true;
      }
      for(std::size_t i = 0; i < class_args.size(); ++i) {
        if(type_has_dependent_non_type_template_argument(type_system,
                                                         class_args[i].type,
                                                         depth + 1)) {
          return true;
        }
      }
    }

    if(type_has_dependent_non_type_template_argument(
           type_system,
           base->named_dependent_qualified_owner,
           depth + 1)) {
      return true;
    }
  }

  if(type_has_dependent_non_type_template_argument(type_system,
                                                   base->inner,
                                                   depth + 1) ||
     type_has_dependent_non_type_template_argument(type_system,
                                                   base->owner,
                                                   depth + 1)) {
    return true;
  }
  for(std::size_t i = 0; i < base->params.size(); ++i) {
    if(type_has_dependent_non_type_template_argument(type_system,
                                                     base->params[i],
                                                     depth + 1)) {
      return true;
    }
  }
  return false;
}

bool dependent_non_type_argument_is_direct_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    std::string text)
{
  text = trim_space(text);
  if(text.size() >= 3 && text.substr(text.size() - 3) == "...") {
    text = trim_space(text.substr(0, text.size() - 3));
  }
  if(text.empty()) {
    return false;
  }
  if((std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_') &&
     std::all_of(text.begin() + 1,
                 text.end(),
                 [](char ch)
                 {
                   return std::isalnum(static_cast<unsigned char>(ch)) ||
                          ch == '_';
                 })) {
    return true;
  }
  const TemplateParameterInfo * parameter =
      find_template_parameter_by_name(parameters, text);
  return parameter && parameter->kind == TemplateParameterInfo::TP_NON_TYPE;
}

bool dependent_non_type_argument_syntax_is_direct_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const DependentAliasTemplateArgumentSyntax & argument)
{
  return dependent_non_type_argument_is_direct_parameter(parameters,
                                                        argument.text) ||
         dependent_non_type_argument_is_direct_parameter(parameters,
                                                        argument.syntax.text);
}

bool dependent_non_type_template_arguments_are_direct_parameters(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    const std::vector<TemplateParameterInfo> & parameters,
    int depth = 0)
{
  if(depth > 32) {
    return true;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return true;
  }

  if(base->kind == Type::TK_NAMED) {
    template_api::TemplateNamedTypeMetadata metadata;
    if(template_api::describe_named_type_metadata(type_system.model,
                                                  base,
                                                  metadata) &&
       metadata.source_template) {
      for(std::size_t i = 0; i < metadata.instantiation_arguments.size(); ++i) {
        const TemplateArgument & argument = metadata.instantiation_arguments[i];
        if(argument.kind == TemplateArgument::TA_VALUE && argument.dependent &&
           !dependent_non_type_argument_is_direct_parameter(parameters,
                                                           argument.text)) {
          return false;
        }
        if(argument.kind == TemplateArgument::TA_TYPE &&
           !dependent_non_type_template_arguments_are_direct_parameters(
               type_system,
               argument.type,
               parameters,
               depth + 1)) {
          return false;
        }
      }
    }

    void * alias_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> alias_args;
    if(named_type_dependent_alias_template(base,
                                           alias_template_decl,
                                           alias_args) &&
       alias_template_decl) {
      const AliasTemplateDecl * alias_template =
          static_cast<const AliasTemplateDecl *>(alias_template_decl);
      for(std::size_t i = 0; i < alias_args.size(); ++i) {
        const std::size_t parameter_index =
            alias_template->parameters.empty() ?
                alias_template->parameters.size() :
                std::min(i, alias_template->parameters.size() - 1);
        if(parameter_index < alias_template->parameters.size() &&
           alias_template->parameters[parameter_index].kind ==
               TemplateParameterInfo::TP_NON_TYPE &&
           alias_args[i].syntax.dependent &&
           !dependent_non_type_argument_syntax_is_direct_parameter(
               parameters,
               alias_args[i])) {
          return false;
        }
        if(!dependent_non_type_template_arguments_are_direct_parameters(
               type_system,
               alias_args[i].type,
               parameters,
               depth + 1)) {
          return false;
        }
      }
    }

    void * class_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> class_args;
    if(named_type_dependent_class_template(base,
                                           class_template_decl,
                                           class_args) &&
       class_template_decl) {
      const ClassTemplateDecl * class_template =
          static_cast<const ClassTemplateDecl *>(class_template_decl);
      for(std::size_t i = 0; i < class_args.size(); ++i) {
        const std::size_t parameter_index =
            class_template->parameters.empty() ?
                class_template->parameters.size() :
                std::min(i, class_template->parameters.size() - 1);
        if(parameter_index < class_template->parameters.size() &&
           class_template->parameters[parameter_index].kind ==
               TemplateParameterInfo::TP_NON_TYPE &&
           class_args[i].syntax.dependent &&
           !dependent_non_type_argument_syntax_is_direct_parameter(
               parameters,
               class_args[i])) {
          return false;
        }
        if(!dependent_non_type_template_arguments_are_direct_parameters(
               type_system,
               class_args[i].type,
               parameters,
               depth + 1)) {
          return false;
        }
      }
    }

    if(!dependent_non_type_template_arguments_are_direct_parameters(
           type_system,
           base->named_dependent_qualified_owner,
           parameters,
           depth + 1)) {
      return false;
    }
  }

  if(!dependent_non_type_template_arguments_are_direct_parameters(type_system,
                                                                  base->inner,
                                                                  parameters,
                                                                  depth + 1) ||
     !dependent_non_type_template_arguments_are_direct_parameters(type_system,
                                                                  base->owner,
                                                                  parameters,
                                                                  depth + 1)) {
    return false;
  }
  for(std::size_t i = 0; i < base->params.size(); ++i) {
    if(!dependent_non_type_template_arguments_are_direct_parameters(type_system,
                                                                    base->params[i],
                                                                    parameters,
                                                                    depth + 1)) {
      return false;
    }
  }
  return true;
}

std::size_t template_parameter_index_for_argument(
    const std::vector<TemplateParameterInfo> & parameters,
    std::size_t argument_index)
{
  std::size_t first_pack_parameter = parameters.size();
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      first_pack_parameter = i;
      break;
    }
  }
  if(first_pack_parameter != parameters.size() &&
     argument_index >= first_pack_parameter) {
    return first_pack_parameter;
  }
  return argument_index;
}

bool dependent_alias_argument_has_non_direct_dependent_non_type(
    template_api::TemplateTypeSystem & type_system,
    const DependentAliasTemplateArgumentSyntax & argument,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(argument.syntax.dependent &&
     !dependent_non_type_argument_syntax_is_direct_parameter(parameters,
                                                            argument)) {
    return true;
  }
  if(argument.type &&
     type_has_dependent_non_type_template_argument(type_system,
                                                   argument.type) &&
     !dependent_non_type_template_arguments_are_direct_parameters(
         type_system,
         argument.type,
         parameters)) {
    return true;
  }
  if(argument.syntax.resolved_type &&
     type_has_dependent_non_type_template_argument(type_system,
                                                   argument.syntax.resolved_type) &&
     !dependent_non_type_template_arguments_are_direct_parameters(
         type_system,
         argument.syntax.resolved_type,
         parameters)) {
    return true;
  }
  return false;
}

bool dependent_non_type_arguments_flow_into_alias_target(
    template_api::TemplateTypeSystem & type_system,
    const AliasTemplateDecl & alias_template,
    const std::vector<DependentAliasTemplateArgumentSyntax> & arguments,
    const std::vector<TemplateParameterInfo> & outer_parameters)
{
  if(!alias_template.resolved_type_pattern) {
    return true;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!dependent_alias_argument_has_non_direct_dependent_non_type(
           type_system,
           arguments[i],
           outer_parameters)) {
      continue;
    }
    const std::size_t parameter_index =
        template_parameter_index_for_argument(alias_template.parameters, i);
    if(parameter_index >= alias_template.parameters.size()) {
      return true;
    }
    std::vector<TemplateParameterInfo> single_parameter;
    single_parameter.push_back(alias_template.parameters[parameter_index]);
    if(alias_template_type_pattern_mentions_parameters(
           alias_template.resolved_type_pattern,
           single_parameter)) {
      return true;
    }
  }
  return false;
}

std::string alias_template_target_text(const CppAstNode & node)
{
  std::string text = trim_space(node_text(node));
  if(!text.empty()) {
    return text;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const std::string part = alias_template_target_text(node.children[i]);
    if(part.empty()) {
      continue;
    }
    if(!text.empty()) {
      text += " ";
    }
    text += part;
  }
  return trim_space(text);
}

std::vector<std::string> template_parameter_type_names(const TypePtr & type)
{
  std::vector<std::string> out;
  if(!type || type->kind != Type::TK_NAMED) {
    return out;
  }
  const auto append_unique =
      [&out](const std::string & raw) -> void
  {
    const std::string name = strip_elaborated_type_prefix(trim_space(raw));
    if(name.empty() ||
       std::find(out.begin(), out.end(), name) != out.end()) {
      return;
    }
    out.push_back(name);
  };
  append_unique(named_type_semantic_payload(type));
  append_unique(type->named_key);
  append_unique(type->named_display);
  return out;
}

bool lookup_template_bound_type(Scope & scope,
                                const std::vector<std::string> & names,
                                TypePtr & out)
{
  out.reset();
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(std::size_t i = 0; i < names.size(); ++i) {
      const std::string & name = names[i];
      if(current->template_bound_type_names.count(name) == 0) {
        continue;
      }
      std::map<std::string, TypePtr>::const_iterator found =
          current->named_types.find(name);
      if(found != current->named_types.end() && found->second) {
        out = found->second;
        return true;
      }
    }
  }
  return false;
}

const std::vector<TypePtr> * lookup_template_bound_type_pack(
    Scope & scope,
    const std::vector<std::string> & names)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(std::size_t i = 0; i < names.size(); ++i) {
      const std::string & name = names[i];
      if(current->template_bound_type_pack_names.count(name) == 0) {
        continue;
      }
      std::map<std::string, std::vector<TypePtr> >::const_iterator found =
          current->named_type_packs.find(name);
      if(found != current->named_type_packs.end()) {
        return &found->second;
      }
    }
  }
  return nullptr;
}

std::vector<std::string> type_pack_lookup_names(
    const TemplateArgument & argument,
    const std::string & arg_text)
{
  std::vector<std::string> names =
      template_parameter_type_names(argument.type);
  std::string pack_text;
  if(strip_trailing_pack_ellipsis(arg_text, pack_text)) {
    const std::string name = strip_elaborated_type_prefix(trim_space(pack_text));
    if(!name.empty() &&
       std::find(names.begin(), names.end(), name) == names.end()) {
      names.push_back(name);
    }
  }
  return names;
}

bool try_expand_alias_template_pattern_structurally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const AliasTemplateDecl & alias_template,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    std::string & expanded_text,
    TypePtr * expanded_type,
    bool allow_dependent_expansion,
    AliasSubstitutionFailure * substitution_failure)
{
  if(substitution_failure) {
    substitution_failure->reset();
  }
  if(expanded_type) {
    expanded_type->reset();
  }
  if(!alias_template.resolved_type_pattern ||
     !template_api::trailing_pack_accepts_argument_count(alias_template.parameters,
                                                         arg_texts.size())) {
    return false;
  }
  if(alias_template.type_id &&
     alias_template_target_mentions_parameters(*alias_template.type_id,
                                                alias_template.parameters) &&
     !alias_template_type_pattern_mentions_parameters(
         alias_template.resolved_type_pattern,
         alias_template.parameters)) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-structural-defer-source-dependent-target alias="
            << alias_template.name;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto type_is_dependent =
      [&type_system](const TypePtr & type) -> bool
      {
        return template_argument_semantics::type_depends_on_template_parameter(
            type_system,
            type);
      };
  const auto argument_text =
      [&type_system](const TemplateArgument & argument) -> std::string
      {
        return template_argument_text_for_matching(type_system, argument);
      };
  const auto type_text =
      [&type_system](const TypePtr & type) -> std::string
      {
        return specialization_argument_type_text(type_system, type);
      };
  std::vector<TemplateArgument> arguments;
  arguments.reserve(arg_texts.size());
  for(std::size_t i = 0; i < alias_template.parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = alias_template.parameters[i];
    if(parameter.kind != TemplateParameterInfo::TP_TYPE &&
       parameter.kind != TemplateParameterInfo::TP_NON_TYPE &&
       parameter.kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      return false;
    }
  }

  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ? argument_scope : match_scope;
  if(!template_api::resolve_template_arguments(
         services,
         effective_argument_scope,
         alias_template.parameters,
         arg_texts,
         arg_syntaxes,
         arguments,
         alias_template.declaring_scope ?
             template_api::make_template_environment(*alias_template.declaring_scope) :
             template_api::TemplateEnvironmentHandle())) {
    return false;
  }

  Scope member_alias_body_scope;
  template_api::TemplateEnvironmentHandle effective_body_scope =
      effective_argument_scope;
  if(prepare_alias_body_scope(services,
                              alias_template,
                              effective_argument_scope,
                              arguments,
                              member_alias_body_scope)) {
    effective_body_scope =
        template_api::make_template_environment(member_alias_body_scope);
  }

  std::size_t first_pack_argument = alias_template.parameters.size();
  for(std::size_t i = 0; i < alias_template.parameters.size(); ++i) {
    if(alias_template.parameters[i].parameter_pack) {
      first_pack_argument = i;
      break;
    }
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    const TemplateParameterInfo * parameter = nullptr;
    if(first_pack_argument != alias_template.parameters.size() &&
       i >= first_pack_argument) {
      parameter = &alias_template.parameters[first_pack_argument];
    } else if(i < alias_template.parameters.size()) {
      parameter = &alias_template.parameters[i];
    }
    if(!parameter) {
      return false;
    }
    if(parameter->kind == TemplateParameterInfo::TP_TYPE) {
      if(arguments[i].kind != TemplateArgument::TA_TYPE || !arguments[i].type) {
        return false;
      }
    } else if(parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
      if(arguments[i].kind != TemplateArgument::TA_VALUE) {
        return false;
      }
    } else if(parameter->kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      if(arguments[i].kind != TemplateArgument::TA_CLASS_TEMPLATE &&
         arguments[i].kind != TemplateArgument::TA_ALIAS_TEMPLATE) {
        return false;
      }
    } else {
      return false;
    }
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!arguments[i].dependent) {
      continue;
    }
    const std::string text =
        i < arg_texts.size() ? arg_texts[i] : arguments[i].text;
    if(text.empty()) {
      return false;
    }
    const bool mentions_placeholders =
        template_argument_semantics::text_mentions_template_placeholders(
            services, effective_argument_scope, text);
    const bool mentions_dependent_bindings =
        template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
            services, effective_argument_scope, text);
    if(!mentions_placeholders &&
       !mentions_dependent_bindings &&
       !template_argument_semantics::scope_has_template_placeholders(
           services, effective_argument_scope)) {
      return false;
    }
  }

  AliasTemplateDecl::StableAliasExpansionKey alias_expansion_cache_key;
  const bool has_alias_expansion_cache_key =
      !witness::source_capture_enabled(services.witness_context) &&
      stable_alias_expansion_key_for_arguments(
          match_scope.require(),
          effective_argument_scope.require(),
          arguments,
          allow_dependent_expansion,
          alias_expansion_cache_key);
  if(has_alias_expansion_cache_key) {
    std::map<AliasTemplateDecl::StableAliasExpansionKey,
             AliasTemplateDecl::StableAliasExpansionValue>::const_iterator
        cached = alias_template.stable_alias_expansions.find(
            alias_expansion_cache_key);
    if(cached != alias_template.stable_alias_expansions.end()) {
      if(cached->second.kind ==
         AliasTemplateDecl::StableAliasExpansionValue::EK_SUCCESS) {
        expanded_text = cached->second.expanded_text;
        if(expanded_type) {
          *expanded_type = cached->second.expanded_type;
        }
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "expand-alias-structural-cache-hit alias="
                << alias_template.name
                << " result=success";
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return true;
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "expand-alias-structural-cache-hit alias="
              << alias_template.name
              << " result=dependent-defer";
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }
  }
  const auto cache_alias_success =
      [&](const std::string & text, const TypePtr & type) -> void
  {
    if(!has_alias_expansion_cache_key || text.empty() || !type) {
      return;
    }
    AliasTemplateDecl::StableAliasExpansionValue value;
    value.kind = AliasTemplateDecl::StableAliasExpansionValue::EK_SUCCESS;
    value.expanded_text = text;
    value.expanded_type = type;
    alias_template.stable_alias_expansions[alias_expansion_cache_key] = value;
  };
  const auto cache_alias_dependent_defer =
      [&]() -> void
  {
    if(!has_alias_expansion_cache_key) {
      return;
    }
    AliasTemplateDecl::StableAliasExpansionValue value;
    value.kind =
        AliasTemplateDecl::StableAliasExpansionValue::EK_DEPENDENT_DEFER;
    alias_template.stable_alias_expansions[alias_expansion_cache_key] = value;
  };

  const auto try_expand_known_conditional_alias = [&]() -> bool
  {
    if(arg_texts.size() != 3 ||
       alias_template.parameters.size() != 3 ||
       (alias_template.name != "conditional_t" &&
        alias_template.name != "__conditional_t") ||
       alias_template.parameters[0].kind != TemplateParameterInfo::TP_NON_TYPE ||
       alias_template.parameters[1].kind != TemplateParameterInfo::TP_TYPE ||
       alias_template.parameters[2].kind != TemplateParameterInfo::TP_TYPE ||
       arguments.size() != 3 ||
       arguments[0].kind != TemplateArgument::TA_VALUE ||
       arguments[0].dependent) {
      return false;
    }

    const std::size_t selected_index = arguments[0].value ? 1 : 2;
    const TemplateArgument & selected = arguments[selected_index];
    if(selected.kind != TemplateArgument::TA_TYPE || !selected.type) {
      return false;
    }

    TypePtr selected_type = selected.type;
    template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
        services,
        effective_body_scope,
        selected_type);
    if(!selected_type) {
      return false;
    }

    if(expanded_type) {
      *expanded_type = selected_type;
    }
    expanded_text = type_text(selected_type);
    if(expanded_text.empty()) {
      expanded_text = selected.text;
    }
    if(expanded_text.empty() &&
       arg_syntaxes &&
       selected_index < arg_syntaxes->size()) {
      expanded_text = (*arg_syntaxes)[selected_index].text;
    }
    if(expanded_text.empty()) {
      return false;
    }
    cache_alias_success(expanded_text, selected_type);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-conditional-known alias="
            << alias_template.name
            << " selected=" << selected_index
            << " type=" << describe_type(selected_type)
            << " text=" << expanded_text;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return true;
  };
  if(try_expand_known_conditional_alias()) {
    return true;
  }

  const auto find_type_parameter =
      [&](const TypePtr & type) -> const TemplateParameterInfo *
  {
    if(!type || type->kind != Type::TK_NAMED) {
      return nullptr;
    }
    const TemplateParameterInfo * parameter =
        find_template_parameter(alias_template.parameters, type->named_key);
    if(!parameter && type->named_display != type->named_key) {
      parameter = find_template_parameter_by_name(alias_template.parameters,
                                                  type->named_display);
    }
    return parameter;
  };
  const auto find_non_type_parameter =
      [&](const TemplateArgument & argument) -> const TemplateParameterInfo *
  {
    if(argument.kind != TemplateArgument::TA_VALUE) {
      return nullptr;
    }
    std::string parameter_name;
    if(!strip_trailing_pack_ellipsis(argument.text, parameter_name)) {
      parameter_name = trim_space(argument.text);
    }
    if(parameter_name.empty()) {
      return nullptr;
    }
    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(alias_template.parameters, parameter_name);
    return parameter && parameter->kind == TemplateParameterInfo::TP_NON_TYPE ?
               parameter :
               nullptr;
  };
  const auto find_template_template_parameter =
      [&](const TemplateArgument & argument) -> const TemplateParameterInfo *
  {
    if(argument.kind != TemplateArgument::TA_CLASS_TEMPLATE &&
       argument.kind != TemplateArgument::TA_ALIAS_TEMPLATE) {
      return nullptr;
    }
    std::string parameter_name = trim_space(argument.text);
    if(parameter_name.empty()) {
      return nullptr;
    }
    const std::string unqualified =
        semantic_utils::unqualified_member_name(parameter_name);
    if(!unqualified.empty()) {
      parameter_name = unqualified;
    }
    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(alias_template.parameters,
                                        parameter_name);
    return parameter &&
           parameter->kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE ?
               parameter :
               nullptr;
  };
  const auto metadata_argument_has_pack_expansion =
      [](const template_api::TemplateNamedTypeMetadata & info,
         std::size_t index) -> bool
  {
    if(index >= info.instantiation_arg_texts.size()) {
      return false;
    }
    std::string element_text;
    return strip_trailing_pack_ellipsis(info.instantiation_arg_texts[index],
                                        element_text);
  };
  const auto argument_needs_alias_target_scope =
      [&](const TemplateArgument & argument) -> bool
  {
    if(argument.kind == TemplateArgument::TA_TYPE &&
       argument.type &&
       find_type_parameter(argument.type)) {
      return true;
    }
    if(argument.kind == TemplateArgument::TA_VALUE &&
       find_non_type_parameter(argument)) {
      return true;
    }
    return false;
  };
  const auto argument_has_dependent_source_syntax =
      [](const TemplateArgument & argument) -> bool
  {
    if(!argument.dependent) {
      return false;
    }
    if(argument.expression) {
      return true;
    }
    return argument.source_syntax && argument.source_syntax->dependent;
  };
  const auto source_argument_needs_alias_target_scope =
      [&](const TemplateArgument & argument) -> bool
  {
    if(argument_needs_alias_target_scope(argument)) {
      return true;
    }
    return argument_has_dependent_source_syntax(argument);
  };
  const auto dependent_alias_arguments_need_alias_target_scope =
      [&](const std::vector<DependentAliasTemplateArgumentSyntax> & alias_args) -> bool
  {
    for(std::size_t i = 0; i < alias_args.size(); ++i) {
      if(alias_args[i].type && find_type_parameter(alias_args[i].type)) {
        return true;
      }
      if(alias_args[i].syntax.dependent || alias_args[i].syntax.expression) {
        return true;
      }
    }
    return false;
  };
  const auto current_owner_member_alias =
      [&](AliasTemplateDecl * alias_template_decl) -> AliasTemplateDecl *
  {
    if(!alias_template_decl ||
       !alias_template_decl->declaring_scope ||
       !alias_template_decl->declaring_scope->class_info ||
       !alias_template_decl->declaring_scope->class_info->source_template ||
       alias_template_decl->name.empty() ||
       !effective_body_scope.valid()) {
      return alias_template_decl;
    }
    ClassInfo * original_owner =
        alias_template_decl->declaring_scope->class_info;
    AliasTemplateDecl * current =
        template_argument_semantics::lookup_alias_template(
            services,
            effective_body_scope.require(),
            alias_template_decl->name);
    if(!current ||
       current == alias_template_decl ||
       !current->declaring_scope ||
       !current->declaring_scope->class_info) {
      return alias_template_decl;
    }
    ClassInfo * current_owner = current->declaring_scope->class_info;
    if(current_owner->source_template == original_owner->source_template) {
      return current;
    }
    return alias_template_decl;
  };

  TypePtr direct_member_alias_owner;
  std::vector<std::string> direct_member_alias_members;
  bool direct_member_alias_leading_typename = false;
  const bool direct_dependent_member_alias =
      named_type_dependent_qualified_member(alias_template.resolved_type_pattern,
                                            direct_member_alias_owner,
                                            direct_member_alias_members,
                                            direct_member_alias_leading_typename) &&
      !direct_member_alias_members.empty();
  const bool cacheable_non_dependent_arguments =
      !template_arguments_are_dependent(
          arguments,
          [&type_is_dependent](const TypePtr & type)
          {
            return type_is_dependent(type);
          });
  AliasTemplateDecl::StableSubstitutionKey substitution_failure_cache_key;
  const bool has_substitution_failure_cache_key =
      direct_dependent_member_alias &&
      cacheable_non_dependent_arguments &&
      stable_substitution_key_for_arguments(arguments,
                                            substitution_failure_cache_key);
  if(has_substitution_failure_cache_key) {
    std::map<AliasTemplateDecl::StableSubstitutionKey,
             AliasTemplateDecl::StableSubstitutionFailure>::const_iterator
        cached = alias_template.stable_substitution_failures.find(
            substitution_failure_cache_key);
    if(cached != alias_template.stable_substitution_failures.end()) {
      if(substitution_failure) {
        substitution_failure->reset();
        substitution_failure->kind =
            AliasSubstitutionFailure::SF_MISSING_NONDEPENDENT_QUALIFIED_MEMBER_TYPE;
        substitution_failure->alias_name = alias_template.name;
        substitution_failure->owner_type_key = cached->second.owner_type_key;
        substitution_failure->owner_type_display = cached->second.owner_type_display;
        substitution_failure->member_name = cached->second.member_name;
        substitution_failure->stable_for_reuse = true;
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "expand-alias-structural-substitution-cache-hit alias="
              << alias_template.name
              << " arg-count=" << substitution_failure_cache_key.arguments.size()
              << " owner=" << cached->second.owner_type_display
              << " member=" << cached->second.member_name;
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }
  }

  bool structural_substitution_failure = false;
  int substitution_depth = 0;
  std::unique_ptr<Scope> alias_target_scope_storage;
  const auto instantiated_alias_owner = [&](Scope & target_scope) -> ClassInfo *
  {
    if(!alias_template.declaring_scope ||
       !alias_template.declaring_scope->class_info) {
      return nullptr;
    }
    ClassInfo * declared_owner = alias_template.declaring_scope->class_info;
    const auto matches_declared_owner =
        [&](ClassInfo * info) -> bool
    {
      return info &&
             info != declared_owner &&
             info->name == declared_owner->name &&
             info->source_template;
    };
    if(effective_argument_scope.valid()) {
      for(Scope * current = &effective_argument_scope.require();
          current;
          current = current->parent) {
        if(matches_declared_owner(current->class_info)) {
          return current->class_info;
        }
      }
    }
    std::map<std::string, TypePtr>::const_iterator found =
        target_scope.named_types.find(declared_owner->name);
    if(found != target_scope.named_types.end()) {
      ClassInfo * bound_info =
          template_api::find_named_type_class_info(type_system.model,
                                                   found->second);
      if(matches_declared_owner(bound_info)) {
        return bound_info;
      }
    }
    return nullptr;
  };
  const auto ensure_alias_target_scope = [&]() -> Scope &
  {
    if(!alias_target_scope_storage) {
      alias_target_scope_storage.reset(
          new Scope(alias_template.declaring_scope ?
                        alias_template.declaring_scope :
                        &match_scope.require(),
                    "",
                    false));
      if(alias_template.declaring_scope) {
        template_api::overlay_instantiation_use_scope_bindings(
            *alias_target_scope_storage,
            effective_argument_scope.require(),
            alias_template.declaring_scope);
      }
      if(ClassInfo * owner =
             instantiated_alias_owner(*alias_target_scope_storage)) {
        alias_target_scope_storage->class_info = owner;
        if(!owner->name.empty() && owner->type) {
          template_scope::bind_named_type(*alias_target_scope_storage,
                                          owner->name,
                                          owner->type);
        }
      }
      template_api::bind_template_arguments_into_scope(
          services,
          *alias_target_scope_storage,
          alias_template.parameters,
          arguments);
    }
    return *alias_target_scope_storage;
  };
  const auto selected_class_arguments_have_defaulted_true_bool_value =
      [&](const std::vector<TemplateArgument> & selected_arguments) -> bool
  {
    bool has_true_bool_value = false;
    for(std::size_t i = 0; i < selected_arguments.size(); ++i) {
      const TemplateArgument & argument = selected_arguments[i];
      if(argument.kind == TemplateArgument::TA_VALUE) {
        TypePtr value_type =
            strip_top_level_cv(remove_reference_type(argument.type));
        if(argument.dependent ||
           (value_type && !is_bool_type(value_type)) ||
           argument.value == 0) {
          return false;
        }
        has_true_bool_value = true;
        continue;
      }
      if(argument.kind == TemplateArgument::TA_TYPE) {
        TypePtr type_argument =
            strip_top_level_cv(remove_reference_type(argument.type));
        if(type_argument &&
           (is_bool_type(type_argument) || is_void_type(type_argument))) {
          continue;
        }
        if(argument.source_defaulted) {
          continue;
        }
        return false;
      }
      return false;
    }
    return has_true_bool_value;
  };
  const auto value_argument_has_structured_source =
      [](const TemplateArgument & argument) -> bool
  {
    if(argument.expression) {
      return true;
    }
    return argument.source_syntax &&
           (argument.source_syntax->expression ||
            argument.source_syntax->type_id ||
            argument.source_syntax->template_id);
  };
  const auto try_evaluate_alias_target_value_argument =
      [&](const TemplateArgument & argument, TemplateArgument & out) -> bool
  {
	    out = argument;
	    if(argument.kind != TemplateArgument::TA_VALUE ||
	       !source_argument_needs_alias_target_scope(argument) ||
	       !value_argument_has_structured_source(argument)) {
	      return false;
	    }

    Scope & eval_scope =
        source_argument_needs_alias_target_scope(argument) ?
            ensure_alias_target_scope() :
            effective_argument_scope.require();
    if(out.type) {
      TypePtr resolved_type = out.type;
      if(template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
             services,
             template_api::make_template_environment(eval_scope),
             resolved_type)) {
        out.type = resolved_type;
      }
    }
    if(!out.type || type_is_dependent(out.type)) {
      return false;
    }

    long long value = 0;
    std::string eval_error;
    template_argument_semantics::NonTypeArgumentStatus status =
        template_argument_semantics::NT_ARG_PARSE_FAILED;
    if(argument.expression) {
      status =
          template_argument_semantics::evaluate_non_type_argument_expression(
              services,
              template_api::make_template_environment(eval_scope),
              *argument.expression,
              value,
              &eval_error,
              out.type);
    }
    if(status != template_argument_semantics::NT_ARG_EVALUATED &&
       argument.source_syntax &&
       (argument.source_syntax->expression ||
        argument.source_syntax->type_id ||
        argument.source_syntax->template_id)) {
      status =
          template_argument_semantics::evaluate_non_type_argument_syntax(
              services,
              template_api::make_template_environment(eval_scope),
              *argument.source_syntax,
              value,
              &eval_error,
              out.type);
    }
    if(status != template_argument_semantics::NT_ARG_EVALUATED) {
      return false;
    }

    out.value = value;
    out.dependent = false;
    out.text.clear();
    out.expression.reset();
    return true;
  };
  const auto dependent_class_instantiation_needs_structured_expansion =
      [&](const TypePtr & type) -> bool
  {
    if(!type_has_dependent_non_type_template_argument(type_system, type)) {
      return false;
    }
    template_api::TemplateNamedTypeMetadata info;
    return template_api::describe_named_type_metadata(type_system.model,
                                                      type,
                                                      info) &&
           info.source_template &&
           !info.instantiation_arguments.empty();
  };
  const auto mark_structural_substitution_failure =
      [&](Scope * lookup_scope, const std::string & member_name) -> void
  {
    const ClassInfo * lookup_class =
        lookup_scope ? lookup_scope->class_info : nullptr;
    const bool stable_member_lookup =
        lookup_class &&
        (lookup_class->complete || lookup_class->reference_members_collected) &&
        !lookup_class->full_member_collection_in_progress &&
        !lookup_class->reference_member_collection_in_progress;
    if(!direct_dependent_member_alias ||
       substitution_depth != 1 ||
       !stable_member_lookup ||
       !lookup_class->type ||
       !is_simple_identifier_text(member_name)) {
      return;
    }
    structural_substitution_failure = true;
    TypePtr owner_type = strip_top_level_cv(lookup_class->type);
    std::string owner_type_key;
    std::string owner_type_display;
    if(owner_type && owner_type->kind == Type::TK_NAMED) {
      owner_type_key = owner_type->named_key;
      owner_type_display = owner_type->named_display;
    } else {
      owner_type_display = describe_type(lookup_class->type);
    }
    if(substitution_failure) {
      substitution_failure->reset();
      substitution_failure->kind =
          AliasSubstitutionFailure::SF_MISSING_NONDEPENDENT_QUALIFIED_MEMBER_TYPE;
      substitution_failure->alias_name = alias_template.name;
      substitution_failure->owner_type_key = owner_type_key;
      substitution_failure->owner_type_display = owner_type_display;
      substitution_failure->member_name = member_name;
      substitution_failure->stable_for_reuse = true;
    }
    if(has_substitution_failure_cache_key) {
      AliasTemplateDecl::StableSubstitutionFailure cached_failure;
      cached_failure.owner_type_key = owner_type_key;
      cached_failure.owner_type_display = owner_type_display;
      cached_failure.member_name = member_name;
      alias_template.stable_substitution_failures[substitution_failure_cache_key] =
          cached_failure;
    }
  };

  struct ScopedSubstitutionDepth
  {
    explicit ScopedSubstitutionDepth(int & depth_in)
      : depth(depth_in)
    {
      ++depth;
    }

    ~ScopedSubstitutionDepth()
    {
      --depth;
    }

    int & depth;
  };

  std::function<bool(const TypePtr &, TypePtr &)> substitute_type =
      [&](const TypePtr & pattern, TypePtr & out) -> bool
  {
    ScopedSubstitutionDepth scoped_substitution_depth(substitution_depth);
    if(!pattern) {
      return false;
    }
    const auto try_expand_make_integer_seq_builtin =
        [&](const TypePtr & candidate, TypePtr & expanded) -> bool
    {
      expanded.reset();
      TypePtr candidate_base = strip_top_level_cv(candidate);
      if(!candidate_base ||
         candidate_base->kind != Type::TK_NAMED ||
         candidate_base->named_semantic_kind != Type::NSK_DEPENDENT_TYPE) {
        return false;
      }

      std::string candidate_text =
          candidate_base->named_display.empty() ? candidate_base->named_key :
                                                  candidate_base->named_display;
      if(candidate_text.empty()) {
        candidate_text = describe_type(candidate_base);
      }
      QualifiedName builtin_name;
      std::vector<std::string> builtin_args;
      if(!semantic_utils::split_top_level_template_id_text(candidate_text,
                                                           builtin_name,
                                                           builtin_args) ||
         builtin_name.name != "__make_integer_seq" ||
         builtin_args.size() != 3) {
        return false;
      }

      ClassTemplateDecl * sequence_template =
          template_argument_semantics::lookup_class_template(
              services,
              effective_body_scope.require(),
              trim_space(builtin_args[0]));
      if(!sequence_template) {
        return false;
      }

      TypePtr value_type;
      const std::string value_type_text = trim_space(builtin_args[1]);
      const TemplateParameterInfo * value_type_parameter =
          find_template_parameter_by_name(alias_template.parameters,
                                          value_type_text);
      if(value_type_parameter &&
         value_type_parameter->kind == TemplateParameterInfo::TP_TYPE) {
        const std::size_t index = static_cast<std::size_t>(
            value_type_parameter - &alias_template.parameters[0]);
        if(index >= arguments.size() ||
           arguments[index].kind != TemplateArgument::TA_TYPE ||
           !arguments[index].type) {
          return false;
        }
        value_type = arguments[index].type;
      } else if(lookup_template_bound_type(effective_body_scope.require(),
                                           std::vector<std::string>(1,
                                                                    value_type_text),
                                           value_type) &&
                value_type) {
      } else {
        return false;
      }
      template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
          services,
          effective_body_scope,
          value_type);
      if(!value_type || type_is_dependent(value_type)) {
        return false;
      }

      long long count = 0;
      const std::string count_text = trim_space(builtin_args[2]);
      const TemplateParameterInfo * count_parameter =
          find_template_parameter_by_name(alias_template.parameters,
                                          count_text);
      if(count_parameter &&
         count_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
        const std::size_t index = static_cast<std::size_t>(
            count_parameter - &alias_template.parameters[0]);
        if(index >= arguments.size() ||
           arguments[index].kind != TemplateArgument::TA_VALUE ||
           arguments[index].dependent) {
          return false;
        }
        count = arguments[index].value;
      } else {
        std::string eval_error;
        const template_argument_semantics::NonTypeArgumentStatus status =
            template_argument_semantics::evaluate_non_type_argument_text(
                services,
                effective_body_scope,
                count_text,
                count,
                &eval_error,
                value_type);
        if(status != template_argument_semantics::NT_ARG_EVALUATED) {
          return false;
        }
      }
      if(count < 0) {
        return false;
      }

      std::vector<TemplateArgument> sequence_arguments;
      sequence_arguments.reserve(static_cast<std::size_t>(count) + 1);
      TemplateArgument type_argument;
      type_argument.kind = TemplateArgument::TA_TYPE;
      type_argument.type = value_type;
      type_argument.text = type_text(value_type);
      sequence_arguments.push_back(type_argument);
      for(long long i = 0; i < count; ++i) {
        TemplateArgument value_argument;
        value_argument.kind = TemplateArgument::TA_VALUE;
        value_argument.type = value_type;
        value_argument.value = i;
        value_argument.text = std::to_string(i);
        sequence_arguments.push_back(value_argument);
      }

      template_api::TemplateSelectedClassTemplateIdRequest request;
      request.lookup.scope = &match_scope.require();
      request.lookup.name.name = sequence_template->name;
      request.lookup.allow_class_templates = true;
      request.class_template = sequence_template;
      request.resolved_arguments = sequence_arguments;
      request.source_arg_texts.reserve(sequence_arguments.size());
      for(std::size_t i = 0; i < sequence_arguments.size(); ++i) {
        request.source_arg_texts.push_back(argument_text(sequence_arguments[i]));
      }
      return type_system.resolve_selected_class_template_id(request, expanded) &&
             expanded;
    };
    if(try_expand_make_integer_seq_builtin(pattern, out)) {
      return true;
    }

    if(pattern->kind == Type::TK_CV) {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = apply_cv(inner, pattern->cv_const, pattern->cv_volatile);
      return true;
    }

    void * dependent_alias_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> dependent_alias_args;
    if(named_type_dependent_alias_template(pattern,
                                           dependent_alias_template_decl,
                                           dependent_alias_args) &&
       dependent_alias_template_decl) {
      TypePtr substituted_alias;
      if(template_argument_semantics::substitute_type(
             effective_body_scope.require(),
             pattern,
             alias_template.parameters,
             arguments,
             substituted_alias) &&
         substituted_alias) {
        void * substituted_alias_template_decl = nullptr;
        std::vector<DependentAliasTemplateArgumentSyntax> substituted_alias_args;
        if(!type_equals(substituted_alias, pattern) &&
           named_type_dependent_alias_template(substituted_alias,
                                               substituted_alias_template_decl,
                                               substituted_alias_args) &&
           substituted_alias_template_decl &&
           substituted_alias_template_decl != &alias_template) {
          AliasTemplateDecl * nested_alias_template =
              current_owner_member_alias(
                  static_cast<AliasTemplateDecl *>(substituted_alias_template_decl));
          std::vector<std::string> nested_arg_texts;
          std::vector<TemplateArgumentSyntax> nested_arg_syntaxes;
          nested_arg_texts.reserve(substituted_alias_args.size());
          nested_arg_syntaxes.reserve(substituted_alias_args.size());
          for(std::size_t i = 0; i < substituted_alias_args.size(); ++i) {
            nested_arg_texts.push_back(substituted_alias_args[i].text);
            nested_arg_syntaxes.push_back(substituted_alias_args[i].syntax);
          }
          if(type_has_dependent_non_type_template_argument(type_system,
                                                           substituted_alias) &&
             !dependent_non_type_template_arguments_are_direct_parameters(
                 type_system,
                 substituted_alias,
                 alias_template.parameters) &&
             dependent_non_type_arguments_flow_into_alias_target(
                 type_system,
                 *nested_alias_template,
                 substituted_alias_args,
                 alias_template.parameters)) {
            out = substituted_alias;
            return true;
          }
          template_api::TemplateEnvironmentHandle nested_argument_scope =
              dependent_alias_arguments_need_alias_target_scope(dependent_alias_args) ?
                  template_api::make_template_environment(ensure_alias_target_scope()) :
                  effective_argument_scope;
          std::string nested_expanded_text;
          TypePtr nested_expanded_type;
          if(try_expand_alias_template_pattern_structurally(
                 services,
                 match_scope,
                 *nested_alias_template,
                 nested_arg_texts,
                 &nested_arg_syntaxes,
                 nested_argument_scope,
                 nested_expanded_text,
                 &nested_expanded_type,
                 allow_dependent_expansion,
	                 substitution_failure) &&
	             nested_expanded_type) {
	            if(alias_template_type_pattern_mentions_parameters(
	                   nested_expanded_type,
	                   nested_alias_template->parameters)) {
	              out = substituted_alias;
	              return true;
	            }
	            out = nested_expanded_type;
	            return true;
	          }
        }
        if(type_is_dependent(substituted_alias) &&
           type_has_dependent_non_type_template_argument(type_system,
                                                         substituted_alias) &&
           !dependent_non_type_template_arguments_are_direct_parameters(
               type_system,
               substituted_alias,
               alias_template.parameters)) {
          out = substituted_alias;
          return true;
        }
        out = substituted_alias;
        return true;
      }
    }

    TypePtr dependent_owner;
    std::vector<std::string> dependent_members;
    std::vector<TemplateIdSyntax> dependent_member_template_ids;
    bool dependent_leading_typename = false;
    if(named_type_dependent_qualified_member(pattern,
                                             dependent_owner,
                                             dependent_members,
                                             dependent_leading_typename,
                                             &dependent_member_template_ids)) {
      TypePtr substituted_owner;
      if(!substitute_type(dependent_owner, substituted_owner) ||
         !substituted_owner) {
        return false;
      }
      if(type_is_dependent(substituted_owner)) {
        std::string display;
        const std::string owner_text = trim_space(type_text(substituted_owner));
        if(!owner_text.empty() && !dependent_members.empty()) {
          display = dependent_leading_typename ? "typename " : "";
          display += owner_text;
          for(std::size_t i = 0; i < dependent_members.size(); ++i) {
            display += "::";
            display += dependent_members[i];
          }
        }
        out = make_dependent_qualified_member_type(display.empty() ?
                                                       pattern->named_display :
                                                       display,
                                                   substituted_owner,
                                                   dependent_members,
                                                   dependent_leading_typename,
                                                   dependent_member_template_ids);
        return out != nullptr;
      }

      Scope * current = nullptr;
      if(!type_system.prepare_named_type_member_scope(
             match_scope,
             substituted_owner,
             current) ||
         !current) {
        return false;
      }
      for(std::size_t i = 0; i < dependent_members.size(); ++i) {
        const std::string member_name = trim_space(dependent_members[i]);
        if(member_name.empty()) {
          return false;
        }
        if(i + 1 != dependent_members.size()) {
          if(Scope * direct_namespace =
                 template_api::resolve_direct_namespace_in_inline_namespaces(
                     *current,
                     member_name)) {
            current = direct_namespace;
            continue;
          }
        }
        TypePtr member_type;
        if(current->class_info) {
          std::map<std::string, TypePtr>::const_iterator found_member =
              current->named_types.find(member_name);
          if(found_member != current->named_types.end()) {
            member_type = found_member->second;
          }
        } else {
          member_type =
              template_api::lookup_direct_named_type_in_inline_namespaces(
                  *current,
                  member_name);
        }
        if(!member_type &&
           current->class_info &&
           !current->class_info->bases.empty()) {
          type_system.resolve_member_type_lookup(match_scope.require(),
                                                 *current->class_info,
                                                 member_name,
                                                 true,
                                                 member_type);
        }
        if(!member_type) {
          mark_structural_substitution_failure(current, member_name);
          return false;
        }
        if(i + 1 == dependent_members.size()) {
          template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
              services,
              template_api::make_template_environment(*current),
              member_type);
          out = member_type;
          return true;
        }
        if(!type_system.prepare_named_type_member_scope(
               match_scope,
               member_type,
               current) ||
           !current) {
          return false;
        }
      }
      return false;
    }

    const TemplateParameterInfo * parameter = find_type_parameter(pattern);
    if(parameter) {
      if(parameter->kind != TemplateParameterInfo::TP_TYPE ||
         parameter->parameter_pack) {
        return false;
      }
      const std::size_t index =
          static_cast<std::size_t>(parameter - &alias_template.parameters[0]);
      if(index >= arguments.size() ||
         arguments[index].kind != TemplateArgument::TA_TYPE ||
         !arguments[index].type) {
        return false;
      }
      TypePtr argument_type = arguments[index].type;
      template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
          services,
          effective_body_scope,
          argument_type);
      out = argument_type;
      return true;
    }

    const TypePtr pattern_base = strip_top_level_cv(pattern);
    TypePtr scope_bound_type;
    const std::vector<std::string> pattern_parameter_names =
        template_parameter_type_names(pattern_base);
    if((lookup_template_bound_type(effective_body_scope.require(),
                                   pattern_parameter_names,
                                   scope_bound_type) ||
        lookup_template_bound_type(effective_argument_scope.require(),
                                   pattern_parameter_names,
                                   scope_bound_type)) &&
       scope_bound_type) {
      out = scope_bound_type;
      return true;
    }
    const bool dependent_builtin_transform =
        pattern_base &&
        pattern_base->kind == Type::TK_NAMED &&
        pattern_base->named_semantic_kind == Type::NSK_DEPENDENT_TYPE &&
        pattern_base->named_semantic_payload.find("$builtin-type-transform:") == 0;
    if(dependent_builtin_transform) {
      TypePtr resolved = pattern;
      if(pattern_base->inner) {
        TypePtr substituted_inner;
        if(substitute_type(pattern_base->inner, substituted_inner) &&
           substituted_inner) {
          TypePtr substituted_pattern(new Type(*pattern_base));
          substituted_pattern->inner = substituted_inner;
          resolved = substituted_pattern;
        }
      }
      if(template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
             services,
             effective_body_scope,
             resolved) &&
         resolved &&
         !type_is_dependent(resolved)) {
        out = resolved;
        return true;
      }
    }

    if(pattern_base &&
       pattern_base->kind == Type::TK_NAMED &&
       type_is_dependent(pattern_base)) {
      TypePtr substituted_pattern;
      if(template_argument_semantics::substitute_type(
             effective_body_scope.require(),
             pattern,
             alias_template.parameters,
             arguments,
             substituted_pattern) &&
         substituted_pattern &&
         !type_equals(substituted_pattern, pattern)) {
        template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
            services,
            effective_body_scope,
            substituted_pattern);
        if(substituted_pattern && !type_is_dependent(substituted_pattern)) {
          out = substituted_pattern;
          return true;
        }
        if(allow_dependent_expansion &&
           !dependent_class_instantiation_needs_structured_expansion(pattern)) {
          out = substituted_pattern;
          return true;
        }
      }
    }

    template_api::TemplateNamedTypeMetadata named_info;
    if(template_api::describe_named_type_metadata(type_system.model,
                                                  pattern,
                                                  named_info) &&
       named_info.source_template &&
       !named_info.instantiation_arguments.empty()) {
      std::vector<TemplateArgument> substituted_arguments;
      substituted_arguments.reserve(
          std::max(named_info.instantiation_arguments.size(), arguments.size()));
      bool selected_arguments_need_alias_target_scope = false;
      for(std::size_t i = 0; i < named_info.instantiation_arguments.size(); ++i) {
        TemplateArgument argument = named_info.instantiation_arguments[i];
        if(source_argument_needs_alias_target_scope(argument)) {
          selected_arguments_need_alias_target_scope = true;
        }
        if(argument.kind == TemplateArgument::TA_TYPE) {
          if(metadata_argument_has_pack_expansion(named_info, i)) {
            const std::vector<std::string> pack_lookup_names =
                type_pack_lookup_names(
                    argument,
                    i < named_info.instantiation_arg_texts.size() ?
                        named_info.instantiation_arg_texts[i] :
                        std::string());
              const std::vector<TypePtr> * bound_pack =
                  lookup_template_bound_type_pack(effective_body_scope.require(),
                                                  pack_lookup_names);
              if(!bound_pack) {
                bound_pack =
                    lookup_template_bound_type_pack(effective_argument_scope.require(),
                                                    pack_lookup_names);
              }
              if(bound_pack) {
              for(std::size_t j = 0; j < bound_pack->size(); ++j) {
                TemplateArgument pack_argument;
                pack_argument.kind = TemplateArgument::TA_TYPE;
                pack_argument.type = (*bound_pack)[j];
                pack_argument.dependent = type_is_dependent(pack_argument.type);
                pack_argument.text = argument_text(pack_argument);
                substituted_arguments.push_back(pack_argument);
              }
              continue;
            }
          }

          const TemplateParameterInfo * argument_parameter =
              find_type_parameter(argument.type);
          if(argument_parameter && argument_parameter->parameter_pack) {
            if(!metadata_argument_has_pack_expansion(named_info, i)) {
              return false;
            }
            const std::size_t pack_index = static_cast<std::size_t>(
                argument_parameter - &alias_template.parameters[0]);
            if(pack_index > arguments.size()) {
              return false;
            }
            for(std::size_t j = pack_index; j < arguments.size(); ++j) {
              TemplateArgument pack_argument = arguments[j];
              if(pack_argument.kind != TemplateArgument::TA_TYPE ||
                 !pack_argument.type ||
                 type_is_dependent(pack_argument.type)) {
                return false;
              }
              pack_argument.dependent = false;
              pack_argument.text.clear();
              pack_argument.text = argument_text(pack_argument);
              substituted_arguments.push_back(pack_argument);
            }
            continue;
          }
        }
        if(argument.kind == TemplateArgument::TA_VALUE) {
          const TemplateParameterInfo * argument_parameter =
              find_non_type_parameter(argument);
          if(argument_parameter) {
            const std::size_t value_index = static_cast<std::size_t>(
                argument_parameter - &alias_template.parameters[0]);
            if(argument_parameter->parameter_pack) {
              if(!metadata_argument_has_pack_expansion(named_info, i)) {
                return false;
              }
              if(value_index > arguments.size()) {
                return false;
              }
              for(std::size_t j = value_index; j < arguments.size(); ++j) {
                TemplateArgument pack_argument = arguments[j];
                if(pack_argument.kind != TemplateArgument::TA_VALUE) {
                  return false;
                }
                if(pack_argument.text.empty()) {
                  pack_argument.text = argument_text(pack_argument);
                }
                substituted_arguments.push_back(pack_argument);
              }
              continue;
            }
            if(value_index >= arguments.size() ||
               arguments[value_index].kind != TemplateArgument::TA_VALUE) {
              return false;
            }
            TemplateArgument value_argument = arguments[value_index];
            if(value_argument.text.empty()) {
              value_argument.text = argument_text(value_argument);
            }
            substituted_arguments.push_back(value_argument);
            continue;
	          }
	          TemplateArgument evaluated_argument;
	          if(try_evaluate_alias_target_value_argument(argument,
	                                                      evaluated_argument)) {
	            substituted_arguments.push_back(evaluated_argument);
	            continue;
          }
          substituted_arguments.push_back(argument);
          continue;
        }
        if(argument.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
           argument.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
          const TemplateParameterInfo * argument_parameter =
              find_template_template_parameter(argument);
          if(argument_parameter) {
            const std::size_t template_index = static_cast<std::size_t>(
                argument_parameter - &alias_template.parameters[0]);
            if(template_index >= arguments.size() ||
               (arguments[template_index].kind !=
                    TemplateArgument::TA_CLASS_TEMPLATE &&
                arguments[template_index].kind !=
                    TemplateArgument::TA_ALIAS_TEMPLATE)) {
              return false;
            }
            substituted_arguments.push_back(arguments[template_index]);
            continue;
          }
          substituted_arguments.push_back(argument);
          continue;
        }
        if(argument.kind != TemplateArgument::TA_TYPE) {
          substituted_arguments.push_back(argument);
          continue;
        }
        TypePtr substituted_argument_type;
        if(!substitute_type(argument.type, substituted_argument_type) ||
           !substituted_argument_type) {
          return false;
        }
        argument.type = substituted_argument_type;
        argument.dependent = false;
        argument.text.clear();
        argument.text = argument_text(argument);
        substituted_arguments.push_back(argument);
      }

      if(template_arguments_are_dependent(substituted_arguments,
                                          [&type_is_dependent](const TypePtr & type)
                                          {
                                            return type_is_dependent(type);
                                          })) {
        std::vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
        dependent_arguments.reserve(substituted_arguments.size());
        for(std::size_t i = 0; i < substituted_arguments.size(); ++i) {
          const TemplateArgument & argument = substituted_arguments[i];
          DependentAliasTemplateArgumentSyntax dependent_argument;
          dependent_argument.text = argument_text(argument);
          if(argument.kind == TemplateArgument::TA_TYPE) {
            dependent_argument.type = argument.type;
          }
          if(argument.source_syntax) {
            dependent_argument.syntax = *argument.source_syntax;
          } else {
            dependent_argument.syntax.text = dependent_argument.text;
          }
          if(argument.kind == TemplateArgument::TA_VALUE &&
             argument.expression &&
             !dependent_argument.syntax.expression) {
            dependent_argument.syntax.text = dependent_argument.text;
            dependent_argument.syntax.source_location_id =
                argument.expression->source_location_id;
            dependent_argument.syntax.expression.reset(
                new CppAstNode(*argument.expression));
          }
          dependent_argument.syntax.dependent =
              dependent_argument.syntax.dependent || argument.dependent;
          dependent_argument.source_defaulted = argument.source_defaulted;
          dependent_arguments.push_back(dependent_argument);
        }

        const std::string template_name =
            named_info.source_template->declaring_scope ?
                semantic_lookup::scope_qualified_name(
                    *named_info.source_template->declaring_scope,
                    named_info.source_template->name) :
                named_info.source_template->name;
        std::ostringstream display;
        display << template_name << "<";
        for(std::size_t i = 0; i < dependent_arguments.size(); ++i) {
          if(i != 0) {
            display << ", ";
          }
          display << dependent_arguments[i].text;
        }
        display << ">";
        out = make_semantic_named(display.str(),
                                  Type::NSK_DEPENDENT_TYPE,
                                  display.str(),
                                  true);
        set_named_type_dependent_class_template(out,
                                                named_info.source_template,
                                                dependent_arguments);
        return true;
      }

      template_api::TemplateSelectedClassTemplateIdRequest request;
      TypePtr pattern_cv_base;
      bool pattern_const = false;
      bool pattern_volatile = false;
      top_level_cv_flags(pattern, pattern_cv_base, pattern_const, pattern_volatile);
      Scope & selected_scope =
          selected_arguments_need_alias_target_scope ?
              ensure_alias_target_scope() :
              match_scope.require();
      request.lookup.scope = &selected_scope;
      request.argument_scope = &selected_scope;
      request.lookup.name.name = named_info.source_template->name;
      request.lookup.allow_class_templates = true;
      request.lookup.top_const = pattern_const;
      request.lookup.top_volatile = pattern_volatile;
      request.class_template = named_info.source_template;
      request.resolved_arguments = substituted_arguments;
      request.source_arg_texts.reserve(substituted_arguments.size());
      for(std::size_t i = 0; i < substituted_arguments.size(); ++i) {
        request.source_arg_texts.push_back(argument_text(substituted_arguments[i]));
      }
      const bool witness_session_active =
          template_api::current_template_witness_session() != nullptr;
      if(!witness_session_active ||
         selected_class_arguments_have_defaulted_true_bool_value(
             substituted_arguments)) {
        if(type_system.resolve_selected_class_template_id(request, out) &&
           out) {
          return true;
        }
      } else {
        const template_api::ScopedTemplateWitnessLifecyclePause
            lifecycle_pause;
        if(type_system.resolve_selected_class_template_id(request, out) &&
           out) {
          return true;
        }
      }
    }

    switch(pattern->kind) {
    case Type::TK_FUNDAMENTAL:
    case Type::TK_NAMED:
      out = pattern;
      return true;

    case Type::TK_CV:
      return false;

    case Type::TK_ATOMIC:
    {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_atomic(inner);
      return true;
    }

    case Type::TK_POINTER:
    {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_pointer(inner);
      return true;
    }

    case Type::TK_MEMBER_POINTER:
    {
      TypePtr owner;
      TypePtr inner;
      if(!substitute_type(pattern->owner, owner) ||
         !substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_member_pointer(owner, inner);
      return true;
    }

    case Type::TK_BLOCK_POINTER:
    {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_block_pointer(inner);
      return true;
    }

    case Type::TK_LVALUE_REFERENCE:
    {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_lvalue_reference_raw(inner);
      return true;
    }

    case Type::TK_RVALUE_REFERENCE:
    {
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_rvalue_reference_raw(inner);
      return true;
    }

    case Type::TK_ARRAY:
    {
      if(!pattern->bound_text.empty()) {
        return false;
      }
      TypePtr inner;
      if(!substitute_type(pattern->inner, inner)) {
        return false;
      }
      out = make_array(inner, pattern->has_bound, pattern->bound, pattern->bound_text);
      return true;
    }

    case Type::TK_FUNCTION:
    {
      TypePtr result_type;
      if(!substitute_type(pattern->inner, result_type)) {
        return false;
      }
      std::vector<TypePtr> params;
      params.reserve(pattern->params.size());
      for(std::size_t i = 0; i < pattern->params.size(); ++i) {
        TypePtr param;
        if(!substitute_type(pattern->params[i], param)) {
          return false;
        }
        params.push_back(param);
      }
      out = make_function(result_type,
                          params,
                          pattern->variadic,
                          pattern->function_const,
                          pattern->function_volatile,
                          false,
                          pattern->function_ref_qualifier);
      return true;
    }
    }

    return false;
  };

  TypePtr substituted;
  if(!substitute_type(alias_template.resolved_type_pattern, substituted) ||
     !substituted) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-structural-substitute-fail alias="
            << alias_template.name
            << " substitution="
            << (structural_substitution_failure ? "yes" : "no");
      if(substitution_failure && substitution_failure->active()) {
        trace << " owner=" << substitution_failure->owner_type_display
              << " member=" << substitution_failure->member_name
              << " stable="
              << (substitution_failure->stable_for_reuse ? "yes" : "no");
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  if(type_is_dependent(substituted)) {
    if(!allow_dependent_expansion) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "expand-alias-structural-dependent alias="
              << alias_template.name
              << " substituted=" << describe_type(substituted);
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      cache_alias_dependent_defer();
      return false;
    }
    if(type_has_dependent_non_type_template_argument(type_system, substituted) &&
       !dependent_non_type_template_arguments_are_direct_parameters(
           type_system,
           substituted,
           alias_template.parameters)) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "expand-alias-structural-dependent-non-type alias="
              << alias_template.name
              << " substituted=" << describe_type(substituted);
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      cache_alias_dependent_defer();
      return false;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-structural-dependent-allowed alias="
            << alias_template.name
            << " substituted=" << describe_type(substituted);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
  }
  expanded_text = type_text(substituted);
  if(allow_dependent_expansion) {
    template_api::TemplateNamedTypeMetadata expanded_info;
    if(template_api::describe_named_type_metadata(type_system.model,
                                                  substituted,
                                                  expanded_info) &&
       expanded_info.source_template &&
       template_metadata::argument_texts_contain_pack_expansion(
           expanded_info.instantiation_arg_texts)) {
      const std::string template_name =
          expanded_info.source_template->declaring_scope ?
              semantic_lookup::scope_qualified_name(
                  *expanded_info.source_template->declaring_scope,
                  expanded_info.source_template->name) :
              expanded_info.source_template->name;
      expanded_text = template_name + "<" +
          join_display_template_arguments(expanded_info.instantiation_arg_texts) + ">";
    }
  }
  if(expanded_text.empty()) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-structural-empty alias="
            << alias_template.name
            << " substituted=" << describe_type(substituted);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }
  if(expanded_type) {
    *expanded_type = substituted;
  }
  cache_alias_success(expanded_text, substituted);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "expand-alias-structural-substituted alias="
          << alias_template.name
          << " type=" << describe_type(substituted)
          << " text=" << expanded_text;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return true;
}

bool expand_alias_template_pattern_id_impl(template_api::TemplateServices & services,
                                           template_api::TemplateEnvironmentHandle match_scope,
                                           const std::string & pattern_text,
                                           const QualifiedName & qualified,
                                           const std::vector<std::string> & arg_texts,
                                           const TemplateArgumentSyntax * pattern_syntax,
                                           template_api::TemplateEnvironmentHandle argument_scope,
                                           std::string & expanded_text,
                                           bool allow_dependent_expansion,
                                           AliasSubstitutionFailure * substitution_failure)
{
  if(substitution_failure) {
    substitution_failure->reset();
  }
  Scope & scope = match_scope.require();
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto type_is_dependent =
      [&type_system](const TypePtr & type) -> bool
      {
        return template_argument_semantics::type_depends_on_template_parameter(
            type_system,
            type);
      };
  const auto type_text =
      [&type_system](const TypePtr & type) -> std::string
      {
        return specialization_argument_type_text(type_system, type);
      };
  const auto bound_template_argument =
      [&scope, &qualified]() -> const TemplateArgument *
  {
    if(qualified.rooted || !qualified.qualifiers.empty() || qualified.name.empty()) {
      return nullptr;
    }
    for(const Scope * current = &scope; current; current = current->parent) {
      std::map<std::string, TemplateArgument>::const_iterator found =
          current->template_bound_template_arguments.find(qualified.name);
      if(found != current->template_bound_template_arguments.end()) {
        return &found->second;
      }
      if(current->namespace_scope || current->parent == nullptr) {
        break;
      }
    }
    return nullptr;
  }();
  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ? argument_scope : match_scope;
  Scope & arg_scope = effective_argument_scope.require();
  AliasTemplateDecl * alias_template = nullptr;
  std::string effective_pattern_text = pattern_text;
  if(bound_template_argument &&
     bound_template_argument->kind == TemplateArgument::TA_ALIAS_TEMPLATE &&
     bound_template_argument->template_decl) {
    alias_template =
        static_cast<AliasTemplateDecl *>(bound_template_argument->template_decl);
    if(!bound_template_argument->text.empty()) {
      effective_pattern_text = bound_template_argument->text;
    }
  }
  if(!alias_template) {
    alias_template =
        template_argument_semantics::lookup_alias_template(
            services, scope, template_api::qualified_name_text(qualified));
  }
  if(!alias_template || !alias_template->type_id) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-pattern miss pattern=" << effective_pattern_text;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  std::vector<std::string> canonical_arg_texts = arg_texts;
  std::vector<TemplateArgumentSyntax> canonical_arg_syntaxes;
  const std::vector<TemplateArgumentSyntax> * canonical_arg_syntaxes_ptr = nullptr;
  bool had_substitution_failure = false;
  bool has_dependent_arguments = false;
  const std::vector<TemplateArgumentSyntax> * pattern_arg_syntaxes =
      pattern_syntax && pattern_syntax->template_id ?
          &pattern_syntax->template_id->argument_syntaxes :
          nullptr;
  canonical_arg_syntaxes_ptr = pattern_arg_syntaxes;
  const auto ensure_canonical_arg_syntaxes =
      [&]() -> std::vector<TemplateArgumentSyntax> &
  {
    if(canonical_arg_syntaxes.empty()) {
      if(pattern_arg_syntaxes) {
        canonical_arg_syntaxes = *pattern_arg_syntaxes;
      }
      canonical_arg_syntaxes.resize(arg_texts.size());
      for(std::size_t j = 0; j < arg_texts.size(); ++j) {
        if(canonical_arg_syntaxes[j].text.empty()) {
          canonical_arg_syntaxes[j].text = semantic_utils::trim_space(arg_texts[j]);
        }
      }
    }
    canonical_arg_syntaxes_ptr = &canonical_arg_syntaxes;
    return canonical_arg_syntaxes;
  };
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "expand-alias-pattern begin pattern=" << effective_pattern_text
          << " alias=" << alias_template->name
          << " args=" << join_arg_texts(arg_texts)
          << " syntax=" << (pattern_arg_syntaxes ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  for(std::size_t i = 0; i < arg_texts.size(); ++i) {
    const std::string trimmed = semantic_utils::trim_space(arg_texts[i]);
    if(i < alias_template->parameters.size() &&
       alias_template->parameters[i].kind == TemplateParameterInfo::TP_TYPE) {
      std::string pack_element_text;
      if(alias_template->parameters[i].parameter_pack &&
         ((pattern_arg_syntaxes &&
           i < pattern_arg_syntaxes->size() &&
           (*pattern_arg_syntaxes)[i].pack_expansion) ||
          strip_trailing_pack_ellipsis(trimmed, pack_element_text))) {
        continue;
      }
      TypePtr resolved_type;
      try {
        const witness::ScopedTemplateWitnessSourceCapturePause
            source_capture_pause;
        const TemplateArgumentSyntax * arg_syntax =
            pattern_arg_syntaxes && i < pattern_arg_syntaxes->size() ?
                &(*pattern_arg_syntaxes)[i] :
                nullptr;
        if(!arg_syntax) {
          continue;
        }
        const bool resolved =
            parse_template_argument_type_syntax(
                services, arg_scope, arg_syntax, resolved_type, true);
        if(resolved &&
           resolved_type &&
           !type_is_dependent(resolved_type)) {
          const std::string canonical = type_text(resolved_type);
          const std::string canonical_text = canonical.empty() ? trimmed : canonical;
          canonical_arg_texts[i] = canonical_text;
          std::vector<TemplateArgumentSyntax> & canonical_syntaxes =
              ensure_canonical_arg_syntaxes();
          canonical_syntaxes[i].text = canonical_text;
          canonical_syntaxes[i].dependent = false;
          canonical_syntaxes[i].template_id.reset();
          canonical_syntaxes[i].type_id.reset();
          canonical_syntaxes[i].expression.reset();
          canonical_syntaxes[i].resolved_type = resolved_type;
          continue;
        }
      } catch(const TemplateSubstitutionFailure &) {
        had_substitution_failure = true;
        break;
      } catch(const SemanticSoftFailure &) {
        had_substitution_failure = true;
        break;
      } catch(const SemanticDiagnosticError &) {
        had_substitution_failure = true;
        break;
      }
    }
    if(template_argument_semantics::text_mentions_template_placeholders(
           services, effective_argument_scope, trimmed) ||
       template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
           services, effective_argument_scope, trimmed)) {
      has_dependent_arguments = true;
    }
  }
  if(had_substitution_failure) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-pattern defer pattern=" << effective_pattern_text
            << " reason=substitution-failure";
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  if(has_dependent_arguments &&
     !alias_template_target_mentions_parameters(*alias_template->type_id,
                                                alias_template->parameters)) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "expand-alias-pattern defer pattern=" << effective_pattern_text
            << " reason=non-propagating-dependent-alias";
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }

  const std::string alias_text = alias_template_target_text(*alias_template->type_id);
  {
    const witness::ScopedTemplateWitnessSourceCapturePause
        source_capture_pause;
    if(try_expand_alias_template_pattern_structurally(
           services,
           match_scope,
           *alias_template,
           canonical_arg_texts,
           canonical_arg_syntaxes_ptr,
           effective_argument_scope,
           expanded_text,
           nullptr,
           allow_dependent_expansion,
           substitution_failure)) {
    } else {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "expand-alias-pattern defer pattern=" << effective_pattern_text
              << " reason=structural-expansion-fail"
              << " substitution="
              << (substitution_failure && substitution_failure->active() ? "yes" : "no");
        if(substitution_failure && substitution_failure->active()) {
          trace << " owner=" << substitution_failure->owner_type_display
                << " member=" << substitution_failure->member_name
                << " stable="
                << (substitution_failure->stable_for_reuse ? "yes" : "no");
        }
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "expand-alias-pattern pattern=" << effective_pattern_text
          << " expanded=" << expanded_text;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(expanded_text != pattern_text) {
    record_alias_pattern_source_use(services,
                                    match_scope,
                                    *alias_template,
                                    qualified,
                                    arg_texts,
                                    pattern_syntax,
                                    expanded_text);
    return true;
  }
  return false;
}

Scope make_partial_match_scope(const std::vector<TemplateParameterInfo> & parameters,
                               Scope & pattern_scope,
                               const DeducedState & deduced)
{
  Scope eval_scope(&pattern_scope, "", false);
  for(const auto & entry : deduced.types) {
    template_scope::bind_named_type(eval_scope, entry.first, entry.second);
  }
  for(const auto & entry : deduced.type_packs) {
    template_scope::bind_type_pack(eval_scope, entry.first, entry.second);
  }
  for(const auto & entry : deduced.values) {
    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(parameters, entry.first);
    std::map<std::string, TemplateArgument>::const_iterator argument_found =
        deduced.value_arguments.find(entry.first);
    TypePtr value_type = argument_found != deduced.value_arguments.end() ?
        argument_found->second.type :
        (parameter ? parameter->value_type : make_fundamental(FT_INT));
    const std::string text = argument_found != deduced.value_arguments.end() ?
        argument_found->second.text :
        std::string();
    template_scope::bind_non_type_value(
        eval_scope, entry.first, value_type, entry.second, false, text);
  }
  for(const auto & entry : deduced.value_packs) {
    const TemplateParameterInfo * parameter =
        find_template_parameter_by_name(parameters, entry.first);
    TypePtr value_type = parameter ? parameter->value_type : make_fundamental(FT_INT);
    template_scope::bind_non_type_value_pack(
        eval_scope, entry.first, value_type, entry.second, false);
  }
  for(const auto & entry : deduced.class_templates) {
    template_scope::bind_class_template(eval_scope, entry.first, entry.second);
  }
  for(const auto & entry : deduced.alias_templates) {
    template_scope::bind_alias_template(eval_scope, entry.first, entry.second);
  }
  return eval_scope;
}

TypePtr resolved_non_type_parameter_value_type(
    template_api::TemplateServices & services,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & pattern_scope,
    const DeducedState & deduced,
    const TemplateParameterInfo & parameter)
{
  TypePtr value_type = parameter.value_type;
  if(!value_type) {
    return value_type;
  }

  Scope eval_scope = make_partial_match_scope(parameters, pattern_scope, deduced);
  template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
      services,
      template_api::make_template_environment(eval_scope),
      value_type);
  return value_type;
}

bool store_deduced_alias_template(DeducedState & deduced,
                                  const std::string & parameter_name,
                                  AliasTemplateDecl * alias_template)
{
  std::map<std::string, AliasTemplateDecl *>::iterator found =
      deduced.alias_templates.find(parameter_name);
  if(found == deduced.alias_templates.end()) {
    deduced.alias_templates[parameter_name] = alias_template;
  } else if(found->second != alias_template) {
    return false;
  }
  return deduced.class_templates.count(parameter_name) == 0;
}

bool store_deduced_class_template(DeducedState & deduced,
                                  const std::string & parameter_name,
                                  ClassTemplateDecl * class_template)
{
  std::map<std::string, ClassTemplateDecl *>::iterator found =
      deduced.class_templates.find(parameter_name);
  if(found == deduced.class_templates.end()) {
    deduced.class_templates[parameter_name] = class_template;
  } else if(found->second != class_template) {
    return false;
  }
  return deduced.alias_templates.count(parameter_name) == 0;
}

bool deduce_template_template_parameter_from_argument(DeducedState & deduced,
                                                      const std::string & parameter_name,
                                                      const TemplateArgument & actual)
{
  if(actual.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    return store_deduced_alias_template(
        deduced, parameter_name, static_cast<AliasTemplateDecl *>(actual.template_decl));
  }
  if(actual.kind == TemplateArgument::TA_CLASS_TEMPLATE) {
    return store_deduced_class_template(
        deduced, parameter_name, static_cast<ClassTemplateDecl *>(actual.template_decl));
  }
  return false;
}

cpp_decl::TypePtr make_partial_order_placeholder_type(const TemplateParameterInfo & parameter,
                                                      std::size_t index)
{
  const std::string name = parameter.name.empty() ? std::string("<unnamed>") : parameter.name;
  std::ostringstream key;
  key << "partial-order " << name;
  if(parameter.parameter_pack) {
    key << "...";
  }
  key << "#" << index;
  return make_named(std::string("typename ") + name, key.str(), false);
}

long long make_partial_order_placeholder_value(std::size_t index)
{
  return static_cast<long long>(0x100000 + index);
}

bool partial_non_type_pattern_text_can_be_value_name(const std::string & text)
{
  if(text.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(std::isalnum(ch) || ch == '_' || ch == ':') {
      continue;
    }
    return false;
  }
  return true;
}

template_api::NonTypeArgumentStatus evaluate_partial_non_type_pattern_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & pattern_text,
    const TemplateArgumentSyntax * pattern_syntax,
    const TypePtr & target_type,
    long long & value)
{
  if(pattern_syntax && pattern_syntax->expression && !pattern_syntax->pack_expansion) {
    return template_api::evaluate_non_type_argument_expression(
        services,
        scope,
        *pattern_syntax->expression,
        value,
        nullptr,
        target_type);
  }

  if(!partial_non_type_pattern_text_can_be_value_name(pattern_text)) {
    return template_api::NT_ARG_PARSE_FAILED;
  }

  CppAstNode expression;
  expression.kind = CppAstKind::id_expression;
  expression.value = pattern_text;
  if(pattern_syntax) {
    expression.source_location_id = pattern_syntax->source_location_id;
    if(pattern_syntax->has_source_token_start) {
      expression.token_start = pattern_syntax->source_token_start;
      expression.token_end = pattern_syntax->source_token_start + 1;
    }
  }
  return template_api::evaluate_non_type_argument_expression(
      services,
      scope,
      expression,
      value,
      nullptr,
      target_type);
}

template <typename PartialDecl>
bool transformed_partial_specialization_arguments(template_api::TemplateServices & services,
                                                  const PartialDecl & partial,
                                                  std::vector<TemplateArgument> & out)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  out.clear();
  if(!partial.pattern_scope) {
    return false;
  }

  DeducedState placeholders;
  for(std::size_t i = 0; i < partial.parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = partial.parameters[i];
    if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
      cpp_decl::TypePtr placeholder = make_partial_order_placeholder_type(parameter, i);
      if(parameter.parameter_pack) {
        placeholders.type_packs[parameter.name].push_back(placeholder);
      } else {
        placeholders.types[parameter.name] = placeholder;
      }
      continue;
    }
    if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
      const long long placeholder = make_partial_order_placeholder_value(i);
      if(parameter.parameter_pack) {
        placeholders.value_packs[parameter.name].push_back(placeholder);
      } else {
        placeholders.values[parameter.name] = placeholder;
      }
      continue;
    }
    return false;
  }

  Scope match_scope =
      make_partial_match_scope(partial.parameters, *partial.pattern_scope, placeholders);
  std::function<bool(const std::string &, const TemplateArgumentSyntax *, TypePtr &)>
      resolve_transformed_argument_type =
      [&](const std::string & raw_text,
          const TemplateArgumentSyntax * syntax,
          TypePtr & resolved_type) -> bool
  {
    const std::string pattern_text = trim_space(raw_text);
    const TemplateParameterInfo * direct_parameter =
        find_template_parameter_by_name(partial.parameters, pattern_text);
    if(direct_parameter && direct_parameter->kind == TemplateParameterInfo::TP_TYPE) {
      if(direct_parameter->parameter_pack) {
        std::map<std::string, std::vector<TypePtr> >::const_iterator found =
            placeholders.type_packs.find(direct_parameter->name);
        if(found == placeholders.type_packs.end() || found->second.empty()) {
          return false;
        }
        resolved_type = found->second[0];
      } else {
        std::map<std::string, TypePtr>::const_iterator found =
            placeholders.types.find(direct_parameter->name);
        if(found == placeholders.types.end()) {
          return false;
        }
        resolved_type = found->second;
      }
      return true;
    }

    std::string pack_element_text;
    if(strip_trailing_pack_ellipsis(pattern_text, pack_element_text)) {
      const TemplateParameterInfo * pack_parameter =
          find_template_parameter_by_name(partial.parameters, pack_element_text);
      if(pack_parameter &&
         pack_parameter->kind == TemplateParameterInfo::TP_TYPE &&
         pack_parameter->parameter_pack) {
        std::map<std::string, std::vector<TypePtr> >::const_iterator found =
            placeholders.type_packs.find(pack_parameter->name);
        if(found == placeholders.type_packs.end() || found->second.empty()) {
          return false;
        }
        resolved_type = found->second[0];
        return true;
      }
    }

    const bool pattern_mentions_parameters =
        alias_template_target_mentions_parameters(pattern_text, partial.parameters);
    if(syntax && !pattern_mentions_parameters) {
      TypePtr pattern_scope_type;
      if(parse_template_argument_type_syntax(
             services, *partial.pattern_scope, syntax, pattern_scope_type, true) &&
         pattern_scope_type &&
         !type_pattern_has_deducible_template_parameter(type_system, pattern_scope_type)) {
        resolved_type = pattern_scope_type;
        return true;
      }
      if(partial.declaring_scope && partial.declaring_scope != partial.pattern_scope) {
        TypePtr declaring_scope_type;
        if(parse_template_argument_type_syntax(
               services, *partial.declaring_scope, syntax, declaring_scope_type, true) &&
           declaring_scope_type &&
           !type_pattern_has_deducible_template_parameter(type_system,
                                                         declaring_scope_type)) {
          resolved_type = declaring_scope_type;
          return true;
        }
      }
    }

    if(parse_template_argument_type_syntax(
           services, match_scope, syntax, resolved_type, true)) {
      return true;
    }

    return false;
  };
  for(std::size_t i = 0; i < partial.arg_texts.size(); ++i) {
    const std::string pattern_text = trim_space(partial.arg_texts[i]);
    const DirectTemplateParameterPattern direct_pattern =
        find_direct_template_parameter_pattern(partial.parameters, pattern_text);
    const TemplateParameterInfo * direct_parameter = direct_pattern.parameter;
    const TemplateArgumentSyntax * pattern_syntax =
        i < partial.arg_syntaxes.size() ? &partial.arg_syntaxes[i] : nullptr;

    TemplateArgument argument;
    if(pattern_syntax) {
      argument.source_syntax.reset(new TemplateArgumentSyntax(*pattern_syntax));
    }
    if(direct_parameter && direct_parameter->kind == TemplateParameterInfo::TP_TYPE) {
      argument.kind = TemplateArgument::TA_TYPE;
      if(direct_parameter->parameter_pack) {
        std::map<std::string, std::vector<TypePtr> >::const_iterator found =
            placeholders.type_packs.find(direct_parameter->name);
        if(found == placeholders.type_packs.end() || found->second.empty()) {
          return false;
        }
        argument.type = found->second[0];
      } else {
        std::map<std::string, TypePtr>::const_iterator found =
            placeholders.types.find(direct_parameter->name);
        if(found == placeholders.types.end()) {
          return false;
        }
        argument.type = found->second;
      }
    } else if(direct_parameter &&
              direct_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
      argument.kind = TemplateArgument::TA_VALUE;
      argument.type = direct_parameter->value_type;
      template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
          services,
          template_api::make_template_environment(match_scope),
          argument.type);
      if(direct_parameter->parameter_pack) {
        std::map<std::string, std::vector<long long> >::const_iterator found =
            placeholders.value_packs.find(direct_parameter->name);
        if(found == placeholders.value_packs.end() || found->second.empty()) {
          return false;
        }
        argument.value = found->second[0];
      } else {
        std::map<std::string, long long>::const_iterator found =
            placeholders.values.find(direct_parameter->name);
        if(found == placeholders.values.end()) {
          return false;
        }
        argument.value = found->second;
      }
      argument.text = template_argument_text(
          argument,
          [&type_system](const TypePtr & type)
          {
            return specialization_argument_type_text(type_system, type);
          });
    } else {
      argument.kind = TemplateArgument::TA_TYPE;
      if(!resolve_transformed_argument_type(pattern_text, pattern_syntax, argument.type)) {
        long long value = 0;
        TypePtr value_type = make_fundamental(FT_INT);
        const template_api::NonTypeArgumentStatus value_status =
            evaluate_partial_non_type_pattern_value(
                services,
                template_api::make_template_environment(match_scope),
                pattern_text,
                pattern_syntax,
                value_type,
                value);
        if(value_status != template_api::NT_ARG_EVALUATED) {
          return false;
        }
        argument.kind = TemplateArgument::TA_VALUE;
        argument.type = value_type;
        argument.value = value;
        argument.text = pattern_text;
      }
    }

    if(argument.kind == TemplateArgument::TA_TYPE) {
      if(template_argument_semantics::type_depends_on_template_parameter(type_system, argument.type)) {
        argument.text.clear();
      } else {
        argument.text =
            specialization_argument_type_text(type_system, argument.type);
      }
    }
    out.push_back(argument);
  }

  return true;
}

template <typename PartialDecl, typename MatchFn>
int compare_partial_specialization_preference_impl(template_api::TemplateServices & services,
                                                   const PartialDecl & current,
                                                   const PartialDecl & best,
                                                   const MatchFn & match)
{
  std::vector<TemplateArgument> current_transformed;
  std::vector<TemplateArgument> best_transformed;
  const bool current_transformed_ok =
      transformed_partial_specialization_arguments(services, current, current_transformed);
  const bool best_transformed_ok =
      transformed_partial_specialization_arguments(services, best, best_transformed);
  if(!current_transformed_ok || !best_transformed_ok) {
    const int direct_constraint_specificity =
        compare_direct_template_parameter_constraint_specificity(current, best);
    if(direct_constraint_specificity != 0) {
      return direct_constraint_specificity;
    }
    const int direct_cv_template_id_specificity =
        compare_direct_cv_parameter_template_id_specificity(current, best);
    if(direct_cv_template_id_specificity != 0) {
      return direct_cv_template_id_specificity;
    }
    const int template_id_head_specificity =
        compare_template_id_head_specificity(current, best);
    if(template_id_head_specificity != 0) {
      return template_id_head_specificity;
    }
    const int template_id_pack_specificity =
        compare_template_id_trailing_pack_specificity(current, best);
    if(template_id_pack_specificity != 0) {
      return template_id_pack_specificity;
    }
    return compare_repeated_template_parameter_constraint_specificity(current, best);
  }

  std::vector<TemplateArgument> deduced_arguments;
  std::size_t specificity_score = 0;
  const bool current_more_specialized =
      match(best, current_transformed, deduced_arguments, specificity_score);
  deduced_arguments.clear();
  specificity_score = 0;
  const bool best_more_specialized =
      match(current, best_transformed, deduced_arguments, specificity_score);

  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "partial-specialization-order current="
          << join_arg_texts(current.arg_texts)
          << " best=" << join_arg_texts(best.arg_texts)
          << " current-more=" << (current_more_specialized ? "yes" : "no")
          << " best-more=" << (best_more_specialized ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }

  if(current_more_specialized && !best_more_specialized) {
    return -1;
  }
  if(best_more_specialized && !current_more_specialized) {
    return 1;
  }
  const int placeholder_specificity =
      compare_transformed_partial_argument_placeholder_specificity(current_transformed,
                                                                  best_transformed);
  if(placeholder_specificity != 0) {
    return placeholder_specificity;
  }
  const int direct_constraint_specificity =
      compare_direct_template_parameter_constraint_specificity(current, best);
  if(direct_constraint_specificity != 0) {
    return direct_constraint_specificity;
  }
  const int direct_cv_template_id_specificity =
      compare_direct_cv_parameter_template_id_specificity(current, best);
  if(direct_cv_template_id_specificity != 0) {
    return direct_cv_template_id_specificity;
  }
  const int template_id_head_specificity =
      compare_template_id_head_specificity(current, best);
  if(template_id_head_specificity != 0) {
    return template_id_head_specificity;
  }
  const int template_id_pack_specificity =
      compare_template_id_trailing_pack_specificity(current, best);
  if(template_id_pack_specificity != 0) {
    return template_id_pack_specificity;
  }
  const int repeated_parameter_specificity =
      compare_repeated_template_parameter_constraint_specificity(current, best);
  if(repeated_parameter_specificity != 0) {
    return repeated_parameter_specificity;
  }
  const int top_cv_specificity =
      compare_transformed_partial_argument_top_cv_specificity(current_transformed,
                                                             best_transformed);
  if(top_cv_specificity != 0) {
    return top_cv_specificity;
  }
  const int reference_cv_specificity =
      compare_transformed_partial_argument_reference_cv_specificity(
          current_transformed,
          best_transformed);
  if(reference_cv_specificity != 0) {
    return reference_cv_specificity;
  }
  const int function_pack_specificity =
      compare_function_type_pack_specificity(current,
                                             best,
                                             current_transformed,
                                             best_transformed);
  if(function_pack_specificity != 0) {
    return function_pack_specificity;
  }
  const int pack_specificity =
      compare_partial_specialization_pack_specificity(current, best);
  if(pack_specificity != 0) {
    return pack_specificity;
  }
  return 0;
}

template <typename PartialDecl>
bool deduce_from_named_template_id_text(template_api::TemplateServices & services,
                                        const PartialDecl & partial,
                                        DeducedState & deduced,
                                        Scope & match_scope,
                                        const std::string & pattern_text,
                                        const TemplateArgumentSyntax * pattern_syntax,
                                        const TypePtr & actual_type,
                                        bool structural_only)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto type_is_dependent =
      [&type_system](const TypePtr & type) -> bool
      {
        return template_argument_semantics::type_depends_on_template_parameter(
            type_system,
            type);
      };
  const auto type_text =
      [&type_system](const TypePtr & type) -> std::string
      {
        return specialization_argument_type_text(type_system, type);
      };
  const auto argument_text =
      [&type_system](const TemplateArgument & argument) -> std::string
      {
        return template_argument_text_for_matching(type_system, argument);
      };
  QualifiedName pattern_name;
  std::vector<std::string> pattern_args;
  QualifiedName actual_name;
  std::vector<std::string> actual_args;
  std::string actual_template_name_text;
  const std::string normalized_pattern = strip_elaborated_type_prefix(trim_space(pattern_text));
  template_api::TemplateNamedTypeMetadata actual_class;
  const bool have_actual_class =
      template_api::describe_named_type_metadata(type_system.model,
                                                 actual_type,
                                                 actual_class);
  struct DirectTemplateParameterMatch
  {
    const TemplateParameterInfo * parameter = nullptr;
    bool pack_expansion = false;
  };
  const auto find_direct_template_parameter_from_arg =
      [&partial](const std::string & raw_arg) -> DirectTemplateParameterMatch
  {
    std::string normalized = strip_elaborated_type_prefix(trim_space(raw_arg));
    static const char * prefixes[] = {
        "template-parameter ",
        "type-parameter ",
        "dependent type ",
        "dependent alias ",
        "dependent value "
    };
    for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
      const std::string prefix(prefixes[i]);
      if(normalized.compare(0, prefix.size(), prefix) == 0) {
        normalized.erase(0, prefix.size());
        break;
      }
    }

    DirectTemplateParameterMatch out;
    if(normalized.size() >= 3 &&
       normalized.compare(normalized.size() - 3, 3, "...") == 0) {
      out.pack_expansion = true;
      normalized.erase(normalized.size() - 3);
      normalized = trim_space(normalized);
    }

    out.parameter = find_template_parameter_by_name(partial.parameters, normalized);
    return out;
  };
  const auto decompose_actual_template_id =
      [&](QualifiedName & out_name,
          std::vector<std::string> & out_args,
          std::string & out_template_name_text) -> bool
  {
    out_name = QualifiedName();
    out_args.clear();
    out_template_name_text.clear();

    if(have_actual_class && actual_class.source_template) {
      const std::string source_template_name =
          actual_class.source_template->declaring_scope ?
              semantic_lookup::scope_qualified_name(*actual_class.source_template->declaring_scope,
                                                    actual_class.source_template->name) :
              actual_class.source_template->name;
      out_name = actual_class.source_template->declaring_scope ?
          semantic_lookup::scope_qualified_name_syntax(
              *actual_class.source_template->declaring_scope,
              actual_class.source_template->name) :
          QualifiedName();
      if(!actual_class.source_template->declaring_scope) {
        out_name.name = actual_class.source_template->name;
      }
      out_template_name_text = source_template_name;
      out_args.reserve(actual_class.instantiation_arguments.size());
      for(std::size_t i = 0; i < actual_class.instantiation_arguments.size(); ++i) {
        const std::string arg_text =
            argument_text(actual_class.instantiation_arguments[i]);
        if(arg_text.empty()) {
          return false;
        }
        out_args.push_back(arg_text);
      }
      return true;
    }

    return false;
  };
  if(!decompose_actual_template_id(actual_name, actual_args, actual_template_name_text)) {
    return false;
  }
  const auto pattern_template_id_matches =
      [&](const QualifiedName & candidate_name) -> bool
  {
    if(template_names_match(candidate_name, actual_name)) {
      return true;
    }
    if(have_actual_class &&
       actual_class.source_template &&
       services.semantic_context) {
      ClassTemplateDecl * pattern_template =
          semantic_lookup::lookup_class_template(*services.semantic_context,
                                                 match_scope,
                                                 candidate_name);
      if(pattern_template == actual_class.source_template) {
        return true;
      }
    }
    if(partial.pattern_scope &&
       !candidate_name.rooted &&
       !candidate_name.qualifiers.empty()) {
      const QualifiedName scoped_candidate =
          qualify_relative_template_name(*partial.pattern_scope, candidate_name);
      if(template_names_match(scoped_candidate, actual_name)) {
        return true;
      }
    }

    if(!candidate_name.rooted && candidate_name.qualifiers.empty()) {
      const DirectTemplateParameterMatch template_name_parameter =
          find_direct_template_parameter_from_arg(candidate_name.name);
      return template_name_parameter.parameter &&
             !template_name_parameter.pack_expansion &&
             template_name_parameter.parameter->kind ==
                 TemplateParameterInfo::TP_TEMPLATE_TEMPLATE;
    }

    return false;
  };
  bool pattern_decomposed = false;
  const auto decompose_template_id_pair =
      [&](const std::string & candidate_pattern) -> bool
  {
    QualifiedName candidate_name;
    std::vector<std::string> candidate_args;
    if(!semantic_utils::split_top_level_template_id_text(candidate_pattern,
                                                         candidate_name,
                                                         candidate_args)) {
      return false;
    }
    pattern_name = candidate_name;
    pattern_args = candidate_args;
    pattern_decomposed = true;
    return pattern_template_id_matches(pattern_name);
  };
  const TemplateIdSyntax * parsed_pattern_id =
      pattern_syntax ? template_argument_template_id_syntax(*pattern_syntax) : nullptr;
  bool parsed_pattern = false;
  if(parsed_pattern_id) {
    pattern_name = parsed_pattern_id->name;
    pattern_args = parsed_pattern_id->arguments;
    pattern_decomposed = true;
    parsed_pattern = pattern_template_id_matches(pattern_name);
  }
  const std::vector<TemplateArgumentSyntax> * pattern_arg_syntaxes =
      parsed_pattern_id ? &parsed_pattern_id->argument_syntaxes : nullptr;
  if(!parsed_pattern && !decompose_template_id_pair(normalized_pattern)) {
    if(structural_only) {
      return false;
    }
    std::string expanded_pattern_text;
    if(pattern_decomposed &&
       expand_alias_template_pattern_id_impl(
           services,
           template_api::make_template_environment(match_scope),
           normalized_pattern,
           pattern_name,
           pattern_args,
           pattern_syntax,
           template_api::TemplateEnvironmentHandle(),
           expanded_pattern_text,
           true)) {
      const std::string normalized_expanded_pattern =
          strip_elaborated_type_prefix(trim_space(expanded_pattern_text));
      if(decompose_template_id_pair(normalized_expanded_pattern)) {
        return deduce_from_named_template_id_text(
            services,
            partial,
            deduced,
            match_scope,
            normalized_expanded_pattern,
            nullptr,
            actual_type);
      }
    }
    TypePtr canonical_pattern_type;
    if(pattern_syntax) {
      parse_template_argument_type_syntax(
          services, match_scope, pattern_syntax, canonical_pattern_type, true);
    }
    if(!canonical_pattern_type) {
      if(pattern_syntax &&
         (pattern_syntax->type_id || pattern_syntax->template_id)) {
        return false;
      }
      return false;
    }
    if(type_is_dependent(canonical_pattern_type)) {
      return false;
    }
    const std::string canonical_pattern =
        strip_elaborated_type_prefix(
            trim_space(type_text(canonical_pattern_type)));
    if(canonical_pattern == normalized_pattern ||
       !decompose_template_id_pair(canonical_pattern)) {
      return false;
    }
  }

  const std::vector<TemplateArgument> * actual_structured_args =
      have_actual_class &&
      actual_class.source_template &&
      actual_class.source_template->name == actual_name.name &&
      actual_class.instantiation_arguments.size() == actual_args.size() ?
          &actual_class.instantiation_arguments :
          nullptr;
  const auto resolve_actual_arg_type =
      [&](std::size_t arg_index, const std::string & actual_arg, TypePtr & actual_arg_type) -> bool
  {
    if(actual_structured_args &&
       arg_index < actual_structured_args->size() &&
       (*actual_structured_args)[arg_index].kind == TemplateArgument::TA_TYPE &&
       (*actual_structured_args)[arg_index].type) {
      actual_arg_type = (*actual_structured_args)[arg_index].type;
      return true;
    }
    return false;
  };
  const auto resolve_pattern_arg_type =
      [&](std::size_t arg_index, const std::string & pattern_arg, TypePtr & pattern_arg_type) -> bool
  {
    if(pattern_arg_syntaxes &&
       arg_index < pattern_arg_syntaxes->size()) {
      const TemplateArgumentSyntax stable_syntax = (*pattern_arg_syntaxes)[arg_index];
      if(parse_template_argument_type_syntax(services,
                                             match_scope,
                                             &stable_syntax,
                                             pattern_arg_type,
                                             true) &&
         pattern_arg_type) {
        return true;
      }
    }
    return false;
  };
  const auto resolve_actual_non_type_value =
      [&](std::size_t arg_index,
          const std::string & actual_arg,
          const TemplateParameterInfo & parameter,
          long long & value) -> bool
  {
    if(actual_structured_args &&
       arg_index < actual_structured_args->size() &&
       (*actual_structured_args)[arg_index].kind == TemplateArgument::TA_VALUE &&
       !(*actual_structured_args)[arg_index].dependent) {
      value = (*actual_structured_args)[arg_index].value;
      return true;
    }
    if(parser_trace::enabled("template.resolve")) {
      parser_trace::note("template.resolve",
                         std::string(),
                         std::string("partial-match missing structured non-type actual: ") +
                             actual_arg);
    }
    return false;
  };

  const TemplateParameterInfo * trailing_pack_parameter = nullptr;
  const DirectTemplateParameterMatch template_name_parameter =
      find_direct_template_parameter_from_arg(pattern_name.name);
  if(template_name_parameter.parameter &&
     !template_name_parameter.pack_expansion &&
     template_name_parameter.parameter->kind ==
         TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
    TemplateArgument resolved;
    if(!template_api::resolve_template_template_argument_text(
           services,
           template_api::make_template_environment(match_scope),
           actual_template_name_text,
           static_cast<std::size_t>(-1),
           false,
           resolved) ||
       !deduce_template_template_parameter_from_argument(
           deduced, template_name_parameter.parameter->name, resolved)) {
      return false;
    }
  }
  std::size_t fixed_argument_count = pattern_args.size();
  if(!pattern_args.empty()) {
    const DirectTemplateParameterMatch trailing_match =
        find_direct_template_parameter_from_arg(pattern_args.back());
    if(trailing_match.parameter &&
       trailing_match.pack_expansion &&
       trailing_match.parameter->parameter_pack) {
      trailing_pack_parameter = trailing_match.parameter;
      fixed_argument_count = pattern_args.size() - 1;
    }
  }

  if(trailing_pack_parameter) {
    if(actual_args.size() < fixed_argument_count) {
      return false;
    }
  } else if(pattern_args.size() != actual_args.size()) {
    return false;
  }

  for(std::size_t arg_index = 0; arg_index < fixed_argument_count; ++arg_index) {
    const std::string pattern_arg = trim_space(pattern_args[arg_index]);
    const std::string actual_arg = trim_space(actual_args[arg_index]);
    const TemplateParameterInfo * direct_parameter =
        find_direct_template_parameter_from_arg(pattern_arg).parameter;
    if(!direct_parameter) {
      if(pattern_arg == actual_arg) {
        continue;
      }
      TypePtr pattern_arg_type;
      TypePtr actual_arg_type;
      if(resolve_pattern_arg_type(arg_index, pattern_arg, pattern_arg_type) &&
         resolve_actual_arg_type(arg_index, actual_arg, actual_arg_type) &&
         pattern_arg_type && actual_arg_type) {
        if(type_equals(pattern_arg_type, actual_arg_type)) {
          continue;
        }
        if(type_pattern_has_deducible_template_parameter(type_system, pattern_arg_type)) {
          DeducedState nested_type_deduced = deduced;
          if(deduce_type_pattern_to_state(partial.parameters,
                                          pattern_arg_type,
                                          actual_arg_type,
                                          nested_type_deduced)) {
            deduced = nested_type_deduced;
            continue;
          }
        }
      }
      bool pattern_arg_is_template_id =
          pattern_arg_syntaxes &&
          arg_index < pattern_arg_syntaxes->size() &&
          template_argument_template_id_syntax((*pattern_arg_syntaxes)[arg_index]);
      if(pattern_arg_is_template_id &&
         resolve_actual_arg_type(arg_index, actual_arg, actual_arg_type)) {
        DeducedState nested_deduced = deduced;
        Scope nested_match_scope =
            make_partial_match_scope(partial.parameters,
                                     *partial.pattern_scope,
                                     nested_deduced);
        const TemplateArgumentSyntax * nested_syntax =
            pattern_arg_syntaxes && arg_index < pattern_arg_syntaxes->size() ?
                &(*pattern_arg_syntaxes)[arg_index] :
                nullptr;
        if(deduce_from_named_template_id_text(services,
                                              partial,
                                              nested_deduced,
                                              nested_match_scope,
                                              pattern_arg,
                                              nested_syntax,
                                              actual_arg_type,
                                              true)) {
          deduced = nested_deduced;
          continue;
        }
      }
      if(pattern_arg != actual_arg) {
        return false;
      }
      continue;
    }

    if(direct_parameter->kind == TemplateParameterInfo::TP_TYPE) {
      TypePtr actual_arg_type;
      if(!resolve_actual_arg_type(arg_index, actual_arg, actual_arg_type)) {
        return false;
      }
      if(!store_deduced_type(deduced, direct_parameter->name, actual_arg_type)) {
        return false;
      }
      continue;
    }

    if(direct_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
      long long value = 0;
      if(!resolve_actual_non_type_value(arg_index, actual_arg, *direct_parameter, value)) {
        return false;
      }
      if(!store_deduced_value(deduced, direct_parameter->name, value)) {
        return false;
      }
      continue;
    }

    TemplateArgument resolved;
    if(template_api::resolve_template_template_argument_text(
           services,
           template_api::make_template_environment(match_scope),
           actual_arg,
           static_cast<std::size_t>(-1),
           false,
           resolved) &&
       deduce_template_template_parameter_from_argument(
           deduced, direct_parameter->name, resolved)) {
      continue;
    }

    return false;
  }

  if(trailing_pack_parameter) {
    if(trailing_pack_parameter->kind == TemplateParameterInfo::TP_TYPE) {
      std::vector<TypePtr> deduced_pack;
      for(std::size_t arg_index = fixed_argument_count;
          arg_index < actual_args.size();
          ++arg_index) {
        TypePtr actual_arg_type;
        if(!resolve_actual_arg_type(arg_index,
                                    trim_space(actual_args[arg_index]),
                                    actual_arg_type)) {
          return false;
        }
        deduced_pack.push_back(actual_arg_type);
      }
      if(!store_deduced_type_pack(deduced,
                                  trailing_pack_parameter->name,
                                  deduced_pack)) {
        return false;
      }
    } else if(trailing_pack_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
      std::vector<long long> deduced_pack;
      for(std::size_t arg_index = fixed_argument_count;
          arg_index < actual_args.size();
          ++arg_index) {
        long long value = 0;
        if(!resolve_actual_non_type_value(arg_index,
                                          trim_space(actual_args[arg_index]),
                                          *trailing_pack_parameter,
                                          value)) {
          return false;
        }
        deduced_pack.push_back(value);
      }
      if(!store_deduced_value_pack(deduced,
                                   trailing_pack_parameter->name,
                                   deduced_pack)) {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

template <typename PartialDecl>
bool match_partial_specialization_impl(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const PartialDecl & partial,
                                       const std::vector<TemplateArgument> & actual_arguments,
                                       std::vector<TemplateArgument> & deduced_arguments,
                                       std::size_t & specificity_score,
                                       std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  try {
    template_api::TemplateTypeSystem & type_system = service_type_system(services);
    const auto type_is_dependent =
        [&type_system](const TypePtr & type) -> bool
    {
      return template_argument_semantics::type_depends_on_template_parameter(
          type_system,
          type);
    };
    const auto type_text =
        [&type_system](const TypePtr & type) -> std::string
    {
      return specialization_argument_type_text(type_system, type);
    };
    const auto argument_text =
        [&type_text](const TemplateArgument & argument) -> std::string
    {
      return template_argument_text(argument, type_text);
    };
    const auto make_deduced_type_argument =
        [&](const TypePtr & type) -> TemplateArgument
    {
      TemplateArgument arg;
      arg.kind = TemplateArgument::TA_TYPE;
      arg.type = type;
      if(type_is_dependent(type)) {
        arg.text.clear();
      } else {
        arg.text = type_text(type);
      }
      return arg;
    };
    const auto make_deduced_value_argument =
        [&](const TypePtr & value_type, long long value) -> TemplateArgument
    {
      TemplateArgument arg;
      arg.kind = TemplateArgument::TA_VALUE;
      arg.type = value_type;
      arg.value = value;
      arg.text = argument_text(arg);
      return arg;
    };
    const auto make_deduced_value_argument_from =
        [&](const TypePtr & value_type, const TemplateArgument & source) -> TemplateArgument
    {
      TemplateArgument arg = source;
      arg.kind = TemplateArgument::TA_VALUE;
      arg.type = value_type;
      if(arg.text.empty()) {
        arg.text = argument_text(arg);
      }
      return arg;
    };
    deduced_arguments.clear();
    specificity_score = 0;
    if(deduced_pack_sizes) {
      deduced_pack_sizes->clear();
    }
    if(!partial.pattern_scope) {
      return false;
    }

    const TemplateParameterInfo * trailing_pack_parameter = nullptr;
    std::size_t fixed_argument_count = partial.arg_texts.size();
    if(!partial.arg_texts.empty()) {
      const std::string trailing_pattern = trim_space(partial.arg_texts.back());
      for(std::size_t i = 0; i < partial.parameters.size(); ++i) {
        if(!partial.parameters[i].parameter_pack || partial.parameters[i].name.empty()) {
          continue;
        }
        if(trailing_pattern == partial.parameters[i].name + "..." ||
           trailing_pattern == partial.parameters[i].name) {
          trailing_pack_parameter = &partial.parameters[i];
          fixed_argument_count = partial.arg_texts.size() - 1;
          break;
        }
      }
    }

    if(trailing_pack_parameter) {
      if(actual_arguments.size() < fixed_argument_count) {
        return false;
      }
    } else if(partial.arg_texts.size() != actual_arguments.size()) {
      return false;
    }

    DeducedState deduced;
    for(std::size_t i = 0; i < fixed_argument_count; ++i) {
      const std::string pattern_text = trim_space(partial.arg_texts[i]);
      const TemplateParameterInfo * direct_parameter =
          find_template_parameter_by_name(partial.parameters, pattern_text);
      if(!direct_parameter || direct_parameter->parameter_pack) {
        continue;
      }

      const TemplateArgument & actual = actual_arguments[i];
      if(direct_parameter->kind == TemplateParameterInfo::TP_TYPE) {
        if(actual.kind != TemplateArgument::TA_TYPE ||
           !actual.type ||
           !store_deduced_type(deduced, direct_parameter->name, actual.type)) {
          return false;
        }
      } else if(direct_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
        TypePtr expected_value_type =
            resolved_non_type_parameter_value_type(services,
                                                   partial.parameters,
                                                   *partial.pattern_scope,
                                                   deduced,
                                                   *direct_parameter);
        if(actual.kind != TemplateArgument::TA_VALUE ||
           actual.dependent ||
           !type_equals(actual.type, expected_value_type) ||
           !store_deduced_value_argument(
               deduced, direct_parameter->name, actual, expected_value_type)) {
          return false;
        }
      } else if(!deduce_template_template_parameter_from_argument(
                    deduced, direct_parameter->name, actual)) {
        return false;
      }
    }

    if(trailing_pack_parameter) {
      if(trailing_pack_parameter->kind == TemplateParameterInfo::TP_TYPE) {
        std::vector<TypePtr> & deduced_pack = deduced.type_packs[trailing_pack_parameter->name];
        for(std::size_t i = fixed_argument_count; i < actual_arguments.size(); ++i) {
          if(actual_arguments[i].kind != TemplateArgument::TA_TYPE) {
            return false;
          }
          deduced_pack.push_back(actual_arguments[i].type);
        }
      } else if(trailing_pack_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
        std::vector<long long> & deduced_pack =
            deduced.value_packs[trailing_pack_parameter->name];
        for(std::size_t i = fixed_argument_count; i < actual_arguments.size(); ++i) {
          if(actual_arguments[i].kind != TemplateArgument::TA_VALUE ||
             actual_arguments[i].dependent) {
            return false;
          }
          deduced_pack.push_back(actual_arguments[i].value);
        }
      } else {
        return false;
      }
    }

    for(std::size_t i = 0; i < fixed_argument_count; ++i) {
      const std::string pattern_text = trim_space(partial.arg_texts[i]);
      const TemplateArgument & actual = actual_arguments[i];
      const TemplateParameterInfo * direct_parameter =
          find_template_parameter_by_name(partial.parameters, pattern_text);
      const TemplateArgumentSyntax * pattern_syntax =
          i < partial.arg_syntaxes.size() ? &partial.arg_syntaxes[i] : nullptr;

      if(direct_parameter &&
         direct_parameter->kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
        if(deduce_template_template_parameter_from_argument(
               deduced, direct_parameter->name, actual)) {
          continue;
        }
        return false;
      }

      if(actual.kind == TemplateArgument::TA_TYPE) {
        if(!actual.type) {
          return false;
        }
        if(direct_parameter &&
           direct_parameter->kind == TemplateParameterInfo::TP_TYPE &&
           pattern_text == direct_parameter->name) {
          if(direct_parameter->parameter_pack ||
             !store_deduced_type(deduced, direct_parameter->name, actual.type)) {
            return false;
          }
          continue;
        }
        if(is_bare_template_parameter_type(actual.type)) {
          if(parser_trace::enabled("template.resolve")) {
            parser_trace::note(
                "template.resolve",
                std::string(),
                std::string("partial-match bare-template-parameter-shape-mismatch pattern=") +
                    pattern_text);
          }
          return false;
        }
        Scope match_scope =
            make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced);
        bool matched_by_type = false;
        DeducedState placeholder_deduced;
        Scope placeholder_match_scope =
            make_partial_match_scope(partial.parameters,
                                     *partial.pattern_scope,
                                     placeholder_deduced);
        const bool pattern_mentions_placeholders =
            template_argument_semantics::text_mentions_template_placeholders(
                services,
                template_api::make_template_environment(placeholder_match_scope),
                pattern_text);
        TypePtr placeholder_pattern_type;
        if(pattern_mentions_placeholders) {
          const TemplateArgumentSyntax * placeholder_syntax =
              i < partial.arg_syntaxes.size() ? &partial.arg_syntaxes[i] : nullptr;
          if(placeholder_syntax) {
            parse_template_argument_type_syntax(
                services,
                placeholder_match_scope,
                placeholder_syntax,
                placeholder_pattern_type,
                true);
          }
        }
        const bool pattern_has_deducible_placeholders =
            pattern_mentions_placeholders &&
            type_pattern_has_deducible_template_parameter(type_system,
                                                          placeholder_pattern_type);
        TypePtr pattern_type;
        bool parsed_pattern_type = false;
        if(pattern_syntax &&
           parse_template_argument_type_syntax(
               services, match_scope, pattern_syntax, pattern_type, true)) {
          parsed_pattern_type = true;
          if(pattern_type &&
             (!pattern_mentions_placeholders || !pattern_has_deducible_placeholders) &&
             partial_specialization_top_cv_matches(pattern_type, actual.type) &&
             type_equals(pattern_type, actual.type)) {
            matched_by_type = true;
          }
        }
        if(!parsed_pattern_type &&
           pattern_syntax &&
           (pattern_syntax->type_id || pattern_syntax->template_id) &&
           !pattern_mentions_placeholders) {
          return false;
        }
        if(!parsed_pattern_type) {
          return false;
        }
        const bool pattern_is_dependent =
            pattern_type &&
            type_is_dependent(pattern_type);
        {
          DeducedState deduced_copy = deduced;
          Scope match_scope_copy =
              make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced_copy);
          if(deduce_from_named_template_id_text(
                 services,
                 partial,
                 deduced_copy,
                 match_scope_copy,
                 pattern_text,
                 pattern_syntax,
                 actual.type)) {
            deduced = deduced_copy;
            matched_by_type = true;
          }
        }

        if(!matched_by_type &&
           (pattern_mentions_placeholders || pattern_is_dependent)) {
          DeducedState deduced_copy = deduced;
          Scope match_scope_copy =
              make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced_copy);
          TypePtr deduction_pattern_type = pattern_type;
          if(pattern_mentions_placeholders && placeholder_pattern_type) {
            deduction_pattern_type = placeholder_pattern_type;
          }
          Scope & deduction_scope =
              pattern_mentions_placeholders ? placeholder_match_scope : match_scope_copy;
          if(deduction_pattern_type &&
             partial_specialization_top_cv_matches(deduction_pattern_type, actual.type) &&
             (template_api::deduce_template_argument(
                  services,
                  partial.parameters,
                  deduction_pattern_type,
                  actual.type,
                  deduced_copy.types,
                  template_api::make_template_environment(deduction_scope),
                  true,
                  template_api::make_template_environment(scope),
                  false) ||
              deduce_type_pattern_to_state(partial.parameters,
                                           deduction_pattern_type,
                                           actual.type,
                                           deduced_copy))) {
            deduced = deduced_copy;
            matched_by_type = true;
          }
        }

        if(!matched_by_type &&
           parsed_pattern_type_allows_direct_function_fallback(pattern_type)) {
          DeducedState deduced_copy = deduced;
          if(deduce_from_function_type_pattern(
                 services, partial, deduced_copy, pattern_type, pattern_syntax, actual.type)) {
            deduced = deduced_copy;
            matched_by_type = true;
          }
        }

        if(!matched_by_type) {
          return false;
        }
        if(!deduce_array_bound_texts_from_actual_type(
               partial.parameters, pattern_text, pattern_type, actual.type, deduced)) {
          return false;
        }
        if(!direct_parameter || direct_parameter->kind != TemplateParameterInfo::TP_TYPE) {
          ++specificity_score;
        }
        continue;
      }

      if(actual.kind == TemplateArgument::TA_VALUE) {
        if(actual.dependent) {
          return false;
        }
        if(direct_parameter && direct_parameter->kind == TemplateParameterInfo::TP_NON_TYPE) {
          TypePtr expected_value_type =
              resolved_non_type_parameter_value_type(services,
                                                     partial.parameters,
                                                     *partial.pattern_scope,
                                                     deduced,
                                                     *direct_parameter);
          if(!type_equals(actual.type, expected_value_type)) {
            return false;
          }
          if(!store_deduced_value_argument(
                 deduced, direct_parameter->name, actual, expected_value_type)) {
            return false;
          }
        } else {
          Scope eval_scope =
              make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced);
          long long expected = 0;
          const template_api::NonTypeArgumentStatus expected_status =
              evaluate_partial_non_type_pattern_value(
                  services,
                  template_api::make_template_environment(eval_scope),
                  pattern_text,
                  pattern_syntax,
                  actual.type,
                  expected);
          if(expected_status != template_api::NT_ARG_EVALUATED) {
            if(parser_trace::enabled("template.resolve")) {
              parser_trace::note(
                  "template.resolve",
                  std::string(),
                  std::string("partial-match missing structured non-type pattern: ") +
                      pattern_text);
            }
            return false;
          }
          if(expected != actual.value) {
            return false;
          }
          ++specificity_score;
        }
        continue;
      }

      if(actual.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
         actual.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
        if(!direct_parameter ||
           direct_parameter->kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
          return false;
        }
        if(!deduce_template_template_parameter_from_argument(
               deduced, direct_parameter->name, actual)) {
          return false;
        }
        continue;
      }

      return false;
    }

    for(std::size_t i = 0; i < partial.parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = partial.parameters[i];
      if(parameter.parameter_pack) {
        std::size_t pack_size = 0;
        if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
          promote_single_deduced_type_to_pack(deduced, parameter.name);
          std::map<std::string, std::vector<TypePtr> >::iterator pack_found =
              deduced.type_packs.find(parameter.name);
          if(pack_found != deduced.type_packs.end()) {
            pack_size = pack_found->second.size();
            for(std::size_t j = 0; j < pack_found->second.size(); ++j) {
              deduced_arguments.push_back(
                  make_deduced_type_argument(pack_found->second[j]));
            }
          }
          if(deduced_pack_sizes && !parameter.name.empty()) {
            (*deduced_pack_sizes)[parameter.name] = pack_size;
          }
          continue;
        }
        if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
          promote_single_deduced_value_to_pack(deduced, parameter.name);
          std::map<std::string, std::vector<long long> >::iterator pack_found =
              deduced.value_packs.find(parameter.name);
          if(pack_found != deduced.value_packs.end()) {
            TypePtr value_type =
                resolved_non_type_parameter_value_type(services,
                                                       partial.parameters,
                                                       *partial.pattern_scope,
                                                       deduced,
                                                       parameter);
            pack_size = pack_found->second.size();
            for(std::size_t j = 0; j < pack_found->second.size(); ++j) {
              deduced_arguments.push_back(
                  make_deduced_value_argument(value_type, pack_found->second[j]));
            }
          }
          if(deduced_pack_sizes && !parameter.name.empty()) {
            (*deduced_pack_sizes)[parameter.name] = pack_size;
          }
          continue;
        }
        return false;
      }

      TemplateArgument arg;
      if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
        std::map<std::string, TypePtr>::iterator found =
            deduced.types.find(parameter.name);
        if(found == deduced.types.end()) {
          return false;
        }
        arg = make_deduced_type_argument(found->second);
      } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
        std::map<std::string, long long>::iterator found =
            deduced.values.find(parameter.name);
        if(found == deduced.values.end()) {
          return false;
        }
        TypePtr value_type =
            resolved_non_type_parameter_value_type(services,
                                                   partial.parameters,
                                                   *partial.pattern_scope,
                                                   deduced,
                                                   parameter);
        std::map<std::string, TemplateArgument>::const_iterator argument_found =
            deduced.value_arguments.find(parameter.name);
        arg = argument_found != deduced.value_arguments.end() ?
            make_deduced_value_argument_from(value_type, argument_found->second) :
            make_deduced_value_argument(value_type, found->second);
      } else {
        std::map<std::string, ClassTemplateDecl *>::iterator class_found =
            deduced.class_templates.find(parameter.name);
        std::map<std::string, AliasTemplateDecl *>::iterator alias_found =
            deduced.alias_templates.find(parameter.name);
        if(class_found == deduced.class_templates.end() &&
           alias_found == deduced.alias_templates.end()) {
          return false;
        }
        arg = make_deduced_template_template_argument(
            parameter,
            class_found != deduced.class_templates.end() ? class_found->second : nullptr,
            alias_found != deduced.alias_templates.end() ? alias_found->second : nullptr);
      }
      deduced_arguments.push_back(arg);
    }

    if(witness::source_capture_enabled(services.witness_context)) {
      Scope matched_pattern_scope =
          make_partial_match_scope(partial.parameters, *partial.pattern_scope, deduced);
      for(std::size_t i = 0; i < fixed_argument_count; ++i) {
        if(i >= partial.arg_syntaxes.size()) {
          continue;
        }
        const std::string pattern_text = trim_space(partial.arg_texts[i]);
        QualifiedName pattern_name;
        std::vector<std::string> pattern_args;
        if(!semantic_utils::split_top_level_template_id_text(pattern_text,
                                                             pattern_name,
                                                             pattern_args)) {
          continue;
        }
        record_alias_pattern_source_use_from_texts(
            services,
            template_api::make_template_environment(matched_pattern_scope),
            pattern_name,
            pattern_args,
            &partial.arg_syntaxes[i]);
      }
    }

    return true;
  } catch(const TemplateSubstitutionFailure &) {
    return false;
  }
}

}  // namespace

bool expand_alias_template_pattern_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const std::string & pattern_text,
    const QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    std::string & expanded_text,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    AliasSubstitutionFailure * substitution_failure)
{
  TemplateArgumentSyntax pattern_syntax;
  const TemplateArgumentSyntax * pattern_syntax_ptr = nullptr;
  if(arg_syntaxes) {
    pattern_syntax.text = pattern_text;
    pattern_syntax.template_id.reset(new TemplateIdSyntax());
    pattern_syntax.template_id->name = qualified;
    pattern_syntax.template_id->arguments = arg_texts;
    pattern_syntax.template_id->argument_syntaxes = *arg_syntaxes;
    pattern_syntax_ptr = &pattern_syntax;
  }
  return expand_alias_template_pattern_id_impl(
      services,
      match_scope,
      pattern_text,
      qualified,
      arg_texts,
      pattern_syntax_ptr,
      argument_scope,
      expanded_text,
      false,
      substitution_failure);
}

bool expand_alias_template_pattern_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle match_scope,
    const QualifiedName & qualified,
    const std::vector<std::string> & arg_texts,
    TypePtr & expanded_type,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    bool allow_dependent_expansion,
    AliasSubstitutionFailure * substitution_failure)
{
  if(substitution_failure) {
    substitution_failure->reset();
  }
  expanded_type.reset();
  Scope & scope = match_scope.require();
  AliasTemplateDecl * alias_template =
      template_argument_semantics::lookup_alias_template(
          services, scope, template_api::qualified_name_text(qualified));
  if(!alias_template || !alias_template->type_id) {
    return false;
  }

  std::string expanded_text;
  const witness::ScopedTemplateWitnessSourceCapturePause
      source_capture_pause(
          template_api::current_template_witness_session() != nullptr);
  return try_expand_alias_template_pattern_structurally(
      services,
      match_scope,
      *alias_template,
      arg_texts,
      arg_syntaxes,
      argument_scope,
      expanded_text,
      &expanded_type,
      allow_dependent_expansion,
      substitution_failure);
}

bool match_partial_class_specialization(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const PartialClassTemplateSpecializationDecl & partial,
    const std::vector<TemplateArgument> & actual_arguments,
    std::vector<TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return match_partial_specialization_impl(services,
                                           scope.require(),
                                           partial,
                                           actual_arguments,
                                           deduced_arguments,
                                           specificity_score,
                                           deduced_pack_sizes);
}

bool match_partial_variable_specialization(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const VariableTemplateSpecializationDecl & partial,
    const std::vector<TemplateArgument> & actual_arguments,
    std::vector<TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return match_partial_specialization_impl(services,
                                           scope.require(),
                                           partial,
                                           actual_arguments,
                                           deduced_arguments,
                                           specificity_score,
                                           deduced_pack_sizes);
}

int compare_partial_class_specialization_preference(
    template_api::TemplateServices & services,
    const PartialClassTemplateSpecializationDecl & current,
    const PartialClassTemplateSpecializationDecl & best)
{
  return compare_partial_specialization_preference_impl(
      services,
      current,
      best,
      [&](const PartialClassTemplateSpecializationDecl & partial,
          const std::vector<TemplateArgument> & actual_arguments,
          std::vector<TemplateArgument> & deduced_arguments,
          std::size_t & specificity_score) -> bool
      {
        Scope & match_scope = partial.pattern_scope ? *partial.pattern_scope :
                                                    *current.pattern_scope;
        return match_partial_specialization_impl(
            services,
            match_scope,
            partial,
            actual_arguments,
            deduced_arguments,
            specificity_score,
            nullptr);
      });
}

int compare_partial_variable_specialization_preference(
    template_api::TemplateServices & services,
    const VariableTemplateSpecializationDecl & current,
    const VariableTemplateSpecializationDecl & best)
{
  return compare_partial_specialization_preference_impl(
      services,
      current,
      best,
      [&](const VariableTemplateSpecializationDecl & partial,
          const std::vector<TemplateArgument> & actual_arguments,
          std::vector<TemplateArgument> & deduced_arguments,
          std::size_t & specificity_score) -> bool
      {
        Scope & match_scope = partial.pattern_scope ? *partial.pattern_scope :
                                                    *current.pattern_scope;
        return match_partial_specialization_impl(
            services,
            match_scope,
            partial,
            actual_arguments,
            deduced_arguments,
            specificity_score,
            nullptr);
      });
}

}  // namespace template_specialization
