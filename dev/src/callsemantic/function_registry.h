#pragma once

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "callsem_output.h"
#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "symbol_linkage.h"

namespace template_model {
struct TemplateArgument;
}

namespace callsemantic {

struct FunctionRegistryState
{
  std::vector<std::unique_ptr<semantic_model::FunctionBinding> > & functions;
  std::unordered_map<std::string,
                     std::vector<semantic_model::FunctionBinding *> > &
      functions_by_internal_symbol;
  std::unordered_map<std::string,
                     std::vector<semantic_model::FunctionBinding *> > &
      functions_by_name;
  std::set<std::string> & used_internal_symbols;
  std::vector<semantic_model::FunctionBinding *> & instantiated_functions;
  std::unordered_set<semantic_model::FunctionBinding *> &
      instantiated_function_set;
  std::vector<semantic_model::FunctionBinding *> &
      required_function_definitions;
  std::unordered_set<semantic_model::FunctionBinding *> &
      required_function_definition_set;
  std::vector<semantic_model::FunctionBinding *> & late_required_class_methods;
  std::unordered_set<semantic_model::FunctionBinding *> &
      late_required_class_method_set;
  std::vector<semantic_model::FunctionBinding *> &
      late_required_class_static_functions;
  std::unordered_set<semantic_model::FunctionBinding *> &
      late_required_class_static_function_set;
  std::vector<semantic_model::FunctionBinding *> & synthetic_functions;
  std::vector<semantic_model::FunctionBinding *> & deferred_constexpr_functions;
  std::vector<semantic_model::FunctionBinding *> &
      pending_function_semantic_validation;
  std::unordered_set<semantic_model::FunctionBinding *> &
      queued_function_semantic_validation;
  std::unordered_set<semantic_model::FunctionBinding *> &
      completed_function_semantic_validation;
  std::size_t & function_semantic_validation_index;
  std::unordered_set<const semantic_model::FunctionBinding *> & live_functions;
  std::vector<std::unique_ptr<semantic_model::FunctionBinding> > &
      retired_functions;
  std::size_t & active_function_binding_borrows;
};

struct FunctionRegistryCallbacks
{
  std::function<bool(const cpp_decl::TypePtr &, const cpp_decl::TypePtr &)>
      types_equivalent_for_member_binding;
};

semantic_model::FunctionBinding * find_function_by_symbol(
    const FunctionRegistryState & state,
    const FunctionRegistryCallbacks & callbacks,
    const symbol_linkage::SymbolIdentity & symbol,
    const std::string & name,
    const cpp_decl::TypePtr & type);

bool types_equivalent_for_member_binding(const cpp_decl::TypePtr & lhs,
                                         const cpp_decl::TypePtr & rhs);

bool function_types_equivalent_for_member_signature(
    const cpp_decl::TypePtr & lhs,
    const cpp_decl::TypePtr & rhs);

bool function_binding_matches_materialized_owner_template_identity(
    const semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key);

bool function_binding_matches_instantiation_identity(
    const semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key);

void maybe_adopt_materialized_owner_template_identity(
    semantic_model::FunctionBinding & binding,
    semantic_model::FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<template_model::TemplateArgument> * instantiation_arguments,
    semantic_model::Scope * declaration_scope);

semantic_model::FunctionBinding * find_exact_function_binding(
    std::map<std::string, std::vector<semantic_model::FunctionBinding *> > & functions,
    const std::string & name,
    const cpp_decl::TypePtr & type,
    semantic_model::FunctionTemplateDecl * source_template = nullptr,
    const std::string & instantiation_key = std::string(),
    semantic_model::RefQualifier ref_qualifier =
        static_cast<semantic_model::RefQualifier>(0));

semantic_model::FunctionBinding * find_defined_function_binding(
    std::map<std::string, std::vector<semantic_model::FunctionBinding *> > & functions,
    const std::string & name,
    const cpp_decl::TypePtr & type,
    semantic_model::FunctionTemplateDecl * source_template = nullptr,
    const std::string & instantiation_key = std::string(),
    semantic_model::RefQualifier ref_qualifier =
        static_cast<semantic_model::RefQualifier>(0));

void index_function_binding(FunctionRegistryState & state,
                            semantic_model::FunctionBinding * binding);

void erase_indexed_function_binding(FunctionRegistryState & state,
                                    semantic_model::FunctionBinding * binding);

void release_function_symbol_reservation(
    FunctionRegistryState & state,
    const semantic_model::FunctionBinding * binding,
    const semantic_model::FunctionBinding * retained_binding = nullptr);

void discard_function_binding(FunctionRegistryState & state,
                              semantic_model::FunctionBinding * binding);

bool class_function_binding_output_has_started(
    const semantic_model::ClassInfo & info);

void discard_class_function_bindings(FunctionRegistryState & state,
                                     semantic_model::ClassInfo & info);

semantic_model::FunctionBinding * first_function_by_internal_symbol(
    const FunctionRegistryState & state,
    const std::string & internal_symbol);

bool resolve_dump_callee_binding(
    const FunctionRegistryState & state,
    const FunctionRegistryCallbacks & callbacks,
    const CallSemNode & callee,
    semantic_model::FunctionBinding *& binding);

}  // namespace callsemantic
