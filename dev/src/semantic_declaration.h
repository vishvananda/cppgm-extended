#pragma once

#include "cppast_ast.h"

namespace semantic_model {
struct Scope;
enum MemberAccess : int;
}

class SemanticContext;

namespace semantic_declaration {

void collect_namespace_alias_definition(SemanticContext & ctx,
                                        semantic_model::Scope & scope,
                                        const CppAstNode & node);

void collect_using_directive(SemanticContext & ctx,
                             semantic_model::Scope & scope,
                             const CppAstNode & node);

void collect_using_declaration(SemanticContext & ctx,
                               semantic_model::Scope & scope,
                               const CppAstNode & node,
                               semantic_model::MemberAccess access =
                                   static_cast<semantic_model::MemberAccess>(0));

void analyze_static_assert_declaration(SemanticContext & ctx,
                                       semantic_model::Scope & scope,
                                       const CppAstNode & node);

bool prepare_alias_type_id(SemanticContext & ctx,
                           semantic_model::Scope & scope,
                           const CppAstNode & node,
                           bool block_scope,
                           CppAstNode & out);

void collect_namespace_definition(SemanticContext & ctx,
                                  semantic_model::Scope & scope,
                                  const CppAstNode & node);

void collect_declaration(SemanticContext & ctx,
                         semantic_model::Scope & scope,
                         const CppAstNode & node,
                         bool is_c_linkage = false,
                         bool linkage_has_braces = false);

void collect_top_level_declarations(SemanticContext & ctx,
                                    semantic_model::Scope & root,
                                    const CppAstNode & translation_unit);

}  // namespace semantic_declaration
