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

bool has_designated_braced_init(const CppAstNode & node);

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

void note_constructor_witness_closure(SemanticContext & ctx,
                                      semantic_model::FunctionBinding * ctor);

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
