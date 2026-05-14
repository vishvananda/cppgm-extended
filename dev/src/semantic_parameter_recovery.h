#pragma once

#include <string>

#include "cppast_ast.h"

namespace semantic_parameter_recovery {

bool recover_parameter_clause_initializer(
    const CppAstNode & parameter_clause,
    CppAstNode & initializer,
    std::string & error);

bool recover_function_style_initializer_declarator(
    const CppAstNode & declarator,
    CppAstNode & stripped_declarator,
    CppAstNode & initializer,
    std::string & error);

}  // namespace semantic_parameter_recovery
