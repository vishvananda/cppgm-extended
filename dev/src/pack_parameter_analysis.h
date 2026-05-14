#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"

namespace pack_parameter_analysis {

std::string parameter_declaration_name(const CppAstNode & parameter);

std::vector<std::pair<std::string, const std::vector<cpp_decl::TypePtr> *> >
referenced_named_type_packs(semantic_model::Scope & scope,
                            const CppAstNode & parameter);

bool infer_named_type_pack_size(semantic_model::Scope & scope,
                                const CppAstNode & parameter,
                                std::size_t & out_pack_size);

}  // namespace pack_parameter_analysis
