#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "constant_value.h"
#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"

class SemanticContext;

namespace resolved_source_semantics {
struct ResolvedQualifiedId;
}

namespace constant_eval {
class Evaluator;
}

namespace callsemantic {

struct ConstantCallEvaluationState
{
  const std::vector<std::unique_ptr<semantic_model::FunctionBinding> > & functions;
};

struct ConstantCallEvaluationCallbacks
{
  std::function<void(semantic_model::Scope &,
                     const CppAstNode &,
                     const resolved_source_semantics::ResolvedQualifiedId &,
                     const cpp_decl::TemplateIdSyntax *,
                     std::size_t)>
      record_constexpr_direct_function_call_source_use;
};

bool evaluate_constant_call_expression_value(
    SemanticContext & ctx,
    const ConstantCallEvaluationState & state,
    const ConstantCallEvaluationCallbacks & callbacks,
    semantic_model::Scope & scope,
    constant_eval::Evaluator & evaluator,
    const CppAstNode & node,
    const std::vector<constant_eval::ConstexprValue> & args,
    constant_eval::ConstexprValue & out);

}  // namespace callsemantic
