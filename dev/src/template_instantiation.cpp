#include "template_instantiation.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

#include "class_template_mangle_info.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "callsemantic/function_registry.h"
#include "callsemantic_internal.h"
#include "callsemantic/source_location_utils.h"
#include "parser_trace.h"
#include "pack_parameter_analysis.h"
#include "semantic_class_model.h"
#include "semantic_errors.h"
#include "semantic_lookup.h"
#include "semantic_lifetime.h"
#include "semantic_overload.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api_internal.h"
#include "template_audit.h"
#include "template_argument_semantics.h"
#include "template_binding.h"
#include "template_decl_ast.h"
#include "template_function_signature.h"
#include "template_metadata.h"
#include "template_resolution.h"
#include "template_services.h"
#include "template_selection_api.h"
#include "template_scope.h"
#include "template_selection.h"
#include "symbol_linkage.h"

namespace template_argument_semantics {

bool type_depends_on_template_parameter(SemanticContext & ctx,
                                        const cpp_decl::TypePtr & type);
bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const cpp_decl::TypePtr & type);

}  // namespace template_argument_semantics

namespace template_instantiation {

using namespace cpp_decl;
using namespace semantic_model;
using namespace template_model;
using callsemantic_internal::reparseable_type_argument_text;
using callsemantic::prefer_earlier_source_location;
using semantic_trace::scope_bindings_for_diagnostic;
using semantic_trace::scope_name_for_diagnostic;

namespace {

thread_local int variable_initializer_replay_depth = 0;

void note_function_instantiation_use_location(FunctionBinding & binding,
                                              const std::string & location)
{
  if(location.empty()) {
    return;
  }
  binding.instantiation_use_location =
      prefer_earlier_source_location(binding.instantiation_use_location, location);
}

bool class_info_is_abstract(const ClassInfo & info)
{
  for(std::size_t i = 0; i < info.vtable_entries.size(); ++i) {
    const FunctionBinding * binding = info.vtable_entries[i];
    if(binding && binding->is_pure_virtual) {
      return true;
    }
  }
  return false;
}

bool variable_template_declared_in_std_namespace_or_inline_child(
    const VariableTemplateDecl & decl)
{
  const Scope * current = decl.declaring_scope;
  while(current && current->namespace_scope && current->inline_namespace) {
    current = current->parent;
  }
  if(!current ||
     !current->namespace_scope ||
     current->name != "std") {
    return false;
  }
  for(const Scope * parent = current->parent; parent; parent = parent->parent) {
    if(parent->namespace_scope &&
       parent->name != "<global>" &&
       parent->name != "<unnamed>") {
      return false;
    }
  }
  return true;
}

bool type_is_abstract_class(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info || !info->complete) {
    info = ctx.complete_class_type(base);
  }
  return info && class_info_is_abstract(*info);
}

bool type_forms_array_of_abstract_class(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  switch(base->kind) {
  case Type::TK_ARRAY:
    return type_is_abstract_class(ctx, base->inner) ||
           type_forms_array_of_abstract_class(ctx, base->inner);
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return type_forms_array_of_abstract_class(ctx, base->inner);
  case Type::TK_MEMBER_POINTER:
    return type_forms_array_of_abstract_class(ctx, base->owner) ||
           type_forms_array_of_abstract_class(ctx, base->inner);
  case Type::TK_FUNCTION:
    if(type_forms_array_of_abstract_class(ctx, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_forms_array_of_abstract_class(ctx, base->params[i])) {
        return true;
      }
    }
    return false;
  default:
    return false;
  }
}

void reject_invalid_instantiated_function_parameter_type(SemanticContext & ctx,
                                                        const FunctionTemplateDecl & decl,
                                                        const TypePtr & type)
{
  if(!type_forms_array_of_abstract_class(ctx, type)) {
    return;
  }
  std::ostringstream out;
  out << "instantiated function template parameter forms array of abstract class";
  out << " [template " << decl.name << "]";
  out << " [type " << describe_type(type) << "]";
  throw TemplateSubstitutionFailure(out.str());
}

// template-boundary-audit: begin semantic_service_access
template_api::TemplateTypeSystem & service_type_system(
    template_api::TemplateServices & services)
{
  return services.type_system;
}
// template-boundary-audit: end semantic_service_access

// template-boundary-audit: begin text_recovery_bridge
std::string instantiation_argument_type_text(SemanticContext & ctx,
                                             const TypePtr & type)
{
  return template_api::type::lookup_text_for_type_argument(ctx, type);
}

std::string instantiation_argument_type_text(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(
      type_system, type);
}

bool recover_instantiation_bound_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  return template_argument_semantics::resolve_instantiated_dependent_type(
      services, scope, type, out);
}

bool recover_instantiation_bound_type(SemanticContext & ctx,
                                      Scope & scope,
                                      const TypePtr & type,
                                      TypePtr & out)
{
  return template_api::resolve_instantiated_dependent_type(
      ctx, scope, type, out);
}

bool template_arguments_are_dependent_for_instantiation(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments);

TypePtr collapse_substituted_lvalue_reference_type(const TypePtr & inner)
{
  TypePtr base = strip_top_level_cv(inner);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    return inner;
  }
  return make_lvalue_reference_raw(inner);
}

TypePtr collapse_substituted_rvalue_reference_type(const TypePtr & inner)
{
  TypePtr base = strip_top_level_cv(inner);
  if(base && base->kind == Type::TK_LVALUE_REFERENCE) {
    return inner;
  }
  if(base && base->kind == Type::TK_RVALUE_REFERENCE) {
    return inner;
  }
  return make_rvalue_reference_raw(inner);
}

bool substitute_owner_value_template_argument(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const TemplateArgument & source,
    TemplateArgument & out)
{
  const std::string text = semantic_utils::trim_space(source.text);
  if(text.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    if(parameters[i].kind != TemplateParameterInfo::TP_NON_TYPE ||
       arguments[i].kind != TemplateArgument::TA_VALUE) {
      continue;
    }
    bool matches = parameters[i].name == text;
    for(std::size_t j = 0; !matches && j < parameters[i].alternate_names.size(); ++j) {
      matches = parameters[i].alternate_names[j] == text;
    }
    if(matches) {
      out = arguments[i];
      return true;
    }
  }
  return false;
}

bool value_source_text_mentions_template_parameter_name(
    const std::string & text,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(text.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() &&
       callsemantic_internal::contains_identifier_token(text, parameters[i].name)) {
      return true;
    }
    if(!parameters[i].placeholder_key.empty() &&
       callsemantic_internal::contains_identifier_token(
           text,
           parameters[i].placeholder_key)) {
      return true;
    }
    for(std::size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      if(!parameters[i].alternate_names[j].empty() &&
         callsemantic_internal::contains_identifier_token(
             text,
             parameters[i].alternate_names[j])) {
        return true;
      }
    }
  }
  return false;
}

std::string concrete_substituted_value_source_text(
    const TemplateArgument & argument,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(argument.kind != TemplateArgument::TA_VALUE || argument.dependent) {
    return std::string();
  }
  std::vector<std::string> candidates;
  if(argument.source_syntax) {
    if(argument.source_syntax->expression) {
      candidates.push_back(semantic_utils::trim_space(
          callsemantic_internal::describe_expression_for_diagnostic(
              *argument.source_syntax->expression)));
    }
    candidates.push_back(semantic_utils::trim_space(
        argument.source_syntax->source_text));
    candidates.push_back(semantic_utils::trim_space(argument.source_syntax->text));
  }
  candidates.push_back(semantic_utils::trim_space(argument.text));
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(!candidates[i].empty() &&
       !value_source_text_mentions_template_parameter_name(candidates[i],
                                                           parameters)) {
      return candidates[i];
    }
  }
  return std::string();
}

bool substitute_owner_arguments_in_class_template_argument(
    SemanticContext & ctx,
    template_api::TemplateServices & services,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const TemplateArgument & source,
    TemplateArgument & out);

bool substitute_owner_arguments_in_class_type(
    SemanticContext & ctx,
    template_api::TemplateServices & services,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_NAMED:
  {
    void * dependent_class_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> dependent_class_arguments;
    if(named_type_dependent_class_template(type,
                                           dependent_class_template_decl,
                                           dependent_class_arguments) &&
       dependent_class_template_decl) {
      ClassTemplateDecl * class_template =
          static_cast<ClassTemplateDecl *>(dependent_class_template_decl);
      std::vector<std::string> arg_texts;
      std::vector<TemplateArgumentSyntax> arg_syntaxes;
      arg_texts.reserve(dependent_class_arguments.size());
      arg_syntaxes.reserve(dependent_class_arguments.size());
      bool changed = false;
      for(std::size_t i = 0; i < dependent_class_arguments.size(); ++i) {
        const DependentAliasTemplateArgumentSyntax & source =
            dependent_class_arguments[i];
        std::string text = semantic_utils::trim_space(source.text.empty() ?
                                                          source.syntax.text :
                                                          source.text);
        TemplateArgumentSyntax syntax = source.syntax;
        if(source.type) {
          TypePtr substituted;
          if(template_argument_semantics::substitute_type(scope,
                                                          source.type,
                                                          parameters,
                                                          arguments,
                                                          substituted) &&
             substituted &&
             !template_argument_semantics::type_depends_on_template_parameter(
                 ctx,
                 substituted)) {
            syntax.resolved_type = substituted;
            text = instantiation_argument_type_text(ctx, substituted);
            syntax.text = text;
            changed = true;
          }
        } else {
          TemplateArgument value_arg;
          TemplateArgument source_arg;
          source_arg.kind = TemplateArgument::TA_VALUE;
          source_arg.text = text;
          if(substitute_owner_value_template_argument(parameters,
                                                      arguments,
                                                      source_arg,
                                                      value_arg)) {
            const std::string source_text =
                concrete_substituted_value_source_text(value_arg, parameters);
            text = !source_text.empty() ?
                       source_text :
                       template_model::template_argument_text(
                           value_arg,
                           [&ctx](const TypePtr & current_type)
                           {
                             return instantiation_argument_type_text(
                                 ctx,
                                 current_type);
                           });
            syntax.text = text;
            changed = true;
          }
        }
        arg_texts.push_back(text);
        arg_syntaxes.push_back(syntax);
      }
      if(changed) {
        std::vector<TemplateArgument> resolved_arguments;
        if(template_api::resolve_template_arguments(
               ctx,
               scope,
               class_template->parameters,
               arg_texts,
               arg_syntaxes.empty() ? nullptr : &arg_syntaxes,
               resolved_arguments,
               class_template->declaring_scope) &&
           !template_arguments_are_dependent_for_instantiation(ctx,
                                                               resolved_arguments)) {
          template_api::TemplateTypeLookupRequest lookup;
          lookup.scope = &scope;
          lookup.allow_class_templates = true;
          lookup.name.name = class_template->name;
          lookup.source_use_mode =
              template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly;
          template_api::TemplateSelectedClassTemplateIdRequest request;
          request.lookup = lookup;
          request.argument_scope = &scope;
          request.class_template = class_template;
          request.resolved_arguments = resolved_arguments;
          request.source_arg_texts = arg_texts;
          request.source_arg_syntaxes = arg_syntaxes;
          if(services.type_system.resolve_selected_class_template_id(request, out) &&
             out &&
             !template_argument_semantics::type_depends_on_template_parameter(ctx, out)) {
            return true;
          }
        }
      }
    }

    std::shared_ptr<const ClassTemplateSpecializationMangleInfo> mangle_info =
        named_type_class_template_specialization_mangle_info_const(type);
    if(mangle_info && mangle_info->class_template_decl) {
      std::vector<TemplateArgument> resolved_arguments;
      resolved_arguments.reserve(mangle_info->arguments.size());
      bool changed = false;
      for(std::size_t i = 0; i < mangle_info->arguments.size(); ++i) {
        TemplateArgument resolved;
        if(substitute_owner_arguments_in_class_template_argument(
               ctx,
               services,
               scope,
               parameters,
               arguments,
               mangle_info->arguments[i],
               resolved)) {
          changed = true;
        } else {
          resolved = mangle_info->arguments[i];
        }
        resolved_arguments.push_back(resolved);
      }
      if(changed &&
         !template_arguments_are_dependent_for_instantiation(ctx,
                                                             resolved_arguments)) {
        ClassTemplateDecl * class_template =
            static_cast<ClassTemplateDecl *>(mangle_info->class_template_decl);
        template_api::TemplateTypeLookupRequest lookup;
        lookup.scope = &scope;
        lookup.allow_class_templates = true;
        lookup.name.name = class_template->name;
        lookup.source_use_mode =
            template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly;
        template_api::TemplateSelectedClassTemplateIdRequest request;
        request.lookup = lookup;
        request.argument_scope = &scope;
        request.class_template = class_template;
        request.resolved_arguments = resolved_arguments;
        if(services.type_system.resolve_selected_class_template_id(request, out) &&
           out &&
           !template_argument_semantics::type_depends_on_template_parameter(ctx, out)) {
          return true;
        }
      }
    }
    return false;
  }

  case Type::TK_CV:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = apply_cv(inner, type->cv_const, type->cv_volatile);
    return true;
  }

  case Type::TK_ATOMIC:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = make_atomic(inner);
    return true;
  }

  case Type::TK_POINTER:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = make_pointer(inner);
    return true;
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = make_block_pointer(inner);
    return true;
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = collapse_substituted_lvalue_reference_type(inner);
    return true;
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = collapse_substituted_rvalue_reference_type(inner);
    return true;
  }

  case Type::TK_ARRAY:
  {
    TypePtr inner;
    if(!substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, inner)) {
      return false;
    }
    out = make_array(inner, type->has_bound, type->bound, type->bound_text);
    return true;
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypePtr owner = type->owner;
    TypePtr inner = type->inner;
    TypePtr resolved_owner;
    TypePtr resolved_inner;
    bool changed = false;
    if(substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->owner, resolved_owner)) {
      owner = resolved_owner;
      changed = true;
    }
    if(substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, resolved_inner)) {
      inner = resolved_inner;
      changed = true;
    }
    if(!changed) {
      return false;
    }
    out = make_member_pointer(owner, inner);
    return true;
  }

  case Type::TK_FUNCTION:
  {
    TypePtr result_type = type->inner;
    TypePtr resolved_result;
    bool changed = false;
    if(substitute_owner_arguments_in_class_type(
           ctx, services, scope, parameters, arguments, type->inner, resolved_result)) {
      result_type = resolved_result;
      changed = true;
    }
    std::vector<TypePtr> params_out;
    params_out.reserve(type->params.size());
    for(std::size_t i = 0; i < type->params.size(); ++i) {
      TypePtr param = type->params[i];
      TypePtr resolved_param;
      if(substitute_owner_arguments_in_class_type(
             ctx, services, scope, parameters, arguments, type->params[i], resolved_param)) {
        param = resolved_param;
        changed = true;
      }
      params_out.push_back(param);
    }
    if(!changed) {
      return false;
    }
    out = make_function(result_type,
                        params_out,
                        type->variadic,
                        type->function_const,
                        type->function_volatile,
                        type->prototype_relaxed,
                        type->function_ref_qualifier);
    return true;
  }

  case Type::TK_FUNDAMENTAL:
    return false;
  }

  return false;
}

bool substitute_owner_arguments_in_class_template_argument(
    SemanticContext & ctx,
    template_api::TemplateServices & services,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const TemplateArgument & source,
    TemplateArgument & out)
{
  out = source;
  if(source.kind == TemplateArgument::TA_TYPE && source.type) {
    TypePtr substituted;
    if(template_argument_semantics::substitute_type(scope,
                                                    source.type,
                                                    parameters,
                                                    arguments,
                                                    substituted) &&
       substituted) {
      TypePtr resolved;
      if(recover_instantiation_bound_type(ctx, scope, substituted, resolved) &&
         resolved) {
        substituted = resolved;
      }
      TypePtr class_substituted;
      if(substitute_owner_arguments_in_class_type(ctx,
                                                  services,
                                                  scope,
                                                  parameters,
                                                  arguments,
                                                  substituted,
                                                  class_substituted) &&
         class_substituted) {
        substituted = class_substituted;
      }
      if(substituted.get() != source.type.get() ||
         template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                        source.type) !=
             template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                            substituted)) {
        out.type = substituted;
        out.dependent =
            template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                           substituted);
        out.text.clear();
        return true;
      }
    }
    return false;
  }
  if(source.kind == TemplateArgument::TA_VALUE) {
    return substitute_owner_value_template_argument(parameters,
                                                   arguments,
                                                   source,
                                                   out);
  }
  return false;
}
// template-boundary-audit: end text_recovery_bridge

bool dependent_alias_argument_list_has_pack_expansion(
    const std::vector<DependentAliasTemplateArgumentSyntax> & arguments);

bool type_has_dependent_alias_pack_expansion(const TypePtr & type)
{
  if(!type) {
    return false;
  }
  if(type->kind == Type::TK_NAMED) {
    if(dependent_alias_argument_list_has_pack_expansion(
           type->named_dependent_alias_arguments) ||
       dependent_alias_argument_list_has_pack_expansion(
           type->named_dependent_class_arguments)) {
      return true;
    }
    if(type_has_dependent_alias_pack_expansion(
           type->named_dependent_qualified_owner)) {
      return true;
    }
  }
  if(type_has_dependent_alias_pack_expansion(type->inner) ||
     type_has_dependent_alias_pack_expansion(type->owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type->params.size(); ++i) {
    if(type_has_dependent_alias_pack_expansion(type->params[i])) {
      return true;
    }
  }
  return false;
}

bool text_mentions_template_parameter_name(
    const std::string & text,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty()) {
      bool changed = false;
      callsemantic_internal::replace_identifier_token_text(
          text, parameters[i].name, std::string(), changed);
      if(changed) {
        return true;
      }
    }
    if(!parameters[i].placeholder_key.empty()) {
      bool changed = false;
      callsemantic_internal::replace_identifier_token_text(
          text, parameters[i].placeholder_key, std::string(), changed);
      if(changed) {
        return true;
      }
    }
    for(std::size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      if(parameters[i].alternate_names[j].empty()) {
        continue;
      }
      bool changed = false;
      callsemantic_internal::replace_identifier_token_text(
          text, parameters[i].alternate_names[j], std::string(), changed);
      if(changed) {
        return true;
      }
    }
  }
  return false;
}

bool type_mentions_template_parameter_name(
    const TypePtr & type,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(!type) {
    return false;
  }

  if(type->kind == Type::TK_NAMED) {
    if(text_mentions_template_parameter_name(type->named_key, parameters) ||
       text_mentions_template_parameter_name(type->named_display, parameters) ||
       text_mentions_template_parameter_name(named_type_semantic_payload(type),
                                             parameters)) {
      return true;
    }
    for(std::size_t i = 0; i < type->named_dependent_alias_arguments.size(); ++i) {
      const DependentAliasTemplateArgumentSyntax & arg =
          type->named_dependent_alias_arguments[i];
      if(text_mentions_template_parameter_name(arg.text, parameters) ||
         type_mentions_template_parameter_name(arg.type, parameters)) {
        return true;
      }
    }
    for(std::size_t i = 0; i < type->named_dependent_class_arguments.size(); ++i) {
      const DependentAliasTemplateArgumentSyntax & arg =
          type->named_dependent_class_arguments[i];
      if(text_mentions_template_parameter_name(arg.text, parameters) ||
         type_mentions_template_parameter_name(arg.type, parameters)) {
        return true;
      }
    }
    if(type_mentions_template_parameter_name(type->named_dependent_qualified_owner,
                                             parameters)) {
      return true;
    }
  }

  if(type_mentions_template_parameter_name(type->inner, parameters) ||
     type_mentions_template_parameter_name(type->owner, parameters)) {
    return true;
  }
  for(std::size_t i = 0; i < type->params.size(); ++i) {
    if(type_mentions_template_parameter_name(type->params[i], parameters)) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_mentions_template_parameter_name(
    const TemplateArgumentSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters);

bool template_id_syntax_mentions_template_parameter_name(
    const TemplateIdSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(text_mentions_template_parameter_name(syntax.arguments[i], parameters)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_template_parameter_name(
           syntax.argument_syntaxes[i], parameters)) {
      return true;
    }
  }
  return false;
}

bool ast_mentions_template_parameter_name(
    const CppAstNode & node,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(text_mentions_template_parameter_name(node.value, parameters)) {
    return true;
  }
  if(node.semantic_type &&
     type_mentions_template_parameter_name(node.semantic_type, parameters)) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_mentions_template_parameter_name(*node.template_id_syntax,
                                                        parameters)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_mentions_template_parameter_name(*node.conversion_type_id_syntax,
                                          parameters)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_mentions_template_parameter_name(*node.base_type_syntax, parameters)) {
    return true;
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_template_parameter_name(
           node.qualifier_template_id_syntaxes[i], parameters)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_mentions_template_parameter_name(node.qualifier_type_syntaxes[i],
                                            parameters)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_mentions_template_parameter_name(node.children[i], parameters)) {
      return true;
    }
  }
  return false;
}

bool template_parameters_have_pack(
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      return true;
    }
  }
  return false;
}

bool ast_node_is_decltype_specifier(const CppAstNode & node)
{
  if(node.kind == CppAstKind::decltype_specifier) {
    return true;
  }
  if(node.kind != CppAstKind::decl_specifier) {
    return false;
  }
  return node.value.compare(0, 8, "decltype") == 0 ||
         node.value.compare(0, 10, "__decltype") == 0;
}

bool type_id_has_top_level_decltype_specifier(const CppAstNode & node)
{
  if(ast_node_is_decltype_specifier(node)) {
    return true;
  }
  if(node.kind != CppAstKind::type_id &&
     node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::type_specifier_seq) {
    return false;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(ast_node_is_decltype_specifier(child)) {
      return true;
    }
    if((child.kind == CppAstKind::decl_specifier_seq ||
        child.kind == CppAstKind::type_specifier_seq) &&
       type_id_has_top_level_decltype_specifier(child)) {
      return true;
    }
  }
  return false;
}

bool template_id_is_enable_if_alias(const TemplateIdSyntax & syntax)
{
  return syntax.name.name == "enable_if" || syntax.name.name == "enable_if_t";
}

bool type_id_has_top_level_enable_if_template_id(const CppAstNode & node)
{
  if(node.template_id_syntax &&
     template_id_is_enable_if_alias(*node.template_id_syntax)) {
    return true;
  }
  if(node.kind != CppAstKind::type_id &&
     node.kind != CppAstKind::decl_specifier_seq &&
     node.kind != CppAstKind::type_specifier_seq) {
    return false;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.template_id_syntax &&
       template_id_is_enable_if_alias(*child.template_id_syntax)) {
      return true;
    }
    if((child.kind == CppAstKind::decl_specifier_seq ||
        child.kind == CppAstKind::type_specifier_seq) &&
       type_id_has_top_level_enable_if_template_id(child)) {
      return true;
    }
  }
  return false;
}

void clear_cached_semantic_types_impl(CppAstNode & node,
                                      bool clear_resolved_argument_types);

void clear_cached_semantic_types_impl(TemplateArgumentSyntax & syntax,
                                      bool clear_resolved_argument_types)
{
  if(clear_resolved_argument_types) {
    syntax.resolved_type.reset();
  }
  if(syntax.template_id) {
    for(std::size_t i = 0; i < syntax.template_id->argument_syntaxes.size(); ++i) {
      clear_cached_semantic_types_impl(syntax.template_id->argument_syntaxes[i],
                                       clear_resolved_argument_types);
    }
  }
  if(syntax.type_id) {
    clear_cached_semantic_types_impl(*syntax.type_id, clear_resolved_argument_types);
  }
  if(syntax.expression) {
    clear_cached_semantic_types_impl(*syntax.expression,
                                     clear_resolved_argument_types);
  }
}

void clear_cached_semantic_types_impl(TemplateIdSyntax & syntax,
                                      bool clear_resolved_argument_types)
{
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    clear_cached_semantic_types_impl(syntax.argument_syntaxes[i],
                                     clear_resolved_argument_types);
  }
}

void clear_cached_semantic_types_impl(CppAstNode & node,
                                      bool clear_resolved_argument_types)
{
  node.semantic_type.reset();
  if(node.template_id_syntax) {
    clear_cached_semantic_types_impl(*node.template_id_syntax,
                                     clear_resolved_argument_types);
  }
  if(node.conversion_type_id_syntax) {
    clear_cached_semantic_types_impl(*node.conversion_type_id_syntax,
                                     clear_resolved_argument_types);
  }
  if(node.base_type_syntax) {
    clear_cached_semantic_types_impl(*node.base_type_syntax,
                                     clear_resolved_argument_types);
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    clear_cached_semantic_types_impl(node.qualifier_template_id_syntaxes[i],
                                     clear_resolved_argument_types);
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    clear_cached_semantic_types_impl(node.qualifier_type_syntaxes[i],
                                     clear_resolved_argument_types);
  }
  for(std::size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    clear_cached_semantic_types_impl(node.exception_type_id_syntaxes[i],
                                     clear_resolved_argument_types);
  }
  for(std::size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    clear_cached_semantic_types_impl(node.alignment_specifier_nodes[i],
                                     clear_resolved_argument_types);
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    clear_cached_semantic_types_impl(node.children[i],
                                     clear_resolved_argument_types);
  }
}

bool template_argument_syntax_mentions_template_parameter_name(
    const TemplateArgumentSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(text_mentions_template_parameter_name(syntax.text, parameters) ||
     text_mentions_template_parameter_name(syntax.source_text, parameters) ||
     (syntax.resolved_type &&
      type_mentions_template_parameter_name(syntax.resolved_type, parameters))) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_template_parameter_name(*syntax.template_id,
                                                        parameters)) {
    return true;
  }
  if(syntax.type_id &&
     ast_mentions_template_parameter_name(*syntax.type_id, parameters)) {
    return true;
  }
  return syntax.expression &&
         ast_mentions_template_parameter_name(*syntax.expression, parameters);
}

bool dependent_alias_argument_list_has_pack_expansion(
    const std::vector<DependentAliasTemplateArgumentSyntax> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].syntax.pack_expansion ||
       type_has_dependent_alias_pack_expansion(arguments[i].type)) {
      return true;
    }
  }
  return false;
}

std::string qualified_variable_template_name(const VariableTemplateDecl & decl)
{
  return decl.declaring_scope ?
      semantic_lookup::scope_qualified_name(*decl.declaring_scope, decl.name) :
      decl.name;
}

std::string source_binding_param_name(const TemplateParameterInfo & parameter,
                                      std::size_t index)
{
  return parameter.name.empty() ?
      std::string("$") + std::to_string(index + 1) :
      parameter.name;
}

std::string normalized_template_name_or_node_location(
    SemanticContext & ctx,
    const CppAstNode * node,
    const std::string & name,
    bool require_name_after_node_begin = false)
{
  if(!node) {
    return std::string();
  }

  std::string location =
      template_api::normalize_template_witness_source_location(
          ctx.source_location_for_name_in_node(*node, name));
  if(!location.empty()) {
    if(!require_name_after_node_begin) {
      return location;
    }
    const template_api::template_witness_detail::ParsedSourceLocation parsed_location =
        template_api::template_witness_detail::parse_source_location(location);
    const template_api::template_witness_detail::ParsedSourceLocation parsed_begin =
        template_api::template_witness_detail::parse_source_location(
            ctx.source_location_for_node(*node));
    if(parsed_location.valid && parsed_begin.valid &&
       parsed_location.file == parsed_begin.file &&
       parsed_location.line >= parsed_begin.line) {
      return location;
    }
  }

  return template_api::normalize_template_witness_source_location(
      ctx.source_location_for_node(*node));
}

std::string source_binding_source_for_arg_text(
    SemanticContext & ctx,
    const TemplateParameterInfo & parameter,
    const std::string & arg_text)
{
  if(parameter.default_argument == nullptr) {
    return "explicit";
  }
  const CppAstNode * default_payload =
      !parameter.default_argument->children.empty() ?
          &parameter.default_argument->children[0] :
          parameter.default_argument;
  std::string default_text =
      semantic_utils::trim_space(cpp_decl::node_text(*default_payload));
  if(default_text.empty()) {
    const template_api::TemplateWitnessContext witness_context =
        ctx.template_witness_context();
    if(witness_context.token_sequence &&
       cpp_decl::has_valid_node_span(*witness_context.token_sequence,
                                     *default_payload)) {
      default_text = semantic_utils::trim_space(
          callsemantic_internal::spaced_token_span_text(
              *witness_context.token_sequence,
              default_payload->token_start,
              default_payload->token_end));
    }
  }
  if(!default_text.empty() &&
     semantic_utils::trim_space(arg_text) == default_text) {
    return "defaulted";
  }
  return "explicit";
}

void append_class_use_binding_texts(SemanticContext & ctx,
                                    const ClassInfo & info,
                                    std::vector<witness::TemplateWitnessSourceBinding> & out)
{
  if(!(info.source_template && !info.instantiation_arguments.empty())) {
    return;
  }
  const std::vector<TemplateParameterInfo> & parameters =
      info.source_template->parameters;
  const std::vector<std::string> * const arg_texts =
      template_metadata::argument_texts(info);
  const std::size_t text_count = arg_texts ?
      std::min(parameters.size(), arg_texts->size()) :
      0;
  if(text_count != 0) {
    for(std::size_t i = 0; i < text_count; ++i) {
      witness::TemplateWitnessSourceBinding binding;
      binding.param = source_binding_param_name(parameters[i], i);
      binding.arg = (*arg_texts)[i];
      binding.source =
          source_binding_source_for_arg_text(ctx, parameters[i], binding.arg);
      binding.type_like =
          parameters[i].kind == TemplateParameterInfo::TP_TYPE;
      out.push_back(binding);
    }
    return;
  }
  template_api::append_class_template_witness_bindings(ctx, &info, out);
}

void note_out_of_class_owner_class_use_for_applied_definition(
    SemanticContext & ctx,
    const ClassTemplateDecl * source_template_override,
    const ClassInfo & info,
    const CppAstNode * anchor_node,
    bool static_member_definition_witness_replay = false)
{
  const ClassTemplateDecl * source_template =
      info.source_template ? info.source_template : source_template_override;
  const bool capture_enabled =
      witness::source_capture_enabled(ctx.template_witness_context());
  if(!(anchor_node &&
       source_template &&
       !info.instantiation_arguments.empty()) ||
     !capture_enabled) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "note-out-of-class-owner-class-use skip"
            << " class=" << info.qualified_name
            << " anchor=" << (anchor_node ? "yes" : "no")
            << " source-template=" << (source_template ? source_template->name : std::string("<none>"))
            << " arg-count=" << info.instantiation_arguments.size()
            << " capture=" << (capture_enabled ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return;
  }

  const std::string location =
      normalized_template_name_or_node_location(ctx, anchor_node, source_template->name);
  if(location.empty()) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "note-out-of-class-owner-class-use skip-empty-location"
            << " class=" << info.qualified_name
            << " source-template=" << source_template->name;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return;
  }

  witness::ClassUseEmitRequest request;
  request.location = location;
  request.use_anchor_present = true;
  request.use_anchor_location = location;
  request.template_name =
      semantic_utils::strip_trailing_top_level_template_arguments(
          semantic_model::class_output_qualified_name(info));
  if(request.template_name.empty()) {
    request.template_name = source_template->name;
  }
  request.selection =
      (info.template_output_node &&
       source_template->class_node &&
       info.template_output_node != source_template->class_node) ||
              info.is_explicit_specialization ?
          witness::SourceSelectionKind::ExplicitSpecialization :
          witness::SourceSelectionKind::Primary;

  const semantic_model::SourceDeclAnchorCache & decl_anchor =
      semantic_trace::class_decl_anchor(ctx, &info);
  witness::set_selected_decl_anchor(request.selected_decl_location,
                                    request.selected_decl_anchor,
                                    decl_anchor);
  append_class_use_binding_texts(ctx, info, request.bindings);
  request.role = witness::SourceUseRole::StaticMemberDefinitionOwner;
  if(static_member_definition_witness_replay) {
    request.origin = witness::ClassUseEmissionOrigin::DeclarationTypeSource;
  }
  witness::emit_class_use(request);
  if(info.member_scope) {
    if(static_member_definition_witness_replay) {
      ctx.emit_static_member_definition_class_use_source_events_from_ast_node(
          *info.member_scope,
          *anchor_node,
          witness::SourceUseOwnership::SourceOwned);
    } else {
      ctx.emit_nested_class_use_source_events_from_ast_node(
          *info.member_scope,
          *anchor_node,
          witness::SourceUseOwnership::SourceOwned);
    }
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "note-out-of-class-owner-class-use recorded"
          << " class=" << info.qualified_name
          << " location=" << location
          << " template=" << request.template_name;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
}

}  // namespace

void clear_cached_semantic_types(CppAstNode & node,
                                 bool clear_resolved_argument_types = true)
{
  clear_cached_semantic_types_impl(node, clear_resolved_argument_types);
}

void clear_dependent_cached_semantic_types_impl(
    TemplateArgumentSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters);

void clear_dependent_cached_semantic_types_impl(
    TemplateIdSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(syntax.argument_syntaxes[i],
                                               parameters);
  }
}

void clear_dependent_cached_semantic_types_impl(
    CppAstNode & node,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(node.semantic_type &&
     type_mentions_template_parameter_name(node.semantic_type, parameters)) {
    node.semantic_type.reset();
  }
  if(node.template_id_syntax) {
    clear_dependent_cached_semantic_types_impl(*node.template_id_syntax,
                                               parameters);
  }
  if(node.conversion_type_id_syntax) {
    clear_dependent_cached_semantic_types_impl(*node.conversion_type_id_syntax,
                                               parameters);
  }
  if(node.base_type_syntax) {
    clear_dependent_cached_semantic_types_impl(*node.base_type_syntax,
                                               parameters);
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(
        node.qualifier_template_id_syntaxes[i],
        parameters);
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(node.qualifier_type_syntaxes[i],
                                               parameters);
  }
  for(std::size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(
        node.exception_type_id_syntaxes[i],
        parameters);
  }
  for(std::size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(
        node.alignment_specifier_nodes[i],
        parameters);
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    clear_dependent_cached_semantic_types_impl(node.children[i], parameters);
  }
}

void clear_dependent_cached_semantic_types_impl(
    TemplateArgumentSyntax & syntax,
    const std::vector<TemplateParameterInfo> & parameters)
{
  if(syntax.resolved_type &&
     type_mentions_template_parameter_name(syntax.resolved_type, parameters)) {
    syntax.resolved_type.reset();
  }
  if(syntax.template_id) {
    clear_dependent_cached_semantic_types_impl(*syntax.template_id,
                                               parameters);
  }
  if(syntax.type_id) {
    clear_dependent_cached_semantic_types_impl(*syntax.type_id, parameters);
  }
  if(syntax.expression) {
    clear_dependent_cached_semantic_types_impl(*syntax.expression, parameters);
  }
}

void clear_dependent_cached_semantic_types(
    CppAstNode & node,
    const std::vector<TemplateParameterInfo> & parameters)
{
  clear_dependent_cached_semantic_types_impl(node, parameters);
}

void bind_template_arguments_into_scope(
    SemanticContext & ctx,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

void bind_declaring_owner_instantiation_context(SemanticContext & ctx,
                                                Scope & scope,
                                                const Scope & declaring_scope);

Scope & bind_template_arguments(
    SemanticContext & ctx,
    Scope & declaring_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    Scope & declaring_scope,
    Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    ClassInfo * active_owner);

Scope & bind_class_template_arguments_for_instantiation(
    SemanticContext & ctx,
    Scope & declaring_scope,
    Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

ClassInfo * instantiate_class_template(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<TemplateArgument> & arguments);

ClassInfo * instantiate_class_template(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<std::string> & arg_texts);

void finalize_instantiated_class(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments);

FunctionBinding * instantiate_function_template(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const std::vector<TemplateArgument> & arguments,
    ClassInfo * active_owner = nullptr,
    const CppAstNode * body_override = nullptr,
    const CppAstNode * definition_node_override = nullptr,
    bool explicit_specialization = false,
    bool explicit_specialization_is_constexpr = false,
    bool include_body = true,
    Scope * use_scope = nullptr,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr,
    bool prefer_overload_suffix = false,
    const std::string & instantiation_use_location_override = std::string());

const ValueBinding * instantiate_variable_template(
    SemanticContext & ctx,
    VariableTemplateDecl & decl,
    const std::vector<TemplateArgument> & arguments,
    const std::string & source_use_location = std::string(),
    Scope * source_use_scope = nullptr);

namespace {

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location)
      : active(!location.empty())
  {
    if(active) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateUseLocation()
  {
    if(active) {
      parser_trace::pop_use_location();
    }
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;

  bool active;
};

struct ScopedVariableInitializerReplay
{
  ScopedVariableInitializerReplay()
  {
    ++variable_initializer_replay_depth;
  }

  ~ScopedVariableInitializerReplay()
  {
    --variable_initializer_replay_depth;
  }

  ScopedVariableInitializerReplay(const ScopedVariableInitializerReplay &) = delete;
  ScopedVariableInitializerReplay & operator=(const ScopedVariableInitializerReplay &) = delete;
};

void parse_explicit_function_nothrow_parse_state(
    const CppAstNode * qualifier,
    ExplicitFunctionNothrowKind & kind,
    std::string & expr_text)
{
  kind = EFNK_NONE;
  expr_text.clear();
  if(!qualifier) {
    return;
  }

  const std::string & raw_text = qualifier->value;
  if(raw_text.find("noexcept") == std::string::npos &&
     raw_text.find("throw") == std::string::npos) {
    kind = EFNK_INVALID;
    return;
  }

  const std::string text = semantic_utils::trim_space(raw_text);
  if(text == "noexcept" || text == "throw()") {
    kind = EFNK_ALWAYS_TRUE;
    return;
  }
  if(text.compare(0, 6, "throw(") == 0 && !text.empty() && text.back() == ')') {
    kind = EFNK_ALWAYS_FALSE;
    return;
  }
  if(text.compare(0, 9, "noexcept(") == 0 && !text.empty() && text.back() == ')') {
    kind = EFNK_EXPR;
    expr_text = text.substr(9, text.size() - 10);
    return;
  }

  kind = EFNK_INVALID;
}

bool out_of_class_definition_exception_spec_matches(
    SemanticContext & ctx,
    FunctionBinding & binding,
    const CppAstNode * declarator,
    const CppAstNode * body)
{
  const CppAstNode * qualifier =
      declarator ? semantic_class_model::declarator_function_qualifier(*declarator) : nullptr;

  ExplicitFunctionNothrowKind candidate_kind = EFNK_UNINITIALIZED;
  std::string candidate_expr_text;
  parse_explicit_function_nothrow_parse_state(qualifier, candidate_kind, candidate_expr_text);

  ExplicitFunctionNothrowKind existing_kind = EFNK_UNINITIALIZED;
  std::string existing_expr_text;
  parse_explicit_function_nothrow_parse_state(binding.function_qualifier,
                                              existing_kind,
                                              existing_expr_text);

  bool existing_value = false;
  const bool existing_explicit =
      existing_kind != EFNK_NONE && existing_kind != EFNK_INVALID;
  const bool candidate_explicit =
      candidate_kind != EFNK_NONE && candidate_kind != EFNK_INVALID;
  const auto compact_whitespace =
      [](const std::string & text) -> std::string
  {
    std::string out;
    out.reserve(text.size());
    for(std::size_t i = 0; i < text.size(); ++i) {
      if(!std::isspace(static_cast<unsigned char>(text[i]))) {
        out.push_back(text[i]);
      }
    }
    return out;
  };
  if(binding.function_qualifier &&
     qualifier &&
     compact_whitespace(binding.function_qualifier->value) ==
         compact_whitespace(qualifier->value)) {
    return true;
  }
  if(existing_explicit != candidate_explicit) {
    if(existing_explicit &&
       !candidate_explicit &&
       body &&
       body->kind == CppAstKind::lazy_function_body &&
       binding.function_qualifier &&
       (!qualifier || semantic_utils::trim_space(qualifier->value).empty())) {
      return true;
    }
    return false;
  }
  if(!existing_explicit) {
    return true;
  }

  const bool existing_known =
      ctx.evaluate_explicit_function_nothrow_semantically(binding, existing_value);
  bool candidate_value = false;
  bool candidate_known = false;
  switch(candidate_kind) {
  case EFNK_ALWAYS_TRUE:
    candidate_value = true;
    candidate_known = true;
    break;
  case EFNK_ALWAYS_FALSE:
    candidate_value = false;
    candidate_known = true;
    break;
  case EFNK_EXPR:
  {
    Scope eval_scope(binding.declaration_scope, "<template-out-of-class-noexcept>");
    eval_scope.class_info = binding.owner_class;
    eval_scope.function = &binding;
    const CppAstNode * expr =
        qualifier && !qualifier->children.empty() ?
            &qualifier->children[0] :
            nullptr;
    constant_eval::ConstexprValue value;
    candidate_known =
        !candidate_expr_text.empty() &&
        expr != nullptr &&
        ctx.evaluate_constant_expression_value(eval_scope, *expr, value) &&
        constant_eval::constexpr_value_truthy(value, candidate_value);
    break;
  }
  case EFNK_NONE:
  case EFNK_INVALID:
  case EFNK_UNINITIALIZED:
    break;
  }

  if(existing_known && candidate_known) {
    return existing_value == candidate_value;
  }
  if(existing_kind != candidate_kind) {
    return false;
  }
  if(existing_kind == EFNK_EXPR) {
    return existing_expr_text == candidate_expr_text;
  }
  return true;
}

void apply_function_instantiation_intent(SemanticContext & ctx,
                                         FunctionBinding * binding,
                                         InstantiatedFunctionOutputMode mode)
{
  if(!binding) {
    return;
  }
  ctx.note_instantiated_function_output(binding, mode);
}

bool is_identifier_char(unsigned char ch)
{
  return std::isalnum(ch) || ch == '_';
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

const CppAstNode * find_qualified_operator_identifier(const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier &&
     node.value.find("::") != std::string::npos &&
     node.value.find("operator") != std::string::npos) {
    return &node;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode * found = find_qualified_operator_identifier(node.children[i]);
    if(found) {
      return found;
    }
  }
  return nullptr;
}

bool is_leading_type_keyword(const std::string & token)
{
  return token == "const" ||
         token == "volatile" ||
         token == "typename" ||
         token == "class" ||
         token == "struct" ||
         token == "union" ||
         token == "enum";
}

std::string strip_template_id_suffix(std::string text)
{
  const std::size_t lt = text.find('<');
  return lt == std::string::npos ? text : text.substr(0, lt);
}

TypePtr rebind_special_member_self_parameter_type(const TypePtr & type,
                                                  const TypePtr & source_class_type,
                                                  const TypePtr & target_class_type)
{
  if(!type || !source_class_type || !target_class_type) {
    return TypePtr();
  }

  TypePtr source_base = strip_top_level_cv(source_class_type);
  if(!source_base) {
    return TypePtr();
  }

  TypePtr param_base = remove_reference_type(type);
  TypePtr param_unqualified = strip_top_level_cv(param_base);
  const auto normalized_named_text = [](const TypePtr & named) -> std::string
  {
    if(!named || named->kind != Type::TK_NAMED) {
      return std::string();
    }
    std::string text = semantic_utils::trim_space(
        semantic_utils::strip_elaborated_type_prefix(
            named->named_display.empty() ? named->named_key : named->named_display));
    const std::string typename_prefix = "typename ";
    while(text.compare(0, typename_prefix.size(), typename_prefix) == 0) {
      text = semantic_utils::trim_space(text.substr(typename_prefix.size()));
    }
    return text;
  };
  const auto self_type_matches = [&](const TypePtr & lhs, const TypePtr & rhs) -> bool
  {
    if(type_equals(lhs, rhs)) {
      return true;
    }
    if(!lhs || !rhs || lhs->kind != Type::TK_NAMED || rhs->kind != Type::TK_NAMED) {
      return false;
    }
    const std::string lhs_text = normalized_named_text(lhs);
    const std::string rhs_text = normalized_named_text(rhs);
    if(lhs_text.empty() || rhs_text.empty()) {
      return false;
    }
    return lhs_text == rhs_text ||
           semantic_utils::unqualified_member_name(lhs_text) ==
               semantic_utils::unqualified_member_name(rhs_text);
  };
  if(!param_unqualified || !self_type_matches(param_unqualified, source_base)) {
    return TypePtr();
  }

  TypePtr rebound =
      apply_cv(target_class_type,
               param_base && param_base->kind == Type::TK_CV && param_base->cv_const,
               param_base && param_base->kind == Type::TK_CV && param_base->cv_volatile);
  if(!rebound) {
    return TypePtr();
  }

  TypePtr outer = strip_top_level_cv(type);
  if(!outer) {
    return rebound;
  }
  if(outer->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(rebound);
  }
  if(outer->kind == Type::TK_RVALUE_REFERENCE) {
    return make_rvalue_reference_raw(rebound);
  }
  return rebound;
}

TypePtr rebind_out_of_class_member_self_type(const TypePtr & type,
                                             SemanticContext & ctx,
                                             const QualifiedName & qualified,
                                             const TypePtr & target_class_type)
{
  if(!type || !target_class_type) {
    return TypePtr();
  }
  if(qualified.qualifiers.empty()) {
    return TypePtr();
  }

  std::string owner_text = semantic_utils::trim_space(qualified.qualifiers.back());
  owner_text = semantic_utils::trim_space(
      semantic_utils::strip_elaborated_type_prefix(owner_text));
  const TypePtr base = remove_reference_type(type);
  const TypePtr unqualified = strip_top_level_cv(base);
  if(!unqualified || unqualified->kind != Type::TK_NAMED) {
    return TypePtr();
  }

  std::string current_text = semantic_utils::trim_space(
      semantic_utils::strip_elaborated_type_prefix(
          unqualified->named_display.empty() ? unqualified->named_key :
                                               unqualified->named_display));
  const std::string typename_prefix = "typename ";
  while(current_text.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    current_text = semantic_utils::trim_space(current_text.substr(typename_prefix.size()));
  }
  while(owner_text.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    owner_text = semantic_utils::trim_space(owner_text.substr(typename_prefix.size()));
  }
  if(current_text != owner_text) {
    const bool unqualified_match =
        semantic_utils::unqualified_member_name(current_text) ==
        semantic_utils::unqualified_member_name(owner_text);
    ClassInfo * source_info = unqualified_match ? ctx.class_info_for_type(unqualified) : nullptr;
    TypePtr target_unqualified = strip_top_level_cv(target_class_type);
    ClassInfo * target_info =
        source_info && target_unqualified ? ctx.class_info_for_type(target_unqualified) : nullptr;
    const bool same_template_identity =
        source_info &&
        target_info &&
        (source_info == target_info ||
         (source_info->source_template &&
          source_info->source_template == target_info->source_template));
    if(!same_template_identity) {
      return TypePtr();
    }
  }

  TypePtr rebound =
      apply_cv(target_class_type,
               base && base->kind == Type::TK_CV && base->cv_const,
               base && base->kind == Type::TK_CV && base->cv_volatile);
  if(!rebound) {
    return TypePtr();
  }

  TypePtr outer = strip_top_level_cv(type);
  if(!outer) {
    return rebound;
  }
  if(outer->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(rebound);
  }
  if(outer->kind == Type::TK_RVALUE_REFERENCE) {
    return make_rvalue_reference_raw(rebound);
  }
  return rebound;
}

TypePtr apply_original_cv_ref_qualifiers(const TypePtr & original,
                                         const TypePtr & rebound)
{
  if(!original || !rebound) {
    return TypePtr();
  }

  const TypePtr base = remove_reference_type(original);
  TypePtr adjusted =
      apply_cv(rebound,
               base && base->kind == Type::TK_CV && base->cv_const,
               base && base->kind == Type::TK_CV && base->cv_volatile);
  if(!adjusted) {
    return TypePtr();
  }

  const TypePtr outer = strip_top_level_cv(original);
  if(!outer) {
    return adjusted;
  }
  if(outer->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(adjusted);
  }
  if(outer->kind == Type::TK_RVALUE_REFERENCE) {
    return make_rvalue_reference_raw(adjusted);
  }
  return adjusted;
}

std::string normalized_current_instantiation_text(std::string text)
{
  text = semantic_utils::trim_space(
      semantic_utils::strip_elaborated_type_prefix(text));
  const std::string typename_prefix = "typename ";
  while(text.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    text = semantic_utils::trim_space(text.substr(typename_prefix.size()));
  }
  return text;
}

bool current_instantiation_owner_text_matches(const std::string & lhs,
                                              const std::string & rhs)
{
  const std::string normalized_lhs = normalized_current_instantiation_text(lhs);
  const std::string normalized_rhs = normalized_current_instantiation_text(rhs);
  if(normalized_lhs == normalized_rhs) {
    return true;
  }
  return semantic_utils::unqualified_member_name(normalized_lhs) ==
         semantic_utils::unqualified_member_name(normalized_rhs);
}

TypePtr rebind_out_of_class_member_nested_self_type(
    const TypePtr & type,
    SemanticContext & ctx,
    const QualifiedName & qualified,
    const TypePtr & target_class_type)
{
  if(!type || !target_class_type || qualified.qualifiers.empty()) {
    return TypePtr();
  }

  const TypePtr base = remove_reference_type(type);
  const TypePtr unqualified = strip_top_level_cv(base);
  if(!unqualified || unqualified->kind != Type::TK_NAMED) {
    return TypePtr();
  }

  const std::string current_text =
      normalized_current_instantiation_text(
          unqualified->named_display.empty() ? unqualified->named_key :
                                               unqualified->named_display);
  QualifiedName current_qualified;
  if(!semantic_utils::split_qualified_name_text(current_text, current_qualified)) {
    return TypePtr();
  }

  ClassInfo * target_info =
      ctx.class_info_for_type(strip_top_level_cv(target_class_type));
  if(!target_info || !target_info->member_scope) {
    return TypePtr();
  }

  if(!current_qualified.qualifiers.empty()) {
    TypePtr rebound =
        ctx.lookup_non_template_type_name(*target_info->member_scope, current_text);
    if(rebound && !type_equals(rebound, unqualified)) {
      return apply_original_cv_ref_qualifiers(type, rebound);
    }
  }

  if(current_qualified.qualifiers.empty()) {
    TypePtr rebound =
        ctx.lookup_non_template_type_name(*target_info->member_scope,
                                          current_qualified.name);
    if(!rebound || type_equals(rebound, unqualified)) {
      return TypePtr();
    }
    return apply_original_cv_ref_qualifiers(type, rebound);
  }

  ClassInfo * nested_info = ctx.class_info_for_type(unqualified);
  ClassInfo * nested_owner_info =
      nested_info && nested_info->enclosing_scope ?
          nested_info->enclosing_scope->class_info :
          nullptr;
  const bool same_template_owner =
      nested_owner_info &&
      (nested_owner_info == target_info ||
       (nested_owner_info->source_template &&
        nested_owner_info->source_template == target_info->source_template));
  if(!same_template_owner) {
    const std::string owner_text = qualified.qualifiers.back();
    const std::string nested_owner_text = current_qualified.qualifiers.back();
    if(!current_instantiation_owner_text_matches(nested_owner_text, owner_text)) {
      return TypePtr();
    }
  }

  TypePtr rebound =
      ctx.lookup_non_template_type_name(*target_info->member_scope,
                                        current_qualified.name);
  if(!rebound) {
    return TypePtr();
  }
  return apply_original_cv_ref_qualifiers(type, rebound);
}

TypePtr rebind_out_of_class_member_self_or_nested_type(
    const TypePtr & type,
    SemanticContext & ctx,
    const QualifiedName & qualified,
    const TypePtr & target_class_type)
{
  TypePtr rebound =
      rebind_out_of_class_member_self_type(type, ctx, qualified, target_class_type);
  if(rebound) {
    return rebound;
  }
  return rebind_out_of_class_member_nested_self_type(
      type, ctx, qualified, target_class_type);
}

TypePtr rebind_out_of_class_member_self_or_nested_type_tree(
    const TypePtr & type,
    SemanticContext & ctx,
    const QualifiedName & qualified,
    const TypePtr & target_class_type)
{
  if(!type) {
    return TypePtr();
  }

  TypePtr direct =
      rebind_out_of_class_member_self_or_nested_type(type,
                                                     ctx,
                                                     qualified,
                                                     target_class_type);
  if(direct) {
    return direct;
  }

  switch(type->kind) {
  case Type::TK_CV:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return apply_cv(inner, type->cv_const, type->cv_volatile);
    }
    break;

  case Type::TK_ATOMIC:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_atomic(inner);
    }
    break;

  case Type::TK_POINTER:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_pointer(inner);
    }
    break;

  case Type::TK_MEMBER_POINTER:
    {
      TypePtr owner =
          rebind_out_of_class_member_self_or_nested_type_tree(
              type->owner, ctx, qualified, target_class_type);
      TypePtr inner =
          rebind_out_of_class_member_self_or_nested_type_tree(
              type->inner, ctx, qualified, target_class_type);
      if(owner || inner) {
        return make_member_pointer(owner ? owner : type->owner,
                                   inner ? inner : type->inner);
      }
    }
    break;

  case Type::TK_BLOCK_POINTER:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_block_pointer(inner);
    }
    break;

  case Type::TK_LVALUE_REFERENCE:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_lvalue_reference_raw(inner);
    }
    break;

  case Type::TK_RVALUE_REFERENCE:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_rvalue_reference_raw(inner);
    }
    break;

  case Type::TK_ARRAY:
    if(TypePtr inner =
           rebind_out_of_class_member_self_or_nested_type_tree(
               type->inner, ctx, qualified, target_class_type)) {
      return make_array(inner, type->has_bound, type->bound, type->bound_text);
    }
    break;

  case Type::TK_FUNCTION:
    {
      bool changed = false;
      TypePtr result = type->inner;
      if(TypePtr rebound_result =
             rebind_out_of_class_member_self_or_nested_type_tree(
                 type->inner, ctx, qualified, target_class_type)) {
        result = rebound_result;
        changed = true;
      }

      std::vector<TypePtr> params = type->params;
      for(std::size_t i = 0; i < params.size(); ++i) {
        if(TypePtr rebound_param =
               rebind_out_of_class_member_self_or_nested_type_tree(
                   params[i], ctx, qualified, target_class_type)) {
          params[i] = rebound_param;
          changed = true;
        }
      }

      if(changed) {
        return make_function(result,
                             params,
                             type->variadic,
                             type->function_const,
                             type->function_volatile,
                             type->prototype_relaxed,
                             type->function_ref_qualifier);
      }
    }
    break;

  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    break;
  }

  return TypePtr();
}

void rebind_out_of_class_member_signature_types(
    SemanticContext & ctx,
    const QualifiedName & qualified,
    ClassInfo & info,
    TypePtr & declared_type,
    std::vector<std::pair<std::string, TypePtr> > & params)
{
  if(declared_type) {
    if(TypePtr rebound =
           rebind_out_of_class_member_self_or_nested_type_tree(
               declared_type, ctx, qualified, info.type)) {
      declared_type = rebound;
    }
  }

  for(std::size_t i = 0; i < params.size(); ++i) {
    if(TypePtr rebound =
           rebind_out_of_class_member_self_or_nested_type_tree(
               params[i].second, ctx, qualified, info.type)) {
      params[i].second = rebound;
    }
  }
}

bool qualified_name_is_special_member(const QualifiedName & qualified)
{
  if(qualified.qualifiers.empty()) {
    return false;
  }

  const std::string owner_name = strip_template_id_suffix(qualified.qualifiers.back());
  return qualified.name == owner_name ||
         qualified.name == ("~" + owner_name);
}

bool stored_member_definition_matches_target_class(
    SemanticContext & ctx,
    const OutOfClassMemberFunctionDecl & stored,
    const ClassInfo & info)
{
  (void)ctx;
  const CppAstNode * target_node =
      info.template_output_node ? info.template_output_node : info.class_node;
  if(!stored.owner_output_node || !target_node) {
    return true;
  }
  return stored.owner_output_node == target_node;
}

bool resolve_stored_out_of_class_method_binding_in_target(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::string & member_name,
    const TypePtr & declared_type,
    bool is_const_method,
    bool is_volatile_method,
    RefQualifier ref_qualifier,
    FunctionBinding *& out)
{
  out = nullptr;
  if(!info.member_scope || !declared_type) {
    return false;
  }

  TypePtr effective_declared_type = declared_type;
  TypePtr resolved_declared_type;
  if(recover_instantiation_bound_type(ctx,
                                      *info.member_scope,
                                      effective_declared_type,
                                      resolved_declared_type) &&
     resolved_declared_type) {
    effective_declared_type = resolved_declared_type;
  }

  TypePtr expected_type =
      semantic_class_model::method_function_type(info.type,
                                                 is_const_method,
                                                 is_volatile_method,
                                                 effective_declared_type);
  if(!expected_type) {
    return false;
  }

  out = ctx.find_exact_class_function(info, member_name, expected_type, ref_qualifier);
  if(!out) {
    out = ctx.find_equivalent_class_function(info,
                                             member_name,
                                             expected_type,
                                             ref_qualifier);
  }
  if(!out && !ctx.is_conversion_function_name(member_name)) {
    const std::string canonical_name =
        semantic_lookup::canonical_function_lookup_name(member_name);
    std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
        info.methods.find(canonical_name);
    if(found != info.methods.end()) {
      FunctionBinding * signature_match = nullptr;
      for(std::size_t i = 0; i < found->second.size(); ++i) {
        FunctionBinding * candidate = found->second[i];
        if(!candidate ||
           candidate->owner_class != &info ||
           candidate->ref_qualifier != ref_qualifier ||
           !callsemantic::function_types_equivalent_for_member_signature(
               candidate->type, expected_type)) {
          continue;
        }
        if(signature_match && signature_match != candidate) {
          signature_match = nullptr;
          break;
        }
        signature_match = candidate;
      }
      out = signature_match;
    }
  }
  return out && out->owner_class == &info;
}

bool resolve_stored_out_of_class_special_member_binding_in_target(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::string & member_name,
    const std::vector<std::pair<std::string, TypePtr> > & params,
    FunctionBinding *& out)
{
  out = nullptr;
  if(!info.member_scope) {
    return false;
  }

  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  for(std::size_t i = 0; i < params.size(); ++i) {
    TypePtr param_type = params[i].second;
    TypePtr resolved_param_type;
    if(param_type &&
       recover_instantiation_bound_type(ctx,
                                        *info.member_scope,
                                        param_type,
                                        resolved_param_type) &&
       resolved_param_type) {
      param_type = resolved_param_type;
    }
    effective_params.push_back(param_type);
  }

  TypePtr expected_type =
      make_function(make_fundamental(FT_VOID), effective_params, false);
  out = ctx.find_exact_class_function(info, member_name, expected_type);
  if(!out) {
    out = ctx.find_equivalent_class_function(info, member_name, expected_type);
  }
  return out && out->owner_class == &info;
}

bool source_special_member_matches_selected_specialization(
    const FunctionBinding & source_binding,
    const ClassInfo & info)
{
  if(!info.template_output_node || !source_binding.owner_class) {
    return true;
  }

  const ClassInfo * owner_info = source_binding.owner_class;
  return owner_info->class_node == info.template_output_node ||
         owner_info->template_output_node == info.template_output_node;
}

ClassInfo * lookup_declared_owner_class_via_leaf_type_lookup(SemanticContext & ctx,
                                                             Scope & scope,
                                                             const std::string & text)
{
  TypePtr type = ctx.lookup_non_template_type_name(scope, text);
  return type ? ctx.class_info_for_type(type) : nullptr;
}

ClassInfo * class_template_dependent_reference_source_owner(
    const std::map<std::string, ClassInfo *> & instantiations,
    ClassTemplateDecl & decl)
{
  ClassInfo * fallback = nullptr;
  for(auto it =
          instantiations.begin();
      it != instantiations.end();
      ++it) {
    ClassInfo * candidate = it->second;
    if(!candidate ||
       candidate->source_template != &decl ||
       !candidate->member_scope ||
       candidate->template_output_node != decl.class_node) {
      continue;
    }
    if(candidate->dependent_instantiation) {
      return candidate;
    }
    if(!fallback) {
      fallback = candidate;
    }
  }
  return fallback;
}

ClassInfo * class_template_dependent_reference_source_owner(
    ClassTemplateDecl & decl)
{
  if(ClassInfo * reference =
         class_template_dependent_reference_source_owner(
             decl.reference_instantiations, decl)) {
    return reference;
  }
  return class_template_dependent_reference_source_owner(decl.instantiations,
                                                        decl);
}

void refresh_definition_parameter_names(
    FunctionBinding & binding,
    const std::vector<std::pair<std::string, TypePtr> > & params);

void record_definition_parameter_aliases(
    FunctionBinding & binding,
    const std::vector<std::pair<std::string, TypePtr> > & params);

bool member_function_template_decl_equivalent(FunctionTemplateDecl * lhs,
                                              FunctionTemplateDecl * rhs);

bool member_function_template_decl_same_source_location(SemanticContext & ctx,
                                                        FunctionTemplateDecl * lhs,
                                                        FunctionTemplateDecl * rhs);

bool member_function_template_decl_semantic_signature_matches(
    FunctionTemplateDecl * lhs,
    FunctionTemplateDecl * rhs);

std::string strip_at_location_prefix(const std::string & location)
{
  if(location.compare(0, 4, " at ") == 0) {
    return location.substr(4);
  }
  return location;
}

std::string nested_member_class_decl_location(SemanticContext & ctx,
                                              const CppAstNode * class_node,
                                              const std::string & name)
{
  if(!class_node) {
    return std::string();
  }
  const std::string named =
      strip_at_location_prefix(ctx.source_location_for_name_in_node(*class_node, name));
  if(!named.empty()) {
    return named;
  }
  return strip_at_location_prefix(ctx.source_location_for_node(*class_node));
}

bool expand_instantiated_function_parameter_clause(
    SemanticContext & ctx,
    Scope & inst_scope,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_args);

TypePtr instantiate_stored_member_declared_type(
    SemanticContext & ctx,
    Scope & inst_scope,
    const OutOfClassMemberFunctionDecl & stored,
    std::vector<std::pair<std::string, TypePtr> > & instantiated_params)
{
  instantiated_params = stored.params;

  if(stored.declarator) {
    if(const CppAstNode * parameter_clause =
           cpp_decl::find_child(*stored.declarator, CppAstKind::parameter_clause)) {
      std::vector<const CppAstNode *> unused_default_args;
      std::vector<std::pair<std::string, TypePtr> > expanded_params;
      if(expand_instantiated_function_parameter_clause(ctx,
                                                       inst_scope,
                                                       *parameter_clause,
                                                       expanded_params,
                                                       unused_default_args)) {
        instantiated_params.swap(expanded_params);
      }
    }
  }

  TypePtr declared_type = stored.declared_type_pattern;
  if(!declared_type && stored.specifiers && stored.declarator) {
    semantic_class_model::PreparedMethodParseContext prepared_method;
    semantic_class_model::prepare_method_parse_context(stored.specifiers,
                                                       *stored.declarator,
                                                       prepared_method,
                                                       true,
                                                       true);
    const CppAstNode * parse_specifiers = prepared_method.parse_specifiers_node();
    if(parse_specifiers) {
      bool is_typedef = false;
      TypePtr base;
      const bool parsed_base =
          stored.body ?
              ctx.parse_function_definition_base(inst_scope,
                                                 *parse_specifiers,
                                                 prepared_method.parse_declarator_node(),
                                                 *stored.body,
                                                 prepared_method.syntax.is_const_method,
                                                 prepared_method.syntax.is_volatile_method,
                                                 is_typedef,
                                                 base,
                                                 true) :
              ctx.parse_trailing_return_base(inst_scope,
                                             *parse_specifiers,
                                             prepared_method.parse_declarator_node(),
                                             is_typedef,
                                             base,
                                             true);
      std::string parsed_name;
      TypePtr parsed_type;
      if(parsed_base &&
         !is_typedef &&
         ctx.parse_declarator(inst_scope,
                              ctx.filter_function_declarator(
                                  prepared_method.parse_declarator_node()),
                              base,
                              parsed_name,
                              parsed_type,
                              true) &&
         parsed_type) {
        declared_type = parsed_type;
      }
    }
  }
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_api::TemplateEnvironmentHandle inst_env =
            template_api::make_template_environment(inst_scope);
        for(std::size_t i = 0; i < instantiated_params.size(); ++i) {
          TypePtr resolved_param_type;
          if(instantiated_params[i].second &&
             recover_instantiation_bound_type(
                 services,
                 inst_env,
                 instantiated_params[i].second,
                 resolved_param_type) &&
             resolved_param_type) {
            instantiated_params[i].second = resolved_param_type;
          }
        }

        TypePtr instantiated_type;
        if(declared_type &&
           recover_instantiation_bound_type(
               services,
               inst_env,
               stored.declared_type_pattern,
               instantiated_type) &&
           instantiated_type) {
          declared_type = instantiated_type;
        }

        TypePtr function_type =
            strip_top_level_cv(stored.declared_type_pattern ? stored.declared_type_pattern :
                                                           declared_type);
        if(function_type && function_type->kind == Type::TK_FUNCTION) {
          TypePtr result_type = function_type->inner;
          TypePtr instantiated_result_type;
          if(result_type &&
             recover_instantiation_bound_type(
                 services,
                 inst_env,
                 result_type,
                 instantiated_result_type) &&
             instantiated_result_type) {
            result_type = instantiated_result_type;
          }
          if(result_type) {
            std::vector<TypePtr> param_types;
            param_types.reserve(instantiated_params.size());
            for(std::size_t i = 0; i < instantiated_params.size(); ++i) {
              param_types.push_back(instantiated_params[i].second);
            }
            declared_type = make_function(result_type,
                                          param_types,
                                          function_type->variadic,
                                          function_type->function_const,
                                          function_type->function_volatile,
                                          function_type->prototype_relaxed,
                                          function_type->function_ref_qualifier);
          }
        }
      }
  );

  return declared_type;
}

void apply_definition_abi_metadata_to_binding(
    SemanticContext & ctx,
    FunctionBinding & binding,
    const OutOfClassMemberFunctionDecl & stored)
{
  if(!stored.body || binding.has_definition) {
    return;
  }
  binding.definition_abi_tags.clear();
  append_function_specifier_and_declarator_abi_tags(binding.definition_abi_tags,
                                                    stored.specifiers,
                                                    stored.declarator);
  binding.definition_suppresses_declaration_abi_tags =
      binding.definition_abi_tags.empty();
  ctx.upgrade_function_symbol_linkage(&binding, binding.symbol.linkage);
}

void apply_stored_out_of_class_member_function_abi_metadata_map(
    SemanticContext & ctx,
    const std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> > & stored_definitions,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    bool bind_stored_parameters)
{
  for(std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> >::const_iterator it =
          stored_definitions.begin();
      it != stored_definitions.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      const OutOfClassMemberFunctionDecl & stored = it->second[i];
      if(!stored.body ||
         !stored_member_definition_matches_target_class(ctx, stored, info)) {
        continue;
      }
      Scope & binding_scope = ctx.append_template_scope(*info.member_scope);
      if(bind_stored_parameters) {
        bind_template_arguments_into_scope(ctx, binding_scope, stored.parameters, arguments);
      }

      const bool needs_instantiated_signature =
          (!stored.declared_type_pattern && stored.specifiers && stored.declarator) ||
          (stored.declared_type_pattern &&
           template_argument_semantics::type_depends_on_template_parameter(ctx, stored.declared_type_pattern)) ||
          [&]() -> bool
          {
            for(std::size_t i = 0; i < stored.params.size(); ++i) {
              if(stored.params[i].second &&
                 template_argument_semantics::type_depends_on_template_parameter(ctx, stored.params[i].second)) {
                return true;
              }
            }
            return false;
          }();
      std::vector<std::pair<std::string, TypePtr> > instantiated_params;
      TypePtr declared_type = stored.declared_type_pattern;
      if(needs_instantiated_signature) {
        declared_type =
            instantiate_stored_member_declared_type(ctx,
                                                    binding_scope,
                                                    stored,
                                                    instantiated_params);
      } else {
        TypePtr instantiated_type;
        if(declared_type &&
           recover_instantiation_bound_type(ctx,
                                            binding_scope,
                                            stored.declared_type_pattern,
                                            instantiated_type) &&
           instantiated_type) {
          declared_type = instantiated_type;
        }
      }

      QualifiedName effective_qualified_name_syntax = stored.qualified_name_syntax;
      const CppAstNode * stored_operator_identifier =
          stored.declarator ? find_qualified_operator_identifier(*stored.declarator) : nullptr;
      std::string qualified_operator_name =
          stored_operator_identifier ? stored_operator_identifier->value : std::string();
      if(qualified_operator_name.empty() &&
         stored.qualified_name.find("operator") != std::string::npos) {
        qualified_operator_name = stored.qualified_name;
      }
      if(!qualified_operator_name.empty()) {
        QualifiedName parsed_operator_name;
        if(semantic_utils::split_qualified_name_text(qualified_operator_name,
                                                     parsed_operator_name) &&
           !parsed_operator_name.qualifiers.empty() &&
           parsed_operator_name.name.find("operator") != std::string::npos) {
          effective_qualified_name_syntax = parsed_operator_name;
        }
      }
      std::vector<std::pair<std::string, TypePtr> > effective_params =
          needs_instantiated_signature ? instantiated_params : stored.params;
      rebind_out_of_class_member_signature_types(ctx,
                                                 effective_qualified_name_syntax,
                                                 info,
                                                 declared_type,
                                                 effective_params);

      const bool stored_declarator_spells_operator =
          stored_operator_identifier != nullptr ||
          stored.qualified_name.find("operator") != std::string::npos;
      const bool stored_name_lost_operator =
          stored_declarator_spells_operator &&
          it->first.find("operator") == std::string::npos &&
          effective_qualified_name_syntax.name.find("operator") == std::string::npos &&
          stored.qualified_name.find("operator") == std::string::npos;
      if(stored_name_lost_operator) {
        continue;
      }

      FunctionBinding * binding = nullptr;
      const bool is_conversion_operator =
          ctx.is_conversion_function_name(effective_qualified_name_syntax.name);
      const bool is_special_member =
          qualified_name_is_special_member(effective_qualified_name_syntax) &&
          !is_conversion_operator;
      bool found =
          is_special_member ?
              resolve_stored_out_of_class_special_member_binding_in_target(
                  ctx,
                  info,
                  effective_qualified_name_syntax.name,
                  effective_params,
                  binding) :
              (declared_type &&
               resolve_stored_out_of_class_method_binding_in_target(
                   ctx,
                   info,
                   effective_qualified_name_syntax.name,
                   declared_type,
                   stored.is_const_method,
                   stored.is_volatile_method,
                   stored.ref_qualifier,
                   binding));
      if(!found || !binding) {
        found =
            is_special_member ?
                ctx.resolve_out_of_class_special_member_binding(binding_scope,
                                                                effective_qualified_name_syntax,
                                                                effective_params,
                                                                binding) :
                (declared_type &&
                 ctx.resolve_out_of_class_method_binding(binding_scope,
                                                         effective_qualified_name_syntax,
                                                         declared_type,
                                                         stored.is_const_method,
                                                         stored.is_volatile_method,
                                                         stored.ref_qualifier,
                                                         binding));
      }
      if(found && binding) {
        apply_definition_abi_metadata_to_binding(ctx, *binding, stored);
      }
    }
  }
}

void apply_stored_out_of_class_member_function_definitions_map(
    SemanticContext & ctx,
    const std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> > & stored_definitions,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    bool selected_specialization,
    bool bind_stored_parameters)
{
  for(std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> >::const_iterator it =
          stored_definitions.begin();
      it != stored_definitions.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      const OutOfClassMemberFunctionDecl & stored = it->second[i];
      if(!stored_member_definition_matches_target_class(ctx, stored, info)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "apply-out-of-class-member-function class=" << info.qualified_name
                << " member=" << it->first
                << " qualified-name=" << stored.qualified_name
                << " skipped=owner-mismatch";
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }
      Scope & binding_scope = ctx.append_template_scope(*info.member_scope);
      if(bind_stored_parameters) {
        bind_template_arguments_into_scope(ctx, binding_scope, stored.parameters, arguments);
      }

      const bool needs_instantiated_signature =
          (!stored.declared_type_pattern && stored.specifiers && stored.declarator) ||
          (stored.declared_type_pattern &&
           template_argument_semantics::type_depends_on_template_parameter(ctx, stored.declared_type_pattern)) ||
          [&]() -> bool
          {
            for(std::size_t i = 0; i < stored.params.size(); ++i) {
              if(stored.params[i].second &&
                 template_argument_semantics::type_depends_on_template_parameter(ctx, stored.params[i].second)) {
                return true;
              }
            }
            return false;
          }();
      std::vector<std::pair<std::string, TypePtr> > instantiated_params;
      TypePtr declared_type = stored.declared_type_pattern;
      if(needs_instantiated_signature) {
        declared_type =
            instantiate_stored_member_declared_type(ctx,
                                                    binding_scope,
                                                    stored,
                                                    instantiated_params);
      } else {
        TypePtr instantiated_type;
        if(declared_type &&
           recover_instantiation_bound_type(ctx,
                                            binding_scope,
                                            stored.declared_type_pattern,
                                            instantiated_type) &&
           instantiated_type) {
          declared_type = instantiated_type;
        }
      }
      QualifiedName effective_qualified_name_syntax = stored.qualified_name_syntax;
      const CppAstNode * stored_operator_identifier =
          stored.declarator ? find_qualified_operator_identifier(*stored.declarator) : nullptr;
      std::string qualified_operator_name =
          stored_operator_identifier ? stored_operator_identifier->value : std::string();
      if(qualified_operator_name.empty() &&
         stored.qualified_name.find("operator") != std::string::npos) {
        qualified_operator_name = stored.qualified_name;
      }
      if(!qualified_operator_name.empty()) {
        QualifiedName parsed_operator_name;
        if(semantic_utils::split_qualified_name_text(qualified_operator_name,
                                                     parsed_operator_name) &&
           !parsed_operator_name.qualifiers.empty() &&
           parsed_operator_name.name.find("operator") != std::string::npos) {
          effective_qualified_name_syntax = parsed_operator_name;
        }
      }
      std::vector<std::pair<std::string, TypePtr> > effective_params =
          needs_instantiated_signature ? instantiated_params : stored.params;
      rebind_out_of_class_member_signature_types(ctx,
                                                 effective_qualified_name_syntax,
                                                 info,
                                                 declared_type,
                                                 effective_params);

      const bool stored_declarator_spells_operator =
          stored_operator_identifier != nullptr ||
          stored.qualified_name.find("operator") != std::string::npos;
      const bool stored_name_lost_operator =
          stored_declarator_spells_operator &&
          it->first.find("operator") == std::string::npos &&
          effective_qualified_name_syntax.name.find("operator") == std::string::npos &&
          stored.qualified_name.find("operator") == std::string::npos;
      if(stored_name_lost_operator) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "apply-out-of-class-member-function class=" << info.qualified_name
                << " member=" << it->first
                << " qualified-name=" << stored.qualified_name
                << " skipped=operator-name-lost";
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }

      FunctionBinding * binding = nullptr;
      const bool is_conversion_operator =
          ctx.is_conversion_function_name(effective_qualified_name_syntax.name);
      const bool is_special_member =
          qualified_name_is_special_member(effective_qualified_name_syntax) &&
          !is_conversion_operator;
      bool found =
          is_special_member ?
              resolve_stored_out_of_class_special_member_binding_in_target(
                  ctx,
                  info,
                  effective_qualified_name_syntax.name,
                  effective_params,
                  binding) :
              (declared_type &&
               resolve_stored_out_of_class_method_binding_in_target(
                   ctx,
                   info,
                   effective_qualified_name_syntax.name,
                   declared_type,
                   stored.is_const_method,
                   stored.is_volatile_method,
                   stored.ref_qualifier,
                   binding));
      if(!found || !binding) {
        found =
            is_special_member ?
                ctx.resolve_out_of_class_special_member_binding(binding_scope,
                                                                effective_qualified_name_syntax,
                                                                effective_params,
                                                                binding) :
                (declared_type &&
                 ctx.resolve_out_of_class_method_binding(binding_scope,
                                                         effective_qualified_name_syntax,
                                                         declared_type,
                                                         stored.is_const_method,
                                                         stored.is_volatile_method,
                                                         stored.ref_qualifier,
                                                         binding));
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "apply-out-of-class-member-function class=" << info.qualified_name
              << " member=" << it->first
              << " qualified-name=" << stored.qualified_name
              << " special-member=" << (is_special_member ? "yes" : "no")
              << " found=" << (found && binding ? "yes" : "no")
              << " definition=" << (stored.body ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!found || !binding) {
        continue;
      }
      apply_definition_abi_metadata_to_binding(ctx, *binding, stored);

      if(stored.body) {
        if(!out_of_class_definition_exception_spec_matches(ctx,
                                                           *binding,
                                                           stored.declarator,
                                                           stored.body)) {
          const CppAstNode * replay_qualifier =
              stored.declarator ?
                  semantic_class_model::declarator_function_qualifier(*stored.declarator) :
                  nullptr;
          throw std::logic_error(
              std::string("mismatched function exception specification [ti-replay]") +
              " [member " + it->first + "]" +
              " [stored " + stored.qualified_name + "]" +
              " [effective " + effective_qualified_name_syntax.name + "]" +
              " [operator " + qualified_operator_name + "]" +
              " [has-declarator " + std::string(stored.declarator ? "yes" : "no") + "]" +
              " [body-kind " + std::to_string(stored.body ? static_cast<int>(stored.body->kind) : -1) + "]" +
              " [binding-qual " + (binding->function_qualifier ?
                                      binding->function_qualifier->value :
                                      std::string("<none>")) + "]" +
              " [replay-qual " + (replay_qualifier ?
                                     replay_qualifier->value :
                                     std::string("<none>")) + "]" +
              semantic_trace::current_location_note(ctx, stored.declarator) +
              semantic_trace::previous_function_location_note(
                  ctx, "previous declaration", binding));
        }
        const bool same_definition_location =
            binding->definition_node &&
            stored.declarator &&
            ctx.source_location_for_node(*binding->definition_node) ==
                ctx.source_location_for_node(*stored.declarator);
        const bool same_body_location =
            binding->body &&
            stored.body &&
            ctx.source_location_for_node(*binding->body) ==
                ctx.source_location_for_node(*stored.body);
        if(binding->has_definition &&
           binding->definition_node &&
           binding->definition_node != stored.declarator) {
          if(selected_specialization || same_definition_location || same_body_location) {
            continue;
          }
          throw std::logic_error(
              std::string("duplicate class member definition") +
              semantic_trace::current_location_note(ctx, stored.declarator) +
              semantic_trace::previous_function_location_note(
                  ctx, "previous definition", binding));
        }
        refresh_definition_parameter_names(*binding, effective_params);
        binding->body = stored.body;
        record_definition_parameter_aliases(*binding, effective_params);
        binding->ctor_initializer = stored.ctor_initializer;
        binding->has_definition = true;
        binding->definition_node = stored.declarator;
        binding->definition_abi_tags.clear();
        append_function_specifier_and_declarator_abi_tags(binding->definition_abi_tags,
                                                          stored.specifiers,
                                                          stored.declarator);
        if(!binding->function_qualifier && stored.declarator) {
          binding->function_qualifier =
              semantic_class_model::declarator_function_qualifier(*stored.declarator);
        }
        binding->exclude_from_explicit_instantiation =
            binding->exclude_from_explicit_instantiation ||
            stored.exclude_from_explicit_instantiation;
        ctx.upgrade_function_symbol_linkage(binding, binding->symbol.linkage);
      } else if(!binding->declaration_node) {
        binding->declaration_node = stored.declarator;
        append_function_specifier_and_declarator_abi_tags(binding->declaration_abi_tags,
                                                          stored.specifiers,
                                                          stored.declarator);
      }
    }
  }
}

void maybe_apply_stored_out_of_class_member_function_template_definition(
    SemanticContext & ctx,
    const std::vector<OutOfClassMemberFunctionTemplateDefinition> & stored_definitions,
    FunctionTemplateDecl & candidate)
{
  if(candidate.body) {
    return;
  }
  for(std::size_t j = 0; j < stored_definitions.size(); ++j) {
    const OutOfClassMemberFunctionTemplateDefinition & stored_def =
        stored_definitions[j];
    const bool equivalent =
        stored_def.declaration &&
        member_function_template_decl_equivalent(stored_def.declaration, &candidate);
    const bool same_source =
        stored_def.declaration &&
        member_function_template_decl_same_source_location(ctx,
                                                           stored_def.declaration,
                                                           &candidate);
    const bool semantic_match =
        stored_def.declaration &&
        member_function_template_decl_semantic_signature_matches(
            stored_def.declaration,
            &candidate);
    if(!stored_def.declaration ||
       !stored_def.body ||
       (!equivalent && !same_source && !semantic_match)) {
      continue;
    }
    if(candidate.params_pattern.size() == stored_def.declaration->params_pattern.size()) {
      for(std::size_t param_index = 0;
          param_index < candidate.params_pattern.size();
          ++param_index) {
        const std::string generated_name = std::string("arg") + std::to_string(param_index);
        const bool candidate_unnamed =
            candidate.params_pattern[param_index].first.empty() ||
            candidate.params_pattern[param_index].first == generated_name ||
            candidate.params_pattern[param_index].first.compare(0, 7, "__param") == 0;
        if(candidate_unnamed &&
           !stored_def.declaration->params_pattern[param_index].first.empty()) {
          candidate.params_pattern[param_index].first =
              stored_def.declaration->params_pattern[param_index].first;
        }
      }
    }
    ensure_function_template_parameter_aliases(candidate);
    for(std::size_t param_index = 0;
        param_index < candidate.params_pattern.size();
        ++param_index) {
      std::string alias_name;
      if(param_index < stored_def.parameter_aliases_pattern.size() &&
         !stored_def.parameter_aliases_pattern[param_index].empty()) {
        alias_name = stored_def.parameter_aliases_pattern[param_index];
      } else {
        alias_name =
            function_template_parameter_alias_name(*stored_def.declaration, param_index);
      }
      if(alias_name.empty()) {
        alias_name = candidate.params_pattern[param_index].first;
      }
      if(param_index < candidate.parameter_aliases_pattern.size()) {
        candidate.parameter_aliases_pattern[param_index] = alias_name;
      }
    }
    candidate.body = stored_def.body;
    candidate.ctor_initializer = stored_def.ctor_initializer;
    candidate.definition_node = stored_def.definition_node;
    candidate.definition_inner = stored_def.definition_node;
    candidate.definition_specifiers = stored_def.definition_specifiers;
    candidate.definition_declarator = stored_def.definition_declarator;
    break;
  }
}

void apply_stored_out_of_class_member_function_template_definitions_map(
    SemanticContext & ctx,
    const std::map<std::string,
                   std::vector<OutOfClassMemberFunctionTemplateDefinition> > & stored_definitions,
    ClassInfo & info)
{
  for(std::map<std::string,
               std::vector<OutOfClassMemberFunctionTemplateDefinition> >::const_iterator stored =
          stored_definitions.begin();
      stored != stored_definitions.end();
      ++stored) {
    std::map<std::string, std::vector<FunctionTemplateDecl *> >::iterator slot =
        info.member_scope->function_templates.find(stored->first);
    if(slot == info.member_scope->function_templates.end()) {
      continue;
    }
    for(std::size_t i = 0; i < slot->second.size(); ++i) {
      FunctionTemplateDecl * candidate = slot->second[i];
      if(!candidate) {
        continue;
      }
      maybe_apply_stored_out_of_class_member_function_template_definition(ctx,
                                                                          stored->second,
                                                                          *candidate);
    }
  }
}

bool text_contains_name_reference(const std::string & text, const std::string & needle)
{
  if(needle.empty()) {
    return false;
  }
  std::size_t pos = text.find(needle);
  while(pos != std::string::npos) {
    const bool left_ok =
        pos == 0 || !is_identifier_char(static_cast<unsigned char>(text[pos - 1]));
    const std::size_t end = pos + needle.size();
    const bool right_ok =
        end >= text.size() || !is_identifier_char(static_cast<unsigned char>(text[end]));
    if(left_ok && right_ok) {
      return true;
    }
    pos = text.find(needle, pos + 1);
  }
  return false;
}

struct TemplateArgumentNameReferences
{
  const std::vector<TemplateArgument> * arguments = nullptr;
  std::set<std::string> identifiers;
  bool contains_function_local_marker = false;

  bool mentions(const std::string & name) const
  {
    if(name.empty()) {
      return false;
    }
    if(name.find("::") == std::string::npos) {
      return identifiers.count(name) != 0;
    }
    if(!arguments) {
      return false;
    }
    for(std::size_t i = 0; i < arguments->size(); ++i) {
      const TemplateArgument & argument = (*arguments)[i];
      if(argument.kind == TemplateArgument::TA_TYPE &&
         text_contains_name_reference(argument.text, name)) {
        return true;
      }
    }
    return false;
  }
};

TemplateArgumentNameReferences collect_template_argument_name_references(
    const std::vector<TemplateArgument> & arguments)
{
  TemplateArgumentNameReferences refs;
  refs.arguments = &arguments;
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind != TemplateArgument::TA_TYPE ||
       arguments[i].text.empty()) {
      continue;
    }
    const std::string & text = arguments[i].text;
    if(text.find("__local_") != std::string::npos) {
      refs.contains_function_local_marker = true;
    }
    for(std::size_t pos = 0; pos < text.size();) {
      const unsigned char ch = static_cast<unsigned char>(text[pos]);
      if(!is_identifier_char(ch)) {
        ++pos;
        continue;
      }

      std::size_t end = pos + 1;
      while(end < text.size() &&
            is_identifier_char(static_cast<unsigned char>(text[end]))) {
        ++end;
      }

      const std::string token = text.substr(pos, end - pos);
      if(!is_leading_type_keyword(token)) {
        refs.identifiers.insert(token);
      }
      pos = end;
    }
  }
  return refs;
}

bool parameter_declaration_has_pack(const CppAstNode & parameter)
{
  const CppAstNode * declarator = find_child(parameter, CppAstKind::declarator);
  if(!declarator) {
    declarator = find_child(parameter, CppAstKind::abstract_declarator);
  }
  return declarator && find_child(*declarator, CppAstKind::parameter_pack);
}

bool ast_node_mentions_identifier_token(const CppAstNode & node,
                                        const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  if(node.kind == CppAstKind::identifier && node.value == name) {
    return true;
  }
  if(!node.value.empty() &&
     callsemantic_internal::contains_identifier_token(node.value, name)) {
    return true;
  }
  if(node.template_id_syntax) {
    const TemplateIdSyntax & syntax = *node.template_id_syntax;
    if(syntax.name.name == name) {
      return true;
    }
    for(std::size_t i = 0; i < syntax.name.qualifiers.size(); ++i) {
      if(syntax.name.qualifiers[i] == name) {
        return true;
      }
    }
    for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
      if(callsemantic_internal::contains_identifier_token(syntax.arguments[i], name)) {
        return true;
      }
    }
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    const TemplateIdSyntax & syntax = node.qualifier_template_id_syntaxes[i];
    if(syntax.name.name == name) {
      return true;
    }
    for(std::size_t j = 0; j < syntax.name.qualifiers.size(); ++j) {
      if(syntax.name.qualifiers[j] == name) {
        return true;
      }
    }
    for(std::size_t j = 0; j < syntax.arguments.size(); ++j) {
      if(callsemantic_internal::contains_identifier_token(syntax.arguments[j], name)) {
        return true;
      }
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_mentions_identifier_token(node.qualifier_type_syntaxes[i], name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_mentions_identifier_token(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

const CppAstNode * function_template_parameter_clause(const FunctionTemplateDecl & decl)
{
  return decl.declarator ? find_child_kind(*decl.declarator, CppAstKind::parameter_clause) :
                           nullptr;
}

void bind_instantiated_function_parameter_values(
    SemanticContext & ctx,
    Scope & inst_scope,
    const FunctionTemplateDecl & source_decl,
    const std::vector<std::pair<std::string, TypePtr> > & params)
{
  for(std::size_t i = 0; i < params.size(); ++i) {
    template_scope::bind_parameter_value(inst_scope, params[i].first, params[i].second);
  }

  const CppAstNode * parameter_clause = function_template_parameter_clause(source_decl);
  if(!parameter_clause) {
    return;
  }

  std::vector<const CppAstNode *> parameters;
  for(std::size_t i = 0; i < parameter_clause->children.size(); ++i) {
    if(parameter_clause->children[i].kind == CppAstKind::parameter_declaration) {
      parameters.push_back(&parameter_clause->children[i]);
    }
  }
  if(parameters.empty()) {
    return;
  }

  std::size_t param_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameter_declaration_has_pack(*parameters[i])) {
      if(param_index < params.size()) {
        ++param_index;
      }
      continue;
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(*parameters[i]);
    if(pack_name.empty()) {
      return;
    }

    std::size_t pack_size = 0;
    if(!pack_parameter_analysis::infer_named_type_pack_size(inst_scope,
                                                            *parameters[i],
                                                            pack_size)) {
      std::size_t remaining_fixed_params = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameter_declaration_has_pack(*parameters[j])) {
          ++remaining_fixed_params;
        }
      }
      if(params.size() < param_index + remaining_fixed_params) {
        return;
      }
      pack_size = params.size() - param_index - remaining_fixed_params;
    }

    if(params.size() < param_index + pack_size) {
      return;
    }

    std::vector<TypePtr> pack_value_types;
    pack_value_types.reserve(pack_size);
    for(std::size_t j = 0; j < pack_size; ++j) {
      pack_value_types.push_back(params[param_index + j].second);
    }
    template_scope::bind_parameter_value_pack(inst_scope, pack_name, pack_value_types);
    param_index += pack_size;
  }
}

std::string template_argument_text_for_diagnostic(SemanticContext & ctx,
                                                  const TemplateArgument & argument)
{
  return template_model::template_argument_text(
      argument,
      [&ctx](const TypePtr & type)
      {
        return instantiation_argument_type_text(ctx, type);
      });
}

std::string template_argument_text_for_diagnostic(template_api::TemplateTypeSystem & type_system,
                                                  const TemplateArgument & argument)
{
  return template_model::template_argument_text(
      argument,
      [&type_system](const TypePtr & type)
      {
        return instantiation_argument_type_text(type_system, type);
      });
}

std::string template_argument_key_for_instantiation_impl(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments)
{
  std::string out;
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(i != 0) {
      out += "|";
    }
    if(arguments[i].kind == TemplateArgument::TA_TYPE && arguments[i].type) {
      out += ctx.instantiation_identity_text_for_type_argument(arguments[i].type);
      continue;
    }
    out += template_model::template_argument_text(
        arguments[i],
        [&ctx](const TypePtr & type)
        {
          return ctx.instantiation_identity_text_for_type_argument(type);
        });
  }
  return out;
}

// template-boundary-audit: begin canonical_key_metadata
std::string class_instantiation_key_for_metadata(
    SemanticContext & ctx,
    const ClassInfo & info)
{
  if(!info.instantiation_key.empty()) {
    return info.instantiation_key;
  }
  return template_argument_key_for_instantiation_impl(ctx, info.instantiation_arguments);
}

std::string canonical_instantiation_arg_text_impl(
    SemanticContext & ctx,
    const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type) {
    return ctx.instantiation_identity_text_for_type_argument(argument.type);
  }
  return template_model::template_argument_text(
      argument,
      [&ctx](const TypePtr & type)
      {
        return instantiation_argument_type_text(ctx, type);
      });
}

std::vector<std::string> canonical_instantiation_arg_texts_impl(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments)
{
  std::vector<std::string> out;
  out.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    out.push_back(canonical_instantiation_arg_text_impl(ctx, arguments[i]));
  }
  return out;
}

bool dependent_argument_source_text_matches_semantic_argument(
    SemanticContext & ctx,
    const TemplateArgument & argument,
    const std::string & source_text)
{
  const std::string trimmed_source = semantic_utils::trim_space(source_text);
  if(argument.kind != TemplateArgument::TA_TYPE ||
     !argument.type ||
     !named_type_is_template_parameter(argument.type) ||
     trimmed_source.empty() ||
     !callsemantic_internal::is_identifier_text(trimmed_source)) {
    return true;
  }
  const std::string canonical =
      ctx.instantiation_identity_text_for_type_argument(argument.type);
  return canonical.empty() ||
         template_argument_semantics::normalized_type_lookup_text_matches(
             source_text,
             canonical);
}

bool template_arguments_are_dependent_for_instantiation(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments);

std::string template_specialization_name_from_argument_texts(
    const std::string & name,
    const std::vector<std::string> & argument_texts)
{
  std::string out = name;
  out += "<";
  for(std::size_t i = 0; i < argument_texts.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += argument_texts[i];
  }
  out += ">";
  return callsemantic_internal::normalize_qualified_name_spacing(out);
}

void update_class_template_dependent_type_display_name(ClassInfo & info)
{
  TypePtr named = strip_top_level_cv(info.type);
  if(!named ||
     named->kind != Type::TK_NAMED ||
     !info.source_template ||
     !info.source_template->declaring_scope ||
     info.instantiation_arg_texts.empty()) {
    return;
  }
  if(info.source_template->parameters.size() != 1 ||
     info.source_template->parameters[0].kind != TemplateParameterInfo::TP_TYPE ||
     info.source_template->parameters[0].parameter_pack ||
     info.instantiation_arguments.size() != 1 ||
     info.instantiation_arguments[0].kind != TemplateArgument::TA_TYPE ||
     !named_type_is_template_parameter(info.instantiation_arguments[0].type)) {
    return;
  }
  const std::string specialization_name =
      template_specialization_name_from_argument_texts(
          info.source_template->name,
          info.instantiation_arg_texts);
  const std::string display_qualified_name =
      semantic_lookup::scope_qualified_name(*info.source_template->declaring_scope,
                                            specialization_name);
  named->named_display =
      callsemantic_internal::normalize_qualified_name_spacing(
          info.class_kind + " " + display_qualified_name);
}

void update_class_template_dependent_type_metadata(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    bool dependent_arguments,
    const std::vector<TemplateArgumentSyntax> * argument_syntaxes = nullptr)
{
  if(!info.source_template || !dependent_arguments) {
    set_named_type_dependent_class_template(
        info.type,
        nullptr,
        std::vector<DependentAliasTemplateArgumentSyntax>());
    return;
  }

  std::vector<DependentAliasTemplateArgumentSyntax> dependent_argument_syntaxes;
  dependent_argument_syntaxes.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    DependentAliasTemplateArgumentSyntax dependent_argument;
    dependent_argument.text =
        i < info.instantiation_arg_texts.size() ?
            info.instantiation_arg_texts[i] :
            template_model::template_argument_text(
                arguments[i],
                [&ctx](const TypePtr & type)
                {
                  return instantiation_argument_type_text(ctx, type);
                });
    if(arguments[i].kind == TemplateArgument::TA_TYPE) {
      dependent_argument.type = arguments[i].type;
    }
    if(argument_syntaxes && i < argument_syntaxes->size()) {
      dependent_argument.syntax = (*argument_syntaxes)[i];
    } else if(arguments[i].kind != TemplateArgument::TA_TYPE) {
      dependent_argument.syntax.text = dependent_argument.text;
    }
    if(arguments[i].kind == TemplateArgument::TA_TYPE &&
       arguments[i].type &&
       !dependent_argument.syntax.resolved_type) {
      dependent_argument.syntax.resolved_type = arguments[i].type;
      if(dependent_argument.syntax.text.empty()) {
        dependent_argument.syntax.text = dependent_argument.text;
      }
    }
    if(arguments[i].dependent) {
      dependent_argument.syntax.dependent = true;
    }
    if(arguments[i].kind == TemplateArgument::TA_VALUE &&
       arguments[i].expression &&
       !dependent_argument.syntax.expression) {
      dependent_argument.syntax.text = dependent_argument.text;
      dependent_argument.syntax.source_location_id =
          arguments[i].expression->source_location_id;
      dependent_argument.syntax.expression.reset(
          new CppAstNode(*arguments[i].expression));
    }
    dependent_argument.source_defaulted = arguments[i].source_defaulted;
    dependent_argument_syntaxes.push_back(dependent_argument);
  }
  set_named_type_dependent_class_template(
      info.type,
      info.source_template,
      dependent_argument_syntaxes);
  update_class_template_dependent_type_display_name(info);
}

bool type_contains_forced_structured_mangling(const TypePtr & type)
{
  if(!type) {
    return false;
  }
  switch(type->kind) {
  case Type::TK_NAMED:
  {
    std::shared_ptr<const ClassTemplateSpecializationMangleInfo> specialization =
        named_type_class_template_specialization_mangle_info_const(type);
    return specialization && specialization->force_structured_mangling;
  }
  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_contains_forced_structured_mangling(type->inner) ||
           type_contains_forced_structured_mangling(type->owner);
  case Type::TK_FUNCTION:
    if(type_contains_forced_structured_mangling(type->inner)) {
      return true;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(type_contains_forced_structured_mangling(type->params[i])) {
        return true;
      }
    }
    return false;
  case Type::TK_FUNDAMENTAL:
    return false;
  }
  return false;
}

bool template_arguments_contain_forced_structured_mangling(
    const std::vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind == TemplateArgument::TA_TYPE &&
       type_contains_forced_structured_mangling(arguments[i].type)) {
      return true;
    }
  }
  return false;
}

void update_class_template_specialization_mangle_info(
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    bool force_structured_mangling,
    const std::vector<TemplateArgumentSyntax> * argument_syntaxes = nullptr,
    const std::vector<TemplateParameterInfo> * mangle_parameters = nullptr,
    const std::vector<TemplateArgument> * mangle_arguments = nullptr,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr)
{
  if(!info.type || !info.source_template || info.is_lambda_closure) {
    set_named_type_class_template_specialization_mangle_info(
        info.type,
        std::shared_ptr<ClassTemplateSpecializationMangleInfo>());
    return;
  }

  std::shared_ptr<ClassTemplateSpecializationMangleInfo> mangle_info(
      new ClassTemplateSpecializationMangleInfo());
  mangle_info->class_template_decl = info.source_template;
  mangle_info->template_name = info.source_template->name;
  if(info.source_template->declaring_scope) {
    const std::string qualified =
        semantic_lookup::scope_qualified_name(*info.source_template->declaring_scope,
                                              info.source_template->name);
    const std::string suffix = std::string("::") + info.source_template->name;
    if(qualified == info.source_template->name) {
      mangle_info->template_scope_prefix.clear();
    } else if(qualified.size() > suffix.size() &&
              qualified.compare(qualified.size() - suffix.size(),
                                suffix.size(),
                                suffix) == 0) {
      mangle_info->template_scope_prefix =
          qualified.substr(0, qualified.size() - suffix.size());
    }
  }
  mangle_info->template_parameters = info.source_template->parameters;
  if(mangle_parameters) {
    mangle_info->mangle_parameters = *mangle_parameters;
  }
  if(mangle_arguments) {
    mangle_info->mangle_arguments = *mangle_arguments;
  }
  mangle_info->arguments = arguments;
  if(argument_syntaxes) {
    mangle_info->argument_syntaxes = *argument_syntaxes;
  }
  if(pack_sizes) {
    mangle_info->pack_sizes = *pack_sizes;
  }
  mangle_info->force_structured_mangling = force_structured_mangling;
  set_named_type_class_template_specialization_mangle_info(info.type, mangle_info);
}

void clear_class_template_cached_lambda_mangle_metadata(
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments)
{
  if(!info.type || info.is_lambda_closure) {
    return;
  }

  (void)arguments;
  info.type->named_lambda_mangle.reset();
}

void record_class_template_argument_state(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<TemplateParameterInfo> * mangle_parameters = nullptr,
    const std::vector<TemplateArgument> * mangle_arguments = nullptr,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr)
{
  info.instantiation_key = key;
  info.instantiation_arg_texts = canonical_instantiation_arg_texts_impl(ctx, arguments);
  info.instantiation_arguments = arguments;
  info.instantiation_binding_arguments = arguments;
  info.instantiation_binding_pack_sizes.clear();
  info.has_instantiation_binding_arguments = true;
  const bool force_structured_mangling =
      template_arguments_contain_forced_structured_mangling(arguments);
  clear_class_template_cached_lambda_mangle_metadata(info, arguments);
  update_class_template_specialization_mangle_info(
      info,
      arguments,
      force_structured_mangling,
      nullptr,
      mangle_parameters,
      mangle_arguments,
      pack_sizes);
  update_class_template_dependent_type_metadata(
      ctx,
      info,
      arguments,
      template_arguments_are_dependent_for_instantiation(ctx, arguments));
}

void record_class_template_binding_state(
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  info.instantiation_binding_arguments = arguments;
  if(pack_sizes) {
    info.instantiation_binding_pack_sizes = *pack_sizes;
  } else {
    info.instantiation_binding_pack_sizes.clear();
  }
  info.has_instantiation_binding_arguments = true;
}

void attach_function_template_registration_identity(
    FunctionRegistrationRequest & request,
    FunctionTemplateDecl & source_decl,
    const std::vector<TemplateArgument> & arguments,
    const std::string & key,
    bool prefer_overload_suffix)
{
  request.template_identity.decl = &source_decl;
  request.template_identity.arguments = &arguments;
  request.template_identity.arguments_present = true;
  request.template_identity.key = key;
  request.template_identity.prefer_overload_suffix = prefer_overload_suffix;
}

bool function_binding_matches_template_registration_identity(
    const FunctionBinding & binding,
    const FunctionTemplateDecl & source_decl,
    const std::string & key)
{
  return binding.source_template == &source_decl &&
         binding.template_instantiation_key == key;
}

const std::string & function_binding_template_registration_key(
    const FunctionBinding & binding)
{
  return binding.template_instantiation_key;
}

void record_function_template_instantiation_cache_entry(
    FunctionBinding & binding,
    FunctionTemplateDecl & source_decl,
    const std::string & key)
{
  if(!binding.instantiation_cache_entries) {
    binding.instantiation_cache_entries.reset(
        new FunctionTemplateInstantiationCacheEntries());
    binding.instantiation_cache_entries->first.decl = &source_decl;
    binding.instantiation_cache_entries->first.key = key;
    return;
  }
  if(binding.instantiation_cache_entries->first.decl == &source_decl &&
     binding.instantiation_cache_entries->first.key == key) {
    return;
  }
  for(std::size_t i = 0; i < binding.instantiation_cache_entries->extra.size(); ++i) {
    FunctionTemplateInstantiationCacheEntry & entry =
        binding.instantiation_cache_entries->extra[i];
    if(entry.decl == &source_decl && entry.key == key) {
      return;
    }
  }
  FunctionTemplateInstantiationCacheEntry entry;
  entry.decl = &source_decl;
  entry.key = key;
  binding.instantiation_cache_entries->extra.push_back(entry);
}
// template-boundary-audit: end canonical_key_metadata

void record_function_template_argument_state_impl(
    FunctionBinding & binding,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    bool mark_has_arguments)
{
  binding.instantiation_arguments = arguments;
  if(mark_has_arguments) {
    binding.has_instantiation_arguments = true;
  }
  if(pack_sizes) {
    binding.instantiation_pack_sizes = *pack_sizes;
  } else {
    binding.instantiation_pack_sizes.clear();
  }
}

std::vector<std::string> instantiate_function_parameter_aliases(
    const FunctionTemplateDecl & decl,
    const std::vector<std::pair<std::string, TypePtr> > & params)
{
  std::vector<std::string> out;
  out.reserve(params.size());
  for(std::size_t i = 0; i < params.size(); ++i) {
    const std::string alias_name = function_template_parameter_alias_name(decl, i);
    out.push_back(alias_name.empty() ? params[i].first : alias_name);
  }
  return out;
}

void apply_instantiated_parameter_aliases(FunctionBinding & binding,
                                          const std::vector<std::string> & aliases)
{
  ensure_function_parameter_aliases(binding);
  const std::size_t offset = function_binding_explicit_parameter_offset(binding);
  for(std::size_t i = 0; i < binding.params.size(); ++i) {
    if(i < offset) {
      binding.parameter_aliases[i] = binding.params[i].first;
      continue;
    }
    const std::size_t explicit_index = i - offset;
    binding.parameter_aliases[i] =
        explicit_index < aliases.size() && !aliases[explicit_index].empty() ?
            aliases[explicit_index] :
            binding.params[i].first;
  }
}

std::vector<std::pair<std::string, TypePtr> > binding_explicit_params(
    const FunctionBinding & binding)
{
  const std::size_t offset = function_binding_explicit_parameter_offset(binding);
  if(binding.params.size() <= offset) {
    return std::vector<std::pair<std::string, TypePtr> >();
  }
  return std::vector<std::pair<std::string, TypePtr> >(
      binding.params.begin() + offset,
      binding.params.end());
}

void ensure_unique_inherited_constructor_parameter_names(FunctionBinding & binding)
{
  const std::size_t offset = function_binding_explicit_parameter_offset(binding);
  if(binding.params.size() <= offset) {
    return;
  }

  std::map<std::string, std::size_t> counts;
  for(std::size_t i = offset; i < binding.params.size(); ++i) {
    const std::string & name = binding.params[i].first;
    if(!name.empty()) {
      ++counts[name];
    }
  }

  std::vector<bool> rename(binding.params.size(), false);
  std::set<std::string> used;
  for(std::size_t i = offset; i < binding.params.size(); ++i) {
    const std::string & name = binding.params[i].first;
    const bool duplicate = !name.empty() && counts[name] > 1;
    rename[i] = name.empty() || duplicate;
    if(!rename[i]) {
      used.insert(name);
    }
  }

  for(std::size_t i = offset; i < binding.params.size(); ++i) {
    if(!rename[i]) {
      continue;
    }
    const std::string base =
        std::string("__param") + std::to_string(i - offset + 1);
    std::string candidate = base;
    std::size_t suffix = 2;
    while(used.count(candidate) != 0) {
      candidate = base + "_" + std::to_string(suffix++);
    }
    binding.params[i].first = candidate;
    used.insert(candidate);
  }
}

std::string inherited_constructor_base_match_name(const std::string & name)
{
  return semantic_utils::strip_trailing_top_level_template_arguments(
      semantic_utils::unqualified_member_name(name));
}

ClassInfo * find_instantiated_inherited_constructor_base(
    SemanticContext & ctx,
    Scope & inst_scope,
    const FunctionTemplateDecl & source_decl,
    const FunctionBinding & binding)
{
  if(!binding.owner_class || !source_decl.ctor_initializer) {
    return nullptr;
  }
  const CppAstNode & ctor_init = *source_decl.ctor_initializer;
  if(ctor_init.children.empty()) {
    return nullptr;
  }
  const CppAstNode * init_id =
      find_child_kind(ctor_init.children[0], CppAstKind::mem_initializer_id);
  if(!init_id) {
    return nullptr;
  }

  TypePtr target_type = init_id->semantic_type;
  if(target_type) {
    TypePtr resolved_type;
    if(recover_instantiation_bound_type(ctx, inst_scope, target_type, resolved_type) &&
       resolved_type) {
      target_type = resolved_type;
    }
    target_type = strip_top_level_cv(target_type);
  }

  const std::string target_name =
      inherited_constructor_base_match_name(init_id->value);
  for(std::size_t i = 0; i < binding.owner_class->bases.size(); ++i) {
    const BaseInfo & base = binding.owner_class->bases[i];
    if(!base.type || !base.type->type) {
      continue;
    }
    if(target_type &&
       type_equals(strip_top_level_cv(base.type->type), target_type)) {
      return base.type;
    }
    if(target_name.empty()) {
      continue;
    }
    if(target_name == inherited_constructor_base_match_name(base.type->name) ||
       target_name == inherited_constructor_base_match_name(base.type->qualified_name)) {
      return base.type;
    }
  }
  return nullptr;
}

CppAstNode make_instantiated_inherited_constructor_type_id(const TypePtr & type)
{
  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;

  CppAstNode specifiers;
  specifiers.kind = CppAstKind::decl_specifier_seq;

  CppAstNode type_name;
  type_name.kind = CppAstKind::type_name;
  type_name.value = type ? template_argument_type_text(type) : std::string();
  type_name.semantic_type = type;
  specifiers.children.push_back(type_name);
  type_id.children.push_back(specifiers);
  return type_id;
}

CppAstNode make_instantiated_inherited_constructor_rvalue_type_id(
    const TypePtr & type)
{
  CppAstNode type_id = make_instantiated_inherited_constructor_type_id(type);

  CppAstNode abstract;
  abstract.kind = CppAstKind::abstract_declarator;

  CppAstNode ptr_operator;
  ptr_operator.kind = CppAstKind::ptr_operator;
  ptr_operator.value = "&&";
  ptr_operator.has_token = true;
  ptr_operator.token_kind = RT_SIMPLE;
  ptr_operator.simple_type = OP_LAND;
  abstract.children.push_back(ptr_operator);
  type_id.children.push_back(abstract);
  return type_id;
}

CppAstNode make_instantiated_inherited_constructor_argument_expression(
    const std::vector<std::pair<std::string, TypePtr> > & params,
    std::size_t index)
{
  CppAstNode id;
  id.kind = CppAstKind::id_expression;
  id.value =
      index < params.size() && !params[index].first.empty() ?
          params[index].first :
          std::string("__param") + std::to_string(index + 1);

  if(index >= params.size() ||
     !params[index].second ||
     strip_top_level_cv(params[index].second)->kind == Type::TK_LVALUE_REFERENCE) {
    return id;
  }

  CppAstNode cast;
  cast.kind = CppAstKind::cast_expression;
  cast.value = "static_cast";
  cast.has_token = true;
  cast.token_kind = RT_SIMPLE;
  cast.simple_type = KW_STATIC_CAST;
  cast.children.push_back(
      make_instantiated_inherited_constructor_rvalue_type_id(
          remove_reference_type(params[index].second)));
  cast.children.push_back(id);
  return cast;
}

CppAstNode make_instantiated_inherited_constructor_initializer(
    const ClassInfo & base_info,
    const std::vector<std::pair<std::string, TypePtr> > & params)
{
  CppAstNode ctor_initializer;
  ctor_initializer.kind = CppAstKind::ctor_initializer;

  CppAstNode mem_initializer;
  mem_initializer.kind = CppAstKind::mem_initializer;

  CppAstNode mem_initializer_id;
  mem_initializer_id.kind = CppAstKind::mem_initializer_id;
  mem_initializer_id.value = !base_info.qualified_name.empty() ?
      base_info.qualified_name :
      base_info.name;
  mem_initializer_id.semantic_type = base_info.type;
  mem_initializer.children.push_back(mem_initializer_id);

  CppAstNode paren_args;
  paren_args.kind = CppAstKind::paren_argument_list;
  for(std::size_t i = 0; i < params.size(); ++i) {
    paren_args.children.push_back(
        make_instantiated_inherited_constructor_argument_expression(params, i));
  }
  mem_initializer.children.push_back(paren_args);
  ctor_initializer.children.push_back(mem_initializer);
  return ctor_initializer;
}

void refresh_instantiated_inherited_constructor_initializer(
    SemanticContext & ctx,
    Scope & inst_scope,
    const FunctionTemplateDecl & source_decl,
    FunctionBinding & binding)
{
  if(!source_decl.is_inherited_constructor || !binding.is_constructor) {
    return;
  }
  ClassInfo * base =
      find_instantiated_inherited_constructor_base(ctx,
                                                  inst_scope,
                                                  source_decl,
                                                  binding);
  if(!base) {
    return;
  }
  ensure_unique_inherited_constructor_parameter_names(binding);
  binding.ctor_initializer =
      ctx.own_synthetic_ast(make_instantiated_inherited_constructor_initializer(
          *base,
          binding_explicit_params(binding)));
  binding.has_definition = true;
  if(!binding.definition_node) {
    binding.definition_node =
        source_decl.definition_node ? source_decl.definition_node :
        (source_decl.declaration_node ? source_decl.declaration_node :
                                       binding.declaration_node);
  }
}

std::string instantiated_source_parameter_name(const FunctionBinding & binding,
                                               std::size_t index)
{
  const std::string alias_name = function_parameter_alias_name(binding, index);
  if(!alias_name.empty()) {
    return alias_name;
  }
  return function_parameter_binding_name(binding, index);
}

void refresh_definition_parameter_names(
    FunctionBinding & binding,
    const std::vector<std::pair<std::string, TypePtr> > & explicit_params)
{
  const std::size_t explicit_offset = function_binding_explicit_parameter_offset(binding);
  if(binding.params.size() != explicit_params.size() + explicit_offset) {
    return;
  }
  for(std::size_t i = 0; i < explicit_params.size(); ++i) {
    if(!explicit_params[i].first.empty()) {
      binding.params[explicit_offset + i].first = explicit_params[i].first;
    }
  }
}

void record_definition_parameter_aliases(
    FunctionBinding & binding,
    const std::vector<std::pair<std::string, TypePtr> > & params)
{
  ensure_function_parameter_aliases(binding);
  const std::size_t offset = function_binding_explicit_parameter_offset(binding);
  if(binding.params.size() != params.size() + offset) {
    return;
  }
  for(std::size_t i = 0; i < params.size(); ++i) {
    binding.parameter_aliases[i + offset] =
        params[i].first.empty() ? binding.params[i + offset].first : params[i].first;
  }
}

void trace_function_template_drift(const char * stage,
                                   FunctionTemplateDecl & decl)
{
  if(!parser_trace::enabled("template.resolve") || decl.debug_signature.empty()) {
    return;
  }
  const std::string current = semantic_trace::function_template_signature_for_diagnostic(decl);
  if(current == decl.debug_signature) {
    return;
  }
  std::ostringstream trace;
  trace << "template-drift stage=" << stage
        << " template=" << decl.name
        << " decl="
        << (decl.debug_decl_location.empty() ? std::string("<none>") : decl.debug_decl_location)
        << " scope="
        << (decl.debug_scope_name.empty() ? std::string("<none>") : decl.debug_scope_name)
        << " original={" << decl.debug_signature << "}"
        << " current={" << current << "}";
  if(!decl.debug_decl_location_details.empty()) {
    trace << " details={" << decl.debug_decl_location_details << "}";
  }
  parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
}

std::string function_template_params_for_trace(const FunctionTemplateDecl & decl)
{
  std::ostringstream trace;
  trace << "{";
  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    if(i != 0) {
      trace << ", ";
    }
    trace << (decl.params_pattern[i].first.empty() ?
                  std::string("<empty>") :
                  decl.params_pattern[i].first)
          << ":" << describe_type(decl.params_pattern[i].second);
    if(i < decl.parameter_aliases_pattern.size()) {
      trace << " alias="
            << (decl.parameter_aliases_pattern[i].empty() ?
                    std::string("<empty>") :
                    decl.parameter_aliases_pattern[i]);
    }
  }
  trace << "}";
  return trace.str();
}

bool generated_function_template_param_name(const std::string & name, std::size_t index)
{
  const std::string generated_name = std::string("arg") + std::to_string(index);
  return name.empty() ||
         name == generated_name ||
         name.compare(0, 7, "__param") == 0;
}

void sync_function_template_parameter_aliases(FunctionTemplateDecl & target,
                                              const FunctionTemplateDecl & source)
{
  if(target.params_pattern.size() != source.params_pattern.size()) {
    return;
  }

  ensure_function_template_parameter_aliases(target);
  for(std::size_t i = 0; i < target.params_pattern.size(); ++i) {
    if(generated_function_template_param_name(target.params_pattern[i].first, i) &&
       !source.params_pattern[i].first.empty()) {
      target.params_pattern[i].first = source.params_pattern[i].first;
    }

    const std::string alias_name = function_template_parameter_alias_name(source, i);
    if(!alias_name.empty() && i < target.parameter_aliases_pattern.size()) {
      target.parameter_aliases_pattern[i] = alias_name;
    } else if(target.parameter_aliases_pattern[i].empty()) {
      target.parameter_aliases_pattern[i] = target.params_pattern[i].first;
    }
  }
}

std::string specialization_name_for_instantiation_impl(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<TemplateArgument> & arguments)
{
  std::string out = name;
  out += "<";
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    if(arguments[i].kind == TemplateArgument::TA_TYPE && arguments[i].type) {
      out += ctx.instantiation_identity_text_for_type_argument(arguments[i].type);
      continue;
    }
    out += template_model::template_argument_text(
        arguments[i],
        [&ctx](const TypePtr & type)
        {
          return ctx.instantiation_identity_text_for_type_argument(type);
        });
  }
  out += ">";
  return out;
}

std::string display_specialization_name_for_instantiation_impl(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<TemplateArgument> & arguments)
{
  std::string out = name;
  out += "<";
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    out += template_argument_text_for_diagnostic(ctx, arguments[i]);
  }
  out += ">";
  return out;
}

bool template_arguments_are_dependent_for_instantiation(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments)
{
  return template_model::template_arguments_are_dependent(
      arguments,
      [&ctx](const TypePtr & type)
      {
        return template_argument_semantics::type_depends_on_template_parameter(ctx, type);
      });
}

void append_template_argument_diagnostic(SemanticContext & ctx,
                                         std::ostringstream & out,
                                         const std::vector<TemplateArgument> & arguments)
{
  out << " [template-args";
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    out << (i == 0 ? " " : ", ")
        << template_argument_text_for_diagnostic(ctx, arguments[i]);
  }
  out << "]";
}

void append_template_argument_diagnostic(template_api::TemplateTypeSystem & type_system,
                                         std::ostringstream & out,
                                         const std::vector<TemplateArgument> & arguments)
{
  out << " [template-args";
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    out << (i == 0 ? " " : ", ")
        << template_argument_text_for_diagnostic(type_system, arguments[i]);
  }
  out << "]";
}

void append_template_parameter_diagnostic(std::ostringstream & out,
                                          const std::vector<TemplateParameterInfo> & parameters)
{
  out << " [template-params";
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    out << (i == 0 ? " " : ", ");
    if(parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE) {
      out << "non-type ";
    } else if(parameters[i].kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      out << "template ";
    } else {
      out << "type ";
    }
    out << (parameters[i].name.empty() ? ("#" + std::to_string(i)) : parameters[i].name);
    if(parameters[i].parameter_pack) {
      out << "...";
    }
  }
  out << "]";
}

void ensure_template_arguments_fully_bind_parameters(
    SemanticContext & ctx,
    const char * stage,
    const std::string & template_name,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments)
{
  if(template_arguments_fully_bind_parameters(parameters, arguments)) {
    return;
  }

  std::ostringstream out;
  out << stage << ": template arguments do not fully bind parameters";
  if(!template_name.empty()) {
    out << " for " << template_name;
  }
  out << " [param-count " << parameters.size() << "]";
  out << " [arg-count " << arguments.size() << "]";
  append_template_parameter_diagnostic(out, parameters);
  append_template_argument_diagnostic(ctx, out, arguments);
  throw std::logic_error(out.str());
}

void ensure_template_arguments_fully_bind_parameters(
    template_api::TemplateTypeSystem & type_system,
    const char * stage,
    const std::string & template_name,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments)
{
  if(template_arguments_fully_bind_parameters(parameters, arguments)) {
    return;
  }

  std::ostringstream out;
  out << stage << ": template arguments do not fully bind parameters";
  if(!template_name.empty()) {
    out << " for " << template_name;
  }
  out << " [param-count " << parameters.size() << "]";
  out << " [arg-count " << arguments.size() << "]";
  append_template_parameter_diagnostic(out, parameters);
  append_template_argument_diagnostic(type_system, out, arguments);
  throw std::logic_error(out.str());
}

std::string instantiated_class_context(SemanticContext & ctx,
                                       const ClassInfo & info,
                                       const ClassTemplateDecl & decl,
                                       const std::vector<TemplateArgument> & arguments)
{
  std::ostringstream out;
  out << " [instantiated class " << info.qualified_name << "]";
  out << " [source template " << decl.name << "]";
  out << " [dependent-instantiation " << (info.dependent_instantiation ? "yes" : "no")
      << "]";
  out << " [member-scope-placeholders "
      << (ctx.scope_has_template_placeholders(*info.member_scope) ? "yes" : "no")
      << "]";
  append_template_argument_diagnostic(ctx, out, arguments);
  return out.str();
}

bool declarator_has_parameter_pack(const CppAstNode & declarator)
{
  if(cpp_decl::find_child(declarator, CppAstKind::parameter_pack)) {
    return true;
  }
  const CppAstNode * nested = cpp_decl::find_child(declarator, CppAstKind::nested_declarator);
  return nested && !nested->children.empty() && declarator_has_parameter_pack(nested->children[0]);
}

bool erase_parameter_pack_nodes(CppAstNode & current)
{
  bool removed = false;
  std::vector<CppAstNode> kept;
  kept.reserve(current.children.size());
  for(std::size_t i = 0; i < current.children.size(); ++i) {
    if(current.children[i].kind == CppAstKind::parameter_pack) {
      removed = true;
      continue;
    }
    removed = erase_parameter_pack_nodes(current.children[i]) || removed;
    kept.push_back(current.children[i]);
  }
  current.children.swap(kept);
  return removed;
}

bool expand_instantiated_function_parameter_clause(
    SemanticContext & ctx,
    Scope & inst_scope,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_args)
{
  params.clear();
  default_args.clear();

  Scope parameter_scope(&inst_scope, "<parameter-clause>", false);
  parameter_scope.class_info = inst_scope.class_info;
  parameter_scope.function = inst_scope.function;
  parameter_scope.namespace_scope = inst_scope.namespace_scope;

  for(std::size_t i = 0; i < parameter_clause.children.size(); ++i) {
    const CppAstNode & parameter = parameter_clause.children[i];
    if(parameter.kind != CppAstKind::parameter_declaration) {
      continue;
    }

    const CppAstNode * declarator =
        cpp_decl::find_child(parameter, CppAstKind::declarator);
    if(!declarator) {
      declarator = cpp_decl::find_child(parameter, CppAstKind::abstract_declarator);
    }
    const bool is_pack_parameter =
        declarator && declarator_has_parameter_pack(*declarator);

    if(!is_pack_parameter) {
      CppAstNode single_clause;
      single_clause.kind = CppAstKind::parameter_clause;
      single_clause.children.push_back(parameter);
      std::vector<std::pair<std::string, TypePtr> > single_params;
      std::vector<const CppAstNode *> single_defaults;
      if(!ctx.parse_parameter_clause(parameter_scope,
                                     single_clause,
                                     single_params,
                                     &single_defaults,
                                     true) ||
         single_params.size() != 1) {
        return false;
      }
      params.push_back(single_params[0]);
      default_args.push_back(single_defaults.empty() ? nullptr : single_defaults[0]);
      template_scope::bind_parameter_value(parameter_scope,
                                           single_params[0].first,
                                           single_params[0].second);
      continue;
    }

    CppAstNode stripped_parameter = parameter;
    if(!erase_parameter_pack_nodes(stripped_parameter)) {
      return false;
    }

    const std::vector<std::pair<std::string, const std::vector<TypePtr> *> > packs =
        pack_parameter_analysis::referenced_named_type_packs(parameter_scope,
                                                             stripped_parameter);
    if(packs.empty()) {
      return false;
    }

    const std::size_t pack_size = packs[0].second->size();
    for(std::size_t j = 1; j < packs.size(); ++j) {
      if(packs[j].second->size() != pack_size) {
        return false;
      }
    }

    for(std::size_t pack_index = 0; pack_index < pack_size; ++pack_index) {
      Scope single_scope(&parameter_scope, "<pack-param>", false);
      single_scope.class_info = parameter_scope.class_info;
      single_scope.function = parameter_scope.function;
      single_scope.namespace_scope = parameter_scope.namespace_scope;
      for(std::size_t pack = 0; pack < packs.size(); ++pack) {
        template_scope::bind_named_type(single_scope,
                                        packs[pack].first,
                                        (*(packs[pack].second))[pack_index]);
      }

      CppAstNode single_clause;
      single_clause.kind = CppAstKind::parameter_clause;
      single_clause.children.push_back(stripped_parameter);
      std::vector<std::pair<std::string, TypePtr> > single_params;
      std::vector<const CppAstNode *> single_defaults;
      if(!ctx.parse_parameter_clause(single_scope,
                                     single_clause,
                                     single_params,
                                     &single_defaults,
                                     true) ||
         single_params.size() != 1) {
        return false;
      }
      params.push_back(single_params[0]);
      default_args.push_back(single_defaults.empty() ? nullptr : single_defaults[0]);
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(parameter);
    if(!pack_name.empty() && params.size() >= pack_size) {
      std::vector<TypePtr> pack_value_types;
      pack_value_types.reserve(pack_size);
      for(std::size_t j = 0; j < pack_size; ++j) {
        pack_value_types.push_back(params[params.size() - pack_size + j].second);
      }
      template_scope::bind_parameter_value_pack(parameter_scope,
                                                pack_name,
                                                pack_value_types);
    }
  }

  return true;
}

void parse_instantiated_function_template_parameter_clause(
    SemanticContext & ctx,
    Scope & inst_scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_args)
{
  const witness::ScopedTemplateWitnessSourceCapturePause source_capture_pause;
  if(ctx.parse_parameter_clause(inst_scope,
                                parameter_clause,
                                params,
                                &default_args,
                                false)) {
    return;
  }

  template_function_signature::collect_function_template_default_arguments(
      parameter_clause,
      default_args);
  std::ostringstream out;
  out << "unsupported instantiated function template parameter-clause";
  out << " [template " << template_name << "]";
  out << " [parameter-clause " << cppast_kind_text(parameter_clause.kind) << "]";
  throw TemplateSubstitutionFailure(out.str());
}

bool build_instantiated_function_parameter_pack_fallback(
    SemanticContext & ctx,
    Scope & inst_scope,
    FunctionTemplateDecl & decl,
    std::string & name,
    TypePtr & type,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_args)
{
  if(!decl.declarator || !decl.has_trailing_function_parameter_pack) {
    return false;
  }
  const CppAstNode * parameter_clause =
      cpp_decl::find_child(*decl.declarator, CppAstKind::parameter_clause);
  if(!parameter_clause) {
    return false;
  }
  if(!expand_instantiated_function_parameter_clause(ctx,
                                                    inst_scope,
                                                    *parameter_clause,
                                                    params,
                                                    default_args)) {
    return false;
  }
  name = decl.name;
  type = decl.type_pattern;
  return true;
}

bool function_template_definition_should_suppress_declaration_abi_tags(
    SemanticContext & ctx,
    const FunctionTemplateDecl & decl)
{
  if(!decl.declaring_scope ||
     !decl.declaring_scope->class_info ||
     !decl.declaring_scope->class_info->source_template ||
     !decl.definition_node ||
     decl.definition_node == decl.declaration_node ||
     !function_template_definition_abi_tags(decl).empty()) {
    return false;
  }
  return true;
}

bool refresh_instantiated_function_parameter_clause(
    SemanticContext & ctx,
    Scope & inst_scope,
    FunctionTemplateDecl & decl,
    TypePtr & type,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_args)
{
  (void)default_args;
  const CppAstNode * parameter_clause = function_template_parameter_clause(decl);
  if(!parameter_clause) {
    return false;
  }

  std::vector<std::pair<std::string, TypePtr> > refreshed_params;
  std::vector<const CppAstNode *> refreshed_default_args;
  if(!expand_instantiated_function_parameter_clause(ctx,
                                                    inst_scope,
                                                    *parameter_clause,
                                                    refreshed_params,
                                                    refreshed_default_args)) {
    return false;
  }

  params.swap(refreshed_params);

  TypePtr function_type = strip_top_level_cv(type);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     function_type->params.size() == params.size()) {
    std::vector<TypePtr> rebuilt_params;
    rebuilt_params.reserve(params.size());
    for(std::size_t i = 0; i < params.size(); ++i) {
      rebuilt_params.push_back(params[i].second);
    }
    type = make_function(function_type->inner,
                         rebuilt_params,
                         function_type->variadic,
                         function_type->function_const,
                         function_type->function_volatile,
                         function_type->prototype_relaxed,
                         function_type->function_ref_qualifier);
  }

  return true;
}

bool member_function_template_decl_equivalent(FunctionTemplateDecl * lhs,
                                              FunctionTemplateDecl * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs ||
     lhs->inner != rhs->inner ||
     lhs->declarator != rhs->declarator ||
     lhs->specifiers != rhs->specifiers ||
     lhs->is_constructor != rhs->is_constructor ||
     lhs->is_destructor != rhs->is_destructor) {
    return false;
  }
  ClassInfo * lhs_owner = lhs->declaring_scope ? lhs->declaring_scope->class_info : nullptr;
  ClassInfo * rhs_owner = rhs->declaring_scope ? rhs->declaring_scope->class_info : nullptr;
  if(lhs_owner == rhs_owner) {
    return true;
  }
  if(!lhs_owner || !rhs_owner || lhs_owner->name != rhs_owner->name) {
    return false;
  }
  return lhs_owner->source_template &&
         lhs_owner->source_template == rhs_owner->source_template;
}

bool member_function_template_decl_same_source_location(SemanticContext & ctx,
                                                        FunctionTemplateDecl * lhs,
                                                        FunctionTemplateDecl * rhs)
{
  if(!lhs || !rhs ||
     lhs->name != rhs->name ||
     lhs->is_constructor != rhs->is_constructor ||
     lhs->is_destructor != rhs->is_destructor ||
     lhs->parameters.size() != rhs->parameters.size() ||
     lhs->params_pattern.size() != rhs->params_pattern.size()) {
    return false;
  }

  const std::string lhs_loc = semantic_trace::template_decl_primary_location(ctx, lhs);
  const std::string rhs_loc = semantic_trace::template_decl_primary_location(ctx, rhs);
  if(lhs_loc.empty() || rhs_loc.empty() || lhs_loc == "<none>" || rhs_loc == "<none>" ||
     lhs_loc != rhs_loc) {
    return false;
  }

  const std::string lhs_details = semantic_trace::template_decl_location_details(ctx, lhs);
  const std::string rhs_details = semantic_trace::template_decl_location_details(ctx, rhs);
  return !lhs_details.empty() && lhs_details == rhs_details;
}

bool template_parameter_index_for_type(
    const TypePtr & type,
    const std::vector<TemplateParameterInfo> & parameters,
    std::size_t & index)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED || parameters.empty()) {
    return false;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base->named_key);
  if(!parameter && base->named_display != base->named_key) {
    parameter = find_template_parameter(parameters, base->named_display);
  }
  if(!parameter) {
    return false;
  }
  index = static_cast<std::size_t>(parameter - &parameters[0]);
  return true;
}

bool template_parameter_type_shape_matches(
    const TypePtr & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters,
    bool allow_nontemplate_named_mismatch)
{
  if(lhs.get() == rhs.get()) {
    return true;
  }
  if(!lhs || !rhs) {
    return lhs == rhs;
  }

  std::size_t lhs_index = 0;
  std::size_t rhs_index = 0;
  const bool lhs_is_parameter =
      template_parameter_index_for_type(lhs, lhs_parameters, lhs_index);
  const bool rhs_is_parameter =
      template_parameter_index_for_type(rhs, rhs_parameters, rhs_index);
  if(lhs_is_parameter || rhs_is_parameter) {
    if(!lhs_is_parameter || !rhs_is_parameter || lhs_index != rhs_index) {
      return false;
    }
    const TemplateParameterInfo & lhs_parameter = lhs_parameters[lhs_index];
    const TemplateParameterInfo & rhs_parameter = rhs_parameters[rhs_index];
    return lhs_parameter.kind == rhs_parameter.kind &&
           lhs_parameter.parameter_pack == rhs_parameter.parameter_pack &&
           lhs_parameter.template_parameter_count ==
               rhs_parameter.template_parameter_count;
  }

  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base) {
    return lhs_base == rhs_base;
  }
  if(lhs_base->kind != rhs_base->kind) {
    return false;
  }

  switch(lhs_base->kind) {
  case Type::TK_FUNDAMENTAL:
    return lhs_base->fundamental == rhs_base->fundamental;

  case Type::TK_NAMED:
    if(!allow_nontemplate_named_mismatch) {
      return lhs_base->named_key == rhs_base->named_key &&
             lhs_base->named_display == rhs_base->named_display;
    }
    // Owner template parameters may already be substituted on one side. Once a
    // type position does not mention the member template parameters, it does
    // not distinguish overloads in this recovery path.
    return true;

  case Type::TK_CV:
    return lhs_base->cv_const == rhs_base->cv_const &&
           lhs_base->cv_volatile == rhs_base->cv_volatile &&
           template_parameter_type_shape_matches(lhs_base->inner,
                                                 lhs_parameters,
                                                 rhs_base->inner,
                                                 rhs_parameters,
                                                 allow_nontemplate_named_mismatch);

  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return template_parameter_type_shape_matches(lhs_base->inner,
                                                 lhs_parameters,
                                                 rhs_base->inner,
                                                 rhs_parameters,
                                                 allow_nontemplate_named_mismatch);

  case Type::TK_MEMBER_POINTER:
    return template_parameter_type_shape_matches(lhs_base->owner,
                                                 lhs_parameters,
                                                 rhs_base->owner,
                                                 rhs_parameters,
                                                 allow_nontemplate_named_mismatch) &&
           template_parameter_type_shape_matches(lhs_base->inner,
                                                 lhs_parameters,
                                                 rhs_base->inner,
                                                 rhs_parameters,
                                                 allow_nontemplate_named_mismatch);

  case Type::TK_ARRAY:
    return lhs_base->has_bound == rhs_base->has_bound &&
           lhs_base->bound == rhs_base->bound &&
           lhs_base->bound_text == rhs_base->bound_text &&
           template_parameter_type_shape_matches(lhs_base->inner,
                                                 lhs_parameters,
                                                 rhs_base->inner,
                                                 rhs_parameters,
                                                 allow_nontemplate_named_mismatch);

  case Type::TK_FUNCTION:
    if(lhs_base->variadic != rhs_base->variadic ||
       lhs_base->prototype_relaxed != rhs_base->prototype_relaxed ||
       lhs_base->function_const != rhs_base->function_const ||
       lhs_base->function_volatile != rhs_base->function_volatile ||
       lhs_base->function_ref_qualifier != rhs_base->function_ref_qualifier ||
       lhs_base->params.size() != rhs_base->params.size() ||
       !template_parameter_type_shape_matches(lhs_base->inner,
                                              lhs_parameters,
                                              rhs_base->inner,
                                              rhs_parameters,
                                              allow_nontemplate_named_mismatch)) {
      return false;
    }
    for(std::size_t i = 0; i < lhs_base->params.size(); ++i) {
      if(!template_parameter_type_shape_matches(lhs_base->params[i],
                                                lhs_parameters,
                                                rhs_base->params[i],
                                                rhs_parameters,
                                                allow_nontemplate_named_mismatch)) {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool template_parameter_type_shape_matches(
    const TypePtr & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TypePtr & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  return template_parameter_type_shape_matches(lhs,
                                               lhs_parameters,
                                               rhs,
                                               rhs_parameters,
                                               false);
}

bool template_parameter_identifier_matches(
    const TemplateParameterInfo & parameter,
    const std::string & name)
{
  if(parameter.name == name) {
    return true;
  }
  for(std::size_t i = 0; i < parameter.alternate_names.size(); ++i) {
    if(parameter.alternate_names[i] == name) {
      return true;
    }
  }
  return false;
}

std::string canonical_template_parameter_syntax_text(
    const std::string & text,
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size();) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(std::isspace(ch)) {
      ++i;
      continue;
    }
    if(std::isalpha(ch) || text[i] == '_') {
      const std::size_t start = i;
      ++i;
      while(i < text.size()) {
        const unsigned char inner = static_cast<unsigned char>(text[i]);
        if(!(std::isalnum(inner) || text[i] == '_')) {
          break;
        }
        ++i;
      }
      const std::string token = text.substr(start, i - start);
      bool replaced = false;
      for(std::size_t p = 0; p < parameters.size(); ++p) {
        if(template_parameter_identifier_matches(parameters[p], token)) {
          out += "$P";
          out += std::to_string(p);
          replaced = true;
          break;
        }
      }
      if(!replaced) {
        out += token;
      }
      continue;
    }
    out += text[i];
    ++i;
  }
  return out;
}

std::string canonical_template_parameter_syntax_text(
    const CppAstNode * node,
    const std::vector<TemplateParameterInfo> & parameters)
{
  return node ?
      canonical_template_parameter_syntax_text(node_text(*node), parameters) :
      std::string();
}

bool template_parameter_optional_syntax_matches(
    const CppAstNode * lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const CppAstNode * rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(!lhs || !rhs) {
    return true;
  }
  return canonical_template_parameter_syntax_text(lhs, lhs_parameters) ==
         canonical_template_parameter_syntax_text(rhs, rhs_parameters);
}

std::string template_parameter_non_type_specifier_text(
    const TemplateParameterInfo & parameter)
{
  if(!parameter.non_type_decl_specifier_text.empty()) {
    return parameter.non_type_decl_specifier_text;
  }
  if(parameter.non_type_decl_specifier_seq) {
    const std::string text = node_text(*parameter.non_type_decl_specifier_seq);
    if(!text.empty()) {
      return text;
    }
  }
  return parameter.value_type ? describe_type(parameter.value_type) : std::string();
}

bool template_parameter_non_type_specifiers_match(
    const TemplateParameterInfo & lhs,
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const TemplateParameterInfo & rhs,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  const std::string lhs_text = template_parameter_non_type_specifier_text(lhs);
  const std::string rhs_text = template_parameter_non_type_specifier_text(rhs);
  if(lhs_text.empty() || rhs_text.empty()) {
    return lhs_text.empty() && rhs_text.empty();
  }
  return canonical_template_parameter_syntax_text(lhs_text, lhs_parameters) ==
         canonical_template_parameter_syntax_text(rhs_text, rhs_parameters);
}

bool member_function_template_parameters_compatible(
    const std::vector<TemplateParameterInfo> & lhs,
    const std::vector<TemplateParameterInfo> & rhs)
{
  if(lhs.size() != rhs.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs.size(); ++i) {
    if(lhs[i].kind != rhs[i].kind ||
       lhs[i].parameter_pack != rhs[i].parameter_pack ||
       lhs[i].template_parameter_count != rhs[i].template_parameter_count) {
      return false;
    }
    if(lhs[i].kind == TemplateParameterInfo::TP_NON_TYPE) {
      if(!template_parameter_type_shape_matches(lhs[i].value_type,
                                                lhs,
                                                rhs[i].value_type,
                                                rhs,
                                                true) ||
         !template_parameter_non_type_specifiers_match(lhs[i],
                                                       lhs,
                                                       rhs[i],
                                                       rhs) ||
         !template_parameter_optional_syntax_matches(lhs[i].non_type_decl_specifier_seq,
                                                     lhs,
                                                     rhs[i].non_type_decl_specifier_seq,
                                                     rhs) ||
         !template_parameter_optional_syntax_matches(lhs[i].default_argument,
                                                     lhs,
                                                     rhs[i].default_argument,
                                                     rhs)) {
        return false;
      }
    }
  }
  return true;
}

bool member_function_template_decl_semantic_signature_matches(
    FunctionTemplateDecl * lhs,
    FunctionTemplateDecl * rhs)
{
  if(!lhs || !rhs ||
     lhs->name != rhs->name ||
     lhs->is_constructor != rhs->is_constructor ||
     lhs->is_destructor != rhs->is_destructor ||
     lhs->is_static_member != rhs->is_static_member ||
     lhs->is_const_method != rhs->is_const_method ||
     lhs->is_volatile_method != rhs->is_volatile_method ||
     lhs->ref_qualifier != rhs->ref_qualifier ||
     !member_function_template_parameters_compatible(lhs->parameters,
                                                     rhs->parameters) ||
     lhs->params_pattern.size() != rhs->params_pattern.size()) {
    return false;
  }

  for(std::size_t i = 0; i < lhs->params_pattern.size(); ++i) {
    if(!template_parameter_type_shape_matches(lhs->params_pattern[i].second,
                                              lhs->parameters,
                                              rhs->params_pattern[i].second,
                                              rhs->parameters)) {
      return false;
    }
  }
  return true;
}

std::string function_template_decl_location(SemanticContext & ctx,
                                            const FunctionTemplateDecl * decl)
{
  return semantic_trace::template_decl_primary_location(ctx, decl);
}

const PartialClassTemplateSpecializationDecl * selected_partial_specialization(
    ClassTemplateDecl & decl,
    const ClassInfo & info)
{
  if(!info.template_output_node || info.template_output_node == decl.class_node) {
    return nullptr;
  }

  for(std::size_t i = 0; i < decl.partial_specializations.size(); ++i) {
    if(decl.partial_specializations[i].class_node == info.template_output_node) {
      return &decl.partial_specializations[i];
    }
  }
  return nullptr;
}

ClassInfo * source_owner_class_for_instantiation(SemanticContext & ctx,
                                                 ClassTemplateDecl & decl,
                                                 ClassInfo & info)
{
  if(!decl.declaring_scope) {
    return nullptr;
  }

  if(const PartialClassTemplateSpecializationDecl * partial =
         selected_partial_specialization(decl, info)) {
    Scope * lookup_scope = partial->pattern_scope ? partial->pattern_scope : partial->declaring_scope;
    const CppAstNode * lookup_node = partial->class_node ? partial->class_node : info.template_output_node;
    ClassInfo * partial_info = nullptr;
    if(lookup_scope &&
       lookup_node &&
       !lookup_node->value.empty()) {
      partial_info = lookup_declared_owner_class_via_leaf_type_lookup(
          ctx, *lookup_scope, lookup_node->value);
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "source-owner-partial class=" << info.qualified_name
              << " template=" << decl.name
              << " lookup=" << lookup_node->value
              << " result=" << (partial_info ? partial_info->qualified_name : std::string("<none>"))
              << " same=" << ((partial_info == &info) ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(partial_info && partial_info != &info) {
        return partial_info;
      }
    }

    // Partial specialization declarations may not have a standalone declaration-side
    // ClassInfo. Do not recreate one by instantiating the declaration pattern text,
    // because that manufactures a dependent owner such as <_Tp, true> while we are
    // finalizing a concrete selected specialization. In that case, keep the current
    // specialization as its own source owner and skip the primary-template fallback.
    return &info;
  }

  const auto acceptable_primary_source_owner = [&](ClassInfo * candidate) -> bool
  {
    if(!candidate ||
       candidate == &info ||
       candidate->source_template != &decl) {
      return false;
    }
    if(candidate->template_output_node &&
       decl.class_node &&
       candidate->template_output_node != decl.class_node) {
      return false;
    }
    if(candidate->dependent_instantiation ||
       candidate->instantiation_arguments.empty()) {
      return true;
    }
    return template_arguments_are_dependent_for_instantiation(
        ctx, candidate->instantiation_arguments);
  };

  ClassInfo * source_owner =
      lookup_declared_owner_class_via_leaf_type_lookup(
          ctx, *decl.declaring_scope, decl.name);
  if(!acceptable_primary_source_owner(source_owner)) {
    source_owner = class_template_dependent_reference_source_owner(decl);
  }
  if(!acceptable_primary_source_owner(source_owner)) {
    source_owner = nullptr;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "source-owner-primary class=" << info.qualified_name
          << " template=" << decl.name
          << " result=" << (source_owner ? source_owner->qualified_name : std::string("<none>"))
          << " same=" << ((source_owner == &info) ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return source_owner != &info ? source_owner : nullptr;
}

ClassInfo * source_owner_class_for_instantiation(SemanticContext & ctx,
                                                 ClassInfo * instantiation_owner)
{
  if(!instantiation_owner || !instantiation_owner->source_template) {
    return nullptr;
  }
  return source_owner_class_for_instantiation(
      ctx, *instantiation_owner->source_template, *instantiation_owner);
}

std::string function_binding_node_location(SemanticContext & ctx,
                                           const FunctionBinding * binding,
                                           bool definition)
{
  if(!binding) {
    return std::string("<none>");
  }
  const CppAstNode * node = definition ? binding->definition_node : binding->declaration_node;
  return node ? ctx.source_location_for_node(*node) : std::string("<none>");
}

void append_function_binding_trace_identity(std::ostringstream & trace,
                                            SemanticContext & ctx,
                                            const FunctionBinding * binding)
{
  trace << " binding=" << static_cast<const void *>(binding);
  if(!binding) {
    return;
  }
  trace << " binding_name=" << binding->name
        << " binding_key="
        << function_binding_template_registration_key(*binding)
        << " binding_owner="
        << (binding->owner_class ? binding->owner_class->qualified_name : std::string("<none>"))
        << " binding_decl_loc=" << function_binding_node_location(ctx, binding, false)
        << " binding_def_loc=" << function_binding_node_location(ctx, binding, true)
        << " binding_source_template="
        << static_cast<const void *>(binding->source_template)
        << " binding_source_decl_loc="
        << function_template_decl_location(ctx, binding->source_template);
  const std::string source_decl_details =
      semantic_trace::template_decl_location_details(ctx, binding->source_template);
  if(!source_decl_details.empty()) {
    trace << " binding_source_decl_detail=" << source_decl_details;
  }
  trace
        << " binding_source_owner="
        << (binding->source_template &&
                    binding->source_template->declaring_scope &&
                    binding->source_template->declaring_scope->class_info ?
                binding->source_template->declaring_scope->class_info->qualified_name :
                std::string("<none>"));
  trace << " params={";
  for(std::size_t i = 0; i < binding->params.size(); ++i) {
    if(i != 0) {
      trace << ", ";
    }
    trace << (binding->params[i].first.empty() ? std::string("<empty>") : binding->params[i].first)
          << ":" << describe_type(binding->params[i].second);
    if(i < binding->parameter_aliases.size()) {
      trace << " alias="
            << (binding->parameter_aliases[i].empty() ?
                    std::string("<empty>") :
                    binding->parameter_aliases[i]);
    }
  }
  trace << "}";
}

FunctionTemplateDecl * source_member_function_template_decl(SemanticContext & ctx,
                                                            ClassInfo * instantiation_owner,
                                                            FunctionTemplateDecl * decl)
{
  if(!instantiation_owner ||
     !instantiation_owner->source_template ||
     !instantiation_owner->source_template->declaring_scope ||
     !decl ||
     !decl->declaring_scope ||
     !decl->declaring_scope->class_info ||
     decl->declaring_scope->class_info != instantiation_owner) {
    return nullptr;
  }

  ClassInfo * source_owner =
      source_owner_class_for_instantiation(ctx, instantiation_owner);
  if(!source_owner || !source_owner->member_scope || source_owner == instantiation_owner) {
    return nullptr;
  }

  const std::vector<FunctionTemplateDecl *> candidates =
      semantic_lookup::lookup_direct_function_templates(*source_owner->member_scope,
                                                        decl->name);
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_equivalent(candidates[i], decl)) {
      return candidates[i];
    }
  }
  FunctionTemplateDecl * location_match = nullptr;
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_same_source_location(ctx, candidates[i], decl)) {
      if(location_match && location_match != candidates[i]) {
        location_match = nullptr;
        break;
      }
      location_match = candidates[i];
    }
  }
  if(location_match) {
    return location_match;
  }
  FunctionTemplateDecl * semantic_match = nullptr;
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_semantic_signature_matches(candidates[i],
                                                                decl)) {
      if(semantic_match && semantic_match != candidates[i]) {
        return nullptr;
      }
      semantic_match = candidates[i];
    }
  }
  return semantic_match;
}

FunctionTemplateDecl * canonical_instantiation_template_decl(SemanticContext & ctx,
                                                             ClassInfo * instantiation_owner,
                                                             FunctionTemplateDecl * decl)
{
  if(!instantiation_owner || !decl || !decl->declaring_scope || !decl->declaring_scope->class_info) {
    return decl;
  }

  if(instantiation_owner->is_explicit_specialization &&
     decl->declaring_scope->class_info == instantiation_owner) {
    return decl;
  }

  if(FunctionTemplateDecl * source_decl =
         source_member_function_template_decl(ctx, instantiation_owner, decl)) {
    sync_function_template_parameter_aliases(*decl, *source_decl);
    if(source_decl->body && !decl->body) {
      decl->body = source_decl->body;
      decl->ctor_initializer = source_decl->ctor_initializer;
    }
    if(source_decl->body || source_decl->ctor_initializer) {
      return source_decl;
    }
  }

  if(decl->declaring_scope->class_info == instantiation_owner) {
    return decl;
  }

  const std::vector<FunctionTemplateDecl *> candidates =
      semantic_lookup::lookup_direct_function_templates(*instantiation_owner->member_scope,
                                                        decl->name);
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_equivalent(candidates[i], decl)) {
      return candidates[i];
    }
  }
  FunctionTemplateDecl * location_match = nullptr;
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_same_source_location(ctx, candidates[i], decl)) {
      if(location_match && location_match != candidates[i]) {
        location_match = nullptr;
        break;
      }
      location_match = candidates[i];
    }
  }
  if(location_match) {
    return location_match;
  }
  FunctionTemplateDecl * semantic_match = nullptr;
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    if(member_function_template_decl_semantic_signature_matches(candidates[i],
                                                                decl)) {
      if(semantic_match && semantic_match != candidates[i]) {
        return decl;
      }
      semantic_match = candidates[i];
    }
  }
  return semantic_match ? semantic_match : decl;
}

ClassInfo * function_template_context_owner(const FunctionTemplateDecl & decl)
{
  if(decl.declaring_scope && decl.declaring_scope->class_info) {
    return decl.declaring_scope->class_info;
  }
  if(!decl.friend_access_classes.empty() &&
     decl.pattern_scope &&
     decl.pattern_scope->class_info) {
    return decl.pattern_scope->class_info;
  }
  return nullptr;
}

Scope * function_template_instantiation_context_scope(const FunctionTemplateDecl & decl)
{
  if(decl.declaring_scope && decl.declaring_scope->class_info) {
    return decl.declaring_scope;
  }
  if(!decl.friend_access_classes.empty() &&
     decl.pattern_scope &&
     decl.pattern_scope->class_info) {
    return decl.pattern_scope;
  }
  return decl.declaring_scope;
}

bool class_template_owner_matches(const ClassInfo * candidate,
                                  const ClassInfo * pattern_owner)
{
  if(!candidate || !pattern_owner) {
    return false;
  }
  if(candidate == pattern_owner) {
    return true;
  }
  if(candidate->source_template &&
     pattern_owner->source_template &&
     candidate->source_template == pattern_owner->source_template) {
    return true;
  }
  return candidate->name == pattern_owner->name &&
         candidate->source_template &&
         pattern_owner->source_template;
}

ClassInfo * select_hidden_friend_instantiation_owner(
    SemanticContext & ctx,
    const FunctionTemplateDecl & decl,
    const std::vector<TemplateArgument> & arguments)
{
  ClassInfo * pattern_owner = function_template_context_owner(decl);
  if(!pattern_owner ||
     decl.friend_access_classes.empty() ||
     (decl.declaring_scope && decl.declaring_scope->class_info)) {
    return nullptr;
  }

  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind != TemplateArgument::TA_TYPE || !arguments[i].type) {
      continue;
    }
    TypePtr candidate_type =
        strip_top_level_cv(remove_reference_type(arguments[i].type));
    ClassInfo * candidate = ctx.class_info_for_type(candidate_type);
    if(class_template_owner_matches(candidate, pattern_owner)) {
      return candidate;
    }
  }
  return nullptr;
}

ValueBinding * direct_static_member_definition_binding(ClassInfo & info,
                                                       const std::string & member_name)
{
  if(!info.member_scope) {
    return nullptr;
  }
  std::map<std::string, ValueBinding>::iterator found =
      info.member_scope->values.find(member_name);
  if(found == info.member_scope->values.end() ||
     found->second.kind != ValueBinding::VK_VARIABLE ||
     found->second.owner_class != &info) {
    return nullptr;
  }
  return &found->second;
}

ValueBinding * static_member_definition_binding_for_key(SemanticContext & ctx,
                                                        ClassInfo & info,
                                                        const std::string & member_key)
{
  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(member_key, qualified) ||
     qualified.qualifiers.empty()) {
    return direct_static_member_definition_binding(info, member_key);
  }

  std::size_t qualifier_start = 0;
  const std::string first_qualifier =
      semantic_utils::strip_trailing_top_level_template_arguments(
          qualified.qualifiers.front());
  if(first_qualifier == info.name ||
     first_qualifier == semantic_utils::unqualified_member_name(info.qualified_name)) {
    qualifier_start = 1;
  }

  ClassInfo * current = &info;
  for(std::size_t i = qualifier_start; i < qualified.qualifiers.size(); ++i) {
    if(!current->member_scope) {
      return nullptr;
    }
    if(!current->reference_members_collected &&
       !current->reference_member_collection_in_progress) {
      ctx.ensure_class_reference_members(*current);
    }
    const std::string member_class_name =
        semantic_utils::strip_trailing_top_level_template_arguments(
            qualified.qualifiers[i]);
    auto found =
        current->member_scope->named_types.find(member_class_name);
    if(found == current->member_scope->named_types.end()) {
      return nullptr;
    }
    ClassInfo * nested = ctx.class_info_for_type(found->second);
    if(!nested || nested->enclosing_scope != current->member_scope.get()) {
      return nullptr;
    }
    current = nested;
  }

  if(!current->member_scope) {
    return nullptr;
  }
  if(!current->reference_members_collected &&
     !current->reference_member_collection_in_progress) {
    ctx.ensure_class_reference_members(*current);
  }
  return direct_static_member_definition_binding(*current, qualified.name);
}

void apply_out_of_class_static_member_definitions(SemanticContext & ctx,
                                                  ClassTemplateDecl & decl,
                                                  ClassInfo & info,
                                                  const std::vector<TemplateArgument> & arguments)
{
  if(info.out_of_class_static_member_definitions_applied ||
     !info.member_scope) {
    return;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "apply-out-of-class-static-members class=" << info.qualified_name
          << " template=" << decl.name
          << " count=" << decl.static_member_definitions.size();
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(decl, info);
  const std::map<std::string, OutOfClassStaticMemberDecl> & static_member_definitions =
      partial ? partial->static_member_definitions : decl.static_member_definitions;
  bool missing_member = false;
  for(std::map<std::string, OutOfClassStaticMemberDecl>::const_iterator it =
          static_member_definitions.begin();
      it != static_member_definitions.end();
      ++it) {
    ValueBinding * member =
        static_member_definition_binding_for_key(ctx, info, it->first);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "apply-out-of-class-static-member class=" << info.qualified_name
            << " member=" << it->first
            << " found=" << (member ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!member) {
      missing_member = true;
      continue;
    }
    const std::string specialization_key =
        class_instantiation_key_for_metadata(ctx, info);
    if(decl.explicit_static_member_specialization_keys.count(
           std::make_pair(it->first, specialization_key)) != 0) {
      continue;
    }
    if(member->is_explicit_specialization) {
      continue;
    }
    note_out_of_class_owner_class_use_for_applied_definition(
        ctx,
        &decl,
        info,
        it->second.node ? it->second.node : it->second.declarator);
    if(witness::source_capture_enabled(ctx.template_witness_context())) {
      member->witness_static_member_definition_source_captured = true;
    }
    member->has_storage_definition = it->second.has_storage_definition;
    member->declaration_node = member->declaration_node ?
        member->declaration_node :
        it->second.declarator;
    member->definition_node = it->second.has_storage_definition ?
        it->second.declarator :
        member->definition_node;
    member->requires_constant_initializer =
        member->requires_constant_initializer ||
        (it->second.specifiers &&
         decl_spec_contains_token(*it->second.specifiers, KW_CONSTEXPR));
    if(!it->second.initializer) {
      continue;
    }

    member->constant_initializer = it->second.initializer;
    member->constant_initializer_scope = nullptr;
    clear_value_binding_constexpr_value(*member);
    member->has_constant_value = false;
    member->constant_value = 0;
    member->dependent_template_value = false;

    ClassInfo * member_owner = member->owner_class ? member->owner_class : &info;
    Scope * member_scope = member_owner->member_scope ?
        member_owner->member_scope.get() :
        info.member_scope.get();
    Scope & init_scope = ctx.append_template_scope(*member_scope);
    if(!partial) {
      bind_template_arguments_into_scope(ctx, init_scope, it->second.parameters, arguments);
    }
    member->constant_initializer_scope = &init_scope;
    ctx.emit_class_use_source_events_after_location(
        init_scope,
        ctx.source_location_for_node(*it->second.initializer),
        witness::SourceUseOwnership::SourceOwned);
  }
  info.out_of_class_static_member_definitions_applied =
      !missing_member ||
      (info.complete &&
       !info.reference_member_collection_in_progress &&
       !info.full_member_collection_in_progress);
}

void apply_out_of_class_member_function_definitions(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments)
{
  if(info.out_of_class_member_function_definitions_applied ||
     !info.member_scope) {
    return;
  }
  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(decl, info);
  const bool selected_specialization =
      info.template_output_node && decl.class_node &&
      info.template_output_node != decl.class_node;
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "apply-out-of-class-member-functions class=" << info.qualified_name
          << " template=" << decl.name
          << " count=" << decl.member_function_definitions.size();
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(!partial) {
    apply_stored_out_of_class_member_function_definitions_map(
        ctx,
        decl.member_function_definitions,
        info,
        arguments,
        selected_specialization,
        true);
  }
  if(partial) {
    apply_stored_out_of_class_member_function_definitions_map(
        ctx,
        partial->member_function_definitions,
        info,
        arguments,
        false,
        false);
  }
  info.out_of_class_member_function_definitions_applied = true;
}

void apply_selected_specialization_member_function_definitions(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info)
{
  if(!info.member_scope) {
    return;
  }

  ClassInfo * source_info = source_owner_class_for_instantiation(ctx, decl, info);
  if(!source_info || !source_info->member_scope || source_info == &info) {
    return;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "apply-selected-specialization-member-definitions class=" << info.qualified_name
          << " source=" << source_info->qualified_name
          << " method-groups=" << source_info->methods.size();
    parser_trace::note("template.resolve", std::string(), trace.str());
  }

  Scope & binding_scope = ctx.append_template_scope(*info.member_scope);
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          source_info->methods.begin();
      it != source_info->methods.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * source_binding = it->second[i];
      const bool is_conversion_operator =
          source_binding &&
          source_binding->is_conversion_operator;
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "apply-selected-specialization-member-candidate class="
              << info.qualified_name
              << " source=" << source_info->qualified_name
              << " name=" << (source_binding ? source_binding->display_name : std::string("<null>"))
              << " has-definition="
              << (source_binding && source_binding->has_definition ? "yes" : "no")
              << " has-body="
              << (source_binding && source_binding->body ? "yes" : "no")
              << " source-template="
              << (source_binding && source_binding->source_template ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!source_binding ||
         source_binding->source_template ||
         source_binding->is_constructor ||
         source_binding->is_destructor ||
         is_conversion_operator ||
         (source_binding->definition_node &&
          source_binding->declaration_node &&
          source_binding->definition_node != source_binding->declaration_node) ||
         !source_binding->has_definition ||
         !source_binding->body) {
        continue;
      }

      std::vector<std::pair<std::string, TypePtr> > instantiated_params;
      bool params_ok = true;
      for(std::size_t param_index = 1;
          param_index < source_binding->params.size();
          ++param_index) {
        TypePtr param_type = source_binding->params[param_index].second;
        TypePtr instantiated_param_type;
        if(param_type &&
           recover_instantiation_bound_type(ctx,
                                            binding_scope,
                                            param_type,
                                            instantiated_param_type) &&
           instantiated_param_type) {
          param_type = instantiated_param_type;
        }
        if(!param_type) {
          params_ok = false;
          break;
        }
        instantiated_params.push_back(
            std::make_pair(instantiated_source_parameter_name(*source_binding, param_index),
                           param_type));
      }
      if(!params_ok) {
        continue;
      }

      TypePtr declared_type = source_binding->declared_type;
      TypePtr instantiated_declared_type;
      if(declared_type &&
         recover_instantiation_bound_type(ctx,
                                          binding_scope,
                                          declared_type,
                                          instantiated_declared_type) &&
         instantiated_declared_type) {
        declared_type = instantiated_declared_type;
      }
      if(!declared_type) {
        continue;
      }

      FunctionBinding * binding =
          ctx.find_equivalent_class_function(info,
                                             source_binding->display_name,
                                             semantic_class_model::method_function_type(
                                                 info.type,
                                                 source_binding->is_const_method,
                                                 source_binding->is_volatile_method,
                                                 declared_type),
                                             source_binding->ref_qualifier);
      if(!binding &&
         template_argument_semantics::type_depends_on_template_parameter(ctx, declared_type)) {
        std::map<std::string, std::vector<FunctionBinding *> >::iterator slot =
            info.methods.find(source_binding->display_name);
        if(slot != info.methods.end()) {
          FunctionBinding * unique_match = nullptr;
          for(std::size_t candidate_index = 0;
              candidate_index < slot->second.size();
              ++candidate_index) {
            FunctionBinding * candidate = slot->second[candidate_index];
            if(!candidate ||
               candidate->has_definition ||
               candidate->ref_qualifier != source_binding->ref_qualifier ||
               candidate->is_const_method != source_binding->is_const_method ||
               candidate->is_volatile_method != source_binding->is_volatile_method) {
              continue;
            }
            if(unique_match && unique_match != candidate) {
              unique_match = nullptr;
              break;
            }
            unique_match = candidate;
          }
          if(unique_match) {
            binding = unique_match;
          }
        }
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "apply-selected-specialization-member-binding class="
              << info.qualified_name
              << " name=" << source_binding->display_name
              << " found=" << (binding ? "yes" : "no")
              << " target-has-definition="
              << (binding && binding->has_definition ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!binding || binding->has_definition) {
        continue;
      }

      refresh_definition_parameter_names(*binding, instantiated_params);
      binding->body = source_binding->body;
      record_definition_parameter_aliases(*binding, instantiated_params);
      binding->ctor_initializer = source_binding->ctor_initializer;
      binding->has_definition = true;
      binding->definition_node = source_binding->definition_node;
      binding->definition_abi_tags = source_binding->definition_abi_tags;
      binding->definition_suppresses_declaration_abi_tags =
          source_binding->definition_suppresses_declaration_abi_tags;
      binding->declaration_abi_tags = source_binding->declaration_abi_tags;
      binding->parameter_syntax_node = source_binding->parameter_syntax_node;
      binding->exclude_from_explicit_instantiation =
          binding->exclude_from_explicit_instantiation ||
          source_binding->exclude_from_explicit_instantiation;
      if(!binding->function_qualifier && source_binding->function_qualifier) {
        binding->function_qualifier = source_binding->function_qualifier;
      }
      ctx.upgrade_function_symbol_linkage(binding, binding->symbol.linkage);
    }
  }
}

void apply_out_of_class_special_member_definitions(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments)
{
  if(info.out_of_class_special_member_definitions_applied) {
    return;
  }
  if(!info.member_scope || !decl.declaring_scope) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "apply-out-of-class-special-members class=" << info.qualified_name
            << " skipped=missing-scope";
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return;
  }

  ClassInfo * source_info = source_owner_class_for_instantiation(ctx, decl, info);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "apply-out-of-class-special-members class=" << info.qualified_name
          << " source-class="
          << (source_info ? source_info->qualified_name : std::string("<none>"))
          << " same-source=" << (source_info == &info ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(!source_info || source_info == &info) {
    info.out_of_class_special_member_definitions_applied = true;
    return;
  }

  const bool selected_specialization =
      info.template_output_node && decl.class_node &&
      info.template_output_node != decl.class_node;

  Scope & binding_scope = ctx.append_template_scope(*info.member_scope);

  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          source_info->methods.begin();
      it != source_info->methods.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * source_binding = it->second[i];
      const bool is_conversion_operator =
          source_binding &&
          source_binding->is_conversion_operator;
      if(parser_trace::enabled("template.resolve") &&
         source_binding &&
         (source_binding->is_constructor ||
          source_binding->is_destructor ||
          is_conversion_operator)) {
        std::ostringstream trace;
        trace << "apply-out-of-class-special-member-source class=" << info.qualified_name
              << " source-owner=" << source_info->qualified_name
              << " member=" << source_binding->name
              << " display=" << source_binding->display_name
              << " has-definition=" << (source_binding->has_definition ? "yes" : "no")
              << " has-body=" << (source_binding->body ? "yes" : "no")
              << " key="
              << function_binding_template_registration_key(*source_binding);
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!source_binding ||
         (!source_binding->is_constructor &&
          !source_binding->is_destructor &&
          !is_conversion_operator) ||
         !source_binding->has_definition ||
         !source_binding->body) {
        continue;
      }
      if(selected_specialization &&
         !source_special_member_matches_selected_specialization(*source_binding, info)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "apply-out-of-class-special-member class=" << info.qualified_name
                << " member=" << source_binding->display_name
                << " skipped=owner-mismatch";
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }

      std::vector<std::pair<std::string, TypePtr> > instantiated_params;
      bool params_ok = true;
      for(std::size_t param_index = 1;
          param_index < source_binding->params.size();
          ++param_index) {
        TypePtr param_type = source_binding->params[param_index].second;
        TypePtr rebound_self_type =
            rebind_special_member_self_parameter_type(param_type,
                                                      source_info->type,
                                                      info.type);
        if(rebound_self_type) {
          param_type = rebound_self_type;
        }
        TypePtr instantiated_type;
        if(param_type &&
           recover_instantiation_bound_type(ctx,
                                            binding_scope,
                                            param_type,
                                            instantiated_type) &&
           instantiated_type) {
          param_type = instantiated_type;
        }
        if(!param_type) {
          params_ok = false;
          break;
        }
        instantiated_params.push_back(
            std::make_pair(instantiated_source_parameter_name(*source_binding, param_index),
                           param_type));
      }
      if(!params_ok) {
        continue;
      }

      FunctionBinding * binding = nullptr;
      if(is_conversion_operator) {
        TypePtr declared_type = source_binding->declared_type;
        TypePtr instantiated_type;
        if(declared_type &&
           recover_instantiation_bound_type(ctx,
                                            binding_scope,
                                            declared_type,
                                            instantiated_type) &&
           instantiated_type) {
          declared_type = instantiated_type;
        }
        if(!declared_type) {
          continue;
        }
        binding =
            ctx.find_equivalent_class_function(info,
                                               source_binding->display_name,
                                               semantic_class_model::method_function_type(
                                                   info.type,
                                                   source_binding->is_const_method,
                                                   source_binding->is_volatile_method,
                                                   declared_type),
                                               source_binding->ref_qualifier);
      } else {
        std::vector<TypePtr> effective_params;
        effective_params.push_back(make_pointer(info.type));
        for(std::size_t param_index = 0;
            param_index < instantiated_params.size();
            ++param_index) {
          effective_params.push_back(instantiated_params[param_index].second);
        }

        binding =
            ctx.find_equivalent_class_function(info,
                                               source_binding->display_name,
                                               make_function(make_fundamental(FT_VOID),
                                                             effective_params,
                                                             false));
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "apply-out-of-class-special-member-target class=" << info.qualified_name
              << " member=" << source_binding->display_name
              << " conversion=" << (is_conversion_operator ? "yes" : "no")
              << " target-found=" << (binding ? "yes" : "no")
              << " target-has-definition="
              << (binding && binding->has_definition ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!binding || binding->has_definition) {
        continue;
      }

      refresh_definition_parameter_names(*binding, instantiated_params);
      binding->body = source_binding->body;
      record_definition_parameter_aliases(*binding, instantiated_params);
      binding->ctor_initializer = source_binding->ctor_initializer;
      binding->has_definition = true;
      binding->definition_node = source_binding->definition_node;
      binding->definition_abi_tags = source_binding->definition_abi_tags;
      binding->definition_suppresses_declaration_abi_tags =
          source_binding->definition_suppresses_declaration_abi_tags;
      binding->declaration_abi_tags = source_binding->declaration_abi_tags;
      binding->parameter_syntax_node = source_binding->parameter_syntax_node;
      binding->exclude_from_explicit_instantiation =
          binding->exclude_from_explicit_instantiation ||
          source_binding->exclude_from_explicit_instantiation;
      if(!binding->function_qualifier && source_binding->function_qualifier) {
        binding->function_qualifier = source_binding->function_qualifier;
      }
      ctx.upgrade_function_symbol_linkage(binding, binding->symbol.linkage);
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "apply-out-of-class-special-member-copied class=" << info.qualified_name
              << " member=" << source_binding->display_name;
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
    }
  }
  info.out_of_class_special_member_definitions_applied = true;
}

void apply_out_of_class_member_function_template_definitions(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info)
{
  if(info.out_of_class_member_function_template_definitions_applied ||
     !info.member_scope) {
    return;
  }
  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(decl, info);
  if(!partial) {
    apply_stored_out_of_class_member_function_template_definitions_map(
        ctx,
        decl.member_function_template_definitions,
        info);
  }
  if(partial) {
    apply_stored_out_of_class_member_function_template_definitions_map(
        ctx,
        partial->member_function_template_definitions,
        info);
  }
  info.out_of_class_member_function_template_definitions_applied = true;
}

void finalize_nested_member_class_instantiation_impl(
    SemanticContext & ctx,
    ClassTemplateDecl & owner_decl,
    ClassInfo & nested,
    const std::vector<TemplateArgument> & owner_arguments,
    bool emit_track_instantiation)
{
  if(!nested.class_node ||
     nested.class_node->kind == CppAstKind::class_forward_declaration) {
    return;
  }

  if(!nested.complete) {
    ctx.ensure_class_reference_members(nested);
  }
  if(!nested.member_scope ||
     (!nested.complete && !nested.reference_members_collected)) {
    return;
  }

  apply_out_of_class_member_function_template_definitions(ctx, owner_decl, nested);
  apply_out_of_class_member_function_definitions(ctx, owner_decl, nested, owner_arguments);
  apply_out_of_class_special_member_definitions(ctx, owner_decl, nested, owner_arguments);
  apply_out_of_class_static_member_definitions(ctx, owner_decl, nested, owner_arguments);

  if(!emit_track_instantiation) {
    return;
  }
  if(!nested.complete || nested.template_instantiation_log_emitted) {
    return;
  }
  if(!nested.template_instantiation_tracked) {
    ctx.track_instantiated_class(&nested);
  }
  if(nested.template_instantiation_log_emitted) {
    return;
  }

  const std::string decl_location =
      nested_member_class_decl_location(ctx, nested.class_node, nested.name);
  nested.template_instantiation_log_emitted = true;
  template_api::note_nested_member_class_track_instantiation(ctx,
                                                             nested,
                                                             decl_location);
}

void finalize_direct_nested_member_classes(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    ClassInfo & info,
    const std::vector<TemplateArgument> & arguments)
{
  if(!info.member_scope) {
    return;
  }

  std::vector<ClassInfo *> nested_classes;
  for(auto it =
          info.member_scope->named_types.begin();
      it != info.member_scope->named_types.end();
      ++it) {
    ClassInfo * nested = ctx.class_info_for_type(it->second);
    if(!nested ||
       nested == &info ||
       nested->source_template ||
       nested->enclosing_scope != info.member_scope.get()) {
      continue;
    }
    if(std::find(nested_classes.begin(), nested_classes.end(), nested) ==
       nested_classes.end()) {
      nested_classes.push_back(nested);
    }
  }

  for(std::size_t i = 0; i < nested_classes.size(); ++i) {
    finalize_nested_member_class_instantiation_impl(
        ctx, decl, *nested_classes[i], arguments, false);
  }
}

}  // namespace

std::string template_argument_key_for_instantiation(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments)
{
  return template_argument_key_for_instantiation_impl(ctx, arguments);
}

std::vector<std::string> canonical_instantiation_arg_texts(
    SemanticContext & ctx,
    const std::vector<TemplateArgument> & arguments)
{
  return canonical_instantiation_arg_texts_impl(ctx, arguments);
}

std::string specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<TemplateArgument> & arguments)
{
  return specialization_name_for_instantiation_impl(ctx, name, arguments);
}

std::string display_specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<TemplateArgument> & arguments)
{
  return display_specialization_name_for_instantiation_impl(ctx, name, arguments);
}

// template-boundary-audit: begin canonical_key_metadata
bool owner_prefixed_instantiation_key_matches(
    const std::string & owner_prefixed_key,
    const std::string & materialized_key)
{
  if(owner_prefixed_key.empty()) {
    return false;
  }
  const std::size_t split = owner_prefixed_key.find("||");
  if(split == std::string::npos) {
    return false;
  }
  return owner_prefixed_key.substr(split + 2) == materialized_key;
}

bool function_binding_matches_materialized_owner_template_identity(
    const FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key)
{
  const auto same_owner_identity =
      [](const ClassInfo * lhs, const ClassInfo * rhs) -> bool
      {
        return lhs && rhs &&
               (lhs == rhs || lhs->qualified_name == rhs->qualified_name);
      };

  if(!binding.owner_class ||
     !binding.source_template ||
     !source_template ||
     binding.source_template == source_template ||
     binding.template_instantiation_key.empty() ||
     !binding.source_template->declaring_scope ||
     !source_template->declaring_scope ||
     !binding.source_template->declaring_scope->class_info ||
     !source_template->declaring_scope->class_info ||
     binding.source_template->name != source_template->name) {
    return false;
  }

  ClassInfo * binding_source_owner =
      binding.source_template->declaring_scope->class_info;
  ClassInfo * incoming_source_owner =
      source_template->declaring_scope->class_info;
  const bool binding_materialized_owner =
      same_owner_identity(binding_source_owner, binding.owner_class);
  const bool incoming_materialized_owner =
      same_owner_identity(incoming_source_owner, binding.owner_class);
  if(binding_materialized_owner == incoming_materialized_owner) {
    return false;
  }

  return binding.template_instantiation_key == instantiation_key ||
         owner_prefixed_instantiation_key_matches(binding.template_instantiation_key,
                                                  instantiation_key) ||
         owner_prefixed_instantiation_key_matches(instantiation_key,
                                                  binding.template_instantiation_key);
}

bool function_binding_matches_instantiation_identity(
    const FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key)
{
  if(binding.source_template || source_template) {
    return (binding.source_template == source_template &&
            binding.template_instantiation_key == instantiation_key) ||
           function_binding_matches_materialized_owner_template_identity(
               binding,
               source_template,
               instantiation_key);
  }
  return true;
}

bool should_preserve_owner_prefixed_template_identity(
    const FunctionBinding & original,
    const FunctionBinding & materialized,
    bool types_equivalent)
{
  if(&original == &materialized ||
     !original.owner_class ||
     original.owner_class != materialized.owner_class ||
     original.display_name != materialized.display_name ||
     original.template_instantiation_key.empty() ||
     materialized.template_instantiation_key.empty() ||
     original.template_instantiation_key == materialized.template_instantiation_key ||
     !types_equivalent ||
     !original.source_template ||
     !materialized.source_template ||
     !original.source_template->declaring_scope ||
     !materialized.source_template->declaring_scope ||
     !original.source_template->declaring_scope->class_info ||
     !materialized.source_template->declaring_scope->class_info) {
    return false;
  }

  const bool same_object_symbol =
      !original.symbol.object_symbol.empty() &&
      original.symbol.object_symbol == materialized.symbol.object_symbol;
  const bool owner_prefixed_match =
      owner_prefixed_instantiation_key_matches(original.template_instantiation_key,
                                               materialized.template_instantiation_key);
  if(!same_object_symbol && !owner_prefixed_match) {
    return false;
  }

  return original.source_template->declaring_scope->class_info != original.owner_class &&
         materialized.source_template->declaring_scope->class_info == original.owner_class;
}

std::string class_template_instance_key(SemanticContext & ctx,
                                        const ClassInfo & info)
{
  return class_instantiation_key_for_metadata(ctx, info);
}

bool class_template_instance_arguments_need_refresh(
    const ClassInfo & info,
    const std::vector<TemplateArgument> & arguments,
    bool was_dependent,
    bool dependent_arguments,
    bool preserve_existing_concrete_instantiation)
{
  if(preserve_existing_concrete_instantiation) {
    return false;
  }
  return (was_dependent && !dependent_arguments) ||
         info.instantiation_arguments.size() != arguments.size() ||
         info.instantiation_arg_texts.size() != arguments.size();
}

void record_class_template_dependent_argument_texts(
    ClassInfo & info,
    const std::vector<std::string> & argument_texts)
{
  info.instantiation_arg_texts = argument_texts;
}

void adopt_materialized_owner_template_identity(
    FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<TemplateArgument> * instantiation_arguments,
    Scope * declaration_scope)
{
  if(!function_binding_matches_materialized_owner_template_identity(binding,
                                                                    source_template,
                                                                    instantiation_key) ||
     !source_template ||
     !source_template->declaring_scope ||
     !source_template->declaring_scope->class_info ||
     source_template->declaring_scope->class_info != binding.owner_class) {
    return;
  }

  binding.source_template = source_template;
  binding.template_instantiation_key = instantiation_key;
  if(instantiation_arguments) {
    binding.instantiation_arguments = *instantiation_arguments;
    binding.has_instantiation_arguments = true;
  }
  if(declaration_scope) {
    binding.declaration_scope = declaration_scope;
  }
}

void adopt_function_template_identity_from_materialized(
    FunctionBinding & target,
    const FunctionBinding & materialized)
{
  target.source_template = materialized.source_template;
  target.template_instantiation_key = materialized.template_instantiation_key;
  target.instantiation_arguments = materialized.instantiation_arguments;
  target.has_instantiation_arguments = materialized.has_instantiation_arguments;
  if(materialized.declaration_scope) {
    target.declaration_scope = materialized.declaration_scope;
  }
}

void record_function_template_identity(
    FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<TemplateArgument> * instantiation_arguments)
{
  binding.source_template = source_template;
  binding.template_instantiation_key = instantiation_key;
  if(instantiation_arguments) {
    record_function_template_argument_state_impl(binding,
                                                 *instantiation_arguments,
                                                 binding.instantiation_pack_sizes.empty() ?
                                                     nullptr :
                                                     &binding.instantiation_pack_sizes,
                                                 true);
  }
}

bool record_class_template_instantiation_state(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments,
    bool is_explicit_specialization,
    bool suppress_implicit_instantiation_definition,
    bool dependent_arguments,
    const std::vector<std::string> * dependent_argument_texts,
    const std::vector<TemplateArgumentSyntax> * dependent_argument_syntaxes,
    const std::vector<TemplateParameterInfo> *
        dependent_argument_mangle_parameters,
    const std::vector<TemplateArgument> * dependent_argument_mangle_arguments,
    const std::map<std::string, std::size_t> * dependent_argument_mangle_pack_sizes)
{
  const bool existing_instantiation_metadata =
      !info.instantiation_key.empty() ||
      !info.instantiation_arguments.empty() ||
      !info.instantiation_arg_texts.empty();
  const bool was_dependent = info.dependent_instantiation;
  const bool preserve_existing_concrete_instantiation =
      existing_instantiation_metadata && !was_dependent && dependent_arguments;
  const bool refresh_arguments =
      class_template_instance_arguments_need_refresh(
          info,
          arguments,
          was_dependent,
          dependent_arguments,
          preserve_existing_concrete_instantiation);

  info.is_explicit_specialization = is_explicit_specialization;
  info.suppress_implicit_instantiation_definition =
      suppress_implicit_instantiation_definition;
  info.dependent_instantiation =
      existing_instantiation_metadata ?
          (was_dependent && dependent_arguments) :
          dependent_arguments;
  info.instantiation_key = key;
  info.instantiation_specialization_epoch =
      info.source_template ? info.source_template->specialization_epoch : 0;
  if(refresh_arguments) {
    info.instantiation_arg_texts =
        canonical_instantiation_arg_texts_impl(ctx, arguments);
    info.instantiation_arguments = arguments;
    clear_class_template_cached_lambda_mangle_metadata(info, arguments);
  }
  if(refresh_arguments || !info.has_instantiation_binding_arguments) {
    record_class_template_binding_state(info, arguments, nullptr);
  }
  if(dependent_argument_texts && !dependent_argument_texts->empty()) {
    if(info.instantiation_arg_texts.size() < arguments.size()) {
      info.instantiation_arg_texts =
          canonical_instantiation_arg_texts_impl(ctx, arguments);
    }
    const std::size_t count =
        std::min(std::min(dependent_argument_texts->size(),
                          info.instantiation_arg_texts.size()),
                 arguments.size());
    for(std::size_t i = 0; i < count; ++i) {
      const std::string & source_text = (*dependent_argument_texts)[i];
      info.instantiation_arg_texts[i] =
          dependent_argument_source_text_matches_semantic_argument(
              ctx,
              arguments[i],
              source_text) ?
              source_text :
              canonical_instantiation_arg_text_impl(ctx, arguments[i]);
      if(i < info.instantiation_arguments.size() &&
         info.instantiation_arguments[i].kind == TemplateArgument::TA_TYPE &&
         info.instantiation_arguments[i].dependent) {
        info.instantiation_arguments[i].text = info.instantiation_arg_texts[i];
      }
    }
  }
  if(dependent_argument_syntaxes) {
    if(info.instantiation_arguments.empty()) {
      info.instantiation_arguments = arguments;
    }
    const std::size_t count =
        std::min(dependent_argument_syntaxes->size(),
                 info.instantiation_arguments.size());
    for(std::size_t i = 0; i < count; ++i) {
      info.instantiation_arguments[i].source_syntax.reset(
          new TemplateArgumentSyntax((*dependent_argument_syntaxes)[i]));
    }
  }
  const std::vector<TemplateArgument> & effective_arguments =
      info.instantiation_arguments.empty() ? arguments : info.instantiation_arguments;
  update_class_template_specialization_mangle_info(
      info,
      effective_arguments,
      template_arguments_contain_forced_structured_mangling(effective_arguments),
      dependent_argument_syntaxes,
      dependent_argument_mangle_parameters,
      dependent_argument_mangle_arguments,
      dependent_argument_mangle_pack_sizes);
  update_class_template_dependent_type_metadata(
      ctx,
      info,
      effective_arguments,
      info.dependent_instantiation,
      dependent_argument_syntaxes);
  return refresh_arguments;
}

bool refresh_forward_class_template_selection(SemanticContext & ctx,
                                              ClassInfo & info)
{
  if(!info.source_template ||
     info.instantiation_arguments.empty() ||
     !info.template_output_node ||
     info.template_output_node->kind != CppAstKind::class_forward_declaration ||
     !info.member_scope) {
    return false;
  }

  const template_api::specialization::ClassSpecializationSelection specialization =
      template_api::specialization::select_class_specialization(
          ctx,
          *info.source_template,
          *info.member_scope,
          class_template_instance_key(ctx, info),
          info.instantiation_arguments);
  if(!specialization.class_node ||
     specialization.class_node == info.template_output_node) {
    return false;
  }

  ctx.reset_instantiated_class_info(info, info.name, specialization.class_node);
  info.is_explicit_specialization =
      specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION;
  record_class_template_binding_state(info,
                                      specialization.arguments,
                                      &specialization.pack_sizes);
  if(specialization.binding_scope) {
    bind_declaring_owner_instantiation_context(ctx,
                                               *info.member_scope,
                                               *specialization.binding_scope);
  }
  template_api::binding::bind_template_arguments_into_scope(
      ctx,
      *info.member_scope,
      *specialization.parameters,
      specialization.arguments,
      &specialization.pack_sizes);
  return true;
}

bool class_template_completion_has_owner_definition(const ClassInfo & info)
{
  return !info.source_template &&
         info.enclosing_scope &&
         info.enclosing_scope->class_info &&
         info.enclosing_scope->class_info->source_template;
}

ClassTemplateCompletionPlan class_template_completion_plan(const ClassInfo & info)
{
  ClassTemplateCompletionPlan plan;
  plan.in_progress = info.template_instantiation_in_progress;
  if(!info.source_template ||
     !info.template_output_node ||
     info.template_output_node->kind == CppAstKind::class_forward_declaration ||
     info.template_instantiation_in_progress) {
    return plan;
  }

  plan.ready = true;
  plan.origin = info.source_template;
  plan.output_node = info.template_output_node;
  plan.arguments = &info.instantiation_arguments;
  plan.trace_suffix = " [source template " + info.source_template->name + "]";
  return plan;
}

bool apply_out_of_class_member_function_abi_metadata(
    SemanticContext & ctx,
    ClassInfo & info)
{
  if(!info.source_template || !info.member_scope) {
    return false;
  }
  const std::vector<TemplateArgument> & arguments = info.instantiation_arguments;
  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(*info.source_template, info);
  if(!partial) {
    apply_stored_out_of_class_member_function_abi_metadata_map(
        ctx,
        info.source_template->member_function_definitions,
        info,
        arguments,
        true);
  } else {
    apply_stored_out_of_class_member_function_abi_metadata_map(
        ctx,
        partial->member_function_definitions,
        info,
        arguments,
        false);
  }
  return true;
}

bool apply_out_of_class_static_member_definitions_to_reference(
    SemanticContext & ctx,
    ClassInfo & info)
{
  if(info.complete ||
     !info.reference_members_collected ||
     info.is_explicit_specialization ||
     !info.source_template ||
     info.instantiation_arguments.empty()) {
    return false;
  }
  if(info.out_of_class_static_member_definitions_applied ||
     !info.member_scope) {
    return true;
  }

  ClassTemplateDecl & decl = *info.source_template;
  const auto id_expression_names_member =
      [](const CppAstNode & node, const std::string & member_name) -> bool
      {
        if(node.kind != CppAstKind::id_expression) {
          return false;
        }
        const QualifiedName * qualified = cppast_qualified_name_syntax(node);
        if(qualified && !qualified->name.empty()) {
          return qualified->name == member_name;
        }
        return node.value == member_name;
      };
  const std::function<bool(const CppAstNode &, const std::string &)>
      subtree_mentions_member =
      [&](const CppAstNode & node, const std::string & member_name) -> bool
      {
        if(id_expression_names_member(node, member_name)) {
          return true;
        }
        for(std::size_t i = 0; i < node.children.size(); ++i) {
          if(subtree_mentions_member(node.children[i], member_name)) {
            return true;
          }
        }
        return false;
      };
  const std::function<bool(const CppAstNode &, const std::string &)>
      member_function_bodies_mention_member =
      [&](const CppAstNode & node, const std::string & member_name) -> bool
      {
        if(node.kind == CppAstKind::function_definition ||
           node.kind == CppAstKind::special_member_definition) {
          const CppAstNode * body = find_child_kind(node, CppAstKind::compound_statement);
          if(body && subtree_mentions_member(*body, member_name)) {
            return true;
          }
          const CppAstNode * ctor_init = find_child_kind(node, CppAstKind::ctor_initializer);
          if(ctor_init && subtree_mentions_member(*ctor_init, member_name)) {
            return true;
          }
          return false;
        }
        for(std::size_t i = 0; i < node.children.size(); ++i) {
          if(member_function_bodies_mention_member(node.children[i], member_name)) {
            return true;
          }
        }
        return false;
      };

  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "apply-reference-out-of-class-static-members class="
          << info.qualified_name
          << " template=" << decl.name
          << " count=" << decl.static_member_definitions.size();
    parser_trace::note("template.resolve", std::string(), trace.str());
  }

  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(decl, info);
  const std::map<std::string, OutOfClassStaticMemberDecl> & static_member_definitions =
      partial ? partial->static_member_definitions : decl.static_member_definitions;
  bool has_concrete_storage_definition = false;
  for(std::map<std::string, OutOfClassStaticMemberDecl>::const_iterator it =
          static_member_definitions.begin();
      it != static_member_definitions.end();
      ++it) {
    ValueBinding * member =
        static_member_definition_binding_for_key(ctx, info, it->first);
    if(!member) {
      continue;
    }
    const CppAstNode * reference_source_node =
        info.template_output_node ? info.template_output_node : info.class_node;
    const std::string member_leaf_name =
        semantic_utils::unqualified_member_name(it->first);
    if(reference_source_node &&
       !member_function_bodies_mention_member(*reference_source_node,
                                              member_leaf_name)) {
      continue;
    }
    member->has_storage_definition = it->second.has_storage_definition;
    has_concrete_storage_definition =
        has_concrete_storage_definition ||
        (it->second.has_storage_definition && !info.dependent_instantiation);
    member->declaration_node = member->declaration_node ?
        member->declaration_node :
        it->second.declarator;
    member->definition_node = it->second.has_storage_definition ?
        it->second.declarator :
        member->definition_node;
    member->requires_constant_initializer =
        member->requires_constant_initializer ||
        (it->second.specifiers &&
         decl_spec_contains_token(*it->second.specifiers, KW_CONSTEXPR));
    if(!it->second.initializer) {
      continue;
    }

    member->constant_initializer = it->second.initializer;
    member->constant_initializer_scope = nullptr;
    clear_value_binding_constexpr_value(*member);
    member->has_constant_value = false;
    member->constant_value = 0;
    member->dependent_template_value = false;

    ClassInfo * member_owner = member->owner_class ? member->owner_class : &info;
    Scope * member_scope = member_owner->member_scope ?
        member_owner->member_scope.get() :
        info.member_scope.get();
    Scope & init_scope = ctx.append_template_scope(*member_scope);
    if(!partial) {
      bind_template_arguments_into_scope(
          ctx,
          init_scope,
          it->second.parameters,
          info.instantiation_arguments);
    }
    member->constant_initializer_scope = &init_scope;
  }
  info.out_of_class_static_member_definitions_applied = true;
  if(has_concrete_storage_definition) {
    ctx.track_instantiated_class(&info);
  }
  return true;
}

namespace {

class ScopedTemplateWitnessSourceCaptureResume
{
public:
  ScopedTemplateWitnessSourceCaptureResume()
    : saved_depth_(template_api::template_witness_detail::
                       current_source_capture_pause_depth_storage())
  {
    template_api::template_witness_detail::
        current_source_capture_pause_depth_storage() = 0;
  }

  ~ScopedTemplateWitnessSourceCaptureResume()
  {
    template_api::template_witness_detail::
        current_source_capture_pause_depth_storage() = saved_depth_;
  }

  ScopedTemplateWitnessSourceCaptureResume(
      const ScopedTemplateWitnessSourceCaptureResume &) = delete;
  ScopedTemplateWitnessSourceCaptureResume & operator=(
      const ScopedTemplateWitnessSourceCaptureResume &) = delete;

private:
  int saved_depth_;
};

void replay_witness_function_pointer_initializer(SemanticContext & ctx,
                                                 Scope & scope,
                                                 const TypePtr & target,
                                                 const CppAstNode * initializer)
{
  const CppAstNode * payload =
      callsemantic_internal::unwrap_initializer_payload(initializer);
  if(!payload) {
    return;
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  ClassInfo * aggregate = target_base ? ctx.complete_class_type(target_base) : nullptr;
  if(payload->kind == CppAstKind::braced_init_list && aggregate) {
    std::vector<const CppAstNode *> field_initializers;
    std::vector<CppAstNode> synthesized_nodes;
    if(!semantic_lifetime::build_aggregate_initializer_plan(ctx,
                                                            scope,
                                                            *aggregate,
                                                            *payload,
                                                            field_initializers,
                                                            synthesized_nodes)) {
      return;
    }
    const std::size_t count =
        std::min(aggregate->fields.size(), field_initializers.size());
    for(std::size_t i = 0; i < count; ++i) {
      replay_witness_function_pointer_initializer(ctx,
                                                  scope,
                                                  aggregate->fields[i].type,
                                                  field_initializers[i]);
    }
    return;
  }

  if(payload->kind != CppAstKind::unary_expression ||
     !node_has_simple_type(*payload, OP_AMP) ||
     payload->children.size() != 1 ||
     payload->children[0].kind != CppAstKind::id_expression) {
    return;
  }

  const CppAstNode & name_node = payload->children[0];
  semantic_conversion::ExprInfo ignored;
  const ScopedTemplateUseLocation use_location_guard(
      ctx.source_location_for_node(name_node));
  try {
    (void)semantic_overload::resolve_function_id_for_target(
        ctx,
        scope,
        name_node.value,
        target,
        ignored,
        cppast_qualified_name_syntax(name_node),
        cppast_template_id_syntax(name_node),
        &name_node,
        false);
  } catch(const std::exception &) {
  }
}

}  // namespace

bool replay_witness_static_member_definition_if_needed(
    SemanticContext & ctx,
    const ValueBinding & binding,
    const ClassInfo * owner_override)
{
  ClassInfo * owner = binding.owner_class ?
      binding.owner_class :
      const_cast<ClassInfo *>(owner_override);
  if(ctx.template_witness_context().session == nullptr ||
     !owner ||
     !owner->source_template ||
     !owner->member_scope) {
    return false;
  }

  ClassInfo & info = *owner;
  ClassTemplateDecl & decl = *info.source_template;
  const PartialClassTemplateSpecializationDecl * partial =
      selected_partial_specialization(decl, info);
  const std::map<std::string, OutOfClassStaticMemberDecl> & static_member_definitions =
      partial ?
          (!partial->witness_static_member_definitions.empty() ?
               partial->witness_static_member_definitions :
               partial->static_member_definitions) :
          (!decl.witness_static_member_definitions.empty() ?
               decl.witness_static_member_definitions :
               decl.static_member_definitions);
  std::map<std::string, OutOfClassStaticMemberDecl>::const_iterator found =
      static_member_definitions.find(binding.name);
  if(found == static_member_definitions.end()) {
    return false;
  }

  const OutOfClassStaticMemberDecl & static_member = found->second;
  {
    const ScopedTemplateWitnessSourceCaptureResume source_capture_resume;
    note_out_of_class_owner_class_use_for_applied_definition(
        ctx,
        &decl,
        info,
        static_member.node ? static_member.node : static_member.declarator,
        true);
  }
  if(!static_member.initializer) {
    return true;
  }

  Scope & init_scope = ctx.append_template_scope(*info.member_scope);
  if(!partial) {
    bind_template_arguments_into_scope(ctx,
                                       init_scope,
                                       static_member.parameters,
                                       info.instantiation_arguments);
  }
  replay_witness_function_pointer_initializer(ctx,
                                              init_scope,
                                              binding.type,
                                              static_member.initializer);
  return true;
}

bool class_template_use_info_for_type(SemanticContext & ctx,
                                      Scope & scope,
                                      const TypePtr & type,
                                      ClassTemplateUseInfo & out,
                                      bool select_specialization)
{
  out = ClassTemplateUseInfo();
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  while(base && base->kind == Type::TK_ARRAY) {
    base = strip_top_level_cv(base->inner);
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info || ctx.type_depends_on_template_parameter(type)) {
    return false;
  }
  return template_instantiation::class_template_use_info_for_class(
      ctx, scope, info, out, select_specialization);
}

bool class_template_use_info_for_class(SemanticContext & ctx,
                                       Scope & scope,
                                       ClassInfo * info,
                                       ClassTemplateUseInfo & out,
                                       bool select_specialization)
{
  out = ClassTemplateUseInfo();
  const bool has_instantiation_metadata =
      info &&
      (info->template_instantiation_tracked ||
       !info->instantiation_key.empty() ||
       !info->instantiation_arguments.empty() ||
       !info->instantiation_arg_texts.empty());
  if(!(info && info->source_template && has_instantiation_metadata)) {
    return false;
  }
  out.instance = info;
  out.origin = info->source_template;
  out.parameters = &info->source_template->parameters;
  out.arguments = &info->instantiation_arguments;
  out.canonical_texts = &info->instantiation_arg_texts;
  out.explicit_case = info->is_explicit_specialization;
  out.has_stored_key = !info->instantiation_key.empty();
  out.selects_specialized_definition =
      info->template_output_node &&
      info->source_template->class_node &&
      info->template_output_node != info->source_template->class_node;
  out.dependent_arguments =
      template_arguments_are_dependent_for_instantiation(
          ctx, info->instantiation_arguments);
  out.key = class_template_instance_key(ctx, *info);
  out.template_name =
      !info->qualified_name.empty() ?
          semantic_utils::strip_trailing_top_level_template_arguments(
              info->qualified_name) :
          info->source_template->name;
  out.unqualified_name =
      semantic_utils::unqualified_member_name(out.template_name).empty() ?
          out.template_name :
          semantic_utils::unqualified_member_name(out.template_name);
  if(select_specialization) {
    out.selection =
        template_api::specialization::select_class_specialization(
            ctx,
            *out.origin,
            scope,
            out.key,
            *out.arguments);
    out.has_selection = true;
  }
  return true;
}

bool template_id_matches_class_template_origin(
    const QualifiedName & template_id,
    const ClassTemplateUseInfo & info)
{
  if(!info.origin || template_id.name != info.origin->name) {
    return false;
  }
  if(!template_id.rooted && template_id.qualifiers.empty()) {
    return true;
  }
  if(!info.origin->declaring_scope) {
    return false;
  }

  const QualifiedName declared_name =
      semantic_lookup::scope_qualified_name_syntax(*info.origin->declaring_scope,
                                                   info.origin->name);

  return template_id.rooted == declared_name.rooted &&
         template_id.qualifiers == declared_name.qualifiers;
}

void append_class_template_type_arguments(
    const ClassInfo * info,
    std::vector<TypePtr> & out)
{
  if(!info) {
    return;
  }
  for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
    if(info->instantiation_arguments[i].kind != TemplateArgument::TA_TYPE ||
       !info->instantiation_arguments[i].type) {
      continue;
    }
    out.push_back(info->instantiation_arguments[i].type);
  }
}

bool class_template_instantiation_depends_on_template_parameter(
    SemanticContext & ctx,
    const ClassInfo & info)
{
  (void)ctx;
  return info.dependent_instantiation;
}

void record_function_template_argument_state(
    FunctionBinding & binding,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    bool mark_has_arguments)
{
  record_function_template_argument_state_impl(binding,
                                               arguments,
                                               pack_sizes,
                                               mark_has_arguments);
}

void record_function_template_arguments_preserving_pack_sizes(
    FunctionBinding & binding,
    const std::vector<TemplateArgument> & arguments,
    bool mark_has_arguments)
{
  const std::map<std::string, std::size_t> * pack_sizes =
      binding.instantiation_pack_sizes.empty() ?
          nullptr :
          &binding.instantiation_pack_sizes;
  record_function_template_argument_state_impl(binding,
                                               arguments,
                                               pack_sizes,
                                               mark_has_arguments);
}
// template-boundary-audit: end canonical_key_metadata

void finalize_nested_member_class_instantiation(
    SemanticContext & ctx,
    ClassTemplateDecl & owner_decl,
    ClassInfo & nested_info,
    const std::vector<TemplateArgument> & owner_arguments,
    bool emit_track_instantiation)
{
  finalize_nested_member_class_instantiation_impl(
      ctx,
      owner_decl,
      nested_info,
      owner_arguments,
      emit_track_instantiation);
}

void finalize_instantiated_class(SemanticContext & ctx,
                                 ClassTemplateDecl & decl,
                                 ClassInfo & info,
                                 const std::vector<TemplateArgument> & arguments)
{
  if(!info.is_explicit_specialization) {
    apply_out_of_class_member_function_template_definitions(ctx, decl, info);
    apply_out_of_class_member_function_definitions(ctx, decl, info, arguments);
    apply_selected_specialization_member_function_definitions(ctx, decl, info);
    apply_out_of_class_special_member_definitions(ctx, decl, info, arguments);
    apply_out_of_class_static_member_definitions(ctx, decl, info, arguments);
    finalize_direct_nested_member_classes(ctx, decl, info, arguments);
  }
  semantic_class_model::finalize_class_constant_members(ctx, info);
}

void overlay_instantiation_use_scope_bindings(Scope & target,
                                              const Scope & use_scope,
                                              const Scope * declaring_scope)
{
  template_scope::overlay_ancestor_scope_bindings(target,
                                                  use_scope,
                                                  declaring_scope,
                                                  template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
}

void overlay_instantiation_use_scope_bindings(
    Scope & target,
    const Scope & use_scope,
    const Scope * declaring_scope,
    const std::set<std::string> & excluded_names)
{
  template_scope::overlay_ancestor_scope_bindings_excluding_names(
      target,
      use_scope,
      declaring_scope,
      template_scope::OVERLAY_TEMPLATE_BOUND_ONLY,
      excluded_names);
}

void overlay_instantiation_local_named_types(
    SemanticContext & ctx,
    Scope & target,
    const Scope & use_scope,
    const Scope * declaring_scope,
    const std::vector<TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names)
{
  const TemplateArgumentNameReferences argument_refs =
      collect_template_argument_name_references(arguments);
  if(argument_refs.identifiers.empty() &&
     !argument_refs.contains_function_local_marker) {
    return;
  }
  std::set<std::string> copied_names;
  for(const Scope * current = &use_scope; current && current != declaring_scope;
      current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(const auto & named : current->named_types) {
      const std::string & name = named.first;
      if(current->template_bound_type_names.count(name) != 0) {
        continue;
      }
      const bool mentions_bound_name = argument_refs.mentions(name);
      if(!mentions_bound_name &&
         !argument_refs.contains_function_local_marker) {
        continue;
      }
      ClassInfo * info = ctx.class_info_for_type(named.second);
      const std::string local_lookup_name = semantic_utils::strip_elaborated_type_prefix(
          semantic_utils::trim_space(reparseable_type_argument_text(named.second)));
      const bool function_local_info =
          info &&
          ((info->enclosing_scope && info->enclosing_scope->function != nullptr) ||
           local_lookup_name.find("__local_") != std::string::npos);
      const bool mentions_local_name =
          mentions_bound_name ||
          (!local_lookup_name.empty() &&
           argument_refs.mentions(local_lookup_name));
      if(!named.second ||
         !info ||
         !function_local_info ||
         !mentions_local_name) {
        continue;
      }
      std::vector<std::string> alias_names;
      if(!name.empty()) {
        alias_names.push_back(name);
      }
      if(!local_lookup_name.empty() && local_lookup_name != name) {
        alias_names.push_back(local_lookup_name);
      }
      for(std::size_t i = 0; i < alias_names.size(); ++i) {
        const std::string & alias_name = alias_names[i];
        if((excluded_names && excluded_names->count(alias_name) != 0) ||
           copied_names.count(alias_name) != 0 ||
           target.named_types.count(alias_name) != 0) {
          continue;
        }
        template_scope::bind_named_type(target, alias_name, named.second);
        copied_names.insert(alias_name);
      }
    }
  }
}

std::set<std::string> collect_template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::set<std::string> names;
  for(const TemplateParameterInfo & parameter : parameters) {
    if(!parameter.name.empty()) {
      names.insert(parameter.name);
    }
    for(std::size_t i = 0; i < parameter.alternate_names.size(); ++i) {
      if(!parameter.alternate_names[i].empty()) {
        names.insert(parameter.alternate_names[i]);
      }
    }
  }
  return names;
}

std::set<std::string> collect_instantiation_overlay_excluded_names(
    const Scope & declaring_scope,
    const Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::set<std::string> names = collect_template_parameter_names(parameters);
  if(declaring_scope.class_info &&
     declaring_scope.class_info->source_template &&
     declaring_scope.class_info->member_scope.get() != &use_scope) {
    const std::set<std::string> class_parameter_names =
        collect_template_parameter_names(
            declaring_scope.class_info->source_template->parameters);
    names.insert(class_parameter_names.begin(), class_parameter_names.end());
  }
  return names;
}

ClassInfo * current_instantiation_owner_for_scope(SemanticContext & ctx,
                                                  Scope & declaring_scope,
                                                  Scope & use_scope,
                                                  ClassInfo * active_owner)
{
  ClassInfo * declared_owner = declaring_scope.class_info;
  if(!declared_owner) {
    return nullptr;
  }

  const auto candidate_matches_declared_owner =
      [declared_owner](const ClassInfo * candidate) -> bool
      {
        if(!candidate ||
           candidate == declared_owner ||
           candidate->name != declared_owner->name) {
          return false;
        }
        if(declared_owner->source_template) {
          if(candidate->source_template != declared_owner->source_template) {
            return false;
          }
          if(!declared_owner->instantiation_key.empty()) {
            return candidate->instantiation_key == declared_owner->instantiation_key;
          }
          return true;
        }
        return candidate->source_template != nullptr;
      };

  if(active_owner &&
     candidate_matches_declared_owner(active_owner)) {
    return active_owner;
  }

  for(Scope * current = &use_scope; current; current = current->parent) {
    if(!current->class_info) {
      continue;
    }
    if(current->class_info == declared_owner) {
      return nullptr;
    }
    if(candidate_matches_declared_owner(current->class_info)) {
      return current->class_info;
    }
  }

  ClassInfo * current_instantiation =
      lookup_declared_owner_class_via_leaf_type_lookup(
          ctx, use_scope, declared_owner->qualified_name);
  if(!current_instantiation) {
    current_instantiation = lookup_declared_owner_class_via_leaf_type_lookup(
        ctx, use_scope, declared_owner->name);
  }
  if(current_instantiation &&
     candidate_matches_declared_owner(current_instantiation)) {
    return current_instantiation;
  }

  if(candidate_matches_declared_owner(use_scope.class_info)) {
    return use_scope.class_info;
  }

  return nullptr;
}

void collect_argument_local_named_types(template_api::TemplateTypeSystem & type_system,
                                        const TypePtr & type,
                                        std::set<std::string> & seen,
                                        std::set<std::string> & visiting_named_keys,
                                        std::vector<std::pair<std::string, TypePtr> > & out)
{
  if(!type) {
    return;
  }

  switch(type->kind) {
  case Type::TK_NAMED:
  {
    if(!type->named_key.empty() &&
       !visiting_named_keys.insert(type->named_key).second) {
      return;
    }
    ClassInfo * info = template_api::find_named_type_class_info(type_system.model, type);
    if(!info || !info->type) {
      return;
    }
    const std::string lookup_name = semantic_utils::strip_elaborated_type_prefix(
        semantic_utils::trim_space(reparseable_type_argument_text(info->type)));
    const bool function_local_info =
        (info->enclosing_scope && info->enclosing_scope->function != nullptr) ||
        lookup_name.find("__local_") != std::string::npos;
    const std::string seen_key =
        !type->named_key.empty() ? type->named_key :
        semantic_utils::trim_space(reparseable_type_argument_text(info->type));
    if(function_local_info && seen.insert(seen_key).second) {
      if(!info->name.empty()) {
        out.push_back(std::make_pair(info->name, info->type));
      }
      if(!lookup_name.empty() && lookup_name != info->name) {
        out.push_back(std::make_pair(lookup_name, info->type));
      }
    }
    for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
      if(info->instantiation_arguments[i].kind != TemplateArgument::TA_TYPE ||
         !info->instantiation_arguments[i].type) {
        continue;
      }
      collect_argument_local_named_types(type_system,
                                         info->instantiation_arguments[i].type,
                                         seen,
                                         visiting_named_keys,
                                         out);
    }
    return;
  }

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    collect_argument_local_named_types(type_system, type->inner, seen, visiting_named_keys, out);
    return;

  case Type::TK_MEMBER_POINTER:
    collect_argument_local_named_types(type_system, type->owner, seen, visiting_named_keys, out);
    collect_argument_local_named_types(type_system, type->inner, seen, visiting_named_keys, out);
    return;

  case Type::TK_FUNCTION:
    collect_argument_local_named_types(type_system, type->inner, seen, visiting_named_keys, out);
    for(std::size_t i = 0; i < type->params.size(); ++i) {
      collect_argument_local_named_types(type_system, type->params[i], seen, visiting_named_keys, out);
    }
    return;

  default:
    return;
  }
}

void bind_argument_local_named_types(template_api::TemplateTypeSystem & type_system,
                                     Scope & scope,
                                     const std::vector<TemplateArgument> & arguments)
{
  std::set<std::string> seen;
  std::set<std::string> visiting_named_keys;
  std::vector<std::pair<std::string, TypePtr> > local_named_types;
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind != TemplateArgument::TA_TYPE || !arguments[i].type) {
      continue;
    }
    collect_argument_local_named_types(
        type_system, arguments[i].type, seen, visiting_named_keys, local_named_types);
  }
  for(std::size_t i = 0; i < local_named_types.size(); ++i) {
    if(local_named_types[i].first.empty() ||
       scope.named_types.count(local_named_types[i].first) != 0) {
      continue;
    }
    template_scope::bind_named_type(scope,
                                    local_named_types[i].first,
                                    local_named_types[i].second);
  }
}

void overlay_instantiation_local_named_types(
    template_api::TemplateServices & services,
    Scope & target,
    const Scope & use_scope,
    const Scope * declaring_scope,
    const std::vector<TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const Scope * scope = &use_scope;
  while(scope && scope != declaring_scope) {
    for(auto it = scope->named_types.begin();
        it != scope->named_types.end();
        ++it) {
      if(excluded_names && excluded_names->count(it->first) != 0) {
        continue;
      }
      if(!it->second) {
        continue;
      }
      const bool is_local_named_type =
          !it->second->named_key.empty() &&
          it->second->named_key.find("__local_") != std::string::npos;
      if(!is_local_named_type) {
        continue;
      }
      if(target.named_types.count(it->first) == 0) {
        template_scope::bind_named_type(target, it->first, it->second);
      }
    }
    scope = scope->parent;
  }
  bind_argument_local_named_types(type_system, target, arguments);
}

void bind_template_arguments_into_scope(SemanticContext & ctx,
                                        Scope & scope,
                                        const std::vector<TemplateParameterInfo> & parameters,
                                        const std::vector<TemplateArgument> & arguments,
                                        const std::map<std::string, std::size_t> * pack_sizes)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        ::template_instantiation::bind_template_arguments_into_scope(
            services, scope, parameters, arguments, pack_sizes);
      });
}

void bind_template_arguments_into_scope(
    template_api::TemplateServices & services,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  ensure_template_arguments_fully_bind_parameters(
      type_system,
      "bind_template_arguments_into_scope",
      scope.class_info ? scope.class_info->qualified_name : std::string(),
      parameters,
      arguments);
  template_binding::Hooks hooks = template_scope::make_scope_binding_hooks(scope);
  hooks.resolve_non_type_value_type =
      [&services, &type_system, &scope, &parameters, &arguments](
          const TemplateParameterInfo & parameter,
          const TemplateArgument &) -> TypePtr
      {
        TypePtr bound_value_type = parameter.value_type;
        if(bound_value_type &&
           template_argument_semantics::type_depends_on_template_parameter(
               type_system,
               bound_value_type)) {
          TypePtr substituted;
          if(template_argument_semantics::substitute_type(
                 bound_value_type, parameters, arguments, substituted)) {
            template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
                services,
                template_api::make_template_environment(scope),
                substituted);
            return substituted;
          }
        }
        return bound_value_type;
      };
  template_binding::bind_arguments(parameters, arguments, hooks, pack_sizes);
  bind_argument_local_named_types(type_system, scope, arguments);
}

struct ClassTemplateBindingContext
{
  const std::vector<TemplateParameterInfo> * parameters = nullptr;
  const std::vector<TemplateArgument> * arguments = nullptr;
  const std::map<std::string, std::size_t> * pack_sizes = nullptr;
};

bool class_template_binding_context(const ClassInfo & info,
                                    ClassTemplateBindingContext & out)
{
  if(!info.source_template || info.instantiation_arguments.empty()) {
    return false;
  }

  out.parameters = &info.source_template->parameters;
  out.arguments = &info.instantiation_arguments;
  out.pack_sizes = nullptr;

  if(info.has_instantiation_binding_arguments) {
    out.arguments = &info.instantiation_binding_arguments;
    out.pack_sizes = &info.instantiation_binding_pack_sizes;
  }

  if(const PartialClassTemplateSpecializationDecl * partial =
         selected_partial_specialization(*info.source_template, info)) {
    out.parameters = &partial->parameters;
  }

  return out.parameters && out.arguments;
}

void bind_active_owner_instantiation_context(SemanticContext & ctx,
                                             Scope & scope,
                                             const Scope & declaring_scope,
                                             ClassInfo & active_owner)
{
  ClassInfo * declared_owner = declaring_scope.class_info;
  if(!declared_owner ||
     declared_owner == &active_owner ||
     !declared_owner->source_template ||
     active_owner.source_template != declared_owner->source_template) {
    return;
  }

  ClassTemplateBindingContext binding;
  if(class_template_binding_context(active_owner, binding)) {
    bind_template_arguments_into_scope(ctx,
                                       scope,
                                       *binding.parameters,
                                       *binding.arguments,
                                       binding.pack_sizes);
  }

  if(!active_owner.member_scope) {
    return;
  }
  for(auto it =
          active_owner.member_scope->named_types.begin();
      it != active_owner.member_scope->named_types.end();
      ++it) {
    if(it->first.empty() ||
       !it->second ||
       declaring_scope.named_types.count(it->first) == 0) {
      continue;
    }
    template_scope::bind_named_type(scope, it->first, it->second);
  }
}

void bind_declaring_owner_instantiation_context(SemanticContext & ctx,
                                                Scope & scope,
                                                const Scope & declaring_scope)
{
  ClassInfo * declared_owner = declaring_scope.class_info;
  if(!declared_owner) {
    return;
  }

  ClassTemplateBindingContext binding;
  if(!class_template_binding_context(*declared_owner, binding)) {
    return;
  }

  bind_template_arguments_into_scope(ctx,
                                     scope,
                                     *binding.parameters,
                                     *binding.arguments,
                                     binding.pack_sizes);
}

Scope & bind_template_arguments(SemanticContext & ctx,
                                Scope & declaring_scope,
                                const std::vector<TemplateParameterInfo> & parameters,
                                const std::vector<TemplateArgument> & arguments,
                                const std::map<std::string, std::size_t> * pack_sizes)
{
  Scope & scope = ctx.append_template_scope(declaring_scope);
  bind_template_arguments_into_scope(ctx, scope, parameters, arguments, pack_sizes);
  return scope;
}

Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    Scope & declaring_scope,
    Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    ClassInfo * active_owner)
{
  Scope & scope = ctx.append_template_scope(declaring_scope);
  const std::set<std::string> excluded_names =
      collect_instantiation_overlay_excluded_names(
          declaring_scope, use_scope, parameters);
  if(excluded_names.empty()) {
    overlay_instantiation_use_scope_bindings(scope, use_scope, &declaring_scope);
  } else {
    overlay_instantiation_use_scope_bindings(
        scope, use_scope, &declaring_scope, excluded_names);
  }
  overlay_instantiation_local_named_types(
      ctx, scope, use_scope, &declaring_scope, arguments, &excluded_names);
  if(ClassInfo * current_owner =
         current_instantiation_owner_for_scope(
             ctx, declaring_scope, use_scope, active_owner)) {
    scope.class_info = current_owner;
    bind_active_owner_instantiation_context(ctx,
                                            scope,
                                            declaring_scope,
                                            *current_owner);
    if(!current_owner->name.empty()) {
      template_scope::bind_named_type(scope, current_owner->name, current_owner->type);
    }
  }
  bind_template_arguments_into_scope(ctx, scope, parameters, arguments, pack_sizes);
  return scope;
}

Scope & bind_class_template_arguments_for_instantiation(
    SemanticContext & ctx,
    Scope & declaring_scope,
    Scope & use_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  Scope & scope = ctx.append_template_scope(declaring_scope);
  const std::set<std::string> excluded_names =
      collect_template_parameter_names(parameters);
  // Class template bodies should not inherit unrelated caller template
  // bindings from the use-site; only the class template arguments and any
  // local named types needed by those arguments should flow in.
  overlay_instantiation_local_named_types(
      ctx, scope, use_scope, &declaring_scope, arguments, &excluded_names);
  bind_declaring_owner_instantiation_context(ctx, scope, declaring_scope);
  if(ClassInfo * current_owner =
         current_instantiation_owner_for_scope(
             ctx, declaring_scope, use_scope, nullptr)) {
    scope.class_info = current_owner;
    bind_active_owner_instantiation_context(ctx,
                                            scope,
                                            declaring_scope,
                                            *current_owner);
    if(!current_owner->name.empty()) {
      template_scope::bind_named_type(scope, current_owner->name, current_owner->type);
    }
  }
  bind_template_arguments_into_scope(ctx, scope, parameters, arguments, pack_sizes);
  return scope;
}

ClassInfo * instantiate_builtin_initializer_list_template(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<TemplateArgument> & arguments,
    const std::string & key,
    const std::string & requested_specialization_name,
    const std::string & internal_specialization_name)
{
  if(arguments.size() != 1 || arguments[0].kind != TemplateArgument::TA_TYPE) {
    throw std::logic_error("initializer_list requires one type argument");
  }

  auto found = decl.instantiations.find(key);
  const bool has_existing_instantiation = found != decl.instantiations.end();
  ClassInfo * info = nullptr;
  if(has_existing_instantiation) {
    info = found->second;
    if(info->creation_context.empty()) {
      template_audit::set_creation_context(
          *info, "instantiate_class_template [" + decl.name + "]");
    }
    record_class_template_argument_state(ctx, *info, key, arguments);
    info->is_initializer_list = true;
    info->initializer_list_element_type = arguments[0].type;
    if(info->complete) {
      decl.instantiations[key] = info;
      return info;
    }
  } else {
    Scope * inst_scope =
        &bind_class_template_arguments_for_instantiation(ctx,
                                                         *decl.declaring_scope,
                                                         use_scope,
                                                         decl.parameters,
                                                         arguments);
    ClassTemplateInfoCreationRequest create_request;
    create_request.scope = inst_scope;
    create_request.class_kind = "class";
    create_request.template_name = decl.name;
    create_request.specialization_name = requested_specialization_name;
    create_request.internal_specialization_name = internal_specialization_name;
    create_request.template_decl = &decl;
    create_request.output_node = decl.class_node;
    create_request.track_output = true;
    info = ctx.create_instantiated_class_info(create_request);
    template_audit::set_creation_context(
        *info, "instantiate_class_template [" + decl.name + "]");
  }
  if(!has_existing_instantiation) {
    record_class_template_argument_state(ctx, *info, key, arguments);
  }

  if(info->template_instantiation_in_progress) {
    return info;
  }

  ctx.reset_instantiated_class_info(*info, decl.name, decl.class_node);
  info->is_initializer_list = true;
  info->initializer_list_element_type = arguments[0].type;
  if(decl.declaring_scope) {
    bind_declaring_owner_instantiation_context(ctx,
                                               *info->member_scope,
                                               *decl.declaring_scope);
  }
  bind_template_arguments_into_scope(ctx, *info->member_scope, decl.parameters, arguments);

  if(decl.class_node == nullptr ||
     decl.class_node->kind == CppAstKind::class_forward_declaration) {
    FieldInfo begin_field;
    begin_field.name = "__begin";
    begin_field.type = make_pointer(arguments[0].type);
    begin_field.access = MA_PUBLIC;
    info->fields.push_back(begin_field);
    info->member_scope->values["__begin"] =
        ValueBinding(ValueBinding::VK_FIELD, "__begin", begin_field.type);
    info->member_scope->values["__begin"].owner_class = info;
    info->member_scope->values["__begin"].access = MA_PUBLIC;

    FieldInfo size_field;
    size_field.name = "__size";
    size_field.type = make_fundamental(FT_LONG_INT);
    size_field.access = MA_PUBLIC;
    info->fields.push_back(size_field);
    info->member_scope->values["__size"] =
        ValueBinding(ValueBinding::VK_FIELD, "__size", size_field.type);
    info->member_scope->values["__size"].owner_class = info;
    info->member_scope->values["__size"].access = MA_PUBLIC;

    ctx.finalize_class_virtuals(*info);
    ctx.finalize_class_layout(*info);
  } else {
    info->template_instantiation_in_progress = true;
    try {
      ctx.populate_class_info(*info, *decl.class_node);
    } catch(...) {
      info->template_instantiation_in_progress = false;
      throw;
    }
    info->template_instantiation_in_progress = false;
  }
  decl.instantiations[key] = info;
  return info;
}

ClassInfo * instantiate_class_template(SemanticContext & ctx,
                                       ClassTemplateDecl & decl,
                                       Scope & use_scope,
                                       const std::vector<TemplateArgument> & arguments)
{
  ensure_template_arguments_fully_bind_parameters(
      ctx,
      "instantiate_class_template",
      decl.name,
      decl.parameters,
      arguments);

  DIAG_CONTEXT("instantiate_class_template [" + decl.name +
               ", args=" + std::to_string(arguments.size()) + "]" +
               (decl.class_node ? ctx.source_location_for_node(*decl.class_node) : std::string()));
  const std::string key = template_argument_key_for_instantiation(ctx, arguments);
  if(ctx.is_builtin_initializer_list_template(decl)) {
    return instantiate_builtin_initializer_list_template(
        ctx,
        decl,
        use_scope,
        arguments,
        key,
        display_specialization_name_for_instantiation(ctx, decl.name, arguments),
        specialization_name_for_instantiation(ctx, decl.name, arguments));
  }

  const template_selection::ClassSpecializationSelection internal_specialization =
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            return template_selection::select_class_specialization(
                services,
                decl,
                template_api::make_template_environment(use_scope),
                key,
                arguments);
          });
  template_api::ClassSpecializationSelection specialization;
  specialization =
      template_api::to_api_class_specialization_selection(internal_specialization);
  return instantiate_selected_class_template(ctx, decl, use_scope, arguments, specialization);
}

ClassInfo * instantiate_selected_class_template(
    SemanticContext & ctx,
    ClassTemplateDecl & decl,
    Scope & use_scope,
    const std::vector<TemplateArgument> & arguments,
    const template_api::ClassSpecializationSelection & specialization)
{
  const std::string computed_key =
      specialization.selection_key.empty() ?
          template_argument_key_for_instantiation(ctx, arguments) :
          std::string();
  const std::string & key =
      specialization.selection_key.empty() ? computed_key : specialization.selection_key;
  const std::string instantiation_use_location =
      parser_trace::current_order_use_location();
  std::string requested_specialization_name;
  std::string internal_specialization_name;
  bool requested_specialization_name_ready = false;
  bool internal_specialization_name_ready = false;
  const auto ensure_requested_specialization_name = [&]() -> const std::string &
  {
    if(!requested_specialization_name_ready) {
      requested_specialization_name =
          display_specialization_name_for_instantiation(ctx, decl.name, arguments);
      requested_specialization_name_ready = true;
    }
    return requested_specialization_name;
  };
  const auto ensure_internal_specialization_name = [&]() -> const std::string &
  {
    if(!internal_specialization_name_ready) {
      internal_specialization_name =
          specialization_name_for_instantiation(ctx, decl.name, arguments);
      internal_specialization_name_ready = true;
    }
    return internal_specialization_name;
  };
  if(ctx.is_builtin_initializer_list_template(decl)) {
    return instantiate_builtin_initializer_list_template(
        ctx,
        decl,
        use_scope,
        arguments,
        key,
        ensure_requested_specialization_name(),
        ensure_internal_specialization_name());
  }
  const bool current_arguments_dependent =
      template_arguments_are_dependent_for_instantiation(ctx, arguments);
  const auto trace_class_instantiation = [&](const char * stage,
                                             ClassInfo * info,
                                             const char * reason = nullptr)
  {
    if(!parser_trace::enabled("template.resolve")) {
      return;
    }
    std::ostringstream trace;
    trace << "class-instantiation stage=" << stage
          << " name=" << decl.name
          << " key=" << key
          << " requested=" << ensure_requested_specialization_name()
          << " decl_loc="
          << (decl.class_node ? ctx.source_location_for_node(*decl.class_node) :
                                std::string("<none>"))
          << " dependent-args="
          << (current_arguments_dependent ? "yes" : "no");
    if(info) {
      trace << " class=" << info->qualified_name
            << " complete=" << (info->complete ? "yes" : "no")
            << " ref_members=" << (info->reference_members_collected ? "yes" : "no")
            << " in_progress=" << (info->template_instantiation_in_progress ? "yes" : "no")
            << " tracked=" << (info->template_instantiation_tracked ? "yes" : "no");
      if(info->template_output_node) {
        trace << " output_loc=" << ctx.source_location_for_node(*info->template_output_node);
      }
    }
    if(reason && *reason) {
      trace << " reason=" << reason;
    }
    parser_trace::note("template.resolve", std::string(), trace.str());
  };
  const CppAstNode * class_node = specialization.class_node;
  Scope * binding_scope = specialization.binding_scope;
  const std::vector<TemplateParameterInfo> * bound_parameters = specialization.parameters;
  const std::vector<TemplateArgument> * bound_arguments = &specialization.arguments;
  const std::map<std::string, std::size_t> * bound_pack_sizes = &specialization.pack_sizes;

  const bool forward_only_selection =
      class_node->kind == CppAstKind::class_forward_declaration;

  auto found = decl.instantiations.find(key);
  if(found == decl.instantiations.end()) {
    auto reference_found =
        decl.reference_instantiations.find(key);
    if(reference_found != decl.reference_instantiations.end()) {
      if(reference_found->second) {
        if(reference_found->second->reference_member_collection_in_progress) {
          trace_class_instantiation("reuse-reference-in-progress",
                                    reference_found->second);
          return reference_found->second;
        }
        decl.instantiations[key] = reference_found->second;
        found = decl.instantiations.find(key);
      }
      decl.reference_instantiations.erase(reference_found);
    }
  }
  ClassInfo * info = nullptr;
  if(found != decl.instantiations.end()) {
    info = found->second;
    info->reentrant_primary_selection = specialization.reentrant_primary;
    if(!instantiation_use_location.empty()) {
      info->first_qualifier_use_location =
          prefer_earlier_source_location(info->first_qualifier_use_location,
                                         instantiation_use_location);
    }
    trace_class_instantiation("cache-hit", info);
    if(info->creation_context.empty()) {
      template_audit::set_creation_context(
          *info, "instantiate_class_template [" + decl.name + "]");
    }
    const bool self_recursive_reference_reuse =
        !info->template_instantiation_tracked &&
        use_scope.class_info == info;
    if(!forward_only_selection &&
       !self_recursive_reference_reuse &&
       !info->template_instantiation_tracked) {
      ctx.track_instantiated_class(info);
    }
    if(info->template_output_node != class_node) {
      trace_class_instantiation("reset", info, "template-output-node-mismatch");
      ctx.reset_instantiated_class_info(*info, decl.name, class_node);
    } else if(info->reference_members_collected && !info->complete) {
      if(info->dependent_instantiation && current_arguments_dependent) {
        ctx.finalize_dependent_class_shape(*info);
        trace_class_instantiation("reuse-dependent", info, "reference-members-only");
        return info;
      }
      trace_class_instantiation(
          "reset",
          info,
          info->dependent_instantiation ?
              "reference-members-collected-now-concrete" :
              "reference-members-collected-incomplete");
      ctx.reset_instantiated_class_info(*info, decl.name, class_node);
    } else if(info->template_instantiation_in_progress) {
      trace_class_instantiation("reuse-in-progress", info);
      return info;
    } else if(info->complete && info->dependent_instantiation &&
              !current_arguments_dependent) {
      trace_class_instantiation("reset", info, "dependent-complete-now-concrete");
      ctx.reset_instantiated_class_info(*info, decl.name, class_node);
    } else if(info->complete) {
      trace_class_instantiation("reuse-complete", info);
      return info;
    }
  } else {
    const CppAstNode * class_key =
        cpp_decl::find_child(*class_node, CppAstKind::class_key);
    if(!class_key) {
      throw std::logic_error("class template missing class-key");
    }
    Scope * inst_scope =
        &bind_class_template_arguments_for_instantiation(ctx,
                                                         *binding_scope,
                                                         use_scope,
                                                         *bound_parameters,
                                                         *bound_arguments,
                                                         bound_pack_sizes);
    ClassTemplateInfoCreationRequest create_request;
    create_request.scope = inst_scope;
    create_request.class_kind = node_text(*class_key);
    create_request.template_name = decl.name;
    create_request.specialization_name = ensure_requested_specialization_name();
    create_request.internal_specialization_name = ensure_internal_specialization_name();
    create_request.template_decl = &decl;
    create_request.output_node = class_node;
    create_request.track_output = !forward_only_selection;
    info = ctx.create_instantiated_class_info(create_request);
    info->reentrant_primary_selection = specialization.reentrant_primary;
    if(!instantiation_use_location.empty()) {
      info->first_qualifier_use_location =
          prefer_earlier_source_location(info->first_qualifier_use_location,
                                         instantiation_use_location);
    }
    decl.instantiations[key] = info;
    trace_class_instantiation("cache-miss", info);
    template_audit::set_creation_context(
        *info, "instantiate_class_template [" + decl.name + "]");
  }
  const bool selected_partial_mangle_context =
      bound_parameters &&
      bound_parameters != &decl.parameters &&
      bound_arguments;
  record_class_template_argument_state(
      ctx,
      *info,
      key,
      arguments,
      selected_partial_mangle_context ? bound_parameters : nullptr,
      selected_partial_mangle_context ? bound_arguments : nullptr,
      selected_partial_mangle_context ? bound_pack_sizes : nullptr);
  info->reentrant_primary_selection = specialization.reentrant_primary;
  record_class_template_binding_state(*info, *bound_arguments, bound_pack_sizes);
  info->dependent_instantiation = current_arguments_dependent;
  info->is_explicit_specialization =
      specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION;
  bind_declaring_owner_instantiation_context(ctx, *info->member_scope, *binding_scope);
  bind_template_arguments_into_scope(
      ctx, *info->member_scope, *bound_parameters, *bound_arguments, bound_pack_sizes);
  ctx.record_primary_alias_base_source_uses(decl);
  if(forward_only_selection) {
    trace_class_instantiation("forward-only", info);
    return info;
  }
  if(info->complete || info->template_instantiation_in_progress) {
    trace_class_instantiation(info->complete ? "reuse-complete" : "reuse-in-progress", info);
    return info;
  }

  info->template_instantiation_in_progress = true;
  const parser_trace::ScopedOrderUseLocation instantiation_use_location_guard(
      instantiation_use_location);
  trace_class_instantiation("populate-begin", info);
  try {
    semantic_trace::append_template_context(
        [&]()
        {
          ctx.populate_class_info(*info, *class_node);
        },
        [&]() -> std::string
        {
          return instantiated_class_context(ctx, *info, decl, arguments);
        });
    finalize_instantiated_class(ctx, decl, *info, arguments);
  } catch(const TemplateSubstitutionFailure &) {
    info->template_instantiation_in_progress = false;
    trace_class_instantiation("populate-fail", info, "template-substitution-failure");
    throw;
  } catch(const std::logic_error & e) {
    info->template_instantiation_in_progress = false;
    const std::string reason = std::string("logic-error:") + e.what();
    trace_class_instantiation("populate-fail", info, reason.c_str());
    throw;
  }
  info->template_instantiation_in_progress = false;
  trace_class_instantiation("populate-end", info);
  return info;
}

FunctionBinding * instantiate_function_template(SemanticContext & ctx,
                                                FunctionTemplateDecl & decl,
                                                const std::vector<TemplateArgument> & arguments,
                                                ClassInfo * active_owner,
                                                const CppAstNode * body_override,
                                                const CppAstNode * definition_node_override,
                                                bool explicit_specialization,
                                                bool explicit_specialization_is_constexpr,
                                                bool include_body,
                                                Scope * use_scope,
                                                const std::map<std::string, std::size_t> * pack_sizes,
                                                bool prefer_overload_suffix,
                                                const std::string & instantiation_use_location_override)
{
  const std::string diagnostic_context =
      "instantiate_function_template [" + decl.name +
      ", args=" + std::to_string(arguments.size()) + "]" +
      (decl.declarator ? ctx.source_location_for_node(*decl.declarator) :
                         (decl.body ? ctx.source_location_for_node(*decl.body)
                                    : std::string()));
  DiagnosticContext::Guard diagnostic_guard(diagnostic_context);
  const std::string instantiation_use_location =
      !instantiation_use_location_override.empty() ?
          instantiation_use_location_override :
          parser_trace::current_order_use_location();
  const parser_trace::ScopedOrderUseLocation instantiation_use_location_guard(
      instantiation_use_location);
  ensure_template_arguments_fully_bind_parameters(
      ctx,
      "instantiate_function_template",
      decl.name,
      decl.parameters,
      arguments);
  const auto ensure_result_type_pattern = [](FunctionTemplateDecl & target)
  {
    if(target.result_type_pattern.kind == CppAstKind::invalid &&
       target.specifiers &&
       target.declarator &&
       !target.is_constructor &&
       !target.is_destructor) {
      target.result_type_pattern =
          template_function_signature::build_function_result_type_pattern(
              *target.specifiers, *target.declarator);
    }
  };
  ensure_result_type_pattern(decl);
  const auto select_instantiation_owner = [&](Scope * scope) -> ClassInfo *
  {
    ClassInfo * declared_owner =
        decl.declaring_scope && decl.declaring_scope->class_info ?
            decl.declaring_scope->class_info :
            nullptr;
    if(!declared_owner || !scope) {
      return declared_owner;
    }

    if(active_owner) {
      if(active_owner == declared_owner) {
        return active_owner;
      }
      if(active_owner->name == declared_owner->name) {
        return active_owner;
      }
    }

    for(Scope * current = scope; current; current = current->parent) {
      ClassInfo * current_owner = current->class_info;
      if(!current_owner) {
        continue;
      }
      if(current_owner == declared_owner) {
        return current_owner;
      }
      if(current_owner->source_template &&
         current_owner->name == declared_owner->name) {
        return current_owner;
      }
    }

    ClassInfo * current_instantiation =
        lookup_declared_owner_class_via_leaf_type_lookup(
            ctx, *scope, declared_owner->qualified_name);
    if(!current_instantiation) {
      current_instantiation = lookup_declared_owner_class_via_leaf_type_lookup(
          ctx, *scope, declared_owner->name);
    }
    if(current_instantiation &&
       current_instantiation != declared_owner &&
       current_instantiation->name == declared_owner->name) {
      return current_instantiation;
    }
    if(!scope->class_info) {
      return declared_owner;
    }
    ClassInfo * active_owner = scope->class_info;
    if(active_owner == declared_owner) {
      return active_owner;
    }
    if(!active_owner->source_template) {
      return declared_owner;
    }
    if(active_owner->name != declared_owner->name) {
      return declared_owner;
    }
    return active_owner;
  };
  const auto effective_template_body = [&](FunctionTemplateDecl & source_decl) -> const CppAstNode *
  {
    if(source_decl.body) {
      return source_decl.body;
    }
    if(source_decl.inner) {
      return cpp_decl::find_child(*source_decl.inner, CppAstKind::compound_statement);
    }
    return nullptr;
  };
  const auto effective_function_qualifier = [&](FunctionTemplateDecl & source_decl)
      -> const CppAstNode *
  {
    if(source_decl.declarator) {
      return semantic_class_model::declarator_function_qualifier(*source_decl.declarator);
    }
    return nullptr;
  };
  std::string key = template_argument_key_for_instantiation(ctx, arguments);
  ClassInfo * instantiation_owner = select_instantiation_owner(use_scope);
  if(!instantiation_owner) {
    instantiation_owner = select_hidden_friend_instantiation_owner(ctx, decl, arguments);
  }
  FunctionTemplateDecl * source_decl =
      canonical_instantiation_template_decl(ctx, instantiation_owner, &decl);
  ensure_result_type_pattern(*source_decl);
  if(!instantiation_owner) {
    instantiation_owner =
        select_hidden_friend_instantiation_owner(ctx, *source_decl, arguments);
  }
  const bool effective_is_constexpr =
      explicit_specialization ?
          explicit_specialization_is_constexpr :
          source_decl->is_constexpr;
  trace_function_template_drift("instantiate-entry", *source_decl);
  ClassInfo * source_decl_context_owner =
      source_decl ? function_template_context_owner(*source_decl) : nullptr;
  if(instantiation_owner &&
     source_decl &&
     source_decl_context_owner &&
     instantiation_owner != source_decl_context_owner) {
    key = instantiation_owner->qualified_name + "||" + key;
  }
  if(instantiation_owner &&
     instantiation_owner->source_template &&
     source_decl &&
     source_decl->declaring_scope &&
     source_decl->declaring_scope->class_info &&
     !source_decl->body) {
    std::map<std::string,
             std::vector<OutOfClassMemberFunctionTemplateDefinition> >::const_iterator stored =
        instantiation_owner->source_template->member_function_template_definitions.find(
            source_decl->name);
    const PartialClassTemplateSpecializationDecl * partial =
        selected_partial_specialization(*instantiation_owner->source_template,
                                        *instantiation_owner);
    if(stored != instantiation_owner->source_template->member_function_template_definitions.end()) {
      maybe_apply_stored_out_of_class_member_function_template_definition(ctx,
                                                                          stored->second,
                                                                          *source_decl);
    }
    if(partial) {
      std::map<std::string,
               std::vector<OutOfClassMemberFunctionTemplateDefinition> >::const_iterator
          partial_stored = partial->member_function_template_definitions.find(source_decl->name);
      if(partial_stored != partial->member_function_template_definitions.end()) {
        maybe_apply_stored_out_of_class_member_function_template_definition(ctx,
                                                                            partial_stored->second,
                                                                            *source_decl);
      }
    }
    apply_out_of_class_member_function_template_definitions(ctx,
                                                            *instantiation_owner->source_template,
                                                            *instantiation_owner);
  }
  const auto refresh_pack_dependent_result_type =
      [&](FunctionTemplateDecl & pattern_decl,
          Scope & result_scope,
          FunctionBinding & binding,
          bool allow_non_pack_cache_refresh) -> void
  {
    const bool pattern_has_pack =
        template_parameters_have_pack(pattern_decl.parameters);
    if(pattern_decl.is_constructor ||
       pattern_decl.is_destructor ||
       pattern_decl.result_type_pattern.kind == CppAstKind::invalid ||
       (!pattern_has_pack &&
        (!allow_non_pack_cache_refresh ||
         !effective_template_body(pattern_decl) ||
         !type_id_has_top_level_enable_if_template_id(
             pattern_decl.result_type_pattern) ||
         type_id_has_top_level_decltype_specifier(
             pattern_decl.result_type_pattern))) ||
       !ast_mentions_template_parameter_name(pattern_decl.result_type_pattern,
                                             pattern_decl.parameters) ||
       !binding.type) {
      return;
    }

    TypePtr function_base = strip_top_level_cv(binding.type);
    if(!function_base || function_base->kind != Type::TK_FUNCTION) {
      return;
    }

    TypePtr parsed_result;
    const bool parsed_result_type =
        template_api::with_template_services(
            ctx,
            [&](template_api::TemplateServices & services)
            {
              const witness::ScopedTemplateWitnessSourceCapturePause
                  source_capture_pause;
              CppAstNode parse_pattern = pattern_decl.result_type_pattern;
              clear_cached_semantic_types(parse_pattern);
              CppAstNode substituted_pattern;
              if(template_argument_semantics::substitute_type_id_node_for_template_arguments(
                     services,
                     result_scope,
                     parse_pattern,
                     pattern_decl.parameters,
                     arguments,
                     substituted_pattern)) {
                parse_pattern = substituted_pattern;
              }
              clear_dependent_cached_semantic_types(parse_pattern,
                                                    pattern_decl.parameters);
              return template_decl_ast::parse_type_id(services,
                                                      result_scope,
                                                      result_scope,
                                                      parse_pattern,
                                                      parsed_result,
                                                      !include_body) &&
                     parsed_result;
            });
    if(!parsed_result_type || !parsed_result) {
      if(!pattern_has_pack &&
         !template_arguments_are_dependent_for_instantiation(ctx, arguments)) {
        throw_substitution_failure(
            "cached function template result type substitution failed",
            std::string(),
            "template-instantiation");
      }
      return;
    }

    TypePtr resolved_result;
    if(recover_instantiation_bound_type(ctx,
                                        result_scope,
                                        parsed_result,
                                        resolved_result) &&
       resolved_result) {
      parsed_result = resolved_result;
    }
    if(template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                       parsed_result) &&
       type_mentions_template_parameter_name(parsed_result, pattern_decl.parameters)) {
      return;
    }

    TypePtr refreshed_type = make_function(parsed_result,
                                           function_base->params,
                                           function_base->variadic,
                                           function_base->function_const,
                                           function_base->function_volatile,
                                           function_base->prototype_relaxed,
                                           function_base->function_ref_qualifier);
    if(!refreshed_type || type_equals(refreshed_type, binding.type)) {
      return;
    }
    binding.type = refreshed_type;
    binding.declared_type = refreshed_type;
    binding.cached_lookup_dedupe_key_valid = false;
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "function-instantiation-result-cache-refresh name="
            << pattern_decl.name
            << " key=" << key
            << " type=" << describe_type(parsed_result);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
  };
  const auto reject_cached_retained_dependent_function_type =
      [&](FunctionTemplateDecl & pattern_decl,
          FunctionBinding & binding) -> void
  {
    if(pattern_decl.is_constructor ||
       pattern_decl.is_destructor ||
       !template_parameters_have_pack(pattern_decl.parameters) ||
       !binding.type) {
      return;
    }
    TypePtr function_base = strip_top_level_cv(binding.type);
    if(!function_base || function_base->kind != Type::TK_FUNCTION) {
      return;
    }
    const bool result_dependent =
        template_argument_semantics::type_depends_on_template_parameter(
            ctx,
            function_base->inner);
    if(!result_dependent ||
       pattern_decl.result_type_pattern.kind == CppAstKind::invalid ||
       !ast_mentions_template_parameter_name(pattern_decl.result_type_pattern,
                                             pattern_decl.parameters) ||
       template_arguments_are_dependent_for_instantiation(ctx, arguments)) {
      return;
    }
    throw_substitution_failure(
        "cached function template retained dependent result type",
        std::string(),
        "template-instantiation");
  };
  std::map<std::string, FunctionBinding *>::iterator found =
      source_decl->instantiations.find(key);
  const std::map<std::string, std::size_t> * effective_pack_sizes = pack_sizes;
  if(!effective_pack_sizes &&
     found != source_decl->instantiations.end() &&
     !found->second->instantiation_pack_sizes.empty()) {
    effective_pack_sizes = &found->second->instantiation_pack_sizes;
  }
  if(found != source_decl->instantiations.end()) {
    FunctionTemplateDecl * cache_source_decl = source_decl;
    if(found->second &&
       found->second->source_template &&
       found->second->source_template != source_decl &&
       instantiation_owner &&
       found->second->source_template->declaring_scope &&
       found->second->source_template->declaring_scope->class_info == instantiation_owner) {
      cache_source_decl = found->second->source_template;
    }
    note_function_instantiation_use_location(*found->second,
                                             instantiation_use_location);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "function-instantiation-cache-hit name=" << source_decl->name
            << " parsed_name=" << found->second->display_name
            << " key=" << key
            << " source_template=" << static_cast<void *>(source_decl)
            << " is_ctor=" << (source_decl->is_constructor ? "yes" : "no")
            << " decl_loc=" << function_template_decl_location(ctx, source_decl);
      const std::string decl_details =
          semantic_trace::template_decl_location_details(ctx, source_decl);
      if(!decl_details.empty()) {
        trace << " decl_loc_detail=" << decl_details;
      }
      trace
            << " declared_owner="
            << (source_decl->declaring_scope && source_decl->declaring_scope->class_info ?
                    source_decl->declaring_scope->class_info->qualified_name :
                    std::string("<none>"))
            << " active_owner="
            << (instantiation_owner ? instantiation_owner->qualified_name : std::string("<none>"))
            << " cache_scope_owner="
            << (cache_source_decl && cache_source_decl->declaring_scope &&
                cache_source_decl->declaring_scope->class_info ?
                    cache_source_decl->declaring_scope->class_info->qualified_name :
                    std::string("<none>"))
            << " include_body=" << (include_body ? "yes" : "no")
            << " source_has_body=" << (effective_template_body(*source_decl) ? "yes" : "no")
            << " source_params=" << function_template_params_for_trace(*source_decl)
            << " ctor_init_decl=" << (source_decl->ctor_initializer ? "yes" : "no")
            << " ctor_init_binding=" << (found->second->ctor_initializer ? "yes" : "no")
            << " binding_suppress_implicit="
            << (found->second->suppress_implicit_instantiation_definition ? "yes" : "no")
            << " owner_suppress_implicit="
            << (found->second->owner_class &&
                found->second->owner_class->suppress_implicit_instantiation_definition ?
                    "yes" :
                    "no")
            << " has_definition=" << (found->second->has_definition ? "yes" : "no");
      append_function_binding_trace_identity(trace, ctx, found->second);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    record_function_template_argument_state(*found->second,
                                            arguments,
                                            effective_pack_sizes,
                                            false);
    Scope * cache_instantiation_context_scope =
        function_template_instantiation_context_scope(*cache_source_decl);
  Scope * owner_member_instantiation_scope =
        instantiation_owner && instantiation_owner->member_scope ?
            instantiation_owner->member_scope.get() :
            nullptr;
    const bool use_owner_scope_for_member_template =
        cache_instantiation_context_scope &&
        cache_instantiation_context_scope->class_info &&
        owner_member_instantiation_scope;
    const bool member_template_decl_already_in_active_owner =
        use_owner_scope_for_member_template &&
        cache_instantiation_context_scope->class_info == instantiation_owner;
    Scope * refreshed_instantiation_scope = nullptr;
    if(use_scope) {
      Scope & refreshed_scope =
          member_template_decl_already_in_active_owner ?
              bind_template_arguments(ctx,
                                      *cache_instantiation_context_scope,
                                      cache_source_decl->parameters,
                                      arguments,
                                      effective_pack_sizes) :
          (use_owner_scope_for_member_template ?
               bind_template_arguments_for_instantiation(ctx,
                                                        *cache_instantiation_context_scope,
                                                       *owner_member_instantiation_scope,
                                                       cache_source_decl->parameters,
                                                       arguments,
                                                       effective_pack_sizes,
                                                       instantiation_owner) :
               bind_template_arguments_for_instantiation(ctx,
                                                        *cache_instantiation_context_scope,
                                                        *use_scope,
                                                        cache_source_decl->parameters,
                                                        arguments,
                                                        effective_pack_sizes,
                                                        instantiation_owner));
      if(use_owner_scope_for_member_template) {
        refreshed_scope.class_info = instantiation_owner;
        bind_active_owner_instantiation_context(ctx,
                                                refreshed_scope,
                                                *cache_instantiation_context_scope,
                                                *instantiation_owner);
      }
      if(use_owner_scope_for_member_template &&
         use_scope != owner_member_instantiation_scope) {
        const std::set<std::string> excluded_names =
            collect_template_parameter_names(cache_source_decl->parameters);
        overlay_instantiation_local_named_types(ctx,
                                                refreshed_scope,
                                                *use_scope,
                                                cache_instantiation_context_scope,
                                                arguments,
                                                &excluded_names);
      }
      found->second->declaration_scope = &refreshed_scope;
      refreshed_instantiation_scope = &refreshed_scope;
    }
    if(found->second->source_template &&
       found->second->symbol.linkage == symbol_linkage::SL_WEAK) {
      ctx.upgrade_function_symbol_linkage(found->second, found->second->symbol.linkage);
    }
    const bool suppressed_by_owner =
        found->second->owner_class &&
        found->second->owner_class->suppress_implicit_instantiation_definition &&
        !(found->second->source_template &&
          !found->second->suppress_implicit_instantiation_definition);
    bool definition_materialized_from_source_template = false;
    if(explicit_specialization) {
      found->second->body = body_override;
      found->second->has_definition = body_override != nullptr;
      if(body_override) {
        found->second->definition_node =
            definition_node_override ? definition_node_override : body_override;
      }
      found->second->is_constexpr = found->second->is_constexpr || effective_is_constexpr;
      found->second->definition_suppresses_declaration_abi_tags = false;
    } else if(include_body &&
              effective_template_body(*cache_source_decl) &&
              !found->second->suppress_implicit_instantiation_definition &&
              !suppressed_by_owner &&
              !found->second->has_definition) {
      found->second->body = effective_template_body(*cache_source_decl);
      found->second->has_definition = true;
      if(cache_source_decl->definition_declarator) {
        found->second->definition_node = cache_source_decl->definition_declarator;
      } else if(cache_source_decl->definition_node) {
        found->second->definition_node = cache_source_decl->definition_node;
      } else {
        found->second->definition_node = found->second->body;
      }
      found->second->definition_abi_tags =
          function_template_definition_abi_tags(*cache_source_decl);
      found->second->definition_suppresses_declaration_abi_tags =
          function_template_definition_should_suppress_declaration_abi_tags(ctx,
                                                                            *cache_source_decl);
      definition_materialized_from_source_template = true;
    } else if(include_body &&
              cache_source_decl &&
              cache_source_decl->is_inherited_constructor &&
              found->second->is_constructor &&
              !found->second->suppress_implicit_instantiation_definition &&
              !suppressed_by_owner &&
              !found->second->has_definition) {
      if(!found->second->ctor_initializer && cache_source_decl->ctor_initializer) {
        found->second->ctor_initializer = cache_source_decl->ctor_initializer;
      }
      if(found->second->ctor_initializer) {
        found->second->has_definition = true;
        if(!found->second->definition_node) {
          found->second->definition_node =
              cache_source_decl->definition_node ?
                  cache_source_decl->definition_node :
                  (cache_source_decl->declaration_node ?
                       cache_source_decl->declaration_node :
                       found->second->declaration_node);
        }
        definition_materialized_from_source_template = true;
      }
    }
    if(!found->second->function_qualifier) {
      found->second->function_qualifier = effective_function_qualifier(*cache_source_decl);
    }
    if(include_body &&
       refreshed_instantiation_scope &&
       cache_source_decl->is_inherited_constructor &&
       found->second->is_constructor) {
      refresh_instantiated_inherited_constructor_initializer(ctx,
                                                            *refreshed_instantiation_scope,
                                                            *cache_source_decl,
                                                            *found->second);
    }
    if(definition_materialized_from_source_template &&
       found->second->source_template &&
       found->second->symbol.linkage == symbol_linkage::SL_WEAK) {
      ctx.upgrade_function_symbol_linkage(found->second,
                                          found->second->symbol.linkage);
    }
    apply_instantiated_parameter_aliases(
        *found->second,
        instantiate_function_parameter_aliases(*cache_source_decl,
                                               binding_explicit_params(*found->second)));
    if(refreshed_instantiation_scope) {
      refresh_pack_dependent_result_type(*cache_source_decl,
                                         *refreshed_instantiation_scope,
                                         *found->second,
                                         !include_body);
    }
    if(!include_body && !explicit_specialization) {
      reject_cached_retained_dependent_function_type(*cache_source_decl,
                                                    *found->second);
    }
    if((include_body || explicit_specialization) &&
       (found->second->has_definition || explicit_specialization)) {
      apply_function_instantiation_intent(
          ctx, found->second, InstantiatedFunctionOutputMode::TrackOnly);
    }
    return found->second;
  }

  Scope * owner_member_instantiation_scope =
      instantiation_owner && instantiation_owner->member_scope ?
          instantiation_owner->member_scope.get() :
          nullptr;
  Scope * instantiation_context_scope =
      function_template_instantiation_context_scope(*source_decl);
  const bool use_owner_scope_for_member_template =
      instantiation_context_scope &&
      instantiation_context_scope->class_info &&
      owner_member_instantiation_scope;
  const bool member_template_decl_already_in_active_owner =
      use_owner_scope_for_member_template &&
      instantiation_context_scope->class_info == instantiation_owner;
  Scope & inst_scope =
      use_scope ?
          (member_template_decl_already_in_active_owner ?
               bind_template_arguments(ctx,
                                       *instantiation_context_scope,
                                       source_decl->parameters,
                                       arguments,
                                       effective_pack_sizes) :
           (use_owner_scope_for_member_template ?
                bind_template_arguments_for_instantiation(ctx,
                                                          *instantiation_context_scope,
                                                          *owner_member_instantiation_scope,
                                                          source_decl->parameters,
                                                          arguments,
                                                          effective_pack_sizes,
                                                          instantiation_owner) :
                bind_template_arguments_for_instantiation(ctx,
                                                          *instantiation_context_scope,
                                                          *use_scope,
                                                          source_decl->parameters,
                                                          arguments,
                                                          effective_pack_sizes,
                                                          instantiation_owner))) :
          bind_template_arguments(ctx,
                                  *instantiation_context_scope,
                                  source_decl->parameters,
                                  arguments,
                                  effective_pack_sizes);
  if(use_scope &&
     use_owner_scope_for_member_template &&
     use_scope != owner_member_instantiation_scope) {
    const std::set<std::string> excluded_names =
        collect_template_parameter_names(source_decl->parameters);
    overlay_instantiation_local_named_types(ctx,
                                            inst_scope,
                                            *use_scope,
                                            instantiation_context_scope,
                                            arguments,
                                            &excluded_names);
  }
  if(use_owner_scope_for_member_template) {
    inst_scope.class_info = instantiation_owner;
    bind_active_owner_instantiation_context(ctx,
                                            inst_scope,
                                            *instantiation_context_scope,
                                            *instantiation_owner);
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "function-instantiation-scope name=" << source_decl->name
          << " key=" << key
          << " active-owner="
          << (instantiation_owner ? instantiation_owner->qualified_name : std::string("<none>"))
          << " owner-member-scope="
          << (owner_member_instantiation_scope ?
                  scope_name_for_diagnostic(*owner_member_instantiation_scope) :
                  std::string("<none>"))
          << " use-owner-scope="
          << (use_owner_scope_for_member_template ? "yes" : "no")
          << " scope=" << scope_name_for_diagnostic(inst_scope)
          << " scope-owner="
          << (inst_scope.class_info ? inst_scope.class_info->qualified_name :
                                      std::string("<none>"))
          << " bindings=" << scope_bindings_for_diagnostic(inst_scope);
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  std::string name;
  TypePtr type;
  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const auto instantiation_context = [&]() -> std::string
  {
    std::ostringstream out;
    out << " [template " << source_decl->name << "]";
    out << " [inst-scope " << scope_name_for_diagnostic(inst_scope) << "]";
    out << " [inst-bindings " << scope_bindings_for_diagnostic(inst_scope) << "]";
    return out.str();
  };

  if(source_decl->is_constructor || source_decl->is_destructor) {
    name = template_api::with_template_services(
        ctx,
        [&](template_api::TemplateServices & services)
        {
          return template_function_signature::normalize_special_member_template_name(
              services,
              source_decl->name,
              source_decl->is_constructor,
              source_decl->is_destructor);
        });
    type = TypePtr();
    params = source_decl->params_pattern;
    default_args = source_decl->default_arguments_pattern;
    const CppAstNode * parameter_clause =
        function_template_parameter_clause(*source_decl);
    if(parameter_clause && !source_decl->has_trailing_function_parameter_pack) {
      std::vector<std::pair<std::string, TypePtr> > parsed_params;
      std::vector<const CppAstNode *> parsed_default_args;
      parse_instantiated_function_template_parameter_clause(
          ctx, inst_scope, name, *parameter_clause, parsed_params, parsed_default_args);
      params.swap(parsed_params);
      default_args.swap(parsed_default_args);
    }
    if(source_decl->has_trailing_function_parameter_pack) {
      std::vector<std::pair<std::string, TypePtr> > expanded_params;
      std::vector<const CppAstNode *> expanded_default_args;
      if(parameter_clause &&
         expand_instantiated_function_parameter_clause(ctx,
                                                       inst_scope,
                                                       *parameter_clause,
                                                       expanded_params,
                                                       expanded_default_args)) {
        params.swap(expanded_params);
        default_args.swap(expanded_default_args);
      } else {
        throw TemplateSubstitutionFailure(
            "unsupported trailing function parameter pack expansion");
      }
    }
  } else if(source_decl->is_conversion_operator) {
    name = source_decl->name;
    type = source_decl->type_pattern;
    params = source_decl->params_pattern;
    default_args = source_decl->default_arguments_pattern;
    if(source_decl->has_trailing_function_parameter_pack) {
      std::vector<std::pair<std::string, TypePtr> > expanded_params;
      std::vector<const CppAstNode *> expanded_default_args;
      const CppAstNode * parameter_clause =
          function_template_parameter_clause(*source_decl);
      if(parameter_clause &&
         expand_instantiated_function_parameter_clause(ctx,
                                                       inst_scope,
                                                       *parameter_clause,
                                                       expanded_params,
                                                       expanded_default_args)) {
        params.swap(expanded_params);
        default_args.swap(expanded_default_args);
      } else {
        throw TemplateSubstitutionFailure(
            "unsupported trailing function parameter pack expansion");
      }
    }
  } else if(source_decl->is_lambda_call_operator_template) {
    name = source_decl->name;
    type = source_decl->type_pattern;
    params = source_decl->params_pattern;
    default_args = source_decl->default_arguments_pattern;
  } else if(source_decl->type_pattern) {
    name = source_decl->name;
    type = source_decl->type_pattern;
    params = source_decl->params_pattern;
    default_args = source_decl->default_arguments_pattern;
    if(source_decl->has_trailing_function_parameter_pack) {
      std::vector<std::pair<std::string, TypePtr> > expanded_params;
      std::vector<const CppAstNode *> expanded_default_args;
      const CppAstNode * parameter_clause =
          function_template_parameter_clause(*source_decl);
      if(parameter_clause &&
         expand_instantiated_function_parameter_clause(ctx,
                                                       inst_scope,
                                                       *parameter_clause,
                                                       expanded_params,
                                                       expanded_default_args)) {
        params.swap(expanded_params);
        default_args.swap(expanded_default_args);
      } else {
        throw TemplateSubstitutionFailure(
            "unsupported trailing function parameter pack expansion");
      }
    }
  } else {
    CppAstNode parse_specifiers;
    CppAstNode parse_declarator;
    ctx.build_function_template_parse_view(*source_decl, parse_specifiers, parse_declarator);
    const bool filter_nonmember_declarator =
        !source_decl->declaring_scope ||
        !source_decl->declaring_scope->class_info ||
        source_decl->is_static_member;
    try {
      template_function_signature::ParsedFunctionTemplateSignature parsed =
          semantic_trace::append_template_context_as_substitution_failure(
              [&]() -> template_function_signature::ParsedFunctionTemplateSignature
              {
                return template_api::with_template_services(
                    ctx,
                    [&](template_api::TemplateServices & services)
                    {
                      return template_function_signature::parse_function_template_signature(
                          services,
                          template_api::make_template_environment(inst_scope),
                          source_decl->name,
                          *source_decl->declarator,
                          parse_specifiers,
                          parse_declarator,
                          filter_nonmember_declarator);
                    });
              },
              instantiation_context);
      name = parsed.name;
      type = parsed.type;
      params.swap(parsed.params);
      default_args.swap(parsed.default_arguments);
    } catch(const TemplateSubstitutionFailure &) {
      if(!build_instantiated_function_parameter_pack_fallback(
             ctx, inst_scope, *source_decl, name, type, params, default_args)) {
        throw;
      }
    }
  }
  if(!source_decl->has_trailing_function_parameter_pack) {
    semantic_trace::append_template_context_as_substitution_failure(
        [&]()
        {
          refresh_instantiated_function_parameter_clause(
              ctx, inst_scope, *source_decl, type, params, default_args);
        },
        instantiation_context);
  }
  const auto source_decl_uses_instantiation_owner_template = [&]() -> bool
  {
    ClassInfo * source_owner =
        source_decl->declaring_scope ?
            source_decl->declaring_scope->class_info :
            nullptr;
    return instantiation_owner &&
           instantiation_owner->source_template &&
           source_owner &&
           (source_owner == instantiation_owner ||
            source_owner->source_template == instantiation_owner->source_template ||
            source_owner->class_node == instantiation_owner->source_template->class_node);
  };
  std::vector<bool> early_parameter_value_bindings(params.size(), false);
  std::vector<const CppAstNode *> parameter_declarations =
      source_decl->parameter_declarations_pattern;
  if(parameter_declarations.empty()) {
    if(const CppAstNode * parameter_clause =
           function_template_parameter_clause(*source_decl)) {
      for(std::size_t i = 0; i < parameter_clause->children.size(); ++i) {
        if(parameter_clause->children[i].kind == CppAstKind::parameter_declaration) {
          parameter_declarations.push_back(&parameter_clause->children[i]);
        }
      }
    }
  }
  if(parameter_declarations.size() == params.size()) {
    for(std::size_t i = 0; i < params.size(); ++i) {
      if(params[i].first.empty()) {
        continue;
      }
      for(std::size_t j = i + 1; j < parameter_declarations.size(); ++j) {
        if(parameter_declarations[j] &&
           ast_node_mentions_identifier_token(*parameter_declarations[j],
                                              params[i].first)) {
          early_parameter_value_bindings[i] = true;
          break;
        }
      }
    }
  }
  auto resolve_instantiated_params = [&]()
  {
    template_api::with_template_services(
        ctx,
        [&](template_api::TemplateServices & services)
        {
          template_api::TemplateEnvironmentHandle inst_env =
              template_api::make_template_environment(inst_scope);
          for(std::size_t i = 0; i < params.size(); ++i) {
            const bool dependent_before =
                template_argument_semantics::type_depends_on_template_parameter(
                    ctx, params[i].second);
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "function-instantiation-param-before name=" << source_decl->name
                    << " index=" << i
                    << " param-name=" << params[i].first
                    << " type=" << describe_type(params[i].second)
                    << " dependent=" << (dependent_before ? "yes" : "no")
                    << " scope=" << scope_name_for_diagnostic(inst_scope);
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            if(dependent_before) {
              TypePtr substituted;
              if(template_argument_semantics::substitute_type(
                     inst_scope,
                     params[i].second,
                     source_decl->parameters,
                     arguments,
                     substituted)) {
                params[i].second = substituted;
              }
            }
            if(source_decl_uses_instantiation_owner_template() &&
               !instantiation_owner->instantiation_arguments.empty()) {
              TypePtr owner_substituted;
              if(template_argument_semantics::substitute_type(
                     inst_scope,
                     params[i].second,
                     instantiation_owner->source_template->parameters,
                     instantiation_owner->instantiation_arguments,
                     owner_substituted)) {
                params[i].second = owner_substituted;
              }
              TypePtr class_owner_substituted;
              if(substitute_owner_arguments_in_class_type(
                     ctx,
                     services,
                     inst_scope,
                     instantiation_owner->source_template->parameters,
                     instantiation_owner->instantiation_arguments,
                     params[i].second,
                     class_owner_substituted) &&
                 class_owner_substituted) {
                if(parser_trace::enabled("template.resolve")) {
                  std::ostringstream trace;
                  trace << "function-instantiation-owner-param-substitute"
                        << " name=" << source_decl->name
                        << " index=" << i
                        << " before=" << describe_type(params[i].second)
                        << " after=" << describe_type(class_owner_substituted);
                  parser_trace::note("template.resolve", std::string(), trace.str());
                }
                params[i].second = class_owner_substituted;
              }
            }
            TypePtr resolved;
            if(recover_instantiation_bound_type(
                   services, inst_env, params[i].second, resolved)) {
              params[i].second = resolved;
            }
            const bool dependent_after =
                template_argument_semantics::type_depends_on_template_parameter(
                    ctx, params[i].second);
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "function-instantiation-param-after name=" << source_decl->name
                    << " index=" << i
                    << " param-name=" << params[i].first
                    << " type=" << describe_type(params[i].second)
                    << " dependent=" << (dependent_after ? "yes" : "no")
                    << " scope=" << scope_name_for_diagnostic(inst_scope);
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            if(!dependent_after &&
               i < early_parameter_value_bindings.size() &&
               early_parameter_value_bindings[i]) {
              template_scope::bind_parameter_value(inst_scope,
                                                   params[i].first,
                                                   params[i].second);
            }
          }
        }
    );
  };
  resolve_instantiated_params();
  const bool instantiation_scope_had_template_placeholders =
      ctx.scope_has_template_placeholders(inst_scope);
  bind_instantiated_function_parameter_values(ctx, inst_scope, *source_decl, params);
  // Some dependent alias/template-pack parameter types only collapse after the
  // instantiated parameter/value pack bindings exist in the function scope.
  resolve_instantiated_params();
  for(std::size_t i = 0; i < params.size(); ++i) {
    reject_invalid_instantiated_function_parameter_type(ctx,
                                                       *source_decl,
                                                       params[i].second);
  }
  const bool dependent_template_arguments =
      template_arguments_are_dependent_for_instantiation(ctx, arguments);
  const bool non_dependent_instantiation =
      !dependent_template_arguments &&
      !instantiation_scope_had_template_placeholders;
  std::vector<TypePtr> signature_param_types;
  signature_param_types.reserve(params.size());
  for(std::size_t i = 0; i < params.size(); ++i) {
    signature_param_types.push_back(params[i].second);
  }
  if(instantiation_owner && instantiation_owner->member_scope) {
    for(std::size_t i = 0; i < signature_param_types.size(); ++i) {
      TypePtr resolved;
      if(recover_instantiation_bound_type(ctx,
                                          *instantiation_owner->member_scope,
                                          signature_param_types[i],
                                          resolved) &&
         resolved) {
        signature_param_types[i] = resolved;
      }
    }
  }
  const bool retained_function_template_parameter =
      !dependent_template_arguments &&
      any_of(signature_param_types.begin(),
             signature_param_types.end(),
             [&ctx, source_decl](const TypePtr & param_type)
             {
               return template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                                     param_type) &&
                      type_mentions_template_parameter_name(param_type,
                                                           source_decl->parameters);
             });
  if((non_dependent_instantiation &&
      any_of(params.begin(),
             params.end(),
             [&ctx](const std::pair<std::string, TypePtr> & param)
             {
               return template_argument_semantics::type_depends_on_template_parameter(ctx, param.second);
             })) ||
     retained_function_template_parameter) {
    std::ostringstream out;
    out << "instantiated function template retained dependent parameter type";
    out << " [name " << source_decl->name << "]";
    out << " [params";
    for(std::size_t i = 0; i < params.size(); ++i) {
      out << (i == 0 ? " " : ", ");
      if(!params[i].first.empty()) {
        out << params[i].first << ":";
      }
      out << describe_type(params[i].second)
          << ":dependent="
          << (template_argument_semantics::type_depends_on_template_parameter(ctx, params[i].second) ? "yes" : "no");
    }
    out << "]";
    out << " [scope " << scope_name_for_diagnostic(inst_scope) << "]";
    out << " [bindings " << scope_bindings_for_diagnostic(inst_scope) << "]";
    throw_substitution_failure(out.str(), std::string(), "template-instantiation");
  }
  if(!source_decl->is_constructor && !source_decl->is_destructor) {
    TypePtr function_base = strip_top_level_cv(type);
    if(function_base && function_base->kind == Type::TK_FUNCTION) {
      TypePtr result_type = function_base->inner;
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "function-instantiation-result-before name=" << source_decl->name
              << " key=" << key
              << " type=" << describe_type(result_type)
              << " dependent="
              << (template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) ? "yes" : "no")
              << " scope=" << scope_name_for_diagnostic(inst_scope)
              << " bindings=" << scope_bindings_for_diagnostic(inst_scope);
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      const bool source_result_type_was_dependent =
          template_argument_semantics::type_depends_on_template_parameter(ctx, result_type);
      if(source_result_type_was_dependent) {
        TypePtr substituted;
        if(template_argument_semantics::substitute_type(
               inst_scope, result_type, source_decl->parameters, arguments, substituted)) {
          result_type = substituted;
        }
      }
      if(template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) &&
         source_decl_uses_instantiation_owner_template() &&
         !instantiation_owner->instantiation_arguments.empty()) {
        template_api::with_template_services(
            ctx,
            [&](template_api::TemplateServices & services)
            {
              std::vector<TemplateParameterInfo> combined_parameters;
              std::vector<TemplateArgument> combined_arguments;
              combined_parameters.insert(combined_parameters.end(),
                                         source_decl->parameters.begin(),
                                         source_decl->parameters.end());
              combined_arguments.insert(combined_arguments.end(),
                                        arguments.begin(),
                                        arguments.end());
              combined_parameters.insert(
                  combined_parameters.end(),
                  instantiation_owner->source_template->parameters.begin(),
                  instantiation_owner->source_template->parameters.end());
              combined_arguments.insert(
                  combined_arguments.end(),
                  instantiation_owner->instantiation_arguments.begin(),
                  instantiation_owner->instantiation_arguments.end());
              if(combined_parameters.size() == combined_arguments.size() &&
                 !combined_parameters.empty()) {
                TypePtr class_substituted;
                if(substitute_owner_arguments_in_class_type(ctx,
                                                            services,
                                                            inst_scope,
                                                            combined_parameters,
                                                            combined_arguments,
                                                            result_type,
                                                            class_substituted) &&
                   class_substituted) {
                  result_type = class_substituted;
                }
              }
            });
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "function-instantiation-result-substituted name=" << source_decl->name
              << " key=" << key
              << " type=" << describe_type(result_type)
              << " dependent="
              << (template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      bool result_type_still_dependent =
          template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                         result_type);
      if(result_type_still_dependent) {
        TypePtr recovered_substituted_result;
        if(recover_instantiation_bound_type(ctx,
                                            inst_scope,
                                            result_type,
                                            recovered_substituted_result) &&
           recovered_substituted_result) {
          result_type = recovered_substituted_result;
          result_type_still_dependent =
              template_argument_semantics::type_depends_on_template_parameter(
                  ctx,
                  result_type);
        }
      }
      const bool source_result_mentions_template_parameter =
          source_decl->result_type_pattern.kind != CppAstKind::invalid &&
          ast_mentions_template_parameter_name(source_decl->result_type_pattern,
                                               source_decl->parameters);
      if((result_type_still_dependent ||
          source_result_mentions_template_parameter) &&
         source_decl->result_type_pattern.kind != CppAstKind::invalid) {
        TypePtr parsed_result;
        const bool parsed_result_type =
            template_api::with_template_services(
                ctx,
                [&](template_api::TemplateServices & services)
                {
                  const witness::ScopedTemplateWitnessSourceCapturePause
                      source_capture_pause;
                  Scope result_scope(&inst_scope, "<trailing-return>", false);
                  Scope owner_result_scope(&inst_scope,
                                           "<trailing-return-owner>",
                                           false);
                  FunctionBinding synthetic_function;
                  Scope * parse_scope = &inst_scope;
                  if(source_decl->declaring_scope &&
                     source_decl->declaring_scope->class_info &&
                     !source_decl->is_static_member &&
                     inst_scope.class_info &&
                     inst_scope.class_info->type) {
                    TypePtr declared_type =
                        make_function(make_fundamental(FT_VOID),
                                      std::vector<TypePtr>(),
                                      false);
                    TypePtr method_type =
                        semantic_class_model::method_function_type(
                            inst_scope.class_info->type,
                            source_decl->is_const_method,
                            source_decl->is_volatile_method,
                            declared_type);
                    if(method_type && !method_type->params.empty()) {
                      synthetic_function.name = "<trailing-return>";
                      synthetic_function.owner_class = inst_scope.class_info;
                      synthetic_function.lexical_access_class = inst_scope.class_info;
                      synthetic_function.is_method = true;
                      synthetic_function.is_const_method = source_decl->is_const_method;
                      synthetic_function.is_volatile_method = source_decl->is_volatile_method;
                      synthetic_function.ref_qualifier = source_decl->ref_qualifier;
                      synthetic_function.type = method_type;
                      result_scope.class_info = inst_scope.class_info;
                      result_scope.namespace_scope = inst_scope.namespace_scope;
                      result_scope.function = &synthetic_function;
                      result_scope.values["this"] =
                          ValueBinding(ValueBinding::VK_PARAMETER,
                                       "this",
                                       method_type->params[0]);
                      parse_scope = &result_scope;
                    }
                  }
                  if(instantiation_owner &&
                     instantiation_owner->source_template &&
                     !instantiation_owner->instantiation_arguments.empty()) {
                    if(parse_scope == &inst_scope) {
                      owner_result_scope.class_info = inst_scope.class_info;
                      owner_result_scope.namespace_scope =
                          inst_scope.namespace_scope;
                      owner_result_scope.function = inst_scope.function;
                      parse_scope = &owner_result_scope;
                    }
                    ClassTemplateDecl & owner_template =
                        *instantiation_owner->source_template;
                    const std::vector<TemplateParameterInfo> * owner_parameters =
                        &owner_template.parameters;
                    const std::vector<TemplateArgument> * owner_arguments =
                        &instantiation_owner->instantiation_arguments;
                    const std::map<std::string, std::size_t> * owner_pack_sizes =
                        nullptr;
                    std::vector<TemplateArgument> selected_arguments;
                    std::map<std::string, std::size_t> selected_pack_sizes;
                    if(selected_partial_specialization(owner_template,
                                                       *instantiation_owner)) {
                      const template_selection::ClassSpecializationSelection selection =
                          template_selection::select_class_specialization(
                              services,
                              owner_template,
                              template_api::make_template_environment(inst_scope),
                              template_argument_key_for_instantiation(
                                  ctx,
                                  instantiation_owner->instantiation_arguments),
                              instantiation_owner->instantiation_arguments);
                      if(selection.kind ==
                             template_selection::MS_PARTIAL_SPECIALIZATION &&
                         selection.parameters) {
                        owner_parameters = selection.parameters;
                        selected_arguments = selection.arguments;
                        selected_pack_sizes = selection.pack_sizes;
                        owner_arguments = &selected_arguments;
                        owner_pack_sizes = selected_pack_sizes.empty() ?
                            nullptr :
                            &selected_pack_sizes;
                      }
                    }
                    ::template_instantiation::bind_template_arguments_into_scope(
                        services,
                        *parse_scope,
                        *owner_parameters,
                        *owner_arguments,
                        owner_pack_sizes);
                  }
                  CppAstNode parse_pattern = source_decl->result_type_pattern;
                  if(template_parameters_have_pack(source_decl->parameters)) {
                    clear_cached_semantic_types(parse_pattern);
                  }
                  CppAstNode substituted_pattern;
                  if(template_argument_semantics::substitute_type_id_node_for_template_arguments(
                         services,
                         *parse_scope,
                         parse_pattern,
                         source_decl->parameters,
                         arguments,
                         substituted_pattern)) {
                    parse_pattern = substituted_pattern;
                  }
                  if(template_parameters_have_pack(source_decl->parameters)) {
                    clear_dependent_cached_semantic_types(parse_pattern,
                                                          source_decl->parameters);
                  }
                  const bool parsed =
                      template_decl_ast::parse_type_id(services,
                                                       *parse_scope,
                                                       *parse_scope,
                                                       parse_pattern,
                                                       parsed_result,
                                                       !include_body) &&
                      parsed_result;
                  return parsed;
        });
        if(parsed_result_type && parsed_result) {
          TypePtr resolved_result;
          if(recover_instantiation_bound_type(ctx,
                                             inst_scope,
                                             parsed_result,
                                             resolved_result) &&
             resolved_result) {
            parsed_result = resolved_result;
          }
          if(!template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                              parsed_result) ||
             template_argument_semantics::type_depends_on_template_parameter(ctx,
                                                                             result_type)) {
            result_type = parsed_result;
          }
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "function-instantiation-result-reparsed name=" << source_decl->name
                  << " key=" << key
                  << " type=" << describe_type(parsed_result)
                  << " dependent="
                  << (template_argument_semantics::type_depends_on_template_parameter(ctx, parsed_result) ? "yes" : "no");
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
        } else if(source_result_mentions_template_parameter &&
                  !dependent_template_arguments &&
                  !source_result_type_was_dependent) {
          throw_substitution_failure(
              "failed function template result type substitution",
              std::string(),
              "template-instantiation");
        }
      }
      TypePtr resolved;
      const bool recover_result_type =
          include_body &&
          (non_dependent_instantiation ||
           (!dependent_template_arguments &&
            !type_has_dependent_alias_pack_expansion(result_type)));
      if(recover_result_type) {
        const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
            function_call_source_capture_pause;
        if(recover_instantiation_bound_type(ctx, inst_scope, result_type, resolved)) {
          result_type = resolved;
        }
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "function-instantiation-result-resolved name=" << source_decl->name
              << " key=" << key
              << " type=" << describe_type(result_type)
              << " dependent="
              << (template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) ? "yes" : "no");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!dependent_template_arguments &&
         template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) &&
         type_mentions_template_parameter_name(result_type, source_decl->parameters)) {
        throw_substitution_failure(
            "instantiated function template retained dependent result type",
            std::string(),
            "template-instantiation");
      }
      std::vector<TypePtr> rebuilt_params;
      for(std::size_t i = 0; i < params.size(); ++i) {
        rebuilt_params.push_back(params[i].second);
      }
      if(non_dependent_instantiation &&
         (template_argument_semantics::type_depends_on_template_parameter(ctx, result_type) ||
          any_of(rebuilt_params.begin(),
                 rebuilt_params.end(),
                 [&ctx](const TypePtr & param_type)
                 {
                   return template_argument_semantics::type_depends_on_template_parameter(ctx, param_type);
                 }))) {
        throw_substitution_failure(
            "instantiated function template retained dependent function type",
            std::string(),
            "template-instantiation");
      }
      type = make_function(result_type,
                           rebuilt_params,
                           function_base->variadic,
                           function_base->function_const,
                           function_base->function_volatile,
                           function_base->prototype_relaxed,
                           function_base->function_ref_qualifier);
    }
  }
  const std::vector<std::string> parameter_aliases =
      instantiate_function_parameter_aliases(*source_decl, params);

  const CppAstNode * instantiated_body =
      include_body ? (explicit_specialization ? body_override :
                                             effective_template_body(*source_decl)) :
                     nullptr;
  FunctionBinding * binding = nullptr;
  if(source_decl->declaring_scope && source_decl->declaring_scope->class_info) {
    ClassInfo & info = *instantiation_owner;
    FunctionRegistrationRequest request;
    request.owner_class = &info;
    request.name = name;
    request.declared_type = type;
    request.params = params;
    request.default_arguments = default_args;
    request.body = instantiated_body;
    request.ctor_initializer = source_decl->ctor_initializer;
    request.declaration_node = source_decl->declaration_node;
    request.parameter_syntax_node = source_decl->declarator;
    request.semantic_flags.access = source_decl->access;
    request.semantic_flags.is_constructor = source_decl->is_constructor;
    request.semantic_flags.is_inherited_constructor =
        source_decl->is_inherited_constructor;
    request.semantic_flags.is_destructor = source_decl->is_destructor;
    request.semantic_flags.is_conversion_operator =
        source_decl->is_conversion_operator;
    request.semantic_flags.is_explicit = source_decl->is_explicit;
    request.semantic_flags.is_const_method = source_decl->is_const_method;
    request.semantic_flags.is_volatile_method = source_decl->is_volatile_method;
    request.semantic_flags.ref_qualifier = source_decl->ref_qualifier;
    request.semantic_flags.is_virtual_specified = source_decl->decl_virtual;
    request.semantic_flags.is_override_specified = source_decl->is_override;
    request.semantic_flags.is_final = source_decl->is_final;
    request.semantic_flags.function_qualifier = effective_function_qualifier(*source_decl);
    request.semantic_flags.is_constexpr = effective_is_constexpr;
    request.semantic_flags.is_deleted = source_decl->is_deleted;
    attach_function_template_registration_identity(
        request, *source_decl, arguments, key, prefer_overload_suffix);
    request.is_static_member =
        source_decl->is_static_member &&
        !source_decl->is_constructor &&
        !source_decl->is_destructor;
    binding = ctx.register_function_entity(request);
    if(binding &&
       source_decl->is_inherited_constructor &&
       binding->is_constructor) {
      refresh_instantiated_inherited_constructor_initializer(ctx,
                                                            inst_scope,
                                                            *source_decl,
                                                            *binding);
    }
  } else {
    FunctionRegistrationRequest request;
    request.scope = source_decl->declaring_scope;
    request.name = name;
    request.declared_type = type;
    request.params = params;
    request.default_arguments = default_args;
    request.body = instantiated_body;
    request.declaration_node = source_decl->declaration_node;
    request.parameter_syntax_node = source_decl->declarator;
    request.declaration_scope = &inst_scope;
    request.function_qualifier = effective_function_qualifier(*source_decl);
    request.is_constexpr = effective_is_constexpr;
    request.semantic_flags.is_deleted = source_decl->is_deleted;
    attach_function_template_registration_identity(
        request, *source_decl, arguments, key, prefer_overload_suffix);
    ctx.register_function_entity(request);
    const std::vector<FunctionBinding *> * slot_found =
        semantic_lookup::find_direct_function_set(*source_decl->declaring_scope, name);
    if(slot_found) {
      for(std::size_t i = 0; i < slot_found->size(); ++i) {
        FunctionBinding * candidate = (*slot_found)[i];
        if(candidate &&
           function_binding_matches_template_registration_identity(
               *candidate, *source_decl, key) &&
           candidate->type &&
           type &&
           type_equals(candidate->type, type)) {
          binding = candidate;
          break;
        }
      }
    }
    if(!binding) {
      throw std::logic_error("missing instantiated function");
    }
  }
  binding->is_deleted = binding->is_deleted || source_decl->is_deleted;
  apply_instantiated_parameter_aliases(*binding, parameter_aliases);
  note_function_instantiation_use_location(*binding, instantiation_use_location);
  binding->exclude_from_explicit_instantiation =
      binding->exclude_from_explicit_instantiation ||
      source_decl->exclude_from_explicit_instantiation;
  if(instantiation_owner &&
     instantiation_owner->source_template &&
     !binding->has_definition) {
    apply_out_of_class_member_function_definitions(ctx,
                                                   *instantiation_owner->source_template,
                                                   *instantiation_owner,
                                                   instantiation_owner->instantiation_arguments);
    apply_out_of_class_special_member_definitions(ctx,
                                                  *instantiation_owner->source_template,
                                                  *instantiation_owner,
                                                  instantiation_owner->instantiation_arguments);
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "function-instantiation-new name=" << source_decl->name
          << " parsed_name=" << name
          << " key=" << key
          << " source_template=" << static_cast<void *>(source_decl)
          << " is_ctor=" << (source_decl->is_constructor ? "yes" : "no")
          << " decl_loc=" << function_template_decl_location(ctx, source_decl);
    const std::string decl_details =
        semantic_trace::template_decl_location_details(ctx, source_decl);
    if(!decl_details.empty()) {
      trace << " decl_loc_detail=" << decl_details;
    }
    trace
          << " declared_owner="
          << (source_decl->declaring_scope && source_decl->declaring_scope->class_info ?
                  source_decl->declaring_scope->class_info->qualified_name :
                  std::string("<none>"))
          << " active_owner="
          << (instantiation_owner ? instantiation_owner->qualified_name : std::string("<none>"))
          << " source_params=" << function_template_params_for_trace(*source_decl)
          << " ctor_init_decl=" << (source_decl->ctor_initializer ? "yes" : "no")
          << " ctor_init_binding=" << (binding && binding->ctor_initializer ? "yes" : "no")
          << " has_definition=" << (binding && binding->has_definition ? "yes" : "no");
    append_function_binding_trace_identity(trace, ctx, binding);
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  binding->is_constexpr = binding->is_constexpr || effective_is_constexpr;
  if(!binding->lexical_access_class) {
    binding->lexical_access_class = source_decl->lexical_access_class;
  }
  if(!binding->lexical_access_function) {
    binding->lexical_access_function = source_decl->lexical_access_function;
  }
  if(!binding->source_template) {
    binding->source_template = source_decl;
  }
  record_function_template_argument_state(*binding,
                                          arguments,
                                          effective_pack_sizes,
                                          true);
  if(binding->body) {
    if(source_decl->definition_declarator) {
      binding->definition_node = source_decl->definition_declarator;
    } else if(source_decl->definition_node) {
      binding->definition_node = source_decl->definition_node;
    }
    binding->definition_abi_tags =
        function_template_definition_abi_tags(*source_decl);
    binding->definition_suppresses_declaration_abi_tags =
        function_template_definition_should_suppress_declaration_abi_tags(ctx, *source_decl);
  }
  if(binding->source_template &&
     binding->symbol.linkage == symbol_linkage::SL_WEAK) {
    ctx.upgrade_function_symbol_linkage(binding, binding->symbol.linkage);
  }
  binding->declaration_scope = &inst_scope;
  refresh_pack_dependent_result_type(*source_decl, inst_scope, *binding, false);
  binding->is_explicit_specialization =
      binding->is_explicit_specialization || explicit_specialization;
  if(explicit_specialization) {
    binding->body = body_override;
    binding->has_definition = body_override != nullptr;
    if(body_override) {
      binding->definition_node =
          definition_node_override ? definition_node_override : body_override;
    }
    binding->definition_suppresses_declaration_abi_tags = false;
  } else if(!include_body) {
    binding->body = nullptr;
    binding->has_definition = false;
  }
  source_decl->instantiations[key] = binding;
  record_function_template_instantiation_cache_entry(*binding, *source_decl, key);
  if((include_body || explicit_specialization) &&
     (binding->has_definition || explicit_specialization)) {
    apply_function_instantiation_intent(
        ctx, binding, InstantiatedFunctionOutputMode::TrackOnly);
  }
  return binding;
}

const ValueBinding * instantiate_variable_template(
    SemanticContext & ctx,
    VariableTemplateDecl & decl,
    const std::vector<TemplateArgument> & arguments,
    const std::string & source_use_location,
    Scope * source_use_scope)
{
  ensure_template_arguments_fully_bind_parameters(
      ctx,
      "instantiate_variable_template",
      decl.name,
      decl.parameters,
      arguments);
  const std::string key = template_argument_key_for_instantiation(ctx, arguments);
  const bool nested_replay_request = variable_initializer_replay_depth > 0;
  const bool dependent_arguments =
      template_arguments_are_dependent_for_instantiation(ctx, arguments);
  const auto note_variable_use =
      [&](const template_selection::VariableSpecializationSelection & selection,
          const ValueBinding & binding,
          bool emit_source_use,
          witness::VariableUseMergePolicy merge_policy) -> void
  {
    const std::string use_location = parser_trace::current_use_location();
    const std::string effective_use_location =
        !source_use_location.empty() ? source_use_location : use_location;
    const bool trace_enabled = parser_trace::enabled("template.resolve");
    const bool source_capture_enabled =
        emit_source_use &&
        witness::source_location_capture_enabled(ctx.template_witness_context(),
                                                 effective_use_location);
    const bool record_direct_source_use_during_pause =
        emit_source_use &&
        !source_capture_enabled &&
        source_use_scope != nullptr &&
        witness::enabled(ctx.template_witness_context()) &&
        witness::source_location_is_from_primary_file(
            ctx.template_witness_context(),
            effective_use_location);
    if(effective_use_location.empty()) {
      return;
    }
    if(!trace_enabled &&
       !source_capture_enabled &&
       !record_direct_source_use_during_pause) {
      return;
    }

    const CppAstNode * selected_decl_node =
        selection.declarator ? selection.declarator : selection.specifiers;
    const witness::SourceSelectionKind selection_kind =
        selection.kind == template_selection::MS_EXPLICIT_SPECIALIZATION ?
            witness::SourceSelectionKind::ExplicitSpecialization :
            (selection.kind == template_selection::MS_PARTIAL_SPECIALIZATION ?
                 witness::SourceSelectionKind::PartialSpecialization :
                 witness::SourceSelectionKind::Primary);
    if(source_capture_enabled || record_direct_source_use_during_pause) {
      witness::VariableUseEmitRequest request;
      request.use_location = effective_use_location;
      request.use_anchor_identifier = decl.name;
      request.ownership =
          nested_replay_request ?
              witness::SourceUseOwnership::NestedDerived :
              witness::SourceUseOwnership::Direct;
      request.template_name = qualified_variable_template_name(decl);
      request.selection = selection_kind;
      const std::vector<std::string> * source_template_arg_texts =
          template_api::current_template_id_source_arguments_ptr(
              effective_use_location,
              decl.name);
      if(selection.parameters == &decl.parameters) {
        const semantic_model::SourceDeclAnchorCache & decl_anchor =
            semantic_trace::variable_template_decl_anchor(ctx, &decl);
        witness::set_selected_decl_anchor(request.selected_decl_location,
                                          request.selected_decl_anchor,
                                          decl_anchor);
      } else {
        request.selected_decl_location =
            normalized_template_name_or_node_location(
                ctx, selection.declarator, decl.name, true);
        if(request.selected_decl_location.empty()) {
          request.selected_decl_location =
              normalized_template_name_or_node_location(
                  ctx, selection.specifiers, decl.name, true);
        }
      }
      if(selection.parameters && selection.parameters != &decl.parameters) {
        template_api::append_template_witness_source_bindings(
            ctx,
            request.bindings,
            decl.parameters,
            arguments,
            "deduced",
            template_api::TemplateWitnessSourceBindingPolicy::
                DeducedWithDefaultedTrailingDefaults);
        template_api::append_template_witness_source_bindings(
            ctx,
            request.specialization_bindings,
            *selection.parameters,
            selection.arguments,
            "deduced",
            template_api::TemplateWitnessSourceBindingPolicy::
                DeducedWithDefaultedTrailingDefaults);
      } else if(selection_kind == witness::SourceSelectionKind::ExplicitSpecialization) {
        if(source_template_arg_texts) {
          template_api::append_template_witness_source_bindings(
              ctx,
              request.bindings,
              decl.parameters,
              arguments,
              *source_template_arg_texts,
              "explicit",
              "defaulted");
        } else {
          template_api::append_template_witness_source_bindings(
              ctx,
              request.bindings,
              decl.parameters,
              arguments,
              "explicit");
        }
      } else {
        template_api::append_template_witness_source_bindings(
            ctx,
            request.bindings,
            decl.parameters,
            arguments,
            "deduced",
            template_api::TemplateWitnessSourceBindingPolicy::
                DeducedWithDefaultedTrailingDefaults);
      }
      request.merge_policy = merge_policy;
      request.record_during_source_capture_pause =
          record_direct_source_use_during_pause;
      witness::emit_variable_use(request);
      if(source_use_scope != nullptr) {
        ctx.emit_nested_class_use_source_events_from_location(
            *source_use_scope,
            request.use_location,
            witness::SourceUseOwnership::NestedDerived);
      }
    }

    if(trace_enabled) {
      const auto append_binding_map =
          [&](std::ostringstream & out,
              const std::vector<TemplateParameterInfo> & parameters_to_use,
              const std::vector<TemplateArgument> & arguments_to_use) -> void
      {
        for(std::size_t i = 0;
            i < parameters_to_use.size() && i < arguments_to_use.size();
            ++i) {
          if(i != 0) {
            out << ",";
          }
          out << (parameters_to_use[i].name.empty() ?
                      std::string("$") + std::to_string(i + 1) :
                      parameters_to_use[i].name)
              << ":" << template_argument_text_for_diagnostic(ctx, arguments_to_use[i]);
        }
      };

      std::ostringstream primary_bindings;
      append_binding_map(primary_bindings, decl.parameters, arguments);

      std::ostringstream specialize_bindings;
      if(selection.parameters && selection.parameters != &decl.parameters) {
        append_binding_map(specialize_bindings, *selection.parameters, selection.arguments);
      }

      std::ostringstream trace;
      trace << "variable-use"
            << " name=" << decl.name
            << " resolved={"
            << specialization_name_for_instantiation(ctx, decl.name, arguments) << "}"
            << " kind=" << witness::source_selection_text(selection_kind)
            << " decl="
            << (selected_decl_node ? ctx.source_location_for_node(*selected_decl_node) :
                                     std::string("<none>"))
            << " bindings={" << primary_bindings.str() << "}";
      if(!specialize_bindings.str().empty()) {
        trace << " specialize_bindings={" << specialize_bindings.str() << "}";
      }
      if(binding.has_constant_value) {
        trace << " value={" << binding.constant_value << "}";
      }
      parser_trace::note("template.resolve", effective_use_location, trace.str());
    }
  };
  std::map<std::string, ValueBinding>::iterator found = decl.instantiations.find(key);
  if(found != decl.instantiations.end()) {
    const std::string cache_hit_effective_use_location =
        !source_use_location.empty() ?
            source_use_location :
            parser_trace::current_use_location();
    const bool cache_hit_direct_source_use_during_pause =
        source_use_scope != nullptr &&
        !cache_hit_effective_use_location.empty() &&
        witness::enabled(ctx.template_witness_context()) &&
        witness::source_location_is_from_primary_file(
            ctx.template_witness_context(),
            cache_hit_effective_use_location);
    if(parser_trace::enabled("template.resolve") ||
       witness::source_capture_enabled() ||
       cache_hit_direct_source_use_during_pause) {
      const template_selection::VariableSpecializationSelection selection =
          template_api::with_template_services(
              ctx,
              [&](template_api::TemplateServices & services)
              {
                return template_selection::select_variable_specialization(
                    services, decl, key, arguments);
              });
      bool member_template_source_use = false;
      for(Scope * scope = source_use_scope; scope; scope = scope->parent) {
        if(scope->class_info ||
           (scope->function && scope->function->owner_class)) {
          member_template_source_use = true;
          break;
        }
      }
      const bool relocate_prior_source_use = !member_template_source_use;
      const witness::VariableUseMergePolicy merge_policy =
          relocate_prior_source_use ?
              witness::VariableUseMergePolicy::ReplaceEquivalentSourceUse :
              witness::VariableUseMergePolicy::AppendIfNew;
      note_variable_use(selection,
                        found->second,
                        relocate_prior_source_use && !dependent_arguments,
                        merge_policy);
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "variable-instantiation name=" << decl.name
            << " key=" << key
            << " cache=hit"
            << " dependent=" << (found->second.dependent_template_value ? "yes" : "no")
            << " has_constant=" << (found->second.has_constant_value ? "yes" : "no");
      if(found->second.has_constant_value) {
        trace << " value=" << found->second.constant_value;
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return &found->second;
  }

  const template_selection::VariableSpecializationSelection specialization =
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            return template_selection::select_variable_specialization(
                services, decl, key, arguments);
          });
  Scope * binding_scope = specialization.binding_scope;
  const std::vector<TemplateParameterInfo> * bound_parameters = specialization.parameters;
  const std::vector<TemplateArgument> * bound_arguments = &specialization.arguments;
  const std::map<std::string, std::size_t> * bound_pack_sizes = &specialization.pack_sizes;
  const CppAstNode * specifiers = specialization.specifiers;
  const CppAstNode * declarator = specialization.declarator;
  const CppAstNode * initializer = specialization.initializer;

  Scope & inst_scope =
      bind_template_arguments(ctx,
                              *binding_scope,
                              *bound_parameters,
                              *bound_arguments,
                              bound_pack_sizes);

  TypePtr type = decl.type_pattern;
  if(specifiers && declarator) {
    std::string specialized_name;
    bool is_typedef = false;
    TypePtr specialized_type;
    if(ctx.parse_variable_declaration_type(inst_scope,
                                           *specifiers,
                                           *declarator,
                                           initializer,
                                           true,
                                           specialized_name,
                                           specialized_type,
                                           is_typedef,
                                           true) &&
       specialized_type && !is_typedef) {
      type = specialized_type;
    }
  }
  if(type && template_argument_semantics::type_depends_on_template_parameter(ctx, type)) {
    TypePtr substituted;
    if(template_argument_semantics::substitute_type(
           type, *bound_parameters, *bound_arguments, substituted)) {
      type = substituted;
    }
  }

  std::map<std::string, ValueBinding>::iterator inserted =
      decl.instantiations.insert(std::make_pair(
          key,
          ValueBinding(ValueBinding::VK_VARIABLE,
                       specialization_name_for_instantiation(ctx, decl.name, arguments),
                       type))).first;
  inserted->second.is_explicit_specialization =
      specialization.kind == template_selection::MS_EXPLICIT_SPECIALIZATION;
  inserted->second.dependent_template_value = true;
  inserted->second.constant_initializer = initializer;
  inserted->second.constant_initializer_scope = initializer ? &inst_scope : nullptr;

  if(parser_trace::enabled("template.resolve")) {
    for(std::size_t i = 0; i < arguments.size(); ++i) {
      std::ostringstream trace;
      trace << "variable-arg name=" << decl.name
            << " index=" << i
            << " kind=" << arguments[i].kind;
      if(arguments[i].kind == TemplateArgument::TA_TYPE) {
        trace << " type=" << describe_type(arguments[i].type)
              << " dependent="
              << (template_argument_semantics::type_depends_on_template_parameter(ctx, arguments[i].type) ? "yes" : "no")
              << " text=" << template_argument_text_for_diagnostic(ctx, arguments[i]);
      } else if(arguments[i].kind == TemplateArgument::TA_VALUE) {
        trace << " dependent=" << (arguments[i].dependent ? "yes" : "no")
              << " text=" << template_argument_text_for_diagnostic(ctx, arguments[i]);
      } else {
        trace << " text=" << template_argument_text_for_diagnostic(ctx, arguments[i]);
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
  }

  if(dependent_arguments) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "variable-instantiation name=" << decl.name
            << " key=" << key
            << " cache=miss"
            << " dependent-arguments=yes";
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return &inserted->second;
  }

  if(initializer) {
    bool initializer_value_handled = false;
    if(decl.comes_from_standard_include_path &&
       variable_template_declared_in_std_namespace_or_inline_child(decl)) {
      bool structured_value = false;
      const template_argument_semantics::NonTypeArgumentStatus status =
          template_api::with_template_services(
              ctx,
              [&](template_api::TemplateServices & services)
              {
                return template_argument_semantics::
                    evaluate_standard_invocable_variable_template_arguments(
                        services,
                        template_api::make_template_environment(inst_scope),
                        decl.name,
                        arguments,
                        structured_value);
              });
      if(status == template_argument_semantics::NT_ARG_EVALUATED) {
        inserted->second.has_constant_value = true;
        inserted->second.constant_value = structured_value ? 1 : 0;
        inserted->second.dependent_template_value = false;
        initializer_value_handled = true;
      } else if(status == template_argument_semantics::NT_ARG_DEPENDENT) {
        inserted->second.dependent_template_value = true;
        initializer_value_handled = true;
      }
    }

    if(!initializer_value_handled) {
      long long value = 0;
      std::string initializer_use_location =
          template_api::normalize_template_witness_source_location(
              ctx.source_location_for_name_in_node(*initializer, decl.name));
      if(initializer_use_location.empty()) {
        initializer_use_location =
            template_api::normalize_template_witness_source_location(
                ctx.source_location_for_node(*initializer));
      }
      const ScopedTemplateUseLocation use_location_guard(
          !initializer_use_location.empty() ? initializer_use_location :
                                              parser_trace::current_use_location());
      if(initializer) {
        const ScopedTemplateWitnessSourceCaptureResume source_capture_resume;
        ctx.emit_nested_class_use_source_events_from_ast_node(
            inst_scope,
            *initializer,
            witness::SourceUseOwnership::SourceOwned,
            true);
      }
      const ScopedVariableInitializerReplay replay_guard;
      if(ctx.evaluate_initializer_constant(inst_scope, *initializer, value)) {
        inserted->second.has_constant_value = true;
        inserted->second.constant_value = value;
        inserted->second.dependent_template_value = false;
      } else if(ctx.scope_has_template_placeholders(inst_scope)) {
        inserted->second.dependent_template_value = true;
      } else {
        inserted->second.dependent_template_value = false;
      }
    }
  } else {
    inserted->second.dependent_template_value = false;
  }

  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "variable-instantiation name=" << decl.name
          << " key=" << key
          << " cache=miss"
          << " dependent=" << (inserted->second.dependent_template_value ? "yes" : "no")
          << " has_constant=" << (inserted->second.has_constant_value ? "yes" : "no");
    if(inserted->second.has_constant_value) {
      trace << " value=" << inserted->second.constant_value;
    }
    parser_trace::note("template.resolve", std::string(), trace.str());
  }

  note_variable_use(specialization,
                    inserted->second,
                    true,
                    witness::VariableUseMergePolicy::AppendIfNew);

  return &inserted->second;
}

}  // namespace template_instantiation
