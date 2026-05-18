#include "template_argument_semantics.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "callsemantic_internal.h"
#include "callsemantic/template_source_utils.h"
#include "class_template_mangle_info.h"
#include "constexpr_eval.h"
#include "cpp_decl_ast.h"
#include "parser_trace.h"
#include "semantic_builtins.h"
#include "semantic_conversion.h"
#include "semantic_context.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_consteval.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_angle_lookup.h"
#include "template_decl_ast.h"
#include "template_instantiation.h"
#include "template_metadata.h"
#include "template_resolution.h"
#include "template_scope.h"
#include "template_specialization.h"
#include "template_services.h"
#include "witness_api.h"

using namespace std;

namespace template_api {

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<cpp_decl::TemplateArgumentSyntax> * syntaxes,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope = TemplateEnvironmentHandle());

bool resolve_template_arguments(
    TemplateServices & services,
    TemplateEnvironmentHandle scope,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<template_model::TemplateArgument> & out,
    TemplateEnvironmentHandle default_argument_declaring_scope = TemplateEnvironmentHandle());

}  // namespace template_api

namespace template_argument_semantics {

using namespace cpp_decl;
using namespace semantic_model;
using namespace template_model;
using callsemantic_internal::is_identifier_text;
using callsemantic_internal::match_wrapped_type_text;
using callsemantic_internal::normalize_type_lookup_name;
using callsemantic_internal::reparseable_type_argument_text;

static bool expression_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    bool check_node_text);

NonTypeArgumentStatus evaluate_structured_bool_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    bool & out);

bool resolve_template_id_syntax_type(template_api::TemplateServices & services,
                                     Scope & scope,
                                     const TemplateIdSyntax & syntax,
                                     bool reference_class_templates_only,
                                     const string & source_location,
                                     TypePtr & out,
                                     template_api::TemplateEnvironmentHandle
                                         argument_scope,
                                     template_api::ClassTemplateSourceUseMode source_use_mode,
                                     bool allow_enclosing_current_specializations);

namespace {

const char kStructuredBoolResultMemberName[] = "value";
thread_local size_t g_default_template_argument_evaluation_depth = 0;
thread_local vector<string> g_base_specifier_type_lookup_keys;

string base_specifier_type_lookup_key(const string & text)
{
  return callsemantic_internal::remove_space_chars(
      normalize_type_lookup_name(semantic_utils::trim_space(text)));
}

bool base_specifier_type_lookup_allows_implicit_typename(
    const string & lookup_text)
{
  const string key = base_specifier_type_lookup_key(lookup_text);
  if(key.empty()) {
    return false;
  }
  for(vector<string>::const_reverse_iterator it =
          g_base_specifier_type_lookup_keys.rbegin();
      it != g_base_specifier_type_lookup_keys.rend();
      ++it) {
    if(*it == key) {
      return true;
    }
  }
  return false;
}

bool is_structured_bool_result_member_name(const string & name)
{
  return name == kStructuredBoolResultMemberName;
}

bool alias_source_text_has_suffix(const string & text, const string & suffix)
{
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool is_simple_alias_source_identifier_text(const string & text)
{
  if(text.empty()) {
    return false;
  }
  if(!(std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_')) {
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

bool named_type_has_function_local_marker(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }
  return base->named_key.find("__local_") != string::npos ||
         base->named_display.find("__local_") != string::npos;
}

}  // namespace

void canonicalize_alias_template_source_argument_texts(
    const vector<TemplateParameterInfo> & parameters,
    vector<string> & arg_texts)
{
  const size_t limit = std::min(parameters.size(), arg_texts.size());
  for(size_t i = 0; i < limit; ++i) {
    string text = semantic_utils::trim_space(arg_texts[i]);
    if(!parameters[i].parameter_pack &&
       alias_source_text_has_suffix(text, "...")) {
      text = semantic_utils::trim_space(text.substr(0, text.size() - 3));
    }
    if(parameters[i].kind == TemplateParameterInfo::TP_TYPE) {
      const string typename_prefix = "typename ";
      if(text.compare(0, typename_prefix.size(), typename_prefix) == 0) {
        const string rest =
            semantic_utils::trim_space(text.substr(typename_prefix.size()));
        if(is_simple_alias_source_identifier_text(rest)) {
          text = rest;
        }
      }
    }
    arg_texts[i] = text;
  }
}

bool template_parameter_is_function_pointer_value(
    const TemplateParameterInfo & parameter)
{
  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
     !parameter.value_type) {
    return false;
  }
  TypePtr base = strip_top_level_cv(parameter.value_type);
  if(!base || base->kind != Type::TK_POINTER || !base->inner) {
    return false;
  }
  TypePtr pointee = strip_top_level_cv(base->inner);
  return pointee && pointee->kind == Type::TK_FUNCTION;
}

AliasTemplateDecl * lookup_alias_template(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const string & name);

bool evaluate_builtin_type_trait(template_api::TemplateServices & services,
                                 Scope & scope,
                                 const string & name,
                                 const vector<TypePtr> & types,
                                 long long & out);
bool parse_type_argument_text(template_api::TemplateServices & services,
                              template_api::TemplateEnvironmentHandle scope,
                              const string & text,
                              TypePtr & out);
const vector<TypePtr> * lookup_type_pack(Scope & scope, const string & name);
TypePtr current_specialization_type_for_lookup_text(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & normalized_name);
bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const TypePtr & type);
bool resolve_instantiated_dependent_type(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const TypePtr & type,
                                         TypePtr & out);
string lookup_text_for_non_type_template_argument(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    long long value);
semantic_conversion::ExprInfo make_builtin_trait_expr_info(const TypePtr & source);

namespace {

bool is_simple_dependent_argument_text(const string & text);

template_api::TemplateElaboratedTypeKind class_key_elaborated_type_kind(
    const CppAstNode & class_key)
{
  if(!class_key.has_token || class_key.token_kind != RT_SIMPLE) {
    return template_api::TETK_NONE;
  }
  switch(class_key.simple_type) {
  case KW_CLASS:
    return template_api::TETK_CLASS;
  case KW_STRUCT:
    return template_api::TETK_STRUCT;
  case KW_UNION:
    return template_api::TETK_UNION;
  default:
    break;
  }
  return template_api::TETK_NONE;
}

bool lookup_leaf_member_expression_type_in_scope(
    template_api::TemplateTypeSystem & type_system,
    Scope & member_scope,
    const string & name,
    TypePtr & out);
bool lookup_leaf_member_expression_value_in_scope(
    template_api::TemplateServices & services,
    Scope & member_scope,
    const string & name,
    constant_eval::ConstexprValue & out,
    bool allow_structured_bool_shortcut = true);
bool evaluate_constant_expression_leaf_impl(template_api::TemplateServices & services,
                                            Scope & scope,
                                            const CppAstNode & node,
                                            constant_eval::ConstexprValue & out,
                                            const TypePtr & target_type);
bool materialize_leaf_member_constant_binding(
    template_api::TemplateServices & services,
    ValueBinding & binding,
    constant_eval::ConstexprValue & out);
void note_structured_bool_integral_constant_value_for_witness(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    bool value);
bool lookup_leaf_member_function_bindings(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & base_type,
    const string & name,
    vector<FunctionBinding *> & out);

// template-boundary-audit: begin semantic_service_access
template_api::TemplateTypeSystem & service_type_system(
    template_api::TemplateServices & services)
{
  return services.type_system;
}

bool service_resolve_direct_type_lookup(
    template_api::TemplateServices & services,
    const template_api::TemplateTypeLookupRequest & request,
    TypePtr & out)
{
  return service_type_system(services).resolve_direct_type_lookup(request, out);
}

bool resolve_direct_type_name_lookup(template_api::TemplateServices & services,
                                     Scope & scope,
                                     const QualifiedName & name,
                                     bool allow_class_templates,
                                     const string & source_location,
                                     TypePtr & out)
{
  out.reset();
  if(name.name.empty()) {
    return false;
  }
  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = allow_class_templates;
  request.name = name;
  request.source_location =
      template_api::normalize_template_witness_source_location(source_location);
  return service_resolve_direct_type_lookup(services, request, out) && out;
}

bool resolve_direct_type_name_lookup(template_api::TemplateServices & services,
                                     Scope & scope,
                                     const string & name,
                                     bool allow_class_templates,
                                     const string & source_location,
                                     TypePtr & out)
{
  QualifiedName qualified;
  qualified.name = semantic_utils::trim_space(name);
  return resolve_direct_type_name_lookup(services,
                                         scope,
                                         qualified,
                                         allow_class_templates,
                                         source_location,
                                         out);
}

bool service_resolve_selected_class_template_id(
    template_api::TemplateServices & services,
    const template_api::TemplateSelectedClassTemplateIdRequest & request,
    TypePtr & out)
{
  return service_type_system(services).resolve_selected_class_template_id(request, out);
}

bool service_prepare_named_type_member_scope(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    Scope *& out)
{
  return service_type_system(services).prepare_named_type_member_scope(scope, type, out);
}

bool service_lookup_leaf_member_expression_type_in_scope(
    template_api::TemplateServices & services,
    Scope & member_scope,
    const string & name,
    TypePtr & out)
{
  return lookup_leaf_member_expression_type_in_scope(
      service_type_system(services),
      member_scope,
      name,
      out);
}

bool service_lookup_leaf_member_expression_value_in_scope(
    template_api::TemplateServices & services,
    Scope & member_scope,
    const string & name,
    constant_eval::ConstexprValue & out)
{
  const bool found = lookup_leaf_member_expression_value_in_scope(
      services,
      member_scope,
      name,
      out,
      !witness::source_capture_enabled(services.witness_context));
  if(found &&
     is_structured_bool_result_member_name(name) &&
     member_scope.class_info &&
     services.witness_context.session != nullptr) {
    bool truthy = false;
    if(constant_eval::constexpr_value_truthy(out, truthy)) {
      note_structured_bool_integral_constant_value_for_witness(
          services,
          *member_scope.class_info,
          truthy);
    }
  }
  return found;
}

bool service_lookup_leaf_member_function_bindings(
    template_api::TemplateServices & services,
    const TypePtr & base_type,
    const string & name,
    vector<FunctionBinding *> & out)
{
  return lookup_leaf_member_function_bindings(
      service_type_system(services),
      base_type,
      name,
      out);
}

const std::map<std::string, ClassInfo *> * service_classes_by_key(
    template_api::TemplateServices & services)
{
  return service_type_system(services).model.classes_by_key;
}

bool service_type_depends_on_template_parameter(
    template_api::TemplateServices & services,
    const TypePtr & type)
{
  return template_argument_semantics::type_depends_on_template_parameter(
      service_type_system(services),
      type);
}

string service_lookup_text_for_type_argument(template_api::TemplateServices & services,
                                             const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(
      service_type_system(services),
      type);
}

string service_lookup_text_for_non_type_template_argument(
    template_api::TemplateServices & services,
    const TypePtr & type,
    long long value)
{
  return lookup_text_for_non_type_template_argument(
      service_type_system(services),
      type,
      value);
}

bool service_describe_named_type_metadata(
    template_api::TemplateServices & services,
    const TypePtr & type,
    template_api::TemplateNamedTypeMetadata & out)
{
  return template_api::describe_named_type_metadata(
      service_type_system(services).model,
      type,
      out);
}

bool service_evaluate_semantic_builtin_type_trait(
    template_api::TemplateServices & services,
    const template_api::TemplateSemanticBuiltinTraitRequest & request,
    long long & out)
{
  return services.recursive_semantic.evaluate_semantic_builtin_type_trait(request, out);
}

bool service_evaluate_initializer_constant_value(
    template_api::TemplateServices & services,
    const template_api::TemplateConstantEvaluationRequest & request,
    constant_eval::ConstexprValue & out)
{
  return services.recursive_semantic.evaluate_initializer_constant_value(request, out);
}

bool service_evaluate_dependent_type_expression(
    template_api::TemplateServices & services,
    const template_api::TemplateDependentTypeExprRequest & request,
    TypePtr & out)
{
  return services.recursive_semantic.evaluate_dependent_type_expression(request, out);
}
// template-boundary-audit: end semantic_service_access

bool resolve_elaborated_type_lookup_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out)
{
  out.reset();
  if(node.kind != CppAstKind::class_forward_declaration || node.value.empty()) {
    return false;
  }
  const CppAstNode * class_key =
      cppast_find_child_kind(node, CppAstKind::class_key);
  if(!class_key) {
    return false;
  }
  const template_api::TemplateElaboratedTypeKind elaborated_kind =
      class_key_elaborated_type_kind(*class_key);
  if(elaborated_kind == template_api::TETK_NONE) {
    return false;
  }

  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = reference_class_templates_only;
  request.elaborated_kind = elaborated_kind;
  request.source_location =
      template_api::normalize_template_witness_source_location(source_location);
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    request.name = *qualified;
  } else if(!semantic_utils::split_qualified_name_text(node.value, request.name)) {
    request.name = QualifiedName();
    request.name.name = node.value;
  }
  if(request.name.name.empty()) {
    request.name.name = node.value;
  }
  return service_resolve_direct_type_lookup(services, request, out) && out;
}

struct ScopedTemplateUseLocation
{
  explicit ScopedTemplateUseLocation(const string & location)
  {
    parser_trace::push_use_location(location);
  }

  ~ScopedTemplateUseLocation()
  {
    parser_trace::pop_use_location();
  }

  ScopedTemplateUseLocation(const ScopedTemplateUseLocation &) = delete;
  ScopedTemplateUseLocation & operator=(const ScopedTemplateUseLocation &) = delete;
};

string template_public_use_location_or(
    const template_api::TemplateWitnessContext & witness_context,
    const string & fallback)
{
  if(parser_trace::use_location_suppressed()) {
    return string();
  }
  if(!witness_context.public_use_location.empty()) {
    return witness_context.public_use_location;
  }
  const string current = parser_trace::current_use_location();
  return !current.empty() ? current : fallback;
}

string repair_compacted_template_argument_expression_spacing(const string & text)
{
  string out = text;
  size_t pos = 0;
  while((pos = out.find(")>>", pos)) != string::npos) {
    out.replace(pos, 3, ") >> ");
    pos += 5;
  }
  return out;
}

bool identifier_token_match_at(const string & text,
                               size_t pos,
                               const string & identifier)
{
  if(identifier.empty() || pos + identifier.size() > text.size()) {
    return false;
  }
  if(text.compare(pos, identifier.size(), identifier) != 0) {
    return false;
  }
  const bool left_boundary =
      pos == 0 ||
      (!std::isalnum(static_cast<unsigned char>(text[pos - 1])) &&
       text[pos - 1] != '_');
  const size_t end = pos + identifier.size();
  const bool right_boundary =
      end >= text.size() ||
      (!std::isalnum(static_cast<unsigned char>(text[end])) &&
       text[end] != '_');
  return left_boundary && right_boundary;
}

size_t find_identifier_occurrence(const string & text,
                                  const string & identifier,
                                  size_t begin)
{
  size_t pos = begin;
  while(pos < text.size()) {
    pos = text.find(identifier, pos);
    if(pos == string::npos) {
      return string::npos;
    }
    if(identifier_token_match_at(text, pos, identifier)) {
      return pos;
    }
    ++pos;
  }
  return string::npos;
}

string format_source_location(const string & file, int line, int column)
{
  if(file.empty() || line <= 0 || column <= 0) {
    return string();
  }
  ostringstream out;
  out << file << ':' << line << ':' << column;
  return out.str();
}

string source_location_with_text_offset(const string & base_location,
                                        const string & text,
                                        size_t offset)
{
  const template_api::template_witness_detail::ParsedSourceLocation parsed =
      template_api::template_witness_detail::parse_source_location(
          template_api::normalize_template_witness_source_location(base_location));
  if(!parsed.valid || parsed.file.empty() || parsed.line <= 0 ||
     parsed.column <= 0) {
    return string();
  }
  int line = parsed.line;
  int column = parsed.column;
  const size_t limit = std::min(offset, text.size());
  for(size_t i = 0; i < limit; ++i) {
    if(text[i] == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
  }
  return format_source_location(parsed.file, line, column);
}

string first_call_callee_name(const CppAstNode & node)
{
  if(node.kind == CppAstKind::call_expression && !node.children.empty()) {
    const CppAstNode & callee = node.children[0];
    if(callee.kind == CppAstKind::id_expression) {
      if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(callee)) {
        if(!template_id->name.name.empty()) {
          return template_id->name.name;
        }
      }
      if(!callee.value.empty()) {
        return callee.value;
      }
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    const string child = first_call_callee_name(node.children[i]);
    if(!child.empty()) {
      return child;
    }
  }
  return string();
}

ClassTemplateDecl * lookup_class_template_impl(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const string & name);
AliasTemplateDecl * lookup_alias_template_impl(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const string & name);

bool is_decltype_or_typeof_text(const string & text)
{
  const string trimmed = semantic_utils::trim_space(text);
  return (trimmed.size() >= 10 && trimmed.compare(0, 9, "decltype(") == 0) ||
         (trimmed.size() >= 12 && trimmed.compare(0, 11, "__decltype(") == 0) ||
         (trimmed.size() >= 14 && trimmed.compare(0, 13, "__decltype__(") == 0) ||
         (trimmed.size() >= 10 && trimmed.compare(0, 9, "__typeof(") == 0) ||
         (trimmed.size() >= 12 && trimmed.compare(0, 11, "__typeof__(") == 0);
}

}  // namespace

namespace {

enum class StructuredTypeLookupResult
{
  NotApplicable,
  Resolved,
  NoMatch
};

vector<string> remaining_dependent_qualified_member_path(
    const QualifiedName & qualified,
    size_t dependent_qualifier_index)
{
  vector<string> path;
  for(size_t i = dependent_qualifier_index + 1; i < qualified.qualifiers.size(); ++i) {
    path.push_back(qualified.qualifiers[i]);
  }
  path.push_back(qualified.name);
  return path;
}

vector<TemplateIdSyntax> remaining_dependent_qualified_member_template_ids(
    const CppAstNode & node,
    const QualifiedName & qualified,
    size_t dependent_qualifier_index)
{
  vector<TemplateIdSyntax> syntax;
  for(size_t i = dependent_qualifier_index + 1; i < qualified.qualifiers.size(); ++i) {
    if(const TemplateIdSyntax * qualifier_template_id =
           cppast_qualifier_template_id_syntax(node, i)) {
      syntax.push_back(*qualifier_template_id);
    } else {
      syntax.push_back(TemplateIdSyntax());
    }
  }
  if(const TemplateIdSyntax * leaf_template_id = cppast_template_id_syntax(node)) {
    syntax.push_back(*leaf_template_id);
  } else {
    syntax.push_back(TemplateIdSyntax());
  }
  return syntax;
}

TypePtr make_structured_dependent_qualified_member_type(
    const string & lookup_text,
    const TypePtr & owner_type,
    const vector<string> & member_path,
    bool leading_typename,
    const vector<TemplateIdSyntax> & member_template_ids =
        vector<TemplateIdSyntax>())
{
  return make_dependent_qualified_member_type(semantic_utils::trim_space(lookup_text),
                                              owner_type,
                                              member_path,
                                              leading_typename,
                                              member_template_ids);
}

StructuredTypeLookupResult resolve_qualified_template_type_lookup_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & lookup_name,
    const CppAstNode & node,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out);
StructuredTypeLookupResult resolve_structured_type_lookup_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out);

}  // namespace

ScopedDefaultTemplateArgumentEvaluation::ScopedDefaultTemplateArgumentEvaluation()
{
  ++g_default_template_argument_evaluation_depth;
}

ScopedDefaultTemplateArgumentEvaluation::~ScopedDefaultTemplateArgumentEvaluation()
{
  if(g_default_template_argument_evaluation_depth != 0) {
    --g_default_template_argument_evaluation_depth;
  }
}

bool default_template_argument_evaluation_active()
{
  return g_default_template_argument_evaluation_depth != 0;
}

TypePtr lookup_structured_type_node(template_api::TemplateServices & services,
                                    Scope & scope,
                                    const CppAstNode & node,
                                    const string & lookup_name,
                                    bool reference_class_templates_only,
                                    const string & source_location)
{
  if(node.semantic_type) {
    TypePtr resolved;
    if(resolve_instantiated_dependent_type(
           services,
           template_api::make_template_environment(scope),
           node.semantic_type,
           resolved) &&
       resolved) {
      return resolved;
    }
    return node.semantic_type;
  }

  TypePtr type;
  if(resolve_elaborated_type_lookup_node(services,
                                         scope,
                                         node,
                                         reference_class_templates_only,
                                         source_location,
                                         type) &&
     type) {
    return type;
  }

  const string repaired_lookup_name =
      repair_compacted_template_argument_expression_spacing(
          lookup_name.empty() ?
              (node.kind == CppAstKind::type_name && !node.value.empty() ?
                   node.value :
                   node_text(node)) :
              lookup_name);
  const StructuredTypeLookupResult qualified_template_result =
      resolve_qualified_template_type_lookup_node(services,
                                                  scope,
                                                  repaired_lookup_name,
                                                  node,
                                                  reference_class_templates_only,
                                                  source_location,
                                                  type);
  if(qualified_template_result == StructuredTypeLookupResult::Resolved && type) {
    return type;
  }
  if(qualified_template_result == StructuredTypeLookupResult::NoMatch) {
    return TypePtr();
  }

  const StructuredTypeLookupResult structured_result =
      resolve_structured_type_lookup_node(services,
                                          scope,
                                          node,
                                          reference_class_templates_only,
                                          source_location,
                                          type);
  if(structured_result == StructuredTypeLookupResult::Resolved && type) {
    return type;
  }
  return TypePtr();
}

ScopedBaseSpecifierTypeLookup::ScopedBaseSpecifierTypeLookup(
    const string & lookup_text)
  : active_(false)
{
  const string key = base_specifier_type_lookup_key(lookup_text);
  if(!key.empty()) {
    g_base_specifier_type_lookup_keys.push_back(key);
    active_ = true;
  }
}

ScopedBaseSpecifierTypeLookup::~ScopedBaseSpecifierTypeLookup()
{
  if(active_ && !g_base_specifier_type_lookup_keys.empty()) {
    g_base_specifier_type_lookup_keys.pop_back();
  }
}

bool parse_type_id_node_for_templates(template_api::TemplateServices & services,
                                      Scope & scope,
                                      const CppAstNode & type_id,
                                      TypePtr & out,
                                      bool reference_class_templates_only)
{
  out.reset();
  if(type_id.kind != CppAstKind::type_id) {
    return false;
  }

  const bool capture_source_locations =
      witness::source_capture_enabled(services.witness_context);
  const string inherited_use_location =
      capture_source_locations ?
          template_api::normalize_template_witness_source_location(
              template_public_use_location_or(services.witness_context, string())) :
          string();
  const string fragment_use_location =
      capture_source_locations ?
          template_api::normalize_template_witness_source_location(
              template_api::preferred_fragment_use_location(services.witness_context, type_id)) :
          string();
  const ScopedTemplateUseLocation use_location_guard(
      !fragment_use_location.empty() ? fragment_use_location :
                                       inherited_use_location);

  const auto lookup_local_template_bound_type =
      [&](const string & name) -> TypePtr
      {
        const string normalized = semantic_utils::trim_space(
            semantic_utils::strip_elaborated_type_prefix(name));
        if(normalized.empty() ||
           normalized.find("::") != string::npos ||
           normalized.find_first_of("<>()[],*&") != string::npos) {
          return TypePtr();
        }
        for(Scope * current = &scope; current; current = current->parent) {
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
          map<string, TypePtr>::const_iterator found =
              current->named_types.find(normalized);
          if(found != current->named_types.end()) {
            if(current->template_bound_type_names.count(normalized) != 0 &&
               found->second) {
              return found->second;
            }
            break;
          }
        }
        return TypePtr();
      };

  cpp_decl::AstDeclHooks hooks;
  hooks.lookup_type_node =
      [&](const CppAstNode & node) -> TypePtr
      {
        if(node.semantic_type) {
          TypePtr resolved;
          if(resolve_instantiated_dependent_type(
                 services,
                 template_api::make_template_environment(scope),
                 node.semantic_type,
                 resolved) &&
             resolved) {
            return resolved;
          }
          return node.semantic_type;
        }
        const string lookup_name =
            node.kind == CppAstKind::type_name && !node.value.empty() ?
                node.value :
                node_text(node);
        const string repaired_lookup_name =
            repair_compacted_template_argument_expression_spacing(lookup_name);
        const TemplateIdSyntax * template_id_syntax =
            cppast_template_id_syntax(node);
        const QualifiedName * qualified_name_syntax =
            cppast_qualified_name_syntax(node);
        const bool structured_type_lookup_expected =
            (template_id_syntax && !template_id_syntax->arguments.empty()) ||
            (qualified_name_syntax &&
             (qualified_name_syntax->rooted ||
              !qualified_name_syntax->qualifiers.empty()));
        const string template_id_name_location =
            capture_source_locations && template_id_syntax ?
                template_api::normalize_template_witness_source_location(
                    template_api::template_witness_detail::source_location_for_location_id(
                        services.witness_context,
                        template_id_syntax->source_location_id)) :
                string();
        const string node_start_location =
            capture_source_locations ?
                template_api::normalize_template_witness_source_location(
                    template_api::template_witness_detail::source_location_for_ast_node_start(
                        services.witness_context,
                        node)) :
                string();
        const string node_preferred_location =
            capture_source_locations ?
                template_api::normalize_template_witness_source_location(
                    template_api::preferred_fragment_use_location(
                        services.witness_context,
                        node)) :
                string();
        const string node_use_location =
            !template_id_name_location.empty() ? template_id_name_location :
            (template_id_syntax && !node_start_location.empty() ?
                 node_start_location :
                 node_preferred_location);
        const ScopedTemplateUseLocation node_use_location_guard(
            !node_use_location.empty() ? node_use_location :
                                         inherited_use_location);
        TypePtr type;
        if(resolve_elaborated_type_lookup_node(
               services,
               scope,
               node,
               reference_class_templates_only,
               node_use_location,
               type) &&
           type) {
          return type;
        }
        const StructuredTypeLookupResult qualified_template_result =
            resolve_qualified_template_type_lookup_node(
                services,
                scope,
                repaired_lookup_name,
                node,
                reference_class_templates_only,
                node_use_location,
                type);
        if(qualified_template_result == StructuredTypeLookupResult::Resolved &&
           type) {
          return type;
        }
        if(qualified_template_result == StructuredTypeLookupResult::NoMatch) {
          return TypePtr();
        }
        const StructuredTypeLookupResult structured_result =
            resolve_structured_type_lookup_node(services,
                                                scope,
                                                node,
                                                reference_class_templates_only,
                                                node_use_location,
                                                type);
        if(structured_result == StructuredTypeLookupResult::Resolved && type) {
          return type;
        }
        if(structured_result == StructuredTypeLookupResult::NoMatch) {
          return TypePtr();
        }
        if(structured_type_lookup_expected) {
          semantic_fallback_audit::hard_fail(
              "template-type-resolution-fallback",
              node_use_location,
              "type-id node lookup fell back from structured syntax"
              " [name " + repaired_lookup_name + "]");
        }
        if(structured_type_lookup_expected) {
          semantic_fallback_audit::hard_fail(
              "template-type-resolution-fallback",
              node_use_location,
              "type-id node lookup fell back from structured syntax"
              " to direct name lookup [name " + repaired_lookup_name + "]");
        }
        if(TypePtr bound_type = lookup_local_template_bound_type(repaired_lookup_name)) {
          return bound_type;
        }
        if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
          if(resolve_direct_type_name_lookup(services,
                                             scope,
                                             *qualified,
                                             reference_class_templates_only,
                                             node_use_location,
                                             type) &&
             type) {
            return type;
          }
        } else if(resolve_direct_type_name_lookup(services,
                                                  scope,
                                                  repaired_lookup_name,
                                                  reference_class_templates_only,
                                                  node_use_location,
                                                  type) &&
                  type) {
          return type;
        }
        return TypePtr();
      };
  hooks.lookup_type =
      [&](const string & name) -> TypePtr
      {
        TypePtr type;
        const string repaired_name =
            repair_compacted_template_argument_expression_spacing(name);
        if(TypePtr bound_type = lookup_local_template_bound_type(repaired_name)) {
          return bound_type;
        }
        if(resolve_direct_type_name_lookup(services,
                                           scope,
                                           repaired_name,
                                           reference_class_templates_only,
                                           std::string(),
                                           type) &&
           type) {
          return type;
        }
        return TypePtr();
      };
  hooks.parse_decltype_specifier =
      [&](const CppAstNode & decltype_node, TypePtr & type) -> bool
      {
        return template_argument_semantics::parse_decltype_or_typeof_node(
            services, scope, decltype_node, type);
      };
  hooks.evaluate_constant_expression =
      [&](const CppAstNode & node, long long & value) -> bool
      {
        constant_eval::ConstexprValue evaluated;
        return evaluate_constant_expression_leaf(services, scope, node, evaluated) &&
               constant_eval::constexpr_value_to_integral(evaluated, value);
      };
  hooks.expand_parameter_clause_packs =
      [&](const CppAstNode & node,
          CppAstNode & expanded_clause,
          vector<const CppAstNode *> * default_args_out) -> bool
      {
        return template_decl_ast::expand_parameter_clause_pack_patterns(
            services,
            scope,
            node,
            expanded_clause,
            default_args_out);
      };
  hooks.type_name_is_parameter_pack =
      [&scope](const string & name)
      {
        return template_scope::scope_has_type_parameter_pack_name(scope, name);
      };
  hooks.normalize_function_parameters = true;
  const bool parsed = cpp_decl::parse_type_id_ast(type_id, hooks, out) && out != nullptr;
  if(parsed && out) {
    TypePtr resolved;
    if(resolve_instantiated_dependent_type(
           services,
           template_api::make_template_environment(scope),
           out,
           resolved) &&
       resolved) {
      out = resolved;
    }
  }
  return parsed && out != nullptr;
}

bool resolve_type_argument_expression_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & expr,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out)
{
  out.reset();
  if(expr.kind != CppAstKind::id_expression &&
     expr.kind != CppAstKind::type_name) {
    return false;
  }

  const string lookup_name =
      expr.kind == CppAstKind::type_name && !expr.value.empty() ?
          expr.value :
          node_text(expr);
  const string repaired_lookup_name =
      repair_compacted_template_argument_expression_spacing(lookup_name);

  const StructuredTypeLookupResult qualified_template_result =
      resolve_qualified_template_type_lookup_node(services,
                                                  scope,
                                                  repaired_lookup_name,
                                                  expr,
                                                  reference_class_templates_only,
                                                  source_location,
                                                  out);
  if(qualified_template_result == StructuredTypeLookupResult::Resolved && out) {
    return true;
  }
  if(qualified_template_result == StructuredTypeLookupResult::NoMatch) {
    out.reset();
    return false;
  }

  const StructuredTypeLookupResult structured_result =
      resolve_structured_type_lookup_node(services,
                                          scope,
                                          expr,
                                          reference_class_templates_only,
                                          source_location,
                                          out);
  if(structured_result == StructuredTypeLookupResult::Resolved && out) {
    return true;
  }
  out.reset();
  return false;
}

using callsemantic_internal::remove_space_chars;
using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::has_top_level_comma;
using semantic_utils::is_wrapped_in_balanced_parens;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;

struct ExpandedTypeArgumentInput
{
  string text;
  TypePtr type;
};

vector<ExpandedTypeArgumentInput> expand_bound_type_pack_arguments(
    template_api::TemplateServices & services,
    Scope & scope,
    const vector<string> & texts);

vector<string> expanded_type_argument_input_texts(
    const vector<ExpandedTypeArgumentInput> & inputs);

bool normalized_type_lookup_text_matches(const string & lhs,
                                         const string & rhs)
{
  return remove_space_chars(normalize_type_lookup_name(lhs)) ==
         remove_space_chars(normalize_type_lookup_name(rhs));
}

bool scope_has_template_placeholders(template_api::TemplateServices & services,
                                     template_api::TemplateEnvironmentHandle scope);

namespace {

bool try_resolve_concrete_unary_type_transform_template_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    TypePtr & out);
bool should_defer_unresolved_type_lookup(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const string & text);
bool prepare_concrete_type_member_scope(template_api::TemplateServices & services,
                                        template_api::TemplateEnvironmentHandle scope,
                                        const TypePtr & type,
                                        Scope *& out);
TypePtr lookup_concrete_type_in_resolved_scope(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle lexical_scope,
    Scope & resolved_scope,
    const string & name);
TypePtr lookup_exact_bound_type_name(Scope & scope, const string & name);
TypePtr lookup_local_dependent_type_placeholder(Scope & scope, const string & text);
bool try_resolve_alias_template_id_locally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_api::TemplateTypeLookupRequest & request,
    const QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    TypePtr & out);
bool try_resolve_class_template_id_locally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_api::TemplateTypeLookupRequest & request,
    const string & original_text,
    ClassTemplateDecl * class_template,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    TypePtr & out);

TemplateArgumentSyntax clone_argument_syntax_for_template_substitution(
    const TemplateArgumentSyntax & source);

bool template_argument_syntax_mentions_template_bound_type_name(
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax)
{
  if(!scope.valid()) {
    return false;
  }
  std::string text = !syntax.source_text.empty() ? syntax.source_text : syntax.text;
  if(text.empty() && syntax.type_id) {
    text = node_text(*syntax.type_id);
  }
  if(text.empty()) {
    return false;
  }
  for(Scope * current = &scope.require(); current; current = current->parent) {
    for(std::set<std::string>::const_iterator it =
             current->template_bound_type_names.begin();
         it != current->template_bound_type_names.end();
         ++it) {
      if(callsemantic_internal::contains_identifier_token(text, *it)) {
        return true;
      }
    }
    for(std::set<std::string>::const_iterator it =
             current->template_bound_type_pack_names.begin();
         it != current->template_bound_type_pack_names.end();
         ++it) {
      if(callsemantic_internal::contains_identifier_token(text, *it)) {
        return true;
      }
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool local_type_qualifier_name_in_scope(Scope & scope, const string & raw_name)
{
  string name = semantic_utils::trim_space(raw_name);
  name = semantic_utils::strip_trailing_top_level_template_arguments(name);
  const string unqualified = semantic_utils::unqualified_member_name(name);
  if(!unqualified.empty()) {
    name = unqualified;
  }
  if(!is_identifier_text(name)) {
    return false;
  }

  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->named_types.find(name) != current->named_types.end() ||
       current->template_bound_type_names.count(name) != 0 ||
       current->template_bound_type_pack_names.count(name) != 0 ||
       current->template_bound_value_names.count(name) != 0 ||
       current->template_bound_template_names.count(name) != 0) {
      return true;
    }
  }
  return false;
}

bool qualified_name_has_local_type_qualifier(Scope & scope,
                                             const QualifiedName & name)
{
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(local_type_qualifier_name_in_scope(scope, name.qualifiers[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_has_local_leading_typename_qualifier(
    Scope & scope,
    const TemplateArgumentSyntax & syntax);

bool template_id_syntax_has_local_leading_typename_qualifier(
    Scope & scope,
    const TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_has_local_leading_typename_qualifier(
           scope, syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool type_name_node_has_local_leading_typename_qualifier(
    Scope & scope,
    const CppAstNode & node)
{
  if(node.has_leading_typename) {
    if(node.qualified_name_syntax &&
       qualified_name_has_local_type_qualifier(
           scope, *node.qualified_name_syntax)) {
      return true;
    }
    if(node.template_id_syntax &&
       qualified_name_has_local_type_qualifier(
           scope, node.template_id_syntax->name)) {
      return true;
    }
    for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
      if(qualified_name_has_local_type_qualifier(
             scope, node.qualifier_template_id_syntaxes[i].name)) {
        return true;
      }
    }
  }
  if(node.template_id_syntax &&
     template_id_syntax_has_local_leading_typename_qualifier(
         scope, *node.template_id_syntax)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_has_local_leading_typename_qualifier(
           scope, node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(type_name_node_has_local_leading_typename_qualifier(
           scope, node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

const CppAstNode * single_type_name_child(const CppAstNode & type_id)
{
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.empty()) {
    return nullptr;
  }
  const CppAstNode & specifiers = type_id.children[0];
  if(specifiers.kind != CppAstKind::type_specifier_seq ||
     specifiers.children.size() != 1 ||
     specifiers.children[0].kind != CppAstKind::type_name) {
    return nullptr;
  }
  return &specifiers.children[0];
}

bool template_argument_syntax_has_local_leading_typename_qualifier(
    Scope & scope,
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.type_id) {
    const CppAstNode * type_name = single_type_name_child(*syntax.type_id);
    if(type_name &&
       type_name_node_has_local_leading_typename_qualifier(scope, *type_name)) {
      return true;
    }
  }
  return syntax.template_id &&
         template_id_syntax_has_local_leading_typename_qualifier(
             scope, *syntax.template_id);
}

bool template_argument_syntax_has_local_leading_typename_qualifier(
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax)
{
  return scope.valid() &&
         template_argument_syntax_has_local_leading_typename_qualifier(
             scope.require(), syntax);
}

CppAstNode * direct_qualified_type_name_argument_syntax(
    TemplateArgumentSyntax & syntax)
{
  if(!syntax.type_id ||
     syntax.type_id->kind != CppAstKind::type_id ||
     syntax.type_id->children.empty()) {
    return nullptr;
  }

  CppAstNode & specifiers = syntax.type_id->children[0];
  if(specifiers.kind != CppAstKind::type_specifier_seq ||
     specifiers.children.size() != 1 ||
     specifiers.children[0].kind != CppAstKind::type_name) {
    return nullptr;
  }

  const QualifiedName * source_qualified =
      cppast_qualified_name_syntax(specifiers.children[0]);
  return source_qualified &&
         (source_qualified->rooted || !source_qualified->qualifiers.empty()) ?
             &specifiers.children[0] :
             nullptr;
}

bool qualified_name_for_resolved_type_annotation(const TypePtr & type,
                                                 QualifiedName & out)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base ||
     base->kind != Type::TK_NAMED ||
     named_type_has_function_local_marker(base)) {
    return false;
  }
  std::string text =
      !base->named_display.empty() ? base->named_display : base->named_key;
  text = strip_elaborated_type_prefix(trim_space(text));
  return !text.empty() &&
         semantic_utils::split_qualified_name_text(text, out) &&
         !out.qualifiers.empty();
}

void compress_qualified_type_owner_for_mangling(QualifiedName & qualified)
{
  if(qualified.qualifiers.size() <= 1) {
    return;
  }
  std::string owner = qualified.rooted ? std::string("::") : std::string();
  for(std::size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(i != 0) {
      owner += "::";
    }
    owner += qualified.qualifiers[i];
  }
  qualified.rooted = false;
  qualified.qualifiers.clear();
  qualified.qualifiers.push_back(owner);
}

void preserve_resolved_qualified_type_in_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & argument,
    TemplateArgumentSyntax & syntax)
{
  CppAstNode * type_name = direct_qualified_type_name_argument_syntax(syntax);
  if(!type_name ||
     argument.kind != TemplateArgument::TA_TYPE ||
     !argument.type ||
     argument.dependent ||
     service_type_depends_on_template_parameter(services, argument.type) ||
     template_argument_syntax_mentions_template_bound_type_name(scope, syntax) ||
     template_argument_syntax_has_local_leading_typename_qualifier(
         scope, syntax)) {
    return;
  }

  QualifiedName resolved;
  if(!qualified_name_for_resolved_type_annotation(argument.type, resolved)) {
    return;
  }
  compress_qualified_type_owner_for_mangling(resolved);
  type_name->qualified_name_syntax.reset(new QualifiedName(resolved));
  type_name->semantic_type = argument.type;
}

string qualified_name_text_for_structured_lookup(const QualifiedName & qualified)
{
  string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out += qualified.qualifiers[i];
    out += "::";
  }
  out += qualified.name;
  return out;
}

vector<string> template_id_syntax_argument_texts(const TemplateIdSyntax & syntax)
{
  vector<string> texts;
  if(!syntax.argument_syntaxes.empty()) {
    texts.reserve(syntax.argument_syntaxes.size());
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      string text = semantic_utils::trim_space(syntax.argument_syntaxes[i].text);
      if(syntax.argument_syntaxes[i].pack_expansion &&
         (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
        text += "...";
      }
      texts.push_back(text);
    }
    return texts;
  }

  texts.reserve(syntax.arguments.size());
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    texts.push_back(semantic_utils::trim_space(syntax.arguments[i]));
  }
  return texts;
}

string compact_source_argument_key(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(!std::isspace(static_cast<unsigned char>(text[i]))) {
      out.push_back(text[i]);
    }
  }
  return out;
}

std::string join_template_source_arguments(const std::vector<std::string> & args)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < args.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << args[i];
  }
  return out.str();
}

std::vector<std::string> current_specialization_parameter_texts(
    const ClassTemplateDecl & source_template)
{
  std::vector<std::string> texts;
  texts.reserve(source_template.parameters.size());
  for(std::size_t i = 0; i < source_template.parameters.size(); ++i) {
    std::string text = source_template.parameters[i].name.empty() ?
        std::string("$") + std::to_string(i + 1) :
        source_template.parameters[i].name;
    if(source_template.parameters[i].parameter_pack &&
       (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
      text += "...";
    }
    texts.push_back(text);
  }
  return texts;
}

std::vector<std::string> current_specialization_argument_texts(
    const ClassInfo & info)
{
  if(info.template_output_node &&
     info.source_template &&
     info.source_template->class_node &&
     info.template_output_node != info.source_template->class_node &&
     !info.instantiation_arg_texts.empty()) {
    return info.instantiation_arg_texts;
  }
  if(info.source_template) {
    return current_specialization_parameter_texts(*info.source_template);
  }
  QualifiedName qualified;
  std::vector<std::string> arg_texts;
  if(semantic_utils::split_top_level_template_id_text(info.name,
                                                      qualified,
                                                      arg_texts) &&
     !arg_texts.empty()) {
    return arg_texts;
  }
  return std::vector<std::string>();
}

std::string source_binding_current_specialization_text_from_scope(
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & argument,
    const std::string & source_text)
{
  if(argument.kind != TemplateArgument::TA_TYPE || !argument.type ||
     !scope.valid()) {
    return std::string();
  }

  const std::string type_name =
      trim_space(strip_elaborated_type_prefix(source_text));
  const std::string unqualified_type_name =
      unqualified_member_name(type_name).empty() ?
          type_name :
          unqualified_member_name(type_name);
  for(Scope * current = &scope.require(); current; current = current->parent) {
    ClassInfo * info = current->class_info;
    if(!info) {
      continue;
    }
    const std::string template_name =
        info->source_template ?
            info->source_template->name :
            strip_trailing_top_level_template_arguments(info->name);
    if(unqualified_type_name != template_name &&
       unqualified_type_name !=
           strip_trailing_top_level_template_arguments(info->name)) {
      continue;
    }
    const std::vector<std::string> arg_texts =
        current_specialization_argument_texts(*info);
    if(arg_texts.empty()) {
      return std::string();
    }
    return type_name + "<" + join_template_source_arguments(arg_texts) + ">";
  }
  return std::string();
}

std::string rewrite_current_specialization_names_in_source_text(
    template_api::TemplateEnvironmentHandle scope,
    const std::string & source_text)
{
  if(!scope.valid() || source_text.empty()) {
    return source_text;
  }
  std::string out = source_text;
  for(Scope * current = &scope.require(); current; current = current->parent) {
    ClassInfo * info = current->class_info;
    if(!info) {
      continue;
    }
    const std::vector<std::string> arg_texts =
        current_specialization_argument_texts(*info);
    if(arg_texts.empty()) {
      continue;
    }
    const std::string template_name =
        info->source_template ?
            info->source_template->name :
            strip_trailing_top_level_template_arguments(info->name);
    if(template_name.empty()) {
      continue;
    }
    const std::string replacement =
        template_name + "<" + join_template_source_arguments(arg_texts) + ">";
    bool changed = false;
    out = callsemantic_internal::replace_identifier_token_text(out,
                                                               template_name,
                                                               replacement,
                                                               changed);
    const std::string class_name =
        strip_trailing_top_level_template_arguments(info->name);
    if(!class_name.empty() && class_name != template_name) {
      out = callsemantic_internal::replace_identifier_token_text(out,
                                                                 class_name,
                                                                 replacement,
                                                                 changed);
    }
  }
  return out;
}

std::string source_binding_current_specialization_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & argument,
    const std::string & explicit_text)
{
  if(argument.kind != TemplateArgument::TA_TYPE || !argument.type) {
    return std::string();
  }

  const std::string source_text = trim_space(explicit_text);
  if(source_text.empty() || source_text.find('<') != std::string::npos) {
    return std::string();
  }

  const std::string scope_text =
      source_binding_current_specialization_text_from_scope(scope,
                                                            argument,
                                                            source_text);
  if(!scope_text.empty()) {
    return scope_text;
  }

  template_api::TemplateNamedTypeMetadata metadata;
  const std::vector<std::string> * metadata_arg_texts = nullptr;
  if(!service_describe_named_type_metadata(services, argument.type, metadata) ||
     !metadata.source_template ||
     !(metadata_arg_texts = template_metadata::argument_texts(metadata))) {
    return std::string();
  }

  const std::string type_name =
      trim_space(strip_elaborated_type_prefix(source_text));
  const std::string unqualified_type_name =
      unqualified_member_name(type_name).empty() ?
          type_name :
          unqualified_member_name(type_name);
  if(unqualified_type_name != metadata.source_template->name &&
     unqualified_type_name != metadata.name) {
    return std::string();
  }

  return type_name + "<" +
         join_template_source_arguments(*metadata_arg_texts) + ">";
}

std::string dependent_member_type_owner_text_from_scope(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & member_name)
{
  if(member_name.empty() || member_name.find("::") != std::string::npos ||
     !scope.valid()) {
    return std::string();
  }
  for(Scope * current = &scope.require(); current; current = current->parent) {
    for(std::map<std::string, TypePtr>::const_iterator it =
            current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      Scope * member_scope = nullptr;
      if(!service_prepare_named_type_member_scope(services,
                                                  scope,
                                                  it->second,
                                                  member_scope) ||
         !member_scope) {
        continue;
      }
      if(member_scope->named_types.find(member_name) !=
         member_scope->named_types.end()) {
        return it->first;
      }
    }
  }
  return std::string();
}

std::string alias_template_source_binding_arg_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & argument,
    const std::string & explicit_text)
{
  const std::string current_specialization_text =
      source_binding_current_specialization_text(services,
                                                 scope,
                                                 argument,
                                                 explicit_text);
  if(!current_specialization_text.empty()) {
    return current_specialization_text;
  }
  const std::string trimmed_explicit = trim_space(explicit_text);
  const std::string rewritten_explicit =
      rewrite_current_specialization_names_in_source_text(scope,
                                                          trimmed_explicit);
  if(rewritten_explicit != trimmed_explicit) {
    return rewritten_explicit;
  }
  if(argument.kind == TemplateArgument::TA_TYPE && !trimmed_explicit.empty()) {
    const std::string typename_prefix = "typename ";
    const bool explicit_had_typename =
        trimmed_explicit.compare(0,
                                 typename_prefix.size(),
                                 typename_prefix) == 0;
    const std::string explicit_core =
        explicit_had_typename ?
            trim_space(trimmed_explicit.substr(typename_prefix.size())) :
            trimmed_explicit;
    const std::string argument_text = trim_space(argument.text);
    const std::string argument_core =
        argument_text.compare(0, typename_prefix.size(), typename_prefix) == 0 ?
            trim_space(argument_text.substr(typename_prefix.size())) :
            argument_text;
    if(explicit_core.find("::") == std::string::npos) {
      const std::string owner =
          dependent_member_type_owner_text_from_scope(services,
                                                     scope,
                                                     explicit_core);
      if(!owner.empty()) {
        return (explicit_had_typename ? typename_prefix : std::string()) +
            owner + "::" + explicit_core;
      }
    }
    if(explicit_core.find("::") == std::string::npos &&
       argument_core.find("::") != std::string::npos &&
       unqualified_member_name(argument_core) == explicit_core) {
      return explicit_had_typename &&
             argument_text.compare(0,
                                   typename_prefix.size(),
                                   typename_prefix) != 0 ?
          typename_prefix + argument_text :
          argument_text;
    }
    const std::string semantic_text =
        template_model::template_argument_text(
            argument,
            [&services](const TypePtr & type)
            {
              return service_lookup_text_for_type_argument(services, type);
            });
    const std::string semantic_core =
        semantic_text.compare(0, typename_prefix.size(), typename_prefix) == 0 ?
            trim_space(semantic_text.substr(typename_prefix.size())) :
            semantic_text;
    if(explicit_core.find("::") == std::string::npos &&
       semantic_core.find("::") != std::string::npos &&
       unqualified_member_name(semantic_core) == explicit_core) {
      return explicit_had_typename &&
             semantic_text.compare(0,
                                   typename_prefix.size(),
                                   typename_prefix) != 0 ?
          typename_prefix + semantic_text :
          semantic_text;
    }
  }
  return trimmed_explicit;
}

vector<string> source_argument_texts_for_occurrence(
    const vector<string> & arg_texts,
    const vector<TemplateArgumentSyntax> * arg_syntaxes)
{
  vector<string> out = arg_texts;
  if(!arg_syntaxes) {
    return out;
  }
  const size_t limit = std::min(out.size(), arg_syntaxes->size());
  for(size_t i = 0; i < limit; ++i) {
    string text = semantic_utils::trim_space(
        (*arg_syntaxes)[i].source_text.empty() ?
            (*arg_syntaxes)[i].text :
            (*arg_syntaxes)[i].source_text);
    if((*arg_syntaxes)[i].expression) {
      const string expression_text =
          semantic_utils::trim_space(
              callsemantic_internal::describe_expression_for_diagnostic(
                  *(*arg_syntaxes)[i].expression));
      if(!expression_text.empty() &&
         compact_source_argument_key(expression_text) ==
             compact_source_argument_key((*arg_syntaxes)[i].text)) {
        text = expression_text;
      }
    }
    if((*arg_syntaxes)[i].type_id) {
      const string type_text =
          semantic_utils::trim_space(node_text(*(*arg_syntaxes)[i].type_id));
      if(!type_text.empty() &&
         (text.empty() ||
          compact_source_argument_key(type_text) ==
              compact_source_argument_key(text))) {
        text = type_text;
      }
    }
    if(text.empty()) {
      continue;
    }
    if(i < out.size() &&
       !out[i].empty() &&
       compact_source_argument_key(text) !=
           compact_source_argument_key(out[i])) {
      continue;
    }
    if((*arg_syntaxes)[i].pack_expansion &&
       (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
      text += "...";
    }
    out[i] = text;
  }
  return out;
}

vector<string> source_argument_texts_from_syntaxes(
    const vector<TemplateArgumentSyntax> * arg_syntaxes)
{
  vector<string> out;
  if(!arg_syntaxes) {
    return out;
  }
  out.reserve(arg_syntaxes->size());
  for(size_t i = 0; i < arg_syntaxes->size(); ++i) {
    string text = semantic_utils::trim_space(
        callsemantic::template_argument_syntax_witness_source_text(
            (*arg_syntaxes)[i]));
    if((*arg_syntaxes)[i].pack_expansion &&
       (text.size() < 3 || text.substr(text.size() - 3) != "...")) {
      text += "...";
    }
    out.push_back(text);
  }
  return out;
}

bool source_arguments_compact_match(const vector<string> & source_args,
                                    const vector<string> & arg_texts)
{
  if(source_args.size() != arg_texts.size()) {
    return false;
  }
  for(size_t i = 0; i < source_args.size(); ++i) {
    const string source_key = compact_source_argument_key(source_args[i]);
    const string arg_key = compact_source_argument_key(arg_texts[i]);
    if(source_key == arg_key) {
      continue;
    }
    const string typename_prefix = "typename";
    string source_core = source_key;
    string arg_core = arg_key;
    const bool source_typename =
        source_core.compare(0, typename_prefix.size(), typename_prefix) == 0;
    const bool arg_typename =
        arg_core.compare(0, typename_prefix.size(), typename_prefix) == 0;
    if(source_typename) {
      source_core = source_core.substr(typename_prefix.size());
    }
    if(arg_typename) {
      arg_core = arg_core.substr(typename_prefix.size());
    }
    if(source_typename != arg_typename ||
       source_core.find("::") == string::npos ||
       arg_core.find("::") != string::npos ||
       unqualified_member_name(source_core) != arg_core) {
      return false;
    }
  }
  return true;
}

void prefer_source_arguments_with_pack_spellings(
    const vector<string> & candidate,
    vector<string> & out)
{
  if(candidate.size() != out.size()) {
    return;
  }
  for(size_t i = 0; i < candidate.size(); ++i) {
    const string trimmed_candidate = semantic_utils::trim_space(candidate[i]);
    if(trimmed_candidate.find("...") == string::npos) {
      continue;
    }
    const string trimmed_current = semantic_utils::trim_space(out[i]);
    if(trimmed_current == trimmed_candidate ||
       compact_source_argument_key(trimmed_current) ==
           compact_source_argument_key(trimmed_candidate)) {
      continue;
    }
    out[i] = trimmed_candidate;
  }
}

bool alias_argument_text_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  if(!scope.valid() || text.empty()) {
    return false;
  }
  return text_mentions_template_placeholders(services, scope, text) ||
         text_mentions_dependent_non_namespace_binding_names(services,
                                                             scope,
                                                             text);
}

bool ast_node_has_leading_typename_qualified_member(
    const CppAstNode & node,
    bool inherited_leading_typename = false)
{
  const bool has_leading_typename =
      inherited_leading_typename || node.has_leading_typename;
  if(has_leading_typename) {
    if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
      if(!qualified->qualifiers.empty()) {
        return true;
      }
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_has_leading_typename_qualified_member(
           node.children[i],
           has_leading_typename)) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_preserves_qualified_member(
    const TemplateArgumentSyntax & syntax)
{
  return (syntax.type_id &&
          ast_node_has_leading_typename_qualified_member(*syntax.type_id)) ||
         (syntax.expression &&
          ast_node_has_leading_typename_qualified_member(*syntax.expression));
}

bool template_argument_preserves_qualified_member(
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_TYPE || !argument.type) {
    return false;
  }
  TypePtr owner;
  vector<string> members;
  bool leading_typename = false;
  return named_type_dependent_qualified_member(argument.type,
                                              owner,
                                              members,
                                              leading_typename,
                                              nullptr) &&
         !members.empty();
}

void mark_alias_source_binding_preserve_from_occurrence(
    vector<template_api::TemplateWitnessSourceBinding> & bindings,
    const semantic_source_use::SourceTemplateIdOccurrence & occurrence)
{
  const size_t limit =
      std::min(bindings.size(), occurrence.arguments.size());
  for(size_t i = 0; i < limit; ++i) {
    if(occurrence.arguments[i].preserve_qualified_member) {
      bindings[i].preserve_qualified_member = true;
    }
  }
}

void mark_alias_template_id_occurrence_argument_facts(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const vector<string> & source_arg_texts,
    const vector<TemplateArgumentSyntax> * arg_syntaxes,
    const vector<TemplateArgument> & arguments,
    semantic_source_use::SourceTemplateIdOccurrence & occurrence)
{
  const size_t limit =
      std::min(occurrence.arguments.size(), source_arg_texts.size());
  for(size_t i = 0; i < limit; ++i) {
    bool dependent_argument =
        alias_argument_text_mentions_template_dependency(services,
                                                         scope,
                                                         source_arg_texts[i]);
    if(arg_syntaxes && i < arg_syntaxes->size()) {
      dependent_argument =
          dependent_argument ||
          (*arg_syntaxes)[i].dependent ||
          (*arg_syntaxes)[i].pack_expansion;
    }
    if(i < arguments.size()) {
      const TemplateArgument & argument = arguments[i];
      dependent_argument =
          dependent_argument ||
          argument.dependent ||
          (argument.kind == TemplateArgument::TA_TYPE &&
           argument.type &&
           service_type_depends_on_template_parameter(services,
                                                      argument.type));
    }
    if((i < arguments.size() &&
        template_argument_preserves_qualified_member(arguments[i])) ||
       (arg_syntaxes &&
        i < arg_syntaxes->size() &&
        template_argument_syntax_preserves_qualified_member((*arg_syntaxes)[i]))) {
      occurrence.arguments[i].preserve_qualified_member = true;
    }
    if(dependent_argument) {
      occurrence.arguments[i].dependent = true;
      occurrence.has_dependent_argument = true;
    }
    if(i < arguments.size()) {
      const TemplateArgument & argument = arguments[i];
      const string current_specialization_text =
          source_binding_current_specialization_text(services,
                                                     scope,
                                                     argument,
                                                     source_arg_texts[i]);
      if(!current_specialization_text.empty()) {
        const bool has_template_id_syntax =
            arg_syntaxes &&
            i < arg_syntaxes->size() &&
            callsemantic::template_argument_syntax_contains_template_id(
                (*arg_syntaxes)[i]);
        if(!has_template_id_syntax) {
          occurrence.arguments[i].semantic_text = current_specialization_text;
        }
        occurrence.arguments[i].current_specialization = true;
        occurrence.has_current_specialization_argument = true;
      } else {
        const string rewritten =
            rewrite_current_specialization_names_in_source_text(
                scope,
                trim_space(source_arg_texts[i]));
        if(rewritten != trim_space(source_arg_texts[i])) {
          occurrence.arguments[i].semantic_text = rewritten;
          occurrence.arguments[i].current_specialization = true;
          occurrence.has_current_specialization_argument = true;
        }
      }
    }
  }
}

void collect_alias_value_owner_parameter_names_from_node(
    const CppAstNode & node,
    std::set<std::string> & out);

void collect_alias_value_owner_parameter_names_from_syntax(
    const TemplateArgumentSyntax & syntax,
    std::set<std::string> & out);

void collect_alias_value_owner_parameter_names_from_template_id(
    const TemplateIdSyntax & syntax,
    std::set<std::string> & out)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    collect_alias_value_owner_parameter_names_from_syntax(
        syntax.argument_syntaxes[i],
        out);
  }
}

void collect_alias_value_owner_parameter_names_from_syntax(
    const TemplateArgumentSyntax & syntax,
    std::set<std::string> & out)
{
  if(syntax.template_id) {
    collect_alias_value_owner_parameter_names_from_template_id(*syntax.template_id,
                                                              out);
  }
  if(syntax.type_id) {
    collect_alias_value_owner_parameter_names_from_node(*syntax.type_id, out);
  }
  if(syntax.expression) {
    collect_alias_value_owner_parameter_names_from_node(*syntax.expression, out);
  }
}

void collect_alias_value_owner_parameter_names_from_node(
    const CppAstNode & node,
    std::set<std::string> & out)
{
  if(node.qualified_name_syntax &&
     node.qualified_name_syntax->qualifiers.size() == 1 &&
     node.qualified_name_syntax->name == "value" &&
     !node.qualified_name_syntax->qualifiers[0].empty()) {
    out.insert(node.qualified_name_syntax->qualifiers[0]);
  }
  if(node.template_id_syntax) {
    collect_alias_value_owner_parameter_names_from_template_id(
        *node.template_id_syntax,
        out);
  }
  if(node.conversion_type_id_syntax) {
    collect_alias_value_owner_parameter_names_from_node(
        *node.conversion_type_id_syntax,
        out);
  }
  if(node.base_type_syntax) {
    collect_alias_value_owner_parameter_names_from_node(*node.base_type_syntax,
                                                       out);
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_alias_value_owner_parameter_names_from_template_id(
        node.qualifier_template_id_syntaxes[i],
        out);
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    collect_alias_value_owner_parameter_names_from_node(
        node.qualifier_type_syntaxes[i],
        out);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_alias_value_owner_parameter_names_from_node(node.children[i], out);
  }
}

std::set<size_t> alias_value_owner_parameter_indices(
    const AliasTemplateDecl & alias_template)
{
  std::set<size_t> out;
  if(!alias_template.type_id) {
    return out;
  }

  std::set<std::string> value_owner_names;
  collect_alias_value_owner_parameter_names_from_node(*alias_template.type_id,
                                                     value_owner_names);
  for(size_t i = 0; i < alias_template.parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = alias_template.parameters[i];
    if(!parameter.name.empty() &&
       value_owner_names.count(parameter.name) != 0) {
      out.insert(i);
      continue;
    }
    for(size_t j = 0; j < parameter.alternate_names.size(); ++j) {
      if(!parameter.alternate_names[j].empty() &&
         value_owner_names.count(parameter.alternate_names[j]) != 0) {
        out.insert(i);
        break;
      }
    }
  }
  return out;
}

std::string class_symbol_or_output_name_for_witness(const ClassInfo & info)
{
  const string symbol_text =
      !info.symbol_qualified_name_syntax.name.empty() ?
          template_api::qualified_name_text(info.symbol_qualified_name_syntax) :
          string();
  if(symbol_text.find("_GLOBAL__N_") != string::npos) {
    return symbol_text;
  }
  if(info.qualified_name.find("_GLOBAL__N_") != string::npos) {
    return info.qualified_name;
  }
  return semantic_model::class_output_qualified_name(info);
}

std::string value_binding_witness_entity(const ValueBinding & binding)
{
  if(binding.declaration_scope &&
     binding.declaration_scope->class_info &&
     !semantic_model::class_output_qualified_name(
          *binding.declaration_scope->class_info).empty()) {
    return class_symbol_or_output_name_for_witness(
        *binding.declaration_scope->class_info) + "::" + binding.name;
  }
  return binding.name;
}

std::string value_binding_witness_decl_location(SemanticContext & ctx,
                                                const ValueBinding & binding)
{
  if(!binding.declaration_node) {
    return std::string();
  }
  std::string location =
      ctx.source_location_for_name_in_node(*binding.declaration_node,
                                           binding.name);
  if(location.empty()) {
    location = ctx.source_location_for_node(*binding.declaration_node);
  }
  return template_api::normalize_template_witness_source_location(location);
}

bool add_value_decl_location_reference(
    const std::string & location,
    semantic_source_use::SourceTemplateArgumentOccurrence & occurrence)
{
  const std::string normalized =
      template_api::normalize_template_witness_source_location(location);
  if(normalized.empty() ||
     std::find(occurrence.referenced_value_decl_locations.begin(),
               occurrence.referenced_value_decl_locations.end(),
               normalized) !=
         occurrence.referenced_value_decl_locations.end()) {
    return false;
  }
  occurrence.referenced_value_decl_locations.push_back(normalized);
  return true;
}

bool add_class_template_value_member_decl_location_reference(
    template_api::TemplateServices & services,
    const ClassTemplateDecl & class_template,
    semantic_source_use::SourceTemplateArgumentOccurrence & occurrence)
{
  if(!services.semantic_context || !class_template.class_node) {
    return false;
  }
  const std::string location =
      services.semantic_context->source_location_for_name_in_node(
          *class_template.class_node,
          "value");
  return add_value_decl_location_reference(location, occurrence);
}

bool add_value_binding_argument_reference(
    template_api::TemplateServices & services,
    const ValueBinding & binding,
    semantic_source_use::SourceTemplateArgumentOccurrence & occurrence)
{
  if(binding.kind != ValueBinding::VK_VARIABLE ||
     !services.semantic_context) {
    return false;
  }

  const std::string entity = value_binding_witness_entity(binding);
  if(!entity.empty() &&
     std::find(occurrence.referenced_value_entities.begin(),
               occurrence.referenced_value_entities.end(),
               entity) == occurrence.referenced_value_entities.end()) {
    occurrence.referenced_value_entities.push_back(entity);
  }

  const std::string decl_location =
      value_binding_witness_decl_location(*services.semantic_context,
                                          binding);
  const bool added_location =
      add_value_decl_location_reference(decl_location, occurrence);
  return !entity.empty() || added_location;
}

bool add_class_value_member_argument_reference(
    template_api::TemplateServices & services,
    Scope & member_scope,
    semantic_source_use::SourceTemplateArgumentOccurrence & occurrence)
{
  std::map<std::string, ValueBinding>::const_iterator found =
      member_scope.values.find("value");
  if(found == member_scope.values.end()) {
    return false;
  }
  return add_value_binding_argument_reference(services,
                                              found->second,
                                              occurrence);
}

bool add_alias_value_owner_argument_syntax_reference(
    template_api::TemplateServices & services,
    Scope * use_scope,
    const TemplateArgumentSyntax * syntax,
    semantic_source_use::SourceTemplateArgumentOccurrence & occurrence)
{
  if(!use_scope || !syntax || !syntax->template_id) {
    return false;
  }
  const std::string template_name =
      template_api::qualified_name_text(syntax->template_id->name);
  ClassTemplateDecl * class_template =
      lookup_class_template_impl(services, *use_scope, template_name);
  if(!class_template) {
    const std::string unqualified =
        semantic_utils::unqualified_member_name(template_name);
    if(unqualified != template_name) {
      class_template =
          lookup_class_template_impl(services, *use_scope, unqualified);
    }
  }
  if(!class_template || !class_template->pattern_scope) {
    return false;
  }
  if(add_class_value_member_argument_reference(services,
                                               *class_template->pattern_scope,
                                               occurrence)) {
    return true;
  }
  if(class_template->pattern_scope->class_info &&
     class_template->pattern_scope->class_info->member_scope) {
    if(add_class_value_member_argument_reference(
        services,
        *class_template->pattern_scope->class_info->member_scope,
        occurrence)) {
      return true;
    }
  }
  return add_class_template_value_member_decl_location_reference(
      services,
      *class_template,
      occurrence);
}

}  // namespace

void mark_alias_template_value_owner_argument_facts(
    template_api::TemplateServices & services,
    Scope * use_scope,
    const AliasTemplateDecl & alias_template,
    const vector<TemplateArgument> & arguments,
    const vector<TemplateArgumentSyntax> * arg_syntaxes,
    semantic_source_use::SourceTemplateIdOccurrence & occurrence)
{
  const std::set<size_t> value_owner_indices =
      alias_value_owner_parameter_indices(alias_template);
  for(std::set<size_t>::const_iterator it = value_owner_indices.begin();
      it != value_owner_indices.end();
      ++it) {
    const size_t index = *it;
    if(index >= arguments.size() || index >= occurrence.arguments.size()) {
      continue;
    }
    if(arg_syntaxes && index < arg_syntaxes->size()) {
      add_alias_value_owner_argument_syntax_reference(
          services,
          use_scope,
          &(*arg_syntaxes)[index],
          occurrence.arguments[index]);
    }
  }
}

namespace {

vector<string> source_argument_texts_for_occurrence(
    const string & use_location,
    const string & template_name,
    const vector<string> & arg_texts,
    const vector<TemplateArgumentSyntax> * arg_syntaxes)
{
  vector<string> syntax_args =
      source_argument_texts_for_occurrence(arg_texts, arg_syntaxes);
  const vector<string> * source_args =
      template_api::current_template_id_source_arguments_ptr(use_location,
                                                             template_name);
  if(source_args && source_arguments_compact_match(*source_args, arg_texts)) {
    return *source_args;
  }
  const vector<string> syntax_source_args =
      source_argument_texts_from_syntaxes(arg_syntaxes);
  if(source_args &&
     !syntax_source_args.empty() &&
     source_arguments_compact_match(*source_args, syntax_source_args)) {
    return *source_args;
  }
  if(!syntax_source_args.empty()) {
    prefer_source_arguments_with_pack_spellings(syntax_source_args, syntax_args);
  }
  return syntax_args;
}

string template_id_syntax_lookup_text(const TemplateIdSyntax & syntax)
{
  ostringstream out;
  out << qualified_name_text_for_structured_lookup(syntax.name) << "<";
  const vector<string> arg_texts = template_id_syntax_argument_texts(syntax);
  for(size_t i = 0; i < arg_texts.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << arg_texts[i];
  }
  out << ">";
  return out.str();
}

bool request_should_defer_unresolved_type_lookup(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & lookup_text)
{
  const template_api::TemplateEnvironmentHandle env =
      template_api::make_template_environment(scope);
  return text_mentions_template_placeholders(services, env, lookup_text) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, env, lookup_text) ||
         should_defer_unresolved_type_lookup(services, scope, lookup_text);
}

string current_specialization_argument_match_key(const string & text)
{
  return remove_space_chars(normalize_type_lookup_name(trim_space(text)));
}

bool current_specialization_argument_texts_match(
    const vector<string> & source_args,
    const vector<string> & current_args)
{
  if(source_args.size() != current_args.size()) {
    return false;
  }
  for(size_t i = 0; i < source_args.size(); ++i) {
    if(current_specialization_argument_match_key(source_args[i]) !=
       current_specialization_argument_match_key(current_args[i])) {
      return false;
    }
  }
  return true;
}

bool current_specialization_template_name_matches(
    const ClassInfo & info,
    const QualifiedName & name)
{
  if(!info.source_template || name.name != info.source_template->name) {
    return false;
  }
  if(!name.rooted && name.qualifiers.empty()) {
    return true;
  }
  if(!info.source_template->declaring_scope) {
    return false;
  }

  string lookup_text = qualified_name_text_for_structured_lookup(name);
  if(lookup_text.compare(0, 2, "::") == 0) {
    lookup_text.erase(0, 2);
  }
  const string qualified_template_name =
      semantic_lookup::scope_qualified_name(*info.source_template->declaring_scope,
                                            info.source_template->name);
  return lookup_text == qualified_template_name;
}

TypePtr current_specialization_type_for_template_id_syntax(
    Scope & scope,
    const TemplateIdSyntax & syntax,
    const vector<string> & arg_texts,
    bool allow_enclosing_current_specializations)
{
  for(Scope * current = &scope; current; current = current->parent) {
    ClassInfo * info = current->class_info;
    if(!info) {
      continue;
    }
    const vector<string> * current_args = nullptr;
    if(!info->source_template ||
       !(current_args = template_metadata::argument_texts(*info))) {
      if(!allow_enclosing_current_specializations) {
        break;
      }
      continue;
    }
    if(current_specialization_template_name_matches(*info, syntax.name) &&
       current_specialization_argument_texts_match(arg_texts, *current_args)) {
      return info->type;
    }
    if(!allow_enclosing_current_specializations) {
      break;
    }
  }
  return TypePtr();
}

bool try_resolve_member_template_id_from_class_scope(
    template_api::TemplateServices & services,
    Scope & lookup_scope,
    const QualifiedName & template_name,
    bool reference_class_templates_only,
    const string & original_text,
    const vector<string> & arg_texts,
    const vector<TemplateArgumentSyntax> * arg_syntaxes,
    const string & source_location,
    template_api::TemplateEnvironmentHandle argument_scope,
    template_api::ClassTemplateSourceUseMode source_use_mode,
    TypePtr & out)
{
  out.reset();
  if(template_name.rooted ||
     !template_name.qualifiers.empty() ||
     template_name.name.empty() ||
     !lookup_scope.class_info ||
     !services.semantic_context) {
    return false;
  }

  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ?
          argument_scope :
          template_api::make_template_environment(lookup_scope);

  semantic_lookup::MemberAliasTemplateLookupResult alias_lookup =
      semantic_lookup::lookup_member_alias_template(
          *services.semantic_context,
          *lookup_scope.class_info,
          template_name.name);
  if(alias_lookup.alias_template) {
    if(alias_lookup.declared_in == lookup_scope.class_info) {
      return false;
    }
    out = services.semantic_context->instantiate_alias_template_with_syntax(
        *alias_lookup.alias_template,
        effective_argument_scope.require(),
        arg_texts,
        arg_syntaxes,
        reference_class_templates_only);
    resolve_instantiated_dependent_type_if_needed(
        services,
        effective_argument_scope,
        out);
    return out != nullptr;
  }

  semantic_lookup::MemberClassTemplateLookupResult class_lookup =
      semantic_lookup::lookup_member_class_template(
          *services.semantic_context,
          *lookup_scope.class_info,
          template_name.name);
  if(class_lookup.class_template) {
    if(class_lookup.declared_in == lookup_scope.class_info) {
      return false;
    }
    template_api::TemplateTypeLookupRequest request;
    request.scope = &lookup_scope;
    request.allow_class_templates = reference_class_templates_only;
    request.name = template_name;
    request.source_use_mode = source_use_mode;
    request.source_location =
        template_api::normalize_template_witness_source_location(source_location);
    return try_resolve_class_template_id_locally(
               services,
               template_api::make_template_environment(lookup_scope),
               request,
               original_text,
               class_lookup.class_template,
               arg_texts,
               arg_syntaxes,
               effective_argument_scope,
               out) &&
           out;
  }

  return false;
}

bool scope_has_dependent_instantiation_owner(Scope & scope);

bool try_make_dependent_qualified_template_id_type(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateIdSyntax & syntax,
    const string & lookup_text,
    bool reference_class_templates_only,
    TypePtr & out)
{
  out.reset();
  if(syntax.name.rooted ||
     syntax.name.qualifiers.empty() ||
     syntax.name.name.empty()) {
    return false;
  }

  const string owner_name =
      strip_elaborated_type_prefix(trim_space(syntax.name.qualifiers[0]));
  if(owner_name.empty() ||
     owner_name.find('<') != string::npos) {
    return false;
  }

  TypePtr owner_type = lookup_exact_bound_type_name(scope, owner_name);
  if(!owner_type) {
    owner_type = lookup_local_dependent_type_placeholder(scope, owner_name);
  }
  if(!owner_type &&
     is_identifier_text(owner_name) &&
     (scope_has_template_placeholders(
          services, template_api::make_template_environment(scope)) ||
      scope_has_dependent_instantiation_owner(scope))) {
    owner_type = make_semantic_named(owner_name,
                                     Type::NSK_DEPENDENT_TYPE,
                                     owner_name,
                                     true);
  }
  if(!owner_type) {
    return false;
  }

  TypePtr resolved_owner;
  if(resolve_instantiated_dependent_type(
         services,
         template_api::make_template_environment(scope),
         owner_type,
         resolved_owner) &&
     resolved_owner) {
    owner_type = resolved_owner;
  }
  if(!service_type_depends_on_template_parameter(services, owner_type)) {
    Scope * member_scope = nullptr;
    if(!prepare_concrete_type_member_scope(services,
                                           template_api::make_template_environment(scope),
                                           owner_type,
                                           member_scope) ||
       !member_scope) {
      return false;
    }

    TemplateIdSyntax leaf = syntax;
    leaf.name.rooted = false;
    leaf.name.qualifiers.clear();
    bool ok = false;
    if(member_scope->class_info && services.semantic_context) {
      const vector<string> arg_texts = template_id_syntax_argument_texts(leaf);
      if(AliasTemplateDecl * alias_template =
             semantic_lookup::lookup_member_alias_template(
                 *services.semantic_context,
                 *member_scope->class_info,
                 leaf.name.name).alias_template) {
        out = services.semantic_context->instantiate_alias_template_with_syntax(
            *alias_template,
            scope,
            arg_texts,
            &leaf.argument_syntaxes,
            reference_class_templates_only);
        resolve_instantiated_dependent_type_if_needed(
            services,
            template_api::make_template_environment(scope),
            out);
        ok = out != nullptr;
      }
      if(!ok) {
        semantic_lookup::MemberClassTemplateLookupResult class_lookup =
            semantic_lookup::lookup_member_class_template(
                *services.semantic_context,
                *member_scope->class_info,
                leaf.name.name);
        if(ClassTemplateDecl * class_template = class_lookup.class_template) {
          template_api::TemplateTypeLookupRequest request;
          request.scope = member_scope;
          request.allow_class_templates = reference_class_templates_only;
          request.name = leaf.name;
          ok = try_resolve_class_template_id_locally(
                   services,
                   template_api::make_template_environment(*member_scope),
                   request,
                   template_id_syntax_lookup_text(leaf),
                   class_template,
                   arg_texts,
                   &leaf.argument_syntaxes,
                   template_api::make_template_environment(scope),
                   out) &&
               out;
        }
      }
    }
    if(!ok) {
      ok = resolve_template_id_syntax_type(
               services,
               *member_scope,
               leaf,
               reference_class_templates_only,
               string(),
               out,
               template_api::make_template_environment(scope),
               template_api::ClassTemplateSourceUseMode::EmitClassUse,
               false) &&
           out;
    }
    return ok;
  }

  vector<string> members;
  vector<TemplateIdSyntax> member_template_ids;
  for(size_t i = 1; i < syntax.name.qualifiers.size(); ++i) {
    members.push_back(syntax.name.qualifiers[i]);
    member_template_ids.push_back(TemplateIdSyntax());
  }
  members.push_back(syntax.name.name);

  TemplateIdSyntax leaf = syntax;
  leaf.name.rooted = false;
  leaf.name.qualifiers.clear();
  member_template_ids.push_back(leaf);

  out = make_structured_dependent_qualified_member_type(lookup_text,
                                                        owner_type,
                                                        members,
                                                        false,
                                                        member_template_ids);
  return out != nullptr;
}

}  // namespace

bool resolve_template_id_syntax_type(template_api::TemplateServices & services,
                                     Scope & scope,
                                     const TemplateIdSyntax & syntax,
                                     bool reference_class_templates_only,
                                     const string & source_location,
                                     TypePtr & out,
                                     template_api::TemplateEnvironmentHandle
                                         argument_scope,
                                     template_api::ClassTemplateSourceUseMode source_use_mode,
                                     bool allow_enclosing_current_specializations)
{
  out.reset();
  const vector<string> arg_texts = template_id_syntax_argument_texts(syntax);
  if(syntax.name.name.empty()) {
    return false;
  }
  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ? argument_scope : template_api::make_template_environment(scope);

  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = reference_class_templates_only;
  request.name = syntax.name;
  request.source_use_mode = source_use_mode;
  request.source_location =
      template_api::normalize_template_witness_source_location(source_location);

  const string lookup_text = template_id_syntax_lookup_text(syntax);
  if(TypePtr current_specialization =
         current_specialization_type_for_template_id_syntax(
             scope,
             syntax,
             arg_texts,
             allow_enclosing_current_specializations)) {
    out = current_specialization;
    return true;
  }
  if(TypePtr current_specialization =
         current_specialization_type_for_lookup_text(services, scope, lookup_text)) {
    out = current_specialization;
    return true;
  }

  if(try_resolve_type_pack_element_template_id(
         services,
         effective_argument_scope,
         syntax.name,
         arg_texts,
         &syntax.argument_syntaxes,
         out)) {
    return true;
  }
  if(try_resolve_concrete_unary_type_transform_template_id(
         services,
         effective_argument_scope,
         syntax.name,
         arg_texts,
         &syntax.argument_syntaxes,
         out)) {
    return true;
  }
  if(try_resolve_member_template_id_from_class_scope(
         services,
         scope,
         syntax.name,
         reference_class_templates_only,
         lookup_text,
         arg_texts,
         &syntax.argument_syntaxes,
         source_location,
         effective_argument_scope,
         source_use_mode,
         out)) {
    return true;
  }
  if(try_resolve_alias_template_id_locally(
         services,
         template_api::make_template_environment(scope),
         request,
         syntax.name,
         arg_texts,
         &syntax.argument_syntaxes,
         effective_argument_scope,
         out)) {
    return true;
  }

  if(ClassTemplateDecl * class_template =
         lookup_class_template_impl(
             services, scope, qualified_name_text_for_structured_lookup(syntax.name))) {
    if(try_resolve_class_template_id_locally(
           services,
           template_api::make_template_environment(scope),
           request,
           lookup_text,
           class_template,
           arg_texts,
           &syntax.argument_syntaxes,
           effective_argument_scope,
           out)) {
      return true;
    }
  }

  if(request_should_defer_unresolved_type_lookup(services, scope, lookup_text)) {
    if(try_make_dependent_qualified_template_id_type(services,
                                                     scope,
                                                     syntax,
                                                     lookup_text,
                                                     reference_class_templates_only,
                                                     out)) {
      return true;
    }
    out = make_semantic_named(lookup_text,
                              Type::NSK_DEPENDENT_TYPE,
                              lookup_text,
                              true);
    return true;
  }
  return false;
}

bool resolve_type_argument_syntax_type(template_api::TemplateServices & services,
                                       template_api::TemplateEnvironmentHandle scope,
                                       const TemplateArgumentSyntax & syntax,
                                       bool reference_class_templates_only,
                                       TypePtr & out)
{
  out.reset();
  if(!scope.valid()) {
    return false;
  }
  if(syntax.resolved_type) {
    out = syntax.resolved_type;
    return true;
  }
  if(syntax.type_id) {
    return parse_type_id_node_for_templates(services,
                                            scope.require(),
                                            *syntax.type_id,
                                            out,
                                            reference_class_templates_only) &&
           out != nullptr;
  }
  if(syntax.template_id) {
    return resolve_template_id_syntax_type(services,
                                           scope.require(),
                                           *syntax.template_id,
                                           reference_class_templates_only,
                                           string(),
                                           out,
                                           scope) &&
           out != nullptr;
  }
  return false;
}

bool resolve_type_argument_input(template_api::TemplateServices & services,
                                 template_api::TemplateEnvironmentHandle scope,
                                 const TemplateArgumentSyntax * syntax,
                                 bool reference_class_templates_only,
                                 TypePtr & out)
{
  if(syntax &&
     resolve_type_argument_syntax_type(
         services, scope, *syntax, reference_class_templates_only, out) &&
     out) {
    return true;
  }
  out.reset();
  return false;
}

namespace {

bool scope_has_dependent_instantiation_owner(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->class_info && current->class_info->dependent_instantiation) {
      return true;
    }
  }
  return false;
}

bool type_matches_current_instantiation(Scope & scope, const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->class_info &&
       current->class_info->type &&
       type_equals(strip_top_level_cv(current->class_info->type), base)) {
      return true;
    }
  }
  return false;
}

bool dependent_qualified_type_missing_required_typename(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & owner_lookup_text,
    const TypePtr & owner_type,
    const string & lookup_text,
    bool has_leading_typename)
{
  if(has_leading_typename ||
     base_specifier_type_lookup_allows_implicit_typename(lookup_text)) {
    return false;
  }
  if(type_matches_current_instantiation(scope, owner_type)) {
    return false;
  }
  const string normalized_owner =
      normalize_type_lookup_name(trim_space(owner_lookup_text));
  return !current_specialization_type_for_lookup_text(
      services, scope, normalized_owner);
}

bool dependent_qualified_lookup_is_type_required_context(
    const CppAstNode & node)
{
  switch(node.kind) {
  case CppAstKind::base_name:
  case CppAstKind::base_specifier:
  case CppAstKind::decl_specifier:
  case CppAstKind::decl_specifier_seq:
  case CppAstKind::default_template_argument:
  case CppAstKind::parameter_declaration:
  case CppAstKind::trailing_return_type:
  case CppAstKind::type_id:
  case CppAstKind::type_name:
  case CppAstKind::type_specifier:
  case CppAstKind::type_specifier_seq:
    return true;
  default:
    return false;
  }
}

StructuredTypeLookupResult resolve_bound_owner_qualified_name_syntax_type(
    template_api::TemplateServices & services,
    Scope & scope,
    const QualifiedName & name,
    bool has_leading_typename,
    const string & lookup_text,
    bool require_missing_typename_diagnostic,
    TypePtr & out)
{
  out.reset();
  if(name.rooted || name.qualifiers.empty()) {
    return StructuredTypeLookupResult::NotApplicable;
  }

  TypePtr owner_type =
      lookup_exact_bound_type_name(scope, strip_elaborated_type_prefix(name.qualifiers[0]));
  if(!owner_type) {
    owner_type = lookup_local_dependent_type_placeholder(scope, name.qualifiers[0]);
  }
  if(!owner_type) {
    const string direct_owner_name =
        strip_elaborated_type_prefix(trim_space(name.qualifiers[0]));
    if(!direct_owner_name.empty()) {
      for(Scope * current = &scope; current; current = current->parent) {
        owner_type =
            lookup_concrete_type_in_resolved_scope(
                services,
                template_api::make_template_environment(scope),
                *current,
                direct_owner_name);
        if(owner_type) {
          break;
        }
        if(current->parent == nullptr) {
          break;
        }
      }
      if(!owner_type &&
         is_identifier_text(direct_owner_name)) {
        QualifiedName namespace_name;
        namespace_name.rooted = name.rooted;
        namespace_name.name = direct_owner_name;
        if(semantic_lookup::lookup_namespace_name(scope, namespace_name)) {
          return StructuredTypeLookupResult::NotApplicable;
        }
      }
      if(!owner_type &&
         is_identifier_text(direct_owner_name) &&
         (has_leading_typename ||
          scope_has_template_placeholders(
              services, template_api::make_template_environment(scope)) ||
          scope_has_dependent_instantiation_owner(scope))) {
        owner_type = make_semantic_named(direct_owner_name,
                                         Type::NSK_DEPENDENT_TYPE,
                                         direct_owner_name,
                                         true);
      }
    }
  }
  if(!owner_type) {
    return StructuredTypeLookupResult::NotApplicable;
  }

  TypePtr resolved_owner;
  if(resolve_instantiated_dependent_type(
         services,
         template_api::make_template_environment(scope),
         owner_type,
         resolved_owner) &&
     resolved_owner) {
    owner_type = resolved_owner;
  }
  if(service_type_depends_on_template_parameter(services, owner_type)) {
    if(require_missing_typename_diagnostic &&
       dependent_qualified_type_missing_required_typename(
           services,
           scope,
           name.qualifiers[0],
           owner_type,
           lookup_text,
           has_leading_typename)) {
      throw DependentQualifiedTypeMissingTypenameError(
          "dependent qualified type requires typename: " + lookup_text);
    }
    out = make_structured_dependent_qualified_member_type(
        lookup_text,
        owner_type,
        remaining_dependent_qualified_member_path(name, 0),
        has_leading_typename);
    return StructuredTypeLookupResult::Resolved;
  }

  Scope * current = nullptr;
  if(!prepare_concrete_type_member_scope(services,
                                         template_api::make_template_environment(scope),
                                         owner_type,
                                         current) ||
     !current) {
    return StructuredTypeLookupResult::NoMatch;
  }

  for(size_t i = 1; i < name.qualifiers.size(); ++i) {
    Scope * direct_namespace =
        template_api::resolve_direct_namespace_in_inline_namespaces(
            *current, name.qualifiers[i]);
    if(direct_namespace) {
      current = direct_namespace;
      continue;
    }

    TypePtr nested_type =
        lookup_concrete_type_in_resolved_scope(
            services,
            template_api::make_template_environment(scope),
            *current,
            name.qualifiers[i]);
    if(!nested_type ||
       !prepare_concrete_type_member_scope(
           services,
           template_api::make_template_environment(scope),
           nested_type,
           current) ||
       !current) {
      return StructuredTypeLookupResult::NoMatch;
    }
  }

  out = lookup_concrete_type_in_resolved_scope(
      services,
      template_api::make_template_environment(scope),
      *current,
      name.name);
  if(!out) {
    return StructuredTypeLookupResult::NoMatch;
  }

  TypePtr resolved;
  if(resolve_instantiated_dependent_type(
         services,
         template_api::make_template_environment(*current),
         out,
         resolved) &&
     resolved) {
    out = resolved;
  }
  return out ? StructuredTypeLookupResult::Resolved :
               StructuredTypeLookupResult::NoMatch;
}

StructuredTypeLookupResult resolve_qualified_name_syntax_type(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node,
    const QualifiedName & name,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out)
{
  out.reset();
  if(name.name.empty() || (!name.rooted && name.qualifiers.empty())) {
    return StructuredTypeLookupResult::NotApplicable;
  }

  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = reference_class_templates_only;
  request.name = name;
  request.source_location =
      template_api::normalize_template_witness_source_location(source_location);

  const string lookup_text = qualified_name_text_for_structured_lookup(name);
  if(TypePtr current_specialization =
         current_specialization_type_for_lookup_text(services, scope, lookup_text)) {
    out = current_specialization;
    return StructuredTypeLookupResult::Resolved;
  }
  const StructuredTypeLookupResult bound_owner_result =
      resolve_bound_owner_qualified_name_syntax_type(services,
                                                     scope,
                                                     name,
                                                     node.has_leading_typename,
                                                     lookup_text,
                                                     dependent_qualified_lookup_is_type_required_context(
                                                         node),
                                                     out);
  if(bound_owner_result != StructuredTypeLookupResult::NotApplicable) {
    return bound_owner_result;
  }
  if(service_resolve_direct_type_lookup(services, request, out) && out) {
    return StructuredTypeLookupResult::Resolved;
  }
  if(request_should_defer_unresolved_type_lookup(services, scope, lookup_text)) {
    out = make_semantic_named(lookup_text,
                              Type::NSK_DEPENDENT_TYPE,
                              lookup_text,
                              true);
    return StructuredTypeLookupResult::Resolved;
  }
  return StructuredTypeLookupResult::NotApplicable;
}

StructuredTypeLookupResult resolve_structured_type_lookup_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out)
{
  out.reset();
  if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
    if(qualified->rooted || !qualified->qualifiers.empty()) {
      const StructuredTypeLookupResult qualified_result =
          resolve_qualified_name_syntax_type(services,
                                             scope,
                                             node,
                                             *qualified,
                                             reference_class_templates_only,
                                             source_location,
                                             out);
      if(qualified_result != StructuredTypeLookupResult::NotApplicable) {
        return qualified_result;
      }
    }
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(resolve_template_id_syntax_type(services,
                                       scope,
                                       *template_id,
                                       reference_class_templates_only,
                                       source_location,
                                       out) &&
       out) {
      return StructuredTypeLookupResult::Resolved;
    }
    return StructuredTypeLookupResult::NoMatch;
  }
  return StructuredTypeLookupResult::NotApplicable;
}

bool lookup_pack_size(Scope & scope, const string & name, size_t & out);

bool template_id_syntax_has_payload(const TemplateIdSyntax & syntax)
{
  return syntax.name.rooted ||
         !syntax.name.name.empty() ||
         !syntax.name.qualifiers.empty() ||
         !syntax.arguments.empty() ||
         !syntax.argument_syntaxes.empty();
}

bool has_qualifier_template_id_syntax_payload(const CppAstNode & node)
{
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_has_payload(node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool has_qualifier_type_syntax(const CppAstNode & node)
{
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(node.qualifier_type_syntaxes[i].kind != CppAstKind::invalid) {
      return true;
    }
  }
  return false;
}

StructuredTypeLookupResult resolve_qualified_template_type_lookup_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & lookup_name,
    const CppAstNode & node,
    bool reference_class_templates_only,
    const string & source_location,
    TypePtr & out)
{
  out.reset();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
  };
  const TemplateIdSyntax * lone_template_id = cppast_template_id_syntax(node);
  const QualifiedName * source_qualified_syntax =
      cppast_qualified_name_syntax(node);
  const bool has_lone_template_id =
      lone_template_id && template_id_syntax_has_payload(*lone_template_id);
  if(!has_qualifier_template_id_syntax_payload(node) &&
     !has_qualifier_type_syntax(node) &&
     !has_lone_template_id) {
    return StructuredTypeLookupResult::NotApplicable;
  }
  const string normalized_lookup_name = normalize_type_lookup_name(lookup_name);
  const auto template_id_source_location =
      [&](const TemplateIdSyntax & syntax) -> string
  {
    if(syntax.source_location_id != 0) {
      const string location =
          template_api::template_witness_detail::source_location_for_location_id(
              services.witness_context,
              syntax.source_location_id);
      if(!location.empty()) {
        return template_api::normalize_template_witness_source_location(location);
      }
    }
    return source_location;
  };

  QualifiedName qualified;
  if(source_qualified_syntax &&
     (source_qualified_syntax->rooted ||
      !source_qualified_syntax->qualifiers.empty()) &&
     normalize_type_lookup_name(template_api::qualified_name_text(
         *source_qualified_syntax)) ==
         normalized_lookup_name) {
    qualified = *source_qualified_syntax;
  } else if(!semantic_utils::split_qualified_name_text(normalized_lookup_name,
                                                       qualified)) {
    return StructuredTypeLookupResult::NotApplicable;
  }
  if(qualified.qualifiers.empty()) {
    return StructuredTypeLookupResult::NotApplicable;
  }

  const auto local_template_id_for_current_scope =
      [](const TemplateIdSyntax & source) -> TemplateIdSyntax
      {
        TemplateIdSyntax local = source;
        if(local.name.rooted || !local.name.qualifiers.empty()) {
          local.name.rooted = false;
          local.name.qualifiers.clear();
          local.name.name = unqualified_member_name(local.name.name);
        }
        return local;
      };

  Scope * current = &scope;
  if(qualified.rooted) {
    while(current->parent) {
      current = current->parent;
    }
  }

  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    const bool qualifier_template_id_from_source =
        i < node.qualifier_template_id_syntaxes.size() &&
        template_id_syntax_has_payload(node.qualifier_template_id_syntaxes[i]);
    const TemplateIdSyntax * qualifier_template_id =
        qualifier_template_id_from_source ?
            &node.qualifier_template_id_syntaxes[i] :
            nullptr;
    if(!qualifier_template_id &&
       i == 0 &&
       lone_template_id &&
       !lone_template_id->name.name.empty()) {
      const string qualifier_head =
          unqualified_member_name(
              strip_trailing_top_level_template_arguments(
                  qualified.qualifiers[i]));
      if(qualifier_head == lone_template_id->name.name) {
        qualifier_template_id = lone_template_id;
      }
    }
    TemplateIdSyntax local_qualifier_template_id;
    const TemplateIdSyntax * lookup_qualifier_template_id = qualifier_template_id;
    if(qualifier_template_id) {
      local_qualifier_template_id =
          local_template_id_for_current_scope(*qualifier_template_id);
      lookup_qualifier_template_id = &local_qualifier_template_id;
    }
    QualifiedName qualifier_name;
    qualifier_name.rooted = qualified.rooted;
    qualifier_name.qualifiers.assign(qualified.qualifiers.begin(),
                                     qualified.qualifiers.begin() + i);
    qualifier_name.name = qualified.qualifiers[i];
    if(!qualifier_template_id) {
      Scope * namespace_scope =
          semantic_lookup::lookup_namespace_name(scope, qualifier_name);
      if(namespace_scope) {
        current = namespace_scope;
        continue;
      }
    }
    if(lookup_qualifier_template_id &&
       i + 1 == qualified.qualifiers.size()) {
      switch(try_resolve_standard_meta_member_type(
          services,
          *current,
          scope,
          qualified.name,
          *lookup_qualifier_template_id,
          out)) {
      case STANDARD_META_MEMBER_RESOLVED:
        return StructuredTypeLookupResult::Resolved;
      case STANDARD_META_MEMBER_SUBSTITUTION_FAILURE:
        return StructuredTypeLookupResult::NoMatch;
      case STANDARD_META_MEMBER_NOT_APPLICABLE:
        break;
      }
    }

    TypePtr qualifier_type;
    const CppAstNode * qualifier_type_syntax =
        cppast_qualifier_type_syntax(node, i);
    if(qualifier_type_syntax) {
      if(qualifier_type_syntax->semantic_type) {
        qualifier_type = qualifier_type_syntax->semantic_type;
      } else {
        parse_decltype_or_typeof_node(
            services, *current, *qualifier_type_syntax, qualifier_type);
      }
    }
    const TemplateIdSyntax * source_qualifier_template_id =
        lookup_qualifier_template_id ?
            (qualifier_template_id ? qualifier_template_id :
                                     lookup_qualifier_template_id) :
            nullptr;
    if(!qualifier_type && lookup_qualifier_template_id) {
      if(current->class_info && services.semantic_context) {
        TemplateIdSyntax unqualified = *lookup_qualifier_template_id;
        unqualified.name.rooted = false;
        unqualified.name.qualifiers.clear();
        const vector<string> arg_texts =
            template_id_syntax_argument_texts(unqualified);
        if(AliasTemplateDecl * alias_template =
               semantic_lookup::lookup_member_alias_template(
                   *services.semantic_context,
                   *current->class_info,
                   unqualified.name.name).alias_template) {
          qualifier_type =
              services.semantic_context->instantiate_alias_template_with_syntax(
                  *alias_template,
                  scope,
                  arg_texts,
                  &unqualified.argument_syntaxes,
                  reference_class_templates_only);
          resolve_instantiated_dependent_type_if_needed(
              services,
              template_api::make_template_environment(scope),
              qualifier_type);
        }
        if(!qualifier_type) {
          if(ClassTemplateDecl * class_template =
                 semantic_lookup::lookup_member_class_template(
                     *services.semantic_context,
                     *current->class_info,
                     unqualified.name.name).class_template) {
            template_api::TemplateTypeLookupRequest request;
            request.scope = current;
            request.allow_class_templates = reference_class_templates_only;
            request.name = unqualified.name;
            request.source_location =
                template_api::normalize_template_witness_source_location(
                    template_id_source_location(*source_qualifier_template_id));
            try_resolve_class_template_id_locally(
                services,
                template_api::make_template_environment(*current),
                request,
                template_id_syntax_lookup_text(unqualified),
                class_template,
                arg_texts,
                &unqualified.argument_syntaxes,
                template_api::make_template_environment(scope),
                qualifier_type);
          }
        }
      }
    }
    if(!qualifier_type && lookup_qualifier_template_id) {
      const string qualifier_use_location =
          template_id_source_location(*source_qualifier_template_id);
      resolve_template_id_syntax_type(services,
                                      *current,
                                      *lookup_qualifier_template_id,
                                      true,
                                      qualifier_use_location,
                                      qualifier_type,
                                      template_api::make_template_environment(scope),
                                      template_api::ClassTemplateSourceUseMode::EmitClassUse,
                                      false);
    }
    if(!qualifier_type && !qualifier_template_id) {
      template_api::TemplateTypeLookupRequest request;
      request.scope = current;
      request.allow_class_templates = true;
      request.name.name = qualified.qualifiers[i];
      service_resolve_direct_type_lookup(services, request, qualifier_type);
    }
    if(!qualifier_type) {
      if(qualifier_template_id &&
         (node.has_leading_typename ||
          base_specifier_type_lookup_allows_implicit_typename(
              normalized_lookup_name))) {
        const string qualifier_lookup_text =
            template_id_syntax_lookup_text(*qualifier_template_id);
        if(!try_make_dependent_qualified_template_id_type(services,
                                                          scope,
                                                          *qualifier_template_id,
                                                          qualifier_lookup_text,
                                                          reference_class_templates_only,
                                                          qualifier_type)) {
          qualifier_type =
              make_semantic_named(qualifier_lookup_text,
                                  Type::NSK_DEPENDENT_TYPE,
                                  qualifier_lookup_text,
                                  true);
        }
      }
    }
    if(!qualifier_type) {
      return StructuredTypeLookupResult::NoMatch;
    }

    if(type_is_dependent(qualifier_type)) {
      const bool source_requires_typename =
          (source_qualified_syntax &&
           i < source_qualified_syntax->qualifiers.size()) ||
          qualifier_template_id_from_source ||
          qualifier_type_syntax;
      if(source_requires_typename &&
         dependent_qualified_lookup_is_type_required_context(node) &&
         dependent_qualified_type_missing_required_typename(
             services,
             scope,
             qualified.qualifiers[i],
             qualifier_type,
             normalized_lookup_name,
             node.has_leading_typename)) {
        throw DependentQualifiedTypeMissingTypenameError(
            "dependent qualified type requires typename: " + normalized_lookup_name);
      }
      out = make_structured_dependent_qualified_member_type(
          normalized_lookup_name,
          qualifier_type,
          remaining_dependent_qualified_member_path(qualified, i),
          node.has_leading_typename,
          remaining_dependent_qualified_member_template_ids(node, qualified, i));
      return out ? StructuredTypeLookupResult::Resolved :
                   StructuredTypeLookupResult::NoMatch;
    }

    Scope * member_scope = nullptr;
    if(!service_prepare_named_type_member_scope(
           services,
           template_api::make_template_environment(*current),
           qualifier_type,
           member_scope) ||
       !member_scope) {
      return StructuredTypeLookupResult::NoMatch;
    }
    current = member_scope;
  }

  const TemplateIdSyntax * final_template_id = cppast_template_id_syntax(node);
  if(final_template_id) {
    TemplateIdSyntax unqualified = *final_template_id;
    unqualified.name.rooted = false;
    unqualified.name.qualifiers.clear();
    const vector<string> arg_texts = template_id_syntax_argument_texts(unqualified);
    if(current->class_info && services.semantic_context) {
      if(AliasTemplateDecl * alias_template =
             semantic_lookup::lookup_member_alias_template(
                 *services.semantic_context,
                 *current->class_info,
                 unqualified.name.name).alias_template) {
        out = services.semantic_context->instantiate_alias_template_with_syntax(
            *alias_template,
            scope,
            arg_texts,
            &unqualified.argument_syntaxes,
            reference_class_templates_only);
        resolve_instantiated_dependent_type_if_needed(
            services,
            template_api::make_template_environment(scope),
            out);
        if(out) {
          return StructuredTypeLookupResult::Resolved;
        }
      }
      if(ClassTemplateDecl * class_template =
             semantic_lookup::lookup_member_class_template(
                 *services.semantic_context,
                 *current->class_info,
                 unqualified.name.name).class_template) {
        template_api::TemplateTypeLookupRequest request;
        request.scope = current;
        request.allow_class_templates = reference_class_templates_only;
        request.name = unqualified.name;
        request.source_location =
            template_api::normalize_template_witness_source_location(
                template_id_source_location(*final_template_id));
        if(try_resolve_class_template_id_locally(
               services,
               template_api::make_template_environment(*current),
               request,
               template_id_syntax_lookup_text(unqualified),
               class_template,
               arg_texts,
               &unqualified.argument_syntaxes,
               template_api::make_template_environment(scope),
               out) &&
           out) {
          return StructuredTypeLookupResult::Resolved;
        }
      }
    }
    if(resolve_template_id_syntax_type(
           services,
           *current,
           unqualified,
           reference_class_templates_only,
           template_id_source_location(*final_template_id),
           out,
           template_api::make_template_environment(scope),
           template_api::ClassTemplateSourceUseMode::EmitClassUse,
           false) &&
       out) {
      return StructuredTypeLookupResult::Resolved;
    }
    return StructuredTypeLookupResult::NoMatch;
  }
  if(current->class_info &&
     qualified.name.find('<') == string::npos) {
    out = lookup_concrete_type_in_resolved_scope(
        services,
        template_api::make_template_environment(scope),
        *current,
        qualified.name);
    return out ? StructuredTypeLookupResult::Resolved :
                 StructuredTypeLookupResult::NoMatch;
  }
  template_api::TemplateTypeLookupRequest final_request;
  final_request.scope = current;
  final_request.allow_class_templates = reference_class_templates_only;
  final_request.name.name = qualified.name;
  if(service_resolve_direct_type_lookup(services, final_request, out) && out) {
    return StructuredTypeLookupResult::Resolved;
  }
  if(!final_template_id) {
    out = lookup_concrete_type_in_resolved_scope(
        services,
        template_api::make_template_environment(scope),
        *current,
        qualified.name);
    if(out) {
      return StructuredTypeLookupResult::Resolved;
    }
  }
  return StructuredTypeLookupResult::NoMatch;
}

const ValueBinding * lookup_leaf_value_binding_in_namespace_scope(
    Scope & scope,
    const string & name,
    unordered_set<const Scope *> & visited)
{
  if(!visited.insert(&scope).second) {
    return nullptr;
  }

  map<string, ValueBinding>::const_iterator found = scope.values.find(name);
  if(found != scope.values.end()) {
    return &found->second;
  }
  if(!scope.namespace_scope) {
    return nullptr;
  }

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    if(scope.namespace_children[i] && scope.namespace_children[i]->inline_namespace) {
      if(const ValueBinding * binding = lookup_leaf_value_binding_in_namespace_scope(
             *scope.namespace_children[i], name, visited)) {
        return binding;
      }
    }
  }
  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    if(scope.using_directives[i]) {
      if(const ValueBinding * binding = lookup_leaf_value_binding_in_namespace_scope(
             *scope.using_directives[i], name, visited)) {
        return binding;
      }
    }
  }
  return nullptr;
}

bool lookup_leaf_value_binding(Scope & scope,
                               const string & name,
                               const ValueBinding *& out)
{
  for(Scope * current = &scope; current != nullptr; current = current->parent) {
    unordered_set<const Scope *> visited;
    if(const ValueBinding * binding =
           lookup_leaf_value_binding_in_namespace_scope(*current, name, visited)) {
      out = binding;
      return true;
    }
  }
  out = nullptr;
  return false;
}

bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const string & name,
                                         const ValueBinding *& out);
bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const QualifiedName & qualified,
                                         const ValueBinding *& out);
bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const QualifiedName & qualified,
                                         const CppAstNode * node,
                                         const ValueBinding *& out);
bool lookup_leaf_qualified_function_bindings(template_api::TemplateServices & services,
                                             Scope & scope,
                                             const QualifiedName & qualified,
                                             vector<FunctionBinding *> & out);
bool lookup_leaf_qualified_function_bindings(template_api::TemplateServices & services,
                                             Scope & scope,
                                             const QualifiedName & qualified,
                                             const CppAstNode * node,
                                             vector<FunctionBinding *> & out);
bool lookup_leaf_qualified_function_templates(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const QualifiedName & qualified,
                                              vector<FunctionTemplateDecl *> & out);
bool lookup_leaf_qualified_function_templates(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const QualifiedName & qualified,
                                              const CppAstNode * node,
                                              vector<FunctionTemplateDecl *> & out);

void collect_leaf_function_bindings_in_namespace_scope(
    Scope & scope,
    const string & name,
    unordered_set<const Scope *> & visited_scopes,
    unordered_set<const FunctionBinding *> & seen_bindings,
    vector<FunctionBinding *> & out)
{
  if(!visited_scopes.insert(&scope).second) {
    return;
  }

  map<string, vector<FunctionBinding *> >::const_iterator found =
      scope.function_sets.find(name);
  if(found != scope.function_sets.end()) {
    for(size_t i = 0; i < found->second.size(); ++i) {
      FunctionBinding * binding = found->second[i];
      if(binding && seen_bindings.insert(binding).second) {
        out.push_back(binding);
      }
    }
  }
  if(!scope.namespace_scope) {
    return;
  }

  for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
    if(scope.namespace_children[i] && scope.namespace_children[i]->inline_namespace) {
      collect_leaf_function_bindings_in_namespace_scope(
          *scope.namespace_children[i], name, visited_scopes, seen_bindings, out);
    }
  }
  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    if(scope.using_directives[i]) {
      collect_leaf_function_bindings_in_namespace_scope(
          *scope.using_directives[i], name, visited_scopes, seen_bindings, out);
    }
  }
}

bool same_leaf_function_binding_entity(const FunctionBinding * lhs,
                                       const FunctionBinding * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs) {
    return false;
  }

  if(lhs->source_template || rhs->source_template) {
    if(!lhs->source_template ||
       !rhs->source_template ||
       !semantic_lookup::same_inline_namespace_function_template_entity(
           lhs->source_template, rhs->source_template) ||
       lhs->template_instantiation_key != rhs->template_instantiation_key ||
       lhs->name != rhs->name ||
       !type_equals(lhs->type, rhs->type)) {
      return false;
    }
    if(lhs->owner_class == rhs->owner_class) {
      return true;
    }
    return lhs->owner_class &&
           rhs->owner_class &&
           lhs->owner_class->qualified_name == rhs->owner_class->qualified_name;
  }

  return semantic_lookup::same_inline_namespace_function_entity(*lhs, *rhs);
}

void append_unique_leaf_function_binding(vector<FunctionBinding *> & out,
                                         FunctionBinding * binding)
{
  if(!binding) {
    return;
  }
  for(size_t i = 0; i < out.size(); ++i) {
    if(same_leaf_function_binding_entity(out[i], binding)) {
      return;
    }
  }
  out.push_back(binding);
}

void append_unique_leaf_function_bindings(vector<FunctionBinding *> & out,
                                          const vector<FunctionBinding *> & in)
{
  for(size_t i = 0; i < in.size(); ++i) {
    append_unique_leaf_function_binding(out, in[i]);
  }
}

bool simple_name_is_one_of(const string & name,
                           const char * const * names,
                           size_t count)
{
  for(size_t i = 0; i < count; ++i) {
    if(name == names[i]) {
      return true;
    }
  }
  return false;
}

bool constant_value_truthy(const constant_eval::ConstexprValue & value,
                           bool & out)
{
  return constant_eval::constexpr_value_truthy(value, out);
}

bool class_member_direct_bool_value(const Scope * member_scope,
                                    const string & name,
                                    bool & out)
{
  if(!member_scope) {
    return false;
  }
  map<string, ValueBinding>::const_iterator found =
      member_scope->values.find(name);
  if(found == member_scope->values.end() ||
     found->second.kind == ValueBinding::VK_FIELD) {
    return false;
  }
  const ValueBinding & binding = found->second;
  const auto is_bool_value_type = [](const TypePtr & type) -> bool
  {
    if(!type) {
      return false;
    }
    return is_bool_type(strip_top_level_cv(remove_reference_type(type)));
  };
  if(!is_bool_value_type(binding.type) &&
     !is_bool_value_type(binding.constexpr_value.type)) {
    return false;
  }
  if(binding.has_constant_value) {
    out = binding.constant_value != 0;
    return true;
  }
  return binding.has_constexpr_value &&
         constant_value_truthy(binding.constexpr_value, out);
}

bool structured_bool_constant_value_for_type(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    bool & out,
    set<string> & visiting,
    template_api::TemplateServices * services = nullptr,
    template_api::TemplateEnvironmentHandle scope =
        template_api::TemplateEnvironmentHandle(),
    bool * evaluation_incomplete = nullptr);
bool structured_bool_constant_value_for_class_info(
    template_api::TemplateTypeSystem & type_system,
    const ClassInfo & info,
    bool & out,
    set<string> & visiting,
    template_api::TemplateServices * services = nullptr,
    template_api::TemplateEnvironmentHandle scope =
        template_api::TemplateEnvironmentHandle(),
    bool * evaluation_incomplete = nullptr);
void note_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    set<const ClassInfo *> & visiting);
void append_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    vector<TemplateValueDependency> & out,
    set<const ClassInfo *> & visiting);

std::string strip_template_location_at_prefix(const std::string & location)
{
  if(location.compare(0, 4, " at ") == 0) {
    return location.substr(4);
  }
  return location;
}

const ClassInfo * structured_bool_integral_constant_info(
    template_api::TemplateTypeSystem & type_system,
    const ClassInfo & info,
    bool value,
    set<const ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return nullptr;
  }
  if(info.source_template &&
     info.source_template->name == "integral_constant") {
    if(info.instantiation_arguments.size() == 2 &&
       info.instantiation_arguments[0].kind == TemplateArgument::TA_TYPE &&
       is_bool_type(info.instantiation_arguments[0].type) &&
       info.instantiation_arguments[1].kind == TemplateArgument::TA_VALUE &&
       (info.instantiation_arguments[1].value != 0) == value) {
      return &info;
    }
    if(info.instantiation_arguments.size() == 1 &&
       info.instantiation_arguments[0].kind == TemplateArgument::TA_VALUE &&
       (info.instantiation_arguments[0].value != 0) == value) {
      return &info;
    }
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type || !info.bases[i].type->type) {
      continue;
    }
    ClassInfo * base_info =
        template_api::find_named_type_class_info(type_system.model,
                                                 info.bases[i].type->type);
    if(!base_info) {
      continue;
    }
    if(const ClassInfo * found =
           structured_bool_integral_constant_info(type_system,
                                                  *base_info,
                                                  value,
                                                  visiting)) {
      return found;
    }
  }
  return nullptr;
}

void note_structured_bool_integral_constant_value_for_witness(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    bool value)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }
  if(info.type &&
     service_type_depends_on_template_parameter(services, info.type)) {
    return;
  }
  set<const ClassInfo *> dependency_visiting;
  note_structured_bool_value_dependencies_for_class_info(
      services,
      info,
      dependency_visiting);
  set<const ClassInfo *> visiting;
  if(info.type &&
     service_type_depends_on_template_parameter(services, info.type)) {
    return;
  }
  const ClassInfo * integral_constant =
      structured_bool_integral_constant_info(
          service_type_system(services),
          info,
          value,
          visiting);
  if(!integral_constant || integral_constant->qualified_name.empty()) {
    return;
  }

  string decl_location;
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(
          const_cast<ClassInfo &>(*integral_constant),
          kStructuredBoolResultMemberName);
  if(member.binding) {
    decl_location = strip_template_location_at_prefix(
        semantic_model::source_decl_anchor_location(
            semantic_trace::value_decl_anchor(*services.semantic_context,
                                              member.binding)));
  }
  if(decl_location.empty() && integral_constant->source_template) {
    decl_location = strip_template_location_at_prefix(
        semantic_model::source_decl_anchor_location(
            semantic_trace::class_template_decl_anchor(
                *services.semantic_context,
                integral_constant->source_template)));
  }
  if(decl_location.empty()) {
    return;
  }

  const string entity =
      class_symbol_or_output_name_for_witness(*integral_constant) +
      "::" + kStructuredBoolResultMemberName;
  const witness::ScopedTemplateWitnessEntryContext entry_context(
      witness::make_template_closure_entry_context(
          witness::TemplateClosureReason::TrackInstantiation,
          entity,
          decl_location,
          true));
  witness::note_template_witness_log_event(
      witness::TemplateWitnessLogEventKind::VariableInstantiation,
      decl_location,
      entity,
      decl_location,
      string(),
      witness::TemplateLifecycleCause::TrackInstantiation,
      true);
}

bool append_template_value_dependency(
    vector<TemplateValueDependency> & dependencies,
    const TemplateValueDependency & dependency)
{
  if(dependency.entity.empty() || dependency.decl_location.empty()) {
    return false;
  }
  for(size_t i = 0; i < dependencies.size(); ++i) {
    if(dependencies[i].entity == dependency.entity &&
       dependencies[i].decl_location == dependency.decl_location) {
      return false;
    }
  }
  dependencies.push_back(dependency);
  return true;
}

bool class_info_has_dependent_template_arguments(
    template_api::TemplateServices & services,
    const ClassInfo & info)
{
  if(info.dependent_instantiation) {
    return true;
  }
  if(info.type &&
     service_type_depends_on_template_parameter(services, info.type)) {
    return true;
  }
  return services.semantic_context &&
         template_api::template_arguments_are_dependent(
             *services.semantic_context,
             info.instantiation_arguments);
}

class ScopedTemplateValueDependencyLifecycleResume
{
public:
  ScopedTemplateValueDependencyLifecycleResume()
    : saved_depth_(template_api::template_witness_detail::
                       current_lifecycle_pause_depth_storage())
  {
    template_api::template_witness_detail::
        current_lifecycle_pause_depth_storage() = 0;
  }

  ~ScopedTemplateValueDependencyLifecycleResume()
  {
    template_api::template_witness_detail::
        current_lifecycle_pause_depth_storage() = saved_depth_;
  }

private:
  int saved_depth_;
};

void append_member_value_binding_dependency(
    template_api::TemplateServices & services,
    const ClassInfo & fallback_owner,
    const ValueBinding & binding,
    vector<TemplateValueDependency> & out)
{
  if(!services.semantic_context || binding.kind == ValueBinding::VK_FIELD) {
    return;
  }

  const ClassInfo * owner = binding.owner_class ? binding.owner_class : &fallback_owner;
  if(!owner || owner->qualified_name.empty()) {
    return;
  }

  const string decl_location = strip_template_location_at_prefix(
      semantic_model::source_decl_anchor_location(
          semantic_trace::value_decl_anchor(*services.semantic_context,
                                            &binding)));
  if(decl_location.empty()) {
    return;
  }

  string member_name = binding.name;
  const size_t qualifier_pos = member_name.rfind("::");
  if(qualifier_pos != string::npos) {
    member_name = member_name.substr(qualifier_pos + 2);
  }

  TemplateValueDependency dependency;
  dependency.entity =
      class_symbol_or_output_name_for_witness(*owner) + "::" + member_name;
  dependency.decl_location = decl_location;
  dependency.value_binding = &binding;
  dependency.value_owner_class = owner;
  dependency.entity_has_template_identity =
      template_api::value_or_owner_has_template_identity(&binding) ||
      template_api::class_has_template_identity(owner);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "append-member-value-dependency entity=" << dependency.entity
          << " decl=" << dependency.decl_location
          << " identity=" << (dependency.entity_has_template_identity ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  append_template_value_dependency(out, dependency);
}

void append_structured_bool_integral_constant_dependency(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    bool value,
    vector<TemplateValueDependency> & out)
{
  set<const ClassInfo *> visiting;
  const ClassInfo * integral_constant =
      structured_bool_integral_constant_info(
          service_type_system(services),
          info,
          value,
          visiting);
  if(!integral_constant || integral_constant->qualified_name.empty()) {
    return;
  }
  if(class_info_has_dependent_template_arguments(services, info) &&
     integral_constant == &info) {
    return;
  }

  string decl_location;
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(
          const_cast<ClassInfo &>(*integral_constant),
          kStructuredBoolResultMemberName);
  if(member.binding) {
    decl_location = strip_template_location_at_prefix(
        semantic_model::source_decl_anchor_location(
            semantic_trace::value_decl_anchor(*services.semantic_context,
                                              member.binding)));
  }
  if(decl_location.empty() && integral_constant->source_template) {
    decl_location = strip_template_location_at_prefix(
        semantic_model::source_decl_anchor_location(
            semantic_trace::class_template_decl_anchor(
                *services.semantic_context,
                integral_constant->source_template)));
  }
  if(decl_location.empty()) {
    return;
  }

  TemplateValueDependency dependency;
  dependency.entity =
      class_symbol_or_output_name_for_witness(*integral_constant) +
      "::" + kStructuredBoolResultMemberName;
  dependency.decl_location = decl_location;
  dependency.value_binding = member.binding;
  dependency.value_owner_class = integral_constant;
  dependency.entity_has_template_identity =
      template_api::class_has_template_identity(integral_constant);
  append_template_value_dependency(out, dependency);
}

void append_structured_bool_direct_value_dependency(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    vector<TemplateValueDependency> & out)
{
  if(!services.semantic_context ||
     info.qualified_name.empty() ||
     info.dependent_instantiation) {
    return;
  }
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    if(info.instantiation_arguments[i].source_defaulted) {
      return;
    }
  }
  bool has_nested_value_dependency = !info.template_value_dependencies.empty();
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    if(!info.instantiation_arguments[i].value_dependencies.empty()) {
      has_nested_value_dependency = true;
      break;
    }
  }
  bool all_template_parameters_are_non_type =
      info.source_template && !info.source_template->parameters.empty();
  for(size_t i = 0;
      all_template_parameters_are_non_type &&
      i < info.source_template->parameters.size();
      ++i) {
    if(info.source_template->parameters[i].kind !=
       TemplateParameterInfo::TP_NON_TYPE) {
      all_template_parameters_are_non_type = false;
    }
  }
  if(!has_nested_value_dependency && !all_template_parameters_are_non_type) {
    return;
  }
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(
          const_cast<ClassInfo &>(info),
          kStructuredBoolResultMemberName);
  if(!member.binding || member.binding->kind == ValueBinding::VK_FIELD) {
    return;
  }
  if(member.binding->owner_class && member.binding->owner_class != &info) {
    return;
  }
  if(member.binding->declaration_scope &&
     member.binding->declaration_scope->class_info &&
     member.binding->declaration_scope->class_info != &info) {
    return;
  }

  append_member_value_binding_dependency(services, info, *member.binding, out);
}

void append_concrete_structured_bool_base_value_dependencies(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    vector<TemplateValueDependency> & out,
    set<const ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return;
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    ClassInfo * base = info.bases[i].type;
    if(!base) {
      continue;
    }
    if(!class_info_has_dependent_template_arguments(services, *base) &&
       base->member_scope) {
      bool ignored_value = false;
      if(class_member_direct_bool_value(base->member_scope.get(),
                                        kStructuredBoolResultMemberName,
                                        ignored_value)) {
        semantic_lookup::MemberValueLookupResult member =
            semantic_lookup::lookup_member_value(
                *base,
                kStructuredBoolResultMemberName);
        if(member.binding &&
           member.binding->kind != ValueBinding::VK_FIELD) {
          append_member_value_binding_dependency(services,
                                                 *base,
                                                 *member.binding,
                                                 out);
        }
      }
    }
    append_concrete_structured_bool_base_value_dependencies(services,
                                                           *base,
                                                           out,
                                                           visiting);
  }
}

const ClassInfo * structured_bool_direct_value_member_info(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    set<const ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return nullptr;
  }
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(
          const_cast<ClassInfo &>(info),
          kStructuredBoolResultMemberName);
  if(member.binding &&
     member.binding->kind != ValueBinding::VK_FIELD &&
     (!member.binding->owner_class || member.binding->owner_class == &info) &&
     (!member.binding->declaration_scope ||
      !member.binding->declaration_scope->class_info ||
      member.binding->declaration_scope->class_info == &info)) {
    return &info;
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type || !info.bases[i].type->type) {
      continue;
    }
    ClassInfo * base_info =
        template_api::find_named_type_class_info(
            service_type_system(services).model,
            info.bases[i].type->type);
    if(!base_info) {
      continue;
    }
    if(const ClassInfo * found =
           structured_bool_direct_value_member_info(services,
                                                    *base_info,
                                                    visiting)) {
      return found;
    }
  }
  return nullptr;
}

void append_static_member_value_dependency_for_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    const string & member_name,
    vector<TemplateValueDependency> & out)
{
  if(!type || !services.semantic_context || member_name.empty()) {
    return;
  }
  TypePtr effective_type = type;
  if(scope.valid()) {
    TypePtr resolved_type;
    if(resolve_instantiated_dependent_type(services,
                                           scope,
                                           effective_type,
                                           resolved_type) &&
       resolved_type) {
      effective_type = resolved_type;
    }
    if(effective_type &&
       service_type_depends_on_template_parameter(services, effective_type) &&
       effective_type->kind == Type::TK_NAMED) {
      string bound_name = effective_type->named_semantic_payload;
      if(bound_name.empty()) {
        bound_name = effective_type->named_display;
      }
      bound_name = trim_space(bound_name);
      const string typename_prefix = "typename ";
      if(bound_name.compare(0, typename_prefix.size(), typename_prefix) == 0) {
        bound_name = trim_space(bound_name.substr(typename_prefix.size()));
      }
      bound_name = strip_elaborated_type_prefix(trim_space(bound_name));
      if(!bound_name.empty()) {
        TypePtr exact_bound =
            lookup_exact_bound_type_name(scope.require(), bound_name);
        if(exact_bound) {
          effective_type = exact_bound;
        }
      }
    }
  }
  if(service_type_depends_on_template_parameter(services, effective_type)) {
    return;
  }

  ClassInfo * direct_info =
      template_api::find_named_type_class_info(service_type_system(services).model,
                                               effective_type);
  ClassInfo * prepared_info = nullptr;
  if(scope.valid()) {
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    Scope * member_scope = nullptr;
    if(prepare_concrete_type_member_scope(services,
                                          scope,
                                          effective_type,
                                          member_scope) &&
       member_scope &&
       member_scope->class_info) {
      prepared_info = member_scope->class_info;
    }
  }
  ClassInfo * info = prepared_info ? prepared_info : direct_info;
  if(!info) {
    return;
  }

  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(*info, member_name);
  const ClassInfo * member_binding_owner =
      member.binding && member.binding->owner_class ? member.binding->owner_class :
      (member.binding && member.binding->declaration_scope ?
           member.binding->declaration_scope->class_info :
           nullptr);
  if(info->is_explicit_specialization ||
     (member.declared_in && member.declared_in->is_explicit_specialization) ||
     (member_binding_owner && member_binding_owner->is_explicit_specialization) ||
     (member.binding && member.binding->is_explicit_specialization)) {
    return;
  }
  const bool member_is_enumerator =
      member.binding &&
      member.binding->kind == ValueBinding::VK_VARIABLE &&
      member.binding->declaration_node &&
      member.binding->declaration_node->kind == CppAstKind::enumerator;
  if(!member.binding ||
     member.binding->kind != ValueBinding::VK_VARIABLE ||
     member_is_enumerator) {
    return;
  }
  const string decl_location = strip_template_location_at_prefix(
      semantic_model::source_decl_anchor_location(
          semantic_trace::value_decl_anchor(*services.semantic_context,
                                            member.binding)));
  if(decl_location.empty()) {
    return;
  }

  const ClassInfo * dependency_owner = member.declared_in ? member.declared_in : info;
  TemplateValueDependency dependency;
  dependency.entity =
      class_symbol_or_output_name_for_witness(*dependency_owner) + "::" +
      member.binding->name;
  dependency.decl_location = decl_location;
  dependency.value_binding = member.binding;
  dependency.value_owner_class = dependency_owner;
  dependency.entity_has_template_identity =
      template_api::value_or_owner_has_template_identity(member.binding) ||
      template_api::class_has_template_identity(dependency_owner);
  append_template_value_dependency(out, dependency);
}

void note_non_bool_static_value_dependency_for_witness(
    template_api::TemplateServices & services,
    const ValueBinding & binding)
{
  if(services.witness_context.session == nullptr ||
     !binding.type ||
     is_bool_type(strip_top_level_cv(remove_reference_type(binding.type)))) {
    return;
  }
  const bool member_is_enumerator =
      binding.kind == ValueBinding::VK_VARIABLE &&
      binding.declaration_node &&
      binding.declaration_node->kind == CppAstKind::enumerator;
  if(binding.kind != ValueBinding::VK_VARIABLE || member_is_enumerator) {
    return;
  }
  const ClassInfo * owner =
      binding.owner_class ? binding.owner_class :
      (binding.declaration_scope ? binding.declaration_scope->class_info : nullptr);
  if(!owner || owner->is_explicit_specialization || binding.is_explicit_specialization) {
    return;
  }
  vector<TemplateValueDependency> dependencies;
  append_member_value_binding_dependency(services,
                                         *owner,
                                         binding,
                                         dependencies);
  note_template_value_dependencies_for_witness(*services.semantic_context,
                                               dependencies);
}

void append_structured_bool_value_dependency_for_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    vector<TemplateValueDependency> & out)
{
  if(!type || !services.semantic_context) {
    return;
  }
  ClassInfo * direct_info =
      template_api::find_named_type_class_info(service_type_system(services).model,
                                               type);
  ClassInfo * prepared_info = nullptr;
  if(scope.valid()) {
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    Scope * member_scope = nullptr;
    if(prepare_concrete_type_member_scope(services,
                                          scope,
                                          type,
                                          member_scope) &&
       member_scope &&
       member_scope->class_info) {
      prepared_info = member_scope->class_info;
    }
  }
  ClassInfo * info = prepared_info ? prepared_info : direct_info;
  if(!info) {
    return;
  }
  set<const ClassInfo *> dependency_visiting;
  append_structured_bool_value_dependencies_for_class_info(
      services,
      *info,
      out,
      dependency_visiting);
  bool value = false;
  set<string> visiting;
  if(!structured_bool_constant_value_for_class_info(
         service_type_system(services), *info, value, visiting)) {
    const size_t dependency_start = out.size();
    set<const ClassInfo *> false_visiting;
    set<const ClassInfo *> true_visiting;
    const ClassInfo * false_constant =
        structured_bool_integral_constant_info(service_type_system(services),
                                               *info,
                                               false,
                                               false_visiting);
    const ClassInfo * true_constant =
        structured_bool_integral_constant_info(service_type_system(services),
                                               *info,
                                               true,
                                               true_visiting);
    if((false_constant != nullptr) != (true_constant != nullptr)) {
      append_structured_bool_integral_constant_dependency(
          services,
          *info,
          true_constant != nullptr,
          out);
    }
    set<const ClassInfo *> base_value_visiting;
    append_concrete_structured_bool_base_value_dependencies(services,
                                                           *info,
                                                           out,
                                                           base_value_visiting);
    if(services.witness_context.session != nullptr &&
       services.semantic_context &&
       out.size() > dependency_start) {
      vector<TemplateValueDependency> new_dependencies(out.begin() +
                                                       dependency_start,
                                                       out.end());
      note_template_value_dependencies_for_witness(*services.semantic_context,
                                                   new_dependencies);
    }
    return;
  }
  append_structured_bool_integral_constant_dependency(services, *info, value, out);
  if(class_info_has_dependent_template_arguments(services, *info)) {
    return;
  }
  append_structured_bool_direct_value_dependency(services, *info, out);
  set<const ClassInfo *> direct_value_visiting;
  if(const ClassInfo * value_info =
         structured_bool_direct_value_member_info(services,
                                                  *info,
                                                  direct_value_visiting)) {
    append_structured_bool_direct_value_dependency(services, *value_info, out);
  }
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(*info, kStructuredBoolResultMemberName);
  if(member.binding &&
     member.binding->owner_class &&
     member.binding->owner_class != info &&
     member.binding->owner_class->source_template &&
     member.binding->owner_class->source_template->name == "integral_constant" &&
     member.binding->owner_class->instantiation_arguments.size() == 1) {
    append_member_value_binding_dependency(services, *info, *member.binding, out);
  }
  append_structured_bool_integral_constant_dependency(services, *info, value, out);
}

void append_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    vector<TemplateValueDependency> & out,
    set<const ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return;
  }
  if(services.semantic_context &&
     !info.reference_members_collected &&
     !info.reference_member_collection_in_progress) {
    const witness::ScopedTemplateWitnessSourceCapturePause source_pause;
    services.semantic_context->ensure_class_reference_members(
        const_cast<ClassInfo &>(info));
  }
  if(!class_info_has_dependent_template_arguments(services, info)) {
    bool value = false;
    set<string> value_visiting;
    if(structured_bool_constant_value_for_class_info(
           service_type_system(services), info, value, value_visiting)) {
      append_structured_bool_direct_value_dependency(services, info, out);
      append_structured_bool_integral_constant_dependency(services,
                                                         info,
                                                         value,
                                                         out);
    }
  }
  for(size_t i = 0; i < info.template_value_dependencies.size(); ++i) {
    append_template_value_dependency(out, info.template_value_dependencies[i]);
  }
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    const TemplateArgument & argument = info.instantiation_arguments[i];
    for(size_t j = 0; j < argument.value_dependencies.size(); ++j) {
      append_template_value_dependency(out, argument.value_dependencies[j]);
    }
  }
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    const TemplateArgument & argument = info.instantiation_arguments[i];
    if(argument.kind != TemplateArgument::TA_TYPE || !argument.type) {
      continue;
    }
    ClassInfo * arg_info =
        template_api::find_named_type_class_info(service_type_system(services).model,
                                                 argument.type);
    if(arg_info) {
      append_structured_bool_value_dependencies_for_class_info(
          services,
          *arg_info,
          out,
          visiting);
    }
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      append_structured_bool_value_dependencies_for_class_info(
          services,
          *info.bases[i].type,
          out,
      visiting);
    }
  }
}

void append_structured_bool_value_dependencies_in_template_id_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    vector<TemplateValueDependency> & out);
bool resolve_concrete_class_template_id_syntax_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    TypePtr & out);
bool resolve_qualified_owner_prefix_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const QualifiedName & qualified,
    size_t qualifier_index,
    TypePtr & out);

bool template_argument_syntax_has_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax);

bool template_id_syntax_has_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_has_template_dependency(
           services, scope, syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(alias_argument_text_mentions_template_dependency(services,
                                                       scope,
                                                       syntax.arguments[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_syntax_has_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.dependent || syntax.pack_expansion) {
    return true;
  }
  if(syntax.resolved_type &&
     service_type_depends_on_template_parameter(services, syntax.resolved_type)) {
    return true;
  }
  const string text =
      !syntax.text.empty() ? syntax.text : syntax.source_text;
  if(alias_argument_text_mentions_template_dependency(services, scope, text)) {
    return true;
  }
  return syntax.template_id &&
         template_id_syntax_has_template_dependency(services,
                                                    scope,
                                                    *syntax.template_id);
}

void append_non_bool_static_value_dependencies_for_qualified_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(!qualified ||
     !is_structured_bool_result_member_name(qualified->name) ||
     qualified->qualifiers.empty()) {
    return;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "structured-bool-qualified-value name="
          << qualified_name_text_for_structured_lookup(*qualified)
          << " scope=" << (scope.valid() ? scope.require().name : std::string("<invalid>"));
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  for(size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    TypePtr owner_type;
    if(const TemplateIdSyntax * qualifier_template_id =
           cppast_qualifier_template_id_syntax(node, qualifier_index)) {
      if((resolve_template_id_syntax_type(services,
                                          scope.require(),
                                          *qualifier_template_id,
                                          false,
                                          string(),
                                          owner_type,
                                          scope) &&
          owner_type) ||
         (resolve_concrete_class_template_id_syntax_type(
              services,
              scope,
              *qualifier_template_id,
              owner_type) &&
          owner_type)) {
        append_static_member_value_dependency_for_type(services,
                                                       scope,
                                                       owner_type,
                                                       qualified->name,
                                                       out);
      }
      return;
    }
    if(const CppAstNode * qualifier_type =
           cppast_qualifier_type_syntax(node, qualifier_index)) {
      if(parse_type_id_node_for_templates(services,
                                          scope.require(),
                                          *qualifier_type,
                                          owner_type,
                                          true) &&
         owner_type) {
        if(!service_type_depends_on_template_parameter(services, owner_type)) {
          append_static_member_value_dependency_for_type(services,
                                                         scope,
                                                         owner_type,
                                                         qualified->name,
                                                         out);
          return;
        }
      }
    }
    if(resolve_qualified_owner_prefix_type(services,
                                           scope,
                                           *qualified,
                                           qualifier_index,
                                           owner_type) &&
       owner_type) {
      append_static_member_value_dependency_for_type(services,
                                                     scope,
                                                     owner_type,
                                                     qualified->name,
                                                     out);
    }
    return;
  }
}

void append_non_bool_static_value_dependencies_for_unqualified_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  if(node.kind != CppAstKind::id_expression ||
     node.value.empty() ||
     !scope.valid()) {
    return;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "non-bool-static-unqualified-check name=" << node.value
          << " scope=" << scope.require().name;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  const ValueBinding * binding = nullptr;
  if(!lookup_leaf_value_binding(scope.require(), node.value, binding) ||
     !binding ||
     binding->kind != ValueBinding::VK_VARIABLE) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "non-bool-static-unqualified-miss name=" << node.value;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return;
  }
  const bool member_is_enumerator =
      binding->declaration_node &&
      binding->declaration_node->kind == CppAstKind::enumerator;
  if(member_is_enumerator ||
     is_bool_type(strip_top_level_cv(remove_reference_type(binding->type)))) {
    return;
  }
  const ClassInfo * owner =
      binding->owner_class ? binding->owner_class :
      (binding->declaration_scope ? binding->declaration_scope->class_info :
                                    scope.require().class_info);
  if(!owner || owner->dependent_instantiation) {
    return;
  }
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "non-bool-static-unqualified-hit name=" << node.value
          << " binding=" << binding->name
          << " owner=" << owner->qualified_name;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  append_member_value_binding_dependency(services, *owner, *binding, out);
}

void append_non_bool_static_value_dependencies_in_expression_ast_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out);

void append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & template_id,
    vector<TemplateValueDependency> & out);

void append_non_bool_static_value_dependencies_in_expression_ast_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }
  append_non_bool_static_value_dependencies_for_unqualified_value(
      services, scope, node, out);
  append_non_bool_static_value_dependencies_for_qualified_value(
      services, scope, node, out);
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
        services, scope, *template_id, out);
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
        services, scope, node.qualifier_template_id_syntaxes[i], out);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    append_non_bool_static_value_dependencies_in_expression_ast_impl(
        services, scope, node.children[i], out);
  }
}

void append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & template_id,
    vector<TemplateValueDependency> & out)
{
  for(size_t i = 0; i < template_id.argument_syntaxes.size(); ++i) {
    const TemplateArgumentSyntax & argument = template_id.argument_syntaxes[i];
    if(argument.expression) {
      append_non_bool_static_value_dependencies_in_expression_ast_impl(
          services, scope, *argument.expression, out);
    }
    if(argument.type_id) {
      append_non_bool_static_value_dependencies_in_expression_ast_impl(
          services, scope, *argument.type_id, out);
    }
    if(argument.template_id) {
      append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
          services, scope, *argument.template_id, out);
    }
  }
}

void append_non_bool_static_value_dependencies_in_template_argument_syntax_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    const TypePtr & bound_value_type,
    vector<TemplateValueDependency> & out)
{
  TypePtr stripped = strip_top_level_cv(remove_reference_type(bound_value_type));
  if(!stripped || is_bool_type(stripped)) {
    return;
  }
  if(syntax.expression) {
    append_non_bool_static_value_dependencies_in_expression_ast_impl(
        services, scope, *syntax.expression, out);
  }
  if(syntax.type_id) {
    append_non_bool_static_value_dependencies_in_expression_ast_impl(
        services, scope, *syntax.type_id, out);
  }
  if(syntax.template_id) {
    append_non_bool_static_value_dependencies_in_template_id_syntax_impl(
        services, scope, *syntax.template_id, out);
  }
}

bool resolve_concrete_class_template_id_syntax_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    TypePtr & out)
{
  out.reset();
  if(!services.semantic_context) {
    return false;
  }
  ClassTemplateDecl * class_template =
      lookup_class_template(services,
                            scope.require(),
                            qualified_name_text_for_structured_lookup(
                                syntax.name));
  if(!class_template) {
    return false;
  }
  vector<TemplateArgument> arguments;
  if(!template_api::resolve_template_arguments(
         *services.semantic_context,
         scope.require(),
         class_template->parameters,
         syntax.arguments,
         &syntax.argument_syntaxes,
         arguments,
         class_template->declaring_scope) ||
     template_api::template_arguments_are_dependent(*services.semantic_context,
                                                    arguments)) {
    return false;
  }
  const string key =
      template_api::template_argument_identity_key(*services.semantic_context,
                                                   arguments);
  const template_api::specialization::ClassSpecializationSelection selection =
      template_api::specialization::select_class_specialization(
          *services.semantic_context,
          *class_template,
          scope.require(),
          key,
          arguments);
  ClassInfo * info =
      services.semantic_context->reference_selected_class_template_instantiation(
          *class_template,
          scope.require(),
          arguments,
          selection,
          nullptr,
          template_api::ClassTemplateSourceUseMode::SemanticLookupOnly,
          nullptr,
          &key);
  if(!info || !info->type) {
    return false;
  }
  out = info->type;
  return true;
}

void append_structured_bool_value_dependencies_in_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    vector<TemplateValueDependency> & out)
{
  if(syntax.expression) {
    append_structured_bool_value_dependencies_in_expression_ast(
        services, scope, *syntax.expression, out);
  }
  if(syntax.type_id) {
    append_structured_bool_value_dependencies_in_expression_ast(
        services, scope, *syntax.type_id, out);
  }
  if(syntax.template_id) {
    append_structured_bool_value_dependencies_in_template_id_syntax(
        services, scope, *syntax.template_id, out);
  }
}

void append_structured_bool_value_dependencies_in_template_id_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    vector<TemplateValueDependency> & out)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    append_structured_bool_value_dependencies_in_argument_syntax(
        services, scope, syntax.argument_syntaxes[i], out);
  }
}

bool resolve_qualified_owner_prefix_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const QualifiedName & qualified,
    size_t qualifier_index,
    TypePtr & out)
{
  out.reset();
  if(qualifier_index >= qualified.qualifiers.size()) {
    return false;
  }
  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope.require();
  request.allow_class_templates = false;
  request.name.rooted = qualified.rooted;
  request.name.qualifiers.assign(qualified.qualifiers.begin(),
                                 qualified.qualifiers.begin() + qualifier_index);
  request.name.name = qualified.qualifiers[qualifier_index];
  return service_resolve_direct_type_lookup(services, request, out) && out;
}

void append_structured_bool_value_dependencies_for_qualified_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  if(!qualified ||
     !is_structured_bool_result_member_name(qualified->name) ||
     qualified->qualifiers.empty()) {
    return;
  }
  for(size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    if(const TemplateIdSyntax * qualifier_template_id =
           cppast_qualifier_template_id_syntax(node, qualifier_index)) {
      TypePtr owner_type;
      const size_t before = out.size();
      const bool source_template_id_dependent =
          template_id_syntax_has_template_dependency(services,
                                                     scope,
                                                     *qualifier_template_id);
      if(resolve_template_id_syntax_type(services,
                                         scope.require(),
                                         *qualifier_template_id,
                                         false,
                                         string(),
                                         owner_type,
                                         scope,
                                         template_api::ClassTemplateSourceUseMode::
                                             SemanticLookupOnly) &&
         owner_type) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             std::string(),
                             "structured-bool-qualified template-id owner resolved");
        }
        if(!source_template_id_dependent) {
          append_structured_bool_value_dependency_for_type(services,
                                                           scope,
                                                           owner_type,
                                                           out);
        }
      }
      if(out.size() == before &&
         !source_template_id_dependent &&
         resolve_concrete_class_template_id_syntax_type(
             services,
             scope,
             *qualifier_template_id,
             owner_type) &&
         owner_type) {
        append_structured_bool_value_dependency_for_type(services,
                                                         scope,
                                                         owner_type,
                                                         out);
      }
      return;
    }
    if(const CppAstNode * qualifier_type =
           cppast_qualifier_type_syntax(node, qualifier_index)) {
      TypePtr owner_type;
      if(parse_type_id_node_for_templates(services,
                                          scope.require(),
                                          *qualifier_type,
                                          owner_type,
                                          true) &&
         owner_type) {
        if(parser_trace::enabled("template.resolve")) {
          parser_trace::note("template.resolve",
                             std::string(),
                             "structured-bool-qualified type owner resolved");
        }
        append_structured_bool_value_dependency_for_type(services,
                                                         scope,
                                                         owner_type,
                                                         out);
      }
      return;
    }
    {
      TypePtr owner_type;
      if(resolve_qualified_owner_prefix_type(services,
                                             scope,
                                             *qualified,
                                             qualifier_index,
                                             owner_type) &&
         owner_type) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "structured-bool-qualified prefix owner resolved type="
                << describe_type(owner_type);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        append_structured_bool_value_dependency_for_type(services,
                                                         scope,
                                                         owner_type,
                                                         out);
      }
    }
    return;
  }
}

void append_structured_bool_value_dependencies_in_expression_ast_impl(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }

  if(node.kind == CppAstKind::binary_expression &&
     node.children.size() == 2 &&
     node.has_token &&
     (node.simple_type == OP_LAND || node.simple_type == OP_LOR)) {
    append_structured_bool_value_dependencies_in_expression_ast_impl(
        services, scope, node.children[0], out);

    bool lhs = false;
    bool lhs_evaluated = false;
    try {
      lhs_evaluated =
          evaluate_structured_bool_expression(
              services, scope, node.children[0], lhs) == NT_ARG_EVALUATED;
    } catch(const TemplateSubstitutionFailure &) {
    } catch(const SemanticSoftFailure &) {
    } catch(const SemanticDiagnosticError &) {
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    } catch(const logic_error &) {
    }
    if(lhs_evaluated &&
       ((node.simple_type == OP_LAND && !lhs) ||
        (node.simple_type == OP_LOR && lhs))) {
      if(!scope_has_template_placeholders(services, scope)) {
        append_structured_bool_value_dependencies_in_expression_ast_impl(
            services, scope, node.children[1], out);
      }
      return;
    }

    append_structured_bool_value_dependencies_in_expression_ast_impl(
        services, scope, node.children[1], out);
    return;
  }

  try {
    append_structured_bool_value_dependencies_for_qualified_value(
        services, scope, node, out);
    if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
      append_structured_bool_value_dependencies_in_template_id_syntax(
          services, scope, *template_id, out);
    }
    for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
      if(const TemplateIdSyntax * qualifier_template_id =
             cppast_qualifier_template_id_syntax(node, i)) {
        append_structured_bool_value_dependencies_in_template_id_syntax(
            services, scope, *qualifier_template_id, out);
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      append_structured_bool_value_dependencies_in_expression_ast_impl(
          services, scope, node.children[i], out);
    }
  } catch(const TemplateSubstitutionFailure &) {
  } catch(const SemanticSoftFailure &) {
  } catch(const SemanticDiagnosticError &) {
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
  } catch(const logic_error &) {
  }
}

void note_template_value_dependency_for_witness(
    const TemplateValueDependency & dependency,
    SemanticContext * ctx = nullptr)
{
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "note-template-value-dependency entity=" << dependency.entity
          << " decl=" << dependency.decl_location
          << " identity="
          << (dependency.entity_has_template_identity ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(dependency.entity.empty() || dependency.decl_location.empty()) {
    return;
  }
  if(template_api::template_witness_detail::
         current_lifecycle_pause_depth_storage() != 0) {
    return;
  }
  const witness::ScopedTemplateWitnessEntryContext entry_context(
      witness::make_template_closure_entry_context(
          witness::TemplateClosureReason::TrackInstantiation,
          dependency.entity,
          dependency.decl_location,
          dependency.entity_has_template_identity));
  const ScopedTemplateValueDependencyLifecycleResume lifecycle_resume;
  witness::note_template_witness_log_event(
      witness::TemplateWitnessLogEventKind::VariableInstantiation,
      dependency.decl_location,
      dependency.entity,
      dependency.decl_location,
      string(),
      witness::TemplateLifecycleCause::TrackInstantiation,
      dependency.entity_has_template_identity);
}

void note_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const ClassInfo & info,
    set<const ClassInfo *> & visiting)
{
  if(!visiting.insert(&info).second) {
    return;
  }
  for(size_t i = 0; i < info.template_value_dependencies.size(); ++i) {
    note_template_value_dependency_for_witness(
        info.template_value_dependencies[i],
        services.semantic_context);
  }
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    for(size_t j = 0; j < info.instantiation_arguments[i].value_dependencies.size(); ++j) {
      note_template_value_dependency_for_witness(
          info.instantiation_arguments[i].value_dependencies[j],
          services.semantic_context);
    }
  }
  for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    if(info.instantiation_arguments[i].kind != TemplateArgument::TA_TYPE ||
       !info.instantiation_arguments[i].type) {
      continue;
    }
    ClassInfo * arg_info =
        template_api::find_named_type_class_info(service_type_system(services).model,
                                                 info.instantiation_arguments[i].type);
    if(arg_info) {
      note_structured_bool_value_dependencies_for_class_info(
          services,
          *arg_info,
          visiting);
    }
  }
  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type) {
      note_structured_bool_value_dependencies_for_class_info(
          services,
          *info.bases[i].type,
          visiting);
    }
  }
}

void note_structured_bool_value_member_if_needed(
    template_api::TemplateServices & services,
    const ClassInfo & info)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr ||
     !info.member_scope) {
    return;
  }
  bool ignored_value = false;
  set<string> visiting;
  if(!structured_bool_constant_value_for_class_info(
         service_type_system(services), info, ignored_value, visiting)) {
    return;
  }
  note_structured_bool_integral_constant_value_for_witness(services,
                                                          info,
                                                          ignored_value);
  vector<TemplateValueDependency> dependencies;
  set<const ClassInfo *> dependency_visiting;
  append_structured_bool_value_dependencies_for_class_info(
      services,
      info,
      dependencies,
      dependency_visiting);
  append_structured_bool_integral_constant_dependency(services,
                                                      info,
                                                      ignored_value,
                                                      dependencies);
  for(size_t i = 0; i < dependencies.size(); ++i) {
    note_template_value_dependency_for_witness(dependencies[i],
                                              services.semantic_context);
  }
  semantic_lookup::MemberValueLookupResult member =
      semantic_lookup::lookup_member_value(
          const_cast<ClassInfo &>(info),
          kStructuredBoolResultMemberName);
  if(!member.binding || member.binding->kind == ValueBinding::VK_FIELD) {
    return;
  }
  template_api::note_template_member_value_instantiation_if_needed(
      *services.semantic_context,
      *member.binding);
}

void note_structured_bool_value_member_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr ||
     !type) {
    return;
  }
  ClassInfo * direct_info =
      template_api::find_named_type_class_info(service_type_system(services).model, type);
  if(direct_info) {
    note_structured_bool_value_member_if_needed(services, *direct_info);
  }
  ClassInfo * prepared_info = nullptr;
  {
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    Scope * member_scope = nullptr;
    if(prepare_concrete_type_member_scope(services, scope, type, member_scope) &&
       member_scope &&
       member_scope->class_info &&
       member_scope->class_info != direct_info) {
      prepared_info = member_scope->class_info;
    }
  }
  if(prepared_info) {
    note_structured_bool_value_member_if_needed(services, *prepared_info);
    return;
  }
}

void note_structured_bool_value_member_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }
  TypePtr owner_type;
  {
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    resolve_template_id_syntax_type(services,
                                    scope.require(),
                                    syntax,
                                    false,
                                    string(),
                                    owner_type,
                                    scope);
  }
  if(owner_type &&
     !service_type_depends_on_template_parameter(services, owner_type)) {
    note_structured_bool_value_member_if_needed(services, scope, owner_type);
  }
}

bool structured_bool_constant_value_for_argument(
    template_api::TemplateTypeSystem & type_system,
    const TemplateArgument & argument,
    bool & out,
    set<string> & visiting,
    template_api::TemplateServices * services = nullptr,
    template_api::TemplateEnvironmentHandle scope =
        template_api::TemplateEnvironmentHandle(),
    bool * evaluation_incomplete = nullptr)
{
  if(argument.kind == TemplateArgument::TA_VALUE) {
    out = argument.value != 0;
    return true;
  }
  if(argument.kind == TemplateArgument::TA_TYPE &&
     structured_bool_constant_value_for_type(
         type_system,
         argument.type,
         out,
         visiting,
         services,
         scope,
         evaluation_incomplete)) {
    return true;
  }
  return false;
}

bool function_binding_has_unconditional_nothrow(const FunctionBinding & binding)
{
  if(binding.explicit_function_nothrow_kind == EFNK_ALWAYS_TRUE) {
    return true;
  }
  if(binding.explicit_function_nothrow_eval_cached) {
    return binding.explicit_function_nothrow_eval_value;
  }
  if(!binding.function_qualifier) {
    return false;
  }
  const string text = trim_space(binding.function_qualifier->value);
  return text == "noexcept" || text == "throw()" || text == "noexcept(true)";
}

bool class_info_source_template_is_std_namespace_or_inline_child(const ClassInfo & info)
{
  if(!info.source_template ||
     !info.source_template->comes_from_standard_include_path) {
    return false;
  }
  const Scope * current =
      info.source_template->declaring_scope;
  while(current && current->namespace_scope && current->inline_namespace) {
    current = current->parent;
  }
  if(!current ||
     !current->namespace_scope ||
     current->name != "std") {
    return false;
  }
  for(const Scope * parent = current->parent; parent; parent = parent->parent) {
    if(parent->namespace_scope &&
       parent->name != "<global>" &&
       parent->name != "<unnamed>") {
      return false;
    }
  }
  return true;
}

semantic_conversion::ExprInfo make_declval_trait_expr_info(const TypePtr & source)
{
  semantic_conversion::ExprInfo expr;
  if(!source) {
    return expr;
  }
  TypePtr result_type = is_void_type(strip_top_level_cv(source)) ?
      source :
      make_rvalue_reference_raw(source);
  expr.type = semantic_conversion::expression_type_for_function_result(result_type);
  semantic_conversion::result_value_category_for_function_result(result_type,
                                                                 expr.category);
  return expr;
}

bool function_type_structured_invocation_result(
    const TypePtr & callable_type,
    const vector<semantic_conversion::ExprInfo> & arg_exprs,
    TypePtr & result_type)
{
  result_type.reset();
  TypePtr function_type;
  if(!resolve_callable_function_type(callable_type, function_type)) {
    return false;
  }
  TypePtr stripped = strip_top_level_cv(function_type);
  if(!stripped ||
     stripped->kind != Type::TK_FUNCTION ||
     !stripped->inner ||
     stripped->prototype_relaxed) {
    return false;
  }
  if(stripped->variadic) {
    if(arg_exprs.size() < stripped->params.size()) {
      return false;
    }
  } else if(arg_exprs.size() != stripped->params.size()) {
    return false;
  }
  for(size_t i = 0; i < stripped->params.size(); ++i) {
    if(semantic_conversion::standard_conversion_rank(stripped->params[i],
                                                     arg_exprs[i]) ==
       semantic_conversion::CR_BAD) {
      return false;
    }
  }
  result_type = stripped->inner;
  return true;
}

bool function_result_convertible_to_invocable_r(const TypePtr & result_type,
                                                const TypePtr & required_return)
{
  TypePtr required_base = strip_top_level_cv(required_return);
  if(!required_base) {
    return false;
  }
  if(is_void_type(required_base)) {
    return true;
  }
  if(!result_type) {
    return false;
  }
  semantic_conversion::ExprInfo result_expr;
  result_expr.type =
      semantic_conversion::expression_type_for_function_result(result_type);
  if(!semantic_conversion::result_value_category_for_function_result(
         result_type,
         result_expr.category)) {
    return false;
  }
  return semantic_conversion::standard_conversion_rank(required_return,
                                                       result_expr) !=
         semantic_conversion::CR_BAD;
}

bool class_member_collection_in_progress(const ClassInfo & info)
{
  return info.template_instantiation_in_progress ||
         info.full_member_collection_in_progress ||
         info.reference_member_collection_in_progress;
}

bool callable_object_structured_invocation_result(
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateServices * services,
    template_api::TemplateEnvironmentHandle scope,
    const semantic_conversion::ExprInfo & callable_expr,
    const vector<semantic_conversion::ExprInfo> & arg_exprs,
    TypePtr & result_type,
    FunctionBinding *& selected_binding,
    bool & lookup_complete)
{
  result_type.reset();
  selected_binding = nullptr;
  lookup_complete = true;
  TypePtr callable_object =
      strip_top_level_cv(remove_reference_type(callable_expr.type));
  if(!callable_object || callable_object->kind != Type::TK_NAMED) {
    return false;
  }

  ClassInfo * callable_info =
      template_api::find_named_type_class_info(type_system.model, callable_object);
  if((!callable_info || !callable_info->member_scope) &&
     services &&
     scope.valid()) {
    Scope * member_scope = nullptr;
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    if(prepare_concrete_type_member_scope(*services,
                                          scope,
                                          callable_object,
                                          member_scope) &&
       member_scope &&
       member_scope->class_info) {
      callable_info = member_scope->class_info;
    }
  }
  if(!callable_info) {
    return false;
  }

  semantic_lookup::MemberFunctionLookupResult lookup =
      semantic_lookup::lookup_member_functions(*callable_info, "operator()");
  if(lookup.functions.empty() &&
     services &&
     scope.valid()) {
    Scope * member_scope = nullptr;
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    if(service_type_system(*services).complete_named_type_member_scope(
           scope,
           callable_object,
           member_scope) &&
       member_scope &&
       member_scope->class_info) {
      callable_info = member_scope->class_info;
      lookup = semantic_lookup::lookup_member_functions(*callable_info, "operator()");
    }
  }
  if(lookup.functions.empty()) {
    lookup_complete =
        callable_info->complete &&
        !class_member_collection_in_progress(*callable_info);
    return false;
  }

  bool saw_viable = false;
  for(size_t i = 0; i < lookup.functions.size(); ++i) {
    FunctionBinding * binding = lookup.functions[i];
    if(!binding || binding->is_deleted) {
      continue;
    }
    const size_t explicit_offset =
        binding->is_method &&
                !binding->params.empty() &&
                binding->params[0].first == "this" ?
            1 :
            0;
    if(binding->params.size() != arg_exprs.size() + explicit_offset) {
      continue;
    }
    if(semantic_conversion::is_const_object_type(callable_expr.type) &&
       !binding->is_const_method) {
      continue;
    }
    if(binding->ref_qualifier == RQ_LVALUE &&
       callable_expr.category != semantic_conversion::VC_LVALUE) {
      continue;
    }
    if(binding->ref_qualifier == RQ_RVALUE &&
       callable_expr.category == semantic_conversion::VC_LVALUE) {
      continue;
    }

    bool viable = true;
    for(size_t arg = 0; arg < arg_exprs.size(); ++arg) {
      const TypePtr & param = binding->params[explicit_offset + arg].second;
      if(semantic_conversion::standard_conversion_rank(param, arg_exprs[arg]) ==
         semantic_conversion::CR_BAD) {
        viable = false;
        break;
      }
    }
    if(!viable) {
      continue;
    }
    TypePtr binding_type = strip_top_level_cv(binding->type);
    if(!binding_type ||
       binding_type->kind != Type::TK_FUNCTION ||
       !binding_type->inner) {
      continue;
    }
    if(saw_viable) {
      continue;
    }
    saw_viable = true;
    selected_binding = binding;
    result_type = binding_type->inner;
  }
  return saw_viable && result_type;
}

bool evaluate_structured_invocable_r_trait(
    template_api::TemplateTypeSystem & type_system,
    const vector<TemplateArgument> & arguments,
    bool require_nothrow,
    bool & out,
    template_api::TemplateServices * services = nullptr,
    template_api::TemplateEnvironmentHandle scope =
        template_api::TemplateEnvironmentHandle(),
    bool * evaluation_incomplete = nullptr)
{
  out = false;
  if(arguments.size() < 2 ||
     arguments[0].kind != TemplateArgument::TA_TYPE ||
     !arguments[0].type ||
     arguments[1].kind != TemplateArgument::TA_TYPE ||
     !arguments[1].type) {
    return false;
  }

  vector<semantic_conversion::ExprInfo> arg_exprs;
  arg_exprs.reserve(arguments.size() - 2);
  for(size_t i = 2; i < arguments.size(); ++i) {
    if(arguments[i].kind != TemplateArgument::TA_TYPE || !arguments[i].type) {
      return false;
    }
    arg_exprs.push_back(make_declval_trait_expr_info(arguments[i].type));
    if(!arg_exprs.back().type) {
      return false;
    }
  }

  const semantic_conversion::ExprInfo callable_expr =
      make_declval_trait_expr_info(arguments[1].type);
  if(!callable_expr.type) {
    return false;
  }

  TypePtr result_type;
  FunctionBinding * selected_binding = nullptr;
  const bool function_type_invocable =
      function_type_structured_invocation_result(callable_expr.type,
                                                 arg_exprs,
                                                 result_type);
  bool callable_lookup_complete = true;
  const bool callable_object_invocable =
      !function_type_invocable &&
      callable_object_structured_invocation_result(type_system,
                                                   services,
                                                   scope,
                                                   callable_expr,
                                                   arg_exprs,
                                                   result_type,
                                                   selected_binding,
                                                   callable_lookup_complete);
  if(!function_type_invocable && !callable_object_invocable) {
    if(!callable_lookup_complete) {
      if(evaluation_incomplete) {
        *evaluation_incomplete = true;
      }
      return false;
    }
    return true;
  }

  if(!function_result_convertible_to_invocable_r(result_type, arguments[0].type)) {
    out = false;
    return true;
  }

  out = !require_nothrow ||
        (selected_binding &&
         function_binding_has_unconditional_nothrow(*selected_binding));
  return true;
}

bool evaluate_structured_nothrow_invocable_trait(
    template_api::TemplateTypeSystem & type_system,
    const vector<TemplateArgument> & arguments,
    bool & out,
    template_api::TemplateServices * services = nullptr,
    template_api::TemplateEnvironmentHandle scope =
        template_api::TemplateEnvironmentHandle(),
    bool * evaluation_incomplete = nullptr)
{
  vector<TemplateArgument> invocable_r_arguments;
  invocable_r_arguments.reserve(arguments.size() + 1);
  TemplateArgument void_argument;
  void_argument.kind = TemplateArgument::TA_TYPE;
  void_argument.type = make_fundamental(FT_VOID);
  void_argument.text = "void";
  invocable_r_arguments.push_back(void_argument);
  invocable_r_arguments.insert(invocable_r_arguments.end(),
                               arguments.begin(),
                               arguments.end());
  return evaluate_structured_invocable_r_trait(type_system,
                                               invocable_r_arguments,
                                               true,
                                               out,
                                               services,
                                               scope,
                                               evaluation_incomplete);
}

bool structured_bool_constant_value_for_class_info(
    template_api::TemplateTypeSystem & type_system,
    const ClassInfo & info,
    bool & out,
    set<string> & visiting,
    template_api::TemplateServices * services,
    template_api::TemplateEnvironmentHandle scope,
    bool * evaluation_incomplete)
{
  const string template_name =
      info.source_template ? info.source_template->name : string();
  if(template_name == "integral_constant" &&
     info.instantiation_arguments.size() == 2 &&
     info.instantiation_arguments[0].kind == TemplateArgument::TA_TYPE &&
     is_bool_type(info.instantiation_arguments[0].type) &&
     info.instantiation_arguments[1].kind == TemplateArgument::TA_VALUE) {
    out = info.instantiation_arguments[1].value != 0;
    return true;
  }

  if(template_name == "__is_nothrow_invocable") {
    return evaluate_structured_nothrow_invocable_trait(
        type_system,
        info.instantiation_arguments,
        out,
        services,
        scope,
        evaluation_incomplete);
  }

  if((template_name == "__is_invocable_r" ||
      template_name == "is_invocable_r") &&
     class_info_source_template_is_std_namespace_or_inline_child(info)) {
    return evaluate_structured_invocable_r_trait(
        type_system,
        info.instantiation_arguments,
        false,
        out,
        services,
        scope,
        evaluation_incomplete);
  }

  if((template_name == "bool_constant" ||
      template_name == "__bool_constant") &&
     info.instantiation_arguments.size() == 1 &&
     info.instantiation_arguments[0].kind == TemplateArgument::TA_VALUE) {
    out = info.instantiation_arguments[0].value != 0;
    return true;
  }

  const char * const conjunction_names[] = {
    "_And",
    "conjunction",
    "__all",
    "__and_"
  };
  if(simple_name_is_one_of(template_name,
                           conjunction_names,
                           sizeof(conjunction_names) / sizeof(conjunction_names[0]))) {
    for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
      bool value = false;
      if(!structured_bool_constant_value_for_argument(
             type_system,
             info.instantiation_arguments[i],
             value,
             visiting,
             services,
             scope,
             evaluation_incomplete)) {
        return false;
      }
      if(!value) {
        out = false;
        return true;
      }
    }
    out = true;
    return true;
  }

  const char * const disjunction_names[] = {
    "_Or",
    "disjunction",
    "__or_"
  };
  if(simple_name_is_one_of(template_name,
                           disjunction_names,
                           sizeof(disjunction_names) / sizeof(disjunction_names[0]))) {
    for(size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
      bool value = false;
      if(!structured_bool_constant_value_for_argument(
             type_system,
             info.instantiation_arguments[i],
             value,
             visiting,
             services,
             scope,
             evaluation_incomplete)) {
        return false;
      }
      if(value) {
        out = true;
        return true;
      }
    }
    out = false;
    return true;
  }

  const char * const negation_names[] = {
    "_Not",
    "negation",
    "__not_"
  };
  if(simple_name_is_one_of(template_name,
                           negation_names,
                           sizeof(negation_names) / sizeof(negation_names[0]))) {
    if(info.instantiation_arguments.size() != 1) {
      return false;
    }
    bool value = false;
    if(!structured_bool_constant_value_for_argument(
           type_system,
           info.instantiation_arguments[0],
           value,
           visiting,
           services,
           scope,
           evaluation_incomplete)) {
      return false;
    }
    out = !value;
    return true;
  }

  if(class_member_direct_bool_value(info.member_scope.get(), "value", out)) {
    return true;
  }

  for(size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].type &&
       structured_bool_constant_value_for_type(
           type_system,
           info.bases[i].type->type,
           out,
           visiting,
           services,
           scope,
           evaluation_incomplete)) {
      return true;
    }
  }
  return false;
}

bool structured_bool_constant_value_for_type(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    bool & out,
    set<string> & visiting,
    template_api::TemplateServices * services,
    template_api::TemplateEnvironmentHandle scope,
    bool * evaluation_incomplete)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }

  const string visit_key = base->named_key.empty() ?
      describe_type(base) :
      base->named_key;
  if(!visiting.insert(visit_key).second) {
    return false;
  }
  const auto finish =
      [&](bool result) -> bool
      {
        visiting.erase(visit_key);
        return result;
      };

  ClassInfo * info =
      template_api::find_named_type_class_info(type_system.model, base);
  if((!info || !info->member_scope) &&
     services &&
     scope.valid()) {
    Scope * member_scope = nullptr;
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    if(prepare_concrete_type_member_scope(*services,
                                          scope,
                                          base,
                                          member_scope) &&
       member_scope &&
       member_scope->class_info) {
      info = member_scope->class_info;
    }
  }
  if(!info) {
    return finish(false);
  }
  return finish(structured_bool_constant_value_for_class_info(
      type_system,
      *info,
      out,
      visiting,
      services,
      scope,
      evaluation_incomplete));
}

bool lookup_leaf_function_bindings(Scope & scope,
                                   const string & name,
                                   vector<FunctionBinding *> & out)
{
  out.clear();
  for(Scope * current = &scope; current != nullptr; current = current->parent) {
    unordered_set<const Scope *> visited_scopes;
    unordered_set<const FunctionBinding *> seen_bindings;
    vector<FunctionBinding *> current_out;
    collect_leaf_function_bindings_in_namespace_scope(
        *current, name, visited_scopes, seen_bindings, current_out);
    if(!current_out.empty()) {
      out.swap(current_out);
      return true;
    }
  }
  return false;
}

const QualifiedName * qualified_syntax_if_qualified(const CppAstNode & node)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(node);
  return qualified && (qualified->rooted || !qualified->qualifiers.empty()) ?
      qualified :
      nullptr;
}

bool lookup_leaf_constant_value(Scope & scope,
                                template_api::TemplateServices & services,
                                const string & name,
                                const CppAstNode * node,
                                constant_eval::ConstexprValue & out)
{
  const ValueBinding * binding = nullptr;
  if(!lookup_leaf_value_binding(scope, name, binding)) {
    const QualifiedName * qualified_node =
        node && node->kind == CppAstKind::id_expression ?
            qualified_syntax_if_qualified(*node) :
            nullptr;
    if(qualified_node) {
      if(lookup_leaf_qualified_value_binding(
             services, scope, *qualified_node, node, binding)) {
        // handled below
      } else {
        return false;
      }
    }
    if(!binding &&
       !lookup_leaf_qualified_value_binding(services, scope, name, binding)) {
      return false;
    }
  }
  if(!binding) {
    return false;
  }
  if(binding->has_constant_value) {
    if(services.semantic_context) {
      template_api::note_template_member_value_instantiation_if_needed(
          *services.semantic_context,
          *binding);
    }
    out = constant_eval::make_integral_value(
        binding->constant_value,
        binding->type ? binding->type : make_fundamental(FT_INT));
    return true;
  }
  if(binding->has_constexpr_value) {
    if(services.semantic_context) {
      template_api::note_template_member_value_instantiation_if_needed(
          *services.semantic_context,
          *binding);
    }
    out = binding->constexpr_value;
    return out.kind != constant_eval::ConstexprValue::CV_INVALID;
  }
  if(materialize_leaf_member_constant_binding(
         services, *const_cast<ValueBinding *>(binding), out)) {
    return true;
  }
  return false;
}

bool lookup_leaf_constant_value(Scope & scope,
                                template_api::TemplateServices & services,
                                const string & name,
                                constant_eval::ConstexprValue & out)
{
  return lookup_leaf_constant_value(scope, services, name, nullptr, out);
}

bool prepare_concrete_type_member_scope(template_api::TemplateServices & services,
                                        template_api::TemplateEnvironmentHandle scope,
                                        const TypePtr & type,
                                        Scope *& out)
{
  out = nullptr;
  if(!type) {
    return false;
  }
  if(service_prepare_named_type_member_scope(services, scope, type, out) && out) {
    return true;
  }

  ClassInfo * info = template_api::find_named_type_class_info(
      service_type_system(services).model,
      type);
  if(!info || !info->member_scope) {
    return false;
  }
  out = info->member_scope.get();
  return true;
}

TypePtr lookup_concrete_type_in_resolved_scope(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle lexical_scope,
    Scope & resolved_scope,
    const string & name)
{
  const string normalized_name = normalize_type_lookup_name(trim_space(name));
  if(normalized_name.empty() ||
     normalized_name.find('<') != string::npos) {
    return TypePtr();
  }

  TypePtr direct;
  if(resolved_scope.class_info) {
    if(!services.semantic_context) {
      return TypePtr();
    }
    semantic_lookup::MemberTypeLookupResult member =
        semantic_lookup::lookup_member_type(*services.semantic_context,
                                            *resolved_scope.class_info,
                                            normalized_name,
                                            true,
                                            lexical_scope.valid() ?
                                                &lexical_scope.require() :
                                                nullptr);
    direct = member.type;
  } else {
    std::map<std::string, TypePtr>::const_iterator local =
        resolved_scope.named_types.find(normalized_name);
    if(local != resolved_scope.named_types.end()) {
      direct = local->second;
    }
  }
  if(!direct && !resolved_scope.class_info) {
    direct = template_api::lookup_direct_named_type_in_inline_namespaces(
        resolved_scope,
        normalized_name);
  }
  if(!direct) {
    return TypePtr();
  }

  TypePtr resolved;
  if(resolve_instantiated_dependent_type(services, lexical_scope, direct, resolved) &&
     resolved) {
    direct = resolved;
  }
  return direct;
}

ClassInfo * class_info_for_named_type(template_api::TemplateServices & services,
                                      const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  const std::map<std::string, ClassInfo *> * const classes_by_key =
      service_classes_by_key(services);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     !classes_by_key) {
    return nullptr;
  }
  map<string, ClassInfo *>::const_iterator found =
      classes_by_key->find(base->named_key);
  return found == classes_by_key->end() ?
             nullptr :
             found->second;
}

ClassInfo * concrete_current_specialization_for_scope(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    ClassInfo * candidate = current->class_info;
    if(!candidate && current->function) {
      candidate = current->function->lexical_access_class ?
          current->function->lexical_access_class :
          current->function->owner_class;
    }
    if(candidate &&
       candidate->source_template &&
       candidate->type &&
       !candidate->dependent_instantiation) {
      return candidate;
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return nullptr;
}

TypePtr current_specialization_type_for_dependent_owner(
    template_api::TemplateServices & services,
    Scope & scope,
    const TypePtr & owner_type)
{
  ClassInfo * current = concrete_current_specialization_for_scope(scope);
  if(!current) {
    return TypePtr();
  }

  const auto owner_text_names_current_specialization = [&]() -> bool
  {
    string owner_text = reparseable_type_argument_text(owner_type);
    if(owner_text.empty()) {
      owner_text =
          owner_type && !owner_type->named_display.empty() ?
              owner_type->named_display :
              (owner_type ? describe_type(owner_type) : string());
    }
    owner_text = normalize_type_lookup_name(trim_space(owner_text));
    QualifiedName template_name;
    vector<string> arg_texts;
    if(!semantic_utils::split_top_level_template_id_text(owner_text,
                                                         template_name,
                                                         arg_texts)) {
      return false;
    }
    if(unqualified_member_name(template_name.name) != current->source_template->name ||
       arg_texts.size() > current->source_template->parameters.size()) {
      return false;
    }
    for(size_t i = 0; i < arg_texts.size(); ++i) {
      const TemplateParameterInfo & parameter =
          current->source_template->parameters[i];
      if(parameter.name.empty()) {
        return false;
      }
      string arg = normalize_type_lookup_name(
          strip_elaborated_type_prefix(trim_space(arg_texts[i])));
      const string typename_prefix = "typename ";
      if(arg.compare(0, typename_prefix.size(), typename_prefix) == 0) {
        arg = trim_space(arg.substr(typename_prefix.size()));
      }
      if(arg != parameter.name) {
        return false;
      }
    }
    return true;
  };

  if(owner_text_names_current_specialization()) {
    return current->type;
  }

  ClassInfo * owner = class_info_for_named_type(services, owner_type);
  if(!owner ||
     !owner->source_template ||
     !owner->dependent_instantiation ||
     owner->source_template != current->source_template ||
     owner->template_output_node != current->template_output_node) {
    return TypePtr();
  }

  return current->type;
}

TypePtr current_specialization_type_for_dependent_qualifier_text(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & qualifier_text)
{
  ClassInfo * current = concrete_current_specialization_for_scope(scope);
  if(!current) {
    return TypePtr();
  }

  const string normalized_qualifier =
      callsemantic_internal::normalize_qualified_name_spacing(
          normalize_type_lookup_name(qualifier_text));
  const auto candidate_matches =
      [&](ClassInfo * candidate) -> bool
  {
    if(!candidate ||
       candidate == current ||
       !candidate->dependent_instantiation ||
       candidate->source_template != current->source_template ||
       candidate->template_output_node != current->template_output_node) {
      return false;
    }
    const string candidate_qualified =
        callsemantic_internal::normalize_qualified_name_spacing(
            normalize_type_lookup_name(candidate->qualified_name));
    const string candidate_display =
        callsemantic_internal::normalize_qualified_name_spacing(
            normalize_type_lookup_name(candidate->display_qualified_name));
    return normalized_qualifier == candidate_qualified ||
           (!candidate_display.empty() && normalized_qualifier == candidate_display);
  };

  for(map<string, ClassInfo *>::const_iterator it =
          current->source_template->instantiations.begin();
      it != current->source_template->instantiations.end();
      ++it) {
    if(candidate_matches(it->second)) {
      return current->type;
    }
  }
  for(map<string, ClassInfo *>::const_iterator it =
          current->source_template->reference_instantiations.begin();
      it != current->source_template->reference_instantiations.end();
      ++it) {
    if(candidate_matches(it->second)) {
      return current->type;
    }
  }
  return TypePtr();
}

bool lookup_leaf_member_expression_type_in_scope(template_api::TemplateTypeSystem & type_system,
                                                 Scope & member_scope,
                                                 const string & name,
                                                 TypePtr & out)
{
  out.reset();

  map<string, ValueBinding>::const_iterator value_found = member_scope.values.find(name);
  if(value_found != member_scope.values.end() && value_found->second.type) {
    out = value_found->second.type;
    return true;
  }

  map<string, vector<FunctionBinding *> >::const_iterator function_found =
      member_scope.function_sets.find(name);
  if(function_found != member_scope.function_sets.end() &&
     function_found->second.size() == 1 &&
     function_found->second[0] &&
     function_found->second[0]->type) {
    out = function_found->second[0]->type;
    return true;
  }

  if(!member_scope.class_info) {
    return false;
  }

  for(size_t i = 0; i < member_scope.class_info->bases.size(); ++i) {
    if(!member_scope.class_info->bases[i].type ||
       member_scope.class_info->bases[i].is_virtual) {
      continue;
    }

    ClassInfo * base_info = template_api::find_named_type_class_info(
        type_system.model,
        member_scope.class_info->bases[i].type->type);
    if(base_info &&
       base_info->member_scope &&
       lookup_leaf_member_expression_type_in_scope(
           type_system, *base_info->member_scope, name, out)) {
      return true;
    }
  }

  return false;
}

bool materialize_leaf_member_constant_binding(
    template_api::TemplateServices & services,
    ValueBinding & binding,
    constant_eval::ConstexprValue & out)
{
  if(binding.kind == ValueBinding::VK_FIELD) {
    return false;
  }
  if(binding.has_constant_value) {
    if(services.semantic_context) {
      template_api::note_template_member_value_instantiation_if_needed(
          *services.semantic_context,
          binding);
    }
    note_non_bool_static_value_dependency_for_witness(services, binding);
    out = constant_eval::make_integral_value(
        binding.constant_value,
        binding.type ? binding.type : make_fundamental(FT_INT));
    return true;
  }
  if(binding.has_constexpr_value) {
    if(services.semantic_context) {
      template_api::note_template_member_value_instantiation_if_needed(
          *services.semantic_context,
          binding);
    }
    note_non_bool_static_value_dependency_for_witness(services, binding);
    out = binding.constexpr_value;
    return out.kind != constant_eval::ConstexprValue::CV_INVALID;
  }
  if(binding.dependent_template_value ||
     service_type_depends_on_template_parameter(services, binding.type) ||
     !binding.constant_initializer ||
     !binding.constant_initializer_scope ||
     binding.constant_value_in_progress) {
    return false;
  }

  constant_eval::ConstexprValue value;
  binding.constant_value_in_progress = true;
  bool evaluated = false;
  try {
    evaluated =
        evaluate_constant_expression_leaf_impl(services,
                                               *binding.constant_initializer_scope,
                                               *binding.constant_initializer,
                                               value,
                                               binding.type);
  } catch(...) {
    binding.constant_value_in_progress = false;
    throw;
  }
  binding.constant_value_in_progress = false;
  if(!evaluated) {
    return false;
  }

  binding.has_constexpr_value = true;
  binding.constexpr_value = value;
  long long integral = 0;
  if(constant_eval::constexpr_value_to_integral(value, integral)) {
    binding.has_constant_value = true;
    binding.constant_value = integral;
  }
  binding.dependent_template_value = false;
  if(services.semantic_context) {
    template_api::note_template_member_value_instantiation_if_needed(
        *services.semantic_context,
        binding);
  }
  note_non_bool_static_value_dependency_for_witness(services, binding);
  out = value;
  return out.kind != constant_eval::ConstexprValue::CV_INVALID;
}

bool lookup_leaf_member_expression_value_in_scope(
    template_api::TemplateServices & services,
    Scope & member_scope,
    const string & name,
    constant_eval::ConstexprValue & out,
    bool allow_structured_bool_shortcut)
{
  out = constant_eval::ConstexprValue();

  if(allow_structured_bool_shortcut && name == "value" && member_scope.class_info) {
    bool structured_value = false;
    bool structured_evaluation_incomplete = false;
    set<string> visiting;
    if(structured_bool_constant_value_for_class_info(
           service_type_system(services),
           *member_scope.class_info,
           structured_value,
           visiting,
           &services,
           template_api::make_template_environment(member_scope),
           &structured_evaluation_incomplete)) {
      out = constant_eval::make_integral_value(
          structured_value ? 1 : 0,
          make_fundamental(FT_BOOL));
      if(services.witness_context.session != nullptr) {
        note_structured_bool_value_member_if_needed(services,
                                                   *member_scope.class_info);
      }
      return true;
    }
    if(structured_evaluation_incomplete) {
      return false;
    }
  }

  map<string, ValueBinding>::iterator value_found = member_scope.values.find(name);
  if(value_found != member_scope.values.end() &&
     materialize_leaf_member_constant_binding(services, value_found->second, out)) {
    return true;
  }

  if(!member_scope.class_info) {
    return false;
  }

  for(size_t i = 0; i < member_scope.class_info->bases.size(); ++i) {
    if(!member_scope.class_info->bases[i].type) {
      continue;
    }
    ClassInfo * base_info = template_api::find_named_type_class_info(
        service_type_system(services).model,
        member_scope.class_info->bases[i].type->type);
    if(!base_info || !base_info->member_scope) {
      continue;
    }
    if(lookup_leaf_member_expression_value_in_scope(
           services,
           *base_info->member_scope,
           name,
           out,
           allow_structured_bool_shortcut)) {
      return true;
    }
  }

  return false;
}

void collect_leaf_member_function_bindings_in_scope(
    template_api::TemplateTypeSystem & type_system,
    Scope & member_scope,
    const string & name,
    unordered_set<const FunctionBinding *> & seen_bindings,
    vector<FunctionBinding *> & out)
{
  map<string, vector<FunctionBinding *> >::const_iterator function_found =
      member_scope.function_sets.find(name);
  if(function_found != member_scope.function_sets.end()) {
    for(size_t i = 0; i < function_found->second.size(); ++i) {
      FunctionBinding * binding = function_found->second[i];
      if(binding && seen_bindings.insert(binding).second) {
        out.push_back(binding);
      }
    }
  }

  if(!member_scope.class_info) {
    return;
  }

  for(size_t i = 0; i < member_scope.class_info->bases.size(); ++i) {
    if(!member_scope.class_info->bases[i].type) {
      continue;
    }
    ClassInfo * base_info = template_api::find_named_type_class_info(
        type_system.model,
        member_scope.class_info->bases[i].type->type);
    if(!base_info || !base_info->member_scope) {
      continue;
    }
    collect_leaf_member_function_bindings_in_scope(
        type_system, *base_info->member_scope, name, seen_bindings, out);
  }
}

bool lookup_leaf_member_function_bindings(template_api::TemplateTypeSystem & type_system,
                                          const TypePtr & base_type,
                                          const string & name,
                                          vector<FunctionBinding *> & out)
{
  out.clear();

  TypePtr class_type = strip_top_level_cv(remove_reference_type(base_type));
  if(!class_type) {
    return false;
  }

  ClassInfo * info = template_api::find_named_type_class_info(type_system.model, class_type);
  if(!info || !info->member_scope) {
    return false;
  }

  unordered_set<const FunctionBinding *> seen_bindings;
  collect_leaf_member_function_bindings_in_scope(
      type_system, *info->member_scope, name, seen_bindings, out);
  return !out.empty();
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

const CppAstNode * unwrap_call_callee(const CppAstNode & expr)
{
  if(expr.kind != CppAstKind::call_expression || expr.children.empty()) {
    return nullptr;
  }
  const CppAstNode * callee = &expr.children[0];
  while(callee->kind == CppAstKind::parenthesized_expression &&
        callee->children.size() == 1) {
    callee = &callee->children[0];
  }
  return callee;
}

const CppAstNode * unwrap_initializer_payload(const CppAstNode & node)
{
  if(node.kind == CppAstKind::initializer && node.children.size() == 1) {
    return &node.children[0];
  }
  return &node;
}

bool constexpr_base_expression_is_lvalue(const CppAstNode & expr)
{
  const CppAstNode * current = &expr;
  while(current->kind == CppAstKind::parenthesized_expression &&
        current->children.size() == 1) {
    current = &current->children[0];
  }
  return current->kind == CppAstKind::id_expression ||
         current->kind == CppAstKind::member_expression ||
         current->kind == CppAstKind::subscript_expression;
}

constant_eval::Hooks build_leaf_constant_eval_hooks(template_api::TemplateServices & services,
                                                    Scope & scope);
bool lookup_leaf_operand_type(template_api::TemplateServices & services,
                              Scope & scope,
                              const CppAstNode & expr,
                              TypePtr & out);
TypePtr collapse_rvalue_reference_type(const TypePtr & inner);
bool try_parse_builtin_type_trait_text(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const string & text,
                                       string & trait_name,
                                       vector<TypePtr> & types);

bool binding_supports_leaf_call_shape(FunctionBinding & binding,
                                      size_t explicit_arg_count)
{
  if(binding.is_deleted) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(binding.type);
  if(!function_type || function_type->kind != Type::TK_FUNCTION ||
     function_type->variadic || function_type->prototype_relaxed) {
    return false;
  }

  const size_t explicit_param_offset = binding.is_method ? 1u : 0u;
  if(explicit_arg_count + explicit_param_offset > binding.params.size()) {
    return false;
  }
  if(function_type->params.size() != binding.params.size() &&
     function_type->params.size() + explicit_param_offset != binding.params.size()) {
    return false;
  }

  for(size_t i = explicit_arg_count + explicit_param_offset;
      i < binding.params.size();
      ++i) {
    if(i >= binding.default_arguments.size() || !binding.default_arguments[i]) {
      return false;
    }
  }

  return true;
}

bool binding_supports_leaf_constexpr_call(FunctionBinding & binding,
                                          size_t explicit_arg_count)
{
  return binding.is_constexpr &&
         binding.body &&
         binding_supports_leaf_call_shape(binding, explicit_arg_count);
}

bool lookup_leaf_expression_type_category(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const CppAstNode & expr,
                                          TypePtr & out,
                                          semantic_conversion::ValueCategory & category);

bool class_operand_declares_address_of_operator(template_api::TemplateServices & services,
                                                const TypePtr & operand_type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(operand_type));
  if(!base) {
    base = strip_top_level_cv(operand_type);
  }
  if(!base) {
    return false;
  }

  ClassInfo * info = nullptr;
  if(services.semantic_context) {
    try {
      info = services.semantic_context->complete_class_type(base);
    } catch(const logic_error &) {
      return false;
    }
  } else {
    info = class_info_for_named_type(services, base);
  }
  if(!info) {
    return false;
  }

  if(services.semantic_context &&
     !info->reference_members_collected &&
     !info->reference_member_collection_in_progress) {
    services.semantic_context->ensure_class_reference_members(*info);
  }

  return !semantic_lookup::lookup_visible_member_functions(*info, "operator&")
              .functions.empty();
}

bool select_unique_leaf_function_binding(
    const vector<FunctionBinding *> & functions,
    const vector<pair<TypePtr, semantic_conversion::ValueCategory> > & arg_infos,
    bool is_method_call,
    const TypePtr & object_type,
    bool object_is_const,
    bool base_is_lvalue,
    FunctionBinding *& out)
{
  out = nullptr;
  for(size_t i = 0; i < functions.size(); ++i) {
    FunctionBinding * candidate = functions[i];
    if(!candidate || candidate->is_method != is_method_call ||
       !binding_supports_leaf_call_shape(*candidate, arg_infos.size()) ||
       candidate->is_volatile_method) {
      continue;
    }
    if(is_method_call) {
      if(object_is_const && !candidate->is_const_method) {
        continue;
      }
      if(candidate->ref_qualifier == RQ_LVALUE && !base_is_lvalue) {
        continue;
      }
      if(candidate->ref_qualifier == RQ_RVALUE && base_is_lvalue) {
        continue;
      }
      if(!object_type) {
        continue;
      }
    }

    bool matches = true;
    for(size_t arg_index = 0; arg_index < arg_infos.size(); ++arg_index) {
      const size_t param_index = (is_method_call ? 1u : 0u) + arg_index;
      semantic_conversion::ExprInfo expr_info;
      expr_info.type = arg_infos[arg_index].first;
      expr_info.category = arg_infos[arg_index].second;
      if(semantic_conversion::standard_conversion_rank(
             candidate->params[param_index].second,
             expr_info) == semantic_conversion::CR_BAD) {
        matches = false;
        break;
      }
    }
    if(!matches) {
      continue;
    }
    if(out) {
      out = nullptr;
      return false;
    }
    out = candidate;
  }

  return out != nullptr;
}

bool leaf_function_type_call_result(
    const TypePtr & callable_type,
    const vector<pair<TypePtr, semantic_conversion::ValueCategory> > & arg_infos,
    TypePtr & out,
    semantic_conversion::ValueCategory & category)
{
  TypePtr function_type = strip_top_level_cv(callable_type);
  if(function_type &&
     (function_type->kind == Type::TK_LVALUE_REFERENCE ||
      function_type->kind == Type::TK_RVALUE_REFERENCE)) {
    function_type = strip_top_level_cv(function_type->inner);
  }
  if(function_type && function_type->kind == Type::TK_POINTER) {
    function_type = strip_top_level_cv(function_type->inner);
  }
  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return false;
  }
  if(function_type->prototype_relaxed) {
    return false;
  }
  if(function_type->variadic) {
    if(arg_infos.size() < function_type->params.size()) {
      return false;
    }
  } else if(arg_infos.size() != function_type->params.size()) {
    return false;
  }

  for(size_t i = 0; i < function_type->params.size(); ++i) {
    semantic_conversion::ExprInfo expr_info;
    expr_info.type = arg_infos[i].first;
    expr_info.category = arg_infos[i].second;
    if(semantic_conversion::standard_conversion_rank(function_type->params[i],
                                                     expr_info) ==
       semantic_conversion::CR_BAD) {
      return false;
    }
  }

  if(!semantic_conversion::result_value_category_for_function_result(
         function_type->inner,
         category)) {
    return false;
  }
  out = function_type->inner;
  return out != nullptr;
}

void append_leaf_function_template_instantiations(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & callee,
    const TemplateIdSyntax * template_id,
    const QualifiedName * callee_qualified,
    const string & callee_lookup_name,
    const vector<pair<TypePtr, semantic_conversion::ValueCategory> > & arg_infos,
    bool include_body,
    vector<FunctionBinding *> & functions)
{
  const QualifiedName * template_qualified =
      template_id &&
      (template_id->name.rooted || !template_id->name.qualifiers.empty()) ?
          &template_id->name :
          callee_qualified;

  vector<FunctionTemplateDecl *> function_templates;
  if(template_qualified) {
    lookup_leaf_qualified_function_templates(
        services, scope, *template_qualified, &callee, function_templates);
  } else if(services.semantic_context) {
    semantic_lookup::collect_function_templates(
        *services.semantic_context, scope, callee_lookup_name, function_templates);
  }

  bool has_dependent_arg_type = false;
  for(size_t i = 0; i < arg_infos.size(); ++i) {
    if(service_type_depends_on_template_parameter(services, arg_infos[i].first)) {
      has_dependent_arg_type = true;
      break;
    }
  }
  if(!services.semantic_context ||
     has_dependent_arg_type ||
     function_templates.empty()) {
    return;
  }

  vector<semantic_conversion::ExprInfo> expr_args;
  expr_args.reserve(arg_infos.size());
  for(size_t i = 0; i < arg_infos.size(); ++i) {
    semantic_conversion::ExprInfo info;
    info.type = arg_infos[i].first;
    info.category = arg_infos[i].second;
    expr_args.push_back(info);
  }

  for(size_t i = 0; i < function_templates.size(); ++i) {
    if(!function_templates[i]) {
      continue;
    }
    vector<TemplateArgument> explicit_arguments;
    const vector<TemplateArgument> * explicit_arguments_ptr = nullptr;
    if(template_id) {
      if(!template_api::resolve_template_arguments(
             services,
             template_api::make_template_environment(scope),
             function_templates[i]->parameters,
             template_id->arguments,
             &template_id->argument_syntaxes,
             explicit_arguments,
             function_templates[i]->declaring_scope ?
                 template_api::make_template_environment(
                     *function_templates[i]->declaring_scope) :
                 template_api::TemplateEnvironmentHandle())) {
        continue;
      }
      explicit_arguments_ptr = &explicit_arguments;
    }
    template_api::TemplateFunctionDeductionRequest deduction_request;
    deduction_request.decl = function_templates[i];
    deduction_request.args = &expr_args;
    deduction_request.use_scope = &scope;
    deduction_request.resolution_scope = &scope;
    deduction_request.explicit_arguments = explicit_arguments_ptr;
    template_api::TemplateFunctionDeductionResult deduction_result;
    if(!template_api::deduce_function_template(*services.semantic_context,
                                               deduction_request,
                                               deduction_result)) {
      continue;
    }
    template_api::TemplateFunctionInstantiationRequest instantiation_request;
    instantiation_request.decl = function_templates[i];
    instantiation_request.arguments = deduction_result.arguments;
    instantiation_request.use_scope = template_api::make_template_environment(scope);
    instantiation_request.include_body = include_body;
    instantiation_request.pack_sizes = deduction_result.pack_sizes;
    instantiation_request.has_pack_sizes = !deduction_result.pack_sizes.empty();
    try {
      FunctionBinding * binding =
          template_api::acquire_function_instantiation(
              *services.semantic_context,
              instantiation_request).function_binding;
      append_unique_leaf_function_binding(functions, binding);
    } catch(const TemplateSubstitutionFailure &) {
    }
  }
}

bool leaf_id_expression_names_function_or_template(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node)
{
  if(node.kind != CppAstKind::id_expression) {
    return false;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
  const QualifiedName * qualified = qualified_syntax_if_qualified(node);
  const string lookup_name = template_id ? template_id->name.name : node.value;
  const QualifiedName * template_qualified =
      template_id &&
      (template_id->name.rooted || !template_id->name.qualifiers.empty()) ?
          &template_id->name :
          qualified;

  vector<FunctionBinding *> functions;
  lookup_leaf_function_bindings(scope, lookup_name, functions);
  if(template_qualified) {
    vector<FunctionBinding *> qualified_functions;
    if(lookup_leaf_qualified_function_bindings(
           services, scope, *template_qualified, &node, qualified_functions)) {
      append_unique_leaf_function_bindings(functions, qualified_functions);
    }
  }
  if(!functions.empty()) {
    return true;
  }

  vector<FunctionTemplateDecl *> function_templates;
  if(template_qualified) {
    lookup_leaf_qualified_function_templates(
        services, scope, *template_qualified, &node, function_templates);
  } else if(services.semantic_context) {
    semantic_lookup::collect_function_templates(
        *services.semantic_context, scope, lookup_name, function_templates);
  }
  return !function_templates.empty();
}

bool lookup_leaf_call_expression_type(template_api::TemplateServices & services,
                                      Scope & scope,
                                      const CppAstNode & expr,
                                      TypePtr & out,
                                      semantic_conversion::ValueCategory & category)
{
  out.reset();
  const CppAstNode * callee = unwrap_call_callee(expr);
  if(!callee) {
    return false;
  }

  const CppAstNode * argument_list = find_child_kind(expr, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
  }
  const bool zero_arg_braced_type_init =
      argument_list &&
      argument_list->children.size() == 1 &&
      argument_list->children[0].kind == CppAstKind::braced_init_list &&
      argument_list->children[0].children.empty();

  vector<pair<TypePtr, semantic_conversion::ValueCategory> > arg_infos;
  if(argument_list && !zero_arg_braced_type_init) {
    for(size_t i = 0; i < argument_list->children.size(); ++i) {
      TypePtr arg_type;
      semantic_conversion::ValueCategory arg_category = semantic_conversion::VC_PRVALUE;
      if(!lookup_leaf_expression_type_category(
             services, scope, argument_list->children[i], arg_type, arg_category) ||
         !arg_type) {
        return false;
      }
      TypePtr resolved_arg_type = arg_type;
      if(resolve_instantiated_dependent_type_if_needed(
             services,
             template_api::make_template_environment(scope),
             resolved_arg_type)) {
        arg_type = resolved_arg_type;
      }
      arg_infos.push_back(make_pair(arg_type, arg_category));
    }
  }

  if(callee->kind == CppAstKind::id_expression) {
    const TemplateIdSyntax * template_id = cppast_template_id_syntax(*callee);
    if(template_id &&
       template_id->arguments.size() == 1 &&
       ((template_id->name.name == "declval" && arg_infos.empty()) ||
        (template_id->name.name == "__declval" &&
         arg_infos.size() == 1 &&
         arg_infos[0].first &&
         is_integral_type(strip_top_level_cv(remove_reference_type(arg_infos[0].first)))))) {
      const TemplateArgumentSyntax * arg_syntax =
          template_id->argument_syntaxes.size() == 1 ?
              &template_id->argument_syntaxes[0] :
              nullptr;
      TypePtr declval_type;
      if(resolve_type_argument_input(
             services,
             template_api::make_template_environment(scope),
             arg_syntax,
             true,
             declval_type) &&
         declval_type) {
        TypePtr result_type = is_void_type(strip_top_level_cv(declval_type)) ?
            declval_type :
            collapse_rvalue_reference_type(declval_type);
        if(semantic_conversion::result_value_category_for_function_result(
               result_type, category)) {
          out = result_type;
          return out != nullptr;
        }
      }
    }

    if(arg_infos.empty()) {
      TypePtr target;
      const string callee_template_location =
          template_id ?
              template_api::normalize_template_witness_source_location(
                  template_api::template_witness_detail::source_location_for_location_id(
                      services.witness_context,
                      template_id->source_location_id)) :
              string();
      const QualifiedName * callee_qualified =
          qualified_syntax_if_qualified(*callee);
      bool resolved_target =
          template_id ?
              resolve_template_id_syntax_type(
                  services,
                  scope,
                  *template_id,
                  false,
                  callee_template_location,
                  target,
                  template_api::make_template_environment(scope),
                  template_api::ClassTemplateSourceUseMode::EmitClassUse,
                  false) :
              (callee_qualified ?
                   resolve_direct_type_name_lookup(
                       services,
                       scope,
                       *callee_qualified,
                       false,
                       callee_template_location,
                       target) :
                   resolve_direct_type_name_lookup(
                       services,
                       scope,
                       callee->value,
                       false,
                       callee_template_location,
                       target));
      if(resolved_target &&
         target) {
        out = target;
        category = semantic_conversion::VC_PRVALUE;
        return true;
      }
    }

    vector<FunctionBinding *> functions;
    const QualifiedName * callee_qualified = qualified_syntax_if_qualified(*callee);
    const string callee_lookup_name =
        template_id ? template_id->name.name : callee->value;
    const QualifiedName * template_qualified =
        template_id &&
        (template_id->name.rooted || !template_id->name.qualifiers.empty()) ?
            &template_id->name :
            callee_qualified;
    if(!template_id) {
      lookup_leaf_function_bindings(scope, callee_lookup_name, functions);
      if(template_qualified) {
        vector<FunctionBinding *> qualified_functions;
        if(lookup_leaf_qualified_function_bindings(
               services, scope, *template_qualified, callee, qualified_functions)) {
          append_unique_leaf_function_bindings(functions, qualified_functions);
        }
      }
    }
    append_leaf_function_template_instantiations(services,
                                                scope,
                                                *callee,
                                                template_id,
                                                callee_qualified,
                                                callee_lookup_name,
                                                arg_infos,
                                                false,
                                                functions);
    if(!functions.empty()) {
      FunctionBinding * selected = nullptr;
      if(select_unique_leaf_function_binding(
             functions,
             arg_infos,
             false,
             TypePtr(),
             false,
             false,
             selected) &&
         selected) {
        TypePtr function_type = strip_top_level_cv(selected->type);
        if(function_type &&
           semantic_conversion::result_value_category_for_function_result(
               function_type->inner, category)) {
          out = function_type->inner;
          if(services.witness_context.session != nullptr &&
             out &&
             !service_type_depends_on_template_parameter(services, out)) {
            note_structured_bool_value_member_if_needed(
                services,
                template_api::make_template_environment(scope),
                out);
          }
          return out != nullptr;
        }
      }
    }

    TypePtr callee_type;
    semantic_conversion::ValueCategory callee_category =
        semantic_conversion::VC_PRVALUE;
    if(lookup_leaf_expression_type_category(
           services, scope, *callee, callee_type, callee_category) &&
       leaf_function_type_call_result(callee_type, arg_infos, out, category)) {
      return true;
    }
    return false;
  }

  if(callee->kind != CppAstKind::member_expression ||
     callee->children.size() != 2 ||
     !node_has_simple_type(*callee, OP_DOT) ||
     (callee->children[1].kind != CppAstKind::identifier &&
      callee->children[1].kind != CppAstKind::id_expression)) {
    TypePtr callee_type;
    semantic_conversion::ValueCategory callee_category =
        semantic_conversion::VC_PRVALUE;
    if(!lookup_leaf_expression_type_category(
           services, scope, *callee, callee_type, callee_category) ||
       !callee_type) {
      return false;
    }

    if(leaf_function_type_call_result(callee_type, arg_infos, out, category)) {
      return true;
    }

    TypePtr object_type = strip_top_level_cv(remove_reference_type(callee_type));
    if(!object_type) {
      return false;
    }

    vector<FunctionBinding *> functions;
    if(!service_lookup_leaf_member_function_bindings(
           services, object_type, "operator()", functions)) {
      return false;
    }

    FunctionBinding * selected = nullptr;
    if(!select_unique_leaf_function_binding(
           functions,
           arg_infos,
           true,
           object_type,
           semantic_conversion::is_const_object_type(object_type),
           callee_category == semantic_conversion::VC_LVALUE,
           selected) ||
       !selected) {
      return false;
    }

    TypePtr function_type = strip_top_level_cv(selected->type);
    if(!function_type ||
       !semantic_conversion::result_value_category_for_function_result(
           function_type->inner, category)) {
      return false;
    }
    out = function_type->inner;
    if(services.witness_context.session != nullptr &&
       out &&
       !service_type_depends_on_template_parameter(services, out)) {
      note_structured_bool_value_member_if_needed(
          services,
          template_api::make_template_environment(scope),
          out);
    }
    return out != nullptr;
  }

  TypePtr base_type;
  semantic_conversion::ValueCategory base_category = semantic_conversion::VC_PRVALUE;
  if(!lookup_leaf_expression_type_category(
         services, scope, callee->children[0], base_type, base_category) ||
     !base_type) {
    return false;
  }

  const bool base_is_lvalue = base_category == semantic_conversion::VC_LVALUE;
  TypePtr object_type = strip_top_level_cv(remove_reference_type(base_type));
  if(!object_type) {
    return false;
  }

  vector<FunctionBinding *> functions;
  if(!service_lookup_leaf_member_function_bindings(
         services, object_type, callee->children[1].value, functions)) {
    return false;
  }

  FunctionBinding * selected = nullptr;
  if(!select_unique_leaf_function_binding(
         functions,
         arg_infos,
         true,
         object_type,
         semantic_conversion::is_const_object_type(object_type),
         base_is_lvalue,
         selected) ||
     !selected) {
    return false;
  }

  TypePtr function_type = strip_top_level_cv(selected->type);
  if(!function_type ||
     !semantic_conversion::result_value_category_for_function_result(
         function_type->inner, category)) {
    return false;
  }
  out = function_type->inner;
  if(services.witness_context.session != nullptr &&
     out &&
     !service_type_depends_on_template_parameter(services, out)) {
    note_structured_bool_value_member_if_needed(
        services,
        template_api::make_template_environment(scope),
        out);
  }
  return out != nullptr;
}

bool lookup_leaf_expression_type_category(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const CppAstNode & expr,
                                          TypePtr & out,
                                          semantic_conversion::ValueCategory & category)
{
  out.reset();
  category = semantic_conversion::VC_PRVALUE;

  if(expr.kind == CppAstKind::parenthesized_expression && expr.children.size() == 1) {
    return lookup_leaf_expression_type_category(
        services, scope, expr.children[0], out, category);
  }

  if(expr.kind == CppAstKind::keyword_literal) {
    if(node_has_simple_type(expr, KW_NULLPTR)) {
      out = make_fundamental(FT_NULLPTR_T);
      category = semantic_conversion::VC_PRVALUE;
      return true;
    }
    if(node_has_simple_type(expr, KW_TRUE) ||
       node_has_simple_type(expr, KW_FALSE)) {
      out = make_fundamental(FT_BOOL);
      category = semantic_conversion::VC_PRVALUE;
      return true;
    }
    return false;
  }

  if(expr.kind == CppAstKind::literal &&
     !expr.value.empty() &&
     std::isdigit(static_cast<unsigned char>(expr.value[0]))) {
    try {
      if(expr.value.find('.') != string::npos ||
         expr.value.find('e') != string::npos ||
         expr.value.find('E') != string::npos) {
        string value;
        EFundamentalType literal_type = FT_DOUBLE;
        string ud_suffix;
        if(!split_floating_literal(expr.value, value, literal_type, ud_suffix) ||
           !ud_suffix.empty()) {
          return false;
        }
        out = make_fundamental(literal_type);
      } else {
        unsigned long long value = 0;
        string ud_suffix;
        const EFundamentalType literal_type =
            classify_int(expr.value, value, ud_suffix);
        if(!ud_suffix.empty()) {
          return false;
        }
        out = make_fundamental(literal_type);
      }
      category = semantic_conversion::VC_PRVALUE;
      return out != nullptr;
    } catch(const logic_error &) {
      return false;
    }
  }

  if(expr.kind == CppAstKind::id_expression) {
    const ValueBinding * binding = nullptr;
    const QualifiedName * qualified = qualified_syntax_if_qualified(expr);
    if((lookup_leaf_value_binding(scope, expr.value, binding) ||
        (qualified &&
         lookup_leaf_qualified_value_binding(
             services, scope, *qualified, &expr, binding))) &&
       binding && binding->type) {
      out = binding->type;
      category = semantic_conversion::VC_LVALUE;
      return true;
    }

    vector<FunctionBinding *> functions;
    if((lookup_leaf_function_bindings(scope, expr.value, functions) ||
        (qualified &&
         lookup_leaf_qualified_function_bindings(
             services, scope, *qualified, &expr, functions))) &&
       functions.size() == 1 && functions[0] && functions[0]->type) {
      out = functions[0]->type;
      category = semantic_conversion::VC_LVALUE;
      return true;
    }
    return false;
  }

  if(expr.kind == CppAstKind::member_expression) {
    if(lookup_leaf_operand_type(services, scope, expr, out) && out) {
      category = semantic_conversion::VC_LVALUE;
      return true;
    }
    return false;
  }

  if(expr.kind == CppAstKind::call_expression) {
    return lookup_leaf_call_expression_type(services, scope, expr, out, category);
  }

  if(expr.kind == CppAstKind::unary_expression && expr.children.size() == 1) {
    TypePtr operand_type;
    semantic_conversion::ValueCategory operand_category =
        semantic_conversion::VC_PRVALUE;
    if(!lookup_leaf_expression_type_category(
           services, scope, expr.children[0], operand_type, operand_category) ||
       !operand_type) {
      return false;
    }

    TypePtr converted_type = strip_top_level_cv(remove_reference_type(operand_type));
    if(!converted_type) {
      converted_type = strip_top_level_cv(operand_type);
    }
    if(!converted_type) {
      return false;
    }

    if(node_has_simple_type(expr, OP_PLUS) || node_has_simple_type(expr, OP_MINUS)) {
      if(!semantic_conversion::is_integral_or_unscoped_enum_type(converted_type) &&
         !is_floating_type(converted_type)) {
        return false;
      }
      out = is_floating_type(converted_type) ?
          converted_type :
          semantic_conversion::promoted_integral_result_type(converted_type);
      category = semantic_conversion::VC_PRVALUE;
      return out != nullptr;
    }

    if(node_has_simple_type(expr, OP_COMPL)) {
      if(!semantic_conversion::is_integral_or_unscoped_enum_type(converted_type)) {
        return false;
      }
      out = semantic_conversion::promoted_integral_result_type(converted_type);
      category = semantic_conversion::VC_PRVALUE;
      return out != nullptr;
    }

    if(node_has_simple_type(expr, OP_LNOT)) {
      if(!semantic_conversion::is_condition_test_type(converted_type)) {
        return false;
      }
      out = make_fundamental(FT_BOOL);
      category = semantic_conversion::VC_PRVALUE;
      return true;
    }

    if(node_has_simple_type(expr, OP_AMP)) {
      if(operand_category != semantic_conversion::VC_LVALUE) {
        return false;
      }
      if(class_operand_declares_address_of_operator(services, operand_type)) {
        return false;
      }
      TypePtr pointee = remove_reference_type(operand_type);
      if(!pointee) {
        pointee = operand_type;
      }
      out = make_pointer(pointee);
      category = semantic_conversion::VC_PRVALUE;
      return out != nullptr;
    }

    if(node_has_simple_type(expr, OP_STAR)) {
      if(converted_type->kind != Type::TK_POINTER || !converted_type->inner) {
        return false;
      }
      out = converted_type->inner;
      category = semantic_conversion::VC_LVALUE;
      return true;
    }
  }

  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     node_has_simple_type(expr, OP_COMMA)) {
    TypePtr ignored;
    semantic_conversion::ValueCategory ignored_category =
        semantic_conversion::VC_PRVALUE;
    return lookup_leaf_expression_type_category(
               services, scope, expr.children[0], ignored, ignored_category) &&
           lookup_leaf_expression_type_category(
               services, scope, expr.children[1], out, category);
  }

  return false;
}

bool evaluate_leaf_constexpr_binding(template_api::TemplateServices & services,
                                     Scope & scope,
                                     FunctionBinding & binding,
                                     const vector<constant_eval::ConstexprValue> & args,
                                     bool has_implicit_object,
                                     const constant_eval::ConstexprValue & implicit_object,
                                     constant_eval::ConstexprValue & out)
{
  if(binding.body && binding.body->kind == CppAstKind::lazy_function_body) {
    if(!services.semantic_context) {
      return false;
    }
    binding.body =
        services.semantic_context->materialize_lazy_function_body(*binding.body);
  }
  const size_t explicit_param_offset = binding.is_method ? 1u : 0u;

  Scope & call_scope = binding.declaration_scope ? *binding.declaration_scope : scope;
  Scope constexpr_default_arg_scope =
      semantic_consteval::make_constexpr_call_scope(call_scope, &binding, false);
  const constant_eval::Hooks default_arg_hooks =
      build_leaf_constant_eval_hooks(services, constexpr_default_arg_scope);
  Scope constexpr_call_scope =
      semantic_consteval::make_constexpr_call_scope(call_scope, &binding);
  const constant_eval::Hooks call_hooks =
      build_leaf_constant_eval_hooks(services, constexpr_call_scope);

  vector<constant_eval::ConstexprValue> final_args = args;
  for(size_t i = final_args.size() + explicit_param_offset;
      i < binding.params.size();
      ++i) {
    const CppAstNode * default_arg = binding.default_arguments[i];
    const CppAstNode * payload =
        default_arg && default_arg->children.size() == 1 ?
            &default_arg->children[0] :
            default_arg;
    if(!payload) {
      return false;
    }
    constant_eval::ConstexprValue value;
    constant_eval::Evaluator default_evaluator(default_arg_hooks);
    if(!default_evaluator.eval_initializer(*payload, value, binding.params[i].second)) {
      return false;
    }
    final_args.push_back(value);
  }

  TypePtr function_type = strip_top_level_cv(binding.type);
  constant_eval::FunctionInfo info;
  info.name = binding.name;
  info.return_type = function_type->inner;
  info.params.assign(binding.params.begin() + explicit_param_offset,
                     binding.params.end());
  info.body = binding.body;
  info.variadic = false;
  info.is_method = binding.is_method;
  if(binding.is_method && has_implicit_object) {
    info.has_implicit_object = true;
    info.implicit_object = implicit_object;
  }

  constant_eval::Evaluator call_evaluator(call_hooks);
  return call_evaluator.call(info, final_args, out, &call_hooks);
}

bool evaluate_leaf_constexpr_function_call(template_api::TemplateServices & services,
                                           Scope & scope,
                                           constant_eval::Evaluator & evaluator,
                                           const CppAstNode & expr,
                                           const vector<constant_eval::ConstexprValue> & args,
                                           constant_eval::ConstexprValue & out)
{
  const CppAstNode * callee = unwrap_call_callee(expr);
  if(!callee) {
    return false;
  }

  constant_eval::ConstexprValue implicit_object;
  vector<FunctionBinding *> candidates;

  if(callee->kind == CppAstKind::id_expression) {
    const TemplateIdSyntax * template_id = cppast_template_id_syntax(*callee);
    const QualifiedName * callee_qualified = qualified_syntax_if_qualified(*callee);
    const string callee_lookup_name =
        template_id ? template_id->name.name : callee->value;
    const QualifiedName * template_qualified =
        template_id &&
        (template_id->name.rooted || !template_id->name.qualifiers.empty()) ?
            &template_id->name :
            callee_qualified;
    vector<FunctionBinding *> functions;
    if(!template_id) {
      lookup_leaf_function_bindings(scope, callee_lookup_name, functions);
      if(template_qualified) {
        vector<FunctionBinding *> qualified_functions;
        if(lookup_leaf_qualified_function_bindings(
               services, scope, *template_qualified, callee, qualified_functions)) {
          append_unique_leaf_function_bindings(functions, qualified_functions);
        }
      }
    }
    vector<pair<TypePtr, semantic_conversion::ValueCategory> > arg_infos;
    arg_infos.reserve(args.size());
    for(size_t i = 0; i < args.size(); ++i) {
      if(!args[i].type) {
        return false;
      }
      arg_infos.push_back(
          make_pair(args[i].type, semantic_conversion::VC_PRVALUE));
    }
    append_leaf_function_template_instantiations(services,
                                                scope,
                                                *callee,
                                                template_id,
                                                callee_qualified,
                                                callee_lookup_name,
                                                arg_infos,
                                                true,
                                                functions);
    if(functions.empty()) {
      return false;
    }
    for(size_t i = 0; i < functions.size(); ++i) {
      if(functions[i] && binding_supports_leaf_constexpr_call(*functions[i], args.size())) {
        candidates.push_back(functions[i]);
      }
    }
    if(candidates.size() != 1 || !candidates[0]) {
      return false;
    }

    FunctionBinding & binding = *candidates[0];
    if(!binding.is_method) {
      return evaluate_leaf_constexpr_binding(
          services, scope, binding, args, false, implicit_object, out);
    }
    if(!evaluator.current_this_object(implicit_object)) {
      return false;
    }
    TypePtr implicit_object_type = remove_reference_type(implicit_object.type);
    if((binding.ref_qualifier == RQ_RVALUE) ||
       (!binding.is_const_method &&
        implicit_object_type &&
        semantic_conversion::is_const_object_type(implicit_object_type))) {
      return false;
    }
    return evaluate_leaf_constexpr_binding(
        services, scope, binding, args, true, implicit_object, out);
  }

  if(callee->kind != CppAstKind::member_expression ||
     callee->children.size() != 2 ||
     !node_has_simple_type(*callee, OP_DOT) ||
     (callee->children[1].kind != CppAstKind::identifier &&
      callee->children[1].kind != CppAstKind::id_expression)) {
    return false;
  }

  if(!evaluator.eval_expr(callee->children[0], implicit_object)) {
    return false;
  }

  TypePtr object_type = remove_reference_type(implicit_object.type);
  const bool object_is_const =
      object_type && semantic_conversion::is_const_object_type(object_type);
  if(!service_lookup_leaf_member_function_bindings(
         services, object_type, callee->children[1].value, candidates)) {
    return false;
  }

  const bool base_is_lvalue = constexpr_base_expression_is_lvalue(callee->children[0]);
  FunctionBinding * selected = nullptr;
  for(size_t i = 0; i < candidates.size(); ++i) {
    FunctionBinding * candidate = candidates[i];
    if(!candidate || !candidate->is_method ||
       !binding_supports_leaf_constexpr_call(*candidate, args.size()) ||
       candidate->is_volatile_method) {
      continue;
    }
    if(object_is_const && !candidate->is_const_method) {
      continue;
    }
    if(candidate->ref_qualifier == RQ_LVALUE && !base_is_lvalue) {
      continue;
    }
    if(candidate->ref_qualifier == RQ_RVALUE && base_is_lvalue) {
      continue;
    }

    bool matches = true;
    for(size_t arg_index = 0; arg_index < args.size(); ++arg_index) {
      const std::size_t param_index = 1u + arg_index;
      const TypePtr & param_type = candidate->params[param_index].second;
      constant_eval::ConstexprValue converted;
      if(!constant_eval::constexpr_value_cast(args[arg_index], param_type, converted)) {
        matches = false;
        break;
      }
    }
    if(!matches) {
      continue;
    }
    if(selected) {
      return false;
    }
    selected = candidate;
  }

  return selected &&
         evaluate_leaf_constexpr_binding(
             services, scope, *selected, args, true, implicit_object, out);
}

bool evaluate_leaf_scalar_zero_value(const TypePtr & type,
                                     constant_eval::ConstexprValue & out)
{
  return constant_eval::constexpr_value_cast(
      constant_eval::make_integral_value(0, make_fundamental(FT_INT)),
      type,
      out);
}

bool evaluate_leaf_typed_initializer(template_api::TemplateServices & services,
                                     Scope & scope,
                                     constant_eval::Evaluator & evaluator,
                                     const CppAstNode & node,
                                     const TypePtr & target,
                                     constant_eval::ConstexprValue & out)
{
  if(!target) {
    return false;
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(!target_base) {
    return false;
  }

  if(is_reference_type(strip_top_level_cv(target))) {
    constant_eval::ConstexprValue referred;
    if(!evaluate_leaf_typed_initializer(
           services, scope, evaluator, node, target_base, referred)) {
      return false;
    }
    referred.type = target;
    out = referred;
    return true;
  }

  if(target_base->kind == Type::TK_ARRAY) {
    return false;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(service_describe_named_type_metadata(services, target_base, info)) {
    return false;
  }

  const CppAstNode * payload = unwrap_initializer_payload(node);
  if(!payload) {
    return false;
  }

  if(payload->kind == CppAstKind::paren_initializer ||
     payload->kind == CppAstKind::paren_argument_list ||
     payload->kind == CppAstKind::braced_init_list) {
    if(payload->children.empty()) {
      return evaluate_leaf_scalar_zero_value(target, out);
    }
    if(payload->children.size() != 1) {
      return false;
    }
    constant_eval::ConstexprValue value;
    if(!evaluator.eval_expr(payload->children[0], value) ||
       !constant_eval::constexpr_value_cast(value, target, out)) {
      return false;
    }
    out.type = target;
    return true;
  }

  constant_eval::ConstexprValue value;
  if(!evaluator.eval_expr(*payload, value) ||
     !constant_eval::constexpr_value_cast(value, target, out)) {
    return false;
  }
  out.type = target;
  return true;
}

bool lookup_leaf_operand_type(template_api::TemplateServices & services,
                              Scope & scope,
                              const CppAstNode & expr,
                              TypePtr & out)
{
  out.reset();
  if(expr.kind == CppAstKind::id_expression) {
    const ValueBinding * binding = nullptr;
    const QualifiedName * qualified = qualified_syntax_if_qualified(expr);
    if((lookup_leaf_value_binding(scope, expr.value, binding) ||
        (qualified &&
         lookup_leaf_qualified_value_binding(
             services, scope, *qualified, &expr, binding))) &&
       binding &&
       binding->type) {
      out = binding->type;
      return true;
    }

    vector<FunctionBinding *> functions;
    if((lookup_leaf_function_bindings(scope, expr.value, functions) ||
        (qualified &&
         lookup_leaf_qualified_function_bindings(
             services, scope, *qualified, &expr, functions))) &&
       functions.size() == 1 &&
       functions[0] &&
       functions[0]->type) {
      out = functions[0]->type;
      return true;
    }

    const TemplateIdSyntax * template_id_syntax = cppast_template_id_syntax(expr);
    if(template_id_syntax) {
      const StructuredTypeLookupResult structured_result =
          resolve_structured_type_lookup_node(services,
                                              scope,
                                              expr,
                                              false,
                                              parser_trace::current_use_location(),
                                              out);
      if(structured_result == StructuredTypeLookupResult::Resolved && out) {
        return true;
      }
      if(structured_result == StructuredTypeLookupResult::NoMatch) {
        return false;
      }
    }

    const QualifiedName * qualified_type_name = qualified_syntax_if_qualified(expr);
    if((qualified_type_name ?
            resolve_direct_type_name_lookup(services,
                                            scope,
                                            *qualified_type_name,
                                            false,
                                            parser_trace::current_use_location(),
                                            out) :
            resolve_direct_type_name_lookup(services,
                                            scope,
                                            expr.value,
                                            false,
                                            parser_trace::current_use_location(),
                                            out)) &&
       out) {
      return true;
    }
    return false;
  }

  if(expr.kind != CppAstKind::member_expression ||
     expr.children.size() != 2 ||
     expr.children[1].kind != CppAstKind::identifier) {
    return false;
  }

  TypePtr base_type;
  if(!lookup_leaf_operand_type(services, scope, expr.children[0], base_type) || !base_type) {
    return false;
  }

  TypePtr member_base = strip_top_level_cv(remove_reference_type(base_type));
  if(!member_base) {
    return false;
  }
  if(node_has_simple_type(expr, OP_ARROW)) {
    if(member_base->kind != Type::TK_POINTER) {
      return false;
    }
    member_base = strip_top_level_cv(remove_reference_type(member_base->inner));
  } else if(!node_has_simple_type(expr, OP_DOT)) {
    return false;
  }
  if(!member_base) {
    return false;
  }

  Scope * member_scope = nullptr;
  if(!prepare_concrete_type_member_scope(services,
                                         template_api::make_template_environment(scope),
                                         member_base,
                                         member_scope)) {
    return false;
  }

  return service_lookup_leaf_member_expression_type_in_scope(
      services, *member_scope, expr.children[1].value, out);
}

bool ast_has_parameter_pack_node(const CppAstNode & node)
{
  if(node.kind == CppAstKind::parameter_pack ||
     node.kind == CppAstKind::pack_expansion_expression) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_has_parameter_pack_node(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool declarator_is_bare_parameter_pack(const CppAstNode & node)
{
  return (node.kind == CppAstKind::declarator ||
          node.kind == CppAstKind::abstract_declarator) &&
         node.children.size() == 1 &&
         node.children[0].kind == CppAstKind::parameter_pack;
}

bool simple_type_pack_name_from_type_id(const CppAstNode & type_id, string & out)
{
  out.clear();
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.size() != 2 ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq ||
     type_id.children[0].children.size() != 1 ||
     type_id.children[0].children[0].kind != CppAstKind::type_name ||
     !declarator_is_bare_parameter_pack(type_id.children[1])) {
    return false;
  }
  out = type_id.children[0].children[0].value;
  return !out.empty();
}

bool expand_builtin_trait_type_arg_ast(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const CppAstNode & arg,
                                       vector<TypePtr> & types)
{
  types.clear();
  if(arg.kind != CppAstKind::type_id) {
    return false;
  }

  if(ast_has_parameter_pack_node(arg)) {
    string simple_pack_name;
    if(simple_type_pack_name_from_type_id(arg, simple_pack_name)) {
      const vector<TypePtr> * direct_pack = lookup_type_pack(scope, simple_pack_name);
      if(direct_pack) {
        types = *direct_pack;
        return true;
      }
    }

    string arg_text = node_text(arg);
    if(arg_text.empty()) {
      return false;
    }
    const string trimmed = trim_space(arg_text);
    if(trimmed.size() > 3 && trimmed.substr(trimmed.size() - 3) == "...") {
      const string pattern = trim_space(trimmed.substr(0, trimmed.size() - 3));
      const vector<TypePtr> * direct_pack = lookup_type_pack(scope, pattern);
      if(direct_pack) {
        types = *direct_pack;
        return true;
      }
    }

    const vector<ExpandedTypeArgumentInput> expanded_inputs =
        expand_bound_type_pack_arguments(services, scope, vector<string>(1, arg_text));
    if(expanded_inputs.empty()) {
      return true;
    }
    if(expanded_inputs.size() != 1 || trim_space(expanded_inputs[0].text) != trimmed) {
      for(size_t i = 0; i < expanded_inputs.size(); ++i) {
        if(expanded_inputs[i].type) {
          types.push_back(expanded_inputs[i].type);
        } else {
          types.clear();
          return false;
        }
      }
      return true;
    }
  }

  TypePtr type;
  if(!parse_type_id_node_for_templates(services, scope, arg, type, true) ||
     !type) {
    return false;
  }
  types.push_back(type);
  return true;
}

bool try_parse_builtin_type_trait_expression_ast(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & expr,
    string & trait_name,
    vector<TypePtr> & types)
{
  trait_name.clear();
  types.clear();
  if(expr.kind == CppAstKind::call_expression) {
    if(expr.children.size() != 2 ||
       expr.children[0].kind != CppAstKind::id_expression ||
       (expr.children[1].kind != CppAstKind::argument_list &&
        expr.children[1].kind != CppAstKind::paren_argument_list) ||
       expr.children[1].children.empty()) {
      return false;
    }
    trait_name = expr.children[0].value;
    if(!semantic_builtins::is_supported_builtin_type_trait_name(trait_name)) {
      trait_name.clear();
      return false;
    }
    for(size_t i = 0; i < expr.children[1].children.size(); ++i) {
      vector<TypePtr> expanded;
      if(!expand_builtin_trait_type_arg_ast(
             services, scope, expr.children[1].children[i], expanded)) {
        trait_name.clear();
        types.clear();
        return false;
      }
      types.insert(types.end(), expanded.begin(), expanded.end());
    }
    return true;
  }

  if(expr.kind != CppAstKind::type_trait_expression ||
     !expr.has_token ||
     expr.token_kind != RT_IDENTIFIER ||
     !semantic_builtins::is_supported_builtin_type_trait_name(expr.value)) {
    return false;
  }

  trait_name = expr.value;
  for(size_t i = 0; i < expr.children.size(); ++i) {
    vector<TypePtr> expanded;
    if(!expand_builtin_trait_type_arg_ast(services, scope, expr.children[i], expanded)) {
      trait_name.clear();
      types.clear();
      return false;
    }
    types.insert(types.end(), expanded.begin(), expanded.end());
  }
  return true;
}

bool evaluate_leaf_special_expression(template_api::TemplateServices & services,
                                      Scope & scope,
                                      constant_eval::Evaluator & evaluator,
                                      const CppAstNode & expr,
                                      constant_eval::ConstexprValue & value)
{
  if(expr.kind == CppAstKind::type_trait_expression ||
     expr.kind == CppAstKind::call_expression) {
    string builtin_name;
    vector<TypePtr> builtin_types;
    bool parsed_builtin_trait =
        try_parse_builtin_type_trait_expression_ast(
            services, scope, expr, builtin_name, builtin_types);
    if(parsed_builtin_trait) {
      long long trait_value = 0;
      if(evaluate_builtin_type_trait(
             services, scope, builtin_name, builtin_types, trait_value)) {
        value = constant_eval::make_integral_value(
            trait_value,
            semantic_builtins::builtin_type_trait_result_type(builtin_name));
        return true;
      }
    }
  }

  if(expr.kind == CppAstKind::call_expression && !expr.children.empty()) {
    const CppAstNode * callee = unwrap_call_callee(expr);
    const CppAstNode * argument_list =
        find_child_kind(expr, CppAstKind::argument_list);
    if(!argument_list) {
      argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
    }
	    if(callee &&
	       callee->kind == CppAstKind::id_expression &&
	       (!argument_list ||
	        argument_list->children.empty() ||
	        (argument_list->children.size() == 1 &&
	         argument_list->children[0].kind == CppAstKind::braced_init_list &&
	         argument_list->children[0].children.empty()))) {
      const TemplateIdSyntax * template_id = cppast_template_id_syntax(*callee);
      const QualifiedName * qualified = qualified_syntax_if_qualified(*callee);
      const QualifiedName * template_qualified =
          template_id &&
          (template_id->name.rooted || !template_id->name.qualifiers.empty()) ?
              &template_id->name :
              qualified;
      vector<FunctionBinding *> functions;
      vector<FunctionTemplateDecl *> function_templates;
      if(template_qualified &&
         ((lookup_leaf_qualified_function_bindings(
               services, scope, *template_qualified, callee, functions) &&
           !functions.empty()) ||
          (lookup_leaf_qualified_function_templates(
               services, scope, *template_qualified, callee, function_templates) &&
           !function_templates.empty()))) {
        return false;
      }
      if(template_id) {
        return false;
      }
      if(lookup_leaf_constant_value(scope, services, callee->value + "::value", value)) {
        return true;
      }
	    }
	  }

  if(expr.kind == CppAstKind::member_expression &&
     expr.children.size() == 2 &&
     (expr.children[1].kind == CppAstKind::id_expression ||
      expr.children[1].kind == CppAstKind::identifier)) {
    TypePtr base_type;
    if(lookup_leaf_operand_type(services, scope, expr.children[0], base_type) &&
       base_type) {
      TypePtr member_base = strip_top_level_cv(remove_reference_type(base_type));
      if(member_base) {
        if(node_has_simple_type(expr, OP_ARROW)) {
          if(member_base->kind == Type::TK_POINTER) {
            member_base =
                strip_top_level_cv(remove_reference_type(member_base->inner));
          } else {
            member_base.reset();
          }
        } else if(!node_has_simple_type(expr, OP_DOT)) {
          member_base.reset();
        }
        if(member_base) {
          ClassInfo * info = template_api::find_named_type_class_info(
              service_type_system(services).model,
              member_base);
          if(info &&
             info->member_scope &&
             service_lookup_leaf_member_expression_value_in_scope(
                 services, *info->member_scope, expr.children[1].value, value)) {
            return true;
          }
        }
      }
    }

    constant_eval::ConstexprValue base;
    if(expr.children[0].kind == CppAstKind::id_expression &&
       expr.children[0].value == "this") {
      if(!evaluator.current_this_object(base)) {
        return false;
      }
    } else if(!evaluator.eval_expr(expr.children[0], base)) {
      return false;
    }
    return constant_eval::aggregate_member_value(
        base, expr.children[1].value, value);
  }

  if(expr.kind == CppAstKind::subscript_expression && expr.children.size() == 2) {
    constant_eval::ConstexprValue base;
    constant_eval::ConstexprValue index;
    if(!evaluator.eval_expr(expr.children[0], base) ||
       !evaluator.eval_expr(expr.children[1], index)) {
      return false;
    }
    long long integral_index = 0;
    if(!constant_eval::constexpr_value_to_integral(index, integral_index) ||
       integral_index < 0) {
      return false;
    }
    return constant_eval::array_element_value(
        base, static_cast<size_t>(integral_index), value);
  }

  return false;
}

constant_eval::Hooks build_leaf_constant_eval_hooks(template_api::TemplateServices & services,
                                                    Scope & scope)
{
  constant_eval::Hooks hooks;
  hooks.lookup_external_value =
      [&services, &scope](const string & name,
                          const CppAstNode * node,
                          constant_eval::ConstexprValue & value)
      {
        return lookup_leaf_constant_value(scope, services, name, node, value);
      };
  hooks.lookup_pack_size =
      [&scope](const string & name, size_t & pack_size)
      {
        return lookup_pack_size(scope, name, pack_size);
      };
  hooks.lookup_type =
      [&services, &scope](const string & name)
      {
        TypePtr type;
        resolve_direct_type_name_lookup(
            services, scope, name, false, std::string(), type);
        return type;
      };
  hooks.lookup_type_node =
      [&services, &scope](const CppAstNode & node)
      {
        if(leaf_id_expression_names_function_or_template(services, scope, node)) {
          return TypePtr();
        }
        const string lookup_name =
            node.kind == CppAstKind::type_name && !node.value.empty() ?
                node.value :
                node_text(node);
        const string repaired_lookup_name =
            repair_compacted_template_argument_expression_spacing(lookup_name);
        const TemplateIdSyntax * template_id = cppast_template_id_syntax(node);
        const string template_location =
            template_id ?
                template_api::normalize_template_witness_source_location(
                    template_api::template_witness_detail::source_location_for_location_id(
                        services.witness_context,
                        template_id->source_location_id)) :
                string();
        TypePtr type;
        const StructuredTypeLookupResult qualified_template_result =
            resolve_qualified_template_type_lookup_node(services,
                                                        scope,
                                                        repaired_lookup_name,
                                                        node,
                                                        false,
                                                        template_location,
                                                        type);
        if(qualified_template_result == StructuredTypeLookupResult::Resolved &&
           type) {
          return type;
        }
        if(qualified_template_result == StructuredTypeLookupResult::NoMatch) {
          return TypePtr();
        }
        const StructuredTypeLookupResult structured_result =
            resolve_structured_type_lookup_node(services,
                                                scope,
                                                node,
                                                false,
                                                template_location,
                                                type);
        if(structured_result == StructuredTypeLookupResult::Resolved && type) {
          return type;
        }
        if(structured_result == StructuredTypeLookupResult::NoMatch) {
          return TypePtr();
        }
        if(const QualifiedName * qualified = cppast_qualified_name_syntax(node)) {
          resolve_direct_type_name_lookup(
              services, scope, *qualified, false, template_location, type);
        } else {
          resolve_direct_type_name_lookup(
              services, scope, repaired_lookup_name, false, template_location, type);
        }
        return type;
      };
  hooks.parse_type_id =
      [&services, &scope](const CppAstNode & type_id, TypePtr & type)
      {
        if(type_id.kind != CppAstKind::type_id ||
           !template_decl_ast::parse_type_id(
               services,
               scope,
               scope,
               type_id,
               type,
               false) ||
           !type) {
          return false;
        }
        if(services.semantic_context) {
          TypePtr sizeof_type = remove_reference_type(type);
          if(!sizeof_type) {
            sizeof_type = type;
          }
          callsemantic_internal::maybe_complete_sizeof_type(
              *services.semantic_context, sizeof_type);
        }
        return true;
      };
  hooks.evaluate_sizeof_operand =
      [&services, &scope](const CppAstNode & expr, size_t & size)
      {
        TypePtr type;
        semantic_conversion::ValueCategory category = semantic_conversion::VC_PRVALUE;
        if(!lookup_leaf_expression_type_category(services, scope, expr, type, category) ||
           !type) {
          return false;
        }
        TypePtr sizeof_type = remove_reference_type(type);
        if(!sizeof_type) {
          sizeof_type = type;
        }
        if(services.semantic_context) {
          callsemantic_internal::maybe_complete_sizeof_type(
              *services.semantic_context, sizeof_type);
        }
        size = type_size(sizeof_type);
        return true;
      };
  hooks.evaluate_special_expression =
      [&services, &scope](constant_eval::Evaluator & evaluator,
                          const CppAstNode & expr,
                          constant_eval::ConstexprValue & value)
      {
        return evaluate_leaf_special_expression(
            services, scope, evaluator, expr, value);
      };
  hooks.evaluate_call =
      [&services, &scope](constant_eval::Evaluator & evaluator,
                          const CppAstNode & expr,
                          const vector<constant_eval::ConstexprValue> & args,
                          constant_eval::ConstexprValue & value)
      {
        return evaluate_leaf_constexpr_function_call(
            services, scope, evaluator, expr, args, value);
      };
  hooks.evaluate_typed_initializer =
      [&services, &scope](constant_eval::Evaluator & evaluator,
                          const CppAstNode & node,
                          const TypePtr & target,
                          constant_eval::ConstexprValue & value)
      {
        return evaluate_leaf_typed_initializer(
            services, scope, evaluator, node, target, value);
      };
  return hooks;
}

bool evaluate_constant_expression_leaf_impl(template_api::TemplateServices & services,
                                            Scope & scope,
                                            const CppAstNode & node,
                                            constant_eval::ConstexprValue & out,
                                            const TypePtr & target_type)
{
  constant_eval::Evaluator evaluator(build_leaf_constant_eval_hooks(services, scope));
  return evaluator.eval_initializer(node, out, target_type);
}

bool constexpr_value_to_template_argument_integral(const TypePtr & target_type,
                                                   const constant_eval::ConstexprValue & value,
                                                   long long & out)
{
  if(target_type) {
    TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
    if(target_base &&
       is_integral_type(target_base) &&
       type_size(target_base) <= sizeof(unsigned long long) &&
       value.kind == constant_eval::ConstexprValue::CV_INTEGRAL) {
      if(is_unsigned_integral_type(target_base)) {
        unsigned long long unsigned_value = 0;
        if(constant_eval::constexpr_value_to_unsigned_integral(value, unsigned_value)) {
          out = static_cast<long long>(unsigned_value);
          return true;
        }
      } else if(constant_eval::constexpr_value_to_integral(value, out)) {
        return true;
      }
    }
  }
  return constant_eval::constexpr_value_to_integral(value, out);
}

bool is_data_member_pointer_template_argument_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base &&
         base->kind == Type::TK_MEMBER_POINTER &&
         !is_function_type(base->inner);
}

bool expression_is_data_member_pointer_address_constant(const CppAstNode & expr)
{
  if(expr.kind == CppAstKind::parenthesized_expression &&
     expr.children.size() == 1) {
    return expression_is_data_member_pointer_address_constant(expr.children[0]);
  }
  if(expr.kind == CppAstKind::cast_expression &&
     expr.children.size() == 2 &&
     expr.children[0].kind == CppAstKind::type_id) {
    return expression_is_data_member_pointer_address_constant(expr.children[1]);
  }
  return expr.kind == CppAstKind::unary_expression &&
         expr.children.size() == 1 &&
         node_has_simple_type(expr, OP_AMP);
}

bool encode_data_member_pointer_template_argument_if_needed(
    const TypePtr & target_type,
    const CppAstNode & expr,
    long long & value)
{
  if(!is_data_member_pointer_template_argument_type(target_type) ||
     !expression_is_data_member_pointer_address_constant(expr)) {
    return true;
  }
  if(value < 0 || value == std::numeric_limits<long long>::max()) {
    return false;
  }
  ++value;
  return true;
}

TemplateArgumentSyntax clone_argument_syntax_for_template_substitution(
    const TemplateArgumentSyntax & source);

bool expand_bound_packs_in_argument_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    TemplateArgumentSyntax & syntax);

}  // namespace

void note_structured_bool_value_member_for_type_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type)
{
  note_structured_bool_value_member_if_needed(services, scope, type);
}

void note_template_value_dependencies_for_witness(
    const vector<TemplateValueDependency> & dependencies)
{
  for(size_t i = 0; i < dependencies.size(); ++i) {
    note_template_value_dependency_for_witness(dependencies[i]);
  }
}

void note_template_value_dependencies_for_witness(
    SemanticContext & ctx,
    const vector<TemplateValueDependency> & dependencies)
{
  for(size_t i = 0; i < dependencies.size(); ++i) {
    note_template_value_dependency_for_witness(dependencies[i], &ctx);
  }
}

void append_structured_bool_value_dependencies_in_expression_ast(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    vector<TemplateValueDependency> & out)
{
  append_structured_bool_value_dependencies_in_expression_ast_impl(
      services,
      scope,
      node,
      out);
}

void append_structured_bool_value_dependencies_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    vector<TemplateValueDependency> & out)
{
  TemplateArgumentSyntax expanded_syntax;
  const TemplateArgumentSyntax * effective_syntax = &syntax;
  if(syntax.text.find("...") != string::npos ||
     syntax.pack_expansion ||
     syntax.template_id ||
     syntax.type_id ||
     syntax.expression) {
    expanded_syntax = clone_argument_syntax_for_template_substitution(syntax);
    if(expand_bound_packs_in_argument_syntax(
           services, scope.require(), expanded_syntax)) {
      effective_syntax = &expanded_syntax;
    }
  }
  if(template_argument_syntax_has_template_dependency(services,
                                                     scope,
                                                     *effective_syntax)) {
    return;
  }
  append_structured_bool_value_dependencies_in_argument_syntax(
      services,
      scope,
      *effective_syntax,
      out);
}

void append_non_bool_static_value_dependencies_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    const TypePtr & bound_value_type,
    vector<TemplateValueDependency> & out)
{
  TemplateArgumentSyntax expanded_syntax;
  const TemplateArgumentSyntax * effective_syntax = &syntax;
  if(syntax.text.find("...") != string::npos ||
     syntax.pack_expansion ||
     syntax.template_id ||
     syntax.type_id ||
     syntax.expression) {
    expanded_syntax = clone_argument_syntax_for_template_substitution(syntax);
    if(expand_bound_packs_in_argument_syntax(
           services, scope.require(), expanded_syntax)) {
      effective_syntax = &expanded_syntax;
    }
  }
  append_non_bool_static_value_dependencies_in_template_argument_syntax_impl(
      services,
      scope,
      *effective_syntax,
      bound_value_type,
      out);
}

void note_structured_bool_value_dependencies_for_class_info(
    template_api::TemplateServices & services,
    const ClassInfo & info)
{
  set<const ClassInfo *> visiting;
  note_structured_bool_value_dependencies_for_class_info(
      services,
      info,
      visiting);
}

bool structured_bool_constant_value_for_type(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    bool & out)
{
  set<string> visiting;
  return structured_bool_constant_value_for_type(type_system, type, out, visiting);
}

bool structured_bool_constant_value_for_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    bool & out)
{
  set<string> visiting;
  return structured_bool_constant_value_for_type(
      service_type_system(services),
      type,
      out,
      visiting,
      &services,
      scope);
}

bool lookup_type_member_constant_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    const string & member_name,
    constant_eval::ConstexprValue & out)
{
  out = constant_eval::ConstexprValue();
  if(!type || member_name.empty()) {
    return false;
  }

  if(is_structured_bool_result_member_name(member_name)) {
    bool structured_value = false;
    bool structured_evaluation_incomplete = false;
    set<string> visiting;
    if(structured_bool_constant_value_for_type(
           service_type_system(services),
           type,
           structured_value,
           visiting,
           &services,
           scope,
           &structured_evaluation_incomplete)) {
      out = constant_eval::make_integral_value(
          structured_value ? 1 : 0,
          make_fundamental(FT_BOOL));
      note_structured_bool_value_member_if_needed(services, scope, type);
      return true;
    }
    if(structured_evaluation_incomplete) {
      return false;
    }
  }

  Scope * member_scope = nullptr;
  if(!prepare_concrete_type_member_scope(services, scope, type, member_scope) ||
     !member_scope) {
    return false;
  }

  if(!service_lookup_leaf_member_expression_value_in_scope(
         services, *member_scope, member_name, out)) {
    return false;
  }

  if(is_structured_bool_result_member_name(member_name)) {
    note_structured_bool_value_member_if_needed(services, scope, type);
  }
  return true;
}

bool structured_bool_constant_value_for_class_info(
    template_api::TemplateTypeSystem & type_system,
    const ClassInfo & info,
    bool & out)
{
  set<string> visiting;
  return structured_bool_constant_value_for_class_info(
      type_system, info, out, visiting);
}

bool structured_bool_constant_value_for_class_info(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const ClassInfo & info,
    bool & out)
{
  set<string> visiting;
  return structured_bool_constant_value_for_class_info(
      service_type_system(services),
      info,
      out,
      visiting,
      &services,
      scope);
}

string lookup_text_for_type_argument(SemanticContext & ctx, const TypePtr & type);
bool type_depends_on_template_parameter(SemanticContext & ctx, const TypePtr & type);
bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const TypePtr & type);
bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         TypePtr & out);
bool resolve_template_template_argument_text(SemanticContext & ctx,
                                             Scope & scope,
                                             const string & text,
                                             size_t expected_parameter_count,
                                             bool allow_dependent_placeholders,
                                             TemplateArgument & out);
NonTypeArgumentStatus evaluate_non_type_argument_text(
    SemanticContext & ctx,
    Scope & scope,
    const string & text,
    long long & value,
    string * eval_error = nullptr,
    const TypePtr & target_type = TypePtr());
NonTypeArgumentStatus evaluate_template_member_value_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text,
    long long & value,
    const TypePtr & target_type);
string lookup_text_for_non_type_template_argument(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    long long value);

namespace {

template<class BuildFn>
void note_template_trace_if_enabled(BuildFn build)
{
  if(!parser_trace::enabled("template.resolve")) {
    return;
  }
  ostringstream trace;
  build(trace);
  parser_trace::note("template.resolve", string(), trace.str());
}

const TemplateParameterInfo * find_substitution_parameter(
    const vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return nullptr;
  }

  string parameter_key = type->named_key;
  const bool is_dependent_alias = named_type_is_dependent_alias(type);
  if(is_dependent_alias || named_type_is_dependent_type(type)) {
    const string semantic_payload = named_type_semantic_payload(type);
    if(!semantic_payload.empty()) {
      parameter_key = semantic_payload;
    }
  }

  const TemplateParameterInfo * parameter = find_template_parameter(parameters, parameter_key);
  if(!parameter && !is_dependent_alias && type->named_display != type->named_key) {
    parameter = find_template_parameter(parameters, type->named_display);
  }
  return parameter;
}

TypePtr collapse_lvalue_reference_type(const TypePtr & inner);
TypePtr collapse_rvalue_reference_type(const TypePtr & inner);
CppAstNode make_substituted_type_id_node(const TypePtr & type,
                                         const string & text);
CppAstNode make_substituted_value_expression_node(const ValueBinding & binding);
bool expression_node_mentions_identifier(const CppAstNode & node,
                                         const string & name);
bool argument_syntax_mentions_identifier(const TemplateArgumentSyntax & syntax,
                                         const string & name);
string replace_identifier_token_text(const string & text,
                                     const string & name,
                                     const string & replacement,
                                     bool & changed);
string replace_identifier_token_text_preserving_sizeof_pack_operands(
    const string & text,
    const string & name,
    const string & replacement,
    bool & changed);
string replace_sizeof_pack_count_text(const string & text,
                                      const map<string, size_t> & pack_size_replacements,
                                      bool & changed);
bool substitute_sizeof_pack_count_expression_node(
    const CppAstNode & node,
    const map<string, size_t> & pack_size_replacements,
    CppAstNode & out,
    bool & changed);
void substitute_sizeof_pack_counts_template_id_arguments(
    TemplateIdSyntax & syntax,
    const map<string, size_t> & pack_size_replacements);
string substituted_value_pack_argument_text(const ValueBinding & binding);
bool lookup_pack_size(Scope & scope, const string & name, size_t & out);
void substitute_type_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    Scope & scope,
    const map<string, TypePtr> & type_replacements);
void substitute_value_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    const map<string, ValueBinding> & value_replacements);

bool is_dependent_builtin_type_transform_type(const TypePtr & type)
{
  if(!type ||
     !named_type_is_dependent_type(type) ||
     !type->inner) {
    return false;
  }

  static const char prefix[] = "$builtin-type-transform:";
  const string payload = named_type_semantic_payload(type);
  return payload.compare(0, sizeof(prefix) - 1, prefix) == 0;
}

bool substitute_dependent_alias_type(const TypePtr & type,
                                     const vector<TemplateParameterInfo> & parameters,
                                     const vector<TemplateArgument> & arguments,
                                     Scope * scope,
                                     TypePtr & out);

bool substitute_type_impl(const TypePtr & type,
                          const vector<TemplateParameterInfo> & parameters,
                          const vector<TemplateArgument> & arguments,
                          Scope * scope,
                          TypePtr & out);

size_t remaining_non_pack_parameter_count(
    const vector<TemplateParameterInfo> & parameters,
    size_t start)
{
  size_t count = 0;
  for(size_t i = start; i < parameters.size(); ++i) {
    if(!parameters[i].parameter_pack) {
      ++count;
    }
  }
  return count;
}

bool dependent_argument_is_pack_expansion(
    const DependentAliasTemplateArgumentSyntax & argument)
{
  const string text = trim_space(argument.text);
  return argument.syntax.pack_expansion ||
         (text.size() >= 3 && text.substr(text.size() - 3) == "...");
}

bool template_parameter_argument_count(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    size_t parameter_index,
    Scope * scope,
    size_t & argument_count)
{
  size_t arg_index = 0;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      const size_t trailing_non_pack =
          remaining_non_pack_parameter_count(parameters, i + 1);
      if(arguments.size() < arg_index + trailing_non_pack) {
        return false;
      }
      size_t pack_count = 0;
      if(scope &&
         !parameters[i].name.empty() &&
         lookup_pack_size(*scope, parameters[i].name, pack_count)) {
        if(arguments.size() < arg_index + pack_count + trailing_non_pack) {
          return false;
        }
      } else {
        pack_count = arguments.size() - arg_index - trailing_non_pack;
      }
      if(i == parameter_index) {
        argument_count = pack_count;
        return true;
      }
      arg_index += pack_count;
      continue;
    }

    if(arg_index >= arguments.size()) {
      return false;
    }
    if(i == parameter_index) {
      argument_count = 1;
      return true;
    }
    ++arg_index;
  }
  return false;
}

const vector<ValueBinding> * lookup_value_pack(Scope & scope, const string & name)
{
  const string trimmed = trim_space(name);
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    map<string, vector<ValueBinding> >::const_iterator found =
        current->named_value_packs.find(trimmed);
    if(found != current->named_value_packs.end()) {
      return &found->second;
    }
  }
  return nullptr;
}

bool direct_value_pack_name_from_argument(
    const DependentAliasTemplateArgumentSyntax & argument,
    string & out)
{
  out.clear();
  const auto try_text =
      [&out](string text) -> bool
      {
        text = trim_space(text);
        if(text.size() >= 3 && text.substr(text.size() - 3) == "...") {
          text = trim_space(text.substr(0, text.size() - 3));
        }
        if(!text.empty() && is_identifier_text(text)) {
          out = text;
          return true;
        }
        return false;
      };
  if(try_text(argument.text) ||
     try_text(argument.syntax.text)) {
    return true;
  }
  if(argument.syntax.expression &&
     argument.syntax.expression->kind == CppAstKind::id_expression &&
     try_text(argument.syntax.expression->value)) {
    return true;
  }
  if(argument.syntax.type_id &&
     try_text(argument.syntax.type_id->value)) {
    return true;
  }
  return false;
}

bool build_pack_element_substitution_arguments(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    size_t pack_parameter_index,
    size_t pack_element_index,
    Scope * scope,
    vector<TemplateArgument> & out)
{
  out.clear();
  out.resize(parameters.size());
  size_t arg_index = 0;
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      const size_t trailing_non_pack =
          remaining_non_pack_parameter_count(parameters, i + 1);
      if(arguments.size() < arg_index + trailing_non_pack) {
        return false;
      }
      size_t pack_count = 0;
      if(scope &&
         !parameters[i].name.empty() &&
         lookup_pack_size(*scope, parameters[i].name, pack_count)) {
        if(arguments.size() < arg_index + pack_count + trailing_non_pack) {
          return false;
        }
      } else {
        pack_count = arguments.size() - arg_index - trailing_non_pack;
      }
      if(i == pack_parameter_index) {
        if(pack_element_index >= pack_count) {
          return false;
        }
        out[i] = arguments[arg_index + pack_element_index];
      } else if(pack_count == 1) {
        out[i] = arguments[arg_index];
      }
      arg_index += pack_count;
      continue;
    }

    if(arg_index >= arguments.size()) {
      return false;
    }
    out[i] = arguments[arg_index++];
  }
  return true;
}

void collect_template_parameter_pack_size_replacements(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    map<string, size_t> & out)
{
  out.clear();
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].parameter_pack || parameters[i].name.empty()) {
      continue;
    }
    size_t argument_count = 0;
    if(template_parameter_argument_count(parameters,
                                         arguments,
                                         i,
                                         nullptr,
                                         argument_count)) {
      out[parameters[i].name] = argument_count;
    }
  }
}

bool type_mentions_substitution_parameter(
    const TypePtr & type,
    const vector<TemplateParameterInfo> & parameters,
    const TemplateParameterInfo & parameter)
{
  if(!type) {
    return false;
  }

  const TemplateParameterInfo * found = find_substitution_parameter(parameters, type);
  if(found && found->name == parameter.name) {
    return true;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;

  case Type::TK_NAMED:
    for(size_t i = 0; i < type->named_dependent_alias_arguments.size(); ++i) {
      if(type_mentions_substitution_parameter(
             type->named_dependent_alias_arguments[i].type,
             parameters,
             parameter)) {
        return true;
      }
    }
    for(size_t i = 0; i < type->named_dependent_class_arguments.size(); ++i) {
      if(type_mentions_substitution_parameter(
             type->named_dependent_class_arguments[i].type,
             parameters,
             parameter)) {
        return true;
      }
    }
    return type_mentions_substitution_parameter(
               type->named_dependent_qualified_owner,
               parameters,
               parameter) ||
           type_mentions_substitution_parameter(type->inner, parameters, parameter) ||
           type_mentions_substitution_parameter(type->owner, parameters, parameter);

  case Type::TK_ATOMIC:
  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_mentions_substitution_parameter(type->inner, parameters, parameter);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_substitution_parameter(type->owner, parameters, parameter) ||
           type_mentions_substitution_parameter(type->inner, parameters, parameter);

  case Type::TK_FUNCTION:
    if(type_mentions_substitution_parameter(type->inner, parameters, parameter)) {
      return true;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(type_mentions_substitution_parameter(type->params[i],
                                              parameters,
                                              parameter)) {
        return true;
      }
    }
    return false;
  }
  return false;
}

bool find_pack_expansion_substitution_parameter(
    const TypePtr & pattern,
    const vector<TemplateParameterInfo> & parameters,
    size_t & parameter_index)
{
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack &&
       type_mentions_substitution_parameter(pattern, parameters, parameters[i])) {
      parameter_index = i;
      return true;
    }
  }
  return false;
}

string carried_dependent_class_template_argument_text(const TypePtr & type)
{
  void * class_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_class_template(type, class_template_decl, arguments)) {
    return string();
  }
  ClassTemplateDecl * class_template =
      static_cast<ClassTemplateDecl *>(class_template_decl);
  if(!class_template) {
    return string();
  }

  const string template_name =
      class_template->declaring_scope ?
          semantic_lookup::scope_qualified_name(*class_template->declaring_scope,
                                                class_template->name) :
          class_template->name;
  if(template_name.empty()) {
    return string();
  }

  ostringstream out;
  out << template_name << "<";
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    string argument_text = trim_space(arguments[i].text);
    if(argument_text.empty() && arguments[i].type) {
      argument_text = trim_space(reparseable_type_argument_text(arguments[i].type));
    }
    if(argument_text.empty()) {
      return string();
    }
    out << argument_text;
  }
  out << ">";
  return out.str();
}

string substituted_dependent_argument_type_text(const TypePtr & type)
{
  const string carried_class_text =
      carried_dependent_class_template_argument_text(type);
  if(!carried_class_text.empty()) {
    return trim_space(carried_class_text);
  }
  return trim_space(reparseable_type_argument_text(type));
}

void update_dependent_argument_with_type(
    DependentAliasTemplateArgumentSyntax & argument,
    const TypePtr & type)
{
  argument.type = type;
  const string argument_text = substituted_dependent_argument_type_text(type);
  if(!argument_text.empty()) {
    if(argument.syntax.source_text.empty()) {
      argument.syntax.source_text = argument.syntax.text;
    }
    argument.text = argument_text;
    argument.syntax.text = argument_text;
    argument.syntax.type_id.reset(
        new CppAstNode(make_substituted_type_id_node(type, argument_text)));
    argument.syntax.template_id.reset();
    argument.syntax.expression.reset();
  }
  argument.syntax.pack_expansion = false;
  argument.syntax.dependent = false;
  argument.syntax.resolved_type = type;
}

bool argument_syntax_mentions_any_substitution_parameter(
    const TemplateArgumentSyntax & syntax,
    const vector<TemplateParameterInfo> & parameters)
{
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() &&
       argument_syntax_mentions_identifier(syntax, parameters[i].name)) {
      return true;
    }
  }
  return false;
}

void collect_type_pack_replacements_for_substitution(
    Scope * scope,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    map<string, vector<TypePtr> > & out)
{
  out.clear();
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].parameter_pack ||
       parameters[i].name.empty() ||
       parameters[i].kind != TemplateParameterInfo::TP_TYPE) {
      continue;
    }
    vector<TypePtr> pack;
    if(scope) {
      if(const vector<TypePtr> * bound_pack =
             lookup_type_pack(*scope, parameters[i].name)) {
        pack = *bound_pack;
      }
    }
    if(pack.empty()) {
      size_t pack_count = 0;
      if(!template_parameter_argument_count(parameters,
                                            arguments,
                                            i,
                                            scope,
                                            pack_count)) {
        continue;
      }
      for(size_t pack_index = 0; pack_index < pack_count; ++pack_index) {
        vector<TemplateArgument> element_arguments;
        if(!build_pack_element_substitution_arguments(parameters,
                                                      arguments,
                                                      i,
                                                      pack_index,
                                                      scope,
                                                      element_arguments) ||
           i >= element_arguments.size() ||
           element_arguments[i].kind != TemplateArgument::TA_TYPE ||
           !element_arguments[i].type) {
          pack.clear();
          break;
        }
        pack.push_back(element_arguments[i].type);
      }
    }
    if(!pack.empty()) {
      out[parameters[i].name] = pack;
    }
  }
}

bool direct_type_pack_expansion_argument(const TemplateArgumentSyntax & argument,
                                         string & pack_name)
{
  pack_name.clear();
  if(argument.type_id &&
     simple_type_pack_name_from_type_id(*argument.type_id, pack_name)) {
    return true;
  }
  if(!argument.pack_expansion) {
    return false;
  }
  if(argument.type_id) {
    const CppAstNode * type_name = single_type_name_child(*argument.type_id);
    if(type_name && !type_name->value.empty()) {
      pack_name = type_name->value;
      return true;
    }
  }
  if(argument.expression &&
     argument.expression->kind == CppAstKind::id_expression &&
     !argument.expression->value.empty()) {
    pack_name = argument.expression->value;
    return true;
  }
  if(!argument.template_id ||
     argument.template_id->name.name.empty()) {
    return false;
  }
  pack_name = argument.template_id->name.name;
  return true;
}

TemplateArgumentSyntax make_expanded_type_pack_argument_syntax(
    const TemplateArgumentSyntax & source,
    const TypePtr & type)
{
  TemplateArgumentSyntax out = source;
  const string text = reparseable_type_argument_text(type);
  if(out.source_text.empty()) {
    out.source_text = source.text;
  }
  out.text = text;
  out.pack_expansion = false;
  out.dependent = false;
  out.resolved_type = type;
  out.template_id.reset();
  out.expression.reset();
  out.type_id.reset(new CppAstNode(make_substituted_type_id_node(type, text)));
  return out;
}

bool expand_type_pack_template_id_syntax_arguments(
    TemplateIdSyntax & syntax,
    const map<string, vector<TypePtr> > & type_pack_replacements)
{
  bool changed = false;
  vector<string> expanded_arguments;
  vector<TemplateArgumentSyntax> expanded_syntaxes;
  const bool has_syntaxes = !syntax.argument_syntaxes.empty();
  const size_t count = has_syntaxes ? syntax.argument_syntaxes.size() :
                                      syntax.arguments.size();
  expanded_arguments.reserve(count);
  if(has_syntaxes) {
    expanded_syntaxes.reserve(count);
  }

  for(size_t i = 0; i < count; ++i) {
    TemplateArgumentSyntax argument;
    if(has_syntaxes) {
      argument = syntax.argument_syntaxes[i];
    } else {
      argument.text = i < syntax.arguments.size() ? syntax.arguments[i] :
                                                    string();
    }

    string pack_name;
    if(direct_type_pack_expansion_argument(argument, pack_name)) {
      map<string, vector<TypePtr> >::const_iterator found =
          type_pack_replacements.find(pack_name);
      if(found != type_pack_replacements.end()) {
        for(size_t j = 0; j < found->second.size(); ++j) {
          TemplateArgumentSyntax expanded =
              make_expanded_type_pack_argument_syntax(argument, found->second[j]);
          expanded_arguments.push_back(expanded.text);
          if(has_syntaxes) {
            expanded_syntaxes.push_back(expanded);
          }
        }
        changed = true;
        continue;
      }
    }

    if(argument.template_id &&
       expand_type_pack_template_id_syntax_arguments(
           *argument.template_id,
           type_pack_replacements)) {
      argument.text = template_id_syntax_lookup_text(*argument.template_id);
      changed = true;
    }
    expanded_arguments.push_back(argument.text);
    if(has_syntaxes) {
      expanded_syntaxes.push_back(argument);
    }
  }

  if(changed) {
    syntax.arguments.swap(expanded_arguments);
    if(has_syntaxes) {
      syntax.argument_syntaxes.swap(expanded_syntaxes);
    }
  }
  return changed;
}

bool expand_type_pack_ast_node_template_id_syntaxes(
    CppAstNode & node,
    const map<string, vector<TypePtr> > & type_pack_replacements)
{
  bool changed = false;
  if(node.template_id_syntax &&
     expand_type_pack_template_id_syntax_arguments(*node.template_id_syntax,
                                                   type_pack_replacements)) {
    node.value = template_id_syntax_lookup_text(*node.template_id_syntax);
    changed = true;
  }
  if(node.conversion_type_id_syntax &&
     expand_type_pack_ast_node_template_id_syntaxes(
         *node.conversion_type_id_syntax,
         type_pack_replacements)) {
    changed = true;
  }
  if(node.base_type_syntax &&
     expand_type_pack_ast_node_template_id_syntaxes(*node.base_type_syntax,
                                                    type_pack_replacements)) {
    changed = true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(expand_type_pack_template_id_syntax_arguments(
           node.qualifier_template_id_syntaxes[i],
           type_pack_replacements)) {
      if(node.qualified_name_syntax &&
         i < node.qualified_name_syntax->qualifiers.size()) {
        QualifiedName qualified = *node.qualified_name_syntax;
        qualified.qualifiers[i] =
            template_id_syntax_lookup_text(node.qualifier_template_id_syntaxes[i]);
        node.qualified_name_syntax.reset(new QualifiedName(qualified));
        node.value = template_api::qualified_name_text(qualified);
      }
      changed = true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(expand_type_pack_ast_node_template_id_syntaxes(
           node.qualifier_type_syntaxes[i],
           type_pack_replacements)) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(expand_type_pack_ast_node_template_id_syntaxes(
           node.exception_type_id_syntaxes[i],
           type_pack_replacements)) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    if(expand_type_pack_ast_node_template_id_syntaxes(
           node.alignment_specifier_nodes[i],
           type_pack_replacements)) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(expand_type_pack_ast_node_template_id_syntaxes(
           node.children[i],
           type_pack_replacements)) {
      changed = true;
    }
  }
  return changed;
}

bool expression_mentions_any_substitution_parameter(
    const CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters)
{
  for(size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() &&
       expression_node_mentions_identifier(node, parameters[i].name)) {
      return true;
    }
  }
  return false;
}

bool substitute_dependent_argument_expression(
    Scope * scope,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    DependentAliasTemplateArgumentSyntax & argument,
    bool & changed)
{
  changed = false;
  if(!scope ||
     !argument.syntax.expression ||
     !argument_syntax_mentions_any_substitution_parameter(argument.syntax,
                                                          parameters)) {
    return true;
  }

  CppAstNode rewritten;
  if(!substitute_expression_node_for_template_arguments(*scope,
                                                        *argument.syntax.expression,
                                                        parameters,
                                                        arguments,
                                                        rewritten)) {
    return false;
  }

  argument.syntax.expression.reset(new CppAstNode(rewritten));
  const string rewritten_text = trim_space(
      callsemantic_internal::describe_expression_for_diagnostic(rewritten));
  if(!rewritten_text.empty()) {
    if(argument.syntax.source_text.empty()) {
      argument.syntax.source_text = argument.syntax.text;
    }
    argument.text = rewritten_text;
    argument.syntax.text = rewritten_text;
  }
  argument.syntax.dependent =
      expression_mentions_any_substitution_parameter(rewritten, parameters);
  changed = true;
  return true;
}

bool substitute_dependent_argument_text_and_syntax(
    Scope * scope,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    DependentAliasTemplateArgumentSyntax & argument,
    bool & changed)
{
  changed = false;
  bool mentions_substitution_parameter =
      argument_syntax_mentions_any_substitution_parameter(argument.syntax,
                                                         parameters);
  if(!mentions_substitution_parameter) {
    for(size_t i = 0; i < parameters.size(); ++i) {
      if(!parameters[i].name.empty() &&
         callsemantic_internal::contains_identifier_token(argument.text,
                                                          parameters[i].name)) {
        mentions_substitution_parameter = true;
        break;
      }
    }
  }
  if(!mentions_substitution_parameter) {
    return true;
  }

  map<string, TypePtr> type_replacements;
  map<string, vector<TypePtr> > type_pack_replacements;
  map<string, ValueBinding> value_replacements;
  map<string, size_t> pack_size_replacements;
  collect_type_pack_replacements_for_substitution(scope,
                                                  parameters,
                                                  arguments,
                                                  type_pack_replacements);
  collect_template_parameter_pack_size_replacements(parameters,
                                                    arguments,
                                                    pack_size_replacements);
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(parameters[i].name.empty()) {
      continue;
    }
    if(parameters[i].parameter_pack) {
      continue;
    }
    if(parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
       arguments[i].kind == TemplateArgument::TA_TYPE &&
       arguments[i].type) {
      type_replacements[parameters[i].name] = arguments[i].type;
    } else if(parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
              arguments[i].kind == TemplateArgument::TA_VALUE) {
      ValueBinding binding(ValueBinding::VK_VARIABLE,
                           !arguments[i].text.empty() ?
                               arguments[i].text :
                               to_string(arguments[i].value),
                           arguments[i].type);
      binding.dependent_template_value = arguments[i].dependent;
      if(arguments[i].dependent) {
        binding.non_type_template_argument_text = binding.name;
      } else {
        binding.has_constant_value = true;
        binding.constant_value = arguments[i].value;
      }
      value_replacements[parameters[i].name] = binding;
    }
  }

  if(type_replacements.empty() &&
     type_pack_replacements.empty() &&
     value_replacements.empty() &&
     pack_size_replacements.empty()) {
    return true;
  }

  string rewritten_text = argument.text;
  bool text_changed = false;
  if(!pack_size_replacements.empty()) {
    rewritten_text = replace_sizeof_pack_count_text(rewritten_text,
                                                    pack_size_replacements,
                                                    text_changed);
  }
  for(map<string, TypePtr>::const_iterator it = type_replacements.begin();
      it != type_replacements.end();
      ++it) {
    rewritten_text = replace_identifier_token_text(
        rewritten_text,
        it->first,
        reparseable_type_argument_text(it->second),
        text_changed);
  }
  for(map<string, ValueBinding>::const_iterator it = value_replacements.begin();
      it != value_replacements.end();
      ++it) {
    rewritten_text = replace_identifier_token_text(
        rewritten_text,
        it->first,
        substituted_value_pack_argument_text(it->second),
        text_changed);
  }

  if(!text_changed &&
     !argument.syntax.template_id &&
     !argument.syntax.type_id &&
     !argument.syntax.expression) {
    return true;
  }

  if(text_changed) {
    if(argument.syntax.source_text.empty()) {
      argument.syntax.source_text = argument.syntax.text;
    }
    argument.text = trim_space(rewritten_text);
    argument.syntax.text = argument.text;
    changed = true;
  }

  bool structured_pack_changed = false;
  if(argument.syntax.template_id) {
    if(!type_pack_replacements.empty()) {
      structured_pack_changed =
          expand_type_pack_template_id_syntax_arguments(
              *argument.syntax.template_id,
              type_pack_replacements);
      if(structured_pack_changed) {
        argument.text = template_id_syntax_lookup_text(*argument.syntax.template_id);
        argument.syntax.text = argument.text;
        changed = true;
      }
    }
    if(!pack_size_replacements.empty()) {
      substitute_sizeof_pack_counts_template_id_arguments(
          *argument.syntax.template_id,
          pack_size_replacements);
    }
    if(!type_replacements.empty() && scope) {
      substitute_type_pack_template_id_arguments(*argument.syntax.template_id,
                                                 *scope,
                                                 type_replacements);
    }
    if(!value_replacements.empty()) {
      substitute_value_pack_template_id_arguments(*argument.syntax.template_id,
                                                  value_replacements);
    }
    if(structured_pack_changed ||
       !type_replacements.empty() ||
       !value_replacements.empty() ||
       !pack_size_replacements.empty()) {
      argument.text = template_id_syntax_lookup_text(*argument.syntax.template_id);
      argument.syntax.text = argument.text;
    }
  }
  if(scope && argument.syntax.type_id) {
    CppAstNode rewritten_type;
    if(substitute_expression_node_for_template_arguments(*scope,
                                                         *argument.syntax.type_id,
                                                         parameters,
                                                         arguments,
                                                         rewritten_type)) {
      if(!type_pack_replacements.empty() &&
         expand_type_pack_ast_node_template_id_syntaxes(rewritten_type,
                                                        type_pack_replacements)) {
        structured_pack_changed = true;
      }
      argument.syntax.type_id.reset(new CppAstNode(rewritten_type));
    }
  }
  if(scope && argument.syntax.expression) {
    CppAstNode rewritten_expression;
    if(substitute_expression_node_for_template_arguments(*scope,
                                                          *argument.syntax.expression,
                                                          parameters,
                                                          arguments,
                                                          rewritten_expression)) {
      if(!type_pack_replacements.empty() &&
         expand_type_pack_ast_node_template_id_syntaxes(rewritten_expression,
                                                        type_pack_replacements)) {
        structured_pack_changed = true;
      }
      argument.syntax.expression.reset(new CppAstNode(rewritten_expression));
    }
  }
  argument.syntax.dependent =
      argument_syntax_mentions_any_substitution_parameter(argument.syntax,
                                                         parameters);
  return true;
}

bool substitute_dependent_template_argument_syntaxes(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    const vector<DependentAliasTemplateArgumentSyntax> & source_arguments,
    vector<DependentAliasTemplateArgumentSyntax> & substituted_arguments,
    Scope * scope,
    bool & changed)
{
  substituted_arguments.clear();
  changed = false;
  for(size_t i = 0; i < source_arguments.size(); ++i) {
    const DependentAliasTemplateArgumentSyntax & source_argument =
        source_arguments[i];
    if(dependent_argument_is_pack_expansion(source_argument)) {
      const auto try_expand_value_pack_argument = [&]() -> bool
      {
        if(!scope) {
          return false;
        }
        string value_pack_name;
        const vector<ValueBinding> * bound_value_pack = nullptr;
        if(direct_value_pack_name_from_argument(source_argument, value_pack_name)) {
          bound_value_pack = lookup_value_pack(*scope, value_pack_name);
        }
        if(!bound_value_pack) {
          return false;
        }
        for(size_t pack_index = 0; pack_index < bound_value_pack->size(); ++pack_index) {
          const ValueBinding & binding = (*bound_value_pack)[pack_index];
          DependentAliasTemplateArgumentSyntax expanded = source_argument;
          if(expanded.syntax.source_text.empty()) {
            expanded.syntax.source_text = expanded.text;
          }
          expanded.text = substituted_value_pack_argument_text(binding);
          expanded.type = binding.type;
          expanded.syntax.text = expanded.text;
          expanded.syntax.pack_expansion = false;
          expanded.syntax.dependent = binding.dependent_template_value;
          expanded.syntax.expression.reset(
              new CppAstNode(make_substituted_value_expression_node(binding)));
          substituted_arguments.push_back(expanded);
        }
        changed = true;
        return true;
      };

      if(!source_argument.type) {
        if(try_expand_value_pack_argument()) {
          continue;
        }
        substituted_arguments.push_back(source_argument);
        continue;
      }

      size_t pack_parameter_index = parameters.size();
      if(!find_pack_expansion_substitution_parameter(source_argument.type,
                                                     parameters,
                                                     pack_parameter_index)) {
        if(try_expand_value_pack_argument()) {
          continue;
        }
        substituted_arguments.push_back(source_argument);
        continue;
      }

      size_t pack_argument_count = 0;
      if(!template_parameter_argument_count(parameters,
                                            arguments,
                                            pack_parameter_index,
                                            scope,
                                            pack_argument_count)) {
        return false;
      }

      for(size_t pack_index = 0; pack_index < pack_argument_count; ++pack_index) {
        vector<TemplateArgument> element_arguments;
        if(!build_pack_element_substitution_arguments(parameters,
                                                      arguments,
                                                      pack_parameter_index,
                                                      pack_index,
                                                      scope,
                                                      element_arguments)) {
          return false;
        }
        TypePtr substituted_type;
        if(!substitute_type_impl(source_argument.type,
                                 parameters,
                                 element_arguments,
                                 scope,
                                 substituted_type) ||
           !substituted_type) {
          return false;
        }
        DependentAliasTemplateArgumentSyntax expanded = source_argument;
        update_dependent_argument_with_type(expanded, substituted_type);
        substituted_arguments.push_back(expanded);
      }
      changed = true;
      continue;
    }

    DependentAliasTemplateArgumentSyntax substituted = source_argument;
    bool expression_changed = false;
    if(!substitute_dependent_argument_expression(scope,
                                                 parameters,
                                                 arguments,
                                                 substituted,
                                                 expression_changed)) {
      return false;
    }
    if(expression_changed) {
      changed = true;
    }
    bool text_or_syntax_changed = false;
    if(!substitute_dependent_argument_text_and_syntax(scope,
                                                      parameters,
                                                      arguments,
                                                      substituted,
                                                      text_or_syntax_changed)) {
      return false;
    }
    if(text_or_syntax_changed) {
      changed = true;
    }
    if(source_argument.type) {
      TypePtr substituted_type;
      if(substitute_type_impl(source_argument.type,
                              parameters,
                              arguments,
                              scope,
                              substituted_type) &&
         substituted_type &&
         !type_equals(source_argument.type, substituted_type)) {
        update_dependent_argument_with_type(substituted, substituted_type);
        changed = true;
      }
    }
    substituted_arguments.push_back(substituted);
  }
  return true;
}

string strip_pack_expansion_suffix_text(string text)
{
  text = trim_space(text);
  if(text.size() >= 3 && text.substr(text.size() - 3) == "...") {
    return trim_space(text.substr(0, text.size() - 3));
  }
  return text;
}

const TemplateArgument * substitution_argument_for_parameter(
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    const TemplateParameterInfo * parameter)
{
  if(!parameter) {
    return nullptr;
  }
  for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
    if(&parameters[i] == parameter ||
       (!parameter->name.empty() && parameters[i].name == parameter->name) ||
       (!parameter->placeholder_key.empty() &&
        parameters[i].placeholder_key == parameter->placeholder_key)) {
      return &arguments[i];
    }
  }
  return nullptr;
}

const TemplateParameterInfo * non_type_substitution_parameter_for_argument(
    const vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument)
{
  if(argument.kind != TemplateArgument::TA_VALUE) {
    return nullptr;
  }
  const string parameter_name = strip_pack_expansion_suffix_text(argument.text);
  if(parameter_name.empty()) {
    return nullptr;
  }
  const TemplateParameterInfo * parameter =
      find_template_parameter_by_name(parameters, parameter_name);
  return parameter && parameter->kind == TemplateParameterInfo::TP_NON_TYPE ?
             parameter :
             nullptr;
}

bool substitute_template_argument_for_mangle_info(
    const TemplateArgument & source,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    Scope * scope,
    TemplateArgument & out)
{
  out = source;
  if(source.kind == TemplateArgument::TA_TYPE && source.type) {
    const TemplateParameterInfo * parameter =
        find_substitution_parameter(parameters, source.type);
    if(const TemplateArgument * replacement =
           substitution_argument_for_parameter(parameters, arguments, parameter)) {
      if(replacement->kind == TemplateArgument::TA_TYPE) {
        out = *replacement;
        return true;
      }
    }
    TypePtr substituted_type;
    if(substitute_type_impl(source.type,
                            parameters,
                            arguments,
                            scope,
                            substituted_type) &&
       substituted_type &&
       !type_equals(source.type, substituted_type)) {
      out.type = substituted_type;
      out.text = substituted_dependent_argument_type_text(substituted_type);
      out.dependent = false;
      return true;
    }
    return false;
  }

  if(source.kind == TemplateArgument::TA_VALUE) {
    const TemplateParameterInfo * parameter =
        non_type_substitution_parameter_for_argument(parameters, source);
    if(const TemplateArgument * replacement =
           substitution_argument_for_parameter(parameters, arguments, parameter)) {
      if(replacement->kind == TemplateArgument::TA_VALUE) {
        out = *replacement;
        return true;
      }
    }
  }
  return false;
}

void update_substituted_dependent_class_mangle_info(
    const TypePtr & substituted,
    const TypePtr & source,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    Scope * scope)
{
  shared_ptr<const ClassTemplateSpecializationMangleInfo> source_info =
      named_type_class_template_specialization_mangle_info_const(source);
  if(!substituted || !source_info) {
    return;
  }

  shared_ptr<ClassTemplateSpecializationMangleInfo> info(
      new ClassTemplateSpecializationMangleInfo(*source_info));
  bool changed = false;
  for(size_t i = 0; i < info->arguments.size(); ++i) {
    TemplateArgument substituted_argument;
    if(substitute_template_argument_for_mangle_info(info->arguments[i],
                                                    parameters,
                                                    arguments,
                                                    scope,
                                                    substituted_argument)) {
      info->arguments[i] = substituted_argument;
      changed = true;
    }
  }
  if(changed) {
    set_named_type_class_template_specialization_mangle_info(substituted, info);
  }
}

bool substitute_dependent_class_type(const TypePtr & type,
                                     const vector<TemplateParameterInfo> & parameters,
                                     const vector<TemplateArgument> & arguments,
                                     Scope * scope,
                                     TypePtr & out)
{
  out.reset();
  void * class_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  if(!named_type_dependent_class_template(type,
                                          class_template_decl,
                                          dependent_arguments)) {
    return false;
  }

  vector<DependentAliasTemplateArgumentSyntax> substituted_arguments;
  bool changed = false;
  if(!substitute_dependent_template_argument_syntaxes(parameters,
                                                     arguments,
                                                     dependent_arguments,
                                                     substituted_arguments,
                                                     scope,
                                                     changed) ||
     !changed) {
    return false;
  }

  TypePtr substituted(new Type(*type));
  set_named_type_dependent_class_template(substituted,
                                          class_template_decl,
                                          substituted_arguments);
  update_substituted_dependent_class_mangle_info(substituted,
                                                type,
                                                parameters,
                                                arguments,
                                                scope);
  out = substituted;
  return true;
}

string dependent_qualified_member_display_text(
    const TypePtr & owner,
    const vector<string> & members,
    bool leading_typename)
{
  const string owner_text = trim_space(reparseable_type_argument_text(owner));
  if(owner_text.empty() || members.empty()) {
    return string();
  }

  string out = leading_typename ? "typename " : "";
  out += owner_text;
  for(size_t i = 0; i < members.size(); ++i) {
    out += "::";
    out += members[i];
  }
  return out;
}

bool substitute_dependent_qualified_member_type(
    const TypePtr & type,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    Scope * scope,
    TypePtr & out)
{
  out.reset();
  TypePtr owner;
  vector<string> members;
  bool leading_typename = false;
  vector<TemplateIdSyntax> member_template_ids;
  if(!named_type_dependent_qualified_member(type,
                                            owner,
                                            members,
                                            leading_typename,
                                            &member_template_ids)) {
    return false;
  }

  TypePtr substituted_owner;
  if(!substitute_type_impl(owner,
                           parameters,
                           arguments,
                           scope,
                           substituted_owner) ||
     !substituted_owner ||
     substituted_owner == owner) {
    return false;
  }

  const string display =
      dependent_qualified_member_display_text(substituted_owner,
                                              members,
                                              leading_typename);
  out = make_dependent_qualified_member_type(
      display.empty() ? type->named_display : display,
      substituted_owner,
      members,
      leading_typename,
      member_template_ids);
  return out != nullptr;
}

bool substitute_type_impl(const TypePtr & type,
                          const vector<TemplateParameterInfo> & parameters,
                          const vector<TemplateArgument> & arguments,
                          Scope * scope,
                          TypePtr & out)
{
  const TemplateParameterInfo * parameter = find_substitution_parameter(parameters, type);
  if(parameter) {
    for(size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
      if(parameters[i].name == parameter->name &&
         arguments[i].kind == TemplateArgument::TA_TYPE) {
        out = arguments[i].type;
        return true;
      }
    }
    return false;
  }

  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    out = type;
    return true;

  case Type::TK_NAMED:
  {
    TypePtr substituted_dependent_qualified_member;
    if(substitute_dependent_qualified_member_type(
           type,
           parameters,
           arguments,
           scope,
           substituted_dependent_qualified_member)) {
      out = substituted_dependent_qualified_member;
      return true;
    }
    TypePtr substituted_class;
    if(substitute_dependent_class_type(type,
                                       parameters,
                                       arguments,
                                       scope,
                                       substituted_class)) {
      out = substituted_class;
      return true;
    }
    TypePtr substituted_alias;
    if(substitute_dependent_alias_type(type,
                                       parameters,
                                       arguments,
                                       scope,
                                       substituted_alias)) {
      out = substituted_alias;
      return true;
    }
    if(is_dependent_builtin_type_transform_type(type)) {
      TypePtr inner;
      if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
        return false;
      }
      TypePtr substituted(new Type(*type));
      substituted->inner = inner;
      out = substituted;
      return true;
    }
    out = type;
    return true;
  }

  case Type::TK_CV:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = apply_cv(inner, type->cv_const, type->cv_volatile);
    return true;
  }

  case Type::TK_ATOMIC:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = make_atomic(inner);
    return true;
  }

  case Type::TK_POINTER:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = make_pointer(inner);
    return true;
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypePtr owner;
    TypePtr inner;
    if(!substitute_type_impl(type->owner, parameters, arguments, scope, owner) ||
       !substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = make_member_pointer(owner, inner);
    return true;
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = make_block_pointer(inner);
    return true;
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = collapse_lvalue_reference_type(inner);
    return true;
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = collapse_rvalue_reference_type(inner);
    return true;
  }

  case Type::TK_ARRAY:
  {
    TypePtr inner;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, inner)) {
      return false;
    }
    out = make_array(inner, type->has_bound, type->bound, type->bound_text);
    return true;
  }

  case Type::TK_FUNCTION:
  {
    TypePtr result_type;
    if(!substitute_type_impl(type->inner, parameters, arguments, scope, result_type)) {
      return false;
    }
    vector<TypePtr> params_out;
    params_out.reserve(type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      TypePtr param;
      if(!substitute_type_impl(type->params[i], parameters, arguments, scope, param)) {
        return false;
      }
      params_out.push_back(param);
    }
    out = make_function(result_type,
                        params_out,
                        type->variadic,
                        type->function_const,
                        type->function_volatile,
                        type->prototype_relaxed);
    return true;
  }
  }

  return false;
}

bool substitute_dependent_alias_type(const TypePtr & type,
                                     const vector<TemplateParameterInfo> & parameters,
                                     const vector<TemplateArgument> & arguments,
                                     Scope * scope,
                                     TypePtr & out)
{
  out.reset();
  void * alias_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  if(!named_type_dependent_alias_template(type,
                                          alias_template_decl,
                                          dependent_arguments)) {
    return false;
  }

  vector<DependentAliasTemplateArgumentSyntax> substituted_arguments;
  bool changed = false;
  if(!substitute_dependent_template_argument_syntaxes(parameters,
                                                     arguments,
                                                     dependent_arguments,
                                                     substituted_arguments,
                                                     scope,
                                                     changed) ||
     !changed) {
    return false;
  }

  dependent_arguments.swap(substituted_arguments);

  string display = type->named_display;
  string payload = named_type_semantic_payload(type);
  if(AliasTemplateDecl * alias_template =
         static_cast<AliasTemplateDecl *>(alias_template_decl)) {
    ostringstream specialization_name;
    specialization_name << alias_template->name << "<";
    for(size_t i = 0; i < dependent_arguments.size(); ++i) {
      if(i != 0) {
        specialization_name << ", ";
      }
      string argument_text = trim_space(dependent_arguments[i].text);
      if(argument_text.empty() && dependent_arguments[i].type) {
        argument_text =
            trim_space(substituted_dependent_argument_type_text(
                dependent_arguments[i].type));
      }
      specialization_name << argument_text;
    }
    specialization_name << ">";
    display = specialization_name.str();
    payload = alias_template->declaring_scope ?
        semantic_lookup::scope_qualified_name(*alias_template->declaring_scope,
                                              display) :
        display;
  }

  out = make_dependent_alias_type(display,
                                  payload,
                                  alias_template_decl,
                                  dependent_arguments);
  return out != nullptr;
}

TypePtr collapse_lvalue_reference_type(const TypePtr & inner)
{
  TypePtr base = strip_top_level_cv(inner);
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    return make_lvalue_reference_raw(base->inner);
  }
  return make_lvalue_reference_raw(inner);
}

TypePtr collapse_rvalue_reference_type(const TypePtr & inner)
{
  TypePtr base = strip_top_level_cv(inner);
  if(base && base->kind == Type::TK_LVALUE_REFERENCE) {
    return make_lvalue_reference_raw(base->inner);
  }
  if(base && base->kind == Type::TK_RVALUE_REFERENCE) {
    return make_rvalue_reference_raw(base->inner);
  }
  return make_rvalue_reference_raw(inner);
}

bool try_resolve_concrete_unary_type_transform_template_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    TypePtr & out)
{
  out.reset();
  if(arg_texts.size() != 1) {
    return false;
  }

  const auto name_matches =
      [&](std::initializer_list<const char *> names) -> bool
  {
    for(const char * name : names) {
      if(template_id.name == name) {
        return true;
      }
    }
    return false;
  };

  if(!name_matches({"__libcpp_remove_reference_t", "__remove_reference_t",
                    "remove_reference_t",
                    "__libcpp_remove_cv_t", "__remove_cv_t", "remove_cv_t",
                    "__libcpp_remove_const_t", "__remove_const_t",
                    "remove_const_t",
                    "__libcpp_remove_volatile_t", "__remove_volatile_t",
                    "remove_volatile_t",
                    "__libcpp_remove_const_ref_t", "__remove_const_ref_t",
                    "__libcpp_remove_cvref_t", "__remove_cvref_t",
                    "remove_cvref_t",
                    "__libcpp_remove_pointer_t", "__remove_pointer_t",
                    "remove_pointer_t",
                    "__libcpp_remove_extent_t", "__remove_extent_t",
                    "remove_extent_t",
                    "__libcpp_remove_all_extents_t", "__remove_all_extents_t",
                    "remove_all_extents_t",
                    "__decay_t", "decay_t",
                    "__libcpp_add_pointer_t", "__add_pointer_t",
                    "add_pointer_t",
                    "__libcpp_add_lvalue_reference_t",
                    "__add_lvalue_reference_t", "add_lvalue_reference_t",
                    "__libcpp_add_rvalue_reference_t",
                    "__add_rvalue_reference_t", "add_rvalue_reference_t",
                    "type_identity_t", "__type_identity_t"})) {
    return false;
  }

  TypePtr arg_type;
  bool resolved_arg = false;
  const TemplateArgumentSyntax * arg_syntax =
      arg_syntaxes && arg_syntaxes->size() == 1 ? &(*arg_syntaxes)[0] : nullptr;
  if(arg_syntax) {
    resolved_arg =
        resolve_type_argument_syntax_type(services, scope, *arg_syntax, true, arg_type) &&
        arg_type != nullptr;
  }
  if(!resolved_arg ||
     !arg_type ||
     service_type_depends_on_template_parameter(services, arg_type)) {
    return false;
  }

  if(name_matches({"__libcpp_remove_reference_t", "__remove_reference_t",
                   "remove_reference_t"})) {
    out = remove_reference_type(arg_type);
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_cv_t", "__remove_cv_t", "remove_cv_t"})) {
    out = strip_top_level_cv(arg_type);
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_const_t", "__remove_const_t",
                   "remove_const_t"})) {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, false, arg_type->cv_volatile) :
        arg_type;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_volatile_t", "__remove_volatile_t",
                   "remove_volatile_t"})) {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, arg_type->cv_const, false) :
        arg_type;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_cvref_t", "__remove_cvref_t",
                   "remove_cvref_t"})) {
    out = strip_top_level_cv(remove_reference_type(arg_type));
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_const_ref_t", "__remove_const_ref_t"})) {
    TypePtr no_ref = remove_reference_type(arg_type);
    out = no_ref && no_ref->kind == Type::TK_CV ?
        make_cv(no_ref->inner, false, no_ref->cv_volatile) :
        no_ref;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_pointer_t", "__remove_pointer_t",
                   "remove_pointer_t"})) {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    out = base->kind == Type::TK_POINTER ? base->inner : arg_type;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_extent_t", "__remove_extent_t",
                   "remove_extent_t"})) {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    out = base->kind == Type::TK_ARRAY ? base->inner : arg_type;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_remove_all_extents_t", "__remove_all_extents_t",
                   "remove_all_extents_t"})) {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    while(base && base->kind == Type::TK_ARRAY) {
      base = strip_top_level_cv(base->inner);
    }
    out = base ? base : arg_type;
    return out != nullptr;
  }
  if(name_matches({"__decay_t", "decay_t"})) {
    TypePtr decayed = remove_reference_type(arg_type);
    TypePtr decayed_base = strip_top_level_cv(decayed);
    if(!decayed_base) {
      return false;
    }
    if(decayed_base->kind == Type::TK_ARRAY) {
      out = make_pointer(decayed_base->inner);
      return out != nullptr;
    }
    if(decayed_base->kind == Type::TK_FUNCTION) {
      out = make_pointer(decayed_base);
      return out != nullptr;
    }
    out = decayed_base;
    return out != nullptr;
  }
  if(name_matches({"__libcpp_add_pointer_t", "__add_pointer_t",
                   "add_pointer_t"})) {
    TypePtr pointee = remove_reference_type(arg_type);
    if(!pointee) {
      return false;
    }
    out = make_pointer(pointee);
    return out != nullptr;
  }
  if(name_matches({"__libcpp_add_lvalue_reference_t",
                   "__add_lvalue_reference_t", "add_lvalue_reference_t",
                   "__libcpp_add_rvalue_reference_t",
                   "__add_rvalue_reference_t", "add_rvalue_reference_t"})) {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    const bool lvalue =
        name_matches({"__libcpp_add_lvalue_reference_t",
                      "__add_lvalue_reference_t",
                      "add_lvalue_reference_t"});
    if(is_void_type(base) || base->kind == Type::TK_LVALUE_REFERENCE) {
      out = arg_type;
      return true;
    }
    if(base->kind == Type::TK_RVALUE_REFERENCE) {
      out = lvalue ? make_lvalue_reference_raw(base->inner) : arg_type;
      return true;
    }
    out = lvalue ? make_lvalue_reference_raw(arg_type) : make_rvalue_reference_raw(arg_type);
    return true;
  }
  if(name_matches({"type_identity_t", "__type_identity_t"})) {
    out = arg_type;
    return true;
  }

  return false;
}

void append_alias_template_source_bindings(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    std::vector<template_api::TemplateWitnessSourceBinding> & out,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts,
    const std::string & source)
{
  const auto preserves_qualified_member =
      [](const TemplateArgument & argument) -> bool
  {
    if(argument.kind != TemplateArgument::TA_TYPE || !argument.type) {
      return false;
    }
    TypePtr owner;
    std::vector<std::string> members;
    bool leading_typename = false;
    return named_type_dependent_qualified_member(argument.type,
                                                owner,
                                                members,
                                                leading_typename,
                                                nullptr) &&
           !members.empty();
  };
  const auto argument_text =
      [&services](const TemplateArgument & argument) -> std::string
  {
    return template_model::template_argument_text(
        argument,
        [&services](const TypePtr & type)
        {
          return service_lookup_text_for_type_argument(services, type);
        });
  };
  std::size_t arg_index = 0;
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    template_api::TemplateWitnessSourceBinding binding;
    binding.param = parameters[i].name.empty() ?
        std::string("$") + std::to_string(i + 1) :
        parameters[i].name;
    binding.source = source;
    binding.type_like = parameters[i].kind == TemplateParameterInfo::TP_TYPE;
    binding.function_pointer_parameter =
        template_parameter_is_function_pointer_value(parameters[i]);
    if(parameters[i].parameter_pack) {
      std::size_t trailing_non_pack = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      if(arguments.size() < arg_index + trailing_non_pack) {
        break;
      }
      const std::size_t pack_end = arguments.size() - trailing_non_pack;
      const std::size_t pack_count = pack_end - arg_index;
      const std::size_t remaining_explicit =
          explicit_argument_texts.size() > explicit_index ?
              explicit_argument_texts.size() - explicit_index :
              0;
      const std::size_t explicit_pack_count =
          std::min(pack_count,
                   remaining_explicit > trailing_non_pack ?
                       remaining_explicit - trailing_non_pack :
                       0);
      binding.pack_binding = true;
      if(pack_count == 0) {
        binding.arg = "<>";
        binding.source = "deduced";
      } else if(explicit_pack_count == 1) {
        const std::string element_text = alias_template_source_binding_arg_text(
            services,
            scope,
            arguments[arg_index],
            explicit_argument_texts[explicit_index]);
        binding.arg = element_text;
        binding.pack_arguments.push_back(element_text);
      } else if(pack_count == 1) {
        const std::string element_text = argument_text(arguments[arg_index]);
        binding.arg = element_text;
        binding.pack_arguments.push_back(element_text);
        binding.source = explicit_pack_count == 0 ? "defaulted" : source;
      } else {
        binding.pack_aggregate = true;
        std::ostringstream pack_text;
        pack_text << "<";
        for(std::size_t j = arg_index; j < pack_end; ++j) {
          if(j != arg_index) {
            pack_text << ", ";
          }
          const std::size_t explicit_offset = j - arg_index;
          std::string element_text;
          if(explicit_offset < explicit_pack_count) {
            element_text = alias_template_source_binding_arg_text(
                services,
                scope,
                arguments[j],
                explicit_argument_texts[explicit_index + explicit_offset]);
          } else {
            element_text = argument_text(arguments[j]);
          }
          binding.pack_arguments.push_back(element_text);
          pack_text << element_text;
        }
        pack_text << ">";
        binding.arg = pack_text.str();
        binding.source =
            explicit_pack_count == pack_count ? source : "defaulted";
      }
      for(std::size_t j = arg_index; j < pack_end; ++j) {
        if(preserves_qualified_member(arguments[j])) {
          binding.preserve_qualified_member = true;
          break;
        }
      }
      out.push_back(binding);
      arg_index = pack_end;
      explicit_index += explicit_pack_count;
      continue;
    }
    if(arg_index >= arguments.size()) {
      break;
    }
    if(explicit_index < explicit_argument_texts.size()) {
      binding.arg = alias_template_source_binding_arg_text(
          services,
          scope,
          arguments[arg_index],
          explicit_argument_texts[explicit_index]);
      ++explicit_index;
    } else {
      binding.arg = argument_text(arguments[arg_index]);
      binding.source = "defaulted";
    }
    binding.preserve_qualified_member =
        preserves_qualified_member(arguments[arg_index]);
    out.push_back(binding);
    ++arg_index;
  }
}

std::string alias_template_decl_location(
    const template_api::TemplateWitnessContext & ctx,
    const AliasTemplateDecl & alias_template)
{
  std::string location =
      semantic_model::source_decl_anchor_location(alias_template.declaration_anchor);
  if(location.empty() && alias_template.type_id) {
    location = template_api::template_witness_detail::source_location_for_token_index(
        ctx,
        alias_template.type_id->token_start);
  }
  return template_api::normalize_template_witness_source_location(location);
}

struct TemplateIdSyntaxOccurrence
{
  const TemplateIdSyntax * syntax = nullptr;
};

void collect_template_id_syntax_occurrences(const CppAstNode & node,
                                            std::vector<TemplateIdSyntaxOccurrence> & out);

void collect_template_id_syntax_occurrences(const TemplateIdSyntax & syntax,
                                            std::vector<TemplateIdSyntaxOccurrence> & out)
{
  TemplateIdSyntaxOccurrence occurrence;
  occurrence.syntax = &syntax;
  out.push_back(occurrence);
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const TemplateArgumentSyntax & argument = syntax.argument_syntaxes[i];
    if(argument.template_id) {
      collect_template_id_syntax_occurrences(*argument.template_id, out);
    }
    if(argument.type_id) {
      collect_template_id_syntax_occurrences(*argument.type_id, out);
    }
    if(argument.expression) {
      collect_template_id_syntax_occurrences(*argument.expression, out);
    }
  }
}

void collect_template_id_syntax_occurrences(const CppAstNode & node,
                                            std::vector<TemplateIdSyntaxOccurrence> & out)
{
  if(node.template_id_syntax) {
    collect_template_id_syntax_occurrences(*node.template_id_syntax, out);
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    collect_template_id_syntax_occurrences(node.qualifier_template_id_syntaxes[i],
                                           out);
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    collect_template_id_syntax_occurrences(node.qualifier_type_syntaxes[i], out);
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    collect_template_id_syntax_occurrences(node.children[i], out);
  }
}

void collect_template_id_syntax_occurrences(
    const std::vector<TemplateArgumentSyntax> & syntaxes,
    std::vector<TemplateIdSyntaxOccurrence> & out)
{
  for(std::size_t i = 0; i < syntaxes.size(); ++i) {
    if(syntaxes[i].template_id) {
      collect_template_id_syntax_occurrences(*syntaxes[i].template_id, out);
    }
    if(syntaxes[i].type_id) {
      collect_template_id_syntax_occurrences(*syntaxes[i].type_id, out);
    }
    if(syntaxes[i].expression) {
      collect_template_id_syntax_occurrences(*syntaxes[i].expression, out);
    }
  }
}

bool argument_syntax_has_pack_expanded_source_text(
    const TemplateArgumentSyntax & syntax);

bool template_id_syntax_has_pack_expanded_source_text(
    const TemplateIdSyntax & syntax)
{
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(argument_syntax_has_pack_expanded_source_text(
           syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

bool ast_node_has_pack_expanded_source_text(const CppAstNode & node)
{
  if(node.template_id_syntax &&
     template_id_syntax_has_pack_expanded_source_text(*node.template_id_syntax)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_node_has_pack_expanded_source_text(*node.conversion_type_id_syntax)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_node_has_pack_expanded_source_text(*node.base_type_syntax)) {
    return true;
  }
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_has_pack_expanded_source_text(
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_has_pack_expanded_source_text(node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_has_pack_expanded_source_text(node.children[i])) {
      return true;
    }
  }
  return false;
}

bool argument_syntax_has_pack_expanded_source_text(
    const TemplateArgumentSyntax & syntax)
{
  const std::string source = trim_space(syntax.source_text);
  const std::string text = trim_space(syntax.text);
  if(!source.empty() &&
     source.find("...") != std::string::npos &&
     compact_source_argument_key(source) != compact_source_argument_key(text)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_has_pack_expanded_source_text(*syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_has_pack_expanded_source_text(*syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_has_pack_expanded_source_text(*syntax.expression)) {
    return true;
  }
  return false;
}

bool argument_syntaxes_have_pack_expanded_source_text(
    const std::vector<TemplateArgumentSyntax> * syntaxes)
{
  if(!syntaxes) {
    return false;
  }
  for(std::size_t i = 0; i < syntaxes->size(); ++i) {
    if(argument_syntax_has_pack_expanded_source_text((*syntaxes)[i])) {
      return true;
    }
  }
  return false;
}

bool argument_texts_have_instantiated_pack_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<std::string> & texts)
{
  bool has_pack_syntax = false;
  for(std::size_t i = 0; i < texts.size(); ++i) {
    if(texts[i].find("...") == std::string::npos) {
      continue;
    }
    has_pack_syntax = true;
    if(alias_argument_text_mentions_template_dependency(services,
                                                        scope,
                                                        texts[i])) {
      return false;
    }
  }
  return has_pack_syntax;
}

std::string source_location_after_identifier(const std::string & location,
                                             const std::string & identifier)
{
  typedef template_api::template_witness_detail::ParsedSourceLocation
      ParsedSourceLocation;
  const ParsedSourceLocation parsed =
      template_api::template_witness_detail::parse_source_location(
          template_api::normalize_template_witness_source_location(location));
  if(!parsed.valid) {
    return location;
  }
  std::ostringstream out;
  out << parsed.file << ":" << parsed.line << ":"
      << parsed.column + static_cast<int>(identifier.size());
  return out.str();
}

void record_nested_alias_template_source_uses_for_arguments(
    template_api::TemplateServices & services,
    Scope & scope,
    const std::vector<std::string> & explicit_argument_texts,
    const std::vector<TemplateArgumentSyntax> * explicit_argument_syntaxes,
    const std::string & source_location,
    const std::string & skip_template_name)
{
  if(!witness::source_capture_enabled(services.witness_context) ||
     source_location.empty()) {
    return;
  }
  std::string search_location = source_location;
  const auto emit_occurrence =
      [&](const QualifiedName & nested_template_id,
          const std::vector<std::string> & nested_arg_texts,
          const std::vector<TemplateArgumentSyntax> * nested_arg_syntaxes,
          const std::string & occurrence_location) -> void
  {
    const std::string nested_name =
        template_api::qualified_name_text(nested_template_id);
    const std::string unqualified =
        semantic_utils::unqualified_member_name(nested_name);
    if(unqualified.empty()) {
      return;
    }
    std::string use_location =
        template_api::normalize_template_witness_source_location(
            occurrence_location);
    if(use_location.empty()) {
      use_location =
          template_api::normalize_template_witness_source_location(
              template_api::template_witness_detail::
                  source_location_for_identifier_token_on_or_after(
                      services.witness_context,
                      search_location,
                      unqualified,
                      false,
                      true));
    }
    if(use_location.empty()) {
      return;
    }
    if(!witness::source_location_capture_enabled(services.witness_context,
                                                 use_location)) {
      return;
    }
    search_location = source_location_after_identifier(use_location,
                                                       unqualified);
    if(nested_name == skip_template_name ||
       unqualified == semantic_utils::unqualified_member_name(skip_template_name)) {
      return;
    }
    AliasTemplateDecl * nested_alias =
        lookup_alias_template(services,
                              scope,
                              nested_name);
    if(!nested_alias) {
      return;
    }
    template_api::TemplateEnvironmentHandle scope_handle =
        template_api::make_template_environment(scope);
    if(argument_texts_have_instantiated_pack_syntax(services,
                                                    scope_handle,
                                                    nested_arg_texts)) {
      return;
    }
    std::vector<TemplateArgument> nested_arguments;
    if(!template_api::resolve_template_arguments(
           services,
           scope_handle,
           nested_alias->parameters,
           nested_arg_texts,
           nested_arg_syntaxes,
           nested_arguments,
           nested_alias->declaring_scope ?
               template_api::make_template_environment(
                   *nested_alias->declaring_scope) :
               template_api::TemplateEnvironmentHandle())) {
      return;
    }
    std::vector<std::string> source_arg_texts =
        source_argument_texts_for_occurrence(nested_arg_texts,
                                             nested_arg_syntaxes);
    canonicalize_alias_template_source_argument_texts(nested_alias->parameters,
                                             source_arg_texts);
    witness::AliasUseEmitRequest request;
    request.use_location = use_location;
    request.template_id_occurrence =
        witness::make_source_template_id_occurrence(use_location,
                                                    source_arg_texts);
    bool has_exact_source_arguments = false;
    bool exact_source_arguments = false;
    bool exact_source_arguments_have_pack_syntax = false;
    if(const std::vector<std::string> * exact_source_args =
           template_api::current_template_id_source_arguments_ptr(
               use_location,
               nested_alias->name)) {
      has_exact_source_arguments = true;
      for(std::size_t i = 0; i < exact_source_args->size(); ++i) {
        if((*exact_source_args)[i].find("...") != std::string::npos) {
          exact_source_arguments_have_pack_syntax = true;
          break;
        }
      }
      std::vector<std::string> canonical_exact_source_args = *exact_source_args;
      canonicalize_alias_template_source_argument_texts(nested_alias->parameters,
                                               canonical_exact_source_args);
      exact_source_arguments =
          source_arguments_compact_match(canonical_exact_source_args,
                                         source_arg_texts);
      request.template_id_occurrence.exact_source_arguments =
          exact_source_arguments;
      request.template_id_occurrence.synthesized =
          !exact_source_arguments;
    }
    if(has_exact_source_arguments &&
       !exact_source_arguments &&
       exact_source_arguments_have_pack_syntax) {
      return;
    }
    mark_alias_template_id_occurrence_argument_facts(
        services,
        template_api::make_template_environment(scope),
        source_arg_texts,
        nested_arg_syntaxes,
        nested_arguments,
        request.template_id_occurrence);
    mark_alias_template_value_owner_argument_facts(
        services,
        &scope,
        *nested_alias,
        nested_arguments,
        nested_arg_syntaxes,
        request.template_id_occurrence);
    request.template_name =
        template_api::alias_template_witness_entity(nested_alias);
    request.selected_decl_location =
        alias_template_decl_location(services.witness_context, *nested_alias);
    request.selected_decl_has_name_location =
        semantic_model::source_decl_anchor_has_name_location(
            nested_alias->declaration_anchor);
    request.origin = witness::AliasUseEmissionOrigin::NestedSourceTemplateId;
    append_alias_template_source_bindings(services,
                                          template_api::make_template_environment(scope),
                                          request.bindings,
                                          nested_alias->parameters,
                                          nested_arguments,
                                          source_arg_texts,
                                          "explicit");
    mark_alias_source_binding_preserve_from_occurrence(
        request.bindings,
        request.template_id_occurrence);
    if(services.semantic_context) {
      callsemantic::rewrite_current_specialization_alias_binding_texts(
          *services.semantic_context,
          scope,
          nested_alias->parameters,
          nested_arguments,
          source_arg_texts,
          nested_arg_syntaxes,
          request.bindings,
          &request.template_id_occurrence);
    }
    witness::emit_alias_use(services.witness_context, request);
  };

  std::vector<TemplateIdSyntaxOccurrence> syntax_occurrences;
  if(explicit_argument_syntaxes) {
    collect_template_id_syntax_occurrences(*explicit_argument_syntaxes,
                                           syntax_occurrences);
  }
  for(std::size_t i = 0; i < syntax_occurrences.size(); ++i) {
    const TemplateIdSyntax * syntax = syntax_occurrences[i].syntax;
    if(!syntax || syntax->name.name.empty()) {
      continue;
    }
    if(template_id_syntax_has_pack_expanded_source_text(*syntax)) {
      continue;
    }
    const std::string syntax_location =
        template_api::normalize_template_witness_source_location(
            template_api::template_witness_detail::source_location_for_location_id(
                services.witness_context,
                syntax->source_location_id));
    emit_occurrence(syntax->name,
                    template_id_syntax_argument_texts(*syntax),
                    &syntax->argument_syntaxes,
                    syntax_location);
  }
  return;
}

bool alias_template_owner_is_current_scope(Scope & scope,
                                           const AliasTemplateDecl & alias_template)
{
  if(!alias_template.declaring_scope ||
     !alias_template.declaring_scope->class_info) {
    return false;
  }
  const ClassInfo * owner = alias_template.declaring_scope->class_info;
  for(Scope * current = &scope; current; current = current->parent) {
    if(!current->class_info) {
      continue;
    }
    if(current->class_info == owner) {
      return true;
    }
    if(owner->source_template &&
       current->class_info->source_template == owner->source_template) {
      return true;
    }
  }
  return false;
}

bool source_template_id_has_dependent_non_current_member_owner(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const AliasTemplateDecl & alias_template,
    const QualifiedName & source_template_id)
{
  if(!scope.valid() ||
     !alias_template.declaring_scope ||
     !alias_template.declaring_scope->class_info) {
    return false;
  }
  if(alias_template_owner_is_current_scope(scope.require(), alias_template)) {
    return false;
  }
  if(callsemantic::scope_is_inside_source_template_context(scope.require())) {
    return true;
  }
  if(source_template_id.qualifiers.empty()) {
    return false;
  }
  const std::string owner_text =
      semantic_utils::trim_space(source_template_id.qualifiers.back());
  if(owner_text.empty()) {
    return false;
  }
  return text_mentions_template_placeholders(services, scope, owner_text) ||
         text_mentions_dependent_non_namespace_binding_names(services,
                                                             scope,
                                                             owner_text);
}

void record_direct_alias_template_source_use_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const AliasTemplateDecl & alias_template,
    const QualifiedName & source_template_id,
    const TypePtr & resolved,
    const std::vector<TemplateArgument> & resolved_arguments,
    const std::vector<std::string> & explicit_argument_texts,
    const std::vector<TemplateArgumentSyntax> * explicit_argument_syntaxes,
    const std::string & source_location)
{
  if(!resolved ||
     !witness::source_capture_enabled(services.witness_context) ||
     source_template_id.name != alias_template.name) {
    return;
  }
  if(source_template_id_has_dependent_non_current_member_owner(
         services, scope, alias_template, source_template_id)) {
    return;
  }
  if(!source_template_id.qualifiers.empty() &&
     alias_template.declaring_scope &&
     alias_template.declaring_scope->class_info) {
    const std::string source_owner =
        semantic_utils::strip_trailing_top_level_template_arguments(
            semantic_utils::unqualified_member_name(
                source_template_id.qualifiers.back()));
    const std::string declared_owner =
        semantic_utils::strip_trailing_top_level_template_arguments(
            semantic_utils::unqualified_member_name(
                alias_template.declaring_scope->class_info->name));
    if(!source_owner.empty() &&
       !declared_owner.empty() &&
       source_owner != declared_owner) {
      return;
    }
  }

  const std::string use_location =
      !source_location.empty() ?
          template_api::normalize_template_witness_source_location(source_location) :
          template_api::normalize_template_witness_source_location(
              parser_trace::current_use_location());
  if(!semantic_trace::source_location_points_at_identifier(use_location,
                                                           alias_template.name)) {
    return;
  }
  if(!witness::source_location_capture_enabled(services.witness_context,
                                               use_location)) {
    return;
  }

  witness::AliasUseEmitRequest request;
  request.use_location = use_location;
  std::vector<std::string> source_argument_texts =
      source_argument_texts_for_occurrence(use_location,
                                           alias_template.name,
                                           explicit_argument_texts,
                                           explicit_argument_syntaxes);
  canonicalize_alias_template_source_argument_texts(alias_template.parameters,
                                           source_argument_texts);
  if(argument_texts_have_instantiated_pack_syntax(services,
                                                  scope,
                                                  source_argument_texts)) {
    return;
  }
  request.template_id_occurrence =
      witness::make_source_template_id_occurrence(
          use_location,
          source_argument_texts);
  const bool pack_expanded_source =
      argument_syntaxes_have_pack_expanded_source_text(
          explicit_argument_syntaxes);
  bool has_exact_source_arguments = false;
  bool exact_source_arguments = false;
  bool exact_source_arguments_have_pack_syntax = false;
  if(const std::vector<std::string> * exact_source_args =
         template_api::current_template_id_source_arguments_ptr(
             use_location,
             alias_template.name)) {
    has_exact_source_arguments = true;
    for(std::size_t i = 0; i < exact_source_args->size(); ++i) {
      if((*exact_source_args)[i].find("...") != std::string::npos) {
        exact_source_arguments_have_pack_syntax = true;
        break;
      }
    }
    std::vector<std::string> canonical_exact_source_args = *exact_source_args;
    canonicalize_alias_template_source_argument_texts(alias_template.parameters,
                                             canonical_exact_source_args);
    exact_source_arguments =
        source_arguments_compact_match(canonical_exact_source_args,
                                       source_argument_texts);
    request.template_id_occurrence.exact_source_arguments =
        exact_source_arguments;
    request.template_id_occurrence.synthesized =
        !exact_source_arguments;
  }
  if(pack_expanded_source &&
     (!has_exact_source_arguments || !exact_source_arguments)) {
    return;
  }
  if(has_exact_source_arguments &&
     !exact_source_arguments &&
     exact_source_arguments_have_pack_syntax) {
    return;
  }
  mark_alias_template_id_occurrence_argument_facts(
      services,
      scope,
      source_argument_texts,
      explicit_argument_syntaxes,
      resolved_arguments,
      request.template_id_occurrence);
  mark_alias_template_value_owner_argument_facts(
      services,
      scope.valid() ? &scope.require() : nullptr,
      alias_template,
      resolved_arguments,
      explicit_argument_syntaxes,
      request.template_id_occurrence);
  {
    const size_t limit =
        std::min(request.template_id_occurrence.arguments.size(),
                 resolved_arguments.size());
    for(size_t i = 0; i < limit; ++i) {
      const TemplateArgument & argument = resolved_arguments[i];
      const bool dependent_argument =
          argument.dependent ||
          (argument.kind == TemplateArgument::TA_TYPE &&
           argument.type &&
           service_type_depends_on_template_parameter(services,
                                                      argument.type));
      if(dependent_argument) {
        request.template_id_occurrence.arguments[i].dependent = true;
        request.template_id_occurrence.has_dependent_argument = true;
      }
      if(argument.kind == TemplateArgument::TA_TYPE && argument.type) {
        TypePtr owner;
        std::vector<std::string> members;
        bool leading_typename = false;
        if(named_type_dependent_qualified_member(argument.type,
                                                 owner,
                                                 members,
                                                 leading_typename,
                                                 nullptr) &&
           !members.empty()) {
          request.template_id_occurrence.arguments[i].preserve_qualified_member =
              true;
        }
      }
    }
  }
  request.template_name =
      template_api::alias_template_witness_entity(&alias_template);
  request.selected_decl_location =
      alias_template_decl_location(services.witness_context, alias_template);
  request.selected_decl_has_name_location =
      semantic_model::source_decl_anchor_has_name_location(
          alias_template.declaration_anchor);
  request.expanded_to = describe_type(resolved);
  request.origin = witness::AliasUseEmissionOrigin::DirectSourceTemplateId;
  append_alias_template_source_bindings(services,
                                        scope,
                                        request.bindings,
                                        alias_template.parameters,
                                        resolved_arguments,
                                        source_argument_texts,
                                        "explicit");
  mark_alias_source_binding_preserve_from_occurrence(
      request.bindings,
      request.template_id_occurrence);
  if(services.semantic_context && scope.valid()) {
    callsemantic::rewrite_current_specialization_alias_binding_texts(
        *services.semantic_context,
        scope.require(),
        alias_template.parameters,
        resolved_arguments,
        source_argument_texts,
        explicit_argument_syntaxes,
        request.bindings,
        &request.template_id_occurrence);
  }
  witness::emit_alias_use(services.witness_context, request);
  bool source_arguments_have_template_dependency = false;
  for(std::size_t i = 0; i < source_argument_texts.size(); ++i) {
    if(alias_argument_text_mentions_template_dependency(services,
                                                        scope,
                                                        source_argument_texts[i])) {
      source_arguments_have_template_dependency = true;
      break;
    }
  }
  if(!source_arguments_have_template_dependency &&
     services.semantic_context &&
     scope.valid()) {
    services.semantic_context
        ->emit_nested_class_use_source_events_from_template_arguments(
            scope.require(),
            explicit_argument_syntaxes ?
                *explicit_argument_syntaxes :
                std::vector<TemplateArgumentSyntax>(),
            semantic_source_use::SourceUseOwnership::SourceOwned);
  }
  record_nested_alias_template_source_uses_for_arguments(
      services,
      scope.require(),
      source_argument_texts,
      explicit_argument_syntaxes,
      use_location,
      request.template_name);
}

bool try_resolve_alias_template_id_locally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_api::TemplateTypeLookupRequest & request,
    const QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    TypePtr & out)
{
  out.reset();

  Scope & raw_scope = scope.require();
  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ? argument_scope : scope;
  Scope & raw_argument_scope = effective_argument_scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
  {
    return service_type_depends_on_template_parameter(services, type);
  };
  const auto type_text =
      [&services](const TypePtr & type) -> string
  {
    return service_lookup_text_for_type_argument(services, type);
  };
  const auto argument_text =
      [&type_text](const TemplateArgument & argument) -> string
  {
    return template_model::template_argument_text(argument, type_text);
  };
  AliasTemplateDecl * alias_template =
      lookup_alias_template_impl(services,
                                 raw_scope,
                                 template_api::qualified_name_text(template_id));
  if(!alias_template) {
    return false;
  }

  const auto dependent_alias_specialization =
      [&](std::vector<TemplateArgument> arguments) -> TypePtr
  {
    vector<string> dependent_source_arg_texts = arg_texts;
    vector<TemplateArgumentSyntax> dependent_source_arg_syntaxes;
    const vector<TemplateArgumentSyntax> * dependent_source_arg_syntaxes_ptr =
        arg_syntaxes;
    if(arg_texts.size() != arguments.size()) {
      ExpandedTemplateArgumentInputs expanded_source_args =
          expand_template_argument_inputs(services,
                                          raw_argument_scope,
                                          arg_texts,
                                          arg_syntaxes);
      if(expanded_source_args.texts.size() == arguments.size()) {
        dependent_source_arg_texts = expanded_source_args.texts;
        if(arg_syntaxes) {
          dependent_source_arg_syntaxes.reserve(expanded_source_args.texts.size());
          for(size_t i = 0; i < expanded_source_args.texts.size(); ++i) {
            if(const TemplateArgumentSyntax * syntax =
                   expanded_source_args.syntax_for(i)) {
              dependent_source_arg_syntaxes.push_back(*syntax);
            } else {
              TemplateArgumentSyntax fallback_syntax;
              fallback_syntax.text = dependent_source_arg_texts[i];
              dependent_source_arg_syntaxes.push_back(fallback_syntax);
            }
          }
          dependent_source_arg_syntaxes_ptr = &dependent_source_arg_syntaxes;
        }
      }
    }

    std::vector<size_t> stored_indices;
    stored_indices.reserve(arguments.size());
    for(size_t i = 0; i < arguments.size(); ++i) {
      if(arguments[i].source_defaulted) {
        continue;
      }
      stored_indices.push_back(i);
      if(arguments[i].kind != TemplateArgument::TA_TYPE ||
         !arguments[i].type ||
         !type_is_dependent(arguments[i].type) ||
         !is_simple_dependent_argument_text(arguments[i].text)) {
        continue;
      }
      const string canonical = type_text(arguments[i].type);
      if(!canonical.empty()) {
        arguments[i].text = canonical;
      }
    }

    ostringstream specialization_name;
    specialization_name << alias_template->name << "<";
    for(size_t i = 0; i < stored_indices.size(); ++i) {
      if(i != 0) {
        specialization_name << ", ";
      }
      specialization_name << argument_text(arguments[stored_indices[i]]);
    }
    specialization_name << ">";

    const string qualified_name =
        alias_template->declaring_scope ?
            semantic_lookup::scope_qualified_name(*alias_template->declaring_scope,
                                                  specialization_name.str()) :
            specialization_name.str();
    vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
    dependent_arguments.reserve(stored_indices.size());
    for(size_t i = 0; i < stored_indices.size(); ++i) {
      const size_t source_index = stored_indices[i];
      DependentAliasTemplateArgumentSyntax dependent_argument;
      dependent_argument.text =
          source_index < dependent_source_arg_texts.size() ?
              trim_space(dependent_source_arg_texts[source_index]) :
              argument_text(arguments[source_index]);
      if(arguments[source_index].kind == TemplateArgument::TA_TYPE) {
        dependent_argument.type = arguments[source_index].type;
      }
      if(dependent_source_arg_syntaxes_ptr &&
         source_index < dependent_source_arg_syntaxes_ptr->size()) {
        dependent_argument.syntax =
            (*dependent_source_arg_syntaxes_ptr)[source_index];
      } else {
        dependent_argument.syntax.text = dependent_argument.text;
      }
      if(arguments[source_index].kind == TemplateArgument::TA_VALUE &&
         arguments[source_index].expression &&
         !dependent_argument.syntax.expression) {
        dependent_argument.syntax.text = dependent_argument.text;
        dependent_argument.syntax.source_location_id =
            arguments[source_index].expression->source_location_id;
        dependent_argument.syntax.expression.reset(
            new CppAstNode(*arguments[source_index].expression));
      }
      dependent_arguments.push_back(dependent_argument);
    }
    return make_dependent_alias_type(specialization_name.str(),
                                     qualified_name,
                                     alias_template,
                                     dependent_arguments);
  };

  const auto is_dependent_conditional_alias_candidate =
      [&]() -> bool
  {
    if(arg_texts.size() != 3 ||
       alias_template->parameters.size() != 3 ||
       (alias_template->name != "conditional_t" &&
        alias_template->name != "__conditional_t")) {
      return false;
    }
    return alias_template->parameters[0].kind == TemplateParameterInfo::TP_NON_TYPE &&
           alias_template->parameters[1].kind == TemplateParameterInfo::TP_TYPE &&
           alias_template->parameters[2].kind == TemplateParameterInfo::TP_TYPE;
  };

  const auto try_defer_dependent_conditional_alias =
      [&](std::vector<TemplateArgument> & arguments, TypePtr & deferred) -> bool
  {
    arguments.clear();
    deferred.reset();
    if(!is_dependent_conditional_alias_candidate()) {
      return false;
    }

    TemplateArgument condition_argument;
    const TemplateArgumentSyntax * condition_syntax =
        arg_syntaxes && !arg_syntaxes->empty() ? &(*arg_syntaxes)[0] : nullptr;
    Scope bound_scope(&raw_argument_scope, std::string(), false);
    try {
      const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
          function_call_source_capture_pause;
      if(!template_resolution::resolve_template_argument(
             services,
             effective_argument_scope,
             template_api::make_template_environment(bound_scope),
             alias_template->parameters[0],
             arg_texts[0],
             condition_syntax,
             condition_argument)) {
        return false;
      }
    } catch(const TemplateSubstitutionFailure &) {
      return false;
    } catch(const SemanticSoftFailure &) {
      return false;
    } catch(const SemanticDiagnosticError &) {
      return false;
    }

    if(condition_argument.kind != TemplateArgument::TA_VALUE ||
       !condition_argument.dependent) {
      return false;
    }

    arguments.reserve(3);
    arguments.push_back(condition_argument);
    for(size_t i = 1; i < 3; ++i) {
      const string branch_text = trim_space(arg_texts[i]);
      if(branch_text.empty()) {
        arguments.clear();
        return false;
      }
      TemplateArgument branch_argument;
      const TemplateArgumentSyntax * branch_syntax =
          arg_syntaxes && i < arg_syntaxes->size() ? &(*arg_syntaxes)[i] : nullptr;
      bool resolved_branch = false;
      try {
        const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
            function_call_source_capture_pause;
        resolved_branch =
            template_resolution::resolve_template_argument(
                services,
                effective_argument_scope,
                template_api::make_template_environment(bound_scope),
                alias_template->parameters[i],
                branch_text,
                branch_syntax,
                branch_argument) &&
            branch_argument.kind == TemplateArgument::TA_TYPE &&
            branch_argument.type;
      } catch(const TemplateSubstitutionFailure &) {
        resolved_branch = false;
      } catch(const SemanticSoftFailure &) {
        resolved_branch = false;
      } catch(const SemanticDiagnosticError &) {
        resolved_branch = false;
      }
      if(!resolved_branch) {
        branch_argument.kind = TemplateArgument::TA_TYPE;
        branch_argument.text = branch_text;
        branch_argument.type =
            make_semantic_named(branch_text, Type::NSK_DEPENDENT_TYPE, branch_text, true);
        branch_argument.dependent = true;
      }
      arguments.push_back(branch_argument);
    }

    deferred = dependent_alias_specialization(arguments);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "alias-conditional-defer name=" << alias_template->name
            << " condition=" << condition_argument.text
            << " type=" << (deferred ? describe_type(deferred) : std::string("<null>"));
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return deferred != nullptr;
  };

  std::vector<TemplateArgument> resolved_arguments;
  TypePtr deferred_conditional_alias;
  if(try_defer_dependent_conditional_alias(resolved_arguments,
                                           deferred_conditional_alias)) {
    out = deferred_conditional_alias;
  }

  if(!out) {
    {
      const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
          function_call_source_capture_pause;
      if(!template_api::resolve_template_arguments(
             services,
             effective_argument_scope,
             alias_template->parameters,
             arg_texts,
             arg_syntaxes,
             resolved_arguments,
             alias_template->declaring_scope ?
                 template_api::make_template_environment(*alias_template->declaring_scope) :
                 template_api::TemplateEnvironmentHandle())) {
        return false;
      }
    }

    const bool dependent_arguments =
        template_arguments_are_dependent(
           resolved_arguments,
           [&type_is_dependent](const TypePtr & type)
           {
             return type_is_dependent(type);
           });

    const auto resolve_alias_type_id_ast =
        [&](TypePtr & resolved, bool allow_dependent_result) -> bool
  {
    resolved.reset();
    if(!alias_template->type_id || !alias_template->declaring_scope) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "alias-type-id-ast-skip name=" << alias_template->name
              << " reason=" << (!alias_template->type_id ? "no-type-id" : "no-decl-scope");
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }

    std::set<std::string> excluded_names;
    for(size_t i = 0; i < alias_template->parameters.size(); ++i) {
      if(!alias_template->parameters[i].name.empty()) {
        excluded_names.insert(alias_template->parameters[i].name);
      }
      for(size_t j = 0; j < alias_template->parameters[i].alternate_names.size(); ++j) {
        if(!alias_template->parameters[i].alternate_names[j].empty()) {
          excluded_names.insert(alias_template->parameters[i].alternate_names[j]);
        }
      }
    }

    Scope inst_scope(alias_template->declaring_scope, std::string(), false);
    template_instantiation::overlay_instantiation_use_scope_bindings(
        inst_scope, raw_scope, alias_template->declaring_scope, excluded_names);
    template_instantiation::overlay_instantiation_local_named_types(
        services,
        inst_scope,
        raw_scope,
        alias_template->declaring_scope,
        resolved_arguments,
        &excluded_names);
    template_instantiation::bind_template_arguments_into_scope(
        services,
        inst_scope,
        alias_template->parameters,
        resolved_arguments);

    TypePtr parsed;
    {
      const witness::ScopedTemplateWitnessSourceCapturePause
          source_capture_pause;
      if(!template_decl_ast::parse_type_id(
             services,
             inst_scope,
             inst_scope,
             *alias_template->type_id,
             parsed,
             false) ||
         !parsed) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "alias-type-id-ast-failed name=" << alias_template->name;
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return false;
      }
    }

    resolve_instantiated_dependent_type_if_needed(
        services, template_api::make_template_environment(inst_scope), parsed);
    const bool parsed_is_dependent =
        parsed &&
        type_is_dependent(parsed);
    if(!parsed || parsed_is_dependent) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "alias-type-id-ast-dependent name=" << alias_template->name
              << " type=" << (parsed ? describe_type(parsed) : std::string("<null>"));
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!allow_dependent_result || !parsed) {
        return false;
      }
    }
    resolved = parsed;
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "alias-type-id-ast-resolved name=" << alias_template->name
            << " type=" << describe_type(resolved);
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return true;
  };

  const auto validate_non_propagating_dependent_alias_arguments =
      [&]() -> bool
  {
    for(size_t i = 0; i < arg_texts.size() && i < alias_template->parameters.size(); ++i) {
      if(alias_template->parameters[i].kind != TemplateParameterInfo::TP_TYPE) {
        continue;
      }
      TypePtr validated_argument_type;
      try {
        bool resolved = false;
        const TemplateArgumentSyntax * arg_syntax =
            arg_syntaxes && i < arg_syntaxes->size() ? &(*arg_syntaxes)[i] : nullptr;
        if(arg_syntaxes && i < arg_syntaxes->size()) {
          const witness::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          resolved = resolve_type_argument_syntax_type(services,
                                                       effective_argument_scope,
                                                       *arg_syntax,
                                                       true,
                                                       validated_argument_type);
        }
        if(!resolved || !validated_argument_type) {
          return false;
        }
      } catch(const TemplateSubstitutionFailure &) {
        return false;
      } catch(const SemanticSoftFailure &) {
        return false;
      } catch(const SemanticDiagnosticError &) {
        return false;
      }

      QualifiedName nested_template_id;
      std::vector<std::string> nested_arg_texts;
      const std::vector<TemplateArgumentSyntax> * nested_arg_syntaxes = nullptr;
      const bool validated_argument_is_dependent =
          service_type_depends_on_template_parameter(services, validated_argument_type);
      if(arg_syntaxes &&
         i < arg_syntaxes->size() &&
         (*arg_syntaxes)[i].template_id) {
        nested_template_id = (*arg_syntaxes)[i].template_id->name;
        nested_arg_texts = (*arg_syntaxes)[i].template_id->arguments;
        nested_arg_syntaxes = &(*arg_syntaxes)[i].template_id->argument_syntaxes;
      } else if(!semantic_utils::split_top_level_template_id_text(
                    trim_space(arg_texts[i]), nested_template_id, nested_arg_texts)) {
        nested_arg_texts.clear();
      }
      if(!validated_argument_is_dependent && !nested_arg_texts.empty()) {
        const witness::ScopedTemplateWitnessSourceCapturePause
            source_capture_pause;
        TypePtr nested_expanded_type;
        if(template_specialization::expand_alias_template_pattern_type(
               services,
               scope,
               nested_template_id,
               nested_arg_texts,
               nested_expanded_type,
               nested_arg_syntaxes,
               effective_argument_scope,
               false) &&
           (!nested_expanded_type ||
            service_type_depends_on_template_parameter(services,
                                                       nested_expanded_type))) {
          return false;
        }
      }
    }
    return true;
  };
  const auto dependent_argument_mentions_alias_parameter =
      [&](const std::string & text) -> bool
  {
    for(size_t i = 0; i < alias_template->parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = alias_template->parameters[i];
      if(!parameter.name.empty() &&
         callsemantic_internal::contains_identifier_token(text, parameter.name)) {
        return true;
      }
      if(!parameter.placeholder_key.empty() &&
         callsemantic_internal::contains_identifier_token(text,
                                                         parameter.placeholder_key)) {
        return true;
      }
      for(size_t j = 0; j < parameter.alternate_names.size(); ++j) {
        if(!parameter.alternate_names[j].empty() &&
           callsemantic_internal::contains_identifier_token(
               text, parameter.alternate_names[j])) {
          return true;
        }
      }
    }
    return false;
  };
  const auto has_stale_concrete_dependent_alias_argument =
      [&]() -> bool
  {
    for(size_t i = 0; i < resolved_arguments.size(); ++i) {
      if(!resolved_arguments[i].dependent) {
        continue;
      }
      const std::string text =
          i < arg_texts.size() ? arg_texts[i] : resolved_arguments[i].text;
      if(text.empty()) {
        return true;
      }
      if(text_mentions_template_placeholders(services, effective_argument_scope, text) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, effective_argument_scope, text) ||
         dependent_argument_mentions_alias_parameter(text)) {
        continue;
      }
      return true;
    }
    return false;
  };
  const auto alias_pattern_is_dependent_member =
      [&]() -> bool
  {
    if(!alias_template->resolved_type_pattern) {
      return false;
    }
    TypePtr owner;
    vector<string> members;
    bool leading_typename = false;
    return named_type_dependent_qualified_member(
               alias_template->resolved_type_pattern,
               owner,
               members,
               leading_typename) &&
           !members.empty();
  };
  const auto dependent_member_alias_needs_deferred_surface =
      [&]() -> bool
  {
    if(!alias_pattern_is_dependent_member()) {
      return false;
    }
    for(size_t i = 0; i < resolved_arguments.size(); ++i) {
      const TemplateArgument & argument = resolved_arguments[i];
      const TemplateArgumentSyntax * syntax =
          arg_syntaxes && i < arg_syntaxes->size() ? &(*arg_syntaxes)[i] : nullptr;
      if(argument.kind == TemplateArgument::TA_VALUE) {
        if(argument.dependent ||
           argument.expression ||
           (syntax && (syntax->dependent || syntax->expression))) {
          return true;
        }
        continue;
      }
      if(argument.kind != TemplateArgument::TA_TYPE || !argument.type) {
        continue;
      }
      TypePtr owner;
      vector<string> members;
      bool leading_typename = false;
      void * dependent_template = nullptr;
      vector<DependentAliasTemplateArgumentSyntax> dependent_args;
      if(named_type_dependent_alias_template(argument.type,
                                             dependent_template,
                                             dependent_args) ||
         named_type_dependent_qualified_member(argument.type,
                                               owner,
                                               members,
                                               leading_typename)) {
        return true;
      }
      if(service_type_depends_on_template_parameter(services, argument.type) &&
         !named_type_is_template_parameter(argument.type)) {
        return true;
      }
    }
    return false;
  };

  std::string expanded_text;
  template_specialization::AliasSubstitutionFailure alias_substitution_failure;
  const bool expanded_alias =
      template_specialization::expand_alias_template_pattern_id(
          services,
          scope,
          template_api::qualified_name_text(request.name),
          template_id,
          arg_texts,
          expanded_text,
          arg_syntaxes,
          effective_argument_scope,
          &alias_substitution_failure);
  const auto resolve_structural_alias_type =
      [&](TypePtr & structural_alias) -> bool
  {
    template_specialization::AliasSubstitutionFailure structural_substitution_failure;
    return template_specialization::expand_alias_template_pattern_type(
        services,
        scope,
        template_id,
        arg_texts,
        structural_alias,
        arg_syntaxes,
        effective_argument_scope,
        false,
        &structural_substitution_failure) &&
           structural_alias &&
           !service_type_depends_on_template_parameter(services, structural_alias);
  };

  if(!dependent_arguments) {
    if(alias_substitution_failure.active()) {
      throw_substitution_failure(
          string("alias template substitution failed [alias ") +
              alias_template->name + "]",
          string(),
          "template-resolution");
    }
    TypePtr ast_alias;
    if(resolve_alias_type_id_ast(ast_alias, false)) {
      out = ast_alias;
    } else {
      TypePtr structural_alias;
      if(resolve_structural_alias_type(structural_alias)) {
        out = structural_alias;
      } else {
        if(alias_template->type_id && alias_template->declaring_scope) {
          return false;
        }
        if(!expanded_alias) {
          return false;
        }
        return false;
      }
    }
  } else {
    std::string candidate_text = expanded_text;
    TypePtr ast_alias;
    if(candidate_text.empty() &&
       has_stale_concrete_dependent_alias_argument()) {
      return false;
    }
    if(candidate_text.empty() &&
       !validate_non_propagating_dependent_alias_arguments()) {
      return false;
    }
    if(dependent_member_alias_needs_deferred_surface()) {
      out = dependent_alias_specialization(resolved_arguments);
    } else if(resolve_alias_type_id_ast(ast_alias, true) && ast_alias) {
      out = ast_alias;
    } else if(alias_template->type_id && alias_template->declaring_scope) {
      out = dependent_alias_specialization(resolved_arguments);
    } else {
      out = dependent_alias_specialization(resolved_arguments);
    }
    if(service_type_depends_on_template_parameter(services, out)) {
      out = dependent_alias_specialization(resolved_arguments);
    }
  }
  }

  if(request.top_const || request.top_volatile) {
    out = apply_cv(out, request.top_const, request.top_volatile);
  }
  template_api::TemplateEnvironmentHandle source_use_scope =
      effective_argument_scope.valid() ? effective_argument_scope : scope;
  record_direct_alias_template_source_use_if_needed(services,
                                                    source_use_scope,
                                                    *alias_template,
                                                    template_id,
                                                    out,
                                                    resolved_arguments,
                                                    arg_texts,
                                                    arg_syntaxes,
                                                    request.source_location);
  return out != nullptr;
}

bool try_resolve_class_template_id_locally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const template_api::TemplateTypeLookupRequest & request,
    const string & original_text,
    ClassTemplateDecl * class_template,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    template_api::TemplateEnvironmentHandle argument_scope,
    TypePtr & out)
{
  out.reset();
  if(!class_template) {
    return false;
  }
  template_api::TemplateEnvironmentHandle effective_argument_scope =
      argument_scope.valid() ? argument_scope : scope;

  std::vector<TemplateArgument> resolved_arguments;
  if(!template_api::resolve_template_arguments(
         services,
         effective_argument_scope,
         class_template->parameters,
         arg_texts,
         arg_syntaxes,
         resolved_arguments,
         class_template->declaring_scope ?
             template_api::make_template_environment(*class_template->declaring_scope) :
             template_api::TemplateEnvironmentHandle())) {
    return false;
  }
  if(arg_syntaxes &&
     services.witness_context.session != nullptr) {
    const size_t count =
        std::min(class_template->parameters.size(),
                 std::min(arg_syntaxes->size(), resolved_arguments.size()));
    for(size_t i = 0; i < count; ++i) {
      const TemplateParameterInfo & parameter = class_template->parameters[i];
      const TemplateArgument & argument = resolved_arguments[i];
      if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
         argument.kind != TemplateArgument::TA_VALUE ||
         argument.dependent) {
        continue;
      }
      TypePtr value_type = argument.type ? argument.type : parameter.value_type;
      if(!value_type) {
        (void)template_resolution::resolve_non_type_template_parameter_type(
            services,
            class_template->declaring_scope ?
                template_api::make_template_environment(*class_template->declaring_scope) :
                effective_argument_scope,
            parameter,
            value_type);
      }
      vector<TemplateValueDependency> dependencies;
      append_non_bool_static_value_dependencies_in_template_argument_syntax(
          services,
          effective_argument_scope,
          (*arg_syntaxes)[i],
          value_type,
          dependencies);
      note_template_value_dependencies_for_witness(*services.semantic_context,
                                                   dependencies);
    }
  }
  template_api::TemplateSelectedClassTemplateIdRequest selected_request;
  selected_request.lookup = request;
  selected_request.argument_scope = effective_argument_scope.valid() ?
      &effective_argument_scope.require() :
      nullptr;
  selected_request.class_template = class_template;
  if(template_arguments_are_dependent(
         resolved_arguments,
         [&services](const TypePtr & type)
         {
           return service_type_depends_on_template_parameter(services, type);
         })) {
    selected_request.lookup.allow_class_templates = true;
  }
  selected_request.resolved_arguments.swap(resolved_arguments);
  selected_request.source_arg_texts = arg_texts;
  if(arg_syntaxes) {
    selected_request.source_arg_syntaxes.reserve(arg_syntaxes->size());
    for(size_t i = 0; i < arg_syntaxes->size(); ++i) {
      selected_request.source_arg_syntaxes.push_back(
          clone_argument_syntax_for_template_substitution((*arg_syntaxes)[i]));
    }
    const size_t count = std::min(selected_request.source_arg_syntaxes.size(),
                                  selected_request.resolved_arguments.size());
    for(size_t i = 0; i < count; ++i) {
      if(selected_request.resolved_arguments[i].dependent) {
        selected_request.source_arg_syntaxes[i].dependent = true;
      }
      preserve_resolved_qualified_type_in_argument_syntax(
          services,
          effective_argument_scope,
          selected_request.resolved_arguments[i],
          selected_request.source_arg_syntaxes[i]);
    }
  }
  return service_resolve_selected_class_template_id(services, selected_request, out);
}

bool type_contains_function_local_class(template_api::TemplateTypeSystem & type_system,
                                        const TypePtr & type)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;

  case Type::TK_NAMED:
  {
    ClassInfo * info = template_api::find_named_type_class_info(
        type_system.model,
        type);
    return info &&
           !info->is_lambda_closure &&
           info->class_kind != "union" &&
           info->enclosing_scope &&
           info->enclosing_scope->function != nullptr;
  }

  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_contains_function_local_class(type_system, type->inner);

  case Type::TK_MEMBER_POINTER:
    return type_contains_function_local_class(type_system, type->owner) ||
           type_contains_function_local_class(type_system, type->inner);

  case Type::TK_FUNCTION:
    if(type_contains_function_local_class(type_system, type->inner)) {
      return true;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(type_contains_function_local_class(type_system, type->params[i])) {
        return true;
      }
    }
    return false;
  }

  return false;
}

bool should_defer_unresolved_type_lookup(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const string & text)
{
  const template_api::TemplateEnvironmentHandle env =
      template_api::make_template_environment(scope);
  return text_mentions_template_placeholders(services, env, text) ||
         text_mentions_dependent_non_namespace_binding_names(services, env, text);
}

string dependency_check_text_for_ast_value(const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier || node.value.empty()) {
    return string();
  }
  if(node.kind == CppAstKind::id_expression) {
    const QualifiedName * qualified = qualified_syntax_if_qualified(node);
    if(qualified) {
      if(qualified->qualifiers.empty()) {
        return string();
      }
      string out = qualified->rooted ? string("::") : string();
      for(size_t i = 0; i < qualified->qualifiers.size(); ++i) {
        if(i != 0) {
          out += "::";
        }
        out += qualified->qualifiers[i];
      }
      return out;
    }
  }
  return node.value;
}

bool initializer_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    bool check_node_text)
{
  const string node_dependency_text =
      check_node_text ? dependency_check_text_for_ast_value(node) : string();
  if(!node_dependency_text.empty()) {
    if(text_mentions_template_placeholders(services, scope, node_dependency_text) ||
       text_mentions_dependent_non_namespace_binding_names(
           services, scope, node_dependency_text)) {
      return true;
    }
  }
  if(node.semantic_type &&
     service_type_depends_on_template_parameter(services, node.semantic_type)) {
    TypePtr resolved_semantic_type;
    if(resolve_instantiated_dependent_type(
           services,
           scope,
           node.semantic_type,
           resolved_semantic_type) &&
       resolved_semantic_type &&
       !service_type_depends_on_template_parameter(services, resolved_semantic_type)) {
      // Keep inspecting child nodes; the retained semantic type has been
      // resolved by the current instantiation bindings.
    } else {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(initializer_ast_mentions_template_dependency(
           services,
           scope,
           node.children[i],
           check_node_text)) {
      return true;
    }
  }
  return false;
}

bool value_binding_depends_on_template_parameters(
    template_api::TemplateServices & services,
    const ValueBinding & binding)
{
  static thread_local set<const ValueBinding *> visiting_bindings;
  if(visiting_bindings.count(&binding) != 0) {
    return false;
  }
  struct VisitingBindingGuard
  {
    set<const ValueBinding *> & visiting;
    const ValueBinding * binding;
    VisitingBindingGuard(set<const ValueBinding *> & visiting_in,
                         const ValueBinding * binding_in)
      : visiting(visiting_in),
        binding(binding_in)
    {
      visiting.insert(binding);
    }
    ~VisitingBindingGuard()
    {
      visiting.erase(binding);
    }
  } guard(visiting_bindings, &binding);

  if(binding.dependent_template_value ||
     service_type_depends_on_template_parameter(services, binding.type)) {
    return true;
  }
  if(binding.has_constant_value || binding.has_constexpr_value) {
    return false;
  }
  if(!binding.constant_initializer || !binding.constant_initializer_scope) {
    return false;
  }
  return initializer_ast_mentions_template_dependency(
      services,
      template_api::make_template_environment(*binding.constant_initializer_scope),
      *binding.constant_initializer,
      true);
}

bool lookup_pack_size(Scope & scope, const string & name, size_t & out)
{
  for(Scope * current = &scope; current != nullptr; current = current->parent) {
    map<string, size_t>::const_iterator found = current->named_pack_sizes.find(name);
    if(found != current->named_pack_sizes.end()) {
      out = found->second;
      return true;
    }
  }
  return false;
}

TypePtr lookup_exact_bound_type_name(Scope & scope, const string & name)
{
  const string trimmed = trim_space(name);
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    map<string, TypePtr>::const_iterator found = current->named_types.find(trimmed);
    if(found != current->named_types.end()) {
      if(current->template_bound_type_names.count(trimmed) != 0 && found->second) {
        return found->second;
      }
      break;
    }
  }
  return TypePtr();
}

bool lookup_exact_bound_type_name_with_scope(Scope & scope,
                                             const string & name,
                                             Scope *& bound_scope,
                                             TypePtr & bound_type)
{
  const string trimmed = trim_space(name);
  bound_scope = nullptr;
  bound_type.reset();
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    map<string, TypePtr>::const_iterator found = current->named_types.find(trimmed);
    if(found != current->named_types.end()) {
      if(current->template_bound_type_names.count(trimmed) != 0 && found->second) {
        bound_scope = current;
        bound_type = found->second;
        return true;
      }
      break;
    }
  }
  return false;
}

bool type_contains_partial_order_artifact(const TypePtr & type)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;

  case Type::TK_NAMED:
    return named_type_is_partial_order_placeholder(type) ||
           named_type_key_contains_partial_order_placeholder(type);

  case Type::TK_ATOMIC:
  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_ARRAY:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_LVALUE_REFERENCE:
    return type_contains_partial_order_artifact(type->inner);

  case Type::TK_MEMBER_POINTER:
    return type_contains_partial_order_artifact(type->inner) ||
           type_contains_partial_order_artifact(type->owner);

  case Type::TK_FUNCTION:
    if(type_contains_partial_order_artifact(type->inner)) {
      return true;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(type_contains_partial_order_artifact(type->params[i])) {
        return true;
      }
    }
    return false;
  }

  return false;
}

bool is_simple_dependent_argument_text(const string & text)
{
  string trimmed = trim_space(text);
  const char * prefixes[] = {"typename ", "class ", "struct "};
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix = prefixes[i];
    if(trimmed.compare(0, prefix.size(), prefix) == 0) {
      trimmed = trim_space(trimmed.substr(prefix.size()));
      break;
    }
  }
  if(trimmed.empty()) {
    return false;
  }
  if(!((trimmed[0] >= 'a' && trimmed[0] <= 'z') ||
       (trimmed[0] >= 'A' && trimmed[0] <= 'Z') ||
       trimmed[0] == '_')) {
    return false;
  }
  for(size_t i = 1; i < trimmed.size(); ++i) {
    const char c = trimmed[i];
    if(!((c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
         c == '_')) {
      return false;
    }
  }
  return true;
}

bool try_parse_builtin_type_trait_text(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const string & text,
                                       string & trait_name,
                                       vector<TypePtr> & types)
{
  trait_name.clear();
  types.clear();

  const string trimmed = trim_space(text);
  const size_t lparen = trimmed.find('(');
  if(lparen == string::npos || lparen == 0 || trimmed.empty() || trimmed.back() != ')') {
    return false;
  }

  trait_name = trim_space(trimmed.substr(0, lparen));
  if(!semantic_builtins::is_supported_builtin_type_trait_name(trait_name)) {
    trait_name.clear();
    return false;
  }

  string args_text = trimmed.substr(lparen + 1, trimmed.size() - lparen - 2);
  vector<string> arg_texts;
  if(args_text.empty()) {
    return true;
  }

  string current;
  int angle_depth = 0;
  int paren_depth = 0;
  int square_depth = 0;
  for(size_t i = 0; i < args_text.size(); ++i) {
    const char c = args_text[i];
    if(c == '<') {
      ++angle_depth;
    } else if(c == '>') {
      if(angle_depth == 0) {
        return false;
      }
      --angle_depth;
    } else if(c == '(') {
      ++paren_depth;
    } else if(c == ')') {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
    } else if(c == '[') {
      ++square_depth;
    } else if(c == ']') {
      if(square_depth == 0) {
        return false;
      }
      --square_depth;
    }

    if(c == ',' && angle_depth == 0 && paren_depth == 0 && square_depth == 0) {
      if(current.empty()) {
        return false;
      }
      arg_texts.push_back(current);
      current.clear();
      continue;
    }
    current += c;
  }

  if(angle_depth != 0 || paren_depth != 0 || square_depth != 0 || current.empty()) {
    return false;
  }
  arg_texts.push_back(current);

  const auto resolve_trait_arg_type =
      [&](const string & raw_text, TypePtr & out) -> bool
      {
        out.reset();
        const string trimmed_arg = trim_space(raw_text);
        if(callsemantic_internal::has_invalid_top_level_qualified_owner_syntax(trimmed_arg)) {
          return false;
        }
        if(!services.semantic_context) {
          return false;
        }
        out = services.semantic_context->lookup_type(scope, trimmed_arg, true);
        return out != nullptr;
      };

  const vector<ExpandedTypeArgumentInput> expanded_args =
      expand_bound_type_pack_arguments(services, scope, arg_texts);
  for(size_t i = 0; i < expanded_args.size(); ++i) {
    TypePtr type = expanded_args[i].type;
    if(!type) {
      if(!resolve_trait_arg_type(expanded_args[i].text, type)) {
        return false;
      }
    }
    types.push_back(type);
  }
  return true;
}

TypePtr lookup_exact_local_type_name_impl(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const string & name)
{
  const string trimmed = trim_space(name);
  if(trimmed.find("::") != string::npos) {
    return TypePtr();
  }
  vector<string> candidate_names;
  const auto add_candidate_name =
      [&](const string & candidate)
  {
    const string normalized = trim_space(strip_elaborated_type_prefix(candidate));
    if(normalized.empty()) {
      return;
    }
    if(find(candidate_names.begin(), candidate_names.end(), normalized) ==
       candidate_names.end()) {
      candidate_names.push_back(normalized);
    }
    const string unqualified = unqualified_member_name(normalized);
    if(!unqualified.empty() &&
       find(candidate_names.begin(), candidate_names.end(), unqualified) ==
           candidate_names.end()) {
      candidate_names.push_back(unqualified);
    }
  };
  add_candidate_name(trimmed);

  const auto is_function_local_type =
      [&](const TypePtr & type) -> bool
  {
    if(!type) {
      return false;
    }
    TypePtr base = strip_top_level_cv(remove_reference_type(type));
    if(!base) {
      return false;
    }
    ClassInfo * info = template_api::find_named_type_class_info(
        service_type_system(services).model,
        base);
    if(info && info->enclosing_scope && info->enclosing_scope->function != nullptr) {
      return true;
    }
    return named_type_has_function_local_marker(base);
  };

  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(size_t i = 0; i < candidate_names.size(); ++i) {
      map<string, TypePtr>::const_iterator found =
          current->named_types.find(candidate_names[i]);
      if(found == current->named_types.end() ||
         !is_function_local_type(found->second)) {
        continue;
      }
      map<string, TypePtr>::const_iterator parent_found =
          current->parent->named_types.find(candidate_names[i]);
      if(parent_found == current->parent->named_types.end() ||
         !type_equals(found->second, parent_found->second)) {
        return found->second;
      }
    }
    for(map<string, TypePtr>::const_iterator it = current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      if(!is_function_local_type(it->second)) {
        continue;
      }
      const string reparseable = trim_space(reparseable_type_argument_text(it->second));
      for(size_t i = 0; i < candidate_names.size(); ++i) {
        if(reparseable != candidate_names[i]) {
          continue;
        }
        map<string, TypePtr>::const_iterator parent_found =
            current->parent->named_types.find(it->first);
        if(parent_found == current->parent->named_types.end() ||
           !type_equals(it->second, parent_found->second)) {
          return it->second;
        }
      }
    }
  }
  return TypePtr();
}

bool strip_leading_typename_text(const string & text, string & stripped);

bool has_invalid_top_level_qualified_owner_syntax(const string & text)
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
      size_t next = i + 1;
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

bool parse_sizeof_pack_text(const string & text, string & pack_name)
{
  pack_name.clear();
  const string trimmed = trim_space(text);
  const string prefix = "sizeof...(";
  if(trimmed.size() <= prefix.size() ||
     trimmed.compare(0, prefix.size(), prefix) != 0 ||
     trimmed[trimmed.size() - 1] != ')') {
    return false;
  }
  const string inner = trim_space(trimmed.substr(prefix.size(),
                                                 trimmed.size() - prefix.size() - 1));
  if(inner.empty()) {
    return false;
  }
  for(size_t i = 0; i < inner.size(); ++i) {
    const char ch = inner[i];
    if(!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
      return false;
    }
  }
  pack_name = inner;
  return true;
}

bool parse_integer_literal_count_text(const string & text, long long & out)
{
  const string trimmed = trim_space(text);
  if(trimmed.empty() ||
     !std::isdigit(static_cast<unsigned char>(trimmed[0]))) {
    return false;
  }
  unsigned long long value = 0;
  string ud_suffix;
  try {
    const EFundamentalType type = classify_int(trimmed, value, ud_suffix);
    if(type == FT_VOID || !ud_suffix.empty() ||
       value > static_cast<unsigned long long>(
                   std::numeric_limits<long long>::max())) {
      return false;
    }
  } catch(const logic_error &) {
    return false;
  }
  out = static_cast<long long>(value);
  return true;
}

string compact_expression_text(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    if(!std::isspace(static_cast<unsigned char>(text[i]))) {
      out.push_back(text[i]);
    }
  }
  return out;
}

  bool parse_simple_signed_integer_text(const string & text, long long & out)
  {
    if(text.empty()) {
      return false;
    }
  size_t pos = 0;
  int sign = 1;
  if(text[pos] == '+') {
    ++pos;
  } else if(text[pos] == '-') {
    sign = -1;
    ++pos;
  }
    if(pos >= text.size() ||
       !std::isdigit(static_cast<unsigned char>(text[pos]))) {
      return false;
    }
    unsigned long long value = 0;
    string ud_suffix;
    try {
      const EFundamentalType type = classify_int(text.substr(pos), value, ud_suffix);
      if(type == FT_VOID || !ud_suffix.empty()) {
        return false;
      }
    } catch(const logic_error &) {
      return false;
    }
    if(sign < 0) {
      if(value > static_cast<unsigned long long>(std::numeric_limits<long long>::max()) + 1ULL) {
        return false;
      }
      if(value == static_cast<unsigned long long>(std::numeric_limits<long long>::max()) + 1ULL) {
        out = std::numeric_limits<long long>::min();
      } else {
        out = -static_cast<long long>(value);
      }
  } else {
    if(value > static_cast<unsigned long long>(std::numeric_limits<long long>::max())) {
      return false;
    }
    out = static_cast<long long>(value);
  }
  return true;
}

bool strip_balanced_outer_parens(string & text)
{
  bool stripped = false;
  while(text.size() >= 2 && text.front() == '(' && text.back() == ')') {
    int depth = 0;
    bool wraps = true;
    for(size_t i = 0; i < text.size(); ++i) {
      if(text[i] == '(') {
        ++depth;
      } else if(text[i] == ')') {
        --depth;
        if(depth == 0 && i + 1 != text.size()) {
          wraps = false;
          break;
        }
        if(depth < 0) {
          wraps = false;
          break;
        }
      }
    }
    if(!wraps || depth != 0) {
      break;
    }
    text = text.substr(1, text.size() - 2);
    stripped = true;
  }
  return stripped;
}

bool try_evaluate_static_cast_integral_text(const string & text, long long & out)
{
  const string compact = compact_expression_text(text);
  const string prefix = "static_cast<";
  if(compact.compare(0, prefix.size(), prefix) != 0) {
    return false;
  }
  size_t depth = 0;
  size_t close_angle = string::npos;
  for(size_t i = prefix.size(); i < compact.size(); ++i) {
    if(compact[i] == '<') {
      ++depth;
    } else if(compact[i] == '>') {
      if(depth == 0) {
        close_angle = i;
        break;
      }
      --depth;
    }
  }
  if(close_angle == string::npos ||
     close_angle + 2 > compact.size() ||
     compact[close_angle + 1] != '(' ||
     compact.back() != ')') {
    return false;
  }

  string operand = compact.substr(close_angle + 2,
                                  compact.size() - close_angle - 3);
  strip_balanced_outer_parens(operand);
  if(parse_simple_signed_integer_text(operand, out)) {
    return true;
  }

  if(!operand.empty() && operand[0] == '(') {
    int paren_depth = 0;
    for(size_t i = 0; i < operand.size(); ++i) {
      if(operand[i] == '(') {
        ++paren_depth;
      } else if(operand[i] == ')') {
        --paren_depth;
        if(paren_depth == 0) {
          string suffix = operand.substr(i + 1);
          strip_balanced_outer_parens(suffix);
          return parse_simple_signed_integer_text(suffix, out);
        }
      }
    }
  }
  return false;
}

bool parse_simple_integral_constant_text(string text, long long & out)
{
  strip_balanced_outer_parens(text);
  if(text == "true") {
    out = 1;
    return true;
  }
  if(text == "false") {
    out = 0;
    return true;
  }
  return parse_simple_signed_integer_text(text, out);
}

size_t find_top_level_comma_text(const string & text)
{
  int paren_depth = 0;
  int angle_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    if(text[i] == '(') {
      ++paren_depth;
    } else if(text[i] == ')') {
      --paren_depth;
    } else if(text[i] == '<') {
      ++angle_depth;
    } else if(text[i] == '>') {
      --angle_depth;
    } else if(text[i] == ',' && paren_depth == 0 && angle_depth == 0) {
      return i;
    }
  }
  return string::npos;
}

bool is_void_discard_expression_text(string text)
{
  strip_balanced_outer_parens(text);
  return text.compare(0, 6, "(void)") == 0 ||
         text.compare(0, 18, "static_cast<void>(") == 0;
}

bool try_evaluate_integral_text(const string & text, long long & out)
{
  string compact = compact_expression_text(text);
  strip_balanced_outer_parens(compact);
  if(parse_simple_integral_constant_text(compact, out) ||
     try_evaluate_static_cast_integral_text(compact, out)) {
    return true;
  }

  const size_t comma = find_top_level_comma_text(compact);
  if(comma == string::npos) {
    return false;
  }
  const string lhs = compact.substr(0, comma);
  if(!is_void_discard_expression_text(lhs)) {
    return false;
  }
  return try_evaluate_integral_text(compact.substr(comma + 1), out);
}

bool parse_sizeof_pack_count_text(Scope & scope,
                                  const string & text,
                                  long long & out)
{
  string pack_name;
  size_t pack_size = 0;
  if(!parse_sizeof_pack_text(text, pack_name) ||
     !lookup_pack_size(scope, pack_name, pack_size)) {
    return false;
  }
  out = static_cast<long long>(pack_size);
  return true;
}

bool lookup_integral_constant_count_text(Scope & scope,
                                         const string & text,
                                         long long & out)
{
  const string name = trim_space(text);
  if(!is_identifier_text(name)) {
    return false;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    map<string, ValueBinding>::const_iterator found = current->values.find(name);
    if(found == current->values.end()) {
      continue;
    }
    const ValueBinding & binding = found->second;
    if(binding.dependent_template_value) {
      return false;
    }
    if(binding.has_constant_value) {
      out = binding.constant_value;
      return true;
    }
    if(binding.has_constexpr_value &&
       constant_eval::constexpr_value_to_integral(binding.constexpr_value, out)) {
      return true;
    }
    return false;
  }
  return false;
}

bool find_top_level_binary_operator_text(const string & text,
                                         const string & operators,
                                         size_t & out)
{
  int paren_depth = 0;
  int angle_depth = 0;
  for(size_t i = text.size(); i > 0; --i) {
    const size_t pos = i - 1;
    const char ch = text[pos];
    if(ch == ')') {
      ++paren_depth;
      continue;
    }
    if(ch == '(') {
      --paren_depth;
      continue;
    }
    if(ch == '>') {
      ++angle_depth;
      continue;
    }
    if(ch == '<') {
      --angle_depth;
      continue;
    }
    if(paren_depth != 0 || angle_depth != 0 ||
       operators.find(ch) == string::npos) {
      continue;
    }
    if((ch == '+' || ch == '-') &&
       (pos == 0 ||
        string("+-*/%<>=!&|^?:,(").find(text[pos - 1]) != string::npos)) {
      continue;
    }
    out = pos;
    return true;
  }
  return false;
}

bool try_evaluate_integral_text_with_pack_scope(Scope & scope,
                                                const string & text,
                                                long long & out)
{
  string compact = compact_expression_text(text);
  strip_balanced_outer_parens(compact);
  if(parse_simple_integral_constant_text(compact, out) ||
     parse_sizeof_pack_count_text(scope, compact, out) ||
     lookup_integral_constant_count_text(scope, compact, out)) {
    return true;
  }

  size_t op = string::npos;
  if(find_top_level_binary_operator_text(compact, "+-", op)) {
    long long lhs = 0;
    long long rhs = 0;
    if(!try_evaluate_integral_text_with_pack_scope(scope, compact.substr(0, op), lhs) ||
       !try_evaluate_integral_text_with_pack_scope(scope, compact.substr(op + 1), rhs)) {
      return false;
    }
    out = compact[op] == '+' ? lhs + rhs : lhs - rhs;
    return true;
  }
  if(find_top_level_binary_operator_text(compact, "*/%", op)) {
    long long lhs = 0;
    long long rhs = 0;
    if(!try_evaluate_integral_text_with_pack_scope(scope, compact.substr(0, op), lhs) ||
       !try_evaluate_integral_text_with_pack_scope(scope, compact.substr(op + 1), rhs) ||
       rhs == 0) {
      return false;
    }
    if(compact[op] == '*') {
      out = lhs * rhs;
    } else if(compact[op] == '/') {
      out = lhs / rhs;
    } else {
      out = lhs % rhs;
    }
    return true;
  }
  return false;
}

bool strip_leading_typename_text(const string & text, string & stripped)
{
  stripped.clear();
  const string trimmed = trim_space(text);
  const string typename_prefix = "typename ";
  if(trimmed.size() <= typename_prefix.size() ||
     trimmed.compare(0, typename_prefix.size(), typename_prefix) != 0) {
    return false;
  }
  stripped = trim_space(trimmed.substr(typename_prefix.size()));
  if(stripped.empty() ||
     !(std::isalpha(static_cast<unsigned char>(stripped[0])) || stripped[0] == '_')) {
    stripped.clear();
    return false;
  }
  for(size_t i = 1; i < stripped.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(stripped[i]);
    if(!(std::isalnum(ch) || stripped[i] == '_')) {
      stripped.clear();
      return false;
    }
  }
  return true;
}

bool is_simple_identifier_text(const string & text)
{
  if(text.empty() ||
     !(std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_')) {
    return false;
  }
  for(size_t i = 1; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!(std::isalnum(ch) || text[i] == '_')) {
      return false;
    }
  }
  return true;
}

string qualified_name_text(const QualifiedName & qualified)
{
  string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out += qualified.qualifiers[i];
    out += "::";
  }
  out += qualified.name;
  return out;
}

struct DependentTypeExprText
{
  bool is_typeof = false;
  bool operand_was_parenthesized = false;
  size_t inner_offset = 0;
  string inner;
};

bool parse_dependent_type_expr_text(const string & text,
                                    DependentTypeExprText & out)
{
  out = DependentTypeExprText();

  const string decltype_prefix = "decltype(";
  const string decltype_gnu_prefix = "__decltype(";
  const string decltype_gnu_alt_prefix = "__decltype__(";
  const string typeof_prefix = "__typeof(";
  const string typeof_alt_prefix = "__typeof__(";

  size_t prefix_size = 0;
  if(text.size() >= decltype_prefix.size() + 1 &&
     text.compare(0, decltype_prefix.size(), decltype_prefix) == 0 &&
     text[text.size() - 1] == ')') {
    prefix_size = decltype_prefix.size();
  } else if(text.size() >= decltype_gnu_prefix.size() + 1 &&
            text.compare(0, decltype_gnu_prefix.size(), decltype_gnu_prefix) == 0 &&
            text[text.size() - 1] == ')') {
    prefix_size = decltype_gnu_prefix.size();
  } else if(text.size() >= decltype_gnu_alt_prefix.size() + 1 &&
            text.compare(0, decltype_gnu_alt_prefix.size(), decltype_gnu_alt_prefix) == 0 &&
            text[text.size() - 1] == ')') {
    prefix_size = decltype_gnu_alt_prefix.size();
  } else if(text.size() >= typeof_prefix.size() + 1 &&
            text.compare(0, typeof_prefix.size(), typeof_prefix) == 0 &&
            text[text.size() - 1] == ')') {
    prefix_size = typeof_prefix.size();
    out.is_typeof = true;
  } else if(text.size() >= typeof_alt_prefix.size() + 1 &&
            text.compare(0, typeof_alt_prefix.size(), typeof_alt_prefix) == 0 &&
            text[text.size() - 1] == ')') {
    prefix_size = typeof_alt_prefix.size();
    out.is_typeof = true;
  } else {
    return false;
  }

  const string raw_inner = text.substr(prefix_size, text.size() - prefix_size - 1);
  size_t leading = 0;
  while(leading < raw_inner.size() &&
        std::isspace(static_cast<unsigned char>(raw_inner[leading]))) {
    ++leading;
  }
  out.inner_offset = prefix_size + leading;
  out.inner = trim_space(raw_inner);
  if(!out.is_typeof && is_wrapped_in_balanced_parens(out.inner)) {
    out.operand_was_parenthesized = true;
    const string parenthesized = out.inner.substr(1, out.inner.size() - 2);
    size_t parenthesized_leading = 0;
    while(parenthesized_leading < parenthesized.size() &&
          std::isspace(static_cast<unsigned char>(
              parenthesized[parenthesized_leading]))) {
      ++parenthesized_leading;
    }
    out.inner_offset += 1 + parenthesized_leading;
    out.inner = trim_space(parenthesized);
  }
  return !out.inner.empty();
}

vector<string> split_comma_list(const string & text)
{
  vector<string> out;
  string current;
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    switch(ch) {
    case '<':
      ++angle_depth;
      break;
    case '>':
      if(angle_depth > 0) {
        --angle_depth;
      }
      break;
    case '(':
      ++paren_depth;
      break;
    case ')':
      if(paren_depth > 0) {
        --paren_depth;
      }
      break;
    case '[':
      ++bracket_depth;
      break;
    case ']':
      if(bracket_depth > 0) {
        --bracket_depth;
      }
      break;
    case '{':
      ++brace_depth;
      break;
    case '}':
      if(brace_depth > 0) {
        --brace_depth;
      }
      break;
    default:
      break;
    }

    if(ch == ',' && angle_depth == 0 && paren_depth == 0 &&
       bracket_depth == 0 && brace_depth == 0) {
      out.push_back(trim_space(current));
      current.clear();
    } else {
      current += ch;
    }
  }
  if(!current.empty()) {
    out.push_back(trim_space(current));
  }
  return out;
}

bool split_top_level_call_expression_text(const string & text,
                                          string & callee_text,
                                          string & arg_text)
{
  const string trimmed = trim_space(text);
  if(trimmed.size() < 2 || trimmed[trimmed.size() - 1] != ')') {
    return false;
  }

  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  size_t split = string::npos;
  for(size_t i = trimmed.size(); i-- > 0;) {
    const char ch = trimmed[i];
    if(ch == '>') {
      ++angle_depth;
    } else if(ch == '<') {
      if(angle_depth > 0) {
        --angle_depth;
      }
    } else if(ch == ')') {
      ++paren_depth;
    } else if(ch == '(') {
      if(paren_depth == 0) {
        return false;
      }
      --paren_depth;
      if(angle_depth == 0 &&
         paren_depth == 0 &&
         bracket_depth == 0 &&
         brace_depth == 0) {
        split = i;
        break;
      }
    } else if(ch == ']') {
      ++bracket_depth;
    } else if(ch == '[') {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
    } else if(ch == '}') {
      ++brace_depth;
    } else if(ch == '{') {
      if(brace_depth > 0) {
        --brace_depth;
      }
    }
  }

  if(split == string::npos) {
    return false;
  }

  callee_text = trim_space(trimmed.substr(0, split));
  if(callee_text.empty()) {
    return false;
  }
  arg_text = trimmed.substr(split + 1, trimmed.size() - split - 2);
  return true;
}

string join_comma_list(const vector<string> & texts)
{
  ostringstream out;
  for(size_t i = 0; i < texts.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << trim_space(texts[i]);
  }
  return out.str();
}

vector<string> rewrite_decltype_expression_pack_texts_impl(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & text)
{
  const string trimmed = trim_space(text);
  if(trimmed.empty()) {
    return vector<string>();
  }

  if(trimmed.size() > 3 && trimmed.substr(trimmed.size() - 3) == "...") {
    const vector<string> expanded =
        template_argument_semantics::expand_bound_expression_pack_texts(
            services, scope, trimmed);
    if(expanded.size() != 1 || trim_space(expanded[0]) != trimmed) {
      return expanded;
    }
  }

  if(is_wrapped_in_balanced_parens(trimmed)) {
    vector<string> rewritten =
        rewrite_decltype_expression_pack_texts_impl(
            services,
            scope,
            trim_space(trimmed.substr(1, trimmed.size() - 2)));
    for(size_t i = 0; i < rewritten.size(); ++i) {
      rewritten[i] = "(" + trim_space(rewritten[i]) + ")";
    }
    return rewritten;
  }

  if(has_top_level_comma(trimmed)) {
    const vector<string> parts = split_comma_list(trimmed);
    vector<string> rewritten_parts;
    for(size_t i = 0; i < parts.size(); ++i) {
      vector<string> expanded =
          rewrite_decltype_expression_pack_texts_impl(services, scope, parts[i]);
      rewritten_parts.insert(rewritten_parts.end(), expanded.begin(), expanded.end());
    }
    if(rewritten_parts.empty()) {
      return vector<string>();
    }
    return vector<string>(1, join_comma_list(rewritten_parts));
  }

  string callee_text;
  string arg_text;
  if(split_top_level_call_expression_text(trimmed, callee_text, arg_text)) {
    vector<string> rewritten_callees =
        rewrite_decltype_expression_pack_texts_impl(services, scope, callee_text);
    if(rewritten_callees.empty()) {
      return vector<string>();
    }

    vector<string> rewritten_args;
    const string trimmed_args = trim_space(arg_text);
    if(!trimmed_args.empty()) {
      const vector<string> parts = split_comma_list(trimmed_args);
      for(size_t i = 0; i < parts.size(); ++i) {
        vector<string> expanded =
            rewrite_decltype_expression_pack_texts_impl(services, scope, parts[i]);
        rewritten_args.insert(rewritten_args.end(), expanded.begin(), expanded.end());
      }
    }

    vector<string> out;
    const string joined_args = join_comma_list(rewritten_args);
    for(size_t i = 0; i < rewritten_callees.size(); ++i) {
      out.push_back(trim_space(rewritten_callees[i]) + "(" + joined_args + ")");
    }
    return out;
  }

  const vector<ExpandedTypeArgumentInput> expanded =
      expand_bound_type_pack_arguments(services, scope, vector<string>(1, trimmed));
  return expanded_type_argument_input_texts(expanded);
}

string replace_identifier_token_text(const string & text,
                                     const string & name,
                                     const string & replacement,
                                     bool & changed);

CppAstNode clone_expression_node_for_template_substitution(const CppAstNode & source);

TemplateArgumentSyntax clone_argument_syntax_for_template_substitution(
    const TemplateArgumentSyntax & source);

TemplateIdSyntax clone_template_id_for_template_substitution(
    const TemplateIdSyntax & source)
{
  TemplateIdSyntax out;
  out.name = source.name;
  out.source_location_id = source.source_location_id;
  out.arguments = source.arguments;
  out.argument_syntaxes.reserve(source.argument_syntaxes.size());
  for(size_t i = 0; i < source.argument_syntaxes.size(); ++i) {
    out.argument_syntaxes.push_back(
        clone_argument_syntax_for_template_substitution(source.argument_syntaxes[i]));
  }
  return out;
}

TemplateArgumentSyntax clone_argument_syntax_for_template_substitution(
    const TemplateArgumentSyntax & source)
{
  TemplateArgumentSyntax out;
  out = source;
  if(source.template_id) {
    out.template_id.reset(new TemplateIdSyntax(
        clone_template_id_for_template_substitution(*source.template_id)));
  }
  if(source.type_id) {
    out.type_id.reset(new CppAstNode(
        clone_expression_node_for_template_substitution(*source.type_id)));
  }
  if(source.expression) {
    out.expression.reset(new CppAstNode(
        clone_expression_node_for_template_substitution(*source.expression)));
  }
  return out;
}

CppAstNode clone_expression_node_for_template_substitution(const CppAstNode & source)
{
  CppAstNode out = source;
  if(source.qualified_name_syntax) {
    out.qualified_name_syntax.reset(new QualifiedName(*source.qualified_name_syntax));
  }
  if(source.template_id_syntax) {
    out.template_id_syntax.reset(new TemplateIdSyntax(
        clone_template_id_for_template_substitution(*source.template_id_syntax)));
  }
  if(source.conversion_type_id_syntax) {
    out.conversion_type_id_syntax.reset(new CppAstNode(
        clone_expression_node_for_template_substitution(
            *source.conversion_type_id_syntax)));
  }
  out.qualifier_template_id_syntaxes.clear();
  out.qualifier_template_id_syntaxes.reserve(
      source.qualifier_template_id_syntaxes.size());
  for(size_t i = 0; i < source.qualifier_template_id_syntaxes.size(); ++i) {
    out.qualifier_template_id_syntaxes.push_back(
        clone_template_id_for_template_substitution(
            source.qualifier_template_id_syntaxes[i]));
  }
  out.qualifier_type_syntaxes.clear();
  out.qualifier_type_syntaxes.reserve(source.qualifier_type_syntaxes.size());
  for(size_t i = 0; i < source.qualifier_type_syntaxes.size(); ++i) {
    out.qualifier_type_syntaxes.push_back(
        clone_expression_node_for_template_substitution(
            source.qualifier_type_syntaxes[i]));
  }
  out.exception_type_id_syntaxes.clear();
  out.exception_type_id_syntaxes.reserve(source.exception_type_id_syntaxes.size());
  for(size_t i = 0; i < source.exception_type_id_syntaxes.size(); ++i) {
    out.exception_type_id_syntaxes.push_back(
        clone_expression_node_for_template_substitution(
            source.exception_type_id_syntaxes[i]));
  }
  out.alignment_specifier_nodes.clear();
  out.alignment_specifier_nodes.reserve(source.alignment_specifier_nodes.size());
  for(size_t i = 0; i < source.alignment_specifier_nodes.size(); ++i) {
    out.alignment_specifier_nodes.push_back(
        clone_expression_node_for_template_substitution(
            source.alignment_specifier_nodes[i]));
  }
  out.children.clear();
  out.children.reserve(source.children.size());
  for(size_t i = 0; i < source.children.size(); ++i) {
    out.children.push_back(
        clone_expression_node_for_template_substitution(source.children[i]));
  }
  return out;
}

string replace_sizeof_pack_count_text(const string & text,
                                      const map<string, size_t> & pack_size_replacements,
                                      bool & changed)
{
  if(pack_size_replacements.empty() || text.find("sizeof...") == string::npos) {
    return text;
  }

  string out;
  size_t pos = 0;
  while(pos < text.size()) {
    const size_t found = text.find("sizeof...", pos);
    if(found == string::npos) {
      out += text.substr(pos);
      break;
    }

    out += text.substr(pos, found - pos);
    size_t open = found + 9;
    while(open < text.size() &&
          std::isspace(static_cast<unsigned char>(text[open]))) {
      ++open;
    }
    if(open >= text.size() || text[open] != '(') {
      out += text.substr(found, 9);
      pos = found + 9;
      continue;
    }

    size_t close = open + 1;
    int depth = 1;
    for(; close < text.size(); ++close) {
      if(text[close] == '(') {
        ++depth;
      } else if(text[close] == ')') {
        --depth;
        if(depth == 0) {
          break;
        }
      }
    }
    if(depth != 0) {
      out += text.substr(found);
      pos = text.size();
      break;
    }

    const string pack_name = trim_space(text.substr(open + 1, close - open - 1));
    map<string, size_t>::const_iterator replacement =
        pack_size_replacements.find(pack_name);
    if(replacement == pack_size_replacements.end() ||
       !is_identifier_text(pack_name)) {
      out += text.substr(found, close - found + 1);
      pos = close + 1;
      continue;
    }

    out += std::to_string(replacement->second);
    changed = true;
    pos = close + 1;
  }
  return out;
}

CppAstNode make_sizeof_pack_count_literal_node(size_t value)
{
  CppAstNode out;
  out.kind = CppAstKind::literal;
  out.value = std::to_string(value);
  out.semantic_type = make_fundamental(FT_UNSIGNED_LONG_INT);
  return out;
}

bool substitute_sizeof_pack_count_expression_node(
    const CppAstNode & node,
    const map<string, size_t> & pack_size_replacements,
    CppAstNode & out,
    bool & changed)
{
  if(pack_size_replacements.empty()) {
    out = clone_expression_node_for_template_substitution(node);
    return true;
  }

  if(node.kind == CppAstKind::sizeof_pack_expression &&
     node.children.size() == 1 &&
     node.children[0].kind == CppAstKind::identifier) {
    map<string, size_t>::const_iterator replacement =
        pack_size_replacements.find(node.children[0].value);
    if(replacement != pack_size_replacements.end()) {
      out = make_sizeof_pack_count_literal_node(replacement->second);
      changed = true;
      return true;
    }
  }

  out = clone_expression_node_for_template_substitution(node);
  bool value_changed = false;
  out.value = replace_sizeof_pack_count_text(out.value,
                                             pack_size_replacements,
                                             value_changed);
  if(value_changed) {
    changed = true;
  }
  if(out.template_id_syntax) {
    substitute_sizeof_pack_counts_template_id_arguments(*out.template_id_syntax,
                                                        pack_size_replacements);
  }
  for(size_t i = 0; i < out.qualifier_template_id_syntaxes.size(); ++i) {
    substitute_sizeof_pack_counts_template_id_arguments(
        out.qualifier_template_id_syntaxes[i],
        pack_size_replacements);
  }
  for(size_t i = 0; i < out.qualifier_type_syntaxes.size(); ++i) {
    CppAstNode rewritten;
    if(!substitute_sizeof_pack_count_expression_node(
           out.qualifier_type_syntaxes[i],
           pack_size_replacements,
           rewritten,
           changed)) {
      return false;
    }
    out.qualifier_type_syntaxes[i] = rewritten;
  }
  for(size_t i = 0; i < out.exception_type_id_syntaxes.size(); ++i) {
    CppAstNode rewritten;
    if(!substitute_sizeof_pack_count_expression_node(
           out.exception_type_id_syntaxes[i],
           pack_size_replacements,
           rewritten,
           changed)) {
      return false;
    }
    out.exception_type_id_syntaxes[i] = rewritten;
  }
  for(size_t i = 0; i < out.alignment_specifier_nodes.size(); ++i) {
    CppAstNode rewritten;
    if(!substitute_sizeof_pack_count_expression_node(
           out.alignment_specifier_nodes[i],
           pack_size_replacements,
           rewritten,
           changed)) {
      return false;
    }
    out.alignment_specifier_nodes[i] = rewritten;
  }
  for(size_t i = 0; i < out.children.size(); ++i) {
    CppAstNode rewritten;
    if(!substitute_sizeof_pack_count_expression_node(out.children[i],
                                                     pack_size_replacements,
                                                     rewritten,
                                                     changed)) {
      return false;
    }
    out.children[i] = rewritten;
  }
  return true;
}

void substitute_sizeof_pack_counts_template_id_arguments(
    TemplateIdSyntax & syntax,
    const map<string, size_t> & pack_size_replacements)
{
  if(pack_size_replacements.empty()) {
    return;
  }
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    bool changed = false;
    syntax.arguments[i] = replace_sizeof_pack_count_text(syntax.arguments[i],
                                                         pack_size_replacements,
                                                         changed);
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    TemplateArgumentSyntax & argument = syntax.argument_syntaxes[i];
    bool text_changed = false;
    const string rewritten_text =
        replace_sizeof_pack_count_text(argument.text,
                                       pack_size_replacements,
                                       text_changed);
    if(text_changed) {
      if(argument.source_text.empty()) {
        argument.source_text = argument.text;
      }
      argument.text = trim_space(rewritten_text);
    }
    if(argument.type_id) {
      CppAstNode rewritten;
      bool changed = false;
      if(substitute_sizeof_pack_count_expression_node(*argument.type_id,
                                                      pack_size_replacements,
                                                      rewritten,
                                                      changed) &&
         changed) {
        argument.type_id.reset(new CppAstNode(rewritten));
      }
    }
    if(argument.expression) {
      CppAstNode rewritten;
      bool changed = false;
      if(substitute_sizeof_pack_count_expression_node(*argument.expression,
                                                      pack_size_replacements,
                                                      rewritten,
                                                      changed) &&
         changed) {
        argument.expression.reset(new CppAstNode(rewritten));
      }
    }
    if(argument.template_id) {
      substitute_sizeof_pack_counts_template_id_arguments(*argument.template_id,
                                                          pack_size_replacements);
    }
  }
}

bool template_id_syntax_mentions_identifier(const TemplateIdSyntax & syntax,
                                            const string & name);

bool expression_node_mentions_identifier(const CppAstNode & node,
                                         const string & name)
{
  if(!name.empty() &&
     callsemantic_internal::contains_identifier_token(node.value, name)) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_mentions_identifier(*node.template_id_syntax, name)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_identifier(node.qualifier_template_id_syntaxes[i],
                                              name)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(expression_node_mentions_identifier(node.qualifier_type_syntaxes[i], name)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(expression_node_mentions_identifier(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

bool argument_syntax_mentions_identifier(const TemplateArgumentSyntax & syntax,
                                         const string & name)
{
  if(callsemantic_internal::contains_identifier_token(syntax.text, name)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_identifier(*syntax.template_id, name)) {
    return true;
  }
  if(syntax.type_id && expression_node_mentions_identifier(*syntax.type_id, name)) {
    return true;
  }
  if(syntax.expression && expression_node_mentions_identifier(*syntax.expression, name)) {
    return true;
  }
  return false;
}

bool template_id_syntax_mentions_identifier(const TemplateIdSyntax & syntax,
                                            const string & name)
{
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(callsemantic_internal::contains_identifier_token(syntax.arguments[i], name)) {
      return true;
    }
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(argument_syntax_mentions_identifier(syntax.argument_syntaxes[i], name)) {
      return true;
    }
  }
  return false;
}

bool expression_node_mentions_pack_expansion_identifier(const CppAstNode & node,
                                                        const string & name);
bool argument_syntax_mentions_pack_expansion_identifier(const TemplateArgumentSyntax & syntax,
                                                       const string & name);

bool template_id_syntax_mentions_pack_expansion_identifier(const TemplateIdSyntax & syntax,
                                                           const string & name)
{
  if(!syntax.argument_syntaxes.empty()) {
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      if(argument_syntax_mentions_pack_expansion_identifier(syntax.argument_syntaxes[i],
                                                            name)) {
        return true;
      }
    }
    return false;
  }
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    if(callsemantic_internal::contains_identifier_token(syntax.arguments[i], name)) {
      return true;
    }
  }
  return false;
}

bool expression_node_mentions_pack_expansion_identifier(const CppAstNode & node,
                                                        const string & name)
{
  if(node.kind == CppAstKind::sizeof_pack_expression) {
    return false;
  }
  if(!name.empty() &&
     callsemantic_internal::contains_identifier_token(node.value, name)) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_mentions_pack_expansion_identifier(*node.template_id_syntax,
                                                           name)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_pack_expansion_identifier(
           node.qualifier_template_id_syntaxes[i],
           name)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(expression_node_mentions_pack_expansion_identifier(
           node.qualifier_type_syntaxes[i],
           name)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(expression_node_mentions_pack_expansion_identifier(node.children[i], name)) {
      return true;
    }
  }
  return false;
}

bool argument_syntax_mentions_pack_expansion_identifier(const TemplateArgumentSyntax & syntax,
                                                       const string & name)
{
  bool has_structured_syntax = false;
  if(syntax.template_id) {
    has_structured_syntax = true;
    if(template_id_syntax_mentions_pack_expansion_identifier(*syntax.template_id,
                                                             name)) {
      return true;
    }
  }
  if(syntax.type_id) {
    has_structured_syntax = true;
    if(expression_node_mentions_pack_expansion_identifier(*syntax.type_id, name)) {
      return true;
    }
  }
  if(syntax.expression) {
    has_structured_syntax = true;
    if(expression_node_mentions_pack_expansion_identifier(*syntax.expression, name)) {
      return true;
    }
  }
  return !has_structured_syntax &&
         callsemantic_internal::contains_identifier_token(syntax.text, name);
}

CppAstNode make_substituted_type_id_node(const TypePtr & type,
                                         const string & text)
{
  CppAstNode type_id;
  type_id.kind = CppAstKind::type_id;
  type_id.value = text;
  type_id.semantic_type = type;

  CppAstNode specifiers;
  specifiers.kind = CppAstKind::type_specifier_seq;
  specifiers.value = text;
  specifiers.semantic_type = type;

  CppAstNode type_name;
  type_name.kind = CppAstKind::type_name;
  type_name.value = text;
  type_name.semantic_type = type;

  specifiers.children.push_back(type_name);
  type_id.children.push_back(specifiers);
  return type_id;
}

CppAstNode make_substituted_value_expression_node(const ValueBinding & binding)
{
  CppAstNode out;
  const TypePtr base_type = strip_top_level_cv(remove_reference_type(binding.type));
  if(base_type && is_bool_type(base_type)) {
    out.kind = CppAstKind::keyword_literal;
    out.value = binding.constant_value != 0 ? "true" : "false";
    out.has_token = true;
    out.token_kind = RT_SIMPLE;
    out.simple_type = binding.constant_value != 0 ? KW_TRUE : KW_FALSE;
    out.semantic_type = binding.type;
    return out;
  }

  out.kind = CppAstKind::literal;
  out.value = binding.has_constant_value && !binding.dependent_template_value ?
      to_string(binding.constant_value) :
      (!binding.name.empty() ? binding.name : to_string(binding.constant_value));
  out.semantic_type = binding.type;
  return out;
}

bool parse_unary_builtin_type_transform_syntax(const string & text,
                                               string & builtin_name,
                                               string & arg_text)
{
  builtin_name.clear();
  arg_text.clear();

  const string trimmed = trim_space(text);
  const size_t open = trimmed.find('(');
  if(open == string::npos || open == 0 || trimmed.empty() ||
     trimmed[trimmed.size() - 1] != ')') {
    return false;
  }

  int depth = 0;
  for(size_t i = open; i < trimmed.size(); ++i) {
    if(trimmed[i] == '(') {
      ++depth;
    } else if(trimmed[i] == ')') {
      --depth;
      if(depth == 0 && i + 1 != trimmed.size()) {
        return false;
      }
      if(depth < 0) {
        return false;
      }
    }
  }
  if(depth != 0) {
    return false;
  }

  builtin_name = trim_space(trimmed.substr(0, open));
  arg_text = trim_space(trimmed.substr(open + 1, trimmed.size() - open - 2));
  return !builtin_name.empty() && !arg_text.empty();
}

bool apply_context_free_builtin_type_transform(const string & builtin_name,
                                               const TypePtr & arg_type,
                                               TypePtr & out)
{
  out.reset();
  if(!arg_type) {
    return false;
  }

  if(builtin_name == "__remove_cv") {
    out = strip_top_level_cv(arg_type);
    return static_cast<bool>(out);
  }
  if(builtin_name == "__remove_const") {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, false, arg_type->cv_volatile) :
        arg_type;
    return static_cast<bool>(out);
  }
  if(builtin_name == "__remove_volatile") {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, arg_type->cv_const, false) :
        arg_type;
    return static_cast<bool>(out);
  }
  if(builtin_name == "__remove_reference" ||
     builtin_name == "__remove_reference_t") {
    out = remove_reference_type(arg_type);
    return static_cast<bool>(out);
  }
  if(builtin_name == "__remove_cvref") {
    out = strip_top_level_cv(remove_reference_type(arg_type));
    return static_cast<bool>(out);
  }
  if(builtin_name == "__decay") {
    TypePtr decayed = remove_reference_type(arg_type);
    TypePtr decayed_base = strip_top_level_cv(decayed);
    if(!decayed_base) {
      return false;
    }
    if(decayed_base->kind == Type::TK_ARRAY) {
      out = make_pointer(decayed_base->inner);
    } else if(decayed_base->kind == Type::TK_FUNCTION) {
      out = make_pointer(decayed_base);
    } else {
      out = decayed_base;
    }
    return static_cast<bool>(out);
  }
  if(builtin_name == "__add_pointer") {
    TypePtr pointee = remove_reference_type(arg_type);
    if(!pointee) {
      return false;
    }
    out = make_pointer(pointee);
    return true;
  }
  if(builtin_name == "__remove_pointer") {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    out = base->kind == Type::TK_POINTER ? base->inner : arg_type;
    return static_cast<bool>(out);
  }
  if(builtin_name == "__add_lvalue_reference" ||
     builtin_name == "__add_rvalue_reference") {
    TypePtr base = strip_top_level_cv(arg_type);
    if(!base) {
      return false;
    }
    if(is_void_type(base) || base->kind == Type::TK_LVALUE_REFERENCE) {
      out = arg_type;
      return true;
    }
    if(base->kind == Type::TK_RVALUE_REFERENCE) {
      out = builtin_name == "__add_lvalue_reference" ?
          make_lvalue_reference_raw(base->inner) :
          arg_type;
      return true;
    }
    out = builtin_name == "__add_lvalue_reference" ?
        make_lvalue_reference_raw(arg_type) :
        make_rvalue_reference_raw(arg_type);
    return true;
  }
  return false;
}

bool resolve_substituted_builtin_type_transform_syntax(
    const string & text,
    const map<string, TypePtr> & type_replacements,
    TypePtr & out)
{
  out.reset();
  string builtin_name;
  string arg_text;
  if(!parse_unary_builtin_type_transform_syntax(text, builtin_name, arg_text)) {
    return false;
  }

  TypePtr arg_type;
  map<string, TypePtr>::const_iterator direct = type_replacements.find(arg_text);
  if(direct != type_replacements.end()) {
    arg_type = direct->second;
  } else if(!resolve_substituted_builtin_type_transform_syntax(arg_text,
                                                               type_replacements,
                                                               arg_type)) {
    return false;
  }

  return apply_context_free_builtin_type_transform(builtin_name, arg_type, out);
}

bool substitute_qualified_name_qualifier_type(
    CppAstNode & node,
    size_t qualifier_index,
    const TypePtr & type,
    const string & replacement_text)
{
  if(!node.qualified_name_syntax) {
    return false;
  }
  QualifiedName qualified = *node.qualified_name_syntax;
  if(qualifier_index >= qualified.qualifiers.size()) {
    return false;
  }

  string replacement_component = trim_space(replacement_text);
  bool replacement_rooted = false;
  if(replacement_component.compare(0, 2, "::") == 0) {
    replacement_rooted = true;
    replacement_component = trim_space(replacement_component.substr(2));
  }
  if(replacement_component.empty()) {
    return false;
  }

  vector<string> new_qualifiers;
  new_qualifiers.reserve(qualified.qualifiers.size());
  new_qualifiers.insert(new_qualifiers.end(),
                        qualified.qualifiers.begin(),
                        qualified.qualifiers.begin() + qualifier_index);
  const size_t replacement_component_index = new_qualifiers.size();
  new_qualifiers.push_back(replacement_component);
  new_qualifiers.insert(new_qualifiers.end(),
                        qualified.qualifiers.begin() + qualifier_index + 1,
                        qualified.qualifiers.end());

  vector<TemplateIdSyntax> new_qualifier_template_ids(new_qualifiers.size());
  for(size_t old_i = 0; old_i < qualifier_index &&
         old_i < node.qualifier_template_id_syntaxes.size(); ++old_i) {
    new_qualifier_template_ids[old_i] = node.qualifier_template_id_syntaxes[old_i];
  }
  for(size_t old_i = qualifier_index + 1;
      old_i < qualified.qualifiers.size() &&
          old_i < node.qualifier_template_id_syntaxes.size();
      ++old_i) {
    new_qualifier_template_ids[old_i] = node.qualifier_template_id_syntaxes[old_i];
  }

  vector<CppAstNode> new_qualifier_types(new_qualifiers.size());
  for(size_t old_i = 0; old_i < qualifier_index &&
         old_i < node.qualifier_type_syntaxes.size(); ++old_i) {
    new_qualifier_types[old_i] = node.qualifier_type_syntaxes[old_i];
  }
  new_qualifier_types[replacement_component_index] =
      make_substituted_type_id_node(type, replacement_text);
  for(size_t old_i = qualifier_index + 1;
      old_i < qualified.qualifiers.size() &&
          old_i < node.qualifier_type_syntaxes.size();
      ++old_i) {
    new_qualifier_types[old_i] = node.qualifier_type_syntaxes[old_i];
  }

  qualified.rooted = qualified.rooted ||
      (qualifier_index == 0 && replacement_rooted);
  qualified.qualifiers = new_qualifiers;
  node.qualified_name_syntax.reset(new QualifiedName(qualified));
  node.qualifier_template_id_syntaxes = new_qualifier_template_ids;
  node.qualifier_type_syntaxes = new_qualifier_types;
  node.value = qualified_name_text(qualified);
  return true;
}

void substitute_type_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    Scope & scope,
    const map<string, TypePtr> & type_replacements);

void refresh_qualified_name_qualifier_template_id_texts(CppAstNode & node)
{
  if(!node.qualified_name_syntax) {
    return;
  }
  QualifiedName qualified = *node.qualified_name_syntax;
  bool changed = false;
  const size_t count =
      std::min(qualified.qualifiers.size(),
               node.qualifier_template_id_syntaxes.size());
  for(size_t i = 0; i < count; ++i) {
    const TemplateIdSyntax & syntax = node.qualifier_template_id_syntaxes[i];
    if(syntax.name.name.empty()) {
      continue;
    }
    const string text = template_id_syntax_lookup_text(syntax);
    if(!text.empty() && text != qualified.qualifiers[i]) {
      qualified.qualifiers[i] = text;
      changed = true;
    }
  }
  if(!changed) {
    return;
  }
  node.qualified_name_syntax.reset(new QualifiedName(qualified));
  node.value = qualified_name_text(qualified);
}

bool substitute_type_pack_expression_node(
    Scope & scope,
    const CppAstNode & node,
    const map<string, TypePtr> & type_replacements,
    CppAstNode & out)
{
  out = clone_expression_node_for_template_substitution(node);
  if(out.kind == CppAstKind::sizeof_pack_expression) {
    return true;
  }
  const bool has_structured_template_id =
      out.template_id_syntax != nullptr ||
      !out.qualifier_template_id_syntaxes.empty();
  if(out.template_id_syntax) {
    substitute_type_pack_template_id_arguments(*out.template_id_syntax,
                                               scope,
                                               type_replacements);
    out.value = qualified_name_text(out.template_id_syntax->name) + "<";
    for(size_t i = 0; i < out.template_id_syntax->arguments.size(); ++i) {
      if(i != 0) {
        out.value += ",";
      }
      out.value += out.template_id_syntax->arguments[i];
    }
    out.value += ">";
  }
  for(size_t i = 0; i < out.qualifier_template_id_syntaxes.size(); ++i) {
    substitute_type_pack_template_id_arguments(out.qualifier_template_id_syntaxes[i],
                                               scope,
                                               type_replacements);
    if(out.qualified_name_syntax &&
       i < out.qualified_name_syntax->qualifiers.size() &&
       !out.qualifier_template_id_syntaxes[i].name.name.empty()) {
      out.qualified_name_syntax->qualifiers[i] =
          template_id_syntax_lookup_text(out.qualifier_template_id_syntaxes[i]);
      out.value = qualified_name_text(*out.qualified_name_syntax);
    }
  }
  refresh_qualified_name_qualifier_template_id_texts(out);
  TypePtr transformed_type;
  if(resolve_substituted_builtin_type_transform_syntax(out.value,
                                                       type_replacements,
                                                       transformed_type)) {
    out.semantic_type = transformed_type;
  }
  for(map<string, TypePtr>::const_iterator it = type_replacements.begin();
      it != type_replacements.end();
      ++it) {
    bool replaced_qualified_component = false;
    if(out.qualified_name_syntax) {
      const std::vector<std::string> qualifiers =
          out.qualified_name_syntax->qualifiers;
      for(size_t i = 0; i < qualifiers.size(); ++i) {
        if(callsemantic_internal::is_identifier_text(qualifiers[i]) &&
           qualifiers[i] == it->first) {
          const string replacement_text = reparseable_type_argument_text(it->second);
          if(!substitute_qualified_name_qualifier_type(out,
                                                       i,
                                                       it->second,
                                                       replacement_text)) {
            if(out.qualifier_type_syntaxes.size() < qualifiers.size()) {
              out.qualifier_type_syntaxes.mutable_vector().resize(qualifiers.size());
            }
            out.qualifier_type_syntaxes[i] =
                make_substituted_type_id_node(it->second, replacement_text);
          }
          replaced_qualified_component = true;
          break;
        }
      }
    }
    if(replaced_qualified_component) {
      continue;
    }
    if(callsemantic_internal::is_identifier_text(out.value) &&
       out.value == it->first) {
      out.value = reparseable_type_argument_text(it->second);
      out.semantic_type = it->second;
      out.qualified_name_syntax.reset();
      out.template_id_syntax.reset();
      out.qualifier_template_id_syntaxes.clear();
      out.qualifier_type_syntaxes.clear();
      break;
    }
    if(has_structured_template_id) {
      continue;
    }
    bool value_changed = false;
    const string substituted_value =
        replace_identifier_token_text(out.value,
                                      it->first,
                                      reparseable_type_argument_text(it->second),
                                      value_changed);
    if(value_changed) {
      out.value = substituted_value;
    }
  }
  if(resolve_substituted_builtin_type_transform_syntax(out.value,
                                                       type_replacements,
                                                       transformed_type)) {
    out.semantic_type = transformed_type;
  }

  vector<CppAstNode> children;
  children.reserve(node.children.size());
  for(size_t i = 0; i < node.children.size(); ++i) {
    CppAstNode child;
    if(!substitute_type_pack_expression_node(scope,
                                             node.children[i],
                                             type_replacements,
                                             child)) {
      return false;
    }
    children.push_back(child);
  }
  out.children.swap(children);
  return true;
}

bool erase_parameter_pack_marker_nodes(CppAstNode & current)
{
  bool removed = false;
  vector<CppAstNode> kept;
  kept.reserve(current.children.size());
  for(size_t i = 0; i < current.children.size(); ++i) {
    if(current.children[i].kind == CppAstKind::parameter_pack) {
      removed = true;
      continue;
    }
    removed = erase_parameter_pack_marker_nodes(current.children[i]) || removed;
    kept.push_back(current.children[i]);
  }
  current.children.swap(kept);
  return removed;
}

string substituted_value_pack_argument_text(const ValueBinding & binding)
{
  if(binding.has_constant_value && !binding.dependent_template_value) {
    const TypePtr base_type =
        strip_top_level_cv(remove_reference_type(binding.type));
    if(base_type && is_bool_type(base_type)) {
      return binding.constant_value != 0 ? "true" : "false";
    }
    return to_string(binding.constant_value);
  }
  return binding.name;
}

void substitute_value_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    const map<string, ValueBinding> & value_replacements);

bool substitute_value_pack_expression_node(
    const CppAstNode & node,
    const map<string, ValueBinding> & value_replacements,
    CppAstNode & out)
{
  out = clone_expression_node_for_template_substitution(node);
  if(out.kind == CppAstKind::sizeof_pack_expression) {
    return true;
  }
  if(out.template_id_syntax) {
    substitute_value_pack_template_id_arguments(*out.template_id_syntax,
                                                value_replacements);
    out.value = template_id_syntax_lookup_text(*out.template_id_syntax);
  }
  for(size_t i = 0; i < out.qualifier_template_id_syntaxes.size(); ++i) {
    substitute_value_pack_template_id_arguments(
        out.qualifier_template_id_syntaxes[i],
        value_replacements);
  }
  refresh_qualified_name_qualifier_template_id_texts(out);
  for(map<string, ValueBinding>::const_iterator it = value_replacements.begin();
      it != value_replacements.end();
      ++it) {
    if(callsemantic_internal::is_identifier_text(out.value) &&
       out.value == it->first) {
      if(it->second.has_constant_value) {
        out = make_substituted_value_expression_node(it->second);
      } else {
        out.value = it->second.name;
        out.semantic_type = it->second.type;
        out.qualified_name_syntax.reset();
        out.template_id_syntax.reset();
        out.qualifier_template_id_syntaxes.clear();
        out.qualifier_type_syntaxes.clear();
      }
      break;
    }
  }

  vector<CppAstNode> children;
  children.reserve(node.children.size());
  for(size_t i = 0; i < node.children.size(); ++i) {
    CppAstNode child;
    if(!substitute_value_pack_expression_node(node.children[i],
                                              value_replacements,
                                              child)) {
      return false;
    }
    children.push_back(child);
  }
  out.children.swap(children);
  return true;
}

void substitute_value_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    const map<string, ValueBinding> & value_replacements)
{
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    string rewritten = syntax.arguments[i];
    bool changed = false;
    for(map<string, ValueBinding>::const_iterator it = value_replacements.begin();
        it != value_replacements.end();
        ++it) {
      rewritten = replace_identifier_token_text(
          rewritten,
          it->first,
          substituted_value_pack_argument_text(it->second),
          changed);
    }
    syntax.arguments[i] = rewritten;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    TemplateArgumentSyntax & argument = syntax.argument_syntaxes[i];
    bool pack_expansion_consumed = false;
    for(map<string, ValueBinding>::const_iterator it = value_replacements.begin();
        it != value_replacements.end();
        ++it) {
      if(!argument_syntax_mentions_identifier(argument, it->first)) {
        continue;
      }
      if(argument.pack_expansion) {
        pack_expansion_consumed = true;
      }
      bool argument_changed = false;
      argument.text = replace_identifier_token_text(
          argument.text,
          it->first,
          substituted_value_pack_argument_text(it->second),
          argument_changed);
      if(argument.type_id &&
         expression_node_mentions_identifier(*argument.type_id, it->first)) {
        CppAstNode rewritten_type;
        map<string, ValueBinding> single;
        single[it->first] = it->second;
        if(substitute_value_pack_expression_node(*argument.type_id,
                                                 single,
                                                 rewritten_type)) {
          argument.type_id.reset(new CppAstNode(rewritten_type));
        }
      }
      if(argument.expression &&
         expression_node_mentions_identifier(*argument.expression, it->first)) {
        CppAstNode rewritten_expr;
        map<string, ValueBinding> single;
        single[it->first] = it->second;
        if(substitute_value_pack_expression_node(*argument.expression,
                                                 single,
                                                 rewritten_expr)) {
          argument.expression.reset(new CppAstNode(rewritten_expr));
        }
      }
    }
    if(argument.template_id) {
      substitute_value_pack_template_id_arguments(*argument.template_id,
                                                  value_replacements);
    }
    if(pack_expansion_consumed) {
      string text = trim_space(argument.text);
      if(text.size() >= 3 && text.substr(text.size() - 3) == "...") {
        text = trim_space(text.substr(0, text.size() - 3));
      }
      argument.text = text;
      argument.pack_expansion = false;
    }
  }
}

bool expand_bound_packs_in_argument_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    TemplateArgumentSyntax & syntax);

bool expand_bound_packs_in_expression_node(
    template_api::TemplateServices & services,
    Scope & scope,
    CppAstNode & node);

bool expand_pack_expressions_in_decltype_operand(Scope & scope,
                                                 const CppAstNode & node,
                                                 CppAstNode & out,
                                                 bool & changed);

bool expand_bound_packs_in_template_id_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    TemplateIdSyntax & syntax)
{
  bool changed = false;
  const vector<string> source_texts = template_id_syntax_argument_texts(syntax);
  const ExpandedTemplateArgumentInputs expanded =
      expand_template_argument_inputs(
          services,
          scope,
          source_texts,
          syntax.argument_syntaxes.empty() ? nullptr : &syntax.argument_syntaxes);

  bool argument_list_changed = expanded.texts.size() != source_texts.size();
  if(!argument_list_changed) {
    for(size_t i = 0; i < expanded.texts.size(); ++i) {
      if(trim_space(expanded.texts[i]) != trim_space(source_texts[i])) {
        argument_list_changed = true;
        break;
      }
    }
  }

  if(argument_list_changed) {
    vector<TemplateArgumentSyntax> expanded_syntaxes;
    expanded_syntaxes.reserve(expanded.texts.size());
    for(size_t i = 0; i < expanded.texts.size(); ++i) {
      TemplateArgumentSyntax argument;
      if(const TemplateArgumentSyntax * expanded_syntax = expanded.syntax_for(i)) {
        argument = clone_argument_syntax_for_template_substitution(*expanded_syntax);
      }
      argument.text = trim_space(
          argument.text.empty() ? expanded.texts[i] : argument.text);
      argument.pack_expansion =
          argument.pack_expansion &&
          argument.text.size() >= 3 &&
          argument.text.substr(argument.text.size() - 3) == "...";
      const TypePtr expanded_type = expanded.type_for(i);
      if(expanded_type) {
        if(!argument.resolved_type) {
          argument.resolved_type = expanded_type;
        }
        if(!argument.type_id &&
           !argument.template_id &&
           !argument.expression) {
          argument.type_id.reset(new CppAstNode(
              make_substituted_type_id_node(expanded_type, argument.text)));
        }
      }
      expand_bound_packs_in_argument_syntax(services, scope, argument);
      expanded_syntaxes.push_back(argument);
    }
    syntax.arguments = expanded.texts;
    syntax.argument_syntaxes.swap(expanded_syntaxes);
    changed = true;
  } else {
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      if(expand_bound_packs_in_argument_syntax(
             services, scope, syntax.argument_syntaxes[i])) {
        changed = true;
      }
    }
  }

  if(changed) {
    syntax.arguments = template_id_syntax_argument_texts(syntax);
  }
  return changed;
}

bool expand_bound_packs_in_argument_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    TemplateArgumentSyntax & syntax)
{
  bool changed = false;
  if(syntax.template_id &&
     expand_bound_packs_in_template_id_syntax(services, scope, *syntax.template_id)) {
    if(syntax.source_text.empty()) {
      syntax.source_text = syntax.text;
    }
    syntax.text = template_id_syntax_lookup_text(*syntax.template_id);
    changed = true;
  }
  if(syntax.type_id &&
     expand_bound_packs_in_expression_node(services, scope, *syntax.type_id)) {
    changed = true;
  }
  if(syntax.expression &&
     expand_bound_packs_in_expression_node(services, scope, *syntax.expression)) {
    changed = true;
  }
  if(changed &&
     syntax.pack_expansion &&
     (syntax.text.size() < 3 || syntax.text.substr(syntax.text.size() - 3) != "...")) {
    syntax.pack_expansion = false;
  }
  return changed;
}

bool expand_bound_packs_in_expression_node(
    template_api::TemplateServices & services,
    Scope & scope,
    CppAstNode & node)
{
  bool changed = false;
  if(node.template_id_syntax &&
     expand_bound_packs_in_template_id_syntax(
         services, scope, *node.template_id_syntax)) {
    node.value = template_id_syntax_lookup_text(*node.template_id_syntax);
    changed = true;
  }
  if(node.conversion_type_id_syntax &&
     expand_bound_packs_in_expression_node(
         services, scope, *node.conversion_type_id_syntax)) {
    changed = true;
  }
  if(node.base_type_syntax &&
     expand_bound_packs_in_expression_node(
         services, scope, *node.base_type_syntax)) {
    changed = true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(expand_bound_packs_in_template_id_syntax(
           services,
           scope,
           node.qualifier_template_id_syntaxes[i])) {
      changed = true;
    }
  }
  if(changed) {
    refresh_qualified_name_qualifier_template_id_texts(node);
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(expand_bound_packs_in_expression_node(
           services, scope, node.qualifier_type_syntaxes[i])) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(expand_bound_packs_in_expression_node(
           services, scope, node.exception_type_id_syntaxes[i])) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    if(expand_bound_packs_in_expression_node(
           services, scope, node.alignment_specifier_nodes[i])) {
      changed = true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(expand_bound_packs_in_expression_node(services, scope, node.children[i])) {
      changed = true;
    }
  }
  CppAstNode expanded;
  bool expanded_changed = false;
  if(expand_pack_expressions_in_decltype_operand(scope, node, expanded, expanded_changed) &&
     expanded_changed) {
    node = expanded;
    changed = true;
  }
  return changed;
}

void substitute_type_pack_template_id_arguments(
    TemplateIdSyntax & syntax,
    Scope & scope,
    const map<string, TypePtr> & type_replacements)
{
  if(syntax.argument_syntaxes.empty() && !syntax.arguments.empty()) {
    bool needs_carried_syntax = false;
    for(size_t i = 0; i < syntax.arguments.size() && !needs_carried_syntax; ++i) {
      const string original = trim_space(syntax.arguments[i]);
      for(map<string, TypePtr>::const_iterator it = type_replacements.begin();
          it != type_replacements.end();
          ++it) {
        if(original == it->first || original == it->first + "...") {
          needs_carried_syntax = true;
          break;
        }
      }
    }
    if(needs_carried_syntax) {
      syntax.argument_syntaxes.reserve(syntax.arguments.size());
      for(size_t i = 0; i < syntax.arguments.size(); ++i) {
        TemplateArgumentSyntax argument;
        argument.text = trim_space(syntax.arguments[i]);
        if(argument.text.size() >= 3 &&
           argument.text.substr(argument.text.size() - 3) == "...") {
          argument.pack_expansion = true;
        }
        syntax.argument_syntaxes.push_back(argument);
      }
    }
  }

  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    string rewritten = syntax.arguments[i];
    const string original = trim_space(rewritten);
    bool changed = false;
    for(map<string, TypePtr>::const_iterator it = type_replacements.begin();
        it != type_replacements.end();
        ++it) {
      rewritten = replace_identifier_token_text_preserving_sizeof_pack_operands(
          rewritten,
          it->first,
          reparseable_type_argument_text(it->second),
          changed);
      if(original == it->first || original == it->first + "...") {
        rewritten = reparseable_type_argument_text(it->second);
      }
    }
    syntax.arguments[i] = rewritten;
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    TemplateArgumentSyntax & argument = syntax.argument_syntaxes[i];
    bool argument_changed = false;
    bool pack_expansion_consumed = false;
    for(map<string, TypePtr>::const_iterator it = type_replacements.begin();
        it != type_replacements.end();
        ++it) {
      if(!argument_syntax_mentions_identifier(argument, it->first)) {
        continue;
      }
      if(argument.pack_expansion) {
        pack_expansion_consumed = true;
      }
      const string original_argument_text = trim_space(argument.text);
      const bool direct_expression_type_argument =
          argument.expression &&
          argument.expression->kind == CppAstKind::id_expression &&
          trim_space(argument.expression->value) == it->first;
      const bool direct_type_pack_argument =
          !argument.template_id &&
          (!argument.expression || direct_expression_type_argument) &&
          (original_argument_text == it->first ||
           original_argument_text == it->first + "...");
      const string replacement = reparseable_type_argument_text(it->second);
      argument.text = replace_identifier_token_text_preserving_sizeof_pack_operands(
          argument.text, it->first, replacement, argument_changed);
      if(direct_type_pack_argument) {
        argument.text = replacement;
        if(argument.source_text.empty()) {
          argument.source_text = original_argument_text;
        }
        argument.pack_expansion = false;
        argument.resolved_type = it->second;
        argument.expression.reset();
        argument.type_id.reset(new CppAstNode(
            make_substituted_type_id_node(it->second, replacement)));
      }
      if(argument.type_id &&
         expression_node_mentions_identifier(*argument.type_id, it->first)) {
        CppAstNode rewritten_type;
        map<string, TypePtr> single;
        single[it->first] = it->second;
        if(substitute_type_pack_expression_node(scope,
                                                *argument.type_id,
                                                single,
                                                rewritten_type)) {
          argument.type_id.reset(new CppAstNode(rewritten_type));
        } else {
          argument.type_id.reset(new CppAstNode(
              make_substituted_type_id_node(it->second, replacement)));
        }
      }
      if(argument.expression &&
         expression_node_mentions_identifier(*argument.expression, it->first)) {
        CppAstNode rewritten_expr;
        map<string, TypePtr> single;
        single[it->first] = it->second;
        if(substitute_type_pack_expression_node(scope,
                                                *argument.expression,
                                                single,
                                                rewritten_expr)) {
          argument.expression.reset(new CppAstNode(rewritten_expr));
        }
      }
    }
    if(argument.template_id) {
      substitute_type_pack_template_id_arguments(*argument.template_id,
                                                 scope,
                                                 type_replacements);
      argument.text = template_id_syntax_lookup_text(*argument.template_id);
    }
    if(pack_expansion_consumed) {
      string text = trim_space(argument.text);
      if(text.size() >= 3 && text.substr(text.size() - 3) == "...") {
        text = trim_space(text.substr(0, text.size() - 3));
      }
      argument.text = text;
      argument.pack_expansion = false;
    }
  }
  if(!syntax.argument_syntaxes.empty()) {
    syntax.arguments = template_id_syntax_argument_texts(syntax);
  }
}

bool collect_type_pack_references_in_node(
    Scope & scope,
    const CppAstNode & node,
    vector<pair<string, const vector<TypePtr> *> > & packs)
{
  if(node.kind == CppAstKind::sizeof_pack_expression) {
    return true;
  }
  set<string> seen;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(map<string, vector<TypePtr> >::const_iterator it =
            current->named_type_packs.begin();
        it != current->named_type_packs.end();
        ++it) {
      if(it->first.empty() ||
         seen.count(it->first) != 0 ||
         !expression_node_mentions_pack_expansion_identifier(node, it->first)) {
        continue;
      }
      seen.insert(it->first);
      packs.push_back(make_pair(it->first, &it->second));
    }
  }
  return true;
}

bool collect_type_pack_references_in_argument_syntax(
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    vector<pair<string, const vector<TypePtr> *> > & packs)
{
  set<string> seen;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(map<string, vector<TypePtr> >::const_iterator it =
            current->named_type_packs.begin();
        it != current->named_type_packs.end();
        ++it) {
      if(it->first.empty() ||
         seen.count(it->first) != 0 ||
         !argument_syntax_mentions_pack_expansion_identifier(syntax, it->first)) {
        continue;
      }
      seen.insert(it->first);
      packs.push_back(make_pair(it->first, &it->second));
    }
  }
  return true;
}

bool collect_value_pack_references_in_node(
    Scope & scope,
    const CppAstNode & node,
    vector<pair<string, const vector<ValueBinding> *> > & packs)
{
  if(node.kind == CppAstKind::sizeof_pack_expression) {
    return true;
  }
  set<string> seen;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(map<string, vector<ValueBinding> >::const_iterator it =
            current->named_value_packs.begin();
        it != current->named_value_packs.end();
        ++it) {
      if(it->first.empty() ||
         seen.count(it->first) != 0 ||
         !expression_node_mentions_pack_expansion_identifier(node, it->first)) {
        continue;
      }
      seen.insert(it->first);
      packs.push_back(make_pair(it->first, &it->second));
    }
  }
  return true;
}

bool collect_value_pack_references_in_argument_syntax(
    Scope & scope,
    const TemplateArgumentSyntax & syntax,
    vector<pair<string, const vector<ValueBinding> *> > & packs)
{
  set<string> seen;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(map<string, vector<ValueBinding> >::const_iterator it =
            current->named_value_packs.begin();
        it != current->named_value_packs.end();
        ++it) {
      if(it->first.empty() ||
         seen.count(it->first) != 0 ||
         !argument_syntax_mentions_pack_expansion_identifier(syntax, it->first)) {
        continue;
      }
      seen.insert(it->first);
      packs.push_back(make_pair(it->first, &it->second));
    }
  }
  return true;
}

bool scope_chain_has_template_bound_type_name(Scope * scope,
                                              const string & name)
{
  for(Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->template_bound_type_names.count(name) != 0 ||
       current->template_bound_type_pack_names.count(name) != 0) {
      return true;
    }
    map<string, TypePtr>::const_iterator found = current->named_types.find(name);
    if(found != current->named_types.end() &&
       found->second &&
       named_type_is_template_parameter(found->second)) {
      return true;
    }
  }
  return false;
}

bool scope_chain_has_template_bound_value_name(Scope * scope,
                                               const string & name)
{
  for(Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->template_bound_value_names.count(name) != 0) {
      return true;
    }
  }
  return false;
}

void collect_bound_type_replacements_in_node(Scope & scope,
                                             const CppAstNode & node,
                                             map<string, TypePtr> & replacements)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(set<string>::const_iterator name = current->template_bound_type_names.begin();
        name != current->template_bound_type_names.end();
        ++name) {
      if(name->empty() ||
         replacements.count(*name) != 0 ||
         !expression_node_mentions_identifier(node, *name)) {
        continue;
      }
      map<string, TypePtr>::const_iterator found = current->named_types.find(*name);
      if(found != current->named_types.end() && found->second) {
        replacements[*name] = found->second;
      }
    }
    for(map<string, TypePtr>::const_iterator found = current->named_types.begin();
        found != current->named_types.end();
        ++found) {
      if(found->first.empty() ||
         replacements.count(found->first) != 0 ||
         !found->second ||
         !scope_chain_has_template_bound_type_name(current, found->first) ||
         !expression_node_mentions_identifier(node, found->first)) {
        continue;
      }
      replacements[found->first] = found->second;
    }
  }
}

void collect_bound_value_replacements_in_node(Scope & scope,
                                              const CppAstNode & node,
                                              map<string, ValueBinding> & replacements)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(set<string>::const_iterator name = current->template_bound_value_names.begin();
        name != current->template_bound_value_names.end();
        ++name) {
      if(name->empty() ||
         replacements.count(*name) != 0 ||
         !expression_node_mentions_identifier(node, *name)) {
        continue;
      }
      map<string, ValueBinding>::const_iterator found = current->values.find(*name);
      if(found != current->values.end()) {
        replacements[*name] = found->second;
      }
    }
    for(map<string, ValueBinding>::const_iterator found = current->values.begin();
        found != current->values.end();
        ++found) {
      if(found->first.empty() ||
         replacements.count(found->first) != 0 ||
         !scope_chain_has_template_bound_value_name(current, found->first) ||
         !expression_node_mentions_identifier(node, found->first)) {
        continue;
      }
      replacements[found->first] = found->second;
    }
  }
}

bool substitute_bound_replacements_in_argument_syntax(Scope & scope,
                                                      TemplateArgumentSyntax & syntax)
{
  map<string, TypePtr> type_replacements;
  map<string, ValueBinding> value_replacements;
  if(syntax.type_id) {
    collect_bound_type_replacements_in_node(scope,
                                            *syntax.type_id,
                                            type_replacements);
    collect_bound_value_replacements_in_node(scope,
                                             *syntax.type_id,
                                             value_replacements);
  }
  if(syntax.expression) {
    collect_bound_type_replacements_in_node(scope,
                                            *syntax.expression,
                                            type_replacements);
    collect_bound_value_replacements_in_node(scope,
                                             *syntax.expression,
                                             value_replacements);
  }
  if(type_replacements.empty() && value_replacements.empty()) {
    return false;
  }

  bool changed = false;
  if(syntax.template_id && !type_replacements.empty()) {
    substitute_type_pack_template_id_arguments(*syntax.template_id,
                                               scope,
                                               type_replacements);
    changed = true;
  }
  if(syntax.template_id && !value_replacements.empty()) {
    substitute_value_pack_template_id_arguments(*syntax.template_id,
                                                value_replacements);
    changed = true;
  }
  if(syntax.type_id && !type_replacements.empty()) {
    CppAstNode substituted;
    if(substitute_type_pack_expression_node(scope,
                                            *syntax.type_id,
                                            type_replacements,
                                            substituted)) {
      erase_parameter_pack_marker_nodes(substituted);
      syntax.type_id.reset(new CppAstNode(substituted));
      changed = true;
    }
  }
  if(syntax.expression && !type_replacements.empty()) {
    CppAstNode substituted;
    if(substitute_type_pack_expression_node(scope,
                                            *syntax.expression,
                                            type_replacements,
                                            substituted)) {
      syntax.expression.reset(new CppAstNode(substituted));
      changed = true;
    }
  }
  if(syntax.type_id && !value_replacements.empty()) {
    CppAstNode substituted;
    if(substitute_value_pack_expression_node(*syntax.type_id,
                                             value_replacements,
                                             substituted)) {
      erase_parameter_pack_marker_nodes(substituted);
      syntax.type_id.reset(new CppAstNode(substituted));
      changed = true;
    }
  }
  if(syntax.expression && !value_replacements.empty()) {
    CppAstNode substituted;
    if(substitute_value_pack_expression_node(*syntax.expression,
                                             value_replacements,
                                             substituted)) {
      syntax.expression.reset(new CppAstNode(substituted));
      changed = true;
    }
  }
  if(changed && syntax.template_id) {
    syntax.text = template_id_syntax_lookup_text(*syntax.template_id);
  }
  return changed;
}

bool expand_pack_expressions_in_decltype_operand(Scope & scope,
                                                 const CppAstNode & node,
                                                 CppAstNode & out,
                                                 bool & changed)
{
  if(node.kind == CppAstKind::pack_expansion_expression &&
     node.children.size() == 1) {
    return false;
  }

  out = clone_expression_node_for_template_substitution(node);
  vector<CppAstNode> children;
  children.reserve(node.children.size());
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CppAstNode & child = node.children[i];
    if(child.kind == CppAstKind::pack_expansion_expression &&
       child.children.size() == 1) {
      vector<pair<string, const vector<TypePtr> *> > packs;
      vector<pair<string, const vector<ValueBinding> *> > value_packs;
      collect_type_pack_references_in_node(scope, child.children[0], packs);
      collect_value_pack_references_in_node(scope, child.children[0], value_packs);
      if(packs.empty() && value_packs.empty()) {
        CppAstNode unwrapped;
        if(!expand_pack_expressions_in_decltype_operand(scope,
                                                        child.children[0],
                                                        unwrapped,
                                                        changed)) {
          return false;
        }
        children.push_back(unwrapped);
        changed = true;
        continue;
      }
      const size_t pack_size =
          !packs.empty() ? packs[0].second->size() : value_packs[0].second->size();
      for(size_t j = 1; j < packs.size(); ++j) {
        if(packs[j].second->size() != pack_size) {
          return false;
        }
      }
      for(size_t j = 0; j < value_packs.size(); ++j) {
        if(value_packs[j].second->size() != pack_size) {
          return false;
        }
      }
      for(size_t pack_index = 0; pack_index < pack_size; ++pack_index) {
        map<string, TypePtr> replacements;
        for(size_t j = 0; j < packs.size(); ++j) {
          replacements[packs[j].first] = (*(packs[j].second))[pack_index];
        }
        map<string, ValueBinding> value_replacements;
        for(size_t j = 0; j < value_packs.size(); ++j) {
          value_replacements[value_packs[j].first] =
              (*(value_packs[j].second))[pack_index];
        }
        collect_bound_type_replacements_in_node(scope,
                                                child.children[0],
                                                replacements);
        collect_bound_value_replacements_in_node(scope,
                                                 child.children[0],
                                                 value_replacements);
        CppAstNode substituted;
        if(!substitute_type_pack_expression_node(scope,
                                                 child.children[0],
                                                 replacements,
                                                 substituted)) {
          return false;
        }
        if(!value_replacements.empty() &&
           !substitute_value_pack_expression_node(substituted,
                                                  value_replacements,
                                                  substituted)) {
          return false;
        }
        CppAstNode expanded;
        if(!expand_pack_expressions_in_decltype_operand(scope,
                                                        substituted,
                                                        expanded,
                                                        changed)) {
          return false;
        }
        children.push_back(expanded);
      }
      changed = true;
      continue;
    }

    CppAstNode expanded_child;
    if(!expand_pack_expressions_in_decltype_operand(scope,
                                                    child,
                                                    expanded_child,
                                                    changed)) {
      return false;
    }
    children.push_back(expanded_child);
  }
  out.children.swap(children);
  return true;
}

bool scope_has_template_template_placeholder(Scope & scope,
                                             const string & name,
                                             bool alias_template)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(alias_template) {
      map<string, AliasTemplateDecl *>::const_iterator found =
          current->alias_templates.find(name);
      if(found != current->alias_templates.end() &&
         current->template_bound_template_names.count(name) != 0 &&
         found->second == nullptr) {
        return true;
      }
    } else {
      map<string, ClassTemplateDecl *>::const_iterator found =
          current->class_templates.find(name);
      if(found != current->class_templates.end() &&
         current->template_bound_template_names.count(name) != 0 &&
         found->second == nullptr) {
        return true;
      }
    }
  }
  return false;
}

Scope * root_scope_for_namespace_fallback(Scope & scope)
{
  Scope * current = &scope;
  while(current->parent) {
    current = current->parent;
  }
  return current;
}

std::vector<std::string> namespace_path_for_scope(const Scope & scope)
{
  std::vector<std::string> reversed;
  for(const Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope &&
       !current->name.empty() &&
       current->name != "<global>" &&
       current->name != "<unnamed>") {
      reversed.push_back(current->name);
    }
  }
  return std::vector<std::string>(reversed.rbegin(), reversed.rend());
}

Scope * resolve_namespace_from_root_path(Scope & root,
                                         const std::vector<std::string> & path)
{
  Scope * current = &root;
  for(size_t i = 0; i < path.size(); ++i) {
    current = semantic_lookup::resolve_direct_namespace(*current, path[i]);
    if(!current) {
      return nullptr;
    }
  }
  return current;
}

Scope * resolve_namespace_from_canonical_enclosing_path(
    Scope & scope,
    const QualifiedName & qualified)
{
  Scope & root = *root_scope_for_namespace_fallback(scope);
  if(qualified.rooted) {
    std::vector<std::string> path = qualified.qualifiers;
    path.push_back(qualified.name);
    return resolve_namespace_from_root_path(root, path);
  }

  if(!qualified.qualifiers.empty() || qualified.name.empty()) {
    return nullptr;
  }

  std::vector<std::string> namespace_path = namespace_path_for_scope(scope);
  for(size_t count = namespace_path.size() + 1; count-- > 0;) {
    std::vector<std::string> path(namespace_path.begin(),
                                  namespace_path.begin() + count);
    path.push_back(qualified.name);
    if(Scope * resolved = resolve_namespace_from_root_path(root, path)) {
      return resolved;
    }
    if(count == 0) {
      break;
    }
  }
  return nullptr;
}

Scope * resolve_qualified_scope_for_class_or_namespace_impl(
    template_api::TemplateServices & services,
    Scope & scope,
    const QualifiedName & qualified,
    bool allow_dependent_class_qualifiers)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return nullptr;
  }
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };

  string qualifier_text = qualified.rooted ? "::" : string();
  Scope * resolved_scope = qualified.rooted ? nullptr : &scope;
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(i != 0) {
      qualifier_text += "::";
    }
    qualifier_text += qualified.qualifiers[i];

    QualifiedName qualifier_name;
    qualifier_name.rooted = qualified.rooted;
    qualifier_name.qualifiers.assign(qualified.qualifiers.begin(),
                                     qualified.qualifiers.begin() + i);
    qualifier_name.name = qualified.qualifiers[i];
    Scope * namespace_scope = semantic_lookup::lookup_namespace_name(scope, qualifier_name);
    if(!namespace_scope) {
      namespace_scope =
          resolve_namespace_from_canonical_enclosing_path(scope, qualifier_name);
    }
    if(namespace_scope) {
      resolved_scope = namespace_scope;
      continue;
    }

    TypePtr qualifier_type =
        current_specialization_type_for_dependent_qualifier_text(
            services, scope, qualifier_text);
    if(!qualifier_type) {
      resolve_direct_type_name_lookup(services,
                                      scope,
                                      qualifier_name,
                                      false,
                                      string(),
                                      qualifier_type);
    }
    if(!qualifier_type) {
      return nullptr;
    }
    resolve_instantiated_dependent_type_if_needed(
        services,
        template_api::make_template_environment(scope),
        qualifier_type);
    if(type_is_dependent(qualifier_type) &&
       !allow_dependent_class_qualifiers) {
      return nullptr;
    }

    Scope * member_scope = nullptr;
    if(!prepare_concrete_type_member_scope(
           services,
           template_api::make_template_environment(scope),
           qualifier_type,
           member_scope)) {
      return nullptr;
    }
    resolved_scope = member_scope;
  }
  return resolved_scope;
}

Scope * resolve_qualified_scope_for_class_or_namespace_node(
    template_api::TemplateServices & services,
    Scope & scope,
    const QualifiedName & qualified,
    const CppAstNode & node,
    bool allow_dependent_class_qualifiers)
{
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return nullptr;
  }
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };

  Scope * current = &scope;
  if(qualified.rooted) {
    while(current->parent) {
      current = current->parent;
    }
  }

  std::string qualifier_text = qualified.rooted ? "::" : std::string();
  Scope * resolved_scope = qualified.rooted ? current : &scope;
  for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(i != 0) {
      qualifier_text += "::";
    }
    qualifier_text += qualified.qualifiers[i];

    QualifiedName qualifier_name;
    qualifier_name.rooted = qualified.rooted;
    qualifier_name.qualifiers.assign(qualified.qualifiers.begin(),
                                     qualified.qualifiers.begin() + i);
    qualifier_name.name = qualified.qualifiers[i];
    Scope * namespace_scope = semantic_lookup::lookup_namespace_name(scope, qualifier_name);
    if(!namespace_scope) {
      namespace_scope =
          resolve_namespace_from_canonical_enclosing_path(*current, qualifier_name);
    }
    if(namespace_scope) {
      current = namespace_scope;
      resolved_scope = namespace_scope;
      continue;
    }

    TypePtr qualifier_type;
    const CppAstNode * qualifier_type_syntax =
        cppast_qualifier_type_syntax(node, i);
    if(qualifier_type_syntax) {
      if(qualifier_type_syntax->semantic_type) {
        qualifier_type = qualifier_type_syntax->semantic_type;
      } else {
        parse_decltype_or_typeof_node(
            services, *current, *qualifier_type_syntax, qualifier_type);
      }
    }
    if(!qualifier_type) {
      qualifier_type =
          current_specialization_type_for_dependent_qualifier_text(
              services, *current, qualifier_text);
    }
    const TemplateIdSyntax * qualifier_template_id =
        i < node.qualifier_template_id_syntaxes.size() &&
                !node.qualifier_template_id_syntaxes[i].name.name.empty() ?
            &node.qualifier_template_id_syntaxes[i] :
            nullptr;
    if(!qualifier_template_id &&
       i == 0) {
      if(const TemplateIdSyntax * lone_template_id = cppast_template_id_syntax(node)) {
        const string qualifier_head =
            unqualified_member_name(
                strip_trailing_top_level_template_arguments(
                    qualified.qualifiers[i]));
        if(qualifier_head == lone_template_id->name.name) {
          qualifier_template_id = lone_template_id;
        }
      }
    }
    if(!qualifier_type && qualifier_template_id) {
      resolve_template_id_syntax_type(
          services,
          *current,
          *qualifier_template_id,
          true,
          std::string(),
          qualifier_type,
          template_api::make_template_environment(scope),
          template_api::ClassTemplateSourceUseMode::EmitClassUse,
          false);
      if(!qualifier_type) {
        return nullptr;
      }
    }
    if(!qualifier_type) {
      resolve_direct_type_name_lookup(services,
                                      scope,
                                      qualifier_name,
                                      false,
                                      string(),
                                      qualifier_type);
    }
    if(!qualifier_type) {
      return nullptr;
    }
    resolve_instantiated_dependent_type_if_needed(
        services,
        template_api::make_template_environment(scope),
        qualifier_type);
    if(type_is_dependent(qualifier_type) &&
       !allow_dependent_class_qualifiers) {
      return nullptr;
    }

    Scope * member_scope = nullptr;
    if(!prepare_concrete_type_member_scope(
           services,
           template_api::make_template_environment(*current),
           qualifier_type,
           member_scope)) {
      return nullptr;
    }
    current = member_scope;
    resolved_scope = member_scope;
  }
  return resolved_scope;
}

ClassTemplateDecl * lookup_class_template_impl(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const string & name)
{
  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(name, qualified) &&
     (qualified.rooted || !qualified.qualifiers.empty())) {
    Scope * target =
        resolve_qualified_scope_for_class_or_namespace_impl(services, scope, qualified, false);
    if(!target) {
      return nullptr;
    }
    map<string, ClassTemplateDecl *>::iterator found =
        target->class_templates.find(qualified.name);
    return found == target->class_templates.end() ? nullptr : found->second;
  }

  return semantic_lookup::lookup_unqualified_class_template(scope, name);
}

bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const QualifiedName & qualified,
                                         const ValueBinding *& out)
{
  out = nullptr;
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return false;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_impl(services, scope, qualified, false);
  if(!target) {
    return false;
  }

  map<string, ValueBinding>::const_iterator found = target->values.find(qualified.name);
  if(found != target->values.end()) {
    out = &found->second;
    return true;
  }
  if(target->class_info) {
    semantic_lookup::MemberValueLookupResult member =
        semantic_lookup::lookup_member_value(*target->class_info, qualified.name);
    if(member.binding) {
      out = member.binding;
      return true;
    }
  }
  return false;
}

bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const QualifiedName & qualified,
                                         const CppAstNode * node,
                                         const ValueBinding *& out)
{
  out = nullptr;
  if(!node) {
    return lookup_leaf_qualified_value_binding(services, scope, qualified, out);
  }
  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_node(
          services, scope, qualified, *node, false);
  if(!target) {
    return false;
  }

  map<string, ValueBinding>::const_iterator found = target->values.find(qualified.name);
  if(found != target->values.end()) {
    out = &found->second;
    return true;
  }
  if(target->class_info) {
    semantic_lookup::MemberValueLookupResult member =
        semantic_lookup::lookup_member_value(*target->class_info, qualified.name);
    if(member.binding) {
      out = member.binding;
      return true;
    }
  }
  return false;
}

bool lookup_leaf_qualified_value_binding(template_api::TemplateServices & services,
                                         Scope & scope,
                                         const string & name,
                                         const ValueBinding *& out)
{
  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(name, qualified)) {
    out = nullptr;
    return false;
  }
  return lookup_leaf_qualified_value_binding(services, scope, qualified, out);
}

bool lookup_leaf_qualified_function_bindings(template_api::TemplateServices & services,
                                             Scope & scope,
                                             const QualifiedName & qualified,
                                             vector<FunctionBinding *> & out)
{
  out.clear();
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return false;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_impl(services, scope, qualified, false);
  if(!target) {
    return false;
  }

  if(target->namespace_scope) {
    unordered_set<const Scope *> visited_scopes;
    unordered_set<const FunctionBinding *> seen_bindings;
    collect_leaf_function_bindings_in_namespace_scope(
        *target, qualified.name, visited_scopes, seen_bindings, out);
    return !out.empty();
  }

  map<string, vector<FunctionBinding *> >::const_iterator found =
      target->function_sets.find(qualified.name);
  if(found == target->function_sets.end()) {
    return false;
  }
  out = found->second;
  return !out.empty();
}

bool lookup_leaf_qualified_function_bindings(template_api::TemplateServices & services,
                                             Scope & scope,
                                             const QualifiedName & qualified,
                                             const CppAstNode * node,
                                             vector<FunctionBinding *> & out)
{
  out.clear();
  if(!node) {
    return lookup_leaf_qualified_function_bindings(services, scope, qualified, out);
  }
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return false;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_node(
          services, scope, qualified, *node, false);
  if(!target) {
    return false;
  }

  if(target->namespace_scope) {
    unordered_set<const Scope *> visited_scopes;
    unordered_set<const FunctionBinding *> seen_bindings;
    collect_leaf_function_bindings_in_namespace_scope(
        *target, qualified.name, visited_scopes, seen_bindings, out);
    return !out.empty();
  }

  map<string, vector<FunctionBinding *> >::const_iterator found =
      target->function_sets.find(qualified.name);
  if(found == target->function_sets.end()) {
    return false;
  }
  out = found->second;
  return !out.empty();
}

bool lookup_leaf_qualified_function_templates(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const QualifiedName & qualified,
                                              vector<FunctionTemplateDecl *> & out)
{
  out.clear();
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return false;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_impl(services, scope, qualified, false);
  if(!target) {
    return false;
  }

  if(target->class_info) {
    semantic_lookup::MemberFunctionTemplateLookupResult result =
        semantic_lookup::lookup_visible_member_function_templates(*target->class_info,
                                                                  qualified.name);
    semantic_lookup::append_unique_function_templates(out, result.templates);
    if(!out.empty()) {
      return true;
    }
  }

  semantic_lookup::collect_direct_function_templates(*target, qualified.name, out);
  return !out.empty();
}

bool lookup_leaf_qualified_function_templates(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const QualifiedName & qualified,
                                              const CppAstNode * node,
                                              vector<FunctionTemplateDecl *> & out)
{
  out.clear();
  if(!node) {
    return lookup_leaf_qualified_function_templates(services, scope, qualified, out);
  }
  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return false;
  }

  Scope * target =
      resolve_qualified_scope_for_class_or_namespace_node(
          services, scope, qualified, *node, false);
  if(!target) {
    return false;
  }

  if(target->class_info) {
    semantic_lookup::MemberFunctionTemplateLookupResult result =
        semantic_lookup::lookup_visible_member_function_templates(*target->class_info,
                                                                  qualified.name);
    semantic_lookup::append_unique_function_templates(out, result.templates);
    if(!out.empty()) {
      return true;
    }
  }

  semantic_lookup::collect_direct_function_templates(*target, qualified.name, out);
  return !out.empty();
}

AliasTemplateDecl * lookup_alias_template_impl(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const string & name)
{
  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(name, qualified) &&
     (qualified.rooted || !qualified.qualifiers.empty())) {
    Scope * target =
        resolve_qualified_scope_for_class_or_namespace_impl(services, scope, qualified, false);
    if(!target) {
      return nullptr;
    }
    map<string, AliasTemplateDecl *>::iterator found =
        target->alias_templates.find(qualified.name);
    return found == target->alias_templates.end() ? nullptr : found->second;
  }

  return semantic_lookup::lookup_unqualified_alias_template(scope, name);
}

TypePtr lookup_local_dependent_type_placeholder(Scope & scope, const string & text)
{
  const string normalized = strip_elaborated_type_prefix(trim_space(text));
  if(normalized.empty()) {
    return TypePtr();
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->template_bound_type_names.count(normalized) == 0) {
      continue;
    }
    map<string, TypePtr>::const_iterator found = current->named_types.find(normalized);
    if(found != current->named_types.end() &&
       found->second &&
       named_type_is_template_parameter(found->second)) {
      return found->second;
    }
    break;
  }
  return TypePtr();
}

bool is_identifier_char_for_rewrite(char ch)
{
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool identifier_is_qualified_component_for_rewrite(const string & text, std::size_t pos)
{
  while(pos > 0) {
    const char ch = text[pos - 1];
    if(std::isspace(static_cast<unsigned char>(ch))) {
      --pos;
      continue;
    }
    if(ch != ':') {
      return false;
    }
    if(pos < 2) {
      return false;
    }
    std::size_t previous = pos - 1;
    while(previous > 0 &&
          std::isspace(static_cast<unsigned char>(text[previous - 1]))) {
      --previous;
    }
    return previous > 0 && text[previous - 1] == ':';
  }
  return false;
}

enum ReferenceSuffixKind
{
  RSK_NONE,
  RSK_LVALUE,
  RSK_RVALUE
};

ReferenceSuffixKind trailing_reference_suffix_kind(const string & text,
                                                   string & base_text)
{
  const string trimmed = trim_space(text);
  if(trimmed.size() >= 2 &&
     trimmed.compare(trimmed.size() - 2, 2, "&&") == 0) {
    base_text = trim_space(trimmed.substr(0, trimmed.size() - 2));
    return RSK_RVALUE;
  }
  if(!trimmed.empty() && trimmed[trimmed.size() - 1] == '&') {
    base_text = trim_space(trimmed.substr(0, trimmed.size() - 1));
    return RSK_LVALUE;
  }
  base_text = trimmed;
  return RSK_NONE;
}

bool find_existing_declarator_group(const string & text,
                                    std::size_t & open_pos,
                                    std::size_t & close_pos)
{
  int angle_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(angle_depth != 0 || ch != '(') {
      continue;
    }

    int paren_depth = 0;
    int nested_angle_depth = 0;
    for(std::size_t j = i; j < text.size(); ++j) {
      const char inner = text[j];
      if(inner == '<') {
        ++nested_angle_depth;
        continue;
      }
      if(inner == '>' && nested_angle_depth > 0) {
        --nested_angle_depth;
        continue;
      }
      if(nested_angle_depth != 0) {
        continue;
      }
      if(inner == '(') {
        ++paren_depth;
      } else if(inner == ')') {
        --paren_depth;
        if(paren_depth == 0) {
          if(!trim_space(text.substr(j + 1)).empty()) {
            open_pos = i;
            close_pos = j;
            return true;
          }
          break;
        }
      }
    }
  }
  return false;
}

std::size_t find_top_level_declarator_suffix_start(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
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
      if(angle_depth == 0 && paren_depth == 0) {
        return i;
      }
      ++paren_depth;
      continue;
    }
    if(ch == ')' && paren_depth > 0) {
      --paren_depth;
      continue;
    }
    if(angle_depth == 0 && paren_depth == 0 && ch == '[') {
      return i;
    }
  }
  return string::npos;
}

string append_or_collapse_reference_in_declarator(const string & declarator,
                                                  const string & suffix)
{
  string base_text;
  const ReferenceSuffixKind existing =
      trailing_reference_suffix_kind(declarator, base_text);
  const ReferenceSuffixKind added = suffix == "&&" ? RSK_RVALUE : RSK_LVALUE;
  ReferenceSuffixKind combined = added;
  if(existing != RSK_NONE) {
    combined = (existing == RSK_LVALUE || added == RSK_LVALUE) ?
        RSK_LVALUE :
        RSK_RVALUE;
  }

  string out = base_text;
  if(combined != RSK_NONE) {
    const bool need_space =
        !out.empty() &&
        (std::isalnum(static_cast<unsigned char>(out[out.size() - 1])) ||
         out[out.size() - 1] == '>' ||
         out[out.size() - 1] == ']' ||
         out[out.size() - 1] == ')');
    if(need_space) {
      out += ' ';
    }
    out += combined == RSK_LVALUE ? "&" : "&&";
  }
  return out;
}

string collapse_reference_suffix_text(const string & replacement,
                                      const string & suffix)
{
  const string trimmed = trim_space(replacement);

  std::size_t open_pos = string::npos;
  std::size_t close_pos = string::npos;
  if(find_existing_declarator_group(trimmed, open_pos, close_pos)) {
    const string before = trim_space(trimmed.substr(0, open_pos));
    const string inner = trim_space(trimmed.substr(open_pos + 1, close_pos - open_pos - 1));
    const string after = trimmed.substr(close_pos + 1);
    string out = before;
    if(!out.empty()) {
      out += " ";
    }
    out += "(" + append_or_collapse_reference_in_declarator(inner, suffix) + ")";
    out += after;
    return out;
  }

  const std::size_t suffix_start = find_top_level_declarator_suffix_start(trimmed);
  if(suffix_start != string::npos) {
    const string before = trim_space(trimmed.substr(0, suffix_start));
    const string after = trimmed.substr(suffix_start);
    string out = before;
    if(!out.empty()) {
      out += " ";
    }
    out += "(" + append_or_collapse_reference_in_declarator(string(), suffix) + ")";
    out += after;
    return out;
  }

  return append_or_collapse_reference_in_declarator(trimmed, suffix);
}

bool extract_leading_cv_qualifier_prefix(const string & text,
                                         std::size_t match_start,
                                         std::size_t & prefix_start,
                                         bool & add_const,
                                         bool & add_volatile)
{
  prefix_start = match_start;
  add_const = false;
  add_volatile = false;

  std::size_t pos = match_start;
  while(pos != 0) {
    std::size_t token_end = pos;
    while(token_end != 0 &&
          std::isspace(static_cast<unsigned char>(text[token_end - 1]))) {
      --token_end;
    }
    if(token_end == 0) {
      break;
    }

    std::size_t token_start = token_end;
    while(token_start != 0 && is_identifier_char_for_rewrite(text[token_start - 1])) {
      --token_start;
    }
    if(token_start == token_end) {
      break;
    }

    const string token = text.substr(token_start, token_end - token_start);
    if(token == "const") {
      add_const = true;
    } else if(token == "volatile") {
      add_volatile = true;
    } else {
      break;
    }
    prefix_start = token_start;
    pos = token_start;
  }

  return prefix_start != match_start;
}

bool replacement_text_has_top_level_paren_or_bracket(const string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  for(std::size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == '<') {
      ++angle_depth;
      continue;
    }
    if(ch == '>' && angle_depth > 0) {
      --angle_depth;
      continue;
    }
    if(angle_depth != 0) {
      continue;
    }
    if(ch == '(') {
      ++paren_depth;
    } else if(ch == ')' && paren_depth > 0) {
      --paren_depth;
    } else if(paren_depth == 0 && (ch == '(' || ch == '[' || ch == ']')) {
      return true;
    }
  }
  return false;
}

bool text_has_top_level_cv_suffix(const string & text, const string & qualifier)
{
  const string trimmed = trim_space(text);
  if(trimmed.size() <= qualifier.size()) {
    return false;
  }
  const string needle = " " + qualifier;
  return trimmed.compare(trimmed.size() - needle.size(), needle.size(), needle) == 0;
}

string apply_leading_cv_qualifiers_to_replacement_text(const string & replacement,
                                                       bool add_const,
                                                       bool add_volatile)
{
  if(!add_const && !add_volatile) {
    return replacement;
  }

  string base_text;
  if(trailing_reference_suffix_kind(replacement, base_text) != RSK_NONE) {
    return replacement;
  }

  string out = trim_space(replacement);
  if(out.empty() || replacement_text_has_top_level_paren_or_bracket(out)) {
    return replacement;
  }

  if(add_const && !text_has_top_level_cv_suffix(out, "const")) {
    out += " const";
  }
  if(add_volatile && !text_has_top_level_cv_suffix(out, "volatile")) {
    out += " volatile";
  }
  return out;
}

bool replacement_text_is_explicit_template_id(const string & replacement)
{
  return replacement.find('<') != string::npos &&
         replacement.find('>') != string::npos;
}

string replace_identifier_token_text(const string & text,
                                     const string & name,
                                     const string & replacement,
                                     bool & changed)
{
  if(text.find(name) == string::npos) {
    return text;
  }
  string out;
  std::size_t i = 0;
  while(i < text.size()) {
    if(text.compare(i, name.size(), name) == 0 &&
       !identifier_is_qualified_component_for_rewrite(text, i) &&
       (i == 0 || !is_identifier_char_for_rewrite(text[i - 1])) &&
       (i + name.size() == text.size() ||
        !is_identifier_char_for_rewrite(text[i + name.size()]))) {
      std::size_t prefix_start = i;
      bool add_const = false;
      bool add_volatile = false;
      string adjusted_replacement = replacement;
      if(extract_leading_cv_qualifier_prefix(text, i, prefix_start, add_const, add_volatile)) {
        adjusted_replacement =
            apply_leading_cv_qualifiers_to_replacement_text(replacement,
                                                            add_const,
                                                            add_volatile);
        out.erase(out.size() - (i - prefix_start));
      }
      std::size_t next = i + name.size();
      std::size_t suffix_pos = next;
      while(suffix_pos < text.size() &&
            std::isspace(static_cast<unsigned char>(text[suffix_pos]))) {
        ++suffix_pos;
      }
      if(suffix_pos < text.size() &&
         text[suffix_pos] == '<' &&
         replacement_text_is_explicit_template_id(adjusted_replacement)) {
        out.append(text, i, next - i);
        i = next;
        continue;
      }

      if(suffix_pos + 1 < text.size() &&
         text.compare(suffix_pos, 2, "&&") == 0) {
        out += collapse_reference_suffix_text(adjusted_replacement, "&&");
        i = suffix_pos + 2;
      } else if(suffix_pos < text.size() && text[suffix_pos] == '&') {
        out += collapse_reference_suffix_text(adjusted_replacement, "&");
        i = suffix_pos + 1;
      } else {
        out += adjusted_replacement;
        i = next;
      }
      changed = true;
      continue;
    }
    out += text[i++];
  }
  return out;
}

string replace_identifier_token_text_preserving_sizeof_pack_operands(
    const string & text,
    const string & name,
    const string & replacement,
    bool & changed)
{
  static const char prefix[] = "sizeof...";
  if(text.find(prefix) == string::npos) {
    return replace_identifier_token_text(text, name, replacement, changed);
  }

  string out;
  size_t pos = 0;
  while(pos < text.size()) {
    const size_t found = text.find(prefix, pos);
    if(found == string::npos) {
      bool segment_changed = false;
      out += replace_identifier_token_text(text.substr(pos),
                                           name,
                                           replacement,
                                           segment_changed);
      changed = changed || segment_changed;
      break;
    }

    bool segment_changed = false;
    out += replace_identifier_token_text(text.substr(pos, found - pos),
                                         name,
                                         replacement,
                                         segment_changed);
    changed = changed || segment_changed;

    size_t open = found + sizeof(prefix) - 1;
    while(open < text.size() &&
          std::isspace(static_cast<unsigned char>(text[open]))) {
      ++open;
    }
    if(open >= text.size() || text[open] != '(') {
      out.append(text, found, sizeof(prefix) - 1);
      pos = found + sizeof(prefix) - 1;
      continue;
    }

    size_t close = open + 1;
    int depth = 1;
    for(; close < text.size(); ++close) {
      if(text[close] == '(') {
        ++depth;
      } else if(text[close] == ')') {
        --depth;
        if(depth == 0) {
          ++close;
          break;
        }
      }
    }
    if(depth != 0) {
      out += text.substr(found);
      break;
    }

    out.append(text, found, close - found);
    pos = close;
  }
  return out;
}

}  // namespace

bool try_resolve_type_pack_element_template_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const QualifiedName & template_id,
    const std::vector<std::string> & arg_texts,
    const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
    TypePtr & out)
{
  out.reset();
  if(!scope.valid() ||
     template_id.name != "__type_pack_element" ||
     !template_id.qualifiers.empty() ||
     arg_texts.size() < 2) {
    return false;
  }

  long long selected_index = -1;
  bool index_resolved = false;
  try {
    index_resolved =
        evaluate_non_type_argument_text(services,
                                        scope,
                                        arg_texts[0],
                                        selected_index) == NT_ARG_EVALUATED;
  } catch(const logic_error &) {
    index_resolved = false;
  }
  if(!index_resolved || selected_index < 0) {
    return false;
  }

  Scope & raw_scope = scope.require();
  vector<string> element_texts;
  element_texts.reserve(arg_texts.size() - 1);
  for(size_t i = 1; i < arg_texts.size(); ++i) {
    element_texts.push_back(arg_texts[i]);
  }

  vector<TemplateArgumentSyntax> element_syntaxes;
  const vector<TemplateArgumentSyntax> * element_syntaxes_ptr = nullptr;
  if(arg_syntaxes && arg_syntaxes->size() >= arg_texts.size()) {
    element_syntaxes.reserve(arg_texts.size() - 1);
    for(size_t i = 1; i < arg_texts.size(); ++i) {
      element_syntaxes.push_back((*arg_syntaxes)[i]);
    }
    element_syntaxes_ptr = &element_syntaxes;
  }

  const ExpandedTemplateArgumentInputs expanded =
      expand_template_argument_inputs(services,
                                      raw_scope,
                                      element_texts,
                                      element_syntaxes_ptr);
  if(static_cast<unsigned long long>(selected_index) >= expanded.texts.size()) {
    return false;
  }

  TypePtr selected_type = expanded.type_for(static_cast<size_t>(selected_index));
  const TemplateArgumentSyntax * selected_syntax =
      expanded.syntax_for(static_cast<size_t>(selected_index));
  if(!selected_type && selected_syntax) {
    resolve_type_argument_syntax_type(services,
                                      scope,
                                      *selected_syntax,
                                      true,
                                      selected_type);
  }
  if(!selected_type) {
    parse_type_argument_text(services,
                             scope,
                             expanded.texts[static_cast<size_t>(selected_index)],
                             selected_type);
  }
  if(!selected_type) {
    return false;
  }

  TypePtr resolved_type;
  if(service_type_depends_on_template_parameter(services, selected_type) &&
     resolve_instantiated_dependent_type(services, scope, selected_type, resolved_type) &&
     resolved_type) {
    selected_type = resolved_type;
  }
  if(service_type_depends_on_template_parameter(services, selected_type)) {
    return false;
  }

  out = selected_type;
  return true;
}

bool substitute_expression_node_for_template_arguments(
    Scope & scope,
    const CppAstNode & node,
    const vector<TemplateParameterInfo> & parameters,
    const vector<TemplateArgument> & arguments,
    CppAstNode & out)
{
  map<string, TypePtr> type_replacements;
  map<string, ValueBinding> value_replacements;
  map<string, size_t> pack_size_replacements;
  collect_template_parameter_pack_size_replacements(parameters,
                                                    arguments,
                                                    pack_size_replacements);
  const size_t count = std::min(parameters.size(), arguments.size());
  for(size_t i = 0; i < count; ++i) {
    if(parameters[i].name.empty()) {
      continue;
    }
    if(parameters[i].parameter_pack) {
      continue;
    }
    if(parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
       arguments[i].kind == TemplateArgument::TA_TYPE &&
       arguments[i].type) {
      type_replacements[parameters[i].name] = arguments[i].type;
      continue;
    }
    if(parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       arguments[i].kind == TemplateArgument::TA_VALUE) {
      ValueBinding binding(ValueBinding::VK_VARIABLE,
                           !arguments[i].text.empty() ?
                               arguments[i].text :
                               to_string(arguments[i].value),
                           arguments[i].type);
      binding.dependent_template_value = arguments[i].dependent;
      if(arguments[i].dependent) {
        binding.non_type_template_argument_text = binding.name;
      } else {
        binding.has_constant_value = true;
        binding.constant_value = arguments[i].value;
      }
      value_replacements[parameters[i].name] = binding;
    }
  }
  CppAstNode current;
  bool pack_size_changed = false;
  if(!pack_size_replacements.empty()) {
    if(!substitute_sizeof_pack_count_expression_node(node,
                                                     pack_size_replacements,
                                                     current,
                                                     pack_size_changed)) {
      return false;
    }
  } else {
    current = clone_expression_node_for_template_substitution(node);
  }

  if(!type_replacements.empty()) {
    CppAstNode substituted;
    if(!substitute_type_pack_expression_node(scope,
                                             current,
                                             type_replacements,
                                             substituted)) {
      return false;
    }
    current = substituted;
  }
  if(!value_replacements.empty()) {
    CppAstNode substituted;
    if(!substitute_value_pack_expression_node(current,
                                              value_replacements,
                                              substituted)) {
      return false;
    }
    current = substituted;
  }
  (void)pack_size_changed;
  out = current;
  return true;
}

vector<string> rewrite_decltype_expression_pack_texts(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & text)
{
  return rewrite_decltype_expression_pack_texts_impl(services, scope, text);
}

TypePtr lookup_exact_local_type_name(template_api::TemplateServices & services,
                                     Scope & scope,
                                     const string & name)
{
  return lookup_exact_local_type_name_impl(services, scope, name);
}

bool text_mentions_template_placeholders(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const string & text)
{
  Scope & raw_scope = scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  const callsemantic_internal::IdentifierTokenSet identifiers =
      callsemantic_internal::collect_identifier_tokens(text);

  const auto identifier_mentions_template_type_placeholder =
      [&](const string & name) -> bool
      {
        for(Scope * current = &raw_scope; current; current = current->parent) {
          if(current->named_type_packs.find(name) !=
                 current->named_type_packs.end() ||
             current->named_pack_sizes.count(name) != 0) {
            return false;
          }
          const bool template_bound =
              current->template_bound_type_names.count(name) != 0;
          map<string, TypePtr>::const_iterator found = current->named_types.find(name);
          if(found == current->named_types.end()) {
            if(template_bound) {
              return true;
            }
            continue;
          }
          if(template_bound) {
            if(!found->second) {
              return true;
            }
            if(type_is_dependent(found->second)) {
              return true;
            }
          }
          return found->second &&
                 named_type_is_template_parameter(found->second);
        }
        return false;
      };
  const auto identifier_mentions_template_value_placeholder =
      [&](const string & name) -> bool
      {
        for(Scope * current = &raw_scope; current; current = current->parent) {
          const bool template_bound =
              current->template_bound_value_names.count(name) != 0;
          map<string, ValueBinding>::const_iterator found = current->values.find(name);
          if(template_bound) {
            if(found == current->values.end() ||
               (!found->second.has_constant_value && !found->second.has_constexpr_value)) {
              return true;
            }
            return value_binding_depends_on_template_parameters(services, found->second);
          }
          if(found != current->values.end()) {
            return value_binding_depends_on_template_parameters(services, found->second);
          }
        }
        return false;
      };
  const auto identifier_mentions_template_type_pack_placeholder =
      [&](const string & name) -> bool
      {
        for(Scope * current = &raw_scope; current; current = current->parent) {
          map<string, vector<TypePtr> >::const_iterator found =
              current->named_type_packs.find(name);
          if(found == current->named_type_packs.end()) {
            continue;
          }
          for(size_t i = 0; i < found->second.size(); ++i) {
            if(type_is_dependent(found->second[i])) {
              return true;
            }
          }
          return false;
        }
        return false;
      };
  const auto identifier_mentions_template_template_placeholder =
      [&](const string & name) -> bool
      {
        for(Scope * current = &raw_scope; current; current = current->parent) {
          if(current->template_bound_template_names.count(name) == 0) {
            continue;
          }
          map<string, ClassTemplateDecl *>::const_iterator found_class =
              current->class_templates.find(name);
          map<string, AliasTemplateDecl *>::const_iterator found_alias =
              current->alias_templates.find(name);
          return (found_class == current->class_templates.end() &&
                  found_alias == current->alias_templates.end()) ||
                 (found_class != current->class_templates.end() && !found_class->second) ||
                 (found_alias != current->alias_templates.end() && !found_alias->second);
        }
        return false;
      };

  for(callsemantic_internal::IdentifierTokenSet::InternedName interned_name :
      identifiers.names) {
    const string & name = *interned_name;
    if(identifier_mentions_template_type_placeholder(name) ||
       identifier_mentions_template_value_placeholder(name) ||
       identifier_mentions_template_type_pack_placeholder(name) ||
       identifier_mentions_template_template_placeholder(name)) {
      return true;
    }
  }
  return false;
}

bool text_mentions_dependent_non_namespace_binding_names(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  Scope & raw_scope = scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  const callsemantic_internal::IdentifierTokenSet identifiers =
      callsemantic_internal::collect_identifier_tokens(text);
  for(callsemantic_internal::IdentifierTokenSet::InternedName interned_name :
      identifiers.names) {
    const string & name = *interned_name;
    bool seen_type_name = false;
    bool seen_value_name = false;
    bool seen_pack_name = false;
    bool seen_template_name = false;
    for(Scope * current = &raw_scope; current; current = current->parent) {
      if(current->namespace_scope || current->parent == nullptr) {
        break;
      }

      if(!seen_type_name) {
        if(current->named_type_packs.find(name) != current->named_type_packs.end() ||
           current->named_pack_sizes.count(name) != 0) {
          seen_type_name = true;
        } else {
          map<string, TypePtr>::const_iterator found = current->named_types.find(name);
          if(found != current->named_types.end()) {
            seen_type_name = true;
            if(found->second &&
               type_is_dependent(found->second)) {
              return true;
            }
          }
        }
      }

      if(!seen_value_name) {
        const bool template_bound =
            current->template_bound_value_names.count(name) != 0;
        map<string, ValueBinding>::const_iterator found = current->values.find(name);
        if(template_bound) {
          seen_value_name = true;
          if(found == current->values.end() ||
             (!found->second.has_constant_value && !found->second.has_constexpr_value) ||
             value_binding_depends_on_template_parameters(services, found->second)) {
            return true;
          }
        } else if(found != current->values.end()) {
          seen_value_name = true;
          if(value_binding_depends_on_template_parameters(services, found->second)) {
            return true;
          }
        }
      }

      if(!seen_pack_name) {
        map<string, vector<TypePtr> >::const_iterator found_type_pack =
            current->named_type_packs.find(name);
        if(found_type_pack != current->named_type_packs.end()) {
          seen_pack_name = true;
          for(size_t i = 0; i < found_type_pack->second.size(); ++i) {
            if(type_is_dependent(found_type_pack->second[i])) {
              return true;
            }
          }
        } else {
          map<string, vector<ValueBinding> >::const_iterator found_value_pack =
              current->named_value_packs.find(name);
          if(found_value_pack != current->named_value_packs.end()) {
            seen_pack_name = true;
            for(size_t i = 0; i < found_value_pack->second.size(); ++i) {
              if(value_binding_depends_on_template_parameters(
                     services, found_value_pack->second[i])) {
                return true;
              }
            }
          }
        }
      }

      if(!seen_template_name &&
         current->template_bound_template_names.count(name) != 0) {
        seen_template_name = true;
        map<string, ClassTemplateDecl *>::const_iterator found_class =
            current->class_templates.find(name);
        map<string, AliasTemplateDecl *>::const_iterator found_alias =
            current->alias_templates.find(name);
        if((found_class == current->class_templates.end() &&
            found_alias == current->alias_templates.end()) ||
           (found_class != current->class_templates.end() && !found_class->second) ||
           (found_alias != current->alias_templates.end() && !found_alias->second)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool text_mentions_non_namespace_binding_names(
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  Scope & raw_scope = scope.require();
  const callsemantic_internal::IdentifierTokenSet identifiers =
      callsemantic_internal::collect_identifier_tokens(text);
  for(callsemantic_internal::IdentifierTokenSet::InternedName interned_name :
      identifiers.names) {
    const string & name = *interned_name;
    for(Scope * current = &raw_scope; current; current = current->parent) {
      if(current->namespace_scope || current->parent == nullptr) {
        break;
      }
      if(current->named_types.find(name) != current->named_types.end() ||
         current->values.find(name) != current->values.end() ||
         current->named_type_packs.find(name) != current->named_type_packs.end() ||
         current->named_value_packs.find(name) != current->named_value_packs.end() ||
         current->class_templates.find(name) != current->class_templates.end()) {
        return true;
      }
    }
  }
  return false;
}

bool text_mentions_current_specialization_names(
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  Scope & raw_scope = scope.require();
  const callsemantic_internal::IdentifierTokenSet identifiers =
      callsemantic_internal::collect_identifier_tokens(text);
  for(callsemantic_internal::IdentifierTokenSet::InternedName interned_name :
      identifiers.names) {
    const string & name = *interned_name;
    for(Scope * current = &raw_scope; current; current = current->parent) {
      if(!current->class_info) {
        if(current->namespace_scope || current->parent == nullptr) {
          break;
        }
        continue;
      }
      if((!current->class_info->name.empty() &&
          current->class_info->name == name) ||
         (current->class_info->source_template &&
          !current->class_info->source_template->name.empty() &&
          current->class_info->source_template->name == name)) {
        return true;
      }
    }
  }
  return false;
}

bool scope_has_template_placeholders(template_api::TemplateServices & services,
                                     template_api::TemplateEnvironmentHandle scope)
{
  Scope & raw_scope = scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  const bool concrete_instantiated_class_scope =
      raw_scope.class_info &&
      raw_scope.class_info->source_template &&
      !raw_scope.class_info->dependent_instantiation;
  unordered_set<string> seen_type_names;
  unordered_set<string> seen_type_pack_names;
  unordered_set<string> seen_value_names;
  seen_type_names.reserve(16);
  seen_type_pack_names.reserve(8);
  seen_value_names.reserve(16);
  for(Scope * current = &raw_scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(concrete_instantiated_class_scope &&
       current != &raw_scope &&
       current->class_info == nullptr) {
      break;
    }
    if(current != &raw_scope &&
       current->class_info &&
       current->class_info->source_template &&
       !current->class_info->dependent_instantiation) {
      break;
    }
    for(const auto & named : current->named_types) {
      if(seen_type_names.count(named.first) == 0 &&
         type_is_dependent(named.second)) {
        return true;
      }
    }
    for(const auto & pack : current->named_type_packs) {
      if(seen_type_pack_names.count(pack.first) != 0) {
        continue;
      }
      for(size_t i = 0; i < pack.second.size(); ++i) {
        if(type_is_dependent(pack.second[i])) {
          return true;
        }
      }
    }
    for(const auto & value : current->values) {
      if(seen_value_names.count(value.first) == 0 &&
         value_binding_depends_on_template_parameters(services, value.second)) {
        return true;
      }
    }
    for(const auto & named : current->named_types) {
      seen_type_names.insert(named.first);
    }
    for(const auto & pack : current->named_type_packs) {
      seen_type_pack_names.insert(pack.first);
    }
    for(const auto & value : current->values) {
      seen_value_names.insert(value.first);
    }
  }
  return false;
}

string instantiation_arg_text_list(const vector<string> & arg_texts)
{
  string out;
  for(size_t i = 0; i < arg_texts.size(); ++i) {
    if(i != 0) {
      out += ",";
    }
    out += arg_texts[i];
  }
  return out;
}

TypePtr current_specialization_type_for_lookup_text(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & normalized_name)
{
  for(Scope * current = &scope; current; current = current->parent) {
    const std::vector<std::string> * class_arg_texts = nullptr;
    if(!current->class_info ||
       !current->class_info->source_template ||
       !(class_arg_texts =
             template_metadata::argument_texts(*current->class_info))) {
      continue;
    }

    const auto match_current_class_name =
        [&](const string & candidate) -> TypePtr
        {
          if(candidate.empty()) {
            return TypePtr();
          }
          TypePtr matched =
              match_wrapped_type_text(normalized_name,
                                      candidate,
                                      current->class_info->type);
          if(matched) {
            return matched;
          }
          return TypePtr();
        };
    if(TypePtr matched =
           match_current_class_name(current->class_info->name)) {
      return matched;
    }
    if(TypePtr matched =
           match_current_class_name(current->class_info->qualified_name)) {
      return matched;
    }
    if(TypePtr matched =
           match_current_class_name(current->class_info->display_qualified_name)) {
      return matched;
    }

    const string arg_list = instantiation_arg_text_list(*class_arg_texts);
    const string injected_unqualified = current->class_info->source_template->name;
    const string unqualified = injected_unqualified + "<" + arg_list + ">";
    TypePtr matched = match_wrapped_type_text(normalized_name,
                                              injected_unqualified,
                                              current->class_info->type);
    if(matched) {
      return matched;
    }
    matched = match_wrapped_type_text(normalized_name,
                                      unqualified,
                                      current->class_info->type);
    if(matched) {
      return matched;
    }
    if(current->class_info->source_template->declaring_scope) {
      const string injected_qualified =
          semantic_lookup::scope_qualified_name(
              *current->class_info->source_template->declaring_scope,
              current->class_info->source_template->name);
      const string qualified = injected_qualified + "<" + arg_list + ">";
      matched = match_wrapped_type_text(normalized_name,
                                        injected_qualified,
                                        current->class_info->type);
      if(matched) {
        return matched;
      }
      matched = match_wrapped_type_text(normalized_name,
                                        qualified,
                                        current->class_info->type);
      if(matched) {
        return matched;
      }
    }
  }
  return TypePtr();
}

bool substitute_type(const TypePtr & type,
                     const vector<TemplateParameterInfo> & parameters,
                     const vector<TemplateArgument> & arguments,
                     TypePtr & out)
{
  out.reset();
  return substitute_type_impl(type, parameters, arguments, nullptr, out);
}

bool substitute_type(Scope & scope,
                     const TypePtr & type,
                     const vector<TemplateParameterInfo> & parameters,
                     const vector<TemplateArgument> & arguments,
                     TypePtr & out)
{
  out.reset();
  return substitute_type_impl(type, parameters, arguments, &scope, out);
}

ClassTemplateDecl * lookup_class_template(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const string & name)
{
  return lookup_class_template_impl(services, scope, name);
}

AliasTemplateDecl * lookup_alias_template(template_api::TemplateServices & services,
                                          Scope & scope,
                                          const string & name)
{
  return lookup_alias_template_impl(services, scope, name);
}

const vector<TypePtr> * lookup_type_pack(Scope & scope, const string & name)
{
  string trimmed = trim_space(name);
  string stripped_typename;
  if(strip_leading_typename_text(trimmed, stripped_typename)) {
    trimmed = stripped_typename;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    map<string, vector<TypePtr> >::const_iterator found =
        current->named_type_packs.find(trimmed);
    if(found != current->named_type_packs.end()) {
      return &found->second;
    }
  }
  return nullptr;
}

bool expand_bound_type_pack_pattern_text(Scope & scope,
                                         const string & pattern,
                                         vector<string> & out)
{
  vector<pair<string, const vector<TypePtr> *> > packs;
  vector<pair<string, const vector<ValueBinding> *> > value_packs;
  const callsemantic_internal::IdentifierTokenSet identifiers =
      callsemantic_internal::collect_identifier_tokens(pattern);
  set<string> seen_type_pack_names;
  set<string> seen_value_pack_names;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(map<string, vector<TypePtr> >::const_iterator pack =
            current->named_type_packs.begin();
        pack != current->named_type_packs.end();
        ++pack) {
      const string & pack_name = pack->first;
      if(pack_name.empty() ||
         seen_type_pack_names.count(pack_name) != 0 ||
         !identifiers.contains(pack_name)) {
        continue;
      }
      seen_type_pack_names.insert(pack_name);
      packs.push_back(make_pair(pack_name, &pack->second));
    }
    for(map<string, vector<ValueBinding> >::const_iterator pack =
            current->named_value_packs.begin();
        pack != current->named_value_packs.end();
        ++pack) {
      const string & pack_name = pack->first;
      if(pack_name.empty() ||
         seen_value_pack_names.count(pack_name) != 0 ||
         !identifiers.contains(pack_name)) {
        continue;
      }
      seen_value_pack_names.insert(pack_name);
      value_packs.push_back(make_pair(pack_name, &pack->second));
    }
  }

  if(packs.empty() && value_packs.empty()) {
    return false;
  }

  const size_t pack_size =
      !packs.empty() ? packs[0].second->size() : value_packs[0].second->size();
  for(size_t i = 1; i < packs.size(); ++i) {
    if(packs[i].second->size() != pack_size) {
      return false;
    }
  }
  for(size_t i = 0; i < value_packs.size(); ++i) {
    if(value_packs[i].second->size() != pack_size) {
      return false;
    }
  }

  for(size_t i = 0; i < pack_size; ++i) {
    string rewritten = pattern;
    bool changed = false;
    for(size_t j = 0; j < packs.size(); ++j) {
      rewritten = replace_identifier_token_text(
          rewritten,
          packs[j].first,
          reparseable_type_argument_text((*(packs[j].second))[i]),
          changed);
    }
    for(size_t j = 0; j < value_packs.size(); ++j) {
      rewritten = replace_identifier_token_text(
          rewritten,
          value_packs[j].first,
          substituted_value_pack_argument_text((*(value_packs[j].second))[i]),
          changed);
    }
    out.push_back(changed ? rewritten : pattern);
  }
  return true;
}

string type_argument_lookup_text(template_api::TemplateTypeSystem & type_system,
                                 const TypePtr & type)
{
  if(!type) {
    return string();
  }
  return type_contains_function_local_class(type_system, type) ?
      reparseable_type_argument_text(type) :
      template_argument_type_text(type);
}

string lookup_text_for_type_argument(template_api::TemplateTypeSystem & type_system,
                                     const TypePtr & type)
{
  return type_argument_lookup_text(type_system, type);
}

string lookup_text_for_type_argument(SemanticContext & ctx,
                                     const TypePtr & type)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return type_argument_lookup_text(type_system, type);
      });
}

void canonicalize_simple_dependent_argument_texts(
    template_api::TemplateTypeSystem & type_system,
    vector<TemplateArgument> & arguments)
{
  for(size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].kind != TemplateArgument::TA_TYPE ||
       !arguments[i].type ||
       !type_depends_on_template_parameter(type_system, arguments[i].type) ||
       !is_simple_dependent_argument_text(arguments[i].text)) {
      continue;
    }
    if(type_contains_partial_order_artifact(arguments[i].type)) {
      continue;
    }
    const string canonical =
        lookup_text_for_type_argument(type_system, arguments[i].type);
    if(!canonical.empty()) {
      arguments[i].text = canonical;
    }
  }
}

bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const TypePtr & type)
{
  if(!type) {
    return false;
  }

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;

  case Type::TK_NAMED:
  {
    if(named_type_has_dependent_semantic(type) ||
       named_type_key_contains_dependent_semantic(type)) {
      return true;
    }
    if(semantic_model::ClassInfo * info =
           template_api::find_named_type_class_info(type_system.model, type)) {
      if(info->dependent_instantiation) {
        return true;
      }
      if(template_arguments_are_dependent(
             info->instantiation_arguments,
             [&type_system, &type](const TypePtr & argument_type)
             {
               return argument_type != type &&
                      type_depends_on_template_parameter(type_system,
                                                         argument_type);
             })) {
        return true;
      }
      if(info->enclosing_scope &&
         info->enclosing_scope->class_info &&
         info->enclosing_scope->class_info->type &&
         type_depends_on_template_parameter(type_system,
                                            info->enclosing_scope->class_info->type)) {
        return true;
      }
    }
    return false;
  }

  case Type::TK_ATOMIC:
    return type_depends_on_template_parameter(type_system, type->inner);

  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return (type->kind == Type::TK_ARRAY && !type->bound_text.empty()) ||
           type_depends_on_template_parameter(type_system, type->inner) ||
           (type->kind == Type::TK_MEMBER_POINTER &&
            type_depends_on_template_parameter(type_system, type->owner));

  case Type::TK_FUNCTION:
    if(type_depends_on_template_parameter(type_system, type->inner)) {
      return true;
    }
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(type_depends_on_template_parameter(type_system, type->params[i])) {
        return true;
      }
    }
    return false;
  }

  return false;
}

bool type_depends_on_template_parameter(SemanticContext & ctx,
                                        const TypePtr & type)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return type_depends_on_template_parameter(type_system, type);
      });
}

enum class DependentNamedTypeResolutionStatus
{
  Fallback,
  Resolved,
  KeepDependent
};

struct DependentNamedTypeResolutionGuard
{
  std::pair<const Scope *, string> key;
  bool active = false;

  explicit DependentNamedTypeResolutionGuard(const Scope * scope, const string & text)
    : key(scope, text)
  {
    active = visiting().insert(key).second;
  }

  ~DependentNamedTypeResolutionGuard()
  {
    if(active) {
      visiting().erase(key);
    }
  }

  static set<pair<const Scope *, string> > & visiting()
  {
    static set<pair<const Scope *, string> > state;
    return state;
  }
};

bool named_type_has_unstable_local_spelling(template_api::TemplateServices & services,
                                            const TypePtr & type)
{
  if(!type) {
    return false;
  }
  ClassInfo * info = template_api::find_named_type_class_info(
      service_type_system(services).model,
      type);
  if(!info) {
    return false;
  }
  return info->is_lambda_closure ||
         (info->enclosing_scope && info->enclosing_scope->function != nullptr);
}

bool try_resolve_bound_value_template_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & arg,
    TemplateArgument & out,
    bool & changed)
{
  out = arg;
  if(arg.kind != TemplateArgument::TA_VALUE) {
    return false;
  }

  if(resolve_instantiated_dependent_type_if_needed(services, scope, out.type)) {
    changed = true;
  }

	  const string name = semantic_utils::trim_space(arg.text);
	  if(out.type &&
	     !service_type_depends_on_template_parameter(services, out.type)) {
	    if(arg.source_syntax &&
	       !arg.source_syntax->expression &&
	       (arg.source_syntax->type_id || arg.source_syntax->template_id)) {
	      long long syntax_value = 0;
	      string syntax_eval_error;
	      const witness::ScopedTemplateWitnessSourceCapturePause source_pause;
      const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
          function_call_pause;
      const template_api::ScopedTemplateWitnessLifecyclePause lifecycle_pause;
      const NonTypeArgumentStatus syntax_status =
          evaluate_non_type_argument_syntax(services,
                                            scope,
                                            *arg.source_syntax,
                                            syntax_value,
                                            &syntax_eval_error,
                                            out.type);
      if(syntax_status == NT_ARG_EVALUATED) {
        out.value = syntax_value;
        out.dependent = false;
        out.text.clear();
        changed = true;
        return true;
      }
      if(syntax_status == NT_ARG_DEPENDENT) {
        return false;
      }
    }

    const CppAstNode * expression = arg.expression.get();
    if(!expression && arg.source_syntax && arg.source_syntax->expression) {
      expression = arg.source_syntax->expression.get();
    }
    if(expression) {
      long long expression_value = 0;
      string eval_error;
      const NonTypeArgumentStatus status =
          evaluate_non_type_argument_expression(
              services,
              scope,
              *expression,
              expression_value,
              &eval_error,
              out.type);
      if(status == NT_ARG_EVALUATED) {
        out.value = expression_value;
        out.dependent = false;
        out.text.clear();
        changed = true;
        return true;
      }
      if(status == NT_ARG_DEPENDENT) {
        return false;
      }
    }

    long long literal_value = 0;
    bool have_literal_value = parse_integer_literal_count_text(name, literal_value);
    if(!have_literal_value && name == "true") {
      literal_value = 1;
      have_literal_value = true;
    } else if(!have_literal_value && name == "false") {
      literal_value = 0;
      have_literal_value = true;
    }
    if(have_literal_value) {
      out.value = literal_value;
      out.dependent = false;
      out.text.clear();
      changed = true;
      return true;
    }
  }

  if(!is_identifier_text(name)) {
    return false;
  }

  Scope & raw_scope = scope.require();
  for(Scope * current = &raw_scope; current; current = current->parent) {
    map<string, ValueBinding>::const_iterator found = current->values.find(name);
    if(found == current->values.end()) {
      continue;
    }
    const ValueBinding & binding = found->second;
    if(binding.dependent_template_value) {
      return false;
    }
    long long value = 0;
    if(binding.has_constant_value) {
      value = binding.constant_value;
    } else if(binding.has_constexpr_value &&
              constant_eval::constexpr_value_to_integral(
                  binding.constexpr_value, value)) {
      // `value` was filled above.
    } else {
      return false;
    }

    out.value = value;
    out.dependent = false;
    out.text.clear();
    if(binding.type) {
      out.type = binding.type;
    }
    changed = true;
    return true;
  }
  return false;
}

bool resolve_instantiated_template_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgument & arg,
    TemplateArgument & out,
    bool & changed)
{
  out = arg;
  switch(arg.kind) {
  case TemplateArgument::TA_TYPE:
    if(!arg.type) {
      return !arg.dependent;
    }
    if(service_type_depends_on_template_parameter(services, arg.type)) {
      TypePtr resolved_type = arg.type;
      if(!resolve_instantiated_dependent_type_if_needed(services, scope, resolved_type) ||
         service_type_depends_on_template_parameter(services, resolved_type)) {
        return false;
      }
      out.type = resolved_type;
      out.dependent = false;
      out.text.clear();
      changed = true;
      return true;
    }
    if(arg.dependent) {
      out.dependent = false;
      changed = true;
    }
    return true;

  case TemplateArgument::TA_VALUE:
    if(arg.dependent ||
       arg.expression ||
       (arg.source_syntax && arg.source_syntax->expression) ||
       !arg.text.empty()) {
      TemplateArgument resolved_value;
      if(try_resolve_bound_value_template_argument(
             services, scope, arg, resolved_value, changed)) {
        out = resolved_value;
        return true;
      }
      if(arg.dependent) {
        return false;
      }
    }
    return !arg.dependent;

  case TemplateArgument::TA_CLASS_TEMPLATE:
  case TemplateArgument::TA_ALIAS_TEMPLATE:
    return !arg.dependent && arg.template_decl != nullptr;
  }
  return false;
}

bool try_resolve_dependent_class_instantiation_from_metadata(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  ClassInfo * info = template_api::find_named_type_class_info(
      service_type_system(services).model,
      type);
  if(!info ||
     !info->source_template ||
     info->instantiation_arguments.empty()) {
    return false;
  }

  vector<TemplateArgument> resolved_arguments;
  resolved_arguments.reserve(info->instantiation_arguments.size());
  bool changed = info->dependent_instantiation;
  for(size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
    TemplateArgument resolved_arg;
    if(!resolve_instantiated_template_argument(
           services, scope, info->instantiation_arguments[i], resolved_arg, changed)) {
      return false;
    }
    resolved_arguments.push_back(resolved_arg);
  }

  if(!changed ||
     !template_arguments_fully_bind_parameters(info->source_template->parameters,
                                               resolved_arguments) ||
     template_arguments_are_dependent(
         resolved_arguments,
         [&services](const TypePtr & candidate)
         {
           return service_type_depends_on_template_parameter(services, candidate);
         })) {
    return false;
  }

  Scope & raw_scope = scope.require();
  template_api::TemplateTypeLookupRequest lookup;
  lookup.scope = &raw_scope;
  lookup.allow_class_templates = true;
  lookup.name.name = info->source_template->name;

  template_api::TemplateSelectedClassTemplateIdRequest request;
  request.lookup = lookup;
  request.class_template = info->source_template;
  request.resolved_arguments.swap(resolved_arguments);
  if(!service_resolve_selected_class_template_id(services, request, out) ||
     !out ||
     service_type_depends_on_template_parameter(services, out)) {
    out.reset();
    return false;
  }
  return true;
}

void bind_resolved_class_template_argument_for_later_parameters(
    Scope & scope,
    const TemplateParameterInfo & parameter,
    const TemplateArgument & argument)
{
  if(parameter.parameter_pack || parameter.name.empty()) {
    return;
  }

  vector<string> names;
  names.push_back(parameter.name);
  for(size_t i = 0; i < parameter.alternate_names.size(); ++i) {
    if(parameter.alternate_names[i].empty() ||
       parameter.alternate_names[i] == parameter.name) {
      continue;
    }
    names.push_back(parameter.alternate_names[i]);
  }

  for(size_t i = 0; i < names.size(); ++i) {
    if(parameter.kind == TemplateParameterInfo::TP_TYPE &&
       argument.kind == TemplateArgument::TA_TYPE &&
       argument.type) {
      template_scope::bind_named_type(scope, names[i], argument.type);
    } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
              argument.kind == TemplateArgument::TA_VALUE) {
      TypePtr value_type = argument.type ? argument.type : parameter.value_type;
      template_scope::bind_non_type_value(scope,
                                          names[i],
                                          value_type,
                                          argument.value,
                                          argument.dependent,
                                          !argument.dependent ? argument.text : string(),
                                          !argument.dependent ?
                                              const_cast<FunctionBinding *>(
                                                  argument.function_value) :
                                              nullptr,
                                          !argument.dependent ?
                                              argument.value_binding :
                                              nullptr);
    } else if(parameter.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE &&
              (argument.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
               argument.kind == TemplateArgument::TA_ALIAS_TEMPLATE)) {
      template_scope::bind_template_template_argument(scope, names[i], argument);
    }
  }
}

bool resolve_carried_class_template_defaults_from_prefix(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const ClassTemplateDecl & class_template,
    const vector<DependentAliasTemplateArgumentSyntax> & dependent_arguments,
    const vector<TemplateArgument> & prefix_arguments,
    vector<TemplateArgument> & out)
{
  vector<string> prefix_texts;
  vector<TemplateArgumentSyntax> prefix_syntaxes;
  prefix_texts.reserve(prefix_arguments.size());
  prefix_syntaxes.reserve(prefix_arguments.size());
  for(size_t i = 0; i < prefix_arguments.size(); ++i) {
    string text = i < dependent_arguments.size() ?
        trim_space(dependent_arguments[i].text) :
        string();
    if(text.empty()) {
      text = template_model::template_argument_text(
          prefix_arguments[i],
          [](const TypePtr & type)
          {
            return type ? describe_type(type) : string();
          });
    }
    TemplateArgumentSyntax syntax =
        i < dependent_arguments.size() ?
            dependent_arguments[i].syntax :
            TemplateArgumentSyntax();
    if(syntax.text.empty()) {
      syntax.text = text;
    }
    if(prefix_arguments[i].kind == TemplateArgument::TA_TYPE) {
      syntax.resolved_type = prefix_arguments[i].type;
    }
    if(prefix_arguments[i].kind == TemplateArgument::TA_VALUE &&
       prefix_arguments[i].expression &&
       !syntax.expression) {
      syntax.expression.reset(new CppAstNode(*prefix_arguments[i].expression));
      syntax.source_location_id = prefix_arguments[i].expression->source_location_id;
    }
    prefix_texts.push_back(text);
    prefix_syntaxes.push_back(syntax);
  }

  vector<TemplateArgument> resolved_arguments;
  if(!template_api::resolve_template_arguments(
         services,
         scope,
         class_template.parameters,
         prefix_texts,
         prefix_syntaxes.empty() ? nullptr : &prefix_syntaxes,
         resolved_arguments,
         class_template.declaring_scope ?
             template_api::make_template_environment(*class_template.declaring_scope) :
             template_api::TemplateEnvironmentHandle())) {
    return false;
  }
  if(!template_arguments_fully_bind_parameters(class_template.parameters,
                                               resolved_arguments)) {
    return false;
  }
  out.swap(resolved_arguments);
  return true;
}

bool try_resolve_carried_dependent_class_type_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const ClassTemplateDecl & class_template,
    const vector<DependentAliasTemplateArgumentSyntax> & dependent_arguments,
    vector<TemplateArgument> & out)
{
  out.clear();
  if(dependent_arguments.empty()) {
    return false;
  }
  size_t pack_index = class_template.parameters.size();
  for(size_t i = 0; i < class_template.parameters.size(); ++i) {
    if(class_template.parameters[i].parameter_pack) {
      pack_index = i;
      break;
    }
  }

  Scope & raw_scope = scope.require();
  Scope bound_scope(&raw_scope, "", false);
  template_api::TemplateEnvironmentHandle bound_env =
      template_api::make_template_environment(bound_scope);

  out.reserve(dependent_arguments.size());
  for(size_t i = 0; i < dependent_arguments.size(); ++i) {
    const TemplateParameterInfo * parameter = nullptr;
    if(pack_index != class_template.parameters.size() &&
       i >= pack_index) {
      parameter = &class_template.parameters[pack_index];
    } else if(i < class_template.parameters.size()) {
      parameter = &class_template.parameters[i];
    }
    if(!parameter ||
       (parameter->kind != TemplateParameterInfo::TP_TYPE &&
        parameter->kind != TemplateParameterInfo::TP_NON_TYPE)) {
      out.clear();
      return false;
    }
    if(dependent_arguments[i].source_defaulted) {
      if(resolve_carried_class_template_defaults_from_prefix(
             services, scope, class_template, dependent_arguments, out, out)) {
        return true;
      }
      out.clear();
      return false;
    }

    TemplateArgument carried;
    carried.text = trim_space(dependent_arguments[i].text);
    carried.source_syntax.reset(new TemplateArgumentSyntax(dependent_arguments[i].syntax));
    if(dependent_arguments[i].syntax.expression) {
      carried.expression.reset(new CppAstNode(*dependent_arguments[i].syntax.expression));
    }
    if(parameter->kind == TemplateParameterInfo::TP_TYPE) {
      TypePtr carried_type = dependent_arguments[i].type;
      if(!carried_type) {
        resolve_type_argument_syntax_type(services,
                                          scope,
                                          dependent_arguments[i].syntax,
                                          true,
                                          carried_type);
      }
      if(!carried_type) {
        out.clear();
        return false;
      }
      carried.kind = TemplateArgument::TA_TYPE;
      carried.type = carried_type;
      carried.dependent =
          service_type_depends_on_template_parameter(services, carried.type);
    } else {
      TypePtr value_type;
      if(!template_resolution::resolve_non_type_template_parameter_type(
             services, bound_env, *parameter, value_type) ||
         !value_type) {
        out.clear();
        return false;
      }
      carried.kind = TemplateArgument::TA_VALUE;
      carried.type = value_type;
      carried.dependent = true;
    }

    TemplateArgument resolved;
    bool changed = false;
    if(!resolve_instantiated_template_argument(
           services, bound_env, carried, resolved, changed)) {
      out.clear();
      return false;
    }
    out.push_back(resolved);
    bind_resolved_class_template_argument_for_later_parameters(
        bound_scope, *parameter, resolved);
  }

  if(!template_arguments_fully_bind_parameters(class_template.parameters, out)) {
    if(resolve_carried_class_template_defaults_from_prefix(
           services, scope, class_template, dependent_arguments, out, out)) {
      return true;
    }
    out.clear();
    return false;
  }
  return true;
}

bool carried_dependent_class_arguments_have_type_semantics(
    const ClassTemplateDecl & class_template,
    const vector<DependentAliasTemplateArgumentSyntax> & dependent_arguments)
{
  if(dependent_arguments.empty()) {
    return false;
  }

  size_t pack_index = class_template.parameters.size();
  for(size_t i = 0; i < class_template.parameters.size(); ++i) {
    if(class_template.parameters[i].parameter_pack) {
      pack_index = i;
      break;
    }
  }

  for(size_t i = 0; i < dependent_arguments.size(); ++i) {
    const TemplateParameterInfo * parameter = nullptr;
    if(pack_index != class_template.parameters.size() &&
       i >= pack_index) {
      parameter = &class_template.parameters[pack_index];
    } else if(i < class_template.parameters.size()) {
      parameter = &class_template.parameters[i];
    }
    if(!parameter ||
       parameter->kind != TemplateParameterInfo::TP_TYPE ||
       !dependent_arguments[i].type) {
      return false;
    }
  }
  return true;
}

bool try_resolve_dependent_class_instantiation_from_carried_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  void * class_template_decl = nullptr;
  vector<DependentAliasTemplateArgumentSyntax> dependent_arguments;
  if(!named_type_dependent_class_template(type,
                                          class_template_decl,
                                          dependent_arguments)) {
    return false;
  }
  ClassTemplateDecl * class_template =
      static_cast<ClassTemplateDecl *>(class_template_decl);
  if(!class_template) {
    return false;
  }

  vector<string> arg_texts;
  vector<TemplateArgumentSyntax> arg_syntaxes;
  arg_texts.reserve(dependent_arguments.size());
  arg_syntaxes.reserve(dependent_arguments.size());
  for(size_t i = 0; i < dependent_arguments.size(); ++i) {
    arg_texts.push_back(dependent_arguments[i].text);
    arg_syntaxes.push_back(dependent_arguments[i].syntax);
  }

  Scope & raw_scope = scope.require();
  vector<TemplateArgument> resolved_arguments;
  if(!try_resolve_carried_dependent_class_type_arguments(services,
                                                         scope,
                                                         *class_template,
                                                         dependent_arguments,
                                                         resolved_arguments)) {
    if(carried_dependent_class_arguments_have_type_semantics(
           *class_template,
           dependent_arguments)) {
      return false;
    }
    if(scope_has_dependent_instantiation_owner(raw_scope)) {
      return false;
    }
    if(!template_api::resolve_template_arguments(
           services,
           scope,
           class_template->parameters,
           arg_texts,
           arg_syntaxes.empty() ? nullptr : &arg_syntaxes,
           resolved_arguments,
           class_template->declaring_scope ?
               template_api::make_template_environment(*class_template->declaring_scope) :
               template_api::TemplateEnvironmentHandle())) {
      return false;
    }
  }

  if(!template_arguments_fully_bind_parameters(class_template->parameters,
                                               resolved_arguments) ||
     template_arguments_are_dependent(
         resolved_arguments,
         [&services](const TypePtr & candidate)
         {
           return service_type_depends_on_template_parameter(services, candidate);
         })) {
    return false;
  }

  template_api::TemplateTypeLookupRequest lookup;
  lookup.scope = &raw_scope;
  lookup.allow_class_templates = true;
  lookup.name.name = class_template->name;
  lookup.source_use_mode =
      template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly;

  template_api::TemplateSelectedClassTemplateIdRequest request;
  request.lookup = lookup;
  request.class_template = class_template;
  request.resolved_arguments.swap(resolved_arguments);
  request.source_arg_texts = arg_texts;
  request.source_arg_syntaxes = arg_syntaxes;
  if(!service_resolve_selected_class_template_id(services, request, out) ||
     !out ||
     service_type_depends_on_template_parameter(services, out)) {
    out.reset();
    return false;
  }
  return true;
}

DependentNamedTypeResolutionStatus resolve_dependent_builtin_type_transform(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  if(!type || !named_type_is_dependent_type(type)) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }

  static const char prefix[] = "$builtin-type-transform:";
  const string payload = named_type_semantic_payload(type);
  if(payload.compare(0, sizeof(prefix) - 1, prefix) != 0) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }

  const size_t name_begin = sizeof(prefix) - 1;
  const size_t name_end = payload.find('|', name_begin);
  const string builtin_name =
      payload.substr(name_begin,
                     name_end == string::npos ? string::npos :
                                                name_end - name_begin);
  if(builtin_name != "__remove_reference" &&
     builtin_name != "__remove_reference_t" &&
     builtin_name != "__remove_cv" &&
     builtin_name != "__remove_const" &&
     builtin_name != "__remove_volatile" &&
     builtin_name != "__remove_cvref" &&
     builtin_name != "__decay") {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  TypePtr arg_type = type->inner;
  if(!arg_type) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }
  if(service_type_depends_on_template_parameter(services, arg_type)) {
    TypePtr resolved_arg;
    if(resolve_instantiated_dependent_type(services, scope, arg_type, resolved_arg) &&
       resolved_arg) {
      arg_type = resolved_arg;
    }
  }
  if(service_type_depends_on_template_parameter(services, arg_type)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  if(builtin_name == "__decay") {
    TypePtr decayed = remove_reference_type(arg_type);
    TypePtr decayed_base = strip_top_level_cv(decayed);
    if(!decayed_base) {
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }
    if(decayed_base->kind == Type::TK_ARRAY) {
      out = make_pointer(decayed_base->inner);
    } else if(decayed_base->kind == Type::TK_FUNCTION) {
      out = make_pointer(decayed_base);
    } else {
      out = decayed_base;
    }
  } else if(builtin_name == "__remove_cv") {
    out = strip_top_level_cv(arg_type);
  } else if(builtin_name == "__remove_const") {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, false, arg_type->cv_volatile) :
        arg_type;
  } else if(builtin_name == "__remove_volatile") {
    out = arg_type->kind == Type::TK_CV ?
        make_cv(arg_type->inner, arg_type->cv_const, false) :
        arg_type;
  } else if(builtin_name == "__remove_cvref") {
    out = strip_top_level_cv(remove_reference_type(arg_type));
  } else {
    out = remove_reference_type(arg_type);
  }
  return out ? DependentNamedTypeResolutionStatus::Resolved :
               DependentNamedTypeResolutionStatus::KeepDependent;
}

bool mentions_local_dependent_type_placeholder(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const string & text)
{
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(const auto & name : current->template_bound_type_names) {
      map<string, TypePtr>::const_iterator found = current->named_types.find(name);
      if(found == current->named_types.end() ||
         !found->second ||
         !type_is_dependent(found->second) ||
         !callsemantic_internal::contains_identifier_token(text, name)) {
        continue;
      }
      return true;
    }
    for(const auto & pack_name : current->template_bound_type_pack_names) {
      map<string, vector<TypePtr> >::const_iterator found =
          current->named_type_packs.find(pack_name);
      if(found == current->named_type_packs.end() ||
         !callsemantic_internal::contains_identifier_token(text, pack_name)) {
        continue;
      }
      for(size_t i = 0; i < found->second.size(); ++i) {
        if(found->second[i] &&
           type_is_dependent(found->second[i])) {
          return true;
        }
      }
    }
    for(const auto & name : current->template_bound_value_names) {
      if(!callsemantic_internal::contains_identifier_token(text, name)) {
        continue;
      }
      map<string, ValueBinding>::const_iterator found =
          current->values.find(name);
      if(found == current->values.end() ||
         value_binding_depends_on_template_parameters(services, found->second)) {
        return true;
      }
    }
  }
  return false;
}

TypePtr try_resolve_direct_concrete_qualified_member_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  Scope & raw_scope = scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & candidate) -> bool
  {
    return service_type_depends_on_template_parameter(services, candidate);
  };
  const string normalized_text = normalize_type_lookup_name(text);
  QualifiedName qualified;
  Scope * qualifier_scope = nullptr;
  string member_name;
  if(semantic_utils::split_qualified_name_text(normalized_text, qualified) &&
     (qualified.rooted || !qualified.qualifiers.empty())) {
    if(normalized_text.find('<') != string::npos &&
       (text_mentions_template_placeholders(services, scope, normalized_text) ||
        text_mentions_dependent_non_namespace_binding_names(services, scope, normalized_text) ||
        should_defer_unresolved_type_lookup(services, raw_scope, normalized_text))) {
      return TypePtr();
    }
    qualifier_scope =
        resolve_qualified_scope_for_class_or_namespace_impl(services, raw_scope, qualified, false);
    member_name = normalize_type_lookup_name(trim_space(qualified.name));
  } else {
    string candidate = normalized_text;
    string stripped_typename_text;
    if(strip_leading_typename_text(candidate, stripped_typename_text)) {
      candidate = stripped_typename_text;
    }

    const size_t split = semantic_utils::top_level_scope_split(candidate);
    if(split == string::npos) {
      return TypePtr();
    }

    const string owner_text = trim_space(candidate.substr(0, split));
    member_name = normalize_type_lookup_name(trim_space(candidate.substr(split + 2)));
    if(owner_text.empty()) {
      return TypePtr();
    }

    TypePtr qualifier_type;
    if(owner_text.find('<') != string::npos) {
      return TypePtr();
    }
    QualifiedName owner_qualified;
    if(semantic_utils::split_qualified_name_text(owner_text, owner_qualified)) {
      resolve_direct_type_name_lookup(services,
                                      raw_scope,
                                      owner_qualified,
                                      false,
                                      string(),
                                      qualifier_type);
    } else {
      resolve_direct_type_name_lookup(services,
                                      raw_scope,
                                      owner_text,
                                      false,
                                      string(),
                                      qualifier_type);
    }
    if(!qualifier_type) {
      return TypePtr();
    }

    if(type_is_dependent(qualifier_type)) {
      TypePtr resolved_qualifier_type;
      if(resolve_instantiated_dependent_type(
             services, scope, qualifier_type, resolved_qualifier_type) &&
         resolved_qualifier_type) {
        qualifier_type = resolved_qualifier_type;
      }
    }
    if(!qualifier_type ||
       type_is_dependent(qualifier_type)) {
      return TypePtr();
    }

    if(!prepare_concrete_type_member_scope(services,
                                           scope,
                                           qualifier_type,
                                           qualifier_scope)) {
      return TypePtr();
    }
  }

  if(!qualifier_scope) {
    return TypePtr();
  }
  if(member_name.empty() ||
     member_name.find('<') != string::npos) {
    return TypePtr();
  }
  TypePtr direct = lookup_concrete_type_in_resolved_scope(services,
                                                          scope,
                                                          *qualifier_scope,
                                                          member_name);
  if(!direct) {
    return TypePtr();
  }
  if(type_is_dependent(direct)) {
    TypePtr resolved_direct;
    if(resolve_instantiated_dependent_type(
           services,
           template_api::make_template_environment(*qualifier_scope),
           direct,
           resolved_direct) &&
       resolved_direct) {
      direct = resolved_direct;
    }
  }
  return direct &&
                 !type_is_dependent(direct) ?
             direct :
             TypePtr();
}

TypePtr resolve_bound_dependent_qualified_owner(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & owner_type)
{
  if(!owner_type ||
     owner_type->kind != Type::TK_NAMED) {
    return TypePtr();
  }

  string owner_text = reparseable_type_argument_text(owner_type);
  if(owner_text.empty()) {
    owner_text =
        owner_type->named_display.empty() ? describe_type(owner_type) :
                                           owner_type->named_display;
  }
  owner_text = normalize_type_lookup_name(trim_space(owner_text));
  string stripped_typename_text;
  if(strip_leading_typename_text(owner_text, stripped_typename_text)) {
    owner_text = normalize_type_lookup_name(trim_space(stripped_typename_text));
  }
  owner_text =
      normalize_type_lookup_name(strip_elaborated_type_prefix(trim_space(owner_text)));
  if(!is_identifier_text(owner_text)) {
    return TypePtr();
  }

  TypePtr bound = lookup_exact_bound_type_name(scope.require(), owner_text);
  if(!bound ||
     service_type_depends_on_template_parameter(services, bound)) {
    template_api::TemplateTypeLookupRequest request;
    request.scope = &scope.require();
    request.name.name = owner_text;
    request.allow_class_templates = true;
    TypePtr direct;
    if(service_resolve_direct_type_lookup(services, request, direct) && direct) {
      if(service_type_depends_on_template_parameter(services, direct)) {
        TypePtr resolved;
        if(resolve_instantiated_dependent_type(services, scope, direct, resolved) &&
           resolved &&
           !service_type_depends_on_template_parameter(services, resolved)) {
          return resolved;
        }
      } else {
        return direct;
      }
    }
    return TypePtr();
  }
  return bound;
}

TypePtr resolve_dependent_qualified_owner_in_current_class_scope(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & owner_type)
{
  if(!owner_type ||
     !scope.valid()) {
    return TypePtr();
  }
  Scope & raw_scope = scope.require();
  ClassInfo * current_info = raw_scope.class_info;
  if(!current_info ||
     !current_info->member_scope) {
    return TypePtr();
  }

  Scope & member_scope = *current_info->member_scope;
  if(&member_scope == &raw_scope) {
    return TypePtr();
  }

  TypePtr resolved;
  template_api::TemplateEnvironmentHandle member_env =
      template_api::make_template_environment(member_scope);
  if(try_resolve_dependent_class_instantiation_from_carried_syntax(
         services,
         member_env,
         owner_type,
         resolved) &&
     resolved &&
     !service_type_depends_on_template_parameter(services, resolved)) {
    return resolved;
  }
  if(try_resolve_dependent_class_instantiation_from_metadata(
         services,
         member_env,
         owner_type,
         resolved) &&
     resolved &&
     !service_type_depends_on_template_parameter(services, resolved)) {
    return resolved;
  }

  return TypePtr();
}

DependentNamedTypeResolutionStatus resolve_structured_dependent_qualified_member_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  const auto type_is_dependent =
      [&services](const TypePtr & candidate) -> bool
  {
    return service_type_depends_on_template_parameter(services, candidate);
  };
  TypePtr owner_type;
  vector<string> member_path;
  vector<TemplateIdSyntax> member_template_ids;
  bool leading_typename = false;
  if(!named_type_dependent_qualified_member(type,
                                            owner_type,
                                            member_path,
                                            leading_typename,
                                            &member_template_ids)) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }
  (void)leading_typename;

  if(type_is_dependent(owner_type)) {
    TypePtr current_owner =
        current_specialization_type_for_dependent_owner(services,
                                                        scope.require(),
                                                        owner_type);
    if(current_owner) {
      owner_type = current_owner;
    }
  }
  if(type_is_dependent(owner_type)) {
    TypePtr resolved_owner =
        resolve_bound_dependent_qualified_owner(services, scope, owner_type);
    if(resolved_owner) {
      owner_type = resolved_owner;
    }
  }
  if(type_is_dependent(owner_type)) {
    TypePtr resolved_owner;
    if(resolve_instantiated_dependent_type(services, scope, owner_type, resolved_owner) &&
       resolved_owner) {
      owner_type = resolved_owner;
    }
  }
  if(type_is_dependent(owner_type)) {
    TypePtr resolved_owner =
        resolve_dependent_qualified_owner_in_current_class_scope(services,
                                                                 scope,
                                                                 owner_type);
    if(resolved_owner) {
      owner_type = resolved_owner;
    }
  }
  if(!owner_type ||
     type_is_dependent(owner_type)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  Scope * current = nullptr;
  if(!prepare_concrete_type_member_scope(services, scope, owner_type, current) ||
     !current) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  for(size_t i = 0; i < member_path.size(); ++i) {
    const string member_name = trim_space(member_path[i]);
    if(member_name.empty()) {
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }

    if(i + 1 != member_path.size()) {
      if(Scope * direct_namespace =
             template_api::resolve_direct_namespace_in_inline_namespaces(
                 *current, member_name)) {
        current = direct_namespace;
        continue;
      }
    }

    TypePtr member_type;
    if(i < member_template_ids.size() &&
       !member_template_ids[i].name.name.empty()) {
      TemplateIdSyntax member_template_id = member_template_ids[i];
      member_template_id.name.rooted = false;
      member_template_id.name.qualifiers.clear();
      if(member_template_id.name.name.empty()) {
        member_template_id.name.name =
            semantic_utils::strip_trailing_top_level_template_arguments(member_name);
      }
      resolve_template_id_syntax_type(services,
                                      *current,
                                      member_template_id,
                                      true,
                                      string(),
                                      member_type,
                                      scope,
                                      template_api::ClassTemplateSourceUseMode::
                                          NestedArgumentsOnly,
                                      false);
    } else {
      if(member_name.find('<') != string::npos) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      member_type = lookup_concrete_type_in_resolved_scope(services,
                                                           scope,
                                                           *current,
                                                           member_name);
    }
    if(!member_type) {
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }

    TypePtr resolved_member;
    if(resolve_instantiated_dependent_type(services, scope, member_type, resolved_member) &&
       resolved_member) {
      member_type = resolved_member;
    }

    if(i + 1 == member_path.size()) {
      if(!member_type ||
         type_is_dependent(member_type)) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      out = member_type;
      return DependentNamedTypeResolutionStatus::Resolved;
    }

    if(!prepare_concrete_type_member_scope(services, scope, member_type, current) ||
       !current) {
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }
  }

  return DependentNamedTypeResolutionStatus::KeepDependent;
}

DependentNamedTypeResolutionStatus resolve_named_member_owner_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  out.reset();
  const TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     !base->named_member_owner_type ||
     base->named_member_name.empty()) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }

  const auto type_is_dependent =
      [&services](const TypePtr & candidate) -> bool
  {
    return service_type_depends_on_template_parameter(services, candidate);
  };

  TypePtr owner_type = base->named_member_owner_type;
  if(type_is_dependent(owner_type)) {
    TypePtr resolved_owner =
        current_specialization_type_for_dependent_owner(services, scope.require(), owner_type);
    if(!resolved_owner) {
      resolve_instantiated_dependent_type(services, scope, owner_type, resolved_owner);
    }
    if(resolved_owner) {
      owner_type = resolved_owner;
    }
  }
  if(!owner_type || type_is_dependent(owner_type)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  Scope * member_scope = nullptr;
  if(!prepare_concrete_type_member_scope(services, scope, owner_type, member_scope) ||
     !member_scope) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  TypePtr member_type =
      lookup_concrete_type_in_resolved_scope(services,
                                             scope,
                                             *member_scope,
                                             base->named_member_name);
  if(!member_type) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }
  if(member_type.get() == base.get()) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  TypePtr resolved_member;
  if(resolve_instantiated_dependent_type(
         services,
         template_api::make_template_environment(*member_scope),
         member_type,
         resolved_member) &&
     resolved_member) {
    member_type = resolved_member;
  }
  if(type_is_dependent(member_type)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  out = member_type;
  return DependentNamedTypeResolutionStatus::Resolved;
}

DependentNamedTypeResolutionStatus resolve_dependent_named_type_locally(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    TypePtr & out)
{
  Scope & raw_scope = scope.require();
  out.reset();
  if(!type || type->kind != Type::TK_NAMED) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }
  const auto type_is_dependent =
      [&services](const TypePtr & candidate) -> bool
  {
    return service_type_depends_on_template_parameter(services, candidate);
  };

  const string & key = type->named_key;
  const string text = [&]() -> string
  {
    const string reparsed = reparseable_type_argument_text(type);
    if(!reparsed.empty()) {
      return reparsed;
    }
    return type->named_display.empty() ? describe_type(type) : type->named_display;
  }();
  const bool syntactically_dependent =
      named_type_has_dependent_semantic(type) ||
      named_type_key_contains_dependent_semantic(type);
  const bool semantically_dependent =
      syntactically_dependent ||
      type_is_dependent(type);
  if(!semantically_dependent) {
    return DependentNamedTypeResolutionStatus::Fallback;
  }
  DependentNamedTypeResolutionGuard recursion_guard(
      &raw_scope,
      key + "\n" + text);
  if(!recursion_guard.active) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }
  if(!syntactically_dependent &&
     named_type_has_unstable_local_spelling(services, type)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  const bool keep_local_type_placeholders_dependent =
      mentions_local_dependent_type_placeholder(services, raw_scope, text);
  const string normalized_text = normalize_type_lookup_name(text);
  {
    string cv_core_name;
    bool top_const = false;
    bool top_volatile = false;
    if(callsemantic_internal::strip_top_level_cv_text(normalized_text,
                                                      cv_core_name,
                                                      top_const,
                                                      top_volatile)) {
      string core_name = normalize_type_lookup_name(trim_space(cv_core_name));
      string stripped_typename_text;
      if(strip_leading_typename_text(core_name, stripped_typename_text)) {
        core_name = normalize_type_lookup_name(trim_space(stripped_typename_text));
      }
      core_name = normalize_type_lookup_name(
          strip_elaborated_type_prefix(trim_space(core_name)));
      TypePtr core_type;
      Scope * exact_bound_scope = nullptr;
      const bool have_exact_bound =
          lookup_exact_bound_type_name_with_scope(
              raw_scope, core_name, exact_bound_scope, core_type);
      if(!have_exact_bound) {
        core_type = lookup_exact_bound_type_name(raw_scope, core_name);
      }
      if(!core_type && is_identifier_text(core_name)) {
        template_api::TemplateTypeLookupRequest request;
        request.scope = &raw_scope;
        request.name.name = core_name;
        request.allow_class_templates = true;
        service_resolve_direct_type_lookup(services, request, core_type);
      }
      if(core_type) {
        TypePtr resolved_core;
        if(resolve_instantiated_dependent_type(services, scope, core_type, resolved_core) &&
           resolved_core) {
          core_type = resolved_core;
        }
        if(type_is_dependent(core_type)) {
          return DependentNamedTypeResolutionStatus::KeepDependent;
        }
        out = apply_cv(core_type, top_const, top_volatile);
        return DependentNamedTypeResolutionStatus::Resolved;
      }
    }
  }
  switch(resolve_structured_dependent_qualified_member_type(services, scope, type, out)) {
  case DependentNamedTypeResolutionStatus::Resolved:
    return DependentNamedTypeResolutionStatus::Resolved;
  case DependentNamedTypeResolutionStatus::KeepDependent:
    return DependentNamedTypeResolutionStatus::KeepDependent;
  case DependentNamedTypeResolutionStatus::Fallback:
    break;
  }
  switch(resolve_named_member_owner_type(services, scope, type, out)) {
  case DependentNamedTypeResolutionStatus::Resolved:
    return DependentNamedTypeResolutionStatus::Resolved;
  case DependentNamedTypeResolutionStatus::KeepDependent:
    return DependentNamedTypeResolutionStatus::KeepDependent;
  case DependentNamedTypeResolutionStatus::Fallback:
    break;
  }
  if(named_type_is_dependent_decltype(type) ||
     named_type_is_dependent_typeof(type)) {
    const CppAstNode * expr_node = named_type_dependent_type_expression_node(type);
    if(expr_node) {
      TypePtr resolved_expr_type;
      if(parse_decltype_or_typeof_node(
             services,
             raw_scope,
             *expr_node,
             resolved_expr_type) &&
         resolved_expr_type &&
         !type_is_dependent(resolved_expr_type)) {
        out = resolved_expr_type;
        return DependentNamedTypeResolutionStatus::Resolved;
      }
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }
    const string payload = trim_space(named_type_semantic_payload(type));
    if(!payload.empty()) {
      TypePtr resolved_expr_type;
      if(parse_decltype_or_typeof_text(services, raw_scope, payload, resolved_expr_type) &&
         resolved_expr_type &&
         !type_is_dependent(resolved_expr_type)) {
        out = resolved_expr_type;
        return DependentNamedTypeResolutionStatus::Resolved;
      }
    }
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }
  switch(resolve_dependent_builtin_type_transform(services, scope, type, out)) {
  case DependentNamedTypeResolutionStatus::Resolved:
    return DependentNamedTypeResolutionStatus::Resolved;
  case DependentNamedTypeResolutionStatus::KeepDependent:
    return DependentNamedTypeResolutionStatus::KeepDependent;
  case DependentNamedTypeResolutionStatus::Fallback:
    break;
  }
  if(named_type_is_dependent_alias(type)) {
    void * alias_template_decl = nullptr;
    vector<DependentAliasTemplateArgumentSyntax> dependent_alias_arguments;
    if(named_type_dependent_alias_template(type,
                                           alias_template_decl,
                                           dependent_alias_arguments)) {
      AliasTemplateDecl * alias_template =
          static_cast<AliasTemplateDecl *>(alias_template_decl);
      if(alias_template) {
        vector<string> alias_arg_texts;
        vector<TemplateArgumentSyntax> alias_arg_syntaxes;
        alias_arg_texts.reserve(dependent_alias_arguments.size());
        alias_arg_syntaxes.reserve(dependent_alias_arguments.size());
        for(size_t i = 0; i < dependent_alias_arguments.size(); ++i) {
          alias_arg_texts.push_back(dependent_alias_arguments[i].text);
          alias_arg_syntaxes.push_back(dependent_alias_arguments[i].syntax);
        }

        QualifiedName alias_template_id;
        vector<string> ignored_arg_texts;
        if(!semantic_utils::split_top_level_template_id_text(
               named_type_semantic_payload(type),
               alias_template_id,
               ignored_arg_texts)) {
          alias_template_id.name = alias_template->name;
        }
        template_api::TemplateTypeLookupRequest alias_request;
        alias_request.scope = &raw_scope;
        alias_request.name = alias_template_id;

        TypePtr alias_resolved;
        if(try_resolve_alias_template_id_locally(
               services,
               scope,
               alias_request,
               alias_template_id,
               alias_arg_texts,
               alias_arg_syntaxes.empty() ? nullptr : &alias_arg_syntaxes,
               scope,
               alias_resolved) &&
           alias_resolved) {
          if(!type_is_dependent(alias_resolved)) {
            out = alias_resolved;
            return DependentNamedTypeResolutionStatus::Resolved;
          }
          return DependentNamedTypeResolutionStatus::KeepDependent;
        }
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
    }

    const string alias_lookup_text = trim_space(named_type_semantic_payload(type));
    bool owned_by_current_class = false;
    if(raw_scope.class_info) {
      const size_t owner_split = semantic_utils::top_level_scope_split(alias_lookup_text);
      if(owner_split != string::npos) {
        const string owner_text =
            callsemantic_internal::normalize_qualified_name_spacing(
                normalize_type_lookup_name(trim_space(alias_lookup_text.substr(0, owner_split))));
        const string current_qualified =
            callsemantic_internal::normalize_qualified_name_spacing(
                raw_scope.class_info->qualified_name);
        const string current_display =
            callsemantic_internal::normalize_qualified_name_spacing(
                raw_scope.class_info->display_qualified_name);
        owned_by_current_class =
            owner_text == current_qualified ||
            (!current_display.empty() && owner_text == current_display);
      }
    }
    if(!owned_by_current_class) {
      const string concrete_alias_lookup_text = alias_lookup_text;
      if(text_mentions_template_placeholders(services, scope, concrete_alias_lookup_text) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, scope, concrete_alias_lookup_text) ||
         should_defer_unresolved_type_lookup(services,
                                             raw_scope,
                                             concrete_alias_lookup_text)) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      if(concrete_alias_lookup_text.find('<') != string::npos) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      if(TypePtr direct_member =
             try_resolve_direct_concrete_qualified_member_type(
                 services, scope, concrete_alias_lookup_text)) {
        out = direct_member;
        return DependentNamedTypeResolutionStatus::Resolved;
      }

      QualifiedName alias_template_id;
      vector<string> alias_arg_texts;
      if(semantic_utils::split_top_level_template_id_text(
             concrete_alias_lookup_text,
             alias_template_id,
             alias_arg_texts)) {
        template_api::TemplateTypeLookupRequest alias_request;
        alias_request.scope = &raw_scope;
        alias_request.name = alias_template_id;
        TypePtr alias_resolved;
        if(try_resolve_alias_template_id_locally(
               services,
               scope,
               alias_request,
               alias_template_id,
               alias_arg_texts,
               nullptr,
               scope,
               alias_resolved) &&
           alias_resolved &&
           !type_is_dependent(alias_resolved)) {
          out = alias_resolved;
          return DependentNamedTypeResolutionStatus::Resolved;
        }
      }
    }
  }
  if(is_identifier_text(normalized_text)) {
    Scope * exact_bound_scope = nullptr;
    TypePtr exact_bound;
    const bool have_exact_bound =
        lookup_exact_bound_type_name_with_scope(
            raw_scope, normalized_text, exact_bound_scope, exact_bound);
    if(!have_exact_bound) {
      exact_bound = lookup_exact_bound_type_name(raw_scope, normalized_text);
    }
    if(exact_bound) {
      if(have_exact_bound &&
         exact_bound_scope == &raw_scope &&
         type_is_dependent(exact_bound)) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      TypePtr resolved_exact;
      if(resolve_instantiated_dependent_type(services, scope, exact_bound, resolved_exact) &&
         resolved_exact &&
         !type_is_dependent(resolved_exact)) {
        out = resolved_exact;
        return DependentNamedTypeResolutionStatus::Resolved;
      }
      if(type_is_dependent(exact_bound)) {
        return DependentNamedTypeResolutionStatus::KeepDependent;
      }
      out = exact_bound;
      return DependentNamedTypeResolutionStatus::Resolved;
    }

    template_api::TemplateTypeLookupRequest request;
    request.scope = &raw_scope;
    request.name.name = normalized_text;
    TypePtr direct;
    if(service_resolve_direct_type_lookup(services, request, direct) &&
       direct &&
       direct.get() != type.get()) {
      TypePtr resolved_direct;
      if(resolve_instantiated_dependent_type(services, scope, direct, resolved_direct) &&
         resolved_direct) {
        direct = resolved_direct;
      }
      if(!type_is_dependent(direct)) {
        out = direct;
        return DependentNamedTypeResolutionStatus::Resolved;
      }
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }
  }

  if(keep_local_type_placeholders_dependent) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  TypePtr resolved_instantiation;
  if(try_resolve_dependent_class_instantiation_from_carried_syntax(
         services, scope, type, resolved_instantiation)) {
    out = resolved_instantiation;
    return DependentNamedTypeResolutionStatus::Resolved;
  }
  if(try_resolve_dependent_class_instantiation_from_metadata(
         services, scope, type, resolved_instantiation)) {
    out = resolved_instantiation;
    return DependentNamedTypeResolutionStatus::Resolved;
  }

  if(normalized_text.find('<') != string::npos) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }

  if(TypePtr direct_member =
         try_resolve_direct_concrete_qualified_member_type(services, scope, normalized_text)) {
    out = direct_member;
    return DependentNamedTypeResolutionStatus::Resolved;
  }

  if(!syntactically_dependent) {
    template_api::TemplateNamedTypeMetadata info;
    if(service_describe_named_type_metadata(services, type, info) &&
       info.source_template &&
       !info.dependent_instantiation &&
       !info.instantiation_arguments.empty() &&
       !template_arguments_are_dependent(
           info.instantiation_arguments,
           [&type_is_dependent](const TypePtr & candidate)
           {
             return type_is_dependent(candidate);
           })) {
      return DependentNamedTypeResolutionStatus::KeepDependent;
    }
  }

  if(!syntactically_dependent &&
     !text_mentions_non_namespace_binding_names(scope, text)) {
    return DependentNamedTypeResolutionStatus::KeepDependent;
  }
  return DependentNamedTypeResolutionStatus::KeepDependent;
}

bool resolve_instantiated_dependent_type(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const TypePtr & type,
                                         TypePtr & out)
{
  out.reset();
  if(!type) {
    return false;
  }
  static thread_local std::set<const Type *> resolving_types;
  const Type * resolving_key = type.get();
  if(!resolving_types.insert(resolving_key).second) {
    return false;
  }
  struct ResolvingTypeGuard {
    std::set<const Type *> & resolving_types;
    const Type * resolving_key;
    ~ResolvingTypeGuard()
    {
      resolving_types.erase(resolving_key);
    }
  } resolving_type_guard = { resolving_types, resolving_key };

  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;

  case Type::TK_NAMED:
  {
    switch(resolve_dependent_named_type_locally(services, scope, type, out)) {
    case DependentNamedTypeResolutionStatus::Resolved:
      return true;
    case DependentNamedTypeResolutionStatus::KeepDependent:
      return false;
    case DependentNamedTypeResolutionStatus::Fallback:
      return false;
    }
    return false;
  }

  case Type::TK_CV:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = apply_cv(inner, type->cv_const, type->cv_volatile);
    return true;
  }

  case Type::TK_ATOMIC:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = make_atomic(inner);
    return true;
  }

  case Type::TK_POINTER:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = make_pointer(inner);
    return true;
  }

  case Type::TK_MEMBER_POINTER:
  {
    bool changed = false;
    TypePtr owner = type->owner;
    TypePtr inner = type->inner;
    TypePtr resolved_owner;
    TypePtr resolved_inner;
    if(resolve_instantiated_dependent_type(services, scope, type->owner, resolved_owner)) {
      owner = resolved_owner;
      changed = true;
    }
    if(resolve_instantiated_dependent_type(services, scope, type->inner, resolved_inner)) {
      inner = resolved_inner;
      changed = true;
    }
    if(!changed) {
      return false;
    }
    out = make_member_pointer(owner, inner);
    return true;
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = make_block_pointer(inner);
    return true;
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = collapse_lvalue_reference_type(inner);
    return true;
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = collapse_rvalue_reference_type(inner);
    return true;
  }

  case Type::TK_ARRAY:
  {
    TypePtr inner;
    if(!resolve_instantiated_dependent_type(services, scope, type->inner, inner)) {
      return false;
    }
    out = make_array(inner, type->has_bound, type->bound, type->bound_text);
    return true;
  }

  case Type::TK_FUNCTION:
  {
    bool changed = false;
    TypePtr result_type = type->inner;
    TypePtr resolved_result;
    if(resolve_instantiated_dependent_type(services, scope, type->inner, resolved_result)) {
      result_type = resolved_result;
      changed = true;
    }
    vector<TypePtr> params_out;
    params_out.reserve(type->params.size());
    for(size_t i = 0; i < type->params.size(); ++i) {
      TypePtr param = type->params[i];
      TypePtr resolved_param;
      if(resolve_instantiated_dependent_type(services, scope, type->params[i], resolved_param)) {
        param = resolved_param;
        changed = true;
      }
      params_out.push_back(param);
    }
    if(!changed) {
      return false;
    }
    out = make_function(result_type,
                        params_out,
                        type->variadic,
                        type->function_const,
                        type->function_volatile,
                        type->prototype_relaxed);
    return true;
  }
  }

  return false;
}

bool resolve_instantiated_dependent_type_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    TypePtr & type)
{
  if(!type ||
     !service_type_depends_on_template_parameter(services, type)) {
    return false;
  }

  TypePtr resolved;
  if(!resolve_instantiated_dependent_type(services, scope, type, resolved) ||
     !resolved) {
    return false;
  }
  type = resolved;
  return true;
}

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         Scope & scope,
                                         const TypePtr & type,
                                         TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return resolve_instantiated_dependent_type(
            services, template_api::make_template_environment(scope), type, out);
      });
}

string lookup_text_for_non_type_template_argument(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    long long value)
{
  TemplateArgument arg;
  arg.kind = TemplateArgument::TA_VALUE;
  arg.type = type;
  arg.value = value;
  return template_model::template_argument_text(
      arg,
      [&](const TypePtr & current_type)
      {
        return lookup_text_for_type_argument(type_system, current_type);
      });
}

string normalize_template_template_argument_lookup_text(const string & text)
{
  string out = trim_space(text);
  static const string disambiguator = "::template";
  size_t pos = out.find(disambiguator);
  while(pos != string::npos) {
    const size_t erase_begin = pos + 2;
    size_t erase_end = erase_begin + disambiguator.size() - 2;
    while(erase_end < out.size() &&
          std::isspace(static_cast<unsigned char>(out[erase_end]))) {
      ++erase_end;
    }
    out.erase(erase_begin, erase_end - erase_begin);
    pos = out.find(disambiguator, pos + 2);
  }
  return trim_space(out);
}

bool resolve_member_template_template_argument_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text,
    size_t expected_parameter_count,
    TemplateArgument & out)
{
  const string normalized = normalize_template_template_argument_lookup_text(text);
  const size_t split = semantic_utils::top_level_scope_split(normalized);
  if(split == string::npos) {
    return false;
  }

  const string owner_text = trim_space(normalized.substr(0, split));
  string member_name = trim_space(normalized.substr(split + 2));
  static const string template_prefix = "template ";
  if(member_name.compare(0, template_prefix.size(), template_prefix) == 0) {
    member_name = trim_space(member_name.substr(template_prefix.size()));
  }
  if(owner_text.empty() ||
     member_name.empty() ||
     member_name.find('<') != string::npos ||
     semantic_utils::top_level_scope_split(member_name) != string::npos) {
    return false;
  }

  TypePtr owner_type;
  if(!parse_type_argument_text(services, scope, owner_text, owner_type) ||
     !owner_type) {
    QualifiedName owner_template_name;
    vector<string> owner_args;
    if(semantic_utils::split_top_level_template_id_text(owner_text,
                                                        owner_template_name,
                                                        owner_args)) {
      TemplateIdSyntax owner_syntax;
      owner_syntax.name = owner_template_name;
      owner_syntax.arguments = owner_args;
      owner_syntax.argument_syntaxes.reserve(owner_args.size());
      for(size_t i = 0; i < owner_args.size(); ++i) {
        TemplateArgumentSyntax syntax;
        syntax.text = owner_args[i];
        owner_syntax.argument_syntaxes.push_back(syntax);
      }
      resolve_template_id_syntax_type(
          services,
          scope.require(),
          owner_syntax,
          true,
          string(),
          owner_type,
          scope,
          template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly);
    }
    if(!owner_type) {
      return false;
    }
  }
  if(service_type_depends_on_template_parameter(services, owner_type)) {
    TypePtr resolved_owner;
    if(resolve_instantiated_dependent_type(services,
                                           scope,
                                           owner_type,
                                           resolved_owner) &&
       resolved_owner) {
      owner_type = resolved_owner;
    }
  }
  if(!owner_type ||
     service_type_depends_on_template_parameter(services, owner_type)) {
    return false;
  }

  Scope * member_scope = nullptr;
  if(!prepare_concrete_type_member_scope(services, scope, owner_type, member_scope) ||
     !member_scope) {
    return false;
  }

  ClassInfo * owner_info =
      template_api::find_named_type_class_info(service_type_system(services).model,
                                               owner_type);
  AliasTemplateDecl * alias_template = nullptr;
  if(owner_info && services.semantic_context) {
    semantic_lookup::MemberAliasTemplateLookupResult member =
        semantic_lookup::lookup_member_alias_template(*services.semantic_context,
                                                      *owner_info,
                                                      member_name);
    alias_template = member.alias_template;
  }
  if(!alias_template) {
    map<string, AliasTemplateDecl *>::iterator found =
        member_scope->alias_templates.find(member_name);
    if(found != member_scope->alias_templates.end()) {
      alias_template = found->second;
    }
  }
  if(alias_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      alias_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    out.template_decl = alias_template;
    out.text = normalized;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trim_space(text)
                << " resolved=member-alias-template canonical=" << out.text;
        });
    return true;
  }

  ClassTemplateDecl * class_template = nullptr;
  if(owner_info && services.semantic_context) {
    semantic_lookup::MemberClassTemplateLookupResult member =
        semantic_lookup::lookup_member_class_template(*services.semantic_context,
                                                      *owner_info,
                                                      member_name);
    class_template = member.class_template;
  }
  if(!class_template) {
    map<string, ClassTemplateDecl *>::iterator found =
        member_scope->class_templates.find(member_name);
    if(found != member_scope->class_templates.end()) {
      class_template = found->second;
    }
  }
  if(class_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      class_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    out.template_decl = class_template;
    out.text = normalized;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trim_space(text)
                << " resolved=member-class-template canonical=" << out.text;
        });
    return true;
  }

  return false;
}

bool resolve_template_template_argument_text(
    SemanticContext & ctx,
    Scope & scope,
    const string & text,
    size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    TemplateArgument & out)
{
  out = TemplateArgument();
  const string trimmed = normalize_template_template_argument_lookup_text(text);
  out.text = trimmed;
  QualifiedName qualified;
  const bool has_structured_qualified_name =
      semantic_utils::split_qualified_name_text(trimmed, qualified) &&
      (qualified.rooted || !qualified.qualifiers.empty());

  AliasTemplateDecl * alias_template = has_structured_qualified_name ?
      ctx.lookup_alias_template(scope, qualified) :
      ctx.lookup_alias_template(scope, trimmed);
  if(alias_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      alias_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    out.template_decl = alias_template;
    out.text = alias_template->declaring_scope ?
        semantic_lookup::scope_qualified_name(*alias_template->declaring_scope,
                                              alias_template->name) :
        alias_template->name;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=alias-template canonical=" << out.text;
        });
    return true;
  }

  ClassTemplateDecl * class_template = has_structured_qualified_name ?
      ctx.lookup_class_template(scope, qualified) :
      ctx.lookup_class_template(scope, trimmed);
  if(class_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      class_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    out.template_decl = class_template;
    out.text = class_template->declaring_scope ?
        semantic_lookup::scope_qualified_name(*class_template->declaring_scope,
                                              class_template->name) :
        class_template->name;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=class-template canonical=" << out.text;
        });
    return true;
  }

  if(!allow_dependent_placeholders) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=none allow_dependent=no";
        });
    return false;
  }

  if(scope_has_template_template_placeholder(scope, trimmed, true)) {
    out.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    out.template_decl = nullptr;
    out.dependent = true;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=dependent-alias-template";
        });
    return true;
  }
  if(scope_has_template_template_placeholder(scope, trimmed, false)) {
    out.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    out.template_decl = nullptr;
    out.dependent = true;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=dependent-class-template";
        });
    return true;
  }
  note_template_trace_if_enabled(
      [&](ostringstream & trace)
      {
        trace << "template-template-arg text=" << trimmed
              << " resolved=none allow_dependent=yes";
      });
  return false;
}

bool resolve_template_template_argument_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text,
    size_t expected_parameter_count,
    bool allow_dependent_placeholders,
    TemplateArgument & out)
{
  Scope & raw_scope = scope.require();
  out = TemplateArgument();
  const string trimmed = normalize_template_template_argument_lookup_text(text);
  out.text = trimmed;

  AliasTemplateDecl * alias_template = lookup_alias_template_impl(services, raw_scope, trimmed);
  if(alias_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      alias_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    out.template_decl = alias_template;
    out.text = alias_template->declaring_scope ?
        semantic_lookup::scope_qualified_name(*alias_template->declaring_scope,
                                              alias_template->name) :
        alias_template->name;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=alias-template canonical=" << out.text;
        });
    return true;
  }

  ClassTemplateDecl * class_template = lookup_class_template_impl(services, raw_scope, trimmed);
  if(class_template &&
     (expected_parameter_count == static_cast<size_t>(-1) ||
      class_template->parameters.size() == expected_parameter_count)) {
    out.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    out.template_decl = class_template;
    out.text = class_template->declaring_scope ?
        semantic_lookup::scope_qualified_name(*class_template->declaring_scope,
                                              class_template->name) :
        class_template->name;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=class-template canonical=" << out.text;
        });
    return true;
  }

  if(resolve_member_template_template_argument_text(services,
                                                   scope,
                                                   trimmed,
                                                   expected_parameter_count,
                                                   out)) {
    return true;
  }

  if(!allow_dependent_placeholders) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=none allow_dependent=no";
        });
    return false;
  }

  if(scope_has_template_template_placeholder(raw_scope, trimmed, true)) {
    out.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
    out.template_decl = nullptr;
    out.dependent = true;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=dependent-alias-template";
        });
    return true;
  }
  if(scope_has_template_template_placeholder(raw_scope, trimmed, false)) {
    out.kind = TemplateArgument::TA_CLASS_TEMPLATE;
    out.template_decl = nullptr;
    out.dependent = true;
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "template-template-arg text=" << trimmed
                << " resolved=dependent-class-template";
        });
    return true;
  }
  note_template_trace_if_enabled(
      [&](ostringstream & trace)
      {
        trace << "template-template-arg text=" << trimmed
              << " resolved=none allow_dependent=yes";
      });
  return false;
}

bool evaluate_structural_builtin_type_trait(const string & name,
                                            const vector<TypePtr> & types,
                                            long long & out)
{
  const auto is_named_enum_type =
      [](const TypePtr & type) -> bool
  {
    TypePtr base = strip_top_level_cv(type);
    return base &&
           base->kind == Type::TK_NAMED &&
           base->named_key.compare(0, 5, "enum ") == 0;
  };
  const auto is_scalar_or_member_pointer_type =
      [&](const TypePtr & type) -> bool
  {
    TypePtr base = strip_top_level_cv(type);
    return base &&
           (base->kind == Type::TK_MEMBER_POINTER ||
            is_integral_type(base) ||
            is_floating_type(base) ||
            is_pointer_type(base) ||
            is_named_enum_type(base) ||
            (base->kind == Type::TK_FUNDAMENTAL &&
             base->fundamental == FT_NULLPTR_T));
  };
  const auto evaluate_unary_array_recursive =
      [&](const string & recursive_name, const TypePtr & base) -> bool
  {
    if(base->kind != Type::TK_ARRAY) {
      return false;
    }
    long long element_value = 0;
    out = (base->inner &&
           evaluate_structural_builtin_type_trait(
               recursive_name, vector<TypePtr>(1, base->inner), element_value) &&
           element_value != 0) ? 1 : 0;
      return true;
  };
  const auto array_rank =
      [](const TypePtr & type) -> long long
  {
    TypePtr current = strip_top_level_cv(type);
    long long rank = 0;
    while(current && current->kind == Type::TK_ARRAY) {
      ++rank;
      current = strip_top_level_cv(current->inner);
    }
    return rank;
  };

  if(types.size() == 2) {
    if(name == "__is_same") {
      out = type_equals(types[0], types[1]) ? 1 : 0;
      return true;
    }
    return false;
  }

  if(types.size() != 1) {
    return false;
  }

  const TypePtr & type = types[0];
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
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
        1 :
        0;
    return true;
  }
  if(name == "__is_unsigned") {
    out = is_unsigned_integral_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_enum") {
    out = is_named_enum_type(base) ? 1 : 0;
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
    out = semantic_conversion::is_const_object_type(type) ? 1 : 0;
    return true;
  }
  if(name == "__is_volatile") {
    TypePtr inner;
    bool cv_const = false;
    bool cv_volatile = false;
    out = semantic_conversion::top_level_cv_flags(type, inner, cv_const, cv_volatile) &&
              cv_volatile ?
              1 :
              0;
    return true;
  }
  if(name == "__is_fundamental") {
    out = base->kind == Type::TK_FUNDAMENTAL ? 1 : 0;
    return true;
  }
  if(name == "__is_scalar") {
    out = is_scalar_or_member_pointer_type(base) ? 1 : 0;
    return true;
  }
  if(name == "__is_compound") {
    out = base->kind != Type::TK_FUNDAMENTAL ? 1 : 0;
    return true;
  }
  if(name == "__is_object") {
    out = (!is_reference_type(base) && base->kind != Type::TK_FUNCTION && !is_void_type(base)) ?
        1 :
        0;
    return true;
  }
  if(name == "__array_rank") {
    out = array_rank(base);
    return true;
  }
  if(name == "__is_literal_type") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    if(is_reference_type(base) ||
       base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(base)) {
      out = 1;
      return true;
    }
    if(base->kind != Type::TK_NAMED) {
      out = 0;
      return true;
    }
    return false;
  }
  if(name == "__is_trivially_copyable") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    out = (base->kind == Type::TK_FUNDAMENTAL ||
           is_scalar_or_member_pointer_type(base)) ? 1 : 0;
    return true;
  }
  if(name == "__is_standard_layout") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    if(base->kind == Type::TK_NAMED &&
       !base->definitely_not_class &&
       !is_named_enum_type(base)) {
      return false;
    }
    out = (base->kind == Type::TK_FUNDAMENTAL ||
           is_scalar_or_member_pointer_type(base)) ? 1 : 0;
    return true;
  }
  if(name == "__is_trivial") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(base)) {
      out = 1;
      return true;
    }
    return false;
  }
  if(name == "__is_trivially_constructible" || name == "__has_trivial_constructor") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(base)) {
      out = 1;
      return true;
    }
    return false;
  }
  if(name == "__is_constructible" || name == "__is_nothrow_constructible") {
    if(is_reference_type(base) || base->kind == Type::TK_FUNCTION ||
       is_void_type(base) || base->kind == Type::TK_ARRAY) {
      out = 0;
      return true;
    }
    if(base->kind == Type::TK_FUNDAMENTAL ||
       is_scalar_or_member_pointer_type(base)) {
      out = 1;
      return true;
    }
    return false;
  }
  if(name == "__is_pod") {
    if(evaluate_unary_array_recursive(name, base)) {
      return true;
    }
    if(base->kind == Type::TK_NAMED &&
       !base->definitely_not_class &&
       !is_named_enum_type(base)) {
      return false;
    }
    long long trivial_value = 0;
    long long standard_layout_value = 0;
    if(evaluate_structural_builtin_type_trait(
           "__is_trivial", vector<TypePtr>(1, base), trivial_value) &&
       evaluate_structural_builtin_type_trait(
           "__is_standard_layout", vector<TypePtr>(1, base), standard_layout_value)) {
      out = (trivial_value != 0 && standard_layout_value != 0) ? 1 : 0;
      return true;
    }
    return false;
  }

  return false;
}

bool is_literal_type(template_api::TemplateTypeSystem & type_system,
                     const TypePtr & type);

bool evaluate_class_info_builtin_type_trait(template_api::TemplateTypeSystem & type_system,
                                            const string & name,
                                            const vector<TypePtr> & types,
                                            long long & out)
{
  const auto class_metadata_for_trait =
      [&](const TypePtr & type, template_api::TemplateNamedTypeMetadata & info) -> bool
      {
        TypePtr base = strip_top_level_cv(type);
        return base &&
               template_api::describe_named_type_metadata(
                   type_system.model, base, info) &&
               info.class_kind != "union" &&
               info.class_kind != "enum";
      };
  const std::function<bool(const TypePtr &, const TypePtr &, std::set<std::string> &)>
      type_is_same_or_derived =
      [&](const TypePtr & derived,
          const TypePtr & base,
          std::set<std::string> & visiting) -> bool
      {
        template_api::TemplateNamedTypeMetadata derived_info;
        template_api::TemplateNamedTypeMetadata base_info;
        if(!class_metadata_for_trait(derived, derived_info) ||
           !class_metadata_for_trait(base, base_info)) {
          return false;
        }
        if(type_equals(strip_top_level_cv(derived), strip_top_level_cv(base))) {
          return true;
        }
        const std::string visit_key =
            derived_info.name + "|" + template_argument_type_text(base);
        if(!visiting.insert(visit_key).second) {
          return false;
        }
        for(size_t i = 0; i < derived_info.direct_base_types.size(); ++i) {
          if(type_is_same_or_derived(derived_info.direct_base_types[i],
                                     base,
                                     visiting)) {
            return true;
          }
        }
        return false;
      };

  if(name == "__is_base_of" && types.size() == 2) {
    std::set<std::string> visiting;
    out = type_is_same_or_derived(types[1], types[0], visiting) ? 1 : 0;
    return true;
  }

  if(types.size() != 1) {
    return false;
  }

  TypePtr base = strip_top_level_cv(types[0]);
  if(!base) {
    return false;
  }

  if(name == "__is_empty") {
    if(base->kind != Type::TK_NAMED) {
      out = 0;
      return true;
    }
    template_api::TemplateNamedTypeMetadata info;
    if(!template_api::describe_named_type_metadata(
           type_system.model, base, info) ||
       !info.complete ||
       !info.type) {
      return false;
    }
    TypePtr info_base = strip_top_level_cv(info.type);
    out = (info_base && info_base->kind == Type::TK_NAMED &&
           info_base->named_is_empty) ? 1 : 0;
    return true;
  }

  template_api::TemplateNamedTypeMetadata info;
  const bool have_info = template_api::describe_named_type_metadata(
      type_system.model, base, info);
  if(name == "__is_final") {
    out = (have_info && info.is_final) ? 1 : 0;
    return true;
  }
  if(name == "__is_abstract") {
    out = (have_info && info.is_abstract) ? 1 : 0;
    return true;
  }
  if(name == "__is_polymorphic") {
    out = (have_info && info.is_polymorphic) ? 1 : 0;
    return true;
  }
  if(name == "__has_virtual_destructor") {
    out = (have_info && info.has_virtual_destructor) ? 1 : 0;
    return true;
  }
  if(name == "__is_literal_type") {
    out = is_literal_type(type_system, base) ? 1 : 0;
    return true;
  }
  if(name == "__is_union") {
    out = (have_info && info.class_kind == "union") ? 1 : 0;
    return true;
  }
  if(name == "__is_class") {
    out = (have_info && info.class_kind != "union") ? 1 : 0;
    return true;
  }

  return false;
}

bool is_destructible_type(template_api::TemplateTypeSystem & type_system,
                          const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_destructible_type(type_system, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T) ||
     (base->kind == Type::TK_NAMED && base->named_key.compare(0, 5, "enum ") == 0)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  template_api::TemplateNamedTypeMetadata info;
  return template_api::describe_named_type_metadata(type_system.model, base, info) &&
         info.complete;
}

bool is_trivially_destructible_type(template_api::TemplateTypeSystem & type_system,
                                    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return is_trivially_destructible_type(type_system, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T) ||
     (base->kind == Type::TK_NAMED && base->named_key.compare(0, 5, "enum ") == 0)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(!template_api::describe_named_type_metadata(type_system.model, base, info) ||
     !info.complete || info.has_user_declared_destructor) {
    return false;
  }
  for(size_t i = 0; i < info.direct_base_types.size(); ++i) {
    if(!is_trivially_destructible_type(type_system, info.direct_base_types[i])) {
      return false;
    }
  }
  for(size_t i = 0; i < info.field_types.size(); ++i) {
    if(!is_trivially_destructible_type(type_system, info.field_types[i])) {
      return false;
    }
  }
  return true;
}

bool is_literal_type(template_api::TemplateTypeSystem & type_system,
                     const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base)) {
    return true;
  }
  if(is_array_type(base)) {
    return base->inner && is_literal_type(type_system, base->inner);
  }
  if(base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    return false;
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T) ||
     (base->kind == Type::TK_NAMED && base->named_key.compare(0, 5, "enum ") == 0)) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(!template_api::describe_named_type_metadata(type_system.model, base, info) ||
     !info.complete ||
     info.is_abstract ||
     !is_trivially_destructible_type(type_system, base)) {
    return false;
  }
  for(size_t i = 0; i < info.direct_base_types.size(); ++i) {
    if(!is_literal_type(type_system, info.direct_base_types[i])) {
      return false;
    }
  }
  for(size_t i = 0; i < info.field_types.size(); ++i) {
    if(!is_literal_type(type_system, info.field_types[i])) {
      return false;
    }
  }
  return true;
}

bool evaluate_destructibility_builtin_type_trait(template_api::TemplateTypeSystem & type_system,
                                                 const string & name,
                                                 const vector<TypePtr> & types,
                                                 long long & out)
{
  if(types.size() != 1) {
    return false;
  }

  if(name == "__is_destructible" || name == "__is_nothrow_destructible") {
    out = is_destructible_type(type_system, types[0]) ? 1 : 0;
    return true;
  }
  if(name == "__is_trivially_destructible" || name == "__has_trivial_destructor") {
    out = is_trivially_destructible_type(type_system, types[0]) ? 1 : 0;
    return true;
  }
  return false;
}

bool try_evaluate_trivially_default_constructible_type(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    bool & out)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  if(is_reference_type(base) || base->kind == Type::TK_FUNCTION || is_void_type(base)) {
    out = false;
    return true;
  }
  if(is_array_type(base)) {
    if(!base->has_bound) {
      out = false;
      return true;
    }
    return try_evaluate_trivially_default_constructible_type(type_system, base->inner, out);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     base->kind == Type::TK_MEMBER_POINTER ||
     is_integral_type(base) ||
     is_floating_type(base) ||
     is_pointer_type(base) ||
     (base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_NULLPTR_T) ||
     (base->kind == Type::TK_NAMED && base->named_key.compare(0, 5, "enum ") == 0)) {
    out = true;
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    out = false;
    return true;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(!template_api::describe_named_type_metadata(type_system.model, base, info)) {
    return false;
  }
  if(info.class_kind == "enum") {
    out = true;
    return true;
  }
  if(!info.complete ||
     info.class_kind == "union" ||
     info.is_polymorphic ||
     info.has_virtual_bases ||
     info.has_default_member_initializers ||
     !info.member_scope) {
    out = false;
    return true;
  }

  const bool implicit_default_constructor =
      !info.has_user_declared_constructor &&
      !info.has_user_declared_constructor_template;
  if(!implicit_default_constructor && !info.has_structural_default_constructor) {
    out = false;
    return true;
  }

  for(size_t i = 0; i < info.direct_base_types.size(); ++i) {
    bool base_value = false;
    if(!try_evaluate_trivially_default_constructible_type(
           type_system, info.direct_base_types[i], base_value)) {
      return false;
    }
    if(!base_value) {
      out = false;
      return true;
    }
  }
  for(size_t i = 0; i < info.field_types.size(); ++i) {
    bool field_value = false;
    if(!try_evaluate_trivially_default_constructible_type(
           type_system, info.field_types[i], field_value)) {
      return false;
    }
    if(!field_value) {
      out = false;
      return true;
    }
  }

  out = true;
  return true;
}

bool evaluate_triviality_builtin_type_trait(template_api::TemplateTypeSystem & type_system,
                                            const string & name,
                                            const vector<TypePtr> & types,
                                            long long & out)
{
  if(types.size() != 1) {
    return false;
  }

  bool value = false;
  if((name == "__is_trivially_constructible" || name == "__has_trivial_constructor") &&
     try_evaluate_trivially_default_constructible_type(type_system, types[0], value)) {
    out = value ? 1 : 0;
    return true;
  }
  if(name == "__is_trivial" &&
     try_evaluate_trivially_default_constructible_type(type_system, types[0], value)) {
    out = (value && is_trivially_destructible_type(type_system, types[0])) ? 1 : 0;
    return true;
  }
  return false;
}

bool is_builtin_conversion_domain_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  return base->kind == Type::TK_FUNDAMENTAL ||
         base->kind == Type::TK_POINTER ||
         base->kind == Type::TK_MEMBER_POINTER ||
         base->kind == Type::TK_ARRAY ||
         base->kind == Type::TK_FUNCTION ||
         (base->kind == Type::TK_NAMED &&
          base->named_key.compare(0, 5, "enum ") == 0);
}

semantic_conversion::ExprInfo make_builtin_trait_expr_info(const TypePtr & source)
{
  semantic_conversion::ExprInfo expr;
  TypePtr source_base = strip_top_level_cv(source);
  if(!source_base) {
    return expr;
  }
  if(source_base->kind == Type::TK_LVALUE_REFERENCE) {
    expr.type = source_base->inner;
    expr.category = semantic_conversion::VC_LVALUE;
  } else if(source_base->kind == Type::TK_RVALUE_REFERENCE) {
    expr.type = source_base->inner;
    expr.category = semantic_conversion::VC_XVALUE;
  } else {
    expr.type = source;
    TypePtr object_base = strip_top_level_cv(remove_reference_type(source));
    expr.category = (object_base && object_base->kind == Type::TK_ARRAY) ?
        semantic_conversion::VC_XVALUE :
        semantic_conversion::VC_PRVALUE;
  }
  return expr;
}

bool evaluate_builtin_conversion_leaf_type_trait(const string & name,
                                                 const vector<TypePtr> & types,
                                                 long long & out)
{
  if(types.size() != 2) {
    return false;
  }

  TypePtr lhs = strip_top_level_cv(types[0]);
  TypePtr rhs = strip_top_level_cv(types[1]);
  if(!lhs || !rhs) {
    return false;
  }

  if(name == "__is_convertible" || name == "__is_nothrow_convertible") {
    if(!is_builtin_conversion_domain_type(lhs) ||
       !is_builtin_conversion_domain_type(rhs)) {
      return false;
    }

    TypePtr source_base = strip_top_level_cv(remove_reference_type(lhs));
    TypePtr target_base = strip_top_level_cv(remove_reference_type(rhs));
    if(!source_base || !target_base) {
      out = 0;
      return true;
    }
    if(is_void_type(target_base)) {
      out = is_void_type(source_base) ? 1 : 0;
      return true;
    }
    if(is_void_type(source_base)) {
      out = 0;
      return true;
    }

    const semantic_conversion::ExprInfo source_expr =
        make_builtin_trait_expr_info(lhs);
    out = semantic_conversion::standard_conversion_rank(rhs, source_expr) !=
                  semantic_conversion::CR_BAD ?
              1 :
              0;
    return true;
  }

  if(name == "__is_constructible" || name == "__is_nothrow_constructible") {
    if(!is_builtin_conversion_domain_type(lhs) ||
       !is_builtin_conversion_domain_type(rhs)) {
      return false;
    }

    TypePtr target_base = strip_top_level_cv(remove_reference_type(lhs));
    if(!target_base) {
      out = 0;
      return true;
    }
    if(target_base->kind == Type::TK_FUNCTION ||
       target_base->kind == Type::TK_ARRAY ||
       is_void_type(target_base)) {
      out = 0;
      return true;
    }

    const semantic_conversion::ExprInfo rhs_expr =
        make_builtin_trait_expr_info(rhs);
    out = semantic_conversion::standard_conversion_rank(lhs, rhs_expr) !=
                  semantic_conversion::CR_BAD ?
              1 :
              0;
    return true;
  }

  if(name == "__is_assignable" || name == "__is_nothrow_assignable" ||
     name == "__is_trivially_assignable") {
    TypePtr lhs_base = strip_top_level_cv(lhs);
    if(!lhs_base || lhs_base->kind != Type::TK_LVALUE_REFERENCE) {
      out = 0;
      return true;
    }

    TypePtr target = lhs_base->inner;
    TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
    if(!is_builtin_conversion_domain_type(target) ||
       !is_builtin_conversion_domain_type(rhs)) {
      return false;
    }
    if(!target_base || semantic_conversion::is_const_object_type(target) ||
       target_base->kind == Type::TK_FUNCTION ||
       target_base->kind == Type::TK_ARRAY ||
       is_void_type(target_base)) {
      out = 0;
      return true;
    }

    const semantic_conversion::ExprInfo rhs_expr =
        make_builtin_trait_expr_info(rhs);
    out = semantic_conversion::standard_conversion_rank(target, rhs_expr) !=
                  semantic_conversion::CR_BAD ?
              1 :
              0;
    return true;
  }

  return false;
}

bool evaluate_class_info_builtin_type_trait(SemanticContext & ctx,
                                            const string & name,
                                            const vector<TypePtr> & types,
                                            long long & out)
{
  if(types.size() != 1) {
    return false;
  }

  TypePtr base = strip_top_level_cv(types[0]);
  if(!base) {
    return false;
  }

  if(name == "__is_empty") {
    if(base->kind != Type::TK_NAMED) {
      out = 0;
      return true;
    }
    if(ctx.type_depends_on_template_parameter(base)) {
      return false;
    }
    ClassInfo * info = ctx.complete_class_type(base);
    if(!info || !info->complete) {
      return false;
    }
    out = ctx.is_empty_class_info(info) ? 1 : 0;
    return true;
  }

  ClassInfo * info = ctx.class_info_for_type(base);
  if(name == "__is_final") {
    out = (info && info->is_final) ? 1 : 0;
    return true;
  }
  if(name == "__is_union") {
    out = (info && info->class_kind == "union") ? 1 : 0;
    return true;
  }
  if(name == "__is_class") {
    out = (info && info->class_kind != "union") ? 1 : 0;
    return true;
  }

  return false;
}

bool classify_semantic_builtin_type_trait(
    const string & name,
    template_api::TemplateSemanticBuiltinTypeTrait & out)
{
  if(name == "__is_constructible") {
    out = template_api::TSBTT_IS_CONSTRUCTIBLE;
    return true;
  }
  if(name == "__is_nothrow_constructible") {
    out = template_api::TSBTT_IS_NOTHROW_CONSTRUCTIBLE;
    return true;
  }
  if(name == "__is_assignable") {
    out = template_api::TSBTT_IS_ASSIGNABLE;
    return true;
  }
  if(name == "__is_nothrow_assignable") {
    out = template_api::TSBTT_IS_NOTHROW_ASSIGNABLE;
    return true;
  }
  if(name == "__is_trivially_assignable") {
    out = template_api::TSBTT_IS_TRIVIALLY_ASSIGNABLE;
    return true;
  }
  if(name == "__is_convertible") {
    out = template_api::TSBTT_IS_CONVERTIBLE;
    return true;
  }
  if(name == "__is_nothrow_convertible") {
    out = template_api::TSBTT_IS_NOTHROW_CONVERTIBLE;
    return true;
  }
  return false;
}

bool evaluate_builtin_type_trait(SemanticContext & ctx,
                                 Scope & scope,
                                 const string & name,
                                 const vector<TypePtr> & types,
                                 long long & out)
{
  if(evaluate_structural_builtin_type_trait(name, types, out) ||
     evaluate_class_info_builtin_type_trait(ctx, name, types, out)) {
    return true;
  }
  if(types.size() == 1) {
    return ctx.evaluate_builtin_type_trait(scope, name, types[0], out);
  }
  if(types.size() == 2) {
    return ctx.evaluate_builtin_binary_type_trait(scope, name, types[0], types[1], out);
  }
  return false;
}

bool builtin_type_trait_needs_complete_class_metadata(const string & name)
{
  return name == "__is_trivial" ||
         name == "__is_trivially_constructible" ||
         name == "__has_trivial_constructor" ||
         name == "__is_trivially_destructible" ||
         name == "__has_trivial_destructor" ||
         name == "__is_destructible" ||
         name == "__is_nothrow_destructible" ||
         name == "__is_empty" ||
         name == "__is_abstract" ||
         name == "__is_polymorphic" ||
         name == "__has_virtual_destructor" ||
         name == "__is_literal_type" ||
         name == "__is_base_of";
}

bool type_has_incomplete_named_class_metadata(template_api::TemplateServices & services,
                                              const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return type_has_incomplete_named_class_metadata(services, base->inner);
  }
  if(base->kind != Type::TK_NAMED || base->definitely_not_class) {
    return false;
  }

  template_api::TemplateNamedTypeMetadata info;
  return template_api::describe_named_type_metadata(
             service_type_system(services).model, base, info) &&
         !info.complete;
}

bool trait_arguments_have_incomplete_named_class_metadata(
    template_api::TemplateServices & services,
    const vector<TypePtr> & types)
{
  for(size_t i = 0; i < types.size(); ++i) {
    if(type_has_incomplete_named_class_metadata(services, types[i])) {
      return true;
    }
  }
  return false;
}

bool type_references_named_class_metadata(template_api::TemplateServices & services,
                                          const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY || base->kind == Type::TK_POINTER) {
    return type_references_named_class_metadata(services, base->inner);
  }
  if(base->kind == Type::TK_MEMBER_POINTER) {
    return type_references_named_class_metadata(services, base->owner) ||
           type_references_named_class_metadata(services, base->inner);
  }
  if(base->kind != Type::TK_NAMED || base->definitely_not_class) {
    return false;
  }

  template_api::TemplateNamedTypeMetadata info;
  if(!template_api::describe_named_type_metadata(
         service_type_system(services).model, base, info)) {
    return true;
  }
  return info.class_kind != "enum";
}

bool trait_arguments_reference_named_class_metadata(
    template_api::TemplateServices & services,
    const vector<TypePtr> & types)
{
  for(size_t i = 0; i < types.size(); ++i) {
    if(type_references_named_class_metadata(services, types[i])) {
      return true;
    }
  }
  return false;
}

bool builtin_conversion_trait_uses_class_semantics(const string & name)
{
  return name == "__is_convertible" ||
         name == "__is_nothrow_convertible" ||
         name == "__is_constructible" ||
         name == "__is_nothrow_constructible" ||
         name == "__is_assignable" ||
         name == "__is_nothrow_assignable" ||
         name == "__is_trivially_assignable";
}

bool evaluate_builtin_type_trait_via_semantic_context(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & name,
    const vector<TypePtr> & types,
    long long & out)
{
  if(!services.semantic_context) {
    return false;
  }
  if(types.size() == 1) {
    return services.semantic_context->evaluate_builtin_type_trait(scope,
                                                                  name,
                                                                  types[0],
                                                                  out);
  }
  if(types.size() == 2) {
    return services.semantic_context->evaluate_builtin_binary_type_trait(scope,
                                                                         name,
                                                                         types[0],
                                                                         types[1],
                                                                         out);
  }
  return false;
}

bool evaluate_builtin_type_trait(template_api::TemplateServices & services,
                                 Scope & scope,
                                 const string & name,
                                 const vector<TypePtr> & types,
                                 long long & out)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  if((name == "__is_pod" || name == "__is_standard_layout") &&
     trait_arguments_reference_named_class_metadata(services, types) &&
     evaluate_builtin_type_trait_via_semantic_context(services, scope, name, types, out)) {
    return true;
  }
  if(name == "__is_pod" && types.size() == 1) {
    long long trivial_value = 0;
    long long standard_layout_value = 0;
    if(evaluate_builtin_type_trait(
           services, scope, "__is_trivial", types, trivial_value) &&
       evaluate_builtin_type_trait(
           services, scope, "__is_standard_layout", types, standard_layout_value)) {
      out = (trivial_value != 0 && standard_layout_value != 0) ? 1 : 0;
      return true;
    }
  }

  if(builtin_type_trait_needs_complete_class_metadata(name) &&
     trait_arguments_have_incomplete_named_class_metadata(services, types) &&
     evaluate_builtin_type_trait_via_semantic_context(services, scope, name, types, out)) {
    return true;
  }
  if(builtin_conversion_trait_uses_class_semantics(name) &&
     trait_arguments_reference_named_class_metadata(services, types) &&
     evaluate_builtin_type_trait_via_semantic_context(services, scope, name, types, out)) {
    return true;
  }

  template_api::TemplateSemanticBuiltinTypeTrait trait;
  template_api::TemplateSemanticBuiltinTraitRequest request;
  request.scope = &scope;
  request.types = types;
  return evaluate_structural_builtin_type_trait(name, types, out) ||
         evaluate_triviality_builtin_type_trait(type_system, name, types, out) ||
         evaluate_destructibility_builtin_type_trait(type_system, name, types, out) ||
         evaluate_builtin_conversion_leaf_type_trait(name, types, out) ||
         evaluate_class_info_builtin_type_trait(type_system, name, types, out) ||
         (services.semantic_context &&
          semantic_builtins::evaluate_builtin_type_trait(
              *services.semantic_context, scope, name, types, out)) ||
         (classify_semantic_builtin_type_trait(name, trait) &&
          (request.trait = trait, true) &&
          service_evaluate_semantic_builtin_type_trait(services, request, out));
}

bool build_template_member_value_syntax(const string & text,
                                        TemplateIdSyntax & qualifier_template_id,
                                        string & member_name,
                                        string & display_name)
{
  const string trimmed = trim_space(text);
  const size_t split = semantic_utils::top_level_scope_split(trimmed);
  if(split == string::npos) {
    return false;
  }

  const string owner_text = trim_space(trimmed.substr(0, split));
  member_name = trim_space(trimmed.substr(split + 2));
  if(owner_text.empty() ||
     member_name.empty() ||
     member_name.find("::") != string::npos) {
    return false;
  }

  vector<string> arg_texts;
  if(!semantic_utils::split_top_level_template_id_text(
         owner_text, qualifier_template_id.name, arg_texts) ||
     arg_texts.empty()) {
    return false;
  }

  qualifier_template_id.arguments = arg_texts;
  qualifier_template_id.argument_syntaxes.clear();
  qualifier_template_id.argument_syntaxes.reserve(arg_texts.size());
  for(size_t i = 0; i < arg_texts.size(); ++i) {
    TemplateArgumentSyntax syntax;
    syntax.text = trim_space(arg_texts[i]);
    qualifier_template_id.argument_syntaxes.push_back(syntax);
  }
  display_name = owner_text + "::" + member_name;
  return true;
}

static bool expression_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    bool check_node_text);

static bool template_id_syntax_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool check_node_text);

static bool template_argument_syntax_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    bool check_node_text);

NonTypeArgumentStatus evaluate_structured_bool_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    bool & out);

NonTypeArgumentStatus evaluate_structured_bool_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out);

NonTypeArgumentStatus evaluate_structured_bool_constant_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    bool & out);

NonTypeArgumentStatus evaluate_structured_template_member_bool_type_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & type_id,
    bool & out);
NonTypeArgumentStatus evaluate_template_member_value_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    const TypePtr & target_type);

bool structured_bool_expression_precheck_allowed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr);

bool try_evaluate_integral_expression_ast(
    template_api::TemplateServices & services,
    Scope & raw_scope,
    const CppAstNode & expr,
    const TypePtr & target_type,
    long long & out)
{
  try {
    constant_eval::ConstexprValue constexpr_value;
    if((evaluate_constant_expression_leaf_impl(
            services, raw_scope, expr, constexpr_value, target_type) ||
        ([&]() -> bool
         {
           template_api::TemplateConstantEvaluationRequest request;
           request.scope = &raw_scope;
           request.expr = expr;
           request.target_type = target_type;
           return service_evaluate_initializer_constant_value(
               services, request, constexpr_value);
         })()) &&
       constant_eval::constexpr_value_to_integral(constexpr_value, out)) {
      return true;
    }
  } catch(const ExplicitSpecializationAfterInstantiationError &) {
    throw;
  } catch(const logic_error &) {
    return false;
  }
  return false;
}

bool type_is_structured_bool_constant(template_api::TemplateServices & services,
                                      template_api::TemplateEnvironmentHandle scope,
                                      const TypePtr & type,
                                      bool & out,
                                      bool * evaluation_incomplete = nullptr)
{
  set<string> visiting;
  return structured_bool_constant_value_for_type(
      service_type_system(services),
      type,
      out,
      visiting,
      &services,
      scope,
      evaluation_incomplete);
}

NonTypeArgumentStatus evaluate_structured_bool_template_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    bool & out)
{
  if(syntax.template_id) {
    const NonTypeArgumentStatus template_status =
        evaluate_structured_bool_template_value(
            services, scope, *syntax.template_id, out);
    const bool can_try_expression =
        syntax.expression &&
        structured_bool_expression_precheck_allowed(
            services, scope, *syntax.expression);
    if(template_status == NT_ARG_EVALUATED ||
       (template_status != NT_ARG_PARSE_FAILED && !can_try_expression)) {
      return template_status;
    }
  }

  if(syntax.expression &&
     structured_bool_expression_precheck_allowed(
         services, scope, *syntax.expression)) {
    const NonTypeArgumentStatus expression_status =
        evaluate_structured_bool_expression(
            services, scope, *syntax.expression, out);
    if(expression_status != NT_ARG_PARSE_FAILED) {
      return expression_status;
    }
  }

  if(syntax.type_id) {
    const NonTypeArgumentStatus member_status =
        evaluate_structured_template_member_bool_type_id(
            services, scope, *syntax.type_id, out);
    if(member_status != NT_ARG_PARSE_FAILED) {
      return member_status;
    }

    if(const TemplateIdSyntax * type_template_id =
           cppast_template_id_syntax(*syntax.type_id)) {
      const NonTypeArgumentStatus template_status =
          evaluate_structured_bool_template_value(
              services, scope, *type_template_id, out);
      if(template_status != NT_ARG_PARSE_FAILED) {
        return template_status;
      }
    }

    TypePtr type;
    if(parse_type_id_node_for_templates(
           services, scope.require(), *syntax.type_id, type, true) &&
       type) {
      if(service_type_depends_on_template_parameter(services, type)) {
        return NT_ARG_DEPENDENT;
      }
      return evaluate_structured_bool_constant_type(
          services, scope, type, out);
    }
  }

  if(syntax.expression) {
    const NonTypeArgumentStatus expression_status =
        evaluate_structured_bool_expression(
            services, scope, *syntax.expression, out);
    if(expression_status != NT_ARG_PARSE_FAILED) {
      return expression_status;
    }
  }

  return syntax.dependent || syntax.pack_expansion ?
      NT_ARG_DEPENDENT :
      NT_ARG_PARSE_FAILED;
}

NonTypeArgumentStatus evaluate_structured_bool_template_argument_at(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    size_t index,
    bool & out)
{
  if(index < syntax.argument_syntaxes.size()) {
    return evaluate_structured_bool_template_argument(
        services, scope, syntax.argument_syntaxes[index], out);
  }
  return index < syntax.arguments.size() ? NT_ARG_DEPENDENT : NT_ARG_PARSE_FAILED;
}

bool template_id_name_is_one_of(const QualifiedName & name,
                                const char * const * names,
                                size_t count)
{
  for(size_t i = 0; i < count; ++i) {
    if(name.name == names[i]) {
      return true;
    }
  }
  return false;
}

bool structured_bool_template_id_names_type_template(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax)
{
  Scope & raw_scope = scope.require();
  const string template_name = template_api::qualified_name_text(syntax.name);
  return lookup_class_template(services, raw_scope, template_name) ||
         lookup_alias_template(services, raw_scope, template_name);
}

bool scope_is_std_namespace_or_inline_child(const Scope * scope)
{
  const Scope * current = scope;
  while(current && current->namespace_scope && current->inline_namespace) {
    current = current->parent;
  }
  if(!current ||
     !current->namespace_scope ||
     current->name != "std") {
    return false;
  }
  for(const Scope * parent = current->parent; parent; parent = parent->parent) {
    if(parent->namespace_scope &&
       parent->name != "<global>" &&
       parent->name != "<unnamed>") {
      return false;
    }
  }
  return true;
}

bool is_standard_library_class_template(template_api::TemplateServices & services,
                                        Scope & scope,
                                        const TemplateIdSyntax & syntax)
{
  ClassTemplateDecl * decl =
      lookup_class_template_impl(
          services,
          scope,
          qualified_name_text_for_structured_lookup(syntax.name));
  return decl &&
         decl->name == syntax.name.name &&
         scope_is_std_namespace_or_inline_child(decl->declaring_scope);
}

VariableTemplateDecl * lookup_standard_library_variable_template(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateIdSyntax & syntax)
{
  if(!services.semantic_context) {
    return nullptr;
  }

  const auto lookup_direct_or_inline =
      [](Scope & candidate, const string & name) -> VariableTemplateDecl *
  {
    struct Walker
    {
      static VariableTemplateDecl * find(Scope & scope, const string & name)
      {
        if(VariableTemplateDecl * direct =
               semantic_lookup::lookup_direct_variable_template(scope, name)) {
          return direct;
        }
        for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
          Scope & child = *scope.namespace_children[i];
          if(!child.inline_namespace && child.name != "<unnamed>") {
            continue;
          }
          if(VariableTemplateDecl * nested = find(child, name)) {
            return nested;
          }
        }
        return nullptr;
      }
    };
    return Walker::find(candidate, name);
  };

  VariableTemplateDecl * decl = nullptr;
  if(syntax.name.rooted || !syntax.name.qualifiers.empty()) {
    Scope * target =
        semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
            *services.semantic_context, scope, syntax.name);
    if(target) {
      decl = lookup_direct_or_inline(*target, syntax.name.name);
    }
  } else {
    decl = semantic_lookup::lookup_variable_template(
        *services.semantic_context, scope, syntax.name);
    if(!decl) {
      for(Scope * current = &scope; current; current = current->parent) {
        decl = lookup_direct_or_inline(*current, syntax.name.name);
        if(decl || current->namespace_scope || current->parent == nullptr) {
          break;
        }
      }
    }
  }
  if(!decl ||
     decl->name != syntax.name.name ||
     !decl->comes_from_standard_include_path ||
     !scope_is_std_namespace_or_inline_child(decl->declaring_scope)) {
    return nullptr;
  }
  return decl;
}

bool is_standard_library_variable_template(template_api::TemplateServices & services,
                                           Scope & scope,
                                           const TemplateIdSyntax & syntax)
{
  return lookup_standard_library_variable_template(services, scope, syntax) != nullptr;
}

bool resolve_standard_meta_bool_argument(
    template_api::TemplateServices & services,
    Scope & argument_scope,
    const TemplateIdSyntax & qualifier_template_id,
    bool & out)
{
  const TypePtr bool_type = make_fundamental(FT_BOOL);
  long long condition_value = 0;
  NonTypeArgumentStatus condition_status = NT_ARG_PARSE_FAILED;
  std::string eval_error;
  if(!qualifier_template_id.argument_syntaxes.empty() &&
     qualifier_template_id.argument_syntaxes[0].expression) {
    condition_status =
        evaluate_non_type_argument_expression(
            services,
            template_api::make_template_environment(argument_scope),
            *qualifier_template_id.argument_syntaxes[0].expression,
            condition_value,
            &eval_error,
            bool_type);
  }
  if(condition_status == NT_ARG_PARSE_FAILED ||
     condition_status == NT_ARG_EVAL_FAILED) {
    try {
      condition_status =
          evaluate_non_type_argument_text(
              services,
              template_api::make_template_environment(argument_scope),
              qualifier_template_id.arguments[0],
              condition_value,
              &eval_error,
              bool_type);
    } catch(const ExplicitSpecializationAfterInstantiationError &) {
      throw;
    } catch(const std::logic_error &) {
      condition_status = NT_ARG_PARSE_FAILED;
    }
  }
  if(condition_status != NT_ARG_EVALUATED) {
    return false;
  }
  out = condition_value != 0;
  return true;
}

bool resolve_standard_meta_type_argument(
    template_api::TemplateServices & services,
    Scope & argument_scope,
    const TemplateIdSyntax & qualifier_template_id,
    std::size_t index,
    TypePtr & out)
{
  out.reset();
  if(index >= qualifier_template_id.arguments.size()) {
    return false;
  }
  const TemplateArgumentSyntax * type_syntax =
      qualifier_template_id.argument_syntaxes.size() > index ?
          &qualifier_template_id.argument_syntaxes[index] :
          nullptr;
  if(type_syntax && type_syntax->resolved_type) {
    out = type_syntax->resolved_type;
  } else if(type_syntax && type_syntax->type_id) {
    parse_type_id_node_for_templates(services,
                                     argument_scope,
                                     *type_syntax->type_id,
                                     out,
                                     true);
  } else if(type_syntax && type_syntax->template_id) {
    resolve_template_id_syntax_type(services,
                                    argument_scope,
                                    *type_syntax->template_id,
                                    true,
                                    string(),
                                    out,
                                    template_api::make_template_environment(argument_scope));
  }
  if(!out) {
    resolve_type_argument_input(
        services,
        template_api::make_template_environment(argument_scope),
        type_syntax,
        true,
        out);
  }

  if(!out) {
    return false;
  }
  resolve_instantiated_dependent_type_if_needed(
      services, template_api::make_template_environment(argument_scope), out);
  return out != nullptr;
}

StandardMetaMemberTypeResolution try_resolve_standard_meta_member_type(
    template_api::TemplateServices & services,
    Scope & scope,
    Scope & argument_scope,
    const std::string & member_name,
    const TemplateIdSyntax & qualifier_template_id,
    TypePtr & out)
{
  out.reset();
  if(witness::source_capture_enabled(services.witness_context) ||
     member_name != "type") {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }

  const bool is_enable_if =
      qualifier_template_id.name.name == "enable_if" &&
      !qualifier_template_id.arguments.empty() &&
      qualifier_template_id.arguments.size() <= 2;
  const bool is_conditional =
      qualifier_template_id.name.name == "conditional" &&
      qualifier_template_id.arguments.size() == 3;
  if(!is_enable_if && !is_conditional) {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }

  ClassTemplateDecl * decl =
      lookup_class_template_impl(
          services,
          scope,
          qualified_name_text_for_structured_lookup(qualifier_template_id.name));
  if(!decl ||
     decl->name != qualifier_template_id.name.name ||
     !scope_is_std_namespace_or_inline_child(decl->declaring_scope)) {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }

  bool condition_value = false;
  if(!qualifier_template_id.argument_syntaxes.empty() &&
     template_argument_syntax_mentions_template_dependency(
         services,
         template_api::make_template_environment(argument_scope),
         qualifier_template_id.argument_syntaxes[0],
         true)) {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }
  if(!resolve_standard_meta_bool_argument(
         services, argument_scope, qualifier_template_id, condition_value)) {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }

  if(is_enable_if) {
    if(!condition_value) {
      if(services.counters) {
        ++services.counters->standard_enable_if_member_type_failures;
      }
      return STANDARD_META_MEMBER_SUBSTITUTION_FAILURE;
    }

    if(qualifier_template_id.arguments.size() == 1) {
      out = make_fundamental(FT_VOID);
    } else if(!resolve_standard_meta_type_argument(
                  services, argument_scope, qualifier_template_id, 1, out)) {
      return STANDARD_META_MEMBER_NOT_APPLICABLE;
    }
    if(services.counters) {
      ++services.counters->standard_enable_if_member_type_successes;
    }
    return STANDARD_META_MEMBER_RESOLVED;
  }

  const std::size_t selected_index = condition_value ? 1 : 2;
  if(!resolve_standard_meta_type_argument(
         services, argument_scope, qualifier_template_id, selected_index, out)) {
    return STANDARD_META_MEMBER_NOT_APPLICABLE;
  }
  if(services.counters) {
    ++services.counters->standard_conditional_member_type_successes;
  }
  return STANDARD_META_MEMBER_RESOLVED;
}

const char * standard_type_trait_builtin_name(const string & name)
{
  if(name == "is_abstract") return "__is_abstract";
  if(name == "is_arithmetic") return "__is_arithmetic";
  if(name == "is_array") return "__is_array";
  if(name == "is_assignable") return "__is_assignable";
  if(name == "is_base_of") return "__is_base_of";
  if(name == "is_class") return "__is_class";
  if(name == "is_compound") return "__is_compound";
  if(name == "is_const") return "__is_const";
  if(name == "is_constructible") return "__is_constructible";
  if(name == "is_convertible") return "__is_convertible";
  if(name == "is_destructible") return "__is_destructible";
  if(name == "is_empty") return "__is_empty";
  if(name == "is_enum") return "__is_enum";
  if(name == "is_final") return "__is_final";
  if(name == "is_floating_point") return "__is_floating_point";
  if(name == "is_function") return "__is_function";
  if(name == "is_fundamental") return "__is_fundamental";
  if(name == "is_integral") return "__is_integral";
  if(name == "is_lvalue_reference") return "__is_lvalue_reference";
  if(name == "is_literal_type") return "__is_literal_type";
  if(name == "is_member_function_pointer") return "__is_member_function_pointer";
  if(name == "is_member_object_pointer") return "__is_member_object_pointer";
  if(name == "is_member_pointer") return "__is_member_pointer";
  if(name == "is_nothrow_assignable") return "__is_nothrow_assignable";
  if(name == "is_nothrow_constructible") return "__is_nothrow_constructible";
  if(name == "is_nothrow_convertible") return "__is_nothrow_convertible";
  if(name == "is_nothrow_destructible") return "__is_nothrow_destructible";
  if(name == "is_object") return "__is_object";
  if(name == "is_pod") return "__is_pod";
  if(name == "is_pointer") return "__is_pointer";
  if(name == "is_polymorphic") return "__is_polymorphic";
  if(name == "is_reference") return "__is_reference";
  if(name == "is_rvalue_reference") return "__is_rvalue_reference";
  if(name == "is_same") return "__is_same";
  if(name == "is_scalar") return "__is_scalar";
  if(name == "is_signed") return "__is_signed";
  if(name == "is_standard_layout") return "__is_standard_layout";
  if(name == "is_trivial") return "__is_trivial";
  if(name == "is_trivially_assignable") return "__is_trivially_assignable";
  if(name == "is_trivially_constructible") return "__is_trivially_constructible";
  if(name == "is_trivially_copyable") return "__is_trivially_copyable";
  if(name == "is_trivially_destructible") return "__is_trivially_destructible";
  if(name == "is_union") return "__is_union";
  if(name == "is_unsigned") return "__is_unsigned";
  if(name == "is_void") return "__is_void";
  if(name == "is_volatile") return "__is_volatile";
  return nullptr;
}

bool resolve_structured_type_trait_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    TypePtr & out)
{
  out.reset();
  if(syntax.pack_expansion) {
    return false;
  }
  if(syntax.resolved_type) {
    out = syntax.resolved_type;
  } else if(syntax.type_id) {
    if(!parse_type_id_node_for_templates(
           services, scope.require(), *syntax.type_id, out, true)) {
      return false;
    }
  } else if(syntax.template_id) {
    if(!resolve_template_id_syntax_type(services,
                                        scope.require(),
                                        *syntax.template_id,
                                        false,
                                        string(),
                                        out,
                                        scope)) {
      return false;
    }
  } else {
    return false;
  }
  if(out && service_type_depends_on_template_parameter(services, out)) {
    TypePtr resolved;
    if(resolve_instantiated_dependent_type(services, scope, out, resolved) &&
       resolved) {
      out = resolved;
    }
  }
  return out && !service_type_depends_on_template_parameter(services, out);
}

bool resolve_structured_type_trait_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    std::size_t index,
    TypePtr & out)
{
  out.reset();
  if(index < syntax.argument_syntaxes.size() &&
     resolve_structured_type_trait_argument(
         services, scope, syntax.argument_syntaxes[index], out)) {
    return true;
  }
  if(index >= syntax.arguments.size()) {
    return false;
  }
  const TemplateArgumentSyntax * arg_syntax =
      index < syntax.argument_syntaxes.size() ? &syntax.argument_syntaxes[index] : nullptr;
  if(!resolve_type_argument_input(
         services, scope, arg_syntax, true, out)) {
    return false;
  }
  if(out && service_type_depends_on_template_parameter(services, out)) {
    TypePtr resolved;
    if(resolve_instantiated_dependent_type(services, scope, out, resolved) &&
       resolved) {
      out = resolved;
    }
  }
  return out && !service_type_depends_on_template_parameter(services, out);
}

bool lookup_concrete_member_type_for_trait(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & owner_type,
    const std::string & member_name,
    TypePtr & out,
    bool & lookup_complete)
{
  out.reset();
  lookup_complete = false;
  TypePtr base = strip_top_level_cv(owner_type);
  if(!base ||
     member_name.empty() ||
     service_type_depends_on_template_parameter(services, base)) {
    return false;
  }
  if(base->kind != Type::TK_NAMED ||
     base->named_key.compare(0, 5, "enum ") == 0) {
    lookup_complete = true;
    return false;
  }

  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  Scope * member_scope = nullptr;
  if(!type_system.prepare_named_type_member_scope(scope, base, member_scope) ||
     !member_scope) {
    return false;
  }

  const auto find_member_type =
      [&](Scope & target_scope) -> bool
      {
        std::map<std::string, TypePtr>::const_iterator direct =
            target_scope.named_types.find(member_name);
        if(direct != target_scope.named_types.end() && direct->second) {
          out = direct->second;
          return true;
        }
        if(target_scope.class_info &&
           !target_scope.class_info->bases.empty() &&
           type_system.resolve_member_type_lookup(scope.require(),
                                                  *target_scope.class_info,
                                                  member_name,
                                                  true,
                                                  out) &&
           out) {
          return true;
        }
        return false;
      };

  if(find_member_type(*member_scope)) {
    lookup_complete = true;
    return true;
  }

  if(member_scope->class_info &&
     !member_scope->class_info->complete &&
     !member_scope->class_info->template_instantiation_in_progress &&
     !member_scope->class_info->full_member_collection_in_progress) {
    Scope * completed_scope = nullptr;
    if(type_system.complete_named_type_member_scope(scope, base, completed_scope) &&
       completed_scope) {
      member_scope = completed_scope;
      if(find_member_type(*member_scope)) {
        lookup_complete = true;
        return true;
      }
    }
  }

  lookup_complete = member_scope->class_info && member_scope->class_info->complete;
  return false;
}

NonTypeArgumentStatus evaluate_libcpp_trivially_relocatable_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  if(syntax.name.name != "__libcpp_is_trivially_relocatable" ||
     syntax.arguments.size() != 1 ||
     syntax.argument_syntaxes.size() != syntax.arguments.size() ||
     !is_standard_library_class_template(services, scope.require(), syntax)) {
    return NT_ARG_PARSE_FAILED;
  }

  TypePtr type;
  if(!resolve_structured_type_trait_argument(
         services, scope, syntax.argument_syntaxes[0], type)) {
    return template_id_syntax_mentions_template_dependency(
               services, scope, syntax, false) ?
        NT_ARG_DEPENDENT :
        NT_ARG_PARSE_FAILED;
  }

  TypePtr marker_type;
  bool member_lookup_complete = false;
  const bool has_marker =
      lookup_concrete_member_type_for_trait(services,
                                            scope,
                                            type,
                                            "__trivially_relocatable",
                                            marker_type,
                                            member_lookup_complete);
  if(has_marker && type_equals(type, marker_type)) {
    out = true;
    return NT_ARG_EVALUATED;
  }
  if(!member_lookup_complete) {
    return NT_ARG_PARSE_FAILED;
  }

  long long copyable_value = 0;
  if(!evaluate_builtin_type_trait(services,
                                  scope.require(),
                                  "__is_trivially_copyable",
                                  std::vector<TypePtr>(1, type),
                                  copyable_value)) {
    return NT_ARG_EVAL_FAILED;
  }
  out = copyable_value != 0;
  return NT_ARG_EVALUATED;
}

NonTypeArgumentStatus evaluate_standard_invocable_variable_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  const string & name = syntax.name.name;
  const bool is_invocable =
      name == "__is_invocable_v" || name == "is_invocable_v";
  const bool is_invocable_r =
      name == "__is_invocable_r_v" || name == "is_invocable_r_v";
  const bool is_nothrow_invocable =
      name == "__is_nothrow_invocable_v" ||
      name == "is_nothrow_invocable_v";
  const bool is_nothrow_invocable_r =
      name == "__is_nothrow_invocable_r_v" ||
      name == "is_nothrow_invocable_r_v";
  if(!is_invocable &&
     !is_invocable_r &&
     !is_nothrow_invocable &&
     !is_nothrow_invocable_r) {
    return NT_ARG_PARSE_FAILED;
  }
  VariableTemplateDecl * variable_template =
      lookup_standard_library_variable_template(services, scope.require(), syntax);
  if(!variable_template) {
    return NT_ARG_PARSE_FAILED;
  }

  TemplateIdSyntax expanded = clone_template_id_for_template_substitution(syntax);
  expand_bound_packs_in_template_id_syntax(services, scope.require(), expanded);
  const std::size_t required_fixed_args =
      (is_invocable_r || is_nothrow_invocable_r) ? 2 : 1;
  if(expanded.arguments.size() < required_fixed_args) {
    return NT_ARG_EVAL_FAILED;
  }

  vector<TemplateArgument> actual_arguments;
  actual_arguments.reserve(expanded.arguments.size());
  vector<TemplateArgument> arguments;
  arguments.reserve(expanded.arguments.size() +
                    ((is_invocable || is_nothrow_invocable) ? 1 : 0));
  if(is_invocable || is_nothrow_invocable) {
    TemplateArgument void_argument;
    void_argument.kind = TemplateArgument::TA_TYPE;
    void_argument.type = make_fundamental(FT_VOID);
    void_argument.text = "void";
    arguments.push_back(void_argument);
  }
  for(std::size_t i = 0; i < expanded.arguments.size(); ++i) {
    TypePtr type;
    if(!resolve_structured_type_trait_argument(
           services, scope, expanded, i, type)) {
      return template_id_syntax_mentions_template_dependency(
                 services, scope, expanded, false) ?
          NT_ARG_DEPENDENT :
          NT_ARG_EVAL_FAILED;
    }
    TemplateArgument argument;
    argument.kind = TemplateArgument::TA_TYPE;
    argument.type = type;
    argument.text = reparseable_type_argument_text(type);
    actual_arguments.push_back(argument);
    arguments.push_back(argument);
  }

  if(witness::source_capture_enabled(services.witness_context) &&
     services.semantic_context) {
    template_api::TemplateVariableInstantiationRequest request;
    request.decl = variable_template;
    request.arguments = actual_arguments;
    request.source_use_scope = &scope.require();
    request.intent = template_api::TemplateInstantiationIntent::TrackInstantiation;
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    (void)template_api::acquire_variable_instantiation(
        *services.semantic_context,
        request);
  }

  const bool require_nothrow =
      is_nothrow_invocable || is_nothrow_invocable_r;
  bool value = false;
  bool evaluation_incomplete = false;
  if(!evaluate_structured_invocable_r_trait(
         service_type_system(services),
         arguments,
         require_nothrow,
         value,
         &services,
         scope,
         &evaluation_incomplete)) {
    return evaluation_incomplete ? NT_ARG_DEPENDENT : NT_ARG_EVAL_FAILED;
  }
  out = value;
  return NT_ARG_EVALUATED;
}

bool concrete_type_is_std_class_template_instantiation(
    template_api::TemplateServices & services,
    const TypePtr & type,
    const char * template_name)
{
  if(!type || type->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = class_info_for_named_type(services, type);
  return info &&
         info->source_template &&
         info->source_template->name == template_name &&
         scope_is_std_namespace_or_inline_child(info->source_template->declaring_scope);
}

NonTypeArgumentStatus evaluate_libcpp_simple_variable_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  if(witness::source_capture_enabled(services.witness_context)) {
    return NT_ARG_PARSE_FAILED;
  }

  const bool is_pair = syntax.name.name == "__is_pair_v";
  const bool is_tuple = syntax.name.name == "__is_tuple_v";
  if(!is_pair && !is_tuple) {
    return NT_ARG_PARSE_FAILED;
  }
  if(syntax.arguments.size() != 1 ||
     syntax.argument_syntaxes.size() != syntax.arguments.size()) {
    return NT_ARG_EVAL_FAILED;
  }
  if(!lookup_standard_library_variable_template(services, scope.require(), syntax)) {
    return NT_ARG_PARSE_FAILED;
  }

  TypePtr type;
  if(!resolve_structured_type_trait_argument(
         services, scope, syntax.argument_syntaxes[0], type)) {
    return template_id_syntax_mentions_template_dependency(
               services, scope, syntax, false) ?
        NT_ARG_DEPENDENT :
        NT_ARG_PARSE_FAILED;
  }

  out = concrete_type_is_std_class_template_instantiation(
      services, type, is_pair ? "pair" : "tuple");
  return NT_ARG_EVALUATED;
}

bool expression_maybe_libcpp_tuple_size_value(const CppAstNode & expr)
{
  if(expr.kind != CppAstKind::id_expression) {
    return false;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(!qualified ||
     qualified->name != "value" ||
     qualified->qualifiers.empty()) {
    return false;
  }
  const size_t qualifier_index = qualified->qualifiers.size() - 1;
  const TemplateIdSyntax * qualifier_template_id =
      cppast_qualifier_template_id_syntax(expr, qualifier_index);
  return qualifier_template_id &&
         qualifier_template_id->name.name == "tuple_size";
}

NonTypeArgumentStatus evaluate_libcpp_tuple_size_member_integral_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & out)
{
  if(witness::source_capture_enabled(services.witness_context) ||
     !expression_maybe_libcpp_tuple_size_value(expr)) {
    return NT_ARG_PARSE_FAILED;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  const size_t qualifier_index = qualified->qualifiers.size() - 1;
  const TemplateIdSyntax * qualifier_template_id =
      cppast_qualifier_template_id_syntax(expr, qualifier_index);
  if(!qualifier_template_id ||
     qualifier_template_id->arguments.size() != 1 ||
     qualifier_template_id->argument_syntaxes.size() !=
         qualifier_template_id->arguments.size() ||
     !is_standard_library_class_template(services,
                                         scope.require(),
                                         *qualifier_template_id)) {
    return NT_ARG_PARSE_FAILED;
  }

  TypePtr type;
  if(!resolve_structured_type_trait_argument(
         services, scope, qualifier_template_id->argument_syntaxes[0], type)) {
    return template_id_syntax_mentions_template_dependency(
               services, scope, *qualifier_template_id, false) ?
        NT_ARG_DEPENDENT :
        NT_ARG_PARSE_FAILED;
  }

  TypePtr base = strip_top_level_cv(type);
  ClassInfo * info = class_info_for_named_type(services, base);
  if(!info ||
     !info->source_template ||
     !scope_is_std_namespace_or_inline_child(info->source_template->declaring_scope)) {
    return NT_ARG_PARSE_FAILED;
  }
  if(info->source_template->name == "tuple") {
    out = static_cast<long long>(info->instantiation_arguments.size());
    return NT_ARG_EVALUATED;
  }
  if(info->source_template->name == "pair") {
    out = 2;
    return NT_ARG_EVALUATED;
  }
  return NT_ARG_PARSE_FAILED;
}

bool type_may_name_user_defined_entity(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_FUNDAMENTAL:
    return false;
  case Type::TK_NAMED:
    return true;
  case Type::TK_CV:
    return type_may_name_user_defined_entity(base->inner);
  case Type::TK_ATOMIC:
    return type_may_name_user_defined_entity(base->inner);
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return type_may_name_user_defined_entity(base->inner);
  case Type::TK_MEMBER_POINTER:
    return type_may_name_user_defined_entity(base->owner) ||
           type_may_name_user_defined_entity(base->inner);
  case Type::TK_FUNCTION:
    if(type_may_name_user_defined_entity(base->inner)) {
      return true;
    }
    for(size_t i = 0; i < base->params.size(); ++i) {
      if(type_may_name_user_defined_entity(base->params[i])) {
        return true;
      }
    }
    return false;
  }
  return true;
}

NonTypeArgumentStatus evaluate_standard_error_enum_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  if(syntax.name.name != "is_error_code_enum" &&
     syntax.name.name != "is_error_condition_enum") {
    return NT_ARG_PARSE_FAILED;
  }
  if(syntax.arguments.size() != 1 ||
     syntax.argument_syntaxes.size() != syntax.arguments.size() ||
     !is_standard_library_class_template(services, scope.require(), syntax)) {
    return NT_ARG_PARSE_FAILED;
  }

  TypePtr type;
  if(!resolve_structured_type_trait_argument(
         services, scope, syntax.argument_syntaxes[0], type)) {
    return template_id_syntax_mentions_template_dependency(
               services, scope, syntax, false) ?
        NT_ARG_DEPENDENT :
        NT_ARG_PARSE_FAILED;
  }

  if(type_may_name_user_defined_entity(type)) {
    return NT_ARG_PARSE_FAILED;
  }
  out = false;
  return NT_ARG_EVALUATED;
}

NonTypeArgumentStatus evaluate_standard_type_trait_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  const NonTypeArgumentStatus error_enum_status =
      evaluate_standard_error_enum_template_value(services, scope, syntax, out);
  if(error_enum_status != NT_ARG_PARSE_FAILED) {
    return error_enum_status;
  }

  const NonTypeArgumentStatus relocatable_status =
      evaluate_libcpp_trivially_relocatable_template_value(
          services, scope, syntax, out);
  if(relocatable_status != NT_ARG_PARSE_FAILED) {
    return relocatable_status;
  }

  const char * builtin_name =
      standard_type_trait_builtin_name(syntax.name.name);
  if(!builtin_name ||
     syntax.argument_syntaxes.size() != syntax.arguments.size() ||
     !is_standard_library_class_template(services, scope.require(), syntax)) {
    return NT_ARG_PARSE_FAILED;
  }

  vector<TypePtr> types;
  types.reserve(syntax.argument_syntaxes.size());
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    TypePtr type;
    bool resolved_argument = false;
    {
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      resolved_argument =
          resolve_structured_type_trait_argument(
              services, scope, syntax.argument_syntaxes[i], type);
    }
    if(!resolved_argument) {
      return template_id_syntax_mentions_template_dependency(
                 services, scope, syntax, false) ?
          NT_ARG_DEPENDENT :
          NT_ARG_PARSE_FAILED;
    }
    types.push_back(type);
  }

  long long value = 0;
  if(!evaluate_builtin_type_trait(services,
                                  scope.require(),
                                  builtin_name,
                                  types,
                                  value)) {
    return NT_ARG_EVAL_FAILED;
  }
  out = value != 0;
  return NT_ARG_EVALUATED;
}

NonTypeArgumentStatus evaluate_structured_bool_template_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool & out)
{
  if(syntax.name.name == "_BoolConstant" ||
     syntax.name.name == "bool_constant") {
    if(syntax.arguments.size() != 1) {
      return NT_ARG_PARSE_FAILED;
    }
    if(syntax.argument_syntaxes.size() >= 1 &&
       syntax.argument_syntaxes[0].expression) {
      long long value = 0;
      if(try_evaluate_integral_expression_ast(
             services,
             scope.require(),
             *syntax.argument_syntaxes[0].expression,
             make_fundamental(FT_BOOL),
             value)) {
        out = value != 0;
        return NT_ARG_EVALUATED;
      }
      return expression_ast_mentions_template_dependency(
                 services, scope, *syntax.argument_syntaxes[0].expression, true) ?
          NT_ARG_DEPENDENT :
          NT_ARG_EVAL_FAILED;
    }
    return evaluate_structured_bool_template_argument_at(
        services, scope, syntax, 0, out);
  }

  if(syntax.name.name == "integral_constant" &&
     syntax.arguments.size() == 2) {
    TypePtr value_type;
    bool value_type_is_bool = false;
    if(syntax.argument_syntaxes.size() >= 1 &&
       syntax.argument_syntaxes[0].type_id) {
      value_type_is_bool =
          parse_type_id_node_for_templates(services,
                                           scope.require(),
                                           *syntax.argument_syntaxes[0].type_id,
                                           value_type,
                                           true) &&
          is_bool_type(value_type);
    }
    if(!value_type_is_bool &&
       syntax.arguments.size() >= 1 &&
       trim_space(syntax.arguments[0]) != "bool") {
      return NT_ARG_PARSE_FAILED;
    }
    if(syntax.argument_syntaxes.size() >= 2 &&
       syntax.argument_syntaxes[1].expression) {
      long long value = 0;
      if(try_evaluate_integral_expression_ast(
             services,
             scope.require(),
             *syntax.argument_syntaxes[1].expression,
             make_fundamental(FT_BOOL),
             value)) {
        out = value != 0;
        return NT_ARG_EVALUATED;
      }
      return expression_ast_mentions_template_dependency(
                 services, scope, *syntax.argument_syntaxes[1].expression, true) ?
          NT_ARG_DEPENDENT :
          NT_ARG_EVAL_FAILED;
    }
    return evaluate_structured_bool_template_argument_at(
        services, scope, syntax, 1, out);
  }

  const char * const conjunction_names[] = {
    "_And",
    "conjunction",
    "__all",
    "__and_"
  };
  if(template_id_name_is_one_of(
         syntax.name,
         conjunction_names,
         sizeof(conjunction_names) / sizeof(conjunction_names[0]))) {
    for(size_t i = 0; i < syntax.arguments.size(); ++i) {
      bool value = false;
      const NonTypeArgumentStatus status =
          evaluate_structured_bool_template_argument_at(
              services, scope, syntax, i, value);
      if(status == NT_ARG_EVALUATED) {
        if(!value) {
          out = false;
          return NT_ARG_EVALUATED;
        }
        continue;
      }
      return status == NT_ARG_PARSE_FAILED ? NT_ARG_DEPENDENT : status;
    }
    out = true;
    return NT_ARG_EVALUATED;
  }

  const char * const disjunction_names[] = {
    "_Or",
    "disjunction",
    "__or_"
  };
  if(template_id_name_is_one_of(
         syntax.name,
         disjunction_names,
         sizeof(disjunction_names) / sizeof(disjunction_names[0]))) {
    for(size_t i = 0; i < syntax.arguments.size(); ++i) {
      bool value = false;
      const NonTypeArgumentStatus status =
          evaluate_structured_bool_template_argument_at(
              services, scope, syntax, i, value);
      if(status == NT_ARG_EVALUATED) {
        if(value) {
          out = true;
          return NT_ARG_EVALUATED;
        }
        continue;
      }
      return status == NT_ARG_PARSE_FAILED ? NT_ARG_DEPENDENT : status;
    }
    out = false;
    return NT_ARG_EVALUATED;
  }

  if((syntax.name.name == "_Not" ||
      syntax.name.name == "negation" ||
      syntax.name.name == "__not_") &&
     syntax.arguments.size() == 1) {
    bool value = false;
    const NonTypeArgumentStatus status =
        evaluate_structured_bool_template_argument_at(
            services, scope, syntax, 0, value);
    if(status == NT_ARG_EVALUATED) {
      out = !value;
    }
    return status;
  }

  const NonTypeArgumentStatus trait_status =
      evaluate_standard_type_trait_template_value(services, scope, syntax, out);
  if(trait_status != NT_ARG_PARSE_FAILED) {
    return trait_status;
  }

  TypePtr owner_type;
  if(resolve_template_id_syntax_type(services,
                                     scope.require(),
                                     syntax,
                                     false,
                                     string(),
                                     owner_type,
                                     scope) &&
     owner_type) {
    if(service_type_depends_on_template_parameter(services, owner_type)) {
      return NT_ARG_DEPENDENT;
    }
    bool structured_evaluation_incomplete = false;
    if(!witness::source_capture_enabled(services.witness_context) &&
       type_is_structured_bool_constant(services,
                                        scope,
                                        owner_type,
                                        out,
                                        &structured_evaluation_incomplete)) {
      note_structured_bool_value_member_if_needed(services, scope, owner_type);
      return NT_ARG_EVALUATED;
    }
    if(structured_evaluation_incomplete) {
      return NT_ARG_DEPENDENT;
    }
    Scope * member_scope = nullptr;
    constant_eval::ConstexprValue member_value;
    bool truthy = false;
    if(prepare_concrete_type_member_scope(
           services, scope, owner_type, member_scope) &&
       member_scope &&
       service_lookup_leaf_member_expression_value_in_scope(
           services,
           *member_scope,
           kStructuredBoolResultMemberName,
           member_value) &&
       constant_eval::constexpr_value_truthy(member_value, truthy)) {
      note_structured_bool_value_member_if_needed(services, scope, owner_type);
      out = truthy;
      return NT_ARG_EVALUATED;
    }
  }

  if(template_id_syntax_mentions_template_dependency(services, scope, syntax, true)) {
    return NT_ARG_DEPENDENT;
  }
  return structured_bool_template_id_names_type_template(services, scope, syntax) ?
      NT_ARG_EVAL_FAILED :
      NT_ARG_PARSE_FAILED;
}

NonTypeArgumentStatus evaluate_structured_bool_constant_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & type,
    bool & out)
{
  if(!type) {
    return NT_ARG_EVAL_FAILED;
  }
  if(service_type_depends_on_template_parameter(services, type)) {
    return NT_ARG_DEPENDENT;
  }
  bool structured_evaluation_incomplete = false;
  if(!witness::source_capture_enabled(services.witness_context) &&
     type_is_structured_bool_constant(services,
                                      scope,
                                      type,
                                      out,
                                      &structured_evaluation_incomplete)) {
    note_structured_bool_value_member_if_needed(services, scope, type);
    return NT_ARG_EVALUATED;
  }
  if(structured_evaluation_incomplete) {
    return NT_ARG_DEPENDENT;
  }

  Scope * member_scope = nullptr;
  constant_eval::ConstexprValue member_value;
  bool truthy = false;
  if(prepare_concrete_type_member_scope(services, scope, type, member_scope) &&
     member_scope &&
     service_lookup_leaf_member_expression_value_in_scope(
         services,
         *member_scope,
         kStructuredBoolResultMemberName,
         member_value) &&
     constant_eval::constexpr_value_truthy(member_value, truthy)) {
    note_structured_bool_value_member_if_needed(services, scope, type);
    out = truthy;
    return NT_ARG_EVALUATED;
  }

  return NT_ARG_EVAL_FAILED;
}

NonTypeArgumentStatus evaluate_structured_template_member_bool_qualifier_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    const QualifiedName & qualified,
    bool & out)
{
  for(size_t offset = 0; offset < qualified.qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified.qualifiers.size() - 1 - offset;
    const CppAstNode * qualifier_type_syntax =
        cppast_qualifier_type_syntax(node, qualifier_index);
    if(!qualifier_type_syntax) {
      continue;
    }

    TypePtr qualifier_type;
    bool parsed_qualifier_type = false;
    try {
      parsed_qualifier_type =
          parse_type_id_node_for_templates(
              services, scope.require(), *qualifier_type_syntax, qualifier_type, true) &&
          qualifier_type;
    } catch(const DependentQualifiedTypeMissingTypenameError &) {
      return NT_ARG_DEPENDENT;
    } catch(const TemplateSubstitutionFailure &) {
      return NT_ARG_EVAL_FAILED;
    } catch(const SemanticSoftFailure &) {
      return NT_ARG_EVAL_FAILED;
    } catch(const SemanticDiagnosticError &) {
      return NT_ARG_EVAL_FAILED;
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      return NT_ARG_EVAL_FAILED;
    }
    if(!parsed_qualifier_type) {
      return NT_ARG_EVAL_FAILED;
    }
    const NonTypeArgumentStatus status =
        evaluate_structured_bool_constant_type(
            services, scope, qualifier_type, out);
    if(status == NT_ARG_EVALUATED) {
      note_structured_bool_value_member_if_needed(services, scope, qualifier_type);
    }
    return status;
  }

  return NT_ARG_PARSE_FAILED;
}

NonTypeArgumentStatus evaluate_structured_template_member_bool_value(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    bool & out)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(!qualified ||
     qualified->name != "value" ||
     qualified->qualifiers.empty()) {
    return NT_ARG_PARSE_FAILED;
  }

  for(size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    const TemplateIdSyntax * qualifier_template_id =
        cppast_qualifier_template_id_syntax(expr, qualifier_index);
    if(!qualifier_template_id) {
      continue;
    }
    NonTypeArgumentStatus status = NT_ARG_PARSE_FAILED;
    {
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      try {
        status =
            evaluate_structured_bool_template_value(
                services, scope, *qualifier_template_id, out);
      } catch(const TemplateSubstitutionFailure &) {
        status = NT_ARG_EVAL_FAILED;
      } catch(const SemanticSoftFailure &) {
        status = NT_ARG_EVAL_FAILED;
      } catch(const SemanticDiagnosticError &) {
        status = NT_ARG_EVAL_FAILED;
      } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        status = NT_ARG_EVAL_FAILED;
      } catch(const logic_error &) {
        status = NT_ARG_EVAL_FAILED;
      }
    }
    if(status == NT_ARG_EVALUATED) {
      note_structured_bool_value_member_if_needed(
          services, scope, *qualifier_template_id);
      return status;
    }
    if(status == NT_ARG_DEPENDENT) {
      return status;
    }
    if(status == NT_ARG_EVAL_FAILED || status == NT_ARG_PARSE_FAILED) {
      long long value = 0;
      try {
        const NonTypeArgumentStatus member_status =
            evaluate_template_member_value_expression(
                services, scope, expr, value, make_fundamental(FT_BOOL));
        if(member_status == NT_ARG_EVALUATED) {
          out = value != 0;
          return member_status;
        }
        if(member_status == NT_ARG_DEPENDENT) {
          return member_status;
        }
      } catch(const TemplateSubstitutionFailure &) {
        return NT_ARG_EVAL_FAILED;
      } catch(const SemanticSoftFailure &) {
        return NT_ARG_EVAL_FAILED;
      } catch(const SemanticDiagnosticError &) {
        return NT_ARG_EVAL_FAILED;
      } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        return NT_ARG_EVAL_FAILED;
      } catch(const logic_error &) {
        return NT_ARG_EVAL_FAILED;
      }
      const bool template_id_directly_owns_value =
          qualifier_index + 1 == qualified->qualifiers.size();
      if(status == NT_ARG_EVAL_FAILED && template_id_directly_owns_value) {
        return NT_ARG_EVAL_FAILED;
      }
      break;
    }
    if(status == NT_ARG_EVAL_FAILED) {
      return NT_ARG_EVAL_FAILED;
    }
    break;
  }

  const NonTypeArgumentStatus qualifier_type_status =
      evaluate_structured_template_member_bool_qualifier_type(
          services, scope, expr, *qualified, out);
  if(qualifier_type_status != NT_ARG_PARSE_FAILED) {
    return qualifier_type_status;
  }

  return NT_ARG_PARSE_FAILED;
}

const CppAstNode * single_type_name_from_type_id(const CppAstNode & type_id)
{
  if(type_id.kind != CppAstKind::type_id) {
    return nullptr;
  }
  const CppAstNode * specifiers =
      cppast_find_child_kind(type_id, CppAstKind::type_specifier_seq);
  if(!specifiers ||
     specifiers->children.size() != 1 ||
     specifiers->children[0].kind != CppAstKind::type_name) {
    return nullptr;
  }
  const CppAstNode * declarator =
      cppast_find_child_kind(type_id, CppAstKind::abstract_declarator);
  if(declarator && !trim_space(node_text(*declarator)).empty()) {
    return nullptr;
  }
  return &specifiers->children[0];
}

NonTypeArgumentStatus evaluate_structured_template_member_bool_type_id(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & type_id,
    bool & out)
{
  const CppAstNode * type_name = single_type_name_from_type_id(type_id);
  if(!type_name) {
    return NT_ARG_PARSE_FAILED;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(*type_name);
  if(!qualified ||
     qualified->name != "value" ||
     qualified->qualifiers.empty()) {
    return NT_ARG_PARSE_FAILED;
  }

  for(size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    const TemplateIdSyntax * qualifier_template_id =
        cppast_qualifier_template_id_syntax(*type_name, qualifier_index);
    if(!qualifier_template_id) {
      continue;
    }
    NonTypeArgumentStatus status = NT_ARG_PARSE_FAILED;
    {
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      status =
          evaluate_structured_bool_template_value(
              services, scope, *qualifier_template_id, out);
    }
    if(status == NT_ARG_EVALUATED) {
      note_structured_bool_value_member_if_needed(
          services, scope, *qualifier_template_id);
      return status;
    }
    if(status == NT_ARG_DEPENDENT) {
      return status;
    }
    break;
  }

  const NonTypeArgumentStatus qualifier_type_status =
      evaluate_structured_template_member_bool_qualifier_type(
          services, scope, *type_name, *qualified, out);
  if(qualifier_type_status != NT_ARG_PARSE_FAILED) {
    return qualifier_type_status;
  }

  return NT_ARG_PARSE_FAILED;
}

const CppAstNode * structured_bool_functional_cast_operand(const CppAstNode & expr)
{
  if(expr.kind != CppAstKind::call_expression) {
    return nullptr;
  }
  const CppAstNode * callee = unwrap_call_callee(expr);
  if(!callee ||
     callee->kind != CppAstKind::id_expression ||
     callee->value != "bool") {
    return nullptr;
  }
  const CppAstNode * argument_list = find_child_kind(expr, CppAstKind::argument_list);
  if(!argument_list) {
    argument_list = find_child_kind(expr, CppAstKind::paren_argument_list);
  }
  if(!argument_list || argument_list->children.size() != 1) {
    return nullptr;
  }
  return &argument_list->children[0];
}

bool structured_template_member_bool_value_is_pure_trait(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr)
{
  if(expr.kind == CppAstKind::parenthesized_expression &&
     expr.children.size() == 1) {
    return structured_template_member_bool_value_is_pure_trait(
        services, scope, expr.children[0]);
  }

  if(expr.kind == CppAstKind::cast_expression &&
     expr.children.size() == 2 &&
     expr.children[0].kind == CppAstKind::type_id) {
    TypePtr target_type;
    if(parse_type_id_node_for_templates(
           services, scope.require(), expr.children[0], target_type, true) &&
       target_type &&
       is_bool_type(strip_top_level_cv(remove_reference_type(target_type)))) {
      return structured_template_member_bool_value_is_pure_trait(
          services, scope, expr.children[1]);
    }
  }

  if(const CppAstNode * operand =
         structured_bool_functional_cast_operand(expr)) {
    return structured_template_member_bool_value_is_pure_trait(
        services, scope, *operand);
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(!qualified ||
     qualified->name != "value" ||
     qualified->qualifiers.empty()) {
    return false;
  }

  const size_t qualifier_index = qualified->qualifiers.size() - 1;
  const TemplateIdSyntax * qualifier_template_id =
      cppast_qualifier_template_id_syntax(expr, qualifier_index);
  if(!qualifier_template_id) {
    return false;
  }

  const char * const conjunction_names[] = {
    "_And",
    "conjunction",
    "__all",
    "__and_"
  };
  if(template_id_name_is_one_of(
         qualifier_template_id->name,
         conjunction_names,
         sizeof(conjunction_names) / sizeof(conjunction_names[0]))) {
    return true;
  }

  const char * const disjunction_names[] = {
    "_Or",
    "disjunction",
    "__or_"
  };
  if(template_id_name_is_one_of(
         qualifier_template_id->name,
         disjunction_names,
         sizeof(disjunction_names) / sizeof(disjunction_names[0]))) {
    return true;
  }

  if(qualifier_template_id->name.name == "_Not" ||
     qualifier_template_id->name.name == "negation" ||
     qualifier_template_id->name.name == "__not_") {
    return true;
  }

  return standard_type_trait_builtin_name(qualifier_template_id->name.name) &&
         is_standard_library_class_template(
             services, scope.require(), *qualifier_template_id);
}

bool type_id_node_is_bool_type(template_api::TemplateServices & services,
                               template_api::TemplateEnvironmentHandle scope,
                               const CppAstNode & type_id)
{
  TypePtr target_type;
  return parse_type_id_node_for_templates(
             services, scope.require(), type_id, target_type, true) &&
         target_type &&
         is_bool_type(strip_top_level_cv(remove_reference_type(target_type)));
}

bool id_expression_maybe_structured_bool_value(const CppAstNode & expr)
{
  if(expr.kind != CppAstKind::id_expression) {
    return false;
  }
  if(expr.value == "true" || expr.value == "false") {
    return true;
  }
  if(cppast_template_id_syntax(expr)) {
    return true;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  return (qualified && (qualified->rooted || !qualified->qualifiers.empty())) ||
         expr.value.find("::") != string::npos;
}

bool structured_bool_expression_precheck_allowed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr)
{
  if(expr.kind == CppAstKind::keyword_literal) {
    return true;
  }
  if(expr.kind == CppAstKind::id_expression) {
    return id_expression_maybe_structured_bool_value(expr);
  }
  if(expr.kind == CppAstKind::parenthesized_expression &&
     expr.children.size() == 1) {
    return structured_bool_expression_precheck_allowed(
        services, scope, expr.children[0]);
  }
  if(expr.kind == CppAstKind::unary_expression &&
     expr.children.size() == 1 &&
     expr.has_token &&
     expr.simple_type == OP_LNOT) {
    return structured_bool_expression_precheck_allowed(
        services, scope, expr.children[0]);
  }
  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token &&
     (expr.simple_type == OP_LAND || expr.simple_type == OP_LOR)) {
    return true;
  }
  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token &&
     (expr.simple_type == OP_EQ || expr.simple_type == OP_NE) &&
     (expression_maybe_libcpp_tuple_size_value(expr.children[0]) ||
      expression_maybe_libcpp_tuple_size_value(expr.children[1]))) {
    return true;
  }
  if(structured_bool_functional_cast_operand(expr)) {
    return true;
  }
  return expr.kind == CppAstKind::cast_expression &&
         expr.children.size() == 2 &&
         expr.children[0].kind == CppAstKind::type_id &&
         type_id_node_is_bool_type(services, scope, expr.children[0]);
}

NonTypeArgumentStatus evaluate_structured_bool_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    bool & out)
{
  if(expr.kind == CppAstKind::keyword_literal) {
    if(node_has_simple_type(expr, KW_TRUE)) {
      out = true;
      return NT_ARG_EVALUATED;
    }
    if(node_has_simple_type(expr, KW_FALSE)) {
      out = false;
      return NT_ARG_EVALUATED;
    }
  }
  if(expr.kind == CppAstKind::id_expression &&
     (expr.value == "true" || expr.value == "false")) {
    out = expr.value == "true";
    return NT_ARG_EVALUATED;
  }

  bool id_member_status_attempted = false;
  NonTypeArgumentStatus id_member_status = NT_ARG_PARSE_FAILED;
  if(expr.kind == CppAstKind::id_expression) {
    if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(expr)) {
      const NonTypeArgumentStatus libcpp_variable_status =
          evaluate_libcpp_simple_variable_template_value(
              services, scope, *template_id, out);
      if(libcpp_variable_status != NT_ARG_PARSE_FAILED) {
        return libcpp_variable_status;
      }

      const NonTypeArgumentStatus variable_template_status =
          evaluate_standard_invocable_variable_template_value(
              services, scope, *template_id, out);
      if(variable_template_status != NT_ARG_PARSE_FAILED) {
        return variable_template_status;
      }
    }

    id_member_status_attempted = true;
    id_member_status =
        evaluate_structured_template_member_bool_value(
            services, scope, expr, out);
    if(id_member_status == NT_ARG_EVALUATED ||
       id_member_status == NT_ARG_DEPENDENT) {
      return id_member_status;
    }

    long long member_value = 0;
    NonTypeArgumentStatus direct_member_status = NT_ARG_PARSE_FAILED;
    try {
      direct_member_status =
          evaluate_template_member_value_expression(
              services,
              scope,
              expr,
              member_value,
              make_fundamental(FT_BOOL));
    } catch(const TemplateSubstitutionFailure &) {
      direct_member_status = NT_ARG_EVAL_FAILED;
    } catch(const SemanticSoftFailure &) {
      direct_member_status = NT_ARG_EVAL_FAILED;
    } catch(const SemanticDiagnosticError &) {
      direct_member_status = NT_ARG_EVAL_FAILED;
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      direct_member_status = NT_ARG_EVAL_FAILED;
    }
    if(direct_member_status == NT_ARG_EVALUATED) {
      out = member_value != 0;
      return direct_member_status;
    }
    if(direct_member_status == NT_ARG_DEPENDENT) {
      return direct_member_status;
    }
  }

  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token &&
     (expr.simple_type == OP_LAND || expr.simple_type == OP_LOR)) {
    bool lhs = false;
    const NonTypeArgumentStatus lhs_status =
        evaluate_structured_bool_expression(
            services, scope, expr.children[0], lhs);
    if(lhs_status != NT_ARG_EVALUATED) {
      if(lhs_status == NT_ARG_PARSE_FAILED &&
         expression_ast_mentions_template_dependency(services, scope, expr, true)) {
        return NT_ARG_DEPENDENT;
      }
      return lhs_status;
    }
    if(expr.simple_type == OP_LAND && !lhs) {
      if(!expression_ast_mentions_template_dependency(
             services, scope, expr.children[1], true)) {
        bool ignored_rhs = false;
        try {
          (void)evaluate_structured_bool_expression(
              services, scope, expr.children[1], ignored_rhs);
        } catch(const TemplateSubstitutionFailure &) {
        } catch(const SemanticSoftFailure &) {
        } catch(const SemanticDiagnosticError &) {
        } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        } catch(const logic_error &) {
        }
      }
      out = false;
      return NT_ARG_EVALUATED;
    }
    if(expr.simple_type == OP_LOR && lhs) {
      if(!expression_ast_mentions_template_dependency(
             services, scope, expr.children[1], true)) {
        bool ignored_rhs = false;
        try {
          (void)evaluate_structured_bool_expression(
              services, scope, expr.children[1], ignored_rhs);
        } catch(const TemplateSubstitutionFailure &) {
        } catch(const SemanticSoftFailure &) {
        } catch(const SemanticDiagnosticError &) {
        } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        } catch(const logic_error &) {
        }
      }
      out = true;
      return NT_ARG_EVALUATED;
    }
    bool rhs = false;
    const NonTypeArgumentStatus rhs_status =
        evaluate_structured_bool_expression(
            services, scope, expr.children[1], rhs);
    if(rhs_status == NT_ARG_EVALUATED) {
      out = rhs;
    }
    return rhs_status;
  }

  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token &&
     (expr.simple_type == OP_EQ || expr.simple_type == OP_NE) &&
     (expression_maybe_libcpp_tuple_size_value(expr.children[0]) ||
      expression_maybe_libcpp_tuple_size_value(expr.children[1]))) {
    const auto evaluate_integral_operand =
        [&](const CppAstNode & operand, long long & value) -> NonTypeArgumentStatus
    {
      const NonTypeArgumentStatus tuple_size_status =
          evaluate_libcpp_tuple_size_member_integral_value(
              services, scope, operand, value);
      if(tuple_size_status != NT_ARG_PARSE_FAILED) {
        return tuple_size_status;
      }
      if(try_evaluate_integral_expression_ast(
             services,
             scope.require(),
             operand,
             make_fundamental(FT_UNSIGNED_LONG_INT),
             value)) {
        return NT_ARG_EVALUATED;
      }
      return expression_ast_mentions_template_dependency(services, scope, operand, true) ?
          NT_ARG_DEPENDENT :
          NT_ARG_PARSE_FAILED;
    };

    long long lhs = 0;
    long long rhs = 0;
    const NonTypeArgumentStatus lhs_status =
        evaluate_integral_operand(expr.children[0], lhs);
    if(lhs_status != NT_ARG_EVALUATED) {
      return lhs_status;
    }
    const NonTypeArgumentStatus rhs_status =
        evaluate_integral_operand(expr.children[1], rhs);
    if(rhs_status != NT_ARG_EVALUATED) {
      return rhs_status;
    }
    out = expr.simple_type == OP_EQ ? lhs == rhs : lhs != rhs;
    return NT_ARG_EVALUATED;
  }

  if(expr.kind == CppAstKind::binary_expression &&
     expr.children.size() == 2 &&
     expr.has_token &&
     (expr.simple_type == OP_EQ || expr.simple_type == OP_NE)) {
    const auto evaluate_integral_operand =
        [&](const CppAstNode & operand, long long & value) -> NonTypeArgumentStatus
    {
      if(operand.kind == CppAstKind::id_expression) {
        bool structured_bool_value = false;
        const NonTypeArgumentStatus structured_bool_status =
            evaluate_structured_template_member_bool_value(
                services, scope, operand, structured_bool_value);
        if(structured_bool_status == NT_ARG_EVALUATED) {
          value = structured_bool_value ? 1 : 0;
          return NT_ARG_EVALUATED;
        }
        if(structured_bool_status == NT_ARG_DEPENDENT) {
          return structured_bool_status;
        }
        const NonTypeArgumentStatus member_status =
            evaluate_template_member_value_expression(
                services, scope, operand, value, TypePtr());
        if(member_status != NT_ARG_PARSE_FAILED) {
          return member_status;
        }
      }
      if(try_evaluate_integral_expression_ast(
             services,
             scope.require(),
             operand,
             TypePtr(),
             value)) {
        return NT_ARG_EVALUATED;
      }
      return expression_ast_mentions_template_dependency(services, scope, operand, true) ?
          NT_ARG_DEPENDENT :
          NT_ARG_PARSE_FAILED;
    };

    long long lhs = 0;
    long long rhs = 0;
    const NonTypeArgumentStatus lhs_status =
        evaluate_integral_operand(expr.children[0], lhs);
    if(lhs_status != NT_ARG_EVALUATED) {
      return lhs_status;
    }
    const NonTypeArgumentStatus rhs_status =
        evaluate_integral_operand(expr.children[1], rhs);
    if(rhs_status != NT_ARG_EVALUATED) {
      return rhs_status;
    }
    out = expr.simple_type == OP_EQ ? lhs == rhs : lhs != rhs;
    return NT_ARG_EVALUATED;
  }

  if(expr.kind == CppAstKind::unary_expression &&
     expr.children.size() == 1 &&
     expr.has_token &&
     expr.simple_type == OP_LNOT) {
    bool operand = false;
    const NonTypeArgumentStatus operand_status =
        evaluate_structured_bool_expression(
            services, scope, expr.children[0], operand);
    if(operand_status == NT_ARG_EVALUATED) {
      out = !operand;
    }
    return operand_status;
  }

  if(const CppAstNode * operand =
         structured_bool_functional_cast_operand(expr)) {
    bool operand_value = false;
    const NonTypeArgumentStatus operand_status =
        evaluate_structured_bool_expression(
            services, scope, *operand, operand_value);
    if(operand_status == NT_ARG_EVALUATED) {
      out = operand_value;
    }
    return operand_status;
  }

  if(expr.kind == CppAstKind::cast_expression &&
     expr.children.size() == 2 &&
     expr.children[0].kind == CppAstKind::type_id &&
     type_id_node_is_bool_type(services, scope, expr.children[0])) {
    bool operand = false;
    const NonTypeArgumentStatus operand_status =
        evaluate_structured_bool_expression(
            services, scope, expr.children[1], operand);
    if(operand_status == NT_ARG_EVALUATED) {
      out = operand;
    }
    return operand_status;
  }

  if(expr.kind == CppAstKind::parenthesized_expression &&
     expr.children.size() == 1) {
    return evaluate_structured_bool_expression(
        services, scope, expr.children[0], out);
  }

  long long integral = 0;
  try {
    if(try_evaluate_integral_expression_ast(
           services, scope.require(), expr, make_fundamental(FT_BOOL), integral)) {
      out = integral != 0;
      return NT_ARG_EVALUATED;
    }
  } catch(const ExplicitSpecializationAfterInstantiationError &) {
    throw;
  } catch(const logic_error &) {
  }

  if(expr.kind == CppAstKind::id_expression) {
    if(id_member_status_attempted) {
      return id_member_status;
    }
    return evaluate_structured_template_member_bool_value(
        services, scope, expr, out);
  }

  return expression_ast_mentions_template_dependency(services, scope, expr, true) ?
      NT_ARG_DEPENDENT :
      NT_ARG_PARSE_FAILED;
}

NonTypeArgumentStatus evaluate_template_member_value_text(
    SemanticContext & ctx,
    Scope & scope,
    const string & text,
    long long & value,
    const TypePtr & target_type)
{
  TemplateIdSyntax qualifier_template_id;
  string member_name;
  string display_name;
  if(!build_template_member_value_syntax(text,
                                         qualifier_template_id,
                                         member_name,
                                         display_name)) {
    return NT_ARG_PARSE_FAILED;
  }

  constant_eval::ConstexprValue constexpr_value;
  if(!ctx.lookup_constant_template_member_value(scope,
                                                qualifier_template_id,
                                                member_name,
                                                display_name,
                                                constexpr_value)) {
    return NT_ARG_EVAL_FAILED;
  }
  return constexpr_value_to_template_argument_integral(
             target_type, constexpr_value, value) ?
             NT_ARG_EVALUATED :
             NT_ARG_EVAL_FAILED;
}

void note_template_member_value_text_witness_if_needed(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & text,
    const TypePtr & target_type)
{
  if(!services.semantic_context ||
     !witness::source_capture_enabled(services.witness_context)) {
    return;
  }

  long long ignored_value = 0;
  try {
    if(default_template_argument_evaluation_active()) {
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      (void)evaluate_template_member_value_text(*services.semantic_context,
                                                scope,
                                                text,
                                                ignored_value,
                                                target_type);
      return;
    }
    (void)evaluate_template_member_value_text(*services.semantic_context,
                                              scope,
                                              text,
                                              ignored_value,
                                              target_type);
  } catch(const std::exception &) {
  }
}

void note_template_member_value_syntax_witness_if_needed(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateIdSyntax & qualifier_template_id,
    const string & member_name)
{
  if(!services.semantic_context ||
     !witness::source_capture_enabled(services.witness_context)) {
    return;
  }

  const string display_name =
      template_id_syntax_lookup_text(qualifier_template_id) + "::" + member_name;
  constant_eval::ConstexprValue ignored_value;
  try {
    if(default_template_argument_evaluation_active()) {
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      (void)services.semantic_context->lookup_constant_template_member_value(
          scope,
          qualifier_template_id,
          member_name,
          display_name,
          ignored_value);
      return;
    }
    (void)services.semantic_context->lookup_constant_template_member_value(
        scope,
        qualifier_template_id,
        member_name,
        display_name,
        ignored_value);
  } catch(const std::exception &) {
  }
}

void note_template_member_value_expression_witness_if_needed(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr)
{
  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(!qualified || qualified->qualifiers.empty()) {
    return;
  }

  for(size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    const TemplateIdSyntax * qualifier_template_id =
        cppast_qualifier_template_id_syntax(expr, qualifier_index);
    if(!qualifier_template_id) {
      continue;
    }
    note_template_member_value_syntax_witness_if_needed(
        services, scope.require(), *qualifier_template_id, qualified->name);
    return;
  }
}

NonTypeArgumentStatus evaluate_template_member_value_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text,
    long long & value,
    const TypePtr & target_type)
{
  Scope & raw_scope = scope.require();
  if(!services.semantic_context) {
    return NT_ARG_PARSE_FAILED;
  }

  TemplateIdSyntax qualifier_template_id;
  string member_name;
  string display_name;
  if(!build_template_member_value_syntax(text,
                                         qualifier_template_id,
                                         member_name,
                                         display_name)) {
    return NT_ARG_PARSE_FAILED;
  }

  constant_eval::ConstexprValue constexpr_value;
  if(!services.semantic_context->lookup_constant_template_member_value(
         raw_scope,
         qualifier_template_id,
         member_name,
         display_name,
         constexpr_value)) {
    return NT_ARG_EVAL_FAILED;
  }
  if(!constexpr_value_to_template_argument_integral(
         target_type, constexpr_value, value)) {
    return NT_ARG_EVAL_FAILED;
  }
  note_template_member_value_text_witness_if_needed(
      services, raw_scope, text, target_type);
  if(is_structured_bool_result_member_name(member_name)) {
    TypePtr owner_type;
    if(resolve_template_id_syntax_type(services,
                                       raw_scope,
                                       qualifier_template_id,
                                       false,
                                       string(),
                                       owner_type,
                                       scope) &&
       owner_type) {
      note_structured_bool_value_member_if_needed(services, scope, owner_type);
    }
  }
  return NT_ARG_EVALUATED;
}

NonTypeArgumentStatus evaluate_template_member_value_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    const TypePtr & target_type)
{
  if(expr.kind != CppAstKind::id_expression) {
    return NT_ARG_PARSE_FAILED;
  }

  Scope & raw_scope = scope.require();
  const QualifiedName * qualified = qualified_syntax_if_qualified(expr);
  if(!qualified) {
    return NT_ARG_PARSE_FAILED;
  }

  const ValueBinding * binding = nullptr;
  if(!lookup_leaf_qualified_value_binding(
         services, raw_scope, *qualified, &expr, binding) ||
     !binding) {
    return NT_ARG_PARSE_FAILED;
  }

  constant_eval::ConstexprValue constexpr_value;
  if(!materialize_leaf_member_constant_binding(
         services, *const_cast<ValueBinding *>(binding), constexpr_value)) {
    return binding->dependent_template_value ||
           service_type_depends_on_template_parameter(services, binding->type) ?
               NT_ARG_DEPENDENT :
               NT_ARG_EVAL_FAILED;
  }

  if(!constexpr_value_to_template_argument_integral(
         target_type, constexpr_value, value)) {
    return NT_ARG_EVAL_FAILED;
  }
  note_non_bool_static_value_dependency_for_witness(services, *binding);
  if(is_structured_bool_result_member_name(qualified->name)) {
    TypePtr owner_type =
        binding->owner_class ? binding->owner_class->type :
        (binding->declaration_scope && binding->declaration_scope->class_info ?
             binding->declaration_scope->class_info->type :
             TypePtr());
    if(owner_type) {
      note_structured_bool_value_member_if_needed(services, scope, owner_type);
    }
  }
  return NT_ARG_EVALUATED;
}

NonTypeArgumentStatus evaluate_non_type_argument_text(SemanticContext & ctx,
                                                      Scope & scope,
                                                      const string & text,
                                                      long long & value,
                                                      string * eval_error,
                                                      const TypePtr & target_type)
{
  const string trimmed = trim_space(text);
  if(try_evaluate_integral_text(trimmed, value)) {
    return NT_ARG_EVALUATED;
  }
  if(try_evaluate_integral_text_with_pack_scope(scope, trimmed, value)) {
    return NT_ARG_EVALUATED;
  }
  const auto evaluate_builtin_trait_text =
      [&]() -> NonTypeArgumentStatus
      {
        string builtin_name;
        vector<TypePtr> builtin_types;
        if(!ctx.try_parse_builtin_type_trait_text(scope, trimmed, builtin_name, builtin_types)) {
          return NT_ARG_PARSE_FAILED;
        }

        for(size_t i = 0; i < builtin_types.size(); ++i) {
          if(template_argument_semantics::type_depends_on_template_parameter(ctx, builtin_types[i])) {
            return NT_ARG_DEPENDENT;
          }
        }

        return evaluate_builtin_type_trait(ctx, scope, builtin_name, builtin_types, value) ?
                   NT_ARG_EVALUATED :
                   NT_ARG_PARSE_FAILED;
      };

  const NonTypeArgumentStatus builtin_status = evaluate_builtin_trait_text();
  if(builtin_status != NT_ARG_PARSE_FAILED) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "non-type-arg text=" << trimmed
                << " result="
                << (builtin_status == NT_ARG_DEPENDENT ? "dependent-builtin-trait" :
                    builtin_status == NT_ARG_EVALUATED ? "evaluated" :
                    "parse-failed");
          if(builtin_status == NT_ARG_EVALUATED) {
            trace << " value=" << value;
          }
        });
    return builtin_status;
  }

  const NonTypeArgumentStatus member_value_status =
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            return evaluate_template_member_value_text(
                services,
                template_api::make_template_environment(scope),
                trimmed,
                value,
                target_type);
          });
  if(member_value_status != NT_ARG_PARSE_FAILED) {
    return member_value_status;
  }

  (void)ctx;
  (void)scope;
  (void)value;
  (void)eval_error;
  (void)target_type;
  throw logic_error("legacy non-type template argument text evaluation: " + trimmed);
}

static bool expression_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    bool check_node_text);

bool unresolved_identifier_argument_may_depend_on_template_context(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text);

static bool ast_subtree_text_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  string node_dependency_text = dependency_check_text_for_ast_value(node);
  if(node_dependency_text.empty()) {
    node_dependency_text = node.value;
  }
  if(!node_dependency_text.empty() &&
     (text_mentions_template_placeholders(services, scope, node_dependency_text) ||
      text_mentions_dependent_non_namespace_binding_names(
          services, scope, node_dependency_text) ||
      unresolved_identifier_argument_may_depend_on_template_context(
          services, scope, node_dependency_text))) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_subtree_text_mentions_template_dependency(
           services, scope, node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool type_id_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  if(ast_has_parameter_pack_node(node)) {
    return true;
  }
  if(ast_subtree_text_mentions_template_dependency(services, scope, node)) {
    return true;
  }
  Scope & raw_scope = scope.require();
  TypePtr type;
  try {
    if(parse_type_id_node_for_templates(services, raw_scope, node, type, true) && type) {
      TypePtr resolved_type;
      if(service_type_depends_on_template_parameter(services, type) &&
         (!resolve_instantiated_dependent_type(services, scope, type, resolved_type) ||
          !resolved_type ||
          service_type_depends_on_template_parameter(services, resolved_type))) {
        return true;
      }
    } else if(scope_has_template_placeholders(services, scope)) {
      return true;
    }
  } catch(const DependentQualifiedTypeMissingTypenameError &) {
    const CppAstNode * type_name = single_type_name_from_type_id(node);
    const QualifiedName * qualified =
        type_name ? cppast_qualified_name_syntax(*type_name) : nullptr;
    if(qualified &&
       qualified->name == kStructuredBoolResultMemberName &&
       !qualified->qualifiers.empty()) {
      return true;
    }
    throw;
  }
  return false;
}

static bool template_id_syntax_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool check_node_text);

bool scope_contains_unqualified_identifier_binding(Scope & scope,
                                                   const string & name)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->named_types.find(name) != current->named_types.end() ||
       current->named_type_packs.find(name) != current->named_type_packs.end() ||
       current->named_pack_sizes.find(name) != current->named_pack_sizes.end() ||
       current->values.find(name) != current->values.end() ||
       current->named_value_packs.find(name) != current->named_value_packs.end() ||
       current->class_templates.find(name) != current->class_templates.end() ||
       current->alias_templates.find(name) != current->alias_templates.end() ||
       current->variable_templates.find(name) != current->variable_templates.end() ||
       current->namespace_bindings.find(name) != current->namespace_bindings.end() ||
       current->function_sets.find(name) != current->function_sets.end() ||
       current->function_templates.find(name) != current->function_templates.end()) {
      return true;
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool scope_contains_template_context_for_unresolved_identifier(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope)
{
  Scope & raw_scope = scope.require();
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  for(Scope * current = &raw_scope; current; current = current->parent) {
    if(!current->template_bound_type_names.empty() ||
       !current->template_bound_type_pack_names.empty() ||
       !current->template_bound_value_names.empty() ||
       !current->template_bound_template_names.empty()) {
      return true;
    }
    for(const auto & named : current->named_types) {
      if(type_is_dependent(named.second)) {
        return true;
      }
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool unresolved_identifier_argument_may_depend_on_template_context(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text)
{
  const string trimmed = trim_space(text);
  if(!is_identifier_text(trimmed) ||
     !scope_contains_template_context_for_unresolved_identifier(services, scope)) {
    return false;
  }
  if(trimmed == "true" ||
     trimmed == "false" ||
     trimmed == "nullptr") {
    return false;
  }
  return !scope_contains_unqualified_identifier_binding(scope.require(), trimmed);
}

static bool template_argument_syntax_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    bool check_node_text)
{
  if(syntax.pack_expansion) {
    return true;
  }
  if(syntax.dependent) {
    return true;
  }
  if(syntax.type_id &&
     type_id_ast_mentions_template_dependency(services, scope, *syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     expression_ast_mentions_template_dependency(
         services, scope, *syntax.expression, check_node_text)) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_mentions_template_dependency(
         services, scope, *syntax.template_id, check_node_text)) {
    return true;
  }
  if(check_node_text &&
     !syntax.text.empty() &&
     (text_mentions_template_placeholders(services, scope, syntax.text) ||
      text_mentions_dependent_non_namespace_binding_names(
          services, scope, syntax.text) ||
      unresolved_identifier_argument_may_depend_on_template_context(
          services, scope, syntax.text))) {
    return true;
  }
  return false;
}

static bool template_id_syntax_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateIdSyntax & syntax,
    bool check_node_text)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(template_argument_syntax_mentions_template_dependency(
           services, scope, syntax.argument_syntaxes[i], check_node_text)) {
      return true;
    }
  }
  if(check_node_text) {
    for(size_t i = syntax.argument_syntaxes.size(); i < syntax.arguments.size(); ++i) {
      if(text_mentions_template_placeholders(services, scope, syntax.arguments[i]) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, scope, syntax.arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

static bool expression_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node,
    bool check_node_text)
{
  if(ast_has_parameter_pack_node(node)) {
    return true;
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    if(template_id_syntax_mentions_template_dependency(
           services, scope, *template_id, check_node_text)) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_mentions_template_dependency(
           services,
           scope,
           node.qualifier_template_id_syntaxes[i],
           check_node_text)) {
      return true;
    }
  }
  if(node.kind == CppAstKind::type_id &&
     type_id_ast_mentions_template_dependency(services, scope, node)) {
    return true;
  }
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  const string node_dependency_text =
      check_node_text ? dependency_check_text_for_ast_value(node) : string();
  if(!node_dependency_text.empty()) {
    if(text_mentions_template_placeholders(services, scope, node_dependency_text) ||
       text_mentions_dependent_non_namespace_binding_names(
           services, scope, node_dependency_text) ||
       unresolved_identifier_argument_may_depend_on_template_context(
           services, scope, node_dependency_text)) {
      return true;
    }
  }
  if(node.semantic_type &&
     type_is_dependent(node.semantic_type)) {
    TypePtr resolved_semantic_type;
    if(resolve_instantiated_dependent_type(
           services,
           scope,
           node.semantic_type,
           resolved_semantic_type) &&
       resolved_semantic_type &&
       !type_is_dependent(resolved_semantic_type)) {
      // The declaration AST can retain placeholder semantic types even after
      // the current instantiation scope has concrete bindings. Treat the node
      // itself as non-dependent, but still inspect its children below.
    } else {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(expression_ast_mentions_template_dependency(
           services,
           scope,
           node.children[i],
           check_node_text)) {
      return true;
    }
  }
  return false;
}

static bool comma_prefix_ast_mentions_template_dependency(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  if(node.kind != CppAstKind::binary_expression ||
     !node.has_token ||
     node.simple_type != OP_COMMA ||
     node.children.size() != 2) {
    return false;
  }
  return expression_ast_mentions_template_dependency(
             services, scope, node.children[0], false) ||
         comma_prefix_ast_mentions_template_dependency(
             services, scope, node.children[1]);
}

static const CppAstNode * decltype_or_typeof_operand_node(const CppAstNode & node)
{
  if(node.kind == CppAstKind::decltype_specifier) {
    return node.children.empty() ? nullptr : &node.children[0];
  }
  if(node.kind != CppAstKind::decl_specifier &&
     node.kind != CppAstKind::type_specifier) {
    return nullptr;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::decltype_specifier) {
      return node.children[i].children.empty() ? nullptr : &node.children[i].children[0];
    }
  }
  return node.children.empty() ? nullptr : &node.children[0];
}

static void note_leaf_constant_values_in_expression_ast(
    template_api::TemplateServices & services,
    Scope & raw_scope,
    const CppAstNode & node)
{
  if(node.kind == CppAstKind::binary_expression &&
     node.children.size() == 2 &&
     node.has_token &&
     (node.simple_type == OP_LAND || node.simple_type == OP_LOR)) {
    note_leaf_constant_values_in_expression_ast(services, raw_scope, node.children[0]);

    constant_eval::ConstexprValue lhs;
    bool lhs_truthy = false;
    template_api::TemplateConstantEvaluationRequest request;
    request.scope = &raw_scope;
    request.expr = node.children[0];
    request.target_type = make_fundamental(FT_BOOL);
    if(((evaluate_constant_expression_leaf_impl(services,
                                                raw_scope,
                                                node.children[0],
                                                lhs,
                                                request.target_type) &&
         constant_eval::constexpr_value_truthy(lhs, lhs_truthy)) ||
        (service_evaluate_initializer_constant_value(services, request, lhs) &&
         constant_eval::constexpr_value_truthy(lhs, lhs_truthy)))) {
      if((node.simple_type == OP_LAND && !lhs_truthy) ||
         (node.simple_type == OP_LOR && lhs_truthy)) {
        if(!scope_has_template_placeholders(
               services, template_api::make_template_environment(raw_scope))) {
          try {
            note_leaf_constant_values_in_expression_ast(services, raw_scope, node.children[1]);
          } catch(const semantic_fallback_audit::SemanticFallbackError &) {
            // Reference discovery should not make unreachable invalid operands fatal.
          }
        }
        return;
      }
    }

    note_leaf_constant_values_in_expression_ast(services, raw_scope, node.children[1]);
    return;
  }

  if(node.kind == CppAstKind::id_expression) {
    constant_eval::ConstexprValue ignored;
    try {
      const bool found =
          lookup_leaf_constant_value(raw_scope, services, node.value, &node, ignored);
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "leaf-constant-discovery name=" << node.value
              << " found=" << (found ? "yes" : "no")
              << " scope=" << raw_scope.name;
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
    } catch(const logic_error &) {
      // This traversal mirrors semantic reference discovery, not constexpr
      // viability. Invalid unevaluated or short-circuited leaves stay non-fatal.
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      // Strict fallback auditing should not make reference discovery more fatal
      // than the constexpr viability pass it follows.
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    note_leaf_constant_values_in_expression_ast(services, raw_scope, node.children[i]);
  }
}

static bool should_track_nested_member_class_type_for_witness(
    const ClassInfo * info)
{
  if(!info ||
     info->source_template ||
     !info->enclosing_scope ||
     !info->enclosing_scope->class_info) {
    return false;
  }
  return template_api::class_has_template_identity(info) &&
         template_api::class_has_template_identity(info->enclosing_scope->class_info);
}

static void note_nested_member_class_type_for_witness(
    template_api::TemplateServices & services,
    ClassInfo & info,
    set<const ClassInfo *> & noted)
{
  if(!services.semantic_context ||
     !should_track_nested_member_class_type_for_witness(&info) ||
     !noted.insert(&info).second) {
    return;
  }

  string decl_location = strip_template_location_at_prefix(
      semantic_model::source_decl_anchor_location(
          semantic_trace::class_decl_anchor(*services.semantic_context, &info)));
  if(decl_location.empty() && info.class_node) {
    decl_location = strip_template_location_at_prefix(
        services.semantic_context->source_location_for_node(*info.class_node));
  }
  if(decl_location.empty()) {
    return;
  }

  const ScopedTemplateValueDependencyLifecycleResume lifecycle_resume;
  template_api::note_nested_member_class_track_instantiation(
      *services.semantic_context,
      info,
      decl_location);
}

static void note_nested_member_class_types_in_expression_ast(
    template_api::TemplateServices & services,
    Scope & raw_scope,
    const CppAstNode & node,
    set<const ClassInfo *> & noted)
{
  if(node.kind == CppAstKind::type_id && services.semantic_context) {
    TypePtr type;
    try {
      if(services.semantic_context->parse_type_id(raw_scope, node, type, true) &&
         type) {
        if(ClassInfo * info =
               template_api::find_named_type_class_info(
                   service_type_system(services).model,
                   type)) {
          note_nested_member_class_type_for_witness(services, *info, noted);
        }
      }
    } catch(const TemplateSubstitutionFailure &) {
    } catch(const SemanticSoftFailure &) {
    } catch(const SemanticDiagnosticError &) {
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    } catch(const logic_error &) {
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    note_nested_member_class_types_in_expression_ast(services,
                                                     raw_scope,
                                                     node.children[i],
                                                     noted);
  }
}

bool note_constant_value_member_instantiations_in_expression(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & expr)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return false;
  }
  const witness::ScopedTemplateWitnessSourceCapturePause source_capture_pause;
  note_leaf_constant_values_in_expression_ast(services, scope, expr);
  set<const ClassInfo *> noted_nested_member_classes;
  note_nested_member_class_types_in_expression_ast(services,
                                                  scope,
                                                  expr,
                                                  noted_nested_member_classes);
  return !noted_nested_member_classes.empty();
}

static void note_structured_bool_value_members_in_expression_ast(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }

  if(node.kind == CppAstKind::binary_expression &&
     node.children.size() == 2 &&
     node.has_token &&
     (node.simple_type == OP_LAND || node.simple_type == OP_LOR)) {
    note_structured_bool_value_members_in_expression_ast(
        services, scope, node.children[0]);
    bool lhs = false;
    bool lhs_evaluated = false;
    try {
      lhs_evaluated =
          evaluate_structured_bool_expression(
              services, scope, node.children[0], lhs) == NT_ARG_EVALUATED;
    } catch(const TemplateSubstitutionFailure &) {
    } catch(const SemanticSoftFailure &) {
    } catch(const SemanticDiagnosticError &) {
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    } catch(const logic_error &) {
    }
    if(lhs_evaluated &&
       ((node.simple_type == OP_LAND && !lhs) ||
        (node.simple_type == OP_LOR && lhs))) {
      if(!expression_ast_mentions_template_dependency(
             services, scope, node.children[1], false)) {
        note_structured_bool_value_members_in_expression_ast(
            services, scope, node.children[1]);
      }
      return;
    }
    note_structured_bool_value_members_in_expression_ast(
        services, scope, node.children[1]);
    return;
  }

  const witness::ScopedTemplateWitnessSourceCapturePause pause;
  try {
    bool ignored = false;
    if(node.kind == CppAstKind::id_expression) {
      (void)evaluate_structured_template_member_bool_value(
          services, scope, node, ignored);
      vector<TemplateValueDependency> dependencies;
      append_structured_bool_value_dependencies_for_qualified_value(
          services,
          scope,
          node,
          dependencies);
      note_template_value_dependencies_for_witness(*services.semantic_context,
                                                   dependencies);
    } else if(node.kind == CppAstKind::type_id) {
      (void)evaluate_structured_template_member_bool_type_id(
          services, scope, node, ignored);
      vector<TemplateValueDependency> dependencies;
      append_structured_bool_value_dependencies_in_expression_ast_impl(
          services,
          scope,
          node,
          dependencies);
      note_template_value_dependencies_for_witness(*services.semantic_context,
                                                   dependencies);
    }
  } catch(const TemplateSubstitutionFailure &) {
  } catch(const SemanticSoftFailure &) {
  } catch(const SemanticDiagnosticError &) {
  } catch(const semantic_fallback_audit::SemanticFallbackError &) {
  } catch(const logic_error &) {
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    note_structured_bool_value_members_in_expression_ast(
        services, scope, node.children[i]);
  }
}

static void note_constexpr_function_calls_in_expression_ast(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr ||
     !scope.valid()) {
    return;
  }

  if(node.kind == CppAstKind::call_expression) {
    const bool dependent =
        expression_ast_mentions_template_dependency(services, scope, node, false);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "constexpr-call-discovery text=" << node_text(node)
            << " dependent=" << (dependent ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    const witness::ScopedTemplateWitnessSourceCapturePause source_capture_pause;
    try {
      (void)services.semantic_context->analyze_expression(scope.require(), node);
    } catch(const TemplateSubstitutionFailure &) {
    } catch(const SemanticSoftFailure &) {
    } catch(const SemanticDiagnosticError &) {
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
    } catch(const logic_error &) {
    }
  }

  for(size_t i = 0; i < node.children.size(); ++i) {
    note_constexpr_function_calls_in_expression_ast(services,
                                                   scope,
                                                   node.children[i]);
  }
}

void note_structured_bool_value_members_in_template_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }

  TemplateArgumentSyntax expanded_syntax;
  const TemplateArgumentSyntax * effective_syntax = &syntax;
  if(syntax.text.find("...") != string::npos ||
     syntax.pack_expansion ||
     syntax.template_id ||
     syntax.type_id ||
     syntax.expression) {
    expanded_syntax = clone_argument_syntax_for_template_substitution(syntax);
    if(expand_bound_packs_in_argument_syntax(
           services, scope.require(), expanded_syntax)) {
      effective_syntax = &expanded_syntax;
    }
  }
  if(template_argument_syntax_has_template_dependency(services,
                                                     scope,
                                                     *effective_syntax)) {
    return;
  }

  if(effective_syntax->expression) {
    note_structured_bool_value_members_in_expression_ast(
        services, scope, *effective_syntax->expression);
  }
  if(effective_syntax->type_id) {
    note_structured_bool_value_members_in_expression_ast(
        services, scope, *effective_syntax->type_id);
  }
  if(effective_syntax->template_id) {
    for(size_t i = 0;
        i < effective_syntax->template_id->argument_syntaxes.size();
        ++i) {
      note_structured_bool_value_members_in_template_argument_syntax(
          services,
          scope,
          effective_syntax->template_id->argument_syntaxes[i]);
    }
  }
}

void note_structured_bool_value_members_in_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const vector<TemplateArgument> & arguments)
{
  if(!services.semantic_context ||
     services.witness_context.session == nullptr) {
    return;
  }
  for(size_t i = 0; i < arguments.size(); ++i) {
    const TemplateArgument & argument = arguments[i];
    if(argument.kind != TemplateArgument::TA_VALUE ||
       (argument.type &&
        !is_bool_type(strip_top_level_cv(remove_reference_type(argument.type))))) {
      continue;
    }
    for(size_t j = 0; j < argument.value_dependencies.size(); ++j) {
      note_template_value_dependency_for_witness(argument.value_dependencies[j]);
    }
    if(argument.expression) {
      note_structured_bool_value_members_in_expression_ast(
          services, scope, *argument.expression);
    }
    if(!argument.source_syntax) {
      continue;
    }
    if(argument.source_syntax->expression) {
      note_structured_bool_value_members_in_expression_ast(
          services, scope, *argument.source_syntax->expression);
    }
    if(argument.source_syntax->type_id) {
      note_structured_bool_value_members_in_expression_ast(
          services, scope, *argument.source_syntax->type_id);
    }
  }
}

NonTypeArgumentStatus evaluate_non_type_argument_expression(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & expr,
    long long & value,
    string * eval_error,
    const TypePtr & target_type)
{
  Scope & raw_scope = scope.require();
  std::string expression_order_use_location;
  if(services.semantic_context) {
    expression_order_use_location =
        template_api::normalize_template_witness_source_location(
            services.semantic_context->source_location_for_node(expr));
  }
  const parser_trace::ScopedOrderUseLocation expression_order_use_location_guard(
      expression_order_use_location);
  NonTypeArgumentStatus status = NT_ARG_EVAL_FAILED;
  bool skip_leaf_constant_discovery = false;
  const bool structured_bool_shortcut_allowed =
      target_type && is_bool_type(remove_reference_type(target_type));
	  const auto evaluate_structured_bool_shortcut =
	      [&](const CppAstNode & candidate, bool & bool_value) -> NonTypeArgumentStatus
	  {
	    const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
	        function_call_source_capture_pause;
	    return evaluate_structured_bool_expression(
	        services, scope, candidate, bool_value);
	  };
  try {
    if(structured_bool_shortcut_allowed &&
       structured_bool_expression_precheck_allowed(services, scope, expr)) {
      bool structured_bool_value = false;
      const NonTypeArgumentStatus structured_status =
          evaluate_structured_bool_shortcut(expr, structured_bool_value);
      if(structured_status == NT_ARG_EVALUATED) {
        value = structured_bool_value ? 1 : 0;
        status = NT_ARG_EVALUATED;
        skip_leaf_constant_discovery =
            structured_template_member_bool_value_is_pure_trait(
                services, scope, expr);
      } else if(structured_status == NT_ARG_DEPENDENT) {
        status = NT_ARG_DEPENDENT;
      } else if(structured_status == NT_ARG_EVAL_FAILED) {
        status = NT_ARG_EVAL_FAILED;
        skip_leaf_constant_discovery = true;
      }
    }
    if(status != NT_ARG_EVALUATED &&
       status != NT_ARG_DEPENDENT &&
       expr.kind == CppAstKind::id_expression) {
      const NonTypeArgumentStatus syntax_member_status =
          evaluate_template_member_value_expression(
              services,
              scope,
              expr,
              value,
              target_type);
      if(syntax_member_status == NT_ARG_EVALUATED ||
         syntax_member_status == NT_ARG_DEPENDENT) {
        status = syntax_member_status;
      }
    }
    if(status != NT_ARG_EVALUATED &&
       status != NT_ARG_DEPENDENT &&
       expr.kind == CppAstKind::id_expression) {
      const string expr_text =
          trim_space(callsemantic_internal::describe_expression_for_diagnostic(expr));
      if(expr_text.find("::") != string::npos) {
        const NonTypeArgumentStatus member_status =
            evaluate_template_member_value_text(
                services,
                scope,
                expr_text,
                value,
                target_type);
        if(member_status == NT_ARG_EVALUATED ||
           member_status == NT_ARG_DEPENDENT) {
          if(member_status == NT_ARG_EVALUATED) {
            note_template_member_value_expression_witness_if_needed(
                services, scope, expr);
          }
          status = member_status;
        }
      }
    }
    constant_eval::ConstexprValue constexpr_value;
    if(status != NT_ARG_EVALUATED &&
       status != NT_ARG_DEPENDENT &&
       evaluate_constant_expression_leaf_impl(
           services, raw_scope, expr, constexpr_value, target_type) &&
       constexpr_value_to_template_argument_integral(target_type, constexpr_value, value)) {
      status = NT_ARG_EVALUATED;
    }
    if(status != NT_ARG_EVALUATED &&
       structured_bool_shortcut_allowed &&
       expr.kind == CppAstKind::id_expression) {
      bool structured_bool_value = false;
      const NonTypeArgumentStatus structured_status =
          evaluate_structured_bool_shortcut(expr, structured_bool_value);
      if(structured_status == NT_ARG_EVALUATED) {
        value = structured_bool_value ? 1 : 0;
        status = NT_ARG_EVALUATED;
        skip_leaf_constant_discovery =
            structured_template_member_bool_value_is_pure_trait(
                services, scope, expr);
      }
    }
    template_api::TemplateConstantEvaluationRequest request;
    request.scope = &raw_scope;
    request.expr = expr;
    request.target_type = target_type;
    if(status != NT_ARG_EVALUATED &&
       status != NT_ARG_DEPENDENT) {
      bool initializer_evaluated = false;
      {
        const witness::ScopedTemplateWitnessSourceCapturePause source_capture_pause(
            witness::enabled(services.witness_context));
        initializer_evaluated =
            service_evaluate_initializer_constant_value(services,
                                                        request,
                                                        constexpr_value);
      }
      if(initializer_evaluated &&
         constexpr_value_to_template_argument_integral(target_type,
                                                       constexpr_value,
                                                       value)) {
        status = NT_ARG_EVALUATED;
      }
    }
    if(status != NT_ARG_EVALUATED) {
      bool structured_bool_value = false;
      const NonTypeArgumentStatus structured_status =
          structured_bool_shortcut_allowed ?
              evaluate_structured_bool_shortcut(expr, structured_bool_value) :
              NT_ARG_PARSE_FAILED;
      if(structured_status == NT_ARG_EVALUATED) {
        value = structured_bool_value ? 1 : 0;
        status = NT_ARG_EVALUATED;
        skip_leaf_constant_discovery =
            structured_template_member_bool_value_is_pure_trait(
                services, scope, expr);
      } else if(structured_status == NT_ARG_DEPENDENT) {
        status = NT_ARG_DEPENDENT;
      } else if(structured_status == NT_ARG_EVAL_FAILED) {
        status = NT_ARG_EVAL_FAILED;
        skip_leaf_constant_discovery = true;
      }
    }
    if(status != NT_ARG_EVALUATED && status != NT_ARG_DEPENDENT) {
      const string expr_text =
          trim_space(callsemantic_internal::describe_expression_for_diagnostic(expr));
      if(try_evaluate_integral_text(expr_text, value)) {
        status = NT_ARG_EVALUATED;
      }
    }
  } catch(const ExplicitSpecializationAfterInstantiationError &) {
    throw;
  } catch(const semantic_fallback_audit::SemanticFallbackError & e) {
    if(eval_error) {
      *eval_error = e.what();
    }
    status = NT_ARG_EVAL_FAILED;
  } catch(const logic_error & e) {
    const string expr_text =
        trim_space(callsemantic_internal::describe_expression_for_diagnostic(expr));
    if(try_evaluate_integral_text(expr_text, value)) {
      status = NT_ARG_EVALUATED;
    } else {
      bool structured_bool_value = false;
      const NonTypeArgumentStatus structured_status =
          structured_bool_shortcut_allowed ?
              evaluate_structured_bool_shortcut(expr, structured_bool_value) :
              NT_ARG_PARSE_FAILED;
      if(structured_status == NT_ARG_EVALUATED) {
        value = structured_bool_value ? 1 : 0;
        status = NT_ARG_EVALUATED;
        skip_leaf_constant_discovery =
            structured_template_member_bool_value_is_pure_trait(
                services, scope, expr);
      } else if(structured_status == NT_ARG_DEPENDENT) {
        status = NT_ARG_DEPENDENT;
      } else if(structured_status == NT_ARG_EVAL_FAILED) {
        status = NT_ARG_EVAL_FAILED;
        skip_leaf_constant_discovery = true;
      } else {
        if(eval_error) {
          *eval_error = e.what();
        }
        status = NT_ARG_EVAL_FAILED;
      }
    }
  }

  if(status == NT_ARG_EVALUATED) {
    if(!encode_data_member_pointer_template_argument_if_needed(
           target_type, expr, value)) {
      if(eval_error) {
        *eval_error = "data member pointer template argument offset overflow";
      }
      return NT_ARG_EVAL_FAILED;
    }
    if(skip_leaf_constant_discovery) {
      note_structured_bool_value_members_in_expression_ast(
          services, scope, expr);
    } else {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "non-type-expression-discovery text=" << node_text(expr)
              << " kind=" << cppast_kind_text(expr.kind)
              << " skip-leaf=no";
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      note_leaf_constant_values_in_expression_ast(services, raw_scope, expr);
    }
    note_constexpr_function_calls_in_expression_ast(services, scope, expr);
    return status;
  }

  if(skip_leaf_constant_discovery) {
    return status;
  }

  if(expression_ast_mentions_template_dependency(services, scope, expr, true)) {
    return NT_ARG_DEPENDENT;
  }

  note_leaf_constant_values_in_expression_ast(services, raw_scope, expr);
  return status;
}

NonTypeArgumentStatus evaluate_non_type_argument_syntax(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax & syntax,
    long long & value,
    string * eval_error,
    const TypePtr & target_type)
{
  TemplateArgumentSyntax expanded_syntax;
  const TemplateArgumentSyntax * effective_syntax = &syntax;
  if(syntax.text.find("...") != string::npos ||
     syntax.pack_expansion ||
     syntax.template_id ||
     syntax.type_id ||
     syntax.expression) {
    expanded_syntax = clone_argument_syntax_for_template_substitution(syntax);
    bool expanded_changed = false;
    if(expand_bound_packs_in_argument_syntax(
           services, scope.require(), expanded_syntax)) {
      expanded_changed = true;
    }
    if(substitute_bound_replacements_in_argument_syntax(scope.require(),
                                                        expanded_syntax)) {
      expanded_changed = true;
    }
    if(expanded_changed) {
      effective_syntax = &expanded_syntax;
    }
  }

  const bool structured_bool_shortcut_allowed =
      target_type && is_bool_type(remove_reference_type(target_type));
  if(structured_bool_shortcut_allowed) {
    bool bool_value = false;
    NonTypeArgumentStatus structured_status = NT_ARG_PARSE_FAILED;
    {
      const witness::ScopedTemplateWitnessFunctionCallSourceCapturePause
          source_capture_pause;
      try {
        structured_status =
            evaluate_structured_bool_template_argument(
                services, scope, *effective_syntax, bool_value);
      } catch(const DependentQualifiedTypeMissingTypenameError &) {
        structured_status = NT_ARG_DEPENDENT;
      }
    }
    if(structured_status == NT_ARG_EVALUATED) {
      value = bool_value ? 1 : 0;
    }
    if(structured_status == NT_ARG_EVAL_FAILED &&
       effective_syntax->expression &&
       effective_syntax->expression->kind == CppAstKind::id_expression) {
      long long member_value = 0;
      NonTypeArgumentStatus member_status = NT_ARG_PARSE_FAILED;
      try {
        member_status =
            evaluate_template_member_value_expression(
                services,
                scope,
                *effective_syntax->expression,
                member_value,
                target_type);
      } catch(const TemplateSubstitutionFailure &) {
        member_status = NT_ARG_EVAL_FAILED;
      } catch(const SemanticSoftFailure &) {
        member_status = NT_ARG_EVAL_FAILED;
      } catch(const SemanticDiagnosticError &) {
        member_status = NT_ARG_EVAL_FAILED;
      } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        member_status = NT_ARG_EVAL_FAILED;
      }
      if(member_status == NT_ARG_EVALUATED) {
        value = member_value;
        return member_status;
      }
      if(member_status == NT_ARG_DEPENDENT) {
        return member_status;
      }
    }
    if(structured_status != NT_ARG_PARSE_FAILED) {
      if(structured_status == NT_ARG_EVALUATED) {
        if(effective_syntax->expression) {
          note_structured_bool_value_members_in_expression_ast(
              services, scope, *effective_syntax->expression);
          note_constexpr_function_calls_in_expression_ast(
              services, scope, *effective_syntax->expression);
        } else if(effective_syntax->type_id) {
          note_structured_bool_value_members_in_expression_ast(
              services, scope, *effective_syntax->type_id);
          note_constexpr_function_calls_in_expression_ast(
              services, scope, *effective_syntax->type_id);
        }
      }
      return structured_status;
    }
  }

  if(effective_syntax->expression) {
    return evaluate_non_type_argument_expression(
        services,
        scope,
        *effective_syntax->expression,
        value,
        eval_error,
        target_type);
  }

  return effective_syntax->dependent || effective_syntax->pack_expansion ?
      NT_ARG_DEPENDENT :
      NT_ARG_PARSE_FAILED;
}

NonTypeArgumentStatus evaluate_non_type_argument_text(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const string & text,
    long long & value,
    string * eval_error,
    const TypePtr & target_type)
{
  Scope & raw_scope = scope.require();
  const string trimmed = trim_space(text);
  if(try_evaluate_integral_text(trimmed, value)) {
    return NT_ARG_EVALUATED;
  }
  if(try_evaluate_integral_text_with_pack_scope(raw_scope, trimmed, value)) {
    return NT_ARG_EVALUATED;
  }
  const auto evaluate_builtin_trait_text =
      [&]() -> NonTypeArgumentStatus
      {
        string builtin_name;
        vector<TypePtr> builtin_types;
        if(!try_parse_builtin_type_trait_text(
               services, raw_scope, trimmed, builtin_name, builtin_types)) {
          return NT_ARG_PARSE_FAILED;
        }

        for(size_t i = 0; i < builtin_types.size(); ++i) {
          if(service_type_depends_on_template_parameter(services, builtin_types[i])) {
            return NT_ARG_DEPENDENT;
          }
        }

        return evaluate_builtin_type_trait(
                   services, raw_scope, builtin_name, builtin_types, value) ?
                   NT_ARG_EVALUATED :
                   NT_ARG_PARSE_FAILED;
      };

  const NonTypeArgumentStatus builtin_status = evaluate_builtin_trait_text();
  if(builtin_status != NT_ARG_PARSE_FAILED) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "non-type-arg text=" << trimmed
                << " result="
                << (builtin_status == NT_ARG_DEPENDENT ? "dependent-builtin-trait" :
                    builtin_status == NT_ARG_EVALUATED ? "evaluated" :
                    "parse-failed");
          if(builtin_status == NT_ARG_EVALUATED) {
            trace << " value=" << value;
          }
        });
    return builtin_status;
  }

  const NonTypeArgumentStatus member_value_status =
      evaluate_template_member_value_text(
          services, scope, trimmed, value, target_type);
  if(member_value_status != NT_ARG_PARSE_FAILED) {
    return member_value_status;
  }

  throw logic_error("legacy non-type template argument text evaluation: " + trimmed);
}

bool evaluate_constant_expression_leaf(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const CppAstNode & node,
                                       constant_eval::ConstexprValue & out,
                                       const TypePtr & target_type)
{
  return evaluate_constant_expression_leaf_impl(services, scope, node, out, target_type);
}

bool evaluate_dependent_type_expression_leaf(
    template_api::TemplateServices & services,
    Scope & scope,
    const template_api::TemplateDependentTypeExprRequest & request,
    TypePtr & out)
{
  out.reset();
  if(request.operand.kind == CppAstKind::member_expression) {
    if(request.kind == template_api::TDTEK_DECLTYPE &&
       request.operand_was_parenthesized) {
      return false;
    }
    return lookup_leaf_operand_type(services, scope, request.operand, out);
  }

  semantic_conversion::ValueCategory category = semantic_conversion::VC_PRVALUE;
  if(!lookup_leaf_expression_type_category(
         services, scope, request.operand, out, category) ||
     !out) {
    return false;
  }

  if(request.kind == template_api::TDTEK_TYPEOF_EXPR) {
    return true;
  }

  if(request.operand.kind == CppAstKind::id_expression ||
     request.operand.kind == CppAstKind::member_expression) {
    if(request.operand_was_parenthesized && category == semantic_conversion::VC_LVALUE) {
      out = make_lvalue_reference_raw(out);
    }
    return out != nullptr;
  }

  if(category == semantic_conversion::VC_LVALUE) {
    out = collapse_lvalue_reference_type(out);
  } else if(category == semantic_conversion::VC_XVALUE) {
    out = collapse_rvalue_reference_type(out);
  }
  return out != nullptr;
}

bool split_top_level_member_expression_text(const string & text,
                                            string & object_text,
                                            string & member_name,
                                            bool & arrow)
{
  const string trimmed = trim_space(text);
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
  size_t split = string::npos;
  bool split_arrow = false;
  for(size_t i = 0; i < trimmed.size(); ++i) {
    const char ch = trimmed[i];
    if(ch == '<') {
      ++angle_depth;
    } else if(ch == '>') {
      if(angle_depth > 0) {
        --angle_depth;
      }
    } else if(ch == '(') {
      ++paren_depth;
    } else if(ch == ')') {
      if(paren_depth > 0) {
        --paren_depth;
      }
    } else if(ch == '[') {
      ++bracket_depth;
    } else if(ch == ']') {
      if(bracket_depth > 0) {
        --bracket_depth;
      }
    } else if(ch == '{') {
      ++brace_depth;
    } else if(ch == '}') {
      if(brace_depth > 0) {
        --brace_depth;
      }
    }
    if(angle_depth != 0 || paren_depth != 0 ||
       bracket_depth != 0 || brace_depth != 0) {
      continue;
    }
    if(ch == '.') {
      split = i;
      split_arrow = false;
    } else if(ch == '-' && i + 1 < trimmed.size() && trimmed[i + 1] == '>') {
      split = i;
      split_arrow = true;
      ++i;
    }
  }
  if(split == string::npos) {
    return false;
  }
  object_text = trim_space(trimmed.substr(0, split));
  member_name = trim_space(trimmed.substr(split + (split_arrow ? 2 : 1)));
  arrow = split_arrow;
  return !object_text.empty() && is_identifier_text(member_name);
}

bool function_type_call_result(const TypePtr & function_type,
                               const vector<pair<TypePtr, semantic_conversion::ValueCategory> > & arg_infos,
                               TypePtr & out,
                               semantic_conversion::ValueCategory & category)
{
  TypePtr stripped = strip_top_level_cv(function_type);
  if(!stripped ||
     stripped->kind != Type::TK_FUNCTION ||
     !stripped->inner) {
    return false;
  }
  if(!stripped->variadic && arg_infos.size() != stripped->params.size()) {
    return false;
  }
  if(arg_infos.size() < stripped->params.size()) {
    return false;
  }
  for(size_t i = 0; i < stripped->params.size(); ++i) {
    semantic_conversion::ExprInfo expr_info;
    expr_info.type = arg_infos[i].first;
    expr_info.category = arg_infos[i].second;
    if(semantic_conversion::standard_conversion_rank(stripped->params[i], expr_info) ==
       semantic_conversion::CR_BAD) {
      return false;
    }
  }
  return semantic_conversion::result_value_category_for_function_result(
             stripped->inner, category) &&
         (out = stripped->inner, out != nullptr);
}

bool function_type_result(const TypePtr & function_type,
                          TypePtr & out,
                          semantic_conversion::ValueCategory & category)
{
  TypePtr stripped = strip_top_level_cv(function_type);
  if(!stripped ||
     stripped->kind != Type::TK_FUNCTION ||
     !stripped->inner) {
    return false;
  }
  return semantic_conversion::result_value_category_for_function_result(
             stripped->inner, category) &&
         (out = stripped->inner, out != nullptr);
}

bool evaluate_declval_expression_type_category_text(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & text,
    TypePtr & out,
    semantic_conversion::ValueCategory & category)
{
  out.reset();
  category = semantic_conversion::VC_PRVALUE;
  const string trimmed = trim_space(text);

  if(has_top_level_comma(trimmed)) {
    const vector<string> parts = split_comma_list(trimmed);
    if(parts.empty()) {
      return false;
    }
    for(size_t i = 0; i < parts.size(); ++i) {
      TypePtr part_type;
      semantic_conversion::ValueCategory part_category =
          semantic_conversion::VC_PRVALUE;
      if(!evaluate_declval_expression_type_category_text(
             services, scope, parts[i], part_type, part_category) ||
         !part_type) {
        return false;
      }
      if(i + 1 == parts.size()) {
        out = part_type;
        category = part_category;
      }
    }
    return out != nullptr;
  }

  string callee_text;
  string arg_text;
  if(!split_top_level_call_expression_text(trimmed, callee_text, arg_text)) {
    const ValueBinding * binding = nullptr;
    if(lookup_leaf_value_binding(scope, trimmed, binding) &&
       binding &&
       binding->type) {
      out = binding->type;
      category = semantic_conversion::VC_LVALUE;
      return true;
    }

    vector<FunctionBinding *> functions;
    if(lookup_leaf_function_bindings(scope, trimmed, functions) &&
       functions.size() == 1 &&
       functions[0] &&
       functions[0]->type) {
      out = functions[0]->type;
      category = semantic_conversion::VC_LVALUE;
      return true;
    }

    return false;
  }

  vector<pair<TypePtr, semantic_conversion::ValueCategory> > arg_infos;
  const string trimmed_args = trim_space(arg_text);
  if(!trimmed_args.empty()) {
    const vector<string> arg_texts = split_comma_list(trimmed_args);
    for(size_t i = 0; i < arg_texts.size(); ++i) {
      TypePtr arg_type;
      semantic_conversion::ValueCategory arg_category =
          semantic_conversion::VC_PRVALUE;
      if(!evaluate_declval_expression_type_category_text(
             services, scope, arg_texts[i], arg_type, arg_category) ||
         !arg_type) {
        return false;
      }
      arg_infos.push_back(make_pair(arg_type, arg_category));
    }
  }

  QualifiedName template_name;
  vector<string> template_args;
  if(arg_infos.empty() &&
     semantic_utils::split_top_level_template_id_text(
         trim_space(callee_text), template_name, template_args) &&
	     template_args.size() == 1 &&
	     unqualified_member_name(template_name.name) == "declval") {
	    TypePtr declared_type;
	    if(!services.semantic_context) {
	      return false;
	    }
	    declared_type =
	        services.semantic_context->lookup_type(scope, template_args[0], true);
	    if(!declared_type) {
	      return false;
	    }
    out = collapse_rvalue_reference_type(make_rvalue_reference_raw(declared_type));
    return out != nullptr &&
           semantic_conversion::result_value_category_for_function_result(out, category);
  }

  string object_text;
  string member_name;
  bool arrow = false;
  if(split_top_level_member_expression_text(
         callee_text, object_text, member_name, arrow)) {
    TypePtr object_expr_type;
    semantic_conversion::ValueCategory object_category =
        semantic_conversion::VC_PRVALUE;
    if(!evaluate_declval_expression_type_category_text(services,
                                                       scope,
                                                       object_text,
                                                       object_expr_type,
                                                       object_category) ||
       !object_expr_type) {
      return false;
    }

    TypePtr object_type = strip_top_level_cv(remove_reference_type(object_expr_type));
    if(arrow) {
      if(!object_type || object_type->kind != Type::TK_POINTER) {
        return false;
      }
      object_type = strip_top_level_cv(remove_reference_type(object_type->inner));
    }
    if(!object_type) {
      return false;
    }
    vector<FunctionBinding *> functions;
    FunctionBinding * selected = nullptr;
    if(!service_lookup_leaf_member_function_bindings(
           services, object_type, member_name, functions) ||
       !select_unique_leaf_function_binding(
           functions,
           arg_infos,
           true,
           object_type,
           semantic_conversion::is_const_object_type(object_type),
           object_category == semantic_conversion::VC_LVALUE,
           selected) ||
       !selected) {
      return false;
    }
    return function_type_result(selected->type, out, category);
  }

  {
    TypePtr callee_type;
    semantic_conversion::ValueCategory callee_category =
        semantic_conversion::VC_PRVALUE;
    if(!evaluate_declval_expression_type_category_text(services,
                                                       scope,
                                                       callee_text,
                                                       callee_type,
                                                       callee_category) ||
       !callee_type) {
      return false;
    }
    TypePtr function_type;
    if(resolve_callable_function_type(callee_type, function_type) &&
       function_type_call_result(function_type, arg_infos, out, category)) {
      return true;
    }

    TypePtr object_type = strip_top_level_cv(remove_reference_type(callee_type));
    vector<FunctionBinding *> functions;
    FunctionBinding * selected = nullptr;
    const bool found_call_ops =
        object_type &&
        service_lookup_leaf_member_function_bindings(
            services, object_type, "operator()", functions);
    const bool selected_call_op =
        found_call_ops &&
        select_unique_leaf_function_binding(
            functions,
            arg_infos,
            true,
            object_type,
            semantic_conversion::is_const_object_type(object_type),
            callee_category == semantic_conversion::VC_LVALUE,
            selected) &&
        selected;
    if(!selected_call_op) {
      return false;
    }
    function_type = strip_top_level_cv(selected->type);
    return function_type_result(function_type, out, category);
  }
}

bool evaluate_declval_expression_type_text(template_api::TemplateServices & services,
                                           Scope & scope,
                                           const string & text,
                                           TypePtr & out)
{
  semantic_conversion::ValueCategory category = semantic_conversion::VC_PRVALUE;
  return evaluate_declval_expression_type_category_text(
      services, scope, text, out, category);
}

bool parse_decltype_or_typeof_text(template_api::TemplateServices & services,
                                   Scope & scope,
                                   const string & text,
                                   TypePtr & out)
{
  out.reset();
  if(!is_decltype_or_typeof_text(text)) {
    return false;
  }

  DependentTypeExprText parsed_text;
  if(!parse_dependent_type_expr_text(text, parsed_text)) {
    return false;
  }

  template_api::TemplateEnvironmentHandle env =
      template_api::make_template_environment(scope);
  const bool mentions_template_placeholders =
      text_mentions_template_placeholders(services, env, parsed_text.inner);
  const bool mentions_dependent_names =
      text_mentions_dependent_non_namespace_binding_names(
          services, env, parsed_text.inner);
  const bool mentions_bound_names =
      text_mentions_non_namespace_binding_names(env, parsed_text.inner);
  const bool scope_has_placeholders =
      scope_has_template_placeholders(services, env);

  string inner = parsed_text.inner;
  const auto comma_prefix_mentions_template_dependency =
      [&]() -> bool
  {
    if(!has_top_level_comma(inner)) {
      return false;
    }
    const vector<string> parts = split_comma_list(inner);
    for(size_t i = 0; i + 1 < parts.size(); ++i) {
      if(text_mentions_template_placeholders(services, env, parts[i]) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, env, parts[i]) ||
         unresolved_identifier_argument_may_depend_on_template_context(
             services, env, parts[i]) ||
         (scope_has_placeholders &&
          text_mentions_non_namespace_binding_names(env, parts[i]))) {
        return true;
      }
    }
    return false;
  }();

  const auto dependent_fallback =
      [&]() -> bool
  {
    out.reset();
    if(mentions_template_placeholders ||
       mentions_dependent_names ||
       comma_prefix_mentions_template_dependency ||
       (scope_has_placeholders && !mentions_bound_names)) {
      out = make_semantic_named(
          text,
          parsed_text.is_typeof ? Type::NSK_DEPENDENT_TYPEOF :
                                  Type::NSK_DEPENDENT_DECLTYPE,
          text,
          true);
      return true;
    }
    return false;
  };

  if(parsed_text.is_typeof) {
    if(services.semantic_context) {
      out = services.semantic_context->lookup_type(scope, inner, true);
    }
    if(out &&
       !service_type_depends_on_template_parameter(services, out)) {
      return true;
    }
    out.reset();
  }

  const vector<string> rewritten_exprs =
      rewrite_decltype_expression_pack_texts(services, scope, inner);
  if(rewritten_exprs.size() != 1) {
    return dependent_fallback();
  }
  if(!parsed_text.is_typeof &&
     evaluate_declval_expression_type_text(
         services, scope, rewritten_exprs[0], out) &&
     out &&
     !service_type_depends_on_template_parameter(services, out)) {
    if(comma_prefix_mentions_template_dependency) {
      return dependent_fallback();
    }
    return true;
  }
  out.reset();

  return dependent_fallback();
}

bool parse_decltype_or_typeof_node(template_api::TemplateServices & services,
                                   Scope & scope,
                                   const CppAstNode & node,
                                   TypePtr & out)
{
  out.reset();
  if((node.kind != CppAstKind::decltype_specifier &&
      node.kind != CppAstKind::decl_specifier) ||
     !is_decltype_or_typeof_text(node.value)) {
    return false;
  }

  DependentTypeExprText parsed_text;
  if(!parse_dependent_type_expr_text(node.value, parsed_text)) {
    return false;
  }

  template_api::TemplateEnvironmentHandle env =
      template_api::make_template_environment(scope);
  const bool mentions_template_placeholders =
      text_mentions_template_placeholders(services, env, parsed_text.inner);
  const bool mentions_dependent_names =
      text_mentions_dependent_non_namespace_binding_names(
          services, env, parsed_text.inner);
  const bool mentions_bound_names =
      text_mentions_non_namespace_binding_names(env, parsed_text.inner);
  const bool scope_has_placeholders =
      scope_has_template_placeholders(services, env);

  string inner = parsed_text.inner;
  const CppAstNode * operand = decltype_or_typeof_operand_node(node);
  const bool comma_prefix_mentions_template_dependency =
      (operand && comma_prefix_ast_mentions_template_dependency(
                      services, env, *operand)) ||
      [&]() -> bool
  {
    if(!has_top_level_comma(inner)) {
      return false;
    }
    const vector<string> parts = split_comma_list(inner);
    for(size_t i = 0; i + 1 < parts.size(); ++i) {
      if(text_mentions_template_placeholders(services, env, parts[i]) ||
         text_mentions_dependent_non_namespace_binding_names(
             services, env, parts[i]) ||
         unresolved_identifier_argument_may_depend_on_template_context(
             services, env, parts[i]) ||
         (scope_has_placeholders &&
          text_mentions_non_namespace_binding_names(env, parts[i]))) {
        return true;
      }
    }
    return false;
  }();

  const auto dependent_fallback =
      [&]() -> bool
  {
    out.reset();
    if(mentions_template_placeholders ||
       mentions_dependent_names ||
       comma_prefix_mentions_template_dependency ||
       (scope_has_placeholders && !mentions_bound_names)) {
      out = make_dependent_type_expression_type(
          node.value,
          parsed_text.is_typeof ? Type::NSK_DEPENDENT_TYPEOF :
                                  Type::NSK_DEPENDENT_DECLTYPE,
          node.value,
          node);
      return true;
    }
    return false;
  };

  if(parsed_text.is_typeof) {
    if(operand && operand->kind == CppAstKind::type_id) {
      if(parse_type_id_node_for_templates(services, scope, *operand, out, false) && out) {
        return true;
      }
    } else if(!operand) {
      if(services.semantic_context) {
        out = services.semantic_context->lookup_type(scope, inner, false);
      }
      if(out) {
        return true;
      }
    }
  }

  const vector<string> rewritten_exprs =
      rewrite_decltype_expression_pack_texts(services, scope, inner);
  if(rewritten_exprs.size() != 1) {
    return dependent_fallback();
  }
  if(!operand || operand->kind == CppAstKind::type_id) {
    return dependent_fallback();
  }

  const CppAstNode & expr = *operand;
  CppAstNode expanded_expr;
  const CppAstNode * request_expr = &expr;
  bool expanded_expr_changed = false;
  if(expand_pack_expressions_in_decltype_operand(scope,
                                                 expr,
                                                 expanded_expr,
                                                 expanded_expr_changed) &&
     expanded_expr_changed) {
    request_expr = &expanded_expr;
  }

  const string base_use_location =
      template_public_use_location_or(services.witness_context, string());
  size_t use_offset = parsed_text.inner_offset;
  const string call_callee = first_call_callee_name(*request_expr);
  if(!call_callee.empty()) {
    const size_t callee_offset =
        find_identifier_occurrence(node.value, call_callee, parsed_text.inner_offset);
    if(callee_offset != string::npos) {
      use_offset = callee_offset;
    }
  }
  if(expr.kind == CppAstKind::new_expression) {
    const size_t placement_declval_offset =
        find_identifier_occurrence(node.value, "declval", parsed_text.inner_offset);
    if(placement_declval_offset != string::npos) {
      use_offset = placement_declval_offset;
    }
  }
  const string token_use_location =
      witness::source_capture_enabled(services.witness_context) ?
          template_api::template_witness_detail::
              source_location_for_identifier_token_on_or_after(
              services.witness_context,
              base_use_location,
              request_expr->kind == CppAstKind::new_expression ? string("declval") : call_callee) :
          string();

  template_api::TemplateDependentTypeExprRequest request;
  request.scope = &scope;
  request.kind = parsed_text.is_typeof ? template_api::TDTEK_TYPEOF_EXPR :
                                         template_api::TDTEK_DECLTYPE;
  request.operand_was_parenthesized = parsed_text.operand_was_parenthesized;
  request.use_location = !token_use_location.empty() ?
      token_use_location :
      source_location_with_text_offset(base_use_location, node.value, use_offset);
  request.operand = *request_expr;
  TypePtr evaluated;
  if(evaluate_dependent_type_expression_leaf(services, scope, request, evaluated) &&
     evaluated &&
     !service_type_depends_on_template_parameter(services, evaluated)) {
    if(comma_prefix_mentions_template_dependency) {
      return dependent_fallback();
    }
    out = evaluated;
    return true;
  }
  if(service_evaluate_dependent_type_expression(services, request, evaluated) &&
     evaluated &&
     !service_type_depends_on_template_parameter(services, evaluated)) {
    if(comma_prefix_mentions_template_dependency) {
      return dependent_fallback();
    }
    out = evaluated;
    return true;
  }

  return dependent_fallback();
}

bool simple_type_specifier_word(const string & word, ETokenType & out)
{
  if(word == "const" || word == "__const" || word == "__const__") {
    out = KW_CONST;
    return true;
  }
  if(word == "volatile" || word == "__volatile" || word == "__volatile__") {
    out = KW_VOLATILE;
    return true;
  }
  if(word == "signed" || word == "__signed" || word == "__signed__") {
    out = KW_SIGNED;
    return true;
  }
  if(word == "unsigned" || word == "__unsigned" || word == "__unsigned__") {
    out = KW_UNSIGNED;
    return true;
  }
  if(word == "short") {
    out = KW_SHORT;
    return true;
  }
  if(word == "long") {
    out = KW_LONG;
    return true;
  }
  if(word == "int") {
    out = KW_INT;
    return true;
  }
  if(word == "char") {
    out = KW_CHAR;
    return true;
  }
  if(word == "char16_t") {
    out = KW_CHAR16_T;
    return true;
  }
  if(word == "char32_t") {
    out = KW_CHAR32_T;
    return true;
  }
  if(word == "wchar_t") {
    out = KW_WCHAR_T;
    return true;
  }
  if(word == "bool") {
    out = KW_BOOL;
    return true;
  }
  if(word == "float") {
    out = KW_FLOAT;
    return true;
  }
  if(word == "double") {
    out = KW_DOUBLE;
    return true;
  }
  if(word == "void") {
    out = KW_VOID;
    return true;
  }
  return false;
}

bool whitespace_separated_type_specifier_words(const string & text,
                                               vector<string> & out)
{
  out.clear();
  string current;
  bool saw_space = false;
  for(size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(std::isspace(ch)) {
      saw_space = true;
      if(!current.empty()) {
        out.push_back(current);
        current.clear();
      }
      continue;
    }
    if(!(std::isalnum(ch) || text[i] == '_')) {
      out.clear();
      return false;
    }
    current += text[i];
  }
  if(!current.empty()) {
    out.push_back(current);
  }
  return saw_space && !out.empty();
}

bool lookup_simple_type_specifier_word(template_api::TemplateServices & services,
                                       Scope & scope,
                                       const string & word,
                                       TypePtr & out)
{
  out.reset();
  if(!is_identifier_text(word)) {
    return false;
  }

  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.name.name = word;
  request.allow_class_templates = true;
  return service_resolve_direct_type_lookup(services, request, out) && out;
}

bool parse_simple_type_specifier_argument_text(
    template_api::TemplateServices & services,
    Scope & scope,
    const string & text,
    TypePtr & out)
{
  out.reset();
  vector<string> words;
  if(!whitespace_separated_type_specifier_words(text, words)) {
    return false;
  }

  TypeSpecifierAccumulator acc;
  for(size_t i = 0; i < words.size(); ++i) {
    ETokenType simple = KW_INT;
    if(simple_type_specifier_word(words[i], simple)) {
      if(acc.add_cv(simple) || acc.add_simple_type(simple)) {
        continue;
      }
      return false;
    }

    TypePtr named_type;
    if(!lookup_simple_type_specifier_word(services, scope, words[i], named_type) ||
       !named_type ||
       !acc.set_named_type(named_type)) {
      return false;
    }
  }

  return acc.finalize(out);
}

bool parse_type_argument_text(template_api::TemplateServices & services,
                              template_api::TemplateEnvironmentHandle scope,
                              const string & text,
                              TypePtr & out)
{
  Scope & raw_scope = scope.require();
  const string trimmed = trim_space(text);
  string stripped_typename_text;
  const string typename_prefix = "typename ";
  const string rewritten = trimmed;
  if(has_invalid_top_level_qualified_owner_syntax(trimmed)) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "resolve-type-arg invalid-owner text=" << trimmed;
        });
    return false;
  }
  const char * route = "lookup";
  bool used_exact_local = false;
  out = lookup_exact_local_type_name(services, raw_scope, trimmed);
  if(out) {
    route = "exact-local";
    used_exact_local = true;
  }
  if(!out) {
    out = lookup_local_dependent_type_placeholder(raw_scope, trimmed);
  }
  if(out) {
    if(!used_exact_local) {
      route = "local-placeholder";
    }
  } else {
    const string exact_bound_name = strip_elaborated_type_prefix(trimmed);
    out = lookup_exact_bound_type_name(raw_scope, exact_bound_name);
    if(out) {
      route = "exact-bound";
    }
  }
  if(!out) {
    if(parse_simple_type_specifier_argument_text(services, raw_scope, trimmed, out)) {
      route = "simple-specifier";
    }
  }
  if(!out) {
    QualifiedName type_pack_element_name;
    vector<string> type_pack_element_args;
    if(semantic_utils::split_top_level_template_id_text(trimmed,
                                                        type_pack_element_name,
                                                        type_pack_element_args) &&
       try_resolve_type_pack_element_template_id(services,
                                                 scope,
                                                 type_pack_element_name,
                                                 type_pack_element_args,
                                                 nullptr,
                                                 out)) {
      route = "__type_pack_element";
    }
  }
  if(!out &&
     trimmed.size() > typename_prefix.size() &&
     trimmed.compare(0, typename_prefix.size(), typename_prefix) == 0) {
    const string typename_rest =
        trim_space(trimmed.substr(typename_prefix.size()));
    const bool mentions_template_placeholders =
        !typename_rest.empty() &&
        text_mentions_template_placeholders(services, scope, typename_rest);
    const bool mentions_dependent_bindings =
        !typename_rest.empty() &&
        text_mentions_dependent_non_namespace_binding_names(services, scope, typename_rest);
    if(!typename_rest.empty() &&
       (mentions_template_placeholders ||
        mentions_dependent_bindings ||
        should_defer_unresolved_type_lookup(services, raw_scope, typename_rest))) {
      route = "typename-dependent";
      out = make_semantic_named(trimmed,
                                Type::NSK_DEPENDENT_TYPE,
                                trimmed,
                                true);
    } else {
      if(strip_leading_typename_text(trimmed, stripped_typename_text)) {
        route = "typename-bound";
        if(!out &&
           scope_has_template_placeholders(
               services, template_api::make_template_environment(raw_scope)) &&
           !has_top_level_comma(stripped_typename_text)) {
          out = make_semantic_named(trimmed,
                                    Type::NSK_TEMPLATE_PARAMETER,
                                    stripped_typename_text,
                                    true);
          route = "typename-placeholder";
        }
      }
    }
  }
  if(!out) {
    out = try_resolve_direct_concrete_qualified_member_type(services, scope, rewritten);
    if(out) {
      route = "direct-qualified";
    }
  }
  if(!out && is_decltype_or_typeof_text(rewritten)) {
    route = "decltype";
    parse_decltype_or_typeof_text(services, raw_scope, rewritten, out);
  }
  if(!out &&
     is_simple_identifier_text(trimmed) &&
     scope_has_template_placeholders(
         services, template_api::make_template_environment(raw_scope))) {
    out = make_semantic_named(trimmed,
                              Type::NSK_TEMPLATE_PARAMETER,
                              trimmed,
                              true);
    route = "identifier-placeholder";
  }
  const bool resolved_dependent =
      resolve_instantiated_dependent_type_if_needed(services, scope, out);
  if(out) {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "type-arg text=" << trimmed
                << " route=" << route
                << (rewritten != trimmed ? " rewritten=" + rewritten : string())
                << (resolved_dependent ? "+resolved" : "")
                << " type=" << describe_type(out);
          TypePtr base = strip_top_level_cv(remove_reference_type(out));
          if(base && base->kind == Type::TK_NAMED) {
            trace << " complete=" << (base->named_complete ? "yes" : "no")
                  << " has_layout=" << (base->named_has_layout ? "yes" : "no");
          }
        });
  } else {
    note_template_trace_if_enabled(
        [&](ostringstream & trace)
        {
          trace << "type-arg text=" << trimmed
                << (rewritten != trimmed ? " rewritten=" + rewritten : string())
                << " route=unresolved";
        });
  }
  return out != nullptr;
}

vector<ExpandedTypeArgumentInput> expand_bound_type_pack_arguments(
    template_api::TemplateServices & services,
    Scope & scope,
    const vector<string> & texts)
{
  (void)services;
  vector<ExpandedTypeArgumentInput> out;
  for(size_t i = 0; i < texts.size(); ++i) {
    const string text = trim_space(texts[i]);
    if(text.size() > 3 && text.substr(text.size() - 3) == "...") {
      const string pattern = trim_space(text.substr(0, text.size() - 3));
      const vector<TypePtr> * bound_pack = lookup_type_pack(scope, pattern);
      if(bound_pack) {
        for(size_t j = 0; j < bound_pack->size(); ++j) {
          ExpandedTypeArgumentInput input;
          input.text = reparseable_type_argument_text((*bound_pack)[j]);
          input.type = (*bound_pack)[j];
          out.push_back(input);
        }
        continue;
      }
      vector<string> expanded_patterns;
      if(expand_bound_type_pack_pattern_text(scope, pattern, expanded_patterns)) {
        for(size_t j = 0; j < expanded_patterns.size(); ++j) {
          ExpandedTypeArgumentInput input;
          input.text = expanded_patterns[j];
          out.push_back(input);
        }
        continue;
      }
    }
    const string integer_pack_prefix = "__integer_pack(";
    if(text.size() > integer_pack_prefix.size() + 4 &&
       text.compare(0, integer_pack_prefix.size(), integer_pack_prefix) == 0 &&
       text.substr(text.size() - 4) == ")...") {
      const string count_text =
          trim_space(text.substr(integer_pack_prefix.size(),
                                 text.size() - integer_pack_prefix.size() - 4));
      long long count = 0;
      if(try_evaluate_integral_text_with_pack_scope(scope, count_text, count) &&
         count >= 0) {
        for(long long value = 0; value < count; ++value) {
          ExpandedTypeArgumentInput input;
          input.text = to_string(value);
          out.push_back(input);
        }
        continue;
      }
    }
    ExpandedTypeArgumentInput input;
    input.text = texts[i];
    out.push_back(input);
  }
  return out;
}

vector<string> expanded_type_argument_input_texts(
    const vector<ExpandedTypeArgumentInput> & inputs)
{
  vector<string> out;
  out.reserve(inputs.size());
  for(size_t i = 0; i < inputs.size(); ++i) {
    out.push_back(inputs[i].text);
  }
  return out;
}

vector<string> expand_bound_type_pack_texts(template_api::TemplateServices & services,
                                            Scope & scope,
                                            const vector<string> & texts)
{
  return expanded_type_argument_input_texts(
      expand_bound_type_pack_arguments(services, scope, texts));
}

static bool ast_node_contains_pack_expansion_syntax(const CppAstNode & node);

bool expand_bound_packs_in_type_id_node(template_api::TemplateServices & services,
                                        Scope & scope,
                                        const CppAstNode & node,
                                        CppAstNode & out)
{
  if(!ast_node_contains_pack_expansion_syntax(node)) {
    return false;
  }
  out = clone_expression_node_for_template_substitution(node);
  return expand_bound_packs_in_expression_node(services, scope, out);
}

vector<string> expand_bound_expression_pack_texts(template_api::TemplateServices & services,
                                                  Scope & scope,
                                                  const string & text)
{
  const string trimmed = trim_space(text);
  if(trimmed.size() <= 3 || trimmed.substr(trimmed.size() - 3) != "...") {
    return vector<string>(1, trimmed);
  }

  const string pattern = trim_space(trimmed.substr(0, trimmed.size() - 3));
  const auto pattern_mentions_expansion_pack =
      [](const string & text, const string & pack_name) -> bool
      {
        string masked = text;
        const string prefix = "sizeof...";
        size_t pos = 0;
        while((pos = masked.find(prefix, pos)) != string::npos) {
          size_t open = pos + prefix.size();
          while(open < masked.size() &&
                std::isspace(static_cast<unsigned char>(masked[open]))) {
            ++open;
          }
          if(open >= masked.size() || masked[open] != '(') {
            pos += prefix.size();
            continue;
          }
          size_t close = open + 1;
          int depth = 1;
          for(; close < masked.size(); ++close) {
            if(masked[close] == '(') {
              ++depth;
            } else if(masked[close] == ')') {
              --depth;
              if(depth == 0) {
                ++close;
                break;
              }
            }
          }
          if(depth != 0) {
            break;
          }
          for(size_t i = pos; i < close; ++i) {
            masked[i] = ' ';
          }
          pos = close;
        }
        return callsemantic_internal::contains_identifier_token(masked, pack_name);
      };
  vector<pair<string, const vector<TypePtr> *> > packs;
  vector<pair<string, const vector<ValueBinding> *> > value_packs;
  set<string> seen_type_pack_names;
  set<string> seen_value_pack_names;
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(const auto & pack : current->named_type_packs) {
      const string & pack_name = pack.first;
      if(pack_name.empty() ||
         seen_type_pack_names.count(pack_name) != 0 ||
         !pattern_mentions_expansion_pack(pattern, pack_name)) {
        continue;
      }
      seen_type_pack_names.insert(pack_name);
      packs.push_back(make_pair(pack_name, &pack.second));
    }
    for(const auto & pack : current->named_value_packs) {
      const string & pack_name = pack.first;
      if(pack_name.empty() ||
         seen_value_pack_names.count(pack_name) != 0 ||
         !pattern_mentions_expansion_pack(pattern, pack_name)) {
        continue;
      }
      seen_value_pack_names.insert(pack_name);
      value_packs.push_back(make_pair(pack_name, &pack.second));
    }
  }

  if(packs.empty() && value_packs.empty()) {
    return expanded_type_argument_input_texts(
        expand_bound_type_pack_arguments(services, scope, vector<string>(1, trimmed)));
  }

  for(size_t i = 0; i < packs.size(); ++i) {
    if(packs[i].second->empty()) {
      return vector<string>();
    }
  }
  for(size_t i = 0; i < value_packs.size(); ++i) {
    if(value_packs[i].second->empty()) {
      return vector<string>();
    }
  }

  const size_t pack_size =
      !packs.empty() ? packs[0].second->size() : value_packs[0].second->size();
  for(size_t i = 1; i < packs.size(); ++i) {
    if(packs[i].second->size() != pack_size) {
      return vector<string>(1, trimmed);
    }
  }
  for(size_t i = 0; i < value_packs.size(); ++i) {
    if(value_packs[i].second->size() != pack_size) {
      return vector<string>(1, trimmed);
    }
  }

  vector<string> out;
  out.reserve(pack_size);
  for(size_t i = 0; i < pack_size; ++i) {
    string rewritten = pattern;
    bool changed = false;
    for(size_t j = 0; j < packs.size(); ++j) {
      rewritten = replace_identifier_token_text(
          rewritten,
          packs[j].first,
          reparseable_type_argument_text((*(packs[j].second))[i]),
          changed);
    }
    for(size_t j = 0; j < value_packs.size(); ++j) {
      const ValueBinding & binding = (*(value_packs[j].second))[i];
      const string replacement =
          binding.has_constant_value && !binding.dependent_template_value ?
              service_lookup_text_for_non_type_template_argument(
                  services, binding.type, binding.constant_value) :
              binding.name;
      rewritten = replace_identifier_token_text(
          rewritten, value_packs[j].first, replacement, changed);
    }
    out.push_back(trim_space(rewritten));
  }
  return out;
}

const TemplateArgumentSyntax * ExpandedTemplateArgumentInputs::syntax_for(size_t index) const
{
  if(index >= syntaxes.size()) {
    return nullptr;
  }
  return syntaxes[index];
}

TypePtr ExpandedTemplateArgumentInputs::type_for(size_t index) const
{
  if(index >= type_arguments.size()) {
    return TypePtr();
  }
  return type_arguments[index];
}

static const TemplateArgumentSyntax * add_owned_expanded_argument_syntax(
    ExpandedTemplateArgumentInputs & inputs,
    const TemplateArgumentSyntax & syntax)
{
  inputs.owned_syntaxes.push_back(
      std::shared_ptr<TemplateArgumentSyntax>(
          new TemplateArgumentSyntax(syntax)));
  return inputs.owned_syntaxes.back().get();
}

static bool template_argument_syntax_matches_text(
    const TemplateArgumentSyntax & syntax,
    const string & text)
{
  string syntax_text = trim_space(syntax.text);
  if(syntax_text.empty() && syntax.type_id) {
    syntax_text = trim_space(node_text(*syntax.type_id));
  }
  if(syntax_text.empty() && syntax.template_id) {
    syntax_text = template_id_syntax_lookup_text(*syntax.template_id);
  }
  if(syntax_text.empty()) {
    return false;
  }
  if(syntax.pack_expansion &&
     (syntax_text.size() < 3 || syntax_text.substr(syntax_text.size() - 3) != "...")) {
    syntax_text += "...";
  }
  const auto syntax_key =
      [](const string & value) -> string
      {
        string key = compact_source_argument_key(value);
        const char * const disambiguators[] = {
          "::template",
          ".template",
          "->template"
        };
        for(size_t i = 0; i < sizeof(disambiguators) / sizeof(disambiguators[0]); ++i) {
          const string token = disambiguators[i];
          size_t pos = 0;
          while((pos = key.find(token, pos)) != string::npos) {
            key.erase(pos + token.size() - 8, 8);
            pos += token.size() - 8;
          }
        }
        return key;
      };
  return syntax_key(syntax_text) == syntax_key(text);
}

bool simple_type_argument_name_from_syntax(const TemplateArgumentSyntax & syntax,
                                           string & out)
{
  out.clear();
  const string text = trim_space(syntax.text);
  if(is_identifier_text(text)) {
    out = text;
    return true;
  }
  if(!syntax.type_id ||
     syntax.type_id->kind != CppAstKind::type_id ||
     syntax.type_id->children.size() != 1 ||
     syntax.type_id->children[0].kind != CppAstKind::type_specifier_seq ||
     syntax.type_id->children[0].children.size() != 1 ||
     syntax.type_id->children[0].children[0].kind != CppAstKind::type_name) {
    return false;
  }
  out = syntax.type_id->children[0].children[0].value;
  return !out.empty();
}

static TypePtr resolved_direct_bound_type_argument_syntax(
    Scope & scope,
    const TemplateArgumentSyntax & syntax)
{
  string name;
  if(!simple_type_argument_name_from_syntax(syntax, name)) {
    return TypePtr();
  }
  return lookup_exact_bound_type_name(scope, name);
}

static const TemplateArgumentSyntax * carry_substituted_bound_type_argument_syntax(
    ExpandedTemplateArgumentInputs & inputs,
    Scope & scope,
    const TemplateArgumentSyntax & source,
    const string & text,
    TypePtr & out_type)
{
  out_type = resolved_direct_bound_type_argument_syntax(scope, source);
  if(!out_type) {
    return nullptr;
  }

  TemplateArgumentSyntax syntax =
      clone_argument_syntax_for_template_substitution(source);
  if(syntax.source_text.empty()) {
    syntax.source_text = trim_space(source.source_text.empty() ?
                                        source.text :
                                        source.source_text);
  }
  syntax.text = text;
  syntax.pack_expansion = false;
  syntax.resolved_type = out_type;
  if(!syntax.template_id && !syntax.expression) {
    syntax.type_id.reset(new CppAstNode(
        make_substituted_type_id_node(out_type, text)));
  }
  return add_owned_expanded_argument_syntax(inputs, syntax);
}

static bool argument_syntax_contains_resolved_type(
    const TemplateArgumentSyntax & syntax);

static bool template_id_syntax_contains_resolved_type(
    const TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(argument_syntax_contains_resolved_type(syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

static bool ast_node_contains_resolved_template_argument_type(
    const CppAstNode & node)
{
  if(node.template_id_syntax &&
     template_id_syntax_contains_resolved_type(*node.template_id_syntax)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_node_contains_resolved_template_argument_type(
         *node.conversion_type_id_syntax)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_node_contains_resolved_template_argument_type(*node.base_type_syntax)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_contains_resolved_type(
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_contains_resolved_template_argument_type(
           node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_contains_resolved_template_argument_type(
           node.exception_type_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    if(ast_node_contains_resolved_template_argument_type(
           node.alignment_specifier_nodes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_contains_resolved_template_argument_type(node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool argument_syntax_contains_resolved_type(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.resolved_type) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_contains_resolved_type(*syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_contains_resolved_template_argument_type(*syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_contains_resolved_template_argument_type(*syntax.expression)) {
    return true;
  }
  return false;
}

static bool argument_syntax_contains_pack_expansion(
    const TemplateArgumentSyntax & syntax);

static bool template_id_syntax_contains_pack_expansion(
    const TemplateIdSyntax & syntax)
{
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    if(argument_syntax_contains_pack_expansion(syntax.argument_syntaxes[i])) {
      return true;
    }
  }
  return false;
}

static bool ast_node_contains_pack_expansion_syntax(const CppAstNode & node)
{
  if(node.kind == CppAstKind::pack_expansion_expression) {
    return true;
  }
  if(node.template_id_syntax &&
     template_id_syntax_contains_pack_expansion(*node.template_id_syntax)) {
    return true;
  }
  if(node.conversion_type_id_syntax &&
     ast_node_contains_pack_expansion_syntax(*node.conversion_type_id_syntax)) {
    return true;
  }
  if(node.base_type_syntax &&
     ast_node_contains_pack_expansion_syntax(*node.base_type_syntax)) {
    return true;
  }
  for(size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(template_id_syntax_contains_pack_expansion(
           node.qualifier_template_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.qualifier_type_syntaxes.size(); ++i) {
    if(ast_node_contains_pack_expansion_syntax(node.qualifier_type_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.exception_type_id_syntaxes.size(); ++i) {
    if(ast_node_contains_pack_expansion_syntax(node.exception_type_id_syntaxes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.alignment_specifier_nodes.size(); ++i) {
    if(ast_node_contains_pack_expansion_syntax(
           node.alignment_specifier_nodes[i])) {
      return true;
    }
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(ast_node_contains_pack_expansion_syntax(node.children[i])) {
      return true;
    }
  }
  return false;
}

static bool argument_syntax_contains_pack_expansion(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.pack_expansion) {
    return true;
  }
  if(syntax.template_id &&
     template_id_syntax_contains_pack_expansion(*syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_contains_pack_expansion_syntax(*syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_contains_pack_expansion_syntax(*syntax.expression)) {
    return true;
  }
  return false;
}

static bool argument_syntax_contains_nested_pack_expansion(
    const TemplateArgumentSyntax & syntax)
{
  if(syntax.template_id &&
     template_id_syntax_contains_pack_expansion(*syntax.template_id)) {
    return true;
  }
  if(syntax.type_id &&
     ast_node_contains_pack_expansion_syntax(*syntax.type_id)) {
    return true;
  }
  if(syntax.expression &&
     ast_node_contains_pack_expansion_syntax(*syntax.expression)) {
    return true;
  }
  return false;
}

static bool should_preserve_type_expanded_syntax(
    const TemplateArgumentSyntax & source,
    const TemplateArgumentSyntax & expanded)
{
  if(expanded.resolved_type) {
    return true;
  }
  if(source.pack_expansion && !expanded.pack_expansion) {
    return true;
  }
  if(argument_syntax_contains_nested_pack_expansion(source) &&
     argument_syntax_contains_resolved_type(expanded)) {
    return true;
  }
  if(!source.template_id || !expanded.template_id) {
    return false;
  }
  const vector<TemplateArgumentSyntax> & source_args =
      source.template_id->argument_syntaxes;
  const vector<TemplateArgumentSyntax> & expanded_args =
      expanded.template_id->argument_syntaxes;
  const size_t count = std::min(source_args.size(), expanded_args.size());
  for(size_t i = 0; i < count; ++i) {
    if(source_args[i].pack_expansion &&
       !expanded_args[i].pack_expansion &&
       expanded_args[i].resolved_type) {
      return true;
    }
  }
  return false;
}

vector<TemplateArgumentSyntax> expand_type_pack_argument_syntaxes(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateArgumentSyntax & source_syntax,
    const vector<string> & expanded_texts)
{
  vector<pair<string, const vector<TypePtr> *> > packs;
  vector<pair<string, const vector<ValueBinding> *> > value_packs;
  collect_type_pack_references_in_argument_syntax(scope, source_syntax, packs);
  collect_value_pack_references_in_argument_syntax(scope, source_syntax, value_packs);
  if(packs.empty() && value_packs.empty()) {
    return vector<TemplateArgumentSyntax>();
  }

  const size_t pack_size =
      !packs.empty() ? packs[0].second->size() : value_packs[0].second->size();
  if(pack_size != expanded_texts.size()) {
    return vector<TemplateArgumentSyntax>();
  }
  for(size_t i = 1; i < packs.size(); ++i) {
    if(packs[i].second->size() != pack_size) {
      return vector<TemplateArgumentSyntax>();
    }
  }
  for(size_t i = 0; i < value_packs.size(); ++i) {
    if(value_packs[i].second->size() != pack_size) {
      return vector<TemplateArgumentSyntax>();
    }
  }

  vector<TemplateArgumentSyntax> expanded;
  expanded.reserve(pack_size);
  for(size_t pack_index = 0; pack_index < pack_size; ++pack_index) {
    map<string, TypePtr> type_replacements;
    for(size_t i = 0; i < packs.size(); ++i) {
      type_replacements[packs[i].first] = (*(packs[i].second))[pack_index];
    }
    map<string, ValueBinding> value_replacements;
    for(size_t i = 0; i < value_packs.size(); ++i) {
      value_replacements[value_packs[i].first] =
          (*(value_packs[i].second))[pack_index];
    }
	    if(source_syntax.type_id) {
	      collect_bound_type_replacements_in_node(scope,
	                                              *source_syntax.type_id,
	                                              type_replacements);
      collect_bound_value_replacements_in_node(scope,
                                               *source_syntax.type_id,
                                               value_replacements);
    }
	    if(source_syntax.expression) {
	      collect_bound_type_replacements_in_node(scope,
	                                              *source_syntax.expression,
	                                              type_replacements);
      collect_bound_value_replacements_in_node(scope,
                                               *source_syntax.expression,
                                               value_replacements);
    }

    TemplateArgumentSyntax syntax =
        clone_argument_syntax_for_template_substitution(source_syntax);
    if(syntax.source_text.empty()) {
      syntax.source_text = trim_space(source_syntax.source_text.empty() ?
                                          source_syntax.text :
                                          source_syntax.source_text);
    }
    syntax.text = trim_space(expanded_texts[pack_index]);
    syntax.pack_expansion = false;
    if(syntax.template_id) {
      substitute_type_pack_template_id_arguments(*syntax.template_id,
                                                 scope,
                                                 type_replacements);
    }
    if(syntax.type_id && !type_replacements.empty()) {
      CppAstNode substituted;
      if(substitute_type_pack_expression_node(scope,
                                              *syntax.type_id,
                                              type_replacements,
                                              substituted)) {
        erase_parameter_pack_marker_nodes(substituted);
        syntax.type_id.reset(new CppAstNode(substituted));
      }
    }
    if(syntax.expression && !type_replacements.empty()) {
      CppAstNode substituted;
      if(substitute_type_pack_expression_node(scope,
                                              *syntax.expression,
                                              type_replacements,
                                              substituted)) {
        syntax.expression.reset(new CppAstNode(substituted));
      }
    }
    if(syntax.expression && !value_replacements.empty()) {
      CppAstNode substituted;
      if(substitute_value_pack_expression_node(*syntax.expression,
                                               value_replacements,
                                               substituted)) {
        syntax.expression.reset(new CppAstNode(substituted));
      }
    }
    if(syntax.type_id && !value_replacements.empty()) {
      CppAstNode substituted;
      if(substitute_value_pack_expression_node(*syntax.type_id,
                                               value_replacements,
                                               substituted)) {
        erase_parameter_pack_marker_nodes(substituted);
        syntax.type_id.reset(new CppAstNode(substituted));
      }
    }
    string direct_pack_name;
    if(simple_type_argument_name_from_syntax(source_syntax, direct_pack_name)) {
      map<string, TypePtr>::const_iterator direct =
          type_replacements.find(direct_pack_name);
      if(direct != type_replacements.end()) {
        syntax.resolved_type = direct->second;
      }
    }
	    expanded.push_back(syntax);
  }
  return expanded;
}

ExpandedTemplateArgumentInputs expand_template_argument_inputs(
    template_api::TemplateServices & services,
    Scope & scope,
    const vector<string> & texts,
    const vector<TemplateArgumentSyntax> * syntaxes)
{
  ExpandedTemplateArgumentInputs out;
  out.texts.reserve(texts.size());
  out.type_arguments.reserve(texts.size());
  if(syntaxes) {
    out.syntaxes.reserve(texts.size());
  }

  for(size_t i = 0; i < texts.size(); ++i) {
    const TemplateArgumentSyntax * source_syntax =
        syntaxes && i < syntaxes->size() ? &(*syntaxes)[i] : nullptr;
    const string trimmed_text = trim_space(texts[i]);
    TypePtr carried_type;
    if(source_syntax &&
       !template_argument_syntax_matches_text(*source_syntax, trimmed_text)) {
      source_syntax =
          carry_substituted_bound_type_argument_syntax(out,
                                                       scope,
                                                       *source_syntax,
                                                       trimmed_text,
                                                       carried_type);
    }
    if(texts[i].find("...") == string::npos) {
      out.texts.push_back(trimmed_text);
      out.type_arguments.push_back(carried_type);
      if(syntaxes) {
        out.syntaxes.push_back(source_syntax);
      }
      continue;
    }

    const vector<ExpandedTypeArgumentInput> type_expanded_inputs =
        expand_bound_type_pack_arguments(services, scope, vector<string>(1, texts[i]));
    vector<string> type_expanded_texts;
    type_expanded_texts.reserve(type_expanded_inputs.size());
    for(size_t j = 0; j < type_expanded_inputs.size(); ++j) {
      type_expanded_texts.push_back(type_expanded_inputs[j].text);
    }
    const vector<TemplateArgumentSyntax> type_expanded_syntaxes =
        source_syntax ?
            expand_type_pack_argument_syntaxes(services,
                                               scope,
                                               *source_syntax,
                                               type_expanded_texts) :
            vector<TemplateArgumentSyntax>();
    for(size_t j = 0; j < type_expanded_texts.size(); ++j) {
      const TemplateArgumentSyntax * type_expanded_syntax =
          j < type_expanded_syntaxes.size() ? &type_expanded_syntaxes[j] :
                                              nullptr;
      const vector<string> expr_expanded_texts =
          expand_bound_expression_pack_texts(services, scope, type_expanded_texts[j]);
      const TemplateArgumentSyntax * expr_source_syntax = type_expanded_syntax;
      if(!expr_source_syntax &&
         source_syntax &&
         type_expanded_texts.size() == 1 &&
         trim_space(type_expanded_texts[j]) == trimmed_text) {
        expr_source_syntax = source_syntax;
      }
      const vector<TemplateArgumentSyntax> expr_expanded_syntaxes =
          expr_source_syntax ?
              expand_type_pack_argument_syntaxes(services,
                                                 scope,
                                                 *expr_source_syntax,
                                                 expr_expanded_texts) :
              vector<TemplateArgumentSyntax>();
      for(size_t k = 0; k < expr_expanded_texts.size(); ++k) {
        const string trimmed_expanded_text = trim_space(expr_expanded_texts[k]);
        const bool unchanged = trimmed_expanded_text == trimmed_text;
        const bool expression_unchanged =
            trimmed_expanded_text == trim_space(type_expanded_texts[j]);
        out.texts.push_back(trimmed_expanded_text);
        out.type_arguments.push_back(
            expression_unchanged && j < type_expanded_inputs.size() ?
                type_expanded_inputs[j].type :
                TypePtr());
        if(syntaxes) {
          if(expression_unchanged &&
             type_expanded_syntax &&
             source_syntax &&
             should_preserve_type_expanded_syntax(*source_syntax,
                                                  *type_expanded_syntax)) {
            out.syntaxes.push_back(
                add_owned_expanded_argument_syntax(out, *type_expanded_syntax));
          } else if(k < expr_expanded_syntaxes.size()) {
            out.syntaxes.push_back(
                add_owned_expanded_argument_syntax(out, expr_expanded_syntaxes[k]));
          } else if(unchanged) {
            out.syntaxes.push_back(source_syntax);
          } else {
            out.syntaxes.push_back(nullptr);
          }
        }
      }
    }
  }

  return out;
}

}  // namespace template_argument_semantics
