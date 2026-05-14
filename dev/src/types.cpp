#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <stdexcept>

using namespace std;

#include "encoding.h"
#include "types.h"

namespace {

const char * const FundamentalTypeNames[] =
{
  "signed char",
  "short int",
  "int",
  "long int",
  "long long int",
  "__int128_t",
  "unsigned char",
  "unsigned short int",
  "unsigned int",
  "unsigned long int",
  "unsigned long long int",
  "__uint128_t",
  "wchar_t",
  "char",
  "char16_t",
  "char32_t",
  "bool",
  "float",
  "double",
  "long double",
  "void",
  "nullptr_t"
};

const size_t FundamentalTypeSizes[] =
{
  1,
  2,
  4,
  8,
  8,
  16,
  1,
  2,
  4,
  8,
  8,
  16,
  4,
  1,
  2,
  4,
  1,
  4,
  8,
  16,
  0,
  8
};

const bool FundamentalTypeSignedness[] =
{
  true,
  true,
  true,
  true,
  true,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false
};

inline size_t fundamental_type_index(EFundamentalType type)
{
  return static_cast<size_t>(type);
}

bool is_hex_digit_char(char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

}  // namespace

std::string type_to_string(EFundamentalType type)
{
  return FundamentalTypeNames[fundamental_type_index(type)];
}

size_t type_to_size(EFundamentalType type)
{
  return FundamentalTypeSizes[fundamental_type_index(type)];
}

bool type_is_signed(EFundamentalType type)
{
  return FundamentalTypeSignedness[fundamental_type_index(type)];
}

EFundamentalType classify_int(const string& data,
                              unsigned long long& result,
                              string& ud_suffix)
{
  if(data.empty()) {
    throw logic_error("empty integer literal");
  }
  unsigned int pos;
  unsigned int len = data.size();
  bool octhex = data[0] == '0';
  if(octhex && len > 1 && (data[1] == 'x' || data[1] == 'X')) {
    for(pos = 2; pos < len; ++pos) {
      if(!is_hex_digit_char(data[pos])) {
        break;
      }
    }
    if(pos == 2)
      throw logic_error("invalid hex escape");
  } else {
    char top = octhex ? '7' : '9';
    for(pos = 1; pos < len; ++pos)
      if(data[pos] < '0' || data[pos] > top)
        break;
  }
  string value = data.substr(0, pos);
  bool u = false;
  bool l = false;
  bool ll = false;
  for(; pos < len; ++pos) {
    if(!u && (data[pos] == 'u' || data[pos] == 'U')) {
      u = true;
    } else if(!l && !ll && (data[pos] == 'l' || data[pos] == 'L')) {
      if(pos + 1 < len && data[pos + 1] == data[pos]) {
        ++pos;
        ll = true;
      } else {
        l = true;
      }
    } else {
      break;
    }
  }
  ud_suffix = data.substr(pos, data.size() - pos);
  if(ud_suffix.size()) {
    if (u || l || ll || ud_suffix[0] != '_' ||
        ud_suffix.find_first_of("+-") != string::npos)
      throw logic_error("invalid integer suffix");
    else
      return FundamentalTypeOf<void>();
  }

  result = strtoull(value.data(), NULL, 0);
  if(result == ULLONG_MAX && errno == ERANGE)
      throw logic_error("out of range");

  if(u) {
    if(!ll && !l && result <= UINT_MAX) {
      return FundamentalTypeOf<unsigned int>();
    } else if(!ll && result <= ULONG_MAX) {
      return FundamentalTypeOf<unsigned long>();
    } else if (l) {
      throw logic_error("out of range");
    } else {
      return FundamentalTypeOf<unsigned long long>();
    }
  } else if(!octhex) {
    if(!ll && !l && result <= INT_MAX) {
      return FundamentalTypeOf<int>();
    } else if(!ll && result <= LONG_MAX) {
      return FundamentalTypeOf<long>();
    } else if (l || result > LLONG_MAX) {
      throw logic_error("out of range");
    } else {
      return FundamentalTypeOf<long long>();
    }
  } else {
    if(!ll && !l && result <= INT_MAX) {
      return FundamentalTypeOf<int>();
    } else if(!ll && !l && result <= UINT_MAX) {
      return FundamentalTypeOf<unsigned int>();
    } else if(!ll && result <= LONG_MAX) {
      return FundamentalTypeOf<long>();
    } else if(!ll && result <= ULONG_MAX) {
      return FundamentalTypeOf<unsigned long>();
    } else if (l) {
      throw logic_error("out of range");
    } else if(result <= LONG_MAX) {
      return FundamentalTypeOf<long long>();
    } else {
      return FundamentalTypeOf<unsigned long long>();
    }
  }
}

bool split_floating_literal(const string& text,
                            string& value,
                            EFundamentalType& type,
                            string& ud_suffix)
{
  size_t pos = 0;
  bool is_hex = text.size() >= 2 &&
                text[0] == '0' &&
                (text[1] == 'x' || text[1] == 'X');
  bool saw_dot = false;
  bool saw_exp = false;
  bool saw_digit = false;

  if(is_hex) {
    pos = 2;
  } else if(pos < text.size() && text[pos] == '.') {
    saw_dot = true;
    ++pos;
  } else {
    while(pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
      saw_digit = true;
      ++pos;
    }
  }

  for(; pos < text.size(); ++pos) {
    const char c = text[pos];
    if(c == '.' && !saw_dot) {
      saw_dot = true;
      continue;
    }
    if(is_hex ?
           std::isxdigit(static_cast<unsigned char>(c)) != 0 :
           std::isdigit(static_cast<unsigned char>(c)) != 0) {
      saw_digit = true;
      continue;
    }
    if(!saw_exp &&
       ((!is_hex && (c == 'e' || c == 'E')) ||
        (is_hex && (c == 'p' || c == 'P')))) {
      saw_exp = true;
      if(pos + 1 < text.size() && (text[pos + 1] == '+' || text[pos + 1] == '-')) {
        ++pos;
      }
      continue;
    }
    break;
  }

  if(!saw_digit) {
    return false;
  }

  // Integer literals like `1L` and `1LL` must not take the floating-literal
  // path just because they share suffix letters with `long double`.
  const bool has_floating_syntax = is_hex ? saw_exp : (saw_dot || saw_exp);
  if(!has_floating_syntax) {
    return false;
  }

  value = text.substr(0, pos);
  ud_suffix.clear();
  type = FT_DOUBLE;

  if(!value.empty() &&
     (value[value.size() - 1] == '+' || value[value.size() - 1] == '-')) {
    return false;
  }

  if(pos == text.size()) {
    return true;
  }

  const string suffix = text.substr(pos);
  if(suffix == "f" || suffix == "F" ||
     suffix == "f16" || suffix == "F16" ||
     suffix == "f32" || suffix == "F32" ||
     suffix == "bf16" || suffix == "BF16") {
    type = FT_FLOAT;
    return true;
  }
  if(suffix == "l" || suffix == "L" ||
     suffix == "q" || suffix == "Q" ||
     suffix == "f128" || suffix == "F128") {
    type = FT_LONG_DOUBLE;
    return true;
  }
  if(suffix == "f64" || suffix == "F64") {
    type = FT_DOUBLE;
    return true;
  }
  if(!suffix.empty() && suffix[0] == '_' &&
     suffix.find_first_of("+-.") == string::npos) {
    type = FT_VOID;
    ud_suffix = suffix;
    return true;
  }
  return false;
}

void parse_quote_literal(const string& data, QuoteLiteralData& out)
{
  out.quote = '\0';
  out.enc = '\0';
  out.contents.clear();
  out.ud_suffix.clear();
  const auto parse_single_quote_literal =
      [&](size_t start_pos, QuoteLiteralData & piece, size_t & next_pos) -> bool
  {
    piece.quote = '\0';
    piece.enc = '\0';
    piece.contents.clear();
    piece.ud_suffix.clear();

    size_t pos = start_pos;
    while(pos < data.size() &&
          isspace(static_cast<unsigned char>(data[pos]))) {
      ++pos;
    }
    if(pos >= data.size()) {
      next_pos = pos;
      return false;
    }

    size_t quote_pos = string::npos;
    for(size_t i = pos; i < data.size(); ++i) {
      if(data[i] == '\'' || data[i] == '"') {
        quote_pos = i;
        piece.quote = data[i];
        break;
      }
      if(isspace(static_cast<unsigned char>(data[i]))) {
        return false;
      }
    }
    if(quote_pos == string::npos) {
      return false;
    }

    const string prefix = data.substr(pos, quote_pos - pos);
    bool raw = false;
    if(!prefix.empty() && prefix[prefix.size() - 1] == 'R') {
      raw = true;
    }

    size_t end_quote = string::npos;
    string payload;
    if(raw) {
      const size_t payload_start = data.find('(', quote_pos + 1);
      if(payload_start == string::npos) {
        return false;
      }
      const string delimiter = data.substr(quote_pos + 1,
                                           payload_start - (quote_pos + 1));
      const string terminator = string(")") + delimiter + piece.quote;
      const size_t payload_end = data.find(terminator, payload_start + 1);
      if(payload_end == string::npos) {
        return false;
      }
      payload = data.substr(payload_start + 1, payload_end - (payload_start + 1));
      end_quote = payload_end + terminator.size() - 1;
      piece.contents = decode_utf8(payload);
    } else {
      payload.reserve(data.size() - (quote_pos + 1));
      bool escaped = false;
      for(size_t i = quote_pos + 1; i < data.size(); ++i) {
        const char c = data[i];
        if(!escaped && c == piece.quote) {
          end_quote = i;
          break;
        }
        payload.push_back(c);
        if(!escaped && c == '\\') {
          escaped = true;
        } else {
          escaped = false;
        }
      }
      if(end_quote == string::npos) {
        return false;
      }
      piece.contents = decode_escape(decode_utf8(payload));
    }

    size_t suffix_pos = end_quote + 1;
    while(suffix_pos < data.size() &&
          (isalnum(static_cast<unsigned char>(data[suffix_pos])) ||
           data[suffix_pos] == '_')) {
      piece.ud_suffix.push_back(data[suffix_pos]);
      ++suffix_pos;
    }
    next_pos = suffix_pos;

    if(piece.quote == '\'') {
      if(prefix == "u8") {
        piece.enc = '8';
      } else if(prefix == "u") {
        piece.enc = 'u';
      } else if(prefix == "U") {
        piece.enc = 'U';
      } else if(prefix == "L") {
        piece.enc = 'L';
      } else {
        piece.enc = '\'';
      }
      if(piece.enc == '\'' && !piece.contents.empty() && piece.contents[0] > 127) {
        piece.enc = 'i';
      }
    } else {
      if(prefix == "u8" || prefix == "u8R") {
        piece.enc = '8';
      } else if(prefix == "u" || prefix == "uR") {
        piece.enc = 'u';
      } else if(prefix == "U" || prefix == "UR") {
        piece.enc = 'U';
      } else if(prefix == "L" || prefix == "LR") {
        piece.enc = 'L';
      } else {
        piece.enc = '"';
      }
    }
    return true;
  };

  size_t pos = 0;
  bool first = true;
  while(true) {
    QuoteLiteralData piece;
    size_t next_pos = pos;
    if(!parse_single_quote_literal(pos, piece, next_pos)) {
      break;
    }

    if(first) {
      out.quote = piece.quote;
      out.enc = piece.enc;
      out.ud_suffix = piece.ud_suffix;
      first = false;
    } else {
      if(piece.enc != '"' &&
         piece.enc != '\'' &&
         out.enc == out.quote) {
        out.enc = piece.enc;
      }
      if(out.ud_suffix.empty() && !piece.ud_suffix.empty()) {
        out.ud_suffix = piece.ud_suffix;
      }
    }
    out.contents.append(piece.contents);
    pos = next_pos;
  }
}

EFundamentalType string_literal_element_type(const QuoteLiteralData & literal)
{
  switch(literal.enc) {
  case 'u':
    return FT_CHAR16_T;
  case 'U':
    return FT_CHAR32_T;
  case 'L':
    return FT_WCHAR_T;
  default:
    return FT_CHAR;
  }
}

EFundamentalType character_literal_type(const QuoteLiteralData & literal)
{
  switch(literal.enc) {
  case 'u':
    return FT_CHAR16_T;
  case 'U':
    return FT_CHAR32_T;
  case 'L':
    return FT_WCHAR_T;
  default:
    return FT_CHAR;
  }
}
