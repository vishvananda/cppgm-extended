#include "template_binding.h"

#include <stdexcept>

using namespace std;

namespace template_binding {

namespace {

size_t remaining_non_pack_parameter_count(
    const vector<template_model::TemplateParameterInfo> & parameters,
    size_t start_index)
{
  size_t count = 0;
  for(size_t i = start_index; i < parameters.size(); ++i) {
    if(!parameters[i].parameter_pack) {
      ++count;
    }
  }
  return count;
}

}  // namespace

void bind_arguments(const vector<template_model::TemplateParameterInfo> & parameters,
                    const vector<template_model::TemplateArgument> & arguments,
                    const Hooks & hooks,
                    const std::map<std::string, std::size_t> * pack_sizes)
{
  const auto bind_type_name = [&hooks](const std::string & name,
                                       const cpp_decl::TypePtr & type)
  {
    if(!name.empty() && hooks.bind_named_type) {
      hooks.bind_named_type(name, type);
    }
  };
  const auto bind_template_name = [&hooks](const std::string & name,
                                           const template_model::TemplateArgument & argument)
  {
    if(!name.empty() && hooks.bind_template_template) {
      hooks.bind_template_template(name, argument);
    }
  };
  const auto bind_non_type_name =
      [&hooks](const std::string & name,
               const cpp_decl::TypePtr & value_type,
               const template_model::TemplateArgument & argument)
  {
    if(!name.empty() && hooks.bind_non_type) {
      hooks.bind_non_type(name, value_type, argument);
    }
  };
  const auto bind_pack_size_name = [&hooks](const std::string & name, std::size_t pack_count)
  {
    if(!name.empty() && hooks.bind_pack_size) {
      hooks.bind_pack_size(name, pack_count);
    }
  };
  const auto bind_type_pack_name =
      [&hooks](const std::string & name, const std::vector<cpp_decl::TypePtr> & bound_pack)
  {
    if(!name.empty() && hooks.bind_type_pack) {
      hooks.bind_type_pack(name, bound_pack);
    }
  };
  const auto bind_non_type_pack_name =
      [&hooks](const std::string & name,
               const cpp_decl::TypePtr & value_type,
               const std::vector<template_model::TemplateArgument> & bound_pack)
  {
    if(!name.empty() && hooks.bind_non_type_pack) {
      hooks.bind_non_type_pack(name, value_type, bound_pack);
    }
  };
  const auto for_each_parameter_name =
      [](const template_model::TemplateParameterInfo & parameter,
         const std::function<void(const std::string &)> & fn)
  {
    if(!parameter.name.empty()) {
      fn(parameter.name);
    }
    for(size_t i = 0; i < parameter.alternate_names.size(); ++i) {
      if(parameter.alternate_names[i].empty() ||
         parameter.alternate_names[i] == parameter.name) {
        continue;
      }
      fn(parameter.alternate_names[i]);
    }
  };
  size_t arg_index = 0;
  for(size_t i = 0; i < parameters.size(); ++i) {
    const template_model::TemplateParameterInfo & parameter = parameters[i];
    if(parameter.parameter_pack) {
      const size_t trailing_non_pack =
          remaining_non_pack_parameter_count(parameters, i + 1);
      size_t pack_count = 0;
      bool have_explicit_pack_count = false;
      if(pack_sizes && !parameter.name.empty()) {
        std::map<std::string, std::size_t>::const_iterator found =
            pack_sizes->find(parameter.name);
        if(found != pack_sizes->end()) {
          pack_count = found->second;
          have_explicit_pack_count = true;
        }
      }
      if(!have_explicit_pack_count) {
        if(arguments.size() < arg_index + trailing_non_pack) {
          throw logic_error("insufficient template arguments after parameter pack");
        }
        pack_count = arguments.size() - arg_index - trailing_non_pack;
      } else if(arguments.size() < arg_index + pack_count + trailing_non_pack) {
        throw logic_error("explicit template pack binding overruns remaining arguments");
      }
      for_each_parameter_name(
          parameter,
          [&](const std::string & name) { bind_pack_size_name(name, pack_count); });
      if(parameter.kind == template_model::TemplateParameterInfo::TP_TYPE) {
        vector<cpp_decl::TypePtr> bound_pack;
        for(size_t end = arg_index + pack_count; arg_index < end; ++arg_index) {
          bound_pack.push_back(arguments[arg_index].type);
        }
        for_each_parameter_name(
            parameter,
            [&](const std::string & name) { bind_type_pack_name(name, bound_pack); });
      } else if(parameter.kind == template_model::TemplateParameterInfo::TP_NON_TYPE) {
        cpp_decl::TypePtr value_type = parameter.value_type;
        if(hooks.resolve_non_type_value_type && pack_count != 0) {
          cpp_decl::TypePtr resolved =
              hooks.resolve_non_type_value_type(parameter, arguments[arg_index]);
          if(resolved) {
            value_type = resolved;
          }
        }
        std::vector<template_model::TemplateArgument> bound_pack;
        for(size_t end = arg_index + pack_count; arg_index < end; ++arg_index) {
          bound_pack.push_back(arguments[arg_index]);
        }
        for_each_parameter_name(
            parameter,
            [&](const std::string & name)
            {
              bind_non_type_pack_name(name, value_type, bound_pack);
            });
      } else {
        arg_index += pack_count;
      }
      continue;
    }

    if(arg_index >= arguments.size()) {
      break;
    }

    const template_model::TemplateArgument & argument = arguments[arg_index];
    if(parameter.kind == template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      for_each_parameter_name(
          parameter,
          [&](const std::string & name) { bind_template_name(name, argument); });
    } else if(parameter.kind == template_model::TemplateParameterInfo::TP_NON_TYPE) {
      cpp_decl::TypePtr value_type = parameter.value_type;
      if(hooks.resolve_non_type_value_type) {
        cpp_decl::TypePtr resolved = hooks.resolve_non_type_value_type(parameter, argument);
        if(resolved) {
          value_type = resolved;
        }
      }
      for_each_parameter_name(
          parameter,
          [&](const std::string & name)
          {
            bind_non_type_name(name, value_type, argument);
          });
    } else {
      for_each_parameter_name(
          parameter,
          [&](const std::string & name) { bind_type_name(name, argument.type); });
    }
    ++arg_index;
  }
}

}  // namespace template_binding
