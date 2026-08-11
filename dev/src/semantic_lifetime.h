#pragma once

#include <vector>

#include "callsem_output.h"
#include "cpp_decl_model.h"

namespace semantic_model {
struct Scope;
struct FunctionBinding;
struct ClassInfo;
}

namespace semantic_conversion {
struct ExprInfo;
}

class SemanticContext;
struct CppAstNode;

namespace semantic_lifetime {

std::string earliest_source_location_for_node(SemanticContext & ctx,
                                              const CppAstNode & node);

bool has_designated_braced_init(const CppAstNode & node);

bool scalar_list_initialization_has_narrowing_conversion(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & initializer,
    const cpp_decl::TypePtr & target_type);

bool build_aggregate_initializer_plan(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const semantic_model::ClassInfo & info,
    const CppAstNode & payload,
    std::vector<const CppAstNode *> & field_initializers,
    std::vector<CppAstNode> & synthesized_nodes);

bool build_designated_aggregate_initializer_plan(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const semantic_model::ClassInfo & info,
    const CppAstNode & payload,
    std::vector<const CppAstNode *> & field_initializers,
    std::vector<CppAstNode> & synthesized_nodes);

bool build_aggregate_constructor_source_args(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    semantic_model::ClassInfo & info,
    const CppAstNode & payload,
    std::vector<semantic_conversion::ExprInfo> & source_args);

bool resolve_constructor_base_initializer(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const semantic_model::FunctionBinding & binding,
    const semantic_model::ClassInfo & owner_info,
    const semantic_model::ClassInfo & base_info,
    CppAstNode & expanded_storage,
    const CppAstNode *& initializer);

void analyze_initializer(SemanticContext & ctx,
                         semantic_model::Scope & scope,
                         const cpp_decl::TypePtr & type,
                         const CppAstNode & node,
                         CallSemNode & out);

void analyze_object_lifetime_actions(SemanticContext & ctx,
                                     semantic_model::Scope & scope,
                                     const std::string & name,
                                     const cpp_decl::TypePtr & type,
                                     const CppAstNode * initializer,
                                     CallSemNode & out,
                                     const std::string & object_use_location = std::string());

void acquire_constructor_witness_definition(
    SemanticContext & ctx,
    semantic_model::FunctionBinding * ctor);

void require_destructor_action_if_needed(SemanticContext & ctx,
                                         const cpp_decl::TypePtr & type,
                                         bool allow_host_abi_skip = true);

void require_reference_bound_temporary_destructor_if_needed(
    SemanticContext & ctx,
    const cpp_decl::TypePtr & target,
    const semantic_conversion::ExprInfo & expr);

void append_named_object_destructor_action(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & name,
    const cpp_decl::TypePtr & type,
    CallSemNode & out);

void append_constructor_generated_statements(SemanticContext & ctx,
                                             semantic_model::Scope & scope,
                                             semantic_model::FunctionBinding & binding,
                                             symbol_linkage::SpecialMemberEntryPointKind entry_point_kind,
                                             CallSemNode & function_node);

void append_copy_assignment_generated_statements(SemanticContext & ctx,
                                                 semantic_model::Scope & scope,
                                                 semantic_model::FunctionBinding & binding,
                                                 CallSemNode & function_node);

void append_move_assignment_generated_statements(SemanticContext & ctx,
                                                 semantic_model::Scope & scope,
                                                 semantic_model::FunctionBinding & binding,
                                                 CallSemNode & function_node);

void append_destructor_generated_statements(SemanticContext & ctx,
                                            semantic_model::Scope & scope,
                                            semantic_model::FunctionBinding & binding,
                                            symbol_linkage::SpecialMemberEntryPointKind entry_point_kind,
                                            CallSemNode & function_node);

}  // namespace semantic_lifetime
