#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"

namespace cpp_decl {

struct AstDeclHooks
{
  std::function<TypePtr(const std::string &)> lookup_type;
  std::function<TypePtr(const CppAstNode &)> lookup_type_node;
  std::function<bool(const TypePtr &)> ignore_semantic_type;
  std::function<bool(const CppAstNode &, TypePtr &)> parse_decltype_specifier;
  std::function<bool(const CppAstNode &, long long &)> evaluate_constant_expression;
  std::function<bool(const CppAstNode &)> array_bound_is_dependent;
  std::function<bool(const CppAstNode &,
                     CppAstNode &,
                     std::vector<const CppAstNode *> *)> expand_parameter_clause_packs;
  std::function<bool(const std::string &)> type_name_is_parameter_pack;
  bool normalize_function_parameters = false;
  bool allow_virtual_specifier = false;
};

bool decl_spec_contains_token(const CppAstNode & node, ETokenType token);
bool parse_type_specifier_seq_ast(const CppAstNode & node,
                                  const AstDeclHooks & hooks,
                                  TypePtr & out);
bool parse_decl_spec_ast(const CppAstNode & node,
                         const AstDeclHooks & hooks,
                         bool & is_typedef,
                         TypePtr & out);
bool parse_parameter_clause_ast(
    const CppAstNode & node,
    const AstDeclHooks & hooks,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> * default_args_out = nullptr,
    bool * variadic_out = nullptr);
bool parse_declarator_ast(const CppAstNode & node,
                          const AstDeclHooks & hooks,
                          const TypePtr & base,
                          std::string & name,
                          TypePtr & out);
bool parse_abstract_declarator_ast(const CppAstNode & node,
                                   const AstDeclHooks & hooks,
                                   const TypePtr & base,
                                   TypePtr & out);
bool parse_type_id_ast(const CppAstNode & node,
                       const AstDeclHooks & hooks,
                       TypePtr & out);

}  // namespace cpp_decl
