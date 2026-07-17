#include "semantic_class_model.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "constructor_lifecycle_service.h"
#include "file_timing.h"
#include "callsemantic_internal.h"
#include "callsemantic/template_source_utils.h"
#include "parser_trace.h"
#include "semantic_declaration.h"
#include "semantic_conversion.h"
#include "semantic_context.h"
#include "semantic_dependent_type.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_hotspot.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_output.h"
#include "semantic_scope_mutation.h"
#include "semantic_template_class.h"
#include "semantic_template_function.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"
#include "template_argument_semantics.h"
#include "template_services.h"
#include "template_witness.h"

namespace semantic_class_model {

using namespace cpp_decl;
using namespace semantic_conversion;
using namespace semantic_lookup;
using namespace semantic_model;

namespace {

const int kMaxReferenceMemberCollectionDepth = 512;

bool reference_collection_can_defer_alias_failure(const std::string & message);

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const std::string & location)
  {
    parser_trace::push_use_location(location);
  }

  ~ScopedTemplateUseLocation()
  {
    parser_trace::pop_use_location();
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;
};

std::string template_public_use_location_or(const std::string & fallback)
{
  if(parser_trace::use_location_suppressed()) {
    return std::string();
  }
  const std::string current = parser_trace::current_use_location();
  return !current.empty() ? current : fallback;
}

bool scope_has_internal_namespace_linkage(const Scope * scope)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope && current->name == "<unnamed>") {
      return true;
    }
  }
  return false;
}

symbol_linkage::SymbolLinkage synthesized_class_member_symbol_linkage(const ClassInfo & info)
{
  return scope_has_internal_namespace_linkage(info.member_scope.get()) ?
             symbol_linkage::SL_INTERNAL :
             symbol_linkage::SL_WEAK;
}

symbol_linkage::FunctionRefQualifier to_symbol_linkage_ref_qualifier(RefQualifier ref_qualifier)
{
  switch(ref_qualifier) {
  case RQ_NONE:
    return symbol_linkage::FRQ_NONE;
  case RQ_LVALUE:
    return symbol_linkage::FRQ_LVALUE;
  case RQ_RVALUE:
    return symbol_linkage::FRQ_RVALUE;
  }
  return symbol_linkage::FRQ_NONE;
}

bool direct_scope_redeclares_template_parameter_name(const Scope & scope,
                                                     const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  if(scope.template_bound_type_names.count(name) != 0 ||
     scope.template_bound_type_pack_names.count(name) != 0 ||
     scope.template_bound_value_names.count(name) != 0 ||
     scope.template_bound_template_names.count(name) != 0) {
    return true;
  }

  std::map<std::string, ClassTemplateDecl *>::const_iterator class_found =
      scope.class_templates.find(name);
  return class_found != scope.class_templates.end() && class_found->second == nullptr;
}

bool class_redeclares_template_parameter_name(const ClassInfo & info,
                                              const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  if(direct_scope_redeclares_template_parameter_name(*info.member_scope, name)) {
    return true;
  }
  if(!info.source_template) {
    return false;
  }
  const std::vector<template_model::TemplateParameterInfo> * parameters =
      &info.source_template->parameters;
  if(info.template_output_node &&
     info.source_template->class_node &&
     info.template_output_node != info.source_template->class_node) {
    for(std::size_t i = 0;
        i < info.source_template->partial_specializations.size();
        ++i) {
      const PartialClassTemplateSpecializationDecl & partial =
          info.source_template->partial_specializations[i];
      if(partial.class_node == info.template_output_node) {
        parameters = &partial.parameters;
        break;
      }
    }
  }
  for(std::size_t i = 0; i < parameters->size(); ++i) {
    const template_model::TemplateParameterInfo & parameter =
        (*parameters)[i];
    if(parameter.name == name) {
      return true;
    }
    for(std::size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(parameter.alternate_names[j] == name) {
        return true;
      }
    }
  }
  return false;
}

const PartialClassTemplateSpecializationDecl *
selected_partial_specialization_for_class_output_node(const ClassInfo & info)
{
  if(!info.source_template ||
     !info.template_output_node ||
     !info.source_template->class_node ||
     info.template_output_node == info.source_template->class_node) {
    return nullptr;
  }
  for(std::size_t i = 0; i < info.source_template->partial_specializations.size(); ++i) {
    const PartialClassTemplateSpecializationDecl & partial =
        info.source_template->partial_specializations[i];
    if(partial.class_node == info.template_output_node) {
      return &partial;
    }
  }
  return nullptr;
}

void class_template_member_substitution_bindings(
    const ClassInfo & info,
    const std::vector<template_model::TemplateParameterInfo> *& parameters,
    const std::vector<template_model::TemplateArgument> *& arguments)
{
  parameters = nullptr;
  arguments = nullptr;
  if(!info.source_template) {
    return;
  }
  parameters = &info.source_template->parameters;
  arguments = &info.instantiation_arguments;
  if(info.has_instantiation_binding_arguments) {
    if(const PartialClassTemplateSpecializationDecl * partial =
           selected_partial_specialization_for_class_output_node(info)) {
      parameters = &partial->parameters;
    }
    arguments = &info.instantiation_binding_arguments;
  }
}

bool ast_node_contains(const CppAstNode * outer, const CppAstNode * inner)
{
  if(!outer || !inner ||
     outer->token_end <= outer->token_start ||
     inner->token_end <= inner->token_start) {
    return false;
  }
  return outer->token_start <= inner->token_start &&
         inner->token_end <= outer->token_end;
}

bool should_preserve_class_template_across_reference_reset(
    const ClassInfo & info,
    const std::string & name,
    const ClassTemplateDecl * decl)
{
  if(!info.member_scope) {
    return false;
  }
  if(info.member_scope->template_bound_template_names.count(name) != 0) {
    return true;
  }

  const CppAstNode * owner_node =
      info.template_output_node ? info.template_output_node : info.class_node;
  if(decl && owner_node) {
    for(std::map<std::string, ClassTemplateSpecializationDecl>::const_iterator it =
            decl->explicit_specializations.begin();
        it != decl->explicit_specializations.end();
        ++it) {
      if(it->second.class_node &&
         !ast_node_contains(owner_node, it->second.class_node)) {
        return true;
      }
    }
    for(std::size_t i = 0; i < decl->partial_specializations.size(); ++i) {
      const PartialClassTemplateSpecializationDecl & partial =
          decl->partial_specializations[i];
      if((partial.class_node &&
          !ast_node_contains(owner_node, partial.class_node)) ||
         !partial.static_member_definitions.empty() ||
         !partial.witness_static_member_definitions.empty() ||
         !partial.member_function_definitions.empty() ||
         !partial.member_function_template_definitions.empty()) {
        return true;
      }
    }
    if(!decl->static_member_definitions.empty() ||
       !decl->witness_static_member_definitions.empty() ||
       !decl->member_class_definitions.empty() ||
       !decl->member_function_definitions.empty() ||
       !decl->member_function_template_definitions.empty()) {
      return true;
    }
  }
  return decl &&
         decl->class_node &&
         owner_node &&
         !ast_node_contains(owner_node, decl->class_node);
}

bool class_template_has_reference_reset_witness_static_member_metadata(
    const ClassTemplateDecl * decl)
{
  if(!decl) {
    return false;
  }
  if(!decl->static_member_definitions.empty()) {
    return true;
  }
  for(std::size_t i = 0; i < decl->partial_specializations.size(); ++i) {
    if(!decl->partial_specializations[i].static_member_definitions.empty()) {
      return true;
    }
  }
  return false;
}

void reset_method_syntax_info(MethodSyntaxInfo & out)
{
  out.filtered_specifiers = CppAstNode();
  out.filtered_specifiers.kind = CppAstKind::decl_specifier_seq;
  out.filtered_declarator = CppAstNode();
  out.decl_static = false;
  out.decl_virtual = false;
  out.decl_explicit = false;
  out.is_override = false;
  out.is_final = false;
  out.is_const_method = false;
  out.is_volatile_method = false;
  out.is_variadic = false;
  out.ref_qualifier = semantic_model::RQ_NONE;
  out.function_qualifier = nullptr;
}

void reset_prepared_method_parse_context(PreparedMethodParseContext & out)
{
  out.has_method_syntax = false;
  out.uses_filtered_parse = false;
  reset_method_syntax_info(out.syntax);
  out.source_specifiers = nullptr;
  out.source_declarator = nullptr;
}

void retarget_polymorphic_imported_destructors_to_base_entry(ClassInfo & info)
{
  if(!info.is_polymorphic) {
    return;
  }

  for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it = info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(!binding || !binding->is_destructor) {
        continue;
      }
      if(binding->body || binding->definition_node || binding->has_definition) {
        continue;
      }

      symbol_linkage::FunctionSymbolOptions options;
      options.is_member_function = true;
      options.has_implicit_object_parameter = true;
      options.is_const_method = binding->is_const_method;
      options.is_volatile_method = binding->is_volatile_method;
      options.ref_qualifier = to_symbol_linkage_ref_qualifier(binding->ref_qualifier);
      options.abi_tags = function_binding_abi_tags(*binding);
      options.is_destructor = true;
      options.special_member_entry_point_kind = symbol_linkage::SMEK_BASE;
      options.lookup_scope = binding->declaration_scope ? binding->declaration_scope :
                             info.member_scope.get();
      options.suppress_template_argument_pack_grouping =
          info.is_explicit_specialization;
      const std::string qualified_name =
          binding->name.find("::") != std::string::npos ?
              binding->name :
              info.qualified_name + "::" + binding->name;
      std::string display_name = binding->display_name.empty() ? binding->name : binding->display_name;
      const std::size_t display_split = display_name.rfind("::");
      if(display_split != std::string::npos) {
        display_name = display_name.substr(display_split + 2);
      }
      QualifiedName qualified_name_syntax;
      const bool has_qualified_name_syntax =
          function_binding_qualified_name_syntax_for_symbol(*binding,
                                                            qualified_name_syntax);
      if(!has_qualified_name_syntax && !binding->is_c_linkage) {
        throw std::logic_error("missing qualified-name syntax for base destructor symbol " +
                               qualified_name);
      }
      symbol_linkage::SymbolIdentity updated =
          has_qualified_name_syntax ?
              symbol_linkage::make_function_symbol_identity(qualified_name_syntax,
                                                            display_name,
                                                            binding->is_c_linkage,
                                                            binding->type,
                                                            options,
                                                            std::string(),
                                                            binding->symbol.linkage) :
              symbol_linkage::make_c_function_symbol_identity(display_name,
                                                              binding->symbol.linkage);
      updated.internal_symbol = binding->symbol.internal_symbol;
      updated.keep_internal_alias = binding->symbol.keep_internal_alias;
      binding->symbol = updated;
    }
  }
}

bool is_union_class_kind(const std::string & class_kind)
{
  return class_kind == "union";
}

bool is_union_class_info(const ClassInfo & info)
{
  return is_union_class_kind(info.class_kind);
}

bool has_user_declared_destructor(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i] && it->second[i]->is_destructor && !it->second[i]->synthesized) {
        return true;
      }
    }
  }
  return false;
}

bool function_body_has_no_statements(const FunctionBinding & binding)
{
  if(!binding.body || binding.body->kind != CppAstKind::compound_statement) {
    return false;
  }
  return binding.body->children.empty();
}

const CppAstNode * find_class_function_body_node(const CppAstNode & node)
{
  if(const CppAstNode * body = find_child(node, CppAstKind::compound_statement)) {
    return body;
  }
  if(const CppAstNode * body = find_child(node, CppAstKind::lazy_function_body)) {
    return body;
  }
  return find_child(node, CppAstKind::try_block);
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

const BaseInfo * find_direct_base_for_mem_initializer(SemanticContext & ctx,
                                                      const ClassInfo & info,
                                                      const CppAstNode & id)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(!base.type) {
      continue;
    }
    if(id.value == base.type->name ||
       id.value == base.type->qualified_name) {
      return &base;
    }
  }

  if(info.member_scope) {
    TypePtr named = ctx.lookup_type_node(*info.member_scope, id, id.value, true);
    if(named) {
      named = strip_top_level_cv(named);
      for(size_t i = 0; i < info.bases.size(); ++i) {
        const BaseInfo & base = info.bases[i];
        if(base.type &&
           type_equals(named, strip_top_level_cv(base.type->type))) {
          return &base;
        }
      }
    }
  }

  return nullptr;
}

bool is_passthrough_copy_ctor_for_host_abi(SemanticContext & ctx,
                                           const FunctionBinding & binding,
                                           const ClassInfo & info)
{
  const std::string source_name = function_parameter_alias_name(binding, 1).empty() ?
                                      function_parameter_binding_name(binding, 1) :
                                      function_parameter_alias_name(binding, 1);
  TypePtr param_base =
      binding.params.size() == 2 ? strip_top_level_cv(binding.params[1].second) : TypePtr();
  if(!binding.is_constructor ||
     binding.params.size() != 2 ||
     !param_base ||
     param_base->kind != Type::TK_LVALUE_REFERENCE ||
     !same_type_with_compatible_top_cv(param_base->inner, info.type) ||
     source_name.empty() ||
     !function_body_has_no_statements(binding) ||
     !info.fields.empty()) {
    return false;
  }

  if(info.bases.empty()) {
    return binding.ctor_initializer == nullptr;
  }
  if(!binding.ctor_initializer ||
     binding.ctor_initializer->children.size() != info.bases.size()) {
    return false;
  }

  std::set<const BaseInfo *> seen_bases;
  for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    const CppAstNode & init = binding.ctor_initializer->children[i];
    const CppAstNode * id = cppast_find_child_kind(init, CppAstKind::mem_initializer_id);
    if(!id) {
      return false;
    }
    const BaseInfo * base = find_direct_base_for_mem_initializer(ctx, info, *id);
    if(!base || base->is_virtual || seen_bases.count(base) != 0) {
      return false;
    }
    if(!mem_initializer_has_single_identifier_argument(init, source_name)) {
      return false;
    }
    seen_bases.insert(base);
  }

  return seen_bases.size() == info.bases.size();
}

bool has_nontrivial_declared_destructor_for_host_abi(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionBinding * binding = it->second[i];
      if(binding &&
         binding->is_destructor &&
         (binding->is_deleted ||
          (!binding->synthesized &&
           !binding->is_defaulted &&
           !function_body_has_no_statements(*binding)))) {
        return true;
      }
    }
  }
  return false;
}

bool has_virtual_destructor_for_host_abi(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionBinding * binding = it->second[i];
      if(binding && binding->is_destructor && binding->has_virtual_slot) {
        return true;
      }
    }
  }
  return false;
}

bool implicit_copy_constructor_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info);
bool implicit_move_constructor_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info);
FunctionBinding * find_constructor_binding(ClassInfo & info,
                                           Type::Kind ref_kind);
bool has_user_declared_move_constructor(const ClassInfo & info);
bool has_user_declared_move_assignment(const ClassInfo & info);

bool has_nontrivial_copy_constructor_for_host_abi(SemanticContext & ctx,
                                                  const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    TypePtr param_base =
        binding && binding->params.size() == 2 ?
            strip_top_level_cv(binding->params[1].second) :
            TypePtr();
    if(binding &&
       binding->is_constructor &&
       binding->params.size() == 2 &&
       param_base &&
       param_base->kind == Type::TK_LVALUE_REFERENCE &&
       same_type_with_compatible_top_cv(param_base->inner, info.type) &&
       (binding->is_deleted ||
        (!binding->synthesized &&
         !binding->is_defaulted &&
         !is_passthrough_copy_ctor_for_host_abi(ctx, *binding, info)))) {
      return true;
    }
  }
  return false;
}

bool type_allows_implicit_copy_construction_for_triviality_query(
    SemanticContext & ctx,
    const TypePtr & type,
    std::set<ClassInfo *> & visiting);

bool implicit_copy_constructor_is_deleted_for_triviality_query(
    SemanticContext & ctx,
    ClassInfo & info,
    std::set<ClassInfo *> & visiting)
{
  if(has_user_declared_move_constructor(info) ||
     has_user_declared_move_assignment(info)) {
    return true;
  }
  if(visiting.count(&info) != 0) {
    return false;
  }

  visiting.insert(&info);
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_implicit_copy_construction_for_triviality_query(
           ctx,
           info.bases[i].type->type,
           visiting)) {
      visiting.erase(&info);
      return true;
    }
  }
  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!type_allows_implicit_copy_construction_for_triviality_query(
           ctx,
           info.fields[i].type,
           visiting)) {
      visiting.erase(&info);
      return true;
    }
  }
  visiting.erase(&info);
  return false;
}

bool type_allows_implicit_copy_construction_for_triviality_query(
    SemanticContext & ctx,
    const TypePtr & type,
    std::set<ClassInfo *> & visiting)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return true;
  }
  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_implicit_copy_construction_for_triviality_query(
               ctx,
               base->inner,
               visiting);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    FunctionBinding * ctor =
        find_constructor_binding(*info, Type::TK_LVALUE_REFERENCE);
    if(ctor) {
      if(ctor->synthesized) {
        ctor->is_deleted =
            implicit_copy_constructor_is_deleted_for_triviality_query(ctx,
                                                                      *info,
                                                                      visiting);
        ctor->has_definition = !ctor->is_deleted;
      }
      return !ctor->is_deleted;
    }
    return !implicit_copy_constructor_is_deleted_for_triviality_query(ctx,
                                                                      *info,
                                                                      visiting);
  }

  return base->kind != Type::TK_FUNCTION && !is_void_type(base);
}

bool is_trivially_default_constructible_type_for_host_abi_impl(
    SemanticContext & ctx,
    const TypePtr & type,
    std::set<ClassInfo *> & visiting)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return false;
  }
  if(is_array_type(base)) {
    return base->has_bound &&
           is_trivially_default_constructible_type_for_host_abi_impl(ctx,
                                                                     base->inner,
                                                                     visiting);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  if(is_named_enum_type(ctx, base)) {
    return true;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete || info->is_polymorphic || !info->member_scope) {
    return false;
  }
  if(visiting.count(info) != 0) {
    return true;
  }

  FunctionBinding * ctor = ctx.select_default_constructor_for_builtin_trait(*info->member_scope,
                                                                            *info);
  if(!ctor || ctor->is_deleted) {
    return false;
  }
  const bool structural_noop =
      ctor->synthesized ||
      ctor->is_defaulted ||
      ctor->is_aggregate_constructor ||
      (ctor->is_constructor &&
       ctor->params.size() == 1 &&
       !ctor->ctor_initializer &&
       function_body_has_no_statements(*ctor));
  if(!structural_noop) {
    return false;
  }

  if(is_union_class_info(*info)) {
    if(!info->bases.empty()) {
      return false;
    }
    for(size_t i = 0; i < info->fields.size(); ++i) {
      if(info->fields[i].default_initializer) {
        return false;
      }
    }
    return true;
  }

  visiting.insert(info);
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_default_constructible_type_for_host_abi_impl(ctx,
                                                                  info->bases[i].type->type,
                                                                  visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(info->fields[i].default_initializer ||
       !is_trivially_default_constructible_type_for_host_abi_impl(ctx,
                                                                  info->fields[i].type,
                                                                  visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  visiting.erase(info);
  return true;
}

bool is_trivially_default_constructible_type_for_host_abi_local(SemanticContext & ctx,
                                                                const TypePtr & type)
{
  std::set<ClassInfo *> visiting;
  return is_trivially_default_constructible_type_for_host_abi_impl(ctx, type, visiting);
}

bool is_trivially_destructible_type_for_host_abi_local(SemanticContext & ctx,
                                                       const TypePtr & type);

bool is_trivially_copy_constructible_type_for_host_abi_local(SemanticContext & ctx,
                                                             const TypePtr & type);

void append_host_integer_abi_chunks_for_size(
    std::size_t size,
    std::vector<Type::HostAbiChunk> & out)
{
  while(size != 0) {
    Type::HostAbiChunk chunk;
    chunk.kind = Type::HostAbiChunk::HC_INTEGER;
    chunk.size = std::min<std::size_t>(8, size);
    out.push_back(chunk);
    size -= chunk.size;
  }
}

bool collect_host_direct_integer_abi_chunks(SemanticContext & ctx,
                                            const TypePtr & type,
                                            std::vector<Type::HostAbiChunk> & out)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_FUNDAMENTAL) {
    switch(base->fundamental) {
    case FT_FLOAT:
    case FT_DOUBLE:
    case FT_LONG_DOUBLE:
    case FT_VOID:
      return false;
    default:
      break;
    }
    const std::size_t size = type_size(base);
    if(size == 0 || size > 16) {
      return false;
    }
    append_host_integer_abi_chunks_for_size(size, out);
    return true;
  }

  if(base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER ||
     base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    const std::size_t size = type_size(base);
    if(size == 0 || size > 16) {
      return false;
    }
    append_host_integer_abi_chunks_for_size(size, out);
    return true;
  }

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      return false;
    }
    const std::size_t size = type_size(base);
    if(size == 0 || size > 16) {
      return false;
    }
    std::vector<Type::HostAbiChunk> nested;
    if(!collect_host_direct_integer_abi_chunks(ctx, base->inner, nested)) {
      return false;
    }
    append_host_integer_abi_chunks_for_size(size, out);
    return true;
  }

  if(base->kind == Type::TK_FUNCTION) {
    return false;
  }

  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  if(is_named_enum_type(ctx, base)) {
    const std::size_t size = type_size(base);
    if(size == 0 || size > 16) {
      return false;
    }
    append_host_integer_abi_chunks_for_size(size, out);
    return true;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete ||
     !base->named_has_layout ||
     !is_trivially_copy_constructible_type_for_host_abi_local(ctx, base) ||
     !is_trivially_destructible_type_for_host_abi_local(ctx, base)) {
    return false;
  }

  const std::size_t size = type_size(base);
  if(size == 0 || size > 16) {
    return false;
  }

  if(is_union_class_info(*info)) {
    for(size_t i = 0; i < info->fields.size(); ++i) {
      const FieldInfo & field = info->fields[i];
      if(field.is_bit_field) {
        return false;
      }
      std::vector<Type::HostAbiChunk> nested;
      if(!collect_host_direct_integer_abi_chunks(ctx, field.type, nested)) {
        return false;
      }
    }
    append_host_integer_abi_chunks_for_size(size, out);
    return true;
  }

  for(size_t i = 0; i < info->bases.size(); ++i) {
    const BaseInfo & base_info = info->bases[i];
    if(base_info.is_virtual) {
      return false;
    }
    try {
      const std::size_t base_align = type_alignment(base_info.type->type);
      if(base_align != 0 && (base_info.offset % std::min<std::size_t>(base_align, 8)) != 0) {
        return false;
      }
    }
    catch(const std::logic_error &) {
      return false;
    }
    std::vector<Type::HostAbiChunk> nested;
    if(!collect_host_direct_integer_abi_chunks(ctx, base_info.type->type, nested)) {
      return false;
    }
  }

  for(size_t i = 0; i < info->fields.size(); ++i) {
    const FieldInfo & field = info->fields[i];
    if(field.is_bit_field) {
      return false;
    }
    try {
      const std::size_t field_align = type_alignment(field.type);
      if(field_align != 0 && (field.offset % std::min<std::size_t>(field_align, 8)) != 0) {
        return false;
      }
    }
    catch(const std::logic_error &) {
      return false;
    }
    std::vector<Type::HostAbiChunk> nested;
    if(!collect_host_direct_integer_abi_chunks(ctx, field.type, nested)) {
      return false;
    }
  }

  append_host_integer_abi_chunks_for_size(size, out);
  return true;
}

bool is_trivially_destructible_type_for_host_abi_local(SemanticContext & ctx,
                                                       const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_destructible_type_for_host_abi_local(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  if(is_named_enum_type(ctx, base)) {
    return true;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete ||
     has_nontrivial_declared_destructor_for_host_abi(*info) ||
     has_virtual_destructor_for_host_abi(*info)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!is_trivially_destructible_type_for_host_abi_local(ctx, info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_destructible_type_for_host_abi_local(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_copy_constructible_type_for_host_abi_local(SemanticContext & ctx,
                                                             const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_constructible_type_for_host_abi_local(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  if(is_named_enum_type(ctx, base)) {
    return true;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete || info->is_polymorphic) {
    return false;
  }
  FunctionBinding * copy_ctor =
      find_constructor_binding(*info, Type::TK_LVALUE_REFERENCE);
  if(copy_ctor && copy_ctor->synthesized && info->complete) {
    std::set<ClassInfo *> deletion_visiting;
    copy_ctor->is_deleted =
        implicit_copy_constructor_is_deleted_for_triviality_query(ctx,
                                                                  *info,
                                                                  deletion_visiting);
    copy_ctor->has_definition = !copy_ctor->is_deleted;
  }
  std::set<ClassInfo *> deletion_visiting;
  const bool implicit_copy_deleted =
      copy_ctor == nullptr &&
      implicit_copy_constructor_is_deleted_for_triviality_query(ctx,
                                                                *info,
                                                                deletion_visiting);
  if(has_nontrivial_declared_destructor_for_host_abi(*info) ||
     implicit_copy_deleted ||
     has_nontrivial_copy_constructor_for_host_abi(ctx, *info)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_copy_constructible_type_for_host_abi_local(ctx,
                                                                info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_copy_constructible_type_for_host_abi_local(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_move_constructible_type_for_host_abi_impl(
    SemanticContext & ctx,
    const TypePtr & type,
    std::set<ClassInfo *> & visiting)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return base->has_bound &&
           is_trivially_move_constructible_type_for_host_abi_impl(ctx,
                                                                  base->inner,
                                                                  visiting);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  if(is_named_enum_type(ctx, base)) {
    return true;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete || info->is_polymorphic) {
    return false;
  }
  if(visiting.count(info) != 0) {
    return true;
  }

  FunctionBinding * move_ctor = nullptr;
  if(!type_is_const_object(type)) {
    move_ctor = find_constructor_binding(*info, Type::TK_RVALUE_REFERENCE);
    if(move_ctor && move_ctor->synthesized && info->complete) {
      move_ctor->is_deleted = implicit_move_constructor_is_deleted(ctx, *info);
      move_ctor->has_definition = !move_ctor->is_deleted;
    }
    if(!move_ctor) {
      move_ctor = ensure_implicit_move_constructor(ctx, *info);
    }
  }

  if(!move_ctor || move_ctor->is_deleted) {
    return is_trivially_copy_constructible_type_for_host_abi_local(ctx, base);
  }

  if(!move_ctor->synthesized && !move_ctor->is_defaulted) {
    return false;
  }

  if(has_nontrivial_declared_destructor_for_host_abi(*info)) {
    return false;
  }

  visiting.insert(info);
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_move_constructible_type_for_host_abi_impl(
           ctx,
           info->bases[i].type->type,
           visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_move_constructible_type_for_host_abi_impl(ctx,
                                                               info->fields[i].type,
                                                               visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  visiting.erase(info);
  return true;
}

bool is_trivially_move_constructible_type_for_host_abi_local(SemanticContext & ctx,
                                                             const TypePtr & type)
{
  std::set<ClassInfo *> visiting;
  return is_trivially_move_constructible_type_for_host_abi_impl(ctx, type, visiting);
}

void bind_member_named_type(Scope & scope,
                            const std::string & name,
                            const TypePtr & type,
                            MemberAccess access)
{
  semantic_scope_mutation::bind_named_type_with_access(scope, name, type, access);
}

bool first_identifier_text_in_subtree(const CppAstNode & node, std::string & out)
{
  if(node.kind == CppAstKind::identifier) {
    out = node.value;
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(first_identifier_text_in_subtree(node.children[i], out)) {
      return true;
    }
  }
  return false;
}

const QualifiedName * first_identifier_name_syntax_in_subtree(const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier) {
    return cppast_qualified_name_syntax(node);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const QualifiedName * found =
           first_identifier_name_syntax_in_subtree(node.children[i])) {
      return found;
    }
  }
  return nullptr;
}

const TemplateIdSyntax * first_identifier_template_id_syntax_in_subtree(
    const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier) {
    return cppast_template_id_syntax(node);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const TemplateIdSyntax * found =
           first_identifier_template_id_syntax_in_subtree(node.children[i])) {
      return found;
    }
  }
  return nullptr;
}

std::string best_effort_node_text(const CppAstNode & node)
{
  std::string text = node_text(node);
  if(!text.empty()) {
    return semantic_utils::trim_space(text);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    const std::string part = best_effort_node_text(node.children[i]);
    if(part.empty()) {
      continue;
    }
    if(!text.empty()) {
      text += " ";
    }
    text += part;
  }
  return semantic_utils::trim_space(text);
}

std::string template_argument_anchor_identifier(const std::string & text)
{
  const std::string trimmed = semantic_utils::trim_space(text);
  if(trimmed.empty()) {
    return std::string();
  }
  std::string without_args = trimmed;
  const std::string stripped =
      semantic_utils::strip_trailing_top_level_template_arguments(trimmed);
  if(stripped != trimmed && !stripped.empty()) {
    without_args = stripped;
    without_args = semantic_utils::trim_space(without_args);
  }
  const std::string identifier = semantic_utils::unqualified_member_name(without_args);
  if(callsemantic_internal::is_identifier_text(identifier)) {
    return identifier;
  }
  return callsemantic_internal::is_identifier_text(trimmed) ? trimmed : std::string();
}

std::vector<std::string> template_argument_source_locations_for_node(
    SemanticContext & ctx,
    const CppAstNode & node,
    const std::vector<std::string> & arg_texts,
    const std::string & template_start_location = std::string())
{
  const auto location_from_node_value =
      [&](const std::string & arg_text,
          const std::string & identifier) -> std::string
  {
    if(identifier.empty() || node.value.empty()) {
      return std::string();
    }
    std::size_t search_start = 0;
    const std::string trimmed_arg = semantic_utils::trim_space(arg_text);
    if(!trimmed_arg.empty()) {
      const std::size_t arg_pos = node.value.find(trimmed_arg);
      if(arg_pos != std::string::npos) {
        search_start = arg_pos;
      }
    }
    const std::size_t identifier_pos = node.value.find(identifier, search_start);
    if(identifier_pos == std::string::npos) {
      return std::string();
    }
    const std::string node_location =
        template_api::normalize_template_witness_source_location(
            !template_start_location.empty() ?
                template_start_location :
                ctx.source_location_for_node(node));
    const template_api::template_witness_detail::ParsedSourceLocation parsed =
        template_api::template_witness_detail::parse_source_location(node_location);
    if(!parsed.valid || parsed.line <= 0 || parsed.column <= 0) {
      return std::string();
    }
    std::ostringstream out;
    out << parsed.file << ":" << parsed.line << ":"
        << (parsed.column + static_cast<int>(identifier_pos));
    return out.str();
  };

  std::vector<std::string> locations;
  locations.reserve(arg_texts.size());
  for(std::size_t i = 0; i < arg_texts.size(); ++i) {
    const std::string identifier =
        template_argument_anchor_identifier(arg_texts[i]);
    std::string location =
        !identifier.empty() ?
            ctx.source_location_for_name_in_node(node, identifier) :
            std::string();
    if(!semantic_trace::source_location_points_at_identifier(location,
                                                             identifier)) {
      location.clear();
    }
    if(location.empty()) {
      location = location_from_node_value(arg_texts[i], identifier);
      if(!semantic_trace::source_location_points_at_identifier(location,
                                                               identifier)) {
        location.clear();
      }
    }
    locations.push_back(
        template_api::normalize_template_witness_source_location(location));
  }
  return locations;
}

std::string dependent_typedef_type_text(const CppAstNode & filtered_specifiers)
{
  std::string text = best_effort_node_text(filtered_specifiers);
  const std::string keyword = "typedef";
  if(text.compare(0, keyword.size(), keyword) == 0) {
    text = semantic_utils::trim_space(text.substr(keyword.size()));
  }
  return text;
}

void maybe_complete_class_member_object_type(SemanticContext & ctx,
                                             const TypePtr & type);

bool scope_has_concrete_template_instantiation_owner(const Scope & scope)
{
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->class_info &&
       current->class_info->source_template &&
       !current->class_info->dependent_instantiation &&
       !current->class_info->instantiation_arguments.empty()) {
      return true;
    }
  }
  return false;
}

bool resolve_instantiated_base_type_if_needed(SemanticContext & ctx,
                                              Scope & scope,
                                              const TypePtr & base_type,
                                              TypePtr & resolved)
{
  resolved.reset();
  return base_type &&
         scope_has_concrete_template_instantiation_owner(scope) &&
         ctx.type_depends_on_template_parameter(base_type) &&
         semantic_dependent_type::resolve_instantiated_dependent_type(ctx,
                                                                      scope,
                                                                      base_type,
                                                                      resolved) &&
         resolved;
}

bool base_text_mentions_template_parameters(
    const std::string & text,
    const std::vector<template_model::TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const template_model::TemplateParameterInfo & parameter = parameters[i];
    if(!parameter.name.empty() &&
       callsemantic_internal::contains_identifier_token(text, parameter.name)) {
      return true;
    }
    for(std::size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(!parameter.alternate_names[j].empty() &&
         callsemantic_internal::contains_identifier_token(
             text,
             parameter.alternate_names[j])) {
        return true;
      }
    }
  }
  return false;
}

bool try_resolve_instantiated_base_type_node(SemanticContext & ctx,
                                             Scope & scope,
                                             const CppAstNode & node,
                                             bool allow_incomplete_lookup,
                                             const ClassInfo * owner_info,
                                             TypePtr & out)
{
  out.reset();
  if(!owner_info ||
     !owner_info->source_template ||
     owner_info->dependent_instantiation ||
     owner_info->instantiation_arguments.empty() ||
     node.value.find("sizeof") == std::string::npos ||
     (!base_text_mentions_template_parameters(
          node.value,
          owner_info->source_template->parameters) &&
      !ctx.text_mentions_template_placeholders(scope, node.value) &&
      !ctx.text_mentions_dependent_non_namespace_binding_names(scope, node.value))) {
    return false;
  }

  CppAstNode substituted;
  const bool substituted_node =
      template_argument_semantics::substitute_type_id_node_for_template_arguments(
          ctx,
          scope,
          node,
          owner_info->source_template->parameters,
          owner_info->instantiation_arguments,
          substituted);
  if(!substituted_node) {
    return false;
  }

  out = ctx.lookup_type_node(scope,
                             substituted,
                             substituted.value,
                             allow_incomplete_lookup);
  TypePtr resolved;
  if(resolve_instantiated_base_type_if_needed(ctx, scope, out, resolved)) {
    out = resolved;
  }
  return out != nullptr;
}

TypePtr lookup_base_type_name(SemanticContext & ctx,
                              Scope & scope,
                              const std::string & text,
                              bool allow_incomplete_lookup,
                              const ClassInfo * owner_info = nullptr)
{
  const template_argument_semantics::ScopedBaseSpecifierTypeLookup
      base_lookup_guard(text, owner_info);
  TypePtr base_type;
  base_type = ctx.lookup_type(scope, text, allow_incomplete_lookup);
  TypePtr resolved;
  if(resolve_instantiated_base_type_if_needed(ctx,
                                              scope,
                                              base_type,
                                              resolved)) {
    return resolved;
  }
  return base_type;
}

bool base_type_node_has_structured_lookup_syntax(const CppAstNode & node)
{
  return node.qualified_name_syntax ||
         node.template_id_syntax ||
         !node.qualifier_template_id_syntaxes.empty() ||
         !node.qualifier_type_syntaxes.empty();
}

TypePtr resolve_base_type_node(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & node,
                               bool allow_incomplete_lookup,
                               const ClassInfo * owner_info = nullptr)
{
  const template_argument_semantics::ScopedBaseSpecifierTypeLookup
      base_lookup_guard(node.value, owner_info);
  TypePtr base_type;
  if(try_resolve_instantiated_base_type_node(ctx,
                                             scope,
                                             node,
                                             allow_incomplete_lookup,
                                             owner_info,
                                             base_type)) {
    return base_type;
  }
  if(node.base_type_syntax &&
     template_api::type::parse_decltype_or_typeof_node(ctx,
                                                       scope,
                                                       *node.base_type_syntax,
                                                       base_type) &&
     base_type) {
    return base_type;
  }
  base_type = ctx.lookup_type_node(scope,
                                   node,
                                   node.value,
                                   allow_incomplete_lookup);
  if(base_type &&
     ctx.type_depends_on_template_parameter(base_type)) {
    TypePtr resolved;
    if(resolve_instantiated_base_type_if_needed(ctx,
                                                scope,
                                                base_type,
                                                resolved)) {
      base_type = resolved;
    }
  }
  if(base_type || base_type_node_has_structured_lookup_syntax(node)) {
    return base_type;
  }
  return lookup_base_type_name(ctx,
                               scope,
                               node.value,
                               allow_incomplete_lookup,
                               owner_info);
}

bool template_argument_is_parameter_placeholder(
    const template_model::TemplateArgument & argument,
    const template_model::TemplateParameterInfo & parameter)
{
  switch(parameter.kind) {
  case template_model::TemplateParameterInfo::TP_TYPE:
    return argument.kind == template_model::TemplateArgument::TA_TYPE &&
           argument.type &&
           argument.type->kind == Type::TK_NAMED &&
           argument.type->named_key == parameter.placeholder_key;

  case template_model::TemplateParameterInfo::TP_NON_TYPE:
  case template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE:
    return false;
  }
  return false;
}

bool type_is_current_class_template_pattern(SemanticContext & ctx,
                                            Scope & scope,
                                            const TypePtr & type,
                                            ClassInfo * current_info)
{
  if(!current_info) {
    current_info = current_class_scope(scope);
  }
  if(!type ||
     !current_info ||
     !current_info->source_template ||
     !current_info->type) {
    return false;
  }

  ClassInfo * candidate = ctx.class_info_for_type(type);
  if(!candidate ||
     candidate->source_template != current_info->source_template ||
     candidate->instantiation_arguments.size() !=
         candidate->source_template->parameters.size()) {
    return false;
  }

  const std::vector<template_model::TemplateParameterInfo> & parameters =
      candidate->source_template->parameters;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!template_argument_is_parameter_placeholder(
           candidate->instantiation_arguments[i],
           parameters[i])) {
      return false;
    }
  }
  return true;
}

TypePtr substitute_current_class_template_pattern_type(SemanticContext & ctx,
                                                       Scope & scope,
                                                       const TypePtr & type,
                                                       ClassInfo * current_info,
                                                       bool & changed)
{
  if(!type) {
    return type;
  }
  if(type_is_current_class_template_pattern(ctx, scope, type, current_info)) {
    changed = true;
    return current_info ? current_info->type : current_class_scope(scope)->type;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return type;

  case Type::TK_CV:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_cv(inner, type->cv_const, type->cv_volatile);
  }

  case Type::TK_ATOMIC:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_atomic(inner);
  }

  case Type::TK_POINTER:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_pointer(inner);
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypePtr owner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->owner, current_info, changed);
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return owner == type->owner && inner == type->inner ?
        type :
        make_member_pointer(owner, inner);
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_block_pointer(inner);
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_lvalue_reference_raw(inner);
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ? type : make_rvalue_reference_raw(inner);
  }

  case Type::TK_ARRAY:
  {
    TypePtr inner =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, changed);
    return inner == type->inner ?
        type :
        make_array(inner, type->has_bound, type->bound, type->bound_text);
  }

  case Type::TK_FUNCTION:
  {
    bool local_changed = false;
    TypePtr result =
        substitute_current_class_template_pattern_type(
            ctx, scope, type->inner, current_info, local_changed);
    std::vector<TypePtr> params;
    params.reserve(type->params.size());
    for(std::size_t i = 0; i < type->params.size(); ++i) {
      params.push_back(substitute_current_class_template_pattern_type(
          ctx, scope, type->params[i], current_info, local_changed));
    }
    if(!local_changed) {
      return type;
    }
    changed = true;
    return make_function(result,
                         params,
                         type->variadic,
                         type->function_const,
                         type->function_volatile,
                         type->prototype_relaxed,
                         type->function_ref_qualifier);
  }
  }

  return type;
}

std::string strip_leading_typename_for_member_alias(const std::string & text)
{
  const std::string trimmed = semantic_utils::trim_space(text);
  static const char prefix[] = "typename";
  const std::size_t prefix_size = sizeof(prefix) - 1;
  if(trimmed.size() <= prefix_size ||
     trimmed.compare(0, prefix_size, prefix) != 0 ||
     (trimmed[prefix_size] != ' ' && trimmed[prefix_size] != '\t' &&
      trimmed[prefix_size] != '\n' && trimmed[prefix_size] != '\r')) {
    return trimmed;
  }
  return semantic_utils::trim_space(trimmed.substr(prefix_size));
}

TypePtr lookup_visible_member_alias_owner_type(SemanticContext & ctx,
                                               Scope & scope,
                                               const std::string & name,
                                               ClassInfo * current_info = nullptr)
{
  if(name.empty()) {
    return TypePtr();
  }
  const auto lookup_in_class =
      [&ctx, &scope, &name](ClassInfo & info) -> TypePtr
  {
    if(info.member_scope) {
      MemberTypeLookupResult member =
          lookup_member_type(ctx, info, name, true, &scope);
      if(member.type) {
        return member.type;
      }
      auto found =
          info.member_scope->named_types.find(name);
      if(found != info.member_scope->named_types.end()) {
        return found->second;
      }
    }
    return TypePtr();
  };
  if(current_info && current_info->member_scope) {
    if(TypePtr direct = lookup_in_class(*current_info)) {
      return direct;
    }
    std::set<ClassInfo *> visited;
    const auto lookup_enclosing_class_scopes =
        [&](Scope * start) -> TypePtr
    {
      for(Scope * current = start; current; current = current->parent) {
        if(current->class_info &&
           current->class_info != current_info &&
           visited.insert(current->class_info).second) {
          if(TypePtr found = lookup_in_class(*current->class_info)) {
            return found;
          }
        }
        if(current->namespace_scope || current->parent == nullptr) {
          break;
        }
      }
      return TypePtr();
    };
    if(TypePtr enclosing =
           lookup_enclosing_class_scopes(current_info->enclosing_scope)) {
      return enclosing;
    }
    if(current_info->source_template &&
       current_info->source_template->declaring_scope) {
      if(TypePtr declaring =
             lookup_enclosing_class_scopes(
                 current_info->source_template->declaring_scope)) {
        return declaring;
      }
    }
  }
  for(Scope * current = &scope; current; current = current->parent) {
    auto found =
        current->named_types.find(name);
    if(found != current->named_types.end()) {
      return found->second;
    }
    if(current->class_info) {
      MemberTypeLookupResult member =
          lookup_member_type(ctx, *current->class_info, name, true, &scope);
      if(member.type) {
        return member.type;
      }
    }
  }
  return TypePtr();
}

TypePtr try_rebase_dependent_member_alias_owner_root(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const TypePtr & owner,
                                                     ClassInfo * current_info,
                                                     const TemplateIdSyntax * owner_template_id_hint)
{
  if(!owner || !current_info) {
    return TypePtr();
  }

  TypePtr dependent_root;
  std::vector<std::string> members;
  bool leading_typename = false;
  std::vector<TemplateIdSyntax> member_template_ids;
  if(!named_type_dependent_qualified_member(owner,
                                            dependent_root,
                                            members,
                                            leading_typename,
                                            &member_template_ids) ||
     !dependent_root ||
     members.empty()) {
    TypePtr owner_base = strip_top_level_cv(owner);
    if(!owner_template_id_hint ||
       !owner_base ||
       owner_base->kind != Type::TK_NAMED) {
      return TypePtr();
    }
    std::string owner_text =
        !owner_base->named_display.empty() ? owner_base->named_display :
                                             owner_base->named_key;
    owner_text = strip_leading_typename_for_member_alias(owner_text);
    owner_text = semantic_utils::trim_space(owner_text);
    const std::size_t split = semantic_utils::top_level_scope_split(owner_text);
    if(split == std::string::npos) {
      return TypePtr();
    }
    std::string root_text =
        semantic_utils::trim_space(owner_text.substr(0, split));
    std::string member_text =
        semantic_utils::trim_space(owner_text.substr(split + 2));
    if(root_text.empty() ||
       root_text.find("::") != std::string::npos ||
       root_text.find('<') != std::string::npos ||
       member_text.empty()) {
      return TypePtr();
    }
    dependent_root = make_semantic_named(root_text,
                                         Type::NSK_DEPENDENT_TYPE,
                                         root_text,
                                         true);
    members.clear();
    members.push_back(member_text);
    member_template_ids.clear();
    member_template_ids.push_back(*owner_template_id_hint);
  }

  TypePtr root_base = strip_top_level_cv(dependent_root);
  if(!root_base || root_base->kind != Type::TK_NAMED) {
    return TypePtr();
  }
  std::string root_name =
      !root_base->named_display.empty() ? root_base->named_display :
                                          root_base->named_key;
  root_name = strip_leading_typename_for_member_alias(root_name);
  root_name = semantic_utils::trim_space(root_name);
  if(root_name.empty() ||
     root_name.find("::") != std::string::npos ||
     root_name.find('<') != std::string::npos) {
    return TypePtr();
  }

  TypePtr visible_root =
      lookup_visible_member_alias_owner_type(ctx, scope, root_name, current_info);
  if(!visible_root ||
     type_equals(visible_root, dependent_root)) {
    return TypePtr();
  }

  TemplateIdSyntax owner_template_id;
  if(TypePtr owner_base = strip_top_level_cv(owner)) {
    if(owner_base->named_dependent_qualified_owner_template_id) {
      owner_template_id = *owner_base->named_dependent_qualified_owner_template_id;
    }
  }
  std::string display = callsemantic_internal::reparseable_type_argument_text(visible_root);
  if(display.empty()) {
    display = describe_type(visible_root);
  }
  for(std::size_t i = 0; i < members.size(); ++i) {
    display += "::";
    display += members[i];
  }

  TypePtr rebased = make_dependent_qualified_member_type(display,
                                                         visible_root,
                                                         members,
                                                         leading_typename,
                                                         member_template_ids,
                                                         owner_template_id);
  if(!rebased) {
    return TypePtr();
  }

  TypePtr instantiated;
  if(semantic_dependent_type::resolve_instantiated_dependent_type(
         ctx, scope, rebased, instantiated) &&
     instantiated) {
    return instantiated;
  }
  return rebased;
}

TypePtr resolve_member_alias_owner_type(SemanticContext & ctx,
                                        Scope & scope,
                                        const TypePtr & owner,
                                        ClassInfo * current_info = nullptr,
                                        const TemplateIdSyntax * owner_template_id_hint = nullptr)
{
  TypePtr resolved = owner;
  if(resolved && ctx.type_depends_on_template_parameter(resolved)) {
    TypePtr instantiated;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx, scope, resolved, instantiated) &&
       instantiated) {
      resolved = instantiated;
    }
  }
  if(resolved &&
     ctx.type_depends_on_template_parameter(resolved) &&
     current_info) {
    if(TypePtr rebased =
           try_rebase_dependent_member_alias_owner_root(ctx,
                                                        scope,
                                                        resolved,
                                                        current_info,
                                                        owner_template_id_hint)) {
      resolved = rebased;
    }
  }
  if(resolved &&
     ctx.type_depends_on_template_parameter(resolved) &&
     current_info &&
     current_info->member_scope) {
    TypePtr instantiated;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx, *current_info->member_scope, resolved, instantiated) &&
       instantiated) {
      resolved = instantiated;
    }
  }
  return resolved;
}

TypePtr try_resolve_instantiated_member_alias_type(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const TypePtr & type,
                                                   ClassInfo * current_info = nullptr)
{
  TypePtr base;
  bool cv_const = false;
  bool cv_volatile = false;
  if(!top_level_cv_flags(type, base, cv_const, cv_volatile)) {
    return TypePtr();
  }
  if(!base || base->kind != Type::TK_NAMED) {
    return TypePtr();
  }

  TypePtr owner_type = base->named_member_owner_type;
  std::string member_name = base->named_member_name;

  if(!owner_type || member_name.empty()) {
    TypePtr dependent_owner;
    std::vector<std::string> dependent_members;
    bool leading_typename = false;
    if(named_type_dependent_qualified_member(type,
                                             dependent_owner,
                                             dependent_members,
                                             leading_typename,
                                             nullptr) &&
       dependent_members.size() == 1) {
      (void)leading_typename;
      owner_type = dependent_owner;
      member_name = dependent_members[0];
    }
  }

  if(!owner_type || member_name.empty()) {
    const std::string candidate =
        strip_leading_typename_for_member_alias(
            !base->named_display.empty() ? base->named_display : base->named_key);
    if(candidate.find('<') != std::string::npos) {
      return TypePtr();
    }
    const std::size_t split = semantic_utils::top_level_scope_split(candidate);
    if(split == std::string::npos) {
      return TypePtr();
    }
    const std::string owner_name =
        semantic_utils::trim_space(candidate.substr(0, split));
    member_name =
        semantic_utils::trim_space(candidate.substr(split + 2));
    if(owner_name.empty() ||
       owner_name.find("::") != std::string::npos ||
       member_name.empty() ||
       member_name.find("::") != std::string::npos ||
       member_name.find('<') != std::string::npos) {
      return TypePtr();
    }
    owner_type =
        lookup_visible_member_alias_owner_type(ctx, scope, owner_name, current_info);
  }

  owner_type =
      resolve_member_alias_owner_type(
          ctx,
          scope,
          owner_type,
          current_info,
          base->named_dependent_qualified_owner_template_id.get());
  if(!owner_type) {
    return TypePtr();
  }

  ClassInfo * owner_info = ctx.class_info_for_type(owner_type);
  if(!owner_info && !ctx.type_depends_on_template_parameter(owner_type)) {
    owner_info = ctx.complete_class_type(owner_type);
  }
  if(!owner_info) {
    if(current_info && current_info->member_scope) {
      auto direct =
          current_info->member_scope->named_types.find(member_name);
      if(direct != current_info->member_scope->named_types.end() &&
         direct->second &&
         !type_equals(direct->second, type)) {
        TypePtr member_type = direct->second;
        if(ctx.type_depends_on_template_parameter(member_type)) {
          TypePtr resolved_member;
          if(semantic_dependent_type::resolve_instantiated_dependent_type(
                 ctx, *current_info->member_scope, member_type, resolved_member) &&
             resolved_member) {
            member_type = resolved_member;
          }
        }
        if(member_type && !ctx.type_depends_on_template_parameter(member_type)) {
          return apply_cv(member_type, cv_const, cv_volatile);
        }
      }
    }
    return TypePtr();
  }

  MemberTypeLookupResult member =
      lookup_member_type(ctx, *owner_info, member_name, true, &scope);
  if(!member.type) {
    return TypePtr();
  }

  TypePtr member_type = member.type;
  if(ctx.type_depends_on_template_parameter(member_type)) {
    TypePtr resolved_member;
    Scope & member_scope =
        owner_info->member_scope ? *owner_info->member_scope : scope;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx, member_scope, member_type, resolved_member) &&
       resolved_member) {
      member_type = resolved_member;
    }
  }
  if(!member_type ||
     ctx.type_depends_on_template_parameter(member_type)) {
    return TypePtr();
  }
  return apply_cv(member_type, cv_const, cv_volatile);
}

TypePtr canonicalize_member_typedef_type(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         ClassInfo * current_info = nullptr)
{
  if(!type) {
    return type;
  }

  TypePtr base;
  bool cv_const = false;
  bool cv_volatile = false;
  if(current_info &&
     current_info->member_scope &&
     top_level_cv_flags(type, base, cv_const, cv_volatile) &&
    base &&
    base->kind == Type::TK_NAMED) {
    std::string name = semantic_utils::trim_space(
        !base->named_display.empty() ? base->named_display : base->named_key);
    const std::string typename_prefix = "typename ";
    if(name.compare(0, typename_prefix.size(), typename_prefix) == 0) {
      name = semantic_utils::trim_space(name.substr(typename_prefix.size()));
    }
    name = semantic_utils::strip_elaborated_type_prefix(name);
    if(!name.empty() &&
       name.find("::") == std::string::npos &&
       name.find('<') == std::string::npos) {
      const auto found = current_info->member_scope->named_types.find(name);
      if(found != current_info->member_scope->named_types.end() &&
         found->second &&
         !type_equals(found->second, base)) {
        return apply_cv(found->second, cv_const, cv_volatile);
      }
    }
  }

  if(TypePtr resolved_member_alias =
         try_resolve_instantiated_member_alias_type(ctx, scope, type, current_info)) {
    return resolved_member_alias;
  }

  if(!ctx.type_depends_on_template_parameter(type)) {
    return type;
  }

  bool substituted_current_class = false;
  TypePtr substituted = substitute_current_class_template_pattern_type(
      ctx, scope, type, current_info, substituted_current_class);
  if(substituted_current_class && substituted) {
    return substituted;
  }
  // Keep dependent typedef aliases in their parsed representation. Re-resolving
  // them from text discards structured template-id syntax and reintroduces the
  // semantic fallback this path is supposed to avoid.
  return type;
}

const std::vector<TypePtr> * lookup_bound_type_pack(Scope & scope,
                                                    const std::string & name)
{
  const std::string trimmed = semantic_utils::trim_space(name);
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    std::map<std::string, std::vector<TypePtr> >::const_iterator found =
        current->named_type_packs.find(trimmed);
    if(found != current->named_type_packs.end()) {
      return &found->second;
    }
  }
  return nullptr;
}

bool simple_template_argument_pack_name(const TemplateArgumentSyntax & syntax,
                                        std::string & out)
{
  out.clear();
  const std::string text = semantic_utils::trim_space(syntax.text);
  if(callsemantic_internal::is_identifier_text(text)) {
    out = text;
    return true;
  }
  if(!syntax.type_id ||
     syntax.type_id->kind != CppAstKind::type_id ||
     syntax.type_id->children.size() != 1 ||
     syntax.type_id->children[0].kind != CppAstKind::type_specifier_seq ||
     syntax.type_id->children[0].children.size() != 1 ||
     syntax.type_id->children[0].children[0].kind != CppAstKind::type_name) {
    return false;
  }
  out = syntax.type_id->children[0].children[0].value;
  return !out.empty();
}

void expand_base_template_argument_syntax_groups(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateIdSyntax & syntax,
    std::size_t expansion_count,
    std::vector<std::vector<TemplateArgumentSyntax> > & out)
{
  out.clear();
  if(expansion_count == 0 ||
     syntax.arguments.size() != syntax.argument_syntaxes.size()) {
    return;
  }

  out.resize(expansion_count);
  bool any_expanded = false;
  for(std::size_t arg_index = 0; arg_index < syntax.arguments.size(); ++arg_index) {
    const TemplateArgumentSyntax & source_arg =
        syntax.argument_syntaxes[arg_index];
    const std::string arg_text =
        semantic_utils::trim_space(syntax.arguments[arg_index]);
    const std::string arg_pattern = arg_text + "...";
    std::vector<std::string> expanded_arg_texts =
        ctx.expand_bound_expression_pack_texts(scope, arg_pattern);
    const bool arg_did_expand =
        expanded_arg_texts.size() == expansion_count &&
        !(expanded_arg_texts.size() == 1 &&
          semantic_utils::trim_space(expanded_arg_texts[0]) == arg_pattern);

    std::vector<TemplateArgumentSyntax> expanded_arg_syntaxes;
    if(arg_did_expand) {
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            expanded_arg_syntaxes =
                template_argument_semantics::expand_type_pack_argument_syntaxes(
                    services,
                    scope,
                    source_arg,
                    expanded_arg_texts);
          });
      any_expanded = true;
    }

    for(std::size_t i = 0; i < expansion_count; ++i) {
      TemplateArgumentSyntax arg_syntax =
          arg_did_expand && i < expanded_arg_syntaxes.size() ?
              expanded_arg_syntaxes[i] :
              source_arg;
      if(arg_did_expand) {
        arg_syntax.text = semantic_utils::trim_space(expanded_arg_texts[i]);
        std::string pack_name;
        const std::vector<TypePtr> * pack = nullptr;
        if(simple_template_argument_pack_name(source_arg, pack_name) &&
           (pack = lookup_bound_type_pack(scope, pack_name)) &&
           pack->size() == expansion_count) {
          arg_syntax.resolved_type = (*pack)[i];
        }
      }
      out[i].push_back(arg_syntax);
    }
  }

  if(!any_expanded) {
    out.clear();
  }
}

std::vector<std::string> expand_base_name_pack_texts(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & base_name,
    std::vector<std::vector<TemplateArgumentSyntax> > * expanded_arg_syntaxes = nullptr,
    bool * expanded_pack = nullptr)
{
  if(expanded_arg_syntaxes) {
    expanded_arg_syntaxes->clear();
  }
  if(expanded_pack) {
    *expanded_pack = false;
  }
  const std::string trimmed = semantic_utils::trim_space(base_name.value);
  if(trimmed.empty()) {
    return std::vector<std::string>();
  }

  const std::string pattern = trimmed + "...";
  std::vector<std::string> expanded = ctx.expand_bound_expression_pack_texts(scope, pattern);
  if(expanded.size() == 1 &&
     semantic_utils::trim_space(expanded[0]) == pattern) {
    return std::vector<std::string>();
  }
  if(expanded_pack) {
    *expanded_pack = true;
  }

  for(std::size_t i = 0; i < expanded.size(); ++i) {
    expanded[i] = semantic_utils::trim_space(expanded[i]);
  }
  if(expanded_arg_syntaxes) {
    if(const TemplateIdSyntax * base_template_syntax =
           cppast_template_id_syntax(base_name)) {
      expand_base_template_argument_syntax_groups(ctx,
                                                  scope,
                                                  *base_template_syntax,
                                                  expanded.size(),
                                                  *expanded_arg_syntaxes);
    }
  }
  return expanded;
}

CppAstNode expanded_base_name_node_with_argument_syntaxes(
    const CppAstNode & source,
    const std::string & expanded_text,
    const std::vector<TemplateArgumentSyntax> & argument_syntaxes)
{
  CppAstNode expanded = source;
  expanded.value = expanded_text;
  const TemplateIdSyntax * source_template_id = cppast_template_id_syntax(source);
  if(!source_template_id) {
    return expanded;
  }

  TemplateIdSyntax template_id = *source_template_id;
  template_id.arguments.clear();
  template_id.argument_syntaxes = argument_syntaxes;
  template_id.arguments.reserve(argument_syntaxes.size());
  for(std::size_t i = 0; i < argument_syntaxes.size(); ++i) {
    template_id.arguments.push_back(
        semantic_utils::trim_space(argument_syntaxes[i].text));
  }
  expanded.template_id_syntax.reset(new TemplateIdSyntax(template_id));
  return expanded;
}

bool node_is_union_class(const CppAstNode & node)
{
  const CppAstNode * class_key = find_child(node, CppAstKind::class_key);
  return class_key && node_text(*class_key) == "union";
}

std::string normalize_special_member_class_name(SemanticContext & ctx,
                                                const std::string & class_name)
{
  std::string normalized = semantic_utils::unqualified_member_name(class_name);
  const std::string stripped =
      semantic_utils::strip_trailing_top_level_template_arguments(normalized);
  if(!stripped.empty()) {
    normalized = stripped;
  }
  return normalized;
}

bool special_member_matches_class_name(SemanticContext & ctx,
                                       const std::string & member_name,
                                       const ClassInfo & info,
                                       bool destructor)
{
  const std::string class_name =
      normalize_special_member_class_name(ctx, info.name);
  if(destructor) {
    if(member_name == std::string("~") + class_name) {
      return true;
    }
    if(member_name.size() > 1 && member_name[0] == '~') {
      const std::string target = member_name.substr(1);
      return semantic_utils::strip_trailing_top_level_template_arguments(target) ==
             class_name;
    }
    return false;
  }
  if(member_name == class_name) {
    return true;
  }
  return semantic_utils::strip_trailing_top_level_template_arguments(member_name) ==
         class_name;
}

std::string constructor_member_name_for_class(SemanticContext & ctx,
                                              const ClassInfo & info)
{
  return normalize_special_member_class_name(ctx, info.name);
}

std::string constructor_lookup_class_name(SemanticContext & ctx,
                                          const std::string & text)
{
  return normalize_special_member_class_name(
      ctx,
      semantic_utils::strip_trailing_top_level_template_arguments(
          semantic_utils::unqualified_member_name(text)));
}

const BaseInfo * find_inherited_constructor_base_by_name(
    SemanticContext & ctx,
    ClassInfo & info,
    const QualifiedName & qualified,
    const std::string & constructor_target_name)
{
  if(qualified.qualifiers.empty()) {
    return nullptr;
  }

  const std::string qualifier_leaf =
      constructor_lookup_class_name(ctx, qualified.qualifiers.back());
  const BaseInfo * match = nullptr;
  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(!base.type || !base.type->type) {
      continue;
    }

    const std::string base_name =
        constructor_lookup_class_name(ctx, base.type->name);
    const std::string base_qualified_name =
        constructor_lookup_class_name(ctx, base.type->qualified_name);
    const bool constructor_names_base =
        constructor_target_name == base_name ||
        constructor_target_name == base_qualified_name;
    const bool qualifier_names_base =
        qualifier_leaf.empty() ||
        qualifier_leaf == base_name ||
        qualifier_leaf == base_qualified_name;
    if(!constructor_names_base || !qualifier_names_base) {
      continue;
    }

    if(match) {
      return nullptr;
    }
    match = &base;
  }
  return match;
}

const BaseInfo * find_inherited_constructor_base(SemanticContext & ctx,
                                                 ClassInfo & info,
                                                 const CppAstNode & node)
{
  const CppAstNode * target = find_child(node, CppAstKind::target);
  if(!target) {
    return nullptr;
  }

  const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
  if(!target_name) {
    return nullptr;
  }

  const QualifiedName qualified = *target_name;
  if(qualified.qualifiers.empty()) {
    return nullptr;
  }

  const std::string constructor_target_name =
      normalize_special_member_class_name(ctx, qualified.name);
  const std::string qualifier_target_name =
      constructor_lookup_class_name(ctx, qualified.qualifiers.back());
  if(constructor_target_name != qualifier_target_name) {
    return nullptr;
  }
  if(const BaseInfo * direct =
         find_inherited_constructor_base_by_name(ctx,
                                                 info,
                                                 qualified,
                                                 constructor_target_name)) {
    return direct;
  }

  TypePtr qualifier_type =
      semantic_lookup::resolve_qualified_owner_type_node(ctx,
                                                         *info.member_scope,
                                                         qualified,
                                                         *target);
  ClassInfo * target_class = ctx.class_info_for_type(qualifier_type);
  if(!target_class) {
    target_class = ctx.complete_class_type(qualifier_type);
  }
  if(!target_class) {
    return find_inherited_constructor_base_by_name(ctx,
                                                   info,
                                                   qualified,
                                                   constructor_target_name);
  }
  bool names_constructor =
      constructor_target_name ==
          normalize_special_member_class_name(ctx, target_class->name) ||
      constructor_target_name ==
          normalize_special_member_class_name(ctx, target_class->qualified_name);
  if(!names_constructor && !qualified.qualifiers.empty()) {
    names_constructor =
        constructor_target_name ==
        normalize_special_member_class_name(ctx, qualified.qualifiers.back());
  }
  if(!names_constructor) {
    return nullptr;
  }

  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(!base.type || !base.type->type) {
      continue;
    }
    if(base.type == target_class ||
       type_equals(base.type->type, target_class->type)) {
      return &base;
    }
  }

  return find_inherited_constructor_base_by_name(ctx,
                                                 info,
                                                 qualified,
                                                 constructor_target_name);
}

std::vector<std::pair<std::string, TypePtr> > inherited_constructor_params(
    const FunctionBinding & base_ctor)
{
  std::vector<std::pair<std::string, TypePtr> > params;
  const std::size_t offset = function_binding_explicit_parameter_offset(base_ctor);
  for(std::size_t i = offset; i < base_ctor.params.size(); ++i) {
    params.push_back(base_ctor.params[i]);
  }
  return params;
}

std::vector<const CppAstNode *> inherited_constructor_default_arguments(
    const FunctionBinding & base_ctor)
{
  std::vector<const CppAstNode *> defaults;
  const std::size_t offset = function_binding_explicit_parameter_offset(base_ctor);
  for(std::size_t i = offset; i < base_ctor.default_arguments.size(); ++i) {
    defaults.push_back(base_ctor.default_arguments[i]);
  }
  return defaults;
}

std::string inherited_constructor_parameter_name(
    const std::vector<std::pair<std::string, TypePtr> > & params,
    std::size_t index)
{
  if(index < params.size() && !params[index].first.empty()) {
    return params[index].first;
  }
  std::ostringstream out;
  out << "__param" << (index + 1);
  return out.str();
}

CppAstNode make_synthetic_type_id_for_type(const TypePtr & type)
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

CppAstNode make_synthetic_rvalue_reference_type_id(const TypePtr & type)
{
  CppAstNode type_id = make_synthetic_type_id_for_type(type);

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

CppAstNode make_inherited_constructor_argument_expression(
    const std::vector<std::pair<std::string, TypePtr> > & params,
    std::size_t index)
{
  CppAstNode id;
  id.kind = CppAstKind::id_expression;
  id.value = inherited_constructor_parameter_name(params, index);

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
      make_synthetic_rvalue_reference_type_id(remove_reference_type(params[index].second)));
  cast.children.push_back(id);
  return cast;
}

CppAstNode make_inherited_constructor_initializer(
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
      base_info.qualified_name : base_info.name;
  mem_initializer_id.semantic_type = base_info.type;
  mem_initializer.children.push_back(mem_initializer_id);

  CppAstNode paren_args;
  paren_args.kind = CppAstKind::paren_argument_list;
  for(std::size_t i = 0; i < params.size(); ++i) {
    paren_args.children.push_back(
        make_inherited_constructor_argument_expression(params, i));
  }
  mem_initializer.children.push_back(paren_args);
  ctor_initializer.children.push_back(mem_initializer);
  return ctor_initializer;
}

bool collect_inherited_constructors(SemanticContext & ctx,
                                    ClassInfo & info,
                                    const CppAstNode & node,
                                    MemberAccess access)
{
  const BaseInfo * base = find_inherited_constructor_base(ctx, info, node);
  if(!base || !base->type || !base->type->member_scope) {
    return false;
  }

  ctx.ensure_class_reference_members(*base->type);
  const std::string base_ctor_name = constructor_member_name_for_class(ctx, *base->type);
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      base->type->methods.find(base_ctor_name);

  const std::string ctor_name = constructor_member_name_for_class(ctx, info);
  if(found != base->type->methods.end()) {
    for(std::size_t i = 0; i < found->second.size(); ++i) {
      FunctionBinding * base_ctor = found->second[i];
      if(!base_ctor || !base_ctor->is_constructor || base_ctor->is_deleted) {
        continue;
      }

      const std::vector<std::pair<std::string, TypePtr> > explicit_params =
          inherited_constructor_params(*base_ctor);
      std::vector<TypePtr> effective_params;
      effective_params.push_back(make_pointer(info.type));
      for(std::size_t j = 0; j < explicit_params.size(); ++j) {
        effective_params.push_back(explicit_params[j].second);
      }
      if(ctx.find_exact_class_function(info,
                                       ctor_name,
                                       make_function(make_fundamental(FT_VOID),
                                                     effective_params,
                                                     false))) {
        continue;
      }

      const CppAstNode * ctor_initializer =
          ctx.own_synthetic_ast(make_inherited_constructor_initializer(*base->type,
                                                                       explicit_params));

      ClassFunctionOptions options;
      options.access = access;
      options.is_constructor = true;
      options.is_inherited_constructor = true;
      options.is_explicit = base_ctor->is_explicit;
      options.is_constexpr = base_ctor->is_constexpr;

      FunctionRegistrationRequest request;
      request.owner_class = &info;
      request.name = ctor_name;
      request.declared_type = base_ctor->declared_type;
      request.params = explicit_params;
      request.default_arguments = inherited_constructor_default_arguments(*base_ctor);
      request.ctor_initializer = ctor_initializer;
      request.declaration_node = &node;
      request.semantic_flags = options;
      FunctionBinding * inherited = ctx.register_function_entity(request);
      if(!inherited) {
        continue;
      }
      inherited->ctor_initializer = ctor_initializer;
      inherited->has_definition = true;
      inherited->is_deleted = false;
      inherited->is_inherited_constructor = true;
      ctx.upgrade_function_symbol_linkage(inherited,
                                          synthesized_class_member_symbol_linkage(info));
    }
  }

  std::vector<FunctionTemplateDecl *> base_constructor_templates =
      lookup_direct_function_templates(*base->type->member_scope, base_ctor_name);
  if(base_constructor_templates.empty()) {
    for(std::map<std::string, std::vector<FunctionTemplateDecl *> >::const_iterator it =
            base->type->member_scope->function_templates.begin();
        it != base->type->member_scope->function_templates.end();
        ++it) {
      for(std::size_t i = 0; i < it->second.size(); ++i) {
        if(it->second[i] && it->second[i]->is_constructor) {
          base_constructor_templates.push_back(it->second[i]);
        }
      }
    }
  }
  for(std::size_t i = 0; i < base_constructor_templates.size(); ++i) {
    FunctionTemplateDecl * base_ctor = base_constructor_templates[i];
    if(!base_ctor || !base_ctor->is_constructor) {
      continue;
    }

    const CppAstNode * ctor_initializer =
        ctx.own_synthetic_ast(make_inherited_constructor_initializer(
            *base->type,
            base_ctor->params_pattern));
    ctx.register_inherited_constructor_template(
        info,
        *base_ctor,
        ctor_name,
        node,
        ctor_initializer,
        access);
  }
  return true;
}

std::string destructor_member_name_for_class(SemanticContext & ctx,
                                             const ClassInfo & info)
{
  return std::string("~") + constructor_member_name_for_class(ctx, info);
}

std::string class_anonymous_union_type_name(std::size_t index)
{
  return std::string("__anonymous_union") + std::to_string(index);
}

std::string class_anonymous_union_storage_name(std::size_t index)
{
  return std::string("__anonymous_union_storage") + std::to_string(index);
}

bool special_member_is_defaulted(const CppAstNode & node)
{
  const CppAstNode * special_definition = find_child(node, CppAstKind::special_definition);
  if(special_definition) {
    return special_definition->value == "default";
  }

  const CppAstNode * initializer = find_child(node, CppAstKind::initializer);
  return initializer &&
         initializer->children.size() == 1 &&
         initializer->children[0].kind == CppAstKind::special_initializer &&
         initializer->children[0].value == "default";
}

bool special_member_is_deleted(const CppAstNode & node)
{
  const CppAstNode * special_definition = find_child(node, CppAstKind::special_definition);
  if(special_definition) {
    return special_definition->value == "delete";
  }

  const CppAstNode * initializer = find_child(node, CppAstKind::initializer);
  return initializer &&
         initializer->children.size() == 1 &&
         initializer->children[0].kind == CppAstKind::special_initializer &&
         initializer->children[0].value == "delete";
}

bool special_member_excluded_from_explicit_instantiation(const CppAstNode & node)
{
  if(node.has_exclude_from_explicit_instantiation) {
    return true;
  }
  const CppAstNode * specifiers = find_child(node, CppAstKind::member_specifiers);
  if(specifiers && specifiers->has_exclude_from_explicit_instantiation) {
    return true;
  }
  const std::string text = node_text(node);
  return text.find("exclude_from_explicit_instantiation") != std::string::npos;
}

bool class_member_declaration_excluded_from_explicit_instantiation(
    const CppAstNode & node)
{
  if(node.has_exclude_from_explicit_instantiation) {
    return true;
  }
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(specifiers && specifiers->has_exclude_from_explicit_instantiation) {
    return true;
  }
  const CppAstNode * member_specifiers = find_child(node, CppAstKind::member_specifiers);
  if(member_specifiers &&
     member_specifiers->has_exclude_from_explicit_instantiation) {
    return true;
  }
  const std::string text = node_text(node);
  return text.find("exclude_from_explicit_instantiation") != std::string::npos;
}

void apply_member_declaration_exclusion(FunctionBinding * binding,
                                        const CppAstNode & declaration)
{
  if(binding) {
    binding->exclude_from_explicit_instantiation =
        binding->exclude_from_explicit_instantiation ||
        class_member_declaration_excluded_from_explicit_instantiation(declaration);
  }
}

const CppAstNode * find_anonymous_union_specifier(const CppAstNode & node)
{
  if(node.kind == CppAstKind::class_specifier && node.value.empty() && node_is_union_class(node)) {
    return &node;
  }
  if(node.kind != CppAstKind::simple_declaration) {
    return nullptr;
  }

  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child(node, CppAstKind::init_declarator_list);
  if(!specifiers || (declarators && !declarators->children.empty())) {
    return nullptr;
  }

  for(size_t i = 0; i < specifiers->children.size(); ++i) {
    const CppAstNode & child = specifiers->children[i];
    if(child.kind == CppAstKind::class_specifier &&
       child.value.empty() &&
       node_is_union_class(child)) {
      return &child;
    }
  }
  return nullptr;
}

std::string anonymous_union_scope_suffix(const CppAstNode & node)
{
  std::ostringstream out;
  out << "__" << node.token_start << "_" << node.token_end;
  return out.str();
}

FieldInfo * find_field_by_name(ClassInfo & info, const std::string & name)
{
  for(std::size_t i = 0; i < info.fields.size(); ++i) {
    if(info.fields[i].name == name) {
      return &info.fields[i];
    }
  }
  return nullptr;
}

std::size_t aggregate_element_count_impl(const ClassInfo & info)
{
  if(info.fields.empty()) {
    return 0;
  }
  return is_union_class_info(info) ? 1 : info.fields.size();
}

const FieldInfo * first_aggregate_field_impl(const ClassInfo & info)
{
  return info.fields.empty() ? nullptr : &info.fields[0];
}

ClassInfo * anonymous_storage_union_info_impl(SemanticContext & ctx,
                                              const FieldInfo & field)
{
  if(!field.is_anonymous_storage) {
    return nullptr;
  }
  ClassInfo * storage_info = ctx.class_info_for_type(field.type);
  return storage_info && storage_info->class_kind == "union" ? storage_info : nullptr;
}

const FieldInfo * aggregate_input_field_impl(SemanticContext & ctx,
                                             const FieldInfo & field)
{
  ClassInfo * storage_info = anonymous_storage_union_info_impl(ctx, field);
  return storage_info ? first_aggregate_field_impl(*storage_info) : &field;
}

TypePtr aggregate_constructor_parameter_type_impl(SemanticContext & ctx,
                                                  const FieldInfo & field)
{
  const FieldInfo * aggregate_field = aggregate_input_field_impl(ctx, field);
  return aggregate_field ? aggregate_field->type : field.type;
}

void sync_anonymous_storage_member_bindings(SemanticContext & ctx, ClassInfo & info)
{
  for(std::map<std::string, ValueBinding>::iterator it = info.member_scope->values.begin();
      it != info.member_scope->values.end();
      ++it) {
    ValueBinding & binding = it->second;
    if(binding.kind != ValueBinding::VK_FIELD ||
       binding.anonymous_storage_field_name.empty()) {
      continue;
    }

    const FieldInfo * storage_field =
        find_field_by_name(info, binding.anonymous_storage_field_name);
    if(!storage_field) {
      continue;
    }

    ClassInfo * storage_info = ctx.class_info_for_type(storage_field->type);
    if(!storage_info || !storage_info->member_scope) {
      continue;
    }

    std::map<std::string, ValueBinding>::const_iterator inner =
        storage_info->member_scope->values.find(binding.name);
    if(inner == storage_info->member_scope->values.end() ||
       inner->second.kind != ValueBinding::VK_FIELD) {
      continue;
    }

    binding.field_offset =
        storage_field->offset + inner->second.field_offset;
    binding.anonymous_storage_member_offset = inner->second.field_offset;
    binding.is_bit_field = inner->second.is_bit_field;
    binding.bit_field_width = inner->second.bit_field_width;
    binding.bit_field_offset = inner->second.bit_field_offset;
    binding.bit_field_storage_size = inner->second.bit_field_storage_size;
  }
}

void inject_anonymous_union_member_bindings(SemanticContext & ctx,
                                            ClassInfo & info,
                                            ClassInfo & storage_info,
                                            const std::string & storage_name,
                                            MemberAccess access)
{
  for(std::map<std::string, ValueBinding>::const_iterator it =
          storage_info.member_scope->values.begin();
      it != storage_info.member_scope->values.end();
      ++it) {
    const ValueBinding & inner = it->second;
    if(inner.kind != ValueBinding::VK_FIELD || inner.name.empty()) {
      continue;
    }
    std::map<std::string, ValueBinding>::const_iterator existing =
        info.member_scope->values.find(inner.name);
    if(existing != info.member_scope->values.end()) {
      if(existing->second.anonymous_storage_field_name == storage_name) {
        continue;
      }
      throw std::logic_error("duplicate anonymous union member " + inner.name);
    }

    ValueBinding alias(ValueBinding::VK_FIELD, inner.name, inner.type);
    alias.owner_class = &info;
    alias.access = combine_member_access(access, inner.access);
    alias.is_mutable = inner.is_mutable;
    alias.declaration_node = inner.declaration_node;
    alias.definition_node = inner.definition_node;
    alias.anonymous_storage_field_name = storage_name;
    alias.anonymous_storage_member_offset = inner.field_offset;
    alias.is_bit_field = inner.is_bit_field;
    alias.bit_field_width = inner.bit_field_width;
    alias.bit_field_offset = inner.bit_field_offset;
    alias.bit_field_storage_size = inner.bit_field_storage_size;
    info.member_scope->values[alias.name] = alias;
  }
}

void collect_anonymous_union_storage(SemanticContext & ctx,
                                     ClassInfo & info,
                                     const CppAstNode & anon,
                                     MemberAccess access,
                                     bool reference_only,
                                     std::size_t ordinal)
{
  const std::string type_name = class_anonymous_union_type_name(ordinal);
  const std::string storage_name = class_anonymous_union_storage_name(ordinal);
  ClassInfo * storage_info =
      ctx.create_class_info(*info.member_scope, "union", type_name, &anon);
  if(!reference_only && anon.kind == CppAstKind::class_specifier && !storage_info->complete) {
    populate_class_info(ctx, *storage_info, anon);
  } else if(reference_only &&
            anon.kind == CppAstKind::class_specifier &&
            !storage_info->reference_members_collected) {
    ensure_class_reference_members(ctx, *storage_info);
  }

  if(!find_field_by_name(info, storage_name)) {
    FieldInfo storage_field;
    storage_field.name = storage_name;
    storage_field.type = storage_info->type;
    storage_field.access = access;
    storage_field.is_anonymous_storage = true;
    info.fields.push_back(storage_field);
  }

  inject_anonymous_union_member_bindings(ctx, info, *storage_info, storage_name, access);
}

void record_anonymous_member_class(ClassInfo & info,
                                   const CppAstNode & node,
                                   const std::string & class_kind)
{
  for(std::size_t i = 0; i < info.anonymous_member_classes.size(); ++i) {
    const AnonymousMemberClassInfo & existing = info.anonymous_member_classes[i];
    if(existing.class_node == &node && existing.class_kind == class_kind) {
      return;
    }
  }
  AnonymousMemberClassInfo member;
  member.class_kind = class_kind;
  member.class_node = &node;
  info.anonymous_member_classes.push_back(member);
}

std::string metrics_class_name(const ClassInfo & info)
{
  if(!info.display_qualified_name.empty()) {
    return info.display_qualified_name;
  }
  if(!info.qualified_name.empty()) {
    return info.qualified_name;
  }
  if(!info.name.empty()) {
    return info.name;
  }
  return "<unnamed>";
}

std::size_t class_member_walk_units(const CppAstNode & node)
{
  std::size_t count = node.children.size();
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if((child.kind == CppAstKind::class_specifier ||
        child.kind == CppAstKind::class_forward_declaration) &&
       child.value.empty() &&
       !node_is_union_class(child)) {
      count += class_member_walk_units(child);
    }
  }
  return count;
}

std::size_t metrics_class_ast_children(const ClassInfo & info)
{
  const CppAstNode * node =
      info.template_output_node ? info.template_output_node : info.class_node;
  return node ? class_member_walk_units(*node) : 0;
}

void trace_class_collection_event(SemanticContext & ctx,
                                  const char * stage,
                                  const ClassInfo & info,
                                  const CppAstNode & node,
                                  const std::string & detail = std::string())
{
  if(!parser_trace::enabled("class.collect")) {
    return;
  }
  std::ostringstream trace;
  trace << stage
        << " class=" << info.qualified_name
        << " complete=" << (info.complete ? "yes" : "no")
        << " ref_members=" << (info.reference_members_collected ? "yes" : "no")
        << " dependent=" << (info.dependent_instantiation ? "yes" : "no")
        << " kind=" << cppast_kind_text(node.kind)
        << " text=" << node_text(node);
  if(!detail.empty()) {
    trace << " " << detail;
  }
  parser_trace::note("class.collect", ctx.source_location_for_node(node), trace.str());
}

bool class_definition_is_replayed_complete_node(const ClassInfo & info,
                                                const CppAstNode & node)
{
  if(!info.class_node) {
    return false;
  }
  return info.class_node == &node ||
         (info.class_node->kind == node.kind &&
          info.class_node->value == node.value &&
          info.class_node->token_start == node.token_start &&
          info.class_node->token_end == node.token_end);
}

TypePtr refine_instantiated_class_alias(SemanticContext & ctx,
                                        Scope & scope,
                                        const TypePtr & alias)
{
  if(!alias || !ctx.type_depends_on_template_parameter(alias)) {
    return alias;
  }

  TypePtr resolved;
  if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, alias, resolved) &&
     resolved &&
     !ctx.type_depends_on_template_parameter(resolved)) {
    return resolved;
  }
  return alias;
}

TypePtr make_dependent_class_alias_placeholder(const ClassInfo & info,
                                               const std::string & alias_name,
                                               const std::string & type_id_text,
                                               const CppAstNode * type_id = nullptr)
{
  TypePtr out =
      make_named(type_id_text.empty() ? alias_name : type_id_text,
                 "dependent alias " + info.qualified_name + "::" + alias_name,
                 true);
  TypePtr base = strip_top_level_cv(out);
  if(base && base->kind == Type::TK_NAMED) {
    if(info.type && !alias_name.empty()) {
      base->named_member_owner_type = info.type;
      base->named_member_name = alias_name;
    }
    if(type_id) {
      base->named_dependent_type_expression_node.reset(new CppAstNode(*type_id));
    }
  }
  return out;
}

TypePtr attach_dependent_alias_type_id_node(const TypePtr & type,
                                            const CppAstNode & type_id)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_ALIAS ||
     base->named_dependent_type_expression_node) {
    return type;
  }
  TypePtr out(new Type(*type));
  TypePtr out_base = strip_top_level_cv(out);
  if(out_base && out_base->kind == Type::TK_NAMED) {
    out_base->named_dependent_type_expression_node.reset(new CppAstNode(type_id));
  }
  return out;
}

bool direct_named_type_depends_on_template_parameter(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  auto found = scope.named_types.find(name);
  if(found != scope.named_types.end() &&
     ctx.type_depends_on_template_parameter(found->second)) {
    return true;
  }
  const std::string unqualified =
      semantic_utils::unqualified_member_name(
          semantic_utils::strip_trailing_top_level_template_arguments(name));
  if(unqualified.empty() || unqualified == name) {
    return false;
  }
  found = scope.named_types.find(unqualified);
  return found != scope.named_types.end() &&
         ctx.type_depends_on_template_parameter(found->second);
}

bool direct_template_name_is_dependent_placeholder(Scope & scope,
                                                   const std::string & name)
{
  if(name.empty()) {
    return false;
  }
  const std::string unqualified =
      semantic_utils::unqualified_member_name(
          semantic_utils::strip_trailing_top_level_template_arguments(name));
  const std::string lookup_name = unqualified.empty() ? name : unqualified;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->template_bound_template_names.count(lookup_name) == 0) {
      continue;
    }
    std::map<std::string, ClassTemplateDecl *>::const_iterator found_class =
        current->class_templates.find(lookup_name);
    std::map<std::string, AliasTemplateDecl *>::const_iterator found_alias =
        current->alias_templates.find(lookup_name);
    return (found_class == current->class_templates.end() &&
            found_alias == current->alias_templates.end()) ||
           (found_class != current->class_templates.end() && !found_class->second) ||
           (found_alias != current->alias_templates.end() && !found_alias->second);
  }
  return false;
}

bool qualified_name_syntax_mentions_dependent_direct_type(
    SemanticContext & ctx,
    Scope & scope,
    const QualifiedName & name)
{
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(direct_named_type_depends_on_template_parameter(ctx,
                                                       scope,
                                                       name.qualifiers[i]) ||
       direct_template_name_is_dependent_placeholder(scope,
                                                     name.qualifiers[i])) {
      return true;
    }
  }
  return direct_named_type_depends_on_template_parameter(ctx, scope, name.name) ||
         direct_template_name_is_dependent_placeholder(scope, name.name);
}

bool type_id_syntax_mentions_dependent_direct_type(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const CppAstNode & node)
{
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    if(qualified_name_syntax_mentions_dependent_direct_type(ctx,
                                                           scope,
                                                           *qualified)) {
      return true;
    }
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(qualified_name_syntax_mentions_dependent_direct_type(ctx,
                                                           scope,
                                                           template_id->name)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(type_id_syntax_mentions_dependent_direct_type(ctx,
                                                     scope,
                                                     node.children[i])) {
      return true;
    }
  }
  return false;
}

bool current_class_qualifier_matches(const ClassInfo & info,
                                     const std::string & qualifier)
{
  const std::string leaf =
      semantic_utils::unqualified_member_name(
          semantic_utils::strip_trailing_top_level_template_arguments(
              semantic_utils::trim_space(qualifier)));
  if(leaf.empty()) {
    return false;
  }
  if(leaf == info.name) {
    return true;
  }
  if(info.source_template && leaf == info.source_template->name) {
    return true;
  }
  const auto matches_unqualified_class_name =
      [&leaf](const std::string & candidate) -> bool
      {
        return !candidate.empty() &&
               leaf == semantic_utils::unqualified_member_name(
                           semantic_utils::strip_trailing_top_level_template_arguments(
                               semantic_utils::trim_space(candidate)));
      };
  return matches_unqualified_class_name(info.qualified_name) ||
         matches_unqualified_class_name(info.display_qualified_name);
}

bool current_class_member_type_is_available_or_deferred(const ClassInfo & info,
                                                        const std::string & name)
{
  const std::string member =
      semantic_utils::unqualified_member_name(
          semantic_utils::strip_trailing_top_level_template_arguments(
              semantic_utils::trim_space(name)));
  if(member.empty() || !info.member_scope) {
    return false;
  }
  return info.member_scope->named_types.count(member) != 0 ||
         info.deferred_member_aliases.count(member) != 0;
}

bool qualified_name_syntax_mentions_current_class_member_type(
    const ClassInfo & info,
    const QualifiedName & name)
{
  return !name.qualifiers.empty() &&
         current_class_qualifier_matches(info, name.qualifiers.back()) &&
         current_class_member_type_is_available_or_deferred(info, name.name);
}

bool template_argument_syntax_mentions_current_class_member_type(
    const ClassInfo & info,
    const TemplateArgumentSyntax & syntax);

bool template_id_syntax_mentions_current_class_member_type(
    const ClassInfo & info,
    const TemplateIdSyntax & syntax)
{
  if(qualified_name_syntax_mentions_current_class_member_type(info, syntax.name)) {
    return true;
  }
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_current_class_member_type(
           info,
           syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < syntax.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_current_class_member_type(
           info,
           syntax.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool type_id_syntax_mentions_current_class_member_type(const ClassInfo & info,
                                                       const CppAstNode & node)
{
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    if(qualified_name_syntax_mentions_current_class_member_type(info,
                                                               *qualified)) {
      return true;
    }
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(template_id_syntax_mentions_current_class_member_type(info, *template_id)) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_current_class_member_type(
           info,
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(type_id_syntax_mentions_current_class_member_type(
           info,
           node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(type_id_syntax_mentions_current_class_member_type(info,
                                                        node.children[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_mentions_current_class_member_type(
    const ClassInfo & info,
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.template_id &&
     template_id_syntax_mentions_current_class_member_type(info,
                                                          *syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     type_id_syntax_mentions_current_class_member_type(info, *syntax.type_id)) {
    return true;
  }
  return syntax.expression &&
         type_id_syntax_mentions_current_class_member_type(info,
                                                          *syntax.expression);
}

bool class_alias_type_id_mentions_current_class_member_type(
    const ClassInfo & info,
    const CppAstNode & type_id)
{
  return !info.complete &&
         (info.reference_member_collection_in_progress ||
          info.full_member_collection_in_progress) &&
         type_id_syntax_mentions_current_class_member_type(info, type_id);
}

TypePtr defer_class_alias_type_id(ClassInfo & info,
                                  const std::string & alias_name,
                                  const CppAstNode & type_id,
                                  const std::string & type_id_text,
                                  bool dependent_class)
{
  ClassInfo::DeferredMemberAlias deferred;
  deferred.type_id = &type_id;
  deferred.type_id_text = type_id_text;
  deferred.dependent_class = dependent_class;
  info.deferred_member_aliases[alias_name] = deferred;
  return make_dependent_class_alias_placeholder(info,
                                                alias_name,
                                                type_id_text,
                                                &type_id);
}

bool class_alias_type_id_is_explicitly_dependent(SemanticContext & ctx,
                                                 const ClassInfo & info,
                                                 const CppAstNode & type_id,
                                                 const std::string & type_id_text,
                                                 bool dependent_class)
{
  if(!dependent_class || !info.member_scope) {
    return false;
  }
  if(type_id_syntax_mentions_dependent_direct_type(ctx,
                                                   *info.member_scope,
                                                   type_id)) {
    return true;
  }
  if(type_id_text.empty()) {
    return false;
  }
  return ctx.text_mentions_template_placeholders(*info.member_scope, type_id_text) ||
         ctx.text_mentions_dependent_non_namespace_binding_names(*info.member_scope,
                                                                 type_id_text) ||
         type_id_text.find("template ") != std::string::npos;
}

bool should_defer_reference_class_alias_type_id(SemanticContext & ctx,
                                                const ClassInfo & info,
                                                const CppAstNode & type_id,
                                                const std::string & type_id_text,
                                                bool dependent_class)
{
  if(witness::source_capture_enabled(ctx.template_witness_context())) {
    return false;
  }
  if(class_alias_type_id_mentions_current_class_member_type(info, type_id)) {
    return true;
  }
  return class_alias_type_id_is_explicitly_dependent(ctx,
                                                     info,
                                                     type_id,
                                                     type_id_text,
                                                     dependent_class);
}

bool class_alias_type_id_contains_decltype_or_typeof(const CppAstNode & node)
{
  if(node.kind == CppAstKind::decltype_specifier) {
    return true;
  }
  const std::string & text = node.value;
  const auto starts_with =
      [&text](const char * prefix) -> bool
      {
        size_t offset = 0;
        while(offset < text.size() &&
              std::isspace(static_cast<unsigned char>(text[offset]))) {
          ++offset;
        }
        const size_t prefix_size = std::strlen(prefix);
        return text.size() >= offset + prefix_size &&
               text.compare(offset, prefix_size, prefix) == 0;
      };
  if(starts_with("decltype") ||
     starts_with("__typeof__") ||
     starts_with("__typeof") ||
     starts_with("__decltype__") ||
     starts_with("__decltype")) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(class_alias_type_id_contains_decltype_or_typeof(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool class_alias_type_id_has_qualified_template_id_syntax(
    const CppAstNode & node)
{
  if(cppast_has_qualifier_template_id_syntaxes(node)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(class_alias_type_id_has_qualified_template_id_syntax(
           node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(class_alias_type_id_has_qualified_template_id_syntax(node.children[i])) {
      return true;
    }
  }
  return false;
}

void clear_class_alias_type_id_semantic_annotations(CppAstNode & node)
{
  node.semantic_type.reset();
  if(node.conversion_type_id_syntax) {
    clear_class_alias_type_id_semantic_annotations(
        *node.conversion_type_id_syntax);
  }
  if(node.base_type_syntax) {
    clear_class_alias_type_id_semantic_annotations(*node.base_type_syntax);
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    clear_class_alias_type_id_semantic_annotations(
        node.qualifier_type_syntaxes[i]);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    clear_class_alias_type_id_semantic_annotations(node.children[i]);
  }
}

bool template_argument_contains_type_pack_element(
    const TemplateArgumentSyntax & argument);

bool template_id_contains_type_pack_element(const TemplateIdSyntax & syntax)
{
  if(syntax.name.name == "__type_pack_element") {
    return true;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_contains_type_pack_element(
           syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool class_alias_type_id_contains_type_pack_element(const CppAstNode & node)
{
  if((node.template_id_syntax &&
      template_id_contains_type_pack_element(*node.template_id_syntax)) ||
     (node.conversion_type_id_syntax &&
      class_alias_type_id_contains_type_pack_element(
          *node.conversion_type_id_syntax)) ||
     (node.base_type_syntax &&
      class_alias_type_id_contains_type_pack_element(*node.base_type_syntax))) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_contains_type_pack_element(
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(class_alias_type_id_contains_type_pack_element(
           node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(class_alias_type_id_contains_type_pack_element(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_contains_type_pack_element(
    const TemplateArgumentSyntax & argument)
{
  return (argument.template_id &&
          template_id_contains_type_pack_element(*argument.template_id)) ||
         (argument.type_id &&
          class_alias_type_id_contains_type_pack_element(*argument.type_id)) ||
         (argument.expression &&
          class_alias_type_id_contains_type_pack_element(*argument.expression));
}

bool class_alias_type_id_text_mentions_decltype_or_typeof(
    const std::string & type_id_text)
{
  return type_id_text.find("decltype") != std::string::npos ||
         type_id_text.find("__typeof") != std::string::npos ||
         type_id_text.find("__decltype") != std::string::npos;
}

bool class_constant_noexcept_operand_can_use_template_fallback(
    const CppAstNode & operand)
{
  const CppAstNode * current = &operand;
  while(current->kind == CppAstKind::parenthesized_expression &&
        current->children.size() == 1) {
    current = &current->children[0];
  }
  return current->kind == CppAstKind::call_expression;
}

bool class_constant_noexcept_operand_is_well_formed(SemanticContext & ctx,
                                                    Scope & scope,
                                                    const CppAstNode & operand)
{
  TypePtr operand_type;
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_api::TemplateDependentTypeExprRequest request;
        request.scope = &scope;
        request.kind = template_api::TDTEK_DECLTYPE;
        request.operand_was_parenthesized = false;
        request.operand = operand;
        return (template_argument_semantics::evaluate_dependent_type_expression_leaf(
                    services,
                    scope,
                    request,
                    operand_type) ||
                services.recursive_semantic.evaluate_dependent_type_expression(
                    request,
                    operand_type)) &&
               operand_type;
      });
}

TypePtr parse_or_defer_class_alias_type_id(SemanticContext & ctx,
                                           ClassInfo & info,
                                           const std::string & alias_name,
                                           const CppAstNode & type_id,
                                           const std::string & type_id_text,
                                           bool dependent_class)
{
  CppAstNode substituted_type_id;
  CppAstNode expanded_type_id;
  const CppAstNode * type_id_for_parse = &type_id;
  if(info.member_scope) {
    template_api::with_template_services(
        ctx,
        [&](template_api::TemplateServices & services)
        {
          const std::vector<template_model::TemplateParameterInfo> * parameters =
              nullptr;
          const std::vector<template_model::TemplateArgument> * arguments = nullptr;
          class_template_member_substitution_bindings(info, parameters, arguments);
          const bool contains_type_pack_element =
              class_alias_type_id_contains_type_pack_element(type_id);
          const bool contains_pack_expansion =
              template_argument_semantics::type_id_node_contains_pack_expansion_syntax(
                  type_id);
          const bool has_fully_bound_class_template_arguments =
              parameters &&
              arguments &&
              template_model::template_arguments_fully_bind_parameters(
                  *parameters, *arguments);
          const bool can_substitute_class_template_arguments =
              parameters &&
              arguments &&
              (contains_type_pack_element ||
               (contains_pack_expansion &&
                has_fully_bound_class_template_arguments));
          if(can_substitute_class_template_arguments) {
            template_api::binding::bind_template_arguments_into_scope(
                ctx,
                *info.member_scope,
                *parameters,
                *arguments,
                info.has_instantiation_binding_arguments ?
                    &info.instantiation_binding_pack_sizes : nullptr);
          }
          if(can_substitute_class_template_arguments &&
             template_argument_semantics::substitute_type_id_node_for_template_arguments(
                 services,
                 *info.member_scope,
                 type_id,
                 *parameters,
                 *arguments,
                 substituted_type_id)) {
            if(class_alias_type_id_has_qualified_template_id_syntax(
                   substituted_type_id)) {
              clear_class_alias_type_id_semantic_annotations(
                  substituted_type_id);
            }
            type_id_for_parse = &substituted_type_id;
          } else if(template_argument_semantics::expand_bound_packs_in_type_id_node(
                 services,
                 *info.member_scope,
                 type_id,
                 expanded_type_id)) {
            type_id_for_parse = &expanded_type_id;
          }
          return true;
        });
  }
  if(!witness::source_capture_enabled(ctx.template_witness_context()) &&
     class_alias_type_id_mentions_current_class_member_type(info,
                                                           *type_id_for_parse)) {
    return defer_class_alias_type_id(info,
                                     alias_name,
                                     type_id,
                                     type_id_text,
                                     dependent_class);
  }
  TypePtr alias;
  bool parsed_alias = false;
  const bool substituted_qualified_template_id =
      type_id_for_parse == &substituted_type_id &&
      class_alias_type_id_has_qualified_template_id_syntax(*type_id_for_parse);
  const bool alias_needs_template_parse =
      substituted_qualified_template_id ||
      (!type_id_text.empty() ?
          class_alias_type_id_text_mentions_decltype_or_typeof(type_id_text) :
          class_alias_type_id_contains_decltype_or_typeof(*type_id_for_parse));
  if(alias_needs_template_parse) {
    parsed_alias =
        template_api::type::parse_type_id_node_for_templates(ctx,
                                                             *info.member_scope,
                                                             *type_id_for_parse,
                                                             alias,
                                                             true) &&
        alias;
  }
  if(!parsed_alias) {
    parsed_alias =
        ctx.parse_type_id(*info.member_scope, *type_id_for_parse, alias, true) &&
        alias;
  }
  if(parsed_alias && alias) {
    try {
      TypePtr canonical =
          canonicalize_member_typedef_type(ctx, *info.member_scope, alias, &info);
      const bool explicitly_dependent =
          class_alias_type_id_is_explicitly_dependent(ctx,
                                                      info,
                                                      type_id,
                                                      type_id_text,
                                                      dependent_class);
      if(explicitly_dependent &&
         canonical &&
         !ctx.type_depends_on_template_parameter(canonical) &&
         !type_is_complete(canonical)) {
        return make_dependent_class_alias_placeholder(info,
                                                      alias_name,
                                                      type_id_text,
                                                      type_id_for_parse);
      }
      if(explicitly_dependent &&
         canonical &&
         ctx.type_depends_on_template_parameter(canonical)) {
        return attach_dependent_alias_type_id_node(canonical, *type_id_for_parse);
      }
      return canonical;
    } catch(const TemplateSubstitutionFailure &) {
      if(!class_alias_type_id_is_explicitly_dependent(ctx,
                                                      info,
                                                      type_id,
                                                      type_id_text,
                                                      dependent_class)) {
        throw;
      }
    }
  }

  if(class_alias_type_id_is_explicitly_dependent(ctx,
                                                 info,
                                                 type_id,
                                                 type_id_text,
                                                 dependent_class)) {
    return make_dependent_class_alias_placeholder(info,
                                                  alias_name,
                                                  type_id_text,
                                                  type_id_for_parse);
  }

  return TypePtr();
}

TypePtr defer_reference_class_alias_type_id(SemanticContext & ctx,
                                            ClassInfo & info,
                                            const std::string & alias_name,
                                            const CppAstNode & type_id,
                                            const std::string & type_id_text,
                                            bool dependent_class)
{
  if(!should_defer_reference_class_alias_type_id(ctx,
                                                 info,
                                                 type_id,
                                                 type_id_text,
                                                 dependent_class)) {
    return TypePtr();
  }
  ClassInfo::DeferredMemberAlias deferred;
  deferred.type_id = &type_id;
  deferred.type_id_text = type_id_text;
  deferred.dependent_class = dependent_class;
  info.deferred_member_aliases[alias_name] = deferred;
  return make_dependent_class_alias_placeholder(info,
                                                alias_name,
                                                type_id_text,
                                                &type_id);
}

TypePtr parse_or_defer_reference_class_alias_type_id(SemanticContext & ctx,
                                                     ClassInfo & info,
                                                     const std::string & alias_name,
                                                     const CppAstNode & type_id,
                                                     const std::string & type_id_text,
                                                     bool dependent_class)
{
  if(TypePtr deferred = defer_reference_class_alias_type_id(ctx,
                                                           info,
                                                           alias_name,
                                                           type_id,
                                                           type_id_text,
                                                           dependent_class)) {
    return deferred;
  }
  try {
    return parse_or_defer_class_alias_type_id(ctx,
                                              info,
                                              alias_name,
                                              type_id,
                                              type_id_text,
                                              dependent_class);
  } catch(const std::logic_error & e) {
    const std::string message = e.what();
    if(info.reference_member_collection_in_progress &&
       info.source_template &&
       reference_collection_can_defer_alias_failure(message)) {
      ClassInfo::DeferredMemberAlias deferred;
      deferred.type_id = &type_id;
      deferred.type_id_text = type_id_text;
      deferred.dependent_class = dependent_class;
      info.deferred_member_aliases[alias_name] = deferred;
      return make_dependent_class_alias_placeholder(info,
                                                    alias_name,
                                                    type_id_text,
                                                    &type_id);
    }
    throw;
  }
}

void trace_class_alias_store(SemanticContext & ctx,
                             const ClassInfo & info,
                             const char * stage,
                             const std::string & name,
                             const std::string & type_id_text,
                             const TypePtr & alias)
{
  if(!parser_trace::enabled("template.resolve")) {
    return;
  }

  std::ostringstream trace;
  trace << "store-class-alias stage=" << stage
        << " class=" << info.qualified_name
        << " name=" << name
        << " type-id=" << type_id_text
        << " stored=" << (alias ? describe_type(alias) : std::string("<none>"))
        << " dependent="
        << ((alias && ctx.type_depends_on_template_parameter(alias)) ? "yes" : "no");
  parser_trace::note("template.resolve", std::string(), trace.str());
}

void collect_class_reference_special_member(SemanticContext & ctx,
                                            ClassInfo & info,
                                            const CppAstNode & node,
                                            MemberAccess access);

void collect_class_reference_method_definition(SemanticContext & ctx,
                                               ClassInfo & info,
                                               const CppAstNode & node,
                                               MemberAccess access);

size_t align_up(size_t value, size_t alignment)
{
  if(alignment <= 1) {
    return value;
  }
  const size_t remainder = value % alignment;
  return remainder == 0 ? value : value + (alignment - remainder);
}

bool build_alignment_expression_from_type_id(const CppAstNode & type_id,
                                             CppAstNode & out)
{
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.empty() ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq ||
     type_id.children[0].children.size() != 1 ||
     type_id.children[0].children[0].kind != CppAstKind::type_name) {
    return false;
  }

  const CppAstNode & type_name = type_id.children[0].children[0];
  const QualifiedName * qualified = cppast_qualified_name_syntax(type_name);
  if(type_name.has_leading_typename ||
     !qualified ||
     (!qualified->rooted && qualified->qualifiers.empty())) {
    return false;
  }

  out = CppAstNode();
  out.kind = CppAstKind::id_expression;
  out.value = type_name.value;
  out.qualified_name_syntax = type_name.qualified_name_syntax;
  out.template_id_syntax = type_name.template_id_syntax;
  out.qualifier_template_id_syntaxes = type_name.qualifier_template_id_syntaxes;
  out.qualifier_type_syntaxes = type_name.qualifier_type_syntaxes;
  out.token_start = type_name.token_start;
  out.token_end = type_name.token_end;
  out.source_location_id = type_name.source_location_id;
  return true;
}

void maybe_complete_alignment_operand_type(SemanticContext & ctx,
                                           const TypePtr & type)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return;
  }
  if(base->kind == Type::TK_ARRAY) {
    maybe_complete_alignment_operand_type(ctx, base->inner);
    return;
  }
  if(base->kind == Type::TK_NAMED && !base->named_has_layout) {
    ctx.complete_class_type(base);
  }
}

size_t evaluate_declared_alignment(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode * node)
{
  if(node == nullptr || node->alignment_specifiers.empty()) {
    return 0;
  }

  size_t out = 0;
  for(size_t i = 0; i < node->alignment_specifiers.size(); ++i) {
    const std::string text = semantic_utils::trim_space(node->alignment_specifiers[i]);
    if(text.empty()) {
      continue;
    }

    const CppAstNode * syntax =
        (i < node->alignment_specifier_nodes.size() &&
         node->alignment_specifier_nodes[i].kind != CppAstKind::invalid) ?
            &node->alignment_specifier_nodes[i] :
            nullptr;
    if(syntax && syntax->kind == CppAstKind::type_id) {
      TypePtr alignment_type;
      if(ctx.parse_type_id(scope, *syntax, alignment_type)) {
        maybe_complete_alignment_operand_type(ctx, alignment_type);
        out = std::max(out, cpp_decl::type_alignment(alignment_type));
        continue;
      }
    }

    long long value = 0;
    CppAstNode expression_syntax;
    const CppAstNode * value_syntax = syntax;
    if(syntax &&
       syntax->kind == CppAstKind::type_id &&
       build_alignment_expression_from_type_id(*syntax, expression_syntax)) {
      value_syntax = &expression_syntax;
    }
    if(value_syntax && ctx.evaluate_constant_expression(scope, *value_syntax, value)) {
      if(value <= 0 || (value & (value - 1)) != 0) {
        throw std::logic_error("alignas requires positive power-of-two alignment");
      }
      out = std::max(out, static_cast<size_t>(value));
      continue;
    }

    std::ostringstream outmsg;
    outmsg << "unsupported alignas(" << text << ")";
    throw std::logic_error(outmsg.str());
  }

  return out;
}

TypePtr const_lvalue_reference_to(const TypePtr & type)
{
  return make_lvalue_reference_raw(make_cv(type, true, false));
}

bool class_derives_from(const ClassInfo & derived,
                        const ClassInfo & base);

bool covariant_return_type_compatible(SemanticContext & ctx,
                                      const TypePtr & derived_return,
                                      const TypePtr & base_return)
{
  if(type_equals(derived_return, base_return)) {
    return true;
  }

  TypePtr derived_base = strip_top_level_cv(derived_return);
  TypePtr base_base = strip_top_level_cv(base_return);
  if(!derived_base || !base_base) {
    return false;
  }

  const bool pointer_form =
      derived_base->kind == Type::TK_POINTER &&
      base_base->kind == Type::TK_POINTER;
  const bool lvalue_ref_form =
      derived_base->kind == Type::TK_LVALUE_REFERENCE &&
      base_base->kind == Type::TK_LVALUE_REFERENCE;
  if(!pointer_form && !lvalue_ref_form) {
    return false;
  }

  TypePtr derived_object = strip_top_level_cv(derived_base->inner);
  TypePtr base_object = strip_top_level_cv(base_base->inner);
  if(!derived_object || !base_object) {
    return false;
  }

  ClassInfo * derived_class = ctx.complete_class_type(derived_object);
  if(!derived_class) {
    derived_class = ctx.class_info_for_type(derived_object);
  }
  ClassInfo * base_class = ctx.complete_class_type(base_object);
  if(!base_class) {
    base_class = ctx.class_info_for_type(base_object);
  }
  if(!derived_class || !base_class) {
    return false;
  }

  if(derived_class != base_class &&
     !class_derives_from(*derived_class, *base_class)) {
    return false;
  }

  TypePtr derived_object_base;
  TypePtr base_object_base;
  bool derived_const = false;
  bool derived_volatile = false;
  bool base_const = false;
  bool base_volatile = false;
  if(!top_level_cv_flags(derived_base->inner,
                         derived_object_base,
                         derived_const,
                         derived_volatile) ||
     !top_level_cv_flags(base_base->inner,
                         base_object_base,
                         base_const,
                         base_volatile)) {
    return false;
  }
  return derived_const == base_const &&
         derived_volatile == base_volatile;
}

bool override_return_types_match(SemanticContext & ctx,
                                 const TypePtr & lhs_return,
                                 const TypePtr & rhs_return)
{
  return covariant_return_type_compatible(ctx, lhs_return, rhs_return) ||
         covariant_return_type_compatible(ctx, rhs_return, lhs_return);
}

bool same_virtual_signature(SemanticContext & ctx,
                            const FunctionBinding & lhs,
                            const FunctionBinding & rhs)
{
  if(lhs.is_constructor || rhs.is_constructor) {
    return false;
  }
  if(lhs.is_destructor || rhs.is_destructor) {
    return lhs.is_destructor && rhs.is_destructor;
  }
  TypePtr lhs_type = strip_top_level_cv(lhs.declared_type);
  TypePtr rhs_type = strip_top_level_cv(rhs.declared_type);
  if(!lhs_type || !rhs_type ||
     lhs_type->kind != Type::TK_FUNCTION ||
     rhs_type->kind != Type::TK_FUNCTION) {
    return false;
  }
  if(lhs_type->params.size() != rhs_type->params.size() ||
     lhs_type->variadic != rhs_type->variadic ||
     lhs_type->prototype_relaxed != rhs_type->prototype_relaxed) {
    return false;
  }
  for(size_t i = 0; i < lhs_type->params.size(); ++i) {
    if(!type_equals(lhs_type->params[i], rhs_type->params[i])) {
      return false;
    }
  }
  return lhs.display_name == rhs.display_name &&
         lhs.is_const_method == rhs.is_const_method &&
         lhs.is_volatile_method == rhs.is_volatile_method &&
         lhs.ref_qualifier == rhs.ref_qualifier &&
         override_return_types_match(ctx, lhs_type->inner, rhs_type->inner);
}

bool has_secondary_virtual_destructor_slot(const FunctionBinding & binding)
{
  return binding.is_destructor;
}

template <typename ClassPtr>
void push_inherited_bases(const ClassInfo & info,
                          std::vector<ClassPtr> & stack,
                          std::set<ClassPtr> & seen_virtual)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    ClassPtr next = info.bases[i].type;
    if(info.bases[i].is_virtual && !seen_virtual.insert(next).second) {
      continue;
    }
    stack.push_back(next);
  }
}

template <typename ClassPtr, typename Visitor>
bool walk_inherited_bases(ClassPtr root, Visitor visit)
{
  std::vector<ClassPtr> stack;
  std::set<ClassPtr> seen_virtual;
  push_inherited_bases(*root, stack, seen_virtual);
  while(!stack.empty()) {
    ClassPtr current = stack.back();
    stack.pop_back();
    if(!visit(current)) {
      return false;
    }
    push_inherited_bases(*current, stack, seen_virtual);
  }
  return true;
}

void collect_direct_virtual_bases(const ClassInfo & info,
                                  std::vector<ClassInfo *> & out,
                                  std::set<ClassInfo *> & seen)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(base.is_virtual && seen.insert(base.type).second) {
      out.push_back(base.type);
    }
  }
}

ClassInfo * primary_polymorphic_base(ClassInfo & info);
bool class_derives_from(const ClassInfo & derived,
                        const ClassInfo & base);

bool class_derives_from(const ClassInfo & derived,
                        const ClassInfo & base)
{
  bool found = false;
  walk_inherited_bases(&derived, [&](const ClassInfo * current) {
    if(current == &base) {
      found = true;
      return false;
    }
    return true;
  });
  return found;
}

bool virtual_function_overrides(SemanticContext & ctx,
                                const FunctionBinding & overriding,
                                const FunctionBinding & overridden)
{
  if(&overriding == &overridden) {
    return true;
  }
  if(!same_virtual_signature(ctx, overriding, overridden)) {
    return false;
  }
  if(!overriding.owner_class || !overridden.owner_class) {
    return true;
  }
  return overriding.owner_class == overridden.owner_class ||
         class_derives_from(*overriding.owner_class, *overridden.owner_class);
}

bool owner_on_primary_polymorphic_path(const ClassInfo & info,
                                       const FunctionBinding & binding)
{
  if(!binding.owner_class) {
    return false;
  }
  ClassInfo * primary_base = primary_polymorphic_base(const_cast<ClassInfo &>(info));
  if(!primary_base) {
    return false;
  }
  return binding.owner_class == primary_base ||
         class_derives_from(*primary_base, *binding.owner_class);
}

FunctionBinding * prefer_unrelated_destructor_override(const ClassInfo & info,
                                                       FunctionBinding * lhs,
                                                       FunctionBinding * rhs)
{
  const bool lhs_primary_path = owner_on_primary_polymorphic_path(info, *lhs);
  const bool rhs_primary_path = owner_on_primary_polymorphic_path(info, *rhs);
  if(lhs_primary_path != rhs_primary_path) {
    return lhs_primary_path ? lhs : rhs;
  }
  if(lhs->has_virtual_slot != rhs->has_virtual_slot) {
    return lhs->has_virtual_slot ? lhs : rhs;
  }
  if(lhs->has_virtual_slot &&
     rhs->has_virtual_slot &&
     lhs->virtual_slot != rhs->virtual_slot) {
    return lhs->virtual_slot < rhs->virtual_slot ? lhs : rhs;
  }
  return lhs;
}

FunctionBinding * find_overridden_virtual(SemanticContext & ctx,
                                          ClassInfo & info,
                                          const FunctionBinding & binding)
{
  FunctionBinding * match = nullptr;
  const bool trace_virtuals = parser_trace::enabled("class.collect");
  walk_inherited_bases(&info, [&](ClassInfo * current) {
    for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it = current->methods.begin();
        it != current->methods.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        FunctionBinding * candidate = it->second[i];
        if(!candidate->is_virtual ||
           !virtual_function_overrides(ctx, binding, *candidate)) {
          continue;
        }
        if(trace_virtuals) {
          std::ostringstream trace;
          trace << "find-overridden-virtual class=" << info.qualified_name
                << " binding=" << binding.name
                << " owner=" << (binding.owner_class ? binding.owner_class->qualified_name :
                                 std::string("<none>"))
                << " candidate=" << candidate->name
                << " candidate_owner="
                << (candidate->owner_class ? candidate->owner_class->qualified_name :
                    std::string("<none>"))
                << " candidate_slot="
                << (candidate->has_virtual_slot ?
                        std::to_string(candidate->virtual_slot) :
                        std::string("<none>"));
          if(match) {
            trace << " existing_owner="
                  << (match->owner_class ? match->owner_class->qualified_name :
                      std::string("<none>"))
                  << " existing_slot="
                  << (match->has_virtual_slot ?
                          std::to_string(match->virtual_slot) :
                          std::string("<none>"));
          }
          parser_trace::note("class.collect", std::string(), trace.str());
        }
        if(match && match != candidate) {
          if(match->has_virtual_slot && candidate->has_virtual_slot &&
             match->virtual_slot == candidate->virtual_slot) {
            continue;
          }
          if(match->owner_class == candidate->owner_class) {
            if(candidate->has_virtual_slot &&
               (!match->has_virtual_slot ||
                candidate->virtual_slot < match->virtual_slot)) {
              match = candidate;
            }
            continue;
          }
          if(match->owner_class && candidate->owner_class) {
            if(class_derives_from(*match->owner_class, *candidate->owner_class)) {
              continue;
            }
            if(class_derives_from(*candidate->owner_class, *match->owner_class)) {
              match = candidate;
              continue;
            }
          }
          if(binding.is_destructor &&
             match->is_destructor &&
             candidate->is_destructor) {
            match = prefer_unrelated_destructor_override(info, match, candidate);
            continue;
          }
          std::ostringstream out;
          out << "ambiguous virtual override"
              << " [binding " << binding.display_name
              << " in " << info.qualified_name
              << "] [match " << match->name;
          out << " declared "
              << (match->declared_type ?
                      cpp_decl::describe_type(match->declared_type) :
                      std::string("<special-member>"));
          out << semantic_trace::previous_function_location_note(ctx, "match", match);
          out << "] [candidate " << candidate->name;
          out << " declared "
              << (candidate->declared_type ?
                      cpp_decl::describe_type(candidate->declared_type) :
                      std::string("<special-member>"));
          out << semantic_trace::previous_function_location_note(
              ctx, "candidate", candidate);
          out << "]";
          throw std::logic_error(out.str());
        }
        match = candidate;
      }
    }
    return true;
  });
  return match;
}

ClassInfo * primary_polymorphic_base(ClassInfo & info)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].is_virtual && info.bases[i].type->is_polymorphic) {
      return info.bases[i].type;
    }
  }
  return nullptr;
}

void collect_unique_virtual_bases(const ClassInfo & info,
                                  std::vector<ClassInfo *> & out,
                                  std::set<ClassInfo *> & seen)
{
  collect_direct_virtual_bases(info, out, seen);
  walk_inherited_bases(&info, [&](const ClassInfo * current) {
    collect_direct_virtual_bases(*current, out, seen);
    return true;
  });
}

void append_nonvirtual_subobjects(const ClassInfo & info,
                                  size_t offset,
                                  MemberAccess access,
                                  bool via_virtual,
                                  std::vector<SubobjectInfo> & out)
{
  SubobjectInfo self;
  self.type = const_cast<ClassInfo *>(&info);
  self.offset = offset;
  self.access = access;
  self.is_virtual = via_virtual;
  out.push_back(self);

  for(size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(base.is_virtual) {
      continue;
    }
    append_nonvirtual_subobjects(*base.type,
                                 offset + base.offset,
                                 combine_member_access(access, base.access),
                                 via_virtual,
                                 out);
  }
}

bool same_subobject_type_identity(const ClassInfo * lhs, const ClassInfo * rhs)
{
  return lhs == rhs ||
         (lhs && rhs && lhs->qualified_name == rhs->qualified_name);
}

bool placement_conflicts_same_type_subobject(const std::vector<SubobjectInfo> & placed,
                                             const ClassInfo & candidate,
                                             size_t candidate_offset)
{
  const auto conflicts_with_existing =
      [&](const ClassInfo * type, size_t offset) -> bool
      {
        if(!type) {
          return false;
        }
        for(size_t i = 0; i < placed.size(); ++i) {
          if(placed[i].offset == offset &&
             same_subobject_type_identity(placed[i].type, type)) {
            return true;
          }
        }
        return false;
      };

  if(candidate.complete_subobjects.empty()) {
    return conflicts_with_existing(&candidate, candidate_offset);
  }

  for(size_t i = 0; i < candidate.complete_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = candidate.complete_subobjects[i];
    if(subobject.is_virtual) {
      continue;
    }
    if(conflicts_with_existing(subobject.type, candidate_offset + subobject.offset)) {
      return true;
    }
  }
  return false;
}

void record_placed_nonvirtual_subobjects(std::vector<SubobjectInfo> & placed,
                                         const ClassInfo & type,
                                         size_t base_offset,
                                         MemberAccess access)
{
  if(type.complete_subobjects.empty()) {
    SubobjectInfo self;
    self.type = const_cast<ClassInfo *>(&type);
    self.offset = base_offset;
    self.access = access;
    self.is_virtual = false;
    placed.push_back(self);
    return;
  }

  for(size_t i = 0; i < type.complete_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = type.complete_subobjects[i];
    if(subobject.is_virtual || !subobject.type) {
      continue;
    }
    SubobjectInfo placed_subobject = subobject;
    placed_subobject.offset += base_offset;
    placed_subobject.access = combine_member_access(access, subobject.access);
    placed_subobject.is_virtual = false;
    placed.push_back(placed_subobject);
  }
}

std::string vtable_view_key(const ClassInfo & dynamic_class,
                            const ClassInfo & view_class,
                            size_t offset)
{
  if(offset == 0) {
    return dynamic_class.qualified_name;
  }
  std::ostringstream out;
  out << dynamic_class.qualified_name << "::__view__" << view_class.qualified_name
      << "__" << offset;
  return out.str();
}

FunctionBinding * find_final_overrider(SemanticContext & ctx,
                                       ClassInfo & dynamic_class,
                                       FunctionBinding & base_virtual)
{
  FunctionBinding * found = &base_virtual;
  for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it =
          dynamic_class.methods.begin();
      it != dynamic_class.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(binding != &base_virtual &&
         virtual_function_overrides(ctx, *binding, base_virtual)) {
        found = binding;
      }
    }
  }
  walk_inherited_bases(&dynamic_class, [&](ClassInfo * current) {
    for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it =
            current->methods.begin();
        it != current->methods.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        FunctionBinding * binding = it->second[i];
        if(binding != &base_virtual &&
           virtual_function_overrides(ctx, *binding, base_virtual)) {
          if(found == &base_virtual) {
            if(found->owner_class && binding->owner_class &&
               class_derives_from(*found->owner_class, *binding->owner_class)) {
              continue;
            }
            found = binding;
            continue;
          }
          if(found->owner_class == binding->owner_class) {
            if(binding->has_virtual_slot &&
               (!found->has_virtual_slot ||
                binding->virtual_slot < found->virtual_slot)) {
              found = binding;
            }
            continue;
          }
          if(found->owner_class && binding->owner_class) {
            if(class_derives_from(*found->owner_class, *binding->owner_class)) {
              continue;
            }
            if(class_derives_from(*binding->owner_class, *found->owner_class)) {
              found = binding;
              continue;
            }
          }
          if(base_virtual.is_destructor &&
             found->is_destructor &&
             binding->is_destructor) {
            found = prefer_unrelated_destructor_override(dynamic_class, found, binding);
            continue;
          }
          std::ostringstream out;
          out << "ambiguous final overrider"
              << " [binding " << base_virtual.display_name
              << " in " << dynamic_class.qualified_name
              << "] [match " << found->name;
          out << " declared "
              << (found->declared_type ?
                      cpp_decl::describe_type(found->declared_type) :
                      std::string("<special-member>"));
          out << semantic_trace::previous_function_location_note(ctx, "match", found);
          out << "] [candidate " << binding->name;
          out << " declared "
              << (binding->declared_type ?
                      cpp_decl::describe_type(binding->declared_type) :
                      std::string("<special-member>"));
          out << semantic_trace::previous_function_location_note(
              ctx, "candidate", binding);
          out << "]";
          throw std::logic_error(out.str());
        }
      }
    }
    return true;
  });
  return found;
}

std::string diagnostic_location_for_member(SemanticContext & ctx,
                                           const CppAstNode & primary,
                                           const CppAstNode * fallback = nullptr)
{
  const std::string primary_location = ctx.source_location_for_node(primary);
  if(!primary_location.empty()) {
    return primary_location;
  }
  return fallback ? ctx.source_location_for_node(*fallback) : std::string();
}

bool type_allows_implicit_default_initialization(SemanticContext & ctx,
                                                 Scope & scope,
                                                 const TypePtr & type,
                                                 std::set<ClassInfo *> & visiting);

bool implicit_default_constructor_is_deleted_impl(SemanticContext & ctx,
                                                  ClassInfo & info,
                                                  std::set<ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return false;
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_implicit_default_initialization(ctx,
                                                    *info.member_scope,
                                                    info.bases[i].type->type,
                                                    visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    const FieldInfo & field = info.fields[i];
    if(field.default_initializer) {
      continue;
    }
    if(!type_allows_implicit_default_initialization(ctx,
                                                    *info.member_scope,
                                                    field.type,
                                                    visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  visiting.erase(&info);
  return false;
}

bool type_allows_implicit_default_initialization(SemanticContext & ctx,
                                                 Scope & scope,
                                                 const TypePtr & type,
                                                 std::set<ClassInfo *> & visiting)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return false;
  }

  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_implicit_default_initialization(ctx, scope, base->inner, visiting);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    if(visiting.count(info)) {
      return true;
    }
    semantic_class_model::ensure_implicit_special_members(ctx, *info);
    std::vector<ExprInfo> args_out;
    std::vector<ExprInfo> source_args;
    try
    {
      const ConstructorSelectionOptions ctor_options =
          constructor_lifecycle_service::selection_options_for(
              constructor_lifecycle_service::implicit_default_constructor_viability_profile(
                  "implicit default constructor viability"));
      FunctionBinding * ctor =
          ctx.select_constructor_from_exprs(scope,
                                            *info,
                                            source_args,
                                            args_out,
                                            nullptr,
                                            ctor_options);
      return ctor != nullptr && !ctor->is_deleted;
    }
    catch(const std::logic_error &)
    {
      return false;
    }
  }

  return !type_is_const_object(type);
}

bool implicit_default_constructor_is_deleted(SemanticContext & ctx,
                                             ClassInfo & info)
{
  if(semantic_hotspot::enabled()) {
    semantic_hotspot::note_semantic_query("implicit_default_constructor_is_deleted",
                                          info.qualified_name);
  }
  std::set<ClassInfo *> visiting;
  return implicit_default_constructor_is_deleted_impl(ctx, info, visiting);
}

bool is_same_class_reference_parameter(const TypePtr & class_type,
                                       const TypePtr & param_type,
                                       Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(param_type);
  if(!base || base->kind != ref_kind) {
    return false;
  }
  return same_type_with_compatible_top_cv(base->inner, class_type);
}

bool has_trailing_constructor_defaults(const FunctionBinding & binding,
                                       size_t first_param)
{
  for(size_t i = first_param; i < binding.params.size(); ++i) {
    if(i >= binding.default_arguments.size() || !binding.default_arguments[i]) {
      return false;
    }
  }
  return true;
}

bool is_same_class_ref_constructor_binding(const ClassInfo & info,
                                           const FunctionBinding & binding,
                                           Type::Kind ref_kind)
{
  return binding.is_constructor &&
         !binding.source_template &&
         binding.params.size() >= 2 &&
         is_same_class_reference_parameter(info.type,
                                           binding.params[1].second,
                                           ref_kind) &&
         has_trailing_constructor_defaults(binding, 2);
}

FunctionBinding * find_constructor_binding(ClassInfo & info,
                                          Type::Kind ref_kind)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding &&
       is_same_class_ref_constructor_binding(info, *binding, ref_kind)) {
      return binding;
    }
  }
  return nullptr;
}

bool has_user_declared_copy_constructor(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    if(binding &&
       !binding->synthesized &&
       is_same_class_ref_constructor_binding(info,
                                             *binding,
                                             Type::TK_LVALUE_REFERENCE)) {
      return true;
    }
  }
  return false;
}

bool has_user_declared_move_constructor(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    if(binding &&
       !binding->synthesized &&
       is_same_class_ref_constructor_binding(info,
                                             *binding,
                                             Type::TK_RVALUE_REFERENCE)) {
      return true;
    }
  }
  return false;
}

bool has_user_declared_copy_assignment(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find("operator=");
  if(found == info.methods.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    if(binding &&
       !binding->source_template &&
       !binding->synthesized &&
       binding->is_copy_assignment) {
      return true;
    }
  }
  return false;
}

bool has_user_declared_move_assignment(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find("operator=");
  if(found == info.methods.end()) {
    return false;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    if(binding &&
       !binding->source_template &&
       !binding->synthesized &&
       binding->is_move_assignment) {
      return true;
    }
  }
  return false;
}

bool type_allows_implicit_copy_construction(SemanticContext & ctx,
                                            const TypePtr & type);
bool type_allows_defaulted_move_construction(SemanticContext & ctx,
                                             const TypePtr & type);

bool implicit_copy_constructor_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info)
{
  if(has_user_declared_move_constructor(info) ||
     has_user_declared_move_assignment(info)) {
    return true;
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_implicit_copy_construction(ctx, info.bases[i].type->type)) {
      return true;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!type_allows_implicit_copy_construction(ctx, info.fields[i].type)) {
      return true;
    }
  }

  return false;
}

bool type_allows_implicit_copy_construction(SemanticContext & ctx,
                                            const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return true;
  }

  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_implicit_copy_construction(ctx, base->inner);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    ensure_implicit_special_members(ctx, *info);
    FunctionBinding * ctor = find_constructor_binding(*info, Type::TK_LVALUE_REFERENCE);
    if(!ctor) {
      ctor = ensure_implicit_copy_constructor(ctx, *info);
    }
    return ctor && !ctor->is_deleted;
  }

  return base->kind != Type::TK_FUNCTION && !is_void_type(base);
}

bool implicit_move_constructor_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_defaulted_move_construction(ctx, info.bases[i].type->type)) {
      return true;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!type_allows_defaulted_move_construction(ctx, info.fields[i].type)) {
      return true;
    }
  }

  return false;
}

bool type_allows_defaulted_move_construction(SemanticContext & ctx,
                                             const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return true;
  }

  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_defaulted_move_construction(ctx, base->inner);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    ensure_implicit_special_members(ctx, *info);

    FunctionBinding * ctor = nullptr;
    if(!type_is_const_object(type)) {
      ctor = find_constructor_binding(*info, Type::TK_RVALUE_REFERENCE);
      if(!ctor) {
        ctor = ensure_implicit_move_constructor(ctx, *info);
      }
    }

    if(!ctor || ctor->is_deleted) {
      ctor = find_constructor_binding(*info, Type::TK_LVALUE_REFERENCE);
      if(!ctor) {
        ctor = ensure_implicit_copy_constructor(ctx, *info);
      }
    }
    return ctor && !ctor->is_deleted;
  }

  return base->kind != Type::TK_FUNCTION && !is_void_type(base);
}

FunctionBinding * find_assignment_operator(ClassInfo & info, bool want_move)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find("operator=");
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if((want_move && binding->is_move_assignment) ||
       (!want_move && binding->is_copy_assignment)) {
      return binding;
    }
  }
  return nullptr;
}

bool type_allows_defaulted_move_assignment(SemanticContext & ctx,
                                           const TypePtr & type,
                                           std::set<ClassInfo *> & visiting);
bool type_allows_defaulted_copy_assignment(SemanticContext & ctx,
                                           const TypePtr & type,
                                           std::set<ClassInfo *> & visiting);

bool defaulted_copy_assignment_is_deleted_impl(SemanticContext & ctx,
                                               ClassInfo & info,
                                               std::set<ClassInfo *> & visiting,
                                               bool implicit_declaration)
{
  if(implicit_declaration &&
     (has_user_declared_move_constructor(info) ||
      has_user_declared_move_assignment(info))) {
    return true;
  }

  if(!visiting.insert(&info).second) {
    return false;
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_defaulted_copy_assignment(ctx, info.bases[i].type->type, visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!type_allows_defaulted_copy_assignment(ctx, info.fields[i].type, visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  visiting.erase(&info);
  return false;
}

bool type_allows_defaulted_copy_assignment(SemanticContext & ctx,
                                           const TypePtr & type,
                                           std::set<ClassInfo *> & visiting)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return false;
  }

  if(type_is_const_object(type)) {
    return false;
  }

  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_defaulted_copy_assignment(ctx, base->inner, visiting);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    if(visiting.count(info)) {
      return true;
    }
    ensure_implicit_special_members(ctx, *info);
    FunctionBinding * op = find_assignment_operator(*info, false);
    if(!op) {
      op = ctx.ensure_implicit_copy_assignment(*info);
    }
    return op != nullptr && !op->is_deleted;
  }

  return base->kind != Type::TK_FUNCTION && !is_void_type(base);
}

bool defaulted_move_assignment_is_deleted_impl(SemanticContext & ctx,
                                               ClassInfo & info,
                                               std::set<ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return false;
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!type_allows_defaulted_move_assignment(ctx, info.bases[i].type->type, visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!type_allows_defaulted_move_assignment(ctx, info.fields[i].type, visiting)) {
      visiting.erase(&info);
      return true;
    }
  }

  visiting.erase(&info);
  return false;
}

bool type_allows_defaulted_move_assignment(SemanticContext & ctx,
                                           const TypePtr & type,
                                           std::set<ClassInfo *> & visiting)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return false;
  }

  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           type_allows_defaulted_move_assignment(ctx, base->inner, visiting);
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(info && info->complete) {
    if(visiting.count(info)) {
      return true;
    }
    ensure_implicit_special_members(ctx, *info);
    FunctionBinding * op = nullptr;
    if(!type_is_const_object(type)) {
      op = find_assignment_operator(*info, true);
      if(!op) {
        op = ctx.ensure_implicit_move_assignment(*info);
      }
    }
    if(!op) {
      op = find_assignment_operator(*info, false);
    }
    if(!op) {
      op = ctx.ensure_implicit_copy_assignment(*info);
    }
    return op != nullptr && !op->is_deleted;
  }

  return !type_is_const_object(type);
}

bool defaulted_move_assignment_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info)
{
  std::set<ClassInfo *> visiting;
  return defaulted_move_assignment_is_deleted_impl(ctx, info, visiting);
}

bool defaulted_copy_assignment_is_deleted(SemanticContext & ctx,
                                          ClassInfo & info,
                                          bool implicit_declaration)
{
  std::set<ClassInfo *> visiting;
  return defaulted_copy_assignment_is_deleted_impl(ctx,
                                                  info,
                                                  visiting,
                                                  implicit_declaration);
}

void refresh_defaulted_move_assignment_state(SemanticContext & ctx,
                                             ClassInfo & info)
{
  if(!info.complete) {
    return;
  }
  for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(!binding ||
         !binding->is_move_assignment ||
         !binding->is_defaulted ||
         binding->synthesized) {
        continue;
      }
      binding->is_deleted = defaulted_move_assignment_is_deleted(ctx, info);
      binding->has_definition = !binding->is_deleted;
    }
  }
}

void refresh_defaulted_copy_assignment_state(SemanticContext & ctx,
                                             ClassInfo & info)
{
  if(!info.complete) {
    return;
  }
  for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(!binding ||
         !binding->is_copy_assignment ||
         !binding->is_defaulted ||
         binding->synthesized) {
        continue;
      }
      binding->is_deleted = defaulted_copy_assignment_is_deleted(ctx, info, false);
      binding->has_definition = !binding->is_deleted;
    }
  }
}

}  // namespace

TypePtr resolve_instantiated_member_alias_type(SemanticContext & ctx,
                                               Scope & scope,
                                               const TypePtr & type,
                                               ClassInfo * current_info)
{
  return canonicalize_member_typedef_type(ctx, scope, type, current_info);
}

MemberAccess default_access_for_class_kind(const std::string & class_kind)
{
  return (class_kind == "struct" || class_kind == "union") ? MA_PUBLIC : MA_PRIVATE;
}

bool is_anonymous_union_specifier(const CppAstNode & node)
{
  return find_anonymous_union_specifier(node) != nullptr;
}

std::string scope_anonymous_union_type_name(const CppAstNode & node)
{
  const CppAstNode * anon = find_anonymous_union_specifier(node);
  if(!anon) {
    throw std::logic_error("anonymous union specifier required");
  }
  return std::string("__anonymous_union_type") + anonymous_union_scope_suffix(*anon);
}

std::string scope_anonymous_union_storage_name(const CppAstNode & node)
{
  const CppAstNode * anon = find_anonymous_union_specifier(node);
  if(!anon) {
    throw std::logic_error("anonymous union specifier required");
  }
  return std::string("__anonymous_union_storage") + anonymous_union_scope_suffix(*anon);
}

bool synthesize_anonymous_union_storage_declaration(const CppAstNode & node,
                                                    CppAstNode & out_decl,
                                                    std::string & out_type_name,
                                                    std::string & out_storage_name)
{
  const CppAstNode * anon = find_anonymous_union_specifier(node);
  if(!anon) {
    return false;
  }

  out_type_name = scope_anonymous_union_type_name(node);
  out_storage_name = scope_anonymous_union_storage_name(node);

  out_decl = node;
  out_decl.kind = CppAstKind::simple_declaration;
  out_decl.value.clear();

  CppAstNode specifiers;
  specifiers.kind = CppAstKind::decl_specifier_seq;
  if(node.kind == CppAstKind::simple_declaration) {
    const CppAstNode * original_specifiers = find_child(node, CppAstKind::decl_specifier_seq);
    if(!original_specifiers) {
      return false;
    }
    specifiers = *original_specifiers;
  } else {
    specifiers.children.push_back(node);
  }

  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    CppAstNode & child = specifiers.children[i];
    if(child.kind == CppAstKind::class_specifier &&
       child.value.empty() &&
       node_is_union_class(child)) {
      child.value = out_type_name;
      break;
    }
  }

  CppAstNode declarator;
  declarator.kind = CppAstKind::declarator;
  CppAstNode identifier;
  identifier.kind = CppAstKind::identifier;
  identifier.value = out_storage_name;
  declarator.children.push_back(identifier);

  CppAstNode init_declarator;
  init_declarator.kind = CppAstKind::init_declarator;
  init_declarator.children.push_back(declarator);

  CppAstNode declarators;
  declarators.kind = CppAstKind::init_declarator_list;
  declarators.children.push_back(init_declarator);

  out_decl.children.clear();
  out_decl.children.push_back(specifiers);
  out_decl.children.push_back(declarators);
  return true;
}

void inject_anonymous_union_variable_bindings(Scope & scope,
                                              ClassInfo & storage_info,
                                              const std::string & storage_name)
{
  std::vector<ValueBinding> aliases;
  for(std::map<std::string, ValueBinding>::const_iterator it =
          storage_info.member_scope->values.begin();
      it != storage_info.member_scope->values.end();
      ++it) {
    const ValueBinding & inner = it->second;
    if(inner.kind != ValueBinding::VK_FIELD || inner.name.empty()) {
      continue;
    }

    std::map<std::string, ValueBinding>::const_iterator existing =
        scope.values.find(inner.name);
    if(existing != scope.values.end()) {
      if(existing->second.anonymous_storage_variable_name == storage_name) {
        continue;
      }
      throw std::logic_error("duplicate anonymous union member " + inner.name);
    }

    ValueBinding alias(ValueBinding::VK_VARIABLE, inner.name, inner.type);
    alias.declaration_node = inner.declaration_node;
    alias.definition_node = inner.definition_node;
    alias.anonymous_storage_variable_name = storage_name;
    alias.anonymous_storage_member_offset = inner.field_offset;
    alias.is_bit_field = inner.is_bit_field;
    alias.bit_field_width = inner.bit_field_width;
    alias.bit_field_offset = inner.bit_field_offset;
    alias.bit_field_storage_size = inner.bit_field_storage_size;
    alias.has_storage_definition = false;
    aliases.push_back(alias);
  }
  semantic_scope_mutation::bind_values(scope, aliases);
}

bool class_member_specifiers_supported(const CppAstNode & specifiers,
                                       bool allow_inline_virtual)
{
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(!child.has_token) {
      continue;
    }
    if(node_has_simple_type(child, KW_EXTERN) ||
       node_has_simple_type(child, KW_FRIEND)) {
      return false;
    }
    if(!allow_inline_virtual &&
       (node_has_simple_type(child, KW_INLINE) || node_has_simple_type(child, KW_VIRTUAL))) {
      return false;
    }
  }
  return true;
}

bool declarator_is_const_method(const CppAstNode & declarator)
{
  bool after_parameter_clause = false;
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::parameter_clause) {
      after_parameter_clause = true;
      continue;
    }
    if(after_parameter_clause &&
       declarator.children[i].kind == CppAstKind::cv_qualifier &&
       node_has_simple_type(declarator.children[i], KW_CONST)) {
      return true;
    }
  }
  return false;
}

bool declarator_is_volatile_method(const CppAstNode & declarator)
{
  bool after_parameter_clause = false;
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::parameter_clause) {
      after_parameter_clause = true;
      continue;
    }
    if(after_parameter_clause &&
       declarator.children[i].kind == CppAstKind::cv_qualifier &&
       node_has_simple_type(declarator.children[i], KW_VOLATILE)) {
      return true;
    }
  }
  return false;
}

RefQualifier declarator_ref_qualifier(const CppAstNode & declarator)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind != CppAstKind::ref_qualifier) {
      continue;
    }
    if(node_has_simple_type(declarator.children[i], OP_AMP)) {
      return RQ_LVALUE;
    }
    if(node_has_simple_type(declarator.children[i], OP_LAND)) {
      return RQ_RVALUE;
    }
  }
  return RQ_NONE;
}

const CppAstNode * declarator_function_qualifier(const CppAstNode & declarator)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::function_qualifier) {
      return &declarator.children[i];
    }
  }
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    if(child.kind != CppAstKind::nested_declarator || child.children.size() != 1) {
      continue;
    }
    if(const CppAstNode * qualifier =
           declarator_function_qualifier(child.children[0])) {
      return qualifier;
    }
  }
  return nullptr;
}

CppAstNode filtered_class_member_decl_specifiers(const CppAstNode & specifiers)
{
  CppAstNode filtered;
  filtered.kind = CppAstKind::decl_specifier_seq;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    if(node_has_simple_type(specifiers.children[i], KW_VIRTUAL) ||
       node_has_simple_type(specifiers.children[i], KW_MUTABLE) ||
       node_has_simple_type(specifiers.children[i], KW_FRIEND)) {
      continue;
    }
    filtered.children.push_back(specifiers.children[i]);
  }
  return filtered;
}

namespace {

bool is_pure_virtual_initializer(const CppAstNode & initializer)
{
  if(initializer.kind != CppAstKind::initializer ||
     initializer.children.size() != 1) {
    return false;
  }

  const CppAstNode & child = initializer.children[0];
  return child.kind == CppAstKind::literal &&
         semantic_utils::trim_space(node_text(child)) == "0";
}

bool method_syntax_allows_pure_virtual_initializer(const MethodSyntaxInfo & syntax)
{
  return syntax.decl_virtual || syntax.is_override || syntax.is_final;
}

bool decl_specifiers_have_virtual(const CppAstNode & specifiers)
{
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    if(node_has_simple_type(specifiers.children[i], KW_VIRTUAL)) {
      return true;
    }
  }
  return false;
}

CppAstNode filtered_method_specifiers(const CppAstNode & specifiers)
{
  CppAstNode filtered;
  filtered.kind = CppAstKind::decl_specifier_seq;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    if(node_has_simple_type(specifiers.children[i], KW_VIRTUAL)) {
      continue;
    }
    filtered.children.push_back(specifiers.children[i]);
  }
  return filtered;
}

CppAstNode filtered_method_declarator(const CppAstNode & declarator)
{
  CppAstNode filtered = declarator;
  std::vector<CppAstNode> kept;
  bool after_parameter_clause = false;
  for(size_t i = 0; i < filtered.children.size(); ++i) {
    const CppAstNode & child = filtered.children[i];
    if(child.kind == CppAstKind::parameter_clause) {
      after_parameter_clause = true;
      kept.push_back(child);
      continue;
    }
    if(child.kind == CppAstKind::virt_specifier ||
       child.kind == CppAstKind::function_qualifier ||
       (after_parameter_clause &&
        (child.kind == CppAstKind::cv_qualifier ||
         child.kind == CppAstKind::ref_qualifier ||
         child.kind == CppAstKind::nullability_qualifier))) {
      continue;
    }
    kept.push_back(child);
  }
  filtered.children.swap(kept);
  return filtered;
}

CppAstNode function_declarator_without_trailing_return(const CppAstNode & declarator)
{
  CppAstNode filtered = declarator;
  std::vector<CppAstNode> kept;
  for(size_t i = 0; i < filtered.children.size(); ++i) {
    if(filtered.children[i].kind == CppAstKind::trailing_return_type) {
      continue;
    }
    kept.push_back(filtered.children[i]);
  }
  filtered.children.swap(kept);
  return filtered;
}

bool declarator_has_complex_direct_nested_declarator(const CppAstNode & declarator)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    const CppAstNode & child = declarator.children[i];
    if(child.kind != CppAstKind::nested_declarator) {
      continue;
    }
    if(child.children.size() != 1 ||
       child.children[0].kind != CppAstKind::declarator ||
       child.children[0].children.size() != 1 ||
       child.children[0].children[0].kind != CppAstKind::identifier) {
      return true;
    }
  }
  return false;
}

bool declarator_has_override(const CppAstNode & declarator)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::virt_specifier &&
       declarator.children[i].value == "override") {
      return true;
    }
  }
  return false;
}

bool declarator_has_final(const CppAstNode & declarator)
{
  for(size_t i = 0; i < declarator.children.size(); ++i) {
    if(declarator.children[i].kind == CppAstKind::virt_specifier &&
       declarator.children[i].value == "final") {
      return true;
    }
  }
  return false;
}

bool class_member_object_type_supported(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || !type_is_complete(type)) {
    return false;
  }
  if(base->kind == Type::TK_FUNCTION) {
    return false;
  }
  return true;
}

bool dependent_class_member_object_type_supported(SemanticContext & ctx,
                                                  const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind == Type::TK_FUNCTION) {
    return false;
  }
  return type_is_complete(type) || ctx.type_depends_on_template_parameter(type);
}

bool deferred_class_member_object_layout_supported(SemanticContext & ctx,
                                                   const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind == Type::TK_FUNCTION) {
    return false;
  }
  if(type_is_complete(type)) {
    return true;
  }
  if(base->kind == Type::TK_ARRAY ||
     base->kind == Type::TK_CV ||
     base->kind == Type::TK_ATOMIC) {
    return deferred_class_member_object_layout_supported(ctx, base->inner);
  }
  if(base->kind != Type::TK_NAMED ||
     base->named_key.compare(0, 5, "enum ") == 0 ||
     ctx.type_depends_on_template_parameter(type)) {
    return false;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
  if(info &&
     info->template_output_node &&
     info->template_output_node->kind == CppAstKind::class_forward_declaration) {
    template_api::refresh_forward_class_template_selection(ctx, *info);
  }
  const CppAstNode * node =
      info ? (info->template_output_node ? info->template_output_node : info->class_node) :
             nullptr;
  return node && node->kind != CppAstKind::class_forward_declaration;
}

bool output_seed_class_member_object_type_supported(SemanticContext & ctx,
                                                    const TypePtr & type)
{
  return class_member_object_type_supported(type) ||
         (semantic_metrics::current_class_demand() ==
              semantic_metrics::CDK_OUTPUT_SEED &&
          deferred_class_member_object_layout_supported(ctx, type));
}

bool output_seed_dependent_class_member_object_type_supported(SemanticContext & ctx,
                                                              const TypePtr & type)
{
  return dependent_class_member_object_type_supported(ctx, type) ||
         (semantic_metrics::current_class_demand() ==
              semantic_metrics::CDK_OUTPUT_SEED &&
          deferred_class_member_object_layout_supported(ctx, type));
}

void synchronize_named_type_layout(const TypePtr & type,
                                   const ClassInfo & info)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED || !info.type) {
    return;
  }
  base->named_complete = info.type->named_complete;
  base->named_has_layout = info.type->named_has_layout;
  base->named_alignment = info.type->named_alignment;
  base->named_size = info.type->named_size;
  base->named_is_empty = info.type->named_is_empty;
  base->named_host_abi_chunks = info.type->named_host_abi_chunks;
  base->set_named_lambda_mangle(info.type->named_lambda_mangle());
  base->named_class_template_specialization_mangle_info =
      info.type->named_class_template_specialization_mangle_info;
  base->named_member_owner_type = info.type->named_member_owner_type;
  base->named_member_name = info.type->named_member_name;
}

void maybe_complete_class_member_object_type(SemanticContext & ctx,
                                             const TypePtr & type)
{
  const semantic_metrics::ClassDemandKind parent_demand =
      semantic_metrics::current_class_demand();
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->member_object_completion_calls;
    ++counters->member_object_completion_by_parent_demand[
        static_cast<std::size_t>(parent_demand)];
  }
  if(parent_demand == semantic_metrics::CDK_OUTPUT_SEED) {
    return;
  }
  thread_local std::vector<std::string> active_named_completions;
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }

  if(base->kind == Type::TK_ARRAY ||
     base->kind == Type::TK_CV ||
     base->kind == Type::TK_ATOMIC) {
    maybe_complete_class_member_object_type(ctx, base->inner);
    return;
  }

  if(base->kind != Type::TK_NAMED) {
    return;
  }
  semantic_metrics::ScopedClassDemand class_demand(
      semantic_metrics::CDK_FIELD_MEMBER_OBJECT);
  if(type_is_complete(type) && base->named_has_layout) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->member_object_completion_already_layout;
    }
    return;
  }
  if(ctx.type_depends_on_template_parameter(type)) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->member_object_completion_dependent;
    }
    return;
  }
  if(!base->named_key.empty()) {
    for(std::size_t i = 0; i < active_named_completions.size(); ++i) {
      if(active_named_completions[i] == base->named_key) {
        return;
      }
    }
  }
  struct ActiveCompletionGuard
  {
    explicit ActiveCompletionGuard(std::vector<std::string> & active,
                                   const std::string & key)
        : active(active),
          key(key)
    {
      if(!key.empty()) {
        active.push_back(key);
      }
    }

    ~ActiveCompletionGuard()
    {
      if(key.empty()) {
        return;
      }
      for(std::size_t i = active.size(); i > 0; --i) {
        if(active[i - 1] == key) {
          active.erase(active.begin() + static_cast<std::ptrdiff_t>(i - 1));
          break;
        }
      }
    }

    std::vector<std::string> & active;
    std::string key;
  } guard(active_named_completions, base->named_key);

  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->member_object_completion_no_class;
    }
    return;
  }
  std::string metrics_name;
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    metrics_name = metrics_class_name(*info);
    counters->note_member_object_completion_class(metrics_name,
                                                  metrics_class_ast_children(*info));
  }
  if(!info->complete) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->member_object_completion_complete_type_calls;
      ++counters->member_object_complete_type_by_parent_demand[
          static_cast<std::size_t>(parent_demand)];
      counters->note_member_object_completion_complete_type_call(metrics_name);
    }
    info = ctx.complete_class_type(base);
    if(!info) {
      ClassInfo * direct = ctx.class_info_for_type(base);
      if(direct &&
         direct->class_node &&
         direct->class_node->kind != CppAstKind::class_forward_declaration &&
         !direct->complete &&
         !direct->full_member_collection_in_progress &&
         !direct->reference_member_collection_in_progress) {
        if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
          ++counters->member_object_completion_direct_populates;
          ++counters->member_object_direct_populate_by_parent_demand[
              static_cast<std::size_t>(parent_demand)];
        }
        populate_class_info(ctx, *direct, *direct->class_node);
        if(direct->complete) {
          info = direct;
        }
      }
    }
  }
  if(info && info->complete) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      ++counters->member_object_completion_layout_syncs;
      counters->note_member_object_completion_layout_sync(metrics_name);
    }
    synchronize_named_type_layout(type, *info);
  }
}

void complete_deferred_class_member_object_layouts(SemanticContext & ctx,
                                                   ClassInfo & info)
{
  semantic_metrics::ScopedClassDemand class_demand(
      semantic_metrics::CDK_CLASS_LAYOUT);
  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!class_member_object_type_supported(info.fields[i].type) &&
       deferred_class_member_object_layout_supported(ctx, info.fields[i].type)) {
      maybe_complete_class_member_object_type(ctx, info.fields[i].type);
    }
  }
}

bool class_has_deferred_class_member_object_layouts(SemanticContext & ctx,
                                                    const ClassInfo & info)
{
  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(!class_member_object_type_supported(info.fields[i].type) &&
       deferred_class_member_object_layout_supported(ctx, info.fields[i].type)) {
      return true;
    }
  }
  return false;
}

bool bit_field_type_supported(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || !type_is_complete(type)) {
    return false;
  }
  return is_integral_type(base) ||
         (base->kind == Type::TK_NAMED &&
          base->named_key.compare(0, 5, "enum ") == 0);
}

bool dependent_bit_field_type_supported(SemanticContext & ctx, const TypePtr & type)
{
  if(bit_field_type_supported(type)) {
    return true;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(ctx.type_depends_on_template_parameter(base)) {
    return true;
  }
  if(base->kind == Type::TK_NAMED &&
     base->named_key.compare(0, 6, "class ") != 0 &&
     base->named_key.compare(0, 7, "struct ") != 0 &&
     base->named_key.compare(0, 6, "union ") != 0) {
    return true;
  }
  return false;
}

bool class_instantiation_is_dependent(SemanticContext & ctx,
                                      const ClassInfo & info)
{
  return info.dependent_instantiation ||
         (info.member_scope &&
          ctx.scope_has_template_placeholders(*info.member_scope));
}

bool class_definition_needs_virtual_validation(const CppAstNode & node)
{
  if(node.kind == CppAstKind::compound_statement ||
     node.kind == CppAstKind::lazy_function_body ||
     node.kind == CppAstKind::try_block) {
    return false;
  }
  if(node.kind == CppAstKind::virt_specifier ||
     node_has_simple_type(node, KW_VIRTUAL)) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(class_definition_needs_virtual_validation(node.children[i])) {
      return true;
    }
  }
  return false;
}

}  // namespace

void analyze_method_syntax(const CppAstNode * specifiers,
                           const CppAstNode & declarator,
                           MethodSyntaxInfo & out)
{
  reset_method_syntax_info(out);
  if(specifiers) {
    const bool spelled_explicit =
        std::any_of(specifiers->children.begin(),
                    specifiers->children.end(),
                    [](const CppAstNode & child)
                    {
                      return ((child.kind == CppAstKind::decl_specifier ||
                               child.kind == CppAstKind::specifier)) &&
                             child.value == "explicit";
                    });
    out.decl_static = decl_spec_contains_token(*specifiers, KW_STATIC);
    out.decl_virtual = decl_specifiers_have_virtual(*specifiers);
    out.decl_explicit =
        decl_spec_contains_token(*specifiers, KW_EXPLICIT) || spelled_explicit;
    out.filtered_specifiers = filtered_method_specifiers(*specifiers);
  }
  out.filtered_declarator = filtered_method_declarator(declarator);
  out.is_override = declarator_has_override(declarator);
  out.is_final = declarator_has_final(declarator);
  out.is_const_method = declarator_is_const_method(declarator);
  out.is_volatile_method = declarator_is_volatile_method(declarator);
  const CppAstNode * parameter_clause = find_child(declarator,
                                                  CppAstKind::parameter_clause);
  if(parameter_clause) {
    for(size_t i = 0; i < parameter_clause->children.size(); ++i) {
      if(parameter_clause->children[i].kind == CppAstKind::parameter_pack) {
        out.is_variadic = true;
        break;
      }
    }
  }
  out.ref_qualifier = declarator_ref_qualifier(declarator);
  out.function_qualifier = declarator_function_qualifier(declarator);
}

void prepare_method_parse_context(const CppAstNode * specifiers,
                                  const CppAstNode & declarator,
                                  PreparedMethodParseContext & out,
                                  bool has_method_syntax,
                                  bool require_parameter_clause_for_filtered_parse)
{
  reset_prepared_method_parse_context(out);
  out.has_method_syntax = has_method_syntax;
  out.set_parse_sources(specifiers, &declarator);
  const bool has_parameter_clause =
      find_child(declarator, CppAstKind::parameter_clause) != nullptr;
  if(!has_method_syntax || !has_parameter_clause) {
    out.has_method_syntax = false;
    return;
  }

  analyze_method_syntax(specifiers, declarator, out.syntax);
  const bool can_use_filtered_parse =
      (!require_parameter_clause_for_filtered_parse || has_parameter_clause) &&
      !declarator_has_complex_direct_nested_declarator(declarator);
  if(can_use_filtered_parse) {
    out.uses_filtered_parse = true;
    out.set_parse_sources(specifiers, &declarator);
  }
}

bool prepare_class_member_declaration_context(
    SemanticContext & ctx,
    Scope & member_scope,
    const CppAstNode & specifiers,
    const CppAstNode * declarators,
    bool collect_embedded_types,
    bool collect_named_forward_declarations,
    bool reference_class_templates_only,
    PreparedClassMemberDeclarationContext & out)
{
  out = PreparedClassMemberDeclarationContext();
  if(!ctx.prepare_namespace_scope_specifiers(member_scope,
                                             specifiers,
                                             declarators,
                                             collect_embedded_types,
                                             collect_named_forward_declarations,
                                             out.resolved_specifiers)) {
    return false;
  }

  out.filtered_specifiers =
      filtered_class_member_decl_specifiers(out.resolved_specifiers);
  const bool has_auto = decl_spec_contains_token(out.resolved_specifiers, KW_AUTO);
  out.parsed_decl_spec =
      has_auto ||
      ctx.parse_decl_spec(out.filtered_specifiers,
                          member_scope,
                          out.declaration_is_typedef,
                          out.base,
                          reference_class_templates_only);
  return true;
}

bool prepare_class_member_function_definition(
    SemanticContext & ctx,
    ClassInfo & info,
    const CppAstNode & node,
    bool reference_class_templates_only,
    PreparedClassMemberFunctionDefinition & out)
{
  out = PreparedClassMemberFunctionDefinition();
  out.specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  out.declarator = find_child(node, CppAstKind::declarator);
  out.body = find_class_function_body_node(node);
  if(!out.specifiers || !out.declarator || !out.body ||
     !class_member_specifiers_supported(*out.specifiers, true) ||
     decl_spec_contains_token(*out.specifiers, KW_MUTABLE)) {
    return false;
  }

  out.is_static_member = decl_spec_contains_token(*out.specifiers, KW_STATIC);
  out.is_constexpr_member = decl_spec_contains_token(*out.specifiers, KW_CONSTEXPR);
  out.is_inline_member = decl_spec_contains_token(*out.specifiers, KW_INLINE);
  prepare_method_parse_context(out.specifiers, *out.declarator, out.method);

  bool is_typedef = false;
  if(!ctx.parse_function_definition_base(*info.member_scope,
                                         *out.method.parse_specifiers_node(),
                                         out.method.parse_declarator_node(),
                                         *out.body,
                                         out.method.syntax.is_const_method,
                                         out.method.syntax.is_volatile_method,
                                         is_typedef,
                                         out.base,
                                         reference_class_templates_only) ||
     is_typedef) {
    if(is_typedef ||
       !try_parse_conversion_operator_result_type(ctx,
                                                 *info.member_scope,
                                                 out.method.parse_declarator_node(),
                                                 out.base)) {
      return false;
    }
  }

  const CppAstNode function_declarator =
      function_declarator_without_trailing_return(
          out.method.parse_declarator_node());
  return ctx.parse_declarator(*info.member_scope,
                              function_declarator,
                              out.base,
                              out.name,
                              out.declared_type,
                              reference_class_templates_only) &&
         !out.name.empty();
}

const CppAstNode * class_member_initializer(const CppAstNode & init_decl)
{
  return init_decl.children.size() > 1 &&
         init_decl.children[1].kind == CppAstKind::initializer ?
             &init_decl.children[1] :
             nullptr;
}

bool parse_class_member_declarator_type(SemanticContext & ctx,
                                        Scope & scope,
                                        const CppAstNode & specifiers,
                                        const CppAstNode & declarator,
                                        const CppAstNode * initializer,
                                        const TypePtr & base,
                                        bool has_auto,
                                        std::string & name,
                                        TypePtr & type,
                                        bool reference_class_templates_only)
{
  if(has_auto) {
    if(find_child(declarator, CppAstKind::parameter_clause) &&
       find_child(declarator, CppAstKind::trailing_return_type)) {
      bool is_typedef = false;
      TypePtr trailing_base;
      if(!ctx.parse_trailing_return_base(scope,
                                         specifiers,
                                         declarator,
                                         is_typedef,
                                         trailing_base,
                                         reference_class_templates_only) ||
         is_typedef || !trailing_base) {
        return false;
      }
      const CppAstNode function_declarator =
          function_declarator_without_trailing_return(declarator);
      return ctx.parse_declarator(scope,
                                  function_declarator,
                                  trailing_base,
                                  name,
                                  type,
                                  reference_class_templates_only);
    }
    if(!initializer) {
      return false;
    }
    const CppAstNode * payload =
        callsemantic_internal::unwrap_initializer_payload(initializer);
    if(!payload) {
      return false;
    }
    ExprInfo initializer_expr = ctx.analyze_expression(scope, *payload);
    if(!initializer_expr.type) {
      return false;
    }
    return ctx.parse_auto_declaration_type_from_expr(scope,
                                                     specifiers,
                                                     declarator,
                                                     initializer_expr,
                                                     name,
                                                     type,
                                                     reference_class_templates_only);
  }
  return ctx.parse_declarator(scope,
                              declarator,
                              base,
                              name,
                              type,
                              reference_class_templates_only);
}

ClassFunctionOptions class_function_options(MemberAccess access,
                                            const MethodSyntaxInfo * syntax,
                                            bool is_constructor,
                                            bool is_destructor,
                                            bool is_constexpr,
                                            bool is_defaulted,
                                            bool is_inline)
{
  ClassFunctionOptions options;
  options.access = access;
  options.is_constructor = is_constructor;
  options.is_destructor = is_destructor;
  options.is_constexpr = is_constexpr;
  options.is_defaulted = is_defaulted;
  options.is_inline = is_inline;
  if(!syntax) {
    return options;
  }

  options.is_explicit = syntax->decl_explicit;
  options.is_const_method = syntax->is_const_method;
  options.is_volatile_method = syntax->is_volatile_method;
  options.is_variadic = syntax->is_variadic;
  options.ref_qualifier = syntax->ref_qualifier;
  options.is_virtual_specified = syntax->decl_virtual;
  options.is_override_specified = syntax->is_override;
  options.is_final = syntax->is_final;
  options.function_qualifier = syntax->function_qualifier;
  return options;
}

void validate_method_virtual_syntax(const MethodSyntaxInfo & info)
{
  (void)info;
}

TypePtr method_function_type(const TypePtr & class_type,
                             bool is_const_method,
                             bool is_volatile_method,
                             const TypePtr & declared_type)
{
  TypePtr function_type = strip_top_level_cv(declared_type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return TypePtr();
  }

  std::vector<TypePtr> params;
  TypePtr this_type = make_cv(class_type, is_const_method, is_volatile_method);
  params.push_back(make_pointer(this_type));
  params.insert(params.end(), function_type->params.begin(), function_type->params.end());
  return make_function(function_type->inner,
                       params,
                       function_type->variadic,
                       function_type->function_const,
                       function_type->function_volatile,
                       function_type->prototype_relaxed,
                       function_type->function_ref_qualifier);
}

std::string unqualified_conversion_operator_member_name(const std::string & name)
{
  if(name.compare(0, 8, "operator") == 0) {
    return name;
  }
  const std::size_t scope_pos = semantic_utils::top_level_scope_split(name);
  if(scope_pos == std::string::npos) {
    return name;
  }
  const std::string unqualified = name.substr(scope_pos + 2);
  return unqualified.compare(0, 8, "operator") == 0 ?
      unqualified :
      name;
}

std::string conversion_operator_identifier_member_name(const CppAstNode & identifier)
{
  const QualifiedName * qualified_name = cppast_qualified_name_syntax(identifier);
  if(qualified_name && !qualified_name->name.empty()) {
    return qualified_name->name;
  }
  return unqualified_conversion_operator_member_name(identifier.value);
}

bool declarator_has_conversion_operator_type_id(const CppAstNode & declarator)
{
  const CppAstNode * identifier = find_child(declarator, CppAstKind::identifier);
  return identifier && cppast_conversion_type_id_syntax(*identifier);
}

bool class_member_declares_conversion_operator(const CppAstNode & node)
{
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  return declarator && declarator_has_conversion_operator_type_id(*declarator);
}

bool source_text_mentions_template_parameter(Scope & scope,
                                             const std::string & text)
{
  if(text.empty()) {
    return false;
  }
  std::vector<std::string> texts;
  texts.push_back(text);
  return callsemantic::template_argument_texts_mention_enclosing_source_template_parameters(
             scope,
             texts) ||
         callsemantic::template_argument_texts_mention_template_bound_scope_names(
             scope,
             texts);
}

bool conversion_type_syntax_mentions_template_parameter(
    Scope & scope,
    const TemplateArgumentSyntax & syntax);

bool conversion_type_syntax_mentions_template_parameter(
    Scope & scope,
    const TemplateIdSyntax & syntax)
{
  if(source_text_mentions_template_parameter(scope, syntax.name.name)) {
    return true;
  }
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(source_text_mentions_template_parameter(scope, syntax.arguments[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(conversion_type_syntax_mentions_template_parameter(
           scope,
           syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool conversion_type_syntax_mentions_template_parameter(Scope & scope,
                                                        const CppAstNode & node)
{
  if(source_text_mentions_template_parameter(scope, node.value)) {
    return true;
  }
  if(const TemplateIdSyntax * syntax = cppast_template_id_syntax(node)) {
    if(conversion_type_syntax_mentions_template_parameter(scope, *syntax)) {
      return true;
    }
  }
  if(const CppAstNode * conversion_type_id =
         cppast_conversion_type_id_syntax(node)) {
    if(conversion_type_syntax_mentions_template_parameter(scope,
                                                          *conversion_type_id)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(conversion_type_syntax_mentions_template_parameter(
           scope,
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(conversion_type_syntax_mentions_template_parameter(
           scope,
           node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(conversion_type_syntax_mentions_template_parameter(scope,
                                                          node.children[i])) {
      return true;
    }
  }
  return false;
}

bool conversion_type_syntax_mentions_template_parameter(
    Scope & scope,
    const TemplateArgumentSyntax & syntax)
{
  if(source_text_mentions_template_parameter(scope, syntax.text)) {
    return true;
  }
  if(syntax.template_id &&
     conversion_type_syntax_mentions_template_parameter(scope,
                                                       *syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     conversion_type_syntax_mentions_template_parameter(scope,
                                                       *syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     conversion_type_syntax_mentions_template_parameter(scope,
                                                       *syntax.expression)) {
    return true;
  }
  return false;
}

bool try_parse_conversion_operator_result_type(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & declarator,
                                               TypePtr & out)
{
  const CppAstNode * identifier = find_child(declarator, CppAstKind::identifier);
  const std::string operator_name =
      identifier ?
          conversion_operator_identifier_member_name(*identifier) :
          std::string();
  if(operator_name.compare(0, 8, "operator") != 0) {
    return false;
  }

  const std::string suffix = operator_name.substr(8);
  if(suffix.empty() || suffix == "new" || suffix == "delete") {
    return false;
  }

  const CppAstNode * conversion_type_id =
      identifier ? cppast_conversion_type_id_syntax(*identifier) : nullptr;
  CppAstNode substituted_conversion_type_id;
  const CppAstNode * conversion_type_id_for_parse = conversion_type_id;
  const std::vector<template_model::TemplateParameterInfo> * substitution_parameters = nullptr;
  const std::vector<template_model::TemplateArgument> * substitution_arguments = nullptr;
  if(scope.class_info) {
    class_template_member_substitution_bindings(*scope.class_info,
                                                substitution_parameters,
                                                substitution_arguments);
  }
  if(conversion_type_id &&
     substitution_parameters &&
     substitution_arguments &&
     !substitution_arguments->empty() &&
     template_argument_semantics::substitute_expression_node_for_template_arguments(
         scope,
         *conversion_type_id,
         *substitution_parameters,
         *substitution_arguments,
         substituted_conversion_type_id)) {
    conversion_type_id_for_parse = &substituted_conversion_type_id;
  }
  if(conversion_type_id_for_parse &&
     ctx.parse_type_id(scope,
                       *conversion_type_id_for_parse,
                       out,
                       false,
                       false)) {
    bool allow_concrete_dependent_argument_spelling = true;
    bool record_conversion_result_class_use = true;
    if(conversion_type_id) {
      std::vector<std::string> source_type_texts;
      source_type_texts.push_back(node_text(*conversion_type_id));
      if(callsemantic::template_argument_texts_mention_enclosing_source_template_parameters(
             scope,
             source_type_texts) ||
         conversion_type_syntax_mentions_template_parameter(scope,
                                                            *conversion_type_id)) {
        allow_concrete_dependent_argument_spelling = false;
        record_conversion_result_class_use = false;
      }
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "conversion-result-class-use"
            << " location="
            << (conversion_type_id ?
                    ctx.source_location_for_node(*conversion_type_id) :
                    (identifier ? ctx.source_location_for_node(*identifier) :
                                  std::string()))
            << " text="
            << (conversion_type_id ? node_text(*conversion_type_id) :
                                     std::string())
            << " allow-concrete="
            << (allow_concrete_dependent_argument_spelling ? "yes" : "no")
            << " record="
            << (record_conversion_result_class_use ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(record_conversion_result_class_use) {
      const CppAstNode & source_use_node =
          conversion_type_id ? *conversion_type_id : *identifier;
      ctx.record_declaration_type_class_use_for_resolved_type_node(
          scope,
          source_use_node,
          out,
          ctx.source_location_for_node(source_use_node),
          allow_concrete_dependent_argument_spelling);
    }
    return true;
  }

  out = ctx.lookup_type(scope, suffix);
  if(out) {
    return true;
  }

  return false;
}

bool parse_conversion_operator_signature(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    std::string & member_name,
    TypePtr & declared_type,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> * default_args,
    MethodSyntaxInfo * syntax_out)
{
  const CppAstNode * member_specifiers = find_child(node, CppAstKind::member_specifiers);
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  if(!declarator) {
    return false;
  }

  PreparedMethodParseContext prepared_method;
  prepare_method_parse_context(member_specifiers, *declarator, prepared_method);
  validate_method_virtual_syntax(prepared_method.syntax);

  TypePtr base;
  if(!try_parse_conversion_operator_result_type(ctx,
                                                scope,
                                                prepared_method.parse_declarator_node(),
                                                base)) {
    return false;
  }

  if(!ctx.parse_declarator(scope,
                           prepared_method.parse_declarator_node(),
                           base,
                           member_name,
                           declared_type) ||
     member_name.empty()) {
    return false;
  }
  const CppAstNode * identifier =
      find_child(prepared_method.parse_declarator_node(), CppAstKind::identifier);
  if(identifier) {
    member_name = conversion_operator_identifier_member_name(*identifier);
  }
  member_name =
      canonical_function_lookup_name(
          unqualified_conversion_operator_member_name(member_name));

  params.clear();
  if(default_args) {
    default_args->clear();
  }
  const CppAstNode * parameter_clause =
      find_child(*declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(scope,
                                 *parameter_clause,
                                 params,
                                 default_args,
                                 true)) {
    return false;
  }

  if(syntax_out) {
    *syntax_out = prepared_method.syntax;
  }
  return true;
}

namespace {

MemberAccess access_from_node(const CppAstNode & node)
{
  if(node_has_simple_type(node, KW_PUBLIC)) {
    return MA_PUBLIC;
  }
  if(node_has_simple_type(node, KW_PROTECTED)) {
    return MA_PROTECTED;
  }
  return MA_PRIVATE;
}

bool has_recorded_direct_base(const ClassInfo & info,
                              ClassInfo * base_class,
                              MemberAccess access,
                              bool is_virtual)
{
  return std::any_of(info.bases.begin(),
                     info.bases.end(),
                     [&](const BaseInfo & existing)
                     {
                       return existing.type == base_class &&
                              existing.access == access &&
                              existing.is_virtual == is_virtual;
                     });
}

void record_direct_base(ClassInfo & info,
                        ClassInfo * base_class,
                        MemberAccess access,
                        bool is_virtual,
                        bool source_dependent)
{
  if(has_recorded_direct_base(info, base_class, access, is_virtual)) {
    return;
  }

  BaseInfo base;
  base.type = base_class;
  base.access = access;
  base.is_virtual = is_virtual;
  base.source_dependent = source_dependent;
  info.bases.push_back(base);
  if(info.member_scope->named_types.find(base_class->name) ==
     info.member_scope->named_types.end()) {
    semantic_scope_mutation::bind_named_type(*info.member_scope,
                                             base_class->name,
                                             base_class->type);
  }
}

void parse_base_clause(SemanticContext & ctx, ClassInfo & info, const CppAstNode & node)
{
  const CppAstNode * clause = find_child(node, CppAstKind::base_clause);
  if(!clause) {
    return;
  }
  for(size_t j = 0; j < clause->children.size(); ++j) {
    const CppAstNode & specifier = clause->children[j];
    bool is_virtual = false;
    bool is_pack_expansion = false;
    for(size_t i = 0; i < specifier.children.size(); ++i) {
      if(specifier.children[i].kind == CppAstKind::virtual_node) {
        is_virtual = true;
      } else if(specifier.children[i].kind == CppAstKind::ellipsis) {
        is_pack_expansion = true;
      }
    }

    const CppAstNode * access_node = find_child(specifier, CppAstKind::access_specifier);
    const CppAstNode * base_name = find_child(specifier, CppAstKind::base_name);
    if(!base_name) {
      throw std::logic_error("invalid base-specifier");
    }

    std::vector<std::string> base_names;
    std::vector<std::vector<TemplateArgumentSyntax> > expanded_base_arg_syntaxes;
    bool expanded_base_pack = false;
    if(is_pack_expansion) {
      base_names = expand_base_name_pack_texts(ctx,
                                               *info.member_scope,
                                               *base_name,
                                               &expanded_base_arg_syntaxes,
                                               &expanded_base_pack);
    }
    if(base_names.empty()) {
      if(is_pack_expansion && expanded_base_pack) {
        continue;
      }
      base_names.push_back(base_name->value);
    }

    for(size_t i = 0; i < base_names.size(); ++i) {
      QualifiedName base_template_id;
      std::vector<std::string> base_arg_texts;
      const std::vector<TemplateArgumentSyntax> * expanded_arg_syntaxes =
          i < expanded_base_arg_syntaxes.size() &&
                  !expanded_base_arg_syntaxes[i].empty() ?
              &expanded_base_arg_syntaxes[i] :
              nullptr;
      const TemplateIdSyntax * base_template_syntax =
          base_names[i] == base_name->value ? cppast_template_id_syntax(*base_name) :
                                              nullptr;
      const std::vector<TemplateArgumentSyntax> * base_arg_syntaxes_for_lookup =
          expanded_arg_syntaxes ? expanded_arg_syntaxes :
                                  (base_template_syntax ?
                                       &base_template_syntax->argument_syntaxes :
                                       nullptr);
      const bool have_base_arg_locations =
          base_template_syntax && !base_template_syntax->name.name.empty();
      if(have_base_arg_locations) {
        base_template_id = base_template_syntax->name;
        base_arg_texts = base_template_syntax->arguments;
      }
      std::string base_template_location;
      if(have_base_arg_locations) {
        base_template_location =
            template_api::normalize_template_witness_source_location(
                ctx.source_location_for_name_in_node(node, base_template_id.name));
        if(!semantic_trace::source_location_points_at_identifier(
               base_template_location,
               base_template_id.name)) {
          base_template_location.clear();
        }
      }
      const std::vector<std::string> base_arg_locations =
          have_base_arg_locations ?
              template_argument_source_locations_for_node(ctx,
                                                          *base_name,
                                                          base_arg_texts,
                                                          base_template_location) :
              std::vector<std::string>();
      const template_api::ScopedTemplateArgumentSourceLocations
          base_argument_source_locations(base_arg_texts, base_arg_locations);
      CppAstNode expanded_base_node;
      const CppAstNode * base_node_for_lookup = base_name;
      if(base_names[i] != base_name->value && expanded_arg_syntaxes) {
        expanded_base_node =
            expanded_base_name_node_with_argument_syntaxes(
                *base_name,
                base_names[i],
                *expanded_arg_syntaxes);
        base_node_for_lookup = &expanded_base_node;
      }
      TypePtr base_type =
          base_node_for_lookup != base_name || base_names[i] == base_name->value ?
              resolve_base_type_node(ctx,
                                     *info.member_scope,
                                     *base_node_for_lookup,
                                     false,
                                     &info) :
              lookup_base_type_name(ctx,
                                    *info.member_scope,
                                    base_names[i],
                                    false,
                                    &info);
      semantic_template_class::append_base_clause_template_value_dependencies(
          ctx,
          *info.member_scope,
          *base_name,
          base_template_syntax,
          base_arg_syntaxes_for_lookup,
          info.template_value_dependencies);
      if(have_base_arg_locations) {
        ctx.record_class_use_for_resolved_type_node(*info.member_scope,
                                                    *base_name,
                                                    base_type,
                                                    base_template_location);
      }
      ClassInfo * base_class = ctx.complete_class_type(base_type);
      if(!base_class || !base_class->complete) {
        const bool defer_base_lookup =
            ctx.should_defer_unresolved_type_lookup(*info.member_scope, base_names[i]);
        if(defer_base_lookup ||
           (base_type &&
            base_type->kind == Type::TK_NAMED &&
            base_type->named_key.find("dependent ") == 0)) {
          continue;
        }
        std::ostringstream out;
        out << "base class must be complete: " << base_names[i]
            << " [class=" << info.qualified_name << "]"
            << " [scope_has_template_placeholders="
            << (ctx.scope_has_template_placeholders(*info.member_scope) ? "yes" : "no") << "]"
            << " [defer_base_lookup="
            << (defer_base_lookup ? "yes" : "no") << "]";
        if(base_type && base_type->kind == Type::TK_NAMED) {
          out << " [base_type_key=" << base_type->named_key << "]";
        }
        throw TemplateSubstitutionFailure(out.str());
      }

      const bool source_dependent =
          callsemantic::template_argument_texts_mention_enclosing_source_template_parameters(
              *info.member_scope,
              std::vector<std::string>(1, base_names[i]));
      record_direct_base(info,
                         base_class,
                         access_node ? access_from_node(*access_node) : info.default_access,
                         is_virtual,
                         source_dependent);
    }
  }
}

void parse_reference_base_clause(SemanticContext & ctx, ClassInfo & info, const CppAstNode & node)
{
  const CppAstNode * clause = find_child(node, CppAstKind::base_clause);
  if(!clause) {
    return;
  }
  for(size_t j = 0; j < clause->children.size(); ++j) {
    const CppAstNode & specifier = clause->children[j];
    bool is_virtual = false;
    bool is_pack_expansion = false;
    for(size_t i = 0; i < specifier.children.size(); ++i) {
      if(specifier.children[i].kind == CppAstKind::virtual_node) {
        is_virtual = true;
      } else if(specifier.children[i].kind == CppAstKind::ellipsis) {
        is_pack_expansion = true;
      }
    }

    const CppAstNode * access_node = find_child(specifier, CppAstKind::access_specifier);
    const CppAstNode * base_name = find_child(specifier, CppAstKind::base_name);
    if(!base_name) {
      continue;
    }

    std::vector<std::string> base_names;
    std::vector<std::vector<TemplateArgumentSyntax> > expanded_base_arg_syntaxes;
    bool expanded_base_pack = false;
    if(is_pack_expansion) {
      base_names = expand_base_name_pack_texts(ctx,
                                               *info.member_scope,
                                               *base_name,
                                               &expanded_base_arg_syntaxes,
                                               &expanded_base_pack);
    }
    if(base_names.empty()) {
      if(is_pack_expansion && expanded_base_pack) {
        continue;
      }
      base_names.push_back(base_name->value);
    }

    for(size_t i = 0; i < base_names.size(); ++i) {
      QualifiedName base_template_id;
      std::vector<std::string> base_arg_texts;
      const std::vector<TemplateArgumentSyntax> * expanded_arg_syntaxes =
          i < expanded_base_arg_syntaxes.size() &&
                  !expanded_base_arg_syntaxes[i].empty() ?
              &expanded_base_arg_syntaxes[i] :
              nullptr;
      const TemplateIdSyntax * base_template_syntax =
          base_names[i] == base_name->value ? cppast_template_id_syntax(*base_name) :
                                              nullptr;
      const std::vector<TemplateArgumentSyntax> * base_arg_syntaxes_for_lookup =
          expanded_arg_syntaxes ? expanded_arg_syntaxes :
                                  (base_template_syntax ?
                                       &base_template_syntax->argument_syntaxes :
                                       nullptr);
      const bool have_base_arg_locations =
          base_template_syntax && !base_template_syntax->name.name.empty();
      if(have_base_arg_locations) {
        base_template_id = base_template_syntax->name;
        base_arg_texts = base_template_syntax->arguments;
      }
      std::string base_template_location;
      if(have_base_arg_locations) {
        base_template_location =
            template_api::normalize_template_witness_source_location(
                ctx.source_location_for_name_in_node(node, base_template_id.name));
        if(!semantic_trace::source_location_points_at_identifier(
               base_template_location,
               base_template_id.name)) {
          base_template_location.clear();
        }
      }
      const std::vector<std::string> base_arg_locations =
          have_base_arg_locations ?
              template_argument_source_locations_for_node(ctx,
                                                          *base_name,
                                                          base_arg_texts,
                                                          base_template_location) :
              std::vector<std::string>();
      const template_api::ScopedTemplateArgumentSourceLocations
          base_argument_source_locations(base_arg_texts, base_arg_locations);
      CppAstNode expanded_base_node;
      const CppAstNode * base_node_for_lookup = base_name;
      if(base_names[i] != base_name->value && expanded_arg_syntaxes) {
        expanded_base_node =
            expanded_base_name_node_with_argument_syntaxes(
                *base_name,
                base_names[i],
                *expanded_arg_syntaxes);
        base_node_for_lookup = &expanded_base_node;
      }
      TypePtr base_type =
          base_node_for_lookup != base_name || base_names[i] == base_name->value ?
              resolve_base_type_node(ctx,
                                     *info.member_scope,
                                     *base_node_for_lookup,
                                     true,
                                     &info) :
              lookup_base_type_name(ctx,
                                    *info.member_scope,
                                    base_names[i],
                                    true,
                                    &info);
      semantic_template_class::append_base_clause_template_value_dependencies(
          ctx,
          *info.member_scope,
          *base_name,
          base_template_syntax,
          base_arg_syntaxes_for_lookup,
          info.template_value_dependencies);
      ClassInfo * base_class = ctx.class_info_for_type(base_type);
      if(!base_class) {
        continue;
      }
      ctx.ensure_class_reference_members(*base_class);
      const bool source_dependent =
          callsemantic::template_argument_texts_mention_enclosing_source_template_parameters(
              *info.member_scope,
              std::vector<std::string>(1, base_names[i]));
      record_direct_base(info,
                         base_class,
                         access_node ? access_from_node(*access_node) : info.default_access,
                         is_virtual,
                         source_dependent);
    }
  }
}

}  // namespace

void reset_instantiated_class_info(ClassInfo & info,
                                   const std::string & template_name,
                                   const CppAstNode * output_node)
{
  const bool was_lambda_closure = info.is_lambda_closure;
  std::shared_ptr<cpp_decl::Type::LambdaMangleMetadata> lambda_mangle =
      info.type ? info.type->named_lambda_mangle() :
                  std::shared_ptr<cpp_decl::Type::LambdaMangleMetadata>();
  info.template_output_node = output_node;
  info.is_final = output_node && output_node->is_final_specifier;
  info.fields.clear();
  info.methods.clear();
  info.method_declaration_order.clear();
  info.bases.clear();
  info.anonymous_member_classes.clear();
  info.vtable_entries.clear();
  info.complete_subobjects.clear();
  info.virtual_base_subobjects.clear();
  info.vtables.clear();
  info.friend_functions.clear();
  info.friend_access_functions.clear();
  info.friend_function_templates.clear();
  info.friend_access_function_templates.clear();
  info.deferred_member_aliases.clear();
  info.is_polymorphic = false;
  info.rtti_required = false;
  info.friend_class_names.clear();
  info.is_initializer_list = false;
  info.initializer_list_element_type = TypePtr();
  info.is_lambda_closure = was_lambda_closure;
  info.is_explicit_specialization = false;
  info.has_own_vptr = false;
  info.nonvirtual_size = 0;
  info.nonvirtual_alignment = 1;
  info.complete = false;
  info.reference_reset_witness_class_templates.clear();
  info.reentrant_primary_selection = false;
  info.type->named_complete = false;
  info.type->named_has_layout = false;
  info.type->named_alignment = 1;
  info.type->named_size = 0;
  info.type->named_is_empty = false;
  if(was_lambda_closure || info.source_is_named_function_local_class) {
    info.type->set_named_lambda_mangle(lambda_mangle);
  } else {
    info.type->set_named_lambda_mangle(
        std::shared_ptr<cpp_decl::Type::LambdaMangleMetadata>());
  }

  info.member_scope->named_types.clear();
  info.member_scope->named_type_packs.clear();
  info.member_scope->named_value_packs.clear();
  info.member_scope->named_pack_sizes.clear();
  info.member_scope->template_bound_type_names.clear();
  info.member_scope->template_bound_type_pack_names.clear();
  info.member_scope->template_bound_value_names.clear();
  info.member_scope->template_bound_value_pack_names.clear();
  info.member_scope->template_bound_template_names.clear();
  info.member_scope->values.clear();
  info.member_scope->namespace_bindings.clear();
  info.member_scope->function_sets.clear();
  info.member_scope->class_templates.clear();
  info.member_scope->function_templates.clear();
  info.member_scope->collected_template_declarations.clear();
  info.member_scope->alias_templates.clear();
  info.member_scope->variable_templates.clear();
  info.member_scope->using_directives.clear();
  semantic_scope_mutation::bind_named_type(*info.member_scope, template_name, info.type);
  info.template_instantiation_in_progress = false;
  info.full_member_collection_in_progress = false;
  info.reference_member_collection_in_progress = false;
  info.reference_members_collected = false;
  info.implicit_special_members_ensured = false;
  info.out_of_class_member_function_template_definitions_applied = false;
  info.out_of_class_member_function_definitions_applied = false;
  info.out_of_class_special_member_definitions_applied = false;
  info.out_of_class_static_member_definitions_applied = false;
  info.definition_output_in_progress = false;
  info.definition_output_emitted = false;
}

void reset_reference_member_state_for_full_collection(ClassInfo & info)
{
  const TypePtr info_type = strip_top_level_cv(info.type);
  Scope::NamedTypeMap preserved_named_types;
  std::map<std::string, MemberAccess> preserved_named_type_access;
  std::map<std::string, std::vector<TypePtr> > preserved_named_type_packs;
  std::map<std::string, std::vector<ValueBinding> > preserved_named_value_packs;
  std::map<std::string, std::size_t> preserved_named_pack_sizes;
  std::set<std::string> preserved_template_bound_value_pack_names;
  std::map<std::string, ValueBinding> preserved_values;
  std::map<std::string, ClassTemplateDecl *> preserved_class_templates;
  std::map<std::string, AliasTemplateDecl *> preserved_alias_templates;
  if(info.member_scope) {
    for(auto it =
            info.member_scope->named_types.begin();
        it != info.member_scope->named_types.end();
        ++it) {
      TypePtr named = strip_top_level_cv(it->second);
      TypePtr owner =
          named && named->kind == Type::TK_NAMED ?
              strip_top_level_cv(named->named_member_owner_type) :
              TypePtr();
      const bool is_nested_class_type =
          info_type &&
          info_type->kind == Type::TK_NAMED &&
          owner &&
          owner->kind == Type::TK_NAMED &&
          owner->named_key == info_type->named_key;
      if(it->second == info.type ||
         is_nested_class_type ||
         info.member_scope->template_bound_type_names.count(it->first) != 0) {
        preserved_named_types[it->first] = it->second;
        std::map<std::string, MemberAccess>::const_iterator access =
            info.member_scope->named_type_access.find(it->first);
        if(access != info.member_scope->named_type_access.end()) {
          preserved_named_type_access[it->first] = access->second;
        }
      }
    }
    for(std::set<std::string>::const_iterator it =
            info.member_scope->template_bound_type_pack_names.begin();
        it != info.member_scope->template_bound_type_pack_names.end();
        ++it) {
      std::map<std::string, std::vector<TypePtr> >::const_iterator pack =
          info.member_scope->named_type_packs.find(*it);
      if(pack != info.member_scope->named_type_packs.end()) {
        preserved_named_type_packs[*it] = pack->second;
      }
    }
    for(std::set<std::string>::const_iterator it =
            info.member_scope->template_bound_value_names.begin();
        it != info.member_scope->template_bound_value_names.end();
        ++it) {
      std::map<std::string, ValueBinding>::const_iterator value =
          info.member_scope->values.find(*it);
      if(value != info.member_scope->values.end()) {
        preserved_values[*it] = value->second;
      }
      std::map<std::string, std::vector<ValueBinding> >::const_iterator pack =
          info.member_scope->named_value_packs.find(*it);
      if(pack != info.member_scope->named_value_packs.end()) {
        preserved_named_value_packs[*it] = pack->second;
      }
    }
    for(std::set<std::string>::const_iterator it =
            info.member_scope->template_bound_value_pack_names.begin();
        it != info.member_scope->template_bound_value_pack_names.end();
        ++it) {
      preserved_template_bound_value_pack_names.insert(*it);
    }
    for(std::map<std::string, std::size_t>::const_iterator it =
            info.member_scope->named_pack_sizes.begin();
        it != info.member_scope->named_pack_sizes.end();
        ++it) {
      if(info.member_scope->template_bound_type_pack_names.count(it->first) != 0 ||
         info.member_scope->template_bound_value_pack_names.count(it->first) != 0) {
        preserved_named_pack_sizes[it->first] = it->second;
      }
    }
    for(std::map<std::string, ClassTemplateDecl *>::const_iterator it =
            info.member_scope->class_templates.begin();
        it != info.member_scope->class_templates.end();
        ++it) {
      if(should_preserve_class_template_across_reference_reset(info,
                                                               it->first,
                                                               it->second)) {
        preserved_class_templates[it->first] = it->second;
      } else if(class_template_has_reference_reset_witness_static_member_metadata(
                    it->second)) {
        info.reference_reset_witness_class_templates[it->first] = it->second;
      }
    }
    for(std::set<std::string>::const_iterator it =
            info.member_scope->template_bound_template_names.begin();
        it != info.member_scope->template_bound_template_names.end();
        ++it) {
      std::map<std::string, AliasTemplateDecl *>::const_iterator alias_template =
          info.member_scope->alias_templates.find(*it);
      if(alias_template != info.member_scope->alias_templates.end()) {
        preserved_alias_templates[*it] = alias_template->second;
      }
    }
  }

  info.fields.clear();
  info.methods.clear();
  info.method_declaration_order.clear();
  info.bases.clear();
  info.anonymous_member_classes.clear();
  info.vtable_entries.clear();
  info.complete_subobjects.clear();
  info.virtual_base_subobjects.clear();
  info.vtables.clear();
  info.friend_functions.clear();
  info.friend_access_functions.clear();
  info.friend_function_templates.clear();
  info.friend_access_function_templates.clear();
  info.friend_class_names.clear();
  info.deferred_member_aliases.clear();
  info.is_polymorphic = false;
  info.rtti_required = false;
  info.is_initializer_list = false;
  info.initializer_list_element_type = TypePtr();
  info.has_own_vptr = false;
  info.nonvirtual_size = 0;
  info.nonvirtual_alignment = 1;
  info.complete = false;
  if(info.type) {
    info.type->named_complete = false;
    info.type->named_has_layout = false;
    info.type->named_alignment = 1;
    info.type->named_size = 0;
    info.type->named_is_empty = false;
    info.type->named_host_abi_chunks.clear();
  }

  if(info.member_scope) {
    info.member_scope->named_types.swap(preserved_named_types);
    info.member_scope->named_type_access.swap(preserved_named_type_access);
    info.member_scope->named_type_packs.swap(preserved_named_type_packs);
    info.member_scope->named_value_packs.swap(preserved_named_value_packs);
    info.member_scope->named_pack_sizes.swap(preserved_named_pack_sizes);
    info.member_scope->template_bound_value_pack_names.swap(
        preserved_template_bound_value_pack_names);
    info.member_scope->values.swap(preserved_values);
    info.member_scope->namespace_bindings.clear();
    info.member_scope->function_sets.clear();
    info.member_scope->function_set_access_overrides.clear();
    info.member_scope->class_templates.swap(preserved_class_templates);
    info.member_scope->function_templates.clear();
    info.member_scope->collected_template_declarations.clear();
    info.member_scope->alias_templates.swap(preserved_alias_templates);
    info.member_scope->variable_templates.clear();
    info.member_scope->using_directives.clear();
    if(info.type && !info.name.empty() &&
       info.member_scope->named_types.count(info.name) == 0) {
      semantic_scope_mutation::bind_named_type(*info.member_scope, info.name, info.type);
    }
  }

  info.reference_members_collected = false;
  info.implicit_special_members_ensured = false;
  info.out_of_class_member_function_template_definitions_applied = false;
  info.out_of_class_member_function_definitions_applied = false;
  info.out_of_class_special_member_definitions_applied = false;
  info.out_of_class_static_member_definitions_applied = false;
}

void invalidate_forward_class_reference_members(
    SemanticContext & ctx,
    ClassInfo & info)
{
  if(!info.reference_members_collected || info.complete) {
    return;
  }
  ctx.discard_class_function_bindings_for_reset(info);
  reset_reference_member_state_for_full_collection(info);
}

void finalize_class_virtuals(SemanticContext & ctx, ClassInfo & info)
{
  ClassInfo * primary_base = primary_polymorphic_base(info);
  bool any_polymorphic_base = false;
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type && info.bases[i].type->is_polymorphic) {
      any_polymorphic_base = true;
      break;
    }
  }

  std::vector<FunctionBinding *> slots =
      primary_base ? primary_base->vtable_entries : std::vector<FunctionBinding *>();
  bool root_virtuals = !slots.empty();

  for(size_t i = 0; i < info.method_declaration_order.size(); ++i) {
    FunctionBinding * binding = info.method_declaration_order[i];
    if(!binding) {
      continue;
    }
    if(binding->is_constructor) {
      if(binding->is_virtual_specified ||
         binding->is_override_specified ||
         binding->is_final) {
        throw std::logic_error("constructors cannot be virtual");
      }
      continue;
    }

    FunctionBinding * overridden = find_overridden_virtual(ctx, info, *binding);
    if(binding->is_override_specified && !overridden) {
      throw std::logic_error("override requires virtual base member");
    }
    if(overridden && overridden->is_final) {
      throw std::logic_error("cannot override final virtual member");
    }
    if(binding->is_final && !binding->is_virtual_specified && !overridden) {
      throw std::logic_error("final requires virtual member");
    }

    if(binding->is_virtual_specified || overridden) {
      binding->is_virtual = true;
      const bool overrides_primary_root_slot =
          overridden &&
          overridden->has_virtual_slot &&
          overridden->virtual_slot < slots.size() &&
          (!primary_base || owner_on_primary_polymorphic_path(info, *overridden));
      if(overrides_primary_root_slot) {
        binding->has_virtual_slot = true;
        binding->virtual_slot = overridden->virtual_slot;
        slots[binding->virtual_slot] = binding;
        if(binding->is_destructor) {
          for(size_t extra = binding->virtual_slot + 1;
              extra < slots.size() && slots[extra] == overridden;
              ++extra) {
            slots[extra] = binding;
          }
        }
      } else if(overridden && overridden->has_virtual_slot &&
                (primary_base || !binding->is_destructor)) {
        binding->has_virtual_slot = true;
        binding->virtual_slot = overridden->virtual_slot;
      } else {
        // Reached for a destructor of a class that introduces its own primary
        // vptr (no non-virtual polymorphic primary base) and whose destructor
        // overrides only a virtual-base slot.  The Itanium ABI places the
        // destructor in every vtable section (primary, each secondary, each
        // virtual base); without giving it a primary slot here the primary
        // section is emitted empty, which propagates empty primary vtables up
        // the basic_istream/ostream/iostream chain.  A non-destructor virtual
        // that overrides only a virtual-base slot stays in the virtual-base
        // section (the branch above) so it is not clobbered by the destructor's
        // primary slot 0 and remains dispatchable through that section.
        binding->has_virtual_slot = true;
        binding->virtual_slot = slots.size();
        slots.push_back(binding);
        if(has_secondary_virtual_destructor_slot(*binding)) {
          slots.push_back(binding);
        }
        root_virtuals = true;
      }
      if(parser_trace::enabled("class.collect")) {
        std::ostringstream trace;
        trace << "virtual-slot-assign class=" << info.qualified_name
              << " binding=" << binding->name
              << " declared="
              << (binding->declared_type ?
                      cpp_decl::describe_type(binding->declared_type) :
                      std::string("<none>"))
              << " slot=" << binding->virtual_slot
              << " overridden="
              << (overridden ? overridden->name : std::string("<none>"));
        parser_trace::note("class.collect", std::string(), trace.str());
      }
    }
  }

  info.vtable_entries = slots;
  info.has_own_vptr = !primary_base && (root_virtuals || any_polymorphic_base);
  info.is_polymorphic = info.has_own_vptr || !slots.empty() || any_polymorphic_base;
  retarget_polymorphic_imported_destructors_to_base_entry(info);
}

bool class_has_virtual_bases(const ClassInfo & info)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual || class_has_virtual_bases(*info.bases[i].type)) {
      return true;
    }
  }
  return false;
}

bool class_uses_extended_virtual_abi(const ClassInfo &)
{
  // Keep vtable entries on the host ABI pointer stride; slot-specific `this`
  // adjustments are represented by thunks, while virtual-base offsets remain in
  // the vtable prefix.
  return false;
}

bool class_needs_vtt(const ClassInfo & info)
{
  return info.is_polymorphic && class_has_virtual_bases(info);
}

std::string construction_vtable_key(const ClassInfo & dynamic_class,
                                    const ClassInfo & base_class,
                                    size_t base_offset)
{
  std::ostringstream out;
  out << dynamic_class.qualified_name
      << "::__construction__"
      << base_class.qualified_name
      << "__"
      << base_offset;
  return out.str();
}

namespace {

bool direct_base_uses_construction_vtt(const BaseInfo & base)
{
  return !base.is_virtual &&
         base.type &&
         base.type->is_polymorphic &&
         class_has_virtual_bases(*base.type);
}

unsigned long long vtable_address_point_offset_for_virtual_bases(const ClassInfo & info)
{
  if(info.virtual_base_subobjects.empty()) {
    return 16ULL;
  }
  return (static_cast<unsigned long long>(info.virtual_base_subobjects.size()) + 2ULL) * 8ULL;
}

// Number of VTT entries contributed by `base`'s construction sub-VTT, computed
// recursively so multi-level virtual-inheritance diamonds reserve the full sub
// tree (a base whose own bases also require construction VTTs contributes more
// than `1 + vbase_count` entries).
std::string construction_section_key(const ClassInfo & dynamic_class,
                                     const ClassInfo & base_class,
                                     size_t base_offset,
                                     size_t section_index)
{
  std::ostringstream out;
  out << construction_vtable_key(dynamic_class, base_class, base_offset)
      << "::s" << section_index;
  return out.str();
}

size_t construction_subvtt_entry_count(const ClassInfo & base)
{
  size_t count = 1;  // primary construction section
  for(size_t i = 0; i < base.bases.size(); ++i) {
    if(direct_base_uses_construction_vtt(base.bases[i]) && base.bases[i].type) {
      count += construction_subvtt_entry_count(*base.bases[i].type);
    }
  }
  if(base.vtables.size() > 1) {
    count += base.vtables.size() - 1;  // secondary / virtual-base sections
  }
  return count;
}

// Append, in Itanium VTT order, the sub-VTT for `view_class` located at
// `view_offset_in_dynamic` within `dynamic_class`.  The top-level call (for the
// dynamic class itself) references the class's own vtable section keys; nested
// calls reference per-(dynamic, base, offset) construction-vtable section keys
// so the embedded sub-VTTs carry dynamic-relative virtual-base offsets.  The
// entry order matches `append_all_vptr_actions` so the regenerated base ctors
// read the slots they were emitted to expect.
void append_subvtt_entries(SemanticContext & ctx,
                           ClassInfo & dynamic_class,
                           ClassInfo & view_class,
                           size_t view_offset_in_dynamic,
                           bool is_top_level,
                           unsigned long long address_point_offset,
                           std::vector<std::pair<std::string, unsigned long long> > & out)
{
  if(view_class.vtables.empty()) {
    return;
  }
  out.push_back(std::make_pair(
      is_top_level ? view_class.vtables[0].key
                   : construction_section_key(dynamic_class, view_class,
                                              view_offset_in_dynamic, 0),
      address_point_offset));

  for(size_t i = 0; i < view_class.bases.size(); ++i) {
    const BaseInfo & base = view_class.bases[i];
    if(!direct_base_uses_construction_vtt(base) || !base.type) {
      continue;
    }
    append_subvtt_entries(ctx,
                          dynamic_class,
                          *base.type,
                          view_offset_in_dynamic + base.offset,
                          false,
                          address_point_offset,
                          out);
  }

  for(size_t i = 1; i < view_class.vtables.size(); ++i) {
    out.push_back(std::make_pair(
        is_top_level ? view_class.vtables[i].key
                     : construction_section_key(dynamic_class, view_class,
                                                view_offset_in_dynamic, i),
        address_point_offset));
  }
}

}  // namespace

void collect_vtt_entries(SemanticContext & ctx,
                         ClassInfo & info,
                         std::vector<std::pair<std::string, unsigned long long> > & out);

void collect_construction_vtables(SemanticContext & ctx,
                                  ClassInfo & dynamic_class,
                                  std::vector<VTableInfo> & out)
{
  if(!class_needs_vtt(dynamic_class)) {
    return;
  }

  // Pre-order walk of every base subobject (at any depth) that itself requires
  // a construction VTT.  Each such subobject S at offset `S_off` gets a full
  // construction-vtable group: a copy of S's own vtable sections (which already
  // carry S's as-of-base final overriders) relocated to `S_off` within
  // `dynamic_class`.  The emission recomputes offset-to-top and virtual-base
  // offsets from `dynamic_class` + the relocated view offset, and each section
  // gets a distinct internal symbol via its construction section key, so a
  // multi-level diamond produces the full set of construction vtables that the
  // recursive VTT (and the regenerated base ctors) reference.
  std::vector<std::pair<ClassInfo *, size_t> > stack;
  for(size_t i = dynamic_class.bases.size(); i-- > 0;) {
    const BaseInfo & base = dynamic_class.bases[i];
    if(direct_base_uses_construction_vtt(base) && base.type) {
      stack.push_back(std::make_pair(base.type, base.offset));
    }
  }
  while(!stack.empty()) {
    ClassInfo & base_class = *stack.back().first;
    const size_t base_offset = stack.back().second;
    stack.pop_back();
    for(size_t i = 0; i < base_class.vtables.size(); ++i) {
      VTableInfo table = base_class.vtables[i];
      size_t section_offset = base_offset + base_class.vtables[i].view_offset;
      // A shared virtual-base section lives at the virtual base's offset within
      // the dynamic class, not at `base_offset` plus its offset inside the base
      // subobject (the virtual base is relocated relative to the base when the
      // base becomes a subobject of a more-derived class).
      if(table.view_type) {
        for(size_t v = 0; v < dynamic_class.virtual_base_subobjects.size(); ++v) {
          const SubobjectInfo & vb = dynamic_class.virtual_base_subobjects[v];
          if(vb.type == table.view_type ||
             (vb.type && vb.type->qualified_name == table.view_type->qualified_name)) {
            section_offset = vb.offset;
            break;
          }
        }
      }
      table.view_offset = section_offset;
      table.key = construction_section_key(dynamic_class, base_class, base_offset, i);
      out.push_back(table);
    }
    for(size_t i = base_class.bases.size(); i-- > 0;) {
      const BaseInfo & inner = base_class.bases[i];
      if(direct_base_uses_construction_vtt(inner) && inner.type) {
        stack.push_back(std::make_pair(inner.type, base_offset + inner.offset));
      }
    }
  }
}

bool find_vtt_direct_base_slice_offset(SemanticContext & ctx,
                                       ClassInfo & dynamic_class,
                                       ClassInfo & base_class,
                                       size_t & out_byte_offset)
{
  if(!class_needs_vtt(dynamic_class)) {
    return false;
  }

  size_t entry_index = 1;
  for(size_t i = 0; i < dynamic_class.bases.size(); ++i) {
    const BaseInfo & base = dynamic_class.bases[i];
    if(!direct_base_uses_construction_vtt(base)) {
      continue;
    }
    if(base.type == &base_class ||
       (base.type && base.type->qualified_name == base_class.qualified_name)) {
      out_byte_offset = entry_index * 8;
      return true;
    }
    entry_index += construction_subvtt_entry_count(*base.type);
  }

  return false;
}

bool find_vtt_self_table_index(SemanticContext & ctx,
                               ClassInfo & info,
                               const std::string & table_key,
                               size_t & out_index)
{
  if(!class_needs_vtt(info)) {
    return false;
  }

  std::vector<std::pair<std::string, unsigned long long> > entries;
  collect_vtt_entries(ctx, info, entries);
  for(size_t i = entries.size(); i-- > 0;) {
    if(entries[i].first == table_key) {
      out_index = i;
      return true;
    }
  }
  return false;
}

void collect_vtt_entries(SemanticContext & ctx,
                         ClassInfo & info,
                         std::vector<std::pair<std::string, unsigned long long> > & out)
{
  out.clear();
  if(!class_needs_vtt(info) || info.vtables.empty()) {
    return;
  }

  const unsigned long long address_point_offset =
      vtable_address_point_offset_for_virtual_bases(info);
  // Emit the VTT in recursive Itanium order so multi-level virtual-inheritance
  // diamonds embed each direct base's full sub-VTT (and the base ctors, which
  // pass their bases recursively-sized slices, read consistent entries).
  append_subvtt_entries(ctx, info, info, 0, true, address_point_offset, out);
}

void finalize_class_layout(SemanticContext & ctx,
                           ClassInfo & info)
{
  const size_t declared_alignment =
      evaluate_declared_alignment(ctx,
                                  *info.member_scope,
                                  info.class_node ? info.class_node :
                                      info.template_output_node);
  if(is_union_class_info(info)) {
    std::size_t class_size = 0;
    std::size_t class_alignment = 1;
    info.complete_subobjects.clear();
    info.virtual_base_subobjects.clear();
    info.vtables.clear();

    for(std::size_t i = 0; i < info.fields.size(); ++i) {
      FieldInfo & field = info.fields[i];
      std::size_t field_alignment = 1;
      std::size_t field_size = 0;
      try {
        field_alignment = cpp_decl::type_alignment(field.type);
        field_size = cpp_decl::type_size(field.type);
      } catch(const std::logic_error & e) {
        std::ostringstream out;
        out << e.what() << " [union " << info.qualified_name;
        if(!field.name.empty()) {
          out << " field " << field.name;
        }
        out << " type " << cpp_decl::describe_type(field.type) << "]";
        throw TemplateSubstitutionFailure(out.str());
      }

      field.offset = 0;
      if(field.is_bit_field) {
        const std::size_t width = ctx.evaluate_bit_field_width(info, field);
        const std::size_t storage_bits = field_size * 8;
        if(width == 0 && !field.name.empty()) {
          throw std::logic_error("named zero-width bit-field unsupported");
        }
        if(width > storage_bits) {
          throw std::logic_error("bit-field width exceeds storage unit");
        }
        field.bit_width = width;
        field.bit_offset = 0;
        field.bit_storage_size = field_size;
      }

      class_alignment = std::max(class_alignment, field_alignment);
      class_size = std::max(class_size, field_size);

      std::map<std::string, ValueBinding>::iterator found =
          info.member_scope->values.find(field.name);
      if(found != info.member_scope->values.end()) {
        found->second.field_offset = 0;
        found->second.is_bit_field = field.is_bit_field;
        found->second.bit_field_width = field.bit_width;
        found->second.bit_field_offset = field.bit_offset;
        found->second.bit_field_storage_size = field.bit_storage_size;
      }
    }

    class_alignment = std::max(class_alignment, declared_alignment);

    if(class_size == 0) {
      class_size = 1;
    }
    class_size = align_up(class_size, class_alignment);

    info.nonvirtual_alignment = class_alignment;
    info.nonvirtual_size = class_size;
    info.type->named_complete = true;
    info.type->named_has_layout = true;
    info.type->named_alignment = class_alignment;
    info.type->named_size = class_size;
    info.type->named_is_empty = ctx.is_empty_class_info(&info);
    info.type->named_host_abi_chunks.clear();
    info.complete = true;
    sync_anonymous_storage_member_bindings(ctx, info);
    return;
  }

  size_t class_size = 0;
  size_t class_alignment = 1;
  size_t open_bit_offset = 0;
  size_t open_bit_size_bits = 0;
  size_t open_bit_used_bits = 0;
  size_t open_bit_alignment = 1;
  TypePtr open_bit_type;
  info.complete_subobjects.clear();
  info.virtual_base_subobjects.clear();
  info.vtables.clear();

  ClassInfo * primary_base = primary_polymorphic_base(info);

  if(info.has_own_vptr) {
    class_size = 8;
    class_alignment = 8;
  }

  std::vector<SubobjectInfo> placed_nonvirtual_subobjects;

  const auto place_nonvirtual_base =
      [&](BaseInfo & base)
      {
        size_t base_alignment = 0;
        try {
          base_alignment = cpp_decl::type_alignment(base.type->type);
        } catch(const std::logic_error & e) {
          std::ostringstream out;
          out << e.what() << " [class " << info.qualified_name
              << " nonvirtual base " << base.type->qualified_name;
          out << " type " << cpp_decl::describe_type(base.type->type);
          out << "]";
          throw TemplateSubstitutionFailure(out.str());
        }
        const size_t base_size = base.type->nonvirtual_size;
        const bool elide_empty_base = ctx.is_empty_class_info(base.type);
        size_t base_offset = align_up(class_size, base_alignment);
        if(elide_empty_base) {
          const size_t search_limit = class_size;
          for(size_t candidate = 0; candidate <= search_limit; ++candidate) {
            if((candidate % base_alignment) != 0) {
              continue;
            }
            if(!placement_conflicts_same_type_subobject(placed_nonvirtual_subobjects,
                                                        *base.type,
                                                        candidate)) {
              base_offset = candidate;
              break;
            }
          }
        }
        base.offset = base_offset;
        if(elide_empty_base) {
          class_size = std::max(class_size, base.offset == 0 ? std::size_t(0) :
                                                base.offset + 1);
        } else {
          class_size = base.offset + base_size;
        }
        class_alignment = std::max(class_alignment, base_alignment);
        record_placed_nonvirtual_subobjects(placed_nonvirtual_subobjects,
                                            *base.type,
                                            base.offset,
                                            base.access);
      };

  if(primary_base) {
    for(size_t i = 0; i < info.bases.size(); ++i) {
      BaseInfo & base = info.bases[i];
      if(!base.is_virtual && base.type == primary_base) {
        place_nonvirtual_base(base);
        break;
      }
    }
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    BaseInfo & base = info.bases[i];
    if(base.is_virtual) {
      continue;
    }
    if(primary_base && base.type == primary_base) {
      continue;
    }
    place_nonvirtual_base(base);
    open_bit_size_bits = 0;
    open_bit_used_bits = 0;
    open_bit_alignment = 1;
    open_bit_type.reset();
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(info.fields[i].is_bit_field) {
      size_t field_alignment = 0;
      size_t field_size = 0;
      try {
        field_alignment = cpp_decl::type_alignment(info.fields[i].type);
        field_size = cpp_decl::type_size(info.fields[i].type);
      } catch(const std::logic_error & e) {
        std::ostringstream out;
        out << e.what() << " [class " << info.qualified_name
            << " bit-field " << info.fields[i].name;
        out << " type " << cpp_decl::describe_type(info.fields[i].type);
        out << "]";
        throw std::logic_error(out.str());
      }

      const size_t storage_bits = field_size * 8;
      const size_t width = ctx.evaluate_bit_field_width(info, info.fields[i]);
      if(width == 0) {
        if(!info.fields[i].name.empty()) {
          throw std::logic_error("named zero-width bit-field unsupported");
        }
        class_size = align_up(class_size, field_alignment);
        class_alignment = std::max(class_alignment, field_alignment);
        open_bit_size_bits = 0;
        open_bit_used_bits = 0;
        open_bit_alignment = 1;
        open_bit_type.reset();
        info.fields[i].bit_width = 0;
        info.fields[i].bit_storage_size = field_size;
        continue;
      }
      if(width > storage_bits) {
        throw std::logic_error("bit-field width exceeds storage unit");
      }

      const bool can_reuse_container =
          open_bit_size_bits == storage_bits &&
          open_bit_alignment == field_alignment &&
          open_bit_type &&
          same_type_with_compatible_top_cv(strip_top_level_cv(open_bit_type),
                                           strip_top_level_cv(info.fields[i].type)) &&
          open_bit_used_bits + width <= open_bit_size_bits;
      if(!can_reuse_container) {
        class_size = align_up(class_size, field_alignment);
        open_bit_offset = class_size;
        class_size += field_size;
        open_bit_size_bits = storage_bits;
        open_bit_used_bits = 0;
        open_bit_alignment = field_alignment;
        open_bit_type = info.fields[i].type;
      }

      info.fields[i].offset = open_bit_offset;
      info.fields[i].bit_offset = open_bit_used_bits;
      info.fields[i].bit_width = width;
      info.fields[i].bit_storage_size = field_size;
      open_bit_used_bits += width;
      class_alignment = std::max(class_alignment, field_alignment);

      std::map<std::string, ValueBinding>::iterator found =
          info.member_scope->values.find(info.fields[i].name);
      if(found != info.member_scope->values.end()) {
        found->second.field_offset = info.fields[i].offset;
        found->second.is_bit_field = true;
        found->second.bit_field_width = info.fields[i].bit_width;
        found->second.bit_field_offset = info.fields[i].bit_offset;
        found->second.bit_field_storage_size = info.fields[i].bit_storage_size;
      }
      continue;
    }

    size_t field_alignment = 0;
    size_t field_size = 0;
    try {
      field_alignment = cpp_decl::type_alignment(info.fields[i].type);
      field_size = cpp_decl::type_size(info.fields[i].type);
    } catch(const std::logic_error & e) {
      maybe_complete_class_member_object_type(ctx, info.fields[i].type);
      try {
        field_alignment = cpp_decl::type_alignment(info.fields[i].type);
        field_size = cpp_decl::type_size(info.fields[i].type);
      } catch(const std::logic_error &) {
      std::ostringstream out;
      out << e.what() << " [class " << info.qualified_name
          << " field " << info.fields[i].name;
      out << " type " << cpp_decl::describe_type(info.fields[i].type);
      out << "]";
      throw TemplateSubstitutionFailure(out.str());
      }
    }
    bool elide_empty_no_unique_address = false;
    if(info.fields[i].is_no_unique_address) {
      if(ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type)) {
        elide_empty_no_unique_address = ctx.is_empty_class_info(field_class);
      }
    }
    open_bit_size_bits = 0;
    open_bit_used_bits = 0;
    open_bit_alignment = 1;
    open_bit_type.reset();
    ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
    size_t field_offset = align_up(class_size, field_alignment);
    if(elide_empty_no_unique_address && field_class) {
      const size_t search_limit = class_size;
      for(size_t candidate = 0; candidate <= search_limit; ++candidate) {
        if((candidate % field_alignment) != 0) {
          continue;
        }
        if(!placement_conflicts_same_type_subobject(placed_nonvirtual_subobjects,
                                                    *field_class,
                                                    candidate)) {
          field_offset = candidate;
          break;
        }
      }
    } else {
      while(field_class &&
            placement_conflicts_same_type_subobject(placed_nonvirtual_subobjects,
                                                    *field_class,
                                                    field_offset)) {
        ++field_offset;
      }
    }
    info.fields[i].offset = field_offset;
    if(!elide_empty_no_unique_address) {
      class_size = info.fields[i].offset + field_size;
    } else {
      class_size = std::max(class_size, info.fields[i].offset == 0 ? std::size_t(0) :
                                            info.fields[i].offset + 1);
    }
    class_alignment = std::max(class_alignment, field_alignment);
    if(field_class) {
      record_placed_nonvirtual_subobjects(placed_nonvirtual_subobjects,
                                          *field_class,
                                          info.fields[i].offset,
                                          MA_PUBLIC);
    }
    std::map<std::string, ValueBinding>::iterator found = info.member_scope->values.find(info.fields[i].name);
    if(found != info.member_scope->values.end()) {
      found->second.field_offset = info.fields[i].offset;
    }
  }

  class_alignment = std::max(class_alignment, declared_alignment);

  if(class_size == 0) {
    class_size = 1;
  }
  class_size = align_up(class_size, class_alignment);

  info.nonvirtual_alignment = class_alignment;
  info.nonvirtual_size = class_size;

  std::vector<ClassInfo *> virtual_bases;
  std::set<ClassInfo *> seen_virtual_bases;
  collect_unique_virtual_bases(info, virtual_bases, seen_virtual_bases);
  std::map<ClassInfo *, size_t> virtual_offsets;
  for(size_t i = 0; i < virtual_bases.size(); ++i) {
    ClassInfo * base = virtual_bases[i];
    const size_t base_alignment = base->nonvirtual_alignment;
    const size_t base_size = base->nonvirtual_size;
    class_size = align_up(class_size, base_alignment);
    virtual_offsets[base] = class_size;
    class_size += base_size;
    class_alignment = std::max(class_alignment, base_alignment);
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual) {
      info.bases[i].offset = virtual_offsets[info.bases[i].type];
    }
  }

  class_size = align_up(class_size, class_alignment);

  append_nonvirtual_subobjects(info, 0, MA_PUBLIC, false, info.complete_subobjects);
  for(size_t i = 0; i < virtual_bases.size(); ++i) {
    append_nonvirtual_subobjects(*virtual_bases[i],
                                 virtual_offsets[virtual_bases[i]],
                                 MA_PUBLIC,
                                 true,
                                 info.complete_subobjects);
    SubobjectInfo subobject;
    subobject.type = virtual_bases[i];
    subobject.offset = virtual_offsets[virtual_bases[i]];
    subobject.is_virtual = true;
    info.virtual_base_subobjects.push_back(subobject);
  }

  if(info.is_polymorphic) {
    VTableInfo root;
    root.key = vtable_view_key(info, info, 0);
    root.view_type = &info;
    root.view_offset = 0;
    root.use_extended_layout = class_uses_extended_virtual_abi(info);
    for(size_t i = 0; i < info.vtable_entries.size(); ++i) {
      FunctionBinding * base_virtual = info.vtable_entries[i];
      FunctionBinding * final = find_final_overrider(ctx, info, *base_virtual);
      size_t target_offset = 0;
      MemberAccess target_access = MA_PUBLIC;
      if(!semantic_lookup::find_unique_base_path(info, final->owner_class, target_offset, target_access)) {
        throw std::logic_error("missing final overrider path");
      }
      VTableSlotInfo slot;
      slot.function = final;
      slot.this_adjust =
          static_cast<long long>(target_offset) - static_cast<long long>(root.view_offset);
      root.slots.push_back(slot);
    }
    info.vtables.push_back(root);
  }

  for(size_t i = 0; i < info.complete_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = info.complete_subobjects[i];
    if(subobject.offset == 0 || !subobject.type->is_polymorphic) {
      continue;
    }
    VTableInfo table;
    table.key = vtable_view_key(info, *subobject.type, subobject.offset);
    table.view_type = subobject.type;
    table.view_offset = subobject.offset;
    table.use_extended_layout = class_uses_extended_virtual_abi(*subobject.type);
    for(size_t j = 0; j < subobject.type->vtable_entries.size(); ++j) {
      FunctionBinding * base_virtual = subobject.type->vtable_entries[j];
      FunctionBinding * final = find_final_overrider(ctx, info, *base_virtual);
      size_t target_offset = 0;
      MemberAccess target_access = MA_PUBLIC;
      if(!semantic_lookup::find_unique_base_path(info, final->owner_class, target_offset, target_access)) {
        throw std::logic_error("missing final overrider path");
      }
      VTableSlotInfo slot;
      slot.function = final;
      slot.this_adjust =
          static_cast<long long>(target_offset) - static_cast<long long>(subobject.offset);
      table.slots.push_back(slot);
    }
    info.vtables.push_back(table);
  }

  info.type->named_complete = true;
  info.type->named_has_layout = true;
  info.type->named_alignment = class_alignment;
  info.type->named_size = class_size;
  info.type->named_is_empty = ctx.is_empty_class_info(&info);
  info.complete = true;
  info.type->named_host_abi_chunks.clear();
  collect_host_direct_integer_abi_chunks(ctx, info.type, info.type->named_host_abi_chunks);
  sync_anonymous_storage_member_bindings(ctx, info);
}

namespace {

FunctionBinding * register_class_function(
    SemanticContext & ctx,
    ClassInfo & info,
    const std::string & simple_name,
    const TypePtr & declared_type,
    const std::vector<std::pair<std::string, TypePtr> > & explicit_params,
    const std::vector<const CppAstNode *> & explicit_default_arguments,
    const CppAstNode * body,
    const CppAstNode * ctor_initializer,
    const ClassFunctionOptions & options,
    const CppAstNode * declaration_node = nullptr)
{
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = simple_name;
  request.declared_type = declared_type;
  request.params = explicit_params;
  request.default_arguments = explicit_default_arguments;
  request.body = body;
  request.ctor_initializer = ctor_initializer;
  request.declaration_node = declaration_node;
  request.semantic_flags = options;
  return ctx.register_function_entity(request);
}

bool body_contains_static_assert_declaration(const CppAstNode & node)
{
  if(node.kind == CppAstKind::static_assert_declaration) {
    return true;
  }
  if(node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration ||
     node.kind == CppAstKind::function_definition ||
     node.kind == CppAstKind::template_declaration) {
    return false;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(body_contains_static_assert_declaration(node.children[i])) {
      return true;
    }
  }
  return false;
}

void validate_member_function_static_asserts(SemanticContext & ctx,
                                             FunctionBinding * binding)
{
  if(!binding ||
     !binding->body ||
     !binding->owner_class ||
     !binding->owner_class->member_scope ||
     !body_contains_static_assert_declaration(*binding->body) ||
     template_api::function_binding_has_template_identity(binding) ||
     template_api::class_has_template_identity(binding->owner_class)) {
    return;
  }

  semantic_output::validate_function_body_and_cache_output(
      ctx, *binding->owner_class->member_scope, *binding);
}

void validate_class_member_function_static_asserts(SemanticContext & ctx,
                                                    ClassInfo & info)
{
  for(std::size_t i = 0; i < info.method_declaration_order.size(); ++i) {
    validate_member_function_static_asserts(ctx, info.method_declaration_order[i]);
  }
}

bool class_definition_has_member_function_static_assert(const CppAstNode & node)
{
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::function_definition &&
       child.kind != CppAstKind::special_member_definition) {
      continue;
    }
    const CppAstNode * body = find_class_function_body_node(child);
    if(body && body_contains_static_assert_declaration(*body)) {
      return true;
    }
  }
  return false;
}

bool function_definition_is_static_constexpr_member(const CppAstNode & node)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  return specifiers &&
         decl_spec_contains_token(*specifiers, KW_STATIC) &&
         decl_spec_contains_token(*specifiers, KW_CONSTEXPR);
}

TypePtr resolve_class_lexical_type(SemanticContext & ctx,
                                   ClassInfo & info,
                                   const TypePtr & type)
{
  if(!type || !info.member_scope || !ctx.type_depends_on_template_parameter(type)) {
    return type;
  }
  TypePtr resolved;
  if(semantic_dependent_type::resolve_instantiated_dependent_type(
         ctx, *info.member_scope, type, resolved) &&
     resolved) {
    return resolved;
  }
  return type;
}

void resolve_class_lexical_params(SemanticContext & ctx,
                                  ClassInfo & info,
                                  std::vector<std::pair<std::string, TypePtr> > & params)
{
  for(size_t i = 0; i < params.size(); ++i) {
    params[i].second = resolve_class_lexical_type(ctx, info, params[i].second);
  }
}

bool register_friend_function_binding(SemanticContext & ctx,
                                      ClassInfo & info,
                                      const std::string & friend_name,
                                      const QualifiedName * friend_name_syntax,
                                      const TypePtr & friend_type,
                                      const std::vector<std::pair<std::string, TypePtr> > & params,
                                      const std::vector<const CppAstNode *> & default_args,
                                      const CppAstNode * body,
                                      const CppAstNode * declaration_node,
                                      const CppAstNode * function_qualifier,
                                      bool is_constexpr)
{
  TypePtr effective_friend_type = resolve_class_lexical_type(ctx, info, friend_type);
  std::vector<std::pair<std::string, TypePtr> > effective_params = params;
  resolve_class_lexical_params(ctx, info, effective_params);
  if(class_instantiation_is_dependent(ctx, info) &&
     ctx.type_depends_on_template_parameter(effective_friend_type)) {
    // Hidden friends in class templates become ordinary namespace functions
    // for concrete specializations. Registering the dependent pattern itself
    // leaves a stale ADL candidate beside the specialization.
    return false;
  }
  if(info.reference_member_collection_in_progress &&
     !class_instantiation_is_dependent(ctx, info) &&
     ctx.type_depends_on_template_parameter(effective_friend_type)) {
    // Concrete reference collection can see inherited aliases before their
    // instantiated type is available. Let full collection register the friend
    // rather than leaving a stale dependent overload candidate behind.
    return false;
  }

  TypePtr stripped = strip_top_level_cv(effective_friend_type);
  if(!stripped || stripped->kind != Type::TK_FUNCTION) {
    return false;
  }

  Scope * friend_scope = unqualified_friend_entity_scope(info);
  Scope * registration_scope = friend_scope;
  std::string registration_name = friend_name;
  const bool qualified_friend_name =
      friend_name_syntax && !friend_name_syntax->qualifiers.empty();
  if(qualified_friend_name) {
    Scope * qualified_scope =
        resolve_qualified_scope_for_class_or_namespace(ctx, *info.member_scope,
                                                       *friend_name_syntax);
    if(!qualified_scope) {
      return false;
    }
    registration_scope = qualified_scope;
    registration_name = friend_name_syntax->name;
  }

  FunctionRegistrationRequest request;
  request.scope = registration_scope;
  request.name = registration_name;
  request.declared_type = effective_friend_type;
  request.params = effective_params;
  request.default_arguments = default_args;
  request.body = body;
  request.declaration_node = declaration_node;
  request.declaration_scope = registration_scope;
  request.function_qualifier = function_qualifier;
  request.is_constexpr = is_constexpr;
  request.lexical_access_class = &info;
  request.hidden_friend_only = !qualified_friend_name;
  ctx.register_function_entity(request);
  FunctionBinding * binding =
      ctx.find_exact_function(*registration_scope, registration_name, effective_friend_type);
  if(binding &&
     std::find(info.friend_access_functions.begin(),
               info.friend_access_functions.end(),
               binding) == info.friend_access_functions.end()) {
    info.friend_access_functions.push_back(binding);
  }
  if(!qualified_friend_name &&
     binding &&
     std::find(info.friend_functions.begin(), info.friend_functions.end(), binding) ==
         info.friend_functions.end()) {
    info.friend_functions.push_back(binding);
  }
  return binding != nullptr;
}

bool register_friend_function_template_binding(SemanticContext & ctx,
                                               ClassInfo & info,
                                               const std::string & friend_name,
                                               const QualifiedName * friend_name_syntax,
                                               const TemplateIdSyntax *
                                                   friend_name_template_id_syntax,
                                               const TypePtr & friend_type)
{
  const auto append_unique_friend_template_access =
      [](FunctionTemplateDecl & decl, ClassInfo & access_class)
  {
    if(std::find(decl.friend_access_classes.begin(),
                 decl.friend_access_classes.end(),
                 &access_class) == decl.friend_access_classes.end()) {
      decl.friend_access_classes.push_back(&access_class);
    }
    if(std::find(access_class.friend_access_function_templates.begin(),
                 access_class.friend_access_function_templates.end(),
                 &decl) == access_class.friend_access_function_templates.end()) {
      access_class.friend_access_function_templates.push_back(&decl);
    }
  };
  (void)friend_name;
  const auto type_mentions_class =
      [&](const TypePtr & type, const ClassInfo & target) -> bool
  {
    std::function<bool(const TypePtr &)> visit =
        [&](const TypePtr & current) -> bool
    {
      if(!current) {
        return false;
      }
      if(type_equals(strip_top_level_cv(current), strip_top_level_cv(target.type))) {
        return true;
      }
      switch(current->kind) {
      case Type::TK_FUNDAMENTAL:
        return false;
      case Type::TK_NAMED:
        return false;
      case Type::TK_CV:
      case Type::TK_ATOMIC:
      case Type::TK_POINTER:
      case Type::TK_BLOCK_POINTER:
      case Type::TK_LVALUE_REFERENCE:
      case Type::TK_RVALUE_REFERENCE:
      case Type::TK_ARRAY:
        return visit(current->inner);
      case Type::TK_MEMBER_POINTER:
        return visit(current->owner) || visit(current->inner);
      case Type::TK_FUNCTION:
        if(visit(current->inner)) {
          return true;
        }
        for(size_t i = 0; i < current->params.size(); ++i) {
          if(visit(current->params[i])) {
            return true;
          }
        }
        return false;
      }
      return false;
    };
    return visit(type);
  };
  const auto enclosing_template_arguments =
      [&](const FunctionTemplateDecl & decl,
          std::vector<template_model::TemplateArgument> & out) -> bool
  {
    out.clear();
    if(info.instantiation_arguments.size() == decl.parameters.size() &&
       template_model::template_arguments_fully_bind_parameters(
           decl.parameters,
           info.instantiation_arguments)) {
      out = info.instantiation_arguments;
      return true;
    }
    if(!info.member_scope) {
      return false;
    }

    std::vector<template_model::TemplateArgument> arguments;
    for(size_t j = 0; j < decl.parameters.size(); ++j) {
      const template_model::TemplateParameterInfo & parameter = decl.parameters[j];
      template_model::TemplateArgument argument;
      if(parameter.kind == template_model::TemplateParameterInfo::TP_TYPE) {
        auto found = info.member_scope->named_types.find(parameter.name);
        if(found == info.member_scope->named_types.end()) {
          return false;
        }
        argument.kind = template_model::TemplateArgument::TA_TYPE;
        argument.type = found->second;
      } else {
        return false;
      }
      arguments.push_back(argument);
    }
    if(!template_model::template_arguments_fully_bind_parameters(decl.parameters,
                                                                 arguments)) {
      return false;
    }
    out.swap(arguments);
    return true;
  };
  Scope * friend_scope = unqualified_friend_entity_scope(info);
  Scope * lookup_scope = friend_scope;
  const bool qualified_friend_name =
      friend_name_syntax && !friend_name_syntax->qualifiers.empty();
  if(qualified_friend_name) {
    Scope * qualified_scope =
        resolve_qualified_scope_for_class_or_namespace(ctx, *info.member_scope,
                                                       *friend_name_syntax);
    if(!qualified_scope) {
      return false;
    }
    lookup_scope = qualified_scope;
  }
  if(!lookup_scope) {
    return false;
  }

  if(!friend_name_template_id_syntax ||
     friend_name_template_id_syntax->name.name.empty()) {
    return false;
  }
  const std::string & template_name = friend_name_template_id_syntax->name.name;
  const std::vector<std::string> & arg_texts =
      friend_name_template_id_syntax->arguments;

  const bool qualified_lookup = qualified_friend_name;
  const std::vector<FunctionTemplateDecl *> templates =
      qualified_lookup ? lookup_direct_function_templates(*lookup_scope, template_name) :
                         ctx.lookup_function_templates(*info.member_scope, template_name);
  bool matched = false;
  for(size_t i = 0; i < templates.size(); ++i) {
    FunctionTemplateDecl * decl = templates[i];
    if(!decl) {
      continue;
    }

    std::vector<template_model::TemplateArgument> arguments;
    if(arg_texts.empty()) {
      FunctionTemplateDecl adjusted = *decl;
      Scope canonical_scope(adjusted.declaring_scope ? adjusted.declaring_scope :
                                                    info.member_scope.get(),
                            "",
                            false);
      if(adjusted.declaring_scope) {
        canonical_scope.class_info = adjusted.declaring_scope->class_info;
        canonical_scope.function = adjusted.declaring_scope->function;
      }
      for(size_t j = 0; j < adjusted.parameters.size(); ++j) {
        const template_model::TemplateParameterInfo & parameter = adjusted.parameters[j];
        if(parameter.name.empty()) {
          continue;
        }
        if(parameter.kind == template_model::TemplateParameterInfo::TP_TYPE) {
          std::string placeholder_payload = parameter.placeholder_key;
          static const char template_parameter_prefix[] =
              "template-parameter ";
          if(placeholder_payload.compare(
                 0,
                 sizeof(template_parameter_prefix) - 1,
                 template_parameter_prefix) == 0) {
            placeholder_payload =
                placeholder_payload.substr(
                    sizeof(template_parameter_prefix) - 1);
          }
          semantic_scope_mutation::ensure_template_named_type(
              canonical_scope,
              parameter.name,
              make_template_parameter_type(
                  std::string("typename ") + parameter.name,
                  placeholder_payload.empty() ?
                      parameter.name :
                      placeholder_payload,
                  parameter.name));
        } else if(parameter.kind == template_model::TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
          semantic_scope_mutation::bind_template_template_parameter(canonical_scope,
                                                                    parameter.name,
                                                                    nullptr);
        } else {
          semantic_scope_mutation::bind_dependent_template_value(canonical_scope,
                                                                 parameter.name,
                                                                 parameter.value_type);
        }
      }
	      adjusted.type_pattern =
	          canonicalize_member_typedef_type(ctx,
	                                           canonical_scope,
	                                           adjusted.type_pattern,
	                                           &info);
	      for(size_t j = 0; j < adjusted.params_pattern.size(); ++j) {
	        adjusted.params_pattern[j].second =
	            canonicalize_member_typedef_type(ctx,
	                                             canonical_scope,
	                                             adjusted.params_pattern[j].second,
	                                             &info);
	      }
	      TypePtr friend_function =
	          strip_top_level_cv(canonicalize_member_typedef_type(ctx,
	                                                              *info.member_scope,
	                                                              friend_type,
	                                                              &info));
      if(!friend_function || friend_function->kind != Type::TK_FUNCTION) {
        continue;
      }
      if(type_mentions_class(friend_function, info) &&
         enclosing_template_arguments(*decl, arguments)) {
        // The empty template-id names the current class specialization's
        // matching friend, so no deduction probe is needed.
      } else {
        std::vector<ExprInfo> deduction_args;
        deduction_args.reserve(friend_function->params.size());
        for(size_t j = 0; j < friend_function->params.size(); ++j) {
          ExprInfo arg;
          arg.type = friend_function->params[j];
          TypePtr arg_base = strip_top_level_cv(arg.type);
          if(arg_base && arg_base->kind == Type::TK_LVALUE_REFERENCE) {
            arg.category = VC_LVALUE;
          } else if(arg_base && arg_base->kind == Type::TK_RVALUE_REFERENCE) {
            arg.category = VC_XVALUE;
          } else {
            arg.category = VC_PRVALUE;
          }
          deduction_args.push_back(arg);
        }
        semantic_template_function::FunctionTemplateDeduction result;
        if(!semantic_template_function::deduce_function_template_from_arguments(
               ctx, adjusted, deduction_args, info.member_scope.get(), result)) {
          continue;
        }
        arguments.swap(result.arguments);
      }
    } else {
      if(!semantic_template_function::resolve_explicit_function_template_arguments(
             ctx,
             *decl,
             *info.member_scope,
             arg_texts,
             arguments)) {
        continue;
      }
    }

    FunctionBinding * binding = nullptr;
    try {
      binding = semantic_template_function::acquire_function_template_binding(
          ctx,
          *decl,
          arguments,
          info.member_scope.get(),
          nullptr,
          false);
    } catch(const TemplateSubstitutionFailure &) {
      continue;
    } catch(const std::logic_error &) {
      continue;
    }
    if(!binding) {
      continue;
    }

    if(!qualified_friend_name &&
       std::find(info.friend_functions.begin(),
                 info.friend_functions.end(),
                 binding) == info.friend_functions.end()) {
      info.friend_functions.push_back(binding);
    }
    if(!qualified_friend_name &&
       std::find(info.friend_function_templates.begin(),
                 info.friend_function_templates.end(),
                 decl) == info.friend_function_templates.end()) {
      info.friend_function_templates.push_back(decl);
    }
    append_unique_friend_template_access(*decl, info);
    matched = true;
  }

  return matched;
}

void append_friend_class_name(ClassInfo & info, const std::string & friend_name)
{
  if(friend_name.empty()) {
    return;
  }
  if(std::find(info.friend_class_names.begin(),
               info.friend_class_names.end(),
               friend_name) == info.friend_class_names.end()) {
    info.friend_class_names.push_back(friend_name);
  }
}

void append_unresolved_friend_class_names(SemanticContext & ctx,
                                          ClassInfo & info,
                                          const CppAstNode & specifiers)
{
  const bool dependent_class = class_instantiation_is_dependent(ctx, info);
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(node_has_simple_type(child, KW_FRIEND)) {
      continue;
    }
    std::string friend_name = child.value;
    if(friend_name.empty()) {
      friend_name = node_text(child);
    }
    if(friend_name.empty()) {
      continue;
    }
    if(dependent_class &&
       (child.kind == CppAstKind::class_forward_declaration ||
        child.kind == CppAstKind::class_specifier)) {
      append_friend_class_name(info, friend_name);
      continue;
    }
    TypePtr friend_type = child.semantic_type;
    if(!friend_type) {
      friend_type = ctx.lookup_type_node(*info.member_scope,
                                         child,
                                         friend_name,
                                         true);
    }
    if(friend_type && !ctx.type_depends_on_template_parameter(friend_type)) {
      const std::string resolved_name =
          semantic_dependent_type::lookup_type_argument_text(ctx, friend_type);
      if(!resolved_name.empty()) {
        friend_name = resolved_name;
      }
    }
    append_friend_class_name(info, friend_name);
  }
}

void collect_class_friend_function_definition(SemanticContext & ctx,
                                              ClassInfo & info,
                                              const CppAstNode & node,
                                              bool keep_body)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  const CppAstNode * body = find_class_function_body_node(node);
  if(!specifiers || !declarator || !body) {
    return;
  }

  bool is_typedef = false;
  TypePtr base;
  PreparedMethodParseContext prepared_method;
  prepare_method_parse_context(specifiers, *declarator, prepared_method);
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(*specifiers);
  if(!ctx.parse_function_definition_base(*info.member_scope,
                                         filtered_specifiers,
                                         prepared_method.parse_declarator_node(),
                                         *body,
                                         prepared_method.syntax.is_const_method,
                                         prepared_method.syntax.is_volatile_method,
                                         is_typedef,
                                         base,
                                         true) ||
     is_typedef) {
    return;
  }

  std::string friend_name;
  TypePtr friend_type;
  const CppAstNode friend_declarator =
      function_declarator_without_trailing_return(
          prepared_method.parse_declarator_node());
  if(!ctx.parse_declarator(*info.member_scope,
                           friend_declarator,
                           base, friend_name, friend_type, true) ||
     friend_name.empty()) {
    return;
  }
  const QualifiedName * friend_name_syntax =
      first_identifier_name_syntax_in_subtree(prepared_method.parse_declarator_node());

  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const CppAstNode * parameter_clause =
      find_child(*declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(
         *info.member_scope, *parameter_clause, params, &default_args, true)) {
    return;
  }

  register_friend_function_binding(ctx,
                                   info,
                                   friend_name,
                                   friend_name_syntax,
                                   friend_type,
                                   params,
                                   default_args,
                                   keep_body ? body : nullptr,
                                   &node,
                                   declarator_function_qualifier(*declarator),
                                   decl_spec_contains_token(*specifiers, KW_CONSTEXPR));
}

void collect_class_friend_declaration(SemanticContext & ctx,
                                      ClassInfo & info,
                                      const CppAstNode & node)
{
  std::function<void(const CppAstNode &)> collect_friend_class_names =
      [&](const CppAstNode & current)
  {
    if(current.kind == CppAstKind::template_declaration) {
      for(size_t i = 0; i < current.children.size(); ++i) {
        collect_friend_class_names(current.children[i]);
      }
      return;
    }
    if(current.kind != CppAstKind::simple_declaration) {
      return;
    }

    const CppAstNode * current_specifiers =
        find_child(current, CppAstKind::decl_specifier_seq);
    if(!current_specifiers) {
      return;
    }
    const bool has_friend =
        any_of(current_specifiers->children.begin(),
               current_specifiers->children.end(),
               [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); });
    if(!has_friend) {
      return;
    }

    for(size_t i = 0; i < current_specifiers->children.size(); ++i) {
      const CppAstNode & child = current_specifiers->children[i];
      if((child.kind == CppAstKind::class_forward_declaration ||
          child.kind == CppAstKind::class_specifier) &&
         !child.value.empty()) {
        append_friend_class_name(info, child.value);
      }
    }
  };
  collect_friend_class_names(node);

  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child(node, CppAstKind::init_declarator_list);
  if(!specifiers) {
    return;
  }

  if(!declarators || declarators->children.empty()) {
    append_unresolved_friend_class_names(ctx, info, *specifiers);
    return;
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, declarators, true,
                                             true, resolved_specifiers)) {
    return;
  }

  bool is_typedef = false;
  TypePtr base;
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  if(!ctx.parse_decl_spec(filtered_specifiers, *info.member_scope, is_typedef, base) || is_typedef) {
    return;
  }
  for(size_t j = 0; j < declarators->children.size(); ++j) {
    const CppAstNode & init_decl = declarators->children[j];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      continue;
    }

    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(&resolved_specifiers,
                                 init_decl.children[0],
                                 prepared_method);
    std::string friend_name;
    TypePtr friend_type;
    if(!ctx.parse_declarator(*info.member_scope,
                             prepared_method.parse_declarator_node(),
                             base, friend_name, friend_type, true) ||
       friend_name.empty()) {
      continue;
    }
    const QualifiedName * friend_name_syntax =
        first_identifier_name_syntax_in_subtree(prepared_method.parse_declarator_node());
    const TemplateIdSyntax * friend_name_template_id_syntax =
        first_identifier_template_id_syntax_in_subtree(
            prepared_method.parse_declarator_node());

    std::vector<std::pair<std::string, TypePtr> > params;
    std::vector<const CppAstNode *> default_args;
    const CppAstNode * parameter_clause =
        find_child(init_decl.children[0], CppAstKind::parameter_clause);
    if(parameter_clause &&
       !ctx.parse_parameter_clause(
           *info.member_scope, *parameter_clause, params, &default_args, true)) {
      continue;
    }

    if(!register_friend_function_template_binding(
           ctx,
           info,
           friend_name,
           friend_name_syntax,
           friend_name_template_id_syntax,
           friend_type)) {
      register_friend_function_binding(ctx,
                                       info,
                                       friend_name,
                                       friend_name_syntax,
                                       friend_type,
                                       params,
                                       default_args,
                                       nullptr,
                                       &node,
                                       prepared_method.syntax.function_qualifier,
                                       decl_spec_contains_token(*specifiers, KW_CONSTEXPR));
    }
  }
}

}  // namespace

void collect_class_simple_declaration(SemanticContext & ctx,
                                      ClassInfo & info,
                                      const CppAstNode & node,
                                      MemberAccess access)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child(node, CppAstKind::init_declarator_list);
  if(specifiers &&
     any_of(specifiers->children.begin(), specifiers->children.end(),
            [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); })) {
    if(!declarators || declarators->children.empty()) {
      collect_class_friend_declaration(ctx, info, node);
      return;
    }
    collect_class_friend_declaration(ctx, info, node);
    return;
  }
  if(!specifiers || !declarators || !class_member_specifiers_supported(*specifiers, true)) {
    throw std::logic_error("unsupported class member declaration" +
                           ctx.source_location_for_node(node));
  }

  PreparedClassMemberDeclarationContext prepared_decl;
  if(!prepare_class_member_declaration_context(ctx,
                                               *info.member_scope,
                                               *specifiers,
                                               declarators,
                                               true,
                                               true,
                                               true,
                                               prepared_decl)) {
    throw std::logic_error("unsupported class member embedded type-specifier" +
                           ctx.source_location_for_node(node));
  }

  const bool is_typedef = prepared_decl.declaration_is_typedef;
  const TypePtr base = prepared_decl.base;
  const bool has_mutable_specifier =
      decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_MUTABLE);
  ScopedTemplateUseLocation use_location_guard(
      template_public_use_location_or(ctx.source_location_for_node(node)));
  if(!prepared_decl.parsed_decl_spec) {
    std::ostringstream outmsg;
    outmsg << "unsupported class member decl-specifier-seq";
    if(!info.qualified_name.empty()) {
      outmsg << " [class " << info.qualified_name << "]";
    }
    outmsg << " [specifiers " << node_text(prepared_decl.resolved_specifiers) << "]";
    outmsg << " [bindings " << ctx.describe_scope_bindings_for_diagnostic(*info.member_scope) << "]";
    outmsg << " [member kind " << cppast_kind_text(node.kind) << "]";
    outmsg << " [member text " << node_text(node) << "]";
    outmsg << " [member ast={" << describe_cppast_translation_unit(node) << "}]";
    outmsg << ctx.source_location_for_node(node);
    throw std::logic_error(outmsg.str());
  }

  const bool is_static_member =
      decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_STATIC);
  const bool is_constexpr_member =
      decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_CONSTEXPR);
  const bool is_thread_local_member =
      decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_THREAD_LOCAL);
  const bool has_auto =
      decl_spec_contains_token(prepared_decl.resolved_specifiers, KW_AUTO);

  for(size_t j = 0; j < declarators->children.size(); ++j) {
    const CppAstNode & init_decl = declarators->children[j];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      throw std::logic_error("unsupported class member declarator" +
                             diagnostic_location_for_member(ctx, init_decl, &node));
    }

    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(&prepared_decl.resolved_specifiers,
                                 init_decl.children[0],
                                 prepared_method);
    validate_method_virtual_syntax(prepared_method.syntax);
    std::string member_name;
    TypePtr member_type;
    const CppAstNode * initializer = class_member_initializer(init_decl);
    if(!parse_class_member_declarator_type(ctx,
                                           *info.member_scope,
                                           prepared_decl.resolved_specifiers,
                                           prepared_method.parse_declarator_node(),
                                           initializer,
                                           base,
                                           has_auto,
                                           member_name,
                                           member_type,
                                           true) ||
       member_name.empty()) {
      throw std::logic_error("unsupported class member type" +
                             diagnostic_location_for_member(ctx, init_decl, &node));
    }
    member_type = callsemantic_internal::apply_initializer_array_bound(
        ctx,
        *info.member_scope,
        member_type,
        initializer);
    if(class_redeclares_template_parameter_name(info, member_name)) {
      throw std::logic_error("template parameter redeclared" +
                             diagnostic_location_for_member(ctx, init_decl, &node));
    }

    TypePtr stripped = strip_top_level_cv(member_type);
    if(is_typedef) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable class typedef");
      }
	      bind_member_named_type(*info.member_scope,
	                             member_name,
	                             canonicalize_member_typedef_type(
	                                 ctx, *info.member_scope, member_type, &info),
	                             access);
      continue;
    }

    if(stripped && stripped->kind == Type::TK_FUNCTION) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable member function");
      }
      if(init_decl.children.size() > 2) {
        throw std::logic_error("unsupported member function initializer" +
                               diagnostic_location_for_member(ctx, init_decl, &node));
      }
      if(init_decl.children.size() == 2) {
        const CppAstNode & initializer = init_decl.children[1];
        const CppAstNode * special =
            initializer.kind == CppAstKind::initializer &&
            initializer.children.size() == 1
                ? &initializer.children[0]
                : nullptr;
        if(!special ||
           special->kind != CppAstKind::special_initializer ||
           (special->value != "delete" && special->value != "default")) {
          if(!method_syntax_allows_pure_virtual_initializer(prepared_method.syntax) ||
             !is_pure_virtual_initializer(initializer)) {
            throw std::logic_error("unsupported member function initializer" +
                                   diagnostic_location_for_member(ctx, init_decl, &node));
          }
        }
      }
      std::vector<std::pair<std::string, TypePtr> > params;
      std::vector<const CppAstNode *> default_args;
      const CppAstNode * parameter_clause =
          find_child(init_decl.children[0], CppAstKind::parameter_clause);
      if(parameter_clause &&
         !ctx.parse_parameter_clause(
             *info.member_scope, *parameter_clause, params, &default_args, true)) {
        throw std::logic_error("unsupported member parameter-clause" +
                               diagnostic_location_for_member(ctx, init_decl, &node));
      }
      if(is_static_member) {
        FunctionRegistrationRequest request;
        request.owner_class = &info;
        request.name = member_name;
        request.declared_type = member_type;
        request.params = params;
        request.default_arguments = default_args;
        request.declaration_node = &init_decl;
        request.parameter_syntax_node = &init_decl.children[0];
        request.function_qualifier = prepared_method.syntax.function_qualifier;
        request.semantic_flags =
            class_function_options(access,
                                   &prepared_method.syntax,
                                   false,
                                   false,
                                   is_constexpr_member,
                                   false,
                                   decl_spec_contains_token(*specifiers, KW_INLINE));
        request.is_static_member = true;
        apply_member_declaration_exclusion(ctx.register_function_entity(request),
                                           node);
      } else {
        const bool is_defaulted =
            init_decl.children.size() == 2 &&
            init_decl.children[1].kind == CppAstKind::initializer &&
            init_decl.children[1].children.size() == 1 &&
            init_decl.children[1].children[0].kind == CppAstKind::special_initializer &&
            init_decl.children[1].children[0].value == "default";
        const bool is_deleted =
            init_decl.children.size() == 2 &&
            init_decl.children[1].kind == CppAstKind::initializer &&
            init_decl.children[1].children.size() == 1 &&
            init_decl.children[1].children[0].kind == CppAstKind::special_initializer &&
            init_decl.children[1].children[0].value == "delete";
        FunctionBinding * binding =
            register_class_function(ctx,
                                    info,
                                    member_name,
                                    member_type,
                                    params,
                                    default_args,
                                    nullptr,
                                    nullptr,
                                    class_function_options(access,
                                                           &prepared_method.syntax,
                                                           false,
                                                           false,
                                                           is_constexpr_member,
                                                           is_defaulted,
                                                           decl_spec_contains_token(*specifiers,
                                                                                    KW_INLINE)),
                                    &init_decl);
        if(binding) {
          binding->is_deleted = is_deleted;
          apply_member_declaration_exclusion(binding, node);
        }
      }
      continue;
    }

    if(is_static_member || is_constexpr_member) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable static/constexpr class member");
      }
      ValueBinding binding(ValueBinding::VK_VARIABLE, member_name, member_type);
      binding.access = access;
      binding.owner_class = &info;
      binding.is_thread_local = is_thread_local_member;
      binding.has_storage_definition = false;
      binding.declaration_node = &init_decl;
      binding.requires_constant_initializer = is_constexpr_member;
      if(initializer && initializer->children.size() == 1) {
        binding.constant_initializer = initializer;
        binding.constant_initializer_scope = info.member_scope.get();
        const bool defer_constant_evaluation =
            class_instantiation_is_dependent(ctx, info) ||
            ctx.type_depends_on_template_parameter(member_type);
        if(!defer_constant_evaluation) {
          constant_eval::ConstexprValue value;
          if(ctx.evaluate_initializer_constant_value(*info.member_scope,
                                                     *initializer,
                                                     member_type,
                                                     value)) {
            set_value_binding_constexpr_value(binding, value);
            long long integral = 0;
            if(constant_eval::constexpr_value_to_integral(value, integral)) {
              binding.has_constant_value = true;
              binding.constant_value = integral;
            }
          }
        }
      }
      info.member_scope->values[member_name] = binding;
      continue;
    }

    const CppAstNode * default_initializer =
        init_decl.children.size() > 1 ? &init_decl.children[1] : nullptr;
    maybe_complete_class_member_object_type(ctx, member_type);
    const bool unsupported_member_syntax =
        prepared_method.syntax.decl_virtual ||
        prepared_method.syntax.is_override ||
        prepared_method.syntax.is_final ||
        init_decl.children.size() > 2;
    const bool unsupported_member_type =
        !output_seed_class_member_object_type_supported(ctx, member_type);
    const bool defer_member_layout =
        !unsupported_member_syntax &&
        unsupported_member_type &&
        deferred_class_member_object_layout_supported(ctx, member_type);
    if((unsupported_member_syntax || unsupported_member_type) &&
       !defer_member_layout) {
      std::ostringstream message;
      message << "unsupported class member object";
      if(!member_name.empty()) {
        message << " name=" << member_name;
      }
      if(member_type) {
        message << " type=" << describe_type(member_type);
      }
      message << diagnostic_location_for_member(ctx, init_decl, &node);
      throw std::logic_error(message.str());
    }

    FieldInfo field;
    field.name = member_name;
    field.type = member_type;
    field.is_mutable = has_mutable_specifier;
    field.default_initializer = default_initializer;
    field.is_no_unique_address = init_decl.has_no_unique_address;
    field.access = access;
    info.fields.push_back(field);

    ValueBinding binding(ValueBinding::VK_FIELD, member_name, member_type);
    binding.owner_class = &info;
    binding.access = access;
    binding.is_mutable = has_mutable_specifier;
    binding.declaration_node = &init_decl;
    info.member_scope->values[member_name] = binding;
  }
}

void collect_class_bit_field_declaration(SemanticContext & ctx,
                                         ClassInfo & info,
                                         const CppAstNode & node,
                                         MemberAccess access)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(!specifiers || !class_member_specifiers_supported(*specifiers, true)) {
    throw std::logic_error("unsupported bit-field declaration");
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, nullptr, true,
                                             false, resolved_specifiers)) {
    throw std::logic_error("unsupported bit-field embedded type-specifier");
  }

  TypePtr base;
  const bool has_mutable_specifier =
      decl_spec_contains_token(resolved_specifiers, KW_MUTABLE);
  bool ignored_typedef = false;
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  ScopedTemplateUseLocation use_location_guard(
      template_public_use_location_or(ctx.source_location_for_node(node)));
  if(!ctx.parse_decl_spec(filtered_specifiers,
                          *info.member_scope,
                          ignored_typedef,
                          base,
                          true) ||
     ignored_typedef || !bit_field_type_supported(base)) {
    throw std::logic_error("unsupported bit-field base type");
  }

  if(decl_spec_contains_token(resolved_specifiers, KW_STATIC) ||
     decl_spec_contains_token(resolved_specifiers, KW_CONSTEXPR)) {
    throw std::logic_error("bit-fields cannot be static or constexpr");
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::bit_field_declarator) {
      continue;
    }

    const CppAstNode * declarator = find_child(child, CppAstKind::declarator);
    const CppAstNode * width =
        !child.children.empty() ? &child.children.back() : nullptr;
    if(!width || width->kind == CppAstKind::declarator) {
      throw std::logic_error("bit-field declarator missing width");
    }

    std::string member_name;
    TypePtr member_type = base;
    if(declarator &&
       (!ctx.parse_declarator(*info.member_scope,
                              *declarator,
                              base,
                              member_name,
                              member_type,
                              true) ||
        (!member_name.empty() && !bit_field_type_supported(member_type)))) {
      throw std::logic_error("unsupported bit-field declarator");
    }
    if(!declarator) {
      member_type = base;
    }

    FieldInfo field;
    field.name = member_name;
    field.type = member_type;
    field.is_mutable = has_mutable_specifier;
    field.bit_width_expression = width;
    field.is_bit_field = true;
    field.access = access;
    info.fields.push_back(field);

    if(!member_name.empty()) {
      ValueBinding binding(ValueBinding::VK_FIELD, member_name, member_type);
      binding.owner_class = &info;
      binding.access = access;
      binding.is_mutable = has_mutable_specifier;
      binding.is_bit_field = true;
      binding.declaration_node = &child;
      info.member_scope->values[member_name] = binding;
    }
  }
}

namespace {

void collect_class_reference_simple_declaration(SemanticContext & ctx,
                                                ClassInfo & info,
                                                const CppAstNode & node,
                                                MemberAccess access)
{
  const bool dependent_class = class_instantiation_is_dependent(ctx, info);
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child(node, CppAstKind::init_declarator_list);
  if(specifiers &&
     any_of(specifiers->children.begin(), specifiers->children.end(),
            [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); })) {
    collect_class_friend_declaration(ctx, info, node);
    return;
  }
  if(!specifiers || !declarators || !class_member_specifiers_supported(*specifiers, true)) {
    return;
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, declarators, true,
                                             true, resolved_specifiers)) {
    return;
  }

  const bool is_typedef_member =
      decl_spec_contains_token(resolved_specifiers, KW_TYPEDEF);
  bool is_typedef = false;
  TypePtr base;
  const bool has_mutable_specifier =
      decl_spec_contains_token(resolved_specifiers, KW_MUTABLE);
  const bool has_auto = decl_spec_contains_token(resolved_specifiers, KW_AUTO);
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  if(!has_auto &&
     !ctx.parse_decl_spec(filtered_specifiers,
                          *info.member_scope,
                          is_typedef,
                          base,
                          true)) {
    if(dependent_class && is_typedef_member) {
      const std::string type_id_text =
          dependent_typedef_type_text(filtered_specifiers);
      for(size_t j = 0; j < declarators->children.size(); ++j) {
        const CppAstNode & init_decl = declarators->children[j];
        if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
          continue;
        }
        std::string member_name;
        if(!first_identifier_text_in_subtree(init_decl.children[0], member_name) ||
           member_name.empty()) {
          continue;
        }
        TypePtr alias =
            make_dependent_class_alias_placeholder(info, member_name, type_id_text);
        trace_class_alias_store(
            ctx, info, "reference-typedef-fallback", member_name, type_id_text, alias);
        bind_member_named_type(*info.member_scope, member_name, alias, access);
      }
      return;
    }
    return;
  }

  const bool is_static_member =
      decl_spec_contains_token(resolved_specifiers, KW_STATIC);
  const bool is_constexpr_member =
      decl_spec_contains_token(resolved_specifiers, KW_CONSTEXPR);
  const bool is_thread_local_member =
      decl_spec_contains_token(resolved_specifiers, KW_THREAD_LOCAL);

  for(size_t j = 0; j < declarators->children.size(); ++j) {
    const CppAstNode & init_decl = declarators->children[j];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      continue;
    }

    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(&resolved_specifiers,
                                 init_decl.children[0],
                                 prepared_method);
    std::string member_name;
    TypePtr member_type;
    const CppAstNode * initializer = class_member_initializer(init_decl);
    if(!parse_class_member_declarator_type(ctx,
                                           *info.member_scope,
                                           resolved_specifiers,
                                           prepared_method.parse_declarator_node(),
                                           initializer,
                                           base,
                                           has_auto,
                                           member_name,
                                           member_type,
                                           true) ||
       member_name.empty()) {
      if(dependent_class && is_typedef) {
        const std::string type_id_text =
            dependent_typedef_type_text(filtered_specifiers);
        if(first_identifier_text_in_subtree(init_decl.children[0], member_name) &&
           !member_name.empty()) {
          TypePtr alias =
              make_dependent_class_alias_placeholder(info, member_name, type_id_text);
          trace_class_alias_store(
              ctx, info, "reference-typedef-declarator-fallback", member_name, type_id_text, alias);
          bind_member_named_type(*info.member_scope, member_name, alias, access);
        }
      }
      continue;
    }
    member_type = callsemantic_internal::apply_initializer_array_bound(
        ctx,
        *info.member_scope,
        member_type,
        initializer);

    if(is_typedef) {
      TypePtr alias = member_type;
      try {
	        alias = canonicalize_member_typedef_type(ctx, *info.member_scope, member_type, &info);
      } catch(const TemplateSubstitutionFailure &) {
        if(!dependent_class) {
          throw;
        }
        alias = make_dependent_class_alias_placeholder(
            info, member_name, dependent_typedef_type_text(filtered_specifiers));
      }
      TypePtr stored_alias = alias ? alias : member_type;
      stored_alias = refine_instantiated_class_alias(ctx, *info.member_scope, stored_alias);
      bind_member_named_type(*info.member_scope, member_name, stored_alias, access);
      continue;
    }

    TypePtr stripped = strip_top_level_cv(member_type);
    if(stripped && stripped->kind == Type::TK_FUNCTION) {
      if(has_mutable_specifier) {
        continue;
      }
      if(init_decl.children.size() > 2) {
        continue;
      }
      bool is_defaulted = false;
      bool is_deleted = false;
      if(init_decl.children.size() == 2) {
        const CppAstNode & initializer = init_decl.children[1];
        const CppAstNode * special =
            initializer.kind == CppAstKind::initializer &&
            initializer.children.size() == 1
                ? &initializer.children[0]
                : nullptr;
        if(!special ||
           special->kind != CppAstKind::special_initializer ||
           (special->value != "delete" && special->value != "default")) {
          if(!method_syntax_allows_pure_virtual_initializer(prepared_method.syntax) ||
             !is_pure_virtual_initializer(initializer)) {
            continue;
          }
        }
        is_defaulted = special &&
                       special->kind == CppAstKind::special_initializer &&
                       special->value == "default";
        is_deleted = special &&
                     special->kind == CppAstKind::special_initializer &&
                     special->value == "delete";
      }

      std::vector<std::pair<std::string, TypePtr> > params;
      std::vector<const CppAstNode *> default_args;
      const CppAstNode * parameter_clause =
          find_child(init_decl.children[0], CppAstKind::parameter_clause);
      if(parameter_clause &&
         !ctx.parse_parameter_clause(
             *info.member_scope, *parameter_clause, params, &default_args, true)) {
        continue;
      }

      if(is_static_member) {
        FunctionRegistrationRequest request;
        request.owner_class = &info;
        request.name = member_name;
        request.declared_type = member_type;
        request.params = params;
        request.default_arguments = default_args;
        request.declaration_node = &init_decl;
        request.parameter_syntax_node = &init_decl.children[0];
        request.function_qualifier = prepared_method.syntax.function_qualifier;
        request.semantic_flags =
            class_function_options(access,
                                   &prepared_method.syntax,
                                   false,
                                   false,
                                   is_constexpr_member,
                                   is_defaulted,
                                   decl_spec_contains_token(*specifiers, KW_INLINE));
        request.is_static_member = true;
        if(FunctionBinding * binding = ctx.register_function_entity(request)) {
          binding->is_deleted = is_deleted;
          apply_member_declaration_exclusion(binding, node);
        }
      } else {
        FunctionBinding * binding =
            register_class_function(ctx,
                                    info,
                                    member_name,
                                    member_type,
                                    params,
                                    default_args,
                                    nullptr,
                                    nullptr,
                                    class_function_options(access,
                                                           &prepared_method.syntax,
                                                           false,
                                                           false,
                                                           is_constexpr_member,
                                                           is_defaulted,
                                                           decl_spec_contains_token(*specifiers,
                                                                                    KW_INLINE)),
                                    &init_decl);
        if(binding) {
          binding->is_deleted = is_deleted;
          apply_member_declaration_exclusion(binding, node);
        }
      }
      continue;
    }

    if(is_static_member || is_constexpr_member) {
      if(has_mutable_specifier) {
        continue;
      }
      ValueBinding binding(ValueBinding::VK_VARIABLE, member_name, member_type);
      binding.access = access;
      binding.owner_class = &info;
      binding.is_thread_local = is_thread_local_member;
      binding.has_storage_definition = false;
      binding.declaration_node = &init_decl;
      binding.requires_constant_initializer = is_constexpr_member;
      if(initializer && initializer->children.size() == 1) {
        binding.constant_initializer = initializer;
        binding.constant_initializer_scope = info.member_scope.get();
        const bool defer_constant_evaluation =
            ctx.scope_has_template_placeholders(*info.member_scope) ||
            ctx.type_depends_on_template_parameter(member_type);
        if(!defer_constant_evaluation) {
          constant_eval::ConstexprValue value;
          if(ctx.evaluate_initializer_constant_value(*info.member_scope,
                                                     *initializer,
                                                     member_type,
                                                     value)) {
            set_value_binding_constexpr_value(binding, value);
            long long integral = 0;
            if(constant_eval::constexpr_value_to_integral(value, integral)) {
              binding.has_constant_value = true;
              binding.constant_value = integral;
            }
          }
        }
      }
      info.member_scope->values[member_name] = binding;
    }
  }
}

void collect_class_reference_bit_field_declaration(SemanticContext & ctx,
                                                   ClassInfo & info,
                                                   const CppAstNode & node,
                                                   MemberAccess access)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(!specifiers || !class_member_specifiers_supported(*specifiers, true)) {
    return;
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, nullptr, true,
                                             false, resolved_specifiers)) {
    return;
  }

  bool is_typedef = false;
  TypePtr base;
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  if(!ctx.parse_decl_spec(filtered_specifiers,
                          *info.member_scope,
                          is_typedef,
                          base,
                          true) ||
     is_typedef || !bit_field_type_supported(base)) {
    return;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::bit_field_declarator) {
      continue;
    }

    const CppAstNode * declarator = find_child(child, CppAstKind::declarator);
    const CppAstNode * width =
        !child.children.empty() ? &child.children.back() : nullptr;
    if(!width || width->kind == CppAstKind::declarator) {
      continue;
    }

    std::string member_name;
    TypePtr member_type = base;
    if(declarator &&
       (!ctx.parse_declarator(*info.member_scope,
                              *declarator,
                              base,
                              member_name,
                              member_type,
                              true) ||
        (!member_name.empty() && !bit_field_type_supported(member_type)))) {
      continue;
    }
    if(!declarator) {
      member_type = base;
    }

    FieldInfo field;
    field.name = member_name;
    field.type = member_type;
    field.bit_width_expression = width;
    field.is_bit_field = true;
    field.access = access;
    info.fields.push_back(field);

    if(!member_name.empty()) {
      ValueBinding binding(ValueBinding::VK_FIELD, member_name, member_type);
      binding.owner_class = &info;
      binding.access = access;
      binding.is_bit_field = true;
      info.member_scope->values[member_name] = binding;
    }
  }
}

const CppAstNode * innermost_template_declaration_payload(const CppAstNode & node)
{
  const CppAstNode * current = &node;
  while(current &&
        current->kind == CppAstKind::template_declaration &&
        !current->children.empty()) {
    current = &current->children.back();
  }
  return current;
}

bool node_has_friend_specifier(const CppAstNode & node)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(!specifiers) {
    specifiers = find_child(node, CppAstKind::member_specifiers);
  }
  return specifiers &&
         any_of(specifiers->children.begin(),
                specifiers->children.end(),
                [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); });
}

bool reference_collection_needs_template_declaration(const CppAstNode & node)
{
  const CppAstNode * payload = innermost_template_declaration_payload(node);
  if(!payload) {
    return false;
  }
  if(payload->kind == CppAstKind::class_specifier ||
     payload->kind == CppAstKind::class_forward_declaration ||
     payload->kind == CppAstKind::alias_declaration ||
     payload->kind == CppAstKind::deduction_guide_declaration ||
     payload->kind == CppAstKind::function_definition ||
     payload->kind == CppAstKind::special_member_declaration ||
     payload->kind == CppAstKind::simple_declaration) {
    return true;
  }
  return node_has_friend_specifier(*payload);
}

bool declarator_subtree_has_parameter_clause(const CppAstNode & node)
{
  if(node.kind == CppAstKind::parameter_clause) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(declarator_subtree_has_parameter_clause(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool template_declaration_payload_is_function_like(const CppAstNode & node)
{
  const CppAstNode * payload = innermost_template_declaration_payload(node);
  if(!payload) {
    return false;
  }
  if(payload->kind == CppAstKind::function_definition ||
     payload->kind == CppAstKind::special_member_definition ||
     payload->kind == CppAstKind::special_member_declaration) {
    return true;
  }
  if(payload->kind != CppAstKind::simple_declaration) {
    return false;
  }
  const CppAstNode * declarators =
      find_child(*payload, CppAstKind::init_declarator_list);
  if(!declarators) {
    return false;
  }
  for(size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    for(size_t j = 0; j < init_decl.children.size(); ++j) {
      if(init_decl.children[j].kind == CppAstKind::declarator &&
         declarator_subtree_has_parameter_clause(init_decl.children[j])) {
        return true;
      }
    }
  }
  return false;
}

bool reference_collection_can_defer_function_template_failure(
    const std::string & message)
{
  return message.find("unsupported function template ") != std::string::npos ||
         message.find("unsupported class member decl-specifier-seq") !=
             std::string::npos ||
         message.find("unsupported dependent class member decl-specifier-seq") !=
             std::string::npos ||
         message.find("failed non-type template argument evaluation") !=
             std::string::npos ||
         message.find("failed function template result type substitution") !=
             std::string::npos ||
         message.find("retained dependent result type") != std::string::npos;
}

bool reference_collection_can_defer_alias_failure(
    const std::string & message)
{
  return message.find("unsupported class member decl-specifier-seq") !=
             std::string::npos ||
         message.find("unsupported dependent class member decl-specifier-seq") !=
             std::string::npos ||
         message.find("failed non-type template argument evaluation") !=
             std::string::npos ||
         message.find("failed function template result type substitution") !=
             std::string::npos ||
         message.find("retained dependent result type") != std::string::npos;
}

void populate_class_reference_members(SemanticContext & ctx,
                                      ClassInfo & info,
                                      const CppAstNode & node)
{
  DIAG_CONTEXT("populate_class_reference_members [" + info.qualified_name + "]");
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << info.qualified_name
          << " complete=" << (info.complete ? "yes" : "no")
          << " ref_members=" << (info.reference_members_collected ? "yes" : "no")
          << " in_progress=" << (info.reference_member_collection_in_progress ? "yes" : "no");
    semantic_hotspot::note_semantic_query("populate_class_reference_members", query.str());
  }
  file_timing::ScopedTimer ref_timer("semantic.class-reference",
                                     ctx.source_location_for_node(node));
  if(!info.full_member_collection_in_progress) {
    parse_reference_base_clause(ctx, info, node);
  }
  const bool dependent_class = class_instantiation_is_dependent(ctx, info);
  MemberAccess current_access = info.default_access;
  std::size_t anonymous_union_counter = 0;
  std::function<void(const CppAstNode &)> collect_reference_named_class_declaration =
      [&](const CppAstNode & member)
  {
    if(member.value.empty()) {
      return;
    }
    const CppAstNode * class_key = find_child(member, CppAstKind::class_key);
    if(!class_key) {
      return;
    }
    ClassInfo * nested =
        ctx.create_class_info(*info.member_scope,
                              node_text(*class_key),
                              member.value,
                              &member);
    (void)dependent_class;
    (void)nested;
  };
  std::function<void(const CppAstNode &, MemberAccess)> collect_reference_template_declaration =
      [&](const CppAstNode & inner, MemberAccess access)
  {
    collect_class_friend_declaration(ctx, info, inner);
    if(inner.kind == CppAstKind::template_declaration) {
      if(!reference_collection_needs_template_declaration(inner)) {
        return;
      }
      const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
          class_source_capture_pause;
      try {
        ctx.collect_template_declaration(*info.member_scope, inner, access);
      } catch(const std::logic_error & e) {
        const std::string message = e.what();
        if(template_declaration_payload_is_function_like(inner) &&
           reference_collection_can_defer_function_template_failure(message)) {
          return;
        }
        throw;
      }
    }
  };

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    DIAG_CONTEXT(std::string("reference_class_member [") +
                 cppast_kind_text(child.kind) +
                 (child.value.empty() ? "" : " " + child.value) +
                 "] text=[" + node_text(child) + "]" +
                 (node_text(child).empty() ?
                      " ast={" + describe_cppast_translation_unit(child) + "}" :
                      ""));
    if(child.kind == CppAstKind::class_key || child.kind == CppAstKind::base_clause) {
      continue;
    }
    if(child.kind == CppAstKind::access_specifier) {
      current_access = access_from_node(child);
      continue;
    }
    if(child.kind == CppAstKind::simple_declaration) {
      collect_class_reference_simple_declaration(ctx, info, child, current_access);
      continue;
    }
    if(child.kind == CppAstKind::bit_field_declaration) {
      collect_class_reference_bit_field_declaration(ctx, info, child, current_access);
      continue;
    }
    if(child.kind == CppAstKind::static_assert_declaration) {
      if(witness::source_capture_enabled(ctx.template_witness_context())) {
        semantic_declaration::analyze_static_assert_declaration(ctx, *info.member_scope, child);
      }
      continue;
    }
    if(child.kind == CppAstKind::special_member_definition) {
      continue;
    }
    if(child.kind == CppAstKind::special_member_declaration) {
      collect_class_reference_special_member(ctx, info, child, current_access);
      continue;
    }
    if(child.kind == CppAstKind::function_definition) {
      const CppAstNode * function_specifiers =
          find_child(child, CppAstKind::decl_specifier_seq);
      if(function_specifiers &&
        any_of(function_specifiers->children.begin(),
               function_specifiers->children.end(),
               [](const CppAstNode & member) { return node_has_simple_type(member, KW_FRIEND); })) {
        collect_class_friend_function_definition(ctx, info, child, true);
      } else if(function_definition_is_static_constexpr_member(child)) {
        collect_class_method_definition(ctx, info, child, current_access);
      } else {
        collect_class_reference_method_definition(ctx, info, child, current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::template_declaration) {
      collect_class_friend_declaration(ctx, info, child);
      collect_reference_template_declaration(child, current_access);
      continue;
    }
    if(child.kind == CppAstKind::using_declaration) {
      if(!collect_inherited_constructors(ctx, info, child, current_access)) {
        semantic_declaration::collect_using_declaration(ctx,
                                                        *info.member_scope,
                                                        child,
                                                        current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration) {
      if(child.value.empty()) {
        if(node_is_union_class(child)) {
          collect_anonymous_union_storage(ctx,
                                          info,
                                          child,
                                          current_access,
                                          true,
                                          ++anonymous_union_counter);
          continue;
        }
        if(const CppAstNode * class_key = find_child(child, CppAstKind::class_key)) {
          record_anonymous_member_class(info, child, node_text(*class_key));
        }
        MemberAccess inner_access = current_access;
        for(size_t j = 0; j < child.children.size(); ++j) {
          const CppAstNode & member = child.children[j];
          DIAG_CONTEXT(std::string("reference_anonymous_member [") +
                       cppast_kind_text(member.kind) +
                       (member.value.empty() ? "" : " " + member.value) +
                       "] text=[" + node_text(member) + "]" +
                       (node_text(member).empty() ?
                            " ast={" + describe_cppast_translation_unit(member) + "}" :
                            ""));
          if(member.kind == CppAstKind::class_key || member.kind == CppAstKind::base_clause) {
            continue;
          }
          if(member.kind == CppAstKind::access_specifier) {
            inner_access = access_from_node(member);
            continue;
          }
          if(member.kind == CppAstKind::simple_declaration) {
            collect_class_reference_simple_declaration(ctx, info, member, inner_access);
            continue;
          }
          if(member.kind == CppAstKind::bit_field_declaration) {
            collect_class_reference_bit_field_declaration(ctx, info, member, inner_access);
            continue;
          }
          if(member.kind == CppAstKind::static_assert_declaration) {
            if(witness::source_capture_enabled(ctx.template_witness_context())) {
              semantic_declaration::analyze_static_assert_declaration(
                  ctx, *info.member_scope, member);
            }
            continue;
          }
          if(member.kind == CppAstKind::special_member_definition) {
            continue;
          }
          if(member.kind == CppAstKind::special_member_declaration) {
            collect_class_reference_special_member(ctx, info, member, inner_access);
            continue;
          }
          if(member.kind == CppAstKind::function_definition) {
            const CppAstNode * function_specifiers =
                find_child(member, CppAstKind::decl_specifier_seq);
            if(function_specifiers &&
               any_of(function_specifiers->children.begin(),
                      function_specifiers->children.end(),
                      [](const CppAstNode & spec)
                      { return node_has_simple_type(spec, KW_FRIEND); })) {
              collect_class_friend_function_definition(ctx, info, member, true);
            } else if(function_definition_is_static_constexpr_member(member)) {
              collect_class_method_definition(ctx, info, member, inner_access);
            } else {
              collect_class_reference_method_definition(ctx, info, member, inner_access);
            }
            continue;
          }
          if(member.kind == CppAstKind::template_declaration) {
            collect_reference_template_declaration(member, inner_access);
            continue;
          }
          if(member.kind == CppAstKind::using_declaration) {
            semantic_declaration::collect_using_declaration(ctx,
                                                            *info.member_scope,
                                                            member,
                                                            inner_access);
            continue;
          }
          if(member.kind == CppAstKind::class_specifier ||
             member.kind == CppAstKind::class_forward_declaration) {
            if(member.value.empty()) {
              continue;
            }
            collect_reference_named_class_declaration(member);
            continue;
          }
          if(member.kind == CppAstKind::enum_specifier) {
            ctx.collect_enum_declaration(*info.member_scope, member);
            continue;
          }
          if(member.kind == CppAstKind::alias_declaration) {
            const CppAstNode * type_id = find_child(member, CppAstKind::type_id);
            if(!type_id) {
              continue;
            }
            const std::string type_id_text = node_text(*type_id);
            TypePtr alias =
                parse_or_defer_reference_class_alias_type_id(ctx,
                                                            info,
                                                            member.value,
                                                            *type_id,
                                                            type_id_text,
                                                            dependent_class);
            if(alias) {
              alias = refine_instantiated_class_alias(ctx, *info.member_scope, alias);
              trace_class_alias_store(ctx,
                                      info,
                                      "reference-template-inner",
                                      member.value,
                                      type_id_text,
                                      alias);
              semantic_scope_mutation::bind_template_named_type_with_access(
                  *info.member_scope, member.value, alias, inner_access);
            }
          }
        }
        continue;
      }
      collect_reference_named_class_declaration(child);
      continue;
    }
    if(child.kind == CppAstKind::enum_specifier) {
      ctx.collect_enum_declaration(*info.member_scope, child);
      continue;
    }
    if(child.kind == CppAstKind::alias_declaration) {
      const CppAstNode * type_id = find_child(child, CppAstKind::type_id);
      if(type_id) {
        const std::string type_id_text = node_text(*type_id);
        TypePtr alias =
            parse_or_defer_reference_class_alias_type_id(ctx,
                                                        info,
                                                        child.value,
                                                        *type_id,
                                                        type_id_text,
                                                        dependent_class);
        if(alias) {
          alias = refine_instantiated_class_alias(ctx, *info.member_scope, alias);
          trace_class_alias_store(
              ctx, info, "reference-top", child.value, type_id_text, alias);
          semantic_scope_mutation::bind_template_named_type_with_access(
              *info.member_scope, child.value, alias, current_access);
        }
      }
    }
  }

  info.reference_members_collected = true;
}

}  // namespace

void ensure_class_reference_members(SemanticContext & ctx,
                                    ClassInfo & info)
{
  static thread_local int reference_member_collection_depth = 0;
  DIAG_CONTEXT("ensure_class_reference_members [" + info.qualified_name + "]");
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << info.qualified_name
          << " complete=" << (info.complete ? "yes" : "no")
          << " ref_members=" << (info.reference_members_collected ? "yes" : "no")
          << " in_progress=" << (info.reference_member_collection_in_progress ? "yes" : "no");
    semantic_hotspot::note_semantic_query("ensure_class_reference_members", query.str());
  }
  if(info.template_output_node &&
     info.template_output_node->kind == CppAstKind::class_forward_declaration) {
    template_api::refresh_forward_class_template_selection(ctx, info);
  }
  const CppAstNode * reference_node =
      info.template_output_node ? info.template_output_node : info.class_node;
  if(info.complete || info.reference_members_collected ||
     info.full_member_collection_in_progress ||
     info.reference_member_collection_in_progress || !reference_node) {
    return;
  }
  if(reference_member_collection_depth > kMaxReferenceMemberCollectionDepth) {
    return;
  }
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->reference_member_collections;
    ++counters->reference_member_collection_by_demand[
        static_cast<std::size_t>(semantic_metrics::current_class_demand())];
    counters->note_reference_member_collection(metrics_class_name(info),
                                               class_member_walk_units(*reference_node));
  }
  info.reference_member_collection_in_progress = true;
  ++reference_member_collection_depth;
  struct ReferenceCollectionGuard
  {
    ClassInfo & info;
    int & depth;
    ~ReferenceCollectionGuard()
    {
      info.reference_member_collection_in_progress = false;
      --depth;
    }
  } guard{info, reference_member_collection_depth};
  populate_class_reference_members(ctx, info, *reference_node);
}

bool resolve_deferred_class_alias(SemanticContext & ctx,
                                  ClassInfo & info,
                                  const std::string & alias_name,
                                  TypePtr & out)
{
  out.reset();
  std::map<std::string, ClassInfo::DeferredMemberAlias>::iterator found =
      info.deferred_member_aliases.find(alias_name);
  if(found == info.deferred_member_aliases.end() ||
     found->second.resolving ||
     !found->second.type_id ||
     !info.member_scope) {
    return false;
  }

  found->second.resolving = true;
  try {
    TypePtr alias =
        parse_or_defer_class_alias_type_id(ctx,
                                           info,
                                           alias_name,
                                           *found->second.type_id,
                                           found->second.type_id_text,
                                           found->second.dependent_class);
    if(alias) {
      alias = refine_instantiated_class_alias(ctx, *info.member_scope, alias);
      trace_class_alias_store(ctx,
                              info,
                              "resolve-deferred-reference",
                              alias_name,
                              found->second.type_id_text,
                              alias);
      std::map<std::string, MemberAccess>::const_iterator access =
          info.member_scope->named_type_access.find(alias_name);
      if(access != info.member_scope->named_type_access.end()) {
        semantic_scope_mutation::bind_template_named_type_with_access(
            *info.member_scope, alias_name, alias, access->second);
      } else {
        semantic_scope_mutation::bind_template_named_type(
            *info.member_scope, alias_name, alias);
      }
      out = alias;
    }
    info.deferred_member_aliases.erase(found);
    return out != nullptr;
  } catch(...) {
    found->second.resolving = false;
    throw;
  }
}

class ScopedTemplateWitnessLifecycleResume
{
public:
  ScopedTemplateWitnessLifecycleResume()
    : saved_depth_(
          template_api::template_witness_detail::
              current_lifecycle_pause_depth_storage())
  {
    template_api::template_witness_detail::
        current_lifecycle_pause_depth_storage() = 0;
  }

  ~ScopedTemplateWitnessLifecycleResume()
  {
    template_api::template_witness_detail::
        current_lifecycle_pause_depth_storage() = saved_depth_;
  }

private:
  int saved_depth_;
};

void finalize_class_constant_members(SemanticContext & ctx,
                                     ClassInfo & info)
{
  std::function<bool(const CppAstNode &)> initializer_has_nontrivial_type_probe;
  initializer_has_nontrivial_type_probe =
      [&](const CppAstNode & node) -> bool
      {
        if(node.kind == CppAstKind::sizeof_expression ||
           node.kind == CppAstKind::call_expression ||
           node.kind == CppAstKind::type_id) {
          return true;
        }
        for(std::size_t i = 0; i < node.children.size(); ++i) {
          if(initializer_has_nontrivial_type_probe(node.children[i])) {
            return true;
          }
        }
        return false;
      };
  const auto note_initializer_value_members =
      [&](ValueBinding & binding) -> bool
      {
        if(ctx.template_witness_context().session == nullptr ||
           !binding.constant_initializer ||
           !binding.constant_initializer_scope) {
          return false;
        }
        return semantic_template_class::note_constant_value_member_instantiations_in_expression(
            ctx,
            *binding.constant_initializer_scope,
            *binding.constant_initializer);
      };
  const auto try_evaluate_noexcept_initializer =
      [&](ValueBinding & binding, constant_eval::ConstexprValue & out) -> bool
      {
        if(!binding.constant_initializer || !binding.constant_initializer_scope) {
          return false;
        }
        const CppAstNode * payload = binding.constant_initializer;
        if(payload->kind == CppAstKind::initializer &&
           payload->children.size() == 1) {
          payload = &payload->children[0];
        }
        if(!payload ||
           !node_has_simple_type(*payload, KW_NOEXCEPT) ||
           payload->children.size() != 1) {
          return false;
        }

        bool is_nothrow = false;
        try {
          if(ctx.expression_is_nothrow(*binding.constant_initializer_scope,
                                       payload->children[0],
                                       is_nothrow)) {
            out = constant_eval::make_integral_value(is_nothrow ? 1 : 0,
                                                     binding.type);
            return true;
          }
        } catch(const std::logic_error &) {
        }

        if(!class_constant_noexcept_operand_can_use_template_fallback(
               payload->children[0]) ||
           !class_constant_noexcept_operand_is_well_formed(
               ctx,
               *binding.constant_initializer_scope,
               payload->children[0])) {
          return false;
        }
        out = constant_eval::make_integral_value(0, binding.type);
        return true;
      };
  const auto note_value_member_with_nested_member_type_dependency =
      [&](ValueBinding & binding, bool has_nested_member_type_dependency) -> void
      {
        if(parser_trace::enabled("template.resolve") && binding.name == "value") {
          std::ostringstream trace;
          trace << "value-member-nested-dependency class=" << info.qualified_name
                << " nested=" << (has_nested_member_type_dependency ? "yes" : "no")
                << " probe="
                << (binding.constant_initializer &&
                    initializer_has_nontrivial_type_probe(*binding.constant_initializer) ?
                        "yes" :
                        "no")
                << " explicit-info="
                << (info.is_explicit_specialization ? "yes" : "no")
                << " explicit-binding="
                << (binding.is_explicit_specialization ? "yes" : "no")
                << " session="
                << (ctx.template_witness_context().session ? "yes" : "no");
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        if(ctx.template_witness_context().session == nullptr ||
           binding.name != "value" ||
           info.dependent_instantiation ||
           (!has_nested_member_type_dependency &&
            (!binding.constant_initializer ||
             !initializer_has_nontrivial_type_probe(*binding.constant_initializer)))) {
          return;
        }
        if(info.is_explicit_specialization ||
           binding.is_explicit_specialization) {
          return;
        }
        std::string decl_location =
            semantic_model::source_decl_anchor_location(
                semantic_trace::value_decl_anchor(ctx, &binding));
        if(decl_location.compare(0, 4, " at ") == 0) {
          decl_location = decl_location.substr(4);
        }
        const std::string owner_name =
            semantic_model::class_output_qualified_name(info);
        if(owner_name.empty() || decl_location.empty()) {
          return;
        }
        const std::string entity = owner_name + "::" + binding.name;
        const bool has_template_identity =
            template_api::value_or_owner_has_template_identity(&binding) ||
            template_api::class_has_template_identity(&info);
        binding.witness_member_value_instantiation_noted = true;
        const ScopedTemplateWitnessLifecycleResume lifecycle_resume;
        const template_api::ScopedTemplateWitnessEntryContext entry_context(
            template_api::make_template_closure_entry_context(
                template_api::TemplateClosureReason::TrackInstantiation,
                entity,
                decl_location,
                has_template_identity));
        template_api::note_template_witness_log_event(
            template_api::TemplateWitnessLogEventKind::VariableInstantiation,
            decl_location,
            entity,
            decl_location,
            std::string(),
            template_api::TemplateLifecycleCause::TrackInstantiation,
            has_template_identity);
      };
  const auto find_current_binding =
      [&](const std::string & name) -> ValueBinding *
      {
        if(!info.member_scope) {
          return nullptr;
        }
        std::map<std::string, ValueBinding>::iterator found =
            info.member_scope->values.find(name);
        return found == info.member_scope->values.end() ? nullptr : &found->second;
      };
  std::vector<std::string> member_names;
  for(std::map<std::string, ValueBinding>::const_iterator it =
          info.member_scope->values.begin();
      it != info.member_scope->values.end();
      ++it) {
    member_names.push_back(it->first);
  }
  for(std::size_t member_index = 0;
      member_index < member_names.size();
      ++member_index) {
    ValueBinding * binding = find_current_binding(member_names[member_index]);
    if(!binding) {
      continue;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "finalize-class-constant class=" << info.qualified_name
            << " member=" << binding->name
            << " has-initializer=" << (binding->constant_initializer ? "yes" : "no")
            << " has-constant=" << (binding->has_constant_value ? "yes" : "no")
            << " has-constexpr=" << (binding->has_constexpr_value ? "yes" : "no")
            << " dependent=" << (binding->dependent_template_value ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!binding->constant_initializer || !binding->constant_initializer_scope) {
      continue;
    }
    if(class_instantiation_is_dependent(ctx, info)) {
      binding->dependent_template_value = true;
      binding->has_constant_value = false;
      clear_value_binding_constexpr_value(*binding);
      continue;
    }
    if(binding->has_constant_value || binding->has_constexpr_value) {
      const bool has_nested_member_type_dependency =
          note_initializer_value_members(*binding);
      binding = find_current_binding(member_names[member_index]);
      if(!binding) {
        continue;
      }
      note_value_member_with_nested_member_type_dependency(
          *binding,
          has_nested_member_type_dependency);
      binding = find_current_binding(member_names[member_index]);
      if(binding) {
        binding->dependent_template_value = false;
      }
      continue;
    }
    if(ctx.scope_has_template_placeholders(*binding->constant_initializer_scope) ||
       ctx.type_depends_on_template_parameter(binding->type)) {
      binding->dependent_template_value = true;
      continue;
    }

    const CppAstNode * initializer = binding->constant_initializer;
    Scope * initializer_scope = binding->constant_initializer_scope;
    const TypePtr initializer_type = binding->type;
    constant_eval::ConstexprValue value;
    binding->constant_value_in_progress = true;
    bool evaluated = false;
    try {
      const template_api::ScopedTemplateWitnessLifecyclePause lifecycle_pause(true);
      evaluated = ctx.evaluate_initializer_constant_value(*initializer_scope,
                                                          *initializer,
                                                          initializer_type,
                                                          value);
      if(!evaluated) {
        binding = find_current_binding(member_names[member_index]);
        if(binding &&
           !binding->has_constant_value &&
           !binding->has_constexpr_value) {
          binding->constant_value_in_progress = true;
          evaluated = try_evaluate_noexcept_initializer(*binding, value);
        }
      }
    } catch(...) {
      binding = find_current_binding(member_names[member_index]);
      if(binding) {
        binding->constant_value_in_progress = false;
      }
      throw;
    }
    binding = find_current_binding(member_names[member_index]);
    if(!binding) {
      continue;
    }
    binding->constant_value_in_progress = false;
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "finalize-class-constant-eval class=" << info.qualified_name
            << " member=" << binding->name
            << " evaluated=" << (evaluated ? "yes" : "no");
      if(binding->constant_initializer) {
        trace << " initializer=" << node_text(*binding->constant_initializer);
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(binding->has_constant_value || binding->has_constexpr_value) {
      binding->dependent_template_value = false;
      continue;
    }
    if(evaluated) {
      const bool has_nested_member_type_dependency =
          note_initializer_value_members(*binding);
      binding = find_current_binding(member_names[member_index]);
      if(!binding) {
        continue;
      }
      note_value_member_with_nested_member_type_dependency(
          *binding,
          has_nested_member_type_dependency);
      binding = find_current_binding(member_names[member_index]);
      if(!binding) {
        continue;
      }
      binding->dependent_template_value = false;
      set_value_binding_constexpr_value(*binding, value);
      long long integral = 0;
      if(constant_eval::constexpr_value_to_integral(value, integral)) {
        binding->has_constant_value = true;
        binding->constant_value = integral;
      }
      continue;
    }

    if(binding->requires_constant_initializer) {
      std::ostringstream message;
      message << "unsupported constexpr class member initializer";
      if(!info.name.empty()) {
        message << " [class " << info.name << "]";
      }
      if(!binding->name.empty()) {
        message << " [member " << binding->name << "]";
      }
      if(binding->type) {
        message << " [type " << describe_type(binding->type) << "]";
      }
      throw std::logic_error(message.str());
    }
  }
}

void collect_dependent_class_simple_declaration(SemanticContext & ctx,
                                                ClassInfo & info,
                                                const CppAstNode & node,
                                                MemberAccess access)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child(node, CppAstKind::init_declarator_list);
  if(specifiers &&
     any_of(specifiers->children.begin(), specifiers->children.end(),
            [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); })) {
    collect_class_friend_declaration(ctx, info, node);
    return;
  }
  if(!specifiers || !declarators || !class_member_specifiers_supported(*specifiers, true)) {
    throw std::logic_error("unsupported dependent class member declaration");
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, declarators, true,
                                             false, resolved_specifiers)) {
    throw std::logic_error("unsupported dependent class member embedded type-specifier");
  }

  bool is_typedef = false;
  TypePtr base;
  const bool has_mutable_specifier =
      decl_spec_contains_token(resolved_specifiers, KW_MUTABLE);
  const bool has_auto = decl_spec_contains_token(resolved_specifiers, KW_AUTO);
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  if(!has_auto &&
     !ctx.parse_decl_spec(filtered_specifiers,
                          *info.member_scope,
                          is_typedef,
                          base,
                          true)) {
    if(class_instantiation_is_dependent(ctx, info) &&
       decl_spec_contains_token(resolved_specifiers, KW_TYPEDEF)) {
      const std::string type_id_text =
          dependent_typedef_type_text(filtered_specifiers);
      for(size_t j = 0; j < declarators->children.size(); ++j) {
        const CppAstNode & init_decl = declarators->children[j];
        if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
          throw std::logic_error("unsupported dependent class member declarator");
        }
        std::string member_name;
        if(!first_identifier_text_in_subtree(init_decl.children[0], member_name) ||
           member_name.empty()) {
          throw std::logic_error("unsupported dependent class member typedef name");
        }
        TypePtr alias =
            make_dependent_class_alias_placeholder(info, member_name, type_id_text);
        trace_class_alias_store(
            ctx, info, "dependent-typedef-fallback", member_name, type_id_text, alias);
        bind_member_named_type(*info.member_scope, member_name, alias, access);
      }
      return;
    }
    if(class_instantiation_is_dependent(ctx, info)) {
      return;
    }
    if(info.source_template &&
       info.member_scope &&
       ctx.scope_has_template_placeholders(*info.member_scope)) {
      return;
    }
    std::ostringstream outmsg;
    outmsg << "unsupported dependent class member decl-specifier-seq";
    if(!info.qualified_name.empty()) {
      outmsg << " [class " << info.qualified_name << "]";
    }
    outmsg << " [specifiers " << node_text(resolved_specifiers) << "]";
    outmsg << " [bindings " << ctx.describe_scope_bindings_for_diagnostic(*info.member_scope) << "]";
    outmsg << " [member kind " << cppast_kind_text(node.kind) << "]";
    outmsg << " [member text " << node_text(node) << "]";
    outmsg << " [member ast={" << describe_cppast_translation_unit(node) << "}]";
    throw std::logic_error(outmsg.str());
  }

  const bool is_static_member =
      decl_spec_contains_token(resolved_specifiers, KW_STATIC);
  const bool is_constexpr_member =
      decl_spec_contains_token(resolved_specifiers, KW_CONSTEXPR);
  const bool is_thread_local_member =
      decl_spec_contains_token(resolved_specifiers, KW_THREAD_LOCAL);

  for(size_t j = 0; j < declarators->children.size(); ++j) {
    const CppAstNode & init_decl = declarators->children[j];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      throw std::logic_error("unsupported dependent class member declarator");
    }

    PreparedMethodParseContext prepared_method;
    prepare_method_parse_context(&resolved_specifiers,
                                 init_decl.children[0],
                                 prepared_method);
    std::string member_name;
    TypePtr member_type;
    const CppAstNode * initializer = class_member_initializer(init_decl);
    if(!parse_class_member_declarator_type(ctx,
                                           *info.member_scope,
                                           resolved_specifiers,
                                           prepared_method.parse_declarator_node(),
                                           initializer,
                                           base,
                                           has_auto,
                                           member_name,
                                           member_type,
                                           false) ||
       member_name.empty()) {
      if(is_typedef) {
        const std::string type_id_text =
            dependent_typedef_type_text(filtered_specifiers);
        if(first_identifier_text_in_subtree(init_decl.children[0], member_name) &&
           !member_name.empty()) {
          TypePtr alias =
              make_dependent_class_alias_placeholder(info, member_name, type_id_text);
          trace_class_alias_store(
              ctx, info, "dependent-typedef-declarator-fallback", member_name, type_id_text, alias);
          bind_member_named_type(*info.member_scope, member_name, alias, access);
          continue;
        }
      }
      if(info.dependent_instantiation ||
         (info.source_template &&
          info.member_scope &&
          ctx.scope_has_template_placeholders(*info.member_scope))) {
        continue;
      }
      throw std::logic_error("unsupported dependent class member type");
    }
    member_type = callsemantic_internal::apply_initializer_array_bound(
        ctx,
        *info.member_scope,
        member_type,
        initializer);

    TypePtr stripped = strip_top_level_cv(member_type);
    if(is_typedef) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable dependent class typedef");
      }
      TypePtr alias = member_type;
      try {
	        alias = canonicalize_member_typedef_type(ctx, *info.member_scope, member_type, &info);
      } catch(const TemplateSubstitutionFailure &) {
        alias = make_dependent_class_alias_placeholder(
            info, member_name, dependent_typedef_type_text(filtered_specifiers));
      }
      bind_member_named_type(*info.member_scope,
                             member_name,
                             alias ? alias : member_type,
                             access);
      continue;
    }

    if(stripped && stripped->kind == Type::TK_FUNCTION) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable dependent member function");
      }
      if(init_decl.children.size() > 2) {
        throw std::logic_error("unsupported dependent member function initializer");
      }
      if(init_decl.children.size() == 2) {
        const CppAstNode & initializer = init_decl.children[1];
        const CppAstNode * special =
            initializer.kind == CppAstKind::initializer &&
            initializer.children.size() == 1
                ? &initializer.children[0]
                : nullptr;
        if(!special ||
           special->kind != CppAstKind::special_initializer ||
           (special->value != "delete" && special->value != "default")) {
          if(!method_syntax_allows_pure_virtual_initializer(prepared_method.syntax) ||
             !is_pure_virtual_initializer(initializer)) {
            throw std::logic_error("unsupported dependent member function initializer");
          }
        }
      }
      std::vector<std::pair<std::string, TypePtr> > params;
      std::vector<const CppAstNode *> default_args;
      const CppAstNode * parameter_clause =
          find_child(init_decl.children[0], CppAstKind::parameter_clause);
      if(parameter_clause &&
         !ctx.parse_parameter_clause(
             *info.member_scope, *parameter_clause, params, &default_args, true)) {
        throw std::logic_error("unsupported dependent member parameter-clause");
      }
      if(is_static_member) {
        FunctionRegistrationRequest request;
        request.owner_class = &info;
        request.name = member_name;
        request.declared_type = member_type;
        request.params = params;
        request.default_arguments = default_args;
        request.declaration_node = &init_decl;
        request.parameter_syntax_node = &init_decl.children[0];
        request.function_qualifier = prepared_method.syntax.function_qualifier;
        request.semantic_flags =
            class_function_options(access,
                                   &prepared_method.syntax,
                                   false,
                                   false,
                                   is_constexpr_member,
                                   false,
                                   decl_spec_contains_token(*specifiers, KW_INLINE));
        request.is_static_member = true;
        apply_member_declaration_exclusion(ctx.register_function_entity(request),
                                           node);
      } else {
        const bool is_defaulted =
            init_decl.children.size() == 2 &&
            init_decl.children[1].kind == CppAstKind::initializer &&
            init_decl.children[1].children.size() == 1 &&
            init_decl.children[1].children[0].kind == CppAstKind::special_initializer &&
            init_decl.children[1].children[0].value == "default";
        const bool is_deleted =
            init_decl.children.size() == 2 &&
            init_decl.children[1].kind == CppAstKind::initializer &&
            init_decl.children[1].children.size() == 1 &&
            init_decl.children[1].children[0].kind == CppAstKind::special_initializer &&
            init_decl.children[1].children[0].value == "delete";
        FunctionBinding * binding =
            register_class_function(ctx,
                                    info,
                                    member_name,
                                    member_type,
                                    params,
                                    default_args,
                                    nullptr,
                                    nullptr,
                                    class_function_options(access,
                                                           &prepared_method.syntax,
                                                           false,
                                                           false,
                                                           is_constexpr_member,
                                                           is_defaulted,
                                                           decl_spec_contains_token(*specifiers,
                                                                                    KW_INLINE)),
                                    &init_decl);
        if(binding) {
          binding->is_deleted = is_deleted;
          apply_member_declaration_exclusion(binding, node);
        }
      }
      continue;
    }

    if(is_static_member || is_constexpr_member) {
      if(has_mutable_specifier) {
        throw std::logic_error("unsupported mutable dependent static/constexpr class member");
      }
      ValueBinding binding(ValueBinding::VK_VARIABLE, member_name, member_type);
      binding.access = access;
      binding.owner_class = &info;
      binding.is_thread_local = is_thread_local_member;
      binding.declaration_node = &init_decl;
      if(init_decl.children.size() > 1 &&
         init_decl.children[1].kind == CppAstKind::initializer &&
         init_decl.children[1].children.size() == 1) {
        long long value = 0;
        if(ctx.evaluate_constant_expression(*info.member_scope,
                                            init_decl.children[1].children[0],
                                            value)) {
          binding.has_constant_value = true;
          binding.constant_value = value;
        }
      }
      info.member_scope->values[member_name] = binding;
      continue;
    }

    const CppAstNode * default_initializer =
        init_decl.children.size() > 1 ? &init_decl.children[1] : nullptr;
    maybe_complete_class_member_object_type(ctx, member_type);
    if(prepared_method.syntax.decl_virtual ||
       prepared_method.syntax.is_override ||
       prepared_method.syntax.is_final ||
       init_decl.children.size() > 2 ||
       !output_seed_dependent_class_member_object_type_supported(ctx, member_type)) {
      continue;
    }

    FieldInfo field;
    field.name = member_name;
    field.type = member_type;
    field.is_mutable = has_mutable_specifier;
    field.default_initializer = default_initializer;
    field.is_no_unique_address = init_decl.has_no_unique_address;
    field.access = access;
    info.fields.push_back(field);

    ValueBinding binding(ValueBinding::VK_FIELD, member_name, member_type);
    binding.owner_class = &info;
    binding.access = access;
    binding.is_mutable = has_mutable_specifier;
    info.member_scope->values[member_name] = binding;
  }
}

void collect_dependent_class_bit_field_declaration(SemanticContext & ctx,
                                                   ClassInfo & info,
                                                   const CppAstNode & node,
                                                   MemberAccess access)
{
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(!specifiers || !class_member_specifiers_supported(*specifiers, true)) {
    throw std::logic_error("unsupported dependent bit-field declaration");
  }

  CppAstNode resolved_specifiers;
  if(!ctx.prepare_namespace_scope_specifiers(*info.member_scope, *specifiers, nullptr, true,
                                             false, resolved_specifiers)) {
    throw std::logic_error("unsupported dependent bit-field embedded type-specifier");
  }

  bool is_typedef = false;
  TypePtr base;
  const bool has_mutable_specifier =
      decl_spec_contains_token(resolved_specifiers, KW_MUTABLE);
  const CppAstNode filtered_specifiers =
      filtered_class_member_decl_specifiers(resolved_specifiers);
  if(!ctx.parse_decl_spec(filtered_specifiers, *info.member_scope, is_typedef, base) ||
     is_typedef || !dependent_bit_field_type_supported(ctx, base)) {
    throw std::logic_error("unsupported dependent bit-field base type");
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::bit_field_declarator) {
      continue;
    }

    const CppAstNode * declarator = find_child(child, CppAstKind::declarator);
    const CppAstNode * width =
        !child.children.empty() ? &child.children.back() : nullptr;
    if(!width || width->kind == CppAstKind::declarator) {
      throw std::logic_error("dependent bit-field missing width");
    }

    std::string member_name;
    TypePtr member_type = base;
    if(declarator &&
       (!ctx.parse_declarator(*info.member_scope,
                              *declarator,
                              base,
                              member_name,
                              member_type) ||
        (!member_name.empty() && !dependent_bit_field_type_supported(ctx, member_type)))) {
      throw std::logic_error("unsupported dependent bit-field declarator");
    }
    if(!declarator) {
      member_type = base;
    }

    FieldInfo field;
    field.name = member_name;
    field.type = member_type;
    field.is_mutable = has_mutable_specifier;
    field.bit_width_expression = width;
    field.is_bit_field = true;
    field.access = access;
    info.fields.push_back(field);

    if(!member_name.empty()) {
      ValueBinding binding(ValueBinding::VK_FIELD, member_name, member_type);
      binding.owner_class = &info;
      binding.access = access;
      binding.is_mutable = has_mutable_specifier;
      binding.is_bit_field = true;
      info.member_scope->values[member_name] = binding;
    }
  }
}

void collect_class_method_definition(SemanticContext & ctx,
                                     ClassInfo & info,
                                     const CppAstNode & node,
                                     MemberAccess access)
{
  trace_class_collection_event(ctx, "method-definition-enter", info, node);
  const CppAstNode * specifiers = find_child(node, CppAstKind::decl_specifier_seq);
  if(specifiers &&
     any_of(specifiers->children.begin(), specifiers->children.end(),
            [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); })) {
    collect_class_friend_function_definition(ctx, info, node, true);
    return;
  }
  PreparedClassMemberFunctionDefinition prepared;
  if(!prepare_class_member_function_definition(ctx, info, node, true, prepared)) {
    throw std::logic_error(std::string("unsupported member function definition ") +
                           node_text(node));
  }
  trace_class_collection_event(ctx,
                               "method-definition-parsed",
                               info,
                               node,
                               std::string("member=") + prepared.name);

  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const CppAstNode * parameter_clause =
      find_child(*prepared.declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(
         *info.member_scope, *parameter_clause, params, &default_args, true)) {
    throw std::logic_error("unsupported member parameter-clause");
  }

  validate_method_virtual_syntax(prepared.method.syntax);
  const ClassFunctionOptions method_flags =
      class_function_options(access,
                             &prepared.method.syntax,
                             false,
                             false,
                             prepared.is_constexpr_member,
                             false,
                             prepared.is_inline_member);

  if(prepared.is_static_member) {
    FunctionRegistrationRequest request;
    request.owner_class = &info;
    request.name = prepared.name;
    request.declared_type = prepared.declared_type;
    request.params = params;
    request.default_arguments = default_args;
    request.body = prepared.body;
    request.declaration_node = &node;
    request.function_qualifier = prepared.method.syntax.function_qualifier;
    request.semantic_flags.access = access;
    request.semantic_flags.is_constexpr = prepared.is_constexpr_member;
    request.semantic_flags.is_inline = prepared.is_inline_member;
    request.is_static_member = true;
    ctx.register_function_entity(request);
  } else {
    register_class_function(ctx,
                            info,
                            prepared.name,
                            prepared.declared_type,
                            params,
                            default_args,
                            prepared.body,
                            nullptr,
                            method_flags,
                            &node);
  }
  trace_class_collection_event(ctx,
                               "method-definition-done",
                               info,
                               node,
                               std::string("member=") + prepared.name);
}

namespace {

void collect_class_reference_method_definition(SemanticContext & ctx,
                                               ClassInfo & info,
                                               const CppAstNode & node,
                                               MemberAccess access)
{
  PreparedClassMemberFunctionDefinition prepared;
  if(!prepare_class_member_function_definition(ctx, info, node, true, prepared)) {
    return;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const CppAstNode * parameter_clause =
      find_child(*prepared.declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(
         *info.member_scope, *parameter_clause, params, &default_args, true)) {
    return;
  }

  validate_method_virtual_syntax(prepared.method.syntax);
  const ClassFunctionOptions method_flags =
      class_function_options(access,
                             &prepared.method.syntax,
                             false,
                             false,
                             prepared.is_constexpr_member,
                             false,
                             prepared.is_inline_member);

  if(prepared.is_static_member) {
    FunctionRegistrationRequest request;
    request.owner_class = &info;
    request.name = prepared.name;
    request.declared_type = prepared.declared_type;
    request.params = params;
    request.default_arguments = default_args;
    request.declaration_node = &node;
    request.function_qualifier = prepared.method.syntax.function_qualifier;
    request.semantic_flags.access = access;
    request.semantic_flags.is_constexpr = prepared.is_constexpr_member;
    request.semantic_flags.is_inline = prepared.is_inline_member;
    request.is_static_member = true;
    ctx.register_function_entity(request);
  } else {
    register_class_function(ctx,
                            info,
                            prepared.name,
                            prepared.declared_type,
                            params,
                            default_args,
                            nullptr,
                            nullptr,
                            method_flags,
                            &node);
  }
}

}  // namespace

void collect_special_member(SemanticContext & ctx,
                            ClassInfo & info,
                            const CppAstNode & node,
                            MemberAccess access)
{
  trace_class_collection_event(ctx, "special-member-enter", info, node);
  const CppAstNode * member_specifiers = find_child(node, CppAstKind::member_specifiers);
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  if(!declarator) {
    throw std::logic_error("invalid special member");
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const CppAstNode * parameter_clause = find_child(*declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(
         *info.member_scope, *parameter_clause, params, &default_args, true)) {
    if(info.dependent_instantiation ||
       (info.source_template &&
        info.member_scope &&
        ctx.scope_has_template_placeholders(*info.member_scope))) {
      return;
    }
    throw std::logic_error("unsupported special-member parameter-clause");
  }

  const bool is_constructor =
      special_member_matches_class_name(ctx, node.value, info, false);
  const bool is_destructor =
      special_member_matches_class_name(ctx, node.value, info, true);
  if(!is_constructor && !is_destructor) {
    throw std::logic_error("unsupported special member in PA14");
  }

  PreparedMethodParseContext prepared_method;
  prepare_method_parse_context(member_specifiers, *declarator, prepared_method);
  validate_method_virtual_syntax(prepared_method.syntax);
  const bool is_defaulted = special_member_is_defaulted(node);
  const bool is_deleted = special_member_is_deleted(node);
  const CppAstNode * ctor_initializer =
      find_child(node, CppAstKind::ctor_initializer);
  const ClassFunctionOptions special_member_flags =
      class_function_options(access,
                             &prepared_method.syntax,
                             is_constructor,
                             is_destructor,
                             member_specifiers &&
                                 decl_spec_contains_token(*member_specifiers, KW_CONSTEXPR),
                             is_defaulted,
                             member_specifiers &&
                                 decl_spec_contains_token(*member_specifiers, KW_INLINE));

  FunctionBinding * binding =
      register_class_function(ctx,
                              info,
                              node.value,
                              TypePtr(),
                              params,
                              default_args,
                              find_class_function_body_node(node),
                              ctor_initializer,
                              special_member_flags,
                              &node);
  if(binding) {
    binding->is_deleted = is_deleted;
    binding->exclude_from_explicit_instantiation =
        binding->exclude_from_explicit_instantiation ||
        special_member_excluded_from_explicit_instantiation(node);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "collect-special-member-binding class=" << info.qualified_name
            << " member=" << node.value
            << " exclude="
            << (binding->exclude_from_explicit_instantiation ? "yes" : "no")
            << " decl-node="
            << (binding->declaration_node ? cppast_kind_text(binding->declaration_node->kind)
                                          : std::string("<none>"));
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
  }
  if(is_constructor && ctor_initializer && info.member_scope) {
    for(std::size_t i = 0; i < ctor_initializer->children.size(); ++i) {
      const CppAstNode & initializer = ctor_initializer->children[i];
      ctx.emit_nested_class_use_source_events_from_ast_node(
          *info.member_scope,
          initializer,
          witness::SourceUseOwnership::SourceOwned);
      const CppAstNode * initializer_id =
          find_child(initializer, CppAstKind::mem_initializer_id);
      const TemplateIdSyntax * syntax =
          initializer_id ? cppast_template_id_syntax(*initializer_id) : nullptr;
      if(!initializer_id ||
         !syntax ||
         syntax->name.name.empty()) {
        continue;
      }
      std::string use_location =
          template_api::normalize_template_witness_source_location(
              ctx.source_location_for_name_in_node(*initializer_id,
                                                   syntax->name.name));
      if(!semantic_trace::source_location_points_at_identifier(
             use_location,
             syntax->name.name)) {
        use_location.clear();
      }
      if(use_location.empty()) {
        use_location =
            template_api::normalize_template_witness_source_location(
                ctx.source_location_for_node(*initializer_id));
      }
      if(!semantic_trace::source_location_points_at_identifier(
             use_location,
             syntax->name.name)) {
        continue;
      }
      const std::vector<std::string> argument_locations =
          template_argument_source_locations_for_node(ctx,
                                                      *initializer_id,
                                                      syntax->arguments,
                                                      use_location);
      semantic_template_class::emit_constructor_initializer_template_id_source_use(
          ctx,
          *info.member_scope,
          *initializer_id,
          *syntax,
          use_location,
          argument_locations);
    }
  }
  trace_class_collection_event(ctx, "special-member-done", info, node);
}

namespace {

void collect_class_reference_special_member(SemanticContext & ctx,
                                            ClassInfo & info,
                                            const CppAstNode & node,
                                            MemberAccess access)
{
  const CppAstNode * member_specifiers = find_child(node, CppAstKind::member_specifiers);
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  if(!declarator) {
    return;
  }

  if(class_member_declares_conversion_operator(node)) {
    std::string member_name;
    TypePtr declared_type;
    std::vector<std::pair<std::string, TypePtr> > params;
    std::vector<const CppAstNode *> default_args;
    MethodSyntaxInfo syntax;
    if(!parse_conversion_operator_signature(ctx,
                                            *info.member_scope,
                                            node,
                                            member_name,
                                            declared_type,
                                            params,
                                            &default_args,
                                            &syntax)) {
      return;
    }
    trace_class_collection_event(ctx,
                                 "reference-conversion-operator-parsed",
                                 info,
                                 node,
                                 std::string("member=") + member_name);

    const bool is_friend =
        member_specifiers &&
        any_of(member_specifiers->children.begin(),
               member_specifiers->children.end(),
               [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); });
    if(is_friend) {
      register_friend_function_binding(ctx,
                                       info,
                                       member_name,
                                       nullptr,
                                       declared_type,
                                       params,
                                       default_args,
                                       nullptr,
                                       &node,
                                       declarator_function_qualifier(*declarator),
                                       member_specifiers &&
                                       decl_spec_contains_token(*member_specifiers, KW_CONSTEXPR));
      return;
    }

    ClassFunctionOptions conversion_options =
        class_function_options(
            access,
            &syntax,
            false,
            false,
            member_specifiers &&
                decl_spec_contains_token(*member_specifiers, KW_CONSTEXPR),
            false,
            member_specifiers &&
                decl_spec_contains_token(*member_specifiers, KW_INLINE));
    conversion_options.is_conversion_operator = true;
    register_class_function(ctx,
                            info,
                            member_name,
                            declared_type,
                            params,
                            default_args,
                            nullptr,
                            nullptr,
                            conversion_options,
                            &node);
    trace_class_collection_event(ctx,
                                 "reference-conversion-operator-done",
                                 info,
                                 node,
                                 std::string("member=") + member_name);
    return;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  const CppAstNode * parameter_clause = find_child(*declarator, CppAstKind::parameter_clause);
  if(parameter_clause &&
     !ctx.parse_parameter_clause(
         *info.member_scope, *parameter_clause, params, &default_args, true)) {
    return;
  }

  const bool is_constructor =
      special_member_matches_class_name(ctx, node.value, info, false);
  const bool is_destructor =
      special_member_matches_class_name(ctx, node.value, info, true);
  if(!is_constructor && !is_destructor) {
    return;
  }

  PreparedMethodParseContext prepared_method;
  prepare_method_parse_context(member_specifiers, *declarator, prepared_method);
  validate_method_virtual_syntax(prepared_method.syntax);
  const bool is_defaulted = special_member_is_defaulted(node);
  const bool is_deleted = special_member_is_deleted(node);
  FunctionBinding * binding =
      register_class_function(ctx,
                              info,
                              node.value,
                              TypePtr(),
                              params,
                              default_args,
                              nullptr,
                              nullptr,
                              class_function_options(
                                  access,
                                  &prepared_method.syntax,
                                  is_constructor,
                                  is_destructor,
                                  member_specifiers &&
                                      decl_spec_contains_token(*member_specifiers, KW_CONSTEXPR),
                                  is_defaulted,
                                  member_specifiers &&
                                      decl_spec_contains_token(*member_specifiers, KW_INLINE)),
                              &node);
  if(binding) {
    binding->is_deleted = is_deleted;
    binding->exclude_from_explicit_instantiation =
        binding->exclude_from_explicit_instantiation ||
        special_member_excluded_from_explicit_instantiation(node);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "collect-reference-special-member-binding class=" << info.qualified_name
            << " member=" << node.value
            << " exclude="
            << (binding->exclude_from_explicit_instantiation ? "yes" : "no")
            << " decl-node="
            << (binding->declaration_node ? cppast_kind_text(binding->declaration_node->kind)
                                          : std::string("<none>"));
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
  }
}

}  // namespace

void collect_conversion_operator_member(SemanticContext & ctx,
                                        ClassInfo & info,
                                        const CppAstNode & node,
                                        MemberAccess access)
{
  trace_class_collection_event(ctx, "conversion-operator-enter", info, node);
  const CppAstNode * member_specifiers = find_child(node, CppAstKind::member_specifiers);
  const CppAstNode * body = find_class_function_body_node(node);
  const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
  std::string member_name;
  TypePtr declared_type;
  std::vector<std::pair<std::string, TypePtr> > params;
  std::vector<const CppAstNode *> default_args;
  MethodSyntaxInfo syntax;
  if(!parse_conversion_operator_signature(ctx,
                                          *info.member_scope,
                                          node,
                                          member_name,
                                          declared_type,
                                          params,
                                          &default_args,
                                          &syntax)) {
    std::ostringstream out;
    out << "invalid conversion operator";
    out << " [class " << info.qualified_name << "]";
    out << " [node kind " << cppast_kind_text(node.kind) << "]";
    if(!node.value.empty()) {
      out << " [node value " << node.value << "]";
    }
    out << " [has member_specifiers " << (member_specifiers ? "yes" : "no") << "]";
    out << " [has declarator " << (declarator ? "yes" : "no") << "]";
    out << " [node ast={" << describe_cppast_translation_unit(node) << "}]";
    throw std::logic_error(out.str());
  }
  trace_class_collection_event(ctx,
                               "conversion-operator-parsed",
                               info,
                               node,
                               std::string("member=") + member_name);

  const bool is_friend =
      member_specifiers &&
      any_of(member_specifiers->children.begin(),
             member_specifiers->children.end(),
             [](const CppAstNode & child) { return node_has_simple_type(child, KW_FRIEND); });
  if(is_friend) {
    register_friend_function_binding(ctx,
                                     info,
                                     member_name,
                                     nullptr,
                                     declared_type,
                                     params,
                                     default_args,
                                     body,
                                     &node,
                                     declarator_function_qualifier(*declarator),
                                     member_specifiers &&
                                     decl_spec_contains_token(*member_specifiers, KW_CONSTEXPR));
    return;
  }

  ClassFunctionOptions conversion_options =
      class_function_options(
          access,
          &syntax,
          false,
          false,
          member_specifiers &&
              decl_spec_contains_token(*member_specifiers,
                                       KW_CONSTEXPR),
          false,
          member_specifiers &&
              decl_spec_contains_token(*member_specifiers,
                                       KW_INLINE));
  conversion_options.is_conversion_operator = true;
  FunctionBinding * binding =
      register_class_function(ctx,
                              info,
                              member_name,
                              declared_type,
                              params,
                              default_args,
                              body,
                              nullptr,
                              conversion_options,
                              &node);
  if(binding) {
    binding->display_name = semantic_utils::unqualified_member_name(node.value);
  }
  trace_class_collection_event(ctx,
                               "conversion-operator-done",
                               info,
                               node,
                               std::string("member=") + member_name);
}

void populate_class_info(SemanticContext & ctx,
                         ClassInfo & info,
                         const CppAstNode & node)
{
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->class_populate_by_demand[
        static_cast<std::size_t>(semantic_metrics::current_class_demand())];
  }
  trace_class_collection_event(ctx, "populate-class-enter", info, node);
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << info.qualified_name
          << " complete=" << (info.complete ? "yes" : "no")
          << " ref=" << (info.reference_members_collected ? "yes" : "no")
          << " full=" << (info.full_member_collection_in_progress ? "yes" : "no");
    semantic_hotspot::note_semantic_query("populate_class_info", query.str());
  }
  if(info.complete) {
    trace_class_collection_event(ctx, "populate-class-complete-skip", info, node);
    return;
  }
  if(info.full_member_collection_in_progress ||
     info.reference_member_collection_in_progress) {
    trace_class_collection_event(ctx,
                                 "populate-class-in-progress-skip",
                                 info,
                                 node);
    return;
  }
  if(info.reference_members_collected && !info.complete &&
     info.dependent_instantiation) {
    trace_class_collection_event(ctx, "populate-class-dependent-skip", info, node);
    return;
  }
  if(info.reference_members_collected && !info.complete) {
    if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
      counters->note_reference_before_full_collection(metrics_class_name(info),
                                                      class_member_walk_units(node));
    }
    ctx.discard_class_function_bindings_for_reset(info);
    reset_reference_member_state_for_full_collection(info);
  }
  bool full_collection_finished = false;
  struct FullCollectionGuard
  {
    ClassInfo & info;
    bool previous = false;
    explicit FullCollectionGuard(ClassInfo & info_)
        : info(info_), previous(info_.full_member_collection_in_progress)
    {
      info.full_member_collection_in_progress = true;
    }
    ~FullCollectionGuard()
    {
      info.full_member_collection_in_progress = previous;
    }
  } full_collection_guard(info);
  struct FullCollectionAbortCleanup
  {
    ClassInfo & info;
    bool & finished;
    ~FullCollectionAbortCleanup()
    {
      if(!finished && !info.complete && !info.reference_members_collected) {
        reset_reference_member_state_for_full_collection(info);
      }
    }
  } full_collection_abort_cleanup{info, full_collection_finished};

  {
    semantic_metrics::ScopedClassDemand class_demand(
        semantic_metrics::CDK_BASE_CLASS_COLLECTION);
    parse_base_clause(ctx, info, node);
  }
  trace_class_collection_event(ctx, "populate-class-bases-done", info, node);
  const bool dependent_class = class_instantiation_is_dependent(ctx, info);
  MemberAccess current_access = info.default_access;
  std::size_t anonymous_union_counter = 0;
  std::function<void(const CppAstNode &)> collect_named_class_member =
      [&](const CppAstNode & member)
  {
    if(member.value.empty()) {
      return;
    }
    const CppAstNode * class_key = find_child(member, CppAstKind::class_key);
    if(!class_key) {
      return;
    }
    auto found =
        info.member_scope->named_types.find(member.value);
    ClassInfo * existing_info =
        found == info.member_scope->named_types.end() ?
            nullptr :
            ctx.class_info_for_type(found->second);
    if(existing_info && existing_info->complete && existing_info->class_node == &member) {
      return;
    }
    ClassInfo * nested =
        ctx.create_class_info(*info.member_scope,
                              node_text(*class_key),
                              member.value,
                              &member);
    const bool owner_template_member_instantiation =
        template_api::class_template_completion_has_owner_definition(info);
    if(!info.source_template &&
       !owner_template_member_instantiation &&
       !dependent_class &&
       nested &&
       member.kind != CppAstKind::class_forward_declaration &&
       !nested->complete &&
       !nested->full_member_collection_in_progress &&
       !nested->reference_member_collection_in_progress) {
      {
        semantic_metrics::ScopedClassDemand class_demand(
            semantic_metrics::CDK_NESTED_CLASS_COLLECTION);
        populate_class_info(ctx, *nested, member);
      }
    }
  };
  std::function<void(const CppAstNode &, MemberAccess)> collect_anonymous_members =
      [&](const CppAstNode & anon, MemberAccess access)
  {
    MemberAccess inner_access = access;
    const bool anonymous_union = node_is_union_class(anon);
    if(anonymous_union) {
      collect_anonymous_union_storage(ctx,
                                      info,
                                      anon,
                                      access,
                                      false,
                                      ++anonymous_union_counter);
      return;
    }
    if(const CppAstNode * class_key = find_child(anon, CppAstKind::class_key)) {
      record_anonymous_member_class(info, anon, node_text(*class_key));
    }
    for(size_t j = 0; j < anon.children.size(); ++j) {
      if(info.complete && info.reference_members_collected) {
        trace_class_collection_event(ctx, "populate-class-mid-complete-skip", info, anon);
        return;
      }
      const CppAstNode & member = anon.children[j];
      if(member.kind == CppAstKind::class_key || member.kind == CppAstKind::base_clause) {
        continue;
      }
      if(member.kind == CppAstKind::access_specifier) {
        inner_access = access_from_node(member);
        continue;
      }
      if(member.kind == CppAstKind::empty_declaration) {
        continue;
      }
      if(member.kind == CppAstKind::simple_declaration) {
        if(dependent_class) {
          collect_dependent_class_simple_declaration(ctx, info, member, inner_access);
        } else {
          collect_class_simple_declaration(ctx, info, member, inner_access);
        }
        continue;
      }
      if(member.kind == CppAstKind::bit_field_declaration) {
        if(dependent_class) {
          collect_dependent_class_bit_field_declaration(ctx, info, member, inner_access);
        } else {
          collect_class_bit_field_declaration(ctx, info, member, inner_access);
        }
        continue;
      }
      if(member.kind == CppAstKind::static_assert_declaration) {
        semantic_declaration::analyze_static_assert_declaration(ctx, *info.member_scope, member);
        continue;
      }
      if((member.kind == CppAstKind::class_specifier ||
          member.kind == CppAstKind::class_forward_declaration) &&
         member.value.empty()) {
        collect_anonymous_members(member, inner_access);
        continue;
      }
      if(member.kind == CppAstKind::class_specifier ||
         member.kind == CppAstKind::class_forward_declaration) {
        collect_named_class_member(member);
        continue;
      }
      if(member.kind == CppAstKind::alias_declaration) {
        const CppAstNode * type_id = find_child(member, CppAstKind::type_id);
        if(!type_id) {
          throw std::logic_error("class alias-declaration missing type-id");
        }
        const std::string type_id_text = node_text(*type_id);
        TypePtr alias =
            parse_or_defer_class_alias_type_id(ctx,
                                               info,
                                               member.value,
                                               *type_id,
                                               type_id_text,
                                               dependent_class);
        if(!alias) {
          throw std::logic_error("unsupported class alias-declaration");
        }
        alias = refine_instantiated_class_alias(ctx, *info.member_scope, alias);
        trace_class_alias_store(
            ctx, info, "populate-anonymous", member.value, type_id_text, alias);
        semantic_scope_mutation::bind_template_named_type_with_access(
            *info.member_scope, member.value, alias, inner_access);
        continue;
      }
      throw std::logic_error("unsupported anonymous class member in hosted slice");
    }
  };

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(info.complete && info.reference_members_collected) {
      trace_class_collection_event(ctx, "populate-class-mid-complete-skip", info, node);
      return;
    }
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::class_key || child.kind == CppAstKind::base_clause) {
      continue;
    }
    if(child.kind == CppAstKind::access_specifier) {
      current_access = access_from_node(child);
      continue;
    }
    if(child.kind == CppAstKind::empty_declaration) {
      continue;
    }
    if(child.kind == CppAstKind::simple_declaration) {
      if(dependent_class) {
        collect_dependent_class_simple_declaration(ctx, info, child, current_access);
      } else {
        collect_class_simple_declaration(ctx, info, child, current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::bit_field_declaration) {
      if(dependent_class) {
        collect_dependent_class_bit_field_declaration(ctx, info, child, current_access);
      } else {
        collect_class_bit_field_declaration(ctx, info, child, current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::static_assert_declaration) {
      semantic_declaration::analyze_static_assert_declaration(ctx, *info.member_scope, child);
      continue;
    }
    if(child.kind == CppAstKind::function_definition) {
      const CppAstNode * function_specifiers =
          find_child(child, CppAstKind::decl_specifier_seq);
      if(function_specifiers &&
         any_of(function_specifiers->children.begin(),
                function_specifiers->children.end(),
                [](const CppAstNode & member) { return node_has_simple_type(member, KW_FRIEND); })) {
        collect_class_friend_function_definition(ctx, info, child, true);
        continue;
      }
      DIAG_CONTEXT("class_member_function_definition [" + node_text(child) + "]" +
                   ctx.source_location_for_node(child));
      if(!dependent_class || function_definition_is_static_constexpr_member(child)) {
        collect_class_method_definition(ctx, info, child, current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::template_declaration) {
      DIAG_CONTEXT("class_member_template_declaration [" + node_text(child) + "]" +
                   ctx.source_location_for_node(child));
      collect_class_friend_declaration(ctx, info, child);
      try {
        const template_api::ScopedTemplateWitnessFunctionCallSourceCapturePause
            class_source_capture_pause;
        ctx.collect_template_declaration(*info.member_scope, child, current_access);
      } catch(const TemplateSubstitutionFailure &) {
        if(dependent_class || !info.source_template) {
          throw;
        }
        continue;
      }
      continue;
    }
    if(child.kind == CppAstKind::special_member_definition ||
       child.kind == CppAstKind::special_member_declaration) {
      DIAG_CONTEXT("class_special_member [" + node_text(child) + "]" +
                   ctx.source_location_for_node(child));
      if(class_member_declares_conversion_operator(child)) {
        collect_conversion_operator_member(ctx, info, child, current_access);
      } else {
        collect_special_member(ctx, info, child, current_access);
      }
      continue;
    }
    if(child.kind == CppAstKind::class_specifier ||
       child.kind == CppAstKind::class_forward_declaration) {
      if(child.value.empty()) {
        collect_anonymous_members(child, current_access);
      } else {
        collect_named_class_member(child);
      }
      continue;
    }
    if(child.kind == CppAstKind::enum_specifier) {
      ctx.collect_enum_declaration(*info.member_scope, child);
      continue;
    }
    if(child.kind == CppAstKind::alias_declaration) {
      const CppAstNode * type_id = find_child(child, CppAstKind::type_id);
      if(!type_id) {
        throw std::logic_error("class alias-declaration missing type-id");
      }
      const std::string type_id_text = node_text(*type_id);
      TypePtr alias =
          parse_or_defer_class_alias_type_id(ctx,
                                             info,
                                             child.value,
                                             *type_id,
                                             type_id_text,
                                             dependent_class);
      alias = refine_instantiated_class_alias(ctx, *info.member_scope, alias);
      if(!alias) {
        std::ostringstream out;
        out << "unsupported class alias-declaration";
        if(!child.value.empty()) {
          out << " [" << child.value << "]";
        }
        out << " [type-id " << node_text(*type_id) << "]";
        out << " [type-id-text " << type_id_text << "]";
        if(type_id_text.empty()) {
          out << " [type-id-ast {" << describe_cppast_translation_unit(*type_id) << "}]";
        }
        throw std::logic_error(out.str());
      }
      trace_class_alias_store(
          ctx, info, "populate-top", child.value, type_id_text, alias);
      semantic_scope_mutation::bind_template_named_type_with_access(
          *info.member_scope, child.value, alias, current_access);
      continue;
    }
    if(child.kind == CppAstKind::using_declaration) {
      if(!collect_inherited_constructors(ctx, info, child, current_access)) {
        semantic_declaration::collect_using_declaration(ctx,
                                                        *info.member_scope,
                                                        child,
                                                        current_access);
      }
      continue;
    }
    std::ostringstream out;
    out << "unsupported class member in PA14";
    out << " [class " << info.qualified_name << "]";
    out << " [member kind " << cppast_kind_text(child.kind) << "]";
    if(!child.value.empty()) {
      out << " [member value " << child.value << "]";
    }
    out << " [member text " << node_text(child) << "]";
    if(node_text(child).empty()) {
      out << " [member ast={" << describe_cppast_translation_unit(child) << "}]";
    }
    throw std::logic_error(out.str());
  }

  if(dependent_class) {
    info.reference_members_collected = true;
    ctx.finalize_dependent_class_shape(info);
    trace_class_collection_event(ctx, "populate-class-dependent-finalized", info, node);
    full_collection_finished = true;
    return;
  }

  trace_class_collection_event(ctx, "populate-class-finalize-constants", info, node);
  finalize_class_constant_members(ctx, info);
  trace_class_collection_event(ctx, "populate-class-finalize-implicit-specials", info, node);
  {
    semantic_metrics::ScopedClassDemand class_demand(
        semantic_metrics::CDK_IMPLICIT_SPECIAL_MEMBERS);
    ensure_implicit_special_members(ctx, info);
  }
  info.implicit_special_members_ensured = true;
  trace_class_collection_event(ctx, "populate-class-finalize-virtuals", info, node);
  {
    semantic_metrics::ScopedClassDemand class_demand(
        semantic_metrics::CDK_CLASS_VIRTUALS);
    finalize_class_virtuals(ctx, info);
  }
  complete_deferred_class_member_object_layouts(ctx, info);
  if(class_has_deferred_class_member_object_layouts(ctx, info)) {
    info.reference_members_collected = true;
    trace_class_collection_event(ctx, "populate-class-deferred-member-layout", info, node);
    full_collection_finished = true;
    return;
  }
  if(ctx.class_layout_depends_on_template_parameters(info)) {
    info.reference_members_collected = true;
    ctx.finalize_dependent_class_shape(info);
    trace_class_collection_event(ctx, "populate-class-dependent-layout", info, node);
    full_collection_finished = true;
    return;
  }
  trace_class_collection_event(ctx, "populate-class-finalize-layout", info, node);
  {
    semantic_metrics::ScopedClassDemand class_demand(
        semantic_metrics::CDK_CLASS_LAYOUT);
    finalize_class_layout(ctx, info);
  }
  validate_class_member_function_static_asserts(ctx, info);
  info.reference_members_collected = true;
  template_api::note_anonymous_member_class_events_if_owner_logged(ctx, info);
  trace_class_collection_event(ctx, "populate-class-done", info, node);
  full_collection_finished = true;
}

void collect_class_declaration(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & node)
{
  if(node.value.empty()) {
    throw std::logic_error("anonymous classes unsupported");
  }

  const CppAstNode * class_key = find_child(node, CppAstKind::class_key);
  if(!class_key) {
    throw std::logic_error("class declaration missing class-key");
  }

  Scope * target_scope = nullptr;
  std::string class_name;
  const QualifiedName * qualified_name = cppast_qualified_name_syntax(node);
  QualifiedName generated_unqualified_name;
  if(!qualified_name && node.value.find("::") == std::string::npos) {
    generated_unqualified_name.name = node.value;
    qualified_name = &generated_unqualified_name;
  }
  if(!qualified_name ||
     !ctx.resolve_declared_class_scope_and_name(
         scope, *qualified_name, target_scope, class_name)) {
    throw std::logic_error("unknown qualified class scope");
  }

  ClassInfo * info = ctx.create_class_info(*target_scope,
                                           node_text(*class_key),
                                           class_name,
                                           &node);
  if(node.kind == CppAstKind::class_forward_declaration) {
    return;
  }
  if(info->complete) {
    if(class_definition_is_replayed_complete_node(*info, node)) {
      return;
    }
    throw std::logic_error(std::string("duplicate class definition") +
                           semantic_trace::current_location_note(ctx, &node) +
                           semantic_trace::previous_class_location_note(
                               ctx, "previous definition", info));
  }

  if(class_instantiation_is_dependent(ctx, *info) ||
     class_definition_has_member_function_static_assert(node) ||
     class_definition_needs_virtual_validation(node) ||
     semantic_lookup::current_function_scope(*target_scope) != nullptr) {
    populate_class_info(ctx, *info, node);
  } else {
    ensure_class_reference_members(ctx, *info);
  }
}

void ensure_implicit_special_members(SemanticContext & ctx,
                                     ClassInfo & info)
{
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << info.qualified_name
          << " complete=" << (info.complete ? "yes" : "no")
          << " full_collect=" << (info.full_member_collection_in_progress ? "yes" : "no");
    semantic_hotspot::note_semantic_query("ensure_implicit_special_members", query.str());
  }
  if(info.implicit_special_members_ensured) {
    refresh_defaulted_copy_assignment_state(ctx, info);
    refresh_defaulted_move_assignment_state(ctx, info);
    return;
  }
  const auto has_user_declared_constructor = [&]() -> bool
  {
    for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
            info.methods.begin();
        it != info.methods.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        const FunctionBinding * binding = it->second[i];
        if(binding->is_constructor && !binding->synthesized) {
          return true;
        }
      }
    }

    for(std::map<std::string, std::vector<FunctionTemplateDecl *> >::const_iterator it =
            info.member_scope->function_templates.begin();
        it != info.member_scope->function_templates.end();
        ++it) {
      for(size_t i = 0; i < it->second.size(); ++i) {
        const FunctionTemplateDecl * decl = it->second[i];
        if(decl && decl->is_constructor) {
          return true;
        }
      }
    }

    return false;
  };

  if(!has_user_declared_constructor()) {
    const std::string ctor_name = constructor_member_name_for_class(ctx, info);
    std::vector<TypePtr> effective_params;
    effective_params.push_back(make_pointer(info.type));
    FunctionBinding * ctor =
        ctx.find_exact_class_function(info,
                                      ctor_name,
                                      make_function(make_fundamental(FT_VOID),
                                                    effective_params,
                                                    false));
    if(!ctor) {
      ClassFunctionOptions options;
      options.access = MA_PUBLIC;
      options.is_constructor = true;
      FunctionRegistrationRequest request;
      request.owner_class = &info;
      request.name = ctor_name;
      request.semantic_flags = options;
      ctor = ctx.register_function_entity(request);
      ctor->synthesized = true;
      ctx.upgrade_function_symbol_linkage(ctor,
                                          synthesized_class_member_symbol_linkage(info));
    }
    const bool finalized_complete_ctor =
        info.complete &&
        ctor->synthesized &&
        (ctor->has_definition || ctor->is_deleted);
    if(!finalized_complete_ctor) {
      ctor->is_deleted = implicit_default_constructor_is_deleted(ctx, info);
      ctor->has_definition = !ctor->is_deleted;
    }
  }

  const std::string dtor_name = destructor_member_name_for_class(ctx, info);
  if(info.methods.find(dtor_name) == info.methods.end()) {
    ClassFunctionOptions options;
    options.access = MA_PUBLIC;
    options.is_destructor = true;
    FunctionRegistrationRequest request;
    request.owner_class = &info;
    request.name = dtor_name;
    request.semantic_flags = options;
    FunctionBinding * dtor = ctx.register_function_entity(request);
    dtor->has_definition = true;
    dtor->synthesized = true;
    ctx.upgrade_function_symbol_linkage(dtor,
                                        synthesized_class_member_symbol_linkage(info));
  }
  if(info.complete) {
    refresh_defaulted_copy_assignment_state(ctx, info);
    refresh_defaulted_move_assignment_state(ctx, info);
    info.implicit_special_members_ensured = true;
  }
}

FunctionBinding * ensure_implicit_copy_constructor(SemanticContext & ctx,
                                                   ClassInfo & info)
{
  const std::string ctor_name = constructor_member_name_for_class(ctx, info);
  FunctionBinding * user_declared = find_constructor_binding(info, Type::TK_LVALUE_REFERENCE);
  if(user_declared && !user_declared->synthesized) {
    return user_declared;
  }

  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  effective_params.push_back(const_lvalue_reference_to(info.type));
  FunctionBinding * existing =
      ctx.find_exact_class_function(info,
                                    ctor_name,
                                    make_function(make_fundamental(FT_VOID),
                                                  effective_params,
                                                  false));
  if(existing) {
    existing->is_copy_constructor = true;
    if(existing->synthesized && info.complete) {
      existing->is_deleted = implicit_copy_constructor_is_deleted(ctx, info);
      existing->has_definition = !existing->is_deleted;
    }
    return existing;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  params.push_back(std::make_pair(std::string("other"), const_lvalue_reference_to(info.type)));
  ClassFunctionOptions options;
  options.access = MA_PUBLIC;
  options.is_constructor = true;
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = ctor_name;
  request.params = params;
  request.semantic_flags = options;
  FunctionBinding * ctor = ctx.register_function_entity(request);
  ctor->synthesized = true;
  ctor->is_copy_constructor = true;
  if(info.complete) {
    ctor->is_deleted = implicit_copy_constructor_is_deleted(ctx, info);
    ctor->has_definition = !ctor->is_deleted;
  } else {
    ctor->has_definition = true;
  }
  ctx.upgrade_function_symbol_linkage(ctor,
                                      synthesized_class_member_symbol_linkage(info));
  return ctor;
}

FunctionBinding * ensure_implicit_move_constructor(SemanticContext & ctx,
                                                   ClassInfo & info)
{
  const std::string ctor_name = constructor_member_name_for_class(ctx, info);
  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  effective_params.push_back(make_rvalue_reference_raw(info.type));
  FunctionBinding * existing =
      ctx.find_exact_class_function(info,
                                    ctor_name,
                                    make_function(make_fundamental(FT_VOID),
                                                  effective_params,
                                                  false));
  if(existing) {
    existing->is_move_constructor = true;
    if(existing->synthesized && info.complete) {
      existing->is_deleted = implicit_move_constructor_is_deleted(ctx, info);
      existing->has_definition = !existing->is_deleted;
    }
    return existing;
  }

  if(has_user_declared_copy_constructor(info) ||
     has_user_declared_copy_assignment(info) ||
     has_user_declared_move_assignment(info) ||
     has_user_declared_destructor(info)) {
    return nullptr;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  params.push_back(std::make_pair(std::string("other"), make_rvalue_reference_raw(info.type)));
  ClassFunctionOptions options;
  options.access = MA_PUBLIC;
  options.is_constructor = true;
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = ctor_name;
  request.params = params;
  request.semantic_flags = options;
  FunctionBinding * ctor = ctx.register_function_entity(request);
  ctor->synthesized = true;
  ctor->is_move_constructor = true;
  if(info.complete) {
    ctor->is_deleted = implicit_move_constructor_is_deleted(ctx, info);
    ctor->has_definition = !ctor->is_deleted;
  } else {
    ctor->has_definition = true;
  }
  ctx.upgrade_function_symbol_linkage(ctor,
                                      synthesized_class_member_symbol_linkage(info));
  return ctor;
}

FunctionBinding * ensure_implicit_move_assignment(SemanticContext & ctx,
                                                  ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator existing_methods =
      info.methods.find("operator=");
  if(existing_methods != info.methods.end()) {
    for(size_t i = 0; i < existing_methods->second.size(); ++i) {
      if(existing_methods->second[i]->is_move_assignment) {
        if(existing_methods->second[i]->synthesized && info.complete) {
          existing_methods->second[i]->is_deleted =
              defaulted_move_assignment_is_deleted(ctx, info);
          existing_methods->second[i]->has_definition =
              !existing_methods->second[i]->is_deleted;
        }
        return existing_methods->second[i];
      }
    }
  }

  if(info.is_lambda_closure) {
    return nullptr;
  }

  if(has_user_declared_copy_constructor(info) ||
     has_user_declared_copy_assignment(info) ||
     has_user_declared_move_constructor(info) ||
     has_user_declared_destructor(info)) {
    return nullptr;
  }

  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  effective_params.push_back(make_rvalue_reference_raw(info.type));
  TypePtr declared_type =
      make_function(make_lvalue_reference_raw(info.type),
                    std::vector<TypePtr>(1, make_rvalue_reference_raw(info.type)),
                    false);
  FunctionBinding * existing =
      ctx.find_exact_class_function(info,
                                    "operator=",
                                    make_function(make_lvalue_reference_raw(info.type),
                                                  effective_params,
                                                  false));
  if(existing) {
    if(existing->synthesized && info.complete) {
      existing->is_deleted = defaulted_move_assignment_is_deleted(ctx, info);
      existing->has_definition = !existing->is_deleted;
    }
    return existing;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  params.push_back(std::make_pair(std::string("other"), make_rvalue_reference_raw(info.type)));
  ClassFunctionOptions options;
  options.access = MA_PUBLIC;
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = "operator=";
  request.declared_type = declared_type;
  request.params = params;
  request.semantic_flags = options;
  FunctionBinding * op = ctx.register_function_entity(request);
  op->synthesized = true;
  if(info.complete) {
    op->is_deleted = defaulted_move_assignment_is_deleted(ctx, info);
    op->has_definition = !op->is_deleted;
  } else {
    op->has_definition = true;
  }
  ctx.upgrade_function_symbol_linkage(op,
                                      synthesized_class_member_symbol_linkage(info));
  return op;
}

bool can_synthesize_aggregate_constructor(const ClassInfo & info)
{
  if(!info.bases.empty() || info.is_polymorphic) {
    return false;
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    const FieldInfo & field = info.fields[i];
    if(field.default_initializer ||
       field.access == MA_PRIVATE ||
       field.access == MA_PROTECTED) {
      return false;
    }
  }

  for(std::map<std::string, std::vector<FunctionTemplateDecl *> >::const_iterator it =
          info.member_scope->function_templates.begin();
      it != info.member_scope->function_templates.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionTemplateDecl * decl = it->second[i];
      if(decl && decl->is_constructor) {
        return false;
      }
    }
  }

  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionBinding * binding = it->second[i];
      if(binding->is_constructor) {
        if(binding->is_explicit) {
          return false;
        }
        if(!binding->synthesized &&
           !binding->is_defaulted &&
           !binding->is_deleted) {
          return false;
        }
      }
    }
  }

  return true;
}

bool class_has_bit_fields(const ClassInfo & info)
{
  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(info.fields[i].is_bit_field) {
      return true;
    }
  }
  return false;
}

std::size_t aggregate_element_count(const ClassInfo & info)
{
  return aggregate_element_count_impl(info);
}

const FieldInfo * first_aggregate_field(const ClassInfo & info)
{
  return first_aggregate_field_impl(info);
}

ClassInfo * anonymous_storage_union_info(SemanticContext & ctx, const FieldInfo & field)
{
  return anonymous_storage_union_info_impl(ctx, field);
}

const FieldInfo * aggregate_input_field(SemanticContext & ctx, const FieldInfo & field)
{
  return aggregate_input_field_impl(ctx, field);
}

TypePtr aggregate_constructor_parameter_type(SemanticContext & ctx,
                                             const FieldInfo & field)
{
  return aggregate_constructor_parameter_type_impl(ctx, field);
}

FunctionBinding * ensure_implicit_aggregate_constructor(SemanticContext & ctx,
                                                        ClassInfo & info)
{
  return ensure_implicit_aggregate_constructor(ctx, info, aggregate_element_count(info));
}

FunctionBinding * ensure_implicit_aggregate_constructor(SemanticContext & ctx,
                                                        ClassInfo & info,
                                                        size_t explicit_arg_count)
{
  if(!can_synthesize_aggregate_constructor(info)) {
    return nullptr;
  }
  const std::size_t aggregate_count = aggregate_element_count(info);
  if(explicit_arg_count > aggregate_count) {
    return nullptr;
  }

  const std::string ctor_name = constructor_member_name_for_class(ctx, info);
  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  std::vector<std::pair<std::string, TypePtr> > params;
  for(size_t i = 0; i < explicit_arg_count; ++i) {
    const FieldInfo & field = info.fields[i];
    const TypePtr param_type = aggregate_constructor_parameter_type_impl(ctx, field);
    effective_params.push_back(param_type);
    std::string param_name = field.name.empty() ?
        std::string("__aggregate_arg") + std::to_string(i) : info.fields[i].name;
    params.push_back(std::make_pair(param_name, param_type));
  }

  FunctionBinding * existing =
      ctx.find_exact_class_function(info,
                                    ctor_name,
                                    make_function(make_fundamental(FT_VOID),
                                                  effective_params,
                                                  false));
  if(existing) {
    return existing;
  }

  ClassFunctionOptions options;
  options.access = MA_PUBLIC;
  options.is_constructor = true;
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = ctor_name;
  request.params = params;
  request.semantic_flags = options;
  FunctionBinding * ctor = ctx.register_function_entity(request);
  ctor->has_definition = true;
  ctor->synthesized = true;
  ctor->is_aggregate_constructor = true;
  ctx.upgrade_function_symbol_linkage(ctor,
                                      synthesized_class_member_symbol_linkage(info));
  return ctor;
}

FunctionBinding * ensure_implicit_copy_assignment(SemanticContext & ctx,
                                                  ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator existing_methods =
      info.methods.find("operator=");
  if(existing_methods != info.methods.end()) {
    for(size_t i = 0; i < existing_methods->second.size(); ++i) {
      if(existing_methods->second[i]->is_copy_assignment) {
        if(existing_methods->second[i]->synthesized && info.complete) {
          existing_methods->second[i]->is_deleted =
              defaulted_copy_assignment_is_deleted(ctx, info, true);
          existing_methods->second[i]->has_definition =
              !existing_methods->second[i]->is_deleted;
        }
        return existing_methods->second[i];
      }
    }
  }

  if(info.is_lambda_closure) {
    return nullptr;
  }

  std::vector<TypePtr> effective_params;
  effective_params.push_back(make_pointer(info.type));
  effective_params.push_back(const_lvalue_reference_to(info.type));
  TypePtr declared_type =
      make_function(make_lvalue_reference_raw(info.type),
                    std::vector<TypePtr>(1, const_lvalue_reference_to(info.type)),
                    false);
  FunctionBinding * existing =
      ctx.find_exact_class_function(info,
                                    "operator=",
                                    make_function(make_lvalue_reference_raw(info.type),
                                                  effective_params,
                                                  false));
  if(existing) {
    if(existing->synthesized && info.complete) {
      existing->is_deleted = defaulted_copy_assignment_is_deleted(ctx, info, true);
      existing->has_definition = !existing->is_deleted;
    }
    return existing;
  }

  std::vector<std::pair<std::string, TypePtr> > params;
  params.push_back(std::make_pair(std::string("other"), const_lvalue_reference_to(info.type)));
  ClassFunctionOptions options;
  options.access = MA_PUBLIC;
  FunctionRegistrationRequest request;
  request.owner_class = &info;
  request.name = "operator=";
  request.declared_type = declared_type;
  request.params = params;
  request.semantic_flags = options;
  FunctionBinding * op = ctx.register_function_entity(request);
  op->synthesized = true;
  if(info.complete) {
    op->is_deleted = defaulted_copy_assignment_is_deleted(ctx, info, true);
    op->has_definition = !op->is_deleted;
  } else {
    op->has_definition = true;
  }
  ctx.upgrade_function_symbol_linkage(op,
                                      synthesized_class_member_symbol_linkage(info));
  return op;
}

bool is_trivially_default_constructible_type_for_host_abi(SemanticContext & ctx,
                                                          const TypePtr & type)
{
  return is_trivially_default_constructible_type_for_host_abi_local(ctx, type);
}

bool is_trivially_destructible_type_for_host_abi(SemanticContext & ctx,
                                                 const TypePtr & type)
{
  return is_trivially_destructible_type_for_host_abi_local(ctx, type);
}

bool is_trivially_copy_constructible_type_for_host_abi(SemanticContext & ctx,
                                                       const TypePtr & type)
{
  return is_trivially_copy_constructible_type_for_host_abi_local(ctx, type);
}

bool is_trivially_move_constructible_type_for_host_abi(SemanticContext & ctx,
                                                       const TypePtr & type)
{
  return is_trivially_move_constructible_type_for_host_abi_local(ctx, type);
}

}  // namespace semantic_class_model
