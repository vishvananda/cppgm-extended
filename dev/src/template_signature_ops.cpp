#include "template_api.h"

#include "template_decl_ast.h"
#include "template_function_signature.h"
#include "template_services.h"

namespace template_api {
namespace signature {

std::string normalize_special_member_template_name(SemanticContext & ctx,
                                                   const std::string & name,
                                                   bool is_constructor,
                                                   bool is_destructor)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_function_signature::normalize_special_member_template_name(
            services, name, is_constructor, is_destructor);
      });
}

void parse_function_template_parameter_clause(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & parameter_clause,
    std::vector<std::pair<std::string, cpp_decl::TypePtr> > & params,
    std::vector<const CppAstNode *> & default_arguments)
{
  template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        template_function_signature::parse_function_template_parameter_clause(
            services,
            template_api::make_template_environment(scope),
            template_name,
            parameter_clause,
            params,
            default_arguments);
      });
}

bool expand_parameter_clause_pack_patterns(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    CppAstNode & expanded_clause,
    std::vector<const CppAstNode *> * default_args_out)
{
  return template_api::with_template_services(
      ctx,
      [&](template_api::TemplateServices & services)
      {
        return template_decl_ast::expand_parameter_clause_pack_patterns(
            services,
            scope,
            node,
            expanded_clause,
            default_args_out);
      });
}

CppAstNode build_function_result_type_pattern(
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator)
{
  return template_function_signature::build_function_result_type_pattern(
      parse_specifiers, parse_declarator);
}

ParsedFunctionTemplateSignature parse_function_template_signature(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  template_function_signature::ParsedFunctionTemplateSignature parsed =
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            return template_function_signature::parse_function_template_signature(
                services,
                template_api::make_template_environment(scope),
                template_name,
                raw_declarator,
                parse_specifiers,
                parse_declarator,
                filter_nonmember_declarator);
          });
  ParsedFunctionTemplateSignature out;
  out.name = parsed.name;
  out.type = parsed.type;
  out.params = parsed.params;
  out.default_arguments = parsed.default_arguments;
  out.parameter_declarations = parsed.parameter_declarations;
  out.result_type_pattern = parsed.result_type_pattern;
  out.effective_declarator = parsed.effective_declarator;
  return out;
}

FunctionTemplateSignatureParseResult try_parse_function_template_signature(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const std::string & template_name,
    const CppAstNode & raw_declarator,
    const CppAstNode & parse_specifiers,
    const CppAstNode & parse_declarator,
    bool filter_nonmember_declarator)
{
  template_function_signature::FunctionTemplateSignatureParseResult parsed =
      template_api::with_template_services(
          ctx,
          [&](template_api::TemplateServices & services)
          {
            return template_function_signature::try_parse_function_template_signature(
                services,
                template_api::make_template_environment(scope),
                template_name,
                raw_declarator,
                parse_specifiers,
                parse_declarator,
                filter_nonmember_declarator);
          });

  FunctionTemplateSignatureParseResult out;
  switch(parsed.status) {
  case template_function_signature::FunctionTemplateSignatureParseStatus::Ok:
    out.status = FunctionTemplateSignatureParseStatus::Ok;
    break;
  case template_function_signature::FunctionTemplateSignatureParseStatus::
      UnsupportedDeclSpecifiers:
    out.status = FunctionTemplateSignatureParseStatus::UnsupportedDeclSpecifiers;
    break;
  case template_function_signature::FunctionTemplateSignatureParseStatus::
      UnsupportedDeclarator:
    out.status = FunctionTemplateSignatureParseStatus::UnsupportedDeclarator;
    break;
  case template_function_signature::FunctionTemplateSignatureParseStatus::
      UnsupportedParameterClause:
    out.status = FunctionTemplateSignatureParseStatus::UnsupportedParameterClause;
    break;
  }
  out.signature.name = parsed.signature.name;
  out.signature.type = parsed.signature.type;
  out.signature.params = parsed.signature.params;
  out.signature.default_arguments = parsed.signature.default_arguments;
  out.signature.parameter_declarations = parsed.signature.parameter_declarations;
  out.signature.result_type_pattern = parsed.signature.result_type_pattern;
  out.signature.effective_declarator = parsed.signature.effective_declarator;
  out.diagnostic = parsed.diagnostic;
  return out;
}

}  // namespace signature
}  // namespace template_api
