#include <iostream>
#include <cstdint>
#include <stdexcept>

using namespace std;

#include "encoding.h"
#include "types.h"
#include "calculator.h"

namespace {

bool calc_token_type(const string & data, ECalcTokenType & type)
{
  if(data.size() == 1) {
    switch(data[0]) {
    case '(': type = CT_LPAREN; return true;
    case ')': type = CT_RPAREN; return true;
    case '!': type = CT_LNOT; return true;
    case '~': type = CT_COMPL; return true;
    case '*': type = CT_STAR; return true;
    case '/': type = CT_DIV; return true;
    case '%': type = CT_MOD; return true;
    case '+': type = CT_PLUS; return true;
    case '-': type = CT_MINUS; return true;
    case '<': type = CT_LT; return true;
    case '>': type = CT_GT; return true;
    case '&': type = CT_AMP; return true;
    case '^': type = CT_XOR; return true;
    case '|': type = CT_BOR; return true;
    case '?': type = CT_QMARK; return true;
    case ':': type = CT_COLON; return true;
    default: return false;
    }
  }
  if(data.size() == 2) {
    if(data == "<<") type = CT_LSHIFT;
    else if(data == ">>") type = CT_RSHIFT;
    else if(data == "<=") type = CT_LE;
    else if(data == ">=") type = CT_GE;
    else if(data == "==") type = CT_EQ;
    else if(data == "!=") type = CT_NE;
    else if(data == "&&") type = CT_LAND;
    else if(data == "||") type = CT_LOR;
    else if(data == "or") type = CT_LOR;
    else return false;
    return true;
  }
  if(data == "not") type = CT_LNOT;
  else if(data == "compl") type = CT_COMPL;
  else if(data == "not_eq") type = CT_NE;
  else if(data == "bitand") type = CT_AMP;
  else if(data == "xor") type = CT_XOR;
  else if(data == "bitor") type = CT_BOR;
  else if(data == "and") type = CT_LAND;
  else return false;
  return true;
}

int binary_precedence(ECalcTokenType type)
{
  switch(type) {
  case CT_LOR: return 1;
  case CT_LAND: return 2;
  case CT_BOR: return 3;
  case CT_XOR: return 4;
  case CT_AMP: return 5;
  case CT_EQ:
  case CT_NE: return 6;
  case CT_LT:
  case CT_GT:
  case CT_LE:
  case CT_GE: return 7;
  case CT_LSHIFT:
  case CT_RSHIFT: return 8;
  case CT_PLUS:
  case CT_MINUS: return 9;
  case CT_STAR:
  case CT_DIV:
  case CT_MOD: return 10;
  default: return 0;
  }
}

void apply_binary(CalcToken & lhs, ECalcTokenType type, const CalcToken & rhs)
{
  switch(type) {
  case CT_LOR: lhs.lor(rhs); break;
  case CT_LAND: lhs.land(rhs); break;
  case CT_EQ: lhs.eq(rhs); break;
  case CT_NE: lhs.ne(rhs); break;
  case CT_LT: lhs.lt(rhs); break;
  case CT_GT: lhs.gt(rhs); break;
  case CT_LE: lhs.le(rhs); break;
  case CT_GE: lhs.ge(rhs); break;
  case CT_BOR: lhs |= rhs; break;
  case CT_XOR: lhs ^= rhs; break;
  case CT_AMP: lhs &= rhs; break;
  case CT_LSHIFT: lhs <<= rhs; break;
  case CT_RSHIFT: lhs >>= rhs; break;
  case CT_PLUS: lhs += rhs; break;
  case CT_MINUS: lhs -= rhs; break;
  case CT_STAR: lhs *= rhs; break;
  case CT_DIV: lhs /= rhs; break;
  case CT_MOD: lhs %= rhs; break;
  default: throw logic_error("invalid calculator binary operator");
  }
}

}  // namespace

Calculator::Calculator()
{}

void Calculator::reset()
{
  tokens.clear();
  token_index = 0;
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
  ECalcTokenType type;
  if(calc_token_type(data, type)) {
    store_symbol(type);
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
  ECalcTokenType type;
  if(calc_token_type(data, type)) {
    store_symbol(type);
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

inline CalcToken Calculator::ternary()
{
  CalcToken result, lhs, rhs;
  result = binary(1);
  if(current_type() == CT_QMARK) {
    next();
    lhs = ternary();
    if(current_type() != CT_COLON)
        throw(expr_error("Missing colon in ternary operation"));
    next();
    rhs = ternary();
    if(result.value.signed_value)
      result = lhs;
    else
      result = rhs;
    if(lhs.type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      result.type = INT_UNSIGNED;
  }
  return result;
}

inline CalcToken Calculator::binary(int minimum_precedence)
{
  CalcToken result = unary();
  for(;;) {
    const ECalcTokenType op = current_type();
    const int precedence = binary_precedence(op);
    if(precedence < minimum_precedence)
      break;
    next();
    const CalcToken rhs = binary(precedence + 1);
    apply_binary(result, op, rhs);
  }
  return result;
}

inline CalcToken Calculator::unary()
{
  CalcToken result;
  const ECalcTokenType op = current_type();
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
    result = ternary();
    if(current_type() != CT_RPAREN)
      throw(expr_error("Unmatched left paren"));
    next();
  } else if(op == CT_RPAREN) {
    throw(expr_error("Unmatched right paren"));
  } else if(op == INT_SIGNED || op == INT_UNSIGNED) {
    result = tokens[token_index];
    next();
  } else {
    throw(expr_error("Expected primary expression"));
  }
  return result;
}

inline void Calculator::next()
{
  if(token_index >= tokens.size())
    throw expr_error("Not enough arguments for operator");
  ++token_index;
}

inline ECalcTokenType Calculator::current_type() const
{
  return token_index < tokens.size() ? tokens[token_index].type : CT_END;
}

inline CalcToken Calculator::reduce()
{
  token_index = 0;
  CalcToken result = ternary();
  if(current_type() != CT_END) {
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
