#include "semantic_lifetime.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <sstream>
#include <stdexcept>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "callsem_output.h"
#include "constructor_lifecycle_service.h"
#include "cppast_dump.h"
#include "cpp_decl_bridge.h"
#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_conversion.h"
#include "semantic_errors.h"
#include "semantic_lookup.h"
#include "semantic_model.h"
#include "semantic_overload.h"
#include "semantic_template_function.h"
#include "template_api.h"
#include "template_witness.h"
#include "types.h"
#include "witness_api.h"

using namespace std;

namespace semantic_lifetime {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using DumpNode = CallSemNode;

namespace {

std::string callsem_node_source_location_text(const CallSemNode & node)
{
  if(!node.has_source_location()) {
    return std::string();
  }
  std::ostringstream out;
  out << callsem_source_file(node) << ":"
      << callsem_source_line(node) << ":"
      << callsem_source_column(node);
  return out.str();
}

bool constructor_witness_source_capture_enabled(SemanticContext & ctx)
{
  return witness::function_call_source_capture_enabled() &&
         witness::source_capture_enabled(ctx.template_witness_context());
}

std::string source_location_for_identifier_before_on_same_line(
    SemanticContext & ctx,
    const std::string & before_location,
    const std::string & identifier)
{
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  if(identifier.empty() ||
     !(witness_ctx.token_sequence && witness_ctx.source_locations)) {
    return std::string();
  }
  const template_api::template_witness_detail::ParsedSourceLocation before =
      template_api::template_witness_detail::parse_source_location(
          template_api::normalize_template_witness_source_location(
              before_location));
  if(!before.valid) {
    return std::string();
  }

  uint32_t best_location_id = 0;
  int best_column = -1;
  const std::size_t token_count = witness_ctx.token_sequence->size();
  for(std::size_t i = 0; i < token_count; ++i) {
    const RecogToken & token = witness_ctx.token_sequence->peek(i);
    if(token.is_eof()) {
      break;
    }
    if(!token.is_identifier() ||
       token.source != identifier ||
       token.location_id == 0 ||
       token.location_id >= witness_ctx.source_locations->locations.size()) {
      continue;
    }
    const SourceLocation & location =
        witness_ctx.source_locations->locations[token.location_id];
    if(location.file_index >= witness_ctx.source_locations->files.size() ||
       witness_ctx.source_locations->files[location.file_index] != before.file ||
       static_cast<int>(location.line) != before.line ||
       static_cast<int>(location.column) >= before.column ||
       static_cast<int>(location.column) <= best_column) {
      continue;
    }
    best_location_id = token.location_id;
    best_column = static_cast<int>(location.column);
  }
  const std::string raw =
      template_api::template_witness_detail::source_location_for_location_id_raw(
          *witness_ctx.source_locations,
          best_location_id);
  return raw.empty() ? std::string() : std::string(" at ") + raw;
}

std::string prefer_later_source_location_text(const std::string & first,
                                              const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }
  const std::size_t first_last = first.rfind(':');
  const std::size_t first_mid =
      first_last == std::string::npos ? std::string::npos : first.rfind(':', first_last - 1);
  const std::size_t second_last = second.rfind(':');
  const std::size_t second_mid =
      second_last == std::string::npos ? std::string::npos : second.rfind(':', second_last - 1);
  if(first_mid == std::string::npos || first_last == std::string::npos ||
     second_mid == std::string::npos || second_last == std::string::npos) {
    return first;
  }
  const std::string first_file = first.substr(0, first_mid);
  const std::string second_file = second.substr(0, second_mid);
  if(first_file != second_file) {
    return first;
  }
  const int first_line =
      std::atoi(first.substr(first_mid + 1, first_last - first_mid - 1).c_str());
  const int first_column = std::atoi(first.substr(first_last + 1).c_str());
  const int second_line =
      std::atoi(second.substr(second_mid + 1, second_last - second_mid - 1).c_str());
  const int second_column = std::atoi(second.substr(second_last + 1).c_str());
  if(second_line > first_line) {
    return second;
  }
  if(second_line < first_line) {
    return first;
  }
  return second_column >= first_column ? second : first;
}

std::string prefer_earlier_source_location_text(const std::string & first,
                                                const std::string & second)
{
  if(first.empty()) {
    return second;
  }
  if(second.empty()) {
    return first;
  }
  const std::size_t first_last = first.rfind(':');
  const std::size_t first_mid =
      first_last == std::string::npos ? std::string::npos : first.rfind(':', first_last - 1);
  const std::size_t second_last = second.rfind(':');
  const std::size_t second_mid =
      second_last == std::string::npos ? std::string::npos : second.rfind(':', second_last - 1);
  if(first_mid == std::string::npos || first_last == std::string::npos ||
     second_mid == std::string::npos || second_last == std::string::npos) {
    return first;
  }
  const std::string first_file = first.substr(0, first_mid);
  const std::string second_file = second.substr(0, second_mid);
  if(first_file != second_file) {
    return first;
  }
  const int first_line =
      std::atoi(first.substr(first_mid + 1, first_last - first_mid - 1).c_str());
  const int first_column = std::atoi(first.substr(first_last + 1).c_str());
  const int second_line =
      std::atoi(second.substr(second_mid + 1, second_last - second_mid - 1).c_str());
  const int second_column = std::atoi(second.substr(second_last + 1).c_str());
  if(second_line < first_line) {
    return second;
  }
  if(second_line > first_line) {
    return first;
  }
  return second_column < first_column ? second : first;
}

std::string location_with_column_delta(const std::string & location,
                                       int column_delta)
{
  const std::size_t last = location.rfind(':');
  const std::size_t mid =
      last == std::string::npos ? std::string::npos : location.rfind(':', last - 1);
  if(mid == std::string::npos || last == std::string::npos) {
    return location;
  }
  const int column = std::atoi(location.substr(last + 1).c_str());
  const int adjusted = column + column_delta;
  if(adjusted <= 0) {
    return location;
  }
  return location.substr(0, last + 1) + std::to_string(adjusted);
}

std::string literal_start_source_location(SemanticContext & ctx,
                                          const CppAstNode & node)
{
  if((node.kind != CppAstKind::literal &&
      node.kind != CppAstKind::keyword_literal) ||
     node.value.empty()) {
    return std::string();
  }
  const std::string token_end_location = ctx.source_location_for_node(node);
  if(token_end_location.empty()) {
    return std::string();
  }
  const template_api::TemplateWitnessContext witness_ctx =
      ctx.template_witness_context();
  if(witness_ctx.token_sequence && witness_ctx.source_locations) {
    const template_api::template_witness_detail::ParsedSourceLocation parsed =
        template_api::template_witness_detail::parse_source_location(
            template_api::normalize_template_witness_source_location(
                token_end_location));
    if(parsed.valid) {
      int best_column = -1;
      uint32_t best_location_id = 0;
      const size_t token_count = witness_ctx.token_sequence->size();
      for(size_t i = 0; i < token_count; ++i) {
        const RecogToken & token = witness_ctx.token_sequence->peek(i);
        if(token.is_eof()) {
          break;
        }
        if(token.source != node.value || token.location_id == 0 ||
           token.location_id >= witness_ctx.source_locations->locations.size()) {
          continue;
        }
        const SourceLocation & location =
            witness_ctx.source_locations->locations[token.location_id];
        if(location.file_index >= witness_ctx.source_locations->files.size() ||
           witness_ctx.source_locations->files[location.file_index] != parsed.file ||
           static_cast<int>(location.line) != parsed.line ||
           static_cast<int>(location.column) > parsed.column ||
           static_cast<int>(location.column) <= best_column) {
          continue;
        }
        best_column = static_cast<int>(location.column);
        best_location_id = token.location_id;
      }
      if(best_location_id != 0) {
        const std::string token_location =
            template_api::template_witness_detail::
                source_location_for_location_id(witness_ctx, best_location_id);
        return node.kind == CppAstKind::literal ?
            location_with_column_delta(
                token_location,
                -static_cast<int>(node.value.size())) :
            token_location;
      }
    }
  }
  return location_with_column_delta(
      token_end_location,
      -static_cast<int>(node.value.size()));
}

std::string earliest_source_location_for_node(SemanticContext & ctx,
                                              const CppAstNode & node)
{
  const std::string literal_start = literal_start_source_location(ctx, node);
  if(!literal_start.empty()) {
    return literal_start;
  }
  const std::string start_location =
      node.token_end > node.token_start ?
          template_api::template_witness_detail::source_location_for_token_index(
              ctx.template_witness_context(),
              node.token_start) :
          std::string();
  if(!start_location.empty()) {
    return start_location;
  }
  const std::string source_id_location =
      template_api::template_witness_detail::source_location_for_ast_node_start(
          ctx.template_witness_context(),
          node);
  if(!source_id_location.empty()) {
    return source_id_location;
  }
  std::string best = ctx.source_location_for_node(node);
  for(size_t i = 0; i < node.children.size(); ++i) {
    best = prefer_earlier_source_location_text(
        best,
        earliest_source_location_for_node(ctx, node.children[i]));
  }
  return best;
}

bool is_trivial_constructor_binding(SemanticContext & ctx, FunctionBinding & binding);
FunctionBinding * copy_constructor_for(ClassInfo & info);
FunctionBinding * move_constructor_for(ClassInfo & info);

void note_skipped_template_lifecycle_definition(SemanticContext & ctx,
                                                FunctionBinding * binding)
{
  semantic_template_function::note_required_function_definition_materialized_by_lifecycle(
      ctx,
      binding);
}

void note_constructor_witness_closure_impl(SemanticContext & ctx,
                                           FunctionBinding * ctor)
{
  if(!ctor || is_trivial_constructor_binding(ctx, *ctor)) {
    return;
  }
  semantic_template_function::note_ensured_function_definition_materialized_by_lifecycle(
      ctx,
      ctor);
}

struct LeadingTrivialStoragePrefix
{
  size_t field_count = 0;
  size_t byte_count = 0;

  bool empty() const
  {
    return field_count == 0 || byte_count == 0;
  }
};

CallValueCategory to_call_value_category(ValueCategory category)
{
  switch(category) {
  case VC_LVALUE: return CVC_LVALUE;
  case VC_PRVALUE: return CVC_PRVALUE;
  case VC_XVALUE: return CVC_XVALUE;
  }

  throw logic_error("unknown value category");
}

void set_expr_metadata(DumpNode & node,
                       const TypePtr & type,
                       ValueCategory category)
{
  node.semantic_type = type;
  node.value_category = to_call_value_category(category);
}

CppAstNode synthetic_identifier_node(const string & name)
{
  CppAstNode id;
  id.kind = CppAstKind::id_expression;
  id.value = name;
  return id;
}

ExprInfo analyze_generated_this_expr(SemanticContext & ctx,
                                     Scope & scope)
{
  Scope * function_scope = nullptr;
  ClassInfo * function_class = nullptr;
  bool saw_member_function_scope = false;
  for(Scope * current = &scope; current; current = current->parent) {
    if(!current->function) {
      continue;
    }
    ClassInfo * current_class =
        current->class_info ? current->class_info : current->function->owner_class;
    if(!current_class) {
      continue;
    }
    saw_member_function_scope = true;
    if(current_class->is_lambda_closure) {
      function_scope = current;
      function_class = current_class;
      break;
    }
    map<string, ValueBinding>::iterator found = current->values.find("this");
    if(found != current->values.end()) {
      function_scope = current;
      function_class = current_class;
      break;
    }
  }

  if(!saw_member_function_scope) {
    throw logic_error("this outside member function");
  }
  if(!function_scope || !function_class) {
    throw logic_error("missing this binding");
  }

  map<string, ValueBinding>::iterator found = function_scope->values.find("this");
  if(found == function_scope->values.end()) {
    throw logic_error("missing this binding");
  }

  ExprInfo result;
  result.type = found->second.type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::id_expression, "this");
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

bool braced_scalar_initialization_has_narrowing_conversion(SemanticContext & ctx,
                                                           Scope & scope,
                                                           const CppAstNode & init,
                                                           const TypePtr & target_type,
                                                           ExprInfo & source_expr)
{
  source_expr = ctx.analyze_expression(scope, init);
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
  TypePtr source_base = strip_top_level_cv(remove_reference_type(source_expr.type));
  if(!target_base || !source_base) {
    return false;
  }

  // C++11 list-initialization forbids narrowing scalar floating-to-integral
  // conversions, so reject them in semantic initialization rather than letting
  // lowering trip over an already-ill-formed initializer.
  return is_integral_type(target_base) && is_floating_type(source_base);
}

logic_error make_narrowing_initializer_error(const TypePtr & target_type,
                                             const ExprInfo & source_expr,
                                             const CppAstNode & init)
{
  ostringstream outmsg;
  outmsg << "narrowing conversion in list-initialization";
  outmsg << " [target " << describe_type(target_type) << "]";
  outmsg << " [source " << describe_type(source_expr.type) << "]";
  outmsg << " [init " << node_text(init) << "]";
  return logic_error(outmsg.str());
}

ExprInfo analyze_initializer_expression_for_target(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const CppAstNode & init,
                                                   const TypePtr & target_type,
                                                   bool allow_explicit_conversion)
{
  ExprInfo result = ctx.analyze_expression_for_target(scope, init, target_type);
  if(!allow_explicit_conversion || can_copy_initialize(ctx, target_type, result)) {
    return result;
  }

  ExprInfo converted;
  ConversionRank rank = CR_BAD;
  if(ctx.try_argument_conversion(scope,
                                 target_type,
                                 result,
                                 converted,
                                 rank,
                                 semantic_policy::allow_explicit_argument_conversion())) {
    return converted;
  }
  return result;
}

std::string synthetic_parameter_name(const FunctionBinding & binding, std::size_t index)
{
  return function_parameter_binding_name(binding, index);
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

bool binding_is_generated_copy_constructor(const ClassInfo & info,
                                           const FunctionBinding & binding)
{
  return binding.is_constructor &&
         binding.params.size() == 2 &&
         (binding.synthesized || binding.is_defaulted) &&
         is_same_class_reference_parameter(info.type,
                                           binding.params[1].second,
                                           Type::TK_LVALUE_REFERENCE);
}

bool binding_is_generated_move_constructor(const ClassInfo & info,
                                           const FunctionBinding & binding)
{
  return binding.is_constructor &&
         binding.params.size() == 2 &&
         (binding.synthesized || binding.is_defaulted) &&
         is_same_class_reference_parameter(info.type,
                                           binding.params[1].second,
                                           Type::TK_RVALUE_REFERENCE);
}

bool binding_is_generated_copy_assignment(const ClassInfo & info,
                                          const FunctionBinding & binding)
{
  return !binding.is_constructor &&
         !binding.is_destructor &&
         binding.display_name == "operator=" &&
         binding.params.size() == 2 &&
         (binding.synthesized || binding.is_defaulted) &&
         is_same_class_reference_parameter(info.type,
                                           binding.params[1].second,
                                           Type::TK_LVALUE_REFERENCE);
}

bool binding_is_generated_move_assignment(const ClassInfo & info,
                                          const FunctionBinding & binding)
{
  return !binding.is_constructor &&
         !binding.is_destructor &&
         binding.display_name == "operator=" &&
         binding.params.size() == 2 &&
         (binding.synthesized || binding.is_defaulted) &&
         is_same_class_reference_parameter(info.type,
                                           binding.params[1].second,
                                           Type::TK_RVALUE_REFERENCE);
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

bool is_function_local_class_info(const ClassInfo & info)
{
  return !info.is_lambda_closure &&
         info.class_kind != "union" &&
         info.enclosing_scope &&
         semantic_lookup::current_function_scope(*info.enclosing_scope) != nullptr;
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
  if(semantic_lookup::is_named_enum_type(ctx, base)) {
    return true;
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
  if(!info->complete || has_user_declared_destructor(*info)) {
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

bool is_trivially_copy_constructible_type(SemanticContext & ctx, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_copy_constructible_type(ctx, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(semantic_lookup::is_named_enum_type(ctx, base)) {
    return true;
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
  return semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(ctx, base);
}

bool is_trivial_constructor_binding(SemanticContext & ctx, FunctionBinding & binding)
{
  if(!binding.owner_class) {
    return false;
  }
  ClassInfo & info = *binding.owner_class;
  const bool definition_from_standard_include =
      ctx.definition_comes_from_standard_include_path(binding.definition_node ?
                                                          binding.definition_node :
                                                          binding.declaration_node,
                                                      binding.body,
                                                      binding.is_defaulted);
  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  const bool default_like =
      binding.is_constructor &&
      binding.params.size() == 1 &&
      implicit_like;
  const bool copy_like =
      binding.is_constructor &&
      binding.params.size() == 2 &&
      is_same_class_reference_parameter(info.type,
                                        binding.params[1].second,
                                        Type::TK_LVALUE_REFERENCE);
  const bool move_like =
      binding.is_constructor &&
      binding.params.size() == 2 &&
      is_same_class_reference_parameter(info.type,
                                        binding.params[1].second,
                                        Type::TK_RVALUE_REFERENCE);

  if(is_function_local_class_info(info)) {
    if(default_like && implicit_like) {
      return semantic_class_model::is_trivially_default_constructible_type_for_host_abi(ctx,
                                                                                        info.type);
    }
    if(copy_like && implicit_like) {
      return semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(ctx,
                                                                                     info.type);
    }
    if(move_like && implicit_like) {
      return semantic_class_model::is_trivially_move_constructible_type_for_host_abi(ctx,
                                                                                     info.type);
    }
    return false;
  }

  if(default_like) {
    return semantic_class_model::is_trivially_default_constructible_type_for_host_abi(ctx,
                                                                                      info.type);
  }
  if(copy_like &&
     (implicit_like || definition_from_standard_include)) {
    return semantic_class_model::is_trivially_copy_constructible_type_for_host_abi(ctx,
                                                                                   info.type);
  }
  if(move_like &&
     implicit_like) {
    return semantic_class_model::is_trivially_move_constructible_type_for_host_abi(ctx,
                                                                                   info.type);
  }
  return false;
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
  if(semantic_lookup::is_named_enum_type(ctx, base)) {
    return true;
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

bool can_skip_destructor_action_for_host_abi(SemanticContext & ctx, const ClassInfo & info)
{
  if(is_function_local_class_info(info) &&
     is_trivially_destructible_type(ctx, info.type)) {
    return true;
  }
  return semantic_class_model::is_trivially_destructible_type_for_host_abi(ctx, info.type);
}

ExprInfo make_xvalue_expr(const ExprInfo & expr)
{
  ExprInfo result = expr;
  result.category = VC_XVALUE;
  result.node.value_category = CVC_XVALUE;
  return result;
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

vector<const CppAstNode *> initializer_argument_nodes(const CppAstNode & node)
{
  vector<const CppAstNode *> args;
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return initializer_argument_nodes(node.children[0]);
  }
  if(node.kind == CppAstKind::paren_initializer ||
     node.kind == CppAstKind::paren_argument_list ||
     node.kind == CppAstKind::braced_init_list) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      args.push_back(&node.children[i]);
    }
    return args;
  }
  args.push_back(&node);
  return args;
}

vector<const CppAstNode *> expand_initializer_argument_nodes(
    SemanticContext & ctx,
    Scope & scope,
    const vector<const CppAstNode *> & raw_args,
    std::deque<CppAstNode> & synthesized_nodes)
{
  vector<const CppAstNode *> out;
  for(size_t i = 0; i < raw_args.size(); ++i) {
    const CppAstNode * arg = raw_args[i];
    if(!arg || arg->kind != CppAstKind::pack_expansion_expression) {
      out.push_back(arg);
      continue;
    }

    vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, *arg, expanded_nodes)) {
      throw logic_error("initializer pack expansion parse mismatch");
    }
    for(size_t j = 0; j < expanded_nodes.size(); ++j) {
      synthesized_nodes.push_back(expanded_nodes[j]);
      out.push_back(&synthesized_nodes.back());
    }
  }
  return out;
}

bool braced_init_list_has_pack_expansion(const CppAstNode & node)
{
  if(node.kind != CppAstKind::braced_init_list) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::pack_expansion_expression) {
      return true;
    }
  }
  return false;
}

const CppAstNode * braced_init_list_with_expanded_packs(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const CppAstNode & node,
                                                        CppAstNode & expanded)
{
  if(!braced_init_list_has_pack_expansion(node)) {
    return &node;
  }

  std::deque<CppAstNode> synthesized_nodes;
  const vector<const CppAstNode *> args =
      expand_initializer_argument_nodes(ctx,
                                        scope,
                                        initializer_argument_nodes(node),
                                        synthesized_nodes);
  expanded = node;
  expanded.children.clear();
  expanded.children.reserve(args.size());
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i]) {
      expanded.children.push_back(*args[i]);
    }
  }
  return &expanded;
}

bool is_designated_initializer_node(const CppAstNode & node)
{
  return node.kind == CppAstKind::designated_initializer &&
         !node.children.empty();
}

bool braced_init_list_has_designators(const CppAstNode & node)
{
  if(node.kind != CppAstKind::braced_init_list) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(is_designated_initializer_node(node.children[i])) {
      return true;
    }
  }
  return false;
}

const CppAstNode * designated_initializer_payload(const CppAstNode & node)
{
  return is_designated_initializer_node(node) ? &node.children.back() : nullptr;
}

bool designator_field_name(const CppAstNode & node, string & out)
{
  if(node.kind != CppAstKind::designator ||
     !node_has_simple_type(node, OP_DOT) ||
     node.children.size() != 1 ||
     node.children[0].kind != CppAstKind::identifier) {
    return false;
  }
  out = node.children[0].value;
  return !out.empty();
}

bool designator_array_index(SemanticContext & ctx,
                            Scope & scope,
                            const CppAstNode & node,
                            size_t & out)
{
  if(node.kind != CppAstKind::designator ||
     !node_has_simple_type(node, OP_LSQUARE) ||
     node.children.size() != 1) {
    return false;
  }

  long long value = 0;
  if(!ctx.evaluate_constant_expression(scope, node.children[0], value) || value < 0) {
    return false;
  }
  out = static_cast<size_t>(value);
  return true;
}

const CppAstNode * synthesize_nested_designated_braced_init(const CppAstNode & node,
                                                            size_t first_designator,
                                                            vector<CppAstNode> & storage)
{
  if(!is_designated_initializer_node(node) ||
     first_designator >= node.children.size() - 1) {
    return designated_initializer_payload(node);
  }

  CppAstNode nested = node;
  nested.children.erase(nested.children.begin(),
                        nested.children.begin() + static_cast<std::ptrdiff_t>(first_designator));

  CppAstNode braces;
  braces.kind = CppAstKind::braced_init_list;
  braces.children.push_back(nested);
  storage.push_back(braces);
  return &storage.back();
}

bool resolve_aggregate_designated_initializer(SemanticContext & ctx,
                                              Scope & scope,
                                              const ClassInfo & info,
                                              const CppAstNode & node,
                                              size_t & field_index_out,
                                              const CppAstNode *& initializer_out,
                                              vector<CppAstNode> & synthesized_nodes)
{
  if(!is_designated_initializer_node(node)) {
    return false;
  }

  string field_name;
  if(!designator_field_name(node.children[0], field_name)) {
    return false;
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    const FieldInfo & field = info.fields[i];
    if(field.name == field_name) {
      field_index_out = i;
      initializer_out =
          synthesize_nested_designated_braced_init(node, 1, synthesized_nodes);
      return true;
    }
    if(!field.is_anonymous_storage) {
      continue;
    }

    ClassInfo * storage_info = ctx.class_info_for_type(field.type);
    if(!storage_info || !storage_info->member_scope) {
      continue;
    }
    map<string, ValueBinding>::const_iterator found =
        storage_info->member_scope->values.find(field_name);
    if(found == storage_info->member_scope->values.end() ||
       found->second.kind != ValueBinding::VK_FIELD) {
      continue;
    }

    field_index_out = i;
    initializer_out =
        synthesize_nested_designated_braced_init(node, 0, synthesized_nodes);
    return true;
  }

  return false;
}

bool build_array_initializer_plan(SemanticContext & ctx,
                                  Scope & scope,
                                  const TypePtr & array_type,
                                  const CppAstNode & payload,
                                  vector<const CppAstNode *> & element_initializers,
                                  vector<CppAstNode> & synthesized_nodes)
{
  TypePtr base = strip_top_level_cv(array_type);
  if(!base || base->kind != Type::TK_ARRAY || !base->has_bound ||
     payload.kind != CppAstKind::braced_init_list) {
    return false;
  }

  const size_t bound = base->bound;
  element_initializers.assign(bound, nullptr);
  size_t next_index = 0;

  for(size_t i = 0; i < payload.children.size(); ++i) {
    const CppAstNode & child = payload.children[i];
    if(is_designated_initializer_node(child)) {
      size_t index = 0;
      if(!designator_array_index(ctx, scope, child.children[0], index) || index >= bound) {
        return false;
      }
      element_initializers[index] =
          synthesize_nested_designated_braced_init(child, 1, synthesized_nodes);
      next_index = index + 1;
      continue;
    }

    while(next_index < bound && element_initializers[next_index]) {
      ++next_index;
    }
    if(next_index >= bound) {
      return false;
    }
    element_initializers[next_index++] = &child;
  }

  return true;
}

const CppAstNode * synthesize_braced_init_span(const CppAstNode & payload,
                                               size_t begin,
                                               size_t count,
                                               vector<CppAstNode> & synthesized_nodes)
{
  CppAstNode braces;
  braces.kind = CppAstKind::braced_init_list;
  braces.children.insert(braces.children.end(),
                         payload.children.begin() + static_cast<std::ptrdiff_t>(begin),
                         payload.children.begin() + static_cast<std::ptrdiff_t>(begin + count));
  synthesized_nodes.push_back(braces);
  return &synthesized_nodes.back();
}

bool array_accepts_string_literal(const TypePtr & array_type,
                                  EFundamentalType literal_element_type);

bool string_literal_directly_initializes_array(const TypePtr & type,
                                               const CppAstNode & child)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_ARRAY || !base->has_bound ||
     child.kind != CppAstKind::literal) {
    return false;
  }

  QuoteLiteralData literal;
  try
  {
    literal = parse_quote_literal(child.value);
  }
  catch(const logic_error &)
  {
    return false;
  }

  if(literal.quote != '"' || !literal.ud_suffix.empty() ||
     !array_accepts_string_literal(base, string_literal_element_type(literal))) {
    return false;
  }

  return quote_literal_string_unit_count(literal) + 1 <= base->bound;
}

bool aggregate_field_can_use_brace_elision(SemanticContext & ctx,
                                           const FieldInfo & field)
{
  const FieldInfo * input_field =
      semantic_class_model::aggregate_input_field(ctx, field);
  TypePtr field_type = strip_top_level_cv(input_field ? input_field->type : field.type);
  if(!field_type) {
    return false;
  }
  if(field_type->kind == Type::TK_ARRAY && field_type->has_bound) {
    return true;
  }

  ClassInfo * nested_info = ctx.class_info_for_type(field_type);
  return nested_info && ctx.can_synthesize_aggregate_constructor(*nested_info);
}

size_t aggregate_brace_elision_capacity(SemanticContext & ctx,
                                        const FieldInfo & field)
{
  TypePtr field_type = strip_top_level_cv(field.type);
  if(!field_type) {
    return 1;
  }

  if(field_type->kind == Type::TK_ARRAY && field_type->has_bound) {
    return field_type->bound;
  }

  ClassInfo * nested_info = ctx.class_info_for_type(field_type);
  if(nested_info && ctx.can_synthesize_aggregate_constructor(*nested_info)) {
    return semantic_class_model::aggregate_element_count(*nested_info);
  }

  return 1;
}

bool initializer_clause_directly_initializes_aggregate_field(SemanticContext & ctx,
                                                            Scope & scope,
                                                            const FieldInfo & field,
                                                            const CppAstNode & child)
{
  const FieldInfo * input_field = semantic_class_model::aggregate_input_field(ctx, field);
  const TypePtr target_type = input_field ? input_field->type : field.type;
  if(string_literal_directly_initializes_array(target_type, child)) {
    return true;
  }
  try
  {
    ExprInfo expr = ctx.analyze_expression_for_target(scope, child, target_type);
    return can_copy_initialize(ctx, target_type, expr);
  }
  catch(const std::logic_error &)
  {
    return false;
  }
}

bool build_positional_aggregate_initializer_plan(
    SemanticContext & ctx,
    Scope & scope,
    const ClassInfo & info,
    const CppAstNode & payload,
    size_t field_index,
    size_t child_index,
    vector<const CppAstNode *> & field_initializers,
    vector<CppAstNode> & synthesized_nodes)
{
  const size_t aggregate_count = semantic_class_model::aggregate_element_count(info);
  if(child_index >= payload.children.size()) {
    return true;
  }
  if(field_index >= aggregate_count || field_index >= info.fields.size()) {
    return false;
  }

  const CppAstNode & child = payload.children[child_index];
  auto try_assign =
      [&](const CppAstNode * initializer, size_t consumed) -> bool
      {
        field_initializers[field_index] = initializer;
        if(build_positional_aggregate_initializer_plan(ctx,
                                                       scope,
                                                       info,
                                                       payload,
                                                       field_index + 1,
                                                       child_index + consumed,
                                                       field_initializers,
                                                       synthesized_nodes)) {
          return true;
        }
        field_initializers[field_index] = nullptr;
        return false;
      };

  if(child.kind == CppAstKind::braced_init_list) {
    return try_assign(&child, 1);
  }

  const size_t remaining_children = payload.children.size() - child_index;
  const bool can_use_brace_elision =
      aggregate_field_can_use_brace_elision(ctx, info.fields[field_index]);
  const size_t max_consume =
      std::min(aggregate_brace_elision_capacity(ctx, info.fields[field_index]),
               remaining_children);
  if(can_use_brace_elision &&
     initializer_clause_directly_initializes_aggregate_field(ctx,
                                                            scope,
                                                            info.fields[field_index],
                                                            child) &&
     try_assign(&child, 1)) {
    return true;
  }
  const size_t min_elided_consume = can_use_brace_elision ? 1 : 2;
  for(size_t consume = max_consume; consume >= min_elided_consume; --consume) {
    const CppAstNode * grouped =
        synthesize_braced_init_span(payload, child_index, consume, synthesized_nodes);
    if(try_assign(grouped, consume)) {
      return true;
    }
    synthesized_nodes.pop_back();
    if(consume == min_elided_consume) {
      break;
    }
  }

  return try_assign(&child, 1);
}

bool build_aggregate_initializer_plan_impl(SemanticContext & ctx,
                                           Scope & scope,
                                           const ClassInfo & info,
                                           const CppAstNode & payload,
                                           vector<const CppAstNode *> & field_initializers,
                                           vector<CppAstNode> & synthesized_nodes)
{
  if(payload.kind != CppAstKind::braced_init_list) {
    return false;
  }

  const size_t aggregate_count = semantic_class_model::aggregate_element_count(info);
  synthesized_nodes.reserve(synthesized_nodes.size() + payload.children.size() +
                            info.fields.size());
  field_initializers.assign(info.fields.size(), nullptr);

  for(size_t i = 0; i < payload.children.size(); ++i) {
    const CppAstNode & child = payload.children[i];
    if(!is_designated_initializer_node(child)) {
      continue;
    }

    size_t field_index = 0;
    const CppAstNode * effective_initializer = nullptr;
    if(!resolve_aggregate_designated_initializer(
            ctx, scope, info, child, field_index, effective_initializer, synthesized_nodes) ||
       field_index >= aggregate_count) {
      return false;
    }
    if(info.class_kind == "union") {
      std::fill(field_initializers.begin(), field_initializers.end(), nullptr);
    }
    field_initializers[field_index] = effective_initializer;
  }

  if(braced_init_list_has_designators(payload)) {
    return true;
  }

  std::fill(field_initializers.begin(), field_initializers.end(), nullptr);
  return build_positional_aggregate_initializer_plan(ctx,
                                                     scope,
                                                     info,
                                                     payload,
                                                     0,
                                                     0,
                                                     field_initializers,
                                                     synthesized_nodes);
}

bool extract_function_style_constructor_args(SemanticContext & ctx,
                                             Scope & scope,
                                             const TypePtr & target,
                                             const CppAstNode & node,
                                             vector<const CppAstNode *> & args)
{
  args.clear();
  if(node.kind != CppAstKind::call_expression || node.children.empty() ||
     node.children[0].kind != CppAstKind::id_expression) {
    return false;
  }

  TypePtr callee_type =
      ctx.lookup_type_node(scope, node.children[0], node.children[0].value);
  if(!callee_type ||
     !same_type_with_compatible_top_cv(strip_top_level_cv(callee_type),
                                       strip_top_level_cv(target))) {
    return false;
  }

  const CppAstNode * argument_list = find_child_kind(node, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(node, CppAstKind::paren_argument_list);
  }
  if(!argument_list) {
    argument_list = find_child_kind(node, CppAstKind::paren_argument_list);
  }
  if(!argument_list) {
    return true;
  }

  for(size_t i = 0; i < argument_list->children.size(); ++i) {
    args.push_back(&argument_list->children[i]);
  }
  return true;
}

bool is_empty_same_type_function_style_constructor_call(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const TypePtr & target,
                                                        const CppAstNode & node)
{
  if(node.kind != CppAstKind::call_expression ||
     node.children.empty() ||
     node.children[0].kind != CppAstKind::id_expression) {
    return false;
  }
  TypePtr callee_type =
      ctx.lookup_type_node(scope, node.children[0], node.children[0].value);
  if(!callee_type ||
     !same_type_with_compatible_top_cv(strip_top_level_cv(callee_type),
                                       strip_top_level_cv(target))) {
    return false;
  }
  const CppAstNode * argument_list = find_child_kind(node, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(node, CppAstKind::paren_argument_list);
  }
  return argument_list && argument_list->children.empty();
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer);

bool analyze_direct_class_materialization_initializer(SemanticContext & ctx,
                                                      Scope & scope,
                                                      const TypePtr & target_type,
                                                      const CppAstNode * initializer,
                                                      ExprInfo & out)
{
  if(!initializer || initializer->kind != CppAstKind::initializer ||
     initializer->children.size() != 1) {
    return false;
  }

  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(!payload) {
    return false;
  }
  if(payload->kind != CppAstKind::call_expression &&
     payload->kind != CppAstKind::lambda_expression &&
     payload->kind != CppAstKind::braced_init_list) {
    return false;
  }

  out = ctx.analyze_expression_for_target(scope, *payload, target_type);
  if(!type_equals(strip_top_level_cv(out.type), strip_top_level_cv(target_type))) {
    return false;
  }
  if(out.category != VC_PRVALUE) {
    return false;
  }
  if(is_empty_same_type_function_style_constructor_call(ctx, scope, target_type, *payload)) {
    return false;
  }

  return out.node.kind == CallSemKind::call_expression ||
         out.node.kind == CallSemKind::closure_object ||
         out.node.kind == CallSemKind::initializer_list_object;
}

bool initializer_uses_copy_initialization(const CppAstNode * initializer)
{
  return initializer &&
         initializer->kind == CppAstKind::initializer &&
         initializer->uses_assignment_form;
}

ConstructorSelectionOptions class_initializer_constructor_options(
    const CppAstNode * initializer,
    const CppAstNode * payload,
    bool uses_function_style_constructor_args)
{
  const bool is_copy_initialization =
      initializer_uses_copy_initialization(initializer);
  const bool is_copy_list_initialization =
      is_copy_initialization &&
      payload &&
      payload->kind == CppAstKind::braced_init_list;
  if(is_copy_list_initialization) {
    return constructor_lifecycle_service::selection_options_for(
        constructor_lifecycle_service::copy_list_initialization_profile(
            "copy-list-initialization"));
  }
  if(is_copy_initialization && uses_function_style_constructor_args) {
    return constructor_lifecycle_service::selection_options_for(
        constructor_lifecycle_service::direct_initialization_profile(
            "copy-initialization direct materialization"));
  }
  if(is_copy_initialization) {
    return constructor_lifecycle_service::selection_options_for(
        constructor_lifecycle_service::non_explicit_construction_profile(
            "copy-initialization"));
  }
  return constructor_lifecycle_service::selection_options_for(
      constructor_lifecycle_service::direct_initialization_profile());
}

void validate_elided_direct_materialization_constructor(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const CppAstNode & initializer,
    const ExprInfo & direct_init)
{
  FunctionBinding * direct_ctor =
      direct_init.category == VC_LVALUE ? copy_constructor_for(info) :
                                          move_constructor_for(info);
  if(!direct_ctor) {
    direct_ctor = copy_constructor_for(info);
  }
  if(direct_ctor && direct_ctor->access == MA_PUBLIC && !direct_ctor->is_deleted) {
    return;
  }

  std::vector<ExprInfo> source_args(1, direct_init);
  ConstructorSelectionOptions ctor_options =
      constructor_lifecycle_service::selection_options_for(
          initializer.uses_assignment_form ?
              constructor_lifecycle_service::non_explicit_construction_profile(
                  "elided copy-initialization") :
              constructor_lifecycle_service::direct_initialization_profile(
                  "elided direct-initialization"));
  ctor_options.instantiate_bodies = false;
  ctor_options.use_location = ctx.source_location_for_node(initializer);

  constructor_lifecycle_service::ConstructorSelectionResult selection;
  constructor_lifecycle_service::select_constructor_from_exprs_into(
      ctx,
      scope,
      info,
      source_args,
      selection,
      ctor_options);
}

void note_elided_direct_materialization_constructor_witness(
    SemanticContext & ctx,
    Scope & scope,
    ClassInfo & info,
    const CppAstNode & initializer,
    const ExprInfo & direct_init)
{
  if(ctx.template_witness_context().session == nullptr) {
    return;
  }
  TypePtr direct_type = strip_top_level_cv(remove_reference_type(direct_init.type));
  TypePtr target_type = strip_top_level_cv(info.type);
  if(type_equals(direct_type, target_type)) {
    FunctionBinding * ctor =
        direct_init.category == VC_LVALUE ? copy_constructor_for(info) :
                                            move_constructor_for(info);
    if(!ctor) {
      ctor = copy_constructor_for(info);
    }
    note_constructor_witness_closure_impl(ctx, ctor);
    return;
  }

  std::vector<ExprInfo> source_args(1, direct_init);
  ConstructorSelectionOptions ctor_options =
      constructor_lifecycle_service::selection_options_for(
          initializer.uses_assignment_form ?
              constructor_lifecycle_service::non_explicit_construction_profile(
                  "elided direct materialization") :
              constructor_lifecycle_service::direct_initialization_profile(
                  "elided direct materialization"));
  ctor_options.instantiate_bodies = false;
  ctor_options.synthesize_implicit_copy_move = false;
  ctor_options.use_location = ctx.source_location_for_node(initializer);

  constructor_lifecycle_service::ConstructorSelectionResult selection;
  try
  {
    constructor_lifecycle_service::select_constructor_from_exprs_into(
        ctx,
        scope,
        info,
        source_args,
        selection,
        ctor_options);
  }
  catch(const std::logic_error &)
  {
    return;
  }

  FunctionBinding * ctor = selection.ctor;
  note_constructor_witness_closure_impl(ctx, ctor);
}

const CppAstNode * find_ctor_mem_initializer(const FunctionBinding & binding,
                                             const string & name)
{
  if(!binding.ctor_initializer) {
    return nullptr;
  }

  for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    const CppAstNode & init = binding.ctor_initializer->children[i];
    const CppAstNode * id = find_child_kind(init, CppAstKind::mem_initializer_id);
    if(id && id->value == name) {
      return &init;
    }
  }
  return nullptr;
}

bool ctor_initializer_type_matches_base(SemanticContext & ctx,
                                        const TypePtr & candidate,
                                        const ClassInfo & base)
{
  TypePtr candidate_base = strip_top_level_cv(candidate);
  if(!candidate_base || !base.type) {
    return false;
  }
  if(type_equals(candidate_base, base.type)) {
    return true;
  }
  ClassInfo * candidate_info = ctx.complete_class_type(candidate_base);
  return candidate_info == &base;
}

const CppAstNode * find_ctor_base_initializer(SemanticContext & ctx,
                                              Scope & scope,
                                              const FunctionBinding & binding,
                                              const ClassInfo & base)
{
  if(!binding.ctor_initializer) {
    return nullptr;
  }

  for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
    const CppAstNode & init = binding.ctor_initializer->children[i];
    const CppAstNode * id = find_child_kind(init, CppAstKind::mem_initializer_id);
    if(!id) {
      continue;
    }
    if(id->value == base.name || id->value == base.qualified_name) {
      return &init;
    }
    if(init.children.size() >= 3 &&
       init.children[2].kind == CppAstKind::pack_expansion_expression) {
      const size_t angle = id->value.find('<');
      const string template_name =
          angle == string::npos ? id->value : id->value.substr(0, angle);
      const size_t qualifier = template_name.rfind("::");
      const string unqualified_name =
          qualifier == string::npos ? template_name : template_name.substr(qualifier + 2);
      if(unqualified_name == base.name) {
        return &init;
      }
    }
    if(id->semantic_type) {
      TypePtr semantic_type = id->semantic_type;
      TypePtr resolved_type;
      if(template_api::type::resolve_instantiated_dependent_type(ctx,
                                                                 scope,
                                                                 semantic_type,
                                                                 resolved_type) &&
         resolved_type) {
        semantic_type = resolved_type;
      }
      if(semantic_type &&
         ctor_initializer_type_matches_base(ctx, semantic_type, base)) {
        return &init;
      }
    }
    TypePtr named = ctx.lookup_type_node(scope, *id, id->value);
    if(named && ctor_initializer_type_matches_base(ctx, named, base)) {
      return &init;
    }
  }
  return nullptr;
}

bool find_ctor_base_pack_expansion_index(const ClassInfo & owner_info,
                                         const ClassInfo & subobject_class,
                                         size_t & out)
{
  for(size_t i = 0; i < owner_info.bases.size(); ++i) {
    const BaseInfo & base = owner_info.bases[i];
    if(base.type &&
       subobject_class.type &&
       type_equals(strip_top_level_cv(base.type->type),
                   strip_top_level_cv(subobject_class.type))) {
      out = i;
      return true;
    }
  }

  for(size_t i = 0; i < owner_info.virtual_base_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = owner_info.virtual_base_subobjects[i];
    if(subobject.type &&
       subobject_class.type &&
       type_equals(strip_top_level_cv(subobject.type->type),
                   strip_top_level_cv(subobject_class.type))) {
      out = i;
      return true;
    }
  }

  return false;
}

void trace_ctor_initializer_state(const FunctionBinding & binding,
                                  const ClassInfo & info,
                                  const std::string & phase,
                                  const std::string & target_name,
                                  const CppAstNode * matched)
{
  if(!parser_trace::enabled("lifetime.init")) {
    return;
  }
  std::ostringstream out;
  out << phase
      << " binding=" << static_cast<const void *>(&binding)
      << " function=" << binding.name
      << " class=" << info.qualified_name
      << " target=" << target_name
      << " ctor_init=" << (binding.ctor_initializer ? "yes" : "no")
      << " matched=" << (matched ? "yes" : "no");
  if(binding.ctor_initializer) {
    out << " ids=";
    bool first = true;
    for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
      const CppAstNode * id =
          find_child_kind(binding.ctor_initializer->children[i], CppAstKind::mem_initializer_id);
      if(!id) {
        continue;
      }
      if(!first) {
        out << ",";
      }
      first = false;
      out << id->value;
    }
  }
  parser_trace::note("lifetime.init", std::string(), out.str());
}

FunctionBinding * destructor_for(ClassInfo & info)
{
  map<string, vector<FunctionBinding *> >::iterator found =
      info.methods.find(string("~") + info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i]->is_destructor) {
      return found->second[i];
    }
  }
  return nullptr;
}

FunctionBinding * copy_constructor_for(ClassInfo & info)
{
  map<string, vector<FunctionBinding *> >::iterator found = info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i]->is_copy_constructor) {
      return found->second[i];
    }
  }
  return nullptr;
}

FunctionBinding * move_constructor_for(ClassInfo & info)
{
  map<string, vector<FunctionBinding *> >::iterator found = info.methods.find(info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i]->is_move_constructor) {
      return found->second[i];
    }
  }
  return nullptr;
}

FunctionBinding * copy_assignment_for(ClassInfo & info)
{
  map<string, vector<FunctionBinding *> >::iterator found = info.methods.find("operator=");
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i]->is_copy_assignment) {
      return found->second[i];
    }
  }
  return nullptr;
}

FunctionBinding * move_assignment_for(ClassInfo & info)
{
  map<string, vector<FunctionBinding *> >::iterator found = info.methods.find("operator=");
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i]->is_move_assignment) {
      return found->second[i];
    }
  }
  return nullptr;
}

typedef std::pair<std::size_t, std::size_t> BitFieldStorageKey;

void append_wrapped_action(DumpNode & out,
                           CallSemKind kind,
                           const string & text,
                           const string & resolved_name,
                           DumpNode call_node);

void append_target_initialization_actions(SemanticContext & ctx,
                                          Scope & scope,
                                          const TypePtr & type,
                                          const CppAstNode * initializer,
                                          const ExprInfo & target,
                                          DumpNode & out,
                                          const std::string & target_use_location = std::string(),
                                          const std::string & source_witness_target_name =
                                              std::string());

void append_value_initialization_actions(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         const ExprInfo & target,
                                         DumpNode & out);

void ensure_bit_field_storage_zeroed(SemanticContext & ctx,
                                     const ExprInfo & base,
                                     const FieldInfo & field,
                                     DumpNode & out,
                                     std::set<BitFieldStorageKey> & zeroed_storage);

void append_bit_field_copy_action(const ExprInfo & target_base,
                                  const ExprInfo & source_base,
                                  const FieldInfo & field,
                                  DumpNode & out,
                                  std::set<BitFieldStorageKey> & copied_storage);

void append_bit_field_initialization_actions(SemanticContext & ctx,
                                             Scope & scope,
                                             const FieldInfo & field,
                                             const CppAstNode * initializer,
                                             const ExprInfo & base,
                                             DumpNode & out,
                                             std::set<BitFieldStorageKey> & zeroed_storage);

const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer);

bool initializer_targets_anonymous_storage_union_member(const CppAstNode * initializer)
{
  if(!initializer) {
    return false;
  }
  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  return payload &&
         payload->kind == CppAstKind::braced_init_list &&
         braced_init_list_has_designators(*payload);
}

enum class UnionStorageTransferKind {
  copy_constructor,
  move_constructor,
  copy_assignment,
  move_assignment,
};

FunctionBinding * union_storage_transfer_binding(SemanticContext & ctx,
                                                 ClassInfo & info,
                                                 UnionStorageTransferKind kind)
{
  switch(kind) {
  case UnionStorageTransferKind::copy_constructor: {
    FunctionBinding * ctor = copy_constructor_for(info);
    return ctor ? ctor : ctx.ensure_implicit_copy_constructor(info);
  }
  case UnionStorageTransferKind::move_constructor: {
    FunctionBinding * ctor = move_constructor_for(info);
    if(!ctor) {
      ctor = ctx.ensure_implicit_move_constructor(info);
    }
    if(ctor) {
      return ctor;
    }
    ctor = copy_constructor_for(info);
    return ctor ? ctor : ctx.ensure_implicit_copy_constructor(info);
  }
  case UnionStorageTransferKind::copy_assignment: {
    FunctionBinding * op = copy_assignment_for(info);
    return op ? op : ctx.ensure_implicit_copy_assignment(info);
  }
  case UnionStorageTransferKind::move_assignment: {
    FunctionBinding * op = move_assignment_for(info);
    if(!op) {
      op = ctx.ensure_implicit_move_assignment(info);
    }
    if(op) {
      return op;
    }
    op = copy_assignment_for(info);
    return op ? op : ctx.ensure_implicit_copy_assignment(info);
  }
  }
  return nullptr;
}

void append_union_copy_action(SemanticContext & ctx,
                              Scope &,
                              ClassInfo & info,
                              const ExprInfo & target_ptr,
                              const ExprInfo & source,
                              DumpNode & out,
                              UnionStorageTransferKind kind)
{
  FunctionBinding * binding = union_storage_transfer_binding(ctx, info, kind);
  if(!binding) {
    throw logic_error("missing union storage transfer binding");
  }

  std::vector<ExprInfo> args;
  args.push_back(target_ptr);
  if(kind == UnionStorageTransferKind::move_constructor ||
     kind == UnionStorageTransferKind::move_assignment) {
    args.push_back(make_xvalue_expr(source));
  } else {
    args.push_back(source);
  }

  constructor_lifecycle_service::ConstructorActionResult action_result =
      constructor_lifecycle_service::prepare_lifecycle_call(ctx,
                                                            binding,
                                                            args,
                                                            true,
                                                            OutputReason::SyntheticDependency);
  DumpNode action = make_dump_node(CallSemKind::constructor_action, binding->name);
  set_callsem_resolved_name(action, function_output_name(*binding));
  action.trivial_lifecycle = action_result.trivial_lifecycle;
  action.children.push_back(std::move(action_result.call_expr.node));
  out.children.push_back(std::move(action));
}

void append_field_initialization_actions(SemanticContext & ctx,
                                         Scope & scope,
                                         const FieldInfo & field,
                                         const CppAstNode * initializer,
                                         const ExprInfo & object_expr,
                                         DumpNode & out,
                                         std::set<BitFieldStorageKey> & zeroed_bit_storage)
{
  if(field.is_bit_field) {
    if(initializer) {
      append_bit_field_initialization_actions(ctx,
                                              scope,
                                              field,
                                              initializer,
                                              object_expr,
                                              out,
                                              zeroed_bit_storage);
    } else {
      ensure_bit_field_storage_zeroed(ctx,
                                      object_expr,
                                      field,
                                      out,
                                      zeroed_bit_storage);
    }
    return;
  }

  const FieldInfo * aggregate_field =
      initializer_targets_anonymous_storage_union_member(initializer) ?
          &field :
          semantic_class_model::aggregate_input_field(ctx, field);
  if(aggregate_field != &field) {
    ExprInfo storage_expr = ctx.make_field_expr(object_expr, field);
    const CppAstNode * effective_initializer =
        initializer ? initializer : aggregate_field->default_initializer;
    append_field_initialization_actions(ctx,
                                        scope,
                                        *aggregate_field,
                                        effective_initializer,
                                        storage_expr,
                                        out,
                                        zeroed_bit_storage);
    return;
  }

  ExprInfo field_expr = ctx.make_field_expr(object_expr, field);
  if(initializer) {
    append_target_initialization_actions(ctx, scope, field.type, initializer, field_expr, out);
  } else {
    append_value_initialization_actions(ctx, scope, field.type, field_expr, out);
  }
}

bool append_anonymous_storage_constructor_initialization_actions(
    SemanticContext & ctx,
    Scope & scope,
    const FunctionBinding & binding,
    const FieldInfo & field,
    const ExprInfo & object_expr,
    DumpNode & out,
    std::set<BitFieldStorageKey> & zeroed_bit_storage)
{
  ClassInfo * storage_info =
      semantic_class_model::anonymous_storage_union_info(ctx, field);
  if(!storage_info) {
    return false;
  }

  const FieldInfo * active_field = nullptr;
  const CppAstNode * active_initializer = nullptr;
  for(size_t i = 0; i < storage_info->fields.size(); ++i) {
    const CppAstNode * mem_init =
        find_ctor_mem_initializer(binding, storage_info->fields[i].name);
    if(mem_init) {
      active_field = &storage_info->fields[i];
      active_initializer =
          mem_init->children.size() >= 2 ? &mem_init->children[1] :
                                           storage_info->fields[i].default_initializer;
      break;
    }
  }

  if(!active_field) {
    for(size_t i = 0; i < storage_info->fields.size(); ++i) {
      if(storage_info->fields[i].default_initializer) {
        active_field = &storage_info->fields[i];
        active_initializer = storage_info->fields[i].default_initializer;
        break;
      }
    }
  }

  if(!active_field) {
    return true;
  }

  ExprInfo storage_expr = ctx.make_field_expr(object_expr, field);
  append_field_initialization_actions(ctx,
                                      scope,
                                      *active_field,
                                      active_initializer,
                                      storage_expr,
                                      out,
                                      zeroed_bit_storage);
  return true;
}

void append_wrapped_action(DumpNode & out,
                           CallSemKind kind,
                           const string & text,
                           const string & resolved_name,
                           DumpNode call_node)
{
  DumpNode wrapper = make_dump_node(kind, text);
  set_callsem_resolved_name(wrapper, resolved_name);
  wrapper.children.push_back(std::move(call_node));
  out.children.push_back(std::move(wrapper));
}

unsigned long long vtable_group_address_point_offset(const ClassInfo & info,
                                                     size_t table_index)
{
  const unsigned long long prefix_words =
      info.virtual_base_subobjects.empty() ?
          2ULL :
          static_cast<unsigned long long>(info.virtual_base_subobjects.size()) + 2ULL;
  const unsigned long long prefix_size = prefix_words * 8ULL;

  unsigned long long group_offset = 0;
  std::map<std::size_t, unsigned long long> address_points_by_view_offset;
  for(size_t i = 0; i <= table_index; ++i) {
    const VTableInfo & table = info.vtables[i];
    const std::size_t view_offset = info.vtables[i].view_offset;
    std::map<std::size_t, unsigned long long>::const_iterator found =
        address_points_by_view_offset.find(view_offset);
    if(found != address_points_by_view_offset.end()) {
      if(i == table_index) {
        return found->second;
      }
      continue;
    }

    const unsigned long long address_point = group_offset + prefix_size;
    if(i == table_index) {
      return address_point;
    }
    address_points_by_view_offset[view_offset] = address_point;
    std::size_t slot_count = table.slots.size();
    if(slot_count == 0 && table.view_type) {
      slot_count = table.view_type->vtable_entries.size();
      for(size_t j = 0; j < table.view_type->method_declaration_order.size(); ++j) {
        FunctionBinding * binding = table.view_type->method_declaration_order[j];
        if(!binding || !binding->is_virtual || !binding->has_virtual_slot) {
          continue;
        }
        slot_count = std::max(slot_count, binding->virtual_slot + 1);
        if(binding->is_destructor) {
          slot_count = std::max(slot_count, binding->virtual_slot + 2);
        }
      }
    }
    group_offset +=
        prefix_size +
        static_cast<unsigned long long>(slot_count) * 8ULL;
  }
  return prefix_size;
}

void append_vptr_action(const string & dynamic_class_key,
                        const TypePtr & dynamic_class_type,
                        const string & table_key,
                        unsigned long long address_point_offset,
                        const size_t * vtt_entry_index,
                        const ExprInfo & object_ptr,
                        DumpNode & out)
{
  DumpNode action = make_dump_node(CallSemKind::vptr_action, table_key);
  set_callsem_resolved_name(action, dynamic_class_key);
  action.semantic_type = dynamic_class_type;
  set_callsem_uint_value(action, address_point_offset);
  if(vtt_entry_index) {
    action.has_vtt_entry_index = true;
    set_callsem_vtt_entry_index(action, *vtt_entry_index);
  }
  action.children.push_back(object_ptr.node);
  out.children.push_back(std::move(action));
}

bool vtable_view_virtual_base_root(const ClassInfo & info,
                                   const VTableInfo & table,
                                   string & root_name,
                                   unsigned long long & offset_in_root)
{
  if(table.view_offset == 0 || !table.view_type) {
    return false;
  }
  for(size_t i = 0; i < info.virtual_base_subobjects.size(); ++i) {
    const SubobjectInfo & virtual_base = info.virtual_base_subobjects[i];
    if(!virtual_base.type || table.view_offset < virtual_base.offset) {
      continue;
    }
    const unsigned long long relative_offset =
        static_cast<unsigned long long>(table.view_offset - virtual_base.offset);
    if((virtual_base.type == table.view_type ||
        virtual_base.type->qualified_name == table.view_type->qualified_name) &&
       relative_offset == 0) {
      root_name = virtual_base.type->qualified_name;
      offset_in_root = 0;
      return true;
    }
    for(size_t j = 0; j < virtual_base.type->complete_subobjects.size(); ++j) {
      const SubobjectInfo & nested = virtual_base.type->complete_subobjects[j];
      if(!nested.type || nested.offset != relative_offset) {
        continue;
      }
      if(nested.type == table.view_type ||
         nested.type->qualified_name == table.view_type->qualified_name) {
        root_name = virtual_base.type->qualified_name;
        offset_in_root = relative_offset;
        return true;
      }
    }
  }
  return false;
}

void mark_virtual_base_vptr_target(ExprInfo & object_ptr,
                                   const string & root_name,
                                   unsigned long long offset_in_root,
                                   const string & view_name)
{
  object_ptr.node.is_virtual_base_subobject = true;
  set_callsem_resolved_name(object_ptr.node, view_name);
  mutable_callsem_virtual_base_layout(object_ptr.node).push_back(
      make_pair(root_name, offset_in_root));
  if(object_ptr.node.kind == CallSemKind::unary_expression &&
     callsem_has_token(object_ptr.node, OP_AMP) &&
     object_ptr.node.children.size() == 1 &&
     object_ptr.node.children[0].kind == CallSemKind::member_expression) {
    object_ptr.node.children[0].is_base_subobject = true;
    object_ptr.node.children[0].is_virtual_base_subobject = true;
    set_callsem_resolved_name(object_ptr.node.children[0], view_name);
    set_callsem_virtual_base_layout(object_ptr.node.children[0],
                                    callsem_virtual_base_layout(object_ptr.node));
  }
}

void append_all_vptr_actions(SemanticContext & ctx,
                             ClassInfo & info,
                             const ExprInfo & this_expr,
                             symbol_linkage::SpecialMemberEntryPointKind entry_point_kind,
                             DumpNode & out)
{
  std::set<size_t> initialized_offsets;
  for(size_t i = 0; i < info.vtables.size(); ++i) {
    const VTableInfo & table = info.vtables[i];
    if(!initialized_offsets.insert(table.view_offset).second) {
      continue;
    }
    ExprInfo object_ptr = table.view_offset == 0 ?
        this_expr :
        ctx.make_base_pointer_expr(this_expr, *table.view_type, table.view_offset);
    string virtual_base_root;
    unsigned long long virtual_base_offset = 0;
    if(vtable_view_virtual_base_root(info,
                                     table,
                                     virtual_base_root,
                                     virtual_base_offset)) {
      mark_virtual_base_vptr_target(object_ptr,
                                    virtual_base_root,
                                    virtual_base_offset,
                                    table.view_type->qualified_name);
    }
    size_t vtt_entry_index = 0;
    const size_t * vtt_entry_index_ptr = nullptr;
    if(entry_point_kind == symbol_linkage::SMEK_BASE &&
       semantic_class_model::class_needs_vtt(info) &&
       semantic_class_model::find_vtt_self_table_index(ctx, info, table.key, vtt_entry_index)) {
      vtt_entry_index_ptr = &vtt_entry_index;
    }
    append_vptr_action(info.qualified_name,
                       info.type,
                       table.key,
                       vtable_group_address_point_offset(info, i),
                       vtt_entry_index_ptr,
                       object_ptr,
                       out);
  }
}

void annotate_special_member_call_with_vtt(SemanticContext & ctx,
                                           ClassInfo * vtt_owner_info,
                                           ClassInfo & target_info,
                                           DumpNode & call_node)
{
  if(call_node.kind != CallSemKind::call_expression ||
     call_node.children.empty() ||
     call_node.children[0].kind != CallSemKind::callee) {
    return;
  }

  DumpNode & callee = call_node.children[0];
  if(!callee.has_special_member_entry_point_kind ||
     callsem_special_member_entry_point_kind(callee) != symbol_linkage::SMEK_BASE ||
     !semantic_class_model::class_needs_vtt(target_info)) {
    return;
  }

  ClassInfo * effective_owner = vtt_owner_info ? vtt_owner_info : &target_info;
  size_t slice_offset = 0;
  if(effective_owner != &target_info &&
     !semantic_class_model::find_vtt_direct_base_slice_offset(ctx,
                                                              *effective_owner,
                                                              target_info,
                                                              slice_offset)) {
    effective_owner = &target_info;
    slice_offset = 0;
  }

  const string vtt_symbol =
      symbol_linkage::internal_symbol_from_name(effective_owner->qualified_name + "::__vtt");
  if(vtt_symbol.empty()) {
    throw logic_error("failed to name VTT symbol for " + effective_owner->qualified_name);
  }

  callee.uses_vtt_parameter = true;
  set_callsem_vtt_symbol(callee, vtt_symbol);
  set_callsem_vtt_object_symbol(callee, symbol_linkage::vtt_object_symbol(*effective_owner));
  set_callsem_vtt_owner_type(callee, effective_owner->type);
  callee.has_vtt_slice_offset = true;
  set_callsem_vtt_slice_offset(callee, slice_offset);
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer);

bool initializer_is_empty_value_initializer(const CppAstNode * initializer)
{
  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(!payload) {
    return false;
  }
  if(payload->kind != CppAstKind::paren_argument_list &&
     payload->kind != CppAstKind::paren_initializer &&
     payload->kind != CppAstKind::braced_init_list) {
    return false;
  }
  return initializer_argument_nodes(*payload).empty();
}

void append_constructor_call_action(SemanticContext & ctx,
                                    Scope & scope,
                                    ClassInfo & info,
                                    const vector<const CppAstNode *> & arg_nodes,
                                    const ExprInfo & object_ptr,
                                    const CppAstNode * direct_braced_init,
                                    const ConstructorSelectionOptions & selection_options,
                                    ClassInfo * vtt_owner_info,
                                    bool value_initializes_result,
                                    DumpNode & out,
                                    const std::string & source_witness_target_name =
                                        std::string());

void append_ctor_subobject_constructor_action(SemanticContext & ctx,
                                              Scope & scope,
                                              FunctionBinding & binding,
                                              ClassInfo & owner_info,
                                              ClassInfo & subobject_class,
                                              const ExprInfo & object_ptr,
                                              DumpNode & out)
{
  const CppAstNode * base_init = find_ctor_base_initializer(ctx, scope, binding, subobject_class);
  bool value_initializes_result = false;
  trace_ctor_initializer_state(binding,
                               owner_info,
                               "base-init",
                               subobject_class.name,
                               base_init);
  vector<const CppAstNode *> args;
  const CppAstNode * direct_braced_init = nullptr;
  std::deque<CppAstNode> pack_expansion_storage;
  if(base_init && base_init->children.size() >= 2) {
    args = initializer_argument_nodes(base_init->children[1]);
    const CppAstNode * payload = unwrap_initializer_payload(&base_init->children[1]);
    if(payload && payload->kind == CppAstKind::braced_init_list) {
      direct_braced_init = payload;
    }
    value_initializes_result =
        initializer_is_empty_value_initializer(&base_init->children[1]);
    if(base_init->children.size() >= 3 &&
       base_init->children[2].kind == CppAstKind::pack_expansion_expression &&
       !args.empty()) {
      size_t expansion_index = 0;
      if(find_ctor_base_pack_expansion_index(owner_info, subobject_class, expansion_index)) {
        vector<const CppAstNode *> expanded_args;
        expanded_args.reserve(args.size());
        bool expansion_ok = true;
        for(size_t i = 0; i < args.size(); ++i) {
          CppAstNode wrapped;
          wrapped.kind = CppAstKind::pack_expansion_expression;
          wrapped.children.push_back(*args[i]);
          vector<CppAstNode> nodes;
          if(!ctx.expand_pack_argument_node(scope, wrapped, nodes) ||
             expansion_index >= nodes.size()) {
            expansion_ok = false;
            break;
          }
          pack_expansion_storage.push_back(nodes[expansion_index]);
          expanded_args.push_back(&pack_expansion_storage.back());
        }
        if(expansion_ok && expanded_args.size() == args.size()) {
          args.swap(expanded_args);
        }
      }
    }
    if(parser_trace::enabled("lifetime.init")) {
      std::ostringstream trace;
      trace << "base-init-payload function=" << binding.name
            << " target=" << subobject_class.qualified_name
            << " payload-kind=" << cppast_kind_text(base_init->children[1].kind)
            << " payload-ast={" << describe_cppast_translation_unit(base_init->children[1]) << "}"
            << " raw-args=" << args.size();
      for(size_t i = 0; i < args.size(); ++i) {
        if(!args[i]) {
          continue;
        }
        trace << " [arg" << i
              << " kind=" << cppast_kind_text(args[i]->kind)
              << " ast={" << describe_cppast_translation_unit(*args[i]) << "}]";
      }
      if(base_init->children.size() >= 3) {
        trace << " extra-kind=" << cppast_kind_text(base_init->children[2].kind)
              << " extra-ast={" << describe_cppast_translation_unit(base_init->children[2]) << "}";
      }
      parser_trace::note("lifetime.init", std::string(), trace.str());
    }
  }
  append_constructor_call_action(ctx,
                                 scope,
                                 subobject_class,
                                 args,
                                 object_ptr,
                                 direct_braced_init,
                                 ConstructorSelectionOptions(),
                                 &owner_info,
                                 value_initializes_result,
                                 out);
}

void append_virtual_base_constructor_actions(SemanticContext & ctx,
                                             Scope & scope,
                                             FunctionBinding & binding,
                                             ClassInfo & info,
                                             const ExprInfo & this_expr,
                                             DumpNode & out)
{
  for(size_t i = 0; i < info.virtual_base_subobjects.size(); ++i) {
    const SubobjectInfo & subobject = info.virtual_base_subobjects[i];
    if(!subobject.type) {
      continue;
    }
    append_ctor_subobject_constructor_action(ctx,
                                             scope,
                                             binding,
                                             info,
                                             *subobject.type,
                                             ctx.make_base_pointer_expr(this_expr,
                                                                        *subobject.type,
                                                                        subobject.offset),
                                             out);
  }
}

void append_constructor_call_action(SemanticContext & ctx,
                                    Scope & scope,
                                    ClassInfo & info,
                                    const vector<const CppAstNode *> & arg_nodes,
                                    const ExprInfo & object_ptr,
                                    const CppAstNode * direct_braced_init,
                                    const ConstructorSelectionOptions & selection_options,
                                    ClassInfo * vtt_owner_info,
                                    bool value_initializes_result,
                                    DumpNode & out,
                                    const std::string & source_witness_target_name)
{
  std::deque<CppAstNode> synthesized_nodes;
  const vector<const CppAstNode *> expanded_arg_nodes =
      expand_initializer_argument_nodes(ctx, scope, arg_nodes, synthesized_nodes);
  const std::string object_use_location =
      callsem_node_source_location_text(object_ptr.node);
  std::string constructor_use_location =
      direct_braced_init ? ctx.source_location_for_node(*direct_braced_init) :
      (!expanded_arg_nodes.empty() ? ctx.source_location_for_node(*expanded_arg_nodes.front()) :
                                     std::string());
  constructor_use_location =
      prefer_later_source_location_text(constructor_use_location,
                                        object_use_location);
  constructor_use_location =
      prefer_later_source_location_text(constructor_use_location,
                                        parser_trace::current_use_location());
  std::string source_witness_location =
      selection_options.source_witness_location;
  if(source_witness_location.empty() &&
     !source_witness_target_name.empty() &&
     constructor_witness_source_capture_enabled(ctx)) {
    source_witness_location =
        source_location_for_identifier_before_on_same_line(
            ctx,
            constructor_use_location,
            source_witness_target_name);
  }
  if(parser_trace::enabled("lifetime.init")) {
    std::ostringstream trace;
    trace << "constructor-call-args target=" << info.qualified_name
          << " raw=" << arg_nodes.size()
          << " expanded=" << expanded_arg_nodes.size();
    for(size_t i = 0; i < expanded_arg_nodes.size(); ++i) {
      if(!expanded_arg_nodes[i]) {
        continue;
      }
      trace << " [expanded" << i
            << " kind=" << cppast_kind_text(expanded_arg_nodes[i]->kind)
            << " ast={" << describe_cppast_translation_unit(*expanded_arg_nodes[i]) << "}]";
    }
    parser_trace::note("lifetime.init", std::string(), trace.str());
  }
  constructor_lifecycle_service::ConstructorSelectionResult selection;
  const auto try_select_aggregate_constructor =
      [&](const CppAstNode & init_node, const std::string & context) -> bool
      {
        if(init_node.kind != CppAstKind::braced_init_list ||
           !ctx.can_synthesize_aggregate_constructor(info)) {
          return false;
        }

        vector<ExprInfo> source_args;
        if(!build_aggregate_constructor_source_args(ctx,
                                                    scope,
                                                    info,
                                                    init_node,
                                                    source_args)) {
          return false;
        }

        ConstructorSelectionOptions ctor_options = selection_options;
        if(ctor_options.use_location.empty()) {
          ctor_options.use_location = constructor_use_location;
        }
        if(ctor_options.source_witness_location.empty() &&
           !source_witness_location.empty()) {
          ctor_options.source_witness_location = source_witness_location;
          ctor_options.source_witness_direct_construction = true;
        }
        constructor_lifecycle_service::apply_selection_profile(
            ctor_options,
            constructor_lifecycle_service::aggregate_partial_match_profile(
                context.c_str(),
                !source_args.empty() &&
                    source_args.size() < semantic_class_model::aggregate_element_count(info)));
        selection = constructor_lifecycle_service::select_constructor_from_exprs(ctx,
                                                                                 scope,
                                                                                 info,
                                                                                 source_args,
                                                                                 ctor_options);
        return true;
      };
  if(direct_braced_init) {
    if(try_select_aggregate_constructor(*direct_braced_init, "constructor action aggregate")) {
      // Aggregate initialization owns direct-braced construction for aggregates.
    } else {
      ConstructorSelectionOptions ctor_options = selection_options;
      if(ctor_options.use_location.empty()) {
        ctor_options.use_location = constructor_use_location;
      }
      if(ctor_options.source_witness_location.empty() &&
         !source_witness_location.empty()) {
        ctor_options.source_witness_location = source_witness_location;
        ctor_options.source_witness_direct_construction = true;
      }
      constructor_lifecycle_service::apply_selection_profile(
          ctor_options,
          constructor_lifecycle_service::direct_initialization_profile(
              "constructor action"));
      selection = constructor_lifecycle_service::select_constructor_for_direct_braced_init(
          ctx,
          scope,
          info,
          *direct_braced_init,
          ctor_options);
    }
  } else {
    const bool aggregate_constructed =
        expanded_arg_nodes.size() == 1 &&
        expanded_arg_nodes[0] &&
        try_select_aggregate_constructor(*expanded_arg_nodes[0],
                                         "constructor action aggregate");
    if(!aggregate_constructed) {
      ConstructorSelectionOptions ctor_options = selection_options;
      if(ctor_options.use_location.empty()) {
        ctor_options.use_location = constructor_use_location;
      }
      if(ctor_options.source_witness_location.empty() &&
         !source_witness_location.empty()) {
        ctor_options.source_witness_location = source_witness_location;
        ctor_options.source_witness_direct_construction = true;
      }
      constructor_lifecycle_service::apply_selection_profile(
          ctor_options,
          constructor_lifecycle_service::direct_initialization_profile(
              "constructor action"));
      selection = constructor_lifecycle_service::select_constructor(ctx,
                                                                    scope,
                                                                    info,
                                                                    expanded_arg_nodes,
                                                                    ctor_options);
    }
  }
  FunctionBinding * ctor = selection.ctor;
  const bool trivial_ctor = ctor && is_trivial_constructor_binding(ctx, *ctor);
  constructor_lifecycle_service::ConstructorActionResult action_result =
      constructor_lifecycle_service::prepare_selected_constructor_action(
          ctx,
          object_ptr,
          selection,
          trivial_ctor,
          OutputReason::ConstructorUse);
  DumpNode action = make_dump_node(CallSemKind::constructor_action, ctor->name);
  set_callsem_resolved_name(action, function_output_name(*ctor));
  action.trivial_lifecycle = action_result.trivial_lifecycle;
  action.children.push_back(std::move(action_result.call_expr.node));
  if(value_initializes_result &&
     constructor_lifecycle_service::value_initialization_requires_zero_init(*ctor)) {
    action.children.back().value_initializes_result = true;
  }
  annotate_special_member_call_with_vtt(ctx, vtt_owner_info, info, action.children.back());
  out.children.push_back(std::move(action));
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode * initializer)
{
  if(initializer &&
     initializer->kind == CppAstKind::initializer &&
     initializer->children.size() == 1) {
    return &initializer->children[0];
  }
  return initializer;
}

ExprInfo make_aggregate_default_field_expr(SemanticContext & ctx,
                                           Scope & scope,
                                           const TypePtr & param_type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(param_type));
  if(base &&
     (base->kind == Type::TK_ARRAY || ctx.class_info_for_type(base) ||
      ctx.complete_class_type(base))) {
    CppAstNode empty;
    empty.kind = CppAstKind::braced_init_list;
    return ctx.analyze_expression_for_target(scope, empty, param_type);
  }
  return ctx.make_value_initialized_expr(param_type);
}

DumpNode make_assignment_statement(const ExprInfo & lhs,
                                   ExprInfo rhs)
{
  DumpNode stmt = make_dump_node(CallSemKind::expression_statement);
  DumpNode assign = make_dump_node(CallSemKind::assignment_expression, "=");
  assign.has_token = true;
  assign.token_type = OP_ASS;
  set_expr_metadata(assign, lhs.type, VC_LVALUE);
  assign.children.push_back(lhs.node);
  assign.children.push_back(std::move(rhs.node));
  stmt.children.push_back(std::move(assign));
  return stmt;
}

bool field_eligible_for_trivial_storage_prefix_copy(SemanticContext & ctx,
                                                    const FieldInfo & field,
                                                    bool assignment_like,
                                                    bool move_like)
{
  if(field.is_bit_field || is_reference_type(field.type)) {
    return false;
  }
  if(ClassInfo * field_class = ctx.class_info_for_type(field.type)) {
    if(ctx.is_empty_class_info(field_class)) {
      return false;
    }
  }
  if(assignment_like) {
    return is_trivially_copy_assignable_type(ctx, field.type);
  }
  if(move_like) {
    return semantic_class_model::is_trivially_move_constructible_type_for_host_abi(
        ctx,
        field.type);
  }
  return is_trivially_copy_constructible_type(ctx, field.type);
}

LeadingTrivialStoragePrefix leading_trivial_storage_prefix(SemanticContext & ctx,
                                                           const ClassInfo & info,
                                                           bool assignment_like,
                                                           bool move_like)
{
  LeadingTrivialStoragePrefix prefix;
  if(info.class_kind == "union" || info.is_polymorphic || !info.bases.empty()) {
    return prefix;
  }

  for(size_t i = 0; i < info.fields.size(); ++i) {
    const FieldInfo & field = info.fields[i];
    if(!field_eligible_for_trivial_storage_prefix_copy(ctx,
                                                       field,
                                                       assignment_like,
                                                       move_like)) {
      break;
    }
    prefix.field_count = i + 1;
    prefix.byte_count = max(prefix.byte_count, field.offset + type_size(field.type));
  }
  return prefix;
}

void append_trivial_storage_prefix_copy_action(const ExprInfo & target_ptr,
                                               const ExprInfo & source,
                                               size_t byte_count,
                                               DumpNode & out)
{
  if(byte_count == 0) {
    return;
  }

  DumpNode callee = make_dump_node(CallSemKind::callee, "<trivial-storage-prefix-copy>");
  set_expr_metadata(callee,
                    make_function(make_fundamental(FT_VOID), vector<TypePtr>(), false),
                    VC_PRVALUE);

  DumpNode call = make_dump_node(CallSemKind::call_expression,
                                 "<trivial-storage-prefix-copy>");
  set_expr_metadata(call, make_fundamental(FT_VOID), VC_PRVALUE);
  call.children.push_back(std::move(callee));
  call.children.push_back(target_ptr.node);
  call.children.push_back(source.node);

  DumpNode action = make_dump_node(CallSemKind::constructor_action,
                                   "<trivial-storage-prefix-copy>");
  action.trivial_lifecycle = true;
  action.has_trivial_storage_copy_prefix = true;
  set_callsem_trivial_storage_copy_prefix_bytes(action, byte_count);
  action.children.push_back(std::move(call));
  out.children.push_back(std::move(action));
}

void append_direct_materialization_initialization_action(SemanticContext & ctx,
                                                         const ExprInfo & target,
                                                         ExprInfo source,
                                                         DumpNode & out)
{
  DumpNode callee = make_dump_node(CallSemKind::callee, "<direct-materialization>");
  set_expr_metadata(callee,
                    make_function(make_fundamental(FT_VOID), vector<TypePtr>(), false),
                    VC_PRVALUE);

  DumpNode call = make_dump_node(CallSemKind::call_expression,
                                 "<direct-materialization>");
  set_expr_metadata(call, make_fundamental(FT_VOID), VC_PRVALUE);
  call.children.push_back(std::move(callee));
  call.children.push_back(ctx.make_address_of_expr(target).node);
  call.children.push_back(std::move(source.node));

  DumpNode action = make_dump_node(CallSemKind::constructor_action,
                                   "<direct-materialization>");
  action.trivial_lifecycle = true;
  action.children.push_back(std::move(call));
  out.children.push_back(std::move(action));
}

BitFieldStorageKey bit_field_storage_key(const FieldInfo & field)
{
  return std::make_pair(field.offset, field.bit_storage_size);
}

unsigned long long low_bits_mask(std::size_t bits)
{
  if(bits == 0) {
    return 0;
  }
  if(bits >= sizeof(unsigned long long) * 8) {
    return ~0ULL;
  }
  return (1ULL << bits) - 1ULL;
}

DumpNode make_typed_integer_literal_node(const TypePtr & type,
                                         unsigned long long value)
{
  DumpNode node = make_dump_node(CallSemKind::literal, std::to_string(value));
  node.semantic_type = type;
  node.value_category = CVC_PRVALUE;
  set_callsem_uint_value(node, value);
  return node;
}

bool array_accepts_string_literal(const TypePtr & array_type,
                                  EFundamentalType literal_element_type)
{
  TypePtr element = strip_top_level_cv(array_type->inner);
  if(!element || element->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }

  EFundamentalType dest = element->fundamental;
  if(literal_element_type == FT_CHAR) {
    return dest == FT_CHAR ||
           dest == FT_SIGNED_CHAR ||
           dest == FT_UNSIGNED_CHAR;
  }

  return dest == literal_element_type;
}

bool build_string_literal_array_initializer(const TypePtr & type,
                                            const CppAstNode & payload,
                                            DumpNode & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_ARRAY || !base->has_bound ||
     (payload.kind != CppAstKind::literal &&
      payload.kind != CppAstKind::braced_init_list)) {
    return false;
  }

  const CppAstNode * literal_node = &payload;
  if(payload.kind == CppAstKind::braced_init_list) {
    if(payload.children.size() != 1 ||
       payload.children[0].kind != CppAstKind::literal) {
      return false;
    }
    literal_node = &payload.children[0];
  }

  if(literal_node->kind != CppAstKind::literal) {
    return false;
  }

  QuoteLiteralData literal;
  try
  {
    literal = parse_quote_literal(literal_node->value);
  }
  catch(const logic_error &)
  {
    return false;
  }

  if(literal.quote != '"' || !literal.ud_suffix.empty()) {
    return false;
  }

  TypePtr element_type = strip_top_level_cv(base->inner);
  if(!element_type || element_type->kind != Type::TK_FUNDAMENTAL ||
     !array_accepts_string_literal(base, string_literal_element_type(literal))) {
    return false;
  }

  const size_t element_size = type_to_size(element_type->fundamental);
  const unsigned long long max_value =
      element_size >= sizeof(unsigned long long) ? ~0ULL :
      ((1ULL << (element_size * 8)) - 1ULL);
  const vector<unsigned long long> & units = quote_literal_string_units(literal);
  if(units.size() + 1 > base->bound) {
    throw logic_error("string literal too long");
  }

  for(size_t i = 0; i < units.size(); ++i) {
    const unsigned long long value = units[i];
    if(value > max_value) {
      throw logic_error("string literal element out of range");
    }
    out.children.push_back(make_typed_integer_literal_node(base->inner, value));
  }
  out.children.push_back(make_typed_integer_literal_node(base->inner, 0));
  while(out.children.size() < base->bound) {
    out.children.push_back(make_typed_integer_literal_node(base->inner, 0));
  }
  return true;
}

ExprInfo make_expr_from_node(const TypePtr & type,
                             ValueCategory category,
                             const DumpNode & node)
{
  ExprInfo result;
  result.type = type;
  result.category = category;
  result.node = node;
  return result;
}

ExprInfo make_generated_binary_expr(ETokenType token_type,
                                    const string & text,
                                    const TypePtr & type,
                                    const ExprInfo & lhs,
                                    const ExprInfo & rhs)
{
  ExprInfo result;
  result.type = type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::binary_expression, text);
  result.node.has_token = true;
  result.node.token_type = token_type;
  set_expr_metadata(result.node, result.type, result.category);
  result.node.children.push_back(lhs.node);
  result.node.children.push_back(rhs.node);
  return result;
}

ExprInfo make_bit_field_storage_expr(const ExprInfo & base,
                                     const FieldInfo & field)
{
  ExprInfo result;
  result.type = field.type;
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::member_expression, field.name);
  set_expr_metadata(result.node, result.type, result.category);
  set_callsem_uint_value(result.node, field.offset);
  result.node.is_reference_storage = is_reference_type(field.type);
  result.node.children.push_back(base.node);
  return result;
}

ExprInfo convert_generated_expr(SemanticContext & ctx,
                                Scope & scope,
                                const TypePtr & target,
                                const ExprInfo & expr)
{
  ExprInfo converted;
  ConversionRank rank = CR_BAD;
  if(ctx.try_argument_conversion(
         scope,
         target,
         expr,
         converted,
         rank,
         semantic_policy::default_argument_conversion())) {
    converted.node.semantic_type = target;
    converted.node.value_category = to_call_value_category(converted.category);
    return converted;
  }
  if(same_type_with_compatible_top_cv(strip_top_level_cv(expr.type),
                                      strip_top_level_cv(target))) {
    ExprInfo adjusted = expr;
    adjusted.type = target;
    adjusted.node.semantic_type = target;
    adjusted.node.value_category = to_call_value_category(adjusted.category);
    return adjusted;
  }
  throw logic_error("invalid generated bit-field conversion");
}

void ensure_bit_field_storage_zeroed(SemanticContext & ctx,
                                     const ExprInfo & base,
                                     const FieldInfo & field,
                                     DumpNode & out,
                                     std::set<BitFieldStorageKey> & zeroed_storage)
{
  if(field.bit_width == 0 ||
     !zeroed_storage.insert(bit_field_storage_key(field)).second) {
    return;
  }

  ExprInfo storage = make_bit_field_storage_expr(base, field);
  out.children.push_back(make_assignment_statement(storage,
                                                   ctx.make_value_initialized_expr(field.type)));
}

void append_bit_field_store_action(SemanticContext & ctx,
                                   Scope & scope,
                                   const ExprInfo & base,
                                   const FieldInfo & field,
                                   const ExprInfo & init,
                                   DumpNode & out,
                                   std::set<BitFieldStorageKey> *zeroed_storage)
{
  if(field.bit_width == 0) {
    return;
  }

  ExprInfo storage = make_bit_field_storage_expr(base, field);
  ExprInfo converted = convert_generated_expr(ctx, scope, field.type, init);
  const std::size_t storage_bits = cpp_decl::type_size(field.type) * 8;
  const bool first_container_write =
      zeroed_storage &&
      zeroed_storage->insert(bit_field_storage_key(field)).second;

  const unsigned long long value_mask = low_bits_mask(field.bit_width);
  ExprInfo value_mask_expr =
      make_expr_from_node(field.type, VC_PRVALUE,
                          make_typed_integer_literal_node(field.type, value_mask));
  ExprInfo masked_value = make_generated_binary_expr(
      OP_AMP, "&", field.type, value_mask_expr, converted);
  if(field.bit_offset != 0) {
    masked_value = make_generated_binary_expr(
        OP_LSHIFT, "<<", field.type, masked_value,
        make_expr_from_node(field.type, VC_PRVALUE,
                            make_typed_integer_literal_node(field.type,
                                                            field.bit_offset)));
  }

  if(first_container_write) {
    out.children.push_back(make_assignment_statement(storage, std::move(masked_value)));
    return;
  }

  const unsigned long long full_mask = low_bits_mask(storage_bits);
  const unsigned long long storage_mask = value_mask << field.bit_offset;
  const unsigned long long clear_mask = full_mask ^ storage_mask;
  ExprInfo cleared_storage = make_generated_binary_expr(
      OP_AMP, "&", field.type, storage,
      make_expr_from_node(field.type, VC_PRVALUE,
                          make_typed_integer_literal_node(field.type, clear_mask)));
  ExprInfo merged = make_generated_binary_expr(OP_BOR, "|", field.type,
                                               cleared_storage, masked_value);
  out.children.push_back(make_assignment_statement(storage, std::move(merged)));
}

void append_bit_field_copy_action(const ExprInfo & target_base,
                                  const ExprInfo & source_base,
                                  const FieldInfo & field,
                                  DumpNode & out,
                                  std::set<BitFieldStorageKey> & copied_storage)
{
  if(field.bit_width == 0 ||
     !copied_storage.insert(bit_field_storage_key(field)).second) {
    return;
  }

  ExprInfo target_storage = make_bit_field_storage_expr(target_base, field);
  ExprInfo source_storage = make_bit_field_storage_expr(source_base, field);
  out.children.push_back(
      make_assignment_statement(target_storage, std::move(source_storage)));
}

void append_bit_field_initialization_actions(SemanticContext & ctx,
                                             Scope & scope,
                                             const FieldInfo & field,
                                             const CppAstNode * initializer,
                                             const ExprInfo & base,
                                             DumpNode & out,
                                             std::set<BitFieldStorageKey> & zeroed_storage)
{
  if(!initializer) {
    ensure_bit_field_storage_zeroed(ctx, base, field, out, zeroed_storage);
    return;
  }

  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(!payload) {
    ensure_bit_field_storage_zeroed(ctx, base, field, out, zeroed_storage);
    return;
  }

  if(payload->kind == CppAstKind::paren_argument_list ||
     payload->kind == CppAstKind::paren_initializer) {
    vector<const CppAstNode *> args = initializer_argument_nodes(*payload);
    if(args.empty()) {
      ensure_bit_field_storage_zeroed(ctx, base, field, out, zeroed_storage);
      return;
    }
    if(args.size() != 1) {
      throw logic_error("non-class member initializer requires one expression");
    }
    ExprInfo init = ctx.analyze_expression_for_target(scope, *args[0], field.type);
    if(!can_copy_initialize(ctx, field.type, init)) {
      throw logic_error("invalid member initializer");
    }
    append_bit_field_store_action(ctx, scope, base, field, init, out, &zeroed_storage);
    return;
  }

  if(payload->kind == CppAstKind::braced_init_list) {
    if(payload->children.empty()) {
      ensure_bit_field_storage_zeroed(ctx, base, field, out, zeroed_storage);
      return;
    }
    if(payload->children.size() != 1) {
      throw logic_error("scalar member braced-init-list requires one element");
    }
    ExprInfo init = ctx.analyze_expression_for_target(scope, payload->children[0], field.type);
    if(!can_copy_initialize(ctx, field.type, init)) {
      throw logic_error("invalid member initializer");
    }
    append_bit_field_store_action(ctx, scope, base, field, init, out, &zeroed_storage);
    return;
  }

  ExprInfo init = ctx.analyze_expression_for_target(scope, *payload, field.type);
  if(!can_copy_initialize(ctx, field.type, init)) {
    throw logic_error("invalid member initializer");
  }
  append_bit_field_store_action(ctx, scope, base, field, init, out, &zeroed_storage);
}

void append_value_initialization_actions(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         const ExprInfo & target,
                                         DumpNode & out);

void append_target_initialization_actions(SemanticContext & ctx,
                                          Scope & scope,
                                          const TypePtr & type,
                                          const CppAstNode * initializer,
                                          const ExprInfo & target,
                                          DumpNode & out,
                                          const std::string & target_use_location,
                                          const std::string & source_witness_target_name);

ExprInfo make_reference_storage_target(const ExprInfo & target)
{
  TypePtr referent = remove_reference_type(target.type);
  if(!referent) {
    throw logic_error("reference storage target requires reference type");
  }
  if(target.node.kind != CallSemKind::member_expression) {
    throw logic_error("reference storage target unsupported");
  }

  ExprInfo storage = target;
  storage.type = make_pointer(referent);
  storage.category = VC_LVALUE;
  storage.node.semantic_type = storage.type;
  storage.node.value_category = CVC_LVALUE;
  storage.node.is_reference_storage_target = true;
  return storage;
}

bool reference_storage_source_is_addressable(const TypePtr & target_type,
                                             const ExprInfo & source)
{
  if(source.category == VC_LVALUE || is_reference_type(source.type)) {
    return true;
  }
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base || source.category != VC_XVALUE) {
    return false;
  }
  if(target_base->kind == Type::TK_RVALUE_REFERENCE) {
    return true;
  }
  return target_base->kind == Type::TK_LVALUE_REFERENCE &&
         is_const_object_type(target_base->inner);
}

void append_reference_initialization_actions(SemanticContext & ctx,
                                             Scope & scope,
                                             const TypePtr & type,
                                             const CppAstNode * initializer,
                                             const ExprInfo & target,
                                             DumpNode & out)
{
  if(!initializer) {
    throw logic_error("reference member requires initializer");
  }

  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(!payload) {
    throw logic_error("reference member requires initializer");
  }

  const CppAstNode * source_expr = payload;
  std::deque<CppAstNode> synthesized_nodes;
  if(payload->kind == CppAstKind::paren_argument_list ||
     payload->kind == CppAstKind::paren_initializer ||
     payload->kind == CppAstKind::braced_init_list) {
    const vector<const CppAstNode *> expanded_args =
        expand_initializer_argument_nodes(ctx,
                                          scope,
                                          initializer_argument_nodes(*payload),
                                          synthesized_nodes);
    if(expanded_args.size() != 1) {
      throw logic_error("reference member initializer requires one expression");
    }
    source_expr = expanded_args[0];
  } else if(payload->kind == CppAstKind::pack_expansion_expression) {
    const vector<const CppAstNode *> expanded_args =
        expand_initializer_argument_nodes(ctx,
                                          scope,
                                          vector<const CppAstNode *>(1, payload),
                                          synthesized_nodes);
    if(expanded_args.size() != 1) {
      throw logic_error("reference member initializer requires one expression");
    }
    source_expr = expanded_args[0];
  }

  ExprInfo init = ctx.analyze_expression_for_target(scope, *source_expr, type);
  if(!can_copy_initialize(ctx, type, init)) {
    throw logic_error("invalid member initializer");
  }
  if(!reference_storage_source_is_addressable(type, init)) {
    throw logic_error("reference member initializer requires lvalue");
  }

  ExprInfo storage = make_reference_storage_target(target);
  out.children.push_back(make_assignment_statement(storage, ctx.make_address_of_expr(init)));
}

void append_reference_binding_action(SemanticContext & ctx,
                                     const ExprInfo & target,
                                     const ExprInfo & source,
                                     DumpNode & out)
{
  if(!is_reference_type(target.type)) {
    throw logic_error("reference binding target must be reference type");
  }
  if(!reference_storage_source_is_addressable(target.type, source)) {
    throw logic_error("reference binding source requires lvalue");
  }

  ExprInfo storage = make_reference_storage_target(target);
  out.children.push_back(make_assignment_statement(storage, ctx.make_address_of_expr(source)));
}

void append_value_initialization_actions(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         const ExprInfo & target,
                                         DumpNode & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("invalid value initialization target");
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    throw logic_error("reference value initialization unsupported");
  }

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      throw logic_error("value initialization requires bounded array");
    }
    for(size_t i = 0; i < base->bound; ++i) {
      append_value_initialization_actions(ctx, scope, base->inner,
                                          ctx.make_subscript_expr(target, i, base->inner), out);
    }
    return;
  }

  ClassInfo * info = ctx.class_info_for_type(type);
  if(!info) {
    info = ctx.complete_class_type(type);
  }
  if(info) {
    if(ctx.can_synthesize_aggregate_constructor(*info)) {
      std::set<BitFieldStorageKey> zeroed_bit_storage;
      for(size_t i = 0; i < info->fields.size(); ++i) {
        if(info->fields[i].is_bit_field) {
          append_bit_field_initialization_actions(ctx, scope, info->fields[i],
                                                  info->fields[i].default_initializer,
                                                  target, out, zeroed_bit_storage);
        } else {
          ExprInfo field_expr = ctx.make_field_expr(target, info->fields[i]);
          if(info->fields[i].default_initializer) {
            append_target_initialization_actions(ctx, scope, info->fields[i].type,
                                                 info->fields[i].default_initializer,
                                                 field_expr, out);
          } else {
            append_value_initialization_actions(ctx, scope, info->fields[i].type,
                                                field_expr, out);
          }
        }
      }
      return;
    }
    append_constructor_call_action(ctx, scope, *info, vector<const CppAstNode *>(),
                                   ctx.make_address_of_expr(target),
                                   nullptr,
                                   ConstructorSelectionOptions(),
                                   nullptr,
                                   true,
                                   out);
    return;
  }

  out.children.push_back(make_assignment_statement(target, ctx.make_value_initialized_expr(type)));
}

void append_default_subobject_initialization_actions(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const TypePtr & type,
                                                     const ExprInfo & target,
                                                     DumpNode & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("invalid default initialization target");
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    throw logic_error("reference member requires initializer");
  }

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      throw logic_error("default initialization requires bounded array");
    }
    for(size_t i = 0; i < base->bound; ++i) {
      append_default_subobject_initialization_actions(
          ctx,
          scope,
          base->inner,
          ctx.make_subscript_expr(target, i, base->inner),
          out);
    }
    return;
  }

  ClassInfo * info = ctx.class_info_for_type(type);
  if(info) {
    append_constructor_call_action(ctx, scope, *info, vector<const CppAstNode *>(),
                                   ctx.make_address_of_expr(target),
                                   nullptr,
                                   ConstructorSelectionOptions(),
                                   nullptr,
                                   false,
                                   out);
  }
}

void append_target_initialization_actions(SemanticContext & ctx,
                                          Scope & scope,
                                          const TypePtr & type,
                                          const CppAstNode * initializer,
                                          const ExprInfo & target,
                                          DumpNode & out,
                                          const std::string & target_use_location,
                                          const std::string & source_witness_target_name)
{
  if(!initializer) {
    append_default_subobject_initialization_actions(ctx, scope, type, target, out);
    return;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("invalid initialization target");
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    append_reference_initialization_actions(ctx, scope, type, initializer, target, out);
    return;
  }

  const CppAstNode * payload = unwrap_initializer_payload(initializer);

  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      throw logic_error("array member initializer requires bounded array");
    }
    if(payload) {
      DumpNode init_node = make_dump_node(CallSemKind::braced_init_list);
      init_node.semantic_type = type;
      init_node.value_category = CVC_LVALUE;
      if(build_string_literal_array_initializer(type, *payload, init_node)) {
        for(size_t i = 0; i < init_node.children.size(); ++i) {
          out.children.push_back(
              make_assignment_statement(
                  ctx.make_subscript_expr(target, i, base->inner),
                  make_expr_from_node(base->inner, VC_PRVALUE, init_node.children[i])));
        }
        return;
      }
    }
    if(payload &&
       (payload->kind == CppAstKind::paren_argument_list ||
        payload->kind == CppAstKind::paren_initializer) &&
       initializer_argument_nodes(*payload).empty()) {
      append_value_initialization_actions(ctx, scope, type, target, out);
      return;
    }
    if(!payload || payload->kind != CppAstKind::braced_init_list) {
      throw logic_error("array member initializer requires braced-init-list");
    }
    CppAstNode expanded_payload;
    payload = braced_init_list_with_expanded_packs(ctx, scope, *payload, expanded_payload);
    vector<const CppAstNode *> element_initializers;
    vector<CppAstNode> synthesized_nodes;
    if(braced_init_list_has_designators(*payload)) {
      if(!build_array_initializer_plan(ctx,
                                       scope,
                                       type,
                                       *payload,
                                       element_initializers,
                                       synthesized_nodes)) {
        throw logic_error("invalid designated array initializer");
      }
    } else {
      if(payload->children.size() > base->bound) {
        throw logic_error("too many array member initializer elements");
      }
      element_initializers.assign(base->bound, nullptr);
      for(size_t i = 0; i < payload->children.size(); ++i) {
        element_initializers[i] = &payload->children[i];
      }
    }
    for(size_t i = 0; i < base->bound; ++i) {
      if(element_initializers[i]) {
        append_target_initialization_actions(ctx,
                                             scope,
                                             base->inner,
                                             element_initializers[i],
                                             ctx.make_subscript_expr(target, i, base->inner),
                                             out);
        continue;
      }
      append_value_initialization_actions(ctx, scope, base->inner,
                                          ctx.make_subscript_expr(target, i, base->inner), out);
    }
    return;
  }

  ClassInfo * info = ctx.class_info_for_type(type);
  if(info && payload && payload->kind == CppAstKind::braced_init_list &&
     ctx.can_synthesize_aggregate_constructor(*info)) {
    CppAstNode expanded_payload;
    payload = braced_init_list_with_expanded_packs(ctx, scope, *payload, expanded_payload);
    const std::size_t aggregate_count = semantic_class_model::aggregate_element_count(*info);
    vector<const CppAstNode *> field_initializers;
    vector<CppAstNode> synthesized_nodes;
    if(!build_aggregate_initializer_plan(ctx,
                                         scope,
                                         *info,
                                         *payload,
                                         field_initializers,
                                         synthesized_nodes)) {
      if(braced_init_list_has_designators(*payload)) {
        throw logic_error("invalid designated aggregate initializer");
      }
      if(payload->children.size() > aggregate_count) {
        throw logic_error("too many aggregate member initializer elements");
      }
      throw logic_error("invalid aggregate initializer");
    }
    std::set<BitFieldStorageKey> zeroed_bit_storage;
    if(info->class_kind == "union") {
      size_t selected_index = info->fields.size();
      for(size_t i = 0; i < aggregate_count && i < field_initializers.size(); ++i) {
        if(field_initializers[i]) {
          selected_index = i;
          break;
        }
      }
      if(selected_index == info->fields.size()) {
        selected_index = 0;
      }
      const FieldInfo * input_field =
          semantic_class_model::aggregate_input_field(ctx, info->fields[selected_index]);
      const CppAstNode * initializer =
          field_initializers[selected_index] ?
              field_initializers[selected_index] :
              (input_field ? input_field->default_initializer : nullptr);
      append_field_initialization_actions(ctx,
                                          scope,
                                          info->fields[selected_index],
                                          initializer,
                                          target,
                                          out,
                                          zeroed_bit_storage);
      return;
    }

    for(size_t i = 0; i < info->fields.size(); ++i) {
      const FieldInfo * input_field =
          semantic_class_model::aggregate_input_field(ctx, info->fields[i]);
      const CppAstNode * initializer =
          i < field_initializers.size() && field_initializers[i] ?
              field_initializers[i] :
              (input_field ? input_field->default_initializer : nullptr);
      append_field_initialization_actions(ctx,
                                          scope,
                                          info->fields[i],
                                          initializer,
                                          target,
                                          out,
                                          zeroed_bit_storage);
      if(i + 1 >= aggregate_count) {
        break;
      }
    }
    return;
  }

  if(info) {
    const bool is_copy_list_initialization =
        initializer_uses_copy_initialization(initializer) &&
        payload &&
        payload->kind == CppAstKind::braced_init_list;
    const bool empty_same_type_function_style =
        payload &&
        is_empty_same_type_function_style_constructor_call(ctx,
                                                           scope,
                                                           type,
                                                           *payload);
    bool uses_function_style_constructor_args = false;
    vector<const CppAstNode *> args;
    const CppAstNode * direct_braced_init = nullptr;
    if(payload) {
      ExprInfo direct_init;
      if(!is_copy_list_initialization &&
         analyze_direct_class_materialization_initializer(ctx,
                                                          scope,
                                                          type,
                                                          initializer,
                                                          direct_init)) {
        validate_elided_direct_materialization_constructor(ctx,
                                                           scope,
                                                           *info,
                                                           *initializer,
                                                           direct_init);
        note_elided_direct_materialization_constructor_witness(ctx,
                                                              scope,
                                                              *info,
                                                              *initializer,
                                                              direct_init);
        append_direct_materialization_initialization_action(ctx,
                                                           target,
                                                           std::move(direct_init),
                                                           out);
        return;
      }
      if(extract_function_style_constructor_args(ctx, scope, type, *payload, args)) {
        uses_function_style_constructor_args = true;
      } else {
        args = initializer_argument_nodes(*payload);
      }
      if(payload->kind == CppAstKind::braced_init_list) {
        direct_braced_init = payload;
      }
    }
    ConstructorSelectionOptions ctor_options =
        class_initializer_constructor_options(initializer,
                                             payload,
                                             uses_function_style_constructor_args);
    if(!target_use_location.empty()) {
      ctor_options.source_witness_location = target_use_location;
      ctor_options.source_witness_direct_construction = true;
    }
    append_constructor_call_action(ctx,
                                   scope,
                                   *info,
                                   args,
                                   ctx.make_address_of_expr(target),
                                   direct_braced_init,
                                   ctor_options,
                                   nullptr,
                                   initializer_is_empty_value_initializer(initializer) ||
                                       empty_same_type_function_style,
                                   out,
                                   source_witness_target_name);
    return;
  }

  if(payload &&
     (payload->kind == CppAstKind::paren_argument_list ||
      payload->kind == CppAstKind::paren_initializer)) {
    std::deque<CppAstNode> synthesized_nodes;
    vector<const CppAstNode *> args =
        expand_initializer_argument_nodes(ctx,
                                          scope,
                                          initializer_argument_nodes(*payload),
                                          synthesized_nodes);
    if(args.empty()) {
      out.children.push_back(make_assignment_statement(target, ctx.make_value_initialized_expr(type)));
      return;
    }
    if(args.size() != 1) {
      throw logic_error("non-class member initializer requires one expression");
    }
    ExprInfo init =
        analyze_initializer_expression_for_target(ctx, scope, *args[0], type, true);
    if(!can_copy_initialize(ctx, type, init)) {
      if(parser_trace::enabled("lifetime.init")) {
        std::ostringstream trace;
        trace << "init-fail kind=paren target_type=" << describe_type(type)
              << " init_type=" << describe_type(init.type)
              << " init_category=" << static_cast<int>(init.category)
              << " target_expr=" << target.node.text;
        parser_trace::note("lifetime.init", std::string(), trace.str());
      }
      throw logic_error("invalid member initializer");
    }
    out.children.push_back(make_assignment_statement(target, std::move(init)));
    return;
  }

  if(payload && payload->kind == CppAstKind::braced_init_list) {
    std::deque<CppAstNode> synthesized_nodes;
    vector<const CppAstNode *> args =
        expand_initializer_argument_nodes(ctx,
                                          scope,
                                          initializer_argument_nodes(*payload),
                                          synthesized_nodes);
    if(args.empty()) {
      out.children.push_back(make_assignment_statement(target, ctx.make_value_initialized_expr(type)));
      return;
    }
    if(args.size() != 1) {
      throw logic_error("scalar member braced-init-list requires one element");
    }
    ExprInfo source_expr;
    if(braced_scalar_initialization_has_narrowing_conversion(ctx, scope, *args[0], type, source_expr)) {
      throw make_narrowing_initializer_error(type, source_expr, *args[0]);
    }
    ExprInfo init =
        analyze_initializer_expression_for_target(ctx, scope, *args[0], type, true);
    if(!can_copy_initialize(ctx, type, init)) {
      if(parser_trace::enabled("lifetime.init")) {
        std::ostringstream trace;
        trace << "init-fail kind=brace target_type=" << describe_type(type)
              << " init_type=" << describe_type(init.type)
              << " init_category=" << static_cast<int>(init.category)
              << " target_expr=" << target.node.text;
        parser_trace::note("lifetime.init", std::string(), trace.str());
      }
      throw logic_error("invalid member initializer");
    }
    out.children.push_back(make_assignment_statement(target, std::move(init)));
    return;
  }

  if(payload && payload->kind == CppAstKind::pack_expansion_expression) {
    std::deque<CppAstNode> synthesized_nodes;
    vector<const CppAstNode *> args =
        expand_initializer_argument_nodes(ctx,
                                          scope,
                                          vector<const CppAstNode *>(1, payload),
                                          synthesized_nodes);
    if(args.empty()) {
      out.children.push_back(make_assignment_statement(target, ctx.make_value_initialized_expr(type)));
      return;
    }
    if(args.size() != 1) {
      throw logic_error("non-class member initializer requires one expression");
    }
    payload = args[0];
  }

  ExprInfo init = ctx.analyze_expression_for_target(scope, *payload, type);
  if(!can_copy_initialize(ctx, type, init)) {
    if(parser_trace::enabled("lifetime.init")) {
      std::ostringstream trace;
      trace << "init-fail kind=direct target_type=" << describe_type(type)
            << " init_type=" << describe_type(init.type)
            << " init_category=" << static_cast<int>(init.category)
            << " target_expr=" << target.node.text;
      parser_trace::note("lifetime.init", std::string(), trace.str());
    }
    throw logic_error("invalid member initializer");
  }
  out.children.push_back(make_assignment_statement(target, std::move(init)));
}

void append_copy_constructor_action(SemanticContext & ctx,
                                    Scope & scope,
                                    ClassInfo & info,
                                    const ExprInfo & object_ptr,
                                    const ExprInfo & source,
                                    DumpNode & out,
                                    ClassInfo * vtt_owner_info = nullptr)
{
  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             object_ptr,
                             source,
                             out,
                             UnionStorageTransferKind::copy_constructor);
    return;
  }
  FunctionBinding * ctor = copy_constructor_for(info);
  if(!ctor) {
    ctor = ctx.ensure_implicit_copy_constructor(info);
  }
  if(!ctor) {
    throw logic_error("missing copy constructor");
  }
  vector<ExprInfo> call_args;
  call_args.push_back(object_ptr);
  call_args.push_back(source);
  const bool trivial_ctor = is_trivial_constructor_binding(ctx, *ctor);
  constructor_lifecycle_service::ConstructorActionResult action_result =
      constructor_lifecycle_service::prepare_lifecycle_call(ctx,
                                                            ctor,
                                                            call_args,
                                                            trivial_ctor,
                                                            OutputReason::ConstructorUse);
  DumpNode action = make_dump_node(CallSemKind::constructor_action, ctor->name);
  set_callsem_resolved_name(action, function_output_name(*ctor));
  action.trivial_lifecycle = action_result.trivial_lifecycle;
  action.children.push_back(std::move(action_result.call_expr.node));
  annotate_special_member_call_with_vtt(ctx, vtt_owner_info, info, action.children.back());
  out.children.push_back(std::move(action));
}

void append_move_constructor_action(SemanticContext & ctx,
                                    Scope & scope,
                                    ClassInfo & info,
                                    const ExprInfo & object_ptr,
                                    const ExprInfo & source,
                                    DumpNode & out,
                                    ClassInfo * vtt_owner_info = nullptr)
{
  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             object_ptr,
                             source,
                             out,
                             UnionStorageTransferKind::move_constructor);
    return;
  }

  vector<ExprInfo> source_args(1, make_xvalue_expr(source));
  constructor_lifecycle_service::ConstructorSelectionResult selection;
  try
  {
    const ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::direct_initialization_profile(
                "move constructor action"));
    selection = constructor_lifecycle_service::select_constructor_from_exprs(
        ctx,
        scope,
        info,
        source_args,
        ctor_options);
  }
  catch(const logic_error &)
  {
    selection = constructor_lifecycle_service::ConstructorSelectionResult();
  }

  FunctionBinding * ctor = selection.ctor;
  if(!ctor) {
    append_copy_constructor_action(ctx, scope, info, object_ptr, source, out, vtt_owner_info);
    return;
  }

  const bool trivial_ctor = ctor && is_trivial_constructor_binding(ctx, *ctor);
  constructor_lifecycle_service::ConstructorActionResult action_result =
      constructor_lifecycle_service::prepare_selected_constructor_action(
          ctx,
          object_ptr,
          selection,
          trivial_ctor,
          OutputReason::ConstructorUse);
  DumpNode action = make_dump_node(CallSemKind::constructor_action, ctor->name);
  set_callsem_resolved_name(action, function_output_name(*ctor));
  action.trivial_lifecycle = action_result.trivial_lifecycle;
  action.children.push_back(std::move(action_result.call_expr.node));
  annotate_special_member_call_with_vtt(ctx, vtt_owner_info, info, action.children.back());
  out.children.push_back(std::move(action));
}

bool require_destructor_function_if_needed(SemanticContext & ctx,
                                           ClassInfo & info,
                                           FunctionBinding * dtor,
                                           bool allow_host_abi_skip)
{
  if(!dtor) {
    return false;
  }
  if(allow_host_abi_skip &&
     can_skip_destructor_action_for_host_abi(ctx, info)) {
    if(info.source_template || !info.instantiation_arguments.empty()) {
      constructor_lifecycle_service::require_lifecycle_function(ctx,
                                                                dtor,
                                                                OutputReason::SyntheticDependency);
    } else if(template_api::class_has_template_identity(&info)) {
      note_skipped_template_lifecycle_definition(ctx, dtor);
    }
    return false;
  }
  constructor_lifecycle_service::require_lifecycle_function(ctx,
                                                            dtor,
                                                            OutputReason::SyntheticDependency);
  return true;
}

void append_destructor_action(SemanticContext & ctx,
                              ClassInfo & info,
                              const ExprInfo & object_ptr,
                              ClassInfo * vtt_owner_info,
                              bool allow_host_abi_skip,
                              DumpNode & out)
{
  FunctionBinding * dtor = destructor_for(info);
  if(!require_destructor_function_if_needed(ctx, info, dtor, allow_host_abi_skip)) {
    return;
  }
  vector<ExprInfo> args(1, object_ptr);
  DumpNode call = ctx.make_direct_call_expr(*dtor, args).node;
  annotate_special_member_call_with_vtt(ctx, vtt_owner_info, info, call);
  append_wrapped_action(out,
                        CallSemKind::destructor_action,
                        dtor->name,
                        function_output_name(*dtor),
                        std::move(call));
}

bool subobject_may_need_destructor_action(SemanticContext & ctx,
                                          const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_array_type(base)) {
    return base->has_bound &&
           subobject_may_need_destructor_action(ctx, base->inner);
  }
  return ctx.class_info_for_type(base) != nullptr;
}

void append_destructor_actions_for_subobject(SemanticContext & ctx,
                                             const TypePtr & type,
                                             const ExprInfo & object_expr,
                                             ClassInfo * vtt_owner_info,
                                             bool allow_host_abi_skip,
                                             DumpNode & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return;
  }
  if(is_array_type(base)) {
    if(!base->has_bound) {
      return;
    }
    for(size_t i = base->bound; i-- > 0;) {
      append_destructor_actions_for_subobject(
          ctx,
          base->inner,
          ctx.make_subscript_expr(object_expr, i, base->inner),
          vtt_owner_info,
          allow_host_abi_skip,
          out);
    }
    return;
  }
  ClassInfo * field_class = ctx.class_info_for_type(base);
  if(!field_class) {
    return;
  }
  append_destructor_action(ctx,
                           *field_class,
                           ctx.make_address_of_expr(object_expr),
                           vtt_owner_info,
                           allow_host_abi_skip,
                           out);
}

void append_copy_assignment_action(SemanticContext & ctx,
                                   Scope & scope,
                                   ClassInfo & info,
                                   const ExprInfo & lhs_ptr,
                                   const ExprInfo & rhs,
                                   DumpNode & out)
{
  if(ctx.is_empty_class_info(&info) &&
     is_trivially_copy_assignable_type(ctx, info.type)) {
    return;
  }
  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             lhs_ptr,
                             rhs,
                             out,
                             UnionStorageTransferKind::copy_assignment);
    return;
  }
  FunctionBinding * op = copy_assignment_for(info);
  if(!op) {
    op = ctx.ensure_implicit_copy_assignment(info);
  }
  if(!op) {
    throw logic_error("missing copy assignment");
  }
  vector<ExprInfo> args;
  args.push_back(lhs_ptr);
  args.push_back(rhs);
  constructor_lifecycle_service::require_lifecycle_function(ctx,
                                                            op,
                                                            OutputReason::SyntheticDependency);
  DumpNode stmt = make_dump_node(CallSemKind::expression_statement);
  ExprInfo call = ctx.make_direct_call_expr(*op, args);
  stmt.children.push_back(std::move(call.node));
  out.children.push_back(std::move(stmt));
}

void append_move_assignment_action(SemanticContext & ctx,
                                   Scope & scope,
                                   ClassInfo & info,
                                   const ExprInfo & lhs_ptr,
                                   const ExprInfo & rhs,
                                   DumpNode & out)
{
  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             lhs_ptr,
                             rhs,
                             out,
                             UnionStorageTransferKind::move_assignment);
    return;
  }
  FunctionBinding * op = move_assignment_for(info);
  if(!op) {
    op = ctx.ensure_implicit_move_assignment(info);
  }
  if(!op) {
    op = copy_assignment_for(info);
  }
  if(!op) {
    op = ctx.ensure_implicit_copy_assignment(info);
  }
  if(!op) {
    throw logic_error("missing move assignment");
  }
  vector<ExprInfo> args;
  args.push_back(lhs_ptr);
  args.push_back(make_xvalue_expr(rhs));
  constructor_lifecycle_service::require_lifecycle_function(ctx,
                                                            op,
                                                            OutputReason::SyntheticDependency);
  DumpNode stmt = make_dump_node(CallSemKind::expression_statement);
  ExprInfo call = ctx.make_direct_call_expr(*op, args);
  stmt.children.push_back(std::move(call.node));
  out.children.push_back(std::move(stmt));
}

}  // namespace

void require_destructor_action_if_needed(SemanticContext & ctx,
                                         const TypePtr & type,
                                         bool allow_host_abi_skip)
{
  TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
  ClassInfo * info = ctx.complete_class_type(object_type);
  if(!info) {
    return;
  }
  FunctionBinding * dtor = destructor_for(*info);
  require_destructor_function_if_needed(ctx, *info, dtor, allow_host_abi_skip);
}

void require_reference_bound_temporary_destructor_if_needed(
    SemanticContext & ctx,
    const TypePtr & target,
    const ExprInfo & expr)
{
  if(expr.category != VC_PRVALUE) {
    return;
  }
  TypePtr target_base = strip_top_level_cv(target);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return;
  }
  TypePtr referent = strip_top_level_cv(remove_reference_type(target));
  TypePtr expr_object_type = strip_top_level_cv(remove_reference_type(expr.type));
  if(!referent || !expr_object_type) {
    return;
  }
  if(!type_equals(referent, expr_object_type) &&
     semantic_conversion::inheritance_conversion_rank(ctx, target, expr) == CR_BAD) {
    return;
  }
  require_destructor_action_if_needed(ctx, expr_object_type);
}

void note_constructor_witness_closure(SemanticContext & ctx,
                                      FunctionBinding * ctor)
{
  note_constructor_witness_closure_impl(ctx, ctor);
}

bool has_designated_braced_init(const CppAstNode & node)
{
  return braced_init_list_has_designators(node);
}

bool build_aggregate_initializer_plan(SemanticContext & ctx,
                                      Scope & scope,
                                      const ClassInfo & info,
                                      const CppAstNode & payload,
                                      vector<const CppAstNode *> & field_initializers,
                                      vector<CppAstNode> & synthesized_nodes)
{
  return build_aggregate_initializer_plan_impl(ctx,
                                               scope,
                                               info,
                                               payload,
                                               field_initializers,
                                               synthesized_nodes);
}

bool build_designated_aggregate_initializer_plan(
    SemanticContext & ctx,
    Scope & scope,
    const ClassInfo & info,
    const CppAstNode & payload,
    vector<const CppAstNode *> & field_initializers,
    vector<CppAstNode> & synthesized_nodes)
{
  return build_aggregate_initializer_plan_impl(ctx,
                                               scope,
                                               info,
                                               payload,
                                               field_initializers,
                                               synthesized_nodes);
}

bool build_aggregate_constructor_source_args(SemanticContext & ctx,
                                             Scope & scope,
                                             ClassInfo & info,
                                             const CppAstNode & payload,
                                             vector<ExprInfo> & source_args)
{
  if(payload.kind != CppAstKind::braced_init_list ||
     !ctx.can_synthesize_aggregate_constructor(info)) {
    return false;
  }

  CppAstNode expanded_payload;
  const CppAstNode * effective_payload =
      braced_init_list_with_expanded_packs(ctx, scope, payload, expanded_payload);
  const size_t aggregate_count = semantic_class_model::aggregate_element_count(info);
  vector<const CppAstNode *> field_initializers;
  vector<CppAstNode> synthesized_nodes;
  if(!build_aggregate_initializer_plan(ctx,
                                       scope,
                                       info,
                                       *effective_payload,
                                       field_initializers,
                                       synthesized_nodes)) {
    return false;
  }

  const auto append_field_arg =
      [&](const FieldInfo & field,
          const CppAstNode * initializer) -> void
      {
        const FieldInfo * input_field =
            semantic_class_model::aggregate_input_field(ctx, field);
        const CppAstNode * effective_initializer =
            initializer ? initializer :
                          (input_field ? input_field->default_initializer : nullptr);
        const TypePtr param_type = input_field ? input_field->type : field.type;
        const CppAstNode * payload_node =
            unwrap_initializer_payload(effective_initializer);
        source_args.push_back(payload_node ?
                                  ctx.analyze_expression_for_target(scope,
                                                                    *payload_node,
                                                                    param_type) :
                                  make_aggregate_default_field_expr(ctx,
                                                                    scope,
                                                                    param_type));
      };

  source_args.clear();
  if(info.class_kind == "union") {
    size_t selected_index = info.fields.size();
    for(size_t i = 0; i < aggregate_count && i < field_initializers.size(); ++i) {
      if(field_initializers[i]) {
        selected_index = i;
        break;
      }
    }
    if(selected_index == info.fields.size()) {
      selected_index = 0;
    }
    if(selected_index >= info.fields.size()) {
      return false;
    }
    append_field_arg(info.fields[selected_index],
                     selected_index < field_initializers.size() ?
                         field_initializers[selected_index] :
                         nullptr);
    return true;
  }

  source_args.reserve(aggregate_count);
  for(size_t i = 0; i < aggregate_count && i < info.fields.size(); ++i) {
    append_field_arg(info.fields[i],
                     i < field_initializers.size() ? field_initializers[i] : nullptr);
  }
  return true;
}

void analyze_initializer(SemanticContext & ctx,
                         Scope & scope,
                         const TypePtr & type,
                         const CppAstNode & node,
                         DumpNode & out)
{
  if(node.kind != CppAstKind::initializer || node.children.size() != 1) {
    throw logic_error("unsupported initializer");
  }

  const CppAstNode & payload = node.children[0];
  TypePtr base = strip_top_level_cv(type);
  if(base && base->kind == Type::TK_ARRAY) {
    DumpNode init_node = make_dump_node(CallSemKind::braced_init_list);
    init_node.semantic_type = type;
    init_node.value_category = CVC_LVALUE;
    if(build_string_literal_array_initializer(type, payload, init_node)) {
      out.children.push_back(std::move(init_node));
      return;
    }
    if(payload.kind != CppAstKind::braced_init_list) {
      throw logic_error("array initializer requires braced-init-list");
    }
    CppAstNode expanded_payload;
    const CppAstNode * effective_payload =
        braced_init_list_with_expanded_packs(ctx, scope, payload, expanded_payload);
    vector<const CppAstNode *> element_initializers;
    vector<CppAstNode> synthesized_nodes;
    if(braced_init_list_has_designators(*effective_payload)) {
      if(!build_array_initializer_plan(ctx,
                                       scope,
                                       type,
                                       *effective_payload,
                                       element_initializers,
                                       synthesized_nodes)) {
        throw logic_error("invalid designated array initializer");
      }
    } else {
      if(effective_payload->children.size() > base->bound) {
        throw logic_error("too many array initializer elements");
      }
      element_initializers.assign(base->bound, nullptr);
      for(size_t i = 0; i < effective_payload->children.size(); ++i) {
        element_initializers[i] = &effective_payload->children[i];
      }
    }
    for(size_t i = 0; i < base->bound; ++i) {
      if(!element_initializers[i]) {
        TypePtr element_base = strip_top_level_cv(remove_reference_type(base->inner));
        if(element_base &&
           (element_base->kind == Type::TK_ARRAY || ctx.complete_class_type(element_base))) {
          CppAstNode empty;
          empty.kind = CppAstKind::braced_init_list;
          ExprInfo element = ctx.analyze_expression_for_target(scope, empty, base->inner);
          init_node.children.push_back(std::move(element.node));
        } else {
          ExprInfo element = ctx.make_value_initialized_expr(base->inner);
          init_node.children.push_back(std::move(element.node));
        }
        continue;
      }
      ExprInfo element =
          ctx.analyze_expression_for_target(scope, *element_initializers[i], base->inner);
      if(!can_copy_initialize(ctx, base->inner, element)) {
        throw logic_error("invalid array initializer element");
      }
      init_node.children.push_back(std::move(element.node));
    }
    out.children.push_back(std::move(init_node));
    return;
  }

  if(payload.kind == CppAstKind::braced_init_list) {
    ExprInfo target_aware;
    const ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::copy_list_initialization_profile(
                "copy-list-initialization"));
    if(ctx.try_analyze_target_aware_expression(scope,
                                               payload,
                                               type,
                                               target_aware,
                                               &ctor_options)) {
      out.children.push_back(std::move(target_aware.node));
      return;
    }
    if(payload.children.empty()) {
      ExprInfo value = ctx.make_value_initialized_expr(type);
      out.children.push_back(std::move(value.node));
      return;
    }
    if(payload.children.size() != 1) {
      throw logic_error("scalar braced-init-list requires one element");
    }
    ExprInfo source_expr;
    if(braced_scalar_initialization_has_narrowing_conversion(
           ctx, scope, payload.children[0], type, source_expr)) {
      throw make_narrowing_initializer_error(type, source_expr, payload.children[0]);
    }
    ExprInfo element =
        analyze_initializer_expression_for_target(ctx,
                                                  scope,
                                                  payload.children[0],
                                                  type,
                                                  !node.uses_assignment_form);
    if(!can_copy_initialize(ctx, type, element)) {
      ostringstream outmsg;
      outmsg << "invalid initializer";
      outmsg << " [target " << describe_type(type) << "]";
      outmsg << " [source " << describe_type(element.type) << "]";
      outmsg << " [init " << node_text(payload.children[0]) << "]";
      throw logic_error(outmsg.str());
    }
    out.children.push_back(std::move(element.node));
    return;
  }

  if(payload.kind == CppAstKind::paren_initializer ||
     payload.kind == CppAstKind::paren_argument_list) {
    if(payload.children.size() != 1) {
      throw logic_error("scalar paren-initializer requires one expression");
    }
    ExprInfo element =
        analyze_initializer_expression_for_target(ctx,
                                                  scope,
                                                  payload.children[0],
                                                  type,
                                                  true);
    if(!can_copy_initialize(ctx, type, element)) {
      ostringstream outmsg;
      outmsg << "invalid initializer";
      outmsg << " [target " << describe_type(type) << "]";
      outmsg << " [source " << describe_type(element.type) << "]";
      outmsg << " [init " << node_text(payload.children[0]) << "]";
      throw logic_error(outmsg.str());
    }
    out.children.push_back(std::move(element.node));
    return;
  }

  ExprInfo expr = ctx.analyze_expression_for_target(scope, payload, type);
  if(!can_copy_initialize(ctx, type, expr)) {
    ostringstream outmsg;
    outmsg << "invalid initializer";
    outmsg << " [target " << describe_type(type) << "]";
    outmsg << " [source " << describe_type(expr.type) << "]";
    outmsg << " [init " << node_text(payload) << "]";
    throw logic_error(outmsg.str());
  }

  out.children.push_back(std::move(expr.node));
}

void analyze_object_lifetime_actions(SemanticContext & ctx,
                                     Scope & scope,
                                     const string & name,
                                     const TypePtr & type,
                                     const CppAstNode * initializer,
                                     DumpNode & out,
                                     const std::string & object_use_location)
{
  TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
  if(object_type && object_type->kind == Type::TK_ARRAY) {
    TypePtr element_type = object_type;
    while(element_type && element_type->kind == Type::TK_ARRAY) {
      element_type = strip_top_level_cv(element_type->inner);
    }
    if(!element_type || !ctx.complete_class_type(element_type)) {
      return;
    }

    ExprInfo object = ctx.analyze_id_expression(scope, synthetic_identifier_node(name));
    append_target_initialization_actions(ctx,
                                         scope,
                                         type,
                                         initializer,
                                         object,
                                         out,
                                         object_use_location);
    append_destructor_actions_for_subobject(ctx, type, object, nullptr, true, out);
    return;
  }

  ClassInfo * info = ctx.complete_class_type(type);
  if(!info) {
    return;
  }

  ExprInfo object = ctx.analyze_id_expression(scope, synthetic_identifier_node(name));
  ExprInfo object_ptr = ctx.make_address_of_expr(object);
  const CppAstNode * payload = unwrap_initializer_payload(initializer);
  if(initializer &&
     payload &&
     payload->kind == CppAstKind::braced_init_list &&
     ctx.is_initializer_list_type(type, nullptr, nullptr)) {
    ExprInfo initlist;
    if(ctx.try_analyze_target_aware_expression(scope,
                                               *payload,
                                               type,
                                               initlist) &&
       type_equals(strip_top_level_cv(initlist.type),
                   strip_top_level_cv(type))) {
      out.children.push_back(std::move(initlist.node));
      append_destructor_action(ctx, *info, object_ptr, nullptr, true, out);
      return;
    }
  }
  if(initializer &&
     payload &&
     payload->kind == CppAstKind::braced_init_list &&
     ctx.can_synthesize_aggregate_constructor(*info)) {
    append_target_initialization_actions(ctx, scope, type, initializer, object, out);
    append_destructor_action(ctx, *info, object_ptr, nullptr, true, out);
    return;
  }

  vector<const CppAstNode *> ctor_args_override;
  bool has_ctor_args_override = false;
  const bool is_copy_list_initialization =
      initializer_uses_copy_initialization(initializer) &&
      payload &&
      payload->kind == CppAstKind::braced_init_list;
  const bool empty_same_type_function_style =
      payload &&
      is_empty_same_type_function_style_constructor_call(ctx,
                                                         scope,
                                                         type,
                                                         *payload);
  if(initializer && initializer->kind == CppAstKind::initializer &&
     initializer->children.size() == 1) {
    ExprInfo direct_init;
    if(!is_copy_list_initialization &&
       analyze_direct_class_materialization_initializer(ctx,
                                                        scope,
                                                        type,
                                                        initializer,
                                                        direct_init)) {
      validate_elided_direct_materialization_constructor(ctx,
                                                         scope,
                                                         *info,
                                                         *initializer,
                                                         direct_init);
      note_elided_direct_materialization_constructor_witness(ctx,
                                                            scope,
                                                            *info,
                                                            *initializer,
                                                            direct_init);
      out.children.push_back(std::move(direct_init.node));
      append_destructor_action(ctx, *info, object_ptr, nullptr, true, out);
      return;
    }
    if(extract_function_style_constructor_args(ctx, scope, type, *payload,
                                               ctor_args_override)) {
      has_ctor_args_override = true;
    }
  }

  vector<const CppAstNode *> args;
  const CppAstNode * direct_braced_init = nullptr;
  if(initializer) {
    args = has_ctor_args_override ? ctor_args_override : initializer_argument_nodes(*initializer);
    if(payload && payload->kind == CppAstKind::braced_init_list) {
      direct_braced_init = payload;
    }
  }
  ConstructorSelectionOptions ctor_options =
      class_initializer_constructor_options(initializer,
                                           payload,
                                           has_ctor_args_override);
  if(!initializer && !object_use_location.empty()) {
    ctor_options.use_location = object_use_location;
  }
  ctor_options.source_witness_direct_construction = true;
  const bool payload_is_literal =
      payload &&
      (payload->kind == CppAstKind::literal ||
       payload->kind == CppAstKind::keyword_literal);
  if(initializer &&
     (initializer->uses_assignment_form || payload_is_literal) &&
     (payload || (!args.empty() && args.front()))) {
    const CppAstNode * source_node =
        (!args.empty() && args.front()) ? args.front() : payload;
    ctor_options.source_witness_location =
        earliest_source_location_for_node(ctx, *source_node);
  } else if(initializer &&
            payload &&
            (payload->kind == CppAstKind::id_expression ||
             payload->kind == CppAstKind::literal ||
             payload->kind == CppAstKind::keyword_literal)) {
    ctor_options.source_witness_location =
        earliest_source_location_for_node(ctx, *payload);
  } else {
    ctor_options.source_witness_location = object_use_location;
  }
  append_constructor_call_action(ctx,
                                 scope,
                                 *info,
                                 args,
                                 object_ptr,
                                 direct_braced_init,
                                 ctor_options,
                                 nullptr,
                                 initializer &&
                                     (initializer_is_empty_value_initializer(initializer) ||
                                      empty_same_type_function_style),
                                 out);
  append_destructor_action(ctx, *info, object_ptr, nullptr, true, out);
}

void append_constructor_generated_statements(SemanticContext & ctx,
                                             Scope & scope,
                                             FunctionBinding & binding,
                                             symbol_linkage::SpecialMemberEntryPointKind entry_point_kind,
                                             DumpNode & function_node)
{
  if(!binding.owner_class) {
    return;
  }

  ExprInfo this_expr = analyze_generated_this_expr(ctx, scope);
  ClassInfo & info = *binding.owner_class;
  const bool construct_virtual_bases =
      entry_point_kind == symbol_linkage::SMEK_COMPLETE;

  if(binding_is_generated_copy_constructor(info, binding)) {
    if(binding.params.size() < 2) {
      throw logic_error("copy constructor missing source parameter");
    }
    ExprInfo source_expr =
        ctx.analyze_id_expression(scope, synthetic_identifier_node(
            synthetic_parameter_name(binding, 1)));
    if(construct_virtual_bases) {
      for(size_t i = 0; i < info.virtual_base_subobjects.size(); ++i) {
        const SubobjectInfo & subobject = info.virtual_base_subobjects[i];
        if(!subobject.type) {
          continue;
        }
        append_copy_constructor_action(ctx,
                                       scope,
                                       *subobject.type,
                                       ctx.make_base_pointer_expr(this_expr,
                                                                  *subobject.type,
                                                                  subobject.offset),
                                       ctx.make_base_expr(source_expr,
                                                          *subobject.type,
                                                          subobject.offset),
                                       function_node,
                                       &info);
      }
    }
    for(size_t i = 0; i < info.bases.size(); ++i) {
      if(info.bases[i].is_virtual) {
        continue;
      }
      append_copy_constructor_action(ctx,
                                     scope,
                                     *info.bases[i].type,
                                     ctx.make_base_pointer_expr(this_expr, *info.bases[i].type,
                                                                info.bases[i].offset),
                                     ctx.make_base_expr(source_expr, *info.bases[i].type,
                                                        info.bases[i].offset),
                                     function_node,
                                     &info);
    }
    if(info.class_kind == "union") {
      append_union_copy_action(ctx,
                               scope,
                               info,
                               this_expr,
                               source_expr,
                               function_node,
                               UnionStorageTransferKind::copy_constructor);
      return;
    }
    append_all_vptr_actions(ctx, info, this_expr, entry_point_kind, function_node);
    const LeadingTrivialStoragePrefix prefix =
        leading_trivial_storage_prefix(ctx, info, false, false);
    if(!prefix.empty()) {
      append_trivial_storage_prefix_copy_action(this_expr,
                                                source_expr,
                                                prefix.byte_count,
                                                function_node);
    }
    std::set<BitFieldStorageKey> copied_bit_storage;
    for(size_t i = prefix.field_count; i < info.fields.size(); ++i) {
      if(info.fields[i].is_bit_field) {
        append_bit_field_copy_action(this_expr, source_expr, info.fields[i],
                                     function_node, copied_bit_storage);
      } else {
        ExprInfo field_expr = ctx.make_field_expr(this_expr, info.fields[i]);
        ExprInfo source_field = ctx.make_field_expr(source_expr, info.fields[i]);
        ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
        if(is_reference_type(info.fields[i].type)) {
          append_reference_binding_action(ctx, field_expr, source_field, function_node);
        } else if(field_class) {
          append_copy_constructor_action(ctx, scope, *field_class,
                                         ctx.make_address_of_expr(field_expr),
                                         source_field, function_node);
        } else {
          function_node.children.push_back(
              make_assignment_statement(field_expr, std::move(source_field)));
        }
      }
    }
    return;
  }

  if(binding_is_generated_move_constructor(info, binding)) {
    ExprInfo source_expr =
        ctx.analyze_id_expression(scope, synthetic_identifier_node(
            synthetic_parameter_name(binding, 1)));
    if(construct_virtual_bases) {
      for(size_t i = 0; i < info.virtual_base_subobjects.size(); ++i) {
        const SubobjectInfo & subobject = info.virtual_base_subobjects[i];
        if(!subobject.type) {
          continue;
        }
        append_move_constructor_action(ctx,
                                       scope,
                                       *subobject.type,
                                       ctx.make_base_pointer_expr(this_expr,
                                                                  *subobject.type,
                                                                  subobject.offset),
                                       ctx.make_base_expr(source_expr,
                                                          *subobject.type,
                                                          subobject.offset),
                                       function_node,
                                       &info);
      }
    }
    for(size_t i = 0; i < info.bases.size(); ++i) {
      if(info.bases[i].is_virtual) {
        continue;
      }
      append_move_constructor_action(ctx,
                                     scope,
                                     *info.bases[i].type,
                                     ctx.make_base_pointer_expr(this_expr, *info.bases[i].type,
                                                                info.bases[i].offset),
                                     ctx.make_base_expr(source_expr, *info.bases[i].type,
                                                        info.bases[i].offset),
                                     function_node,
                                     &info);
    }
    if(info.class_kind == "union") {
      append_union_copy_action(ctx,
                               scope,
                               info,
                               this_expr,
                               source_expr,
                               function_node,
                               UnionStorageTransferKind::move_constructor);
      return;
    }
    append_all_vptr_actions(ctx, info, this_expr, entry_point_kind, function_node);
    const LeadingTrivialStoragePrefix prefix =
        leading_trivial_storage_prefix(ctx, info, false, true);
    if(!prefix.empty()) {
      append_trivial_storage_prefix_copy_action(this_expr,
                                                source_expr,
                                                prefix.byte_count,
                                                function_node);
    }
    std::set<BitFieldStorageKey> copied_bit_storage;
    for(size_t i = prefix.field_count; i < info.fields.size(); ++i) {
      if(info.fields[i].is_bit_field) {
        append_bit_field_copy_action(this_expr, source_expr, info.fields[i],
                                     function_node, copied_bit_storage);
      } else {
        ExprInfo field_expr = ctx.make_field_expr(this_expr, info.fields[i]);
        ExprInfo source_field = ctx.make_field_expr(source_expr, info.fields[i]);
        ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
        if(is_reference_type(info.fields[i].type)) {
          append_reference_binding_action(ctx, field_expr, source_field, function_node);
        } else if(field_class) {
          append_move_constructor_action(ctx,
                                         scope,
                                         *field_class,
                                         ctx.make_address_of_expr(field_expr),
                                         source_field,
                                         function_node);
        } else {
          function_node.children.push_back(
              make_assignment_statement(field_expr, make_xvalue_expr(source_field)));
        }
      }
    }
    return;
  }

  if(binding.is_aggregate_constructor && binding.synthesized) {
    std::set<BitFieldStorageKey> zeroed_bit_storage;
    const size_t aggregate_count =
        semantic_class_model::aggregate_element_count(info);
    const size_t explicit_arg_count =
        binding.params.empty() ? 0 : binding.params.size();
    for(size_t i = 0; i < info.fields.size(); ++i) {
      if(i + 1 < explicit_arg_count && i < aggregate_count) {
        ExprInfo source_expr =
            ctx.analyze_id_expression(scope, synthetic_identifier_node(
                synthetic_parameter_name(binding, i + 1)));
        if(info.fields[i].is_anonymous_storage) {
          ExprInfo storage_expr = ctx.make_field_expr(this_expr, info.fields[i]);
          const FieldInfo * input_field =
              semantic_class_model::aggregate_input_field(ctx, info.fields[i]);
          if(input_field && input_field != &info.fields[i]) {
            if(input_field->is_bit_field) {
              append_bit_field_store_action(ctx,
                                            scope,
                                            storage_expr,
                                            *input_field,
                                            source_expr,
                                            function_node,
                                            &zeroed_bit_storage);
            } else {
              ExprInfo active_expr = ctx.make_field_expr(storage_expr, *input_field);
              ClassInfo * field_class = ctx.class_info_for_type(input_field->type);
              if(field_class) {
                append_move_constructor_action(ctx,
                                               scope,
                                               *field_class,
                                               ctx.make_address_of_expr(active_expr),
                                               source_expr,
                                               function_node);
              } else {
                function_node.children.push_back(
                    make_assignment_statement(active_expr, std::move(source_expr)));
              }
            }
          }
        } else if(info.fields[i].is_bit_field) {
          append_bit_field_store_action(ctx, scope, this_expr, info.fields[i], source_expr,
                                        function_node, &zeroed_bit_storage);
        } else {
          ExprInfo field_expr = ctx.make_field_expr(this_expr, info.fields[i]);
          if(is_reference_type(info.fields[i].type)) {
            append_reference_binding_action(ctx, field_expr, source_expr, function_node);
            continue;
          }
          ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
          if(field_class) {
            append_move_constructor_action(ctx, scope, *field_class,
                                           ctx.make_address_of_expr(field_expr),
                                           source_expr, function_node);
          } else {
            function_node.children.push_back(
                make_assignment_statement(field_expr, std::move(source_expr)));
          }
        }
      } else {
        const FieldInfo * input_field =
            semantic_class_model::aggregate_input_field(ctx, info.fields[i]);
        const CppAstNode * initializer =
            input_field ? input_field->default_initializer : nullptr;
        append_field_initialization_actions(ctx,
                                            scope,
                                            info.fields[i],
                                            initializer,
                                            this_expr,
                                            function_node,
                                            zeroed_bit_storage);
      }
      if(info.class_kind == "union") {
        break;
      }
    }
    return;
  }

  if(binding.ctor_initializer) {
    for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
      const CppAstNode & init = binding.ctor_initializer->children[i];
      const CppAstNode * init_id = find_child_kind(init, CppAstKind::mem_initializer_id);
      if(!init_id) {
        continue;
      }
      if(init_id->value == info.name) {
        vector<const CppAstNode *> args;
        const CppAstNode * direct_braced_init = nullptr;
        if(init.children.size() >= 2) {
          args = initializer_argument_nodes(init.children[1]);
          const CppAstNode * payload = unwrap_initializer_payload(&init.children[1]);
          if(payload && payload->kind == CppAstKind::braced_init_list) {
            direct_braced_init = payload;
          }
        }
        append_constructor_call_action(ctx,
                                       scope,
                                       info,
                                       args,
                                       this_expr,
                                       direct_braced_init,
                                       ConstructorSelectionOptions(),
                                       &info,
                                       init.children.size() >= 2 &&
                                           initializer_is_empty_value_initializer(
                                               &init.children[1]),
                                       function_node);
        return;
      }
    }
  }

  if(info.class_kind == "union") {
    const FieldInfo * active_field = nullptr;
    const CppAstNode * active_initializer = nullptr;
    if(binding.ctor_initializer) {
      for(size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
        const CppAstNode & init = binding.ctor_initializer->children[i];
        const CppAstNode * init_id = find_child_kind(init, CppAstKind::mem_initializer_id);
        if(!init_id) {
          continue;
        }
        for(size_t j = 0; j < info.fields.size(); ++j) {
          if(info.fields[j].name != init_id->value) {
            continue;
          }
          active_field = &info.fields[j];
          active_initializer =
              init.children.size() >= 2 ? &init.children[1] : info.fields[j].default_initializer;
          break;
        }
        if(active_field) {
          break;
        }
      }
    }
    if(!active_field) {
      for(size_t i = 0; i < info.fields.size(); ++i) {
        if(info.fields[i].default_initializer) {
          active_field = &info.fields[i];
          active_initializer = info.fields[i].default_initializer;
          break;
        }
      }
    }
    if(!active_field) {
      return;
    }
    trace_ctor_initializer_state(
        binding, info, "union-active-field", active_field->name, active_initializer);
    std::set<BitFieldStorageKey> zeroed_bit_storage;
    append_field_initialization_actions(ctx,
                                        scope,
                                        *active_field,
                                        active_initializer,
                                        this_expr,
                                        function_node,
                                        zeroed_bit_storage);
    return;
  }

  if(construct_virtual_bases) {
    append_virtual_base_constructor_actions(ctx, scope, binding, info, this_expr, function_node);
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    const BaseInfo & base = info.bases[i];
    if(base.is_virtual) {
      continue;
    }
    append_ctor_subobject_constructor_action(ctx,
                                             scope,
                                             binding,
                                             info,
                                             *base.type,
                                             ctx.make_base_pointer_expr(this_expr,
                                                                        *base.type,
                                                                        base.offset),
                                             function_node);
  }

  append_all_vptr_actions(ctx, info, this_expr, entry_point_kind, function_node);

  std::set<BitFieldStorageKey> zeroed_bit_storage;
  for(size_t i = 0; i < info.fields.size(); ++i) {
    const FieldInfo & field = info.fields[i];
    if(field.is_anonymous_storage &&
       append_anonymous_storage_constructor_initialization_actions(
           ctx,
           scope,
           binding,
           field,
           this_expr,
           function_node,
           zeroed_bit_storage)) {
      continue;
    }
    const CppAstNode * mem_init = find_ctor_mem_initializer(binding, field.name);
    trace_ctor_initializer_state(binding, info, "field-init", field.name, mem_init);
    const CppAstNode * effective_initializer =
        mem_init && mem_init->children.size() >= 2 ? &mem_init->children[1]
                                                   : field.default_initializer;
    if(field.is_bit_field) {
      if(effective_initializer) {
        append_bit_field_initialization_actions(ctx, scope, field,
                                                effective_initializer, this_expr,
                                                function_node, zeroed_bit_storage);
      }
      continue;
    }
    ExprInfo field_expr = ctx.make_field_expr(this_expr, field);
    std::string member_initializer_use_location;
    if(mem_init &&
       !binding.source_template &&
       constructor_witness_source_capture_enabled(ctx)) {
      const CppAstNode * initializer_id =
          find_child_kind(*mem_init, CppAstKind::mem_initializer_id);
      member_initializer_use_location =
          ctx.source_location_for_name_in_node(initializer_id ? *initializer_id :
                                                               *mem_init,
                                               field.name,
                                               true);
      if(member_initializer_use_location.empty() && effective_initializer) {
        member_initializer_use_location =
            source_location_for_identifier_before_on_same_line(
                ctx,
                ctx.source_location_for_node(*effective_initializer),
                field.name);
      }
      if(parser_trace::enabled("lifetime.init")) {
        std::ostringstream trace;
        trace << "member-init-source-location field=" << field.name
              << " location=" << member_initializer_use_location
              << " before="
              << (effective_initializer ?
                      ctx.source_location_for_node(*effective_initializer) :
                      std::string())
              << " id-kind="
              << (initializer_id ? cppast_kind_text(initializer_id->kind) :
                                   std::string("<none>"));
        parser_trace::note("lifetime.init", std::string(), trace.str());
      }
    }
    append_target_initialization_actions(ctx, scope, field.type, effective_initializer,
                                         field_expr, function_node,
                                         member_initializer_use_location,
                                         !binding.source_template ? field.name :
                                             std::string());
  }
}

void append_copy_assignment_generated_statements(SemanticContext & ctx,
                                                 Scope & scope,
                                                 FunctionBinding & binding,
                                                 DumpNode & function_node)
{
  if(!binding.owner_class ||
     !binding_is_generated_copy_assignment(*binding.owner_class, binding)) {
    return;
  }

  if(binding.params.size() < 2) {
    throw logic_error("copy assignment missing source parameter");
  }

  ExprInfo this_expr = analyze_generated_this_expr(ctx, scope);
  ExprInfo source_expr =
      ctx.analyze_id_expression(scope, synthetic_identifier_node(
          synthetic_parameter_name(binding, 1)));
  ClassInfo & info = *binding.owner_class;

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual) {
      continue;
    }
    append_copy_assignment_action(ctx,
                                  scope,
                                  *info.bases[i].type,
                                  ctx.make_base_pointer_expr(this_expr, *info.bases[i].type,
                                                             info.bases[i].offset),
                                  ctx.make_base_expr(source_expr, *info.bases[i].type,
                                                     info.bases[i].offset),
                                  function_node);
  }

  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             this_expr,
                             source_expr,
                             function_node,
                             UnionStorageTransferKind::copy_assignment);
    DumpNode ret = make_dump_node(CallSemKind::return_statement);
    ret.children.push_back(std::move(this_expr.node));
    function_node.children.push_back(std::move(ret));
    return;
  }

  const LeadingTrivialStoragePrefix prefix =
      leading_trivial_storage_prefix(ctx, info, true, false);
  if(!prefix.empty()) {
    append_trivial_storage_prefix_copy_action(this_expr,
                                              source_expr,
                                              prefix.byte_count,
                                              function_node);
  }
  std::set<BitFieldStorageKey> copied_bit_storage;
  for(size_t i = prefix.field_count; i < info.fields.size(); ++i) {
    if(info.fields[i].is_bit_field) {
      append_bit_field_copy_action(this_expr, source_expr, info.fields[i],
                                   function_node, copied_bit_storage);
    } else {
      ExprInfo field_expr = ctx.make_field_expr(this_expr, info.fields[i]);
      ExprInfo source_field = ctx.make_field_expr(source_expr, info.fields[i]);
      ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
      if(field_class) {
        append_copy_assignment_action(ctx, scope, *field_class, ctx.make_address_of_expr(field_expr),
                                      source_field, function_node);
      } else {
        function_node.children.push_back(
            make_assignment_statement(field_expr, std::move(source_field)));
      }
    }
  }

  DumpNode ret = make_dump_node(CallSemKind::return_statement);
  ret.children.push_back(std::move(this_expr.node));
  function_node.children.push_back(std::move(ret));
}

void append_move_assignment_generated_statements(SemanticContext & ctx,
                                                 Scope & scope,
                                                 FunctionBinding & binding,
                                                 DumpNode & function_node)
{
  if(!binding.owner_class ||
     !binding_is_generated_move_assignment(*binding.owner_class, binding)) {
    return;
  }

  if(binding.params.size() < 2) {
    throw logic_error("move assignment missing source parameter");
  }

  ExprInfo this_expr = analyze_generated_this_expr(ctx, scope);
  ExprInfo source_expr =
      ctx.analyze_id_expression(scope, synthetic_identifier_node(
          synthetic_parameter_name(binding, 1)));
  ClassInfo & info = *binding.owner_class;

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual) {
      continue;
    }
    append_move_assignment_action(ctx,
                                  scope,
                                  *info.bases[i].type,
                                  ctx.make_base_pointer_expr(this_expr, *info.bases[i].type,
                                                             info.bases[i].offset),
                                  ctx.make_base_expr(source_expr, *info.bases[i].type,
                                                     info.bases[i].offset),
                                  function_node);
  }

  if(info.class_kind == "union") {
    append_union_copy_action(ctx,
                             scope,
                             info,
                             this_expr,
                             source_expr,
                             function_node,
                             UnionStorageTransferKind::move_assignment);
    DumpNode ret = make_dump_node(CallSemKind::return_statement);
    ret.children.push_back(std::move(this_expr.node));
    function_node.children.push_back(std::move(ret));
    return;
  }

  const LeadingTrivialStoragePrefix prefix =
      leading_trivial_storage_prefix(ctx, info, true, false);
  if(!prefix.empty()) {
    append_trivial_storage_prefix_copy_action(this_expr,
                                              source_expr,
                                              prefix.byte_count,
                                              function_node);
  }
  std::set<BitFieldStorageKey> copied_bit_storage;
  for(size_t i = prefix.field_count; i < info.fields.size(); ++i) {
    if(info.fields[i].is_bit_field) {
      append_bit_field_copy_action(this_expr, source_expr, info.fields[i],
                                   function_node, copied_bit_storage);
    } else {
      ExprInfo field_expr = ctx.make_field_expr(this_expr, info.fields[i]);
      ExprInfo source_field = ctx.make_field_expr(source_expr, info.fields[i]);
      ClassInfo * field_class = ctx.class_info_for_type(info.fields[i].type);
      if(field_class) {
        append_move_assignment_action(ctx,
                                      scope,
                                      *field_class,
                                      ctx.make_address_of_expr(field_expr),
                                      source_field,
                                      function_node);
      } else {
        function_node.children.push_back(
            make_assignment_statement(field_expr, std::move(source_field)));
      }
    }
  }

  DumpNode ret = make_dump_node(CallSemKind::return_statement);
  ret.children.push_back(std::move(this_expr.node));
  function_node.children.push_back(std::move(ret));
}

void append_destructor_generated_statements(SemanticContext & ctx,
                                            Scope & scope,
                                            FunctionBinding & binding,
                                            symbol_linkage::SpecialMemberEntryPointKind entry_point_kind,
                                            DumpNode & function_node)
{
  if(!binding.owner_class) {
    return;
  }

  DumpNode body_scope;
  const bool has_compound_body =
      function_node.kind == CallSemKind::compound_statement;
  if(has_compound_body) {
    body_scope = std::move(function_node);
    body_scope.is_destructor_body_scope = true;
    function_node = make_dump_node(CallSemKind::compound_statement);
  }

  ExprInfo this_expr = analyze_generated_this_expr(ctx, scope);
  ClassInfo & info = *binding.owner_class;
  if(info.class_kind == "union") {
    if(has_compound_body) {
      function_node.children.push_back(std::move(body_scope));
    }
    return;
  }
  DumpNode vptr_prefix = make_dump_node(CallSemKind::compound_statement);
  append_all_vptr_actions(ctx, info, this_expr, entry_point_kind, vptr_prefix);
  if(!vptr_prefix.children.empty()) {
    function_node.children.insert(function_node.children.begin(),
                                  vptr_prefix.children.begin(),
                                  vptr_prefix.children.end());
  }
  if(has_compound_body) {
    function_node.children.push_back(std::move(body_scope));
  }
  for(size_t i = info.fields.size(); i-- > 0;) {
    if(info.fields[i].is_bit_field ||
       !subobject_may_need_destructor_action(ctx, info.fields[i].type)) {
      continue;
    }
    append_destructor_actions_for_subobject(
        ctx,
        info.fields[i].type,
        ctx.make_field_expr(this_expr, info.fields[i]),
        &info,
        true,
        function_node);
  }
  for(size_t i = info.bases.size(); i-- > 0;) {
    if(info.bases[i].is_virtual) {
      continue;
    }
    append_destructor_action(ctx,
                             *info.bases[i].type,
                             ctx.make_base_pointer_expr(this_expr, *info.bases[i].type,
                                                        info.bases[i].offset),
                             &info,
                             true,
                             function_node);
  }
  if(entry_point_kind == symbol_linkage::SMEK_COMPLETE) {
    for(size_t i = info.virtual_base_subobjects.size(); i-- > 0;) {
      const SubobjectInfo & subobject = info.virtual_base_subobjects[i];
      if(!subobject.type) {
        continue;
      }
      append_destructor_action(ctx,
                               *subobject.type,
                               ctx.make_base_pointer_expr(this_expr,
                                                          *subobject.type,
                                                          subobject.offset),
                               &info,
                               true,
                               function_node);
    }
  }
  if(entry_point_kind == symbol_linkage::SMEK_DELETING) {
    CppAstNode call;
    call.kind = CppAstKind::call_expression;

    CppAstNode callee;
    callee.kind = CppAstKind::id_expression;
    callee.value = "operator delete";
    call.children.push_back(callee);

    CppAstNode arguments;
    arguments.kind = CppAstKind::paren_argument_list;
    arguments.children.push_back(synthetic_identifier_node("this"));
    call.children.push_back(arguments);

    DumpNode stmt = make_dump_node(CallSemKind::expression_statement);
    ExprInfo delete_call = ctx.analyze_call_expression(scope, call);
    stmt.children.push_back(std::move(delete_call.node));
    function_node.children.push_back(std::move(stmt));
  }
}

}  // namespace semantic_lifetime
