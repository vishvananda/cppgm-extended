#include "abi_mangle.h"
#include "abi_model.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace abi_mangle {

namespace {

string trim(const string & text)
{
  size_t begin = 0;
  while(begin < text.size() &&
        isspace(static_cast<unsigned char>(text[begin]))) {
    ++begin;
  }
  size_t end = text.size();
  while(end > begin && isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }
  return text.substr(begin, end - begin);
}

vector<string> split_words(const string & text)
{
  vector<string> words;
  istringstream in(text);
  string word;
  while(in >> word) {
    words.push_back(word);
  }
  return words;
}

bool starts_with(const string & text, const string & prefix)
{
  return text.compare(0, prefix.size(), prefix) == 0;
}

string base36_number(size_t value)
{
  static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  string out;
  do {
    out.push_back(digits[value % 36]);
    value /= 36;
  } while(value != 0);
  reverse(out.begin(), out.end());
  return out;
}

vector<string> split_qualified_name(string name)
{
  if(starts_with(name, "::")) {
    name = name.substr(2);
  }
  vector<string> parts;
  size_t begin = 0;
  while(begin <= name.size()) {
    const size_t pos = name.find("::", begin);
    const string part = name.substr(begin,
                                    pos == string::npos ? string::npos :
                                    pos - begin);
    if(part.empty()) {
      throw logic_error("empty qualified-name component in '" + name + "'");
    }
    parts.push_back(part);
    if(pos == string::npos) {
      break;
    }
    begin = pos + 2;
  }
  if(parts.empty()) {
    throw logic_error("empty qualified name");
  }
  return parts;
}

string unqualified_name(const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  return parts.back();
}

size_t parse_index(const string & text)
{
  if(text.empty()) {
    throw logic_error("empty ABI fact index");
  }
  for(size_t i = 0; i < text.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(text[i]))) {
      throw logic_error("ABI fact index must be decimal in '" + text + "'");
    }
  }
  return static_cast<size_t>(strtoull(text.c_str(), nullptr, 10));
}

long long parse_signed_integer(const string & text)
{
  if(text.empty()) {
    throw logic_error("empty ABI fact integer");
  }
  char * end = nullptr;
  const long long value = strtoll(text.c_str(), &end, 10);
  if(!end || *end != '\0') {
    throw logic_error("ABI fact integer must be decimal in '" + text + "'");
  }
  return value;
}

bool boolean_word(const string & word)
{
  return word == "1" || word == "yes" || word == "true";
}

AbiBuiltinType builtin_type_from_name(const string & name)
{
  if(name == "void") { return ABI_BUILTIN_VOID; }
  if(name == "bool") { return ABI_BUILTIN_BOOL; }
  if(name == "char") { return ABI_BUILTIN_CHAR; }
  if(name == "schar") { return ABI_BUILTIN_SIGNED_CHAR; }
  if(name == "uchar") { return ABI_BUILTIN_UNSIGNED_CHAR; }
  if(name == "short") { return ABI_BUILTIN_SHORT; }
  if(name == "ushort") { return ABI_BUILTIN_UNSIGNED_SHORT; }
  if(name == "int") { return ABI_BUILTIN_INT; }
  if(name == "uint") { return ABI_BUILTIN_UNSIGNED_INT; }
  if(name == "long") { return ABI_BUILTIN_LONG; }
  if(name == "ulong") { return ABI_BUILTIN_UNSIGNED_LONG; }
  if(name == "longlong") { return ABI_BUILTIN_LONG_LONG; }
  if(name == "ulonglong") { return ABI_BUILTIN_UNSIGNED_LONG_LONG; }
  if(name == "int128") { return ABI_BUILTIN_INT128; }
  if(name == "uint128") { return ABI_BUILTIN_UINT128; }
  if(name == "wchar") { return ABI_BUILTIN_WCHAR; }
  if(name == "char16") { return ABI_BUILTIN_CHAR16; }
  if(name == "char32") { return ABI_BUILTIN_CHAR32; }
  if(name == "float") { return ABI_BUILTIN_FLOAT; }
  if(name == "double") { return ABI_BUILTIN_DOUBLE; }
  if(name == "longdouble") { return ABI_BUILTIN_LONG_DOUBLE; }
  if(name == "nullptr") { return ABI_BUILTIN_NULLPTR; }
  return ABI_BUILTIN_INVALID;
}

string builtin_code(AbiBuiltinType type)
{
  switch(type) {
  case ABI_BUILTIN_VOID: return "v";
  case ABI_BUILTIN_BOOL: return "b";
  case ABI_BUILTIN_CHAR: return "c";
  case ABI_BUILTIN_SIGNED_CHAR: return "a";
  case ABI_BUILTIN_UNSIGNED_CHAR: return "h";
  case ABI_BUILTIN_SHORT: return "s";
  case ABI_BUILTIN_UNSIGNED_SHORT: return "t";
  case ABI_BUILTIN_INT: return "i";
  case ABI_BUILTIN_UNSIGNED_INT: return "j";
  case ABI_BUILTIN_LONG: return "l";
  case ABI_BUILTIN_UNSIGNED_LONG: return "m";
  case ABI_BUILTIN_LONG_LONG: return "x";
  case ABI_BUILTIN_UNSIGNED_LONG_LONG: return "y";
  case ABI_BUILTIN_INT128: return "n";
  case ABI_BUILTIN_UINT128: return "o";
  case ABI_BUILTIN_WCHAR: return "w";
  case ABI_BUILTIN_CHAR16: return "Ds";
  case ABI_BUILTIN_CHAR32: return "Di";
  case ABI_BUILTIN_FLOAT: return "f";
  case ABI_BUILTIN_DOUBLE: return "d";
  case ABI_BUILTIN_LONG_DOUBLE: return "e";
  case ABI_BUILTIN_NULLPTR: return "Dn";
  case ABI_BUILTIN_INVALID: break;
  }
  return string();
}

string builtin_name(AbiBuiltinType type)
{
  switch(type) {
  case ABI_BUILTIN_VOID: return "void";
  case ABI_BUILTIN_BOOL: return "bool";
  case ABI_BUILTIN_CHAR: return "char";
  case ABI_BUILTIN_SIGNED_CHAR: return "schar";
  case ABI_BUILTIN_UNSIGNED_CHAR: return "uchar";
  case ABI_BUILTIN_SHORT: return "short";
  case ABI_BUILTIN_UNSIGNED_SHORT: return "ushort";
  case ABI_BUILTIN_INT: return "int";
  case ABI_BUILTIN_UNSIGNED_INT: return "uint";
  case ABI_BUILTIN_LONG: return "long";
  case ABI_BUILTIN_UNSIGNED_LONG: return "ulong";
  case ABI_BUILTIN_LONG_LONG: return "longlong";
  case ABI_BUILTIN_UNSIGNED_LONG_LONG: return "ulonglong";
  case ABI_BUILTIN_INT128: return "int128";
  case ABI_BUILTIN_UINT128: return "uint128";
  case ABI_BUILTIN_WCHAR: return "wchar";
  case ABI_BUILTIN_CHAR16: return "char16";
  case ABI_BUILTIN_CHAR32: return "char32";
  case ABI_BUILTIN_FLOAT: return "float";
  case ABI_BUILTIN_DOUBLE: return "double";
  case ABI_BUILTIN_LONG_DOUBLE: return "longdouble";
  case ABI_BUILTIN_NULLPTR: return "nullptr";
  case ABI_BUILTIN_INVALID: break;
  }
  return string();
}

AbiStdSubstitution std_substitution_from_word(const string & word)
{
  if(word == "St") { return ABI_STD_SUBSTITUTION_STD; }
  if(word == "Sa") { return ABI_STD_SUBSTITUTION_ALLOCATOR; }
  if(word == "Sb") { return ABI_STD_SUBSTITUTION_BASIC_STRING; }
  if(word == "Ss") { return ABI_STD_SUBSTITUTION_STRING; }
  if(word == "Si") { return ABI_STD_SUBSTITUTION_ISTREAM; }
  if(word == "So") { return ABI_STD_SUBSTITUTION_OSTREAM; }
  if(word == "Sd") { return ABI_STD_SUBSTITUTION_IOSTREAM; }
  if(word == "none" || word == "-") { return ABI_STD_SUBSTITUTION_NONE; }
  return ABI_STD_SUBSTITUTION_NONE;
}

string word_from_std_substitution(AbiStdSubstitution substitution)
{
  switch(substitution) {
  case ABI_STD_SUBSTITUTION_NONE: return "none";
  case ABI_STD_SUBSTITUTION_STD: return "St";
  case ABI_STD_SUBSTITUTION_ALLOCATOR: return "Sa";
  case ABI_STD_SUBSTITUTION_BASIC_STRING: return "Sb";
  case ABI_STD_SUBSTITUTION_STRING: return "Ss";
  case ABI_STD_SUBSTITUTION_ISTREAM: return "Si";
  case ABI_STD_SUBSTITUTION_OSTREAM: return "So";
  case ABI_STD_SUBSTITUTION_IOSTREAM: return "Sd";
  }
  throw logic_error("unknown ABI standard substitution");
}

bool builtin_code_from_name(const string & word, string & code)
{
  const AbiBuiltinType type = builtin_type_from_name(word);
  if(type == ABI_BUILTIN_INVALID) {
    return false;
  }
  code = builtin_code(type);
  return true;
}

bool is_builtin_type_name(const string & word)
{
  string code;
  return builtin_code_from_name(word, code);
}

AbiType builtin_type_spec(const string & name)
{
  AbiType out;
  out.kind = ABI_TYPE_BUILTIN;
  out.name = name;
  out.builtin_type = builtin_type_from_name(name);
  if(out.builtin_type == ABI_BUILTIN_INVALID) {
    throw logic_error("unknown builtin ABI fact type '" + name + "'");
  }
  return out;
}

AbiArrayBound integer_array_bound(unsigned long long value)
{
  AbiArrayBound out;
  out.kind = ABI_ARRAY_BOUND_INTEGER;
  out.integer_value = value;
  return out;
}

AbiArrayBound expression_array_bound(const string & expression_reference)
{
  AbiArrayBound out;
  out.kind = ABI_ARRAY_BOUND_EXPRESSION;
  out.expression_reference = expression_reference;
  return out;
}

bool parse_unsigned_integer_word(const string & text, unsigned long long & value)
{
  if(text.empty()) {
    return false;
  }
  for(size_t i = 0; i < text.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(text[i]))) {
      return false;
    }
  }
  char * end = nullptr;
  value = strtoull(text.c_str(), &end, 10);
  return end && *end == '\0';
}

AbiArrayBound parse_array_bound_word(const string & word)
{
  if(starts_with(word, "expr:")) {
    const string ref = word.substr(5);
    if(ref.empty()) {
      throw logic_error("array expression bound requires expr:<reference>");
    }
    return expression_array_bound(ref);
  }
  unsigned long long value = 0;
  if(!parse_unsigned_integer_word(word, value)) {
    throw logic_error("array bound must be an integer or expr:<reference>");
  }
  return integer_array_bound(value);
}

string word_from_array_bound(const AbiArrayBound & bound)
{
  switch(bound.kind) {
  case ABI_ARRAY_BOUND_INTEGER:
    return to_string(bound.integer_value);
  case ABI_ARRAY_BOUND_EXPRESSION:
    if(bound.expression_reference.empty()) {
      throw logic_error("array expression bound requires an expression reference");
    }
    return "expr:" + bound.expression_reference;
  case ABI_ARRAY_BOUND_NONE:
    break;
  }
  throw logic_error("array type requires a bound");
}

AbiType reference_type_spec(const string & id)
{
  AbiType out;
  out.kind = ABI_TYPE_REFERENCE;
  out.reference = id;
  return out;
}

AbiType unary_type_spec(AbiTypeKind kind, const AbiType & child)
{
  AbiType out;
  out.kind = kind;
  out.child_types.push_back(child);
  return out;
}

AbiType named_type_spec(const string & qualified_name)
{
  AbiType out;
  out.kind = ABI_TYPE_NAMED;
  out.name = qualified_name;
  return out;
}

AbiType parse_single_type_token(const string & text)
{
  if(starts_with(text, "ptr:")) {
    return unary_type_spec(ABI_TYPE_POINTER,
                           parse_single_type_token(text.substr(4)));
  }
  if(starts_with(text, "ref:")) {
    return unary_type_spec(ABI_TYPE_LVALUE_REFERENCE,
                           parse_single_type_token(text.substr(4)));
  }
  if(starts_with(text, "rref:")) {
    return unary_type_spec(ABI_TYPE_RVALUE_REFERENCE,
                           parse_single_type_token(text.substr(5)));
  }
  if(starts_with(text, "const:")) {
    return unary_type_spec(ABI_TYPE_CONST,
                           parse_single_type_token(text.substr(6)));
  }
  if(starts_with(text, "volatile:")) {
    return unary_type_spec(ABI_TYPE_VOLATILE,
                           parse_single_type_token(text.substr(9)));
  }
  if(starts_with(text, "vendor:")) {
    const string rest = text.substr(7);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("vendor type requires vendor:<qualifier>:<operand>");
    }
    AbiType out;
    out.kind = ABI_TYPE_VENDOR_QUALIFIED;
    out.name = rest.substr(0, pos);
    if(out.name == "_Atomic") {
      out.vendor_qualifier = ABI_VENDOR_QUALIFIER_ATOMIC;
    }
    out.child_types.push_back(parse_single_type_token(rest.substr(pos + 1)));
    return out;
  }
  if(starts_with(text, "transform:")) {
    const string rest = text.substr(10);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("builtin transform type requires transform:<name>:<operand>");
    }
    AbiType out;
    out.kind = ABI_TYPE_BUILTIN_TYPE_TRANSFORM;
    out.name = rest.substr(0, pos);
    out.child_types.push_back(parse_single_type_token(rest.substr(pos + 1)));
    return out;
  }
  if(starts_with(text, "array:")) {
    const string rest = text.substr(6);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0) {
      throw logic_error("array type requires array:<bound>:<element>");
    }
    AbiType out;
    out.kind = ABI_TYPE_ARRAY;
    out.array_bound = parse_array_bound_word(rest.substr(0, pos));
    out.child_types.push_back(parse_single_type_token(rest.substr(pos + 1)));
    return out;
  }
  if(starts_with(text, "memberptr:")) {
    const string rest = text.substr(10);
    const size_t pos = rest.rfind(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("member pointer type requires memberptr:<owner>:<member-type>");
    }
    AbiType out;
    out.kind = ABI_TYPE_MEMBER_POINTER;
    out.child_types.push_back(named_type_spec(rest.substr(0, pos)));
    out.child_types.push_back(parse_single_type_token(rest.substr(pos + 1)));
    return out;
  }
  if(starts_with(text, "named:")) {
    return named_type_spec(text.substr(6));
  }
  if(is_builtin_type_name(text)) {
    return builtin_type_spec(text);
  }
  return reference_type_spec(text);
}

AbiType parse_type_spec(const vector<string> & words, size_t begin)
{
  if(begin >= words.size()) {
    throw logic_error("missing ABI fact type");
  }
  if(begin + 1 == words.size()) {
    return parse_single_type_token(words[begin]);
  }

  const string & kind = words[begin];
  AbiType out;
  if(kind == "template-param" || kind == "template-param-subst") {
    if(begin + 2 != words.size()) {
      throw logic_error("template-param type requires one index");
    }
    out.kind = ABI_TYPE_TEMPLATE_PARAMETER;
    out.template_parameter_index = parse_index(words[begin + 1]);
    out.substitutable_template_parameter = kind == "template-param-subst";
    return out;
  }
  if(kind == "ptr" || kind == "ref" || kind == "rref" ||
     kind == "const" || kind == "volatile" || kind == "pack") {
    if(begin + 2 != words.size()) {
      throw logic_error(kind + " type requires one operand");
    }
    if(kind == "ptr") {
      out.kind = ABI_TYPE_POINTER;
    } else if(kind == "ref") {
      out.kind = ABI_TYPE_LVALUE_REFERENCE;
    } else if(kind == "rref") {
      out.kind = ABI_TYPE_RVALUE_REFERENCE;
    } else if(kind == "const") {
      out.kind = ABI_TYPE_CONST;
    } else if(kind == "volatile") {
      out.kind = ABI_TYPE_VOLATILE;
    } else {
      out.kind = ABI_TYPE_PACK_EXPANSION;
    }
    out.child_types.push_back(parse_single_type_token(words[begin + 1]));
    return out;
  }
  if(kind == "vendor") {
    if(begin + 3 != words.size()) {
      throw logic_error("vendor type requires qualifier and operand");
    }
    out.kind = ABI_TYPE_VENDOR_QUALIFIED;
    out.name = words[begin + 1];
    if(out.name == "_Atomic") {
      out.vendor_qualifier = ABI_VENDOR_QUALIFIER_ATOMIC;
    }
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "array") {
    if(begin + 3 != words.size()) {
      throw logic_error("array type requires bound and element type");
    }
    out.kind = ABI_TYPE_ARRAY;
    out.array_bound = parse_array_bound_word(words[begin + 1]);
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "transform" || kind == "builtin-transform") {
    if(begin + 3 != words.size()) {
      throw logic_error("builtin-transform type requires name and operand");
    }
    out.kind = ABI_TYPE_BUILTIN_TYPE_TRANSFORM;
    out.name = words[begin + 1];
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "function-type" || kind == "function-type-variadic") {
    if(begin + 2 >= words.size()) {
      throw logic_error("function-type requires a result type");
    }
    out.kind = ABI_TYPE_FUNCTION;
    out.variadic = kind == "function-type-variadic";
    for(size_t i = begin + 1; i < words.size(); ++i) {
      if(words[i] == "variadic" || words[i] == "varargs") {
        out.variadic = true;
      } else {
        out.child_types.push_back(parse_single_type_token(words[i]));
      }
    }
    return out;
  }
  if(kind == "member-pointer") {
    if(begin + 3 != words.size()) {
      throw logic_error("member-pointer type requires owner and member type");
    }
    out.kind = ABI_TYPE_MEMBER_POINTER;
    out.child_types.push_back(parse_single_type_token(words[begin + 1]));
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "name") {
    if(begin + 2 != words.size()) {
      throw logic_error("name type requires one qualified name");
    }
    return named_type_spec(words[begin + 1]);
  }
  if(kind == "template") {
    if(begin + 2 > words.size()) {
      throw logic_error("template type requires a qualified template name");
    }
    out.kind = ABI_TYPE_CLASS_TEMPLATE;
    out.name = words[begin + 1];
    out.template_argument_references.assign(words.begin() + begin + 2,
                                            words.end());
    return out;
  }
  if(kind == "template-param-template") {
    if(begin + 3 > words.size()) {
      throw logic_error(
          "template-param-template type requires index and template arguments");
    }
    out.kind = ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE;
    out.template_parameter_index = parse_index(words[begin + 1]);
    out.template_argument_references.assign(words.begin() + begin + 2,
                                            words.end());
    return out;
  }
  if(kind == "std-template") {
    if(begin + 4 > words.size()) {
      throw logic_error(
          "std-template type requires substitution, includes flag, and name");
    }
    out.kind = ABI_TYPE_STD_CLASS_TEMPLATE;
    out.std_substitution = std_substitution_from_word(words[begin + 1]);
    if(out.std_substitution == ABI_STD_SUBSTITUTION_NONE &&
       words[begin + 1] != "none" && words[begin + 1] != "-") {
      throw logic_error("unknown std substitution '" + words[begin + 1] + "'");
    }
    out.std_substitution_includes_template_arguments =
        boolean_word(words[begin + 2]);
    out.name = words[begin + 3];
    out.template_argument_references.assign(words.begin() + begin + 4,
                                            words.end());
    return out;
  }
  if(kind == "member") {
    if(begin + 3 != words.size()) {
      throw logic_error("member type requires owner type and member name");
    }
    out.kind = ABI_TYPE_MEMBER_TYPE;
    out.child_types.push_back(parse_single_type_token(words[begin + 1]));
    out.name = words[begin + 2];
    return out;
  }
  if(kind == "member-template") {
    if(begin + 3 > words.size()) {
      throw logic_error("member-template type requires owner type and name");
    }
    out.kind = ABI_TYPE_MEMBER_CLASS_TEMPLATE;
    out.child_types.push_back(parse_single_type_token(words[begin + 1]));
    out.name = words[begin + 2];
    out.template_argument_references.assign(words.begin() + begin + 3,
                                            words.end());
    return out;
  }
  if(kind == "decltype") {
    if(begin + 2 != words.size()) {
      throw logic_error("decltype type requires one expression reference");
    }
    out.kind = ABI_TYPE_DECLTYPE;
    out.expression_reference = words[begin + 1];
    return out;
  }
  if(kind == "lambda-closure") {
    if(begin + 3 > words.size()) {
      throw logic_error("lambda-closure type requires context and discriminator");
    }
    out.kind = ABI_TYPE_LAMBDA_CLOSURE;
    out.context_reference = words[begin + 1];
    out.discriminator = words[begin + 2];
    for(size_t i = begin + 3; i < words.size(); ++i) {
      out.child_types.push_back(parse_single_type_token(words[i]));
    }
    return out;
  }
  if(kind == "local-type") {
    if(begin + 4 != words.size()) {
      throw logic_error("local-type requires context, source name, discriminator");
    }
    out.kind = ABI_TYPE_LOCAL_TYPE;
    out.context_reference = words[begin + 1];
    out.source_name = words[begin + 2];
    out.discriminator = words[begin + 3];
    return out;
  }

  throw logic_error("unknown ABI fact type kind '" + kind + "'");
}

AbiFunctionTerminal terminal_from_fact_word(const string & word)
{
  if(word == "operator-call") {
    return ABI_FUNCTION_TERMINAL_OPERATOR_CALL;
  }
  if(word == "operator-assign") {
    return ABI_FUNCTION_TERMINAL_OPERATOR_ASSIGN;
  }
  if(word == "operator-code") {
    return ABI_FUNCTION_TERMINAL_OPERATOR_CODE;
  }
  if(word == "conversion") {
    return ABI_FUNCTION_TERMINAL_CONVERSION;
  }
  if(word == "constructor-complete") {
    return ABI_FUNCTION_TERMINAL_CONSTRUCTOR_COMPLETE;
  }
  if(word == "constructor-base") {
    return ABI_FUNCTION_TERMINAL_CONSTRUCTOR_BASE;
  }
  if(word == "destructor-complete") {
    return ABI_FUNCTION_TERMINAL_DESTRUCTOR_COMPLETE;
  }
  if(word == "destructor-base") {
    return ABI_FUNCTION_TERMINAL_DESTRUCTOR_BASE;
  }
  if(word == "destructor-deleting") {
    return ABI_FUNCTION_TERMINAL_DESTRUCTOR_DELETING;
  }
  AbiFunctionTerminal terminal = ABI_FUNCTION_TERMINAL_SOURCE_NAME;
  (void)terminal;
  throw logic_error("unknown ABI fact terminal name '" + word + "'");
}

string terminal_word_from_fact_terminal(AbiFunctionTerminal terminal)
{
  switch(terminal) {
  case ABI_FUNCTION_TERMINAL_OPERATOR_CALL:
    return "operator-call";
  case ABI_FUNCTION_TERMINAL_OPERATOR_ASSIGN:
    return "operator-assign";
  case ABI_FUNCTION_TERMINAL_OPERATOR_CODE:
    return "operator-code";
  case ABI_FUNCTION_TERMINAL_CONVERSION:
    return "conversion";
  case ABI_FUNCTION_TERMINAL_CONSTRUCTOR_COMPLETE:
    return "constructor-complete";
  case ABI_FUNCTION_TERMINAL_CONSTRUCTOR_BASE:
    return "constructor-base";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_COMPLETE:
    return "destructor-complete";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_BASE:
    return "destructor-base";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_DELETING:
    return "destructor-deleting";
  case ABI_FUNCTION_TERMINAL_SOURCE_NAME:
    break;
  }
  throw logic_error("source-name terminal is not serialized as a terminal word");
}

bool single_type_token(const AbiType & type, string & token)
{
  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    token = type.reference;
    return true;
  case ABI_TYPE_BUILTIN:
    token = type.name.empty() ? builtin_name(type.builtin_type) : type.name;
    return !token.empty();
  case ABI_TYPE_POINTER:
  case ABI_TYPE_LVALUE_REFERENCE:
  case ABI_TYPE_RVALUE_REFERENCE:
  case ABI_TYPE_CONST:
  case ABI_TYPE_VOLATILE:
  case ABI_TYPE_VENDOR_QUALIFIED:
  case ABI_TYPE_BUILTIN_TYPE_TRANSFORM: {
    if(type.child_types.size() != 1) {
      return false;
    }
    string child;
    if(!single_type_token(type.child_types[0], child)) {
      return false;
    }
    if(type.kind == ABI_TYPE_POINTER) {
      token = "ptr:" + child;
    } else if(type.kind == ABI_TYPE_LVALUE_REFERENCE) {
      token = "ref:" + child;
    } else if(type.kind == ABI_TYPE_RVALUE_REFERENCE) {
      token = "rref:" + child;
    } else if(type.kind == ABI_TYPE_CONST) {
      token = "const:" + child;
    } else if(type.kind == ABI_TYPE_VOLATILE) {
      token = "volatile:" + child;
    } else if(type.kind == ABI_TYPE_VENDOR_QUALIFIED) {
      string qualifier = type.name;
      if(qualifier.empty() &&
         type.vendor_qualifier == ABI_VENDOR_QUALIFIER_ATOMIC) {
        qualifier = "_Atomic";
      }
      if(qualifier.empty()) {
        return false;
      }
      token = "vendor:" + qualifier + ":" + child;
    } else {
      if(type.name.empty()) {
        return false;
      }
      token = "transform:" + type.name + ":" + child;
    }
    return true;
  }
  case ABI_TYPE_ARRAY: {
    if(type.child_types.size() != 1) {
      return false;
    }
    string child;
    if(!single_type_token(type.child_types[0], child)) {
      return false;
    }
    token = "array:" + word_from_array_bound(type.array_bound) + ":" + child;
    return true;
  }
  case ABI_TYPE_NAMED:
    token = "named:" + type.name;
    return true;
  case ABI_TYPE_MEMBER_POINTER: {
    if(type.child_types.size() != 2 ||
       type.child_types[0].kind != ABI_TYPE_NAMED) {
      return false;
    }
    string member;
    if(!single_type_token(type.child_types[1], member)) {
      return false;
    }
    token = "memberptr:" + type.child_types[0].name + ":" + member;
    return true;
  }
  default:
    return false;
  }
}

string join_words(const vector<string> & words)
{
  string out;
  for(size_t i = 0; i < words.size(); ++i) {
    if(i != 0) {
      out += ' ';
    }
    out += words[i];
  }
  return out;
}

void append_single_type_token(vector<string> & words, const AbiType & type)
{
  string token;
  if(!single_type_token(type, token)) {
    throw logic_error("complex inline ABI type must be named with let-type");
  }
  words.push_back(token);
}

void append_type_spec_words(vector<string> & words, const AbiType & type)
{
  string token;
  if(single_type_token(type, token)) {
    words.push_back(token);
    return;
  }
  switch(type.kind) {
  case ABI_TYPE_TEMPLATE_PARAMETER:
    words.push_back(type.substitutable_template_parameter ?
                    "template-param-subst" : "template-param");
    words.push_back(to_string(type.template_parameter_index));
    break;
  case ABI_TYPE_PACK_EXPANSION:
    words.push_back("pack");
    append_single_type_token(words, type.child_types[0]);
    break;
  case ABI_TYPE_VENDOR_QUALIFIED:
    words.push_back("vendor");
    words.push_back(type.name);
    append_single_type_token(words, type.child_types[0]);
    break;
  case ABI_TYPE_BUILTIN_TYPE_TRANSFORM:
    words.push_back("builtin-transform");
    words.push_back(type.name);
    append_single_type_token(words, type.child_types[0]);
    break;
  case ABI_TYPE_FUNCTION:
    words.push_back(type.variadic ? "function-type-variadic" : "function-type");
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      append_single_type_token(words, type.child_types[i]);
    }
    break;
  case ABI_TYPE_CLASS_TEMPLATE:
    words.push_back("template");
    words.push_back(type.name);
    words.insert(words.end(),
                 type.template_argument_references.begin(),
                 type.template_argument_references.end());
    break;
  case ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE:
    words.push_back("template-param-template");
    words.push_back(to_string(type.template_parameter_index));
    words.insert(words.end(),
                 type.template_argument_references.begin(),
                 type.template_argument_references.end());
    break;
  case ABI_TYPE_STD_CLASS_TEMPLATE:
    words.push_back("std-template");
    words.push_back(word_from_std_substitution(type.std_substitution));
    words.push_back(type.std_substitution_includes_template_arguments ?
                    "true" : "false");
    words.push_back(type.name);
    words.insert(words.end(),
                 type.template_argument_references.begin(),
                 type.template_argument_references.end());
    break;
  case ABI_TYPE_MEMBER_TYPE:
    words.push_back("member");
    append_single_type_token(words, type.child_types[0]);
    words.push_back(type.name);
    break;
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE:
    words.push_back("member-template");
    append_single_type_token(words, type.child_types[0]);
    words.push_back(type.name);
    words.insert(words.end(),
                 type.template_argument_references.begin(),
                 type.template_argument_references.end());
    break;
  case ABI_TYPE_DECLTYPE:
    words.push_back("decltype");
    words.push_back(type.expression_reference);
    break;
  case ABI_TYPE_LAMBDA_CLOSURE:
    words.push_back("lambda-closure");
    words.push_back(type.context_reference);
    words.push_back(type.discriminator);
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      append_single_type_token(words, type.child_types[i]);
    }
    break;
  case ABI_TYPE_LOCAL_TYPE:
    words.push_back("local-type");
    words.push_back(type.context_reference);
    words.push_back(type.source_name);
    words.push_back(type.discriminator);
    break;
  default:
    throw logic_error("unable to serialize ABI type fact");
  }
}

AbiFunctionPath parse_function_path(const vector<string> & words,
                                    size_t name_index)
{
  if(name_index >= words.size()) {
    throw logic_error("function path requires a qualified name");
  }
  AbiFunctionPath out;
  out.qualified_name = words[name_index];
  size_t i = name_index + 1;
  if(i + 1 < words.size() && words[i] == "result") {
    out.has_result_type = true;
    out.result_type = parse_single_type_token(words[i + 1]);
    i += 2;
  }
  for(; i < words.size(); ++i) {
    if(words[i] == "variadic" || words[i] == "varargs") {
      out.variadic = true;
    } else {
      out.parameter_types.push_back(parse_single_type_token(words[i]));
    }
  }
  return out;
}

AbiTemplateArg parse_template_argument_fact(
    const vector<string> & words)
{
  if(words.size() < 4) {
    throw logic_error("let-arg requires id and argument");
  }
  AbiTemplateArg out;
  const string & kind = words[2];
  if(kind == "type") {
    out.kind = ABI_TEMPLATE_ARG_TYPE;
    out.type = parse_type_spec(words, 3);
    return out;
  }
  if(kind == "value") {
    if(words.size() != 5) {
      throw logic_error("value template argument requires type and integer");
    }
    out.kind = ABI_TEMPLATE_ARG_INTEGRAL_VALUE;
    out.type = parse_single_type_token(words[3]);
    out.integer_value = parse_signed_integer(words[4]);
    return out;
  }
  if(kind == "dependent-value") {
    if(words.size() != 6) {
      throw logic_error(
          "dependent-value template argument requires parameter type, value type or -, and integer");
    }
    out.kind = ABI_TEMPLATE_ARG_DEPENDENT_INTEGRAL_VALUE;
    out.parameter_type = parse_single_type_token(words[3]);
    if(words[4] != "-") {
      out.type = parse_single_type_token(words[4]);
    }
    out.integer_value = parse_signed_integer(words[5]);
    return out;
  }
  if(kind == "untyped-value") {
    if(words.size() != 4) {
      throw logic_error("untyped-value template argument requires an integer");
    }
    out.kind = ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE;
    out.integer_value = parse_signed_integer(words[3]);
    return out;
  }
  if(kind == "expression") {
    if(words.size() != 4) {
      throw logic_error("expression template argument requires one expression");
    }
    out.kind = ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION;
    out.expression_reference = words[3];
    return out;
  }
  if(kind == "template-entity") {
    if(words.size() != 4) {
      throw logic_error("template-entity template argument requires a qualified name");
    }
    out.kind = ABI_TEMPLATE_ARG_TEMPLATE_ENTITY;
    out.entity_reference = words[3];
    return out;
  }
  if(kind == "template-param-template") {
    if(words.size() != 4) {
      throw logic_error(
          "template-param-template template argument requires one index");
    }
    out.kind = ABI_TEMPLATE_ARG_TEMPLATE_PARAMETER_ENTITY;
    out.template_parameter_index = parse_index(words[3]);
    return out;
  }
  if(kind == "external-address" || kind == "external-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " template argument requires a raw symbol");
    }
    out.kind = ABI_TEMPLATE_ARG_EXTERNAL_ENTITY;
    out.address_of = kind == "external-address";
    out.symbol = words[3];
    return out;
  }
  if(kind == "member-external-address" || kind == "member-external-reference") {
    if(words.size() < 12) {
      throw logic_error(
          kind + " template argument requires symbol, owner, member, function flag, cv/ref flags, variadic flag, and optional parameters");
    }
    out.kind = ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY;
    out.address_of = kind == "member-external-address";
    out.symbol = words[3];
    out.owner_type = parse_single_type_token(words[4]);
    out.member_name = words[5];
    out.member_is_function = boolean_word(words[6]);
    out.member_function_const = boolean_word(words[7]);
    out.member_function_volatile = boolean_word(words[8]);
    out.member_function_lvalue_ref = boolean_word(words[9]);
    out.member_function_rvalue_ref = boolean_word(words[10]);
    out.member_function_variadic = boolean_word(words[11]);
    for(size_t i = 12; i < words.size(); ++i) {
      out.parameter_types.push_back(parse_single_type_token(words[i]));
    }
    return out;
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " template argument requires one entity");
    }
    out.kind = kind == "entity-address" ?
        ABI_TEMPLATE_ARG_ENTITY_ADDRESS : ABI_TEMPLATE_ARG_ENTITY_REFERENCE;
    out.entity_reference = words[3];
    return out;
  }
  if(kind == "pack") {
    out.kind = ABI_TEMPLATE_ARG_PACK;
    out.pack_argument_references.assign(words.begin() + 3, words.end());
    return out;
  }
  throw logic_error("unknown ABI fact template argument kind '" + kind + "'");
}

AbiExpressionOperator expression_operator_from_word(const string & word)
{
  if(word == "de") { return ABI_EXPR_OP_DEREFERENCE; }
  if(word == "ad") { return ABI_EXPR_OP_ADDRESS_OF; }
  if(word == "ps") { return ABI_EXPR_OP_UNARY_PLUS; }
  if(word == "ng") { return ABI_EXPR_OP_UNARY_MINUS; }
  if(word == "nt") { return ABI_EXPR_OP_NOT; }
  if(word == "co") { return ABI_EXPR_OP_COMPLEMENT; }
  if(word == "pl") { return ABI_EXPR_OP_ADD; }
  if(word == "dv") { return ABI_EXPR_OP_DIVIDE; }
  if(word == "rm") { return ABI_EXPR_OP_REMAINDER; }
  if(word == "eq") { return ABI_EXPR_OP_EQUAL; }
  return ABI_EXPR_OP_INVALID;
}

string word_from_expression_operator(AbiExpressionOperator op)
{
  switch(op) {
  case ABI_EXPR_OP_DEREFERENCE:
    return "de";
  case ABI_EXPR_OP_ADDRESS_OF:
    return "ad";
  case ABI_EXPR_OP_UNARY_PLUS:
    return "ps";
  case ABI_EXPR_OP_UNARY_MINUS:
    return "ng";
  case ABI_EXPR_OP_NOT:
    return "nt";
  case ABI_EXPR_OP_COMPLEMENT:
    return "co";
  case ABI_EXPR_OP_ADD:
    return "pl";
  case ABI_EXPR_OP_DIVIDE:
    return "dv";
  case ABI_EXPR_OP_REMAINDER:
    return "rm";
  case ABI_EXPR_OP_EQUAL:
    return "eq";
  case ABI_EXPR_OP_INVALID:
    break;
  }
  throw logic_error("unknown ABI expression operator");
}

string expression_operator_code(const AbiDependentExpr & expr)
{
  return expr.expression_operator_code.empty() ?
      word_from_expression_operator(expr.expression_operator) :
      expr.expression_operator_code;
}

AbiDependentExpr parse_expression_fact(const vector<string> & words)
{
  if(words.size() < 4) {
    throw logic_error("let-expr requires id and expression");
  }
  AbiDependentExpr out;
  const string & kind = words[2];
  if(kind == "template-param") {
    if(words.size() != 4) {
      throw logic_error("template-param expression requires one index");
    }
    out.kind = ABI_EXPR_TEMPLATE_PARAMETER;
    out.index = parse_index(words[3]);
    return out;
  }
  if(kind == "function-param") {
    if(words.size() != 4) {
      throw logic_error("function-param expression requires one index");
    }
    out.kind = ABI_EXPR_FUNCTION_PARAMETER;
    out.index = parse_index(words[3]);
    return out;
  }
  if(kind == "literal") {
    if(words.size() != 4) {
      throw logic_error("literal expression requires one value");
    }
    out.kind = ABI_EXPR_LITERAL;
    out.literal = words[3];
    return out;
  }
  if(kind == "integral-value") {
    if(words.size() != 5) {
      throw logic_error("integral-value expression requires type and integer");
    }
    out.kind = ABI_EXPR_INTEGRAL_VALUE;
    out.value_type = parse_single_type_token(words[3]);
    out.literal = words[4];
    return out;
  }
  if(kind == "unary") {
    if(words.size() != 5) {
      throw logic_error("unary expression requires operator and operand");
    }
    out.kind = ABI_EXPR_UNARY;
    out.expression_operator = expression_operator_from_word(words[3]);
    out.expression_operator_code = words[3];
    out.first_reference = words[4];
    return out;
  }
  if(kind == "binary") {
    if(words.size() != 6) {
      throw logic_error("binary expression requires operator and two operands");
    }
    out.kind = ABI_EXPR_BINARY;
    out.expression_operator = expression_operator_from_word(words[3]);
    out.expression_operator_code = words[3];
    out.first_reference = words[4];
    out.second_reference = words[5];
    return out;
  }
  if(kind == "conditional") {
    if(words.size() != 6) {
      throw logic_error(
          "conditional expression requires condition, true, and false operands");
    }
    out.kind = ABI_EXPR_CONDITIONAL;
    out.first_reference = words[3];
    out.second_reference = words[4];
    out.third_reference = words[5];
    return out;
  }
  if(kind == "pack") {
    if(words.size() != 4) {
      throw logic_error("pack expression requires one operand");
    }
    out.kind = ABI_EXPR_PACK_EXPANSION;
    out.first_reference = words[3];
    return out;
  }
  if(kind == "call") {
    if(words.size() < 4) {
      throw logic_error("call expression requires callee and optional arguments");
    }
    out.kind = ABI_EXPR_CALL;
    out.first_reference = words[3];
    out.argument_references.assign(words.begin() + 4, words.end());
    return out;
  }
  if(kind == "conversion") {
    if(words.size() < 4) {
      throw logic_error("conversion expression requires type and optional arguments");
    }
    out.kind = ABI_EXPR_CONVERSION;
    out.expression_operator_code = "cv";
    out.owner_type = parse_single_type_token(words[3]);
    out.argument_references.assign(words.begin() + 4, words.end());
    return out;
  }
  if(kind == "cast") {
    if(words.size() != 6) {
      throw logic_error("cast expression requires operator, type, and operand");
    }
    out.kind = ABI_EXPR_CONVERSION;
    out.expression_operator_code = words[3];
    out.owner_type = parse_single_type_token(words[4]);
    out.argument_references.push_back(words[5]);
    return out;
  }
  if(kind == "template-id") {
    if(words.size() < 4) {
      throw logic_error("template-id expression requires name and arguments");
    }
    out.kind = ABI_EXPR_TEMPLATE_ID;
    out.member_name = words[3];
    out.template_argument_references.assign(words.begin() + 4, words.end());
    return out;
  }
  if(kind == "type-trait") {
    if(words.size() < 4) {
      throw logic_error("type-trait expression requires name and type operands");
    }
    out.kind = ABI_EXPR_TYPE_TRAIT;
    out.member_name = words[3];
    for(size_t i = 4; i < words.size(); ++i) {
      out.type_arguments.push_back(parse_single_type_token(words[i]));
    }
    return out;
  }
  if(kind == "sizeof-type") {
    if(words.size() != 4) {
      throw logic_error("sizeof-type expression requires one type");
    }
    out.kind = ABI_EXPR_SIZEOF_TYPE;
    out.owner_type = parse_single_type_token(words[3]);
    return out;
  }
  if(kind == "member") {
    if(words.size() < 6) {
      throw logic_error(
          "member expression requires owner type, close flag, name, and optional template arguments");
    }
    out.kind = ABI_EXPR_MEMBER;
    out.owner_type = parse_single_type_token(words[3]);
    out.close_template_arguments = boolean_word(words[4]);
    out.member_name = words[5];
    if(words.size() > 6) {
      out.template_argument_references.assign(words.begin() + 6, words.end());
    }
    return out;
  }
  if(kind == "object-member") {
    if(words.size() < 6) {
      throw logic_error(
          "object-member expression requires operator, object, member name, and optional template arguments");
    }
    out.kind = ABI_EXPR_OBJECT_MEMBER;
    out.expression_operator_code = words[3];
    out.first_reference = words[4];
    out.member_name = words[5];
    out.template_argument_references.assign(words.begin() + 6, words.end());
    return out;
  }
  if(kind == "external-address" || kind == "external-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " expression requires one raw symbol");
    }
    out.kind = ABI_EXPR_EXTERNAL_ENTITY;
    out.address_of = kind == "external-address";
    out.symbol = words[3];
    return out;
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " expression requires one entity");
    }
    out.kind = kind == "entity-address" ?
        ABI_EXPR_ENTITY_ADDRESS : ABI_EXPR_ENTITY_REFERENCE;
    out.entity_reference = words[3];
    return out;
  }
  throw logic_error("unknown ABI fact expression kind '" + kind + "'");
}

void require_no_target(const AbiFactCase & fact_case)
{
  if(fact_case.target.kind != ABI_MANGLE_NONE) {
    throw logic_error("ABI fact case must test exactly one mangle");
  }
}

bool target_has_function(AbiMangleTargetKind kind)
{
  return kind == ABI_MANGLE_FUNCTION ||
         kind == ABI_MANGLE_THUNK ||
         kind == ABI_MANGLE_VIRTUAL_BASE_THUNK;
}

void parse_function_target(AbiFunction & function,
                           const vector<string> & words,
                           size_t begin,
                           const string & context)
{
  if(begin >= words.size()) {
    throw logic_error(context + " fact requires a function name form");
  }
  if(words[begin] == "function") {
    ++begin;
  }
  if(begin >= words.size()) {
    throw logic_error(context + " fact requires a function name form");
  }
  if(words[begin] == "path") {
    if(begin + 1 >= words.size()) {
      throw logic_error(context + " function path requires a qualified name");
    }
    function.form = ABI_FUNCTION_PATH;
    function.qualified_name = words[begin + 1];
    function.template_argument_references.assign(words.begin() + begin + 2,
                                                 words.end());
    return;
  }
  if(words[begin] == "lambda") {
    if(begin + 3 >= words.size()) {
      throw logic_error(
          context + " function lambda requires context, discriminator, and terminal");
    }
    function.form = ABI_FUNCTION_LAMBDA;
    function.context_reference = words[begin + 1];
    function.discriminator = words[begin + 2];
    function.terminal = terminal_from_fact_word(words[begin + 3]);
    for(size_t i = begin + 4; i < words.size(); ++i) {
      function.lambda_signature_parameter_types.push_back(
          parse_single_type_token(words[i]));
    }
    return;
  }
  if(words[begin] == "local") {
    if(begin + 3 >= words.size() || begin + 5 < words.size()) {
      throw logic_error(
          context + " function local requires context, source name, terminal, and optional discriminator");
    }
    function.form = ABI_FUNCTION_LOCAL;
    function.context_reference = words[begin + 1];
    function.source_name = words[begin + 2];
    function.terminal = terminal_from_fact_word(words[begin + 3]);
    function.discriminator = begin + 4 < words.size() ? words[begin + 4] : "0";
    return;
  }
  function.form = ABI_FUNCTION_PATH;
  function.qualified_name = words[begin];
  for(size_t i = begin + 1; i < words.size(); ++i) {
    if(words[i] == "variadic" || words[i] == "varargs") {
      function.variadic = true;
    } else {
      function.parameter_types.push_back(parse_single_type_token(words[i]));
    }
  }
}

void apply_fact_words(AbiFactCase & fact_case, const vector<string> & words)
{
  if(words.empty()) {
    return;
  }
  const string & command = words[0];
  if(command == "let-type") {
    if(words.size() < 3) {
      throw logic_error("let-type requires id and type");
    }
    AbiFact fact;
    fact.kind = ABI_FACT_TYPE;
    fact.id = words[1];
    fact.type = parse_type_spec(words, 2);
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-arg") {
    AbiFact fact;
    fact.kind = ABI_FACT_TEMPLATE_ARGUMENT;
    fact.id = words[1];
    fact.template_argument = parse_template_argument_fact(words);
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-expr") {
    AbiFact fact;
    fact.kind = ABI_FACT_EXPRESSION;
    fact.id = words[1];
    fact.expression = parse_expression_fact(words);
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-context") {
    if(words.size() < 4 || words[2] != "function") {
      throw logic_error(
          "let-context requires id, function, name, and optional parameters");
    }
    AbiFact fact;
    fact.kind = ABI_FACT_LOCAL_CONTEXT;
    fact.id = words[1];
    fact.context_function = parse_function_path(words, 3);
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-entity") {
    if(words.size() < 4) {
      throw logic_error("let-entity requires id, kind, and entity data");
    }
    AbiFact fact;
    fact.kind = ABI_FACT_ENTITY;
    fact.id = words[1];
    if(words[2] == "function") {
      fact.entity.kind = ABI_ENTITY_FUNCTION;
      fact.entity.function = parse_function_path(words, 3);
    } else if(words[2] == "variable") {
      if(words.size() != 4) {
        throw logic_error("let-entity variable requires a qualified name");
      }
      fact.entity.kind = ABI_ENTITY_VARIABLE;
      fact.entity.qualified_name = words[3];
    } else {
      throw logic_error("unknown ABI fact entity kind '" + words[2] + "'");
    }
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "type") {
    if(words.size() < 2) {
      throw logic_error("type fact requires a type");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_TYPE;
    fact_case.target.type = parse_type_spec(words, 1);
    return;
  }
  if(command == "typeinfo" || command == "vtable" || command == "vtt") {
    if(words.size() < 2) {
      throw logic_error(command + " fact requires a type");
    }
    require_no_target(fact_case);
    fact_case.target.kind =
        command == "typeinfo" ? ABI_MANGLE_TYPEINFO :
        command == "vtable" ? ABI_MANGLE_VTABLE :
                               ABI_MANGLE_VTT;
    fact_case.target.type = parse_type_spec(words, 1);
    return;
  }
  if(command == "construction-vtable") {
    if(words.size() != 4) {
      throw logic_error(
          "construction-vtable requires dynamic type, base offset, and base type");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_CONSTRUCTION_VTABLE;
    fact_case.target.type = parse_single_type_token(words[1]);
    if(!parse_unsigned_integer_word(words[2], fact_case.target.base_offset)) {
      throw logic_error("construction-vtable base offset must be decimal");
    }
    fact_case.target.base_type = parse_single_type_token(words[3]);
    return;
  }
  if(command == "tls-wrapper" || command == "thread-local-wrapper") {
    if(words.size() != 3 ||
       (words[1] != "variable" && words[1] != "c-variable")) {
      throw logic_error(command + " requires variable or c-variable and a qualified name");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_THREAD_LOCAL_WRAPPER;
    fact_case.target.c_linkage = words[1] == "c-variable";
    fact_case.target.qualified_name = words[2];
    return;
  }
  if(command == "thunk") {
    if(words.size() < 4) {
      throw logic_error(
          "thunk requires this adjustment, optional result adjustment, and a function target");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_THUNK;
    fact_case.target.this_adjust = parse_signed_integer(words[1]);
    size_t function_begin = 2;
    if(words[function_begin] != "function") {
      if(function_begin + 1 >= words.size() || words[function_begin + 1] != "function") {
        throw logic_error("thunk requires a function target");
      }
      fact_case.target.has_result_adjust = true;
      fact_case.target.result_adjust = parse_signed_integer(words[function_begin]);
      ++function_begin;
    }
    parse_function_target(fact_case.target.function, words, function_begin, command);
    return;
  }
  if(command == "virtual-base-thunk") {
    if(words.size() < 4) {
      throw logic_error(
          "virtual-base-thunk requires vcall offset and a function target");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_VIRTUAL_BASE_THUNK;
    fact_case.target.vcall_offset = parse_signed_integer(words[1]);
    parse_function_target(fact_case.target.function, words, 2, command);
    return;
  }
  if(command == "variable" || command == "c-variable") {
    if(words.size() != 2 && !(words.size() == 3 && words[1] == "path")) {
      throw logic_error(command + " requires a qualified name");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_VARIABLE;
    fact_case.target.c_linkage = command == "c-variable";
    fact_case.target.qualified_name =
        words.size() == 3 ? words[2] : words[1];
    return;
  }
  if(command == "function" || command == "c-function") {
    if(words.size() < 2) {
      throw logic_error(command + " fact requires a function name form");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_FUNCTION;
    AbiFunction & function = fact_case.target.function;
    function.c_linkage = command == "c-function";
    parse_function_target(function, words, 1, command);
	return;
      }
  if(command == "variadic" || command == "varargs") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error(command + " appears before function fact");
    }
    if(words.size() != 1) {
      throw logic_error(command + " takes no operands");
    }
    fact_case.target.function.variadic = true;
    return;
  }
  if(command == "abi-tag") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error("abi-tag appears before function fact");
    }
    if(words.size() != 2) {
      throw logic_error("abi-tag requires one tag");
    }
    fact_case.target.function.abi_tags.push_back(words[1]);
    return;
  }
  if(command == "function-qualifier" || command == "qualifier") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error(command + " appears before function fact");
    }
    for(size_t i = 1; i < words.size(); ++i) {
      if(words[i] == "const") {
        fact_case.target.function.nested_const = true;
      } else if(words[i] == "volatile") {
        fact_case.target.function.nested_volatile = true;
      } else if(words[i] == "lvalue-ref" || words[i] == "ref") {
        fact_case.target.function.nested_lvalue_ref = true;
      } else if(words[i] == "rvalue-ref" || words[i] == "rref") {
        fact_case.target.function.nested_rvalue_ref = true;
      } else {
        throw logic_error("unknown function qualifier '" + words[i] + "'");
      }
    }
    return;
  }
  if(command == "operator-terminal") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error("operator-terminal appears before function fact");
    }
    if(words.size() != 2) {
      throw logic_error("operator-terminal requires one Itanium operator code");
    }
    fact_case.target.function.terminal = ABI_FUNCTION_TERMINAL_OPERATOR_CODE;
    fact_case.target.function.terminal_operator_code = words[1];
    return;
  }
  if(command == "conversion-terminal") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error("conversion-terminal appears before function fact");
    }
    if(words.size() < 2) {
      throw logic_error("conversion-terminal requires a type");
    }
    fact_case.target.function.terminal = ABI_FUNCTION_TERMINAL_CONVERSION;
    fact_case.target.function.conversion_type = parse_type_spec(words, 1);
    return;
  }
  if(command == "param") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error("param appears before function fact");
    }
    if(words.size() < 2) {
      throw logic_error("param requires a type");
    }
    fact_case.target.function.parameter_types.push_back(
        parse_type_spec(words, 1));
    return;
  }
  if(command == "result") {
    if(!target_has_function(fact_case.target.kind)) {
      throw logic_error("result appears before function fact");
    }
    if(words.size() < 2) {
      throw logic_error("result requires a type");
    }
    fact_case.target.function.has_result_type = true;
    fact_case.target.function.result_type = parse_type_spec(words, 1);
    return;
  }
  throw logic_error("unknown ABI fact command '" + command + "'");
}

void append_function_path_words(vector<string> & words,
                                const AbiFunctionPath & path)
{
  words.push_back("function");
  words.push_back(path.qualified_name);
  if(path.has_result_type) {
    words.push_back("result");
    append_single_type_token(words, path.result_type);
  }
  for(size_t i = 0; i < path.parameter_types.size(); ++i) {
    append_single_type_token(words, path.parameter_types[i]);
  }
  if(path.variadic) {
    words.push_back("variadic");
  }
}

vector<string> fact_words(const AbiFact & fact)
{
  vector<string> words;
  switch(fact.kind) {
  case ABI_FACT_TYPE:
    words.push_back("let-type");
    words.push_back(fact.id);
    append_type_spec_words(words, fact.type);
    return words;
  case ABI_FACT_TEMPLATE_ARGUMENT:
    words.push_back("let-arg");
    words.push_back(fact.id);
    switch(fact.template_argument.kind) {
    case ABI_TEMPLATE_ARG_TYPE:
      words.push_back("type");
      append_type_spec_words(words, fact.template_argument.type);
      break;
    case ABI_TEMPLATE_ARG_INTEGRAL_VALUE:
      words.push_back("value");
      append_single_type_token(words, fact.template_argument.type);
      words.push_back(to_string(fact.template_argument.integer_value));
      break;
    case ABI_TEMPLATE_ARG_DEPENDENT_INTEGRAL_VALUE:
      words.push_back("dependent-value");
      append_single_type_token(words, fact.template_argument.parameter_type);
      if(fact.template_argument.type.kind == ABI_TYPE_REFERENCE &&
         fact.template_argument.type.reference.empty()) {
        words.push_back("-");
      } else {
        append_single_type_token(words, fact.template_argument.type);
      }
      words.push_back(to_string(fact.template_argument.integer_value));
      break;
    case ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE:
      words.push_back("untyped-value");
      words.push_back(to_string(fact.template_argument.integer_value));
      break;
    case ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION:
      words.push_back("expression");
      words.push_back(fact.template_argument.expression_reference);
      break;
    case ABI_TEMPLATE_ARG_TEMPLATE_ENTITY:
      words.push_back("template-entity");
      words.push_back(fact.template_argument.entity_reference);
      break;
    case ABI_TEMPLATE_ARG_TEMPLATE_PARAMETER_ENTITY:
      words.push_back("template-param-template");
      words.push_back(to_string(fact.template_argument.template_parameter_index));
      break;
    case ABI_TEMPLATE_ARG_EXTERNAL_ENTITY:
      words.push_back(fact.template_argument.address_of ?
                      "external-address" : "external-reference");
      words.push_back(fact.template_argument.symbol);
      break;
    case ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY:
      words.push_back(fact.template_argument.address_of ?
                      "member-external-address" : "member-external-reference");
      words.push_back(fact.template_argument.symbol);
      append_single_type_token(words, fact.template_argument.owner_type);
      words.push_back(fact.template_argument.member_name);
      words.push_back(fact.template_argument.member_is_function ? "yes" : "no");
      words.push_back(fact.template_argument.member_function_const ? "yes" : "no");
      words.push_back(fact.template_argument.member_function_volatile ? "yes" : "no");
      words.push_back(fact.template_argument.member_function_lvalue_ref ? "yes" : "no");
      words.push_back(fact.template_argument.member_function_rvalue_ref ? "yes" : "no");
      words.push_back(fact.template_argument.member_function_variadic ? "yes" : "no");
      for(size_t i = 0; i < fact.template_argument.parameter_types.size(); ++i) {
        append_single_type_token(words, fact.template_argument.parameter_types[i]);
      }
      break;
    case ABI_TEMPLATE_ARG_ENTITY_ADDRESS:
      words.push_back("entity-address");
      words.push_back(fact.template_argument.entity_reference);
      break;
    case ABI_TEMPLATE_ARG_ENTITY_REFERENCE:
      words.push_back("entity-reference");
      words.push_back(fact.template_argument.entity_reference);
      break;
    case ABI_TEMPLATE_ARG_PACK:
      words.push_back("pack");
      words.insert(words.end(),
                   fact.template_argument.pack_argument_references.begin(),
                   fact.template_argument.pack_argument_references.end());
      break;
    }
    return words;
  case ABI_FACT_EXPRESSION:
    words.push_back("let-expr");
    words.push_back(fact.id);
    switch(fact.expression.kind) {
    case ABI_EXPR_TEMPLATE_PARAMETER:
      words.push_back("template-param");
      words.push_back(to_string(fact.expression.index));
      break;
    case ABI_EXPR_FUNCTION_PARAMETER:
      words.push_back("function-param");
      words.push_back(to_string(fact.expression.index));
      break;
    case ABI_EXPR_LITERAL:
      words.push_back("literal");
      words.push_back(fact.expression.literal);
      break;
    case ABI_EXPR_INTEGRAL_VALUE:
      words.push_back("integral-value");
      append_single_type_token(words, fact.expression.value_type);
      words.push_back(fact.expression.literal);
      break;
    case ABI_EXPR_OBJECT_MEMBER:
      words.push_back("object-member");
      words.push_back(fact.expression.expression_operator_code);
      words.push_back(fact.expression.first_reference);
      words.push_back(fact.expression.member_name);
      words.insert(words.end(),
                   fact.expression.template_argument_references.begin(),
                   fact.expression.template_argument_references.end());
      break;
    case ABI_EXPR_UNARY:
      words.push_back("unary");
      words.push_back(fact.expression.expression_operator_code.empty() ?
          word_from_expression_operator(fact.expression.expression_operator) :
          fact.expression.expression_operator_code);
      words.push_back(fact.expression.first_reference);
      break;
    case ABI_EXPR_BINARY:
      words.push_back("binary");
      words.push_back(fact.expression.expression_operator_code.empty() ?
          word_from_expression_operator(fact.expression.expression_operator) :
          fact.expression.expression_operator_code);
      words.push_back(fact.expression.first_reference);
      words.push_back(fact.expression.second_reference);
      break;
    case ABI_EXPR_CONDITIONAL:
      words.push_back("conditional");
      words.push_back(fact.expression.first_reference);
      words.push_back(fact.expression.second_reference);
      words.push_back(fact.expression.third_reference);
      break;
    case ABI_EXPR_PACK_EXPANSION:
      words.push_back("pack");
      words.push_back(fact.expression.first_reference);
      break;
    case ABI_EXPR_CALL:
      words.push_back("call");
      words.push_back(fact.expression.first_reference);
      words.insert(words.end(),
                   fact.expression.argument_references.begin(),
                   fact.expression.argument_references.end());
      break;
    case ABI_EXPR_CONVERSION:
      if(fact.expression.expression_operator_code.empty() ||
         fact.expression.expression_operator_code == "cv") {
        words.push_back("conversion");
        append_single_type_token(words, fact.expression.owner_type);
        words.insert(words.end(),
                     fact.expression.argument_references.begin(),
                     fact.expression.argument_references.end());
      } else {
        words.push_back("cast");
        words.push_back(fact.expression.expression_operator_code);
        append_single_type_token(words, fact.expression.owner_type);
        if(fact.expression.argument_references.size() != 1) {
          throw logic_error("cast expression requires exactly one operand");
        }
        words.push_back(fact.expression.argument_references[0]);
      }
      break;
    case ABI_EXPR_TEMPLATE_ID:
      words.push_back("template-id");
      words.push_back(fact.expression.member_name);
      words.insert(words.end(),
                   fact.expression.template_argument_references.begin(),
                   fact.expression.template_argument_references.end());
      break;
    case ABI_EXPR_TYPE_TRAIT:
      words.push_back("type-trait");
      words.push_back(fact.expression.member_name);
      for(size_t i = 0; i < fact.expression.type_arguments.size(); ++i) {
        append_single_type_token(words, fact.expression.type_arguments[i]);
      }
      break;
    case ABI_EXPR_SIZEOF_TYPE:
      words.push_back("sizeof-type");
      append_single_type_token(words, fact.expression.owner_type);
      break;
    case ABI_EXPR_MEMBER:
      words.push_back("member");
      append_single_type_token(words, fact.expression.owner_type);
      words.push_back(fact.expression.close_template_arguments ? "yes" : "no");
      words.push_back(fact.expression.member_name);
      words.insert(words.end(),
                   fact.expression.template_argument_references.begin(),
                   fact.expression.template_argument_references.end());
      break;
    case ABI_EXPR_EXTERNAL_ENTITY:
      words.push_back(fact.expression.address_of ?
                      "external-address" : "external-reference");
      words.push_back(fact.expression.symbol);
      break;
    case ABI_EXPR_ENTITY_ADDRESS:
      words.push_back("entity-address");
      words.push_back(fact.expression.entity_reference);
      break;
    case ABI_EXPR_ENTITY_REFERENCE:
      words.push_back("entity-reference");
      words.push_back(fact.expression.entity_reference);
      break;
    }
    return words;
  case ABI_FACT_LOCAL_CONTEXT:
    words.push_back("let-context");
    words.push_back(fact.id);
    append_function_path_words(words, fact.context_function);
    return words;
  case ABI_FACT_ENTITY:
    words.push_back("let-entity");
    words.push_back(fact.id);
    if(fact.entity.kind == ABI_ENTITY_FUNCTION) {
      append_function_path_words(words, fact.entity.function);
    } else {
      words.push_back("variable");
      words.push_back(fact.entity.qualified_name);
    }
    return words;
  }
  throw logic_error("unknown ABI fact kind");
}

void append_function_target_words(vector<string> & words,
                                  const AbiFunction & function,
                                  const string & command)
{
  words.push_back(function.c_linkage && command == "function" ?
                  "c-function" : command);
  if(function.form == ABI_FUNCTION_PATH) {
    words.push_back("path");
    words.push_back(function.qualified_name);
    words.insert(words.end(),
                 function.template_argument_references.begin(),
                 function.template_argument_references.end());
  } else if(function.form == ABI_FUNCTION_LAMBDA) {
    words.push_back("lambda");
    words.push_back(function.context_reference);
    words.push_back(function.discriminator);
    words.push_back(terminal_word_from_fact_terminal(function.terminal));
    for(size_t i = 0;
        i < function.lambda_signature_parameter_types.size();
        ++i) {
      append_single_type_token(words,
                               function.lambda_signature_parameter_types[i]);
    }
  } else {
    words.push_back("local");
    words.push_back(function.context_reference);
    words.push_back(function.source_name);
    words.push_back(terminal_word_from_fact_terminal(function.terminal));
    words.push_back(function.discriminator);
  }
}

void append_function_modifier_lines(vector<vector<string> > & lines,
                                    const AbiFunction & function)
{
  if(function.terminal == ABI_FUNCTION_TERMINAL_OPERATOR_CODE) {
    vector<string> terminal_words;
    terminal_words.push_back("operator-terminal");
    terminal_words.push_back(function.terminal_operator_code);
    lines.push_back(terminal_words);
  } else if(function.terminal == ABI_FUNCTION_TERMINAL_CONVERSION) {
    vector<string> terminal_words;
    terminal_words.push_back("conversion-terminal");
    append_type_spec_words(terminal_words, function.conversion_type);
    lines.push_back(terminal_words);
  }
  if(function.nested_const ||
     function.nested_volatile ||
     function.nested_lvalue_ref ||
     function.nested_rvalue_ref) {
    vector<string> qualifier_words;
    qualifier_words.push_back("function-qualifier");
    if(function.nested_const) {
      qualifier_words.push_back("const");
    }
    if(function.nested_volatile) {
      qualifier_words.push_back("volatile");
    }
    if(function.nested_lvalue_ref) {
      qualifier_words.push_back("lvalue-ref");
    }
    if(function.nested_rvalue_ref) {
      qualifier_words.push_back("rvalue-ref");
    }
    lines.push_back(qualifier_words);
  }
  for(size_t i = 0; i < function.abi_tags.size(); ++i) {
    vector<string> tag_words;
    tag_words.push_back("abi-tag");
    tag_words.push_back(function.abi_tags[i]);
    lines.push_back(tag_words);
  }
  if(function.has_result_type) {
    vector<string> result_words;
    result_words.push_back("result");
    append_type_spec_words(result_words, function.result_type);
    lines.push_back(result_words);
  }
  for(size_t i = 0; i < function.parameter_types.size(); ++i) {
    vector<string> param_words;
    param_words.push_back("param");
    append_type_spec_words(param_words, function.parameter_types[i]);
    lines.push_back(param_words);
  }
  if(function.variadic) {
    vector<string> variadic_words;
    variadic_words.push_back("variadic");
    lines.push_back(variadic_words);
  }
}

vector<vector<string> > target_lines(const AbiMangleTarget & target)
{
  vector<vector<string> > lines;
  vector<string> words;
  switch(target.kind) {
  case ABI_MANGLE_NONE:
    return lines;
  case ABI_MANGLE_TYPE:
    words.push_back("type");
    append_type_spec_words(words, target.type);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_TYPEINFO:
    words.push_back("typeinfo");
    append_type_spec_words(words, target.type);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_VTABLE:
    words.push_back("vtable");
    append_type_spec_words(words, target.type);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_VTT:
    words.push_back("vtt");
    append_type_spec_words(words, target.type);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_CONSTRUCTION_VTABLE:
    words.push_back("construction-vtable");
    append_single_type_token(words, target.type);
    words.push_back(to_string(target.base_offset));
    append_single_type_token(words, target.base_type);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_THREAD_LOCAL_WRAPPER:
    words.push_back("tls-wrapper");
    words.push_back(target.c_linkage ? "c-variable" : "variable");
    words.push_back(target.qualified_name);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_THUNK:
    words.push_back("thunk");
    words.push_back(to_string(target.this_adjust));
    if(target.has_result_adjust) {
      words.push_back(to_string(target.result_adjust));
    }
    append_function_target_words(words, target.function, "function");
    lines.push_back(words);
    append_function_modifier_lines(lines, target.function);
    return lines;
  case ABI_MANGLE_VIRTUAL_BASE_THUNK:
    words.push_back("virtual-base-thunk");
    words.push_back(to_string(target.vcall_offset));
    append_function_target_words(words, target.function, "function");
    lines.push_back(words);
    append_function_modifier_lines(lines, target.function);
    return lines;
  case ABI_MANGLE_VARIABLE:
    words.push_back(target.c_linkage ? "c-variable" : "variable");
    words.push_back(target.qualified_name);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_FUNCTION:
    append_function_target_words(words, target.function, "function");
    lines.push_back(words);
    append_function_modifier_lines(lines, target.function);
    return lines;
  }
  throw logic_error("unknown ABI mangle target kind");
}

enum DirectSubstitutionKeyKind
{
  DIRECT_SUBST_EMPTY,
  DIRECT_SUBST_NAME,
  DIRECT_SUBST_TYPE,
  DIRECT_SUBST_TEMPLATE_ARGUMENT,
  DIRECT_SUBST_EXPRESSION,
  DIRECT_SUBST_FUNCTION_PATH,
  DIRECT_SUBST_FUNCTION_TEMPLATE_PREFIX,
  DIRECT_SUBST_ENTITY
};

struct DirectSubstitutionKey
{
  DirectSubstitutionKeyKind kind = DIRECT_SUBST_EMPTY;
  AbiTypeKind type_kind = ABI_TYPE_REFERENCE;
  AbiTemplateArgumentKind template_arg_kind = ABI_TEMPLATE_ARG_TYPE;
  AbiDependentExpressionKind expression_kind = ABI_EXPR_LITERAL;
  AbiExpressionOperator expression_operator = ABI_EXPR_OP_INVALID;
  AbiEntityKind entity_kind = ABI_ENTITY_VARIABLE;
  AbiBuiltinType builtin_type = ABI_BUILTIN_INVALID;
  AbiVendorQualifier vendor_qualifier = ABI_VENDOR_QUALIFIER_NONE;
  AbiStdSubstitution std_substitution = ABI_STD_SUBSTITUTION_NONE;
  AbiArrayBoundKind array_bound_kind = ABI_ARRAY_BOUND_NONE;
  AbiFunctionTerminal terminal = ABI_FUNCTION_TERMINAL_SOURCE_NAME;
  size_t index = 0;
  unsigned long long unsigned_value = 0;
  long long signed_value = 0;
  bool flag = false;
  bool second_flag = false;
  bool third_flag = false;
  string name;
  string secondary_name;
  string literal;
  vector<DirectSubstitutionKey> children;

  bool empty() const { return kind == DIRECT_SUBST_EMPTY; }

  bool operator==(const DirectSubstitutionKey & rhs) const
  {
    return kind == rhs.kind &&
           type_kind == rhs.type_kind &&
           template_arg_kind == rhs.template_arg_kind &&
           expression_kind == rhs.expression_kind &&
           expression_operator == rhs.expression_operator &&
           entity_kind == rhs.entity_kind &&
           builtin_type == rhs.builtin_type &&
           vendor_qualifier == rhs.vendor_qualifier &&
           std_substitution == rhs.std_substitution &&
           array_bound_kind == rhs.array_bound_kind &&
           terminal == rhs.terminal &&
           index == rhs.index &&
           unsigned_value == rhs.unsigned_value &&
           signed_value == rhs.signed_value &&
           flag == rhs.flag &&
           second_flag == rhs.second_flag &&
           third_flag == rhs.third_flag &&
           name == rhs.name &&
           secondary_name == rhs.secondary_name &&
           literal == rhs.literal &&
           children == rhs.children;
  }
};

struct DirectEncoder
{
  vector<DirectSubstitutionKey> substitutions;
};

struct DirectLocalContext
{
  AbiFunctionPath function;
};

struct DirectFactContext
{
  map<string, AbiType> types;
  map<string, AbiTemplateArg> args;
  map<string, AbiDependentExpr> exprs;
  map<string, DirectLocalContext> contexts;
  map<string, AbiEntity> entities;
};

const AbiTemplateArg & direct_require_arg_ref(const DirectFactContext & ctx,
                                              const string & id)
{
  map<string, AbiTemplateArg>::const_iterator found = ctx.args.find(id);
  if(found == ctx.args.end()) {
    throw logic_error("unknown ABI fact template argument reference '" + id + "'");
  }
  return found->second;
}

const AbiDependentExpr & direct_require_expr_ref(const DirectFactContext & ctx,
                                                 const string & id)
{
  map<string, AbiDependentExpr>::const_iterator found = ctx.exprs.find(id);
  if(found == ctx.exprs.end()) {
    throw logic_error("unknown ABI fact expression reference '" + id + "'");
  }
  return found->second;
}

const AbiEntity & direct_require_entity_ref(const DirectFactContext & ctx,
                                            const string & id)
{
  map<string, AbiEntity>::const_iterator found = ctx.entities.find(id);
  if(found == ctx.entities.end()) {
    throw logic_error("unknown ABI fact entity reference '" + id + "'");
  }
  return found->second;
}

const DirectLocalContext & direct_require_context_ref(
    const DirectFactContext & ctx,
    const string & id)
{
  map<string, DirectLocalContext>::const_iterator found = ctx.contexts.find(id);
  if(found == ctx.contexts.end()) {
    throw logic_error("unknown ABI fact local context reference '" + id + "'");
  }
  return found->second;
}

DirectSubstitutionKey direct_name_key(const string & qualified_name)
{
  DirectSubstitutionKey out;
  out.kind = DIRECT_SUBST_NAME;
  out.name = qualified_name;
  return out;
}

DirectSubstitutionKey direct_function_template_prefix_key(
    const string & qualified_name)
{
  DirectSubstitutionKey out;
  out.kind = DIRECT_SUBST_FUNCTION_TEMPLATE_PREFIX;
  out.name = qualified_name;
  return out;
}

bool direct_emit_substitution(DirectEncoder & encoder,
                              const DirectSubstitutionKey & key,
                              string & out)
{
  if(key.empty()) {
    return false;
  }
  for(size_t i = 0; i < encoder.substitutions.size(); ++i) {
    if(encoder.substitutions[i] == key) {
      out += 'S';
      if(i != 0) {
        out += base36_number(i - 1);
      }
      out += '_';
      return true;
    }
  }
  return false;
}

void direct_register_substitution(DirectEncoder & encoder,
                                  const DirectSubstitutionKey & key)
{
  if(key.empty()) {
    return;
  }
  for(size_t i = 0; i < encoder.substitutions.size(); ++i) {
    if(encoder.substitutions[i] == key) {
      return;
    }
  }
  encoder.substitutions.push_back(key);
}

bool direct_emit_source_name(const string & name, string & out)
{
  if(name.empty()) {
    return false;
  }
  out += to_string(name.size());
  out += name;
  return true;
}

bool direct_builtin_code_from_type(const AbiType & type, string & out)
{
  out = builtin_code(type.builtin_type);
  if(!out.empty()) {
    return true;
  }
  return builtin_code_from_name(type.name, out);
}

const AbiType & direct_resolve_type_ref(const DirectFactContext & ctx,
                                        const AbiType & type,
                                        AbiType & builtin_storage)
{
  if(type.kind != ABI_TYPE_REFERENCE) {
    return type;
  }
  map<string, AbiType>::const_iterator found = ctx.types.find(type.reference);
  if(found != ctx.types.end()) {
    return found->second;
  }
  if(is_builtin_type_name(type.reference)) {
    builtin_storage = builtin_type_spec(type.reference);
    return builtin_storage;
  }
  throw logic_error("unknown ABI fact type reference '" + type.reference + "'");
}

string direct_join_key_parts(const vector<string> & parts)
{
  string out;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i != 0) {
      out += "::";
    }
    out += parts[i];
  }
  return out;
}

bool direct_type_key(const DirectFactContext & ctx,
                     const AbiType & type,
                     DirectSubstitutionKey & out);
bool direct_type_identity_key(const DirectFactContext & ctx,
                              const AbiType & type,
                              DirectSubstitutionKey & out);
bool direct_template_arg_key(const DirectFactContext & ctx,
                             const AbiTemplateArg & arg,
                             DirectSubstitutionKey & out);
bool direct_expression_key(const DirectFactContext & ctx,
                           const AbiDependentExpr & expr,
                           DirectSubstitutionKey & out);
bool direct_entity_key(const DirectFactContext & ctx,
                       const AbiEntity & entity,
                       DirectSubstitutionKey & out);
bool direct_function_path_key(const DirectFactContext & ctx,
                              const AbiFunctionPath & path,
                              DirectSubstitutionKey & out);

bool direct_function_path_key(const DirectFactContext & ctx,
                              const AbiFunctionPath & path,
                              DirectSubstitutionKey & out)
{
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_FUNCTION_PATH;
  out.name = path.qualified_name;
  out.index = path.template_argument_references.size();
  out.unsigned_value = path.parameter_types.size();
  out.flag = path.has_result_type;
  out.second_flag = path.variadic;
  for(size_t i = 0; i < path.template_argument_references.size(); ++i) {
    DirectSubstitutionKey arg_key;
    if(!direct_template_arg_key(
           ctx,
           direct_require_arg_ref(ctx, path.template_argument_references[i]),
           arg_key)) {
      return false;
    }
    out.children.push_back(arg_key);
  }
  if(path.has_result_type) {
    DirectSubstitutionKey result_key;
    if(!direct_type_identity_key(ctx, path.result_type, result_key)) {
      return false;
    }
    out.children.push_back(result_key);
  }
  for(size_t i = 0; i < path.parameter_types.size(); ++i) {
    DirectSubstitutionKey param_key;
    if(!direct_type_identity_key(ctx, path.parameter_types[i], param_key)) {
      return false;
    }
    out.children.push_back(param_key);
  }
  return true;
}

bool direct_entity_key(const DirectFactContext & ctx,
                       const AbiEntity & entity,
                       DirectSubstitutionKey & out)
{
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_ENTITY;
  out.entity_kind = entity.kind;
  if(entity.kind == ABI_ENTITY_FUNCTION) {
    DirectSubstitutionKey path_key;
    if(!direct_function_path_key(ctx, entity.function, path_key)) {
      return false;
    }
    out.children.push_back(path_key);
  } else {
    out.name = entity.qualified_name;
  }
  return true;
}

bool direct_array_bound_identity_key(const DirectFactContext & ctx,
                                     const AbiArrayBound & bound,
                                     DirectSubstitutionKey & out)
{
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_TYPE;
  out.type_kind = ABI_TYPE_ARRAY;
  out.array_bound_kind = bound.kind;
  if(bound.kind == ABI_ARRAY_BOUND_INTEGER) {
    out.unsigned_value = bound.integer_value;
    return true;
  }
  if(bound.kind == ABI_ARRAY_BOUND_EXPRESSION) {
    DirectSubstitutionKey expr_key;
    if(!direct_expression_key(
           ctx,
           direct_require_expr_ref(ctx, bound.expression_reference),
           expr_key)) {
      return false;
    }
    out.children.push_back(expr_key);
    return true;
  }
  return false;
}

bool direct_type_identity_key(const DirectFactContext & ctx,
                              const AbiType & input,
                              DirectSubstitutionKey & out)
{
  AbiType builtin_storage;
  const AbiType & type = direct_resolve_type_ref(ctx, input, builtin_storage);
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_TYPE;
  out.type_kind = type.kind;
  out.builtin_type = type.builtin_type;
  out.vendor_qualifier = type.vendor_qualifier;
  out.std_substitution = type.std_substitution;
  out.index = type.template_parameter_index;
  out.flag = type.substitutable_template_parameter;
  out.second_flag = type.std_substitution_includes_template_arguments;
  out.third_flag = type.variadic;
  out.name = type.name;
  out.secondary_name = type.source_name;
  out.literal = type.discriminator;

  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    return false;
  case ABI_TYPE_BUILTIN:
  case ABI_TYPE_TEMPLATE_PARAMETER:
  case ABI_TYPE_NAMED:
    return true;
  case ABI_TYPE_POINTER:
  case ABI_TYPE_LVALUE_REFERENCE:
  case ABI_TYPE_RVALUE_REFERENCE:
  case ABI_TYPE_CONST:
  case ABI_TYPE_VOLATILE:
  case ABI_TYPE_VENDOR_QUALIFIED:
  case ABI_TYPE_BUILTIN_TYPE_TRANSFORM:
  case ABI_TYPE_PACK_EXPANSION:
  case ABI_TYPE_FUNCTION:
  case ABI_TYPE_MEMBER_POINTER:
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      DirectSubstitutionKey child_key;
      if(!direct_type_identity_key(ctx, type.child_types[i], child_key)) {
        return false;
      }
      out.children.push_back(child_key);
    }
    return true;
  case ABI_TYPE_ARRAY: {
    DirectSubstitutionKey bound_key;
    if(!direct_array_bound_identity_key(ctx, type.array_bound, bound_key)) {
      return false;
    }
    out.children.push_back(bound_key);
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      DirectSubstitutionKey child_key;
      if(!direct_type_identity_key(ctx, type.child_types[i], child_key)) {
        return false;
      }
      out.children.push_back(child_key);
    }
    return true;
  }
  case ABI_TYPE_CLASS_TEMPLATE:
  case ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE:
  case ABI_TYPE_STD_CLASS_TEMPLATE:
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE:
    out.unsigned_value = type.template_argument_references.size();
    for(size_t i = 0; i < type.template_argument_references.size(); ++i) {
      DirectSubstitutionKey arg_key;
      if(!direct_template_arg_key(
             ctx,
             direct_require_arg_ref(ctx, type.template_argument_references[i]),
             arg_key)) {
        return false;
      }
      out.children.push_back(arg_key);
    }
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      DirectSubstitutionKey owner_key;
      if(!direct_type_identity_key(ctx, type.child_types[i], owner_key)) {
        return false;
      }
      out.children.push_back(owner_key);
    }
    return true;
  case ABI_TYPE_MEMBER_TYPE:
    if(type.child_types.size() != 1) {
      return false;
    }
    {
      DirectSubstitutionKey owner_key;
      if(!direct_type_identity_key(ctx, type.child_types[0], owner_key)) {
        return false;
      }
      out.children.push_back(owner_key);
    }
    return true;
  case ABI_TYPE_DECLTYPE:
    {
      DirectSubstitutionKey expr_key;
      if(!direct_expression_key(
             ctx,
             direct_require_expr_ref(ctx, type.expression_reference),
             expr_key)) {
        return false;
      }
      out.children.push_back(expr_key);
    }
    return true;
  case ABI_TYPE_LAMBDA_CLOSURE:
  case ABI_TYPE_LOCAL_TYPE:
    {
      const DirectLocalContext & context =
          direct_require_context_ref(ctx, type.context_reference);
      DirectSubstitutionKey context_key;
      if(!direct_function_path_key(ctx, context.function, context_key)) {
        return false;
      }
      out.children.push_back(context_key);
      for(size_t i = 0; i < type.child_types.size(); ++i) {
        DirectSubstitutionKey signature_key;
        if(!direct_type_identity_key(ctx, type.child_types[i], signature_key)) {
          return false;
        }
        out.children.push_back(signature_key);
      }
    }
    return true;
  }
  return false;
}

bool direct_type_key(const DirectFactContext & ctx,
                     const AbiType & input,
                     DirectSubstitutionKey & out)
{
  AbiType builtin_storage;
  const AbiType & type = direct_resolve_type_ref(ctx, input, builtin_storage);
  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    return false;
  case ABI_TYPE_BUILTIN:
    return false;
  case ABI_TYPE_TEMPLATE_PARAMETER:
    if(type.substitutable_template_parameter) {
      return direct_type_identity_key(ctx, type, out);
    }
    return false;
  case ABI_TYPE_POINTER:
  case ABI_TYPE_LVALUE_REFERENCE:
  case ABI_TYPE_RVALUE_REFERENCE:
  case ABI_TYPE_CONST:
  case ABI_TYPE_VOLATILE:
  case ABI_TYPE_VENDOR_QUALIFIED:
  case ABI_TYPE_BUILTIN_TYPE_TRANSFORM:
  case ABI_TYPE_PACK_EXPANSION:
  case ABI_TYPE_ARRAY:
  case ABI_TYPE_MEMBER_POINTER:
  case ABI_TYPE_FUNCTION:
    return direct_type_identity_key(ctx, type, out);
  case ABI_TYPE_NAMED:
    return direct_type_identity_key(ctx, type, out);
  case ABI_TYPE_CLASS_TEMPLATE:
  case ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE:
  case ABI_TYPE_STD_CLASS_TEMPLATE:
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE:
  case ABI_TYPE_MEMBER_TYPE:
    return direct_type_identity_key(ctx, type, out);
  case ABI_TYPE_DECLTYPE:
    return direct_type_identity_key(ctx, type, out);
  case ABI_TYPE_LAMBDA_CLOSURE:
  case ABI_TYPE_LOCAL_TYPE:
    return direct_type_identity_key(ctx, type, out);
  }
  return false;
}

bool direct_expression_key(const DirectFactContext & ctx,
                           const AbiDependentExpr & expr,
                           DirectSubstitutionKey & out)
{
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_EXPRESSION;
  out.expression_kind = expr.kind;
  out.expression_operator = expr.expression_operator;
  out.index = expr.index;
  out.flag = expr.close_template_arguments;
  out.second_flag = expr.address_of;
  out.name = expr.member_name;
  out.secondary_name = expr.expression_operator_code;
  out.literal = expr.literal;
  if(!expr.first_reference.empty()) {
    DirectSubstitutionKey child_key;
    if(!direct_expression_key(ctx,
                              direct_require_expr_ref(ctx,
                                                      expr.first_reference),
                              child_key)) {
      return false;
    }
    out.children.push_back(child_key);
  }
  if(!expr.second_reference.empty()) {
    DirectSubstitutionKey child_key;
    if(!direct_expression_key(ctx,
                              direct_require_expr_ref(ctx,
                                                      expr.second_reference),
                              child_key)) {
      return false;
    }
    out.children.push_back(child_key);
  }
  if(!expr.third_reference.empty()) {
    DirectSubstitutionKey child_key;
    if(!direct_expression_key(ctx,
                              direct_require_expr_ref(ctx,
                                                      expr.third_reference),
                              child_key)) {
      return false;
    }
    out.children.push_back(child_key);
  }
  if(expr.kind == ABI_EXPR_MEMBER ||
     expr.kind == ABI_EXPR_SIZEOF_TYPE ||
     expr.kind == ABI_EXPR_CONVERSION) {
    DirectSubstitutionKey owner_key;
    if(!direct_type_identity_key(ctx, expr.owner_type, owner_key)) {
      return false;
    }
    out.children.push_back(owner_key);
  }
  if(expr.kind == ABI_EXPR_INTEGRAL_VALUE) {
    DirectSubstitutionKey value_type_key;
    if(!direct_type_identity_key(ctx, expr.value_type, value_type_key)) {
      return false;
    }
    out.children.push_back(value_type_key);
  }
  if(!expr.entity_reference.empty()) {
    DirectSubstitutionKey entity_key;
    if(!direct_entity_key(ctx,
                          direct_require_entity_ref(ctx,
                                                    expr.entity_reference),
                          entity_key)) {
      return false;
    }
    out.children.push_back(entity_key);
  }
  if(!expr.symbol.empty()) {
    DirectSubstitutionKey symbol_key;
    symbol_key.kind = DIRECT_SUBST_EXPRESSION;
    symbol_key.name = expr.symbol;
    symbol_key.flag = expr.address_of;
    out.children.push_back(symbol_key);
  }
  for(size_t i = 0; i < expr.argument_references.size(); ++i) {
    DirectSubstitutionKey child_key;
    if(!direct_expression_key(ctx,
                              direct_require_expr_ref(ctx,
                                                      expr.argument_references[i]),
                              child_key)) {
      return false;
    }
    out.children.push_back(child_key);
  }
  for(size_t i = 0; i < expr.template_argument_references.size(); ++i) {
    DirectSubstitutionKey arg_key;
    if(!direct_template_arg_key(
           ctx,
           direct_require_arg_ref(ctx, expr.template_argument_references[i]),
           arg_key)) {
      return false;
    }
    out.children.push_back(arg_key);
  }
  for(size_t i = 0; i < expr.type_arguments.size(); ++i) {
    DirectSubstitutionKey type_key;
    if(!direct_type_identity_key(ctx, expr.type_arguments[i], type_key)) {
      return false;
    }
    out.children.push_back(type_key);
  }
  return true;
}

bool direct_template_arg_key(const DirectFactContext & ctx,
                             const AbiTemplateArg & arg,
                             DirectSubstitutionKey & out)
{
  out = DirectSubstitutionKey();
  out.kind = DIRECT_SUBST_TEMPLATE_ARGUMENT;
  out.template_arg_kind = arg.kind;
  out.signed_value = arg.integer_value;
  out.flag = arg.address_of;
  out.second_flag = arg.member_is_function;
  out.third_flag = arg.member_function_variadic;
  out.name = arg.entity_reference;
  out.secondary_name = arg.symbol;
  out.index = arg.template_parameter_index;
  out.unsigned_value =
      (arg.member_function_const ? 1 : 0) |
      (arg.member_function_volatile ? 2 : 0) |
      (arg.member_function_lvalue_ref ? 4 : 0) |
      (arg.member_function_rvalue_ref ? 8 : 0);
  switch(arg.kind) {
  case ABI_TEMPLATE_ARG_TYPE: {
    DirectSubstitutionKey type_key;
    if(!direct_type_identity_key(ctx, arg.type, type_key)) {
      return false;
    }
    out.children.push_back(type_key);
    return true;
  }
  case ABI_TEMPLATE_ARG_INTEGRAL_VALUE: {
    DirectSubstitutionKey type_key;
    if(!direct_type_identity_key(ctx, arg.type, type_key)) {
      return false;
    }
    out.children.push_back(type_key);
    return true;
  }
  case ABI_TEMPLATE_ARG_DEPENDENT_INTEGRAL_VALUE: {
    DirectSubstitutionKey parameter_type_key;
    if(!direct_type_identity_key(ctx, arg.parameter_type, parameter_type_key)) {
      return false;
    }
    out.children.push_back(parameter_type_key);
    if(!(arg.type.kind == ABI_TYPE_REFERENCE && arg.type.reference.empty())) {
      DirectSubstitutionKey value_type_key;
      if(!direct_type_identity_key(ctx, arg.type, value_type_key)) {
        return false;
      }
      out.children.push_back(value_type_key);
    }
    return true;
  }
  case ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE:
    return true;
  case ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION: {
    const AbiDependentExpr & expression =
        direct_require_expr_ref(ctx, arg.expression_reference);
    DirectSubstitutionKey expr_key;
    if(!direct_expression_key(ctx, expression, expr_key)) {
      return false;
    }
    out.children.push_back(expr_key);
    return true;
  }
  case ABI_TEMPLATE_ARG_TEMPLATE_ENTITY:
  case ABI_TEMPLATE_ARG_TEMPLATE_PARAMETER_ENTITY:
  case ABI_TEMPLATE_ARG_EXTERNAL_ENTITY:
    return true;
  case ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY: {
    DirectSubstitutionKey owner_key;
    if(!direct_type_identity_key(ctx, arg.owner_type, owner_key)) {
      return false;
    }
    out.children.push_back(owner_key);
    out.literal = arg.member_name;
    out.flag = arg.address_of;
    out.second_flag = arg.member_is_function;
    out.third_flag = arg.member_function_variadic;
    for(size_t i = 0; i < arg.parameter_types.size(); ++i) {
      DirectSubstitutionKey param_key;
      if(!direct_type_identity_key(ctx, arg.parameter_types[i], param_key)) {
        return false;
      }
      out.children.push_back(param_key);
    }
    return true;
  }
  case ABI_TEMPLATE_ARG_ENTITY_ADDRESS:
  case ABI_TEMPLATE_ARG_ENTITY_REFERENCE: {
    DirectSubstitutionKey entity_key;
    if(!direct_entity_key(ctx,
                          direct_require_entity_ref(ctx,
                                                    arg.entity_reference),
                          entity_key)) {
      return false;
    }
    out.children.push_back(entity_key);
    return true;
  }
  case ABI_TEMPLATE_ARG_PACK: {
    for(size_t i = 0; i < arg.pack_argument_references.size(); ++i) {
      const AbiTemplateArg & item =
          direct_require_arg_ref(ctx, arg.pack_argument_references[i]);
      DirectSubstitutionKey item_key;
      if(!direct_template_arg_key(ctx, item, item_key)) {
        return false;
      }
      out.children.push_back(item_key);
    }
    return true;
  }
  }
  return false;
}

bool direct_emit_type(const DirectFactContext & ctx,
                      DirectEncoder & encoder,
                      const AbiType & type,
                      string & out);
bool direct_emit_template_arg(const DirectFactContext & ctx,
                              DirectEncoder & encoder,
                              const AbiTemplateArg & arg,
                              string & out);
bool direct_emit_expression_body(const DirectFactContext & ctx,
                                 DirectEncoder & encoder,
                                 const AbiDependentExpr & expr,
                                 string & out);
bool direct_emit_function_path_encoding(const DirectFactContext & ctx,
                                        DirectEncoder & encoder,
                                        const AbiFunctionPath & path,
                                        string & out);

bool direct_emit_prefix_component(DirectEncoder & encoder,
                                  const vector<string> & parts,
                                  size_t index,
                                  string & out)
{
  if(index == 0 && parts[index] == "std") {
    out += "St";
    return true;
  }
  const string qualified = direct_join_key_parts(
      vector<string>(parts.begin(), parts.begin() + index + 1));
  const DirectSubstitutionKey key = direct_name_key(qualified);
  if(direct_emit_substitution(encoder, key, out)) {
    return true;
  }
  if(!direct_emit_source_name(parts[index], out)) {
    return false;
  }
  direct_register_substitution(encoder, key);
  return true;
}

bool direct_emit_template_args(const DirectFactContext & ctx,
                               DirectEncoder & encoder,
                               const vector<string> & refs,
                               string & out)
{
  out += 'I';
  for(size_t i = 0; i < refs.size(); ++i) {
    if(!direct_emit_template_arg(ctx,
                                 encoder,
                                 direct_require_arg_ref(ctx, refs[i]),
                                 out)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

bool direct_emit_qualified_source_name_prefix(
    DirectEncoder & encoder,
    const string & qualified_name,
    const vector<string> & template_arg_refs,
    const DirectFactContext & ctx,
    string & out)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  const bool direct_std = parts.size() == 2 && parts[0] == "std";
  if(direct_std) {
    out += "St";
  } else {
    for(size_t i = 0; i + 1 < parts.size(); ++i) {
      if(!direct_emit_prefix_component(encoder, parts, i, out)) {
        return false;
      }
    }
  }
  if(!direct_emit_source_name(parts.back(), out)) {
    return false;
  }
  if(!template_arg_refs.empty()) {
    if(!direct_emit_template_args(ctx, encoder, template_arg_refs, out)) {
      return false;
    }
  }
  return true;
}

bool direct_emit_qualified_source_name(DirectEncoder & encoder,
                                       const string & qualified_name,
                                       const vector<string> & template_arg_refs,
                                       const DirectFactContext & ctx,
                                       string & out)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  const bool direct_std = parts.size() == 2 && parts[0] == "std";
  const bool nested = parts.size() > 1 && !direct_std;
  if(nested) {
    out += 'N';
  }
  if(!direct_emit_qualified_source_name_prefix(encoder,
                                               qualified_name,
                                               template_arg_refs,
                                               ctx,
                                               out)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

bool direct_emit_type_name_prefix(const DirectFactContext & ctx,
                                  DirectEncoder & encoder,
                                  const AbiType & input,
                                  string & out)
{
  DirectSubstitutionKey key;
  const bool has_key = direct_type_key(ctx, input, key);
  if(has_key && direct_emit_substitution(encoder, key, out)) {
    return true;
  }

  AbiType builtin_storage;
  const AbiType & type = direct_resolve_type_ref(ctx, input, builtin_storage);
  const size_t begin = out.size();
  switch(type.kind) {
  case ABI_TYPE_NAMED:
    if(!direct_emit_qualified_source_name_prefix(encoder,
                                                 type.name,
                                                 vector<string>(),
                                                 ctx,
                                                 out)) {
      out.resize(begin);
      return false;
    }
    break;
  case ABI_TYPE_CLASS_TEMPLATE:
  case ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE:
  case ABI_TYPE_STD_CLASS_TEMPLATE:
    if(type.kind == ABI_TYPE_STD_CLASS_TEMPLATE &&
       type.std_substitution != ABI_STD_SUBSTITUTION_NONE) {
      out += word_from_std_substitution(type.std_substitution);
      if(!type.std_substitution_includes_template_arguments &&
         !direct_emit_template_args(ctx,
                                    encoder,
                                    type.template_argument_references,
                                    out)) {
        out.resize(begin);
        return false;
      }
    } else if(type.kind == ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE) {
      out += 'T';
      if(type.template_parameter_index > 0) {
        out += to_string(type.template_parameter_index - 1);
      }
      out += '_';
      if(!direct_emit_template_args(ctx,
                                    encoder,
                                    type.template_argument_references,
                                    out)) {
        out.resize(begin);
        return false;
      }
    } else if(!direct_emit_qualified_source_name_prefix(
                  encoder,
                  type.name,
                  type.template_argument_references,
                  ctx,
                  out)) {
      out.resize(begin);
      return false;
    }
    break;
  case ABI_TYPE_MEMBER_TYPE:
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE:
    if(type.child_types.size() != 1 ||
       !direct_emit_type_name_prefix(ctx, encoder, type.child_types[0], out) ||
       !direct_emit_source_name(type.name, out)) {
      out.resize(begin);
      return false;
    }
    if(type.kind == ABI_TYPE_MEMBER_CLASS_TEMPLATE &&
       !direct_emit_template_args(ctx,
                                  encoder,
                                  type.template_argument_references,
                                  out)) {
      out.resize(begin);
      return false;
    }
    break;
  default:
    if(!direct_emit_type(ctx, encoder, input, out)) {
      out.resize(begin);
      return false;
    }
    return true;
  }

  if(has_key) {
    direct_register_substitution(encoder, key);
  }
  return true;
}

bool direct_emit_type_body(const DirectFactContext & ctx,
                           DirectEncoder & encoder,
                           const AbiType & input,
                           string & out)
{
  AbiType builtin_storage;
  const AbiType & type = direct_resolve_type_ref(ctx, input, builtin_storage);
  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    return false;
  case ABI_TYPE_BUILTIN: {
    string code;
    if(!direct_builtin_code_from_type(type, code)) {
      return false;
    }
    out += code;
    return true;
  }
  case ABI_TYPE_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;
  case ABI_TYPE_POINTER:
    if(type.child_types.size() != 1) { return false; }
    out += 'P';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_LVALUE_REFERENCE:
    if(type.child_types.size() != 1) { return false; }
    out += 'R';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_RVALUE_REFERENCE:
    if(type.child_types.size() != 1) { return false; }
    out += 'O';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_CONST:
    if(type.child_types.size() != 1) { return false; }
    out += 'K';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_VOLATILE:
    if(type.child_types.size() != 1) { return false; }
    out += 'V';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_VENDOR_QUALIFIED:
    if(type.child_types.size() != 1 || type.name.empty()) {
      return false;
    }
    out += 'U';
    if(!direct_emit_source_name(type.name, out)) { return false; }
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_BUILTIN_TYPE_TRANSFORM:
    if(type.child_types.size() != 1 || type.name.empty()) {
      return false;
    }
    out += 'u';
    if(!direct_emit_source_name(type.name, out)) { return false; }
    out += 'I';
    if(!direct_emit_type(ctx, encoder, type.child_types[0], out)) {
      return false;
    }
    out += 'E';
    return true;
  case ABI_TYPE_PACK_EXPANSION:
    if(type.child_types.size() != 1) { return false; }
    out += "Dp";
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_ARRAY:
    if(type.child_types.size() != 1) { return false; }
    out += 'A';
    if(type.array_bound.kind == ABI_ARRAY_BOUND_INTEGER) {
      out += to_string(type.array_bound.integer_value);
    } else if(type.array_bound.kind == ABI_ARRAY_BOUND_EXPRESSION) {
      if(!direct_emit_expression_body(
             ctx,
             encoder,
             direct_require_expr_ref(ctx, type.array_bound.expression_reference),
             out)) {
        return false;
      }
    } else {
      return false;
    }
    out += '_';
    return direct_emit_type(ctx, encoder, type.child_types[0], out);
  case ABI_TYPE_FUNCTION:
    if(type.child_types.empty()) { return false; }
    out += 'F';
    if(!direct_emit_type(ctx, encoder, type.child_types[0], out)) {
      return false;
    }
    if(type.child_types.size() == 1) {
      out += type.variadic ? 'z' : 'v';
    } else {
      for(size_t i = 1; i < type.child_types.size(); ++i) {
        if(!direct_emit_type(ctx, encoder, type.child_types[i], out)) {
          return false;
        }
      }
      if(type.variadic) {
        out += 'z';
      }
    }
    out += 'E';
    return true;
  case ABI_TYPE_MEMBER_POINTER:
    if(type.child_types.size() != 2) { return false; }
    out += 'M';
    return direct_emit_type(ctx, encoder, type.child_types[0], out) &&
           direct_emit_type(ctx, encoder, type.child_types[1], out);
  case ABI_TYPE_NAMED:
    return direct_emit_qualified_source_name(encoder,
                                             type.name,
                                             vector<string>(),
                                             ctx,
                                             out);
  case ABI_TYPE_CLASS_TEMPLATE:
  case ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE:
  case ABI_TYPE_STD_CLASS_TEMPLATE:
    if(type.kind == ABI_TYPE_STD_CLASS_TEMPLATE &&
       type.std_substitution != ABI_STD_SUBSTITUTION_NONE) {
      out += word_from_std_substitution(type.std_substitution);
      if(type.std_substitution_includes_template_arguments) {
        return true;
      }
      return direct_emit_template_args(ctx,
                                       encoder,
                                       type.template_argument_references,
                                       out);
    }
    if(type.kind == ABI_TYPE_TEMPLATE_PARAMETER_CLASS_TEMPLATE) {
      out += 'T';
      if(type.template_parameter_index > 0) {
        out += to_string(type.template_parameter_index - 1);
      }
      out += '_';
      return direct_emit_template_args(ctx,
                                       encoder,
                                       type.template_argument_references,
                                       out);
    }
    return direct_emit_qualified_source_name(encoder,
                                             type.name,
                                             type.template_argument_references,
                                             ctx,
                                             out);
  case ABI_TYPE_MEMBER_TYPE:
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE: {
    if(type.child_types.size() != 1) {
      return false;
    }
    out += 'N';
    if(!direct_emit_type_name_prefix(ctx, encoder, type.child_types[0], out)) {
      return false;
    }
    if(!direct_emit_source_name(type.name, out)) {
      return false;
    }
    if(type.kind == ABI_TYPE_MEMBER_CLASS_TEMPLATE &&
       !direct_emit_template_args(ctx,
                                  encoder,
                                  type.template_argument_references,
                                  out)) {
      return false;
    }
    out += 'E';
    return true;
  }
  case ABI_TYPE_DECLTYPE: {
    out += "DT";
    if(!direct_emit_expression_body(ctx,
                                    encoder,
                                    direct_require_expr_ref(ctx,
                                                            type.expression_reference),
                                    out)) {
      return false;
    }
    out += 'E';
    return true;
  }
  case ABI_TYPE_LAMBDA_CLOSURE:
  case ABI_TYPE_LOCAL_TYPE: {
    const DirectLocalContext & context =
        direct_require_context_ref(ctx, type.context_reference);
    out += 'Z';
    if(!direct_emit_function_path_encoding(ctx, encoder, context.function, out)) {
      return false;
    }
    out += 'E';
    if(type.kind == ABI_TYPE_LOCAL_TYPE) {
      return direct_emit_source_name(type.source_name, out);
    }
    out += "Ul";
    if(type.child_types.empty()) {
      out += 'v';
    } else {
      for(size_t i = 0; i < type.child_types.size(); ++i) {
        if(!direct_emit_type(ctx, encoder, type.child_types[i], out)) {
          return false;
        }
      }
    }
    out += 'E';
    out += type.discriminator;
    out += '_';
    return true;
  }
  }
  return false;
}

bool direct_emit_type(const DirectFactContext & ctx,
                      DirectEncoder & encoder,
                      const AbiType & type,
                      string & out)
{
  DirectSubstitutionKey key;
  const bool has_key = direct_type_key(ctx, type, key);
  if(has_key && direct_emit_substitution(encoder, key, out)) {
    return true;
  }
  const size_t begin = out.size();
  if(!direct_emit_type_body(ctx, encoder, type, out)) {
    out.resize(begin);
    return false;
  }
  if(has_key) {
    direct_register_substitution(encoder, key);
  }
  return true;
}

string direct_terminal_code(AbiFunctionTerminal terminal)
{
  switch(terminal) {
  case ABI_FUNCTION_TERMINAL_OPERATOR_CALL:
    return "cl";
  case ABI_FUNCTION_TERMINAL_OPERATOR_ASSIGN:
    return "aS";
  case ABI_FUNCTION_TERMINAL_OPERATOR_CODE:
  case ABI_FUNCTION_TERMINAL_CONVERSION:
    break;
  case ABI_FUNCTION_TERMINAL_CONSTRUCTOR_COMPLETE:
    return "C1";
  case ABI_FUNCTION_TERMINAL_CONSTRUCTOR_BASE:
    return "C2";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_COMPLETE:
    return "D1";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_BASE:
    return "D2";
  case ABI_FUNCTION_TERMINAL_DESTRUCTOR_DELETING:
    return "D0";
  case ABI_FUNCTION_TERMINAL_SOURCE_NAME:
    break;
  }
  return string();
}

bool direct_emit_function_name_path(const DirectFactContext & ctx,
                                    DirectEncoder & encoder,
                                    const string & qualified_name,
                                    const vector<string> & template_arg_refs,
                                    string & out)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  const bool direct_std = parts.size() == 2 && parts[0] == "std";
  const bool nested = parts.size() > 1 && !direct_std;
  if(direct_std) {
    out += "St";
  } else if(nested) {
    out += 'N';
    for(size_t i = 0; i + 1 < parts.size(); ++i) {
      if(!direct_emit_prefix_component(encoder, parts, i, out)) {
        return false;
      }
    }
  }
  if(!direct_emit_source_name(parts.back(), out)) {
    return false;
  }
  if(!template_arg_refs.empty()) {
    direct_register_substitution(
        encoder,
        direct_function_template_prefix_key(qualified_name));
    if(!direct_emit_template_args(ctx, encoder, template_arg_refs, out)) {
      return false;
    }
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

void direct_emit_abi_tags(const vector<string> & tags, string & out)
{
  vector<string> sorted = tags;
  sort(sorted.begin(), sorted.end());
  string previous;
  for(size_t i = 0; i < sorted.size(); ++i) {
    if(sorted[i].empty() || sorted[i] == previous) {
      continue;
    }
    out += 'B';
    out += to_string(sorted[i].size());
    out += sorted[i];
    previous = sorted[i];
  }
}

bool direct_emit_function_name_for_function(const DirectFactContext & ctx,
                                            DirectEncoder & encoder,
                                            const AbiFunction & function,
                                            string & out)
{
  const vector<string> parts = split_qualified_name(function.qualified_name);
  const bool direct_std = parts.size() == 2 && parts[0] == "std";
  const bool nested = parts.size() > 1 && !direct_std;
  if(direct_std) {
    out += "St";
  } else if(nested) {
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
    for(size_t i = 0; i + 1 < parts.size(); ++i) {
      if(!direct_emit_prefix_component(encoder, parts, i, out)) {
        return false;
      }
    }
  }

  if(function.terminal == ABI_FUNCTION_TERMINAL_OPERATOR_CODE) {
    if(function.terminal_operator_code.empty()) {
      return false;
    }
    out += function.terminal_operator_code;
  } else if(function.terminal == ABI_FUNCTION_TERMINAL_CONVERSION) {
    out += "cv";
    if(!direct_emit_type(ctx, encoder, function.conversion_type, out)) {
      return false;
    }
  } else if(!direct_emit_source_name(parts.back(), out)) {
    return false;
  }
  direct_emit_abi_tags(function.abi_tags, out);
  if(!function.template_argument_references.empty()) {
    direct_register_substitution(
        encoder,
        direct_function_template_prefix_key(function.qualified_name));
    if(!direct_emit_template_args(ctx,
                                  encoder,
                                  function.template_argument_references,
                                  out)) {
      return false;
    }
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

bool direct_emit_function_path_encoding(const DirectFactContext & ctx,
                                        DirectEncoder & encoder,
                                        const AbiFunctionPath & path,
                                        string & out)
{
  if(!direct_emit_function_name_path(ctx,
                                     encoder,
                                     path.qualified_name,
                                     path.template_argument_references,
                                     out)) {
    return false;
  }
  if(path.has_result_type &&
     !direct_emit_type(ctx, encoder, path.result_type, out)) {
    return false;
  }
  if(path.parameter_types.empty()) {
    out += path.variadic ? 'z' : 'v';
  } else {
    for(size_t i = 0; i < path.parameter_types.size(); ++i) {
      if(!direct_emit_type(ctx, encoder, path.parameter_types[i], out)) {
        return false;
      }
    }
    if(path.variadic) {
      out += 'z';
    }
  }
  return true;
}

string direct_function_path_symbol(const DirectFactContext & ctx,
                                   const AbiFunctionPath & path)
{
  DirectEncoder encoder;
  string out = "_Z";
  if(!direct_emit_function_path_encoding(ctx, encoder, path, out)) {
    throw logic_error("unable to encode ABI fact function entity");
  }
  return out;
}

string direct_variable_symbol(const string & qualified_name)
{
  DirectFactContext empty_ctx;
  DirectEncoder encoder;
  string out = "_Z";
  if(!direct_emit_function_name_path(empty_ctx,
                                     encoder,
                                     qualified_name,
                                     vector<string>(),
                                     out)) {
    throw logic_error("unable to encode ABI fact variable entity");
  }
  return out;
}

string direct_entity_symbol(const DirectFactContext & ctx,
                            const AbiEntity & entity)
{
  if(entity.kind == ABI_ENTITY_FUNCTION) {
    return direct_function_path_symbol(ctx, entity.function);
  }
  return direct_variable_symbol(entity.qualified_name);
}

bool direct_emit_expression_body(const DirectFactContext & ctx,
                                 DirectEncoder & encoder,
                                 const AbiDependentExpr & expr,
                                 string & out)
{
  switch(expr.kind) {
  case ABI_EXPR_TEMPLATE_PARAMETER:
    out += 'T';
    if(expr.index > 0) {
      out += to_string(expr.index - 1);
    }
    out += '_';
    return true;
  case ABI_EXPR_FUNCTION_PARAMETER:
    out += "fp";
    if(expr.index > 0) {
      out += to_string(expr.index - 1);
    }
    out += '_';
    return true;
  case ABI_EXPR_LITERAL:
    out += "Li";
    out += expr.literal;
    out += 'E';
    return true;
  case ABI_EXPR_INTEGRAL_VALUE:
    out += 'L';
    if(!direct_emit_type(ctx, encoder, expr.value_type, out)) {
      return false;
    }
    out += expr.literal;
    out += 'E';
    return true;
  case ABI_EXPR_OBJECT_MEMBER:
    if(expr.expression_operator_code.empty() || expr.member_name.empty()) {
      return false;
    }
    out += expr.expression_operator_code;
    if(!direct_emit_expression_body(ctx,
                                    encoder,
                                    direct_require_expr_ref(ctx,
                                                            expr.first_reference),
                                    out) ||
       !direct_emit_source_name(expr.member_name, out)) {
      return false;
    }
    if(!expr.template_argument_references.empty()) {
      if(!direct_emit_template_args(ctx,
                                    encoder,
                                    expr.template_argument_references,
                                    out)) {
        return false;
      }
    }
    return true;
  case ABI_EXPR_UNARY:
    out += expression_operator_code(expr);
    return direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.first_reference),
                                       out);
  case ABI_EXPR_BINARY:
    out += expression_operator_code(expr);
    return direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.first_reference),
                                       out) &&
           direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.second_reference),
                                       out);
  case ABI_EXPR_CONDITIONAL:
    out += "qu";
    return direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.first_reference),
                                       out) &&
           direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.second_reference),
                                       out) &&
           direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.third_reference),
                                       out);
  case ABI_EXPR_PACK_EXPANSION:
    out += "sp";
    return direct_emit_expression_body(ctx,
                                       encoder,
                                       direct_require_expr_ref(ctx,
                                                               expr.first_reference),
                                       out);
  case ABI_EXPR_CALL:
    out += "cl";
    if(!direct_emit_expression_body(ctx,
                                    encoder,
                                    direct_require_expr_ref(ctx,
                                                            expr.first_reference),
                                    out)) {
      return false;
    }
    for(size_t i = 0; i < expr.argument_references.size(); ++i) {
      if(!direct_emit_expression_body(ctx,
                                      encoder,
                                      direct_require_expr_ref(
                                          ctx,
                                          expr.argument_references[i]),
                                      out)) {
        return false;
      }
    }
    out += 'E';
    return true;
  case ABI_EXPR_CONVERSION: {
    const string op_code = expr.expression_operator_code.empty() ?
        string("cv") : expr.expression_operator_code;
    out += op_code;
    if(!direct_emit_type(ctx, encoder, expr.owner_type, out)) {
      return false;
    }
    if(op_code == "cv") {
      out += '_';
      for(size_t i = 0; i < expr.argument_references.size(); ++i) {
        if(!direct_emit_expression_body(ctx,
                                        encoder,
                                        direct_require_expr_ref(
                                            ctx,
                                            expr.argument_references[i]),
                                        out)) {
          return false;
        }
      }
      out += 'E';
      return true;
    }
    return expr.argument_references.size() == 1 &&
           direct_emit_expression_body(
               ctx,
               encoder,
               direct_require_expr_ref(ctx, expr.argument_references[0]),
               out);
  }
  case ABI_EXPR_TEMPLATE_ID:
    if(expr.member_name.empty() ||
       !direct_emit_source_name(expr.member_name, out)) {
      return false;
    }
    return direct_emit_template_args(ctx,
                                     encoder,
                                     expr.template_argument_references,
                                     out);
  case ABI_EXPR_TYPE_TRAIT:
    if(expr.member_name.empty()) {
      return false;
    }
    out += 'u';
    if(!direct_emit_source_name(expr.member_name, out)) {
      return false;
    }
    for(size_t i = 0; i < expr.type_arguments.size(); ++i) {
      if(!direct_emit_type(ctx, encoder, expr.type_arguments[i], out)) {
        return false;
      }
    }
    out += 'E';
    return true;
  case ABI_EXPR_SIZEOF_TYPE:
    out += "st";
    return direct_emit_type(ctx, encoder, expr.owner_type, out);
  case ABI_EXPR_MEMBER:
    out += "sr";
    if(!direct_emit_type(ctx, encoder, expr.owner_type, out)) {
      return false;
    }
    if(expr.close_template_arguments) {
      out += 'E';
    }
    if(!direct_emit_source_name(expr.member_name, out)) {
      return false;
    }
    if(!expr.template_argument_references.empty()) {
      return direct_emit_template_args(ctx,
                                       encoder,
                                       expr.template_argument_references,
                                       out);
    }
    return true;
  case ABI_EXPR_EXTERNAL_ENTITY:
    if(expr.symbol.empty()) {
      return false;
    }
    out += expr.address_of ? "adL" : "L";
    out += expr.symbol;
    out += 'E';
    if(expr.address_of) {
      out += 'E';
    }
    return true;
  case ABI_EXPR_ENTITY_ADDRESS:
  case ABI_EXPR_ENTITY_REFERENCE: {
    const AbiEntity & entity = direct_require_entity_ref(ctx,
                                                         expr.entity_reference);
    out += expr.kind == ABI_EXPR_ENTITY_ADDRESS ? "adL" : "L";
    out += direct_entity_symbol(ctx, entity);
    out += 'E';
    if(expr.kind == ABI_EXPR_ENTITY_ADDRESS) {
      out += 'E';
    }
    return true;
  }
  }
  return false;
}

bool direct_emit_template_entity_name(const DirectFactContext & ctx,
                                      DirectEncoder & encoder,
                                      const string & qualified_name,
                                      string & out)
{
  return direct_emit_qualified_source_name(encoder,
                                           qualified_name,
                                           vector<string>(),
                                           ctx,
                                           out);
}

bool direct_emit_external_member_entity_symbol(
    const DirectFactContext & ctx,
    DirectEncoder & encoder,
    const AbiTemplateArg & arg,
    string & out)
{
  if(arg.member_name.empty()) {
    return false;
  }
  out += "_ZN";
  if(arg.member_function_const) {
    out += 'K';
  }
  if(arg.member_function_volatile) {
    out += 'V';
  }
  if(arg.member_function_lvalue_ref) {
    out += 'R';
  }
  if(arg.member_function_rvalue_ref) {
    out += 'O';
  }
  if(!direct_emit_type_name_prefix(ctx, encoder, arg.owner_type, out) ||
     !direct_emit_source_name(arg.member_name, out)) {
    return false;
  }
  out += 'E';
  if(arg.member_is_function) {
    if(arg.parameter_types.empty()) {
      out += arg.member_function_variadic ? 'z' : 'v';
    } else {
      for(size_t i = 0; i < arg.parameter_types.size(); ++i) {
        if(!direct_emit_type(ctx, encoder, arg.parameter_types[i], out)) {
          return false;
        }
      }
      if(arg.member_function_variadic) {
        out += 'z';
      }
    }
  }
  return true;
}

bool direct_emit_external_template_arg(const DirectFactContext & ctx,
                                       DirectEncoder & encoder,
                                       const AbiTemplateArg & arg,
                                       string & out)
{
  if(arg.symbol.empty()) {
    return false;
  }
  if(arg.address_of) {
    out += "Xad";
  }
  out += 'L';
  if(arg.kind == ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY) {
    if(!direct_emit_external_member_entity_symbol(ctx, encoder, arg, out)) {
      out += arg.symbol;
    }
  } else {
    out += arg.symbol;
  }
  out += 'E';
  if(arg.address_of) {
    out += 'E';
  }
  return true;
}

bool direct_emit_template_arg(const DirectFactContext & ctx,
                              DirectEncoder & encoder,
                              const AbiTemplateArg & arg,
                              string & out)
{
  switch(arg.kind) {
  case ABI_TEMPLATE_ARG_TYPE:
    return direct_emit_type(ctx, encoder, arg.type, out);
  case ABI_TEMPLATE_ARG_INTEGRAL_VALUE:
    out += 'L';
    if(!direct_emit_type(ctx, encoder, arg.type, out)) {
      return false;
    }
    out += to_string(arg.integer_value);
    out += 'E';
    return true;
  case ABI_TEMPLATE_ARG_DEPENDENT_INTEGRAL_VALUE:
    out += "Tn";
    if(!direct_emit_type(ctx, encoder, arg.parameter_type, out)) {
      return false;
    }
    out += 'L';
    if(arg.type.kind == ABI_TYPE_REFERENCE && arg.type.reference.empty()) {
      out += 'i';
    } else {
      if(!direct_emit_type(ctx, encoder, arg.type, out)) {
        return false;
      }
    }
    out += to_string(arg.integer_value);
    out += 'E';
    return true;
  case ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE:
    out += "Li";
    out += to_string(arg.integer_value);
    out += 'E';
    return true;
  case ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION: {
    out += 'X';
    if(!direct_emit_expression_body(ctx,
                                    encoder,
                                    direct_require_expr_ref(ctx,
                                                            arg.expression_reference),
                                    out)) {
      return false;
    }
    out += 'E';
    return true;
  }
  case ABI_TEMPLATE_ARG_TEMPLATE_ENTITY:
    return direct_emit_template_entity_name(ctx,
                                            encoder,
                                            arg.entity_reference,
                                            out);
  case ABI_TEMPLATE_ARG_TEMPLATE_PARAMETER_ENTITY:
    out += 'T';
    if(arg.template_parameter_index > 0) {
      out += to_string(arg.template_parameter_index - 1);
    }
    out += '_';
    return true;
  case ABI_TEMPLATE_ARG_EXTERNAL_ENTITY:
  case ABI_TEMPLATE_ARG_MEMBER_EXTERNAL_ENTITY:
    return direct_emit_external_template_arg(ctx, encoder, arg, out);
  case ABI_TEMPLATE_ARG_ENTITY_ADDRESS:
  case ABI_TEMPLATE_ARG_ENTITY_REFERENCE: {
    const AbiEntity & entity = direct_require_entity_ref(ctx,
                                                         arg.entity_reference);
    out += arg.kind == ABI_TEMPLATE_ARG_ENTITY_ADDRESS ? "XadL" : "L";
    out += direct_entity_symbol(ctx, entity);
    out += 'E';
    if(arg.kind == ABI_TEMPLATE_ARG_ENTITY_ADDRESS) {
      out += 'E';
    }
    return true;
  }
  case ABI_TEMPLATE_ARG_PACK:
    out += 'J';
    for(size_t i = 0; i < arg.pack_argument_references.size(); ++i) {
      if(!direct_emit_template_arg(
             ctx,
             encoder,
             direct_require_arg_ref(ctx, arg.pack_argument_references[i]),
             out)) {
        return false;
      }
    }
    out += 'E';
    return true;
  }
  return false;
}

void direct_apply_fact(DirectFactContext & ctx, const AbiFact & fact)
{
  switch(fact.kind) {
  case ABI_FACT_TYPE:
    ctx.types[fact.id] = fact.type;
    return;
  case ABI_FACT_TEMPLATE_ARGUMENT:
    ctx.args[fact.id] = fact.template_argument;
    return;
  case ABI_FACT_EXPRESSION:
    ctx.exprs[fact.id] = fact.expression;
    return;
  case ABI_FACT_LOCAL_CONTEXT: {
    DirectLocalContext context;
    context.function = fact.context_function;
    ctx.contexts[fact.id] = context;
    return;
  }
  case ABI_FACT_ENTITY:
    ctx.entities[fact.id] = fact.entity;
    return;
  }
}

string direct_emit_function_symbol_body(const DirectFactContext & ctx,
                                        const AbiFunction & function)
{
  DirectEncoder encoder;
  string out;
  if(function.form == ABI_FUNCTION_PATH) {
    if(!direct_emit_function_name_for_function(ctx, encoder, function, out)) {
      throw logic_error("unable to encode ABI fact function");
    }
    if(function.has_result_type &&
       !direct_emit_type(ctx, encoder, function.result_type, out)) {
      throw logic_error("unable to encode ABI fact result type");
    }
    if(function.parameter_types.empty()) {
      out += function.variadic ? 'z' : 'v';
    } else {
      for(size_t i = 0; i < function.parameter_types.size(); ++i) {
        if(!direct_emit_type(ctx, encoder, function.parameter_types[i], out)) {
          throw logic_error("unable to encode ABI fact parameter type");
        }
      }
      if(function.variadic) {
        out += 'z';
      }
    }
    return out;
  }

  const DirectLocalContext & context =
      direct_require_context_ref(ctx, function.context_reference);
  out += 'Z';
  if(!direct_emit_function_path_encoding(ctx, encoder, context.function, out)) {
    throw logic_error("unable to encode ABI fact function context");
  }
  out += 'E';
  if(function.form == ABI_FUNCTION_LAMBDA) {
    out += 'N';
    out += "Ul";
    if(function.lambda_signature_parameter_types.empty()) {
      out += 'v';
    } else {
      for(size_t i = 0; i < function.lambda_signature_parameter_types.size(); ++i) {
        if(!direct_emit_type(ctx,
                             encoder,
                             function.lambda_signature_parameter_types[i],
                             out)) {
          throw logic_error("unable to encode ABI fact lambda signature");
        }
      }
    }
    out += 'E';
    out += function.discriminator;
    out += '_';
    out += direct_terminal_code(function.terminal);
    out += 'E';
  } else {
    out += 'N';
    if(!direct_emit_source_name(function.source_name, out)) {
      throw logic_error("unable to encode ABI fact local function name");
    }
    out += direct_terminal_code(function.terminal);
    out += 'E';
  }
  if(function.has_result_type &&
     !direct_emit_type(ctx, encoder, function.result_type, out)) {
    throw logic_error("unable to encode ABI fact result type");
  }
  if(function.parameter_types.empty()) {
    out += function.variadic ? 'z' : 'v';
  } else {
    for(size_t i = 0; i < function.parameter_types.size(); ++i) {
      if(!direct_emit_type(ctx, encoder, function.parameter_types[i], out)) {
        throw logic_error("unable to encode ABI fact parameter type");
      }
    }
    if(function.variadic) {
      out += 'z';
    }
  }
  return out;
}

string direct_emit_function_symbol(const DirectFactContext & ctx,
                                   const AbiFunction & function)
{
  return "_Z" + direct_emit_function_symbol_body(ctx, function);
}

string direct_emit_type_encoding(const DirectFactContext & ctx,
                                 const AbiType & type)
{
  DirectEncoder encoder;
  string out;
  if(!direct_emit_type(ctx, encoder, type, out)) {
    throw logic_error("ABI model mangler failed to encode fact type");
  }
  return out;
}

string direct_emit_special_type_symbol(const DirectFactContext & ctx,
                                       SpecialTypeSymbolKind kind,
                                       const AbiType & type)
{
  string out;
  if(!emit_special_type_symbol_from_encoding(kind,
                                             direct_emit_type_encoding(ctx, type),
                                             out)) {
    throw logic_error("unable to encode ABI fact special type symbol");
  }
  return out;
}

string mangle_case(const AbiFactCase & fact_case)
{
  if(fact_case.facts.empty() && fact_case.target.kind == ABI_MANGLE_NONE) {
    throw logic_error("empty ABI fact case");
  }
  DirectFactContext ctx;
  for(size_t i = 0; i < fact_case.facts.size(); ++i) {
    direct_apply_fact(ctx, fact_case.facts[i]);
  }

  switch(fact_case.target.kind) {
  case ABI_MANGLE_TYPE:
    return direct_emit_type_encoding(ctx, fact_case.target.type);
  case ABI_MANGLE_FUNCTION:
    if(fact_case.target.function.c_linkage) {
      return unqualified_name(fact_case.target.function.qualified_name);
    }
    return direct_emit_function_symbol(ctx, fact_case.target.function);
  case ABI_MANGLE_VARIABLE:
    if(fact_case.target.c_linkage) {
      return unqualified_name(fact_case.target.qualified_name);
    }
    return direct_variable_symbol(fact_case.target.qualified_name);
  case ABI_MANGLE_TYPEINFO:
    return direct_emit_special_type_symbol(ctx,
                                           SPECIAL_TYPEINFO,
                                           fact_case.target.type);
  case ABI_MANGLE_VTABLE:
    return direct_emit_special_type_symbol(ctx,
                                           SPECIAL_VTABLE,
                                           fact_case.target.type);
  case ABI_MANGLE_VTT:
    return direct_emit_special_type_symbol(ctx,
                                           SPECIAL_VTT,
                                           fact_case.target.type);
  case ABI_MANGLE_CONSTRUCTION_VTABLE: {
    string out;
    if(!emit_construction_vtable_symbol_from_encodings(
           direct_emit_type_encoding(ctx, fact_case.target.type),
           fact_case.target.base_offset,
           direct_emit_type_encoding(ctx, fact_case.target.base_type),
           out)) {
      throw logic_error("unable to encode ABI fact construction vtable symbol");
    }
    return out;
  }
  case ABI_MANGLE_THREAD_LOCAL_WRAPPER: {
    string out;
    DirectFactContext empty_ctx;
    DirectEncoder encoder;
    string name_encoding;
    if(fact_case.target.qualified_name.empty() ||
       !direct_emit_function_name_path(empty_ctx,
                                       encoder,
                                       fact_case.target.qualified_name,
                                       vector<string>(),
                                       name_encoding) ||
       !emit_thread_local_wrapper_symbol_from_encoding(
           name_encoding,
           out)) {
      throw logic_error("unable to encode ABI fact TLS wrapper symbol");
    }
    return out;
  }
  case ABI_MANGLE_THUNK: {
    string out;
    if(!emit_virtual_override_thunk_symbol_from_encoding(
           direct_emit_function_symbol_body(ctx, fact_case.target.function),
           fact_case.target.this_adjust,
           fact_case.target.has_result_adjust,
           fact_case.target.result_adjust,
           out)) {
      throw logic_error("unable to encode ABI fact thunk symbol");
    }
    return out;
  }
  case ABI_MANGLE_VIRTUAL_BASE_THUNK: {
    string out;
    if(!emit_virtual_base_override_thunk_symbol_from_encoding(
           direct_emit_function_symbol_body(ctx, fact_case.target.function),
           fact_case.target.vcall_offset,
           out)) {
      throw logic_error("unable to encode ABI fact virtual-base thunk symbol");
    }
    return out;
  }
  case ABI_MANGLE_NONE:
    break;
  }
  throw logic_error("ABI fact case must test exactly one mangle");
}

void flush_case(AbiFactFile & file, AbiFactCase & pending)
{
  if(pending.label.empty() &&
     pending.facts.empty() &&
     pending.target.kind == ABI_MANGLE_NONE) {
    return;
  }
  if(pending.facts.empty() && pending.target.kind == ABI_MANGLE_NONE) {
    throw logic_error("case '" + pending.label + "' has no ABI fact");
  }
  file.cases.push_back(pending);
  pending = AbiFactCase();
}

AbiFactFile parse_fact_stream(istream & in, const string & input_name)
{
  AbiFactFile file;
  AbiFactCase pending;
  string line;
  size_t line_number = 0;
  while(getline(in, line)) {
    ++line_number;
    const size_t comment = line.find('#');
    if(comment != string::npos) {
      line = line.substr(0, comment);
    }
    line = trim(line);
    if(line.empty()) {
      continue;
    }
    vector<string> words = split_words(line);
    if(words.empty()) {
      continue;
    }
    if(words[0] == "case") {
      if(words.size() != 2) {
        throw logic_error(input_name + ":" + to_string(line_number) +
                          ": case requires exactly one label");
      }
      flush_case(file, pending);
      pending.label = words[1];
      continue;
    }
    apply_fact_words(pending, words);
  }
  flush_case(file, pending);
  return file;
}

}  // namespace

AbiFactFile parse_fact_text(const string & text)
{
  istringstream in(text);
  return parse_fact_stream(in, "<string>");
}

string serialize_fact_file(const AbiFactFile & file)
{
  ostringstream out;
  for(size_t i = 0; i < file.cases.size(); ++i) {
    const AbiFactCase & fact_case = file.cases[i];
    if(!fact_case.label.empty()) {
      out << "case " << fact_case.label << "\n";
    }
    for(size_t j = 0; j < fact_case.facts.size(); ++j) {
      out << join_words(fact_words(fact_case.facts[j])) << "\n";
    }
    const vector<vector<string> > targets = target_lines(fact_case.target);
    for(size_t j = 0; j < targets.size(); ++j) {
      out << join_words(targets[j]) << "\n";
    }
    if(i + 1 != file.cases.size()) {
      out << "\n";
    }
  }
  return out.str();
}

string mangle_fact_file(const AbiFactFile & file)
{
  ostringstream out;
  for(size_t i = 0; i < file.cases.size(); ++i) {
    const AbiFactCase & fact_case = file.cases[i];
    out << mangle_case(fact_case) << "\n";
  }
  return out.str();
}

string mangle_fact_files(const vector<string> & input_paths)
{
  if(input_paths.empty()) {
    throw logic_error("abimangle requires at least one input file");
  }
  ostringstream out;
  for(size_t i = 0; i < input_paths.size(); ++i) {
    ifstream in(input_paths[i].c_str());
    if(!in) {
      throw logic_error("unable to open input file '" + input_paths[i] + "'");
    }
    AbiFactFile file = parse_fact_stream(in, input_paths[i]);
    out << mangle_fact_file(parse_fact_text(serialize_fact_file(file)));
  }
  return out.str();
}

}  // namespace abi_mangle
