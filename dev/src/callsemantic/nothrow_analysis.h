#pragma once

#include <functional>
#include <set>
#include <string>

#include "callsemantic/type_trait_analysis.h"
#include "callsem_output.h"
#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_conversion.h"
#include "semantic_model.h"

namespace callsemantic {

struct NothrowCallbacks
{
  TypeTraitCallbacks type_traits() const;

  std::function<TypeTraitCallbacks()> make_type_trait_callbacks;
  std::function<semantic_model::ClassInfo *(const cpp_decl::TypePtr &)>
      complete_class_type;
  std::function<semantic_conversion::ExprInfo(
      semantic_model::Scope &,
      const CppAstNode &,
      const cpp_decl::TypePtr &)> analyze_expression_for_target;
  std::function<semantic_model::FunctionBinding *(
      semantic_model::Scope &,
      semantic_model::ClassInfo &)> select_default_constructor;
  std::function<bool(semantic_model::FunctionBinding &, bool &)>
      evaluate_explicit_function_nothrow_semantically;
  std::function<cpp_decl::TypePtr(semantic_model::Scope &,
                                  const std::string &)> lookup_type;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      destructor_for;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      copy_constructor_for;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      move_constructor_for;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      ensure_implicit_copy_constructor;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      ensure_implicit_move_constructor;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      copy_assignment_for;
  std::function<semantic_model::FunctionBinding *(semantic_model::ClassInfo &)>
      move_assignment_for;
  std::function<bool(const CallSemNode &, semantic_model::FunctionBinding *&)>
      resolve_dump_callee_binding;
  std::function<bool(semantic_model::Scope &, const CallSemNode &)>
      is_declval_dump_callee;
};

bool initializer_is_nothrow(const cpp_decl::TypePtr & target,
                            semantic_model::Scope & scope,
                            const CppAstNode & initializer,
                            std::set<semantic_model::FunctionBinding *> & visiting,
                            const NothrowCallbacks & callbacks);

bool default_initialization_is_nothrow(
    const cpp_decl::TypePtr & type,
    semantic_model::Scope & scope,
    std::set<semantic_model::FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks);

bool function_binding_is_nothrow(
    semantic_model::FunctionBinding & binding,
    std::set<semantic_model::FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks);

bool function_binding_is_nothrow(semantic_model::FunctionBinding & binding,
                                 const NothrowCallbacks & callbacks);

bool callsem_node_can_throw(
    semantic_model::Scope & scope,
    const CallSemNode & node,
    std::set<semantic_model::FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks);

}  // namespace callsemantic
