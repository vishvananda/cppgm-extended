#include "template_function_signature.h"

#include <sstream>
#include <stdexcept>

#include "cpp_decl_bridge.h"
#include "semantic_errors.h"
#include "semantic_utils.h"
#include "template_decl_ast.h"

namespace template_function_signature {

using namespace cpp_decl;
using namespace semantic_model;

namespace {

bool is_non_result_type_decl_specifier(const CppAstNode & node)
{
  if(node.kind != CppAstKind::decl_specifier) {
    return false;
  }

  if(node.has_token && node.token_kind == RT_SIMPLE) {
    switch(node.simple_type) {
    case KW_TYPEDEF:
    case KW_EXTERN:
    case KW_STATIC:
    case KW_THREAD_LOCAL:
    case KW_MUTABLE:
    case KW_REGISTER:
    case KW_INLINE:
    case KW_VIRTUAL:
    case KW_EXPLICIT:
    case KW_FRIEND:
    case KW_CONSTEXPR:
      return true;
    default:
      return false;
    }
  }

  return node.value == "__inline" ||
         node.value == "__inline__" ||
         node.value == "__forceinline" ||
         node.value == "__thread";
}

CppAstNode result_type_specifier_pattern(const CppAstNode & parse_specifiers)
{
  CppAstNode out = parse_specifiers;
  out.children.clear();
  for(std::size_t i = 0; i < parse_specifiers.children.size(); ++i) {
    const CppAstNode & child = parse_specifiers.children[i];
    if(is_non_result_type_decl_specifier(child)) {
      continue;
    }
    out.children.push_back(child);
  }
  return out;
}

CppAstNode filter_function_declarator(const CppAstNode & declarator)
{
  CppAstNode filtered = declarator;
  std::vector<CppAstNode> kept;
  for(std::size_t i = 0; i < filtered.children.size(); ++i) {
    if(filtered.children[i].kind == CppAstKind::function_qualifier ||
       filtered.children[i].kind == CppAstKind::nullability_qualifier ||
       filtered.children[i].kind == CppAstKind::trailing_return_type) {
      continue;
    }
    kept.push_back(filtered.children[i]);
  }
  filtered.children.swap(kept);
  return filtered;
}

CppAstNode function_result_declarator(const CppAstNode & declarator)
{
  CppAstNode filtered = declarator;
  std::vector<CppAstNode> kept;
  for(std::size_t i = 0; i < filtered.children.size(); ++i) {
    if(filtered.children[i].kind == CppAstKind::parameter_clause) {
      continue;
    }
    kept.push_back(filtered.children[i]);
  }
  filtered.children.swap(kept);
  return filtered;
}

bool is_empty_abstract_declarator(const CppAstNode & node)
{
  return (node.kind == CppAstKind::abstract_declarator ||
          node.kind == CppAstKind::declarator) &&
         node.children.empty();
}

bool is_empty_nested_declarator(const CppAstNode & node)
{
  return node.kind == CppAstKind::nested_declarator &&
         node.children.size() == 1 &&
         is_empty_abstract_declarator(node.children[0]);
}

void append_function_result_declarator_children(
    const CppAstNode & source,
    CppAstNode & target,
    bool & erased_name,
    bool & erased_parameter_clause)
{
  bool erase_declared_function_suffixes = false;
  for(std::size_t i = 0; i < source.children.size(); ++i) {
    const CppAstNode & child = source.children[i];
    if(child.kind == CppAstKind::identifier) {
      erased_name = true;
      continue;
    }

    if(child.kind == CppAstKind::nested_declarator) {
      CppAstNode nested = child;
      nested.value.clear();
      nested.children.clear();
      if(child.children.size() == 1) {
        CppAstNode nested_declarator;
        nested_declarator.kind = CppAstKind::abstract_declarator;
        append_function_result_declarator_children(child.children[0],
                                                   nested_declarator,
                                                   erased_name,
                                                   erased_parameter_clause);
        if(!is_empty_abstract_declarator(nested_declarator)) {
          nested.children.push_back(nested_declarator);
        }
      }
      if(!is_empty_nested_declarator(nested)) {
        target.children.push_back(nested);
      }
      continue;
    }

    if(child.kind == CppAstKind::parameter_clause &&
       erased_name && !erased_parameter_clause) {
      erased_parameter_clause = true;
      erase_declared_function_suffixes = true;
      continue;
    }

    if(erase_declared_function_suffixes &&
       (child.kind == CppAstKind::function_qualifier ||
        child.kind == CppAstKind::ref_qualifier ||
        child.kind == CppAstKind::trailing_return_type)) {
      continue;
    }

    target.children.push_back(child);
  }
}

CppAstNode build_function_result_abstract_declarator(
    const CppAstNode & parse_declarator,
    bool & erased_function_signature)
{
  CppAstNode abstract;
  abstract.kind = CppAstKind::abstract_declarator;
  bool erased_name = false;
  bool erased_parameter_clause = false;
  append_function_result_declarator_children(parse_declarator,
                                             abstract,
                                             erased_name,
                                             erased_parameter_clause);
  erased_function_signature = erased_name && erased_parameter_clause;
  return abstract;
}

std::vector<TypePtr> normalized_parameter_types(
    const std::vector<std::pair<std::string, TypePtr> > & params)
{
  std::vector<TypePtr> out;
  out.reserve(params.size());
  for(std::size_t i = 0; i < params.size(); ++i) {
    out.push_back(normalize_parameter_type(params[i].second));
  }
  return out;
}

void sync_parameter_types_from_function_type(
    const TypePtr & function_type,
    std::vector<std::pair<std::string, TypePtr> > & params)
{
  TypePtr base = strip_top_level_cv(function_type);
  if(!base || base->kind != Type::TK_FUNCTION ||
     base->params.size() != params.size()) {
    return;
  }
  for(std::size_t i = 0; i < params.size(); ++i) {
    params[i].second = base->params[i];
  }
}

std::vector<const CppAstNode *> collect_parameter_declarations(
    const CppAstNode & parameter_clause)
{
  std::vector<const CppAstNode *> out;
  for(std::size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind == CppAstKind::parameter_declaration) {
      out.push_back(&parameter_clause.children[i]);
    }
  }
  return out;
}

bool parameter_clause_has_default_argument(const CppAstNode & parameter_clause)
{
  for(std::size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind != CppAstKind::parameter_declaration) {
      continue;
    }
    if(cpp_decl::find_child(parameter_clause.children[i],
                            CppAstKind::default_argument)) {
      return true;
    }
  }
  return false;
}

std::string diagnostic_node_label(const CppAstNode & node)
{
  const std::string text = semantic_utils::trim_space(node.value);
  return text.empty() ? std::string(cppast_kind_text(node.kind)) : text;
}

FunctionTemplateSignatureParseResult parsed_result(
    const ParsedFunctionTemplateSignature & signature)
{
  FunctionTemplateSignatureParseResult out;
  out.status = FunctionTemplateSignatureParseStatus::Ok;
  out.signature = signature;
  return out;
}

FunctionTemplateSignatureParseResult unsupported_result(
    FunctionTemplateSignatureParseStatus status,
    const std::string & diagnostic)
{
  FunctionTemplateSignatureParseResult out;
  out.status = status;
  out.diagnostic = diagnostic;
  return out;
}

}  // namespace

CppAstNode build_function_result_type_pattern(
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator)
{
  const CppAstNode * trailing =
      cpp_decl::find_child(parse_declarator, CppAstKind::trailing_return_type);
  if(trailing) {
    const CppAstNode * type_id = cpp_decl::find_child(*trailing, CppAstKind::type_id);
    if(type_id) {
      return *type_id;
    }
  }

  CppAstNode out;
  out.kind = CppAstKind::type_id;
  CppAstNode result_specifiers = result_type_specifier_pattern(parse_specifiers);
  out.value = result_specifiers.value;
  out.children.push_back(result_specifiers);

  bool erased_function_signature = false;
  CppAstNode abstract =
      build_function_result_abstract_declarator(parse_declarator,
                                                erased_function_signature);
  if(!erased_function_signature) {
    abstract = CppAstNode();
    abstract.kind = CppAstKind::abstract_declarator;
    for(std::size_t i = 0; i < parse_declarator.children.size(); ++i) {
      const CppAstNode & child = parse_declarator.children[i];
      if(child.kind == CppAstKind::ptr_operator) {
        abstract.children.push_back(child);
      }
    }
  }
  if(!abstract.children.empty()) {
    out.children.push_back(abstract);
  }
  return out;
}

std::string normalize_special_member_template_name(
    template_api::TemplateServices & services,
    const std::string & name,
    bool is_constructor,
    bool is_destructor)
{
  std::string normalized = semantic_utils::unqualified_member_name(name);
  if(is_constructor) {
    const std::string stripped =
        semantic_utils::strip_trailing_top_level_template_arguments(normalized);
    if(!stripped.empty()) {
      normalized = stripped;
    }
  } else if(is_destructor && normalized.size() > 1 && normalized[0] == '~') {
    const std::string destructor_target = normalized.substr(1);
    const std::string stripped =
        semantic_utils::strip_trailing_top_level_template_arguments(destructor_target);
    if(!stripped.empty()) {
      normalized = std::string("~") + stripped;
    }
  }
  return normalized;
}

void collect_function_template_default_arguments(
    const CppAstNode & parameter_clause,
    std::vector<const CppAstNode *> & default_arguments)
{
  default_arguments.clear();
  for(std::size_t i = 0; i < parameter_clause.children.size(); ++i) {
    if(parameter_clause.children[i].kind != CppAstKind::parameter_declaration) {
      continue;
    }
    default_arguments.push_back(
        cpp_decl::find_child(parameter_clause.children[i], CppAstKind::default_argument));
  }
}

void parse_function_template_parameter_clause(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, TypePtr> > & params,
    std::vector<const CppAstNode *> & default_arguments)
{
  Scope & scope_ref = scope.require();
  if(template_decl_ast::parse_parameter_clause(
         services, scope_ref, scope_ref, parameter_clause, params, &default_arguments, nullptr, true)) {
    return;
  }

  collect_function_template_default_arguments(parameter_clause, default_arguments);
  std::ostringstream out;
  out << "unsupported function template parameter-clause";
  out << " [template " << template_name << "]";
  out << " [parameter-clause " << diagnostic_node_label(parameter_clause) << "]";
  if(parameter_clause_has_default_argument(parameter_clause)) {
    throw TemplateSubstitutionFailure(out.str());
  }
  throw std::logic_error(out.str());
}

FunctionTemplateSignatureParseResult try_parse_function_template_signature(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  Scope & scope_ref = scope.require();
  ParsedFunctionTemplateSignature out;
  (void)filter_nonmember_declarator;

  bool is_typedef = false;
  TypePtr base;
  if(!template_decl_ast::parse_trailing_return_base(
         services, scope_ref, parse_specifiers, parse_declarator, is_typedef, base, true) ||
     is_typedef) {
    std::ostringstream msg;
    msg << "unsupported function template decl-specifier-seq";
    msg << " [name " << template_name << "]";
    msg << " [specifiers " << diagnostic_node_label(parse_specifiers) << "]";
    msg << " [declarator " << diagnostic_node_label(parse_declarator) << "]";
    return unsupported_result(
        FunctionTemplateSignatureParseStatus::UnsupportedDeclSpecifiers,
        msg.str());
  }

  out.effective_declarator = filter_function_declarator(parse_declarator);
  out.result_type_pattern =
      build_function_result_type_pattern(parse_specifiers, parse_declarator);

  if(!template_decl_ast::parse_declarator(
         services, scope_ref, scope_ref, out.effective_declarator, base, out.name, out.type, true) ||
     !out.type || strip_top_level_cv(out.type)->kind != Type::TK_FUNCTION) {
    const CppAstNode * parameter_clause =
        cpp_decl::find_child(raw_declarator, CppAstKind::parameter_clause);
    bool variadic = false;
    if(parameter_clause) {
      if(!template_decl_ast::parse_parameter_clause(
             services,
             scope_ref,
             scope_ref,
             *parameter_clause,
             out.params,
             &out.default_arguments,
             &variadic,
             true)) {
        std::ostringstream msg;
        msg << "unsupported function template parameter-clause";
        msg << " [template " << template_name << "]";
        msg << " [parameter-clause " << diagnostic_node_label(*parameter_clause) << "]";
        return unsupported_result(
            FunctionTemplateSignatureParseStatus::UnsupportedParameterClause,
            msg.str());
      }
      CppAstNode result_declarator =
          function_result_declarator(out.effective_declarator);
      TypePtr result_type;
      std::string parsed_name;
      if(template_decl_ast::parse_declarator(
             services, scope_ref, scope_ref, result_declarator, base, parsed_name, result_type, true) &&
         result_type) {
        out.name = parsed_name;
        out.type = make_function(result_type,
                                 normalized_parameter_types(out.params),
                                 variadic,
                                 false,
                                 false);
        out.parameter_declarations = collect_parameter_declarations(*parameter_clause);
        return parsed_result(out);
      }
    }

    std::ostringstream msg;
    msg << "unsupported function template declarator";
    msg << " [name " << template_name << "]";
    msg << " [declarator " << diagnostic_node_label(out.effective_declarator) << "]";
    return unsupported_result(
        FunctionTemplateSignatureParseStatus::UnsupportedDeclarator,
        msg.str());
  }

  const CppAstNode * parameter_clause =
      cpp_decl::find_child(raw_declarator, CppAstKind::parameter_clause);
  if(parameter_clause) {
    if(!template_decl_ast::parse_parameter_clause(
           services,
           scope_ref,
           scope_ref,
           *parameter_clause,
           out.params,
           &out.default_arguments,
           nullptr,
           true)) {
      std::ostringstream msg;
      msg << "unsupported function template parameter-clause";
      msg << " [template " << template_name << "]";
      msg << " [parameter-clause " << diagnostic_node_label(*parameter_clause) << "]";
      return unsupported_result(
          FunctionTemplateSignatureParseStatus::UnsupportedParameterClause,
          msg.str());
    }
    sync_parameter_types_from_function_type(out.type, out.params);
    out.parameter_declarations = collect_parameter_declarations(*parameter_clause);
  }

  return parsed_result(out);
}

ParsedFunctionTemplateSignature parse_function_template_signature(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  FunctionTemplateSignatureParseResult result =
      try_parse_function_template_signature(services,
                                            scope,
                                            template_name,
                                            raw_declarator,
                                            parse_specifiers,
                                            parse_declarator,
                                            filter_nonmember_declarator);
  if(!result.ok()) {
    throw std::logic_error(result.diagnostic);
  }
  return result.signature;
}

}  // namespace template_function_signature
