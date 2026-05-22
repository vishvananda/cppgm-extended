#include "template_resolution.h"

#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "callsem_output.h"
#include "callsemantic_internal.h"
#include "cpp_decl_bridge.h"
#include "pack_parameter_analysis.h"
#include "parser_trace.h"
#include "semantic_context.h"
#include "semantic_conversion.h"
#include "semantic_errors.h"
#include "semantic_fallback_audit.h"
#include "semantic_hotspot.h"
#include "semantic_lookup.h"
#include "semantic_metrics.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "text_intern.h"
#include "template_api_internal.h"
#include "template_argument_semantics.h"
#include "template_decl_ast.h"
#include "template_instantiation.h"
#include "template_metadata.h"
#include "template_services.h"
#include "template_scope.h"
#include "template_specialization.h"
#include "template_witness.h"
#include "types.h"

namespace template_argument_semantics {

// template-boundary-audit: begin text_recovery_bridge
std::string lookup_text_for_type_argument(SemanticContext & ctx,
                                          const cpp_decl::TypePtr & type);
std::string lookup_text_for_type_argument(template_api::TemplateTypeSystem & type_system,
                                          const cpp_decl::TypePtr & type);

bool type_depends_on_template_parameter(SemanticContext & ctx,
                                        const cpp_decl::TypePtr & type);
bool type_depends_on_template_parameter(template_api::TemplateTypeSystem & type_system,
                                        const cpp_decl::TypePtr & type);

bool resolve_instantiated_dependent_type(SemanticContext & ctx,
                                         semantic_model::Scope & scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);
bool resolve_instantiated_dependent_type(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const cpp_decl::TypePtr & type,
                                         cpp_decl::TypePtr & out);

// template-boundary-audit: end text_recovery_bridge

}  // namespace template_argument_semantics

namespace template_resolution {

using namespace cpp_decl;
using namespace semantic_conversion;
using namespace semantic_model;
using namespace template_model;
using callsemantic_internal::reparseable_type_argument_text;
using callsemantic_internal::replace_elaborated_identifier_token_text;
using callsemantic_internal::replace_identifier_token_text;
using semantic_trace::scope_bindings_for_diagnostic;
using semantic_trace::scope_name_for_diagnostic;

bool resolve_template_argument(SemanticContext & ctx,
                               Scope & argument_scope,
                               Scope & parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               const TemplateArgumentSyntax * syntax,
                               TemplateArgument & out);

bool resolve_template_argument(SemanticContext & ctx,
                               Scope & argument_scope,
                               Scope & parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               TemplateArgument & out);

bool deduce_function_template_arguments(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const std::vector<ExprInfo> & args,
    std::vector<TemplateArgument> & out,
    Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const TypePtr & target,
    std::vector<TemplateArgument> & out,
    Scope * use_scope = nullptr,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_from_target_type_with_explicit(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & resolution_scope,
    const std::vector<TemplateArgument> & explicit_arguments,
    const TypePtr & target,
    std::vector<TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool deduce_function_template_arguments_with_explicit(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & resolution_scope,
    const std::vector<TemplateArgument> & explicit_arguments,
    const std::vector<ExprInfo> & args,
    std::vector<TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out = nullptr);

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count);

void bind_explicit_function_template_pack_arguments(
    Scope & bound_scope,
    const TemplateParameterInfo & parameter,
    const std::vector<TemplateArgument> & arguments);

namespace {

bool env_flag_enabled(const char * name)
{
  const char * value = std::getenv(name);
  if(!value || !*value) {
    return false;
  }
  return std::string(value) != "0";
}

bool bound_member_failure_cache_enabled()
{
  static const bool enabled =
      !env_flag_enabled("CPPGM_DISABLE_BOUND_MEMBER_FAILURE_CACHE");
  return enabled;
}

bool template_arguments_cache_enabled()
{
  static const bool enabled =
      !env_flag_enabled("CPPGM_DISABLE_TEMPLATE_ARGUMENTS_CACHE");
  return enabled;
}

bool template_arguments_fast_cache_enabled()
{
  static const bool enabled =
      !env_flag_enabled("CPPGM_DISABLE_TEMPLATE_ARGUMENTS_FAST_CACHE");
  return enabled;
}

typedef std::map<std::string, TypePtr> DeducedTypeMap;
typedef std::map<std::string, long long> DeducedValueMap;
typedef std::map<std::string, std::vector<TemplateArgument> > DeducedPackArgumentMap;

using semantic_utils::strip_elaborated_type_prefix;
using semantic_utils::trim_space;

void bind_single_template_argument_into_scope(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    const TemplateArgument & argument);

bool try_resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out);

struct ExplicitFunctionTemplateArgumentBindings
{
  std::vector<TemplateArgument> fixed_arguments;
  std::size_t pack_parameter_index = static_cast<std::size_t>(-1);
  std::vector<TemplateArgument> pack_arguments;
};

struct ScopedTemplateArgumentUseLocation
{
  explicit ScopedTemplateArgumentUseLocation(const std::string & location)
    : active(!location.empty())
  {
    if(active) {
      parser_trace::push_use_location(location);
    }
  }

  ~ScopedTemplateArgumentUseLocation()
  {
    if(active) {
      parser_trace::pop_use_location();
    }
  }

  ScopedTemplateArgumentUseLocation(const ScopedTemplateArgumentUseLocation &) = delete;
  ScopedTemplateArgumentUseLocation & operator=(
      const ScopedTemplateArgumentUseLocation &) = delete;

  bool active = false;
};

std::string source_location_for_ast_start(
    const template_api::TemplateWitnessContext & ctx,
    const CppAstNode & node)
{
  return template_api::template_witness_detail::source_location_for_ast_node_start(
      ctx, node);
}

std::string source_location_for_template_argument_syntax(
    const template_api::TemplateWitnessContext & ctx,
    const TemplateArgumentSyntax * syntax)
{
  if(!syntax) {
    return std::string();
  }
  if(syntax->source_location_id != 0 && ctx.source_locations) {
    const std::string location =
        ctx.source_locations->describe(syntax->source_location_id);
    if(!location.empty() && location != "<unknown>") {
      return template_api::normalize_template_witness_source_location(
          std::string(" at ") + location);
    }
  }
  if(!syntax->has_source_token_start) {
    return std::string();
  }
  return template_api::normalize_template_witness_source_location(
      template_api::template_witness_detail::source_location_for_token_index(
          ctx,
          syntax->source_token_start));
}

std::string compact_source_argument_key(const std::string & text)
{
  std::string out;
  out.reserve(text.size());
  for(std::size_t i = 0; i < text.size(); ++i) {
    if(!std::isspace(static_cast<unsigned char>(text[i]))) {
      out.push_back(text[i]);
    }
  }
  return out;
}

std::string source_text_for_qualified_name(const QualifiedName & qualified)
{
  std::string out;
  if(qualified.rooted) {
    out += "::";
  }
  for(std::size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out += qualified.qualifiers[i];
    out += "::";
  }
  out += qualified.name;
  return out;
}

std::string source_text_for_template_id_syntax(const TemplateIdSyntax & syntax)
{
  std::string out = source_text_for_qualified_name(syntax.name);
  out += "<";
  const std::size_t count =
      std::max(syntax.arguments.size(), syntax.argument_syntaxes.size());
  for(std::size_t i = 0; i < count; ++i) {
    if(i != 0) {
      out += ",";
    }
    std::string arg;
    if(i < syntax.argument_syntaxes.size()) {
      arg = trim_space(syntax.argument_syntaxes[i].text);
    }
    if(arg.empty() && i < syntax.arguments.size()) {
      arg = trim_space(syntax.arguments[i]);
    }
    out += arg;
  }
  out += ">";
  return out;
}

const TemplateIdSyntax * bare_template_id_syntax_for_type_argument(
    const CppAstNode & node,
    const std::string & text)
{
  const CppAstNode * type_name = nullptr;
  if(node.kind == CppAstKind::type_name) {
    type_name = &node;
  } else if(node.kind == CppAstKind::type_id) {
    const CppAstNode * specifiers =
        cppast_find_child_kind(node, CppAstKind::type_specifier_seq);
    if(!specifiers || specifiers->children.size() != 1 ||
       specifiers->children[0].kind != CppAstKind::type_name) {
      return nullptr;
    }
    const CppAstNode * declarator =
        cppast_find_child_kind(node, CppAstKind::abstract_declarator);
    if(declarator && !trim_space(node_text(*declarator)).empty()) {
      return nullptr;
    }
    type_name = &specifiers->children[0];
  } else {
    return nullptr;
  }

  const TemplateIdSyntax * template_id = cppast_template_id_syntax(*type_name);
  if(!template_id || template_id->name.name.empty()) {
    return nullptr;
  }
  return compact_source_argument_key(source_text_for_template_id_syntax(*template_id)) ==
         compact_source_argument_key(text) ?
      template_id :
      nullptr;
}

TemplateArgumentSyntax make_default_template_argument_syntax(
    const TemplateParameterInfo & parameter,
    const CppAstNode & node,
    const std::string & text)
{
  TemplateArgumentSyntax syntax;
  syntax.text = text;
  syntax.has_source_token_start = node.token_end > node.token_start;
  syntax.source_token_start = node.token_start;
  syntax.source_location_id = node.source_location_id;
  if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
    syntax.expression.reset(new CppAstNode(node));
  } else {
    syntax.type_id.reset(new CppAstNode(node));
    if(const TemplateIdSyntax * template_id =
           bare_template_id_syntax_for_type_argument(node, text)) {
      syntax.template_id.reset(new TemplateIdSyntax(*template_id));
    }
  }
  return syntax;
}

std::string default_type_argument_text_from_ast(
    const TemplateParameterInfo & parameter,
    const CppAstNode & node);

bool make_substituted_default_template_argument_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    const CppAstNode & node,
    const std::string & fallback_text,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    TemplateArgumentSyntax & syntax)
{
  CppAstNode substituted;
  if(!template_argument_semantics::substitute_expression_node_for_template_arguments(
         scope, node, parameters, arguments, substituted)) {
    return false;
  }
  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
    CppAstNode expanded;
    if(template_argument_semantics::expand_bound_packs_in_type_id_node(
           services, scope, substituted, expanded)) {
      substituted = expanded;
    }
  }
  std::string text = default_type_argument_text_from_ast(parameter, substituted);
  if(text.empty()) {
    text = fallback_text;
  }
  syntax = make_default_template_argument_syntax(parameter, substituted, text);
  syntax.text = text;
  return true;
}

bool make_substituted_default_template_argument_syntax(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    const CppAstNode & node,
    const std::string & fallback_text,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    TemplateArgumentSyntax & syntax)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return make_substituted_default_template_argument_syntax(services,
                                                                 scope,
                                                                 parameter,
                                                                 node,
                                                                 fallback_text,
                                                                 parameters,
                                                                 arguments,
                                                                 syntax);
      });
}

std::shared_ptr<CppAstNode> make_dependent_template_id_expression(
    const TemplateArgumentSyntax & syntax)
{
  if(!syntax.template_id) {
    return std::shared_ptr<CppAstNode>();
  }

  CppAstNode expression;
  expression.kind = CppAstKind::id_expression;
  expression.value = source_text_for_template_id_syntax(*syntax.template_id);
  expression.source_location_id = syntax.source_location_id;
  if(syntax.has_source_token_start) {
    expression.token_start = syntax.source_token_start;
    expression.token_end = syntax.source_token_start + 1;
  }
  set_cppast_template_id_syntax(expression, *syntax.template_id);

  if(!syntax.pack_expansion) {
    return std::shared_ptr<CppAstNode>(new CppAstNode(std::move(expression)));
  }

  CppAstNode expanded;
  expanded.kind = CppAstKind::pack_expansion_expression;
  expanded.value = "...";
  expanded.source_location_id = syntax.source_location_id;
  if(syntax.has_source_token_start) {
    expanded.token_start = syntax.source_token_start;
    expanded.token_end = syntax.source_token_start + 1;
  }
  expanded.children.push_back(std::move(expression));
  return std::shared_ptr<CppAstNode>(new CppAstNode(std::move(expanded)));
}

void attach_dependent_non_type_argument_expression(
    const TemplateArgumentSyntax * syntax,
    const std::string & selected_text,
    const std::string & original_text,
    TemplateArgument & out)
{
  if(!syntax || selected_text != original_text) {
    return;
  }
  if(syntax->expression) {
    out.expression.reset(new CppAstNode(*syntax->expression));
    return;
  }
  out.expression = make_dependent_template_id_expression(*syntax);
}

std::string ast_leaf_text(const CppAstNode & node)
{
  std::string text = node_text(node);
  if(!text.empty()) {
    return text;
  }
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    const std::string part = ast_leaf_text(node.children[i]);
    if(part.empty()) {
      continue;
    }
    if(!text.empty()) {
      text += " ";
    }
    text += part;
  }
  return text;
}

std::string default_type_argument_text_from_ast(
    const TemplateParameterInfo & parameter,
    const CppAstNode & node)
{
  if(parameter.kind == TemplateParameterInfo::TP_TYPE &&
     node.kind == CppAstKind::type_id) {
    return ast_leaf_text(node);
  }
  return node_text(node);
}

std::vector<std::string> source_argument_texts_for_template_id_syntax(
    const TemplateIdSyntax & syntax)
{
  std::vector<std::string> out;
  const std::size_t syntax_count =
      std::min(syntax.argument_syntaxes.size(), syntax.arguments.size());
  out.reserve(syntax.arguments.size());
  for(std::size_t i = 0; i < syntax_count; ++i) {
    std::string text = trim_space(
        syntax.argument_syntaxes[i].source_text.empty() ?
            syntax.argument_syntaxes[i].text :
            syntax.argument_syntaxes[i].source_text);
    if(syntax.argument_syntaxes[i].expression) {
      const std::string expression_text =
          trim_space(callsemantic_internal::describe_expression_for_diagnostic(
              *syntax.argument_syntaxes[i].expression));
      if(!expression_text.empty() &&
         compact_source_argument_key(expression_text) ==
             compact_source_argument_key(syntax.argument_syntaxes[i].text)) {
        text = expression_text;
      }
    }
    out.push_back(text.empty() ? trim_space(syntax.arguments[i]) : text);
  }
  for(std::size_t i = syntax_count; i < syntax.arguments.size(); ++i) {
    out.push_back(trim_space(syntax.arguments[i]));
  }
  return out;
}

void clone_deduced_type_map(const DeducedTypeMap & source, DeducedTypeMap & out)
{
  out.clear();
  for(DeducedTypeMap::const_iterator it = source.begin(); it != source.end(); ++it) {
    out[it->first] = it->second;
  }
}

void clone_deduced_value_map(const DeducedValueMap & source, DeducedValueMap & out)
{
  out.clear();
  for(DeducedValueMap::const_iterator it = source.begin(); it != source.end(); ++it) {
    out[it->first] = it->second;
  }
}

std::string typed_non_type_template_argument_text(template_api::TemplateTypeSystem & type_system,
                                                  const TypePtr & type,
                                                  long long value);

bool encode_data_member_pointer_template_argument_offset(std::size_t offset,
                                                         long long & out)
{
  if(offset >= static_cast<std::size_t>(std::numeric_limits<long long>::max())) {
    return false;
  }
  out = static_cast<long long>(offset + 1);
  return true;
}

const ValueBinding * lookup_unqualified_value(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const std::string & name);

std::vector<FunctionBinding *> lookup_unqualified_functions(Scope & scope,
                                                            const std::string & name);

bool try_resolve_named_non_type_template_argument(template_api::TemplateServices & services,
                                                  Scope & scope,
                                                  const std::string & text,
                                                  const TypePtr & target_type,
                                                  TemplateArgument & out,
                                                  const ValueBinding ** out_binding = nullptr);

bool is_identifier_text(const std::string & text);

bool lookup_direct_bound_type_argument(Scope & scope,
                                       const std::string & text,
                                       TypePtr & out);

std::string type_argument_text_for_deduction(template_api::TemplateTypeSystem & type_system,
                                             const TypePtr & type);

bool non_type_template_parameter_is_still_dependent(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter);

bool text_mentions_template_dependency(template_api::TemplateServices & services,
                                       template_api::TemplateEnvironmentHandle scope,
                                       const std::string & text);

bool should_defer_unresolved_type_lookup(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const std::string & text);

std::string default_argument_expression_text(const CppAstNode & node);

bool default_argument_expression_is_still_dependent(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node);

bool qualified_name_has_template_id_component(const QualifiedName & name)
{
  QualifiedName ignored_name;
  std::vector<std::string> ignored_args;
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(semantic_utils::split_top_level_template_id_text(
           trim_space(name.qualifiers[i]),
           ignored_name,
           ignored_args)) {
      return true;
    }
  }
  return semantic_utils::split_top_level_template_id_text(
      trim_space(name.name),
      ignored_name,
      ignored_args);
}

// template-boundary-audit: begin semantic_service_access, text_recovery_bridge
template_api::TemplateTypeSystem & service_type_system(
    template_api::TemplateServices & services)
{
  return services.type_system;
}

std::string service_typed_non_type_template_argument_text(
    template_api::TemplateServices & services,
    const TypePtr & type,
    long long value)
{
  return typed_non_type_template_argument_text(
      service_type_system(services),
      type,
      value);
}

bool service_type_depends_on_template_parameter(
    template_api::TemplateServices & services,
    const TypePtr & type)
{
  return template_argument_semantics::type_depends_on_template_parameter(
      service_type_system(services),
      type);
}

// template-boundary-audit: end semantic_service_access, text_recovery_bridge

std::string typed_non_type_template_argument_text(SemanticContext & ctx,
                                                  const TypePtr & type,
                                                  long long value)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return typed_non_type_template_argument_text(type_system, type, value);
      });
}

// template-boundary-audit: begin text_recovery_bridge
std::string typed_non_type_template_argument_text(template_api::TemplateTypeSystem & type_system,
                                                  const TypePtr & type,
                                                  long long value)
{
  TemplateArgument argument;
  argument.kind = TemplateArgument::TA_VALUE;
  argument.type = type;
  argument.value = value;
  return template_argument_text(
      argument,
      [&type_system](const TypePtr & current_type)
      {
        return template_argument_semantics::lookup_text_for_type_argument(type_system,
                                                                          current_type);
      });
}
// template-boundary-audit: end text_recovery_bridge

bool non_type_function_target_type(const TypePtr & target_type,
                                   bool explicit_address,
                                   TypePtr & out_function_type)
{
  out_function_type.reset();
  TypePtr target_base = strip_top_level_cv(target_type);
  if(!target_base) {
    return false;
  }
  if(target_base->kind == Type::TK_POINTER &&
     target_base->inner &&
     strip_top_level_cv(target_base->inner)->kind == Type::TK_FUNCTION) {
    out_function_type = target_base->inner;
    return true;
  }
  if(!explicit_address &&
     (target_base->kind == Type::TK_LVALUE_REFERENCE ||
      target_base->kind == Type::TK_RVALUE_REFERENCE) &&
     target_base->inner &&
     strip_top_level_cv(target_base->inner)->kind == Type::TK_FUNCTION) {
    out_function_type = target_base->inner;
    return true;
  }
  return false;
}

FunctionBinding * select_non_type_function_argument(
    const std::vector<FunctionBinding *> & functions,
    const TypePtr & function_type)
{
  FunctionBinding * selected = nullptr;
  for(std::size_t i = 0; i < functions.size(); ++i) {
    FunctionBinding * candidate = functions[i];
    if(!candidate ||
       !candidate->type ||
       !type_equals(strip_top_level_cv(candidate->type),
                    strip_top_level_cv(function_type))) {
      continue;
    }
    if(selected) {
      return nullptr;
    }
    selected = candidate;
  }
  return selected;
}

bool bind_non_type_function_argument(const TypePtr & target_type,
                                     FunctionBinding * function,
                                     const std::string & text,
                                     TemplateArgument & out)
{
  if(!function) {
    return false;
  }
  out.kind = TemplateArgument::TA_VALUE;
  out.type = target_type;
  out.function_value = function;
  out.text = text;
  out.dependent = false;
  return true;
}

bool try_resolve_function_non_type_template_argument_name(
    template_api::TemplateServices & services,
    Scope & scope,
    const TypePtr & target_type,
    const QualifiedName * qualified,
    const std::string & unqualified_name,
    bool explicit_address,
    TemplateArgument & out)
{
  TypePtr function_type;
  if(!non_type_function_target_type(target_type, explicit_address, function_type)) {
    return false;
  }

  std::vector<FunctionBinding *> functions;
  std::string argument_text;
  if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
    if(!services.semantic_context) {
      return false;
    }
    try {
      functions = services.semantic_context->lookup_qualified_functions(scope, *qualified);
    } catch(const TemplateSubstitutionFailure &) {
      return false;
    } catch(const SemanticSoftFailure &) {
      return false;
    } catch(const SemanticDiagnosticError &) {
      return false;
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      return false;
    }
    argument_text = source_text_for_qualified_name(*qualified);
  } else {
    if(unqualified_name.empty()) {
      return false;
    }
    functions = lookup_unqualified_functions(scope, unqualified_name);
    argument_text = unqualified_name;
  }

  return bind_non_type_function_argument(
      target_type,
      select_non_type_function_argument(functions, function_type),
      argument_text,
      out);
}

bool lookup_member_pointer_function_candidates(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const QualifiedName & qualified,
                                               std::vector<FunctionBinding *> & out)
{
  out.clear();
  if(!services.semantic_context ||
     (!qualified.rooted && qualified.qualifiers.empty())) {
    return false;
  }

  Scope * target =
      semantic_lookup::resolve_qualified_scope_for_class_or_namespace(
          *services.semantic_context,
          scope,
          qualified);
  if(!target || !target->class_info) {
    return false;
  }

  ClassInfo * target_class = target->class_info;
  if(!target_class->complete && target_class->type) {
    if(ClassInfo * completed =
           services.semantic_context->complete_class_type(target_class->type)) {
      target_class = completed;
    }
  }

  semantic_lookup::MemberCallableLookupResult callables =
      semantic_lookup::lookup_visible_member_callables(*target_class, qualified.name);
  if(callables.functions.empty() && !callables.templates.empty()) {
    return false;
  }

  out = callables.functions;
  return !out.empty();
}

bool lookup_qualified_function_candidates_node(template_api::TemplateServices & services,
                                               Scope & scope,
                                               const QualifiedName & qualified,
                                               const CppAstNode & node,
                                               std::vector<FunctionBinding *> & out)
{
  out.clear();
  if(!services.semantic_context ||
     (!qualified.rooted && qualified.qualifiers.empty())) {
    return false;
  }

  Scope * target =
      services.semantic_context->resolve_qualified_scope_for_node(scope,
                                                                  qualified,
                                                                  node,
                                                                  false);
  if(!target) {
    return false;
  }

  if(target->class_info) {
    ClassInfo * target_class = target->class_info;
    if(!target_class->complete && target_class->type) {
      if(ClassInfo * completed =
             services.semantic_context->complete_class_type(target_class->type)) {
        target_class = completed;
      }
    }
    out = semantic_lookup::lookup_visible_member_functions(*target_class,
                                                           qualified.name).functions;
    return !out.empty();
  }

  semantic_lookup::lookup_functions_in_scopes(std::vector<Scope *>(1, target),
                                              qualified.name,
                                              out);
  return !out.empty();
}

bool try_resolve_function_non_type_template_argument_syntax(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateArgumentSyntax * syntax,
    const TypePtr & target_type,
    TemplateArgument & out)
{
  if(!syntax || !syntax->expression) {
    return false;
  }
  const CppAstNode * operand = syntax->expression.get();
  bool explicit_address = false;
  if(operand->kind == CppAstKind::unary_expression &&
     operand->children.size() == 1 &&
     operand->has_token &&
     operand->simple_type == OP_AMP) {
    operand = &operand->children[0];
    explicit_address = true;
  }
  if(!operand || operand->kind != CppAstKind::id_expression) {
    return false;
  }
  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(*operand)) {
    TypePtr function_type;
    if(!services.semantic_context ||
       !non_type_function_target_type(target_type, explicit_address, function_type)) {
      return false;
    }

    std::vector<FunctionBinding *> functions;
    try {
      functions =
          services.semantic_context->lookup_function_template_id_node(
              scope,
              *operand,
              *template_id,
              semantic_policy::without_body_instantiation());
    } catch(const TemplateSubstitutionFailure &) {
      return false;
    } catch(const SemanticSoftFailure &) {
      return false;
    } catch(const SemanticDiagnosticError &) {
      return false;
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      return false;
    }

    return bind_non_type_function_argument(
        target_type,
        select_non_type_function_argument(functions, function_type),
        operand->value,
        out);
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(*operand);
  if(qualified &&
     (qualified->rooted || !qualified->qualifiers.empty()) &&
     (!operand->qualifier_template_id_syntaxes.empty() ||
      !operand->qualifier_type_syntaxes.empty())) {
    TypePtr function_type;
    if(non_type_function_target_type(target_type, explicit_address, function_type)) {
      std::vector<FunctionBinding *> functions;
      try {
        if(lookup_qualified_function_candidates_node(services,
                                                     scope,
                                                     *qualified,
                                                     *operand,
                                                     functions)) {
          return bind_non_type_function_argument(
              target_type,
              select_non_type_function_argument(functions, function_type),
              operand->value,
              out);
        }
      } catch(const TemplateSubstitutionFailure &) {
        return false;
      } catch(const SemanticSoftFailure &) {
        return false;
      } catch(const SemanticDiagnosticError &) {
        return false;
      } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        return false;
      }
    }
  }
  return try_resolve_function_non_type_template_argument_name(
      services,
      scope,
      target_type,
      qualified,
      operand->value,
      explicit_address,
      out);
}

bool try_resolve_named_non_type_template_argument(template_api::TemplateServices & services,
                                                  Scope & scope,
                                                  const std::string & text,
                                                  const TypePtr & target_type,
                                                  TemplateArgument & out,
                                                  const ValueBinding ** out_binding)
{
  if(out_binding) {
    *out_binding = nullptr;
  }
  const std::string trimmed = trim_space(text);
  if(trimmed.empty() || !target_type) {
    return false;
  }
  const auto non_type_argument_text =
      [&services, &target_type](long long value) -> std::string
      {
        return service_typed_non_type_template_argument_text(
            services,
            target_type,
            value);
      };

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
  const auto integral_value_fits_target =
      [](const TypePtr & target, long long value) -> bool
      {
        TypePtr base = strip_top_level_cv(target);
        if(!base || base->kind != Type::TK_FUNDAMENTAL ||
           !is_integral_type(base)) {
          return false;
        }
        if(base->fundamental == FT_BOOL) {
          return true;
        }
        const std::size_t bits = type_size(base) * 8;
        if(is_unsigned_integral_type(base)) {
          if(value < 0) {
            return false;
          }
          if(bits >= 63) {
            return true;
          }
          const unsigned long long max_value = (1ull << bits) - 1ull;
          return static_cast<unsigned long long>(value) <= max_value;
        }
        if(bits >= 63) {
          return true;
        }
        const long long max_value = (1ll << (bits - 1)) - 1ll;
        const long long min_value = -(1ll << (bits - 1));
        return value >= min_value && value <= max_value;
      };
  const auto integral_binding_conversion_compatible =
      [&](const TypePtr & binding_base,
          const ValueBinding & binding) -> bool
      {
        if(!target_base || !binding_base ||
           !is_integral_type(target_base) ||
           !is_integral_type(binding_base)) {
          return false;
        }
        return !binding.has_constant_value ||
               integral_value_fits_target(target_base, binding.constant_value);
      };
  if(target_base && is_bool_type(target_base) &&
     (trimmed == "true" || trimmed == "false")) {
    const long long value = trimmed == "true" ? 1 : 0;
    out.kind = TemplateArgument::TA_VALUE;
    out.type = target_type;
    out.value = value;
    out.text = non_type_argument_text(value);
    out.dependent = false;
    return true;
  }
  if(target_base && is_integral_type(target_base) &&
     std::isdigit(static_cast<unsigned char>(trimmed[0]))) {
    unsigned long long unsigned_value = 0;
    std::string ud_suffix;
    try {
      const EFundamentalType literal_type =
          classify_int(trimmed, unsigned_value, ud_suffix);
      if(literal_type != FT_VOID &&
         ud_suffix.empty() &&
         unsigned_value <= static_cast<unsigned long long>(
                               std::numeric_limits<long long>::max())) {
        const long long value = static_cast<long long>(unsigned_value);
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.value = value;
        out.text = non_type_argument_text(value);
        out.dependent = false;
        return true;
      }
    } catch(const std::logic_error &) {
    }
  }
  if(is_identifier_text(trimmed)) {
    const ValueBinding * binding = lookup_unqualified_value(services, scope, trimmed);
    if(binding && binding->type) {
      TypePtr binding_base = strip_top_level_cv(remove_reference_type(binding->type));
      const bool compatible =
          (!target_base || !binding_base) ?
              type_equals(binding->type, target_type) :
              type_equals(binding_base, target_base) ||
              integral_binding_conversion_compatible(binding_base, *binding) ||
              (is_integral_type(target_base) &&
               semantic_conversion::is_unscoped_enum_type(binding_base));
      if(compatible && binding->has_constant_value) {
        if(out_binding) {
          *out_binding = binding;
        }
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.value = binding->constant_value;
        out.text = non_type_argument_text(binding->constant_value);
        out.dependent = false;
        return true;
      }
      if(compatible && !binding->non_type_template_argument_text.empty()) {
        const std::string rebound_text =
            trim_space(binding->non_type_template_argument_text);
        if(rebound_text != trimmed &&
           try_resolve_named_non_type_template_argument(services,
                                                        scope,
                                                        rebound_text,
                                                        target_type,
                                                        out,
                                                        nullptr)) {
          if(out_binding) {
            *out_binding = binding;
          }
          return true;
        }
      }
      if(compatible && binding->dependent_template_value) {
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.text =
            !binding->non_type_template_argument_text.empty() ?
                binding->non_type_template_argument_text :
                trimmed;
        out.dependent = true;
        return true;
      }
    }
  }
  QualifiedName qualified_value_name;
  if(services.semantic_context &&
     semantic_utils::split_qualified_name_text(trimmed, qualified_value_name) &&
     (qualified_value_name.rooted || !qualified_value_name.qualifiers.empty()) &&
     !qualified_name_has_template_id_component(qualified_value_name) &&
     compact_source_argument_key(source_text_for_qualified_name(qualified_value_name)) ==
         compact_source_argument_key(trimmed) &&
     !text_mentions_template_dependency(
         services,
         template_api::make_template_environment(scope),
         trimmed)) {
    const ValueBinding * binding = nullptr;
    try {
      binding =
          semantic_lookup::lookup_qualified_value_binding(*services.semantic_context,
                                                          scope,
                                                          qualified_value_name);
    } catch(const TemplateSubstitutionFailure &) {
      binding = nullptr;
    } catch(const SemanticSoftFailure &) {
      binding = nullptr;
    } catch(const SemanticDiagnosticError &) {
      binding = nullptr;
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      binding = nullptr;
    }
    if(binding && binding->type) {
      TypePtr binding_base = strip_top_level_cv(remove_reference_type(binding->type));
      const bool compatible =
          (!target_base || !binding_base) ?
              type_equals(binding->type, target_type) :
              type_equals(binding_base, target_base) ||
              integral_binding_conversion_compatible(binding_base, *binding) ||
              (is_integral_type(target_base) &&
               semantic_conversion::is_unscoped_enum_type(binding_base));
      if(compatible && binding->has_constant_value) {
        if(out_binding) {
          *out_binding = binding;
        }
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.value = binding->constant_value;
        out.text = non_type_argument_text(binding->constant_value);
        out.dependent = false;
        return true;
      }
      if(compatible && !binding->non_type_template_argument_text.empty()) {
        const std::string rebound_text =
            trim_space(binding->non_type_template_argument_text);
        if(rebound_text != trimmed &&
           try_resolve_named_non_type_template_argument(services,
                                                        scope,
                                                        rebound_text,
                                                        target_type,
                                                        out,
                                                        nullptr)) {
          if(out_binding) {
            *out_binding = binding;
          }
          return true;
        }
      }
      if(compatible && binding->dependent_template_value) {
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.text =
            !binding->non_type_template_argument_text.empty() ?
                binding->non_type_template_argument_text :
                trimmed;
        out.dependent = true;
        return true;
      }
    }
  }
  TypePtr function_target_type;
  if(non_type_function_target_type(target_type, false, function_target_type) &&
     is_identifier_text(trimmed)) {
    if(try_resolve_function_non_type_template_argument_name(
           services,
           scope,
           target_type,
           nullptr,
           trimmed,
           false,
           out)) {
      return true;
    }
  }

  if(target_type->kind == Type::TK_LVALUE_REFERENCE ||
     target_type->kind == Type::TK_RVALUE_REFERENCE) {
    const ValueBinding * binding = lookup_unqualified_value(services, scope, trimmed);
    if(binding && binding->type) {
      TypePtr bound_type = strip_top_level_cv(remove_reference_type(binding->type));
      if(type_equals(bound_type, target_base)) {
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.value_binding = binding;
        out.text = trimmed;
        out.dependent = false;
        return true;
      }
    }
  }

  if(target_base &&
     target_base->kind == Type::TK_MEMBER_POINTER &&
     target_base->inner &&
     !is_function_type(target_base->inner) &&
     trimmed.size() > 1 &&
     trimmed[0] == '&') {
    const std::string member_text = trim_space(trimmed.substr(1));
    QualifiedName qualified;
    if(semantic_utils::split_qualified_name_text(member_text, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      const ValueBinding * binding =
          services.semantic_context ?
              semantic_lookup::lookup_qualified_value_binding(*services.semantic_context,
                                                              scope,
                                                              qualified) :
              nullptr;
      if(binding &&
         binding->kind == ValueBinding::VK_FIELD &&
         binding->owner_class &&
         binding->owner_class->type &&
         type_equals(strip_top_level_cv(binding->type),
                     strip_top_level_cv(target_base->inner)) &&
         type_equals(strip_top_level_cv(binding->owner_class->type),
                     strip_top_level_cv(target_base->owner))) {
        long long encoded_offset = 0;
        if(!encode_data_member_pointer_template_argument_offset(
               binding->field_offset, encoded_offset)) {
          return false;
        }
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.value_binding = binding;
        out.value = encoded_offset;
        out.text.clear();
        out.dependent = false;
        return true;
      }
    }
  }

  if(target_base &&
     target_base->kind == Type::TK_MEMBER_POINTER &&
     target_base->inner &&
     is_function_type(target_base->inner) &&
     trimmed.size() > 1 &&
     trimmed[0] == '&') {
    const std::string member_text = trim_space(trimmed.substr(1));
    QualifiedName qualified;
    if(services.semantic_context &&
       semantic_utils::split_qualified_name_text(member_text, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      std::vector<FunctionBinding *> functions;
      try {
        if(!lookup_member_pointer_function_candidates(services,
                                                      scope,
                                                      qualified,
                                                      functions)) {
          return false;
        }
      } catch(const TemplateSubstitutionFailure &) {
        return false;
      } catch(const SemanticSoftFailure &) {
        return false;
      } catch(const SemanticDiagnosticError &) {
        return false;
      } catch(const semantic_fallback_audit::SemanticFallbackError &) {
        return false;
      } catch(const std::logic_error &) {
        return false;
      }
      FunctionBinding * selected = nullptr;
      for(std::size_t i = 0; i < functions.size(); ++i) {
        FunctionBinding * binding = functions[i];
        if(!binding ||
           !binding->is_method ||
           !binding->owner_class ||
           binding->is_constructor ||
           binding->is_destructor ||
           !binding->owner_class->type ||
           !type_equals(strip_top_level_cv(binding->owner_class->type),
                        strip_top_level_cv(target_base->owner))) {
          continue;
        }
        TypePtr member_function_type = binding->declared_type;
        TypePtr stripped_function_type = strip_top_level_cv(member_function_type);
        if(stripped_function_type &&
           stripped_function_type->kind == Type::TK_FUNCTION &&
           (stripped_function_type->function_const != binding->is_const_method ||
            stripped_function_type->function_volatile != binding->is_volatile_method)) {
          member_function_type = make_function(stripped_function_type->inner,
                                               stripped_function_type->params,
                                               stripped_function_type->variadic,
                                               binding->is_const_method,
                                               binding->is_volatile_method,
                                               stripped_function_type->prototype_relaxed);
        }
        if(!type_equals(strip_top_level_cv(member_function_type),
                        strip_top_level_cv(target_base->inner))) {
          continue;
        }
        if(selected) {
          return false;
        }
        selected = binding;
      }
      if(selected) {
        out.kind = TemplateArgument::TA_VALUE;
        out.type = target_type;
        out.function_value = selected;
        out.text = trimmed;
        out.dependent = false;
        return true;
      }
    }
  }

  return false;
}

struct ResolveTemplateArgumentsCacheEntry
{
  bool success = false;
  std::vector<TemplateArgument> arguments;
};

struct TypePtrAddressHash
{
  std::size_t operator()(const TypePtr & type) const
  {
    return std::hash<const Type *>()(type.get());
  }
};

struct TypePtrAddressEqual
{
  bool operator()(const TypePtr & lhs, const TypePtr & rhs) const
  {
    return lhs.get() == rhs.get();
  }
};

struct FunctionTemplateDeductionArgCacheKey
{
  TypePtr type;
  std::uint64_t type_fingerprint = 0;
  int category = 0;
  bool null_pointer_constant = false;

  bool operator==(const FunctionTemplateDeductionArgCacheKey & other) const
  {
    return type_fingerprint == other.type_fingerprint &&
           (type.get() == other.type.get() || type_equals(type, other.type)) &&
           category == other.category &&
           null_pointer_constant == other.null_pointer_constant;
  }
};

struct FunctionTemplateDeductionCacheKey
{
  const FunctionTemplateDecl * decl = nullptr;
  std::size_t use_scope_instance_id = 0;
  std::uint64_t use_scope_binding_fingerprint = 0;
  std::size_t declaring_scope_instance_id = 0;
  std::uint64_t declaring_scope_binding_fingerprint = 0;
  std::vector<FunctionTemplateDeductionArgCacheKey> args;
  std::size_t hash_value = 0;

  bool operator==(const FunctionTemplateDeductionCacheKey & other) const
  {
    return hash_value == other.hash_value &&
           decl == other.decl &&
           use_scope_instance_id == other.use_scope_instance_id &&
           use_scope_binding_fingerprint == other.use_scope_binding_fingerprint &&
           declaring_scope_instance_id == other.declaring_scope_instance_id &&
           declaring_scope_binding_fingerprint ==
               other.declaring_scope_binding_fingerprint &&
           args == other.args;
  }
};

struct FunctionTemplateDeductionCacheKeyHash
{
  std::size_t operator()(const FunctionTemplateDeductionCacheKey & key) const
  {
    return key.hash_value;
  }
};

struct FunctionTemplateDeductionCacheEntry
{
  bool success = false;
  std::vector<TemplateArgument> arguments;
  std::map<std::string, std::size_t> pack_sizes;
};

struct TemplateParameterCacheKey
{
  int kind = 0;
  std::size_t hash_value = 0;
  std::string name;
  bool parameter_pack = false;
  std::size_t template_parameter_count = 0;
  std::string placeholder_key;
  std::string non_type_decl_specifier_text;
  std::uint64_t value_type_fingerprint = 0;
  std::uint64_t default_argument_fingerprint = 0;
  std::uint64_t default_argument_child_fingerprint = 0;

  bool operator==(const TemplateParameterCacheKey & other) const
  {
    return hash_value == other.hash_value &&
           kind == other.kind &&
           name == other.name &&
           parameter_pack == other.parameter_pack &&
           template_parameter_count == other.template_parameter_count &&
           placeholder_key == other.placeholder_key &&
           non_type_decl_specifier_text == other.non_type_decl_specifier_text &&
           value_type_fingerprint == other.value_type_fingerprint &&
           default_argument_fingerprint == other.default_argument_fingerprint &&
           default_argument_child_fingerprint == other.default_argument_child_fingerprint;
  }
};

struct TemplateParameterCacheKeyHash
{
  std::size_t operator()(const TemplateParameterCacheKey & key) const
  {
    return key.hash_value;
  }
};

typedef std::unordered_set<TemplateParameterCacheKey,
                           TemplateParameterCacheKeyHash>
    TemplateParameterCacheKeyPool;

struct ResolveTemplateArgumentsCacheKey
{
  std::size_t scope_instance_id = 0;
  std::uint64_t scope_binding_fingerprint = 0;
  std::size_t default_scope_instance_id = 0;
  std::uint64_t default_scope_binding_fingerprint = 0;
  std::vector<const TemplateParameterCacheKey *> parameters;
  std::vector<text_intern::Atom> texts;
  std::vector<std::uint64_t> syntax_fingerprints;
  std::size_t hash_value = 0;

  bool operator==(const ResolveTemplateArgumentsCacheKey & other) const
  {
    return scope_instance_id == other.scope_instance_id &&
           scope_binding_fingerprint == other.scope_binding_fingerprint &&
           default_scope_instance_id == other.default_scope_instance_id &&
           default_scope_binding_fingerprint == other.default_scope_binding_fingerprint &&
           parameters == other.parameters &&
           texts == other.texts &&
           syntax_fingerprints == other.syntax_fingerprints;
  }
};

struct ResolveTemplateArgumentsCacheProbe
{
  std::size_t scope_instance_id = 0;
  std::uint64_t scope_binding_fingerprint = 0;
  std::size_t default_scope_instance_id = 0;
  std::uint64_t default_scope_binding_fingerprint = 0;
  std::size_t parameter_count = 0;
  std::size_t text_count = 0;
  std::vector<std::uint64_t> syntax_fingerprints;
  std::size_t hash_value = 0;
};

struct ResolveTemplateArgumentsCacheKeyHash
{
  std::size_t operator()(const ResolveTemplateArgumentsCacheKey & key) const
  {
    return key.hash_value;
  }
};

std::uint64_t type_syntax_fingerprint(const TypePtr & type,
                                      bool include_source_identity);

template <typename Integer>
typename std::enable_if<std::is_integral<Integer>::value &&
                        !std::is_same<typename std::decay<Integer>::type, bool>::value>::type
hash_combine(std::size_t & seed, Integer value)
{
  seed ^= static_cast<std::size_t>(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

void hash_combine(std::size_t & seed, bool value)
{
  hash_combine(seed, static_cast<unsigned int>(value ? 1U : 0U));
}

void hash_combine(std::size_t & seed, const std::string & value)
{
  hash_combine(seed, std::hash<std::string>()(value));
}

void hash_type_ptr_for_scope_cache(std::size_t & seed, const TypePtr & type)
{
  hash_combine(seed, type_syntax_fingerprint(type, false));
}

void hash_value_binding_for_scope_cache(std::size_t & seed,
                                        const ValueBinding & binding)
{
  hash_combine(seed, static_cast<int>(binding.kind));
  hash_combine(seed, binding.name);
  hash_type_ptr_for_scope_cache(seed, binding.type);
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(binding.owner_class));
  hash_combine(seed, binding.has_constant_value ? 1 : 0);
  hash_combine(seed, binding.constant_value);
  hash_combine(seed, binding.dependent_template_value ? 1 : 0);
  hash_combine(seed, binding.non_type_template_argument_text);
  hash_combine(seed,
               reinterpret_cast<std::uintptr_t>(
                   binding.non_type_template_function_value));
  hash_combine(seed,
               reinterpret_cast<std::uintptr_t>(
                   binding.non_type_template_value_binding));
}

void hash_template_argument_for_scope_cache(std::size_t & seed,
                                            const TemplateArgument & argument)
{
  hash_combine(seed, static_cast<int>(argument.kind));
  hash_combine(seed, argument.dependent ? 1 : 0);
  hash_type_ptr_for_scope_cache(seed, argument.type);
  hash_combine(seed, argument.value);
  hash_combine(seed, argument.text);
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(argument.template_decl));
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(argument.function_value));
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(argument.value_binding));
}

template <typename MapT>
bool map_keys_are_subset_of_set(const MapT & values,
                                const std::set<std::string> & names)
{
  for(typename MapT::const_iterator it = values.begin();
      it != values.end();
      ++it) {
    if(names.count(it->first) == 0) {
      return false;
    }
  }
  return true;
}

bool pack_size_keys_are_template_bound(const Scope & scope)
{
  for(std::map<std::string, std::size_t>::const_iterator it =
          scope.named_pack_sizes.begin();
      it != scope.named_pack_sizes.end();
      ++it) {
    if(scope.template_bound_type_pack_names.count(it->first) == 0 &&
       scope.template_bound_value_pack_names.count(it->first) == 0) {
      return false;
    }
  }
  return true;
}

bool scope_has_only_template_bound_cacheable_bindings(const Scope & scope)
{
  return !scope.namespace_scope &&
         scope.named_type_access.empty() &&
         scope.namespace_bindings.empty() &&
         scope.function_sets.empty() &&
         scope.function_set_access_overrides.empty() &&
         scope.function_templates.empty() &&
         scope.variable_templates.empty() &&
         scope.using_directives.empty() &&
         scope.namespace_children.empty() &&
         scope.collected_template_declarations.empty() &&
         map_keys_are_subset_of_set(scope.named_types,
                                    scope.template_bound_type_names) &&
         map_keys_are_subset_of_set(scope.named_type_packs,
                                    scope.template_bound_type_pack_names) &&
         map_keys_are_subset_of_set(scope.values,
                                    scope.template_bound_value_names) &&
         map_keys_are_subset_of_set(scope.named_value_packs,
                                    scope.template_bound_value_pack_names) &&
         map_keys_are_subset_of_set(scope.class_templates,
                                    scope.template_bound_template_names) &&
         map_keys_are_subset_of_set(scope.alias_templates,
                                    scope.template_bound_template_names) &&
         map_keys_are_subset_of_set(scope.template_bound_template_arguments,
                                    scope.template_bound_template_names) &&
         pack_size_keys_are_template_bound(scope);
}

void hash_string_set_for_scope_cache(std::size_t & seed,
                                     const std::set<std::string> & values)
{
  hash_combine(seed, values.size());
  for(std::set<std::string>::const_iterator it = values.begin();
      it != values.end();
      ++it) {
    hash_combine(seed, *it);
  }
}

bool try_stable_template_bound_scope_cache_fingerprint(const Scope & scope,
                                                       std::uint64_t & out)
{
  out = 0;
  if(!scope_has_only_template_bound_cacheable_bindings(scope)) {
    return false;
  }

  std::size_t seed = 0;
  hash_combine(seed, scope.name);
  hash_combine(seed, scope.inline_namespace ? 1 : 0);
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(scope.class_info));
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(scope.function));

  hash_string_set_for_scope_cache(seed, scope.template_bound_type_names);
  for(std::map<std::string, TypePtr>::const_iterator it =
          scope.named_types.begin();
      it != scope.named_types.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_type_ptr_for_scope_cache(seed, it->second);
  }

  hash_string_set_for_scope_cache(seed, scope.template_bound_type_pack_names);
  for(std::map<std::string, std::vector<TypePtr> >::const_iterator it =
          scope.named_type_packs.begin();
      it != scope.named_type_packs.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_combine(seed, it->second.size());
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      hash_type_ptr_for_scope_cache(seed, it->second[i]);
    }
  }

  hash_string_set_for_scope_cache(seed, scope.template_bound_value_names);
  for(std::map<std::string, ValueBinding>::const_iterator it =
          scope.values.begin();
      it != scope.values.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_value_binding_for_scope_cache(seed, it->second);
  }

  hash_string_set_for_scope_cache(seed, scope.template_bound_value_pack_names);
  for(std::map<std::string, std::vector<ValueBinding> >::const_iterator it =
          scope.named_value_packs.begin();
      it != scope.named_value_packs.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_combine(seed, it->second.size());
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      hash_value_binding_for_scope_cache(seed, it->second[i]);
    }
  }

  for(std::map<std::string, std::size_t>::const_iterator it =
          scope.named_pack_sizes.begin();
      it != scope.named_pack_sizes.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_combine(seed, it->second);
  }

  hash_string_set_for_scope_cache(seed, scope.template_bound_template_names);
  for(std::map<std::string, ClassTemplateDecl *>::const_iterator it =
          scope.class_templates.begin();
      it != scope.class_templates.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_combine(seed, reinterpret_cast<std::uintptr_t>(it->second));
  }
  for(std::map<std::string, AliasTemplateDecl *>::const_iterator it =
          scope.alias_templates.begin();
      it != scope.alias_templates.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_combine(seed, reinterpret_cast<std::uintptr_t>(it->second));
  }
  for(std::map<std::string, TemplateArgument>::const_iterator it =
          scope.template_bound_template_arguments.begin();
      it != scope.template_bound_template_arguments.end();
      ++it) {
    hash_combine(seed, it->first);
    hash_template_argument_for_scope_cache(seed, it->second);
  }

  if(scope.parent) {
    std::uint64_t parent_stable = 0;
    if(try_stable_template_bound_scope_cache_fingerprint(*scope.parent,
                                                         parent_stable)) {
      hash_combine(seed, static_cast<std::size_t>(parent_stable));
    } else {
      hash_combine(seed, scope.parent->instance_id);
      hash_combine(seed, template_scope::scope_binding_fingerprint(*scope.parent));
    }
  }

  out = static_cast<std::uint64_t>(seed ? seed : 1);
  return true;
}

void fill_template_bound_aware_scope_cache_identity(
    const Scope & scope,
    std::size_t & instance_id,
    std::uint64_t & binding_fingerprint)
{
  if(try_stable_template_bound_scope_cache_fingerprint(scope,
                                                       binding_fingerprint)) {
    instance_id = 0;
    return;
  }
  instance_id = scope.instance_id;
  binding_fingerprint =
      static_cast<std::uint64_t>(template_scope::scope_binding_fingerprint(scope));
}

std::uint64_t template_argument_syntax_fingerprint(
    const TemplateArgumentSyntax * syntax,
    bool include_source_identity);

void attach_template_argument_source_syntax(
    const TemplateArgumentSyntax * syntax,
    TemplateArgument & out);

std::uint64_t cppast_node_syntax_fingerprint(const CppAstNode * node,
                                             bool include_source_identity);

std::uint64_t type_syntax_fingerprint(const TypePtr & type,
                                      bool include_source_identity,
                                      std::vector<const Type *> & active);

std::uint64_t type_syntax_fingerprint(const TypePtr & type,
                                      bool include_source_identity);

void hash_qualified_name_syntax(std::size_t & seed, const QualifiedName & name)
{
  hash_combine(seed, name.rooted);
  hash_combine(seed, name.name);
  hash_combine(seed, name.qualifiers.size());
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    hash_combine(seed, name.qualifiers[i]);
  }
}

bool function_template_deduction_type_cacheable_impl(
    const TypePtr & type,
    std::vector<const Type *> & active)
{
  if(!type) {
    return false;
  }
  if(std::find(active.begin(), active.end(), type.get()) != active.end()) {
    return true;
  }
  active.push_back(type.get());

  bool cacheable = false;
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    cacheable = true;
    break;
  case Type::TK_NAMED:
    cacheable =
        type->named_semantic_kind == Type::NSK_ORDINARY &&
        (type->definitely_not_class || type->named_complete);
    break;
  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    cacheable = function_template_deduction_type_cacheable_impl(type->inner, active);
    break;
  case Type::TK_MEMBER_POINTER:
    cacheable =
        function_template_deduction_type_cacheable_impl(type->inner, active) &&
        function_template_deduction_type_cacheable_impl(type->owner, active);
    break;
  case Type::TK_FUNCTION:
    cacheable = function_template_deduction_type_cacheable_impl(type->inner, active);
    for(std::size_t i = 0; cacheable && i < type->params.size(); ++i) {
      cacheable = function_template_deduction_type_cacheable_impl(type->params[i], active);
    }
    break;
  }

  active.pop_back();
  return cacheable;
}

std::unordered_set<TypePtr, TypePtrAddressHash, TypePtrAddressEqual> &
function_template_deduction_cacheable_type_cache()
{
  static std::unordered_set<TypePtr, TypePtrAddressHash, TypePtrAddressEqual> cache;
  return cache;
}

bool function_template_deduction_type_cacheable(
    const TypePtr & type,
    semantic_metrics::AnalyzerCounters * counters)
{
  if(!type) {
    return false;
  }
  if(counters) {
    ++counters->function_template_deduction_cacheable_type_checks;
  }
  std::unordered_set<TypePtr, TypePtrAddressHash, TypePtrAddressEqual> & cache =
      function_template_deduction_cacheable_type_cache();
  if(cache.count(type) != 0) {
    if(counters) {
      ++counters->function_template_deduction_cacheable_type_hits;
    }
    return true;
  }
  if(counters) {
    ++counters->function_template_deduction_cacheable_type_scans;
  }
  std::vector<const Type *> active;
  const bool cacheable =
      function_template_deduction_type_cacheable_impl(type, active);
  if(cacheable) {
    if(cache.size() > 262144) {
      if(counters) {
        ++counters->function_template_deduction_cacheable_type_clears;
      }
      cache.clear();
    }
    cache.insert(type);
    if(counters) {
      ++counters->function_template_deduction_cacheable_type_entries;
    }
  }
  return cacheable;
}

bool function_template_deduction_type_contains_local_named_type_impl(
    const TypePtr & type,
    std::vector<const Type *> & active)
{
  if(!type) {
    return false;
  }
  if(std::find(active.begin(), active.end(), type.get()) != active.end()) {
    return false;
  }
  active.push_back(type.get());

  bool found = false;
  switch(type->kind) {
  case Type::TK_NAMED:
    found =
        type->named_key.find("__local_") != std::string::npos ||
        type->named_display.find("__local_") != std::string::npos;
    break;
  case Type::TK_CV:
  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    found = function_template_deduction_type_contains_local_named_type_impl(
        type->inner, active);
    break;
  case Type::TK_MEMBER_POINTER:
    found =
        function_template_deduction_type_contains_local_named_type_impl(type->inner,
                                                                       active) ||
        function_template_deduction_type_contains_local_named_type_impl(type->owner,
                                                                       active);
    break;
  case Type::TK_FUNCTION:
    found = function_template_deduction_type_contains_local_named_type_impl(type->inner,
                                                                           active);
    for(std::size_t i = 0; !found && i < type->params.size(); ++i) {
      found = function_template_deduction_type_contains_local_named_type_impl(
          type->params[i], active);
    }
    break;
  case Type::TK_FUNDAMENTAL:
    found = false;
    break;
  }

  active.pop_back();
  return found;
}

bool function_template_deduction_type_contains_local_named_type(const TypePtr & type)
{
  std::vector<const Type *> active;
  return function_template_deduction_type_contains_local_named_type_impl(type, active);
}

bool function_template_deduction_use_scope_affects_cache_key(
    FunctionTemplateDecl & decl,
    const std::vector<ExprInfo> & args,
    Scope * use_scope)
{
  if(!use_scope) {
    return false;
  }
  if(!decl.declaring_scope ||
     !decl.declaring_scope->namespace_scope ||
     decl.lexical_access_class ||
     decl.is_constructor ||
     decl.is_destructor) {
    return true;
  }
  for(std::size_t i = 0; i < args.size(); ++i) {
    if(function_template_deduction_type_contains_local_named_type(args[i].type)) {
      return true;
    }
  }
  return false;
}

bool function_template_deduction_cache_allowed(SemanticContext & ctx,
                                               FunctionTemplateDecl & decl,
                                               const std::vector<ExprInfo> & args)
{
  semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
  if(counters) {
    ++counters->function_template_deduction_cache_allowed_checks;
  }
  if(parser_trace::enabled("template.resolve") ||
     witness::source_capture_enabled(ctx.template_witness_context()) ||
     decl.parameters.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < args.size(); ++i) {
    if(!function_template_deduction_type_cacheable(args[i].type, counters)) {
      if(counters) {
        ++counters->function_template_deduction_cache_uncacheable_arg_rejects;
      }
      return false;
    }
  }
  return true;
}

FunctionTemplateDeductionCacheKey make_function_template_deduction_cache_key(
    FunctionTemplateDecl & decl,
    const std::vector<ExprInfo> & args,
    Scope * use_scope,
    semantic_metrics::AnalyzerCounters * counters)
{
  FunctionTemplateDeductionCacheKey key;
  key.decl = &decl;
  if(counters) {
    ++counters->function_template_deduction_cache_key_builds;
    counters->function_template_deduction_cache_key_args += args.size();
  }
  if(function_template_deduction_use_scope_affects_cache_key(decl, args, use_scope)) {
    fill_template_bound_aware_scope_cache_identity(
        *use_scope,
        key.use_scope_instance_id,
        key.use_scope_binding_fingerprint);
    if(counters) {
      ++counters->function_template_deduction_cache_use_scope_sensitive_keys;
    }
  }
  if(decl.declaring_scope) {
    fill_template_bound_aware_scope_cache_identity(
        *decl.declaring_scope,
        key.declaring_scope_instance_id,
        key.declaring_scope_binding_fingerprint);
  }

  std::size_t seed = 0;
  hash_combine(seed, reinterpret_cast<std::uintptr_t>(key.decl));
  hash_combine(seed, key.use_scope_instance_id);
  hash_combine(seed, key.use_scope_binding_fingerprint);
  hash_combine(seed, key.declaring_scope_instance_id);
  hash_combine(seed, key.declaring_scope_binding_fingerprint);
  hash_combine(seed, args.size());

  key.args.reserve(args.size());
  for(std::size_t i = 0; i < args.size(); ++i) {
    FunctionTemplateDeductionArgCacheKey arg_key;
    arg_key.type = args[i].type;
    arg_key.type_fingerprint = type_syntax_fingerprint(args[i].type, false);
    arg_key.category = static_cast<int>(args[i].category);
    arg_key.null_pointer_constant = args[i].null_pointer_constant;
    key.args.push_back(arg_key);
    hash_combine(seed, arg_key.type_fingerprint);
    hash_combine(seed, arg_key.category);
    hash_combine(seed, arg_key.null_pointer_constant);
  }
  key.hash_value = seed;
  return key;
}

std::unordered_map<FunctionTemplateDeductionCacheKey,
                   FunctionTemplateDeductionCacheEntry,
                   FunctionTemplateDeductionCacheKeyHash> &
function_template_deduction_cache()
{
  static std::unordered_map<FunctionTemplateDeductionCacheKey,
                            FunctionTemplateDeductionCacheEntry,
                            FunctionTemplateDeductionCacheKeyHash> cache;
  return cache;
}

void note_function_template_deduction_cache_entry(
    const FunctionTemplateDeductionCacheKey & key,
    bool success,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> * pack_sizes,
    semantic_metrics::AnalyzerCounters * counters)
{
  std::unordered_map<FunctionTemplateDeductionCacheKey,
                     FunctionTemplateDeductionCacheEntry,
                     FunctionTemplateDeductionCacheKeyHash> & cache =
      function_template_deduction_cache();
  if(cache.size() > 262144) {
    if(counters) {
      ++counters->function_template_deduction_cache_clears;
    }
    cache.clear();
  }
  FunctionTemplateDeductionCacheEntry entry;
  entry.success = success;
  if(success) {
    entry.arguments = arguments;
    if(pack_sizes) {
      entry.pack_sizes = *pack_sizes;
    }
  }
  cache[key] = entry;
  if(counters) {
    ++counters->function_template_deduction_cache_entries;
  }
}

std::uint64_t template_id_syntax_fingerprint(
    const TemplateIdSyntax & syntax,
    bool include_source_identity)
{
  std::size_t seed = 0;
  hash_qualified_name_syntax(seed, syntax.name);
  if(include_source_identity) {
    hash_combine(seed, syntax.source_location_id);
  }
  hash_combine(seed, syntax.arguments.size());
  for(std::size_t i = 0; i < syntax.arguments.size(); ++i) {
    hash_combine(seed, syntax.arguments[i]);
  }
  hash_combine(seed, syntax.argument_syntaxes.size());
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    hash_combine(seed,
                 template_argument_syntax_fingerprint(
                     &syntax.argument_syntaxes[i],
                     include_source_identity));
  }
  return static_cast<std::uint64_t>(seed ? seed : 2);
}

void hash_cppast_node_vector_syntax(std::size_t & seed,
                                    const CppAstLazyVector<CppAstNode> & nodes,
                                    bool include_source_identity)
{
  const std::vector<CppAstNode> & values = nodes.as_vector();
  hash_combine(seed, values.size());
  for(std::size_t i = 0; i < values.size(); ++i) {
    hash_combine(seed,
                 cppast_node_syntax_fingerprint(&values[i],
                                                include_source_identity));
  }
}

void hash_template_id_vector_syntax(std::size_t & seed,
                                    const CppAstLazyVector<TemplateIdSyntax> & syntaxes,
                                    bool include_source_identity)
{
  const std::vector<TemplateIdSyntax> & values = syntaxes.as_vector();
  hash_combine(seed, values.size());
  for(std::size_t i = 0; i < values.size(); ++i) {
    hash_combine(seed,
                 template_id_syntax_fingerprint(values[i],
                                                include_source_identity));
  }
}

void hash_string_vector_syntax(std::size_t & seed,
                               const CppAstLazyVector<std::string> & values)
{
  const std::vector<std::string> & raw = values.as_vector();
  hash_combine(seed, raw.size());
  for(std::size_t i = 0; i < raw.size(); ++i) {
    hash_combine(seed, raw[i]);
  }
}

std::uint64_t type_syntax_fingerprint(const TypePtr & type,
                                      bool include_source_identity,
                                      std::vector<const Type *> & active)
{
  if(!type) {
    return 1;
  }

  if(include_source_identity) {
    return static_cast<std::uint64_t>(
        reinterpret_cast<std::uintptr_t>(type.get()) + 2);
  }

  const Type * raw = type.get();
  if(std::find(active.begin(), active.end(), raw) != active.end()) {
    return 3;
  }
  active.push_back(raw);

  std::size_t seed = 0;
  hash_combine(seed, static_cast<int>(type->kind));
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    hash_combine(seed, static_cast<int>(type->fundamental));
    break;

  case Type::TK_NAMED:
    hash_combine(seed, type->named_key);
    hash_combine(seed, type->named_complete);
    break;

  case Type::TK_CV:
    hash_combine(seed, type->cv_const);
    hash_combine(seed, type->cv_volatile);
    hash_combine(seed,
                 type_syntax_fingerprint(type->inner,
                                         include_source_identity,
                                         active));
    break;

  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    hash_combine(seed,
                 type_syntax_fingerprint(type->inner,
                                         include_source_identity,
                                         active));
    break;

  case Type::TK_MEMBER_POINTER:
    hash_combine(seed,
                 type_syntax_fingerprint(type->owner,
                                         include_source_identity,
                                         active));
    hash_combine(seed,
                 type_syntax_fingerprint(type->inner,
                                         include_source_identity,
                                         active));
    break;

  case Type::TK_ARRAY:
    hash_combine(seed, type->has_bound);
    hash_combine(seed, type->bound);
    hash_combine(seed, type->bound_text);
    hash_combine(seed,
                 type_syntax_fingerprint(type->inner,
                                         include_source_identity,
                                         active));
    break;

  case Type::TK_FUNCTION:
    hash_combine(seed, type->variadic);
    hash_combine(seed, type->prototype_relaxed);
    hash_combine(seed, type->function_const);
    hash_combine(seed, type->function_volatile);
    hash_combine(seed,
                 type_syntax_fingerprint(type->inner,
                                         include_source_identity,
                                         active));
    hash_combine(seed, type->params.size());
    for(std::size_t i = 0; i < type->params.size(); ++i) {
      hash_combine(seed,
                   type_syntax_fingerprint(type->params[i],
                                           include_source_identity,
                                           active));
    }
    break;
  }

  active.pop_back();
  return static_cast<std::uint64_t>(seed ? seed : 2);
}

std::uint64_t type_syntax_fingerprint(const TypePtr & type,
                                      bool include_source_identity)
{
  std::vector<const Type *> active;
  return type_syntax_fingerprint(type, include_source_identity, active);
}

std::uint64_t cppast_node_syntax_fingerprint(const CppAstNode * node,
                                             bool include_source_identity)
{
  if(!node) {
    return 1;
  }

  std::size_t seed = 0;
  hash_combine(seed, static_cast<int>(node->kind));
  hash_combine(seed, node->value);
  hash_combine(seed, node->builtin_type_transform_name);
  hash_combine(seed,
               type_syntax_fingerprint(node->semantic_type,
                                       include_source_identity));
  hash_combine(seed, node->qualified_name_syntax.get() != nullptr);
  if(node->qualified_name_syntax) {
    hash_qualified_name_syntax(seed, *node->qualified_name_syntax);
  }
  hash_combine(seed, node->template_id_syntax.get() != nullptr);
  if(node->template_id_syntax) {
    hash_combine(seed,
                 template_id_syntax_fingerprint(*node->template_id_syntax,
                                                include_source_identity));
  }
  hash_combine(seed,
               cppast_node_syntax_fingerprint(node->conversion_type_id_syntax.get(),
                                              include_source_identity));
  hash_combine(seed,
               cppast_node_syntax_fingerprint(node->base_type_syntax.get(),
                                              include_source_identity));
  hash_template_id_vector_syntax(seed,
                                 node->qualifier_template_id_syntaxes,
                                 include_source_identity);
  hash_cppast_node_vector_syntax(seed,
                                 node->qualifier_type_syntaxes,
                                 include_source_identity);
  hash_combine(seed, node->has_leading_typename);
  hash_combine(seed, node->has_exception_type_id_syntaxes);
  hash_cppast_node_vector_syntax(seed,
                                 node->exception_type_id_syntaxes,
                                 include_source_identity);
  hash_combine(seed, node->linkage_has_braces);
  hash_combine(seed, node->has_no_unique_address);
  hash_combine(seed, node->has_using_if_exists);
  hash_combine(seed, node->has_exclude_from_explicit_instantiation);
  hash_string_vector_syntax(seed, node->abi_tags);
  hash_string_vector_syntax(seed, node->alignment_specifiers);
  hash_cppast_node_vector_syntax(seed,
                                 node->alignment_specifier_nodes,
                                 include_source_identity);
  hash_combine(seed, node->is_final_specifier);
  hash_combine(seed, node->uses_assignment_form);
  hash_combine(seed, node->has_token);
  hash_combine(seed, static_cast<int>(node->token_kind));
  hash_combine(seed, static_cast<int>(node->simple_type));
  if(include_source_identity) {
    hash_combine(seed, node->token_start);
    hash_combine(seed, node->token_end);
    hash_combine(seed, node->source_location_id);
  }
  hash_combine(seed, node->children.size());
  for(std::size_t i = 0; i < node->children.size(); ++i) {
    hash_combine(seed,
                 cppast_node_syntax_fingerprint(&node->children[i],
                                                include_source_identity));
  }
  return static_cast<std::uint64_t>(seed ? seed : 2);
}

std::uint64_t template_argument_syntax_fingerprint(
    const TemplateArgumentSyntax * syntax,
    bool include_source_identity)
{
  if(!syntax) {
    return 1;
  }

  std::size_t seed = 0;
  hash_combine(seed, syntax->text);
  hash_combine(seed, syntax->pack_expansion);
  hash_combine(seed, syntax->dependent);
  if(include_source_identity) {
    hash_combine(seed, syntax->source_text);
    hash_combine(seed, syntax->has_source_token_start);
    hash_combine(seed, syntax->source_token_start);
    hash_combine(seed, syntax->source_location_id);
  }
  hash_combine(seed, syntax->template_id.get() != nullptr);
  if(syntax->template_id) {
    hash_combine(seed,
                 template_id_syntax_fingerprint(*syntax->template_id,
                                                include_source_identity));
  }
  hash_combine(seed, syntax->type_id.get() != nullptr);
  if(syntax->type_id) {
    hash_combine(seed,
                 cppast_node_syntax_fingerprint(syntax->type_id.get(),
                                                include_source_identity));
  }
  hash_combine(seed, syntax->expression.get() != nullptr);
  if(syntax->expression) {
    hash_combine(seed,
                 cppast_node_syntax_fingerprint(syntax->expression.get(),
                                                include_source_identity));
  }
  hash_combine(seed, syntax->resolved_type.get() != nullptr);
  if(syntax->resolved_type) {
    hash_combine(seed,
                 type_syntax_fingerprint(syntax->resolved_type,
                                         include_source_identity));
  }
  return static_cast<std::uint64_t>(seed ? seed : 2);
}

std::vector<std::uint64_t> template_argument_syntax_fingerprints(
    const template_argument_semantics::ExpandedTemplateArgumentInputs & inputs,
    bool include_source_identity)
{
  if(inputs.syntaxes.empty()) {
    return std::vector<std::uint64_t>();
  }
  std::vector<std::uint64_t> fingerprints;
  fingerprints.reserve(inputs.texts.size());
  for(std::size_t i = 0; i < inputs.texts.size(); ++i) {
    fingerprints.push_back(
        template_argument_syntax_fingerprint(inputs.syntax_for(i),
                                             include_source_identity));
  }
  return fingerprints;
}

bool type_name_has_actual_template_id_syntax(const CppAstNode & type_name)
{
  if(type_name.template_id_syntax &&
     !type_name.template_id_syntax->name.name.empty()) {
    return true;
  }
  for(std::size_t i = 0; i < type_name.qualifier_template_id_syntaxes.size(); ++i) {
    if(!type_name.qualifier_template_id_syntaxes[i].name.name.empty()) {
      return true;
    }
  }
  return false;
}

bool simple_type_argument_type_name_syntax(const TemplateArgumentSyntax * syntax,
                                           const CppAstNode *& type_name)
{
  type_name = nullptr;
  if(!syntax || !syntax->type_id) {
    return false;
  }
  const CppAstNode & type_id = *syntax->type_id;
  if(type_id.kind != CppAstKind::type_id ||
     type_id.children.size() != 1 ||
     type_id.children[0].kind != CppAstKind::type_specifier_seq ||
     type_id.children[0].children.size() != 1) {
    return false;
  }
  type_name = &type_id.children[0].children[0];
  return type_name->kind == CppAstKind::type_name;
}

bool simple_bound_member_type_argument_syntax(
    const TemplateArgumentSyntax * syntax,
    std::string & owner_name,
    std::string & member_name)
{
  owner_name.clear();
  member_name.clear();
  const CppAstNode * type_name = nullptr;
  if(!simple_type_argument_type_name_syntax(syntax, type_name) ||
     !type_name ||
     !type_name->has_leading_typename ||
     type_name_has_actual_template_id_syntax(*type_name)) {
    return false;
  }
  const QualifiedName * qualified = cppast_qualified_name_syntax(*type_name);
  if(!qualified ||
     qualified->rooted ||
     qualified->qualifiers.size() != 1 ||
     qualified->qualifiers[0].empty() ||
     qualified->name.empty()) {
    return false;
  }
  owner_name = qualified->qualifiers[0];
  member_name = qualified->name;
  return true;
}

bool stable_class_member_scope_lookup(const ClassInfo & info)
{
  return (info.complete || info.reference_members_collected) &&
         !info.full_member_collection_in_progress &&
         !info.reference_member_collection_in_progress;
}

struct StableBoundMemberFailureKey
{
  const void * model_id = nullptr;
  const Type * owner_type = nullptr;
  std::string member_name;

  bool operator==(const StableBoundMemberFailureKey & other) const
  {
    return model_id == other.model_id &&
           owner_type == other.owner_type &&
           member_name == other.member_name;
  }
};

struct StableBoundMemberFailureKeyHash
{
  std::size_t operator()(const StableBoundMemberFailureKey & key) const
  {
    std::size_t h = reinterpret_cast<std::uintptr_t>(key.model_id);
    h ^= reinterpret_cast<std::uintptr_t>(key.owner_type) +
         0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<std::string>()(key.member_name) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

std::unordered_set<StableBoundMemberFailureKey,
                   StableBoundMemberFailureKeyHash> &
stable_bound_member_failure_cache()
{
  static std::unordered_set<StableBoundMemberFailureKey,
                            StableBoundMemberFailureKeyHash> cache;
  return cache;
}

bool make_stable_bound_member_failure_key(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & owner_type,
    const std::string & member_name,
    StableBoundMemberFailureKey & out)
{
  out = StableBoundMemberFailureKey();
  TypePtr base = strip_top_level_cv(owner_type);
  if(!base ||
     member_name.empty() ||
     !type_system.model.classes_by_key) {
    return false;
  }
  out.model_id = type_system.model.classes_by_key;
  out.owner_type = base.get();
  out.member_name = member_name;
  return true;
}

bool try_resolve_member_type_on_known_owner(
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    const TypePtr & owner_type,
    const std::string & member_name,
    TypePtr & out,
    bool & determinate_failure,
    semantic_metrics::AnalyzerCounters * counters)
{
  out.reset();
  determinate_failure = false;
  if(!owner_type ||
     member_name.empty() ||
     template_argument_semantics::type_depends_on_template_parameter(
         type_system,
         owner_type)) {
    return false;
  }

  StableBoundMemberFailureKey failure_key;
  const bool have_failure_key =
      bound_member_failure_cache_enabled() &&
      make_stable_bound_member_failure_key(type_system,
                                           owner_type,
                                           member_name,
                                           failure_key);
  if(have_failure_key) {
    std::unordered_set<StableBoundMemberFailureKey,
                       StableBoundMemberFailureKeyHash> & cache =
        stable_bound_member_failure_cache();
    if(cache.find(failure_key) != cache.end()) {
      if(counters) {
        ++counters->bound_member_failure_cache_hits;
      }
      determinate_failure = true;
      return true;
    }
  }

  Scope * member_scope = nullptr;
  if(!type_system.prepare_named_type_member_scope(
         template_api::make_template_environment(raw_argument_scope),
         owner_type,
         member_scope) ||
     !member_scope) {
    return false;
  }

  if(member_scope->class_info) {
    std::map<std::string, TypePtr>::const_iterator direct =
        member_scope->named_types.find(member_name);
    if(direct != member_scope->named_types.end()) {
      out = direct->second;
      return out != nullptr;
    }
    if(!member_scope->class_info->bases.empty() &&
       type_system.resolve_member_type_lookup(raw_argument_scope,
                                              *member_scope->class_info,
                                              member_name,
                                              true,
                                              out) &&
       out) {
      return true;
    }
    if(!member_scope->class_info->complete &&
       !member_scope->class_info->template_instantiation_in_progress &&
       !member_scope->class_info->full_member_collection_in_progress) {
      Scope * completed_scope = nullptr;
      if(type_system.complete_named_type_member_scope(
             template_api::make_template_environment(raw_argument_scope),
             owner_type,
             completed_scope) &&
         completed_scope &&
         completed_scope->class_info) {
        member_scope = completed_scope;
        direct = member_scope->named_types.find(member_name);
        if(direct != member_scope->named_types.end()) {
          out = direct->second;
          return out != nullptr;
        }
        if(!member_scope->class_info->bases.empty() &&
           type_system.resolve_member_type_lookup(raw_argument_scope,
                                                  *member_scope->class_info,
                                                  member_name,
                                                  true,
                                                  out) &&
           out) {
          return true;
        }
      }
    }
    if(stable_class_member_scope_lookup(*member_scope->class_info)) {
      if(have_failure_key) {
        const bool inserted =
            stable_bound_member_failure_cache().insert(failure_key).second;
        if(inserted && counters) {
          ++counters->bound_member_failure_cache_entries;
        }
      }
      determinate_failure = true;
      return true;
    }
    return false;
  }

  out = template_api::lookup_direct_named_type_in_inline_namespaces(
      *member_scope,
      member_name);
  return out != nullptr;
}

bool try_resolve_bound_member_type_argument(
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    Scope & raw_parameter_scope,
    const TemplateArgumentSyntax * syntax,
    TypePtr & out,
    bool & determinate_failure,
    semantic_metrics::AnalyzerCounters * counters = nullptr)
{
  out.reset();
  determinate_failure = false;
  std::string owner_name;
  std::string member_name;
  if(!simple_bound_member_type_argument_syntax(syntax, owner_name, member_name)) {
    return false;
  }
  std::map<std::string, TypePtr>::const_iterator owner_found =
      raw_parameter_scope.named_types.find(owner_name);
  if(owner_found == raw_parameter_scope.named_types.end() ||
     !owner_found->second) {
    return false;
  }
  return try_resolve_member_type_on_known_owner(type_system,
                                               raw_argument_scope,
                                               owner_found->second,
                                               member_name,
                                               out,
                                               determinate_failure,
                                               counters);
}

bool type_trait_can_fail_from_missing_bound_member(const TemplateIdSyntax & syntax)
{
  return syntax.name.name == "is_same" ||
         syntax.name.name == "__is_same";
}

bool type_argument_has_stable_bound_member_failure(
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    const TemplateArgumentSyntax & syntax,
    semantic_metrics::AnalyzerCounters * counters)
{
  std::string owner_name;
  std::string member_name;
  if(!simple_bound_member_type_argument_syntax(&syntax, owner_name, member_name)) {
    return false;
  }

  TypePtr owner_type;
  if(!lookup_direct_bound_type_argument(raw_argument_scope, owner_name, owner_type) ||
     !owner_type) {
    return false;
  }

  TypePtr member_type;
  bool determinate_failure = false;
  return try_resolve_member_type_on_known_owner(type_system,
                                                raw_argument_scope,
                                                owner_type,
                                                member_name,
                                                member_type,
                                                determinate_failure,
                                                counters) &&
         determinate_failure;
}

bool non_type_trait_argument_has_stable_bound_member_failure(
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    const TemplateArgumentSyntax * syntax,
    semantic_metrics::AnalyzerCounters * counters)
{
  if(!syntax || !syntax->expression) {
    return false;
  }

  const CppAstNode & expr = *syntax->expression;
  const QualifiedName * qualified = cppast_qualified_name_syntax(expr);
  if(!qualified ||
     qualified->name != "value" ||
     qualified->qualifiers.empty()) {
    return false;
  }

  const TemplateIdSyntax * trait = nullptr;
  for(std::size_t offset = 0; offset < qualified->qualifiers.size(); ++offset) {
    const std::size_t qualifier_index = qualified->qualifiers.size() - 1 - offset;
    trait = cppast_qualifier_template_id_syntax(expr, qualifier_index);
    if(trait) {
      break;
    }
  }
  if(!trait ||
     !type_trait_can_fail_from_missing_bound_member(*trait) ||
     trait->argument_syntaxes.size() != trait->arguments.size()) {
    return false;
  }

  for(std::size_t i = 0; i < trait->argument_syntaxes.size(); ++i) {
    if(type_argument_has_stable_bound_member_failure(type_system,
                                                     raw_argument_scope,
                                                     trait->argument_syntaxes[i],
                                                     counters)) {
      return true;
    }
  }
  return false;
}

bool try_resolve_pre_expansion_known_failure(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<TemplateArgumentSyntax> * syntaxes)
{
  if(parser_trace::enabled("template.resolve") ||
     witness::source_capture_enabled(services.witness_context) ||
     !scope.valid() ||
     !syntaxes ||
     syntaxes->size() != texts.size()) {
    return false;
  }

  Scope & raw_scope = scope.require();
  const std::size_t limit = std::min(parameters.size(), texts.size());
  for(std::size_t i = 0; i < limit; ++i) {
    if(parameters[i].kind != TemplateParameterInfo::TP_NON_TYPE ||
       !parameters[i].value_type ||
       !is_bool_type(remove_reference_type(parameters[i].value_type))) {
      continue;
    }
    if(non_type_trait_argument_has_stable_bound_member_failure(
           type_system, raw_scope, &(*syntaxes)[i], services.counters)) {
      if(services.counters) {
        ++services.counters
              ->resolve_template_argument_pre_expansion_bound_member_failures;
        ++services.counters
              ->resolve_template_argument_pre_expansion_trait_bound_member_failures;
        ++services.counters->resolve_template_argument_bound_member_failures;
      }
      return true;
    }
  }
  return false;
}

bool strip_leading_typename_argument_text(const std::string & text,
                                          std::string & out)
{
  const std::string trimmed = trim_space(text);
  const std::string keyword = "typename";
  if(trimmed.size() <= keyword.size() ||
     trimmed.compare(0, keyword.size(), keyword) != 0 ||
     !std::isspace(static_cast<unsigned char>(trimmed[keyword.size()]))) {
    out = trimmed;
    return false;
  }
  out = trim_space(trimmed.substr(keyword.size()));
  return !out.empty();
}

bool simple_qualified_name_components(const QualifiedName & name)
{
  if(name.rooted ||
     name.qualifiers.empty() ||
     !is_identifier_text(name.name)) {
    return false;
  }
  for(std::size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(!is_identifier_text(name.qualifiers[i])) {
      return false;
    }
  }
  return true;
}

std::string qualified_component_at(const QualifiedName & qualified,
                                   std::size_t index)
{
  return index < qualified.qualifiers.size() ? qualified.qualifiers[index] :
                                               qualified.name;
}

bool qualified_component_prefix_name(const QualifiedName & qualified,
                                     std::size_t component_count,
                                     QualifiedName & out)
{
  out = QualifiedName();
  if(component_count == 0 ||
     component_count > qualified.qualifiers.size() + 1) {
    return false;
  }

  out.rooted = qualified.rooted;
  out.qualifiers.reserve(component_count - 1);
  for(std::size_t i = 0; i + 1 < component_count; ++i) {
    const std::string component = trim_space(qualified_component_at(qualified, i));
    if(component.empty()) {
      return false;
    }
    out.qualifiers.push_back(component);
  }
  out.name = trim_space(qualified_component_at(qualified, component_count - 1));
  return !out.name.empty();
}

bool qualified_component_has_template_arguments(const std::string & component)
{
  QualifiedName unused_name;
  std::vector<std::string> unused_arguments;
  return semantic_utils::split_top_level_template_id_text(trim_space(component),
                                                          unused_name,
                                                          unused_arguments) &&
         !unused_arguments.empty();
}

TemplateIdSyntax template_id_syntax_from_text_component(const std::string & text)
{
  TemplateIdSyntax syntax;
  QualifiedName name;
  std::vector<std::string> arguments;
  if(!semantic_utils::split_top_level_template_id_text(trim_space(text),
                                                       name,
                                                       arguments)) {
    return syntax;
  }

  syntax.name = name;
  syntax.arguments = arguments;
  syntax.argument_syntaxes.reserve(arguments.size());
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    TemplateArgumentSyntax argument;
    argument.text = trim_space(arguments[i]);
    syntax.argument_syntaxes.push_back(argument);
  }
  return syntax;
}

bool template_id_syntax_from_qualified_leaf_component(const QualifiedName & qualified,
                                                      TemplateIdSyntax & syntax)
{
  syntax = TemplateIdSyntax();
  TemplateIdSyntax leaf = template_id_syntax_from_text_component(qualified.name);
  if(leaf.name.name.empty()) {
    return false;
  }

  syntax = leaf;
  syntax.name.rooted = qualified.rooted || leaf.name.rooted;
  std::vector<std::string> qualifiers = qualified.qualifiers;
  qualifiers.insert(qualifiers.end(),
                    leaf.name.qualifiers.begin(),
                    leaf.name.qualifiers.end());
  syntax.name.qualifiers.swap(qualifiers);
  return true;
}

bool resolve_qualified_component_prefix_type(
    template_api::TemplateServices & services,
    Scope & raw_argument_scope,
    const QualifiedName & qualified,
    std::size_t owner_count,
    TypePtr & out)
{
  out.reset();
  QualifiedName prefix;
  if(!qualified_component_prefix_name(qualified, owner_count, prefix)) {
    return false;
  }

  for(std::size_t i = 0; i < prefix.qualifiers.size(); ++i) {
    if(qualified_component_has_template_arguments(prefix.qualifiers[i])) {
      return false;
    }
  }

  if(qualified_component_has_template_arguments(prefix.name)) {
    TemplateIdSyntax syntax;
    if(!template_id_syntax_from_qualified_leaf_component(prefix, syntax)) {
      return false;
    }
    return template_argument_semantics::resolve_template_id_syntax_type(
               services,
               raw_argument_scope,
               syntax,
               true,
               std::string(),
               out,
               template_api::make_template_environment(raw_argument_scope),
               template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly) &&
           out != nullptr;
  }

  template_api::TemplateTypeLookupRequest request;
  request.scope = &raw_argument_scope;
  request.allow_class_templates = true;
  request.name = prefix;
  return service_type_system(services).resolve_direct_type_lookup(request, out) && out;
}

bool try_resolve_template_qualified_dependent_member_type_argument(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    const std::string & text,
    TypePtr & out)
{
  out.reset();
  std::string lookup_text;
  const bool leading_typename =
      strip_leading_typename_argument_text(text, lookup_text);

  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(lookup_text, qualified) ||
     qualified.qualifiers.empty()) {
    return false;
  }

  const std::size_t component_count = qualified.qualifiers.size() + 1;
  for(std::size_t owner_count = component_count - 1;
      owner_count > 0;
      --owner_count) {
    TypePtr owner_type;
    if(!resolve_qualified_component_prefix_type(services,
                                                raw_argument_scope,
                                                qualified,
                                                owner_count,
                                                owner_type) ||
       !owner_type) {
      continue;
    }
    template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
        services,
        template_api::make_template_environment(raw_argument_scope),
        owner_type);
    if(!owner_type ||
       !template_argument_semantics::type_depends_on_template_parameter(
           type_system,
           owner_type)) {
      continue;
    }

    std::vector<std::string> member_path;
    std::vector<TemplateIdSyntax> member_template_ids;
    member_path.reserve(component_count - owner_count);
    member_template_ids.reserve(component_count - owner_count);
    for(std::size_t i = owner_count; i < component_count; ++i) {
      const std::string component =
          i < qualified.qualifiers.size() ? qualified.qualifiers[i] :
                                            qualified.name;
      member_path.push_back(component);
      member_template_ids.push_back(
          template_id_syntax_from_text_component(component));
    }
    if(member_path.empty()) {
      continue;
    }

    out = make_dependent_qualified_member_type(trim_space(text),
                                               owner_type,
                                               member_path,
                                               leading_typename,
                                               member_template_ids);
    return out != nullptr;
  }

  return false;
}

bool try_resolve_dependent_qualified_member_type_argument(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    Scope & raw_argument_scope,
    const std::string & text,
    TypePtr & out)
{
  out.reset();
  std::string lookup_text;
  const bool leading_typename =
      strip_leading_typename_argument_text(text, lookup_text);

  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(lookup_text, qualified) ||
     !simple_qualified_name_components(qualified)) {
    return try_resolve_template_qualified_dependent_member_type_argument(
        services,
        type_system,
        raw_argument_scope,
        text,
        out);
  }

  TypePtr owner_type;
  if(!lookup_direct_bound_type_argument(raw_argument_scope,
                                        qualified.qualifiers[0],
                                        owner_type) ||
     !owner_type ||
     !template_argument_semantics::type_depends_on_template_parameter(
         type_system,
         owner_type)) {
    return false;
  }

  std::vector<std::string> member_path;
  member_path.reserve(qualified.qualifiers.size());
  for(std::size_t i = 1; i < qualified.qualifiers.size(); ++i) {
    member_path.push_back(qualified.qualifiers[i]);
  }
  member_path.push_back(qualified.name);
  out = make_dependent_qualified_member_type(trim_space(text),
                                             owner_type,
                                             member_path,
                                             leading_typename);
  return out != nullptr;
}

bool template_id_argument_texts_have_recoverable_bound_type_argument(
    template_api::TemplateServices & services,
    Scope & raw_argument_scope,
    const std::vector<std::string> & arg_texts,
    unsigned depth = 0)
{
  (void)services;
  if(depth > 2) {
    return false;
  }

  for(std::size_t i = 0; i < arg_texts.size(); ++i) {
    const std::string arg = trim_space(arg_texts[i]);
    if(arg.empty()) {
      continue;
    }
    if(arg.find("typename ") != std::string::npos) {
      return true;
    }

    TypePtr bound_type;
    if((lookup_direct_bound_type_argument(raw_argument_scope, arg, bound_type) &&
        bound_type)) {
      return true;
    }

    QualifiedName nested_template_id;
    std::vector<std::string> nested_arg_texts;
    if(semantic_utils::split_top_level_template_id_text(arg,
                                                        nested_template_id,
                                                        nested_arg_texts) &&
       !nested_arg_texts.empty() &&
       template_id_argument_texts_have_recoverable_bound_type_argument(
           services,
           raw_argument_scope,
           nested_arg_texts,
           depth + 1)) {
      return true;
    }
  }

  return false;
}

bool resolve_recoverable_bound_template_id_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle argument_scope,
    const std::string & text,
    TypePtr & out)
{
  out.reset();
  Scope & raw_argument_scope = argument_scope.require();
  QualifiedName template_id;
  std::vector<std::string> arg_texts;
  if(!semantic_utils::split_top_level_template_id_text(trim_space(text),
                                                       template_id,
                                                       arg_texts) ||
     arg_texts.empty()) {
    return false;
  }
  if(!template_id_argument_texts_have_recoverable_bound_type_argument(
         services,
         raw_argument_scope,
         arg_texts)) {
    return false;
  }

  ClassTemplateDecl * class_template =
      template_argument_semantics::lookup_class_template(
          services,
          raw_argument_scope,
          template_api::qualified_name_text(template_id));
  if(!class_template) {
    return false;
  }

  std::vector<TemplateArgument> resolved_arguments;
  if(!template_api::resolve_template_arguments(
         services,
         argument_scope,
         class_template->parameters,
         arg_texts,
         nullptr,
         resolved_arguments,
         class_template->declaring_scope ?
             template_api::make_template_environment(*class_template->declaring_scope) :
             template_api::TemplateEnvironmentHandle())) {
    return false;
  }

  template_api::TemplateTypeLookupRequest lookup;
  lookup.scope = &raw_argument_scope;
  lookup.name = template_id;
  lookup.allow_class_templates = true;
  lookup.source_use_mode =
      template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly;

  template_api::TemplateSelectedClassTemplateIdRequest request;
  request.lookup = lookup;
  request.class_template = class_template;
  request.resolved_arguments.swap(resolved_arguments);
  request.source_arg_texts = arg_texts;
  return service_type_system(services).resolve_selected_class_template_id(request, out) && out;
}

std::size_t template_parameter_info_cache_hash(const TemplateParameterInfo & parameter)
{
  std::size_t seed = 0;
  hash_combine(seed, static_cast<int>(parameter.kind));
  hash_combine(seed, parameter.name);
  hash_combine(seed, parameter.parameter_pack);
  hash_combine(seed, parameter.template_parameter_count);
  hash_combine(seed, parameter.placeholder_key);
  hash_combine(seed, parameter.non_type_decl_specifier_text);
  hash_combine(seed, reinterpret_cast<std::size_t>(parameter.value_type.get()));
  hash_combine(seed, reinterpret_cast<std::size_t>(parameter.default_argument));
  if(parameter.default_argument && !parameter.default_argument->children.empty()) {
    hash_combine(seed,
                 reinterpret_cast<std::size_t>(&parameter.default_argument->children[0]));
  } else {
    hash_combine(seed, static_cast<std::size_t>(0));
  }
  return seed;
}

bool template_parameter_matches_cache_key(const TemplateParameterInfo & parameter,
                                          const TemplateParameterCacheKey & key)
{
  std::uint64_t default_argument_child_fingerprint = 0;
  if(parameter.default_argument && !parameter.default_argument->children.empty()) {
    default_argument_child_fingerprint =
        static_cast<std::uint64_t>(
            reinterpret_cast<std::uintptr_t>(&parameter.default_argument->children[0]));
  }
  return key.kind == static_cast<int>(parameter.kind) &&
         key.name == parameter.name &&
         key.parameter_pack == parameter.parameter_pack &&
         key.template_parameter_count == parameter.template_parameter_count &&
         key.placeholder_key == parameter.placeholder_key &&
         key.non_type_decl_specifier_text == parameter.non_type_decl_specifier_text &&
         key.value_type_fingerprint ==
             static_cast<std::uint64_t>(
                 reinterpret_cast<std::uintptr_t>(parameter.value_type.get())) &&
         key.default_argument_fingerprint ==
             static_cast<std::uint64_t>(
                 reinterpret_cast<std::uintptr_t>(parameter.default_argument)) &&
         key.default_argument_child_fingerprint == default_argument_child_fingerprint;
}

TemplateParameterCacheKey make_template_parameter_cache_key(const TemplateParameterInfo & parameter)
{
  TemplateParameterCacheKey key;
  key.hash_value = template_parameter_info_cache_hash(parameter);
  key.kind = static_cast<int>(parameter.kind);
  key.name = parameter.name;
  key.parameter_pack = parameter.parameter_pack;
  key.template_parameter_count = parameter.template_parameter_count;
  key.placeholder_key = parameter.placeholder_key;
  key.non_type_decl_specifier_text = parameter.non_type_decl_specifier_text;
  key.value_type_fingerprint =
      static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(parameter.value_type.get()));
  key.default_argument_fingerprint =
      static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(parameter.default_argument));
  if(parameter.default_argument && !parameter.default_argument->children.empty()) {
    key.default_argument_child_fingerprint =
        static_cast<std::uint64_t>(
            reinterpret_cast<std::uintptr_t>(&parameter.default_argument->children[0]));
  }
  return key;
}

TemplateParameterCacheKeyPool & template_parameter_cache_key_pool()
{
  static TemplateParameterCacheKeyPool pool;
  return pool;
}

const TemplateParameterCacheKey * intern_template_parameter_cache_key(
    const TemplateParameterInfo & parameter)
{
  TemplateParameterCacheKeyPool & pool = template_parameter_cache_key_pool();
  TemplateParameterCacheKey key = make_template_parameter_cache_key(parameter);
  return &*pool.insert(std::move(key)).first;
}

ResolveTemplateArgumentsCacheProbe make_resolve_template_arguments_cache_probe(
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & expanded_texts,
    const std::vector<std::uint64_t> & syntax_fingerprints,
    Scope * default_argument_declaring_scope)
{
  ResolveTemplateArgumentsCacheProbe probe;
  fill_template_bound_aware_scope_cache_identity(scope,
                                                 probe.scope_instance_id,
                                                 probe.scope_binding_fingerprint);
  if(default_argument_declaring_scope) {
    fill_template_bound_aware_scope_cache_identity(
        *default_argument_declaring_scope,
        probe.default_scope_instance_id,
        probe.default_scope_binding_fingerprint);
  }
  probe.parameter_count = parameters.size();
  probe.text_count = expanded_texts.size();
  probe.syntax_fingerprints = syntax_fingerprints;
  return probe;
}

std::size_t resolve_template_arguments_cache_hash(
    const ResolveTemplateArgumentsCacheProbe & probe,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & expanded_texts)
{
  std::size_t seed = 0;
  hash_combine(seed, probe.scope_instance_id);
  hash_combine(seed, probe.scope_binding_fingerprint);
  hash_combine(seed, probe.default_scope_instance_id);
  hash_combine(seed, probe.default_scope_binding_fingerprint);
  hash_combine(seed, parameters.size());
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    hash_combine(seed, template_parameter_info_cache_hash(parameters[i]));
  }
  hash_combine(seed, expanded_texts.size());
  for(std::size_t i = 0; i < expanded_texts.size(); ++i) {
    hash_combine(seed, expanded_texts[i]);
  }
  hash_combine(seed, probe.syntax_fingerprints.size());
  for(std::size_t i = 0; i < probe.syntax_fingerprints.size(); ++i) {
    hash_combine(seed, probe.syntax_fingerprints[i]);
  }
  return seed;
}

ResolveTemplateArgumentsCacheKey make_resolve_template_arguments_cache_key(
    const ResolveTemplateArgumentsCacheProbe & probe,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & expanded_texts)
{
  ResolveTemplateArgumentsCacheKey key;
  key.scope_instance_id = probe.scope_instance_id;
  key.scope_binding_fingerprint = probe.scope_binding_fingerprint;
  key.default_scope_instance_id = probe.default_scope_instance_id;
  key.default_scope_binding_fingerprint = probe.default_scope_binding_fingerprint;

  key.parameters.reserve(parameters.size());
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    key.parameters.push_back(intern_template_parameter_cache_key(parameters[i]));
  }

  key.texts.reserve(expanded_texts.size());
  for(std::size_t i = 0; i < expanded_texts.size(); ++i) {
    key.texts.push_back(text_intern::intern(expanded_texts[i]));
  }
  key.syntax_fingerprints = probe.syntax_fingerprints;
  key.hash_value = probe.hash_value;
  return key;
}

bool cache_key_matches_resolve_inputs(
    const ResolveTemplateArgumentsCacheKey & key,
    const ResolveTemplateArgumentsCacheProbe & probe,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & expanded_texts)
{
  if(key.scope_instance_id != probe.scope_instance_id ||
     key.scope_binding_fingerprint != probe.scope_binding_fingerprint ||
     key.default_scope_instance_id != probe.default_scope_instance_id ||
     key.default_scope_binding_fingerprint != probe.default_scope_binding_fingerprint ||
     key.parameters.size() != parameters.size() ||
     key.texts.size() != expanded_texts.size() ||
     key.syntax_fingerprints != probe.syntax_fingerprints) {
    return false;
  }
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!key.parameters[i] ||
       !template_parameter_matches_cache_key(parameters[i], *key.parameters[i])) {
      return false;
    }
  }
  for(std::size_t i = 0; i < expanded_texts.size(); ++i) {
    if(!key.texts[i] || *key.texts[i] != expanded_texts[i]) {
      return false;
    }
  }
  return true;
}

struct ResolveTemplateArgumentsFastCacheEntry
{
  bool valid = false;
  ResolveTemplateArgumentsCacheKey key;
  ResolveTemplateArgumentsCacheEntry entry;
};

std::vector<ResolveTemplateArgumentsFastCacheEntry> & resolve_template_arguments_fast_cache()
{
  static std::vector<ResolveTemplateArgumentsFastCacheEntry> cache(512);
  return cache;
}

std::size_t & resolve_template_arguments_fast_cache_cursor()
{
  static std::size_t cursor = 0;
  return cursor;
}

const ResolveTemplateArgumentsCacheEntry * find_resolve_template_arguments_fast_cache_entry(
    const ResolveTemplateArgumentsCacheProbe & probe,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & expanded_texts)
{
  std::vector<ResolveTemplateArgumentsFastCacheEntry> & cache =
      resolve_template_arguments_fast_cache();
  for(std::size_t i = 0; i < cache.size(); ++i) {
    if(cache[i].valid &&
       cache[i].key.hash_value == probe.hash_value &&
       cache_key_matches_resolve_inputs(cache[i].key,
                                        probe,
                                        parameters,
                                        expanded_texts)) {
      return &cache[i].entry;
    }
  }
  return nullptr;
}

void strip_cached_template_argument_source_syntaxes(
    ResolveTemplateArgumentsCacheEntry & entry)
{
  for(std::size_t i = 0; i < entry.arguments.size(); ++i) {
    entry.arguments[i].source_syntax.reset();
  }
}

ResolveTemplateArgumentsCacheEntry make_resolve_template_arguments_cache_entry(
    bool success,
    const std::vector<TemplateArgument> & arguments)
{
  ResolveTemplateArgumentsCacheEntry entry;
  entry.success = success;
  if(success) {
    entry.arguments = arguments;
    strip_cached_template_argument_source_syntaxes(entry);
  }
  return entry;
}

void reattach_template_argument_source_syntaxes_from_inputs(
    const template_argument_semantics::ExpandedTemplateArgumentInputs & inputs,
    std::vector<TemplateArgument> & arguments)
{
  const std::size_t explicit_count =
      std::min(arguments.size(), inputs.texts.size());
  for(std::size_t i = 0; i < explicit_count; ++i) {
    if(arguments[i].source_syntax || arguments[i].source_defaulted) {
      continue;
    }
    attach_template_argument_source_syntax(inputs.syntax_for(i), arguments[i]);
  }
}

void rehydrate_cached_defaulted_non_type_argument_witness_dependencies(
    template_api::TemplateServices & services,
    Scope & raw_scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const template_argument_semantics::ExpandedTemplateArgumentInputs & inputs,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope,
    std::vector<TemplateArgument> & arguments)
{
  if(!witness::enabled(services.witness_context) ||
     !services.semantic_context ||
     arguments.empty()) {
    return;
  }

  Scope bound_scope(&raw_scope, "", false);
  std::unique_ptr<Scope> default_argument_overlay;
  Scope * default_argument_scope = &bound_scope;
  if(default_argument_declaring_scope.scope &&
     default_argument_declaring_scope.scope != &raw_scope) {
    default_argument_overlay.reset(
        new Scope(default_argument_declaring_scope.scope, "", false));
    template_scope::overlay_ancestor_scope_bindings(*default_argument_overlay,
                                                    raw_scope,
                                                    default_argument_declaring_scope.scope,
                                                    template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
    default_argument_scope = default_argument_overlay.get();
  }

  const std::size_t count = std::min(parameters.size(), arguments.size());
  for(std::size_t i = 0; i < count; ++i) {
    TemplateArgument & argument = arguments[i];
    const TemplateParameterInfo & parameter = parameters[i];

    if(argument.source_defaulted &&
       parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
       !parameter.parameter_pack &&
       parameter.default_argument &&
       !parameter.default_argument->children.empty() &&
       argument.kind == TemplateArgument::TA_VALUE &&
       !argument.dependent) {
      const CppAstNode & child = parameter.default_argument->children[0];
      const std::string default_text = default_argument_expression_text(child);
      const TemplateArgumentSyntax default_syntax =
          make_default_template_argument_syntax(parameter, child, default_text);
      TemplateArgumentSyntax dependency_default_syntax = default_syntax;

      CppAstNode substituted_default_expression;
      if(template_argument_semantics::
             substitute_expression_node_for_template_arguments(
                 *default_argument_scope,
                 child,
                 parameters,
                 std::vector<TemplateArgument>(arguments.begin(),
                                               arguments.begin() + i),
                 substituted_default_expression)) {
        const std::string substituted_default_text =
            default_argument_expression_text(substituted_default_expression);
        dependency_default_syntax =
            make_default_template_argument_syntax(parameter,
                                                  substituted_default_expression,
                                                  substituted_default_text);
        if(!argument.expression) {
          argument.expression.reset(new CppAstNode(substituted_default_expression));
        }
      }

      if(!argument.source_syntax) {
        attach_template_argument_source_syntax(&default_syntax, argument);
      }

      const template_argument_semantics::ScopedDefaultTemplateArgumentEvaluation
          default_argument_evaluation;
      const template_api::TemplateEnvironmentHandle default_argument_env =
          template_api::make_template_environment(*default_argument_scope);
      TypePtr bound_value_type = argument.type;
      if(!bound_value_type) {
        (void)try_resolve_non_type_template_parameter_type(
            services, default_argument_env, parameter, bound_value_type);
      }

      template_argument_semantics::
          append_structured_bool_value_dependencies_in_template_argument_syntax(
              services,
              default_argument_env,
              dependency_default_syntax,
              argument.value_dependencies);
      template_argument_semantics::
          append_non_bool_static_value_dependencies_in_template_argument_syntax(
              services,
              default_argument_env,
              dependency_default_syntax,
              bound_value_type,
              argument.value_dependencies);
      template_argument_semantics::note_template_value_dependencies_for_witness(
          *services.semantic_context,
          argument.value_dependencies);
      template_argument_semantics::
          note_structured_bool_value_members_in_template_argument_syntax(
              services,
              default_argument_env,
              dependency_default_syntax);
    } else if(i < inputs.texts.size() && !argument.source_syntax) {
      attach_template_argument_source_syntax(inputs.syntax_for(i), argument);
    }

    bind_single_template_argument_into_scope(services,
                                             bound_scope,
                                             parameter,
                                             argument);
    if(default_argument_scope != &bound_scope) {
      bind_single_template_argument_into_scope(services,
                                               *default_argument_scope,
                                               parameter,
                                               argument);
    }
  }
}

void note_resolve_template_arguments_fast_cache_entry(
    const ResolveTemplateArgumentsCacheKey & key,
    const ResolveTemplateArgumentsCacheEntry & entry)
{
  std::vector<ResolveTemplateArgumentsFastCacheEntry> & cache =
      resolve_template_arguments_fast_cache();
  if(cache.empty()) {
    return;
  }
  std::size_t & cursor = resolve_template_arguments_fast_cache_cursor();
  ResolveTemplateArgumentsFastCacheEntry & slot = cache[cursor % cache.size()];
  slot.valid = true;
  slot.key = key;
  slot.entry = entry;
  cursor = (cursor + 1) % cache.size();
}

std::unordered_map<ResolveTemplateArgumentsCacheKey,
                   ResolveTemplateArgumentsCacheEntry,
                   ResolveTemplateArgumentsCacheKeyHash> &
resolve_template_arguments_cache()
{
  static std::unordered_map<ResolveTemplateArgumentsCacheKey,
                            ResolveTemplateArgumentsCacheEntry,
                            ResolveTemplateArgumentsCacheKeyHash> cache;
  return cache;
}

size_t cache_string_storage_bytes(const std::string & value)
{
  const char * data = value.data();
  const char * object_begin = reinterpret_cast<const char *>(&value);
  const char * object_end = object_begin + sizeof(value);
  if(data >= object_begin && data < object_end) {
    return 0;
  }
  return value.capacity() + 1;
}

template<class T>
size_t cache_vector_storage_bytes(const std::vector<T> & value)
{
  return value.capacity() * sizeof(T);
}

template<class T>
size_t cache_lazy_vector_storage_bytes(const CppAstLazyVector<T> & value)
{
  return cache_vector_storage_bytes(value.as_vector());
}

size_t qualified_name_cache_dynamic_bytes(const QualifiedName & name)
{
  size_t bytes = cache_vector_storage_bytes(name.qualifiers) +
                 cache_string_storage_bytes(name.name);
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    bytes += cache_string_storage_bytes(name.qualifiers[i]);
  }
  return bytes;
}

size_t template_argument_syntax_cache_dynamic_bytes(
    const TemplateArgumentSyntax & syntax,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes);

size_t template_argument_syntax_cache_owned_bytes(
    const TemplateArgumentSyntax * syntax,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  if(!syntax || !seen_syntaxes.insert(syntax).second) {
    return 0;
  }
  return sizeof(TemplateArgumentSyntax) +
         template_argument_syntax_cache_dynamic_bytes(*syntax,
                                                      seen_syntaxes,
                                                      seen_template_ids,
                                                      seen_ast_nodes);
}

size_t template_id_syntax_cache_dynamic_bytes(
    const TemplateIdSyntax & syntax,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  size_t bytes = qualified_name_cache_dynamic_bytes(syntax.name) +
                 cache_vector_storage_bytes(syntax.arguments) +
                 cache_vector_storage_bytes(syntax.argument_syntaxes);
  for(size_t i = 0; i < syntax.arguments.size(); ++i) {
    bytes += cache_string_storage_bytes(syntax.arguments[i]);
  }
  for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    bytes += template_argument_syntax_cache_dynamic_bytes(
        syntax.argument_syntaxes[i],
        seen_syntaxes,
        seen_template_ids,
        seen_ast_nodes);
  }
  return bytes;
}

size_t template_id_syntax_cache_owned_bytes(
    const TemplateIdSyntax * syntax,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  if(!syntax || !seen_template_ids.insert(syntax).second) {
    return 0;
  }
  return sizeof(TemplateIdSyntax) +
         template_id_syntax_cache_dynamic_bytes(*syntax,
                                                seen_syntaxes,
                                                seen_template_ids,
                                                seen_ast_nodes);
}

size_t cppast_node_cache_dynamic_bytes(
    const CppAstNode & node,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes);

size_t cppast_node_cache_owned_bytes(
    const CppAstNode * node,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  if(!node || !seen_ast_nodes.insert(node).second) {
    return 0;
  }
  return sizeof(CppAstNode) +
         cppast_node_cache_dynamic_bytes(*node,
                                         seen_syntaxes,
                                         seen_template_ids,
                                         seen_ast_nodes);
}

size_t cppast_node_cache_dynamic_bytes(
    const CppAstNode & node,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  size_t bytes = cache_string_storage_bytes(node.value) +
                 cache_string_storage_bytes(node.builtin_type_transform_name);
  if(node.qualified_name_syntax) {
    bytes += sizeof(QualifiedName) +
             qualified_name_cache_dynamic_bytes(*node.qualified_name_syntax);
  }
  bytes += template_id_syntax_cache_owned_bytes(node.template_id_syntax.get(),
                                                seen_syntaxes,
                                                seen_template_ids,
                                                seen_ast_nodes);
  bytes += cppast_node_cache_owned_bytes(node.conversion_type_id_syntax.get(),
                                         seen_syntaxes,
                                         seen_template_ids,
                                         seen_ast_nodes);
  bytes += cppast_node_cache_owned_bytes(node.base_type_syntax.get(),
                                         seen_syntaxes,
                                         seen_template_ids,
                                         seen_ast_nodes);

  bytes += cache_lazy_vector_storage_bytes(node.qualifier_template_id_syntaxes);
  const std::vector<TemplateIdSyntax> & qualifier_template_ids =
      node.qualifier_template_id_syntaxes.as_vector();
  for(size_t i = 0; i < qualifier_template_ids.size(); ++i) {
    bytes += template_id_syntax_cache_dynamic_bytes(qualifier_template_ids[i],
                                                    seen_syntaxes,
                                                    seen_template_ids,
                                                    seen_ast_nodes);
  }

  bytes += cache_lazy_vector_storage_bytes(node.qualifier_type_syntaxes);
  const std::vector<CppAstNode> & qualifier_type_syntaxes =
      node.qualifier_type_syntaxes.as_vector();
  for(size_t i = 0; i < qualifier_type_syntaxes.size(); ++i) {
    bytes += cppast_node_cache_dynamic_bytes(qualifier_type_syntaxes[i],
                                             seen_syntaxes,
                                             seen_template_ids,
                                             seen_ast_nodes);
  }

  bytes += cache_lazy_vector_storage_bytes(node.exception_type_id_syntaxes);
  const std::vector<CppAstNode> & exception_type_id_syntaxes =
      node.exception_type_id_syntaxes.as_vector();
  for(size_t i = 0; i < exception_type_id_syntaxes.size(); ++i) {
    bytes += cppast_node_cache_dynamic_bytes(exception_type_id_syntaxes[i],
                                             seen_syntaxes,
                                             seen_template_ids,
                                             seen_ast_nodes);
  }

  bytes += cache_lazy_vector_storage_bytes(node.abi_tags);
  const std::vector<std::string> & abi_tags = node.abi_tags.as_vector();
  for(size_t i = 0; i < abi_tags.size(); ++i) {
    bytes += cache_string_storage_bytes(abi_tags[i]);
  }

  bytes += cache_lazy_vector_storage_bytes(node.alignment_specifiers);
  const std::vector<std::string> & alignment_specifiers =
      node.alignment_specifiers.as_vector();
  for(size_t i = 0; i < alignment_specifiers.size(); ++i) {
    bytes += cache_string_storage_bytes(alignment_specifiers[i]);
  }

  bytes += cache_lazy_vector_storage_bytes(node.alignment_specifier_nodes);
  const std::vector<CppAstNode> & alignment_specifier_nodes =
      node.alignment_specifier_nodes.as_vector();
  for(size_t i = 0; i < alignment_specifier_nodes.size(); ++i) {
    bytes += cppast_node_cache_dynamic_bytes(alignment_specifier_nodes[i],
                                             seen_syntaxes,
                                             seen_template_ids,
                                             seen_ast_nodes);
  }

  bytes += cache_vector_storage_bytes(node.children);
  for(size_t i = 0; i < node.children.size(); ++i) {
    bytes += cppast_node_cache_dynamic_bytes(node.children[i],
                                             seen_syntaxes,
                                             seen_template_ids,
                                             seen_ast_nodes);
  }
  return bytes;
}

size_t template_argument_syntax_cache_dynamic_bytes(
    const TemplateArgumentSyntax & syntax,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  return cache_string_storage_bytes(syntax.text) +
         cache_string_storage_bytes(syntax.source_text) +
         template_id_syntax_cache_owned_bytes(syntax.template_id.get(),
                                              seen_syntaxes,
                                              seen_template_ids,
                                              seen_ast_nodes) +
         cppast_node_cache_owned_bytes(syntax.type_id.get(),
                                       seen_syntaxes,
                                       seen_template_ids,
                                       seen_ast_nodes) +
         cppast_node_cache_owned_bytes(syntax.expression.get(),
                                       seen_syntaxes,
                                       seen_template_ids,
                                       seen_ast_nodes);
}

void template_argument_cache_payload_bytes(
    const TemplateArgument & argument,
    size_t & text_bytes,
    size_t & source_syntax_bytes,
    size_t & expression_bytes,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  text_bytes += cache_string_storage_bytes(argument.text);
  source_syntax_bytes += template_argument_syntax_cache_owned_bytes(
      argument.source_syntax.get(),
      seen_syntaxes,
      seen_template_ids,
      seen_ast_nodes);
  expression_bytes += cppast_node_cache_owned_bytes(argument.expression.get(),
                                                    seen_syntaxes,
                                                    seen_template_ids,
                                                    seen_ast_nodes);
}

size_t template_parameter_cache_key_payload_bytes(
    const TemplateParameterCacheKey & parameter,
    size_t & string_bytes)
{
  string_bytes += cache_string_storage_bytes(parameter.name);
  string_bytes += cache_string_storage_bytes(parameter.placeholder_key);
  string_bytes += cache_string_storage_bytes(parameter.non_type_decl_specifier_text);
  return 0;
}

size_t resolve_template_cache_key_dynamic_bytes(
    const ResolveTemplateArgumentsCacheKey & key,
    size_t & parameter_vector_bytes,
    size_t & parameter_string_bytes,
    size_t & text_vector_bytes,
    size_t & text_string_bytes,
    size_t & syntax_fingerprint_bytes)
{
  (void)parameter_string_bytes;
  (void)text_string_bytes;
  parameter_vector_bytes += cache_vector_storage_bytes(key.parameters);
  text_vector_bytes += cache_vector_storage_bytes(key.texts);
  syntax_fingerprint_bytes += cache_vector_storage_bytes(key.syntax_fingerprints);
  return 0;
}

size_t resolve_template_cache_entry_dynamic_bytes(
    const ResolveTemplateArgumentsCacheEntry & entry,
    size_t & argument_vector_bytes,
    size_t & argument_text_bytes,
    size_t & argument_source_syntax_bytes,
    size_t & argument_expression_bytes,
    std::unordered_set<const TemplateArgumentSyntax *> & seen_syntaxes,
    std::unordered_set<const TemplateIdSyntax *> & seen_template_ids,
    std::unordered_set<const CppAstNode *> & seen_ast_nodes)
{
  argument_vector_bytes += cache_vector_storage_bytes(entry.arguments);
  for(size_t i = 0; i < entry.arguments.size(); ++i) {
    template_argument_cache_payload_bytes(entry.arguments[i],
                                          argument_text_bytes,
                                          argument_source_syntax_bytes,
                                          argument_expression_bytes,
                                          seen_syntaxes,
                                          seen_template_ids,
                                          seen_ast_nodes);
  }
  return 0;
}

void dump_template_resolution_memory_line(std::ostream & out,
                                          const char * prefix,
                                          const std::string & kind,
                                          size_t count,
                                          size_t bytes)
{
  out << prefix
      << " kind=" << kind
      << " count=" << count
      << " bytes=" << bytes
      << '\n';
}

void dump_template_resolution_cache_memory_census_impl(std::ostream & out)
{
  typedef std::unordered_map<ResolveTemplateArgumentsCacheKey,
                             ResolveTemplateArgumentsCacheEntry,
                             ResolveTemplateArgumentsCacheKeyHash> Cache;
  Cache & cache = resolve_template_arguments_cache();

  size_t map_bucket_bytes = cache.bucket_count() * sizeof(void *);
  size_t map_node_bytes = cache.size() * sizeof(Cache::value_type);
  size_t key_parameter_vector_bytes = 0;
  size_t key_parameter_string_bytes = 0;
  size_t key_text_vector_bytes = 0;
  size_t key_text_string_bytes = 0;
  size_t key_syntax_fingerprint_bytes = 0;
  size_t entry_argument_vector_bytes = 0;
  size_t entry_argument_text_bytes = 0;
  size_t entry_argument_source_syntax_bytes = 0;
  size_t entry_argument_expression_bytes = 0;
  std::unordered_set<const TemplateArgumentSyntax *> seen_syntaxes;
  std::unordered_set<const TemplateIdSyntax *> seen_template_ids;
  std::unordered_set<const CppAstNode *> seen_ast_nodes;

  for(Cache::const_iterator it = cache.begin(); it != cache.end(); ++it) {
    resolve_template_cache_key_dynamic_bytes(it->first,
                                             key_parameter_vector_bytes,
                                             key_parameter_string_bytes,
                                             key_text_vector_bytes,
                                             key_text_string_bytes,
                                               key_syntax_fingerprint_bytes);
    resolve_template_cache_entry_dynamic_bytes(it->second,
                                               entry_argument_vector_bytes,
                                               entry_argument_text_bytes,
                                               entry_argument_source_syntax_bytes,
                                               entry_argument_expression_bytes,
                                               seen_syntaxes,
                                               seen_template_ids,
                                               seen_ast_nodes);
  }

  TemplateParameterCacheKeyPool & parameter_key_pool =
      template_parameter_cache_key_pool();
  const size_t parameter_key_pool_bucket_bytes =
      parameter_key_pool.bucket_count() * sizeof(void *);
  const size_t parameter_key_pool_node_bytes =
      parameter_key_pool.size() * sizeof(TemplateParameterCacheKeyPool::value_type);
  size_t parameter_key_pool_string_bytes = 0;
  for(TemplateParameterCacheKeyPool::const_iterator it = parameter_key_pool.begin();
      it != parameter_key_pool.end();
      ++it) {
    template_parameter_cache_key_payload_bytes(*it, parameter_key_pool_string_bytes);
  }
  const size_t parameter_key_pool_bytes =
      parameter_key_pool_bucket_bytes +
      parameter_key_pool_node_bytes +
      parameter_key_pool_string_bytes;

  const size_t cache_bytes =
      map_bucket_bytes +
      map_node_bytes +
      key_parameter_vector_bytes +
      key_parameter_string_bytes +
      parameter_key_pool_bytes +
      key_text_vector_bytes +
      key_text_string_bytes +
      key_syntax_fingerprint_bytes +
      entry_argument_vector_bytes +
      entry_argument_text_bytes +
      entry_argument_source_syntax_bytes +
      entry_argument_expression_bytes;

  std::vector<ResolveTemplateArgumentsFastCacheEntry> & fast_cache =
      resolve_template_arguments_fast_cache();
  size_t fast_valid_count = 0;
  size_t fast_key_parameter_vector_bytes = 0;
  size_t fast_key_parameter_string_bytes = 0;
  size_t fast_key_text_vector_bytes = 0;
  size_t fast_key_text_string_bytes = 0;
  size_t fast_key_syntax_fingerprint_bytes = 0;
  size_t fast_entry_argument_vector_bytes = 0;
  size_t fast_entry_argument_text_bytes = 0;
  size_t fast_entry_argument_source_syntax_bytes = 0;
  size_t fast_entry_argument_expression_bytes = 0;
  std::unordered_set<const TemplateArgumentSyntax *> fast_seen_syntaxes;
  std::unordered_set<const TemplateIdSyntax *> fast_seen_template_ids;
  std::unordered_set<const CppAstNode *> fast_seen_ast_nodes;
  for(size_t i = 0; i < fast_cache.size(); ++i) {
    if(!fast_cache[i].valid) {
      continue;
    }
    ++fast_valid_count;
    resolve_template_cache_key_dynamic_bytes(
        fast_cache[i].key,
        fast_key_parameter_vector_bytes,
        fast_key_parameter_string_bytes,
        fast_key_text_vector_bytes,
        fast_key_text_string_bytes,
        fast_key_syntax_fingerprint_bytes);
    resolve_template_cache_entry_dynamic_bytes(
        fast_cache[i].entry,
        fast_entry_argument_vector_bytes,
        fast_entry_argument_text_bytes,
        fast_entry_argument_source_syntax_bytes,
        fast_entry_argument_expression_bytes,
        fast_seen_syntaxes,
        fast_seen_template_ids,
        fast_seen_ast_nodes);
  }
  const size_t fast_storage_bytes = cache_vector_storage_bytes(fast_cache);
  const size_t fast_bytes =
      fast_storage_bytes +
      fast_key_parameter_vector_bytes +
      fast_key_parameter_string_bytes +
      fast_key_text_vector_bytes +
      fast_key_text_string_bytes +
      fast_key_syntax_fingerprint_bytes +
      fast_entry_argument_vector_bytes +
      fast_entry_argument_text_bytes +
      fast_entry_argument_source_syntax_bytes +
      fast_entry_argument_expression_bytes;

  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global",
      "template_resolution.resolve_template_arguments_cache",
      cache.size(),
      cache_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global",
      "template_resolution.resolve_template_arguments_fast_cache",
      fast_valid_count,
      fast_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.map_buckets",
      cache.bucket_count(),
      map_bucket_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.map_nodes",
      cache.size(),
      map_node_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_vector_storage",
      cache.size(),
      key_parameter_vector_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_string_capacity",
      cache.size(),
      key_parameter_string_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_intern_pool",
      parameter_key_pool.size(),
      parameter_key_pool_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_intern_buckets",
      parameter_key_pool.bucket_count(),
      parameter_key_pool_bucket_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_intern_nodes",
      parameter_key_pool.size(),
      parameter_key_pool_node_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.parameter_intern_string_capacity",
      parameter_key_pool.size(),
      parameter_key_pool_string_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.text_vector_storage",
      cache.size(),
      key_text_vector_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.text_string_capacity",
      cache.size(),
      key_text_string_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.key.syntax_fingerprint_storage",
      cache.size(),
      key_syntax_fingerprint_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.entry.argument_vector_storage",
      cache.size(),
      entry_argument_vector_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.entry.argument_text_capacity",
      cache.size(),
      entry_argument_text_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.entry.argument_source_syntax",
      seen_syntaxes.size(),
      entry_argument_source_syntax_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_cache.entry.argument_expression_ast",
      seen_ast_nodes.size(),
      entry_argument_expression_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_fast_cache.storage",
      fast_cache.size(),
      fast_storage_bytes);
  dump_template_resolution_memory_line(
      out,
      "semantic-memory-global-detail",
      "template_resolution.resolve_template_arguments_fast_cache.entry.argument_source_syntax",
      fast_seen_syntaxes.size(),
      fast_entry_argument_source_syntax_bytes);
}

std::string join_template_texts(const std::vector<std::string> & texts)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < texts.size(); ++i) {
    if(i != 0) {
      out << ",";
    }
    out << trim_space(texts[i]);
  }
  return out.str();
}

bool has_invalid_top_level_qualified_owner_syntax(const std::string & text)
{
  int angle_depth = 0;
  int paren_depth = 0;
  int bracket_depth = 0;
  int brace_depth = 0;
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

struct DebugSkipTemplateParameterShadowCleanupConfig
{
  bool enabled_all = false;
  std::unordered_set<std::string> names;
};

const DebugSkipTemplateParameterShadowCleanupConfig &
debug_skip_template_parameter_shadow_cleanup_config()
{
  static const DebugSkipTemplateParameterShadowCleanupConfig config = []()
  {
    DebugSkipTemplateParameterShadowCleanupConfig out;
    const char * value = std::getenv("CPPGM_DEBUG_SKIP_TEMPLATE_PARAMETER_SHADOW_CLEANUP");
    if(!value || !*value) {
      return out;
    }
    const std::string configured = value;
    if(configured == "0") {
      return out;
    }
    if(configured == "1" || configured == "all" || configured == "true") {
      out.enabled_all = true;
      return out;
    }
    std::string current;
    for(std::size_t i = 0; i <= configured.size(); ++i) {
      if(i == configured.size() || configured[i] == ',' || configured[i] == ' ' ||
         configured[i] == '\t') {
        if(!current.empty()) {
          out.names.insert(current);
          current.clear();
        }
        continue;
      }
      current += configured[i];
    }
    return out;
  }();
  return config;
}

bool debug_skip_template_parameter_shadow_cleanup(const FunctionTemplateDecl & decl)
{
  const DebugSkipTemplateParameterShadowCleanupConfig & config =
      debug_skip_template_parameter_shadow_cleanup_config();
  return config.enabled_all || config.names.count(decl.name) != 0;
}

std::set<std::string> function_template_parameter_names(
    const std::vector<TemplateParameterInfo> & parameters)
{
  std::set<std::string> names;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty()) {
      names.insert(parameters[i].name);
    }
  }
  return names;
}

void append_declaring_scope_template_bound_names(const Scope * scope,
                                                 std::set<std::string> & names)
{
  for(const Scope * current = scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    names.insert(current->template_bound_type_names.begin(),
                 current->template_bound_type_names.end());
    names.insert(current->template_bound_type_pack_names.begin(),
                 current->template_bound_type_pack_names.end());
    names.insert(current->template_bound_value_names.begin(),
                 current->template_bound_value_names.end());
    names.insert(current->template_bound_template_names.begin(),
                 current->template_bound_template_names.end());
  }
}

std::set<std::string> deduction_overlay_excluded_names(const FunctionTemplateDecl & decl)
{
  std::set<std::string> names = function_template_parameter_names(decl.parameters);
  append_declaring_scope_template_bound_names(decl.declaring_scope, names);
  return names;
}

bool contains_identifier_token(const std::string & text, const std::string & name)
{
  if(name.empty()) {
    return false;
  }

  std::size_t pos = 0;
  while((pos = text.find(name, pos)) != std::string::npos) {
    const bool left_ok =
        pos == 0 ||
        !(std::isalnum(static_cast<unsigned char>(text[pos - 1])) || text[pos - 1] == '_');
    const std::size_t end = pos + name.size();
    const bool right_ok =
        end == text.size() ||
        !(std::isalnum(static_cast<unsigned char>(text[end])) || text[end] == '_');
    if(left_ok && right_ok) {
      return true;
    }
    pos = end;
  }
  return false;
}

bool is_identifier_text(const std::string & text)
{
  if(text.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < text.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(text[i]);
    if(!(std::isalnum(ch) || ch == '_')) {
      return false;
    }
  }
  return true;
}

bool parse_simple_identifier_pack_expansion_text(const std::string & text,
                                                 std::string & name,
                                                 bool & pack_expansion)
{
  std::string trimmed = trim_space(text);
  pack_expansion = false;
  if(trimmed.size() > 3 && trimmed.substr(trimmed.size() - 3) == "...") {
    pack_expansion = true;
    trimmed = trim_space(trimmed.substr(0, trimmed.size() - 3));
  }
  if(!is_identifier_text(trimmed)) {
    return false;
  }
  name = trimmed;
  return true;
}

bool simple_non_type_argument_names_type_placeholder(Scope & scope,
                                                     const std::string & text)
{
  std::string name;
  bool pack_expansion = false;
  if(!parse_simple_identifier_pack_expansion_text(text, name, pack_expansion)) {
    return false;
  }

  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }

    if(pack_expansion) {
      if(current->named_value_packs.count(name) != 0) {
        return false;
      }
      if(current->named_type_packs.count(name) != 0 ||
         current->template_bound_type_pack_names.count(name) != 0 ||
         current->named_types.count(name) != 0 ||
         current->template_bound_type_names.count(name) != 0) {
        return true;
      }
      continue;
    }

    if(current->values.count(name) != 0 ||
       current->template_bound_value_names.count(name) != 0) {
      return false;
    }
    if(current->named_types.count(name) != 0 ||
       current->template_bound_type_names.count(name) != 0 ||
       current->named_type_packs.count(name) != 0 ||
       current->template_bound_type_pack_names.count(name) != 0) {
      return true;
    }
  }

  return false;
}

bool non_type_dependency_literal_name(const std::string & name)
{
  return name == "true" ||
         name == "false" ||
         name == "nullptr" ||
         name == "__null" ||
         name == "this" ||
         name == "__func__" ||
         name == "__FUNCTION__" ||
         name == "__PRETTY_FUNCTION__";
}

bool scope_contains_any_unqualified_binding(
    template_api::TemplateServices & services,
    Scope & scope,
    const std::string & name)
{
  if(name.empty() ||
     !is_identifier_text(name)) {
    return false;
  }
  if(lookup_unqualified_value(services, scope, name)) {
    return true;
  }
  if(!lookup_unqualified_functions(scope, name).empty()) {
    return true;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->named_types.find(name) != current->named_types.end() ||
       current->named_type_packs.find(name) != current->named_type_packs.end() ||
       current->named_pack_sizes.find(name) != current->named_pack_sizes.end() ||
       current->template_bound_type_names.find(name) !=
           current->template_bound_type_names.end() ||
       current->template_bound_type_pack_names.find(name) !=
           current->template_bound_type_pack_names.end() ||
       current->template_bound_value_names.find(name) !=
           current->template_bound_value_names.end() ||
       current->template_bound_template_names.find(name) !=
           current->template_bound_template_names.end() ||
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
    if(current->namespace_scope) {
      if(template_api::lookup_direct_named_type_in_inline_namespaces(
             *current, name)) {
        return true;
      }
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool scope_has_dependent_template_context_for_carried_expression(
    template_api::TemplateServices & services,
    Scope & scope)
{
  const auto type_is_dependent =
      [&services](const TypePtr & type) -> bool
      {
        return service_type_depends_on_template_parameter(services, type);
      };
  const auto value_is_dependent =
      [](const ValueBinding & value) -> bool
      {
        return value.dependent_template_value;
      };
  const auto lookup_type_binding =
      [](Scope & start, const std::string & name) -> const TypePtr *
      {
        for(Scope * current = &start; current; current = current->parent) {
          std::map<std::string, TypePtr>::const_iterator found =
              current->named_types.find(name);
          if(found != current->named_types.end()) {
            return &found->second;
          }
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
        }
        return nullptr;
      };
  const auto lookup_type_pack_binding =
      [](Scope & start, const std::string & name) ->
          const std::vector<TypePtr> *
      {
        for(Scope * current = &start; current; current = current->parent) {
          std::map<std::string, std::vector<TypePtr> >::const_iterator found =
              current->named_type_packs.find(name);
          if(found != current->named_type_packs.end()) {
            return &found->second;
          }
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
        }
        return nullptr;
      };
  const auto lookup_value_binding =
      [](Scope & start, const std::string & name) -> const ValueBinding *
      {
        for(Scope * current = &start; current; current = current->parent) {
          std::map<std::string, ValueBinding>::const_iterator found =
              current->values.find(name);
          if(found != current->values.end()) {
            return &found->second;
          }
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
        }
        return nullptr;
      };
  const auto lookup_class_template_binding =
      [](Scope & start, const std::string & name) -> ClassTemplateDecl *
      {
        for(Scope * current = &start; current; current = current->parent) {
          std::map<std::string, ClassTemplateDecl *>::const_iterator found =
              current->class_templates.find(name);
          if(found != current->class_templates.end()) {
            return found->second;
          }
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
        }
        return nullptr;
      };
  const auto lookup_alias_template_binding =
      [](Scope & start, const std::string & name) -> AliasTemplateDecl *
      {
        for(Scope * current = &start; current; current = current->parent) {
          std::map<std::string, AliasTemplateDecl *>::const_iterator found =
              current->alias_templates.find(name);
          if(found != current->alias_templates.end()) {
            return found->second;
          }
          if(current->namespace_scope || current->parent == nullptr) {
            break;
          }
        }
        return nullptr;
      };
  for(Scope * current = &scope; current; current = current->parent) {
    for(std::set<std::string>::const_iterator it =
            current->template_bound_type_names.begin();
        it != current->template_bound_type_names.end();
        ++it) {
      const TypePtr * found = lookup_type_binding(*current, *it);
      if(!found || type_is_dependent(*found)) {
        return true;
      }
    }
    for(std::set<std::string>::const_iterator it =
            current->template_bound_type_pack_names.begin();
        it != current->template_bound_type_pack_names.end();
        ++it) {
      const std::vector<TypePtr> * found = lookup_type_pack_binding(*current, *it);
      if(!found) {
        return true;
      }
      for(std::size_t i = 0; i < found->size(); ++i) {
        if(type_is_dependent((*found)[i])) {
          return true;
        }
      }
    }
    for(std::set<std::string>::const_iterator it =
            current->template_bound_value_names.begin();
        it != current->template_bound_value_names.end();
        ++it) {
      const ValueBinding * found = lookup_value_binding(*current, *it);
      if(!found || value_is_dependent(*found)) {
        return true;
      }
    }
    for(std::set<std::string>::const_iterator it =
            current->template_bound_template_names.begin();
        it != current->template_bound_template_names.end();
        ++it) {
      ClassTemplateDecl * found_class = lookup_class_template_binding(*current, *it);
      AliasTemplateDecl * found_alias = lookup_alias_template_binding(*current, *it);
      if(!found_class && !found_alias) {
        return true;
      }
    }
    for(std::map<std::string, std::vector<TypePtr> >::const_iterator pack =
            current->named_type_packs.begin();
        pack != current->named_type_packs.end();
        ++pack) {
      for(std::size_t i = 0; i < pack->second.size(); ++i) {
        if(type_is_dependent(pack->second[i])) {
          return true;
        }
      }
    }
    for(std::map<std::string, std::vector<ValueBinding> >::const_iterator pack =
            current->named_value_packs.begin();
        pack != current->named_value_packs.end();
        ++pack) {
      for(std::size_t i = 0; i < pack->second.size(); ++i) {
        if(value_is_dependent(pack->second[i])) {
          return true;
        }
      }
    }
    for(std::map<std::string, TypePtr>::const_iterator it =
            current->named_types.begin();
        it != current->named_types.end();
        ++it) {
      if(it->second && type_is_dependent(it->second)) {
        return true;
      }
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool template_argument_syntax_can_retain_carried_non_type_dependency(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateArgumentSyntax * syntax)
{
  return syntax &&
         syntax->expression &&
         (syntax->dependent ||
          (syntax->type_id &&
           scope_has_dependent_template_context_for_carried_expression(
               services, scope)));
}

bool try_evaluate_sizeof_pack_non_type_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateArgumentSyntax * syntax,
    const TypePtr & bound_value_type,
    long long & value,
    std::string & eval_error,
    template_api::NonTypeArgumentStatus & status)
{
  status = template_api::NT_ARG_PARSE_FAILED;
  if(!syntax ||
     !syntax->expression ||
     syntax->expression->kind != CppAstKind::sizeof_pack_expression) {
    return false;
  }

  status = static_cast<template_api::NonTypeArgumentStatus>(
      template_argument_semantics::evaluate_non_type_argument_syntax(
          services,
          scope,
          *syntax,
          value,
          &eval_error,
          bound_value_type));
  return status == template_api::NT_ARG_EVALUATED;
}

std::string leading_name_for_dependency_probe(const QualifiedName & qualified)
{
  if(!qualified.qualifiers.empty()) {
    return qualified.qualifiers.front();
  }
  return qualified.name;
}

bool carried_dependent_expression_has_unresolved_lookup(
    template_api::TemplateServices & services,
    Scope & scope,
    const CppAstNode & node)
{
  if(node.kind == CppAstKind::id_expression) {
    const QualifiedName * qualified = cppast_qualified_name_syntax(node);
    const bool has_qualified_lookup =
        qualified && (qualified->rooted || !qualified->qualifiers.empty());
    const std::string leading =
        qualified ? leading_name_for_dependency_probe(*qualified) : node.value;
    if(non_type_dependency_literal_name(leading)) {
      return false;
    }
    if(has_qualified_lookup &&
       !scope_contains_any_unqualified_binding(services, scope, leading)) {
      return true;
    }
    if(!has_qualified_lookup &&
       is_identifier_text(leading) &&
       !scope_contains_any_unqualified_binding(services, scope, leading)) {
      return true;
    }
  }

  if(const TemplateIdSyntax * template_id = cppast_template_id_syntax(node)) {
    const std::string leading = leading_name_for_dependency_probe(template_id->name);
    if(!non_type_dependency_literal_name(leading) &&
       !scope_contains_any_unqualified_binding(services, scope, leading)) {
      return true;
    }
  }

  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    const std::string leading =
        leading_name_for_dependency_probe(node.qualifier_template_id_syntaxes[i].name);
    if(!non_type_dependency_literal_name(leading) &&
       !scope_contains_any_unqualified_binding(services, scope, leading)) {
      return true;
    }
  }

  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(carried_dependent_expression_has_unresolved_lookup(
           services, scope, node.children[i])) {
      return true;
    }
  }
  return false;
}

bool template_id_head_name_from_type_text(const std::string & text,
                                          QualifiedName & out)
{
  out = QualifiedName();
  const std::string lookup_text = strip_elaborated_type_prefix(trim_space(text));
  if(lookup_text.empty()) {
    return false;
  }

  const std::string template_head =
      semantic_utils::strip_trailing_top_level_template_arguments(lookup_text);
  if(template_head == lookup_text || template_head.empty()) {
    return false;
  }
  if(semantic_utils::split_qualified_name_text(template_head, out)) {
    return !out.name.empty();
  }
  if(!is_identifier_text(template_head)) {
    return false;
  }
  out.name = template_head;
  return true;
}

std::string named_type_head_text(const std::string & text)
{
  const std::string trimmed = trim_space(text);
  std::size_t angle = trimmed.find('<');
  if(angle == std::string::npos) {
    return trimmed;
  }
  return trim_space(trimmed.substr(0, angle));
}

// template-boundary-audit: begin text_recovery_bridge
std::string deduction_lookup_type_text(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(
      type_system, type);
}

std::string deduction_lookup_type_text(SemanticContext & ctx,
                                       const TypePtr & type)
{
  return template_argument_semantics::lookup_text_for_type_argument(ctx, type);
}

TypePtr lookup_type_for_deduction(template_api::TemplateServices & services,
                                  Scope & scope,
                                  const std::string & text,
                                  bool allow_class_templates)
{
  TypePtr out;
  const std::string lookup_text = strip_elaborated_type_prefix(trim_space(text));
  if(lookup_text.empty() ||
     lookup_text.find('<') != std::string::npos) {
    return out;
  }
  template_api::TemplateTypeLookupRequest request;
  request.scope = &scope;
  request.allow_class_templates = allow_class_templates;
  if(!semantic_utils::split_qualified_name_text(lookup_text, request.name)) {
    if(!is_identifier_text(lookup_text)) {
      return out;
    }
    request.name.name = lookup_text;
  }
  service_type_system(services).resolve_direct_type_lookup(request, out);
  return out;
}

bool recover_non_type_parameter_value_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TypePtr & value_type,
    TypePtr & out)
{
  return template_argument_semantics::resolve_instantiated_dependent_type(
      services, scope, value_type, out);
}

void set_resolved_type_template_argument(
    template_api::TemplateTypeSystem & type_system,
    const TypePtr & type,
    const std::string & dependent_text,
    TemplateArgument & out)
{
  out.kind = TemplateArgument::TA_TYPE;
  out.type = type;
  out.text =
      template_argument_semantics::type_depends_on_template_parameter(
          type_system,
          type) ?
          dependent_text :
          deduction_lookup_type_text(type_system, type);
}

bool strip_leading_typename_for_dependent_member(const std::string & text,
                                                 std::string & out)
{
  const std::string trimmed = trim_space(text);
  static const char keyword[] = "typename";
  static const std::size_t keyword_size = sizeof(keyword) - 1;
  if(trimmed.size() <= keyword_size ||
     trimmed.compare(0, keyword_size, keyword) != 0 ||
     !std::isspace(static_cast<unsigned char>(trimmed[keyword_size]))) {
    return false;
  }
  out = trim_space(trimmed.substr(keyword_size));
  return !out.empty();
}

bool leading_typename_template_member_text(const std::string & text)
{
  std::string qualified_text;
  if(!strip_leading_typename_for_dependent_member(text, qualified_text)) {
    return false;
  }

  const std::size_t split = semantic_utils::top_level_scope_split(qualified_text);
  if(split == std::string::npos || split + 2 >= qualified_text.size()) {
    return false;
  }

  const std::string owner_text = trim_space(qualified_text.substr(0, split));
  const std::string member_text = trim_space(qualified_text.substr(split + 2));
  if(owner_text.empty() ||
     !is_identifier_text(member_text)) {
    return false;
  }

  const std::string owner_template_head =
      semantic_utils::strip_trailing_top_level_template_arguments(owner_text);
  return owner_template_head != owner_text &&
         !owner_template_head.empty();
}

bool leading_typename_qualified_member_text(const std::string & text)
{
  std::string qualified_text;
  if(!strip_leading_typename_for_dependent_member(text, qualified_text)) {
    return false;
  }

  QualifiedName qualified;
  if(!semantic_utils::split_qualified_name_text(qualified_text, qualified) ||
     qualified.rooted ||
     qualified.qualifiers.empty() ||
     !is_identifier_text(qualified.name)) {
    return false;
  }
  for(std::size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    if(!is_identifier_text(qualified.qualifiers[i])) {
      return false;
    }
  }
  return true;
}

bool type_argument_can_remain_dependent_after_structured_failure(
    const TemplateArgumentSyntax * syntax,
    const std::string & text)
{
  if(syntax) {
    if(syntax->dependent || syntax->pack_expansion) {
      return true;
    }

    const CppAstNode * type_name = nullptr;
    if(simple_type_argument_type_name_syntax(syntax, type_name) &&
       type_name &&
       type_name->has_leading_typename &&
       type_name_has_actual_template_id_syntax(*type_name)) {
      return true;
    }

    std::string owner_name;
    std::string member_name;
    if(simple_bound_member_type_argument_syntax(syntax, owner_name, member_name)) {
      return true;
    }
  }

  return leading_typename_template_member_text(text) ||
         leading_typename_qualified_member_text(text);
}

TypePtr make_deferred_dependent_type_argument(const std::string & text)
{
  const std::string trimmed = trim_space(text);
  return make_semantic_named(trimmed,
                             Type::NSK_DEPENDENT_TYPE,
                             trimmed,
                             true);
}

void attach_template_argument_source_syntax(
    const TemplateArgumentSyntax * syntax,
    TemplateArgument & out)
{
  if(syntax) {
    out.source_syntax.reset(new TemplateArgumentSyntax(*syntax));
    if(out.kind == TemplateArgument::TA_TYPE && out.type) {
      out.source_syntax->resolved_type = out.type;
    }
  }
}

bool lookup_direct_bound_type_argument(Scope & scope,
                                       const std::string & text,
                                       TypePtr & out)
{
  out.reset();
  std::string name = strip_elaborated_type_prefix(trim_space(text));
  std::string stripped_typename;
  if(strip_leading_typename_argument_text(name, stripped_typename)) {
    name = stripped_typename;
  }
  if(name.empty() ||
     !is_identifier_text(name)) {
    return false;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    std::map<std::string, TypePtr>::const_iterator found =
        current->named_types.find(name);
    if(found != current->named_types.end() && found->second) {
      out = found->second;
      return true;
    }
  }
  return false;
}

bool text_is_top_level_function_type_argument(const std::string & text)
{
  const std::string trimmed = trim_space(text);
  if(trimmed.empty() || trimmed[trimmed.size() - 1] != ')') {
    return false;
  }

  int angle_depth = 0;
  int paren_depth = 0;
  std::size_t open = std::string::npos;
  for(std::size_t i = 0; i < trimmed.size(); ++i) {
    const char ch = trimmed[i];
    if(ch == '<') {
      ++angle_depth;
    } else if(ch == '>' && angle_depth > 0) {
      --angle_depth;
    } else if(ch == '(' && angle_depth == 0) {
      if(paren_depth == 0 && open == std::string::npos) {
        open = i;
      }
      ++paren_depth;
    } else if(ch == ')' && angle_depth == 0) {
      --paren_depth;
      if(paren_depth < 0) {
        return false;
      }
      if(paren_depth == 0 && i + 1 != trimmed.size()) {
        return false;
      }
    }
  }

  return open != std::string::npos &&
         open != 0 &&
         paren_depth == 0 &&
         angle_depth == 0;
}

bool scope_is_inside_source_template_class_instantiation(Scope & scope)
{
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    if(current->class_info && current->class_info->source_template) {
      return true;
    }
  }
  return false;
}

bool lookup_rewritten_bound_type_argument(Scope & scope,
                                          const std::string & text,
                                          TypePtr & out)
{
  out.reset();
  std::string name = strip_elaborated_type_prefix(trim_space(text));
  std::string stripped_typename;
  if(strip_leading_typename_argument_text(name, stripped_typename)) {
    name = stripped_typename;
  }
  if(name.empty()) {
    return false;
  }

  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
    for(std::set<std::string>::const_iterator it =
            current->template_bound_type_names.begin();
        it != current->template_bound_type_names.end();
        ++it) {
      std::map<std::string, TypePtr>::const_iterator found =
          current->named_types.find(*it);
      if(found == current->named_types.end() || !found->second) {
        continue;
      }
      if(trim_space(reparseable_type_argument_text(found->second)) == name) {
        out = found->second;
        return true;
      }
    }
  }
  return false;
}

bool lookup_exact_visible_type_argument_text(Scope & scope,
                                             const std::string & text,
                                             TypePtr & out)
{
  out.reset();
  std::string name = strip_elaborated_type_prefix(trim_space(text));
  std::string stripped_typename;
  if(strip_leading_typename_argument_text(name, stripped_typename)) {
    name = stripped_typename;
  }
  if(name.empty()) {
    return false;
  }
  QualifiedName qualified;
  if(semantic_utils::split_qualified_name_text(name, qualified) &&
     (qualified.rooted || !qualified.qualifiers.empty())) {
    QualifiedName owner;
    owner.rooted = qualified.rooted;
    if(!qualified.qualifiers.empty()) {
      owner.qualifiers.assign(qualified.qualifiers.begin(),
                              qualified.qualifiers.end() - 1);
      owner.name = qualified.qualifiers.back();
    }
    Scope * owner_scope = semantic_lookup::lookup_namespace_name(scope, owner);
    if(owner_scope) {
      out = template_api::lookup_direct_named_type_in_inline_namespaces(
          *owner_scope,
          qualified.name);
      return out != nullptr;
    }
    return false;
  }
  for(Scope * current = &scope; current; current = current->parent) {
    TypePtr found_type;
    if(current->namespace_scope) {
      found_type =
          template_api::lookup_direct_named_type_in_inline_namespaces(*current, name);
    } else {
      std::map<std::string, TypePtr>::const_iterator found =
          current->named_types.find(name);
      if(found != current->named_types.end()) {
        found_type = found->second;
      }
    }
    if(found_type) {
      out = found_type;
      return true;
    }
  }
  return false;
}

bool scope_has_concrete_template_class_context(Scope & scope)
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
      return true;
    }
    if(current->namespace_scope || current->parent == nullptr) {
      break;
    }
  }
  return false;
}

bool resolve_non_dependent_direct_type_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & text,
    TypePtr & out)
{
  out.reset();
  if(!scope.valid()) {
    return false;
  }
  Scope & raw_scope = scope.require();
  if(!scope_has_concrete_template_class_context(raw_scope)) {
    return false;
  }

  std::string name = strip_elaborated_type_prefix(trim_space(text));
  std::string stripped_typename;
  if(strip_leading_typename_argument_text(name, stripped_typename)) {
    name = stripped_typename;
  }
  if(name.empty() ||
     !is_identifier_text(name)) {
    return false;
  }

  // In instantiated member-template bodies, source-scope aliases can still be
  // visible as dependent placeholders.  Prefer the concrete current
  // specialization for simple type arguments, but keep this out of generic
  // direct lookup because dependent owner/member checks rely on the old order.
  template_api::TemplateTypeLookupRequest request;
  request.scope = &raw_scope;
  request.name.name = name;
  request.allow_class_templates = true;
  if(!services.type_system.resolve_direct_type_lookup(request, out) || !out) {
    out.reset();
    return false;
  }
  template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
      services, scope, out);
  if(!out ||
     template_argument_semantics::type_depends_on_template_parameter(
         services.type_system, out)) {
    out.reset();
    return false;
  }
  return true;
}

bool try_resolve_single_bound_type_argument_fast(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const template_argument_semantics::ExpandedTemplateArgumentInputs & inputs,
    std::vector<TemplateArgument> & out)
{
  if(parser_trace::enabled("template.resolve") ||
     witness::source_capture_enabled(services.witness_context) ||
     parameters.size() != 1 ||
     inputs.texts.size() != 1 ||
     parameters[0].kind != TemplateParameterInfo::TP_TYPE ||
     parameters[0].parameter_pack) {
    return false;
  }

  TypePtr type;
  if(is_identifier_text(strip_elaborated_type_prefix(trim_space(inputs.texts[0])))) {
    return false;
  }
  if(!lookup_direct_bound_type_argument(scope.require(), inputs.texts[0], type) ||
     !type) {
    return false;
  }

  template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
      services, scope, type);
  TemplateArgument arg;
  set_resolved_type_template_argument(type_system, type, inputs.texts[0], arg);
  attach_template_argument_source_syntax(inputs.syntax_for(0), arg);
  out.clear();
  out.push_back(arg);
  return true;
}

enum PreExpansionResolveStatus
{
  PERTA_UNSUPPORTED,
  PERTA_SUCCESS,
  PERTA_FAILURE
};

PreExpansionResolveStatus try_resolve_pre_expansion_simple_type_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<TemplateArgumentSyntax> * syntaxes,
    std::vector<TemplateArgument> & out)
{
  if(parser_trace::enabled("template.resolve") ||
     witness::source_capture_enabled(services.witness_context) ||
     !scope.valid() ||
     texts.empty() ||
     texts.size() != parameters.size() ||
     (syntaxes && syntaxes->size() != texts.size())) {
    return PERTA_UNSUPPORTED;
  }

  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack ||
       parameters[i].kind != TemplateParameterInfo::TP_TYPE) {
      return PERTA_UNSUPPORTED;
    }
  }

  Scope & raw_scope = scope.require();
  std::vector<TemplateArgument> resolved;
  resolved.reserve(parameters.size());
  std::vector<std::pair<std::string, TypePtr> > resolved_by_parameter_name;
  resolved_by_parameter_name.reserve(parameters.size());
  const auto resolved_parameter_type =
      [&](const std::string & name) -> TypePtr
      {
        if(name.empty()) {
          return TypePtr();
        }
        for(std::size_t i = resolved_by_parameter_name.size(); i > 0; --i) {
          if(resolved_by_parameter_name[i - 1].first == name) {
            return resolved_by_parameter_name[i - 1].second;
          }
        }
        return TypePtr();
      };
  for(std::size_t index = 0; index < parameters.size(); ++index) {
    const TemplateArgumentSyntax * syntax =
        syntaxes ? &(*syntaxes)[index] : nullptr;
    const std::string simple_name =
        strip_elaborated_type_prefix(trim_space(texts[index]));
    TypePtr type;
    if(is_identifier_text(simple_name)) {
      lookup_direct_bound_type_argument(raw_scope, simple_name, type);
    }
    if(!type && syntax && syntax->resolved_type) {
      type = syntax->resolved_type;
    }

    std::string owner_name;
    std::string member_name;
    if(!type &&
       simple_bound_member_type_argument_syntax(syntax, owner_name, member_name)) {
      TypePtr owner_type = resolved_parameter_type(owner_name);
      if(!owner_type) {
        return PERTA_UNSUPPORTED;
      }

      bool determinate_failure = false;
      if(!try_resolve_member_type_on_known_owner(type_system,
                                                 raw_scope,
                                                 owner_type,
                                                 member_name,
                                                 type,
                                                 determinate_failure,
                                                 services.counters)) {
        return PERTA_UNSUPPORTED;
      }
      if(determinate_failure) {
        if(services.counters) {
          ++services.counters
                ->resolve_template_argument_pre_expansion_bound_member_failures;
          ++services.counters->resolve_template_argument_bound_member_failures;
        }
        return PERTA_FAILURE;
      }
    }

    if(!type) {
      type = resolved_parameter_type(simple_name);
    }
    if(!type) {
      resolve_non_dependent_direct_type_argument(
          services, scope, texts[index], type);
    }
    if(!type && is_identifier_text(simple_name)) {
      return PERTA_UNSUPPORTED;
    }
    if(!type &&
       !lookup_direct_bound_type_argument(raw_scope, texts[index], type)) {
      lookup_exact_visible_type_argument_text(raw_scope, texts[index], type);
    }
    if(!type) {
      return PERTA_UNSUPPORTED;
    }

    template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
        services, scope, type);
    TemplateArgument arg;
    set_resolved_type_template_argument(type_system, type, texts[index], arg);
    attach_template_argument_source_syntax(syntax, arg);
    resolved.push_back(arg);
    if(!parameters[index].name.empty()) {
      resolved_by_parameter_name.push_back(
          std::make_pair(parameters[index].name, arg.type));
    }
  }

  if(services.counters) {
    ++services.counters
          ->resolve_template_argument_pre_expansion_simple_type_successes;
  }
  out.swap(resolved);
  return PERTA_SUCCESS;
}

bool try_resolve_expanded_type_template_argument(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter,
    const std::string & text,
    const TemplateArgumentSyntax * syntax,
    const TypePtr & expanded_type,
    TemplateArgument & out)
{
  if(parameter.kind != TemplateParameterInfo::TP_TYPE ||
     !expanded_type) {
    return false;
  }

  TypePtr type = expanded_type;
  template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
      services, scope, type);
  set_resolved_type_template_argument(type_system, type, text, out);
  attach_template_argument_source_syntax(syntax, out);
  return true;
}

enum FastResolveTemplateArgumentsStatus
{
  FRTA_UNSUPPORTED,
  FRTA_SUCCESS,
  FRTA_FAILURE
};

FastResolveTemplateArgumentsStatus try_resolve_simple_template_arguments_fast(
    template_api::TemplateServices & services,
    template_api::TemplateTypeSystem & type_system,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const template_argument_semantics::ExpandedTemplateArgumentInputs & inputs,
    std::vector<TemplateArgument> & out,
    bool allow_expensive_resolution)
{
  if(parser_trace::enabled("template.resolve") ||
     witness::enabled(services.witness_context) ||
     parameters.empty() ||
     inputs.texts.size() > parameters.size() ||
     (!allow_expensive_resolution &&
      parameters.size() != inputs.texts.size())) {
    return FRTA_UNSUPPORTED;
  }

  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack ||
       parameters[i].kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      return FRTA_UNSUPPORTED;
    }
  }

  Scope & raw_scope = scope.require();
  if(scope_is_inside_source_template_class_instantiation(raw_scope)) {
    return FRTA_UNSUPPORTED;
  }
  Scope bound_scope(&raw_scope, "", false);
  std::vector<TemplateArgument> resolved;
  resolved.reserve(parameters.size());

  for(std::size_t i = 0; i < parameters.size(); ++i) {
    TemplateArgument arg;
    if(i >= inputs.texts.size()) {
      if(!allow_expensive_resolution) {
        return FRTA_UNSUPPORTED;
      }
      if(!parameters[i].default_argument ||
         parameters[i].default_argument->children.empty()) {
        return FRTA_UNSUPPORTED;
      }
      const CppAstNode & child = parameters[i].default_argument->children[0];
      const std::string original_default_text =
          parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE ?
              default_argument_expression_text(child) :
              default_type_argument_text_from_ast(parameters[i], child);
      if(original_default_text.empty()) {
        return FRTA_UNSUPPORTED;
      }
      for(std::size_t parameter_index = 0;
          parameter_index < parameters.size();
          ++parameter_index) {
        if(contains_identifier_token(original_default_text,
                                     parameters[parameter_index].name)) {
          return FRTA_UNSUPPORTED;
        }
      }
      TemplateArgumentSyntax original_default_syntax =
          make_default_template_argument_syntax(parameters[i],
                                                child,
                                                original_default_text);
      std::string prepared_default_text = original_default_text;
      TemplateArgumentSyntax prepared_default_syntax;
      const TemplateArgumentSyntax * prepared_default_syntax_ptr =
          &original_default_syntax;
      if(make_substituted_default_template_argument_syntax(
             services,
             bound_scope,
             parameters[i],
             child,
             original_default_text,
             std::vector<TemplateParameterInfo>(parameters.begin(),
                                                parameters.begin() + i),
             resolved,
             prepared_default_syntax)) {
        prepared_default_text = prepared_default_syntax.text;
        prepared_default_syntax_ptr = &prepared_default_syntax;
      }
      if(text_mentions_template_dependency(
             services,
             template_api::make_template_environment(bound_scope),
             prepared_default_text)) {
        return FRTA_UNSUPPORTED;
      }
      if(!template_resolution::resolve_template_argument(
             services,
             template_api::make_template_environment(bound_scope),
             template_api::make_template_environment(bound_scope),
             parameters[i],
             prepared_default_text,
             prepared_default_syntax_ptr,
             arg)) {
        return FRTA_UNSUPPORTED;
      }
      arg.source_defaulted = true;
    } else if(parameters[i].kind == TemplateParameterInfo::TP_TYPE) {
      TypePtr type;
      const TypePtr expanded_type = inputs.type_for(i);
      if(!expanded_type &&
         text_is_top_level_function_type_argument(inputs.texts[i])) {
        return FRTA_UNSUPPORTED;
      }
      if(!try_resolve_expanded_type_template_argument(
             services,
             type_system,
             template_api::make_template_environment(bound_scope),
             parameters[i],
             inputs.texts[i],
             inputs.syntax_for(i),
             expanded_type,
             arg)) {
        bool determinate_member_failure = false;
        if(try_resolve_bound_member_type_argument(type_system,
                                                  raw_scope,
                                                  bound_scope,
                                                  inputs.syntax_for(i),
                                                  type,
                                                  determinate_member_failure,
                                                  services.counters)) {
          if(determinate_member_failure) {
            if(services.counters) {
              ++services.counters->resolve_template_argument_bound_member_failures;
            }
            return FRTA_FAILURE;
          }
          if(!type) {
            return FRTA_UNSUPPORTED;
          }
          template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
              services, template_api::make_template_environment(bound_scope), type);
          set_resolved_type_template_argument(type_system, type, inputs.texts[i], arg);
          attach_template_argument_source_syntax(inputs.syntax_for(i), arg);
        } else if((lookup_direct_bound_type_argument(bound_scope,
                                                     inputs.texts[i],
                                                     type) ||
                   lookup_exact_visible_type_argument_text(bound_scope,
                                                          inputs.texts[i],
                                                          type)) &&
                  type) {
          template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
              services, template_api::make_template_environment(bound_scope), type);
          set_resolved_type_template_argument(type_system, type, inputs.texts[i], arg);
          attach_template_argument_source_syntax(inputs.syntax_for(i), arg);
        } else {
          return FRTA_UNSUPPORTED;
        }
      }
    } else if(parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE) {
      TypePtr bound_value_type;
      if(!try_resolve_non_type_template_parameter_type(
             services,
             template_api::make_template_environment(bound_scope),
             parameters[i],
             bound_value_type)) {
        return FRTA_UNSUPPORTED;
      }
      const ValueBinding * named_binding = nullptr;
      if(!try_resolve_named_non_type_template_argument(services,
                                                       raw_scope,
                                                       inputs.texts[i],
                                                       bound_value_type,
                                                       arg,
                                                       &named_binding)) {
        if(!allow_expensive_resolution) {
          return FRTA_UNSUPPORTED;
        }
        long long value = 0;
        std::string eval_error;
        const TemplateArgumentSyntax * syntax = inputs.syntax_for(i);
        const bool has_structured_non_type_syntax =
            syntax &&
            (syntax->expression || syntax->type_id || syntax->template_id);
        template_api::NonTypeArgumentStatus status =
            template_api::NT_ARG_PARSE_FAILED;
        template_api::TemplateEnvironmentHandle argument_scope =
            template_api::make_template_environment(raw_scope);
        const bool needs_structured_pack_evaluation =
            syntax &&
            (syntax->pack_expansion ||
             inputs.texts[i].find("...") != std::string::npos ||
             syntax->text.find("...") != std::string::npos ||
             syntax->source_text.find("...") != std::string::npos);
        const bool can_evaluate_structured_syntax =
            needs_structured_pack_evaluation &&
            (syntax->expression || syntax->type_id || syntax->template_id);
        if(text_mentions_template_dependency(services,
                                             argument_scope,
                                             inputs.texts[i])) {
          if(!try_evaluate_sizeof_pack_non_type_argument(
                 services,
                 argument_scope,
                 syntax,
                 bound_value_type,
                 value,
                 eval_error,
                 status)) {
            if(can_evaluate_structured_syntax) {
              status = static_cast<template_api::NonTypeArgumentStatus>(
                  template_argument_semantics::evaluate_non_type_argument_syntax(
                      services,
                      argument_scope,
                      *syntax,
                      value,
                      &eval_error,
                      bound_value_type));
            }
            if(status != template_api::NT_ARG_EVALUATED) {
              status = template_api::NT_ARG_DEPENDENT;
            }
          }
        } else if(syntax) {
          if(can_evaluate_structured_syntax) {
            status = static_cast<template_api::NonTypeArgumentStatus>(
                template_argument_semantics::evaluate_non_type_argument_syntax(
                    services,
                    argument_scope,
                    *syntax,
                    value,
                    &eval_error,
                    bound_value_type));
          } else if(syntax->expression) {
            status = template_api::evaluate_non_type_argument_expression(
                services,
                argument_scope,
                *syntax->expression,
                value,
                &eval_error,
                bound_value_type);
          }
        }
        if(status == template_api::NT_ARG_PARSE_FAILED &&
           has_structured_non_type_syntax) {
          return FRTA_UNSUPPORTED;
        }
        if(status == template_api::NT_ARG_PARSE_FAILED) {
          try {
            status = template_api::evaluate_non_type_argument_text(
                services,
                argument_scope,
                inputs.texts[i],
                value,
                &eval_error,
                bound_value_type);
          } catch(const std::logic_error &) {
            status = template_api::NT_ARG_PARSE_FAILED;
          }
        }
        if(status != template_api::NT_ARG_EVALUATED &&
           status != template_api::NT_ARG_DEPENDENT) {
          if(template_argument_syntax_can_retain_carried_non_type_dependency(
                 services, raw_scope, syntax) &&
             carried_dependent_expression_has_unresolved_lookup(
                 services, raw_scope, *syntax->expression)) {
            status = template_api::NT_ARG_DEPENDENT;
          }
        }
        if(status == template_api::NT_ARG_EVALUATED) {
          arg.kind = TemplateArgument::TA_VALUE;
          arg.type = bound_value_type;
          arg.value = value;
          arg.text = typed_non_type_template_argument_text(
              type_system, bound_value_type, value);
          arg.dependent = false;
        } else if(status == template_api::NT_ARG_DEPENDENT) {
          arg.kind = TemplateArgument::TA_VALUE;
          arg.type = bound_value_type;
          arg.text = inputs.texts[i];
          attach_dependent_non_type_argument_expression(
              inputs.syntax_for(i), inputs.texts[i], inputs.texts[i], arg);
          arg.dependent = true;
        } else {
          return FRTA_UNSUPPORTED;
        }
      }
      (void)named_binding;
      attach_template_argument_source_syntax(inputs.syntax_for(i), arg);
    } else {
      return FRTA_UNSUPPORTED;
    }
    resolved.push_back(arg);
    bind_single_template_argument_into_scope(services,
                                             bound_scope,
                                             parameters[i],
                                             resolved.back());
  }

  out.swap(resolved);
  return FRTA_SUCCESS;
}
// template-boundary-audit: end text_recovery_bridge

std::string type_argument_text_for_deduction(SemanticContext & ctx, const TypePtr & type)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return type_argument_text_for_deduction(type_system, type);
      });
}

std::string type_argument_text_for_deduction(template_api::TemplateTypeSystem & type_system,
                                             const TypePtr & type)
{
  const std::string text = deduction_lookup_type_text(type_system, type);
  return !text.empty() ? text : describe_type(type);
}

bool type_depends_on_template_parameter(SemanticContext & ctx, const TypePtr & type)
{
  return template_argument_semantics::type_depends_on_template_parameter(ctx, type);
}

bool describe_named_type(SemanticContext & ctx,
                         const TypePtr & type,
                         template_api::TemplateNamedTypeMetadata & out)
{
  return template_api::with_template_type_system(
      ctx,
      [&](template_api::TemplateTypeSystem & type_system)
      {
        return template_api::describe_named_type_metadata(type_system.model, type, out);
      });
}

TypePtr lookup_type_for_deduction(SemanticContext & ctx,
                                  Scope & scope,
                                  const std::string & text,
                                  bool allow_class_templates)
{
  return ctx.lookup_type(scope, text, allow_class_templates);
}

bool type_mentions_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type);

bool type_mentions_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type);

bool type_mentions_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type);

struct DeductionContextOps
{
  SemanticContext * semantic_context = nullptr;
  template_api::TemplateServices * services = nullptr;
  template_api::TemplateTypeSystem * type_system = nullptr;

  explicit DeductionContextOps(SemanticContext & ctx)
    : semantic_context(&ctx)
  {}

  explicit DeductionContextOps(template_api::TemplateServices & current_services)
    : services(&current_services),
      type_system(&service_type_system(current_services))
  {}

  std::string type_argument_text(const TypePtr & type) const
  {
    return type_system ?
        type_argument_text_for_deduction(*type_system, type) :
        type_argument_text_for_deduction(*semantic_context, type);
  }

  TypePtr lookup_type(Scope & scope,
                      const std::string & text,
                      bool allow_class_templates) const
  {
    if(!services) {
      return lookup_type_for_deduction(
          *semantic_context, scope, text, allow_class_templates);
    }

    return lookup_type_for_deduction(*services, scope, text, allow_class_templates);
  }

  std::string lookup_type_text(const TypePtr & type) const
  {
    return type_system ?
        deduction_lookup_type_text(*type_system, type) :
        deduction_lookup_type_text(*semantic_context, type);
  }

  bool type_depends(const TypePtr & type) const
  {
    return type_system ?
        template_argument_semantics::type_depends_on_template_parameter(*type_system, type) :
        template_resolution::type_depends_on_template_parameter(*semantic_context, type);
  }

  bool describe_named_type(
      const TypePtr & type,
      template_api::TemplateNamedTypeMetadata & out) const
  {
    return type_system ?
        template_api::describe_named_type_metadata(type_system->model, type, out) :
        template_resolution::describe_named_type(*semantic_context, type, out);
  }

  bool describe_named_type_for_base_deduction(
      const TypePtr & type,
      Scope * completion_scope,
      template_api::TemplateNamedTypeMetadata & out) const
  {
    if(!describe_named_type(type, out)) {
      return false;
    }
    if(out.complete || !out.direct_base_types.empty() || !completion_scope) {
      return true;
    }

    if(type_system) {
      Scope * completed_scope = nullptr;
      type_system->complete_named_type_member_scope(
          template_api::make_template_environment(*completion_scope),
          type,
          completed_scope);
    } else if(semantic_context) {
      semantic_context->complete_class_type(type);
    }

    template_api::TemplateNamedTypeMetadata completed;
    if(describe_named_type(type, completed)) {
      out = completed;
    }
    return true;
  }
};

const TemplateParameterInfo * direct_type_parameter_pack_pattern(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern)
{
  TypePtr base = strip_top_level_cv(pattern);
  if(!base || base->kind != Type::TK_NAMED) {
    return nullptr;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, base->named_key);
  if(!parameter) {
    parameter = find_template_parameter_by_name(parameters, base->named_key);
  }
  if(!parameter && base->named_display != base->named_key) {
    parameter = find_template_parameter(parameters, base->named_display);
  }
  if(!parameter && base->named_display != base->named_key) {
    parameter = find_template_parameter_by_name(parameters, base->named_display);
  }
  return parameter &&
         parameter->kind == TemplateParameterInfo::TP_TYPE &&
         parameter->parameter_pack ?
             parameter :
             nullptr;
}

bool record_deduced_type_pack_arguments_for_deduction(
    const DeductionContextOps & deduction_ops,
    const TemplateParameterInfo & parameter,
    const std::vector<TypePtr> & actual_arg_types,
    DeducedPackArgumentMap * deduced_pack_arguments)
{
  if(!deduced_pack_arguments || parameter.name.empty()) {
    return false;
  }

  std::vector<TemplateArgument> pack_arguments;
  pack_arguments.reserve(actual_arg_types.size());
  for(std::size_t i = 0; i < actual_arg_types.size(); ++i) {
    TemplateArgument arg;
    arg.kind = TemplateArgument::TA_TYPE;
    arg.type = actual_arg_types[i];
    arg.text = deduction_ops.type_depends(actual_arg_types[i]) ?
                   parameter.name :
                   deduction_ops.lookup_type_text(actual_arg_types[i]);
    pack_arguments.push_back(arg);
  }

  DeducedPackArgumentMap::iterator found =
      deduced_pack_arguments->find(parameter.name);
  if(found == deduced_pack_arguments->end()) {
    (*deduced_pack_arguments)[parameter.name] = pack_arguments;
    return true;
  }
  if(found->second.size() != pack_arguments.size()) {
    return false;
  }
  for(std::size_t i = 0; i < pack_arguments.size(); ++i) {
    if(found->second[i].kind != TemplateArgument::TA_TYPE ||
       !type_equals(found->second[i].type, pack_arguments[i].type)) {
      return false;
    }
  }
  return true;
}

bool deduction_pattern_mentions_function_template_parameter(
    const DeductionContextOps & ops,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type)
{
  if(ops.semantic_context) {
    return type_mentions_unbound_function_template_parameter(
               *ops.semantic_context, parameters, scope, type) ||
           type_mentions_function_template_parameter(
               *ops.semantic_context, parameters, type);
  }
  if(ops.services && ops.services->semantic_context) {
    return type_mentions_unbound_function_template_parameter(
               *ops.services->semantic_context, parameters, scope, type) ||
           type_mentions_function_template_parameter(
               *ops.services->semantic_context, parameters, type);
  }
  if(ops.type_system) {
    return type_mentions_function_template_parameter(
        *ops.type_system, parameters, type);
  }
  return false;
}

const ValueBinding * lookup_unqualified_value(template_api::TemplateServices & services,
                                              Scope & scope,
                                              const std::string & name)
{
  std::vector<Scope *> scope_path;
  for(Scope * current = &scope; current; current = current->parent) {
    scope_path.push_back(current);
  }

  for(std::size_t scope_index = 0; scope_index < scope_path.size(); ++scope_index) {
    Scope * current = scope_path[scope_index];
    const ValueBinding * direct = semantic_lookup::lookup_direct_value(*current, name);
    if(!current->namespace_scope && direct) {
      return direct;
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
      semantic_lookup::MemberValueLookupResult member =
          semantic_lookup::lookup_member_value(*lexical_class, name);
      if(lexical_only && member.binding && member.binding->kind == ValueBinding::VK_FIELD) {
        member.binding = nullptr;
      }
      if(member.binding) {
        return member.binding;
      }
    }

    if(current->namespace_scope) {
      std::set<const Scope *> visited;
      semantic_lookup::ValueLookupFromUsingDirectivesResult imported =
          semantic_lookup::lookup_value_from_using_directives(*current, name, visited);
      if(imported.ambiguous) {
        return nullptr;
      }
      if(direct && imported.binding &&
         !semantic_lookup::same_value_binding_entity(direct, imported.binding)) {
        return nullptr;
      }
      if(direct) {
        return direct;
      }
      if(imported.binding) {
        return imported.binding;
      }
    }
  }
  return nullptr;
}

std::vector<FunctionBinding *> lookup_unqualified_functions(Scope & scope,
                                                            const std::string & name)
{
  std::vector<Scope *> scope_path;
  for(Scope * current = &scope; current; current = current->parent) {
    scope_path.push_back(current);
  }

  for(std::size_t scope_index = 0; scope_index < scope_path.size(); ++scope_index) {
    Scope * current = scope_path[scope_index];
    std::vector<FunctionBinding *> found =
        semantic_lookup::lookup_direct_functions(*current, name);
    if(!current->namespace_scope && !found.empty()) {
      return found;
    }

    const bool has_lexical_class =
        !current->class_info && current->function && current->function->lexical_access_class;
    ClassInfo * lexical_class = current->class_info;
    if(!lexical_class && has_lexical_class) {
      lexical_class = current->function->lexical_access_class;
    }
    if(lexical_class) {
      semantic_lookup::append_unique_functions(
          found,
          semantic_lookup::lookup_visible_member_functions(*lexical_class, name).functions);
      if(!found.empty()) {
        return found;
      }
    }

    if(current->namespace_scope) {
      std::set<const Scope *> visited;
      semantic_lookup::lookup_functions_from_using_directives(*current, name, visited, found);
      if(!found.empty()) {
        return found;
      }
    }
  }

  return std::vector<FunctionBinding *>();
}

bool dependent_alias_argument_needs_deferred_surface(
    SemanticContext & ctx,
    const DependentAliasTemplateArgumentSyntax & argument)
{
  if(argument.syntax.expression) {
    return true;
  }
  if(!argument.type) {
    return argument.syntax.dependent;
  }
  void * dependent_template = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> dependent_args;
  TypePtr owner;
  std::vector<std::string> members;
  bool leading_typename = false;
  if(named_type_dependent_alias_template(argument.type,
                                         dependent_template,
                                         dependent_args) ||
     named_type_dependent_qualified_member(argument.type,
                                           owner,
                                           members,
                                           leading_typename)) {
    return true;
  }
  return template_argument_semantics::type_depends_on_template_parameter(
             ctx,
             argument.type) &&
         !named_type_is_template_parameter(argument.type);
}

bool dependent_alias_argument_needs_deferred_surface(
    template_api::TemplateTypeSystem & type_system,
    const DependentAliasTemplateArgumentSyntax & argument)
{
  if(argument.syntax.expression) {
    return true;
  }
  if(!argument.type) {
    return argument.syntax.dependent;
  }
  void * dependent_template = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> dependent_args;
  TypePtr owner;
  std::vector<std::string> members;
  bool leading_typename = false;
  if(named_type_dependent_alias_template(argument.type,
                                         dependent_template,
                                         dependent_args) ||
     named_type_dependent_qualified_member(argument.type,
                                           owner,
                                           members,
                                           leading_typename)) {
    return true;
  }
  return template_argument_semantics::type_depends_on_template_parameter(
             type_system,
             argument.type) &&
         !named_type_is_template_parameter(argument.type);
}

TypePtr canonicalize_dependent_alias_type_for_deduction(SemanticContext & ctx,
                                                        Scope & scope,
                                                        const TypePtr & type,
                                                        Scope * preferred_parse_scope = nullptr)
{
  if(!type) {
    return type;
  }
  void * dependent_alias_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> dependent_alias_args;
  if(named_type_dependent_alias_template(type,
                                         dependent_alias_template_decl,
                                         dependent_alias_args)) {
    for(std::size_t i = 0; i < dependent_alias_args.size(); ++i) {
      if(dependent_alias_argument_needs_deferred_surface(
             ctx,
             dependent_alias_args[i])) {
        return type;
      }
    }
  }

  TypePtr resolved;
  if(preferred_parse_scope &&
     template_argument_semantics::resolve_instantiated_dependent_type(
         ctx, *preferred_parse_scope, type, resolved) &&
     resolved) {
    return resolved;
  }
  if(!template_argument_semantics::resolve_instantiated_dependent_type(
         ctx, scope, type, resolved) ||
     !resolved) {
    return type;
  }

  return resolved;
}

TypePtr canonicalize_dependent_alias_type_for_deduction(
    template_api::TemplateServices & services,
    Scope & scope,
    const TypePtr & type,
    Scope * preferred_parse_scope = nullptr)
{
  if(!type) {
    return type;
  }
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  void * dependent_alias_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> dependent_alias_args;
  if(named_type_dependent_alias_template(type,
                                         dependent_alias_template_decl,
                                         dependent_alias_args)) {
    for(std::size_t i = 0; i < dependent_alias_args.size(); ++i) {
      if(dependent_alias_argument_needs_deferred_surface(
             type_system,
             dependent_alias_args[i])) {
        return type;
      }
    }
  }

  TypePtr resolved;
  if(preferred_parse_scope &&
     template_argument_semantics::resolve_instantiated_dependent_type(
         services, template_api::make_template_environment(*preferred_parse_scope), type, resolved) &&
     resolved) {
    return resolved;
  }
  if(!template_argument_semantics::resolve_instantiated_dependent_type(
         services, template_api::make_template_environment(scope), type, resolved) ||
     !resolved) {
    return type;
  }

  return resolved;
}

std::vector<TemplateArgument> deduction_scope_local_type_arguments(
    SemanticContext & ctx,
    const std::vector<ExprInfo> & args)
{
  std::vector<TemplateArgument> out;
  out.reserve(args.size());
  for(std::size_t i = 0; i < args.size(); ++i) {
    if(!args[i].type) {
      continue;
    }
    TemplateArgument arg;
    arg.kind = TemplateArgument::TA_TYPE;
    arg.type = args[i].type;
    arg.text = reparseable_type_argument_text(args[i].type);
    if(arg.text.empty()) {
      arg.text = type_argument_text_for_deduction(ctx, args[i].type);
    }
    out.push_back(arg);
  }
  return out;
}

std::string qualified_name_text(const QualifiedName & qualified)
{
  std::ostringstream out;
  if(qualified.rooted) {
    out << "::";
  }
  for(std::size_t i = 0; i < qualified.qualifiers.size(); ++i) {
    out << qualified.qualifiers[i] << "::";
  }
  out << qualified.name;
  return out.str();
}

bool lookup_exact_bound_type_name_for_deduction(Scope & scope,
                                                const std::string & name,
                                                TypePtr & out)
{
  const std::string trimmed = trim_space(name);
  out.reset();
  for(Scope * current = &scope; current; current = current->parent) {
    if(current->namespace_scope || !current->parent) {
      break;
    }
    std::map<std::string, TypePtr>::const_iterator found =
        current->named_types.find(trimmed);
    if(found != current->named_types.end()) {
      if(current->template_bound_type_names.count(trimmed) != 0 &&
         found->second) {
        out = found->second;
        return true;
      }
      break;
    }
  }
  return false;
}

bool template_names_match(const QualifiedName & lhs, const QualifiedName & rhs)
{
  if(lhs.name != rhs.name) {
    return false;
  }
  if(lhs.rooted) {
    return rhs.rooted && lhs.qualifiers == rhs.qualifiers;
  }
  if(lhs.qualifiers.empty()) {
    return true;
  }
  return !rhs.rooted && lhs.qualifiers == rhs.qualifiers;
}

bool same_template_parameter_list_shape(
    const std::vector<TemplateParameterInfo> & lhs_parameters,
    const std::vector<TemplateParameterInfo> & rhs_parameters)
{
  if(lhs_parameters.size() != rhs_parameters.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs_parameters.size(); ++i) {
    if(lhs_parameters[i].kind != rhs_parameters[i].kind ||
       lhs_parameters[i].parameter_pack != rhs_parameters[i].parameter_pack ||
       lhs_parameters[i].template_parameter_count !=
           rhs_parameters[i].template_parameter_count) {
      return false;
    }
    if(lhs_parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       !type_equals(lhs_parameters[i].value_type, rhs_parameters[i].value_type)) {
      return false;
    }
  }
  return true;
}

bool same_inline_namespace_class_template_entity_for_deduction(
    const ClassTemplateDecl * lhs,
    const ClassTemplateDecl * rhs)
{
  if(lhs == rhs) {
    return true;
  }
  if(!lhs || !rhs || lhs->name != rhs->name) {
    return false;
  }
  if(semantic_lookup::inline_namespace_collapsed_scope_name(lhs->declaring_scope) !=
     semantic_lookup::inline_namespace_collapsed_scope_name(rhs->declaring_scope)) {
    return false;
  }
  return same_template_parameter_list_shape(lhs->parameters, rhs->parameters);
}

std::string template_argument_text_for_matching(template_api::TemplateServices & services,
                                                const TemplateArgument & arg)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  if(arg.kind == TemplateArgument::TA_VALUE &&
     !arg.dependent &&
     arg.type) {
    const std::string canonical =
        trim_space(template_model::template_argument_text(
            arg,
            [&type_system](const TypePtr & type) -> std::string
            {
              return type_argument_text_for_deduction(type_system, type);
            }));
    if(!canonical.empty()) {
      return canonical;
    }
  }
  if(!arg.text.empty()) {
    return trim_space(arg.text);
  }
  return trim_space(template_model::template_argument_text(
      arg,
      [&type_system](const TypePtr & type) -> std::string
      {
        return type_argument_text_for_deduction(type_system, type);
      }));
}

struct DecomposedTemplateInstantiation
{
  ClassTemplateDecl * source_template = nullptr;
  QualifiedName name;
  std::vector<TemplateArgument> arguments;
  std::vector<std::string> argument_texts;
};

bool align_explicit_template_arguments(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> & explicit_argument_texts,
    std::vector<TemplateArgument> & out)
{
  out.clear();
  std::size_t arg_index = 0;
  std::size_t explicit_index = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      std::size_t trailing_non_pack = 0;
      for(std::size_t j = i + 1; j < parameters.size(); ++j) {
        if(!parameters[j].parameter_pack) {
          ++trailing_non_pack;
        }
      }
      if(arguments.size() < arg_index + trailing_non_pack) {
        return false;
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
      for(std::size_t j = 0; j < explicit_pack_count; ++j) {
        out.push_back(arguments[arg_index + j]);
      }
      arg_index = pack_end;
      explicit_index += explicit_pack_count;
      continue;
    }

    if(arg_index >= arguments.size()) {
      return false;
    }
    if(explicit_index < explicit_argument_texts.size()) {
      out.push_back(arguments[arg_index]);
      ++explicit_index;
    }
    ++arg_index;
  }

  return explicit_index == explicit_argument_texts.size() &&
         out.size() == explicit_argument_texts.size();
}

bool decompose_template_instantiation(template_api::TemplateServices & services,
                                      Scope & lookup_scope,
                                      const TypePtr & type,
                                      DecomposedTemplateInstantiation & out);

bool decompose_template_instantiation(SemanticContext & ctx,
                                      Scope & lookup_scope,
                                      const TypePtr & type,
                                      DecomposedTemplateInstantiation & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return decompose_template_instantiation(services, lookup_scope, type, out);
      });
}

bool decompose_template_instantiation(template_api::TemplateServices & services,
                                      Scope & lookup_scope,
                                      const TypePtr & type,
                                      DecomposedTemplateInstantiation & out)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }

  const std::string normalized_text = strip_elaborated_type_prefix(
      trim_space(type_argument_text_for_deduction(type_system, base)));
  QualifiedName parsed_name;
  std::vector<std::string> parsed_arg_texts;
  const bool parsed_template_id =
      !normalized_text.empty() &&
      semantic_utils::split_top_level_template_id_text(normalized_text,
                                                       parsed_name,
                                                       parsed_arg_texts);

  template_api::TemplateNamedTypeMetadata info;
  const bool have_info =
      template_api::describe_named_type_metadata(type_system.model, base, info);
  void * dependent_class_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> dependent_class_args;
  const bool have_dependent_class_template =
      named_type_dependent_class_template(base,
                                          dependent_class_template_decl,
                                          dependent_class_args);
  std::vector<std::string> dependent_class_arg_texts;
  dependent_class_arg_texts.reserve(dependent_class_args.size());
  for(std::size_t i = 0; i < dependent_class_args.size(); ++i) {
    dependent_class_arg_texts.push_back(trim_space(dependent_class_args[i].text));
  }
  const bool dependent_class_args_have_pack_expansion =
      template_metadata::argument_texts_contain_pack_expansion(
          dependent_class_arg_texts);
  const auto info_instantiation_fully_binds_source_template =
      [&]() -> bool
  {
    return have_info &&
           info.source_template &&
           template_arguments_fully_bind_parameters(info.source_template->parameters,
                                                    info.instantiation_arguments);
  };
  if(have_info &&
     info.source_template &&
     !dependent_class_args_have_pack_expansion &&
     (!parsed_template_id ||
      info_instantiation_fully_binds_source_template() ||
      template_metadata::has_argument_texts(info))) {
    const auto try_recover_arguments_from_member_scope =
        [&](std::vector<TemplateArgument> & recovered) -> bool
    {
      if(!info.member_scope) {
        return false;
      }
      recovered.clear();
      for(std::size_t i = 0; i < info.source_template->parameters.size(); ++i) {
        const TemplateParameterInfo & parameter = info.source_template->parameters[i];
        if(parameter.name.empty()) {
          return false;
        }

        if(parameter.parameter_pack) {
          if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
            std::map<std::string, std::vector<TypePtr> >::const_iterator found_pack =
                info.member_scope->named_type_packs.find(parameter.name);
            if(found_pack == info.member_scope->named_type_packs.end()) {
              return false;
            }
            for(std::size_t j = 0; j < found_pack->second.size(); ++j) {
              TemplateArgument arg;
              arg.kind = TemplateArgument::TA_TYPE;
              arg.type = found_pack->second[j];
              arg.text = deduction_lookup_type_text(type_system, arg.type);
              recovered.push_back(arg);
            }
            continue;
          }
          return false;
        }

        TemplateArgument arg;
        if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
          std::map<std::string, TypePtr>::const_iterator found_type =
              info.member_scope->named_types.find(parameter.name);
          if(found_type == info.member_scope->named_types.end()) {
            return false;
          }
          arg.kind = TemplateArgument::TA_TYPE;
          arg.type = found_type->second;
          arg.text = deduction_lookup_type_text(type_system, arg.type);
        } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
          std::map<std::string, ValueBinding>::const_iterator found_value =
              info.member_scope->values.find(parameter.name);
          if(found_value == info.member_scope->values.end()) {
            return false;
          }
          arg.kind = TemplateArgument::TA_VALUE;
          arg.type = found_value->second.type;
          if(found_value->second.has_constant_value) {
            arg.value = found_value->second.constant_value;
            arg.text = typed_non_type_template_argument_text(
                type_system,
                found_value->second.type,
                found_value->second.constant_value);
          } else if(found_value->second.dependent_template_value) {
            arg.dependent = true;
            arg.text = parameter.name;
          } else {
            return false;
          }
        } else {
          std::map<std::string, ClassTemplateDecl *>::const_iterator found_class_template =
              info.member_scope->class_templates.find(parameter.name);
          if(found_class_template != info.member_scope->class_templates.end()) {
            arg.kind = TemplateArgument::TA_CLASS_TEMPLATE;
            arg.template_decl = found_class_template->second;
            arg.text = parameter.name;
          } else if(info.member_scope->alias_templates.find(parameter.name) !=
                    info.member_scope->alias_templates.end()) {
            arg.kind = TemplateArgument::TA_ALIAS_TEMPLATE;
            arg.template_decl =
                info.member_scope->alias_templates.find(parameter.name)->second;
            arg.text = parameter.name;
          } else {
            return false;
          }
        }
        recovered.push_back(arg);
      }
      return template_arguments_fully_bind_parameters(info.source_template->parameters,
                                                      recovered);
    };
    out.source_template = info.source_template;
    out.arguments = info.instantiation_arguments;
    // Do not recover typed arguments by reparsing stored source text here.
    // Deduction can still use the explicit argument text shape below, and
    // forcing text back through template-id lookup loses the structured origin.
    if(!template_arguments_fully_bind_parameters(info.source_template->parameters,
                                                 out.arguments)) {
      std::vector<TemplateArgument> recovered_arguments;
      if(try_recover_arguments_from_member_scope(recovered_arguments)) {
        out.arguments.swap(recovered_arguments);
      }
    }
    if(parsed_template_id) {
      out.name = parsed_name;
    } else {
      const std::string source_template_name =
          info.source_template->declaring_scope ?
              semantic_lookup::scope_qualified_name(*info.source_template->declaring_scope,
                                                    info.source_template->name) :
              info.source_template->name;
      out.name = info.source_template->declaring_scope ?
          semantic_lookup::scope_qualified_name_syntax(
              *info.source_template->declaring_scope,
              info.source_template->name) :
          QualifiedName();
      if(!info.source_template->declaring_scope) {
        out.name.name = info.source_template->name;
      }
    }
    const bool fully_bound =
        template_arguments_fully_bind_parameters(info.source_template->parameters,
                                                 out.arguments);
    const std::vector<std::string> * const metadata_arg_texts =
        template_metadata::selected_argument_texts(
            info,
            fully_bound,
            out.arguments.size());
    if(parsed_template_id &&
       template_metadata::argument_texts_contain_pack_expansion(parsed_arg_texts)) {
      out.argument_texts = parsed_arg_texts;
    } else if(metadata_arg_texts) {
      out.argument_texts = *metadata_arg_texts;
    } else {
      out.argument_texts.clear();
      out.argument_texts.reserve(out.arguments.size());
      for(std::size_t i = 0; i < out.arguments.size(); ++i) {
        out.argument_texts.push_back(
            template_argument_text_for_matching(services, out.arguments[i]));
      }
    }
    if(out.argument_texts.empty() && parsed_template_id) {
      out.argument_texts = parsed_arg_texts;
    }
    return true;
  }

  if(have_dependent_class_template) {
    ClassTemplateDecl * source_template =
        static_cast<ClassTemplateDecl *>(dependent_class_template_decl);
    if(source_template) {
      out.source_template = source_template;
      out.name = source_template->declaring_scope ?
          semantic_lookup::scope_qualified_name_syntax(
              *source_template->declaring_scope,
              source_template->name) :
          QualifiedName();
      if(!source_template->declaring_scope) {
        out.name.name = source_template->name;
      }
      const bool parsed_args_keep_pack_expansion =
          template_metadata::argument_texts_contain_pack_expansion(parsed_arg_texts);
      const bool dependent_args_keep_pack_expansion =
          dependent_class_args_have_pack_expansion;
      out.argument_texts.clear();
      out.argument_texts.reserve(dependent_class_args.size());
      out.arguments.clear();
      out.arguments.reserve(dependent_class_args.size());
      const bool use_normalized_arg_texts =
          parsed_template_id &&
          parsed_arg_texts.size() == dependent_class_args.size() &&
          (!dependent_args_keep_pack_expansion ||
           parsed_args_keep_pack_expansion);
      for(std::size_t i = 0; i < dependent_class_args.size(); ++i) {
        const DependentAliasTemplateArgumentSyntax & dependent_arg =
            dependent_class_args[i];
        out.argument_texts.push_back(
            use_normalized_arg_texts ? trim_space(parsed_arg_texts[i]) :
                                       dependent_class_arg_texts[i]);
        if(dependent_arg.type) {
          TemplateArgument arg;
          arg.kind = TemplateArgument::TA_TYPE;
          arg.type = dependent_arg.type;
          arg.text = trim_space(dependent_arg.text);
          arg.dependent =
              service_type_depends_on_template_parameter(services, dependent_arg.type);
          attach_template_argument_source_syntax(&dependent_arg.syntax, arg);
          out.arguments.push_back(arg);
        }
      }
      return true;
    }
  }

  if(parsed_template_id && named_type_is_dependent_alias(base)) {
    void * dependent_alias_template_decl = nullptr;
    std::vector<DependentAliasTemplateArgumentSyntax> dependent_alias_args;
    AliasTemplateDecl * alias_template =
        named_type_dependent_alias_template(base,
                                            dependent_alias_template_decl,
                                            dependent_alias_args) ?
            static_cast<AliasTemplateDecl *>(dependent_alias_template_decl) :
            nullptr;
    AliasTemplateDecl * visible_alias_template = alias_template ?
        template_argument_semantics::lookup_alias_template(
            services, lookup_scope, qualified_name_text(parsed_name)) :
        nullptr;
    if(alias_template && visible_alias_template == alias_template) {
      std::vector<std::string> alias_arg_texts;
      std::vector<TemplateArgumentSyntax> alias_arg_syntaxes;
      alias_arg_texts.reserve(dependent_alias_args.size());
      alias_arg_syntaxes.reserve(dependent_alias_args.size());
      for(std::size_t i = 0; i < dependent_alias_args.size(); ++i) {
        alias_arg_texts.push_back(dependent_alias_args[i].text);
        alias_arg_syntaxes.push_back(dependent_alias_args[i].syntax);
      }

      TypePtr expanded_type;
      const std::vector<TemplateArgumentSyntax> * alias_arg_syntaxes_ptr =
          alias_arg_syntaxes.size() == alias_arg_texts.size() ?
              &alias_arg_syntaxes :
              nullptr;
      if(template_specialization::expand_alias_template_pattern_type(
             services,
             template_api::make_template_environment(lookup_scope),
             parsed_name,
             alias_arg_texts,
             expanded_type,
             alias_arg_syntaxes_ptr,
             template_api::make_template_environment(lookup_scope),
             true) &&
         expanded_type && !type_equals(expanded_type, base)) {
        TypePtr expanded_owner;
        std::vector<std::string> expanded_members;
        bool expanded_leading_typename = false;
        if(named_type_dependent_qualified_member(expanded_type,
                                                 expanded_owner,
                                                 expanded_members,
                                                 expanded_leading_typename)) {
          return false;
        }
        return decompose_template_instantiation(services, lookup_scope, expanded_type, out);
      }
    }
  }
  if(!parsed_template_id) {
    return false;
  }

  ClassTemplateDecl * source_template =
      template_argument_semantics::lookup_class_template(
          services, lookup_scope, qualified_name_text(parsed_name));
  if(!source_template) {
    return false;
  }

  out.source_template = source_template;
  out.name = parsed_name;
  out.argument_texts = parsed_arg_texts;
  return true;
}

bool resolve_template_argument_for_deduction(
    SemanticContext & ctx,
    Scope & argument_scope,
    Scope & parameter_scope,
    const TemplateParameterInfo & parameter,
    const std::string & text,
    TemplateArgument & out)
{
  return resolve_template_argument(ctx, argument_scope, parameter_scope, parameter, text, out);
}

bool resolve_template_argument_for_deduction(
    template_api::TemplateServices & services,
    Scope & argument_scope,
    Scope & parameter_scope,
    const TemplateParameterInfo & parameter,
    const std::string & text,
    TemplateArgument & out)
{
  return template_resolution::resolve_template_argument(
      services,
      template_api::make_template_environment(argument_scope),
      template_api::make_template_environment(parameter_scope),
      parameter,
      text,
      out);
}

void bind_single_template_argument_into_scope(
    template_api::TemplateServices & services,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    const TemplateArgument & argument)
{
  if(parameter.parameter_pack || parameter.name.empty()) {
    return;
  }
  std::vector<TemplateParameterInfo> parameters(1, parameter);
  std::vector<TemplateArgument> arguments(1, argument);
  template_api::bind_template_arguments_into_scope(services, scope, parameters, arguments);
}

bool template_instantiations_match(const DecomposedTemplateInstantiation & lhs,
                                   const DecomposedTemplateInstantiation & rhs)
{
  if(lhs.source_template && rhs.source_template) {
    return same_inline_namespace_class_template_entity_for_deduction(lhs.source_template,
                                                                     rhs.source_template);
  }
  return template_names_match(lhs.name, rhs.name);
}

std::string template_parameter_binding_summary(const Scope & scope,
                                               const TemplateParameterInfo & parameter)
{
  if(parameter.name.empty()) {
    return std::string("<unnamed>");
  }

  std::ostringstream out;
  bool found = false;
  std::map<std::string, TypePtr>::const_iterator type_found = scope.named_types.find(parameter.name);
  if(type_found != scope.named_types.end()) {
    out << "type=" << (type_found->second ? describe_type(type_found->second) :
                                           std::string("<null>"))
        << " bound=" << (scope.template_bound_type_names.count(parameter.name) ? "yes" : "no");
    found = true;
  }
  std::map<std::string, ValueBinding>::const_iterator value_found = scope.values.find(parameter.name);
  if(value_found != scope.values.end()) {
    if(found) {
      out << ' ';
    }
    out << "value-type="
        << (value_found->second.type ? describe_type(value_found->second.type) :
                                       std::string("<null>"))
        << " dependent=" << (value_found->second.dependent_template_value ? "yes" : "no");
    found = true;
  }
  std::map<std::string, ClassTemplateDecl *>::const_iterator class_template_found =
      scope.class_templates.find(parameter.name);
  if(class_template_found != scope.class_templates.end()) {
    if(found) {
      out << ' ';
    }
    out << "class-template=" << (class_template_found->second ? "set" : "placeholder");
    found = true;
  }
  if(scope.alias_templates.find(parameter.name) != scope.alias_templates.end()) {
    if(found) {
      out << ' ';
    }
    out << "alias-template=set";
    found = true;
  }
  return found ? out.str() : std::string("<none>");
}

std::string template_parameter_ancestor_binding_summary(const Scope * scope,
                                                        const Scope * stop_before,
                                                        const TemplateParameterInfo & parameter)
{
  std::size_t depth = 0;
  for(const Scope * current = scope; current && current != stop_before;
      current = current->parent, ++depth) {
    const std::string binding = template_parameter_binding_summary(*current, parameter);
    if(binding != "<none>") {
      std::ostringstream out;
      out << "depth=" << depth
          << " scope=" << semantic_trace::scope_name_for_diagnostic(*current)
          << " binding={" << binding << "}";
      return out.str();
    }
  }
  return std::string("<none>");
}

void trace_template_parameter_bindings(const char * stage,
                                       FunctionTemplateDecl & decl,
                                       Scope & bound_scope,
                                       Scope * use_scope)
{
  if(!parser_trace::enabled("template.resolve")) {
    return;
  }
  for(std::size_t i = 0; i < decl.parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = decl.parameters[i];
    if(parameter.name.empty()) {
      continue;
    }
    std::ostringstream trace;
    trace << "template-param-binding stage=" << stage
          << " template=" << decl.name
          << " param=" << parameter.name
          << " bound_scope={" << template_parameter_binding_summary(bound_scope, parameter) << "}";
    if(decl.declaring_scope) {
      trace << " declaring_scope={"
            << template_parameter_binding_summary(*decl.declaring_scope, parameter) << "}";
    }
    if(use_scope) {
      trace << " use_scope={" << template_parameter_binding_summary(*use_scope, parameter) << "}";
      trace << " use_scope_ancestor={"
            << template_parameter_ancestor_binding_summary(use_scope,
                                                           decl.declaring_scope,
                                                           parameter)
            << "}";
    }
    parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
  }
}

void trace_function_template_drift(const char * stage,
                                   FunctionTemplateDecl & decl)
{
  if(!parser_trace::enabled("template.resolve") || decl.debug_signature.empty()) {
    return;
  }
  const std::string current = semantic_trace::function_template_signature_for_diagnostic(decl);
  if(current == decl.debug_signature) {
    return;
  }
  std::ostringstream trace;
  trace << "template-drift stage=" << stage
        << " template=" << decl.name
        << " decl="
        << (decl.debug_decl_location.empty() ? std::string("<none>") : decl.debug_decl_location)
        << " scope="
        << (decl.debug_scope_name.empty() ? std::string("<none>") : decl.debug_scope_name)
        << " original={" << decl.debug_signature << "}"
        << " current={" << current << "}";
  if(!decl.debug_decl_location_details.empty()) {
    trace << " details={" << decl.debug_decl_location_details << "}";
  }
  parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
}

bool is_forwarding_reference_pattern(const std::vector<TemplateParameterInfo> & parameters,
                                     const TypePtr & pattern)
{
  TypePtr base = strip_top_level_cv(pattern);
  if(!base || base->kind != Type::TK_RVALUE_REFERENCE) {
    return false;
  }

  TypePtr inner = base->inner;
  if(!inner || inner->kind != Type::TK_NAMED) {
    return false;
  }

  const TemplateParameterInfo * parameter =
      find_template_parameter(parameters, inner->named_key);
  return parameter && parameter->kind == TemplateParameterInfo::TP_TYPE;
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

const TemplateParameterInfo * find_template_parameter_for_text(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  std::string trimmed = trim_space(text);
  if(trimmed.size() >= 3 &&
     trimmed.compare(trimmed.size() - 3, 3, "...") == 0) {
    trimmed.erase(trimmed.size() - 3);
    trimmed = trim_space(trimmed);
  }
  const TemplateParameterInfo * parameter =
      find_template_parameter_by_name(parameters, trimmed);
  if(!parameter) {
    parameter = find_template_parameter(parameters, trimmed);
  }
  return parameter;
}

bool record_deduced_non_type_value(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text,
    long long value,
    DeducedValueMap & deduced_values)
{
  const TemplateParameterInfo * parameter =
      find_template_parameter_for_text(parameters, text);
  if(!parameter || parameter->kind != TemplateParameterInfo::TP_NON_TYPE ||
     parameter->name.empty()) {
    return false;
  }

  DeducedValueMap::iterator found = deduced_values.find(parameter->name);
  if(found == deduced_values.end()) {
    deduced_values[parameter->name] = value;
    return true;
  }
  return found->second == value;
}

bool try_resolve_non_type_template_parameter_type(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out);

bool try_resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out);

std::string non_type_template_parameter_spelling(
    const TemplateParameterInfo & parameter)
{
  std::string out = trim_space(parameter.non_type_decl_specifier_text);
  if(out.empty() && parameter.non_type_decl_specifier_seq) {
    out = trim_space(node_text(*parameter.non_type_decl_specifier_seq));
  }
  if(parameter.non_type_declarator) {
    const std::string declarator_text = trim_space(node_text(*parameter.non_type_declarator));
    if(!declarator_text.empty()) {
      if(!out.empty()) {
        out += " ";
      }
      out += declarator_text;
    }
  } else if(parameter.non_type_abstract_declarator) {
    const std::string declarator_text =
        trim_space(node_text(*parameter.non_type_abstract_declarator));
    if(!declarator_text.empty()) {
      if(!out.empty()) {
        out += " ";
      }
      out += declarator_text;
    }
  }
  return out;
}

bool non_type_template_parameter_is_still_dependent(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return non_type_template_parameter_is_still_dependent(
            services, template_api::make_template_environment(scope), parameter);
      });
}

bool non_type_template_parameter_is_still_dependent(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter)
{
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto type_is_dependent =
      [&type_system](const TypePtr & type) -> bool
      {
        return template_argument_semantics::type_depends_on_template_parameter(
            type_system,
            type);
      };
  const auto type_argument_text =
      [&type_system](const TypePtr & type) -> std::string
      {
        return deduction_lookup_type_text(type_system, type);
      };
  if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
     parameter.non_type_decl_specifier_seq) {
    // Prefer the retained declaration syntax over recovered value_type text
    // so dependent defaults do not finalize before substitution.
    Scope & raw_scope = scope.require();
    TypePtr structured_value_type;
    try {
      if(template_decl_ast::parse_type_specifier_seq(
             services,
             raw_scope,
             raw_scope,
             *parameter.non_type_decl_specifier_seq,
             structured_value_type,
             true,
             true) &&
         structured_value_type) {
        template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
            services, scope, structured_value_type);
        if(type_is_dependent(structured_value_type)) {
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "non-type-param-dependent name=" << parameter.name
                  << " value-type=" << describe_type(structured_value_type)
                  << " structural-dependent=yes";
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          return true;
        }
        return false;
      }
    } catch(const TemplateSubstitutionFailure &) {
    } catch(const SemanticSoftFailure &) {
    } catch(const SemanticDiagnosticError &) {
    }
  }
  if(parameter.value_type) {
    TypePtr resolved_value_type;
    if(recover_non_type_parameter_value_type(
           services,
           scope,
           parameter.value_type,
           resolved_value_type) &&
       resolved_value_type &&
       !type_is_dependent(resolved_value_type)) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "non-type-param-dependent name=" << parameter.name
              << " value-type=" << describe_type(parameter.value_type)
              << " resolved=" << describe_type(resolved_value_type)
              << " structural-dependent=no";
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }
    if(type_is_dependent(parameter.value_type)) {
      const std::string value_type_text = type_argument_text(parameter.value_type);
      if(!value_type_text.empty() &&
         !text_mentions_template_dependency(services, scope, value_type_text)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "non-type-param-dependent name=" << parameter.name
                << " value-type=" << describe_type(parameter.value_type)
                << " value-type-text=" << value_type_text
                << " structural-dependent=no";
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return false;
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "non-type-param-dependent name=" << parameter.name
              << " value-type=" << describe_type(parameter.value_type)
              << " structural-dependent=yes";
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return true;
    }
  }

  const std::string text = non_type_template_parameter_spelling(parameter);
  const bool mentions_placeholders =
      !text.empty() &&
      template_argument_semantics::text_mentions_template_placeholders(
          services, scope, text);
  const bool mentions_dependent =
      !text.empty() &&
      template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
          services, scope, text);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "non-type-param-dependent name=" << parameter.name
          << " spelling=" << text
          << " mentions-placeholders=" << (mentions_placeholders ? "yes" : "no")
          << " mentions-dependent-bindings=" << (mentions_dependent ? "yes" : "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return mentions_placeholders || mentions_dependent;
}

bool text_mentions_template_dependency(template_api::TemplateServices & services,
                                       template_api::TemplateEnvironmentHandle scope,
                                       const std::string & text)
{
  return !text.empty() &&
         (template_argument_semantics::text_mentions_template_placeholders(
              services, scope, text) ||
          template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
              services, scope, text));
}

bool should_defer_unresolved_type_lookup(template_api::TemplateServices & services,
                                         template_api::TemplateEnvironmentHandle scope,
                                         const std::string & text)
{
  return text_mentions_template_dependency(services, scope, text);
}

std::string default_argument_expression_text(const CppAstNode & node)
{
  const std::string described = callsemantic_internal::describe_expression_for_diagnostic(node);
  if(!described.empty() &&
     (node.children.empty() || described != node.value)) {
    return described;
  }

  const std::string text = node_text(node);
  if(!text.empty()) {
    return text;
  }

  return node.value;
}

bool default_argument_expression_is_still_dependent(
    SemanticContext & ctx,
    Scope & scope,
    const CppAstNode & node)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return default_argument_expression_is_still_dependent(
            services, template_api::make_template_environment(scope), node);
      });
}

bool default_argument_expression_is_still_dependent(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const CppAstNode & node)
{
  std::string text = default_argument_expression_text(node);
  return !text.empty() &&
         text_mentions_template_dependency(services, scope, text);
}

bool non_type_template_parameter_has_non_dependent_binding(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter)
{
  if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
     parameter.name.empty()) {
    return false;
  }

  std::map<std::string, ValueBinding>::const_iterator found =
      scope.values.find(parameter.name);
  return found != scope.values.end() &&
         !found->second.dependent_template_value &&
         found->second.type &&
         !template_argument_semantics::type_depends_on_template_parameter(ctx, found->second.type);
}

bool try_bind_resolvable_default_non_type_template_argument(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter)
{
  if(parameter.parameter_pack ||
     parameter.kind != TemplateParameterInfo::TP_NON_TYPE ||
     parameter.name.empty() ||
     !parameter.default_argument ||
     parameter.default_argument->children.empty() ||
     non_type_template_parameter_has_non_dependent_binding(ctx, scope, parameter)) {
    return false;
  }

  TypePtr bound_value_type;
  if(!try_resolve_non_type_template_parameter_type(ctx, scope, parameter, bound_value_type)) {
    return false;
  }

  const CppAstNode & child = parameter.default_argument->children[0];
  long long value = 0;
  bool evaluated = false;
  try {
    evaluated = ctx.evaluate_constant_expression(scope, child, value);
  } catch(const std::logic_error &) {
    evaluated = false;
  }
  if(!evaluated) {
    return false;
  }

  TemplateArgument argument;
  argument.kind = TemplateArgument::TA_VALUE;
  argument.type = bound_value_type;
  argument.value = value;
  argument.text = typed_non_type_template_argument_text(ctx, bound_value_type, value);
  ctx.bind_single_template_argument_into_scope(scope, parameter, argument);
  return true;
}

void bind_resolvable_default_non_type_template_arguments_into_scope(
    SemanticContext & ctx,
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters)
{
  bool progress = true;
  while(progress) {
    progress = false;
    for(std::size_t i = 0; i < parameters.size(); ++i) {
      if(try_bind_resolvable_default_non_type_template_argument(
             ctx, scope, parameters[i])) {
        progress = true;
      }
    }
  }
}

bool function_template_has_trailing_parameter_pack(FunctionTemplateDecl & decl)
{
  if(decl.has_trailing_function_parameter_pack) {
    decl.trailing_function_parameter_pack_analyzed = true;
    return true;
  }
  if(decl.trailing_function_parameter_pack_analyzed) {
    return decl.has_trailing_function_parameter_pack;
  }
  if(decl.declarator) {
    decl.has_trailing_function_parameter_pack =
        pack_parameter_analysis::declarator_has_trailing_template_parameter_pack(
            *decl.declarator,
            decl.parameters);
  }
  decl.trailing_function_parameter_pack_analyzed = true;
  return decl.has_trailing_function_parameter_pack;
}

std::size_t first_template_parameter_pack_index(
    const std::vector<TemplateParameterInfo> & parameters)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      return i;
    }
  }
  return parameters.size();
}

bool function_template_accepts_argument_count(FunctionTemplateDecl & decl,
                                              std::size_t argument_count)
{
  std::size_t required_count = decl.params_pattern.size();
  const bool has_trailing_pack = function_template_has_trailing_parameter_pack(decl);
  if(has_trailing_pack && required_count > 0) {
    --required_count;
  }
  while(required_count > 0 &&
        required_count - 1 < decl.default_arguments_pattern.size() &&
        decl.default_arguments_pattern[required_count - 1]) {
    --required_count;
  }

  if(argument_count < required_count) {
    return false;
  }

  if(has_trailing_pack) {
    return argument_count + 1 >= decl.params_pattern.size();
  }

  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count >= decl.params_pattern.size()) {
    return true;
  }
  return argument_count <= decl.params_pattern.size();
}

bool uses_trailing_function_parameter_pack(FunctionTemplateDecl & decl)
{
  return function_template_has_trailing_parameter_pack(decl);
}

bool type_mentions_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type);

bool type_mentions_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type);

bool type_mentions_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type);

bool function_template_parameter_has_non_dependent_binding(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter);

bool template_argument_text_mentions_function_template_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const std::string & text)
{
  if(find_template_parameter_for_text(parameters, text)) {
    return true;
  }
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(!parameters[i].name.empty() &&
       contains_identifier_token(text, parameters[i].name)) {
      return true;
    }
  }
  return false;
}

bool template_argument_mentions_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type &&
     type_mentions_function_template_parameter(ctx, parameters, argument.type)) {
    return true;
  }
  return !argument.text.empty() &&
         template_argument_text_mentions_function_template_parameter(
             parameters, argument.text);
}

bool dependent_alias_arguments_mention_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  void * alias_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_alias_template(type, alias_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_function_template_parameter(ctx, parameters, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_function_template_parameter(
           parameters, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

bool dependent_class_arguments_mention_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  void * class_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_class_template(type, class_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_function_template_parameter(ctx, parameters, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_function_template_parameter(
           parameters, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

bool dependent_named_type_head_is_template_template_parameter(
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base ||
     base->kind != Type::TK_NAMED ||
     base->named_semantic_kind != Type::NSK_DEPENDENT_TYPE) {
    return false;
  }

  const auto head_matches =
      [](const std::string & text, const std::string & name) -> bool
      {
        std::string trimmed = strip_elaborated_type_prefix(trim_space(text));
        if(trimmed.compare(0, 6, "class ") == 0) {
          trimmed = trim_space(trimmed.substr(6));
        } else if(trimmed.compare(0, 7, "struct ") == 0) {
          trimmed = trim_space(trimmed.substr(7));
        } else if(trimmed.compare(0, 9, "typename ") == 0) {
          trimmed = trim_space(trimmed.substr(9));
        }
        if(trimmed.size() <= name.size() ||
           trimmed.compare(0, name.size(), name) != 0) {
          return false;
        }
        const unsigned char next =
            static_cast<unsigned char>(trimmed[name.size()]);
        return trimmed[name.size()] == '<' ||
               (std::isspace(next) &&
                trim_space(trimmed.substr(name.size())).compare(0, 1, "<") == 0);
      };

  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].kind != TemplateParameterInfo::TP_TEMPLATE_TEMPLATE ||
       parameters[i].name.empty()) {
      continue;
    }
    if(head_matches(base->named_display, parameters[i].name) ||
       head_matches(base->named_key, parameters[i].name) ||
       head_matches(base->named_semantic_payload, parameters[i].name)) {
      return true;
    }
  }
  return false;
}

bool template_argument_mentions_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type &&
     type_mentions_function_template_parameter(type_system, parameters, argument.type)) {
    return true;
  }
  return !argument.text.empty() &&
         template_argument_text_mentions_function_template_parameter(
             parameters, argument.text);
}

bool dependent_alias_arguments_mention_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  void * alias_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_alias_template(type, alias_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_function_template_parameter(
           type_system, parameters, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_function_template_parameter(
           parameters, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

bool dependent_class_arguments_mention_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  void * class_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_class_template(type, class_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_function_template_parameter(
           type_system, parameters, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_function_template_parameter(
           parameters, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

bool template_argument_text_mentions_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const std::string & text)
{
  const TemplateParameterInfo * direct =
      find_template_parameter_for_text(parameters, text);
  if(direct &&
     !function_template_parameter_has_non_dependent_binding(ctx, scope, *direct)) {
    return true;
  }
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].name.empty() ||
       !contains_identifier_token(text, parameters[i].name)) {
      continue;
    }
    if(!function_template_parameter_has_non_dependent_binding(
           ctx, scope, parameters[i])) {
      return true;
    }
  }
  return false;
}

bool template_argument_mentions_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE && argument.type &&
     type_mentions_unbound_function_template_parameter(
         ctx, parameters, scope, argument.type)) {
    return true;
  }
  return !argument.text.empty() &&
         template_argument_text_mentions_unbound_function_template_parameter(
             ctx, parameters, scope, argument.text);
}

bool dependent_alias_arguments_mention_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type)
{
  void * alias_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_alias_template(type, alias_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_unbound_function_template_parameter(
           ctx, parameters, scope, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_unbound_function_template_parameter(
           ctx, parameters, scope, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

bool dependent_class_arguments_mention_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type)
{
  void * class_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  if(!named_type_dependent_class_template(type, class_template_decl, arguments)) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(arguments[i].type &&
       type_mentions_unbound_function_template_parameter(
           ctx, parameters, scope, arguments[i].type)) {
      return true;
    }
    if(!arguments[i].text.empty() &&
       template_argument_text_mentions_unbound_function_template_parameter(
           ctx, parameters, scope, arguments[i].text)) {
      return true;
    }
  }
  return false;
}

std::size_t function_template_deduction_parameter_count(FunctionTemplateDecl & decl,
                                                        std::size_t argument_count)
{
  if(uses_trailing_function_parameter_pack(decl)) {
    return argument_count;
  }
  TypePtr function_type = strip_top_level_cv(decl.type_pattern);
  if(function_type &&
     function_type->kind == Type::TK_FUNCTION &&
     (function_type->variadic || function_type->prototype_relaxed) &&
     argument_count > decl.params_pattern.size()) {
    return decl.params_pattern.size();
  }
  return argument_count;
}

bool type_mentions_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    if(find_template_parameter(parameters, base->named_key) ||
       find_template_parameter_by_name(parameters, base->named_key) ||
       (!base->named_display.empty() &&
        (find_template_parameter(parameters, base->named_display) ||
         find_template_parameter_by_name(parameters, base->named_display)))) {
      return true;
    }
    if(dependent_alias_arguments_mention_function_template_parameter(
           ctx, parameters, base)) {
      return true;
    }
    if(dependent_class_arguments_mention_function_template_parameter(
           ctx, parameters, base)) {
      return true;
    }
    if(dependent_named_type_head_is_template_template_parameter(parameters, base)) {
      return true;
    }

    ClassInfo * info = ctx.class_info_for_type(base);
    if(info) {
      for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
        if(template_argument_mentions_function_template_parameter(
               ctx, parameters, info->instantiation_arguments[i])) {
          return true;
        }
      }
    }
    return false;
  }

  case Type::TK_CV:
    return type_mentions_function_template_parameter(ctx, parameters, base->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return type_mentions_function_template_parameter(ctx, parameters, base->inner);

  case Type::TK_ARRAY:
    return (!base->bound_text.empty() &&
            template_argument_text_mentions_function_template_parameter(
                parameters, base->bound_text)) ||
           type_mentions_function_template_parameter(ctx, parameters, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_function_template_parameter(ctx, parameters, base->owner) ||
           type_mentions_function_template_parameter(ctx, parameters, base->inner);

  case Type::TK_FUNCTION:
    if(type_mentions_function_template_parameter(ctx, parameters, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_mentions_function_template_parameter(ctx, parameters, base->params[i])) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

bool type_mentions_function_template_parameter(
    template_api::TemplateTypeSystem & type_system,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    if(find_template_parameter(parameters, base->named_key) ||
       find_template_parameter_by_name(parameters, base->named_key) ||
       (!base->named_display.empty() &&
        (find_template_parameter(parameters, base->named_display) ||
         find_template_parameter_by_name(parameters, base->named_display)))) {
      return true;
    }
    if(dependent_alias_arguments_mention_function_template_parameter(
           type_system, parameters, base)) {
      return true;
    }
    if(dependent_class_arguments_mention_function_template_parameter(
           type_system, parameters, base)) {
      return true;
    }
    if(dependent_named_type_head_is_template_template_parameter(parameters, base)) {
      return true;
    }

    template_api::TemplateNamedTypeMetadata info;
    if(template_api::describe_named_type_metadata(type_system.model, base, info)) {
      for(std::size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
        if(template_argument_mentions_function_template_parameter(
               type_system, parameters, info.instantiation_arguments[i])) {
          return true;
        }
      }
    }
    return false;
  }

  case Type::TK_CV:
    return type_mentions_function_template_parameter(
        type_system, parameters, base->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return type_mentions_function_template_parameter(
        type_system, parameters, base->inner);

  case Type::TK_ARRAY:
    return (!base->bound_text.empty() &&
            template_argument_text_mentions_function_template_parameter(
                parameters, base->bound_text)) ||
           type_mentions_function_template_parameter(
               type_system, parameters, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_function_template_parameter(
               type_system, parameters, base->owner) ||
           type_mentions_function_template_parameter(
               type_system, parameters, base->inner);

  case Type::TK_FUNCTION:
    if(type_mentions_function_template_parameter(
           type_system, parameters, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_mentions_function_template_parameter(
             type_system, parameters, base->params[i])) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

bool function_template_parameter_has_non_dependent_binding(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter)
{
  if(parameter.name.empty()) {
    return false;
  }

  if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
    if(scope.template_bound_type_names.find(parameter.name) !=
       scope.template_bound_type_names.end()) {
      return false;
    }
    std::map<std::string, TypePtr>::const_iterator found =
        scope.named_types.find(parameter.name);
    return found != scope.named_types.end() &&
           found->second &&
           !template_argument_semantics::type_depends_on_template_parameter(ctx, found->second);
  }

  if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
    return non_type_template_parameter_has_non_dependent_binding(
        ctx, scope, parameter);
  }

  if(parameter.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
    std::map<std::string, ClassTemplateDecl *>::const_iterator class_template_found =
        scope.class_templates.find(parameter.name);
    if(class_template_found != scope.class_templates.end() &&
       class_template_found->second) {
      return true;
    }
    return scope.alias_templates.find(parameter.name) != scope.alias_templates.end();
  }

  return false;
}

bool type_mentions_unbound_function_template_parameter(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_NAMED:
  {
    const TemplateParameterInfo * parameter =
        find_template_parameter(parameters, base->named_key);
    if(!parameter) {
      parameter = find_template_parameter_by_name(parameters, base->named_key);
    }
    if(!parameter && !base->named_display.empty()) {
      parameter = find_template_parameter(parameters, base->named_display);
      if(!parameter) {
        parameter = find_template_parameter_by_name(parameters, base->named_display);
      }
    }
    if(parameter &&
       !function_template_parameter_has_non_dependent_binding(ctx, scope, *parameter)) {
      return true;
    }
    if(dependent_alias_arguments_mention_unbound_function_template_parameter(
           ctx, parameters, scope, base)) {
      return true;
    }
    if(dependent_class_arguments_mention_unbound_function_template_parameter(
           ctx, parameters, scope, base)) {
      return true;
    }
    if(dependent_named_type_head_is_template_template_parameter(parameters, base)) {
      return true;
    }

    ClassInfo * info = ctx.class_info_for_type(base);
    if(info) {
      for(std::size_t i = 0; i < info->instantiation_arguments.size(); ++i) {
        if(template_argument_mentions_unbound_function_template_parameter(
               ctx, parameters, scope, info->instantiation_arguments[i])) {
          return true;
        }
      }
    }
    return false;
  }

  case Type::TK_CV:
    return type_mentions_unbound_function_template_parameter(
        ctx, parameters, scope, base->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return type_mentions_unbound_function_template_parameter(
        ctx, parameters, scope, base->inner);

  case Type::TK_ARRAY:
    return (!base->bound_text.empty() &&
            template_argument_text_mentions_unbound_function_template_parameter(
                ctx, parameters, scope, base->bound_text)) ||
           type_mentions_unbound_function_template_parameter(
               ctx, parameters, scope, base->inner);

  case Type::TK_MEMBER_POINTER:
    return type_mentions_unbound_function_template_parameter(
               ctx, parameters, scope, base->owner) ||
           type_mentions_unbound_function_template_parameter(
               ctx, parameters, scope, base->inner);

  case Type::TK_FUNCTION:
    if(type_mentions_unbound_function_template_parameter(
           ctx, parameters, scope, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(type_mentions_unbound_function_template_parameter(
             ctx, parameters, scope, base->params[i])) {
        return true;
      }
    }
    return false;

  default:
    return false;
  }
}

void bind_empty_template_parameter_pack(Scope & scope,
                                        const TemplateParameterInfo & parameter)
{
  template_scope::bind_empty_parameter_pack(scope, parameter);
}

bool append_deduced_pack_argument(SemanticContext & ctx,
                                  Scope & bound_scope,
                                  const TemplateParameterInfo & parameter,
                                  const DeducedTypeMap & deduced_types,
                                  const DeducedValueMap & deduced_values,
                                  DeducedPackArgumentMap & out)
{
  if(parameter.name.empty()) {
    return false;
  }

  TemplateArgument arg;
  if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
    DeducedTypeMap::const_iterator found = deduced_types.find(parameter.name);
    if(found == deduced_types.end()) {
      return false;
    }
    arg.kind = TemplateArgument::TA_TYPE;
    arg.type = found->second;
    arg.text = template_argument_semantics::type_depends_on_template_parameter(ctx, found->second) ?
                   parameter.name :
                   deduction_lookup_type_text(ctx, found->second);
  } else if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
    DeducedValueMap::const_iterator found = deduced_values.find(parameter.name);
    if(found == deduced_values.end()) {
      return false;
    }
    TypePtr bound_value_type;
    if(!try_resolve_non_type_template_parameter_type(
           ctx, bound_scope, parameter, bound_value_type)) {
      const bool still_dependent =
          non_type_template_parameter_is_still_dependent(ctx, bound_scope, parameter);
      if(!still_dependent) {
        return false;
      }
      bound_value_type = parameter.value_type;
    }
    arg.kind = TemplateArgument::TA_VALUE;
    arg.type = bound_value_type;
    arg.value = found->second;
    arg.text = typed_non_type_template_argument_text(ctx, bound_value_type, found->second);
  } else {
    return false;
  }

  out[parameter.name].push_back(arg);
  return true;
}

bool template_arguments_equivalent(const TemplateArgument & lhs,
                                   const TemplateArgument & rhs)
{
  if(lhs.kind != rhs.kind || lhs.dependent != rhs.dependent) {
    return false;
  }
  if(lhs.kind == TemplateArgument::TA_TYPE) {
    return type_equals(lhs.type, rhs.type);
  }
  if(lhs.kind == TemplateArgument::TA_VALUE) {
    const bool types_match =
        (!lhs.type && !rhs.type) ||
        (lhs.type && rhs.type && type_equals(lhs.type, rhs.type));
    return types_match &&
           lhs.value == rhs.value &&
           lhs.text == rhs.text;
  }
  return lhs.template_decl == rhs.template_decl &&
         lhs.text == rhs.text;
}

bool pack_argument_matches_scalar_deduction(SemanticContext & ctx,
                                            Scope & bound_scope,
                                            const TemplateParameterInfo & parameter,
                                            const DeducedTypeMap & deduced_types,
                                            const DeducedValueMap & deduced_values,
                                            const TemplateArgument & pack_argument)
{
  DeducedPackArgumentMap scalar_as_pack;
  if(!append_deduced_pack_argument(ctx,
                                   bound_scope,
                                   parameter,
                                   deduced_types,
                                   deduced_values,
                                   scalar_as_pack)) {
    return false;
  }
  DeducedPackArgumentMap::const_iterator found =
      scalar_as_pack.find(parameter.name);
  return found != scalar_as_pack.end() &&
         found->second.size() == 1 &&
         template_arguments_equivalent(found->second[0], pack_argument);
}

bool append_deduced_pack_arguments(const TemplateParameterInfo & parameter,
                                   const std::vector<TemplateArgument> & arguments,
                                   DeducedPackArgumentMap & out)
{
  if(parameter.name.empty()) {
    return false;
  }
  std::vector<TemplateArgument> & slot = out[parameter.name];
  slot.insert(slot.end(), arguments.begin(), arguments.end());
  return !arguments.empty();
}

bool merge_deduced_pack_arguments(SemanticContext & ctx,
                                  FunctionTemplateDecl & decl,
                                  Scope & bound_scope,
                                  DeducedTypeMap & deduced_types,
                                  DeducedValueMap & deduced_values,
                                  DeducedTypeMap & temp_deduced_types,
                                  DeducedValueMap & temp_deduced_values,
                                  DeducedPackArgumentMap & temp_deduced_pack_arguments,
                                  DeducedPackArgumentMap & deduced_pack_arguments)
{
  bool appended_any = false;
  for(std::size_t i = 0; i < decl.parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = decl.parameters[i];
    if(!parameter.parameter_pack || parameter.name.empty()) {
      continue;
    }
    DeducedPackArgumentMap::iterator pack_found =
        temp_deduced_pack_arguments.find(parameter.name);
    if(pack_found != temp_deduced_pack_arguments.end()) {
      if(pack_found->second.empty()) {
        return false;
      }
      const bool has_scalar_type =
          temp_deduced_types.find(parameter.name) != temp_deduced_types.end();
      const bool has_scalar_value =
          temp_deduced_values.find(parameter.name) != temp_deduced_values.end();
      if((has_scalar_type || has_scalar_value) &&
         (pack_found->second.size() != 1 ||
          !pack_argument_matches_scalar_deduction(ctx,
                                                  bound_scope,
                                                  parameter,
                                                  temp_deduced_types,
                                                  temp_deduced_values,
                                                  pack_found->second[0]))) {
        return false;
      }
      if(append_deduced_pack_arguments(parameter,
                                       pack_found->second,
                                       deduced_pack_arguments)) {
        appended_any = true;
      }
      temp_deduced_pack_arguments.erase(pack_found);
    } else if(append_deduced_pack_argument(ctx,
                                           bound_scope,
                                           parameter,
                                           temp_deduced_types,
                                           temp_deduced_values,
                                           deduced_pack_arguments)) {
      appended_any = true;
    }
    temp_deduced_types.erase(parameter.name);
    temp_deduced_values.erase(parameter.name);
  }
  if(!appended_any) {
    return false;
  }
  deduced_types.swap(temp_deduced_types);
  deduced_values.swap(temp_deduced_values);
  return true;
}

bool finalize_deduced_function_template_arguments(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & bound_scope,
    const DeducedTypeMap & deduced_types,
    const DeducedValueMap & deduced_values,
    const DeducedPackArgumentMap & deduced_pack_arguments,
    const ExplicitFunctionTemplateArgumentBindings * explicit_arguments,
    std::vector<TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  const auto trace_finalize_failure = [&](const std::string & detail)
  {
    if(!parser_trace::enabled("template.resolve")) {
      return;
    }
    std::ostringstream trace;
    trace << "finalize-failed template=" << decl.name
          << " detail=" << detail;
    parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
  };
  out.clear();
  if(pack_sizes_out) {
    pack_sizes_out->clear();
  }
  for(std::size_t i = 0; i < decl.parameters.size(); ++i) {
    if(decl.parameters[i].parameter_pack) {
      if(explicit_arguments &&
         explicit_arguments->pack_parameter_index == i) {
        if(pack_sizes_out && !decl.parameters[i].name.empty()) {
          (*pack_sizes_out)[decl.parameters[i].name] =
              explicit_arguments->pack_arguments.size();
        }
        bind_explicit_function_template_pack_arguments(bound_scope,
                                                      decl.parameters[i],
                                                      explicit_arguments->pack_arguments);
        for(std::size_t j = 0; j < explicit_arguments->pack_arguments.size(); ++j) {
          out.push_back(explicit_arguments->pack_arguments[j]);
        }
        continue;
      }
      DeducedPackArgumentMap::const_iterator pack_found =
          deduced_pack_arguments.find(decl.parameters[i].name);
      if(pack_found == deduced_pack_arguments.end()) {
        if(pack_sizes_out && !decl.parameters[i].name.empty()) {
          (*pack_sizes_out)[decl.parameters[i].name] = 0;
        }
        bind_empty_template_parameter_pack(bound_scope, decl.parameters[i]);
        continue;
      }
      if(pack_sizes_out && !decl.parameters[i].name.empty()) {
        (*pack_sizes_out)[decl.parameters[i].name] = pack_found->second.size();
      }
      template_scope::bind_template_argument_pack(
          bound_scope,
          decl.parameters[i],
          pack_found->second,
          true);
      for(std::size_t j = 0; j < pack_found->second.size(); ++j) {
        out.push_back(pack_found->second[j]);
      }
      continue;
    }

    TemplateArgument arg;
    if(explicit_arguments && i < explicit_arguments->fixed_arguments.size()) {
      arg = explicit_arguments->fixed_arguments[i];
    } else if(decl.parameters[i].kind == TemplateParameterInfo::TP_TYPE) {
      DeducedTypeMap::const_iterator found = deduced_types.find(decl.parameters[i].name);
      if(found == deduced_types.end()) {
        if(!decl.parameters[i].default_argument ||
           decl.parameters[i].default_argument->children.empty()) {
          trace_finalize_failure(std::string("missing-type-parameter name=") +
                                 decl.parameters[i].name);
          return false;
        }
        const CppAstNode & child = decl.parameters[i].default_argument->children[0];
        const std::string original_default_text =
            default_type_argument_text_from_ast(decl.parameters[i], child);
        const TemplateArgumentSyntax original_default_syntax =
            make_default_template_argument_syntax(decl.parameters[i],
                                                  child,
                                                  original_default_text);
        std::string prepared_default_text = original_default_text;
        TemplateArgumentSyntax prepared_default_syntax;
        const TemplateArgumentSyntax * prepared_default_syntax_ptr =
            &original_default_syntax;
        if(!original_default_text.empty()) {
          std::vector<TemplateParameterInfo> prefix_parameters(
              decl.parameters.begin(),
              decl.parameters.begin() + i);
          std::vector<TemplateArgument> prefix_arguments(out.begin(), out.end());
          if(make_substituted_default_template_argument_syntax(
                 ctx,
                 bound_scope,
                 decl.parameters[i],
                 child,
                 original_default_text,
                 prefix_parameters,
                 prefix_arguments,
                 prepared_default_syntax)) {
            prepared_default_text = prepared_default_syntax.text;
            prepared_default_syntax_ptr = &prepared_default_syntax;
          }
        }
        bool resolved_default = false;
        const bool default_mentions_template_placeholders =
            !prepared_default_text.empty() &&
            ctx.text_mentions_template_placeholders(bound_scope,
                                                   prepared_default_text);
        const bool default_mentions_dependent_bindings =
            !prepared_default_text.empty() &&
            ctx.text_mentions_dependent_non_namespace_binding_names(
                bound_scope, prepared_default_text);
        const bool default_should_defer_lookup =
            !prepared_default_text.empty() &&
            ctx.should_defer_unresolved_type_lookup(bound_scope,
                                                    prepared_default_text);
        const bool should_defer_original_default_text =
            !prepared_default_text.empty() &&
            (default_mentions_template_placeholders ||
             default_mentions_dependent_bindings ||
             default_should_defer_lookup);
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "default-type-arg name=" << decl.parameters[i].name
                << " text=" << original_default_text
                << " prepared=" << prepared_default_text
                << " placeholders="
                << (default_mentions_template_placeholders ? "yes" : "no")
                << " dependent-bindings="
                << (default_mentions_dependent_bindings ? "yes" : "no")
                << " defer-lookup="
                << (default_should_defer_lookup ? "yes" : "no")
                << " defer-now="
                << (should_defer_original_default_text ? "yes" : "no");
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        if(should_defer_original_default_text) {
          try {
            const ScopedTemplateArgumentUseLocation default_use_location(
                template_api::normalize_template_witness_source_location(
                    source_location_for_ast_start(ctx.template_witness_context(),
                                                  child)));
            resolved_default = resolve_template_argument(ctx,
                                                        bound_scope,
                                                        bound_scope,
                                                        decl.parameters[i],
                                                        original_default_text,
                                                        &original_default_syntax,
                                                        arg);
          } catch(const TemplateSubstitutionFailure &) {
            resolved_default = false;
          }
          if(resolved_default &&
             (!arg.type ||
              template_argument_semantics::type_depends_on_template_parameter(
                  ctx, arg.type))) {
            arg = TemplateArgument();
            resolved_default = false;
          }
        }
        if(should_defer_original_default_text && !resolved_default) {
          arg.kind = TemplateArgument::TA_TYPE;
          arg.type = make_named(prepared_default_text,
                                "dependent type " + prepared_default_text,
                                true);
          arg.text = prepared_default_text;
          arg.dependent = true;
          attach_template_argument_source_syntax(prepared_default_syntax_ptr, arg);
          resolved_default = true;
        }
        if(!original_default_text.empty()) {
          try {
            if(!resolved_default) {
              resolved_default = resolve_template_argument(ctx,
                                                          bound_scope,
                                                          bound_scope,
                                                          decl.parameters[i],
                                                          original_default_text,
                                                          &original_default_syntax,
                                                          arg);
            }
          } catch(const TemplateSubstitutionFailure &) {
            resolved_default = false;
          }
        }
        if(!resolved_default) {
          if(prepared_default_text.empty() ||
             !resolve_template_argument(ctx,
                                        bound_scope,
                                        bound_scope,
                                        decl.parameters[i],
                                        prepared_default_text,
                                        prepared_default_syntax_ptr,
                                        arg)) {
            trace_finalize_failure(std::string("default-type-resolution-failed name=") +
                                   decl.parameters[i].name +
                                   " text=" + prepared_default_text);
            return false;
          }
        }
      } else {
        arg.type = found->second;
      }
      if(arg.type) {
        arg.text = deduction_lookup_type_text(ctx, arg.type);
      }
      arg.kind = TemplateArgument::TA_TYPE;
      if(found == deduced_types.end()) {
        arg.source_defaulted = true;
      }
    } else if(decl.parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE) {
      TypePtr bound_value_type;
      if(!try_resolve_non_type_template_parameter_type(
             ctx, bound_scope, decl.parameters[i], bound_value_type)) {
        const bool still_dependent =
            non_type_template_parameter_is_still_dependent(ctx,
                                                           bound_scope,
                                                           decl.parameters[i]);
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "finalize-non-type-param name=" << decl.parameters[i].name
                << " spelling=" << non_type_template_parameter_spelling(decl.parameters[i])
                << " resolved-type=no"
                << " still-dependent=" << (still_dependent ? "yes" : "no");
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        if(!still_dependent) {
          return false;
        }
        bound_value_type = decl.parameters[i].value_type;
      }
      DeducedValueMap::const_iterator found = deduced_values.find(decl.parameters[i].name);
      if(found != deduced_values.end()) {
        arg.kind = TemplateArgument::TA_VALUE;
        arg.type = bound_value_type;
        arg.value = found->second;
        arg.text = typed_non_type_template_argument_text(ctx, bound_value_type, found->second);
      } else {
        if(!decl.parameters[i].default_argument ||
           decl.parameters[i].default_argument->children.empty()) {
          return false;
        }
        const CppAstNode & child = decl.parameters[i].default_argument->children[0];
        const std::string default_text = default_argument_expression_text(child);
        const TemplateArgumentSyntax default_syntax =
            make_default_template_argument_syntax(decl.parameters[i],
                                                  child,
                                                  default_text);
        long long value = 0;
        bool evaluated = false;
        std::string eval_error;
        template_argument_semantics::NonTypeArgumentStatus value_status =
            template_argument_semantics::NT_ARG_EVAL_FAILED;
        try {
          value_status =
              template_api::with_template_services(
                  ctx,
                  [&](template_api::TemplateServices & services)
                  {
                    return template_argument_semantics::evaluate_non_type_argument_syntax(
                        services,
                        template_api::make_template_environment(bound_scope),
                        default_syntax,
                        value,
                        &eval_error,
                        bound_value_type);
                  });
          evaluated = value_status == template_argument_semantics::NT_ARG_EVALUATED;
        } catch(const std::logic_error &) {
          evaluated = false;
        }
        if(!evaluated) {
          if(value_status != template_argument_semantics::NT_ARG_DEPENDENT &&
             !default_argument_expression_is_still_dependent(ctx, bound_scope, child)) {
            return false;
          }
          arg.kind = TemplateArgument::TA_VALUE;
          arg.type = bound_value_type;
          arg.text = node_text(child);
          arg.dependent = true;
        } else {
          arg.kind = TemplateArgument::TA_VALUE;
          arg.type = bound_value_type;
          arg.value = value;
          arg.text = typed_non_type_template_argument_text(ctx, bound_value_type, value);
        }
        arg.source_defaulted = true;
      }
    } else if(decl.parameters[i].kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
      DeducedPackArgumentMap::const_iterator found =
          deduced_pack_arguments.find(decl.parameters[i].name);
      if(found == deduced_pack_arguments.end() ||
         found->second.size() != 1 ||
         (found->second[0].kind != TemplateArgument::TA_CLASS_TEMPLATE &&
          found->second[0].kind != TemplateArgument::TA_ALIAS_TEMPLATE)) {
        trace_finalize_failure(std::string("missing-template-template-parameter name=") +
                               decl.parameters[i].name);
        return false;
      }
      arg = found->second[0];
    } else {
      return false;
    }

    out.push_back(arg);
    ctx.bind_single_template_argument_into_scope(bound_scope, decl.parameters[i], arg);
  }
  return true;
}

void bind_known_deductions_into_scope(SemanticContext & ctx,
                                      Scope & scope,
                                      const std::vector<TemplateParameterInfo> & parameters,
                                      const DeducedTypeMap & deduced_types,
                                      const DeducedValueMap & deduced_values)
{
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    const TemplateParameterInfo & parameter = parameters[i];
    if(parameter.name.empty()) {
      continue;
    }

    if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
      std::map<std::string, TypePtr>::iterator existing =
          scope.named_types.find(parameter.name);
      if(existing != scope.named_types.end() &&
         existing->second &&
         !template_argument_semantics::type_depends_on_template_parameter(ctx, existing->second)) {
        continue;
      }
      DeducedTypeMap::const_iterator found = deduced_types.find(parameter.name);
      if(found == deduced_types.end()) {
        continue;
      }

      TemplateArgument argument;
      argument.kind = TemplateArgument::TA_TYPE;
      argument.type = found->second;
      argument.text = deduction_lookup_type_text(ctx, found->second);
      ctx.bind_single_template_argument_into_scope(scope, parameter, argument);
      continue;
    }

    if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
      continue;
    }

    std::map<std::string, ValueBinding>::iterator existing_value =
        scope.values.find(parameter.name);
    if(existing_value != scope.values.end() &&
       !existing_value->second.dependent_template_value &&
       !template_argument_semantics::type_depends_on_template_parameter(ctx, existing_value->second.type)) {
      continue;
    }

    DeducedValueMap::const_iterator found = deduced_values.find(parameter.name);
    if(found == deduced_values.end()) {
      continue;
    }

    TypePtr bound_value_type;
    if(!try_resolve_non_type_template_parameter_type(
           ctx, scope, parameter, bound_value_type)) {
      bound_value_type = parameter.value_type;
    }

    TemplateArgument argument;
    argument.kind = TemplateArgument::TA_VALUE;
    argument.type = bound_value_type;
    argument.value = found->second;
    argument.text = typed_non_type_template_argument_text(ctx, bound_value_type, found->second);
    ctx.bind_single_template_argument_into_scope(scope, parameter, argument);
  }
}

bool is_dependent_qualified_nondeduced_type_context(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & type)
{
  if(!type) {
    return false;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }

  switch(base->kind) {
  case Type::TK_CV:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
  case Type::TK_ARRAY:
    return is_dependent_qualified_nondeduced_type_context(
        ctx, parameters, base->inner);

  case Type::TK_MEMBER_POINTER:
    return is_dependent_qualified_nondeduced_type_context(
               ctx, parameters, base->owner) ||
           is_dependent_qualified_nondeduced_type_context(
               ctx, parameters, base->inner);

  case Type::TK_FUNCTION:
    if(is_dependent_qualified_nondeduced_type_context(
           ctx, parameters, base->inner)) {
      return true;
    }
    for(std::size_t i = 0; i < base->params.size(); ++i) {
      if(is_dependent_qualified_nondeduced_type_context(
             ctx, parameters, base->params[i])) {
        return true;
      }
    }
    return false;

  case Type::TK_NAMED:
  {
    const std::string text = strip_elaborated_type_prefix(
        trim_space(type_argument_text_for_deduction(ctx, base)));
    const std::size_t qualifier_end = text.rfind("::");
    if(qualifier_end == std::string::npos) {
      return false;
    }

    const std::string qualifier = text.substr(0, qualifier_end);
    for(std::size_t i = 0; i < parameters.size(); ++i) {
      const TemplateParameterInfo & parameter = parameters[i];
      if(parameter.name.empty()) {
        continue;
      }
      if(contains_identifier_token(qualifier, parameter.name) ||
         (!parameter.placeholder_key.empty() &&
          contains_identifier_token(qualifier, parameter.placeholder_key))) {
        return true;
      }
    }
    return false;
  }

  default:
    return false;
  }
}

// template-boundary-audit: begin text_recovery_bridge
bool recover_function_template_deduction_pattern_type(
    SemanticContext & ctx,
    Scope & scope,
    const TypePtr & pattern,
    TypePtr & resolved)
{
  return template_api::resolve_instantiated_dependent_type(
      ctx, scope, pattern, resolved);
}

// template-boundary-audit: end text_recovery_bridge

TypePtr resolve_function_template_deduction_pattern(SemanticContext & ctx,
                                                    Scope & scope,
                                                    const TypePtr & pattern)
{
  if(!pattern) {
    return pattern;
  }

  TypePtr resolved;
  if(recover_function_template_deduction_pattern_type(
         ctx, scope, pattern, resolved)) {
    return resolved;
  }
  return pattern;
}

TypePtr partially_resolve_function_template_deduction_pattern(SemanticContext & ctx,
                                                              Scope & scope,
                                                              const TypePtr & pattern)
{
  if(!pattern) {
    return pattern;
  }

  switch(pattern->kind) {
  case Type::TK_FUNDAMENTAL:
    return pattern;

  case Type::TK_NAMED:
  {
    TypePtr resolved;
    if(recover_function_template_deduction_pattern_type(
           ctx, scope, pattern, resolved) &&
       resolved) {
      return resolved;
    }
    return pattern;
  }

  case Type::TK_CV:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ?
               pattern :
               apply_cv(inner, pattern->cv_const, pattern->cv_volatile);
  }

  case Type::TK_ATOMIC:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ? pattern : make_atomic(inner);
  }

  case Type::TK_POINTER:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ? pattern : make_pointer(inner);
  }

  case Type::TK_BLOCK_POINTER:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ? pattern : make_block_pointer(inner);
  }

  case Type::TK_LVALUE_REFERENCE:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ? pattern : collapse_lvalue_reference_type(inner);
  }

  case Type::TK_RVALUE_REFERENCE:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ? pattern : collapse_rvalue_reference_type(inner);
  }

  case Type::TK_ARRAY:
  {
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(inner, pattern->inner) ?
               pattern :
               make_array(inner, pattern->has_bound, pattern->bound, pattern->bound_text);
  }

  case Type::TK_MEMBER_POINTER:
  {
    TypePtr owner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->owner);
    TypePtr inner = partially_resolve_function_template_deduction_pattern(ctx,
                                                                          scope,
                                                                          pattern->inner);
    return type_equals(owner, pattern->owner) && type_equals(inner, pattern->inner) ?
               pattern :
               make_member_pointer(owner, inner);
  }

  case Type::TK_FUNCTION:
  {
    bool changed = false;
    TypePtr result = partially_resolve_function_template_deduction_pattern(ctx,
                                                                           scope,
                                                                           pattern->inner);
    changed = !type_equals(result, pattern->inner);
    std::vector<TypePtr> params;
    params.reserve(pattern->params.size());
    for(std::size_t i = 0; i < pattern->params.size(); ++i) {
      TypePtr param = partially_resolve_function_template_deduction_pattern(ctx,
                                                                            scope,
                                                                            pattern->params[i]);
      changed = changed || !type_equals(param, pattern->params[i]);
      params.push_back(param);
    }
    return !changed ?
               pattern :
               make_function(result,
                             params,
                             pattern->variadic,
                             pattern->function_const,
                             pattern->function_volatile,
                             pattern->prototype_relaxed);
  }
  }

  return pattern;
}

TypePtr prepare_function_template_deduction_pattern(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    Scope & scope,
    const TypePtr & original_pattern)
{
  const bool mentions_function_template_parameter =
      type_mentions_function_template_parameter(ctx, parameters, original_pattern);
  TypePtr pattern = original_pattern;
  bool partially_rewritten = false;
  try {
    pattern = resolve_function_template_deduction_pattern(
        ctx, scope, original_pattern);
  } catch(const TemplateSubstitutionFailure &) {
    if(mentions_function_template_parameter) {
      return original_pattern;
    }
    throw;
  } catch(const std::logic_error & e) {
    throw TemplateSubstitutionFailure(
        std::string("invalid function template deduction pattern: ") + e.what());
  }
  if(type_equals(pattern, original_pattern) &&
     mentions_function_template_parameter &&
     template_argument_semantics::type_depends_on_template_parameter(ctx, original_pattern)) {
    pattern = partially_resolve_function_template_deduction_pattern(
        ctx,
        scope,
        original_pattern);
    partially_rewritten = !type_equals(pattern, original_pattern);
  }
  if(parser_trace::enabled("template.resolve") &&
     describe_type(pattern) != describe_type(original_pattern)) {
    std::ostringstream trace;
    trace << "deduction-pattern-concretized original=" << describe_type(original_pattern)
          << " resolved=" << describe_type(pattern);
    if(!parameters.empty()) {
      trace << " param-bindings=";
      for(std::size_t i = 0; i < parameters.size(); ++i) {
        if(i != 0) {
          trace << ";";
        }
        trace << parameters[i].name << "={" << template_parameter_binding_summary(scope,
                                                                                  parameters[i])
              << "}";
      }
    }
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(!partially_rewritten &&
     mentions_function_template_parameter &&
     !type_mentions_function_template_parameter(ctx, parameters, pattern)) {
    return original_pattern;
  }
  if(!partially_rewritten &&
     template_argument_semantics::type_depends_on_template_parameter(ctx, pattern) &&
     mentions_function_template_parameter) {
    return original_pattern;
  }
  return pattern;
}

void apply_function_template_call_deduction_adjustments(
    FunctionTemplateDecl & decl,
    const ExprInfo & arg,
    TypePtr & pattern,
    TypePtr & actual)
{
  TypePtr pattern_base = strip_top_level_cv(pattern);
  if(pattern_base &&
     pattern_base->kind == Type::TK_RVALUE_REFERENCE &&
     is_forwarding_reference_pattern(decl.parameters, pattern) &&
     arg.category == VC_LVALUE) {
    pattern = pattern_base->inner;
    actual = collapse_lvalue_reference_type(actual);
    return;
  }

  if(pattern_base &&
     (pattern_base->kind == Type::TK_LVALUE_REFERENCE ||
      pattern_base->kind == Type::TK_RVALUE_REFERENCE)) {
    pattern = pattern_base->inner;
    return;
  }

  pattern = normalize_parameter_type(pattern);
  actual = normalize_parameter_type(actual);
}

bool argument_is_braced_init_list_for_deduction(const ExprInfo & arg)
{
  return arg.node.kind == CallSemKind::braced_init_list;
}

bool deduction_pattern_accepts_braced_init_list_argument(SemanticContext & ctx,
                                                        const TypePtr & pattern)
{
  if(!pattern) {
    return false;
  }

  TypePtr base = strip_top_level_cv(remove_reference_type(strip_top_level_cv(pattern)));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return true;
  }
  return ctx.is_initializer_list_type(base, nullptr, nullptr);
}

void remove_shadowing_template_parameter_bindings(
    Scope & scope,
    const std::vector<TemplateParameterInfo> & parameters)
{
  template_scope::bind_template_parameter_placeholders(scope, parameters);
}

bool try_resolve_non_type_template_parameter_type(
    SemanticContext & ctx,
    Scope & scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return try_resolve_non_type_template_parameter_type(
            services, template_api::make_template_environment(scope), parameter, out);
      });
}

bool try_resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out)
{
  try {
    Scope & raw_scope = scope.require();
    template_api::TemplateTypeSystem & type_system = service_type_system(services);
    out = parameter.value_type;
    bool resolved = false;
    if(parameter.kind != TemplateParameterInfo::TP_NON_TYPE) {
      resolved = static_cast<bool>(out);
    } else {
      const bool has_structured_decl_specifier =
          parameter.non_type_decl_specifier_seq &&
          !parameter.non_type_declarator &&
          !parameter.non_type_abstract_declarator;
      if(has_structured_decl_specifier &&
         (!out ||
          template_argument_semantics::type_depends_on_template_parameter(
              type_system, out))) {
        TypePtr structured_type;
        if(template_decl_ast::parse_type_specifier_seq(
               services,
               raw_scope,
               raw_scope,
               *parameter.non_type_decl_specifier_seq,
               structured_type,
               true,
               true) &&
           structured_type) {
          out = structured_type;
          template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
              services, scope, out);
          resolved =
              static_cast<bool>(out) &&
              !template_argument_semantics::type_depends_on_template_parameter(type_system, out);
        }
        if(!resolved) {
          return false;
        }
      }
      if(!resolved) {
        template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
            services, scope, out);
      }
      if(out && !template_argument_semantics::type_depends_on_template_parameter(type_system, out)) {
        resolved = true;
      }
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "non-type-param-type name=" << parameter.name
            << " spelling=" << non_type_template_parameter_spelling(parameter)
            << " resolved=" << (resolved ? "yes" : "no")
            << " type=" << (out ? describe_type(out) : std::string("<null>"));
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return resolved;
  } catch(const TemplateSubstitutionFailure & e) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "non-type-param-type name=" << parameter.name
            << " spelling=" << non_type_template_parameter_spelling(parameter)
            << " resolved=no"
            << " type=<substitution-failure>";
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return false;
  }
}

bool can_skip_resolved_non_dependent_pattern_check(
    SemanticContext & ctx,
    Scope & scope,
    const FunctionTemplateDecl & decl,
    std::size_t param_index,
    const TypePtr & original_pattern,
    const TypePtr & resolved_pattern)
{
  if(!resolved_pattern ||
     !decl.declarator) {
    return false;
  }

  if(type_mentions_function_template_parameter(ctx, decl.parameters, original_pattern)) {
    if(type_mentions_function_template_parameter(ctx, decl.parameters, resolved_pattern) ||
       type_mentions_unbound_function_template_parameter(
           ctx, decl.parameters, scope, original_pattern)) {
      return false;
    }

    if(is_dependent_qualified_nondeduced_type_context(
           ctx, decl.parameters, original_pattern)) {
      return true;
    }

    TypePtr original_base = strip_top_level_cv(original_pattern);
    if(original_base && original_base->kind == Type::TK_NAMED) {
      QualifiedName parsed_name;
      const std::string normalized_text = strip_elaborated_type_prefix(
          trim_space(type_argument_text_for_deduction(ctx, original_base)));
      if(template_id_head_name_from_type_text(normalized_text, parsed_name)) {
        AliasTemplateDecl * alias_decl =
            ((!parsed_name.rooted && parsed_name.qualifiers.empty()) ?
                 semantic_lookup::lookup_unqualified_alias_template(
                     scope, parsed_name.name) :
                 nullptr);
        if(!alias_decl) {
          alias_decl = ctx.lookup_alias_template(scope, parsed_name);
        }
        if(alias_decl) {
          if(alias_decl->resolved_type_pattern &&
             is_dependent_qualified_nondeduced_type_context(
                 ctx, alias_decl->parameters, alias_decl->resolved_type_pattern)) {
            return true;
          }
          return true;
        }
      }
    }

    return false;
  }

  if(type_mentions_function_template_parameter(ctx, decl.parameters, resolved_pattern)) {
    return false;
  }

  const CppAstNode * parameter_clause =
      find_child(*decl.declarator, CppAstKind::parameter_clause);
  if(!parameter_clause || param_index >= parameter_clause->children.size()) {
    return false;
  }

  const CppAstNode & parameter = parameter_clause->children[param_index];
  const CppAstNode * specifiers =
      find_child(parameter, CppAstKind::decl_specifier_seq);
  if(!specifiers) {
    return false;
  }

  bool mentions_template_parameter = false;
  for(std::size_t i = 0; i < specifiers->children.size(); ++i) {
    const std::string & text = specifiers->children[i].value;
    for(std::size_t j = 0; j < decl.parameters.size(); ++j) {
      const std::string & parameter_name = decl.parameters[j].name;
      if(parameter_name.empty()) {
        continue;
      }
      if(contains_identifier_token(text, parameter_name) &&
         !non_type_template_parameter_has_non_dependent_binding(
             ctx, scope, decl.parameters[j])) {
        mentions_template_parameter = true;
        break;
      }
    }
  }

  return !mentions_template_parameter;
}

}  // namespace

void dump_template_resolution_cache_memory_census(std::ostream & out)
{
  dump_template_resolution_cache_memory_census_impl(out);
}

bool resolve_non_type_template_parameter_type(SemanticContext & ctx,
                                              Scope & scope,
                                              const TemplateParameterInfo & parameter,
                                              TypePtr & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return try_resolve_non_type_template_parameter_type(
            services, template_api::make_template_environment(scope), parameter, out);
      });
}

bool resolve_non_type_template_parameter_type(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const TemplateParameterInfo & parameter,
    TypePtr & out)
{
  return try_resolve_non_type_template_parameter_type(services, scope, parameter, out);
}

bool type_has_dependent_template_owner(const TypePtr & type)
{
  TypePtr owner;
  std::vector<std::string> members;
  bool leading_typename = false;
  if(!named_type_dependent_qualified_member(type, owner, members, leading_typename)) {
    return false;
  }
  if(leading_typename && owner) {
    return true;
  }

  void * class_template_decl = nullptr;
  std::vector<DependentAliasTemplateArgumentSyntax> arguments;
  return named_type_dependent_class_template(owner, class_template_decl, arguments) &&
         class_template_decl != nullptr &&
         !arguments.empty();
}

bool resolve_instantiated_dependent_template_owner_type_argument(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle argument_scope,
    const TemplateArgumentSyntax * syntax,
    TypePtr & type)
{
  if(!argument_scope.valid() ||
     !syntax ||
     !type ||
     !type_has_dependent_template_owner(type)) {
    return false;
  }

  TypePtr scoped_type;
  if(!template_argument_semantics::resolve_type_argument_input(
         services, argument_scope, syntax, true, scoped_type) ||
     !scoped_type) {
    return false;
  }
  template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
      services,
      argument_scope,
      scoped_type);
  if(!scoped_type ||
     template_argument_semantics::type_depends_on_template_parameter(
         service_type_system(services),
         scoped_type)) {
    return false;
  }

  type = scoped_type;
  return true;
}

bool resolve_template_argument(SemanticContext & ctx,
                               Scope & argument_scope,
                               Scope & parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               const TemplateArgumentSyntax * syntax,
                               TemplateArgument & out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_api::resolve_template_argument(
            services, argument_scope, parameter_scope, parameter, text, syntax, out);
      });
}

bool resolve_template_argument(SemanticContext & ctx,
                               Scope & argument_scope,
                               Scope & parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               TemplateArgument & out)
{
  return resolve_template_argument(
      ctx, argument_scope, parameter_scope, parameter, text, nullptr, out);
}

bool binding_owner_template_has_parameter(
    const ValueBinding * binding,
    const TemplateParameterInfo & parameter)
{
  if(!binding ||
     !binding->owner_class ||
     !binding->owner_class->source_template ||
     parameter.name.empty()) {
    return false;
  }
  const std::vector<TemplateParameterInfo> & owner_parameters =
      binding->owner_class->source_template->parameters;
  for(std::size_t i = 0; i < owner_parameters.size(); ++i) {
    if(owner_parameters[i].kind == TemplateParameterInfo::TP_NON_TYPE &&
       owner_parameters[i].name == parameter.name) {
      return true;
    }
  }
  return false;
}

void note_qualified_member_type_non_type_binding_if_needed(
    template_api::TemplateServices & services,
    const TemplateParameterInfo & parameter,
    const ValueBinding * binding)
{
  if(!services.semantic_context ||
     !template_api::template_witness_qualified_member_type_lookup_active() ||
     !binding_owner_template_has_parameter(binding, parameter)) {
    return;
  }
  template_api::note_template_member_value_instantiation_if_needed(
      *services.semantic_context,
      *binding);
}

bool resolve_template_argument(template_api::TemplateServices & services,
                               template_api::TemplateEnvironmentHandle argument_scope,
                               template_api::TemplateEnvironmentHandle parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               const TemplateArgumentSyntax * syntax,
                               TemplateArgument & out)
{
  Scope & raw_argument_scope = argument_scope.require();
  Scope & raw_parameter_scope = parameter_scope.require();
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const std::string syntax_source_location =
      source_location_for_template_argument_syntax(services.witness_context,
                                                  syntax);
  const ScopedTemplateArgumentUseLocation syntax_argument_use_location(
      syntax_source_location);
  std::vector<std::string> syntax_template_id_arg_texts;
  const TemplateIdSyntax * syntax_template_id =
      syntax && syntax->template_id ? syntax->template_id.get() : nullptr;
  if(syntax_template_id && !syntax_source_location.empty()) {
    syntax_template_id_arg_texts =
        source_argument_texts_for_template_id_syntax(*syntax_template_id);
  }
  const template_api::ScopedTemplateIdSourceArguments
      syntax_template_id_source_arguments(
          syntax_source_location,
          syntax_template_id ? syntax_template_id->name.name : std::string(),
          std::move(syntax_template_id_arg_texts));
  out = TemplateArgument();
  const std::string trimmed = trim_space(text);
  std::string rewritten = trimmed;
  out.text = rewritten;
  attach_template_argument_source_syntax(syntax, out);

  if(parameter.kind == TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
    const bool ok = template_api::resolve_template_template_argument_text(
        services,
        argument_scope,
        trimmed,
        parameter.template_parameter_count,
        true,
        out);
    attach_template_argument_source_syntax(syntax, out);
    return ok;
  }

  if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
    long long value = 0;
    TypePtr bound_value_type;
    if(!try_resolve_non_type_template_parameter_type(
           services, parameter_scope, parameter, bound_value_type)) {
      if(!non_type_template_parameter_is_still_dependent(
             services, parameter_scope, parameter)) {
        return false;
      }
      bound_value_type = parameter.value_type;
    }

    const auto try_resolve_named_non_type =
        [&](const std::string & candidate_text) -> bool
    {
      const ValueBinding * named_binding = nullptr;
      if(!try_resolve_named_non_type_template_argument(
             services,
             raw_argument_scope,
             candidate_text,
             bound_value_type,
             out,
             &named_binding)) {
        return false;
      }
      note_qualified_member_type_non_type_binding_if_needed(
          services,
          parameter,
          named_binding);
      if(services.semantic_context && named_binding) {
        template_api::note_template_member_value_instantiation_if_needed(
            *services.semantic_context,
            *named_binding);
      }
      return true;
    };

    if(trimmed != rewritten &&
       try_resolve_named_non_type(trimmed)) {
      return true;
    }
    if(try_resolve_named_non_type(rewritten)) {
      return true;
    }
    if(try_resolve_function_non_type_template_argument_syntax(
           services, raw_argument_scope, syntax, bound_value_type, out)) {
      return true;
    }

    const bool mentions_placeholders =
        template_argument_semantics::text_mentions_template_placeholders(
            services, argument_scope, rewritten);
    const bool mentions_dependent_bindings =
        template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
            services, argument_scope, rewritten);
    const bool carried_dependent_expression_unresolved =
        template_argument_syntax_can_retain_carried_non_type_dependency(
            services, raw_argument_scope, syntax) &&
        (mentions_placeholders || mentions_dependent_bindings) &&
        syntax->expression &&
        carried_dependent_expression_has_unresolved_lookup(
            services, raw_argument_scope, *syntax->expression);

    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "resolve-template-arg kind=non-type text=" << trimmed
            << " rewritten=" << rewritten
            << " syntax=" << (syntax ? "yes" : "no")
            << " syntax-expr="
            << ((syntax && syntax->expression) ? "yes" : "no")
            << " syntax-type="
            << ((syntax && syntax->type_id) ? "yes" : "no")
            << " syntax-pack="
            << ((syntax && syntax->pack_expansion) ? "yes" : "no")
            << " mentions-placeholders=" << (mentions_placeholders ? "yes" : "no")
            << " mentions-dependent-bindings="
            << (mentions_dependent_bindings ? "yes" : "no")
            << " carried-unresolved="
            << (carried_dependent_expression_unresolved ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }

    const bool syntax_dependency_still_visible =
        syntax &&
        syntax->dependent &&
        (!syntax->expression ||
         mentions_placeholders ||
         mentions_dependent_bindings);
    const bool can_try_carried_dependent_evaluation =
        carried_dependent_expression_unresolved &&
        !mentions_placeholders &&
        !mentions_dependent_bindings &&
        !syntax_dependency_still_visible;
    bool allow_dependent_non_type_argument =
        mentions_placeholders ||
        mentions_dependent_bindings ||
        syntax_dependency_still_visible ||
        carried_dependent_expression_unresolved;
    if(!allow_dependent_non_type_argument &&
       is_identifier_text(rewritten) &&
       template_argument_semantics::scope_has_template_placeholders(
           services, template_api::make_template_environment(raw_argument_scope))) {
      const ValueBinding * named_value =
          lookup_unqualified_value(services, raw_argument_scope, rewritten);
      if(named_value &&
         !named_value->has_constant_value &&
         !named_value->has_constexpr_value) {
        allow_dependent_non_type_argument = true;
      }
    }
    std::string selected_text = rewritten;
    std::string eval_error;
    template_api::NonTypeArgumentStatus value_status =
        template_api::NT_ARG_PARSE_FAILED;
    std::string original_eval_error;
    long long original_value = 0;
    if(allow_dependent_non_type_argument) {
      if(try_evaluate_sizeof_pack_non_type_argument(
             services,
             argument_scope,
             syntax,
             bound_value_type,
             value,
             eval_error,
             value_status)) {
        selected_text = rewritten.empty() ? trimmed : rewritten;
      } else {
        const bool can_evaluate_structured_syntax =
            can_try_carried_dependent_evaluation &&
            syntax &&
            (syntax->expression || syntax->type_id || syntax->template_id);
        if(can_evaluate_structured_syntax) {
          const template_api::ScopedTemplateWitnessSourceCapturePause
              source_capture_pause;
          value_status = static_cast<template_api::NonTypeArgumentStatus>(
              template_argument_semantics::evaluate_non_type_argument_syntax(
                  services,
                  argument_scope,
                  *syntax,
                  value,
                  &eval_error,
                  bound_value_type));
        }
        if(value_status == template_api::NT_ARG_EVALUATED) {
          selected_text = rewritten.empty() ? trimmed : rewritten;
        } else {
          value_status = template_api::NT_ARG_DEPENDENT;
          selected_text = rewritten.empty() ? trimmed : rewritten;
        }
      }
    } else if(syntax && syntax->pack_expansion) {
      value_status = template_api::NT_ARG_DEPENDENT;
      selected_text = rewritten.empty() ? trimmed : rewritten;
    } else {
      const bool can_evaluate_original_syntax =
          syntax &&
          (syntax->expression || syntax->type_id || syntax->template_id);
      template_api::NonTypeArgumentStatus original_status =
          template_api::NT_ARG_PARSE_FAILED;
      if(can_evaluate_original_syntax) {
        original_status = static_cast<template_api::NonTypeArgumentStatus>(
            template_argument_semantics::evaluate_non_type_argument_syntax(
                services,
                argument_scope,
                *syntax,
                original_value,
                &original_eval_error,
                bound_value_type));
      } else {
        original_status =
            template_api::evaluate_non_type_argument_text(
                services,
                argument_scope,
                trimmed,
                original_value,
                &original_eval_error,
                bound_value_type);
      }
      if(can_evaluate_original_syntax &&
         original_status == template_api::NT_ARG_PARSE_FAILED) {
        if(syntax->expression || syntax->template_id || !syntax->type_id) {
          long long original_text_value = 0;
          std::string original_text_eval_error;
          const template_api::NonTypeArgumentStatus original_text_status =
              template_api::evaluate_non_type_argument_text(
                  services,
                  argument_scope,
                  trimmed,
                  original_text_value,
                  &original_text_eval_error,
                  bound_value_type);
          if(original_text_status == template_api::NT_ARG_EVALUATED ||
             original_text_status == template_api::NT_ARG_DEPENDENT) {
            original_status = original_text_status;
            original_value = original_text_value;
            original_eval_error = original_text_eval_error;
          }
        }
      }
      if(original_status == template_api::NT_ARG_EVALUATED ||
         original_status == template_api::NT_ARG_DEPENDENT ||
         rewritten == trimmed) {
        value_status = original_status;
        value = original_value;
        eval_error = original_eval_error;
        selected_text =
            original_status == template_api::NT_ARG_DEPENDENT &&
                    rewritten != trimmed ?
                rewritten :
                trimmed;
      } else {
        long long rewritten_value = 0;
        std::string rewritten_eval_error;
        const template_api::NonTypeArgumentStatus rewritten_status =
            template_api::evaluate_non_type_argument_text(
                services,
                argument_scope,
                rewritten,
                rewritten_value,
                &rewritten_eval_error,
                bound_value_type);
        if(rewritten_status == template_api::NT_ARG_EVALUATED ||
           rewritten_status == template_api::NT_ARG_DEPENDENT) {
          value_status = rewritten_status;
          value = rewritten_value;
          eval_error = rewritten_eval_error;
          selected_text = rewritten;
        } else {
          value_status = original_status;
          value = original_value;
          eval_error = original_eval_error;
          selected_text = trimmed;
        }
      }
    }
    if(value_status != template_api::NT_ARG_EVALUATED &&
       value_status != template_api::NT_ARG_DEPENDENT &&
       carried_dependent_expression_unresolved) {
      value_status = template_api::NT_ARG_DEPENDENT;
      selected_text = rewritten.empty() ? trimmed : rewritten;
      allow_dependent_non_type_argument = true;
      eval_error.clear();
      if(parser_trace::enabled("template.resolve")) {
        parser_trace::note(
            "template.resolve",
            std::string(),
            "non-type-arg carried dependent syntax remains unresolved structurally");
      }
    }
    if(value_status != template_api::NT_ARG_EVALUATED &&
       simple_non_type_argument_names_type_placeholder(raw_argument_scope,
                                                       selected_text)) {
      std::ostringstream detail;
      detail << "template argument for non-type template parameter must be an expression: "
             << selected_text;
      if(!parameter.name.empty()) {
        detail << " [parameter " << parameter.name << "]";
      }
      throw_substitution_failure(detail.str(), std::string(), "template-resolution");
    }
    if(value_status == template_api::NT_ARG_PARSE_FAILED) {
      return false;
    }
    if(value_status == template_api::NT_ARG_DEPENDENT &&
       !allow_dependent_non_type_argument) {
      return false;
    }
    if(value_status == template_api::NT_ARG_DEPENDENT) {
      out.kind = TemplateArgument::TA_VALUE;
      out.type = bound_value_type;
      out.text = selected_text;
      attach_dependent_non_type_argument_expression(
          syntax, selected_text, trimmed, out);
      out.dependent = true;
      if(syntax && services.witness_context.session != nullptr) {
        template_argument_semantics::
            append_structured_bool_value_dependencies_in_template_argument_syntax(
                services,
                argument_scope,
                *syntax,
                out.value_dependencies);
        template_argument_semantics::
            append_non_bool_static_value_dependencies_in_template_argument_syntax(
                services,
                argument_scope,
                *syntax,
                bound_value_type,
                out.value_dependencies);
        template_argument_semantics::note_template_value_dependencies_for_witness(
            *services.semantic_context,
            out.value_dependencies);
        template_argument_semantics::
            note_structured_bool_value_members_in_template_argument_syntax(
                services,
                argument_scope,
                *syntax);
      }
      return true;
    }
    if(value_status != template_api::NT_ARG_EVALUATED &&
       try_resolve_named_non_type(selected_text)) {
      return true;
    }
    if(value_status != template_api::NT_ARG_EVALUATED) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "non-type-arg-state text=" << trimmed
              << " mentions-placeholders=" << (mentions_placeholders ? "yes" : "no")
              << " mentions-dependent-bindings="
              << (mentions_dependent_bindings ? "yes" : "no")
              << " allow-dependent=" << (allow_dependent_non_type_argument ? "yes" : "no")
              << " parameter-type="
              << (bound_value_type ? describe_type(bound_value_type) : std::string("<null>"));
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      if(!allow_dependent_non_type_argument) {
        std::ostringstream detail;
        detail << "failed non-type template argument evaluation: " << selected_text;
        detail << " [scope " << scope_name_for_diagnostic(raw_argument_scope) << "]";
        detail << " [bindings " << scope_bindings_for_diagnostic(raw_argument_scope) << "]";
        detail << " [parameter-scope " << scope_name_for_diagnostic(raw_parameter_scope) << "]";
        detail << " [parameter-bindings "
               << scope_bindings_for_diagnostic(raw_parameter_scope) << "]";
        if(!eval_error.empty()) {
          detail << " [eval " << eval_error << "]";
        }
        throw_substitution_failure(detail.str(), std::string(), "template-resolution");
      }
      out.kind = TemplateArgument::TA_VALUE;
      out.type = bound_value_type;
      out.text = selected_text;
      attach_dependent_non_type_argument_expression(
          syntax, selected_text, trimmed, out);
      out.dependent = true;
      return true;
    }

    out.kind = TemplateArgument::TA_VALUE;
    out.type = bound_value_type;
    out.value = value;
    out.text = typed_non_type_template_argument_text(
        type_system, bound_value_type, value);
    if(syntax && services.witness_context.session != nullptr) {
      template_argument_semantics::
          append_structured_bool_value_dependencies_in_template_argument_syntax(
              services,
              argument_scope,
              *syntax,
              out.value_dependencies);
      template_argument_semantics::
          append_non_bool_static_value_dependencies_in_template_argument_syntax(
              services,
              argument_scope,
              *syntax,
              bound_value_type,
              out.value_dependencies);
      template_argument_semantics::note_template_value_dependencies_for_witness(
          *services.semantic_context,
          out.value_dependencies);
      template_argument_semantics::
          note_structured_bool_value_members_in_template_argument_syntax(
              services,
              argument_scope,
              *syntax);
    }
    return true;
  }

  std::string type_rewritten = trimmed;
  TypePtr type;
  bool type_dependency_flags_computed = false;
  bool type_mentions_placeholders = false;
  bool type_mentions_dependent_bindings = false;
  const bool has_structured_type_syntax =
      syntax && (syntax->resolved_type || syntax->type_id || syntax->template_id);
  const auto compute_type_dependency_flags =
      [&]() -> void
  {
    if(type_dependency_flags_computed) {
      return;
    }
    type_mentions_placeholders =
        template_argument_semantics::text_mentions_template_placeholders(
            services, argument_scope, trimmed);
    type_mentions_dependent_bindings =
        template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
            services, argument_scope, trimmed);
    type_dependency_flags_computed = true;
  };
  const auto should_defer_dependent_type_resolution =
      [&](const TypePtr & candidate) -> bool
  {
    if(!candidate ||
       !template_argument_semantics::type_depends_on_template_parameter(
           type_system, candidate)) {
      return false;
    }
    compute_type_dependency_flags();
    return type_mentions_placeholders ||
           type_mentions_dependent_bindings ||
           should_defer_unresolved_type_lookup(
               services,
               template_api::make_template_environment(raw_argument_scope),
               trimmed);
  };
  const auto resolve_type_argument_if_needed =
      [&](TypePtr & candidate) -> void
  {
    if(should_defer_dependent_type_resolution(candidate)) {
      return;
    }
    template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
        services, argument_scope, candidate);
  };
  bool attempted_structured_type_syntax = false;
  bool bound_member_type_failure = false;
  if(syntax && syntax->resolved_type) {
    type = syntax->resolved_type;
    resolve_type_argument_if_needed(type);
  }
  if(!type &&
     try_resolve_bound_member_type_argument(type_system,
                                            raw_argument_scope,
                                            raw_parameter_scope,
                                            syntax,
                                            type,
                                            bound_member_type_failure,
                                            services.counters)) {
    if(bound_member_type_failure) {
      return false;
    }
  }
  if(!type && syntax && syntax->type_id) {
    attempted_structured_type_syntax = true;
    try {
      template_argument_semantics::parse_type_id_node_for_templates(
          services, raw_argument_scope, *syntax->type_id, type, true);
    } catch(const semantic_fallback_audit::SemanticFallbackError &) {
      if(!type_argument_can_remain_dependent_after_structured_failure(
             syntax, trimmed)) {
        throw;
      }
      type = make_deferred_dependent_type_argument(trimmed);
    }
    resolve_type_argument_if_needed(type);
  }
  if(!type && syntax && syntax->template_id) {
    attempted_structured_type_syntax = true;
    template_argument_semantics::resolve_template_id_syntax_type(
        services,
        raw_argument_scope,
        *syntax->template_id,
        true,
        syntax_source_location,
        type);
    resolve_type_argument_if_needed(type);
  }
  if(!type && syntax && syntax->expression) {
    attempted_structured_type_syntax = true;
    template_argument_semantics::resolve_type_argument_expression_syntax(
        services,
        raw_argument_scope,
        *syntax->expression,
        true,
        syntax_source_location,
        type);
    resolve_type_argument_if_needed(type);
  }
  if(!type && !has_structured_type_syntax) {
    resolve_non_dependent_direct_type_argument(
        services, argument_scope, trimmed, type);
  }
  if(!type && !has_structured_type_syntax) {
    TypePtr direct_bound_type;
    if(lookup_direct_bound_type_argument(raw_argument_scope,
                                         trimmed,
                                         direct_bound_type) &&
       direct_bound_type) {
      resolve_type_argument_if_needed(direct_bound_type);
      type = direct_bound_type;
    }
  }
  if(!type && !has_structured_type_syntax) {
    TypePtr rewritten_bound_type;
    if(lookup_rewritten_bound_type_argument(raw_argument_scope,
                                            trimmed,
                                            rewritten_bound_type) &&
       rewritten_bound_type) {
      resolve_type_argument_if_needed(rewritten_bound_type);
      type = rewritten_bound_type;
    }
  }
  if(!type && !has_structured_type_syntax) {
    TypePtr exact_visible_type;
    if(lookup_exact_visible_type_argument_text(raw_argument_scope,
                                               trimmed,
                                               exact_visible_type) &&
       exact_visible_type) {
      resolve_type_argument_if_needed(exact_visible_type);
      type = exact_visible_type;
    }
  }
  if(!type && !has_structured_type_syntax) {
    TypePtr parsed_type_text;
    if(template_argument_semantics::parse_type_argument_text(
           services,
           argument_scope,
           trimmed,
           parsed_type_text) &&
       parsed_type_text) {
      template_argument_semantics::resolve_instantiated_dependent_type_if_needed(
          services, argument_scope, parsed_type_text);
      type = parsed_type_text;
    }
  }
  if(!type && attempted_structured_type_syntax) {
    return false;
  }
  if(!type) {
    TypePtr dependent_member_type;
    if(try_resolve_dependent_qualified_member_type_argument(services,
                                                            type_system,
                                                            raw_argument_scope,
                                                            trimmed,
                                                            dependent_member_type) &&
       dependent_member_type) {
      type = dependent_member_type;
    }
  }
  if(!type &&
     !has_structured_type_syntax &&
     !has_invalid_top_level_qualified_owner_syntax(trimmed)) {
    TypePtr recovered_template_id;
    if(resolve_recoverable_bound_template_id_type(
           services,
           argument_scope,
           trimmed,
           recovered_template_id) &&
       recovered_template_id) {
      resolve_type_argument_if_needed(recovered_template_id);
      type = recovered_template_id;
    }
  }
  if(!type) {
    compute_type_dependency_flags();
  }

  if(!type &&
     type_argument_can_remain_dependent_after_structured_failure(syntax, trimmed)) {
    type = make_deferred_dependent_type_argument(trimmed);
  }

  if(!type &&
     !is_identifier_text(trimmed) &&
     !type_mentions_placeholders &&
     !type_mentions_dependent_bindings &&
     !should_defer_unresolved_type_lookup(
         services, template_api::make_template_environment(raw_argument_scope), trimmed)) {
    semantic_fallback_audit::hard_fail(
        "explicit-type-argument-text-fallback",
        std::string(),
        "explicit type template argument resolution reached text fallback"
        " [arg " + trimmed + "]"
        " [scope " + scope_name_for_diagnostic(raw_argument_scope) + "]"
        " [bindings " + scope_bindings_for_diagnostic(raw_argument_scope) + "]");
  }

  resolve_type_argument_if_needed(type);
  if(type &&
     template_argument_semantics::type_depends_on_template_parameter(type_system, type)) {
    resolve_instantiated_dependent_template_owner_type_argument(
        services, argument_scope, syntax, type);
  }

  if(type &&
     template_argument_semantics::type_depends_on_template_parameter(type_system, type) &&
     !template_argument_semantics::scope_has_template_placeholders(
         services, template_api::make_template_environment(raw_argument_scope))) {
    compute_type_dependency_flags();
    if(!type_mentions_placeholders &&
       !type_mentions_dependent_bindings &&
       !should_defer_unresolved_type_lookup(
           services, template_api::make_template_environment(raw_argument_scope), trimmed)) {
      if(raw_argument_scope.class_info && raw_argument_scope.class_info->source_template) {
        out.kind = TemplateArgument::TA_TYPE;
        out.type = type;
        out.text = trimmed;
        out.dependent = true;
        return true;
      }
      if(type_has_dependent_template_owner(type)) {
        out.kind = TemplateArgument::TA_TYPE;
        out.type = type;
        out.text = trimmed;
        out.dependent = true;
        return true;
      }
      std::ostringstream detail;
      detail << "explicit type template argument remained dependent: " << trimmed;
      detail << " [rewritten " << type_rewritten << "]";
      detail << " [scope " << scope_name_for_diagnostic(raw_argument_scope) << "]";
      detail << " [bindings " << scope_bindings_for_diagnostic(raw_argument_scope) << "]";
      detail << " [type " << describe_type(type) << "]";
      throw_substitution_failure(detail.str(), std::string(), "template-resolution");
    }
  }

  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "resolve-template-arg kind=type text=" << trimmed
          << " rewritten=" << type_rewritten
          << " result=" << (type ? describe_type(type) : std::string("<null>"))
          << " dependent="
          << ((type && template_argument_semantics::type_depends_on_template_parameter(type_system, type)) ? "yes" :
                                                                                         "no");
    parser_trace::note("template.resolve", std::string(), trace.str());
  }

  if(!type) {
    compute_type_dependency_flags();
    const bool should_defer =
        type_mentions_placeholders ||
        type_mentions_dependent_bindings ||
        should_defer_unresolved_type_lookup(
            services, template_api::make_template_environment(raw_argument_scope), trimmed);
    if(!should_defer) {
      std::ostringstream detail;
      detail << "failed type template argument resolution: " << trimmed;
      detail << " [scope " << scope_name_for_diagnostic(raw_argument_scope) << "]";
      detail << " [bindings " << scope_bindings_for_diagnostic(raw_argument_scope) << "]";
      throw_substitution_failure(detail.str(), std::string(), "template-resolution");
    }
    out.kind = TemplateArgument::TA_TYPE;
    out.type = make_named(trimmed,
                          "dependent type " + trimmed,
                          true);
    out.text = trimmed;
    out.dependent = true;
    return true;
  }

  set_resolved_type_template_argument(type_system, type, trimmed, out);
  if(syntax && services.witness_context.session != nullptr) {
    template_argument_semantics::
        append_structured_bool_value_dependencies_in_template_argument_syntax(
            services,
            argument_scope,
            *syntax,
            out.value_dependencies);
    template_argument_semantics::note_template_value_dependencies_for_witness(
        *services.semantic_context,
        out.value_dependencies);
  }
  return true;
}

bool resolve_template_argument(template_api::TemplateServices & services,
                               template_api::TemplateEnvironmentHandle argument_scope,
                               template_api::TemplateEnvironmentHandle parameter_scope,
                               const TemplateParameterInfo & parameter,
                               const std::string & text,
                               TemplateArgument & out)
{
  return template_resolution::resolve_template_argument(
      services, argument_scope, parameter_scope, parameter, text, nullptr, out);
}

bool resolve_template_arguments(SemanticContext & ctx,
                                Scope & scope,
                                const std::vector<TemplateParameterInfo> & parameters,
                                const std::vector<std::string> & texts,
                                const std::vector<TemplateArgumentSyntax> * syntaxes,
                                std::vector<TemplateArgument> & out,
                                Scope * default_argument_declaring_scope)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_api::TemplateEnvironmentHandle default_scope;
        if(default_argument_declaring_scope) {
          default_scope =
              template_api::make_template_environment(*default_argument_declaring_scope);
        }
        return template_api::resolve_template_arguments(
            services,
            template_api::make_template_environment(scope),
            parameters,
            texts,
            syntaxes,
            out,
            default_scope);
      });
}

bool resolve_template_arguments(SemanticContext & ctx,
                                Scope & scope,
                                const std::vector<TemplateParameterInfo> & parameters,
                                const std::vector<std::string> & texts,
                                std::vector<TemplateArgument> & out,
                                Scope * default_argument_declaring_scope)
{
  return resolve_template_arguments(ctx,
                                    scope,
                                    parameters,
                                    texts,
                                    nullptr,
                                    out,
                                    default_argument_declaring_scope);
}

bool resolve_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    const std::vector<TemplateArgumentSyntax> * syntaxes,
    std::vector<TemplateArgument> & out,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope)
{
  semantic_metrics::AnalyzerCounters * counters = services.counters;
  template_api::TemplateTypeSystem & type_system = service_type_system(services);
  const auto type_is_dependent =
      [&type_system](const TypePtr & type) -> bool
      {
        return template_argument_semantics::type_depends_on_template_parameter(
            type_system,
            type);
      };
  const auto type_argument_text =
      [&type_system](const TypePtr & type) -> std::string
      {
        return deduction_lookup_type_text(type_system, type);
      };
  if(counters) {
    ++counters->resolve_template_argument_calls;
  }
  Scope & raw_scope = scope.require();
  out.clear();
  if(try_resolve_pre_expansion_known_failure(services,
                                             type_system,
                                             scope,
                                             parameters,
                                             texts,
                                             syntaxes)) {
    if(semantic_hotspot::enabled()) {
      std::ostringstream query;
      query << "params=" << parameters.size()
            << " texts=" << texts.size()
            << " [" << join_template_texts(texts) << "]";
      semantic_hotspot::note_semantic_query(
          "resolve_template_arguments_pre_expansion_trait_bound_member_failure",
          query.str());
    }
    return false;
  }
  const PreExpansionResolveStatus pre_expansion_status =
      try_resolve_pre_expansion_simple_type_arguments(services,
                                                      type_system,
                                                      scope,
                                                      parameters,
                                                      texts,
                                                      syntaxes,
                                                      out);
  if(pre_expansion_status == PERTA_SUCCESS) {
    return true;
  }
  if(pre_expansion_status == PERTA_FAILURE) {
    if(semantic_hotspot::enabled()) {
      std::ostringstream query;
      query << "params=" << parameters.size()
            << " texts=" << texts.size()
            << " [" << join_template_texts(texts) << "]";
      semantic_hotspot::note_semantic_query(
          "resolve_template_arguments_pre_expansion_bound_member_failure",
          query.str());
    }
    return false;
  }
  template_argument_semantics::ExpandedTemplateArgumentInputs resolution_inputs =
      template_argument_semantics::expand_template_argument_inputs(
          services, raw_scope, texts, syntaxes);
  if(counters) {
    counters->resolve_template_argument_expanded_texts += resolution_inputs.texts.size();
  }
  const auto hotspot_query_text_for =
      [&](const std::vector<std::string> & query_texts) -> std::string
  {
    std::ostringstream query;
    query << "params=" << parameters.size()
          << " texts=" << query_texts.size()
          << " [" << join_template_texts(query_texts) << "]";
    return query.str();
  };
  const auto note_template_argument_key_texts =
      [&]() -> void
  {
    if(!counters) {
      return;
    }
    counters->resolve_template_argument_key_text_count += resolution_inputs.texts.size();
    for(std::size_t i = 0; i < resolution_inputs.texts.size(); ++i) {
      counters->resolve_template_argument_key_text_chars += resolution_inputs.texts[i].size();
    }
  };
  const auto note_expensive_simple_fast_status =
      [&](FastResolveTemplateArgumentsStatus status) -> void
  {
    if(!counters) {
      return;
    }
    switch(status) {
    case FRTA_SUCCESS:
      ++counters->resolve_template_argument_simple_fast_expensive_successes;
      break;
    case FRTA_FAILURE:
      ++counters->resolve_template_argument_simple_fast_expensive_failures;
      break;
    case FRTA_UNSUPPORTED:
      ++counters->resolve_template_argument_simple_fast_expensive_unsupported;
      break;
    }
  };
  if(semantic_hotspot::enabled()) {
    semantic_hotspot::note_semantic_query("resolve_template_arguments",
                                          hotspot_query_text_for(
                                              resolution_inputs.texts));
  }
  if(try_resolve_single_bound_type_argument_fast(services,
                                                 type_system,
                                                 scope,
                                                 parameters,
                                                 resolution_inputs,
                                                 out)) {
    if(counters) {
      ++counters->resolve_template_argument_single_bound_type_fast_hits;
    }
    return true;
  }
  const FastResolveTemplateArgumentsStatus simple_template_fast_status =
      try_resolve_simple_template_arguments_fast(services,
                                                 type_system,
                                                 scope,
                                                 parameters,
                                                 resolution_inputs,
                                                 out,
                                                 false);
  if(counters) {
    switch(simple_template_fast_status) {
    case FRTA_SUCCESS:
      ++counters->resolve_template_argument_simple_fast_successes;
      break;
    case FRTA_FAILURE:
      ++counters->resolve_template_argument_simple_fast_failures;
      break;
    case FRTA_UNSUPPORTED:
      ++counters->resolve_template_argument_simple_fast_unsupported;
      break;
    }
  }
  if(simple_template_fast_status == FRTA_SUCCESS) {
    return true;
  }
  if(simple_template_fast_status == FRTA_FAILURE) {
    return false;
  }
  const bool cache_enabled = template_arguments_cache_enabled();
  const bool fast_cache_enabled =
      cache_enabled && template_arguments_fast_cache_enabled();
  std::vector<std::uint64_t> syntax_fingerprints;
  ResolveTemplateArgumentsCacheProbe cache_probe;
  if(cache_enabled) {
    const bool include_source_identity_in_cache_key =
        witness::source_capture_enabled(services.witness_context);
    syntax_fingerprints =
        template_argument_syntax_fingerprints(resolution_inputs,
                                              include_source_identity_in_cache_key);
    cache_probe =
        make_resolve_template_arguments_cache_probe(raw_scope,
                                                    parameters,
                                                    resolution_inputs.texts,
                                                    syntax_fingerprints,
                                                    default_argument_declaring_scope.scope);
    cache_probe.hash_value =
        resolve_template_arguments_cache_hash(cache_probe,
                                              parameters,
                                              resolution_inputs.texts);
  }
  if(fast_cache_enabled) {
    if(const ResolveTemplateArgumentsCacheEntry * fast_cached =
           find_resolve_template_arguments_fast_cache_entry(cache_probe,
                                                            parameters,
                                                            resolution_inputs.texts)) {
      if(counters) {
        ++counters->resolve_template_argument_fast_cache_hits;
        ++counters->resolve_template_argument_cache_hits;
      }
      out = fast_cached->arguments;
      reattach_template_argument_source_syntaxes_from_inputs(resolution_inputs, out);
      if(fast_cached->success) {
        rehydrate_cached_defaulted_non_type_argument_witness_dependencies(
            services,
            raw_scope,
            parameters,
            resolution_inputs,
            default_argument_declaring_scope,
            out);
      }
      return fast_cached->success;
    }
  }
  const bool all_arguments_explicit =
      resolution_inputs.texts.size() >= parameters.size();
  const bool expensive_simple_fast_scope_ok =
      !default_argument_declaring_scope.scope ||
      default_argument_declaring_scope.scope == &raw_scope ||
      all_arguments_explicit;
  if(expensive_simple_fast_scope_ok) {
    const FastResolveTemplateArgumentsStatus precache_simple_template_fast_status =
        try_resolve_simple_template_arguments_fast(services,
                                                   type_system,
                                                   scope,
                                                   parameters,
                                                   resolution_inputs,
                                                   out,
                                                   true);
    note_expensive_simple_fast_status(precache_simple_template_fast_status);
    if(precache_simple_template_fast_status == FRTA_SUCCESS ||
       precache_simple_template_fast_status == FRTA_FAILURE) {
      if(fast_cache_enabled) {
        note_template_argument_key_texts();
        if(counters) {
          ++counters->resolve_template_argument_fast_entry_key_builds;
        }
        const ResolveTemplateArgumentsCacheKey fast_cache_key =
            make_resolve_template_arguments_cache_key(cache_probe,
                                                      parameters,
                                                      resolution_inputs.texts);
        const ResolveTemplateArgumentsCacheEntry fast_entry =
            make_resolve_template_arguments_cache_entry(
                precache_simple_template_fast_status == FRTA_SUCCESS,
                out);
        note_resolve_template_arguments_fast_cache_entry(fast_cache_key, fast_entry);
      }
      return precache_simple_template_fast_status == FRTA_SUCCESS;
    }
  }
  const std::vector<std::string> & cache_texts = resolution_inputs.texts;
  ResolveTemplateArgumentsCacheKey cache_key;
  typedef std::unordered_map<ResolveTemplateArgumentsCacheKey,
                             ResolveTemplateArgumentsCacheEntry,
                             ResolveTemplateArgumentsCacheKeyHash> Cache;
  Cache * cache = nullptr;
  if(cache_enabled) {
    note_template_argument_key_texts();
    if(counters) {
      ++counters->resolve_template_argument_key_builds;
    }
    cache_key =
        make_resolve_template_arguments_cache_key(cache_probe,
                                                  parameters,
                                                  resolution_inputs.texts);
    cache = &resolve_template_arguments_cache();
    Cache::const_iterator cached = cache->find(cache_key);
    if(cached != cache->end()) {
      if(counters) {
        ++counters->resolve_template_argument_cache_hits;
      }
      if(fast_cache_enabled) {
        note_resolve_template_arguments_fast_cache_entry(cache_key, cached->second);
      }
      out = cached->second.arguments;
      reattach_template_argument_source_syntaxes_from_inputs(resolution_inputs, out);
      if(cached->second.success) {
        rehydrate_cached_defaulted_non_type_argument_witness_dependencies(
            services,
            raw_scope,
            parameters,
            resolution_inputs,
            default_argument_declaring_scope,
            out);
      }
      return cached->second.success;
    }
    if(counters) {
      ++counters->resolve_template_argument_cache_misses;
    }
    if(semantic_hotspot::enabled()) {
      semantic_hotspot::note_semantic_query("resolve_template_arguments_cache_miss",
                                            hotspot_query_text_for(cache_texts));
    }
  }
  const auto note_cache_failure =
      [&]() -> void
  {
    if(cache_enabled) {
      if(counters) {
        ++counters->resolve_template_argument_failure_entries;
      }
      ResolveTemplateArgumentsCacheEntry entry =
          make_resolve_template_arguments_cache_entry(false,
                                                      std::vector<TemplateArgument>());
      (*cache)[cache_key] = entry;
      if(fast_cache_enabled) {
        note_resolve_template_arguments_fast_cache_entry(cache_key, entry);
      }
    }
  };
  const auto note_cache_success =
      [&]() -> void
  {
    if(cache_enabled) {
      if(counters) {
        ++counters->resolve_template_argument_success_entries;
      }
      ResolveTemplateArgumentsCacheEntry entry =
          make_resolve_template_arguments_cache_entry(true, out);
      (*cache)[cache_key] = entry;
      if(fast_cache_enabled) {
        note_resolve_template_arguments_fast_cache_entry(cache_key, entry);
      }
    }
  };
  const bool trailing_pack = !parameters.empty() && parameters.back().parameter_pack;
  if(!trailing_pack && cache_texts.size() > parameters.size()) {
    note_cache_failure();
    return false;
  }

  Scope bound_scope(&raw_scope, "", false);
  std::unique_ptr<Scope> default_argument_overlay;
  Scope * default_argument_scope = &bound_scope;
  if(default_argument_declaring_scope.scope &&
     default_argument_declaring_scope.scope != &raw_scope) {
    default_argument_overlay.reset(
        new Scope(default_argument_declaring_scope.scope, "", false));
    template_scope::overlay_ancestor_scope_bindings(*default_argument_overlay,
                                                    raw_scope,
                                                    default_argument_declaring_scope.scope,
                                                    template_scope::OVERLAY_TEMPLATE_BOUND_ONLY);
    default_argument_scope = default_argument_overlay.get();
  }
  std::size_t text_index = 0;
  const bool source_locations_active =
      template_api::current_template_argument_source_locations_active();
  const auto current_source_location =
      [&](const std::string & text, std::size_t index) -> std::string
  {
    if(counters) {
      ++counters->resolve_template_argument_source_location_calls;
    }
    return template_api::current_template_argument_source_location(text, index);
  };
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      if(i + 1 != parameters.size()) {
        note_cache_failure();
        return false;
      }
      while(text_index < cache_texts.size()) {
        TemplateArgument arg;
        const ScopedTemplateArgumentUseLocation argument_use_location(
            source_locations_active ?
                current_source_location(cache_texts[text_index], text_index) :
                std::string());
        if(!try_resolve_expanded_type_template_argument(
               services,
               type_system,
               scope,
               parameters[i],
               cache_texts[text_index],
               resolution_inputs.syntax_for(text_index),
               resolution_inputs.type_for(text_index),
               arg) &&
           !template_resolution::resolve_template_argument(
                services,
                scope,
                template_api::make_template_environment(bound_scope),
                parameters[i],
                cache_texts[text_index],
                resolution_inputs.syntax_for(text_index),
                arg)) {
          note_cache_failure();
          return false;
        }
        out.push_back(arg);
        ++text_index;
      }
      note_cache_success();
      return true;
    }

    if(text_index >= cache_texts.size()) {
      continue;
    }

    TemplateArgument arg;
    const ScopedTemplateArgumentUseLocation argument_use_location(
        source_locations_active ?
            current_source_location(cache_texts[text_index], text_index) :
            std::string());
    if(!try_resolve_expanded_type_template_argument(
           services,
           type_system,
           scope,
           parameters[i],
           cache_texts[text_index],
           resolution_inputs.syntax_for(text_index),
           resolution_inputs.type_for(text_index),
           arg) &&
       !template_resolution::resolve_template_argument(
            services,
            scope,
            template_api::make_template_environment(bound_scope),
            parameters[i],
            cache_texts[text_index],
            resolution_inputs.syntax_for(text_index),
            arg)) {
      note_cache_failure();
      return false;
    }
    out.push_back(arg);
    bind_single_template_argument_into_scope(services, bound_scope, parameters[i], arg);
    if(default_argument_scope != &bound_scope) {
      bind_single_template_argument_into_scope(
          services, *default_argument_scope, parameters[i], arg);
    }
    ++text_index;
  }

  for(std::size_t i = text_index; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      if(i + 1 != parameters.size()) {
        note_cache_failure();
        return false;
      }
      note_cache_success();
      return true;
    }
    if(!parameters[i].default_argument || parameters[i].default_argument->children.empty()) {
      note_cache_failure();
      return false;
    }

    TemplateArgument arg;
    const CppAstNode & child = parameters[i].default_argument->children[0];
    const template_argument_semantics::ScopedDefaultTemplateArgumentEvaluation
        default_argument_evaluation;
    const template_api::TemplateEnvironmentHandle default_argument_env =
        template_api::make_template_environment(*default_argument_scope);
    if(parameters[i].kind != TemplateParameterInfo::TP_NON_TYPE) {
      const std::string original_default_text =
          default_type_argument_text_from_ast(parameters[i], child);
      const TemplateArgumentSyntax original_default_syntax =
          make_default_template_argument_syntax(parameters[i],
                                                child,
                                                original_default_text);
      std::string prepared_default_text = original_default_text;
      TemplateArgumentSyntax prepared_default_syntax;
      const TemplateArgumentSyntax * prepared_default_syntax_ptr =
          &original_default_syntax;
      if(!original_default_text.empty()) {
        std::vector<TemplateParameterInfo> prefix_parameters(
            parameters.begin(),
            parameters.begin() + i);
        std::vector<TemplateArgument> prefix_arguments(out.begin(), out.end());
        if(make_substituted_default_template_argument_syntax(
               services,
               *default_argument_scope,
               parameters[i],
               child,
               original_default_text,
               prefix_parameters,
               prefix_arguments,
               prepared_default_syntax)) {
          prepared_default_text = prepared_default_syntax.text;
          prepared_default_syntax_ptr = &prepared_default_syntax;
        }
      }
      bool resolved_default = false;
      if(parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
         !original_default_text.empty()) {
        const bool default_mentions_template_placeholders =
            template_argument_semantics::text_mentions_template_placeholders(
                services,
                template_api::make_template_environment(*default_argument_scope),
                prepared_default_text);
        const bool default_mentions_dependent_bindings =
            template_argument_semantics::text_mentions_dependent_non_namespace_binding_names(
                services,
                template_api::make_template_environment(*default_argument_scope),
                prepared_default_text);
        const bool default_should_defer_lookup =
            should_defer_unresolved_type_lookup(
                services,
                template_api::make_template_environment(*default_argument_scope),
                prepared_default_text);
        const bool should_defer_original_default_text =
            default_mentions_template_placeholders ||
            default_mentions_dependent_bindings ||
            default_should_defer_lookup;
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "default-type-arg name=" << parameters[i].name
                << " text=" << original_default_text
                << " prepared=" << prepared_default_text
                << " placeholders="
                << (default_mentions_template_placeholders ? "yes" : "no")
                << " dependent-bindings="
                << (default_mentions_dependent_bindings ? "yes" : "no")
                << " defer-lookup="
                << (default_should_defer_lookup ? "yes" : "no")
                << " defer-now="
                << (should_defer_original_default_text ? "yes" : "no");
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        if(should_defer_original_default_text) {
          try {
            const ScopedTemplateArgumentUseLocation default_use_location(
                template_api::normalize_template_witness_source_location(
                    source_location_for_ast_start(services.witness_context,
                                                  child)));
            resolved_default = template_resolution::resolve_template_argument(
                services,
                default_argument_env,
                default_argument_env,
                parameters[i],
                prepared_default_text,
                prepared_default_syntax_ptr,
                arg);
          } catch(const TemplateSubstitutionFailure &) {
            resolved_default = false;
          }
        }
        if(should_defer_original_default_text && !resolved_default) {
          try {
            const ScopedTemplateArgumentUseLocation default_use_location(
                template_api::normalize_template_witness_source_location(
                    source_location_for_ast_start(services.witness_context,
                                                  child)));
            resolved_default = template_resolution::resolve_template_argument(
                services,
                default_argument_env,
                default_argument_env,
                parameters[i],
                original_default_text,
                &original_default_syntax,
                arg);
          } catch(const TemplateSubstitutionFailure &) {
            resolved_default = false;
          }
          if(resolved_default &&
             (!arg.type ||
              type_is_dependent(arg.type))) {
            arg = TemplateArgument();
            resolved_default = false;
          }
        }
        if(should_defer_original_default_text && !resolved_default) {
          arg.kind = TemplateArgument::TA_TYPE;
          arg.type = make_named(prepared_default_text,
                                "dependent type " + prepared_default_text,
                                true);
          arg.text = prepared_default_text;
          arg.dependent = true;
          attach_template_argument_source_syntax(prepared_default_syntax_ptr, arg);
          resolved_default = true;
        }
        try {
          if(!resolved_default) {
            resolved_default = template_resolution::resolve_template_argument(
                services,
                default_argument_env,
                default_argument_env,
                parameters[i],
                original_default_text,
                &original_default_syntax,
                arg);
          }
        } catch(const TemplateSubstitutionFailure &) {
          resolved_default = false;
        }
      }

      if(!resolved_default) {
        if(prepared_default_text.empty() ||
           !template_resolution::resolve_template_argument(services,
                                                           default_argument_env,
                                                           default_argument_env,
                                                           parameters[i],
                                                           prepared_default_text,
                                                           prepared_default_syntax_ptr,
                                                           arg)) {
          note_cache_failure();
          return false;
        }
      }
      if(parameters[i].kind == TemplateParameterInfo::TP_TYPE &&
         arg.type &&
         type_is_dependent(arg.type) &&
         i != 0) {
        std::vector<TemplateParameterInfo> prefix_parameters(parameters.begin(),
                                                             parameters.begin() + i);
        std::vector<TemplateArgument> prefix_arguments(out.begin(), out.end());
        TypePtr substituted_type;
        if(template_argument_semantics::substitute_type(arg.type,
                                                        prefix_parameters,
                                                        prefix_arguments,
                                                        substituted_type) &&
           substituted_type) {
          arg.type = substituted_type;
          if(!type_is_dependent(arg.type)) {
            arg.text = type_argument_text(arg.type);
          }
        }
      }
    } else {
      TypePtr bound_value_type;
      if(!try_resolve_non_type_template_parameter_type(
             services, default_argument_env, parameters[i], bound_value_type)) {
        if(!non_type_template_parameter_is_still_dependent(
               services, default_argument_env, parameters[i])) {
          note_cache_failure();
          return false;
        }
        bound_value_type = parameters[i].value_type;
      }
      std::string default_text = default_argument_expression_text(child);
      const TemplateArgumentSyntax default_syntax =
          make_default_template_argument_syntax(parameters[i],
                                                child,
                                                default_text);
      TemplateArgumentSyntax dependency_default_syntax = default_syntax;
      CppAstNode substituted_default_expression;
      std::string substituted_default_text;
      const bool have_substituted_default_expression =
          template_argument_semantics::
              substitute_expression_node_for_template_arguments(
                  *default_argument_scope,
                  child,
                  parameters,
                  out,
                  substituted_default_expression);
      if(have_substituted_default_expression) {
        substituted_default_text =
            default_argument_expression_text(substituted_default_expression);
        dependency_default_syntax =
            make_default_template_argument_syntax(parameters[i],
                                                  substituted_default_expression,
                                                  substituted_default_text);
      }
      long long value = 0;
      template_argument_semantics::NonTypeArgumentStatus value_status =
          template_argument_semantics::NT_ARG_EVAL_FAILED;
      std::string eval_error;
      try {
        value_status =
            template_argument_semantics::evaluate_non_type_argument_expression(
                services,
                default_argument_env,
                child,
                value,
                &eval_error,
                bound_value_type);
      } catch(const std::logic_error & e) {
        eval_error = e.what();
      }
      if(value_status != template_argument_semantics::NT_ARG_EVALUATED) {
        if(value_status != template_argument_semantics::NT_ARG_DEPENDENT &&
           !default_argument_expression_is_still_dependent(
               services, default_argument_env, child)) {
          std::ostringstream detail;
          detail << "failed default non-type template argument evaluation: "
                 << default_text;
          detail << " [scope " << scope_name_for_diagnostic(*default_argument_scope) << "]";
          detail << " [bindings "
                 << scope_bindings_for_diagnostic(*default_argument_scope) << "]";
          const std::string described =
              callsemantic_internal::describe_expression_for_diagnostic(child);
          if(!described.empty()) {
            detail << " [expr " << described << "]";
          }
          if(!eval_error.empty()) {
            detail << " [eval " << eval_error << "]";
          }
          throw_substitution_failure(detail.str(), std::string(), "template-resolution");
        }
        arg.kind = TemplateArgument::TA_VALUE;
        arg.type = bound_value_type;
        if(have_substituted_default_expression) {
          arg.expression.reset(new CppAstNode(substituted_default_expression));
          default_text = substituted_default_text;
        }
        arg.text = default_text;
        attach_template_argument_source_syntax(&default_syntax, arg);
        arg.dependent = true;
      } else {
        arg.kind = TemplateArgument::TA_VALUE;
        arg.type = bound_value_type;
        arg.value = value;
        arg.text = typed_non_type_template_argument_text(
            type_system,
            bound_value_type,
            value);
        if(have_substituted_default_expression) {
          arg.expression.reset(new CppAstNode(substituted_default_expression));
        }
        attach_template_argument_source_syntax(&default_syntax, arg);
        if(services.witness_context.session != nullptr &&
           services.semantic_context) {
          template_argument_semantics::
              append_structured_bool_value_dependencies_in_template_argument_syntax(
                  services,
                  default_argument_env,
                  dependency_default_syntax,
                  arg.value_dependencies);
          template_argument_semantics::
              append_non_bool_static_value_dependencies_in_template_argument_syntax(
                  services,
                  default_argument_env,
                  dependency_default_syntax,
                  bound_value_type,
                  arg.value_dependencies);
          template_argument_semantics::note_template_value_dependencies_for_witness(
              *services.semantic_context,
              arg.value_dependencies);
          template_argument_semantics::
              note_structured_bool_value_members_in_template_argument_syntax(
                  services,
                  default_argument_env,
                  dependency_default_syntax);
        }
      }
    }

    arg.source_defaulted = true;
    out.push_back(arg);
    bind_single_template_argument_into_scope(services, bound_scope, parameters[i], arg);
    if(default_argument_scope != &bound_scope) {
      bind_single_template_argument_into_scope(
          services, *default_argument_scope, parameters[i], arg);
    }
  }

  note_cache_success();
  return true;
}

bool resolve_template_arguments(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::vector<TemplateParameterInfo> & parameters,
    const std::vector<std::string> & texts,
    std::vector<TemplateArgument> & out,
    template_api::TemplateEnvironmentHandle default_argument_declaring_scope)
{
  return template_resolution::resolve_template_arguments(
      services, scope, parameters, texts, nullptr, out, default_argument_declaring_scope);
}

bool resolve_function_explicit_template_arguments(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & resolution_scope,
    const std::vector<std::string> & explicit_arg_texts,
    std::vector<TemplateArgument> & out,
    const std::vector<TemplateArgumentSyntax> * explicit_arg_syntaxes)
{
  out.clear();
  if(explicit_arg_texts.empty()) {
    return true;
  }

  const std::size_t pack_index = first_template_parameter_pack_index(decl.parameters);
  if(pack_index == decl.parameters.size() ||
     explicit_arg_texts.size() <= pack_index) {
    std::vector<TemplateParameterInfo> explicit_parameters(
        decl.parameters.begin(),
        decl.parameters.begin() + explicit_arg_texts.size());
    try {
      return resolve_template_arguments(ctx,
                                        resolution_scope,
                                        explicit_parameters,
                                        explicit_arg_texts,
                                        explicit_arg_syntaxes,
                                        out,
                                        decl.declaring_scope) &&
             out.size() == explicit_arg_texts.size();
    } catch(const TemplateSubstitutionFailure &) {
      out.clear();
      return false;
    }
  }

  std::vector<TemplateArgument> fixed_arguments;
  if(pack_index != 0) {
    std::vector<TemplateParameterInfo> fixed_parameters(
        decl.parameters.begin(),
        decl.parameters.begin() + pack_index);
    std::vector<std::string> fixed_texts(explicit_arg_texts.begin(),
                                         explicit_arg_texts.begin() + pack_index);
    std::vector<TemplateArgumentSyntax> fixed_syntaxes;
    const std::vector<TemplateArgumentSyntax> * fixed_arg_syntaxes = nullptr;
    if(explicit_arg_syntaxes && explicit_arg_syntaxes->size() >= pack_index) {
      fixed_syntaxes.assign(explicit_arg_syntaxes->begin(),
                            explicit_arg_syntaxes->begin() + pack_index);
      fixed_arg_syntaxes = &fixed_syntaxes;
    }
    bool fixed_ok = false;
    try {
      fixed_ok = resolve_template_arguments(ctx,
                                            resolution_scope,
                                            fixed_parameters,
                                            fixed_texts,
                                            fixed_arg_syntaxes,
                                            fixed_arguments,
                                            decl.declaring_scope) &&
                 fixed_arguments.size() == fixed_texts.size();
    } catch(const TemplateSubstitutionFailure &) {
      fixed_ok = false;
    }
    if(!fixed_ok) {
      return false;
    }
  }

  Scope explicit_argument_scope(&resolution_scope, "", false);
  out = fixed_arguments;
  for(std::size_t i = 0; i < fixed_arguments.size(); ++i) {
    ctx.bind_single_template_argument_into_scope(explicit_argument_scope,
                                                 decl.parameters[i],
                                                 fixed_arguments[i]);
  }

  const TemplateParameterInfo & pack_parameter = decl.parameters[pack_index];
  std::vector<std::string> pack_arg_texts(explicit_arg_texts.begin() + pack_index,
                                          explicit_arg_texts.end());
  std::vector<TemplateArgumentSyntax> pack_arg_syntax_storage;
  const std::vector<TemplateArgumentSyntax> * pack_arg_syntaxes = nullptr;
  if(explicit_arg_syntaxes && explicit_arg_syntaxes->size() == explicit_arg_texts.size()) {
    pack_arg_syntax_storage.assign(explicit_arg_syntaxes->begin() + pack_index,
                                   explicit_arg_syntaxes->end());
    pack_arg_syntaxes = &pack_arg_syntax_storage;
  }
  template_argument_semantics::ExpandedTemplateArgumentInputs expanded_pack_inputs;
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        expanded_pack_inputs =
            template_argument_semantics::expand_template_argument_inputs(
                services, explicit_argument_scope, pack_arg_texts, pack_arg_syntaxes);
        pack_arg_texts = expanded_pack_inputs.texts;
        return true;
      });
  for(std::size_t i = 0; i < pack_arg_texts.size(); ++i) {
    TemplateArgument argument;
    bool argument_ok = false;
    try {
      argument_ok = resolve_template_argument(ctx,
                                              explicit_argument_scope,
                                              explicit_argument_scope,
                                              pack_parameter,
                                              pack_arg_texts[i],
                                              expanded_pack_inputs.syntax_for(i),
                                              argument);
    } catch(const TemplateSubstitutionFailure &) {
      argument_ok = false;
    }
    if(!argument_ok) {
      return false;
    }
    out.push_back(argument);
  }
  return true;
}

bool partition_explicit_function_template_arguments(
    FunctionTemplateDecl & decl,
    const std::vector<TemplateArgument> & explicit_arguments,
    ExplicitFunctionTemplateArgumentBindings & out)
{
  out = ExplicitFunctionTemplateArgumentBindings();
  if(explicit_arguments.empty()) {
    return true;
  }

  const std::size_t pack_index = first_template_parameter_pack_index(decl.parameters);
  if(decl.parameters.empty() ||
     (pack_index == decl.parameters.size() &&
      explicit_arguments.size() > decl.parameters.size())) {
    return false;
  }

  if(pack_index == decl.parameters.size() ||
     explicit_arguments.size() <= pack_index) {
    out.fixed_arguments = explicit_arguments;
    return true;
  }

  if(pack_index != 0) {
    out.fixed_arguments.assign(explicit_arguments.begin(),
                               explicit_arguments.begin() + pack_index);
  }
  out.pack_parameter_index = pack_index;
  out.pack_arguments.assign(explicit_arguments.begin() + pack_index,
                            explicit_arguments.end());
  return true;
}

void bind_explicit_function_template_pack_arguments(
    Scope & bound_scope,
    const TemplateParameterInfo & parameter,
    const std::vector<TemplateArgument> & arguments)
{
  template_scope::bind_template_argument_pack(
      bound_scope, parameter, arguments, true);
}

bool deduction_top_level_cv_flags(const TypePtr & type,
                                  TypePtr & base,
                                  bool & cv_const,
                                  bool & cv_volatile)
{
  if(!top_level_cv_flags(type, base, cv_const, cv_volatile)) {
    return false;
  }
  if((cv_const || cv_volatile) ||
     !type ||
     type->kind != Type::TK_ARRAY ||
     !type->inner) {
    return true;
  }

  TypePtr element_base;
  bool element_const = false;
  bool element_volatile = false;
  if(!top_level_cv_flags(type->inner,
                         element_base,
                         element_const,
                         element_volatile) ||
     (!element_const && !element_volatile)) {
    return true;
  }

  base = make_array(element_base,
                    type->has_bound,
                    type->bound,
                    type->bound_text);
  cv_const = element_const;
  cv_volatile = element_volatile;
  return true;
}

template <typename DeductionContext>
bool deduce_template_argument_impl(DeductionContext & ctx,
                                   const std::vector<TemplateParameterInfo> & parameters,
                                   const TypePtr & pattern,
                                   const TypePtr & actual,
                                   DeducedTypeMap & deduced_types,
                                   DeducedValueMap & deduced_values,
                                   Scope * deduction_scope,
                                   bool partial_top_level_cv_deduction,
                                   Scope * actual_lookup_scope,
                                   DeducedPackArgumentMap * deduced_pack_arguments = nullptr,
                                   bool allow_actual_base_deduction = true)
{
  const DeductionContextOps deduction_ops(ctx);
  TypePtr pattern_cv_inner;
  TypePtr actual_cv_inner;
  bool pattern_const = false;
  bool pattern_volatile = false;
  bool actual_const = false;
  bool actual_volatile = false;
  if(partial_top_level_cv_deduction &&
     top_level_cv_flags(pattern, pattern_cv_inner, pattern_const, pattern_volatile) &&
     top_level_cv_flags(actual, actual_cv_inner, actual_const, actual_volatile) &&
     (pattern_const || pattern_volatile)) {
    if((pattern_const && !actual_const) ||
       (pattern_volatile && !actual_volatile)) {
      return false;
    }
    return deduce_template_argument_impl(ctx,
                                         parameters,
                                         pattern_cv_inner,
                                         actual_cv_inner,
                                         deduced_types,
                                         deduced_values,
                                         deduction_scope,
                                         partial_top_level_cv_deduction,
                                         actual_lookup_scope,
                                         deduced_pack_arguments,
                                         allow_actual_base_deduction);
  }

  TypePtr pattern_base = pattern;
  TypePtr actual_base = actual;
  if(!partial_top_level_cv_deduction) {
    TypePtr stripped;
    bool cv_const = false;
    bool cv_volatile = false;
    if(top_level_cv_flags(pattern, stripped, cv_const, cv_volatile)) {
      pattern_base = stripped;
    }
    if(top_level_cv_flags(actual, stripped, cv_const, cv_volatile)) {
      actual_base = stripped;
    }
  }
  if(!pattern_base || !actual_base) {
    return false;
  }

  if(pattern_base->kind == Type::TK_NAMED) {
    const TemplateParameterInfo * parameter =
        find_template_parameter(parameters, pattern_base->named_key);
    if(!parameter && pattern_base->named_display != pattern_base->named_key) {
      parameter = find_template_parameter(parameters, pattern_base->named_display);
    }
    if(parameter && parameter->kind == TemplateParameterInfo::TP_TYPE) {
      TypePtr deduced_type = actual;
      TypePtr actual_cv_inner;
      TypePtr pattern_cv_inner;
      bool pattern_named_const = false;
      bool pattern_named_volatile = false;
      bool actual_const = false;
      bool actual_volatile = false;
      if(deduction_top_level_cv_flags(pattern,
                                      pattern_cv_inner,
                                      pattern_named_const,
                                      pattern_named_volatile) &&
         deduction_top_level_cv_flags(actual,
                                      actual_cv_inner,
                                      actual_const,
                                      actual_volatile) &&
         ((pattern_named_const && actual_const) ||
          (pattern_named_volatile && actual_volatile))) {
        deduced_type = actual_cv_inner;
      }
      DeducedTypeMap::iterator found = deduced_types.find(parameter->name);
      if(found == deduced_types.end()) {
        deduced_types[parameter->name] = deduced_type;
        return true;
      }
      return type_equals(found->second, deduced_type);
    }

    TypePtr pattern_named_cv_inner;
    TypePtr actual_named_cv_inner;
    bool pattern_named_const = false;
    bool pattern_named_volatile = false;
    bool actual_named_const = false;
    bool actual_named_volatile = false;
    if(partial_top_level_cv_deduction &&
       deduction_top_level_cv_flags(pattern,
                                    pattern_named_cv_inner,
                                    pattern_named_const,
                                    pattern_named_volatile) &&
       deduction_top_level_cv_flags(actual,
                                    actual_named_cv_inner,
                                    actual_named_const,
                                    actual_named_volatile) &&
       ((actual_named_const && !pattern_named_const) ||
        (actual_named_volatile && !pattern_named_volatile))) {
      return false;
    }

    const bool pattern_mentions_template_parameter =
        deduction_scope &&
        deduction_pattern_mentions_function_template_parameter(
            deduction_ops, parameters, *deduction_scope, pattern_base);
    if(!pattern_mentions_template_parameter &&
       (type_equals(pattern, actual) ||
        type_equals(pattern_base, actual_base))) {
      return true;
    }

    if(deduction_scope) {
      TypePtr canonical_pattern = canonicalize_dependent_alias_type_for_deduction(
          ctx, *deduction_scope, pattern_base, deduction_scope);
      if(canonical_pattern &&
         !type_equals(canonical_pattern, pattern_base) &&
         deduce_template_argument_impl(ctx,
                                       parameters,
                                       canonical_pattern,
                                       actual,
                                       deduced_types,
                                       deduced_values,
                                       deduction_scope,
                                       partial_top_level_cv_deduction,
                                       actual_lookup_scope,
                                       deduced_pack_arguments,
                                       allow_actual_base_deduction)) {
        return true;
      }

      TypePtr canonical_actual = canonicalize_dependent_alias_type_for_deduction(
          ctx, *deduction_scope, actual_base, actual_lookup_scope);
      if(canonical_actual &&
         !type_equals(canonical_actual, actual_base) &&
         deduce_template_argument_impl(ctx,
                                       parameters,
                                       pattern,
                                       canonical_actual,
                                       deduced_types,
                                       deduced_values,
                                       deduction_scope,
                                       partial_top_level_cv_deduction,
                                       actual_lookup_scope,
                                       deduced_pack_arguments,
                                       allow_actual_base_deduction)) {
        return true;
      }

      const std::string normalized_pattern = strip_elaborated_type_prefix(
          trim_space(deduction_ops.type_argument_text(pattern_base)));
      const std::string normalized_actual = strip_elaborated_type_prefix(
          trim_space(deduction_ops.type_argument_text(actual_base)));
      const std::string pattern_head = named_type_head_text(normalized_pattern);
      const std::string actual_head = named_type_head_text(normalized_actual);
      struct DirectTemplateParameterMatch
      {
        const TemplateParameterInfo * parameter = nullptr;
        bool pack_expansion = false;
      };
      const auto find_direct_template_parameter_from_arg =
          [&parameters](const std::string & raw_arg) -> DirectTemplateParameterMatch
      {
        std::string normalized = strip_elaborated_type_prefix(trim_space(raw_arg));
        static const char * prefixes[] = {
            "typename ",
            "class ",
            "struct ",
            "template-parameter ",
            "type-parameter ",
            "dependent type ",
            "dependent alias "
        };
        for(std::size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
          const std::string prefix(prefixes[i]);
          if(normalized.compare(0, prefix.size(), prefix) == 0) {
            normalized.erase(0, prefix.size());
            break;
          }
        }
        DirectTemplateParameterMatch out;
        if(normalized.size() >= 3 &&
           normalized.compare(normalized.size() - 3, 3, "...") == 0) {
          out.pack_expansion = true;
          normalized.erase(normalized.size() - 3);
          normalized = trim_space(normalized);
        }
        const TemplateParameterInfo * direct =
            find_template_parameter_by_name(parameters, normalized);
        if(!direct) {
          direct = find_template_parameter(parameters, normalized);
        }
        if(direct &&
           (direct->kind == TemplateParameterInfo::TP_TYPE ||
            direct->kind == TemplateParameterInfo::TP_NON_TYPE)) {
          out.parameter = direct;
        }
        return out;
      };

      const auto lookup_actual_arg_type =
          [&](const std::string & actual_arg, TypePtr & actual_arg_type) -> bool
      {
        actual_arg_type.reset();
        if(actual_lookup_scope) {
          actual_arg_type =
              deduction_ops.lookup_type(*actual_lookup_scope, actual_arg, true);
        }
        if(!actual_arg_type &&
           (!actual_lookup_scope || actual_lookup_scope != deduction_scope)) {
          actual_arg_type =
              deduction_ops.lookup_type(*deduction_scope, actual_arg, true);
        }
        if(actual_arg_type) {
          return true;
        }

        if(actual_lookup_scope) {
          semantic_fallback_audit::hard_fail(
              "template-type-resolution-fallback",
              std::string(),
              "actual template argument resolution fell back from lookup"
              " to semantic-only resolution [arg " + actual_arg + "]");
          return false;
        }
        if(!actual_lookup_scope || actual_lookup_scope != deduction_scope) {
          semantic_fallback_audit::hard_fail(
              "template-type-resolution-fallback",
              std::string(),
              "actual template argument resolution fell back to deduction scope"
              " [arg " + actual_arg + "]");
          return false;
        }
        return false;
      };
      const auto resolve_direct_actual_template_argument =
          [&](const TemplateParameterInfo & parameter,
              const std::string & actual_arg,
              const TemplateArgument * structured_actual,
              TemplateArgument & resolved_arg) -> bool
      {
        resolved_arg = TemplateArgument();
        if(parameter.kind == TemplateParameterInfo::TP_TYPE) {
          if(structured_actual) {
            if(structured_actual->kind != TemplateArgument::TA_TYPE ||
               !structured_actual->type) {
              return false;
            }
            resolved_arg = *structured_actual;
            return true;
          }

          TypePtr actual_arg_type;
          if(!lookup_actual_arg_type(actual_arg, actual_arg_type)) {
            return false;
          }
          resolved_arg.kind = TemplateArgument::TA_TYPE;
          resolved_arg.type = actual_arg_type;
          resolved_arg.text =
              deduction_ops.lookup_type_text(actual_arg_type);
          return true;
        }

        if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE) {
          if(structured_actual) {
            if(structured_actual->kind != TemplateArgument::TA_VALUE ||
               structured_actual->dependent) {
              return false;
            }
            resolved_arg = *structured_actual;
            return true;
          }

          Scope & actual_scope = actual_lookup_scope ? *actual_lookup_scope : *deduction_scope;
          return resolve_template_argument_for_deduction(ctx,
                                                         actual_scope,
                                                         *deduction_scope,
                                                         parameter,
                                                         actual_arg,
                                                         resolved_arg) &&
                 resolved_arg.kind == TemplateArgument::TA_VALUE &&
                 !resolved_arg.dependent;
        }

        return false;
      };
      const auto template_arguments_are_all_types =
          [](const std::vector<TemplateArgument> & arguments) -> bool
      {
        for(std::size_t i = 0; i < arguments.size(); ++i) {
          if(arguments[i].kind != TemplateArgument::TA_TYPE ||
             !arguments[i].type) {
            return false;
          }
        }
        return true;
      };
      const auto deduce_from_effective_type_template_arguments =
          [&](const std::vector<TemplateArgument> & pattern_effective_args,
              const std::vector<TemplateArgument> & actual_effective_args) -> bool
      {
        if(pattern_effective_args.size() != actual_effective_args.size() ||
           !template_arguments_are_all_types(pattern_effective_args) ||
           !template_arguments_are_all_types(actual_effective_args)) {
          return false;
        }
        for(std::size_t i = 0; i < pattern_effective_args.size(); ++i) {
          if(!deduce_template_argument_impl(ctx,
                                            parameters,
                                            pattern_effective_args[i].type,
                                            actual_effective_args[i].type,
                                            deduced_types,
                                            deduced_values,
                                            deduction_scope,
                                            partial_top_level_cv_deduction,
                                            actual_lookup_scope,
                                            deduced_pack_arguments,
                                            allow_actual_base_deduction)) {
            return false;
          }
        }
        return true;
      };
      std::function<bool(const std::string &, const std::string &)>
          deduce_template_argument_text_shape;
      deduce_template_argument_text_shape =
          [&](const std::string & raw_pattern_arg,
              const std::string & raw_actual_arg) -> bool
      {
        const std::string pattern_arg = trim_space(raw_pattern_arg);
        const std::string actual_arg = trim_space(raw_actual_arg);
        if(pattern_arg == actual_arg) {
          return true;
        }

        const auto split_trailing_indirection =
            [](const std::string & text,
               std::string & inner,
               std::string & suffix) -> bool
        {
          std::string trimmed = trim_space(text);
          if(trimmed.size() >= 2 &&
             trimmed.compare(trimmed.size() - 2, 2, "&&") == 0) {
            inner = trim_space(trimmed.substr(0, trimmed.size() - 2));
            suffix = "&&";
            return !inner.empty();
          }
          if(!trimmed.empty() &&
             (trimmed[trimmed.size() - 1] == '*' ||
              trimmed[trimmed.size() - 1] == '&')) {
            inner = trim_space(trimmed.substr(0, trimmed.size() - 1));
            suffix.assign(1, trimmed[trimmed.size() - 1]);
            return !inner.empty();
          }
          return false;
        };

        std::string pattern_inner;
        std::string actual_inner;
        std::string pattern_suffix;
        std::string actual_suffix;
        const bool pattern_has_indirection =
            split_trailing_indirection(pattern_arg, pattern_inner, pattern_suffix);
        const bool actual_has_indirection =
            split_trailing_indirection(actual_arg, actual_inner, actual_suffix);
        if(pattern_has_indirection || actual_has_indirection) {
          if(pattern_suffix.empty() || actual_suffix.empty() ||
             pattern_suffix != actual_suffix) {
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "deduce-text-shape-indirection-mismatch pattern="
                    << pattern_arg
                    << " actual=" << actual_arg
                    << " pattern-suffix=" << pattern_suffix
                    << " actual-suffix=" << actual_suffix;
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            return false;
          }
          const bool nested =
              deduce_template_argument_text_shape(pattern_inner, actual_inner);
          if(!nested && parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "deduce-text-shape-indirection-inner-fail pattern="
                  << pattern_inner
                  << " actual=" << actual_inner
                  << " suffix=" << pattern_suffix;
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          return nested;
        }

        const DirectTemplateParameterMatch direct_match =
            find_direct_template_parameter_from_arg(pattern_arg);
        if(direct_match.parameter && !direct_match.pack_expansion) {
          TemplateArgument actual_direct_argument;
          if(!resolve_direct_actual_template_argument(*direct_match.parameter,
                                                      actual_arg,
                                                      nullptr,
                                                      actual_direct_argument)) {
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "deduce-text-shape-direct-resolve-fail parameter="
                    << direct_match.parameter->name
                    << " actual=" << actual_arg;
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            return false;
          }
          if(direct_match.parameter->kind == TemplateParameterInfo::TP_TYPE) {
            DeducedTypeMap::iterator found =
                deduced_types.find(direct_match.parameter->name);
            if(found == deduced_types.end()) {
              deduced_types[direct_match.parameter->name] =
                  actual_direct_argument.type;
              return true;
            }
            return type_equals(found->second, actual_direct_argument.type);
          }
          return record_deduced_non_type_value(parameters,
                                               direct_match.parameter->name,
                                               actual_direct_argument.value,
                                               deduced_values);
        }

        TypePtr bound_pattern_arg_type;
        if(lookup_exact_bound_type_name_for_deduction(*deduction_scope,
                                                      pattern_arg,
                                                      bound_pattern_arg_type) &&
           !deduction_ops.type_depends(bound_pattern_arg_type)) {
          TypePtr actual_arg_type;
          if(!lookup_actual_arg_type(actual_arg, actual_arg_type) ||
             !actual_arg_type) {
            return false;
          }
          return deduce_template_argument_impl(ctx,
                                               parameters,
                                               bound_pattern_arg_type,
                                               actual_arg_type,
                                               deduced_types,
                                               deduced_values,
                                               deduction_scope,
                                               partial_top_level_cv_deduction,
                                               actual_lookup_scope,
                                               deduced_pack_arguments,
                                               allow_actual_base_deduction);
        }

        QualifiedName pattern_name;
        QualifiedName actual_name;
        std::vector<std::string> pattern_nested_args;
        std::vector<std::string> actual_nested_args;
        const bool pattern_is_template_id =
            semantic_utils::split_top_level_template_id_text(pattern_arg,
                                                             pattern_name,
                                                             pattern_nested_args);
        const bool actual_is_template_id =
            semantic_utils::split_top_level_template_id_text(actual_arg,
                                                             actual_name,
                                                             actual_nested_args);
        if(pattern_is_template_id || actual_is_template_id) {
          if(!pattern_is_template_id ||
             !actual_is_template_id ||
             !template_names_match(pattern_name, actual_name) ||
             pattern_nested_args.size() != actual_nested_args.size()) {
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "deduce-text-shape-template-mismatch pattern="
                    << pattern_arg
                    << " actual=" << actual_arg
                    << " pattern-template=" << (pattern_is_template_id ? "yes" : "no")
                    << " actual-template=" << (actual_is_template_id ? "yes" : "no")
                    << " pattern-args=" << pattern_nested_args.size()
                    << " actual-args=" << actual_nested_args.size();
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            return false;
          }
          for(std::size_t i = 0; i < pattern_nested_args.size(); ++i) {
            if(!deduce_template_argument_text_shape(pattern_nested_args[i],
                                                    actual_nested_args[i])) {
              return false;
            }
          }
          return true;
        }

        return false;
      };
      const auto record_deduced_type_pack_arguments =
          [&](const TemplateParameterInfo & parameter,
              const std::vector<TypePtr> & actual_arg_types) -> bool
      {
        if(!deduced_pack_arguments || parameter.name.empty()) {
          return false;
        }

        std::vector<TemplateArgument> pack_arguments;
        pack_arguments.reserve(actual_arg_types.size());
        for(std::size_t i = 0; i < actual_arg_types.size(); ++i) {
          TemplateArgument arg;
          arg.kind = TemplateArgument::TA_TYPE;
          arg.type = actual_arg_types[i];
          arg.text = deduction_ops.type_depends(actual_arg_types[i]) ?
                         parameter.name :
                         deduction_ops.lookup_type_text(actual_arg_types[i]);
          pack_arguments.push_back(arg);
        }

        DeducedPackArgumentMap::iterator found =
            deduced_pack_arguments->find(parameter.name);
        if(found == deduced_pack_arguments->end()) {
          (*deduced_pack_arguments)[parameter.name] = pack_arguments;
          return true;
        }
        if(found->second.size() != pack_arguments.size()) {
          return false;
        }
        for(std::size_t i = 0; i < pack_arguments.size(); ++i) {
          if(found->second[i].kind != TemplateArgument::TA_TYPE ||
             !type_equals(found->second[i].type, pack_arguments[i].type)) {
            return false;
          }
        }
        return true;
      };
      const auto template_arguments_equivalent =
          [](const TemplateArgument & lhs, const TemplateArgument & rhs) -> bool
      {
        if(lhs.kind != rhs.kind || lhs.dependent != rhs.dependent) {
          return false;
        }
        if(lhs.kind == TemplateArgument::TA_TYPE) {
          return type_equals(lhs.type, rhs.type);
        }
        if(lhs.kind == TemplateArgument::TA_VALUE) {
          const bool types_match =
              (!lhs.type && !rhs.type) ||
              (lhs.type && rhs.type && type_equals(lhs.type, rhs.type));
          return types_match &&
                 lhs.value == rhs.value &&
                 lhs.text == rhs.text;
        }
        return lhs.template_decl == rhs.template_decl &&
               lhs.text == rhs.text;
      };
      const auto record_deduced_template_pack_arguments =
          [&](const TemplateParameterInfo & parameter,
              const std::vector<TemplateArgument> & pack_arguments) -> bool
      {
        if(!deduced_pack_arguments || parameter.name.empty()) {
          return false;
        }

        DeducedPackArgumentMap::iterator found =
            deduced_pack_arguments->find(parameter.name);
        if(found == deduced_pack_arguments->end()) {
          (*deduced_pack_arguments)[parameter.name] = pack_arguments;
          return true;
        }
        if(found->second.size() != pack_arguments.size()) {
          return false;
        }
        for(std::size_t i = 0; i < pack_arguments.size(); ++i) {
          if(!template_arguments_equivalent(found->second[i], pack_arguments[i])) {
            return false;
          }
        }
        return true;
      };

      Scope * actual_metadata_scope =
          actual_lookup_scope ? actual_lookup_scope : deduction_scope;
      template_api::TemplateNamedTypeMetadata actual_class;
      bool actual_class_metadata_attempted = false;
      bool have_actual_class = false;
      const auto ensure_actual_class_for_base_deduction = [&]() -> bool
      {
        if(!allow_actual_base_deduction) {
          return false;
        }
        if(!actual_class_metadata_attempted) {
          actual_class_metadata_attempted = true;
          try {
            have_actual_class =
                deduction_ops.describe_named_type_for_base_deduction(
                    actual_base, actual_metadata_scope, actual_class);
          } catch(const TemplateSubstitutionFailure &) {
            have_actual_class = false;
          } catch(const std::logic_error &) {
            have_actual_class = false;
          }
        }
        return have_actual_class;
      };
      const auto try_actual_base_deduction = [&]() -> bool
      {
        if(!ensure_actual_class_for_base_deduction()) {
          return false;
        }
        for(std::size_t i = 0; i < actual_class.direct_base_types.size(); ++i) {
          DeducedTypeMap deduced_types_copy;
          DeducedValueMap deduced_values_copy;
          DeducedPackArgumentMap deduced_pack_arguments_copy;
          clone_deduced_type_map(deduced_types, deduced_types_copy);
          clone_deduced_value_map(deduced_values, deduced_values_copy);
          DeducedPackArgumentMap * pack_arguments_copy = nullptr;
          if(deduced_pack_arguments) {
            deduced_pack_arguments_copy = *deduced_pack_arguments;
            pack_arguments_copy = &deduced_pack_arguments_copy;
          }
          if(deduce_template_argument_impl(ctx,
                                           parameters,
                                           pattern,
                                           actual_class.direct_base_types[i],
                                           deduced_types_copy,
                                           deduced_values_copy,
                                           deduction_scope,
                                           partial_top_level_cv_deduction,
                                           actual_lookup_scope,
                                           pack_arguments_copy,
                                           allow_actual_base_deduction)) {
            deduced_types.swap(deduced_types_copy);
            deduced_values.swap(deduced_values_copy);
            if(deduced_pack_arguments) {
              *deduced_pack_arguments = deduced_pack_arguments_copy;
            }
            return true;
          }
        }
        return false;
      };
      Scope & actual_parse_scope = actual_lookup_scope ? *actual_lookup_scope : *deduction_scope;
      QualifiedName pattern_text_template_name;
      std::vector<std::string> pattern_text_template_args;
      const bool pattern_text_is_template_id =
          semantic_utils::split_top_level_template_id_text(normalized_pattern,
                                                           pattern_text_template_name,
                                                           pattern_text_template_args);
      DecomposedTemplateInstantiation pattern_instantiation;
      DecomposedTemplateInstantiation actual_instantiation;
      const bool pattern_decomposed =
          decompose_template_instantiation(
              ctx, *deduction_scope, pattern_base, pattern_instantiation);
      const bool actual_decomposed =
          decompose_template_instantiation(
              ctx, actual_parse_scope, actual_base, actual_instantiation);
      if(!pattern_decomposed && pattern_text_is_template_id) {
        pattern_instantiation.name = pattern_text_template_name;
        pattern_instantiation.argument_texts = pattern_text_template_args;
      }
      const bool pattern_template_id_available =
          pattern_decomposed || pattern_text_is_template_id;
      const TemplateParameterInfo * pattern_template_template_parameter = nullptr;
      if(pattern_template_id_available) {
        const std::string pattern_template_name =
            qualified_name_text(pattern_instantiation.name);
        pattern_template_template_parameter =
            find_template_parameter_for_text(parameters, pattern_template_name);
        if(!pattern_template_template_parameter &&
           pattern_template_name != pattern_instantiation.name.name) {
          pattern_template_template_parameter =
              find_template_parameter_for_text(parameters,
                                               pattern_instantiation.name.name);
        }
        if(pattern_template_template_parameter &&
           pattern_template_template_parameter->kind !=
               TemplateParameterInfo::TP_TEMPLATE_TEMPLATE) {
          pattern_template_template_parameter = nullptr;
        }
      }
      const bool parsed_matching_template_ids =
          pattern_decomposed &&
          actual_decomposed &&
          template_instantiations_match(pattern_instantiation, actual_instantiation);
      const bool parsed_template_template_deduction =
          pattern_template_template_parameter &&
          pattern_template_id_available &&
          actual_decomposed &&
          actual_instantiation.source_template &&
          (pattern_template_template_parameter->template_parameter_count == 0 ||
           pattern_template_template_parameter->template_parameter_count ==
               actual_instantiation.source_template->parameters.size());
      if(pattern_template_template_parameter &&
         pattern_template_id_available &&
         !parsed_template_template_deduction) {
        return false;
      }
      if(parsed_matching_template_ids || parsed_template_template_deduction) {
        DeducedTypeMap saved_deduced_types;
        DeducedValueMap saved_deduced_values;
        DeducedPackArgumentMap saved_deduced_pack_arguments;
        clone_deduced_type_map(deduced_types, saved_deduced_types);
        clone_deduced_value_map(deduced_values, saved_deduced_values);
        if(deduced_pack_arguments) {
          saved_deduced_pack_arguments = *deduced_pack_arguments;
        }
        const auto match_parsed_template_ids = [&]() -> bool
        {
        const std::vector<std::string> & pattern_args =
            pattern_instantiation.argument_texts;
        const std::vector<std::string> & actual_args =
            actual_instantiation.argument_texts;
        std::vector<TemplateArgument> pattern_explicit_structured_args_storage;
        const std::vector<TemplateArgument> * pattern_structured_args =
            pattern_instantiation.source_template &&
                    align_explicit_template_arguments(
                        pattern_instantiation.source_template->parameters,
                        pattern_instantiation.arguments,
                        pattern_args,
                        pattern_explicit_structured_args_storage) ?
                &pattern_explicit_structured_args_storage :
            pattern_instantiation.arguments.size() == pattern_args.size() ?
                &pattern_instantiation.arguments :
                nullptr;
        const ClassTemplateDecl * actual_template_decl =
            actual_instantiation.source_template;
        if(parsed_template_template_deduction) {
          TemplateArgument actual_template_argument;
          actual_template_argument.kind = TemplateArgument::TA_CLASS_TEMPLATE;
          actual_template_argument.template_decl =
              actual_instantiation.source_template;
          actual_template_argument.text = qualified_name_text(actual_instantiation.name);
          if(actual_template_argument.text.empty()) {
            actual_template_argument.text = actual_instantiation.source_template->name;
          }
          if(!record_deduced_template_pack_arguments(
                 *pattern_template_template_parameter,
                 std::vector<TemplateArgument>(1, actual_template_argument))) {
            return false;
          }
        }
        if(actual_template_decl) {
          if(pattern_instantiation.arguments.size() ==
                 actual_instantiation.arguments.size() &&
             (pattern_args.size() != actual_args.size() ||
              pattern_instantiation.arguments.size() != pattern_args.size()) &&
             template_arguments_are_all_types(pattern_instantiation.arguments) &&
             template_arguments_are_all_types(actual_instantiation.arguments)) {
            return deduce_from_effective_type_template_arguments(
                pattern_instantiation.arguments,
                actual_instantiation.arguments);
          }
        }
        std::vector<TemplateArgument> actual_explicit_structured_args_storage;
        const std::vector<TemplateArgument> * actual_structured_args =
            actual_template_decl &&
                    align_explicit_template_arguments(actual_template_decl->parameters,
                                                      actual_instantiation.arguments,
                                                      actual_args,
                                                      actual_explicit_structured_args_storage) ?
                &actual_explicit_structured_args_storage :
            actual_instantiation.arguments.size() == actual_args.size() ?
                &actual_instantiation.arguments :
                nullptr;
        std::vector<std::string> actual_effective_args_storage;
        const std::vector<std::string> * actual_match_args = &actual_args;
        const std::vector<TemplateArgument> * actual_match_structured_args =
            actual_structured_args;
        if(pattern_args.size() != actual_args.size() &&
           actual_instantiation.arguments.size() == pattern_args.size()) {
          actual_effective_args_storage.reserve(actual_instantiation.arguments.size());
          for(std::size_t arg_i = 0;
              arg_i < actual_instantiation.arguments.size();
              ++arg_i) {
            actual_effective_args_storage.push_back(
                trim_space(template_model::template_argument_text(
                    actual_instantiation.arguments[arg_i],
                    [&](const TypePtr & type) -> std::string
                    {
                      return deduction_ops.type_argument_text(type);
                    })));
          }
          actual_match_args = &actual_effective_args_storage;
          actual_match_structured_args = &actual_instantiation.arguments;
        }
        std::size_t direct_pack_count = 0;
        for(std::size_t i = 0; i < pattern_args.size(); ++i) {
          const DirectTemplateParameterMatch direct_match =
              find_direct_template_parameter_from_arg(pattern_args[i]);
          if(direct_match.parameter &&
             direct_match.pack_expansion &&
             direct_match.parameter->parameter_pack) {
            ++direct_pack_count;
          }
        }
        if(direct_pack_count > 1) {
          return false;
        }
        const std::vector<std::string> & actual_args_for_match =
            *actual_match_args;
        if(direct_pack_count == 0 &&
           pattern_args.size() != actual_args_for_match.size()) {
          return false;
        }
        std::size_t actual_index = 0;
        for(std::size_t i = 0; i < pattern_args.size(); ++i) {
          const std::string pattern_arg = trim_space(pattern_args[i]);
          const DirectTemplateParameterMatch direct_match =
              find_direct_template_parameter_from_arg(pattern_arg);
          if(direct_match.parameter &&
             direct_match.pack_expansion &&
             direct_match.parameter->parameter_pack) {
            const std::size_t trailing_non_pack = pattern_args.size() - i - 1;
            if(actual_args_for_match.size() < actual_index + trailing_non_pack) {
              return false;
            }
            const std::size_t pack_count =
                actual_args_for_match.size() - actual_index - trailing_non_pack;
            std::vector<TemplateArgument> actual_pack_arguments;
            actual_pack_arguments.reserve(pack_count);
            for(std::size_t j = 0; j < pack_count; ++j) {
              const TemplateArgument * structured_actual =
                  actual_match_structured_args ?
                      &(*actual_match_structured_args)[actual_index + j] :
                      nullptr;
              TemplateArgument actual_pack_argument;
              if(!resolve_direct_actual_template_argument(
                     *direct_match.parameter,
                     trim_space(actual_args_for_match[actual_index + j]),
                     structured_actual,
                     actual_pack_argument)) {
                return false;
              }
              actual_pack_arguments.push_back(actual_pack_argument);
            }
            if(direct_match.parameter->kind == TemplateParameterInfo::TP_TYPE) {
              std::vector<TypePtr> actual_pack_types;
              actual_pack_types.reserve(actual_pack_arguments.size());
              for(std::size_t j = 0; j < actual_pack_arguments.size(); ++j) {
                actual_pack_types.push_back(actual_pack_arguments[j].type);
              }
              if(!record_deduced_type_pack_arguments(*direct_match.parameter,
                                                     actual_pack_types)) {
                return false;
              }
            } else if(!record_deduced_template_pack_arguments(*direct_match.parameter,
                                                              actual_pack_arguments)) {
              return false;
            }
            actual_index += pack_count;
            continue;
          }

          if(actual_index >= actual_args_for_match.size()) {
            return false;
          }
          const std::string actual_arg = trim_space(actual_args_for_match[actual_index]);
          const TemplateArgument * structured_pattern =
              pattern_structured_args ? &(*pattern_structured_args)[i] : nullptr;
          const TemplateArgument * structured_actual =
              actual_match_structured_args ?
                  &(*actual_match_structured_args)[actual_index] :
                  nullptr;
          if(direct_match.parameter) {
            TemplateArgument actual_direct_argument;
            if(!resolve_direct_actual_template_argument(*direct_match.parameter,
                                                        actual_arg,
                                                        structured_actual,
                                                        actual_direct_argument)) {
              return false;
            }
            if(direct_match.parameter->parameter_pack) {
              std::vector<TemplateArgument> actual_pack_arguments(1,
                                                                  actual_direct_argument);
              if(!record_deduced_template_pack_arguments(*direct_match.parameter,
                                                          actual_pack_arguments)) {
                return false;
              }
            } else if(direct_match.parameter->kind == TemplateParameterInfo::TP_TYPE) {
              DeducedTypeMap::iterator found =
                  deduced_types.find(direct_match.parameter->name);
              if(found == deduced_types.end()) {
                deduced_types[direct_match.parameter->name] = actual_direct_argument.type;
              } else if(!type_equals(found->second, actual_direct_argument.type)) {
                return false;
              }
            } else if(!record_deduced_non_type_value(parameters,
                                                     direct_match.parameter->name,
                                                     actual_direct_argument.value,
                                                     deduced_values)) {
              return false;
            }
            ++actual_index;
            continue;
          }

          if(pattern_arg == actual_arg) {
            ++actual_index;
            continue;
          }

          if(structured_pattern &&
             structured_pattern->kind == TemplateArgument::TA_TYPE &&
             structured_pattern->type) {
            TypePtr actual_arg_type =
                structured_actual &&
                structured_actual->kind == TemplateArgument::TA_TYPE ?
                    structured_actual->type :
                    TypePtr();
            if(!actual_arg_type &&
               !lookup_actual_arg_type(actual_arg, actual_arg_type)) {
              return false;
            }
            if(!deduce_template_argument_impl(ctx,
                                              parameters,
                                              structured_pattern->type,
                                              actual_arg_type,
                                              deduced_types,
                                              deduced_values,
                                              deduction_scope,
                                              partial_top_level_cv_deduction,
                                              actual_lookup_scope,
                                              deduced_pack_arguments,
                                              allow_actual_base_deduction)) {
              return false;
            }
            ++actual_index;
            continue;
          }

          if(deduce_template_argument_text_shape(pattern_arg, actual_arg)) {
            ++actual_index;
            continue;
          }
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "deduce-template-arg-text-shape-fail pattern="
                  << pattern_arg
                  << " actual=" << actual_arg;
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          return false;
        }
        if(actual_index != actual_args_for_match.size()) {
          return false;
        }
        return true;
        };
        if(match_parsed_template_ids()) {
          return true;
        }
        deduced_types.swap(saved_deduced_types);
        deduced_values.swap(saved_deduced_values);
        if(deduced_pack_arguments) {
          *deduced_pack_arguments = saved_deduced_pack_arguments;
        }
        return try_actual_base_deduction();
      }

      if(!pattern_head.empty() &&
         !actual_head.empty() &&
         pattern_head != actual_head) {
        if(semantic_hotspot::enabled()) {
          std::ostringstream query;
          query << pattern_head << " vs " << actual_head;
          semantic_hotspot::note_semantic_query("deduce_named_type_head_mismatch",
                                                query.str());
        }
        if(ensure_actual_class_for_base_deduction()) {
          for(std::size_t i = 0; i < actual_class.direct_base_types.size(); ++i) {
            DeducedTypeMap deduced_types_copy;
            DeducedValueMap deduced_values_copy;
            clone_deduced_type_map(deduced_types, deduced_types_copy);
            clone_deduced_value_map(deduced_values, deduced_values_copy);
            if(deduce_template_argument_impl(ctx,
                                             parameters,
                                             pattern,
                                             actual_class.direct_base_types[i],
                                             deduced_types_copy,
                                             deduced_values_copy,
                                             deduction_scope,
                                             partial_top_level_cv_deduction,
                                             actual_lookup_scope,
                                             deduced_pack_arguments,
                                             allow_actual_base_deduction)) {
              deduced_types.swap(deduced_types_copy);
              deduced_values.swap(deduced_values_copy);
              return true;
            }
          }
        }
        return false;
      }

      if(ensure_actual_class_for_base_deduction()) {
        for(std::size_t i = 0; i < actual_class.direct_base_types.size(); ++i) {
          DeducedTypeMap deduced_types_copy;
          DeducedValueMap deduced_values_copy;
          clone_deduced_type_map(deduced_types, deduced_types_copy);
          clone_deduced_value_map(deduced_values, deduced_values_copy);
          if(deduce_template_argument_impl(ctx,
                                           parameters,
                                           pattern,
                                           actual_class.direct_base_types[i],
                                           deduced_types_copy,
                                           deduced_values_copy,
                                           deduction_scope,
                                           partial_top_level_cv_deduction,
                                           actual_lookup_scope,
                                           deduced_pack_arguments,
                                           allow_actual_base_deduction)) {
            deduced_types.swap(deduced_types_copy);
            deduced_values.swap(deduced_values_copy);
            return true;
          }
        }
      }
    }
  }

  if(pattern_base->kind != actual_base->kind) {
    return false;
  }

  switch(pattern_base->kind) {
  case Type::TK_FUNDAMENTAL:
  case Type::TK_NAMED:
    return type_equals(pattern, actual) ||
           type_equals(pattern_base, actual_base);

  case Type::TK_CV:
    return deduce_template_argument_impl(ctx,
                                         parameters,
                                         pattern_base->inner,
                                         actual_base->inner,
                                         deduced_types,
                                         deduced_values,
                                         deduction_scope,
                                         partial_top_level_cv_deduction,
                                         actual_lookup_scope,
                                         deduced_pack_arguments,
                                         allow_actual_base_deduction);

  case Type::TK_ATOMIC:
    return deduce_template_argument_impl(ctx,
                                         parameters,
                                         pattern_base->inner,
                                         actual_base->inner,
                                         deduced_types,
                                         deduced_values,
                                         deduction_scope,
                                         partial_top_level_cv_deduction,
                                         actual_lookup_scope,
                                         deduced_pack_arguments,
                                         allow_actual_base_deduction);

  case Type::TK_POINTER:
  case Type::TK_MEMBER_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    if(pattern_base->kind == Type::TK_MEMBER_POINTER &&
       !deduce_template_argument_impl(ctx,
                                      parameters,
                                      pattern_base->owner,
                                      actual_base->owner,
                                      deduced_types,
                                      deduced_values,
                                      deduction_scope,
                                      partial_top_level_cv_deduction,
                                      actual_lookup_scope,
                                      deduced_pack_arguments,
                                      allow_actual_base_deduction)) {
      return false;
    }
    return deduce_template_argument_impl(ctx,
                                         parameters,
                                         pattern_base->inner,
                                         actual_base->inner,
                                         deduced_types,
                                         deduced_values,
                                         deduction_scope,
                                         partial_top_level_cv_deduction,
                                         actual_lookup_scope,
                                         deduced_pack_arguments,
                                         allow_actual_base_deduction);

  case Type::TK_ARRAY:
    if(pattern_base->has_bound) {
      if(!actual_base->has_bound || pattern_base->bound != actual_base->bound) {
        return false;
      }
    } else if(!pattern_base->bound_text.empty()) {
      if(!actual_base->has_bound ||
         !record_deduced_non_type_value(parameters,
                                        pattern_base->bound_text,
                                        static_cast<long long>(actual_base->bound),
                                        deduced_values)) {
        return false;
      }
    }
    return deduce_template_argument_impl(ctx,
                                         parameters,
                                         pattern_base->inner,
                                         actual_base->inner,
                                         deduced_types,
                                         deduced_values,
                                         deduction_scope,
                                         partial_top_level_cv_deduction,
                                         actual_lookup_scope,
                                         deduced_pack_arguments,
                                         allow_actual_base_deduction);

  case Type::TK_FUNCTION:
    if(pattern_base->variadic != actual_base->variadic ||
       pattern_base->prototype_relaxed != actual_base->prototype_relaxed ||
       pattern_base->function_const != actual_base->function_const ||
       pattern_base->function_volatile != actual_base->function_volatile ||
       !deduce_template_argument_impl(ctx,
                                      parameters,
                                      pattern_base->inner,
                                      actual_base->inner,
                                      deduced_types,
                                      deduced_values,
                                      deduction_scope,
                                      partial_top_level_cv_deduction,
                                      actual_lookup_scope,
                                      deduced_pack_arguments,
                                      allow_actual_base_deduction)) {
      return false;
    }
    if(!pattern_base->params.empty()) {
      const TemplateParameterInfo * trailing_type_pack =
          direct_type_parameter_pack_pattern(parameters, pattern_base->params.back());
      if(trailing_type_pack) {
        const std::size_t fixed_param_count = pattern_base->params.size() - 1;
        if(actual_base->params.size() < fixed_param_count) {
          return false;
        }
        for(std::size_t i = 0; i < fixed_param_count; ++i) {
          if(!deduce_template_argument_impl(ctx,
                                            parameters,
                                            pattern_base->params[i],
                                            actual_base->params[i],
                                            deduced_types,
                                            deduced_values,
                                            deduction_scope,
                                            partial_top_level_cv_deduction,
                                            actual_lookup_scope,
                                            deduced_pack_arguments,
                                            allow_actual_base_deduction)) {
            return false;
          }
        }
        std::vector<TypePtr> pack_argument_types;
        pack_argument_types.reserve(actual_base->params.size() - fixed_param_count);
        for(std::size_t i = fixed_param_count; i < actual_base->params.size(); ++i) {
          pack_argument_types.push_back(actual_base->params[i]);
        }
        return record_deduced_type_pack_arguments_for_deduction(
            deduction_ops,
            *trailing_type_pack,
            pack_argument_types,
            deduced_pack_arguments);
      }
    }
    if(pattern_base->params.size() != actual_base->params.size()) {
      return false;
    }
    for(std::size_t i = 0; i < pattern_base->params.size(); ++i) {
      if(!deduce_template_argument_impl(ctx,
                                        parameters,
                                        pattern_base->params[i],
                                        actual_base->params[i],
                                        deduced_types,
                                        deduced_values,
                                        deduction_scope,
                                        partial_top_level_cv_deduction,
                                        actual_lookup_scope,
                                        deduced_pack_arguments,
                                        allow_actual_base_deduction)) {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool deduce_template_argument(SemanticContext & ctx,
                              const std::vector<TemplateParameterInfo> & parameters,
                              const TypePtr & pattern,
                              const TypePtr & actual,
                              std::map<std::string, TypePtr> & deduced,
                              Scope * deduction_scope,
                              bool partial_top_level_cv_deduction,
                              Scope * actual_lookup_scope,
                              bool allow_actual_base_deduction)
{
  DeducedValueMap deduced_values;
  return deduce_template_argument_impl(ctx,
                                       parameters,
                                       pattern,
                                       actual,
                                       deduced,
                                       deduced_values,
                                       deduction_scope,
                                       partial_top_level_cv_deduction,
                                       actual_lookup_scope,
                                       nullptr,
                                       allow_actual_base_deduction);
}

bool deduce_template_argument(
    template_api::TemplateServices & services,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern,
    const TypePtr & actual,
    std::map<std::string, TypePtr> & deduced,
    template_api::TemplateEnvironmentHandle deduction_scope,
    bool partial_top_level_cv_deduction,
    template_api::TemplateEnvironmentHandle actual_lookup_scope,
    bool allow_actual_base_deduction)
{
  DeducedValueMap deduced_values;
  return deduce_template_argument_impl(services,
                                       parameters,
                                       pattern,
                                       actual,
                                       deduced,
                                       deduced_values,
                                       deduction_scope.scope,
                                       partial_top_level_cv_deduction,
                                       actual_lookup_scope.scope,
                                       nullptr,
                                       allow_actual_base_deduction);
}

bool deduce_function_template_arguments(SemanticContext & ctx,
                                        FunctionTemplateDecl & decl,
                                        const std::vector<ExprInfo> & args,
                                        std::vector<TemplateArgument> & out,
                                        Scope * use_scope,
                                        std::map<std::string, std::size_t> * pack_sizes_out);

bool deduce_initializer_list_pattern_from_list_like_argument(
    SemanticContext & ctx,
    const std::vector<TemplateParameterInfo> & parameters,
    const TypePtr & pattern,
    const TypePtr & actual,
    DeducedTypeMap & deduced_types,
    DeducedValueMap & deduced_values,
    Scope * deduction_scope,
    DeducedPackArgumentMap * deduced_pack_arguments)
{
  if(!pattern || !actual) {
    return false;
  }

  TypePtr pattern_base = strip_top_level_cv(remove_reference_type(strip_top_level_cv(pattern)));
  TypePtr pattern_element;
  if(!pattern_base ||
     !ctx.is_initializer_list_type(pattern_base, &pattern_element, nullptr) ||
     !pattern_element) {
    return false;
  }

  TypePtr actual_base = strip_top_level_cv(remove_reference_type(strip_top_level_cv(actual)));
  if(!actual_base ||
     (actual_base->kind != Type::TK_ARRAY && actual_base->kind != Type::TK_POINTER) ||
     !actual_base->inner) {
    return false;
  }

  return deduce_template_argument_impl(ctx,
                                       parameters,
                                       pattern_element,
                                       actual_base->inner,
                                       deduced_types,
                                       deduced_values,
                                       deduction_scope,
                                       false,
                                       nullptr,
                                       deduced_pack_arguments);
}

bool deduce_function_template_target_function_type_with_trailing_pack(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & bound_scope,
    const TypePtr & pattern,
    const TypePtr & target,
    Scope * actual_lookup_scope,
    DeducedTypeMap & deduced_types,
    DeducedValueMap & deduced_values,
    DeducedPackArgumentMap & deduced_pack_arguments)
{
  if(!uses_trailing_function_parameter_pack(decl)) {
    return false;
  }

  TypePtr pattern_base = strip_top_level_cv(pattern);
  TypePtr target_base = strip_top_level_cv(target);
  if(!pattern_base ||
     !target_base ||
     pattern_base->kind != Type::TK_FUNCTION ||
     target_base->kind != Type::TK_FUNCTION ||
     pattern_base->params.empty()) {
    return false;
  }
  if(pattern_base->variadic != target_base->variadic ||
     pattern_base->prototype_relaxed != target_base->prototype_relaxed ||
     pattern_base->function_const != target_base->function_const ||
     pattern_base->function_volatile != target_base->function_volatile) {
    return false;
  }

  const std::size_t fixed_param_count = pattern_base->params.size() - 1;
  if(target_base->params.size() < fixed_param_count) {
    return false;
  }

  if(!deduce_template_argument_impl(ctx,
                                    decl.parameters,
                                    pattern_base->inner,
                                    target_base->inner,
                                    deduced_types,
                                    deduced_values,
                                    &bound_scope,
                                    false,
                                    actual_lookup_scope,
                                    &deduced_pack_arguments)) {
    return false;
  }

  for(std::size_t i = 0; i < fixed_param_count; ++i) {
    if(!deduce_template_argument_impl(ctx,
                                      decl.parameters,
                                      pattern_base->params[i],
                                      target_base->params[i],
                                      deduced_types,
                                      deduced_values,
                                      &bound_scope,
                                      false,
                                      actual_lookup_scope,
                                      &deduced_pack_arguments)) {
      return false;
    }
  }

  const TypePtr pack_pattern = pattern_base->params.back();
  for(std::size_t i = fixed_param_count; i < target_base->params.size(); ++i) {
    DeducedTypeMap temp_deduced_types;
    DeducedValueMap temp_deduced_values;
    DeducedPackArgumentMap temp_deduced_pack_arguments;
    clone_deduced_type_map(deduced_types, temp_deduced_types);
    clone_deduced_value_map(deduced_values, temp_deduced_values);
    if(!deduce_template_argument_impl(ctx,
                                      decl.parameters,
                                      pack_pattern,
                                      target_base->params[i],
                                      temp_deduced_types,
                                      temp_deduced_values,
                                      &bound_scope,
                                      false,
                                      actual_lookup_scope,
                                      &temp_deduced_pack_arguments)) {
      return false;
    }
    if(!merge_deduced_pack_arguments(ctx,
                                     decl,
                                     bound_scope,
                                     deduced_types,
                                     deduced_values,
                                     temp_deduced_types,
                                     temp_deduced_values,
                                     temp_deduced_pack_arguments,
                                     deduced_pack_arguments)) {
      return false;
    }
  }

  return true;
}

bool deduce_function_template_target_pattern(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & bound_scope,
    const TypePtr & pattern,
    const TypePtr & target,
    Scope * actual_lookup_scope,
    DeducedTypeMap & deduced_types,
    DeducedValueMap & deduced_values,
    DeducedPackArgumentMap & deduced_pack_arguments)
{
  if(!uses_trailing_function_parameter_pack(decl)) {
    return deduce_template_argument_impl(ctx,
                                         decl.parameters,
                                         pattern,
                                         target,
                                         deduced_types,
                                         deduced_values,
                                         &bound_scope,
                                         false,
                                         actual_lookup_scope,
                                         &deduced_pack_arguments);
  }

  DeducedTypeMap working_types;
  DeducedValueMap working_values;
  DeducedPackArgumentMap working_pack_arguments = deduced_pack_arguments;
  clone_deduced_type_map(deduced_types, working_types);
  clone_deduced_value_map(deduced_values, working_values);

  if(deduce_template_argument_impl(ctx,
                                   decl.parameters,
                                   pattern,
                                   target,
                                   working_types,
                                   working_values,
                                   &bound_scope,
                                   false,
                                   actual_lookup_scope,
                                   &working_pack_arguments)) {
    deduced_types.swap(working_types);
    deduced_values.swap(working_values);
    deduced_pack_arguments.swap(working_pack_arguments);
    return true;
  }

  working_pack_arguments = deduced_pack_arguments;
  clone_deduced_type_map(deduced_types, working_types);
  clone_deduced_value_map(deduced_values, working_values);
  if(deduce_function_template_target_function_type_with_trailing_pack(
         ctx,
         decl,
         bound_scope,
         pattern,
         target,
         actual_lookup_scope,
         working_types,
         working_values,
         working_pack_arguments)) {
    deduced_types.swap(working_types);
    deduced_values.swap(working_values);
    deduced_pack_arguments.swap(working_pack_arguments);
    return true;
  }

  return false;
}

bool deduce_function_template_arguments_uncached(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const std::vector<ExprInfo> & args,
    std::vector<TemplateArgument> & out,
    Scope * use_scope,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  DIAG_CONTEXT("deduce_function_template_arguments [" + decl.name +
               ", args=" + std::to_string(args.size()) + "]");
  if(semantic_hotspot::enabled()) {
    std::ostringstream query;
    query << decl.name << "(";
    for(std::size_t i = 0; i < args.size(); ++i) {
      if(i != 0) {
        query << ", ";
      }
      query << (args[i].type ? describe_type(args[i].type) : std::string("<null>"))
            << "/" << static_cast<int>(args[i].category);
    }
    query << ")";
    semantic_hotspot::note_semantic_query("deduce_function_template_arguments", query.str());
    std::ostringstream candidate_query;
    candidate_query << decl.name << " signature={"
                    << (decl.debug_signature.empty() ?
                            semantic_trace::function_template_signature_for_diagnostic(decl) :
                            decl.debug_signature)
                    << "} args=" << args.size()
                    << " patterns=[";
    for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
      if(i != 0) {
        candidate_query << "; ";
      }
      candidate_query << (decl.params_pattern[i].second ?
                              describe_type(decl.params_pattern[i].second) :
                              std::string("<null>"));
    }
    candidate_query << "]";
    semantic_hotspot::note_semantic_query("deduce_function_template_candidate",
                                          candidate_query.str());
  }
  try {
    out.clear();
    if(decl.parameters.empty() ||
       !function_template_accepts_argument_count(decl, args.size())) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "deduce-entry-rejected template=" << decl.name
              << " parameter_count=" << decl.parameters.size()
              << " params_pattern_count=" << decl.params_pattern.size()
              << " default_argument_count=" << decl.default_arguments_pattern.size()
              << " arg_count=" << args.size()
              << " trailing_pack="
              << (uses_trailing_function_parameter_pack(decl) ? "yes" : "no");
        parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
      }
      return false;
    }

    DeducedTypeMap deduced_types;
    DeducedValueMap deduced_values;
    DeducedPackArgumentMap deduced_pack_arguments;
    const auto summarize_deductions =
        [&](const DeducedTypeMap & types,
            const DeducedValueMap & values,
            const DeducedPackArgumentMap & packs) -> std::string
    {
      std::ostringstream summary;
      summary << "types={";
      bool first = true;
      for(DeducedTypeMap::const_iterator it = types.begin(); it != types.end(); ++it) {
        if(!first) {
          summary << ",";
        }
        first = false;
        summary << it->first << ":"
                << (it->second ? describe_type(it->second) : std::string("<null>"));
      }
      summary << "} values={";
      first = true;
      for(DeducedValueMap::const_iterator it = values.begin(); it != values.end(); ++it) {
        if(!first) {
          summary << ",";
        }
        first = false;
        summary << it->first << ":" << it->second;
      }
      summary << "} packs={";
      first = true;
      for(DeducedPackArgumentMap::const_iterator it = packs.begin(); it != packs.end(); ++it) {
        if(!first) {
          summary << ",";
        }
        first = false;
        summary << it->first << ":" << it->second.size();
      }
      summary << "}";
      return summary.str();
    };
    Scope bound_scope(decl.declaring_scope, "", false);
    const std::set<std::string> excluded_parameter_names =
        deduction_overlay_excluded_names(decl);
    const std::vector<TemplateArgument> local_type_arguments =
        use_scope ? deduction_scope_local_type_arguments(ctx, args) :
                    std::vector<TemplateArgument>();
    trace_function_template_drift("deduce-entry", decl);
    if(use_scope) {
      template_instantiation::overlay_instantiation_use_scope_bindings(
          bound_scope,
          *use_scope,
          decl.declaring_scope,
          excluded_parameter_names);
      template_api::overlay_instantiation_local_named_types(ctx,
                                                            bound_scope,
                                                            *use_scope,
                                                            decl.declaring_scope,
                                                            local_type_arguments,
                                                            &excluded_parameter_names);
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "deduce-bound-scope"
              << " template=" << decl.name
              << " use_scope=" << semantic_trace::scope_name_for_diagnostic(*use_scope)
              << " use_bindings=" << semantic_trace::scope_bindings_for_diagnostic(*use_scope)
              << " bound_bindings=" << semantic_trace::scope_bindings_for_diagnostic(bound_scope);
        parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
      }
    }
    trace_template_parameter_bindings("pre-shadow-cleanup", decl, bound_scope, use_scope);
    if(debug_skip_template_parameter_shadow_cleanup(decl)) {
      if(parser_trace::enabled("template.resolve")) {
        parser_trace::note("template.resolve",
                           decl.debug_decl_location,
                           std::string("skip-shadow-cleanup template=") + decl.name);
      }
    } else {
      remove_shadowing_template_parameter_bindings(bound_scope, decl.parameters);
    }
    trace_template_parameter_bindings("post-shadow-cleanup", decl, bound_scope, use_scope);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    Scope initial_bound_scope = bound_scope;
    const std::size_t deduction_count =
        function_template_deduction_parameter_count(decl, args.size());
    for(std::size_t i = 0; i < deduction_count; ++i) {
      bind_known_deductions_into_scope(
          ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
      bind_resolvable_default_non_type_template_arguments_into_scope(
          ctx, bound_scope, decl.parameters);
      const bool deducing_pack_element =
          uses_trailing_function_parameter_pack(decl) &&
          !decl.params_pattern.empty() &&
          i + 1 >= decl.params_pattern.size();
      const std::size_t pattern_index =
          deducing_pack_element && !decl.params_pattern.empty() ?
              decl.params_pattern.size() - 1 :
              i;
      const TypePtr original_pattern = decl.params_pattern[pattern_index].second;
      TypePtr pattern = prepare_function_template_deduction_pattern(
          ctx, decl.parameters, bound_scope, original_pattern);
      TypePtr actual = remove_reference_type(args[i].type);
      if(!actual) {
        actual = args[i].type;
      }
      apply_function_template_call_deduction_adjustments(decl, args[i], pattern, actual);
      TypePtr cv_inner;
      TypePtr actual_cv_inner;
      bool pattern_const = false;
      bool pattern_volatile = false;
      bool actual_const = false;
      bool actual_volatile = false;
      if(top_level_cv_flags(pattern, cv_inner, pattern_const, pattern_volatile) &&
         top_level_cv_flags(actual,
                            actual_cv_inner,
                            actual_const,
                            actual_volatile) &&
         ((pattern_const && actual_const) ||
          (pattern_volatile && actual_volatile))) {
        pattern = cv_inner;
        actual = actual_cv_inner;
      }
      const bool original_pattern_participates_in_deduction =
          type_mentions_unbound_function_template_parameter(
              ctx, decl.parameters, initial_bound_scope, original_pattern);
      if(!type_mentions_unbound_function_template_parameter(
             ctx, decl.parameters, bound_scope, pattern) &&
         !original_pattern_participates_in_deduction) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "skip-nondependent-pattern template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }
      if(argument_is_braced_init_list_for_deduction(args[i]) &&
         !deduction_pattern_accepts_braced_init_list_argument(ctx, pattern)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "deduction-braced-init-rejected template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern)
                << " actual=" << describe_type(actual);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return false;
      }
      if(can_skip_resolved_non_dependent_pattern_check(
             ctx, bound_scope, decl, pattern_index, original_pattern, pattern)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "skip-resolved-nondependent template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }
      DeducedTypeMap temp_deduced_types;
      DeducedValueMap temp_deduced_values;
      DeducedPackArgumentMap temp_deduced_pack_arguments;
      clone_deduced_type_map(deduced_types, temp_deduced_types);
      clone_deduced_value_map(deduced_values, temp_deduced_values);
      bool recovered_alias_pattern_deduction = false;
      DeducedPackArgumentMap * const deduction_pack_arguments =
          deducing_pack_element ? &temp_deduced_pack_arguments :
                                  &deduced_pack_arguments;
      bool deduced_argument =
          deduce_initializer_list_pattern_from_list_like_argument(ctx,
                                                                 decl.parameters,
                                                                 pattern,
                                                                 actual,
                                                                 temp_deduced_types,
                                                                 temp_deduced_values,
                                                                 &bound_scope,
                                                                 deduction_pack_arguments);
      if(!deduced_argument) {
        deduced_argument =
            deduce_template_argument_impl(ctx,
                                          decl.parameters,
                                          pattern,
                                          actual,
                                          temp_deduced_types,
                                          temp_deduced_values,
                                          &bound_scope,
                                          false,
                                          nullptr,
                                          deduction_pack_arguments);
      }
      if(!deduced_argument) {
        TypePtr original_base = strip_top_level_cv(original_pattern);
        if(original_base && original_base->kind == Type::TK_NAMED) {
          void * dependent_alias_template_decl = nullptr;
          std::vector<DependentAliasTemplateArgumentSyntax> dependent_alias_args;
          const bool have_dependent_alias_args =
              named_type_dependent_alias_template(original_base,
                                                  dependent_alias_template_decl,
                                                  dependent_alias_args);
          const std::string normalized_text = strip_elaborated_type_prefix(
              trim_space(type_argument_text_for_deduction(ctx, original_base)));
          QualifiedName parsed_name;
          if(template_id_head_name_from_type_text(normalized_text, parsed_name)) {
            AliasTemplateDecl * alias_decl =
                ((!parsed_name.rooted && parsed_name.qualifiers.empty()) ?
                     semantic_lookup::lookup_unqualified_alias_template(
                         bound_scope, parsed_name.name) :
                     nullptr);
            if(!alias_decl) {
              alias_decl = ctx.lookup_alias_template(bound_scope, parsed_name);
            }
            if(alias_decl) {
              QualifiedName alias_name;
              std::vector<std::string> alias_arg_texts;
              std::vector<TemplateArgumentSyntax> alias_arg_syntaxes;
              const std::vector<TemplateArgumentSyntax> * alias_arg_syntaxes_ptr =
                  nullptr;
              if(!have_dependent_alias_args ||
                 dependent_alias_template_decl != alias_decl) {
                continue;
              }
              alias_arg_texts.reserve(dependent_alias_args.size());
              alias_arg_syntaxes.reserve(dependent_alias_args.size());
              for(std::size_t arg_index = 0;
                  arg_index < dependent_alias_args.size();
                  ++arg_index) {
                alias_arg_texts.push_back(dependent_alias_args[arg_index].text);
                alias_arg_syntaxes.push_back(dependent_alias_args[arg_index].syntax);
              }
              alias_arg_syntaxes_ptr = &alias_arg_syntaxes;
              alias_name = parsed_name;
              TypePtr expanded_pattern;
              const auto expand_alias_pattern =
                  [&](template_api::TemplateServices & services) -> bool
              {
                return template_specialization::expand_alias_template_pattern_type(
                    services,
                    template_api::make_template_environment(bound_scope),
                    alias_name,
                    alias_arg_texts,
                    expanded_pattern,
                    alias_arg_syntaxes_ptr,
                    template_api::make_template_environment(bound_scope),
                    true);
              };
              const bool expanded_alias_pattern =
                  template_api::with_template_services(ctx, expand_alias_pattern);
              if(expanded_alias_pattern &&
                 expanded_pattern &&
                 !type_equals(expanded_pattern, original_base) &&
                 deduce_template_argument_impl(ctx,
                                               decl.parameters,
                                               expanded_pattern,
                                               actual,
                                               temp_deduced_types,
                                               temp_deduced_values,
                                               &bound_scope,
                                               false,
                                               nullptr,
                                               deduction_pack_arguments)) {
                recovered_alias_pattern_deduction = true;
              } else {
                continue;
              }
            }
          }
        }
        if(!recovered_alias_pattern_deduction &&
           template_argument_semantics::type_depends_on_template_parameter(ctx, pattern) &&
           is_dependent_qualified_nondeduced_type_context(
               ctx, decl.parameters, original_pattern)) {
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "deduction-deferred-nondeduced template=" << decl.name
                  << " param_index=" << pattern_index
                  << " pattern=" << describe_type(pattern)
                  << " actual=" << describe_type(actual);
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          continue;
        }
        if(!recovered_alias_pattern_deduction && parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "deduction-failed template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern)
                << " actual=" << describe_type(actual);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        if(!recovered_alias_pattern_deduction) {
          return false;
        }
      }
      if(deducing_pack_element) {
        if(!merge_deduced_pack_arguments(ctx,
                                         decl,
                                         bound_scope,
                                         deduced_types,
                                         deduced_values,
                                         temp_deduced_types,
                                         temp_deduced_values,
                                         temp_deduced_pack_arguments,
                                         deduced_pack_arguments)) {
          return false;
        }
      } else {
        deduced_types.swap(temp_deduced_types);
        deduced_values.swap(temp_deduced_values);
      }
    }

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    const bool finalized = finalize_deduced_function_template_arguments(
        ctx,
        decl,
        bound_scope,
        deduced_types,
        deduced_values,
        deduced_pack_arguments,
        nullptr,
        out,
        pack_sizes_out);
    if(!finalized && parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "deduce-finalize-failed template=" << decl.name
            << " " << summarize_deductions(deduced_types,
                                            deduced_values,
                                            deduced_pack_arguments);
      parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
    }
    return finalized;
  } catch(const TemplateSubstitutionFailure & e) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "deduce-substitution-failed template=" << decl.name
            << " reason=" << e.what();
      parser_trace::note("template.resolve", decl.debug_decl_location, trace.str());
    }
    return false;
  }
}

bool deduce_function_template_arguments(SemanticContext & ctx,
                                        FunctionTemplateDecl & decl,
                                        const std::vector<ExprInfo> & args,
                                        std::vector<TemplateArgument> & out,
                                        Scope * use_scope,
                                        std::map<std::string, std::size_t> * pack_sizes_out)
{
  semantic_metrics::AnalyzerCounters * counters = ctx.performance_counters();
  if(!function_template_deduction_cache_allowed(ctx, decl, args)) {
    return deduce_function_template_arguments_uncached(
        ctx, decl, args, out, use_scope, pack_sizes_out);
  }

  const FunctionTemplateDeductionCacheKey cache_key =
      make_function_template_deduction_cache_key(decl, args, use_scope, counters);
  typedef std::unordered_map<FunctionTemplateDeductionCacheKey,
                             FunctionTemplateDeductionCacheEntry,
                             FunctionTemplateDeductionCacheKeyHash> Cache;
  Cache & cache = function_template_deduction_cache();
  Cache::const_iterator cached = cache.find(cache_key);
  if(cached != cache.end()) {
    if(counters) {
      ++counters->function_template_deduction_cache_hits;
    }
    out = cached->second.arguments;
    if(pack_sizes_out) {
      *pack_sizes_out = cached->second.pack_sizes;
    }
    return cached->second.success;
  }
  if(counters) {
    ++counters->function_template_deduction_cache_misses;
  }

  std::map<std::string, std::size_t> pack_sizes;
  std::map<std::string, std::size_t> * const effective_pack_sizes =
      pack_sizes_out ? pack_sizes_out : &pack_sizes;
  const bool success =
      deduce_function_template_arguments_uncached(ctx,
                                                  decl,
                                                  args,
                                                  out,
                                                  use_scope,
                                                  effective_pack_sizes);
  note_function_template_deduction_cache_entry(cache_key,
                                               success,
                                               out,
                                               effective_pack_sizes,
                                               counters);
  return success;
}

bool deduce_function_template_arguments_from_target_type(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    const TypePtr & target,
    std::vector<TemplateArgument> & out,
    Scope * use_scope,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  DIAG_CONTEXT("deduce_function_template_arguments_from_target_type [" + decl.name + "]");
  try {
    out.clear();
    if(!target) {
      return false;
    }

    DeducedTypeMap deduced_types;
    DeducedValueMap deduced_values;
    DeducedPackArgumentMap deduced_pack_arguments;
    Scope bound_scope(decl.declaring_scope, "", false);
    const std::set<std::string> excluded_parameter_names =
        deduction_overlay_excluded_names(decl);
    trace_function_template_drift("deduce-target-entry", decl);
    if(use_scope) {
      template_instantiation::overlay_instantiation_use_scope_bindings(
          bound_scope,
          *use_scope,
          decl.declaring_scope,
          excluded_parameter_names);
    }
    remove_shadowing_template_parameter_bindings(bound_scope, decl.parameters);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    TypePtr pattern = prepare_function_template_deduction_pattern(
        ctx, decl.parameters, bound_scope, decl.type_pattern);
    if(!deduce_function_template_target_pattern(ctx,
                                                decl,
                                                bound_scope,
                                                pattern,
                                                target,
                                                use_scope,
                                                deduced_types,
                                                deduced_values,
                                                deduced_pack_arguments)) {
      return false;
    }

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    return finalize_deduced_function_template_arguments(ctx,
                                                        decl,
                                                        bound_scope,
                                                        deduced_types,
                                                        deduced_values,
                                                        deduced_pack_arguments,
                                                        nullptr,
                                                        out,
                                                        pack_sizes_out);
  } catch(const TemplateSubstitutionFailure &) {
    return false;
  }
}

bool deduce_function_template_arguments_from_target_type_with_explicit(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & resolution_scope,
    const std::vector<TemplateArgument> & explicit_arguments,
    const TypePtr & target,
    std::vector<TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  DIAG_CONTEXT("deduce_function_template_arguments_from_target_type_with_explicit [" +
               decl.name + "]");
  try {
    out.clear();
    if(explicit_arguments.empty()) {
      return deduce_function_template_arguments_from_target_type(
          ctx, decl, target, out, &resolution_scope, pack_sizes_out);
    }
    if(!target || decl.parameters.empty()) {
      return false;
    }

    ExplicitFunctionTemplateArgumentBindings explicit_argument_bindings;
    if(!partition_explicit_function_template_arguments(
           decl, explicit_arguments, explicit_argument_bindings)) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "explicit-target-arg-resolution-failed template=" << decl.name
              << " explicit_count=" << explicit_arguments.size();
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }

    std::vector<TemplateParameterInfo> deducible_parameters;
    if(explicit_argument_bindings.pack_parameter_index != static_cast<std::size_t>(-1)) {
      if(explicit_argument_bindings.pack_parameter_index + 1 < decl.parameters.size()) {
        deducible_parameters.assign(
            decl.parameters.begin() + explicit_argument_bindings.pack_parameter_index + 1,
            decl.parameters.end());
      }
    } else if(explicit_argument_bindings.fixed_arguments.size() < decl.parameters.size()) {
      deducible_parameters.assign(
          decl.parameters.begin() + explicit_argument_bindings.fixed_arguments.size(),
          decl.parameters.end());
    }

    DeducedTypeMap deduced_types;
    DeducedValueMap deduced_values;
    DeducedPackArgumentMap deduced_pack_arguments;
    Scope bound_scope(decl.declaring_scope, "", false);
    const std::set<std::string> excluded_parameter_names =
        deduction_overlay_excluded_names(decl);
    trace_function_template_drift("deduce-target-explicit-entry", decl);
    template_instantiation::overlay_instantiation_use_scope_bindings(
        bound_scope,
        resolution_scope,
        decl.declaring_scope,
        excluded_parameter_names);
    remove_shadowing_template_parameter_bindings(bound_scope, decl.parameters);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);

    for(std::size_t i = 0; i < explicit_argument_bindings.fixed_arguments.size(); ++i) {
      const TemplateParameterInfo & parameter = decl.parameters[i];
      const TemplateArgument & argument =
          explicit_argument_bindings.fixed_arguments[i];
      if(parameter.kind == TemplateParameterInfo::TP_TYPE && !parameter.name.empty()) {
        deduced_types[parameter.name] = argument.type;
      }
      if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE &&
         !parameter.name.empty() &&
         argument.kind == TemplateArgument::TA_VALUE &&
         !argument.dependent) {
        deduced_values[parameter.name] = argument.value;
      }
      ctx.bind_single_template_argument_into_scope(bound_scope, parameter, argument);
    }
    if(explicit_argument_bindings.pack_parameter_index != static_cast<std::size_t>(-1)) {
      bind_explicit_function_template_pack_arguments(
          bound_scope,
          decl.parameters[explicit_argument_bindings.pack_parameter_index],
          explicit_argument_bindings.pack_arguments);
    }
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    TypePtr pattern = prepare_function_template_deduction_pattern(
        ctx, decl.parameters, bound_scope, decl.type_pattern);
    if(!deducible_parameters.empty() &&
       type_mentions_unbound_function_template_parameter(
           ctx, deducible_parameters, bound_scope, pattern) &&
       !deduce_function_template_target_pattern(ctx,
                                                decl,
                                                bound_scope,
                                                pattern,
                                                target,
                                                &resolution_scope,
                                                deduced_types,
                                                deduced_values,
                                                deduced_pack_arguments)) {
      return false;
    }

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    return finalize_deduced_function_template_arguments(
        ctx,
        decl,
        bound_scope,
        deduced_types,
        deduced_values,
        deduced_pack_arguments,
        &explicit_argument_bindings,
        out,
        pack_sizes_out);
  } catch(const TemplateSubstitutionFailure &) {
    return false;
  }
}

bool deduce_function_template_arguments_with_explicit(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    Scope & resolution_scope,
    const std::vector<TemplateArgument> & explicit_arguments,
    const std::vector<ExprInfo> & args,
    std::vector<TemplateArgument> & out,
    std::map<std::string, std::size_t> * pack_sizes_out)
{
  try {
    out.clear();
    if(explicit_arguments.empty()) {
      return deduce_function_template_arguments(
          ctx, decl, args, out, &resolution_scope, pack_sizes_out);
    }
    if(decl.parameters.empty() ||
       !function_template_accepts_argument_count(decl, args.size())) {
      return false;
    }

    ExplicitFunctionTemplateArgumentBindings explicit_argument_bindings;
    if(!partition_explicit_function_template_arguments(
           decl, explicit_arguments, explicit_argument_bindings)) {
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "explicit-arg-resolution-failed template=" << decl.name
              << " explicit_count=" << explicit_arguments.size();
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      return false;
    }
    std::vector<TemplateParameterInfo> deducible_parameters;
    if(explicit_argument_bindings.pack_parameter_index != static_cast<std::size_t>(-1)) {
      if(explicit_argument_bindings.pack_parameter_index + 1 < decl.parameters.size()) {
        deducible_parameters.assign(
            decl.parameters.begin() + explicit_argument_bindings.pack_parameter_index + 1,
            decl.parameters.end());
      }
    } else if(explicit_argument_bindings.fixed_arguments.size() < decl.parameters.size()) {
      deducible_parameters.assign(
          decl.parameters.begin() + explicit_argument_bindings.fixed_arguments.size(),
          decl.parameters.end());
    }

    DeducedTypeMap deduced_types;
    DeducedValueMap deduced_values;
    DeducedPackArgumentMap deduced_pack_arguments;
    Scope bound_scope(decl.declaring_scope, "", false);
    const std::set<std::string> excluded_parameter_names =
        deduction_overlay_excluded_names(decl);
    const std::vector<TemplateArgument> local_type_arguments =
        deduction_scope_local_type_arguments(ctx, args);
    template_instantiation::overlay_instantiation_use_scope_bindings(
        bound_scope,
        resolution_scope,
        decl.declaring_scope,
        excluded_parameter_names);
    template_api::overlay_instantiation_local_named_types(ctx,
                                                          bound_scope,
                                                          resolution_scope,
                                                          decl.declaring_scope,
                                                          local_type_arguments,
                                                          &excluded_parameter_names);
    if(debug_skip_template_parameter_shadow_cleanup(decl)) {
      if(parser_trace::enabled("template.resolve")) {
        parser_trace::note("template.resolve",
                           decl.debug_decl_location,
                           std::string("skip-shadow-cleanup template=") + decl.name);
      }
    } else {
      remove_shadowing_template_parameter_bindings(bound_scope, decl.parameters);
    }
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);

    for(std::size_t i = 0; i < explicit_argument_bindings.fixed_arguments.size(); ++i) {
      const TemplateParameterInfo & parameter = decl.parameters[i];
      if(parameter.kind == TemplateParameterInfo::TP_TYPE && !parameter.name.empty()) {
        deduced_types[parameter.name] = explicit_argument_bindings.fixed_arguments[i].type;
      }
      if(parameter.kind == TemplateParameterInfo::TP_NON_TYPE && !parameter.name.empty() &&
         explicit_argument_bindings.fixed_arguments[i].kind == TemplateArgument::TA_VALUE &&
         !explicit_argument_bindings.fixed_arguments[i].dependent) {
        deduced_values[parameter.name] = explicit_argument_bindings.fixed_arguments[i].value;
      }
      ctx.bind_single_template_argument_into_scope(bound_scope,
                                                   parameter,
                                                   explicit_argument_bindings.fixed_arguments[i]);
    }
    if(explicit_argument_bindings.pack_parameter_index != static_cast<std::size_t>(-1)) {
      bind_explicit_function_template_pack_arguments(
          bound_scope,
          decl.parameters[explicit_argument_bindings.pack_parameter_index],
          explicit_argument_bindings.pack_arguments);
    }
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    Scope initial_bound_scope = bound_scope;

    const std::size_t deduction_count =
        function_template_deduction_parameter_count(decl, args.size());
    for(std::size_t i = 0; i < deduction_count; ++i) {
      bind_known_deductions_into_scope(
          ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
      bind_resolvable_default_non_type_template_arguments_into_scope(
          ctx, bound_scope, decl.parameters);
      const bool deducing_pack_element =
          uses_trailing_function_parameter_pack(decl) &&
          !decl.params_pattern.empty() &&
          i + 1 >= decl.params_pattern.size();
      const std::size_t pattern_index =
          deducing_pack_element && !decl.params_pattern.empty() ?
              decl.params_pattern.size() - 1 :
              i;
      const TypePtr original_pattern = decl.params_pattern[pattern_index].second;
      TypePtr pattern = prepare_function_template_deduction_pattern(
          ctx, decl.parameters, bound_scope, original_pattern);
      TypePtr actual = remove_reference_type(args[i].type);
      if(!actual) {
        actual = args[i].type;
      }
      apply_function_template_call_deduction_adjustments(decl, args[i], pattern, actual);
      TypePtr cv_inner;
      TypePtr actual_cv_inner;
      bool pattern_const = false;
      bool pattern_volatile = false;
      bool actual_const = false;
      bool actual_volatile = false;
      if(top_level_cv_flags(pattern, cv_inner, pattern_const, pattern_volatile) &&
         top_level_cv_flags(actual,
                            actual_cv_inner,
                            actual_const,
                            actual_volatile) &&
         ((pattern_const && actual_const) ||
          (pattern_volatile && actual_volatile))) {
        pattern = cv_inner;
        actual = actual_cv_inner;
      }
      const bool original_pattern_participates_in_deduction =
          type_mentions_unbound_function_template_parameter(
              ctx, deducible_parameters, initial_bound_scope, original_pattern);
      if(deducible_parameters.empty() ||
         (!type_mentions_unbound_function_template_parameter(
              ctx, deducible_parameters, bound_scope, pattern) &&
          !original_pattern_participates_in_deduction)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "explicit-skip-resolved-pattern template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }
      if(argument_is_braced_init_list_for_deduction(args[i]) &&
         !deduction_pattern_accepts_braced_init_list_argument(ctx, pattern)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "explicit-deduction-braced-init-rejected template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern)
                << " actual=" << describe_type(actual);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return false;
      }
      if(can_skip_resolved_non_dependent_pattern_check(
             ctx, bound_scope, decl, pattern_index, original_pattern, pattern)) {
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "skip-resolved-nondependent template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        continue;
      }
      DeducedTypeMap temp_deduced_types;
      DeducedValueMap temp_deduced_values;
      DeducedPackArgumentMap temp_deduced_pack_arguments;
      clone_deduced_type_map(deduced_types, temp_deduced_types);
      clone_deduced_value_map(deduced_values, temp_deduced_values);
      DeducedPackArgumentMap * const deduction_pack_arguments =
          deducing_pack_element ? &temp_deduced_pack_arguments :
                                  &deduced_pack_arguments;
      bool deduced_argument =
          deduce_initializer_list_pattern_from_list_like_argument(ctx,
                                                                 decl.parameters,
                                                                 pattern,
                                                                 actual,
                                                                 temp_deduced_types,
                                                                 temp_deduced_values,
                                                                 &bound_scope,
                                                                 deduction_pack_arguments);
      if(!deduced_argument) {
        deduced_argument =
            deduce_template_argument_impl(ctx,
                                          decl.parameters,
                                          pattern,
                                          actual,
                                          temp_deduced_types,
                                          temp_deduced_values,
                                          &bound_scope,
                                          false,
                                          nullptr,
                                          deduction_pack_arguments);
      }
      if(!deduced_argument) {
        TypePtr original_base = strip_top_level_cv(original_pattern);
        if(original_base && original_base->kind == Type::TK_NAMED) {
          const std::string normalized_text = strip_elaborated_type_prefix(
              trim_space(type_argument_text_for_deduction(ctx, original_base)));
          QualifiedName parsed_name;
          if(template_id_head_name_from_type_text(normalized_text, parsed_name)) {
            AliasTemplateDecl * alias_decl =
                ((!parsed_name.rooted && parsed_name.qualifiers.empty()) ?
                     semantic_lookup::lookup_unqualified_alias_template(
                         bound_scope, parsed_name.name) :
                     nullptr);
            if(!alias_decl) {
              alias_decl = ctx.lookup_alias_template(bound_scope, parsed_name);
            }
            if(alias_decl) {
              continue;
            }
          }
        }
        if(template_argument_semantics::type_depends_on_template_parameter(ctx, pattern) &&
           is_dependent_qualified_nondeduced_type_context(
               ctx, decl.parameters, original_pattern)) {
          if(parser_trace::enabled("template.resolve")) {
            std::ostringstream trace;
            trace << "explicit-deduction-deferred-nondeduced template=" << decl.name
                  << " param_index=" << pattern_index
                  << " pattern=" << describe_type(pattern)
                  << " actual=" << describe_type(actual);
            parser_trace::note("template.resolve", std::string(), trace.str());
          }
          continue;
        }
        if(parser_trace::enabled("template.resolve")) {
          std::ostringstream trace;
          trace << "explicit-deduction-failed template=" << decl.name
                << " param_index=" << pattern_index
                << " pattern=" << describe_type(pattern)
                << " actual=" << describe_type(actual);
          parser_trace::note("template.resolve", std::string(), trace.str());
        }
        return false;
      }
      if(deducing_pack_element) {
        if(!merge_deduced_pack_arguments(ctx,
                                         decl,
                                         bound_scope,
                                         deduced_types,
                                         deduced_values,
                                         temp_deduced_types,
                                         temp_deduced_values,
                                         temp_deduced_pack_arguments,
                                         deduced_pack_arguments)) {
          return false;
        }
      } else {
        deduced_types.swap(temp_deduced_types);
        deduced_values.swap(temp_deduced_values);
      }
    }

    bind_known_deductions_into_scope(
        ctx, bound_scope, decl.parameters, deduced_types, deduced_values);
    bind_resolvable_default_non_type_template_arguments_into_scope(
        ctx, bound_scope, decl.parameters);
    return finalize_deduced_function_template_arguments(
        ctx,
        decl,
        bound_scope,
        deduced_types,
        deduced_values,
        deduced_pack_arguments,
        &explicit_argument_bindings,
        out,
        pack_sizes_out);
  } catch(const TemplateSubstitutionFailure &) {
    return false;
  }
}

bool explicit_function_template_arguments_determine_signature(
    SemanticContext & ctx,
    FunctionTemplateDecl & decl,
    std::size_t explicit_argument_count)
{
  const std::size_t pack_index = first_template_parameter_pack_index(decl.parameters);
  if(pack_index == decl.parameters.size() || explicit_argument_count > pack_index) {
    return true;
  }

  const TemplateParameterInfo & omitted_pack = decl.parameters[pack_index];
  if(!omitted_pack.parameter_pack) {
    return true;
  }

  const std::vector<TemplateParameterInfo> omitted_parameters(1, omitted_pack);
  if(decl.type_pattern &&
     type_mentions_function_template_parameter(ctx, omitted_parameters, decl.type_pattern)) {
    return false;
  }
  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    if(type_mentions_function_template_parameter(
           ctx, omitted_parameters, decl.params_pattern[i].second)) {
      return false;
    }
  }
  return true;
}

bool trailing_pack_accepts_argument_count(const std::vector<TemplateParameterInfo> & parameters,
                                          std::size_t argument_count)
{
  std::size_t minimum_required = 0;
  for(std::size_t i = 0; i < parameters.size(); ++i) {
    if(parameters[i].parameter_pack) {
      break;
    }
    if(!parameters[i].default_argument) {
      ++minimum_required;
    }
  }

  if(argument_count < minimum_required) {
    return false;
  }
  if(parameters.empty()) {
    return argument_count == 0;
  }
  if(parameters.back().parameter_pack) {
    return true;
  }
  return argument_count <= parameters.size();
}

}  // namespace template_resolution
