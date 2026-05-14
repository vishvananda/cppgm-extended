#pragma once

#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"
#include "template_service_interfaces.h"

namespace template_decl_ast {

bool expand_parameter_clause_pack_patterns(
    template_api::TemplateServices & services,
    semantic_model::Scope & semantic_scope,
    const CppAstNode & node,
    CppAstNode & expanded_clause,
    std::vector<const CppAstNode *> * default_args_out = nullptr);

bool parse_parameter_clause(
    template_api::TemplateServices & services,
    semantic_model::Scope & parse_scope,
    semantic_model::Scope & semantic_scope,
    const CppAstNode & node,
    std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
    std::vector<const CppAstNode *> * default_args_out = nullptr,
    bool * variadic_out = nullptr,
    bool reference_class_templates_only = false);

bool parse_declarator(template_api::TemplateServices & services,
                      semantic_model::Scope & parse_scope,
                      semantic_model::Scope & semantic_scope,
                      const CppAstNode & declarator,
                      const cpp_decl::TypePtr & base,
                      std::string & name,
                      cpp_decl::TypePtr & type,
                      bool reference_class_templates_only = false);

bool parse_type_specifier_seq(template_api::TemplateServices & services,
                              semantic_model::Scope & parse_scope,
                              semantic_model::Scope & semantic_scope,
                              const CppAstNode & specifiers,
                              cpp_decl::TypePtr & out,
                              bool reference_class_templates_only = false,
                              bool re_resolve_dependent_semantic_types = false);

bool parse_type_id(template_api::TemplateServices & services,
                   semantic_model::Scope & parse_scope,
                   semantic_model::Scope & semantic_scope,
                   const CppAstNode & type_id,
                   cpp_decl::TypePtr & out,
                   bool reference_class_templates_only = false);

bool parse_trailing_return_base(template_api::TemplateServices & services,
                                semantic_model::Scope & scope,
                                const CppAstNode & specifiers,
                                const CppAstNode & declarator,
                                bool & is_typedef,
                                cpp_decl::TypePtr & out,
                                bool reference_class_templates_only = false);

}  // namespace template_decl_ast
