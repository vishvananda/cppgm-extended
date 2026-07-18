#include "semantic_builtins.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>

#include "builtin_type_transforms.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"
#include "cppast_parser.h"
#include "constructor_lifecycle_service.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_errors.h"
#include "semantic_lookup.h"
#include "semantic_template_function.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_api.h"
#include "types.h"

namespace semantic_builtins {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using semantic_lookup::is_named_enum_type;

namespace {

using semantic_utils::trim_space;

const char kDependentBuiltinTypeTransformPrefix[] = "$builtin-type-transform:";

bool has_invalid_top_level_qualified_owner_syntax(const std::string & text);

ExprInfo make_builtin_trait_expr_info(const TypePtr & source)
{
  ExprInfo expr;
  TypePtr source_base = strip_top_level_cv(source);
  if(!source_base) {
    return expr;
  }
  if(source_base->kind == Type::TK_LVALUE_REFERENCE) {
    expr.type = source_base->inner;
    expr.category = VC_LVALUE;
  } else if(source_base->kind == Type::TK_RVALUE_REFERENCE) {
    expr.type = source_base->inner;
    expr.category = VC_XVALUE;
  } else if(source_base->kind == Type::TK_FUNCTION) {
    expr.type = source;
    expr.category = VC_LVALUE;
  } else {
    expr.type = source;
    TypePtr object_base = strip_top_level_cv(remove_reference_type(source));
    expr.category = (object_base && object_base->kind == Type::TK_ARRAY) ?
        VC_XVALUE :
        VC_PRVALUE;
  }
  return expr;
}

TypePtr make_dependent_builtin_type_transform_type_impl(
    const std::string & builtin_name,
    const std::string & arg_text,
    const TypePtr & arg_type)
{
  const std::string display =
      builtin_name + "(" + (arg_text.empty() ?
          template_argument_type_text(arg_type) :
          arg_text) + ")";
  TypePtr result =
      make_semantic_named(display,
                          Type::NSK_DEPENDENT_TYPE,
                          std::string(kDependentBuiltinTypeTransformPrefix) +
                              builtin_name + "|" +
                              template_argument_type_text(arg_type),
                          true);
  TypePtr base = strip_top_level_cv(result);
  if(base && base->kind == Type::TK_NAMED) {
    base->inner = arg_type;
  }
  return result;
}

TypePtr apply_decay_type_transform(const TypePtr & arg_type)
{
  TypePtr decayed = remove_reference_type(arg_type);
  TypePtr decayed_base = strip_top_level_cv(decayed);
  if(!decayed_base) {
    return TypePtr();
  }
  if(decayed_base->kind == Type::TK_ARRAY) {
    return make_pointer(decayed_base->inner);
  }
  if(decayed_base->kind == Type::TK_FUNCTION) {
    return make_pointer(decayed_base);
  }
  return decayed_base;
}

bool class_info_has_virtual_destructor(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      const FunctionBinding * binding = it->second[i];
      if(binding && binding->is_destructor && binding->has_virtual_slot) {
        return true;
      }
    }
  }
  return false;
}

bool class_info_is_abstract(const ClassInfo & info)
{
  for(size_t i = 0; i < info.vtable_entries.size(); ++i) {
    const FunctionBinding * binding = info.vtable_entries[i];
    if(binding && binding->is_pure_virtual) {
      return true;
    }
  }
  return false;
}

bool has_invalid_top_level_qualified_owner_syntax(const std::string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(ch == '[') {
      ++bracket_depth;
      continue;
    }
    if(ch == ']' && bracket_depth > 0) {
      --bracket_depth;
      continue;
    }
    if(ch == '{') {
      ++brace_depth;
      continue;
    }
    if(ch == '}' && brace_depth > 0) {
      --brace_depth;
      continue;
    }
    if(angle_depth != 0 || paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(ch == '*' || ch == '&') {
      std::size_t next = i + 1;
      while(next < text.size() &&
            std::isspace(static_cast<unsigned char>(text[next]))) {
        ++next;
      }
      if(next + 1 < text.size() &&
         text[next] == ':' &&
         text[next + 1] == ':') {
        return true;
      }
    }
  }
  return false;
}

TypePtr exact_gnu_complex_type(EFundamentalType component)
{
  switch(component) {
  case FT_FLOAT:
    return make_named("_Complex float", "builtin _Complex float", true, true, 4, 8);
  case FT_DOUBLE:
    return make_named("_Complex double", "builtin _Complex double", true, true, 8, 16);
  case FT_LONG_DOUBLE:
    return make_named("_Complex long double", "builtin _Complex long double", true, true, 16, 32);
  default:
    return TypePtr();
  }
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

bool scope_has_template_placeholders(SemanticContext & ctx, Scope & scope)
{
  return ctx.scope_has_template_placeholders(scope);
}

const semantic_model::FieldInfo * first_aggregate_field(const semantic_model::ClassInfo & info)
{
  return info.fields.empty() ? nullptr : &info.fields[0];
}

const semantic_model::FieldInfo * aggregate_input_field(SemanticContext & ctx,
                                                        const semantic_model::FieldInfo & field)
{
  if(!field.is_anonymous_storage) {
    return &field;
  }
  semantic_model::ClassInfo * storage_info = ctx.class_info_for_type(field.type);
  return storage_info ? first_aggregate_field(*storage_info) : &field;
}

bool analyze_expression(SemanticContext & ctx,
                        Scope & scope,
                        const CppAstNode & expr,
                        ExprInfo & out)
{
  try
  {
    out = ctx.analyze_expression(scope, expr);
    return true;
  }
  catch(const std::logic_error & e)
  {
    const char * debug = std::getenv("CPPGM_DEBUG_NOTHROW_CATCH");
    if(debug && debug[0] != '\0') {
      std::cerr << "NOTHROW_CATCH expr=" << node_text(expr)
                << " error=" << e.what() << "\n";
    }
    return false;
  }
}

bool analyze_expression_for_target(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & expr,
                                   const TypePtr & target,
                                   ExprInfo & out)
{
  try
  {
    out = ctx.analyze_expression_for_target(scope, expr, target);
    return true;
  }
  catch(const std::logic_error &)
  {
    return false;
  }
}

bool try_argument_conversion(SemanticContext & ctx,
                             Scope & scope,
                             const TypePtr & target,
                             const ExprInfo & arg,
                             ExprInfo & out)
{
  const auto prepare_class_type =
      [&](const TypePtr & type)
      {
        TypePtr base = strip_top_level_cv(remove_reference_type(type));
        if(!base) {
          return;
        }
        ClassInfo * info = ctx.complete_class_type(base);
        if(info) {
          ctx.ensure_class_reference_members(*info);
        }
      };
  prepare_class_type(target);
  prepare_class_type(arg.type);
  ConversionRank rank = CR_BAD;
  try
  {
    return ctx.try_argument_conversion(
        scope,
        target,
        arg,
        out,
        rank,
        semantic_policy::default_argument_conversion());
  }
  catch(const std::logic_error &)
  {
    return false;
  }
}

bool try_direct_class_construction_for_trait(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & target,
    const ExprInfo & arg,
    constructor_lifecycle_service::ConstructorSelectionResult & selection)
{
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base || is_reference_type(target_base) || target_base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(target_base);
  if(!info || !info->complete) {
    return false;
  }

  std::vector<ExprInfo> args;
  args.push_back(arg);
  ConstructorSelectionOptions options =
      constructor_lifecycle_service::selection_options_for(
          constructor_lifecycle_service::direct_initialization_profile(
              "__is_constructible"));
  options.instantiate_bodies = false;
  try
  {
    constructor_lifecycle_service::select_constructor_from_exprs_into(
        ctx, scope, *info, args, selection, options);
    return selection.ctor != nullptr;
  }
  catch(const std::logic_error &)
  {
    selection = constructor_lifecycle_service::ConstructorSelectionResult();
    return true;
  }
}

bool try_direct_class_construction_for_trait(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & target,
    const std::vector<ExprInfo> & args,
    constructor_lifecycle_service::ConstructorSelectionResult & selection)
{
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base || is_reference_type(target_base) || target_base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(target_base);
  if(!info || !info->complete) {
    return false;
  }

  ConstructorSelectionOptions options =
      constructor_lifecycle_service::selection_options_for(
          constructor_lifecycle_service::direct_initialization_profile(
              "__is_constructible"));
  options.instantiate_bodies = false;
  try
  {
    constructor_lifecycle_service::select_constructor_from_exprs_into(
        ctx, scope, *info, args, selection, options);
    return true;
  }
  catch(const std::logic_error &)
  {
    selection = constructor_lifecycle_service::ConstructorSelectionResult();
    return true;
  }
}

TypePtr map_signedness_builtin_transform(const std::string & builtin_name,
                                         const TypePtr & arg_type)
{
  TypePtr base = strip_top_level_cv(arg_type);
  if(!base) {
    return TypePtr();
  }

  if(base->kind != Type::TK_FUNDAMENTAL) {
    return TypePtr();
  }

  const auto unsigned_integral_type_for_size =
      [](std::size_t size) -> EFundamentalType
      {
        switch(size) {
        case 1: return FT_UNSIGNED_CHAR;
        case 2: return FT_UNSIGNED_SHORT_INT;
        case 4: return FT_UNSIGNED_INT;
        case 8: return FT_UNSIGNED_LONG_LONG_INT;
        case 16: return FT_UINT128;
        default: return FT_VOID;
        }
      };
  const auto signed_integral_type_for_size =
      [](std::size_t size) -> EFundamentalType
      {
        switch(size) {
        case 1: return FT_SIGNED_CHAR;
        case 2: return FT_SHORT_INT;
        case 4: return FT_INT;
        case 8: return FT_LONG_LONG_INT;
        case 16: return FT_INT128;
        default: return FT_VOID;
        }
      };

  EFundamentalType mapped = base->fundamental;
  if(builtin_name == "__make_unsigned") {
    switch(base->fundamental) {
    case FT_CHAR:
      mapped = FT_UNSIGNED_CHAR;
      break;
    case FT_WCHAR_T:
    case FT_CHAR16_T:
    case FT_CHAR32_T:
      mapped = unsigned_integral_type_for_size(type_to_size(base->fundamental));
      if(mapped == FT_VOID) {
        return TypePtr();
      }
      break;
    case FT_SIGNED_CHAR: mapped = FT_UNSIGNED_CHAR; break;
    case FT_SHORT_INT: mapped = FT_UNSIGNED_SHORT_INT; break;
    case FT_INT: mapped = FT_UNSIGNED_INT; break;
    case FT_LONG_INT: mapped = FT_UNSIGNED_LONG_INT; break;
    case FT_LONG_LONG_INT: mapped = FT_UNSIGNED_LONG_LONG_INT; break;
    case FT_INT128: mapped = FT_UINT128; break;
    case FT_UNSIGNED_CHAR:
    case FT_UNSIGNED_SHORT_INT:
    case FT_UNSIGNED_INT:
    case FT_UNSIGNED_LONG_INT:
    case FT_UNSIGNED_LONG_LONG_INT:
    case FT_UINT128:
      mapped = base->fundamental;
      break;
    default:
      return TypePtr();
    }
  } else if(builtin_name == "__make_signed") {
    switch(base->fundamental) {
    case FT_CHAR:
      mapped = FT_SIGNED_CHAR;
      break;
    case FT_WCHAR_T:
    case FT_CHAR16_T:
    case FT_CHAR32_T:
      mapped = signed_integral_type_for_size(type_to_size(base->fundamental));
      if(mapped == FT_VOID) {
        return TypePtr();
      }
      break;
    case FT_UNSIGNED_CHAR: mapped = FT_SIGNED_CHAR; break;
    case FT_UNSIGNED_SHORT_INT: mapped = FT_SHORT_INT; break;
    case FT_UNSIGNED_INT: mapped = FT_INT; break;
    case FT_UNSIGNED_LONG_INT: mapped = FT_LONG_INT; break;
    case FT_UNSIGNED_LONG_LONG_INT: mapped = FT_LONG_LONG_INT; break;
    case FT_UINT128: mapped = FT_INT128; break;
    case FT_SIGNED_CHAR:
    case FT_SHORT_INT:
    case FT_INT:
    case FT_LONG_INT:
    case FT_LONG_LONG_INT:
    case FT_INT128:
      mapped = base->fundamental;
      break;
    default:
      return TypePtr();
    }
  } else {
    return TypePtr();
  }

  TypePtr result = make_fundamental(mapped);
  if(arg_type->kind == Type::TK_CV) {
    result = make_cv(result, arg_type->cv_const, arg_type->cv_volatile);
  }
  return result;
}

bool has_user_declared_destructor(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
      if(it->second[i]->is_destructor && !it->second[i]->synthesized) {
        return true;
      }
    }
  }
  return false;
}

bool is_same_class_reference_parameter(const TypePtr & class_type,
                                       const TypePtr & param_type,
                                       Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(param_type);
  if(!base || base->kind != ref_kind) {
    return false;
  }
  return same_type_with_compatible_top_cv(base->inner, class_type);
}

bool has_nontrivial_copy_or_move_assignment(const ClassInfo & info)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(size_t i = 0; i < it->second.size(); ++i) {
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

bool is_trivially_copy_assignable_type(SemanticContext & ctx, const TypePtr & type)
{
  if(is_const_object_type(type)) {
    return false;
  }
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return false;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_assignable_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.class_info_for_type(base);
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
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_copy_assignable_type(ctx, info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_copy_assignable_type(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

bool is_named_union_type(SemanticContext & ctx, const TypePtr & type)
{
  ClassInfo * info = ctx.class_info_for_type(type);
  return info && info->class_kind == "union";
}

bool is_named_class_type(SemanticContext & ctx, const TypePtr & type)
{
  ClassInfo * info = ctx.class_info_for_type(type);
  return info && info->class_kind != "union" && info->class_kind != "enum";
}

bool is_scalar_or_member_pointer_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base &&
         (base->kind == Type::TK_MEMBER_POINTER ||
          is_integral_type(base) ||
          is_floating_type(base) ||
          is_pointer_type(base) ||
          is_named_enum_type(ctx, base) ||
          (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T));
}

bool class_has_nonstatic_data_members(const ClassInfo & info)
{
  if(!info.fields.empty()) {
    return true;
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type && class_has_nonstatic_data_members(*info.bases[i].type)) {
      return true;
    }
  }
  return false;
}

bool is_standard_layout_type_impl(SemanticContext & ctx,
                                  const TypePtr & type,
                                  std::set<std::string> & visiting)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return base->inner && is_standard_layout_type_impl(ctx, base->inner, visiting);
  }
  if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info) {
    return false;
  }
  if(info->class_kind == "enum") {
    return true;
  }
  if(!info->complete ||
     info->is_polymorphic ||
     info->has_own_vptr) {
    return false;
  }
  if(!visiting.insert(info->qualified_name).second) {
    return false;
  }

  bool have_field_access = false;
  MemberAccess field_access = MA_PUBLIC;
  for(size_t i = 0; i < info->fields.size(); ++i) {
    const FieldInfo & field = info->fields[i];
    if(!have_field_access) {
      have_field_access = true;
      field_access = field.access;
    } else if(field.access != field_access) {
      visiting.erase(info->qualified_name);
      return false;
    }
    if(!is_standard_layout_type_impl(ctx, field.type, visiting)) {
      visiting.erase(info->qualified_name);
      return false;
    }
  }

  size_t bases_with_data = 0;
  for(size_t i = 0; i < info->bases.size(); ++i) {
    const BaseInfo & base_info = info->bases[i];
    if(base_info.is_virtual ||
       !base_info.type ||
       !is_standard_layout_type_impl(ctx, base_info.type->type, visiting)) {
      visiting.erase(info->qualified_name);
      return false;
    }
    if(class_has_nonstatic_data_members(*base_info.type)) {
      ++bases_with_data;
    }
  }
  if((!info->fields.empty() && bases_with_data != 0) || bases_with_data > 1) {
    visiting.erase(info->qualified_name);
    return false;
  }

  visiting.erase(info->qualified_name);
  return true;
}

bool is_standard_layout_type(SemanticContext & ctx, const TypePtr & type)
{
  std::set<std::string> visiting;
  return is_standard_layout_type_impl(ctx, type, visiting);
}

bool is_destructible_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_destructible_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  return info && info->complete;
}

bool is_trivially_destructible_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_destructible_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete || has_user_declared_destructor(*info)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!is_trivially_destructible_type(ctx, info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_trivially_destructible_type(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

bool is_trivially_copyable_type_impl(SemanticContext & ctx,
                                     const TypePtr & type,
                                     std::set<ClassInfo *> & visiting)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || is_reference_type(base) || base->kind == Type::TK_FUNCTION ||
     is_void_type(base)) {
    return false;
  }
  if(is_array_type(base)) {
    return is_trivially_copyable_type_impl(ctx, base->inner, visiting);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete || info->is_polymorphic ||
     !semantic_class_model::is_trivially_destructible_type_for_host_abi(ctx,
                                                                        base)) {
    return false;
  }
  if(visiting.count(info) != 0) {
    return true;
  }

  semantic_class_model::ensure_implicit_special_members(ctx, *info);
  FunctionBinding * special_members[] = {
    semantic_class_model::ensure_implicit_copy_constructor(ctx, *info),
    semantic_class_model::ensure_implicit_move_constructor(ctx, *info),
    semantic_class_model::ensure_implicit_copy_assignment(ctx, *info),
    semantic_class_model::ensure_implicit_move_assignment(ctx, *info)
  };
  bool has_eligible_copy_or_move = false;
  for(size_t i = 0;
      i < sizeof(special_members) / sizeof(special_members[0]);
      ++i) {
    const FunctionBinding * binding = special_members[i];
    if(!binding || binding->is_deleted) {
      continue;
    }
    has_eligible_copy_or_move = true;
    if(!binding->synthesized && !binding->is_defaulted) {
      return false;
    }
  }
  if(!has_eligible_copy_or_move) {
    return false;
  }

  visiting.insert(info);
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(info->bases[i].is_virtual ||
       !is_trivially_copyable_type_impl(ctx,
                                        info->bases[i].type->type,
                                        visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    TypePtr field_type = strip_top_level_cv(info->fields[i].type);
    if(field_type && is_reference_type(field_type)) {
      continue;
    }
    if(!is_trivially_copyable_type_impl(ctx,
                                        info->fields[i].type,
                                        visiting)) {
      visiting.erase(info);
      return false;
    }
  }
  visiting.erase(info);
  return true;
}

bool is_trivially_copyable_type(SemanticContext & ctx, const TypePtr & type)
{
  std::set<ClassInfo *> visiting;
  return is_trivially_copyable_type_impl(ctx, type, visiting);
}

bool is_literal_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return base->inner && is_literal_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete || class_info_is_abstract(*info) ||
     !is_trivially_destructible_type(ctx, base)) {
    return false;
  }
  for(size_t i = 0; i < info->bases.size(); ++i) {
    if(!is_literal_type(ctx, info->bases[i].type->type)) {
      return false;
    }
  }
  for(size_t i = 0; i < info->fields.size(); ++i) {
    if(!is_literal_type(ctx, info->fields[i].type)) {
      return false;
    }
  }
  return true;
}

long long array_rank(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  long long rank = 0;
  while(base && base->kind == Type::TK_ARRAY) {
    ++rank;
    base = strip_top_level_cv(base->inner);
  }
  return rank;
}

bool initializer_is_nothrow(SemanticContext & ctx,
                            Scope & scope,
                            const TypePtr & target,
                            const CppAstNode & initializer,
                            std::set<FunctionBinding *> & visiting);
bool default_initialization_is_nothrow(SemanticContext & ctx,
                                       Scope & scope,
                                       const TypePtr & type,
                                       std::set<FunctionBinding *> & visiting);
bool function_binding_is_nothrow(SemanticContext & ctx,
                                 Scope & scope,
                                 const FunctionBinding & binding,
                                 std::set<FunctionBinding *> & visiting);

FunctionBinding * find_constructor_for_trait(ClassInfo & info, bool want_move)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding &&
       ((want_move && binding->is_move_constructor) ||
        (!want_move && binding->is_copy_constructor))) {
      return binding;
    }
  }
  return nullptr;
}

bool copy_or_move_construction_type_is_nothrow(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & type,
    bool move,
    std::set<FunctionBinding *> & visiting)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(base->kind == Type::TK_ARRAY) {
    return base->has_bound &&
           copy_or_move_construction_type_is_nothrow(ctx,
                                                     scope,
                                                     base->inner,
                                                     move,
                                                     visiting);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(!move &&
     semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(
         ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }
  semantic_class_model::ensure_implicit_special_members(ctx, *info);

  FunctionBinding * ctor = nullptr;
  if(move && !type_is_const_object(type)) {
    ctor = find_constructor_for_trait(*info, true);
    if(!ctor) {
      ctor = ctx.ensure_implicit_move_constructor(*info);
    }
  }
  if(!ctor || ctor->is_deleted) {
    ctor = find_constructor_for_trait(*info, false);
    if(!ctor) {
      ctor = ctx.ensure_implicit_copy_constructor(*info);
    }
  }
  return ctor && !ctor->is_deleted &&
         function_binding_is_nothrow(ctx, scope, *ctor, visiting);
}

bool constructor_binding_is_implicitly_nothrow(SemanticContext & ctx,
                                               Scope & scope,
                                               const FunctionBinding & binding,
                                               std::set<FunctionBinding *> & visiting)
{
  if(!binding.owner_class || !binding.declaration_scope || !binding.is_constructor) {
    return false;
  }
  if(!binding.is_defaulted && !binding.synthesized && !binding.is_aggregate_constructor) {
    return false;
  }

  const ClassInfo & info = *binding.owner_class;
  std::set<std::string> explicitly_initialized;
  const bool copy_or_move_ctor =
      binding.is_copy_constructor || binding.is_move_constructor;
  const bool move_ctor = binding.is_move_constructor;

  if(binding.ctor_initializer) {
    for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
      const CppAstNode & mem_init = binding.ctor_initializer->children[i];
      const CppAstNode * id = find_child_kind(mem_init, CppAstKind::mem_initializer_id);
      if(!id) {
        return false;
      }
      explicitly_initialized.insert(id->value);

      TypePtr target_type;
      bool matched = false;
      for(size_t j = 0; j < info.fields.size(); ++j) {
        if(info.fields[j].name == id->value) {
          target_type = info.fields[j].type;
          matched = true;
          break;
        }
      }
      if(!matched) {
        for(size_t j = 0; j < info.bases.size(); ++j) {
          if(info.bases[j].type &&
             (info.bases[j].type->qualified_name == id->value ||
              info.bases[j].type->name == id->value)) {
            target_type = info.bases[j].type->type;
            matched = true;
            break;
          }
        }
      }
      if(!matched) {
        target_type = ctx.lookup_type_node(scope, *id, id->value);
        matched = static_cast<bool>(target_type);
      }
      if(!matched) {
        return false;
      }

      const CppAstNode * payload = mem_init.children.empty() ? nullptr : &mem_init.children.back();
      if(!payload || !initializer_is_nothrow(ctx, scope, target_type, *payload, visiting)) {
        return false;
      }
    }
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type) {
      return false;
    }
    const std::string & base_name = info.bases[i].type->name;
    const std::string & qualified_name = info.bases[i].type->qualified_name;
    if(explicitly_initialized.count(base_name) != 0 ||
       explicitly_initialized.count(qualified_name) != 0) {
      continue;
    }
    const bool base_nothrow = copy_or_move_ctor ?
        copy_or_move_construction_type_is_nothrow(ctx,
                                                  scope,
                                                  info.bases[i].type->type,
                                                  move_ctor,
                                                  visiting) :
        default_initialization_is_nothrow(ctx,
                                          scope,
                                          info.bases[i].type->type,
                                          visiting);
    if(!base_nothrow) {
      return false;
    }
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    if(explicitly_initialized.count(info.fields[i].name) != 0) {
      continue;
    }
    if(copy_or_move_ctor) {
      if(!copy_or_move_construction_type_is_nothrow(ctx,
                                                    scope,
                                                    info.fields[i].type,
                                                    move_ctor,
                                                    visiting)) {
        return false;
      }
    } else if(info.fields[i].default_initializer) {
          if(!initializer_is_nothrow(ctx,
                                     scope,
                                     info.fields[i].type,
                                     *info.fields[i].default_initializer,
                                     visiting)) {
            return false;
          }
    } else {
      if(!default_initialization_is_nothrow(ctx,
                                            scope,
                                            info.fields[i].type,
                                            visiting)) {
        return false;
      }
    }
  }

  return true;
}

bool assignment_binding_is_implicitly_nothrow(
    SemanticContext & ctx,
    Scope & scope,
    const FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting);

bool function_binding_is_nothrow(SemanticContext & ctx,
                                 Scope & scope,
                                 const FunctionBinding & binding,
                                 std::set<FunctionBinding *> & visiting)
{
  bool explicit_value = false;
  if(ctx.evaluate_explicit_function_nothrow_semantically(
         const_cast<FunctionBinding &>(binding), explicit_value)) {
    return explicit_value;
  }

  FunctionBinding * mutable_binding = const_cast<FunctionBinding *>(&binding);
  if(!visiting.insert(mutable_binding).second) {
    return false;
  }

  const bool result = binding.is_constructor ?
      constructor_binding_is_implicitly_nothrow(ctx, scope, binding, visiting) :
      (binding.is_copy_assignment || binding.is_move_assignment) ?
          assignment_binding_is_implicitly_nothrow(ctx, scope, binding, visiting) :
      false;
  visiting.erase(mutable_binding);
  return result;
}

FunctionBinding * find_assignment_operator_for_trait(ClassInfo & info, bool want_move)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find("operator=");
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding &&
       ((want_move && binding->is_move_assignment) ||
        (!want_move && binding->is_copy_assignment))) {
      return binding;
    }
  }
  return nullptr;
}

bool assignment_binding_accepts_rhs(SemanticContext & ctx,
                                    Scope & scope,
                                    const FunctionBinding & binding,
                                    const ExprInfo & rhs)
{
  TypePtr function_type = strip_top_level_cv(binding.type);
  if(!function_type ||
     function_type->kind != Type::TK_FUNCTION ||
     function_type->params.size() != 2) {
    return false;
  }

  ExprInfo converted;
  return try_argument_conversion(ctx, scope, function_type->params[1], rhs, converted);
}

bool assignment_type_is_nothrow(SemanticContext & ctx,
                                Scope & scope,
                                const TypePtr & type,
                                bool move,
                                std::set<FunctionBinding *> & visiting)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return assignment_type_is_nothrow(ctx, scope, base->inner, move, visiting);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  if(is_trivially_copy_assignable_type(ctx, base)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }
  semantic_class_model::ensure_implicit_special_members(ctx, *info);
  ctx.ensure_implicit_copy_assignment(*info);
  if(move) {
    ctx.ensure_implicit_move_assignment(*info);
  }

  FunctionBinding * op = find_assignment_operator_for_trait(*info, move);
  if(!op && move) {
    op = find_assignment_operator_for_trait(*info, false);
  }
  return op && function_binding_is_nothrow(ctx, scope, *op, visiting);
}

bool assignment_binding_is_implicitly_nothrow(
    SemanticContext & ctx,
    Scope & scope,
    const FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting)
{
  if(!binding.owner_class ||
     (!binding.is_copy_assignment && !binding.is_move_assignment)) {
    return false;
  }
  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  if(!implicit_like) {
    return false;
  }

  ClassInfo & info = *binding.owner_class;
  if(info.class_kind == "union") {
    return false;
  }

  const bool move = binding.is_move_assignment;
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual ||
       !info.bases[i].type ||
       !assignment_type_is_nothrow(ctx,
                                   scope,
                                   info.bases[i].type->type,
                                   move,
                                   visiting)) {
      return false;
    }
  }
  for(size_t i = 0; i < info.fields.size(); ++i) {
    TypePtr field_type = strip_top_level_cv(info.fields[i].type);
    if(info.fields[i].is_bit_field ||
       is_reference_type(field_type)) {
      continue;
    }
    if(!assignment_type_is_nothrow(ctx,
                                   scope,
                                   info.fields[i].type,
                                   move,
                                   visiting)) {
      return false;
    }
  }
  return true;
}

FunctionBinding * find_class_assignment_operator_for_trait(SemanticContext & ctx,
                                                           Scope & scope,
                                                           const TypePtr & target,
                                                           const ExprInfo & rhs)
{
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  ClassInfo * info = ctx.complete_class_type(target_base);
  if(!info || !info->complete) {
    return nullptr;
  }

  semantic_class_model::ensure_implicit_special_members(ctx, *info);
  ctx.ensure_implicit_copy_assignment(*info);
  if(rhs.category != VC_LVALUE) {
    ctx.ensure_implicit_move_assignment(*info);
  }

  const std::string operator_name = "operator=";
  semantic_lookup::MemberFunctionLookupResult candidates =
      semantic_lookup::lookup_visible_member_functions(*info, operator_name);
  semantic_lookup::MemberFunctionTemplateLookupResult template_candidates =
      semantic_lookup::lookup_visible_member_function_templates(*info, operator_name);

  for(size_t i = 0; i < candidates.functions.size(); ++i) {
    FunctionBinding * binding = candidates.functions[i];
    if(!binding || binding->is_deleted) {
      continue;
    }
    try
    {
      if(assignment_binding_accepts_rhs(ctx, scope, *binding, rhs)) {
        return binding;
      }
    }
    catch(const std::logic_error &)
    {
    }
  }

  if(template_candidates.templates.empty()) {
    return nullptr;
  }

  const bool has_exact_template_owner =
      std::any_of(template_candidates.templates.begin(),
                  template_candidates.templates.end(),
                  [info](FunctionTemplateDecl * decl)
                  {
                    return decl && decl->declaring_scope &&
                           decl->declaring_scope->class_info == info;
                  });
  std::vector<ExprInfo> args(1, rhs);
  for(size_t i = 0; i < template_candidates.templates.size(); ++i) {
    FunctionTemplateDecl * decl = template_candidates.templates[i];
    if(!decl) {
      continue;
    }
    if(has_exact_template_owner &&
       decl->declaring_scope &&
       decl->declaring_scope->class_info &&
       decl->declaring_scope->class_info != info) {
      continue;
    }

    Scope template_use_scope(&scope, "", false);
    template_use_scope.class_info = info;
    template_use_scope.function = scope.function;
    if(info->member_scope) {
      semantic_template_function::overlay_instantiation_use_scope_bindings(
          template_use_scope,
          *info->member_scope,
          decl->declaring_scope);
    }

    semantic_template_function::FunctionTemplateDeduction deduction;
    if(!semantic_template_function::deduce_function_template_from_arguments(
           ctx, *decl, args, &template_use_scope, deduction)) {
      continue;
    }

    FunctionBinding * binding = nullptr;
    try
    {
      binding = semantic_template_function::acquire_function_template_binding(
          ctx,
          *decl,
          deduction.arguments,
          &template_use_scope,
          &deduction.pack_sizes,
          false);
    }
    catch(const TemplateSubstitutionFailure &)
    {
      continue;
    }
    catch(const std::logic_error &)
    {
      continue;
    }
    if(!binding || binding->is_deleted) {
      continue;
    }
    try
    {
      if(assignment_binding_accepts_rhs(ctx, scope, *binding, rhs)) {
        return binding;
      }
    }
    catch(const std::logic_error &)
    {
    }
  }
  return nullptr;
}

struct SameClassAssignmentTrait
{
  bool known = false;
  bool assignable = false;
  FunctionBinding * binding = nullptr;
};

SameClassAssignmentTrait evaluate_same_class_assignment_trait(SemanticContext & ctx,
                                                              Scope & scope,
                                                              const TypePtr & target,
                                                              const ExprInfo & rhs)
{
  SameClassAssignmentTrait result;
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base) {
    return result;
  }

  ClassInfo * info = ctx.complete_class_type(target_base);
  if(!info || !info->complete) {
    return result;
  }

  TypePtr rhs_object = remove_reference_type(rhs.type);
  TypePtr rhs_base = strip_top_level_cv(rhs_object);
  if(!rhs_base || !type_equals(target_base, rhs_base)) {
    return result;
  }

  result.known = true;
  semantic_class_model::ensure_implicit_special_members(ctx, *info);
  ctx.ensure_implicit_copy_assignment(*info);

  if(rhs.category != VC_LVALUE) {
    ctx.ensure_implicit_move_assignment(*info);
    FunctionBinding * move = find_assignment_operator_for_trait(*info, true);
    if(move && assignment_binding_accepts_rhs(ctx, scope, *move, rhs)) {
      result.binding = move;
      result.assignable = !move->is_deleted;
      return result;
    }
  }

  FunctionBinding * copy = find_assignment_operator_for_trait(*info, false);
  if(!copy) {
    copy = ctx.ensure_implicit_copy_assignment(*info);
  }
  if(copy && assignment_binding_accepts_rhs(ctx, scope, *copy, rhs)) {
    if(copy->is_deleted) {
      if(FunctionBinding * alternate =
             find_class_assignment_operator_for_trait(ctx, scope, target, rhs)) {
        result.binding = alternate;
        result.assignable = !alternate->is_deleted;
        return result;
      }
    }
    result.binding = copy;
    result.assignable = !copy->is_deleted;
  }
  return result;
}

bool default_initialization_is_nothrow(SemanticContext & ctx,
                                       Scope & scope,
                                       const TypePtr & type,
                                       std::set<FunctionBinding *> & visiting)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return default_initialization_is_nothrow(ctx, scope, base->inner, visiting);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(ctx, base)) {
    return true;
  }
  ClassInfo * info = ctx.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }

  FunctionBinding * ctor = ctx.select_default_constructor_for_builtin_trait(scope, *info);
  if(!ctor) {
    return false;
  }
  return function_binding_is_nothrow(ctx, scope, *ctor, visiting);
}

bool initializer_is_nothrow(SemanticContext & ctx,
                            Scope & scope,
                            const TypePtr & target,
                            const CppAstNode & initializer,
                            std::set<FunctionBinding *> & visiting)
{
  const CppAstNode * payload = &initializer;
  if(initializer.kind == CppAstKind::initializer && initializer.children.size() == 1) {
    payload = &initializer.children[0];
  }

  if(payload->kind == CppAstKind::braced_init_list && payload->children.empty()) {
    return default_initialization_is_nothrow(ctx, scope, target, visiting);
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(payload->kind == CppAstKind::braced_init_list && target_base) {
    if(target_base->kind == Type::TK_ARRAY) {
      for(size_t i = 0; i < payload->children.size(); ++i) {
        if(!initializer_is_nothrow(
                ctx, scope, target_base->inner, payload->children[i], visiting)) {
          return false;
        }
      }
      return true;
    }

    ClassInfo * aggregate_info =
        ctx.class_info_for_type(target_base);
    if(aggregate_info && ctx.can_synthesize_aggregate_constructor(*aggregate_info)) {
      const std::size_t aggregate_count =
          semantic_class_model::aggregate_element_count(*aggregate_info);
      if(payload->children.size() > aggregate_count) {
        return false;
      }
      for(size_t i = 0; i < aggregate_info->fields.size(); ++i) {
        const FieldInfo * input_field =
            aggregate_input_field(ctx, aggregate_info->fields[i]);
        if(i >= payload->children.size() || i >= aggregate_count) {
          break;
        }
        if(!initializer_is_nothrow(ctx,
                                   scope,
                                   input_field->type,
                                   payload->children[i],
                                   visiting)) {
          return false;
        }
        if(aggregate_info->class_kind == "union") {
          break;
        }
      }
      for(size_t i = payload->children.size(); i < aggregate_info->fields.size(); ++i) {
        const FieldInfo * input_field =
            aggregate_input_field(ctx, aggregate_info->fields[i]);
        if(input_field->default_initializer) {
          if(!initializer_is_nothrow(ctx,
                                     scope,
                                     input_field->type,
                                     *input_field->default_initializer,
                                     visiting)) {
            return false;
          }
        } else if(!default_initialization_is_nothrow(ctx,
                                                     scope,
                                                     input_field->type,
                                                     visiting)) {
          return false;
        }
        if(aggregate_info->class_kind == "union") {
          break;
        }
      }
      return true;
    }
  }

  ExprInfo converted;
  if(!analyze_expression_for_target(ctx, scope, *payload, target, converted)) {
    return false;
  }
  return !ctx.callsem_node_can_throw(scope, converted.node, visiting);
}

bool try_expand_builtin_type_trait_call_arg(SemanticContext & ctx,
                                            Scope & scope,
                                            const CppAstNode & arg,
                                            std::vector<TypePtr> & types)
{
  types.clear();
  if(arg.kind == CppAstKind::pack_expansion_expression && arg.children.size() == 1) {
    const CppAstNode & inner = arg.children[0];
    if(inner.kind == CppAstKind::id_expression) {
      if(const std::vector<TypePtr> * bound_pack = ctx.lookup_type_pack(scope, inner.value)) {
        types = *bound_pack;
        return true;
      }
    }

    TypePtr inner_type;
    if(!try_parse_builtin_type_trait_call_arg(ctx, scope, inner, inner_type)) {
      return false;
    }
    types.push_back(inner_type);
    return true;
  }

  TypePtr type;
  if(!try_parse_builtin_type_trait_call_arg(ctx, scope, arg, type)) {
    return false;
  }
  types.push_back(type);
  return true;
}

}  // namespace

TypePtr make_dependent_builtin_type_transform_type(
    const std::string & builtin_name,
    const std::string & arg_text,
    const TypePtr & arg_type)
{
  return make_dependent_builtin_type_transform_type_impl(
      builtin_name, arg_text, arg_type);
}

bool describe_dependent_builtin_type_transform(const TypePtr & type,
                                               std::string & builtin_name,
                                               TypePtr & arg_type)
{
  builtin_name.clear();
  arg_type.reset();
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_TYPE) {
    return false;
  }

  const std::string payload = named_type_semantic_payload(base);
  if(payload.compare(0,
                     sizeof(kDependentBuiltinTypeTransformPrefix) - 1,
                     kDependentBuiltinTypeTransformPrefix) != 0) {
    return false;
  }

  const size_t name_begin = sizeof(kDependentBuiltinTypeTransformPrefix) - 1;
  const size_t name_end = payload.find('|', name_begin);
  builtin_name =
      payload.substr(name_begin,
                     name_end == std::string::npos ? std::string::npos :
                                                      name_end - name_begin);
  arg_type = base->inner;
  return !builtin_name.empty() && arg_type;
}

bool is_dependent_builtin_type_transform_type(const TypePtr & type)
{
  std::string builtin_name;
  TypePtr arg_type;
  return describe_dependent_builtin_type_transform(type,
                                                   builtin_name,
                                                   arg_type);
}

bool is_supported_builtin_type_transform_name(const std::string & name)
{
  return builtin_type_transforms::is_supported_name(name);
}

bool apply_builtin_type_transform_kind(builtin_type_transforms::Kind kind,
                                       const TypePtr & arg_type,
                                       TypePtr & out)
{
  out.reset();
  if(!arg_type) {
    return false;
  }

  switch(kind) {
  case builtin_type_transforms::BTK_REMOVE_CV:
    out = strip_top_level_cv(arg_type);
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_REMOVE_CONST:
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, false, arg_type->cv_volatile) :
        arg_type;
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_REMOVE_VOLATILE:
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, arg_type->cv_const, false) :
        arg_type;
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_REMOVE_EXTENT:
  {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    out = base->kind == Type::TK_ARRAY ? base->inner : arg_type;
    return static_cast<bool>(out);
  }

  case builtin_type_transforms::BTK_REMOVE_ALL_EXTENTS:
  {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    while(base && base->kind == Type::TK_ARRAY) {
      base = strip_top_level_cv(base->inner);
    }
    out = base ? base : arg_type;
    return static_cast<bool>(out);
  }

  case builtin_type_transforms::BTK_REMOVE_REFERENCE:
    out = remove_reference_type(arg_type);
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_REMOVE_CONST_REF:
  {
    TypePtr no_ref = remove_reference_type(arg_type);
    out = no_ref && no_ref->kind == Type::TK_CV ?
        make_cv(no_ref->inner, false, no_ref->cv_volatile) :
        no_ref;
    return static_cast<bool>(out);
  }

  case builtin_type_transforms::BTK_REMOVE_CVREF:
    out = strip_top_level_cv(remove_reference_type(arg_type));
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_DECAY:
    out = apply_decay_type_transform(arg_type);
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_ADD_POINTER:
  {
    TypePtr pointee = remove_reference_type(arg_type);
    if(!pointee) {
      return false;
    }
    out = make_pointer(pointee);
    return static_cast<bool>(out);
  }

  case builtin_type_transforms::BTK_REMOVE_POINTER:
  {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    out = base->kind == Type::TK_POINTER ? base->inner : arg_type;
    return static_cast<bool>(out);
  }

  case builtin_type_transforms::BTK_MAKE_UNSIGNED:
  case builtin_type_transforms::BTK_MAKE_SIGNED:
    out = map_signedness_builtin_transform(
        kind == builtin_type_transforms::BTK_MAKE_UNSIGNED ?
            "__make_unsigned" :
            "__make_signed",
        arg_type);
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_ADD_LVALUE_REFERENCE:
  case builtin_type_transforms::BTK_ADD_RVALUE_REFERENCE:
  {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    if(is_void_type(base) || base->kind == Type::TK_LVALUE_REFERENCE) {
      out = arg_type;
      return true;
    }
    if(base->kind == Type::TK_RVALUE_REFERENCE) {
      out = kind == builtin_type_transforms::BTK_ADD_LVALUE_REFERENCE ?
          make_lvalue_reference_raw(base->inner) :
          arg_type;
      return true;
    }
    out = kind == builtin_type_transforms::BTK_ADD_LVALUE_REFERENCE ?
        make_lvalue_reference_raw(arg_type) :
        make_rvalue_reference_raw(arg_type);
    return true;
  }

  case builtin_type_transforms::BTK_IDENTITY:
    out = arg_type;
    return true;

  case builtin_type_transforms::BTK_GNU_COMPLEX:
    out = gnu_complex_type_for_component(arg_type);
    return static_cast<bool>(out);

  case builtin_type_transforms::BTK_UNDERLYING_TYPE:
  case builtin_type_transforms::BTK_UNKNOWN:
    break;
  }

  return false;
}

bool apply_builtin_type_transform(const std::string & name,
                                  const TypePtr & arg_type,
                                  TypePtr & out)
{
  return apply_builtin_type_transform_kind(
      builtin_type_transforms::kind_for_name(name),
      arg_type,
      out);
}

TypePtr gnu_complex_type_for_component(const TypePtr & component_type)
{
  TypePtr base = strip_top_level_cv(component_type);
  if(!base || base->kind != Type::TK_FUNDAMENTAL) {
    return TypePtr();
  }
  return exact_gnu_complex_type(base->fundamental);
}

bool is_gnu_complex_type(const TypePtr & type, TypePtr * component_type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }

  TypePtr component;
  if(base->named_key == "builtin _Complex float") {
    component = make_fundamental(FT_FLOAT);
  } else if(base->named_key == "builtin _Complex double") {
    component = make_fundamental(FT_DOUBLE);
  } else if(base->named_key == "builtin _Complex long double") {
    component = make_fundamental(FT_LONG_DOUBLE);
  } else {
    return false;
  }

  if(component_type) {
    *component_type = component;
  }
  return true;
}

bool is_builtin_va_list_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base &&
         base->kind == Type::TK_NAMED &&
         base->named_key == "builtin __builtin_va_list";
}

void register_builtin_types(Scope & scope)
{
  const TypePtr complex_float_type = exact_gnu_complex_type(FT_FLOAT);
  const TypePtr complex_double_type = exact_gnu_complex_type(FT_DOUBLE);
  const TypePtr complex_long_double_type = exact_gnu_complex_type(FT_LONG_DOUBLE);

  scope.named_types["bool"] = make_fundamental(FT_BOOL);
  scope.named_types["char"] = make_fundamental(FT_CHAR);
  scope.named_types["signed"] = make_fundamental(FT_INT);
  scope.named_types["signed int"] = make_fundamental(FT_INT);
  scope.named_types["signed char"] = make_fundamental(FT_SIGNED_CHAR);
  scope.named_types["signed short"] = make_fundamental(FT_SHORT_INT);
  scope.named_types["signed short int"] = make_fundamental(FT_SHORT_INT);
  scope.named_types["short"] = make_fundamental(FT_SHORT_INT);
  scope.named_types["short int"] = make_fundamental(FT_SHORT_INT);
  scope.named_types["int"] = make_fundamental(FT_INT);
  scope.named_types["signed long"] = make_fundamental(FT_LONG_INT);
  scope.named_types["signed long int"] = make_fundamental(FT_LONG_INT);
  scope.named_types["long"] = make_fundamental(FT_LONG_INT);
  scope.named_types["long int"] = make_fundamental(FT_LONG_INT);
  scope.named_types["signed long long"] = make_fundamental(FT_LONG_LONG_INT);
  scope.named_types["signed long long int"] = make_fundamental(FT_LONG_LONG_INT);
  scope.named_types["long long"] = make_fundamental(FT_LONG_LONG_INT);
  scope.named_types["long long int"] = make_fundamental(FT_LONG_LONG_INT);
  scope.named_types["__int128_t"] = make_fundamental(FT_INT128);
  scope.named_types["__int128"] = make_fundamental(FT_INT128);
  scope.named_types["signed __int128"] = make_fundamental(FT_INT128);
  scope.named_types["__int128 signed"] = make_fundamental(FT_INT128);
  scope.named_types["unsigned char"] = make_fundamental(FT_UNSIGNED_CHAR);
  scope.named_types["unsigned short"] = make_fundamental(FT_UNSIGNED_SHORT_INT);
  scope.named_types["unsigned short int"] = make_fundamental(FT_UNSIGNED_SHORT_INT);
  scope.named_types["unsigned int"] = make_fundamental(FT_UNSIGNED_INT);
  scope.named_types["unsigned long"] = make_fundamental(FT_UNSIGNED_LONG_INT);
  scope.named_types["unsigned long int"] = make_fundamental(FT_UNSIGNED_LONG_INT);
  scope.named_types["unsigned long long"] = make_fundamental(FT_UNSIGNED_LONG_LONG_INT);
  scope.named_types["unsigned long long int"] = make_fundamental(FT_UNSIGNED_LONG_LONG_INT);
  scope.named_types["__uint128_t"] = make_fundamental(FT_UINT128);
  scope.named_types["unsigned __int128"] = make_fundamental(FT_UINT128);
  scope.named_types["__int128 unsigned"] = make_fundamental(FT_UINT128);
  scope.named_types["wchar_t"] = make_fundamental(FT_WCHAR_T);
  scope.named_types["char16_t"] = make_fundamental(FT_CHAR16_T);
  scope.named_types["char32_t"] = make_fundamental(FT_CHAR32_T);
  scope.named_types["float"] = make_fundamental(FT_FLOAT);
  scope.named_types["_Float16"] = make_named("_Float16", "builtin _Float16", true, true, 2, 2);
  scope.named_types["_Float32"] = make_named("_Float32", "builtin _Float32", true, true, 4, 4);
  scope.named_types["_Float32x"] = make_named("_Float32x", "builtin _Float32x", true, true, 8, 8);
  scope.named_types["_Float64"] = make_named("_Float64", "builtin _Float64", true, true, 8, 8);
  scope.named_types["_Float64x"] = make_named("_Float64x", "builtin _Float64x", true, true, 16, 16);
  scope.named_types["_Float128"] = make_named("_Float128", "builtin _Float128", true, true, 16, 16);
  scope.named_types["__float128"] = make_named("__float128", "builtin __float128", true, true, 16, 16);
  scope.named_types["__ibm128"] = make_named("__ibm128", "builtin __ibm128", true, true, 16, 16);
  scope.named_types["_Complex float"] = complex_float_type;
  scope.named_types["__complex float"] = complex_float_type;
  scope.named_types["__complex__ float"] = complex_float_type;
  scope.named_types["double"] = make_fundamental(FT_DOUBLE);
  scope.named_types["_Complex double"] = complex_double_type;
  scope.named_types["__complex double"] = complex_double_type;
  scope.named_types["__complex__ double"] = complex_double_type;
  scope.named_types["long double"] = make_fundamental(FT_LONG_DOUBLE);
  scope.named_types["_Complex long double"] = complex_long_double_type;
  scope.named_types["__complex long double"] = complex_long_double_type;
  scope.named_types["__complex__ long double"] = complex_long_double_type;
  scope.named_types["void"] = make_fundamental(FT_VOID);
  scope.named_types["nullptr_t"] = make_fundamental(FT_NULLPTR_T);

#if defined(__APPLE__) && defined(__x86_64__)
  const size_t builtin_va_list_size = 24;
  const size_t builtin_va_list_alignment = 8;
#elif defined(__APPLE__)
  const size_t builtin_va_list_size = 8;
  const size_t builtin_va_list_alignment = 8;
#elif defined(__linux__)
  const size_t builtin_va_list_size = 24;
  const size_t builtin_va_list_alignment = 8;
#else
  const size_t builtin_va_list_size = 8;
  const size_t builtin_va_list_alignment = 8;
#endif
  scope.named_types["__builtin_va_list"] =
      make_named("__builtin_va_list",
                 "builtin __builtin_va_list",
                 true,
                 true,
                 builtin_va_list_alignment,
                 builtin_va_list_size);
}

void register_builtin_functions(Scope & scope,
                                const RegistrationHooks & hooks)
{
  const auto add_builtin =
      [&scope, &hooks](const std::string & name,
                       const TypePtr & result_type,
                       const std::vector<TypePtr> & params)
      {
        hooks.register_function(scope, name, result_type, params, false);
      };
  const auto add_nothrow_builtin =
      [&scope, &hooks](const std::string & name,
                       const TypePtr & result_type,
                       const std::vector<TypePtr> & params)
      {
        hooks.register_function(scope, name, result_type, params, true);
      };

  const TypePtr const_char_ptr = make_pointer(make_cv(make_fundamental(FT_CHAR), true, false));
  const TypePtr char_ptr = make_pointer(make_fundamental(FT_CHAR));
  const TypePtr void_ptr = make_pointer(make_fundamental(FT_VOID));
  const TypePtr const_void_ptr = make_pointer(make_cv(make_fundamental(FT_VOID), true, false));
  const TypePtr size_type = make_fundamental(FT_UNSIGNED_LONG_INT);
  const TypePtr builtin_va_list_type = scope.named_types["__builtin_va_list"];
  const TypePtr align_val_type =
      make_named("enum class align_val_t",
                 "enum class std::__1::align_val_t",
                 true,
                 true,
                 sizeof(unsigned long),
                 sizeof(unsigned long));
  align_val_type->named_enum_underlying_type = size_type;
  const TypePtr bool_type = make_fundamental(FT_BOOL);
  const TypePtr int_type = make_fundamental(FT_INT);
  const TypePtr long_type = make_fundamental(FT_LONG_INT);
  const TypePtr long_long_type = make_fundamental(FT_LONG_LONG_INT);
  const TypePtr unsigned_char_type = make_fundamental(FT_UNSIGNED_CHAR);
  const TypePtr unsigned_short_type = make_fundamental(FT_UNSIGNED_SHORT_INT);
  const TypePtr unsigned_int_type = make_fundamental(FT_UNSIGNED_INT);
  const TypePtr unsigned_long_type = make_fundamental(FT_UNSIGNED_LONG_INT);
  const TypePtr unsigned_long_long_type =
      make_fundamental(FT_UNSIGNED_LONG_LONG_INT);
  const TypePtr float_type = make_fundamental(FT_FLOAT);
  const TypePtr double_type = make_fundamental(FT_DOUBLE);
  const TypePtr long_double_type = make_fundamental(FT_LONG_DOUBLE);
  const TypePtr float_ptr = make_pointer(float_type);
  const TypePtr double_ptr = make_pointer(double_type);
  const TypePtr long_double_ptr = make_pointer(long_double_type);
  const TypePtr int_ptr = make_pointer(int_type);

  const auto add_float_triplet =
      [&](const std::string & base)
      {
        add_builtin("__builtin_" + base + "f", float_type, std::vector<TypePtr>(1, float_type));
        add_builtin("__builtin_" + base, double_type, std::vector<TypePtr>(1, double_type));
        add_builtin("__builtin_" + base + "l",
                    long_double_type,
                    std::vector<TypePtr>(1, long_double_type));
      };
  const auto add_float_triplet_to =
      [&](const std::string & base, const TypePtr & result_type)
      {
        add_builtin("__builtin_" + base + "f", result_type, std::vector<TypePtr>(1, float_type));
        add_builtin("__builtin_" + base, result_type, std::vector<TypePtr>(1, double_type));
        add_builtin("__builtin_" + base + "l",
                    result_type,
                    std::vector<TypePtr>(1, long_double_type));
      };
  const auto add_binary_float_triplet =
      [&](const std::string & base)
      {
        add_builtin("__builtin_" + base + "f", float_type, std::vector<TypePtr>(2, float_type));
        add_builtin("__builtin_" + base, double_type, std::vector<TypePtr>(2, double_type));
        add_builtin("__builtin_" + base + "l",
                    long_double_type,
                    std::vector<TypePtr>(2, long_double_type));
      };
  const auto add_overloaded_unary_builtin =
      [&](const std::string & name, const TypePtr & result_type)
      {
        add_builtin(name, result_type, std::vector<TypePtr>(1, float_type));
        add_builtin(name, result_type, std::vector<TypePtr>(1, double_type));
        add_builtin(name, result_type, std::vector<TypePtr>(1, long_double_type));
      };
  const auto add_overloaded_binary_builtin =
      [&](const std::string & name, const TypePtr & result_type)
      {
        add_builtin(name, result_type, std::vector<TypePtr>(2, float_type));
        add_builtin(name, result_type, std::vector<TypePtr>(2, double_type));
        add_builtin(name, result_type, std::vector<TypePtr>(2, long_double_type));
      };
  const auto add_fpclassify_builtin =
      [&]()
      {
        std::vector<TypePtr> params(5, int_type);
        params.push_back(float_type);
        add_builtin("__builtin_fpclassify", int_type, params);
        params.back() = double_type;
        add_builtin("__builtin_fpclassify", int_type, params);
        params.back() = long_double_type;
        add_builtin("__builtin_fpclassify", int_type, params);
      };
  const auto add_overloaded_unsigned_builtin =
      [&](const std::string & name,
          const TypePtr & result_type,
          bool allow_fallback)
      {
        const std::vector<TypePtr> unsigned_types{
            unsigned_char_type,
            unsigned_short_type,
            unsigned_int_type,
            unsigned_long_type,
            unsigned_long_long_type};
        for(size_t i = 0; i < unsigned_types.size(); ++i) {
          add_builtin(name, result_type, std::vector<TypePtr>(1, unsigned_types[i]));
          if(allow_fallback) {
            add_builtin(name, result_type, std::vector<TypePtr>{unsigned_types[i], int_type});
          }
        }
      };
  const auto add_ternary_float_triplet =
      [&](const std::string & base)
      {
        add_builtin("__builtin_" + base + "f", float_type, std::vector<TypePtr>(3, float_type));
        add_builtin("__builtin_" + base, double_type, std::vector<TypePtr>(3, double_type));
        add_builtin("__builtin_" + base + "l",
                    long_double_type,
                    std::vector<TypePtr>(3, long_double_type));
      };
  const auto add_same_type_overflow_builtin =
      [&](const std::string & name)
      {
        const std::vector<TypePtr> integer_types{
            make_fundamental(FT_SIGNED_CHAR),
            make_fundamental(FT_SHORT_INT),
            int_type,
            long_type,
            long_long_type,
            unsigned_char_type,
            unsigned_short_type,
            unsigned_int_type,
            unsigned_long_type,
            unsigned_long_long_type};
        for(size_t i = 0; i < integer_types.size(); ++i) {
          const TypePtr & value_type = integer_types[i];
          add_builtin(name,
                      bool_type,
                      std::vector<TypePtr>{value_type, value_type, make_pointer(value_type)});
        }
      };

  add_float_triplet("fabs");
  add_float_triplet("exp");
  add_float_triplet("exp2");
  add_float_triplet("expm1");
  add_float_triplet("sqrt");
  add_float_triplet("cbrt");
  add_float_triplet("acos");
  add_float_triplet("asin");
  add_float_triplet("atan");
  add_float_triplet("cos");
  add_float_triplet("sin");
  add_float_triplet("tan");
  add_float_triplet("acosh");
  add_float_triplet("asinh");
  add_float_triplet("atanh");
  add_float_triplet("cosh");
  add_float_triplet("sinh");
  add_float_triplet("tanh");
  add_float_triplet("log");
  add_float_triplet("log10");
  add_float_triplet("log1p");
  add_float_triplet("log2");
  add_float_triplet("logb");
  add_float_triplet("lgamma");
  add_float_triplet("tgamma");
  add_float_triplet("erf");
  add_float_triplet("erfc");
  add_float_triplet_to("ilogb", int_type);
  add_float_triplet("ceil");
  add_float_triplet("floor");
  add_float_triplet("nearbyint");
  add_float_triplet("rint");
  add_float_triplet_to("lrint", long_type);
  add_float_triplet_to("llrint", long_long_type);
  add_float_triplet_to("lround", long_type);
  add_float_triplet_to("llround", long_long_type);
  add_binary_float_triplet("atan2");
  add_binary_float_triplet("pow");
  add_binary_float_triplet("fdim");
  add_binary_float_triplet("fmax");
  add_binary_float_triplet("fmin");
  add_binary_float_triplet("fmod");
  add_binary_float_triplet("remainder");
  add_binary_float_triplet("copysign");
  add_binary_float_triplet("hypot");
  add_ternary_float_triplet("fma");
  add_overloaded_binary_builtin("__builtin_isgreater", bool_type);
  add_overloaded_binary_builtin("__builtin_isgreaterequal", bool_type);
  add_overloaded_binary_builtin("__builtin_isless", bool_type);
  add_overloaded_binary_builtin("__builtin_islessequal", bool_type);
  add_overloaded_binary_builtin("__builtin_islessgreater", bool_type);
  add_overloaded_binary_builtin("__builtin_isunordered", bool_type);
  add_overloaded_unary_builtin("__builtin_signbit", bool_type);
  add_overloaded_unary_builtin("__builtin_isfinite", bool_type);
  add_overloaded_unary_builtin("__builtin_isinf", bool_type);
  add_overloaded_unary_builtin("__builtin_isnan", bool_type);
  add_overloaded_unary_builtin("__builtin_isnormal", bool_type);
  add_fpclassify_builtin();

  add_builtin("__builtin_abs", int_type, std::vector<TypePtr>(1, int_type));
  add_builtin("__builtin_labs", long_type, std::vector<TypePtr>(1, long_type));
  add_builtin("__builtin_llabs", long_long_type, std::vector<TypePtr>(1, long_long_type));
  add_builtin("__builtin_inff", float_type, std::vector<TypePtr>());
  add_builtin("__builtin_inf", double_type, std::vector<TypePtr>());
  add_builtin("__builtin_infl", long_double_type, std::vector<TypePtr>());
  add_builtin("__builtin_huge_valf", float_type, std::vector<TypePtr>());
  add_builtin("__builtin_huge_val", double_type, std::vector<TypePtr>());
  add_builtin("__builtin_huge_vall", long_double_type, std::vector<TypePtr>());
  add_builtin("__builtin_nanf", float_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_nan", double_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_nanl", long_double_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_nansf", float_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_nans", double_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_nansl", long_double_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_frexpf", float_type, std::vector<TypePtr>{float_type, int_ptr});
  add_builtin("__builtin_frexp", double_type, std::vector<TypePtr>{double_type, int_ptr});
  add_builtin("__builtin_frexpl", long_double_type, std::vector<TypePtr>{long_double_type, int_ptr});
  add_builtin("__builtin_ldexpf", float_type, std::vector<TypePtr>{float_type, int_type});
  add_builtin("__builtin_ldexp", double_type, std::vector<TypePtr>{double_type, int_type});
  add_builtin("__builtin_ldexpl", long_double_type, std::vector<TypePtr>{long_double_type, int_type});
  add_builtin("__builtin_scalblnf", float_type, std::vector<TypePtr>{float_type, long_type});
  add_builtin("__builtin_scalbln", double_type, std::vector<TypePtr>{double_type, long_type});
  add_builtin("__builtin_scalblnl", long_double_type, std::vector<TypePtr>{long_double_type, long_type});
  add_builtin("__builtin_scalbnf", float_type, std::vector<TypePtr>{float_type, int_type});
  add_builtin("__builtin_scalbn", double_type, std::vector<TypePtr>{double_type, int_type});
  add_builtin("__builtin_scalbnl", long_double_type, std::vector<TypePtr>{long_double_type, int_type});
  add_builtin("__builtin_modff", float_type, std::vector<TypePtr>{float_type, float_ptr});
  add_builtin("__builtin_modf", double_type, std::vector<TypePtr>{double_type, double_ptr});
  add_builtin("__builtin_modfl", long_double_type, std::vector<TypePtr>{long_double_type, long_double_ptr});
  add_builtin("__builtin_remquof", float_type, std::vector<TypePtr>{float_type, float_type, int_ptr});
  add_builtin("__builtin_remquo", double_type, std::vector<TypePtr>{double_type, double_type, int_ptr});
  add_builtin("__builtin_remquol", long_double_type, std::vector<TypePtr>{long_double_type, long_double_type, int_ptr});
  add_builtin("__builtin_nextafterf", float_type, std::vector<TypePtr>{float_type, float_type});
  add_builtin("__builtin_nextafter", double_type, std::vector<TypePtr>{double_type, double_type});
  add_builtin("__builtin_nextafterl", long_double_type, std::vector<TypePtr>{long_double_type, long_double_type});
  add_builtin("__builtin_nexttowardf", float_type, std::vector<TypePtr>{float_type, long_double_type});
  add_builtin("__builtin_nexttoward", double_type, std::vector<TypePtr>{double_type, long_double_type});
  add_builtin("__builtin_nexttowardl", long_double_type, std::vector<TypePtr>{long_double_type, long_double_type});
  add_builtin("__builtin_round", float_type, std::vector<TypePtr>(1, float_type));
  add_builtin("__builtin_round", double_type, std::vector<TypePtr>(1, double_type));
  add_builtin("__builtin_roundl", long_double_type, std::vector<TypePtr>(1, long_double_type));
  add_builtin("__builtin_trunc", float_type, std::vector<TypePtr>(1, float_type));
  add_builtin("__builtin_trunc", double_type, std::vector<TypePtr>(1, double_type));
  add_builtin("__builtin_truncl", long_double_type, std::vector<TypePtr>(1, long_double_type));
  add_builtin("__builtin_flt_rounds", int_type, std::vector<TypePtr>());
  add_builtin("__builtin_constant_p", int_type, std::vector<TypePtr>(1, int_type));
  add_builtin("__builtin_is_constant_evaluated", bool_type, std::vector<TypePtr>());
  add_builtin("__builtin_memcpy", void_ptr, std::vector<TypePtr>{void_ptr, const_void_ptr, size_type});
  add_builtin("__builtin_memmove", void_ptr, std::vector<TypePtr>{void_ptr, const_void_ptr, size_type});
  add_builtin("__builtin_memcmp", int_type, std::vector<TypePtr>{const_void_ptr, const_void_ptr, size_type});
  add_builtin("__builtin_memchr", void_ptr, std::vector<TypePtr>{const_void_ptr, int_type, size_type});
  add_builtin("__builtin_memset", void_ptr, std::vector<TypePtr>{void_ptr, int_type, size_type});
  add_builtin("__builtin_bzero", make_fundamental(FT_VOID), std::vector<TypePtr>{void_ptr, size_type});
  add_builtin("__builtin_strcmp", int_type, std::vector<TypePtr>(2, const_char_ptr));
  add_builtin("__builtin_strchr", char_ptr, std::vector<TypePtr>{const_char_ptr, int_type});
  add_builtin("__builtin_strlen", size_type, std::vector<TypePtr>(1, const_char_ptr));
  add_builtin("__builtin_vsnprintf",
              int_type,
              std::vector<TypePtr>{char_ptr, size_type, const_char_ptr, builtin_va_list_type});
  add_builtin("__builtin_alloca", void_ptr, std::vector<TypePtr>{size_type});
  add_builtin("__builtin_expect", long_type, std::vector<TypePtr>{long_type, long_type});
  add_builtin("__builtin_prefetch", make_fundamental(FT_VOID), std::vector<TypePtr>{const_void_ptr});
  add_builtin("__builtin_prefetch",
              make_fundamental(FT_VOID),
              std::vector<TypePtr>{const_void_ptr, int_type});
  add_builtin("__builtin_prefetch",
              make_fundamental(FT_VOID),
              std::vector<TypePtr>{const_void_ptr, int_type, int_type});
  add_builtin("__builtin_assume_aligned",
              void_ptr,
              std::vector<TypePtr>{const_void_ptr, size_type});
  add_builtin("__builtin_assume_aligned",
              void_ptr,
              std::vector<TypePtr>{const_void_ptr, size_type, size_type});
  add_builtin("__builtin_unreachable", make_fundamental(FT_VOID), std::vector<TypePtr>());
  add_same_type_overflow_builtin("__builtin_add_overflow");
  add_same_type_overflow_builtin("__builtin_sub_overflow");
  add_same_type_overflow_builtin("__builtin_mul_overflow");
  const auto add_allocation_builtin_family =
      [&](const std::string & new_name,
          const std::string & new_array_name,
          const std::string & delete_name,
          const std::string & delete_array_name)
      {
        add_builtin(new_name, void_ptr, std::vector<TypePtr>{size_type});
        add_builtin(new_name, void_ptr, std::vector<TypePtr>{size_type, align_val_type});
        if(!new_array_name.empty()) {
          add_builtin(new_array_name, void_ptr, std::vector<TypePtr>{size_type});
          add_builtin(new_array_name, void_ptr, std::vector<TypePtr>{size_type, align_val_type});
        }

        add_nothrow_builtin(delete_name, make_fundamental(FT_VOID), std::vector<TypePtr>{void_ptr});
        add_nothrow_builtin(delete_name,
                            make_fundamental(FT_VOID),
                            std::vector<TypePtr>{void_ptr, size_type});
        add_nothrow_builtin(delete_name,
                            make_fundamental(FT_VOID),
                            std::vector<TypePtr>{void_ptr, align_val_type});
        add_nothrow_builtin(delete_name,
                            make_fundamental(FT_VOID),
                            std::vector<TypePtr>{void_ptr, size_type, align_val_type});
        if(!delete_array_name.empty()) {
          add_nothrow_builtin(delete_array_name,
                              make_fundamental(FT_VOID),
                              std::vector<TypePtr>{void_ptr});
          add_nothrow_builtin(delete_array_name,
                              make_fundamental(FT_VOID),
                              std::vector<TypePtr>{void_ptr, size_type});
          add_nothrow_builtin(delete_array_name,
                              make_fundamental(FT_VOID),
                              std::vector<TypePtr>{void_ptr, align_val_type});
          add_nothrow_builtin(delete_array_name,
                              make_fundamental(FT_VOID),
                              std::vector<TypePtr>{void_ptr, size_type, align_val_type});
        }
      };
  add_allocation_builtin_family("operator new",
                                "operator new[]",
                                "operator delete",
                                "operator delete[]");
  add_allocation_builtin_family("__builtin_operator_new",
                                std::string(),
                                "__builtin_operator_delete",
                                std::string());
  add_builtin("__builtin_bswap16",
              unsigned_short_type,
              std::vector<TypePtr>{unsigned_short_type});
  add_builtin("__builtin_bswap32",
              unsigned_int_type,
              std::vector<TypePtr>{unsigned_int_type});
  add_builtin("__builtin_bswap64",
              unsigned_long_type,
              std::vector<TypePtr>{unsigned_long_type});
  add_builtin("__builtin_bswap64",
              unsigned_long_long_type,
              std::vector<TypePtr>{unsigned_long_long_type});
  add_builtin("__builtin_clz",
              int_type,
              std::vector<TypePtr>{unsigned_int_type});
  add_builtin("__builtin_clzl",
              int_type,
              std::vector<TypePtr>{unsigned_long_type});
  add_builtin("__builtin_clzll",
              int_type,
              std::vector<TypePtr>{unsigned_long_long_type});
  add_builtin("__builtin_ctz",
              int_type,
              std::vector<TypePtr>{unsigned_int_type});
  add_builtin("__builtin_ctzl",
              int_type,
              std::vector<TypePtr>{unsigned_long_type});
  add_builtin("__builtin_ctzll",
              int_type,
              std::vector<TypePtr>{unsigned_long_long_type});
  add_overloaded_unsigned_builtin("__builtin_clzg", int_type, true);
  add_overloaded_unsigned_builtin("__builtin_ctzg", int_type, true);
  add_overloaded_unsigned_builtin("__builtin_popcountg", int_type, false);
  add_builtin("__builtin_popcount",
              int_type,
              std::vector<TypePtr>{unsigned_int_type});
  add_builtin("__builtin_popcountl",
              int_type,
              std::vector<TypePtr>{unsigned_long_type});
  add_builtin("__builtin_popcountll",
              int_type,
              std::vector<TypePtr>{unsigned_long_long_type});
  add_builtin("__c11_atomic_thread_fence", make_fundamental(FT_VOID), std::vector<TypePtr>{int_type});
  add_builtin("__c11_atomic_signal_fence", make_fundamental(FT_VOID), std::vector<TypePtr>{int_type});
  add_builtin("__c11_atomic_is_lock_free", bool_type, std::vector<TypePtr>{size_type});
  add_builtin("__atomic_thread_fence", make_fundamental(FT_VOID), std::vector<TypePtr>{int_type});
  add_builtin("__atomic_signal_fence", make_fundamental(FT_VOID), std::vector<TypePtr>{int_type});
}

bool expression_is_nothrow(SemanticContext & ctx,
                           Scope & scope,
                           const CppAstNode & expr,
                           bool & out)
{
  ExprInfo analyzed;
  if(!analyze_expression(ctx, scope, expr, analyzed)) {
    return false;
  }

  std::set<FunctionBinding *> visiting;
  out = !ctx.callsem_node_can_throw(scope, analyzed.node, visiting);
  return true;
}

bool evaluate_builtin_type_trait(SemanticContext & ctx,
                                 Scope & scope,
                                 const std::string & name,
                                 const TypePtr & type,
                                 long long & out)
{
  DIAG_CONTEXT("evaluate_builtin_type_trait [" + name + " " +
               (type ? describe_type(type) : std::string("<null-type>")) + "]");
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  if(name == "__is_trivially_destructible" || name == "__has_trivial_destructor") {
    out = is_trivially_destructible_type(ctx, base) ? 1 : 0;
    return true;
  }

  if(name == "__is_pod") {
    if(base->kind == Type::TK_ARRAY) {
      long long element_value = 0;
      out = (base->inner &&
             evaluate_builtin_type_trait(ctx, scope, name, base->inner, element_value) &&
             element_value != 0) ? 1 : 0;
      return true;
    }
    long long trivial_value = 0;
    long long standard_layout_value = 0;
    out = (evaluate_builtin_type_trait(ctx, scope, "__is_trivial", base, trivial_value) &&
           evaluate_builtin_type_trait(
               ctx, scope, "__is_standard_layout", base, standard_layout_value) &&
           trivial_value != 0 && standard_layout_value != 0) ? 1 : 0;
    return true;
  }

  if(name == "__is_trivial") {
    if(base->kind == Type::TK_ARRAY) {
      long long element_value = 0;
      out = (base->inner &&
             evaluate_builtin_type_trait(ctx, scope, name, base->inner, element_value) &&
             element_value != 0) ? 1 : 0;
      return true;
    }
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(ctx, base)) {
      out = 1;
      return true;
    }
    ClassInfo * info = ctx.complete_class_type(base);
    if(!info || !info->complete) {
      return false;
    }
    FunctionBinding * ctor = ctx.select_default_constructor_for_builtin_trait(scope, *info);
    out = (ctor &&
           (ctor->is_defaulted || ctor->synthesized || ctor->is_aggregate_constructor) &&
           is_trivially_destructible_type(ctx, base)) ? 1 : 0;
    return true;
  }

  if(name == "__is_trivially_constructible" || name == "__has_trivial_constructor") {
    if(base->kind == Type::TK_ARRAY) {
      long long element_value = 0;
      out = (base->inner &&
             evaluate_builtin_type_trait(ctx, scope, name, base->inner, element_value) &&
             element_value != 0) ? 1 : 0;
      return true;
    }
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(ctx, base)) {
      out = 1;
      return true;
    }
    out = semantic_class_model::is_trivially_default_constructible_type_for_host_abi(ctx, base) ?
              1 :
              0;
    return true;
  }

  if(name == "__is_trivially_copyable") {
    out = is_trivially_copyable_type(ctx, base) ? 1 : 0;
    return true;
  }

  if(name == "__is_destructible" || name == "__is_nothrow_destructible") {
    out = is_destructible_type(ctx, base) ? 1 : 0;
    return true;
  }

  if(name == "__is_constructible" || name == "__is_nothrow_constructible") {
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION ||
       is_void_type(base) || base->kind == Type::TK_ARRAY) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(ctx, base)) {
      out = 1;
      return true;
    }
    ClassInfo * info = ctx.complete_class_type(base);
    if(!info || !info->complete) {
      return false;
    }
    FunctionBinding * ctor = ctx.select_default_constructor_for_builtin_trait(scope, *info);
    if(!ctor || ctor->is_deleted) {
      out = 0;
      return true;
    }
    if(name == "__is_nothrow_constructible") {
      std::set<FunctionBinding *> visiting;
      out = function_binding_is_nothrow(ctx, scope, *ctor, visiting) ? 1 : 0;
    } else {
      out = 1;
    }
    return true;
  }

  if(name == "__is_integral") {
    out = is_integral_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_floating_point") {
    out = is_floating_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_arithmetic") {
    out = (is_integral_type(base) || is_floating_type(base)) ? 1 : 0;
    return true;
  }
  if(name == "__is_signed") {
    out = (is_floating_type(base) ||
           (is_integral_type(base) && !is_bool_type(base) && !is_unsigned_integral_type(base))) ?
        1 : 0;
    return true;
  }
  if(name == "__is_unsigned") {
    out = is_unsigned_integral_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_reference") {
    out = is_reference_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_lvalue_reference") {
    out = base->kind == Type::TK_LVALUE_REFERENCE ? 1 : 0;
    return true;
  }
  if(name == "__is_rvalue_reference") {
    out = base->kind == Type::TK_RVALUE_REFERENCE ? 1 : 0;
    return true;
  }
  if(name == "__is_void") {
    out = is_void_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_array") {
    out = is_array_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_pointer") {
    out = is_pointer_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_member_pointer") {
    out = base->kind == Type::TK_MEMBER_POINTER ? 1 : 0;
    return true;
  }
  if(name == "__is_member_object_pointer") {
    out = (base->kind == Type::TK_MEMBER_POINTER &&
           !is_function_type(base->inner)) ? 1 : 0;
    return true;
  }
  if(name == "__is_member_function_pointer") {
    out = (base->kind == Type::TK_MEMBER_POINTER &&
           is_function_type(base->inner)) ? 1 : 0;
    return true;
  }
  if(name == "__is_function") {
    out = is_function_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_const") {
    out = is_const_object_type(type) ? 1 : 0;
    return true;
  }
  if(name == "__is_volatile") {
    TypePtr inner;
    bool cv_const = false;
    bool cv_volatile = false;
    out = top_level_cv_flags(type, inner, cv_const, cv_volatile) && cv_volatile ? 1 : 0;
    return true;
  }
  if(name == "__is_enum") {
    out = is_named_enum_type(ctx, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_union") {
    out = is_named_union_type(ctx, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_class") {
    out = is_named_class_type(ctx, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_fundamental") {
    out = base->kind == Type::TK_FUNDAMENTAL ? 1 : 0;
    return true;
  }
  if(name == "__is_scalar") {
    out = is_scalar_or_member_pointer_type(ctx, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_compound") {
    out = base->kind != Type::TK_FUNDAMENTAL ? 1 : 0;
    return true;
  }
  if(name == "__is_object") {
    out = (!is_reference_type(base) && base->kind != Type::TK_FUNCTION && !is_void_type(base)) ?
        1 : 0;
    return true;
  }
  if(name == "__array_rank") {
    out = array_rank(base);
    return true;
  }
  if(name == "__is_literal_type") {
    out = is_literal_type(ctx, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_empty") {
    ClassInfo * info = ctx.complete_class_type(base);
    if(!info) {
      out = 0;
      return true;
    }
    out = ctx.is_empty_class_info(info) ? 1 : 0;
    return true;
  }
  if(name == "__is_final") {
    ClassInfo * info = ctx.complete_class_type(base);
    out = (info && info->is_final) ? 1 : 0;
    return true;
  }
  if(name == "__is_abstract") {
    ClassInfo * info = ctx.complete_class_type(base);
    out = (info && class_info_is_abstract(*info)) ? 1 : 0;
    return true;
  }
  if(name == "__is_polymorphic") {
    ClassInfo * info = ctx.complete_class_type(base);
    out = (info && info->is_polymorphic) ? 1 : 0;
    return true;
  }
  if(name == "__has_virtual_destructor") {
    ClassInfo * info = ctx.complete_class_type(base);
    out = (info && class_info_has_virtual_destructor(*info)) ? 1 : 0;
    return true;
  }
  if(name == "__is_standard_layout") {
    out = is_standard_layout_type(ctx, base) ? 1 : 0;
    return true;
  }

  return false;
}

bool is_supported_builtin_type_trait_name(const std::string & name)
{
  static const std::set<std::string> supported = {
      "__array_rank",
      "__has_trivial_destructor",
      "__has_virtual_destructor",
      "__is_abstract",
      "__is_arithmetic",
      "__is_array",
      "__is_assignable",
      "__is_base_of",
      "__is_class",
      "__is_compound",
      "__is_const",
      "__is_constructible",
      "__is_convertible",
      "__is_destructible",
      "__is_empty",
      "__is_enum",
      "__is_final",
      "__is_floating_point",
      "__is_function",
      "__is_fundamental",
      "__is_integral",
      "__is_lvalue_reference",
      "__is_literal_type",
      "__is_member_function_pointer",
      "__is_member_object_pointer",
      "__is_member_pointer",
      "__is_nothrow_constructible",
      "__is_nothrow_convertible",
      "__is_nothrow_destructible",
      "__is_nothrow_assignable",
      "__is_object",
      "__is_pod",
      "__is_pointer",
      "__is_polymorphic",
      "__is_reference",
      "__is_rvalue_reference",
      "__is_same",
      "__is_scalar",
      "__reference_constructs_from_temporary",
      "__reference_binds_to_temporary",
      "__is_signed",
      "__is_standard_layout",
      "__is_trivial",
      "__has_trivial_constructor",
      "__is_trivially_constructible",
      "__is_trivially_assignable",
      "__is_trivially_copyable",
      "__is_trivially_destructible",
      "__is_union",
      "__is_unsigned",
      "__is_void",
      "__is_volatile",
  };
  return supported.find(name) != supported.end();
}

TypePtr builtin_type_trait_result_type(const std::string & name)
{
  if(name == "__array_rank") {
    return make_fundamental(FT_UNSIGNED_LONG_INT);
  }
  return make_fundamental(FT_BOOL);
}

bool evaluate_builtin_binary_type_trait(SemanticContext & ctx,
                                        Scope & scope,
                                        const std::string & name,
                                        const TypePtr & lhs,
                                        const TypePtr & rhs,
                                        long long & out)
{
  if(name == "__is_same") {
    out = type_equals(lhs, rhs) ? 1 : 0;
    return true;
  }

  if(name == "__is_base_of") {
    TypePtr base_type = strip_top_level_cv(lhs);
    TypePtr derived_type = strip_top_level_cv(rhs);
    ClassInfo * base_info = base_type ? ctx.class_info_for_type(base_type) : nullptr;
    ClassInfo * derived_info =
        derived_type ? ctx.complete_class_type(derived_type) : nullptr;
    if(!derived_info && derived_type) {
      derived_info = ctx.class_info_for_type(derived_type);
    }
    out = (base_info &&
           derived_info &&
           base_info->class_kind != "union" &&
           base_info->class_kind != "enum" &&
           derived_info->class_kind != "union" &&
           derived_info->class_kind != "enum" &&
           semantic_lookup::is_same_or_derived(derived_info, base_info)) ? 1 : 0;
    return true;
  }

  if(name == "__is_assignable") {
    TypePtr lhs_base = strip_top_level_cv(lhs);
    if(!lhs_base || lhs_base->kind != Type::TK_LVALUE_REFERENCE) {
      out = 0;
      return true;
    }

    TypePtr target = lhs_base->inner;
    if(is_const_object_type(target)) {
      out = 0;
      return true;
    }

    TypePtr rhs_base = strip_top_level_cv(rhs);
    if(!rhs_base) {
      return false;
    }

    ExprInfo rhs_expr;
    if(rhs_base->kind == Type::TK_LVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_LVALUE;
    } else if(rhs_base->kind == Type::TK_RVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_XVALUE;
    } else {
      rhs_expr.type = rhs;
      rhs_expr.category = VC_PRVALUE;
    }

    SameClassAssignmentTrait class_trait =
        evaluate_same_class_assignment_trait(ctx, scope, target, rhs_expr);
    if(class_trait.known) {
      out = class_trait.assignable ? 1 : 0;
      return true;
    }

    if(FunctionBinding * binding =
           find_class_assignment_operator_for_trait(ctx, scope, target, rhs_expr)) {
      out = binding->is_deleted ? 0 : 1;
      return true;
    }

    ExprInfo converted;
    out = try_argument_conversion(ctx, scope, target, rhs_expr, converted) ? 1 : 0;
    return true;
  }

  if(name == "__is_nothrow_assignable") {
    TypePtr lhs_base = strip_top_level_cv(lhs);
    if(!lhs_base || lhs_base->kind != Type::TK_LVALUE_REFERENCE) {
      out = 0;
      return true;
    }

    TypePtr target = lhs_base->inner;
    if(is_const_object_type(target)) {
      out = 0;
      return true;
    }

    TypePtr rhs_base = strip_top_level_cv(rhs);
    if(!rhs_base) {
      return false;
    }

    ExprInfo rhs_expr;
    if(rhs_base->kind == Type::TK_LVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_LVALUE;
    } else if(rhs_base->kind == Type::TK_RVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_XVALUE;
    } else {
      rhs_expr.type = rhs;
      rhs_expr.category = VC_PRVALUE;
    }

    SameClassAssignmentTrait class_trait =
        evaluate_same_class_assignment_trait(ctx, scope, target, rhs_expr);
    if(class_trait.known) {
      if(!class_trait.assignable || !class_trait.binding) {
        out = 0;
        return true;
      }
      std::set<FunctionBinding *> visiting;
      out = function_binding_is_nothrow(ctx, scope, *class_trait.binding, visiting) ? 1 : 0;
      return true;
    }

    if(FunctionBinding * binding =
           find_class_assignment_operator_for_trait(ctx, scope, target, rhs_expr)) {
      std::set<FunctionBinding *> visiting;
      out = function_binding_is_nothrow(ctx, scope, *binding, visiting) ? 1 : 0;
      return true;
    }

    ExprInfo converted;
    if(!try_argument_conversion(ctx, scope, target, rhs_expr, converted)) {
      out = 0;
      return true;
    }

    std::set<FunctionBinding *> visiting;
    out = ctx.callsem_node_can_throw(scope, converted.node, visiting) ? 0 : 1;
    return true;
  }

  if(name == "__is_convertible" || name == "__is_nothrow_convertible") {
    TypePtr source = strip_top_level_cv(lhs);
    TypePtr target = strip_top_level_cv(rhs);
    if(!source || !target) {
      return false;
    }
    if(is_void_type(target)) {
      out = is_void_type(source) ? 1 : 0;
      return true;
    }
    if(is_void_type(source)) {
      out = 0;
      return true;
    }

    ExprInfo source_expr = make_builtin_trait_expr_info(lhs);

    ExprInfo converted;
    if(!try_argument_conversion(ctx, scope, rhs, source_expr, converted)) {
      out = 0;
      return true;
    }
    if(name == "__is_nothrow_convertible") {
      std::set<FunctionBinding *> visiting;
      out = ctx.callsem_node_can_throw(scope, converted.node, visiting) ? 0 : 1;
    } else {
      out = 1;
    }
    return true;
  }

  if(name == "__is_constructible" || name == "__is_nothrow_constructible") {
    TypePtr target = strip_top_level_cv(lhs);
    TypePtr source = strip_top_level_cv(rhs);
    if(!target || !source) {
      return false;
    }

    ExprInfo rhs_expr = make_builtin_trait_expr_info(rhs);

    constructor_lifecycle_service::ConstructorSelectionResult direct_selection;
    if(try_direct_class_construction_for_trait(
           ctx, scope, target, rhs_expr, direct_selection)) {
      if(!direct_selection.ctor) {
        out = 0;
        return true;
      }
      if(name == "__is_nothrow_constructible") {
        std::set<FunctionBinding *> visiting;
        bool can_throw = false;
        for(size_t i = 0; i < direct_selection.converted_args.size(); ++i) {
          if(ctx.callsem_node_can_throw(scope,
                                        direct_selection.converted_args[i].node,
                                        visiting)) {
            can_throw = true;
            break;
          }
        }
        if(!can_throw) {
          can_throw = !function_binding_is_nothrow(ctx,
                                                   scope,
                                                   *direct_selection.ctor,
                                                   visiting);
        }
        out = can_throw ? 0 : 1;
      } else {
        out = 1;
      }
      return true;
    }

    SameClassAssignmentTrait class_trait =
        evaluate_same_class_assignment_trait(ctx, scope, target, rhs_expr);
    if(class_trait.known && !class_trait.assignable) {
      out = 0;
      return true;
    }

    ExprInfo converted;
    if(!try_argument_conversion(ctx, scope, target, rhs_expr, converted)) {
      out = 0;
      return true;
    }
    if(name == "__is_nothrow_constructible") {
      std::set<FunctionBinding *> visiting;
      out = ctx.callsem_node_can_throw(scope, converted.node, visiting) ? 0 : 1;
    } else {
      out = 1;
    }
    return true;
  }

  if(name == "__is_trivially_constructible") {
    TypePtr target = strip_top_level_cv(lhs);
    TypePtr source = strip_top_level_cv(rhs);
    if(!target || !source) {
      return false;
    }

    ExprInfo rhs_expr;
    if(source->kind == Type::TK_LVALUE_REFERENCE) {
      rhs_expr.type = source->inner;
      rhs_expr.category = VC_LVALUE;
    } else if(source->kind == Type::TK_RVALUE_REFERENCE) {
      rhs_expr.type = source->inner;
      rhs_expr.category = VC_XVALUE;
    } else {
      rhs_expr.type = rhs;
      rhs_expr.category = VC_PRVALUE;
    }

    ExprInfo converted;
    if(!try_argument_conversion(ctx, scope, target, rhs_expr, converted)) {
      out = 0;
      return true;
    }

    TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
    TypePtr source_base = strip_top_level_cv(remove_reference_type(source));
    if(!target_base || !source_base) {
      out = 0;
      return true;
    }

    if(target_base->kind == Type::TK_FUNCTION || is_void_type(target_base) ||
       target_base->kind == Type::TK_ARRAY) {
      out = 0;
      return true;
    }

    if(target_base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(ctx, target_base)) {
      out = 1;
      return true;
    }

    out = (type_equals(target_base, source_base) &&
           semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(
               ctx, target_base)) ? 1 : 0;
    return true;
  }

  if(name == "__reference_constructs_from_temporary" ||
     name == "__reference_binds_to_temporary") {
    TypePtr target = strip_top_level_cv(lhs);
    TypePtr source = strip_top_level_cv(rhs);
    if(!target || !source) {
      return false;
    }

    if((target->kind != Type::TK_LVALUE_REFERENCE &&
        target->kind != Type::TK_RVALUE_REFERENCE) ||
       source->kind == Type::TK_LVALUE_REFERENCE ||
       source->kind == Type::TK_RVALUE_REFERENCE) {
      out = 0;
      return true;
    }

    ExprInfo rhs_expr;
    rhs_expr.type = rhs;
    rhs_expr.category = VC_PRVALUE;

    ExprInfo converted;
    out = try_argument_conversion(ctx, scope, lhs, rhs_expr, converted) ? 1 : 0;
    return true;
  }

  if(name == "__is_trivially_assignable") {
    TypePtr lhs_base = strip_top_level_cv(lhs);
    if(!lhs_base || lhs_base->kind != Type::TK_LVALUE_REFERENCE) {
      out = 0;
      return true;
    }

    TypePtr target = lhs_base->inner;
    if(is_const_object_type(target)) {
      out = 0;
      return true;
    }

    TypePtr rhs_base = strip_top_level_cv(rhs);
    if(!rhs_base) {
      return false;
    }

    ExprInfo rhs_expr;
    if(rhs_base->kind == Type::TK_LVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_LVALUE;
    } else if(rhs_base->kind == Type::TK_RVALUE_REFERENCE) {
      rhs_expr.type = rhs_base->inner;
      rhs_expr.category = VC_XVALUE;
    } else {
      rhs_expr.type = rhs;
      rhs_expr.category = VC_PRVALUE;
    }

    ExprInfo converted;
    if(!try_argument_conversion(ctx, scope, target, rhs_expr, converted)) {
      out = 0;
      return true;
    }

    if(target->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(ctx, target)) {
      out = 1;
      return true;
    }

    if(target->kind != Type::TK_NAMED) {
      out = 0;
      return true;
    }

    if(is_same_class_reference_parameter(target, rhs_base, Type::TK_LVALUE_REFERENCE) ||
       is_same_class_reference_parameter(target, rhs_base, Type::TK_RVALUE_REFERENCE)) {
      out = is_trivially_copy_assignable_type(ctx, target) ? 1 : 0;
      return true;
    }

    out = 0;
    return true;
  }

  return false;
}

bool evaluate_builtin_type_trait(SemanticContext & ctx,
                                 Scope & scope,
                                 const std::string & name,
                                 const std::vector<TypePtr> & types,
                                 long long & out)
{
  if(types.empty()) {
    return false;
  }

  if(types.size() == 1) {
    return evaluate_builtin_type_trait(ctx, scope, name, types[0], out);
  }

  if(types.size() == 2) {
    return evaluate_builtin_binary_type_trait(
        ctx, scope, name, types[0], types[1], out);
  }

  if(name != "__is_constructible" && name != "__is_nothrow_constructible") {
    return false;
  }

  TypePtr target = strip_top_level_cv(types[0]);
  if(!target) {
    return false;
  }
  if(is_reference_type(target) ||
     target->kind == Type::TK_FUNCTION ||
     is_void_type(target) ||
     target->kind == Type::TK_ARRAY) {
    out = 0;
    return true;
  }
  if(target->kind != Type::TK_NAMED) {
    out = 0;
    return true;
  }

  std::vector<ExprInfo> args;
  args.reserve(types.size() - 1);
  for(size_t i = 1; i < types.size(); ++i) {
    TypePtr arg_type = strip_top_level_cv(types[i]);
    if(!arg_type) {
      return false;
    }
    args.push_back(make_builtin_trait_expr_info(types[i]));
  }

  constructor_lifecycle_service::ConstructorSelectionResult direct_selection;
  if(!try_direct_class_construction_for_trait(
         ctx, scope, target, args, direct_selection)) {
    return false;
  }
  if(!direct_selection.ctor) {
    out = 0;
    return true;
  }

  if(name == "__is_nothrow_constructible") {
    std::set<FunctionBinding *> visiting;
    bool can_throw = false;
    for(size_t i = 0; i < direct_selection.converted_args.size(); ++i) {
      if(ctx.callsem_node_can_throw(scope,
                                    direct_selection.converted_args[i].node,
                                    visiting)) {
        can_throw = true;
        break;
      }
    }
    if(!can_throw) {
      can_throw = !function_binding_is_nothrow(ctx,
                                               scope,
                                               *direct_selection.ctor,
                                               visiting);
    }
    out = can_throw ? 0 : 1;
  } else {
    out = 1;
  }
  return true;
}

bool try_parse_builtin_type_trait_call_arg(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & arg,
                                           TypePtr & type)
{
  type = TypePtr();
  if(arg.kind == CppAstKind::type_id) {
    return ctx.parse_type_id(scope, arg, type, true);
  }
  if(arg.kind == CppAstKind::id_expression) {
    type = ctx.lookup_type_node(scope, arg, arg.value, true);
    if(!type && !scope_has_template_placeholders(ctx, scope)) {
      std::ostringstream out;
      out << "failed builtin type trait id-expression lookup: " << arg.value;
      out << " [scope " << semantic_trace::scope_name_for_diagnostic(scope) << "]";
      const std::string bindings = semantic_trace::scope_bindings_for_diagnostic(scope);
      if(!bindings.empty()) {
        out << " [bindings " << bindings << "]";
      }
      throw std::logic_error(out.str());
    }
    return static_cast<bool>(type);
  }

  const std::string arg_text = node_text(arg);
  if(has_invalid_top_level_qualified_owner_syntax(trim_space(arg_text))) {
    return false;
  }

  type = ctx.lookup_type_node(scope, arg, trim_space(arg_text), true);
  if(type) {
    return true;
  }

  if(!scope_has_template_placeholders(ctx, scope)) {
    std::ostringstream out;
    out << "failed builtin type trait argument parse: " << arg_text;
    out << " [kind " << cppast_kind_text(arg.kind) << "]";
    out << " [scope " << semantic_trace::scope_name_for_diagnostic(scope) << "]";
    const std::string bindings = semantic_trace::scope_bindings_for_diagnostic(scope);
    if(!bindings.empty()) {
      out << " [bindings " << bindings << "]";
    }
    throw std::logic_error(out.str());
  }
  return false;
}

bool try_parse_builtin_type_trait_call(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node,
                                       std::string & trait_name,
                                       std::vector<TypePtr> & types)
{
  trait_name.clear();
  types.clear();

  if(node.kind != CppAstKind::call_expression ||
     node.children.size() != 2 ||
     node.children[0].kind != CppAstKind::id_expression ||
     (node.children[1].kind != CppAstKind::argument_list &&
      node.children[1].kind != CppAstKind::paren_argument_list) ||
     node.children[1].children.empty()) {
    return false;
  }

  trait_name = node.children[0].value;
  if(!is_supported_builtin_type_trait_name(trait_name)) {
    trait_name.clear();
    return false;
  }

  for(size_t i = 0; i < node.children[1].children.size(); ++i) {
    std::vector<TypePtr> expanded;
    if(!try_expand_builtin_type_trait_call_arg(
            ctx, scope, node.children[1].children[i], expanded)) {
      trait_name.clear();
      types.clear();
      return false;
    }
    types.insert(types.end(), expanded.begin(), expanded.end());
  }
  return true;
}

bool try_parse_builtin_type_trait_expression(SemanticContext & ctx,
                                             Scope & scope,
                                             const CppAstNode & node,
                                             std::string & trait_name,
                                             std::vector<TypePtr> & types)
{
  trait_name.clear();
  types.clear();

  if(node.kind == CppAstKind::call_expression) {
    return try_parse_builtin_type_trait_call(ctx, scope, node, trait_name, types);
  }

  if(node.kind != CppAstKind::type_trait_expression ||
     !node.has_token ||
     node.token_kind != RT_IDENTIFIER ||
     !is_supported_builtin_type_trait_name(node.value) ||
     node.children.empty()) {
    return false;
  }

  trait_name = node.value;
  for(size_t i = 0; i < node.children.size(); ++i) {
    std::vector<TypePtr> expanded;
    if(!try_expand_builtin_type_trait_call_arg(ctx, scope, node.children[i], expanded)) {
      trait_name.clear();
      types.clear();
      return false;
    }
    types.insert(types.end(), expanded.begin(), expanded.end());
  }
  return true;
}

}  // namespace semantic_builtins
