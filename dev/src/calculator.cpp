#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <deque>
#include <typeinfo>

using namespace std;

#include "encoding.h"
#include "types.h"
#include "calculator.h"

const unordered_map<string, ECalcTokenType> StringToCalcTokenTypeMap =
{
  // primary
  {"(", CT_LPAREN},
  {")", CT_RPAREN},

  // unary
  {"!", CT_LNOT},
  {"not", CT_LNOT},
  {"~", CT_COMPL},
  {"compl", CT_COMPL},
  // CT_PLUS
  // CT_MINUS

  // multiplicative
  {"*", CT_STAR},
  {"/", CT_DIV},
  {"%", CT_MOD},

  // additative
  {"+", CT_PLUS},
  {"-", CT_MINUS},

  // shift
  {"<<", CT_LSHIFT},
  {">>", CT_RSHIFT},

  // relational
  {"<", CT_LT},
  {">", CT_GT},
  {"<=", CT_LE},
  {">=", CT_GE},

  // equality
  {"==", CT_EQ},
  {"!=", CT_NE},
  {"not_eq", CT_NE},

  // and
  {"&", CT_AMP},
  {"bitand", CT_AMP},

  // xor
  {"^", CT_XOR},
  {"xor", CT_XOR},

  // or
  {"|", CT_BOR},
  {"bitor", CT_BOR},

  // land
  {"&&", CT_LAND},
  {"and", CT_LAND},

  // lor
  {"||", CT_LOR},
  {"or", CT_LOR},

  // controlling
  {"?", CT_QMARK},
  {":", CT_COLON},
};

typedef CalcToken & (CalcToken::*token_op)(const CalcToken&);

unordered_map<ECalcTokenType, token_op, hash<int> > TypeToOpMap =
{
  {CT_LOR, &CalcToken::lor},
  {CT_LAND, &CalcToken::land},
  {CT_EQ, &CalcToken::eq},
  {CT_NE, &CalcToken::ne},
  {CT_LT, &CalcToken::lt},
  {CT_GT, &CalcToken::gt},
  {CT_LE, &CalcToken::le},
  {CT_GE, &CalcToken::ge},
  {CT_BOR, &CalcToken::operator|=},
  {CT_XOR, &CalcToken::operator^=},
  {CT_AMP, &CalcToken::operator&=},
  {CT_LSHIFT, &CalcToken::operator<<=},
  {CT_RSHIFT, &CalcToken::operator>>=},
  {CT_PLUS, &CalcToken::operator+=},
  {CT_MINUS, &CalcToken::operator-=},
  {CT_STAR, &CalcToken::operator*=},
  {CT_DIV, &CalcToken::operator/=},
  {CT_MOD, &CalcToken::operator%=}
};

vector<unordered_set<ECalcTokenType, hash<int> > > BinaryOps =
{
  {CT_LOR},
  {CT_LAND},
  {CT_BOR},
  {CT_XOR},
  {CT_AMP},
  {CT_EQ, CT_NE},
  {CT_LT, CT_GT, CT_LE, CT_GE},
  {CT_LSHIFT, CT_RSHIFT},
  {CT_PLUS, CT_MINUS},
  {CT_STAR, CT_DIV, CT_MOD}
};

Calculator::Calculator()
{}

void Calculator::reset()
{
  tokens.clear();
  token = CalcToken();
  error.clear();
}

void Calculator::accumulate(const EPPTokenType type, const string & data)
{
  if(error.size())
    return;
  switch(type) {
  case PP_HEADER_NAME:
    error = "Illegal header name";
    break;
  case PP_IDENTIFIER:
    acc_identifier(data);
    break;
  case PP_INT_LITERAL:
    acc_int_literal(data);
    break;
  case PP_FLOAT_LITERAL:
    error = "Illegal floating literal";
    break;
  case PP_QUOTE_LITERAL:
    acc_quote_literal(data);
    break;
  case PP_PREPROCESSING_OP:
    acc_preprocessing_op_or_punc(data);
    break;
  case PP_NON_WHITESPACE:
    error = "Illegal non-whitespace char";
    break;
  case PP_EOF:
    cout << "eof" << endl;
    break;
  default:
    break;
  }
}

bool Calculator::calculate()
{
  string error_out;
  auto active = try_calculate(error_out);
  if(!error_out.empty()) {
    throw expr_error(error_out);
  }
  return active;
}

bool Calculator::try_calculate(string& error_out)
{
  error_out.clear();
  if(error.size()) {
    error_out = error;
    reset();
    return false;
  }
  if(not tokens.size()) {
    error_out = "No tokens in controlling expression";
    return false;
  }
  try {
    auto reduced = reduce();
    reset();
    if(!reduced.error.empty()) {
      error_out = reduced.error;
      return false;
    }
    if(reduced.type == INT_SIGNED) {
      issigned = true;
    } else {
      issigned = false;
    }
    value = reduced.value.unsigned_value;
    return value != 0;
  } catch(expr_error& e) {
    reset();
    error_out = e.what();
    return false;
  }
}

inline void Calculator::acc_identifier(const string& data)
{
  auto it = StringToCalcTokenTypeMap.find(data);
  if(it != StringToCalcTokenTypeMap.end()) {
    store_symbol(it->second);
  } else if(data == "true") {
    store_signed(1);
  } else {
    store_signed(0);
  }
}

inline void Calculator::acc_int_literal(const string& data)
{
  try {
    unsigned long long result;
    string ud_suffix;
    auto type = classify_int(data, result, ud_suffix);
    if(ud_suffix.size()) {
      error = "Illegal ud_suffix on integer literal";
    } else {
      if(type_is_signed(type))
        store_signed(result);
      else
        store_unsigned(result);
    }
  } catch (const logic_error & e) {
    error = e.what();
  }
}

inline void Calculator::acc_preprocessing_op_or_punc(const string& data)
{
  auto it = StringToCalcTokenTypeMap.find(data);
  if(it != StringToCalcTokenTypeMap.end()) {
    store_symbol(it->second);
  } else {
    error = string("Illegal symbol: ") + data;
  }
}

inline void Calculator::acc_quote_literal(const string& data)
{
  auto qdata = parse_quote_literal(data);
  if (qdata.quote == '"') {
    error = "Illegal string literal";
    return;
  }
  if (qdata.ud_suffix.size()) {
    error = "Illegal ud_suffix on character literal";
    return;
  }
  if(qdata.contents.size() != 1) {
    error = "Illegal character literal length";
    return;
  }
  auto c = qdata.contents[0];
  if(c < 0 || c >= 0x110000 || (c >= 0xD800 && c < 0xE000))
    error = "Illegal value in character literal";
  if(qdata.enc == 'u') {
    auto utf16res = encode_utf16(qdata.contents);
    if(utf16res.size() != 1)
      error = "Illegal character literal length";
  }
  if(qdata.enc == 'u' || qdata.enc == 'U') {
    store_unsigned(c);
  } else {
    store_signed(c);
  }
}

inline CalcToken Calculator::ternary(
    vector<unordered_set<ECalcTokenType, hash<int> > >::iterator it)
{
  CalcToken result, lhs, rhs;
  result = binary(it);
  if(token.type == CT_QMARK) {
    next();
    lhs = ternary(it);
    if(token.type != CT_COLON)
        throw(expr_error("Missing colon in ternary operation"));
    next();
    rhs = ternary(it);
    if(result.value.signed_value)
      result = lhs;
    else
      result = rhs;
    if(lhs.type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      result.type = INT_UNSIGNED;
  }
  return result;
}

inline CalcToken Calculator::get_result(
    vector<unordered_set<ECalcTokenType, hash<int> > >::iterator it)
{
  if(it + 1 != BinaryOps.end())
    return binary(it + 1);
  else
    return unary();
}

inline CalcToken Calculator::binary(
    vector<unordered_set<ECalcTokenType, hash<int> > >::iterator it)
{
  CalcToken result, rhs;
  result = get_result(it);
  for(auto op = token.type; it->count(op); op = token.type) {
    next();
    rhs = get_result(it);
    auto method = TypeToOpMap.at(op);
    (result.*method)(rhs);
  }
  return result;
}

inline CalcToken Calculator::unary()
{
  CalcToken result;
  auto op = token.type;
  if(op == CT_LNOT || op == CT_COMPL || op == CT_PLUS || op == CT_MINUS) {
    next();
    result = unary();
    if(op == CT_LNOT) {
      result = !result;
    } else if(op == CT_COMPL) {
      result = ~result;
    } else if(op == CT_MINUS) {
      result = -result;
    }
  } else if(op == CT_LPAREN) {
    next();
    result = ternary(BinaryOps.begin());
    if(token.type !=  CT_RPAREN)
      throw(expr_error("Unmatched left paren"));
    next();
  } else if(op == CT_RPAREN) {
    throw(expr_error("Unmatched right paren"));
  } else if(op == INT_SIGNED || op == INT_UNSIGNED) {
    result = tokens.front();
    next();
  } else {
    throw(expr_error("Expected primary expression"));
  }
  return result;
}

inline void Calculator::next()
{
  if(tokens.empty())
    throw expr_error("Not enough arguments for operator");
  tokens.pop_front();
  if(tokens.empty())
    token = CalcToken();
  else
    token = tokens.front();
}

inline CalcToken Calculator::reduce()
{
  token = tokens.front();
  CalcToken result = ternary(BinaryOps.begin());
  if(token.type != CT_END) {
    throw expr_error("Unexpected trailing tokens");
  }
  return result;
}

inline void Calculator::store_unsigned(uintmax_t value)
{
  CalcToken token;
  token.type = INT_UNSIGNED;
  token.value.unsigned_value = value;
  tokens.push_back(token);
}

inline void Calculator::store_signed(intmax_t value)
{
  CalcToken token;
  token.type = INT_SIGNED;
  token.value.signed_value = value;
  tokens.push_back(token);
}

inline void Calculator::store_symbol(ECalcTokenType symbol)
{
  CalcToken token;
  token.type = symbol;
  tokens.push_back(token);
}
