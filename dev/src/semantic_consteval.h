#pragma once

#include <string>
#include <vector>

#include "constant_value.h"
#include "constexpr_eval.h"

class SemanticContext;

namespace semantic_model {
struct FunctionBinding;
struct Scope;
}

namespace semantic_consteval {

struct OffsetofFieldInfo
{
  bool found = false;
  bool is_bit_field = false;
  std::size_t offset = 0;
};

constant_eval::Hooks build_hooks(SemanticContext & ctx,
                                 semantic_model::Scope & scope);

semantic_model::Scope make_constexpr_call_scope(
    semantic_model::Scope & parent,
    semantic_model::FunctionBinding * binding,
    bool bind_parameters = true);

std::vector<std::pair<std::string, cpp_decl::TypePtr> >
constexpr_function_parameters(semantic_model::FunctionBinding & binding);

bool evaluate_expression_value(SemanticContext & ctx,
                               semantic_model::Scope & scope,
                               const CppAstNode & node,
                               constant_eval::ConstexprValue & out);
bool evaluate_expression_integral(SemanticContext & ctx,
                                  semantic_model::Scope & scope,
                                  const CppAstNode & node,
                                  long long & out);
bool reduce_fold_expression(SemanticContext & ctx,
                            semantic_model::Scope & scope,
                            const CppAstNode & node,
                            CppAstNode & out);
bool reduce_bound_fold_expressions(SemanticContext & ctx,
                                   semantic_model::Scope & scope,
                                   const CppAstNode & node,
                                   CppAstNode & out);
bool evaluate_initializer_value(SemanticContext & ctx,
                                semantic_model::Scope & scope,
                                const CppAstNode & initializer,
                                constant_eval::ConstexprValue & out);
bool evaluate_initializer_value(SemanticContext & ctx,
                                semantic_model::Scope & scope,
                                const CppAstNode & initializer,
                                const cpp_decl::TypePtr & target,
                                constant_eval::ConstexprValue & out);
bool evaluate_default_initialized_value(SemanticContext & ctx,
                                        semantic_model::Scope & scope,
                                        const cpp_decl::TypePtr & target,
                                        constant_eval::ConstexprValue & out);
bool evaluate_initializer_integral(SemanticContext & ctx,
                                   semantic_model::Scope & scope,
                                   const CppAstNode & initializer,
                                   long long & out);

}  // namespace semantic_consteval
