#include "template_model.h"

#include <sstream>

using namespace std;

namespace template_model {

namespace {

bool is_named_enum_argument_type(const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  return base &&
         base->kind == cpp_decl::Type::TK_NAMED &&
         (base->named_key.compare(0, 5, "enum ") == 0 ||
          named_type_display_text(base).compare(0, 5, "enum ") == 0);
}

}  // namespace

bool template_arguments_are_dependent(
    const vector<TemplateArgument> & arguments,
    const function<bool(const cpp_decl::TypePtr &)> & type_is_dependent)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind == TemplateArgument::TA_TYPE &&
       type_is_dependent &&
       type_is_dependent(arguments[i].type)) {
      return true;
    }
    if(arguments[i].kind == TemplateArgument::TA_VALUE &&
       arguments[i].dependent) {
      return true;
    }
    if((arguments[i].kind == TemplateArgument::TA_CLASS_TEMPLATE ||
        arguments[i].kind == TemplateArgument::TA_ALIAS_TEMPLATE) &&
       (arguments[i].dependent ||
        arguments[i].template_decl == nullptr ||
        (arguments[i].template_owner_type &&
         type_is_dependent &&
         type_is_dependent(arguments[i].template_owner_type)))) {
      return true;
    }
  }
  return false;
}

bool template_arguments_fully_bind_parameters(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments)
{
  size_t arg_index = 0;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      size_t trailing_non_pack = 0;
      for(size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      if(arguments.size() < arg_index + trailing_non_pack) {
        return false;
      }
      arg_index = arguments.size() - trailing_non_pack;
      continue;
    }
    if(arg_index >= arguments.size()) {
      return false;
    }
    ++arg_index;
  }
  return arg_index == arguments.size();
}

std::string template_argument_text(
    const TemplateArgument & arg,
    const function<string(const cpp_decl::TypePtr &)> & type_text)
{
  if(arg.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
     arg.kind == TemplateArgument::TA_ALIAS_TEMPLATE) {
    return arg.text;
  }
  if(arg.kind == TemplateArgument::TA_VALUE) {
    if(arg.dependent) {
      return arg.text;
    }
    if(!arg.text.empty() &&
       (!arg.type ||
        (!cpp_decl::is_integral_type(arg.type) &&
         !cpp_decl::is_bool_type(arg.type) &&
         !is_named_enum_argument_type(arg.type)))) {
      return arg.text;
    }
    if(cpp_decl::is_bool_type(arg.type)) {
      return arg.value != 0 ? "true" : "false";
    }
    if(type_text && is_named_enum_argument_type(arg.type)) {
      return string("(") + type_text(arg.type) + ")" + std::to_string(arg.value);
    }
    std::ostringstream out;
    out << arg.value;
    return out.str();
  }
  if(arg.text.empty() && type_text) {
    return type_text(arg.type);
  }
  return arg.text;
}

semantic_model::AliasTemplateDecl * template_argument_alias_template(
    const TemplateArgument & arg)
{
  return arg.kind == TemplateArgument::TA_ALIAS_TEMPLATE ?
             static_cast<semantic_model::AliasTemplateDecl *>(arg.template_decl) :
             nullptr;
}

semantic_model::ClassTemplateDecl * template_argument_class_template(
    const TemplateArgument & arg)
{
  return arg.kind == TemplateArgument::TA_CLASS_TEMPLATE ?
             static_cast<semantic_model::ClassTemplateDecl *>(arg.template_decl) :
             nullptr;
}

std::string template_argument_key(
    const vector<TemplateArgument> & args,
    const function<string(const cpp_decl::TypePtr &)> & type_text)
{
  string out;
  for(size_t i = 0; i < args.size(); ++i) {
    if(i != 0) {
      out += "|";
    }
    if(args[i].kind == TemplateArgument::TA_TYPE && type_text) {
      out += type_text(args[i].type);
      continue;
    }
    out += template_argument_text(args[i], type_text);
  }
  return out;
}

const TemplateParameterInfo * find_template_parameter_by_name(
    const vector<TemplateParameterInfo> & parameters,
    const string & name)
{
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].name == name) {
      return &parameters[i];
    }
    for(size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      if(parameters[i].alternate_names[j] == name) {
        return &parameters[i];
      }
    }
  }
  return nullptr;
}

const TemplateParameterInfo * find_template_parameter(
    const vector<TemplateParameterInfo> & parameters,
    const string & placeholder_key)
{
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].placeholder_key == placeholder_key ||
       (!parameters[i].name.empty() &&
        (parameters[i].name == placeholder_key ||
         string("typename ") + parameters[i].name == placeholder_key))) {
      return &parameters[i];
    }
    for(size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      if(parameters[i].alternate_names[j] == placeholder_key ||
         string("typename ") + parameters[i].alternate_names[j] == placeholder_key) {
        return &parameters[i];
      }
    }
  }
  return nullptr;
}

const TemplateParameterInfo * find_template_parameter(
    const vector<TemplateParameterInfo> & parameters,
    const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base || base->kind != cpp_decl::Type::TK_NAMED) {
    return nullptr;
  }

  if(base->named_key_identity && base->named_key_prefix_kind == 0) {
    for(size_t i = 0; i < parameters.size(); ++i) {
      if(parameters[i].placeholder_key_identity == base->named_key_identity) {
        return &parameters[i];
      }
    }
  }

  const string * candidates[] = {
      &base->named_key,
      &base->named_semantic_payload,
      &base->named_source_name(),
  };
  for(size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
    if(candidates[i]->empty()) {
      continue;
    }
    if(const TemplateParameterInfo * parameter =
           find_template_parameter(parameters, *candidates[i])) {
      return parameter;
    }
    if(const TemplateParameterInfo * parameter =
           find_template_parameter_by_name(parameters, *candidates[i])) {
      return parameter;
    }
  }
  return nullptr;
}

}  // namespace template_model
