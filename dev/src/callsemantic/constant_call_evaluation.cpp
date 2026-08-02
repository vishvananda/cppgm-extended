#include "callsemantic/constant_call_evaluation.h"

#include "callsemantic_internal.h"
#include "constexpr_eval.h"
#include "semantic_context.h"
#include "semantic_context_facets.h"
#include "semantic_builtins.h"
#include "semantic_consteval.h"
#include "semantic_conversion.h"
#include "semantic_lookup.h"
#include "semantic_template_function.h"
#include "semantic_utils.h"
#include "template_scope.h"
#include "types.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace callsemantic {

using callsemantic_internal::find_child_kind;
using cpp_decl::TemplateIdSyntax;
using cpp_decl::Type;
using cpp_decl::TypePtr;
using cpp_decl::describe_type;
using cpp_decl::is_integral_type;
using cpp_decl::make_fundamental;
using cpp_decl::remove_reference_type;
using cpp_decl::strip_top_level_cv;
using cpp_decl::type_equals;
using cpp_decl::type_size;
using semantic_conversion::ExprInfo;
using semantic_conversion::VC_LVALUE;
using semantic_conversion::VC_PRVALUE;
using semantic_conversion::can_copy_initialize;
using semantic_lookup::MemberValueLookupResult;
using semantic_lookup::lookup_member_value;
using semantic_lookup::scope_qualified_name;
using semantic_model::BCEK_EXPECT;
using semantic_model::ClassInfo;
using semantic_model::FunctionBinding;
using semantic_model::FunctionTemplateDecl;
using semantic_model::Scope;
using semantic_model::ValueBinding;

namespace {

const CppAstNode * constexpr_function_body(SemanticContext & ctx,
                                           const FunctionBinding & binding)
{
  if(!binding.body) {
    return nullptr;
  }
  return ctx.materialize_lazy_function_body(*binding.body);
}

FunctionBinding * ensure_constexpr_function_definition(SemanticContext & ctx,
                                                       FunctionBinding * binding,
                                                       Scope & use_scope)
{
  if(!binding) {
    return nullptr;
  }
  if(binding->owner_class && binding->owner_class->is_explicit_specialization) {
    return binding;
  }
  binding = semantic_template_function::acquire_function_definition_binding(
      ctx,
      binding,
      use_scope);
  if(!binding) {
    return nullptr;
  }
  semantic_template_function::note_ensured_function_definition_materialized_by_lifecycle(
      ctx,
      binding);
  return binding;
}

bool bind_constexpr_function_parameter_value_packs(
    Scope & scope,
    const FunctionBinding & binding,
    const std::vector<constant_eval::ConstexprValue> & explicit_args,
    const std::vector<std::pair<std::string, TypePtr> > & explicit_params)
{
  if(!binding.source_template ||
     !binding.source_template->has_trailing_function_parameter_pack ||
     binding.source_template->params_pattern.empty()) {
    return false;
  }

  const FunctionTemplateDecl & source = *binding.source_template;
  const std::size_t fixed_count = source.params_pattern.size() - 1;
  if(explicit_args.size() < fixed_count ||
     explicit_params.size() < fixed_count) {
    return false;
  }

  const std::string pack_name =
      function_template_parameter_alias_name(source, source.params_pattern.size() - 1);
  if(pack_name.empty()) {
    return false;
  }

  const std::size_t pack_count = explicit_args.size() - fixed_count;
  std::vector<ValueBinding> pack_bindings;
  pack_bindings.reserve(pack_count);
  for(std::size_t i = 0; i < pack_count; ++i) {
    const std::size_t arg_index = fixed_count + i;
    TypePtr value_type = arg_index < explicit_params.size() ?
        explicit_params[arg_index].second :
        explicit_args[arg_index].type;
    if(!value_type) {
      value_type = explicit_args[arg_index].type;
    }

    const std::string alias_name = template_scope::pack_value_alias_name(pack_name, i);
    ValueBinding value(ValueBinding::VK_PARAMETER, alias_name, value_type);
    set_value_binding_constexpr_value(value, explicit_args[arg_index]);
    long long integral_value = 0;
    if(constant_eval::constexpr_value_to_integral(explicit_args[arg_index],
                                                  integral_value)) {
      value.has_constant_value = true;
      value.constant_value = integral_value;
    }
    scope.values[alias_name] = value;
    pack_bindings.push_back(scope.values[alias_name]);
  }
  template_scope::bind_value_pack(scope, pack_name, pack_bindings, true);
  return true;
}

bool constexpr_function_body_contains_pack_expansion(const CppAstNode & node)
{
  if(node.kind == CppAstKind::pack_expansion_expression) {
    return true;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(constexpr_function_body_contains_pack_expansion(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool expand_constexpr_function_body_packs(SemanticContext & ctx,
                                          Scope & scope,
                                          const CppAstNode & node,
                                          CppAstNode & out)
{
  if(node.kind == CppAstKind::pack_expansion_expression) {
    return false;
  }

  out = node;
  std::vector<CppAstNode> children;
  children.reserve(node.children.size());
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::pack_expansion_expression) {
      std::vector<CppAstNode> expanded_nodes;
      if(!ctx.expand_pack_argument_node(scope, child, expanded_nodes)) {
        return false;
      }
      for(std::size_t j = 0; j < expanded_nodes.size(); ++j) {
        CppAstNode expanded_child;
        if(!expand_constexpr_function_body_packs(ctx,
                                                 scope,
                                                 expanded_nodes[j],
                                                 expanded_child)) {
          return false;
        }
        children.push_back(expanded_child);
      }
      continue;
    }

    CppAstNode expanded_child;
    if(!expand_constexpr_function_body_packs(ctx, scope, child, expanded_child)) {
      return false;
    }
    children.push_back(expanded_child);
  }
  out.children.swap(children);
  return true;
}

bool evaluate_method_call_implicit_object(
    constant_eval::Evaluator & evaluator,
    const CppAstNode & callee,
    const FunctionBinding & binding,
    constant_eval::ConstexprValue & out)
{
  if(callee.kind == CppAstKind::member_expression &&
     callee.children.size() == 2) {
    if(callee.children[0].kind == CppAstKind::id_expression &&
       callee.children[0].value == "this") {
      return evaluator.current_this_object(out);
    }
    if(node_has_simple_type(callee, OP_ARROW)) {
      CppAstNode dereference;
      dereference.kind = CppAstKind::unary_expression;
      dereference.has_token = true;
      dereference.token_kind = RT_SIMPLE;
      dereference.simple_type = OP_STAR;
      dereference.value = "*";
      dereference.children.push_back(callee.children[0]);
      return evaluator.eval_expr(dereference, out);
    }
    return evaluator.eval_expr(callee.children[0], out);
  }

  if(callee.kind == CppAstKind::id_expression ||
     callee.kind == CppAstKind::identifier) {
    if(semantic_utils::unqualified_member_name(binding.name) == "operator()" &&
       callee.value != "operator()") {
      return evaluator.eval_expr(callee, out);
    }
    return evaluator.current_this_object(out);
  }

  return evaluator.eval_expr(callee, out);
}

bool constexpr_template_specialization_matches_argument_types(
    const FunctionBinding & binding,
    const std::vector<constant_eval::ConstexprValue> & args)
{
  TypePtr function_type = strip_top_level_cv(binding.type);
  if(!function_type ||
     function_type->kind != Type::TK_FUNCTION ||
     args.size() > function_type->params.size()) {
    return false;
  }
  for(std::size_t i = 0; i < args.size(); ++i) {
    TypePtr parameter_type =
        strip_top_level_cv(remove_reference_type(function_type->params[i]));
    TypePtr argument_type =
        strip_top_level_cv(remove_reference_type(args[i].type));
    if(!parameter_type || !argument_type) {
      return false;
    }
    if(type_equals(parameter_type, argument_type)) {
      continue;
    }
    if((parameter_type->kind == Type::TK_POINTER &&
        argument_type->kind == Type::TK_POINTER) ||
       (parameter_type->kind == Type::TK_MEMBER_POINTER &&
        argument_type->kind == Type::TK_MEMBER_POINTER)) {
      return false;
    }
  }
  return true;
}

}  // namespace

bool evaluate_constant_call_expression_value(
    SemanticContext & ctx,
    const ConstantCallEvaluationState & state,
    const ConstantCallEvaluationCallbacks & callbacks,
    Scope & scope,
    constant_eval::Evaluator & evaluator,
    const CppAstNode & node,
    const std::vector<constant_eval::ConstexprValue> & args,
    constant_eval::ConstexprValue & out)
{
  if(node.kind != CppAstKind::call_expression || node.children.empty()) {
    return false;
  }

  const auto try_evaluate_builtin_trait =
      [&](const std::string & builtin_name,
          const std::vector<TypePtr> & builtin_types) -> bool
      {
        for(std::size_t i = 0; i < builtin_types.size(); ++i) {
          if(ctx.type_depends_on_template_parameter(builtin_types[i])) {
            if(!ctx.scope_has_template_placeholders(scope)) {
              std::ostringstream outmsg;
              outmsg << "builtin type trait remained dependent: " << builtin_name;
              outmsg << " [scope " << scope_qualified_name(scope, "<here>") << "]";
              outmsg << " [bindings "
                     << ctx.describe_scope_bindings_for_diagnostic(scope) << "]";
              outmsg << " [types";
              for(std::size_t j = 0; j < builtin_types.size(); ++j) {
                outmsg << (j == 0 ? " " : ", ") << describe_type(builtin_types[j]);
              }
              outmsg << "]";
              throw std::logic_error(outmsg.str());
            }
            return false;
          }
        }

        long long trait_value = 0;
        if(builtin_types.size() == 1 &&
           ctx.evaluate_builtin_type_trait(scope, builtin_name, builtin_types[0], trait_value)) {
          out = constant_eval::make_integral_value(
              trait_value,
              semantic_builtins::builtin_type_trait_result_type(builtin_name));
          return true;
        }
        if(builtin_types.size() == 2 &&
           ctx.evaluate_builtin_binary_type_trait(scope,
                                                  builtin_name,
                                                  builtin_types[0],
                                                  builtin_types[1],
                                                  trait_value)) {
          out = constant_eval::make_integral_value(
              trait_value,
              semantic_builtins::builtin_type_trait_result_type(builtin_name));
          return true;
        }
        return false;
      };

  std::string builtin_name;
  std::vector<TypePtr> builtin_types;
  if(ctx.try_parse_builtin_type_trait_call(scope, node, builtin_name, builtin_types) &&
     try_evaluate_builtin_trait(builtin_name, builtin_types)) {
    return true;
  }

  const CppAstNode & callee = node.children[0];
  const CppAstNode * argument_list = find_child_kind(node, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(node, CppAstKind::paren_argument_list);
  }
  if(callee.kind == CppAstKind::id_expression && callee.value == "__builtin_offsetof") {
    if(!argument_list || argument_list->children.size() != 2) {
      return false;
    }

    TypePtr object_type;
    if(!ctx.try_parse_builtin_type_trait_call_arg(scope, argument_list->children[0], object_type)) {
      return false;
    }
    if(ctx.type_depends_on_template_parameter(object_type)) {
      return false;
    }

    ClassInfo * info = ctx.complete_class_type(strip_top_level_cv(object_type));
    if(!info || !info->complete) {
      return false;
    }

    const CppAstNode & member = argument_list->children[1];
    if(member.kind != CppAstKind::id_expression) {
      return false;
    }

    MemberValueLookupResult found = lookup_member_value(*info, member.value);
    if(!found.binding || found.binding->kind != ValueBinding::VK_FIELD) {
      return false;
    }
    if(found.binding->is_bit_field) {
      throw std::logic_error("__builtin_offsetof on bit-field unsupported");
    }

    out = constant_eval::make_integral_value(
        static_cast<long long>(found.path_offset + found.binding->field_offset),
        make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression && args.size() == 1) {
    TypePtr target_type = ctx.lookup_type_node(scope, callee, callee.value);
    if(target_type &&
       constant_eval::constexpr_value_cast(args[0], target_type, out)) {
      return true;
    }
  }

  if(callee.kind == CppAstKind::id_expression &&
     (callee.value == "__atomic_always_lock_free" ||
      callee.value == "__atomic_is_lock_free") &&
     args.size() == 2) {
    long long size_value = 0;
    if(constant_eval::constexpr_value_to_integral(args[0], size_value)) {
      const bool lock_free =
          size_value == 1 || size_value == 2 || size_value == 4 || size_value == 8;
      out = constant_eval::make_integral_value(lock_free ? 1 : 0,
                                               make_fundamental(FT_BOOL));
      return true;
    }
  }

  if(callee.kind == CppAstKind::id_expression &&
     callee.value == "__c11_atomic_is_lock_free" &&
     args.size() == 1) {
    long long size_value = 0;
    if(constant_eval::constexpr_value_to_integral(args[0], size_value)) {
      const bool lock_free =
          size_value == 1 || size_value == 2 || size_value == 4 || size_value == 8;
      out = constant_eval::make_integral_value(lock_free ? 1 : 0,
                                               make_fundamental(FT_BOOL));
      return true;
    }
  }

  const auto fixed_width_bit_builtin_type =
      [](const std::string & builtin_name) -> TypePtr
  {
    if(builtin_name == "__builtin_clz" ||
       builtin_name == "__builtin_ctz" ||
       builtin_name == "__builtin_popcount") {
      return make_fundamental(FT_UNSIGNED_INT);
    }
    if(builtin_name == "__builtin_clzl" ||
       builtin_name == "__builtin_ctzl" ||
       builtin_name == "__builtin_popcountl") {
      return make_fundamental(FT_UNSIGNED_LONG_INT);
    }
    if(builtin_name == "__builtin_clzll" ||
       builtin_name == "__builtin_ctzll" ||
       builtin_name == "__builtin_popcountll") {
      return make_fundamental(FT_UNSIGNED_LONG_LONG_INT);
    }
    return TypePtr();
  };

  const auto evaluate_builtin_bit_argument =
      [&](const constant_eval::ConstexprValue & arg,
          const TypePtr & forced_type,
          unsigned long long & value,
          unsigned & bit_count) -> bool
  {
    long long raw_value = 0;
    if(!constant_eval::constexpr_value_to_integral(arg, raw_value)) {
      return false;
    }

    TypePtr value_type = forced_type ? strip_top_level_cv(forced_type)
                                     : strip_top_level_cv(arg.type);
    if(!value_type || !is_integral_type(value_type)) {
      return false;
    }

    bit_count = static_cast<unsigned>(type_size(value_type) * 8);
    if(bit_count == 0 || bit_count > 64) {
      return false;
    }

    value = static_cast<unsigned long long>(raw_value);
    if(bit_count < 64) {
      value &= ((1ULL << bit_count) - 1ULL);
    }
    return true;
  };

  const auto count_leading_zeros =
      [](unsigned long long value, unsigned bit_count) -> unsigned
  {
    return bit_count == 64 ? static_cast<unsigned>(__builtin_clzll(value))
                           : static_cast<unsigned>(__builtin_clzll(value) -
                                                   (64 - bit_count));
  };

  if(callee.kind == CppAstKind::id_expression &&
     (callee.value == "__builtin_clz" ||
      callee.value == "__builtin_clzl" ||
      callee.value == "__builtin_clzll") &&
     args.size() == 1) {
    unsigned long long value = 0;
    unsigned bit_count = 0;
    if(!evaluate_builtin_bit_argument(args[0],
                                      fixed_width_bit_builtin_type(callee.value),
                                      value,
                                      bit_count)) {
      return false;
    }
    if(value == 0ULL) {
      return false;
    }

    const unsigned leading = count_leading_zeros(value, bit_count);
    out = constant_eval::make_integral_value(static_cast<long long>(leading),
                                             make_fundamental(FT_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression &&
     (callee.value == "__builtin_ctz" ||
      callee.value == "__builtin_ctzl" ||
      callee.value == "__builtin_ctzll") &&
     args.size() == 1) {
    unsigned long long value = 0;
    unsigned bit_count = 0;
    if(!evaluate_builtin_bit_argument(args[0],
                                      fixed_width_bit_builtin_type(callee.value),
                                      value,
                                      bit_count)) {
      return false;
    }
    if(value == 0ULL) {
      return false;
    }

    unsigned trailing = 0;
    while((value & 1ULL) == 0ULL) {
      ++trailing;
      value >>= 1;
    }

    out = constant_eval::make_integral_value(static_cast<long long>(trailing),
                                             make_fundamental(FT_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression &&
     (callee.value == "__builtin_popcount" ||
      callee.value == "__builtin_popcountl" ||
      callee.value == "__builtin_popcountll" ||
      callee.value == "__builtin_popcountg") &&
     args.size() == 1) {
    unsigned long long value = 0;
    unsigned bit_count = 0;
    if(!evaluate_builtin_bit_argument(args[0],
                                      fixed_width_bit_builtin_type(callee.value),
                                      value,
                                      bit_count)) {
      return false;
    }
    (void)bit_count;

    unsigned popcount = 0;
    while(value != 0ULL) {
      popcount += static_cast<unsigned>(value & 1ULL);
      value >>= 1;
    }

    out = constant_eval::make_integral_value(static_cast<long long>(popcount),
                                             make_fundamental(FT_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression &&
     (callee.value == "__builtin_bswap16" ||
      callee.value == "__builtin_bswap32" ||
      callee.value == "__builtin_bswap64") &&
     args.size() == 1) {
    long long raw_value = 0;
    if(!constant_eval::constexpr_value_to_integral(args[0], raw_value)) {
      return false;
    }

    if(callee.value == "__builtin_bswap16") {
      const unsigned short value = static_cast<unsigned short>(raw_value);
      const unsigned short swapped =
          static_cast<unsigned short>(((value & 0x00FFu) << 8) |
                                      ((value & 0xFF00u) >> 8));
      out = constant_eval::make_integral_value(
          static_cast<long long>(swapped),
          make_fundamental(FT_UNSIGNED_SHORT_INT));
      return true;
    }

    if(callee.value == "__builtin_bswap32") {
      const unsigned int value = static_cast<unsigned int>(raw_value);
      const unsigned int swapped =
          ((value & 0x000000FFu) << 24) |
          ((value & 0x0000FF00u) << 8) |
          ((value & 0x00FF0000u) >> 8) |
          ((value & 0xFF000000u) >> 24);
      out = constant_eval::make_integral_value(
          static_cast<long long>(swapped),
          make_fundamental(FT_UNSIGNED_INT));
      return true;
    }

    const unsigned long long value = static_cast<unsigned long long>(raw_value);
    const unsigned long long swapped =
        ((value & 0x00000000000000FFULL) << 56) |
        ((value & 0x000000000000FF00ULL) << 40) |
        ((value & 0x0000000000FF0000ULL) << 24) |
        ((value & 0x00000000FF000000ULL) << 8) |
        ((value & 0x000000FF00000000ULL) >> 8) |
        ((value & 0x0000FF0000000000ULL) >> 24) |
        ((value & 0x00FF000000000000ULL) >> 40) |
        ((value & 0xFF00000000000000ULL) >> 56);
    out = constant_eval::make_integral_value(
        static_cast<long long>(swapped),
        make_fundamental(FT_UNSIGNED_LONG_LONG_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression &&
     callee.value == "__builtin_clzg" &&
     (args.size() == 1 || args.size() == 2)) {
    unsigned long long value = 0;
    unsigned bit_count = 0;
    if(!evaluate_builtin_bit_argument(args[0], TypePtr(), value, bit_count)) {
      return false;
    }

    if(value == 0ULL) {
      long long fallback = static_cast<long long>(bit_count);
      if(args.size() == 2 &&
         !constant_eval::constexpr_value_to_integral(args[1], fallback)) {
        return false;
      }
      out = constant_eval::make_integral_value(fallback,
                                               make_fundamental(FT_INT));
      return true;
    }

    const unsigned leading = count_leading_zeros(value, bit_count);
    out = constant_eval::make_integral_value(static_cast<long long>(leading),
                                             make_fundamental(FT_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression &&
     callee.value == "__builtin_ctzg" &&
     (args.size() == 1 || args.size() == 2)) {
    unsigned long long value = 0;
    unsigned bit_count = 0;
    if(!evaluate_builtin_bit_argument(args[0], TypePtr(), value, bit_count)) {
      return false;
    }

    if(value == 0ULL) {
      long long fallback = static_cast<long long>(bit_count);
      if(args.size() == 2 &&
         !constant_eval::constexpr_value_to_integral(args[1], fallback)) {
        return false;
      }
      out = constant_eval::make_integral_value(fallback,
                                               make_fundamental(FT_INT));
      return true;
    }

    unsigned trailing = 0;
    while((value & 1ULL) == 0ULL) {
      ++trailing;
      value >>= 1;
    }
    out = constant_eval::make_integral_value(static_cast<long long>(trailing),
                                             make_fundamental(FT_INT));
    return true;
  }

  if(callee.kind == CppAstKind::id_expression) {
    const TemplateIdSyntax * template_id =
        cppast_template_id_syntax(callee);
    std::vector<FunctionBinding *> candidates =
        template_id ?
            ctx.lookup_function_template_id_node(
                scope,
                callee,
                *template_id,
                semantic_policy::default_call_analysis()) :
            ctx.lookup_functions_node(scope,
                                      callee,
                                      callee.value,
                                      semantic_policy::default_call_analysis());
    if(candidates.empty()) {
      for(std::size_t i = 0; i < state.functions.size(); ++i) {
        if(state.functions[i]->name == callee.value) {
          candidates.push_back(state.functions[i].get());
        }
      }
    }
    std::vector<FunctionBinding *> viable;
    for(std::size_t i = 0; i < candidates.size(); ++i) {
      FunctionBinding * candidate = candidates[i];
      TypePtr function_type = strip_top_level_cv(candidate->type);
      if(!function_type || function_type->kind != Type::TK_FUNCTION ||
         (!template_id &&
         candidate->source_template &&
          !candidate->is_explicit_specialization &&
          !constexpr_template_specialization_matches_argument_types(*candidate,
                                                                     args)) ||
         ctx.type_depends_on_template_parameter(function_type) ||
         candidate->is_method) {
        continue;
      }
      const bool variadic =
          function_type->variadic || function_type->prototype_relaxed;
      if(!variadic && args.size() > candidate->params.size()) {
        continue;
      }

      std::size_t required_args = candidate->params.size();
      while(required_args > 0 &&
            required_args - 1 < candidate->default_arguments.size() &&
            candidate->default_arguments[required_args - 1]) {
        --required_args;
      }
      if(args.size() < required_args) {
        continue;
      }

      bool matches = true;
      const std::size_t matched_arg_count =
          std::min(args.size(), candidate->params.size());
      for(std::size_t arg_index = 0; arg_index < matched_arg_count; ++arg_index) {
        ExprInfo expr;
        expr.type = args[arg_index].type;
        if(!expr.type) {
          expr.type = args[arg_index].kind == constant_eval::ConstexprValue::CV_FLOATING ?
              make_fundamental(FT_DOUBLE) : make_fundamental(FT_INT);
        }
        expr.category =
            (args[arg_index].kind == constant_eval::ConstexprValue::CV_FUNCTION ||
             args[arg_index].kind == constant_eval::ConstexprValue::CV_ADDRESSABLE) ?
                VC_LVALUE :
                VC_PRVALUE;
        long long integral_value = 0;
        expr.null_pointer_constant =
            args[arg_index].kind == constant_eval::ConstexprValue::CV_NULLPTR ||
            (constant_eval::constexpr_value_to_integral(args[arg_index], integral_value) &&
             integral_value == 0);
        if(!can_copy_initialize(ctx, candidate->params[arg_index].second, expr)) {
          matches = false;
          break;
        }
      }
      if(matches) {
        viable.push_back(candidate);
      }
    }

    if(viable.size() == 1) {
      FunctionBinding * binding = viable[0];
      if(binding->builtin_constant_evaluation_kind == BCEK_EXPECT &&
         args.size() == 2) {
        TypePtr function_type = strip_top_level_cv(binding->type);
        return function_type && function_type->kind == Type::TK_FUNCTION &&
               constant_eval::constexpr_value_cast(args[0], function_type->inner, out);
      }
      if(!binding->is_constexpr) {
        return false;
      }
      binding = ensure_constexpr_function_definition(ctx, binding, scope);
      if(!binding) {
        return false;
      }
      if(callbacks.record_constexpr_direct_function_call_source_use) {
        callbacks.record_constexpr_direct_function_call_source_use(
            scope,
            callee,
            *binding,
            template_id,
            template_id ? template_id->arguments.size() : 0);
      }
      Scope & call_scope = binding->declaration_scope ? *binding->declaration_scope : scope;
      Scope constexpr_default_arg_scope =
          semantic_consteval::make_constexpr_call_scope(call_scope, binding, false);
      const constant_eval::Hooks default_arg_hooks =
          semantic_consteval::build_hooks(ctx, constexpr_default_arg_scope);
      Scope constexpr_call_scope =
          semantic_consteval::make_constexpr_call_scope(call_scope, binding);
      const constant_eval::Hooks call_hooks =
          semantic_consteval::build_hooks(ctx, constexpr_call_scope);
      std::vector<constant_eval::ConstexprValue> final_args = args;
      for(std::size_t i = final_args.size(); i < binding->params.size(); ++i) {
        if(i >= binding->default_arguments.size() || !binding->default_arguments[i]) {
          return false;
        }
        const CppAstNode * default_arg = binding->default_arguments[i];
        const CppAstNode * payload =
            default_arg->children.size() == 1 ? &default_arg->children[0] : default_arg;
        constant_eval::ConstexprValue value;
        constant_eval::Evaluator default_evaluator(default_arg_hooks);
        if(!default_evaluator.eval_initializer(*payload, value, binding->params[i].second)) {
          return false;
        }
        final_args.push_back(value);
      }

      constant_eval::FunctionInfo info;
      info.name = binding->name;
      TypePtr function_type = strip_top_level_cv(binding->type);
      info.return_type = function_type && function_type->kind == Type::TK_FUNCTION ?
          function_type->inner : TypePtr();
      info.params = semantic_consteval::constexpr_function_parameters(*binding);
      CppAstNode expanded_body;
      info.body = constexpr_function_body(ctx, *binding);
      const bool body_has_pack_expansion =
          info.body && constexpr_function_body_contains_pack_expansion(*info.body);
      if(body_has_pack_expansion) {
        bind_constexpr_function_parameter_value_packs(
            constexpr_call_scope, *binding, final_args, info.params);
      }
      if(body_has_pack_expansion &&
         expand_constexpr_function_body_packs(ctx,
                                             constexpr_call_scope,
                                             *info.body,
                                             expanded_body)) {
        info.body = &expanded_body;
      }
      info.variadic = function_type && function_type->kind == Type::TK_FUNCTION &&
                      (function_type->variadic || function_type->prototype_relaxed);
      info.is_method = binding->is_method;
      return evaluator.call(info, final_args, out, &call_hooks);
    }
  }

  ExprInfo analyzed;
  try
  {
    analyzed = ctx.analyze_call_expression(scope, node);
  }
  catch(const std::logic_error &)
  {
    return false;
  }
  if(analyzed.node.kind != CallSemKind::call_expression ||
     analyzed.node.children.empty() ||
     !analyzed.node.children[0].semantic_type) {
    return false;
  }

  const CallSemNode & resolved_callee = analyzed.node.children[0];
  FunctionBinding * binding = nullptr;
  if(resolved_callee.kind == CallSemKind::callee) {
    const symbol_linkage::SymbolIdentity & resolved_symbol =
        callsem_symbol(resolved_callee);
    if(!resolved_symbol.internal_symbol.empty()) {
      binding = ctx.first_function_by_internal_symbol(resolved_symbol.internal_symbol);
    }
    if(!resolved_symbol.object_symbol.empty() &&
       (!binding || binding->symbol.object_symbol != resolved_symbol.object_symbol)) {
      binding = ctx.first_function_by_object_symbol(resolved_symbol.object_symbol);
    }
    for(std::size_t i = 0; i < state.functions.size(); ++i) {
      const bool legacy_match =
          state.functions[i]->name == resolved_callee.text &&
          type_equals(state.functions[i]->type, resolved_callee.semantic_type);
      if(!binding && legacy_match) {
        binding = state.functions[i].get();
        break;
      }
    }
  }
  if(!binding) {
    constant_eval::ConstexprValue callable;
    if(evaluator.eval_expr(callee, callable) &&
       (callable.kind == constant_eval::ConstexprValue::CV_POINTER ||
        callable.kind == constant_eval::ConstexprValue::CV_FUNCTION) &&
       !callable.storage_identity.empty()) {
      binding = ctx.first_function_by_object_symbol(callable.storage_identity);
      if(!binding) {
        binding = ctx.first_function_by_internal_symbol(callable.storage_identity);
      }
    }
  }
  if(!binding) {
    return false;
  }
  if(!binding->is_constexpr) {
    return false;
  }
  binding = ensure_constexpr_function_definition(ctx, binding, scope);
  if(!binding) {
    return false;
  }

  Scope & call_scope = binding->declaration_scope ? *binding->declaration_scope : scope;
  Scope constexpr_default_arg_scope =
      semantic_consteval::make_constexpr_call_scope(call_scope, binding, false);
  const constant_eval::Hooks default_arg_hooks =
      semantic_consteval::build_hooks(ctx, constexpr_default_arg_scope);
  Scope constexpr_call_scope =
      semantic_consteval::make_constexpr_call_scope(call_scope, binding);
  const constant_eval::Hooks call_hooks =
      semantic_consteval::build_hooks(ctx, constexpr_call_scope);
  std::vector<constant_eval::ConstexprValue> final_args = args;
  const std::size_t explicit_param_offset = binding->is_method ? 1u : 0u;
  for(std::size_t i = final_args.size() + explicit_param_offset;
      i < binding->params.size();
      ++i) {
    if(i >= binding->default_arguments.size() || !binding->default_arguments[i]) {
      return false;
    }
    const CppAstNode * default_arg = binding->default_arguments[i];
    const CppAstNode * payload =
        default_arg->children.size() == 1 ? &default_arg->children[0] : default_arg;
    constant_eval::ConstexprValue value;
    constant_eval::Evaluator default_evaluator(default_arg_hooks);
    if(!default_evaluator.eval_initializer(*payload, value, binding->params[i].second)) {
      return false;
    }
    final_args.push_back(value);
  }

  constant_eval::FunctionInfo info;
  info.name = binding->name;
  TypePtr function_type = strip_top_level_cv(binding->type);
  info.return_type = function_type && function_type->kind == Type::TK_FUNCTION ?
      function_type->inner : TypePtr();
  const std::vector<std::pair<std::string, TypePtr> > constexpr_params =
      semantic_consteval::constexpr_function_parameters(*binding);
  info.params.assign(constexpr_params.begin() + explicit_param_offset,
                     constexpr_params.end());
  CppAstNode expanded_body;
  info.body = constexpr_function_body(ctx, *binding);
  const bool body_has_pack_expansion =
      info.body && constexpr_function_body_contains_pack_expansion(*info.body);
  if(body_has_pack_expansion) {
    bind_constexpr_function_parameter_value_packs(
        constexpr_call_scope, *binding, final_args, info.params);
  }
  if(body_has_pack_expansion &&
     expand_constexpr_function_body_packs(ctx,
                                         constexpr_call_scope,
                                         *info.body,
                                         expanded_body)) {
    info.body = &expanded_body;
  }
  info.variadic = function_type && function_type->kind == Type::TK_FUNCTION &&
                  (function_type->variadic || function_type->prototype_relaxed);
  info.is_method = binding->is_method;
  if(binding->is_method) {
    constant_eval::ConstexprValue implicit_object;
    if(!evaluate_method_call_implicit_object(evaluator,
                                             callee,
                                             *binding,
                                             implicit_object)) {
      return false;
    }
    info.has_implicit_object = true;
    info.implicit_object = implicit_object;
  }
  return evaluator.call(info, final_args, out, &call_hooks);
}

}  // namespace callsemantic
