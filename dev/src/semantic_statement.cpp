#include "semantic_statement.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_ast.h"
#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_conversion.h"
#include "semantic_declaration.h"
#include "semantic_dependent_type.h"
#include "semantic_lifetime.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_model.h"
#include "semantic_parameter_recovery.h"
#include "semantic_scope_mutation.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "symbol_linkage.h"
#include "parser_trace.h"
#include "template_api.h"

using namespace std;

namespace semantic_statement {

using namespace cpp_decl;
using namespace semantic_model;
using namespace semantic_conversion;
using semantic_lookup::is_named_enum_type;
using DumpNode = CallSemNode;
using semantic_utils::trim_space;

namespace {

bool simple_identifier_text(const std::string & text)
{
  if(text.empty() ||
     !(std::isalpha(static_cast<unsigned char>(text[0])) ||
       text[0] == '_')) {
    return false;
  }
  for(size_t i = 1; i < text.size(); ++i) {
    if(!(std::isalnum(static_cast<unsigned char>(text[i])) ||
         text[i] == '_')) {
      return false;
    }
  }
  return true;
}

std::string simple_type_identifier_from_specifiers(const CppAstNode & specifiers)
{
  if(specifiers.children.size() != 1 ||
     specifiers.children[0].kind != CppAstKind::decl_specifier ||
     !simple_identifier_text(specifiers.children[0].value)) {
    return std::string();
  }
  return specifiers.children[0].value;
}

bool scope_level_has_expression_binding(const Scope & scope, const std::string & name)
{
  return scope.values.find(name) != scope.values.end() ||
         scope.function_sets.find(name) != scope.function_sets.end() ||
         scope.function_templates.find(name) != scope.function_templates.end() ||
         scope.variable_templates.find(name) != scope.variable_templates.end() ||
         scope.template_bound_value_names.find(name) != scope.template_bound_value_names.end() ||
         scope.template_bound_value_pack_names.find(name) !=
             scope.template_bound_value_pack_names.end() ||
         scope.named_value_packs.find(name) != scope.named_value_packs.end();
}

bool scope_level_has_type_binding(const Scope & scope, const std::string & name)
{
  return scope.named_types.find(name) != scope.named_types.end() ||
         scope.class_templates.find(name) != scope.class_templates.end() ||
         scope.alias_templates.find(name) != scope.alias_templates.end() ||
         scope.template_bound_type_names.find(name) != scope.template_bound_type_names.end() ||
         scope.template_bound_type_pack_names.find(name) !=
             scope.template_bound_type_pack_names.end() ||
         scope.named_type_packs.find(name) != scope.named_type_packs.end();
}

bool unqualified_lookup_prefers_expression_binding(const Scope & scope,
                                                   const std::string & name)
{
  for(const Scope * current = &scope; current; current = current->parent) {
    const bool has_expression = scope_level_has_expression_binding(*current, name);
    const bool has_type = scope_level_has_type_binding(*current, name);
    if(has_expression || has_type) {
      return has_expression;
    }
  }
  return false;
}

bool initializer_mentions_expression_binding(const Scope & scope, const CppAstNode & node)
{
  if(node.kind == CppAstKind::id_expression &&
     !node.qualified_name_syntax &&
     !node.template_id_syntax &&
     simple_identifier_text(node.value) &&
     unqualified_lookup_prefers_expression_binding(scope, node.value)) {
    return true;
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(initializer_mentions_expression_binding(scope, node.children[i])) {
      return true;
    }
  }
  return false;
}

bool should_recover_function_style_local_initializer(
    Scope & scope,
    const CppAstNode & declarator)
{
  CppAstNode stripped_declarator;
  CppAstNode recovered_initializer;
  std::string recovery_error;
  if(!semantic_parameter_recovery::recover_function_style_initializer_declarator(
         declarator,
         stripped_declarator,
         recovered_initializer,
         recovery_error)) {
    return false;
  }
  return initializer_mentions_expression_binding(scope, recovered_initializer);
}

class ScopedStatementTemplateUseLocation
{
public:
  explicit ScopedStatementTemplateUseLocation(const std::string & location)
    : active_(!location.empty())
  {
    if(active_) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedStatementTemplateUseLocation()
  {
    if(active_) {
      parser_trace::pop_use_location();
    }
  }

private:
  bool active_;
};

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

TypePtr resolve_local_alias_type(SemanticContext & ctx, Scope & scope, TypePtr alias)
{
  if(alias && ctx.type_depends_on_template_parameter(alias)) {
    TypePtr resolved;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, alias, resolved) && resolved) {
      alias = resolved;
    }
  }
  return alias;
}

TypePtr resolve_local_declaration_type(SemanticContext & ctx,
                                       Scope & scope,
                                       TypePtr type)
{
  if(type && ctx.type_depends_on_template_parameter(type)) {
    TypePtr resolved;
    if(semantic_dependent_type::resolve_instantiated_dependent_type(ctx, scope, type, resolved) &&
       resolved) {
      type = resolved;
    }
  }
  return type;
}

string local_static_scope_discriminator(const Scope & scope,
                                        const CppAstNode * declaration_node)
{
  if(declaration_node &&
     declaration_node->token_end >= declaration_node->token_start &&
     !(declaration_node->token_start == 0 && declaration_node->token_end == 0)) {
    ostringstream out;
    out << "tokens" << declaration_node->token_start
        << "_" << declaration_node->token_end;
    return out.str();
  }
  ostringstream out;
  out << "scope" << scope.instance_id;
  return out.str();
}

string symbol_discriminator_text(const string & text)
{
  static const char hex[] = "0123456789abcdef";
  string out;
  out.reserve(text.size() * 2);
  for(size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    out += hex[ch >> 4];
    out += hex[ch & 0x0F];
  }
  return out;
}

string local_static_function_owner_name(const Scope & scope,
                                        const string & fallback_name)
{
  if(scope.function && !scope.function->name.empty()) {
    if(scope.function->has_instantiation_arguments) {
      string discriminator = scope.function->symbol.object_symbol;
      if(discriminator.empty()) {
        discriminator = scope.function->symbol.internal_symbol;
      }
      if(discriminator.empty()) {
        discriminator = scope.function->template_instantiation_key;
      }
      if(!discriminator.empty()) {
        return string("function_symbol_") + symbol_discriminator_text(discriminator);
      }
    }
    if(scope.function->declaration_scope) {
      return semantic_lookup::scope_symbol_qualified_name(
          *scope.function->declaration_scope,
          scope.function->name);
    }
    return scope.function->name;
  }
  return semantic_lookup::scope_symbol_qualified_name(scope, fallback_name);
}

string local_static_internal_symbol(const Scope & scope,
                                    const string & name,
                                    const CppAstNode * declaration_node)
{
  ostringstream out;
  out << "__local_static::";
  out << local_static_function_owner_name(scope, name);
  out << "::" << name << "::"
      << local_static_scope_discriminator(scope, declaration_node);
  return symbol_linkage::internal_symbol_from_name(out.str());
}

string local_static_guard_internal_symbol(const Scope & scope,
                                          const string & name,
                                          const CppAstNode * declaration_node)
{
  return local_static_internal_symbol(scope, name, declaration_node) + "__guard";
}

bool scope_has_internal_namespace_linkage(const Scope * scope)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope && current->name == "<unnamed>") {
      return true;
    }
  }
  return false;
}

symbol_linkage::SymbolLinkage local_static_storage_linkage(const Scope & scope)
{
  const FunctionBinding * function = scope.function;
  if(!function) {
    return symbol_linkage::SL_INTERNAL;
  }
  if(function->symbol.linkage == symbol_linkage::SL_INTERNAL ||
     scope_has_internal_namespace_linkage(function->declaration_scope)) {
    return symbol_linkage::SL_INTERNAL;
  }
  if(symbol_linkage::has_weak_linkage(function->symbol) ||
     function->odr_mergeable_definition ||
     function->is_inline ||
     function->is_constexpr ||
     template_api::function_binding_has_linkage_template_identity(function)) {
    return symbol_linkage::SL_WEAK;
  }
  return symbol_linkage::SL_INTERNAL;
}

void postprocess_anonymous_union_storage(SemanticContext & ctx,
                                         Scope & scope,
                                         const CppAstNode & original_node)
{
  const std::string type_name =
      semantic_class_model::scope_anonymous_union_type_name(original_node);
  const std::string storage_name =
      semantic_class_model::scope_anonymous_union_storage_name(original_node);
  cpp_decl::TypePtr storage_type = ctx.lookup_type(scope, type_name);
  ClassInfo * storage_info = storage_type ? ctx.class_info_for_type(storage_type) : nullptr;
  if(!storage_info) {
    throw logic_error("missing anonymous union storage class");
  }
  semantic_class_model::inject_anonymous_union_variable_bindings(scope,
                                                                 *storage_info,
                                                                 storage_name);
}

bool decl_spec_contains_auto(const CppAstNode & node)
{
  return decl_spec_contains_token(node, KW_AUTO);
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

void set_expr_metadata(DumpNode & node,
                       const TypePtr & type,
                       ValueCategory category)
{
  node.semantic_type = type;
  node.value_category = to_call_value_category(category);
}

DumpNode make_located_dump_node(CallSemKind kind,
                                const CppAstNode & ast,
                                const string & text = string())
{
  DumpNode node = make_dump_node(kind, text);
  set_dump_source_location(node, ast);
  return node;
}

DumpNode make_integer_literal_node(long long value)
{
  DumpNode node = make_dump_node(CallSemKind::literal, to_string(value));
  set_callsem_int_value(node, value);
  node.semantic_type = make_fundamental(FT_INT);
  node.value_category = CVC_PRVALUE;
  if(value >= 0) {
    set_callsem_uint_value(node, static_cast<unsigned long long>(value));
  }
  return node;
}

CppAstNode make_identifier_node(const string & name)
{
  CppAstNode node;
  node.kind = CppAstKind::identifier;
  node.value = name;
  return node;
}

CppAstNode make_id_expr_ast_node(const string & name)
{
  CppAstNode node;
  node.kind = CppAstKind::id_expression;
  node.value = name;
  return node;
}

CppAstNode make_ptr_operator_ast_node(ETokenType token_type,
                                      const string & text)
{
  CppAstNode node;
  node.kind = CppAstKind::ptr_operator;
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = token_type;
  node.value = text;
  return node;
}

CppAstNode make_declarator_ast_node(const string & name,
                                    const CppAstNode * ptr_operator = nullptr)
{
  CppAstNode node;
  node.kind = CppAstKind::declarator;
  if(ptr_operator) {
    node.children.push_back(*ptr_operator);
  }
  node.children.push_back(make_identifier_node(name));
  return node;
}

CppAstNode make_initializer_ast_node(const CppAstNode & expr)
{
  CppAstNode node;
  node.kind = CppAstKind::initializer;
  node.children.push_back(expr);
  return node;
}

CppAstNode make_init_declarator_ast_node(const CppAstNode & declarator,
                                         const CppAstNode & initializer)
{
  CppAstNode node;
  node.kind = CppAstKind::init_declarator;
  node.children.push_back(declarator);
  node.children.push_back(initializer);
  return node;
}

CppAstNode make_init_declarator_list_ast_node(const CppAstNode & init_declarator)
{
  CppAstNode node;
  node.kind = CppAstKind::init_declarator_list;
  node.children.push_back(init_declarator);
  return node;
}

CppAstNode make_simple_declaration_ast_node(const CppAstNode & specifiers,
                                            const CppAstNode & init_declarator_list)
{
  CppAstNode node;
  node.kind = CppAstKind::simple_declaration;
  node.children.push_back(specifiers);
  node.children.push_back(init_declarator_list);
  return node;
}

DumpNode make_id_expr_node(const string & name, const TypePtr & type)
{
  DumpNode node = make_dump_node(CallSemKind::id_expression, name);
  set_expr_metadata(node, type, VC_LVALUE);
  return node;
}

DumpNode make_binary_expr_node(ETokenType token_type,
                               const string & text,
                               const TypePtr & type,
                               const DumpNode & lhs,
                               const DumpNode & rhs)
{
  DumpNode node = make_dump_node(CallSemKind::binary_expression, text);
  node.has_token = true;
  node.token_type = token_type;
  set_expr_metadata(node, type, VC_PRVALUE);
  node.children.push_back(lhs);
  node.children.push_back(rhs);
  return node;
}

DumpNode make_postfix_expr_node(ETokenType token_type,
                                const string & text,
                                const TypePtr & type,
                                const DumpNode & child)
{
  DumpNode node = make_dump_node(CallSemKind::postfix_expression, text);
  node.has_token = true;
  node.token_type = token_type;
  set_expr_metadata(node, type, VC_PRVALUE);
  node.children.push_back(child);
  return node;
}

DumpNode make_subscript_expr_node(const DumpNode & base,
                                  const DumpNode & index,
                                  const TypePtr & element_type)
{
  DumpNode node = make_dump_node(CallSemKind::subscript_expression);
  set_expr_metadata(node, element_type, VC_LVALUE);
  node.children.push_back(base);
  node.children.push_back(index);
  return node;
}

void append_hidden_variable_declaration(DumpNode & outer,
                                        const string & name,
                                        const TypePtr & type,
                                        DumpNode initializer)
{
  DumpNode decl = make_dump_node(CallSemKind::simple_declaration);
  DumpNode var = make_dump_node(CallSemKind::variable, name);
  var.semantic_type = type;
  var.children.push_back(std::move(initializer));
  decl.children.push_back(std::move(var));
  outer.children.push_back(std::move(decl));
}

bool class_has_range_member_name(ClassInfo * info, const string & name)
{
  if(!info || !info->member_scope) {
    return false;
  }

  return info->methods.find(name) != info->methods.end() ||
         info->member_scope->function_sets.find(name) != info->member_scope->function_sets.end() ||
         info->member_scope->values.find(name) != info->member_scope->values.end() ||
         info->member_scope->named_types.find(name) != info->member_scope->named_types.end();
}

void track_embedded_local_class_output(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & specifiers)
{
  for(size_t i = 0; i < specifiers.children.size(); ++i) {
    const CppAstNode & child = specifiers.children[i];
    if(child.kind != CppAstKind::class_specifier &&
       child.kind != CppAstKind::class_forward_declaration) {
      continue;
    }
    if(child.value.empty()) {
      continue;
    }
    TypePtr type = ctx.lookup_type(scope, child.value);
    ClassInfo * info = type ? ctx.class_info_for_type(type) : nullptr;
    if(!info) {
      continue;
    }
    ctx.track_instantiated_class(info);
  }
}

ExprInfo analyze_range_helper_expression(SemanticContext & ctx,
                                         Scope & scope,
                                         CppAstNode expr)
{
  const CppAstNode * owned_expr = ctx.own_synthetic_ast(std::move(expr));
  return ctx.analyze_expression(scope, *owned_expr);
}

CppAstNode make_member_expr_ast(const CppAstNode & base,
                                const string & member_name)
{
  CppAstNode node;
  node.kind = CppAstKind::member_expression;
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = OP_DOT;
  node.value = ".";
  node.children.push_back(base);
  CppAstNode member = make_identifier_node(member_name);
  QualifiedName qualified;
  qualified.name = member_name;
  set_cppast_qualified_name_syntax(member, qualified);
  node.children.push_back(member);
  return node;
}

CppAstNode make_call_expr_ast(const CppAstNode & callee,
                              const vector<CppAstNode> & args = vector<CppAstNode>())
{
  CppAstNode node;
  node.kind = CppAstKind::call_expression;
  node.children.push_back(callee);

  CppAstNode paren;
  paren.kind = CppAstKind::paren_argument_list;
  paren.children = args;
  node.children.push_back(paren);
  return node;
}

CppAstNode make_unary_expr_ast_node(ETokenType token_type,
                                    const string & text,
                                    const CppAstNode & child)
{
  CppAstNode node;
  node.kind = CppAstKind::unary_expression;
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = token_type;
  node.value = text;
  node.children.push_back(child);
  return node;
}

CppAstNode make_binary_expr_ast_node(ETokenType token_type,
                                     const string & text,
                                     const CppAstNode & lhs,
                                     const CppAstNode & rhs)
{
  CppAstNode node;
  node.kind = CppAstKind::binary_expression;
  node.has_token = true;
  node.token_kind = RT_SIMPLE;
  node.simple_type = token_type;
  node.value = text;
  node.children.push_back(lhs);
  node.children.push_back(rhs);
  return node;
}

bool condition_type_ok(SemanticContext & ctx, Scope & scope, ExprInfo & expr)
{
  return try_condition_test_conversion(ctx, scope, expr);
}

bool switch_condition_expr_ok(SemanticContext & ctx, const ExprInfo & expr)
{
  TypePtr converted = value_conversion_type(expr);
  return converted && (is_integral_type(converted) || is_named_enum_type(ctx, converted));
}

bool try_switch_condition_conversion(SemanticContext & ctx,
                                     Scope & scope,
                                     ExprInfo & expr)
{
  if(switch_condition_expr_ok(ctx, expr)) {
    return true;
  }

  ExprInfo converted_expr;
  ConversionRank rank = CR_BAD;
  if(!ctx.try_argument_conversion(scope,
                                  make_fundamental(FT_INT),
                                  expr,
                                  converted_expr,
                                  rank)) {
    return false;
  }
  if(!switch_condition_expr_ok(ctx, converted_expr)) {
    return false;
  }

  expr = converted_expr;
  return true;
}

void analyze_simple_declaration_statement(SemanticContext & ctx,
                                          Scope & scope,
                                          const CppAstNode & node,
                                          DumpNode & out);

void analyze_statement_impl(SemanticContext & ctx,
                            Scope & scope,
                            const TypePtr & return_type,
                            const CppAstNode & node,
                            DumpNode & out,
                            std::vector<ExprInfo> * collected_returns = nullptr,
                            bool * saw_void_return = nullptr,
                            const TypePtr & active_switch_type = TypePtr());

TypePtr catch_match_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(is_reference_type(base)) {
    return strip_top_level_cv(base->inner);
  }
  return base;
}

semantic_source_use::SourceUseRole declaration_alias_class_use_role(
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE ||
     base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_MEMBER_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_FUNCTION) {
    return semantic_source_use::SourceUseRole::TypeUse;
  }
  while(base && base->kind == Type::TK_ARRAY) {
    base = strip_top_level_cv(base->inner);
  }
  if(base && base->kind == Type::TK_NAMED && !base->definitely_not_class) {
    return semantic_source_use::SourceUseRole::MaterializedTypeUse;
  }
  return semantic_source_use::SourceUseRole::TypeUse;
}

void analyze_structured_binding_declaration_statement(SemanticContext & ctx,
                                                      Scope & scope,
                                                      const CppAstNode & node,
                                                      DumpNode & out)
{
  const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarator =
      find_child_kind(node, CppAstKind::structured_binding_declarator);
  const CppAstNode * initializer = find_child_kind(node, CppAstKind::initializer);
  if(!specifiers || !declarator || !initializer || initializer->children.size() != 1) {
    throw logic_error("malformed structured-binding declaration");
  }

  const CppAstNode * identifiers =
      find_child_kind(*declarator, CppAstKind::structured_binding_identifier_list);
  if(!identifiers || identifiers->children.empty()) {
    throw logic_error("structured-binding declaration missing identifiers");
  }

  const CppAstNode * hidden_ref = find_child_kind(*declarator, CppAstKind::ref_qualifier);
  const string hidden_name = ctx.next_synthetic_local_name("sb");

  CppAstNode hidden_ptr_operator;
  const CppAstNode * hidden_ptr_operator_ptr = nullptr;
  if(hidden_ref) {
    hidden_ptr_operator = make_ptr_operator_ast_node(hidden_ref->simple_type, hidden_ref->value);
    hidden_ptr_operator_ptr = &hidden_ptr_operator;
  }

  CppAstNode hidden_decl =
      make_simple_declaration_ast_node(*specifiers,
                                       make_init_declarator_list_ast_node(
                                           make_init_declarator_ast_node(
                                               make_declarator_ast_node(hidden_name,
                                                                        hidden_ptr_operator_ptr),
                                               *initializer)));
  const CppAstNode * owned_hidden_decl = ctx.own_synthetic_ast(std::move(hidden_decl));
  analyze_simple_declaration_statement(ctx, scope, *owned_hidden_decl, out);

  map<string, ValueBinding>::const_iterator hidden_binding = scope.values.find(hidden_name);
  if(hidden_binding == scope.values.end() || !hidden_binding->second.type) {
    throw logic_error("structured-binding hidden variable was not created");
  }

  TypePtr hidden_base = strip_top_level_cv(remove_reference_type(hidden_binding->second.type));
  ClassInfo * info = ctx.complete_class_type(hidden_base);
  if(!info) {
    throw logic_error("structured-binding declarations currently require class-member decomposition");
  }
  if(info->fields.size() != identifiers->children.size()) {
    throw logic_error("structured-binding field count mismatch");
  }

  const CppAstNode hidden_expr = make_id_expr_ast_node(hidden_name);
  const CppAstNode binding_ptr_operator = make_ptr_operator_ast_node(OP_AMP, "&");
  for(size_t i = 0; i < identifiers->children.size(); ++i) {
    const string & binding_name = identifiers->children[i].value;
    const FieldInfo & field = info->fields[i];
    if(binding_name.empty() || field.name.empty()) {
      throw logic_error("structured-binding declarations require named non-anonymous fields");
    }
    if(field.is_bit_field) {
      throw logic_error("structured-binding declarations do not yet support bit-fields");
    }
    if(field.access != MA_PUBLIC) {
      throw logic_error("structured-binding declarations require accessible public fields");
    }

    CppAstNode binding_decl =
        make_simple_declaration_ast_node(*specifiers,
                                         make_init_declarator_list_ast_node(
                                             make_init_declarator_ast_node(
                                                 make_declarator_ast_node(binding_name,
                                                                          &binding_ptr_operator),
                                                 make_initializer_ast_node(
                                                     make_member_expr_ast(hidden_expr,
                                                                          field.name)))));
    const CppAstNode * owned_binding_decl = ctx.own_synthetic_ast(std::move(binding_decl));
    analyze_simple_declaration_statement(ctx, scope, *owned_binding_decl, out);
  }
}

void analyze_exception_declaration(SemanticContext & ctx,
                                   Scope & handler_scope,
                                   const CppAstNode & node,
                                   DumpNode & handler_node)
{
  if(node.kind != CppAstKind::exception_declaration || node.children.empty()) {
    throw logic_error("unsupported exception-declaration");
  }

  if(node.children[0].kind == CppAstKind::ellipsis) {
    handler_node.text = "...";
    return;
  }

  const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
  if(!specifiers) {
    throw logic_error("exception-declaration missing decl-specifier-seq");
  }

  string name;
  TypePtr catch_type;
  bool is_typedef = false;
  const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
  if(declarator) {
    if(!ctx.parse_variable_declaration_type(handler_scope,
                                            *specifiers,
                                            *declarator,
                                            nullptr,
                                            false,
                                            name,
                                            catch_type,
                                            is_typedef) || is_typedef) {
      throw logic_error("unsupported catch declarator");
    }
  } else {
    CppAstNode type_id;
    type_id.kind = CppAstKind::type_id;
    type_id.children = node.children;
    if(!ctx.parse_type_id(handler_scope, type_id, catch_type)) {
      throw logic_error("unsupported catch type-id");
    }
  }

  if(!catch_type) {
    throw logic_error("catch type missing");
  }

  handler_node.semantic_type = catch_type;
  const TypePtr match_type = catch_match_type(catch_type);
  if(match_type) {
    ctx.note_rtti_use(match_type, false);
    ctx.append_exception_candidates(match_type, handler_node);
  }

  if(!name.empty()) {
    semantic_scope_mutation::bind_value(
        handler_scope, name, ValueBinding(ValueBinding::VK_VARIABLE, name, catch_type));
    DumpNode variable_node = make_dump_node(CallSemKind::variable, name);
    variable_node.semantic_type = catch_type;
    handler_node.children.push_back(std::move(variable_node));
  }
}

void analyze_condition_declaration(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & node,
                                   DumpNode & out,
                                   bool synthesize_condition_test = true,
                                   bool synthesize_switch_condition_test = false)
{
  if(node.kind != CppAstKind::condition_declaration || node.children.size() < 3) {
    throw logic_error("unsupported condition-declaration");
  }

  const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarator = find_child_kind(node, CppAstKind::declarator);
  const CppAstNode * initializer = find_child_kind(node, CppAstKind::initializer);
  if(!specifiers || !declarator || !initializer) {
    throw logic_error("condition-declaration missing children");
  }

  string name;
  TypePtr type;
  bool is_typedef = false;
  if(!ctx.parse_variable_declaration_type(scope, *specifiers, *declarator, initializer,
                                          true, name, type, is_typedef) || is_typedef) {
    throw logic_error("unsupported condition decl-specifier-seq");
  }
  if(name.empty()) {
    throw logic_error("unsupported condition declarator");
  }
  if(type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
    throw logic_error("function condition declarations unsupported");
  }

  ValueBinding binding(ValueBinding::VK_VARIABLE, name, type);
  if(decl_spec_contains_token(*specifiers, KW_CONSTEXPR) || is_const_object_type(type)) {
    long long value = 0;
    if(ctx.evaluate_initializer_constant(scope, *initializer, value)) {
      binding.has_constant_value = true;
      binding.constant_value = value;
    }
  }
  semantic_scope_mutation::bind_value(scope, name, binding);

  out = make_dump_node(CallSemKind::condition_declaration);
  DumpNode variable_node = make_dump_node(CallSemKind::variable, name);
  variable_node.semantic_type = type;
  out.children.push_back(variable_node);
  if(ctx.complete_class_type(type)) {
    semantic_lifetime::analyze_object_lifetime_actions(
        ctx,
        scope,
        name,
        type,
        initializer,
        out.children.back(),
        ctx.source_location_for_name_in_node(*declarator, name));
  } else {
    semantic_lifetime::analyze_initializer(ctx, scope, type, *initializer, out.children.back());
  }

  if(!synthesize_condition_test && !synthesize_switch_condition_test) {
    return;
  }

  ExprInfo condition_expr = ctx.analyze_expression(scope, make_id_expr_ast_node(name));
  const bool condition_ok =
      synthesize_switch_condition_test ?
          try_switch_condition_conversion(ctx, scope, condition_expr) :
          condition_type_ok(ctx, scope, condition_expr);
  if(!condition_ok) {
    throw logic_error("invalid condition declaration");
  }
  mutable_callsem_lowered_condition_test(out).reset(
      new CallSemNode(std::move(condition_expr.node)));
}

void analyze_condition_node(SemanticContext & ctx,
                            Scope & scope,
                            const CppAstNode & node,
                            DumpNode & out,
                            bool allow_switch_condition = false)
{
  if(node.kind != CppAstKind::condition || node.children.size() != 1) {
    throw logic_error("unsupported condition");
  }

  out = make_dump_node(CallSemKind::condition);
  const CppAstNode & child = node.children[0];
  if(child.kind == CppAstKind::condition_declaration) {
    DumpNode condition_decl;
    analyze_condition_declaration(ctx,
                                  scope,
                                  child,
                                  condition_decl,
                                  !allow_switch_condition,
                                  allow_switch_condition);
    out.children.push_back(std::move(condition_decl));
    return;
  }

  ExprInfo expr = ctx.analyze_expression(scope, child);
  if(!(allow_switch_condition ? try_switch_condition_conversion(ctx, scope, expr)
                              : condition_type_ok(ctx, scope, expr))) {
    throw logic_error("invalid condition expression");
  }
  long long constant_value = 0;
  if(ctx.evaluate_constant_expression(scope, child, constant_value)) {
    set_callsem_int_value(expr.node, constant_value);
    if(constant_value >= 0) {
      set_callsem_uint_value(expr.node, static_cast<unsigned long long>(constant_value));
    }
  }
  out.children.push_back(std::move(expr.node));
}

void analyze_for_init_statement(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & node,
                                DumpNode & out);

TypePtr switch_condition_type(const DumpNode & condition)
{
  if(condition.kind != CallSemKind::condition || condition.children.size() != 1) {
    throw logic_error("invalid switch condition");
  }

  const DumpNode & child = condition.children[0];
  if(child.kind == CallSemKind::condition_declaration) {
    if(child.children.size() != 1 || child.children[0].kind != CallSemKind::variable) {
      throw logic_error("invalid switch condition declaration");
    }
    if(callsem_lowered_condition_test(child)) {
      return callsem_lowered_condition_test(child)->semantic_type;
    }
    return child.children[0].semantic_type;
  }

  return child.semantic_type;
}

void analyze_switch_body(SemanticContext & ctx,
                         Scope & scope,
                         const TypePtr & switch_type,
                         const TypePtr & return_type,
                         const CppAstNode & node,
                         DumpNode & out,
                         std::vector<ExprInfo> * collected_returns = nullptr,
                         bool * saw_void_return = nullptr)
{
  if(node.kind == CppAstKind::compound_statement) {
    Scope block_scope(&scope);
    DumpNode block = make_located_dump_node(CallSemKind::compound_statement, node);
    for(size_t i = 0; i < node.children.size(); ++i) {
      analyze_switch_body(ctx,
                          block_scope,
                          switch_type,
                          return_type,
                          node.children[i],
                          block,
                          collected_returns,
                          saw_void_return);
    }
    out.children.push_back(std::move(block));
    return;
  }

  if(node.kind == CppAstKind::case_statement) {
    if(node.children.size() != 2) {
      throw logic_error("invalid case-statement");
    }
    DumpNode case_node = make_located_dump_node(CallSemKind::case_statement, node);
    ExprInfo case_expr = ctx.analyze_expression_for_target(scope, node.children[0], switch_type);
    case_node.children.push_back(std::move(case_expr.node));
    analyze_switch_body(ctx,
                        scope,
                        switch_type,
                        return_type,
                        node.children[1],
                        case_node,
                        collected_returns,
                        saw_void_return);
    out.children.push_back(std::move(case_node));
    return;
  }

  if(node.kind == CppAstKind::default_statement) {
    if(node.children.size() != 1) {
      throw logic_error("invalid default-statement");
    }
    DumpNode default_node = make_located_dump_node(CallSemKind::default_statement, node);
    analyze_switch_body(ctx,
                        scope,
                        switch_type,
                        return_type,
                        node.children[0],
                        default_node,
                        collected_returns,
                        saw_void_return);
    out.children.push_back(std::move(default_node));
    return;
  }

  analyze_statement_impl(ctx,
                         scope,
                         return_type,
                         node,
                         out,
                         collected_returns,
                         saw_void_return,
                         switch_type);
}

void analyze_switch_statement(SemanticContext & ctx,
                              Scope & scope,
                              const TypePtr & return_type,
                              const CppAstNode & node,
                              DumpNode & out,
                              std::vector<ExprInfo> * collected_returns = nullptr,
                              bool * saw_void_return = nullptr)
{
  DIAG_CONTEXT("analyze_switch_statement" + ctx.source_location_for_node(node));
  if(node.children.size() != 2 || node.children[0].kind != CppAstKind::condition) {
    throw logic_error("invalid switch-statement");
  }

  Scope switch_scope(&scope);
  DumpNode switch_node = make_located_dump_node(CallSemKind::switch_statement, node);
  DumpNode condition_node;
  analyze_condition_node(ctx, switch_scope, node.children[0], condition_node, true);

  const TypePtr condition_type = switch_condition_type(condition_node);
  const TypePtr switch_type = remove_reference_type(condition_type);
  if(!switch_type ||
     !(is_integral_type(switch_type) || is_named_enum_type(ctx, switch_type))) {
    throw logic_error("invalid switch condition type");
  }

  switch_node.children.push_back(std::move(condition_node));
  analyze_switch_body(ctx,
                      switch_scope,
                      switch_type,
                      return_type,
                      node.children[1],
                      switch_node,
                      collected_returns,
                      saw_void_return);
  out.children.push_back(std::move(switch_node));
}

void analyze_simple_declaration_statement(SemanticContext & ctx,
                                          Scope & scope,
                                          const CppAstNode & node,
                                          DumpNode & out)
{
  CppAstNode synthetic_decl;
  std::string synthetic_type_name;
  std::string synthetic_storage_name;
  if(semantic_class_model::synthesize_anonymous_union_storage_declaration(node,
                                                                          synthetic_decl,
                                                                          synthetic_type_name,
                                                                          synthetic_storage_name)) {
    const CppAstNode * owned_synthetic_decl = ctx.own_synthetic_ast(std::move(synthetic_decl));
    analyze_simple_declaration_statement(ctx, scope, *owned_synthetic_decl, out);
    postprocess_anonymous_union_storage(ctx, scope, node);
    return;
  }

  const CppAstNode * specifiers = find_child_kind(node, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarators = find_child_kind(node, CppAstKind::init_declarator_list);
  if(!specifiers) {
    throw logic_error("simple-declaration missing decl-specifier-seq");
  }

  if(specifiers->children.empty() &&
     (!declarators || declarators->children.empty())) {
    out.children.push_back(make_located_dump_node(CallSemKind::simple_declaration, node));
    return;
  }

  bool declaration_is_typedef = false;
  const bool has_embedded_class_specifier =
      any_of(specifiers->children.begin(), specifiers->children.end(),
             [](const CppAstNode & child)
             {
               return child.kind == CppAstKind::class_specifier ||
                      child.kind == CppAstKind::class_forward_declaration;
             });
  PreparedDeclarationSpecifiers prepared_specifiers;
  if(!ctx.prepare_block_scope_declaration_specifiers(scope,
                                                     *specifiers,
                                                     declarators,
                                                     prepared_specifiers)) {
    throw logic_error("unsupported local embedded class-specifier");
  }
  if(has_embedded_class_specifier) {
    track_embedded_local_class_output(ctx, scope, prepared_specifiers.resolved_specifiers);
  }
  declaration_is_typedef = prepared_specifiers.declaration_is_typedef;
  if(!prepared_specifiers.has_auto && !prepared_specifiers.parsed_decl_spec) {
    ostringstream outmsg;
    outmsg << "unsupported local decl-specifier-seq";
    outmsg << " [specifiers " << node_text(*specifiers) << "]";
    outmsg << " [spec_kinds";
    for(size_t i = 0; i < specifiers->children.size(); ++i) {
      outmsg << (i == 0 ? " " : ",") << cppast_kind_text(specifiers->children[i].kind);
      if(!specifiers->children[i].value.empty()) {
        outmsg << "=" << specifiers->children[i].value;
      }
    }
    if(specifiers->children.empty()) {
      outmsg << " <none>";
    }
    outmsg << "]";
    if(declarators) {
      outmsg << " [declarator_kinds";
      for(size_t i = 0; i < declarators->children.size(); ++i) {
        outmsg << (i == 0 ? " " : ",") << cppast_kind_text(declarators->children[i].kind);
      }
      if(declarators->children.empty()) {
        outmsg << " <none>";
      }
      outmsg << "]";
    }
    outmsg << " [declaration " << node_text(node) << "]";
    throw logic_error(outmsg.str());
  }

  DumpNode decl_node = make_located_dump_node(CallSemKind::simple_declaration, node);

  if(!declarators) {
    out.children.push_back(std::move(decl_node));
    return;
  }

  for(size_t i = 0; i < declarators->children.size(); ++i) {
    const CppAstNode & init_decl = declarators->children[i];
    if(init_decl.kind != CppAstKind::init_declarator || init_decl.children.empty()) {
      throw logic_error("unsupported local init-declarator");
    }

    string name;
    TypePtr type;
    const CppAstNode * initializer =
        init_decl.children.size() > 1 ? &init_decl.children[1] : nullptr;
    bool is_typedef = declaration_is_typedef;
    CppAstNode synthesized_initializer;
    string recover_local_init_error;
    auto try_recover_function_style_local_initializer =
        [&](const CppAstNode & declarator_node,
            string & recovered_name,
            TypePtr & recovered_type,
            bool & recovered_typedef,
            CppAstNode & recovered_initializer) -> bool
    {
      recover_local_init_error.clear();
      if(initializer || recovered_typedef) {
        recover_local_init_error = "has explicit initializer or typedef";
        return false;
      }

      CppAstNode stripped_declarator;
      if(!semantic_parameter_recovery::recover_function_style_initializer_declarator(
             declarator_node,
             stripped_declarator,
             recovered_initializer,
             recover_local_init_error)) {
        return false;
      }

      string local_name;
      TypePtr local_type;
      bool local_typedef = recovered_typedef;
      if(!ctx.parse_variable_declaration_type(scope,
                                              prepared_specifiers.resolved_specifiers,
                                              stripped_declarator,
                                              &recovered_initializer,
                                              true,
                                              local_name,
                                              local_type,
                                              local_typedef)) {
        recover_local_init_error = "failed stripped declaration parse";
        return false;
      }
      if(local_typedef) {
        recover_local_init_error = "recovered declarator is typedef";
        return false;
      }
      if(!local_type) {
        recover_local_init_error = "null recovered local type";
        return false;
      }
      if(strip_top_level_cv(local_type)->kind == Type::TK_FUNCTION) {
        recover_local_init_error = "recovered type still function";
        return false;
      }

      recovered_name = local_name;
      recovered_type = local_type;
      recovered_typedef = false;
      return true;
    };

    const bool parsed_as_variable =
        ctx.parse_variable_declaration_type(scope,
                                           prepared_specifiers.resolved_specifiers,
                                           init_decl.children[0],
                                           initializer,
                                           true,
                                           name,
                                           type,
                                           is_typedef);
    const bool parsed_as_function =
        parsed_as_variable && type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION;
    const bool recover_function_style_initializer =
        parsed_as_function &&
        !initializer &&
        should_recover_function_style_local_initializer(scope, init_decl.children[0]);
    if(parsed_as_function && !initializer && !recover_function_style_initializer) {
      vector<pair<string, TypePtr> > params;
      vector<const CppAstNode *> default_args;
      const CppAstNode * parameter_clause =
          find_child_kind(init_decl.children[0], CppAstKind::parameter_clause);
      if(parameter_clause &&
         !ctx.parse_parameter_clause(scope, *parameter_clause, params, &default_args)) {
        throw logic_error("unsupported local function parameter-clause");
      }
      FunctionBinding * binding =
          ctx.register_block_scope_function_declaration(
              scope,
              name,
              type,
              params,
              default_args,
              node,
              init_decl.children[0],
              prepared_specifiers.resolved_specifiers);
      DumpNode fn_node = make_dump_node(CallSemKind::function_declaration, name);
      fn_node.semantic_type = binding ? binding->type : type;
      if(binding) {
        set_callsem_resolved_name(fn_node, function_output_name(*binding));
        set_dump_symbol(fn_node, binding->symbol);
      }
      decl_node.children.push_back(std::move(fn_node));
      continue;
    }
    if((!parsed_as_variable || parsed_as_function) &&
       !try_recover_function_style_local_initializer(init_decl.children[0], name, type,
                                                     is_typedef, synthesized_initializer)) {
      ostringstream outmsg;
      outmsg << "unsupported local declarator";
      outmsg << " [specifiers " << node_text(*specifiers) << "]";
      outmsg << " [spec_kinds";
      for(size_t j = 0; j < specifiers->children.size(); ++j) {
        outmsg << (j == 0 ? " " : ",") << cppast_kind_text(specifiers->children[j].kind);
        if(!specifiers->children[j].value.empty()) {
          outmsg << "=" << specifiers->children[j].value;
        }
      }
      if(specifiers->children.empty()) {
        outmsg << " <none>";
      }
      outmsg << "]";
      outmsg << " [declarator " << node_text(init_decl.children[0]) << "]";
      outmsg << " [declarator_kind " << cppast_kind_text(init_decl.children[0].kind) << "]";
      if(!init_decl.children[0].children.empty()) {
        outmsg << " [declarator_child_kinds";
        for(size_t j = 0; j < init_decl.children[0].children.size(); ++j) {
          outmsg << (j == 0 ? " " : ",")
                 << cppast_kind_text(init_decl.children[0].children[j].kind);
          if(!init_decl.children[0].children[j].value.empty()) {
            outmsg << "=" << init_decl.children[0].children[j].value;
          }
        }
        outmsg << "]";
      }
      if(const CppAstNode * parameter_clause =
             find_child_kind(init_decl.children[0], CppAstKind::parameter_clause)) {
        outmsg << " [parameter_clause " << node_text(*parameter_clause) << "]";
      }
      if(!recover_local_init_error.empty()) {
        outmsg << " [recover_local_init_error " << recover_local_init_error << "]";
      }
      if(initializer) {
        outmsg << " [initializer " << node_text(*initializer) << "]";
      }
      outmsg << " [declaration " << node_text(node) << "]";
      throw logic_error(outmsg.str());
    }
    if(!initializer && !synthesized_initializer.children.empty()) {
      initializer = &synthesized_initializer;
    }

    if(!is_typedef) {
      type = resolve_local_declaration_type(ctx, scope, type);
    }

    if(type && strip_top_level_cv(type)->kind == Type::TK_FUNCTION) {
      throw logic_error("local function declarations unsupported");
    }

    if(!is_typedef && type) {
      const std::string specifier_text =
          simple_type_identifier_from_specifiers(*specifiers);
      if(!specifier_text.empty()) {
        const std::string specifier_location =
            ctx.source_location_for_name_in_node(*specifiers, specifier_text);
        if(!specifier_location.empty()) {
          ctx.record_deduced_class_use_for_resolved_alias_type(scope,
                                                               type,
                                                               specifier_location,
                                                               declaration_alias_class_use_role(type));
        }
      }
    }

    if(is_typedef) {
      type = resolve_local_alias_type(ctx, scope, type);
      semantic_scope_mutation::bind_named_type(scope, name, type);
      DumpNode alias_node = make_dump_node(CallSemKind::type_alias, name);
      alias_node.semantic_type = type;
      decl_node.children.push_back(std::move(alias_node));
    } else {
      TypePtr direct_storage_type = strip_top_level_cv(remove_reference_type(type));
      TypePtr static_storage_base = direct_storage_type;
      while(static_storage_base && static_storage_base->kind == Type::TK_ARRAY) {
        static_storage_base = strip_top_level_cv(static_storage_base->inner);
      }
      const bool is_reference_declaration =
          is_reference_type(strip_top_level_cv(type));
      const bool has_class_lifetime =
          !is_reference_declaration &&
          direct_storage_type && ctx.complete_class_type(direct_storage_type);
      const bool has_automatic_array_class_lifetime =
          direct_storage_type != static_storage_base &&
          static_storage_base &&
          ctx.complete_class_type(static_storage_base);
      const bool is_thread_local =
          decl_spec_contains_token(prepared_specifiers.resolved_specifiers, KW_THREAD_LOCAL);
      long long constexpr_static_value = 0;
      const bool has_constant_static_initializer =
          initializer && ctx.evaluate_initializer_constant(scope, *initializer,
                                                           constexpr_static_value);
      const bool has_static_storage_specifier =
          is_thread_local ||
          decl_spec_contains_token(prepared_specifiers.resolved_specifiers, KW_STATIC);
      const bool use_global_static_storage = has_static_storage_specifier;
      const bool needs_local_static_guard =
          use_global_static_storage &&
          (has_class_lifetime ||
           (initializer && !has_constant_static_initializer));
      const std::string object_use_location =
          ctx.source_location_for_name_in_node(node, name, true);

      ValueBinding binding(ValueBinding::VK_VARIABLE, name, type);
      binding.declaration_node = &init_decl;
      binding.is_thread_local = is_thread_local;
      if(use_global_static_storage) {
        binding.symbol = symbol_linkage::make_internal_symbol_identity(
            local_static_internal_symbol(scope, name, &init_decl),
            local_static_storage_linkage(scope));
        if(is_thread_local) {
          binding.symbol.thread_local_wrapper_object_symbol =
              symbol_linkage::thread_local_wrapper_internal_symbol(
                  binding.symbol.object_symbol);
        }
        binding.declaration_scope = &scope;
      }
      if(initializer &&
         (decl_spec_contains_token(*specifiers, KW_CONSTEXPR) || is_const_object_type(type))) {
        long long value = 0;
        if(ctx.evaluate_initializer_constant(scope, *initializer, value)) {
          binding.has_constant_value = true;
          binding.constant_value = value;
        }
      }
      semantic_scope_mutation::bind_value(scope, name, binding);
      DumpNode var_node = make_dump_node(CallSemKind::variable, name);
      var_node.semantic_type = type;
      var_node.is_thread_local = is_thread_local;
      if(use_global_static_storage) {
        var_node.is_static_storage = true;
        set_dump_symbol(var_node, binding.symbol);
        if(needs_local_static_guard) {
          set_callsem_local_static_guard_symbol(
              var_node,
              local_static_guard_internal_symbol(scope, name, &init_decl));
        }
      }
      if(use_global_static_storage && has_class_lifetime) {
        semantic_lifetime::analyze_object_lifetime_actions(
            ctx,
            scope,
            name,
            type,
            initializer,
            var_node,
            object_use_location);
      } else if(use_global_static_storage) {
        if(initializer) {
          semantic_lifetime::analyze_initializer(ctx, scope, type, *initializer, var_node);
        }
      } else if(ctx.complete_class_type(type) ||
                (has_automatic_array_class_lifetime && !initializer)) {
        semantic_lifetime::analyze_object_lifetime_actions(
            ctx,
            scope,
            name,
            type,
            initializer,
            var_node,
            object_use_location);
      } else if(initializer) {
        semantic_lifetime::analyze_initializer(ctx, scope, type, *initializer, var_node);
      }
      if(needs_local_static_guard) {
        for(size_t i = 0; i < var_node.children.size(); ++i) {
          set_callsem_local_static_guard_symbol(
              var_node.children[i],
              callsem_local_static_guard_symbol(var_node));
        }
      }
      ctx.record_declaration_type_class_use_for_resolved_type_node(
          scope,
          *specifiers,
          type,
          ctx.source_location_for_node(*specifiers));
      decl_node.children.push_back(std::move(var_node));
    }
  }

  out.children.push_back(std::move(decl_node));
}

void analyze_for_init_statement(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & node,
                                DumpNode & out)
{
  if(node.kind != CppAstKind::for_init_statement) {
    throw logic_error("unsupported for-init-statement");
  }

  out = make_located_dump_node(CallSemKind::for_init_statement, node);
  if(node.children.empty()) {
    return;
  }
  if(node.children.size() != 1) {
    throw logic_error("invalid for-init-statement arity");
  }

  if(node.children[0].kind == CppAstKind::simple_declaration) {
    analyze_simple_declaration_statement(ctx, scope, node.children[0], out);
    return;
  }

  if(node.children[0].kind == CppAstKind::structured_binding_declaration) {
    analyze_structured_binding_declaration_statement(ctx, scope, node.children[0], out);
    return;
  }

  if(node.children[0].kind == CppAstKind::using_declaration) {
    semantic_declaration::collect_using_declaration(ctx, scope, node.children[0]);
    return;
  }

  if(node.children[0].kind == CppAstKind::alias_declaration) {
    const CppAstNode * type_id = find_child_kind(node.children[0], CppAstKind::type_id);
    if(!type_id) {
      throw logic_error("alias-declaration missing type-id");
    }
    CppAstNode prepared_type_id;
    if(!semantic_declaration::prepare_alias_type_id(
           ctx, scope, node.children[0], true, prepared_type_id)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    TypePtr alias;
    const ScopedStatementTemplateUseLocation use_location_guard(
        template_api::normalize_template_witness_source_location(
            ctx.source_location_for_node(*type_id)));
    if(!ctx.parse_type_id(scope, prepared_type_id, alias, true)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    alias = resolve_local_alias_type(ctx, scope, alias);
    semantic_scope_mutation::bind_named_type(scope, node.children[0].value, alias);
    DumpNode alias_node = make_dump_node(CallSemKind::type_alias, node.children[0].value);
    alias_node.semantic_type = alias;
    out.children.push_back(std::move(alias_node));
    return;
  }

  ExprInfo expr = ctx.analyze_expression(scope, node.children[0]);
  out.children.push_back(std::move(expr.node));
}

void analyze_range_for_statement(SemanticContext & ctx,
                                 Scope & scope,
                                 const TypePtr & return_type,
                                 const CppAstNode & node,
                                 DumpNode & out,
                                 std::vector<ExprInfo> * collected_returns = nullptr,
                                 bool * saw_void_return = nullptr,
                                 const TypePtr & active_switch_type = TypePtr())
{
  if(node.kind != CppAstKind::range_for_statement || node.children.size() != 3) {
    throw logic_error("malformed range-for statement");
  }

  const CppAstNode & range_decl = node.children[0];
  const CppAstNode & range_init = node.children[1];
  const CppAstNode & body_ast = node.children[2];
  const CppAstNode * specifiers = find_child_kind(range_decl, CppAstKind::decl_specifier_seq);
  const CppAstNode * declarator = find_child_kind(range_decl, CppAstKind::declarator);
  if(find_child_kind(range_decl, CppAstKind::structured_binding_declarator)) {
    throw logic_error("structured-binding range declarations are only structurally accepted in PA32");
  }
  if(!specifiers || !declarator || range_init.children.size() != 1) {
    throw logic_error("unsupported range-for declaration");
  }

  ExprInfo range_expr = range_init.children[0].kind == CppAstKind::braced_init_list ?
      ctx.analyze_braced_init_list_expression(scope, range_init.children[0]) :
      ctx.analyze_expression(scope, range_init.children[0]);
  TypePtr range_type = strip_top_level_cv(range_expr.type);
  TypePtr range_object_type = strip_top_level_cv(remove_reference_type(range_expr.type));
  TypePtr element_type;
  ClassInfo * range_initlist_info = nullptr;
  const bool range_is_initializer_list =
      ctx.is_initializer_list_type(range_expr.type, &element_type, &range_initlist_info);

  Scope body_scope(&scope);
  DumpNode outer = make_located_dump_node(CallSemKind::compound_statement, node);
  DumpNode range_base = range_expr.node;
  string range_name = range_expr.node.text;
  if(range_init.children[0].kind == CppAstKind::braced_init_list ||
     range_expr.category != VC_LVALUE ||
     range_expr.node.kind != CallSemKind::id_expression) {
    const string hidden_range = ctx.next_synthetic_local_name("range");
    TypePtr hidden_range_type = range_expr.type;
    if(range_init.children[0].kind != CppAstKind::braced_init_list &&
       range_expr.category == VC_LVALUE &&
       hidden_range_type) {
      hidden_range_type = make_lvalue_reference_raw(hidden_range_type);
    }
    semantic_scope_mutation::bind_value(
        body_scope,
        hidden_range,
        ValueBinding(ValueBinding::VK_VARIABLE, hidden_range, hidden_range_type));
    append_hidden_variable_declaration(
        outer, hidden_range, hidden_range_type, std::move(range_expr.node));
    range_base = make_id_expr_node(hidden_range, range_expr.type);
    range_name = hidden_range;
  }

  string loop_name;
  TypePtr loop_type;
  bool is_typedef = false;
  if(!range_type) {
    throw logic_error("range-for requires bounded array or std::initializer_list");
  }

  if(!range_is_initializer_list &&
     (range_type->kind != Type::TK_ARRAY || !range_type->has_bound)) {
    ClassInfo * range_info = ctx.complete_class_type(range_object_type);
    if(!range_info) {
      throw logic_error("range-for requires bounded array or std::initializer_list");
    }

    const bool use_member_begin_end =
        class_has_range_member_name(range_info, "begin") &&
        class_has_range_member_name(range_info, "end");
    const CppAstNode range_id = make_id_expr_ast_node(range_name);
    ExprInfo begin_expr =
        analyze_range_helper_expression(ctx,
                                        body_scope,
                                        use_member_begin_end ?
                                            make_call_expr_ast(make_member_expr_ast(range_id,
                                                                                    "begin")) :
                                            make_call_expr_ast(make_id_expr_ast_node("begin"),
                                                               vector<CppAstNode>(1, range_id)));
    ExprInfo end_expr =
        analyze_range_helper_expression(ctx,
                                        body_scope,
                                        use_member_begin_end ?
                                            make_call_expr_ast(make_member_expr_ast(range_id,
                                                                                    "end")) :
                                            make_call_expr_ast(make_id_expr_ast_node("end"),
                                                               vector<CppAstNode>(1, range_id)));
    const string begin_name = ctx.next_synthetic_local_name("begin");
    const string end_name = ctx.next_synthetic_local_name("end");
    std::vector<ValueBinding> iterator_bindings;
    iterator_bindings.push_back(
        ValueBinding(ValueBinding::VK_VARIABLE, begin_name, begin_expr.type));
    iterator_bindings.push_back(
        ValueBinding(ValueBinding::VK_VARIABLE, end_name, end_expr.type));
    semantic_scope_mutation::bind_values(body_scope, iterator_bindings);
    append_hidden_variable_declaration(
        outer, begin_name, begin_expr.type, std::move(begin_expr.node));
    append_hidden_variable_declaration(
        outer, end_name, end_expr.type, std::move(end_expr.node));

    ExprInfo element_expr =
        analyze_range_helper_expression(ctx,
                                        body_scope,
                                        make_unary_expr_ast_node(OP_STAR,
                                                                 "*",
                                                                 make_id_expr_ast_node(begin_name)));
    if(decl_spec_contains_auto(*specifiers)) {
      if(!ctx.parse_auto_declaration_type_from_expr(body_scope,
                                                    *specifiers,
                                                    *declarator,
                                                    element_expr,
                                                    loop_name,
                                                    loop_type)) {
        throw logic_error("unsupported range declaration");
      }
    } else if(!ctx.parse_variable_declaration_type(body_scope,
                                                   *specifiers,
                                                   *declarator,
                                                   nullptr,
                                                   false,
                                                   loop_name,
                                                   loop_type,
                                                   is_typedef) || is_typedef) {
      throw logic_error("unsupported range declaration");
    }

    if(loop_name.empty()) {
      throw logic_error("unsupported range declaration type");
    }
    semantic_scope_mutation::bind_value(
        body_scope, loop_name, ValueBinding(ValueBinding::VK_VARIABLE, loop_name, loop_type));

    DumpNode for_node = make_located_dump_node(CallSemKind::for_statement, node);
    for_node.children.push_back(make_located_dump_node(CallSemKind::for_init_statement, node));

    ExprInfo cond_expr =
        analyze_range_helper_expression(ctx,
                                        body_scope,
                                        make_binary_expr_ast_node(OP_NE,
                                                                  "!=",
                                                                  make_id_expr_ast_node(begin_name),
                                                                  make_id_expr_ast_node(end_name)));
    if(!condition_type_ok(ctx, body_scope, cond_expr)) {
      throw logic_error("invalid range-for condition");
    }
    DumpNode cond_node = make_dump_node(CallSemKind::condition);
    cond_node.children.push_back(std::move(cond_expr.node));
    for_node.children.push_back(std::move(cond_node));

    ExprInfo iter_expr =
        analyze_range_helper_expression(ctx,
                                        body_scope,
                                        make_unary_expr_ast_node(OP_INC,
                                                                 "++",
                                                                 make_id_expr_ast_node(begin_name)));
    DumpNode iter_node = make_dump_node(CallSemKind::iteration);
    iter_node.children.push_back(std::move(iter_expr.node));
    for_node.children.push_back(std::move(iter_node));

    DumpNode body_node = make_located_dump_node(CallSemKind::compound_statement, body_ast);
    DumpNode loop_var_decl = make_dump_node(CallSemKind::simple_declaration);
    DumpNode loop_var = make_dump_node(CallSemKind::variable, loop_name);
    loop_var.semantic_type = loop_type;
    loop_var.children.push_back(std::move(element_expr.node));
    loop_var_decl.children.push_back(std::move(loop_var));
    body_node.children.push_back(std::move(loop_var_decl));
    analyze_statement_impl(ctx,
                           body_scope,
                           return_type,
                           body_ast,
                           body_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    for_node.children.push_back(std::move(body_node));

    outer.children.push_back(std::move(for_node));
    out.children.push_back(std::move(outer));
    return;
  }

  if(!range_is_initializer_list) {
    element_type = range_type->inner;
  }
  if((range_type->kind != Type::TK_ARRAY || !range_type->has_bound) &&
     !range_is_initializer_list) {
    throw logic_error("range-for requires bounded array or std::initializer_list");
  }
  if(decl_spec_contains_auto(*specifiers)) {
    ExprInfo pseudo_element;
    pseudo_element.type = element_type;
    pseudo_element.category = VC_LVALUE;
    if(!ctx.parse_auto_declaration_type_from_expr(body_scope,
                                                  *specifiers,
                                                  *declarator,
                                                  pseudo_element,
                                                  loop_name,
                                                  loop_type)) {
      throw logic_error("unsupported range declaration");
    }
  } else if(!ctx.parse_variable_declaration_type(body_scope, *specifiers, *declarator, nullptr,
                                                 false, loop_name, loop_type, is_typedef) ||
            is_typedef) {
    throw logic_error("unsupported range declaration");
  }

  if(loop_name.empty()) {
    throw logic_error("unsupported range declaration type");
  }
  semantic_scope_mutation::bind_value(
      body_scope, loop_name, ValueBinding(ValueBinding::VK_VARIABLE, loop_name, loop_type));

  const string index_name = ctx.next_synthetic_local_name("idx");
  DumpNode idx_decl = make_located_dump_node(CallSemKind::simple_declaration, node);
  DumpNode idx_var = make_dump_node(CallSemKind::variable, index_name);
  idx_var.semantic_type = make_fundamental(FT_INT);
  idx_var.children.push_back(make_integer_literal_node(0));
  idx_decl.children.push_back(std::move(idx_var));

  DumpNode for_node = make_located_dump_node(CallSemKind::for_statement, node);
  DumpNode init_node = make_located_dump_node(CallSemKind::for_init_statement, node);
  init_node.children.push_back(std::move(idx_decl));
  for_node.children.push_back(std::move(init_node));

  DumpNode idx_id = make_id_expr_node(index_name, make_fundamental(FT_INT));
  DumpNode bound_expr;
  if(range_is_initializer_list) {
    ExprInfo range_base_expr;
    range_base_expr.type = range_expr.type;
    range_base_expr.category = VC_LVALUE;
    range_base_expr.node = range_base;
    bound_expr = ctx.make_field_expr(range_base_expr, range_initlist_info->fields[1]).node;
  } else {
    bound_expr = make_integer_literal_node(static_cast<long long>(range_type->bound));
  }
  DumpNode cond_expr =
      make_binary_expr_node(OP_LT, "<", make_fundamental(FT_INT), idx_id, bound_expr);
  DumpNode cond_node = make_dump_node(CallSemKind::condition);
  cond_node.children.push_back(std::move(cond_expr));
  for_node.children.push_back(std::move(cond_node));

  DumpNode iter_node = make_dump_node(CallSemKind::iteration);
  iter_node.children.push_back(
      make_postfix_expr_node(OP_INC, "++", make_fundamental(FT_INT), idx_id));
  for_node.children.push_back(std::move(iter_node));

  DumpNode body_node = make_located_dump_node(CallSemKind::compound_statement, body_ast);
  DumpNode loop_var_decl = make_dump_node(CallSemKind::simple_declaration);
  DumpNode loop_var = make_dump_node(CallSemKind::variable, loop_name);
  loop_var.semantic_type = loop_type;
  DumpNode element_base = range_base;
  if(range_is_initializer_list) {
    ExprInfo range_base_expr;
    range_base_expr.type = range_expr.type;
    range_base_expr.category = VC_LVALUE;
    range_base_expr.node = range_base;
    element_base = ctx.make_field_expr(range_base_expr, range_initlist_info->fields[0]).node;
  }
  loop_var.children.push_back(
      make_subscript_expr_node(element_base,
                               make_id_expr_node(index_name, make_fundamental(FT_INT)),
                               element_type));
  loop_var_decl.children.push_back(std::move(loop_var));
  body_node.children.push_back(std::move(loop_var_decl));
  analyze_statement_impl(ctx,
                         body_scope,
                         return_type,
                         body_ast,
                         body_node,
                         collected_returns,
                         saw_void_return,
                         active_switch_type);
  for_node.children.push_back(std::move(body_node));

  outer.children.push_back(std::move(for_node));
  out.children.push_back(std::move(outer));
}

void analyze_statement_impl(SemanticContext & ctx,
                            Scope & scope,
                            const TypePtr & return_type,
                            const CppAstNode & node,
                            DumpNode & out,
                            std::vector<ExprInfo> * collected_returns,
                            bool * saw_void_return,
                            const TypePtr & active_switch_type)
{
  if(semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters()) {
    ++counters->statement_analysis_by_demand[
        static_cast<std::size_t>(semantic_metrics::current_class_demand())];
  }
  if(active_switch_type && node.kind == CppAstKind::case_statement) {
    if(node.children.size() != 2) {
      throw logic_error("invalid case-statement");
    }
    DumpNode case_node = make_located_dump_node(CallSemKind::case_statement, node);
    ExprInfo case_expr =
        ctx.analyze_expression_for_target(scope, node.children[0], active_switch_type);
    case_node.children.push_back(std::move(case_expr.node));
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[1],
                           case_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    out.children.push_back(std::move(case_node));
    return;
  }

  if(active_switch_type && node.kind == CppAstKind::default_statement) {
    if(node.children.size() != 1) {
      throw logic_error("invalid default-statement");
    }
    DumpNode default_node = make_located_dump_node(CallSemKind::default_statement, node);
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[0],
                           default_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    out.children.push_back(std::move(default_node));
    return;
  }

  if(node.kind == CppAstKind::attributed_statement) {
    if(node.children.size() != 1) {
      throw logic_error("invalid attributed-statement");
    }
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[0],
                           out,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    return;
  }

  if(node.kind == CppAstKind::static_assert_declaration) {
    semantic_declaration::analyze_static_assert_declaration(ctx, scope, node);
    return;
  }

  if(node.kind == CppAstKind::simple_declaration) {
    analyze_simple_declaration_statement(ctx, scope, node, out);
    return;
  }

  if(node.kind == CppAstKind::structured_binding_declaration) {
    analyze_structured_binding_declaration_statement(ctx, scope, node, out);
    return;
  }

  if(node.kind == CppAstKind::using_declaration) {
    semantic_declaration::collect_using_declaration(ctx, scope, node);
    return;
  }

  if(node.kind == CppAstKind::namespace_alias_definition) {
    semantic_declaration::collect_namespace_alias_definition(ctx, scope, node);
    return;
  }

  if(node.kind == CppAstKind::using_directive) {
    semantic_declaration::collect_using_directive(ctx, scope, node);
    return;
  }

  if(node.kind == CppAstKind::alias_declaration) {
    const CppAstNode * type_id = find_child_kind(node, CppAstKind::type_id);
    if(!type_id) {
      throw logic_error("alias-declaration missing type-id");
    }
    CppAstNode prepared_type_id;
    if(!semantic_declaration::prepare_alias_type_id(
           ctx, scope, node, true, prepared_type_id)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    TypePtr alias;
    const ScopedStatementTemplateUseLocation use_location_guard(
        template_api::normalize_template_witness_source_location(
            ctx.source_location_for_node(*type_id)));
    if(!ctx.parse_type_id(scope, prepared_type_id, alias, true)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    alias = resolve_local_alias_type(ctx, scope, alias);
    semantic_scope_mutation::bind_named_type(scope, node.value, alias);
    DumpNode alias_node = make_dump_node(CallSemKind::type_alias, node.value);
    alias_node.semantic_type = alias;
    out.children.push_back(std::move(alias_node));
    return;
  }

  if(node.kind == CppAstKind::return_statement) {
    if(collected_returns || saw_void_return) {
      DumpNode return_node = make_located_dump_node(CallSemKind::return_statement, node);
      if(node.children.empty()) {
        if(saw_void_return) {
          *saw_void_return = true;
        }
      } else {
        ExprInfo expr = ctx.analyze_expression(scope, node.children[0]);
        if(collected_returns) {
          collected_returns->push_back(expr);
        }
        return_node.children.push_back(std::move(expr.node));
      }
      out.children.push_back(std::move(return_node));
      return;
    }

    DumpNode return_node = make_located_dump_node(CallSemKind::return_statement, node);
    if(node.children.empty()) {
      if(!is_void_type(return_type)) {
        throw logic_error("non-void return without expression");
      }
    } else {
      ExprInfo expr = ctx.analyze_expression_for_target(scope, node.children[0], return_type);
      if(!can_copy_initialize(ctx, return_type, expr)) {
        ostringstream err;
        err << "invalid return expression";
        err << " [target " << describe_type(return_type) << "]";
        err << " [expr " << node_text(node.children[0]) << "]";
        err << " [expr_type " << describe_type(expr.type) << "]";
        err << " [expr_category "
            << call_value_category_text(to_call_value_category(expr.category)) << "]";
        throw logic_error(err.str());
      }
      return_node.children.push_back(std::move(expr.node));
    }
    out.children.push_back(std::move(return_node));
    return;
  }

  if(node.kind == CppAstKind::throw_statement) {
    DumpNode throw_node = make_located_dump_node(CallSemKind::throw_statement, node);
    if(node.children.size() > 1) {
      throw logic_error("invalid throw-statement");
    }
    if(!node.children.empty()) {
      ExprInfo thrown = ctx.analyze_expression(scope, node.children[0]);
      TypePtr thrown_type = catch_match_type(thrown.type);
      if(thrown_type) {
        ctx.note_rtti_use(thrown_type, false);
      }
      throw_node.children.push_back(std::move(thrown.node));
    }
    out.children.push_back(std::move(throw_node));
    return;
  }

  if(node.kind == CppAstKind::try_block) {
    if(node.children.size() < 2) {
      throw logic_error("invalid try-block");
    }

    DumpNode try_node = make_located_dump_node(CallSemKind::try_statement, node);
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[0],
                           try_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);

    for(size_t i = 1; i < node.children.size(); ++i) {
      const CppAstNode & handler = node.children[i];
      if(handler.kind != CppAstKind::handler || handler.children.size() != 2) {
        throw logic_error("invalid catch handler");
      }

      Scope handler_scope(&scope);
      DumpNode handler_node = make_located_dump_node(CallSemKind::catch_handler, handler);
      analyze_exception_declaration(ctx, handler_scope, handler.children[0], handler_node);
      analyze_statement_impl(ctx,
                             handler_scope,
                             return_type,
                             handler.children[1],
                             handler_node,
                             collected_returns,
                             saw_void_return,
                             active_switch_type);
      try_node.children.push_back(std::move(handler_node));
    }

    out.children.push_back(std::move(try_node));
    return;
  }

  if(node.kind == CppAstKind::asm_statement) {
    DumpNode asm_node = make_located_dump_node(CallSemKind::asm_statement,
                                               node,
                                               node_text(node));
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CppAstKind::asm_clause) {
        asm_node.children.push_back(make_located_dump_node(CallSemKind::asm_clause,
                                                           node.children[i],
                                                           node_text(node.children[i])));
      }
    }
    out.children.push_back(std::move(asm_node));
    return;
  }

  if(node.kind == CppAstKind::expression_statement) {
    DumpNode expr_stmt = make_located_dump_node(CallSemKind::expression_statement, node);
    if(!node.children.empty()) {
      ExprInfo expr = ctx.analyze_expression(scope, node.children[0]);
      expr_stmt.children.push_back(std::move(expr.node));
    }
    out.children.push_back(std::move(expr_stmt));
    return;
  }

  if(node.kind == CppAstKind::if_statement) {
    if(node.children.size() < 2) {
      throw logic_error("invalid if-statement");
    }

    Scope condition_scope(&scope);
    const CppAstNode * init_stmt = find_child_kind(node, CppAstKind::for_init_statement);
    const CppAstNode * condition = find_child_kind(node, CppAstKind::condition);
    const CppAstNode * then_branch = find_child_kind(node, CppAstKind::then_node);
    if(!condition || !then_branch || then_branch->children.size() != 1) {
      throw logic_error("unsupported if-statement");
    }

    DumpNode init_node;
    if(init_stmt) {
      analyze_for_init_statement(ctx, condition_scope, *init_stmt, init_node);
    }

    if(node.value == "constexpr" &&
       condition->children.size() == 1 &&
       condition->children[0].kind != CppAstKind::condition_declaration) {
      long long constant_value = 0;
      if(ctx.evaluate_constant_expression(condition_scope, condition->children[0], constant_value)) {
        if(init_stmt) {
          out.children.push_back(std::move(init_node));
        }
        const bool take_then = constant_value != 0;
        const CppAstNode * else_branch = find_child_kind(node, CppAstKind::else_node);
        if(take_then) {
          analyze_statement_impl(ctx,
                                 condition_scope,
                                 return_type,
                                 then_branch->children[0],
                                 out,
                                 collected_returns,
                                 saw_void_return,
                                 active_switch_type);
        } else if(else_branch && else_branch->children.size() == 1) {
          analyze_statement_impl(ctx,
                                 condition_scope,
                                 return_type,
                                 else_branch->children[0],
                                 out,
                                 collected_returns,
                                 saw_void_return,
                                 active_switch_type);
        }
        return;
      }
    }

    DumpNode if_node = make_located_dump_node(CallSemKind::if_statement, node);
    if(init_stmt) {
      if_node.children.push_back(std::move(init_node));
    }

    DumpNode condition_node;
    analyze_condition_node(ctx, condition_scope, *condition, condition_node);
    if_node.children.push_back(std::move(condition_node));

    DumpNode then_node = make_located_dump_node(CallSemKind::then_node, *then_branch);
    analyze_statement_impl(ctx,
                           condition_scope,
                           return_type,
                           then_branch->children[0],
                           then_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    if_node.children.push_back(std::move(then_node));

    const CppAstNode * else_branch = find_child_kind(node, CppAstKind::else_node);
    if(else_branch) {
      if(else_branch->children.size() != 1) {
        throw logic_error("unsupported else-statement");
      }
      DumpNode else_node = make_located_dump_node(CallSemKind::else_node, *else_branch);
      analyze_statement_impl(ctx,
                             condition_scope,
                             return_type,
                             else_branch->children[0],
                             else_node,
                             collected_returns,
                             saw_void_return,
                             active_switch_type);
      if_node.children.push_back(std::move(else_node));
    }

    out.children.push_back(std::move(if_node));
    return;
  }

  if(node.kind == CppAstKind::while_statement) {
    if(node.children.size() != 2) {
      throw logic_error("invalid while-statement");
    }

    Scope condition_scope(&scope);
    DumpNode while_node = make_located_dump_node(CallSemKind::while_statement, node);

    DumpNode condition_node;
    analyze_condition_node(ctx, condition_scope, node.children[0], condition_node);
    while_node.children.push_back(std::move(condition_node));
    analyze_statement_impl(ctx,
                           condition_scope,
                           return_type,
                           node.children[1],
                           while_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    out.children.push_back(std::move(while_node));
    return;
  }

  if(node.kind == CppAstKind::do_statement) {
    if(node.children.size() != 2 || node.children[1].kind != CppAstKind::condition) {
      throw logic_error("invalid do-statement");
    }

    DumpNode do_node = make_located_dump_node(CallSemKind::do_statement, node);
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[0],
                           do_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    DumpNode condition_node;
    analyze_condition_node(ctx, scope, node.children[1], condition_node);
    do_node.children.push_back(std::move(condition_node));
    out.children.push_back(std::move(do_node));
    return;
  }

  if(node.kind == CppAstKind::switch_statement) {
    analyze_switch_statement(ctx,
                             scope,
                             return_type,
                             node,
                             out,
                             collected_returns,
                             saw_void_return);
    return;
  }

  if(node.kind == CppAstKind::for_statement) {
    if(node.children.empty()) {
      throw logic_error("invalid for-statement");
    }

    Scope for_scope(&scope);
    DumpNode for_node = make_located_dump_node(CallSemKind::for_statement, node);

    size_t index = 0;
    if(node.children[index].kind != CppAstKind::for_init_statement) {
      throw logic_error("for-statement missing init");
    }
    DumpNode init_node;
    analyze_for_init_statement(ctx, for_scope, node.children[index], init_node);
    for_node.children.push_back(std::move(init_node));
    ++index;

    if(index < node.children.size() && node.children[index].kind == CppAstKind::condition) {
      DumpNode condition_node;
      analyze_condition_node(ctx, for_scope, node.children[index], condition_node);
      for_node.children.push_back(std::move(condition_node));
      ++index;
    }

    if(index < node.children.size() && node.children[index].kind == CppAstKind::iteration) {
      if(node.children[index].children.size() != 1) {
        throw logic_error("invalid iteration");
      }
      DumpNode iteration_node = make_located_dump_node(CallSemKind::iteration,
                                                       node.children[index]);
      ExprInfo iteration_expr =
          ctx.analyze_expression(for_scope, node.children[index].children[0]);
      iteration_node.children.push_back(std::move(iteration_expr.node));
      for_node.children.push_back(std::move(iteration_node));
      ++index;
    }

    if(index >= node.children.size()) {
      throw logic_error("for-statement missing body");
    }
    analyze_statement_impl(ctx,
                           for_scope,
                           return_type,
                           node.children[index],
                           for_node,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    out.children.push_back(std::move(for_node));
    return;
  }

  if(node.kind == CppAstKind::range_for_statement) {
    analyze_range_for_statement(ctx,
                                scope,
                                return_type,
                                node,
                                out,
                                collected_returns,
                                saw_void_return,
                                active_switch_type);
    return;
  }

  if(node.kind == CppAstKind::break_statement || node.kind == CppAstKind::continue_statement) {
    out.children.push_back(make_located_dump_node(
        node.kind == CppAstKind::break_statement ? CallSemKind::break_statement :
                                                   CallSemKind::continue_statement,
        node));
    return;
  }

  if(node.kind == CppAstKind::labeled_statement) {
    if(node.children.size() != 1) {
      throw logic_error("invalid labeled-statement");
    }
    DumpNode labeled = make_located_dump_node(CallSemKind::labeled_statement, node, node.value);
    analyze_statement_impl(ctx,
                           scope,
                           return_type,
                           node.children[0],
                           labeled,
                           collected_returns,
                           saw_void_return,
                           active_switch_type);
    out.children.push_back(std::move(labeled));
    return;
  }

  if(node.kind == CppAstKind::goto_statement) {
    out.children.push_back(make_located_dump_node(CallSemKind::goto_statement, node, node.value));
    return;
  }

  if(node.kind == CppAstKind::compound_statement) {
    Scope block_scope(&scope);
    DumpNode block = make_located_dump_node(CallSemKind::compound_statement, node);
    for(size_t i = 0; i < node.children.size(); ++i) {
      analyze_statement_impl(ctx,
                             block_scope,
                             return_type,
                             node.children[i],
                             block,
                             collected_returns,
                             saw_void_return,
                             active_switch_type);
    }
    out.children.push_back(std::move(block));
    return;
  }

  if(node.kind == CppAstKind::class_specifier ||
     node.kind == CppAstKind::class_forward_declaration) {
    CppAstNode synthetic_decl;
    std::string synthetic_type_name;
    std::string synthetic_storage_name;
    if(semantic_class_model::synthesize_anonymous_union_storage_declaration(node,
                                                                            synthetic_decl,
                                                                            synthetic_type_name,
                                                                            synthetic_storage_name)) {
      const CppAstNode * owned_synthetic_decl = ctx.own_synthetic_ast(std::move(synthetic_decl));
      analyze_simple_declaration_statement(ctx, scope, *owned_synthetic_decl, out);
      postprocess_anonymous_union_storage(ctx, scope, node);
      return;
    }

    if(node.value.empty()) {
      throw logic_error("anonymous local class unsupported");
    }
    ctx.collect_class_declaration(scope, node);
    track_embedded_local_class_output(ctx, scope, node);
    out.children.push_back(make_dump_node(CallSemKind::simple_declaration));
    return;
  }

  if(node.kind == CppAstKind::enum_specifier) {
    ctx.collect_enum_declaration(scope, node);
    out.children.push_back(make_dump_node(CallSemKind::simple_declaration));
    return;
  }

  if(node.kind == CppAstKind::empty_declaration) {
    out.children.push_back(make_dump_node(CallSemKind::simple_declaration));
    return;
  }

  {
    ostringstream outmsg;
    outmsg << "unsupported statement in PA12 first slice kind="
           << cppast_kind_text(node.kind);
    if(!node.value.empty()) {
      outmsg << " value=" << node.value;
    }
    throw logic_error(outmsg.str());
  }
}

}  // namespace

void analyze_statement(SemanticContext & ctx,
                       Scope & scope,
                       const TypePtr & return_type,
                       const CppAstNode & node,
                       DumpNode & out)
{
  DIAG_CONTEXT("analyze_statement [" + std::string(cppast_kind_text(node.kind)) + "]" +
               ctx.source_location_for_node(node));
  analyze_statement_impl(ctx, scope, return_type, node, out, nullptr, nullptr);
}

void collect_return_expressions(SemanticContext & ctx,
                                Scope & scope,
                                const CppAstNode & node,
                                std::vector<ExprInfo> & out,
                                bool & saw_void_return)
{
  CallSemNode ignored;
  saw_void_return = false;
  analyze_statement_impl(ctx, scope, TypePtr(), node, ignored, &out, &saw_void_return);
}

void analyze_compound_body_and_collect_returns(SemanticContext & ctx,
                                               Scope & scope,
                                               const CppAstNode & node,
                                               CallSemNode & out,
                                               std::vector<ExprInfo> & collected_returns,
                                               bool & saw_void_return)
{
  if(node.kind != CppAstKind::compound_statement) {
    throw logic_error("body must be compound-statement");
  }
  saw_void_return = false;
  for(size_t i = 0; i < node.children.size(); ++i) {
    analyze_statement_impl(ctx,
                           scope,
                           TypePtr(),
                           node.children[i],
                           out,
                           &collected_returns,
                           &saw_void_return);
  }
}

}  // namespace semantic_statement
