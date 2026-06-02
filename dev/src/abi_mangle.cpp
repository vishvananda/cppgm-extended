#include "abi_mangle.h"
#include "abi_model.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
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
  while(begin < text.size() && isspace(static_cast<unsigned char>(text[begin]))) {
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

string dash_empty(const string & word)
{
  return word == "-" ? string() : word;
}

bool boolean_word(const string & word)
{
  return word == "1" || word == "yes" || word == "true";
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
  char * end = nullptr;
  const long long value = strtoll(text.c_str(), &end, 10);
  if(text.empty() || !end || *end != '\0') {
    throw logic_error("ABI fact integer must be decimal in '" + text + "'");
  }
  return value;
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

string join_qualified_parts(const vector<string> & parts, size_t end)
{
  string out;
  for(size_t i = 0; i < end; ++i) {
    if(i != 0) {
      out += "::";
    }
    out += parts[i];
  }
  return out;
}

string unqualified_name(const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  return parts.back();
}

string builtin_code_from_name(const string & name)
{
  if(name == "void") { return "v"; }
  if(name == "bool") { return "b"; }
  if(name == "char") { return "c"; }
  if(name == "schar") { return "a"; }
  if(name == "uchar") { return "h"; }
  if(name == "short") { return "s"; }
  if(name == "ushort") { return "t"; }
  if(name == "int") { return "i"; }
  if(name == "uint") { return "j"; }
  if(name == "long") { return "l"; }
  if(name == "ulong") { return "m"; }
  if(name == "longlong") { return "x"; }
  if(name == "ulonglong") { return "y"; }
  if(name == "int128") { return "n"; }
  if(name == "uint128") { return "o"; }
  if(name == "wchar") { return "w"; }
  if(name == "char16") { return "Ds"; }
  if(name == "char32") { return "Di"; }
  if(name == "float") { return "f"; }
  if(name == "double") { return "d"; }
  if(name == "longdouble") { return "e"; }
  if(name == "nullptr") { return "Dn"; }
  return string();
}

string std_substitution_from_word(const string & word)
{
  if(word == "none" || word == "-") { return string(); }
  if(word == "St" || word == "Sa" || word == "Sb" || word == "Ss" ||
     word == "Si" || word == "So" || word == "Sd") {
    return word;
  }
  throw logic_error("unknown std substitution '" + word + "'");
}

struct FactSubstitutionSink : SubstitutionSink
{
  vector<SubstitutionKey> substitutions;

  bool emit_substitution(const SubstitutionKey & key, string & out) override
  {
    if(key.empty()) {
      return false;
    }
    for(size_t i = 0; i < substitutions.size(); ++i) {
      if(substitutions[i] == key) {
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

  void register_substitution(const SubstitutionKey & key) override
  {
    if(key.empty()) {
      return;
    }
    for(size_t i = 0; i < substitutions.size(); ++i) {
      if(substitutions[i] == key) {
        return;
      }
    }
    substitutions.push_back(key);
  }
};

struct ParseContext
{
  map<string, Type> types;
  map<string, TemplateArgument> args;
  map<string, DependentExpression> exprs;
  map<string, LocalContext> contexts;
  map<string, AbiEntity> entities;
};

const Type & require_type_ref(const ParseContext & ctx, const string & id)
{
  map<string, Type>::const_iterator found = ctx.types.find(id);
  if(found == ctx.types.end()) {
    throw logic_error("unknown ABI fact type reference '" + id + "'");
  }
  return found->second;
}

const TemplateArgument & require_arg_ref(const ParseContext & ctx, const string & id)
{
  map<string, TemplateArgument>::const_iterator found = ctx.args.find(id);
  if(found == ctx.args.end()) {
    throw logic_error("unknown ABI fact template argument reference '" + id + "'");
  }
  return found->second;
}

const DependentExpression & require_expr_ref(const ParseContext & ctx, const string & id)
{
  map<string, DependentExpression>::const_iterator found = ctx.exprs.find(id);
  if(found == ctx.exprs.end()) {
    throw logic_error("unknown ABI fact expression reference '" + id + "'");
  }
  return found->second;
}

const LocalContext & require_context_ref(const ParseContext & ctx, const string & id)
{
  map<string, LocalContext>::const_iterator found = ctx.contexts.find(id);
  if(found == ctx.contexts.end()) {
    throw logic_error("unknown ABI fact local context reference '" + id + "'");
  }
  return found->second;
}

const AbiEntity & require_entity_ref(const ParseContext & ctx, const string & id)
{
  map<string, AbiEntity>::const_iterator found = ctx.entities.find(id);
  if(found == ctx.entities.end()) {
    throw logic_error("unknown ABI fact entity reference '" + id + "'");
  }
  return found->second;
}

vector<Type::NameComponent> prefix_components_for_parts(const vector<string> & parts,
                                                        size_t terminal)
{
  vector<Type::NameComponent> out;
  for(size_t i = 0; i < terminal; ++i) {
    if(i == 0 && parts[i] == "std") {
      out.push_back(Type::NameComponent::std_namespace());
    } else {
      out.push_back(Type::NameComponent::source(parts[i],
                                                join_qualified_parts(parts, i + 1)));
    }
  }
  return out;
}

vector<FunctionNameComponent> function_components_for_qualified(const string & qualified_name,
                                                                bool include_terminal)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  vector<FunctionNameComponent> out;
  const size_t prefix_end = include_terminal ? parts.size() - 1 : parts.size();
  for(size_t i = 0; i < prefix_end; ++i) {
    if(i == 0 && parts[i] == "std") {
      out.push_back(FunctionNameComponent::std_namespace());
    } else {
      out.push_back(FunctionNameComponent::source(parts[i],
                                                  join_qualified_parts(parts, i + 1)));
    }
  }
  if(include_terminal) {
    out.push_back(FunctionNameComponent::source(parts.back(), string()));
  }
  return out;
}

Type named_type_from_qualified(const string & qualified_name)
{
  const vector<string> parts = split_qualified_name(qualified_name);
  Type out = Type::named_type(prefix_components_for_parts(parts, parts.size() - 1),
                              parts.back(),
                              join_qualified_parts(parts, parts.size()));
  // A named class/enum type is a substitutable component: give it a whole-type
  // substitution key so a repeated occurrence emits a back-reference (S_/S0_/...)
  // instead of re-spelling. The key matches the qualified substitution name the
  // trailing name-component registers during spelling, so emit_type's existing
  // pre-check/register handles compression consistently.
  set_substitution(out,
                   SubstitutionKey::named(join_qualified_parts(parts, parts.size())));
  return out;
}

vector<Type::ClassTemplateArgument> class_args_from_refs(const ParseContext & ctx,
                                                         const vector<string> & refs);
TemplateArgument template_arg_from_class_arg(const Type::ClassTemplateArgument & arg);
Type::ClassTemplateArgument class_arg_from_template_arg(const TemplateArgument & arg);
Type parse_type_spec(const ParseContext & ctx, const vector<string> & words, size_t begin);
Type parse_single_type_token(const ParseContext & ctx, const string & text);
DependentExpression parse_expression_fact(const ParseContext & ctx, const vector<string> & words);
string variable_symbol(const string & qualified_name);

void apply_local_context(FunctionEncoding::LambdaMetadata & lambda,
                         const LocalContext & context)
{
  lambda.context_fragment = context.context_fragment;
  lambda.context_substitution_slots = context.context_substitution_slots;
  if(context.context_function) {
    lambda.context_function.reset(new FunctionEncoding(*context.context_function));
  } else {
    lambda.context_function.reset();
  }
}

void apply_local_context(Type::LambdaMetadata & lambda,
                         const LocalContext & context)
{
  lambda.context_fragment = context.context_fragment;
  lambda.context_substitution_slots = context.context_substitution_slots;
  if(context.context_function) {
    lambda.context_function.reset(new FunctionEncoding(*context.context_function));
  } else {
    lambda.context_function.reset();
  }
}

vector<TemplateArgument> template_args_from_refs(const ParseContext & ctx,
                                                 const vector<string> & refs)
{
  vector<TemplateArgument> out;
  out.reserve(refs.size());
  for(size_t i = 0; i < refs.size(); ++i) {
    out.push_back(require_arg_ref(ctx, refs[i]));
  }
  return out;
}

vector<Type::ClassTemplateArgument> class_args_from_refs(const ParseContext & ctx,
                                                         const vector<string> & refs)
{
  vector<Type::ClassTemplateArgument> out;
  out.reserve(refs.size());
  for(size_t i = 0; i < refs.size(); ++i) {
    out.push_back(class_arg_from_template_arg(require_arg_ref(ctx, refs[i])));
  }
  return out;
}

vector<Type> parse_type_tokens(const ParseContext & ctx,
                               const vector<string> & words,
                               size_t begin)
{
  vector<Type> out;
  for(size_t i = begin; i < words.size(); ++i) {
    out.push_back(parse_single_type_token(ctx, words[i]));
  }
  return out;
}

Type::ClassTemplateArgument class_arg_from_template_arg(const TemplateArgument & arg)
{
  switch(arg.kind) {
  case TemplateArgument::TAK_TYPE:
    if(!arg.value_type) { break; }
    return Type::ClassTemplateArgument::type_arg(*arg.value_type);
  case TemplateArgument::TAK_INTEGRAL_VALUE:
    if(!arg.value_type) { break; }
    return Type::ClassTemplateArgument::integral_value_arg(*arg.value_type,
                                                           arg.integral_value);
  case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE:
    if(!arg.parameter_type) { break; }
    if(arg.value_type) {
      return Type::ClassTemplateArgument::dependent_integral_value_arg(
          *arg.parameter_type, *arg.value_type, arg.integral_value);
    }
    return Type::ClassTemplateArgument::dependent_untyped_integral_value_arg(
        *arg.parameter_type, arg.integral_value);
  case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
    if(arg.expression) {
      return Type::ClassTemplateArgument::dependent_expression_arg(*arg.expression);
    }
    break;
  case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
    return Type::ClassTemplateArgument::untyped_integral_value_arg(arg.integral_value);
  case TemplateArgument::TAK_TEMPLATE_ENTITY:
    if(!arg.metadata) { break; }
    if(arg.metadata->template_name_is_template_parameter) {
      return Type::ClassTemplateArgument::template_parameter_template_arg(
          arg.metadata->template_parameter_index);
    }
    if(arg.metadata->template_owner_type) {
      return Type::ClassTemplateArgument::member_template_entity_arg(
          *arg.metadata->template_owner_type,
          arg.metadata->template_name,
          arg.metadata->template_name_substitution);
    }
    return Type::ClassTemplateArgument::template_entity_arg(
        arg.metadata->prefix_components,
        arg.metadata->template_name,
        arg.metadata->template_name_substitution);
  case TemplateArgument::TAK_EXTERNAL_ENTITY:
    if(!arg.metadata) { break; }
    if(arg.metadata->external_entity_is_member &&
       arg.metadata->external_entity_owner_type) {
      return Type::ClassTemplateArgument::external_member_entity_arg(
          arg.metadata->external_entity_symbol,
          arg.metadata->external_entity_address_of,
          *arg.metadata->external_entity_owner_type,
          arg.metadata->external_entity_member_name,
          arg.metadata->external_entity_parameter_types,
          arg.metadata->external_entity_is_function,
          arg.metadata->external_entity_function_const,
          arg.metadata->external_entity_function_volatile,
          arg.metadata->external_entity_function_lvalue_ref,
          arg.metadata->external_entity_function_rvalue_ref,
          arg.metadata->external_entity_function_variadic);
    }
    return Type::ClassTemplateArgument::external_entity_arg(
        arg.metadata->external_entity_symbol,
        arg.metadata->external_entity_address_of);
  case TemplateArgument::TAK_ARGUMENT_PACK:
    if(arg.metadata) {
      vector<Type::ClassTemplateArgument> items;
      for(size_t i = 0; i < arg.metadata->pack_arguments.size(); ++i) {
        items.push_back(class_arg_from_template_arg(arg.metadata->pack_arguments[i]));
      }
      return Type::ClassTemplateArgument::argument_pack(items);
    }
    break;
  case TemplateArgument::TAK_INVALID:
    break;
  }
  throw logic_error("unable to convert ABI template argument to class-template argument");
}

TemplateArgument template_arg_from_class_arg(const Type::ClassTemplateArgument & arg)
{
  switch(arg.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE:
    if(!arg.type) { break; }
    return TemplateArgument::type_arg(*arg.type);
  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    if(!arg.type) { break; }
    return TemplateArgument::integral_value_arg(*arg.type, arg.integral_value);
  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!arg.parameter_type) { break; }
    if(arg.type) {
      return TemplateArgument::dependent_integral_value_arg(*arg.parameter_type,
                                                            *arg.type,
                                                            arg.integral_value);
    }
    return TemplateArgument::dependent_untyped_integral_value_arg(*arg.parameter_type,
                                                                  arg.integral_value);
  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(arg.expression) {
      return TemplateArgument::dependent_expression_arg(*arg.expression);
    }
    break;
  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    return TemplateArgument::untyped_integral_value_arg(arg.integral_value);
  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(!arg.metadata) { break; }
    if(arg.metadata->template_name_is_template_parameter) {
      return TemplateArgument::template_parameter_template_arg(
          arg.metadata->template_parameter_index);
    }
    if(arg.metadata->template_owner_type) {
      return TemplateArgument::member_template_entity_arg(
          *arg.metadata->template_owner_type,
          arg.metadata->template_name,
          arg.metadata->template_name_substitution);
    }
    return TemplateArgument::template_entity_arg(arg.metadata->prefix_components,
                                                 arg.metadata->template_name,
                                                 arg.metadata->template_name_substitution);
  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    if(!arg.metadata) { break; }
    if(arg.metadata->external_entity_is_member &&
       arg.metadata->external_entity_owner_type) {
      return TemplateArgument::external_member_entity_arg(
          arg.metadata->external_entity_symbol,
          arg.metadata->external_entity_address_of,
          *arg.metadata->external_entity_owner_type,
          arg.metadata->external_entity_member_name,
          arg.metadata->external_entity_parameter_types,
          arg.metadata->external_entity_is_function,
          arg.metadata->external_entity_function_const,
          arg.metadata->external_entity_function_volatile,
          arg.metadata->external_entity_function_lvalue_ref,
          arg.metadata->external_entity_function_rvalue_ref,
          arg.metadata->external_entity_function_variadic);
    }
    return TemplateArgument::external_entity_arg(arg.metadata->external_entity_symbol,
                                                 arg.metadata->external_entity_address_of);
  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(arg.metadata) {
      vector<TemplateArgument> items;
      for(size_t i = 0; i < arg.metadata->pack_arguments.size(); ++i) {
        items.push_back(template_arg_from_class_arg(arg.metadata->pack_arguments[i]));
      }
      return TemplateArgument::argument_pack(items);
    }
    break;
  case Type::ClassTemplateArgument::CTAK_INVALID:
    break;
  }
  throw logic_error("unable to convert ABI class-template argument");
}

string array_bound_text(const ParseContext & ctx, const string & word)
{
  if(starts_with(word, "raw:")) {
    return word.substr(4);
  }
  if(starts_with(word, "expr:")) {
    const DependentExpression & expr = require_expr_ref(ctx, word.substr(5));
    FactSubstitutionSink sink;
    string out;
    if(!emit_dependent_expression_body(expr, out, &sink)) {
      throw logic_error("unable to encode ABI fact array bound expression");
    }
    return out;
  }
  unsigned long long value = 0;
  if(!parse_unsigned_integer_word(word, value)) {
    throw logic_error("array bound must be an integer or expr:<reference>");
  }
  return word;
}

// cv-qualified, pointer, and reference types are Itanium substitution candidates:
// each must be given a whole-type substitution key so it is counted (advancing later
// seq-ids to match clang, which counts e.g. `const T` and `T&` as candidates) and is
// back-referenced on repeat. The `template`/`std-template` builders already do this,
// and codegen sets it on every constructed type; the plain cv/ref/pointer builders
// historically did not, so a fresh type following a cv/ref *parameter* mangled with a
// too-low S<n>_ (e.g. mixed_subst_one's inner std::__1 ref: NS1_ vs clang's NS3_).
// Only these qualifier/indirection builders need this — array/member-pointer/vendor/
// transform types reach the mangler as template arguments, which already register via
// the class-template-argument path; wrapping those too would double-count them.
// make_type_substitution_key returns false for non-substitutable kinds (builtins,
// bare template params), so the call is a no-op there.
Type with_type_substitution_key(Type out)
{
  SubstitutionKey key;
  if(make_type_substitution_key(out, key)) {
    set_substitution(out, key);
  }
  return out;
}

Type parse_single_type_token(const ParseContext & ctx, const string & text)
{
  map<string, Type>::const_iterator named_ref = ctx.types.find(text);
  if(named_ref != ctx.types.end()) {
    return named_ref->second;
  }
  if(starts_with(text, "ptr:")) {
    return with_type_substitution_key(
        Type::pointer(parse_single_type_token(ctx, text.substr(4))));
  }
  if(starts_with(text, "ref:")) {
    return with_type_substitution_key(
        Type::lvalue_reference(parse_single_type_token(ctx, text.substr(4))));
  }
  if(starts_with(text, "rref:")) {
    return with_type_substitution_key(
        Type::rvalue_reference(parse_single_type_token(ctx, text.substr(5))));
  }
  if(starts_with(text, "const:")) {
    return with_type_substitution_key(
        Type::cv(true, false, parse_single_type_token(ctx, text.substr(6))));
  }
  if(starts_with(text, "volatile:")) {
    return with_type_substitution_key(
        Type::cv(false, true, parse_single_type_token(ctx, text.substr(9))));
  }
  if(starts_with(text, "vendor:")) {
    const string rest = text.substr(7);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("vendor type requires vendor:<qualifier>:<operand>");
    }
    return Type::vendor_qualified(rest.substr(0, pos),
                                  parse_single_type_token(ctx, rest.substr(pos + 1)));
  }
  if(starts_with(text, "transform:")) {
    const string rest = text.substr(10);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("builtin transform type requires transform:<name>:<operand>");
    }
    return Type::builtin_type_transform(rest.substr(0, pos),
                                        parse_single_type_token(ctx, rest.substr(pos + 1)));
  }
  if(starts_with(text, "array:")) {
    const string rest = text.substr(6);
    const size_t pos = rest.find(':');
    if(pos == string::npos || pos == 0) {
      throw logic_error("array type requires array:<bound>:<element>");
    }
    return Type::array(array_bound_text(ctx, rest.substr(0, pos)),
                       parse_single_type_token(ctx, rest.substr(pos + 1)));
  }
  if(starts_with(text, "memberptr:")) {
    const string rest = text.substr(10);
    const size_t pos = rest.rfind(':');
    if(pos == string::npos || pos == 0 || pos + 1 >= rest.size()) {
      throw logic_error("member pointer type requires memberptr:<owner>:<member-type>");
    }
    const string owner_text = rest.substr(0, pos);
    Type owner = ctx.types.count(owner_text) ? require_type_ref(ctx, owner_text) :
                                               named_type_from_qualified(owner_text);
    return Type::member_pointer(owner,
                                parse_single_type_token(ctx, rest.substr(pos + 1)));
  }
  if(starts_with(text, "named:")) {
    return named_type_from_qualified(text.substr(6));
  }
  const string builtin = builtin_code_from_name(text);
  if(!builtin.empty()) {
    return Type::builtin(builtin);
  }
  return named_type_from_qualified(text);
}

Type parse_type_spec(const ParseContext & ctx, const vector<string> & words, size_t begin)
{
  if(begin >= words.size()) {
    throw logic_error("missing ABI fact type");
  }
  if(begin + 1 == words.size()) {
    return parse_single_type_token(ctx, words[begin]);
  }

  const string & kind = words[begin];
  if(kind == "template-param" || kind == "template-param-subst") {
    if(begin + 2 != words.size()) {
      throw logic_error("template-param type requires one index");
    }
    Type out = Type::template_parameter(parse_index(words[begin + 1]));
    if(kind == "template-param-subst") {
      set_substitution(out,
                       SubstitutionKey::type_template_parameter(
                           out.template_parameter_index));
    }
    return out;
  }
  if(kind == "ptr" || kind == "ref" || kind == "rref" ||
     kind == "const" || kind == "volatile" || kind == "const-volatile" ||
     kind == "cv" || kind == "pack") {
    if(begin + 2 != words.size()) {
      throw logic_error(kind + " type requires one operand");
    }
    Type child = parse_single_type_token(ctx, words[begin + 1]);
    if(kind == "ptr") { return with_type_substitution_key(Type::pointer(child)); }
    if(kind == "ref") { return with_type_substitution_key(Type::lvalue_reference(child)); }
    if(kind == "rref") { return with_type_substitution_key(Type::rvalue_reference(child)); }
    if(kind == "const") { return with_type_substitution_key(Type::cv(true, false, child)); }
    if(kind == "volatile") { return with_type_substitution_key(Type::cv(false, true, child)); }
    if(kind == "const-volatile" || kind == "cv") {
      return with_type_substitution_key(Type::cv(true, true, child));
    }
    return Type::pack_expansion(child);
  }
  if(kind == "vendor") {
    if(begin + 3 != words.size()) {
      throw logic_error("vendor type requires qualifier and operand");
    }
    return Type::vendor_qualified(words[begin + 1],
                                  parse_single_type_token(ctx, words[begin + 2]));
  }
  if(kind == "array") {
    if(begin + 3 != words.size()) {
      throw logic_error("array type requires bound and element type");
    }
    return Type::array(array_bound_text(ctx, words[begin + 1]),
                       parse_single_type_token(ctx, words[begin + 2]));
  }
  if(kind == "transform" || kind == "builtin-transform") {
    if(begin + 3 != words.size()) {
      throw logic_error("builtin-transform type requires name and operand");
    }
    return Type::builtin_type_transform(words[begin + 1],
                                        parse_single_type_token(ctx, words[begin + 2]));
  }
  if(kind == "function-type" || kind == "function-type-variadic") {
    if(begin + 2 >= words.size()) {
      throw logic_error("function-type requires a result type");
    }
    vector<Type> params;
    bool variadic = kind == "function-type-variadic";
    bool lvalue_ref = false;
    bool rvalue_ref = false;
    Type result = parse_single_type_token(ctx, words[begin + 1]);
    for(size_t i = begin + 2; i < words.size(); ++i) {
      if(words[i] == "variadic" || words[i] == "varargs") {
        variadic = true;
      } else if(words[i] == "lvalue-ref" || words[i] == "ref") {
        lvalue_ref = true;
      } else if(words[i] == "rvalue-ref" || words[i] == "rref") {
        rvalue_ref = true;
      } else {
        params.push_back(parse_single_type_token(ctx, words[i]));
      }
    }
    return Type::function(result, params, variadic, lvalue_ref, rvalue_ref);
  }
  if(kind == "member-pointer") {
    if(begin + 3 != words.size()) {
      throw logic_error("member-pointer type requires owner and member type");
    }
    return Type::member_pointer(parse_single_type_token(ctx, words[begin + 1]),
                                parse_single_type_token(ctx, words[begin + 2]));
  }
  if(kind == "name") {
    if(begin + 2 != words.size()) {
      throw logic_error("name type requires one qualified name");
    }
    return named_type_from_qualified(words[begin + 1]);
  }
  if(kind == "template") {
    if(begin + 2 > words.size()) {
      throw logic_error("template type requires a qualified template name");
    }
    vector<string> refs(words.begin() + begin + 2, words.end());
    const vector<string> parts = split_qualified_name(words[begin + 1]);
    Type out = Type::class_template_specialization(
        prefix_components_for_parts(parts, parts.size() - 1),
        parts.back(),
        join_qualified_parts(parts, parts.size()),
        class_args_from_refs(ctx, refs),
        string(),
        false);
    // A class-template specialization is a substitutable component: give it a
    // whole-type substitution key so a repeated occurrence back-references it
    // (e.g. RS4_) instead of re-spelling its components.
    SubstitutionKey key;
    if(make_type_substitution_key(out, key)) {
      set_substitution(out, key);
    }
    return out;
  }
  if(kind == "template-param-template") {
    if(begin + 3 > words.size()) {
      throw logic_error("template-param-template type requires index and template arguments");
    }
    vector<string> refs(words.begin() + begin + 2, words.end());
    return Type::template_parameter_class_template_specialization(
        parse_index(words[begin + 1]), class_args_from_refs(ctx, refs));
  }
  if(kind == "std-template") {
    if(begin + 4 > words.size()) {
      throw logic_error("std-template type requires substitution, includes flag, and name");
    }
    vector<string> refs(words.begin() + begin + 4, words.end());
    const vector<string> parts = split_qualified_name(words[begin + 3]);
    Type out = Type::class_template_specialization(
        prefix_components_for_parts(parts, parts.size() - 1),
        parts.back(),
        join_qualified_parts(parts, parts.size()),
        class_args_from_refs(ctx, refs),
        std_substitution_from_word(words[begin + 1]),
        boolean_word(words[begin + 2]));
    SubstitutionKey key;
    if(make_type_substitution_key(out, key)) {
      set_substitution(out, key);
    }
    return out;
  }
  if(kind == "member") {
    if(begin + 3 != words.size()) {
      throw logic_error("member type requires owner type and member name");
    }
    return Type::member_named_type(parse_single_type_token(ctx, words[begin + 1]),
                                   words[begin + 2],
                                   words[begin + 2]);
  }
  if(kind == "member-template") {
    if(begin + 3 > words.size()) {
      throw logic_error("member-template type requires owner type and name");
    }
    vector<string> refs(words.begin() + begin + 3, words.end());
    return Type::member_class_template_specialization(
        parse_single_type_token(ctx, words[begin + 1]),
        words[begin + 2],
        words[begin + 2],
        class_args_from_refs(ctx, refs));
  }
  if(kind == "decltype") {
    if(begin + 2 != words.size()) {
      throw logic_error("decltype type requires one expression reference");
    }
    Type out;
    out.kind = Type::TK_DECLTYPE_EXPRESSION;
    out.expression.reset(new DependentExpression(require_expr_ref(ctx, words[begin + 1])));
    SubstitutionKey key;
    if(make_type_substitution_key(out, key)) {
      set_substitution(out, key);
    }
    return out;
  }
  if(kind == "lambda-closure") {
    if(begin + 3 > words.size()) {
      throw logic_error("lambda-closure type requires context and discriminator");
    }
    Type out = Type::lambda_closure(string(),
                                    vector<SubstitutionSlot>(),
                                    shared_ptr<FunctionEncoding>(),
                                    parse_type_tokens(ctx, words, begin + 3),
                                    words[begin + 2]);
    apply_local_context(*out.lambda, require_context_ref(ctx, words[begin + 1]));
    return out;
  }
  if(kind == "local-type") {
    if(begin + 4 != words.size()) {
      throw logic_error("local-type requires context, source name, discriminator");
    }
    Type out = Type::lambda_closure(string(),
                                    vector<SubstitutionSlot>(),
                                    shared_ptr<FunctionEncoding>(),
                                    vector<Type>(),
                                    words[begin + 3],
                                    words[begin + 2]);
    apply_local_context(*out.lambda, require_context_ref(ctx, words[begin + 1]));
    return out;
  }
  throw logic_error("unknown ABI fact type kind '" + kind + "'");
}

TemplateArgument parse_template_argument_fact(const ParseContext & ctx,
                                              const vector<string> & words)
{
  if(words.size() < 4) {
    throw logic_error("let-arg requires id and argument");
  }
  const string & kind = words[2];
  if(kind == "type") {
    return TemplateArgument::type_arg(parse_type_spec(ctx, words, 3));
  }
  if(kind == "value") {
    if(words.size() != 5) {
      throw logic_error("value template argument requires type and integer");
    }
    return TemplateArgument::integral_value_arg(parse_single_type_token(ctx, words[3]),
                                                parse_signed_integer(words[4]));
  }
  if(kind == "dependent-value") {
    if(words.size() != 6) {
      throw logic_error("dependent-value template argument requires parameter type, value type or -, and integer");
    }
    Type parameter = parse_single_type_token(ctx, words[3]);
    if(words[4] == "-") {
      return TemplateArgument::dependent_untyped_integral_value_arg(
          parameter, parse_signed_integer(words[5]));
    }
    return TemplateArgument::dependent_integral_value_arg(
        parameter,
        parse_single_type_token(ctx, words[4]),
        parse_signed_integer(words[5]));
  }
  if(kind == "untyped-value") {
    if(words.size() != 4) {
      throw logic_error("untyped-value template argument requires an integer");
    }
    return TemplateArgument::untyped_integral_value_arg(parse_signed_integer(words[3]));
  }
  if(kind == "expression") {
    if(words.size() != 4) {
      throw logic_error("expression template argument requires one expression");
    }
    return TemplateArgument::dependent_expression_arg(require_expr_ref(ctx, words[3]));
  }
  if(kind == "template-entity") {
    if(words.size() != 4) {
      throw logic_error("template-entity template argument requires a qualified name");
    }
    const vector<string> parts = split_qualified_name(words[3]);
    return TemplateArgument::template_entity_arg(
        prefix_components_for_parts(parts, parts.size() - 1),
        parts.back(),
        join_qualified_parts(parts, parts.size()));
  }
  if(kind == "member-template-entity") {
    if(words.size() < 5 || words.size() > 6) {
      throw logic_error("member-template-entity template argument requires owner type, member name, and optional substitution");
    }
    return TemplateArgument::member_template_entity_arg(
        parse_single_type_token(ctx, words[3]),
        words[4],
        words.size() == 6 ? dash_empty(words[5]) : words[4]);
  }
  if(kind == "template-param-template") {
    if(words.size() != 4) {
      throw logic_error("template-param-template template argument requires one index");
    }
    return TemplateArgument::template_parameter_template_arg(parse_index(words[3]));
  }
  if(kind == "external-address" || kind == "external-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " template argument requires a raw symbol");
    }
    return TemplateArgument::external_entity_arg(words[3], kind == "external-address");
  }
  if(kind == "member-external-address" || kind == "member-external-reference") {
    if(words.size() < 12) {
      throw logic_error(kind + " template argument requires symbol, owner, member, function flag, cv/ref flags, variadic flag, and optional parameters");
    }
    return TemplateArgument::external_member_entity_arg(
        words[3],
        kind == "member-external-address",
        parse_single_type_token(ctx, words[4]),
        words[5],
        parse_type_tokens(ctx, words, 12),
        boolean_word(words[6]),
        boolean_word(words[7]),
        boolean_word(words[8]),
        boolean_word(words[9]),
        boolean_word(words[10]),
        boolean_word(words[11]));
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(words.size() != 4) {
      throw logic_error(kind + " template argument requires one entity");
    }
    const AbiEntity & entity = require_entity_ref(ctx, words[3]);
    string symbol = entity.qualified_name;
    if(entity.kind == ABI_ENTITY_FUNCTION) {
      symbol.clear();
      FactSubstitutionSink sink;
      if(!emit_function_encoding(entity.function, symbol, &sink)) {
        throw logic_error("unable to encode ABI fact function entity");
      }
    } else if(entity.kind == ABI_ENTITY_VARIABLE) {
      symbol = variable_symbol(entity.qualified_name);
    }
    return TemplateArgument::external_entity_arg(symbol, kind == "entity-address");
  }
  if(kind == "pack") {
    vector<TemplateArgument> items;
    for(size_t i = 3; i < words.size(); ++i) {
      items.push_back(require_arg_ref(ctx, words[i]));
    }
    return TemplateArgument::argument_pack(items);
  }
  throw logic_error("unknown ABI fact template argument kind '" + kind + "'");
}

DependentExpression parse_expression_fact(const ParseContext & ctx,
                                          const vector<string> & words)
{
  if(words.size() < 3) {
    throw logic_error("let-expr requires id and expression");
  }
  const string & kind = words[2];
  if(kind == "template-param") {
    if(words.size() != 4) { throw logic_error("template-param expression requires one index"); }
    return DependentExpression::template_parameter(parse_index(words[3]));
  }
  if(kind == "function-param") {
    if(words.size() != 4) { throw logic_error("function-param expression requires one index"); }
    return DependentExpression::function_parameter(parse_index(words[3]));
  }
  if(kind == "literal") {
    if(words.size() != 4) { throw logic_error("literal expression requires one value"); }
    return DependentExpression::literal(words[3]);
  }
  if(kind == "integral-value") {
    if(words.size() != 5) { throw logic_error("integral-value expression requires type and value"); }
    return DependentExpression::typed_integral_value(parse_single_type_token(ctx, words[3]),
                                                     parse_signed_integer(words[4]));
  }
  if(kind == "unary") {
    if(words.size() != 5) { throw logic_error("unary expression requires operator and operand"); }
    return DependentExpression::unary(words[3], require_expr_ref(ctx, words[4]));
  }
  if(kind == "binary") {
    if(words.size() != 6) { throw logic_error("binary expression requires operator and operands"); }
    return DependentExpression::binary(words[3],
                                       require_expr_ref(ctx, words[4]),
                                       require_expr_ref(ctx, words[5]));
  }
  if(kind == "conditional") {
    if(words.size() != 6) { throw logic_error("conditional expression requires three operands"); }
    return DependentExpression::conditional(require_expr_ref(ctx, words[3]),
                                            require_expr_ref(ctx, words[4]),
                                            require_expr_ref(ctx, words[5]));
  }
  if(kind == "pack") {
    if(words.size() != 4) { throw logic_error("pack expression requires one operand"); }
    return DependentExpression::pack_expansion(require_expr_ref(ctx, words[3]));
  }
  if(kind == "call") {
    if(words.size() < 4) { throw logic_error("call expression requires a callee"); }
    vector<DependentExpression> args;
    for(size_t i = 4; i < words.size(); ++i) {
      args.push_back(require_expr_ref(ctx, words[i]));
    }
    return DependentExpression::call(require_expr_ref(ctx, words[3]), args);
  }
  if(kind == "conversion") {
    if(words.size() < 4) { throw logic_error("conversion expression requires type"); }
    vector<DependentExpression> args;
    for(size_t i = 4; i < words.size(); ++i) {
      args.push_back(require_expr_ref(ctx, words[i]));
    }
    return DependentExpression::conversion(parse_single_type_token(ctx, words[3]), args);
  }
  if(kind == "cast") {
    if(words.size() != 6) { throw logic_error("cast expression requires operator, type, and operand"); }
    return DependentExpression::cast(words[3],
                                     parse_single_type_token(ctx, words[4]),
                                     require_expr_ref(ctx, words[5]));
  }
  if(kind == "template-id") {
    if(words.size() < 4) { throw logic_error("template-id expression requires name and arguments"); }
    vector<string> refs(words.begin() + 4, words.end());
    return DependentExpression::template_id(words[3], template_args_from_refs(ctx, refs));
  }
  if(kind == "type-trait") {
    if(words.size() < 4) { throw logic_error("type-trait expression requires name and types"); }
    return DependentExpression::type_trait(words[3], parse_type_tokens(ctx, words, 4));
  }
  if(kind == "sizeof-type") {
    if(words.size() < 4) { throw logic_error("sizeof-type expression requires a type"); }
    return DependentExpression::sizeof_type(parse_type_spec(ctx, words, 3));
  }
  if(kind == "member") {
    if(words.size() < 6) { throw logic_error("member expression requires owner type, close flag, name, and optional template arguments"); }
    DependentExpression out = DependentExpression::member(
        parse_single_type_token(ctx, words[3]), boolean_word(words[4]), words[5]);
    vector<string> refs(words.begin() + 6, words.end());
    out.template_arguments = template_args_from_refs(ctx, refs);
    return out;
  }
  if(kind == "object-member") {
    if(words.size() < 6) { throw logic_error("object-member expression requires operator, object, member name, and optional template arguments"); }
    vector<string> refs(words.begin() + 6, words.end());
    return DependentExpression::object_member(words[3],
                                              require_expr_ref(ctx, words[4]),
                                              words[5],
                                              template_args_from_refs(ctx, refs));
  }
  if(kind == "external-address" || kind == "external-reference") {
    if(words.size() != 4) { throw logic_error(kind + " expression requires a symbol"); }
    return DependentExpression::external_entity(words[3], kind == "external-address");
  }
  if(kind == "entity-address" || kind == "entity-reference") {
    if(words.size() != 4) { throw logic_error(kind + " expression requires an entity"); }
    const AbiEntity & entity = require_entity_ref(ctx, words[3]);
    string symbol = entity.qualified_name;
    if(entity.kind == ABI_ENTITY_FUNCTION) {
      symbol.clear();
      FactSubstitutionSink sink;
      if(!emit_function_encoding(entity.function, symbol, &sink)) {
        throw logic_error("unable to encode ABI fact function entity");
      }
    } else if(entity.kind == ABI_ENTITY_VARIABLE) {
      symbol = variable_symbol(entity.qualified_name);
    }
    return DependentExpression::external_entity(symbol, kind == "entity-address");
  }
  throw logic_error("unknown ABI fact expression kind '" + kind + "'");
}

FunctionOperatorTerminal terminal_from_fact_word(const string & word)
{
  if(word == "operator-call") { return FUNCTION_OPERATOR_CALL; }
  if(word == "operator-assign") { return FUNCTION_OPERATOR_ASSIGN; }
  if(word == "constructor-complete") { return FUNCTION_OPERATOR_NONE; }
  if(word == "constructor-base") { return FUNCTION_OPERATOR_NONE; }
  if(word == "destructor-complete") { return FUNCTION_OPERATOR_NONE; }
  if(word == "destructor-base") { return FUNCTION_OPERATOR_NONE; }
  if(word == "destructor-deleting") { return FUNCTION_OPERATOR_NONE; }
  throw logic_error("unknown ABI fact terminal name '" + word + "'");
}

string terminal_fragment_from_fact_word(const string & word)
{
  if(word == "constructor-complete") { return "C1"; }
  if(word == "constructor-base") { return "C2"; }
  if(word == "destructor-complete") { return "D1"; }
  if(word == "destructor-base") { return "D2"; }
  if(word == "destructor-deleting") { return "D0"; }
  return string();
}

string fact_word_from_terminal_fragment(const string & fragment)
{
  if(fragment == "C1") { return "constructor-complete"; }
  if(fragment == "C2") { return "constructor-base"; }
  if(fragment == "D1") { return "destructor-complete"; }
  if(fragment == "D2") { return "destructor-base"; }
  if(fragment == "D0") { return "destructor-deleting"; }
  return string();
}

void apply_terminal_word(FunctionEncoding & function, const string & word)
{
  const string fragment = terminal_fragment_from_fact_word(word);
  if(!fragment.empty()) {
    function.terminal_fragment = fragment;
    return;
  }
  function.operator_terminal = terminal_from_fact_word(word);
}

bool operator_terminal_from_semantic_name(const string & name,
                                          FunctionOperatorTerminal & out,
                                          string & literal_suffix)
{
  if(name == "new") { out = FUNCTION_OPERATOR_NEW; return true; }
  if(name == "new-array") { out = FUNCTION_OPERATOR_NEW_ARRAY; return true; }
  if(name == "delete") { out = FUNCTION_OPERATOR_DELETE; return true; }
  if(name == "delete-array") { out = FUNCTION_OPERATOR_DELETE_ARRAY; return true; }
  if(name == "unary-plus") { out = FUNCTION_OPERATOR_UNARY_PLUS; return true; }
  if(name == "plus" || name == "binary-plus") { out = FUNCTION_OPERATOR_PLUS; return true; }
  if(name == "unary-minus") { out = FUNCTION_OPERATOR_UNARY_MINUS; return true; }
  if(name == "minus" || name == "binary-minus") { out = FUNCTION_OPERATOR_MINUS; return true; }
  if(name == "address-of" || name == "bit-and") { out = FUNCTION_OPERATOR_ADDRESS_OF; return true; }
  if(name == "deref") { out = FUNCTION_OPERATOR_DEREFERENCE; return true; }
  if(name == "multiply") { out = FUNCTION_OPERATOR_MULTIPLY; return true; }
  if(name == "divide") { out = FUNCTION_OPERATOR_DIVIDE; return true; }
  if(name == "remainder") { out = FUNCTION_OPERATOR_REMAINDER; return true; }
  if(name == "bit-or") { out = FUNCTION_OPERATOR_BIT_OR; return true; }
  if(name == "bit-xor") { out = FUNCTION_OPERATOR_BIT_XOR; return true; }
  if(name == "assign") { out = FUNCTION_OPERATOR_ASSIGN; return true; }
  if(name == "plus-assign") { out = FUNCTION_OPERATOR_PLUS_ASSIGN; return true; }
  if(name == "minus-assign") { out = FUNCTION_OPERATOR_MINUS_ASSIGN; return true; }
  if(name == "multiply-assign") { out = FUNCTION_OPERATOR_MULTIPLY_ASSIGN; return true; }
  if(name == "divide-assign") { out = FUNCTION_OPERATOR_DIVIDE_ASSIGN; return true; }
  if(name == "remainder-assign") { out = FUNCTION_OPERATOR_REMAINDER_ASSIGN; return true; }
  if(name == "bit-and-assign") { out = FUNCTION_OPERATOR_BIT_AND_ASSIGN; return true; }
  if(name == "bit-or-assign") { out = FUNCTION_OPERATOR_BIT_OR_ASSIGN; return true; }
  if(name == "bit-xor-assign") { out = FUNCTION_OPERATOR_BIT_XOR_ASSIGN; return true; }
  if(name == "shift-left") { out = FUNCTION_OPERATOR_SHIFT_LEFT; return true; }
  if(name == "shift-right") { out = FUNCTION_OPERATOR_SHIFT_RIGHT; return true; }
  if(name == "shift-left-assign") { out = FUNCTION_OPERATOR_SHIFT_LEFT_ASSIGN; return true; }
  if(name == "shift-right-assign") { out = FUNCTION_OPERATOR_SHIFT_RIGHT_ASSIGN; return true; }
  if(name == "equal") { out = FUNCTION_OPERATOR_EQUAL; return true; }
  if(name == "not-equal") { out = FUNCTION_OPERATOR_NOT_EQUAL; return true; }
  if(name == "less") { out = FUNCTION_OPERATOR_LESS; return true; }
  if(name == "greater") { out = FUNCTION_OPERATOR_GREATER; return true; }
  if(name == "less-equal") { out = FUNCTION_OPERATOR_LESS_EQUAL; return true; }
  if(name == "greater-equal") { out = FUNCTION_OPERATOR_GREATER_EQUAL; return true; }
  if(name == "logical-not") { out = FUNCTION_OPERATOR_LOGICAL_NOT; return true; }
  if(name == "bit-not") { out = FUNCTION_OPERATOR_BIT_NOT; return true; }
  if(name == "logical-and") { out = FUNCTION_OPERATOR_LOGICAL_AND; return true; }
  if(name == "logical-or") { out = FUNCTION_OPERATOR_LOGICAL_OR; return true; }
  if(name == "increment") { out = FUNCTION_OPERATOR_INCREMENT; return true; }
  if(name == "decrement") { out = FUNCTION_OPERATOR_DECREMENT; return true; }
  if(name == "comma") { out = FUNCTION_OPERATOR_COMMA; return true; }
  if(name == "member-pointer") { out = FUNCTION_OPERATOR_MEMBER_POINTER; return true; }
  if(name == "arrow") { out = FUNCTION_OPERATOR_ARROW; return true; }
  if(name == "call") { out = FUNCTION_OPERATOR_CALL; return true; }
  if(name == "index") { out = FUNCTION_OPERATOR_INDEX; return true; }
  if(name == "literal") { out = FUNCTION_OPERATOR_LITERAL; return true; }
  return false;
}

bool semantic_name_from_operator_terminal(FunctionOperatorTerminal terminal,
                                          string & out)
{
  switch(terminal) {
  case FUNCTION_OPERATOR_NONE: return false;
  case FUNCTION_OPERATOR_NEW: out = "new"; return true;
  case FUNCTION_OPERATOR_NEW_ARRAY: out = "new-array"; return true;
  case FUNCTION_OPERATOR_DELETE: out = "delete"; return true;
  case FUNCTION_OPERATOR_DELETE_ARRAY: out = "delete-array"; return true;
  case FUNCTION_OPERATOR_UNARY_PLUS: out = "unary-plus"; return true;
  case FUNCTION_OPERATOR_PLUS: out = "plus"; return true;
  case FUNCTION_OPERATOR_UNARY_MINUS: out = "unary-minus"; return true;
  case FUNCTION_OPERATOR_MINUS: out = "minus"; return true;
  case FUNCTION_OPERATOR_ADDRESS_OF: out = "address-of"; return true;
  case FUNCTION_OPERATOR_BIT_AND: out = "bit-and"; return true;
  case FUNCTION_OPERATOR_DEREFERENCE: out = "deref"; return true;
  case FUNCTION_OPERATOR_MULTIPLY: out = "multiply"; return true;
  case FUNCTION_OPERATOR_DIVIDE: out = "divide"; return true;
  case FUNCTION_OPERATOR_REMAINDER: out = "remainder"; return true;
  case FUNCTION_OPERATOR_BIT_OR: out = "bit-or"; return true;
  case FUNCTION_OPERATOR_BIT_XOR: out = "bit-xor"; return true;
  case FUNCTION_OPERATOR_ASSIGN: out = "assign"; return true;
  case FUNCTION_OPERATOR_PLUS_ASSIGN: out = "plus-assign"; return true;
  case FUNCTION_OPERATOR_MINUS_ASSIGN: out = "minus-assign"; return true;
  case FUNCTION_OPERATOR_MULTIPLY_ASSIGN: out = "multiply-assign"; return true;
  case FUNCTION_OPERATOR_DIVIDE_ASSIGN: out = "divide-assign"; return true;
  case FUNCTION_OPERATOR_REMAINDER_ASSIGN: out = "remainder-assign"; return true;
  case FUNCTION_OPERATOR_BIT_AND_ASSIGN: out = "bit-and-assign"; return true;
  case FUNCTION_OPERATOR_BIT_OR_ASSIGN: out = "bit-or-assign"; return true;
  case FUNCTION_OPERATOR_BIT_XOR_ASSIGN: out = "bit-xor-assign"; return true;
  case FUNCTION_OPERATOR_SHIFT_LEFT: out = "shift-left"; return true;
  case FUNCTION_OPERATOR_SHIFT_RIGHT: out = "shift-right"; return true;
  case FUNCTION_OPERATOR_SHIFT_LEFT_ASSIGN: out = "shift-left-assign"; return true;
  case FUNCTION_OPERATOR_SHIFT_RIGHT_ASSIGN: out = "shift-right-assign"; return true;
  case FUNCTION_OPERATOR_EQUAL: out = "equal"; return true;
  case FUNCTION_OPERATOR_NOT_EQUAL: out = "not-equal"; return true;
  case FUNCTION_OPERATOR_LESS: out = "less"; return true;
  case FUNCTION_OPERATOR_GREATER: out = "greater"; return true;
  case FUNCTION_OPERATOR_LESS_EQUAL: out = "less-equal"; return true;
  case FUNCTION_OPERATOR_GREATER_EQUAL: out = "greater-equal"; return true;
  case FUNCTION_OPERATOR_LOGICAL_NOT: out = "logical-not"; return true;
  case FUNCTION_OPERATOR_BIT_NOT: out = "bit-not"; return true;
  case FUNCTION_OPERATOR_LOGICAL_AND: out = "logical-and"; return true;
  case FUNCTION_OPERATOR_LOGICAL_OR: out = "logical-or"; return true;
  case FUNCTION_OPERATOR_INCREMENT: out = "increment"; return true;
  case FUNCTION_OPERATOR_DECREMENT: out = "decrement"; return true;
  case FUNCTION_OPERATOR_COMMA: out = "comma"; return true;
  case FUNCTION_OPERATOR_MEMBER_POINTER: out = "member-pointer"; return true;
  case FUNCTION_OPERATOR_ARROW: out = "arrow"; return true;
  case FUNCTION_OPERATOR_CALL: out = "call"; return true;
  case FUNCTION_OPERATOR_INDEX: out = "index"; return true;
  case FUNCTION_OPERATOR_LITERAL: out = "literal"; return true;
  }
  return false;
}

void parse_function_path(FunctionEncoding & function,
                         const ParseContext & ctx,
                         const vector<string> & words,
                         size_t name_index)
{
  if(name_index >= words.size()) {
    throw logic_error("function path requires a qualified name");
  }
  const vector<string> parts = split_qualified_name(words[name_index]);
  const string qualified_name = join_qualified_parts(parts, parts.size());
  function.name_components = function_components_for_qualified(words[name_index], true);
  vector<string> refs;
  for(size_t i = name_index + 1; i < words.size(); ++i) {
    if(words[i] == "result") {
      if(i + 1 >= words.size()) {
        throw logic_error("function path result requires a type");
      }
      function.has_result_type = true;
      function.result_type.reset(
          new Type(parse_single_type_token(ctx, words[++i])));
    } else if(words[i] == "variadic" || words[i] == "varargs") {
      function.variadic = true;
    } else if(ctx.args.count(words[i])) {
      refs.push_back(words[i]);
    } else {
      function.parameter_types.push_back(parse_single_type_token(ctx, words[i]));
    }
  }
  if(!refs.empty()) {
    const vector<TemplateArgument> arguments = template_args_from_refs(ctx, refs);
    if(function.name_components.empty()) {
      function.template_arguments = arguments;
    } else {
      string complete_name = qualified_name + "<";
      for(size_t i = 0; i < refs.size(); ++i) {
        if(i != 0) {
          complete_name += ",";
        }
        complete_name += refs[i];
      }
      complete_name += ">";
      function.name_components.back() =
          FunctionNameComponent::template_component(parts.back(),
                                                    qualified_name,
                                                    complete_name,
                                                    arguments,
                                                    string(),
                                                    false);
    }
  }
}

void parse_function_target(FunctionEncoding & function,
                           string & c_qualified_name,
                           const ParseContext & ctx,
                           const vector<string> & words,
                           size_t begin,
                           const string & command)
{
  if(begin >= words.size()) {
    throw logic_error(command + " fact requires a function name form");
  }
  if(words[begin] == "function") {
    ++begin;
  }
  if(begin >= words.size()) {
    throw logic_error(command + " fact requires a function name form");
  }
  if(words[begin] == "encoding" || words[begin] == "model") {
    return;
  }
  if(words[begin] == "path") {
    if(begin + 1 >= words.size()) {
      throw logic_error(command + " function path requires a qualified name");
    }
    c_qualified_name = words[begin + 1];
    parse_function_path(function, ctx, words, begin + 1);
    return;
  }
  if(words[begin] == "lambda") {
    if(begin + 3 >= words.size()) {
      throw logic_error(command + " function lambda requires context, discriminator, and terminal");
    }
    FunctionEncoding::LambdaMetadata & lambda =
        FunctionEncoding::ensure_lambda_metadata(function);
    apply_local_context(lambda, require_context_ref(ctx, words[begin + 1]));
    lambda.discriminator = words[begin + 2];
    apply_terminal_word(function, words[begin + 3]);
    lambda.signature_parameter_types = parse_type_tokens(ctx, words, begin + 4);
    return;
  }
  if(words[begin] == "local") {
    if(begin + 3 >= words.size() || begin + 5 < words.size()) {
      throw logic_error(command + " function local requires context, source name, terminal, and optional discriminator");
    }
    FunctionEncoding::LambdaMetadata & lambda =
        FunctionEncoding::ensure_lambda_metadata(function);
    apply_local_context(lambda, require_context_ref(ctx, words[begin + 1]));
    lambda.source_name = words[begin + 2];
    lambda.discriminator = begin + 4 < words.size() ? words[begin + 4] : "0";
    apply_terminal_word(function, words[begin + 3]);
    return;
  }

  c_qualified_name = words[begin];
  parse_function_path(function, ctx, words, begin);
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

FunctionEncoding & target_function(AbiFactCase & fact_case)
{
  if(!target_has_function(fact_case.target.kind)) {
    throw logic_error("function modifier appears before function fact");
  }
  return fact_case.target.function;
}

void apply_fact_words(AbiFactCase & fact_case,
                      ParseContext & ctx,
                      const vector<string> & words)
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
    fact.type = parse_type_spec(ctx, words, 2);
    ctx.types[fact.id] = fact.type;
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-arg") {
    AbiFact fact;
    fact.kind = ABI_FACT_TEMPLATE_ARGUMENT;
    fact.id = words[1];
    fact.template_argument = parse_template_argument_fact(ctx, words);
    ctx.args[fact.id] = fact.template_argument;
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-expr") {
    AbiFact fact;
    fact.kind = ABI_FACT_EXPRESSION;
    fact.id = words[1];
    fact.expression = parse_expression_fact(ctx, words);
    ctx.exprs[fact.id] = fact.expression;
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "let-context") {
    if(words.size() < 4) {
      throw logic_error("let-context requires id and context data");
    }
    AbiFact fact;
    fact.kind = ABI_FACT_LOCAL_CONTEXT;
    fact.id = words[1];
    if(words[2] == "raw") {
      if(words.size() != 4) {
        throw logic_error("let-context raw requires one context fragment");
      }
      fact.context = LocalContext::raw(words[3]);
    } else if(words[2] == "function") {
      parse_function_target(fact.context_function,
                            fact.entity.qualified_name,
                            ctx,
                            words,
                            3,
                            command);
      fact.context = LocalContext::function(fact.context_function);
    } else {
      throw logic_error("let-context requires raw or function context data");
    }
    ctx.contexts[fact.id] = fact.context;
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
      parse_function_target(fact.entity.function,
                            fact.entity.qualified_name,
                            ctx,
                            words,
                            3,
                            command);
    } else if(words[2] == "variable") {
      if(words.size() != 4) {
        throw logic_error("let-entity variable requires a qualified name");
      }
      fact.entity.kind = ABI_ENTITY_VARIABLE;
      fact.entity.qualified_name = words[3];
    } else if(words[2] == "symbol") {
      if(words.size() != 4) {
        throw logic_error("let-entity symbol requires a raw symbol");
      }
      fact.entity.kind = ABI_ENTITY_SYMBOL;
      fact.entity.qualified_name = words[3];
    } else {
      throw logic_error("unknown ABI fact entity kind '" + words[2] + "'");
    }
    ctx.entities[fact.id] = fact.entity;
    fact_case.facts.push_back(fact);
    return;
  }
  if(command == "type") {
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_TYPE;
    fact_case.target.type = parse_type_spec(ctx, words, 1);
    return;
  }
  if(command == "typeinfo" || command == "vtable" || command == "vtt") {
    require_no_target(fact_case);
    fact_case.target.kind = command == "typeinfo" ? ABI_MANGLE_TYPEINFO :
                            command == "vtable" ? ABI_MANGLE_VTABLE :
                                                   ABI_MANGLE_VTT;
    fact_case.target.type = parse_type_spec(ctx, words, 1);
    return;
  }
  if(command == "construction-vtable") {
    if(words.size() != 4) {
      throw logic_error("construction-vtable requires dynamic type, base offset, and base type");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_CONSTRUCTION_VTABLE;
    fact_case.target.type = parse_single_type_token(ctx, words[1]);
    if(!parse_unsigned_integer_word(words[2], fact_case.target.base_offset)) {
      throw logic_error("construction-vtable base offset must be decimal");
    }
    fact_case.target.base_type = parse_single_type_token(ctx, words[3]);
    return;
  }
  if(command == "tls-wrapper" || command == "thread-local-wrapper") {
    if(words.size() != 3 || (words[1] != "variable" && words[1] != "c-variable")) {
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
      throw logic_error("thunk requires this adjustment, optional result adjustment, and a function target");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_THUNK;
    fact_case.target.this_adjust = parse_signed_integer(words[1]);
    size_t function_begin = 2;
    if(words[function_begin] != "function") {
      fact_case.target.has_result_adjust = true;
      fact_case.target.result_adjust = parse_signed_integer(words[function_begin]);
      ++function_begin;
    }
    parse_function_target(fact_case.target.function,
                          fact_case.target.qualified_name,
                          ctx,
                          words,
                          function_begin,
                          command);
    return;
  }
  if(command == "virtual-base-thunk") {
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_VIRTUAL_BASE_THUNK;
    fact_case.target.vcall_offset = parse_signed_integer(words[1]);
    parse_function_target(fact_case.target.function,
                          fact_case.target.qualified_name,
                          ctx,
                          words,
                          2,
                          command);
    return;
  }
  if(command == "variable" || command == "c-variable") {
    if(words.size() != 2 && !(words.size() == 3 && words[1] == "path")) {
      throw logic_error(command + " requires a qualified name");
    }
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_VARIABLE;
    fact_case.target.c_linkage = command == "c-variable";
    fact_case.target.qualified_name = words.size() == 3 ? words[2] : words[1];
    return;
  }
  if(command == "function" || command == "c-function") {
    require_no_target(fact_case);
    fact_case.target.kind = ABI_MANGLE_FUNCTION;
    fact_case.target.c_linkage = command == "c-function";
    parse_function_target(fact_case.target.function,
                          fact_case.target.qualified_name,
                          ctx,
                          words,
                          1,
                          command);
    return;
  }
  if(command == "name-source") {
    if(words.size() < 2 || words.size() > 3) {
      throw logic_error("name-source requires source name and optional substitution");
    }
    target_function(fact_case).name_components.push_back(
        FunctionNameComponent::source(words[1], words.size() == 3 ? dash_empty(words[2]) : string()));
    return;
  }
  if(command == "name-std") {
    target_function(fact_case).name_components.push_back(FunctionNameComponent::std_namespace());
    return;
  }
  if(command == "name-template") {
    if(words.size() < 6) {
      throw logic_error("name-template requires source, substitution, complete substitution, std substitution, includes flag, and optional args");
    }
    vector<string> refs(words.begin() + 6, words.end());
    target_function(fact_case).name_components.push_back(
        FunctionNameComponent::template_component(words[1],
                                                  dash_empty(words[2]),
                                                  dash_empty(words[3]),
                                                  template_args_from_refs(ctx, refs),
                                                  std_substitution_from_word(words[4]),
                                                  boolean_word(words[5])));
    return;
  }
  if(command == "template-arg" || command == "function-template-arg") {
    if(words.size() != 2) {
      throw logic_error(command + " requires one template argument reference");
    }
    target_function(fact_case).template_arguments.push_back(require_arg_ref(ctx, words[1]));
    return;
  }
  if(command == "function-template-prefix") {
    if(words.size() != 2) {
      throw logic_error("function-template-prefix requires one substitution key");
    }
    target_function(fact_case).template_prefix_key =
        SubstitutionKey::function_template_prefix(dash_empty(words[1]));
    return;
  }
  if(command == "local-context") {
    if(words.size() != 4) {
      throw logic_error("local-context requires context, source name, and discriminator");
    }
    FunctionEncoding::LambdaMetadata & lambda =
        FunctionEncoding::ensure_lambda_metadata(target_function(fact_case));
    apply_local_context(lambda, require_context_ref(ctx, words[1]));
    lambda.source_name = words[2];
    lambda.discriminator = words[3];
    return;
  }
  if(command == "lambda-context") {
    if(words.size() < 3) {
      throw logic_error("lambda-context requires context and discriminator");
    }
    FunctionEncoding::LambdaMetadata & lambda =
        FunctionEncoding::ensure_lambda_metadata(target_function(fact_case));
    apply_local_context(lambda, require_context_ref(ctx, words[1]));
    lambda.discriminator = words[2];
    lambda.signature_parameter_types = parse_type_tokens(ctx, words, 3);
    return;
  }
  if(command == "terminal-source") {
    if(words.size() != 2) {
      throw logic_error("terminal-source requires one source name");
    }
    target_function(fact_case).terminal_source_name = words[1];
    return;
  }
  if(command == "terminal") {
    if(words.size() != 2) {
      throw logic_error("terminal requires one terminal word");
    }
    apply_terminal_word(target_function(fact_case), words[1]);
    return;
  }
  if(command == "variadic" || command == "varargs") {
    target_function(fact_case).variadic = true;
    return;
  }
  if(command == "abi-tag") {
    if(words.size() != 2) {
      throw logic_error("abi-tag requires one tag");
    }
    target_function(fact_case).abi_tags.push_back(words[1]);
    return;
  }
  if(command == "function-qualifier" || command == "qualifier") {
    FunctionEncoding & function = target_function(fact_case);
    for(size_t i = 1; i < words.size(); ++i) {
      if(words[i] == "const") { function.nested_const = true; }
      else if(words[i] == "volatile") { function.nested_volatile = true; }
      else if(words[i] == "lvalue-ref" || words[i] == "ref") { function.nested_lvalue_ref = true; }
      else if(words[i] == "rvalue-ref" || words[i] == "rref") { function.nested_rvalue_ref = true; }
      else { throw logic_error("unknown function qualifier '" + words[i] + "'"); }
    }
    return;
  }
  if(command == "operator-terminal") {
    if(words.size() < 2 || words.size() > 3) {
      throw logic_error("operator-terminal requires an operator name");
    }
    FunctionEncoding & function = target_function(fact_case);
    string literal_suffix;
    if(!operator_terminal_from_semantic_name(words[1], function.operator_terminal, literal_suffix)) {
      throw logic_error("unknown operator-terminal '" + words[1] + "'");
    }
    if(words[1] == "literal") {
      if(words.size() != 3) {
        throw logic_error("literal operator-terminal requires a suffix");
      }
      function.operator_literal_suffix = words[2];
    } else if(words.size() != 2) {
      throw logic_error("operator-terminal suffix is only valid for literal");
    }
    return;
  }
  if(command == "conversion-terminal") {
    FunctionEncoding & function = target_function(fact_case);
    function.has_conversion_type = true;
    function.conversion_type.reset(new Type(parse_type_spec(ctx, words, 1)));
    return;
  }
  if(command == "param") {
    target_function(fact_case).parameter_types.push_back(parse_type_spec(ctx, words, 1));
    return;
  }
  if(command == "result") {
    FunctionEncoding & function = target_function(fact_case);
    function.has_result_type = true;
    function.result_type.reset(new Type(parse_type_spec(ctx, words, 1)));
    return;
  }
  throw logic_error("unknown ABI fact command '" + command + "'");
}

string type_encoding(const Type & type)
{
  FactSubstitutionSink sink;
  string out;
  if(!emit_type(type, out, &sink)) {
    throw logic_error("ABI model mangler failed to encode fact type");
  }
  return out;
}

string function_symbol(const FunctionEncoding & function)
{
  FactSubstitutionSink sink;
  string out;
  if(!emit_function_encoding(function, out, &sink)) {
    throw logic_error("unable to encode ABI fact function");
  }
  return out;
}

string variable_symbol(const string & qualified_name)
{
  FunctionEncoding name;
  name.name_components = function_components_for_qualified(qualified_name, true);
  FactSubstitutionSink sink;
  string out;
  if(!emit_function_name_symbol(name, out, &sink)) {
    throw logic_error("unable to encode ABI fact variable");
  }
  return out;
}

string mangle_case(const AbiFactCase & fact_case)
{
  switch(fact_case.target.kind) {
  case ABI_MANGLE_TYPE:
    return type_encoding(fact_case.target.type);
  case ABI_MANGLE_FUNCTION:
    if(fact_case.target.c_linkage) {
      return unqualified_name(fact_case.target.qualified_name);
    }
    return function_symbol(fact_case.target.function);
  case ABI_MANGLE_VARIABLE:
    if(fact_case.target.c_linkage) {
      return unqualified_name(fact_case.target.qualified_name);
    }
    return variable_symbol(fact_case.target.qualified_name);
  case ABI_MANGLE_TYPEINFO: {
    string out;
    if(!emit_special_type_symbol_from_encoding(SPECIAL_TYPEINFO,
                                               type_encoding(fact_case.target.type),
                                               out)) {
      throw logic_error("unable to encode ABI fact typeinfo symbol");
    }
    return out;
  }
  case ABI_MANGLE_VTABLE: {
    string out;
    if(!emit_special_type_symbol_from_encoding(SPECIAL_VTABLE,
                                               type_encoding(fact_case.target.type),
                                               out)) {
      throw logic_error("unable to encode ABI fact vtable symbol");
    }
    return out;
  }
  case ABI_MANGLE_VTT: {
    string out;
    if(!emit_special_type_symbol_from_encoding(SPECIAL_VTT,
                                               type_encoding(fact_case.target.type),
                                               out)) {
      throw logic_error("unable to encode ABI fact VTT symbol");
    }
    return out;
  }
  case ABI_MANGLE_CONSTRUCTION_VTABLE: {
    string out;
    if(!emit_construction_vtable_symbol_from_encodings(
           type_encoding(fact_case.target.type),
           fact_case.target.base_offset,
           type_encoding(fact_case.target.base_type),
           out)) {
      throw logic_error("unable to encode ABI fact construction vtable symbol");
    }
    return out;
  }
  case ABI_MANGLE_THREAD_LOCAL_WRAPPER: {
    string body;
    if(!object_symbol_body(variable_symbol(fact_case.target.qualified_name), body)) {
      throw logic_error("unable to encode ABI fact TLS wrapper body");
    }
    string out;
    if(!emit_thread_local_wrapper_symbol_from_encoding(body, out)) {
      throw logic_error("unable to encode ABI fact TLS wrapper symbol");
    }
    return out;
  }
  case ABI_MANGLE_THUNK: {
    string out;
    if(!emit_virtual_override_thunk_symbol(function_symbol(fact_case.target.function),
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
    if(!emit_virtual_base_override_thunk_symbol(function_symbol(fact_case.target.function),
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

void flush_case(AbiFactFile & file, AbiFactCase & pending, ParseContext & ctx)
{
  if(pending.label.empty() && pending.facts.empty() &&
     pending.target.kind == ABI_MANGLE_NONE) {
    return;
  }
  if(pending.facts.empty() && pending.target.kind == ABI_MANGLE_NONE) {
    throw logic_error("case '" + pending.label + "' has no ABI fact");
  }
  file.cases.push_back(pending);
  pending = AbiFactCase();
  ctx = ParseContext();
}

AbiFactFile parse_fact_stream(istream & in, const string & input_name)
{
  AbiFactFile file;
  AbiFactCase pending;
  ParseContext ctx;
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
      flush_case(file, pending, ctx);
      pending.label = words[1];
      continue;
    }
    try {
      apply_fact_words(pending, ctx, words);
    } catch(const exception & e) {
      throw logic_error(input_name + ":" + to_string(line_number) + ": " + e.what());
    }
  }
  flush_case(file, pending, ctx);
  return file;
}

string builtin_name_from_code(const string & code)
{
  if(code == "v") { return "void"; }
  if(code == "b") { return "bool"; }
  if(code == "c") { return "char"; }
  if(code == "a") { return "schar"; }
  if(code == "h") { return "uchar"; }
  if(code == "s") { return "short"; }
  if(code == "t") { return "ushort"; }
  if(code == "i") { return "int"; }
  if(code == "j") { return "uint"; }
  if(code == "l") { return "long"; }
  if(code == "m") { return "ulong"; }
  if(code == "x") { return "longlong"; }
  if(code == "y") { return "ulonglong"; }
  if(code == "n") { return "int128"; }
  if(code == "o") { return "uint128"; }
  if(code == "w") { return "wchar"; }
  if(code == "Ds") { return "char16"; }
  if(code == "Di") { return "char32"; }
  if(code == "f") { return "float"; }
  if(code == "d") { return "double"; }
  if(code == "e") { return "longdouble"; }
  if(code == "Dn") { return "nullptr"; }
  return string();
}

bool is_decimal_text(const string & text)
{
  if(text.empty()) {
    return false;
  }
  for(size_t i = 0; i < text.size(); ++i) {
    if(!isdigit(static_cast<unsigned char>(text[i]))) {
      return false;
    }
  }
  return true;
}

string bool_fact_word(bool value)
{
  return value ? "yes" : "no";
}

string raw_array_bound_word(const string & bound)
{
  return is_decimal_text(bound) ? bound : string("raw:") + bound;
}

string type_component_name(const Type::NameComponent & component)
{
  return component.std_abbrev ? string("std") : component.source_name;
}

string function_component_name(const FunctionNameComponent & component)
{
  return component.std_abbrev ? string("std") : component.source_name;
}

string qualified_type_name(const vector<Type::NameComponent> & prefix,
                           const string & terminal)
{
  vector<string> parts;
  for(size_t i = 0; i < prefix.size(); ++i) {
    parts.push_back(type_component_name(prefix[i]));
  }
  if(!terminal.empty()) {
    parts.push_back(terminal);
  }
  return join_qualified_parts(parts, parts.size());
}

bool qualified_function_name(const FunctionEncoding & function, string & out)
{
  if(function.name_components.empty()) {
    return false;
  }
  vector<string> parts;
  for(size_t i = 0; i < function.name_components.size(); ++i) {
    const FunctionNameComponent & component = function.name_components[i];
    if(!component.template_arguments.empty() ||
       component.source_name.empty()) {
      return false;
    }
    parts.push_back(function_component_name(component));
  }
  out = join_qualified_parts(parts, parts.size());
  return true;
}

struct FactSerializer
{
  vector<vector<string> > lines;
  set<string> used_ids;
  size_t next_type_id = 0;
  size_t next_arg_id = 0;
  size_t next_expr_id = 0;
  size_t next_context_id = 0;
  size_t next_entity_id = 0;

  void add_line(const vector<string> & words)
  {
    lines.push_back(words);
  }

  string next_id(const string & prefix, size_t & counter)
  {
    for(;;) {
      const string id = prefix + to_string(counter++);
      if(used_ids.insert(id).second) {
        return id;
      }
    }
  }

  void reserve_case_ids(const AbiFactCase & fact_case)
  {
    for(size_t i = 0; i < fact_case.facts.size(); ++i) {
      if(!fact_case.facts[i].id.empty()) {
        used_ids.insert(fact_case.facts[i].id);
      }
    }
  }

  bool compact_type_token(const Type & type, string & out)
  {
    switch(type.kind) {
    case Type::TK_BUILTIN:
      out = builtin_name_from_code(string(type.builtin_code));
      return !out.empty();
    case Type::TK_POINTER:
      if(type.inner && compact_type_token(*type.inner, out)) {
        out = "ptr:" + out;
        return true;
      }
      return false;
    case Type::TK_LVALUE_REFERENCE:
      if(type.inner && compact_type_token(*type.inner, out)) {
        out = "ref:" + out;
        return true;
      }
      return false;
    case Type::TK_RVALUE_REFERENCE:
      if(type.inner && compact_type_token(*type.inner, out)) {
        out = "rref:" + out;
        return true;
      }
      return false;
    case Type::TK_CV:
      if(type.inner && type.cv_const != type.cv_volatile &&
         compact_type_token(*type.inner, out)) {
        out = string(type.cv_const ? "const:" : "volatile:") + out;
        return true;
      }
      return false;
    case Type::TK_VENDOR_QUALIFIED:
      if(type.inner && !type.vendor_qualifier_name.empty() &&
         compact_type_token(*type.inner, out)) {
        out = "vendor:" + type.vendor_qualifier_name + ":" + out;
        return true;
      }
      return false;
    case Type::TK_BUILTIN_TYPE_TRANSFORM:
      if(type.inner && !type.builtin_transform_name.empty() &&
         compact_type_token(*type.inner, out)) {
        out = "transform:" + type.builtin_transform_name + ":" + out;
        return true;
      }
      return false;
    case Type::TK_NAMED:
      if(type.name && !type.name_owner &&
         type.name->template_arguments.empty() &&
         type.name->standard_substitution.empty() &&
         !type.name->template_name.empty()) {
        out = "named:" + qualified_type_name(type.name->prefix_components,
                                             type.name->template_name);
        return true;
      }
      return false;
    default:
      return false;
    }
  }

  string type_ref(const Type & type)
  {
    string token;
    if(compact_type_token(type, token)) {
      return token;
    }
    const string id = next_id("__abi_type", next_type_id);
    emit_type_fact(id, type);
    return id;
  }

  string arg_ref(const TemplateArgument & argument)
  {
    const string id = next_id("__abi_arg", next_arg_id);
    emit_template_argument_fact(id, argument);
    return id;
  }

  string class_arg_ref(const Type::ClassTemplateArgument & argument)
  {
    return arg_ref(template_arg_from_class_arg(argument));
  }

  string expr_ref(const DependentExpression & expression)
  {
    const string id = next_id("__abi_expr", next_expr_id);
    emit_expression_fact(id, expression);
    return id;
  }

  string context_ref(const string & fragment,
                     const vector<SubstitutionSlot> & slots,
                     const shared_ptr<FunctionEncoding> & function)
  {
    const string id = next_id("__abi_context", next_context_id);
    LocalContext context;
    context.context_fragment = fragment;
    context.context_substitution_slots = slots;
    context.context_function = function;
    emit_context_fact(id, context);
    return id;
  }

  string context_ref(const LocalContext & context)
  {
    const string id = next_id("__abi_context", next_context_id);
    emit_context_fact(id, context);
    return id;
  }

  string entity_ref(const AbiEntity & entity)
  {
    const string id = next_id("__abi_entity", next_entity_id);
    emit_entity_fact(id, entity);
    return id;
  }

  void emit_type_fact(const string & id, const Type & type)
  {
    vector<string> words;
    words.push_back("let-type");
    words.push_back(id);
    switch(type.kind) {
    case Type::TK_BUILTIN: {
      const string name = builtin_name_from_code(string(type.builtin_code));
      if(name.empty()) { throw logic_error("invalid builtin ABI type"); }
      words.push_back(name);
      add_line(words);
      return;
    }
    case Type::TK_CV:
      words.push_back(type.cv_const && type.cv_volatile ? "const-volatile" :
                      type.cv_const ? "const" : "volatile");
      if(!type.inner) { throw logic_error("cv ABI type has no operand"); }
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_POINTER:
      words.push_back("ptr");
      if(!type.inner) { throw logic_error("pointer ABI type has no operand"); }
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_LVALUE_REFERENCE:
      words.push_back("ref");
      if(!type.inner) { throw logic_error("reference ABI type has no operand"); }
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_RVALUE_REFERENCE:
      words.push_back("rref");
      if(!type.inner) { throw logic_error("rvalue reference ABI type has no operand"); }
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_ARRAY:
      words.push_back("array");
      if(!type.inner) { throw logic_error("array ABI type has no element"); }
      words.push_back(raw_array_bound_word(type.array_bound));
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_FUNCTION:
      words.push_back(type.variadic ? "function-type-variadic" : "function-type");
      if(!type.inner) { throw logic_error("function ABI type has no result"); }
      words.push_back(type_ref(*type.inner));
      for(size_t i = 0; i < type.params.size(); ++i) {
        words.push_back(type_ref(type.params[i]));
      }
      if(type.function_lvalue_ref) { words.push_back("lvalue-ref"); }
      if(type.function_rvalue_ref) { words.push_back("rvalue-ref"); }
      add_line(words);
      return;
    case Type::TK_MEMBER_POINTER:
      words.push_back("member-pointer");
      if(!type.owner || !type.inner) {
        throw logic_error("member pointer ABI type has missing operands");
      }
      words.push_back(type_ref(*type.owner));
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_VENDOR_QUALIFIED:
      words.push_back("vendor");
      if(type.vendor_qualifier_name.empty() || !type.inner) {
        throw logic_error("vendor ABI type has missing operands");
      }
      words.push_back(type.vendor_qualifier_name);
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_BUILTIN_TYPE_TRANSFORM:
      words.push_back("builtin-transform");
      if(type.builtin_transform_name.empty() || !type.inner) {
        throw logic_error("builtin transform ABI type has missing operands");
      }
      words.push_back(type.builtin_transform_name);
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_PACK_EXPANSION:
      words.push_back("pack");
      if(!type.inner) { throw logic_error("pack ABI type has no operand"); }
      words.push_back(type_ref(*type.inner));
      add_line(words);
      return;
    case Type::TK_TEMPLATE_PARAMETER:
      words.push_back(type_has_substitution(type) ?
                      "template-param-subst" :
                      "template-param");
      words.push_back(to_string(type.template_parameter_index));
      add_line(words);
      return;
    case Type::TK_NAMED:
      if(!type.name) { throw logic_error("named ABI type has no name metadata"); }
      if(type.name_owner) {
        words.push_back("member");
        words.push_back(type_ref(*type.name_owner));
        words.push_back(type.name->template_name);
      } else {
        words.push_back("name");
        words.push_back(qualified_type_name(type.name->prefix_components,
                                            type.name->template_name));
      }
      add_line(words);
      return;
    case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
      if(!type.name) {
        throw logic_error("class-template ABI type has no name metadata");
      }
      if(type.name->template_name_is_template_parameter) {
        words.push_back("template-param-template");
        words.push_back(to_string(type.name->template_name_parameter_index));
      } else if(type.name_owner) {
        words.push_back("member-template");
        words.push_back(type_ref(*type.name_owner));
        words.push_back(type.name->template_name);
      } else if(!type.name->standard_substitution.empty()) {
        words.push_back("std-template");
        words.push_back(type.name->standard_substitution);
        words.push_back(bool_fact_word(
            type.name->standard_substitution_includes_arguments));
        words.push_back(qualified_type_name(type.name->prefix_components,
                                            type.name->template_name));
      } else {
        words.push_back("template");
        words.push_back(qualified_type_name(type.name->prefix_components,
                                            type.name->template_name));
      }
      for(size_t i = 0; i < type.name->template_arguments.size(); ++i) {
        words.push_back(class_arg_ref(type.name->template_arguments[i]));
      }
      add_line(words);
      return;
    case Type::TK_DECLTYPE_EXPRESSION:
      words.push_back("decltype");
      if(!type.expression) { throw logic_error("decltype ABI type has no expression"); }
      words.push_back(expr_ref(*type.expression));
      add_line(words);
      return;
    case Type::TK_LAMBDA_CLOSURE:
      if(!type.lambda) { throw logic_error("lambda ABI type has no metadata"); }
      words.push_back(type.lambda->source_name.empty() ?
                      "lambda-closure" :
                      "local-type");
      words.push_back(context_ref(type.lambda->context_fragment,
                                  type.lambda->context_substitution_slots,
                                  type.lambda->context_function));
      if(type.lambda->source_name.empty()) {
        words.push_back(type.lambda->discriminator);
        for(size_t i = 0; i < type.params.size(); ++i) {
          words.push_back(type_ref(type.params[i]));
        }
      } else {
        words.push_back(type.lambda->source_name);
        words.push_back(type.lambda->discriminator);
      }
      add_line(words);
      return;
    case Type::TK_INVALID:
      break;
    }
    throw logic_error("invalid ABI type cannot be serialized");
  }

  void emit_template_argument_fact(const string & id,
                                   const TemplateArgument & argument)
  {
    vector<string> words;
    words.push_back("let-arg");
    words.push_back(id);
    switch(argument.kind) {
    case TemplateArgument::TAK_TYPE:
      if(!argument.value_type) {
        throw logic_error("type template argument has no type");
      }
      words.push_back("type");
      words.push_back(type_ref(*argument.value_type));
      add_line(words);
      return;
    case TemplateArgument::TAK_INTEGRAL_VALUE:
      if(!argument.value_type) {
        throw logic_error("integral template argument has no type");
      }
      words.push_back("value");
      words.push_back(type_ref(*argument.value_type));
      words.push_back(to_string(argument.integral_value));
      add_line(words);
      return;
    case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE:
      if(!argument.parameter_type) {
        throw logic_error("dependent integral template argument has no parameter type");
      }
      words.push_back("dependent-value");
      words.push_back(type_ref(*argument.parameter_type));
      words.push_back(argument.value_type ? type_ref(*argument.value_type) : "-");
      words.push_back(to_string(argument.integral_value));
      add_line(words);
      return;
    case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
      if(!argument.expression) {
        throw logic_error("expression template argument has no expression");
      }
      words.push_back("expression");
      words.push_back(expr_ref(*argument.expression));
      add_line(words);
      return;
    case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
      words.push_back("untyped-value");
      words.push_back(to_string(argument.integral_value));
      add_line(words);
      return;
    case TemplateArgument::TAK_TEMPLATE_ENTITY:
      if(!argument.metadata) {
        throw logic_error("template entity argument has no metadata");
      }
      if(argument.metadata->template_name_is_template_parameter) {
        words.push_back("template-param-template");
        words.push_back(to_string(argument.metadata->template_parameter_index));
      } else if(argument.metadata->template_owner_type) {
        words.push_back("member-template-entity");
        words.push_back(type_ref(*argument.metadata->template_owner_type));
        words.push_back(argument.metadata->template_name);
        words.push_back(argument.metadata->template_name_substitution.empty() ?
                        "-" :
                        argument.metadata->template_name_substitution);
      } else {
        words.push_back("template-entity");
        words.push_back(qualified_type_name(argument.metadata->prefix_components,
                                            argument.metadata->template_name));
      }
      add_line(words);
      return;
    case TemplateArgument::TAK_EXTERNAL_ENTITY:
      if(!argument.metadata) {
        throw logic_error("external entity argument has no metadata");
      }
      if(argument.metadata->external_entity_is_member &&
         argument.metadata->external_entity_owner_type) {
        words.push_back(argument.metadata->external_entity_address_of ?
                        "member-external-address" :
                        "member-external-reference");
        words.push_back(argument.metadata->external_entity_symbol);
        words.push_back(type_ref(*argument.metadata->external_entity_owner_type));
        words.push_back(argument.metadata->external_entity_member_name);
        words.push_back(bool_fact_word(argument.metadata->external_entity_is_function));
        words.push_back(bool_fact_word(argument.metadata->external_entity_function_const));
        words.push_back(bool_fact_word(argument.metadata->external_entity_function_volatile));
        words.push_back(bool_fact_word(argument.metadata->external_entity_function_lvalue_ref));
        words.push_back(bool_fact_word(argument.metadata->external_entity_function_rvalue_ref));
        words.push_back(bool_fact_word(argument.metadata->external_entity_function_variadic));
        for(size_t i = 0;
            i < argument.metadata->external_entity_parameter_types.size();
            ++i) {
          words.push_back(type_ref(
              argument.metadata->external_entity_parameter_types[i]));
        }
      } else {
        words.push_back(argument.metadata->external_entity_address_of ?
                        "external-address" :
                        "external-reference");
        words.push_back(argument.metadata->external_entity_symbol);
      }
      add_line(words);
      return;
    case TemplateArgument::TAK_ARGUMENT_PACK:
      if(!argument.metadata) {
        throw logic_error("argument pack has no metadata");
      }
      words.push_back("pack");
      for(size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
        words.push_back(arg_ref(argument.metadata->pack_arguments[i]));
      }
      add_line(words);
      return;
    case TemplateArgument::TAK_INVALID:
      break;
    }
    throw logic_error("invalid ABI template argument cannot be serialized");
  }

  void emit_expression_fact(const string & id,
                            const DependentExpression & expression)
  {
    vector<string> words;
    words.push_back("let-expr");
    words.push_back(id);
    switch(expression.kind) {
    case DependentExpression::EK_TEMPLATE_PARAMETER:
      words.push_back("template-param");
      words.push_back(to_string(expression.template_parameter_index));
      add_line(words);
      return;
    case DependentExpression::EK_FUNCTION_PARAMETER:
      words.push_back("function-param");
      words.push_back(to_string(expression.template_parameter_index));
      add_line(words);
      return;
    case DependentExpression::EK_LITERAL:
      words.push_back("literal");
      words.push_back(expression.text);
      add_line(words);
      return;
    case DependentExpression::EK_INTEGRAL_VALUE:
      if(!expression.owner_type) {
        throw logic_error("integral expression has no type");
      }
      words.push_back("integral-value");
      words.push_back(type_ref(*expression.owner_type));
      words.push_back(to_string(expression.integral_value));
      add_line(words);
      return;
    case DependentExpression::EK_MEMBER:
      if(!expression.owner_type) {
        throw logic_error("member expression has no owner type");
      }
      words.push_back("member");
      words.push_back(type_ref(*expression.owner_type));
      words.push_back(bool_fact_word(expression.close_member_owner));
      words.push_back(expression.text);
      for(size_t i = 0; i < expression.template_arguments.size(); ++i) {
        words.push_back(arg_ref(expression.template_arguments[i]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_OBJECT_MEMBER:
      if(!expression.inner) {
        throw logic_error("object-member expression has no object");
      }
      words.push_back("object-member");
      words.push_back(expression.op_code);
      words.push_back(expr_ref(*expression.inner));
      words.push_back(expression.text);
      for(size_t i = 0; i < expression.template_arguments.size(); ++i) {
        words.push_back(arg_ref(expression.template_arguments[i]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_UNARY:
      if(!expression.inner) {
        throw logic_error("unary expression has no operand");
      }
      words.push_back("unary");
      words.push_back(expression.op_code);
      words.push_back(expr_ref(*expression.inner));
      add_line(words);
      return;
    case DependentExpression::EK_BINARY:
      if(!expression.inner || expression.arguments.size() != 1) {
        throw logic_error("binary expression has invalid operands");
      }
      words.push_back("binary");
      words.push_back(expression.op_code);
      words.push_back(expr_ref(*expression.inner));
      words.push_back(expr_ref(expression.arguments[0]));
      add_line(words);
      return;
    case DependentExpression::EK_CONDITIONAL:
      if(!expression.inner || expression.arguments.size() != 2) {
        throw logic_error("conditional expression has invalid operands");
      }
      words.push_back("conditional");
      words.push_back(expr_ref(*expression.inner));
      words.push_back(expr_ref(expression.arguments[0]));
      words.push_back(expr_ref(expression.arguments[1]));
      add_line(words);
      return;
    case DependentExpression::EK_PACK_EXPANSION:
      if(!expression.inner) {
        throw logic_error("pack expression has no operand");
      }
      words.push_back("pack");
      words.push_back(expr_ref(*expression.inner));
      add_line(words);
      return;
    case DependentExpression::EK_CALL:
      if(!expression.inner) {
        throw logic_error("call expression has no callee");
      }
      words.push_back("call");
      words.push_back(expr_ref(*expression.inner));
      for(size_t i = 0; i < expression.arguments.size(); ++i) {
        words.push_back(expr_ref(expression.arguments[i]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_CONVERSION:
      if(!expression.owner_type) {
        throw logic_error("conversion expression has no type");
      }
      if(expression.op_code.empty() || expression.op_code == "cv") {
        words.push_back("conversion");
        words.push_back(type_ref(*expression.owner_type));
        for(size_t i = 0; i < expression.arguments.size(); ++i) {
          words.push_back(expr_ref(expression.arguments[i]));
        }
      } else {
        if(expression.arguments.size() != 1) {
          throw logic_error("cast expression has invalid operands");
        }
        words.push_back("cast");
        words.push_back(expression.op_code);
        words.push_back(type_ref(*expression.owner_type));
        words.push_back(expr_ref(expression.arguments[0]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_TEMPLATE_ID:
      words.push_back("template-id");
      words.push_back(expression.text);
      for(size_t i = 0; i < expression.template_arguments.size(); ++i) {
        words.push_back(arg_ref(expression.template_arguments[i]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_TYPE_TRAIT:
      words.push_back("type-trait");
      words.push_back(expression.text);
      for(size_t i = 0; i < expression.type_arguments.size(); ++i) {
        words.push_back(type_ref(expression.type_arguments[i]));
      }
      add_line(words);
      return;
    case DependentExpression::EK_SIZEOF_TYPE:
      if(!expression.owner_type) {
        throw logic_error("sizeof expression has no type");
      }
      words.push_back("sizeof-type");
      words.push_back(type_ref(*expression.owner_type));
      add_line(words);
      return;
    case DependentExpression::EK_EXTERNAL_ENTITY:
      words.push_back(expression.external_entity_address_of ?
                      "external-address" :
                      "external-reference");
      words.push_back(expression.text);
      add_line(words);
      return;
    case DependentExpression::EK_INVALID:
      break;
    }
    throw logic_error("invalid ABI expression cannot be serialized");
  }

  void append_function_path_tail(vector<string> & words,
                                 const FunctionEncoding & function)
  {
    for(size_t i = 0; i < function.template_arguments.size(); ++i) {
      words.push_back(arg_ref(function.template_arguments[i]));
    }
    if(function.has_result_type && function.result_type) {
      words.push_back("result");
      words.push_back(type_ref(*function.result_type));
    }
    for(size_t i = 0; i < function.parameter_types.size(); ++i) {
      words.push_back(type_ref(function.parameter_types[i]));
    }
    if(function.variadic) {
      words.push_back("variadic");
    }
  }

  bool try_function_path_words(const FunctionEncoding & function,
                               vector<string> & words)
  {
    if(function.lambda ||
       !function.terminal_fragment.empty() ||
       !function.terminal_source_name.empty() ||
       function.operator_terminal != FUNCTION_OPERATOR_NONE ||
       function.has_conversion_type ||
       function.nested_const ||
       function.nested_volatile ||
       function.nested_lvalue_ref ||
       function.nested_rvalue_ref ||
       !function.abi_tags.empty()) {
      return false;
    }
    string qualified;
    if(!qualified_function_name(function, qualified)) {
      return false;
    }
    words.push_back("path");
    words.push_back(qualified);
    append_function_path_tail(words, function);
    return true;
  }

  string local_context_fragment(const LocalContext & context)
  {
    if(context.context_function) {
      string body;
      FactSubstitutionSink sink;
      if(!emit_local_entity_context_function_encoding_body(
             *context.context_function, body, &sink)) {
        throw logic_error("unable to serialize local ABI context function");
      }
      return "Z" + body + "E";
    }
    return context.context_fragment;
  }

  void emit_context_fact(const string & id, const LocalContext & context)
  {
    vector<string> words;
    words.push_back("let-context");
    words.push_back(id);
    if(context.context_function) {
      vector<string> function_words;
      if(try_function_path_words(*context.context_function, function_words)) {
        words.push_back("function");
        words.insert(words.end(), function_words.begin(), function_words.end());
        add_line(words);
        return;
      }
    }
    words.push_back("raw");
    words.push_back(local_context_fragment(context));
    add_line(words);
  }

  void emit_entity_fact(const string & id, const AbiEntity & entity)
  {
    vector<string> words;
    words.push_back("let-entity");
    words.push_back(id);
    if(entity.kind == ABI_ENTITY_FUNCTION) {
      vector<string> function_words;
      if(try_function_path_words(entity.function, function_words)) {
        words.push_back("function");
        words.insert(words.end(), function_words.begin(), function_words.end());
        add_line(words);
        return;
      }
      words.push_back("symbol");
      words.push_back(function_symbol(entity.function));
      add_line(words);
      return;
    }
    words.push_back(entity.kind == ABI_ENTITY_SYMBOL ? "symbol" : "variable");
    words.push_back(entity.qualified_name);
    add_line(words);
  }

  void emit_existing_fact(const AbiFact & fact)
  {
    const string id = fact.id.empty() ?
        next_id("__abi_fact", next_type_id) :
        fact.id;
    used_ids.insert(id);
    switch(fact.kind) {
    case ABI_FACT_TYPE:
      emit_type_fact(id, fact.type);
      return;
    case ABI_FACT_TEMPLATE_ARGUMENT:
      emit_template_argument_fact(id, fact.template_argument);
      return;
    case ABI_FACT_EXPRESSION:
      emit_expression_fact(id, fact.expression);
      return;
    case ABI_FACT_LOCAL_CONTEXT:
      if(fact.context.context_function ||
         !fact.context.context_fragment.empty()) {
        emit_context_fact(id, fact.context);
      } else {
        emit_context_fact(id, LocalContext::function(fact.context_function));
      }
      return;
    case ABI_FACT_ENTITY:
      emit_entity_fact(id, fact.entity);
      return;
    }
  }

  void append_function_encoding_lines(const FunctionEncoding & function)
  {
    if(function.lambda) {
      const FunctionEncoding::LambdaMetadata & lambda = *function.lambda;
      vector<string> words;
      words.push_back(lambda.source_name.empty() ?
                      "lambda-context" :
                      "local-context");
      words.push_back(context_ref(lambda.context_fragment,
                                  lambda.context_substitution_slots,
                                  lambda.context_function));
      if(lambda.source_name.empty()) {
        words.push_back(lambda.discriminator);
        for(size_t i = 0; i < lambda.signature_parameter_types.size(); ++i) {
          words.push_back(type_ref(lambda.signature_parameter_types[i]));
        }
      } else {
        words.push_back(lambda.source_name);
        words.push_back(lambda.discriminator);
      }
      add_line(words);
    } else {
      for(size_t i = 0; i < function.name_components.size(); ++i) {
        const FunctionNameComponent & component = function.name_components[i];
        vector<string> words;
        if(component.std_abbrev) {
          words.push_back("name-std");
        } else if(component.template_arguments.empty()) {
          words.push_back("name-source");
          words.push_back(component.source_name);
          words.push_back(component.substitution_name.empty() ?
                          "-" :
                          component.substitution_name);
        } else {
          words.push_back("name-template");
          words.push_back(component.source_name);
          words.push_back(component.substitution_name.empty() ?
                          "-" :
                          component.substitution_name);
          words.push_back(component.complete_substitution_name.empty() ?
                          "-" :
                          component.complete_substitution_name);
          words.push_back(component.standard_substitution.empty() ?
                          "-" :
                          component.standard_substitution);
          words.push_back(bool_fact_word(
              component.standard_substitution_includes_arguments));
          for(size_t j = 0; j < component.template_arguments.size(); ++j) {
            words.push_back(arg_ref(component.template_arguments[j]));
          }
        }
        add_line(words);
      }
    }
    if(!function.terminal_fragment.empty()) {
      vector<string> words;
      words.push_back("terminal");
      const string terminal = fact_word_from_terminal_fragment(
          function.terminal_fragment);
      words.push_back(terminal.empty() ?
                      function.terminal_fragment :
                      terminal);
      add_line(words);
    }
    if(!function.terminal_source_name.empty()) {
      vector<string> words;
      words.push_back("terminal-source");
      words.push_back(function.terminal_source_name);
      add_line(words);
    }
    if(function.operator_terminal != FUNCTION_OPERATOR_NONE) {
      string terminal;
      if(!semantic_name_from_operator_terminal(function.operator_terminal,
                                               terminal)) {
        throw logic_error("invalid ABI operator terminal cannot be serialized");
      }
      vector<string> words;
      words.push_back("operator-terminal");
      words.push_back(terminal);
      if(function.operator_terminal == FUNCTION_OPERATOR_LITERAL) {
        words.push_back(function.operator_literal_suffix);
      }
      add_line(words);
    }
    if(function.has_conversion_type && function.conversion_type) {
      vector<string> words;
      words.push_back("conversion-terminal");
      words.push_back(type_ref(*function.conversion_type));
      add_line(words);
    }
    if(function.nested_const ||
       function.nested_volatile ||
       function.nested_lvalue_ref ||
       function.nested_rvalue_ref) {
      vector<string> words;
      words.push_back("function-qualifier");
      if(function.nested_const) { words.push_back("const"); }
      if(function.nested_volatile) { words.push_back("volatile"); }
      if(function.nested_lvalue_ref) { words.push_back("lvalue-ref"); }
      if(function.nested_rvalue_ref) { words.push_back("rvalue-ref"); }
      add_line(words);
    }
    for(size_t i = 0; i < function.abi_tags.size(); ++i) {
      vector<string> words;
      words.push_back("abi-tag");
      words.push_back(function.abi_tags[i]);
      add_line(words);
    }
    for(size_t i = 0; i < function.template_arguments.size(); ++i) {
      if(i == 0 && !function.template_prefix_key.empty()) {
        const SubstitutionKey & template_prefix_key =
            function.template_prefix_key.get();
        if(template_prefix_key.kind !=
           SubstitutionKey::SK_FUNCTION_TEMPLATE_PREFIX) {
          throw logic_error("invalid ABI function template prefix key");
        }
        vector<string> words;
        words.push_back("function-template-prefix");
        words.push_back(template_prefix_key.payload.empty() ?
                        "-" :
                        template_prefix_key.payload);
        add_line(words);
      }
      vector<string> words;
      words.push_back("function-template-arg");
      words.push_back(arg_ref(function.template_arguments[i]));
      add_line(words);
    }
    if(function.has_result_type && function.result_type) {
      vector<string> words;
      words.push_back("result");
      words.push_back(type_ref(*function.result_type));
      add_line(words);
    }
    for(size_t i = 0; i < function.parameter_types.size(); ++i) {
      vector<string> words;
      words.push_back("param");
      words.push_back(type_ref(function.parameter_types[i]));
      add_line(words);
    }
    if(function.variadic) {
      vector<string> words;
      words.push_back("variadic");
      add_line(words);
    }
  }

  void append_function_target(const string & command,
                              const FunctionEncoding & function,
                              bool c_linkage,
                              const string & qualified_name)
  {
    vector<string> words;
    words.push_back(command);
    if(c_linkage) {
      words.push_back("path");
      words.push_back(qualified_name);
      add_line(words);
      return;
    }
    words.push_back("encoding");
    add_line(words);
    append_function_encoding_lines(function);
  }

  void append_target(const AbiMangleTarget & target)
  {
    vector<string> words;
    switch(target.kind) {
    case ABI_MANGLE_NONE:
      return;
    case ABI_MANGLE_TYPE:
      words.push_back("type");
      words.push_back(type_ref(target.type));
      add_line(words);
      return;
    case ABI_MANGLE_FUNCTION:
      append_function_target(target.c_linkage ? "c-function" : "function",
                             target.function,
                             target.c_linkage,
                             target.qualified_name);
      return;
    case ABI_MANGLE_VARIABLE:
      words.push_back(target.c_linkage ? "c-variable" : "variable");
      words.push_back(target.qualified_name);
      add_line(words);
      return;
    case ABI_MANGLE_TYPEINFO:
      words.push_back("typeinfo");
      words.push_back(type_ref(target.type));
      add_line(words);
      return;
    case ABI_MANGLE_VTABLE:
      words.push_back("vtable");
      words.push_back(type_ref(target.type));
      add_line(words);
      return;
    case ABI_MANGLE_VTT:
      words.push_back("vtt");
      words.push_back(type_ref(target.type));
      add_line(words);
      return;
    case ABI_MANGLE_CONSTRUCTION_VTABLE:
      words.push_back("construction-vtable");
      words.push_back(type_ref(target.type));
      words.push_back(to_string(target.base_offset));
      words.push_back(type_ref(target.base_type));
      add_line(words);
      return;
    case ABI_MANGLE_THREAD_LOCAL_WRAPPER:
      words.push_back("thread-local-wrapper");
      words.push_back(target.c_linkage ? "c-variable" : "variable");
      words.push_back(target.qualified_name);
      add_line(words);
      return;
    case ABI_MANGLE_THUNK:
      words.push_back("thunk");
      words.push_back(to_string(target.this_adjust));
      if(target.has_result_adjust) {
        words.push_back(to_string(target.result_adjust));
      }
      words.push_back("function");
      words.push_back("encoding");
      add_line(words);
      append_function_encoding_lines(target.function);
      return;
    case ABI_MANGLE_VIRTUAL_BASE_THUNK:
      words.push_back("virtual-base-thunk");
      words.push_back(to_string(target.vcall_offset));
      words.push_back("function");
      words.push_back("encoding");
      add_line(words);
      append_function_encoding_lines(target.function);
      return;
    }
  }
};

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
    FactSerializer serializer;
    serializer.reserve_case_ids(fact_case);
    for(size_t j = 0; j < fact_case.facts.size(); ++j) {
      serializer.emit_existing_fact(fact_case.facts[j]);
    }
    serializer.append_target(fact_case.target);
    for(size_t j = 0; j < serializer.lines.size(); ++j) {
      out << join_words(serializer.lines[j]) << "\n";
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
    out << mangle_case(file.cases[i]) << "\n";
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
    out << mangle_fact_file(parse_fact_stream(in, input_paths[i]));
  }
  return out.str();
}

}  // namespace abi_mangle
