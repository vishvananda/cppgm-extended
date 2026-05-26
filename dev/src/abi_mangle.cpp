#include "abi_mangle.h"

#include "cpp_decl_model.h"
#include "symbol_linkage.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace cpp_decl;

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

QualifiedName parse_qualified_name(const string & text)
{
  const vector<string> parts = split_qualified_name(text);
  QualifiedName out;
  out.rooted = false;
  out.name = parts.back();
  out.qualifiers.assign(parts.begin(), parts.end() - 1);
  return out;
}

string unqualified_name(const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  return parts.back();
}

TypePtr parse_type(const string & type_text);

map<string, EFundamentalType> fundamental_types()
{
  map<string, EFundamentalType> out;
  out["void"] = FT_VOID;
  out["bool"] = FT_BOOL;
  out["char"] = FT_CHAR;
  out["schar"] = FT_SIGNED_CHAR;
  out["uchar"] = FT_UNSIGNED_CHAR;
  out["short"] = FT_SHORT_INT;
  out["ushort"] = FT_UNSIGNED_SHORT_INT;
  out["int"] = FT_INT;
  out["uint"] = FT_UNSIGNED_INT;
  out["long"] = FT_LONG_INT;
  out["ulong"] = FT_UNSIGNED_LONG_INT;
  out["longlong"] = FT_LONG_LONG_INT;
  out["ulonglong"] = FT_UNSIGNED_LONG_LONG_INT;
  out["int128"] = FT_INT128;
  out["uint128"] = FT_UINT128;
  out["wchar"] = FT_WCHAR_T;
  out["char16"] = FT_CHAR16_T;
  out["char32"] = FT_CHAR32_T;
  out["float"] = FT_FLOAT;
  out["double"] = FT_DOUBLE;
  out["longdouble"] = FT_LONG_DOUBLE;
  out["nullptr"] = FT_NULLPTR_T;
  return out;
}

unsigned long long parse_decimal_bound(const string & text)
{
  if(text.empty()) {
    throw logic_error("array bound must not be empty");
  }
  for(size_t i = 0; i < text.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(text[i]))) {
      throw logic_error("array bound must be decimal in '" + text + "'");
    }
  }
  return stoull(text);
}

TypePtr parse_array_type(const string & rest)
{
  const size_t pos = rest.find(':');
  if(pos == string::npos || pos == 0) {
    throw logic_error("array type requires array:<bound>:<element>");
  }
  const string bound = rest.substr(0, pos);
  return make_array(parse_type(rest.substr(pos + 1)),
                    true,
                    parse_decimal_bound(bound),
                    bound);
}

TypePtr parse_member_pointer_type(const string & rest)
{
  const size_t pos = rest.rfind(':');
  if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
    throw logic_error("member pointer type requires memberptr:<owner>:<member-type>");
  }
  const string owner = rest.substr(0, pos);
  const string member_type = rest.substr(pos + 1);
  return make_member_pointer(make_named(owner, owner, true), parse_type(member_type));
}

TypePtr parse_type(const string & type_text)
{
  const map<string, EFundamentalType> fundamentals = fundamental_types();
  const map<string, EFundamentalType>::const_iterator found = fundamentals.find(type_text);
  if(found != fundamentals.end()) {
    return make_fundamental(found->second);
  }
  if(starts_with(type_text, "ptr:")) {
    return make_pointer(parse_type(type_text.substr(4)));
  }
  if(starts_with(type_text, "ref:")) {
    return make_lvalue_reference_raw(parse_type(type_text.substr(4)));
  }
  if(starts_with(type_text, "rref:")) {
    return make_rvalue_reference_raw(parse_type(type_text.substr(5)));
  }
  if(starts_with(type_text, "const:")) {
    return apply_cv(parse_type(type_text.substr(6)), true, false);
  }
  if(starts_with(type_text, "volatile:")) {
    return apply_cv(parse_type(type_text.substr(9)), false, true);
  }
  if(starts_with(type_text, "array:")) {
    return parse_array_type(type_text.substr(6));
  }
  if(starts_with(type_text, "memberptr:")) {
    return parse_member_pointer_type(type_text.substr(10));
  }
  if(starts_with(type_text, "named:")) {
    const string name = type_text.substr(6);
    return make_named(name, name, true);
  }
  throw logic_error("unknown ABI fact type '" + type_text + "'");
}

string mangle_type(const TypePtr & type)
{
  string out;
  if(!symbol_linkage::mangle_itanium_type_encoding(type, out)) {
    throw logic_error("shared mangler failed to encode ABI fact type");
  }
  return out;
}

string mangle_function(const string & qualified_name,
                       const vector<string> & param_types,
                       bool c_linkage)
{
  if(c_linkage) {
    return symbol_linkage::make_c_function_symbol_identity(
        unqualified_name(qualified_name)).object_symbol;
  }

  vector<TypePtr> params;
  params.reserve(param_types.size());
  for(size_t i = 0; i < param_types.size(); ++i) {
    params.push_back(parse_type(param_types[i]));
  }

  const QualifiedName parsed = parse_qualified_name(qualified_name);
  const TypePtr function_type = make_function(make_fundamental(FT_VOID), params, false);
  const symbol_linkage::SymbolIdentity identity =
      symbol_linkage::make_function_symbol_identity(parsed,
                                                    parsed.name,
                                                    false,
                                                    function_type);
  if(identity.object_symbol.empty()) {
    throw logic_error("shared mangler failed to encode function '" + qualified_name + "'");
  }
  return identity.object_symbol;
}

string mangle_variable(const string & qualified_name, bool c_linkage)
{
  if(c_linkage) {
    return unqualified_name(qualified_name);
  }
  string encoding;
  if(!symbol_linkage::mangle_itanium_name_encoding(parse_qualified_name(qualified_name),
                                                   encoding)) {
    throw logic_error("shared mangler failed to encode variable '" + qualified_name + "'");
  }
  return "_Z" + encoding;
}

string mangle_case(const vector<string> & words)
{
  if(words.empty()) {
    throw logic_error("empty ABI fact case");
  }
  const string & kind = words[0];
  if(kind == "function" || kind == "c-function") {
    if(words.size() < 2) {
      throw logic_error(kind + " requires a qualified name");
    }
    vector<string> params(words.begin() + 2, words.end());
    return mangle_function(words[1], params, kind == "c-function");
  }
  if(kind == "variable" || kind == "c-variable") {
    if(words.size() != 2) {
      throw logic_error(kind + " requires exactly one qualified name");
    }
    return mangle_variable(words[1], kind == "c-variable");
  }
  if(kind == "type") {
    if(words.size() != 2) {
      throw logic_error("type requires exactly one type operand");
    }
    return mangle_type(parse_type(words[1]));
  }
  if(kind == "typeinfo") {
    if(words.size() != 2) {
      throw logic_error("typeinfo requires exactly one type operand");
    }
    return symbol_linkage::typeinfo_symbol_for_type(parse_type(words[1]));
  }
  if(kind == "vtable") {
    if(words.size() != 2) {
      throw logic_error("vtable requires exactly one type operand");
    }
    return symbol_linkage::vtable_object_symbol_for_type(parse_type(words[1]));
  }
  throw logic_error("unknown ABI fact kind '" + kind + "'");
}

struct PendingCase
{
  string label;
  vector<string> words;
};

void flush_case(ostream & out, PendingCase & pending)
{
  if(pending.label.empty()) {
    return;
  }
  if(pending.words.empty()) {
    throw logic_error("case '" + pending.label + "' has no ABI fact");
  }
  out << "case " << pending.label << "\n";
  out << "mangled " << mangle_case(pending.words) << "\n";
  pending = PendingCase();
}

void read_fact_file(const string & input_path, ostream & out)
{
  ifstream in(input_path.c_str());
  if(!in) {
    throw logic_error("unable to open input file '" + input_path + "'");
  }

  PendingCase pending;
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
        throw logic_error(input_path + ":" + to_string(line_number) +
                          ": case requires exactly one label");
      }
      flush_case(out, pending);
      pending.label = words[1];
      continue;
    }
    if(pending.label.empty()) {
      throw logic_error(input_path + ":" + to_string(line_number) +
                        ": ABI fact appears before case label");
    }
    if(!pending.words.empty()) {
      throw logic_error(input_path + ":" + to_string(line_number) +
                        ": case already has an ABI fact");
    }
    pending.words = words;
  }
  flush_case(out, pending);
}

}  // namespace

string mangle_fact_files(const vector<string> & input_paths)
{
  if(input_paths.empty()) {
    throw logic_error("abimangle requires at least one input file");
  }
  ostringstream out;
  for(size_t i = 0; i < input_paths.size(); ++i) {
    read_fact_file(input_paths[i], out);
  }
  return out.str();
}

}  // namespace abi_mangle
