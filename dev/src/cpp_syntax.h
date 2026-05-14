#pragma once

#include <string>

#include "recog_token.h"

inline bool is_cv_qualifier(const RecogToken & token)
{
  return token.is_simple(KW_CONST) || token.is_simple(KW_VOLATILE);
}

inline bool is_decl_specifier_keyword(const RecogToken & token)
{
  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case KW_AUTO:
  case KW_BOOL:
  case KW_CHAR:
  case KW_CHAR16_T:
  case KW_CHAR32_T:
  case KW_DOUBLE:
  case KW_EXTERN:
  case KW_FLOAT:
  case KW_FRIEND:
  case KW_INLINE:
  case KW_INT:
  case KW_LONG:
  case KW_MUTABLE:
  case KW_REGISTER:
  case KW_SHORT:
  case KW_SIGNED:
  case KW_STATIC:
  case KW_THREAD_LOCAL:
  case KW_TYPEDEF:
  case KW_UNSIGNED:
  case KW_VIRTUAL:
  case KW_VOID:
  case KW_WCHAR_T:
  case KW_CONSTEXPR:
    return true;
  default:
    return false;
  }
}

inline bool is_simple_type_specifier(const RecogToken & token)
{
  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case KW_AUTO:
  case KW_BOOL:
  case KW_CHAR:
  case KW_CHAR16_T:
  case KW_CHAR32_T:
  case KW_DOUBLE:
  case KW_FLOAT:
  case KW_INT:
  case KW_LONG:
  case KW_SHORT:
  case KW_SIGNED:
  case KW_UNSIGNED:
  case KW_VOID:
  case KW_WCHAR_T:
    return true;
  default:
    return false;
  }
}

inline bool is_type_name_identifier(const RecogToken & token)
{
  return token.is_identifier() &&
      (token.is_class_name() || token.is_typedef_name() || token.is_enum_name());
}

inline bool is_template_id_follower(const RecogToken & token)
{
  if(token.is_eof()) {
    return true;
  }

  if(token.is_close_angle_bracket()) {
    return true;
  }

  if(token.is_identifier()) {
    return true;
  }

  if(is_decl_specifier_keyword(token)) {
    return true;
  }

  if(is_cv_qualifier(token)) {
    return true;
  }

  if(token.is_simple(KW_OPERATOR)) {
    return true;
  }

  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_COLON2:
  case OP_LPAREN:
  case OP_RPAREN:
  case OP_LSQUARE:
  case OP_RSQUARE:
  case OP_LBRACE:
  case OP_RBRACE:
  case OP_COMMA:
  case OP_SEMICOLON:
  case OP_COLON:
  case OP_PLUS:
  case OP_MINUS:
  case OP_STAR:
  case OP_DIV:
  case OP_MOD:
  case OP_XOR:
  case OP_AMP:
  case OP_BOR:
  case OP_LT:
  case OP_GT:
  case OP_PLUSASS:
  case OP_MINUSASS:
  case OP_STARASS:
  case OP_DIVASS:
  case OP_MODASS:
  case OP_XORASS:
  case OP_BANDASS:
  case OP_BORASS:
  case OP_LSHIFT:
  case OP_RSHIFTASS:
  case OP_LSHIFTASS:
  case OP_EQ:
  case OP_NE:
  case OP_LE:
  case OP_GE:
  case OP_LAND:
  case OP_LOR:
  case OP_INC:
  case OP_DEC:
  case OP_ASS:
  case OP_QMARK:
  case OP_DOTS:
  case OP_DOT:
  case OP_ARROW:
  case OP_DOTSTAR:
  case OP_ARROWSTAR:
    return true;
  default:
    return false;
  }
}

inline bool is_member_function_specifier(const RecogToken & token)
{
  return token.is_simple(KW_INLINE) || token.is_simple(KW_VIRTUAL) ||
      token.is_simple(KW_EXPLICIT) || token.is_simple(KW_CONSTEXPR) ||
      token.is_simple(KW_FRIEND) || token.is_simple(KW_STATIC);
}

inline bool is_assignment_operator(const RecogToken & token)
{
  if(token.kind != RT_SIMPLE) {
    return false;
  }

  switch(token.simple_type) {
  case OP_ASS:
  case OP_PLUSASS:
  case OP_MINUSASS:
  case OP_STARASS:
  case OP_DIVASS:
  case OP_MODASS:
  case OP_XORASS:
  case OP_BANDASS:
  case OP_BORASS:
  case OP_LSHIFTASS:
  case OP_RSHIFTASS:
    return true;
  default:
    return false;
  }
}

inline bool is_class_key(const RecogToken & token)
{
  return token.is_simple(KW_CLASS) || token.is_simple(KW_STRUCT) ||
      token.is_simple(KW_UNION);
}

inline bool is_access_specifier_token(const RecogToken & token)
{
  return token.is_simple(KW_PUBLIC) || token.is_simple(KW_PRIVATE) ||
      token.is_simple(KW_PROTECTED);
}

inline std::string describe_recog_token(const RecogToken & token)
{
  if(token.is_eof()) {
    return "EOF";
  }
  if(token.kind == RT_RSHIFT_1) {
    return "ST_RSHIFT_1";
  }
  if(token.kind == RT_RSHIFT_2) {
    return "ST_RSHIFT_2";
  }
  if(token.kind == RT_SIMPLE) {
    return std::string(token_type_to_string(token.simple_type)) + ":" + token.source;
  }
  if(token.kind == RT_IDENTIFIER) {
    return std::string("TT_IDENTIFIER:") + token.source;
  }
  return std::string("TT_LITERAL:") + token.source;
}
