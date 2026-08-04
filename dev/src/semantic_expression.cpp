#include "semantic_expression.h"

#include <cctype>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "callsem_output.h"
#include "constant_value.h"
#include "cpp_decl_ast.h"
#include "constructor_lifecycle_service.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "encoding.h"
#include "pack_parameter_analysis.h"
#include "parser_trace.h"
#include "pptokenizer.h"
#include "rtti_names.h"
#include "semantic_builtins.h"
#include "semantic_class_model.h"
#include "semantic_conversion.h"
#include "semantic_dependent_type.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_hotspot.h"
#include "semantic_lifetime.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_overload.h"
#include "semantic_output.h"
#include "semantic_parameter_recovery.h"
#include "semantic_scope_mutation.h"
#include "semantic_statement.h"
#include "semantic_trace.h"
#include "semantic_template_variable.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"
#include "template_argument_semantics.h"
#include "template_api.h"
#include "template_services.h"
#include "types.h"
#include "witness_api.h"

using namespace std;

namespace semantic_expression {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using namespace semantic_lookup;
using DumpNode = CallSemNode;

namespace {

thread_local std::size_t unevaluated_operand_depth = 0;

}  // namespace

ScopedUnevaluatedOperand::ScopedUnevaluatedOperand()
{
  ++unevaluated_operand_depth;
}

ScopedUnevaluatedOperand::~ScopedUnevaluatedOperand()
{
  --unevaluated_operand_depth;
}

bool unevaluated_operand_active()
{
  return unevaluated_operand_depth != 0;
}

namespace {

size_t string_literal_code_unit_count(const QuoteLiteralData & literal)
{
  return quote_literal_string_unit_count(literal);
}

string literal_without_ud_suffix(const string & text, const string & ud_suffix)
{
  if(ud_suffix.empty()) {
    return text;
  }
  if(text.size() < ud_suffix.size() ||
     text.compare(text.size() - ud_suffix.size(), ud_suffix.size(), ud_suffix) != 0) {
    throw logic_error("literal suffix does not match source");
  }
  return text.substr(0, text.size() - ud_suffix.size());
}

CppAstNode make_synthetic_literal_node(const CppAstNode & source,
                                       const string & value)
{
  CppAstNode out;
  out.kind = CppAstKind::literal;
  out.value = value;
  out.source_location_id = source.source_location_id;
  out.token_start = source.token_start;
  out.token_end = source.token_end;
  mutable_cppast_name_lookup_snapshot(out) =
      cppast_name_lookup_snapshot(source);
  return out;
}

string char_literal_source(char c)
{
  switch(c) {
  case '\n':
    return "'\\n'";
  case '\r':
    return "'\\r'";
  case '\t':
    return "'\\t'";
  case '\v':
    return "'\\v'";
  case '\f':
    return "'\\f'";
  case '\b':
    return "'\\b'";
  case '\a':
    return "'\\a'";
  case '\\':
    return "'\\\\'";
  case '\'':
    return "'\\''";
  default:
    break;
  }
  return string("'") + c + "'";
}

TemplateArgumentSyntax make_char_template_argument_syntax(
    const CppAstNode & source,
    const string & text)
{
  TemplateArgumentSyntax out;
  out.text = text;
  out.source_text = text;
  out.source_location_id = source.source_location_id;
  out.has_source_token_start = true;
  out.source_token_start = source.token_start;
  out.expression.reset(new CppAstNode(make_synthetic_literal_node(source, text)));
  return out;
}

bool has_cooked_numeric_literal_operator(SemanticContext & ctx,
                                         Scope & scope,
                                         const string & operator_name,
                                         EFundamentalType parameter_type)
{
  const vector<FunctionBinding *> functions =
      ctx.lookup_functions(scope, operator_name);
  for(size_t i = 0; i < functions.size(); ++i) {
    const FunctionBinding * function = functions[i];
    if(!function || function->is_method || function->params.size() != 1) {
      continue;
    }
    const TypePtr type = strip_top_level_cv(function->params[0].second);
    if(type &&
       type->kind == Type::TK_FUNDAMENTAL &&
       type->fundamental == parameter_type) {
      return true;
    }
  }
  return false;
}

ExprInfo analyze_string_user_defined_literal(SemanticContext & ctx,
                                             Scope & scope,
                                             const CppAstNode & node,
                                             const QuoteLiteralData & literal)
{
  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.source_location_id = node.source_location_id;
  call.token_start = node.token_start;
  call.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(call) =
      cppast_name_lookup_snapshot(node);

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = string("operator\"\"") + literal.ud_suffix;
  callee.source_location_id = node.source_location_id;
  callee.token_start = node.token_start;
  callee.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(callee) =
      cppast_name_lookup_snapshot(node);
  call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.source_location_id = node.source_location_id;
  arguments.token_start = node.token_start;
  arguments.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(arguments) =
      cppast_name_lookup_snapshot(node);
  arguments.children.push_back(
      make_synthetic_literal_node(
          node,
          literal_without_ud_suffix(node.value, literal.ud_suffix)));
  arguments.children.push_back(
      make_synthetic_literal_node(
          node,
          to_string(string_literal_code_unit_count(literal))));
  call.children.push_back(arguments);

  return ctx.analyze_call_expression(scope, call);
}

ExprInfo analyze_cooked_numeric_user_defined_literal(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const CppAstNode & node,
                                                     const string & ud_suffix,
                                                     const string & literal_text)
{
  const string operator_name = string("operator\"\"") + ud_suffix;

  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.source_location_id = node.source_location_id;
  call.token_start = node.token_start;
  call.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(call) =
      cppast_name_lookup_snapshot(node);

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = operator_name;
  callee.source_location_id = node.source_location_id;
  callee.token_start = node.token_start;
  callee.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(callee) =
      cppast_name_lookup_snapshot(node);
  call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.source_location_id = node.source_location_id;
  arguments.token_start = node.token_start;
  arguments.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(arguments) =
      cppast_name_lookup_snapshot(node);
  arguments.children.push_back(make_synthetic_literal_node(node, literal_text));
  call.children.push_back(arguments);

  return ctx.analyze_call_expression(scope, call);
}

ExprInfo analyze_numeric_user_defined_literal_template(SemanticContext & ctx,
                                                       Scope & scope,
                                                       const CppAstNode & node,
                                                       const string & ud_suffix)
{
  const string operator_name = string("operator\"\"") + ud_suffix;
  const string literal_text = literal_without_ud_suffix(node.value, ud_suffix);

  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.source_location_id = node.source_location_id;
  call.token_start = node.token_start;
  call.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(call) =
      cppast_name_lookup_snapshot(node);

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = operator_name;
  callee.source_location_id = node.source_location_id;
  callee.token_start = node.token_start;
  callee.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(callee) =
      cppast_name_lookup_snapshot(node);

  TemplateIdSyntax template_id;
  template_id.name.name = operator_name;
  template_id.source_location_id = node.source_location_id;
  template_id.arguments.reserve(literal_text.size());
  template_id.argument_syntaxes.reserve(literal_text.size());
  for(size_t i = 0; i < literal_text.size(); ++i) {
    const string arg = char_literal_source(literal_text[i]);
    template_id.arguments.push_back(arg);
    template_id.argument_syntaxes.push_back(
        make_char_template_argument_syntax(node, arg));
  }
  set_cppast_template_id_syntax(callee, std::move(template_id));
  call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.source_location_id = node.source_location_id;
  arguments.token_start = node.token_start;
  arguments.token_end = node.token_end;
  mutable_cppast_name_lookup_snapshot(arguments) =
      cppast_name_lookup_snapshot(node);
  call.children.push_back(arguments);

  return ctx.analyze_call_expression(scope, call);
}

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const string & location)
      : active(!location.empty())
  {
    if(active) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateUseLocation()
  {
    if(active) {
      parser_trace::pop_use_location();
    }
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;

  bool active;
};

bool expression_decl_spec_contains_token(const CppAstNode * node,
                                         ETokenType token)
{
  if(!node) {
    return false;
  }
  const CppAstNode * specifiers =
      node->kind == CppAstKind::decl_specifier_seq ||
              node->kind == CppAstKind::member_specifiers ?
          node :
          find_child(*node, CppAstKind::decl_specifier_seq);
  if(!specifiers) {
    specifiers = find_child(*node, CppAstKind::member_specifiers);
  }
  return specifiers && decl_spec_contains_token(*specifiers, token);
}

TypePtr subscript_pointer_operand_type(const ExprInfo & expr)
{
  TypePtr converted = value_conversion_type(expr);
  if(converted && converted->kind == Type::TK_POINTER) {
    return converted;
  }

  TypePtr base = strip_top_level_cv(remove_reference_type(expr.type));
  if(base && base->kind == Type::TK_ARRAY) {
    return make_pointer(base->inner);
  }
  return TypePtr();
}

bool binding_supports_implicit_return_move(const ValueBinding & binding)
{
  if(binding.kind != ValueBinding::VK_VARIABLE &&
     binding.kind != ValueBinding::VK_PARAMETER) {
    return false;
  }
  if(!binding.symbol.internal_symbol.empty() ||
     !binding.symbol.object_symbol.empty()) {
    return false;
  }

  TypePtr binding_type = strip_top_level_cv(binding.type);
  if(!binding_type) {
    return false;
  }
  if(binding_type->kind == Type::TK_LVALUE_REFERENCE) {
    return false;
  }
  if(binding_type->kind == Type::TK_RVALUE_REFERENCE) {
    return binding_type->inner && !type_is_const_object(binding_type->inner);
  }
  return !type_is_const_object(binding.type);
}

symbol_linkage::SymbolLinkage expression_variable_symbol_linkage(const ValueBinding & binding)
{
  if(binding.is_c_linkage) {
    return symbol_linkage::SL_EXTERNAL;
  }
  if(binding.owner_class &&
     (!binding.has_storage_definition ||
      template_api::value_binding_output_suppressed_by_explicit_instantiation(
          binding))) {
    return symbol_linkage::SL_EXTERNAL;
  }
  if(template_api::value_or_owner_has_template_identity(&binding)) {
    return symbol_linkage::SL_WEAK;
  }
  const CppAstNode * declaration =
      binding.definition_node ? binding.definition_node : binding.declaration_node;
  if(binding.owner_class &&
     (expression_decl_spec_contains_token(declaration, KW_INLINE) ||
      expression_decl_spec_contains_token(declaration, KW_CONSTEXPR))) {
    return symbol_linkage::SL_WEAK;
  }
  if(!binding.owner_class &&
     expression_decl_spec_contains_token(declaration, KW_INLINE)) {
    return symbol_linkage::SL_WEAK;
  }
  return binding.symbol.linkage;
}

symbol_linkage::SymbolIdentity expression_static_member_variable_symbol_identity(
    const ClassInfo & owner,
    const ValueBinding & binding)
{
  if(binding.variable_template_instantiation &&
     binding.variable_template_instantiation->source_template) {
    const VariableTemplateDecl & source_template =
        *binding.variable_template_instantiation->source_template;
    return symbol_linkage::make_static_member_variable_template_symbol_identity(
        owner,
        binding.name,
        source_template.name,
        binding.variable_template_instantiation->arguments,
        source_template.parameters,
        binding.is_c_linkage,
        expression_variable_symbol_linkage(binding));
  }
  return symbol_linkage::make_static_member_variable_symbol_identity(
      owner,
      binding.name,
      binding.is_c_linkage,
      expression_variable_symbol_linkage(binding));
}

FunctionBinding * find_delete_destructor_binding(ClassInfo & info)
{
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      info.methods.find("~" + info.name);
  if(found == info.methods.end()) {
    return nullptr;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * binding = found->second[i];
    if(binding && binding->is_destructor) {
      return binding;
    }
  }
  return nullptr;
}

bool is_nullable_pointer_like_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return is_pointer_type(base) ||
         (base && base->kind == Type::TK_MEMBER_POINTER);
}

bool is_nullptr_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base &&
         base->kind == Type::TK_FUNDAMENTAL &&
         base->fundamental == FT_NULLPTR_T;
}

struct FoldedIntegralLiteral
{
  string text;
  bool has_int_value = false;
  long long int_value = 0;
  bool has_uint_value = false;
  unsigned long long uint_value = 0;
};

bool is_int128_integral_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base &&
         base->kind == Type::TK_FUNDAMENTAL &&
         (base->fundamental == FT_INT128 || base->fundamental == FT_UINT128);
}

bool constexpr_value_to_literal_value(SemanticContext & ctx,
                                      const constant_eval::ConstexprValue & value,
                                      const TypePtr & type,
                                      FoldedIntegralLiteral & out)
{
  if(is_int128_integral_type(type) || is_int128_integral_type(value.type)) {
    return false;
  }

  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  TypePtr value_base = strip_top_level_cv(remove_reference_type(value.type));
  if(is_floating_type(base) || is_floating_type(value_base)) {
    constant_eval::ConstexprValue converted = value;
    if(converted.kind != constant_eval::ConstexprValue::CV_FLOATING &&
       !constant_eval::constexpr_value_cast(value, type, converted)) {
      return false;
    }
    if(converted.kind != constant_eval::ConstexprValue::CV_FLOATING) {
      return false;
    }

    EFundamentalType fundamental =
        base && base->kind == Type::TK_FUNDAMENTAL ? base->fundamental : FT_DOUBLE;
    long double emitted_value = converted.floating_value;
    if(fundamental == FT_FLOAT) {
      emitted_value = static_cast<float>(converted.floating_value);
    } else if(fundamental == FT_DOUBLE) {
      emitted_value = static_cast<double>(converted.floating_value);
    }

    ostringstream text;
    text << setprecision(20) << emitted_value;
    out.text = text.str();
    if(out.text.find('.') == string::npos &&
       out.text.find('e') == string::npos &&
       out.text.find('E') == string::npos) {
      out.text += ".0";
    }
    if(fundamental == FT_FLOAT) {
      out.text += "f";
    } else if(fundamental == FT_LONG_DOUBLE) {
      out.text += "L";
    }
    return true;
  }

  long long signed_value = 0;
  if(constant_eval::constexpr_value_to_integral(value, signed_value)) {
    out.text = to_string(signed_value);
    out.has_int_value = true;
    out.int_value = signed_value;
    return true;
  }

  if((is_unsigned_integral_type(base) ||
      is_named_enum_type(ctx, base) ||
      is_unsigned_integral_type(value_base) ||
      is_named_enum_type(ctx, value_base)) &&
     constant_eval::constexpr_value_to_unsigned_integral(value, out.uint_value)) {
    out.text = to_string(out.uint_value);
    out.has_uint_value = true;
    return true;
  }
  return false;
}

bool deduce_implicit_return_type_from_exprs(const vector<ExprInfo> & return_exprs,
                                            bool saw_void_return,
                                            TypePtr & out);

bool lambda_body_contains_disallowed_cache_control_flow(const CppAstNode & node);
bool lambda_body_contains_local_class_declaration(const CppAstNode & node);

ClassInfo * complete_class_type_for_lookup(SemanticContext & ctx,
                                           const TypePtr & type);

void set_expr_metadata(CallSemNode & node,
                       const TypePtr & type,
                       ValueCategory category);

bool pointer_class_hierarchy_equality_compatible(SemanticContext & ctx,
                                                 const TypePtr & lhs,
                                                 const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base ||
     lhs_base->kind != Type::TK_POINTER ||
     rhs_base->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr lhs_pointee = strip_top_level_cv(lhs_base->inner);
  TypePtr rhs_pointee = strip_top_level_cv(rhs_base->inner);
  if(!lhs_pointee || !rhs_pointee) {
    return false;
  }

  ClassInfo * lhs_class = complete_class_type_for_lookup(ctx, lhs_pointee);
  if(!lhs_class) {
    lhs_class = ctx.class_info_for_type(lhs_pointee);
  }
  ClassInfo * rhs_class = complete_class_type_for_lookup(ctx, rhs_pointee);
  if(!rhs_class) {
    rhs_class = ctx.class_info_for_type(rhs_pointee);
  }
  if(!lhs_class || !rhs_class || lhs_class == rhs_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(find_unique_base_path(*lhs_class, rhs_class, offset, access) &&
     access == MA_PUBLIC) {
    return true;
  }

  offset = 0;
  access = MA_PUBLIC;
  return find_unique_base_path(*rhs_class, lhs_class, offset, access) &&
         access == MA_PUBLIC;
}

bool try_apply_pointer_class_hierarchy_comparison_conversion(
    SemanticContext & ctx,
    ExprInfo & lhs,
    ExprInfo & rhs)
{
  TypePtr lhs_pointer = strip_top_level_cv(value_conversion_type(lhs));
  TypePtr rhs_pointer = strip_top_level_cv(value_conversion_type(rhs));
  if(!lhs_pointer || !rhs_pointer ||
     lhs_pointer->kind != Type::TK_POINTER ||
     rhs_pointer->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr lhs_pointee;
  TypePtr rhs_pointee;
  bool lhs_const = false;
  bool lhs_volatile = false;
  bool rhs_const = false;
  bool rhs_volatile = false;
  if(!top_level_cv_flags(lhs_pointer->inner,
                         lhs_pointee,
                         lhs_const,
                         lhs_volatile) ||
     !top_level_cv_flags(rhs_pointer->inner,
                         rhs_pointee,
                         rhs_const,
                         rhs_volatile)) {
    return false;
  }

  ClassInfo * lhs_class = complete_class_type_for_lookup(ctx, lhs_pointee);
  if(!lhs_class) {
    lhs_class = ctx.class_info_for_type(lhs_pointee);
  }
  ClassInfo * rhs_class = complete_class_type_for_lookup(ctx, rhs_pointee);
  if(!rhs_class) {
    rhs_class = ctx.class_info_for_type(rhs_pointee);
  }
  if(!lhs_class || !rhs_class || lhs_class == rhs_class) {
    return false;
  }

  const bool combined_const = lhs_const || rhs_const;
  const bool combined_volatile = lhs_volatile || rhs_volatile;
  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(find_unique_base_path(*lhs_class, rhs_class, offset, access) &&
     access == MA_PUBLIC) {
    const TypePtr target =
        make_pointer(make_cv(rhs_pointee, combined_const, combined_volatile));
    ExprInfo converted;
    if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                             target,
                                                             lhs,
                                                             converted)) {
      lhs = converted;
      return true;
    }
  }

  offset = 0;
  access = MA_PUBLIC;
  if(find_unique_base_path(*rhs_class, lhs_class, offset, access) &&
     access == MA_PUBLIC) {
    const TypePtr target =
        make_pointer(make_cv(lhs_pointee, combined_const, combined_volatile));
    ExprInfo converted;
    if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                             target,
                                                             rhs,
                                                             converted)) {
      rhs = converted;
      return true;
    }
  }
  return false;
}

bool const_cast_similar_object_types(const TypePtr & lhs, const TypePtr & rhs)
{
  TypePtr lhs_base = strip_top_level_cv(lhs);
  TypePtr rhs_base = strip_top_level_cv(rhs);
  if(!lhs_base || !rhs_base || lhs_base->kind != rhs_base->kind) {
    return false;
  }

  switch(lhs_base->kind) {
  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return type_equals(lhs_base, rhs_base);

  case Type::TK_CV:
  case Type::TK_ATOMIC:
    return const_cast_similar_object_types(lhs_base->inner, rhs_base->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return const_cast_similar_object_types(lhs_base->inner, rhs_base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_equals(strip_top_level_cv(lhs_base->owner),
                       strip_top_level_cv(rhs_base->owner)) &&
           const_cast_similar_object_types(lhs_base->inner, rhs_base->inner);

  case Type::TK_ARRAY:
    return lhs_base->has_bound == rhs_base->has_bound &&
           lhs_base->bound == rhs_base->bound &&
           lhs_base->bound_text == rhs_base->bound_text &&
           const_cast_similar_object_types(lhs_base->inner, rhs_base->inner);

  case Type::TK_FUNCTION:
    return type_equals(lhs_base, rhs_base);
  }

  return false;
}

bool class_path_crosses_virtual_base(const ClassInfo & derived,
                                     const ClassInfo * base)
{
  vector<pair<const ClassInfo *, bool> > stack;
  stack.push_back(make_pair(&derived, false));
  while(!stack.empty()) {
    const ClassInfo * current = stack.back().first;
    const bool crossed_virtual_base = stack.back().second;
    stack.pop_back();
    if(current == base && crossed_virtual_base) {
      return true;
    }
    for(size_t i = 0; i < current->bases.size(); ++i) {
      stack.push_back(
          make_pair(current->bases[i].type,
                    crossed_virtual_base || current->bases[i].is_virtual));
    }
  }
  return false;
}

bool supports_zero_offset_static_reference_downcast(SemanticContext & ctx,
                                                    const TypePtr & target_type,
                                                    const ExprInfo & operand)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }

  TypePtr target_object_type = strip_top_level_cv(target_base->inner);
  TypePtr source_object_type = strip_top_level_cv(remove_reference_type(operand.type));
  if(!target_object_type || !source_object_type) {
    return false;
  }

  ClassInfo * target_class = complete_class_type_for_lookup(ctx, target_object_type);
  if(!target_class) {
    target_class = ctx.class_info_for_type(target_object_type);
  }
  ClassInfo * source_class = complete_class_type_for_lookup(ctx, source_object_type);
  if(!source_class) {
    source_class = ctx.class_info_for_type(source_object_type);
  }
  if(!target_class || !source_class || target_class == source_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  return find_unique_base_path(*target_class, source_class, offset, access) &&
         !class_path_crosses_virtual_base(*target_class, source_class) &&
         offset == 0;
}

bool pointer_downcast_crosses_virtual_base(SemanticContext & ctx,
                                           const TypePtr & target_type,
                                           const ExprInfo & operand)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  TypePtr source_base = value_conversion_type(operand);
  if(!target_base || !source_base ||
     target_base->kind != Type::TK_POINTER ||
     source_base->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr target_pointee = strip_top_level_cv(target_base->inner);
  TypePtr source_pointee = strip_top_level_cv(source_base->inner);
  if(type_equals(target_pointee, source_pointee)) {
    return false;
  }

  ClassInfo * target_class =
      complete_class_type_for_lookup(ctx, target_pointee);
  ClassInfo * source_class =
      complete_class_type_for_lookup(ctx, source_pointee);
  if(!target_class || !source_class || target_class == source_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!find_unique_base_path(*target_class, source_class, offset, access)) {
    return false;
  }

  return class_path_crosses_virtual_base(*target_class, source_class);
}

bool top_level_cv_compatible_for_direct_reference_cast(const TypePtr & target,
                                                       const TypePtr & source)
{
  TypePtr target_base;
  TypePtr source_base;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;
  return top_level_cv_flags(target, target_base, target_const, target_volatile) &&
         top_level_cv_flags(source, source_base, source_const, source_volatile) &&
         !(source_const && !target_const) &&
         !(source_volatile && !target_volatile);
}

bool direct_static_reference_cast_preserves_object(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const TypePtr & target_type,
                                                   const ExprInfo & operand)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }
  if(target_base->kind == Type::TK_LVALUE_REFERENCE &&
     operand.category != VC_LVALUE) {
    return false;
  }

  TypePtr target_object_type = target_base->inner;
  TypePtr operand_object_type = remove_reference_type(operand.type);
  if(!target_object_type || !operand_object_type) {
    return false;
  }

  TypePtr normalized_target_object_type = target_object_type;
  TypePtr normalized_operand_object_type = operand_object_type;
  TypePtr resolved_object_type;
  if(semantic_dependent_type::resolve_instantiated_dependent_type(
         ctx, scope, target_object_type, resolved_object_type) &&
     resolved_object_type) {
    normalized_target_object_type = resolved_object_type;
  }
  if(semantic_dependent_type::resolve_instantiated_dependent_type(
         ctx, scope, operand_object_type, resolved_object_type) &&
     resolved_object_type) {
    normalized_operand_object_type = resolved_object_type;
  }

  if(same_type_with_compatible_top_cv(target_object_type, operand_object_type) ||
     same_type_with_compatible_top_cv(normalized_target_object_type,
                                      normalized_operand_object_type)) {
    return true;
  }

  TypePtr target_unqualified_object_type =
      strip_top_level_cv(normalized_target_object_type);
  TypePtr operand_unqualified_object_type =
      strip_top_level_cv(normalized_operand_object_type);
  ClassInfo * target_object_class =
      target_unqualified_object_type
          ? ctx.class_info_for_type(target_unqualified_object_type)
          : nullptr;
  ClassInfo * operand_object_class =
      operand_unqualified_object_type
          ? ctx.class_info_for_type(operand_unqualified_object_type)
          : nullptr;
  return target_object_class &&
         target_object_class == operand_object_class &&
         top_level_cv_compatible_for_direct_reference_cast(
             normalized_target_object_type, normalized_operand_object_type);
}

void enforce_static_inheritance_cast_access(SemanticContext & ctx,
                                            Scope & scope,
                                            const TypePtr & target_type,
                                            const ExprInfo & operand)
{
  TypePtr target_form = strip_top_level_cv(target_type);
  TypePtr source_form = value_conversion_type(operand);
  if(!target_form || !source_form) {
    return;
  }

  TypePtr target_object_type;
  TypePtr source_object_type;
  if(target_form->kind == Type::TK_POINTER &&
     source_form->kind == Type::TK_POINTER) {
    target_object_type = strip_top_level_cv(target_form->inner);
    source_object_type = strip_top_level_cv(source_form->inner);
  } else if(target_form->kind == Type::TK_LVALUE_REFERENCE ||
            target_form->kind == Type::TK_RVALUE_REFERENCE) {
    target_object_type = strip_top_level_cv(target_form->inner);
    source_object_type =
        strip_top_level_cv(remove_reference_type(operand.type));
  } else {
    return;
  }

  ClassInfo * target_class =
      target_object_type ? complete_class_type_for_lookup(ctx, target_object_type) : nullptr;
  if(!target_class && target_object_type) {
    target_class = ctx.class_info_for_type(target_object_type);
  }
  ClassInfo * source_class =
      source_object_type ? complete_class_type_for_lookup(ctx, source_object_type) : nullptr;
  if(!source_class && source_object_type) {
    source_class = ctx.class_info_for_type(source_object_type);
  }
  if(!target_class || !source_class || target_class == source_class) {
    return;
  }

  size_t offset = 0;
  MemberAccess path_access = MA_PUBLIC;
  ClassInfo * derived_class = nullptr;
  if(find_unique_base_path(*source_class, target_class, offset, path_access)) {
    derived_class = source_class;
  } else if(find_unique_base_path(*target_class,
                                  source_class,
                                  offset,
                                  path_access)) {
    derived_class = target_class;
  } else {
    return;
  }

  if(!member_access_allowed(&scope,
                            current_class_scope(scope),
                            current_function_scope(scope),
                            derived_class,
                            MA_PUBLIC,
                            path_access)) {
    throw logic_error("inaccessible base class conversion");
  }
}

bool try_apply_static_reference_base_cast(SemanticContext & ctx,
                                          const TypePtr & target_type,
                                          const ExprInfo & operand,
                                          ExprInfo & out)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }
  if(target_base->kind == Type::TK_LVALUE_REFERENCE && operand.category != VC_LVALUE) {
    return false;
  }

  TypePtr target_object_type = strip_top_level_cv(target_base->inner);
  TypePtr source_object_type = strip_top_level_cv(remove_reference_type(operand.type));
  if(!target_object_type || !source_object_type) {
    return false;
  }

  ClassInfo * target_class = complete_class_type_for_lookup(ctx, target_object_type);
  if(!target_class) {
    target_class = ctx.class_info_for_type(target_object_type);
  }
  ClassInfo * source_class = complete_class_type_for_lookup(ctx, source_object_type);
  if(!source_class) {
    source_class = ctx.class_info_for_type(source_object_type);
  }
  if(!target_class || !source_class || target_class == source_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!find_unique_base_path(*source_class, target_class, offset, access)) {
    return false;
  }

  out = ctx.apply_base_subobject_adjustment(operand, target_type, *target_class, offset);
  return true;
}

bool try_apply_static_reference_derived_cast(SemanticContext & ctx,
                                             const TypePtr & target_type,
                                             const ExprInfo & operand,
                                             ExprInfo & out)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base ||
     (target_base->kind != Type::TK_LVALUE_REFERENCE &&
      target_base->kind != Type::TK_RVALUE_REFERENCE)) {
    return false;
  }
  if(target_base->kind == Type::TK_LVALUE_REFERENCE && operand.category != VC_LVALUE) {
    return false;
  }
  if(target_base->kind == Type::TK_RVALUE_REFERENCE && operand.category == VC_PRVALUE) {
    return false;
  }

  TypePtr target_object_type = strip_top_level_cv(target_base->inner);
  TypePtr source_object_type = strip_top_level_cv(remove_reference_type(operand.type));
  if(!target_object_type || !source_object_type) {
    return false;
  }

  ClassInfo * target_class = complete_class_type_for_lookup(ctx, target_object_type);
  if(!target_class) {
    target_class = ctx.class_info_for_type(target_object_type);
  }
  ClassInfo * source_class = complete_class_type_for_lookup(ctx, source_object_type);
  if(!source_class) {
    source_class = ctx.class_info_for_type(source_object_type);
  }
  if(!target_class || !source_class || target_class == source_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!find_unique_base_path(*target_class, source_class, offset, access) ||
     class_path_crosses_virtual_base(*target_class, source_class)) {
    return false;
  }

  out = operand;
  out.type = target_base->inner ? target_base->inner : target_class->type;
  out.category = target_base->kind == Type::TK_RVALUE_REFERENCE ? VC_XVALUE : VC_LVALUE;
  if(offset != 0) {
    out.node = make_dump_node(CallSemKind::member_expression, target_class->name);
    set_callsem_resolved_name(out.node, target_class->qualified_name);
    set_callsem_int_value(out.node, -static_cast<long long>(offset));
    out.node.is_base_subobject = true;
    out.node.children.push_back(operand.node);
  }
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

bool try_apply_static_pointer_derived_cast(SemanticContext & ctx,
                                           const TypePtr & target_type,
                                           const ExprInfo & operand,
                                           ExprInfo & out)
{
  TypePtr target_base = strip_top_level_cv(target_type);
  TypePtr source_base = value_conversion_type(operand);
  if(!target_base || !source_base ||
     target_base->kind != Type::TK_POINTER ||
     source_base->kind != Type::TK_POINTER) {
    return false;
  }

  TypePtr target_object_type;
  TypePtr source_object_type;
  bool target_const = false;
  bool target_volatile = false;
  bool source_const = false;
  bool source_volatile = false;
  if(!top_level_cv_flags(target_base->inner,
                         target_object_type,
                         target_const,
                         target_volatile) ||
     !top_level_cv_flags(source_base->inner,
                         source_object_type,
                         source_const,
                         source_volatile) ||
     (source_const && !target_const) ||
     (source_volatile && !target_volatile)) {
    return false;
  }

  target_object_type = strip_top_level_cv(target_object_type);
  source_object_type = strip_top_level_cv(source_object_type);
  if(type_equals(target_object_type, source_object_type)) {
    return false;
  }
  ClassInfo * target_class =
      target_object_type ? complete_class_type_for_lookup(ctx, target_object_type) : nullptr;
  if(!target_class && target_object_type) {
    target_class = ctx.class_info_for_type(target_object_type);
  }
  ClassInfo * source_class =
      source_object_type ? complete_class_type_for_lookup(ctx, source_object_type) : nullptr;
  if(!source_class && source_object_type) {
    source_class = ctx.class_info_for_type(source_object_type);
  }
  if(!target_class || !source_class || target_class == source_class) {
    return false;
  }

  size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  if(!find_unique_base_path(*target_class, source_class, offset, access) ||
     class_path_crosses_virtual_base(*target_class, source_class)) {
    return false;
  }

  if(offset == 0) {
    out = operand;
    out.type = target_type;
    out.category = VC_PRVALUE;
    set_expr_metadata(out.node, out.type, out.category);
    return true;
  }

  ExprInfo adjusted_object;
  adjusted_object.type = target_base->inner ? target_base->inner : target_class->type;
  adjusted_object.category = VC_LVALUE;
  adjusted_object.node = make_dump_node(CallSemKind::member_expression, target_class->name);
  set_callsem_resolved_name(adjusted_object.node, target_class->qualified_name);
  set_callsem_int_value(adjusted_object.node, -static_cast<long long>(offset));
  adjusted_object.node.is_base_subobject = true;
  adjusted_object.node.children.push_back(operand.node);
  set_expr_metadata(adjusted_object.node,
                    adjusted_object.type,
                    adjusted_object.category);

  out = ctx.make_address_of_expr(adjusted_object);
  out.type = target_type;
  out.category = VC_PRVALUE;
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

bool deduce_implicit_return_type_from_exprs(const vector<ExprInfo> & return_exprs,
                                            bool saw_void_return,
                                            TypePtr & out)
{
  if(return_exprs.empty()) {
    out = make_fundamental(FT_VOID);
    return true;
  }
  if(saw_void_return) {
    return false;
  }

  TypePtr deduced;
  for(size_t i = 0; i < return_exprs.size(); ++i) {
    TypePtr candidate = value_conversion_type(return_exprs[i]);
    if(!candidate) {
      return false;
    }
    if(!deduced) {
      deduced = candidate;
      continue;
    }
    if(!type_equals(deduced, candidate)) {
      return false;
    }
  }

  out = deduced;
  return static_cast<bool>(out);
}

CallValueCategory to_call_value_category(ValueCategory category)
{
  switch(category) {
  case VC_LVALUE: return CVC_LVALUE;
  case VC_PRVALUE: return CVC_PRVALUE;
  case VC_XVALUE: return CVC_XVALUE;
  }

  throw logic_error("unknown value category");
}

bool is_gnu_complex_unary_operator(const std::string & text)
{
  return text == "__real" || text == "__real__" ||
         text == "__imag" || text == "__imag__";
}

void hard_fail_semantic_fallback(SemanticContext & ctx,
                                 const CppAstNode & node,
                                 const char * category,
                                 const std::string & detail)
{
  semantic_fallback_audit::hard_fail(
      category, ctx.source_location_for_node(node), detail);
}

bool should_defer_shift_rhs_id_analysis(const std::string & operator_name,
                                        const CppAstNode & rhs_node,
                                        const std::logic_error & error)
{
  if((operator_name != "operator<<" && operator_name != "operator>>") ||
     rhs_node.kind != CppAstKind::id_expression) {
    return false;
  }

  const std::string message = error.what();
  return message.rfind("unknown id-expression ", 0) == 0 ||
         message == "overloaded id-expression unsupported";
}

struct StreamShiftOverloadInput
{
  bool have_rhs = false;
  bool have_deferred_error = false;
  ExprInfo rhs;
  std::string deferred_error;
};

StreamShiftOverloadInput analyze_stream_shift_overload_input(SemanticContext & ctx,
                                                             Scope & scope,
                                                             const std::string & operator_name,
                                                             const CppAstNode & rhs_node)
{
  StreamShiftOverloadInput out;
  if(operator_name != "operator<<" && operator_name != "operator>>") {
    return out;
  }

  try
  {
    out.rhs = ctx.analyze_expression(scope, rhs_node);
    out.have_rhs = true;
    return out;
  }
  catch(const logic_error & error)
  {
    if(!should_defer_shift_rhs_id_analysis(operator_name, rhs_node, error)) {
      throw;
    }
    out.have_deferred_error = true;
    out.deferred_error = error.what();
    return out;
  }
}

bool is_non_class_builtin_binary_type(SemanticContext & ctx, const TypePtr & type)
{
  if(!type) {
    return false;
  }
  return complete_class_type_for_lookup(ctx, type) == nullptr;
}

TypePtr member_object_cv_source_type(const ExprInfo & base)
{
  TypePtr object_type = remove_reference_type(base.type);
  TypePtr stripped = strip_top_level_cv(object_type);
  if(stripped && stripped->kind == Type::TK_POINTER) {
    return stripped->inner;
  }
  return object_type;
}

TypePtr apply_member_object_cv(const TypePtr & member_type,
                               const TypePtr & object_type,
                               bool is_mutable_member = false)
{
  if(!member_type || !object_type || is_reference_type(member_type)) {
    return member_type;
  }

  TypePtr source = remove_reference_type(object_type);
  if(!source || source->kind != Type::TK_CV) {
    return member_type;
  }

  return apply_cv(member_type,
                  source->cv_const && !is_mutable_member,
                  source->cv_volatile);
}

TypePtr resolve_instantiated_member_object_type(SemanticContext & ctx,
                                                Scope & scope,
                                                const MemberValueLookupResult & member,
                                                const TypePtr & member_type)
{
  if(!member_type || !ctx.type_depends_on_template_parameter(member_type)) {
    return member_type;
  }

  TypePtr resolved_type = member_type;
  if(member.declared_in && member.declared_in->member_scope) {
    TypePtr resolved_in_owner;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx, *member.declared_in->member_scope, resolved_type, resolved_in_owner) &&
       resolved_in_owner) {
      resolved_type = resolved_in_owner;
    }
  }

  if(ctx.type_depends_on_template_parameter(resolved_type)) {
    TypePtr resolved_in_scope;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(
           ctx, scope, resolved_type, resolved_in_scope) &&
       resolved_in_scope) {
      resolved_type = resolved_in_scope;
    }
  }

  if(ctx.type_depends_on_template_parameter(resolved_type)) {
    FunctionBinding * function = current_function_scope(scope);
    ClassInfo * current_info =
        function && function->owner_class ? function->owner_class : current_class_scope(scope);
    TypePtr resolved_member_alias =
        semantic_class_model::resolve_instantiated_member_alias_type(ctx,
                                                                     scope,
                                                                     resolved_type,
                                                                     current_info);
    if(resolved_member_alias) {
      resolved_type = resolved_member_alias;
    }
  }

  return resolved_type;
}

bool try_analyze_member_pointer_data_access(SemanticContext & ctx,
                                            Scope & scope,
                                            const CppAstNode & node,
                                            const ExprInfo & object_expr,
                                            const ExprInfo & member_pointer_expr,
                                            ExprInfo & out)
{
  if(!node_has_simple_type(node, OP_DOTSTAR) &&
     !node_has_simple_type(node, OP_ARROWSTAR)) {
    return false;
  }

  TypePtr member_pointer_type =
      strip_top_level_cv(remove_reference_type(member_pointer_expr.type));
  if(!member_pointer_type ||
     member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
     is_function_type(member_pointer_type->inner)) {
    return false;
  }

  TypePtr owner_type = strip_top_level_cv(member_pointer_type->owner);
  if(!owner_type) {
    throw logic_error("invalid member-object pointer owner");
  }

  const TypePtr cv_qualified_owner_type =
      apply_member_object_cv(owner_type,
                             member_object_cv_source_type(object_expr));

  ExprInfo adjusted_object = object_expr;
  if(node_has_simple_type(node, OP_DOTSTAR)) {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(object_expr.type));
    if(!object_type) {
      throw logic_error("dot-star requires class object");
    }

    ExprInfo converted;
    if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                             cv_qualified_owner_type,
                                                             object_expr,
                                                             converted)) {
      adjusted_object = converted;
    } else if(!same_type_with_compatible_top_cv(owner_type, object_type)) {
      throw logic_error("dot-star requires class object matching member pointer owner");
    }
  } else {
    TypePtr object_type = value_conversion_type(object_expr);
    if(!object_type || object_type->kind != Type::TK_POINTER) {
      throw logic_error("arrow-star requires object pointer");
    }

    ExprInfo converted;
    TypePtr target_pointer = make_pointer(cv_qualified_owner_type);
    if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                             target_pointer,
                                                             object_expr,
                                                             converted)) {
      adjusted_object = converted;
    } else if(!same_type_with_compatible_top_cv(owner_type,
                                                strip_top_level_cv(object_type->inner))) {
      throw logic_error("arrow-star requires pointer matching member pointer owner");
    }
  }

  out.type = apply_member_object_cv(member_pointer_type->inner,
                                    member_object_cv_source_type(adjusted_object));
  if(is_reference_type(out.type)) {
    out.category = VC_LVALUE;
  } else if(node_has_simple_type(node, OP_ARROWSTAR)) {
    out.category = VC_LVALUE;
  } else {
    out.category = adjusted_object.category == VC_LVALUE ? VC_LVALUE : VC_XVALUE;
  }

  out.node = make_dump_node(CallSemKind::binary_expression, node.value);
  set_dump_token(out.node, node);
  ctx.set_expr_info_metadata(out, out.type, out.category);
  out.node.children.push_back(std::move(adjusted_object.node));
  out.node.children.push_back(member_pointer_expr.node);
  return true;
}

std::vector<TypePtr> builtin_numeric_conversion_targets()
{
  std::vector<TypePtr> out;
  out.push_back(make_fundamental(FT_INT));
  out.push_back(make_fundamental(FT_UNSIGNED_INT));
  out.push_back(make_fundamental(FT_LONG_INT));
  out.push_back(make_fundamental(FT_UNSIGNED_LONG_INT));
  out.push_back(make_fundamental(FT_LONG_LONG_INT));
  out.push_back(make_fundamental(FT_UNSIGNED_LONG_LONG_INT));
  out.push_back(make_fundamental(FT_FLOAT));
  out.push_back(make_fundamental(FT_DOUBLE));
  out.push_back(make_fundamental(FT_LONG_DOUBLE));
  return out;
}

TypePtr builtin_common_object_pointer_target()
{
  return make_pointer(make_cv(make_fundamental(FT_VOID), true, false));
}

std::vector<TypePtr> builtin_increment_reference_targets()
{
  const EFundamentalType fundamentals[] = {
      FT_BOOL,
      FT_CHAR,
      FT_SIGNED_CHAR,
      FT_UNSIGNED_CHAR,
      FT_WCHAR_T,
      FT_CHAR16_T,
      FT_CHAR32_T,
      FT_SHORT_INT,
      FT_UNSIGNED_SHORT_INT,
      FT_INT,
      FT_UNSIGNED_INT,
      FT_LONG_INT,
      FT_UNSIGNED_LONG_INT,
      FT_LONG_LONG_INT,
      FT_UNSIGNED_LONG_LONG_INT,
      FT_FLOAT,
      FT_DOUBLE,
      FT_LONG_DOUBLE,
  };

  std::vector<TypePtr> out;
  for(size_t i = 0; i < sizeof(fundamentals) / sizeof(fundamentals[0]); ++i) {
    out.push_back(make_lvalue_reference_raw(make_fundamental(fundamentals[i])));
  }
  return out;
}

bool is_complete_object_pointer_type(SemanticContext & ctx,
                                     const TypePtr & type);

bool is_builtin_increment_operand_type(SemanticContext & ctx,
                                       const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return is_integral_type(base) ||
         is_floating_type(base) ||
         is_complete_object_pointer_type(ctx, base);
}

bool try_builtin_increment_class_conversion(SemanticContext & ctx,
                                            Scope & scope,
                                            const ExprInfo & operand,
                                            ExprInfo & out)
{
  if(complete_class_type_for_lookup(ctx, value_conversion_type(operand)) == nullptr) {
    return false;
  }

  struct Candidate
  {
    ExprInfo expr;
    ConversionRank rank = CR_BAD;
  };

  std::vector<Candidate> candidates;
  const std::vector<TypePtr> targets = builtin_increment_reference_targets();
  for(size_t i = 0; i < targets.size(); ++i) {
    ExprInfo converted;
    ConversionRank rank = CR_BAD;
    if(!ctx.try_argument_conversion(scope,
                                    targets[i],
                                    operand,
                                    converted,
                                    rank,
                                    semantic_policy::default_argument_conversion())) {
      continue;
    }
    if(!is_modifiable_lvalue(converted) ||
       !is_builtin_increment_operand_type(ctx, converted.type)) {
      continue;
    }
    Candidate candidate;
    candidate.expr = converted;
    candidate.rank = rank;
    candidates.push_back(candidate);
  }

  if(candidates.empty()) {
    return false;
  }

  size_t best = 0;
  bool ambiguous = false;
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i].rank < candidates[best].rank) {
      best = i;
      ambiguous = false;
    } else if(candidates[i].rank == candidates[best].rank &&
              !type_equals(candidates[i].expr.type, candidates[best].expr.type)) {
      ambiguous = true;
    }
  }
  if(ambiguous) {
    return false;
  }

  out = candidates[best].expr;
  return true;
}

bool builtin_unary_operator_supports_type(const CppAstNode & node,
                                          const TypePtr & type)
{
  if(!type) {
    return false;
  }
  if(node_has_simple_type(node, OP_PLUS)) {
    return is_integral_or_unscoped_enum_type(type) ||
           is_floating_type(type) ||
           is_pointer_type(type);
  }
  if(node_has_simple_type(node, OP_MINUS)) {
    return is_integral_or_unscoped_enum_type(type) ||
           is_floating_type(type);
  }
  if(node_has_simple_type(node, OP_COMPL)) {
    return is_integral_or_unscoped_enum_type(type);
  }
  return false;
}

bool try_builtin_unary_class_conversion(SemanticContext & ctx,
                                        Scope & scope,
                                        const CppAstNode & node,
                                        const ExprInfo & operand,
                                        ExprInfo & out,
                                        const ArgumentConversionOptions & options)
{
  if(complete_class_type_for_lookup(ctx, value_conversion_type(operand)) == nullptr) {
    return false;
  }

  struct Candidate
  {
    ExprInfo expr;
    ConversionRank rank = CR_BAD;
  };

  std::vector<Candidate> candidates;
  if(node_has_simple_type(node, OP_PLUS)) {
    ExprInfo converted;
    TypePtr pointer_type;
    if(try_builtin_pointer_operand_conversion(ctx,
                                              scope,
                                              operand,
                                              converted,
                                              pointer_type,
                                              options)) {
      Candidate candidate;
      candidate.expr = converted;
      candidate.rank = CR_USER_DEFINED;
      candidates.push_back(candidate);
    }
  }
  const std::vector<TypePtr> targets = builtin_numeric_conversion_targets();
  for(size_t i = 0; i < targets.size(); ++i) {
    ExprInfo converted;
    ConversionRank rank = CR_BAD;
    if(!ctx.try_argument_conversion(scope,
                                    targets[i],
                                    operand,
                                    converted,
                                    rank,
                                    options)) {
      continue;
    }
    if(!builtin_unary_operator_supports_type(node, value_conversion_type(converted))) {
      continue;
    }
    Candidate candidate;
    candidate.expr = converted;
    candidate.rank = rank;
    candidates.push_back(candidate);
  }

  if(candidates.empty()) {
    return false;
  }

  size_t best = 0;
  bool ambiguous = false;
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i].rank < candidates[best].rank) {
      best = i;
      ambiguous = false;
    } else if(candidates[i].rank == candidates[best].rank &&
              !type_equals(value_conversion_type(candidates[i].expr),
                           value_conversion_type(candidates[best].expr))) {
      ambiguous = true;
    }
  }
  if(ambiguous) {
    return false;
  }

  out = candidates[best].expr;
  return true;
}

void maybe_complete_layout_type(SemanticContext & ctx, const TypePtr & type)
{
  if(!type) {
    return;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return;
  }
  if(base->kind == Type::TK_ARRAY) {
    maybe_complete_layout_type(ctx, base->inner);
    return;
  }
  if(base->kind == Type::TK_NAMED && !base->named_has_layout) {
    ctx.complete_class_type(base);
  }
}

bool prepare_sizeof_operand_type(SemanticContext & ctx, const TypePtr & type)
{
  maybe_complete_layout_type(ctx, type);
  return type_is_valid_sizeof_operand(type);
}

void set_expr_metadata(CallSemNode & node,
                       const TypePtr & type,
                       ValueCategory category)
{
  node.semantic_type = type;
  set_callsem_materialization_source_type(node, TypePtr());
  set_callsem_conversion_source_type(node, TypePtr());
  node.value_category = to_call_value_category(category);
}

bool try_analyze_qualified_member_pointer_expression(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const CppAstNode & operand_node,
                                                     ExprInfo & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(operand_node);
  if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
    return false;
  }

  TypePtr qualifier_type;
  const ValueBinding * value_binding =
      lookup_qualified_value_binding_node(ctx,
                                          scope,
                                          *qualified,
                                          operand_node,
                                          &qualifier_type);
  ValueBinding retained_value_binding;
  if(value_binding) {
    retained_value_binding = *value_binding;
    value_binding = &retained_value_binding;
  }
  ClassInfo * naming_class =
      qualifier_type ?
          ctx.class_info_for_type(strip_top_level_cv(remove_reference_type(qualifier_type))) :
          nullptr;

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(operand_node);
  vector<FunctionBinding *> functions =
      template_id ?
          ctx.lookup_function_template_id_node(
              scope,
              operand_node,
              *template_id,
              semantic_policy::without_body_instantiation()) :
          ctx.lookup_functions_node(
              scope,
              operand_node,
              operand_node.value,
              semantic_policy::without_body_instantiation());
  if(functions.size() == 1) {
    FunctionBinding * binding = functions[0];
    if(binding && binding->is_method && binding->owner_class &&
       !binding->is_constructor && !binding->is_destructor) {
      MemberAccess member_access = binding->access;
      if(binding->owner_class->member_scope) {
        member_access = effective_direct_function_access(
            *binding->owner_class->member_scope,
            binding->name,
            *binding);
      }
      MemberAccess path_access = MA_PUBLIC;
      if(naming_class && naming_class != binding->owner_class) {
        size_t path_offset = 0;
        if(!find_unique_base_path(*naming_class,
                                  binding->owner_class,
                                  path_offset,
                                  path_access)) {
          return false;
        }
      }
      if(!member_pointer_access_allowed(&scope,
                                        current_class_scope(scope),
                                        current_function_scope(scope),
                                        naming_class,
                                        binding->owner_class,
                                        member_access,
                                        path_access)) {
        throw logic_error("inaccessible member function pointer target");
      }
      TypePtr member_function_type = binding->declared_type;
      TypePtr stripped_function_type = strip_top_level_cv(member_function_type);
      const FunctionTypeRefQualifier ref_qualifier =
          binding->ref_qualifier == RQ_LVALUE ? FTRQ_LVALUE :
          binding->ref_qualifier == RQ_RVALUE ? FTRQ_RVALUE :
                                               FTRQ_NONE;
      if(stripped_function_type &&
         stripped_function_type->kind == Type::TK_FUNCTION &&
         (stripped_function_type->function_const != binding->is_const_method ||
          stripped_function_type->function_volatile != binding->is_volatile_method ||
          stripped_function_type->function_ref_qualifier != ref_qualifier)) {
        member_function_type = make_function(stripped_function_type->inner,
                                             stripped_function_type->params,
                                             stripped_function_type->variadic,
                                             binding->is_const_method,
                                             binding->is_volatile_method,
                                             stripped_function_type->prototype_relaxed,
                                             ref_qualifier);
      }
      ctx.require_function_definition(binding,
                                      OutputReason::FunctionIdUse,
                                      !binding->is_deleted);
      out.type = make_member_pointer(binding->owner_class->type, member_function_type);
      out.category = VC_PRVALUE;
      out.node = make_dump_node(CallSemKind::unary_expression, "&");

      CallSemNode child = make_dump_node(CallSemKind::id_expression, operand_node.value);
      child.is_c_linkage = binding->is_c_linkage;
      set_dump_symbol(child, binding->symbol);
      child.text = binding->name;
      set_expr_metadata(child, binding->type, VC_LVALUE);
      out.node.is_c_linkage = binding->is_c_linkage;
      set_dump_symbol(out.node, binding->symbol);
      if(binding->is_virtual) {
        out.node.is_virtual_dispatch = true;
        set_callsem_uint_value(out.node, binding->virtual_slot);
        out.node.uses_extended_vtable_layout =
            binding->owner_class &&
            semantic_class_model::class_uses_extended_virtual_abi(*binding->owner_class);
      }
      out.node.children.push_back(std::move(child));
      set_expr_metadata(out.node, out.type, out.category);
      return true;
    }
  }

  if(!value_binding || value_binding->kind != ValueBinding::VK_FIELD ||
     !value_binding->owner_class) {
    return false;
  }
  if(value_binding->is_bit_field) {
    throw logic_error("address-of bit-field member unsupported");
  }
  MemberAccess path_access = MA_PUBLIC;
  if(naming_class && naming_class != value_binding->owner_class) {
    size_t path_offset = 0;
    if(!find_unique_base_path(*naming_class,
                              value_binding->owner_class,
                              path_offset,
                              path_access)) {
      return false;
    }
  }
  if(!member_pointer_access_allowed(&scope,
                                    current_class_scope(scope),
                                    current_function_scope(scope),
                                    naming_class,
                                    value_binding->owner_class,
                                    value_binding->access,
                                    path_access)) {
    throw logic_error("inaccessible data member pointer target");
  }

  out.type = make_member_pointer(value_binding->owner_class->type, value_binding->type);
  out.category = VC_PRVALUE;
  out.node = make_dump_node(CallSemKind::unary_expression, "&");
  set_callsem_uint_value(out.node, value_binding->field_offset);

  CallSemNode child = make_dump_node(CallSemKind::id_expression, operand_node.value);
  set_expr_metadata(child, value_binding->type, VC_LVALUE);
  out.node.children.push_back(std::move(child));
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

TypePtr member_function_pointer_target_type(const FunctionBinding & binding)
{
  TypePtr member_function_type = binding.declared_type;
  TypePtr stripped_function_type = strip_top_level_cv(member_function_type);
  const FunctionTypeRefQualifier ref_qualifier =
      binding.ref_qualifier == RQ_LVALUE ? FTRQ_LVALUE :
      binding.ref_qualifier == RQ_RVALUE ? FTRQ_RVALUE :
                                           FTRQ_NONE;
  if(stripped_function_type &&
     stripped_function_type->kind == Type::TK_FUNCTION &&
     (stripped_function_type->function_const != binding.is_const_method ||
      stripped_function_type->function_volatile != binding.is_volatile_method ||
      stripped_function_type->function_ref_qualifier != ref_qualifier)) {
    member_function_type = make_function(stripped_function_type->inner,
                                         stripped_function_type->params,
                                         stripped_function_type->variadic,
                                         binding.is_const_method,
                                         binding.is_volatile_method,
                                         stripped_function_type->prototype_relaxed,
                                         ref_qualifier);
  }
  return member_function_type;
}

bool try_analyze_non_type_template_function_value(
    SemanticContext & ctx,
    const ValueBinding & binding,
    ExprInfo & out)
{
  FunctionBinding * function = nullptr;
  if(!binding.non_type_template_function_internal_symbol.empty()) {
    function = ctx.first_function_by_internal_symbol(
        binding.non_type_template_function_internal_symbol);
  }
  if(!function) {
    function = binding.non_type_template_function_value;
  }
  if(function && !function->symbol.object_symbol.empty()) {
    if(FunctionBinding * canonical =
           ctx.first_function_by_object_symbol(function->symbol.object_symbol)) {
      function = canonical;
    }
  }
  TypePtr binding_base = strip_top_level_cv(remove_reference_type(binding.type));
  if(!function || !binding_base) {
    return false;
  }

  TypePtr function_type = function->type;
  if((binding_base->kind == Type::TK_POINTER ||
      binding_base->kind == Type::TK_LVALUE_REFERENCE ||
      binding_base->kind == Type::TK_RVALUE_REFERENCE) &&
     binding_base->inner &&
     strip_top_level_cv(binding_base->inner)->kind == Type::TK_FUNCTION &&
     type_equals(strip_top_level_cv(binding_base->inner),
                 strip_top_level_cv(function_type))) {
    ctx.require_function_definition(function,
                                    OutputReason::FunctionIdUse,
                                    !function->is_deleted);
    if(FunctionBinding * refreshed =
           ctx.refresh_required_function_definition(function, true)) {
      function = refreshed;
    }
    out.type = function->type;
    out.category = VC_LVALUE;
    out.node = make_dump_node(CallSemKind::id_expression, function->name);
    out.node.is_c_linkage = function->is_c_linkage;
    set_dump_symbol(out.node, function->symbol);
    set_expr_metadata(out.node, out.type, out.category);
    return true;
  }

  if(binding_base->kind != Type::TK_MEMBER_POINTER ||
     !binding_base->inner ||
     !is_function_type(binding_base->inner) ||
     !function->is_method ||
     !function->owner_class ||
     function->is_constructor ||
     function->is_destructor ||
     !function->owner_class->type) {
    return false;
  }

  TypePtr member_function_type = member_function_pointer_target_type(*function);
  TypePtr member_pointer_type =
      make_member_pointer(function->owner_class->type, member_function_type);
  if(!type_equals(strip_top_level_cv(member_pointer_type), binding_base)) {
    return false;
  }

  ctx.require_function_definition(function,
                                  OutputReason::FunctionIdUse,
                                  !function->is_deleted);
  if(FunctionBinding * refreshed =
         ctx.refresh_required_function_definition(function, true)) {
    function = refreshed;
  }
  member_function_type = member_function_pointer_target_type(*function);
  member_pointer_type =
      make_member_pointer(function->owner_class->type, member_function_type);
  out.type = member_pointer_type;
  out.category = VC_PRVALUE;
  out.node = make_dump_node(CallSemKind::unary_expression, "&");
  out.node.has_token = true;
  out.node.token_type = OP_AMP;
  out.node.is_c_linkage = function->is_c_linkage;
  set_dump_symbol(out.node, function->symbol);
  if(function->is_virtual) {
    out.node.is_virtual_dispatch = true;
    set_callsem_uint_value(out.node, function->virtual_slot);
    out.node.uses_extended_vtable_layout =
        function->owner_class &&
        semantic_class_model::class_uses_extended_virtual_abi(*function->owner_class);
  }

  CallSemNode child = make_dump_node(CallSemKind::id_expression, function->name);
  child.is_c_linkage = function->is_c_linkage;
  child.text = function->name;
  set_dump_symbol(child, function->symbol);
  set_expr_metadata(child, function->type, VC_LVALUE);
  out.node.children.push_back(std::move(child));
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

ExprInfo make_typed_integer_literal_expr(SemanticContext & ctx,
                                         const TypePtr & type,
                                         unsigned long long value)
{
  ExprInfo result;
  result.type = type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::literal, to_string(value));
  ctx.set_expr_info_metadata(result, result.type, result.category);
  set_callsem_uint_value(result.node, value);
  return result;
}

string quote_ascii_string_literal(const string & text)
{
  string out = "\"";
  out.reserve(text.size() + 2);
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    switch(ch) {
    case '\\': out += "\\\\"; break;
    case '"': out += "\\\""; break;
    case '\n': out += "\\n"; break;
    case '\r': out += "\\r"; break;
    case '\t': out += "\\t"; break;
    default:
      out.push_back(ch);
      break;
    }
  }
  out += '"';
  return out;
}

ExprInfo make_string_literal_expr(SemanticContext & ctx, const string & text)
{
  ExprInfo result;
  result.type = make_array(make_cv(make_fundamental(FT_CHAR), true, false),
                           true,
                           text.size() + 1);
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::literal, quote_ascii_string_literal(text));
  ctx.set_expr_info_metadata(result, result.type, result.category);
  return result;
}

ExprInfo make_bit_field_storage_expr(SemanticContext & ctx,
                                     const ExprInfo & base,
                                     const ValueBinding & binding,
                                     size_t path_offset)
{
  ExprInfo result;
  result.type = apply_member_object_cv(binding.type,
                                       member_object_cv_source_type(base),
                                       binding.is_mutable);
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::member_expression, binding.name);
  ctx.set_expr_info_metadata(result, result.type, result.category);
  set_callsem_uint_value(result.node, path_offset + binding.field_offset);
  result.node.is_bit_field = true;
  result.node.is_reference_storage = is_reference_type(binding.type);
  set_callsem_bit_field_width(result.node, binding.bit_field_width);
  set_callsem_bit_field_offset(result.node, binding.bit_field_offset);
  set_callsem_bit_field_storage_size(result.node, binding.bit_field_storage_size);
  result.node.children.push_back(base.node);
  return result;
}

ExprInfo adjust_member_declaring_base_if_needed(SemanticContext & ctx,
                                                const ExprInfo & base,
                                                const TypePtr & member_object_type,
                                                const ClassInfo * object_class,
                                                const ClassInfo * declared_in,
                                                size_t path_offset)
{
  if(!object_class || !declared_in || object_class == declared_in) {
    return base;
  }

  TypePtr adjusted_type =
      apply_member_object_cv(declared_in->type, member_object_type, false);
  TypePtr base_type = strip_top_level_cv(remove_reference_type(base.type));
  if(base_type && base_type->kind == Type::TK_POINTER) {
    adjusted_type = make_pointer(adjusted_type);
  }
  return ctx.apply_base_subobject_adjustment(base,
                                             adjusted_type,
                                             *declared_in,
                                             path_offset);
}

size_t member_binding_owner_path_offset(const MemberValueLookupResult & member)
{
  size_t offset = member.path_offset;
  if(!member.binding ||
     member.binding->kind != ValueBinding::VK_FIELD ||
     !member.binding->owner_class ||
     !member.declared_in ||
     member.binding->owner_class == member.declared_in) {
    return offset;
  }

  size_t nested_offset = 0;
  MemberAccess nested_access = MA_PUBLIC;
  if(find_unique_base_path(*member.declared_in,
                           member.binding->owner_class,
                           nested_offset,
                           nested_access)) {
    offset += nested_offset;
  }
  return offset;
}

bool classify_literal_token(const string & text, EPPTokenType & out)
{
  vector<EPPToken> tokens = tokenize(text);
  bool found = false;
  for(size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].type == PP_WHITESPACE ||
       tokens[i].type == PP_NEW_LINE ||
       tokens[i].type == PP_EOF) {
      continue;
    }
    if(found) {
      return false;
    }
    out = tokens[i].type;
    found = true;
  }
  return found;
}

bool is_integer_literal(const string & text)
{
  EPPTokenType type = PP_EOF;
  return classify_literal_token(text, type) && type == PP_INT_LITERAL;
}

bool is_floating_literal(const string & text)
{
  EPPTokenType type = PP_EOF;
  return classify_literal_token(text, type) && type == PP_FLOAT_LITERAL;
}

bool classify_floating_literal_type(const string & text,
                                    EFundamentalType & out_type,
                                    string & ud_suffix)
{
  string value;
  return split_floating_literal(text, value, out_type, ud_suffix);
}

string overloaded_unary_operator_name(const CppAstNode & node, bool postfix)
{
  if(postfix) {
    if(node_has_simple_type(node, OP_INC)) {
      return "operator++";
    }
    if(node_has_simple_type(node, OP_DEC)) {
      return "operator--";
    }
    return string();
  }

  if(node_has_simple_type(node, OP_PLUS)) {
    return "operator+";
  }
  if(node_has_simple_type(node, OP_MINUS)) {
    return "operator-";
  }
  if(node_has_simple_type(node, OP_COMPL)) {
    return "operator~";
  }
  if(node_has_simple_type(node, OP_LNOT)) {
    return "operator!";
  }
  if(node_has_simple_type(node, OP_AMP)) {
    return "operator&";
  }
  if(node_has_simple_type(node, OP_STAR)) {
    return "operator*";
  }
  if(node_has_simple_type(node, OP_INC)) {
    return "operator++";
  }
  if(node_has_simple_type(node, OP_DEC)) {
    return "operator--";
  }
  return string();
}

TypePtr overloaded_operator_operand_base_type(const TypePtr & type)
{
  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  return strip_top_level_cv(base);
}

ClassInfo * complete_class_type_for_lookup(SemanticContext & ctx,
                                           const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return nullptr;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(info && info->class_kind == "enum") {
    return nullptr;
  }
  if(info && info->complete) {
    return info;
  }
  return info ? ctx.complete_class_type(base) : nullptr;
}

ClassInfo * class_info_for_operator_operand(SemanticContext & ctx,
                                            const TypePtr & type)
{
  TypePtr base = overloaded_operator_operand_base_type(type);
  if(!base) {
    return nullptr;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(info && info->complete) {
    return info;
  }
  if(info) {
    ClassInfo * completed = ctx.complete_class_type(base);
    return completed ? completed : info;
  }
  return nullptr;
}

struct OverloadableOperandInfo
{
  TypePtr base_type;
  ClassInfo * class_info = nullptr;
  bool has_class_operand = false;
  bool has_overloadable_operand = false;
};

bool named_type_has_enum_key(const TypePtr & type)
{
  return type &&
         type->kind == Type::TK_NAMED &&
         type->named_key.compare(0, 5, "enum ") == 0;
}

OverloadableOperandInfo classify_overloadable_operator_operand(SemanticContext & ctx,
                                                               const TypePtr & type)
{
  OverloadableOperandInfo out;
  out.base_type = overloaded_operator_operand_base_type(type);
  if(!out.base_type) {
    return out;
  }

  out.class_info = class_info_for_operator_operand(ctx, out.base_type);

  const bool enum_operand =
      (out.class_info && out.class_info->class_kind == "enum") ||
      named_type_has_enum_key(out.base_type);
  out.has_class_operand =
      out.class_info && out.class_info->class_kind != "enum";
  out.has_overloadable_operand = out.class_info || enum_operand;
  return out;
}

bool has_class_operand(SemanticContext & ctx, const TypePtr & type)
{
  return classify_overloadable_operator_operand(ctx, type).has_class_operand;
}

bool has_overloadable_operator_operand(SemanticContext & ctx, const TypePtr & type)
{
  return classify_overloadable_operator_operand(ctx, type).has_overloadable_operand;
}

bool is_complete_object_pointer_type(SemanticContext & ctx,
                                     const TypePtr & type)
{
  TypePtr pointer = strip_top_level_cv(type);
  if(!pointer || pointer->kind != Type::TK_POINTER || !pointer->inner) {
    return false;
  }

  TypePtr pointee = strip_top_level_cv(pointer->inner);
  if(!pointee ||
     pointee->kind == Type::TK_FUNCTION ||
     is_void_type(pointee)) {
    return false;
  }
  if(pointee->kind == Type::TK_NAMED && !pointee->named_complete) {
    ctx.complete_class_type(pointee);
  }
  return type_is_complete(pointee);
}

CppAstNode make_operator_identifier_node(const string & operator_name)
{
  CppAstNode identifier;
  identifier.kind = CppAstKind::identifier;
  identifier.value = operator_name;
  QualifiedName name;
  name.name = operator_name;
  set_cppast_qualified_name_syntax(identifier, name);
  return identifier;
}

CppAstNode make_dot_member_operator_callee(const CppAstNode & operand,
                                           const string & operator_name)
{
  CppAstNode callee;
  callee.kind = CppAstKind::member_expression;
  callee.has_token = true;
  callee.token_kind = RT_SIMPLE;
  callee.simple_type = OP_DOT;
  callee.value = ".";
  callee.children.push_back(operand);
  callee.children.push_back(make_operator_identifier_node(operator_name));
  return callee;
}

CppAstNode make_operator_argument_list(const CppAstNode & operand,
                                       bool postfix)
{
  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.children.push_back(operand);
  if(postfix) {
    CppAstNode zero;
    zero.kind = CppAstKind::literal;
    zero.value = "0";
    arguments.children.push_back(zero);
  }
  return arguments;
}

bool function_binding_accepts_argument_count(const FunctionBinding * binding,
                                             size_t argument_count)
{
  if(!binding) {
    return false;
  }

  size_t required_count = binding->params.size();
  while(required_count > 0 &&
        required_count - 1 < binding->default_arguments.size() &&
        binding->default_arguments[required_count - 1]) {
    --required_count;
  }
  if(argument_count < required_count) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(binding->type);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= binding->params.size()) {
    return true;
  }
  return argument_count <= binding->params.size();
}

void filter_function_bindings_by_argument_count(vector<FunctionBinding *> & functions,
                                                size_t argument_count)
{
  vector<FunctionBinding *> filtered;
  filtered.reserve(functions.size());
  for(size_t i = 0; i < functions.size(); ++i) {
    if(function_binding_accepts_argument_count(functions[i], argument_count)) {
      filtered.push_back(functions[i]);
    }
  }
  functions.swap(filtered);
}

bool new_type_id_implies_empty_initializer(const CppAstNode & type_id)
{
  const CppAstNode * abstract = find_child(type_id, CppAstKind::abstract_declarator);
  if(!abstract || abstract->children.size() != 1) {
    return false;
  }
  const CppAstNode & child = abstract->children[0];
  return child.kind == CppAstKind::parameter_clause && child.children.empty();
}

CppAstNode strip_new_type_id_empty_initializer(const CppAstNode & type_id)
{
  CppAstNode adjusted = type_id;
  if(!new_type_id_implies_empty_initializer(type_id)) {
    return adjusted;
  }

  adjusted.children.clear();
  for(size_t i = 0; i < type_id.children.size(); ++i) {
    if(type_id.children[i].kind != CppAstKind::abstract_declarator) {
      adjusted.children.push_back(type_id.children[i]);
    }
  }
  return adjusted;
}

const CppAstNode * find_new_array_bound_expression(const CppAstNode & type_id)
{
  const CppAstNode * abstract = find_child(type_id, CppAstKind::abstract_declarator);
  if(!abstract) {
    return nullptr;
  }
  for(size_t i = 0; i < abstract->children.size(); ++i) {
    const CppAstNode & child = abstract->children[i];
    if(child.kind != CppAstKind::array_suffix) {
      continue;
    }
    if(child.children.size() != 1) {
      return nullptr;
    }
    return &child.children[0];
  }
  return nullptr;
}

size_t class_array_new_cookie_size(const TypePtr & element_type)
{
  return max<size_t>(8, type_alignment(element_type));
}

CppAstNode add_new_array_cookie_size(CppAstNode size_expr,
                                     size_t cookie_size)
{
  if(cookie_size == 0) {
    return size_expr;
  }

  CppAstNode adjusted;
  adjusted.kind = CppAstKind::binary_expression;
  adjusted.value = "+";
  adjusted.has_token = true;
  adjusted.token_kind = RT_SIMPLE;
  adjusted.simple_type = OP_PLUS;
  adjusted.children.push_back(size_expr);

  CppAstNode cookie;
  cookie.kind = CppAstKind::literal;
  cookie.value = to_string(cookie_size);
  adjusted.children.push_back(cookie);
  return adjusted;
}

CppAstNode build_new_array_size_expression(const CppAstNode & bound_expr,
                                           size_t element_size,
                                           size_t cookie_size)
{
  if(element_size == 1) {
    return add_new_array_cookie_size(bound_expr, cookie_size);
  }

  CppAstNode size_expr;
  size_expr.kind = CppAstKind::binary_expression;
  size_expr.value = "*";
  size_expr.has_token = true;
  size_expr.token_kind = RT_SIMPLE;
  size_expr.simple_type = OP_STAR;
  size_expr.children.push_back(bound_expr);

  CppAstNode factor;
  factor.kind = CppAstKind::literal;
  factor.value = to_string(element_size);
  size_expr.children.push_back(factor);
  return add_new_array_cookie_size(size_expr, cookie_size);
}

bool parse_new_placement_argument_nodes(const CppAstNode & placement,
                                        vector<CppAstNode> & out)
{
  out.clear();
  const CppAstNode * arguments = find_child(placement, CppAstKind::paren_argument_list);
  if(!arguments) {
    return false;
  }

  out = arguments->children;
  return true;
}

vector<const CppAstNode *> new_initializer_argument_nodes(const CppAstNode & node)
{
  vector<const CppAstNode *> args;
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return new_initializer_argument_nodes(node.children[0]);
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

bool new_initializer_is_braced(const CppAstNode & node)
{
  if(node.kind == CppAstKind::braced_init_list) {
    return true;
  }
  return node.kind == CppAstKind::initializer &&
         node.children.size() == 1 &&
         new_initializer_is_braced(node.children[0]);
}

vector<const CppAstNode *> expand_new_argument_nodes(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const vector<const CppAstNode *> & args,
                                                     vector<unique_ptr<CppAstNode> > & storage)
{
  storage.clear();
  vector<const CppAstNode *> out;
  for(size_t i = 0; i < args.size(); ++i) {
    if(args[i]->kind != CppAstKind::pack_expansion_expression) {
      out.push_back(args[i]);
      continue;
    }
    vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, *args[i], expanded_nodes)) {
      throw logic_error("unsupported new-expression pack-expansion argument");
    }
    for(size_t j = 0; j < expanded_nodes.size(); ++j) {
      storage.emplace_back(new CppAstNode(expanded_nodes[j]));
      out.push_back(storage.back().get());
    }
  }
  return out;
}

CallSemNode make_bound_callee_node(SemanticContext & ctx, FunctionBinding & binding)
{
  FunctionBinding * resolved =
      semantic_output::resolve_output_function_binding(ctx, &binding);
  FunctionBinding & emitted = resolved ? *resolved : binding;
  CallSemNode callee = make_dump_node(CallSemKind::callee, emitted.name);
  set_callsem_resolved_name(callee, function_output_name(emitted));
  callee.semantic_type = emitted.type;
  callee.is_c_linkage = emitted.is_c_linkage;
  callee.is_semantically_nothrow = ctx.function_binding_is_nothrow(emitted);
  set_callsem_runtime_bridge_symbol(
      callee,
      runtime_bridge_symbol_for_bound_function(
          emitted.name,
          emitted.owner_class ? emitted.owner_class->qualified_name : "",
          emitted.type));
  set_dump_symbol(callee, emitted.symbol);
  return callee;
}

bool try_overloaded_unary_operator(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node,
                                   bool postfix,
                                   ExprInfo & out)
{
  if(node.children.size() != 1) {
    return false;
  }

  const string operator_name = overloaded_unary_operator_name(node, postfix);
  if(operator_name.empty()) {
    return false;
  }

  const auto normalize_overloaded_result_category =
      [&](ExprInfo & result)
      {
        TypePtr node_semantic_type = result.node.semantic_type;
        if(is_reference_type(strip_top_level_cv(result.type))) {
          ValueCategory category = VC_PRVALUE;
          if(!result_value_category_for_function_result(result.type, category)) {
            throw logic_error("invalid overloaded operator result");
          }
          result.type = expression_type_for_function_result(result.type);
          result.category = category;
        }
        ctx.set_expr_info_metadata(result, result.type, result.category);
        if(result.node.kind == CallSemKind::call_expression && node_semantic_type) {
          result.node.semantic_type = node_semantic_type;
        }
      };

  ExprInfo operand = ctx.analyze_expression(scope, node.children[0]);
  if(parser_trace::enabled("overload")) {
    OverloadableOperandInfo operand_info =
        classify_overloadable_operator_operand(ctx, operand.type);
    ostringstream trace;
    trace << "unary-operator-operand"
          << " op=" << operator_name
          << " operand=" << describe_type(operand.type)
          << " base="
          << (operand_info.base_type ? describe_type(operand_info.base_type) :
                                      string("<none>"))
          << " class="
          << (operand_info.class_info ?
                  operand_info.class_info->qualified_name :
                  string("<none>"))
          << " has_class=" << (operand_info.has_class_operand ? "yes" : "no")
          << " overloadable="
          << (operand_info.has_overloadable_operand ? "yes" : "no");
    parser_trace::note("overload", ctx.source_location_for_node(node), trace.str());
  }
  if(!has_overloadable_operator_operand(ctx, operand.type)) {
    return false;
  }

  TypePtr operand_type = overloaded_operator_operand_base_type(operand.type);

  ClassInfo * class_info = complete_class_type_for_lookup(ctx, operand_type);
  if(class_info) {
    MemberFunctionLookupResult member_candidates =
        lookup_visible_member_functions(*class_info, operator_name);
    filter_function_bindings_by_argument_count(
        member_candidates.functions,
        postfix ? 2 : 1);
    MemberFunctionTemplateLookupResult member_template_candidates =
        lookup_visible_member_function_templates(*class_info, operator_name);
    if(parser_trace::enabled("overload")) {
      ostringstream trace;
      trace << "unary-operator-member-candidates"
            << " op=" << operator_name
            << " operand=" << describe_type(operand.type)
            << " class=" << class_info->qualified_name
            << " function_count=" << member_candidates.functions.size()
            << " template_count=" << member_template_candidates.templates.size();
      parser_trace::note("overload", ctx.source_location_for_node(node), trace.str());
    }
    if(!member_candidates.functions.empty() ||
       !member_template_candidates.templates.empty()) {
      CppAstNode call;
      call.kind = CppAstKind::call_expression;
      call.children.push_back(make_dot_member_operator_callee(node.children[0], operator_name));

      CppAstNode arguments;
      arguments.kind = CppAstKind::paren_argument_list;
      if(postfix) {
        CppAstNode zero;
        zero.kind = CppAstKind::literal;
        zero.value = "0";
        arguments.children.push_back(zero);
      }
      call.children.push_back(arguments);
      semantic_overload::CallAnalysisHints hints;
      hints.use_location = ctx.source_location_for_node(node);
      out = ctx.analyze_call_expression(
          scope,
          call,
          semantic_overload::CallAnalysisOptions(true, &hints));
      normalize_overloaded_result_category(out);
      return true;
    }
  }

  vector<TypePtr> operator_operand_types;
  operator_operand_types.push_back(operand.type);
  semantic_overload::NonmemberOperatorCandidateSet operator_candidates;
  semantic_overload::collect_nonmember_operator_candidates(
      ctx,
      scope,
      operator_name,
      operator_operand_types,
      postfix ? 2 : 1,
      operator_candidates,
      semantic_overload::MERGE_FRIEND_TEMPLATES_BEFORE_ASSOCIATED_SCOPES);
  vector<FunctionBinding *> operator_functions = operator_candidates.functions;
  vector<FunctionTemplateDecl *> operator_templates = operator_candidates.templates;
  if(operator_functions.empty() && operator_templates.empty()) {
    return false;
  }

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = operator_name;

  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.children.push_back(callee);
  call.children.push_back(make_operator_argument_list(node.children[0], postfix));

  Scope operator_scope(&scope, "", false);
  semantic_overload::initialize_operator_candidate_scope(
      operator_scope,
      scope,
      operator_name,
      operator_candidates.associated_scopes,
      operator_functions,
      operator_templates);
  semantic_overload::CallAnalysisHints hints;
  hints.use_location = ctx.source_location_for_node(node.children[0]);
  hints.adl_candidates_precollected = true;
  const auto handle_unresolved_operator =
      [&](const logic_error & error) -> bool
      {
        if(node_has_simple_type(node, OP_LNOT)) {
          ExprInfo bool_operand = operand;
          if(try_condition_test_conversion(ctx, scope, bool_operand)) {
            return true;
          }
        }
        if(!has_class_operand(ctx, operand.type)) {
          // Enum-only unary lookup may legitimately continue to builtins.
          return true;
        }
        if(node_has_simple_type(node, OP_PLUS) ||
           node_has_simple_type(node, OP_MINUS) ||
           node_has_simple_type(node, OP_COMPL)) {
          ArgumentConversionOptions builtin_probe_options =
              semantic_policy::without_user_defined_body_instantiation();
          builtin_probe_options.materialize_user_defined_output = false;
          builtin_probe_options.materialize_standard_adjustments = false;
          ExprInfo converted_probe;
          if(try_builtin_unary_class_conversion(ctx,
                                                scope,
                                                node,
                                                operand,
                                                converted_probe,
                                                builtin_probe_options)) {
            return true;
          }
        }
        if(node.value == "&") {
          // Built-in address-of remains viable after non-member operator&
          // candidates from ordinary lookup or ADL are rejected.
          return true;
        }
        return false;
      };
  try {
    out = ctx.analyze_call_expression(
        operator_scope,
        call,
        semantic_overload::CallAnalysisOptions(true, &hints));
    normalize_overloaded_result_category(out);
    return true;
  } catch(const NoViableOverloadError & error) {
    if(handle_unresolved_operator(error)) {
      return false;
    }
    throw;
  } catch(const UnknownFunctionError & error) {
    if(handle_unresolved_operator(error)) {
      return false;
    }
    throw;
  }
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  return find_child(node, kind);
}

TypePtr rtti_object_type(SemanticContext & ctx, Scope & scope)
{
  QualifiedName type_info_name;
  type_info_name.qualifiers.push_back("std");
  type_info_name.name = "type_info";
  TypePtr type_info =
      ctx.lookup_non_template_type_name(scope, type_info_name);
  if(!type_info) {
    throw logic_error("typeid requires declared std::type_info");
  }
  return make_cv(type_info, true, false);
}

bool type_info_comparison_operator_declared(SemanticContext & ctx,
                                            const TypePtr & type,
                                            const std::string & operator_name)
{
  ClassInfo * info = complete_class_type_for_lookup(ctx, type);
  if(!info) {
    return false;
  }
  if(!lookup_visible_member_functions(*info, operator_name).functions.empty()) {
    return true;
  }
  return !lookup_visible_member_function_templates(*info, operator_name).templates.empty();
}

bool is_initializer_list_type(SemanticContext & ctx,
                              const TypePtr & type,
                              TypePtr * element_type = nullptr)
{
  ClassInfo * info = ctx.class_info_for_type(type);
  if(!info || !info->is_initializer_list || !info->initializer_list_element_type) {
    return false;
  }
  if(element_type) {
    *element_type = info->initializer_list_element_type;
  }
  return true;
}

bool lookup_member_value_in_scope_chain(Scope & scope,
                                        const string & name,
                                        MemberValueLookupResult & out)
{
  for(Scope * current = &scope; current; current = current->parent) {
    map<string, ValueBinding>::const_iterator found = current->values.find(name);
    if(found != current->values.end()) {
      return false;
    }
    const bool has_lexical_class =
        !current->class_info && current->function && current->function->lexical_access_class;
    const bool lexical_only =
        has_lexical_class &&
        (!current->function->is_method ||
         current->function->owner_class != current->function->lexical_access_class);
    ClassInfo * lexical_class = current->class_info;
    if(!lexical_class && has_lexical_class) {
      lexical_class = current->function->lexical_access_class;
    }
    if(lexical_class) {
      MemberValueLookupResult member = lookup_member_value(*lexical_class, name);
      if(lexical_only && member.binding && member.binding->kind == ValueBinding::VK_FIELD) {
        member.binding = nullptr;
      }
      if(member.binding) {
        out = member;
        return true;
      }
    }
  }
  return false;
}

bool member_value_lookup_result_for_binding(Scope & scope,
                                            const ValueBinding & binding,
                                            MemberValueLookupResult & out)
{
  if(binding.kind != ValueBinding::VK_FIELD || !binding.owner_class) {
    return false;
  }
  ClassInfo * current_class = current_class_scope(scope);
  if(!current_class) {
    return false;
  }

  size_t path_offset = 0;
  MemberAccess path_access = MA_PUBLIC;
  if(binding.owner_class != current_class &&
     !find_unique_base_path(*current_class,
                            binding.owner_class,
                            path_offset,
                            path_access)) {
    return false;
  }

  out = MemberValueLookupResult();
  out.binding = &binding;
  out.declared_in = binding.owner_class;
  out.path_access = path_access;
  out.path_offset = path_offset;
  return true;
}

ClassInfo * effective_function_class_for_this(Scope * current)
{
  if(!current || !current->function) {
    return nullptr;
  }
  return current->class_info ? current->class_info : current->function->owner_class;
}

Scope * find_this_function_scope(Scope & scope,
                                 bool & saw_member_function_scope,
                                 ClassInfo *& function_class)
{
  Scope * function_scope = nullptr;
  saw_member_function_scope = false;
  function_class = nullptr;
  for(Scope * current = &scope; current; current = current->parent) {
    if(!current->function) {
      continue;
    }
    ClassInfo * current_class = effective_function_class_for_this(current);
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
  return function_scope;
}

ExprInfo make_raw_this_expr(SemanticContext & ctx,
                            Scope & scope,
                            const CppAstNode & node,
                            Scope *& function_scope,
                            ClassInfo *& function_class)
{
  bool saw_member_function_scope = false;
  function_scope = find_this_function_scope(scope, saw_member_function_scope, function_class);
  if(!saw_member_function_scope) {
    throw logic_error("this outside member function");
  }
  if(!function_scope) {
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
  set_dump_token(result.node, node);
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo make_implicit_member_expression_impl(SemanticContext & ctx,
                                              Scope & scope,
                                              const MemberValueLookupResult & member)
{
  const ClassInfo * current_class = current_class_scope(scope);
  const ValueBinding & binding = *member.binding;
  if(!binding.owner_class || !current_class ||
     !member_access_allowed(&scope, current_class, current_function_scope(scope),
                            member.declared_in, binding.access, member.path_access)) {
    throw logic_error("inaccessible member");
  }

  CppAstNode this_node;
  this_node.kind = CppAstKind::keyword_literal;
  this_node.has_token = true;
  this_node.token_kind = RT_SIMPLE;
  this_node.simple_type = KW_THIS;
  this_node.value = "this";

  ExprInfo base;
  if(current_class->is_lambda_closure && binding.owner_class == current_class) {
    Scope * function_scope = nullptr;
    ClassInfo * function_class = nullptr;
    base = make_raw_this_expr(ctx, scope, this_node, function_scope, function_class);
  } else {
    base = ctx.analyze_this_expression(scope, this_node);
  }
  if(binding.is_bit_field) {
    const size_t owner_path_offset = member_binding_owner_path_offset(member);
    ExprInfo adjusted_base =
        adjust_member_declaring_base_if_needed(ctx,
                                               base,
                                               member_object_cv_source_type(base),
                                               current_class,
                                               binding.owner_class,
                                               owner_path_offset);
    return make_bit_field_storage_expr(ctx,
                                       adjusted_base,
                                       binding,
                                       binding.owner_class == current_class ? owner_path_offset : 0);
  }
  const size_t owner_path_offset = member_binding_owner_path_offset(member);
  ExprInfo member_base =
      adjust_member_declaring_base_if_needed(ctx,
                                             base,
                                             member_object_cv_source_type(base),
                                             current_class,
                                             binding.owner_class,
                                             owner_path_offset);
  ExprInfo result;
  result.type = apply_member_object_cv(binding.type,
                                       member_object_cv_source_type(base),
                                       binding.is_mutable);
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::member_expression, binding.name);
  set_expr_metadata(result.node, result.type, result.category);
  set_callsem_uint_value(
      result.node,
      (binding.owner_class == current_class ? owner_path_offset : 0) +
          binding.field_offset);
  result.node.is_reference_storage = is_reference_type(binding.type);
  result.node.children.push_back(std::move(member_base.node));
  return result;
}

vector<string> split_comma_list(const string & text)
{
  vector<string> out;
  string current;
  int depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if(c == '<' || c == '(' || c == '[' || c == '{') {
      ++depth;
    } else if(c == '>' || c == ')' || c == ']' || c == '}') {
      --depth;
    } else if(c == ',' && depth == 0) {
      out.push_back(semantic_utils::trim_space(current));
      current.clear();
      continue;
    }
    current.push_back(c);
  }
  if(!current.empty()) {
    out.push_back(semantic_utils::trim_space(current));
  }
  return out;
}

enum LambdaCaptureDefaultMode
{
  LCD_NONE,
  LCD_BY_COPY,
  LCD_BY_REFERENCE
};

bool parse_lambda_capture_list(const string & introducer,
                               vector<pair<string, bool> > & captures,
                               LambdaCaptureDefaultMode & default_mode)
{
  captures.clear();
  default_mode = LCD_NONE;
  if(introducer.size() < 2 || introducer.front() != '[' || introducer.back() != ']') {
    return false;
  }
  const string body =
      semantic_utils::trim_space(introducer.substr(1, introducer.size() - 2));
  if(body.empty()) {
    return true;
  }
  const vector<string> items = split_comma_list(body);
  for(size_t i = 0; i < items.size(); ++i) {
    const string item = semantic_utils::trim_space(items[i]);
    if(item.empty()) {
      return false;
    }
    if(item == "this") {
      captures.push_back(make_pair(string("this"), false));
      continue;
    }
    if(item == "=" || item == "&") {
      if(default_mode != LCD_NONE) {
        return false;
      }
      default_mode = item == "=" ? LCD_BY_COPY : LCD_BY_REFERENCE;
      continue;
    }
    bool by_reference = false;
    string name = item;
    if(name[0] == '&') {
      by_reference = true;
      name = semantic_utils::trim_space(name.substr(1));
    }
    if(name.empty() || name == "this" || name.find('=') != string::npos) {
      return false;
    }
    captures.push_back(make_pair(name, by_reference));
  }
  return true;
}

void collect_declared_names(const CppAstNode & node,
                            std::set<std::string> & declared_names)
{
  if(node.kind == CppAstKind::declarator && node.value.empty()) {
    const CppAstNode * identifier = find_child_kind(node, CppAstKind::identifier);
    if(identifier && !identifier->value.empty()) {
      declared_names.insert(identifier->value);
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_declared_names(node.children[i], declared_names);
  }
}

void collect_implicit_lambda_capture_names(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & node,
                                           std::set<std::string> & declared_names,
                                           std::vector<std::string> & out,
                                           std::set<std::string> & seen);

const std::vector<ValueBinding> * lookup_named_value_pack_for_capture(
    Scope & scope,
    const std::string & name)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    std::map<std::string, std::vector<ValueBinding> >::const_iterator found =
        current->named_value_packs.find(name);
    if(found != current->named_value_packs.end()) {
      return &found->second;
    }
  }
  return nullptr;
}

void append_capture_name(const std::string & name,
                         std::vector<std::string> & out,
                         std::set<std::string> & seen)
{
  if(!name.empty() && seen.insert(name).second) {
    out.push_back(name);
  }
}

bool recover_function_style_local_initializer_for_capture(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & declaration,
    const CppAstNode & init_decl,
    CppAstNode & recovered_initializer)
{
  recovered_initializer = CppAstNode();
  if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
    return false;
  }

  const CppAstNode * specifiers =
      find_child_kind(declaration, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators =
      find_child_kind(declaration, CppAstKind::init_declarator_list);
  if(!specifiers || !declarators || init_decl.children.size() > 1) {
    return false;
  }

  try {
    PreparedDeclarationSpecifiers prepared_specifiers;
    if(!ctx.prepare_block_scope_declaration_specifiers(scope,
                                                       *specifiers,
                                                       declarators,
                                                       prepared_specifiers)) {
      return false;
    }
    if(prepared_specifiers.declaration_is_typedef ||
       !prepared_specifiers.parsed_decl_spec ||
       (!prepared_specifiers.has_auto && !prepared_specifiers.base)) {
      return false;
    }

    const CppAstNode & declarator_node = init_decl.children[0];
    std::string name;
    TypePtr type;
    bool is_typedef = prepared_specifiers.declaration_is_typedef;
    const bool parsed_as_variable =
        ctx.parse_variable_declaration_type(scope,
                                           prepared_specifiers.resolved_specifiers,
                                           declarator_node,
                                           nullptr,
                                           true,
                                           name,
                                           type,
                                           is_typedef);
    const bool parsed_as_function =
        parsed_as_variable && type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION;
    if(parsed_as_variable && !parsed_as_function) {
      return false;
    }

    CppAstNode stripped_declarator;
    std::string recovery_error;
    if(!semantic_parameter_recovery::recover_function_style_initializer_declarator(
           declarator_node,
           stripped_declarator,
           recovered_initializer,
           recovery_error)) {
      return false;
    }

    name.clear();
    type.reset();
    is_typedef = prepared_specifiers.declaration_is_typedef;
    if(!ctx.parse_variable_declaration_type(scope,
                                           prepared_specifiers.resolved_specifiers,
                                           stripped_declarator,
                                           &recovered_initializer,
                                           true,
                                           name,
                                           type,
                                           is_typedef) ||
       is_typedef ||
       !type ||
       strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
      return false;
    }

    return true;
  }
  catch(const std::logic_error &)
  {
    return false;
  }
}

void collect_function_style_local_initializer_captures(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & declaration,
    std::set<std::string> & declared_names,
    std::vector<std::string> & out,
    std::set<std::string> & seen)
{
  if(declaration.kind != CppAstKind::simple_declaration) {
    return;
  }
  const CppAstNode * declarators =
      find_child_kind(declaration, CppAstKind::init_declarator_list);
  if(!declarators) {
    return;
  }
  for(size_t i = 0; i < declarators->children.size(); ++i) {
    CppAstNode recovered_initializer;
    if(recover_function_style_local_initializer_for_capture(
           ctx, scope, declaration, declarators->children[i], recovered_initializer)) {
      collect_implicit_lambda_capture_names(
          ctx, scope, recovered_initializer, declared_names, out, seen);
    }
  }
}

void collect_implicit_lambda_capture_names(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & node,
                                           std::set<std::string> & declared_names,
                                           std::vector<std::string> & out,
                                           std::set<std::string> & seen)
{
  const auto binding_requires_lambda_capture = [](const ValueBinding & binding) -> bool
  {
    if(binding.declaration_node &&
       binding.declaration_node->kind == CppAstKind::enumerator) {
      return false;
    }
    if(binding.kind == ValueBinding::VK_PARAMETER ||
       binding.kind == ValueBinding::VK_FIELD) {
      return true;
    }
    if(binding.owner_class ||
       binding.is_thread_local ||
       !binding.symbol.internal_symbol.empty() ||
       !binding.symbol.object_symbol.empty()) {
      return false;
    }
    return !binding.declaration_scope ||
           (!binding.declaration_scope->namespace_scope &&
            binding.declaration_scope->parent != nullptr);
  };

  if(node.kind == CppAstKind::lambda_expression) {
    const CppAstNode * introducer = find_child_kind(node, CppAstKind::lambda_introducer);
    const CppAstNode * declarator = find_child_kind(node, CppAstKind::lambda_declarator);
    const CppAstNode * body = find_child_kind(node, CppAstKind::compound_statement);

    vector<pair<string, bool> > nested_captures;
    LambdaCaptureDefaultMode nested_default = LCD_NONE;
    if(introducer &&
      parse_lambda_capture_list(introducer->value, nested_captures, nested_default)) {
      for(size_t i = 0; i < nested_captures.size(); ++i) {
        if(declared_names.count(nested_captures[i].first) == 0 &&
           seen.insert(nested_captures[i].first).second) {
          out.push_back(nested_captures[i].first);
        }
      }
      if(nested_default != LCD_NONE && body) {
        std::set<std::string> nested_declared_names = declared_names;
        if(declarator) {
          collect_declared_names(*declarator, nested_declared_names);
        }
        collect_implicit_lambda_capture_names(
            ctx, scope, *body, nested_declared_names, out, seen);
      }
    }
    return;
  }

  if(node.kind == CppAstKind::keyword_literal &&
     node.has_token &&
     node.simple_type == KW_THIS) {
    if(seen.insert("this").second) {
      out.push_back("this");
    }
  }

  if(node.kind == CppAstKind::simple_declaration ||
     node.kind == CppAstKind::for_init_statement ||
     node.kind == CppAstKind::condition) {
    std::set<std::string> extended = declared_names;
    collect_declared_names(node, extended);
    collect_function_style_local_initializer_captures(
        ctx, scope, node, extended, out, seen);
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_implicit_lambda_capture_names(ctx, scope, node.children[i], extended, out, seen);
    }
    return;
  }

  if(node.kind == CppAstKind::compound_statement) {
    std::set<std::string> active_declared_names = declared_names;
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_implicit_lambda_capture_names(
          ctx, scope, node.children[i], active_declared_names, out, seen);
      if(node.children[i].kind == CppAstKind::simple_declaration) {
        collect_declared_names(node.children[i], active_declared_names);
      }
    }
    return;
  }

  if(node.kind == CppAstKind::id_expression) {
    const QualifiedName * qualified = cppast_qualified_name_syntax(node);
    if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
      const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
      const std::string & lookup_name =
          template_id && !template_id->name.name.empty() ?
              template_id->name.name :
              node.value;
      if(declared_names.count(lookup_name) == 0) {
        const ValueBinding * binding = ctx.lookup_value(scope, lookup_name);
        if(binding) {
          if(!binding_requires_lambda_capture(*binding)) {
            return;
          }
          const std::vector<ValueBinding> * value_pack =
              lookup_named_value_pack_for_capture(scope, lookup_name);
          if(value_pack) {
            for(size_t i = 0; i < value_pack->size(); ++i) {
              if(binding_requires_lambda_capture((*value_pack)[i])) {
                append_capture_name((*value_pack)[i].name, out, seen);
              }
            }
            return;
          }
          if(binding->kind == ValueBinding::VK_FIELD) {
            if(binding->owner_class &&
               binding->owner_class->is_lambda_closure &&
               binding->name != "this") {
              append_capture_name(binding->name, out, seen);
            } else {
              append_capture_name("this", out, seen);
            }
          } else {
            append_capture_name(lookup_name, out, seen);
          }
        } else {
          MemberValueLookupResult member;
          if(lookup_member_value_in_scope_chain(scope, lookup_name, member) &&
             member.binding) {
            append_capture_name("this", out, seen);
          } else {
            for(Scope * current = &scope; current; current = current->parent) {
              if(!current->class_info) {
                continue;
              }
              const MemberCallableLookupResult member_callables =
                  lookup_visible_member_callables(*current->class_info, lookup_name);
              if(!member_callables.functions.empty() ||
                 !member_callables.templates.empty()) {
                bool requires_implicit_object = false;
                for(size_t i = 0; i < member_callables.functions.size(); ++i) {
                  if(member_callables.functions[i] &&
                     member_callables.functions[i]->is_method) {
                    requires_implicit_object = true;
                    break;
                  }
                }
                for(size_t i = 0;
                    !requires_implicit_object && i < member_callables.templates.size();
                    ++i) {
                  if(member_callables.templates[i] &&
                     !member_callables.templates[i]->is_static_member) {
                    requires_implicit_object = true;
                  }
                }
                if(requires_implicit_object) {
                  append_capture_name("this", out, seen);
                }
                break;
              }
            }
          }
        }
      }
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_implicit_lambda_capture_names(ctx, scope, node.children[i], declared_names, out, seen);
  }
}

ExprInfo build_closure_object_expr(SemanticContext & ctx,
                                   Scope & scope,
                                   ClassInfo & closure_info)
{
  ExprInfo closure;
  closure.type = closure_info.type;
  closure.category = VC_PRVALUE;
  closure.node = make_dump_node(CallSemKind::closure_object, closure_info.qualified_name);
  set_expr_metadata(closure.node, closure.type, closure.category);
  for(size_t i = 0; i < closure_info.fields.size(); ++i) {
    const FieldInfo & field_info = closure_info.fields[i];
    CppAstNode identifier;
    identifier.kind = CppAstKind::id_expression;
    identifier.value = field_info.name;
    ExprInfo capture = ctx.analyze_id_expression(scope, identifier);
    DumpNode capture_node = make_dump_node(CallSemKind::closure_capture, field_info.name);
    capture_node.semantic_type = field_info.type;
    capture_node.value_category = capture.node.value_category;
    std::map<std::string, ValueBinding>::const_iterator field =
        closure_info.member_scope->values.find(field_info.name);
    if(field != closure_info.member_scope->values.end()) {
      set_callsem_uint_value(capture_node, field->second.field_offset);
    }
    capture_node.children.push_back(std::move(capture.node));
    closure.node.children.push_back(std::move(capture_node));
  }
  return closure;
}

bool parameter_declaration_has_pack(const CppAstNode & parameter)
{
  const CppAstNode * declarator = find_child(parameter, CppAstKind::declarator);
  if(!declarator) {
    declarator = find_child(parameter, CppAstKind::abstract_declarator);
  }
  return declarator && find_child(*declarator, CppAstKind::parameter_pack);
}

void bind_lambda_parameter_pack_sizes(SemanticContext & ctx,
                                      Scope & parent_scope,
                                      const CppAstNode * declarator,
                                      Scope & lambda_scope)
{
  if(!declarator) {
    return;
  }
  const CppAstNode * parameter_clause =
      find_child_kind(*declarator, CppAstKind::parameter_clause);
  if(!parameter_clause) {
    return;
  }

  for(size_t i = 0; i < parameter_clause->children.size(); ++i) {
    const CppAstNode & parameter = parameter_clause->children[i];
    if(parameter.kind != CppAstKind::parameter_declaration ||
       !parameter_declaration_has_pack(parameter)) {
      continue;
    }

    const std::string pack_name =
        pack_parameter_analysis::parameter_declaration_name(parameter);
    if(pack_name.empty()) {
      continue;
    }

    std::size_t pack_size = 0;
    if(pack_parameter_analysis::infer_named_type_pack_size(parent_scope,
                                                           parameter,
                                                           pack_size)) {
      lambda_scope.named_pack_sizes[pack_name] = pack_size;
    }
  }
}

std::string lambda_pack_value_alias_name(const std::string & pack_name, std::size_t index)
{
  if(index == 0) {
    return pack_name;
  }
  std::ostringstream out;
  out << pack_name << "__pack" << (index + 1);
  return out.str();
}

void bind_lambda_parameter_value_packs(SemanticContext & ctx,
                                       Scope & parent_scope,
                                       const CppAstNode * declarator,
                                       Scope & lambda_scope)
{
  if(!declarator) {
    return;
  }
  const CppAstNode * parameter_clause =
      find_child_kind(*declarator, CppAstKind::parameter_clause);
  if(!parameter_clause) {
    return;
  }

  std::vector<const CppAstNode *> parameters;
  for(size_t i = 0; i < parameter_clause->children.size(); ++i) {
    if(parameter_clause->children[i].kind == CppAstKind::parameter_declaration) {
      parameters.push_back(&parameter_clause->children[i]);
    }
  }
  if(parameters.empty() || !parameter_declaration_has_pack(*parameters.back())) {
    return;
  }

  const std::string pack_name =
      pack_parameter_analysis::parameter_declaration_name(*parameters.back());
  if(pack_name.empty()) {
    return;
  }

  std::map<std::string, std::size_t>::const_iterator found_size =
      lambda_scope.named_pack_sizes.find(pack_name);
  if(found_size == lambda_scope.named_pack_sizes.end()) {
    return;
  }

  const std::size_t pack_size = found_size->second;
  const std::size_t pack_param_index = parameters.size() - 1;
  std::vector<ValueBinding> pack_bindings;
  pack_bindings.reserve(pack_size);

  for(std::size_t i = 0; i < pack_size; ++i) {
    Scope single_pack_scope(&parent_scope, "<lambda-pack-analysis>", false);
    single_pack_scope.class_info = current_class_scope(parent_scope);
    std::set<std::string> seen_pack_names;
    for(Scope * current = &parent_scope; current; current = current->parent) {
      if(current->namespace_scope || current->parent == nullptr) {
        break;
      }
      for(const auto & pack : current->named_type_packs) {
        if(!seen_pack_names.insert(pack.first).second || pack.second.size() <= i) {
          continue;
        }
        single_pack_scope.named_type_packs[pack.first] = {pack.second[i]};
        if(current->template_bound_type_pack_names.count(pack.first) != 0) {
          single_pack_scope.template_bound_type_pack_names.insert(pack.first);
        }
      }
    }

    std::vector<std::pair<std::string, TypePtr> > parsed_params;
    if(!ctx.parse_parameter_clause(single_pack_scope, *parameter_clause, parsed_params, nullptr)) {
      throw logic_error("unsupported lambda parameter pack value binding");
    }
    if(parsed_params.size() <= pack_param_index || !parsed_params[pack_param_index].second) {
      throw logic_error("lambda parameter pack value binding parse mismatch");
    }

    const std::string alias_name = lambda_pack_value_alias_name(pack_name, i);
    ValueBinding alias(ValueBinding::VK_PARAMETER,
                       alias_name,
                       parsed_params[pack_param_index].second);
    pack_bindings.push_back(alias);
  }

  semantic_scope_mutation::bind_value_pack(lambda_scope, pack_name, pack_bindings);
}

struct PreparedLambdaExpression
{
  const CppAstNode * introducer = nullptr;
  const CppAstNode * body = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * template_parameters = nullptr;
  vector<pair<string, TypePtr> > params;
  vector<const CppAstNode *> default_arguments;
  TypePtr result_type;
  bool defer_implicit_result_type = false;
  bool mutable_lambda = false;
  LambdaCaptureDefaultMode default_capture = LCD_NONE;
  vector<pair<string, bool> > captures;
  unique_ptr<DumpNode> cached_body_output;
};

bool lambda_body_output_cache_allowed(const PreparedLambdaExpression & prepared)
{
  if(prepared.default_capture != LCD_NONE) {
    return false;
  }
  if(prepared.body &&
     lambda_body_contains_local_class_declaration(*prepared.body)) {
    return false;
  }
  if(prepared.body &&
     lambda_body_contains_disallowed_cache_control_flow(*prepared.body)) {
    return false;
  }
  return prepared.captures.empty();
}

bool lambda_body_contains_local_class_declaration(const CppAstNode & node)
{
  if(node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration) {
    return true;
  }
  if(node.kind == CppAstKind::lambda_expression) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(lambda_body_contains_local_class_declaration(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool lambda_body_creates_local_type(const CppAstNode & node)
{
  if(node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration ||
     node.kind == CppAstKind::lambda_expression) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(lambda_body_creates_local_type(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool lambda_body_contains_disallowed_cache_control_flow(const CppAstNode & node)
{
  if(node.kind == CppAstKind::labeled_statement ||
     node.kind == CppAstKind::goto_statement ||
     node.kind == CppAstKind::case_statement ||
     node.kind == CppAstKind::default_statement) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(lambda_body_contains_disallowed_cache_control_flow(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool lambda_body_contains_outer_return_statement(const CppAstNode & node)
{
  if(node.kind == CppAstKind::return_statement) {
    return true;
  }
  if(node.kind == CppAstKind::lambda_expression) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(lambda_body_contains_outer_return_statement(node.children[i])) {
      return true;
    }
  }
  return false;
}

PreparedLambdaExpression prepare_lambda_expression(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const CppAstNode & node)
{
  PreparedLambdaExpression prepared;
  prepared.introducer = find_child_kind(node, CppAstKind::lambda_introducer);
  prepared.body = find_child_kind(node, CppAstKind::compound_statement);
  if(!prepared.introducer || !prepared.body) {
    throw logic_error("malformed lambda-expression");
  }
  prepared.declarator = find_child_kind(node, CppAstKind::lambda_declarator);
  prepared.template_parameters =
      prepared.declarator ?
          find_child_kind(*prepared.declarator, CppAstKind::template_parameter_clause) :
          nullptr;
  if(!parse_lambda_capture_list(prepared.introducer->value,
                                prepared.captures,
                                prepared.default_capture)) {
    throw logic_error("unsupported lambda capture list");
  }
  if(prepared.declarator && !prepared.template_parameters) {
    const CppAstNode * parameter_clause =
        find_child_kind(*prepared.declarator, CppAstKind::parameter_clause);
    if(parameter_clause &&
       !ctx.parse_parameter_clause(scope,
                                   *parameter_clause,
                                   prepared.params,
                                   &prepared.default_arguments)) {
      throw logic_error("unsupported lambda parameter-clause");
    }
  }

  Scope lambda_analysis_scope(&scope, "<lambda-analysis>", false);
  lambda_analysis_scope.class_info = current_class_scope(scope);
  std::vector<ValueBinding> parameter_bindings;
  for(size_t i = 0; i < prepared.params.size(); ++i) {
    if(prepared.params[i].first.empty()) {
      continue;
    }
    parameter_bindings.push_back(ValueBinding(ValueBinding::VK_PARAMETER,
                                              prepared.params[i].first,
                                              prepared.params[i].second));
  }
  semantic_scope_mutation::bind_values(lambda_analysis_scope, parameter_bindings);
  bind_lambda_parameter_pack_sizes(ctx, scope, prepared.declarator, lambda_analysis_scope);
  bind_lambda_parameter_value_packs(ctx, scope, prepared.declarator, lambda_analysis_scope);

  prepared.result_type = make_fundamental(FT_VOID);
  if(prepared.declarator) {
    const CppAstNode * trailing =
        find_child_kind(*prepared.declarator, CppAstKind::trailing_return_type);
    if(trailing) {
      const CppAstNode * type_id = find_child_kind(*trailing, CppAstKind::type_id);
      if(!type_id || !ctx.parse_type_id(lambda_analysis_scope, *type_id, prepared.result_type)) {
        throw logic_error("unsupported lambda trailing return type");
      }
    } else if(prepared.template_parameters) {
      // A generic lambda's parameter types are still dependent here.  Deduce
      // its implicit auto result when operator() is instantiated with concrete
      // template arguments instead of analyzing the body in an empty lambda
      // parameter scope.
      prepared.result_type = make_fundamental(FT_VOID);
    } else if(!prepared.body->children.empty()) {
      if(!lambda_body_contains_outer_return_statement(*prepared.body)) {
        prepared.result_type = make_fundamental(FT_VOID);
      } else if(lambda_body_creates_local_type(*prepared.body)) {
        // A returned lambda or local class is owned by this closure's
        // operator(), which does not exist yet.  Deduce after synthesizing the
        // closure so the local type has the same owner during deduction and
        // later body emission.
        prepared.result_type = make_fundamental(FT_VOID);
        prepared.defer_implicit_result_type = true;
      } else {
        vector<ExprInfo> return_exprs;
        bool saw_void_return = false;
        if(lambda_body_output_cache_allowed(prepared)) {
          unique_ptr<DumpNode> cached_body(
              new DumpNode(make_dump_node(CallSemKind::compound_statement)));
          semantic_statement::analyze_compound_body_and_collect_returns(ctx,
                                                                       lambda_analysis_scope,
                                                                       *prepared.body,
                                                                       *cached_body,
                                                                       return_exprs,
                                                                       saw_void_return);
          prepared.cached_body_output = std::move(cached_body);
        } else {
          semantic_statement::collect_return_expressions(ctx,
                                                         lambda_analysis_scope,
                                                         *prepared.body,
                                                         return_exprs,
                                                         saw_void_return);
        }
        if(!deduce_implicit_return_type_from_exprs(return_exprs,
                                                   saw_void_return,
                                                   prepared.result_type)) {
          throw logic_error("undeduced lambda return type");
        }
      }
    } else if(prepared.body->children.empty()) {
      prepared.result_type = make_fundamental(FT_VOID);
    }
  } else if(!prepared.body->children.empty()) {
    if(!lambda_body_contains_outer_return_statement(*prepared.body)) {
      prepared.result_type = make_fundamental(FT_VOID);
    } else if(lambda_body_creates_local_type(*prepared.body)) {
      prepared.result_type = make_fundamental(FT_VOID);
      prepared.defer_implicit_result_type = true;
    } else {
      vector<ExprInfo> return_exprs;
      bool saw_void_return = false;
      if(lambda_body_output_cache_allowed(prepared)) {
        unique_ptr<DumpNode> cached_body(
            new DumpNode(make_dump_node(CallSemKind::compound_statement)));
        semantic_statement::analyze_compound_body_and_collect_returns(ctx,
                                                                     lambda_analysis_scope,
                                                                     *prepared.body,
                                                                     *cached_body,
                                                                     return_exprs,
                                                                     saw_void_return);
        prepared.cached_body_output = std::move(cached_body);
      } else {
        semantic_statement::collect_return_expressions(ctx,
                                                       lambda_analysis_scope,
                                                       *prepared.body,
                                                       return_exprs,
                                                       saw_void_return);
      }
      if(!deduce_implicit_return_type_from_exprs(return_exprs,
                                                 saw_void_return,
                                                 prepared.result_type)) {
        throw logic_error("undeduced lambda return type");
      }
    }
  }

  if(prepared.declarator) {
    const CppAstNode * spec =
        find_child_kind(*prepared.declarator, CppAstKind::lambda_specifier);
    prepared.mutable_lambda = spec && spec->value == "mutable";
  }

  if(prepared.default_capture != LCD_NONE) {
    std::set<std::string> declared_names;
    for(size_t i = 0; i < prepared.params.size(); ++i) {
      if(!prepared.params[i].first.empty()) {
        declared_names.insert(prepared.params[i].first);
      }
    }
    if(prepared.declarator) {
      const CppAstNode * parameter_clause =
          find_child_kind(*prepared.declarator, CppAstKind::parameter_clause);
      if(parameter_clause) {
        collect_declared_names(*parameter_clause, declared_names);
      }
    }
    std::set<std::string> seen;
    for(size_t i = 0; i < prepared.captures.size(); ++i) {
      seen.insert(prepared.captures[i].first);
    }
    std::vector<std::string> implicit_names;
    collect_implicit_lambda_capture_names(ctx,
                                          scope,
                                          *prepared.body,
                                          declared_names,
                                          implicit_names,
                                          seen);
    for(size_t i = 0; i < implicit_names.size(); ++i) {
      prepared.captures.push_back(std::make_pair(implicit_names[i],
                                                 implicit_names[i] == "this" ?
                                                     false :
                                                     prepared.default_capture ==
                                                         LCD_BY_REFERENCE));
    }
  }

  return prepared;
}

ExprInfo build_lambda_closure_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         const CppAstNode & node,
                                         PreparedLambdaExpression & prepared)
{
  const CppAstNode * durable_body =
      prepared.body ? ctx.own_synthetic_ast(*prepared.body) : nullptr;
  FunctionBinding * call_operator = nullptr;
  ClassInfo * closure =
      ctx.synthesize_lambda_closure_class(scope,
                                          prepared.captures,
                                          prepared.params,
                                          prepared.default_arguments,
                                          prepared.result_type,
                                          prepared.defer_implicit_result_type,
                                          prepared.mutable_lambda,
                                          prepared.declarator,
                                          durable_body,
                                          call_operator);
  ctx.register_synthetic_lambda_closure(scope, node, *closure);
  if(call_operator) {
    if(prepared.cached_body_output) {
      call_operator->cached_body_output = std::move(prepared.cached_body_output);
    }
    ctx.register_synthetic_lambda_binding(scope, node, *call_operator);
  }
  return build_closure_object_expr(ctx, scope, *closure);
}

DumpNode make_integer_literal_node(long long value)
{
  DumpNode node = make_dump_node(CallSemKind::literal, to_string(value));
  node.semantic_type = make_fundamental(FT_INT);
  node.value_category = CVC_PRVALUE;
  if(value >= 0) {
    set_callsem_uint_value(node, static_cast<unsigned long long>(value));
  }
  return node;
}

DumpNode make_nullptr_literal_node()
{
  DumpNode node = make_dump_node(CallSemKind::literal, "nullptr");
  node.semantic_type = make_fundamental(FT_NULLPTR_T);
  node.value_category = CVC_PRVALUE;
  return node;
}

bool extract_disguised_call_callee(const CppAstNode & type_id,
                                   CppAstNode & out)
{
  out = CppAstNode();
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.empty() ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq) {
    return false;
  }
  if(type_id.children.size() > 1 &&
     (!type_id.children[1].children.empty() ||
      !type_id.children[1].value.empty())) {
    return false;
  }

  const CppAstNode & specifiers = type_id.children[0];
  const CppAstNode * type_name = nullptr;
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.kind == CppAstKind::type_name) {
      if(type_name) {
        return false;
      }
      type_name = &child;
      continue;
    }
    return false;
  }
  if(!type_name || type_name->value.empty()) {
    return false;
  }

  out = *type_name;
  out.kind = CppAstKind::id_expression;
  out.semantic_type.reset();
  out.children.clear();
  return true;
}

void collect_disguised_call_arguments(const CppAstNode & node,
                                      std::vector<CppAstNode> & out)
{
  if(node.kind == CppAstKind::binary_expression &&
     node.simple_type == OP_COMMA &&
     node.children.size() == 2) {
    collect_disguised_call_arguments(node.children[0], out);
    collect_disguised_call_arguments(node.children[1], out);
    return;
  }
  out.push_back(node);
}

bool try_analyze_disguised_parenthesized_call(SemanticContext & ctx,
                                              Scope & scope,
                                              const CppAstNode & node,
                                              ExprInfo & out)
{
  if(node.simple_type != OP_LPAREN || node.children.size() != 2) {
    return false;
  }

  CppAstNode callee;
  if(!extract_disguised_call_callee(node.children[0], callee)) {
    return false;
  }

  const CppAstNode & operand = node.children[1];
  if(operand.kind != CppAstKind::parenthesized_expression ||
     operand.children.size() != 1) {
    return false;
  }

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  collect_disguised_call_arguments(operand.children[0], arguments.children);

  CppAstNode call;
  call.kind = CppAstKind::call_expression;
  call.children.push_back(callee);
  call.children.push_back(arguments);
  out = ctx.analyze_call_expression(scope, call);
  return true;
}

bool try_analyze_disguised_parenthesized_binary_expression(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    ExprInfo & out)
{
  if(node.simple_type != OP_LPAREN || node.children.size() != 2) {
    return false;
  }

  CppAstNode lhs;
  if(!extract_disguised_call_callee(node.children[0], lhs)) {
    return false;
  }

  const CppAstNode & operand = node.children[1];
  if(operand.kind != CppAstKind::unary_expression ||
     operand.children.size() != 1 ||
     (operand.simple_type != OP_PLUS &&
      operand.simple_type != OP_MINUS &&
      operand.simple_type != OP_STAR &&
      operand.simple_type != OP_AMP)) {
    return false;
  }

  CppAstNode parenthesized;
  parenthesized.kind = CppAstKind::parenthesized_expression;
  parenthesized.token_start = node.children[0].token_start;
  parenthesized.token_end = node.children[0].token_end;
  parenthesized.children.push_back(lhs);

  CppAstNode binary;
  binary.kind = CppAstKind::binary_expression;
  binary.value = operand.value;
  binary.has_token = operand.has_token;
  binary.token_kind = operand.token_kind;
  binary.simple_type = operand.simple_type;
  binary.token_start = node.token_start;
  binary.token_end = node.token_end;
  binary.children.push_back(parenthesized);
  binary.children.push_back(operand.children[0]);
  out = ctx.analyze_expression(scope, binary);
  return true;
}

}  // namespace

ExprInfo make_implicit_member_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         const MemberValueLookupResult & member)
{
  return make_implicit_member_expression_impl(ctx, scope, member);
}

bool qualify_class_allocation_function(CppAstNode & callee,
                                       ClassInfo & allocation_class,
                                       const string & allocation_name,
                                       const CppAstNode & source)
{
  if(!allocation_class.member_scope) {
    return false;
  }
  MemberFunctionLookupResult class_allocation =
      lookup_visible_member_functions(allocation_class, allocation_name);
  if(class_allocation.functions.empty()) {
    return false;
  }

  QualifiedName qualified_allocation =
      scope_qualified_name_syntax(*allocation_class.member_scope,
                                  allocation_name);
  set_cppast_qualified_name_syntax(callee, qualified_allocation);
  callee.value = allocation_class.qualified_name + "::" + allocation_name;
  if(qualified_allocation.qualifiers.empty()) {
    return true;
  }

  vector<CppAstNode> qualifier_types(qualified_allocation.qualifiers.size());
  size_t qualifier_index = qualified_allocation.qualifiers.size();
  for(const Scope * current = allocation_class.member_scope.get();
      current && qualifier_index != 0;
      current = current->parent) {
    const bool named_class_scope =
        current->class_info &&
        current->class_info->member_scope.get() == current &&
        current->name != "<unnamed>";
    if(!current->namespace_scope && !named_class_scope) {
      continue;
    }
    --qualifier_index;
    if(!named_class_scope) {
      continue;
    }
    CppAstNode & class_qualifier = qualifier_types[qualifier_index];
    class_qualifier.kind = CppAstKind::type_name;
    class_qualifier.value = qualified_allocation.qualifiers[qualifier_index];
    class_qualifier.semantic_type = current->class_info->type;
    class_qualifier.semantic_type_is_resolved_qualifier = true;
    class_qualifier.source_location_id = source.source_location_id;
    mutable_cppast_name_lookup_snapshot(class_qualifier) =
        cppast_name_lookup_snapshot(source);
  }
  set_cppast_qualifier_type_syntaxes(callee, std::move(qualifier_types));
  return true;
}

bool class_delete_uses_sized_deallocation(ClassInfo & allocation_class,
                                           const string & allocation_name)
{
  const MemberFunctionLookupResult class_allocation =
      lookup_visible_member_functions(allocation_class, allocation_name);
  const TypePtr size_type = make_fundamental(FT_UNSIGNED_LONG_INT);
  bool has_unsized = false;
  bool has_sized = false;
  for(size_t i = 0; i < class_allocation.functions.size(); ++i) {
    const FunctionBinding * binding = class_allocation.functions[i];
    if(!binding || binding->is_deleted) {
      continue;
    }
    if(binding->params.size() == 1) {
      has_unsized = true;
    } else if(binding->params.size() == 2 &&
              type_equals(strip_top_level_cv(binding->params[1].second),
                          size_type)) {
      has_sized = true;
    }
  }
  return !has_unsized && has_sized;
}

ExprInfo analyze_array_new_cleanup_deallocation(SemanticContext & ctx,
                                                Scope & scope,
                                                ClassInfo * allocation_class,
                                                const ExprInfo & object_ptr,
                                                const CppAstNode & source)
{
  CppAstNode call;
  call.kind = CppAstKind::call_expression;

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  callee.value = "operator delete[]";
  if(allocation_class) {
    qualify_class_allocation_function(callee,
                                      *allocation_class,
                                      "operator delete[]",
                                      source);
  }
  call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  CppAstNode placeholder;
  placeholder.kind = CppAstKind::literal;
  placeholder.value = "0";
  arguments.children.push_back(placeholder);
  call.children.push_back(arguments);

  ExprInfo deallocation_address = object_ptr;
  deallocation_address.type = make_pointer(make_fundamental(FT_VOID));
  deallocation_address.category = VC_PRVALUE;
  deallocation_address.null_pointer_constant = false;
  ctx.set_expr_info_metadata(deallocation_address,
                             deallocation_address.type,
                             deallocation_address.category);
  semantic_overload::CallAnalysisHints hints;
  hints.args.push_back(&deallocation_address);
  ExprInfo result = ctx.analyze_call_expression(
      scope,
      call,
      semantic_overload::CallAnalysisOptions(true, &hints));
  result.node.has_token = true;
  result.node.token_type = KW_DELETE;
  return result;
}

ExprInfo analyze_new_expression(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & node)
{
  DIAG_CONTEXT("analyze_new_expression [" + node_text(node) + "]" +
               ctx.source_location_for_node(node));

  const CppAstNode * type_id = find_child(node, CppAstKind::type_id);
  if(!type_id) {
    throw logic_error("new-expression missing type-id");
  }

  const bool implied_empty_initializer =
      find_child(node, CppAstKind::initializer) == nullptr &&
      new_type_id_implies_empty_initializer(*type_id);
  CppAstNode adjusted_type_id =
      implied_empty_initializer ? strip_new_type_id_empty_initializer(*type_id) : *type_id;

  TypePtr allocated_type;
  if(!ctx.parse_type_id(scope, adjusted_type_id, allocated_type)) {
    throw logic_error("unsupported new-expression type-id");
  }
  if(!allocated_type) {
    throw logic_error("new-expression null allocated type");
  }

  TypePtr allocated_base = strip_top_level_cv(allocated_type);
  if(!allocated_base) {
    allocated_base = allocated_type;
  }

  const bool is_array_new = allocated_base->kind == Type::TK_ARRAY;
  TypePtr allocated_object_type = is_array_new ? allocated_base->inner : allocated_base;
  if(!allocated_object_type) {
    throw logic_error("new-expression null allocated object type");
  }
  if(allocated_object_type->kind == Type::TK_FUNCTION) {
    throw logic_error("function new-expression unsupported");
  }
  maybe_complete_layout_type(ctx, allocated_object_type);
  TypePtr array_lifecycle_element_type = allocated_object_type;
  while(array_lifecycle_element_type) {
    TypePtr element_base = strip_top_level_cv(array_lifecycle_element_type);
    if(!element_base || element_base->kind != Type::TK_ARRAY) {
      break;
    }
    array_lifecycle_element_type = element_base->inner;
  }
  ClassInfo * array_element_class =
      is_array_new ? ctx.complete_class_type(array_lifecycle_element_type) : nullptr;
  ClassInfo * allocation_class =
      is_array_new ? array_element_class : ctx.complete_class_type(allocated_object_type);
  if(allocation_class &&
     semantic_class_model::class_info_is_abstract(*allocation_class)) {
    throw NoViableConstructorError(
        "cannot allocate an object of abstract class type");
  }
  const size_t array_cookie_size =
      array_element_class ? class_array_new_cookie_size(allocated_object_type) : 0;

  CppAstNode allocation_call;
  allocation_call.kind = CppAstKind::call_expression;
  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  const string allocation_name = is_array_new ? "operator new[]" : "operator new";
  callee.value = allocation_name;
  const bool globally_qualified_new =
      find_child(node, CppAstKind::global_scope) != nullptr;
  if(!globally_qualified_new && allocation_class) {
    qualify_class_allocation_function(callee,
                                      *allocation_class,
                                      allocation_name,
                                      adjusted_type_id);
  }
  allocation_call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  CppAstNode size_expr;
  size_t array_element_size = 0;
  if(is_array_new) {
    array_element_size = type_size(allocated_object_type);
    if(allocated_base->has_bound) {
      size_expr.kind = CppAstKind::literal;
      size_expr.value =
          to_string(allocated_base->bound * array_element_size + array_cookie_size);
    } else {
      if(const CppAstNode * bound_expr = find_new_array_bound_expression(adjusted_type_id)) {
        size_expr = build_new_array_size_expression(*bound_expr,
                                                    array_element_size,
                                                    array_cookie_size);
      } else {
        throw logic_error("array new-expression requires a bound");
      }
    }
  } else {
    size_expr.kind = CppAstKind::literal;
    size_expr.value = to_string(type_size(allocated_type));
  }
  arguments.children.push_back(size_expr);

  if(const CppAstNode * placement = find_child(node, CppAstKind::placement)) {
    vector<CppAstNode> placement_args;
    if(!parse_new_placement_argument_nodes(*placement, placement_args)) {
      throw logic_error("unsupported new-expression placement");
    }
    arguments.children.insert(arguments.children.end(), placement_args.begin(), placement_args.end());
  }
  allocation_call.children.push_back(arguments);

  ExprInfo allocation = ctx.analyze_call_expression(scope, allocation_call);

  TypePtr result_type = make_pointer(allocated_object_type);
  TypePtr object_ptr_type = make_pointer(allocated_object_type);
  TypePtr allocation_base = strip_top_level_cv(remove_reference_type(allocation.type));
  if(!allocation_base || allocation_base->kind != Type::TK_POINTER) {
    throw logic_error("new-expression allocation result is not a pointer");
  }
  ExprInfo object_ptr = allocation;
  object_ptr.type = object_ptr_type;
  ctx.set_expr_info_metadata(object_ptr, object_ptr.type, object_ptr.category);

  ExprInfo array_cleanup_deallocation;
  if(is_array_new && array_element_class) {
    array_cleanup_deallocation =
        analyze_array_new_cleanup_deallocation(ctx,
                                               scope,
                                               allocation_class,
                                               object_ptr,
                                               adjusted_type_id);
  }

  vector<const CppAstNode *> ctor_arg_nodes;
  const CppAstNode * initializer = find_child(node, CppAstKind::initializer);
  const bool braced_new_initializer =
      initializer && new_initializer_is_braced(*initializer);
  if(initializer) {
    ctor_arg_nodes = new_initializer_argument_nodes(*initializer);
  } else if(implied_empty_initializer) {
    ctor_arg_nodes.clear();
  }
  vector<unique_ptr<CppAstNode> > expanded_ctor_arg_storage;
  ctor_arg_nodes = expand_new_argument_nodes(ctx, scope, ctor_arg_nodes, expanded_ctor_arg_storage);

  if(is_array_new) {
    const bool has_new_initializer = find_child(node, CppAstKind::initializer) != nullptr;
    const bool empty_value_initializer =
        (has_new_initializer || implied_empty_initializer) && ctor_arg_nodes.empty();
    if((has_new_initializer || implied_empty_initializer) && !empty_value_initializer) {
      throw logic_error("array new-expression initializer unsupported");
    }
    ClassInfo * element_class = array_element_class;
    constructor_lifecycle_service::ConstructorSelectionResult array_ctor;
    bool array_constructor_required = false;
    bool class_array_zero_initializes = false;
    if(element_class) {
      constructor_lifecycle_service::select_constructor_into(
          ctx,
          scope,
          *element_class,
          ctor_arg_nodes,
          array_ctor,
          constructor_lifecycle_service::selection_options_for(
              constructor_lifecycle_service::direct_initialization_profile(
                  "array new-expression")));
      if(!array_ctor.ctor) {
        throw NoViableConstructorError("no viable constructor for array new-expression");
      }
      array_constructor_required =
          !semantic_class_model::is_trivially_default_constructible_type_for_host_abi(
              ctx,
              array_lifecycle_element_type);
      if(array_constructor_required &&
         !ctx.function_binding_is_nothrow(*array_ctor.ctor)) {
        semantic_lifetime::require_destructor_action_if_needed(
            ctx,
            array_lifecycle_element_type,
            false);
      }
      class_array_zero_initializes =
          empty_value_initializer &&
          constructor_lifecycle_service::value_initialization_requires_zero_init(
              *array_ctor.ctor);
    }
    ExprInfo result;
    result.type = result_type;
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::new_expression);
    set_expr_metadata(result.node, result.type, result.category);
    if(element_class) {
      set_callsem_uint_value(result.node, type_size(array_lifecycle_element_type));
    }
    if(element_class ? class_array_zero_initializes : empty_value_initializer) {
      result.node.value_initializes_result = true;
    }
    result.node.children.push_back(std::move(object_ptr.node));
    if(array_constructor_required) {
      constructor_lifecycle_service::ConstructorActionResult ctor_action;
      constructor_lifecycle_service::prepare_selected_constructor_action_into(
          ctx,
          object_ptr,
          array_ctor,
          false,
          OutputReason::NewExpression,
          ctor_action);
      result.node.children.push_back(make_bound_callee_node(ctx, *ctor_action.ctor));
      for(size_t i = 1; i < ctor_action.call_args.size(); ++i) {
        result.node.children.push_back(std::move(ctor_action.call_args[i].node));
      }
    }
    if(array_element_class) {
      result.node.children.push_back(std::move(array_cleanup_deallocation.node));
    }
    return result;
  }

  ClassInfo * class_info = ctx.complete_class_type(allocated_object_type);
  if(!class_info) {
    ExprInfo result;
    result.type = result_type;
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::new_expression);
    set_expr_metadata(result.node, result.type, result.category);
    result.node.children.push_back(std::move(object_ptr.node));

    if(find_child(node, CppAstKind::initializer) != nullptr || implied_empty_initializer) {
      ExprInfo init_expr;
      if(ctor_arg_nodes.empty()) {
        init_expr = make_value_initialized_expr(allocated_type);
      } else {
        if(ctor_arg_nodes.size() != 1) {
          throw logic_error("non-class new-expression supports a single initializer");
        }
        init_expr = analyze_expression_for_target(ctx, scope, *ctor_arg_nodes[0], allocated_type);
        if(!can_copy_initialize(ctx, allocated_type, init_expr)) {
          throw logic_error("invalid non-class new-expression initializer");
        }
      }
      result.node.children.push_back(std::move(init_expr.node));
    }
    return result;
  }

  constructor_lifecycle_service::ConstructorSelectionResult ctor;
  {
    ConstructorSelectionOptions ctor_options =
        constructor_lifecycle_service::selection_options_for(
            constructor_lifecycle_service::direct_initialization_profile(
                "new-expression"));
    if(braced_new_initializer &&
       semantic_class_model::can_synthesize_aggregate_constructor(*class_info)) {
      constructor_lifecycle_service::apply_selection_profile(
          ctor_options,
          constructor_lifecycle_service::aggregate_construction_profile(
              "braced new-expression"));
    }
    constructor_lifecycle_service::select_constructor_into(ctx,
                                                           scope,
                                                           *class_info,
                                                           ctor_arg_nodes,
                                                           ctor,
                                                           ctor_options);
  }
  if(!ctor.ctor) {
    throw NoViableConstructorError("no viable constructor for new-expression");
  }

  ExprInfo result;
  result.type = result_type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::new_expression);
  ctx.set_expr_info_metadata(result, result.type, result.category);
  result.node.children.push_back(std::move(object_ptr.node));
  if(constructor_lifecycle_service::selected_constructor_allows_direct_materialization(
         *class_info,
         ctor)) {
    result.node.children.push_back(std::move(ctor.converted_args[0].node));
    return result;
  }

  constructor_lifecycle_service::ConstructorActionResult ctor_action;
  constructor_lifecycle_service::prepare_selected_constructor_action_into(
      ctx,
      object_ptr,
      ctor,
      false,
      OutputReason::NewExpression,
      ctor_action);

  result.node.children.push_back(make_bound_callee_node(ctx, *ctor_action.ctor));
  for(size_t i = 1; i < ctor_action.call_args.size(); ++i) {
    result.node.children.push_back(std::move(ctor_action.call_args[i].node));
  }
  if((find_child(node, CppAstKind::initializer) != nullptr || implied_empty_initializer) &&
     ctor_arg_nodes.empty() &&
     constructor_lifecycle_service::value_initialization_requires_zero_init(
         *ctor_action.ctor)) {
    result.node.value_initializes_result = true;
  }
  return result;
}

ExprInfo analyze_delete_expression(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node)
{
  DIAG_CONTEXT("analyze_delete_expression [" + node_text(node) + "]" +
               ctx.source_location_for_node(node));

  size_t child_index = 0;
  if(child_index < node.children.size() &&
     node.children[child_index].kind == CppAstKind::global_scope) {
    ++child_index;
  }
  const bool is_array_delete =
      child_index < node.children.size() &&
      node.children[child_index].kind == CppAstKind::array_delete;
  if(is_array_delete) {
    ++child_index;
  }
  const CppAstNode * operand =
      child_index < node.children.size() ? &node.children[child_index] : nullptr;
  if(!operand) {
    throw logic_error("delete-expression missing operand");
  }

  ExprInfo pointer = ctx.analyze_expression(scope, *operand);
  TypePtr pointer_type = strip_top_level_cv(remove_reference_type(pointer.type));
  if(!pointer_type || pointer_type->kind != Type::TK_POINTER) {
    ExprInfo converted_pointer;
    TypePtr converted_pointer_type;
    if(!try_builtin_pointer_operand_conversion(ctx,
                                               scope,
                                               pointer,
                                               converted_pointer,
                                               converted_pointer_type)) {
      throw logic_error("delete-expression operand must be a pointer");
    }
    pointer = converted_pointer;
    pointer_type = converted_pointer_type;
  }
  TypePtr pointee_type = strip_top_level_cv(pointer_type->inner);
  TypePtr deletion_element_type = pointee_type;
  while(deletion_element_type) {
    TypePtr element_base = strip_top_level_cv(deletion_element_type);
    if(!element_base || element_base->kind != Type::TK_ARRAY) {
      break;
    }
    deletion_element_type = element_base->inner;
  }
  ClassInfo * pointee_class = ctx.complete_class_type(deletion_element_type);

  CppAstNode call;
  call.kind = CppAstKind::call_expression;

  CppAstNode callee;
  callee.kind = CppAstKind::id_expression;
  const string allocation_name =
      is_array_delete ? "operator delete[]" : "operator delete";
  callee.value = allocation_name;
  const bool globally_qualified_delete =
      find_child(node, CppAstKind::global_scope) != nullptr;
  const bool uses_class_deallocation =
      !globally_qualified_delete && pointee_class &&
      qualify_class_allocation_function(callee,
                                        *pointee_class,
                                        allocation_name,
                                        *operand);
  call.children.push_back(callee);

  CppAstNode arguments;
  arguments.kind = CppAstKind::paren_argument_list;
  arguments.children.push_back(*operand);
  if(!is_array_delete && uses_class_deallocation &&
     class_delete_uses_sized_deallocation(*pointee_class, allocation_name)) {
    CppAstNode size;
    size.kind = CppAstKind::literal;
    size.value = to_string(type_size(pointee_type));
    size.semantic_type = make_fundamental(FT_UNSIGNED_LONG_INT);
    arguments.children.push_back(size);
  }
  call.children.push_back(arguments);

  // The first argument passed to a usual deallocation function is the address
  // of the deleted storage, not an ordinary conversion of the source operand.
  // In particular, deleting through pointer-to-const is valid even though an
  // ordinary pointer-to-const object cannot convert to mutable void*.
  ExprInfo deallocation_address = pointer;
  deallocation_address.type = make_pointer(make_fundamental(FT_VOID));
  deallocation_address.category = VC_PRVALUE;
  deallocation_address.null_pointer_constant = false;
  ctx.set_expr_info_metadata(deallocation_address,
                             deallocation_address.type,
                             deallocation_address.category);
  semantic_overload::CallAnalysisHints call_hints;
  call_hints.args.push_back(&deallocation_address);
  ExprInfo result = ctx.analyze_call_expression(
      scope,
      call,
      semantic_overload::CallAnalysisOptions(true, &call_hints));
  if(result.node.children.size() != arguments.children.size() + 1) {
    throw logic_error("delete-expression deallocation call shape");
  }
  result.node.children[1] = std::move(pointer.node);

  if(ClassInfo * info = pointee_class) {
    if(info->complete) {
      if(is_array_delete) {
        result.node.has_token = true;
        result.node.token_type = KW_DELETE;
      } else {
        FunctionBinding * dtor = find_delete_destructor_binding(*info);
        if(dtor) {
          result.node.has_token = true;
          result.node.token_type = KW_DELETE;
          if(dtor->is_virtual && dtor->has_virtual_slot) {
            set_callsem_uint_value(result.node, dtor->virtual_slot + 1);
            result.node.uses_extended_vtable_layout =
                semantic_class_model::class_uses_extended_virtual_abi(*info);
          }
        }
      }
    }
  }

  return result;
}

ExprInfo analyze_statement_expression(SemanticContext & ctx,
                                      Scope & scope,
                                      const CppAstNode & node)
{
  if(node.children.size() != 1 ||
     node.children[0].kind != CppAstKind::compound_statement) {
    throw logic_error("statement-expression shape");
  }

  const CppAstNode & body = node.children[0];
  Scope body_scope(&scope, "<statement-expression>", false);
  DumpNode prefix = make_dump_node(CallSemKind::compound_statement);

  const CppAstNode * result_statement = nullptr;
  if(!body.children.empty()) {
    const CppAstNode & last = body.children.back();
    if(last.kind == CppAstKind::expression_statement &&
       last.children.size() == 1) {
      result_statement = &last;
    }
  }

  const size_t prefix_count =
      result_statement ? body.children.size() - 1 : body.children.size();
  TypePtr return_type = make_fundamental(FT_VOID);
  if(FunctionBinding * function = current_function_scope(scope)) {
    TypePtr function_type = strip_top_level_cv(function->declared_type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION) {
      function_type = strip_top_level_cv(function->type);
    }
    if(function_type && function_type->kind == Type::TK_FUNCTION &&
       function_type->inner) {
      return_type = function_type->inner;
    }
  }
  for(size_t i = 0; i < prefix_count; ++i) {
    semantic_statement::analyze_statement(ctx,
                                          body_scope,
                                          return_type,
                                          body.children[i],
                                          prefix);
  }

  ExprInfo result;
  if(!result_statement) {
    result.type = make_fundamental(FT_VOID);
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::statement_expression);
    result.node.children.push_back(std::move(prefix));
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  result = ctx.analyze_expression(body_scope, result_statement->children[0]);
  DumpNode statement_expr = make_dump_node(CallSemKind::statement_expression);
  statement_expr.children.push_back(std::move(prefix));
  statement_expr.children.push_back(std::move(result.node));
  result.node = std::move(statement_expr);
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo analyze_throw_expression(SemanticContext & ctx,
                                  Scope & scope,
                                  const CppAstNode & node)
{
  if(node.children.size() > 1) {
    throw logic_error("throw-expression arity");
  }

  ExprInfo result;
  result.type = make_fundamental(FT_VOID);
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::throw_statement);
  if(!node.children.empty()) {
    ExprInfo thrown = ctx.analyze_expression(scope, node.children[0]);
    thrown = adjust_exception_operand_type(ctx, thrown);
    result.node.children.push_back(std::move(thrown.node));
  }
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo analyze_expression(SemanticContext & ctx,
                            Scope & scope,
                            const CppAstNode & node)
{
  DIAG_CONTEXT("semantic_expression::analyze_expression [" + node_text(node) + "]" +
               ctx.source_location_for_node(node));
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->expression_analysis_by_demand[
        static_cast<std::size_t>(semantic_metrics::current_class_demand())];
  }
  ScopedCallSemConstructionPath construction_path(cppast_kind_text(node.kind));
  semantic_hotspot::note_expression_analysis(node, ctx.source_location_for_node(node));
  ExprInfo result;
  if(node.kind == CppAstKind::lambda_expression) {
    result = analyze_lambda_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::braced_init_list) {
    result = analyze_braced_init_list_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::literal) {
    result = analyze_literal(ctx, scope, node);
  } else if(node.kind == CppAstKind::keyword_literal && node_has_simple_type(node, KW_THIS)) {
    result = analyze_this_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::keyword_literal) {
    result = analyze_literal(ctx, scope, node);
  } else if(node.kind == CppAstKind::parenthesized_expression) {
    if(node.children.size() != 1) {
      throw logic_error("parenthesized-expression arity");
    }
    result = ctx.analyze_expression(scope, node.children[0]);
  } else if(node.kind == CppAstKind::statement_expression) {
    result = analyze_statement_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::id_expression) {
    result = analyze_id_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::call_expression) {
    result = ctx.analyze_call_expression(scope, node);
  } else if(node.kind == CppAstKind::subscript_expression) {
    result = analyze_subscript_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::member_expression) {
    result = analyze_member_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::binary_expression) {
    result = analyze_binary_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::conditional_expression) {
    result = analyze_conditional_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::unary_expression) {
    result = analyze_unary_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::postfix_expression) {
    result = analyze_postfix_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::assignment_expression) {
    result = analyze_assignment_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::throw_statement) {
    result = analyze_throw_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::sizeof_expression) {
    result = analyze_sizeof_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::sizeof_pack_expression) {
    result = analyze_sizeof_pack_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::type_trait_expression) {
    result = analyze_type_trait_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::cast_expression) {
    result = analyze_cast_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::new_expression) {
    result = analyze_new_expression(ctx, scope, node);
  } else if(node.kind == CppAstKind::delete_expression) {
    result = analyze_delete_expression(ctx, scope, node);
  } else {
    ostringstream out;
    out << "unsupported expression in PA12 first slice";
    out << " [kind " << cppast_kind_text(node.kind) << "]";
    out << " [text " << node_text(node) << "]";
    throw logic_error(out.str());
  }

  return result;
}

namespace {

ExprInfo analyze_unevaluated_sizeof_operand(SemanticContext & ctx,
                                            Scope & scope,
                                            const CppAstNode & expr_node)
{
  if(expr_node.kind == CppAstKind::parenthesized_expression &&
     expr_node.children.size() == 1) {
    return analyze_unevaluated_sizeof_operand(ctx, scope, expr_node.children[0]);
  }
  if(expr_node.kind == CppAstKind::call_expression) {
    return ctx.analyze_call_expression(scope,
                                       expr_node,
                                       semantic_policy::without_body_instantiation());
  }
  return ctx.analyze_expression(scope, expr_node);
}

bool try_analyze_recovered_sizeof_type_id_operand(SemanticContext & ctx,
                                                  Scope & scope,
                                                  const CppAstNode & type_id,
                                                  TypePtr & out)
{
  CppAstNode operand;
  if(!cppast_recover_sizeof_type_id_expression_operand(type_id, operand)) {
    return false;
  }

  const CppAstNode * owned_operand = ctx.own_synthetic_ast(std::move(operand));
  try {
    ExprInfo expr = analyze_unevaluated_sizeof_operand(ctx, scope, *owned_operand);
    out = expr.type;
    return true;
  } catch(const logic_error &) {
    return false;
  }
}

const CppAstNode * sizeof_type_id_witness_anchor(const CppAstNode & type_id)
{
  const CppAstNode * specifiers = find_child(type_id, CppAstKind::type_specifier_seq);
  if(!specifiers) {
    return nullptr;
  }
  for(size_t i = 0; i < specifiers->children.size(); ++i) {
    if(specifiers->children[i].kind == CppAstKind::type_name) {
      return &specifiers->children[i];
    }
  }
  return nullptr;
}

std::string class_template_identifier_for_witness_type(SemanticContext & ctx,
                                                       const TypePtr & type)
{
  ClassInfo * info = ctx.class_info_for_type(strip_top_level_cv(type));
  if(!info || !info->source_template) {
    return std::string();
  }
  return info->source_template->name;
}

std::size_t final_template_identifier_offset(const std::string & text,
                                             const std::string & identifier)
{
  if(text.empty() || identifier.empty()) {
    return std::string::npos;
  }
  const std::size_t search_end = text.find('<');
  std::size_t pos = search_end == std::string::npos ?
      text.rfind(identifier) :
      text.rfind(identifier, search_end);
  while(pos != std::string::npos) {
    const bool left_ok =
        pos == 0 ||
        !(std::isalnum(static_cast<unsigned char>(text[pos - 1])) ||
          text[pos - 1] == '_');
    const std::size_t after = pos + identifier.size();
    const bool right_ok =
        after >= text.size() ||
        !(std::isalnum(static_cast<unsigned char>(text[after])) ||
          text[after] == '_');
    if(left_ok && right_ok) {
      return pos;
    }
    if(pos == 0) {
      break;
    }
    pos = text.rfind(identifier, pos - 1);
  }
  return std::string::npos;
}

std::string source_location_with_column_offset(const std::string & location,
                                               std::size_t offset)
{
  if(offset == 0 || location.empty()) {
    return location;
  }
  const std::string normalized =
      template_api::normalize_template_witness_source_location(location);
  const template_api::template_witness_detail::ParsedSourceLocation parsed =
      template_api::template_witness_detail::parse_source_location(normalized);
  if(!parsed.valid || parsed.column <= 0) {
    return location;
  }
  std::ostringstream out;
  out << " at " << parsed.file << ":" << parsed.line << ":"
      << (parsed.column + static_cast<int>(offset));
  return out.str();
}

void record_sizeof_type_id_class_use_if_needed(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & type_id,
                                               const TypePtr & type)
{
  if(!witness::source_capture_enabled(ctx.template_witness_context())) {
    return;
  }

  const CppAstNode * anchor = sizeof_type_id_witness_anchor(type_id);
  if(!anchor) {
    anchor = &type_id;
  } else {
    CppAstNode witness_anchor = *anchor;
    witness_anchor.template_id_syntax.reset();
    witness_anchor.qualifier_template_id_syntaxes.clear();
    const CppAstNode * owned_anchor =
        ctx.own_synthetic_ast(std::move(witness_anchor));
    if(owned_anchor) {
      anchor = owned_anchor;
    }
  }

  std::string anchor_location = ctx.source_location_for_node(*anchor);
  const std::string template_identifier =
      class_template_identifier_for_witness_type(ctx, type);
  if(!template_identifier.empty()) {
    const std::string template_name_location =
        ctx.source_location_for_name_in_node(*anchor, template_identifier);
    if(!template_name_location.empty()) {
      anchor_location = template_name_location;
    } else {
      const std::size_t offset =
          final_template_identifier_offset(anchor->value,
                                           template_identifier);
      if(offset != std::string::npos) {
        anchor_location =
            source_location_with_column_offset(anchor_location, offset);
      }
    }
  }

  ctx.record_class_use_for_resolved_type_node(scope,
                                              *anchor,
                                              type,
                                              anchor_location);
}

ExprInfo make_static_member_variable_expr(SemanticContext & ctx,
                                          const ValueBinding & binding,
                                          bool allow_constant_fold = true);

bool pure_constant_only_value_binding(const ValueBinding & binding)
{
  return binding.kind == ValueBinding::VK_VARIABLE &&
         binding.has_constant_value &&
         !binding.declaration_node &&
         !binding.owner_class;
}

bool id_expression_binding_allows_constant_fold(const ValueBinding & binding)
{
  return pure_constant_only_value_binding(binding) ||
         (binding.kind == ValueBinding::VK_VARIABLE && binding.owner_class);
}

bool value_binding_is_enumerator(const ValueBinding & binding)
{
  return binding.kind == ValueBinding::VK_VARIABLE &&
         binding.declaration_node &&
         binding.declaration_node->kind == CppAstKind::enumerator;
}

ExprInfo make_enumerator_value_expr(const CppAstNode & node,
                                    const ValueBinding & binding)
{
  ExprInfo result;
  result.type = strip_top_level_cv(binding.type);
  result.category = VC_PRVALUE;
  if(value_binding_has_constexpr_value(binding)) {
    const constant_eval::ConstexprValue & value =
        value_binding_constexpr_value(binding);
    TypePtr enum_base = strip_top_level_cv(binding.type);
    TypePtr underlying =
        enum_base && enum_base->kind == Type::TK_NAMED ?
            strip_top_level_cv(enum_base->named_enum_underlying_type) :
            TypePtr();
    unsigned long long unsigned_value = 0;
    long long signed_value = 0;
    if(underlying &&
       is_unsigned_integral_type(underlying) &&
       constant_eval::constexpr_value_to_unsigned_integral(
           value, unsigned_value)) {
      result.node =
          make_dump_node(CallSemKind::literal, to_string(unsigned_value));
      set_callsem_uint_value(result.node, unsigned_value);
    } else if(constant_eval::constexpr_value_to_integral(value, signed_value)) {
      result.node =
          make_dump_node(CallSemKind::literal, to_string(signed_value));
      set_callsem_int_value(result.node, signed_value);
    } else {
      result.node = make_dump_node(CallSemKind::id_expression, node.value);
    }
  } else if(binding.has_constant_value) {
    result.node = make_dump_node(CallSemKind::literal,
                                 to_string(binding.constant_value));
    set_callsem_int_value(result.node, binding.constant_value);
  } else {
    result.node = make_dump_node(CallSemKind::id_expression, node.value);
  }
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

bool storage_backed_primary_template_static_member(const ValueBinding & binding)
{
  return binding.kind == ValueBinding::VK_VARIABLE &&
         binding.owner_class &&
         binding.owner_class->source_template &&
         binding.has_storage_definition &&
         !binding.is_explicit_specialization &&
         !binding.owner_class->is_explicit_specialization &&
         binding.owner_class->source_template->explicit_static_member_specializations.count(
             binding.name) != 0 &&
         template_api::class_has_template_identity(binding.owner_class);
}

bool can_inline_constexpr_static_member_initializer_without_storage(
    const ValueBinding & binding)
{
  if(!binding.requires_constant_initializer ||
     !binding.constant_initializer ||
     !binding.constant_initializer_scope) {
    return false;
  }
  const TypePtr value_type = strip_top_level_cv(binding.type);
  return is_int128_integral_type(value_type) || is_pointer_type(value_type);
}

const CppAstNode & static_member_initializer_payload(const CppAstNode & node)
{
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return node.children[0];
  }
  return node;
}

bool qualifier_template_id_needs_constant_member_shortcut(
    const TemplateIdSyntax & template_id)
{
  for(size_t i = 0; i < template_id.argument_syntaxes.size(); ++i) {
    if(template_id.argument_syntaxes[i].expression) {
      return true;
    }
  }
  return false;
}

string qualifier_name_text(const QualifiedName & qualified_name)
{
  string out;
  if(qualified_name.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified_name.qualifiers.size(); ++i) {
    if(i != 0) {
      out += "::";
    }
    out += qualified_name.qualifiers[i];
  }
  return out;
}

string qualified_name_text(const QualifiedName & qualified_name)
{
  string out = qualifier_name_text(qualified_name);
  const string name = !qualified_name.name.empty() ?
      qualified_name.name :
      string();
  if(name.empty()) {
    return out;
  }
  if(!out.empty() && out != "::") {
    out += "::";
  }
  out += name;
  return out;
}

string qualified_template_use_location(SemanticContext & ctx,
                                       const CppAstNode & node,
                                       const QualifiedName & qualified)
{
  string qualifier;
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(const TemplateIdSyntax * syntax =
           cppast_qualifier_template_id_syntax(node, i)) {
      const string candidate =
          semantic_utils::unqualified_member_name(syntax->name.name);
      if(!candidate.empty()) {
        qualifier = candidate;
      }
      continue;
    }
    if(qualified.qualifiers[i].find('<') == string::npos) {
      continue;
    }
    string candidate =
        semantic_utils::strip_trailing_top_level_template_arguments(
            qualified.qualifiers[i]);
    candidate = semantic_utils::unqualified_member_name(candidate);
    if(!candidate.empty()) {
      qualifier = candidate;
    }
  }
  if(qualifier.empty()) {
    return string();
  }

  string location = ctx.source_location_for_name_in_node(node, qualifier);
  if(!semantic_trace::source_location_points_at_identifier(location, qualifier)) {
    location.clear();
  }
  if(location.empty()) {
    location = ctx.source_location_for_node(node);
    if(!semantic_trace::source_location_points_at_identifier(location, qualifier)) {
      location.clear();
    }
  }
  return template_api::normalize_template_witness_source_location(location);
}

void emit_structured_qualified_value_class_use(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & node,
                                               const QualifiedName & qualified,
                                               const ValueBinding & binding)
{
  if(!(binding.owner_class ||
       (binding.declaration_scope && binding.declaration_scope->class_info))) {
    return;
  }
  const string qualifier_name = qualifier_name_text(qualified);
  if(qualifier_name.empty()) {
    return;
  }
  const CppAstNode qualifier_node =
      make_value_qualifier_type_lookup_node(node, qualified, qualifier_name);
  TypePtr qualifier_type =
      ctx.lookup_type_node(scope, qualifier_node, qualifier_name, false);
  if(qualifier_type) {
    ctx.record_class_use_for_resolved_type_node(
        scope,
        qualifier_node,
        qualifier_type,
        ctx.source_location_for_node(qualifier_node));
  }
}

const ValueBinding * lookup_id_expression_value_binding(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const CppAstNode & node,
                                                        bool & allow_constant_fold)
{
  allow_constant_fold = false;
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  const string qualified_use_location =
      qualified ? qualified_template_use_location(ctx, node, *qualified) :
                  string();
  const ScopedTemplateUseLocation use_location_guard(qualified_use_location);
  const parser_trace::ScopedOrderUseLocation order_use_location_guard(
      qualified_use_location);

  const bool structured_qualified_lookup =
      qualified &&
      (qualified->rooted || !qualified->qualifiers.empty()) &&
      (cppast_has_qualifier_template_id_syntaxes(node) ||
       !node.qualifier_type_syntaxes.empty());
  const ValueBinding * binding =
      structured_qualified_lookup ? nullptr :
                                    ctx.lookup_value_node(scope, node, node.value);
  if(binding) {
    allow_constant_fold = id_expression_binding_allows_constant_fold(*binding);
    return binding;
  }

  if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
    return nullptr;
  }

  if(structured_qualified_lookup) {
    TypePtr structured_qualifier_type;
    const ValueBinding * structured_binding =
        lookup_qualified_value_binding_node(ctx,
                                            scope,
                                            *qualified,
                                            node,
                                            &structured_qualifier_type);
    if(structured_binding) {
      emit_structured_qualified_value_class_use(ctx,
                                                scope,
                                                node,
                                                *qualified,
                                                *structured_binding);
      const bool concrete_structured_qualifier =
          structured_qualifier_type &&
          !ctx.type_depends_on_template_parameter(structured_qualifier_type);
      allow_constant_fold =
          (node.qualifier_type_syntaxes.empty() ||
           concrete_structured_qualifier) &&
          id_expression_binding_allows_constant_fold(*structured_binding);
      return structured_binding;
    }
    return nullptr;
  }

  if(cppast_has_qualifier_template_id_syntaxes(node)) {
    Scope * qualifier_scope =
        ctx.resolve_qualified_scope_for_node(scope, *qualified, node, false);
    if(qualifier_scope && qualifier_scope->class_info) {
      ClassInfo * qualifier_info = qualifier_scope->class_info;
      if(!qualifier_info->complete && qualifier_info->type) {
        if(ClassInfo * completed = ctx.complete_class_type(qualifier_info->type)) {
          qualifier_info = completed;
        }
      }
      if(qualifier_info->member_scope) {
        const bool qualifier_scope_can_fold =
            !ctx.scope_has_template_placeholders(*qualifier_info->member_scope);
        map<string, ValueBinding>::const_iterator found =
            qualifier_info->member_scope->values.find(qualified->name);
        if(found != qualifier_info->member_scope->values.end()) {
          allow_constant_fold =
              qualifier_scope_can_fold &&
              id_expression_binding_allows_constant_fold(found->second);
          return &found->second;
        }
        MemberValueLookupResult member =
            lookup_member_value(*qualifier_info, qualified->name);
        if(member.binding && member.binding->kind != ValueBinding::VK_FIELD) {
          allow_constant_fold =
              qualifier_scope_can_fold &&
              id_expression_binding_allows_constant_fold(*member.binding);
          return member.binding;
        }
        return nullptr;
      }
    }
  }

  const ValueBinding * qualified_binding =
      lookup_qualified_value_binding_node(ctx, scope, *qualified, node);
  if(qualified_binding) {
    allow_constant_fold =
        id_expression_binding_allows_constant_fold(*qualified_binding);
    return qualified_binding;
  }

  Scope * target = resolve_qualified_scope_for_class_or_namespace(ctx, scope, *qualified);
  if(!target || !target->class_info ||
     !ctx.scope_has_template_placeholders(*target->class_info->member_scope)) {
    return nullptr;
  }

  const string qualifier_name = qualifier_name_text(*qualified);
  if(!qualifier_name.empty()) {
    const CppAstNode qualifier_node =
        make_value_qualifier_type_lookup_node(node, *qualified, qualifier_name);
    TypePtr qualifier_type =
        ctx.lookup_type_node(scope, qualifier_node, qualifier_name, false);
    ClassInfo * completed = qualifier_type ? ctx.complete_class_type(qualifier_type) : nullptr;
    if(completed && !ctx.scope_has_template_placeholders(*completed->member_scope)) {
      target = completed->member_scope.get();
    }
  }

  const bool target_scope_can_fold =
      target->class_info &&
      !ctx.scope_has_template_placeholders(*target->class_info->member_scope);
  map<string, ValueBinding>::const_iterator found = target->values.find(qualified->name);
  if(found != target->values.end()) {
    allow_constant_fold =
        target_scope_can_fold &&
        id_expression_binding_allows_constant_fold(found->second);
    return &found->second;
  }

  MemberValueLookupResult member = lookup_member_value(*target->class_info, qualified->name);
  if(member.binding && member.binding->kind != ValueBinding::VK_FIELD) {
    allow_constant_fold =
        target_scope_can_fold &&
        id_expression_binding_allows_constant_fold(*member.binding);
    return member.binding;
  }

  return nullptr;
}

TypePtr lookup_id_expression_type_name(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  string lookup_name;
  if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    lookup_name = qualified_name_text(*qualified);
  }
  if(lookup_name.empty()) {
    lookup_name = node.value;
  }
  return ctx.lookup_type_node(scope, node, lookup_name);
}

bool try_analyze_dependent_qualified_id_expression(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const CppAstNode & node,
                                                   const QualifiedName & qualified,
                                                   ExprInfo & out)
{
  if(!cppast_has_qualifier_template_id_syntaxes(node) &&
     node.qualifier_type_syntaxes.empty()) {
    return false;
  }

  const string qualifier_name = qualifier_name_text(qualified);
  if(qualifier_name.empty()) {
    return false;
  }

  const CppAstNode qualifier_node =
      make_value_qualifier_type_lookup_node(node, qualified, qualifier_name);
  TypePtr qualifier_type =
      ctx.lookup_type_node(scope, qualifier_node, qualifier_name, false);
  if(!qualifier_type || !ctx.type_depends_on_template_parameter(qualifier_type)) {
    return false;
  }

  const string display_name = qualified_name_text(qualified);
  out.type = make_named(display_name,
                        "dependent type " + display_name,
                        true);
  out.category = VC_LVALUE;
  out.node = make_dump_node(CallSemKind::id_expression, display_name);
  set_dump_token(out.node, node);
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

bool try_resolve_typeid_type_operand(SemanticContext & ctx,
                                     Scope & scope,
                                     const CppAstNode & operand,
                                     TypePtr & out)
{
  out.reset();
  if(operand.kind == CppAstKind::type_id) {
    return ctx.parse_type_id(scope, operand, out, true) && out != nullptr;
  }

  if(operand.kind != CppAstKind::id_expression) {
    return false;
  }

  bool allow_constant_fold = true;
  if(lookup_id_expression_value_binding(ctx, scope, operand, allow_constant_fold)) {
    return false;
  }

  out = lookup_id_expression_type_name(ctx, scope, operand);
  return out != nullptr;
}

void require_complete_typeid_class_operand(SemanticContext & ctx,
                                           const TypePtr & type)
{
  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  base = strip_top_level_cv(base);
  if(!base || ctx.type_depends_on_template_parameter(base)) {
    return;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(!info || info->class_kind == "enum") {
    return;
  }
  if(info->complete) {
    return;
  }
  if(!ctx.complete_class_type(base)) {
    throw logic_error("typeid requires complete class type " + describe_type(base));
  }
}

TypePtr canonical_typeid_operand_type(const TypePtr & type)
{
  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  TypePtr unqualified = strip_top_level_cv(base);
  return unqualified ? unqualified : base;
}

ExprInfo make_value_binding_expr(SemanticContext & ctx,
                                 Scope & scope,
                                 const CppAstNode & node,
                                 const ValueBinding & binding,
                                 bool allow_constant_fold)
{
  if(value_binding_is_enumerator(binding)) {
    return make_enumerator_value_expr(node, binding);
  }
  if(binding.kind == ValueBinding::VK_VARIABLE && binding.owner_class) {
    return make_static_member_variable_expr(ctx, binding, allow_constant_fold);
  }
  if(binding.kind == ValueBinding::VK_FIELD) {
    MemberValueLookupResult member;
    if((!lookup_member_value_in_scope_chain(scope, node.value, member) || !member.binding) &&
       !member_value_lookup_result_for_binding(scope, binding, member)) {
      throw logic_error("failed to resolve member id-expression");
    }
    if(unevaluated_operand_active() &&
       !current_function_scope(scope)) {
      const ClassInfo * current_class = current_class_scope(scope);
      if(!binding.owner_class || !current_class ||
         !member_access_allowed(&scope,
                                current_class,
                                nullptr,
                                member.declared_in,
                                binding.access,
                                member.path_access)) {
        throw logic_error("inaccessible member");
      }
      ExprInfo result;
      result.type = binding.type;
      result.category = VC_LVALUE;
      result.node =
          make_dump_node(CallSemKind::id_expression, binding.name);
      set_dump_token(result.node, node);
      set_expr_metadata(result.node, result.type, result.category);
      return result;
    }
    return make_implicit_member_expression_impl(ctx, scope, member);
  }
  if(!binding.anonymous_storage_variable_name.empty()) {
    const ValueBinding * storage =
        ctx.lookup_value(scope, binding.anonymous_storage_variable_name);
    if(!storage || storage->kind != ValueBinding::VK_VARIABLE) {
      throw logic_error("failed to resolve anonymous union storage");
    }

    ExprInfo base;
    base.type = storage->type;
    base.category = VC_LVALUE;
    base.node = make_dump_node(CallSemKind::id_expression, storage->name);
    set_dump_symbol(base.node, storage->symbol);
    set_expr_metadata(base.node, base.type, base.category);

    if(binding.is_bit_field) {
      ValueBinding storage_member = binding;
      storage_member.field_offset = binding.anonymous_storage_member_offset;
      return make_bit_field_storage_expr(ctx, base, storage_member, 0);
    }

    ExprInfo result;
    result.type = binding.type;
    result.category = VC_LVALUE;
    result.node = make_dump_node(CallSemKind::member_expression, binding.name);
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, binding.anonymous_storage_member_offset);
    result.node.is_reference_storage = is_reference_type(binding.type);
    result.node.children.push_back(std::move(base.node));
    return result;
  }
  constant_eval::ConstexprValue constant_value;
  FoldedIntegralLiteral folded_literal;
  const TemplateIdSyntax * folded_template_id = cppast_template_id_syntax(node);
  const TemplateIdSyntax * folded_qualifier_template_id =
      node.qualifier_template_id_syntaxes.empty() ?
          nullptr :
          &node.qualifier_template_id_syntaxes.back();
  const auto lookup_folded_constant_value = [&]() -> bool
  {
    if(folded_qualifier_template_id &&
       ctx.lookup_constant_value_node(scope, node.value, &node, constant_value)) {
      return true;
    }
    if(folded_template_id &&
       node.qualifier_template_id_syntaxes.empty() &&
       node.qualifier_type_syntaxes.empty()) {
      return ctx.lookup_constant_template_id_value(scope,
                                                   *folded_template_id,
                                                   node.value,
                                                   constant_value);
    }
    if(folded_qualifier_template_id) {
      std::string qualifier_use_location =
          ctx.source_location_for_name_in_node(
              node,
              folded_qualifier_template_id->name.name,
              false);
      if(qualifier_use_location.empty()) {
        qualifier_use_location = ctx.source_location_for_node(node);
      }
      const ScopedTemplateUseLocation use_location(qualifier_use_location);
      const parser_trace::ScopedOrderUseLocation order_use_location(
          qualifier_use_location);
      if(ctx.lookup_constant_template_member_value(scope,
                                                   *folded_qualifier_template_id,
                                                   binding.name,
                                                   node.value,
                                                   constant_value)) {
        return true;
      }
    }
    return ctx.lookup_constant_value_node(scope, node.value, &node, constant_value);
  };
  if(allow_constant_fold &&
     (is_integral_type(binding.type) ||
      is_named_enum_type(ctx, strip_top_level_cv(binding.type))) &&
     lookup_folded_constant_value() &&
     constexpr_value_to_literal_value(ctx,
                                      constant_value,
                                      strip_top_level_cv(binding.type),
                                      folded_literal)) {
    ExprInfo result;
    result.type = strip_top_level_cv(binding.type);
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::literal, folded_literal.text);
    set_expr_metadata(result.node, result.type, result.category);
    if(folded_literal.has_int_value) {
      set_callsem_int_value(result.node, folded_literal.int_value);
    }
    if(folded_literal.has_uint_value) {
      set_callsem_uint_value(result.node, folded_literal.uint_value);
    }
    return result;
  }
  TypePtr constant_binding_type =
      strip_top_level_cv(remove_reference_type(binding.type));
  if(allow_constant_fold &&
     binding.has_constant_value &&
     constant_binding_type &&
     constant_binding_type->kind == Type::TK_MEMBER_POINTER &&
     !is_function_type(constant_binding_type->inner)) {
    ExprInfo result;
    result.type = binding.type;
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::literal,
                                 to_string(binding.constant_value));
    set_callsem_int_value(result.node, binding.constant_value);
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }
  ExprInfo result;
  TypePtr binding_type = strip_top_level_cv(binding.type);
  if(binding_type &&
     (binding_type->kind == Type::TK_LVALUE_REFERENCE ||
      binding_type->kind == Type::TK_RVALUE_REFERENCE)) {
    result.type = binding_type->inner;
  } else {
    result.type = binding.type;
  }
  result.category = VC_LVALUE;
  const string output_name =
      binding.kind == ValueBinding::VK_PARAMETER ? binding.name : node.value;
  result.node = make_dump_node(CallSemKind::id_expression, output_name);
  if(binding.kind == ValueBinding::VK_VARIABLE) {
    symbol_linkage::SymbolIdentity symbol = binding.symbol;
    if(symbol.internal_symbol.empty() &&
       binding.owner_class &&
       binding.owner_class->member_scope) {
      symbol = expression_static_member_variable_symbol_identity(*binding.owner_class,
                                                                 binding);
    }
    set_dump_symbol(result.node, symbol);
    result.node.is_thread_local = binding.is_thread_local;
  }
  if(binding.has_constant_value &&
     constant_binding_type &&
     (is_integral_type(constant_binding_type) ||
      is_named_enum_type(ctx, constant_binding_type))) {
    set_callsem_int_value(result.node, binding.constant_value);
  }
  result.node.implicit_return_move_eligible =
      binding_supports_implicit_return_move(binding);
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

bool try_analyze_non_type_template_object_pointer_value(
    SemanticContext & ctx,
    Scope & scope,
    const ValueBinding & binding,
    ExprInfo & out)
{
  const ValueBinding * target_binding = binding.non_type_template_value_binding;
  TypePtr binding_base = strip_top_level_cv(remove_reference_type(binding.type));
  if(!target_binding ||
     !binding_base ||
     binding_base->kind != Type::TK_POINTER ||
     !binding_base->inner ||
     strip_top_level_cv(binding_base->inner)->kind == Type::TK_FUNCTION ||
     target_binding->kind != ValueBinding::VK_VARIABLE ||
     !target_binding->type ||
     target_binding->is_bit_field) {
    return false;
  }

  TypePtr target_object_type = remove_reference_type(target_binding->type);
  TypePtr target_object_base = strip_top_level_cv(target_object_type);
  const bool array_decay_binding =
      target_object_base &&
      target_object_base->kind == Type::TK_ARRAY &&
      target_object_base->inner &&
      same_type_with_compatible_top_cv(binding_base->inner,
                                       target_object_base->inner);
  if(!array_decay_binding &&
     !same_type_with_compatible_top_cv(binding_base->inner,
                                       target_object_type)) {
    return false;
  }

  CppAstNode target_node;
  target_node.kind = CppAstKind::id_expression;
  target_node.value = target_binding->name;
  ExprInfo target = make_value_binding_expr(ctx,
                                            scope,
                                            target_node,
                                            *target_binding,
                                            false);
  if(target.category != VC_LVALUE) {
    return false;
  }

  symbol_linkage::SymbolIdentity symbol = target_binding->symbol;
  if(symbol.internal_symbol.empty() &&
     target_binding->owner_class &&
     target_binding->owner_class->member_scope) {
    symbol = expression_static_member_variable_symbol_identity(
        *target_binding->owner_class,
        *target_binding);
  }

  out.type = binding.type;
  out.category = VC_PRVALUE;
  out.node = make_dump_node(CallSemKind::unary_expression, "&");
  out.node.has_token = true;
  out.node.token_type = OP_AMP;
  set_dump_symbol(out.node, symbol);
  out.node.children.push_back(std::move(target.node));
  set_expr_metadata(out.node, out.type, out.category);
  return true;
}

ExprInfo make_static_member_variable_expr(SemanticContext & ctx,
                                          const ValueBinding & binding,
                                          bool allow_constant_fold)
{
  FoldedIntegralLiteral folded_literal;
  const bool force_storage_load =
      storage_backed_primary_template_static_member(binding);
  const bool constant_foldable_static_member =
      binding.requires_constant_initializer ||
      is_const_object_type(remove_reference_type(binding.type));
  if(allow_constant_fold &&
     !force_storage_load &&
     can_inline_constexpr_static_member_initializer_without_storage(binding)) {
    const CppAstNode & payload =
        static_member_initializer_payload(*binding.constant_initializer);
    return ctx.analyze_expression_for_target(*binding.constant_initializer_scope,
                                             payload,
                                             strip_top_level_cv(binding.type));
  }
  if(allow_constant_fold &&
     constant_foldable_static_member &&
     (is_integral_type(binding.type) ||
      is_floating_type(strip_top_level_cv(binding.type)) ||
      is_named_enum_type(ctx, strip_top_level_cv(binding.type)))) {
    bool can_fold = false;
    const TypePtr binding_fold_type = strip_top_level_cv(binding.type);
    const bool binding_fold_type_integral_like =
        is_integral_type(binding_fold_type) ||
        is_named_enum_type(ctx, binding_fold_type);
    const bool narrow_constant_value_allowed =
        !is_int128_integral_type(binding.type);
    if(!force_storage_load &&
       narrow_constant_value_allowed &&
       binding_fold_type_integral_like &&
       binding.has_constant_value) {
      folded_literal.text = to_string(binding.constant_value);
      folded_literal.has_int_value = true;
      folded_literal.int_value = binding.constant_value;
      can_fold = true;
    } else if(!force_storage_load &&
              value_binding_has_constexpr_value(binding)) {
      can_fold = constexpr_value_to_literal_value(ctx,
                                                  value_binding_constexpr_value(
                                                      binding),
                                                  binding.type,
                                                  folded_literal);
    } else if(!force_storage_load &&
              binding.constant_initializer &&
              binding.constant_initializer_scope) {
      constant_eval::ConstexprValue constexpr_value;
      if(ctx.evaluate_initializer_constant_value(*binding.constant_initializer_scope,
                                                 *binding.constant_initializer,
                                                 binding.type,
                                                 constexpr_value)) {
        can_fold = constexpr_value_to_literal_value(ctx,
                                                    constexpr_value,
                                                    binding.type,
                                                    folded_literal);
      }
    } else if(!force_storage_load &&
              binding.owner_class &&
              binding.owner_class->member_scope) {
      constant_eval::ConstexprValue constexpr_value;
      if(ctx.lookup_constant_value(*binding.owner_class->member_scope,
                                   binding.name,
                                   constexpr_value)) {
        can_fold = constexpr_value_to_literal_value(ctx,
                                                    constexpr_value,
                                                    binding.type,
                                                    folded_literal);
      }
    }
    if(can_fold) {
      if(witness::source_capture_enabled(ctx.template_witness_context())) {
        template_api::note_template_member_value_instantiation_if_needed(
            ctx,
            binding);
      }
      ExprInfo result;
      result.type = strip_top_level_cv(binding.type);
      result.category = VC_PRVALUE;
      result.node = make_dump_node(CallSemKind::literal, folded_literal.text);
      set_expr_metadata(result.node, result.type, result.category);
      if(folded_literal.has_int_value) {
        set_callsem_int_value(result.node, folded_literal.int_value);
      }
      if(folded_literal.has_uint_value) {
        set_callsem_uint_value(result.node, folded_literal.uint_value);
      }
      return result;
    }
  }

  if(binding.owner_class && binding.owner_class->member_scope) {
    if(binding.owner_class->source_template &&
       !binding.owner_class->out_of_class_static_member_definitions_applied) {
      template_api::TemplateClassFinalizationRequest request;
      if(template_api::build_class_finalization_request(*binding.owner_class, request)) {
        template_api::finalize_class_instantiation(ctx, request);
      }
    }
    map<string, ValueBinding>::iterator required =
        binding.owner_class->member_scope->values.find(binding.name);
    if(required != binding.owner_class->member_scope->values.end() &&
       required->second.kind == ValueBinding::VK_VARIABLE &&
       required->second.owner_class == binding.owner_class) {
      const bool emit_required_static_member_definition =
          !template_api::value_binding_output_suppressed_by_explicit_instantiation(
              required->second);
      if(emit_required_static_member_definition) {
        add_output_requirement(required->second.output_requirements, ORK_DEFINITION);
        if(template_api::class_has_template_identity(binding.owner_class) &&
           !binding.owner_class->definition_output_in_progress &&
           !required->second.definition_output_emitted) {
          binding.owner_class->has_late_required_static_member_output = true;
        }
        ctx.track_instantiated_class(binding.owner_class);
        template_api::note_template_member_value_instantiation_if_needed(
            ctx,
            required->second);
      }
    }
  }

  ExprInfo result;
  TypePtr binding_type = strip_top_level_cv(binding.type);
  if(binding_type &&
     (binding_type->kind == Type::TK_LVALUE_REFERENCE ||
      binding_type->kind == Type::TK_RVALUE_REFERENCE)) {
    result.type = binding_type->inner;
  } else {
    result.type = binding.type;
  }
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::id_expression, binding.name);
  symbol_linkage::SymbolIdentity symbol = binding.symbol;
  if(symbol.internal_symbol.empty() &&
     binding.owner_class &&
     binding.owner_class->member_scope) {
    symbol = expression_static_member_variable_symbol_identity(*binding.owner_class,
                                                               binding);
  }
  if(binding.is_thread_local &&
     symbol.thread_local_wrapper_object_symbol.empty()) {
    if(binding.owner_class) {
      symbol.thread_local_wrapper_object_symbol =
          symbol_linkage::thread_local_wrapper_object_symbol_for_static_member_variable(
              *binding.owner_class,
              binding.name);
    } else if(binding.declaration_scope) {
      symbol.thread_local_wrapper_object_symbol =
          symbol_linkage::thread_local_wrapper_object_symbol_for_scoped_variable(
              *binding.declaration_scope,
              binding.name);
    }
  }
  set_dump_symbol(result.node, symbol);
  result.node.is_thread_local = binding.is_thread_local;
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

bool try_analyze_reference_binding_source_expression_impl(SemanticContext & ctx,
                                                          Scope & scope,
                                                          const CppAstNode & node,
                                                          ExprInfo & out)
{
  if(node.kind != CppAstKind::id_expression) {
    return false;
  }

  bool allow_constant_fold = false;
  const ValueBinding * binding =
      lookup_id_expression_value_binding(ctx, scope, node, allow_constant_fold);
  if(!binding || pure_constant_only_value_binding(*binding)) {
    return false;
  }
  if(try_analyze_non_type_template_function_value(ctx, *binding, out)) {
    return true;
  }
  if(try_analyze_non_type_template_object_pointer_value(ctx,
                                                        scope,
                                                        *binding,
                                                        out)) {
    return true;
  }
  if(binding->non_type_template_value_binding &&
     binding->type &&
     (binding->type->kind == Type::TK_LVALUE_REFERENCE ||
      binding->type->kind == Type::TK_RVALUE_REFERENCE)) {
    CppAstNode resolved_node = node;
    resolved_node.value = binding->non_type_template_value_binding->name;
    out = make_value_binding_expr(ctx,
                                  scope,
                                  resolved_node,
                                  *binding->non_type_template_value_binding,
                                  false);
    return true;
  }
  out = make_value_binding_expr(ctx, scope, node, *binding, false);
  return true;
}

VariableTemplateDecl * lookup_variable_template_id_for_node(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    const TemplateIdSyntax & template_id,
    ClassInfo ** owner_out = nullptr)
{
  if(owner_out) {
    *owner_out = nullptr;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified &&
     (qualified->rooted || !qualified->qualifiers.empty()) &&
     (cppast_has_qualifier_template_id_syntaxes(node) ||
      !node.qualifier_type_syntaxes.empty())) {
    Scope * target = ctx.resolve_qualified_scope_for_node(
        scope, *qualified, node, false);
    if(!target) {
      return nullptr;
    }
    if(target->class_info) {
      semantic_lookup::MemberVariableTemplateLookupResult member =
          semantic_lookup::lookup_member_variable_template(ctx,
                                                           *target->class_info,
                                                           template_id.name.name);
      if(member.variable_template) {
        if(owner_out) {
          *owner_out = const_cast<ClassInfo *>(
              member.declared_in ? member.declared_in : target->class_info);
        }
        return member.variable_template;
      }
      VariableTemplateDecl * direct =
          semantic_lookup::lookup_direct_variable_template(*target,
                                                           template_id.name.name);
      if(owner_out && direct) {
        *owner_out = target->class_info;
      }
      return direct;
    }
    return semantic_lookup::lookup_direct_variable_template(
        *target, template_id.name.name);
  }
  return semantic_lookup::lookup_variable_template(ctx, scope, template_id.name);
}

bool target_aware_decline_requires_audit(SemanticContext & ctx,
                                         Scope & scope,
                                         const CppAstNode & node,
                                         const TypePtr & target,
                                         string & detail)
{
  if(!target) {
    return false;
  }

  const CppAstNode * payload = &node;
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    payload = &node.children[0];
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(payload->kind == CppAstKind::braced_init_list) {
    TypePtr element_type;
    if(is_initializer_list_type(ctx, target_base, &element_type)) {
      detail = "target-aware initializer_list braced-init declined [target " +
               describe_type(target) + "]";
      return true;
    }
    if(target_base && target_base->kind == Type::TK_ARRAY) {
      detail = "target-aware array braced-init declined [target " +
               describe_type(target) + "]";
      return true;
    }
    if(target_base && ctx.complete_class_type(target_base)) {
      detail = "target-aware class braced-init declined [target " +
               describe_type(target) + "]";
      return true;
    }
    return false;
  }

  if(payload->kind != CppAstKind::call_expression ||
     payload->children.empty() ||
     payload->children[0].kind != CppAstKind::id_expression) {
    return false;
  }

  TypePtr stripped_target = strip_top_level_cv(target);
  if(stripped_target &&
     (stripped_target->kind == Type::TK_LVALUE_REFERENCE ||
      stripped_target->kind == Type::TK_RVALUE_REFERENCE)) {
    return false;
  }
  TypePtr stripped_target_base = strip_top_level_cv(remove_reference_type(target));
  if(!stripped_target_base || !ctx.complete_class_type(stripped_target_base)) {
    return false;
  }

  TypePtr callee_type =
      ctx.lookup_type_node(scope,
                           payload->children[0],
                           payload->children[0].value,
                           false);
  if(!callee_type ||
     !same_type_with_compatible_top_cv(strip_top_level_cv(callee_type),
                                       strip_top_level_cv(target))) {
    return false;
  }

  if(!ctx.lookup_functions_node(scope,
                                payload->children[0],
                                payload->children[0].value,
                                semantic_policy::without_body_instantiation()).empty()) {
    return false;
  }

  detail = "target-aware type-style construction declined [target " +
           describe_type(target) + "]";
  return true;
}

}  // namespace

bool try_analyze_reference_binding_source_expression(SemanticContext & ctx,
                                                     Scope & scope,
                                                     const CppAstNode & node,
                                                     ExprInfo & out)
{
  return try_analyze_reference_binding_source_expression_impl(ctx, scope, node, out);
}

ExprInfo analyze_expression_for_target(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node,
                                       const TypePtr & target)
{
  DIAG_CONTEXT("semantic_expression::analyze_expression_for_target [" + node_text(node) +
               " -> " + (target ? describe_type(target) : std::string("<null-type>")) + "]" +
               ctx.source_location_for_node(node));
  ScopedCallSemConstructionPath construction_path("expression-for-target");
  ExprInfo expr;
  if(!ctx.try_analyze_target_aware_expression(scope, node, target, expr)) {
    string target_aware_detail;
    if(target_aware_decline_requires_audit(ctx, scope, node, target, target_aware_detail)) {
      hard_fail_semantic_fallback(ctx,
                                  node,
                                  "target-aware-to-generic-expression",
                                  target_aware_detail);
    }
    if(node.kind == CppAstKind::braced_init_list &&
       node.children.empty() &&
       target &&
       !is_reference_type(target)) {
      expr = ctx.make_value_initialized_expr(target);
    } else if(!is_reference_type(target) ||
              !try_analyze_reference_binding_source_expression_impl(ctx, scope, node, expr)) {
      expr = ctx.analyze_expression(scope, node);
    }
  }
  semantic_lifetime::require_reference_bound_temporary_destructor_if_needed(ctx,
                                                                            target,
                                                                            expr);
  ExprInfo converted;
  ConversionRank rank = CR_BAD;
  if(ctx.try_argument_conversion(scope,
                                 target,
                                 expr,
                                 converted,
                                 rank,
                                 semantic_policy::no_output_materialization_argument_conversion())) {
    return converted;
  }
  return expr;
}

namespace {

vector<const CppAstNode *> expand_braced_init_list_elements(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node,
    vector<unique_ptr<CppAstNode> > & storage)
{
  vector<const CppAstNode *> out;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind != CppAstKind::pack_expansion_expression) {
      out.push_back(&child);
      continue;
    }
    vector<CppAstNode> expanded_nodes;
    if(!ctx.expand_pack_argument_node(scope, child, expanded_nodes)) {
      throw logic_error("unsupported braced-init-list pack-expansion element");
    }
    for(size_t j = 0; j < expanded_nodes.size(); ++j) {
      storage.emplace_back(new CppAstNode(expanded_nodes[j]));
      out.push_back(storage.back().get());
    }
  }
  return out;
}

}  // namespace

ExprInfo make_initializer_list_expression(SemanticContext & ctx,
                                          Scope & scope,
                                          const TypePtr & initlist_type,
                                          const CppAstNode & node)
{
  ScopedCallSemConstructionPath construction_path("initializer-list-expression");
  TypePtr element_type;
  if(node.kind != CppAstKind::braced_init_list ||
     !is_initializer_list_type(ctx, initlist_type, &element_type)) {
    throw logic_error("invalid initializer_list construction");
  }

  ExprInfo result;
  result.type = initlist_type;
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::initializer_list_object);
  set_expr_metadata(result.node, result.type, result.category);
  set_callsem_initializer_list_element_type(result.node, element_type);
  vector<unique_ptr<CppAstNode> > expanded_storage;
  vector<const CppAstNode *> elements =
      expand_braced_init_list_elements(ctx, scope, node, expanded_storage);
  for(size_t i = 0; i < elements.size(); ++i) {
    if(semantic_lifetime::scalar_list_initialization_has_narrowing_conversion(
           ctx, scope, *elements[i], element_type)) {
      throw logic_error("narrowing initializer_list element");
    }
    ExprInfo element = ctx.analyze_expression_for_target(scope, *elements[i], element_type);
    if(!can_copy_initialize(ctx, element_type, element)) {
      throw logic_error("invalid initializer_list element");
    }
    result.node.children.push_back(std::move(element.node));
  }
  return result;
}

bool try_analyze_array_braced_init_list_expression(SemanticContext & ctx,
                                                   Scope & scope,
                                                   const TypePtr & expr_type,
                                                   const CppAstNode & node,
                                                   ExprInfo & out)
{
  TypePtr expr_base = strip_top_level_cv(remove_reference_type(expr_type));
  if(!expr_base || expr_base->kind != Type::TK_ARRAY ||
     node.kind != CppAstKind::braced_init_list ||
     semantic_lifetime::has_designated_braced_init(node)) {
    return false;
  }

  vector<unique_ptr<CppAstNode> > expanded_storage;
  vector<const CppAstNode *> elements =
      expand_braced_init_list_elements(ctx, scope, node, expanded_storage);
  if((expr_base->has_bound && elements.size() > expr_base->bound) ||
     (!expr_base->has_bound && elements.empty())) {
    return false;
  }
  const size_t bound = expr_base->has_bound ? expr_base->bound : elements.size();

  TypePtr direct_expr_base = strip_top_level_cv(expr_type);
  const bool reference_target =
      direct_expr_base &&
      (direct_expr_base->kind == Type::TK_LVALUE_REFERENCE ||
       direct_expr_base->kind == Type::TK_RVALUE_REFERENCE);

  ExprInfo result;
  result.type = expr_base->has_bound ?
                    expr_type :
                    make_array(expr_base->inner, true, bound);
  result.category = reference_target ? VC_XVALUE : VC_LVALUE;
  result.node = make_dump_node(CallSemKind::braced_init_list);
  set_expr_metadata(result.node, result.type, result.category);

  try
  {
    for(size_t i = 0; i < bound; ++i) {
      ExprInfo element;
      if(i < elements.size()) {
        if(semantic_lifetime::scalar_list_initialization_has_narrowing_conversion(
               ctx, scope, *elements[i], expr_base->inner)) {
          return false;
        }
        element = ctx.analyze_expression_for_target(scope, *elements[i], expr_base->inner);
        if(!can_copy_initialize(ctx, expr_base->inner, element)) {
          return false;
        }
      } else {
        TypePtr element_base =
            strip_top_level_cv(remove_reference_type(expr_base->inner));
        if(element_base &&
           (element_base->kind == Type::TK_ARRAY ||
            ctx.complete_class_type(element_base))) {
          CppAstNode empty;
          empty.kind = CppAstKind::braced_init_list;
          element = ctx.analyze_expression_for_target(scope, empty, expr_base->inner);
        } else {
          element = ctx.make_value_initialized_expr(expr_base->inner);
        }
      }
      result.node.children.push_back(element.node);
    }
  }
  catch(const std::logic_error &)
  {
    return false;
  }

  out = result;
  return true;
}

ExprInfo analyze_braced_init_list_expression(SemanticContext & ctx,
                                             Scope & scope,
                                             const CppAstNode & node)
{
  ScopedCallSemConstructionPath construction_path("braced-init-list");
  if(node.kind != CppAstKind::braced_init_list || node.children.empty()) {
    throw logic_error("unsupported braced-init-list expression");
  }

  vector<unique_ptr<CppAstNode> > expanded_storage;
  vector<const CppAstNode *> elements =
      expand_braced_init_list_elements(ctx, scope, node, expanded_storage);
  if(elements.empty()) {
    throw logic_error("unsupported braced-init-list expression");
  }

  ExprInfo first = ctx.analyze_expression(scope, *elements[0]);
  TypePtr element_type = value_conversion_type(first);
  if(!element_type || is_void_type(element_type)) {
    throw logic_error("invalid braced-init-list element");
  }

  ExprInfo result;
  result.type = make_array(element_type, true, elements.size());
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::braced_init_list);
  set_expr_metadata(result.node, result.type, result.category);
  result.node.children.push_back(std::move(first.node));

  for(size_t i = 1; i < elements.size(); ++i) {
    ExprInfo element = ctx.analyze_expression_for_target(scope, *elements[i], element_type);
    if(!can_copy_initialize(ctx, element_type, element)) {
      throw logic_error("inconsistent braced-init-list element type");
    }
    result.node.children.push_back(std::move(element.node));
  }

  return result;
}

ExprInfo analyze_this_expression(SemanticContext & ctx,
                                 Scope & scope,
                                 const CppAstNode & node)
{
  Scope * function_scope = nullptr;
  ClassInfo * function_class = nullptr;
  ExprInfo raw_this = make_raw_this_expr(ctx, scope, node, function_scope, function_class);
  if(function_class && function_class->is_lambda_closure) {
    MemberValueLookupResult captured_this =
        lookup_member_value(*function_class, "this");
    if(captured_this.binding && captured_this.binding->kind == ValueBinding::VK_FIELD &&
       function_scope->function) {
      TypePtr method_type = strip_top_level_cv(function_scope->function->type);
      if(method_type && method_type->kind == Type::TK_FUNCTION &&
         !method_type->params.empty()) {
        ExprInfo result;
        result.type = captured_this.binding->type;
        result.category = VC_LVALUE;
        result.node = make_dump_node(CallSemKind::member_expression,
                                     captured_this.binding->name);
        set_dump_token(result.node, node);
        set_expr_metadata(result.node, result.type, result.category);
        set_callsem_uint_value(
            result.node,
            captured_this.path_offset + captured_this.binding->field_offset);
        result.node.is_reference_storage =
            is_reference_type(captured_this.binding->type);
        result.node.children.push_back(std::move(raw_this.node));
        return result;
      }
    }
  }
  return raw_this;
}

ExprInfo analyze_member_expression(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node)
{
  DIAG_CONTEXT("analyze_member_expression [" + node_text(node) + "]" +
               ctx.source_location_for_node(node));
  if(node.children.size() != 2 || node.children[1].kind != CppAstKind::identifier) {
    throw logic_error("unsupported member-expression");
  }

  ExprInfo base = ctx.analyze_expression(scope, node.children[0]);
  TypePtr base_type = strip_top_level_cv(remove_reference_type(base.type));
  TypePtr member_object_type = member_object_cv_source_type(base);
  ClassInfo * class_info = nullptr;
  const bool is_arrow_access = node_has_simple_type(node, OP_ARROW);
  const auto member_access_result_category =
      [&base, is_arrow_access](const TypePtr & member_type) -> ValueCategory
      {
        if(is_reference_type(member_type)) {
          return VC_LVALUE;
        }
        if(is_arrow_access) {
          return VC_LVALUE;
        }
        return base.category == VC_LVALUE ? VC_LVALUE : VC_XVALUE;
      };
  if(node_has_simple_type(node, OP_DOT)) {
    if(base.category != VC_LVALUE &&
       base.category != VC_XVALUE &&
       base.category != VC_PRVALUE) {
      throw logic_error("dot requires class object");
    }
    class_info = complete_class_type_for_lookup(ctx, base_type);
  } else if(node_has_simple_type(node, OP_ARROW)) {
    if(base_type && base_type->kind == Type::TK_POINTER) {
      class_info = complete_class_type_for_lookup(ctx, base_type->inner);
    } else {
      ClassInfo * base_class =
          base_type ? complete_class_type_for_lookup(ctx, base_type) : nullptr;
      if(!base_class && base_type) {
        base_class = ctx.class_info_for_type(base_type);
      }
      const bool has_member_arrow =
          base_class &&
          (!lookup_visible_member_functions(*base_class, "operator->").functions.empty() ||
           (base_class->member_scope &&
            !lookup_direct_function_templates(*base_class->member_scope, "operator->").empty()));
      if(has_member_arrow) {
        CppAstNode operator_call;
        operator_call.kind = CppAstKind::call_expression;
        operator_call.children.push_back(
            make_dot_member_operator_callee(node.children[0], "operator->"));

        CppAstNode arguments;
        arguments.kind = CppAstKind::paren_argument_list;
        operator_call.children.push_back(arguments);

        CppAstNode rewritten = node;
        rewritten.children[0] = operator_call;
        return analyze_member_expression(ctx, scope, rewritten);
      }
      if((!base_class || !base_class->complete) &&
         ctx.type_depends_on_template_parameter(base.type)) {
        class_info = base_class;
      } else {
        ostringstream out;
        out << "arrow requires pointer";
        if(base_type) {
          out << " [base type " << describe_type(base_type) << "]";
        }
        out << " [base expr " << callsem_display_text(base.node) << "]";
        out << " [member " << node.children[1].value << "]";
        out << " [class "
            << (base_class ? base_class->qualified_name : string("<none>")) << "]";
        throw logic_error(out.str());
      }
    }
  } else {
    throw logic_error("unsupported member access operator");
  }

  if((!class_info || !class_info->complete) &&
     ctx.type_depends_on_template_parameter(base.type)) {
    const string member_text = semantic_utils::trim_space(node.children[1].value);
    const string template_prefix = "template ";
    const bool has_template_keyword =
        member_text.size() > template_prefix.size() &&
        member_text.compare(0, template_prefix.size(), template_prefix) == 0;
    const string template_check_text =
        has_template_keyword ?
            semantic_utils::trim_space(member_text.substr(template_prefix.size())) :
            member_text;
    if(cppast_template_id_syntax(node.children[1]) && !has_template_keyword) {
      throw logic_error("dependent member template requires template keyword");
    }

    ExprInfo result;
    result.type = make_named(node_text(node),
                             "dependent type " + node_text(node),
                             true);
    result.category = is_arrow_access || base.category == VC_LVALUE ?
                      VC_LVALUE : VC_XVALUE;
    result.node = make_dump_node(CallSemKind::member_expression,
                                 template_check_text);
    set_dump_token(result.node, node);
    set_expr_metadata(result.node, result.type, result.category);
    result.node.children.push_back(std::move(base.node));
    return result;
  }

  if(!class_info || !class_info->complete) {
    ostringstream out;
    out << "member access requires complete class type";
    if(base_type) {
      out << " [base type " << describe_type(base_type) << "]";
    }
    out << " [base expr " << callsem_display_text(base.node) << "]";
    out << " [member " << node.children[1].value << "]";
    out << " [expr " << node_text(node) << "]";
    throw logic_error(out.str());
  }

  const QualifiedName * member_name = cppast_qualified_name_syntax(node.children[1]);
  if(!member_name) {
    throw logic_error("member-expression target missing structured name");
  }

  QualifiedMemberTarget target;
  if(!resolve_qualified_member_target(ctx,
                                      scope,
                                      *class_info,
                                      *member_name,
                                      target,
                                      true,
                                      &node.children[1])) {
    throw logic_error("unsupported qualified member-expression");
  }

  MemberValueLookupResult field = lookup_member_value(*target.target_class, target.lookup_name);
  field.path_access = combine_member_access(target.path_access, field.path_access);
  field.path_offset += target.path_offset;
  if(!field.binding) {
    MemberFunctionLookupResult member_functions = target.qualified ?
        lookup_visible_member_functions(*target.target_class, target.lookup_name) :
        lookup_visible_member_functions(*class_info, target.lookup_name);
    member_functions.path_access =
        combine_member_access(target.path_access, member_functions.path_access);
    vector<FunctionBinding *> static_functions;
    for(size_t i = 0; i < member_functions.functions.size(); ++i) {
      FunctionBinding * function = member_functions.functions[i];
      if(function && !function->is_method) {
        static_functions.push_back(function);
      }
    }
    if(static_functions.size() == 1) {
      FunctionBinding * function = static_functions[0];
      const ClassInfo * declared_in =
          member_functions.declared_in ? member_functions.declared_in :
                                         function->owner_class;
      MemberAccess access =
          declared_in && declared_in->member_scope ?
              effective_direct_function_access(*declared_in->member_scope,
                                               target.lookup_name,
                                               *function) :
              function->access;
      if(!member_access_allowed_through_object(&scope,
                                               current_class_scope(scope),
                                               current_function_scope(scope),
                                               target.target_class,
                                               declared_in,
                                               access,
                                               member_functions.path_access)) {
        throw logic_error("inaccessible member");
      }
      ctx.require_function_definition(function,
                                      OutputReason::FunctionIdUse,
                                      !function->is_deleted);
      ExprInfo result;
      result.type = function->type;
      result.category = VC_LVALUE;
      result.node = make_dump_node(CallSemKind::id_expression, target.lookup_name);
      result.node.is_c_linkage = function->is_c_linkage;
      set_dump_symbol(result.node, function->symbol);
      set_dump_token(result.node, node);
      set_expr_metadata(result.node, result.type, result.category);
      return result;
    }
    throw NotDataMemberExpressionError("member expression is not a data member");
  }
  if(!member_access_allowed_through_object(&scope,
                                           current_class_scope(scope),
                                           current_function_scope(scope),
                                           target.target_class,
                                           field.declared_in,
                                           field.binding->access,
                                           field.path_access)) {
    throw logic_error("inaccessible member");
  }
  if(value_binding_is_enumerator(*field.binding)) {
    return make_enumerator_value_expr(node.children[1], *field.binding);
  }
  if(field.binding->kind == ValueBinding::VK_VARIABLE) {
    return make_static_member_variable_expr(ctx, *field.binding);
  }
  if(field.binding->kind != ValueBinding::VK_FIELD) {
    throw NotDataMemberExpressionError("member expression is not a data member");
  }
  const ClassInfo * field_owner_class =
      field.binding->owner_class ? field.binding->owner_class : field.declared_in;
  const size_t owner_path_offset = member_binding_owner_path_offset(field);
  ExprInfo member_base =
      adjust_member_declaring_base_if_needed(ctx,
                                             base,
                                             member_object_type,
                                             target.target_class,
                                             field_owner_class,
                                             owner_path_offset);
  if(field.binding->is_bit_field) {
    return make_bit_field_storage_expr(ctx,
                                       member_base,
                                       *field.binding,
                                       field_owner_class == target.target_class ?
                                           owner_path_offset : 0);
  }

  ClassInfo * base_subobject_class = ctx.class_info_for_type(
      strip_top_level_cv(remove_reference_type(field.binding->type)));
  const size_t subobject_offset =
      (field_owner_class == target.target_class ? owner_path_offset : 0) +
      field.binding->field_offset;
  if(!is_reference_type(field.binding->type) &&
     base_subobject_class &&
     (field.binding->name == base_subobject_class->name ||
      field.binding->name == base_subobject_class->qualified_name)) {
    ExprInfo adjusted =
        ctx.apply_base_subobject_adjustment(
            member_base,
            resolve_instantiated_member_object_type(
                ctx,
                scope,
                field,
                apply_member_object_cv(field.binding->type,
                                       member_object_type,
                                       field.binding->is_mutable)),
            *base_subobject_class,
            subobject_offset);
    if(is_arrow_access &&
       adjusted.node.kind == CallSemKind::unary_expression &&
       callsem_has_token(adjusted.node, OP_AMP) &&
       adjusted.node.children.size() == 1) {
      ExprInfo result;
      result.type = resolve_instantiated_member_object_type(
          ctx,
          scope,
          field,
          apply_member_object_cv(field.binding->type,
                                 member_object_type,
                                 field.binding->is_mutable));
      result.category = member_access_result_category(result.type);
      result.node = adjusted.node.children[0];
      set_dump_token(result.node, node);
      set_expr_metadata(result.node, result.type, result.category);
      return result;
    }
    adjusted.type = resolve_instantiated_member_object_type(
        ctx,
        scope,
        field,
        apply_member_object_cv(field.binding->type,
                               member_object_type,
                               field.binding->is_mutable));
    adjusted.category = member_access_result_category(adjusted.type);
    set_dump_token(adjusted.node, node);
    set_expr_metadata(adjusted.node, adjusted.type, adjusted.category);
    return adjusted;
  }

  ExprInfo result;
  result.type = resolve_instantiated_member_object_type(
      ctx,
      scope,
      field,
      apply_member_object_cv(field.binding->type,
                             member_object_type,
                             field.binding->is_mutable));
  result.category = member_access_result_category(result.type);
  result.node = make_dump_node(CallSemKind::member_expression, field.binding->name);
  set_dump_token(result.node, node);
  set_expr_metadata(result.node, result.type, result.category);
  set_callsem_uint_value(result.node, subobject_offset);
  result.node.is_reference_storage = is_reference_type(field.binding->type);
  result.node.children.push_back(std::move(member_base.node));
  return result;
}

ExprInfo analyze_literal(SemanticContext & ctx, Scope & scope, const CppAstNode & node)
{
  ExprInfo result;
  result.node = make_dump_node(CallSemKind::literal, node.value);
  if(node_has_simple_type(node, KW_NULLPTR)) {
    result.type = make_fundamental(FT_NULLPTR_T);
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    set_dump_token(result.node, node);
    return result;
  }
  if(node_has_simple_type(node, KW_TRUE)) {
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, 1);
    set_dump_token(result.node, node);
    return result;
  }
  if(node_has_simple_type(node, KW_FALSE)) {
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, 0);
    set_dump_token(result.node, node);
    return result;
  }
  if(is_floating_literal(node.value)) {
    EFundamentalType literal_type = FT_DOUBLE;
    string ud_suffix;
    if(!classify_floating_literal_type(node.value, literal_type, ud_suffix)) {
      throw logic_error("invalid floating literal suffix");
    }
    if(!ud_suffix.empty()) {
      const string operator_name = string("operator\"\"") + ud_suffix;
      if(has_cooked_numeric_literal_operator(ctx,
                                             scope,
                                             operator_name,
                                             FT_LONG_DOUBLE)) {
        return analyze_cooked_numeric_user_defined_literal(
            ctx, scope, node, ud_suffix, literal_without_ud_suffix(node.value, ud_suffix));
      }
      return analyze_numeric_user_defined_literal_template(
          ctx, scope, node, ud_suffix);
    }
    result.type = make_fundamental(literal_type);
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  if(is_integer_literal(node.value)) {
    unsigned long long literal_value = 0;
    string ud_suffix;
    EFundamentalType literal_type = classify_int(node.value, literal_value, ud_suffix);
    if(!ud_suffix.empty()) {
      const string operator_name = string("operator\"\"") + ud_suffix;
      if(has_cooked_numeric_literal_operator(ctx,
                                             scope,
                                             operator_name,
                                             FT_UNSIGNED_LONG_LONG_INT)) {
        return analyze_cooked_numeric_user_defined_literal(
            ctx, scope, node, ud_suffix, literal_without_ud_suffix(node.value, ud_suffix));
      }
      return analyze_numeric_user_defined_literal_template(ctx, scope, node, ud_suffix);
    }
    TypePtr annotated_type = strip_top_level_cv(node.semantic_type);
    const bool annotated_integral_or_enum =
        annotated_type &&
        (is_integral_type(annotated_type) ||
         is_named_enum_type(ctx, annotated_type));
    result.type = annotated_integral_or_enum ?
        annotated_type :
        make_fundamental(literal_type);
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, literal_value);
    return result;
  }

  if(node.value.find('"') != string::npos) {
    QuoteLiteralData literal = parse_quote_literal(node.value);
    if(literal.quote == '"') {
      if(!literal.ud_suffix.empty()) {
        return analyze_string_user_defined_literal(ctx, scope, node, literal);
      }
      result.type = make_array(
          make_cv(make_fundamental(string_literal_element_type(literal)),
                  true,
                  false),
          true,
          string_literal_code_unit_count(literal) + 1);
      result.category = VC_LVALUE;
      set_expr_metadata(result.node, result.type, result.category);
      return result;
    }
  }

  if(node.value.find('\'') != string::npos) {
    QuoteLiteralData literal = parse_quote_literal(node.value);
    if(literal.contents.empty()) {
      throw logic_error("empty character literal");
    }
    unsigned int multicharacter_value = 0;
    const bool is_multicharacter =
        ordinary_multicharacter_literal_value(literal,
                                              multicharacter_value);
    if(!is_multicharacter && literal.contents.size() != 1) {
      throw logic_error("invalid multicharacter literal");
    }
    result.type = make_fundamental(
        is_multicharacter ? FT_INT : character_literal_type(literal));
    result.category = VC_PRVALUE;
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node,
                           is_multicharacter
                               ? static_cast<unsigned long long>(
                                     multicharacter_value)
                               : static_cast<unsigned long long>(
                                     literal.contents[0]));
    return result;
  }

  throw logic_error(string("unsupported literal: ") + node.value);
}

ExprInfo analyze_id_expression(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & node)
{
  DIAG_CONTEXT("analyze_id_expression [" + node.value + "]" +
               ctx.source_location_for_node(node));
  if(node.value == "this") {
    CppAstNode this_node = node;
    this_node.kind = CppAstKind::keyword_literal;
    this_node.has_token = true;
    this_node.token_kind = RT_SIMPLE;
    this_node.simple_type = KW_THIS;
    return ctx.analyze_this_expression(scope, this_node);
  }
  if(node.value == "__null") {
    // Clang's hosted headers spell NULL as __null, which behaves like the
    // integral null constant 0L rather than std::nullptr_t.
    return make_typed_integer_literal_expr(ctx, make_fundamental(FT_LONG_INT), 0);
  }
  if((node.value == "__func__" ||
      node.value == "__FUNCTION__" ||
      node.value == "__PRETTY_FUNCTION__")) {
    if(FunctionBinding * current_function = current_function_scope(scope)) {
      if(node.value == "__PRETTY_FUNCTION__") {
        return make_string_literal_expr(
            ctx, semantic_model::predefined_pretty_function_text(*current_function));
      }
      return make_string_literal_expr(
          ctx, function_binding_display_name_for_symbol(*current_function));
    }
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(qualified &&
     !qualified->qualifiers.empty() &&
     node.qualifier_template_id_syntaxes.size() == 1) {
    const TemplateIdSyntax & qualifier_template_id =
        node.qualifier_template_id_syntaxes.back();
    const bool use_folded_member_value =
        qualifier_template_id_needs_constant_member_shortcut(qualifier_template_id) ||
        qualifier_template_id.name.name == "is_same";
    constant_eval::ConstexprValue constexpr_value;
    std::string qualifier_use_location =
        ctx.source_location_for_name_in_node(node,
                                             qualifier_template_id.name.name,
                                             false);
    if(qualifier_use_location.empty()) {
      qualifier_use_location = ctx.source_location_for_node(node);
    }
    const ScopedTemplateUseLocation use_location(qualifier_use_location);
    const parser_trace::ScopedOrderUseLocation order_use_location(
        qualifier_use_location);
    if(ctx.lookup_constant_template_member_value(scope,
                                                 qualifier_template_id,
                                                 qualified->name,
                                                 node.value,
                                                 constexpr_value)) {
      if(use_folded_member_value) {
        FoldedIntegralLiteral folded_literal;
        const TypePtr result_type =
            strip_top_level_cv(remove_reference_type(constexpr_value.type));
        if(result_type &&
           constexpr_value_to_literal_value(ctx,
                                            constexpr_value,
                                            result_type,
                                            folded_literal)) {
          ExprInfo result;
          result.type = result_type;
          result.category = VC_PRVALUE;
          result.node = make_dump_node(CallSemKind::literal, folded_literal.text);
          set_expr_metadata(result.node, result.type, result.category);
          if(folded_literal.has_int_value) {
            set_callsem_int_value(result.node, folded_literal.int_value);
          }
          if(folded_literal.has_uint_value) {
            set_callsem_uint_value(result.node, folded_literal.uint_value);
          }
          return result;
        }
      }
    }
  }

  bool allow_constant_fold = false;
  const ValueBinding * binding = lookup_id_expression_value_binding(ctx, scope, node,
                                                                    allow_constant_fold);
  if(binding) {
    ExprInfo template_function_value;
    if(try_analyze_non_type_template_function_value(ctx,
                                                    *binding,
                                                    template_function_value)) {
      return template_function_value;
    }
    ExprInfo template_object_pointer_value;
    if(try_analyze_non_type_template_object_pointer_value(
           ctx,
           scope,
           *binding,
           template_object_pointer_value)) {
      return template_object_pointer_value;
    }
    if(binding->non_type_template_value_binding &&
       binding->type &&
       (binding->type->kind == Type::TK_LVALUE_REFERENCE ||
        binding->type->kind == Type::TK_RVALUE_REFERENCE)) {
      CppAstNode resolved_node = node;
      resolved_node.value = binding->non_type_template_value_binding->name;
      return make_value_binding_expr(ctx,
                                     scope,
                                     resolved_node,
                                     *binding->non_type_template_value_binding,
                                     allow_constant_fold);
    }
    return make_value_binding_expr(ctx, scope, node, *binding, allow_constant_fold);
  }

  ExprInfo dependent_qualified;
  if(qualified &&
     try_analyze_dependent_qualified_id_expression(ctx,
                                                   scope,
                                                   node,
                                                   *qualified,
                                                   dependent_qualified)) {
    return dependent_qualified;
  }

  const TemplateIdSyntax * parsed_template_id = cppast_template_id_syntax(node);
  if(parsed_template_id) {
    ClassInfo * variable_template_owner = nullptr;
    VariableTemplateDecl * variable_template =
        lookup_variable_template_id_for_node(ctx,
                                             scope,
                                             node,
                                             *parsed_template_id,
                                             &variable_template_owner);
    if(variable_template) {
      const ValueBinding * instantiated =
          variable_template_owner ?
              semantic_template_variable::
                  acquire_member_variable_template_binding_for_template_id_source_use(
                      ctx,
                      *variable_template,
                      *variable_template_owner,
                      scope,
                      node,
                      *parsed_template_id) :
              semantic_template_variable::
                  acquire_variable_template_binding_for_template_id_source_use(
                      ctx,
                      *variable_template,
                      scope,
                      node,
                      *parsed_template_id);
      if(instantiated) {
        return make_value_binding_expr(ctx,
                                       scope,
                                       node,
                                       *instantiated,
                                       !instantiated->dependent_template_value);
      }
    }
  }

  vector<FunctionBinding *> functions;
  if(parsed_template_id) {
    functions = ctx.lookup_function_template_id_node(
        scope,
        node,
        *parsed_template_id,
        semantic_policy::default_call_analysis());
  } else if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    functions = ctx.lookup_functions_node(scope,
                                          node,
                                          node.value,
                                          semantic_policy::default_call_analysis());
  } else {
    functions = ctx.lookup_functions(scope,
                                     node.value,
                                     semantic_policy::default_call_analysis());
  }
  if(functions.size() == 1) {
    ctx.require_function_definition(functions[0],
                                    OutputReason::FunctionIdUse,
                                    !functions[0]->is_deleted);
    ExprInfo result;
    result.type = functions[0]->type;
    result.category = VC_LVALUE;
    result.node = make_dump_node(CallSemKind::id_expression, node.value);
    result.node.is_c_linkage = functions[0]->is_c_linkage;
    set_dump_symbol(result.node, functions[0]->symbol);
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }
  if(functions.size() > 1) {
    throw logic_error("overloaded id-expression unsupported");
  }

  ostringstream out;
  out << "unknown id-expression " << node.value;
  if(parsed_template_id) {
    out << " [template-id yes]";
    out << " [template name " << parsed_template_id->name.name << "]";
    out << " [template arg_count " << parsed_template_id->arguments.size() << "]";
    VariableTemplateDecl * variable_template =
        lookup_variable_template_id_for_node(ctx, scope, node, *parsed_template_id);
    out << " [variable template " << (variable_template ? "found" : "missing") << "]";
  } else {
    out << " [template-id no]";
  }
  out << " [scope " << scope.name << "]";
  out << " [bindings " << semantic_model::describe_scope_bindings(scope) << "]";
  throw logic_error(out.str());
}

ExprInfo analyze_unary_expression(SemanticContext & ctx,
                                  Scope & scope,
                                  const CppAstNode & node)
{
  if(node.children.size() != 1) {
    throw logic_error("unary-expression arity");
  }

  if(node_has_simple_type(node, OP_AMP)) {
    ExprInfo member_pointer;
    if(try_analyze_qualified_member_pointer_expression(ctx, scope, node.children[0],
                                                       member_pointer)) {
      set_dump_token(member_pointer.node, node);
      return member_pointer;
    }
  }

  ExprInfo overloaded_result;
  if(try_overloaded_unary_operator(ctx, scope, node, false, overloaded_result)) {
    return overloaded_result;
  }

  ExprInfo operand;
  if(node_has_simple_type(node, OP_AMP) &&
     try_analyze_reference_binding_source_expression_impl(ctx,
                                                          scope,
                                                          node.children[0],
                                                          operand)) {
  } else {
    operand = ctx.analyze_expression(scope, node.children[0]);
  }
  ExprInfo result;
  result.node = make_dump_node(CallSemKind::unary_expression, node.value);
  set_dump_token(result.node, node);
  if(is_gnu_complex_unary_operator(node.value)) {
    TypePtr component_type;
    if(!semantic_builtins::is_gnu_complex_type(
           strip_top_level_cv(remove_reference_type(operand.type)),
           &component_type)) {
      throw logic_error("unsupported GNU complex unary operand");
    }
    result.type = component_type;
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_PLUS) || node_has_simple_type(node, OP_MINUS)) {
    const bool unary_plus = node_has_simple_type(node, OP_PLUS);
    TypePtr operand_type = value_conversion_type(operand);
    if(!builtin_unary_operator_supports_type(node, operand_type)) {
      ExprInfo converted_operand;
      if(try_builtin_unary_class_conversion(ctx,
                                            scope,
                                            node,
                                            operand,
                                            converted_operand,
                                            semantic_policy::default_argument_conversion())) {
        operand = converted_operand;
        operand_type = value_conversion_type(operand);
      }
    }
    if(!builtin_unary_operator_supports_type(node, operand_type)) {
      throw logic_error("unsupported unary arithmetic operand");
    }
    if(unary_plus && is_pointer_type(operand_type)) {
      result.type = operand_type;
    } else if(is_floating_type(operand_type)) {
      result.type = operand_type;
    } else {
      result.type = promoted_integral_result_type(operand_type);
    }
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_COMPL)) {
    TypePtr operand_type = value_conversion_type(operand);
    if(!builtin_unary_operator_supports_type(node, operand_type)) {
      ExprInfo converted_operand;
      if(try_builtin_unary_class_conversion(ctx,
                                            scope,
                                            node,
                                            operand,
                                            converted_operand,
                                            semantic_policy::default_argument_conversion())) {
        operand = converted_operand;
        operand_type = value_conversion_type(operand);
      }
    }
    if(!builtin_unary_operator_supports_type(node, operand_type)) {
      throw logic_error("unsupported unary integral operand");
    }
    result.type = promoted_integral_result_type(operand_type);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_LNOT)) {
    if(!try_condition_test_conversion(ctx, scope, operand)) {
      throw logic_error("unsupported logical negation operand");
    }
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_AMP)) {
    if(operand.category != VC_LVALUE) {
      throw logic_error("address-of requires lvalue");
    }
    TypePtr pointee = remove_reference_type(operand.type);
    if(!pointee) {
      pointee = operand.type;
    }
    result.type = make_pointer(pointee);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_STAR)) {
    TypePtr operand_type = value_conversion_type(operand);
    if(!operand_type || operand_type->kind != Type::TK_POINTER) {
      throw logic_error("indirection requires pointer");
    }
    result.type = operand_type->inner;
    result.category = VC_LVALUE;
  } else if(node_has_simple_type(node, OP_INC) || node_has_simple_type(node, OP_DEC)) {
    if(!is_modifiable_lvalue(operand) ||
       !is_builtin_increment_operand_type(ctx, operand.type)) {
      ExprInfo converted_operand;
      if(try_builtin_increment_class_conversion(ctx, scope, operand, converted_operand)) {
        operand = converted_operand;
      }
    }
    if(!is_modifiable_lvalue(operand) ||
       !is_builtin_increment_operand_type(ctx, operand.type)) {
      throw logic_error("invalid prefix increment/decrement");
    }
    result.type = remove_reference_type(operand.type);
    if(!result.type) {
      result.type = operand.type;
    }
    result.category = VC_LVALUE;
  } else {
    throw logic_error("unsupported unary operator");
  }

  result.node.children.push_back(std::move(operand.node));
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo analyze_postfix_expression(SemanticContext & ctx,
                                    Scope & scope,
                                    const CppAstNode & node)
{
  if(node.children.size() != 1) {
    throw logic_error("postfix-expression arity");
  }

  ExprInfo overloaded_result;
  if(try_overloaded_unary_operator(ctx, scope, node, true, overloaded_result)) {
    return overloaded_result;
  }

  ExprInfo operand = ctx.analyze_expression(scope, node.children[0]);
  if(!is_modifiable_lvalue(operand) ||
     !is_builtin_increment_operand_type(ctx, operand.type)) {
    ExprInfo converted_operand;
    if(try_builtin_increment_class_conversion(ctx, scope, operand, converted_operand)) {
      operand = converted_operand;
    }
  }
  if(!is_modifiable_lvalue(operand) ||
     !is_builtin_increment_operand_type(ctx, operand.type)) {
    throw logic_error("invalid postfix increment/decrement");
  }

  ExprInfo result;
  result.type = remove_reference_type(operand.type);
  if(!result.type) {
    result.type = operand.type;
  }
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::postfix_expression, node.value);
  set_dump_token(result.node, node);
  set_expr_metadata(result.node, result.type, result.category);
  result.node.children.push_back(std::move(operand.node));
  return result;
}

bool try_structured_bool_condition_constant(SemanticContext & ctx,
                                            const ExprInfo & condition,
                                            long long & out)
{
  TypePtr condition_type =
      strip_top_level_cv(remove_reference_type(value_conversion_type(condition)));
  if(!condition_type || condition_type->kind != Type::TK_NAMED) {
    return false;
  }

  bool bool_value = false;
  const bool evaluated =
      template_api::with_template_type_system(
          ctx,
          [&](template_api::TemplateTypeSystem & type_system)
          {
            return template_argument_semantics::
                structured_bool_constant_value_for_type(
                    type_system,
                    condition_type,
                    bool_value);
          });
  if(!evaluated) {
    return false;
  }

  out = bool_value ? 1 : 0;
  return true;
}

ExprInfo analyze_conditional_expression(SemanticContext & ctx,
                                        Scope & scope,
                                        const CppAstNode & node)
{
  if(node.children.size() != 3) {
    throw logic_error("conditional-expression arity");
  }

  ExprInfo condition = ctx.analyze_expression(scope, node.children[0]);
  ExprInfo unconverted_condition = condition;
  if(!try_condition_test_conversion(ctx, scope, condition)) {
    throw logic_error("invalid conditional condition");
  }

  long long condition_constant = 0;
  const bool condition_known =
      try_structured_bool_condition_constant(ctx,
                                             unconverted_condition,
                                             condition_constant);
  ExprInfo then_expr =
      condition_known && condition_constant == 0 ?
          ctx.analyze_expression_without_output_materialization(scope, node.children[1]) :
          ctx.analyze_expression(scope, node.children[1]);
  ExprInfo else_expr =
      condition_known && condition_constant != 0 ?
          ctx.analyze_expression_without_output_materialization(scope, node.children[2]) :
          ctx.analyze_expression(scope, node.children[2]);

  ExprInfo result;
  result.node = make_dump_node(CallSemKind::conditional_expression);

  const auto same_type_with_compatible_conditional_top_cv =
      [](const TypePtr & target, const TypePtr & source) -> bool
  {
    TypePtr target_base;
    TypePtr source_base;
    bool target_const = false;
    bool target_volatile = false;
    bool source_const = false;
    bool source_volatile = false;
    if(!top_level_cv_flags(target, target_base, target_const, target_volatile) ||
       !top_level_cv_flags(source, source_base, source_const, source_volatile)) {
      return false;
    }
    return type_equals(target_base, source_base) &&
           (!source_const || target_const) &&
           (!source_volatile || target_volatile);
  };

  if(then_expr.category == VC_LVALUE && else_expr.category == VC_LVALUE) {
    TypePtr then_lvalue_type = remove_reference_type(then_expr.type);
    TypePtr else_lvalue_type = remove_reference_type(else_expr.type);
    if(type_equals(then_expr.type, else_expr.type) ||
       same_type_with_compatible_conditional_top_cv(then_lvalue_type, else_lvalue_type)) {
      result.type = then_lvalue_type;
      result.category = VC_LVALUE;
    } else if(same_type_with_compatible_conditional_top_cv(else_lvalue_type,
                                                           then_lvalue_type)) {
      result.type = else_lvalue_type;
      result.category = VC_LVALUE;
    } else {
      const auto try_lvalue_conditional_conversion =
          [&](const TypePtr & target_type,
              const ExprInfo & source_expr,
              ExprInfo & converted_source) -> bool
          {
            ConversionRank rank = CR_BAD;
            ArgumentConversionOptions options(false);
            if(!ctx.try_argument_conversion(scope,
                                            target_type,
                                            source_expr,
                                            converted_source,
                                            rank,
                                            options)) {
              return false;
            }
            if(converted_source.category != VC_LVALUE) {
              return false;
            }
            return same_type_with_compatible_conditional_top_cv(
                target_type,
                remove_reference_type(converted_source.type));
          };

      ExprInfo converted_then;
      ExprInfo converted_else;
      const bool else_to_then =
          try_lvalue_conditional_conversion(then_lvalue_type,
                                            else_expr,
                                            converted_else);
      const bool then_to_else =
          try_lvalue_conditional_conversion(else_lvalue_type,
                                            then_expr,
                                            converted_then);
      if(else_to_then && !then_to_else) {
        else_expr = converted_else;
        result.type = then_lvalue_type;
        result.category = VC_LVALUE;
      } else if(then_to_else && !else_to_then) {
        then_expr = converted_then;
        result.type = else_lvalue_type;
        result.category = VC_LVALUE;
      }
    }
  }

  if(result.category != VC_LVALUE) {
    TypePtr then_type = value_conversion_type(then_expr);
    TypePtr else_type = value_conversion_type(else_expr);
    const auto is_null_pointer_expr = [](const ExprInfo & expr) -> bool
    {
      TypePtr base = strip_top_level_cv(remove_reference_type(expr.type));
      return expr.null_pointer_constant ||
             (base && base->kind == Type::TK_FUNDAMENTAL &&
              base->fundamental == FT_NULLPTR_T);
    };
    const auto try_common_conditional_conversion = [&]() -> bool
    {
      const auto reference_target_for_conditional_operand =
          [](const ExprInfo & expr) -> TypePtr
          {
            if(!expr.type) {
              return TypePtr();
            }
            if(expr.category == VC_LVALUE) {
              return make_lvalue_reference_raw(expr.type);
            }
            if(expr.category == VC_XVALUE) {
              return make_rvalue_reference_raw(expr.type);
            }
            return TypePtr();
          };
      const auto converted_matches_reference_target =
          [&](const TypePtr & target, const ExprInfo & converted) -> bool
          {
            TypePtr target_base = strip_top_level_cv(target);
            if(!target_base) {
              return false;
            }
            if(target_base->kind == Type::TK_LVALUE_REFERENCE &&
               converted.category != VC_LVALUE) {
              return false;
            }
            if(target_base->kind == Type::TK_RVALUE_REFERENCE &&
               converted.category != VC_XVALUE) {
              return false;
            }
            TypePtr target_object = remove_reference_type(target_base);
            return target_object &&
                   same_type_with_compatible_conditional_top_cv(
                       target_object,
                       converted.type);
          };
      const auto try_reference_conditional_conversion = [&]() -> bool
      {
        const bool class_operand =
            complete_class_type_for_lookup(ctx, then_type) ||
            complete_class_type_for_lookup(ctx, else_type);
        if(!class_operand) {
          return false;
        }

        TypePtr then_reference_target =
            reference_target_for_conditional_operand(then_expr);
        TypePtr else_reference_target =
            reference_target_for_conditional_operand(else_expr);
        if(!then_reference_target && !else_reference_target) {
          return false;
        }

        ExprInfo converted_then;
        ExprInfo converted_else;
        ConversionRank then_to_else_rank = CR_BAD;
        ConversionRank else_to_then_rank = CR_BAD;
        const bool then_to_else =
            else_reference_target &&
            ctx.try_argument_conversion(
                scope,
                else_reference_target,
                then_expr,
                converted_then,
                then_to_else_rank,
                semantic_policy::default_argument_conversion()) &&
            converted_matches_reference_target(else_reference_target,
                                               converted_then);
        const bool else_to_then =
            then_reference_target &&
            ctx.try_argument_conversion(
                scope,
                then_reference_target,
                else_expr,
                converted_else,
                else_to_then_rank,
                semantic_policy::default_argument_conversion()) &&
            converted_matches_reference_target(then_reference_target,
                                               converted_else);
        if(then_to_else &&
           (!else_to_then || then_to_else_rank < else_to_then_rank)) {
          then_expr = converted_then;
          result.type = remove_reference_type(else_reference_target);
          result.category = else_expr.category;
          return result.type != nullptr;
        }
        if(else_to_then &&
           (!then_to_else || else_to_then_rank < then_to_else_rank)) {
          else_expr = converted_else;
          result.type = remove_reference_type(then_reference_target);
          result.category = then_expr.category;
          return result.type != nullptr;
        }
        return false;
      };
      if(try_reference_conditional_conversion()) {
        return true;
      }

      ExprInfo converted_then;
      ExprInfo converted_else;
      ConversionRank then_to_else_rank = CR_BAD;
      ConversionRank else_to_then_rank = CR_BAD;
      const bool then_to_else =
          ctx.try_argument_conversion(scope,
                                      else_type,
                                      then_expr,
                                      converted_then,
                                      then_to_else_rank,
                                      semantic_policy::default_argument_conversion());
      const bool else_to_then =
          ctx.try_argument_conversion(scope,
                                      then_type,
                                      else_expr,
                                      converted_else,
                                      else_to_then_rank,
                                      semantic_policy::default_argument_conversion());
      if(else_to_then &&
         (!then_to_else || else_to_then_rank < then_to_else_rank)) {
        else_expr = converted_else;
        result.type = then_type;
        return true;
      }
      if(then_to_else &&
         (!else_to_then || then_to_else_rank < else_to_then_rank)) {
        then_expr = converted_then;
        result.type = else_type;
        return true;
      }
      return false;
    };
    if(!then_type || !else_type) {
      throw logic_error("unsupported conditional operands");
    }
    if(type_equals(then_type, else_type)) {
      result.type = then_type;
    } else if((is_integral_or_unscoped_enum_type(then_type) ||
               is_floating_type(then_type)) &&
              (is_integral_or_unscoped_enum_type(else_type) ||
               is_floating_type(else_type))) {
      result.type = common_arithmetic_result_type(then_type, else_type);
    } else if(is_nullable_pointer_like_type(then_type) &&
              is_nullable_pointer_like_type(else_type)) {
      TypePtr then_base = strip_top_level_cv(then_type);
      TypePtr else_base = strip_top_level_cv(else_type);
      if(then_base && else_base &&
         then_base->kind == Type::TK_MEMBER_POINTER &&
         else_base->kind == Type::TK_MEMBER_POINTER &&
         type_equals(then_base->owner, else_base->owner) &&
         same_type_with_compatible_top_cv(then_base->inner, else_base->inner)) {
        result.type = then_type;
      } else if(then_base && else_base &&
                then_base->kind == Type::TK_MEMBER_POINTER &&
                else_base->kind == Type::TK_MEMBER_POINTER &&
                type_equals(then_base->owner, else_base->owner) &&
                same_type_with_compatible_top_cv(else_base->inner, then_base->inner)) {
        result.type = else_type;
      } else if(then_base && else_base &&
         same_type_with_compatible_top_cv(then_base->inner, else_base->inner)) {
        result.type = then_type;
      } else if(then_base && else_base &&
                same_type_with_compatible_top_cv(else_base->inner, then_base->inner)) {
        result.type = else_type;
      } else if(!try_common_conditional_conversion()) {
        throw logic_error("unsupported conditional operands");
      }
    } else if(is_nullable_pointer_like_type(then_type) && is_null_pointer_expr(else_expr)) {
      else_expr = ctx.analyze_expression(scope, node.children[2]);
      result.type = then_type;
    } else if(is_nullable_pointer_like_type(else_type) && is_null_pointer_expr(then_expr)) {
      then_expr = ctx.analyze_expression(scope, node.children[1]);
      result.type = else_type;
    } else if(!try_common_conditional_conversion()) {
      throw logic_error("unsupported conditional operands");
    }
    if(result.category != VC_LVALUE && result.category != VC_XVALUE) {
      result.category = VC_PRVALUE;
    }
  }
  if(condition_known) {
    ExprInfo & selected_expr =
        condition_constant != 0 ? then_expr : else_expr;
    TypePtr selected_type =
        result.category == VC_LVALUE ?
            remove_reference_type(selected_expr.type) :
            value_conversion_type(selected_expr);
    if(type_equals(selected_type, result.type)) {
      selected_expr.type = result.type;
      selected_expr.category = result.category;
      set_expr_metadata(selected_expr.node, selected_expr.type, selected_expr.category);
      return std::move(selected_expr);
    }
  }
  result.node.children.push_back(std::move(condition.node));
  result.node.children.push_back(std::move(then_expr.node));
  result.node.children.push_back(std::move(else_expr.node));
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo analyze_binary_expression(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node)
{
  if(node.children.size() != 2) {
    throw logic_error("binary-expression arity");
  }

  const auto overloaded_operator_name = [&]() -> string
  {
    if(node_has_simple_type(node, OP_PLUS)) {
      return "operator+";
    }
    if(node_has_simple_type(node, OP_MINUS)) {
      return "operator-";
    }
    if(node_has_simple_type(node, OP_STAR)) {
      return "operator*";
    }
    if(node_has_simple_type(node, OP_DIV)) {
      return "operator/";
    }
    if(node_has_simple_type(node, OP_MOD)) {
      return "operator%";
    }
    if(node_has_simple_type(node, OP_BOR)) {
      return "operator|";
    }
    if(node_has_simple_type(node, OP_XOR)) {
      return "operator^";
    }
    if(node_has_simple_type(node, OP_AMP)) {
      return "operator&";
    }
    if(node_has_simple_type(node, OP_LSHIFT)) {
      return "operator<<";
    }
    if(node_has_simple_type(node, OP_RSHIFT)) {
      return "operator>>";
    }
    if(node_has_simple_type(node, OP_EQ)) {
      return "operator==";
    }
    if(node_has_simple_type(node, OP_NE)) {
      return "operator!=";
    }
    if(node_has_simple_type(node, OP_LT)) {
      return "operator<";
    }
    if(node_has_simple_type(node, OP_GT)) {
      return "operator>";
    }
    if(node_has_simple_type(node, OP_LE)) {
      return "operator<=";
    }
    if(node_has_simple_type(node, OP_GE)) {
      return "operator>=";
    }
    if(node_has_simple_type(node, OP_LAND)) {
      return "operator&&";
    }
    if(node_has_simple_type(node, OP_LOR)) {
      return "operator||";
    }
    if(node_has_simple_type(node, OP_COMMA)) {
      return "operator,";
    }
    if(node_has_simple_type(node, OP_ARROWSTAR)) {
      return "operator->*";
    }
    return string();
  };
  const string operator_name = overloaded_operator_name();
  ExprInfo lhs = ctx.analyze_expression(scope, node.children[0]);
  const OverloadableOperandInfo lhs_operand =
      classify_overloadable_operator_operand(ctx, lhs.type);
  bool deferred_operator_builtin_fallback = false;
  std::string deferred_operator_builtin_fallback_error;
  auto try_overloaded_binary_operator =
      [&](const ExprInfo * rhs_expr,
          const OverloadableOperandInfo * rhs_operand,
          ExprInfo & out,
          bool instantiate_bodies,
          vector<ConversionRank> * selected_ranks_out = nullptr) -> bool
      {
        if(operator_name.empty()) {
          return false;
        }

        const bool rhs_overloadable =
            rhs_expr && rhs_operand && rhs_operand->has_overloadable_operand;
        if(!lhs_operand.has_overloadable_operand && !rhs_overloadable) {
          return false;
        }

        ClassInfo * lhs_class =
            lhs_operand.has_class_operand ? lhs_operand.class_info : nullptr;

        vector<FunctionBinding *> member_operator_functions;
        MemberFunctionTemplateLookupResult member_template_candidates;
        const ClassInfo * member_declared_in = lhs_class;
        MemberAccess member_path_access = MA_PUBLIC;
        if(lhs_operand.has_class_operand && lhs_class) {
          member_template_candidates =
              lookup_visible_member_function_templates(*lhs_class, operator_name);
          if(member_template_candidates.declared_in == lhs_class &&
             !member_template_candidates.templates.empty()) {
            if(lhs_class->member_scope) {
              member_operator_functions =
                  lookup_direct_functions(*lhs_class->member_scope, operator_name);
            }
            member_declared_in = lhs_class;
            member_path_access = MA_PUBLIC;
          } else {
            MemberFunctionLookupResult member_candidates =
                lookup_visible_member_functions(*lhs_class, operator_name);
            member_operator_functions = member_candidates.functions;
            if(member_candidates.declared_in) {
              member_declared_in = member_candidates.declared_in;
              member_path_access = member_candidates.path_access;
            } else if(member_template_candidates.declared_in) {
              member_declared_in = member_template_candidates.declared_in;
              member_path_access = member_template_candidates.path_access;
            }
          }
        }

        vector<TypePtr> operator_operand_types;
        operator_operand_types.push_back(lhs.type);
        if(rhs_expr && rhs_expr->type) {
          operator_operand_types.push_back(rhs_expr->type);
        }
        semantic_overload::NonmemberOperatorCandidateSet operator_candidates;
        semantic_overload::collect_nonmember_operator_candidates(ctx,
                                                                 scope,
                                                                 operator_name,
                                                                 operator_operand_types,
                                                                 2,
                                                                 operator_candidates);
        vector<FunctionBinding *> operator_functions = operator_candidates.functions;
        vector<FunctionTemplateDecl *> operator_templates = operator_candidates.templates;
        append_unique_function_templates(operator_templates,
                                         member_template_candidates.templates);
        const bool enum_only_operator_lookup =
            !lhs_operand.has_class_operand &&
            !(rhs_expr && rhs_operand && rhs_operand->has_class_operand);
        const auto operator_param_is_non_enum_class_like =
            [&](const TypePtr & type) -> bool
            {
              TypePtr param = strip_top_level_cv(remove_reference_type(type));
              if(!param) {
                param = strip_top_level_cv(type);
              }
              ClassInfo * info =
                  param ? complete_class_type_for_lookup(ctx, param) : nullptr;
              if(info) {
                return info->class_kind != "enum";
              }
              param = strip_top_level_cv(param);
              if(!param ||
                 param->kind != Type::TK_NAMED ||
                 param->definitely_not_class ||
                 param->named_semantic_kind == Type::NSK_TEMPLATE_PARAMETER ||
                 param->named_semantic_kind == Type::NSK_PARTIAL_ORDER) {
                return false;
              }
              return ctx.type_depends_on_template_parameter(param);
            };
        if(enum_only_operator_lookup) {
          vector<FunctionBinding *> enum_operator_functions;
          for(size_t i = 0; i < operator_functions.size(); ++i) {
            FunctionBinding * fn = operator_functions[i];
            TypePtr function_type = fn ? strip_top_level_cv(fn->type) : TypePtr();
            bool has_non_enum_class_param = false;
            if(function_type && function_type->kind == Type::TK_FUNCTION) {
              for(size_t j = 0; j < function_type->params.size() && j < 2; ++j) {
                if(operator_param_is_non_enum_class_like(function_type->params[j])) {
                  has_non_enum_class_param = true;
                  break;
                }
              }
            }
            if(!has_non_enum_class_param) {
              enum_operator_functions.push_back(fn);
            }
          }
          operator_functions.swap(enum_operator_functions);
        }
        const vector<Scope *> & associated_scopes = operator_candidates.associated_scopes;

        const auto handle_unresolved_operator =
            [&](const logic_error & error) -> bool
            {
              const bool rhs_has_class_operand =
                  rhs_expr && rhs_operand && rhs_operand->has_class_operand;
              if(!lhs_operand.has_class_operand && !rhs_has_class_operand) {
                // Enum operands enter operator lookup for ADL, but builtin
                // arithmetic remains viable when no user operator matches.
                return false;
              }
              deferred_operator_builtin_fallback = true;
              deferred_operator_builtin_fallback_error = error.what();
              return false;
            };

        append_unique_functions(operator_functions, member_operator_functions);
        if(parser_trace::enabled("overload")) {
          ostringstream trace;
          trace << "binary-operator-candidates"
                << " op=" << operator_name
                << " lhs=" << describe_type(lhs.type)
                << " rhs="
                << (rhs_expr && rhs_expr->type ? describe_type(rhs_expr->type)
                                               : string("<pending>"))
                << " member_count=" << member_operator_functions.size()
                << " associated_scopes={";
          for(size_t i = 0; i < associated_scopes.size(); ++i) {
            if(i != 0) {
              trace << ", ";
            }
            trace << scope_qualified_name(*associated_scopes[i], "<null>");
          }
          trace << "}"
                << " function_count=" << operator_functions.size()
                << " template_count=" << operator_templates.size();
          parser_trace::note("overload", ctx.source_location_for_node(node), trace.str());
        }
        if(operator_functions.empty() && operator_templates.empty()) {
          return false;
        }

        CppAstNode callee;
        callee.kind = CppAstKind::id_expression;
        callee.value = operator_name;

        CppAstNode arguments;
        arguments.kind = CppAstKind::paren_argument_list;
        arguments.children.push_back(node.children[0]);
        arguments.children.push_back(node.children[1]);

        CppAstNode call;
        call.kind = CppAstKind::call_expression;
        call.children.push_back(callee);
        call.children.push_back(arguments);
        Scope operator_scope(&scope, "", false);
        semantic_overload::initialize_operator_candidate_scope(
            operator_scope,
            scope,
            operator_name,
            associated_scopes,
            operator_functions,
            operator_templates);
        semantic_overload::CallAnalysisHints hints;
        hints.use_location = ctx.source_location_for_node(node.children[1]);
        hints.explicit_member_base = &lhs;
        hints.explicit_member_arg_prefix = 1;
        hints.explicit_member_declared_in = member_declared_in;
        hints.explicit_member_path_access = member_path_access;
        hints.selected_ranks_out = selected_ranks_out;
        hints.suppress_user_defined_output_materialization = !instantiate_bodies;
        hints.adl_candidates_precollected = true;
        hints.args.push_back(&lhs);
        hints.args.push_back(rhs_expr);
        try
        {
          out = ctx.analyze_call_expression(
              operator_scope,
              call,
              semantic_overload::CallAnalysisOptions(instantiate_bodies, &hints));
          return true;
        }
        catch(const NoViableOverloadError & error)
        {
          return handle_unresolved_operator(error);
        }
        catch(const UnknownFunctionError & error)
        {
          return handle_unresolved_operator(error);
        }
      };

  const bool require_rhs_for_shift_overload =
      operator_name == "operator<<" || operator_name == "operator>>";

  ExprInfo overloaded_result;

  ExprInfo rhs;
  bool have_rhs = false;
  OverloadableOperandInfo rhs_operand;
  bool deferred_shift_rhs = false;
  std::string deferred_shift_rhs_error;
  if(require_rhs_for_shift_overload) {
    StreamShiftOverloadInput rhs_input =
        analyze_stream_shift_overload_input(ctx, scope, operator_name, node.children[1]);
    deferred_shift_rhs = rhs_input.have_deferred_error;
    deferred_shift_rhs_error = rhs_input.deferred_error;
    if(rhs_input.have_rhs) {
      rhs = rhs_input.rhs;
      rhs_operand = classify_overloadable_operator_operand(ctx, rhs.type);
      have_rhs = true;
    }
  } else {
    rhs = ctx.analyze_expression(scope, node.children[1]);
    rhs_operand = classify_overloadable_operator_operand(ctx, rhs.type);
    have_rhs = true;
  }

  const bool direct_rtti_object_comparison =
      (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
      lhs.node.kind == CallSemKind::typeid_expression &&
      rhs.node.kind == CallSemKind::typeid_expression;
  if(direct_rtti_object_comparison) {
    if(!type_info_comparison_operator_declared(ctx, lhs.type, operator_name)) {
      throw logic_error("typeid comparison requires declared std::type_info operator");
    }
    ExprInfo result;
    result.node = make_dump_node(CallSemKind::binary_expression, node.value);
    set_dump_token(result.node, node);
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
    result.node.children.push_back(std::move(lhs.node));
    result.node.children.push_back(std::move(rhs.node));
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  const auto try_builtin_user_defined_class_conversion =
      [&](const TypePtr & target,
          const ExprInfo & expr,
          bool expr_has_complete_class_type,
          ExprInfo & out_expr,
          ConversionRank & out_rank,
          const ArgumentConversionOptions & conversion_options) -> bool
      {
        if(!target || !expr.type || !expr_has_complete_class_type) {
          return false;
        }
        ExprInfo converted;
        ConversionRank rank = CR_BAD;
        bool converted_ok = false;
        try
        {
          converted_ok = ctx.try_argument_conversion(scope,
                                                     target,
                                                     expr,
                                                     converted,
                                                     rank,
                                                     conversion_options);
        }
        catch(const logic_error &)
        {
          converted_ok = false;
        }
        if(!converted_ok) {
          return false;
        }
        TypePtr converted_type = value_conversion_type(converted);
        if(!is_non_class_builtin_binary_type(ctx, converted_type)) {
          return false;
        }
        out_expr = converted;
        out_rank = rank;
        return true;
      };

  const auto try_builtin_binary_class_conversions =
      [&](ExprInfo & lhs_expr,
          ExprInfo & rhs_expr,
          vector<ConversionRank> * ranks_out,
          const ArgumentConversionOptions & conversion_options) -> bool
      {
        if(ranks_out) {
          ranks_out->clear();
        }
        TypePtr lhs_base = value_conversion_type(lhs_expr);
        TypePtr rhs_base = value_conversion_type(rhs_expr);
        const bool lhs_class = complete_class_type_for_lookup(ctx, lhs_base) != nullptr;
        const bool rhs_class = complete_class_type_for_lookup(ctx, rhs_base) != nullptr;
        if(!lhs_class && !rhs_class) {
          return false;
        }

        const auto try_convert_class_operand_for_pointer_arithmetic =
            [&](const ExprInfo & class_expr,
                bool class_complete,
                const ExprInfo & pointer_expr,
                bool class_is_lhs,
                ExprInfo & converted_out,
                ConversionRank & converted_rank_out) -> bool
            {
              TypePtr pointer_base = value_conversion_type(pointer_expr);
              if(!is_complete_object_pointer_type(ctx, pointer_base)) {
                return false;
              }
              if(!node_has_simple_type(node, OP_PLUS) &&
                 !(node_has_simple_type(node, OP_MINUS) && !class_is_lhs)) {
                return false;
              }

              struct Candidate
              {
                ExprInfo expr;
                ConversionRank rank = CR_BAD;
              };

              std::vector<Candidate> candidates;
              const std::vector<TypePtr> targets = builtin_numeric_conversion_targets();
              for(size_t i = 0; i < targets.size(); ++i) {
                if(!is_integral_or_unscoped_enum_type(targets[i])) {
                  continue;
                }
                Candidate candidate;
                if(!try_builtin_user_defined_class_conversion(targets[i],
                                                              class_expr,
                                                              class_complete,
                                                              candidate.expr,
                                                              candidate.rank,
                                                              conversion_options)) {
                  continue;
                }
                if(!is_integral_or_unscoped_enum_type(
                       value_conversion_type(candidate.expr))) {
                  continue;
                }
                candidates.push_back(candidate);
              }
              if(candidates.empty()) {
                return false;
              }

              size_t best = 0;
              bool ambiguous = false;
              for(size_t i = 1; i < candidates.size(); ++i) {
                if(candidates[i].rank < candidates[best].rank) {
                  best = i;
                  ambiguous = false;
                } else if(candidates[i].rank == candidates[best].rank &&
                          !type_equals(value_conversion_type(candidates[i].expr),
                                       value_conversion_type(candidates[best].expr))) {
                  ambiguous = true;
                }
              }
              if(ambiguous) {
                return false;
              }

              converted_out = candidates[best].expr;
              converted_rank_out = candidates[best].rank;
              return true;
            };

        const auto try_convert_pointer_class_operand_for_arithmetic =
            [&](const ExprInfo & class_expr,
                bool class_complete,
                const ExprInfo & integral_expr,
                bool class_is_lhs,
                ExprInfo & converted_out,
                ConversionRank & converted_rank_out) -> bool
            {
              if(!class_complete ||
                 !is_integral_or_unscoped_enum_type(
                     value_conversion_type(integral_expr))) {
                return false;
              }
              if(!node_has_simple_type(node, OP_PLUS) &&
                 !(node_has_simple_type(node, OP_MINUS) && class_is_lhs)) {
                return false;
              }

              TypePtr converted_pointer_type;
              if(!try_builtin_pointer_operand_conversion(
                     ctx,
                     scope,
                     class_expr,
                     converted_out,
                     converted_pointer_type,
                     conversion_options) ||
                 !is_complete_object_pointer_type(ctx, converted_pointer_type)) {
                return false;
              }
              converted_rank_out = CR_USER_DEFINED;
              return true;
            };

        const auto try_convert_single_class_operand =
            [&](const ExprInfo & class_expr,
                bool class_complete,
                const ExprInfo & other_expr,
                ExprInfo & converted_out,
                ConversionRank & converted_rank_out,
                TypePtr & target_out) -> bool
            {
              TypePtr other_base = value_conversion_type(other_expr);
              ConversionRank rank = CR_BAD;
              if(try_builtin_user_defined_class_conversion(other_base,
                                                           class_expr,
                                                           class_complete,
                                                           converted_out,
                                                           rank,
                                                           conversion_options)) {
                converted_rank_out = rank;
                target_out = other_base;
                return true;
              }

              if(is_pointer_type(other_base)) {
                TypePtr converted_pointer_type;
                if(try_builtin_pointer_operand_conversion(
                       ctx,
                       scope,
                       class_expr,
                       converted_out,
                       converted_pointer_type,
                       conversion_options) &&
                   standard_conversion_rank(converted_pointer_type,
                                            other_expr) != CR_BAD) {
                  converted_rank_out = CR_USER_DEFINED;
                  target_out = converted_pointer_type;
                  return true;
                }
              }

              TypePtr probe = TypePtr();
              if(is_integral_or_unscoped_enum_type(other_base)) {
                probe = promoted_integral_result_type(other_base);
              } else if(is_floating_type(other_base)) {
                probe = strip_top_level_cv(other_base);
              }
              if(!probe ||
                 type_equals(strip_top_level_cv(other_base), strip_top_level_cv(probe))) {
                return false;
              }
              rank = CR_BAD;
              if(!try_builtin_user_defined_class_conversion(probe,
                                                            class_expr,
                                                            class_complete,
                                                            converted_out,
                                                            rank,
                                                            conversion_options)) {
                return false;
              }
              converted_rank_out = rank;
              target_out = probe;
              return true;
            };

        if(lhs_class != rhs_class) {
          ExprInfo converted;
          ConversionRank converted_rank = CR_BAD;
          TypePtr target;
          if(lhs_class &&
             try_convert_pointer_class_operand_for_arithmetic(lhs_expr,
                                                               lhs_class,
                                                               rhs_expr,
                                                               true,
                                                               converted,
                                                               converted_rank)) {
            lhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(converted_rank);
              ranks_out->push_back(CR_EXACT);
            }
            return true;
          } else if(rhs_class &&
                    try_convert_pointer_class_operand_for_arithmetic(rhs_expr,
                                                                      rhs_class,
                                                                      lhs_expr,
                                                                      false,
                                                                      converted,
                                                                      converted_rank)) {
            rhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(CR_EXACT);
              ranks_out->push_back(converted_rank);
            }
            return true;
          } else if(lhs_class &&
             try_convert_class_operand_for_pointer_arithmetic(lhs_expr,
                                                              lhs_class,
                                                              rhs_expr,
                                                              true,
                                                              converted,
                                                              converted_rank)) {
            lhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(converted_rank);
              ranks_out->push_back(CR_EXACT);
            }
            return true;
          } else if(rhs_class &&
                    try_convert_class_operand_for_pointer_arithmetic(rhs_expr,
                                                                     rhs_class,
                                                                     lhs_expr,
                                                                     false,
                                                                     converted,
                                                                     converted_rank)) {
            rhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(CR_EXACT);
              ranks_out->push_back(converted_rank);
            }
            return true;
          } else if(lhs_class &&
                    try_convert_single_class_operand(lhs_expr,
                                                     lhs_class,
                                                     rhs_expr,
                                                     converted,
                                                     converted_rank,
                                                     target)) {
            ConversionRank other_rank = standard_conversion_rank(target, rhs_expr);
            if(other_rank == CR_BAD) {
              return false;
            }
            lhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(converted_rank);
              ranks_out->push_back(other_rank);
            }
            return true;
          } else if(rhs_class &&
                    try_convert_single_class_operand(rhs_expr,
                                                     rhs_class,
                                                     lhs_expr,
                                                     converted,
                                                     converted_rank,
                                                     target)) {
            ConversionRank other_rank = standard_conversion_rank(target, lhs_expr);
            if(other_rank == CR_BAD) {
              return false;
            }
            rhs_expr = converted;
            if(ranks_out) {
              ranks_out->push_back(other_rank);
              ranks_out->push_back(converted_rank);
            }
            return true;
          }
          return false;
        }

        struct Candidate
        {
          ExprInfo lhs;
          ExprInfo rhs;
          ConversionRank lhs_rank = CR_BAD;
          ConversionRank rhs_rank = CR_BAD;
        };

        std::vector<Candidate> candidates;
        std::vector<TypePtr> targets = builtin_numeric_conversion_targets();
        const bool pointer_comparison =
            node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE) ||
            node_has_simple_type(node, OP_LT) || node_has_simple_type(node, OP_GT) ||
            node_has_simple_type(node, OP_LE) || node_has_simple_type(node, OP_GE);
        if(node_has_simple_type(node, OP_MINUS) || pointer_comparison) {
          const auto append_pointer_conversion_target =
              [&](const ExprInfo & expr)
              {
                ExprInfo converted;
                TypePtr pointer_type;
                if(!try_builtin_pointer_operand_conversion(
                       ctx,
                       scope,
                       expr,
                       converted,
                       pointer_type,
                       conversion_options)) {
                  return;
                }
                for(size_t i = 0; i < targets.size(); ++i) {
                  if(type_equals(targets[i], pointer_type)) {
                    return;
                  }
                }
                targets.push_back(pointer_type);
              };
          append_pointer_conversion_target(lhs_expr);
          append_pointer_conversion_target(rhs_expr);
        }
        if(pointer_comparison) {
          targets.push_back(builtin_common_object_pointer_target());
        }
        for(size_t i = 0; i < targets.size(); ++i) {
          Candidate candidate;
          if(!try_builtin_user_defined_class_conversion(targets[i],
                                                        lhs_expr,
                                                        lhs_class,
                                                        candidate.lhs,
                                                        candidate.lhs_rank,
                                                        conversion_options) ||
             !try_builtin_user_defined_class_conversion(targets[i],
                                                        rhs_expr,
                                                        rhs_class,
                                                        candidate.rhs,
                                                        candidate.rhs_rank,
                                                        conversion_options)) {
            continue;
          }
          bool duplicate = false;
          for(size_t j = 0; j < candidates.size(); ++j) {
            if(candidates[j].lhs_rank == candidate.lhs_rank &&
               candidates[j].rhs_rank == candidate.rhs_rank &&
               type_equals(value_conversion_type(candidates[j].lhs),
                           value_conversion_type(candidate.lhs)) &&
               type_equals(value_conversion_type(candidates[j].rhs),
                           value_conversion_type(candidate.rhs))) {
              duplicate = true;
              break;
            }
          }
          if(duplicate) {
            continue;
          }
          candidates.push_back(candidate);
        }
        if(candidates.empty()) {
          return false;
        }

        size_t best = 0;
        bool ambiguous = false;
        for(size_t i = 1; i < candidates.size(); ++i) {
          const int lhs_cmp = (int)candidates[i].lhs_rank - (int)candidates[best].lhs_rank;
          const int rhs_cmp = (int)candidates[i].rhs_rank - (int)candidates[best].rhs_rank;
          const int current_worst =
              std::max((int)candidates[i].lhs_rank, (int)candidates[i].rhs_rank);
          const int best_worst =
              std::max((int)candidates[best].lhs_rank, (int)candidates[best].rhs_rank);
          const int current_sum =
              (int)candidates[i].lhs_rank + (int)candidates[i].rhs_rank;
          const int best_sum =
              (int)candidates[best].lhs_rank + (int)candidates[best].rhs_rank;
          if(current_worst < best_worst ||
             (current_worst == best_worst && current_sum < best_sum) ||
             (current_worst == best_worst && current_sum == best_sum &&
              lhs_cmp <= 0 && rhs_cmp <= 0 &&
              (lhs_cmp < 0 || rhs_cmp < 0))) {
            best = i;
            ambiguous = false;
          } else if(current_worst == best_worst && current_sum == best_sum &&
                    lhs_cmp == 0 && rhs_cmp == 0) {
            ambiguous = true;
          }
        }
        if(ambiguous) {
          return false;
        }

        lhs_expr = candidates[best].lhs;
        rhs_expr = candidates[best].rhs;
        if(ranks_out) {
          ranks_out->push_back(candidates[best].lhs_rank);
          ranks_out->push_back(candidates[best].rhs_rank);
        }
        return true;
      };

  const auto compare_binary_conversion_rank_preference =
      [](const vector<ConversionRank> & current,
         const vector<ConversionRank> & best) -> int
      {
        if(current.size() != 2 || best.size() != 2) {
          return 0;
        }
        bool current_better = false;
        bool best_better = false;
        for(size_t i = 0; i < 2; ++i) {
          if(current[i] < best[i]) {
            current_better = true;
          } else if(current[i] > best[i]) {
            best_better = true;
          }
        }
        if(current_better && !best_better) {
          return -1;
        }
        if(best_better && !current_better) {
          return 1;
        }
        return 0;
      };

  const auto builtin_binary_operator_supported =
      [&](const ExprInfo & lhs_expr, const ExprInfo & rhs_expr) -> bool
      {
        TypePtr lhs_type = value_conversion_type(lhs_expr);
        TypePtr rhs_type = value_conversion_type(rhs_expr);
        if(!lhs_type || !rhs_type) {
          return false;
        }

        const bool lhs_integral_like = is_integral_or_unscoped_enum_type(lhs_type);
        const bool rhs_integral_like = is_integral_or_unscoped_enum_type(rhs_type);
        if(node_has_simple_type(node, OP_MINUS) &&
           is_complete_object_pointer_type(ctx, lhs_type) &&
           pointer_subtraction_operands_compatible(lhs_type, rhs_type)) {
          return true;
        }
        if(node_has_simple_type(node, OP_PLUS)) {
          return (is_complete_object_pointer_type(ctx, lhs_type) &&
                  is_integral_or_unscoped_enum_type(rhs_type)) ||
                 (is_integral_or_unscoped_enum_type(lhs_type) &&
                  is_complete_object_pointer_type(ctx, rhs_type)) ||
                 ((lhs_integral_like || is_floating_type(lhs_type)) &&
                  (rhs_integral_like || is_floating_type(rhs_type)));
        }
        if(node_has_simple_type(node, OP_MINUS)) {
          return (is_complete_object_pointer_type(ctx, lhs_type) &&
                  is_integral_or_unscoped_enum_type(rhs_type)) ||
                 ((lhs_integral_like || is_floating_type(lhs_type)) &&
                  (rhs_integral_like || is_floating_type(rhs_type)));
        }
        if(node_has_simple_type(node, OP_STAR) || node_has_simple_type(node, OP_DIV)) {
          return (lhs_integral_like || is_floating_type(lhs_type)) &&
                 (rhs_integral_like || is_floating_type(rhs_type));
        }
        if(node_has_simple_type(node, OP_MOD) ||
           node_has_simple_type(node, OP_BOR) ||
           node_has_simple_type(node, OP_XOR) ||
           node_has_simple_type(node, OP_AMP) ||
           node_has_simple_type(node, OP_LSHIFT) ||
           node_has_simple_type(node, OP_RSHIFT)) {
          return lhs_integral_like && rhs_integral_like;
        }
        if(node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE) ||
           node_has_simple_type(node, OP_LT) || node_has_simple_type(node, OP_GT) ||
           node_has_simple_type(node, OP_LE) || node_has_simple_type(node, OP_GE)) {
          const auto is_null_pointer_constant_for_syntax =
              [&](const ExprInfo & expr, const CppAstNode & syntax) -> bool
              {
                if(expr.null_pointer_constant) {
                  return true;
                }
                if(!expr.type || !is_integral_type(expr.type)) {
                  return false;
                }
                long long value = 0;
                return ctx.evaluate_constant_expression(scope, syntax, value) &&
                       value == 0;
              };
          const bool pointer_null_constant_comparison =
              (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
              ((is_nullable_pointer_like_type(lhs_type) &&
                is_null_pointer_constant_for_syntax(rhs_expr, node.children[1])) ||
               (is_nullable_pointer_like_type(rhs_type) &&
                is_null_pointer_constant_for_syntax(lhs_expr, node.children[0])));
          const bool nullptr_equality_comparison =
              (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
              is_nullptr_type(lhs_type) &&
              is_nullptr_type(rhs_type);
          return pointer_equality_operands_compatible(lhs_type, rhs_type) ||
                 pointer_class_hierarchy_equality_compatible(ctx, lhs_type, rhs_type) ||
                 pointer_null_constant_comparison ||
                 nullptr_equality_comparison ||
                 ((is_integral_type(lhs_type) || is_floating_type(lhs_type) ||
                   is_named_enum_type(ctx, lhs_type)) &&
                  (is_integral_type(rhs_type) || is_floating_type(rhs_type) ||
                   is_named_enum_type(ctx, rhs_type)));
        }
        if(node_has_simple_type(node, OP_LAND) || node_has_simple_type(node, OP_LOR)) {
          ExprInfo lhs_condition = lhs_expr;
          ExprInfo rhs_condition = rhs_expr;
          return try_condition_test_conversion(ctx, scope, lhs_condition) &&
                 try_condition_test_conversion(ctx, scope, rhs_condition);
        }
        if(node_has_simple_type(node, OP_COMMA)) {
          return true;
        }
        return false;
      };

  bool selected_builtin_class_conversion = false;
  if(have_rhs) {
    const bool preserve_builtin_operands =
        node_has_simple_type(node, OP_COMMA) ||
        node_has_simple_type(node, OP_LAND) ||
        node_has_simple_type(node, OP_LOR);
    ArgumentConversionOptions builtin_probe_options =
        semantic_policy::without_user_defined_body_instantiation();
    builtin_probe_options.materialize_user_defined_output = false;
    builtin_probe_options.materialize_standard_adjustments = false;

    ExprInfo builtin_lhs_probe = lhs;
    ExprInfo builtin_rhs_probe = rhs;
    vector<ConversionRank> builtin_ranks;
    const bool builtin_probe_ok =
        (preserve_builtin_operands ||
         try_builtin_binary_class_conversions(builtin_lhs_probe,
                                              builtin_rhs_probe,
                                              &builtin_ranks,
                                              builtin_probe_options)) &&
        builtin_binary_operator_supported(builtin_lhs_probe, builtin_rhs_probe);

    if(builtin_probe_ok) {
      ExprInfo overloaded_probe;
      vector<ConversionRank> overloaded_ranks;
      if(try_overloaded_binary_operator(have_rhs ? &rhs : nullptr,
                                        have_rhs ? &rhs_operand : nullptr,
                                        overloaded_probe,
                                        false,
                                        &overloaded_ranks)) {
        const bool builtin_is_better =
            compare_binary_conversion_rank_preference(builtin_ranks,
                                                      overloaded_ranks) < 0;
        if(builtin_is_better) {
          if(parser_trace::enabled("overload")) {
            ostringstream trace;
            trace << "binary-operator-builtin-candidate-selected"
                  << " op=" << operator_name
                  << " builtin_ranks={";
            for(size_t i = 0; i < builtin_ranks.size(); ++i) {
              if(i != 0) {
                trace << ",";
              }
              trace << static_cast<int>(builtin_ranks[i]);
            }
            trace << "} overloaded_ranks={";
            for(size_t i = 0; i < overloaded_ranks.size(); ++i) {
              if(i != 0) {
                trace << ",";
              }
              trace << static_cast<int>(overloaded_ranks[i]);
            }
            trace << "}";
            parser_trace::note("overload",
                               ctx.source_location_for_node(node),
                               trace.str());
          }
          if(!preserve_builtin_operands &&
             !try_builtin_binary_class_conversions(
                 lhs,
                 rhs,
                 nullptr,
                 semantic_policy::default_argument_conversion())) {
            throw logic_error("failed to materialize selected builtin conversions");
          }
          selected_builtin_class_conversion = true;
        } else {
          if(!try_overloaded_binary_operator(have_rhs ? &rhs : nullptr,
                                             have_rhs ? &rhs_operand : nullptr,
                                             overloaded_result,
                                             true)) {
            throw logic_error("failed to materialize selected overloaded operator");
          }
          return overloaded_result;
        }
      } else if(!deferred_shift_rhs) {
        if(!preserve_builtin_operands &&
           !try_builtin_binary_class_conversions(
               lhs,
               rhs,
               nullptr,
               semantic_policy::default_argument_conversion())) {
          throw logic_error("failed to materialize selected builtin conversions");
        }
        selected_builtin_class_conversion = true;
      }
    } else if(try_overloaded_binary_operator(have_rhs ? &rhs : nullptr,
                                             have_rhs ? &rhs_operand : nullptr,
                                             overloaded_result,
                                             true)) {
      return overloaded_result;
    }
  } else if(try_overloaded_binary_operator(have_rhs ? &rhs : nullptr,
                                           have_rhs ? &rhs_operand : nullptr,
                                           overloaded_result,
                                           true)) {
    return overloaded_result;
  }

  if(!selected_builtin_class_conversion) {
    if(deferred_shift_rhs) {
      throw logic_error(deferred_shift_rhs_error);
    }
    if(!node_has_simple_type(node, OP_COMMA) &&
       !node_has_simple_type(node, OP_LAND) &&
       !node_has_simple_type(node, OP_LOR)) {
      try_builtin_binary_class_conversions(
          lhs,
          rhs,
          nullptr,
          semantic_policy::default_argument_conversion());
    }
  }

  const auto has_non_enum_class_value_type =
      [&](const ExprInfo & expr) -> bool
      {
        ClassInfo * info =
            complete_class_type_for_lookup(ctx, value_conversion_type(expr));
        return info && info->class_kind != "enum";
      };
  const bool legal_builtin_comma_fallback =
      deferred_operator_builtin_fallback &&
      node_has_simple_type(node, OP_COMMA) &&
      builtin_binary_operator_supported(lhs, rhs);
  if(deferred_operator_builtin_fallback &&
     (has_non_enum_class_value_type(lhs) || has_non_enum_class_value_type(rhs)) &&
     !builtin_binary_operator_supported(lhs, rhs)) {
    throw logic_error(deferred_operator_builtin_fallback_error);
  }
  if(deferred_operator_builtin_fallback &&
     !selected_builtin_class_conversion &&
     (has_non_enum_class_value_type(lhs) || has_non_enum_class_value_type(rhs)) &&
     !legal_builtin_comma_fallback) {
    if(parser_trace::enabled("overload")) {
      ostringstream trace;
      trace << "binary-operator-fallback-check"
            << " op=" << operator_name
            << " lhs=" << describe_type(lhs.type)
            << " rhs=" << describe_type(rhs.type)
            << " error=" << deferred_operator_builtin_fallback_error;
      parser_trace::note("overload", ctx.source_location_for_node(node), trace.str());
    }
    hard_fail_semantic_fallback(
        ctx,
        node,
        "operator-overload-to-builtin",
        "binary operator overload attempt fell back to builtin [op " +
            operator_name + "] [lhs " + describe_type(lhs.type) +
            "] [rhs " + describe_type(rhs.type) + "] [lhs_value " +
            describe_type(value_conversion_type(lhs)) + "] [rhs_value " +
            describe_type(value_conversion_type(rhs)) + "] [error " +
            deferred_operator_builtin_fallback_error + "]");
  }

  TypePtr lhs_type = value_conversion_type(lhs);
  TypePtr rhs_type = value_conversion_type(rhs);
  if(!lhs_type || !rhs_type) {
    throw logic_error("unsupported builtin conversion");
  }

  ExprInfo member_pointer_data_access;
  if(try_analyze_member_pointer_data_access(ctx,
                                            scope,
                                            node,
                                            lhs,
                                            rhs,
                                            member_pointer_data_access)) {
    return member_pointer_data_access;
  }

  const bool lhs_integral_like = is_integral_or_unscoped_enum_type(lhs_type);
  const bool rhs_integral_like = is_integral_or_unscoped_enum_type(rhs_type);

  ExprInfo result;
  result.node = make_dump_node(CallSemKind::binary_expression, node.value);
  set_dump_token(result.node, node);
  const auto is_null_pointer_constant =
      [&](const ExprInfo & expr, const CppAstNode & syntax) -> bool
      {
        if(expr.null_pointer_constant) {
          return true;
        }
        if(!expr.type || !is_integral_type(expr.type)) {
          return false;
        }
        long long value = 0;
        return ctx.evaluate_constant_expression(scope, syntax, value) && value == 0;
      };
  const bool pointer_null_constant_comparison =
      (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
      ((is_nullable_pointer_like_type(lhs_type) &&
        is_null_pointer_constant(rhs, node.children[1])) ||
       (is_nullable_pointer_like_type(rhs_type) &&
        is_null_pointer_constant(lhs, node.children[0])));
  const bool nullptr_equality_comparison =
      (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
      is_nullptr_type(lhs_type) &&
      is_nullptr_type(rhs_type);
  const bool rtti_object_comparison =
      (node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) &&
      lhs.node.kind == CallSemKind::typeid_expression &&
      rhs.node.kind == CallSemKind::typeid_expression;
  const bool pointer_class_hierarchy_comparison =
      pointer_class_hierarchy_equality_compatible(ctx, lhs_type, rhs_type);
  if(pointer_class_hierarchy_comparison &&
     !try_apply_pointer_class_hierarchy_comparison_conversion(ctx, lhs, rhs)) {
    throw logic_error("failed pointer class hierarchy comparison conversion");
  }
  if(pointer_class_hierarchy_comparison) {
    lhs_type = value_conversion_type(lhs);
    rhs_type = value_conversion_type(rhs);
  }

  if(node_has_simple_type(node, OP_MINUS) &&
     is_complete_object_pointer_type(ctx, lhs_type) &&
     pointer_subtraction_operands_compatible(lhs_type, rhs_type)) {
    result.type = make_fundamental(FT_LONG_INT);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_PLUS) &&
            ((is_complete_object_pointer_type(ctx, lhs_type) &&
              is_integral_or_unscoped_enum_type(rhs_type)) ||
             (is_integral_or_unscoped_enum_type(lhs_type) &&
              is_complete_object_pointer_type(ctx, rhs_type)))) {
    result.type = is_pointer_type(lhs_type) ? lhs_type : rhs_type;
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_MINUS) &&
            is_complete_object_pointer_type(ctx, lhs_type) &&
            is_integral_or_unscoped_enum_type(rhs_type)) {
    result.type = lhs_type;
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_PLUS) || node_has_simple_type(node, OP_MINUS) ||
            node_has_simple_type(node, OP_STAR) || node_has_simple_type(node, OP_DIV) ||
            node_has_simple_type(node, OP_MOD)) {
    if(node_has_simple_type(node, OP_MOD)) {
      if(!lhs_integral_like || !rhs_integral_like) {
        throw logic_error("unsupported arithmetic operands");
      }
      result.type = common_integral_result_type(lhs_type, rhs_type);
      result.category = VC_PRVALUE;
    } else if(!(lhs_integral_like || is_floating_type(lhs_type)) ||
              !(rhs_integral_like || is_floating_type(rhs_type))) {
      throw logic_error("unsupported arithmetic operands");
    } else {
      result.type = common_arithmetic_result_type(lhs_type, rhs_type);
      result.category = VC_PRVALUE;
    }
  } else if(node_has_simple_type(node, OP_BOR) || node_has_simple_type(node, OP_XOR) ||
            node_has_simple_type(node, OP_AMP)) {
    if(!lhs_integral_like || !rhs_integral_like) {
      throw logic_error("unsupported bitwise operands");
    }
    result.type = common_integral_result_type(lhs_type, rhs_type);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_LSHIFT) || node_has_simple_type(node, OP_RSHIFT)) {
    if(!lhs_integral_like || !rhs_integral_like) {
      throw logic_error("unsupported shift operands");
    }
    result.type = promoted_integral_result_type(lhs_type);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE) ||
            node_has_simple_type(node, OP_LT) || node_has_simple_type(node, OP_GT) ||
            node_has_simple_type(node, OP_LE) || node_has_simple_type(node, OP_GE)) {
    if(rtti_object_comparison) {
      result.type = make_fundamental(FT_BOOL);
      result.category = VC_PRVALUE;
    } else if((pointer_equality_operands_compatible(lhs_type, rhs_type) ||
        pointer_class_hierarchy_comparison ||
        pointer_null_constant_comparison) &&
       ((node_has_simple_type(node, OP_EQ) || node_has_simple_type(node, OP_NE)) ||
        (node_has_simple_type(node, OP_LT) || node_has_simple_type(node, OP_GT) ||
         node_has_simple_type(node, OP_LE) || node_has_simple_type(node, OP_GE)))) {
      result.type = make_fundamental(FT_BOOL);
      result.category = VC_PRVALUE;
    } else if(nullptr_equality_comparison) {
      result.type = make_fundamental(FT_BOOL);
      result.category = VC_PRVALUE;
    } else if((is_integral_type(lhs_type) || is_floating_type(lhs_type) ||
               is_named_enum_type(ctx, lhs_type)) &&
              (is_integral_type(rhs_type) || is_floating_type(rhs_type) ||
               is_named_enum_type(ctx, rhs_type))) {
      result.type = make_fundamental(FT_BOOL);
      result.category = VC_PRVALUE;
    } else {
      ostringstream out;
      out << "unsupported comparison operands";
      if(lhs_type) {
        out << " lhs=" << describe_type(lhs_type);
      }
      if(rhs_type) {
        out << " rhs=" << describe_type(rhs_type);
      }
      out << " expr=" << node_text(node);
      throw logic_error(out.str());
    }
  } else if(node_has_simple_type(node, OP_LAND) || node_has_simple_type(node, OP_LOR)) {
    ExprInfo lhs_condition = lhs;
    ExprInfo rhs_condition = rhs;
    if(!try_condition_test_conversion(ctx, scope, lhs_condition) ||
       !try_condition_test_conversion(ctx, scope, rhs_condition)) {
      throw logic_error("unsupported logical operands");
    }
    lhs = lhs_condition;
    rhs = rhs_condition;
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
  } else if(node_has_simple_type(node, OP_COMMA)) {
    result.type = rhs.type;
    result.category = rhs.category;
  } else {
    throw logic_error("unsupported binary operator");
  }

  result.node.children.push_back(std::move(lhs.node));
  result.node.children.push_back(std::move(rhs.node));
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

ExprInfo analyze_subscript_expression(SemanticContext & ctx,
                                      Scope & scope,
                                      const CppAstNode & node)
{
  if(node.children.size() != 2) {
    throw logic_error("subscript-expression arity");
  }

  ExprInfo base = ctx.analyze_expression(scope, node.children[0]);
  ExprInfo index = ctx.analyze_expression(scope, node.children[1]);
  TypePtr base_type = value_conversion_type(base);
  TypePtr index_type = value_conversion_type(index);
  if(has_overloadable_operator_operand(ctx, base.type)) {
    TypePtr base_class_type = overloaded_operator_operand_base_type(base.type);
    ClassInfo * base_class = complete_class_type_for_lookup(ctx, base_class_type);
    if(!base_class) {
      base_class = ctx.class_info_for_type(base_class_type);
    }
    if(base_class) {
      const bool has_member_operator =
          !lookup_visible_member_functions(*base_class, "operator[]").functions.empty();
      const bool has_member_operator_templates =
          !lookup_visible_member_function_templates(*base_class, "operator[]")
               .templates.empty();
      if(has_member_operator || has_member_operator_templates) {
        CppAstNode member_callee;
        member_callee.kind = CppAstKind::member_expression;
        member_callee.has_token = true;
        member_callee.token_kind = RT_SIMPLE;
        member_callee.simple_type = OP_DOT;
        member_callee.value = ".";
        member_callee.children.push_back(node.children[0]);
        member_callee.children.push_back(make_operator_identifier_node("operator[]"));

        CppAstNode arguments;
        arguments.kind = CppAstKind::paren_argument_list;
        arguments.children.push_back(node.children[1]);

        CppAstNode call;
        call.kind = CppAstKind::call_expression;
        call.children.push_back(member_callee);
        call.children.push_back(arguments);
        return ctx.analyze_call_expression(scope, call);
      }
    }
  }
  ExprInfo pointer_operand;
  ExprInfo index_operand;
  TypePtr pointer_type;
  TypePtr base_pointer_type = subscript_pointer_operand_type(base);
  TypePtr index_pointer_type = subscript_pointer_operand_type(index);
  bool base_is_subscript_index =
      base_type && is_integral_or_unscoped_enum_type(base_type);
  bool index_is_subscript_index =
      index_type && is_integral_or_unscoped_enum_type(index_type);
  const auto try_convert_subscript_index =
      [&](ExprInfo & operand, bool & is_subscript_index) -> bool
      {
        if(is_subscript_index) {
          return true;
        }
        if(complete_class_type_for_lookup(ctx, value_conversion_type(operand)) == nullptr) {
          return false;
        }

        ExprInfo converted;
        ConversionRank rank = CR_BAD;
        if(!ctx.try_argument_conversion(scope,
                                        make_fundamental(FT_LONG_INT),
                                        operand,
                                        converted,
                                        rank,
                                        semantic_policy::default_argument_conversion()) ||
           !is_integral_or_unscoped_enum_type(value_conversion_type(converted))) {
          return false;
        }
        operand = converted;
        is_subscript_index = true;
        return true;
      };

  if(base_pointer_type &&
     !index_is_subscript_index &&
     try_convert_subscript_index(index, index_is_subscript_index)) {
    index_type = value_conversion_type(index);
  }
  if(index_pointer_type &&
     !base_is_subscript_index &&
     try_convert_subscript_index(base, base_is_subscript_index)) {
    base_type = value_conversion_type(base);
  }
  if(!base_pointer_type && index_is_subscript_index) {
    ExprInfo converted_base;
    TypePtr converted_pointer_type;
    if(try_builtin_pointer_operand_conversion(ctx,
                                              scope,
                                              base,
                                              converted_base,
                                              converted_pointer_type)) {
      base = converted_base;
      base_pointer_type = converted_pointer_type;
      base_type = value_conversion_type(base);
      base_is_subscript_index =
          base_type && is_integral_or_unscoped_enum_type(base_type);
    }
  }
  if(!index_pointer_type && base_is_subscript_index) {
    ExprInfo converted_index;
    TypePtr converted_pointer_type;
    if(try_builtin_pointer_operand_conversion(ctx,
                                              scope,
                                              index,
                                              converted_index,
                                              converted_pointer_type)) {
      index = converted_index;
      index_pointer_type = converted_pointer_type;
      index_type = value_conversion_type(index);
      index_is_subscript_index =
          index_type && is_integral_or_unscoped_enum_type(index_type);
    }
  }

  const bool base_is_pointer = static_cast<bool>(base_pointer_type);
  const bool index_is_pointer = static_cast<bool>(index_pointer_type);
  if(base_is_pointer && index_is_subscript_index) {
    pointer_operand = base;
    index_operand = index;
    pointer_type = base_pointer_type;
  } else if(index_is_pointer && base_is_subscript_index) {
    pointer_operand = index;
    index_operand = base;
    pointer_type = index_pointer_type;
  } else {
    throw logic_error("unsupported subscript operands");
  }

  ExprInfo result;
  result.type = pointer_type->inner;
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::subscript_expression);
  set_expr_metadata(result.node, result.type, result.category);
  result.node.children.push_back(std::move(pointer_operand.node));
  result.node.children.push_back(std::move(index_operand.node));
  return result;
}

ExprInfo analyze_sizeof_expression(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node)
{
  if(node.children.size() != 1) {
    throw logic_error("sizeof-expression arity");
  }

  TypePtr operand_type;
  const CppAstNode & child = node.children[0];
  if(child.kind == CppAstKind::type_id) {
    bool parsed_type_id = ctx.parse_type_id(scope, child, operand_type) &&
                          static_cast<bool>(operand_type);
    if(parsed_type_id) {
      prepare_sizeof_operand_type(ctx, operand_type);
      if(type_is_valid_sizeof_operand(operand_type)) {
        record_sizeof_type_id_class_use_if_needed(ctx,
                                                  scope,
                                                  child,
                                                  operand_type);
      }
    }
    if(!parsed_type_id || !type_is_valid_sizeof_operand(operand_type)) {
      TypePtr recovered_operand_type;
      if(try_analyze_recovered_sizeof_type_id_operand(ctx,
                                                      scope,
                                                      child,
                                                      recovered_operand_type)) {
        operand_type = recovered_operand_type;
        prepare_sizeof_operand_type(ctx, operand_type);
        record_sizeof_type_id_class_use_if_needed(ctx,
                                                  scope,
                                                  child,
                                                  operand_type);
      }
    }
    if(!type_is_valid_sizeof_operand(operand_type)) {
      throw logic_error("invalid sizeof type-id");
    }
  } else if(child.kind == CppAstKind::id_expression) {
    bool allow_constant_fold = true;
    const ValueBinding * binding =
        lookup_id_expression_value_binding(ctx, scope, child, allow_constant_fold);
    (void)allow_constant_fold;
    if(binding) {
      ExprInfo expr = analyze_unevaluated_sizeof_operand(ctx, scope, child);
      operand_type = expr.type;
    } else {
      operand_type = lookup_id_expression_type_name(ctx, scope, child);
    }
    if(!prepare_sizeof_operand_type(ctx, operand_type)) {
      throw logic_error("invalid sizeof operand");
    }
  } else {
    ExprInfo expr = analyze_unevaluated_sizeof_operand(ctx, scope, child);
    operand_type = expr.type;
    if(!prepare_sizeof_operand_type(ctx, operand_type)) {
      throw logic_error("invalid sizeof operand");
    }
  }

  ExprInfo result;
  result.type = make_fundamental(FT_UNSIGNED_LONG_INT);
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::sizeof_expression);
  set_expr_metadata(result.node, result.type, result.category);
  if(ctx.sizeof_depends_on_template_parameters(operand_type) &&
     ctx.scope_has_template_placeholders(scope)) {
    return result;
  }
  TypePtr sizeof_type = sizeof_operand_type(operand_type);
  set_callsem_uint_value(result.node, type_size(sizeof_type));
  return result;
}

ExprInfo analyze_sizeof_pack_expression(SemanticContext & ctx,
                                        Scope & scope,
                                        const CppAstNode & node)
{
  if(node.children.size() != 1 || node.children[0].kind != CppAstKind::identifier) {
    throw logic_error("sizeof-pack-expression arity");
  }

  ExprInfo result;
  result.type = make_fundamental(FT_UNSIGNED_LONG_INT);
  result.category = VC_PRVALUE;
  result.node = make_dump_node(CallSemKind::sizeof_expression);
  set_expr_metadata(result.node, result.type, result.category);

  size_t pack_size = 0;
  if(ctx.lookup_pack_size(scope, node.children[0].value, pack_size)) {
    set_callsem_uint_value(result.node, pack_size);
    return result;
  }

  if(ctx.scope_has_template_placeholders(scope)) {
    return result;
  }

  throw logic_error("invalid sizeof... operand");
}

ExprInfo analyze_type_trait_expression(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node)
{
  if(node.kind != CppAstKind::type_trait_expression || !node.has_token ||
     node.children.empty()) {
    throw logic_error("unsupported type trait expression");
  }

  ExprInfo result;
  if(node.simple_type == KW_ALIGNOF) {
    if(node.children[0].kind != CppAstKind::type_id) {
      throw logic_error("alignof requires type-id");
    }
    TypePtr type;
    bool parsed_type_id = ctx.parse_type_id(scope, node.children[0], type) &&
                          static_cast<bool>(type);
    const bool gnu_expression_form =
        (node.value == "__alignof" || node.value == "__alignof__");
    if(!parsed_type_id && gnu_expression_form) {
      CppAstNode recovered_operand;
      size_t recovered_alignment = 0;
      if(cppast_recover_sizeof_type_id_expression_operand(
             node.children[0], recovered_operand) &&
         ctx.evaluate_alignof_expression_operand(
             scope, recovered_operand, recovered_alignment)) {
        result.type = make_fundamental(FT_UNSIGNED_LONG_INT);
        result.category = VC_PRVALUE;
        result.node = make_dump_node(CallSemKind::sizeof_expression);
        set_expr_metadata(result.node, result.type, result.category);
        set_callsem_uint_value(result.node, recovered_alignment);
        return result;
      }
    }
    if(!parsed_type_id) {
      throw logic_error("unsupported alignof type-id");
    }
    maybe_complete_layout_type(ctx, type);
    result.type = make_fundamental(FT_UNSIGNED_LONG_INT);
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::sizeof_expression);
    set_expr_metadata(result.node, result.type, result.category);
    try {
      TypePtr alignof_type = remove_reference_type(type);
      if(!alignof_type) {
        alignof_type = type;
      }
      set_callsem_uint_value(result.node, type_alignment(alignof_type));
    } catch(const logic_error & e) {
      ostringstream out;
      out << e.what() << " [alignof type " << describe_type(type)
          << " complete=" << (type && type->named_complete ? "yes" : "no")
          << " has_layout=" << (type && type->named_has_layout ? "yes" : "no");
      if(type && type->kind == Type::TK_NAMED) {
        out << " key=" << type->named_key;
        ClassInfo * info = ctx.class_info_for_type(type);
        if(info && info->member_scope) {
          out << " scope_has_template_placeholders="
              << (ctx.scope_has_template_placeholders(*info->member_scope) ? "yes" : "no")
              << " dependent_named_types=";
          bool first = true;
          for(const auto & named : info->member_scope->named_types) {
            if(ctx.type_depends_on_template_parameter(named.second)) {
              if(!first) {
                out << ",";
              }
              out << named.first << "=" << describe_type(named.second);
              first = false;
            }
          }
          for(const auto & pack : info->member_scope->named_type_packs) {
            bool pack_dependent = false;
            for(size_t i = 0; i < pack.second.size(); ++i) {
              if(ctx.type_depends_on_template_parameter(pack.second[i])) {
                pack_dependent = true;
                break;
              }
            }
            if(pack_dependent) {
              if(!first) {
                out << ",";
              }
              out << pack.first << "=<pack>";
              first = false;
            }
          }
          if(first) {
            out << "<none>";
          }
        }
      }
      out << "]";
      throw logic_error(out.str());
    }
    return result;
  }

  if(node.simple_type == KW_NOEXCEPT) {
    bool is_nothrow = false;
    if(!ctx.expression_is_nothrow(scope, node.children[0], is_nothrow)) {
      throw logic_error("unsupported noexcept expression");
    }
    result.type = make_fundamental(FT_BOOL);
    result.category = VC_PRVALUE;
    result.node = make_dump_node(CallSemKind::literal, is_nothrow ? "true" : "false");
    result.node.has_token = true;
    result.node.token_type = is_nothrow ? KW_TRUE : KW_FALSE;
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, is_nothrow ? 1 : 0);
    return result;
  }

  if(node.token_kind == RT_IDENTIFIER &&
     semantic_builtins::is_supported_builtin_type_trait_name(node.value)) {
    std::string builtin_name = node.value;
    vector<TypePtr> types;
    if(!semantic_builtins::try_parse_builtin_type_trait_expression(
           ctx, scope, node, builtin_name, types)) {
      throw logic_error("unsupported builtin type trait expression");
    }

    long long value = 0;
    const bool evaluated =
        ctx.evaluate_builtin_type_trait(scope, builtin_name, types, value);
    if(!evaluated) {
      throw logic_error("unsupported builtin type trait expression");
    }

    result.type = semantic_builtins::builtin_type_trait_result_type(builtin_name);
    result.category = VC_PRVALUE;
    const bool bool_result = is_bool_type(result.type);
    result.node = make_dump_node(
        CallSemKind::literal,
        bool_result ? (value ? "true" : "false") : std::to_string(value));
    if(bool_result) {
      result.node.has_token = true;
      result.node.token_type = value ? KW_TRUE : KW_FALSE;
    }
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_uint_value(result.node, static_cast<unsigned long long>(value));
    return result;
  }

  if(node.simple_type != KW_TYPEID) {
    throw logic_error("unsupported type trait expression");
  }

  result.type = rtti_object_type(ctx, scope);
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::typeid_expression);
  set_expr_metadata(result.node, result.type, result.category);

  TypePtr type;
  const bool has_type_operand =
      try_resolve_typeid_type_operand(ctx, scope, node.children[0], type);
  if(node.children[0].kind == CppAstKind::type_id && !has_type_operand) {
    throw logic_error("unsupported typeid type-id");
  }
  if(has_type_operand) {
    TypePtr typeid_type = canonical_typeid_operand_type(type);
    require_complete_typeid_class_operand(ctx, typeid_type);
    ctx.note_rtti_use(typeid_type, false);
    result.node.text = rtti_symbol_for_type(typeid_type);
    set_callsem_typeid_operand_type(result.node, typeid_type);
    return result;
  }

  ExprInfo operand = ctx.analyze_expression(scope, node.children[0]);
  ClassInfo * info = complete_class_type_for_lookup(ctx, operand.type);
  if(info && info->is_polymorphic && operand.category == VC_LVALUE) {
    ctx.note_dynamic_typeid_use(info->type);
    result.node.text = rtti_symbol_for_type(info->type);
    result.node.children.push_back(std::move(operand.node));
    ctx.append_rtti_candidates(info->type, result.node);
    return result;
  }

  TypePtr typeid_type = canonical_typeid_operand_type(operand.type);
  ctx.note_rtti_use(typeid_type, false);
  result.node.text = rtti_symbol_for_type(typeid_type);
  set_callsem_typeid_operand_type(result.node, typeid_type);
  return result;
}

ExprInfo analyze_cast_expression(SemanticContext & ctx,
                                 Scope & scope,
                                 const CppAstNode & node)
{
  if(node.kind != CppAstKind::cast_expression || !node.has_token ||
     node.children.size() != 2) {
    throw logic_error("unsupported cast expression");
  }

  const auto semantic_type_id_type =
      [](const CppAstNode & type_id) -> TypePtr
  {
    if(type_id.semantic_type) {
      return type_id.semantic_type;
    }
    if(type_id.kind != CppAstKind::type_id || type_id.children.empty()) {
      return TypePtr();
    }
    if(type_id.children.size() != 1) {
      return TypePtr();
    }
    const CppAstNode & specifiers = type_id.children[0];
    if(specifiers.semantic_type) {
      return specifiers.semantic_type;
    }
    if(specifiers.kind == CppAstKind::type_specifier_seq &&
       specifiers.children.size() == 1 &&
       specifiers.children[0].kind == CppAstKind::type_name) {
      return specifiers.children[0].semantic_type;
    }
    return TypePtr();
  };

  TypePtr target_type;
  TypePtr semantic_target_type = semantic_type_id_type(node.children[0]);
  if(semantic_target_type &&
     !ctx.type_depends_on_template_parameter(semantic_target_type)) {
    target_type = semantic_target_type;
  }
  if(!target_type &&
     !ctx.parse_type_id(scope, node.children[0], target_type, true)) {
    ExprInfo disguised_call;
    if(try_analyze_disguised_parenthesized_call(ctx, scope, node, disguised_call)) {
      return disguised_call;
    }
    ExprInfo disguised_binary;
    if(try_analyze_disguised_parenthesized_binary_expression(
           ctx, scope, node, disguised_binary)) {
      return disguised_binary;
    }
    ostringstream out;
    out << "unsupported cast target type";
    out << " [target " << node_text(node.children[0]) << "]";
    out << " [target_ast {" << describe_cppast_translation_unit(node.children[0]) << "}]";
    throw logic_error(out.str());
  }

  const bool c_style_cast =
      node.simple_type != KW_STATIC_CAST &&
      node.simple_type != KW_DYNAMIC_CAST &&
      node.simple_type != KW_CONST_CAST &&
      node.simple_type != KW_REINTERPET_CAST;
  if(c_style_cast && node.children[1].kind == CppAstKind::braced_init_list) {
    ExprInfo target_aware;
    if(ctx.try_analyze_target_aware_expression(scope, node.children[1], target_type, target_aware)) {
      return target_aware;
    }
    return semantic_overload::analyze_functional_cast(
        ctx,
        scope,
        target_type,
        vector<const CppAstNode *>(),
        &node.children[1]);
  }

  TypePtr cast_target_base = strip_top_level_cv(target_type);
  TypePtr cast_target_object_type = strip_top_level_cv(remove_reference_type(target_type));
  const bool class_object_direct_cast =
      (node.simple_type == KW_STATIC_CAST || c_style_cast) &&
      cast_target_base &&
      cast_target_base->kind != Type::TK_LVALUE_REFERENCE &&
      cast_target_base->kind != Type::TK_RVALUE_REFERENCE &&
      cast_target_object_type &&
      cast_target_object_type->kind == Type::TK_NAMED &&
      (ctx.class_info_for_type(cast_target_object_type) ||
       complete_class_type_for_lookup(ctx, cast_target_object_type));
  if(class_object_direct_cast) {
    vector<const CppAstNode *> arg_nodes(1, &node.children[1]);
    return semantic_overload::analyze_functional_cast(
        ctx,
        scope,
        target_type,
        arg_nodes,
        nullptr);
  }

  ExprInfo operand;
  TypePtr overload_target = strip_top_level_cv(remove_reference_type(target_type));
  TypePtr overload_target_function =
      overload_target &&
              (overload_target->kind == Type::TK_POINTER ||
               overload_target->kind == Type::TK_MEMBER_POINTER) ?
          strip_top_level_cv(overload_target->inner) :
          TypePtr();
  const bool selects_overloaded_function =
      node.simple_type == KW_STATIC_CAST &&
      overload_target_function &&
      overload_target_function->kind == Type::TK_FUNCTION &&
      node.children[1].kind == CppAstKind::unary_expression &&
      node_has_simple_type(node.children[1], OP_AMP);
  if(!selects_overloaded_function ||
     !ctx.try_analyze_target_aware_expression(
         scope, node.children[1], target_type, operand)) {
    operand = ctx.analyze_expression(scope, node.children[1]);
  }
  const bool direct_static_class_reference_cast =
      node.simple_type == KW_STATIC_CAST &&
      direct_static_reference_cast_preserves_object(ctx, scope, target_type, operand);
  const TypePtr reinterpret_reference_target =
      node.simple_type == KW_REINTERPET_CAST ?
          strip_top_level_cv(remove_reference_type(target_type)) :
          TypePtr();
  const bool reinterpret_reference_cast =
      reinterpret_reference_target &&
      is_reference_type(target_type) &&
      !is_void_type(reinterpret_reference_target) &&
      !is_function_type(reinterpret_reference_target);
  if(node.simple_type == KW_STATIC_CAST) {
    enforce_static_inheritance_cast_access(ctx, scope, target_type, operand);
  }
  bool applied_direct_static_reference_cast = false;
  if(node.simple_type == KW_STATIC_CAST || c_style_cast) {
    TypePtr explicit_target = strip_top_level_cv(remove_reference_type(target_type));
    const bool explicit_target_may_be_class =
        explicit_target && explicit_target->kind == Type::TK_NAMED;
    ClassInfo * explicit_target_info =
        explicit_target_may_be_class ?
            ctx.class_info_for_type(explicit_target) :
            nullptr;
    const bool explicit_target_completion_in_progress =
        explicit_target_info &&
        (explicit_target_info->full_member_collection_in_progress ||
         explicit_target_info->reference_member_collection_in_progress);
    if(explicit_target &&
       !is_void_type(explicit_target) &&
       !explicit_target_completion_in_progress &&
       (!explicit_target_may_be_class ||
        !complete_class_type_for_lookup(ctx, explicit_target))) {
      ExprInfo converted;
      ConversionRank conversion_rank = CR_BAD;
      if(ctx.try_argument_conversion(scope,
                                     target_type,
                                     operand,
                                     converted,
                                     conversion_rank,
                                     semantic_policy::allow_explicit_argument_conversion())) {
        operand = converted;
      }
    }
    const bool explicit_class_reference_target =
        node.simple_type == KW_STATIC_CAST &&
        is_reference_type(target_type) &&
        cast_target_object_type &&
        cast_target_object_type->kind == Type::TK_NAMED &&
        (ctx.class_info_for_type(cast_target_object_type) ||
         complete_class_type_for_lookup(ctx, cast_target_object_type));
    if(explicit_class_reference_target && !direct_static_class_reference_cast) {
      ExprInfo converted;
      ConversionRank conversion_rank = CR_BAD;
      if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                               target_type,
                                                               operand,
                                                               converted) ||
         try_apply_static_reference_base_cast(ctx,
                                              target_type,
                                              operand,
                                              converted) ||
         try_apply_static_reference_derived_cast(ctx,
                                                 target_type,
                                                 operand,
                                                 converted)) {
        operand = converted;
        applied_direct_static_reference_cast = true;
      } else if(ctx.try_argument_conversion(
                    scope,
                    target_type,
                    operand,
                    converted,
                    conversion_rank,
                    semantic_policy::allow_explicit_argument_conversion())) {
        operand = converted;
      }
    }

    ExprInfo inherited_conversion;
    if(!applied_direct_static_reference_cast) {
      if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                               target_type,
                                                               operand,
                                                               inherited_conversion)) {
        operand = inherited_conversion;
      } else if(try_apply_static_reference_base_cast(ctx,
                                                     target_type,
                                                     operand,
                                                     inherited_conversion)) {
        operand = inherited_conversion;
      } else if(try_apply_static_reference_derived_cast(ctx,
                                                        target_type,
                                                        operand,
                                                        inherited_conversion)) {
        operand = inherited_conversion;
      } else if(try_apply_static_pointer_derived_cast(ctx,
                                                      target_type,
                                                      operand,
                                                      inherited_conversion)) {
        operand = inherited_conversion;
      }
    }
  }
  if((node.simple_type == KW_STATIC_CAST || c_style_cast) &&
     pointer_downcast_crosses_virtual_base(ctx, target_type, operand)) {
    throw logic_error("pointer downcast through virtual base");
  }
  if(node.simple_type == KW_DYNAMIC_CAST) {
    TypePtr target_base = strip_top_level_cv(target_type);
    if(!target_base) {
      throw logic_error("dynamic_cast requires pointer or reference forms");
    }

    const bool target_pointer_form = target_base->kind == Type::TK_POINTER;
    const bool target_reference_form =
        target_base->kind == Type::TK_LVALUE_REFERENCE ||
        target_base->kind == Type::TK_RVALUE_REFERENCE;
    if(!target_pointer_form && !target_reference_form) {
      throw logic_error("dynamic_cast requires matching pointer/reference forms");
    }

    TypePtr source_object_type;
    if(target_pointer_form) {
      TypePtr operand_base = value_conversion_type(operand);
      if(!operand_base || operand_base->kind != Type::TK_POINTER) {
        throw logic_error("dynamic_cast requires matching pointer/reference forms");
      }
      source_object_type = operand_base->inner;
    } else {
      if(operand.category != VC_LVALUE && operand.category != VC_XVALUE) {
        throw logic_error("dynamic_cast requires matching pointer/reference forms");
      }
      TypePtr operand_base = strip_top_level_cv(remove_reference_type(operand.type));
      if(!operand_base || operand_base->kind == Type::TK_POINTER) {
        throw logic_error("dynamic_cast requires matching pointer/reference forms");
      }
      source_object_type = operand_base;
    }

    ClassInfo * source_class =
        complete_class_type_for_lookup(ctx, source_object_type);
    if(!source_class) {
      throw logic_error("dynamic_cast requires class pointers/references");
    }
    if(target_pointer_form && is_void_type(target_base->inner)) {
      if(!source_class->is_polymorphic) {
        throw logic_error("dynamic_cast requires polymorphic class pointers/references");
      }
      ctx.note_rtti_use(source_class->type, true);

      ExprInfo result;
      result.type = target_type;
      result.category = VC_PRVALUE;
      result.node = make_dump_node(CallSemKind::dynamic_cast_expression, "void");
      set_dump_token(result.node, node);
      set_expr_metadata(result.node, result.type, result.category);
    result.node.children.push_back(std::move(operand.node));
      ctx.append_rtti_candidates(source_class->type, result.node);
      return result;
    }

    ClassInfo * target_class =
        complete_class_type_for_lookup(ctx, target_base->inner);
    if(!target_class) {
      throw logic_error("dynamic_cast requires class target");
    }
    ExprInfo base_conversion;
    if(semantic_conversion::try_apply_inheritance_conversion(ctx,
                                                             target_type,
                                                             operand,
                                                             base_conversion) ||
       try_apply_static_reference_base_cast(ctx,
                                            target_type,
                                            operand,
                                            base_conversion)) {
      return base_conversion;
    }
    if(!source_class->is_polymorphic) {
      throw logic_error("dynamic_cast requires polymorphic class pointers/references");
    }
    if(!ctx.has_dynamic_cast_candidate(source_class, target_class)) {
      throw logic_error("unsupported dynamic_cast relationship");
    }

    ctx.note_rtti_use(source_class->type, true);
    ctx.note_rtti_use(target_class->type, false);

    ExprInfo result;
    result.type = target_type;
    result.category = target_reference_form ? VC_LVALUE : VC_PRVALUE;
    result.node =
        make_dump_node(CallSemKind::dynamic_cast_expression, target_class->qualified_name);
    set_dump_token(result.node, node);
    set_expr_metadata(result.node, result.type, result.category);
      result.node.children.push_back(std::move(operand.node));
    ctx.append_rtti_candidates(source_class->type, result.node, target_class);
    return result;
  }

  if(!reinterpret_reference_cast && !can_copy_initialize(ctx, target_type, operand)) {
    TypePtr target_base = strip_top_level_cv(target_type);
    TypePtr operand_base = value_conversion_type(operand);
    bool supported = false;
    if(target_base && operand_base) {
      const bool reinterpret_like_cast =
          node.simple_type == KW_REINTERPET_CAST || c_style_cast;
      TypePtr target_object_type = remove_reference_type(target_type);
      TypePtr operand_object_type = remove_reference_type(operand.type);
      TypePtr normalized_target_object_type = target_object_type;
      TypePtr normalized_operand_object_type = operand_object_type;
      TypePtr resolved_object_type;
      if(target_object_type &&
         semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, target_object_type, resolved_object_type) &&
         resolved_object_type) {
        normalized_target_object_type = resolved_object_type;
      }
      if(operand_object_type &&
         semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, operand_object_type, resolved_object_type) &&
         resolved_object_type) {
        normalized_operand_object_type = resolved_object_type;
      }
      TypePtr target_unqualified_object_type =
          strip_top_level_cv(normalized_target_object_type);
      TypePtr operand_unqualified_object_type =
          strip_top_level_cv(normalized_operand_object_type);
      ClassInfo * target_object_class =
          target_unqualified_object_type
              ? ctx.class_info_for_type(target_unqualified_object_type)
              : nullptr;
      ClassInfo * operand_object_class =
          operand_unqualified_object_type
              ? ctx.class_info_for_type(operand_unqualified_object_type)
              : nullptr;
      const bool same_reference_object_type =
          type_equals(target_object_type, operand_object_type) ||
          type_equals(normalized_target_object_type, normalized_operand_object_type) ||
          (target_unqualified_object_type && operand_unqualified_object_type &&
           type_equals(target_unqualified_object_type, operand_unqualified_object_type)) ||
          (target_object_class && operand_object_class &&
           target_object_class == operand_object_class) ||
          same_type_with_compatible_top_cv(target_object_type, operand_object_type) ||
          same_type_with_compatible_top_cv(operand_object_type, target_object_type) ||
          same_type_with_compatible_top_cv(normalized_target_object_type,
                                           normalized_operand_object_type) ||
          same_type_with_compatible_top_cv(normalized_operand_object_type,
                                           normalized_target_object_type);
      if(node.simple_type == KW_CONST_CAST &&
         target_base->kind == Type::TK_LVALUE_REFERENCE) {
        supported =
            operand.category == VC_LVALUE &&
            (same_reference_object_type ||
             const_cast_similar_object_types(target_base->inner,
                                             operand_object_type) ||
             const_cast_similar_object_types(target_base->inner,
                                             normalized_operand_object_type));
      } else if(reinterpret_like_cast &&
                target_base->kind == Type::TK_LVALUE_REFERENCE) {
        supported =
            supports_reinterpret_like_reference_cast(target_type, operand);
      } else if(node.simple_type == KW_CONST_CAST &&
                target_base->kind == Type::TK_RVALUE_REFERENCE) {
        supported =
            operand.category != VC_PRVALUE &&
            (same_reference_object_type ||
             const_cast_similar_object_types(target_base->inner,
                                             operand_object_type) ||
             const_cast_similar_object_types(target_base->inner,
                                             normalized_operand_object_type));
      } else if(reinterpret_like_cast &&
                target_base->kind == Type::TK_RVALUE_REFERENCE) {
        supported =
            supports_reinterpret_like_reference_cast(target_type, operand);
      } else if(node.simple_type == KW_STATIC_CAST &&
         target_base->kind == Type::TK_LVALUE_REFERENCE) {
        supported =
            operand.category == VC_LVALUE &&
            (same_reference_object_type ||
             same_type_with_compatible_top_cv(target_base->inner, operand_object_type) ||
             same_type_with_compatible_top_cv(target_base->inner,
                                              normalized_operand_object_type) ||
             standard_conversion_rank_non_reference(target_base->inner, operand) != CR_BAD ||
             supports_zero_offset_static_reference_downcast(ctx,
                                                            target_type,
                                                            operand));
      } else if(node.simple_type == KW_STATIC_CAST &&
                target_base->kind == Type::TK_RVALUE_REFERENCE) {
        supported =
            same_reference_object_type ||
            same_type_with_compatible_top_cv(target_base->inner, operand_object_type) ||
            same_type_with_compatible_top_cv(target_base->inner,
                                             normalized_operand_object_type) ||
            standard_conversion_rank_non_reference(target_base->inner, operand) != CR_BAD ||
            supports_zero_offset_static_reference_downcast(ctx, target_type, operand);
      } else {
        supported =
            supports_non_reference_explicit_cast(ctx, target_type, operand, reinterpret_like_cast);
      }
    }
    if(!supported) {
      ostringstream out;
      out << "unsupported cast expression"
          << " [op " << node.value << "]"
          << " [target " << describe_type(target_type) << "]";
      TypePtr operand_base = value_conversion_type(operand);
      out << " [operand "
          << (operand_base ? describe_type(operand_base) : string("<null>")) << "]";
      throw logic_error(out.str());
    }
  }

  ExprInfo result;
  result.type = target_type;
  if(!result_value_category_for_function_result(target_type, result.category)) {
    result.category = VC_PRVALUE;
  }
  if(is_void_type(target_type)) {
    result.node = make_dump_node(CallSemKind::cast_expression, node.value);
    set_dump_token(result.node, node);
    set_expr_metadata(result.node, result.type, result.category);
    set_callsem_conversion_source_type(result.node, operand.type);
    result.node.children.push_back(std::move(operand.node));
    return result;
  }
  TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
  const bool use_explicit_lowir_cast =
      !is_void_type(target_type) &&
      !is_reference_type(target_type) &&
      target_base &&
      target_base->kind != Type::TK_ARRAY &&
      !is_function_type(target_base) &&
      (is_integral_or_unscoped_enum_type(target_base) ||
       is_floating_type(target_base) ||
       is_pointer_type(target_base));
  if(use_explicit_lowir_cast) {
    result.node = make_dump_node(CallSemKind::cast_expression, node.value);
    set_dump_token(result.node, node);
    set_expr_metadata(result.node, result.type, result.category);
    result.node.children.push_back(std::move(operand.node));
    return result;
  }
  const TypePtr materialization_source =
      reinterpret_reference_cast
          ? remove_reference_type(target_type)
          : (callsem_materialization_source_type(operand.node)
                 ? callsem_materialization_source_type(operand.node)
                 : (callsem_conversion_source_type(operand.node) ?
                        callsem_conversion_source_type(operand.node) :
                        operand.type));
  result.node = std::move(operand.node);
  result.node.semantic_type = result.type;
  set_callsem_materialization_source_type(result.node, materialization_source);
  set_callsem_conversion_source_type(result.node, operand.type);
  result.node.value_category = to_call_value_category(result.category);
  return result;
}

ExprInfo analyze_lambda_expression(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node)
{
  PreparedLambdaExpression prepared;
  bool prepared_ready = false;
  TypePtr prepared_function_type;
  bool prepared_function_type_ready = false;
  const auto ensure_prepared = [&]() -> PreparedLambdaExpression &
  {
    if(!prepared_ready) {
      prepared = prepare_lambda_expression(ctx, scope, node);
      prepared_ready = true;
    }
    return prepared;
  };
  const auto current_captureless_function_type = [&]() -> TypePtr
  {
    PreparedLambdaExpression & current = ensure_prepared();
    if(current.introducer->value != "[]" || current.template_parameters) {
      return TypePtr();
    }
    if(!prepared_function_type_ready) {
      vector<TypePtr> param_types;
      for(size_t i = 0; i < current.params.size(); ++i) {
        param_types.push_back(current.params[i].second);
      }
      prepared_function_type = make_function(current.result_type, param_types, false);
      prepared_function_type_ready = true;
    }
    return prepared_function_type;
  };

  FunctionBinding * existing_lambda = ctx.lookup_synthetic_lambda_binding(scope, node);
  if(existing_lambda) {
    if(existing_lambda->owner_class && existing_lambda->owner_class->is_lambda_closure) {
      return build_closure_object_expr(ctx, scope, *existing_lambda->owner_class);
    }
    TypePtr function_type = current_captureless_function_type();
    if(function_type && type_equals(existing_lambda->type, function_type)) {
      ExprInfo existing;
      existing.type = existing_lambda->type;
      existing.category = VC_LVALUE;
      existing.node = make_dump_node(CallSemKind::id_expression, existing_lambda->name);
      set_expr_metadata(existing.node, existing.type, existing.category);
      return existing;
    }
  }

  ClassInfo * existing_closure = ctx.lookup_synthetic_lambda_closure(scope, node);
  if(existing_closure && existing_closure->is_lambda_closure) {
    return build_closure_object_expr(ctx, scope, *existing_closure);
  }

  PreparedLambdaExpression & current = ensure_prepared();

  bool has_default_arguments = false;
  for(size_t i = 0; i < current.default_arguments.size(); ++i) {
    if(current.default_arguments[i]) {
      has_default_arguments = true;
      break;
    }
  }
  const bool captureless_lambda_can_use_synthetic_function =
      current.introducer->value == "[]" &&
      !current.template_parameters &&
      !has_default_arguments &&
      (!current.body ||
       !lambda_body_contains_local_class_declaration(*current.body));
  if(captureless_lambda_can_use_synthetic_function) {
    TypePtr function_type = current_captureless_function_type();
    FunctionBinding * binding = ctx.create_synthetic_lambda_function(scope,
                                                                    function_type,
                                                                    current.params,
                                                                    current.default_arguments,
                                                                    current.declarator,
                                                                    current.body);
    if(current.cached_body_output) {
      binding->cached_body_output = std::move(current.cached_body_output);
    }
    ctx.register_synthetic_lambda_binding(scope, node, *binding);

    ExprInfo result;
    result.type = function_type;
    result.category = VC_LVALUE;
    result.node = make_dump_node(CallSemKind::id_expression, binding->name);
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  return build_lambda_closure_expression(ctx, scope, node, current);
}

ExprInfo analyze_lambda_expression_as_closure(SemanticContext & ctx,
                                              Scope & scope,
                                              const CppAstNode & node)
{
  FunctionBinding * existing_lambda = ctx.lookup_synthetic_lambda_binding(scope, node);
  if(existing_lambda && existing_lambda->owner_class &&
     existing_lambda->owner_class->is_lambda_closure) {
    return build_closure_object_expr(ctx, scope, *existing_lambda->owner_class);
  }

  ClassInfo * existing_closure = ctx.lookup_synthetic_lambda_closure(scope, node);
  if(existing_closure && existing_closure->is_lambda_closure) {
    return build_closure_object_expr(ctx, scope, *existing_closure);
  }

  PreparedLambdaExpression prepared = prepare_lambda_expression(ctx, scope, node);
  return build_lambda_closure_expression(ctx, scope, node, prepared);
}

ExprInfo analyze_assignment_expression(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node)
{
  const auto overloaded_assignment_operator_name = [&]() -> string
  {
    if(node_has_simple_type(node, OP_ASS)) {
      return "operator=";
    }
    if(node_has_simple_type(node, OP_PLUSASS)) {
      return "operator+=";
    }
    if(node_has_simple_type(node, OP_MINUSASS)) {
      return "operator-=";
    }
    if(node_has_simple_type(node, OP_STARASS)) {
      return "operator*=";
    }
    if(node_has_simple_type(node, OP_DIVASS)) {
      return "operator/=";
    }
    if(node_has_simple_type(node, OP_MODASS)) {
      return "operator%=";
    }
    if(node_has_simple_type(node, OP_XORASS)) {
      return "operator^=";
    }
    if(node_has_simple_type(node, OP_BANDASS)) {
      return "operator&=";
    }
    if(node_has_simple_type(node, OP_BORASS)) {
      return "operator|=";
    }
    if(node_has_simple_type(node, OP_LSHIFTASS)) {
      return "operator<<=";
    }
    if(node_has_simple_type(node, OP_RSHIFTASS)) {
      return "operator>>=";
    }
    return string();
  };
  auto invalid_assignment = [&node](const ExprInfo * lhs = nullptr,
                                    const ExprInfo * rhs = nullptr,
                                    const string & detail = string()) -> void
  {
    ostringstream out;
    out << "invalid assignment";
    out << " [expr " << node_text(node) << "]";
    if(node.has_token) {
      out << " [op " << node.value << "]";
    }
    if(lhs) {
      out << " [lhs " << callsem_display_text(lhs->node) << "]";
      out << " [lhs_type " << describe_type(lhs->type) << "]";
    }
    if(rhs) {
      out << " [rhs " << callsem_display_text(rhs->node) << "]";
      out << " [rhs_type " << describe_type(rhs->type) << "]";
    }
    if(!detail.empty()) {
      out << " [detail " << detail << "]";
    }
    throw logic_error(out.str());
  };

  if(node.children.size() != 2) {
    ostringstream out;
    out << "unsupported assignment-expression";
    out << " [expr " << node_text(node) << "]";
    if(node.has_token) {
      out << " [op " << node.value << "]";
    }
    out << " [child_count " << node.children.size() << "]";
    throw logic_error(out.str());
  }

  const bool simple_assignment = node_has_simple_type(node, OP_ASS);
  ExprInfo lhs = ctx.analyze_expression(scope, node.children[0]);
  TypePtr lhs_object_type = strip_top_level_cv(remove_reference_type(lhs.type));
  if(!lhs_object_type) {
    lhs_object_type = strip_top_level_cv(lhs.type);
  }
  if(ctx.complete_class_type(lhs_object_type)) {
    if(simple_assignment) {
      return semantic_overload::analyze_overloaded_assignment_expression(ctx, scope, node, lhs);
    }
    try
    {
      return semantic_overload::analyze_overloaded_assignment_expression(ctx, scope, node, lhs);
    }
    catch(const logic_error & error)
    {
      const string detail = error.what();
      if(detail.find("[detail no viable operator=") == string::npos &&
         detail.find("[detail no operator") == string::npos) {
        throw;
      }
    }
  }
  bool rhs_preanalyzed = false;
  ExprInfo rhs;
  bool deferred_operator_builtin_fallback = false;
  string deferred_operator_builtin_fallback_operator;
  string deferred_operator_builtin_fallback_error;
  auto try_overloaded_compound_assignment =
      [&](const ExprInfo & rhs_expr, ExprInfo & out) -> bool
      {
        const string operator_name = overloaded_assignment_operator_name();
        if(operator_name.empty() || operator_name == "operator=") {
          return false;
        }

        const bool lhs_overloadable = has_overloadable_operator_operand(ctx, lhs.type);
        const bool rhs_overloadable =
            has_overloadable_operator_operand(ctx, rhs_expr.type);
        if(!lhs_overloadable && !rhs_overloadable) {
          return false;
        }

        vector<TypePtr> operator_operand_types;
        operator_operand_types.push_back(lhs.type);
        operator_operand_types.push_back(rhs_expr.type);
        semantic_overload::NonmemberOperatorCandidateSet operator_candidates;
        semantic_overload::collect_nonmember_operator_candidates(ctx,
                                                                 scope,
                                                                 operator_name,
                                                                 operator_operand_types,
                                                                 2,
                                                                 operator_candidates);
        vector<FunctionBinding *> operator_functions = operator_candidates.functions;
        vector<FunctionTemplateDecl *> operator_templates = operator_candidates.templates;
        if(operator_functions.empty() && operator_templates.empty()) {
          return false;
        }

        CppAstNode callee;
        callee.kind = CppAstKind::id_expression;
        callee.value = operator_name;

        CppAstNode arguments;
        arguments.kind = CppAstKind::paren_argument_list;
        arguments.children.push_back(node.children[0]);
        arguments.children.push_back(node.children[1]);

        CppAstNode call;
        call.kind = CppAstKind::call_expression;
        call.children.push_back(callee);
        call.children.push_back(arguments);

        Scope operator_scope(&scope, "", false);
        semantic_overload::initialize_operator_candidate_scope(
            operator_scope,
            scope,
            operator_name,
            operator_candidates.associated_scopes,
            operator_functions,
            operator_templates);

        semantic_overload::CallAnalysisHints hints;
        hints.use_location = ctx.source_location_for_node(node.children[1]);
        hints.adl_candidates_precollected = true;
        hints.args.push_back(&lhs);
        hints.args.push_back(&rhs_expr);
        const auto handle_unresolved_operator =
            [&](const logic_error & error) -> bool
            {
              if(!has_class_operand(ctx, lhs.type) &&
                 !has_class_operand(ctx, rhs_expr.type)) {
                // Enum-only compound assignment lookup may fall through to the
                // builtin assignment checks below.
                return false;
              }
              deferred_operator_builtin_fallback = true;
              deferred_operator_builtin_fallback_operator = operator_name;
              deferred_operator_builtin_fallback_error = error.what();
              return false;
            };
        try
        {
          out = ctx.analyze_call_expression(
              operator_scope,
              call,
              semantic_overload::CallAnalysisOptions(true, &hints));
          return true;
        }
        catch(const NoViableOverloadError & error)
        {
          return handle_unresolved_operator(error);
        }
        catch(const UnknownFunctionError & error)
        {
          return handle_unresolved_operator(error);
        }
      };
  if(!simple_assignment) {
    rhs = ctx.analyze_expression(scope, node.children[1]);
    rhs_preanalyzed = true;
    ExprInfo overloaded_result;
    if(try_overloaded_compound_assignment(rhs, overloaded_result)) {
      return overloaded_result;
    }
  }
  if(lhs.category != VC_LVALUE) {
    invalid_assignment(&lhs, nullptr, "lhs not lvalue");
  }

  TypePtr assign_target = remove_reference_type(lhs.type);
  if(!assign_target) {
    assign_target = lhs.type;
  }
  TypePtr assign_base = strip_top_level_cv(assign_target);
  if(is_const_object_type(assign_target) ||
     (assign_base && assign_base->kind == Type::TK_ARRAY)) {
    invalid_assignment(&lhs, nullptr, "lhs not modifiable");
  }

  if(simple_assignment) {
    rhs = ctx.analyze_expression_for_target(scope, node.children[1], assign_target);
  } else if(!rhs_preanalyzed) {
    rhs = ctx.analyze_expression(scope, node.children[1]);
  }
  if(simple_assignment) {
    if(ctx.type_depends_on_template_parameter(assign_target) ||
       ctx.type_depends_on_template_parameter(rhs.type)) {
      // Dependent assignment validity is checked at a later instantiation point.
    } else if(!can_copy_initialize(ctx, assign_target, rhs)) {
      invalid_assignment(&lhs, &rhs, "copy initialization failed");
    }
  } else {
    TypePtr lhs_base = strip_top_level_cv(remove_reference_type(lhs.type));
    if(!lhs_base) {
      lhs_base = strip_top_level_cv(lhs.type);
    }
    TypePtr rhs_base = strip_top_level_cv(remove_reference_type(rhs.type));
    if(!rhs_base) {
      rhs_base = strip_top_level_cv(rhs.type);
    }
    auto rhs_has_class_value_type = [&]() -> bool
    {
      return complete_class_type_for_lookup(ctx, value_conversion_type(rhs)) != nullptr;
    };
    auto refresh_rhs_base = [&]() -> void
    {
      rhs_base = strip_top_level_cv(remove_reference_type(rhs.type));
      if(!rhs_base) {
        rhs_base = strip_top_level_cv(rhs.type);
      }
    };
    auto try_convert_rhs_to_compound_builtin_target =
        [&](const TypePtr & target, bool integral_target) -> bool
        {
          if(!target || !rhs_has_class_value_type()) {
            return false;
          }
          ExprInfo converted;
          ConversionRank rank = CR_BAD;
          if(!ctx.try_argument_conversion(scope,
                                          target,
                                          rhs,
                                          converted,
                                          rank,
                                          semantic_policy::default_argument_conversion())) {
            return false;
          }
          TypePtr converted_type = value_conversion_type(converted);
          if(integral_target) {
            if(!is_integral_or_unscoped_enum_type(converted_type)) {
              return false;
            }
          } else if(!is_integral_or_unscoped_enum_type(converted_type) &&
                    !is_floating_type(converted_type)) {
            return false;
          }
          rhs = converted;
          refresh_rhs_base();
          return true;
        };
    const bool arithmetic_compound_operator =
        node_has_simple_type(node, OP_PLUSASS) ||
        node_has_simple_type(node, OP_MINUSASS) ||
        node_has_simple_type(node, OP_STARASS) ||
        node_has_simple_type(node, OP_DIVASS);
    const bool integral_compound_operator =
        arithmetic_compound_operator ||
        node_has_simple_type(node, OP_MODASS) ||
        node_has_simple_type(node, OP_XORASS) ||
        node_has_simple_type(node, OP_BANDASS) ||
        node_has_simple_type(node, OP_BORASS) ||
        node_has_simple_type(node, OP_LSHIFTASS) ||
        node_has_simple_type(node, OP_RSHIFTASS);
    if(rhs_has_class_value_type()) {
      if(arithmetic_compound_operator &&
         (is_integral_or_unscoped_enum_type(lhs_base) || is_floating_type(lhs_base))) {
        try_convert_rhs_to_compound_builtin_target(lhs_base, false);
      } else if(integral_compound_operator &&
                is_integral_or_unscoped_enum_type(lhs_base)) {
        try_convert_rhs_to_compound_builtin_target(lhs_base, true);
      } else if((node_has_simple_type(node, OP_PLUSASS) ||
                 node_has_simple_type(node, OP_MINUSASS)) &&
                is_complete_object_pointer_type(ctx, lhs_base)) {
        try_convert_rhs_to_compound_builtin_target(make_fundamental(FT_LONG_INT), true);
      }
    }
    const bool integral_compound =
        integral_compound_operator &&
        is_integral_type(lhs_base) && is_integral_or_unscoped_enum_type(rhs_base);
    const bool arithmetic_compound =
        arithmetic_compound_operator &&
        (is_integral_or_unscoped_enum_type(lhs_base) || is_floating_type(lhs_base)) &&
        (is_integral_or_unscoped_enum_type(rhs_base) || is_floating_type(rhs_base));
    const bool pointer_compound =
        (node_has_simple_type(node, OP_PLUSASS) ||
         node_has_simple_type(node, OP_MINUSASS)) &&
        is_complete_object_pointer_type(ctx, lhs_base) &&
        is_integral_or_unscoped_enum_type(rhs_base);
    const bool bool_pointer_plus_compound =
        node_has_simple_type(node, OP_PLUSASS) &&
        is_bool_type(lhs_base) &&
        is_complete_object_pointer_type(ctx, rhs_base);
    if(deferred_operator_builtin_fallback &&
       (integral_compound || arithmetic_compound || pointer_compound ||
        bool_pointer_plus_compound) &&
       (complete_class_type_for_lookup(ctx, value_conversion_type(lhs)) ||
        complete_class_type_for_lookup(ctx, value_conversion_type(rhs)))) {
      hard_fail_semantic_fallback(
          ctx,
          node,
          "operator-overload-to-builtin",
          "compound-assignment overload attempt fell back to builtin [op " +
              deferred_operator_builtin_fallback_operator + "] [error " +
              deferred_operator_builtin_fallback_error + "]");
    }
    if(!integral_compound && !arithmetic_compound && !pointer_compound &&
       !bool_pointer_plus_compound) {
      ostringstream out;
      out << "unsupported assignment-expression";
      out << " [expr " << node_text(node) << "]";
      if(node.has_token) {
        out << " [op " << node.value << "]";
      }
      out << " [lhs_type " << describe_type(lhs.type) << "]";
      out << " [rhs_type " << describe_type(rhs.type) << "]";
      throw logic_error(out.str());
    }
  }

  ExprInfo result;
  result.type = lhs.type;
  result.category = VC_LVALUE;
  result.node = make_dump_node(CallSemKind::assignment_expression, node.value);
  set_dump_token(result.node, node);
  set_expr_metadata(result.node, result.type, result.category);
  result.node.children.push_back(std::move(lhs.node));
  result.node.children.push_back(std::move(rhs.node));
  return result;
}

ExprInfo make_value_initialized_expr(const TypePtr & type)
{
  ExprInfo result;
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("invalid value-initialized type");
  }

  result.type = type;
  result.category = VC_PRVALUE;
  if(base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T)) {
    result.node = make_nullptr_literal_node();
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  if(base->kind == Type::TK_FUNDAMENTAL) {
    switch(base->fundamental) {
    case FT_FLOAT:
      result.node = make_dump_node(CallSemKind::literal, "0.0F");
      break;
    case FT_DOUBLE:
      result.node = make_dump_node(CallSemKind::literal, "0.0");
      break;
    case FT_LONG_DOUBLE:
      result.node = make_dump_node(CallSemKind::literal, "0.0L");
      break;
    default:
      result.node = make_integer_literal_node(0);
      break;
    }
    set_expr_metadata(result.node, result.type, result.category);
    return result;
  }

  result.node = make_integer_literal_node(0);
  set_expr_metadata(result.node, result.type, result.category);
  return result;
}

}  // namespace semantic_expression
