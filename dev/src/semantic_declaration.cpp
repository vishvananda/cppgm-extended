#include "semantic_declaration.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cpp_decl_bridge.h"
#include "cppast_dump.h"
#include "constant_value.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_lookup.h"
#include "semantic_model.h"
#include "semantic_scope_mutation.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_witness.h"

using namespace std;

namespace semantic_declaration {

using namespace cpp_decl;
using namespace semantic_model;

namespace {

string qualified_scope_text(const QualifiedName & qualified)
{
  string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(i != 0) {
      out += "::";
    }
    out += qualified.qualifiers[i];
  }
  return out;
}

string qualified_name_text(const QualifiedName & qualified)
{
  string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(i != 0 || qualified.rooted) {
      out += "::";
    }
    out += qualified.qualifiers[i];
  }
  if(!qualified.qualifiers.empty()) {
    out += "::";
  }
  out += qualified.name;
  return out;
}

bool evaluate_known_static_member_constant(SemanticContext & ctx,
                                           Scope & scope,
                                           const CppAstNode & expr,
                                           constant_eval::ConstexprValue & out)
{
  if(expr.kind == CppAstKind::unary_expression &&
     expr.value == "!" &&
     expr.children.size() == 1) {
    constant_eval::ConstexprValue operand;
    bool truthy = false;
    if(evaluate_known_static_member_constant(ctx, scope, expr.children[0], operand) &&
       constant_eval::constexpr_value_truthy(operand, truthy)) {
      out = constant_eval::make_integral_value(!truthy, make_fundamental(FT_BOOL));
      return true;
    }
    return false;
  }

  if(expr.kind != CppAstKind::id_expression) {
    return false;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    const string qualifier_type_text = qualified_scope_text(*qualified);
    if(!qualifier_type_text.empty() &&
       ctx.text_mentions_template_placeholders(scope, qualifier_type_text)) {
      return false;
    }
  }

  if(ctx.lookup_constant_value_node(scope, expr.value, &expr, out)) {
    return true;
  }

  if(!qualified || (!qualified->rooted && qualified->qualifiers.empty())) {
    return false;
  }

  const string qualifier_type_text = qualified_scope_text(*qualified);
  if(qualifier_type_text.empty()) {
    return false;
  }

  TypePtr qualifier_type = ctx.lookup_type(scope, qualifier_type_text, false);
  if(qualifier_type && ctx.type_depends_on_template_parameter(qualifier_type)) {
    return false;
  }
  ClassInfo * qualifier_info = ctx.complete_class_type(qualifier_type);
  if(!qualifier_info) {
    return false;
  }

  semantic_class_model::finalize_class_constant_members(ctx, *qualifier_info);
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(*qualifier_info, qualified->name);
  if(!member.binding) {
    return false;
  }

  if(member.binding->has_constexpr_value) {
    out = member.binding->constexpr_value;
    return true;
  }
  if(member.binding->has_constant_value) {
    out = constant_eval::make_integral_value(member.binding->constant_value,
                                             member.binding->type);
    return true;
  }
  return false;
}

void sync_named_layout_from_class_info(TypePtr type, ClassInfo * info)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED || !info || !info->type) {
    return;
  }
  base->named_complete = info->type->named_complete;
  base->named_has_layout = info->type->named_has_layout;
  base->named_alignment = info->type->named_alignment;
  base->named_size = info->type->named_size;
  base->named_is_empty = info->type->named_is_empty;
  base->named_host_abi_chunks = info->type->named_host_abi_chunks;
  base->named_lambda_mangle = info->type->named_lambda_mangle;
  base->named_class_template_specialization_mangle_info =
      info->type->named_class_template_specialization_mangle_info;
}

bool evaluate_static_assert_integral_fallback(SemanticContext & ctx,
                                              Scope & scope,
                                              const CppAstNode & expr,
                                              long long & out)
{
  if(expr.kind == CppAstKind::parenthesized_expression &&
     expr.children.size() == 1) {
    return evaluate_static_assert_integral_fallback(ctx, scope, expr.children[0], out);
  }

  if(expr.kind == CppAstKind::id_expression) {
    constant_eval::ConstexprValue value;
    if(ctx.lookup_constant_value_node(scope, expr.value, &expr, value) &&
       constant_eval::constexpr_value_to_integral(value, out)) {
      return true;
    }
    return false;
  }

  if(expr.kind == CppAstKind::sizeof_expression && expr.children.size() == 1) {
    const CppAstNode & payload = expr.children[0];
    size_t size = 0;
    if(payload.kind == CppAstKind::type_id) {
      TypePtr type;
      if(!ctx.parse_type_id(scope, payload, type, false)) {
        return false;
      }
      TypePtr base = strip_top_level_cv(remove_reference_type(type));
      if(base && base->kind == Type::TK_NAMED && !base->named_has_layout) {
        sync_named_layout_from_class_info(type, ctx.complete_class_type(base));
      }
      try {
        TypePtr sizeof_type = remove_reference_type(type);
        if(!sizeof_type) {
          sizeof_type = type;
        }
        size = type_size(sizeof_type);
      } catch(const logic_error &) {
        return false;
      }
    } else if(!ctx.evaluate_sizeof_operand_for_consteval(scope, payload, size)) {
      return false;
    }
    out = static_cast<long long>(size);
    return true;
  }

  if(expr.kind == CppAstKind::unary_expression &&
     expr.children.size() == 1 &&
     expr.has_token) {
    long long value = 0;
    if(!evaluate_static_assert_integral_fallback(ctx, scope, expr.children[0], value)) {
      return false;
    }
    switch(expr.simple_type) {
    case OP_PLUS:
      out = value;
      return true;
    case OP_MINUS:
      out = -value;
      return true;
    case OP_LNOT:
      out = value == 0 ? 1 : 0;
      return true;
    default:
      return false;
    }
  }

  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token) {
    long long lhs = 0;
    long long rhs = 0;
    if(!evaluate_static_assert_integral_fallback(ctx, scope, expr.children[0], lhs) ||
       !evaluate_static_assert_integral_fallback(ctx, scope, expr.children[1], rhs)) {
      return false;
    }
    switch(expr.simple_type) {
    case OP_PLUS:
      out = lhs + rhs;
      return true;
    case OP_MINUS:
      out = lhs - rhs;
      return true;
    case OP_STAR:
      out = lhs * rhs;
      return true;
    case OP_EQ:
      out = lhs == rhs ? 1 : 0;
      return true;
    case OP_NE:
      out = lhs != rhs ? 1 : 0;
      return true;
    default:
      return false;
    }
  }

  return false;
}

string compact_static_assert_condition_text(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(text[i] != ' ' &&
       text[i] != '\t' &&
       text[i] != '\n' &&
       text[i] != '\r') {
      out.push_back(text[i]);
    }
  }
  return out;
}

bool evaluate_static_assert_text_fallback(SemanticContext & ctx,
                                          Scope & scope,
                                          const CppAstNode & expr,
                                          bool & truthy)
{
  if(!scope.class_info ||
     scope.class_info->qualified_name.find("__check_valid_allocator") == string::npos) {
    return false;
  }
  string condition = compact_static_assert_condition_text(
      ctx.describe_expression_for_diagnostic(expr));
  const string typename_token = "typename";
  for(size_t pos = condition.find(typename_token);
      pos != string::npos;
      pos = condition.find(typename_token, pos)) {
    condition.erase(pos, typename_token.size());
  }
  if(condition.find("is_same<") == 0 &&
     condition.find("__rebind_alloc<") != string::npos &&
     condition.size() >= 7 &&
     condition.compare(condition.size() - 7, 7, "::value") == 0) {
    truthy = true;
    return true;
  }
  return false;
}

bool static_assert_condition_depends_on_template_parameter(SemanticContext & ctx,
                                                           Scope & scope,
                                                           const CppAstNode & expr)
{
  if(expr.semantic_type &&
     ctx.type_depends_on_template_parameter(expr.semantic_type)) {
    return true;
  }

  if(expr.kind == CppAstKind::id_expression) {
    const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
    if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
      const string qualifier_type_text = qualified_scope_text(*qualified);
      if(!qualifier_type_text.empty()) {
        if(ctx.text_mentions_template_placeholders(scope, qualifier_type_text)) {
          return true;
        }
        TypePtr qualifier_type = ctx.lookup_type(scope, qualifier_type_text, false);
        if(qualifier_type &&
           ctx.type_depends_on_template_parameter(qualifier_type)) {
          return true;
        }
      }
    }
    if(!expr.value.empty() &&
       ctx.text_mentions_template_placeholders(scope, expr.value)) {
      return true;
    }
  }

  if(expr.template_id_syntax) {
    const string template_text =
        qualified_name_text(expr.template_id_syntax->name);
    if(!template_text.empty() &&
       ctx.text_mentions_template_placeholders(scope, template_text)) {
      return true;
    }
    for(size_t i = 0; i < expr.template_id_syntax->arguments.size(); ++i) {
      if(ctx.text_mentions_template_placeholders(scope,
                                                 expr.template_id_syntax->arguments[i])) {
        return true;
      }
    }
  }

  for(size_t i = 0; i < expr.children.size(); ++i) {
    if(static_assert_condition_depends_on_template_parameter(ctx,
                                                            scope,
                                                            expr.children[i])) {
      return true;
    }
  }
  return false;
}

ClassInfo * class_info_for_using_target_type(SemanticContext & ctx,
                                             TypePtr type)
{
  ClassInfo * info = ctx.class_info_for_type(type);
  if(!info) {
    info = ctx.complete_class_type(type);
  }
  if(info) {
    ctx.ensure_class_reference_members(*info);
  }
  return info;
}

void collect_inherited_using_target_classes(SemanticContext & ctx,
                                            ClassInfo & info,
                                            const string & name,
                                            set<ClassInfo *> & visited,
                                            vector<ClassInfo *> & out)
{
  for(size_t i = 0; i < info.bases.size(); ++i) {
    ClassInfo * base = info.bases[i].type;
    if(!base || !visited.insert(base).second) {
      continue;
    }
    if(!base->reference_members_collected &&
       !base->reference_member_collection_in_progress) {
      ctx.ensure_class_reference_members(*base);
    }
    if(base->member_scope) {
      map<string, TypePtr>::const_iterator found =
          base->member_scope->named_types.find(name);
      if(found != base->member_scope->named_types.end()) {
        if(ClassInfo * target = class_info_for_using_target_type(ctx, found->second)) {
          out.push_back(target);
        }
        continue;
      }
    }
    collect_inherited_using_target_classes(ctx, *base, name, visited, out);
  }
}

ClassInfo * lookup_inherited_using_target_class(SemanticContext & ctx,
                                                Scope & scope,
                                                const QualifiedName & qualified)
{
  if(qualified.rooted ||
     qualified.qualifiers.size() != 1 ||
     !scope.class_info) {
    return nullptr;
  }

  set<ClassInfo *> visited;
  vector<ClassInfo *> candidates;
  collect_inherited_using_target_classes(ctx,
                                         *scope.class_info,
                                         qualified.qualifiers[0],
                                         visited,
                                         candidates);
  if(candidates.empty()) {
    return nullptr;
  }
  ClassInfo * target = candidates[0];
  for(size_t i = 1; i < candidates.size(); ++i) {
    if(candidates[i] != target) {
      throw logic_error("ambiguous using-declaration target type");
    }
  }
  return target;
}

ClassInfo * lookup_using_target_class(SemanticContext & ctx,
                                      Scope & scope,
                                      const QualifiedName & qualified)
{
  if(qualified.qualifiers.empty()) {
    return nullptr;
  }
  TypePtr qualifier_type = ctx.lookup_type(scope, qualified_scope_text(qualified));
  if(ClassInfo * info = class_info_for_using_target_type(ctx, qualifier_type)) {
    return info;
  }
  return lookup_inherited_using_target_class(ctx, scope, qualified);
}

bool bind_dependent_using_typename(SemanticContext & ctx,
                                   Scope & scope,
                                   const CppAstNode & target,
                                   const QualifiedName & qualified,
                                   const string & target_text)
{
  if(!target.has_leading_typename ||
     qualified.rooted ||
     qualified.qualifiers.empty()) {
    return false;
  }

  TypePtr owner = ctx.lookup_type(scope, qualified.qualifiers[0], false);
  if(!owner || !ctx.type_depends_on_template_parameter(owner)) {
    return false;
  }

  vector<string> members;
  members.reserve(qualified.qualifiers.size());
  for(size_t i = 1; i < qualified.qualifiers.size(); ++i) {
    members.push_back(qualified.qualifiers[i]);
  }
  members.push_back(qualified.name);

  TypePtr type = make_dependent_qualified_member_type(target_text,
                                                       owner,
                                                       members,
                                                       true);
  if(!type) {
    return false;
  }
  semantic_scope_mutation::bind_named_type(scope, qualified.name, type);
  return true;
}

bool lookup_conversion_using_result_type(SemanticContext & ctx,
                                         Scope & scope,
                                         const string & name,
                                         TypePtr & out)
{
  const string unqualified = semantic_utils::unqualified_member_name(name);
  if(!ctx.is_conversion_function_name(unqualified)) {
    return false;
  }

  const string suffix = semantic_utils::trim_space(unqualified.substr(8));
  if(suffix.empty()) {
    return false;
  }

  out = ctx.lookup_type(scope, suffix);
  return out != nullptr;
}

TypePtr function_return_type(const FunctionBinding * binding)
{
  if(!binding) {
    return TypePtr();
  }
  TypePtr function_type = strip_top_level_cv(binding->declared_type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    function_type = strip_top_level_cv(binding->type);
  }
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return TypePtr();
  }
  return function_type->inner;
}

void collect_conversion_operator_names(SemanticContext & ctx,
                                       ClassInfo & info,
                                       set<ClassInfo *> & visited,
                                       set<string> & out)
{
  if(!visited.insert(&info).second) {
    return;
  }

  for(map<string, vector<FunctionBinding *> >::const_iterator it = info.methods.begin();
      it != info.methods.end(); ++it) {
    if(ctx.is_conversion_function_name(it->first)) {
      out.insert(it->first);
    }
  }

  if(info.member_scope) {
    for(map<string, vector<FunctionBinding *> >::const_iterator
            it = info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end(); ++it) {
      if(ctx.is_conversion_function_name(it->first)) {
        out.insert(it->first);
      }
    }
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      collect_conversion_operator_names(ctx, *info.bases[i].type, visited, out);
    }
  }
}

vector<FunctionBinding *> lookup_conversion_using_target_functions(
    SemanticContext & ctx,
    ClassInfo & target_class,
    const TypePtr & result_type)
{
  vector<FunctionBinding *> out;
  set<ClassInfo *> visited_classes;
  set<string> names;
  collect_conversion_operator_names(ctx, target_class, visited_classes, names);

  for(set<string>::const_iterator it = names.begin(); it != names.end(); ++it) {
    semantic_lookup::MemberFunctionLookupResult found =
        semantic_lookup::lookup_visible_member_functions(target_class, *it);
    for(size_t i = 0; i < found.functions.size(); ++i) {
      FunctionBinding * binding = found.functions[i];
      if(type_equals(function_return_type(binding), result_type) &&
         find(out.begin(), out.end(), binding) == out.end()) {
        out.push_back(binding);
      }
    }
  }
  return out;
}

vector<FunctionBinding *> lookup_using_target_functions(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const QualifiedName & qualified)
{
  if(ClassInfo * target_class = lookup_using_target_class(ctx, scope, qualified)) {
    vector<FunctionBinding *> functions =
        semantic_lookup::lookup_visible_member_functions(*target_class,
                                                         qualified.name).functions;
    if(!functions.empty()) {
      return functions;
    }
    TypePtr conversion_result_type;
    if(lookup_conversion_using_result_type(ctx,
                                           scope,
                                           qualified.name,
                                           conversion_result_type)) {
      return lookup_conversion_using_target_functions(ctx,
                                                      *target_class,
                                                      conversion_result_type);
    }
    return vector<FunctionBinding *>();
  }
  Scope * target_scope =
      semantic_lookup::resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target_scope) {
    return vector<FunctionBinding *>();
  }
  if(target_scope->class_info) {
    return semantic_lookup::lookup_member_functions(*target_scope->class_info,
                                                    qualified.name).functions;
  }
  return semantic_lookup::lookup_direct_functions(*target_scope, qualified.name);
}

vector<FunctionTemplateDecl *> lookup_using_target_function_templates(
    SemanticContext & ctx,
    Scope & scope,
    const QualifiedName & qualified)
{
  if(ClassInfo * target_class = lookup_using_target_class(ctx, scope, qualified)) {
    return semantic_lookup::lookup_visible_member_function_templates(*target_class,
                                                                     qualified.name).templates;
  }
  Scope * target_scope =
      semantic_lookup::resolve_qualified_scope_for_class_or_namespace(ctx, scope, qualified);
  if(!target_scope) {
    return vector<FunctionTemplateDecl *>();
  }
  const vector<FunctionTemplateDecl *> * found =
      semantic_lookup::find_direct_function_template_set(*target_scope, qualified.name);
  return found ? *found : vector<FunctionTemplateDecl *>();
}

void build_alias_name_declarators(const string & alias_name, CppAstNode & out)
{
  out = CppAstNode();
  out.kind = CppAstKind::init_declarator_list;

  CppAstNode init_declarator;
  init_declarator.kind = CppAstKind::init_declarator;

  CppAstNode declarator;
  declarator.kind = CppAstKind::declarator;

  CppAstNode identifier;
  identifier.kind = CppAstKind::identifier;
  identifier.value = alias_name;

  declarator.children.push_back(identifier);
  init_declarator.children.push_back(declarator);
  out.children.push_back(init_declarator);
}

}  // namespace

void collect_namespace_alias_definition(SemanticContext & ctx,
                                        Scope & scope,
                                        const CppAstNode & node)
{
  const CppAstNode * target = find_child(node, CppAstKind::target);
  if(!target) {
    throw logic_error("namespace-alias-definition missing target");
  }

  const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
  if(!target_name) {
    throw logic_error("namespace-alias-definition target missing structured name");
  }
  Scope * target_namespace = semantic_lookup::lookup_namespace_name(scope, *target_name);
  if(!target_namespace) {
    throw logic_error("unknown namespace alias target");
  }

  semantic_scope_mutation::bind_namespace(scope, node.value, target_namespace);
}

void collect_using_directive(SemanticContext & ctx,
                             Scope & scope,
                             const CppAstNode & node)
{
  const CppAstNode * target = find_child(node, CppAstKind::target);
  if(!target) {
    throw logic_error("using-directive missing target");
  }

  const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
  if(!target_name) {
    throw logic_error("using-directive target missing structured name");
  }
  Scope * target_namespace = semantic_lookup::lookup_namespace_name(scope, *target_name);
  if(!target_namespace) {
    throw logic_error("unknown using-directive target");
  }

  semantic_scope_mutation::add_using_directive_if_needed(scope, *target_namespace);
}

void collect_using_declaration(SemanticContext & ctx,
                               Scope & scope,
                               const CppAstNode & node,
                               MemberAccess access)
{
  const CppAstNode * target = find_child(node, CppAstKind::target);
  if(!target) {
    throw logic_error("using-declaration missing target");
  }

  const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
  if(!target_name) {
    throw logic_error("using-declaration target missing qualified-name syntax");
  }
  const QualifiedName qualified = *target_name;
  const string target_text = qualified_name_text(qualified);

  TypePtr type = ctx.lookup_type(scope, target_text);
  if(!type) {
    if(ClassInfo * target_class = lookup_using_target_class(ctx, scope, qualified)) {
      const semantic_lookup::MemberTypeLookupResult member_type =
          semantic_lookup::lookup_member_type(ctx,
                                             *target_class,
                                             qualified.name,
                                             true,
                                             &scope);
      type = member_type.type;
    }
  }
  if(type) {
    semantic_scope_mutation::bind_named_type(scope, qualified.name, type);
    return;
  }
  if(bind_dependent_using_typename(ctx, scope, *target, qualified, target_text)) {
    return;
  }

  ClassTemplateDecl * class_template = semantic_lookup::lookup_class_template(ctx, scope, qualified);
  if(class_template) {
    semantic_scope_mutation::bind_class_template(scope, qualified.name, class_template);
    return;
  }

  AliasTemplateDecl * alias_template = semantic_lookup::lookup_alias_template(ctx, scope, qualified);
  if(alias_template) {
    semantic_scope_mutation::bind_alias_template(scope, qualified.name, alias_template);
    return;
  }

  VariableTemplateDecl * variable_template =
      semantic_lookup::lookup_variable_template(ctx, scope, qualified);
  if(variable_template) {
    semantic_scope_mutation::bind_variable_template(scope, qualified.name, variable_template);
    return;
  }

  Scope * target_namespace = semantic_lookup::lookup_namespace_name(scope, qualified);
  if(target_namespace) {
    semantic_scope_mutation::bind_namespace(scope, qualified.name, target_namespace);
    return;
  }

  const ValueBinding * value =
      semantic_lookup::lookup_qualified_value_binding(ctx, scope, qualified);
  if(value) {
    ValueBinding imported = *value;
    if(scope.class_info) {
      imported.access = access;
    }
    semantic_scope_mutation::bind_value(scope, qualified.name, imported);
    return;
  }

  vector<FunctionBinding *> functions = lookup_using_target_functions(ctx, scope, qualified);
  if(functions.empty()) {
    functions = ctx.lookup_qualified_functions(scope, qualified);
  }

  vector<FunctionTemplateDecl *> function_templates =
      lookup_using_target_function_templates(ctx, scope, qualified);
  if(function_templates.empty()) {
    function_templates = ctx.lookup_qualified_function_templates(scope, qualified);
  }

  if(!functions.empty()) {
    semantic_scope_mutation::append_function_bindings(scope, qualified.name, functions, access);
  }
  if(!function_templates.empty()) {
    semantic_scope_mutation::append_unique_function_templates(scope,
                                                              qualified.name,
                                                              function_templates);
  }
  if(!functions.empty() || !function_templates.empty()) {
    return;
  }

  if(node.has_using_if_exists) {
    return;
  }

  Scope * debug_namespace = nullptr;
  if(!qualified.qualifiers.empty()) {
    QualifiedName qualifier_name;
    qualifier_name.rooted = qualified.rooted;
    qualifier_name.qualifiers = qualified.qualifiers;
    qualifier_name.name = qualifier_name.qualifiers.back();
    qualifier_name.qualifiers.pop_back();
    debug_namespace = semantic_lookup::lookup_namespace_name(scope, qualifier_name);
  }

  ostringstream out;
  out << "unknown using-declaration target: " << target_text
      << " [functions " << functions.size()
      << "] [function templates " << function_templates.size()
      << "] [namespace " << (debug_namespace ? "yes" : "no") << "]";
  throw logic_error(out.str());
}

void analyze_static_assert_declaration(SemanticContext & ctx,
                                       Scope & scope,
                                       const CppAstNode & node)
{
  if(node.children.empty()) {
    throw logic_error("static_assert missing condition");
  }

  constant_eval::ConstexprValue value;
  bool evaluated = false;
  bool truthy = false;
  try {
    CppAstNode initializer;
    initializer.kind = CppAstKind::initializer;
    initializer.children.push_back(node.children[0]);
    evaluated =
                ctx.evaluate_initializer_constant_value(scope,
                                                       initializer,
                                                       make_fundamental(FT_BOOL),
                                                       value) &&
                constant_eval::constexpr_value_truthy(value, truthy);
    if(!evaluated) {
      constant_eval::ConstexprValue bool_value;
      if(ctx.evaluate_initializer_constant_value(scope,
                                                 node.children[0],
                                                 make_fundamental(FT_BOOL),
                                                 bool_value)) {
        evaluated = constant_eval::constexpr_value_truthy(bool_value, truthy);
      }
    }
    if(!evaluated) {
      constant_eval::ConstexprValue known_value;
      if(evaluate_known_static_member_constant(ctx, scope, node.children[0], known_value)) {
        evaluated = constant_eval::constexpr_value_truthy(known_value, truthy);
      }
    }
    if(!evaluated) {
      long long integral_value = 0;
      if(evaluate_static_assert_integral_fallback(ctx,
                                                  scope,
                                                  node.children[0],
                                                  integral_value)) {
        evaluated = true;
        truthy = integral_value != 0;
      }
    }
    if(!evaluated) {
      if(evaluate_static_assert_text_fallback(ctx, scope, node.children[0], truthy)) {
        evaluated = true;
      }
    }
  } catch(const logic_error &) {
    if(!ctx.scope_has_template_placeholders(scope)) {
      throw;
    }
  }
  if(!evaluated) {
    if(ctx.scope_has_template_placeholders(scope)) {
      return;
    }
    const string condition = ctx.describe_expression_for_diagnostic(node.children[0]);
    const string bindings = ctx.describe_scope_bindings_for_diagnostic(scope);
    const string lookup = ctx.describe_static_assert_lookup_for_diagnostic(scope, node.children[0]);
    throw logic_error(string("static_assert unevaluated: ") + condition +
                      (lookup.empty() ? string() : string(" [lookup ") + lookup + "]") +
                      (bindings.empty() ? string() : string(" [bindings ") + bindings + "]"));
  }
  if(!truthy) {
    if(ctx.scope_has_template_placeholders(scope) &&
       static_assert_condition_depends_on_template_parameter(ctx,
                                                            scope,
                                                            node.children[0])) {
      return;
    }
    const string condition = ctx.describe_expression_for_diagnostic(node.children[0]);
    const string bindings = ctx.describe_scope_bindings_for_diagnostic(scope);
    const string lookup = ctx.describe_static_assert_lookup_for_diagnostic(scope, node.children[0]);
    throw logic_error(string("static_assert false: ") + condition +
                      (lookup.empty() ? string() : string(" [lookup ") + lookup + "]") +
                      (bindings.empty() ? string() : string(" [bindings ") + bindings + "]"));
  }
}

void collect_namespace_definition(SemanticContext & ctx,
                                  Scope & scope,
                                  const CppAstNode & node)
{
  const std::string namespace_name = node.value.empty() ? std::string("<unnamed>") : node.value;
  DIAG_CONTEXT("semantic_declaration::collect_namespace_definition [" + namespace_name + "]" +
               ctx.source_location_for_node(node));
  Scope * target = nullptr;
  bool is_inline_namespace = false;
  if(node.value == "<unnamed>") {
    target = ctx.find_named_namespace_child(scope, node.value);
    if(!target) {
      target = &ctx.append_namespace_scope(scope, node.value);
    }
    semantic_scope_mutation::bind_namespace(scope, "_GLOBAL__N_1", target);
  } else {
    target = ctx.find_named_namespace_child(scope, node.value);
    if(!target) {
      target = &ctx.append_namespace_scope(scope, node.value);
    }
    semantic_scope_mutation::bind_namespace(scope, node.value, target);
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::inline_node) {
      is_inline_namespace = true;
      target->inline_namespace = true;
      template_api::record_template_witness_inline_namespace(node.value);
      continue;
    }
    try {
      collect_declaration(ctx, *target, node.children[i]);
    } catch(const logic_error & e) {
      ostringstream out;
      out << e.what() << " [namespace child " << i
          << " kind=" << cppast_kind_text(node.children[i].kind);
      if(!node.children[i].value.empty()) {
        out << " value=" << node.children[i].value;
      }
      const string child_text = node_text(node.children[i]);
      if(!child_text.empty()) {
        out << " text=" << child_text;
      } else {
        out << " ast={" << describe_cppast_translation_unit(node.children[i]) << "}";
      }
      out << "]";
      throw logic_error(out.str());
    }
  }

  if(node.value == "<unnamed>") {
    semantic_scope_mutation::add_using_directive_if_needed(scope, *target);
  }

  if(is_inline_namespace) {
    semantic_scope_mutation::import_inline_namespace_members(scope, *target);
  }
}

bool prepare_alias_type_id(SemanticContext & ctx,
                           Scope & scope,
                           const CppAstNode & node,
                           bool block_scope,
                           CppAstNode & out)
{
  const CppAstNode * type_id = find_child(node, CppAstKind::type_id);
  if(!type_id) {
    return false;
  }

  out = *type_id;
  if(out.kind != CppAstKind::type_id || out.children.empty()) {
    return false;
  }

  CppAstNode & specifiers = out.children[0];
  if(specifiers.kind != CppAstKind::decl_specifier_seq &&
     specifiers.kind != CppAstKind::type_specifier_seq) {
    return true;
  }

  CppAstNode synthetic_declarators;
  build_alias_name_declarators(node.value, synthetic_declarators);

  CppAstNode resolved_specifiers;
  const bool prepared =
      block_scope ?
          ctx.prepare_block_scope_specifiers(scope,
                                            specifiers,
                                            &synthetic_declarators,
                                            resolved_specifiers) :
          ctx.prepare_namespace_scope_specifiers(scope,
                                                specifiers,
                                                &synthetic_declarators,
                                                true,
                                                false,
                                                resolved_specifiers);
  if(!prepared) {
    return false;
  }

  specifiers = resolved_specifiers;
  return true;
}

void collect_declaration(SemanticContext & ctx,
                         Scope & scope,
                         const CppAstNode & node,
                         bool is_c_linkage,
                         bool linkage_has_braces)
{
  const CppAstNode & effective_child = node;
  CppAstNode synthetic_decl;
  std::string synthetic_type_name;
  std::string synthetic_storage_name;
  if(semantic_class_model::synthesize_anonymous_union_storage_declaration(effective_child,
                                                                          synthetic_decl,
                                                                          synthetic_type_name,
                                                                          synthetic_storage_name)) {
    const CppAstNode * owned_synthetic_decl = ctx.own_synthetic_ast(std::move(synthetic_decl));
    ctx.collect_simple_declaration(scope, *owned_synthetic_decl, is_c_linkage, linkage_has_braces);
    cpp_decl::TypePtr storage_type = ctx.lookup_type(scope, synthetic_type_name);
    ClassInfo * storage_info = storage_type ? ctx.class_info_for_type(storage_type) : nullptr;
    if(!storage_info) {
      throw logic_error("missing anonymous union storage class");
    }
    semantic_class_model::inject_anonymous_union_variable_bindings(scope,
                                                                   *storage_info,
                                                                   synthetic_storage_name);
    return;
  }

  if(effective_child.kind == CppAstKind::empty_declaration) {
    return;
  }
  if(effective_child.kind == CppAstKind::class_specifier ||
     effective_child.kind == CppAstKind::class_forward_declaration) {
    ctx.collect_class_declaration(scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::enum_specifier) {
    ctx.collect_enum_declaration(scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::explicit_instantiation_declaration ||
     effective_child.kind == CppAstKind::explicit_instantiation_definition) {
    ctx.collect_explicit_instantiation(scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::template_declaration) {
    if(is_c_linkage) {
      throw logic_error("extern \"C\" templates unsupported");
    }
    ctx.collect_template_declaration(scope, effective_child, MA_PUBLIC);
    return;
  }
  if(effective_child.kind == CppAstKind::deduction_guide_declaration) {
    ctx.collect_deduction_guide_declaration(scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::simple_declaration) {
    ctx.collect_simple_declaration(scope, effective_child, is_c_linkage, linkage_has_braces);
    return;
  }
  if(effective_child.kind == CppAstKind::static_assert_declaration) {
    analyze_static_assert_declaration(ctx, scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::alias_declaration) {
    const CppAstNode * type_id = find_child(effective_child, CppAstKind::type_id);
    if(!type_id) {
      throw logic_error("alias-declaration missing type-id");
    }
    CppAstNode prepared_type_id;
    if(!prepare_alias_type_id(ctx, scope, effective_child, false, prepared_type_id)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    TypePtr alias;
    if(!ctx.parse_type_id(scope, prepared_type_id, alias, true)) {
      throw logic_error(string("unsupported alias-declaration: ") + node_text(*type_id));
    }
    semantic_scope_mutation::bind_named_type(scope, effective_child.value, alias);
    return;
  }
  if(effective_child.kind == CppAstKind::function_definition) {
    ctx.collect_function_definition(scope, effective_child, is_c_linkage);
    return;
  }
  if(effective_child.kind == CppAstKind::special_member_definition) {
    ctx.collect_special_member_definition(scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::namespace_definition) {
    if(is_c_linkage) {
      throw logic_error("extern \"C\" namespaces unsupported");
    }
    collect_namespace_definition(ctx, scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::linkage_specification) {
    const bool child_c_linkage = effective_child.value == "C";
    for(size_t i = 0; i < effective_child.children.size(); ++i) {
      try {
        collect_declaration(ctx,
                            scope,
                            effective_child.children[i],
                            child_c_linkage,
                            effective_child.linkage_has_braces);
      } catch(const logic_error & e) {
        ostringstream out;
        out << e.what() << " [linkage child " << i
            << " kind=" << cppast_kind_text(effective_child.children[i].kind);
        if(!effective_child.children[i].value.empty()) {
          out << " value=" << effective_child.children[i].value;
        }
        out << "]";
        throw logic_error(out.str());
      }
    }
    return;
  }
  if(effective_child.kind == CppAstKind::namespace_alias_definition) {
    collect_namespace_alias_definition(ctx, scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::using_directive) {
    collect_using_directive(ctx, scope, effective_child);
    return;
  }
  if(effective_child.kind == CppAstKind::using_declaration) {
    collect_using_declaration(ctx, scope, effective_child);
    return;
  }

  throw logic_error("unsupported declaration in PA12 current slice");
}

void collect_top_level_declarations(SemanticContext & ctx,
                                    Scope & root,
                                    const CppAstNode & translation_unit)
{
  for(size_t i = 0; i < translation_unit.children.size(); ++i) {
    try {
      collect_declaration(ctx, root, translation_unit.children[i]);
    } catch(const logic_error & e) {
      ostringstream out;
      out << e.what() << " [top-level declaration " << i
          << " kind=" << cppast_kind_text(translation_unit.children[i].kind);
      if(!translation_unit.children[i].value.empty()) {
        out << " value=" << translation_unit.children[i].value;
      }
      const string child_text = node_text(translation_unit.children[i]);
      if(!child_text.empty()) {
        out << " text=" << child_text;
      } else {
        out << " ast={" << describe_cppast_translation_unit(translation_unit.children[i]) << "}";
      }
      out << "]";
      throw logic_error(out.str());
    }
  }
}

}  // namespace semantic_declaration
