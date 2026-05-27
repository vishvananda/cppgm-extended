#include "abi_mangle.h"

#include "cpp_decl_model.h"
#include "itanium_mangle_ir.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace cpp_decl;
namespace abi_ir = itanium_mangle_ir;

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

struct SimpleIrSubstitutionSink : public abi_ir::SubstitutionSink
{
  bool emit_substitution(const abi_ir::SubstitutionKey & key,
                         string & out) override
  {
    for(size_t i = 0; i < slots.size(); ++i) {
      if(slots[i] == key) {
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

  void register_substitution(const abi_ir::SubstitutionKey & key) override
  {
    if(key.empty()) {
      return;
    }
    for(size_t i = 0; i < slots.size(); ++i) {
      if(slots[i] == key) {
        return;
      }
    }
    slots.push_back(key);
  }

  bool emit_dependent_parameter_type(const abi_ir::Type & type,
                                     string & out) override
  {
    return abi_ir::emit_type(type, out, this);
  }

  vector<abi_ir::SubstitutionKey> slots;
};

vector<abi_ir::SubstitutionSlot> substitution_slots_from_sink(
    const SimpleIrSubstitutionSink & sink)
{
  vector<abi_ir::SubstitutionSlot> out;
  out.reserve(sink.slots.size());
  for(size_t i = 0; i < sink.slots.size(); ++i) {
    out.push_back(abi_ir::SubstitutionSlot::typed(sink.slots[i]));
  }
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

string join_qualified_prefix(const vector<string> & parts, size_t count)
{
  string out;
  for(size_t i = 0; i < count; ++i) {
    if(i != 0) {
      out += "::";
    }
    out += parts[i];
  }
  return out;
}

vector<abi_ir::Type::NameComponent> make_ir_prefix_components(
    const vector<string> & parts)
{
  vector<abi_ir::Type::NameComponent> out;
  for(size_t i = 0; i + 1 < parts.size(); ++i) {
    if(i == 0 && parts[i] == "std") {
      out.push_back(abi_ir::Type::NameComponent::std_namespace());
      continue;
    }
    out.push_back(abi_ir::Type::NameComponent::source(
        parts[i],
        join_qualified_prefix(parts, i + 1)));
  }
  return out;
}

vector<abi_ir::FunctionNameComponent> make_ir_function_components(
    const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  vector<abi_ir::FunctionNameComponent> out;
  for(size_t i = 0; i < parts.size(); ++i) {
    if(i == 0 && parts[i] == "std") {
      out.push_back(abi_ir::FunctionNameComponent::std_namespace());
      continue;
    }
    out.push_back(abi_ir::FunctionNameComponent::source(
        parts[i],
        join_qualified_prefix(parts, i + 1)));
  }
  return out;
}

map<string, EFundamentalType> fundamental_types();

abi_ir::Type make_ir_named_type(const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  return abi_ir::Type::named_type(make_ir_prefix_components(parts),
                                  parts.back(),
                                  join_qualified_prefix(parts, parts.size()));
}

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

struct FactTemplateArgument
{
  abi_ir::Type::ClassTemplateArgument class_arg;
  abi_ir::TemplateArgument function_arg;
};

struct FactLocalContext
{
  string fragment;
  vector<abi_ir::SubstitutionSlot> substitution_slots;
  shared_ptr<abi_ir::FunctionEncoding> function;
};

struct FactEntity
{
  string symbol;
};

struct FactContext
{
  map<string, abi_ir::Type> types;
  map<string, FactTemplateArgument> args;
  map<string, abi_ir::DependentExpression> exprs;
  map<string, FactLocalContext> contexts;
  map<string, FactEntity> entities;
  abi_ir::FunctionEncoding function;
  bool has_function = false;
  bool has_type = false;
  abi_ir::Type result_type;
};

bool boolean_word(const string & word)
{
  return word == "1" || word == "yes" || word == "true";
}

const FactTemplateArgument & require_fact_arg_ref(const FactContext & ctx,
                                                  const string & id)
{
  map<string, FactTemplateArgument>::const_iterator found = ctx.args.find(id);
  if(found == ctx.args.end()) {
    throw logic_error("unknown ABI fact template argument reference '" + id + "'");
  }
  return found->second;
}

const abi_ir::DependentExpression & require_fact_expr_ref(const FactContext & ctx,
                                                          const string & id)
{
  map<string, abi_ir::DependentExpression>::const_iterator found =
      ctx.exprs.find(id);
  if(found == ctx.exprs.end()) {
    throw logic_error("unknown ABI fact expression reference '" + id + "'");
  }
  return found->second;
}

const FactLocalContext & require_fact_context_ref(const FactContext & ctx,
                                                  const string & id)
{
  map<string, FactLocalContext>::const_iterator found = ctx.contexts.find(id);
  if(found == ctx.contexts.end()) {
    throw logic_error("unknown ABI fact local context reference '" + id + "'");
  }
  return found->second;
}

const FactEntity & require_fact_entity_ref(const FactContext & ctx,
                                           const string & id)
{
  map<string, FactEntity>::const_iterator found = ctx.entities.find(id);
  if(found == ctx.entities.end()) {
    throw logic_error("unknown ABI fact entity reference '" + id + "'");
  }
  return found->second;
}

string fundamental_mangle_code(EFundamentalType fundamental)
{
  switch(fundamental) {
  case FT_SIGNED_CHAR:
    return "a";
  case FT_SHORT_INT:
    return "s";
  case FT_INT:
    return "i";
  case FT_LONG_INT:
    return "l";
  case FT_LONG_LONG_INT:
    return "x";
  case FT_INT128:
    return "n";
  case FT_UNSIGNED_CHAR:
    return "h";
  case FT_UNSIGNED_SHORT_INT:
    return "t";
  case FT_UNSIGNED_INT:
    return "j";
  case FT_UNSIGNED_LONG_INT:
    return "m";
  case FT_UNSIGNED_LONG_LONG_INT:
    return "y";
  case FT_UINT128:
    return "o";
  case FT_WCHAR_T:
    return "w";
  case FT_CHAR:
    return "c";
  case FT_CHAR16_T:
    return "Ds";
  case FT_CHAR32_T:
    return "Di";
  case FT_BOOL:
    return "b";
  case FT_FLOAT:
    return "f";
  case FT_DOUBLE:
    return "d";
  case FT_LONG_DOUBLE:
    return "e";
  case FT_VOID:
    return "v";
  case FT_NULLPTR_T:
    return "Dn";
  }
  return string();
}

bool builtin_code_from_name(const string & word, string & code)
{
  const map<string, EFundamentalType> fundamentals = fundamental_types();
  const map<string, EFundamentalType>::const_iterator fundamental =
      fundamentals.find(word);
  if(fundamental == fundamentals.end()) {
    return false;
  }
  code = fundamental_mangle_code(fundamental->second);
  if(code.empty()) {
    throw logic_error("unable to map builtin ABI fact type '" + word + "'");
  }
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
  return out;
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
    out.array_bound = rest.substr(0, pos);
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
  if(kind == "builtin") {
    if(begin + 2 != words.size()) {
      throw logic_error("builtin type requires one ABI code");
    }
    out.kind = ABI_TYPE_BUILTIN_CODE;
    out.abi_code = words[begin + 1];
    return out;
  }
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
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "array") {
    if(begin + 3 != words.size()) {
      throw logic_error("array type requires bound and element type");
    }
    out.kind = ABI_TYPE_ARRAY;
    out.array_bound = words[begin + 1];
    out.child_types.push_back(parse_single_type_token(words[begin + 2]));
    return out;
  }
  if(kind == "function-type") {
    if(begin + 2 >= words.size()) {
      throw logic_error("function-type requires a result type");
    }
    out.kind = ABI_TYPE_FUNCTION;
    for(size_t i = begin + 1; i < words.size(); ++i) {
      out.child_types.push_back(parse_single_type_token(words[i]));
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
  if(kind == "std-template") {
    if(begin + 4 > words.size()) {
      throw logic_error(
          "std-template type requires substitution, includes flag, and name");
    }
    out.kind = ABI_TYPE_STD_CLASS_TEMPLATE;
    out.std_substitution = words[begin + 1];
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

abi_ir::Type lower_type(FactContext & ctx, const AbiType & type);
FactTemplateArgument lower_argument(FactContext & ctx,
                                    const AbiTemplateArg & fact);
abi_ir::DependentExpression lower_expression(
    FactContext & ctx,
    const AbiDependentExpr & fact);

abi_ir::Type fact_type_ref_or_builtin(FactContext & ctx, const string & word)
{
  map<string, abi_ir::Type>::const_iterator found = ctx.types.find(word);
  if(found != ctx.types.end()) {
    return found->second;
  }

  string code;
  if(builtin_code_from_name(word, code)) {
    return abi_ir::Type::builtin(code);
  }

  throw logic_error("unknown ABI fact type reference '" + word + "'");
}

vector<abi_ir::Type::ClassTemplateArgument> class_args_from_refs(
    FactContext & ctx,
    const vector<string> & refs)
{
  vector<abi_ir::Type::ClassTemplateArgument> out;
  for(size_t i = 0; i < refs.size(); ++i) {
    out.push_back(require_fact_arg_ref(ctx, refs[i]).class_arg);
  }
  return out;
}

vector<abi_ir::TemplateArgument> function_args_from_refs(
    FactContext & ctx,
    const vector<string> & refs)
{
  vector<abi_ir::TemplateArgument> out;
  for(size_t i = 0; i < refs.size(); ++i) {
    out.push_back(require_fact_arg_ref(ctx, refs[i]).function_arg);
  }
  return out;
}

string terminal_fragment_from_fact_word(const string & word)
{
  if(word == "operator-call") {
    return "cl";
  }
  if(word == "operator-assign") {
    return "aS";
  }
  if(word == "constructor-complete") {
    return "C1";
  }
  if(word == "constructor-base") {
    return "C2";
  }
  if(word == "destructor-complete") {
    return "D1";
  }
  if(word == "destructor-base") {
    return "D2";
  }
  if(word == "destructor-deleting") {
    return "D0";
  }
  throw logic_error("unknown ABI fact terminal name '" + word + "'");
}

AbiFunctionTerminal terminal_from_fact_word(const string & word)
{
  if(word == "operator-call") {
    return ABI_FUNCTION_TERMINAL_OPERATOR_CALL;
  }
  if(word == "operator-assign") {
    return ABI_FUNCTION_TERMINAL_OPERATOR_ASSIGN;
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

string terminal_fragment_from_fact_terminal(AbiFunctionTerminal terminal)
{
  return terminal_fragment_from_fact_word(
      terminal_word_from_fact_terminal(terminal));
}

abi_ir::FunctionEncoding fact_path_function_encoding(
    FactContext & ctx,
    const AbiFunctionPath & path)
{
  abi_ir::FunctionEncoding function;
  function.name_components = make_ir_function_components(path.qualified_name);
  function.template_arguments =
      function_args_from_refs(ctx, path.template_argument_references);
  if(!function.template_arguments.empty()) {
    function.template_prefix_key =
        abi_ir::SubstitutionKey::function_template_prefix(path.qualified_name);
  }
  if(path.has_result_type) {
    function.parameter_types.push_back(lower_type(ctx, path.result_type));
  }
  for(size_t i = 0; i < path.parameter_types.size(); ++i) {
    function.parameter_types.push_back(lower_type(ctx, path.parameter_types[i]));
  }
  return function;
}

string emit_fact_function_symbol(const abi_ir::FunctionEncoding & function)
{
  SimpleIrSubstitutionSink sink;
  string out;
  if(!abi_ir::emit_function_encoding(function, out, &sink)) {
    throw logic_error("unable to encode ABI fact function entity");
  }
  return out;
}

string emit_fact_variable_symbol(const string & qualified_name)
{
  abi_ir::FunctionEncoding name;
  name.name_components = make_ir_function_components(qualified_name);
  SimpleIrSubstitutionSink sink;
  string encoding;
  if(!abi_ir::emit_function_name(name, encoding, &sink)) {
    throw logic_error("unable to encode ABI fact variable entity");
  }
  return "_Z" + encoding;
}

FactLocalContext emit_fact_local_context(
    const abi_ir::FunctionEncoding & function)
{
  SimpleIrSubstitutionSink sink;
  string encoding;
  if(!abi_ir::emit_function_encoding(function, encoding, &sink) ||
     encoding.compare(0, 2, "_Z") != 0) {
    throw logic_error("unable to encode ABI fact local context");
  }
  FactLocalContext out;
  out.fragment = "Z" + encoding.substr(2) + "E";
  out.substitution_slots = substitution_slots_from_sink(sink);
  out.function.reset(new abi_ir::FunctionEncoding(function));
  return out;
}

abi_ir::Type lower_type(FactContext & ctx, const AbiType & type)
{
  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    return fact_type_ref_or_builtin(ctx, type.reference);
  case ABI_TYPE_BUILTIN: {
    string code;
    if(!builtin_code_from_name(type.name, code)) {
      throw logic_error("unknown builtin ABI fact type '" + type.name + "'");
    }
    return abi_ir::Type::builtin(code);
  }
  case ABI_TYPE_BUILTIN_CODE:
    return abi_ir::Type::builtin(type.abi_code);
  case ABI_TYPE_TEMPLATE_PARAMETER: {
    abi_ir::Type out =
        abi_ir::Type::template_parameter(type.template_parameter_index);
    if(type.substitutable_template_parameter) {
      abi_ir::set_substitution(
          out,
          abi_ir::SubstitutionKey::type_template_parameter(
              type.template_parameter_index));
    }
    return out;
  }
  case ABI_TYPE_POINTER:
  case ABI_TYPE_LVALUE_REFERENCE:
  case ABI_TYPE_RVALUE_REFERENCE:
  case ABI_TYPE_CONST:
  case ABI_TYPE_VOLATILE:
  case ABI_TYPE_VENDOR_QUALIFIED:
  case ABI_TYPE_PACK_EXPANSION: {
    if(type.child_types.size() != 1) {
      throw logic_error("unary ABI fact type requires one child type");
    }
    const abi_ir::Type inner = lower_type(ctx, type.child_types[0]);
    if(type.kind == ABI_TYPE_POINTER) {
      return abi_ir::Type::pointer(inner);
    }
    if(type.kind == ABI_TYPE_LVALUE_REFERENCE) {
      return abi_ir::Type::lvalue_reference(inner);
    }
    if(type.kind == ABI_TYPE_RVALUE_REFERENCE) {
      return abi_ir::Type::rvalue_reference(inner);
    }
    if(type.kind == ABI_TYPE_CONST) {
      return abi_ir::Type::cv(true, false, inner);
    }
    if(type.kind == ABI_TYPE_VOLATILE) {
      return abi_ir::Type::cv(false, true, inner);
    }
    if(type.kind == ABI_TYPE_VENDOR_QUALIFIED) {
      if(type.name.empty()) {
        throw logic_error("vendor ABI fact type requires a qualifier");
      }
      return abi_ir::Type::vendor_qualified(type.name, inner);
    }
    return abi_ir::Type::pack_expansion(inner);
  }
  case ABI_TYPE_ARRAY:
    if(type.child_types.size() != 1) {
      throw logic_error("array ABI fact type requires one element type");
    }
    return abi_ir::Type::array(type.array_bound,
                               lower_type(ctx, type.child_types[0]));
  case ABI_TYPE_FUNCTION: {
    if(type.child_types.empty()) {
      throw logic_error("function ABI fact type requires a result type");
    }
    vector<abi_ir::Type> params;
    for(size_t i = 1; i < type.child_types.size(); ++i) {
      params.push_back(lower_type(ctx, type.child_types[i]));
    }
    return abi_ir::Type::function(lower_type(ctx, type.child_types[0]),
                                  params,
                                  false);
  }
  case ABI_TYPE_MEMBER_POINTER:
    if(type.child_types.size() != 2) {
      throw logic_error("member-pointer ABI fact type requires owner and member type");
    }
    return abi_ir::Type::member_pointer(
        lower_type(ctx, type.child_types[0]),
        lower_type(ctx, type.child_types[1]));
  case ABI_TYPE_NAMED:
    return make_ir_named_type(type.name);
  case ABI_TYPE_CLASS_TEMPLATE:
    return abi_ir::Type::class_template_specialization(
        make_ir_prefix_components(split_qualified_name(type.name)),
        split_qualified_name(type.name).back(),
        type.name,
        class_args_from_refs(ctx, type.template_argument_references),
        string(),
        false);
  case ABI_TYPE_STD_CLASS_TEMPLATE: {
    const vector<string> parts = split_qualified_name(type.name);
    return abi_ir::Type::class_template_specialization(
        make_ir_prefix_components(parts),
        parts.back(),
        type.name,
        class_args_from_refs(ctx, type.template_argument_references),
        type.std_substitution,
        type.std_substitution_includes_template_arguments);
  }
  case ABI_TYPE_MEMBER_TYPE:
    if(type.child_types.size() != 1) {
      throw logic_error("member ABI fact type requires one owner type");
    }
    return abi_ir::Type::member_named_type(
        lower_type(ctx, type.child_types[0]),
        type.name,
        type.name);
  case ABI_TYPE_MEMBER_CLASS_TEMPLATE:
    if(type.child_types.size() != 1) {
      throw logic_error("member-template ABI fact type requires one owner type");
    }
    return abi_ir::Type::member_class_template_specialization(
        lower_type(ctx, type.child_types[0]),
        type.name,
        type.name,
        class_args_from_refs(ctx, type.template_argument_references));
  case ABI_TYPE_DECLTYPE: {
    abi_ir::Type out;
    out.kind = abi_ir::Type::TK_DECLTYPE_EXPRESSION;
    out.expression.reset(
        new abi_ir::DependentExpression(require_fact_expr_ref(ctx,
                                                              type.expression_reference)));
    return out;
  }
  case ABI_TYPE_LAMBDA_CLOSURE: {
    const FactLocalContext & context =
        require_fact_context_ref(ctx, type.context_reference);
    vector<abi_ir::Type> signature;
    for(size_t i = 0; i < type.child_types.size(); ++i) {
      signature.push_back(lower_type(ctx, type.child_types[i]));
    }
    return abi_ir::Type::lambda_closure(context.fragment,
                                        context.substitution_slots,
                                        context.function,
                                        signature,
                                        type.discriminator);
  }
  case ABI_TYPE_LOCAL_TYPE: {
    const FactLocalContext & context =
        require_fact_context_ref(ctx, type.context_reference);
    return abi_ir::Type::lambda_closure(context.fragment,
                                        context.substitution_slots,
                                        context.function,
                                        vector<abi_ir::Type>(),
                                        type.discriminator,
                                        type.source_name);
  }
  }
  throw logic_error("unknown ABI fact type kind");
}

FactTemplateArgument lower_argument(FactContext & ctx,
                                    const AbiTemplateArg & fact)
{
  FactTemplateArgument out;
  switch(fact.kind) {
  case ABI_TEMPLATE_ARG_TYPE: {
    const abi_ir::Type type = lower_type(ctx, fact.type);
    out.class_arg = abi_ir::Type::ClassTemplateArgument::type_arg(type);
    out.function_arg = abi_ir::TemplateArgument::type_arg(type);
    return out;
  }
  case ABI_TEMPLATE_ARG_INTEGRAL_VALUE: {
    const abi_ir::Type type = lower_type(ctx, fact.type);
    const long long value = fact.integer_value;
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::integral_value_arg(type, value);
    out.function_arg = abi_ir::TemplateArgument::integral_value_arg(type, value);
    return out;
  }
  case ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE: {
    const long long value = fact.integer_value;
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::untyped_integral_value_arg(value);
    out.function_arg = abi_ir::TemplateArgument::untyped_integral_value_arg(value);
    return out;
  }
  case ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION: {
    const abi_ir::DependentExpression & expression =
        require_fact_expr_ref(ctx, fact.expression_reference);
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::dependent_expression_arg(expression);
    out.function_arg =
        abi_ir::TemplateArgument::dependent_expression_arg(expression);
    return out;
  }
  case ABI_TEMPLATE_ARG_ENTITY_ADDRESS:
  case ABI_TEMPLATE_ARG_ENTITY_REFERENCE: {
    const bool address_of = fact.kind == ABI_TEMPLATE_ARG_ENTITY_ADDRESS;
    const FactEntity & entity = require_fact_entity_ref(ctx,
                                                        fact.entity_reference);
    out.class_arg = abi_ir::Type::ClassTemplateArgument::external_entity_arg(
        entity.symbol,
        address_of);
    out.function_arg =
        abi_ir::TemplateArgument::external_entity_arg(entity.symbol,
                                                     address_of);
    return out;
  }
  case ABI_TEMPLATE_ARG_PACK: {
    vector<abi_ir::Type::ClassTemplateArgument> class_pack;
    vector<abi_ir::TemplateArgument> function_pack;
    for(size_t i = 0; i < fact.pack_argument_references.size(); ++i) {
      const FactTemplateArgument & argument =
          require_fact_arg_ref(ctx, fact.pack_argument_references[i]);
      class_pack.push_back(argument.class_arg);
      function_pack.push_back(argument.function_arg);
    }
    out.class_arg = abi_ir::Type::ClassTemplateArgument::argument_pack(class_pack);
    out.function_arg = abi_ir::TemplateArgument::argument_pack(function_pack);
    return out;
  }
  }
  throw logic_error("unknown ABI fact template argument kind");
}

abi_ir::DependentExpression lower_expression(
    FactContext & ctx,
    const AbiDependentExpr & fact)
{
  switch(fact.kind) {
  case ABI_EXPR_TEMPLATE_PARAMETER:
    return abi_ir::DependentExpression::template_parameter(
        fact.index);
  case ABI_EXPR_FUNCTION_PARAMETER:
    return abi_ir::DependentExpression::function_parameter(
        fact.index);
  case ABI_EXPR_LITERAL:
    return abi_ir::DependentExpression::literal(fact.literal);
  case ABI_EXPR_UNARY:
    return abi_ir::DependentExpression::unary(
        fact.opcode,
        require_fact_expr_ref(ctx, fact.first_reference));
  case ABI_EXPR_BINARY:
    return abi_ir::DependentExpression::binary(
        fact.opcode,
        require_fact_expr_ref(ctx, fact.first_reference),
        require_fact_expr_ref(ctx, fact.second_reference));
  case ABI_EXPR_CONDITIONAL:
    return abi_ir::DependentExpression::conditional(
        require_fact_expr_ref(ctx, fact.first_reference),
        require_fact_expr_ref(ctx, fact.second_reference),
        require_fact_expr_ref(ctx, fact.third_reference));
  case ABI_EXPR_MEMBER:
    return abi_ir::DependentExpression::member(
        lower_type(ctx, fact.owner_type),
        fact.close_template_arguments,
        fact.member_name);
  case ABI_EXPR_ENTITY_ADDRESS:
  case ABI_EXPR_ENTITY_REFERENCE: {
    const FactEntity & entity = require_fact_entity_ref(ctx,
                                                        fact.entity_reference);
    return abi_ir::DependentExpression::external_entity(
        entity.symbol,
        fact.kind == ABI_EXPR_ENTITY_ADDRESS);
  }
  }
  throw logic_error("unknown ABI fact expression kind");
}

bool single_type_token(const AbiType & type, string & token)
{
  switch(type.kind) {
  case ABI_TYPE_REFERENCE:
    token = type.reference;
    return true;
  case ABI_TYPE_BUILTIN:
    token = type.name;
    return true;
  case ABI_TYPE_POINTER:
  case ABI_TYPE_LVALUE_REFERENCE:
  case ABI_TYPE_RVALUE_REFERENCE:
  case ABI_TYPE_CONST:
  case ABI_TYPE_VOLATILE:
  case ABI_TYPE_VENDOR_QUALIFIED: {
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
    } else {
      if(type.name.empty()) {
        return false;
      }
      token = "vendor:" + type.name + ":" + child;
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
    token = "array:" + type.array_bound + ":" + child;
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
  case ABI_TYPE_BUILTIN_CODE:
    words.push_back("builtin");
    words.push_back(type.abi_code);
    break;
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
  case ABI_TYPE_FUNCTION:
    words.push_back("function-type");
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
  case ABI_TYPE_STD_CLASS_TEMPLATE:
    words.push_back("std-template");
    words.push_back(type.std_substitution);
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
    out.parameter_types.push_back(parse_single_type_token(words[i]));
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
  if(kind == "unary") {
    if(words.size() != 5) {
      throw logic_error("unary expression requires opcode and operand");
    }
    out.kind = ABI_EXPR_UNARY;
    out.opcode = words[3];
    out.first_reference = words[4];
    return out;
  }
  if(kind == "binary") {
    if(words.size() != 6) {
      throw logic_error("binary expression requires opcode and two operands");
    }
    out.kind = ABI_EXPR_BINARY;
    out.opcode = words[3];
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
  if(kind == "member") {
    if(words.size() != 6) {
      throw logic_error("member expression requires owner type, close flag, name");
    }
    out.kind = ABI_EXPR_MEMBER;
    out.owner_type = parse_single_type_token(words[3]);
    out.close_template_arguments = boolean_word(words[4]);
    out.member_name = words[5];
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
  if(command == "typeinfo" || command == "vtable") {
    if(words.size() < 2) {
      throw logic_error(command + " fact requires a type");
    }
    require_no_target(fact_case);
    fact_case.target.kind = command == "typeinfo" ?
        ABI_MANGLE_TYPEINFO : ABI_MANGLE_VTABLE;
    fact_case.target.type = parse_type_spec(words, 1);
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
    if(words[1] == "path") {
      if(words.size() < 3) {
        throw logic_error("function path requires a qualified name");
      }
      function.form = ABI_FUNCTION_PATH;
      function.qualified_name = words[2];
      function.template_argument_references.assign(words.begin() + 3,
                                                   words.end());
      return;
    }
    if(words[1] == "lambda") {
      if(words.size() < 5) {
        throw logic_error(
            "function lambda requires context, discriminator, and terminal");
      }
      function.form = ABI_FUNCTION_LAMBDA;
      function.context_reference = words[2];
      function.discriminator = words[3];
      function.terminal = terminal_from_fact_word(words[4]);
      for(size_t i = 5; i < words.size(); ++i) {
        function.lambda_signature_parameter_types.push_back(
            parse_single_type_token(words[i]));
      }
      return;
    }
    if(words[1] == "local") {
      if(words.size() < 5 || words.size() > 6) {
        throw logic_error(
            "function local requires context, source name, terminal, and optional discriminator");
      }
      function.form = ABI_FUNCTION_LOCAL;
      function.context_reference = words[2];
      function.source_name = words[3];
      function.terminal = terminal_from_fact_word(words[4]);
      function.discriminator = words.size() == 6 ? words[5] : "0";
      return;
    }
    function.form = ABI_FUNCTION_PATH;
    function.qualified_name = words[1];
    for(size_t i = 2; i < words.size(); ++i) {
      function.parameter_types.push_back(parse_single_type_token(words[i]));
    }
    return;
  }
  if(command == "param") {
    if(fact_case.target.kind != ABI_MANGLE_FUNCTION) {
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
    if(fact_case.target.kind != ABI_MANGLE_FUNCTION) {
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

void apply_fact(FactContext & ctx, const AbiFact & fact)
{
  switch(fact.kind) {
  case ABI_FACT_TYPE:
    ctx.types[fact.id] = lower_type(ctx, fact.type);
    return;
  case ABI_FACT_TEMPLATE_ARGUMENT:
    ctx.args[fact.id] = lower_argument(ctx, fact.template_argument);
    return;
  case ABI_FACT_EXPRESSION:
    ctx.exprs[fact.id] = lower_expression(ctx, fact.expression);
    return;
  case ABI_FACT_LOCAL_CONTEXT:
    ctx.contexts[fact.id] =
        emit_fact_local_context(fact_path_function_encoding(ctx,
                                                            fact.context_function));
    return;
  case ABI_FACT_ENTITY: {
    FactEntity entity;
    if(fact.entity.kind == ABI_ENTITY_FUNCTION) {
      entity.symbol = emit_fact_function_symbol(
          fact_path_function_encoding(ctx, fact.entity.function));
    } else {
      entity.symbol = emit_fact_variable_symbol(fact.entity.qualified_name);
    }
    ctx.entities[fact.id] = entity;
    return;
  }
  }
  throw logic_error("unknown ABI fact kind");
}

abi_ir::FunctionEncoding lower_function_target(FactContext & ctx,
                                               const AbiFunction & target)
{
  abi_ir::FunctionEncoding function;
  if(target.form == ABI_FUNCTION_PATH) {
    function.name_components = make_ir_function_components(target.qualified_name);
    function.template_arguments =
        function_args_from_refs(ctx, target.template_argument_references);
    if(!function.template_arguments.empty()) {
      function.template_prefix_key =
          abi_ir::SubstitutionKey::function_template_prefix(
              target.qualified_name);
    }
  } else if(target.form == ABI_FUNCTION_LAMBDA) {
    const FactLocalContext & context =
        require_fact_context_ref(ctx, target.context_reference);
    abi_ir::FunctionEncoding::LambdaMetadata & lambda =
        abi_ir::FunctionEncoding::ensure_lambda_metadata(function);
    lambda.context_fragment = context.fragment;
    lambda.context_substitution_slots = context.substitution_slots;
    lambda.discriminator = target.discriminator;
    function.terminal_fragment =
        terminal_fragment_from_fact_terminal(target.terminal);
    for(size_t i = 0; i < target.lambda_signature_parameter_types.size(); ++i) {
      lambda.signature_parameter_types.push_back(
          lower_type(ctx, target.lambda_signature_parameter_types[i]));
    }
  } else if(target.form == ABI_FUNCTION_LOCAL) {
    const FactLocalContext & context =
        require_fact_context_ref(ctx, target.context_reference);
    abi_ir::FunctionEncoding::LambdaMetadata & local =
        abi_ir::FunctionEncoding::ensure_lambda_metadata(function);
    local.context_fragment = context.fragment;
    local.context_substitution_slots = context.substitution_slots;
    local.source_name = target.source_name;
    local.discriminator = target.discriminator.empty() ? "0" :
        target.discriminator;
    function.terminal_fragment =
        terminal_fragment_from_fact_terminal(target.terminal);
  }
  if(target.has_result_type) {
    function.parameter_types.push_back(lower_type(ctx, target.result_type));
  }
  for(size_t i = 0; i < target.parameter_types.size(); ++i) {
    function.parameter_types.push_back(lower_type(ctx, target.parameter_types[i]));
  }
  return function;
}

string emit_type_encoding(FactContext & ctx, const AbiType & type)
{
  SimpleIrSubstitutionSink sink;
  string out;
  if(!abi_ir::emit_type(lower_type(ctx, type), out, &sink)) {
    throw logic_error("shared ABI IR mangler failed to encode fact type");
  }
  return out;
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
    case ABI_TEMPLATE_ARG_UNTYPED_INTEGRAL_VALUE:
      words.push_back("untyped-value");
      words.push_back(to_string(fact.template_argument.integer_value));
      break;
    case ABI_TEMPLATE_ARG_DEPENDENT_EXPRESSION:
      words.push_back("expression");
      words.push_back(fact.template_argument.expression_reference);
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
    case ABI_EXPR_UNARY:
      words.push_back("unary");
      words.push_back(fact.expression.opcode);
      words.push_back(fact.expression.first_reference);
      break;
    case ABI_EXPR_BINARY:
      words.push_back("binary");
      words.push_back(fact.expression.opcode);
      words.push_back(fact.expression.first_reference);
      words.push_back(fact.expression.second_reference);
      break;
    case ABI_EXPR_CONDITIONAL:
      words.push_back("conditional");
      words.push_back(fact.expression.first_reference);
      words.push_back(fact.expression.second_reference);
      words.push_back(fact.expression.third_reference);
      break;
    case ABI_EXPR_MEMBER:
      words.push_back("member");
      append_single_type_token(words, fact.expression.owner_type);
      words.push_back(fact.expression.close_template_arguments ? "yes" : "no");
      words.push_back(fact.expression.member_name);
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
  case ABI_MANGLE_VARIABLE:
    words.push_back(target.c_linkage ? "c-variable" : "variable");
    words.push_back(target.qualified_name);
    lines.push_back(words);
    return lines;
  case ABI_MANGLE_FUNCTION: {
    words.push_back(target.function.c_linkage ? "c-function" : "function");
    if(target.function.form == ABI_FUNCTION_PATH) {
      words.push_back("path");
      words.push_back(target.function.qualified_name);
      words.insert(words.end(),
                   target.function.template_argument_references.begin(),
                   target.function.template_argument_references.end());
    } else if(target.function.form == ABI_FUNCTION_LAMBDA) {
      words.push_back("lambda");
      words.push_back(target.function.context_reference);
      words.push_back(target.function.discriminator);
      words.push_back(terminal_word_from_fact_terminal(
          target.function.terminal));
      for(size_t i = 0;
          i < target.function.lambda_signature_parameter_types.size();
          ++i) {
        append_single_type_token(
            words,
            target.function.lambda_signature_parameter_types[i]);
      }
    } else {
      words.push_back("local");
      words.push_back(target.function.context_reference);
      words.push_back(target.function.source_name);
      words.push_back(terminal_word_from_fact_terminal(
          target.function.terminal));
      words.push_back(target.function.discriminator);
    }
    lines.push_back(words);
    if(target.function.has_result_type) {
      vector<string> result_words;
      result_words.push_back("result");
      append_type_spec_words(result_words, target.function.result_type);
      lines.push_back(result_words);
    }
    for(size_t i = 0; i < target.function.parameter_types.size(); ++i) {
      vector<string> param_words;
      param_words.push_back("param");
      append_type_spec_words(param_words, target.function.parameter_types[i]);
      lines.push_back(param_words);
    }
    return lines;
  }
  }
  throw logic_error("unknown ABI mangle target kind");
}

string mangle_case(const AbiFactCase & fact_case)
{
  if(fact_case.facts.empty() && fact_case.target.kind == ABI_MANGLE_NONE) {
    throw logic_error("empty ABI fact case");
  }
  FactContext ctx;
  for(size_t i = 0; i < fact_case.facts.size(); ++i) {
    apply_fact(ctx, fact_case.facts[i]);
  }

  switch(fact_case.target.kind) {
  case ABI_MANGLE_TYPE:
    return emit_type_encoding(ctx, fact_case.target.type);
  case ABI_MANGLE_FUNCTION:
    if(fact_case.target.function.c_linkage) {
      return unqualified_name(fact_case.target.function.qualified_name);
    }
    return emit_fact_function_symbol(
        lower_function_target(ctx, fact_case.target.function));
  case ABI_MANGLE_VARIABLE:
    if(fact_case.target.c_linkage) {
      return unqualified_name(fact_case.target.qualified_name);
    }
    return emit_fact_variable_symbol(fact_case.target.qualified_name);
  case ABI_MANGLE_TYPEINFO:
    return "_ZTI" + emit_type_encoding(ctx, fact_case.target.type);
  case ABI_MANGLE_VTABLE:
    return "_ZTV" + emit_type_encoding(ctx, fact_case.target.type);
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
