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
  true,   // signed char
  true,   // short int
  true,   // int
  true,   // long int
  true,   // long long int
  true,   // __int128_t
  false,  // unsigned char
  false,  // unsigned short int
  false,  // unsigned int
  false,  // unsigned long int
  false,  // unsigned long long int
  false,  // __uint128_t
  true,   // wchar_t
  true,   // char
  false,  // char16_t
  false,  // char32_t
  false,  // bool
  false,  // float
  false,  // double
  false,  // long double
  false,  // void
  false   // nullptr_t
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

unsigned long long parse_integer_digits(const string & value,
                                        unsigned int start,
                                        unsigned int base)
{
  unsigned long long result = 0;
  for(unsigned int i = start; i < value.size(); ++i) {
    unsigned int digit = 0;
    if(value[i] >= '0' && value[i] <= '9') {
      digit = static_cast<unsigned int>(value[i] - '0');
    } else if(value[i] >= 'a' && value[i] <= 'f') {
      digit = 10u + static_cast<unsigned int>(value[i] - 'a');
    } else if(value[i] >= 'A' && value[i] <= 'F') {
      digit = 10u + static_cast<unsigned int>(value[i] - 'A');
    } else {
      throw logic_error("invalid integer digit");
    }
    if(digit >= base) {
      throw logic_error("invalid integer digit");
    }
    if(result > (ULLONG_MAX - digit) / base) {
      throw logic_error("out of range");
    }
    result = result * base + digit;
  }
  return result;
}

char quote_literal_encoding(const string & prefix, char quote)
{
  if(quote == '\'') {
    if(prefix == "u8") {
      return '8';
    }
    if(prefix == "u") {
      return 'u';
    }
    if(prefix == "U") {
      return 'U';
    }
    if(prefix == "L") {
      return 'L';
    }
    return '\'';
  }

  if(prefix == "u8" || prefix == "u8R") {
    return '8';
  }
  if(prefix == "u" || prefix == "uR") {
    return 'u';
  }
  if(prefix == "U" || prefix == "UR") {
    return 'U';
  }
  if(prefix == "L" || prefix == "LR") {
    return 'L';
  }
  return '"';
}

void append_encoded_code_point_units(char enc,
                                     char32_t value,
                                     vector<unsigned long long> & units)
{
  switch(enc) {
  case 'u':
    {
      const u16string encoded = encode_utf16(u32string(1, value));
      for(size_t i = 0; i < encoded.size(); ++i) {
        units.push_back(static_cast<unsigned long long>(encoded[i]));
      }
    }
    break;
  case 'U':
  case 'L':
    units.push_back(static_cast<unsigned long long>(value));
    break;
  default:
    {
      const string encoded = encode_utf8(u32string(1, value));
      for(size_t i = 0; i < encoded.size(); ++i) {
        units.push_back(
            static_cast<unsigned long long>(
                static_cast<unsigned char>(encoded[i])));
      }
    }
    break;
  }
}

void append_code_point(char enc,
                       char32_t value,
                       u32string & contents,
                       vector<unsigned long long> & string_units)
{
  contents.push_back(value);
  append_encoded_code_point_units(enc, value, string_units);
}

void append_numeric_escape(unsigned long long value,
                           u32string & contents,
                           vector<unsigned long long> & string_units)
{
  contents.push_back(static_cast<char32_t>(value));
  string_units.push_back(value);
}

unsigned long long parse_fixed_hex_escape(const u32string & data,
                                          size_t & pos,
                                          size_t digits)
{
  if(pos + digits >= data.size()) {
    throw logic_error("invalid universal character name");
  }

  unsigned long long value = 0;
  for(size_t i = 0; i < digits; ++i) {
    const char32_t c = data[++pos];
    try {
      value = (value << 4) +
              static_cast<unsigned long long>(hex_to_value(c));
    } catch(const logic_error &) {
      throw logic_error("invalid universal character name");
    }
  }
  return value;
}

unsigned long long parse_variable_hex_escape(const u32string & data,
                                             size_t & pos)
{
  unsigned long long value = 0;
  bool any = false;
  while(pos + 1 < data.size()) {
    const char32_t c = data[pos + 1];
    try {
      value = (value << 4) +
              static_cast<unsigned long long>(hex_to_value(c));
      any = true;
      ++pos;
    } catch(const logic_error &) {
      break;
    }
  }
  if(!any) {
    throw logic_error("invalid hex escape");
  }
  return value;
}

unsigned long long parse_octal_escape(const u32string & data,
                                      size_t & pos,
                                      char32_t first)
{
  unsigned long long value = static_cast<unsigned long long>(first - '0');
  size_t count = 1;
  while(count < 3 && pos + 1 < data.size()) {
    const char32_t c = data[pos + 1];
    if(c < '0' || c > '7') {
      break;
    }
    value = (value << 3) + static_cast<unsigned long long>(c - '0');
    ++pos;
    ++count;
  }
  return value;
}

void decode_quote_payload(const string & payload,
                          bool raw,
                          char enc,
                          u32string & contents,
                          vector<unsigned long long> & string_units)
{
  contents.clear();
  string_units.clear();

  const u32string decoded = decode_utf8(payload);
  for(size_t i = 0; i < decoded.size(); ++i) {
    const char32_t c = decoded[i];
    if(raw || c != '\\') {
      append_code_point(enc, c, contents, string_units);
      continue;
    }

    if(++i >= decoded.size()) {
      throw logic_error("unterminated escape sequence");
    }

    const char32_t escaped = decoded[i];
    switch(escaped) {
    case '\'':
    case '\"':
    case '\\':
    case '\?':
      append_code_point(enc, escaped, contents, string_units);
      break;
    case 'a':
      append_code_point(enc, '\a', contents, string_units);
      break;
    case 'b':
      append_code_point(enc, '\b', contents, string_units);
      break;
    case 'f':
      append_code_point(enc, '\f', contents, string_units);
      break;
    case 'n':
      append_code_point(enc, '\n', contents, string_units);
      break;
    case 'r':
      append_code_point(enc, '\r', contents, string_units);
      break;
    case 't':
      append_code_point(enc, '\t', contents, string_units);
      break;
    case 'v':
      append_code_point(enc, '\v', contents, string_units);
      break;
    case 'x':
      append_numeric_escape(parse_variable_hex_escape(decoded, i),
                            contents,
                            string_units);
      break;
    case 'u':
      append_code_point(enc,
                        static_cast<char32_t>(
                            parse_fixed_hex_escape(decoded, i, 4)),
                        contents,
                        string_units);
      break;
    case 'U':
      append_code_point(enc,
                        static_cast<char32_t>(
                            parse_fixed_hex_escape(decoded, i, 8)),
                        contents,
                        string_units);
      break;
    default:
      if(escaped >= '0' && escaped <= '7') {
        append_numeric_escape(parse_octal_escape(decoded, i, escaped),
                              contents,
                              string_units);
      } else {
        append_code_point(enc, 0, contents, string_units);
      }
      break;
    }
  }
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
  bool binary = false;
  unsigned int digit_start = 0;
  unsigned int base = 10;
  if(octhex && len > 1 && (data[1] == 'x' || data[1] == 'X')) {
    digit_start = 2;
    base = 16;
    for(pos = 2; pos < len; ++pos) {
      if(!is_hex_digit_char(data[pos])) {
        break;
      }
    }
    if(pos == 2)
      throw logic_error("invalid hex escape");
  } else if(octhex && len > 1 && (data[1] == 'b' || data[1] == 'B')) {
    binary = true;
    digit_start = 2;
    base = 2;
    for(pos = 2; pos < len; ++pos) {
      if(data[pos] != '0' && data[pos] != '1') {
        break;
      }
    }
    if(pos == 2)
      throw logic_error("invalid binary literal");
  } else {
    char top = octhex ? '7' : '9';
    base = octhex ? 8 : 10;
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

  if(binary) {
    result = parse_integer_digits(value, digit_start, base);
  } else {
    errno = 0;
    result = strtoull(value.data(), NULL, 0);
    if(result == ULLONG_MAX && errno == ERANGE)
        throw logic_error("out of range");
  }

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
     suffix == "f128" || suffix == "F128" ||
     suffix == "f64x" || suffix == "F64x" ||
     suffix == "F64X") {
    type = FT_LONG_DOUBLE;
    return true;
  }
  if(suffix == "f64" || suffix == "F64" ||
     suffix == "f32x" || suffix == "F32x" ||
     suffix == "F32X") {
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
  out.string_units.clear();
  out.ud_suffix.clear();
  const auto parse_single_quote_literal =
      [&](size_t start_pos, QuoteLiteralData & piece, size_t & next_pos) -> bool
  {
    piece.quote = '\0';
    piece.enc = '\0';
    piece.contents.clear();
    piece.string_units.clear();
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
    piece.enc = quote_literal_encoding(prefix, piece.quote);

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
    }
    decode_quote_payload(payload, raw, piece.enc, piece.contents, piece.string_units);

    size_t suffix_pos = end_quote + 1;
    while(suffix_pos < data.size() &&
          (isalnum(static_cast<unsigned char>(data[suffix_pos])) ||
           data[suffix_pos] == '_')) {
      piece.ud_suffix.push_back(data[suffix_pos]);
      ++suffix_pos;
    }
    next_pos = suffix_pos;

    if(piece.quote == '\'') {
      if(piece.enc == '\'' && !piece.contents.empty() && piece.contents[0] > 127) {
        piece.enc = 'i';
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
    out.string_units.insert(out.string_units.end(),
                            piece.string_units.begin(),
                            piece.string_units.end());
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

bool ordinary_multicharacter_literal_value(const QuoteLiteralData & literal,
                                           unsigned int & value)
{
  if(literal.quote != '\'' ||
     literal.enc != '\'' ||
     !literal.ud_suffix.empty() ||
     literal.contents.size() <= 1 ||
     literal.string_units.size() <= 1) {
    return false;
  }

  const size_t first =
      literal.string_units.size() > sizeof(int)
          ? literal.string_units.size() - sizeof(int)
          : 0;
  unsigned int packed = 0;
  for(size_t i = first; i < literal.string_units.size(); ++i) {
    if(literal.string_units[i] > UCHAR_MAX) {
      return false;
    }
    packed = (packed << CHAR_BIT) |
             static_cast<unsigned int>(literal.string_units[i]);
  }
  value = packed;
  return true;
}

const vector<unsigned long long> &
quote_literal_string_units(const QuoteLiteralData & literal)
{
  return literal.string_units;
}

size_t quote_literal_string_unit_count(const QuoteLiteralData & literal)
{
  return literal.string_units.size();
}
