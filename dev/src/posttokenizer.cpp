#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

using namespace std;

#include "types.h"
#include "posttokenizer.h"

inline bool valid_unicode_scalar_value(char32_t value)
{
  return value < 0x110000 && (value < 0xD800 || value >= 0xE000);
}

inline bool valid_course_character_code_points(const QuoteLiteralData & data)
{
  for(size_t i = 0; i < data.contents.size(); ++i) {
    if(!valid_unicode_scalar_value(data.contents[i])) {
      return false;
    }
  }
  for(size_t i = 0; i < data.string_units.size(); ++i) {
    if(data.string_units[i] >= 0x110000) {
      return false;
    }
  }
  return true;
}

inline bool lookup_identifier_token_type(const string & data,
                                         ETokenType & token_type)
{
  switch(data.size()) {
  case 2:
    switch(data[0]) {
    case 'd':
      if(data == "do") { token_type = KW_DO; return true; }
      return false;
    case 'i':
      if(data == "if") { token_type = KW_IF; return true; }
      return false;
    case 'o':
      if(data == "or") { token_type = OP_LOR; return true; }
      return false;
    default:
      return false;
    }
  case 3:
    switch(data[0]) {
    case 'a':
      if(data == "and") { token_type = OP_LAND; return true; }
      if(data == "asm") { token_type = KW_ASM; return true; }
      return false;
    case 'f':
      if(data == "for") { token_type = KW_FOR; return true; }
      return false;
    case 'i':
      if(data == "int") { token_type = KW_INT; return true; }
      return false;
    case 'n':
      if(data == "new") { token_type = KW_NEW; return true; }
      if(data == "not") { token_type = OP_LNOT; return true; }
      return false;
    case 't':
      if(data == "try") { token_type = KW_TRY; return true; }
      return false;
    case 'x':
      if(data == "xor") { token_type = OP_XOR; return true; }
      return false;
    default:
      return false;
    }
  case 4:
    switch(data[0]) {
    case 'a':
      if(data == "auto") { token_type = KW_AUTO; return true; }
      return false;
    case 'b':
      if(data == "bool") { token_type = KW_BOOL; return true; }
      return false;
    case 'c':
      if(data == "case") { token_type = KW_CASE; return true; }
      if(data == "char") { token_type = KW_CHAR; return true; }
      return false;
    case 'e':
      if(data == "else") { token_type = KW_ELSE; return true; }
      if(data == "enum") { token_type = KW_ENUM; return true; }
      return false;
    case 'g':
      if(data == "goto") { token_type = KW_GOTO; return true; }
      return false;
    case 'l':
      if(data == "long") { token_type = KW_LONG; return true; }
      return false;
    case 't':
      if(data == "this") { token_type = KW_THIS; return true; }
      if(data == "true") { token_type = KW_TRUE; return true; }
      return false;
    case 'v':
      if(data == "void") { token_type = KW_VOID; return true; }
      return false;
    default:
      return false;
    }
  case 5:
    switch(data[0]) {
    case 'b':
      if(data == "bitor") { token_type = OP_BOR; return true; }
      if(data == "break") { token_type = KW_BREAK; return true; }
      return false;
    case 'c':
      if(data == "catch") { token_type = KW_CATCH; return true; }
      if(data == "class") { token_type = KW_CLASS; return true; }
      if(data == "compl") { token_type = OP_COMPL; return true; }
      if(data == "const") { token_type = KW_CONST; return true; }
      return false;
    case 'f':
      if(data == "false") { token_type = KW_FALSE; return true; }
      if(data == "float") { token_type = KW_FLOAT; return true; }
      return false;
    case 'o':
      if(data == "or_eq") { token_type = OP_BORASS; return true; }
      return false;
    case 's':
      if(data == "short") { token_type = KW_SHORT; return true; }
      return false;
    case 't':
      if(data == "throw") { token_type = KW_THROW; return true; }
      return false;
    case 'u':
      if(data == "union") { token_type = KW_UNION; return true; }
      if(data == "using") { token_type = KW_USING; return true; }
      return false;
    case 'w':
      if(data == "while") { token_type = KW_WHILE; return true; }
      return false;
    default:
      return false;
    }
  case 6:
    switch(data[0]) {
    case 'a':
      if(data == "and_eq") { token_type = OP_BANDASS; return true; }
      return false;
    case 'b':
      if(data == "bitand") { token_type = OP_AMP; return true; }
      return false;
    case 'd':
      if(data == "delete") { token_type = KW_DELETE; return true; }
      if(data == "double") { token_type = KW_DOUBLE; return true; }
      return false;
    case 'e':
      if(data == "export") { token_type = KW_EXPORT; return true; }
      if(data == "extern") { token_type = KW_EXTERN; return true; }
      return false;
    case 'f':
      if(data == "friend") { token_type = KW_FRIEND; return true; }
      return false;
    case 'i':
      if(data == "inline") { token_type = KW_INLINE; return true; }
      return false;
    case 'n':
      if(data == "not_eq") { token_type = OP_NE; return true; }
      return false;
    case 'p':
      if(data == "public") { token_type = KW_PUBLIC; return true; }
      return false;
    case 'r':
      if(data == "return") { token_type = KW_RETURN; return true; }
      return false;
    case 's':
      if(data == "signed") { token_type = KW_SIGNED; return true; }
      if(data == "sizeof") { token_type = KW_SIZEOF; return true; }
      if(data == "static") { token_type = KW_STATIC; return true; }
      if(data == "struct") { token_type = KW_STRUCT; return true; }
      if(data == "switch") { token_type = KW_SWITCH; return true; }
      return false;
    case 't':
      if(data == "typeid") { token_type = KW_TYPEID; return true; }
      return false;
    case 'x':
      if(data == "xor_eq") { token_type = OP_XORASS; return true; }
      return false;
    default:
      return false;
    }
  case 7:
    switch(data[0]) {
    case 'a':
      if(data == "alignas") { token_type = KW_ALIGNAS; return true; }
      if(data == "alignof") { token_type = KW_ALIGNOF; return true; }
      return false;
    case 'd':
      if(data == "default") { token_type = KW_DEFAULT; return true; }
      return false;
    case 'm':
      if(data == "mutable") { token_type = KW_MUTABLE; return true; }
      return false;
    case 'n':
      if(data == "nullptr") { token_type = KW_NULLPTR; return true; }
      return false;
    case 'p':
      if(data == "private") { token_type = KW_PRIVATE; return true; }
      return false;
    case 't':
      if(data == "typedef") { token_type = KW_TYPEDEF; return true; }
      return false;
    case 'v':
      if(data == "virtual") { token_type = KW_VIRTUAL; return true; }
      return false;
    case 'w':
      if(data == "wchar_t") { token_type = KW_WCHAR_T; return true; }
      return false;
    default:
      return false;
    }
  case 8:
    switch(data[0]) {
    case 'c':
      if(data == "char16_t") { token_type = KW_CHAR16_T; return true; }
      if(data == "char32_t") { token_type = KW_CHAR32_T; return true; }
      if(data == "continue") { token_type = KW_CONTINUE; return true; }
      return false;
    case 'd':
      if(data == "decltype") { token_type = KW_DECLTYPE; return true; }
      return false;
    case 'e':
      if(data == "explicit") { token_type = KW_EXPLICIT; return true; }
      return false;
    case 'n':
      if(data == "noexcept") { token_type = KW_NOEXCEPT; return true; }
      return false;
    case 'o':
      if(data == "operator") { token_type = KW_OPERATOR; return true; }
      return false;
    case 'r':
      if(data == "register") { token_type = KW_REGISTER; return true; }
      return false;
    case 't':
      if(data == "template") { token_type = KW_TEMPLATE; return true; }
      if(data == "typename") { token_type = KW_TYPENAME; return true; }
      return false;
    case 'u':
      if(data == "unsigned") { token_type = KW_UNSIGNED; return true; }
      return false;
    case 'v':
      if(data == "volatile") { token_type = KW_VOLATILE; return true; }
      return false;
    default:
      return false;
    }
  case 9:
    switch(data[0]) {
    case 'c':
      if(data == "constexpr") { token_type = KW_CONSTEXPR; return true; }
      return false;
    case 'n':
      if(data == "namespace") { token_type = KW_NAMESPACE; return true; }
      return false;
    case 'p':
      if(data == "protected") { token_type = KW_PROTECTED; return true; }
      return false;
    default:
      return false;
    }
  case 10:
    switch(data[0]) {
    case 'c':
      if(data == "const_cast") { token_type = KW_CONST_CAST; return true; }
      return false;
    default:
      return false;
    }
  case 11:
    switch(data[0]) {
    case 's':
      if(data == "static_cast") { token_type = KW_STATIC_CAST; return true; }
      return false;
    default:
      return false;
    }
  case 12:
    switch(data[0]) {
    case 'd':
      if(data == "dynamic_cast") { token_type = KW_DYNAMIC_CAST; return true; }
      return false;
    case 't':
      if(data == "thread_local") { token_type = KW_THREAD_LOCAL; return true; }
      return false;
    default:
      return false;
    }
  case 13:
    switch(data[0]) {
    case 's':
      if(data == "static_assert") { token_type = KW_STATIC_ASSERT; return true; }
      return false;
    default:
      return false;
    }
  case 16:
    switch(data[0]) {
    case 'r':
      if(data == "reinterpret_cast") { token_type = KW_REINTERPET_CAST; return true; }
      return false;
    default:
      return false;
    }
  default:
    return false;
  }
}

inline bool lookup_preprocessing_op_or_punc(const string & data,
                                            ETokenType & token_type)
{
  switch(data.size()) {
  case 1:
    switch(data[0]) {
    case '{': token_type = OP_LBRACE; return true;
    case '}': token_type = OP_RBRACE; return true;
    case '[': token_type = OP_LSQUARE; return true;
    case ']': token_type = OP_RSQUARE; return true;
    case '(': token_type = OP_LPAREN; return true;
    case ')': token_type = OP_RPAREN; return true;
    case '|': token_type = OP_BOR; return true;
    case '^': token_type = OP_XOR; return true;
    case '~': token_type = OP_COMPL; return true;
    case '&': token_type = OP_AMP; return true;
    case '!': token_type = OP_LNOT; return true;
    case ';': token_type = OP_SEMICOLON; return true;
    case ':': token_type = OP_COLON; return true;
    case '?': token_type = OP_QMARK; return true;
    case '.': token_type = OP_DOT; return true;
    case '+': token_type = OP_PLUS; return true;
    case '-': token_type = OP_MINUS; return true;
    case '*': token_type = OP_STAR; return true;
    case '/': token_type = OP_DIV; return true;
    case '%': token_type = OP_MOD; return true;
    case '=': token_type = OP_ASS; return true;
    case '<': token_type = OP_LT; return true;
    case '>': token_type = OP_GT; return true;
    case ',': token_type = OP_COMMA; return true;
    default: return false;
    }
  case 2:
    switch(data[0]) {
    case ':':
      if(data[1] == ':') { token_type = OP_COLON2; return true; }
      if(data[1] == '>') { token_type = OP_RSQUARE; return true; }
      return false;
    case '.':
      if(data[1] == '*') { token_type = OP_DOTSTAR; return true; }
      return false;
    case '+':
      if(data[1] == '=') { token_type = OP_PLUSASS; return true; }
      if(data[1] == '+') { token_type = OP_INC; return true; }
      return false;
    case '-':
      if(data[1] == '=') { token_type = OP_MINUSASS; return true; }
      if(data[1] == '-') { token_type = OP_DEC; return true; }
      if(data[1] == '>') { token_type = OP_ARROW; return true; }
      return false;
    case '*':
      if(data[1] == '=') { token_type = OP_STARASS; return true; }
      return false;
    case '/':
      if(data[1] == '=') { token_type = OP_DIVASS; return true; }
      return false;
    case '%':
      if(data[1] == '=') { token_type = OP_MODASS; return true; }
      if(data[1] == '>') { token_type = OP_RBRACE; return true; }
      return false;
    case '^':
      if(data[1] == '=') { token_type = OP_XORASS; return true; }
      return false;
    case '&':
      if(data[1] == '=') { token_type = OP_BANDASS; return true; }
      if(data[1] == '&') { token_type = OP_LAND; return true; }
      return false;
    case '|':
      if(data[1] == '=') { token_type = OP_BORASS; return true; }
      if(data[1] == '|') { token_type = OP_LOR; return true; }
      return false;
    case '<':
      if(data[1] == '<') { token_type = OP_LSHIFT; return true; }
      if(data[1] == '=') { token_type = OP_LE; return true; }
      if(data[1] == '%') { token_type = OP_LBRACE; return true; }
      if(data[1] == ':') { token_type = OP_LSQUARE; return true; }
      return false;
    case '>':
      if(data[1] == '>') { token_type = OP_RSHIFT; return true; }
      if(data[1] == '=') { token_type = OP_GE; return true; }
      return false;
    case '=':
      if(data[1] == '=') { token_type = OP_EQ; return true; }
      return false;
    case '!':
      if(data[1] == '=') { token_type = OP_NE; return true; }
      return false;
    case 'o':
      if(data == "or") { token_type = OP_LOR; return true; }
      return false;
    default:
      return false;
    }
  case 3:
    switch(data[0]) {
    case '.':
      if(data == "...") { token_type = OP_DOTS; return true; }
      return false;
    case '>':
      if(data == ">>=") { token_type = OP_RSHIFTASS; return true; }
      return false;
    case '<':
      if(data == "<<=") { token_type = OP_LSHIFTASS; return true; }
      return false;
    case '-':
      if(data == "->*") { token_type = OP_ARROWSTAR; return true; }
      return false;
    case 'a':
      if(data == "and") { token_type = OP_LAND; return true; }
      return false;
    case 'x':
      if(data == "xor") { token_type = OP_XOR; return true; }
      return false;
    case 'n':
      if(data == "not") { token_type = OP_LNOT; return true; }
      if(data == "new") { token_type = KW_NEW; return true; }
      return false;
    default:
      return false;
    }
  case 5:
    switch(data[0]) {
    case 'b':
      if(data == "bitor") { token_type = OP_BOR; return true; }
      return false;
    case 'c':
      if(data == "compl") { token_type = OP_COMPL; return true; }
      return false;
    case 'o':
      if(data == "or_eq") { token_type = OP_BORASS; return true; }
      return false;
    default:
      return false;
    }
  case 6:
    switch(data[0]) {
    case 'a':
      if(data == "and_eq") { token_type = OP_BANDASS; return true; }
      return false;
    case 'b':
      if(data == "bitand") { token_type = OP_AMP; return true; }
      return false;
    case 'd':
      if(data == "delete") { token_type = KW_DELETE; return true; }
      return false;
    case 'n':
      if(data == "not_eq") { token_type = OP_NE; return true; }
      return false;
    case 'x':
      if(data == "xor_eq") { token_type = OP_XORASS; return true; }
      return false;
    default:
      return false;
    }
  default:
    return false;
  }
}


template<typename T>
T decode_float(const string& s);

template<>
float decode_float<float>(const string& s)
{
  errno = 0;
  char * end = nullptr;
  const float result = strtof(s.c_str(), &end);
  if(end == s.c_str() || *end != '\0') {
    throw logic_error("Invalid floating literal [" + s + "]");
  }
  return result;
}

template<>
double decode_float<double>(const string& s)
{
  errno = 0;
  char * end = nullptr;
  const double result = strtod(s.c_str(), &end);
  if(end == s.c_str() || *end != '\0') {
    throw logic_error("Invalid floating literal [" + s + "]");
  }
  return result;
}

template<>
long double decode_float<long double>(const string& s)
{
  errno = 0;
  char * end = nullptr;
  const long double result = strtold(s.c_str(), &end);
  if(end == s.c_str() || *end != '\0') {
    throw logic_error("Invalid floating literal [" + s + "]");
  }
  return result;
}

vector<unsigned char> canonical_long_double_bytes(long double value)
{
  vector<unsigned char> bytes(sizeof(long double), 0);
  size_t limit = sizeof(long double) < 10 ? sizeof(long double) : 10;
  memcpy(bytes.data(), &value, limit);
  return bytes;
}

inline vector<unsigned char> copy_bytes(const void * data, size_t nbytes)
{
  auto begin = reinterpret_cast<const unsigned char*>(data);
  return vector<unsigned char>(begin, begin + nbytes);
}

bool file_only_source_locations_enabled()
{
  static const bool enabled = []() -> bool
  {
    const char * value = getenv("CPPGM_FILE_ONLY_SOURCE_LOCATIONS");
    return value != nullptr && value[0] != '\0' &&
           !(value[0] == '0' && value[1] == '\0');
  }();
  return enabled;
}

PostTokenizer::PostTokenizer(IPPTokenSource & input,
                             SourceLocationTable * location_table,
                             const ISourceLocationProvider * location_provider,
                             bool file_only_source_locations,
                             bool allow_ordinary_multicharacter_literals) :
  input(input),
  n_encoding('"'),
  n_valid(true),
  location_table(location_table),
  location_provider(location_provider),
  current_location_id(0),
  cached_location_file_index(0),
  cached_location_file_valid(false),
  file_only_source_locations(file_only_source_locations),
  allow_ordinary_multicharacter_literals(
      allow_ordinary_multicharacter_literals),
  previous_token_was_operator_keyword(false)
{}

PostToken PostTokenizer::get()
{
  while(ready.empty()) {
    auto token = input.get();
    process(token.type, token.data);
  }
  auto result = std::move(ready.front());
  ready.pop_front();
  return result;
}

void PostTokenizer::get_many(vector<PostToken> & out, size_t max_tokens)
{
  while(out.size() < max_tokens) {
    while(ready.empty()) {
      auto token = input.get();
      process(token.type, token.data);
    }

    while(!ready.empty() && out.size() < max_tokens) {
      PostToken token = std::move(ready.front());
      const EPostTokenKind kind = token.kind;
      ready.pop_front();
      out.push_back(std::move(token));
      if(kind == PT_EOF || kind == PT_INVALID) {
        return;
      }
    }
  }
}

inline void PostTokenizer::capture_current_location()
{
  current_location_id = 0;
  if(location_table == nullptr || location_provider == nullptr) {
    return;
  }

  const string & file = location_provider->current_source_file();
  if(file.empty()) {
    return;
  }

  uint16_t file_index = 0;
  if(cached_location_file_valid && cached_location_file == file) {
    file_index = cached_location_file_index;
  } else {
    file_index = location_table->add_file(file);
    cached_location_file = file;
    cached_location_file_index = file_index;
    cached_location_file_valid = true;
  }
  if(file_only_source_locations || file_only_source_locations_enabled()) {
    current_location_id = location_table->add(
        file_index, 1, 1,
        location_provider->current_source_is_macro_expansion());
    return;
  }

  const uint32_t line = location_provider->current_source_line();
  const uint32_t column = location_provider->current_source_column();
  if(line == 0 || column == 0) {
    return;
  }
  current_location_id = location_table->add(
      file_index, line, column,
      location_provider->current_source_is_macro_expansion());
}

inline void PostTokenizer::process(const EPPTokenType type,
                                   const std::string & data)
{
  switch(type) {
  case PP_WHITESPACE:
    current_location_id = 0;
    break;
  case PP_NEW_LINE:
    current_location_id = 0;
    break;
  case PP_HEADER_NAME:
    capture_current_location();
    emit_header_name(data);
    break;
  case PP_IDENTIFIER:
    capture_current_location();
    emit_identifier(data);
    break;
  case PP_INT_LITERAL:
    capture_current_location();
    emit_int_literal(data);
    break;
  case PP_FLOAT_LITERAL:
    capture_current_location();
    emit_float_literal(data);
    break;
  case PP_QUOTE_LITERAL:
    capture_current_location();
    emit_quote_literal(data);
    break;
  case PP_PREPROCESSING_OP:
    capture_current_location();
    emit_preprocessing_op_or_punc(data);
    break;
  case PP_NON_WHITESPACE:
    capture_current_location();
    emit_non_whitespace_char(data);
    break;
  case PP_EOF:
    current_location_id = 0;
    emit_eof();
    break;
  }
}

inline void PostTokenizer::push_invalid(const std::string& data)
{
  PostToken token{PT_INVALID, data};
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_simple(const std::string& data,
                                       ETokenType token_type)
{
  PostToken token{PT_SIMPLE, data};
  token.token_type = token_type;
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = token_type == KW_OPERATOR;
}

inline void PostTokenizer::push_identifier(const std::string& data)
{
  PostToken token{PT_IDENTIFIER, data};
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_literal(const std::string& data,
                                        EFundamentalType type,
                                        const void * result,
                                        size_t size)
{
  PostToken token{PT_LITERAL, data};
  token.type = type;
  token.data = copy_bytes(result, size);
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_literal_array(const std::string& data,
                                              size_t num_elements,
                                              EFundamentalType type,
                                              const void * result,
                                              size_t size)
{
  PostToken token{PT_LITERAL_ARRAY, data};
  token.num_elements = num_elements;
  token.type = type;
  token.data = copy_bytes(result, size);
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_ud_character(const std::string& data,
                                             const std::string& ud_suffix,
                                             EFundamentalType type,
                                             const void * result,
                                             size_t size)
{
  PostToken token{PT_USER_DEFINED_LITERAL_CHARACTER, data};
  token.ud_suffix = ud_suffix;
  token.type = type;
  token.data = copy_bytes(result, size);
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_ud_string_array(const std::string& data,
                                                const std::string& ud_suffix,
                                                size_t num_elements,
                                                EFundamentalType type,
                                                const void * result,
                                                size_t size)
{
  PostToken token{PT_USER_DEFINED_LITERAL_STRING_ARRAY, data};
  token.ud_suffix = ud_suffix;
  token.num_elements = num_elements;
  token.type = type;
  token.data = copy_bytes(result, size);
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_ud_integer(const std::string& data,
                                           const std::string& ud_suffix,
                                           const std::string& prefix)
{
  PostToken token{PT_USER_DEFINED_LITERAL_INTEGER, data};
  token.ud_suffix = ud_suffix;
  token.prefix = prefix;
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::push_ud_floating(const std::string& data,
                                            const std::string& ud_suffix,
                                            const std::string& prefix)
{
  PostToken token{PT_USER_DEFINED_LITERAL_FLOATING, data};
  token.ud_suffix = ud_suffix;
  token.prefix = prefix;
  token.location_id = current_location_id;
  ready.push_back(std::move(token));
  previous_token_was_operator_keyword = false;
}

//protected:

inline void PostTokenizer::emit_header_name(const string& data)
{
  emit_next_string();
  push_invalid(data);
}

inline void PostTokenizer::emit_identifier(const string& data)
{
  emit_next_string();
  ETokenType token_type;
  if(!lookup_identifier_token_type(data, token_type)) {
    push_identifier(data);
  } else {
    push_simple(data, token_type);
  }
}

inline void PostTokenizer::emit_float_literal(const string& data)
{
  emit_next_string();
  string value;
  string ud_suffix;
  EFundamentalType literal_type = FT_DOUBLE;
  if(!split_floating_literal(data, value, literal_type, ud_suffix)) {
    // PP-number scanning only has a lexical float/integer hint.  An
    // exponent-looking identifier in an integer UDL suffix (for example
    // `123_e3`) can set that hint even though phase 7 classifies the token as
    // a user-defined integer literal.
    return emit_int_literal(data);
  }

  if(ud_suffix.size()) {
    return push_ud_floating(data, ud_suffix,
                            data.substr(0, data.size() - ud_suffix.size()));
  }

  try
  {
    if(literal_type == FT_FLOAT) {
      auto result = decode_float<float>(value);
      push_literal(data, FundamentalTypeOf<float>(), &result, sizeof(result));
    } else if(literal_type == FT_LONG_DOUBLE) {
      auto result = decode_float<long double>(value);
      auto bytes = canonical_long_double_bytes(result);
      push_literal(data, FundamentalTypeOf<long double>(), bytes.data(),
                   bytes.size());
    } else {
      auto result = decode_float<double>(value);
      push_literal(data, FundamentalTypeOf<double>(), &result, sizeof(result));
    }
  }
  catch(const logic_error &)
  {
    push_invalid(data);
  }
}

inline void PostTokenizer::emit_int_literal(const string& data)
{
  emit_next_string();
  try {
    unsigned long long result;
    string ud_suffix;
    auto type = classify_int(data, result, ud_suffix);
    if(ud_suffix.size()) {
      push_ud_integer(data, ud_suffix,
                      data.substr(0, data.size() - ud_suffix.size()));
    } else {
      push_literal(data, type, &result, type_to_size(type));
    }
  } catch (const logic_error &) {
    push_invalid(data);
  }
}

inline void PostTokenizer::emit_quote_literal(const string& data)
{
  QuoteLiteralData qdata;
  try {
    parse_quote_literal(data, qdata);
  } catch(const exception &) {
    const size_t single_quote = data.find('\'');
    const size_t double_quote = data.find('"');
    if(single_quote != string::npos &&
       (double_quote == string::npos || single_quote < double_quote)) {
      emit_next_string();
      return push_invalid(data);
    }
    if(n_data.size()) {
      n_data += ' ';
    }
    n_data.append(data);
    n_valid = false;
    return;
  }
  if(qdata.quote == '\'') {
    emit_next_string();
    if(!valid_course_character_code_points(qdata)) {
      return push_invalid(data);
    }
    if(qdata.contents.size() != 1) {
      unsigned int value = 0;
      if(allow_ordinary_multicharacter_literals &&
         ordinary_multicharacter_literal_value(qdata, value)) {
        return push_literal(data, FT_INT, &value, sizeof(value));
      }
      return push_invalid(data);
    }
    auto c = qdata.contents[0];
    if(c < 0 || c >= 0x110000 || (c >= 0xD800 && c < 0xE000))
      return push_invalid(data);
    emit_encoded(data, qdata.quote,
                 (qdata.enc == '\'' && c > 127) ? 'i' : qdata.enc,
                 qdata.contents, qdata.ud_suffix);
  } else {
    if(n_data.size())
      n_data += ' ';
    n_data.append(data);
    n_contents.append(qdata.contents);
    if(n_valid) {
      if(qdata.ud_suffix.size()) {
        if(n_suffix.size() && n_suffix != qdata.ud_suffix)
          n_valid = false;
        else
          n_suffix = qdata.ud_suffix;
      }
      if(qdata.enc != '"') {
        if(n_encoding != '"' && n_encoding != qdata.enc)
          n_valid = false;
        else
          n_encoding = qdata.enc;
      }
    }
  }
}

inline void PostTokenizer::emit_preprocessing_op_or_punc(const string& data)
{
  emit_next_string();
  ETokenType token_type;
  if(!lookup_preprocessing_op_or_punc(data, token_type)) {
    push_invalid(data);
  } else {
    push_simple(data, token_type);
  }
}

inline void PostTokenizer::emit_non_whitespace_char(const string& data)
{
  emit_next_string();
  push_invalid(data);
}

inline void PostTokenizer::emit_eof()
{
  emit_next_string();
  ready.push_back(PostToken(PT_EOF, string()));
  previous_token_was_operator_keyword = false;
}

inline void PostTokenizer::emit_next_string()
{
  if(!n_data.size())
    return;
  const bool reserved_literal_operator_suffix =
      previous_token_was_operator_keyword &&
      n_valid &&
      n_encoding == '"' &&
      !n_suffix.empty() &&
      n_suffix[0] != '_' &&
      n_contents.empty() &&
      n_data.size() == n_suffix.size() + 2 &&
      n_data.compare(0, 2, "\"\"") == 0;
  if(reserved_literal_operator_suffix) {
    const char terminator = '\0';
    push_literal_array("\"\"",
                       1,
                       FundamentalTypeOf<char>(),
                       &terminator,
                       sizeof(terminator));
    push_identifier(n_suffix);
  } else if(!n_valid) {
    push_invalid(n_data);
  } else {
    try {
      const QuoteLiteralData literal = parse_quote_literal(n_data);
      emit_string_units(n_data,
                        n_encoding,
                        quote_literal_string_units(literal),
                        n_suffix);
    } catch(const exception &) {
      push_invalid(n_data);
    }
  }
  n_data.clear();
  n_encoding = '"';
  n_contents.clear();
  n_suffix.clear();
  n_valid = true;
}

inline void PostTokenizer::emit_stringlike(char quote, const string& data,
    const string& ud_suffix, EFundamentalType type, const void * result,
    size_t size, size_t length)
{

  if(ud_suffix.size()) {
    if(ud_suffix[0] != '_') {
      push_invalid(data);
    } else {
      if(quote == '\'') {
        push_ud_character(data, ud_suffix, type, result, size);
      } else {
        push_ud_string_array(data, ud_suffix, length + 1, type, result,
                             size * (length + 1));
      }
    }
  } else if(quote == '\'') {
    push_literal(data, type, result, size);
  } else {
    push_literal_array(data, length + 1, type, result,
                       size * (length + 1));
  }
}

inline void PostTokenizer::emit_encoded(const string& data, char quote,
    char enc, const u32string& result, const string& ud_suffix)
{
  switch(enc) {
  case 'L':
    emit_stringlike(quote, data, ud_suffix, FundamentalTypeOf<wchar_t>(),
                    result.data(), sizeof(char32_t), result.size());
    break;
  case 'U':
    emit_stringlike(quote, data, ud_suffix, FundamentalTypeOf<char32_t>(),
                    result.data(), sizeof(char32_t), result.size());
    break;
  case 'i':
    emit_stringlike(quote, data, ud_suffix, FundamentalTypeOf<int>(),
                    result.data(), sizeof(char32_t), result.size());
    break;
  case 'u':
    {
    auto utf16res = encode_utf16(result);
    if(quote == '\'' && utf16res.size() != 1)
      return push_invalid(data);
    emit_stringlike(quote, data, ud_suffix, FundamentalTypeOf<char16_t>(),
                    utf16res.data(), sizeof(char16_t), utf16res.size());
    }
    break;
  default:
    {
    auto utf8res = encode_utf8(result);
    emit_stringlike(quote, data, ud_suffix, FundamentalTypeOf<char>(),
                    utf8res.data(), sizeof(char), utf8res.size());
    }
    break;
  }
};

inline void PostTokenizer::emit_string_units(
    const string& data,
    char enc,
    const vector<unsigned long long>& units,
    const string& ud_suffix)
{
  EFundamentalType type = FT_CHAR;
  size_t element_size = 1;
  unsigned long long max_value = UCHAR_MAX;
  switch(enc) {
  case 'u':
    type = FT_CHAR16_T;
    element_size = 2;
    max_value = 0xffffU;
    break;
  case 'U':
    type = FT_CHAR32_T;
    element_size = 4;
    max_value = 0xffffffffULL;
    break;
  case 'L':
    type = FT_WCHAR_T;
    element_size = 4;
    max_value = 0xffffffffULL;
    break;
  default:
    break;
  }

  vector<unsigned char> bytes((units.size() + 1) * element_size, 0);
  for(size_t i = 0; i < units.size(); ++i) {
    if(units[i] > max_value) {
      return push_invalid(data);
    }
    for(size_t byte = 0; byte < element_size; ++byte) {
      bytes[i * element_size + byte] =
          static_cast<unsigned char>((units[i] >> (byte * CHAR_BIT)) & 0xffU);
    }
  }

  if(!ud_suffix.empty()) {
    if(ud_suffix[0] != '_') {
      return push_invalid(data);
    }
    push_ud_string_array(data,
                         ud_suffix,
                         units.size() + 1,
                         type,
                         bytes.data(),
                         bytes.size());
  } else {
    push_literal_array(data,
                       units.size() + 1,
                       type,
                       bytes.data(),
                       bytes.size());
  }
}
