#pragma once

#include "callsem_output.h"
#include "cpp_decl_model.h"
#include "semantic_conversion.h"

namespace semantic_model {
struct Scope;
}

class SemanticContext;
struct CppAstNode;

namespace semantic_statement {

void analyze_statement(SemanticContext & ctx,
                       semantic_model::Scope & scope,
                       const cpp_decl::TypePtr & return_type,
                       const CppAstNode & node,
                       CallSemNode & out);

void collect_return_expressions(SemanticContext & ctx,
                                semantic_model::Scope & scope,
                                const CppAstNode & node,
                                std::vector<semantic_conversion::ExprInfo> & out,
                                bool & saw_void_return);

void analyze_compound_body_and_collect_returns(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    CallSemNode & out,
    std::vector<semantic_conversion::ExprInfo> & collected_returns,
    bool & saw_void_return);

}  // namespace semantic_statement
