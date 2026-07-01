#include "callsemantic/type_trait_analysis.h"

#include <map>
#include <string>
#include <vector>

#include "semantic_conversion.h"
#include "semantic_lookup.h"

namespace callsemantic {

using cpp_decl::Type;
using cpp_decl::TypePtr;
using semantic_model::ClassInfo;
using semantic_model::FieldInfo;
using semantic_model::FunctionBinding;

namespace {

bool is_named_enum_type(const TypePtr & type, const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  if(base && base->kind == Type::TK_NAMED &&
     base->named_key.compare(0, 5, "enum ") == 0) {
    return true;
  }
  ClassInfo * info = callbacks.class_info_for_type(type);
  return info && info->class_kind == "enum";
}

bool is_scalar_or_member_pointer_type_impl(const TypePtr & type,
                                           const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  return base &&
         (base->kind == Type::TK_MEMBER_POINTER ||
          is_integral_type(base) ||
          is_floating_type(base) ||
          is_pointer_type(base) ||
          is_named_enum_type(base, callbacks) ||
          (base->kind == Type::TK_FUNDAMENTAL &&
           base->fundamental == FT_NULLPTR_T));
}

bool is_same_class_reference_parameter(const TypePtr & class_type,
                                       const TypePtr & param_type,
                                       Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(param_type);
  if(!base || base->kind != ref_kind) {
    return false;
  }
  return semantic_conversion::same_type_with_compatible_top_cv(base->inner,
                                                              class_type);
}

}  // namespace

bool is_scalar_or_member_pointer_type(const TypePtr & type,
                                      const TypeTraitCallbacks & callbacks)
{
  return is_scalar_or_member_pointer_type_impl(type, callbacks);
}

bool has_user_declared_destructor(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i]->is_destructor && !it->second[i]->synthesized) {
        return true;
      }
    }
  }
  return false;
}

bool has_nontrivial_copy_constructor(const ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::const_iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return false;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * binding = found->second[i];
    if(binding &&
       binding->is_constructor &&
       binding->params.size() == 2 &&
       is_same_class_reference_parameter(info.type,
                                         binding->params[1].second,
                                         Type::TK_LVALUE_REFERENCE) &&
       (binding->is_deleted ||
        (!binding->synthesized && !binding->is_defaulted))) {
      return true;
    }
  }
  return false;
}

bool has_nontrivial_copy_or_move_assignment(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(!binding ||
         binding->is_constructor ||
         binding->is_destructor ||
         binding->display_name != "operator=" ||
         binding->params.size() != 2) {
        continue;
      }
      const bool copy_like =
          is_same_class_reference_parameter(info.type,
                                            binding->params[1].second,
                                            Type::TK_LVALUE_REFERENCE);
      const bool move_like =
          is_same_class_reference_parameter(info.type,
                                            binding->params[1].second,
                                            Type::TK_RVALUE_REFERENCE);
      if((copy_like || move_like) &&
         !binding->synthesized &&
         !binding->is_defaulted) {
        return true;
      }
    }
  }
  return false;
}

bool is_destructible_type(const TypePtr & type,
                          const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_destructible_type(base->inner, callbacks);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type_impl(base, callbacks)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = callbacks.class_info_for_type(base);
  return info && info->complete;
}

bool is_trivially_destructible_type(const TypePtr & type,
                                    const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_destructible_type(base->inner, callbacks);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type_impl(base, callbacks)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = callbacks.class_info_for_type(base);
  if(!info || !info->complete || has_user_declared_destructor(*info)) {
    return false;
  }
  for(std::size_t i = 0; i < info->bases.size(); ++i) {
    if(!is_trivially_destructible_type(info->bases[i].type->type,
                                       callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_destructible_type(info->fields[i].type, callbacks)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_copy_constructible_type(const TypePtr & type,
                                          const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_constructible_type(base->inner, callbacks);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type_impl(base, callbacks)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = callbacks.class_info_for_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete || info->is_polymorphic ||
     has_user_declared_destructor(*info) ||
     has_nontrivial_copy_constructor(*info)) {
    return false;
  }
  for(std::size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_copy_constructible_type(info->bases[i].type->type,
                                             callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_copy_constructible_type(info->fields[i].type,
                                             callbacks)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_copy_assignable_type(const TypePtr & type,
                                       const TypeTraitCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(semantic_conversion::is_const_object_type(base)) {
    return false;
  }
  if(is_reference_type(base)) {
    return false;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_assignable_type(base->inner, callbacks);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type_impl(base, callbacks)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = callbacks.class_info_for_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete ||
     info->is_polymorphic ||
     has_user_declared_destructor(*info) ||
     has_nontrivial_copy_or_move_assignment(*info)) {
    return false;
  }
  for(std::size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_copy_assignable_type(info->bases[i].type->type,
                                          callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_copy_assignable_type(info->fields[i].type, callbacks)) {
      return false;
    }
  }
  return true;
}

bool is_function_local_class_info(const ClassInfo & info)
{
  return !info.is_lambda_closure &&
         info.class_kind != "union" &&
         info.enclosing_scope &&
         semantic_lookup::current_function_scope(*info.enclosing_scope) != nullptr;
}

bool is_local_special_member_elision_owner(const ClassInfo & info)
{
  return info.class_kind != "union" &&
         info.enclosing_scope &&
         semantic_lookup::current_function_scope(*info.enclosing_scope) != nullptr;
}

bool type_contains_function_local_class(const TypePtr & type,
                                        const TypeTraitCallbacks & callbacks)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_NAMED:
  {
    ClassInfo * info = callbacks.class_info_for_type(type);
    return info && is_function_local_class_info(*info);
  }

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_contains_function_local_class(type->inner, callbacks);

  case Type::TK_MEMBER_POINTER:
    return type_contains_function_local_class(type->owner, callbacks) ||
           type_contains_function_local_class(type->inner, callbacks);

  case Type::TK_FUNCTION:
    if(type_contains_function_local_class(type->inner, callbacks)) {
      return true;
    }
    for(std::size_t i = 0; i < type->params.size(); ++i) {
      if(type_contains_function_local_class(type->params[i], callbacks)) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

bool is_trivial_special_member_binding(
    const FunctionBinding & binding,
    const TypeTraitCallbacks & callbacks)
{
  if(!binding.owner_class) {
    return false;
  }
  const ClassInfo & info = *binding.owner_class;
  if(!is_local_special_member_elision_owner(info)) {
    return false;
  }
  if(binding.is_destructor) {
    return is_trivially_destructible_type(info.type, callbacks);
  }
  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  const bool generated_copy_constructor =
      binding.is_constructor &&
      binding.params.size() == 2 &&
      implicit_like &&
      is_same_class_reference_parameter(info.type,
                                        binding.params[1].second,
                                        Type::TK_LVALUE_REFERENCE);
  const bool generated_move_constructor =
      binding.is_constructor &&
      binding.params.size() == 2 &&
      implicit_like &&
      is_same_class_reference_parameter(info.type,
                                        binding.params[1].second,
                                        Type::TK_RVALUE_REFERENCE);
  if(generated_copy_constructor || generated_move_constructor) {
    return is_trivially_copy_constructible_type(info.type, callbacks);
  }
  return false;
}

bool is_empty_class_info(const ClassInfo * info,
                         const TypeTraitCallbacks & callbacks)
{
  if(!info || info->has_own_vptr) {
    return false;
  }
  for(std::size_t i = 0; i < info->bases.size(); ++i) {
    ClassInfo * base_info = info->bases[i].type;
    if(!base_info || !is_empty_class_info(base_info, callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info->fields.size(); ++i) {
    const FieldInfo & field = info->fields[i];
    if(field.is_bit_field || !field.is_no_unique_address) {
      return false;
    }
    ClassInfo * field_info = callbacks.class_info_for_type(field.type);
    if(!field_info || !is_empty_class_info(field_info, callbacks)) {
      return false;
    }
  }
  return true;
}

}  // namespace callsemantic
