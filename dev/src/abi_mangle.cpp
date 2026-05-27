#include "abi_mangle.h"

#include "cpp_decl_model.h"
#include "itanium_mangle_ir.h"
#include "symbol_linkage.h"

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

struct FactTemplateArgument
{
  abi_ir::Type::ClassTemplateArgument class_arg;
  abi_ir::TemplateArgument function_arg;
};

struct FactLocalContext
{
  string fragment;
  vector<abi_ir::SubstitutionSlot> substitution_slots;
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

abi_ir::Type fact_type(FactContext & ctx,
                       const vector<string> & words,
                       size_t begin);
FactTemplateArgument fact_argument(FactContext & ctx,
                                   const vector<string> & words,
                                   size_t begin);
abi_ir::DependentExpression fact_expression(FactContext & ctx,
                                            const vector<string> & words,
                                            size_t begin);

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

abi_ir::Type fact_type_ref_or_builtin(FactContext & ctx, const string & word)
{
  map<string, abi_ir::Type>::const_iterator found = ctx.types.find(word);
  if(found != ctx.types.end()) {
    return found->second;
  }

  const map<string, EFundamentalType> fundamentals = fundamental_types();
  const map<string, EFundamentalType>::const_iterator fundamental =
      fundamentals.find(word);
  if(fundamental != fundamentals.end()) {
    TypePtr type = make_fundamental(fundamental->second);
    string code;
    if(!symbol_linkage::mangle_itanium_type_encoding(type, code)) {
      throw logic_error("unable to map builtin ABI fact type '" + word + "'");
    }
    return abi_ir::Type::builtin(code);
  }

  throw logic_error("unknown ABI fact type reference '" + word + "'");
}

vector<abi_ir::Type::ClassTemplateArgument> class_args_from_words(
    FactContext & ctx,
    const vector<string> & words,
    size_t begin)
{
  vector<abi_ir::Type::ClassTemplateArgument> out;
  for(size_t i = begin; i < words.size(); ++i) {
    out.push_back(require_fact_arg_ref(ctx, words[i]).class_arg);
  }
  return out;
}

vector<abi_ir::TemplateArgument> function_args_from_words(
    FactContext & ctx,
    const vector<string> & words,
    size_t begin)
{
  vector<abi_ir::TemplateArgument> out;
  for(size_t i = begin; i < words.size(); ++i) {
    out.push_back(require_fact_arg_ref(ctx, words[i]).function_arg);
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

abi_ir::FunctionEncoding fact_path_function_encoding(
    FactContext & ctx,
    const string & qualified_name,
    const vector<string> & parameter_words)
{
  abi_ir::FunctionEncoding function;
  function.name_components = make_ir_function_components(qualified_name);
  for(size_t i = 0; i < parameter_words.size(); ++i) {
    function.parameter_types.push_back(
        fact_type_ref_or_builtin(ctx, parameter_words[i]));
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
  return out;
}

abi_ir::Type fact_type(FactContext & ctx,
                       const vector<string> & words,
                       size_t begin)
{
  if(begin >= words.size()) {
    throw logic_error("missing ABI fact type");
  }
  if(begin + 1 == words.size()) {
    return fact_type_ref_or_builtin(ctx, words[begin]);
  }

  const string & kind = words[begin];
  if(kind == "builtin") {
    if(begin + 2 != words.size()) {
      throw logic_error("builtin type requires one ABI code");
    }
    return abi_ir::Type::builtin(words[begin + 1]);
  }
  if(kind == "template-param") {
    if(begin + 2 != words.size()) {
      throw logic_error("template-param type requires one index");
    }
    return abi_ir::Type::template_parameter(parse_index(words[begin + 1]));
  }
  if(kind == "ptr" || kind == "ref" || kind == "rref" ||
     kind == "const" || kind == "volatile" || kind == "pack") {
    if(begin + 2 != words.size()) {
      throw logic_error(kind + " type requires one operand");
    }
    const abi_ir::Type inner = fact_type_ref_or_builtin(ctx, words[begin + 1]);
    if(kind == "ptr") {
      return abi_ir::Type::pointer(inner);
    }
    if(kind == "ref") {
      return abi_ir::Type::lvalue_reference(inner);
    }
    if(kind == "rref") {
      return abi_ir::Type::rvalue_reference(inner);
    }
    if(kind == "const") {
      return abi_ir::Type::cv(true, false, inner);
    }
    if(kind == "volatile") {
      return abi_ir::Type::cv(false, true, inner);
    }
    return abi_ir::Type::pack_expansion(inner);
  }
  if(kind == "array") {
    if(begin + 3 != words.size()) {
      throw logic_error("array type requires bound and element type");
    }
    return abi_ir::Type::array(words[begin + 1],
                               fact_type_ref_or_builtin(ctx, words[begin + 2]));
  }
  if(kind == "function-type") {
    if(begin + 2 >= words.size()) {
      throw logic_error("function-type requires a result type");
    }
    vector<abi_ir::Type> params;
    for(size_t i = begin + 2; i < words.size(); ++i) {
      params.push_back(fact_type_ref_or_builtin(ctx, words[i]));
    }
    return abi_ir::Type::function(fact_type_ref_or_builtin(ctx, words[begin + 1]),
                                  params,
                                  false);
  }
  if(kind == "member-pointer") {
    if(begin + 3 != words.size()) {
      throw logic_error("member-pointer type requires owner and member type");
    }
    return abi_ir::Type::member_pointer(
        fact_type_ref_or_builtin(ctx, words[begin + 1]),
        fact_type_ref_or_builtin(ctx, words[begin + 2]));
  }
  if(kind == "name") {
    if(begin + 2 != words.size()) {
      throw logic_error("name type requires one qualified name");
    }
    return make_ir_named_type(words[begin + 1]);
  }
  if(kind == "template") {
    if(begin + 2 > words.size()) {
      throw logic_error("template type requires a qualified template name");
    }
    return abi_ir::Type::class_template_specialization(
        make_ir_prefix_components(split_qualified_name(words[begin + 1])),
        split_qualified_name(words[begin + 1]).back(),
        words[begin + 1],
        class_args_from_words(ctx, words, begin + 2),
        string(),
        false);
  }
  if(kind == "std-template") {
    if(begin + 4 > words.size()) {
      throw logic_error(
          "std-template type requires substitution, includes flag, and name");
    }
    const vector<string> parts = split_qualified_name(words[begin + 3]);
    return abi_ir::Type::class_template_specialization(
        make_ir_prefix_components(parts),
        parts.back(),
        words[begin + 3],
        class_args_from_words(ctx, words, begin + 4),
        words[begin + 1],
        boolean_word(words[begin + 2]));
  }
  if(kind == "member") {
    if(begin + 3 != words.size()) {
      throw logic_error("member type requires owner type and member name");
    }
    return abi_ir::Type::member_named_type(
        fact_type_ref_or_builtin(ctx, words[begin + 1]),
        words[begin + 2],
        words[begin + 2]);
  }
  if(kind == "member-template") {
    if(begin + 3 > words.size()) {
      throw logic_error("member-template type requires owner type and name");
    }
    return abi_ir::Type::member_class_template_specialization(
        fact_type_ref_or_builtin(ctx, words[begin + 1]),
        words[begin + 2],
        words[begin + 2],
        class_args_from_words(ctx, words, begin + 3));
  }
  if(kind == "decltype") {
    if(begin + 2 != words.size()) {
      throw logic_error("decltype type requires one expression reference");
    }
    abi_ir::Type type;
    type.kind = abi_ir::Type::TK_DECLTYPE_EXPRESSION;
    type.expression.reset(
        new abi_ir::DependentExpression(require_fact_expr_ref(ctx,
                                                              words[begin + 1])));
    return type;
  }
  if(kind == "lambda-closure") {
    if(begin + 3 > words.size()) {
      throw logic_error("lambda-closure type requires context and discriminator");
    }
    const FactLocalContext & context =
        require_fact_context_ref(ctx, words[begin + 1]);
    vector<abi_ir::Type> signature;
    for(size_t i = begin + 3; i < words.size(); ++i) {
      signature.push_back(fact_type_ref_or_builtin(ctx, words[i]));
    }
    return abi_ir::Type::lambda_closure(context.fragment,
                                        context.substitution_slots,
                                        signature,
                                        words[begin + 2]);
  }
  if(kind == "local-type") {
    if(begin + 4 != words.size()) {
      throw logic_error("local-type requires context, source name, discriminator");
    }
    const FactLocalContext & context =
        require_fact_context_ref(ctx, words[begin + 1]);
    return abi_ir::Type::lambda_closure(context.fragment,
                                        context.substitution_slots,
                                        vector<abi_ir::Type>(),
                                        words[begin + 3],
                                        words[begin + 2]);
  }

  throw logic_error("unknown ABI fact type kind '" + kind + "'");
}

FactTemplateArgument fact_argument(FactContext & ctx,
                                   const vector<string> & words,
                                   size_t begin)
{
  if(begin >= words.size()) {
    throw logic_error("missing ABI fact template argument");
  }
  const string & kind = words[begin];
  FactTemplateArgument out;
  if(kind == "type") {
    const abi_ir::Type type = fact_type(ctx, words, begin + 1);
    out.class_arg = abi_ir::Type::ClassTemplateArgument::type_arg(type);
    out.function_arg = abi_ir::TemplateArgument::type_arg(type);
    return out;
  }
  if(kind == "value") {
    if(begin + 3 != words.size()) {
      throw logic_error("value template argument requires type and integer");
    }
    const abi_ir::Type type = fact_type_ref_or_builtin(ctx, words[begin + 1]);
    const long long value = parse_signed_integer(words[begin + 2]);
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::integral_value_arg(type, value);
    out.function_arg = abi_ir::TemplateArgument::integral_value_arg(type, value);
    return out;
  }
  if(kind == "untyped-value") {
    if(begin + 2 != words.size()) {
      throw logic_error("untyped-value template argument requires an integer");
    }
    const long long value = parse_signed_integer(words[begin + 1]);
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::untyped_integral_value_arg(value);
    out.function_arg = abi_ir::TemplateArgument::untyped_integral_value_arg(value);
    return out;
  }
  if(kind == "expression") {
    if(begin + 2 != words.size()) {
      throw logic_error("expression template argument requires one expression");
    }
    const abi_ir::DependentExpression & expression =
        require_fact_expr_ref(ctx, words[begin + 1]);
    out.class_arg =
        abi_ir::Type::ClassTemplateArgument::dependent_expression_arg(expression);
    out.function_arg =
        abi_ir::TemplateArgument::dependent_expression_arg(expression);
    return out;
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(begin + 2 != words.size()) {
      throw logic_error(kind + " template argument requires one entity");
    }
    const bool address_of = kind == "entity-address";
    const FactEntity & entity = require_fact_entity_ref(ctx, words[begin + 1]);
    out.class_arg = abi_ir::Type::ClassTemplateArgument::external_entity_arg(
        entity.symbol,
        address_of);
    out.function_arg =
        abi_ir::TemplateArgument::external_entity_arg(entity.symbol,
                                                     address_of);
    return out;
  }
  if(kind == "pack") {
    vector<abi_ir::Type::ClassTemplateArgument> class_pack;
    vector<abi_ir::TemplateArgument> function_pack;
    for(size_t i = begin + 1; i < words.size(); ++i) {
      const FactTemplateArgument & argument = require_fact_arg_ref(ctx, words[i]);
      class_pack.push_back(argument.class_arg);
      function_pack.push_back(argument.function_arg);
    }
    out.class_arg = abi_ir::Type::ClassTemplateArgument::argument_pack(class_pack);
    out.function_arg = abi_ir::TemplateArgument::argument_pack(function_pack);
    return out;
  }
  throw logic_error("unknown ABI fact template argument kind '" + kind + "'");
}

abi_ir::DependentExpression fact_expression(FactContext & ctx,
                                            const vector<string> & words,
                                            size_t begin)
{
  if(begin >= words.size()) {
    throw logic_error("missing ABI fact expression");
  }
  if(begin + 1 == words.size()) {
    return require_fact_expr_ref(ctx, words[begin]);
  }

  const string & kind = words[begin];
  if(kind == "template-param") {
    if(begin + 2 != words.size()) {
      throw logic_error("template-param expression requires one index");
    }
    return abi_ir::DependentExpression::template_parameter(
        parse_index(words[begin + 1]));
  }
  if(kind == "function-param") {
    if(begin + 2 != words.size()) {
      throw logic_error("function-param expression requires one index");
    }
    return abi_ir::DependentExpression::function_parameter(
        parse_index(words[begin + 1]));
  }
  if(kind == "literal") {
    if(begin + 2 != words.size()) {
      throw logic_error("literal expression requires one value");
    }
    return abi_ir::DependentExpression::literal(words[begin + 1]);
  }
  if(kind == "unary") {
    if(begin + 3 != words.size()) {
      throw logic_error("unary expression requires opcode and operand");
    }
    return abi_ir::DependentExpression::unary(
        words[begin + 1],
        require_fact_expr_ref(ctx, words[begin + 2]));
  }
  if(kind == "binary") {
    if(begin + 4 != words.size()) {
      throw logic_error("binary expression requires opcode and two operands");
    }
    return abi_ir::DependentExpression::binary(
        words[begin + 1],
        require_fact_expr_ref(ctx, words[begin + 2]),
        require_fact_expr_ref(ctx, words[begin + 3]));
  }
  if(kind == "conditional") {
    if(begin + 4 != words.size()) {
      throw logic_error(
          "conditional expression requires condition, true, and false operands");
    }
    return abi_ir::DependentExpression::conditional(
        require_fact_expr_ref(ctx, words[begin + 1]),
        require_fact_expr_ref(ctx, words[begin + 2]),
        require_fact_expr_ref(ctx, words[begin + 3]));
  }
  if(kind == "member") {
    if(begin + 4 != words.size()) {
      throw logic_error("member expression requires owner type, close flag, name");
    }
    return abi_ir::DependentExpression::member(
        fact_type_ref_or_builtin(ctx, words[begin + 1]),
        boolean_word(words[begin + 2]),
        words[begin + 3]);
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(begin + 2 != words.size()) {
      throw logic_error(kind + " expression requires one entity");
    }
    const FactEntity & entity = require_fact_entity_ref(ctx, words[begin + 1]);
    return abi_ir::DependentExpression::external_entity(
        entity.symbol,
        kind == "entity-address");
  }

  throw logic_error("unknown ABI fact expression kind '" + kind + "'");
}

bool is_structured_function_line(const vector<string> & words)
{
  return words.size() > 1 &&
         (words[1] == "path" || words[1] == "lambda" || words[1] == "local");
}

bool is_legacy_type_line(const vector<string> & words)
{
  return words.size() == 2 && words[1].find(':') != string::npos;
}

AbiFactLineKind fact_line_kind_from_words(const vector<string> & words)
{
  if(words.empty()) {
    return ABI_FACT_LEGACY;
  }
  const string & command = words[0];
  if(command == "let-type") {
    return ABI_FACT_LET_TYPE;
  }
  if(command == "let-arg") {
    return ABI_FACT_LET_ARG;
  }
  if(command == "let-expr") {
    return ABI_FACT_LET_EXPR;
  }
  if(command == "let-context") {
    return ABI_FACT_LET_CONTEXT;
  }
  if(command == "let-entity") {
    return ABI_FACT_LET_ENTITY;
  }
  if(command == "param") {
    return ABI_FACT_PARAM;
  }
  if(command == "function" && is_structured_function_line(words)) {
    return ABI_FACT_RESULT_FUNCTION;
  }
  if(command == "type" && !is_legacy_type_line(words)) {
    return ABI_FACT_RESULT_TYPE;
  }
  return ABI_FACT_LEGACY;
}

AbiFactLine make_fact_line(const vector<string> & words)
{
  AbiFactLine line;
  line.kind = fact_line_kind_from_words(words);
  switch(line.kind) {
  case ABI_FACT_LEGACY:
    if(!words.empty()) {
      line.op = words[0];
      line.operands.assign(words.begin() + 1, words.end());
    }
    break;
  case ABI_FACT_LET_TYPE:
  case ABI_FACT_LET_ARG:
  case ABI_FACT_LET_EXPR:
  case ABI_FACT_LET_CONTEXT:
  case ABI_FACT_LET_ENTITY:
    if(words.size() >= 2) {
      line.id = words[1];
    }
    if(words.size() >= 3) {
      line.op = words[2];
      line.operands.assign(words.begin() + 3, words.end());
    }
    break;
  case ABI_FACT_RESULT_TYPE:
  case ABI_FACT_RESULT_FUNCTION:
  case ABI_FACT_PARAM:
    if(words.size() >= 2) {
      line.op = words[1];
      line.operands.assign(words.begin() + 2, words.end());
    }
    break;
  }
  return line;
}

vector<string> fact_line_words(const AbiFactLine & line)
{
  vector<string> words;
  switch(line.kind) {
  case ABI_FACT_LEGACY:
    break;
  case ABI_FACT_LET_TYPE:
    words.push_back("let-type");
    break;
  case ABI_FACT_LET_ARG:
    words.push_back("let-arg");
    break;
  case ABI_FACT_LET_EXPR:
    words.push_back("let-expr");
    break;
  case ABI_FACT_LET_CONTEXT:
    words.push_back("let-context");
    break;
  case ABI_FACT_LET_ENTITY:
    words.push_back("let-entity");
    break;
  case ABI_FACT_RESULT_TYPE:
    words.push_back("type");
    break;
  case ABI_FACT_RESULT_FUNCTION:
    words.push_back("function");
    break;
  case ABI_FACT_PARAM:
    words.push_back("param");
    break;
  }
  if(line.kind == ABI_FACT_LEGACY) {
    if(!line.op.empty()) {
      words.push_back(line.op);
    }
  } else if(line.kind == ABI_FACT_LET_TYPE ||
            line.kind == ABI_FACT_LET_ARG ||
            line.kind == ABI_FACT_LET_EXPR ||
            line.kind == ABI_FACT_LET_CONTEXT ||
            line.kind == ABI_FACT_LET_ENTITY) {
    words.push_back(line.id);
    words.push_back(line.op);
  } else {
    words.push_back(line.op);
  }
  words.insert(words.end(), line.operands.begin(), line.operands.end());
  return words;
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

void apply_fact_line(FactContext & ctx, const AbiFactLine & line)
{
  const vector<string> words = fact_line_words(line);
  if(words.empty()) {
    return;
  }
  const string & command = words[0];
  if(command == "let-type") {
    if(words.size() < 4) {
      throw logic_error("let-type requires id and type");
    }
    ctx.types[words[1]] = fact_type(ctx, words, 2);
    return;
  }
  if(command == "let-arg") {
    if(words.size() < 4) {
      throw logic_error("let-arg requires id and argument");
    }
    ctx.args[words[1]] = fact_argument(ctx, words, 2);
    return;
  }
  if(command == "let-expr") {
    if(words.size() < 4) {
      throw logic_error("let-expr requires id and expression");
    }
    ctx.exprs[words[1]] = fact_expression(ctx, words, 2);
    return;
  }
  if(command == "let-context") {
    if(words.size() < 4 || words[2] != "function") {
      throw logic_error(
          "let-context requires id, function, name, and optional parameters");
    }
    vector<string> parameter_words(words.begin() + 4, words.end());
    ctx.contexts[words[1]] = emit_fact_local_context(
        fact_path_function_encoding(ctx, words[3], parameter_words));
    return;
  }
  if(command == "let-entity") {
    if(words.size() < 4) {
      throw logic_error("let-entity requires id, kind, and entity data");
    }
    FactEntity entity;
    if(words[2] == "function") {
      vector<string> parameter_words(words.begin() + 4, words.end());
      entity.symbol = emit_fact_function_symbol(
          fact_path_function_encoding(ctx, words[3], parameter_words));
    } else if(words[2] == "variable") {
      if(words.size() != 4) {
        throw logic_error("let-entity variable requires a qualified name");
      }
      entity.symbol = emit_fact_variable_symbol(words[3]);
    } else {
      throw logic_error("unknown ABI fact entity kind '" + words[2] + "'");
    }
    ctx.entities[words[1]] = entity;
    return;
  }
  if(command == "type") {
    if(words.size() < 2) {
      throw logic_error("type fact requires a type");
    }
    ctx.result_type = fact_type(ctx, words, 1);
    ctx.has_type = true;
    return;
  }
  if(command == "function") {
    if(words.size() < 3) {
      throw logic_error("function fact requires a function name form");
    }
    const string & form = words[1];
    ctx.function = abi_ir::FunctionEncoding();
    ctx.has_function = true;
    if(form == "path") {
      ctx.function.name_components = make_ir_function_components(words[2]);
      ctx.function.template_arguments = function_args_from_words(ctx, words, 3);
      return;
    }
    if(form == "lambda") {
      if(words.size() < 5) {
        throw logic_error(
            "function lambda requires context, discriminator, and terminal");
      }
      const FactLocalContext & context = require_fact_context_ref(ctx, words[2]);
      abi_ir::FunctionEncoding::LambdaMetadata & lambda =
          abi_ir::FunctionEncoding::ensure_lambda_metadata(ctx.function);
      lambda.context_fragment = context.fragment;
      lambda.context_substitution_slots = context.substitution_slots;
      lambda.discriminator = words[3];
      ctx.function.terminal_fragment = terminal_fragment_from_fact_word(words[4]);
      for(size_t i = 5; i < words.size(); ++i) {
        lambda.signature_parameter_types.push_back(
            fact_type_ref_or_builtin(ctx, words[i]));
      }
      return;
    }
    if(form == "local") {
      if(words.size() < 5 || words.size() > 6) {
        throw logic_error(
            "function local requires context, source name, terminal, and optional discriminator");
      }
      const FactLocalContext & context = require_fact_context_ref(ctx, words[2]);
      abi_ir::FunctionEncoding::LambdaMetadata & local =
          abi_ir::FunctionEncoding::ensure_lambda_metadata(ctx.function);
      local.context_fragment = context.fragment;
      local.context_substitution_slots = context.substitution_slots;
      local.source_name = words[3];
      local.discriminator = words.size() == 6 ? words[5] : "0";
      ctx.function.terminal_fragment = terminal_fragment_from_fact_word(words[4]);
      return;
    }
    throw logic_error("unknown ABI fact function form '" + form + "'");
  }
  if(command == "param") {
    if(!ctx.has_function) {
      throw logic_error("param appears before function fact");
    }
    if(words.size() < 2) {
      throw logic_error("param requires a type");
    }
    ctx.function.parameter_types.push_back(fact_type(ctx, words, 1));
    return;
  }
  throw logic_error("unknown ABI fact command '" + command + "'");
}

string mangle_line_fact_case(const AbiFactCase & fact_case)
{
  FactContext ctx;
  for(size_t i = 0; i < fact_case.lines.size(); ++i) {
    apply_fact_line(ctx, fact_case.lines[i]);
  }

  if(ctx.has_type == ctx.has_function) {
    throw logic_error("ABI fact case must have exactly one result type or function");
  }
  SimpleIrSubstitutionSink sink;
  string out;
  if(ctx.has_type) {
    if(!abi_ir::emit_type(ctx.result_type, out, &sink)) {
      throw logic_error("shared ABI IR mangler failed to encode fact type");
    }
    return out;
  }
  if(!abi_ir::emit_function_encoding(ctx.function, out, &sink)) {
    throw logic_error("shared ABI IR mangler failed to encode fact function");
  }
  return out;
}

string mangle_legacy_case(const vector<string> & words)
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

string mangle_case(const AbiFactCase & fact_case)
{
  if(fact_case.lines.empty()) {
    throw logic_error("empty ABI fact case");
  }
  if(fact_case.lines.size() == 1 &&
     fact_case.lines[0].kind == ABI_FACT_LEGACY) {
    return mangle_legacy_case(fact_line_words(fact_case.lines[0]));
  }
  return mangle_line_fact_case(fact_case);
}

void flush_case(AbiFactFile & file, AbiFactCase & pending)
{
  if(pending.label.empty() && pending.lines.empty()) {
    return;
  }
  if(pending.lines.empty()) {
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
    if(pending.label.empty()) {
      throw logic_error(input_name + ":" + to_string(line_number) +
                        ": ABI fact appears before case label");
    }
    pending.lines.push_back(make_fact_line(words));
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
    out << "case " << fact_case.label << "\n";
    for(size_t j = 0; j < fact_case.lines.size(); ++j) {
      out << join_words(fact_line_words(fact_case.lines[j])) << "\n";
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
    if(fact_case.label.empty()) {
      throw logic_error("ABI fact case is missing a label");
    }
    out << "case " << fact_case.label << "\n";
    out << "mangled " << mangle_case(fact_case) << "\n";
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
