#pragma once

#include <string>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "semantic_model.h"
#include "template_environment.h"
#include "template_service_interfaces.h"

namespace template_function_signature {

struct ParsedFunctionTemplateSignature
{
  std::string name;
  cpp_decl::TypePtr type;
  std::vector<std::pair<std::string, cpp_decl::TypePtr> > params;
  std::vector<const CppAstNode *> default_arguments;
  std::vector<const CppAstNode *> parameter_declarations;
  CppAstNode result_type_pattern;
  CppAstNode effective_declarator;
};

enum class FunctionTemplateSignatureParseStatus
{
  Ok,
  UnsupportedDeclSpecifiers,
  UnsupportedDeclarator,
  UnsupportedParameterClause,
};

struct FunctionTemplateSignatureParseResult
{
  FunctionTemplateSignatureParseStatus status =
      FunctionTemplateSignatureParseStatus::Ok;
  ParsedFunctionTemplateSignature signature;
  std::string diagnostic;

  bool ok() const
  {
    return status == FunctionTemplateSignatureParseStatus::Ok;
  }
};

CppAstNode build_function_result_type_pattern(
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator);

std::string normalize_special_member_template_name(
    template_api::TemplateServices & services,
    const std::string & name,
    bool is_constructor,
    bool is_destructor);

void parse_function_template_parameter_clause(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
    std::vector<const CppAstNode *> & default_arguments);

void collect_function_template_default_arguments(
    const CppAstNode & parameter_clause,
    std::vector<const CppAstNode *> & default_arguments);

ParsedFunctionTemplateSignature parse_function_template_signature(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator);

FunctionTemplateSignatureParseResult try_parse_function_template_signature(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator);

}  // namespace template_function_signature
