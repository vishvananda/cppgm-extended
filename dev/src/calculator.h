#pragma once

#include <climits>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "pptokenizer.h"

enum ECalcTokenType
{
  CT_LPAREN,
  CT_RPAREN,
  CT_BOR,
  CT_XOR,
  CT_COMPL,
  CT_AMP,
  CT_LNOT,
  CT_COLON,
  CT_QMARK,
  CT_PLUS,
  CT_MINUS,
  CT_LSHIFT,
  CT_RSHIFT,
  CT_STAR,
  CT_DIV,
  CT_MOD,
  CT_LT,
  CT_GT,
  CT_EQ,
  CT_NE,
  CT_LE,
  CT_GE,
  CT_LAND,
  CT_LOR,
  INT_SIGNED,
  INT_UNSIGNED
};

struct expr_error : std::logic_error
{
  explicit expr_error (const std::string& what_arg) :
    std::logic_error(what_arg)
  {}
  explicit expr_error (const char* what_arg) :
    std::logic_error(what_arg)
  {}
};

struct CalcToken
{
  union mixed {
    std::intmax_t signed_value;
    std::uintmax_t unsigned_value;
  };

  ECalcTokenType type;
  mixed value;
  std::string error;

  inline void copy_error(const CalcToken& rhs) {
    if(error.empty() && !rhs.error.empty()) {
      error = rhs.error;
    }
  }
  inline void promote(const CalcToken & rhs) {
    if(rhs.type == INT_UNSIGNED) {
      type = INT_UNSIGNED;
    }
  }
  CalcToken & lor(const CalcToken& rhs) {
    if(!value.signed_value)
      copy_error(rhs);
    value.signed_value = (value.signed_value || rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & land(const CalcToken& rhs) {
    if(value.signed_value)
      copy_error(rhs);
    value.signed_value = (value.signed_value && rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & eq(const CalcToken& rhs) {
    copy_error(rhs);
    value.signed_value = (value.signed_value == rhs.value.signed_value);

    type = INT_SIGNED;
    return *this;
  }
  CalcToken & ne(const CalcToken& rhs) {
    copy_error(rhs);
    value.signed_value = (value.signed_value != rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & lt(const CalcToken& rhs) {
    copy_error(rhs);
    if(type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      value.signed_value = (value.unsigned_value < rhs.value.unsigned_value);
    else
      value.signed_value = (value.signed_value < rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & gt(const CalcToken& rhs) {
    copy_error(rhs);
    if(type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      value.signed_value = (value.unsigned_value > rhs.value.unsigned_value);
    else
      value.signed_value = (value.signed_value > rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & le(const CalcToken& rhs) {
    copy_error(rhs);
    if(type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      value.signed_value = (value.unsigned_value <= rhs.value.unsigned_value);
    else
      value.signed_value = (value.signed_value <= rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & ge(const CalcToken& rhs) {
    copy_error(rhs);
    if(type == INT_UNSIGNED || rhs.type == INT_UNSIGNED)
      value.signed_value = (value.unsigned_value >= rhs.value.unsigned_value);
    else
      value.signed_value = (value.signed_value >= rhs.value.signed_value);
    type = INT_SIGNED;
    return *this;
  }
  CalcToken & operator|=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value |= rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator^=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value ^= rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator&=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value &= rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator<<=(const CalcToken& rhs) {
    if(rhs.value.signed_value < 0 || rhs.value.signed_value >= 64) {
      error = "Illegal shift";
    } else {
      copy_error(rhs);
      value.signed_value <<= rhs.value.signed_value;
    }
    return *this;
  }
  CalcToken & operator>>=(const CalcToken& rhs) {
    if(rhs.value.signed_value < 0 || rhs.value.signed_value >= 64) {
      error = "Illegal shift";
    } else {
      copy_error(rhs);
      if(type == INT_SIGNED)
        value.signed_value >>= rhs.value.signed_value;
      else
        value.unsigned_value >>= rhs.value.unsigned_value;
    }
    return *this;
  }
  CalcToken & operator+=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value += rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator-=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value -= rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator*=(const CalcToken& rhs) {
    copy_error(rhs);
    promote(rhs);
    value.signed_value *= rhs.value.signed_value;
    return *this;
  }
  CalcToken & operator/=(const CalcToken& rhs) {
    promote(rhs);
    if(rhs.value.signed_value == 0) {
      error = "Division by zero";
    } else {
      copy_error(rhs);
      if(type == INT_SIGNED) {
        if(value.signed_value == LLONG_MIN && rhs.value.signed_value == -1)
          error = "Integer overflow";
        else
          value.signed_value /= rhs.value.signed_value;
      } else {
        value.unsigned_value /= rhs.value.unsigned_value;
      }
    }
    return *this;
  }
  CalcToken & operator%=(const CalcToken& rhs) {
    promote(rhs);
    if(rhs.value.signed_value == 0) {
      error = "Division by zero";
    } else {
      copy_error(rhs);
      if(type == INT_SIGNED) {
        if(value.signed_value == LLONG_MIN && rhs.value.signed_value == -1)
          error = "Integer overflow";
        else
          value.signed_value %= rhs.value.signed_value;
      } else {
        value.unsigned_value %= rhs.value.unsigned_value;
      }
    }
    return *this;
  }
  CalcToken & operator!() {
    value.signed_value = !value.signed_value;
    return *this;
  }
  CalcToken & operator~() {
    value.signed_value = ~value.signed_value;
    return *this;
  }
  CalcToken & operator-() {
    value.signed_value = -value.signed_value;
    return *this;
  }
};

struct Calculator
{
  Calculator();
  void accumulate(const EPPTokenType type,
                  const std::string & data = std::string());
  bool calculate();
  bool try_calculate(std::string& error_out);
  unsigned long long value;
  bool issigned;
protected:

  inline void acc_identifier(const std::string& data);
  inline void acc_int_literal(const std::string& data);
  inline void acc_preprocessing_op_or_punc(const std::string& data);
  inline void acc_quote_literal(const std::string& data);
  inline CalcToken ternary(
      std::vector<std::unordered_set<ECalcTokenType,
                                     std::hash<int> > >::iterator it);
  inline CalcToken get_result(
      std::vector<std::unordered_set<ECalcTokenType,
                                     std::hash<int> > >::iterator it);
  inline CalcToken binary(std::vector<std::unordered_set<ECalcTokenType,
                          std::hash<int> > >::iterator it);
  inline CalcToken unary();
  inline void next();
  inline CalcToken reduce();
  inline void store_unsigned(std::uintmax_t value);
  inline void store_signed(std::intmax_t value);
  inline void store_symbol(ECalcTokenType symbol);

  std::deque<CalcToken> tokens;
  CalcToken token;
  std::string error;
};
