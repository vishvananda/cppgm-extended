#pragma once

#include <functional>

#include "cpp_decl_model.h"
#include "semantic_model.h"

namespace callsemantic {

struct TypeTraitCallbacks
{
  std::function<semantic_model::ClassInfo *(const cpp_decl::TypePtr &)>
      class_info_for_type;
};

bool has_user_declared_destructor(const semantic_model::ClassInfo & info);
bool has_nontrivial_copy_or_move_assignment(const semantic_model::ClassInfo & info);

bool is_scalar_or_member_pointer_type(const cpp_decl::TypePtr & type,
                                      const TypeTraitCallbacks & callbacks);
bool is_destructible_type(const cpp_decl::TypePtr & type,
                          const TypeTraitCallbacks & callbacks);
bool is_trivially_destructible_type(const cpp_decl::TypePtr & type,
                                    const TypeTraitCallbacks & callbacks);
bool is_trivially_copy_constructible_type(const cpp_decl::TypePtr & type,
                                          const TypeTraitCallbacks & callbacks);
bool is_trivially_move_constructible_type(const cpp_decl::TypePtr & type,
                                          const TypeTraitCallbacks & callbacks);
bool is_trivially_copy_assignable_type(const cpp_decl::TypePtr & type,
                                       const TypeTraitCallbacks & callbacks);

bool is_function_local_class_info(const semantic_model::ClassInfo & info);
bool type_contains_function_local_class(const cpp_decl::TypePtr & type,
                                        const TypeTraitCallbacks & callbacks);
bool is_trivial_special_member_binding(
    const semantic_model::FunctionBinding & binding,
    const TypeTraitCallbacks & callbacks);

bool is_empty_class_info(const semantic_model::ClassInfo * info,
                         const TypeTraitCallbacks & callbacks);

}  // namespace callsemantic
