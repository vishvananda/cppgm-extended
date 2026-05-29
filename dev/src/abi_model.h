#pragma once

// Itanium ABI encoder helpers layered on the assignment-facing data model in
// abi_mangle.h. This header adds construction helpers, substitution support,
// and emitters without introducing a second ABI representation.

#include "abi_mangle.h"

namespace abi_mangle {

inline void set_substitution(Type & type, SubstitutionKey key)
{
  if(!type.substitution || !type.substitution.unique()) {
    type.substitution.reset(type.substitution ?
        new Type::SubstitutionMetadata(*type.substitution) :
        new Type::SubstitutionMetadata);
  }
  type.substitution->key = std::move(key);
}

inline Type with_substitution(Type type, SubstitutionKey key)
{
  set_substitution(type, std::move(key));
  return type;
}

inline const SubstitutionKey & type_substitution_key(const Type & type)
{
  static const SubstitutionKey empty_key;
  return type.substitution ? type.substitution->key : empty_key;
}

inline bool type_has_substitution(const Type & type)
{
  return !type_substitution_key(type).empty();
}

inline bool make_type_substitution_key(const Type & type, SubstitutionKey & out);
inline bool make_template_argument_substitution_key(
    const TemplateArgument & argument,
    SubstitutionKey & out);
inline bool make_class_template_argument_substitution_key(
    const Type::ClassTemplateArgument & argument,
    SubstitutionKey & out);
inline bool make_dependent_expression_substitution_key(
    const DependentExpression & expression,
    SubstitutionKey & out);
inline bool emit_local_entity_context_function_encoding_body(
    const FunctionEncoding & function,
    std::string & out,
    SubstitutionSink * sink);

inline bool make_lambda_closure_substitution_key(
    const std::string & context_fragment,
    const std::shared_ptr<FunctionEncoding> & context_function,
    const std::string & source_name,
    const std::vector<Type> & signature_parameter_types,
    const std::string & discriminator,
    SubstitutionKey & out)
{
  std::string context_key = context_fragment;
  if(context_key.empty() && context_function) {
    std::string encoded_context;
    if(!emit_local_entity_context_function_encoding_body(*context_function,
                                                         encoded_context,
                                                         nullptr)) {
      return false;
    }
    context_key = std::string("function:") + encoded_context;
  }
  if(context_key.empty()) {
    return false;
  }
  std::string payload = std::string("lambda-closure:") + context_key + ":";
  if(!source_name.empty()) {
    payload += "source:";
    payload += source_name;
  } else {
    payload += "signature:";
    for(std::size_t i = 0; i < signature_parameter_types.size(); ++i) {
      SubstitutionKey parameter_key;
      if(!make_type_substitution_key(signature_parameter_types[i],
                                     parameter_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += parameter_key.structural_text();
    }
  }
  payload += ":discriminator:";
  payload += discriminator;
  out = SubstitutionKey::type(payload);
  return true;
}

inline bool make_type_substitution_key(const Type & type, SubstitutionKey & out)
{
  if(type_has_substitution(type)) {
    out = type_substitution_key(type);
    return true;
  }
  switch(type.kind) {
  case Type::TK_BUILTIN:
    if(type.builtin_code[0] == '\0') {
      return false;
    }
    out = SubstitutionKey::type_builtin(std::string(type.builtin_code));
    return true;

  case Type::TK_CV: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_cv(type.cv_const,
                                   type.cv_volatile,
                                   std::move(inner_key));
    return true;
  }

  case Type::TK_POINTER: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_pointer(std::move(inner_key));
    return true;
  }

  case Type::TK_LVALUE_REFERENCE: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_lvalue_reference(std::move(inner_key));
    return true;
  }

  case Type::TK_RVALUE_REFERENCE: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_rvalue_reference(std::move(inner_key));
    return true;
  }

  case Type::TK_ARRAY: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    const std::string bound_key =
        type.array_substitution_bound_key.empty() ?
            type.array_bound :
            type.array_substitution_bound_key;
    out = SubstitutionKey::type_array(bound_key, std::move(inner_key));
    return true;
  }

  case Type::TK_FUNCTION: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey result_key;
    if(!make_type_substitution_key(*type.inner, result_key)) {
      return false;
    }
    std::vector<SubstitutionKey> param_keys;
    param_keys.reserve(type.params.size());
    for(std::size_t i = 0; i < type.params.size(); ++i) {
      SubstitutionKey param_key;
      if(!make_type_substitution_key(type.params[i], param_key)) {
        return false;
      }
      param_keys.push_back(std::move(param_key));
    }
    out = SubstitutionKey::type_function(std::move(result_key),
                                         std::move(param_keys),
                                         type.variadic,
                                         type.function_lvalue_ref,
                                         type.function_rvalue_ref);
    return true;
  }

  case Type::TK_MEMBER_POINTER: {
    if(!type.owner || !type.inner) {
      return false;
    }
    SubstitutionKey owner_key;
    SubstitutionKey member_key;
    if(!make_type_substitution_key(*type.owner, owner_key) ||
       !make_type_substitution_key(*type.inner, member_key)) {
      return false;
    }
    out = SubstitutionKey::type_member_pointer(std::move(owner_key),
                                               std::move(member_key));
    return true;
  }

  case Type::TK_VENDOR_QUALIFIED: {
    if(type.vendor_qualifier_name.empty() || !type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type(
        std::string("vendor-qualified:") +
        type.vendor_qualifier_name + ":" + inner_key.structural_text());
    return true;
  }

  case Type::TK_BUILTIN_TYPE_TRANSFORM: {
    if(type.builtin_transform_name.empty() || !type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type(
        std::string("builtin-type-transform:") +
        type.builtin_transform_name + ":" + inner_key.structural_text());
    return true;
  }

  case Type::TK_PACK_EXPANSION: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type(
        std::string("pack-expansion:") + inner_key.structural_text());
    return true;
  }

  case Type::TK_TEMPLATE_PARAMETER:
    out = SubstitutionKey::type_template_parameter(
        type.template_parameter_index);
    return true;

  case Type::TK_NAMED:
    if(!type_has_substitution(type)) {
      return false;
    }
    out = type_substitution_key(type);
    return true;

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
    if(!type_has_substitution(type)) {
      return false;
    }
    out = type_substitution_key(type);
    return true;

  case Type::TK_DECLTYPE_EXPRESSION:
    if(!type.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*type.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::type(std::string("decltype(") +
                                  expression_key.structural_text() + ")");
      return true;
    }

  case Type::TK_LAMBDA_CLOSURE:
    if(!type.lambda) {
      return false;
    }
    return make_lambda_closure_substitution_key(
        type.lambda->context_fragment,
        type.lambda->context_function,
        type.lambda->source_name,
        type.params,
        type.lambda->discriminator,
        out);

  case Type::TK_INVALID:
    return false;
  }
  return false;
}

inline bool make_class_template_argument_substitution_key(
    const Type::ClassTemplateArgument & argument,
    SubstitutionKey & out)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE:
    if(!argument.type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*argument.type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_type(std::move(type_key));
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    if(!argument.type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*argument.type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("integral:") + type_key.structural_text() + ":" +
          std::to_string(argument.integral_value));
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    {
      SubstitutionKey parameter_type_key;
      if(!make_type_substitution_key(*argument.parameter_type,
                                     parameter_type_key)) {
        return false;
      }
      std::string payload = std::string("dependent-integral:") +
                            parameter_type_key.structural_text() + ":";
      if(argument.type) {
        SubstitutionKey value_type_key;
        if(!make_type_substitution_key(*argument.type, value_type_key)) {
          return false;
        }
        payload += value_type_key.structural_text();
      }
      payload += ":";
      payload += std::to_string(argument.integral_value);
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*argument.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("dependent-expression:") +
          expression_key.structural_text());
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    out = SubstitutionKey::template_argument_value(
        std::string("untyped-integral:") +
        std::to_string(argument.integral_value));
    return true;

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    if(argument.metadata->template_name_is_template_parameter) {
      out = SubstitutionKey::template_argument_template(
          0,
          std::string("template-parameter:") +
              std::to_string(argument.metadata->template_parameter_index));
      return !out.empty();
    }
    if(argument.metadata->template_owner_type) {
      SubstitutionKey owner_key;
      if(!make_type_substitution_key(*argument.metadata->template_owner_type,
                                     owner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_template(
          0,
          std::string("member-template:") +
              owner_key.structural_text() +
              "::" +
              argument.metadata->template_name);
      return !out.empty();
    }
    out = SubstitutionKey::template_argument_template(
        0,
        argument.metadata->template_name_substitution.empty() ?
            argument.metadata->template_name :
            argument.metadata->template_name_substitution);
    return !out.empty();

  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("external:") +
        (argument.metadata->external_entity_address_of ? "address:" : "reference:") +
        argument.metadata->external_entity_symbol);
    return !argument.metadata->external_entity_symbol.empty();

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK: {
    if(!argument.metadata) {
      return false;
    }
    std::string payload = "pack:";
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      SubstitutionKey child_key;
      if(!make_class_template_argument_substitution_key(
             argument.metadata->pack_arguments[i],
             child_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += child_key.structural_text();
    }
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

inline bool make_template_argument_substitution_key(
    const TemplateArgument & argument,
    SubstitutionKey & out)
{
  switch(argument.kind) {
  case TemplateArgument::TAK_TYPE: {
    SubstitutionKey type_key;
    if(!argument.value_type ||
       !make_type_substitution_key(*argument.value_type, type_key)) {
      return false;
    }
    out = SubstitutionKey::template_argument_type(std::move(type_key));
    return true;
  }

  case TemplateArgument::TAK_INTEGRAL_VALUE: {
    SubstitutionKey type_key;
    if(!argument.value_type ||
       !make_type_substitution_key(*argument.value_type, type_key)) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("integral:") + type_key.structural_text() + ":" +
        std::to_string(argument.integral_value));
    return true;
  }

  case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE: {
    SubstitutionKey parameter_type_key;
    if(!argument.parameter_type ||
       !make_type_substitution_key(*argument.parameter_type,
                                   parameter_type_key)) {
      return false;
    }
    std::string payload = std::string("dependent-integral:") +
                          parameter_type_key.structural_text() + ":";
    if(argument.value_type) {
      SubstitutionKey value_type_key;
      if(!make_type_substitution_key(*argument.value_type, value_type_key)) {
        return false;
      }
      payload += value_type_key.structural_text();
    }
    payload += ":";
    payload += std::to_string(argument.integral_value);
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*argument.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("dependent-expression:") +
          expression_key.structural_text());
      return true;
    }

  case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
    out = SubstitutionKey::template_argument_value(
        std::string("untyped-integral:") +
        std::to_string(argument.integral_value));
    return true;

  case TemplateArgument::TAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    if(argument.metadata->template_name_is_template_parameter) {
      out = SubstitutionKey::template_argument_template(
          0,
          std::string("template-parameter:") +
              std::to_string(argument.metadata->template_parameter_index));
      return !out.empty();
    }
    if(argument.metadata->template_owner_type) {
      SubstitutionKey owner_key;
      if(!make_type_substitution_key(*argument.metadata->template_owner_type,
                                     owner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_template(
          0,
          std::string("member-template:") +
              owner_key.structural_text() +
              "::" +
              argument.metadata->template_name);
      return !out.empty();
    }
    out = SubstitutionKey::template_argument_template(
        0,
        argument.metadata->template_name_substitution.empty() ?
            argument.metadata->template_name :
            argument.metadata->template_name_substitution);
    return !out.empty();

  case TemplateArgument::TAK_EXTERNAL_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("external:") +
        (argument.metadata->external_entity_address_of ? "address:" : "reference:") +
        argument.metadata->external_entity_symbol);
    return !argument.metadata->external_entity_symbol.empty();

  case TemplateArgument::TAK_ARGUMENT_PACK: {
    if(!argument.metadata) {
      return false;
    }
    std::string payload = "pack:";
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      SubstitutionKey child_key;
      if(!make_template_argument_substitution_key(
             argument.metadata->pack_arguments[i],
             child_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += child_key.structural_text();
    }
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case TemplateArgument::TAK_INVALID:
    return false;
  }
  return false;
}

inline bool make_dependent_expression_substitution_key(
    const DependentExpression & expression,
    SubstitutionKey & out)
{
  switch(expression.kind) {
  case DependentExpression::EK_TEMPLATE_PARAMETER:
    out = SubstitutionKey::template_argument_value(
        std::string("expr-template-parameter:") +
        std::to_string(expression.template_parameter_index));
    return true;

  case DependentExpression::EK_FUNCTION_PARAMETER:
    out = SubstitutionKey::template_argument_value(
        std::string("expr-function-parameter:") +
        std::to_string(expression.template_parameter_index));
    return true;

  case DependentExpression::EK_LITERAL:
    if(expression.text.empty()) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("expr-literal:") +
        (expression.text[0] == '+' ? expression.text.substr(1) :
                                     expression.text));
    return true;

  case DependentExpression::EK_INTEGRAL_VALUE:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-integral:") + type_key.structural_text() + ":" +
          std::to_string(expression.integral_value));
      return true;
    }

  case DependentExpression::EK_MEMBER:
    if(!expression.owner_type || expression.text.empty()) {
      return false;
    }
    {
      SubstitutionKey owner_key;
      if(!make_type_substitution_key(*expression.owner_type, owner_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-member:") + owner_key.structural_text() + ":" +
          (expression.close_member_owner ? "close:" : "open:") +
          expression.text;
      if(!expression.template_arguments.empty()) {
        payload += '<';
        for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
          SubstitutionKey argument_key;
          if(!make_template_argument_substitution_key(
                 expression.template_arguments[i],
                 argument_key)) {
            return false;
          }
          if(i != 0) {
            payload += ',';
          }
          payload += argument_key.structural_text();
        }
        payload += '>';
      }
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_OBJECT_MEMBER:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.text.empty()) {
      return false;
    }
    {
      SubstitutionKey object_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     object_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-object-member:") + expression.op_code + ":" +
          object_key.structural_text() + ":" + expression.text;
      if(!expression.template_arguments.empty()) {
        payload += '<';
        for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
          SubstitutionKey argument_key;
          if(!make_template_argument_substitution_key(
                 expression.template_arguments[i],
                 argument_key)) {
            return false;
          }
          if(i != 0) {
            payload += ',';
          }
          payload += argument_key.structural_text();
        }
        payload += '>';
      }
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_UNARY:
    if(expression.op_code.empty() || !expression.inner) {
      return false;
    }
    {
      SubstitutionKey inner_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     inner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-unary:") + expression.op_code + ":" +
          inner_key.structural_text());
      return true;
    }

  case DependentExpression::EK_BINARY:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.arguments.size() != 1) {
      return false;
    }
    {
      SubstitutionKey left_key;
      SubstitutionKey right_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     left_key) ||
         !make_dependent_expression_substitution_key(expression.arguments[0],
                                                     right_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-binary:") + expression.op_code + ":" +
          left_key.structural_text() + ":" + right_key.structural_text());
      return true;
    }

  case DependentExpression::EK_CONDITIONAL:
    if(!expression.inner || expression.arguments.size() != 2) {
      return false;
    }
    {
      SubstitutionKey condition_key;
      SubstitutionKey true_key;
      SubstitutionKey false_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     condition_key) ||
         !make_dependent_expression_substitution_key(expression.arguments[0],
                                                     true_key) ||
         !make_dependent_expression_substitution_key(expression.arguments[1],
                                                     false_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-conditional:") +
          condition_key.structural_text() + ":" +
          true_key.structural_text() + ":" +
          false_key.structural_text());
      return true;
    }

  case DependentExpression::EK_PACK_EXPANSION:
    if(!expression.inner) {
      return false;
    }
    {
      SubstitutionKey inner_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     inner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-pack-expansion:") + inner_key.structural_text());
      return true;
    }

  case DependentExpression::EK_CALL:
    if(!expression.inner) {
      return false;
    }
    {
      SubstitutionKey callee_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     callee_key)) {
        return false;
      }
      std::string payload = std::string("expr-call:") +
                            callee_key.structural_text() + "(";
      for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
        SubstitutionKey argument_key;
        if(!make_dependent_expression_substitution_key(expression.arguments[i],
                                                       argument_key)) {
          return false;
        }
        if(i != 0) {
          payload += ',';
        }
        payload += argument_key.structural_text();
      }
      payload += ')';
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_CONVERSION:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-conversion:") +
          (expression.op_code.empty() ? std::string("cv") : expression.op_code) +
          ":" + type_key.structural_text() + "(";
      for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
        SubstitutionKey argument_key;
        if(!make_dependent_expression_substitution_key(expression.arguments[i],
                                                       argument_key)) {
          return false;
        }
        if(i != 0) {
          payload += ',';
        }
        payload += argument_key.structural_text();
      }
      payload += ')';
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_TEMPLATE_ID: {
    if(expression.text.empty()) {
      return false;
    }
    std::string payload = std::string("expr-template-id:") +
                          expression.text + "<";
    for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
      SubstitutionKey argument_key;
      if(!make_template_argument_substitution_key(
             expression.template_arguments[i],
             argument_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += argument_key.structural_text();
    }
    payload += '>';
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case DependentExpression::EK_TYPE_TRAIT: {
    if(expression.text.empty()) {
      return false;
    }
    std::string payload = std::string("expr-type-trait:") +
                          expression.text + "(";
    for(std::size_t i = 0; i < expression.type_arguments.size(); ++i) {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(expression.type_arguments[i], type_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += type_key.structural_text();
    }
    payload += ')';
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case DependentExpression::EK_SIZEOF_TYPE:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-sizeof-type:") + type_key.structural_text());
      return true;
    }

  case DependentExpression::EK_EXTERNAL_ENTITY:
    if(expression.text.empty()) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("expr-external:") +
        (expression.external_entity_address_of ? "address:" : "reference:") +
        expression.text);
    return true;

  case DependentExpression::EK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_type(const Type & type, std::string & out, SubstitutionSink * sink);
inline bool emit_type_owned(Type & type, std::string & out, SubstitutionSink * sink);
inline bool emit_type_body_owned(Type & type, std::string & out, SubstitutionSink * sink);
inline bool emit_class_template_argument(
    const Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_class_template_argument_owned(
    Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_class_template_arguments(
    const std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_class_template_arguments_owned(
    std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_template_argument(const TemplateArgument & argument,
                                   std::string & out,
                                   SubstitutionSink * sink);
inline bool emit_template_arguments(
    const std::vector<TemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_dependent_expression_body(const DependentExpression & expression,
                                           std::string & out,
                                           SubstitutionSink * sink);
inline bool emit_type_as_name_prefix(const Type & type,
                                     std::string & out,
                                     SubstitutionSink * sink);
inline bool emit_type_as_name_prefix_body(const Type & type,
                                          std::string & out,
                                          SubstitutionSink * sink);
inline bool emit_type_as_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_source_name(const std::string & name, std::string & out);
inline bool emit_function_operator_terminal(
    FunctionOperatorTerminal terminal,
    const std::string & literal_suffix,
    std::string & out);
inline bool function_operator_terminal_from_cpp_spelling(
    const std::string & compact_spelling,
    std::size_t explicit_parameter_count,
    bool member_function,
    FunctionOperatorTerminal & out,
    std::string & literal_suffix);
inline bool function_operator_terminal_from_semantic_name(
    const std::string & name,
    std::size_t explicit_parameter_count,
    bool member_function,
    FunctionOperatorTerminal & out);
inline bool function_operator_terminal_substitution_text(
    FunctionOperatorTerminal terminal,
    const std::string & literal_suffix,
    std::string & out);
inline void emit_abi_tags(const std::vector<std::string> & abi_tags,
                          std::string & out);
inline bool emit_function_name(const FunctionEncoding & function,
                               std::string & out,
                               SubstitutionSink * sink);
inline bool emit_function_encoding(const FunctionEncoding & function,
                                   std::string & out,
                                   SubstitutionSink * sink);
inline bool emit_function_encoding_owned(FunctionEncoding & function,
                                         std::string & out,
                                         SubstitutionSink * sink);
inline bool emit_function_encoding_body(const FunctionEncoding & function,
                                        std::string & out,
                                        SubstitutionSink * sink);
inline bool emit_function_encoding_body_owned(FunctionEncoding & function,
                                             std::string & out,
                                             SubstitutionSink * sink);
inline bool emit_local_entity_context_function_encoding_body(
    const FunctionEncoding & function,
    std::string & out,
    SubstitutionSink * sink);

inline bool type_needs_member_expression_template_name_registration(
    const Type & type);

inline bool class_template_argument_needs_member_expression_template_name_registration(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.type &&
     type_needs_member_expression_template_name_registration(*argument.type)) {
    return true;
  }
  if(argument.parameter_type &&
     type_needs_member_expression_template_name_registration(
         *argument.parameter_type)) {
    return true;
  }
  if(argument.metadata) {
    if(argument.metadata->template_owner_type &&
       type_needs_member_expression_template_name_registration(
           *argument.metadata->template_owner_type)) {
      return true;
    }
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_needs_member_expression_template_name_registration(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool template_arguments_need_member_expression_template_name_registration(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_needs_member_expression_template_name_registration(
           arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool type_needs_member_expression_template_name_registration(
    const Type & type)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(type.kind == Type::TK_CLASS_TEMPLATE_SPECIALIZATION &&
     metadata &&
     (metadata->register_member_expression_template_name ||
      template_arguments_need_member_expression_template_name_registration(
          metadata->template_arguments))) {
    return true;
  }
  if(type.inner &&
     type_needs_member_expression_template_name_registration(*type.inner)) {
    return true;
  }
  if(type.owner &&
     type_needs_member_expression_template_name_registration(*type.owner)) {
    return true;
  }
  if(type.name_owner &&
     type_needs_member_expression_template_name_registration(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_needs_member_expression_template_name_registration(type.params[i])) {
      return true;
    }
  }
  return false;
}

struct LookupOnlySubstitutionSink : public SubstitutionSink
{
  explicit LookupOnlySubstitutionSink(SubstitutionSink * inner) : inner(inner) {}

  bool emit_substitution(const SubstitutionKey & key, std::string & out) override
  {
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    (void)key;
  }

  void register_substitution_owned(SubstitutionKey key) override
  {
    (void)key;
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return inner &&
           inner->suppress_template_parameter_type_substitution_in_template_argument();
  }

  SubstitutionSink * inner;
};

struct NoLookupSubstitutionSink : public SubstitutionSink
{
  explicit NoLookupSubstitutionSink(SubstitutionSink * inner) : inner(inner) {}

  bool emit_substitution(const SubstitutionKey &, std::string &) override
  {
    return false;
  }

  void register_substitution(const SubstitutionKey &) override
  {
  }

  void register_substitution_owned(SubstitutionKey) override
  {
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return inner &&
           inner->suppress_template_parameter_type_substitution_in_template_argument();
  }

  SubstitutionSink * inner;
};

inline bool emit_name_component(const Type::NameComponent & component,
                                std::string & out,
                                SubstitutionSink * sink)
{
  if(component.std_abbrev) {
    out += "St";
    return true;
  }
  if(component.source_name.empty()) {
    return false;
  }
  SubstitutionKey key =
      component.substitution_name.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(component.substitution_name);
  if(sink && !key.empty() && sink->emit_substitution(key, out)) {
    return true;
  }
  static const char anonymous_enum_prefix[] = "__anonymous_enum";
  if(component.source_name.compare(0,
                                   sizeof(anonymous_enum_prefix) - 1,
                                   anonymous_enum_prefix) == 0) {
    out += "Ut_";
  } else if(!emit_source_name(component.source_name, out)) {
    return false;
  }
  if(sink && !key.empty()) {
    sink->register_substitution_owned(std::move(key));
  }
  return true;
}

inline bool emit_name_prefix_components(
    const std::vector<Type::NameComponent> & components,
    std::string & out,
    SubstitutionSink * sink)
{
  std::size_t start = 0;
  if(sink) {
    for(std::size_t len = components.size(); len > 0; --len) {
      const std::string & substitution = components[len - 1].substitution_name;
      if(substitution.empty()) {
        continue;
      }
      if(sink->emit_substitution(SubstitutionKey::named(substitution), out)) {
        start = len;
        break;
      }
    }
  }
  for(std::size_t i = start; i < components.size(); ++i) {
    if(!emit_name_component(components[i], out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_template_name_component(const Type & type,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata ||
     (metadata->template_name.empty() &&
      !metadata->template_name_is_template_parameter)) {
    return false;
  }
  SubstitutionKey key =
      metadata->template_name_is_template_parameter ?
          SubstitutionKey::none() :
      metadata->template_name_substitution.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(metadata->template_name_substitution);
  if(sink && !key.empty() && sink->emit_substitution(key, out)) {
    return true;
  }
  if(metadata->template_name_is_template_parameter) {
    out += 'T';
    if(metadata->template_name_parameter_index > 0) {
      out += std::to_string(metadata->template_name_parameter_index - 1);
    }
    out += '_';
  } else if(!emit_source_name(metadata->template_name, out)) {
    return false;
  }
  if(sink && !key.empty()) {
    sink->register_substitution_owned(std::move(key));
  }
  return true;
}

inline bool emit_type_as_name_prefix_body(const Type & type,
                                          std::string & out,
                                          SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += std::to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
      return false;
    }
    return emit_name_component(
        Type::NameComponent::source(metadata->template_name,
                                    metadata->template_name_substitution),
        out,
        sink);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata) {
      return false;
    }
    if(!metadata->standard_substitution.empty()) {
      out += metadata->standard_substitution;
      if(metadata->standard_substitution_includes_arguments) {
        return true;
      }
      return emit_class_template_arguments(metadata->template_arguments, out, sink);
    }
    if(metadata->template_name.empty() &&
       !metadata->template_name_is_template_parameter) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
      return false;
    }
    return emit_template_name_component(type, out, sink) &&
           emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  case Type::TK_BUILTIN_TYPE_TRANSFORM:
    return emit_type(type, out, sink);

  default:
    return false;
  }
}

inline bool emit_type_as_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink)
{
  NoLookupSubstitutionSink no_lookup_sink(sink);
  SubstitutionSink * const owner_name_sink = sink ? &no_lookup_sink : nullptr;
  switch(type.kind) {
  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      out += 'N';
      if(!emit_type_as_name_prefix(*type.name_owner, out, owner_name_sink)) {
        return false;
      }
      if(!emit_source_name(metadata->template_name, out)) {
        return false;
      }
      out += 'E';
      return true;
    } else if(!emit_name_prefix_components(metadata->prefix_components,
                                           out,
                                           owner_name_sink)) {
      return false;
    }
    return emit_source_name(metadata->template_name, out);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata) {
      return false;
    }
    if(!metadata->standard_substitution.empty()) {
      out += metadata->standard_substitution;
      if(metadata->standard_substitution_includes_arguments) {
        return true;
      }
      return emit_class_template_arguments(metadata->template_arguments, out, sink);
    }
    if(metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      SubstitutionSink * const member_owner_sink =
          metadata->register_member_expression_template_name ? sink :
                                                               owner_name_sink;
      out += 'N';
      if(!emit_type_as_name_prefix(*type.name_owner, out, member_owner_sink)) {
        return false;
      }
      if(!emit_source_name(metadata->template_name, out)) {
        return false;
      }
      if(sink &&
         (metadata->register_member_expression_template_name ||
          template_arguments_need_member_expression_template_name_registration(
              metadata->template_arguments))) {
        if(!metadata->template_name_ir_substitution.empty()) {
          sink->register_substitution(
              metadata->template_name_ir_substitution.get());
        } else if(!metadata->template_name_substitution.empty()) {
          sink->register_substitution_owned(
              SubstitutionKey::named(metadata->template_name_substitution));
        }
      }
      if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
        return false;
      }
      out += 'E';
      return true;
    } else if(!emit_name_prefix_components(metadata->prefix_components,
                                           out,
                                           owner_name_sink)) {
      return false;
    }
    if(!emit_source_name(metadata->template_name, out)) {
      return false;
    }
    if(sink &&
       (metadata->register_member_expression_template_name ||
        template_arguments_need_member_expression_template_name_registration(
            metadata->template_arguments))) {
      if(!metadata->template_name_ir_substitution.empty()) {
        sink->register_substitution(
            metadata->template_name_ir_substitution.get());
      } else if(!metadata->template_name_substitution.empty()) {
        sink->register_substitution_owned(
            SubstitutionKey::named(metadata->template_name_substitution));
      }
    }
    if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
      return false;
    }
    return true;
  }

  default:
    return emit_type(type, out, sink);
  }
}

inline bool emit_type_as_unqualified_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    return emit_source_name(metadata->template_name, out);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(!emit_source_name(metadata->template_name, out)) {
      return false;
    }
    return emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  default:
    return emit_type_as_member_expression_owner_prefix_body(type, out, sink);
  }
}

inline bool emit_type_as_name_prefix(const Type & type,
                                     std::string & out,
                                     SubstitutionSink * sink)
{
  const SubstitutionKey & substitution_key = type_substitution_key(type);
  if(sink && !substitution_key.empty() &&
     sink->emit_substitution(substitution_key, out)) {
    return true;
  }

  const std::size_t begin = out.size();
  if(!emit_type_as_name_prefix_body(type, out, sink)) {
    out.resize(begin);
    return false;
  }

  if(sink && !substitution_key.empty()) {
    sink->register_substitution(substitution_key);
  }
  return true;
}

inline bool emit_named_type(const Type & type,
                            std::string & out,
                            SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata || metadata->template_name.empty()) {
    return false;
  }
  if(type.name_owner) {
    out += 'N';
    if(!emit_type_as_name_prefix_body(type, out, sink)) {
      return false;
    }
    out += 'E';
    return true;
  }
  const bool direct_std_prefix =
      metadata->prefix_components.size() == 1 &&
      metadata->prefix_components[0].std_abbrev;
  if(direct_std_prefix) {
    out += "St";
    return emit_name_component(
        Type::NameComponent::source(metadata->template_name,
                                    metadata->template_name_substitution),
        out,
        sink);
  }

  const bool nested = !metadata->prefix_components.empty();
  if(nested) {
    out += 'N';
  }
  if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
    return false;
  }
  if(!emit_name_component(
         Type::NameComponent::source(metadata->template_name,
                                     metadata->template_name_substitution),
         out,
         sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool class_template_argument_is_simple_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.kind != Type::ClassTemplateArgument::CTAK_TYPE ||
     !argument.type) {
    return false;
  }
  SubstitutionKey key;
  return make_type_substitution_key(*argument.type, key) &&
         key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER;
}

inline bool substitution_key_contains_template_parameter(
    const SubstitutionKey & key)
{
  if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
    return true;
  }
  for(std::size_t i = 0; i < key.children.size(); ++i) {
    if(substitution_key_contains_template_parameter(key.children[i])) {
      return true;
    }
  }
  return false;
}

inline bool type_contains_template_parameter_ref(const Type & type);

inline bool class_template_argument_shape_contains_template_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE:
    return argument.type &&
           type_contains_template_parameter_ref(*argument.type);

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    return argument.type &&
           type_contains_template_parameter_ref(*argument.type);

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    return (argument.parameter_type &&
            type_contains_template_parameter_ref(*argument.parameter_type)) ||
           (argument.type &&
            type_contains_template_parameter_ref(*argument.type));

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_shape_contains_template_parameter_ref(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
    return false;

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    return argument.metadata &&
           argument.metadata->template_owner_type &&
           type_contains_template_parameter_ref(
               *argument.metadata->template_owner_type);
  }
  return false;
}

inline bool type_contains_template_parameter_ref(const Type & type)
{
  if(type.kind == Type::TK_TEMPLATE_PARAMETER) {
    return true;
  }
  const Type::NameMetadata * metadata = type.name.get();
  if(type.kind == Type::TK_CLASS_TEMPLATE_SPECIALIZATION &&
     metadata &&
     metadata->template_name_is_template_parameter) {
    return true;
  }
  if(type.inner && type_contains_template_parameter_ref(*type.inner)) {
    return true;
  }
  if(type.owner && type_contains_template_parameter_ref(*type.owner)) {
    return true;
  }
  if(type.name_owner &&
     type_contains_template_parameter_ref(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_contains_template_parameter_ref(type.params[i])) {
      return true;
    }
  }
  if(metadata) {
    for(std::size_t i = 0; i < metadata->template_arguments.size(); ++i) {
      if(class_template_argument_shape_contains_template_parameter_ref(
             metadata->template_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool class_template_argument_contains_template_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  if(class_template_argument_shape_contains_template_parameter_ref(argument)) {
    return true;
  }
  SubstitutionKey key;
  return make_class_template_argument_substitution_key(argument, key) &&
         substitution_key_contains_template_parameter(key);
}

inline bool class_template_arguments_contain_template_parameter_ref(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_contains_template_parameter_ref(arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool class_template_arguments_are_only_simple_parameter_refs(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  if(arguments.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!class_template_argument_is_simple_parameter_ref(arguments[i])) {
      return false;
    }
  }
  return true;
}

inline bool type_contains_pack_expansion(const Type & type);

inline bool class_template_argument_contains_pack_expansion(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.type && type_contains_pack_expansion(*argument.type)) {
    return true;
  }
  if(argument.parameter_type &&
     type_contains_pack_expansion(*argument.parameter_type)) {
    return true;
  }
  if(argument.kind == Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK) {
    return true;
  }
  if(argument.metadata) {
    if(argument.metadata->template_owner_type &&
       type_contains_pack_expansion(*argument.metadata->template_owner_type)) {
      return true;
    }
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_contains_pack_expansion(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool type_contains_pack_expansion(const Type & type)
{
  if(type.kind == Type::TK_PACK_EXPANSION) {
    return true;
  }
  if(type.inner && type_contains_pack_expansion(*type.inner)) {
    return true;
  }
  if(type.owner && type_contains_pack_expansion(*type.owner)) {
    return true;
  }
  if(type.name_owner && type_contains_pack_expansion(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_contains_pack_expansion(type.params[i])) {
      return true;
    }
  }
  const Type::NameMetadata * metadata = type.name.get();
  if(metadata) {
    for(std::size_t i = 0; i < metadata->template_arguments.size(); ++i) {
      if(class_template_argument_contains_pack_expansion(
             metadata->template_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool class_template_arguments_contain_pack_expansion(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_contains_pack_expansion(arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool emit_class_template_specialization_type(const Type & type,
                                                    std::string & out,
                                                    SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata) {
    return false;
  }
  if(type.name_owner) {
    out += 'N';
    if(!emit_type_as_name_prefix_body(type, out, sink)) {
      return false;
    }
    out += 'E';
    return true;
  }

  if(!metadata->standard_substitution.empty()) {
    out += metadata->standard_substitution;
    if(metadata->standard_substitution_includes_arguments) {
      return true;
    }
    return emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  const bool direct_std_prefix =
      metadata->prefix_components.size() == 1 &&
      metadata->prefix_components[0].std_abbrev;
  const bool nested = !metadata->prefix_components.empty() && !direct_std_prefix;
  SubstitutionKey template_key =
      metadata->template_name_is_template_parameter ?
          SubstitutionKey::type_template_parameter(
              metadata->template_name_parameter_index) :
      metadata->template_name_substitution.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(metadata->template_name_substitution);
  SubstitutionKey template_prefix_key =
      nested &&
              !metadata->template_name_substitution.empty() &&
              class_template_arguments_contain_pack_expansion(
                  metadata->template_arguments) ?
          SubstitutionKey::prefix(metadata->template_name_substitution) :
          SubstitutionKey::none();
  if(nested) {
    out += 'N';
  }
  if(sink && !template_key.empty() && sink->emit_substitution(template_key, out)) {
    if(!template_prefix_key.empty()) {
      sink->register_substitution_owned(std::move(template_prefix_key));
    }
    if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
      return false;
    }
    if(nested) {
      out += 'E';
    }
    return true;
  }
  if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
    return false;
  }
  if(metadata->template_name_is_template_parameter) {
    if(sink && !template_key.empty() &&
       sink->emit_substitution(template_key, out)) {
      if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
        return false;
      }
      if(nested) {
        out += 'E';
      }
      return true;
    }
    out += 'T';
    if(metadata->template_name_parameter_index > 0) {
      out += std::to_string(metadata->template_name_parameter_index - 1);
    }
    out += '_';
  } else if(!emit_source_name(metadata->template_name, out)) {
    return false;
  }
  if(sink && !template_key.empty()) {
    sink->register_substitution_owned(std::move(template_key));
  }
  if(sink && !template_prefix_key.empty()) {
    sink->register_substitution_owned(std::move(template_prefix_key));
  }
  if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool emit_class_template_specialization_type_owned(Type & type,
                                                          std::string & out,
                                                          SubstitutionSink * sink)
{
  Type::NameMetadata * metadata = type.name.get();
  if(!metadata) {
    return false;
  }
  if(type.name_owner) {
    out += 'N';
    if(!emit_type_as_name_prefix_body(type, out, sink)) {
      return false;
    }
    out += 'E';
    return true;
  }

  if(!metadata->standard_substitution.empty()) {
    out += metadata->standard_substitution;
    if(metadata->standard_substitution_includes_arguments) {
      return true;
    }
    return emit_class_template_arguments_owned(metadata->template_arguments,
                                               out,
                                               sink);
  }

  const bool direct_std_prefix =
      metadata->prefix_components.size() == 1 &&
      metadata->prefix_components[0].std_abbrev;
  const bool nested = !metadata->prefix_components.empty() && !direct_std_prefix;
  SubstitutionKey template_key =
      metadata->template_name_is_template_parameter ?
          SubstitutionKey::type_template_parameter(
              metadata->template_name_parameter_index) :
      metadata->template_name_substitution.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(metadata->template_name_substitution);
  SubstitutionKey template_prefix_key =
      nested &&
              !metadata->template_name_substitution.empty() &&
              class_template_arguments_contain_pack_expansion(
                  metadata->template_arguments) ?
          SubstitutionKey::prefix(metadata->template_name_substitution) :
          SubstitutionKey::none();
  if(nested) {
    out += 'N';
  }
  if(sink && !template_key.empty() && sink->emit_substitution(template_key, out)) {
    if(!template_prefix_key.empty()) {
      sink->register_substitution_owned(std::move(template_prefix_key));
    }
    if(!emit_class_template_arguments_owned(metadata->template_arguments,
                                            out,
                                            sink)) {
      return false;
    }
    if(nested) {
      out += 'E';
    }
    return true;
  }
  if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
    return false;
  }
  if(metadata->template_name_is_template_parameter) {
    if(sink && !template_key.empty() &&
       sink->emit_substitution(template_key, out)) {
      if(!emit_class_template_arguments_owned(metadata->template_arguments,
                                              out,
                                              sink)) {
        return false;
      }
      if(nested) {
        out += 'E';
      }
      return true;
    }
    out += 'T';
    if(metadata->template_name_parameter_index > 0) {
      out += std::to_string(metadata->template_name_parameter_index - 1);
    }
    out += '_';
  } else if(!emit_source_name(metadata->template_name, out)) {
    return false;
  }
  if(sink && !template_key.empty()) {
    sink->register_substitution_owned(std::move(template_key));
  }
  if(sink && !template_prefix_key.empty()) {
    sink->register_substitution_owned(std::move(template_prefix_key));
  }
  if(!emit_class_template_arguments_owned(metadata->template_arguments,
                                          out,
                                          sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool emit_template_entity_name(
    const std::vector<Type::NameComponent> & prefix_components,
    const std::string & template_name,
    const std::string & template_name_substitution,
    std::string & out,
    SubstitutionSink * sink)
{
  if(template_name.empty()) {
    return false;
  }
  const bool direct_std_prefix =
      prefix_components.size() == 1 &&
      prefix_components[0].std_abbrev;
  if(direct_std_prefix) {
    out += "St";
    return emit_name_component(
        Type::NameComponent::source(template_name, template_name_substitution),
        out,
        sink);
  }

  const bool nested = !prefix_components.empty();
  if(nested) {
    out += 'N';
  }
  if(!emit_name_prefix_components(prefix_components, out, sink) ||
     !emit_name_component(
         Type::NameComponent::source(template_name, template_name_substitution),
         out,
         sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool emit_member_template_entity_name(
    const Type & owner_type,
    const std::string & template_name,
    const std::string & template_name_substitution,
    std::string & out,
    SubstitutionSink * sink)
{
  if(template_name.empty()) {
    return false;
  }
  out += 'N';
  if(!emit_type_as_name_prefix_body(owner_type, out, sink) ||
     !emit_name_component(
         Type::NameComponent::source(template_name, template_name_substitution),
         out,
         sink)) {
    return false;
  }
  out += 'E';
  return true;
}

inline bool emit_external_member_entity_symbol(
    const Type * owner_type,
    const std::string & member_name,
    const std::vector<Type> & parameter_types,
    bool is_function,
    bool function_const,
    bool function_volatile,
    bool function_lvalue_ref,
    bool function_rvalue_ref,
    bool function_variadic,
    std::string & out,
    SubstitutionSink * sink)
{
  if(!owner_type || member_name.empty()) {
    return false;
  }

  out += "_ZN";
  if(function_const) {
    out += 'K';
  }
  if(function_volatile) {
    out += 'V';
  }
  if(function_lvalue_ref) {
    out += 'R';
  }
  if(function_rvalue_ref) {
    out += 'O';
  }
  if(!emit_type_as_name_prefix_body(*owner_type, out, sink) ||
     !emit_source_name(member_name, out)) {
    return false;
  }
  out += 'E';

  if(is_function) {
    if(parameter_types.empty()) {
      out += function_variadic ? 'z' : 'v';
    } else {
      for(std::size_t i = 0; i < parameter_types.size(); ++i) {
        if(!emit_type(parameter_types[i], out, sink)) {
          return false;
        }
      }
      if(function_variadic) {
        out += 'z';
      }
    }
  }
  return true;
}

inline bool emit_external_entity_argument(const std::string & symbol,
                                          bool address_of,
                                          const Type * owner_type,
                                          const std::string & member_name,
                                          const std::vector<Type> & parameter_types,
                                          bool is_member,
                                          bool is_function,
                                          bool function_const,
                                          bool function_volatile,
                                          bool function_lvalue_ref,
                                          bool function_rvalue_ref,
                                          bool function_variadic,
                                          std::string & out,
                                          SubstitutionSink * sink)
{
  if(symbol.empty()) {
    return false;
  }
  if(address_of) {
    out += "Xad";
  }
  out += 'L';
  if(!is_member ||
     !emit_external_member_entity_symbol(owner_type,
                                         member_name,
                                         parameter_types,
                                         is_function,
                                         function_const,
                                         function_volatile,
                                         function_lvalue_ref,
                                         function_rvalue_ref,
                                         function_variadic,
                                         out,
                                         sink)) {
    out += symbol;
  }
  out += 'E';
  if(address_of) {
    out += 'E';
  }
  return true;
}

inline bool emit_integral_template_value(const Type * value_type,
                                         long long value,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  if(value_type) {
    const std::size_t begin = out.size();
    out += 'L';
    if(!emit_type(*value_type, out, sink)) {
      out.resize(begin);
      return false;
    }
    out += std::to_string(value);
    out += 'E';
    return true;
  }

  out += "Li";
  out += std::to_string(value);
  out += 'E';
  return true;
}

inline bool emit_integral_template_value_owned(Type * value_type,
                                               long long value,
                                               std::string & out,
                                               SubstitutionSink * sink)
{
  if(value_type) {
    const std::size_t begin = out.size();
    out += 'L';
    if(!emit_type_owned(*value_type, out, sink)) {
      out.resize(begin);
      return false;
    }
    out += std::to_string(value);
    out += 'E';
    return true;
  }

  out += "Li";
  out += std::to_string(value);
  out += 'E';
  return true;
}

inline void register_lambda_context_substitutions(
    const std::vector<SubstitutionSlot> & slots,
    SubstitutionSink * sink)
{
  if(!sink) {
    return;
  }
  for(std::size_t i = 0; i < slots.size(); ++i) {
    if(!slots[i].ir_key.empty()) {
      sink->register_substitution(slots[i].ir_key);
    }
  }
}

inline bool emit_local_entity_context_fragment(
    const std::string & context_fragment,
    const std::vector<SubstitutionSlot> & context_substitution_slots,
    const std::shared_ptr<FunctionEncoding> & context_function,
    std::string & out,
    SubstitutionSink * sink)
{
  if(context_function) {
    out += 'Z';
    if(!emit_local_entity_context_function_encoding_body(*context_function,
                                                         out,
                                                         sink)) {
      return false;
    }
    out += 'E';
    return true;
  }
  if(context_fragment.empty()) {
    return false;
  }
  register_lambda_context_substitutions(context_substitution_slots, sink);
  out += context_fragment;
  return true;
}

inline bool emit_type_body(const Type & type, std::string & out, SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_BUILTIN:
    if(type.builtin_code[0] == '\0') {
      return false;
    }
    out += type.builtin_code;
    return true;

  case Type::TK_CV:
    if(!type.inner) {
      return false;
    }
    if(type.cv_const) {
      out += 'K';
    }
    if(type.cv_volatile) {
      out += 'V';
    }
    return emit_type(*type.inner, out, sink);

  case Type::TK_POINTER:
    if(!type.inner) {
      return false;
    }
    out += 'P';
    return emit_type(*type.inner, out, sink);

  case Type::TK_LVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'R';
    return emit_type(*type.inner, out, sink);

  case Type::TK_RVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'O';
    return emit_type(*type.inner, out, sink);

  case Type::TK_ARRAY:
    if(!type.inner) {
      return false;
    }
    out += 'A';
    out += type.array_bound;
    out += '_';
    return emit_type(*type.inner, out, sink);

  case Type::TK_FUNCTION:
    if(!type.inner) {
      return false;
    }
    out += 'F';
    if(!emit_type(*type.inner, out, sink)) {
      return false;
    }
    if(type.params.empty()) {
      out += type.variadic ? 'z' : 'v';
    } else {
      for(std::size_t i = 0; i < type.params.size(); ++i) {
        if(!emit_type(type.params[i], out, sink)) {
          return false;
        }
      }
      if(type.variadic) {
        out += 'z';
      }
    }
    if(type.function_lvalue_ref) {
      out += 'R';
    } else if(type.function_rvalue_ref) {
      out += 'O';
    }
    out += 'E';
    return true;

  case Type::TK_MEMBER_POINTER:
    if(!type.owner || !type.inner) {
      return false;
    }
    out += 'M';
    return emit_type(*type.owner, out, sink) &&
           emit_type(*type.inner, out, sink);

  case Type::TK_VENDOR_QUALIFIED:
    if(type.vendor_qualifier_name.empty() || !type.inner) {
      return false;
    }
    out += 'U';
    if(!emit_source_name(type.vendor_qualifier_name, out)) {
      return false;
    }
    return emit_type(*type.inner, out, sink);

  case Type::TK_BUILTIN_TYPE_TRANSFORM:
    if(type.builtin_transform_name.empty() || !type.inner) {
      return false;
    }
    out += 'u';
    if(!emit_source_name(type.builtin_transform_name, out)) {
      return false;
    }
    out += 'I';
    if(!emit_type(*type.inner, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_PACK_EXPANSION:
    if(!type.inner) {
      return false;
    }
    out += "Dp";
    return emit_type(*type.inner, out, sink);

  case Type::TK_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += std::to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case Type::TK_NAMED:
    return emit_named_type(type, out, sink);

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
    return emit_class_template_specialization_type(type, out, sink);

  case Type::TK_DECLTYPE_EXPRESSION:
    if(!type.expression) {
      return false;
    }
    out += "DT";
    if(!emit_dependent_expression_body(*type.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_LAMBDA_CLOSURE:
    if(!type.lambda ||
       (type.lambda->context_fragment.empty() &&
        !type.lambda->context_function)) {
      return false;
    }
    if(!emit_local_entity_context_fragment(type.lambda->context_fragment,
                                           type.lambda->context_substitution_slots,
                                           type.lambda->context_function,
                                           out,
                                           sink)) {
      return false;
    }
    if(!type.lambda->source_name.empty()) {
      if(!emit_source_name(type.lambda->source_name, out)) {
        return false;
      }
    } else {
      out += "Ul";
      if(type.params.empty()) {
        out += 'v';
      } else {
        for(std::size_t i = 0; i < type.params.size(); ++i) {
          if(!emit_type(type.params[i], out, sink)) {
            return false;
          }
        }
      }
      out += 'E';
      out += type.lambda->discriminator;
      out += '_';
    }
    return true;

  case Type::TK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_type_body_owned(Type & type, std::string & out, SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_BUILTIN:
    if(type.builtin_code[0] == '\0') {
      return false;
    }
    out += type.builtin_code;
    return true;

  case Type::TK_CV:
    if(!type.inner) {
      return false;
    }
    if(type.cv_const) {
      out += 'K';
    }
    if(type.cv_volatile) {
      out += 'V';
    }
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_POINTER:
    if(!type.inner) {
      return false;
    }
    out += 'P';
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_LVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'R';
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_RVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'O';
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_ARRAY:
    if(!type.inner) {
      return false;
    }
    out += 'A';
    out += type.array_bound;
    out += '_';
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_FUNCTION:
    if(!type.inner) {
      return false;
    }
    out += 'F';
    if(!emit_type_owned(*type.inner, out, sink)) {
      return false;
    }
    if(type.params.empty()) {
      out += type.variadic ? 'z' : 'v';
    } else {
      for(std::size_t i = 0; i < type.params.size(); ++i) {
        if(!emit_type_owned(type.params[i], out, sink)) {
          return false;
        }
      }
      if(type.variadic) {
        out += 'z';
      }
    }
    if(type.function_lvalue_ref) {
      out += 'R';
    } else if(type.function_rvalue_ref) {
      out += 'O';
    }
    out += 'E';
    return true;

  case Type::TK_MEMBER_POINTER:
    if(!type.owner || !type.inner) {
      return false;
    }
    out += 'M';
    return emit_type_owned(*type.owner, out, sink) &&
           emit_type_owned(*type.inner, out, sink);

  case Type::TK_VENDOR_QUALIFIED:
    if(type.vendor_qualifier_name.empty() || !type.inner) {
      return false;
    }
    out += 'U';
    if(!emit_source_name(type.vendor_qualifier_name, out)) {
      return false;
    }
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_BUILTIN_TYPE_TRANSFORM:
    if(type.builtin_transform_name.empty() || !type.inner) {
      return false;
    }
    out += 'u';
    if(!emit_source_name(type.builtin_transform_name, out)) {
      return false;
    }
    out += 'I';
    if(!emit_type_owned(*type.inner, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_PACK_EXPANSION:
    if(!type.inner) {
      return false;
    }
    out += "Dp";
    return emit_type_owned(*type.inner, out, sink);

  case Type::TK_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += std::to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case Type::TK_NAMED:
    return emit_named_type(type, out, sink);

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
    return emit_class_template_specialization_type_owned(type, out, sink);

  case Type::TK_DECLTYPE_EXPRESSION:
    if(!type.expression) {
      return false;
    }
    out += "DT";
    if(!emit_dependent_expression_body(*type.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_LAMBDA_CLOSURE:
    if(!type.lambda ||
       (type.lambda->context_fragment.empty() &&
        !type.lambda->context_function)) {
      return false;
    }
    if(!emit_local_entity_context_fragment(type.lambda->context_fragment,
                                           type.lambda->context_substitution_slots,
                                           type.lambda->context_function,
                                           out,
                                           sink)) {
      return false;
    }
    if(!type.lambda->source_name.empty()) {
      if(!emit_source_name(type.lambda->source_name, out)) {
        return false;
      }
    } else {
      out += "Ul";
      if(type.params.empty()) {
        out += 'v';
      } else {
        for(std::size_t i = 0; i < type.params.size(); ++i) {
          if(!emit_type_owned(type.params[i], out, sink)) {
            return false;
          }
        }
      }
      out += 'E';
      out += type.lambda->discriminator;
      out += '_';
    }
    return true;

  case Type::TK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_type(const Type & type, std::string & out, SubstitutionSink * sink)
{
  const SubstitutionKey & substitution_key = type_substitution_key(type);
  if(sink && !substitution_key.empty() &&
     sink->emit_substitution(substitution_key, out)) {
    return true;
  }

  const std::size_t begin = out.size();
  if(!emit_type_body(type, out, sink)) {
    out.resize(begin);
    return false;
  }

  if(sink && !substitution_key.empty()) {
    sink->register_substitution(substitution_key);
  }
  return true;
}

inline bool emit_type_owned(Type & type, std::string & out, SubstitutionSink * sink)
{
  SubstitutionKey * const substitution_key =
      type.substitution ? &type.substitution->key : nullptr;
  if(sink &&
     substitution_key &&
     !substitution_key->empty() &&
     sink->emit_substitution(*substitution_key, out)) {
    return true;
  }

  const std::size_t begin = out.size();
  if(!emit_type_body_owned(type, out, sink)) {
    out.resize(begin);
    return false;
  }

  if(sink && substitution_key && !substitution_key->empty()) {
    sink->register_substitution_owned(std::move(*substitution_key));
  }
  return true;
}

inline bool SubstitutionSink::emit_dependent_parameter_type(
    const Type & type,
    std::string & out)
{
  return emit_type(type, out, this);
}

struct SuppressTemplateParameterTypeSubstitutionSink : SubstitutionSink
{
  explicit SuppressTemplateParameterTypeSubstitutionSink(SubstitutionSink * inner)
      : inner(inner)
  {
  }

  bool emit_substitution(const SubstitutionKey & key,
                         std::string & out) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
      return false;
    }
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER || !inner) {
      return;
    }
    inner->register_substitution(key);
  }

  void register_substitution_owned(SubstitutionKey key) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER || !inner) {
      return;
    }
    inner->register_substitution_owned(std::move(key));
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return true;
  }

  SubstitutionSink * inner;
};

struct EmitTemplateParameterTypeDirectSubstitutionSink : SubstitutionSink
{
  explicit EmitTemplateParameterTypeDirectSubstitutionSink(SubstitutionSink * inner)
      : inner(inner)
  {
  }

  bool emit_substitution(const SubstitutionKey & key,
                         std::string & out) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
      return false;
    }
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    if(inner) {
      inner->register_substitution(key);
    }
  }

  void register_substitution_owned(SubstitutionKey key) override
  {
    if(inner) {
      inner->register_substitution_owned(std::move(key));
    }
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return inner &&
           inner->suppress_template_parameter_type_substitution_in_template_argument();
  }

  SubstitutionSink * inner;
};

struct TypeTraitOperandSubstitutionSink : SubstitutionSink
{
  explicit TypeTraitOperandSubstitutionSink(SubstitutionSink * inner)
      : inner(inner)
  {
  }

  bool emit_substitution(const SubstitutionKey & key,
                         std::string & out) override
  {
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    if(inner) {
      inner->register_substitution(key);
    }
  }

  void register_substitution_owned(SubstitutionKey key) override
  {
    if(inner) {
      inner->register_substitution_owned(std::move(key));
    }
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return true;
  }

  SubstitutionSink * inner;
};

inline bool emit_class_template_argument(
    const Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE: {
    if(!argument.type) {
      return false;
    }
    SuppressTemplateParameterTypeSubstitutionSink argument_sink(sink);
    return emit_type(
        *argument.type,
        out,
        sink &&
                sink->suppress_template_parameter_type_substitution_in_template_argument() ?
            &argument_sink :
            sink);
  }

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE: {
    if(!argument.type) {
      return false;
    }
    return emit_integral_template_value(argument.type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);
  }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    out += "Tn";
    if(!sink ? !emit_type(*argument.parameter_type, out, sink) :
              !sink->emit_dependent_parameter_type(*argument.parameter_type, out)) {
      return false;
    }
    return emit_integral_template_value(argument.type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    out += 'X';
    if(!emit_dependent_expression_body(*argument.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    return emit_integral_template_value(nullptr, argument.integral_value, out, sink);

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    if(argument.metadata->template_name_is_template_parameter) {
      out += 'T';
      if(argument.metadata->template_parameter_index > 0) {
        out += std::to_string(argument.metadata->template_parameter_index - 1);
      }
      out += '_';
      return true;
    }
    if(argument.metadata->template_owner_type) {
      return emit_member_template_entity_name(
          *argument.metadata->template_owner_type,
          argument.metadata->template_name,
          argument.metadata->template_name_substitution,
          out,
          sink);
    }
    return emit_template_entity_name(argument.metadata->prefix_components,
                                     argument.metadata->template_name,
                                     argument.metadata->template_name_substitution,
                                     out,
                                     sink);

  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    return argument.metadata &&
           emit_external_entity_argument(argument.metadata->external_entity_symbol,
                                         argument.metadata->external_entity_address_of,
                                         argument.metadata->external_entity_owner_type.get(),
                                         argument.metadata->external_entity_member_name,
                                         argument.metadata->external_entity_parameter_types,
                                         argument.metadata->external_entity_is_member,
                                         argument.metadata->external_entity_is_function,
                                         argument.metadata->external_entity_function_const,
                                         argument.metadata->external_entity_function_volatile,
                                         argument.metadata->external_entity_function_lvalue_ref,
                                         argument.metadata->external_entity_function_rvalue_ref,
                                         argument.metadata->external_entity_function_variadic,
                                         out,
                                         sink);

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    out += 'J';
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(!emit_class_template_argument(argument.metadata->pack_arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_class_template_argument_owned(
    Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE: {
    if(!argument.type) {
      return false;
    }
    SuppressTemplateParameterTypeSubstitutionSink argument_sink(sink);
    return emit_type_owned(
        *argument.type,
        out,
        sink &&
                sink->suppress_template_parameter_type_substitution_in_template_argument() ?
            &argument_sink :
            sink);
  }

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    out += 'J';
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(!emit_class_template_argument_owned(
             argument.metadata->pack_arguments[i],
             out,
             sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE: {
    if(!argument.type) {
      return false;
    }
    return emit_integral_template_value_owned(argument.type.get(),
                                             argument.integral_value,
                                             out,
                                             sink);
  }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    out += "Tn";
    if(!sink ? !emit_type_owned(*argument.parameter_type, out, sink) :
              !sink->emit_dependent_parameter_type(*argument.parameter_type, out)) {
      return false;
    }
    return emit_integral_template_value_owned(argument.type.get(),
                                             argument.integral_value,
                                             out,
                                             sink);

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(argument.metadata && argument.metadata->template_owner_type) {
      return emit_member_template_entity_name(
          *argument.metadata->template_owner_type,
          argument.metadata->template_name,
          argument.metadata->template_name_substitution,
          out,
          sink);
    }
    return emit_class_template_argument(argument, out, sink);

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
  case Type::ClassTemplateArgument::CTAK_INVALID:
    return emit_class_template_argument(argument, out, sink);
  }
  return false;
}

inline bool emit_class_template_arguments(
    const std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink)
{
  out += 'I';
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!emit_class_template_argument(arguments[i], out, sink)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

inline bool emit_class_template_arguments_owned(
    std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink)
{
  out += 'I';
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!emit_class_template_argument_owned(arguments[i], out, sink)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

inline bool emit_template_argument(const TemplateArgument & argument,
                                   std::string & out,
                                   SubstitutionSink * sink)
{
  switch(argument.kind) {
  case TemplateArgument::TAK_TYPE:
    return argument.value_type &&
           emit_type(*argument.value_type, out, sink);

  case TemplateArgument::TAK_INTEGRAL_VALUE: {
    return argument.value_type &&
           emit_integral_template_value(argument.value_type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);
  }

  case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    out += "Tn";
    if(!sink ? !emit_type(*argument.parameter_type, out, sink) :
              !sink->emit_dependent_parameter_type(*argument.parameter_type, out)) {
      return false;
    }
    return emit_integral_template_value(argument.value_type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);

  case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    out += 'X';
    if(!emit_dependent_expression_body(*argument.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
    return emit_integral_template_value(nullptr, argument.integral_value, out, sink);

  case TemplateArgument::TAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    if(argument.metadata->template_name_is_template_parameter) {
      out += 'T';
      if(argument.metadata->template_parameter_index > 0) {
        out += std::to_string(argument.metadata->template_parameter_index - 1);
      }
      out += '_';
      return true;
    }
    if(argument.metadata->template_owner_type) {
      return emit_member_template_entity_name(
          *argument.metadata->template_owner_type,
          argument.metadata->template_name,
          argument.metadata->template_name_substitution,
          out,
          sink);
    }
    return emit_template_entity_name(argument.metadata->prefix_components,
                                     argument.metadata->template_name,
                                     argument.metadata->template_name_substitution,
                                     out,
                                     sink);

  case TemplateArgument::TAK_EXTERNAL_ENTITY:
    return argument.metadata &&
           emit_external_entity_argument(argument.metadata->external_entity_symbol,
                                         argument.metadata->external_entity_address_of,
                                         argument.metadata->external_entity_owner_type.get(),
                                         argument.metadata->external_entity_member_name,
                                         argument.metadata->external_entity_parameter_types,
                                         argument.metadata->external_entity_is_member,
                                         argument.metadata->external_entity_is_function,
                                         argument.metadata->external_entity_function_const,
                                         argument.metadata->external_entity_function_volatile,
                                         argument.metadata->external_entity_function_lvalue_ref,
                                         argument.metadata->external_entity_function_rvalue_ref,
                                         argument.metadata->external_entity_function_variadic,
                                         out,
                                         sink);

  case TemplateArgument::TAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    out += 'J';
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(!emit_template_argument(argument.metadata->pack_arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case TemplateArgument::TAK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_template_arguments(
    const std::vector<TemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink)
{
  out += 'I';
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!emit_template_argument(arguments[i], out, sink)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

inline bool emit_source_name(const std::string & name, std::string & out)
{
  if(name.empty()) {
    return false;
  }
  out += std::to_string(name.size());
  out += name;
  return true;
}

inline bool emit_function_operator_terminal(
    FunctionOperatorTerminal terminal,
    const std::string & literal_suffix,
    std::string & out)
{
  const char * code = nullptr;
  switch(terminal) {
  case FUNCTION_OPERATOR_NEW: code = "nw"; break;
  case FUNCTION_OPERATOR_NEW_ARRAY: code = "na"; break;
  case FUNCTION_OPERATOR_DELETE: code = "dl"; break;
  case FUNCTION_OPERATOR_DELETE_ARRAY: code = "da"; break;
  case FUNCTION_OPERATOR_UNARY_PLUS: code = "ps"; break;
  case FUNCTION_OPERATOR_PLUS: code = "pl"; break;
  case FUNCTION_OPERATOR_UNARY_MINUS: code = "ng"; break;
  case FUNCTION_OPERATOR_MINUS: code = "mi"; break;
  case FUNCTION_OPERATOR_ADDRESS_OF: code = "ad"; break;
  case FUNCTION_OPERATOR_BIT_AND: code = "an"; break;
  case FUNCTION_OPERATOR_DEREFERENCE: code = "de"; break;
  case FUNCTION_OPERATOR_MULTIPLY: code = "ml"; break;
  case FUNCTION_OPERATOR_DIVIDE: code = "dv"; break;
  case FUNCTION_OPERATOR_REMAINDER: code = "rm"; break;
  case FUNCTION_OPERATOR_BIT_OR: code = "or"; break;
  case FUNCTION_OPERATOR_BIT_XOR: code = "eo"; break;
  case FUNCTION_OPERATOR_ASSIGN: code = "aS"; break;
  case FUNCTION_OPERATOR_PLUS_ASSIGN: code = "pL"; break;
  case FUNCTION_OPERATOR_MINUS_ASSIGN: code = "mI"; break;
  case FUNCTION_OPERATOR_MULTIPLY_ASSIGN: code = "mL"; break;
  case FUNCTION_OPERATOR_DIVIDE_ASSIGN: code = "dV"; break;
  case FUNCTION_OPERATOR_REMAINDER_ASSIGN: code = "rM"; break;
  case FUNCTION_OPERATOR_BIT_AND_ASSIGN: code = "aN"; break;
  case FUNCTION_OPERATOR_BIT_OR_ASSIGN: code = "oR"; break;
  case FUNCTION_OPERATOR_BIT_XOR_ASSIGN: code = "eO"; break;
  case FUNCTION_OPERATOR_SHIFT_LEFT: code = "ls"; break;
  case FUNCTION_OPERATOR_SHIFT_RIGHT: code = "rs"; break;
  case FUNCTION_OPERATOR_SHIFT_LEFT_ASSIGN: code = "lS"; break;
  case FUNCTION_OPERATOR_SHIFT_RIGHT_ASSIGN: code = "rS"; break;
  case FUNCTION_OPERATOR_EQUAL: code = "eq"; break;
  case FUNCTION_OPERATOR_NOT_EQUAL: code = "ne"; break;
  case FUNCTION_OPERATOR_LESS: code = "lt"; break;
  case FUNCTION_OPERATOR_GREATER: code = "gt"; break;
  case FUNCTION_OPERATOR_LESS_EQUAL: code = "le"; break;
  case FUNCTION_OPERATOR_GREATER_EQUAL: code = "ge"; break;
  case FUNCTION_OPERATOR_LOGICAL_NOT: code = "nt"; break;
  case FUNCTION_OPERATOR_BIT_NOT: code = "co"; break;
  case FUNCTION_OPERATOR_LOGICAL_AND: code = "aa"; break;
  case FUNCTION_OPERATOR_LOGICAL_OR: code = "oo"; break;
  case FUNCTION_OPERATOR_INCREMENT: code = "pp"; break;
  case FUNCTION_OPERATOR_DECREMENT: code = "mm"; break;
  case FUNCTION_OPERATOR_COMMA: code = "cm"; break;
  case FUNCTION_OPERATOR_MEMBER_POINTER: code = "pm"; break;
  case FUNCTION_OPERATOR_ARROW: code = "pt"; break;
  case FUNCTION_OPERATOR_CALL: code = "cl"; break;
  case FUNCTION_OPERATOR_INDEX: code = "ix"; break;
  case FUNCTION_OPERATOR_LITERAL:
    out += "li";
    return emit_source_name(literal_suffix, out);
  case FUNCTION_OPERATOR_NONE:
    break;
  }
  if(!code) {
    return false;
  }
  out += code;
  return true;
}

inline bool function_operator_terminal_from_semantic_name(
    const std::string & name,
    std::size_t explicit_parameter_count,
    bool member_function,
    FunctionOperatorTerminal & out)
{
  const bool unary_shape =
      member_function ? explicit_parameter_count == 0 :
                        explicit_parameter_count == 1;
  if(name == "plus") {
    out = unary_shape ? FUNCTION_OPERATOR_UNARY_PLUS : FUNCTION_OPERATOR_PLUS;
    return true;
  }
  if(name == "minus") {
    out = unary_shape ? FUNCTION_OPERATOR_UNARY_MINUS : FUNCTION_OPERATOR_MINUS;
    return true;
  }
  if(name == "address-of" || name == "address") {
    out = unary_shape ? FUNCTION_OPERATOR_ADDRESS_OF : FUNCTION_OPERATOR_BIT_AND;
    return true;
  }
  if(name == "deref" || name == "dereference") {
    out = unary_shape ? FUNCTION_OPERATOR_DEREFERENCE :
                        FUNCTION_OPERATOR_MULTIPLY;
    return true;
  }

  struct Entry
  {
    const char * name;
    FunctionOperatorTerminal terminal;
  };
  static const Entry entries[] = {
      {"new", FUNCTION_OPERATOR_NEW},
      {"new-array", FUNCTION_OPERATOR_NEW_ARRAY},
      {"delete", FUNCTION_OPERATOR_DELETE},
      {"delete-array", FUNCTION_OPERATOR_DELETE_ARRAY},
      {"unary-plus", FUNCTION_OPERATOR_UNARY_PLUS},
      {"binary-plus", FUNCTION_OPERATOR_PLUS},
      {"negate", FUNCTION_OPERATOR_UNARY_MINUS},
      {"unary-minus", FUNCTION_OPERATOR_UNARY_MINUS},
      {"binary-minus", FUNCTION_OPERATOR_MINUS},
      {"bit-and", FUNCTION_OPERATOR_BIT_AND},
      {"multiply", FUNCTION_OPERATOR_MULTIPLY},
      {"divide", FUNCTION_OPERATOR_DIVIDE},
      {"remainder", FUNCTION_OPERATOR_REMAINDER},
      {"mod", FUNCTION_OPERATOR_REMAINDER},
      {"bit-or", FUNCTION_OPERATOR_BIT_OR},
      {"bit-xor", FUNCTION_OPERATOR_BIT_XOR},
      {"assign", FUNCTION_OPERATOR_ASSIGN},
      {"plus-assign", FUNCTION_OPERATOR_PLUS_ASSIGN},
      {"minus-assign", FUNCTION_OPERATOR_MINUS_ASSIGN},
      {"multiply-assign", FUNCTION_OPERATOR_MULTIPLY_ASSIGN},
      {"divide-assign", FUNCTION_OPERATOR_DIVIDE_ASSIGN},
      {"remainder-assign", FUNCTION_OPERATOR_REMAINDER_ASSIGN},
      {"mod-assign", FUNCTION_OPERATOR_REMAINDER_ASSIGN},
      {"bit-and-assign", FUNCTION_OPERATOR_BIT_AND_ASSIGN},
      {"bit-or-assign", FUNCTION_OPERATOR_BIT_OR_ASSIGN},
      {"bit-xor-assign", FUNCTION_OPERATOR_BIT_XOR_ASSIGN},
      {"shift-left", FUNCTION_OPERATOR_SHIFT_LEFT},
      {"shift-right", FUNCTION_OPERATOR_SHIFT_RIGHT},
      {"shift-left-assign", FUNCTION_OPERATOR_SHIFT_LEFT_ASSIGN},
      {"shift-right-assign", FUNCTION_OPERATOR_SHIFT_RIGHT_ASSIGN},
      {"equal", FUNCTION_OPERATOR_EQUAL},
      {"not-equal", FUNCTION_OPERATOR_NOT_EQUAL},
      {"less", FUNCTION_OPERATOR_LESS},
      {"greater", FUNCTION_OPERATOR_GREATER},
      {"less-equal", FUNCTION_OPERATOR_LESS_EQUAL},
      {"greater-equal", FUNCTION_OPERATOR_GREATER_EQUAL},
      {"logical-not", FUNCTION_OPERATOR_LOGICAL_NOT},
      {"bit-not", FUNCTION_OPERATOR_BIT_NOT},
      {"logical-and", FUNCTION_OPERATOR_LOGICAL_AND},
      {"logical-or", FUNCTION_OPERATOR_LOGICAL_OR},
      {"increment", FUNCTION_OPERATOR_INCREMENT},
      {"decrement", FUNCTION_OPERATOR_DECREMENT},
      {"comma", FUNCTION_OPERATOR_COMMA},
      {"member-pointer", FUNCTION_OPERATOR_MEMBER_POINTER},
      {"arrow", FUNCTION_OPERATOR_ARROW},
      {"call", FUNCTION_OPERATOR_CALL},
      {"index", FUNCTION_OPERATOR_INDEX}};
  for(std::size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
    if(name == entries[i].name) {
      out = entries[i].terminal;
      return true;
    }
  }
  return false;
}

inline bool function_operator_terminal_from_cpp_spelling(
    const std::string & compact_spelling,
    std::size_t explicit_parameter_count,
    bool member_function,
    FunctionOperatorTerminal & out,
    std::string & literal_suffix)
{
  literal_suffix.clear();
  const std::string literal_prefix = "operator\"\"";
  if(compact_spelling.compare(0,
                              literal_prefix.size(),
                              literal_prefix) == 0) {
    literal_suffix = compact_spelling.substr(literal_prefix.size());
    out = FUNCTION_OPERATOR_LITERAL;
    return !literal_suffix.empty();
  }
  if(compact_spelling == "operator+") {
    out = (member_function ? explicit_parameter_count == 0 :
                             explicit_parameter_count == 1) ?
        FUNCTION_OPERATOR_UNARY_PLUS : FUNCTION_OPERATOR_PLUS;
    return true;
  }
  if(compact_spelling == "operator-") {
    out = (member_function ? explicit_parameter_count == 0 :
                             explicit_parameter_count == 1) ?
        FUNCTION_OPERATOR_UNARY_MINUS : FUNCTION_OPERATOR_MINUS;
    return true;
  }
  if(compact_spelling == "operator&") {
    out = (member_function ? explicit_parameter_count == 0 :
                             explicit_parameter_count == 1) ?
        FUNCTION_OPERATOR_ADDRESS_OF : FUNCTION_OPERATOR_BIT_AND;
    return true;
  }
  if(compact_spelling == "operator*") {
    out = (member_function ? explicit_parameter_count == 0 :
                             explicit_parameter_count == 1) ?
        FUNCTION_OPERATOR_DEREFERENCE : FUNCTION_OPERATOR_MULTIPLY;
    return true;
  }

  struct Entry
  {
    const char * spelling;
    FunctionOperatorTerminal terminal;
  };
  static const Entry entries[] = {
      {"operatornew", FUNCTION_OPERATOR_NEW},
      {"operatornew[]", FUNCTION_OPERATOR_NEW_ARRAY},
      {"operatordelete", FUNCTION_OPERATOR_DELETE},
      {"operatordelete[]", FUNCTION_OPERATOR_DELETE_ARRAY},
      {"operator/", FUNCTION_OPERATOR_DIVIDE},
      {"operator%", FUNCTION_OPERATOR_REMAINDER},
      {"operator|", FUNCTION_OPERATOR_BIT_OR},
      {"operator^", FUNCTION_OPERATOR_BIT_XOR},
      {"operator=", FUNCTION_OPERATOR_ASSIGN},
      {"operator+=", FUNCTION_OPERATOR_PLUS_ASSIGN},
      {"operator-=", FUNCTION_OPERATOR_MINUS_ASSIGN},
      {"operator*=", FUNCTION_OPERATOR_MULTIPLY_ASSIGN},
      {"operator/=", FUNCTION_OPERATOR_DIVIDE_ASSIGN},
      {"operator%=", FUNCTION_OPERATOR_REMAINDER_ASSIGN},
      {"operator&=", FUNCTION_OPERATOR_BIT_AND_ASSIGN},
      {"operator|=", FUNCTION_OPERATOR_BIT_OR_ASSIGN},
      {"operator^=", FUNCTION_OPERATOR_BIT_XOR_ASSIGN},
      {"operator<<", FUNCTION_OPERATOR_SHIFT_LEFT},
      {"operator>>", FUNCTION_OPERATOR_SHIFT_RIGHT},
      {"operator<<=", FUNCTION_OPERATOR_SHIFT_LEFT_ASSIGN},
      {"operator>>=", FUNCTION_OPERATOR_SHIFT_RIGHT_ASSIGN},
      {"operator==", FUNCTION_OPERATOR_EQUAL},
      {"operator!=", FUNCTION_OPERATOR_NOT_EQUAL},
      {"operator<", FUNCTION_OPERATOR_LESS},
      {"operator>", FUNCTION_OPERATOR_GREATER},
      {"operator<=", FUNCTION_OPERATOR_LESS_EQUAL},
      {"operator>=", FUNCTION_OPERATOR_GREATER_EQUAL},
      {"operator!", FUNCTION_OPERATOR_LOGICAL_NOT},
      {"operator~", FUNCTION_OPERATOR_BIT_NOT},
      {"operator&&", FUNCTION_OPERATOR_LOGICAL_AND},
      {"operator||", FUNCTION_OPERATOR_LOGICAL_OR},
      {"operator++", FUNCTION_OPERATOR_INCREMENT},
      {"operator--", FUNCTION_OPERATOR_DECREMENT},
      {"operator,", FUNCTION_OPERATOR_COMMA},
      {"operator->*", FUNCTION_OPERATOR_MEMBER_POINTER},
      {"operator->", FUNCTION_OPERATOR_ARROW},
      {"operator()", FUNCTION_OPERATOR_CALL},
      {"operator[]", FUNCTION_OPERATOR_INDEX}};
  for(std::size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
    if(compact_spelling == entries[i].spelling) {
      out = entries[i].terminal;
      return true;
    }
  }
  return false;
}

inline bool function_operator_terminal_substitution_text(
    FunctionOperatorTerminal terminal,
    const std::string & literal_suffix,
    std::string & out)
{
  std::string terminal_text;
  if(!emit_function_operator_terminal(terminal, literal_suffix, terminal_text)) {
    return false;
  }
  out = std::string("operator-name:") + terminal_text;
  return true;
}

inline bool emit_dependent_expression_body(const DependentExpression & expression,
                                           std::string & out,
                                           SubstitutionSink * sink)
{
  switch(expression.kind) {
  case DependentExpression::EK_TEMPLATE_PARAMETER:
    out += 'T';
    if(expression.template_parameter_index > 0) {
      out += std::to_string(expression.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case DependentExpression::EK_FUNCTION_PARAMETER:
    out += "fp";
    if(expression.template_parameter_index > 0) {
      out += std::to_string(expression.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case DependentExpression::EK_LITERAL:
    if(expression.text.empty()) {
      return false;
    }
    out += "Li";
    out += expression.text[0] == '+' ? expression.text.substr(1) :
                                       expression.text;
    out += 'E';
    return true;

  case DependentExpression::EK_INTEGRAL_VALUE:
    if(!expression.owner_type) {
      return false;
    }
    return emit_integral_template_value(expression.owner_type.get(),
                                        expression.integral_value,
                                        out,
                                        sink);

  case DependentExpression::EK_MEMBER:
    if(!expression.owner_type || expression.text.empty()) {
      return false;
    }
    out += "sr";
    if(expression.suppress_member_owner_prefix ?
           !emit_type_as_unqualified_member_expression_owner_prefix_body(
               *expression.owner_type,
               out,
               sink) :
           !emit_type_as_member_expression_owner_prefix_body(
               *expression.owner_type,
               out,
               sink)) {
      return false;
    }
    if(expression.close_member_owner) {
      out += 'E';
    }
    if(!emit_source_name(expression.text, out)) {
      return false;
    }
    if(!expression.template_arguments.empty()) {
      if(sink && !expression.suppress_template_prefix_substitution) {
        sink->register_substitution_owned(
            SubstitutionKey::function_template_prefix(expression.text));
      }
      if(!emit_template_arguments(expression.template_arguments, out, sink)) {
        return false;
      }
    }
    return true;

  case DependentExpression::EK_OBJECT_MEMBER:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.text.empty()) {
      return false;
    }
    out += expression.op_code;
    if(!emit_dependent_expression_body(*expression.inner, out, sink) ||
       !emit_source_name(expression.text, out)) {
      return false;
    }
    if(!expression.template_arguments.empty()) {
      if(sink && !expression.suppress_template_prefix_substitution) {
        sink->register_substitution_owned(
            SubstitutionKey::function_template_prefix(expression.text));
      }
      if(!emit_template_arguments(expression.template_arguments, out, sink)) {
        return false;
      }
    }
    return true;

  case DependentExpression::EK_UNARY:
    if(expression.op_code.empty() || !expression.inner) {
      return false;
    }
    out += expression.op_code;
    return emit_dependent_expression_body(*expression.inner, out, sink);

  case DependentExpression::EK_BINARY:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.arguments.size() != 1) {
      return false;
    }
    out += expression.op_code;
    return emit_dependent_expression_body(*expression.inner, out, sink) &&
           emit_dependent_expression_body(expression.arguments[0], out, sink);

  case DependentExpression::EK_CONDITIONAL:
    if(!expression.inner || expression.arguments.size() != 2) {
      return false;
    }
    out += "qu";
    return emit_dependent_expression_body(*expression.inner, out, sink) &&
           emit_dependent_expression_body(expression.arguments[0], out, sink) &&
           emit_dependent_expression_body(expression.arguments[1], out, sink);

  case DependentExpression::EK_PACK_EXPANSION:
    if(!expression.inner) {
      return false;
    }
    out += "sp";
    return emit_dependent_expression_body(*expression.inner, out, sink);

  case DependentExpression::EK_CALL:
    if(!expression.inner) {
      return false;
    }
    out += "cl";
    if(!emit_dependent_expression_body(*expression.inner, out, sink)) {
      return false;
    }
    for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
      if(!emit_dependent_expression_body(expression.arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case DependentExpression::EK_CONVERSION:
    if(!expression.owner_type) {
      return false;
    }
    {
      const std::string op_code =
          expression.op_code.empty() ? std::string("cv") : expression.op_code;
      out += op_code;
      if(!emit_type(*expression.owner_type, out, sink)) {
        return false;
      }
      if(op_code == "cv") {
        out += '_';
        for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
          if(!emit_dependent_expression_body(expression.arguments[i], out, sink)) {
            return false;
          }
        }
        out += 'E';
        return true;
      }
      if(expression.arguments.size() != 1 ||
         !emit_dependent_expression_body(expression.arguments[0], out, sink)) {
        return false;
      }
      return true;
    }

  case DependentExpression::EK_TEMPLATE_ID:
    if(expression.text.empty() ||
       !emit_source_name(expression.text, out)) {
      return false;
    }
    if(sink && !expression.suppress_template_prefix_substitution) {
      sink->register_substitution_owned(
          SubstitutionKey::function_template_prefix(expression.text));
    }
    if(!emit_template_arguments(expression.template_arguments, out, sink)) {
      return false;
    }
    return true;

  case DependentExpression::EK_TYPE_TRAIT: {
    if(expression.text.empty()) {
      return false;
    }
    out += 'u';
    if(!emit_source_name(expression.text, out)) {
      return false;
    }
    TypeTraitOperandSubstitutionSink type_trait_sink(sink);
    for(std::size_t i = 0; i < expression.type_arguments.size(); ++i) {
      if(!emit_type(expression.type_arguments[i],
                    out,
                    sink ? &type_trait_sink : sink)) {
        return false;
      }
    }
    out += 'E';
    return true;
  }

  case DependentExpression::EK_SIZEOF_TYPE:
    if(!expression.owner_type) {
      return false;
    }
    out += "st";
    return emit_type(*expression.owner_type, out, sink);

  case DependentExpression::EK_EXTERNAL_ENTITY:
    return emit_external_entity_argument(expression.text,
                                         expression.external_entity_address_of,
                                         nullptr,
                                         std::string(),
                                         std::vector<Type>(),
                                         false,
                                         false,
                                         false,
                                         false,
                                         false,
                                         false,
                                         false,
                                         out,
                                         sink);

  case DependentExpression::EK_INVALID:
    return false;
  }
  return false;
}

inline void emit_abi_tags(const std::vector<std::string> & abi_tags,
                          std::string & out)
{
  if(abi_tags.empty()) {
    return;
  }

  std::vector<std::string> sorted = abi_tags;
  for(std::size_t i = 1; i < sorted.size(); ++i) {
    std::string value = sorted[i];
    std::size_t j = i;
    while(j > 0 && value < sorted[j - 1]) {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = value;
  }

  std::vector<std::string> unique_tags;
  unique_tags.reserve(sorted.size());
  for(std::size_t i = 0; i < sorted.size(); ++i) {
    if(sorted[i].empty()) {
      continue;
    }
    if(!unique_tags.empty() && unique_tags.back() == sorted[i]) {
      continue;
    }
    unique_tags.push_back(sorted[i]);
  }

  for(std::size_t i = 0; i < unique_tags.size(); ++i) {
    out += 'B';
    out += std::to_string(unique_tags[i].size());
    out += unique_tags[i];
  }
}

inline SubstitutionKey function_name_component_prefix_key(
    const FunctionNameComponent & component)
{
  if(!component.standard_substitution.empty() &&
     component.standard_substitution_includes_arguments) {
    return SubstitutionKey::none();
  }
  if(!component.complete_ir_substitution_key.empty()) {
    return component.complete_ir_substitution_key.get();
  }
  if(!component.complete_substitution_name.empty()) {
    return SubstitutionKey::named(component.complete_substitution_name);
  }
  if(!component.ir_substitution_key.empty()) {
    return component.ir_substitution_key.get();
  }
  if(!component.substitution_name.empty()) {
    return SubstitutionKey::named(component.substitution_name);
  }
  return SubstitutionKey::none();
}

inline SubstitutionKey function_name_component_name_key(
    const FunctionNameComponent & component)
{
  if(!component.ir_substitution_key.empty()) {
    return component.ir_substitution_key.get();
  }
  return component.substitution_name.empty() ?
      SubstitutionKey::none() :
      SubstitutionKey::named(component.substitution_name);
}

inline SubstitutionKey function_name_component_complete_key(
    const FunctionNameComponent & component)
{
  if(!component.complete_ir_substitution_key.empty()) {
    return component.complete_ir_substitution_key.get();
  }
  return component.complete_substitution_name.empty() ?
      SubstitutionKey::none() :
      SubstitutionKey::named(component.complete_substitution_name);
}

inline bool emit_function_name_component(const FunctionNameComponent & component,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  if(component.std_abbrev) {
    out += "St";
    return true;
  }
  if(component.source_name.empty()) {
    return false;
  }

  if(!component.standard_substitution.empty()) {
    out += component.standard_substitution;
    if(!component.standard_substitution_includes_arguments &&
       !emit_template_arguments(component.template_arguments, out, sink)) {
      return false;
    }
    SubstitutionKey complete_key =
        function_name_component_complete_key(component);
    if(sink &&
       !component.standard_substitution_includes_arguments &&
       !complete_key.empty()) {
      sink->register_substitution_owned(std::move(complete_key));
    }
    return true;
  }

  SubstitutionKey name_key = function_name_component_name_key(component);
  if(sink && !name_key.empty() && sink->emit_substitution(name_key, out)) {
    if(!component.template_arguments.empty() &&
       !emit_template_arguments(component.template_arguments, out, sink)) {
      return false;
    }
    SubstitutionKey complete_key =
        function_name_component_complete_key(component);
    if(sink && !complete_key.empty()) {
      sink->register_substitution_owned(std::move(complete_key));
    }
    return true;
  }
  if(!emit_source_name(component.source_name, out)) {
    return false;
  }
  if(sink && !name_key.empty()) {
    sink->register_substitution_owned(std::move(name_key));
  }
  if(!component.template_arguments.empty() &&
     !emit_template_arguments(component.template_arguments, out, sink)) {
    return false;
  }
  SubstitutionKey complete_key =
      function_name_component_complete_key(component);
  if(sink && !complete_key.empty()) {
    sink->register_substitution_owned(std::move(complete_key));
  }
  return true;
}

inline bool emit_function_name_prefix_components(
    const std::vector<FunctionNameComponent> & components,
    std::string & out,
    SubstitutionSink * sink)
{
  std::size_t start = 0;
  if(sink) {
    for(std::size_t len = components.size(); len > 0; --len) {
      const SubstitutionKey key =
          function_name_component_prefix_key(components[len - 1]);
      if(key.empty()) {
        continue;
      }
      if(sink->emit_substitution(key, out)) {
        start = len;
        break;
      }
    }
  }
  for(std::size_t i = start; i < components.size(); ++i) {
    if(!emit_function_name_component(components[i], out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_function_name(const FunctionEncoding & function,
                               std::string & out,
                               SubstitutionSink * sink)
{
  if(function.lambda) {
    const FunctionEncoding::LambdaMetadata & lambda = *function.lambda;
    if(lambda.context_fragment.empty() && !lambda.context_function) {
      return false;
    }
    if(!emit_local_entity_context_fragment(lambda.context_fragment,
                                           lambda.context_substitution_slots,
                                           lambda.context_function,
                                           out,
                                           sink)) {
      return false;
    }
    out += 'N';
    if(function.nested_const) {
      out += 'K';
    }
    if(function.nested_volatile) {
      out += 'V';
    }
    if(function.nested_lvalue_ref) {
      out += 'R';
    } else if(function.nested_rvalue_ref) {
      out += 'O';
    }
    if(!lambda.source_name.empty()) {
      if(!emit_source_name(lambda.source_name, out)) {
        return false;
      }
    } else {
      out += "Ul";
      if(lambda.signature_parameter_types.empty()) {
        out += 'v';
      } else {
        for(std::size_t i = 0;
            i < lambda.signature_parameter_types.size();
            ++i) {
          if(!emit_type(lambda.signature_parameter_types[i],
                        out,
                        sink)) {
            return false;
          }
        }
      }
      out += 'E';
      out += lambda.discriminator;
      out += '_';
    }
    if(sink) {
      SubstitutionKey lambda_key;
      if(!make_lambda_closure_substitution_key(lambda.context_fragment,
                                               lambda.context_function,
                                               lambda.source_name,
                                               lambda.signature_parameter_types,
                                               lambda.discriminator,
                                               lambda_key)) {
        return false;
      }
      sink->register_substitution_owned(std::move(lambda_key));
    }
    if(function.has_conversion_type && function.conversion_type) {
      out += "cv";
      if(!emit_type(*function.conversion_type, out, nullptr)) {
        return false;
      }
    } else if(!function.terminal_source_name.empty()) {
      if(!emit_source_name(function.terminal_source_name, out)) {
        return false;
      }
    } else if(function.operator_terminal != FUNCTION_OPERATOR_NONE) {
      if(!emit_function_operator_terminal(function.operator_terminal,
                                          function.operator_literal_suffix,
                                          out)) {
        return false;
      }
    } else {
      out += function.terminal_fragment.empty() ?
          std::string("cl") :
          function.terminal_fragment;
    }
    emit_abi_tags(function.abi_tags, out);
    if(!function.template_arguments.empty()) {
      if(sink && !function.template_prefix_key.empty()) {
        sink->register_substitution(function.template_prefix_key.get());
      }
      if(!emit_template_arguments(function.template_arguments, out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;
  }

  if(!function.name_components.empty()) {
    const bool direct_std_prefix =
        function.name_components.size() == 2 &&
        function.name_components[0].std_abbrev;
    const bool nested = function.name_components.size() > 1 && !direct_std_prefix;
    if(nested) {
      out += 'N';
      if(function.nested_const) {
        out += 'K';
      }
      if(function.nested_volatile) {
        out += 'V';
      }
      if(function.nested_lvalue_ref) {
        out += 'R';
      } else if(function.nested_rvalue_ref) {
        out += 'O';
      }
    }
    if(!function.template_arguments.empty() &&
       sink &&
       !function.template_prefix_key.empty() &&
       sink->emit_substitution(function.template_prefix_key.get(), out)) {
      if(!emit_template_arguments(function.template_arguments, out, sink)) {
        return false;
      }
      if(nested) {
        out += 'E';
      }
      return true;
    }
    const std::size_t terminal = function.name_components.size() - 1;
    if(direct_std_prefix) {
      out += "St";
    } else if(terminal != 0) {
      std::vector<FunctionNameComponent> prefix_components(
          function.name_components.begin(),
          function.name_components.begin() + terminal);
      if(!emit_function_name_prefix_components(prefix_components, out, sink)) {
        return false;
      }
    }
    if(function.has_conversion_type && function.conversion_type) {
      out += "cv";
      if(!emit_type(*function.conversion_type,
                    out,
                    nullptr)) {
        return false;
      }
    } else if(function.operator_terminal != FUNCTION_OPERATOR_NONE) {
      if(!emit_function_operator_terminal(function.operator_terminal,
                                          function.operator_literal_suffix,
                                          out)) {
        return false;
      }
    } else if(!function.terminal_fragment.empty()) {
      out += function.terminal_fragment;
    } else if(!emit_function_name_component(function.name_components[terminal],
                                            out,
                                            sink)) {
      return false;
    }
    emit_abi_tags(function.abi_tags, out);
    if(!function.template_arguments.empty()) {
      if(sink && !function.template_prefix_key.empty()) {
        sink->register_substitution(function.template_prefix_key.get());
      }
      if(!emit_template_arguments(function.template_arguments, out, sink)) {
        return false;
      }
    }
    if(nested) {
      out += 'E';
    }
    return true;
  }
  if(function.name_fragment.empty() &&
     function.operator_terminal == FUNCTION_OPERATOR_NONE &&
     !function.has_conversion_type) {
    return false;
  }
  if(!function.template_arguments.empty() &&
     sink &&
     !function.template_prefix_key.empty() &&
     sink->emit_substitution(function.template_prefix_key.get(), out)) {
    return emit_template_arguments(function.template_arguments, out, sink);
  }
  if(function.has_conversion_type && function.conversion_type) {
    out += "cv";
    if(!emit_type(*function.conversion_type,
                  out,
                  nullptr)) {
      return false;
    }
  } else if(function.operator_terminal != FUNCTION_OPERATOR_NONE) {
    if(!emit_function_operator_terminal(function.operator_terminal,
                                        function.operator_literal_suffix,
                                        out)) {
      return false;
    }
  } else {
    out += function.name_fragment;
  }
  emit_abi_tags(function.abi_tags, out);
  if(!function.template_arguments.empty()) {
    if(sink && !function.template_prefix_key.empty()) {
      sink->register_substitution(function.template_prefix_key.get());
    }
    if(!emit_template_arguments(function.template_arguments, out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_function_encoding(const FunctionEncoding & function,
                                   std::string & out,
                                   SubstitutionSink * sink)
{
  out += "_Z";
  return emit_function_encoding_body(function, out, sink);
}

inline bool emit_function_encoding_owned(FunctionEncoding & function,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  out += "_Z";
  return emit_function_encoding_body_owned(function, out, sink);
}

inline bool emit_local_entity_context_function_type(const Type & type,
                                                    std::string & out,
                                                    SubstitutionSink * sink)
{
  if(type.kind == Type::TK_TEMPLATE_PARAMETER && sink) {
    EmitTemplateParameterTypeDirectSubstitutionSink direct_sink(sink);
    return emit_type(type, out, &direct_sink);
  }
  return emit_type(type, out, sink);
}

inline bool emit_function_encoding_body(const FunctionEncoding & function,
                                        std::string & out,
                                        SubstitutionSink * sink)
{
  if(!emit_function_name(function, out, sink)) {
    return false;
  }
  if(function.has_result_type &&
     (!function.result_type || !emit_type(*function.result_type, out, sink))) {
    return false;
  }
  if(function.parameter_types.empty()) {
    out += function.variadic ? 'z' : 'v';
    return true;
  }
  for(std::size_t i = 0; i < function.parameter_types.size(); ++i) {
    if(!emit_type(function.parameter_types[i], out, sink)) {
      return false;
    }
  }
  if(function.variadic) {
    out += 'z';
  }
  return true;
}

inline bool emit_function_encoding_body_owned(FunctionEncoding & function,
                                             std::string & out,
                                             SubstitutionSink * sink)
{
  if(!emit_function_name(function, out, sink)) {
    return false;
  }
  if(function.has_result_type &&
     (!function.result_type ||
      !emit_type_owned(*function.result_type, out, sink))) {
    return false;
  }
  if(function.parameter_types.empty()) {
    out += function.variadic ? 'z' : 'v';
    return true;
  }
  for(std::size_t i = 0; i < function.parameter_types.size(); ++i) {
    if(!emit_type_owned(function.parameter_types[i], out, sink)) {
      return false;
    }
  }
  if(function.variadic) {
    out += 'z';
  }
  return true;
}

inline bool emit_local_entity_context_function_encoding_body(
    const FunctionEncoding & function,
    std::string & out,
    SubstitutionSink * sink)
{
  if(!emit_function_name(function, out, sink)) {
    return false;
  }
  if(function.parameter_types.empty()) {
    out += function.variadic ? 'z' : 'v';
    return true;
  }
  for(std::size_t i = 0; i < function.parameter_types.size(); ++i) {
    if(!emit_local_entity_context_function_type(function.parameter_types[i],
                                               out,
                                               sink)) {
      return false;
    }
  }
  if(function.variadic) {
    out += 'z';
  }
  return true;
}

enum SpecialTypeSymbolKind
{
  SPECIAL_TYPEINFO,
  SPECIAL_TYPEINFO_NAME,
  SPECIAL_VTABLE,
  SPECIAL_VTT
};

enum SpecialMemberEntryKind
{
  SPECIAL_MEMBER_COMPLETE,
  SPECIAL_MEMBER_BASE,
  SPECIAL_MEMBER_DELETING
};

inline const char * special_type_symbol_prefix(SpecialTypeSymbolKind kind)
{
  switch(kind) {
  case SPECIAL_TYPEINFO:
    return "_ZTI";
  case SPECIAL_TYPEINFO_NAME:
    return "_ZTS";
  case SPECIAL_VTABLE:
    return "_ZTV";
  case SPECIAL_VTT:
    return "_ZTT";
  }
  return "";
}

inline bool emit_special_type_symbol_from_encoding(
    SpecialTypeSymbolKind kind,
    const std::string & type_encoding,
    std::string & out)
{
  if(type_encoding.empty()) {
    return false;
  }
  out = special_type_symbol_prefix(kind);
  if(out.empty()) {
    return false;
  }
  out += type_encoding;
  return true;
}

inline bool emit_special_type_symbol(SpecialTypeSymbolKind kind,
                                     const Type & type,
                                     std::string & out,
                                     SubstitutionSink * sink)
{
  std::string type_encoding;
  if(!emit_type(type, type_encoding, sink)) {
    return false;
  }
  return emit_special_type_symbol_from_encoding(kind, type_encoding, out);
}

inline bool emit_construction_vtable_symbol_from_encodings(
    const std::string & dynamic_type_encoding,
    unsigned long long base_offset,
    const std::string & base_type_encoding,
    std::string & out)
{
  if(dynamic_type_encoding.empty() || base_type_encoding.empty()) {
    return false;
  }
  out = "_ZTC";
  out += dynamic_type_encoding;
  out += std::to_string(base_offset);
  out += '_';
  out += base_type_encoding;
  return true;
}

inline bool emit_construction_vtable_symbol(const Type & dynamic_type,
                                            unsigned long long base_offset,
                                            const Type & base_type,
                                            std::string & out,
                                            SubstitutionSink * sink)
{
  std::string dynamic_type_encoding;
  std::string base_type_encoding;
  if(!emit_type(dynamic_type, dynamic_type_encoding, sink) ||
     !emit_type(base_type, base_type_encoding, sink)) {
    return false;
  }
  return emit_construction_vtable_symbol_from_encodings(dynamic_type_encoding,
                                                        base_offset,
                                                        base_type_encoding,
                                                        out);
}

inline bool object_symbol_body(const std::string & object_symbol,
                               std::string & out)
{
  if(object_symbol.size() < 2 ||
     object_symbol[0] != '_' ||
     object_symbol[1] != 'Z') {
    return false;
  }
  out = object_symbol.substr(2);
  return true;
}

inline bool emit_thread_local_wrapper_symbol_from_encoding(
    const std::string & name_encoding,
    std::string & out)
{
  if(name_encoding.empty()) {
    return false;
  }
  out = "_ZTW";
  out += name_encoding;
  return true;
}

inline bool emit_thread_local_wrapper_symbol_from_symbol(
    const std::string & object_symbol,
    std::string & out)
{
  std::string body;
  if(!object_symbol_body(object_symbol, body)) {
    return false;
  }
  return emit_thread_local_wrapper_symbol_from_encoding(body, out);
}

inline std::string encode_abi_offset(long long value)
{
  return value < 0 ? std::string("n") + std::to_string(-value) :
                     std::to_string(value);
}

inline std::string encode_nonvirtual_call_offset(long long value)
{
  return std::string("h") + encode_abi_offset(value) + "_";
}

inline bool emit_virtual_override_thunk_symbol_from_encoding(
    const std::string & target_body,
    long long this_adjust,
    bool has_result_adjust,
    long long result_adjust,
    std::string & out);

inline bool emit_virtual_base_override_thunk_symbol_from_encoding(
    const std::string & target_body,
    long long vcall_offset,
    std::string & out);

inline bool emit_virtual_override_thunk_symbol(
    const std::string & target_object_symbol,
    long long this_adjust,
    bool has_result_adjust,
    long long result_adjust,
    std::string & out)
{
  std::string target_body;
  if(!object_symbol_body(target_object_symbol, target_body)) {
    return false;
  }
  return emit_virtual_override_thunk_symbol_from_encoding(target_body,
                                                          this_adjust,
                                                          has_result_adjust,
                                                          result_adjust,
                                                          out);
}

inline bool emit_virtual_override_thunk_symbol_from_encoding(
    const std::string & target_body,
    long long this_adjust,
    bool has_result_adjust,
    long long result_adjust,
    std::string & out)
{
  if(target_body.empty()) {
    return false;
  }
  if(has_result_adjust) {
    out = "_ZTc";
    out += encode_nonvirtual_call_offset(this_adjust);
    out += encode_nonvirtual_call_offset(result_adjust);
    out += target_body;
    return true;
  }
  out = "_ZT";
  out += encode_nonvirtual_call_offset(this_adjust);
  out += target_body;
  return true;
}

inline bool emit_virtual_base_override_thunk_symbol(
    const std::string & target_object_symbol,
    long long vcall_offset,
    std::string & out)
{
  std::string target_body;
  if(!object_symbol_body(target_object_symbol, target_body)) {
    return false;
  }
  return emit_virtual_base_override_thunk_symbol_from_encoding(target_body,
                                                               vcall_offset,
                                                               out);
}

inline bool emit_virtual_base_override_thunk_symbol_from_encoding(
    const std::string & target_body,
    long long vcall_offset,
    std::string & out)
{
  if(target_body.empty()) {
    return false;
  }
  out = "_ZTv0_";
  out += encode_abi_offset(vcall_offset);
  out += '_';
  out += target_body;
  return true;
}

inline bool emit_function_name_symbol(const FunctionEncoding & function,
                                      std::string & out,
                                      SubstitutionSink * sink)
{
  out = "_Z";
  return emit_function_name(function, out, sink);
}

inline bool special_member_entry_code(bool is_constructor,
                                      SpecialMemberEntryKind entry_kind,
                                      const char *& out)
{
  switch(entry_kind) {
  case SPECIAL_MEMBER_COMPLETE:
    out = is_constructor ? "C1" : "D1";
    return true;
  case SPECIAL_MEMBER_BASE:
    out = is_constructor ? "C2" : "D2";
    return true;
  case SPECIAL_MEMBER_DELETING:
    if(is_constructor) {
      return false;
    }
    out = "D0";
    return true;
  }
  return false;
}

inline bool find_special_member_entry_token(const std::string & object_symbol,
                                            const std::string & token,
                                            std::size_t & pos)
{
  pos = object_symbol.rfind(token);
  while(pos != std::string::npos) {
    const std::size_t after = pos + token.size();
    if(after < object_symbol.size() &&
       (object_symbol[after] == 'E' ||
        object_symbol[after] == 'B' ||
        object_symbol[after] == 'I')) {
      return true;
    }
    if(pos == 0) {
      break;
    }
    pos = object_symbol.rfind(token, pos - 1);
  }
  return false;
}

inline bool replace_special_member_entry_token(
    const std::string & object_symbol,
    const std::string & from,
    const std::string & to,
    std::string & out)
{
  std::size_t pos = std::string::npos;
  if(!find_special_member_entry_token(object_symbol, from, pos)) {
    return false;
  }
  out = object_symbol;
  out.replace(pos, from.size(), to);
  return out != object_symbol;
}

inline bool special_member_entry_point_symbol_from_complete_symbol(
    const std::string & complete_object_symbol,
    bool is_constructor,
    SpecialMemberEntryKind entry_kind,
    std::string & out)
{
  if(complete_object_symbol.empty()) {
    return false;
  }
  const char * from = is_constructor ? "C1" : "D1";
  const char * to = nullptr;
  if(!special_member_entry_code(is_constructor, entry_kind, to)) {
    return false;
  }
  if(std::string(from) == std::string(to)) {
    out = complete_object_symbol;
    return true;
  }
  return replace_special_member_entry_token(complete_object_symbol,
                                            from,
                                            to,
                                            out);
}

inline bool special_member_entry_point_symbol_from_symbol(
    const std::string & object_symbol,
    bool is_constructor,
    SpecialMemberEntryKind entry_kind,
    std::string & out)
{
  if(object_symbol.empty()) {
    return false;
  }
  const char * target = nullptr;
  if(!special_member_entry_code(is_constructor, entry_kind, target)) {
    return false;
  }
  std::size_t target_pos = std::string::npos;
  if(find_special_member_entry_token(object_symbol, target, target_pos)) {
    out = object_symbol;
    return true;
  }

  const char * constructor_entries[] = {"C1", "C2"};
  const char * destructor_entries[] = {"D1", "D2", "D0"};
  const char ** entries = is_constructor ? constructor_entries : destructor_entries;
  const std::size_t entry_count =
      is_constructor ?
          sizeof(constructor_entries) / sizeof(constructor_entries[0]) :
          sizeof(destructor_entries) / sizeof(destructor_entries[0]);
  for(std::size_t i = 0; i < entry_count; ++i) {
    if(entries[i][0] == target[0] && entries[i][1] == target[1]) {
      continue;
    }
    if(replace_special_member_entry_token(object_symbol,
                                          entries[i],
                                          target,
                                          out)) {
      return true;
    }
  }
  return false;
}

inline std::vector<std::string> implicit_special_member_symbol_aliases(
    const std::string & object_symbol)
{
  std::vector<std::string> out;
  const struct {
    const char * from;
    const char * to;
  } replacements[] = {
      {"C1", "C2"},
      {"D1", "D2"}};

  for(std::size_t i = 0; i < sizeof(replacements) / sizeof(replacements[0]); ++i) {
    std::string alias;
    if(replace_special_member_entry_token(object_symbol,
                                          replacements[i].from,
                                          replacements[i].to,
                                          alias)) {
      out.push_back(alias);
    }
  }

  if(out.size() == 2 && out[1] < out[0]) {
    std::string tmp = out[0];
    out[0] = out[1];
    out[1] = tmp;
  }
  if(out.size() == 2 && out[0] == out[1]) {
    out.pop_back();
  }
  return out;
}

}  // namespace abi_mangle
