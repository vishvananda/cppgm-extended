#include "template_api_internal.h"
#include "callsemantic/template_source_utils.h"
#include "template_argument_semantics.h"
#include "callsemantic_internal.h"
#include "class_template_mangle_info.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "template_function_signature.h"
#include "template_instantiation.h"
#include "template_resolution.h"
#include "template_services.h"
#include "template_selection_api.h"
#include "template_selection.h"
#include "template_specialization.h"
#include "template_scope.h"
#include "semantic_hotspot.h"
#include "semantic_class_model.h"
#include "semantic_errors.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"

#include <algorithm>
#include <climits>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace template_resolution {

bool deduce_function_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool resolve_function_explicit_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_arg_syntaxes = nullptr);

bool deduce_function_template_arguments_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count);

}  // namespace template_resolution

namespace template_argument_semantics {

// template-boundary-audit: begin text_recovery_bridge
std::string lookup_text_for_type_argument(SemanticContext & ctx,
                                          const cpp_decl::TypePtr & type);

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);

// template-boundary-audit: end text_recovery_bridge

}  // namespace template_argument_semantics

namespace template_instantiation {

semantic_model::Scope & bind_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::Scope & bind_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    semantic_model::ClassInfo * active_owner);

semantic_model::Scope & bind_class_template_arguments_for_instantiation(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    semantic_model::Scope & use_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr);

semantic_model::ClassInfo * instantiate_class_template(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::vector<std::string> & arg_texts);

void finalize_instantiated_class(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::ClassInfo & info,
    const std::vector<template_model::TemplateArgument> & arguments);

semantic_model::FunctionBinding * instantiate_function_template(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::ClassInfo * active_owner = nullptr,
    const CppAstNode * body_override = nullptr,
    const CppAstNode * definition_node_override = nullptr,
    bool explicit_specialization = false,
    bool explicit_specialization_is_constexpr = false,
    bool include_body = true,
    semantic_model::Scope * use_scope = nullptr,
    const std::map<std::string, std::size_t> * pack_sizes = nullptr,
    bool prefer_overload_suffix = false,
    const std::string & instantiation_use_location_override = std::string());

const semantic_model::ValueBinding * instantiate_variable_template(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::string & source_use_location = std::string(),
    semantic_model::Scope * source_use_scope = nullptr);

}  // namespace template_instantiation

namespace template_api {

std::string class_witness_output_qualified_name(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info);

std::string class_witness_output_qualified_name_for_lifecycle(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info);

bool template_argument_has_underqualified_source_type_for_witness(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument);

std::size_t witness_visible_class_template_argument_count(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info,
    bool allow_explicit_default_equivalent);

namespace {

struct TemplateArgumentSourceLocationFrame
{
  std::vector<std::string> texts;
  std::vector<std::string> locations;
};

struct TemplateIdSourceArgumentFrame
{
  std::string location;
  std::string template_name;
  std::vector<std::string> arg_texts;
};

typedef std::pair<std::string, std::string> TemplateIdSourceArgumentKey;

thread_local std::vector<TemplateArgumentSourceLocationFrame>
    template_argument_source_location_stack;
thread_local std::vector<TemplateIdSourceArgumentFrame>
    template_id_source_argument_stack;
thread_local std::map<TemplateIdSourceArgumentKey,
                      std::vector<std::string> > template_id_source_argument_cache;
struct SourceTypeMaterializationFrame
{
  SourceTypeMaterializationOwner owner =
      SourceTypeMaterializationOwner::None;
  SourceTypeMaterializationOperation operation =
      SourceTypeMaterializationOperation::None;
  const CppAstNode * source_root = nullptr;
  const CppAstNode * semantic_owner_root = nullptr;
  std::uint32_t source_occurrence_id = 0;
  const void * semantic_owner = nullptr;
  SourceTypeMaterializationOwner semantic_owner_kind =
      SourceTypeMaterializationOwner::None;
  bool semantic_owner_committed = false;
};

thread_local std::vector<SourceTypeMaterializationFrame>
    source_type_materialization_stack;

bool has_nonempty_location(const std::vector<std::string> & locations)
{
  for(std::size_t i = 0; i < locations.size(); ++i) {
    if(!locations[i].empty()) {
      return true;
    }
  }
  return false;
}

bool template_parameter_is_function_pointer_value(
    const template_model::TemplateParameterInfo & parameter)
{
  if(parameter.kind != template_model::TemplateParameterInfo::TP_NON_TYPE ||
     !parameter.value_type) {
    return false;
  }
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(parameter.value_type);
  if(!base || base->kind != cpp_decl::Type::TK_POINTER || !base->inner) {
    return false;
  }
  cpp_decl::TypePtr pointee = cpp_decl::strip_top_level_cv(base->inner);
  return pointee && pointee->kind == cpp_decl::Type::TK_FUNCTION;
}

bool template_argument_text_matches(const std::string & left,
                                    const std::string & right)
{
  return semantic_utils::trim_space(left) == semantic_utils::trim_space(right);
}

std::string template_witness_argument_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg);

std::string template_witness_node_text(SemanticContext & ctx,
                                       const CppAstNode & node);

std::string template_witness_substituted_default_text(
    SemanticContext & ctx,
    const std::string & text,
    const std::vector<template_model::TemplateParameterInfo> * all_parameters,
    const std::vector<template_model::TemplateArgument> * all_arguments,
    std::size_t parameter_index);

bool template_witness_text_matches_default(
    const template_model::TemplateParameterInfo & parameter,
    const std::string & text,
    const std::string & default_text);

TemplateIdSourceArgumentKey make_template_id_source_argument_key(
    const std::string & location,
    const std::string & template_name)
{
  return std::make_pair(normalize_template_witness_source_location(location),
                        template_name);
}

std::string created_new_detail(bool created_new)
{
  return std::string("created-new=") + (created_new ? "yes" : "no");
}

std::string function_instantiation_detail(bool created_new,
                                          bool definition_materialized)
{
  std::ostringstream detail;
  detail << created_new_detail(created_new)
         << " definition-materialized="
         << (definition_materialized ? "yes" : "no");
  return detail.str();
}

// template-boundary-audit: begin canonical_key_metadata
bool class_info_has_template_instantiation_key(
    const semantic_model::ClassInfo & info)
{
  return !semantic_model::class_instantiation_key(info).empty();
}

const std::string & function_binding_template_instantiation_key(
    const semantic_model::FunctionBinding & binding)
{
  return binding.template_instantiation_key;
}

bool function_binding_has_empty_template_identity_key(
    const semantic_model::FunctionBinding & binding)
{
  return function_binding_template_instantiation_key(binding).empty();
}
// template-boundary-audit: end canonical_key_metadata

NonTypeArgumentStatus to_api_non_type_argument_status(
    template_argument_semantics::NonTypeArgumentStatus status)
{
  switch(status) {
  case template_argument_semantics::NT_ARG_PARSE_FAILED:
    return NT_ARG_PARSE_FAILED;
  case template_argument_semantics::NT_ARG_DEPENDENT:
    return NT_ARG_DEPENDENT;
  case template_argument_semantics::NT_ARG_EVAL_FAILED:
    return NT_ARG_EVAL_FAILED;
  case template_argument_semantics::NT_ARG_EVALUATED:
    return NT_ARG_EVALUATED;
  }
  return NT_ARG_EVAL_FAILED;
}

MatchKind to_api_match_kind(template_selection::MatchKind kind)
{
  switch(kind) {
  case template_selection::MS_EXPLICIT_SPECIALIZATION:
    return MS_EXPLICIT_SPECIALIZATION;
  case template_selection::MS_PARTIAL_SPECIALIZATION:
    return MS_PARTIAL_SPECIALIZATION;
  case template_selection::MS_PRIMARY:
  default:
    return MS_PRIMARY;
  }
}

ClassSpecializationSelection to_api_class_specialization_selection_impl(
    const template_selection::ClassSpecializationSelection & selection)
{
  ClassSpecializationSelection out;
  out.class_node = selection.class_node;
  out.binding_scope = selection.binding_scope;
  out.parameters = selection.parameters;
  out.arguments = selection.arguments;
  out.argument_syntaxes = selection.argument_syntaxes;
  out.pack_sizes = selection.pack_sizes;
  out.selection_key = selection.selection_key;
  out.value_dependencies = selection.value_dependencies;
  out.kind = to_api_match_kind(selection.kind);
  out.reentrant_primary = selection.reentrant_primary;
  return out;
}

VariableSpecializationSelection to_api_variable_specialization_selection_impl(
    const template_selection::VariableSpecializationSelection & selection)
{
  VariableSpecializationSelection out;
  out.binding_scope = selection.binding_scope;
  out.parameters = selection.parameters;
  out.arguments = selection.arguments;
  out.pack_sizes = selection.pack_sizes;
  out.specifiers = selection.specifiers;
  out.declarator = selection.declarator;
  out.initializer = selection.initializer;
  out.selection_key = selection.selection_key;
  out.kind = to_api_match_kind(selection.kind);
  return out;
}

ParsedFunctionTemplateSignature to_api_parsed_function_template_signature(
    const template_function_signature::ParsedFunctionTemplateSignature & parsed)
{
  ParsedFunctionTemplateSignature out;
  out.name = parsed.name;
  out.type = parsed.type;
  out.params = parsed.params;
  out.default_arguments = parsed.default_arguments;
  out.parameter_declarations = parsed.parameter_declarations;
  out.result_type_pattern = parsed.result_type_pattern;
  out.effective_declarator = parsed.effective_declarator;
  return out;
}

std::string strip_at_prefix(const std::string & location)
{
  if(location.compare(0, 4, " at ") == 0) {
    return location.substr(4);
  }
  return location;
}

std::string current_template_log_location(SemanticContext & ctx)
{
  const std::string location =
      strip_at_prefix(ctx.template_witness_context().public_use_location);
  if(!location.empty()) {
    return location;
  }
  return template_api::current_template_witness_entry_context().trigger_decl_location;
}

bool closure_template_log_enabled(SemanticContext & ctx)
{
  return template_api::current_template_witness_entry_context().origin ==
      TemplateWitnessOrigin::Closure &&
      ctx.template_witness_context().session != nullptr;
}

bool class_is_named_function_local_for_witness(
    const semantic_model::ClassInfo * info)
{
  if(!info || info->name.empty() || info->source_is_unnamed_class) {
    return false;
  }
  if(info->class_kind == "enum" ||
     info->class_kind == "enum class" ||
     info->class_kind == "enum struct") {
    return false;
  }
  return info->source_is_named_function_local_class;
}

bool function_local_type_argument_text(SemanticContext & ctx,
                                       const cpp_decl::TypePtr & type,
                                       std::string & out,
                                       unsigned depth = 0)
{
  out.clear();
  if(!type || depth > 8) {
    return false;
  }
  cpp_decl::TypePtr base = type;
  if(base->kind == cpp_decl::Type::TK_CV) {
    std::string inner;
    if(!function_local_type_argument_text(ctx, base->inner, inner, depth + 1)) {
      return false;
    }
    if(base->cv_const) {
      inner = std::string("const ") + inner;
    }
    if(base->cv_volatile) {
      inner = std::string("volatile ") + inner;
    }
    out = inner;
    return true;
  }
  if(base->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ||
     base->kind == cpp_decl::Type::TK_RVALUE_REFERENCE) {
    std::string inner;
    if(!function_local_type_argument_text(ctx, base->inner, inner, depth + 1)) {
      return false;
    }
    out = inner + (base->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ? " &" : " &&");
    return true;
  }
  if(base->kind == cpp_decl::Type::TK_POINTER) {
    std::string inner;
    if(!function_local_type_argument_text(ctx, base->inner, inner, depth + 1)) {
      return false;
    }
    out = inner + " *";
    return true;
  }
  if(base->kind != cpp_decl::Type::TK_NAMED) {
    return false;
  }
  semantic_model::ClassInfo * info = ctx.class_info_for_type(base);
  if(!class_is_named_function_local_for_witness(info)) {
    return false;
  }
  out = info->name;
  return true;
}

bool template_argument_contains_named_function_local_type_for_witness(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument)
{
  if(argument.kind != template_model::TemplateArgument::TA_TYPE ||
     !argument.type) {
    return false;
  }
  std::string text;
  return function_local_type_argument_text(ctx, argument.type, text);
}

const semantic_model::ClassInfo * class_witness_owner(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  cpp_decl::TypePtr type = cpp_decl::strip_top_level_cv(info.type);
  if(type && type->kind == cpp_decl::Type::TK_NAMED) {
    cpp_decl::TypePtr owner_type =
        cpp_decl::strip_top_level_cv(
            type->named_rare().named_member_owner_type);
    if(owner_type && owner_type->kind == cpp_decl::Type::TK_NAMED) {
      semantic_model::ClassInfo * owner =
          owner_type->named_rare().named_class_info;
      if(!owner) {
        owner = ctx.class_info_for_type(owner_type);
      }
      if(owner && owner != &info) {
        return owner;
      }
    }
  }
  for(const semantic_model::Scope * scope = info.enclosing_scope;
      scope;
      scope = scope->parent) {
    if(scope->class_info && scope->class_info != &info) {
      return scope->class_info;
    }
  }
  return nullptr;
}

std::string conversion_result_template_name(const CppAstNode & node)
{
  if(const cpp_decl::TemplateIdSyntax * syntax =
         cppast_template_id_syntax(node)) {
    if(!syntax->name.name.empty()) {
      return syntax->name.name;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const std::string name = conversion_result_template_name(node.children[i]);
    if(!name.empty()) {
      return name;
    }
  }
  return std::string();
}

std::string conversion_result_pattern_template_name(
    const semantic_model::FunctionBinding & binding)
{
  if(!binding.source_template) {
    return std::string();
  }
  cpp_decl::TypePtr pattern = cpp_decl::strip_top_level_cv(
      binding.source_template->type_pattern);
  if(pattern && pattern->kind == cpp_decl::Type::TK_FUNCTION) {
    pattern = pattern->inner;
  }
  if(pattern &&
     (pattern->kind == cpp_decl::Type::TK_POINTER ||
      pattern->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ||
      pattern->kind == cpp_decl::Type::TK_RVALUE_REFERENCE)) {
    pattern = pattern->inner;
  }
  pattern = cpp_decl::strip_top_level_cv(pattern);
  void * class_template_identity = nullptr;
  std::vector<cpp_decl::DependentAliasTemplateArgumentSyntax> arguments;
  if(pattern &&
     cpp_decl::named_type_dependent_class_template(
         pattern,
         class_template_identity,
         arguments) &&
     class_template_identity) {
    const semantic_model::ClassTemplateDecl * decl =
        static_cast<const semantic_model::ClassTemplateDecl *>(
            class_template_identity);
    if(decl && !decl->name.empty()) {
      return semantic_utils::unqualified_member_name(decl->name);
    }
  }
  return conversion_result_template_name(
      binding.source_template->result_type_pattern);
}

std::string conversion_result_base_type_text(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base) {
    return std::string();
  }
  if(base->kind == cpp_decl::Type::TK_NAMED) {
    if(semantic_model::ClassInfo * info = ctx.class_info_for_type(base)) {
      return class_witness_output_qualified_name(ctx, *info);
    }
    return semantic_utils::strip_elaborated_type_prefix(
        cpp_decl::named_type_display_text(base));
  }
  return cpp_decl::template_argument_type_text(base);
}

std::string conversion_operator_log_entity(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding & binding,
    const std::string & owner)
{
  cpp_decl::TypePtr function_type = cpp_decl::strip_top_level_cv(
      binding.declared_type ? binding.declared_type : binding.type);
  if(!binding.is_conversion_operator ||
     !function_type ||
     function_type->kind != cpp_decl::Type::TK_FUNCTION ||
     !function_type->inner ||
     owner.empty()) {
    return std::string();
  }

  cpp_decl::TypePtr target = function_type->inner;
  enum class Indirection { None, Pointer, LvalueReference, RvalueReference };
  Indirection indirection = Indirection::None;
  if(target->kind == cpp_decl::Type::TK_POINTER) {
    indirection = Indirection::Pointer;
    target = target->inner;
  } else if(target->kind == cpp_decl::Type::TK_LVALUE_REFERENCE) {
    indirection = Indirection::LvalueReference;
    target = target->inner;
  } else if(target->kind == cpp_decl::Type::TK_RVALUE_REFERENCE) {
    indirection = Indirection::RvalueReference;
    target = target->inner;
  }

  std::string cv;
  if(target && target->kind == cpp_decl::Type::TK_CV) {
    if(target->cv_const) {
      cv = "const";
    }
    if(target->cv_volatile) {
      if(!cv.empty()) {
        cv += " ";
      }
      cv += "volatile";
    }
    target = target->inner;
  }

  std::string target_text;
  if(binding.source_template) {
    target_text = conversion_result_pattern_template_name(binding);
  }
  if(target_text.empty()) {
    target_text = conversion_result_base_type_text(ctx, target);
  }
  // Clang names a by-value conversion function by the converted class
  // template, alias target, or deduced class name, not by concrete class
  // specialization arguments.  Reference and pointer conversion names retain
  // their concrete target spelling, so keep those on the full typed path.
  if(indirection == Indirection::None) {
    target_text = semantic_utils::strip_trailing_top_level_template_arguments(
        semantic_utils::unqualified_member_name(target_text));
  }
  target_text = semantic_utils::strip_elaborated_type_prefix(target_text);
  if(target_text.empty()) {
    return std::string();
  }

  std::string suffix;
  switch(indirection) {
  case Indirection::Pointer:
    suffix = " *";
    break;
  case Indirection::LvalueReference:
    suffix = " &";
    break;
  case Indirection::RvalueReference:
    suffix = " &&";
    break;
  case Indirection::None:
    break;
  }

  if(indirection == Indirection::Pointer && !cv.empty()) {
    return cv + " " + owner + "::operator " + target_text + suffix;
  }
  return owner + "::" + (cv.empty() ? std::string() : cv + " ") +
      "operator " + target_text + suffix;
}

std::string binding_log_entity(SemanticContext & ctx,
                               const semantic_model::FunctionBinding * binding)
{
  if(!binding) {
    return std::string();
  }
  const std::string simple_name =
      semantic_model::function_binding_display_name_for_symbol(*binding);
  if(binding->owner_class &&
     !binding->owner_class->qualified_name.empty()) {
    std::string owner =
        binding->owner_class->source_template ?
            class_witness_output_qualified_name_for_lifecycle(
                ctx,
                *binding->owner_class) :
            class_witness_output_qualified_name(
                ctx,
                *binding->owner_class);
    if(binding->is_conversion_operator) {
      const std::string conversion_entity =
          conversion_operator_log_entity(ctx, *binding, owner);
      if(!conversion_entity.empty()) {
        return conversion_entity;
      }
    }
    return owner + "::" + simple_name;
  }
  return semantic_model::function_binding_qualified_name_for_symbol(*binding);
}

std::string binding_decl_location(SemanticContext & ctx,
                                  const semantic_model::FunctionBinding * binding)
{
  const semantic_model::SourceDeclAnchorCache & anchor =
      semantic_trace::function_binding_decl_anchor(ctx, binding);
  return strip_at_prefix(semantic_model::source_decl_anchor_location(anchor));
}

std::string strip_elaborated_prefix_for_anonymous_entity(const std::string & text)
{
  static const char * prefixes[] = {
      "enum class ",
      "enum struct ",
      "class ",
      "struct ",
      "union ",
      "enum "};
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const std::string prefix = prefixes[i];
    if(text.compare(0, prefix.size(), prefix) == 0) {
      return text.substr(prefix.size());
    }
  }
  return text;
}

std::string anonymous_entity_source_location_text(const std::string & location)
{
  template_witness_detail::ParsedSourceLocation parsed =
      template_witness_detail::parse_source_location(location);
  if(!parsed.valid) {
    return strip_at_prefix(location);
  }
  std::ostringstream out;
  out << parsed.line << ":" << parsed.column;
  return out.str();
}

const semantic_model::FunctionBinding * lexical_function_for_scope(
    const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->function) {
      return current->function;
    }
  }
  return nullptr;
}

const semantic_model::FunctionBinding * lexical_function_for_class(
    const semantic_model::ClassInfo * info)
{
  return info ? lexical_function_for_scope(info->enclosing_scope) : nullptr;
}

std::string function_scope_entity_for_anonymous_class(
    const semantic_model::FunctionBinding & binding)
{
  std::ostringstream out;
  out << semantic_model::function_binding_qualified_name_for_symbol(binding)
      << "(";
  bool wrote_param = false;
  for(std::size_t i = 0; i < binding.params.size(); ++i) {
    if(binding.params[i].first == "this") {
      continue;
    }
    if(wrote_param) {
      out << ", ";
    }
    out << template_witness_detail::normalize_template_log_type_spellings(
        strip_elaborated_prefix_for_anonymous_entity(
            cpp_decl::describe_type(binding.params[i].second)));
    wrote_param = true;
  }
  out << ")";
  return out.str();
}

std::string source_unnamed_class_node_location(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  const CppAstNode * node =
      info.source_unnamed_class_node ? info.source_unnamed_class_node :
      (info.template_output_node ? info.template_output_node : info.class_node);
  return node ? anonymous_entity_source_location_text(ctx.source_location_for_node(*node)) :
                std::string();
}

std::string source_unnamed_class_short_entity(
    const semantic_model::ClassInfo & info,
    bool include_location,
    const std::string & location)
{
  std::ostringstream out;
  out << "(unnamed " << info.class_kind;
  if(include_location && !location.empty()) {
    out << " at " << location;
  }
  out << ")";
  return out.str();
}

std::string source_unnamed_class_log_entity(SemanticContext & ctx,
                                            const semantic_model::ClassInfo & info)
{
  const std::string location = source_unnamed_class_node_location(ctx, info);
  const semantic_model::ClassInfo * parent =
      info.enclosing_scope ? info.enclosing_scope->class_info : nullptr;
  if(parent && parent->source_is_unnamed_class) {
    std::ostringstream out;
    if(const semantic_model::FunctionBinding * function =
           lexical_function_for_class(parent)) {
      out << function_scope_entity_for_anonymous_class(*function) << "::";
    } else if(parent->enclosing_scope && parent->enclosing_scope->class_info) {
      out << class_witness_output_qualified_name(
          ctx,
          *parent->enclosing_scope->class_info) << "::";
    }
    out << source_unnamed_class_short_entity(*parent, false, std::string())
        << "::"
        << source_unnamed_class_short_entity(info, true, location);
    return out.str();
  }
  if(!lexical_function_for_class(&info) && parent) {
    return class_witness_output_qualified_name(ctx, *parent) + "::" +
        source_unnamed_class_short_entity(info, true, location);
  }
  return source_unnamed_class_short_entity(info, true, location);
}

std::string anonymous_member_class_log_entity(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & owner,
    const semantic_model::AnonymousMemberClassInfo & member)
{
  const std::string location =
      member.class_node ?
          anonymous_entity_source_location_text(ctx.source_location_for_node(*member.class_node)) :
          std::string();
  std::ostringstream out;
  if(owner.source_is_unnamed_class) {
    if(const semantic_model::FunctionBinding * function =
           lexical_function_for_class(&owner)) {
      out << function_scope_entity_for_anonymous_class(*function) << "::";
    }
    out << source_unnamed_class_short_entity(owner, false, std::string()) << "::";
  } else {
    out << class_witness_output_qualified_name(ctx, owner) << "::";
  }
  out << "(anonymous " << member.class_kind;
  if(!location.empty()) {
    out << " at " << location;
  }
  out << ")";
  return out.str();
}

std::string class_log_entity(SemanticContext & ctx,
                             const semantic_model::ClassInfo * info)
{
  if(!info) {
    return std::string();
  }
  std::string entity;
  if(info->source_is_unnamed_class) {
    entity = source_unnamed_class_log_entity(ctx, *info);
  } else if(class_is_named_function_local_for_witness(info)) {
    entity = info->name;
  } else {
    entity = class_witness_output_qualified_name_for_lifecycle(ctx, *info);
  }
  return entity;
}

std::string class_decl_location(SemanticContext & ctx,
                                const semantic_model::ClassInfo * info)
{
  const semantic_model::SourceDeclAnchorCache & anchor =
      semantic_trace::class_decl_anchor(ctx, info);
  return strip_at_prefix(semantic_model::source_decl_anchor_location(anchor));
}

bool class_is_unnamed_for_witness(const semantic_model::ClassInfo * info)
{
  if(!info) {
    return false;
  }
  if(info->source_is_unnamed_class) {
    return true;
  }
  const CppAstNode * node =
      info->template_output_node ? info->template_output_node : info->class_node;
  return node &&
      (node->kind == CppAstKind::class_specifier ||
      node->kind == CppAstKind::class_forward_declaration) &&
      node->value.empty();
}

std::string anonymous_symbol_class_log_name(
    const semantic_model::ClassInfo & info)
{
  const std::string symbol_text =
      !semantic_model::class_symbol_qualified_name_syntax(info).name.empty() ?
          qualified_name_text(
              semantic_model::class_symbol_qualified_name_syntax(info)) :
          std::string();
  if(symbol_text.find("_GLOBAL__N_") != std::string::npos) {
    return symbol_text;
  }
  if(info.qualified_name.find("_GLOBAL__N_") != std::string::npos) {
    return info.qualified_name;
  }
  return std::string();
}

std::string value_log_entity(SemanticContext & ctx,
                             const semantic_model::ValueBinding * binding)
{
  if(!binding) {
    return std::string();
  }
  if(binding->declaration_scope && binding->declaration_scope->class_info &&
     !binding->declaration_scope->class_info->qualified_name.empty()) {
    const semantic_model::ClassInfo & owner =
        *binding->declaration_scope->class_info;
    return class_witness_output_qualified_name(ctx, owner) +
        "::" + binding->name;
  }
  return binding->name;
}

const semantic_model::ClassInfo * nearest_scope_class_info(
    const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope;
      current;
      current = current->parent) {
    if(current->class_info) {
      return current->class_info;
    }
  }
  return nullptr;
}

std::string stable_template_parameter_log_name(
    const template_model::TemplateParameterInfo & parameter,
    std::size_t index)
{
  switch(parameter.kind) {
  case template_model::TemplateParameterInfo::TP_NON_TYPE:
    return std::string("value-parameter-0-") + std::to_string(index);
  case template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE:
    return std::string("template-parameter-0-") + std::to_string(index);
  case template_model::TemplateParameterInfo::TP_TYPE:
  default:
    return std::string("type-parameter-0-") + std::to_string(index);
  }
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

std::string normalize_pattern_function_type_arg_text(
    const std::string & text,
    const cpp_decl::TemplateArgumentSyntax * syntax)
{
  if(!(syntax && syntax->type_id &&
       ast_contains_kind(*syntax->type_id, CppAstKind::parameter_clause))) {
    return text;
  }
  int angle_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '<') {
      ++angle_depth;
      continue;
    }
    if(text[i] == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(text[i] != '(' || angle_depth != 0) {
      continue;
    }
    std::size_t previous = i;
    while(previous > 0 &&
          std::isspace(static_cast<unsigned char>(text[previous - 1]))) {
      --previous;
    }
    if(previous == 0 || previous != i) {
      return text;
    }
    std::string out = text;
    out.insert(i, " ");
    return out;
  }
  return text;
}

std::string canonicalize_template_parameter_log_text(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::string & text)
{
  std::string out = text;
  bool changed = false;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const std::string replacement =
        stable_template_parameter_log_name(parameters[i], i);
    if(!parameters[i].placeholder_key.empty()) {
      out = callsemantic_internal::replace_identifier_token_text(
          out,
          parameters[i].placeholder_key,
          replacement,
          changed);
    }
    if(!parameters[i].name.empty()) {
      out = callsemantic_internal::replace_identifier_token_text(
          out,
          parameters[i].name,
          replacement,
          changed);
    }
    for(std::size_t j = 0; j < parameters[i].alternate_names.size(); ++j) {
      out = callsemantic_internal::replace_identifier_token_text(
          out,
          parameters[i].alternate_names[j],
          replacement,
          changed);
    }
  }
  return out;
}

const semantic_model::PartialClassTemplateSpecializationDecl *
selected_partial_log_decl(const semantic_model::ClassInfo * owner)
{
  if(!(owner &&
       owner->source_template &&
       owner->template_output_node &&
       owner->source_template->class_node &&
       owner->template_output_node != owner->source_template->class_node)) {
    return nullptr;
  }
  for(std::size_t i = 0;
      i < owner->source_template->partial_specializations.size();
      ++i) {
    const semantic_model::PartialClassTemplateSpecializationDecl & partial =
        owner->source_template->partial_specializations[i];
    if(partial.class_node == owner->template_output_node) {
      return &partial;
    }
  }
  return nullptr;
}

std::string class_source_template_log_owner(
    const semantic_model::ClassInfo * owner)
{
  if(!owner ||
     owner->is_explicit_specialization) {
    return std::string();
  }
  if(!owner->source_template) {
    return owner->qualified_name.find('<') != std::string::npos ?
        semantic_model::class_output_qualified_name(*owner) :
        std::string();
  }
  if(!owner->source_template->class_node ||
     owner->source_template->class_node->value.empty()) {
    return std::string();
  }
  const std::string source_owner =
      owner->source_template->declaring_scope ?
          semantic_lookup::scope_qualified_name(
              *owner->source_template->declaring_scope,
              owner->source_template->class_node->value) :
          owner->source_template->class_node->value;
  if(const semantic_model::PartialClassTemplateSpecializationDecl * partial =
         selected_partial_log_decl(owner)) {
    if(partial->arg_texts.empty()) {
      return source_owner;
    }
    std::ostringstream out;
    out << source_owner << "<";
    for(std::size_t i = 0; i < partial->arg_texts.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      const cpp_decl::TemplateArgumentSyntax * syntax =
          i < partial->arg_syntaxes.size() ? &partial->arg_syntaxes[i] : nullptr;
      out << canonicalize_template_parameter_log_text(
          partial->parameters,
          normalize_pattern_function_type_arg_text(partial->arg_texts[i],
                                                   syntax));
    }
    out << ">";
    return out.str();
  }
  if(owner->template_output_node &&
     owner->source_template->class_node &&
     owner->template_output_node != owner->source_template->class_node &&
     !owner->instantiation_arg_texts.empty()) {
    std::ostringstream out;
    out << source_owner << "<";
    for(std::size_t i = 0; i < owner->instantiation_arg_texts.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << owner->instantiation_arg_texts[i];
    }
    out << ">";
    return out.str();
  }
  return source_owner;
}

std::string alias_template_log_entity(
    const semantic_model::AliasTemplateDecl * decl)
{
  if(!decl) {
    return std::string();
  }
  if(decl->declaring_scope) {
    std::string source_owner;
    if(decl->pattern_scope) {
      source_owner =
          class_source_template_log_owner(
              nearest_scope_class_info(decl->pattern_scope));
    }
    if(source_owner.empty()) {
      source_owner =
          class_source_template_log_owner(
              nearest_scope_class_info(decl->declaring_scope));
    }
    if(!source_owner.empty()) {
      return source_owner + "::" + decl->name;
    }
    return semantic_lookup::scope_qualified_name(*decl->declaring_scope,
                                                 decl->name);
  }
  return decl->name;
}

std::string value_decl_location(SemanticContext & ctx,
                                const semantic_model::ValueBinding * binding)
{
  const semantic_model::SourceDeclAnchorCache & anchor =
      semantic_trace::value_decl_anchor(ctx, binding);
  return strip_at_prefix(semantic_model::source_decl_anchor_location(anchor));
}

std::string variable_template_decl_log_entity(
    const semantic_model::VariableTemplateDecl * decl)
{
  if(!decl) {
    return std::string();
  }
  if(decl->declaring_scope) {
    return semantic_lookup::scope_qualified_name(*decl->declaring_scope,
                                                 decl->name);
  }
  return decl->name;
}

std::string variable_template_decl_location(
    SemanticContext & ctx,
    const semantic_model::VariableTemplateDecl * decl)
{
  const semantic_model::SourceDeclAnchorCache & anchor =
      semantic_trace::variable_template_decl_anchor(ctx, decl);
  return strip_at_prefix(semantic_model::source_decl_anchor_location(anchor));
}

semantic_model::ClassInfo * enclosing_template_instantiation_owner(
    semantic_model::ClassInfo * info)
{
  while(info) {
    if(info->source_template &&
       !info->dependent_instantiation &&
       !info->is_explicit_specialization) {
      return info;
    }
    info = info->enclosing_scope ? info->enclosing_scope->class_info : nullptr;
  }
  return nullptr;
}

bool class_has_template_identity_impl(const semantic_model::ClassInfo * info)
{
  for(const semantic_model::ClassInfo * current = info; current; ) {
    if(current->source_template || class_info_has_template_instantiation_key(*current)) {
      return true;
    }
    current = current->enclosing_scope ? current->enclosing_scope->class_info : nullptr;
  }
  return false;
}

bool scope_has_template_owner_identity_impl(const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->class_info && class_has_template_identity_impl(current->class_info)) {
      return true;
    }
  }
  return false;
}

bool class_has_linkage_template_identity_impl(const semantic_model::ClassInfo * info)
{
  for(const semantic_model::ClassInfo * current = info; current; ) {
    if((current->source_template ||
        class_info_has_template_instantiation_key(*current)) &&
       !current->is_explicit_specialization) {
      return true;
    }
    current = current->enclosing_scope ? current->enclosing_scope->class_info : nullptr;
  }
  return false;
}

bool scope_has_linkage_template_owner_identity_impl(const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope; current; current = current->parent) {
    if(current->class_info &&
       class_has_linkage_template_identity_impl(current->class_info)) {
      return true;
    }
  }
  return false;
}

bool scope_has_internal_namespace_linkage_impl(const semantic_model::Scope * scope)
{
  for(const semantic_model::Scope * current = scope;
      current;
      current = current->parent) {
    if(current->namespace_scope && current->name == "<unnamed>") {
      return true;
    }
  }
  return false;
}

bool type_identity_has_internal_namespace_linkage_impl(
    const cpp_decl::TypePtr & type,
    std::set<const cpp_decl::Type *> & visited_types,
    std::set<const semantic_model::ClassInfo *> & visited_classes);

bool template_arguments_have_internal_namespace_linkage_impl(
    const std::vector<template_model::TemplateArgument> & arguments,
    std::set<const cpp_decl::Type *> & visited_types,
    std::set<const semantic_model::ClassInfo *> & visited_classes)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(type_identity_has_internal_namespace_linkage_impl(
           arguments[i].type, visited_types, visited_classes) ||
       type_identity_has_internal_namespace_linkage_impl(
           arguments[i].template_owner_type, visited_types, visited_classes)) {
      return true;
    }
  }
  return false;
}

bool class_identity_has_internal_namespace_linkage_impl(
    const semantic_model::ClassInfo * info,
    std::set<const cpp_decl::Type *> & visited_types,
    std::set<const semantic_model::ClassInfo *> & visited_classes)
{
  if(!info) {
    return false;
  }
  if(scope_has_internal_namespace_linkage_impl(info->enclosing_scope)) {
    return true;
  }
  if(!visited_classes.insert(info).second) {
    return false;
  }
  if(template_arguments_have_internal_namespace_linkage_impl(
         info->instantiation_arguments, visited_types, visited_classes) ||
     template_arguments_have_internal_namespace_linkage_impl(
         info->instantiation_binding_arguments, visited_types, visited_classes)) {
    return true;
  }
  if(info->instantiation_binding_arguments_view &&
     info->instantiation_binding_arguments_view != &info->instantiation_arguments &&
     info->instantiation_binding_arguments_view !=
         &info->instantiation_binding_arguments &&
     template_arguments_have_internal_namespace_linkage_impl(
         *info->instantiation_binding_arguments_view,
         visited_types,
         visited_classes)) {
    return true;
  }
  return info->enclosing_scope &&
         class_identity_has_internal_namespace_linkage_impl(
             info->enclosing_scope->class_info, visited_types, visited_classes);
}

bool type_identity_has_internal_namespace_linkage_impl(
    const cpp_decl::TypePtr & type,
    std::set<const cpp_decl::Type *> & visited_types,
    std::set<const semantic_model::ClassInfo *> & visited_classes)
{
  if(!type || !visited_types.insert(type.get()).second) {
    return false;
  }
  if(type->kind == cpp_decl::Type::TK_NAMED &&
     class_identity_has_internal_namespace_linkage_impl(
         type->named_rare().named_class_info, visited_types, visited_classes)) {
    return true;
  }
  if(type_identity_has_internal_namespace_linkage_impl(
         type->inner, visited_types, visited_classes) ||
     type_identity_has_internal_namespace_linkage_impl(
         type->owner, visited_types, visited_classes)) {
    return true;
  }
  for(std::size_t i = 0; i < type->params.size(); ++i) {
    if(type_identity_has_internal_namespace_linkage_impl(
           type->params[i], visited_types, visited_classes)) {
      return true;
    }
  }
  return false;
}

bool function_binding_identity_has_internal_namespace_linkage_impl(
    const semantic_model::FunctionBinding * binding)
{
  if(!binding) {
    return false;
  }
  std::set<const cpp_decl::Type *> visited_types;
  std::set<const semantic_model::ClassInfo *> visited_classes;
  return class_identity_has_internal_namespace_linkage_impl(
             binding->owner_class, visited_types, visited_classes) ||
         class_identity_has_internal_namespace_linkage_impl(
             binding->lexical_access_class, visited_types, visited_classes) ||
         template_arguments_have_internal_namespace_linkage_impl(
             binding->instantiation_arguments, visited_types, visited_classes);
}

bool value_binding_has_template_identity_impl(const semantic_model::ValueBinding * binding)
{
  return binding &&
         (binding->variable_template_instantiation ||
          binding->dependent_template_value ||
          class_has_template_identity_impl(binding->owner_class) ||
          scope_has_template_owner_identity_impl(binding->declaration_scope));
}

bool function_binding_is_template_owned_for_definition_closure(
    const semantic_model::FunctionBinding * binding)
{
  return binding &&
         (binding->source_template ||
          enclosing_template_instantiation_owner(binding->owner_class) != nullptr ||
          class_has_template_identity_impl(binding->lexical_access_class));
}

const CppAstNode * find_direct_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

bool node_marks_exclude_from_explicit_instantiation(const CppAstNode * node)
{
  if(!node) {
    return false;
  }
  if(node->has_exclude_from_explicit_instantiation) {
    return true;
  }
  const CppAstNode * decl_specifiers =
      find_direct_child_kind(*node, CppAstKind::decl_specifier_seq);
  if(decl_specifiers && decl_specifiers->has_exclude_from_explicit_instantiation) {
    return true;
  }
  const CppAstNode * member_specifiers =
      find_direct_child_kind(*node, CppAstKind::member_specifiers);
  if(member_specifiers &&
     member_specifiers->has_exclude_from_explicit_instantiation) {
    return true;
  }
  return cpp_decl::node_text(*node).find("exclude_from_explicit_instantiation") !=
      std::string::npos;
}

bool class_has_noop_default_constructor_storage_shape(
    const semantic_model::ClassInfo & info)
{
  if(info.class_kind == "union" || info.is_polymorphic || !info.fields.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type ||
       info.bases[i].is_virtual ||
       !class_has_noop_default_constructor_storage_shape(*info.bases[i].type)) {
      return false;
    }
  }
  return true;
}

bool mem_initializer_has_single_identifier_argument(const CppAstNode & init,
                                                    const std::string & source_name)
{
  if(init.children.size() < 2) {
    return false;
  }

  const CppAstNode & arg_list = init.children[1];
  if(arg_list.kind != CppAstKind::paren_argument_list &&
     arg_list.kind != CppAstKind::argument_list &&
     arg_list.kind != CppAstKind::braced_init_list) {
    return false;
  }
  if(arg_list.children.size() != 1) {
    return false;
  }

  const CppAstNode & arg = arg_list.children[0];
  if(arg.kind != CppAstKind::id_expression &&
     arg.kind != CppAstKind::identifier) {
    return false;
  }
  return arg.value == source_name;
}

bool binding_is_noop_default_constructor_for_suppressed_emission(
    const semantic_model::FunctionBinding & binding)
{
  if(!binding.owner_class ||
     !binding.is_constructor ||
     binding.is_deleted ||
     binding.params.size() != 1 ||
     !symbol_linkage::has_weak_linkage(binding.symbol) ||
     !class_has_noop_default_constructor_storage_shape(*binding.owner_class)) {
    return false;
  }

  return binding.synthesized ||
         binding.is_defaulted ||
         binding.is_aggregate_constructor ||
         (binding.body &&
          binding.body->kind == CppAstKind::compound_statement &&
          binding.body->children.empty() &&
          !binding.ctor_initializer);
}

bool binding_is_noop_copy_constructor_for_suppressed_emission(
    const semantic_model::FunctionBinding & binding)
{
  const std::string source_name =
      semantic_model::function_parameter_alias_name(binding, 1).empty() ?
          semantic_model::function_parameter_binding_name(binding, 1) :
          semantic_model::function_parameter_alias_name(binding, 1);
  if(!binding.owner_class ||
     !binding.is_constructor ||
     binding.is_deleted ||
     binding.params.size() != 2 ||
     source_name.empty() ||
     !symbol_linkage::has_weak_linkage(binding.symbol) ||
     !class_has_noop_default_constructor_storage_shape(*binding.owner_class)) {
    return false;
  }

  if(binding.synthesized || binding.is_defaulted) {
    return true;
  }
  if(!binding.body ||
     binding.body->kind != CppAstKind::compound_statement ||
     !binding.body->children.empty()) {
    return false;
  }
  if(binding.owner_class->bases.empty()) {
    return binding.ctor_initializer == nullptr;
  }
  if(!binding.ctor_initializer ||
     binding.ctor_initializer->children.size() != binding.owner_class->bases.size()) {
    return false;
  }
  for(std::size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    if(!mem_initializer_has_single_identifier_argument(binding.ctor_initializer->children[i],
                                                       source_name)) {
      return false;
    }
  }
  return true;
}

bool binding_is_noop_inline_function_for_suppressed_emission(
    const semantic_model::FunctionBinding & binding)
{
  const bool inline_class_definition =
      binding.owner_class &&
      binding.declaration_node &&
      binding.definition_node &&
      binding.declaration_node == binding.definition_node;
  const bool inline_definition =
      binding.is_inline || binding.is_constexpr || inline_class_definition;
  const bool empty_compound_body =
      binding.body &&
      binding.body->kind == CppAstKind::compound_statement &&
      binding.body->children.empty();
  const bool empty_lazy_body =
      binding.body &&
      binding.body->kind == CppAstKind::lazy_function_body &&
      binding.body->token_end <= binding.body->token_start + 2;
  return binding.owner_class &&
         !binding.is_deleted &&
         inline_definition &&
         (empty_compound_body || empty_lazy_body) &&
         !binding.ctor_initializer;
}

void apply_function_instantiation_intent(SemanticContext & ctx,
                                         semantic_model::FunctionBinding * binding,
                                         TemplateInstantiationIntent intent,
                                         TemplateInstantiationResult * result)
{
  if(!binding) {
    return;
  }
  switch(intent) {
  case TemplateInstantiationIntent::LookupOnly:
    break;

  case TemplateInstantiationIntent::TrackInstantiation:
    ctx.note_instantiated_function_output(binding, InstantiatedFunctionOutputMode::TrackOnly);
    if(result) {
      result->output_tracked = true;
    }
    break;

  case TemplateInstantiationIntent::RequireDefinition:
    ctx.require_function_definition(binding, OutputReason::TemplateUpgrade);
    if(result) {
      result->definition_required = true;
    }
    break;

  case TemplateInstantiationIntent::RequireDefinitionAndExport:
    ctx.require_function_definition(binding, OutputReason::TemplateUpgrade);
    ctx.upgrade_function_symbol_linkage(binding, binding->symbol.linkage);
    if(result) {
      result->definition_required = true;
      result->output_tracked = true;
    }
    break;
  }
}

TemplateLifecycleCause lifecycle_cause_for_instantiation_intent(
    TemplateInstantiationIntent intent)
{
  switch(intent) {
  case TemplateInstantiationIntent::LookupOnly:
    return TemplateLifecycleCause::ImplicitUse;
  case TemplateInstantiationIntent::TrackInstantiation:
    return TemplateLifecycleCause::TrackInstantiation;
  case TemplateInstantiationIntent::RequireDefinition:
    return TemplateLifecycleCause::RequireDefinition;
  case TemplateInstantiationIntent::RequireDefinitionAndExport:
    return TemplateLifecycleCause::ExplicitInstantiationDefinition;
  }
  return TemplateLifecycleCause::ImplicitUse;
}

TemplateLifecycleCause lifecycle_cause_for_current_context_or_intent(
    TemplateInstantiationIntent intent)
{
  const TemplateWitnessEntryContext current =
      current_template_witness_entry_context();
  if(current.origin == TemplateWitnessOrigin::Closure &&
     current.closure_reason != TemplateClosureReason::None) {
    return template_lifecycle_cause_from_closure_reason(current.closure_reason);
  }
  return lifecycle_cause_for_instantiation_intent(intent);
}

TemplateLifecycleCause lifecycle_cause_for_current_context_or_default(
    TemplateLifecycleCause default_cause)
{
  const TemplateWitnessEntryContext current =
      current_template_witness_entry_context();
  if(current.origin == TemplateWitnessOrigin::Closure &&
     current.closure_reason != TemplateClosureReason::None) {
    return template_lifecycle_cause_from_closure_reason(current.closure_reason);
  }
  return default_cause;
}

TemplateClosureReason closure_reason_for_function_binding_acquisition_cause(
    TemplateFunctionBindingAcquisitionCause cause)
{
  switch(cause) {
  case TemplateFunctionBindingAcquisitionCause::None:
    return TemplateClosureReason::None;
  case TemplateFunctionBindingAcquisitionCause::RequireDefinition:
    return TemplateClosureReason::RequireDefinition;
  case TemplateFunctionBindingAcquisitionCause::EnsureDefinition:
    return TemplateClosureReason::EnsureDefinition;
  case TemplateFunctionBindingAcquisitionCause::DefinitionAlreadyEnsured:
    return TemplateClosureReason::RequireDefinition;
  case TemplateFunctionBindingAcquisitionCause::DeclarationInstantiation:
    return TemplateClosureReason::None;
  case TemplateFunctionBindingAcquisitionCause::ExplicitInstantiationDefinition:
    return TemplateClosureReason::TrackInstantiation;
  }
  return TemplateClosureReason::None;
}

}  // namespace

void source_type_materialization_detail::push(
    SourceTypeMaterializationOwner owner,
    SourceTypeMaterializationOperation operation,
    const CppAstNode * source_root,
    const cpp_decl::TemplateIdSyntax * source_syntax,
    const void * semantic_owner,
    bool semantic_owner_committed)
{
  SourceTypeMaterializationFrame frame =
      source_type_materialization_stack.empty() ?
          SourceTypeMaterializationFrame() :
          source_type_materialization_stack.back();
  frame.owner = owner;
  frame.operation = operation;
  if(source_root) {
    frame.source_root = source_root;
  }
  if(source_syntax) {
    frame.source_occurrence_id = source_syntax->source_location_id;
  } else if(source_root) {
    frame.source_occurrence_id = 0;
  }
  if(semantic_owner) {
    frame.semantic_owner = semantic_owner;
    frame.semantic_owner_kind =
        owner == SourceTypeMaterializationOwner::None ?
            SourceTypeMaterializationOwner::DeclarationType : owner;
    frame.semantic_owner_root = source_root;
    frame.semantic_owner_committed = semantic_owner_committed;
  }
  source_type_materialization_stack.push_back(frame);
}

void source_type_materialization_detail::pop()
{
  source_type_materialization_stack.pop_back();
}

SourceTypeMaterializationOwner current_source_type_materialization_owner()
{
  return source_type_materialization_stack.empty() ?
      SourceTypeMaterializationOwner::None :
      source_type_materialization_stack.back().owner;
}

SourceTypeMaterializationOperation
current_source_type_materialization_operation()
{
  return source_type_materialization_stack.empty() ?
      SourceTypeMaterializationOperation::None :
      source_type_materialization_stack.back().operation;
}

namespace {

bool template_id_syntax_contains_source_occurrence(
    const cpp_decl::TemplateIdSyntax & syntax,
    std::uint32_t source_occurrence_id);

bool ast_node_contains_template_id_source_occurrence(
    const CppAstNode & node,
    std::uint32_t source_occurrence_id)
{
  if(node.template_id_syntax &&
     template_id_syntax_contains_source_occurrence(
         *node.template_id_syntax, source_occurrence_id)) {
    return true;
  }
  for(std::size_t i = 0;
      i < node.qualifier_template_id_syntaxes.size();
      ++i) {
    if(template_id_syntax_contains_source_occurrence(
           node.qualifier_template_id_syntaxes[i], source_occurrence_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_contains_template_id_source_occurrence(
           node.qualifier_type_syntaxes[i], source_occurrence_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_contains_template_id_source_occurrence(
           node.exception_type_id_syntaxes[i], source_occurrence_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    if(ast_node_contains_template_id_source_occurrence(
           node.alignment_specifier_nodes[i], source_occurrence_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_contains_template_id_source_occurrence(
           node.children[i], source_occurrence_id)) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_contains_source_occurrence(
    const cpp_decl::TemplateArgumentSyntax & argument,
    std::uint32_t source_occurrence_id)
{
  return
      (argument.template_id &&
       template_id_syntax_contains_source_occurrence(
           *argument.template_id, source_occurrence_id)) ||
      (argument.type_id &&
       ast_node_contains_template_id_source_occurrence(
           *argument.type_id, source_occurrence_id)) ||
      (argument.source_type_id &&
       ast_node_contains_template_id_source_occurrence(
           *argument.source_type_id, source_occurrence_id)) ||
      (argument.expression &&
       ast_node_contains_template_id_source_occurrence(
           *argument.expression, source_occurrence_id));
}

bool template_id_syntax_contains_source_occurrence(
    const cpp_decl::TemplateIdSyntax & syntax,
    std::uint32_t source_occurrence_id)
{
  if(source_occurrence_id != 0 &&
     syntax.source_location_id == source_occurrence_id) {
    return true;
  }
  for(std::size_t i = 0;
      i < syntax.qualifier_template_id_syntaxes.size();
      ++i) {
    if(template_id_syntax_contains_source_occurrence(
           syntax.qualifier_template_id_syntaxes[i], source_occurrence_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_contains_source_occurrence(
           syntax.argument_syntaxes[i], source_occurrence_id)) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool current_source_type_materialization_matches(
    const cpp_decl::TemplateIdSyntax * source_syntax)
{
  if(!source_syntax || source_syntax->source_location_id == 0 ||
     source_type_materialization_stack.empty()) {
    return false;
  }
  const SourceTypeMaterializationFrame & frame =
      source_type_materialization_stack.back();
  const bool operation_matches =
      frame.source_occurrence_id == source_syntax->source_location_id ||
      (frame.source_root &&
       ast_node_contains_template_id_source_occurrence(
           *frame.source_root,
           source_syntax->source_location_id));
  if(!operation_matches) {
    return false;
  }
  return !frame.semantic_owner_root ||
      ast_node_contains_template_id_source_occurrence(
          *frame.semantic_owner_root,
          source_syntax->source_location_id);
}

bool current_source_type_materialization_owner_committed()
{
  return !source_type_materialization_stack.empty() &&
      source_type_materialization_stack.back().semantic_owner &&
      source_type_materialization_stack.back().semantic_owner_committed;
}

bool current_source_type_materialization_commits_semantic_owner(
    SourceTypeMaterializationOwner owner,
    const void * semantic_owner)
{
  if(source_type_materialization_stack.empty() || !semantic_owner) {
    return false;
  }
  const SourceTypeMaterializationFrame & frame =
      source_type_materialization_stack.back();
  return frame.semantic_owner_committed &&
      frame.semantic_owner_kind == owner &&
      frame.semantic_owner == semantic_owner;
}

#if defined(CPPGM_ENABLE_WITNESS_PROVENANCE)
SourceTypeMaterializationOwner
current_source_type_materialization_semantic_owner_kind()
{
  return source_type_materialization_stack.empty() ?
      SourceTypeMaterializationOwner::None :
      source_type_materialization_stack.back().semantic_owner_kind;
}

const void * current_source_type_materialization_semantic_owner()
{
  return source_type_materialization_stack.empty() ?
      nullptr : source_type_materialization_stack.back().semantic_owner;
}
#endif

const char * source_type_materialization_owner_name(
    SourceTypeMaterializationOwner owner)
{
  switch(owner) {
    case SourceTypeMaterializationOwner::FunctionBody:
      return "function_body";
    case SourceTypeMaterializationOwner::DeclarationType:
      return "declaration_type";
    case SourceTypeMaterializationOwner::StaticMemberInitializer:
      return "static_member_initializer";
    case SourceTypeMaterializationOwner::VariableTemplateInitializer:
      return "variable_template_initializer";
    case SourceTypeMaterializationOwner::None:
      break;
  }
  return "none";
}

const char * source_type_materialization_operation_name(
    SourceTypeMaterializationOperation operation)
{
  switch(operation) {
    case SourceTypeMaterializationOperation::ContainingSemanticOwner:
      return "containing_semantic_owner";
    case SourceTypeMaterializationOperation::SourceTypeNode:
      return "source_type_node";
    case SourceTypeMaterializationOperation::StaticMemberInitializer:
      return "static_member_initializer";
    case SourceTypeMaterializationOperation::VariableTemplateInitializer:
      return "variable_template_initializer";
    case SourceTypeMaterializationOperation::None:
      break;
  }
  return "none";
}

std::string canonicalize_template_parameter_source_text(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::string & text,
    const cpp_decl::TemplateArgumentSyntax * syntax)
{
  return canonicalize_template_parameter_log_text(
      parameters,
      normalize_pattern_function_type_arg_text(text, syntax));
}

FunctionTemplateRegistrationIdentity function_binding_registration_identity(
    const semantic_model::FunctionBinding & binding)
{
  FunctionTemplateRegistrationIdentity identity;
  identity.decl = binding.source_template;
  identity.arguments =
      binding.instantiation_arguments.empty() ? nullptr : &binding.instantiation_arguments;
  identity.arguments_present = binding.has_instantiation_arguments ||
                               !binding.instantiation_arguments.empty();
  identity.key = function_binding_template_instantiation_key(binding);
  return identity;
}

bool function_binding_matches_instantiation_identity(
    const semantic_model::FunctionBinding & binding,
    const FunctionTemplateRegistrationIdentity & identity)
{
  return template_instantiation::function_binding_matches_instantiation_identity(
      binding, identity.decl, identity.key);
}

bool function_binding_matches_materialized_owner_template_identity(
    const semantic_model::FunctionBinding & binding,
    const FunctionTemplateRegistrationIdentity & identity)
{
  return template_instantiation::
      function_binding_matches_materialized_owner_template_identity(
          binding, identity.decl, identity.key);
}

bool should_preserve_owner_prefixed_template_identity(
    const semantic_model::FunctionBinding & original,
    const semantic_model::FunctionBinding & materialized,
    bool types_match)
{
  return template_instantiation::should_preserve_owner_prefixed_template_identity(
      original, materialized, types_match);
}

void record_function_template_identity(
    semantic_model::FunctionBinding & binding,
    const FunctionTemplateRegistrationIdentity & identity)
{
  template_instantiation::record_function_template_identity(
      binding, identity.decl, identity.key, identity.arguments);
}

void record_function_template_arguments_preserving_pack_sizes(
    semantic_model::FunctionBinding & binding,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool has_arguments)
{
  template_instantiation::record_function_template_arguments_preserving_pack_sizes(
      binding, arguments, has_arguments);
}

void adopt_materialized_owner_template_identity(
    semantic_model::FunctionBinding & binding,
    const FunctionTemplateRegistrationIdentity & identity,
    semantic_model::Scope * declaration_scope)
{
  template_instantiation::adopt_materialized_owner_template_identity(
      binding,
      identity.decl,
      identity.key,
      identity.arguments,
      declaration_scope);
}

void adopt_function_template_identity_from_materialized(
    semantic_model::FunctionBinding & original,
    const semantic_model::FunctionBinding & materialized)
{
  template_instantiation::adopt_function_template_identity_from_materialized(
      original, materialized);
}

// template-boundary-audit: begin canonical_key_metadata
const std::string & class_template_instantiation_key(
    const semantic_model::ClassInfo & info)
{
  return semantic_model::class_instantiation_key(info);
}

std::string class_template_effective_instantiation_key(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return template_instantiation::class_template_instance_key(ctx, info);
}

std::string template_argument_identity_key(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return template_instantiation::template_argument_key_for_instantiation(
      ctx, arguments);
}

std::vector<std::string> canonical_template_argument_texts(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return template_instantiation::canonical_instantiation_arg_texts(
      ctx, arguments);
}

bool template_arguments_are_dependent(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return with_template_type_system(
      ctx,
      [&](TemplateTypeSystem & type_system)
      {
        return template_model::template_arguments_are_dependent(
            arguments,
            [&](const cpp_decl::TypePtr & type)
            {
              return template_argument_semantics::type_depends_on_template_parameter(
                  type_system, type);
            });
      });
}

void canonicalize_simple_dependent_argument_texts(
    SemanticContext & ctx,
    std::vector<template_model::TemplateArgument> & arguments)
{
  with_template_type_system(
      ctx,
      [&](TemplateTypeSystem & type_system)
      {
        template_argument_semantics::canonicalize_simple_dependent_argument_texts(
            type_system, arguments);
      });
}

std::string specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return template_instantiation::specialization_name_for_instantiation(
      ctx, name, arguments);
}

std::string display_specialization_name_for_instantiation(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return template_instantiation::display_specialization_name_for_instantiation(
      ctx, name, arguments);
}

bool record_class_template_instantiation_state(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool is_explicit_specialization,
    bool suppress_implicit_instantiation_definition,
    bool dependent_arguments,
    const std::vector<std::string> * dependent_argument_texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * dependent_argument_syntaxes,
    const std::vector<template_model::TemplateParameterInfo> *
        dependent_argument_mangle_parameters,
    const std::vector<template_model::TemplateArgument> *
        dependent_argument_mangle_arguments,
    const std::map<std::string, std::size_t> *
        dependent_argument_mangle_pack_sizes)
{
  return template_instantiation::record_class_template_instantiation_state(
      ctx,
      info,
      key,
      arguments,
      is_explicit_specialization,
      suppress_implicit_instantiation_definition,
      dependent_arguments,
      dependent_argument_texts,
      dependent_argument_syntaxes,
      dependent_argument_mangle_parameters,
      dependent_argument_mangle_arguments,
      dependent_argument_mangle_pack_sizes);
}

bool refresh_referenced_class_template_selection(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info)
{
  return template_instantiation::refresh_referenced_class_template_selection(
      ctx, info);
}

bool class_template_completion_has_owner_definition(
    const semantic_model::ClassInfo & info)
{
  return template_instantiation::class_template_completion_has_owner_definition(info);
}

bool nested_member_class_owner_definition_available(
    const semantic_model::ClassInfo & info)
{
  if(!class_template_completion_has_owner_definition(info)) {
    return false;
  }
  const semantic_model::ClassInfo * owner =
      info.enclosing_scope->class_info;
  if(owner->is_explicit_specialization) {
    return false;
  }
  const semantic_model::ClassTemplateDecl * owner_template =
      owner->source_template;
  const semantic_model::PartialClassTemplateSpecializationDecl * partial =
      selected_partial_log_decl(owner);
  const std::map<std::string, semantic_model::OutOfClassMemberClassDecl> &
      definitions = partial ? partial->member_class_definitions :
                              owner_template->member_class_definitions;
  std::map<std::string, semantic_model::OutOfClassMemberClassDecl>::const_iterator
      found = definitions.find(info.name);
  return found != definitions.end() &&
         found->second.class_node &&
         found->second.class_node->kind != CppAstKind::class_forward_declaration;
}

ClassTemplateCompletionPlan class_template_completion_plan(
    const semantic_model::ClassInfo & info)
{
  return template_instantiation::class_template_completion_plan(info);
}

bool apply_out_of_class_static_member_definitions_to_reference(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info)
{
  return template_instantiation::
      apply_out_of_class_static_member_definitions_to_reference(ctx, info);
}

bool apply_out_of_class_member_function_abi_metadata(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info)
{
  return template_instantiation::
      apply_out_of_class_member_function_abi_metadata(ctx, info);
}

bool class_template_use_info_for_type(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const cpp_decl::TypePtr & type,
    ClassTemplateUseInfo & out,
    bool select_specialization)
{
  return template_instantiation::class_template_use_info_for_type(
      ctx, scope, type, out, select_specialization);
}

bool class_template_use_info_for_class(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    semantic_model::ClassInfo * info,
    ClassTemplateUseInfo & out,
    bool select_specialization)
{
  return template_instantiation::class_template_use_info_for_class(
      ctx, scope, info, out, select_specialization);
}

bool template_id_matches_class_template_origin(
    const cpp_decl::QualifiedName & template_id,
    const ClassTemplateUseInfo & info)
{
  return template_instantiation::template_id_matches_class_template_origin(
      template_id, info);
}

void append_class_template_type_arguments(const semantic_model::ClassInfo * info,
                                          std::vector<cpp_decl::TypePtr> & out)
{
  template_instantiation::append_class_template_type_arguments(info, out);
}

bool class_template_instantiation_depends_on_template_parameter(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return template_instantiation::
      class_template_instantiation_depends_on_template_parameter(ctx, info);
}
// template-boundary-audit: end canonical_key_metadata

bool build_class_finalization_request(
    semantic_model::ClassInfo & info,
    TemplateClassFinalizationRequest & out)
{
  if(!info.source_template) {
    return false;
  }
  out = TemplateClassFinalizationRequest();
  out.decl = info.source_template;
  out.info = &info;
  out.arguments = info.instantiation_arguments;
  return true;
}

bool build_function_definition_upgrade_request(
    const semantic_model::FunctionBinding & binding,
    semantic_model::Scope & use_scope,
    TemplateFunctionInstantiationRequest & out)
{
  if(!binding.source_template) {
    return false;
  }
  out = TemplateFunctionInstantiationRequest();
  out.decl = binding.source_template;
  out.arguments = binding.instantiation_arguments;
  out.use_scope = make_template_environment(use_scope);
  if(!binding.instantiation_pack_sizes.empty()) {
    out.pack_sizes = binding.instantiation_pack_sizes;
    out.has_pack_sizes = true;
  }
  out.include_body = true;
  out.active_owner = binding.owner_class;
  return true;
}

ClassSpecializationSelection to_api_class_specialization_selection(
    const template_selection::ClassSpecializationSelection & selection)
{
  return to_api_class_specialization_selection_impl(selection);
}

VariableSpecializationSelection to_api_variable_specialization_selection(
    const template_selection::VariableSpecializationSelection & selection)
{
  return to_api_variable_specialization_selection_impl(selection);
}

ScopedTemplateArgumentSourceLocations::ScopedTemplateArgumentSourceLocations(
    const std::vector<std::string> & texts,
    const std::vector<std::string> & locations)
  : active_(texts.size() == locations.size() && has_nonempty_location(locations))
{
  if(active_) {
    TemplateArgumentSourceLocationFrame frame;
    frame.texts = texts;
    frame.locations = locations;
    template_argument_source_location_stack.push_back(frame);
  }
}

ScopedTemplateArgumentSourceLocations::~ScopedTemplateArgumentSourceLocations()
{
  if(active_ && !template_argument_source_location_stack.empty()) {
    template_argument_source_location_stack.pop_back();
  }
}

bool current_template_argument_source_locations_active()
{
  return !template_argument_source_location_stack.empty();
}

std::string current_template_argument_source_location(const std::string & text,
                                                      std::size_t index)
{
  for(std::size_t i = template_argument_source_location_stack.size(); i > 0; --i) {
    const TemplateArgumentSourceLocationFrame & frame =
        template_argument_source_location_stack[i - 1];
    if(index >= frame.locations.size()) {
      continue;
    }
    if(index < frame.texts.size() &&
       !template_argument_text_matches(frame.texts[index], text)) {
      continue;
    }
    return frame.locations[index];
  }
  return std::string();
}

ScopedTemplateIdSourceArguments::ScopedTemplateIdSourceArguments(
    const std::string & location,
    const std::string & template_name,
    std::vector<std::string> arg_texts)
  : active_(!location.empty() && !template_name.empty())
{
  if(active_) {
    TemplateIdSourceArgumentFrame frame;
    frame.location = normalize_template_witness_source_location(location);
    frame.template_name = template_name;
    frame.arg_texts = std::move(arg_texts);
    template_id_source_argument_cache[
        make_template_id_source_argument_key(frame.location, frame.template_name)] =
        frame.arg_texts;
    template_id_source_argument_stack.push_back(std::move(frame));
  }
}

ScopedTemplateIdSourceArguments::~ScopedTemplateIdSourceArguments()
{
  if(active_ && !template_id_source_argument_stack.empty()) {
    template_id_source_argument_stack.pop_back();
  }
}

bool current_template_id_source_arguments(
    const std::string & location,
    const std::string & template_name,
    std::vector<std::string> & arg_texts)
{
  arg_texts.clear();
  const std::vector<std::string> * current =
      current_template_id_source_arguments_ptr(location, template_name);
  if(!current) {
    return false;
  }
  arg_texts = *current;
  return true;
}

const std::vector<std::string> * current_template_id_source_arguments_ptr(
    const std::string & location,
    const std::string & template_name)
{
  if(location.empty() || template_name.empty()) {
    return nullptr;
  }
  const TemplateIdSourceArgumentKey key =
      make_template_id_source_argument_key(location, template_name);
  for(std::size_t i = template_id_source_argument_stack.size(); i > 0; --i) {
    const TemplateIdSourceArgumentFrame & frame =
        template_id_source_argument_stack[i - 1];
    if(make_template_id_source_argument_key(frame.location, frame.template_name) == key) {
      return &frame.arg_texts;
    }
  }
  std::map<TemplateIdSourceArgumentKey,
           std::vector<std::string> >::const_iterator cached =
      template_id_source_argument_cache.find(key);
  if(cached != template_id_source_argument_cache.end()) {
    return &cached->second;
  }
  return nullptr;
}

bool class_has_template_identity(const semantic_model::ClassInfo * info)
{
  return class_has_template_identity_impl(info);
}

bool class_has_source_template_identity(const semantic_model::ClassInfo * info)
{
  return info && info->source_template != nullptr;
}

bool class_source_template_identity_matches(
    const semantic_model::ClassInfo * info,
    const semantic_model::ClassTemplateDecl * decl)
{
  return info && decl && info->source_template == decl;
}

bool class_has_non_dependent_source_template_identity(
    const semantic_model::ClassInfo * info)
{
  return class_has_source_template_identity(info) && !info->dependent_instantiation;
}

bool class_is_explicit_specialization(const semantic_model::ClassInfo * info)
{
  return info && info->is_explicit_specialization;
}

bool scope_has_template_owner_identity(const semantic_model::Scope * scope)
{
  return scope_has_template_owner_identity_impl(scope);
}

bool scope_has_linkage_template_owner_identity(const semantic_model::Scope * scope)
{
  return scope_has_linkage_template_owner_identity_impl(scope);
}

bool class_owner_scope_has_template_identity(const semantic_model::ClassInfo * info)
{
  return info && info->member_scope &&
         scope_has_template_owner_identity_impl(info->member_scope.get());
}

bool function_binding_has_template_identity(const semantic_model::FunctionBinding * binding)
{
  return binding &&
         (binding->source_template ||
          class_has_template_identity_impl(binding->owner_class) ||
          class_has_template_identity_impl(binding->lexical_access_class) ||
          scope_has_template_owner_identity_impl(binding->declaration_scope));
}

bool function_or_owner_has_template_identity(const semantic_model::FunctionBinding * binding)
{
  return binding &&
         (function_binding_has_template_identity(binding) ||
          class_owner_scope_has_template_identity(binding->owner_class));
}

bool function_binding_has_linkage_template_identity(
    const semantic_model::FunctionBinding * binding)
{
  return binding &&
         (((binding->source_template || !binding->template_instantiation_key.empty()) &&
           !binding->is_explicit_specialization) ||
          class_has_linkage_template_identity_impl(binding->owner_class) ||
          class_has_linkage_template_identity_impl(binding->lexical_access_class) ||
          scope_has_linkage_template_owner_identity_impl(binding->declaration_scope));
}

bool function_binding_identity_has_internal_namespace_linkage(
    const semantic_model::FunctionBinding * binding)
{
  return function_binding_identity_has_internal_namespace_linkage_impl(binding);
}

bool value_or_owner_has_template_identity(const semantic_model::ValueBinding * binding)
{
  return binding &&
         (value_binding_has_template_identity_impl(binding) ||
          class_owner_scope_has_template_identity(binding->owner_class));
}

bool function_binding_has_source_template_identity(
    const semantic_model::FunctionBinding * binding)
{
  return binding && binding->source_template != nullptr;
}

bool function_binding_has_parameter_name_syntax_source(
    const semantic_model::FunctionBinding & binding)
{
  return binding.declaration_node ||
         binding.definition_node ||
         binding.source_template;
}

const CppAstNode * function_binding_source_template_declarator(
    const semantic_model::FunctionBinding & binding)
{
  return binding.source_template ? binding.source_template->declarator : nullptr;
}

const void * function_binding_source_template_debug_identity(
    const semantic_model::FunctionBinding * binding)
{
  return binding ? binding->source_template : nullptr;
}

bool function_binding_has_template_or_body_definition_source(
    const semantic_model::FunctionBinding & binding)
{
  return binding.has_definition || binding.source_template || binding.body;
}

bool function_binding_is_declaration_only_template(
    const semantic_model::FunctionBinding & binding)
{
  return binding.source_template &&
         (!binding.source_template->body ||
          (binding.is_explicit_specialization && !binding.has_definition));
}

bool function_binding_has_empty_template_identity(
    const semantic_model::FunctionBinding & binding)
{
  return !binding.source_template &&
         function_binding_has_empty_template_identity_key(binding);
}

bool class_suppresses_implicit_instantiation_definition(
    const semantic_model::ClassInfo * info)
{
  return info && info->suppress_implicit_instantiation_definition;
}

bool function_template_decl_is_member_function_template(
    const semantic_model::FunctionTemplateDecl & decl)
{
  return decl.is_member_function_template &&
         decl.declaring_scope &&
         decl.declaring_scope->class_info;
}

bool function_binding_is_member_function_template(
    const semantic_model::FunctionBinding & binding)
{
  return binding.source_template &&
         function_template_decl_is_member_function_template(*binding.source_template);
}

bool function_binding_owner_class_suppresses_implicit_instantiation_definition(
    const semantic_model::FunctionBinding & binding)
{
  const semantic_model::ClassInfo * owner = binding.owner_class;
  if(!owner) {
    return false;
  }
  if(function_binding_is_member_function_template(binding)) {
    return false;
  }
  const semantic_model::ClassInfo * output_owner =
      effective_instantiated_class_output_owner(*owner);
  if(!output_owner) {
    output_owner = owner;
  }
  if(output_owner->suppress_implicit_instantiation_definition) {
    return true;
  }
  const semantic_model::ClassTemplateDecl * source_template =
      output_owner->source_template;
  const std::string & instantiation_key =
      output_owner == owner && !binding.template_instantiation_key.empty() ?
          binding.template_instantiation_key :
          semantic_model::class_instantiation_key(*output_owner);
  return source_template &&
         !instantiation_key.empty() &&
         source_template->suppress_implicit_instantiation_definitions.find(
             instantiation_key) !=
             source_template->suppress_implicit_instantiation_definitions.end();
}

bool function_binding_excluded_from_explicit_instantiation(
    const semantic_model::FunctionBinding & binding)
{
  return binding.exclude_from_explicit_instantiation ||
         (binding.source_template &&
          binding.source_template->exclude_from_explicit_instantiation) ||
         node_marks_exclude_from_explicit_instantiation(binding.definition_node) ||
         node_marks_exclude_from_explicit_instantiation(binding.declaration_node);
}

bool function_binding_bypasses_explicit_instantiation_suppression(
    const semantic_model::FunctionBinding & binding,
    bool explicit_instantiation_suppressed)
{
  if(binding.is_explicit_instantiation_definition) {
    return true;
  }
  // A lambda's functions have no separately provided explicit-instantiation
  // definition. If emitted code reaches the closure, its local definitions
  // must accompany that code even when the enclosing class specialization is
  // covered by an extern-template declaration.
  if(binding.owner_class && binding.owner_class->is_lambda_closure) {
    return true;
  }
  const bool weak_local_definition_candidate =
      symbol_linkage::has_weak_linkage(binding.symbol) &&
      (binding.has_definition ||
       binding.body ||
       binding.source_template ||
       binding.synthesized);
  if(weak_local_definition_candidate &&
     !explicit_instantiation_suppressed) {
    return true;
  }
  if(explicit_instantiation_suppressed &&
     binding_is_noop_default_constructor_for_suppressed_emission(binding)) {
    return true;
  }
  if(explicit_instantiation_suppressed &&
     binding_is_noop_copy_constructor_for_suppressed_emission(binding)) {
    return true;
  }
  if(explicit_instantiation_suppressed &&
     binding_is_noop_inline_function_for_suppressed_emission(binding)) {
    return true;
  }

  const bool implicit_special_member =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  if(!implicit_special_member) {
    return false;
  }
  return binding.is_constructor ||
         binding.is_destructor ||
         binding.is_copy_assignment ||
         binding.is_move_assignment;
}

bool function_binding_bypasses_explicit_instantiation_suppression(
    const semantic_model::FunctionBinding & binding)
{
  const bool explicit_instantiation_suppressed =
      binding.suppress_implicit_instantiation_definition ||
      function_binding_owner_class_suppresses_implicit_instantiation_definition(
          binding);
  return function_binding_bypasses_explicit_instantiation_suppression(
      binding,
      explicit_instantiation_suppressed);
}

bool function_binding_has_in_class_member_definition(
    const semantic_model::FunctionBinding & binding)
{
  return binding.owner_class &&
         binding.declaration_node &&
         binding.definition_node &&
         binding.declaration_node == binding.definition_node;
}

bool function_binding_output_suppressed_by_explicit_instantiation(
    const semantic_model::FunctionBinding & binding)
{
  const bool class_instantiation_suppressed =
      function_binding_owner_class_suppresses_implicit_instantiation_definition(
          binding);
  const bool explicit_instantiation_suppressed =
      binding.suppress_implicit_instantiation_definition ||
      class_instantiation_suppressed;
  if(class_instantiation_suppressed &&
     !binding.suppress_implicit_instantiation_definition &&
     function_binding_has_in_class_member_definition(binding)) {
    return false;
  }
  return explicit_instantiation_suppressed &&
         !function_binding_excluded_from_explicit_instantiation(binding) &&
         !function_binding_bypasses_explicit_instantiation_suppression(
             binding, explicit_instantiation_suppressed);
}

bool value_binding_owner_class_suppresses_implicit_instantiation_definition(
    const semantic_model::ValueBinding & binding)
{
  const semantic_model::ClassInfo * owner = binding.owner_class;
  if(!owner) {
    return false;
  }
  if(binding.variable_template_instantiation &&
     binding.variable_template_instantiation->source_template) {
    return false;
  }
  if(binding.is_explicit_specialization ||
     owner->is_explicit_specialization) {
    return false;
  }
  if(owner->suppress_implicit_instantiation_definition) {
    return true;
  }
  const semantic_model::ClassTemplateDecl * source_template =
      owner->source_template;
  const std::string & instantiation_key =
      semantic_model::class_instantiation_key(*owner);
  return source_template &&
         !instantiation_key.empty() &&
         source_template->suppress_implicit_instantiation_definitions.find(
             instantiation_key) !=
             source_template->suppress_implicit_instantiation_definitions.end();
}

bool value_binding_output_suppressed_by_explicit_instantiation(
    const semantic_model::ValueBinding & binding)
{
  return value_binding_owner_class_suppresses_implicit_instantiation_definition(
      binding);
}

void apply_function_template_symbol_options(
    semantic_model::FunctionTemplateDecl * source_template,
    const std::vector<template_model::TemplateArgument> * instantiation_arguments,
    bool has_instantiation_arguments,
    const semantic_model::ClassInfo * owner_class,
    bool is_constructor,
    bool is_destructor,
    symbol_linkage::FunctionSymbolOptions & options)
{
  if(owner_class && owner_class->is_explicit_specialization) {
    options.suppress_template_argument_pack_grouping = true;
  }
  if(owner_class &&
     owner_class->is_lambda_closure &&
     owner_class->type) {
    options.lambda_closure_type = owner_class->type;
  }
  const bool function_local_owner =
      owner_class &&
      (owner_class->source_is_named_function_local_class ||
       owner_class->source_is_unnamed_class ||
       owner_class->name.rfind("__local_type", 0) == 0);
  if(owner_class &&
     !owner_class->is_lambda_closure &&
     owner_class->type &&
     function_local_owner &&
     owner_class->type->named_lambda_mangle()) {
    options.local_class_type = owner_class->type;
  }
  const std::vector<template_model::TemplateParameterInfo> *
      nearest_owner_parameters = nullptr;
  const std::vector<template_model::TemplateArgument> *
      nearest_owner_arguments = nullptr;
  const std::vector<template_model::TemplateParameterInfo> *
      nearest_owner_mangle_parameters = nullptr;
  std::string nearest_owner_template_name;
  std::vector<symbol_linkage::FunctionSymbolOptions::OwnerTemplateComponent>
      owner_components;
  for(const semantic_model::ClassInfo * current = owner_class;
      current;
      current = current->enclosing_scope ? current->enclosing_scope->class_info : nullptr) {
    const semantic_model::ClassTemplateDecl * component_template = nullptr;
    const std::vector<template_model::TemplateArgument> * component_arguments = nullptr;
    const std::vector<template_model::TemplateParameterInfo> *
        component_mangle_parameters = nullptr;
    const std::vector<cpp_decl::TemplateArgumentSyntax> *
        component_argument_syntaxes = nullptr;
    std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
        specialization =
            cpp_decl::named_type_class_template_specialization_mangle_info_const(
                current->type);
    if(current->source_template &&
       !current->instantiation_arguments.empty()) {
      component_template = current->source_template;
      component_arguments = &current->instantiation_arguments;
    } else if(specialization &&
              specialization->class_template_decl) {
      component_template =
          static_cast<const semantic_model::ClassTemplateDecl *>(
              specialization->class_template_decl);
      component_arguments = &specialization->arguments.const_values();
    }
    if(specialization && component_arguments) {
      if(!specialization->mangle_parameters.empty()) {
        component_mangle_parameters = &specialization->mangle_parameters;
      }
      if(specialization->argument_syntaxes.size() == component_arguments->size()) {
        component_argument_syntaxes = &specialization->argument_syntaxes;
      }
    }
    if(component_template && component_arguments) {
      const semantic_model::PartialClassTemplateSpecializationDecl *
          selected_partial = selected_partial_log_decl(current);
      const bool selected_partial_has_argument_syntaxes =
          selected_partial &&
          selected_partial->arg_syntaxes.size() == component_arguments->size();
      const std::vector<template_model::TemplateParameterInfo> *
          effective_mangle_parameters =
              selected_partial_has_argument_syntaxes ?
                  &selected_partial->parameters :
                  component_mangle_parameters;
      const std::vector<cpp_decl::TemplateArgumentSyntax> *
          effective_argument_syntaxes =
              selected_partial_has_argument_syntaxes ?
                  &selected_partial->arg_syntaxes :
                  component_argument_syntaxes;
      if(!nearest_owner_parameters) {
        nearest_owner_parameters = &component_template->parameters;
        nearest_owner_arguments = component_arguments;
        nearest_owner_template_name = component_template->name;
        nearest_owner_mangle_parameters = effective_mangle_parameters;
      }
      symbol_linkage::FunctionSymbolOptions::OwnerTemplateComponent component;
      component.template_name = component_template->name;
      component.parameters = &component_template->parameters;
      component.arguments = component_arguments;
      component.mangle_parameters = effective_mangle_parameters;
      component.argument_syntaxes = effective_argument_syntaxes;
      owner_components.push_back(component);
    }
  }
  std::reverse(owner_components.begin(), owner_components.end());
  options.owner_template_components.insert(options.owner_template_components.end(),
                                           owner_components.begin(),
                                           owner_components.end());
  if(nearest_owner_parameters && nearest_owner_arguments) {
    options.owner_template_parameters = nearest_owner_parameters;
    options.owner_template_arguments = nearest_owner_arguments;
    options.owner_mangle_parameters = nearest_owner_mangle_parameters;
    options.owner_template_name = nearest_owner_template_name;
  }
  if(source_template &&
     instantiation_arguments &&
     has_instantiation_arguments) {
    options.is_conversion_operator =
        options.is_conversion_operator || source_template->is_conversion_operator;
    if(source_template->result_type_pattern.kind == CppAstKind::invalid &&
       source_template->specifiers &&
       source_template->declarator &&
       !is_constructor &&
       !is_destructor) {
      source_template->result_type_pattern =
          template_api::signature::build_function_result_type_pattern(
              *source_template->specifiers, *source_template->declarator);
    }
    options.function_type_pattern = source_template->type_pattern;
    options.template_parameters = &source_template->parameters;
    options.template_arguments = instantiation_arguments;
    options.parameter_pattern = &source_template->params_pattern;
    if(source_template->result_type_pattern.kind != CppAstKind::invalid) {
      options.result_type_pattern = &source_template->result_type_pattern;
    }
    options.parameter_declarations_pattern =
        &source_template->parameter_declarations_pattern;
    options.has_trailing_function_parameter_pack =
        source_template->has_trailing_function_parameter_pack;
  }
}

void apply_function_binding_template_symbol_options(
    const semantic_model::FunctionBinding & binding,
    symbol_linkage::FunctionSymbolOptions & options)
{
  const bool has_arguments = !binding.instantiation_arguments.empty();
  const semantic_model::ClassInfo * owner_class = binding.owner_class;
  if(!owner_class &&
     binding.declaration_scope &&
     binding.declaration_scope->class_info) {
    owner_class = binding.declaration_scope->class_info;
  }
  apply_function_template_symbol_options(
      binding.source_template,
      has_arguments ? &binding.instantiation_arguments : nullptr,
      has_arguments,
      owner_class,
      binding.is_constructor,
      binding.is_destructor,
      options);
  if(!binding.instantiation_pack_sizes.empty()) {
    options.template_argument_pack_sizes = &binding.instantiation_pack_sizes;
  }
}

const semantic_model::ClassInfo * effective_instantiated_class_output_owner(
    const semantic_model::ClassInfo & info)
{
  if(info.source_template) {
    return &info;
  }

  const semantic_model::Scope * scope = info.enclosing_scope;
  while(scope) {
    if(scope->class_info && scope->class_info->source_template) {
      return scope->class_info;
    }
    scope = scope->parent;
  }

  return nullptr;
}

TemplateClassOutputReadiness compute_instantiated_class_output_readiness(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  semantic_hotspot::note_semantic_query("instantiated_class_output_readiness",
                                        info.qualified_name);
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->instantiated_class_output_readiness_calls;
  }

  TemplateClassOutputReadiness out;
  const semantic_model::ClassInfo * output_owner =
      effective_instantiated_class_output_owner(info);
  if(!output_owner) {
    return out;
  }
  out.templated_context = true;
  out.suppress_implicit_definition =
      output_owner->suppress_implicit_instantiation_definition;
  if(output_owner->is_explicit_specialization) {
    return out;
  }

  const bool member_scope_has_placeholders =
      output_owner->member_scope &&
      ctx.scope_has_template_placeholders(*output_owner->member_scope);
  out.output_blocked_by_placeholders =
      output_owner->member_scope &&
      !output_owner->dependent_instantiation &&
      member_scope_has_placeholders;
  out.non_dependent =
      !output_owner->dependent_instantiation &&
      !member_scope_has_placeholders &&
      !template_model::template_arguments_are_dependent(
          output_owner->instantiation_arguments,
          [&ctx](const cpp_decl::TypePtr & type)
          {
            return ctx.type_depends_on_template_parameter(type);
          });
  if(!out.non_dependent) {
    out.complete = false;
    return out;
  }

  for(std::size_t i = 0; i < output_owner->instantiation_arguments.size(); ++i) {
    const template_model::TemplateArgument & argument =
        output_owner->instantiation_arguments[i];
    if(argument.kind != template_model::TemplateArgument::TA_TYPE || !argument.type) {
      continue;
    }

    cpp_decl::TypePtr base =
        cpp_decl::strip_top_level_cv(cpp_decl::remove_reference_type(argument.type));
    if(!base || cpp_decl::type_is_complete(base)) {
      continue;
    }

    out.complete = false;
    return out;
  }

  return out;
}

bool instantiated_class_arguments_non_dependent(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return compute_instantiated_class_output_readiness(ctx, info).non_dependent;
}

bool function_binding_signature_mentions_source_template_parameter(
    const semantic_model::FunctionBinding & binding,
    const std::string & signature_text)
{
  if(!binding.source_template) {
    return false;
  }
  if(signature_text.find("template-parameter ") != std::string::npos ||
     signature_text.find("dependent alias ") != std::string::npos ||
     signature_text.find("dependent type ") != std::string::npos ||
     signature_text.find("dependent decltype ") != std::string::npos) {
    return true;
  }
  for(std::size_t i = 0; i < binding.source_template->parameters.size(); ++i) {
    const template_model::TemplateParameterInfo & parameter =
        binding.source_template->parameters[i];
    if(!parameter.name.empty() &&
       signature_text.find(parameter.name) != std::string::npos) {
      return true;
    }
    if(!parameter.placeholder_key.empty() &&
       signature_text.find(parameter.placeholder_key) != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool function_binding_instantiation_arguments_dependent(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding & binding)
{
  return template_model::template_arguments_are_dependent(
      binding.instantiation_arguments,
      [&ctx](const cpp_decl::TypePtr & type)
      {
        return ctx.type_depends_on_template_parameter(type);
      });
}

bool function_binding_instantiation_arguments_complete(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding & binding)
{
  if(!binding.source_template) {
    return true;
  }
  if(!function_binding_instantiation_arguments_dependent(ctx, binding)) {
    return true;
  }

  if(ctx.type_depends_on_template_parameter(binding.type)) {
    return false;
  }

  if(binding.declaration_scope &&
     ctx.scope_has_template_placeholders(*binding.declaration_scope)) {
    const std::string signature_text = cpp_decl::describe_type(binding.type);
    if(function_binding_signature_mentions_source_template_parameter(binding,
                                                                     signature_text)) {
      return false;
    }
  }
  return true;
}

void bump_scope_template_binding_fingerprint_epoch(semantic_model::Scope & scope)
{
  template_scope::bump_binding_fingerprint_epoch(scope);
}

std::size_t scope_template_binding_fingerprint(
    const semantic_model::Scope & scope)
{
  return template_scope::scope_binding_fingerprint(scope);
}

std::size_t scope_template_instance_fingerprint(
    const semantic_model::Scope & scope)
{
  return template_scope::scope_instance_fingerprint(scope);
}

semantic_model::FunctionBinding * find_defined_class_function_matching_template_identity(
    SemanticContext & ctx,
    semantic_model::ClassInfo & owner,
    const std::string & lookup_name,
    const semantic_model::FunctionBinding & binding)
{
  const FunctionTemplateRegistrationIdentity identity =
      function_binding_registration_identity(binding);
  return ctx.find_defined_class_function(owner,
                                         lookup_name,
                                         binding.type,
                                         identity,
                                         binding.ref_qualifier);
}

semantic_model::FunctionBinding * find_defined_function_matching_template_identity(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & lookup_name,
    const semantic_model::FunctionBinding & binding)
{
  const FunctionTemplateRegistrationIdentity identity =
      function_binding_registration_identity(binding);
  return ctx.find_defined_function(scope,
                                   lookup_name,
                                   binding.type,
                                   identity);
}

std::string function_binding_witness_entity(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding)
{
  return binding_log_entity(ctx, binding);
}

std::string function_binding_witness_decl_location(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding)
{
  return binding_decl_location(ctx, binding);
}

std::string function_binding_template_trace_key(
    const semantic_model::FunctionBinding * binding)
{
  return binding ? function_binding_template_instantiation_key(*binding) :
                   std::string();
}

std::string alias_template_witness_entity(
    const semantic_model::AliasTemplateDecl * decl)
{
  return alias_template_log_entity(decl);
}

std::string alias_template_witness_source_entity(
    SemanticContext & ctx,
    const semantic_model::AliasTemplateDecl * decl,
    const semantic_model::ClassTemplateDecl * lexical_source_class_template,
    const std::vector<cpp_decl::TemplateArgumentSyntax> *
        lexical_source_class_arguments,
    const semantic_model::ClassInfo * selected_concrete_owner)
{
  if(!decl) {
    return std::string();
  }
  if(selected_concrete_owner && !lexical_source_class_template) {
    return class_witness_output_qualified_name(ctx, *selected_concrete_owner) +
        "::" + decl->name;
  }
  if(!lexical_source_class_template) {
    return alias_template_log_entity(decl);
  }

  std::ostringstream owner;
  owner << class_template_witness_qualified_name(
      ctx, *lexical_source_class_template);
  if(lexical_source_class_arguments) {
    const std::vector<template_model::TemplateParameterInfo> * parameters =
        nullptr;
    for(std::size_t i = 0;
        i < lexical_source_class_template->partial_specializations.size();
        ++i) {
      const semantic_model::PartialClassTemplateSpecializationDecl & partial =
          lexical_source_class_template->partial_specializations[i];
      if(&partial.arg_syntaxes == lexical_source_class_arguments) {
        parameters = &partial.parameters;
        break;
      }
    }
    owner << "<";
    for(std::size_t i = 0;
        i < lexical_source_class_arguments->size();
        ++i) {
      if(i != 0) {
        owner << ", ";
      }
      std::string argument =
          callsemantic_internal::render_template_argument_syntax(
              (*lexical_source_class_arguments)[i]);
      if(parameters) {
        argument = canonicalize_template_parameter_log_text(
            *parameters, argument);
      }
      owner << argument;
    }
    owner << ">";
  }
  owner << "::" << decl->name;
  return owner.str();
}

TemplateFunctionDefinitionClosureState function_definition_closure_state(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding)
{
  TemplateFunctionDefinitionClosureState state;
  if(!binding) {
    return state;
  }
  state.template_owner =
      enclosing_template_instantiation_owner(binding->owner_class);
  state.template_owned_binding =
      function_binding_is_template_owned_for_definition_closure(binding);
  if(ctx.template_witness_context().session != nullptr) {
    const TemplateWitnessEntryContext current =
        current_template_witness_entry_context();
    if(current.origin == TemplateWitnessOrigin::Closure &&
       !current.trigger_entity.empty()) {
      state.closure_trigger_differs =
          current.trigger_entity != binding_log_entity(ctx, binding);
    }
  }
  return state;
}

bool function_definition_materialized_by_enclosing_closure(
    const semantic_model::FunctionBinding * binding)
{
  return binding && binding->template_definition_materialized_by_enclosing_closure;
}

void mark_function_definition_materialized_by_enclosing_closure(
    semantic_model::FunctionBinding * binding)
{
  if(binding) {
    binding->template_definition_materialized_by_enclosing_closure = true;
  }
}

void note_closure_owner_class_instantiation_if_needed(
    SemanticContext & ctx,
    semantic_model::ClassInfo * owner,
    const TemplateFunctionDefinitionClosureState & state)
{
  if(!(owner && owner != state.template_owner)) {
    return;
  }
  if(!owner->template_instantiation_tracked) {
    owner->template_instantiation_tracked = true;
  }
  TemplateLifecycleTransition transition;
  transition.transition_kind = TemplateLifecycleTransitionKind::Instantiated;
  transition.class_info = owner;
  transition.cause = lifecycle_cause_for_current_context_or_default(
      TemplateLifecycleCause::ImplicitUse);
  transition.use_declaration_as_event_location = true;
  observe_template_lifecycle_transition(ctx, transition);
}

TemplateWitnessEntryContext make_function_binding_closure_entry_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::FunctionBinding * binding)
{
  return make_template_closure_entry_context(
      reason,
      binding_log_entity(ctx, binding),
      binding_decl_location(ctx, binding),
      function_binding_has_template_identity(binding),
      TemplateWitnessTriggerKind::Function);
}

TemplateWitnessEntryContext make_class_closure_entry_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::ClassInfo * info)
{
  return make_template_closure_entry_context(
      reason,
      class_log_entity(ctx, info),
      class_decl_location(ctx, info),
      class_has_template_identity(info),
      TemplateWitnessTriggerKind::Class);
}

TemplateWitnessEntryContext make_value_binding_closure_entry_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::ValueBinding * binding)
{
  return make_template_closure_entry_context(
      reason,
      value_log_entity(ctx, binding),
      value_decl_location(ctx, binding),
      value_or_owner_has_template_identity(binding),
      TemplateWitnessTriggerKind::Variable);
}

ScopedTemplateWitnessEntryContext maybe_enter_function_binding_closure_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::FunctionBinding * binding)
{
  if(ctx.template_witness_context().session == nullptr ||
     current_template_witness_entry_context().origin == TemplateWitnessOrigin::Closure ||
     !function_binding_has_template_identity(binding)) {
    return ScopedTemplateWitnessEntryContext();
  }
  return ScopedTemplateWitnessEntryContext(
      make_function_binding_closure_entry_context(ctx, reason, binding));
}

ScopedTemplateWitnessEntryContext maybe_enter_function_body_materialization_context(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding)
{
  return maybe_enter_function_binding_closure_context(
      ctx,
      TemplateClosureReason::EnsureDefinition,
      binding);
}

ScopedTemplateWitnessEntryContext maybe_enter_class_closure_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::ClassInfo * info)
{
  if(ctx.template_witness_context().session == nullptr ||
     current_template_witness_entry_context().origin == TemplateWitnessOrigin::Closure ||
     !class_has_template_identity(info)) {
    return ScopedTemplateWitnessEntryContext();
  }
  return ScopedTemplateWitnessEntryContext(
      make_class_closure_entry_context(ctx, reason, info));
}

ScopedTemplateWitnessEntryContext maybe_enter_class_tracking_context(
    SemanticContext & ctx,
    const semantic_model::ClassInfo * info)
{
  return maybe_enter_class_closure_context(
      ctx,
      TemplateClosureReason::TrackInstantiation,
      info);
}

ScopedTemplateWitnessEntryContext maybe_enter_class_finalization_context(
    SemanticContext & ctx,
    const semantic_model::ClassInfo * info)
{
  return maybe_enter_class_closure_context(
      ctx,
      TemplateClosureReason::FinalizeClass,
      info);
}

ScopedTemplateWitnessEntryContext maybe_enter_value_binding_closure_context(
    SemanticContext & ctx,
    TemplateClosureReason reason,
    const semantic_model::ValueBinding * binding)
{
  if(ctx.template_witness_context().session == nullptr ||
     current_template_witness_entry_context().origin == TemplateWitnessOrigin::Closure ||
     !value_binding_has_template_identity_impl(binding)) {
    return ScopedTemplateWitnessEntryContext();
  }
  return ScopedTemplateWitnessEntryContext(
      make_value_binding_closure_entry_context(ctx, reason, binding));
}

void note_output_tracked_class_instantiation_if_needed(
    SemanticContext & ctx,
    semantic_model::ClassInfo * info,
    bool already_tracked)
{
  if(!info || already_tracked) {
    return;
  }
  TemplateLifecycleTransition transition;
  transition.transition_kind = TemplateLifecycleTransitionKind::Instantiated;
  transition.class_info = info;
  transition.cause = lifecycle_cause_for_current_context_or_default(
      TemplateLifecycleCause::ImplicitUse);
  observe_template_lifecycle_transition(ctx, transition);
}

std::string default_elided_type_argument_text(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type);

bool template_argument_contains_default_elided_type(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument,
    unsigned depth = 0);

static bool is_qualified_identifier_value_text(const std::string & text)
{
  const std::string trimmed = semantic_utils::trim_space(text);
  if(trimmed.empty()) {
    return false;
  }
  std::size_t pos = 0;
  bool expect_identifier = true;
  while(pos < trimmed.size()) {
    if(expect_identifier) {
      if(!(std::isalpha(static_cast<unsigned char>(trimmed[pos])) ||
           trimmed[pos] == '_')) {
        return false;
      }
      ++pos;
      while(pos < trimmed.size() &&
            (std::isalnum(static_cast<unsigned char>(trimmed[pos])) ||
             trimmed[pos] == '_')) {
        ++pos;
      }
      expect_identifier = false;
      continue;
    }
    if(pos + 1 >= trimmed.size() ||
       trimmed[pos] != ':' ||
       trimmed[pos + 1] != ':') {
      return false;
    }
    pos += 2;
    expect_identifier = true;
  }
  return !expect_identifier;
}

static bool value_argument_has_named_enum_type(
    const template_model::TemplateArgument & arg)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(arg.type);
  return base &&
         base->kind == cpp_decl::Type::TK_NAMED &&
         (base->named_key.compare(0, 5, "enum ") == 0 ||
          named_type_display_text(base).compare(0, 5, "enum ") == 0);
}

std::string enum_witness_enumerator_text_for_value(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg)
{
  if(arg.kind != template_model::TemplateArgument::TA_VALUE ||
     arg.dependent ||
     !value_argument_has_named_enum_type(arg)) {
    return std::string();
  }
  semantic_model::Scope * enum_scope = ctx.scope_for_type(arg.type);
  if(!enum_scope) {
    return std::string();
  }
  std::string match;
  for(std::map<std::string, semantic_model::ValueBinding>::const_iterator it =
          enum_scope->values.begin();
      it != enum_scope->values.end();
      ++it) {
    const semantic_model::ValueBinding & binding = it->second;
    if(binding.kind != semantic_model::ValueBinding::VK_VARIABLE ||
       !binding.declaration_node ||
       binding.declaration_node->kind != CppAstKind::enumerator ||
       !binding.has_constant_value ||
       binding.constant_value != arg.value ||
       !cpp_decl::type_equals(cpp_decl::strip_top_level_cv(binding.type),
                              cpp_decl::strip_top_level_cv(arg.type))) {
      continue;
    }
    if(!match.empty()) {
      return std::string();
    }
    match = semantic_lookup::scope_qualified_name(*enum_scope, binding.name);
  }
  return match;
}

bool type_is_non_dependent_reference_argument(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type)
{
  return type &&
         cpp_decl::is_reference_type(type) &&
         !ctx.type_depends_on_template_parameter(type);
}

std::string strip_witness_elaborated_type_prefix(const std::string & text)
{
  static const char * prefixes[] = {
      "class ",
      "struct ",
      "union ",
      "enum class ",
      "enum struct ",
      "enum "};
  std::string trimmed = semantic_utils::trim_space(text);
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const std::string prefix = prefixes[i];
    if(trimmed.compare(0, prefix.size(), prefix) == 0) {
      return trimmed.substr(prefix.size());
    }
  }
  return trimmed;
}

bool witness_text_is_owner_qualified_suffix(const std::string & qualified,
                                            const std::string & suffix)
{
  if(qualified.empty() ||
     suffix.empty() ||
     qualified == suffix ||
     qualified.size() <= suffix.size() + 2) {
    return false;
  }
  const std::size_t suffix_pos = qualified.size() - suffix.size();
  return qualified.compare(suffix_pos, suffix.size(), suffix) == 0 &&
         qualified[suffix_pos - 1] == ':' &&
         qualified[suffix_pos - 2] == ':';
}

std::string canonical_named_type_key_for_witness(const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base || base->kind != cpp_decl::Type::TK_NAMED) {
    return std::string();
  }
  if(base->named_key.compare(0, 8, "builtin ") == 0 ||
     base->named_key.find("template-parameter ") != std::string::npos ||
     base->named_key.find("dependent ") != std::string::npos ||
     base->named_key.find("partial-order ") != std::string::npos) {
    return std::string();
  }
  const std::string key =
      strip_witness_elaborated_type_prefix(base->named_key);
  const std::string display =
      strip_witness_elaborated_type_prefix(named_type_display_text(base));
  return witness_text_is_owner_qualified_suffix(key, display) ? key :
                                                               std::string();
}

std::string normalize_witness_angle_spacing(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '>') {
      while(!out.empty() &&
            std::isspace(static_cast<unsigned char>(out[out.size() - 1]))) {
        out.resize(out.size() - 1);
      }
    }
    out.push_back(text[i]);
  }
  return semantic_utils::trim_space(out);
}

bool template_argument_is_witness_default_equivalent(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    std::size_t index,
    bool allow_explicit_default_equivalent)
{
  if(index >= parameters.size() || index >= arguments.size()) {
    return false;
  }
  const template_model::TemplateParameterInfo & parameter = parameters[index];
  const template_model::TemplateArgument & argument = arguments[index];
  if(argument.source_defaulted) {
    return true;
  }
  if(!allow_explicit_default_equivalent) {
    return false;
  }
  if(parameter.parameter_pack || !parameter.default_argument) {
    return false;
  }
  const CppAstNode * default_payload =
      !parameter.default_argument->children.empty() ?
          &parameter.default_argument->children[0] :
          parameter.default_argument;
  const std::string default_text =
      template_witness_node_text(ctx, *default_payload);
  const std::string substituted_default_text =
      semantic_utils::trim_space(
          template_witness_substituted_default_text(ctx,
                                                    default_text,
                                                    &parameters,
                                                    &arguments,
                                                    index));
  const std::string rendered_arg_text =
      semantic_utils::trim_space(template_witness_argument_text(ctx, argument));
  return template_witness_text_matches_default(parameter,
                                               rendered_arg_text,
                                               default_text) ||
         template_witness_text_matches_default(parameter,
                                               rendered_arg_text,
                                               substituted_default_text);
}

std::size_t witness_visible_template_argument_count(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    bool allow_explicit_default_equivalent)
{
  if(parameters.size() != arguments.size()) {
    return arguments.size();
  }

  std::size_t count = arguments.size();
  while(count > 0) {
    const template_model::TemplateParameterInfo & parameter =
        parameters[count - 1];
    if(parameter.parameter_pack ||
       !parameter.default_argument ||
       !template_argument_is_witness_default_equivalent(
           ctx,
           parameters,
           arguments,
           count - 1,
           allow_explicit_default_equivalent)) {
      break;
    }
    --count;
  }
  return count;
}

std::size_t witness_visible_class_template_argument_count(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info,
    bool allow_explicit_default_equivalent)
{
  if(!(info.source_template && !info.instantiation_arguments.empty())) {
    return info.instantiation_arguments.size();
  }
  return witness_visible_template_argument_count(
      ctx,
      info.source_template->parameters,
      info.instantiation_arguments,
      allow_explicit_default_equivalent);
}

std::size_t witness_visible_class_template_argument_count(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    bool allow_explicit_default_equivalent)
{
  return witness_visible_template_argument_count(
      ctx,
      info.template_parameters,
      info.arguments,
      allow_explicit_default_equivalent);
}

std::string source_spelled_enum_witness_argument_text(
    const template_model::TemplateArgument & argument)
{
  if(argument.kind != template_model::TemplateArgument::TA_VALUE ||
     argument.dependent ||
     !value_argument_has_named_enum_type(argument) ||
     !argument.source_syntax) {
    return std::string();
  }
  const std::string source_text =
      semantic_utils::trim_space(argument.source_syntax->source_text.empty() ?
                                     argument.source_syntax->text :
                                     argument.source_syntax->source_text);
  return is_qualified_identifier_value_text(source_text) ? source_text :
                                                           std::string();
}

std::string source_spelled_member_pointer_witness_argument_text(
    const template_model::TemplateArgument & argument)
{
  if(argument.kind != template_model::TemplateArgument::TA_VALUE ||
     argument.dependent ||
     !argument.source_syntax) {
    return std::string();
  }
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(argument.type);
  if(!base || base->kind != cpp_decl::Type::TK_MEMBER_POINTER) {
    return std::string();
  }
  return semantic_utils::trim_space(argument.source_syntax->source_text.empty() ?
                                       argument.source_syntax->text :
                                       argument.source_syntax->source_text);
}

bool template_argument_has_member_pointer_type(
    const template_model::TemplateArgument & argument)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(argument.type);
  return argument.kind == template_model::TemplateArgument::TA_VALUE &&
         base &&
         base->kind == cpp_decl::Type::TK_MEMBER_POINTER;
}

std::string member_pointer_witness_argument_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument)
{
  const std::string source_text =
      source_spelled_member_pointer_witness_argument_text(argument);
  if(!source_text.empty()) {
    return source_text;
  }
  if(!template_argument_has_member_pointer_type(argument) ||
     argument.dependent) {
    return std::string();
  }
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(argument.type);
  if(cpp_decl::is_function_type(base->inner)) {
    return std::string();
  }
  semantic_model::ClassInfo * owner = ctx.class_info_for_type(base->owner);
  if(!owner || !owner->member_scope) {
    return std::string();
  }
  for(std::map<std::string, semantic_model::ValueBinding>::const_iterator it =
          owner->member_scope->values.begin();
      it != owner->member_scope->values.end();
      ++it) {
    const semantic_model::ValueBinding & binding = it->second;
    if(binding.kind == semantic_model::ValueBinding::VK_FIELD &&
       !binding.is_bit_field &&
       argument.value >= 0 &&
       binding.field_offset == static_cast<std::size_t>(argument.value)) {
      const std::string owner_text =
          semantic_utils::trim_space(
              semantic_model::class_output_qualified_name(*owner));
      if(!owner_text.empty() && !binding.name.empty()) {
        return "&" + owner_text + "::" + binding.name;
      }
    }
  }
  return std::string();
}

bool template_argument_contains_source_spelled_enum(
    const template_model::TemplateArgument & argument)
{
  return !source_spelled_enum_witness_argument_text(argument).empty();
}

bool template_argument_contains_source_spelled_member_pointer(
    const template_model::TemplateArgument & argument)
{
  return template_argument_has_member_pointer_type(argument);
}

bool type_contains_qualified_enum_argument_for_witness(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type,
    unsigned depth);

bool template_argument_contains_qualified_enum_type_for_witness(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument,
    unsigned depth = 0)
{
  return argument.kind == template_model::TemplateArgument::TA_TYPE &&
         argument.type &&
         type_contains_qualified_enum_argument_for_witness(ctx,
                                                          argument.type,
                                                          depth + 1);
}

bool template_argument_has_underqualified_source_type_for_witness(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument)
{
  if(argument.kind != template_model::TemplateArgument::TA_TYPE ||
     !argument.type ||
     argument.text.empty() ||
     ctx.type_depends_on_template_parameter(argument.type)) {
    return false;
  }
  const std::string source =
      normalize_witness_angle_spacing(
          strip_witness_elaborated_type_prefix(argument.text));
  if(source.find("::") == std::string::npos) {
    return false;
  }
  std::string structured =
      normalize_witness_angle_spacing(
          strip_witness_elaborated_type_prefix(
              default_elided_type_argument_text(ctx, argument.type)));
  if(!witness_text_is_owner_qualified_suffix(structured, source)) {
    structured =
        normalize_witness_angle_spacing(
            strip_witness_elaborated_type_prefix(
                ctx.instantiation_identity_text_for_type_argument(
                    argument.type)));
  }
  if(source.empty() ||
     structured.empty() ||
     source == structured ||
     structured.size() <= source.size() + 2) {
    return false;
  }
  return witness_text_is_owner_qualified_suffix(structured, source);
}

std::string witness_text_for_qualified_enum_type(const cpp_decl::TypePtr & type);
std::string class_template_mangle_info_witness_text(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth);
bool class_template_mangle_info_contains_qualified_enum_for_witness(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth);
bool class_template_mangle_info_contains_default_elided_type_for_witness(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth);

std::string witness_specialization_name_for_visible_args(
    SemanticContext & ctx,
    const std::string & name,
    const std::vector<template_model::TemplateArgument> & arguments,
    std::size_t count)
{
  std::ostringstream out;
  out << name << "<";
  for(std::size_t i = 0; i < count && i < arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    if(arguments[i].kind == template_model::TemplateArgument::TA_TYPE &&
       arguments[i].type) {
      out << default_elided_type_argument_text(ctx, arguments[i].type);
    } else {
      const std::string enum_source_text =
          source_spelled_enum_witness_argument_text(arguments[i]);
      if(!enum_source_text.empty()) {
        out << enum_source_text;
      } else {
        const std::string member_pointer_text =
            member_pointer_witness_argument_text(ctx, arguments[i]);
        if(!member_pointer_text.empty()) {
          out << member_pointer_text;
        } else {
          out << template_model::template_argument_text(
              arguments[i],
              [&ctx](const cpp_decl::TypePtr & type)
              {
                return default_elided_type_argument_text(ctx, type);
              });
        }
      }
    }
  }
  out << ">";
  return out.str();
}

bool template_argument_has_typedef_source_spelling_for_witness(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument)
{
  if(argument.kind != template_model::TemplateArgument::TA_TYPE ||
     !argument.type ||
     argument.text.empty() ||
     ctx.type_depends_on_template_parameter(argument.type)) {
    return false;
  }
  const std::string source_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(argument.text));
  const std::string canonical_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(
              default_elided_type_argument_text(ctx, argument.type)));
  return !source_text.empty() &&
         !canonical_text.empty() &&
         source_text != canonical_text &&
         canonical_text.find("_GLOBAL__N_") == std::string::npos &&
         canonical_text.find("__local_") == std::string::npos;
}

std::string class_template_witness_scope_qualified_name(
    const semantic_model::Scope & scope,
    const std::string & name)
{
  std::vector<std::string> parts;
  parts.push_back(name);
  for(const semantic_model::Scope * current = &scope;
      current;
      current = current->parent) {
    if(!current->namespace_scope || current->name == "<global>") {
      continue;
    }
    if(current->name == "<unnamed>") {
      parts.push_back("(anonymous namespace)");
    } else if(!current->name.empty()) {
      parts.push_back(current->name);
    }
  }
  std::string out;
  for(std::size_t i = parts.size(); i > 0; --i) {
    if(!out.empty()) {
      out += "::";
    }
    out += parts[i - 1];
  }
  return out;
}

std::string class_witness_output_qualified_name_impl(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info,
    bool allow_explicit_default_equivalent,
    const std::size_t * visible_count_override = nullptr)
{
  if(class_is_named_function_local_for_witness(&info)) {
    return info.name;
  }
  const semantic_model::ClassInfo * owner = class_witness_owner(ctx, info);
  if(!info.source_template &&
     !info.is_lambda_closure &&
     owner &&
     !info.name.empty()) {
    return normalize_witness_angle_spacing(
        class_witness_output_qualified_name_impl(
            ctx,
            *owner,
            allow_explicit_default_equivalent) +
        "::" + info.name);
  }
  if(!info.source_template &&
     !info.is_lambda_closure &&
     info.instantiation_arguments.empty()) {
    const std::string anonymous_name =
        anonymous_symbol_class_log_name(info);
    if(!anonymous_name.empty()) {
      return normalize_witness_angle_spacing(anonymous_name);
    }
  }
  if(!info.source_template && info.type) {
    if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
           mangle_info =
               cpp_decl::named_type_class_template_specialization_mangle_info_const(
                   info.type)) {
      if(!mangle_info->template_name.empty()) {
        return class_template_mangle_info_witness_text(
            ctx,
            *mangle_info,
            0);
      }
    }
  }
  if(info.is_explicit_specialization) {
    return normalize_witness_angle_spacing(
        semantic_model::class_output_qualified_name(info));
  }
  if(!visible_count_override &&
     !allow_explicit_default_equivalent &&
     info.source_template &&
     info.type) {
    if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
           mangle_info =
               cpp_decl::named_type_class_template_specialization_mangle_info_const(
                   info.type)) {
      if(mangle_info->class_template_decl == info.source_template &&
         (class_template_mangle_info_contains_default_elided_type_for_witness(
              ctx,
              *mangle_info,
              0) ||
          class_template_mangle_info_contains_qualified_enum_for_witness(
              ctx,
              *mangle_info,
              0))) {
        return class_template_mangle_info_witness_text(ctx, *mangle_info, 0);
      }
    }
  }
  const std::size_t visible_count = visible_count_override ?
      std::min(*visible_count_override, info.instantiation_arguments.size()) :
      witness_visible_class_template_argument_count(
          ctx,
          info,
          allow_explicit_default_equivalent);
  const std::string structured_template_name =
      info.source_template ?
          info.source_template->name :
          semantic_utils::strip_trailing_top_level_template_arguments(
              info.name);
  bool rebuild_from_structured_arguments =
      !structured_template_name.empty() &&
      (!info.instantiation_arguments.empty() ||
       (info.source_template && owner && !info.dependent_instantiation));
  for(std::size_t i = 0;
      !rebuild_from_structured_arguments &&
      i < visible_count &&
      i < info.instantiation_arguments.size();
      ++i) {
    rebuild_from_structured_arguments =
        template_argument_contains_default_elided_type(
            ctx,
            info.instantiation_arguments[i]);
    if(!rebuild_from_structured_arguments &&
       template_argument_contains_source_spelled_enum(
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       template_argument_contains_source_spelled_member_pointer(
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       template_argument_contains_qualified_enum_type_for_witness(
           ctx,
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       template_argument_has_underqualified_source_type_for_witness(
           ctx,
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       template_argument_has_typedef_source_spelling_for_witness(
           ctx,
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       info.instantiation_arguments[i].kind ==
           template_model::TemplateArgument::TA_TYPE &&
       type_is_non_dependent_reference_argument(
           ctx,
           info.instantiation_arguments[i].type)) {
      rebuild_from_structured_arguments = true;
    }
    if(!rebuild_from_structured_arguments &&
       template_argument_contains_named_function_local_type_for_witness(
           ctx,
           info.instantiation_arguments[i])) {
      rebuild_from_structured_arguments = true;
    }
  }
  if(!rebuild_from_structured_arguments) {
    return normalize_witness_angle_spacing(
        semantic_model::class_output_qualified_name(info));
  }
  const std::string specialization_name =
      witness_specialization_name_for_visible_args(
          ctx,
          structured_template_name,
          info.instantiation_arguments,
          visible_count);
  const std::string qualified_name =
      owner ?
          class_witness_output_qualified_name_impl(
              ctx,
              *owner,
              allow_explicit_default_equivalent) +
              "::" + specialization_name :
          (info.enclosing_scope ?
               class_template_witness_scope_qualified_name(
                   *info.enclosing_scope,
                   specialization_name) :
               specialization_name);
  return normalize_witness_angle_spacing(qualified_name);
}

std::string class_witness_output_qualified_name(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return class_witness_output_qualified_name_impl(ctx, info, false);
}

std::string class_witness_output_qualified_name_with_visible_arguments(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info,
    std::size_t visible_count)
{
  return class_witness_output_qualified_name_impl(
      ctx,
      info,
      false,
      &visible_count);
}

std::string class_template_witness_qualified_name(
    SemanticContext & ctx,
    const semantic_model::ClassTemplateDecl & decl)
{
  const semantic_model::ClassInfo * owner =
      nearest_scope_class_info(decl.declaring_scope);
  if(owner) {
    return class_witness_output_qualified_name(ctx, *owner) +
        "::" + decl.name;
  }
  return decl.declaring_scope ?
      class_template_witness_scope_qualified_name(*decl.declaring_scope,
                                                  decl.name) :
      decl.name;
}

std::string class_witness_output_qualified_name_for_lifecycle(
    SemanticContext & ctx,
    const semantic_model::ClassInfo & info)
{
  return class_witness_output_qualified_name_impl(ctx, info, true);
}

// template-boundary-audit: begin text_recovery_bridge
std::string default_elided_type_argument_text(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type)
{
  std::string local_type_text;
  if(function_local_type_argument_text(ctx, type, local_type_text)) {
    return normalize_witness_angle_spacing(local_type_text);
  }
  if(type &&
     type->kind == cpp_decl::Type::TK_CV &&
     type->inner &&
     ctx.class_info_for_type(type->inner)) {
    std::string text = default_elided_type_argument_text(ctx, type->inner);
    if(type->cv_const && type->cv_volatile) {
      text = "const volatile " + text;
    } else if(type->cv_const) {
      text = "const " + text;
    } else if(type->cv_volatile) {
      text = "volatile " + text;
    }
    return normalize_witness_angle_spacing(text);
  }
  if(type &&
     (type->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ||
      type->kind == cpp_decl::Type::TK_RVALUE_REFERENCE) &&
     type->inner) {
    cpp_decl::TypePtr referenced = cpp_decl::strip_top_level_cv(type->inner);
    if(!(referenced &&
         referenced->kind == cpp_decl::Type::TK_NAMED &&
         referenced->named_rare().named_member_owner_type &&
         !cpp_decl::named_type_class_template_specialization_mangle_info_const(
              referenced))) {
      referenced.reset();
    }
    if(referenced) {
      return normalize_witness_angle_spacing(
          default_elided_type_argument_text(ctx, type->inner) +
          (type->kind == cpp_decl::Type::TK_LVALUE_REFERENCE ? " &" : " &&"));
    }
  }
  const std::string enum_text = witness_text_for_qualified_enum_type(type);
  if(!enum_text.empty()) {
    return enum_text;
  }
  cpp_decl::TypePtr named_type = cpp_decl::strip_top_level_cv(type);
  if(named_type &&
     named_type->kind == cpp_decl::Type::TK_NAMED &&
     named_type->named_rare().named_member_owner_type &&
     !named_type->named_rare().named_member_name.empty() &&
     !cpp_decl::named_type_class_template_specialization_mangle_info_const(
          named_type)) {
    cpp_decl::TypePtr owner_type = cpp_decl::strip_top_level_cv(
        named_type->named_rare().named_member_owner_type);
    if(semantic_model::ClassInfo * owner =
           ctx.class_info_for_type(owner_type)) {
      return normalize_witness_angle_spacing(
          class_witness_output_qualified_name(ctx, *owner) + "::" +
          named_type->named_rare().named_member_name);
    }
  }
  if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
         mangle_info =
             cpp_decl::named_type_class_template_specialization_mangle_info_const(type)) {
    if(class_template_mangle_info_contains_default_elided_type_for_witness(
           ctx,
           *mangle_info,
           0) ||
       class_template_mangle_info_contains_qualified_enum_for_witness(
           ctx,
           *mangle_info,
           0)) {
      return class_template_mangle_info_witness_text(ctx, *mangle_info, 0);
    }
  }
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(type)) {
    return class_witness_output_qualified_name(ctx, *info);
  }
  const std::string canonical_named_key =
      canonical_named_type_key_for_witness(type);
  if(!canonical_named_key.empty()) {
    return normalize_witness_angle_spacing(canonical_named_key);
  }
  std::string text =
      template_argument_semantics::lookup_text_for_type_argument(ctx, type);
  if(text.empty()) {
    text = cpp_decl::describe_type(type);
  }
  return normalize_witness_angle_spacing(text);
}
// template-boundary-audit: end text_recovery_bridge

std::size_t enum_witness_scope_operator_count(const std::string & text)
{
  std::size_t count = 0;
  for(std::size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == ':' && text[i + 1] == ':') {
      ++count;
      ++i;
    }
  }
  return count;
}

std::string strip_enum_witness_type_prefix(const std::string & text)
{
  static const char * prefixes[] = {
      "enum class ",
      "enum struct ",
      "enum "};
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const std::string prefix = prefixes[i];
    if(text.compare(0, prefix.size(), prefix) == 0) {
      return text.substr(prefix.size());
    }
  }
  return text;
}

std::string witness_text_for_qualified_enum_type(const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!(base &&
       base->kind == cpp_decl::Type::TK_NAMED &&
       (base->named_key.compare(0, 5, "enum ") == 0 ||
        named_type_display_text(base).compare(0, 5, "enum ") == 0))) {
    return std::string();
  }
  if(enum_witness_scope_operator_count(base->named_key) <=
     enum_witness_scope_operator_count(named_type_display_text(base))) {
    return std::string();
  }
  return normalize_witness_angle_spacing(
      strip_enum_witness_type_prefix(base->named_key));
}

bool named_type_needs_qualified_enum_witness_text(const cpp_decl::TypePtr & type)
{
  return !witness_text_for_qualified_enum_type(type).empty();
}

std::string class_template_mangle_info_witness_text(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth)
{
  if(depth > 8) {
    return std::string();
  }
  std::ostringstream out;
  const semantic_model::ClassTemplateDecl * class_template =
      static_cast<const semantic_model::ClassTemplateDecl *>(
          info.class_template_decl);
  if(class_template) {
    out << class_template_witness_qualified_name(ctx, *class_template);
  } else {
    if(!info.template_scope_prefix.empty()) {
      out << info.template_scope_prefix << "::";
    }
    out << info.template_name;
  }
  out << "<";
  const std::size_t visible_count =
      witness_visible_class_template_argument_count(ctx, info, false);
  for(std::size_t i = 0; i < visible_count && i < info.arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    const template_model::TemplateArgument & argument = info.arguments[i];
    if(argument.kind == template_model::TemplateArgument::TA_TYPE &&
       argument.type) {
      out << default_elided_type_argument_text(ctx, argument.type);
      continue;
    }
    const std::string enum_source_text =
        source_spelled_enum_witness_argument_text(argument);
    if(!enum_source_text.empty()) {
      out << enum_source_text;
      continue;
    }
    out << template_model::template_argument_text(
        argument,
        [&ctx](const cpp_decl::TypePtr & current_type)
        {
          return default_elided_type_argument_text(ctx, current_type);
        });
  }
  out << ">";
  return normalize_witness_angle_spacing(out.str());
}

bool class_template_mangle_info_contains_qualified_enum_for_witness(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth)
{
  if(depth > 8) {
    return false;
  }
  for(std::size_t i = 0; i < info.arguments.size(); ++i) {
    const template_model::TemplateArgument & argument = info.arguments[i];
    if(argument.kind == template_model::TemplateArgument::TA_TYPE &&
       argument.type &&
       type_contains_qualified_enum_argument_for_witness(ctx,
                                                        argument.type,
                                                        depth + 1)) {
      return true;
    }
    if(template_argument_contains_source_spelled_enum(argument)) {
      return true;
    }
  }
  return false;
}

bool class_template_mangle_info_contains_default_elided_type_for_witness(
    SemanticContext & ctx,
    const cpp_decl::ClassTemplateSpecializationMangleInfo & info,
    unsigned depth)
{
  if(depth > 8) {
    return false;
  }
  if(witness_visible_class_template_argument_count(ctx, info, false) <
     info.arguments.size()) {
    return true;
  }
  for(std::size_t i = 0; i < info.arguments.size(); ++i) {
    if(template_argument_contains_default_elided_type(ctx,
                                                     info.arguments[i],
                                                     depth + 1)) {
      return true;
    }
  }
  return false;
}

bool type_contains_qualified_enum_argument_for_witness(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type,
    unsigned depth)
{
  if(!type || depth > 8) {
    return false;
  }
  if(named_type_needs_qualified_enum_witness_text(type)) {
    return true;
  }
  if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
         mangle_info =
             cpp_decl::named_type_class_template_specialization_mangle_info_const(type)) {
    if(class_template_mangle_info_contains_qualified_enum_for_witness(
           ctx,
           *mangle_info,
           depth + 1)) {
      return true;
    }
  }
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(type)) {
    for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
      if(template_argument_contains_qualified_enum_type_for_witness(
             ctx,
             info->instantiation_arguments[i],
             depth + 1)) {
        return true;
      }
    }
  }
  return false;
}

bool type_contains_default_elided_template_argument(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type,
    unsigned depth)
{
  if(!type || depth > 8) {
    return false;
  }
  if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
         mangle_info =
             cpp_decl::named_type_class_template_specialization_mangle_info_const(type)) {
    if(class_template_mangle_info_contains_default_elided_type_for_witness(
           ctx,
           *mangle_info,
           depth + 1)) {
      return true;
    }
  }
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(type)) {
    if(witness_visible_class_template_argument_count(ctx, *info, false) <
       info->instantiation_arguments.size()) {
      return true;
    }
    for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
      if(template_argument_contains_default_elided_type(
             ctx,
             info->instantiation_arguments[i],
             depth + 1)) {
        return true;
      }
    }
  }
  return false;
}

bool template_argument_contains_default_elided_type(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument,
    unsigned depth)
{
  if(argument.kind != template_model::TemplateArgument::TA_TYPE ||
     !argument.type) {
    return false;
  }
  return type_contains_default_elided_template_argument(
      ctx,
      argument.type,
      depth + 1);
}

std::string value_binding_member_instantiation_entity(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding,
    const semantic_model::ClassInfo * owner_override = nullptr,
    const std::size_t * visible_owner_argument_count = nullptr)
{
  const semantic_model::ClassInfo * owner =
      owner_override ? owner_override : binding.owner_class;
  if(owner &&
     !semantic_model::class_output_qualified_name(*owner).empty()) {
    const std::string owner_entity = visible_owner_argument_count ?
        class_witness_output_qualified_name_with_visible_arguments(
            ctx,
            *owner,
            *visible_owner_argument_count) :
        class_witness_output_qualified_name(ctx, *owner);
    return owner_entity + "::" + binding.name;
  }
  return binding.name;
}

std::string value_binding_member_instantiation_decl_location(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding)
{
  const semantic_model::SourceDeclAnchorCache & anchor =
      semantic_trace::value_decl_anchor(ctx, &binding);
  return strip_at_prefix(semantic_model::source_decl_anchor_location(anchor));
}

bool source_type_lookup_is_collecting_owner_template_members(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding)
{
  if(!template_witness_source_type_lookup_active() ||
     !binding.owner_class ||
     !binding.owner_class->source_template) {
    return false;
  }

  const std::string public_location =
      normalize_template_witness_source_location(
          strip_at_prefix(ctx.template_witness_context().public_use_location));
  if(binding.name == "value" &&
     template_argument_semantics::default_template_argument_evaluation_active()) {
    return false;
  }
  if(public_location.empty()) {
    return true;
  }
  if(template_witness_qualified_member_type_lookup_active() ||
     template_witness_detail::source_location_line_mentions_qualified_member_token(
         ctx.template_witness_context(),
         public_location,
         "type")) {
    return false;
  }
  const auto source_line_mentions_identifier =
      [&](const std::string & identifier) -> bool
  {
    const TemplateWitnessContext & witness_context =
        ctx.template_witness_context();
    if(identifier.empty() ||
       !(witness_context.token_sequence && witness_context.source_locations)) {
      return false;
    }
    return !template_witness_detail::source_location_for_identifier_token_on_or_after(
                witness_context,
                public_location,
                identifier,
                true,
                false).empty();
  };
  if(binding.name != "value" &&
     template_witness_detail::source_location_line_mentions_qualified_member_token(
         ctx.template_witness_context(),
         public_location,
         binding.name)) {
    return false;
  }
  if(binding.name == "value") {
    return source_line_mentions_identifier("conditional_t");
  }

  const std::string owner_template_name =
      semantic_utils::unqualified_member_name(
          binding.owner_class->source_template->name);
  if(owner_template_name.empty()) {
    return false;
  }

  return !template_witness_detail::source_location_for_identifier_token_on_or_after(
              ctx.template_witness_context(),
              public_location,
	              owner_template_name,
	              true).empty();
}

bool source_location_is_inside_recorded_template_body(
    const TemplateWitnessContext & ctx,
    const std::string & location)
{
  if(!ctx.session) {
    return false;
  }
  const template_witness_detail::ParsedSourceLocation parsed =
      template_witness_detail::parse_source_location(
          normalize_template_witness_source_location(location));
  if(!parsed.valid) {
    return false;
  }
  for(std::size_t i = 0; i < ctx.session->template_body_ranges.size(); ++i) {
    const TemplateWitnessSourceRange & range =
        ctx.session->template_body_ranges[i];
    if(range.file != parsed.file ||
       parsed.line < range.begin_line ||
       parsed.line > range.end_line) {
      continue;
    }
    if(parsed.line == range.begin_line &&
       parsed.column < range.first_body_column) {
      continue;
    }
    return true;
  }
  return false;
}

bool default_argument_member_value_note_is_speculative(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding,
    const TemplateWitnessEntryContext & current_entry_context)
{
  if(!template_argument_semantics::default_template_argument_evaluation_active() ||
     current_entry_context.origin == TemplateWitnessOrigin::Closure) {
    return false;
  }

  const std::string public_location =
      normalize_template_witness_source_location(
          strip_at_prefix(ctx.template_witness_context().public_use_location));
  const std::string parser_location =
      normalize_template_witness_source_location(
          strip_at_prefix(parser_trace::current_use_location()));
  const std::string effective_location =
      !public_location.empty() ? public_location : parser_location;
  if(effective_location.empty()) {
    return true;
  }

  if(template_witness_qualified_member_type_lookup_active()) {
    return false;
  }
  if(template_witness_source_type_lookup_active() &&
     binding.name == "value") {
    return !source_location_is_inside_recorded_template_body(
        ctx.template_witness_context(),
        effective_location);
  }

  const bool source_line_mentions_member =
      template_witness_detail::source_location_line_mentions_qualified_member_token(
          ctx.template_witness_context(),
          effective_location,
          binding.name);
  if(template_witness_source_type_lookup_active() &&
     !source_line_mentions_member) {
    return false;
  }

  const std::string owner_template_name =
      binding.owner_class && binding.owner_class->source_template ?
          semantic_utils::unqualified_member_name(
              binding.owner_class->source_template->name) :
          std::string();
  if(!owner_template_name.empty() &&
     !template_witness_detail::source_location_for_identifier_token_on_or_after(
           ctx.template_witness_context(),
           effective_location,
           owner_template_name,
           true).empty()) {
    return false;
  }

  return true;
}

TemplateLifecycleTransition materialize_template_member_value_transition(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding,
    const TemplateMemberValueInstantiationRequest & request)
{
  TemplateLifecycleTransition transition;
  transition.value_binding = &binding;
  transition.source_value_binding =
      request.source_binding ? request.source_binding : &binding;
  transition.value_owner = request.source_owner ?
      request.source_owner :
      transition.source_value_binding->owner_class;
  transition.visible_owner_argument_count =
      request.visible_owner_argument_count;
  transition.has_visible_owner_argument_count =
      request.has_visible_owner_argument_count;
  const bool retained_dependency =
      request.origin ==
          TemplateMemberValueInstantiationOrigin::RetainedDependency;
  transition.retained_dependency = retained_dependency;
  if(retained_dependency &&
     binding.variable_template_instantiation &&
     binding.variable_template_instantiation->source_template) {
    // A retained member-variable-template dependency is still an acquisition
    // of its VariableTemplateDecl.  Keep it on the same lifecycle identity and
    // public entity as an ordinary variable-template instantiation instead of
    // manufacturing a second specialized static-member outcome.
    transition.variable_template =
        binding.variable_template_instantiation->source_template;
  }
  const bool trace_enabled = parser_trace::enabled("template.resolve");
  const auto trace_skip =
      [&](const char * reason) -> void
  {
    if(!trace_enabled) {
      return;
    }
    std::ostringstream trace;
    trace << "member-value-instantiation-skip reason=" << reason
          << " name=" << binding.name
          << " owner="
          << (binding.owner_class ? binding.owner_class->qualified_name :
                                    std::string("<none>"))
          << " owner-source-template="
          << (binding.owner_class && binding.owner_class->source_template ? "yes" : "no")
          << " owner-key="
          << (binding.owner_class ?
                  semantic_model::class_instantiation_key(
                      *binding.owner_class) :
                  std::string())
          << " dependent-owner="
          << (binding.owner_class && binding.owner_class->dependent_instantiation ? "yes" : "no")
          << " explicit-owner="
          << (binding.owner_class && binding.owner_class->is_explicit_specialization ? "yes" : "no")
          << " explicit-binding="
          << (binding.is_explicit_specialization ? "yes" : "no")
          << " public="
          << ctx.template_witness_context().public_use_location
          << " parser-use=" << parser_trace::current_use_location()
          << " source-type-active="
          << (template_witness_source_type_lookup_active() ? "yes" : "no")
          << " qmember-active="
          << (template_witness_qualified_member_type_lookup_active() ? "yes" : "no")
          << " default-arg-active="
          << (template_argument_semantics::default_template_argument_evaluation_active() ?
                  "yes" :
                  "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  };
  if(ctx.template_witness_context().session == nullptr ||
     !binding.owner_class) {
    trace_skip("no-session-or-owner");
    return transition;
  }
  if(binding.owner_class->is_explicit_specialization ||
     binding.is_explicit_specialization) {
    trace_skip("explicit-specialization");
    return transition;
  }
  if(!class_info_has_template_instantiation_key(*binding.owner_class) &&
     !enclosing_template_instantiation_owner(binding.owner_class)) {
    trace_skip("no-template-key");
    return transition;
  }
  if(binding.owner_class->dependent_instantiation) {
    trace_skip("dependent-owner");
    return transition;
  }
  const TemplateWitnessEntryContext current_entry_context =
      current_template_witness_entry_context();
  if(default_argument_member_value_note_is_speculative(ctx,
                                                       binding,
                                                       current_entry_context)) {
    trace_skip("default-arg-speculative");
    return transition;
  }
  const auto replay_static_member_definition_once =
      [&]() -> void
  {
    const bool replay_initializer =
        request.replay_static_member_initializer &&
        !binding.witness_static_member_initializer_replayed;
    if(binding.witness_static_member_definition_replayed &&
       !replay_initializer) {
      return;
    }
    if(template_instantiation::replay_witness_static_member_definition_if_needed(
           ctx,
           binding,
           nullptr,
           replay_initializer)) {
      binding.witness_static_member_definition_replayed = true;
      if(replay_initializer) {
        binding.witness_static_member_initializer_replayed = true;
      }
    }
  };
  if(!retained_dependency &&
     template_witness_source_type_lookup_active() &&
     template_witness_qualified_member_type_lookup_active()) {
    template_model::TemplateValueDependency dependency;
    dependency.entity =
        value_binding_member_instantiation_entity(ctx, binding);
    dependency.decl_location =
        value_binding_member_instantiation_decl_location(ctx, binding);
    dependency.value_scope = binding.declaration_scope;
    dependency.value_binding = &binding;
    dependency.value_owner = binding.owner_class;
    dependency.value_name = binding.name;
    if(binding.owner_class) {
      dependency.visible_owner_argument_count =
          witness_visible_class_template_argument_count(
              ctx,
              *binding.owner_class,
              true);
      dependency.has_visible_owner_argument_count = true;
    }
    dependency.entity_has_template_identity =
        value_or_owner_has_template_identity(&binding);
    dependency.public_use_location =
        normalize_template_witness_source_location(
            strip_at_prefix(ctx.template_witness_context().public_use_location));
    if(!dependency.entity.empty() &&
       !dependency.decl_location.empty() &&
       template_argument_semantics::
           collect_template_member_value_dependency_if_active(dependency)) {
      trace_skip("qualified-member-dependency-collected");
      return transition;
    }
  }

  const std::string entity =
      value_binding_member_instantiation_entity(ctx, binding);
  const std::string decl_location =
      value_binding_member_instantiation_decl_location(ctx, binding);
  if(entity.empty() || decl_location.empty()) {
    trace_skip("empty-entity-or-decl");
    return transition;
  }
  if(!retained_dependency &&
     source_type_lookup_is_collecting_owner_template_members(ctx, binding)) {
    trace_skip("source-type-owner-collection");
    return transition;
  }
  if(template_witness_detail::current_lifecycle_pause_depth_storage() != 0) {
    template_model::TemplateValueDependency dependency;
    dependency.entity = entity;
    dependency.decl_location = decl_location;
    dependency.value_scope = binding.declaration_scope;
    dependency.value_binding = &binding;
    dependency.value_owner = binding.owner_class;
    dependency.value_name = binding.name;
    if(binding.owner_class) {
      dependency.visible_owner_argument_count =
          witness_visible_class_template_argument_count(
              ctx,
              *binding.owner_class,
              true);
      dependency.has_visible_owner_argument_count = true;
    }
    dependency.entity_has_template_identity =
        value_or_owner_has_template_identity(&binding);
    dependency.public_use_location =
        normalize_template_witness_source_location(
            strip_at_prefix(ctx.template_witness_context().public_use_location));
    if(template_argument_semantics::
           collect_template_member_value_dependency_if_active(dependency)) {
      trace_skip("lifecycle-collected");
      return transition;
    }
    trace_skip("lifecycle-paused");
    return transition;
  }

  if(!retained_dependency) {
    binding.witness_member_value_source_capture_noted = true;
    replay_static_member_definition_once();
  }
  transition.transition_kind = TemplateLifecycleTransitionKind::Instantiated;
  return transition;
}

namespace {

enum TemplateLifecycleIdentityFlag
{
  LifecycleIdentityFunction = 1u,
  LifecycleIdentityValue = 2u,
  LifecycleIdentityClass = 4u,
  LifecycleIdentitySemanticEntity = 8u,
  LifecycleIdentitySourceLocationOwner = 16u,
  LifecycleIdentityInstantiationKey = 32u,
  LifecycleIdentityFinalizedClass = 64u,
  LifecycleIdentityTemplateDeclaration = 128u,
};

TemplateLifecycleIdentity template_lifecycle_value_identity(
    const semantic_model::ValueBinding * binding,
    const semantic_model::ClassInfo * owner_override = nullptr)
{
  TemplateLifecycleIdentity identity;
  const semantic_model::ClassInfo * owner = owner_override ?
      owner_override :
      (binding ? binding->owner_class : nullptr);
  if(!binding || !owner || binding->name.empty()) {
    return identity;
  }
  identity.entity = reinterpret_cast<std::size_t>(
      owner->source_template ?
          static_cast<const void *>(owner->source_template) :
          static_cast<const void *>(owner));
  identity.owner = reinterpret_cast<std::size_t>(owner->type.get());
  identity.instantiation = std::hash<std::string>()(
      semantic_model::class_instantiation_key(*owner));
  identity.event_anchor = std::hash<std::string>()(binding->name);
  identity.flags = LifecycleIdentityValue |
      (owner->source_template ? LifecycleIdentityTemplateDeclaration : 0u);
  for(std::size_t i = 0;
      i < owner->instantiation_arguments.size();
      ++i) {
    if(!owner->instantiation_arguments[i].source_defaulted) {
      ++identity.detail;
    }
  }
  return identity;
}

std::string function_lifecycle_transition_event_location(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition)
{
  const semantic_model::FunctionBinding * binding =
      transition.function_binding;
  if(!binding) {
    return std::string();
  }
  const std::string decl_location = binding_decl_location(ctx, binding);
  std::string event_location;
  if(transition.source_use_node) {
    event_location = strip_at_prefix(
        ctx.source_location_for_node(*transition.source_use_node));
  } else if(transition.use_declaration_as_event_location) {
    event_location = decl_location;
  } else {
    event_location = current_template_log_location(ctx);
  }
  return event_location.empty() ? decl_location : event_location;
}

TemplateLifecycleIdentity template_lifecycle_function_identity(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition,
    const std::string & event_location)
{
  TemplateLifecycleIdentity identity;
  const semantic_model::FunctionBinding * binding =
      transition.function_binding;
  if(!binding) {
    return identity;
  }
  identity.event_anchor = std::hash<std::string>()(event_location);
  identity.cause = transition.cause;
  identity.flags = LifecycleIdentityFunction;
  const std::string lookup_name =
      semantic_model::function_binding_display_name_for_symbol(*binding);
  identity.entity = std::hash<std::string>()(
      semantic_lookup::canonical_function_lookup_name(lookup_name));
  if(!binding->owner_class) {
    const semantic_model::Scope * declaration_scope =
        binding->source_template && binding->source_template->declaring_scope ?
            binding->source_template->declaring_scope :
            binding->declaration_scope;
    if(declaration_scope) {
      identity.owner = reinterpret_cast<std::size_t>(declaration_scope);
    }
  }
  if(binding->owner_class) {
    const semantic_model::ClassInfo * identity_owner =
        enclosing_template_instantiation_owner(binding->owner_class);
    if(!identity_owner) {
      identity_owner = binding->owner_class;
    }
    const CppAstNode * owner_declaration = identity_owner->source_template ?
        identity_owner->source_template->class_node :
        identity_owner->class_node;
    if(owner_declaration && owner_declaration->source_location_id != 0) {
      identity.owner = owner_declaration->source_location_id;
      identity.flags |= LifecycleIdentitySourceLocationOwner;
    } else {
      const void * owner_pointer = identity_owner->source_template ?
          static_cast<const void *>(identity_owner->source_template) :
          (owner_declaration ?
               static_cast<const void *>(owner_declaration) :
               static_cast<const void *>(identity_owner));
      identity.owner = reinterpret_cast<std::size_t>(owner_pointer);
    }
    const std::string owner_key = identity_owner->source_template ?
        class_witness_output_qualified_name_for_lifecycle(
            ctx, *identity_owner) :
        class_witness_output_qualified_name(ctx, *identity_owner);
    if(!owner_key.empty()) {
      identity.instantiation = std::hash<std::string>()(owner_key);
      identity.flags |= LifecycleIdentityInstantiationKey;
    }
  }
  return identity;
}

unsigned int template_lifecycle_transition_state_bit(
    TemplateLifecycleTransitionKind transition_kind)
{
  return 1u << (static_cast<unsigned int>(transition_kind) - 1);
}

bool claim_template_lifecycle_transition(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition,
    std::string * function_event_location)
{
  TemplateWitnessSession * session = ctx.template_witness_context().session;
  const unsigned int bit =
      template_lifecycle_transition_state_bit(transition.transition_kind);
  if(!session || bit == 0) {
    return false;
  }
  if(transition.value_binding) {
    const semantic_model::ValueBinding * identity_binding =
        transition.source_value_binding ? transition.source_value_binding :
                                          transition.value_binding;
    TemplateLifecycleIdentity request_identity =
        template_lifecycle_value_identity(identity_binding,
                                           transition.value_owner);
    if(transition.has_visible_owner_argument_count) {
      request_identity.detail =
          transition.visible_owner_argument_count;
    }
    if(request_identity.flags == 0 && transition.value_binding) {
      request_identity.entity = reinterpret_cast<std::size_t>(
          transition.value_binding);
      request_identity.cause = transition.cause;
      request_identity.flags = LifecycleIdentityValue;
    }
    if(request_identity.flags != 0) {
      unsigned int & state =
          session->lifecycle_transition_states[request_identity];
      if((state & bit) != 0) {
        return false;
      }
      state |= bit;
      return true;
    }
  }
  if(transition.function_binding) {
    const std::string event_location =
        function_lifecycle_transition_event_location(ctx, transition);
    const TemplateLifecycleIdentity identity =
        template_lifecycle_function_identity(ctx, transition, event_location);
    if(!event_location.empty()) {
      unsigned int & state =
          session->lifecycle_transition_states[identity];
      if(transition.transition_kind ==
             TemplateLifecycleTransitionKind::Instantiated &&
         transition.include_definition_materialized_detail &&
         !transition.created_new &&
         transition.definition_materialized &&
         (state & template_lifecycle_transition_state_bit(
                      TemplateLifecycleTransitionKind::DefinitionMaterialized)) != 0) {
        return false;
      }
      if((state & bit) != 0) {
        return false;
      }
      state |= bit;
      if(transition.transition_kind ==
             TemplateLifecycleTransitionKind::Instantiated &&
         transition.definition_materialized) {
        state |= template_lifecycle_transition_state_bit(
            TemplateLifecycleTransitionKind::DefinitionMaterialized);
      }
      if(function_event_location) {
        *function_event_location = event_location;
      }
      return true;
    }
  }
  return false;
}

void emit_template_lifecycle_event(TemplateLifecycleEvent event)
{
  CPPGM_NOTE_TEMPLATE_WITNESS_LIFECYCLE_EVENT(
      witness_provenance::WitnessProducerSite::LifecycleTransitionObserver01,
      event);
}

void observe_function_lifecycle_transition(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition,
    const std::string & event_location)
{
  const semantic_model::FunctionBinding * binding = transition.function_binding;
  const std::string entity = binding_log_entity(ctx, binding);
  const std::string decl_location = binding_decl_location(ctx, binding);
  if(entity.empty() || decl_location.empty() || event_location.empty()) {
    return;
  }
  TemplateLifecycleEvent event;
  event.location = event_location;
  event.entity = entity;
  event.decl_location = decl_location;
  event.cause = binding->is_explicit_specialization ?
      TemplateLifecycleCause::ExplicitSpecialization :
      transition.cause;
  event.entity_has_template_identity =
      function_or_owner_has_template_identity(binding);
  event.public_source_required =
      binding->template_definition_required_by_public_source_call;
  event.entity_is_constexpr_function = binding->is_constexpr;
  event.entity_is_defaulted_copy_or_move_constructor =
      binding->is_constructor &&
      (binding->is_copy_constructor || binding->is_move_constructor) &&
      (binding->is_defaulted || binding->synthesized);
  switch(transition.transition_kind) {
  case TemplateLifecycleTransitionKind::DefinitionRequired:
    event.kind = TemplateLifecycleEventKind::RequireDefinition;
    break;
  case TemplateLifecycleTransitionKind::DefinitionEnsured:
    event.kind = TemplateLifecycleEventKind::EnsureDefinition;
    break;
  case TemplateLifecycleTransitionKind::DefinitionMaterialized:
    event.kind = TemplateLifecycleEventKind::FunctionInstantiation;
    event.detail = function_instantiation_detail(false, true);
    break;
  case TemplateLifecycleTransitionKind::Instantiated:
    event.kind = TemplateLifecycleEventKind::FunctionInstantiation;
    event.detail = transition.include_definition_materialized_detail ?
        function_instantiation_detail(transition.created_new,
                                      transition.definition_materialized) :
        created_new_detail(transition.created_new);
    break;
  default:
    return;
  }
  emit_template_lifecycle_event(event);
}

std::string class_template_lifecycle_decl_location(
    SemanticContext & ctx,
    const semantic_model::ClassTemplateDecl * decl)
{
  if(!(decl && decl->class_node)) {
    return std::string();
  }
  std::string location = strip_at_prefix(
      ctx.source_location_for_name_in_node(*decl->class_node, decl->name));
  if(!location.empty()) {
    return normalize_template_witness_source_location(location);
  }
  return normalize_template_witness_source_location(
      strip_at_prefix(ctx.source_location_for_node(*decl->class_node)));
}

std::string class_lifecycle_transition_event_location(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition,
    const std::string & decl_location)
{
  if(transition.source_use_node) {
    if(transition.use_declaration_as_event_location &&
       transition.class_info &&
       !transition.class_info->name.empty()) {
      const std::string named = normalize_template_witness_source_location(
          strip_at_prefix(ctx.source_location_for_name_in_node(
              *transition.source_use_node,
              transition.class_info->name)));
      if(!named.empty()) {
        return named;
      }
    }
    return normalize_template_witness_source_location(
        strip_at_prefix(
            ctx.source_location_for_node(*transition.source_use_node)));
  }
  if(transition.use_declaration_as_event_location) {
    return decl_location;
  }
  return current_template_log_location(ctx);
}

bool class_lifecycle_transition_suppressed(
    const TemplateLifecycleTransition & transition)
{
  if(transition.transition_kind ==
         TemplateLifecycleTransitionKind::AnonymousMemberInstantiated) {
    return false;
  }
  const semantic_model::ClassInfo * info = transition.class_info;
  if(transition.transition_kind !=
         TemplateLifecycleTransitionKind::Instantiated ||
     !(info && info->source_template && info->template_output_node &&
       info->source_template->declaring_scope &&
       info->source_template->declaring_scope->class_info)) {
    return false;
  }
  for(std::size_t i = 0;
      i < info->source_template->partial_specializations.size();
      ++i) {
    const semantic_model::PartialClassTemplateSpecializationDecl & partial =
        info->source_template->partial_specializations[i];
    if(partial.class_node == info->template_output_node &&
       partial.declaring_scope &&
       partial.declaring_scope != info->source_template->declaring_scope) {
      return true;
    }
  }
  return false;
}

bool claim_class_lifecycle_transition(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition,
    const std::string & event_location)
{
  TemplateWitnessSession * session = ctx.template_witness_context().session;
  const semantic_model::ClassInfo * info = transition.class_info;
  const void * entity = transition.class_template ?
      static_cast<const void *>(transition.class_template) :
      static_cast<const void *>(info);
  std::string semantic_entity_key;
  if(info) {
    semantic_entity_key = class_decl_location(ctx, info);
    semantic_entity_key += "\n";
    semantic_entity_key += class_log_entity(ctx, info);
    if(info->source_template) {
      semantic_entity_key += "\n";
      semantic_entity_key += class_template_instantiation_key(*info);
    }
  }
  const unsigned int bit =
      template_lifecycle_transition_state_bit(transition.transition_kind);
  if(!session || !entity || bit == 0 || event_location.empty()) {
    return false;
  }
  TemplateLifecycleIdentity identity;
  identity.entity = reinterpret_cast<std::size_t>(entity);
  identity.flags = LifecycleIdentityClass;
  if(!semantic_entity_key.empty()) {
    identity.entity = std::hash<std::string>()(semantic_entity_key);
    identity.flags |= LifecycleIdentitySemanticEntity;
  }
  identity.event_anchor = std::hash<std::string>()(event_location);
  identity.cause = transition.cause;
  identity.flags |=
      (transition.class_finalized ? LifecycleIdentityFinalizedClass : 0u) |
      (transition.class_template ? LifecycleIdentityTemplateDeclaration : 0u);
  unsigned int & state = session->lifecycle_transition_states[identity];
  if((state & bit) != 0) {
    return false;
  }
  state |= bit;
  return true;
}

void observe_class_lifecycle_transition_in_current_context(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition)
{
  if(class_lifecycle_transition_suppressed(transition)) {
    return;
  }
  const semantic_model::ClassInfo * info = transition.class_info;
  const semantic_model::ClassTemplateDecl * decl = transition.class_template;
  semantic_model::AnonymousMemberClassInfo anonymous_member;
  if(transition.transition_kind ==
         TemplateLifecycleTransitionKind::AnonymousMemberInstantiated &&
     transition.source_use_node) {
    anonymous_member.class_node = transition.source_use_node;
    if(const CppAstNode * class_key =
           cpp_decl::find_child(*transition.source_use_node,
                                CppAstKind::class_key)) {
      anonymous_member.class_kind = cpp_decl::node_text(*class_key);
    }
  }
  const std::string entity =
      transition.transition_kind ==
              TemplateLifecycleTransitionKind::AnonymousMemberInstantiated &&
          info ?
          anonymous_member_class_log_entity(ctx, *info, anonymous_member) :
          (info ? class_log_entity(ctx, info) :
                  (decl ? decl->name : std::string()));
  std::string decl_location =
      transition.transition_kind ==
              TemplateLifecycleTransitionKind::AnonymousMemberInstantiated &&
          transition.source_use_node ?
          normalize_template_witness_source_location(strip_at_prefix(
              ctx.source_location_for_node(*transition.source_use_node))) :
          (info ? class_decl_location(ctx, info) :
                  class_template_lifecycle_decl_location(ctx, decl));
  const std::string event_location =
      class_lifecycle_transition_event_location(
          ctx, transition, decl_location);
  if(transition.use_declaration_as_event_location &&
     transition.transition_kind !=
         TemplateLifecycleTransitionKind::AnonymousMemberInstantiated) {
    decl_location = event_location;
  }
  if(entity.empty() || decl_location.empty() || event_location.empty() ||
     !claim_class_lifecycle_transition(ctx, transition, event_location)) {
    return;
  }

  TemplateLifecycleEvent event;
  event.location = event_location;
  event.entity = entity;
  event.decl_location = decl_location;
  switch(transition.transition_kind) {
  case TemplateLifecycleTransitionKind::Instantiated:
  case TemplateLifecycleTransitionKind::AnonymousMemberInstantiated:
    event.kind = TemplateLifecycleEventKind::ClassInstantiation;
    event.detail = created_new_detail(transition.created_new);
    break;
  case TemplateLifecycleTransitionKind::Finalized:
    event.kind = TemplateLifecycleEventKind::ClassFinalization;
    if(info) {
      event.detail = std::string("finalized=") +
          (transition.class_finalized ? "yes" : "no");
    }
    break;
  default:
    return;
  }

  event.cause =
      transition.cause != TemplateLifecycleCause::None ?
          transition.cause :
          lifecycle_cause_for_current_context_or_default(
              transition.transition_kind ==
                      TemplateLifecycleTransitionKind::Finalized ?
                  TemplateLifecycleCause::FinalizeClass :
                  TemplateLifecycleCause::ImplicitUse);
  event.entity_has_template_identity =
      info ? class_has_template_identity(info) : true;
  event.entity_is_unnamed_class =
      transition.transition_kind ==
          TemplateLifecycleTransitionKind::AnonymousMemberInstantiated ||
      (info ? class_is_unnamed_for_witness(info) : false);
  emit_template_lifecycle_event(event);
}

void observe_class_lifecycle_transition(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition)
{
  if(transition.class_info &&
     transition.source_use_node &&
     transition.use_declaration_as_event_location &&
     (transition.transition_kind ==
          TemplateLifecycleTransitionKind::AnonymousMemberInstantiated ||
      !transition.class_info->source_is_unnamed_class)) {
    semantic_model::AnonymousMemberClassInfo anonymous_member;
    if(transition.transition_kind ==
       TemplateLifecycleTransitionKind::AnonymousMemberInstantiated) {
      anonymous_member.class_node = transition.source_use_node;
      if(const CppAstNode * class_key =
             cpp_decl::find_child(*transition.source_use_node,
                                  CppAstKind::class_key)) {
        anonymous_member.class_kind = cpp_decl::node_text(*class_key);
      }
    }
    const std::string entity =
        transition.transition_kind ==
                TemplateLifecycleTransitionKind::AnonymousMemberInstantiated ?
        anonymous_member_class_log_entity(
            ctx, *transition.class_info, anonymous_member) :
        class_log_entity(ctx, transition.class_info);
    const std::string decl_location =
        class_lifecycle_transition_event_location(
            ctx,
            transition,
            class_decl_location(ctx, transition.class_info));
    const ScopedTemplateWitnessEntryContext entry_context(
        make_template_closure_entry_context(
            TemplateClosureReason::TrackInstantiation,
            entity,
            decl_location,
            class_has_template_identity(transition.class_info),
            TemplateWitnessTriggerKind::Class));
    observe_class_lifecycle_transition_in_current_context(ctx, transition);
    return;
  }
  if(transition.class_info || !transition.class_template) {
    observe_class_lifecycle_transition_in_current_context(ctx, transition);
    return;
  }
  const std::string entity = transition.class_template->name;
  const std::string decl_location = class_template_lifecycle_decl_location(
      ctx, transition.class_template);
  const ScopedTemplateWitnessEntryContext entry_context(
      make_template_closure_entry_context(
          TemplateClosureReason::FinalizeClass,
          entity,
          decl_location,
          true,
          TemplateWitnessTriggerKind::Class));
  observe_class_lifecycle_transition_in_current_context(ctx, transition);
}

}  // namespace

void observe_template_lifecycle_transition(
    SemanticContext & ctx,
    const TemplateLifecycleTransition & transition)
{
  if(!transition.valid()) {
    return;
  }
  const bool completed_class_transition =
      transition.cause == TemplateLifecycleCause::TrackInstantiation &&
      transition.class_info &&
      ((transition.transition_kind ==
            TemplateLifecycleTransitionKind::Instantiated &&
        transition.class_info->template_instantiation_tracked) ||
       transition.transition_kind ==
           TemplateLifecycleTransitionKind::AnonymousMemberInstantiated);
  if(template_witness_detail::current_lifecycle_pause_depth_storage() != 0 &&
     !completed_class_transition) {
    return;
  }
  if(transition.class_info ||
     (!transition.function_binding && !transition.value_binding &&
      transition.class_template)) {
    observe_class_lifecycle_transition(ctx, transition);
    return;
  }
  std::string function_event_location;
  if((transition.function_binding &&
      (transition.function_binding->is_inherited_constructor ||
       (transition.function_binding->owner_class &&
        transition.function_binding->owner_class->dependent_instantiation))) ||
     !claim_template_lifecycle_transition(
         ctx, transition, &function_event_location)) {
    return;
  }
  if(transition.function_binding) {
    observe_function_lifecycle_transition(
        ctx, transition, function_event_location);
    return;
  }
  if(!transition.value_binding) {
    return;
  }

  const semantic_model::ValueBinding & binding = *transition.value_binding;
  const semantic_model::ValueBinding & source_binding =
      transition.source_value_binding ? *transition.source_value_binding :
                                        binding;
  const bool variable_template_acquisition =
      transition.variable_template != nullptr;
  const std::string binding_entity = variable_template_acquisition ?
      value_log_entity(ctx, &binding) :
      value_binding_member_instantiation_entity(
          ctx,
          source_binding,
          transition.value_owner,
          transition.has_visible_owner_argument_count ?
              &transition.visible_owner_argument_count :
              nullptr);
  const std::string template_entity = variable_template_acquisition ?
      variable_template_decl_log_entity(transition.variable_template) :
      std::string();
  const std::string entity =
      !template_entity.empty() ? template_entity : binding_entity;
  const std::string binding_decl_location = variable_template_acquisition ?
      value_decl_location(ctx, &binding) :
      value_binding_member_instantiation_decl_location(ctx, source_binding);
  const std::string template_decl_location = variable_template_acquisition ?
      variable_template_decl_location(ctx, transition.variable_template) :
      std::string();
  const std::string decl_location =
      !binding_decl_location.empty() ? binding_decl_location :
                                      template_decl_location;
  if(entity.empty() || decl_location.empty()) {
    return;
  }

  TemplateLifecycleEvent event;
  event.kind = TemplateLifecycleEventKind::VariableInstantiation;
  event.entity = entity;
  event.decl_location = decl_location;
  const semantic_model::ClassInfo * const semantic_owner =
      transition.value_owner ? transition.value_owner : binding.owner_class;
  event.semantic_owner_type = semantic_owner ?
      semantic_owner->type.get() : nullptr;
  event.entity_has_template_identity =
      value_or_owner_has_template_identity(&binding) ||
      transition.variable_template != nullptr;
  event.detail = transition.retained_dependency ?
      std::string() :
      created_new_detail(transition.created_new);
  event.cause = transition.retained_dependency ?
      TemplateLifecycleCause::TrackInstantiation :
      (variable_template_acquisition ?
           transition.cause :
           TemplateLifecycleCause::None);
  event.location = variable_template_acquisition ?
      current_template_log_location(ctx) :
      decl_location;
  event.public_source_required = transition.retained_dependency;

  if(variable_template_acquisition &&
     event.cause == TemplateLifecycleCause::TrackInstantiation &&
     current_template_witness_entry_context().origin !=
         TemplateWitnessOrigin::Closure) {
    const ScopedTemplateWitnessEntryContext entry_context(
        make_template_closure_entry_context(
            TemplateClosureReason::TrackInstantiation,
            entity,
            decl_location,
            event.entity_has_template_identity,
            TemplateWitnessTriggerKind::Variable));
    emit_template_lifecycle_event(event);
    return;
  }

  const ScopedTemplateWitnessEntryContext entry_context =
      maybe_enter_value_binding_closure_context(
          ctx,
          TemplateClosureReason::TrackInstantiation,
          &binding);
  emit_template_lifecycle_event(event);
}

void observe_template_member_value_transition(
    SemanticContext & ctx,
    const semantic_model::ValueBinding & binding,
    const TemplateMemberValueInstantiationRequest & request)
{
  TemplateMemberValueInstantiationRequest effective_request = request;
  if(effective_request.origin ==
         TemplateMemberValueInstantiationOrigin::SemanticUse &&
     ctx.template_witness_context().public_source_use_active) {
    effective_request.replay_static_member_initializer = true;
  }
  observe_template_lifecycle_transition(
      ctx,
      materialize_template_member_value_transition(ctx,
                                                   binding,
                                                   effective_request));
}

TemplateLifecycleTransition
note_nested_member_class_instantiation_completed_if_needed(
    SemanticContext &,
    semantic_model::ClassInfo * info,
    const CppAstNode * preferred_decl_node,
    const CppAstNode * fallback_decl_node)
{
  TemplateLifecycleTransition transition;
  if(!info) {
    return transition;
  }
  if(!info->template_instantiation_tracked) {
    info->template_instantiation_tracked = true;
  }
  if(!preferred_decl_node && !fallback_decl_node) {
    return transition;
  }
  transition.transition_kind = TemplateLifecycleTransitionKind::Instantiated;
  transition.class_info = info;
  transition.cause = TemplateLifecycleCause::TrackInstantiation;
  transition.source_use_node = preferred_decl_node ?
      preferred_decl_node : fallback_decl_node;
  transition.use_declaration_as_event_location = true;
  return transition;
}

void observe_source_unnamed_class_completion(
    SemanticContext & ctx,
    semantic_model::ClassInfo & info)
{
  if(!info.source_is_unnamed_class ||
     !class_has_template_identity(&info) ||
     info.dependent_instantiation ||
     !info.complete) {
    return;
  }

  if(lexical_function_for_class(&info)) {
    TemplateLifecycleTransition finalized;
    finalized.transition_kind = TemplateLifecycleTransitionKind::Finalized;
    finalized.class_info = &info;
    finalized.source_use_node = info.source_unnamed_class_node;
    finalized.cause = TemplateLifecycleCause::FinalizeClass;
    finalized.class_finalized = info.complete;
    finalized.use_declaration_as_event_location = true;
    observe_template_lifecycle_transition(ctx, finalized);
  }

  TemplateLifecycleTransition instantiated;
  instantiated.transition_kind = TemplateLifecycleTransitionKind::Instantiated;
  instantiated.class_info = &info;
  instantiated.source_use_node = info.source_unnamed_class_node;
  instantiated.cause = TemplateLifecycleCause::TrackInstantiation;
  instantiated.use_declaration_as_event_location = true;
  observe_template_lifecycle_transition(ctx, instantiated);
}

void observe_anonymous_member_class_completion(
    SemanticContext & ctx,
    semantic_model::ClassInfo & owner,
    const CppAstNode & class_node)
{
  if(!class_has_template_identity(&owner) || owner.dependent_instantiation) {
    return;
  }
  TemplateLifecycleTransition transition;
  transition.transition_kind =
      TemplateLifecycleTransitionKind::AnonymousMemberInstantiated;
  transition.class_info = &owner;
  transition.source_use_node = &class_node;
  transition.cause = TemplateLifecycleCause::TrackInstantiation;
  transition.use_declaration_as_event_location = true;
  observe_template_lifecycle_transition(ctx, transition);
}

namespace {

// template-boundary-audit: begin text_recovery_bridge
std::string witness_lookup_text_for_type_argument(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type);
bool witness_argument_text_should_prefer_structured(
    const std::string & structured_text,
    const std::string & source_text);

std::string normalize_witness_function_type_argument_text(
    const cpp_decl::TypePtr & type,
    const std::string & text)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  const bool semantic_function_type =
      base && base->kind == cpp_decl::Type::TK_FUNCTION;
  std::string out = semantic_utils::trim_space(text);
  int angle_depth = 0;
  for(std::size_t i = 0; i < out.size(); ++i) {
    const char ch = out[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(' && angle_depth == 0) {
      if(i > 0 &&
         !std::isspace(static_cast<unsigned char>(out[i - 1])) &&
         (semantic_function_type || out[i - 1] == '>')) {
        out.insert(i, " ");
      }
      return out;
    }
  }
  return out;
}

std::string witness_argument_text_for_binding(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg)
{
  if(arg.kind == template_model::TemplateArgument::TA_VALUE) {
    const std::string member_pointer_text =
        member_pointer_witness_argument_text(ctx, arg);
    if(!member_pointer_text.empty()) {
      return member_pointer_text;
    }
  }
  if(arg.kind == template_model::TemplateArgument::TA_TYPE && arg.type) {
    const std::string structured_text =
        witness_lookup_text_for_type_argument(ctx, arg.type);
    if((cpp_decl::is_reference_type(arg.type) &&
        !ctx.type_depends_on_template_parameter(arg.type)) ||
       type_contains_default_elided_template_argument(ctx, arg.type, 0) ||
       template_argument_contains_named_function_local_type_for_witness(
           ctx,
           arg) ||
       arg.text.empty() ||
       witness_argument_text_should_prefer_structured(structured_text,
                                                      arg.text)) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          structured_text);
    }
  }
  return normalize_witness_angle_spacing(
      normalize_witness_function_type_argument_text(
          arg.type,
          template_model::template_argument_text(
              arg,
              [&ctx](const cpp_decl::TypePtr & type)
              {
                return semantic_utils::trim_space(
                    template_argument_semantics::lookup_text_for_type_argument(ctx,
                                                                               type));
              })));
}
// template-boundary-audit: end text_recovery_bridge

bool template_witness_argument_is_type_like(
    const template_model::TemplateArgument & arg)
{
  return arg.kind == template_model::TemplateArgument::TA_TYPE;
}

bool template_witness_argument_range_is_type_like(
    const std::vector<template_model::TemplateArgument> & arguments,
    std::size_t begin,
    std::size_t end)
{
  if(begin >= end || end > arguments.size()) {
    return false;
  }
  for(std::size_t i = begin; i < end; ++i) {
    if(!template_witness_argument_is_type_like(arguments[i])) {
      return false;
    }
  }
  return true;
}

std::string witness_binding_param_name(
    const template_model::TemplateParameterInfo & param,
    std::size_t index)
{
  return param.name.empty() ? std::string("$") + std::to_string(index + 1) :
                              param.name;
}

bool type_contains_template_parameter_placeholder(
    const cpp_decl::TypePtr & type,
    const template_model::TemplateParameterInfo & param)
{
  if(!type) {
    return false;
  }
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base) {
    base = type;
  }
  switch(base->kind) {
  case cpp_decl::Type::TK_NAMED:
    if((!param.placeholder_key.empty() && base->named_key == param.placeholder_key) ||
       (!param.name.empty() && base->named_key == param.name)) {
      return true;
    }
    for(std::size_t i = 0; i < param.alternate_names.size(); ++i) {
      if(base->named_key == param.alternate_names[i]) {
        return true;
      }
    }
    return false;
  case cpp_decl::Type::TK_FUNCTION:
    if(type_contains_template_parameter_placeholder(base->inner, param)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_contains_template_parameter_placeholder(base->params[i], param)) {
        return true;
      }
    }
    return false;
  case cpp_decl::Type::TK_POINTER:
  case cpp_decl::Type::TK_BLOCK_POINTER:
  case cpp_decl::Type::TK_LVALUE_REFERENCE:
  case cpp_decl::Type::TK_RVALUE_REFERENCE:
  case cpp_decl::Type::TK_ARRAY:
  case cpp_decl::Type::TK_ATOMIC:
  case cpp_decl::Type::TK_CV:
    return type_contains_template_parameter_placeholder(base->inner, param) ||
        (base->owner &&
         type_contains_template_parameter_placeholder(base->owner, param));
  case cpp_decl::Type::TK_MEMBER_POINTER:
    return type_contains_template_parameter_placeholder(base->owner, param) ||
        type_contains_template_parameter_placeholder(base->inner, param);
  case cpp_decl::Type::TK_FUNDAMENTAL:
    return false;
  }
  return false;
}

bool function_template_parameter_is_deduced_from_call(
    const semantic_model::FunctionTemplateDecl & decl,
    const template_model::TemplateParameterInfo & param)
{
  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    if(type_contains_template_parameter_placeholder(decl.params_pattern[i].second,
                                                   param)) {
      return true;
    }
  }
  return false;
}

}  // namespace

void append_function_template_witness_bindings(
    SemanticContext & ctx,
    const semantic_model::FunctionBinding * binding,
    std::size_t explicit_arg_count,
    std::vector<TemplateWitnessSourceBinding> & out)
{
  if(!(binding && binding->source_template && binding->has_instantiation_arguments)) {
    return;
  }
  const std::vector<template_model::TemplateParameterInfo> & params =
      binding->source_template->parameters;
  std::size_t argument_index = 0;
  for(std::size_t i = 0; i < params.size(); ++i) {
    const template_model::TemplateParameterInfo & param = params[i];
    TemplateWitnessSourceBinding source_binding;
    source_binding.param = witness_binding_param_name(param, i);
    source_binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(param);
    const bool explicit_source = i < explicit_arg_count;
    const bool defaulted_source =
        !explicit_source &&
        param.default_argument &&
        !function_template_parameter_is_deduced_from_call(
            *binding->source_template,
            param);
    source_binding.source = explicit_source ? "explicit" :
        (defaulted_source ? "defaulted" : "deduced");
    if(param.parameter_pack) {
      source_binding.pack_binding = true;
      std::size_t pack_count = 0;
      if(!param.name.empty()) {
        std::map<std::string, std::size_t>::const_iterator named_count =
            binding->instantiation_pack_sizes.find(param.name);
        if(named_count != binding->instantiation_pack_sizes.end()) {
          pack_count = named_count->second;
        }
      }
      if(pack_count == 0 && !param.placeholder_key.empty()) {
        std::map<std::string, std::size_t>::const_iterator placeholder_count =
            binding->instantiation_pack_sizes.find(param.placeholder_key);
        if(placeholder_count != binding->instantiation_pack_sizes.end()) {
          pack_count = placeholder_count->second;
        }
      }
      if(pack_count == 0 && i + 1 == params.size() &&
         binding->instantiation_arguments.size() >= argument_index) {
        pack_count = binding->instantiation_arguments.size() - argument_index;
      }
      if(pack_count == 0) {
        source_binding.arg = "<>";
      } else {
        source_binding.pack_aggregate = pack_count > 1;
        source_binding.type_like =
            template_witness_argument_range_is_type_like(
                binding->instantiation_arguments,
                argument_index,
                argument_index + pack_count);
        std::ostringstream pack_text;
        pack_text << "<";
        for(std::size_t j = 0;
            j < pack_count &&
            argument_index + j < binding->instantiation_arguments.size();
            ++j) {
          if(j != 0) {
            pack_text << ", ";
          }
          const std::string element_text = witness_argument_text_for_binding(
              ctx,
              binding->instantiation_arguments[argument_index + j]);
          source_binding.pack_arguments.push_back(element_text);
          pack_text << element_text;
        }
        pack_text << ">";
        source_binding.arg = pack_text.str();
      }
      argument_index += pack_count;
      out.push_back(source_binding);
      continue;
    }
    if(argument_index >= binding->instantiation_arguments.size()) {
      continue;
    }
    source_binding.type_like = template_witness_argument_is_type_like(
        binding->instantiation_arguments[argument_index]);
    source_binding.arg = witness_argument_text_for_binding(
        ctx,
        binding->instantiation_arguments[argument_index++]);
    out.push_back(source_binding);
  }
}

void append_class_template_witness_bindings(
    SemanticContext & ctx,
    const semantic_model::ClassInfo * info,
    std::vector<TemplateWitnessSourceBinding> & out)
{
  if(!(info && info->source_template)) {
    return;
  }
  const std::vector<template_model::TemplateParameterInfo> & params =
      info->source_template->parameters;
  std::size_t argument_index = 0;
  for(std::size_t i = 0; i < params.size(); ++i) {
    const template_model::TemplateParameterInfo & param = params[i];
    TemplateWitnessSourceBinding source_binding;
    source_binding.param = witness_binding_param_name(param, i);
    source_binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(param);
    if(param.parameter_pack) {
      source_binding.pack_binding = true;
      const std::size_t pack_count =
          i + 1 == params.size() &&
          info->instantiation_arguments.size() >= argument_index ?
              info->instantiation_arguments.size() - argument_index :
              0;
      if(pack_count == 0) {
        source_binding.arg = "<>";
        source_binding.source = "deduced";
      } else {
        source_binding.pack_aggregate = pack_count > 1;
        source_binding.type_like =
            template_witness_argument_range_is_type_like(
                info->instantiation_arguments,
                argument_index,
                argument_index + pack_count);
        std::ostringstream pack_text;
        pack_text << "<";
        bool all_defaulted = true;
        for(std::size_t j = 0; j < pack_count; ++j) {
          if(j != 0) {
            pack_text << ", ";
          }
          const template_model::TemplateArgument & argument =
              info->instantiation_arguments[argument_index + j];
          all_defaulted = all_defaulted && argument.source_defaulted;
          const std::string element_text =
              witness_argument_text_for_binding(ctx, argument);
          source_binding.pack_arguments.push_back(element_text);
          pack_text << element_text;
        }
        pack_text << ">";
        source_binding.arg = pack_text.str();
        source_binding.source = all_defaulted ? "defaulted" : "explicit";
      }
      argument_index += pack_count;
      out.push_back(source_binding);
      continue;
    }
    if(argument_index >= info->instantiation_arguments.size()) {
      break;
    }
    source_binding.type_like = template_witness_argument_is_type_like(
        info->instantiation_arguments[argument_index]);
    source_binding.arg = witness_argument_text_for_binding(
        ctx,
        info->instantiation_arguments[argument_index++]);
    source_binding.source =
        info->instantiation_arguments[argument_index - 1].source_defaulted ?
            "defaulted" :
            "explicit";
    out.push_back(source_binding);
  }
}

namespace {

std::size_t witness_scope_operator_count(const std::string & text)
{
  std::size_t count = 0;
  for(std::size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == ':' && text[i + 1] == ':') {
      ++count;
      ++i;
    }
  }
  return count;
}

bool witness_argument_text_should_prefer_structured(const std::string & structured_text,
                                                    const std::string & source_text)
{
  if(structured_text.empty()) {
    return false;
  }
  if(structured_text.find("_GLOBAL__N_") != std::string::npos) {
    return false;
  }
  if(witness_scope_operator_count(structured_text) >
     witness_scope_operator_count(source_text)) {
    return true;
  }
  return structured_text.find('<') != std::string::npos &&
         source_text.find('<') == std::string::npos;
}

std::string strip_witness_elaborated_type_prefix(const std::string & text)
{
  static const char * prefixes[] = {
      "enum class ",
      "enum struct ",
      "class ",
      "struct ",
      "union ",
      "enum "};
  for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const std::string prefix = prefixes[i];
    if(text.compare(0, prefix.size(), prefix) == 0) {
      return text.substr(prefix.size());
    }
  }
  return text;
}

std::string witness_non_side_effect_named_type_text(
    const cpp_decl::TypePtr & type,
    const std::string & source_text)
{
  if(!type || type->kind != cpp_decl::Type::TK_NAMED) {
    return std::string();
  }
  const std::string key_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(type->named_key));
  if(key_text.find("_GLOBAL__N_") != std::string::npos) {
    const std::string display_text =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(
                named_type_display_text(type)));
    if(!display_text.empty() &&
       display_text.find("_GLOBAL__N_") == std::string::npos &&
       witness_argument_text_should_prefer_structured(display_text,
                                                      source_text)) {
      return display_text;
    }
  }
  if(key_text.empty() ||
     key_text.find("_GLOBAL__N_") != std::string::npos ||
     key_text.find("__local_") != std::string::npos ||
     key_text.find('(') != std::string::npos ||
     key_text.find("template-parameter ") == 0 ||
     key_text.find("dependent ") == 0 ||
     key_text.find("builtin ") == 0) {
    return std::string();
  }
  if(witness_argument_text_should_prefer_structured(key_text, source_text)) {
    return key_text;
  }
  return std::string();
}

bool witness_type_argument_should_prefer_named_key(const cpp_decl::TypePtr & type)
{
  if(!type || type->kind != cpp_decl::Type::TK_NAMED) {
    return false;
  }
  const std::string display_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(
              named_type_display_text(type)));
  const std::string key_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(type->named_key));
  if(key_text.empty() ||
     key_text.find("_GLOBAL__N_") != std::string::npos ||
     key_text.find("template-parameter ") == 0 ||
     key_text.find("dependent ") == 0 ||
     key_text.find("builtin ") == 0) {
    return false;
  }
  return witness_scope_operator_count(key_text) >
      witness_scope_operator_count(display_text);
}

// template-boundary-audit: begin text_recovery_bridge
std::string witness_lookup_text_for_type_argument(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type)
{
  std::string local_type_text;
  if(function_local_type_argument_text(ctx, type, local_type_text)) {
    return semantic_utils::trim_space(local_type_text);
  }
  if(type &&
     type->kind == cpp_decl::Type::TK_CV &&
     type->inner &&
     ctx.class_info_for_type(type->inner)) {
    return default_elided_type_argument_text(ctx, type);
  }
  // Prefer the structured specialization metadata over display/source
  // spellings.  A typedef used as an outer template argument can otherwise
  // leave aliases from an inner specialization (for example K and D) in the
  // public binding even though the semantic type is fully concrete.
  if(std::shared_ptr<const cpp_decl::ClassTemplateSpecializationMangleInfo>
         mangle_info =
             cpp_decl::named_type_class_template_specialization_mangle_info_const(type)) {
    if(!mangle_info->template_name.empty()) {
      const std::string structured =
          class_template_mangle_info_witness_text(ctx, *mangle_info, 0);
      if(structured.find("_GLOBAL__N_") == std::string::npos) {
        return structured;
      }
      const std::string public_lookup =
          semantic_utils::trim_space(
              template_argument_semantics::lookup_text_for_type_argument(
                  ctx,
                  type));
      if(!public_lookup.empty() &&
         public_lookup.find("_GLOBAL__N_") == std::string::npos) {
        return public_lookup;
      }
      return structured;
    }
  }
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(type)) {
    const std::string default_elided_text =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(
                class_witness_output_qualified_name(ctx, *info)));
    if(!default_elided_text.empty() &&
       default_elided_text !=
           semantic_utils::trim_space(
               strip_witness_elaborated_type_prefix(
                   semantic_model::class_output_qualified_name(*info)))) {
      return default_elided_text;
    }
    const std::string display_text =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(
                semantic_model::class_output_qualified_name(*info)));
    const std::string named_display =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(
                named_type_display_text(type)));
    if(witness_text_is_owner_qualified_suffix(display_text, named_display)) {
      return display_text;
    }
  }
  if(witness_type_argument_should_prefer_named_key(type)) {
    return semantic_utils::trim_space(
        strip_witness_elaborated_type_prefix(type->named_key));
  }
  const std::string lookup_text =
      semantic_utils::trim_space(
          template_argument_semantics::lookup_text_for_type_argument(ctx, type));
  const std::string identity_text =
      semantic_utils::trim_space(
          strip_witness_elaborated_type_prefix(
              ctx.instantiation_identity_text_for_type_argument(type)));
  if(!identity_text.empty() &&
     witness_scope_operator_count(identity_text) >
         witness_scope_operator_count(lookup_text)) {
    return identity_text;
  }
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(type)) {
    const std::string semantic_text =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(info->qualified_name));
    if(!semantic_text.empty() &&
       witness_scope_operator_count(semantic_text) >
           witness_scope_operator_count(lookup_text)) {
      return semantic_text;
    }
  }
  return semantic_utils::trim_space(lookup_text);
}
// template-boundary-audit: end text_recovery_bridge

std::string template_witness_argument_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg)
{
  if(arg.kind == template_model::TemplateArgument::TA_VALUE) {
    const std::string member_pointer_text =
        member_pointer_witness_argument_text(ctx, arg);
    if(!member_pointer_text.empty()) {
      return member_pointer_text;
    }
  }
  if(arg.kind == template_model::TemplateArgument::TA_TYPE && arg.type) {
    const std::string structured_text =
        witness_lookup_text_for_type_argument(ctx, arg.type);
    if(cpp_decl::is_reference_type(arg.type) &&
       !ctx.type_depends_on_template_parameter(arg.type)) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          structured_text);
    }
    if(type_contains_default_elided_template_argument(ctx, arg.type, 0)) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          structured_text);
    }
    if(template_argument_contains_named_function_local_type_for_witness(ctx,
                                                                        arg)) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          structured_text);
    }
    if(arg.text.empty() ||
       witness_argument_text_should_prefer_structured(structured_text,
                                                      arg.text)) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          structured_text);
    }
    return normalize_witness_function_type_argument_text(arg.type, arg.text);
  }
  return template_model::template_argument_text(
      arg,
      [&ctx](const cpp_decl::TypePtr & type)
      {
        return witness_lookup_text_for_type_argument(ctx, type);
      });
}

std::string template_witness_type_argument_text(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & type)
{
  template_model::TemplateArgument type_arg;
  type_arg.kind = template_model::TemplateArgument::TA_TYPE;
  type_arg.type = type;
  return template_witness_argument_text(ctx, type_arg);
}

bool template_type_parameter_feeds_non_type_parameter(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t parameter_index)
{
  if(parameter_index >= parameters.size() ||
     parameters[parameter_index].kind != template_model::TemplateParameterInfo::TP_TYPE) {
    return false;
  }
  for(std::size_t i = parameter_index + 1; i < parameters.size(); ++i) {
    if(parameters[i].kind == template_model::TemplateParameterInfo::TP_NON_TYPE &&
       type_contains_template_parameter_placeholder(parameters[i].value_type,
                                                    parameters[parameter_index])) {
      return true;
    }
  }
  return false;
}

std::string unsigned_integral_witness_value_text(
    const template_model::TemplateArgument & arg)
{
  if(arg.kind != template_model::TemplateArgument::TA_VALUE ||
     arg.dependent ||
     !arg.type ||
     !cpp_decl::is_unsigned_integral_type(arg.type) ||
     cpp_decl::is_bool_type(arg.type)) {
    return std::string();
  }
  unsigned long long value = static_cast<unsigned long long>(arg.value);
  const std::size_t byte_count = cpp_decl::type_size(arg.type);
  const std::size_t bit_count = byte_count * CHAR_BIT;
  if(bit_count > 0 && bit_count < sizeof(unsigned long long) * CHAR_BIT) {
    value &= (1ULL << bit_count) - 1ULL;
  }
  std::ostringstream out;
  out << value;
  return out.str();
}

bool explicit_value_text_is_static_cast(const std::string & text)
{
  const std::string trimmed = semantic_utils::trim_space(text);
  return trimmed.compare(0, 12, "static_cast<") == 0;
}

std::string template_witness_value_binding_arg_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg,
    const std::string & explicit_text)
{
  const std::string trimmed_explicit =
      semantic_utils::trim_space(explicit_text);
  if(arg.kind == template_model::TemplateArgument::TA_VALUE &&
     !arg.dependent &&
     value_argument_has_named_enum_type(arg) &&
     is_qualified_identifier_value_text(trimmed_explicit)) {
    return trimmed_explicit;
  }
  const std::string enumerator_text =
      enum_witness_enumerator_text_for_value(ctx, arg);
  if(!enumerator_text.empty()) {
    return enumerator_text;
  }
  if(arg.kind == template_model::TemplateArgument::TA_VALUE &&
     arg.source_syntax) {
    const std::string source_text = semantic_utils::trim_space(
        callsemantic::template_argument_syntax_witness_source_text(
            *arg.source_syntax));
    if(!source_text.empty() &&
       (arg.source_syntax->expression ||
        !semantic_utils::trim_space(arg.source_syntax->source_text).empty()) &&
       (trimmed_explicit.empty() ||
        callsemantic_internal::remove_space_chars(source_text) !=
            callsemantic_internal::remove_space_chars(trimmed_explicit))) {
      return source_text;
    }
  }
  std::string value_text = unsigned_integral_witness_value_text(arg);
  if(value_text.empty()) {
    value_text = template_witness_argument_text(ctx, arg);
  }
  if(!arg.dependent && explicit_value_text_is_static_cast(trimmed_explicit)) {
    const std::string cast_type = template_witness_type_argument_text(ctx, arg.type);
    if(!cast_type.empty()) {
      return "(" + cast_type + ")" + value_text;
    }
  }
  return value_text;
}

bool type_is_array_with_cv_qualified_element(const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  while(base && base->kind == cpp_decl::Type::TK_ARRAY) {
    cpp_decl::TypePtr element = base->inner;
    if(element &&
       element->kind == cpp_decl::Type::TK_CV &&
       (element->cv_const || element->cv_volatile)) {
      return true;
    }
    base = cpp_decl::strip_top_level_cv(element);
  }
  return false;
}

bool source_text_drops_array_cv(const cpp_decl::TypePtr & type,
                                const std::string & source_text,
                                const std::string & semantic_text)
{
  if(!type_is_array_with_cv_qualified_element(type)) {
    return false;
  }
  const std::string source_key =
      callsemantic_internal::remove_space_chars(source_text);
  const std::string semantic_key =
      callsemantic_internal::remove_space_chars(semantic_text);
  return !semantic_key.empty() &&
         source_key != semantic_key &&
         ((semantic_key.find("const") != std::string::npos &&
           source_key.find("const") == std::string::npos) ||
          (semantic_key.find("volatile") != std::string::npos &&
           source_key.find("volatile") == std::string::npos));
}

bool explicit_type_argument_requires_structured_spelling(
    const cpp_decl::TypePtr & type)
{
  if(!type) {
    return false;
  }
  const cpp_decl::TypePtr unqualified = cpp_decl::strip_top_level_cv(type);
  if(unqualified && unqualified->kind == cpp_decl::Type::TK_ARRAY) {
    return true;
  }
  return type->kind == cpp_decl::Type::TK_CV &&
         type->cv_const &&
         type->cv_volatile &&
         unqualified &&
         unqualified->kind == cpp_decl::Type::TK_FUNDAMENTAL;
}

bool is_builtin_type_keyword_spelling(const std::string & text)
{
  return text == "bool" ||
         text == "char" ||
         text == "char16_t" ||
         text == "char32_t" ||
         text == "double" ||
         text == "float" ||
         text == "int" ||
         text == "long" ||
         text == "short" ||
         text == "signed" ||
         text == "unsigned" ||
         text == "void" ||
         text == "wchar_t";
}

bool explicit_type_argument_uses_typedef_spelling(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg,
    const std::string & source_text,
    std::string & resolved_text)
{
  resolved_text.clear();
  if(arg.kind != template_model::TemplateArgument::TA_TYPE ||
     !arg.type ||
     ctx.type_depends_on_template_parameter(arg.type)) {
    return false;
  }
  const std::string trimmed_source = semantic_utils::trim_space(source_text);
  std::string lookup_source = trimmed_source;
  bool stripped_cv = true;
  while(stripped_cv) {
    stripped_cv = false;
    if(lookup_source.compare(0, 6, "const ") == 0) {
      lookup_source = semantic_utils::trim_space(lookup_source.substr(6));
      stripped_cv = true;
    }
    if(lookup_source.compare(0, 9, "volatile ") == 0) {
      lookup_source = semantic_utils::trim_space(lookup_source.substr(9));
      stripped_cv = true;
    }
  }
  const bool source_is_identifier =
      callsemantic_internal::is_identifier_text(lookup_source);
  if(lookup_source.empty() ||
     is_builtin_type_keyword_spelling(lookup_source)) {
    return false;
  }
  resolved_text = witness_lookup_text_for_type_argument(ctx, arg.type);
  if(semantic_model::ClassInfo * info = ctx.class_info_for_type(arg.type)) {
    if(info->source_is_named_function_local_class &&
       source_is_identifier &&
       semantic_utils::unqualified_member_name(info->name) == lookup_source) {
      return false;
    }
    const std::string display_text =
        semantic_utils::trim_space(
            strip_witness_elaborated_type_prefix(
                class_witness_output_qualified_name(ctx, *info)));
    if(!display_text.empty() &&
       display_text.find("_GLOBAL__N_") == std::string::npos &&
       (resolved_text.empty() ||
        resolved_text == trimmed_source ||
        resolved_text.find("_GLOBAL__N_") != std::string::npos) &&
       witness_text_is_owner_qualified_suffix(display_text, trimmed_source)) {
      resolved_text = display_text;
    }
  }
  return !resolved_text.empty() &&
         resolved_text.find("_GLOBAL__N_") == std::string::npos &&
         resolved_text.find("__local_") == std::string::npos &&
         semantic_utils::trim_space(resolved_text) != trimmed_source;
}

std::string template_witness_source_binding_arg_text(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t parameter_index,
    const template_model::TemplateArgument & arg,
    const std::string & explicit_text,
    bool has_explicit_text)
{
  if(arg.kind == template_model::TemplateArgument::TA_VALUE) {
    return template_witness_value_binding_arg_text(ctx, arg, explicit_text);
  }
  if((arg.kind == template_model::TemplateArgument::TA_CLASS_TEMPLATE ||
      arg.kind == template_model::TemplateArgument::TA_ALIAS_TEMPLATE) &&
     arg.source_syntax) {
    // Clang's TemplateArgument::print renders a semantic TemplateName here.
    // In particular, the written `template` disambiguator is parsing syntax,
    // not part of the public template-name spelling.
    return template_witness_semantic_argument_text(ctx, arg);
  }
  if(has_explicit_text &&
     !template_type_parameter_feeds_non_type_parameter(parameters, parameter_index)) {
    if(arg.kind == template_model::TemplateArgument::TA_TYPE &&
       arg.type &&
       cpp_decl::is_reference_type(arg.type) &&
       !ctx.type_depends_on_template_parameter(arg.type)) {
      return witness_lookup_text_for_type_argument(ctx, arg.type);
    }
    const std::string source_text = semantic_utils::trim_space(explicit_text);
    std::string semantic_text;
    if(arg.kind == template_model::TemplateArgument::TA_TYPE && arg.type) {
      semantic_text =
          semantic_utils::trim_space(witness_lookup_text_for_type_argument(ctx,
                                                                           arg.type));
      if(source_text_drops_array_cv(arg.type, source_text, semantic_text)) {
        return semantic_text;
      }
    }
    std::string resolved_typedef_text;
    if(explicit_type_argument_uses_typedef_spelling(ctx,
                                                    arg,
                                                    source_text,
                                                    resolved_typedef_text)) {
      return resolved_typedef_text;
    }
    if(explicit_type_argument_requires_structured_spelling(arg.type) &&
       !semantic_text.empty()) {
      return normalize_witness_function_type_argument_text(arg.type,
                                                          semantic_text);
    }
    const std::string structured_text =
        witness_non_side_effect_named_type_text(arg.type, source_text);
    if(!structured_text.empty()) {
      return structured_text;
    }
    return normalize_witness_function_type_argument_text(arg.type,
                                                        source_text);
  }
  return template_witness_argument_text(ctx, arg);
}

std::string template_witness_defaulted_source_binding_arg_text(
    SemanticContext & ctx,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t parameter_index,
    const template_model::TemplateArgument & arg)
{
  if(arg.kind == template_model::TemplateArgument::TA_VALUE && !arg.dependent) {
    return template_witness_argument_text(ctx, arg);
  }
  return template_witness_source_binding_arg_text(ctx,
                                                 parameters,
                                                 parameter_index,
                                                 arg,
                                                 std::string(),
                                                 false);
}

std::string template_witness_node_text(SemanticContext & ctx,
                                       const CppAstNode & node)
{
  std::string text = semantic_utils::trim_space(cpp_decl::node_text(node));
  if(!text.empty()) {
    return text;
  }
  const TemplateWitnessContext witness_context = ctx.template_witness_context();
  if(witness_context.token_sequence &&
     cpp_decl::has_valid_node_span(*witness_context.token_sequence, node)) {
    text = semantic_utils::trim_space(
        callsemantic_internal::spaced_token_span_text(
            *witness_context.token_sequence,
            node.token_start,
            node.token_end));
    if(!text.empty()) {
      return text;
    }
  }
  return text;
}

std::string template_witness_substituted_default_text(
    SemanticContext & ctx,
    const std::string & default_text,
    const std::vector<template_model::TemplateParameterInfo> * all_parameters,
    const std::vector<template_model::TemplateArgument> * all_arguments,
    std::size_t parameter_index)
{
  std::string out = default_text;
  if(!all_parameters || !all_arguments) {
    return out;
  }
  const std::size_t limit =
      std::min(parameter_index,
               std::min(all_parameters->size(), all_arguments->size()));
  for(std::size_t i = 0; i < limit; ++i) {
    const std::string replacement =
        template_witness_argument_text(ctx, (*all_arguments)[i]);
    if(replacement.empty()) {
      continue;
    }
    const template_model::TemplateParameterInfo & parameter =
        (*all_parameters)[i];
    bool changed = false;
    if(!parameter.name.empty()) {
      out = callsemantic_internal::replace_identifier_token_text(
          out,
          parameter.name,
          replacement,
          changed);
    }
    for(std::size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(parameter.alternate_names[j].empty()) {
        continue;
      }
      bool alias_changed = false;
      out = callsemantic_internal::replace_identifier_token_text(
          out,
          parameter.alternate_names[j],
          replacement,
          alias_changed);
    }
  }
  return out;
}

std::string namespace_insensitive_template_text_key(const std::string & text)
{
  std::string out;
  const std::string value = semantic_utils::trim_space(text);
  for(std::size_t i = 0; i < value.size();) {
    if(std::isspace(static_cast<unsigned char>(value[i]))) {
      ++i;
      continue;
    }
    if(std::isalpha(static_cast<unsigned char>(value[i])) ||
       value[i] == '_') {
      const std::size_t begin = i;
      std::size_t end = i + 1;
      while(end < value.size()) {
        const char ch = value[end];
        if(std::isalnum(static_cast<unsigned char>(ch)) ||
           ch == '_' ||
           (ch == ':' && end + 1 < value.size() && value[end + 1] == ':')) {
          end += ch == ':' ? 2 : 1;
          continue;
        }
        break;
      }
      std::string name = value.substr(begin, end - begin);
      const std::size_t scope = name.rfind("::");
      if(scope != std::string::npos) {
        name = name.substr(scope + 2);
      }
      out += name;
      i = end;
      continue;
    }
    out += value[i++];
  }
  return out;
}

bool template_witness_text_matches_default(
    const template_model::TemplateParameterInfo & parameter,
    const std::string & text,
    const std::string & default_text)
{
  const std::string trimmed_text = semantic_utils::trim_space(text);
  const std::string trimmed_default = semantic_utils::trim_space(default_text);
  if(trimmed_text.empty() || trimmed_default.empty()) {
    return false;
  }
  if(trimmed_text == trimmed_default) {
    return true;
  }
  if(parameter.kind != template_model::TemplateParameterInfo::TP_TYPE &&
     parameter.kind !=
         template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
    return false;
  }
  return namespace_insensitive_template_text_key(trimmed_text) ==
      namespace_insensitive_template_text_key(trimmed_default);
}

std::string template_witness_explicit_binding_source(
    SemanticContext & ctx,
    const template_model::TemplateParameterInfo & parameter,
    const template_model::TemplateArgument & arg,
    const std::string & explicit_text,
    const std::string & explicit_source,
    const std::string & defaulted_source,
    const std::vector<template_model::TemplateParameterInfo> * all_parameters =
        nullptr,
    const std::vector<template_model::TemplateArgument> * all_arguments =
        nullptr,
    std::size_t parameter_index = 0,
    bool treat_explicit_defaults_as_defaulted = true)
{
  if(!treat_explicit_defaults_as_defaulted) {
    return explicit_source;
  }
  const std::string rendered_arg_text =
      semantic_utils::trim_space(template_witness_argument_text(ctx, arg));
  if(parameter.default_argument != nullptr) {
    const CppAstNode * default_payload =
        !parameter.default_argument->children.empty() ?
            &parameter.default_argument->children[0] :
            parameter.default_argument;
    const std::string default_text =
        template_witness_node_text(ctx, *default_payload);
    const std::string substituted_default_text =
        semantic_utils::trim_space(
            template_witness_substituted_default_text(ctx,
                                                      default_text,
                                                      all_parameters,
                                                      all_arguments,
                                                      parameter_index));
    if(!default_text.empty() &&
       (template_witness_text_matches_default(parameter,
                                              explicit_text,
                                              default_text) ||
        template_witness_text_matches_default(parameter,
                                              rendered_arg_text,
                                              default_text) ||
        template_witness_text_matches_default(parameter,
                                              explicit_text,
                                              substituted_default_text) ||
        template_witness_text_matches_default(parameter,
                                              rendered_arg_text,
                                              substituted_default_text))) {
      return defaulted_source;
    }
  }
  return explicit_source;
}

std::string template_witness_binding_param_name(
    const template_model::TemplateParameterInfo & parameter,
    std::size_t index)
{
  return parameter.name.empty() ?
      std::string("$") + std::to_string(index + 1) :
      parameter.name;
}

}  // namespace

std::string template_witness_source_argument_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & arg)
{
  return template_witness_argument_text(ctx, arg);
}

void append_template_witness_source_bindings(
    SemanticContext & ctx,
    std::vector<TemplateWitnessSourceBinding> & out,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::string & source,
    TemplateWitnessSourceBindingPolicy policy,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  std::size_t trailing_default_count = 0;
  if(policy ==
     TemplateWitnessSourceBindingPolicy::DeducedWithDefaultedTrailingDefaults) {
    for(std::size_t j = parameters.size(); j > 0; --j) {
      const template_model::TemplateParameterInfo & parameter =
          parameters[j - 1];
      if(parameter.parameter_pack || !parameter.default_argument) {
        break;
      }
      ++trailing_default_count;
    }
  }
  const std::size_t required_count =
      parameters.size() >= trailing_default_count ?
          parameters.size() - trailing_default_count :
          0;
  const auto binding_source_for_parameter =
      [&](const template_model::TemplateParameterInfo & parameter,
          std::size_t arg_index_for_parameter) -> std::string
  {
    if(policy ==
           TemplateWitnessSourceBindingPolicy::
               DeducedWithDefaultedTrailingDefaults &&
       !parameter.parameter_pack &&
       parameter.default_argument &&
       arg_index_for_parameter >= required_count) {
      return "defaulted";
    }
    return source;
  };
  std::size_t arg_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    TemplateWitnessSourceBinding binding;
    binding.param = template_witness_binding_param_name(parameters[i], i);
    binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(parameters[i]);
    binding.source = binding_source_for_parameter(parameters[i], arg_index);
    if(parameters[i].parameter_pack) {
      binding.pack_binding = true;
      std::size_t selected_pack_size = 0;
      bool has_selected_pack_size = false;
      if(pack_sizes) {
        if(!parameters[i].name.empty()) {
          std::map<std::string, std::size_t>::const_iterator named_size =
              pack_sizes->find(parameters[i].name);
          if(named_size != pack_sizes->end()) {
            selected_pack_size = named_size->second;
            has_selected_pack_size = true;
          }
        }
        if(!has_selected_pack_size &&
           !parameters[i].placeholder_key.empty()) {
          std::map<std::string, std::size_t>::const_iterator placeholder_size =
              pack_sizes->find(parameters[i].placeholder_key);
          if(placeholder_size != pack_sizes->end()) {
            selected_pack_size = placeholder_size->second;
            has_selected_pack_size = true;
          }
        }
      }
      std::size_t pack_end = arg_index;
      if(has_selected_pack_size) {
        if(selected_pack_size > arguments.size() - arg_index) {
          break;
        }
        pack_end += selected_pack_size;
      } else {
        std::size_t trailing_non_pack = 0;
        for(std::size_t j = i + 1; j < parameters.size(); ++j) {
          if(!parameters[j].parameter_pack) {
            ++trailing_non_pack;
          }
        }
        if(arguments.size() < arg_index + trailing_non_pack) {
          break;
        }
        pack_end = arguments.size() - trailing_non_pack;
      }
      binding.type_like =
          template_witness_argument_range_is_type_like(arguments,
                                                      arg_index,
                                                      pack_end);
      std::ostringstream pack_text;
      pack_text << "<";
      for(std::size_t j = arg_index; j < pack_end; ++j) {
        if(j != arg_index) {
          pack_text << ", ";
        }
        const std::string element_text =
            template_witness_argument_text(ctx, arguments[j]);
        binding.pack_arguments.push_back(element_text);
        pack_text << element_text;
      }
      pack_text << ">";
      binding.arg = pack_text.str();
      binding.pack_aggregate = pack_end > arg_index + 1;
      out.push_back(binding);
      arg_index = pack_end;
      continue;
    }
    if(arg_index >= arguments.size()) {
      break;
    }
    binding.type_like = template_witness_argument_is_type_like(
        arguments[arg_index]);
    binding.arg = template_witness_argument_text(ctx, arguments[arg_index]);
    out.push_back(binding);
    ++arg_index;
  }
}

void append_template_witness_source_bindings(
    SemanticContext & ctx,
    std::vector<TemplateWitnessSourceBinding> & out,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts,
    const std::string & explicit_source,
    const std::string & defaulted_source,
    bool treat_explicit_defaults_as_defaulted)
{
  std::size_t arg_index = 0;
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    TemplateWitnessSourceBinding binding;
    binding.param = template_witness_binding_param_name(parameters[i], i);
    binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(parameters[i]);
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
      binding.type_like =
          template_witness_argument_range_is_type_like(arguments,
                                                      arg_index,
                                                      pack_end);
      const std::size_t remaining_explicit =
          explicit_argument_texts.size() > explicit_index ?
              explicit_argument_texts.size() - explicit_index :
              0;
      const std::size_t explicit_pack_count =
          std::min(pack_count,
                   remaining_explicit > trailing_non_pack ?
                       remaining_explicit - trailing_non_pack :
                       0);
      std::ostringstream pack_text;
      pack_text << "<";
      bool any_explicit = false;
      for(std::size_t j = 0; j < pack_count; ++j) {
        if(j != 0) {
          pack_text << ", ";
        }
        std::string element_text;
        if(j < explicit_pack_count) {
          const template_model::TemplateArgument & explicit_argument =
              arguments[arg_index + j];
          const std::string element_source =
              template_witness_explicit_binding_source(
                  ctx,
                  parameters[i],
                  explicit_argument,
                  explicit_argument_texts[explicit_index + j],
                  explicit_source,
                  defaulted_source,
                  &parameters,
                  &arguments,
                  i,
                  treat_explicit_defaults_as_defaulted);
          any_explicit = any_explicit || element_source == explicit_source;
          element_text = template_witness_source_binding_arg_text(
              ctx,
              parameters,
              i,
              explicit_argument,
              explicit_argument_texts[explicit_index + j],
              true);
        } else {
          element_text = template_witness_defaulted_source_binding_arg_text(
              ctx,
              parameters,
              i,
              arguments[arg_index + j]);
        }
        binding.pack_arguments.push_back(element_text);
        pack_text << element_text;
      }
      binding.source = pack_count == 0 ? "deduced" :
          (any_explicit ? explicit_source : defaulted_source);
      pack_text << ">";
      binding.arg = pack_text.str();
      binding.pack_aggregate = pack_count > 1;
      out.push_back(binding);
      arg_index = pack_end;
      explicit_index += explicit_pack_count;
      continue;
    }
    if(arg_index >= arguments.size()) {
      break;
    }
    binding.type_like = template_witness_argument_is_type_like(
        arguments[arg_index]);
    binding.function_type_argument =
        arguments[arg_index].kind == template_model::TemplateArgument::TA_TYPE &&
        arguments[arg_index].type &&
        cpp_decl::strip_top_level_cv(arguments[arg_index].type)->kind ==
            cpp_decl::Type::TK_FUNCTION;
    binding.structured_type_spelling =
        arguments[arg_index].kind == template_model::TemplateArgument::TA_TYPE &&
        explicit_type_argument_requires_structured_spelling(
            arguments[arg_index].type);
    if(explicit_index < explicit_argument_texts.size()) {
      binding.source =
          template_witness_explicit_binding_source(ctx,
                                                   parameters[i],
                                                   arguments[arg_index],
                                                   explicit_argument_texts[explicit_index],
                                                   explicit_source,
                                                   defaulted_source,
                                                   &parameters,
                                                   &arguments,
                                                   i,
                                                   treat_explicit_defaults_as_defaulted);
      binding.arg =
          template_witness_source_binding_arg_text(
              ctx,
              parameters,
              i,
              arguments[arg_index],
              explicit_argument_texts[explicit_index],
              true);
      ++explicit_index;
    } else {
      binding.arg =
          template_witness_defaulted_source_binding_arg_text(ctx,
                                                             parameters,
                                                             i,
                                                             arguments[arg_index]);
      binding.source = defaulted_source;
    }
    out.push_back(binding);
    ++arg_index;
  }
}

namespace {

std::map<std::string, std::string>
injected_class_type_name_replacements(
    SemanticContext & ctx,
    const semantic_model::Scope * source_scope,
    const semantic_model::ClassTemplateDecl *
        lexical_source_class_template,
    const std::vector<cpp_decl::TemplateArgumentSyntax> *
        lexical_source_class_arguments)
{
  std::map<std::string, std::string> out;
  if(lexical_source_class_template &&
     !lexical_source_class_template->name.empty()) {
    const std::string & name = lexical_source_class_template->name;
    bool shadowed = false;
    for(const semantic_model::Scope * lookup = source_scope;
        lookup;
        lookup = lookup->parent) {
      semantic_model::Scope::NamedTypeMap::const_iterator named_type =
          lookup->named_types.find(name);
      if(named_type != lookup->named_types.end()) {
        const semantic_model::ClassInfo * info =
            ctx.class_info_for_type(named_type->second);
        shadowed = !info ||
            info->source_template != lexical_source_class_template;
        break;
      }
      const auto class_template = lookup->class_templates.find(name);
      if(class_template != lookup->class_templates.end()) {
        shadowed = class_template->second != lexical_source_class_template;
        break;
      }
    }
    if(!shadowed) {
      std::ostringstream text;
      text << name << "<";
      if(lexical_source_class_arguments) {
        for(std::size_t i = 0;
            i < lexical_source_class_arguments->size();
            ++i) {
          if(i != 0) {
            text << ", ";
          }
          text << callsemantic_internal::render_template_argument_syntax(
              (*lexical_source_class_arguments)[i]);
        }
      } else {
        const std::vector<template_model::TemplateParameterInfo> & parameters =
            lexical_source_class_template->parameters;
        for(std::size_t i = 0; i < parameters.size(); ++i) {
          if(i != 0) {
            text << ", ";
          }
          const template_model::TemplateParameterInfo & parameter =
              parameters[i];
          if(!parameter.name.empty()) {
            text << parameter.name;
          } else if(!parameter.placeholder_key.empty()) {
            text << parameter.placeholder_key;
          } else {
            text << "template-parameter-0-" << i;
          }
          if(parameter.parameter_pack) {
            text << "...";
          }
        }
      }
      text << ">";
      out[name] = text.str();
    }
  }

  std::set<const semantic_model::ClassInfo *> seen;
  for(const semantic_model::Scope * current = source_scope;
      current;
      current = current->parent) {
    const semantic_model::ClassInfo * info = current->class_info;
    if(!info ||
       !seen.insert(info).second ||
       !info->source_template ||
       info->source_template->parameters.empty()) {
      continue;
    }
    const std::string name = !info->source_template->name.empty() ?
        info->source_template->name : info->name;
    if(name.empty() || out.count(name) != 0) {
      continue;
    }

    bool shadowed = false;
    for(const semantic_model::Scope * lookup = source_scope;
        lookup;
        lookup = lookup->parent) {
      semantic_model::Scope::NamedTypeMap::const_iterator found =
          lookup->named_types.find(name);
      if(found != lookup->named_types.end()) {
        shadowed = ctx.class_info_for_type(found->second) != info;
        break;
      }
      if(lookup == current) {
        break;
      }
    }
    if(shadowed) {
      continue;
    }

    std::ostringstream text;
    text << name << "<";
    const std::vector<template_model::TemplateParameterInfo> & parameters =
        info->source_template->parameters;
    for(std::size_t i = 0; i < parameters.size(); ++i) {
      if(i != 0) {
        text << ", ";
      }
      const template_model::TemplateParameterInfo & parameter = parameters[i];
      if(!parameter.name.empty()) {
        text << parameter.name;
      } else if(!parameter.placeholder_key.empty()) {
        text << parameter.placeholder_key;
      } else {
        text << "template-parameter-0-" << i;
      }
      if(parameter.parameter_pack) {
        text << "...";
      }
    }
    text << ">";
    out[name] = text.str();
  }
  return out;
}

std::string template_witness_semantic_argument_text_with_replacements(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument,
    const std::map<std::string, std::string> & type_name_replacements,
    bool preserve_template_name_source_disambiguator)
{
  std::string text;
  if(argument.source_syntax) {
    if(argument.kind == template_model::TemplateArgument::TA_VALUE &&
       argument.source_syntax->expression) {
      text = callsemantic_internal::render_template_argument_expression(
          *argument.source_syntax->expression,
          type_name_replacements);
    } else {
      text = callsemantic_internal::render_template_argument_syntax(
          *argument.source_syntax,
          type_name_replacements,
          preserve_template_name_source_disambiguator ||
              (argument.kind !=
                   template_model::TemplateArgument::TA_CLASS_TEMPLATE &&
               argument.kind !=
                   template_model::TemplateArgument::TA_ALIAS_TEMPLATE));
    }
  } else if(argument.kind == template_model::TemplateArgument::TA_VALUE &&
            argument.expression) {
    text = callsemantic_internal::render_template_argument_expression(
        *argument.expression);
  } else {
    text = template_witness_argument_text(ctx, argument);
  }
  text = semantic_utils::trim_space(text);
  if(argument.source_syntax &&
     argument.source_syntax->pack_expansion &&
     (text.size() < 3 || text.compare(text.size() - 3, 3, "...") != 0)) {
    text += "...";
  }
  return text;
}

}  // namespace

std::string template_witness_semantic_argument_text(
    SemanticContext & ctx,
    const template_model::TemplateArgument & argument,
    const semantic_model::Scope * source_scope)
{
  const std::map<std::string, std::string> type_name_replacements =
      injected_class_type_name_replacements(ctx,
                                            source_scope,
                                            nullptr,
                                            nullptr);
  return template_witness_semantic_argument_text_with_replacements(
      ctx, argument, type_name_replacements, false);
}

void append_alias_template_witness_source_bindings(
    SemanticContext & ctx,
    std::vector<TemplateWitnessSourceBinding> & out,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> &
        source_occurrence_arguments,
    const semantic_model::Scope & source_scope,
    const semantic_model::ClassTemplateDecl * lexical_source_class_template,
    const std::vector<cpp_decl::TemplateArgumentSyntax> *
        lexical_source_class_arguments)
{
  const std::map<std::string, std::string> type_name_replacements =
      injected_class_type_name_replacements(
          ctx,
          &source_scope,
          lexical_source_class_template,
          lexical_source_class_arguments);
  const std::size_t count =
      std::min(parameters.size(), source_occurrence_arguments.size());
  for(std::size_t i = 0; i < count; ++i) {
    TemplateWitnessSourceBinding binding;
    binding.param = template_witness_binding_param_name(parameters[i], i);
    binding.type_like = template_witness_argument_is_type_like(
        source_occurrence_arguments[i]);
    binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(parameters[i]);
    binding.pack_binding = parameters[i].parameter_pack;
    binding.arg = template_witness_semantic_argument_text_with_replacements(
        ctx,
        source_occurrence_arguments[i],
        type_name_replacements,
        true);
    binding.source = "explicit";
    out.push_back(binding);
  }
}

NonTypeArgumentStatus evaluate_non_type_argument_expression(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    std::string * eval_error,
    const cpp_decl::TypePtr & target_type)
{
  return to_api_non_type_argument_status(
      template_argument_semantics::evaluate_non_type_argument_expression(
          services, scope, expr, value, eval_error, target_type));
}

// template-boundary-audit: begin text_recovery_bridge
std::string lookup_text_for_type_argument(SemanticContext & ctx,
                                          const cpp_decl::TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(ctx, type);
}

bool substitute_type(const cpp_decl::TypePtr & type,
                     const std::vector<template_model::TemplateParameterInfo> & parameters,
                     const std::vector<template_model::TemplateArgument> & arguments,
                     cpp_decl::TypePtr & out)
{
  return template_argument_semantics::substitute_type(type, parameters, arguments, out);
}

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out)
{
  return template_argument_semantics::resolve_instantiated_dependent_type(
      ctx, scope, type, out);
}

// template-boundary-audit: end text_recovery_bridge

bool resolve_non_type_template_parameter_type(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return resolve_non_type_template_parameter_type(
            services, make_template_environment(scope), parameter, out);
      });
}

bool resolve_non_type_template_parameter_type(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const template_model::TemplateParameterInfo & parameter,
    cpp_decl::TypePtr & out)
{
  return template_resolution::resolve_non_type_template_parameter_type(
      services, scope, parameter, out);
}

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return resolve_template_argument(
            services, argument_scope, parameter_scope, parameter, text, syntax, out);
      });
}

bool resolve_template_argument(SemanticContext & ctx,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out)
{
  return resolve_template_argument(
      ctx, argument_scope, parameter_scope, parameter, text, nullptr, out);
}

bool resolve_template_argument(TemplateServices & services,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out)
{
  return resolve_template_argument(services,
                                   make_template_environment(argument_scope),
                                   make_template_environment(parameter_scope),
                                   parameter,
                                   text,
                                   syntax,
                                   out);
}

bool resolve_template_argument(TemplateServices & services,
                               semantic_model::Scope & argument_scope,
                               semantic_model::Scope & parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out)
{
  return resolve_template_argument(
      services, argument_scope, parameter_scope, parameter, text, nullptr, out);
}

bool resolve_template_argument(TemplateServices & services,
                               TemplateEnvironmentHandle argument_scope,
                               TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               const cpp_decl::TemplateArgumentSyntax * syntax,
                               template_model::TemplateArgument & out)
{
  return template_resolution::resolve_template_argument(
      services, argument_scope, parameter_scope, parameter, text, syntax, out);
}

bool resolve_template_argument(TemplateServices & services,
                               TemplateEnvironmentHandle argument_scope,
                               TemplateEnvironmentHandle parameter_scope,
                               const template_model::TemplateParameterInfo & parameter,
                               const std::string & text,
                               template_model::TemplateArgument & out)
{
  return resolve_template_argument(
      services, argument_scope, parameter_scope, parameter, text, nullptr, out);
}

bool resolve_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return resolve_template_arguments(
            services, scope, parameters, texts, syntaxes, out,
            default_argument_declaring_scope);
      });
}

bool resolve_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope)
{
  return resolve_template_arguments(
      ctx, scope, parameters, texts, nullptr, out, default_argument_declaring_scope);
}

bool resolve_template_arguments(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope)
{
  return resolve_template_arguments(services,
                                    make_template_environment(scope),
                                    parameters,
                                    texts,
                                    syntaxes,
                                    out,
                                    default_argument_declaring_scope ?
                                        make_template_environment(*default_argument_declaring_scope) :
                                        TemplateEnvironmentHandle());
}

bool resolve_template_arguments(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * default_argument_declaring_scope)
{
  return resolve_template_arguments(
      services, scope, parameters, texts, nullptr, out, default_argument_declaring_scope);
}

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope)
{
  return template_resolution::resolve_template_arguments(
      services, scope, parameters, texts, syntaxes, out, default_argument_declaring_scope);
}

bool resolve_template_arguments_for_candidate(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope)
{
  try {
    return template_resolution::resolve_template_arguments(
        services,
        scope,
        parameters,
        texts,
        syntaxes,
        out,
        default_argument_declaring_scope);
  } catch(const TemplateSubstitutionFailure &) {
    out.clear();
    return false;
  }
}

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope)
{
  return resolve_template_arguments(
      services, scope, parameters, texts, nullptr, out, default_argument_declaring_scope);
}

bool trailing_pack_accepts_argument_count(
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    std::size_t argument_count)
{
  return template_resolution::trailing_pack_accepts_argument_count(parameters, argument_count);
}

bool deduce_template_argument(SemanticContext & ctx,
                              const std::vector<template_model::TemplateParameterInfo> & parameters,
                              const cpp_decl::TypePtr & pattern,
                              const cpp_decl::TypePtr & actual,
                              std::map<std::string, cpp_decl::TypePtr> & deduced,
                              semantic_model::Scope * deduction_scope,
                              bool partial_top_level_cv_deduction,
                              semantic_model::Scope * actual_lookup_scope,
                              bool allow_actual_base_deduction)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return deduce_template_argument(
            services,
            parameters,
            pattern,
            actual,
            deduced,
            deduction_scope ? make_template_environment(*deduction_scope) :
                              TemplateEnvironmentHandle(),
            partial_top_level_cv_deduction,
            actual_lookup_scope ? make_template_environment(*actual_lookup_scope) :
                                  TemplateEnvironmentHandle(),
            allow_actual_base_deduction);
      });
}

bool deduce_template_argument(TemplateServices & services,
                              const std::vector<template_model::TemplateParameterInfo> & parameters,
                              const cpp_decl::TypePtr & pattern,
                              const cpp_decl::TypePtr & actual,
                              std::map<std::string, cpp_decl::TypePtr> & deduced,
                              TemplateEnvironmentHandle deduction_scope,
                              bool partial_top_level_cv_deduction,
                              TemplateEnvironmentHandle actual_lookup_scope,
                              bool allow_actual_base_deduction)
{
  return template_resolution::deduce_template_argument(services,
                                                       parameters,
                                                       pattern,
                                                       actual,
                                                       deduced,
                                                       deduction_scope,
                                                       partial_top_level_cv_deduction,
                                                       actual_lookup_scope,
                                                       allow_actual_base_deduction);
}

bool deduce_function_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  return template_resolution::deduce_function_template_arguments(
      ctx, decl, args, out, use_scope, pack_sizes_out);
}

bool deduce_function_template_arguments_from_target_type(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    semantic_model::Scope * use_scope,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  return template_resolution::deduce_function_template_arguments_from_target_type(
      ctx, decl, target, out, use_scope, pack_sizes_out);
}

bool deduce_function_template_arguments_from_target_type_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const cpp_decl::TypePtr & target,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  return template_resolution::
      deduce_function_template_arguments_from_target_type_with_explicit(
          ctx,
          decl,
          resolution_scope,
          explicit_arguments,
          target,
          out,
          pack_sizes_out);
}

bool resolve_function_explicit_template_arguments(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<template_model::TemplateArgument> & out,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * explicit_arg_syntaxes)
{
  return template_resolution::resolve_function_explicit_template_arguments(
      ctx, decl, resolution_scope, explicit_arg_texts, out, explicit_arg_syntaxes);
}

bool deduce_function_template_arguments_with_explicit(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    semantic_model::Scope & resolution_scope,
    const std::vector<template_model::TemplateArgument> & explicit_arguments,
    const std::vector<semantic_conversion::ExprInfo> & args,
    std::vector<template_model::TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  return template_resolution::deduce_function_template_arguments_with_explicit(
      ctx, decl, resolution_scope, explicit_arguments, args, out, pack_sizes_out);
}

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    semantic_model::FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count)
{
  return template_resolution::explicit_function_template_arguments_determine_signature(
      ctx, decl, explicit_argument_count);
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
  with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        overlay_instantiation_local_named_types(
            services, target, use_scope, declaring_scope, arguments, excluded_names);
      });
}

void overlay_instantiation_local_named_types(
    TemplateServices & services,
    semantic_model::Scope & target,
    const semantic_model::Scope & use_scope,
    const semantic_model::Scope * declaring_scope,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::set<std::string> * excluded_names)
{
  template_instantiation::overlay_instantiation_local_named_types(
      services, target, use_scope, declaring_scope, arguments, excluded_names);
}

void bind_template_arguments_into_scope(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        bind_template_arguments_into_scope(services, scope, parameters, arguments, pack_sizes);
      });
}

void bind_template_arguments_into_scope(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  template_instantiation::bind_template_arguments_into_scope(
      services, scope, parameters, arguments, pack_sizes);
}

semantic_model::Scope & bind_template_arguments(
    SemanticContext & ctx,
    semantic_model::Scope & declaring_scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes)
{
  return template_instantiation::bind_template_arguments(
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
  return template_instantiation::bind_class_template_arguments_for_instantiation(
      ctx, declaring_scope, use_scope, parameters, arguments, pack_sizes);
}

TemplateInstantiationResult acquire_function_instantiation(
    SemanticContext & ctx,
    const TemplateFunctionInstantiationRequest & request)
{
  TemplateInstantiationResult result;
  result.intent = request.intent;
  const std::size_t size_before =
      request.decl ? request.decl->instantiations.size() : 0;
  result.function_binding = template_instantiation::instantiate_function_template(
      ctx,
      *request.decl,
      request.arguments,
      request.active_owner,
      request.body_override,
      request.definition_node_override,
      request.explicit_specialization,
      request.explicit_specialization_is_constexpr,
      request.include_body,
      request.use_scope.valid() ? &request.use_scope.require() : nullptr,
      request.has_pack_sizes ? &request.pack_sizes : nullptr,
      request.prefer_overload_suffix,
      request.instantiation_use_location);
  result.created_new_binding =
      result.function_binding &&
      request.decl &&
      request.decl->instantiations.size() > size_before;
  result.definition_materialized =
      result.function_binding && result.function_binding->has_definition;
  if(closure_template_log_enabled(ctx) && result.function_binding) {
    result.lifecycle_transition.transition_kind =
        TemplateLifecycleTransitionKind::Instantiated;
    result.lifecycle_transition.function_binding = result.function_binding;
    result.lifecycle_transition.cause = request.explicit_specialization ?
        TemplateLifecycleCause::ExplicitSpecialization :
        lifecycle_cause_for_current_context_or_intent(request.intent);
    result.lifecycle_transition.created_new = result.created_new_binding;
    result.lifecycle_transition.definition_materialized =
        result.definition_materialized;
    result.lifecycle_transition.include_definition_materialized_detail = true;
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  apply_function_instantiation_intent(
      ctx, result.function_binding, request.intent, &result);
  return result;
}

TemplateInstantiationResult acquire_class_instantiation(
    SemanticContext & ctx,
    const TemplateClassInstantiationRequest & request)
{
  TemplateInstantiationResult result;
  const std::size_t size_before =
      request.decl ? request.decl->instantiations.size() : 0;
  if(!request.decl || !request.use_scope.valid()) {
    return result;
  }

  result.class_info = template_instantiation::instantiate_class_template(
      ctx, *request.decl, request.use_scope.require(), request.arguments);
  result.created_new_class =
      result.class_info &&
      request.decl->instantiations.size() > size_before;
  if(closure_template_log_enabled(ctx) && result.class_info) {
    result.lifecycle_transition.transition_kind =
        TemplateLifecycleTransitionKind::Instantiated;
    result.lifecycle_transition.class_info = result.class_info;
    result.lifecycle_transition.cause =
        lifecycle_cause_for_current_context_or_default(
            TemplateLifecycleCause::ImplicitUse);
    result.lifecycle_transition.created_new = result.created_new_class;
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  return result;
}

TemplateInstantiationResult acquire_selected_class_instantiation(
    SemanticContext & ctx,
    const TemplateSelectedClassInstantiationRequest & request)
{
  TemplateInstantiationResult result;
  const std::size_t size_before =
      request.decl ? request.decl->instantiations.size() : 0;
  if(!request.decl || !request.use_scope.valid()) {
    return result;
  }

  result.class_info = template_instantiation::instantiate_selected_class_template(
      ctx,
      *request.decl,
      request.use_scope.require(),
      request.arguments,
      request.specialization);
  result.created_new_class =
      result.class_info &&
      request.decl->instantiations.size() > size_before;
  if(closure_template_log_enabled(ctx) && result.class_info) {
    result.lifecycle_transition.transition_kind =
        TemplateLifecycleTransitionKind::Instantiated;
    result.lifecycle_transition.class_info = result.class_info;
    result.lifecycle_transition.cause =
        result.class_info->is_explicit_specialization ?
            TemplateLifecycleCause::ExplicitSpecialization :
            lifecycle_cause_for_current_context_or_default(
                TemplateLifecycleCause::ImplicitUse);
    result.lifecycle_transition.created_new = result.created_new_class;
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  return result;
}

TemplateInstantiationResult finalize_class_instantiation(
    SemanticContext & ctx,
    const TemplateClassFinalizationRequest & request)
{
  TemplateInstantiationResult result;
  result.class_info = request.info;
  if(!request.decl) {
    return result;
  }

  if(request.info) {
    const bool was_complete = request.info->complete;
    template_instantiation::finalize_instantiated_class(
        ctx, *request.decl, *request.info, request.arguments);
    result.class_finalized = request.info->complete && !was_complete;
    if(closure_template_log_enabled(ctx)) {
      result.lifecycle_transition.transition_kind =
          TemplateLifecycleTransitionKind::Finalized;
      result.lifecycle_transition.class_info = result.class_info;
      result.lifecycle_transition.cause =
          lifecycle_cause_for_current_context_or_default(
              TemplateLifecycleCause::FinalizeClass);
      result.lifecycle_transition.class_finalized = result.class_finalized;
      observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
    }
  }
  if(request.explicit_instantiation_node &&
     request.explicit_instantiation_cause != TemplateLifecycleCause::None) {
    TemplateLifecycleTransition explicit_transition;
    explicit_transition.transition_kind =
        TemplateLifecycleTransitionKind::Finalized;
    explicit_transition.class_template = request.decl;
    explicit_transition.source_use_node = request.explicit_instantiation_node;
    explicit_transition.cause = request.explicit_instantiation_cause;
    observe_template_lifecycle_transition(ctx, explicit_transition);
  }
  return result;
}

TemplateInstantiationResult finalize_nested_member_class_instantiation(
    SemanticContext & ctx,
    const TemplateNestedMemberClassFinalizationRequest & request)
{
  TemplateInstantiationResult result;
  result.class_info = request.nested_info;
  if(!request.owner_decl || !request.nested_info) {
    return result;
  }

  const bool was_complete = request.nested_info->complete;
  result.lifecycle_transition =
      template_instantiation::finalize_nested_member_class_instantiation(
      ctx,
      *request.owner_decl,
      *request.nested_info,
      request.owner_arguments,
      request.emit_track_instantiation);
  result.class_finalized = request.nested_info->complete && !was_complete;
  observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  return result;
}

bool prepare_nested_member_class_reference_from_owner_definition(
    SemanticContext & ctx,
    semantic_model::ClassInfo * nested,
    std::size_t incoming_template_parameter_count,
    std::size_t & owner_template_parameter_count)
{
  owner_template_parameter_count = 0;
  if(!(nested &&
       !nested->source_template &&
       nested->enclosing_scope &&
       nested->enclosing_scope->class_info &&
       nested->enclosing_scope->class_info->source_template)) {
    return false;
  }

  semantic_model::ClassInfo * owner = nested->enclosing_scope->class_info;
  if(owner->is_explicit_specialization) {
    return false;
  }
  semantic_model::ClassTemplateDecl * owner_template = owner->source_template;
  const semantic_model::PartialClassTemplateSpecializationDecl * partial =
      selected_partial_log_decl(owner);
  const std::map<std::string, semantic_model::OutOfClassMemberClassDecl> &
      definitions = partial ? partial->member_class_definitions :
                              owner_template->member_class_definitions;
  std::map<std::string, semantic_model::OutOfClassMemberClassDecl>::const_iterator
      member_def = definitions.find(nested->name);
  if(member_def == definitions.end() ||
     !member_def->second.class_node ||
     member_def->second.class_node->kind == CppAstKind::class_forward_declaration ||
     incoming_template_parameter_count <= member_def->second.parameters.size()) {
    return false;
  }
  owner_template_parameter_count = member_def->second.parameters.size();

  if(!nested->class_node ||
     nested->class_node->kind == CppAstKind::class_forward_declaration) {
    semantic_class_model::invalidate_forward_class_reference_members(ctx, *nested);
    nested->class_node = member_def->second.class_node;
    nested->is_final = member_def->second.class_node->is_final_specifier;
  }
  const std::vector<template_model::TemplateArgument> & owner_arguments =
      class_instantiation_binding_arguments(*owner);
  if(nested->member_scope &&
     owner->has_instantiation_binding_arguments &&
     template_model::template_arguments_fully_bind_parameters(
         member_def->second.parameters,
         owner_arguments)) {
    template_api::binding::bind_template_arguments_into_scope(
        ctx,
        *nested->member_scope,
        member_def->second.parameters,
        owner_arguments);
  }
  if(!nested->complete) {
    ctx.ensure_class_reference_members(*nested);
  }
  return true;
}

TemplateNestedMemberClassCompletionResult complete_nested_member_class_from_owner_definition(
    SemanticContext & ctx,
    const TemplateNestedMemberClassCompletionRequest & request)
{
  TemplateNestedMemberClassCompletionResult result;
  result.nested_info = request.nested_info;
  semantic_model::ClassInfo * nested = request.nested_info;
  if(!(nested &&
       !nested->source_template &&
       nested->enclosing_scope &&
       nested->enclosing_scope->class_info &&
       nested->enclosing_scope->class_info->source_template)) {
    return result;
  }

  semantic_model::ClassTemplateDecl * owner_template =
      nested->enclosing_scope->class_info->source_template;
  if(nested->enclosing_scope->class_info->is_explicit_specialization) {
    return result;
  }
  const semantic_model::PartialClassTemplateSpecializationDecl * partial =
      selected_partial_log_decl(nested->enclosing_scope->class_info);
  const std::map<std::string, semantic_model::OutOfClassMemberClassDecl> &
      definitions = partial ? partial->member_class_definitions :
                              owner_template->member_class_definitions;
  std::map<std::string, semantic_model::OutOfClassMemberClassDecl>::const_iterator
      member_def = definitions.find(nested->name);
  if(member_def == definitions.end() ||
     !member_def->second.class_node ||
     member_def->second.class_node->kind == CppAstKind::class_forward_declaration) {
    return result;
  }

  result.attempted = true;
  const CppAstNode * nested_decl_node = nested->class_node;
  if(!nested->class_node ||
     nested->class_node->kind == CppAstKind::class_forward_declaration) {
    nested->class_node = member_def->second.class_node;
    nested->is_final = member_def->second.class_node->is_final_specifier;
  }
  semantic_model::ClassInfo * owner = nested->enclosing_scope->class_info;
  const std::vector<template_model::TemplateArgument> * owner_arguments =
      owner ? &class_instantiation_binding_arguments(*owner) : nullptr;
  if(!nested->complete &&
     nested->member_scope &&
     owner &&
     owner->has_instantiation_binding_arguments &&
     owner_arguments &&
     template_model::template_arguments_fully_bind_parameters(
         member_def->second.parameters,
         *owner_arguments)) {
    template_api::binding::bind_template_arguments_into_scope(
        ctx,
        *nested->member_scope,
        member_def->second.parameters,
        *owner_arguments);
  }
  if(!nested->complete) {
    ctx.populate_class_info(*nested, *member_def->second.class_node);
  }
  result.completed = nested->complete;
  if(result.completed) {
    result.lifecycle_transition =
        template_api::note_nested_member_class_instantiation_completed_if_needed(
        ctx,
        nested,
        nested_decl_node,
        member_def->second.class_node);
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  return result;
}

TemplateInstantiationResult finalize_nested_member_class_instantiation_from_owner(
    SemanticContext & ctx,
    semantic_model::ClassInfo * nested_info,
    bool emit_track_instantiation)
{
  TemplateNestedMemberClassFinalizationRequest request;
  request.nested_info = nested_info;
  request.emit_track_instantiation = emit_track_instantiation;
  if(nested_info &&
     nested_info->enclosing_scope &&
     nested_info->enclosing_scope->class_info &&
     nested_info->enclosing_scope->class_info->source_template) {
    request.owner_decl = nested_info->enclosing_scope->class_info->source_template;
    request.owner_arguments =
        nested_info->enclosing_scope->class_info->instantiation_arguments;
  }
  return template_api::finalize_nested_member_class_instantiation(ctx, request);
}

namespace {

void observe_function_definition_transition(
    SemanticContext & ctx,
    TemplateInstantiationResult & result,
    TemplateLifecycleTransitionKind transition_kind,
    TemplateLifecycleCause cause)
{
  if(ctx.template_witness_context().session == nullptr ||
     !result.function_binding) {
    return;
  }
  TemplateLifecycleTransition transition;
  transition.transition_kind = transition_kind;
  transition.function_binding = result.function_binding;
  transition.cause = cause;
  transition.definition_materialized =
      transition_kind == TemplateLifecycleTransitionKind::DefinitionMaterialized;
  transition.use_declaration_as_event_location = true;
  result.lifecycle_transition = transition;
  observe_template_lifecycle_transition(ctx, transition);
}

}  // namespace

TemplateInstantiationResult acquire_function_binding_in_current_context(
    SemanticContext & ctx,
    const TemplateFunctionBindingAcquisitionRequest & request)
{
  TemplateInstantiationResult result;
  result.intent = request.intent;
  result.function_binding = request.binding;
  if(!result.function_binding) {
    return result;
  }

  const TemplateFunctionBindingAcquisitionCause acquisition_cause =
      request.cause;
  const bool require_definition_transition =
      closure_template_log_enabled(ctx) &&
      (acquisition_cause ==
           TemplateFunctionBindingAcquisitionCause::RequireDefinition ||
       acquisition_cause ==
           TemplateFunctionBindingAcquisitionCause::DefinitionAlreadyEnsured ||
       current_template_witness_entry_context().closure_reason ==
           TemplateClosureReason::RequireDefinition);
  const bool ensure_definition_transition =
      closure_template_log_enabled(ctx) &&
      (acquisition_cause ==
           TemplateFunctionBindingAcquisitionCause::DefinitionAlreadyEnsured ||
       function_binding_is_template_owned_for_definition_closure(
           result.function_binding));
  const bool definition_lifecycle_requested =
      require_definition_transition ||
      (closure_template_log_enabled(ctx) &&
       acquisition_cause ==
           TemplateFunctionBindingAcquisitionCause::EnsureDefinition);
  semantic_model::Scope * use_scope =
      request.use_scope.valid() ? &request.use_scope.require() :
                                  result.function_binding->declaration_scope;
  if(request.include_body && use_scope) {
    if(definition_lifecycle_requested) {
      const template_api::ScopedTemplateWitnessEntryContext entry_context(
          template_api::make_function_binding_closure_entry_context(
              ctx,
              TemplateClosureReason::EnsureDefinition,
              result.function_binding));
      result.function_binding =
          ctx.ensure_function_template_definition(result.function_binding, *use_scope);
    } else {
      result.function_binding =
          ctx.ensure_function_template_definition(result.function_binding, *use_scope);
    }
  }

  result.definition_materialized =
      result.function_binding && result.function_binding->has_definition;
  if(definition_lifecycle_requested) {
    if(require_definition_transition) {
      observe_function_definition_transition(
          ctx,
          result,
          TemplateLifecycleTransitionKind::DefinitionRequired,
          TemplateLifecycleCause::RequireDefinition);
    }
    const bool definition_ensure_completed =
        (request.include_body && use_scope) ||
        acquisition_cause ==
            TemplateFunctionBindingAcquisitionCause::DefinitionAlreadyEnsured;
    if(definition_ensure_completed && ensure_definition_transition) {
      TemplateWitnessEntryContext ensure_context =
          current_template_witness_entry_context();
      ensure_context.closure_reason = TemplateClosureReason::EnsureDefinition;
      const template_api::ScopedTemplateWitnessEntryContext entry_context(
          ensure_context);
      observe_function_definition_transition(
          ctx,
          result,
          TemplateLifecycleTransitionKind::DefinitionEnsured,
          TemplateLifecycleCause::EnsureDefinition);
      if(result.definition_materialized) {
        observe_function_definition_transition(
            ctx,
            result,
            TemplateLifecycleTransitionKind::DefinitionMaterialized,
            TemplateLifecycleCause::EnsureDefinition);
      }
    } else if(!definition_ensure_completed &&
              require_definition_transition &&
              result.definition_materialized) {
      observe_function_definition_transition(
          ctx,
          result,
          TemplateLifecycleTransitionKind::DefinitionMaterialized,
          TemplateLifecycleCause::RequireDefinition);
    }
  }
  const bool declaration_instantiation =
      request.cause ==
          TemplateFunctionBindingAcquisitionCause::DeclarationInstantiation;
  const bool explicit_instantiation =
      request.cause ==
          TemplateFunctionBindingAcquisitionCause::ExplicitInstantiationDefinition;
  if(ctx.template_witness_context().session != nullptr &&
     result.function_binding &&
     (declaration_instantiation || explicit_instantiation)) {
    result.lifecycle_transition.transition_kind =
        TemplateLifecycleTransitionKind::Instantiated;
    result.lifecycle_transition.function_binding = result.function_binding;
    result.lifecycle_transition.source_use_node = request.source_use_node;
    result.lifecycle_transition.cause = explicit_instantiation ?
        TemplateLifecycleCause::ExplicitInstantiationDefinition :
        lifecycle_cause_for_current_context_or_intent(request.intent);
    result.lifecycle_transition.definition_materialized =
        result.definition_materialized;
    result.lifecycle_transition.use_declaration_as_event_location =
        declaration_instantiation;
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  apply_function_instantiation_intent(
      ctx, result.function_binding, request.intent, &result);
  return result;
}

TemplateInstantiationResult acquire_function_binding(
    SemanticContext & ctx,
    const TemplateFunctionBindingAcquisitionRequest & request)
{
  const TemplateClosureReason request_closure_reason =
      closure_reason_for_function_binding_acquisition_cause(request.cause);
  const bool reports_existing_definition =
      request.cause ==
          TemplateFunctionBindingAcquisitionCause::DefinitionAlreadyEnsured;
  if(request_closure_reason != TemplateClosureReason::None &&
     ctx.template_witness_context().session != nullptr &&
     current_template_witness_entry_context().origin !=
         TemplateWitnessOrigin::Closure &&
     (reports_existing_definition ||
      function_binding_has_template_identity(request.binding))) {
    const ScopedTemplateWitnessEntryContext entry_context(
        make_function_binding_closure_entry_context(ctx,
                                                    request_closure_reason,
                                                    request.binding));
    return acquire_function_binding_in_current_context(ctx, request);
  }
  return acquire_function_binding_in_current_context(ctx, request);
}

TemplateInstantiationResult acquire_variable_instantiation(
    SemanticContext & ctx,
    const TemplateVariableInstantiationRequest & request)
{
  TemplateInstantiationResult result;
  result.intent = request.intent;
  const std::size_t size_before =
      request.decl ? request.decl->instantiations.size() : 0;
  if(!request.decl) {
    return result;
  }

  const auto instantiate = [&]()
  {
    const ScopedSourceTypeMaterialization source_type_materialization(
        ctx.template_witness_context().session != nullptr,
        SourceTypeMaterializationOwner::VariableTemplateInitializer,
        SourceTypeMaterializationOperation::VariableTemplateInitializer,
        request.decl ? request.decl->initializer : nullptr,
        nullptr,
        request.decl,
        true);
    return template_instantiation::instantiate_variable_template(
        ctx,
        *request.decl,
        request.arguments,
        request.source_use_location,
        request.source_use_scope);
  };
  const bool enter_variable_instantiation_context =
      ctx.template_witness_context().session != nullptr &&
      request.intent == TemplateInstantiationIntent::TrackInstantiation &&
      current_template_witness_entry_context().origin !=
          TemplateWitnessOrigin::Closure;
  if(enter_variable_instantiation_context) {
    const ScopedTemplateWitnessEntryContext entry_context(
        make_template_closure_entry_context(
            TemplateClosureReason::TrackInstantiation,
            variable_template_decl_log_entity(request.decl),
            variable_template_decl_location(ctx, request.decl),
            true,
            TemplateWitnessTriggerKind::Variable));
    result.value_binding = instantiate();
  } else {
    result.value_binding = instantiate();
  }
  result.created_new_value =
      result.value_binding &&
      request.decl->instantiations.size() > size_before;
  const TemplateWitnessEntryContext current_entry_context =
      template_api::current_template_witness_entry_context();
  const bool direct_source_explicit_specialization =
      current_entry_context.origin == TemplateWitnessOrigin::Source &&
      result.value_binding &&
      result.value_binding->is_explicit_specialization;
  const bool variable_log_enabled =
      ctx.template_witness_context().session != nullptr &&
      result.value_binding &&
      !direct_source_explicit_specialization &&
      (current_entry_context.origin == TemplateWitnessOrigin::Closure ||
       request.intent == TemplateInstantiationIntent::TrackInstantiation);
  if(variable_log_enabled) {
    result.lifecycle_transition.transition_kind =
        TemplateLifecycleTransitionKind::Instantiated;
    result.lifecycle_transition.value_binding = result.value_binding;
    result.lifecycle_transition.source_value_binding = result.value_binding;
    result.lifecycle_transition.value_owner =
        result.value_binding ? result.value_binding->owner_class : nullptr;
    result.lifecycle_transition.variable_template = request.decl;
    result.lifecycle_transition.cause =
        lifecycle_cause_for_current_context_or_intent(request.intent);
    result.lifecycle_transition.created_new = result.created_new_value;
    observe_template_lifecycle_transition(ctx, result.lifecycle_transition);
  }
  return result;
}

ClassSpecializationSelection select_class_specialization(
    SemanticContext & ctx,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return select_class_specialization(
            services, decl, use_scope, key, arguments, dependent_source_argument_texts);
      });
}

ClassSpecializationSelection select_class_specialization(
    TemplateServices & services,
    semantic_model::ClassTemplateDecl & decl,
    semantic_model::Scope & use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  return select_class_specialization(
      services,
      decl,
      make_template_environment(use_scope),
      key,
      arguments,
      dependent_source_argument_texts);
}

ClassSpecializationSelection select_class_specialization(
    TemplateServices & services,
    semantic_model::ClassTemplateDecl & decl,
    TemplateEnvironmentHandle use_scope,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  return to_api_class_specialization_selection(
      template_selection::select_class_specialization(
          services, decl, use_scope, key, arguments, dependent_source_argument_texts));
}

VariableSpecializationSelection select_variable_specialization(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return select_variable_specialization(services, decl, key, arguments);
      });
}

VariableSpecializationSelection select_variable_specialization(
    TemplateServices & services,
    semantic_model::VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  return to_api_variable_specialization_selection(
      template_selection::select_variable_specialization(
          services, decl, key, arguments));
}

bool match_partial_class_specialization(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const semantic_model::PartialClassTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return match_partial_class_specialization(
            services,
            make_template_environment(scope),
            partial,
            actual_arguments,
            deduced_arguments,
            specificity_score,
            deduced_pack_sizes);
      });
}

bool match_partial_class_specialization(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const semantic_model::PartialClassTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return template_specialization::match_partial_class_specialization(services,
                                                                     scope,
                                                                     partial,
                                                                     actual_arguments,
                                                                     deduced_arguments,
                                                                     specificity_score,
                                                                     deduced_pack_sizes);
}

bool match_partial_variable_specialization(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const semantic_model::VariableTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return match_partial_variable_specialization(
            services,
            make_template_environment(scope),
            partial,
            actual_arguments,
            deduced_arguments,
            specificity_score,
            deduced_pack_sizes);
      });
}

bool match_partial_variable_specialization(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const semantic_model::VariableTemplateSpecializationDecl & partial,
    const std::vector<template_model::TemplateArgument> & actual_arguments,
    std::vector<template_model::TemplateArgument> & deduced_arguments,
    std::size_t & specificity_score,
    std::map<std::string, std::size_t> * deduced_pack_sizes)
{
  return template_specialization::match_partial_variable_specialization(services,
                                                                        scope,
                                                                        partial,
                                                                        actual_arguments,
                                                                        deduced_arguments,
                                                                        specificity_score,
                                                                        deduced_pack_sizes);
}

std::string normalize_special_member_template_name(SemanticContext & ctx,
                                                   const std::string & name,
                                                   bool is_constructor,
                                                   bool is_destructor)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return normalize_special_member_template_name(
            services, name, is_constructor, is_destructor);
      });
}

std::string normalize_special_member_template_name(TemplateServices & services,
                                                   const std::string & name,
                                                   bool is_constructor,
                                                   bool is_destructor)
{
  return template_function_signature::normalize_special_member_template_name(
      services, name, is_constructor, is_destructor);
}

void parse_function_template_parameter_clause(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
    std::vector<const CppAstNode *> & default_arguments)
{
  with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        template_function_signature::parse_function_template_parameter_clause(
            services,
            make_template_environment(scope),
            template_name,
            parameter_clause,
            params,
            default_arguments);
      });
}

ParsedFunctionTemplateSignature parse_function_template_signature(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  return with_template_services(
      ctx,
      [&](TemplateServices & services)
      {
        return parse_function_template_signature(services,
                                                 scope,
                                                 template_name,
                                                 raw_declarator,
                                                 parse_specifiers,
                                                 parse_declarator,
                                                 filter_nonmember_declarator);
      });
}

ParsedFunctionTemplateSignature parse_function_template_signature(
    TemplateServices & services,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  return to_api_parsed_function_template_signature(
      template_function_signature::parse_function_template_signature(
          services,
          make_template_environment(scope),
          template_name,
          raw_declarator,
          parse_specifiers,
          parse_declarator,
          filter_nonmember_declarator));
}

}  // namespace template_api
